# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: MIT

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(hello_world)
if(UR_ENABLE_TRACING)
    add_subdirectory(collector)
endif()
