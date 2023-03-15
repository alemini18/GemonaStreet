//
// Created by ale on 15/03/23.
//

#include "MarketState.h"


MarketState::MarketState() {
    askPrices = askVolumes = bidPrices = bidVolumes = array<int,5>();
    mean = prevMean = 0;
}
int MarketState::getMinAsk(){
    return *min_element(askPrices.begin(),askPrices.end());

}
int MarketState::getMaxBid(){
    return *max_element(bidPrices.begin(),bidPrices.end());
}
int MarketState::calcMean(){
    int maxAsk = MarketState::getMinAsk();
    int minBid = MarketState::getMaxBid();
    if(maxAsk == 0) maxAsk = minBid;
    if(minBid == 0)minBid = maxAsk;
    int m = (maxAsk + minBid)/2;
    return m - m % 100;
}
int MarketState::getMean(){

}
int MarketState::getBidVolumeByImportance(){

}
int MarketState::getAskVolumeByImportance(){

}