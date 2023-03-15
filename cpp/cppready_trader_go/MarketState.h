//
// Created by ale on 15/03/23.
//

#ifndef GEMONASTREET_MARKETSTATE_H
#define GEMONASTREET_MARKETSTATE_H


class MarketState {

private:
    array<int,5> askPrices,
                 askVolumes,
                 bidPrices,
                 bidVolumes;
    int mean,
        prevMean;

public:
    MarketState();
    int getMinAsk();
    int getMaxBid();
    int calcMean();
    int getMean();
    int getBidVolumeByImportance();
    int getAskVolumeByImportance();

};


#endif //GEMONASTREET_MARKETSTATE_H
