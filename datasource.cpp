#include "public.h"
#include "datasource.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>  
#include <string>
#include <vector>
using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::posix_time;

OrderSeq ChannelSeq(int channel, OrderSeq seq)
{
	if (channel <= 6 && channel >= 1)
		return 10 * seq + channel - 1;
	else if (channel >= 2011 && channel <= 2014)
		return 10 * seq + channel - 2005;
	else{
		return INVALID_ORDER_SEQ;
    }

}

// OrderInfo ----------------------------------------------
const string OrderInfo::ToStr() const{
    string str("");
    tm *tempTm = localtime(&dt);
    str += "sym=" + lexical_cast<string>(symbol) + ",";
    str += "dt=" + to_simple_string(ptime_from_tm(*tempTm)) + ",";
    str += "ch=" + lexical_cast<string>(channel) + ",";
    str += "seq=" + lexical_cast<string>(cSeq) + ",";
    str += "p=" + lexical_cast<string>(price) + ",";
    str += "n=" + lexical_cast<string>(num) + ",";
    str += "d=" + lexical_cast<string>(direct) + ",";
    str += "t=" + lexical_cast<string>(type) + ",";
    str += "tn=" + lexical_cast<string>(transNum) + ",";
    str += "cn=" + lexical_cast<string>(cancelNum);
    return str;
}

// TransInfo ----------------------------------------------
const string TransInfo::ToStr() const{
    string str("");
    tm *tempTm = localtime(&dt);
    str += "sym=" + lexical_cast<string>(symbol) + ",";
    str += "dt=" + to_simple_string(ptime_from_tm(*tempTm)) + ",";
    str += "ch=" + lexical_cast<string>(channel) + ",";
    str += "seq=" + lexical_cast<string>(cSeq) + ",";
    str += "bseq=" + lexical_cast<string>(cBidSeq) + ",";
    str += "oseq=" + lexical_cast<string>(cOfferSeq) + ",";
    str += "p=" + lexical_cast<string>(price) + ",";
    str += "n=" + lexical_cast<string>(num) + ",";
    str += "t=" + lexical_cast<string>(type);
    return str;
}	

// Order --------------------------------------------------
void* Order::operator new(size_t s) { return nullptr;} 
void Order::operator delete(void* s) {}
Order* Order::operator &() { return nullptr;} 

Order::Order(){
    info = nullptr; 
}

Order::Order(Symbol symbol, time_t dt){
    info = new OrderInfo();
    info->symbol = symbol;
    info->dt = dt;
    info->channel = 0;
    info->seq = INVALID_ORDER_SEQ;
    info->cSeq = INVALID_ORDER_SEQ;
    info->price = 0;  
    info->num = 0;
    info->direct = ' ';
    info->type = ' ';
    info->transNum = 0;
    info->cancelNum = 0;
    info->refCount = 1;
    info->prior = Order();
    info->next = Order(); 
}

Order::Order(Symbol symbol, time_t dt, int channel, OrderSeq seq, float price, unsigned int num, char direct, char type){
    info = new OrderInfo();
    info->symbol = symbol;
    info->dt = dt;
    info->channel = channel;
    info->seq = ChannelSeq(channel, seq);
    info->cSeq = seq;
    info->price = int(round(price * 100));  //委托价格在内部会被乘以100并转换成整数，防止浮点数在计算中的误差
    info->num = num;
    info->direct = direct;
    info->type = type;
    info->transNum = 0;
    info->cancelNum = 0;
    info->refCount = 1;
    if(info->seq == INVALID_ORDER_SEQ){
        logFile.WriteLog("unknown", "ERROR", "got an order with invalid seq " + ToStr());
    }
}

Order::Order(const Order &order){
    info = order.info;
    if (info != nullptr) {
        info->refCount++;
    }
}

Order& Order::operator=(const Order& order){
    if (info == order.info){
        return *this; 
    }
    if (!IsEmpty()){
        info->refCount--;
        if (info->refCount == 0){
            delete info;
        }
    }    
    info = order.info;  
    if (!order.IsEmpty()){
        order.info->refCount++;
    }
    return *this;     
}

bool Order::operator==(const Order& order) const {
    return info == order.info;
}

bool Order::operator!=(const Order& order) const {
    return info != order.info;
}

Order::~Order(){
    if (!IsEmpty()){
        info->refCount--;
        if (info->refCount == 0){
            Remove();
            delete info;
        }
    }
}

int Order::Cancel(int num){
    if (IsEmpty()){
        return 0;
    }
    else{
        int cNum = info->num > num ? num : info->num;
        info->num -= cNum;
        info->cancelNum += cNum;
        int temp = info->num;
        if (info->num == 0)
            Remove(); 
        return temp;
    }
}

int Order::Trans(int num)	{
    if (IsEmpty()){
        return 0;
    }
    else{
        int tNum = info->num > num ? num : info->num;
        info->num -= tNum;
        info->transNum += tNum;
        int temp = info->num;
        if (info->num == 0)
            Remove();
        return temp;
    }
}

void Order::Remove(){
    if (!IsEmpty()){
        if (!(info->prior.IsEmpty())){
            info->prior.SetNext(info->next);
        }
        if (!(info->next.IsEmpty())){
            info->next.SetPrior(info->prior);
        }
        SetPrior(Order()); 
        SetNext(Order());   
    }
}

const string Order::ToStr() const{
    return info == nullptr ? "[null order info pointer]" : info->ToStr();
}

const OrderInfo* Order::GetInfo() const{
    return info;
}

bool Order::IsEmpty() const{
    return info == nullptr;
}

bool Order::IsValid() const{
    return info == nullptr ? false : (info->seq != INVALID_ORDER_SEQ);
}

bool Order::IsEqual(const Order& order) const {
    return info == order.info;
}

Order& Order::GetNext() const{
    return info->next;
}

Order& Order::GetPrior() const{
    return info->prior;
}

void Order::SetNext(Order order){
    info->next = order;
}

void Order::SetPrior(Order order){
    info->prior = order;
}

// Trans --------------------------------------------------
void* Trans::operator new(size_t s){ return nullptr;}
void Trans::operator delete(void* s) {}
Trans* Trans::operator &(){ return nullptr;}

Trans::Trans(){
    info = nullptr;
}

Trans::Trans(Symbol symbol, time_t dt){
    info = new TransInfo();
    info->symbol = symbol;
    info->dt = dt;
    info->channel = 0;
    info->seq = INVALID_TRANS_SEQ;
    info->cSeq = INVALID_TRANS_SEQ;
    info->bidSeq = INVALID_ORDER_SEQ;
    info->cBidSeq = INVALID_ORDER_SEQ;
    info->offerSeq = INVALID_ORDER_SEQ;
    info->cOfferSeq = INVALID_ORDER_SEQ;
    info->price = INVALID_PRICE;
    info->num = 0;
    info->type = ' ';
    info->refCount = 1;
}

Trans::Trans(Symbol symbol, time_t dt, int channel, TransSeq seq, OrderSeq bidSeq, OrderSeq offerSeq, float price, unsigned int num, char type){
    info = new TransInfo();
    info->symbol = symbol;
    info->dt = dt;
    info->channel = channel;
    info->seq = ChannelSeq(channel, seq);
    info->cSeq = seq;
    info->bidSeq = ChannelSeq(channel, bidSeq);
    info->cBidSeq = bidSeq;
    info->offerSeq = ChannelSeq(channel, offerSeq);
    info->cOfferSeq = offerSeq;
    info->price = int(round(price * 100));
    info->num = num;
    info->type = type;
    info->refCount = 1;
    if(info->seq == INVALID_TRANS_SEQ){
        logFile.WriteLog("unknown", "ERROR", "got an trans with invalid seq " + ToStr());
    }
}

Trans::Trans(const Trans &trans){
    info = trans.info;
    if (info != nullptr){
        info->refCount++;
    }
}

Trans& Trans::operator=(const Trans& trans){
    if (info != nullptr){
        info->refCount--;
        if (info->refCount == 0){
            delete info;
        }
    }
    info = trans.info;
    if (info != nullptr){
        trans.info->refCount++;
    }
    return *this;
}

Trans::~Trans(){
    if (!IsEmpty()) {
        info->refCount--;
        if (info->refCount == 0){
            delete info;
        }
    }
}

const string Trans::ToStr() const{
    return info == nullptr ? "[null trans info pointer]" : info->ToStr();
}

const TransInfo* Trans::GetInfo() const {
    return info;
}

bool Trans::IsEmpty(){
    return info == nullptr;
}

bool Trans::IsValid(){
    return info == nullptr ? false : (info->seq != INVALID_TRANS_SEQ);
}

// OrderStream --------------------------------------------
OrderStream::OrderStream(){
    this->symbol = INVALID_SYMBOL;
}

OrderStream::~OrderStream(){
    orderFile.close();
}

void OrderStream::Init(Symbol symbol, const string &path){
    this->symbol = symbol;
    this->path = path;
    orderFile.open(path);
}

Order OrderStream::StrToOrder(const string &str){
    char* last = nullptr;
    char* pArr[11];
    char* p = strtok_r((char*)(str.data()), ",", &last);
    int c = 0;
    while (p != NULL){
        pArr[c] = p;
        p = strtok_r(NULL, ",", &last);
        c++;
        // if (c > 7){
        //     break;
        // }
    }
    int swift = c > 9 ? 1 : 0;
    const string dtStr = str.substr(0, 4) + "-" + str.substr(4, 2) + "-" + str.substr(6, 2) + " " + string(pArr[1]);
    tm dtTm = to_tm(time_from_string(dtStr));
    time_t dt = mktime(&dtTm);
    int channel = atoi(pArr[2]);
    OrderSeq seq = atoi(pArr[3]);
    float price = atof(pArr[4 + swift]);
    int num = atoi(pArr[5 + swift]);;
    char direct = pArr[6 + swift][0];
    char type = pArr[7 + swift][0];
    Order order(symbol, dt, channel, seq, price, num, direct, type);
    return order;
}

bool OrderStream::GetOrder(Order& order){
    if(!orderFile.eof()){
        string orderStr;
        getline(orderFile, orderStr);
        if(orderStr.size() <= 1){
            return false;
        }
        order = StrToOrder(orderStr);
        return true;
    }
    else{
        return false;
    }
}


// TransStream --------------------------------------------
TransStream::TransStream(){
    this->symbol = INVALID_SYMBOL;
}

TransStream::~TransStream(){
    transFile.close();
}

void TransStream::Init(Symbol symbol, const string &path){
    this->symbol = symbol;
    this->path = path;
    transFile.open(path);
}

Trans TransStream::StrToTrans(const string &str){
    char* last = nullptr;
    char* pArr[12];
    char* p = strtok_r((char*)(str.data()), ",", &last);
    int c = 0;
    while (p != NULL){
        pArr[c] = p;
        p = strtok_r(NULL, ",", &last);
        c++;
        // if (c > 8){
        //     break;
        // }
    }    
    int swift = c > 10 ? 1 : 0;
    const string dtStr = str.substr(0, 4) + "-" + str.substr(4, 2) + "-" + str.substr(6, 2) + " " + string(pArr[1]);
    tm dtTm = to_tm(time_from_string(dtStr));
    time_t dt = mktime(&dtTm);
    int channel = atoi(pArr[2]);
    long seq = atoi(pArr[3]);
    long bidSeq = atoi(pArr[4 + swift]);
    long offerSeq = atoi(pArr[5 + swift]);
    float price = atof(pArr[6 + swift]);
    int num = atoi(pArr[7 + swift]);
    char type = pArr[8 + swift][0];
    if (type == 'B' || type == 'S') //此为上交所成交信息
        type = 'F';
    Trans trans(symbol, dt, channel, seq, bidSeq, offerSeq, price, num, type);
    return trans;
}

bool TransStream::GetTrans(Trans& trans){
    if(!transFile.eof()){
        string transStr;
        getline(transFile, transStr);
        if(transStr.size() <= 1){
            return false;
        }
        trans = StrToTrans(transStr);
        return true;
    }
    else{
        return false;
    }
}

// OrderList ----------------------------------------------
OrderList::OrderList(){
    head = Order(INVALID_SYMBOL, INVALID_TIME);
    tail = Order(INVALID_SYMBOL, INVALID_TIME);
    head.SetNext(tail);
    tail.SetPrior(head);
    loc = tail;
}

OrderList::~OrderList(){
    head.Remove();
    tail.Remove();
    loc = Order();
}

void OrderList::LocFirst(){
    loc = head.GetNext();
}

bool OrderList::GetEach(Order& order){
    while(loc != tail){
        order = loc;
        loc = loc.GetNext();
        return true;
    }
    return false;
}

void OrderList::AddOrder(Order order){
    Order prior = tail.GetPrior();
    prior.SetNext(order);
    order.SetPrior(prior);
    order.SetNext(tail);
    tail.SetPrior(order);
}

bool OrderList::IsEmpty(){
    return head.GetNext() == tail;
}

const string OrderList::ToStr(){ 
    string str("****PRICE\n");
    Order order;
    LocFirst();
    while(GetEach(order)){
        str += order.ToStr() + "\n";
    }
    return str;
}

// OrderDeque ---------------------------------------------
OrderDeque::OrderDeque(Symbol symbol, bool direct) {
    this->symbol = symbol;
    this->direct = direct;
    basePrice = INVALID_PRICE;
    maxPrice = INVALID_PRICE;
    minPrice = INVALID_PRICE;
    loc = 0;
    lastTime = MIN_TIME;
}

OrderDeque::~OrderDeque(){
    priceVec.clear();
}

void OrderDeque::InitNewDay(Price firstPrice){
    basePrice = firstPrice;
    maxPrice = (Price)(firstPrice * (1 + PRICE_SCOPE));
    minPrice = (Price)(firstPrice * (1 - PRICE_SCOPE));
    priceVec.clear();
    priceVec = vector<OrderList>(maxPrice - minPrice + 1);
}

unsigned int OrderDeque::PriceToIndex(Price price){
    return direct ? (maxPrice - price) : (price - minPrice);
}

Price OrderDeque::IndexToPrice(unsigned int index){
    return direct ? (maxPrice - index) : (minPrice + index);
}

void OrderDeque::AddOrder(Order &order){
    if(order.IsEmpty() || !order.IsValid()){
        return;
    }
    time_t orderTime = order.GetInfo()->dt;
    Price orderPrice = order.GetInfo()->price != INVALID_PRICE ? order.GetInfo()->price : GetOptPrice();
    if(orderTime - lastTime >= HOUR_SECONDS * 12){  // for new day
        InitNewDay(orderPrice);
    }
    if(orderPrice > maxPrice || orderPrice < minPrice){
        logFile.WriteLog("unknown", "ERROR", "got an order with invalid price: " + order.ToStr());
        return;
    }
    unsigned int index = PriceToIndex(orderPrice);
    priceVec.at(index).AddOrder(order);
    lastTime = orderTime;
}

Price OrderDeque::GetOptPrice(){
    for (unsigned int i = 0; i < maxPrice - minPrice + 1; i++){
        if(!priceVec.at(i).IsEmpty()){
            return IndexToPrice(i);
        }
    }
    return IndexToPrice(maxPrice - minPrice);
}

const string OrderDeque::ToStr(){
    string str(direct ? "****BID****\n" : "****OFFER****\n");
    LocFirst();
    Order order;
    while(GetEach(order)){
        str += order.ToStr() + "\n";
    }
    return str;
} 

void OrderDeque::LocFirst(){
    loc = 0;
}

bool OrderDeque::GetEach(Order& order){
    while(basePrice != INVALID_PRICE && loc < maxPrice - minPrice + 1){
        if (priceVec.at(loc).GetEach(order)){
            return true;
        }
        else {
            loc++;
            if (loc < maxPrice - minPrice + 1){
                priceVec.at(loc).LocFirst();
            }
        }
    }
    loc = 0;
    return false;
}

// Channel ------------------------------------------------
Channel::Channel(){
    orderLoc = 0;
    transLoc = 0;
}

Channel::~Channel(){
    for(int i = 0; i < orderStreamVec.size(); i++){
        delete orderStreamVec.at(i);
    }
    for(int j = 0; j < transStreamVec.size(); j++){
        delete transStreamVec.at(j);
    }
}

void Channel::LocFirst(){
    orderLoc = 0;
    transLoc = 0;
}

void Channel::AddOrderStream(const string &path, Symbol symbol){
    orderPathVec.push_back(path);
    OrderStream *os = new OrderStream();
    os->Init(symbol, path);
    orderStreamVec.push_back(os);
    orderBufferVec.push_back(Order(symbol, MIN_TIME));
}

void Channel::AddTransStream(const string &path, Symbol symbol){
    transPathVec.push_back(path);
    TransStream *ts = new TransStream();
    ts->Init(symbol, path);
    transStreamVec.push_back(ts);
    transBufferVec.push_back(Trans(symbol, MIN_TIME));
}

// Get the orders one by one in this channel at a special time.
// By calling this function once, only one order will be got at most.
// If this function return true, it means that a valid order has been got, 
// and you can try to call this function again to get next order.
// If this function return false, it means that no valid order has been got, 
// and the orders at that special time have been got completely.
bool Channel::GetOrder(time_t time, Order &order){
    for(; orderLoc < orderStreamVec.size(); orderLoc++){
        while(!orderBufferVec.at(orderLoc).IsEmpty() && orderBufferVec.at(orderLoc).GetInfo()->dt <= time){
            order = orderBufferVec.at(orderLoc); // candicate for return order
            Order newOrder;
            orderStreamVec.at(orderLoc)->GetOrder(newOrder);
            orderBufferVec.at(orderLoc) = newOrder; 
            if(order.IsValid() && order.GetInfo()->dt == time){
                return true;
            }
        }
    }
    orderLoc = 0;
    return false;
} 

bool Channel::GetTrans(time_t time, Trans &trans){
    for(; transLoc < transStreamVec.size(); transLoc++){
        while(!transBufferVec.at(transLoc).IsEmpty() && transBufferVec.at(transLoc).GetInfo()->dt <= time){
            trans = transBufferVec.at(transLoc); // candicate for return trans
            Trans newTrans;
            transStreamVec.at(transLoc)->GetTrans(newTrans);
            transBufferVec.at(transLoc) = newTrans;
            if(trans.IsValid() && trans.GetInfo()->dt == time){
                return true;
            }
        }
    }
    transLoc = 0;
    return false;
}