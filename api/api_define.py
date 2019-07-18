# -*- coding: utf-8 -*-

from ctypes import c_char

FLOAT_MIN = 0.000001

# 事件
CTPOnOrder = 0
CTPOnDepthMarketData = 1
TimerInterval = 2
TimerCron = 3

# ExchangeID
CFFEX = 'CFFEX'  # 金融期货交易所
CZCE = 'CZCE'    # 郑商所
DCE = 'DCE'      # 大商所
INE = 'INE'      # 能源期货交易所
SHFE = 'SHFE'    # 上期所

# 买卖方向
CTP_D_BUY = c_char(b'0')
CTP_D_SELL = c_char(b'1')

# 组合开平标志
CTP_OF_OPEN = c_char(b'0')               # 开仓
CTP_OF_CLOSE = c_char(b'1')              # 平仓
CTP_OF_FORCE_CLOSE = c_char(b'2')        # 强平
CTP_OF_CLOSE_TODAY = c_char(b'3')        # 平今
CTP_OF_CLOSE_YESTERDAY = c_char(b'4')    # 平昨
CTP_OF_FORCE_OFF = c_char(b'5')          # 强减
CTP_OF_LOCAL_FORCE_CLOSE = c_char(b'6')  # 本地强平

# 报单价格条件
CTP_OPT_ANY_PRICE = c_char(b'1')                    # 任意价
CTP_OPT_LIMIT_PRICE = c_char(b'2')                  # 限价
CTP_OPT_BEST_PRICE = c_char(b'3')                   # 最优价
CTP_OPT_LAST_PRICE = c_char(b'4')                   # 最新价
CTP_OPT_LAST_PRICE_PLUS_ONE_TICKS = c_char(b'5')    # 最新价浮动上浮1个ticks
CTP_OPT_LAST_PRICE_PLUS_TWO_TICKS = c_char(b'6')    # 最新价浮动上浮2个ticks
CTP_OPT_LAST_PRICE_PLUS_THREE_TICKS = c_char(b'7')  # 最新价浮动上浮3个ticks
CTP_OPT_ASK_PRICE1 = c_char(b'8')                   # 卖一价
CTP_OPT_ASK_PRICE1_PLUS_ONE_TICKS = c_char(b'9')    # 卖一价浮动上浮1个ticks
CTP_OPT_ASK_PRICE1_PLUS_TWO_TICKS = c_char(b'A')    # 卖一价浮动上浮2个ticks
CTP_OPT_ASK_PRICE1_PLUS_THREE_TICKS = c_char(b'B')  # 卖一价浮动上浮3个ticks
CTP_OPT_BID_PRICE1 = c_char(b'C')                   # 买一价
CTP_OPT_BID_PRICE1_PLUS_ONE_TICKS = c_char(b'D')    # 买一价浮动上浮1个ticks
CTP_OPT_BID_PRICE1_PLUS_TWO_TICKS = c_char(b'E')    # 买一价浮动上浮2个ticks
CTP_OPT_BID_PRICE1_PLUS_THREE_TICKS = c_char(b'F')  # 买一价浮动上浮3个ticks
CTP_OPT_FIVE_LEVEL_PRICE = c_char(b'G')             # 五档价

# engine label
CTP_FUTURE = 0
