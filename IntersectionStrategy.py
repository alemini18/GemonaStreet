from typing import List

from ready_trader_go import Instrument, Lifespan
from MarketState import MarketState
from TradingStrategy import TradingStrategy

import logging
LOT_SIZE = 30
LIFESPAN: Lifespan = Lifespan.FILL_AND_KILL

class IntersectionStrategy(TradingStrategy):

    def __init__(self):
        super().__init__()
        self.logger = logging.getLogger("DIFFERENCES")
        self.differences_positive = []
        self.differences_negative = []

    def canBuy(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:  #canBuy e canSell vanno chiamate assieme
        delta = etf.getMean() - fut.getMean()
        if delta <= 0:
            self.differences_negative.append(delta)
        if len(self.differences_negative) < 100:
            return False
        if len(self.differences_positive) > 100:
            self.differences_positive.pop(0)
        if len(self.differences_negative) > 100:
            self.differences_negative.pop(0)
        THRESHOLD = sorted(self.differences_negative)[14]
        self.logger.warning(THRESHOLD)
        return delta < THRESHOLD

    def canSell(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        delta = etf.getMean() - fut.getMean()
        if delta >= 0:
            self.differences_positive.append(delta)
        if len(self.differences_positive) < 100:
            return False
        if len(self.differences_positive) > 100:
            self.differences_positive.pop(0)
        if len(self.differences_negative) > 100:
            self.differences_negative.pop(0)
        THRESHOLD = sorted(self.differences_positive)[-15]
        self.logger.warning(THRESHOLD)
        return delta > THRESHOLD

    def calcAskSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        if instrument == Instrument.ETF:
            return [etf.getMaxBid(), LOT_SIZE, LIFESPAN]
        else:
            return [fut.getMaxBid(), LOT_SIZE, LIFESPAN]

    def calcBidSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        if instrument == Instrument.ETF:
            return [etf.getMinAsk(), LOT_SIZE, LIFESPAN]
        else:
            return [fut.getMinAsk(), LOT_SIZE, LIFESPAN]
