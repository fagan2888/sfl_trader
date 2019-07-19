import time
from ctypes import c_char_p

from api.ctp.td.ctp_td import CTPTd
from api.api_define import CTP_D_BUY, CTP_OF_OPEN, CTP_OPT_LIMIT_PRICE

ctp_td = CTPTd()


if __name__ == '__main__':
    print(ctp_td.so.is_api_ok())
    exchange = c_char_p(b'SHFE')
    order_ref = ctp_td.insert_order('al1909', exchange, CTP_D_BUY, CTP_OF_OPEN, CTP_OPT_LIMIT_PRICE, 13500.0, 1)
    time.sleep(10)
    ctp_td.delete_order('al1909', exchange, order_ref)
    while input() != 'q':
        pass

