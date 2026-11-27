// SPDX-License-Identifier: Apache-2.0
// native_handle_t helpers for the out-of-tree AIDL build.
//
// libcutils is not part of the NDK, and the AIDL FMQ descriptor shim only needs the create /
// close / delete trio. The handle is a plain fd array, so the implementation is trivial.
#include <cutils/native_handle.h>

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <android/log.h>  // QTI-FIX4
#include <cerrno>
#include <dlfcn.h>

extern "C" {

native_handle_t* native_handle_create(int numFds, int numInts) {
    if (numFds < 0 || numInts < 0 || numFds > NATIVE_HANDLE_MAX_FDS ||
        numInts > NATIVE_HANDLE_MAX_INTS) {
        return nullptr;
    }
    const size_t size = sizeof(native_handle_t) + sizeof(int) * (numFds + numInts);
    auto* handle = static_cast<native_handle_t*>(std::malloc(size));
    if (handle == nullptr) return nullptr;
    handle->version = sizeof(native_handle_t);
    handle->numFds = numFds;
    handle->numInts = numInts;
    std::memset(handle->data, 0, sizeof(int) * (numFds + numInts));
    return handle;
}

int native_handle_close(const native_handle_t* handle) {
    if (handle == nullptr) return 0;
    for (int i = 0; i < handle->numFds; ++i) {
        if (handle->data[i] >= 0) ::close(handle->data[i]);
    }
    return 0;
}

int native_handle_delete(native_handle_t* handle) {
    std::free(handle);
    return 0;
}

native_handle_t* native_handle_init(char* storage, int numFds, int numInts) {
    if (storage == nullptr || numFds < 0 || numInts < 0) return nullptr;
    auto* handle = reinterpret_cast<native_handle_t*>(storage);
    handle->version = sizeof(native_handle_t);
    handle->numFds = numFds;
    handle->numInts = numInts;
    std::memset(handle->data, 0, sizeof(int) * (numFds + numInts));
    return handle;
}

native_handle_t* native_handle_clone(const native_handle_t* handle) {
    if (handle == nullptr) return nullptr;
    native_handle_t* copy = native_handle_create(handle->numFds, handle->numInts);
    if (copy != nullptr) {
        std::memcpy(copy->data, handle->data, sizeof(int) * (handle->numFds + handle->numInts));
    }
    return copy;
}

}  // extern "C"

extern "C" int android_errorWriteLog(int, const char*) { return 0; }


// ashmem_create_region/ashmem_set_prot_region are normally in libcutils, which is not part of
// the NDK. FMQ allocates its ring buffer with them; use /dev/ashmem when present and memfd
// otherwise (both are mmap-able by the framework through the native handle).
// QTI-FIX4: the platform libcutils is already mapped in every audio HAL; its allocator is the
// one SELinux and the kernel expect for FMQs (memfd with seals on recent releases).
static void* rfxCutils() {
    static void* handle = [] {
        void* h = ::dlopen("libcutils.so", RTLD_NOW | RTLD_NOLOAD);
        if (h == nullptr) h = ::dlopen("libcutils.so", RTLD_NOW);
        return h;
    }();
    return handle;
}

static int rfxFallbackCreateRegion(const char* name, size_t size);

extern "C" int ashmem_create_region(const char* name, size_t size) {
    using CreateFn = int (*)(const char*, size_t);
    if (void* h = rfxCutils()) {
        auto fn = reinterpret_cast<CreateFn>(::dlsym(h, "ashmem_create_region"));
        if (fn != nullptr && fn != &ashmem_create_region) {
            int fd = fn(name, size);
            if (fd >= 0) return fd;
            __android_log_print(ANDROID_LOG_ERROR, "ResonanceFX.Aidl",
                                "libcutils ashmem_create_region failed: errno %d", errno);
        }
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "ResonanceFX.Aidl", "libcutils not loaded: %s",
                            ::dlerror());
    }
    return rfxFallbackCreateRegion(name, size);
}

static int rfxFallbackCreateRegion(const char* name, size_t size) {
    int fd = ::open("/dev/ashmem", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "ResonanceFX.Aidl", "/dev/ashmem: errno %d",
                            errno);
    }
    if (fd >= 0) {
        char label[ASHMEM_NAME_LEN] = {};
        if (name != nullptr) std::strncpy(label, name, sizeof(label) - 1);
        if (::ioctl(fd, ASHMEM_SET_NAME, label) == 0 &&
            ::ioctl(fd, ASHMEM_SET_SIZE, size) == 0) {
            return fd;
        }
        ::close(fd);
    }
    fd = static_cast<int>(::syscall(__NR_memfd_create, "resonancefx-fmq", MFD_CLOEXEC));
    if (fd < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "ResonanceFX.Aidl", "memfd_create: errno %d",
                            errno);
        return -1;
    }
    if (::ftruncate(fd, static_cast<off_t>(size)) != 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

extern "C" int ashmem_set_prot_region(int fd, int prot) {
    using ProtFn = int (*)(int, int);  // QTI-FIX4: platform implementation first
    if (void* h = rfxCutils()) {
        auto fn = reinterpret_cast<ProtFn>(::dlsym(h, "ashmem_set_prot_region"));
        if (fn != nullptr && fn != &ashmem_set_prot_region && fd >= 0) return fn(fd, prot);
    }
    // memfd regions are normal files; there is no separate protection mask to set.
    if (fd < 0) return -1;
    (void)::ioctl(fd, ASHMEM_SET_PROT_MASK, prot);
    return 0;
}



