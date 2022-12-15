#ifndef REPORT_H
#define REPORT_H

#include <iostream>
#include <vector>
#include "./mount.h"
#include "./structs.h"

using namespace std;

class Report {
  public:
    Report();
    void handelReports(Mount::MountedDisk mounted[99], string name, string path, string id);
    void createMbrReport(Structs::MBR mbr, string path, string reoprtPath);
    void createDiskReport(Structs::MBR mbr, string path, string reoprtPath);
    string createTableRow(string label, string value);
    string createTableHeader(string type);
  private:
    vector<char> alphabet = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    int getIndex(char letter);
};

#endif