# -*- coding: utf-8 -*-
from ctypes import Structure
from ctypes import c_char, c_double, c_int


class CTPOrderField(Structure):
    _fields_ = [
        ('BrokerID', c_char * 11),            # 经纪公司代码
        ('InvestorID', c_char * 13),          # 投资者代码
        ('InstrumentID', c_char * 31),        # 合约代码
        ('OrderRef', c_char * 13),            # 报单引用
        ('UserID', c_char * 16),              # 用户代码
        ('OrderPriceType', c_char * 1),       # 报单价格条件
        ('Direction', c_char * 1),            # 买卖方向
        ('CombOffsetFlag', c_char * 5),       # 组合开平标志
        ('CombHedgeFlag', c_char * 5),        # 组合投机套保标志
        ('LimitPrice', c_double),             # 价格
        ('VolumeTotalOriginal', c_int),       # 数量
        ('TimeCondition', c_char * 1),        # 有效期类型
        ('GTDDate', c_char * 9),              # GTD日期
        ('VolumeCondition',	c_char * 1),      # 成交量类型
        ('MinVolume', c_int),                 # 最小成交量
        ('ContingentCondition', c_char * 1),  # 触发条件
        ('StopPrice', c_double),              # 止损价
        ('ForceCloseReason', c_char * 1),     # 强平原因
        ('IsAutoSuspend', c_int),             # 自动挂起标志
        ('BusinessUnit', c_char * 21),        # 业务单元
        ('RequestID', c_int),                 # 请求编号
        ('OrderLocalID', c_char * 13),        # 本地报单编号
        ('ExchangeID', c_char * 9),           # 交易所代码
        ('ParticipantID', c_char * 11),       # 会员代码
        ('ClientID', c_char * 11),            # 客户代码
        ('ExchangeInstID', c_char * 31),      # 合约在交易所的代码
        ('TraderID', c_char * 21),            # 交易所交易员代码
        ('InstallID', c_int),                 # 安装编号
        ('OrderSubmitStatus', c_char * 1),    # 报单提交状态
        ('NotifySequence', c_int),            # 报单提示序号
        ('TradingDay', c_char * 9),           # 交易日
        ('SettlementID', c_int),              # 结算编号
        ('OrderSysID', c_char * 21),          # 报单编号
        ('OrderSource', c_char * 1),          # 报单来源
        ('OrderStatus', c_char * 1),          # 报单状态
        ('OrderType', c_char * 1),            # 报单类型
        ('VolumeTraded', c_int),              # 今成交数量
        ('VolumeTotal', c_int),               # 剩余数量
        ('InsertDate', c_char * 9),           # 报单日期
        ('InsertTime', c_char * 9),           # 委托时间
        ('ActiveTime', c_char * 9),           # 激活时间
        ('SuspendTime', c_char * 9),          # 挂起时间
        ('UpdateTime', c_char * 9),           # 最后修改时间
        ('CancelTime', c_char * 9),           # 撤销时间
        ('ActiveTraderID', c_char * 21),      # 最后修改交易所交易员代码
        ('ClearingPartID', c_char * 11),      # 结算会员编号
        ('SequenceNo', c_int),                # 序号
        ('FrontID',	c_int),                   # 前置编号
        ('SessionID', c_int),                 # 会话编号
        ('UserProductInfo', c_char * 11),     # 用户端产品信息
        ('StatusMsg', c_char * 81),           # 状态信息
        ('UserForceClose', c_int),            # 用户强评标志
        ('ActiveUserID', c_char * 16),        # 操作用户代码
        ('BrokerOrderSeq', c_int),            # 经纪公司报单编号
        ('RelativeOrderSysID', c_char * 21),  # 相关报单
        ('ZCETotalTradedVolume', c_int),      # 郑商所成交数量
        ('IsSwapOrder', c_int),               # 互换单标志
        ('BranchID', c_char * 9),             # 营业部编号
        ('InvestUnitID', c_char * 17),        # 投资单元代码
        ('AccountID', c_char * 13),           # 资金账号
        ('CurrencyID', c_char * 4),           # 币种代码
        ('IPAddress', c_char * 16),           # IP地址
        ('MacAddress', c_char * 21)           # Mac地址
    ]

