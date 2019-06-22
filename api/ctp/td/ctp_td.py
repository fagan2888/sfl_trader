# -*- coding: utf-8 -*-
import time
from ctypes import CDLL, POINTER, cast, c_char_p, c_char, c_double, c_int, c_void_p

from .ctp_td_type import CTPOrderField


class CTPTd(object):
    def __init__(self):
        self.so = CDLL('api/ctp/td/td.so')
        while not self.so.is_api_ok():
            time.sleep(1)
        print("CTP TD API OK")
        # for i in range(10):
        #     if self.so.is_api_ok():
        #         print("CTP TD API OK")
        #         break
        #     time.sleep(0.5)
        self._insert_order = self.so.insert_order
        self._insert_order.argtypes = [c_char_p, c_char, c_char, c_char, c_double, c_int]
        self._insert_order.restype = c_int

        self._delete_order = self.so.delete_order
        self._delete_order.argtypes = [c_char_p, c_int]
        self._delete_order.restype = c_int

        self._get_order_data = self.so.get_order_data
        self._get_order_data.argtypes = []
        self._get_order_data.restype = c_void_p

        self.delete_order_count = 0

    def insert_order(self, instrument, direction, offset_flag, price_type, price, num):
        a = self._insert_order(c_char_p(instrument.encode()), direction, offset_flag, price_type, price, num)
        return a

    def delete_order(self, instrument, order_ref):
        self.delete_order_count += 1
        if self.delete_order_count <= 100:
            return self._delete_order(c_char_p(instrument.encode()), order_ref)
        else:
            print('撤单次数大于100次,不能再撤单')
            return -1

    def get_order_data(self):
        return cast(self._get_order_data(), POINTER(CTPOrderField))[0]

