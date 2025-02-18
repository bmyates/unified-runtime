// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT
#include <uur/fixtures.h>

using urQueueFlushTest = uur::urQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urQueueFlushTest);

TEST_P(urQueueFlushTest, Success) {
    constexpr size_t buffer_size = 1024;
    ur_mem_handle_t buffer = nullptr;
    ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_READ_WRITE, buffer_size, nullptr, &buffer));

    std::vector<uint8_t> data(buffer_size, 42);
    ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue, buffer, /* blocking */ false, 0, 1024, data.data(), 0, nullptr, nullptr));

    ASSERT_SUCCESS(urQueueFlush(queue));
}

TEST_P(urQueueFlushTest, InvalidNullHandleQueue) { ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urQueueFlush(nullptr)); }
