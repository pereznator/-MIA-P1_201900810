#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "./structs.h"
#include <map>
#include <vector>

using namespace std;

class Utils {
  public:
    Utils();
    static bool compare(string text1, string text2);
    static string toUpperCase(string text);
    static bool confirm(string message);
    static void success(string method, string message);
    static void displayErrorMessage(string method, string message);
    static bool validateParameters(map<string, string> params, vector<string> requiredParams);
    static string validateStringPossibleValues(string text, vector<string> possibleValues);
    static bool isNumber(const std::string& s);
    static string getPath(string text);
    static bool validateFileExtension(string path, string extension);
    static void createDirectoryForPath(string path);
    static void comment(string text);
    static vector<Structs::Partition> getPartitions(Structs::MBR disk);
    static string wrongParam(map<string, string> params, vector<string> requiredParams);
};

#endif