cmake_minimum_required(VERSION 2.6)
project(vlan_sdn)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${NPU_LIB_DIR}")

add_executable(vlan_sdn
	src/main.c
	src/npu.c
	src/table.c
	src/tests.c
	src/parse.c
	src/match_action.c
	src/learn.c)

target_link_libraries(vlan_sdn npu)
