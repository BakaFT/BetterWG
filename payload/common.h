#pragma once

#include <atomic>
#include "include/capi/cef_base_capi.h"

#ifndef COMMON_H
#define COMMON_H

#define CEF_STR(name, tmp, contents)                                                                                   \
    cef_string_t name = {};                                                                                            \
    const char *tmp = contents;                                                                                        \
    cef_string_from_ascii(tmp, strlen(tmp), &name)

template <typename T> struct CefRefCount : public T
{
    template <typename U> CefRefCount(const U*) noexcept : T{}, ref_(1)
    {
        T::base.size = sizeof(U);
        T::base.add_ref = _Base_AddRef;
        T::base.release = _Base_Release;
        T::base.has_one_ref = _Base_HasOneRef;
        T::base.has_at_least_one_ref = _Base_HasAtLeastOneRef;
        self_delete_ = [](void* self) noexcept { delete static_cast<U*>(self); };
    }

    CefRefCount(nullptr_t) noexcept : CefRefCount(static_cast<T*>(nullptr))
    {
    }

private:
    void (*self_delete_)(void*);
    std::atomic<size_t> ref_;

    static void CALLBACK _Base_AddRef(cef_base_ref_counted_t* _) noexcept
    {
        ++reinterpret_cast<CefRefCount*>(_)->ref_;
    }

    static int CALLBACK _Base_Release(cef_base_ref_counted_t* _) noexcept
    {
        CefRefCount* self = reinterpret_cast<CefRefCount*>(_);
        if (--self->ref_ == 0)
        {
            self->self_delete_(_);
            return 1;
        }
        return 0;
    }

    static int CALLBACK _Base_HasOneRef(cef_base_ref_counted_t* _) noexcept
    {
        return reinterpret_cast<CefRefCount*>(_)->ref_ == 1;
    }

    static int CALLBACK _Base_HasAtLeastOneRef(cef_base_ref_counted_t* _) noexcept
    {
        return reinterpret_cast<CefRefCount*>(_)->ref_ > 0;
    }
};

#endif