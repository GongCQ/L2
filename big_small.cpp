#include "big_small.h"
#include <iostream>
#include <cmath>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;
using std::cout;
using std::endl;
using std::range_error;
using std::min;
using std::max;
using std::fixed;
using boost::filesystem::ofstream;
using boost::lexical_cast;

const double AMOUNT_LIMIT = 9999999999999999;
const double DIV_ARR[] = {      0,   50000,  100000,   200000,       500000, 
                          1000000, 2000000, 5000000, 10000000, AMOUNT_LIMIT};
const unsigned int DIV_SIZE = 9;

unsigned int GetAmountInterval(double amount){
    for(int d = 0; d < DIV_SIZE; d++){
        if(amount >= DIV_ARR[d] && amount < DIV_ARR[d + 1]){
            return d;
        }
    }
    return DIV_SIZE - 1;
}

size_t BigSmall::RESERVED_SIZE;
string BigSmall::cutOffStr;
int BigSmall::cutOffHour;
int BigSmall::cutOffMinute; 
int BigSmall::dateCount;
vector<string> BigSmall::dateStrVec;
list<Symbol> BigSmall::symbolList;
vector<vector<vector<double> > > BigSmall::buyVolVec;
vector<vector<vector<double> > > BigSmall::sellVolVec;
vector<vector<vector<double> > > BigSmall::quitsVolVec;
vector<vector<vector<double> > > BigSmall::buyAmountVec;
vector<vector<vector<double> > > BigSmall::sellAmountVec; 
vector<vector<vector<double> > > BigSmall::quitsAmountVec;

const string BigSmall::GetHeaderLine(const int columnWide){
    format fmt("%4d");
    format fmts("%" + lexical_cast<string>(columnWide) + "d");
    string s = "symbol"; 
    for(int d = 0; d < DIV_SIZE; d++){
        string intervalBegin = (fmt % (DIV_ARR[d] / 10000)).str() + "w";
        string intervalEnd = (fmt % (DIV_ARR[d + 1] / 10000)).str() + "w";
        s += "," + (fmts % (intervalBegin + "~" + intervalEnd)).str();
    }
    return s;
}

void BigSmall::Init(size_t reservedSize, const string cutOff){
    RESERVED_SIZE = reservedSize;
    dateCount = -1;
    dateStrVec.resize(RESERVED_SIZE);
    cutOffStr = cutOff;
    cutOffHour = atoi(cutOff.substr(0, 2).c_str());
    cutOffMinute = atoi(cutOff.substr(2, 2).c_str());
    cout<<"BigSmall initialized, "<<endl
        <<"RESERVED_SIZE "<<RESERVED_SIZE<<endl
        <<"cutOffStr "<<cutOffStr<<endl 
        <<"cutOffHour "<<cutOffHour<<endl 
        <<"cutOffMinute "<<cutOffMinute<<endl;

    buyVolVec.resize(DIV_SIZE);
    sellVolVec.resize(DIV_SIZE);
    quitsVolVec.resize(DIV_SIZE);
    buyAmountVec.resize(DIV_SIZE);
    sellAmountVec.resize(DIV_SIZE);
    quitsAmountVec.resize(DIV_SIZE);
    for(int d = 0; d < DIV_SIZE; d++){
        buyVolVec.at(d).resize(MAX_SYMBOL);
        sellVolVec.at(d).resize(MAX_SYMBOL);
        quitsVolVec.at(d).resize(MAX_SYMBOL);
        buyAmountVec.at(d).resize(MAX_SYMBOL);
        sellAmountVec.at(d).resize(MAX_SYMBOL);
        quitsAmountVec.at(d).resize(MAX_SYMBOL);
    }

}

void BigSmall::Open(Market &market){
    dateCount++;
    if(dateCount >= RESERVED_SIZE){
        cout<<"max reserved size has reached, exit."<<endl;
        throw range_error("max reserved size has reached.");
    }
    dateStrVec.at(dateCount) = market.GetDateTimeStr();

    cout<<"open: "<<market.GetDateTimeStr()<<endl;
    symbolList.clear();
    market.GetSymbolList(symbolList);
    cout<<"stock number is "<<symbolList.size()<<endl;

    for(list<Symbol>::const_iterator symbol = symbolList.begin(); symbol != symbolList.end(); symbol++){
        for(int d = 0; d < DIV_SIZE; d++){
            buyVolVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
            sellVolVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
            quitsVolVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
            buyAmountVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
            sellAmountVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
            quitsAmountVec.at(d).at(*symbol).resize(RESERVED_SIZE, 0);
        }
    }
}

void BigSmall::Bar(Market &market){
    const string timeStr = market.GetDateTimeStr();
    int hour = atoi(timeStr.substr(9, 2).c_str());
    int minute = atoi(timeStr.substr(11, 2).c_str());
    int second = atoi(timeStr.substr(13, 2).c_str());
    cout<<"bar: "<<market.GetDateTimeStr()<<endl;

    for(list<Symbol>::const_iterator sym = symbolList.begin(); sym != symbolList.end(); sym++){
        Symbol symbol = *sym;         
        list<Order> bidDeque, offerDeque;
        list<Trans> transList;
        list<Order> orderList;
        market.GetOrderDeque(symbol, bidDeque, offerDeque);
        market.GetTransList(symbol, transList);
        market.GetOrderList(symbol, orderList);

        vector<double> divBuyVol = vector<double>(DIV_SIZE, 0);
        vector<double> divSellVol = vector<double>(DIV_SIZE, 0);
        vector<double> divQuitsVol = vector<double>(DIV_SIZE, 0);
        vector<double> divBuyAmount = vector<double>(DIV_SIZE, 0);
        vector<double> divSellAmount = vector<double>(DIV_SIZE, 0);
        vector<double> divQuitsAmount = vector<double>(DIV_SIZE, 0); 

        for(list<Trans>::const_iterator trans = transList.begin(); trans != transList.end(); trans++){
            const TransInfo *transInfo = trans->GetInfo();
            if(transInfo == nullptr){
                cout<<"a invalid trans "<<trans->ToStr()<<endl;
                continue;
            }
            Order bidOrder = market.GetOrder(transInfo->bidSeq);
            Order offerOrder = market.GetOrder(transInfo->offerSeq);

            if(transInfo->type == 'F' && bidOrder.IsValid() && offerOrder.IsValid()) {  // transact
                const OrderInfo *bidOrderInfo = bidOrder.GetInfo(); 
                const OrderInfo *offerOrderInfo = offerOrder.GetInfo();
                double price = double(transInfo->price) / 100;
                double volumn = transInfo->num;
                double amount = price * volumn;
                double bidPrice = double(bidOrderInfo->price) / 100;
                double offerPrice = double(offerOrderInfo->price) / 100;
                double bidVol = bidOrderInfo->num + bidOrderInfo->transNum + bidOrderInfo->cancelNum;
                double offerVol = offerOrderInfo->num + offerOrderInfo->transNum + offerOrderInfo->cancelNum; 
                double bidAmount = bidPrice * bidVol;
                double offerAmount = offerPrice * offerVol;
                unsigned int bidDivSeq = GetAmountInterval(bidAmount);
                unsigned int offerDivSeq = GetAmountInterval(offerAmount);

                if(100 * hour + minute <= 100 * cutOffHour + cutOffMinute){
                    buyVolVec.at(bidDivSeq).at(symbol).at(dateCount) += volumn;
                    sellVolVec.at(offerDivSeq).at(symbol).at(dateCount) += volumn;
                    buyAmountVec.at(bidDivSeq).at(symbol).at(dateCount) += amount;
                    sellAmountVec.at(offerDivSeq).at(symbol).at(dateCount) += amount;
                    if(bidDivSeq == offerDivSeq){
                        quitsVolVec.at(bidDivSeq).at(symbol).at(dateCount) += volumn;
                        quitsAmountVec.at(bidDivSeq).at(symbol).at(dateCount) += amount; 
                    }
                }
            }
        }
    }
}

void BigSmall::Close(Market &market){
    cout<<"close: "<<market.GetDateTimeStr()<<endl;
}

void BigSmall::FactorToFile(const string &path, int saveDays){
    const string fileNamePrefix = "factor_bstr_";
    int beginDateSeq = saveDays > 0 ? max(0, dateCount - saveDays + 1) : 0; 
    format symbolFmt("%06d");
    format valueFmt("%14d");
    for(int dateSeq = beginDateSeq; dateSeq <= dateCount; dateSeq++){
        const string dateStr = dateStrVec.at(dateSeq).substr(0, 8);
        const string headerLine = GetHeaderLine(14);
        ofstream buyVolFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "buyVol.csv");
        buyVolFile<<headerLine;
        ofstream sellVolFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "sellVol.csv");
        sellVolFile<<headerLine;
        ofstream quitsVolFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "quitsVol.csv");
        quitsVolFile<<headerLine;
        ofstream buyAmountFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "buyAmount.csv");
        buyAmountFile<<headerLine;
        ofstream sellAmountFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "sellAmount.csv");
        sellAmountFile<<headerLine;
        ofstream quitsAmountFile(path + fileNamePrefix + dateStr + "_" + cutOffStr + "quitsAmount.csv");
        quitsAmountFile<<headerLine;

        for(list<Symbol>::const_iterator symbol = symbolList.begin(); symbol != symbolList.end(); symbol++){
            const string symbolStr = Market::GetSymbolStr(*symbol);
            buyVolFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            sellVolFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            quitsVolFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            buyAmountFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            sellAmountFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            quitsAmountFile<<std::fixed<<endl<<std::setw(6)<<symbolStr;
            for(int div = 0; div < DIV_SIZE; div++){
                buyVolFile<<','<<std::setw(14)<<std::setprecision(0)<<buyVolVec.at(div).at(*symbol).at(dateSeq);
                sellVolFile<<','<<std::setw(14)<<std::setprecision(0)<<sellVolVec.at(div).at(*symbol).at(dateSeq);
                quitsVolFile<<','<<std::setw(14)<<std::setprecision(0)<<quitsVolVec.at(div).at(*symbol).at(dateSeq);
                buyAmountFile<<','<<std::setw(14)<<std::setprecision(0)<<buyAmountVec.at(div).at(*symbol).at(dateSeq);
                sellAmountFile<<','<<std::setw(14)<<std::setprecision(0)<<sellAmountVec.at(div).at(*symbol).at(dateSeq);
                quitsAmountFile<<','<<std::setw(14)<<std::setprecision(0)<<quitsAmountVec.at(div).at(*symbol).at(dateSeq);                
            }
        }
    }
}