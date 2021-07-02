cmake_minimum_required(VERSION 2.6)
project(lib)

include_directories("${NPU_LIB_DIR}")

add_library(npu STATIC
        src/helpers.c
        src/mac.c
        src/memory.c
        src/test_helpers.c
        src/replicator.c)
