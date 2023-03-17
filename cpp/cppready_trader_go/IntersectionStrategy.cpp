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
    vector<double> avg_gain_up;
    int tau_up = 400;
    vector<double> avg_gain_down;
    int tau_down = -400;
    int lastMax;
    int lastMin;
    mutex updateMutex;

public:

    IntersectionStrategy(){
        avg_gain_up = vector<double>(7,350);
        avg_gain_down = vector<double>(7, 350);
    }

    int argo_max(vector<double> const& vec){
        return distance(vec.begin(), max_element(vec.begin(), vec.end()));
    }

    void calc_up_threshold(int last_diff_up,int meanETF) {
        updateMutex.lock();
        for (int i=0; i<7; i++) {
            int t = (i+1)*100;
            if (t >= last_diff_up) {
                avg_gain_up[i] = avg_gain_up[i] * 5.0 / 6.0;
            } else {
                avg_gain_up[i] = (avg_gain_up[i] * 5.0 + (t - 0.0002 * (meanETF/100))) / 6.0;
            }
        }
        tau_up = (argo_max(avg_gain_up)+1) * 100;
        updateMutex.unlock();
    };

    void calc_down_threshold(int last_diff_down, int meanETF) {
        updateMutex.lock();
        for (int i=0; i<7; i++) {
            int t = -(i+1)*100;
            if (t <= last_diff_down) {
                avg_gain_down[i] = avg_gain_down[i] * 5.0 / 6.0;
            } else {
                avg_gain_down[i] = (avg_gain_down[i] * 5.0 + (abs(t) - 0.0002 * (meanETF/100))) / 6.0;
            }
        }
        tau_down = -(argo_max(avg_gain_down)+1) * 100;
        updateMutex.unlock();
    };

    bool canSell(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        signed int delta = etf.getMean() - fut.getMean();
        updateMutex.lock();
        lastMax = max(lastMax,delta);
        if(delta == 0 and lastMax > 0){
            updateMutex.unlock();
            calc_up_threshold(lastMax,etf.getMean());
            updateMutex.lock();
            lastMax = 0;
        }
        updateMutex.unlock();
        cout<<"tau_up: "<<tau_up<<endl;
        return delta >= max(tau_up,400);
    }
    bool canBuy(Instrument instrument,
                 MarketState &etf,
                 MarketState &fut){
        signed int delta = etf.getMean() - fut.getMean();
        updateMutex.lock();
        lastMin = min(lastMin,delta);
        if(delta == 0 and lastMin < 0){
            updateMutex.unlock();
            calc_down_threshold(lastMin,etf.getMean());
            updateMutex.lock();
            lastMin = 0;
        }
        updateMutex.unlock();
        cout<<"tau_down: "<<tau_down<<endl;
        return delta <= min(tau_down,-400);
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

