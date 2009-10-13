#!/bin/bash
env LD_PRELOAD=/home/chengwe2/local/lib/libprofiler.so CPUPROFILE=tmp/test.prof CPUPROFILE_FREQUENCY=10000 ./test_gfmesh huge.base >output3
