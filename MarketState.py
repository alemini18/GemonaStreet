class MarketState:
    """
    This class is used to trace the market of a specific title (i.e. ETF, FUTURE).
    It contains latest best prices and related volumes, used by Trading Strategies.
    """
    def __init__(self):
        self.ask_prices = []
        self.ask_volumes = []
        self.bid_prices = []
        self.bid_volumes = []
        self.prevMean = 0
        self.mean = 0

    def update(self, ask_prices, ask_volumes, bid_prices, bid_volumes):
        """
        Update values of the market when a new order book is sent. Then also the means are updated.
        @param ask_prices: vettori di 5 elementi
        @param ask_volumes:
        @param bid_prices:
        @param bid_volumes:
        """
        self.ask_prices = ask_prices
        self.ask_volumes = ask_volumes
        self.bid_prices = bid_prices
        self.bid_volumes = bid_volumes
        self.prevMean = self.mean
        self.mean = self.calcMean()

    def getMinAsk(self):
        return min(self.ask_prices)

    def getMaxBid(self):
        return max(self.bid_prices)

    def calcMean(self):
        mas = self.getMaxBid()
        mini = self.getMinAsk()
        if mas == 0:
            mas = mini
        elif mini == 0:
            mini = mas
        m = (mas + mini) // 2
        return m - (m % 100)

    def getMean(self):
        return self.mean

    def getPrevMean(self):
        return self.prevMean

    def getBidVolumeByImportance(self):
        sum = 0
        for i in range(5):
            sum += self.bid_volumes[i] // ((i + 1) * (i + 1))
        return sum

    def getAskVolumeByImportance(self):
        sum = 0
        for i in range(5):
            sum += self.ask_volumes[i] // ((i + 1) * (i + 1))
        return sum