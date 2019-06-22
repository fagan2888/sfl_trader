#include <iostream>
#include <fstream>
#include <stdio.h>
#include "pevents.h"

using namespace neosmart;
using namespace std;

neosmart_event_t * events = NULL;
bool flag = true;
int nn;

__attribute__((constructor)) void create_events() {
    //读文件得到事件个数，创建事件
    printf("constructor flag: %d\n", flag);
    if (flag) {
        string n;
        ifstream cfg("api/base/base.cfg");
        if (cfg.is_open())
        {
            getline(cfg, n);
            getline(cfg, n);
            cfg.close();
            nn = stoi(n);
            events = new neosmart_event_t[nn];
            for (int i = 0; i < nn; i++)
                events[i] = CreateEvent(false, false);
        }
        else
        {
            printf("读取base.cfg时出错\n");
        }
        printf("n: %s\n", n.c_str());
        flag = false;
    }
}

extern "C"
{
    void api_close()
    {
        if (events != NULL) {
            delete[]events;
            cerr << "delete []events" << endl;
        }
    }

    int api_wait()
    {
        int index;
        WaitForMultipleEvents(events, nn, false, -1, index);
        return index;
    }

    void api_set(int n)
    {
        SetEvent(events[n]);
    }
}
