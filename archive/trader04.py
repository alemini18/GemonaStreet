# Copyright 2021 Optiver Asia Pacific Pty. Ltd.
#
# This file is part of Ready Trader Go.
#
#     Ready Trader Go is free software: you can redistribute it and/or
#     modify it under the terms of the GNU Affero General Public License
#     as published by the Free Software Foundation, either version 3 of
#     the License, or (at your option) any later version.
#
#     Ready Trader Go is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU Affero General Public License for more details.
#
#     You should have received a copy of the GNU Affero General Public
#     License along with Ready Trader Go.  If not, see
#     <https://www.gnu.org/licenses/>.
import asyncio
import itertools
import time

from typing import List

from pyready_trader_go.archive.MarketTracer02 import MarketTracer
from pyready_trader_go.archive.MarketState import MarketState

from ready_trader_go import BaseAutoTrader, Instrument, Lifespan, MAXIMUM_ASK, MINIMUM_BID, Side

LOT_SIZE_BID = 2
LOT_SIZE_ASK = 2
LOT_LIMIT = 80

POSITION_LIMIT = 100
TICK_SIZE_IN_CENTS = 100
MIN_BID_NEAREST_TICK = (MINIMUM_BID + TICK_SIZE_IN_CENTS) // TICK_SIZE_IN_CENTS * TICK_SIZE_IN_CENTS
MAX_ASK_NEAREST_TICK = MAXIMUM_ASK // TICK_SIZE_IN_CENTS * TICK_SIZE_IN_CENTS


class AutoTrader(BaseAutoTrader):
    """Example Auto-trader.

    When it starts this auto-trader places ten-lot bid and ask orders at the
    current best-bid and best-ask prices respectively. Thereafter, if it has
    a long position (it has bought more lots than it has sold) it reduces its
    bid and ask prices. Conversely, if it has a short position (it has sold
    more lots than it has bought) then it increases its bid and ask prices.
    """

    # REMINDER: max lot per order = 20
    def __init__(self, loop: asyncio.AbstractEventLoop, team_name: str, secret: str):
        """Initialise a new instance of the AutoTrader class."""
        super().__init__(loop, team_name, secret)
        self.order_ids = itertools.count(1)
        self.asks = {}  # active ask orders with their price and volume
        self.bids = {}  # active bid ....
        self.hasks = {}  # active ask orders with their price and volume
        self.hbids = {}  # active bid ....
        self.etfs = 0  # actual etfs (may be negative)
        self.fut = 0
        self.soldi = 2000000  # available money
        self.operation = 0  # max 50 orders per second
        self.lastSecond = time.time()
        self.ETF = MarketTracer(Instrument.ETF)
        self.FUT = MarketTracer(Instrument.FUTURE)
        self.ETFp = MarketState()
        self.FUTp = MarketState()

    def checkOperations(self) -> None:
        if time.time() - self.lastSecond >= 1:
            self.lastSecond = time.time()
            self.operation = 0
        if self.operation < 40:
            self.operation += 1
        else:
            time.sleep(1)
            self.lastSecond = time.time()
            self.operation = 1
            print("Superate le 40 operazioni. Aspettato un secondo.")

    def checkLots(self, instrument: int, request: int) -> bool:  # request is negative for asks and positive for bids
        if instrument == Instrument.ETF:
            if (request > 0 and self.etfs + request > LOT_LIMIT) or (request < 0 and self.etfs + request < -LOT_LIMIT):
                print("Attenzione: stavi per sforare gli etf")
                return False
        if instrument == Instrument.FUTURE:
            if (request > 0 and self.fut + request > LOT_LIMIT) or (request < 0 and self.fut + request < -LOT_LIMIT):
                print("Attenzione: stavi per sforare i future")
                return False
        return True

    def checkLimitOrders(self):
        allOrders = list(self.asks.keys()) + list(self.bids.keys())
        allOrders.sort()
        if len(allOrders) >= 10:
            for i in range(3):
                self.checkOperations()
                self.send_cancel_order(allOrders[i])
                if allOrders[i] in self.asks: self.asks.pop(allOrders[i])
                if allOrders[i] in self.bids: self.bids.pop(allOrders[i])
                print("canceled: ", allOrders[i])


    def checkCrossing(self, history, price):

        for order in history:
            if history[order][0] == price:
                return False
        return True

    def checkUnhedged(self):
        """
        If there are some unhedged lots (> 10), fix it.
        """
        if self.etfs + self.fut < -8:
            request_price = self.calcAskPrice(self.FUT)
            volume = abs(self.etfs + self.fut + 8)
            if request_price and self.checkLots(Instrument.FUTURE, volume):
                print("Special HBID")
                bid_id = next(self.order_ids)
                self.checkOperations()
                self.send_hedge_order(bid_id, Side.BID, request_price, volume)
                self.hbids[bid_id] = [request_price, volume]
        if self.etfs + self.fut > 8:
            request_price = self.calcBidPrice(self.FUT)
            volume = abs(self.etfs + self.fut - 8)
            if request_price and self.checkLots(Instrument.FUTURE, -volume):
                print("Special HASK")
                ask_id = next(self.order_ids)
                self.checkOperations()
                self.send_hedge_order(ask_id, Side.ASK, request_price, volume)
                self.hasks[ask_id] = [request_price, volume]


    def calcAskPrice(self, tracer: MarketTracer):
        mean = tracer.getMean()
        precMean = tracer.getPrecMean()
        lFactor = 0.5
        global LOT_SIZE_ASK
        LOT_SIZE_ASK = 2
        if tracer.instrument == Instrument.ETF:
            if self.ETFp.isEligibleForTradeSell(mean):
                LOT_SIZE_ASK = 4
        if tracer.instrument == Instrument.FUTURE:
            if self.FUTp.isEligibleForTradeSell(mean):
                LOT_SIZE_ASK = 4
        if precMean == 0:
            precMean = mean
        qs = tracer.getAskVolumeByImportance()
        qb = tracer.getBidVolumeByImportance()
        res = 0
        if (qb - qs)/(qb + qs) > lFactor:
            res = mean + (abs(mean-precMean)//100+2)*100  #tick is not 1, so i need to scale deltas
        elif (qs - qb)/(qb + qs) > lFactor:   #0.5 is the risk (the smaller the value, the bigger the risk)
            res = mean + abs(mean-precMean)
        else:
            res = mean + (abs(mean - precMean)//100 + 1) * 100
        return res - res%100

    def calcBidPrice(self, tracer: MarketTracer):
        mean = tracer.getMean()
        precMean = tracer.getPrecMean()
        lFactor = 0.5
        global LOT_SIZE_BID
        LOT_SIZE_BID = 2
        if tracer.instrument == Instrument.ETF:
            if self.ETFp.isEligibleForTradeBuy(mean):
                LOT_SIZE_BID = 4
        if tracer.instrument == Instrument.FUTURE:
            if self.FUTp.isEligibleForTradeBuy(mean):
                LOT_SIZE_BID = 4
        self.logger.info("Mean: %d, precMean: %d", mean, precMean)
        if precMean == 0:
            precMean = mean
        qs = tracer.getAskVolumeByImportance()
        qb = tracer.getBidVolumeByImportance()
        res = 0
        if (qb - qs) / (qb + qs) > lFactor:
            res = mean - abs(mean - precMean)
        elif (qs - qb) / (qb + qs) > lFactor:  #Usiamo lo stesso lFactor sia per i future che per i etf
            res = mean - (abs(mean - precMean)//100+2) * 100
        else:
            res = mean - (abs(mean - precMean)//100 + 1) * 100
        self.logger.info("Bid Price: %d", res)
        return res - res%100


    def on_error_message(self, client_order_id: int, error_message: bytes) -> None:
        """Called when the exchange detects an error.

        If the error pertains to a particular order, then the client_order_id
        will identify that order, otherwise the client_order_id will be zero.
        """
        self.logger.warning("error with order %d: %s", client_order_id, error_message.decode())
        # TODO: cancellare tutti gli ordini attivi
        for order in self.asks:
            self.checkOperations()
            self.send_cancel_order(order)
        for order in self.bids:
            self.checkOperations()
            self.send_cancel_order(order)
        self.asks = {}
        self.bids = {}
        self.logger.warning("canceled orders")

    def on_hedge_filled_message(self, client_order_id: int, price: int, volume: int) -> None:
        """Called when one of your hedge orders is filled.

        The price is the average price at which the order was (partially) filled,
        which may be better than the order's limit price. The volume is
        the number of lots filled at that price.
        """
        self.logger.info("received hedge filled for order %d with average price %d and volume %d", client_order_id,
                         price, volume)
        if client_order_id in self.hasks:
            self.fut -= volume
            self.soldi += volume * price
            self.hasks.pop(client_order_id)
        if client_order_id in self.hbids:
            self.fut += volume
            self.soldi -= volume * price
            self.hbids.pop(client_order_id)
        print("fut: ", self.fut, "soldi: ", self.soldi)

    def on_order_book_update_message(self, instrument: int, sequence_number: int, ask_prices: List[int],
                                     ask_volumes: List[int], bid_prices: List[int], bid_volumes: List[int]) -> None:
        """Called periodically to report the status of an order book.

        The sequence number can be used to detect missed or out-of-order
        messages. The five best available ask (i.e. sell) and bid (i.e. buy)
        prices are reported along with the volume available at each of those
        price levels.
        """
        self.logger.info("received order book for instrument %d with sequence number %d", instrument,
                         sequence_number)
        self.checkLimitOrders()
        if instrument == Instrument.ETF:
            self.ETF.update(ask_prices, ask_volumes, bid_prices, bid_volumes)
            self.ETFp.addEntry(self.ETF.getMaxBid(),self.ETF.getMinAsk())
            minAsk = self.ETF.getMinAsk()
            maxBid = self.ETF.getMaxBid()
            self.checkUnhedged()
            if minAsk > maxBid:
                bidPrice = self.calcBidPrice(self.ETF)
                askPrice = self.calcAskPrice(self.ETF)
                if askPrice > 0 and self.checkLots(Instrument.ETF, -LOT_SIZE_ASK) and self.checkCrossing(self.asks,
                                                                                                         askPrice):
                    print("SELL")
                    ask_id = next(self.order_ids)
                    self.checkOperations()
                    self.send_insert_order(ask_id, Side.SELL, askPrice, LOT_SIZE_ASK, Lifespan.GOOD_FOR_DAY)
                    self.asks[ask_id] = [askPrice, LOT_SIZE_ASK]
                if bidPrice > 0 and self.checkLots(Instrument.ETF, LOT_SIZE_BID) and self.checkCrossing(self.bids,
                                                                                                        bidPrice):
                    print("BUY")
                    bid_id = next(self.order_ids)
                    self.checkOperations()
                    self.send_insert_order(bid_id, Side.BUY, bidPrice, LOT_SIZE_BID, Lifespan.GOOD_FOR_DAY)
                    self.bids[bid_id] = [bidPrice, LOT_SIZE_BID]

        else:
            self.FUT.update(ask_prices, ask_volumes, bid_prices, bid_volumes)
            self.FUTp.addEntry(self.FUT.getMaxBid(), self.FUT.getMinAsk())

    def on_order_filled_message(self, client_order_id: int, price: int, volume: int) -> None:
        """Called when one of your orders is filled, partially or fully.

        The price is the price at which the order was (partially) filled,
        which may be better than the order's limit price. The volume is
        the number of lots filled at that price.
        """
        self.logger.info("received order filled for order %d with price %d and volume %d", client_order_id,
                         price, volume)
        if client_order_id in self.asks:
            self.etfs -= volume
            self.soldi += volume * price
            request_price = self.calcBidPrice(self.FUT)
            if request_price and self.checkLots(Instrument.FUTURE, volume):
                print("HBID")
                bid_id = next(self.order_ids)
                self.checkOperations()
                self.send_hedge_order(bid_id, Side.BID, request_price, volume)
                self.hbids[bid_id] = [request_price, volume]
        if client_order_id in self.bids:
            self.etfs += volume
            self.soldi -= volume * price
            request_price = self.calcAskPrice(self.FUT)
            if request_price and self.checkLots(Instrument.FUTURE, -volume):
                print("HASK")
                ask_id = next(self.order_ids)
                self.checkOperations()
                self.send_hedge_order(ask_id, Side.ASK, request_price, volume)
                self.hasks[ask_id] = [request_price, volume]
        print("etfs: ", self.etfs, "soldi: ", self.soldi)

    def on_order_status_message(self, client_order_id: int, fill_volume: int, remaining_volume: int,
                                fees: int) -> None:
        """Called when the status of one of your orders changes.

        The fill_volume is the number of lots already traded, remaining_volume
        is the number of lots yet to be traded and fees is the total fees for
        this order. Remember that you pay fees for being a market taker, but
        you receive fees for being a market maker, so fees can be negative.

        If an order is cancelled its remaining volume will be zero.
        """
        self.logger.info("received order status for order %d with fill volume %d remaining %d and fees %d",
                         client_order_id, fill_volume, remaining_volume, fees)
        if remaining_volume == 0:
            if client_order_id in self.asks: self.asks.pop(client_order_id)
            if client_order_id in self.bids: self.bids.pop(client_order_id)

    def on_trade_ticks_message(self, instrument: int, sequence_number: int, ask_prices: List[int],
                               ask_volumes: List[int], bid_prices: List[int], bid_volumes: List[int]) -> None:
        """Called periodically when there is trading activity on the market.

        The five best ask (i.e. sell) and bid (i.e. buy) prices at which there
        has been trading activity are reported along with the aggregated volume
        traded at each of those price levels.

        If there are less than five prices on a side, then zeros will appear at
        the end of both the prices and volumes arrays.
        """
        self.logger.info("received trade ticks for instrument %d with sequence number %d", instrument,
                         sequence_number)
