#!/bin/bash
g++ md.cc api/base/base.so api/ctp/thostmduserapi_se.so -pthread -fPIC -shared -o md.so
