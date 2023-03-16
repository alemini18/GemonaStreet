//
// Created by ale on 16/03/23.
//
#include <bits/stdc++.h>
#include "libs/ready_trader_go/types.h"
#include "MarketState.cpp"

using namespace std;



const unsigned int LOT_SIZE_IS = 30;
const Lifespan LIFESPAN_IS = Lifespan::FILL_AND_KILL;
const int THRESHOLD_IS = 400;


class IntersectionStrategy {

private:

public:

    bool canSell(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        signed int delta = etf.getMean() - fut.getMean();
        return delta >= THRESHOLD_IS;
    }
    bool canBuy(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        signed int delta = etf.getMean() - fut.getMean();
        return delta <= -THRESHOLD_IS;
    }
    void calcAskSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
    if(instrument == Instrument::ETF){
        newPrice = etf.getMaxBid();
    }else{
        newPrice = fut.getMaxBid();
    }
    volume = LOT_SIZE_IS;
    lifeSpan = LIFESPAN_IS;
    }

    void calcBidSettings(Instrument instrument,
                         MarketState &etf,
                         MarketState &fut,
                         unsigned int &newPrice,
                         unsigned int &volume,
                         Lifespan &lifeSpan){
        if(instrument == Instrument::ETF){
            newPrice = etf.getMinAsk();
        }else{
            newPrice = fut.getMinAsk();
        }

        volume = LOT_SIZE_IS;
        lifeSpan = LIFESPAN_IS;
    }

};

