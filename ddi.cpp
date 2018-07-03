#include "ddi.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <gsl/gsl_statistics_double.h>
using std::cout;
using std::endl;
using std::setw;
using std::setprecision;
using std::ios;
using std::pow;
using std::exception;
using std::out_of_range;
using std::swap;
using std::min;
using std::max;
using boost::filesystem::ofstream;
using boost::lexical_cast;

const long long BIG_AMOUNT = 500000000;
const double OUTLIER_RETURN_BASE = 0.11;

void isort(vector<double> &vec, vector<size_t> &ivec){
	size_t size = vec.size();
	bool noswap = false;
	while(!noswap){
		noswap = true;
		for(size_t s = 0; s + 1 < size; s++){
			if(vec[s] > vec[s + 1]){
				swap(vec[s], vec[s + 1]);
				swap(ivec[s], ivec[s + 1]);
				noswap = false;
			}
		}
	}	
}

vector<size_t> isort(vector<double> &vec){
	size_t size = vec.size();
	vector<size_t> ivec = vector<size_t>(size);
	for(size_t s = 0; s < size; s++){
		ivec[s] = s;
	}
	isort(vec, ivec);
	return ivec;
}

size_t DDI::RESERVED_SIZE; 
string DDI::cutOffStr;
int DDI::cutOffHour;
int DDI::cutOffMinute;
int DDI::dateCount;
vector<string> DDI::dateStrVec;
list<Symbol> DDI::symbolList;
vector<vector<long long> > DDI::openVec; 
vector<vector<long long> > DDI::closeVec;
vector<vector<long long> > DDI::limitUpVec;
vector<vector<long long> > DDI::bigTradeInVec;    // amount
vector<vector<long long> > DDI::bigTradeOutVec;   // amount
vector<vector<long long> > DDI::bigOrderInVec;    // number
vector<vector<long long> > DDI::bigOrderOutVec;   // number
vector<vector<long long> > DDI::totalOrderInVec;  // number
vector<vector<long long> > DDI::totalOrderOutVec; // number
vector<vector<long long> > DDI::bigCancelInVec;   // number
vector<vector<long long> > DDI::bigCancelOutVec;  // number
vector<vector<long long> > DDI::transTotalNumVec; // number
vector<vector<long long> > DDI::transTotalAmountVec; // amount
vector<vector<long long> > DDI::bigEatSmallAmountVec;
vector<vector<long long> > DDI::extraUpOrderAmountVec;
vector<vector<long long> > DDI::extraDownOrderAmountVec;
vector<vector<long long> > DDI::limitUpOrderAmountVec;
vector<vector<long long> > DDI::limitUpOrderVolVec;

vector<vector<vector<long long> > *> DDI::allVec;
vector<string> DDI::allName;

void DDI::Init(size_t reservedSize, const string &cutOff){
    RESERVED_SIZE = reservedSize;
    cutOffStr = cutOff;
    cutOffHour = atoi(cutOff.substr(0, 2).c_str());
    cutOffMinute = atoi(cutOff.substr(2, 2).c_str());
    cout<<"DDI initialized, "<<endl
        <<"RESERVED_SIZE "<<RESERVED_SIZE<<endl
        <<"cutOffStr "<<cutOffStr<<endl 
        <<"cutOffHour "<<cutOffHour<<endl 
        <<"cutOffMinute "<<cutOffMinute<<endl;

    dateCount = -1;
    dateStrVec.resize(RESERVED_SIZE);

    allVec.push_back(&openVec);
    allVec.push_back(&closeVec);
    allVec.push_back(&limitUpVec);
    allVec.push_back(&bigTradeInVec);
    allVec.push_back(&bigTradeOutVec);
    allVec.push_back(&bigOrderInVec);
    allVec.push_back(&bigOrderOutVec);
    allVec.push_back(&totalOrderInVec);
    allVec.push_back(&totalOrderOutVec);
    allVec.push_back(&bigCancelInVec);
    allVec.push_back(&bigCancelOutVec);
    allVec.push_back(&transTotalNumVec);
    allVec.push_back(&transTotalAmountVec);
    allVec.push_back(&bigEatSmallAmountVec);
    allVec.push_back(&extraUpOrderAmountVec);
    allVec.push_back(&extraDownOrderAmountVec);
    allVec.push_back(&limitUpOrderAmountVec);
    allVec.push_back(&limitUpOrderVolVec);

    allName.push_back("openVec");
    allName.push_back("closeVec");
    allName.push_back("limitUpVec");
    allName.push_back("bigTradeInVec");
    allName.push_back("bigTradeOutVec");
    allName.push_back("bigOrderInVec");
    allName.push_back("bigOrderOutVec");
    allName.push_back("totalOrderInVec");
    allName.push_back("totalOrderOutVec");
    allName.push_back("bigCancelInVec");
    allName.push_back("bigCancelOutVec");
    allName.push_back("transTotalNumVec");
    allName.push_back("transTotalAmountVec");
    allName.push_back("bigEatSmallAmountVec");
    allName.push_back("extraUpOrderAmountVec");
    allName.push_back("extraDownOrderAmountVec");
    allName.push_back("limitUpOrderAmountVec");
    allName.push_back("limitUpOrderVolVec");

    openVec.resize(MAX_SYMBOL);
    closeVec.resize(MAX_SYMBOL);
    limitUpVec.resize(MAX_SYMBOL);
    bigTradeInVec.resize(MAX_SYMBOL);
    bigTradeOutVec.resize(MAX_SYMBOL);
    bigOrderInVec.resize(MAX_SYMBOL);
    bigOrderOutVec.resize(MAX_SYMBOL);
    totalOrderInVec.resize(MAX_SYMBOL);
    totalOrderOutVec.resize(MAX_SYMBOL);
    bigCancelInVec.resize(MAX_SYMBOL);
    bigCancelOutVec.resize(MAX_SYMBOL);
    transTotalNumVec.resize(MAX_SYMBOL);
    transTotalAmountVec.resize(MAX_SYMBOL);
    bigEatSmallAmountVec.resize(MAX_SYMBOL);
    extraUpOrderAmountVec.resize(MAX_SYMBOL);
    extraDownOrderAmountVec.resize(MAX_SYMBOL);
    limitUpOrderAmountVec.resize(MAX_SYMBOL);
    limitUpOrderVolVec.resize(MAX_SYMBOL);
}

void DDI::Open(Market &market){
    dateCount++;
    dateStrVec.at(dateCount) = market.GetDateTimeStr();
    symbolList.clear();
    cout<<market.GetDateTimeStr()<<" open"<<endl;
    market.GetSymbolList(symbolList);    
    cout<<"stocks number "<<symbolList.size()<<endl;
    for(list<Symbol>::const_iterator symbol = symbolList.begin(); symbol != symbolList.end(); symbol++){
        for(vector<vector<vector<long long> > *>::iterator vecIte = allVec.begin(); vecIte != allVec.end(); vecIte++){
            vector<vector<long long> > &vec = **vecIte;
            vector<long long> &stockVec = vec.at(*symbol);
            // initialize size
            if(stockVec.size() == 0){
                stockVec.resize(RESERVED_SIZE);
                for(size_t i = 0; i < RESERVED_SIZE; i++){
                    stockVec.at(i) = 0;
                }
            }
            // // extend size
            // if(stockVec.size() < dateCount + 1){
            //     size_t oldSize = stockVec.size();
            //     stockVec.resize(dateCount + 1);
            //     for(size_t i = oldSize; i < dateCount; i++){
            //         stockVec.at(i) = 0;
            //     }
            // }
        }
    }
}

void DDI::Bar(Market &market){
    cout<<market.GetDateTimeStr()<<" bar"<<endl;
    const string timeStr = market.GetDateTimeStr();
    int hour = atoi(timeStr.substr(9, 2).c_str());
    int minute = atoi(timeStr.substr(11, 2).c_str());
    int second = atoi(timeStr.substr(13, 2).c_str());

    for(list<Symbol>::const_iterator sym = symbolList.begin(); 
            sym != symbolList.end(); sym++){   
        Symbol symbol = *sym;         
        list<Order> bidDeque, offerDeque;
        list<Trans> transList;
        list<Order> orderList;
        market.GetOrderDeque(symbol, bidDeque, offerDeque);
        market.GetTransList(symbol, transList);
        market.GetOrderList(symbol, orderList);
        long long preClose = dateCount >= 1 ? closeVec.at(symbol).at(dateCount - 1) : 0;

        long long bigTradeIn = 0;
        long long bigTradeOut = 0; 
        long long bigOrderIn = 0;
        long long bigOrderOut = 0;
        long long totalOrderIn = 0;
        long long totalOrderOut = 0;
        long long bigCancelIn = 0;
        long long bigCancelOut = 0;
        long long transTotalNum = 0;
        long long transTotalAmount = 0;
        long long bigEatSmallAmount = 0;
        long long extraUpOrderAmount = 0;
        long long extraDownOrderAmount = 0;
        long long limitUpOrderAmount = 0;
        long long limitUpOrderVol = 0;
        Price firstPrice = 0;
        Price lastPrice = 0;
        bool limited = true;
        bool existInitBuy = false;
        bool existInitSell = false;
        int transTimes = 0;
        double returnRate = 0;
        char upDownLimit = ' ';

        for(list<Order>::const_iterator order = orderList.begin(); order != orderList.end(); order++){
            const OrderInfo *info = order->GetInfo();
            long long totalNum = info->num + info->transNum + info->cancelNum;
            if(info->direct == '1'){
                totalOrderIn += totalNum;
                if(info->type == '2' && totalNum * info->price >= BIG_AMOUNT){
                    bigOrderIn += totalNum;
                }
                if(preClose > 0 && double(info->price) / double(preClose) - 1 >= 0.099){
                    limitUpOrderAmount += info->price * totalNum;
                    limitUpOrderVol += totalNum;
                }
            }
            else if(info->direct == '2'){
                totalOrderOut += totalNum;
                if(info->type == '2' && totalNum * info->price >= BIG_AMOUNT){
                    bigOrderOut += totalNum;
                }
            }
        }
        for(list<Trans>::const_iterator trans = transList.begin(); trans != transList.end(); trans++){
            const TransInfo *transInfo = trans->GetInfo();
            OrderSeq bidSeq = transInfo->bidSeq;
            OrderSeq offerSeq = transInfo->offerSeq;
            const OrderInfo *bidInfo = market.GetOrder(bidSeq).GetInfo();
            const OrderInfo *offerInfo = market.GetOrder(offerSeq).GetInfo();

            if(transInfo->type == '4' && bidInfo != nullptr) { // big cancel bid
                if((bidInfo->num + bidInfo->transNum + bidInfo->cancelNum) * bidInfo->price >= BIG_AMOUNT){
                    bigCancelIn += transInfo->num;
                }
            }
            if(transInfo->type == '4' && offerInfo != nullptr) { // big cancel offer
                if((offerInfo->num + offerInfo->transNum + offerInfo->cancelNum) * offerInfo->price >= BIG_AMOUNT){
                    bigCancelOut += transInfo->num;
                }
            }

            if(bidInfo == nullptr || offerInfo == nullptr){
                continue;
            }

            if(transInfo->type == 'F'){ // trade
                if(bidInfo == nullptr || offerInfo == nullptr){
                    continue;
                }
                transTimes += 1;
                lastPrice = transInfo->price;
                if(firstPrice == 0){
                    firstPrice = transInfo->price;
                }
                if(transInfo->price != firstPrice && transTimes >= 2){
                    limited = false;
                }
                if(bidInfo->cSeq > offerInfo->cSeq){
                    existInitBuy = true;
                }
                if(bidInfo->cSeq < offerInfo->cSeq){
                    existInitSell = true;
                }
                long long bidAmount = bidInfo != nullptr ? 
                            (bidInfo->price * (bidInfo->num + bidInfo->cancelNum + bidInfo->transNum)) : 0;
                long long offerAmount = offerInfo != nullptr ? 
                            (offerInfo->price * (offerInfo->num + offerInfo->cancelNum + offerInfo->transNum)) : 0;
                long long transAmount = transInfo->num * transInfo->price;

                if(bidInfo->type == '2' && bidAmount >= BIG_AMOUNT) { // limit order only
                    bigTradeIn += transInfo->price * transInfo->num;
                }
                if(offerInfo->type == '2' && offerAmount >= BIG_AMOUNT) { // limit order only
                    bigTradeOut += transInfo->price * transInfo->num;
                }

                transTotalNum += transInfo->num;
                transTotalAmount += transInfo->num * transInfo->price;

                if(bidInfo->type == '2' && offerInfo->type == '2' && bidAmount > BIG_AMOUNT && offerAmount < BIG_AMOUNT){
                    bigEatSmallAmount += transInfo->num * transInfo->price;
                }

                if(bidInfo->price >= transInfo->price * (1 + 0.03)){
                    extraUpOrderAmount += transAmount;
                }
                if(offerInfo->price <= transInfo->price * (1 - 0.03)){
                    extraDownOrderAmount += transAmount;
                }
            }
        }
        returnRate = transTimes >= 1 ? ((double)lastPrice / (double)firstPrice - 1) : 0;
        upDownLimit = (!existInitSell && limited) ? 'd' : (!existInitBuy && limited ? 'u' : ' '); 

        if(hour == 9 && minute == 25 && second == 0){ // open
            openVec.at(symbol).at(dateCount) = lastPrice;
        }
        if(hour == 15 && minute == 0 && second == 0){ // close
            closeVec.at(symbol).at(dateCount) = lastPrice;
            limitUpVec.at(symbol).at(dateCount) = (upDownLimit == 'u' ? 1 : 0);
        }
        if(100 * hour + minute <= 100 * cutOffHour + cutOffMinute){
            bigTradeInVec.at(symbol).at(dateCount) += bigTradeIn;
            bigTradeOutVec.at(symbol).at(dateCount) += bigTradeOut;
            bigOrderInVec.at(symbol).at(dateCount) += bigOrderIn;
            bigOrderOutVec.at(symbol).at(dateCount) += bigOrderOut;
            totalOrderInVec.at(symbol).at(dateCount) += totalOrderIn;
            totalOrderOutVec.at(symbol).at(dateCount) += totalOrderOut;
            bigCancelInVec.at(symbol).at(dateCount) += bigCancelIn;
            bigCancelOutVec.at(symbol).at(dateCount) += bigCancelOut;
            transTotalNumVec.at(symbol).at(dateCount) += transTotalNum;
            transTotalAmountVec.at(symbol).at(dateCount) += transTotalAmount;
            bigEatSmallAmountVec.at(symbol).at(dateCount) += bigEatSmallAmount;
            extraUpOrderAmountVec.at(symbol).at(dateCount) += extraUpOrderAmount;
            extraDownOrderAmountVec.at(symbol).at(dateCount) +=  extraDownOrderAmount;
            limitUpOrderAmountVec.at(symbol).at(dateCount) += limitUpOrderAmount;
            limitUpOrderVolVec.at(symbol).at(dateCount) += limitUpOrderVol;
        }
    }
}

void DDI::Close(Market &market){
    cout<<market.GetDateTimeStr()<<" close"<<endl;
}

void DDI::PrimaryFactorToFile(const string &path, int saveDays){
    int beginDateSeq = saveDays > 0 ? max(0, dateCount - saveDays + 1) : 0;
    for(int factorSeq = 0; factorSeq < allName.size(); factorSeq++){
        const string factorName = allName.at(factorSeq);
        for(int dateSeq = beginDateSeq; dateSeq <= dateCount; dateSeq++){
            const string dateStr = dateStrVec.at(dateSeq).substr(0, 8);
            ofstream file(path + "factor_base_" + dateStr + "_" + factorName + ".csv");
            for(Symbol symbol = 0; symbol < MAX_SYMBOL; symbol++){
                vector<long long> &record = allVec.at(factorSeq)->at(symbol);
                if(record.size() == RESERVED_SIZE){ // valid symbol
                    file<<Market::GetSymbolStr(symbol)<<","<<setw(14)<<record.at(dateSeq)<<endl;
                }
            }
            file.flush();
            file.close();
        }
    }
}

void DDI::DerivativeFactorToFile(const string &path, int saveDays){
    int beginDateSeq = saveDays > 0 ? max(0, dateCount - saveDays + 1) : 0;
    for(int f = 0; f < GetFactorNum(); f++){
        const string factorName = GetFactorName(f);
        for(int dateSeq = beginDateSeq; dateSeq <= dateCount; dateSeq++){
            const string dateStr = dateStrVec.at(dateSeq).substr(0, 8);
            ofstream file(path + "factor_deri_" + dateStr + "_" + factorName + ".csv");
            for(Symbol symbol = 0; symbol < MAX_SYMBOL; symbol++){
                double value = GetFactorValue(symbol, dateSeq, f);
                if(!isnan(value)){
                    file<<Market::GetSymbolStr(symbol)<<","
                        <<setw(14)<<lexical_cast<string>(value)<<","
                        <<limitUpVec.at(symbol).at(dateSeq)<<endl;
                }
            }
            file.flush();
            file.close();
        }
    }
}

void DDI::Summary(int returnDays, int factorId, int top){
    double outlierReturn = 0.11;
    vector<double> allRtnVec, rtnVec, topRtnVec;
    allRtnVec.reserve(RESERVED_SIZE * 3000);
    rtnVec.reserve(RESERVED_SIZE * 3000);
    topRtnVec.reserve(RESERVED_SIZE * 300);
    vector<vector<long long> > infoVec, topInfoVec;
    infoVec.reserve(RESERVED_SIZE * 3000);
    topInfoVec.reserve(RESERVED_SIZE * 300);
    for(int dateSeq = 0; dateSeq < RESERVED_SIZE - returnDays; dateSeq++){
        int dateInt = atoi(dateStrVec.at(dateSeq).substr(0, 8).c_str());
        vector<double> dateSymVec, dateRtnVec, dateValVec;
        dateSymVec.reserve(300);
        dateRtnVec.reserve(300);
        dateValVec.reserve(300);
        
        for(Symbol symbol = 0; symbol < MAX_SYMBOL; symbol++){
            vector<long long> &closeRecord = closeVec.at(symbol);
            if(closeRecord.size() != RESERVED_SIZE){
                continue;
            }
            if(closeRecord.at(dateSeq) == 0 || closeRecord.at(dateSeq + returnDays) == 0){
                continue;
            }
            double rtn = double(closeRecord.at(dateSeq + returnDays)) / double(closeRecord.at(dateSeq)) - 1;
            allRtnVec.push_back(rtn);
            double value = -9999;
            if(StockPicking(factorId, symbol, dateSeq, value)){ 
                if(rtn > pow(1 + outlierReturn, (double)returnDays) - 1 || rtn < pow(1 - outlierReturn, (double)returnDays) - 1){
                    cout<<"WARNING: unexpected return rate "<<rtn<<", "
                        <<", symbol "<<symbol<<", date "<<dateInt<<", days "<<returnDays
                        <<", beginPrice "<<closeRecord.at(dateSeq)<<", endPrice"<<closeRecord.at(dateSeq + returnDays)<<endl;
                    continue;
                }
                rtnVec.push_back(rtn);
                vector<long long> info(4);
                info.at(0) = symbol;
                info.at(1) = dateInt;
                info.at(2) = (long long)(rtn * 10000);
                info.at(3) = (long long)(value * 10000);
                infoVec.push_back(info);

                dateSymVec.push_back(symbol);
                dateRtnVec.push_back((long long)(rtn * 10000));
                dateValVec.push_back((long long)(value * 10000));
            }
        }

        vector<size_t> ivec = isort(dateValVec);
        for(int k = 0; k < min((size_t)top, dateValVec.size()); k++){
            vector<long long> topInfo(4);
            int index = dateSymVec.size() - 1 - k;
            topInfo.at(0) = dateSymVec.at(index);
            topInfo.at(1) = dateInt;
            topInfo.at(2) = dateRtnVec.at(index);
            topInfo.at(3) = dateValVec.at(index);
            topInfoVec.push_back(topInfo);
            topRtnVec.push_back(double(dateRtnVec.at(index)) / 10000);
        }
    }

    const string factorName = GetFactorName(factorId);
    const double cutOff = GetFactorCutOff(factorId);
    string fileName = "./summary/stat_" + lexical_cast<string>(factorId) + "_" + factorName + "_" + 
                      lexical_cast<string>(returnDays) + "_" + lexical_cast<string>(cutOff) + ".csv";
    string topFileName = "./summary/stat_top" + lexical_cast<string>(top) + "_" + lexical_cast<string>(factorId) + "_" + factorName + "_" + 
                         lexical_cast<string>(returnDays) + "_" + lexical_cast<string>(cutOff) + ".csv";
    vector<string> fileNameOptionVec(2);
    vector<vector<double>* > rtnOptionVec(2);
    vector<vector<vector<long long> >* > infoOptionVec(2);
    fileNameOptionVec.at(0) = fileName;
    fileNameOptionVec.at(1) = topFileName;
    rtnOptionVec.at(0) = &rtnVec;
    rtnOptionVec.at(1) = &topRtnVec;
    infoOptionVec.at(0) = &infoVec;
    infoOptionVec.at(1) = &topInfoVec;
    
    for(int i = 0; i < 2; i++){
        string &fileName = fileNameOptionVec.at(i);
        vector<double> &rtnVec = *(rtnOptionVec.at(i));
        vector<vector<long long> > &infoVec = *(infoOptionVec.at(i));

        ofstream file(fileName);
        if(rtnVec.size() < 1){
            file<<"sample is not enough!"<<endl;
            continue;
        }
        double returnMean = gsl_stats_mean(&(rtnVec.at(0)), 1, rtnVec.size());
        double allReturnMean = gsl_stats_mean(&(allRtnVec.at(0)), 1, allRtnVec.size());
        double returnStd = gsl_stats_absdev(&(rtnVec.at(0)), 1, rtnVec.size());
        double allReturnStd = gsl_stats_absdev(&(allRtnVec.at(0)), 1, allRtnVec.size());
        
        file<<std::setw(8)<<"strategy"<<", "
            <<std::setw(8)<<std::setiosflags(ios::fixed)<<"number"<<", "
            <<std::setw(8)<<std::setiosflags(ios::fixed)<<"mean"<<", "
            <<std::setw(8)<<std::setiosflags(ios::fixed)<<"std"<<endl;
        file<<std::setw(8)<<"ddi"<<", "
            <<std::setw(8)<<std::setiosflags(ios::fixed)<<rtnVec.size()<<", "
            <<std::setw(8)<<std::setprecision(4)<<std::setiosflags(ios::fixed)<<returnMean<<", "
            <<std::setw(8)<<std::setprecision(4)<<std::setiosflags(ios::fixed)<<returnStd<<endl;
        file<<std::setw(8)<<"all"<<", "
            <<std::setw(8)<<std::setiosflags(ios::fixed)<<allRtnVec.size()<<", "
            <<std::setw(8)<<std::setprecision(4)<<std::setiosflags(ios::fixed)<<allReturnMean<<", "
            <<std::setw(8)<<std::setprecision(4)<<std::setiosflags(ios::fixed)<<allReturnStd<<endl;
        file<<endl;

        file<<setw(6)<<std::setiosflags(ios::fixed)<<"symbol"<<",   "
            <<setw(8)<<std::setiosflags(ios::fixed)<<"date"<<", "
            <<setw(8)<<std::setiosflags(ios::fixed)<<"rtn"<<",  "
            <<setw(14)<<std::setiosflags(ios::fixed)<<"value"<<",  "
            <<setw(8)<<std::setiosflags(ios::fixed)<<"days"<<endl;
        for(int j = 0; j < infoVec.size(); j++){
            file<<setw(6)<<std::setfill('0')<<std::setiosflags(ios::fixed)<<infoVec.at(j).at(0)<<",   "
                <<setw(8)<<std::setfill(' ')<<std::setiosflags(ios::fixed)<<infoVec.at(j).at(1)<<", "
                <<setw(8)<<std::setfill(' ')<<std::setiosflags(ios::fixed)<<std::setprecision(2)<<(infoVec.at(j).at(2)) / 100.0<<", "
                <<setw(14)<<std::setfill(' ')<<std::setiosflags(ios::fixed)<<std::setprecision(4)<<(infoVec.at(j).at(3)) / 10000.0<<", "
                <<setw(8)<<std::setfill(' ')<<std::setiosflags(ios::fixed)<<returnDays<<endl;
        }
        file<<endl;
        file.flush();
        file.close();
    }
}

bool DDI::StockPicking(const int factorId, const Symbol symbol, const int dateSeq, double &value){
    bool qualified = true;
    vector<long long> &limitUpRecord = limitUpVec.at(symbol);
    if(limitUpRecord.at(dateSeq) != 0){
        qualified = false;
        return qualified;
    }
  
    value = GetFactorValue(symbol, dateSeq, factorId);
    qualified = (!isnan(value) && value >= GetFactorCutOff(factorId));

    return qualified;
}

const int DDI::GetFactorNum() {
    return 20;
}

const string DDI::GetFactorName(int factorId) {
    static string factorNameArr[] = 
           {cutOffStr + "委买总量比委卖总量", 
            cutOffStr + "大单委买总量比大单委卖总量",
            cutOffStr + "大单买成交额占总成交量比",
            cutOffStr + "大单卖成交额占总成交额比",
            cutOffStr + "大单撤买量占大单委买量比",
            cutOffStr + "大单撤卖量占大单委卖量比",
            cutOffStr + "涨停价委买量占总委买量比",
            cutOffStr + "大单吃小单成交额占总成交额比",
            cutOffStr + "异常溢价买入成交金额占总成交金额比",
            cutOffStr + "异常溢价卖出成交金额占总成交金额比",
            cutOffStr + "大单委买占总委买量比",
            cutOffStr + "大单委卖占总委卖量比",
            cutOffStr + "大单净流入金额",
            cutOffStr + "大买单成交金额",
            cutOffStr + "大卖单成交金额",
            cutOffStr + "大单吃小单成交金额",
            cutOffStr + "异常溢价买入成交金额",
            cutOffStr + "异常溢价卖出成交金额",
            cutOffStr + "异常溢价成交净额",
            cutOffStr + "涨停价委托金额"};   
    return factorNameArr[factorId];
}

const double DDI::GetFactorCutOff(int factorId){
    static double factorCutOffArr[] = 
    {1.1, 
     1.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     0.1,
     30000000,
     30000000,
     30000000,
     30000000,
     30000000,
     30000000,
     30000000,
     30000000};
     return factorCutOffArr[factorId];
}

const double DDI::GetFactorValue(Symbol symbol, int dateSeq, int factorId) {
    double value = nan("");
    switch(factorId){
        case 0:{
            value = 
                (allVec.at(7)->at(symbol).size() == RESERVED_SIZE && 
                    allVec.at(8)->at(symbol).size() == RESERVED_SIZE && 
                    allVec.at(8)->at(symbol).at(dateSeq) != 0) 
                ?
                (double(allVec.at(7)->at(symbol).at(dateSeq)) / double(allVec.at(8)->at(symbol).at(dateSeq))) 
                : 
                nan("");
            break;
        }
        case 1: {
            value = 
                (allVec.at(5)->at(symbol).size() == RESERVED_SIZE && 
                    allVec.at(6)->at(symbol).size() == RESERVED_SIZE && 
                    allVec.at(6)->at(symbol).at(dateSeq) != 0) 
                ?
                (double(allVec.at(5)->at(symbol).at(dateSeq)) / double(allVec.at(6)->at(symbol).at(dateSeq))) 
                : 
                nan("");
            break;
        }
        case 2: {
            value = 
                (allVec.at(3)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(3)->at(symbol).at(dateSeq)) / double(allVec.at(12)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 3: {
            value = 
                (allVec.at(4)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(4)->at(symbol).at(dateSeq)) / double(allVec.at(12)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 4: {
            value = 
                (allVec.at(9)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(5)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(5)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(9)->at(symbol).at(dateSeq)) / double(allVec.at(5)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 5: {
            value = 
                (allVec.at(10)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(6)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(6)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(10)->at(symbol).at(dateSeq)) / double(allVec.at(6)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 6: {
            value = 
                (allVec.at(17)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(7)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(7)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(17)->at(symbol).at(dateSeq)) / double(allVec.at(7)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 7: {
            value = 
                (allVec.at(13)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(13)->at(symbol).at(dateSeq)) / double(allVec.at(12)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 8: {
            value = 
                (allVec.at(14)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(14)->at(symbol).at(dateSeq)) / double(allVec.at(12)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 9: {
            value = 
                (allVec.at(15)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(12)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(15)->at(symbol).at(dateSeq)) / double(allVec.at(12)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 10: {
            value = 
                (allVec.at(5)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(7)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(7)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(5)->at(symbol).at(dateSeq)) / double(allVec.at(7)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 11: {
            value = 
                (allVec.at(6)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(8)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(8)->at(symbol).at(dateSeq) != 0)
                ?
                (double(allVec.at(6)->at(symbol).at(dateSeq)) / double(allVec.at(8)->at(symbol).at(dateSeq)))
                :
                nan("");
            break;
        }
        case 12: {
            value = 
                (allVec.at(3)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(4)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(3)->at(symbol).at(dateSeq) - allVec.at(4)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 13: {
            value = 
                (allVec.at(3)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(3)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 14: {
            value = 
                (allVec.at(4)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(4)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 15: {
            value = 
                (allVec.at(13)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(13)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 16: {
            value = 
                (allVec.at(14)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(14)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 17: {
            value = 
                (allVec.at(15)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(15)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 18: {
            value = 
                (allVec.at(14)->at(symbol).size() == RESERVED_SIZE && 
                allVec.at(15)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(14)->at(symbol).at(dateSeq) - allVec.at(15)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        case 19: {
            value = 
                (allVec.at(16)->at(symbol).size() == RESERVED_SIZE )
                ?
                (double(allVec.at(16)->at(symbol).at(dateSeq)) / 100)
                :
                nan("");
            break;
        }
        default:{
            cout<<"WARNING: unknown option "<<factorId<<endl;
        }
    }
    return value;
}