#include "../lib/utils.h"
#include "../lib/structs.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

Utils::Utils() {}

bool Utils::compare(string text1, string text2) {
  if (toUpperCase(text1) == toUpperCase(text2)) {
    return true;
  }
  return false;
}
string Utils::toUpperCase(string text) {
  string upper = "";
  for (char &text : text) {
    upper += toupper(text);
  }
  return upper;
}
bool Utils::confirm(string message) {
  cout << message << "Confirmar(S), cualquier otra letra para cancelar" << endl;
  string respuesta;
  getline(cin,respuesta);
  if (compare(respuesta, "s")){
      return true;
  }
  return false;
}
void Utils::success(string method, string message){
  cout << "\033[0;42m(" + method + ")~~> \033[0m"<< message << endl;
}
void Utils::displayErrorMessage(string method, string message){
  cout << "\033[1;41m Error\033"<< "\033[0;31m(" + method + ")~~> \033[0m"<< message << endl;
}

void Utils::comment(string text) {
  cout << "\033[0;43m(COMMENT)~~> \033[0m"<< text << endl;
}

bool Utils::validateParameters(map<string, string> params, vector<string> requiredParams) {
  bool validated = true;
  for (string &label : requiredParams) {
    if (params.count(label) != 1) {
      validated = false;
    }
  }
  return validated;
}

string Utils::validateStringPossibleValues(string text, vector<string> possibleValues) {
  string finalText;
  for (string &value : possibleValues) {
    if (compare(text, value)) {
      finalText = value;
    }
  }
  return finalText;
}

bool Utils::isNumber(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

string Utils::getPath(string text) {
  string path = "";
  for (char &c : text) {
    if (c != '\"') {
      path += c;
    }
  }
  return path;
}

bool Utils::validateFileExtension(string path, string extension) {
  return compare(path.substr(path.find_last_of(".") + 1, extension.length()), extension);
}

void Utils::createDirectoryForPath(string path) {
  string comando1 = "mkdir -p \""+ path + "\"";
  string comando2 = "rmdir \""+ path + "\"";
  system(comando1.c_str());
  system(comando2.c_str());
}

vector<Structs::Partition> Utils::getPartitions(Structs::MBR disk) {
  vector<Structs::Partition> partitions;
  partitions.push_back(disk.mbr_Partition_1);
  partitions.push_back(disk.mbr_Partition_2);
  partitions.push_back(disk.mbr_Partition_3);
  partitions.push_back(disk.mbr_Partition_4);
  return partitions;
}