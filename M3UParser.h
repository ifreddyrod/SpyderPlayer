#ifndef M3UPARSER_H
#define M3UPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

using namespace std;

struct Channel 
{
    string name;
    string id;
    string logo;
    string group;
    string url;
};

class M3UParser 
{
public:
    M3UParser() {};
    vector<Channel> ParseM3UFile(const string& filePath); 
    
private:
    string DecodeHTMLEntities(const string& input);
};

#endif