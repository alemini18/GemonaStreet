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

const int LOT_SIZE = 7;
const Lifespan LIFESPAN = Lifespan::GOOD_FOR_DAY;
class LIVStrategy {

private:

public:


    bool canBuy(Instrument instrument,
                MarketState &etf,
                MarketState &fut){
        return etf.getMinAsk() - etf.getMaxBid() >= 0;

    }
    bool canSell(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        return etf.getMinAsk() - etf.getMaxBid() >= 0;

    }
    void calcAskSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
        MarketState *state ;
        state=(instrument == Instrument::ETF ? &etf : &fut);
        unsigned int mean = state->getMean();
        unsigned int prevMean = state->getPrevMean();
        if(prevMean == 0)
            prevMean = mean;
        unsigned int qs = state->getAskVolumeByImportance();
        unsigned int qb = state->getBidVolumeByImportance();
        unsigned int res = 0;
        if(2*(qb - qs) > (qb + qs))
            res = mean + (abs((int)(mean - prevMean)) / 100 + 2) * 100;
        else if(2*(qs - qb) > (qb + qs))
            res = mean + (abs((int)(mean - prevMean)) / 100 + 0) * 100;
        else
            res = mean + (abs((int)(mean - prevMean)) / 100 + 1) * 100;
        newPrice = res + (res % 100 >= 50 ? 100-res%100 : -res%100);
        volume = LOT_SIZE;
        lifeSpan = LIFESPAN;

    }
    void calcBidSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
        MarketState &state = (instrument == Instrument::ETF ? etf : fut);
        unsigned int mean = state.getMean();
        unsigned int prevMean = state.getPrevMean();
        if(prevMean == 0)
            prevMean = mean;
        unsigned int qs = state.getAskVolumeByImportance();
        unsigned int qb = state.getBidVolumeByImportance();
        unsigned int res = 0;
        if(2*(qb - qs) > (qb + qs))
            res = mean + (uabs(mean - prevMean) / 100 + 0) * 100;
        else if(2*(qs - qb) > (qb + qs))
            res = mean + (uabs(mean - prevMean) / 100 + 2) * 100;
        else
            res = mean + (uabs(mean - prevMean) / 100 + 1) * 100;
        newPrice = res + (res % 100 >= 50 ? 100-res%100 : -res%100);
        volume = LOT_SIZE;
        lifeSpan = LIFESPAN;
    }


};

#endif //GEMONASTREET_LIVSTRATEGY
