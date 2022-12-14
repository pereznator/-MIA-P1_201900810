#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include "../lib/mount.h"
#include "../lib/structs.h"
#include "../lib/utils.h"
#include "../lib/disk.h"


using namespace std;


Mount::Mount() {}

void Mount::mountPartition(Structs::Partition part, Disk disk) {
  cout << "Part name  : " << part.part_name << endl;
  cout << "Part fit   : " << part.part_fit << endl;
  cout << "Part size  : " << part.part_size << endl;
  cout << "Part start : " << part.part_start << endl;
  cout << "Part status: " << part.part_status << endl;
  cout << "Part type  : " << part.part_type << endl;
  string partName = part.part_name;
  string type(1, part.part_type);
  if (Utils::compare(type, "e")) {
    vector<Structs::EBR> ebrs = Disk::getLogicPartitions(part, disk.diskPath);
    if (ebrs.empty()) {
      throw runtime_error("No se puede montar una particion extendida");
    }
    Structs::EBR ebr = ebrs.at(0);
    partName = ebr.part_name;
  }

  MountedDisk usedMountedDisk;
  bool foundDisk = false;
  int index = -1;

  for (int i = 0; i < 99; i++) {
    if (mounted[i].path == partName) {
      usedMountedDisk = mounted[i];
      foundDisk = true;
      index = i;
      break;
    }
  }


  if (foundDisk) {
    for (int j = 0; j < 26; j++) {
      if (usedMountedDisk.mpartitions[j].status == '0') {
        usedMountedDisk.mpartitions[j].status = '1';
        usedMountedDisk.mpartitions[j].letter = alphabet.at(j);
        usedMountedDisk.mpartitions[j].num = index;
        strcpy(usedMountedDisk.mpartitions[j].name, partName.c_str());
        string re = to_string(index + 1) + alphabet.at(j);
        mounted[index] = usedMountedDisk;
        return Utils::success("MOUNT", "Se ha realizado correctamente el mount -id=10" + re);
      }
    }
  }

  MountedDisk availableMountedDisk;

  for (int i = 0; i < 99; i++) {
    if (mounted[i].status == '0') {
      availableMountedDisk = mounted[i];
      index = i;
      break;
    }
  }

  availableMountedDisk.status = '1';
  strcpy(availableMountedDisk.path, disk.diskPath.c_str());
  for (int j = 0; j < 26; j++) {
    if (availableMountedDisk.mpartitions[j].status == '0') {
      availableMountedDisk.mpartitions[j].status = '1';
      availableMountedDisk.mpartitions[j].letter = alphabet.at(j);
      availableMountedDisk.mpartitions[j].num = index;
      strcpy(availableMountedDisk.mpartitions[j].name, partName.c_str());
      string re = to_string(index + 1) + alphabet.at(j);
      mounted[index] = availableMountedDisk;
      return Utils::success("MOUNT", "Se ha realizado correctamente el mount -id=10" + re);
    }
  }

  throw runtime_error("No se pudo montar la particion.");

}

void Mount::listMounts() {
  cout << "\n<========================== MOUNTS ==========================>" << endl;
  for (int i = 0; i < 99; i++) {
    for (int j = 0; j < 26; j++) {
      if (mounted[i].mpartitions[j].status == '1') {
        cout << "=> 10" << i + 1 << alphabet.at(j) << " : " << mounted[i].mpartitions[j].name  << endl;
      }
    }
  }
}

void Mount::unmount(string id) {
  string idString = id;
  string firstTwoLetters = id.substr(0, 2);
  if (firstTwoLetters == "10") {
    idString = id.substr(2, id.length() - 1);
  }
  
  char lastLetter = idString[idString.length() - 1];

  if (!count(alphabet.begin(), alphabet.end(), lastLetter)) {
    throw runtime_error("No se encontro la particion montada con id: " + id);
  }

  int idxPartition = getIndex(lastLetter);
  if (idxPartition == -1) {
    throw runtime_error("No se encontro la particion montada con id: " + id);
  }

  string diskNum = idString.substr(0, idString.length() - 1);

  if (!Utils::isNumber(diskNum)) {
    throw runtime_error("No se encontro la particion montada con id: " + id);
  }


  int idxDisco = stoi(diskNum) - 1;

  if (idxDisco >= 99 || idxDisco < 0) {
    throw runtime_error("No se encontro la particion montada con id: " + id);
  }
  if (mounted[idxDisco].mpartitions[idxPartition].status == -1) {
    throw runtime_error("No se encontro la particion montada con id: " + id);
  }
  mounted[idxDisco].mpartitions[idxPartition].status = -1;
  strcpy(mounted[idxDisco].mpartitions[idxPartition].name, "");
  mounted[idxDisco].mpartitions[idxPartition].num = -1;
  mounted[idxDisco].mpartitions[idxPartition].letter = '-';
  Utils::success("UNMOUNT", "Se desmonto la particion con id: " + id);
  return listMounts();
}

int Mount::getIndex(char letter) {
  int i = 0;
  for (char &a : alphabet) {
    if (a == letter) {
      return i;
    }
    i++;
  }
  return -1;
}