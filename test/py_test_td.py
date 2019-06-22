import time

from api.base.api_base import APIBase
from api.ctp.td.ctp_td import CTPTd
from api.api_define import *

ctp_td = CTPTd()


def print_order():
    order = ctp_td.get_order_data()
    print('----------------------------------------')
    print('--InstrumentID', order.InstrumentID)
    print('--OrderRef', order.OrderRef)
    print('--UserID', order.UserID)
    print('--OrderStatus', order.OrderStatus)
    print('--VolumeTraded', order.VolumeTraded)
    print('--VolumeTotal', order.VolumeTotal)
    print('--ZCETotalTradedVolume', order.ZCETotalTradedVolume)


if __name__ == '__main__':
    func_map = {CTPOnOrder: print_order}
    api_base = APIBase()
    api_base.start_wait(func_map)
    instrument = 'rb1910'
    print(ctp_td.so.is_api_ok())
    # order_ref = ctp_td.insert_order(instrument, CTP_D_BUY, CTP_OF_OPEN, CTP_OPT_LIMIT_PRICE, 3466.0, 1)
    # time.sleep(6)
    # ctp_td.delete_order(instrument, order_ref)
    while input() != 'q':
        pass
    api_base.close()

