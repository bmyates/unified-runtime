// Copyright (C) 2022-2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <uur/environment.h>

int main(int argc, char **argv) {
#ifdef KERNELS_ENVIRONMENT
    auto *environment = new uur::KernelsEnvironment(argc, argv, KERNELS_DEFAULT_DIR);
#endif
#ifdef DEVICES_ENVIRONMENT
    auto *environment = new uur::DevicesEnvironment(argc, argv);
#endif
#ifdef PLATFORM_ENVIRONMENT
    auto *environment = new uur::PlatformEnvironment(argc, argv);
#endif
    ::testing::InitGoogleTest(&argc, argv);
#if defined(DEVICES_ENVIRONMENT) || defined(PLATFORM_ENVIRONMENT) || defined(KERNELS_ENVIRONMENT)
    ::testing::AddGlobalTestEnvironment(environment);
#endif
    return RUN_ALL_TESTS();
}
