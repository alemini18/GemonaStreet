from ready_trader_go import Instrument, Lifespan
from MarketState import MarketState
from TradingStrategy import TradingStrategy

LOT_SIZE_LIV = 7
LIFESPAN_LIV: Lifespan = Lifespan.GOOD_FOR_DAY
class LIVStrategy(TradingStrategy):

    def __init__(self):
        super().__init__()

    def canBuy(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        return etf.getMinAsk() - etf.getMaxBid() > 300

    def canSell(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        return etf.getMinAsk() - etf.getMaxBid() > 300

    def calcAskSettings(self, instrument, etf, fut):
        trader = 0
        if instrument == Instrument.ETF:
            tracer = etf
        else:
            tracer = fut
        mean = tracer.getMean()
        prevMean = tracer.getPrevMean()
        if prevMean == 0:
            prevMean = mean
        qs = tracer.getAskVolumeByImportance()
        qb = tracer.getBidVolumeByImportance()
        res = 0
        if (qb - qs) / (qb + qs) > 0.5:
            res = mean + (abs(mean - prevMean) // 100 + 2) * 100  # tick is not 1, so i need to scale deltas
        elif (qs - qb) / (qb + qs) > 0.5:  # 0.5 is the risk (the smaller the value, the bigger the risk)
            res = mean + abs(mean - prevMean)
        else:
            res = mean + (abs(mean - prevMean) // 100 + 1) * 100
        return [res - res % 100,LOT_SIZE_LIV,LIFESPAN_LIV]

    def calcBidSettings(self, instrument, etf, fut):
        tracer = 0
        if instrument == Instrument.ETF:
            tracer = etf
        else:
            tracer = fut
        mean = tracer.getMean()
        prevMean = tracer.getPrevMean()
        if prevMean == 0:
            prevMean = mean
        qs = tracer.getAskVolumeByImportance()
        qb = tracer.getBidVolumeByImportance()
        res = 0
        if (qb - qs) / (qb + qs) > 0.5:
            res = mean - abs(mean - prevMean)
        elif (qs - qb) / (qb + qs) > 0.5:
            res = mean - (abs(mean - prevMean) // 100 + 2) * 100
        else:
            res = mean - (abs(mean - prevMean) // 100 + 1) * 100
        return [res - res % 100,LOT_SIZE_LIV,LIFESPAN_LIV]




