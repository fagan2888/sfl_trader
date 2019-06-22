#!/bin/bash
g++ td.cc api/ctp/thosttraderapi_se.so api/base/base.so -pthread -fPIC -shared -o td.so
