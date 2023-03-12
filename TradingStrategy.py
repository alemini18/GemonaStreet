from abc import ABC, abstractmethod
from typing import List

from MarketState import MarketState
from ready_trader_go import Lifespan


class TradingStrategy(ABC):
    """
    This class is used to handle a trading strategies. The autotrader choose a trading strategy
    according to the market situation.
    """
    def __init__(self):
        pass

    @abstractmethod
    def canBuy(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        pass
    @abstractmethod
    def canSell(self, instrument: int, etf: MarketState, fut: MarketState) -> bool:
        pass

    @abstractmethod
    def calcAskSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        pass

    @abstractmethod
    def calcBidSettings(self, instrument: int, etf: MarketState, fut: MarketState):
        pass

