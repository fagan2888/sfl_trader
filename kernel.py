# -*- coding:utf8 -*-
import os
import logging
from collections import defaultdict
# from threading import Lock

from api.base.api_base import APIBase
from timer import Timer
from email_queue import EmailQueue


class Kernel(object):
    mt_map = {}
    instrument_strategy_map = defaultdict(list)
    # 策略若在不同的时段启动,要加锁
    # instrument_strategy_map_lock = Lock()
    timer_interval_strategy = []
    timer_cron_strategy = []

    api_base = APIBase()
    timer = Timer(api_base)
    email_queue = EmailQueue()

    def __init__(self, strategy_name):
        self.active = False
        self.logger = logging.getLogger(strategy_name)
        self.logger.setLevel(logging.INFO)
        handler = logging.FileHandler(os.path.join('log', strategy_name + ".log"), encoding='utf-8')
        handler.setLevel(logging.INFO)
        formatter = logging.Formatter('[%(asctime)s] %(levelname)-9s %(message)s', "%Y-%m-%d %H:%M:%S")
        handler.setFormatter(formatter)
        self.logger.addHandler(handler)

    @staticmethod
    def init_mt(label, md, td):
        if label not in Kernel.mt_map:
            # md, td, order_ref_strategy_map
            Kernel.mt_map[label] = (md(), td(), {}, )

    def set_mt(self, instrument_list, label):
        # with Kernel.instrument_strategy_map_lock:
        for instrument in instrument_list:
            Kernel.instrument_strategy_map[instrument].append(self)
            Kernel.mt_map[label][0].subscribe(instrument)

    @staticmethod
    def get_mt(label):
        return Kernel.mt_map[label]

    def add_into_timer_interval(self):
        Kernel.timer_interval_strategy.append(self)

    def add_into_timer_cron(self):
        Kernel.timer_cron_strategy.append(self)

    def add_order_ref(self, order_ref, order_ref_map):
        order_ref_map[order_ref] = self

    @staticmethod
    def delete_order_ref(order_ref, order_ref_map):
        int_order_ref = int(order_ref)
        if int_order_ref in order_ref_map:
            del order_ref_map[int_order_ref]

    @staticmethod
    def kernel_on_timer():
        [i.on_timer() for i in Kernel.timer_interval_strategy if i.active]

    @staticmethod
    def kernel_on_cron():
        [i.on_cron() for i in Kernel.timer_cron_strategy if i.active]

    @staticmethod
    def kernel_on_tick(label):
        def func():
            instrument = Kernel.mt_map[label][0].get_tick_instrument()
            # with Kernel.instrument_strategy_map_lock:
            [i.on_tick(instrument) for i in Kernel.instrument_strategy_map[instrument] if i.active]
        return func

    @staticmethod
    def kernel_on_order(label):
        def func():
            md, td, order_ref_strategy_map = Kernel.mt_map[label]
            order_data = td.get_order_data()
            order_ref = int(order_data.OrderRef)
            if order_ref in order_ref_strategy_map:
                order_ref_strategy_map[order_ref].on_order(order_data)
        return func

    def start(self):
        pass

    def on_tick(self, instrument):
        pass

    def on_order(self, order_data):
        pass

    def on_timer(self):
        pass

    def on_cron(self):
        pass

