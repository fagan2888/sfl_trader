# -*- coding: utf-8 -*-
from queue import Queue
from threading import Thread

import smtplib
from email.header import Header
from email.mime.text import MIMEText
from email.utils import parseaddr, formataddr

from utils import get_time_str


class EmailQueue(object):
    def __init__(self):
        self.from_addr = '*****@*****.com'
        self.password = '**************'
        self.to_addr = '***********@qq.com'
        self.smtp_server = 'smtp.qq.com'
        self.msg_queue = Queue()
        self.do()

    @staticmethod
    def _format_addr(s):
        name, addr = parseaddr(s)
        return formataddr((Header(name, 'utf-8').encode(), addr))

    def put(self, msg):
        self.msg_queue.put(str(msg))

    def send(self):
        while True:
            msg = self.msg_queue.get()
            time_str = get_time_str()
            print('[send email]', time_str, msg)
            body = f"<html><body><h1>{time_str}</h1><p>{msg}</p></body></html>"
            # -------------------------------------------------------------------------------------------------
            msg = MIMEText(body, 'html', 'utf-8')
            msg['From'] = self._format_addr('TraderMonitor <%s>' % self.from_addr)
            msg['To'] = self._format_addr('<%s>' % self.to_addr)
            msg['Subject'] = Header('sfl_trader', 'utf-8').encode()
            server = smtplib.SMTP_SSL(self.smtp_server, 465)
            # server.set_debuglevel(1)
            server.login(self.from_addr, self.password)
            server.sendmail(self.from_addr, [self.to_addr], msg.as_string())
            server.quit()

    def do(self):
        t = Thread(target=self.send)
        t.daemon = True
        t.start()


if __name__ == '__main__':
    import time
    eq = EmailQueue()
    eq.put('你好测试')
    time.sleep(10)
