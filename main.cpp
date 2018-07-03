#include "market.h"
#include "ddi.h"
#include "ddi_min5.h"
#include "big_small.h"
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::posix_time::microsec_clock;
using boost::posix_time::to_simple_string;
using boost::posix_time::from_iso_string;
using boost::gregorian::days;

int main(int argc, char **argv){
    string helpInfo = "parameter example: -dataRoot /home/gongcq/data/L2/ -targetPath ./summary/ -backDays 1 -end 20180426 -saveDays 1 -reservedSize 90 -cutOff 1500 -begin 20180301";
    string dataRoot = "", targetPath = "./summary/";
    string begin = "", end = "";
    int saveDays = 0, backDays = 1;
    size_t reservedSize = 90;
    string cutOff = "1500";
    string freq = "day";
    for(int p = 1; p < argc; p += 2){
        if(strcmp(argv[p], "-dataRoot") == 0){
            dataRoot = argv[p + 1];
        }
        else if(strcmp(argv[p], "-targetPath") == 0){
            targetPath = argv[p + 1];
        }
        else if(strcmp(argv[p], "-begin") == 0){
            begin = argv[p + 1];
        }
        else if(strcmp(argv[p], "-end") == 0){
            end = argv[p + 1];
        }
        else if(strcmp(argv[p], "-saveDays") == 0){
            saveDays = atoi(argv[p + 1]);
        }
        else if(strcmp(argv[p], "-backDays") == 0){
            backDays = atoi(argv[p + 1]);
        }
        else if(strcmp(argv[p], "-reservedSize") == 0){
            reservedSize = atoi(argv[p + 1]);
        }
        else if(strcmp(argv[p], "-cutOff") == 0){
            cutOff = argv[p + 1];
        }
        else if(strcmp(argv[p], "-freq") == 0){
            freq = argv[p + 1];
        }
    }

    cout<<"======== parameter ========"<<endl;
    cout<<"dataRoot    : "<<dataRoot<<endl;
    cout<<"targetPath  : "<<targetPath<<endl;
    cout<<"begin       : "<<begin<<endl;
    cout<<"end         : "<<end<<endl;
    cout<<"saveDays    : "<<saveDays<<endl;
    cout<<"backDays    : "<<backDays<<endl;
    cout<<"reservedSize: "<<reservedSize<<endl;
    cout<<"cutOff      : "<<cutOff<<endl;
    cout<<"freq        : "<<freq<<endl;
    cout<<"==========================="<<endl;

    if(strcmp(dataRoot.c_str(), "") == 0 || strcmp(end.c_str(), "") == 0){
        cout<<"ERROR: parameter 'dataRoot' and 'end' is necessary."<<endl;
        return 1;
    }

    if(strcmp(begin.c_str(), "") == 0){
        cout<<"parameter 'begin' is not specified, using backDays to search begin date instead."<<endl;
        int backCount = 0;
        const int maxSearch = 9999;
        begin = end;
        int back = 1;
        for( ; backCount < backDays && back <= maxSearch; back++){
            string backDateStr = to_iso_string(from_iso_string(end + "T000000") - days(back)).substr(0, 8);
            string backDatePath = dataRoot + backDateStr;
            if(exists(backDatePath) && is_directory(backDatePath)){
                cout<<"got a back date "<<backDateStr<<endl;
                begin = backDateStr;
                backCount++;
            }
        }
        if(back == maxSearch){
            cout<<"WARNING: max search time reached."<<endl;
        }
        cout<<"searching back date completed, begin date is "<<begin<<", back count is "<<backCount<<endl;
    }

    logFile.WriteLog("unknown", "INFO", "LAUNCH!");
    set<Symbol> symbolSet;
    set<string> sourceSet;
    sourceSet.insert("szOrder");
    sourceSet.insert("szTrans");

    if(freq == "day"){

        DDI::Init(reservedSize, cutOff);

        Market market(dataRoot, begin, end, symbolSet, sourceSet, 300);
        market.AddBarMon(DDI::Bar);
        market.AddOpenMon(DDI::Open);
        market.AddCloseMon(DDI::Close);
        market.Launch();

        DDI::PrimaryFactorToFile(targetPath, saveDays);
        DDI::DerivativeFactorToFile(targetPath, saveDays);

        // int returnDaysArr[] = {1, 2, 3, 5, 10};
        // for(int f = 0; f < DDI::GetFactorNum(); f++){
        //     for(int d = 0; d < sizeof(returnDaysArr) / sizeof(returnDaysArr[0]); d++){
        //         DDI::Summary(returnDaysArr[d], f, 10);
        //     }
        // }
    }
    else if(freq == "min5"){
        DDI_MIN5::Init(reservedSize);

        Market market(dataRoot, begin, end, symbolSet, sourceSet, 300);
        market.AddBarMon(DDI_MIN5::Bar);
        market.AddOpenMon(DDI_MIN5::Open);
        market.AddCloseMon(DDI_MIN5::Close);
        market.Launch();

        DDI_MIN5::FactorToFile(targetPath, saveDays);
    }
    else if(freq == "bstr"){
        BigSmall::Init(reservedSize, cutOff);

        Market market(dataRoot, begin, end, symbolSet, sourceSet, 300);
        market.AddBarMon(BigSmall::Bar);
        market.AddOpenMon(BigSmall::Open);
        market.AddCloseMon(BigSmall::Close);
        market.Launch();

        BigSmall::FactorToFile(targetPath, saveDays);

    }
    else{
        cout<<"unknown freq, exited. "<<endl;
        return 1;
    }

    return 0;
}