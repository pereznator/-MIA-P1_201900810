#include "../lib/scanner.h"
#include "../lib/disk.h"
#include "../lib/utils.h"
#include "../lib/mount.h"
#include "../lib/report.h"
#include <iostream>
#include <stdlib.h>
#include <locale>
#include <cstring>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <map>

using namespace std;

Mount mount;

Scanner::Scanner() {}

void Scanner::start() {
  while (true) {
    cout << "======= Ingresar Coamdndo =======" << endl;
    cout << "--------- exit para salir ---------" << endl;
    cout << ">>";
    string text;
    getline(cin, text);
    if (Utils::compare(text, "exit")) {
      cout << "BYE BYE";
      break;
    }
    text = "exec -path=../script.mia";
    string firstToken = getToken(text);
    text.erase(0, firstToken.length() + 1);
    vector<string> tokens = splitTokens(text);
    processData(firstToken, tokens);
  }  
}

void Scanner::processData(string instruction, vector<string> tks) {
  map<string, string> params = getParameters(tks);

  if (Utils::compare(instruction, "mkdisk")) {
    if (!Utils::validateParameters(params, { "path", "s" })) {
      return Utils::displayErrorMessage("MKDISK", "No se ingreso el parametro path.");
    }

    if (!Utils::isNumber(params["s"])) {
      return Utils::displayErrorMessage("MKDISK", "El parametro size solo puede ser un numero.");
    }

    return Disk::makeDisk(params["path"], stoi(params["s"]), params["f"], params["u"]);
  }
  if (Utils::compare(instruction, "rmdisk")) {

    if (!Utils::validateParameters(params, { "path" })) {
      return Utils::displayErrorMessage("RMDISK", "Parametro path es obligatorio para comando rmdisk.");
    }

    return Disk::removeDisk(params["path"]);
  }
  if (Utils::compare(instruction, "fdisk")) {
    if (!Utils::validateParameters(params, { "path", "name" })) {
      return Utils::displayErrorMessage("FDISK", "Parametros para la ruta o el nombre de la particion faltan.");
    }
    int size = 0;
    if (params.count("s") == 1){
      if (!Utils::isNumber(params["s"])) {
        return Utils::displayErrorMessage("FDISK", "El parametro size solo puede ser un numero.");
      }
      size = stoi(params["s"]);
    }

    int addParam = 0;
    if (params.count("add") == 1) {
      bool isNegative = false;
      if (params["add"].substr(0,1) == "-") {
        isNegative = true;
        params["add"] = params["add"].substr(1, params["add"].length() + 1);
      }
      if (!Utils::isNumber(params["add"])) {
        return Utils::displayErrorMessage("FDISK", "El parametro add solo puede ser un numero.");
      }
      addParam = stoi(params["add"]);
      if (isNegative) {
        addParam = -addParam;
      }
    }

    try {
      Structs::MBR diskMbr = Disk::findDisk(params["path"]);
      Disk disk(diskMbr, params["path"]);
      return disk.handelPartitions(params["name"], size, params["u"], params["t"], params["f"], params["delete"], addParam);
    } catch (exception &e) {
      return Utils::displayErrorMessage("FDSK", e.what());
    }

  }
  if (Utils::compare(instruction, "mount")) {
    try {
      if (!Utils::validateParameters(params, { "path", "name" })) {
        throw runtime_error("Mount necesita parametros path y name para ejecutarse.");
      }
      Structs::MBR diskMbr = Disk::findDisk(params["path"]);
      Structs::Partition diskPartition = Disk::getPartition(diskMbr, params["name"], params["path"]);
      Disk foundDisk(diskMbr, params["path"]);
      mount.mountPartition(diskPartition, foundDisk);
      mount.listMounts();
      return;
    } catch (exception &e) {
      return Utils::displayErrorMessage("MOUNT", e.what());
    }
  }
  if (Utils::compare(instruction, "unmount")) {
    try {
      if (!Utils::validateParameters(params, { "id"})) {
        throw runtime_error("Id no ingresado.");
      }
      return mount.unmount(params["id"]);
    } catch (exception &e) {
      return Utils::displayErrorMessage("UNMOUNT", e.what());
    }
  }
  if (Utils::compare(instruction, "mkfs")) {
    return;
  }
  if (Utils::compare(instruction, "login")) {
    return;
  }
  if (Utils::compare(instruction, "logout")) {
    return;
  }
  if (Utils::compare(instruction, "mkgrp")) {
    return;
  }
  if (Utils::compare(instruction, "rmgrp")) {
    return;
  }
  if (Utils::compare(instruction, "mkusr")) {
    return;
  }
  if (Utils::compare(instruction, "rmusr")) {
    return;
  }
  if (Utils::compare(instruction, "mkdir")) {
    return;
  }
  if (Utils::compare(instruction, "rep")) {
    try {
      if (!Utils::validateParameters(params, { "name", "path", "id" })) {
        throw runtime_error("Faltan parametros necesarios para comando rep.");
      }
      Report rep;
      rep.handelReports(mount.mounted, params["name"], params["path"], params["id"]);
    } catch(exception &e) {
       Utils::displayErrorMessage("REP", e.what());;
    }
    return;
  }
  if (Utils::compare(instruction, "exec")) {
    if (params.count("path") != 1) {
      Utils::displayErrorMessage("EXEC", "No se ingreso parametro path para leer el archivo de comandos.");
      return;
    }
    exec(params["path"]);
    return;
  }
  if (Utils::compare(instruction, "pause")) {
    cout << "Presione cualquier tecla para continuar..." << endl;
    string nextIns;
    cin >> nextIns;
    return;
  }
  if (Utils::compare(instruction.substr(0,1), "#")) {
    return Utils::comment(instruction);
  }
  Utils::displayErrorMessage("SYSTEM", "No se reconoce el comando ingresado \""+ instruction + "\"");
}

string Scanner::getToken(string text) {
  string token = "";
  bool endCycle = false;
  for (char &c : text) {
    if (endCycle) {
      if (c == ' ' || c == '-') {
        break;
      }
      token += c;
    }else if (c != ' ' && !endCycle) {
      if (c == '#') {
        token = text;
        break;
      }else {
        token += c;
        endCycle = true;
      }
    }
  }
  return token;
}

vector<string> Scanner::split(string text, string textSplit) {
  vector<string> splitedText;
  if (text.empty()) {
    return splitedText;
  }
  int textLength = text.length();
  char charArray[textLength + 1];
  strcpy(charArray, text.c_str());
  char* point = strtok(charArray, textSplit.c_str());
  while (point != NULL) {
    splitedText.push_back(string(point));
    point = strtok(NULL, textSplit.c_str());
  }
  return splitedText;
}

map<string, string> Scanner::getParameters(vector<string> params) {
  map<string, string> mapParams;
  for (string &param : params) {
    string label = "";
    string value = "";
    bool foundEqual = false;
    for (char &c : param) {
      if (!foundEqual) {
        if (c != '=') {
          label += c;
        }else {
          foundEqual = true;
        }
      } else {
        value += c;
      }
    }
    mapParams[label] = value;
  }
  return mapParams;
}

vector<string> Scanner::splitTokens(string text) {
  vector<string> tokens;
  if (text.empty()) {
    return tokens;
  }
  text.push_back(' ');
  string token = "";
  int state = 0;
  for (char &c : text) {
    if (state == 0 && c == '-') {
      state = 1;
    }else if (state == 0 && c == '#') {
      continue;
    } else if (state != 0) {
      if (state == 1) {
        if (c == '=') {
          state = 2;
        }else if (c == ' ') {
          continue;
        }
      } else if (state == 2) {
        if (c == '\"') {
          state = 3;
        } else {
          state = 4;
        }
      } else if (state == 3) {
        if (c == '\"') {
          state = 4;
        }
      } else if (state == 4 && c == '\"') {
        tokens.clear();
        continue;
      } else if (state == 4 && c == ' ') {
        state = 0;
        tokens.push_back(token);
        token = "";
        continue;
      }
      token += c;
    }
  }
  return tokens;
}

void Scanner::funcionExec(vector<string> tokens) {

}

void Scanner::exec(string path) {
  fstream file (path, ios_base::in);
  if (!file) {
    Utils::displayErrorMessage("EXEC", "No se pudo encontrar el archivo usando la ruta ingresada.");
    return;
  }
  string line;
  while (getline(file, line)) {

    if (line.empty()) {
      continue;
    }

    if (Utils::compare(line, "exit")) {
      cout << "BYE BYE";
      break;
    }
    string firstToken = getToken(line);
    line.erase(0, firstToken.length() + 1);
    vector<string> tokens = splitTokens(line);
    processData(firstToken, tokens);
  }
  file.close();
}

