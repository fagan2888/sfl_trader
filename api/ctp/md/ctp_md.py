# -*- coding: utf-8 -*-
import time
from ctypes import CDLL, POINTER, cast, c_char_p, c_void_p

from .ctp_md_type import DepthMarketData


class CTPMd(object):
    def __init__(self):
        self.index = {}
        self.so = CDLL('api/ctp/md/md.so')
        while not self.so.is_api_ok():
            time.sleep(1)
        print("CTP MD API OK")
        # for i in range(10):
        #     if self.so.is_api_ok():
        #         print("CTP MD API OK")
        #         break
        #     time.sleep(0.5)
        self._get_tick_instrument = self.so.get_tick_instrument
        self._get_tick_instrument.argtypes = []
        self._get_tick_instrument.restype = c_char_p

        self._subscribe = self.so.subscribe
        self._subscribe.argtypes = [c_char_p]

        self._unsubscribe = self.so.unsubscribe
        self._unsubscribe.argtypes = [c_char_p]

        self.get_tick_data = self.so.get_tick_data
        self.get_tick_data.argtypes = [c_char_p]
        self.get_tick_data.restype = c_void_p

    def is_ok(self):
        return self.so.is_api_ok()

    def get_tick_instrument(self):
        return self._get_tick_instrument().decode()

    def subscribe(self, instrument):
        self._subscribe(c_char_p(instrument.encode()))
        if instrument not in self.index:
            tick_data = self.get_tick_data(c_char_p(instrument.encode()))
            self.index[instrument] = cast(tick_data, POINTER(DepthMarketData))[0]

    def unsubscribe(self, instrument):
        self._unsubscribe(c_char_p(instrument.encode()))
        if instrument in self.index:
            del self.index[instrument]

    def tick(self, instrument):
        return self.index[instrument]


if __name__ == '__main__':
    pass

