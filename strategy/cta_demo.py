# -*- coding: utf-8 -*-

from kernel import Kernel
from api.api_define import CTP_FUTURE, CTP_D_BUY, CTP_D_SELL, CTP_OF_OPEN, \
    CTP_OF_CLOSE_TODAY, CTP_OPT_LIMIT_PRICE
from api.api_define import FLOAT_MIN


class CTADemo(Kernel):

    def __init__(self, instrument, tick_size, threshold=2):
        super().__init__('cta_demo_' + instrument + '_' + str(threshold))
        self.instrument = instrument
        self.tick_size = tick_size
        self.threshold = threshold
        self.status = 0
        self.bid_p1 = 0
        self.bid_v1 = 0
        self.ask_p1 = 0
        self.ask_v1 = 0
        self.vol = 0
        self.conditions_long = []
        self.conditions_short = []
        self.count_long = 0
        self.count_short = 0
        self.delete_order_count = 0      # 撤单次数计数
        self.price_long = 0
        self.price_short = 0
        self.max_bid_long = 0            # 持多单期间最大买一价
        self.min_ask_short = 9999999.0   # 持空单期间最小卖一价
        self.order_ref_long = 0
        self.order_ref_short = 0
        self.md, self.td, self.order_ref_map = self.get_mt(CTP_FUTURE)
        self.tick_count = 0
        self.upper_limit_price = 0.0
        self.lower_limit_price = 0.0
        # 单个策略实例撤单次数
        self.my_delete_count = 0

    def reset(self):
        self.count_long = 0
        self.count_short = 0
        self.delete_order_count = 0  # 撤单次数计数
        self.price_long = 0
        self.price_short = 0
        self.max_bid_long = 0  # 持多单期间最大买一价
        self.min_ask_short = 9999999.0  # 持空单期间最小卖一价
        self.order_ref_long = 0
        self.order_ref_short = 0

    def start(self):
        self.set_mt([self.instrument], CTP_FUTURE)
        self.add_into_timer_interval()
        self.add_into_timer_cron()
        self.active = True

    def on_tick(self, instrument):
        self.tick_count += 1
        tick_data = self.md.tick(instrument)
        bid_p1 = tick_data.BidPrice1
        bid_v1 = tick_data.BidVolume1
        ask_p1 = tick_data.AskPrice1
        ask_v1 = tick_data.AskVolume1
        vol = tick_data.Volume
        upper_limit_price = tick_data.UpperLimitPrice
        lower_limit_price = tick_data.LowerLimitPrice
        last_price = tick_data.LastPrice
        tick_time = int(''.join(tick_data.UpdateTime.decode().split(':')))
        if self.tick_count > 5:
            tmp = self.tick_size - FLOAT_MIN
            vol_diff = vol - self.vol
            self.logger.info(f'{instrument} | {self.status} | {vol_diff} | {bid_v1} | {ask_v1} '
                             f'| {vol} | {bid_p1} | {ask_p1} | {upper_limit_price} |'
                             f' {lower_limit_price} | {last_price} | {tick_time}')
            tick_size20 = 20.0 * self.tick_size - FLOAT_MIN
            t = 90000 < tick_time < 145000
            a1 = (bid_p1 - self.bid_p1) > tmp
            b1 = (ask_p1 - self.ask_p1) > tmp
            c1 = vol_diff > (self.ask_v1 - FLOAT_MIN)
            d1 = (upper_limit_price - last_price) > tick_size20
            a2 = (self.bid_p1 - bid_p1) > tmp
            b2 = (self.ask_p1 - ask_p1) > tmp
            c2 = vol_diff > (self.bid_v1 - FLOAT_MIN)
            d2 = (last_price - lower_limit_price) > tick_size20
            self.conditions_long.append(t and a1 and b1 and c1 and d1)
            self.conditions_short.append(t and a2 and b2 and c2 and d2)
            if len(self.conditions_long) > 100:
                self.conditions_long = self.conditions_long[-100:]
            if len(self.conditions_short) > 100:
                self.conditions_short = self.conditions_short[-100:]
            if self.status == 0:
                if sum(self.conditions_long[-self.threshold:]) == self.threshold:
                    order_ref = self.td.insert_order(self.instrument, CTP_D_BUY, CTP_OF_OPEN,
                                                     CTP_OPT_LIMIT_PRICE, self.bid_p1 + self.tick_size, 1)
                    self.add_order_ref(order_ref, self.order_ref_map)
                    self.order_ref_long = order_ref
                    self.price_long = self.bid_p1
                    self.count_long = 0
                    self.max_bid_long = 0.0
                    self.status = 1
                    self.delete_order_count = 0
                    self.logger.info(f'status 0->1, order_ref:{order_ref}, price:{self.price_long}')
                elif sum(self.conditions_short[-self.threshold:]) == self.threshold:
                    order_ref = self.td.insert_order(self.instrument, CTP_D_SELL, CTP_OF_OPEN,
                                                     CTP_OPT_LIMIT_PRICE, self.ask_p1 - self.tick_size, 1)
                    self.add_order_ref(order_ref, self.order_ref_map)
                    self.price_short = self.ask_p1
                    self.order_ref_short = order_ref
                    self.count_short = 0
                    self.min_ask_short = 9999999.0
                    self.status = 2
                    self.delete_order_count = 0
                    self.logger.info(f'status 0->2, order_ref:{order_ref}, price:{self.price_short}')
            elif self.status == 1:
                self.count_long += 1
                if self.count_long >= 5:
                    self.td.delete_order(self.instrument, self.order_ref_long)
                    self.my_delete_count += 1
                    self.delete_order_count += 1
                    if self.delete_order_count > 5:
                        self.delete_order_count = 0
                        self.status = 8
            elif self.status == 2:
                self.count_short += 1
                if self.count_short >= 5:
                    self.td.delete_order(self.instrument, self.order_ref_short)
                    self.my_delete_count += 1
                    self.delete_order_count += 1
                    if self.delete_order_count > 5:
                        self.delete_order_count = 0
                        self.status = 8
            elif self.status == 3:
                if bid_p1 > self.max_bid_long:
                    self.max_bid_long = bid_p1
                if bid_p1 < max(self.price_long, self.max_bid_long) or (151000 > tick_time > 145500):
                    order_ref = self.td.insert_order(self.instrument, CTP_D_SELL, CTP_OF_CLOSE_TODAY,
                                                     CTP_OPT_LIMIT_PRICE, bid_p1, 1)
                    self.add_order_ref(order_ref, self.order_ref_map)
                    self.order_ref_long = order_ref
                    self.max_bid_long = 0.0
                    self.status = 5
                    self.count_long = 0
                    self.delete_order_count = 0
                    self.logger.info(f'status 3->5, order_ref:{order_ref}, close_price:{bid_p1}')
            elif self.status == 4:
                if ask_p1 < self.min_ask_short:
                    self.min_ask_short = ask_p1
                if ask_p1 > min(self.price_short, self.min_ask_short) or (151000 > tick_time > 145500):
                    order_ref = self.td.insert_order(self.instrument, CTP_D_BUY, CTP_OF_CLOSE_TODAY,
                                                     CTP_OPT_LIMIT_PRICE, ask_p1, 1)
                    self.add_order_ref(order_ref, self.order_ref_map)
                    self.order_ref_short = order_ref
                    self.min_ask_short = 9999999.0
                    self.status = 6
                    self.count_short = 0
                    self.delete_order_count = 0
                    self.logger.info(f'status 4->6, order_ref:{order_ref}, close_price:{ask_p1}')
            elif self.status == 5:
                self.count_long += 1
                if self.count_long >= 2:
                    self.td.delete_order(self.instrument, self.order_ref_long)
                    self.my_delete_count += 1
                    self.delete_order_count += 1
                    if self.delete_order_count > 5:
                        self.delete_order_count = 0
                        self.status = 8
            elif self.status == 6:
                self.count_short += 1
                if self.count_short >= 2:
                    self.td.delete_order(self.instrument, self.order_ref_short)
                    self.my_delete_count += 1
                    self.delete_order_count += 1
                    if self.delete_order_count > 5:
                        self.delete_order_count = 0
                        self.status = 8
        self.bid_p1 = bid_p1
        self.bid_v1 = bid_v1
        self.ask_p1 = ask_p1
        self.ask_v1 = ask_v1
        self.vol = vol
        self.upper_limit_price = upper_limit_price
        self.lower_limit_price = lower_limit_price

    def on_order(self, order_data):
        int_order_ref = int(order_data.OrderRef)
        order_status = order_data.OrderStatus
        if order_status == b'0':
            self.logger.info(f'方向{order_data.Direction.decode()} | '
                             f'开平{order_data.CombOffsetFlag.decode()} | '
                             f'成交价{order_data.LimitPrice}')
        if self.status == 1:
            if order_status == b'0':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 3
            elif order_status == b'5':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 0
        elif self.status == 2:
            if order_status == b'0':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 4
            elif order_status == b'5':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 0
        elif self.status == 5:
            if order_status == b'0':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 0
                self.reset()
            elif order_status == b'5':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                order_ref = self.td.insert_order(self.instrument, CTP_D_SELL, CTP_OF_CLOSE_TODAY,
                                                 CTP_OPT_LIMIT_PRICE, self.lower_limit_price, 1)
                self.add_order_ref(order_ref, self.order_ref_map)
                self.order_ref_long = order_ref
                self.status = 7
        elif self.status == 6:
            if order_status == b'0':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 0
                self.reset()
            elif order_status == b'5':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                order_ref = self.td.insert_order(self.instrument, CTP_D_BUY, CTP_OF_CLOSE_TODAY,
                                                 CTP_OPT_LIMIT_PRICE, self.upper_limit_price, 1)
                self.add_order_ref(order_ref, self.order_ref_map)
                self.order_ref_short = order_ref
                self.status = 7
        elif self.status == 7:
            if order_status == b'0':
                self.delete_order_ref(int_order_ref, self.order_ref_map)
                self.status = 0
                self.reset()

    def on_timer(self):
        # print("每隔60秒打印Hello World")
        self.logger.info('Hello World')

    def on_cron(self):
        # print("收盘后发送今日交易情况报告")
        self.logger.info('今日交易情况很好')
        self.email_queue.put('今日交易情况很好')

