#include "dispatch_wrapper.h"

static void helper_void_size_t(void *context, size_t idx)
{
    const std::function<void(size_t)> &func = *(std::function<void(size_t)> *)context;

    func(idx);
}

static void helper_void_void(void *context)
{
    const std::function<void(void)> &func = *(std::function<void(void)> *)context;

    func();
}

namespace dispatch
{
    void dispatch_async(dispatch_queue_t queue, const std::function<void(void)> &func)
    {
        dispatch_async_f(queue, (void *)&func, helper_void_void);
    }

    void dispatch_sync(dispatch_queue_t queue, const std::function<void(void)> &func)
    {
        dispatch_sync_f(queue, (void *)&func, helper_void_void);
    }

    void dispatch_apply(size_t iterations, dispatch_queue_t queue, const std::function<void(size_t)> &func)
    {
        dispatch_apply_f(iterations, queue, (void *)&func, helper_void_size_t);
    }

    void dispatch_group_async(dispatch_group_t group, dispatch_queue_t queue, const std::function<void(void)> &func)
    {
        dispatch_group_async_f(group, queue, (void *)&func, helper_void_void);
    }
};
