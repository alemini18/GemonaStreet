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
    int mean,
        prevMean;
    mutex updateMutex;

public:
    MarketState(){
        askPrices = askVolumes = bidPrices = bidVolumes = array<long unsigned int,5>();
        mean = prevMean = 140000;
    }
    void update(array<long unsigned int,5> askPrices,
                array<long unsigned int,5> askVolumes,
                array<long unsigned int,5> bidPrices,
                array<long unsigned int,5> bidVolumes){
        lock_guard<mutex> guard(updateMutex);
        this->askPrices = askPrices;
        this->askVolumes = askVolumes;
        this->bidPrices = bidPrices;
        this->bidVolumes = bidVolumes;
        prevMean = mean;
        mean = calcMean();
    }
    int getMinAsk(){
        return *min_element(askPrices.begin(),askPrices.end());
    }
    int getMaxBid(){
        return *max_element(bidPrices.begin(),bidPrices.end());
    }
    int calcMean(){
        int minAsk = getMinAsk();
        int maxBid = getMaxBid();
        if(minAsk == 0) minAsk = maxBid;
        if(maxBid == 0)maxBid = minAsk;
        int m = (minAsk + maxBid)/2;
        if (m == 0){
            cout<<"ZERO\n\n";
            return 140000;
        }
        return m + (m % 100 > 50 ? 100-m%100 : -m%100);
    }
    int getMean(){
        return mean;
    }
    int getPrevMean(){
        return prevMean;
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

};
#endif //GEMONASTREET_MARKETSTATE_H
