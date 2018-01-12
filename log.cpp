#include "log.h"
#include <string>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::filesystem;

Log::Log(){
    path = "./log/"; 
    currentDateStr = "19700101";
    logFile = nullptr; 
}

Log::Log(const string &logPath){
    path = logPath;
    currentDateStr = "19700101";
    logFile = nullptr;
}

void Log::ChangeFile(const string &dateStr){
    if(logFile != nullptr){
        delete logFile;
    }
    currentDateStr = dateStr;
    logFile = new boost::filesystem::ofstream(path + dateStr, ios::app);
}

void Log::WriteLog(const string &marketTimeStr, const string &label, const string &message){
    if(logFile == nullptr)
        logFile = new boost::filesystem::ofstream(path + currentDateStr, ios::app);        
    const string nowStr = to_iso_string(microsec_clock::local_time());
    (*logFile) << nowStr << ',' << label << ',' << marketTimeStr << ',' << message <<endl;
}

Log::~Log(){
    if(logFile != nullptr){
        delete logFile;
        logFile = nullptr;
    }
}