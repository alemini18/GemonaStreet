//
// Created by ale on 16/03/23.
//

#include "Trader.h"
#define cwarn cerr
// Copyright 2021 Optiver Asia Pacific Pty. Ltd.
//
// This file is part of Ready Trader Go.
//
//     Ready Trader Go is free software: you can redistribute it and/or
//     modify it under the terms of the GNU Affero General Public License
//     as published by the Free Software Foundation, either version 3 of
//     the License, or (at your option) any later version.
//
//     Ready Trader Go is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Affero General Public License for more details.
//
//     You should have received a copy of the GNU Affero General Public
//     License along with Ready Trader Go.  If not, see
//     <https://www.gnu.org/licenses/>.
#include <array>

#include <boost/asio/io_context.hpp>

#include <ready_trader_go/logging.h>
#include <boost/stacktrace.hpp>
#include "Trader.h"

using namespace ReadyTraderGo;
using namespace std;

RTG_INLINE_GLOBAL_LOGGER_WITH_CHANNEL(LG_AT, "AUTO")

constexpr int LOT_LIMIT = 100;

AutoTrader::AutoTrader(boost::asio::io_context& context) : BaseAutoTrader(context)
{
}

void AutoTrader::DisconnectHandler()
{
    BaseAutoTrader::DisconnectHandler();
    RLOG(LG_AT, LogLevel::LL_INFO) << "execution connection lost";
}

void AutoTrader::ErrorMessageHandler(unsigned long clientOrderId,
                                     const std::string& errorMessage)
{
    RLOG(LG_AT, LogLevel::LL_INFO) << "error with order " << clientOrderId << ": " << errorMessage<<endl;

    if (clientOrderId != 0 && ((asks.count(clientOrderId) == 1) || (bids.count(clientOrderId) == 1)))
    {
        OrderStatusMessageHandler(clientOrderId, 0, 0, 0);
    }
    cout<<"CODROIPO"<<endl;
}

void AutoTrader::HedgeFilledMessageHandler(unsigned long clientOrderId,
                                           unsigned long price,
                                           unsigned long volume)
{
    updateMutex.lock();
    RLOG(LG_AT, LogLevel::LL_INFO) << "hedge order " << clientOrderId << " filled for " << volume
                                   << " lots at $" << price << " average price in cents";
    if(hAsks.count(clientOrderId)){
        fut -= (int)volume;
        money += (int)price*volume;
        hAsks.erase(clientOrderId);
    }if(hBids.count(clientOrderId)){
        fut += (int)volume;
        money -= (int)price*volume;
        hBids.erase(clientOrderId);
    }
    updateMutex.unlock();
}

void AutoTrader::OrderBookMessageHandler(Instrument instrument,
                                         unsigned long sequenceNumber,
                                         const std::array<unsigned long, TOP_LEVEL_COUNT>& askPrices,
                                         const std::array<unsigned long, TOP_LEVEL_COUNT>& askVolumes,
                                         const std::array<unsigned long, TOP_LEVEL_COUNT>& bidPrices,
                                         const std::array<unsigned long, TOP_LEVEL_COUNT>& bidVolumes)
{
    RLOG(LG_AT, LogLevel::LL_INFO) << "order book received for " << instrument << " instrument"
                                   << ": ask prices: " << askPrices[0]
                                   << "; ask volumes: " << askVolumes[0]
                                   << "; bid prices: " << bidPrices[0]
                                   << "; bid volumes: " << bidVolumes[0];
    if(sequenceNumber<5)return;
    checkLimitOrders();
    if(instrument == Instrument::ETF){
        ETF.update(askPrices,askVolumes,bidPrices,bidVolumes);
        fixUnhedged();
        unsigned int price;
        unsigned int volume;
        Lifespan lifespan;
        if(false and IS.canSell(Instrument::ETF,ETF,FUT)){
            IS.calcAskSettings(Instrument::ETF,ETF,FUT,price,volume,lifespan);
            sendOrder(Side::SELL,true,price,volume,lifespan);
        }else if(false and IS.canBuy(Instrument::ETF,ETF,FUT)){
            IS.calcBidSettings(Instrument::ETF,ETF,FUT,price,volume,lifespan);
            sendOrder(Side::BUY,true,price,volume,lifespan);
        }else if(LIV.canBuy(Instrument::ETF,ETF,FUT)){
            LIV.calcAskSettings(Instrument::ETF,ETF,FUT,price,volume,lifespan);
            sendOrder(Side::SELL,true,price,volume,lifespan);
            LIV.calcBidSettings(Instrument::ETF,ETF,FUT,price,volume,lifespan);
            sendOrder(Side::BUY,true,price,volume,lifespan);
        }
    }else{
        FUT.update(askPrices,askVolumes,bidPrices,bidVolumes);
    }
}

void AutoTrader::OrderFilledMessageHandler(unsigned long clientOrderId,
                                           unsigned long price,
                                           unsigned long volume)
{
    RLOG(LG_AT, LogLevel::LL_INFO) << "order " << clientOrderId << " filled for " << volume
                                   << " lots at $" << price << " cents";
    updateMutex.lock();
    if(asks.count(clientOrderId)){
        etf -= (int)volume;
        money += (int)price*volume;
        unsigned int requestPrice;
        unsigned int _Volume;
        Lifespan _Lifespan;
        IS.calcBidSettings(Instrument::FUTURE, ETF, FUT, requestPrice, _Volume, _Lifespan);
        if(marketMaking.count(clientOrderId)){
            LIV.calcBidSettings(Instrument::FUTURE, ETF, FUT, requestPrice, _Volume, _Lifespan);
        }
        updateMutex.unlock();
        sendOrder(Side::BUY, false,requestPrice, volume,Lifespan::GOOD_FOR_DAY);
    }
    else if(bids.count(clientOrderId)){
        etf += (int)volume;
        money -= (int)price*volume;
        unsigned int requestPrice;
        unsigned int _Volume;
        Lifespan _Lifespan;
        IS.calcAskSettings(Instrument::FUTURE, ETF, FUT, requestPrice, _Volume, _Lifespan);
        if(marketMaking.count(clientOrderId)){
            LIV.calcAskSettings(Instrument::FUTURE, ETF, FUT, requestPrice, _Volume, _Lifespan);
        }
        updateMutex.unlock();
        sendOrder(Side::SELL, false,requestPrice, volume,Lifespan::GOOD_FOR_DAY);
    }
    else updateMutex.unlock();
}

void AutoTrader::OrderStatusMessageHandler(unsigned long clientOrderId,
                                           unsigned long fillVolume,
                                           unsigned long remainingVolume,
                                           signed long fees)
{
    updateMutex.lock();
    if (remainingVolume == 0){
        asks.erase(clientOrderId);
        bids.erase(clientOrderId);
        marketMaking.erase(clientOrderId);
    }
    updateMutex.unlock();
}

void AutoTrader::TradeTicksMessageHandler(Instrument instrument,
                                          unsigned long sequenceNumber,
                                          const std::array<unsigned long, TOP_LEVEL_COUNT>& askPrices,
                                          const std::array<unsigned long, TOP_LEVEL_COUNT>& askVolumes,
                                          const std::array<unsigned long, TOP_LEVEL_COUNT>& bidPrices,
                                          const std::array<unsigned long, TOP_LEVEL_COUNT>& bidVolumes)
{
    RLOG(LG_AT, LogLevel::LL_INFO) << "trade ticks received for " << instrument << " instrument"
                                   << ": ask prices: " << askPrices[0]
                                   << "; ask volumes: " << askVolumes[0]
                                   << "; bid prices: " << bidPrices[0]
                                   << "; bid volumes: " << bidVolumes[0];
}

bool AutoTrader::sendOrder(Side side, bool etf, int price, int volume, Lifespan lifeSpan = Lifespan::GOOD_FOR_DAY) {
    updateMutex.lock();
    unsigned long id = mNextMessageId++;
    signed int signedVolume = (side == Side::SELL ? -1 : 1) * volume;
    if(etf){     //Ordine sugli ETF
        bool can=checkLots(Instrument::ETF,signedVolume);
            if(can){
                if(side == Side::SELL) asks[id] = make_pair(price,volume);
                if(side == Side::BUY)  bids[id] = make_pair(price,volume);
            }
            //CheckOperations steady_clock
            updateMutex.unlock();
            if(can)
                SendInsertOrder(id,side,price,volume,lifeSpan);
    }else{ //Ordine sui FUT
        bool can=checkLots(Instrument::FUTURE,signedVolume);
            if(can) {
                if (side == Side::SELL) hAsks[id] = make_pair(price, volume);
                if (side == Side::BUY) hBids[id] = make_pair(price, volume);
            }
            //CheckOperation
            updateMutex.unlock();
        if(can)
         SendHedgeOrder(id,side,price,volume);
    }
    return true;
}

bool AutoTrader::checkLots(Instrument instrument, signed int request){

    if(instrument == Instrument::ETF){
        signed int sumAsks = 0;
        signed int sumBids = 0;
        for(auto x: asks) sumAsks += x.second.second;
        for(auto x: bids) sumBids += x.second.second;

        if((request > 0 and etf + request + sumBids > LOT_LIMIT) or
           (request < 0 and etf + request - sumAsks < -LOT_LIMIT)){
            return false;
        }
    }
    if(instrument == Instrument::FUTURE){
        signed int sumAsks = 0;
        signed int sumBids = 0;
        for(auto x: hAsks) sumAsks += x.second.second;
        for(auto x: hBids) sumBids += x.second.second;

        if((request > 0 and fut + request + sumBids > LOT_LIMIT) or
           (request < 0 and fut + request - sumAsks < -LOT_LIMIT)){
            return false;
        }
    }
    return true;
}

bool AutoTrader::checkLimitOrders() {
    /*while(asks.size() + bids.size() > 10){
        if(asks.begin()->first<bids.begin()->first){
            //CheckOperations
            SendCancelOrder(asks.begin()->first);
        }else{
            SendCancelOrder(bids.begin()->first);
        }
    }*/
    if(asks.size() + bids.size() > 6){
        vector<unsigned long> toBeRemoved;
        int i=0;
        for(auto x:asks){
            toBeRemoved.push_back(x.first);
            i++;
            if(i==3)break;
        }
        i=0;
        for(auto x:bids){
            toBeRemoved.push_back(x.first);
            i++;
            if(i==3)break;
        }
        sort(toBeRemoved.begin(),toBeRemoved.end());
        for(int i=0;i<3;i++)SendCancelOrder(toBeRemoved[i]);
    }
    return true;
}

void AutoTrader::fixUnhedged() {
    if(etf + fut<-10){
        unsigned int requestPrice = FUT.getMinAsk();
        unsigned int volume = abs(etf+fut+10);
        sendOrder(Side::BUY,false,requestPrice,volume);
    }
    else if(etf + fut>10){
        unsigned int requestPrice = FUT.getMaxBid();
        unsigned int volume = abs(etf+fut-10);
        sendOrder(Side::SELL,false,requestPrice,volume);
    }
}




