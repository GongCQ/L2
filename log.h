#pragma once
#include <string>
#include <boost/filesystem.hpp>
using std::string;

class Log{
protected:
    string path;
    string currentDateStr;
    boost::filesystem::ofstream *logFile;

public:
    Log();
    Log(const string &logPath);
    void ChangeFile(const string &dateStr);
    void WriteLog(const string &marketTimeStr, const string &label, const string &message);
    ~Log();
};