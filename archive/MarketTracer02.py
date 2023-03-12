
import time
class MarketTracer:

    def __init__(self, instrument):
        self.ask_prices = []
        self.ask_volumes = []
        self.bid_prices = []
        self.bid_volumes = []
        self.precMean = 0
        self.mean = 0
        self.instrument = instrument

    def update(self, ask_prices, ask_volumes, bid_prices, bid_volumes):
        self.ask_prices = ask_prices
        self.ask_volumes = ask_volumes
        self.bid_prices = bid_prices
        self.bid_volumes = bid_volumes
        """print(ask_prices)
        print(ask_volumes)
        print(bid_prices)
        print(bid_volumes)"""
        self.precMean = self.mean
        self.mean = self.calcMean()

    def getMinAsk(self):
        return min(self.ask_prices)

    def getMaxBid(self):
        return max(self.bid_prices)

    def calcMean(self):
        mas = self.getMaxBid()
        mini = self.getMinAsk()
        if mas == 0:
            mas= mini
        elif mini == 0:
            mini = mas
        m = (mas+mini)//2
        return m-(m%100)

    def getMean(self):
        return self.mean

    def getPrecMean(self):
        return self.precMean

    def getBidVolumeByImportance(self):
        sum = 0
        mean = self.getMean()
        for i in range(5):
            sum += self.bid_volumes[i]//((i+1)**2)#((abs(mean-self.bid_prices[i])//100+1)**2)
        return sum

    def getAskVolumeByImportance(self):
        sum = 0
        mean = self.getMean()
        for i in range(5):
            sum += self.ask_volumes[i] //((i+1)**2)#((abs(mean-self.ask_prices[i])//100+1)**2)
        return sum