from typing import List
import numpy as np

from ready_trader_go import Instrument, Lifespan
from MarketState import MarketState
from TradingStrategy import TradingStrategy

import logging
LOT_SIZE_IS = 30
LIFESPAN_IS: Lifespan = Lifespan.FILL_AND_KILL
FEE = 0.0002

class IntersectionStrategy(TradingStrategy):

    def __init__(self):
        super().__init__()
        self.logger = logging.getLogger("DIFFERENCES")
        self.differences_positive = []
        self.differences_negative = []
        self.possible_threshold = np.arange(100, 700, 100)
        self.avg_gain_up = np.ones(10) * 250
        self.tau_up = 400
        self.avg_gain_down = np.ones(10) * 250
        self.tau_down = -400
        self.localMax = 0
        self.localMin = 0
        self.haveSell = False
    def calc_up_threshold(self,last_max):
        '''
        Called everytime a new local max has been recived.
        '''
        for i, t in enumerate(self.possible_threshold):
            if self.tau_up > last_max:
                self.avg_gain_up[i] = self.avg_gain_up[i] * 9 / 10
            else:
                self.avg_gain_up[i] = (self.avg_gain_up[i] * 9 + (t - FEE * 1400))/10
        self.tau_up = (np.argmax(self.avg_gain_up)+1) * 100 #il valore da usare per canSell è questo

    def calc_down_threshold(self,last_min):
        '''
        Called everytime a new local min has been recived.
        '''
        for i, t in enumerate(self.possible_threshold):
            if self.tau_down < last_min:
                self.avg_gain_down[i] = self.avg_gain_down[i] * 9 / 10
            else:
                self.avg_gain_down[i] = (self.avg_gain_down[i] * 9 + (t - FEE * 1400)) / 10
        self.tau_down = -(np.argmax(self.avg_gain_down) + 1) * 100  ##il valore da usare per canBuy è questo

    def canSell(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:  #canBuy e canSell vanno chiamate assieme
        delta =etf.getMean() - fut.getMean()
        self.localMax = max(self.localMax,delta)
        if delta == 0 and self.localMax > 0:
            self.calc_up_threshold(self.localMax)
            self.localMax = 0
        self.logger.warning(self.tau_up)
        return delta >= self.tau_up

    def canBuy(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        delta = etf.getMean() - fut.getMean()
        self.localMin = min(self.localMin,delta)
        if delta == 0 and self.localMin < 0:

            self.calc_down_threshold(self.localMin)
            self.localMin = 0
        self.logger.warning(self.tau_down)
        return delta <= self.tau_down

    def calcAskSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        if instrument == Instrument.ETF:
            return [etf.getMaxBid(), LOT_SIZE_IS, LIFESPAN_IS]
        else:
            return [fut.getMaxBid(), LOT_SIZE_IS, LIFESPAN_IS]

    def calcBidSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        if instrument == Instrument.ETF:
            return [etf.getMinAsk(), LOT_SIZE_IS, LIFESPAN_IS]
        else:
            return [fut.getMinAsk(), LOT_SIZE_IS, LIFESPAN_IS]
