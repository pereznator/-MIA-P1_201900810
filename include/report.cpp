#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
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
  string htmlStart = "<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\"> \n  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n <title>Report</title>\n  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65\" crossorigin=\"anonymous\"> \n</head>\n";
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
  cout << "checkpoint " << endl;

  ofstream htmlFile;
  htmlFile.open(reportPath, ios::out | ios::trunc);
  htmlFile << htmlStart;
  htmlFile.close();

}

void Report::createDiskReport(Structs::MBR mbr, string path, string reportPath) {

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