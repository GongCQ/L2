#include "market.h"
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using boost::lexical_cast;
using namespace boost::posix_time;
using namespace boost::gregorian; 
using namespace boost::filesystem;

// Stock --------------------------------------------------
Stock::Stock(Symbol symbol): bidDeque(symbol, true), offerDeque(symbol, false){
    this->symbol = symbol;
}

Stock::~Stock(){
    transList.clear();
}

void Stock::AddOrder(Order &order){
    const OrderInfo *orderInfo = order.GetInfo();
    if(orderInfo->direct == '1'){ // bid
        bidDeque.AddOrder(order);
    }
    else if(orderInfo->direct == '2'){ // offer
        offerDeque.AddOrder(order);
    }
    else{
        logFile.WriteLog("unknown_market_time", "WARNING", "invalid order direct: " + orderInfo->ToStr());
    }
}

void Stock::AddTrans(Trans trans){
    transList.push_back(trans);
}

void Stock::ClearTrans(){
    transList.clear();
}

const string Stock::GetBidDequeStr(){
    return bidDeque.ToStr();
}

const string Stock::GetOfferDequeStr(){
    return offerDeque.ToStr();
}

// Market -------------------------------------------------
Market::Market(string dataPath, string beginDateStr, string endDateStr){
    currentTime = 0;
    currentStr = GetDateTimeStr();
    this->dataPath = dataPath[dataPath.size() - 1] == '/' ? dataPath : (dataPath + "/"); 
    this->beginDateStr = beginDateStr;
    this->endDateStr = endDateStr;
    this->currentDateStr = to_iso_string(from_iso_string(beginDateStr + "T000000") - days(1)).substr(0, 8);;
    this->orders = nullptr;
    this->stocks = nullptr;
    this->channels = nullptr;
}

Market::~Market(){
    Clear();
}

void Market::Launch(){
    while(FetchDate()){
        while(FetchTime());
    }
}

bool Market::FetchDate(){
    Clear();
    while(currentDateStr < endDateStr){
        currentDateStr = to_iso_string(from_iso_string(currentDateStr + "T000000") + days(1)).substr(0, 8);
        tm tempTm = to_tm(from_iso_string(currentDateStr + "T091459"));
        currentTime = mktime(&tempTm);
        currentStr = GetDateTimeStr();
        path currentDatePath(dataPath + currentDateStr + "/");
        if(exists(currentDatePath) && is_directory(currentDatePath)){
            logFile.ChangeFile(currentDateStr);
            logFile.WriteLog(currentStr, "INFO", "launch for " + currentDateStr + " *********************");
            InitForNewDay();
            return true;
        }
    }
    return false;
}

bool Market::FetchTime(){
    tm *tempTm1 = localtime(&currentTime);
    int timeSeconds = ptime_from_tm(*tempTm1).time_of_day().total_seconds();
    if(timeSeconds == HOUR_SECONDS * 9 + MINUTE_SECONDS * 25) { // 09:25:00
        currentTime += MINUTE_SECONDS * 5; // fetch to 09:30:00
    }
    else if(timeSeconds == HOUR_SECONDS * 11 + MINUTE_SECONDS * 30) { // 11:30:00
        currentTime += MINUTE_SECONDS * 90; // fetch to 13:00:00
    }
    else if(timeSeconds == HOUR_SECONDS * 15){ // 15:00:00
        return false; // end of day
    }
    else{
        currentTime++;
        int d = 0;
    }
    currentStr = GetDateTimeStr();

    for(int cn = 0; cn < CHANNEL_NUM; cn++){ // for each channel
        channels[cn].LocFirst();
        Order order;
        while(channels[cn].GetOrder(currentTime, order)){ // get a new order
            if(order.IsValid()){
                const OrderInfo *orderInfo = order.GetInfo();
                orders[orderInfo->seq] = order;
                if(stocks[orderInfo->symbol] != nullptr){
                    stocks[orderInfo->symbol]->AddOrder(order);
                }
                else{
                    logFile.WriteLog(currentStr, "ERROR", "encounter a symbol which is not in current stocks");
                }
            }
        }
        Trans trans;
        while(channels[cn].GetTrans(currentTime, trans)){ // get a new trans
            if(trans.IsValid()){ 
                const TransInfo *transInfo = trans.GetInfo();
                if(transInfo->type == 'F'){  // transact
                    const OrderSeq bidSeq = transInfo->bidSeq;   
                    if(orders[bidSeq].IsValid()){
                        orders[bidSeq].Trans(transInfo->num);
                    }
                    else if(transInfo->symbol < 60000){
                        logFile.WriteLog(currentStr, "ERROR", "attempt to transact a invalid bid order" + 
                            string(",symbol=") + lexical_cast<string>(transInfo->symbol) + 
                            string(",orderSeq=") + lexical_cast<string>(bidSeq) + 
                            string(",transSeq=") + lexical_cast<string>(transInfo->seq));
                    }
                    const OrderSeq offerSeq = transInfo->offerSeq;
                    if(orders[offerSeq].IsValid()){
                        orders[offerSeq].Trans(transInfo->num);
                    }
                    else if(transInfo->symbol < 60000){
                        logFile.WriteLog(currentStr, "ERROR", "attempt to transact a invalid offer order" + 
                            string(",symbol=") + lexical_cast<string>(transInfo->symbol) + 
                            string(",orderSeq=") + lexical_cast<string>(offerSeq) + 
                            string(",transSeq=") + lexical_cast<string>(transInfo->seq));
                    }
                }
                else if(transInfo->type == '4') { // cancel
                    if(transInfo->cBidSeq > 0 && transInfo->cOfferSeq == 0) { // cancel bid
                        if(orders[transInfo->bidSeq].IsValid()){
                            orders[transInfo->bidSeq].Cancel(transInfo->num);
                        }
                        else if(transInfo->symbol < 60000){
                            logFile.WriteLog(currentStr, "ERROR", "attempt to cancel a invalid bid order" + 
                                string(",symbol=") + lexical_cast<string>(transInfo->symbol) + 
                                string(",orderSeq=") + lexical_cast<string>(transInfo->bidSeq) + 
                                string(",transSeq=") + lexical_cast<string>(transInfo->seq));
                        }
                    }
                    else if(transInfo->cBidSeq == 0 && transInfo->cOfferSeq > 0) { // cancel offer
                        if(orders[transInfo->offerSeq].IsValid()){
                            orders[transInfo->offerSeq].Cancel(transInfo->num);
                        }
                        else if(transInfo->symbol < 60000){
                            logFile.WriteLog(currentStr, "ERROR", "attempt to cancel a invalid offer order" + 
                                string(",symbol=") + lexical_cast<string>(transInfo->symbol) + 
                                string(",orderSeq=") + lexical_cast<string>(transInfo->offerSeq) + 
                                string(",transSeq=") + lexical_cast<string>(transInfo->seq));
                        }
                    }
                    else{ // unknown
                        logFile.WriteLog(currentStr, "ERROR", "unknown cancel type: bid or offer?" + 
                            string(",symbol=") + lexical_cast<string>(transInfo->symbol) + 
                            string(",transSeq=") + lexical_cast<string>(transInfo->seq));
                    }
                }
            }
        }
    }
    if(currentTime % BAR_INTERVEL == 0){
        for(list<BarMonitor>::iterator ite = monList.begin(); ite != monList.end(); ite++){
            (*ite)(*this);
        }
    }
    return true;
}

const string Market::GetDateTimeStr(){ 
    tm *tempTm = localtime(&currentTime);
    string str = to_simple_string(ptime_from_tm(*tempTm));
    return str;
}

void Market::GetOrderDequeStr(Symbol symbol, string &bidStr, string &offerStr){
    if(symbol >= MAX_SYMBOL){
        logFile.WriteLog(currentStr, "WARNING", 
            "attempt to get order deque string for a invalid symbol: " + lexical_cast<string>(symbol));
            return;
    }
    bidStr = GetDateTimeStr() + "    ";
    offerStr = GetDateTimeStr() + "    ";
    bidStr += stocks[symbol]->GetBidDequeStr();
    offerStr += stocks[symbol]->GetOfferDequeStr();
}

void Market::AddMon(BarMonitor mon){
    monList.push_back(mon);
}

void Market::OrgFile(){
    const string dateFolderPath = dataPath + currentDateStr + "/";
    const string szOrderPath = dateFolderPath + "szlv2order" + "/";
    const string szTransPath = dateFolderPath + "szlv2trans" + "/";
    const string shTransPath = dateFolderPath + "shlv2trans" + "/";
    set<string> szOrderSymbolSet;
    set<string> szTransSymbolSet;
    set<string> shTransSymbolSet;
    Market::GetStockFile(szOrderPath, szOrderSymbolSet);
    Market::GetStockFile(szTransPath, szTransSymbolSet);
    Market::GetStockFile(shTransPath, shTransSymbolSet); 
    unsigned int szFileSeq = 0;
    for(set<string>::iterator szo = szOrderSymbolSet.begin(); szo != szOrderSymbolSet.end(); szo++){
        unsigned int channelSeq = szFileSeq % CHANNEL_NUM;
        Symbol symbol = lexical_cast<Symbol>((*szo).substr(2, 6));
        channels[channelSeq].AddOrderStream(dataPath + currentDateStr + "/szlv2order/" + *szo, symbol);
        logFile.WriteLog(currentStr, "INFO", "add order file ./" + currentDateStr + "/szlv2order/" + *szo);
        if(szTransSymbolSet.find(*szo) != szTransSymbolSet.end()){
            channels[channelSeq].AddTransStream(dataPath + currentDateStr + "/szlv2trans/" + *szo, symbol);
            logFile.WriteLog(currentStr, "INFO", "add trans file ./" + currentDateStr + "/szlv2trans/" + *szo);
        }
        stocks[symbol] = new Stock(symbol);
        szFileSeq++;
    }
    unsigned int shFileSeq = 0;
    for(set<string>::iterator sht = shTransSymbolSet.begin(); sht != shTransSymbolSet.end(); sht++){
        unsigned int channelSeq = shFileSeq % CHANNEL_NUM;
        Symbol symbol = lexical_cast<Symbol>((*sht).substr(2, 6));
        channels[channelSeq].AddTransStream(dataPath + currentDateStr + "/shlv2trans/" + *sht, symbol);
        logFile.WriteLog(currentStr, "INFO", "add trans file ./" + currentDateStr + "/shlv2trans/" + *sht);
        stocks[symbol] = new Stock(symbol);
        shFileSeq++;
    }
}

void Market::InitForNewDay(){
    Clear();
    orders = new Order[MAX_ORDER_SEQ];
    stocks = new Stock*[MAX_SYMBOL];
    for(int s = 0; s < MAX_SYMBOL; s++){
        stocks[s] = nullptr;
    }
    channels = new Channel[CHANNEL_NUM];
    OrgFile();
}

void Market::Clear(){
    if(stocks != nullptr){
        for(int s = 0; s < MAX_SYMBOL; s++){
            if(stocks[s] != nullptr){
                delete stocks[s];
            }
        }
        delete[] stocks;
        stocks = nullptr;
    }
    if(channels != nullptr){
        delete[] channels;
        channels = nullptr; 
    }
    if(orders != nullptr){
        for(int i = 0; i < MAX_ORDER_SEQ; i++){
            orders[i].Remove(); 
        }
        delete[] orders;
        orders = nullptr;
    }
}

bool Market::IsStock(const string &symbol){        
	if (symbol.substr(0, 3) == string("000") ||
		symbol.substr(0, 3) == string("001") ||
		symbol.substr(0, 3) == string("002") ||
		symbol.substr(0, 3) == string("300") ||
		symbol.substr(0, 3) == string("600") ||
		symbol.substr(0, 3) == string("601") ||
		symbol.substr(0, 3) == string("603") )
		return true;
	else
		return false;
}

void Market::GetStockFile(const string pathStr, set<string> &fileNameSet){ 
    path director(pathStr);
    directory_iterator dirIteEnd;
	for(directory_iterator dirIte(director); dirIte != dirIteEnd; dirIte++)	{
		if(!is_directory(*dirIte)) {
			string fileName = (*dirIte).path().filename().string();
            if(Market::IsStock(fileName.substr(2, 6))){
    			fileNameSet.insert(fileName);
            }
		}
	}
}