# -*- coding: utf-8 -*-

from apscheduler.schedulers.background import BackgroundScheduler
from api.api_define import TimerCron, TimerInterval


class Timer(object):
    def __init__(self, api_base):
        self.api_base = api_base
        self.do()

    def set(self, index):
        def func():
            self.api_base.set(index)
        return func

    def do(self):
        scheduler = BackgroundScheduler()
        # 定时任务
        scheduler.add_job(func=self.set(TimerCron), trigger='cron', hour=15, minute=15)
        # 一次性任务
        # scheduler.add_job(func=self.set(TimerCron), run_date='2019-04-08 18:25:30')
        # 循环任务
        scheduler.add_job(func=self.set(TimerInterval), trigger='interval', seconds=60)
        scheduler.start()


if __name__ == '__main__':
    pass

