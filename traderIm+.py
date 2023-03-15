import asyncio
import itertools
import time

from typing import List
from threading import *


from ready_trader_go import BaseAutoTrader, Instrument, MAXIMUM_ASK, MINIMUM_BID, Side
from IntersectionStrategy import IntersectionStrategy
from LIVstrategy import LIVStrategy
from MarketState import MarketState

LOT_LIMIT = 100

class AutoTrader(BaseAutoTrader):

    def __init__(self, loop: asyncio.AbstractEventLoop, team_name: str, secret: str):
        """Initialise a new instance of the AutoTrader class."""
        super().__init__(loop, team_name, secret)
        self.order_ids = itertools.count(1)
        self.asks = {}  # active ask orders with their price and volume
        self.bids = {}  # active bid ....
        self.marketMaking = set()
        self.etfs = 0  # actual etfs (may be negative)
        self.fut = 0
        self.operation = 0  # max 50 orders per second
        self.lastTime = time.time()
        self.ETF = MarketState() #tiene traccia dei valori dell'etf e ne calcola media, massimo, minimo, ecc
        self.FUT = MarketState() #tiene traccia dei valori del future e ne calcola media, massimo, minimo, ecc
        self.LIV = LIVStrategy() #strategia di market making
        self.IS = IntersectionStrategy() #stategia di statistical arbitrage
        #self.semaforo = Semaphore(1)
        #self.semaforo1 = Semaphore(1)
        self.operationSemaphore = Semaphore(1)

    def checkOperations(self) -> bool:
        self.operationSemaphore.acquire()
        if time.time() - self.lastTime > 1:
            self.lastTime = time.time()
            self.operation = 0
        if self.operation < 50:
            self.operation+=1

    def

