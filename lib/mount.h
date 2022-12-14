#ifndef MOUNT_H
#define MOUNT_H

#include "./structs.h"
#include "./disk.h"
#include <vector>

class Mount {
  public:
    Mount();
    void mountPartition(Structs::Partition partition, Disk disk);
    void listMounts();
    void unmount(string id);
    typedef struct _MP {
        char letter;
        int num;
        char status = '0';
        char name[16];
    } MountedPartition;

    typedef struct _MD {
        char path[150];
        char status = '0';
        MountedPartition mpartitions[26];
    } MountedDisk;


    MountedDisk mounted[99];

    Structs::Partition getMount(string id, string *p);

  private:
    vector<char> alphabet = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    int getIndex(char letter);

};

#endif