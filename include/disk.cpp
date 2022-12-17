#include "../lib/disk.h"
#include "../lib/utils.h"
#include "../lib/structs.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <algorithm>

using namespace std;

int startValue;
Structs::MBR mainDisk;
string diskPath;

Disk::Disk(Structs::MBR disk, string path) {
  mainDisk = disk;
  diskPath = Utils::getPath(path);
}

void Disk::makeDisk(string path, int size, string fit, string units) {
  if (size < 0) {
    return Utils::displayErrorMessage("MKDISK", "Size no puede ser menor a cero.");
  }
  if (fit.empty()) {
    fit = "BF";
  } else if (Utils::validateStringPossibleValues(fit, { "BF", "FF", "WF" }).empty()) {
     return Utils::displayErrorMessage("MKDISK", "Valor invalido para fit.");
  }
  if (units.empty()) {
    units = "M";
  } else if (Utils::validateStringPossibleValues(units, { "M", "K" }).empty()) {
     return Utils::displayErrorMessage("MKDISK", "Valor invalido para units.");
  }


  path = Utils::getPath(path);

  if (!Utils::validateFileExtension(path, "dsk")) {
    return Utils::displayErrorMessage("MKDISK", "El archivo debe tener la extension .dsk");
  }
  
  FILE *file = fopen(path.c_str(), "r");

  if (file != NULL) {
    fclose(file);
    return Utils::displayErrorMessage("MKDISK", "El archivo apuntado en la ruta ya existe.");
  }

  if (Utils::compare(units, "m")) {
    size = size * 1024 * 1024;
  }else if (Utils::compare(units, "k")) {
    size = size * 1024;
  }
  
  Structs::MBR newDisk;

  newDisk.mbr_tamano = size;
  newDisk.mbr_fecha_creacion = time(nullptr);
  newDisk.disk_fit = toupper(fit[0]);
  newDisk.mbr_disk_signature = random() % 999 + 500;
  newDisk.mbr_Partition_1 = Structs::Partition();
  newDisk.mbr_Partition_2 = Structs::Partition();
  newDisk.mbr_Partition_3 = Structs::Partition();
  newDisk.mbr_Partition_4 = Structs::Partition();

  try {
    FILE *firstTryFile = fopen(path.c_str(), "w+b");
    if (firstTryFile != NULL) {
      fwrite("\0", 1, 1, firstTryFile);
      fseek(firstTryFile, size-1, SEEK_SET);
      fwrite("\0", 1, 1, firstTryFile);
      rewind(firstTryFile);
      fwrite(&newDisk, sizeof(Structs::MBR), 1, firstTryFile);
      fclose(firstTryFile);
    } else {
      Utils::createDirectoryForPath(path);
      FILE *secondTryFile = fopen(path.c_str(), "w+b");
      fwrite("\0", 1, 1, secondTryFile);
      fseek(secondTryFile, size - 1, SEEK_SET);
      fwrite("\0", 1, 1, secondTryFile);
      rewind(secondTryFile);
      fwrite(&newDisk, sizeof(Structs::MBR), 1, secondTryFile);
      fclose(secondTryFile);
    }

    FILE *imprimir = fopen(path.c_str(), "r");
    if (imprimir != NULL){
      Structs::MBR discoI;
      fseek(imprimir, 0, SEEK_SET);
      fread(&discoI, sizeof(Structs::MBR), 1,imprimir);
      struct tm *tm;
      tm = localtime(&discoI.mbr_fecha_creacion);
      char mostrar_fecha [20];
      strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);                
      Utils::success("MKDISK","   Disco creado exitosamente");
      cout << "********Nuevo Disco********" << endl;
      cout << "Size:           "<< discoI.mbr_tamano << endl;
      cout << "Fecha:          "<< mostrar_fecha << endl;
      cout << "Fit:            "<< discoI.disk_fit << endl;
      cout << "Disk_Signature: "<< discoI.mbr_disk_signature << endl;
      cout << "Bits del MBR:   " << sizeof(Structs::MBR) << endl;
      cout << "Path:           "<< path << endl;
    }
    fclose(imprimir);
  } catch (const exception& e) {
    return Utils::displayErrorMessage("MKDISK", "No se pudo crear el disco.");
  }

}

void Disk::removeDisk(string path) {
  path = Utils::getPath(path);
  FILE *file = fopen(path.c_str(), "r");
  if (file == NULL) {
    return Utils::displayErrorMessage("RMDISK", "No se encontro ningun archivo en la ruta ingresada.");
  }
  if (!Utils::validateFileExtension(path, "dsk")) {
    return Utils::displayErrorMessage("RMDISK", "El archivo ingresado no tiene extension .dsk");
  }
  fclose(file);

  if (remove(path.c_str()) != 0) {
    return Utils::displayErrorMessage("RMDISK", "No se pudo eliminar el disco.");
  }

  return Utils::success("RMDISK", "Disk removed successfully.");

}

Structs::MBR Disk::findDisk(string path) {
  path = Utils::getPath(path);
  FILE *file = fopen(path.c_str(), "r");
  if (file == NULL) {
    throw runtime_error("No se encontro el dico en la ruta ingresada.");
  }
  Structs::MBR disk;
  fseek(file, 0, SEEK_SET);
  fread(&disk, sizeof(Structs::MBR), 1, file);
  fclose(file);
  return disk;
}

void Disk::handelPartitions(string name, int size, string units, string type, string fit, string deleteInstruction, int add) {
  if (name.length() > 16) {
    return Utils::displayErrorMessage("FDISK", "El nombre de la particion no puede superar los 16 caracteres.");
  }

  if (!deleteInstruction.empty()) {
    if (!Utils::compare(deleteInstruction, "full")) {
      throw runtime_error("Delete solo puede tener valor full."); 
    }
    return Disk::deletePartition(name);
  }

  if (add != 0) {
    return Disk::addOrRemoveDataFomPartition(name, units, add);
  }

  return Disk::createPartition(name, size, units, type, fit);

}

void Disk::createPartition(string name, int size, string units, string type, string fit) {
  if (units.empty()) {
    units = "k";
  }else if (Utils::validateStringPossibleValues(units, { "k", "m" }).empty()) {
    return Utils::displayErrorMessage("FDISK", "No se reconoce el valor para las unidades.");
  }
  if (type.empty()) {
    type ="p";
  } else if (Utils::validateStringPossibleValues(type, { "p", "e", "l" }).empty()) {
    return Utils::displayErrorMessage("FDISK", "No se reconoce el valor para el tipo.");
  }
  if (fit.empty()) {
    fit = "wf";
  } else if (Utils::validateStringPossibleValues(fit, { "bf", "ff", "wf" }).empty()) {
    return Utils::displayErrorMessage("FDISK", "No se reconoce el valor para el tipo de fit.");
  }

  vector<Structs::Partition> partitions = Utils::getPartitions(mainDisk);
  vector<Transition> between;

  int usedPartitions = 0;
  int extendedPartitions = 0;
  int c = 1;
  int base = sizeof(mainDisk);
  int usedSpace = sizeof(mainDisk);
  Structs::Partition extended;
  for(Structs::Partition p : partitions){
    if (p.part_status == '1') {
      usedSpace += p.part_size;
      Transition trn;
      trn.partition = c;
      trn.start = p.part_start;
      trn.end = p.part_start + p.part_size;

      trn.before = trn.start = base;
      base = trn.end;

      if (usedPartitions != 0) {
        between.at(usedPartitions - 1).after = trn.start - between.at(usedPartitions - 1).end;
      }
      between.push_back(trn);
      usedPartitions++;

      if (p.part_type == 'e' || p.part_type == 'E') {
        extendedPartitions++;
        extended = p;
      }
    }
    if (usedPartitions == 4 && !Utils::compare(type, "l")) {
      return Utils::displayErrorMessage("FDISK", "No se pueden crear mas particiones primarias");
    } else if (extendedPartitions == 1 && Utils::compare(type, "e")) {
      return Utils::displayErrorMessage("FDISK", "No se pueden crear mas particiones extendidas.");
    }
    c++;
  }

  if (extendedPartitions == 0 && Utils::compare(type, "l")) {
    return Utils::displayErrorMessage("FDISK", "No se puede crear una particion logica sin una extendida.");
  }

  if (usedPartitions != 0) {
    between.at(between.size() - 1).after = mainDisk.mbr_tamano - between.at(between.size() - 1).end;
  }

  if(Disk::validatePartitionNames(mainDisk, name, diskPath)) {
    return Utils::displayErrorMessage("FDISK", "El nombre " + name + " ya existe en las particiones del disco.");
  }
  
  if (Utils::compare(type, "l")) {
    return createLogicPartition(extended, name, fit, units, size);
  }

  int partitionSize = Utils::compare(units, "k") ? size * 1024 : size * 1024 * 1024;

  if (usedSpace + partitionSize > mainDisk.mbr_tamano) {
    return Utils::displayErrorMessage("FDISK", "No se puede crear una particion de tamaño " +to_string(partitionSize)+" porque supera el espacio disponible.");
  }

  Structs::Partition newPartition;
  newPartition.part_fit = toupper(fit[0]);
  strcpy(newPartition.part_name, name.c_str());
  newPartition.part_size = partitionSize;
  newPartition.part_status = '1';
  newPartition.part_type = toupper(type[0]);
  /*
  // new block of code
  Structs::Partition parts[4];
  parts[0] = mainDisk.mbr_Partition_1;
  parts[1] = mainDisk.mbr_Partition_2;
  parts[2] = mainDisk.mbr_Partition_3;
  parts[3] = mainDisk.mbr_Partition_4;

  int startValue = 0;
  int endValue = mainDisk.mbr_tamano;
  bool created = false;

  for (int idx = 0; idx < 4; idx++) {
    if (parts[idx].part_status == '1') {
      created = true;
      if (startValue < parts[idx].part_start) {
        startValue += parts[idx].part_status;
      } else {

      }
    } else {
      if (created) {

      }
    }
  }

  //end new block of code
  */
  mainDisk = partitionAdjustment(mainDisk, newPartition, between, partitions, usedPartitions);

  FILE *binaryFile = fopen(diskPath.c_str(), "rb+");
  fseek(binaryFile, 0, SEEK_SET);
  fwrite(&mainDisk, sizeof(Structs::MBR), 1, binaryFile);
  if(Utils::compare(type, "e")){
    Structs::EBR ebrSys;
    ebrSys.part_start = startValue;
    fseek(binaryFile, startValue, SEEK_SET);
    fwrite(&ebrSys, sizeof(Structs::EBR), 1, binaryFile);
  }
  fclose(binaryFile);
  Utils::success("FDISK", "Particion creada correctamente");

}

void Disk::createLogicPartition(Structs::Partition extended, string name, string fit, string units, int size) {
  if (Utils::compare(units, "k")) {
    size = size * 1024;
  } else {
    size = size * 1024 * 1024;
  }
  FILE *file = fopen(diskPath.c_str(), "rb+");
  rewind(file);
  fseek(file, extended.part_start, SEEK_SET);
  
  Structs::EBR aux;
  Structs::EBR previousPartition;
  fread(&aux, sizeof(Structs::EBR), 1, file);
  bool noMorePartitions = false;
  int startingPoint = extended.part_start;
  int contador = 0;
  do {
    contador++;
    if (aux.part_status == '1') {
      if (aux.part_next == -1) {
        noMorePartitions = true;
        previousPartition = aux;
        startingPoint = aux.part_start + aux.part_size + sizeof(Structs::EBR);
      } else {
        fseek(file, aux.part_next, SEEK_SET);
        fread(&aux, sizeof(Structs::EBR), 1, file);
      }
    } else {
      noMorePartitions = true;
    }
    if (contador > 10) {
      break;
    }
  } while (!noMorePartitions);

  if (startingPoint > extended.part_start + extended.part_size) {
    throw runtime_error("El punto de inicio supera el tamaño de la particion.");
  }
  if (startingPoint + size > extended.part_start + extended.part_size) {
    throw runtime_error("No cabe una particion logica con el tamaño ingresado.");
  }
  Structs::EBR newEbr;
  strcpy(newEbr.part_name, name.c_str());
  newEbr.part_size = size;
  newEbr.part_start = startingPoint;
  newEbr.part_fit = toupper(fit[0]);
  newEbr.part_status = '1';
  newEbr.part_next = -1;
  fseek(file, startingPoint, SEEK_SET);
  fwrite(&newEbr, sizeof(Structs::EBR), 1, file);

  Structs::Partition newLogicPartition;
  newLogicPartition.part_fit = toupper(fit[0]);
  strcpy(newLogicPartition.part_name, name.c_str());
  newLogicPartition.part_size = size;
  newLogicPartition.part_start = startingPoint + sizeof(Structs::EBR);
  newLogicPartition.part_status = '1';
  newLogicPartition.part_type = 'L';
  fseek(file, startingPoint + sizeof(Structs::EBR), SEEK_SET);
  fwrite(&newLogicPartition, sizeof(Structs::Partition), 1, file);

  if (previousPartition.part_status == '1') {
    previousPartition.part_next = startingPoint;
    fseek(file, previousPartition.part_start, SEEK_SET);
    fwrite(&previousPartition, sizeof(Structs::EBR), 1, file);
  }
  fclose(file);

  return Utils::success("FDISK", "Se creo una nueva particion logica.");
}


bool Disk::validatePartitionNames(Structs::MBR disk, string name, string path) {
  bool nameRepeated = false;
  Structs::Partition partitions[4];
  partitions[0] = disk.mbr_Partition_1;
  partitions[1] = disk.mbr_Partition_2;
  partitions[2] = disk.mbr_Partition_3;
  partitions[3] = disk.mbr_Partition_4;

  bool isExtended = false;
  Structs::Partition extendedPartition;

  for (Structs::Partition &partition : partitions) {
    if (partition.part_status == '1') {
      if (Utils::compare(partition.part_name, name)) {
        nameRepeated = true;
      } else if (partition.part_type == 'e' || partition.part_type == 'E') {
        isExtended = true;
        extendedPartition = partition;
      }
    }
  }

  if (isExtended) {
    vector<Structs::EBR> logicPartitions = Disk::getLogicPartitions(extendedPartition, path);
    if (logicPartitions.size() != 0) {
      for (Structs::EBR &logic : logicPartitions) {
        if (logic.part_status == '1') {
          if (Utils::compare(logic.part_name, name)) {
            nameRepeated = true;
          }
        }
      }
    }
  }
  return nameRepeated;
}

vector<Structs::EBR> Disk::getLogicPartitions(Structs::Partition partition, string path) {
  vector<Structs::EBR> logicPartitions;

  FILE *file = fopen(path.c_str(), "rb+");
  Structs::EBR aux;
  int contador = 0;
  fseek(file, partition.part_start, SEEK_SET);
  fread(&aux, sizeof(Structs::EBR), 1, file);

  bool morePartitions = true;
  do {
    contador++;
    if (aux.part_status == '0') {
      morePartitions = false;
    } else {
      logicPartitions.push_back(aux);
      if (aux.part_next == -1) {
        morePartitions = false;
      } else {
        fseek(file, aux.part_next, SEEK_SET);
        fread(&aux, sizeof(Structs::EBR), 1, file);
      }
    }
    if (contador > 10) {
      break;
    }
  } while (morePartitions);
  fclose(file);
  return logicPartitions;
}

Structs::MBR Disk::partitionAdjustment(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u){
  /*for (Transition tr : t) {
    cout << "=====Transitions=====" << endl;
    cout << "partition: " << tr.partition << endl;
    cout << "start    : " << tr.start << endl;
    cout << "end      : " << tr.end << endl;
    cout << "before   : " << tr.before << endl;
    cout << "after    : " << tr.after << endl;
  }*/
  if (u == 0) {
    p.part_start = sizeof(mbr);
    startValue = p.part_start;
    mbr.mbr_Partition_1 = p;
    return mbr;
  }
  Transition toUse;
  int c = 0;
  for (Transition tr : t) {
    if (c == 0) {
      toUse = tr;
      c++;
      continue;
    }
    if (toupper(mbr.disk_fit) == 'F') {
      if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
        break;
      }
      toUse = tr;
    } else if (toupper(mbr.disk_fit) == 'B') {
      if (toUse.before < p.part_size || toUse.after < p.part_size) {
        toUse = tr;
      } else {
        if (tr.before >= p.part_size || tr.after >= p.part_size) {
          int b1 = toUse.before - p.part_size;
          int a1 = toUse.after - p.part_size;
          int b2 = tr.before - p.part_size;
          int a2 = tr.after - p.part_size;

          if ((b1 < b2 && b1 < a2) || (a1 < b2 && a1 < a2)) {
            c++;
            continue;
          }
          toUse = tr;
        }
      }
    } else if (toupper(mbr.disk_fit) == 'W') {
      if (!(toUse.before >= p.part_size) || !(toUse.after >= p.part_size)) {
          toUse = tr;
      } else {
        if (tr.before >= p.part_size || tr.after >= p.part_size) {
          int b1 = toUse.before - p.part_size;
          int a1 = toUse.after - p.part_size;
          int b2 = tr.before - p.part_size;
          int a2 = tr.after - p.part_size;

          if ((b1 > b2 && b1 > a2) || (a1 > b2 && a1 > a2)) {
            c++;
            continue;
          }
          toUse = tr;
        }
      }
    }
    c++;
  }
  if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
    if (toupper(mbr.disk_fit) == 'F') {
      if (toUse.before >= p.part_size) {
        if (toUse.start - toUse.before != 0) {
          p.part_start = (toUse.start - toUse.before);
        }else {
          p.part_start = toUse.end;
        }
        startValue = p.part_start;
      } else {
        p.part_start = toUse.end;
        startValue = p.part_start;
      }
    } else if (toupper(mbr.disk_fit) == 'B') {
      int b1 = toUse.before - p.part_size;
      int a1 = toUse.after - p.part_size;

      if ((toUse.before >= p.part_size && b1 < a1) || !(toUse.after >= p.part_start)) {
        if (toUse.start - toUse.before != 0) {
          p.part_start = (toUse.start - toUse.before);
        }else {
          p.part_start = toUse.end;
        }
        startValue = p.part_start;
      } else {
        p.part_start = toUse.end;
        startValue = p.part_start;
      }
    } else if (toupper(mbr.disk_fit) == 'W') {
      int b1 = toUse.before - p.part_size;
      int a1 = toUse.after - p.part_size;

      if ((toUse.before >= p.part_size && b1 > a1) || !(toUse.after >= p.part_start)) {
        if (toUse.start - toUse.before != 0) {
          p.part_start = (toUse.start - toUse.before);
        }else {
          p.part_start = toUse.end;
        }
        startValue = p.part_start;
      } else {
        p.part_start = toUse.end;
        startValue = p.part_start;
      }
    }

    Structs::Partition partitions[4];
    for (int i = 0; i < ps.size(); i++) {
        partitions[i] = ps.at(i);
    }

    for (auto &partition : partitions) {
      if (partition.part_status == '0') {
        partition = p;
        break;
      }
    }

    Structs::Partition aux;
    for (int i = 3; i >= 0; i--) {
      for (int j = 0; j < i; j++) {
        if ((partitions[j].part_start > partitions[j + 1].part_start)) {
          aux = partitions[j + 1];
          partitions[j + 1] = partitions[j];
          partitions[j] = aux;
        }
      }
    }

    for (int i = 3; i >= 0; i--) {
      for (int j = 0; j < i; j++) {
        if (partitions[j].part_status == '0') {
          aux = partitions[j];
          partitions[j] = partitions[j + 1];
          partitions[j + 1] = aux;
        }
      }
    }
    mbr.mbr_Partition_1 = partitions[0];
    mbr.mbr_Partition_2 = partitions[1];
    mbr.mbr_Partition_3 = partitions[2];
    mbr.mbr_Partition_4 = partitions[3];
    return mbr;
  } else {
    throw runtime_error("No hay espacio suficiente para esta particion");
  }
}

void Disk::addOrRemoveDataFomPartition(string name, string units, int add) {
  if (units.empty()) {
    units = "k";
  }else if (Utils::validateStringPossibleValues(units, { "k", "m" }).empty()) {
    return Utils::displayErrorMessage("FDISK", "No se reconoce el valor para las unidades.");
  }
  Structs::Partition partition = getPartition(mainDisk, name, diskPath);

  if (Utils::compare(units, "k")) {
    add = add * 1024; 
  } else if (Utils::compare(units, "m")) {
    add = add * 1024 * 1024;
  }
  if (partition.part_size + add < 0) {
    throw runtime_error("No se puede quitar mas del espacio disponible en la particion.");
  }

  Structs::Partition partitions[4];
  partitions[0] = mainDisk.mbr_Partition_1;
  partitions[1] = mainDisk.mbr_Partition_2;
  partitions[2] = mainDisk.mbr_Partition_3;
  partitions[3] = mainDisk.mbr_Partition_4;

  int index = -1;
  for (int aux = 0; aux < 4; aux++) {
    if (Utils::compare(partitions[aux].part_name, partition.part_name)) {
      index = aux;
    }
  }

  if (index == -1) {
    throw runtime_error("No se pudo encontrar el indice de la particion");
  }

  if (index != 3) {
    if (partitions[index + 1].part_start != -1) {
      if (partitions[index].part_size + add + partitions[index].part_start > partitions[index + 1].part_start) {
        throw runtime_error("El monto agregado sobrepasa el limite.");
      }
    }
  }

  if (partitions[index].part_size + add + partitions[index].part_start > mainDisk.mbr_tamano) {
    throw runtime_error("El monto sobrepasa el limite del disco.");
  }

  partition.part_size += add;
  partitions[index] = partition;

   mainDisk.mbr_Partition_1 = partitions[0];
   mainDisk.mbr_Partition_2 = partitions[1];
   mainDisk.mbr_Partition_3 = partitions[2];
   mainDisk.mbr_Partition_4 = partitions[3];

  FILE *file = fopen(diskPath.c_str(), "rb+");
  rewind(file);
  fwrite(&mainDisk, sizeof(Structs::MBR), 1, file);
  fclose(file);
  Utils::success("FDISK", "la partición se ha aumentado correctamente");

}

void Disk::deletePartition(string name) {
  Structs::Partition partition = Disk::getPartition(mainDisk, name, diskPath);
  Structs::Partition partitions[4];

  partitions[0] = mainDisk.mbr_Partition_1;
  partitions[1] = mainDisk.mbr_Partition_2;
  partitions[2] = mainDisk.mbr_Partition_3;
  partitions[3] = mainDisk.mbr_Partition_4;

  if (partition.part_type == 'l' || partition.part_type == 'L') {

    vector<Structs::EBR> lpartitions;
    int startingPoint = -1;

    for (int i = 0; i < 4; i++) {
      if (partitions[i].part_status == '1' && (partitions[i].part_type == 'e' || partitions[i].part_type == 'E')) {
        startingPoint = partitions[i].part_start;
        lpartitions = Disk::getLogicPartitions(partitions[i], diskPath);
        break;
      }
    }

    Structs::EBR previousEbr;
    int idx = -1;
    for (Structs::EBR &logic : lpartitions) {
      idx++;
      if (Utils::compare(logic.part_name, name)) {
        break;
      }
      previousEbr = logic;
    }

    bool nextLogicExitst = false;
    Structs::EBR nextLogic;
    if (idx < lpartitions.size()) {
      nextLogicExitst = true;
      nextLogic = lpartitions[idx + 1];
    }
  
    return Utils::success("FDISK", "Particion logica eliminada correctamente.");
  }

  for (int i = 0; i < 4; i++) {
    if (Utils::compare(partitions[i].part_name, name)) {
      partitions[i].part_status = '0';
      char emptyName[16];
      strcpy(partitions[i].part_name, emptyName);
      partitions[i].part_size = 0;
      partitions[i].part_fit = ' ';
      partitions[i].part_start = -1;
      partitions[i].part_type = ' ';
      break;
    }
  }
  mainDisk.mbr_Partition_1 = partitions[0];
  mainDisk.mbr_Partition_2 = partitions[1];
  mainDisk.mbr_Partition_3 = partitions[2];
  mainDisk.mbr_Partition_4 = partitions[3];
  FILE *file = fopen(diskPath.c_str(), "rb+");
  fseek(file, 0, SEEK_SET);
  fwrite(&mainDisk, sizeof(Structs::MBR), 1, file);

  fseek(file, partition.part_start, SEEK_SET);
  fwrite(&partition, partition.part_size, 0, file);
  return Utils::success("FDISK", "Particion eliminada correctamente.");
}

Structs::Partition Disk::getPartition(Structs::MBR disk, string name, string path) {
  path = Utils::getPath(path);
  Structs::Partition foundPartition;
  Structs::Partition partitions[4];
  partitions[0] = disk.mbr_Partition_1;
  partitions[1] = disk.mbr_Partition_2;
  partitions[2] = disk.mbr_Partition_3;
  partitions[3] = disk.mbr_Partition_4;

  bool isExtended = false;
  bool notFounndPartition = true;
  Structs::Partition extendedPartition;

  for (Structs::Partition &partition : partitions) {
    if (partition.part_status == '1') {
      if (Utils::compare(partition.part_name, name)) {
        foundPartition = partition;
        notFounndPartition = false;
        break;
      } else if (partition.part_type == 'e' || partition.part_type == 'E') {
        isExtended = true;
        extendedPartition = partition;
      }
    }
  }

  if (isExtended && notFounndPartition) {
    vector<Structs::EBR> logicPartitions = Disk::getLogicPartitions(extendedPartition, path);
    if (logicPartitions.size() == 0) {
      throw runtime_error("No se encontro ninguna particion con el nombre \"" + name + "\".");
    }
    for (Structs::EBR &logic : logicPartitions) {
      if (logic.part_status == '1') {
        if (Utils::compare(logic.part_name, name)) {
          foundPartition.part_status = '1';
          foundPartition.part_type = 'L';
          foundPartition.part_fit = logic.part_fit;
          foundPartition.part_start = logic.part_start;
          foundPartition.part_size = logic.part_size;
          strcpy(foundPartition.part_name, logic.part_name);
          notFounndPartition = false;
        }
      }
    }
  }
  if (notFounndPartition) {
    throw runtime_error("No se encontro ninguna particion con el nombre \"" + name + "\".");
  }
  return foundPartition;
}