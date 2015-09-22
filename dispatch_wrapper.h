#include <dispatch/dispatch.h>
#include <functional>
#include <mutex>

namespace dispatch
{
    void dispatch_async(dispatch_queue_t queue, const std::function<void(void)> &func);
    void dispatch_sync(dispatch_queue_t queue, const std::function<void(void)> &func);

    void dispatch_set_finalizer(dispatch_queue_t queue, const std::function<void(void *)> &func);

    void dispatch_apply(size_t iterations, dispatch_queue_t queue, const std::function<void(size_t)> &func);

    void dispatch_group_async(dispatch_group_t group, dispatch_queue_t queue, const std::function<void(void)> &func);
};
