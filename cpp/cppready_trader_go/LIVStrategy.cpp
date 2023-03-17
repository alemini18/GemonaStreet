//
// Created by ale on 16/03/23.
//
#ifndef GEMONASTREET_LIVSTRATEGY
#define GEMONASTREET_LIVSTRATEGY
#include <bits/stdc++.h>
#include "libs/ready_trader_go/types.h"
#include "MarketState.cpp"
#define uabs(u) abs((int)(u))
using namespace std;
using namespace ReadyTraderGo;

typedef unsigned long price;

const unsigned int LOT_SIZE = 7;
const Lifespan LIFESPAN = Lifespan::GOOD_FOR_DAY;

class LIVStrategy {
public:

    int square(int x){
        return x*x;
    }
    bool canBuy(Instrument instrument, //Se si usa anche canSell aggiungere la volatilità anche li
                MarketState &etf,
                MarketState &fut){
        static auto start = chrono::steady_clock::now();
        int sum = 0;
        //cerr<<etf.historicalSize()<<endl;
        if(etf.historicalSize() < 60)return false;
        for(int i=1;i<60;i++){
            sum+=square((etf.getHistoricalMean(i)-etf.getHistoricalMean(i-1))/100);
        }
        auto now = chrono::steady_clock::now();
        chrono::duration<double> diff = now - start;
        cerr<<diff.count()<<"sigma: "<<(double)sum/60.0<<endl;
        return true;
        //return ((etf.getMinAsk() - etf.getMaxBid() > 0) and ((1.9 > (double)sum/60.0) > 0.7 ));

    }
    bool canSell(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        return etf.getMinAsk() - etf.getMaxBid() > 300;

    }
    void calcAskSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
        MarketState *state;
        state=(instrument == Instrument::ETF ? &etf : &fut);
        unsigned int mean = state->getMean();
        unsigned int prevMean = state->getPrevMean();
        if(prevMean == 0)
            prevMean = mean;
        unsigned int qs = state->getAskVolumeByImportance();
        unsigned int qb = state->getBidVolumeByImportance();
        unsigned int res = 0;
        if(2*(qb - qs) > (qb + qs))
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 2) * 100;
        else if(2*(qs - qb) > (qb + qs))
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 0) * 100;
        else
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 1) * 100;
        newPrice = res + (res % 100 >= 50 ? 100-(res%100) : -(res%100));
        volume = LOT_SIZE;
        lifeSpan = LIFESPAN;

    }
    void calcBidSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
        MarketState *state;
        state = (instrument == Instrument::ETF ? &etf : &fut);
        unsigned int mean = state->getMean();
        unsigned int prevMean = state->getPrevMean();
        if(prevMean == 0)
            prevMean = mean;
        unsigned int qs = state->getAskVolumeByImportance();
        unsigned int qb = state->getBidVolumeByImportance();
        unsigned int res = 0;
        if(2*(qb - qs) > (qb + qs))
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 0) * 100;
        else if(2*(qs - qb) > (qb + qs))
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 2) * 100;
        else
            res = mean + (abs((int)mean - (int)prevMean) / 100 + 1) * 100;
        newPrice = res + (res % 100 >= 50 ? 100-(res%100) : -(res%100));
        volume = LOT_SIZE;
        lifeSpan = LIFESPAN;
    }


};

#endif //GEMONASTREET_LIVSTRATEGY
