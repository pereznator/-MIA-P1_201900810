#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <sstream>
#include "../lib/report.h"
#include "../lib/mount.h"
#include "../lib/utils.h"
#include "../lib/disk.h"
using namespace std;

Report::Report() {}

void Report::handelReports(Mount::MountedDisk mounted[99], string name, string path, string id) {
  path = Utils::getPath(path);
  if (!Utils::validateFileExtension(path, "html")) {
    throw runtime_error("La extension del reporte debe ser html.");
  }
  
  if (!Utils::compare(name, "mbr") && !Utils::compare(name, "disk")) {
    throw runtime_error("No se reconoce el reporte: " + name);
  }

  string firstTwoLetters = id.substr(0, 2);
  if (!Utils::compare(firstTwoLetters, "10")) {
    throw runtime_error("No se reconoce el id.");
  }
  char lastLetter = id[id.length() - 1];

  if (!count(alphabet.begin(), alphabet.end(), lastLetter)) {
    throw runtime_error("No se encontro la particion montada con id: " + id + " (alfabeto)");
  }

  string idxDiscoString = id.substr(2, id.length() - 3);
  if (!Utils::isNumber(idxDiscoString)) {
    throw runtime_error("No se encontro la particion montada con id: " + id + " (no es numero)");
  }
  int idxDisco = stoi(idxDiscoString) - 1;

  int idxPartition = getIndex(lastLetter);

  if (idxPartition == -1) {
    throw runtime_error("No se encontro la particion montada con id: " + id + " (idxPartition)");
  }

  if (mounted[idxDisco].mpartitions[idxPartition].status == '0') {
    throw runtime_error("No esta activa la particion montada con id: " + id);
  }

  string partitionName = mounted[idxDisco].mpartitions[idxPartition].name;
  string partitionPath = mounted[idxDisco].path;

  Structs::MBR diskMbr = Disk::findDisk(partitionPath);

  if (Utils::compare(name, "mbr")) {
    return createMbrReport(diskMbr, partitionPath, path);
  } else {
    return createDiskReport(diskMbr, partitionPath, path);
  }

}

void Report::createMbrReport(Structs::MBR mbr, string path, string reportPath) {
  string htmlStart = "<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\"> \n  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n <title>MBR Report</title>\n  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65\" crossorigin=\"anonymous\"> \n</head>\n";
  htmlStart += "<body>\n  <div class=\"container mt-3\">\n    <h1>Reporte MBR</h1>\n    <hr>\n  <table class=\"table table-striped text-center\">\n";
  htmlStart += createTableHeader("m");
  htmlStart += "      <tbody>\n";
  htmlStart += createTableRow("mbr_tamano", to_string(mbr.mbr_tamano));

  struct tm *tm;
  tm = localtime(&mbr.mbr_fecha_creacion);
  char mostrar_fecha [20];
  strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);

  htmlStart += createTableRow("mbr_fecha_creacion", mostrar_fecha);
  htmlStart += createTableRow("mbr_disk_signature", to_string(mbr.mbr_disk_signature));
  htmlStart += "      </tbody>\n";

  Structs::Partition mbrPartitions[4];
  mbrPartitions[0] = mbr.mbr_Partition_1;
  mbrPartitions[1] = mbr.mbr_Partition_2;
  mbrPartitions[2] = mbr.mbr_Partition_3;
  mbrPartitions[3] = mbr.mbr_Partition_4;

  for (int i = 0; i < 4; i++) {
    string status(1, mbrPartitions[i].part_status);
    string type(1, mbrPartitions[i].part_type);
    string fit(1, mbrPartitions[i].part_fit);
    htmlStart += createTableHeader("n");

    htmlStart += "      <tbody>\n";
    
    htmlStart += createTableRow("part_status", status);
    htmlStart += createTableRow("part_type", type);
    htmlStart += createTableRow("part_fit", fit);
    htmlStart += createTableRow("part_start", to_string(mbrPartitions[i].part_start));
    htmlStart += createTableRow("part_size", to_string(mbrPartitions[i].part_size));
    htmlStart += createTableRow("part_name", mbrPartitions[i].part_name);

    htmlStart += "      </tbody>\n";

    if (Utils::compare(type, "e")) {
      vector<Structs::EBR> logics = Disk::getLogicPartitions(mbrPartitions[i], path);
      for (Structs::EBR &logic : logics) {
        string lfit(1,logic.part_fit);
        string lstatus(1,logic.part_status);
        htmlStart += createTableHeader("l");

        htmlStart += "      <tbody>\n";

        htmlStart += createTableRow("part_status", lstatus);
        htmlStart += createTableRow("part_next", to_string(logic.part_next));
        htmlStart += createTableRow("part_fit", lfit);
        htmlStart += createTableRow("part_start", to_string(logic.part_start));
        htmlStart += createTableRow("part_size", to_string(logic.part_size));
        htmlStart += createTableRow("part_name", logic.part_name);
        
        htmlStart += "      </tbody>\n";
      }
    }
  }

  htmlStart += "    </table>\n  </div>\n  <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-kenU1KFdBIe4zVF0s0G1M5b4hcpxyD9F7jL+jjXkk+Q2h455rYXK/7HAuoJl+0I4\" crossorigin=\"anonymous\"></script>\n";
  htmlStart += "</body>\n</html>\n";


  FILE *file = fopen(reportPath.c_str(), "rb+");
  if (file == NULL) {
    Utils::createDirectoryForPath(reportPath);
  }

  ofstream htmlFile;
  htmlFile.open(reportPath, ios::out | ios::trunc);
  htmlFile << htmlStart;
  htmlFile.close();
  Utils::success("REP", "Se creó reporte MBR exitosamente.");
}

int Report::getIndex(char letter) {
  int i = 0;
  for (char &a : alphabet) {
    if (a == letter) {
      return i;
    }
    i++;
  }
  return -1;
}

string Report::createTableRow(string label, string value) {
  string row = "        <tr>\n          <td>"+label+"</td>\n          <td>"+value+"</td>\n        </tr>\n";
  return row;
}

string Report::createTableHeader(string type) {
  string tableRowClass = "";
  string partitionType = "";
  if (type == "l") {
    tableRowClass = "table-info";
    partitionType = "Partición Lógica";
  } else if (type == "n") {
    partitionType = "Partición";
    tableRowClass = "table-secondary";
  } else {
    partitionType = "MBR";
    tableRowClass = "table-dark";
  }
  string tableHeader = "      <thead>\n       <tr class=\""+tableRowClass+"\">\n          <th colspan=\"2\">"+partitionType+"</th>\n        </tr>\n     </thead>";
  return tableHeader;
}

void Report::createDiskReport(Structs::MBR mbr, string path, string reportPath) {
  string htmlStart = "<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\"> \n  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n <title>Disk Report</title>\n  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65\" crossorigin=\"anonymous\"> \n</head>\n";
  htmlStart += "<body>\n  <div class=\"container-fluid mt-3\">\n    <h1>Reporte Disk</h1>\n    <hr>\n  <table class=\"table table-bordered text-center\">\n";
  htmlStart += "      <tr>\n        <td class=\"bg-primary text-white\" style=\"width: 5%;\" rowspan=\"2\">MBR</td>\n";
  int total = mbr.mbr_tamano;
  Structs::Partition parts[4];
  parts[0] = mbr.mbr_Partition_1;
  parts[1] = mbr.mbr_Partition_2;
  parts[2] = mbr.mbr_Partition_3;
  parts[3] = mbr.mbr_Partition_4;\

  bool extendedFound = false;
  Structs::Partition extended;
  vector<Report::MbrEmptySpace> spaces;
  vector<Report::MbrEmptySpace> lspaces;
  for (int i = 0; i < 4; i++) {
    if (parts[i].part_status == '1') {
      Report::MbrEmptySpace aux;
      aux.size = parts[i].part_size;
      aux.start = parts[i].part_start;
      string html;
      float widthF = (aux.size*100)/mbr.mbr_tamano;
      ostringstream ss;
      ss << widthF;
      string width(ss.str());
      if (parts[i].part_type == 'P') {
        aux.type = 'P';
        html = "        <td class=\"bg-light\" style=\"width: "+width+"%\" rowspan=\"2\">Primaria <br> <small>"+width+"% del disco</small></td>\n";
      } else {
        extendedFound = true;
        extended = parts[i];
        aux.type = 'E';
        html = "        <td class=\"bg-success text-white\" style=\"width: "+width+"%\">Extendida<br> <small>"+width+"% del disco</small></td>\n";
      }
      strcpy(aux.html, html.c_str());
      spaces.push_back(aux);
    }
  }
  if (extendedFound) {
    vector<Structs::EBR> lpartitions = Disk::getLogicPartitions(extended, path);
    if (lpartitions.size() > 0)  {
      for (Structs::EBR &lpart : lpartitions) {
        if (lpart.part_status == '1') {
          Report::MbrEmptySpace aux;
          aux.type = 'l';
          aux.size = lpart.part_size;
          aux.start = lpart.part_start;
          float widthF = (lpart.part_size * 12)/extended.part_size;
          int widthI = int(widthF);
          float percentage = (lpart.part_size * 100)/mbr.mbr_tamano;
          ostringstream ss;
          ss << percentage;
          string widthPercentage(ss.str());
          string html = "           <div class=\"col-1 border border-secondary bg-light\">EBR</div>\n            <div class=\"col-"+to_string(widthI)+" border border-secondary bg-light\">Logica <br> <small>"+widthPercentage+"%</small></div>\n";
          strcpy(aux.html, html.c_str());
          lspaces.push_back(aux);
        }
      }
    }
  }
  if (spaces.size() == 0) {
    throw runtime_error("No se encontraron particiones en el disco");
  }

  int startingPoint = sizeof(Structs::MBR);

  vector<Report::MbrEmptySpace> emptySpaces;

  for (Report::MbrEmptySpace &space: spaces) {
    if (space.start == startingPoint) {
      startingPoint += space.size;
      continue;
    }
    if (startingPoint < space.start) {
      Report::MbrEmptySpace aux;
      aux.start = startingPoint;
      aux.size = space.start - startingPoint;
      float widthF = (aux.size*100)/mbr.mbr_tamano;
      ostringstream ss;
      ss << widthF;
      string width(ss.str());
      string html = "       <td class=\"bg-secondary text-white\" style=\"width: "+width+"%\" rowspan=\"2\">Libre <br> <small>"+width+"% del disco</small></td>\n";
      strcpy(aux.html, html.c_str());
      emptySpaces.push_back(aux);
      startingPoint = space.start + space.size;
    }
  }

  if (spaces.at(spaces.size() - 1).start + spaces.at(spaces.size() - 1).size < mbr.mbr_tamano) {
    Report::MbrEmptySpace aux;
    aux.start = spaces.at(spaces.size() - 1).start + spaces.at(spaces.size() - 1).size;
    aux.size = mbr.mbr_tamano - aux.start;
    float widthF = (aux.size*100)/mbr.mbr_tamano;
    ostringstream ss;
    ss << widthF;
    string width(ss.str());
    string html = "       <td class=\"bg-secondary text-white\" style=\"width: "+width+"%\" rowspan=\"2\">Libre <br> <small>"+width+"% del disco</small></td>\n";
    strcpy(aux.html, html.c_str());

    emptySpaces.push_back(aux);
  }
  
  if (emptySpaces.size() > 0) {
    spaces.insert(spaces.end(), emptySpaces.begin(), emptySpaces.end());
  }

  Report::MbrEmptySpace orderAux;
  for (int i = spaces.size() - 1; i >= 0; i--) {
    for (int j = 0; j < i; j++) {
      if (spaces.at(j).start > spaces.at(j+1).start) {
        orderAux = spaces.at(j+1);
        spaces.at(j+1) = spaces.at(j);
        spaces.at(j) = orderAux;
      }
    }
  }

  for (Report::MbrEmptySpace &space: spaces) {
    htmlStart += space.html;
  }

  htmlStart += "      </tr>\n     <tr>\n        <td class=\"bg-dark\">\n          <div class=\"row\">\n";

  if (lspaces.size() > 0) {
    vector<Report::MbrEmptySpace> lemptySpaces;
    int lIndex = extended.part_start;

    for (Report::MbrEmptySpace lempty: lspaces) {
      if (lempty.start == lIndex) {
        lIndex += lempty.size + sizeof(Structs::EBR);
        continue;
      }
      if (lIndex < lempty.start) {
        Report::MbrEmptySpace aux;
        aux.start = lIndex;
        aux.size = lempty.start - lIndex;
        float widthF = (aux.size * 12)/extended.part_size;
        int widthI = int(widthF);

        float percentageWidth = (aux.size*100)/mbr.mbr_tamano;
        ostringstream ss;
        ss << percentageWidth;
        string width(ss.str());
        string html = "            <div class=\"col-"+to_string(widthI)+" border border-secondary bg-light \">Libre <br>"+width+"%</div>\n";
        strcpy(aux.html, html.c_str());
        lemptySpaces.push_back(aux);
        lIndex = lempty.start + sizeof(Structs::EBR) + lempty.size;
      }
    }

    if (lspaces.at(lspaces.size() - 1).start + lspaces.at(lspaces.size() - 1).size < extended.part_start + extended.part_size) {
      Report::MbrEmptySpace aux;
      aux.size = extended.part_start + extended.part_size - lspaces.at(lspaces.size() - 1).start - lspaces.at(lspaces.size() - 1).size;
      aux.start = extended.part_start + extended.part_size - aux.size;
      float widthF = (aux.size * 12)/extended.part_size;
      int widthI = int(widthF);

      float percentageWidth = (aux.size*100)/mbr.mbr_tamano;
      ostringstream ss;
      ss << percentageWidth;
      string width(ss.str());
      string html = "            <div class=\"col-"+to_string(widthI)+" border border-secondary bg-light\">Libre <br>"+width+"%</div>\n"; 
      strcpy(aux.html, html.c_str());
      lemptySpaces.push_back(aux);
    }


    if (lemptySpaces.size() > 0) {
      lspaces.insert(lspaces.end(), lemptySpaces.begin(), lemptySpaces.end());
    }

    Report::MbrEmptySpace lEmptyAux;
    for (int i = lspaces.size() - 1; i >= 0; i--) {
      for (int j = 0; j < i; j++) {
        if (lspaces.at(j).start > lspaces.at(j+1).start) {
          lEmptyAux = lspaces.at(j+1);
          lspaces.at(j+1) = lspaces.at(j);
          lspaces.at(j) = lEmptyAux;
        }
      }
    }

    for (Report::MbrEmptySpace &lemp : lspaces) {
      htmlStart += lemp.html;
    }
  }

  htmlStart += "          </div>\n        </td>\n      </tr>\n    </table>\n  </div>\n";
  htmlStart += "<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-kenU1KFdBIe4zVF0s0G1M5b4hcpxyD9F7jL+jjXkk+Q2h455rYXK/7HAuoJl+0I4\" crossorigin=\"anonymous\"></script>\n</body>\n</html>";

  FILE *file = fopen(reportPath.c_str(), "rb+");
  if (file == NULL) {
    Utils::createDirectoryForPath(reportPath);
  }

  ofstream htmlFile;
  htmlFile.open(reportPath, ios::out | ios::trunc);
  htmlFile << htmlStart;
  htmlFile.close();
  Utils::success("REP", "Se creó reporte DISK exitosamente.");

}