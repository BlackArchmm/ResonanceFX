// SPDX-License-Identifier: Apache-2.0
// Minimal stand-in for the platform's <android/binder_shell.h> (not shipped in the NDK).
// Its presence makes binder_interface_utils.h declare ICInterface::handleShellCommand, so the
// IEffect vtable built here matches the platform's android.hardware.audio.effect-V3-ndk.so.
#pragma once

#include <android/binder_ibinder.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

typedef binder_status_t (*AIBinder_handleShellCommand)(AIBinder* binder, int in, int out, int err,
                                                       const char** argv, uint32_t argc);

// Exported by the platform libbinder_ndk (API 30+); weak so the NDK link never needs it.
__attribute__((weak)) void AIBinder_Class_setHandleShellCommand(
        AIBinder_Class* clazz, AIBinder_handleShellCommand handleShellCommand);

__END_DECLS
