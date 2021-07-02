cmake_minimum_required(VERSION 2.6)
project(npu_scenarios)

set(CMAKE_C_FLAGS "-Wall -Werror")
set(NPU_LIB_DIR "${PROJECT_SOURCE_DIR}")

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose build type: None Debug Release RelWithDebInfo MinSizeRel"
            FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif (DOXYGEN_FOUND)

add_subdirectory(lib)

add_subdirectory(npu_vlan_sdn)

