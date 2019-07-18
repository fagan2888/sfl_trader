# -*- coding:utf8 -*-
import time

from api.ctp.md.ctp_md import CTPMd
from api.ctp.td.ctp_td import CTPTd
from api.api_define import CTPOnDepthMarketData, CTPOnOrder, TimerInterval, TimerCron
from api.api_define import CTP_FUTURE, SHFE, DCE, CZCE, INE
from kernel import Kernel

from strategy.cta_demo import CTADemo

FUNC_MAP = {
    CTPOnDepthMarketData: Kernel.kernel_on_tick(CTP_FUTURE),
    CTPOnOrder: Kernel.kernel_on_order(CTP_FUTURE),
    TimerInterval: Kernel.kernel_on_timer,
    TimerCron: Kernel.kernel_on_cron,
}
Kernel.api_base.start_wait(FUNC_MAP)
Kernel.init_mt(CTP_FUTURE, CTPMd, CTPTd)


if __name__ == '__main__':
    demo = CTADemo('cu1910', SHFE, 10.0)
    demo.start()
    while True:
        time.sleep(600)
    # Kernel.api_base.close()

