//===--------- ur.hpp - Unified Runtime  -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-----------------------------------------------------------------===//
#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include <ur_api.h>

template <class To, class From> To ur_cast(From Value) {
  // TODO: see if more sanity checks are possible.
  assert(sizeof(From) == sizeof(To));
  return (To)(Value);
}

template <> uint32_t inline ur_cast(uint64_t Value) {
  // Cast value and check that we don't lose any information.
  uint32_t CastedValue = (uint32_t)(Value);
  assert((uint64_t)CastedValue == Value);
  return CastedValue;
}

// TODO: promote all of the below extensions to the Unified Runtime
//       and get rid of these ZER_EXT constants.
const int UR_EXT_DEVICE_INFO_END = UR_DEVICE_INFO_FORCE_UINT32;
const int UR_EXT_DEVICE_INFO_BUILD_ON_SUBDEVICE = UR_EXT_DEVICE_INFO_END - 1;
const int UR_EXT_DEVICE_INFO_MAX_WORK_GROUPS_3D = UR_EXT_DEVICE_INFO_END - 2;
// const int UR_DEVICE_INFO_ATOMIC_MEMORY_SCOPE_CAPABILITIES =
//     UR_EXT_DEVICE_INFO_END - 3;
// const int ZER_EXT_DEVICE_INFO_BFLOAT16_MATH_FUNCTIONS =
//     UR_EXT_DEVICE_INFO_END - 4;
const int UR_EXT_DEVICE_INFO_GPU_HW_THREADS_PER_EU = UR_EXT_DEVICE_INFO_END - 7;
const int UR_EXT_DEVICE_INFO_GPU_EU_COUNT_PER_SUBSLICE =
    UR_EXT_DEVICE_INFO_END - 8;
// const int UR_DEVICE_INFO_MAX_COMPUTE_QUEUE_INDICES =
//     UR_EXT_DEVICE_INFO_END - 10;
const int UR_EXT_DEVICE_INFO_MEMORY_BUS_WIDTH = UR_EXT_DEVICE_INFO_END - 11;
// const int ZER_EXT_DEVICE_INFO_MEMORY_CLOCK_RATE = UR_EXT_DEVICE_INFO_END -
// 12;
// const int ZER_EXT_DEVICE_INFO_DEVICE_ID = UR_EXT_DEVICE_INFO_END - 14;
// const int ZER_EXT_DEVICE_INFO_IMAGE_MAX_ARRAY_SIZE =
//     UR_DEVICE_INFO_IMAGE_MAX_ARRAY_SIZE;
const int UR_EXT_DEVICE_INFO_MEM_CHANNEL_SUPPORT = UR_EXT_DEVICE_INFO_END - 15;

const ur_device_info_t UR_EXT_DEVICE_INFO_OPENCL_C_VERSION =
    (ur_device_info_t)0x103D;

const uint32_t UR_EXT_MAP_FLAG_WRITE_INVALIDATE_REGION =
    (UR_MAP_FLAG_WRITE << 1);

const int UR_EXT_RESULT_END = 0x1000;
const ur_result_t UR_EXT_RESULT_ADAPTER_SPECIFIC_ERROR =
    ur_result_t(UR_EXT_RESULT_END - 1);

const int UR_EXT_USM_CAPS_ACCESS = 1 << 0;
const int UR_EXT_USM_CAPS_ATOMIC_ACCESS = 1 << 1;
const int UR_EXT_USM_CAPS_CONCURRENT_ACCESS = 1 << 2;
const int UR_EXT_USM_CAPS_CONCURRENT_ATOMIC_ACCESS = 1 << 3;

const int UR_EXT_USM_MEM_FLAG_WRITE_COMBINED = 1 << 27;
const int UR_EXT_USM_MEM_FLAG_INITIAL_PLACEMENT_DEVICE = 1 << 28;
const int UR_EXT_USM_MEM_FLAG_INITIAL_PLACEMENT_HOST = 1 << 29;
const int UR_EXT_USM_MEM_FLAG_DEVICE_READ_ONLY = 1 << 30;

const ur_context_info_t UR_EXT_CONTEXT_INFO_ATOMIC_MEMORY_ORDER_CAPABILITIES =
    (ur_context_info_t)(UR_CONTEXT_INFO_FORCE_UINT32 - 1);

const ur_queue_info_t UR_EXT_ONEAPI_QUEUE_INFO_EMPTY =
    (ur_queue_info_t)(UR_QUEUE_INFO_SIZE + 1);

const ur_command_t UR_EXT_COMMAND_TYPE_USER =
    (ur_command_t)((uint32_t)UR_COMMAND_FORCE_UINT32 - 1);

const ur_image_channel_order_t UR_EXT_IMAGE_CHANNEL_ORDER_ABGR =
    ur_image_channel_order_t(UR_IMAGE_CHANNEL_ORDER_FORCE_UINT32 - 1);

const ur_kernel_exec_info_t UR_EXT_KERNEL_EXEC_INFO_CACHE_CONFIG =
    (ur_kernel_exec_info_t)(UR_KERNEL_EXEC_INFO_FORCE_UINT32 - 1);

typedef enum {
  // No preference for SLM or data cache.
  UR_EXT_KERNEL_EXEC_INFO_CACHE_DEFAULT = 0x0,
  // Large SLM size.
  UR_EXT_KERNEL_EXEC_INFO_CACHE_LARGE_SLM = 0x1,
  // Large General Data size.
  UR_EXT_KERNEL_EXEC_INFO_CACHE_LARGE_DATA = 0x2
} ur_kernel_cache_config;

// Terminates the process with a catastrophic error message.
[[noreturn]] inline void die(const char *Message) {
  std::cerr << "die: " << Message << std::endl;
  std::terminate();
}

// A single-threaded app has an opportunity to enable this mode to avoid
// overhead from mutex locking. Default value is 0 which means that single
// thread mode is disabled.
static const bool SingleThreadMode = [] {
  const char *Ret = std::getenv("SYCL_PI_LEVEL_ZERO_SINGLE_THREAD_MODE");
  const bool RetVal = Ret ? std::stoi(Ret) : 0;
  return RetVal;
}();

// Class which acts like shared_mutex if SingleThreadMode variable is not set.
// If SingleThreadMode variable is set then mutex operations are turned into
// nop.
class ur_shared_mutex {
  std::shared_mutex Mutex;

public:
  void lock() {
    if (!SingleThreadMode)
      Mutex.lock();
  }
  bool try_lock() { return SingleThreadMode ? true : Mutex.try_lock(); }
  void unlock() {
    if (!SingleThreadMode)
      Mutex.unlock();
  }

  void lock_shared() {
    if (!SingleThreadMode)
      Mutex.lock_shared();
  }
  bool try_lock_shared() {
    return SingleThreadMode ? true : Mutex.try_lock_shared();
  }
  void unlock_shared() {
    if (!SingleThreadMode)
      Mutex.unlock_shared();
  }
};

// Class which acts like std::mutex if SingleThreadMode variable is not set.
// If SingleThreadMode variable is set then mutex operations are turned into
// nop.
class ur_mutex {
  std::mutex Mutex;

public:
  void lock() {
    if (!SingleThreadMode)
      Mutex.lock();
  }
  bool try_lock() { return SingleThreadMode ? true : Mutex.try_lock(); }
  void unlock() {
    if (!SingleThreadMode)
      Mutex.unlock();
  }
};

/// SpinLock is a synchronization primitive, that uses atomic variable and
/// causes thread trying acquire lock wait in loop while repeatedly check if
/// the lock is available.
///
/// One important feature of this implementation is that std::atomic<bool> can
/// be zero-initialized. This allows SpinLock to have trivial constructor and
/// destructor, which makes it possible to use it in global context (unlike
/// std::mutex, that doesn't provide such guarantees).
class SpinLock {
public:
  void lock() {
    while (MLock.test_and_set(std::memory_order_acquire))
      std::this_thread::yield();
  }
  void unlock() { MLock.clear(std::memory_order_release); }

private:
  std::atomic_flag MLock = ATOMIC_FLAG_INIT;
};

// The wrapper for immutable data.
// The data is initialized only once at first access (via ->) with the
// initialization function provided in Init. All subsequent access to
// the data just returns the already stored data.
//
template <class T> struct ZeCache : private T {
  // The initialization function takes a reference to the data
  // it is going to initialize, since it is private here in
  // order to disallow access other than through "->".
  //
  using InitFunctionType = std::function<void(T &)>;
  InitFunctionType Compute{nullptr};
  std::once_flag Computed;

  ZeCache() : T{} {}

  // Access to the fields of the original T data structure.
  T *operator->() {
    std::call_once(Computed, Compute, static_cast<T &>(*this));
    return this;
  }
};

// Helper for one-liner validation
#define UR_ASSERT(condition, error)                                            \
  if (!(condition))                                                            \
    return error;

// TODO: populate with target agnostic handling of UR platforms
struct _ur_platform {};

// Controls tracing UR calls from within the UR itself.
extern bool PrintTrace;

// Apparatus for maintaining immutable cache of platforms.
//
// Note we only create a simple pointer variables such that C++ RT won't
// deallocate them automatically at the end of the main program.
// The heap memory allocated for these global variables reclaimed only at
// explicit tear-down.
extern std::vector<ur_platform_handle_t> *PiPlatformsCache;
extern SpinLock *PiPlatformsCacheMutex;
extern bool PiPlatformCachePopulated;

// The getInfo*/ReturnHelper facilities provide shortcut way of
// writing return bytes for the various getInfo APIs.
template <typename T, typename Assign>
ur_result_t getInfoImpl(size_t param_value_size, void *param_value,
                        size_t *param_value_size_ret, T value,
                        size_t value_size, Assign &&assign_func) {

  if (param_value != nullptr) {

    if (param_value_size < value_size) {
      return UR_RESULT_ERROR_INVALID_VALUE;
    }

    assign_func(param_value, value, value_size);
  }

  if (param_value_size_ret != nullptr) {
    *param_value_size_ret = value_size;
  }

  return UR_RESULT_SUCCESS;
}

template <typename T>
ur_result_t getInfo(size_t param_value_size, void *param_value,
                    size_t *param_value_size_ret, T value) {

  auto assignment = [](void *param_value, T value, size_t value_size) {
    std::ignore = value_size;
    *static_cast<T *>(param_value) = value;
  };

  return getInfoImpl(param_value_size, param_value, param_value_size_ret, value,
                     sizeof(T), assignment);
}

template <typename T>
ur_result_t getInfoArray(size_t array_length, size_t param_value_size,
                         void *param_value, size_t *param_value_size_ret,
                         const T *value) {
  return getInfoImpl(param_value_size, param_value, param_value_size_ret, value,
                     array_length * sizeof(T), memcpy);
}

template <typename T, typename RetType>
ur_result_t getInfoArray(size_t array_length, size_t param_value_size,
                         void *param_value, size_t *param_value_size_ret,
                         const T *value) {
  if (param_value) {
    memset(param_value, 0, param_value_size);
    for (uint32_t I = 0; I < array_length; I++)
      ((RetType *)param_value)[I] = (RetType)value[I];
  }
  if (param_value_size_ret)
    *param_value_size_ret = array_length * sizeof(RetType);
  return UR_RESULT_SUCCESS;
}

template <>
inline ur_result_t
getInfo<const char *>(size_t param_value_size, void *param_value,
                      size_t *param_value_size_ret, const char *value) {
  return getInfoArray(strlen(value) + 1, param_value_size, param_value,
                      param_value_size_ret, value);
}

class UrReturnHelper {
public:
  UrReturnHelper(size_t param_value_size, void *param_value,
                 size_t *param_value_size_ret)
      : param_value_size(param_value_size), param_value(param_value),
        param_value_size_ret(param_value_size_ret) {}

  // A version where in/out info size is represented by a single pointer
  // to a value which is updated on return
  UrReturnHelper(size_t *param_value_size, void *param_value)
      : param_value_size(*param_value_size), param_value(param_value),
        param_value_size_ret(param_value_size) {}

  // Scalar return value
  template <class T> ur_result_t operator()(const T &t) {
    return getInfo(param_value_size, param_value, param_value_size_ret, t);
  }

  // Array return value
  template <class T> ur_result_t operator()(const T *t, size_t s) {
    return getInfoArray(s, param_value_size, param_value, param_value_size_ret,
                        t);
  }

  // Array return value where element type is differrent from T
  template <class RetType, class T>
  ur_result_t operator()(const T *t, size_t s) {
    return getInfoArray<T, RetType>(s, param_value_size, param_value,
                                    param_value_size_ret, t);
  }

protected:
  size_t param_value_size;
  void *param_value;
  size_t *param_value_size_ret;
};