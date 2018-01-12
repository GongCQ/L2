#include "market.h"
#include <iostream>
#include <boost/date_time.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::to_simple_string;

boost::filesystem::ofstream debugFile000001("./debugFile/output000001", ios::app);
boost::filesystem::ofstream debugFile300642("./debugFile/output300642", ios::app);
void BarMon(Market &market){
    string bidStr000001, offerStr000001;
    market.GetOrderDequeStr(1, bidStr000001, offerStr000001);
    debugFile000001<<bidStr000001<<endl<<offerStr000001<<endl;

    string bidStr300642, offerStr300642;
    market.GetOrderDequeStr(300642, bidStr300642, offerStr300642);
    debugFile300642<<bidStr300642<<endl<<offerStr300642<<endl;

    ptime nowPt = microsec_clock::local_time();
    cout<<"[" + to_simple_string(nowPt) + "] " + market.GetDateTimeStr()<<endl;
}

int main(){
    logFile.WriteLog("unknown", "INFO", "LAUNCH!");
    Market market("/home/gongcq/program/c++/L2data/", "20170608", "20170612");
    market.AddMon(BarMon);
    market.Launch();
    return 0;
}