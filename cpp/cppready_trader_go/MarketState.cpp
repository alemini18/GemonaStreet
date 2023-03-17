//
// Created by ale on 15/03/23.
//

#ifndef GEMONASTREET_MARKETSTATE_H
#define GEMONASTREET_MARKETSTATE_H

#include <bits/stdc++.h>

using namespace std;



class MarketState {

private:
    array<long unsigned int,5> askPrices,
                 askVolumes,
                 bidPrices,
                 bidVolumes;
    deque<int> priceHistory;
    int mean,
        prevMean;
    mutex updateMutex;

public:
    MarketState(){
        askPrices = askVolumes = bidPrices = bidVolumes = array<long unsigned int,5>();
        mean = prevMean = 0;
    }
    int getMinAsk(){
        return askPrices[0];//*min_element(askPrices.begin(),askPrices.end());
    }
    int getMaxBid(){
        return bidPrices[0];//*max_element(bidPrices.begin(),bidPrices.end());
    }
    int calcMean(){
        int minAsk = getMinAsk();
        int maxBid = getMaxBid();
        if(minAsk == 0) minAsk = maxBid;
        if(maxBid == 0)maxBid = minAsk;
        int m = (minAsk + maxBid)/2;
        return m + (m % 100 > 50 ? 100-(m%100) : -(m%100));
    }
    int getMean(){
        return mean;
    }
    int getPrevMean(){
        return prevMean;
    }
    void update(array<long unsigned int,5> askPrice,
                array<long unsigned int,5> askVolume,
                array<long unsigned int,5> bidPrice,
                array<long unsigned int,5> bidVolume){
        updateMutex.lock();
        askPrices = askPrice;
        askVolumes = askVolume;
        bidPrices = bidPrice;
        bidVolumes = bidVolume;
        prevMean = mean;
        mean = calcMean();
        priceHistory.push_back(mean);
        if(priceHistory.size() > 60)priceHistory.pop_front();
        updateMutex.unlock();
    }
    int getBidVolumeByImportance(){
        int sum = 0;
        for(int i = 0;i < 5;i++){
            sum+=bidVolumes[i]/((i+1)*(i+1));
        }
        return sum;
    }
    int getAskVolumeByImportance(){
        int sum = 0;
        for(int i = 0;i < 5;i++){
            sum+=askVolumes[i]/((i+1)*(i+1));
        }
        return sum;
    }
    int historicalSize(){
        return priceHistory.size();
    }
    int getHistoricalMean(int id){
        if(id >= priceHistory.size()) id = priceHistory.size() - 1;
        return priceHistory[id];

    }


};
#endif //GEMONASTREET_MARKETSTATE_H
