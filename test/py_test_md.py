import time

from api.base.api_base import APIBase
from api.ctp.md.ctp_md import CTPMd
from api.api_define import *

ctp_md = CTPMd()


def print_price():
    instrument = ctp_md.get_tick_instrument()
    tick = ctp_md.tick(instrument)
    print('++++++++++++++  ', tick.LastPrice)


if __name__ == '__main__':
    func_map = {CTPOnDepthMarketData: print_price}
    api_base = APIBase()
    api_base.start_wait(func_map)
    if ctp_md.is_ok():
        ctp_md.subscribe('cu1909')
    while input() != 'q':
        pass
    ctp_md.unsubscribe('cu1909')
    api_base.close()

