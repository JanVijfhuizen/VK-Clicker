#include "pch.h"
#include "QueueFamily.h"

namespace gr {
    bool QueueFamily::Complete()
    {
        for (uint32_t i = 0; i < (int)QueueType::length; i++)
        {
            if (queues[i] == -1)
                return false;
        }
        return true;
    }
}
