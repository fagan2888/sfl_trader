from ctypes import CDLL, c_int
from threading import Thread


class APIBase(object):
    def __init__(self):
        self.so = CDLL('api/base/base.so')
        self.wait = self.so.api_wait
        self.wait.argtypes = []
        self.wait.restype = c_int
        self._close = self.so.api_close
        self._close.argtypes = []
        self._set = self.so.api_set
        self._set.argtypes = [c_int]

    def _api_wait(self, func_map):
        while True:
            func_map[self.wait()]()

    def start_wait(self, func_map):
        t = Thread(target=self._api_wait, args=(func_map, ))
        t.setDaemon(True)
        t.start()

    def set(self, index):
        self._set(index)

    def close(self):
        self._close()

