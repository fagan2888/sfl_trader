# sfl_trader 一个简洁快速轻量级的可扩展的程序化交易框架
* Linux，python3，目前的交易接口只有CTP
* 采用了[Quicklib](http://www.quicklib.cn/)的底层架构，感谢Quicklib作者[wdgwz](https://github.com/wdgwz)
* 用[pevents](https://github.com/neosmart/pevents)将Quicklib使用的Windows系统的WaitForMultipleObjects移植到Linux系统，感谢pevents的作者[neosmart](https://github.com/neosmart)、[mqudsi](https://github.com/mqudsi)。

### 扩展
&emsp;&emsp;事件机制类似于WaitForMultipleObjects。目前系统有4个事件，分别为CTP的tick数据，CTP的报单回报，定时任务，循环任务。<br/>
&emsp;&emsp;如果想增加事件，首先修改base.cfg中的事件总数，然后在底层调用base.so的api_set函数抛事件，或者用api_base.py中APIBase对象的set方法抛事件。最后自定回调函数，处理事件的线程由my_main.py中的Kernel.api_base.start_wait(FUNC_MAP)启动。base.so的数据在同一个进程中是共用的(确保调用了相同目录下的base.so)，在不同的进程间不共用。<br/>
&emsp;&emsp;如果想增加交易API，建议在api目录下新建文件夹，调用api/base目录下的base.so将所有事件串联起来。
### 使用
&emsp;&emsp;CTP行情和交易接口的配置文件分别是md.cfg和td.cfg。<br/>
&emsp;&emsp;新建策略：在strategy目录下新建一个python文件，定义一个继承Kernel的策略对象，实现有关的回调函数。<br/>
&emsp;&emsp;在my_main.py中实例化策略对象，调用start方法启动策略。<br/>
&emsp;&emsp;一个例子见cta_demo.py，该例子的描述见doc目录下的cta_demo描述.docx和cta_demo状态机.pdf。<br/>
&emsp;&emsp;登录CTP交易接口时会在当前目录形成一个all_instruments.csv文件，为市场全部合约代码。<br/>
&emsp;&emsp;email_queue.py用于发送邮件通知。在策略自动运行于云服务器上，无人看守的情况下，适时发送策略运行情况是很有必要的。邮箱的配置信息在代码中。

### 作者联系方式
shifulin666@qq.com
