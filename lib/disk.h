#ifndef DISK_H
#define DISK_H

#include "./structs.h"
#include <string>
#include <vector>

using namespace std;

class Disk {
  public:
    Disk(Structs::MBR disk, string diskPath);
    static void makeDisk(string path, int size, string fit, string units);
    static void removeDisk(string path);
    static Structs::MBR findDisk (string path);
    void handelPartitions(string name, int size, string units, string type, string fit, string deleteInstruction, int add);
    void deletePartition(string name);
    void createPartition(string name, int size, string units, string type, string fit);
    void createLogicPartition(Structs::Partition extended, string name, string fit, string units, int size);
    void addOrRemoveDataFomPartition(string name, string units, int add);
    static bool validatePartitionNames(Structs::MBR disk, string name, string path);
    static vector<Structs::EBR> getLogicPartitions(Structs::Partition partition, string path);

    static Structs::Partition getPartition(Structs::MBR disk, string name, string path);

    typedef struct _Transition {
      int partition;
      int start;
      int end;
      int before;
      int after;
    } Transition;

    Structs::MBR partitionAdjustment(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u);

    string diskPath;
    Structs::MBR mainDisk;

};

#endif