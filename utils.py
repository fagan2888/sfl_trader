# -*- coding: utf-8 -*-
import time


def get_time_str(t_format='%Y-%m-%d %H:%M:%S'):
    """获取当前时间字符串，用于记录日志
    :param t_format: 时间格式 例:'%Y-%m-%d %H:%M:%S'
    :return: 时间字符串"""
    return time.strftime(t_format, time.localtime())

