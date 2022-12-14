#ifndef SCANNER_H
#define SCANNER_H
#include <vector>
#include <string>
#include <map>

using namespace std;


class Scanner {
  public:
    Scanner();
    void start();
    void processData(string instruction, vector<string> tks);
    string getToken(string text);
    vector<string> split(string text, string textSplit);
    map<string, string> getParameters(vector<string> parameter);
    vector<string> splitTokens(string text);
    void funcionExec(vector<string> tokens);
    void exec(string path);
};

#endif