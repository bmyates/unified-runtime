# Copyright (C) 2023 Intel Corporation
# SPDX-License-Identifier: MIT

add_subdirectory(validation)

if(UR_ENABLE_TRACING)
    add_subdirectory(tracing)
endif()
