#include "pch.h"
#include "Queues.h"

namespace gr {
    bool Queues::Equal(uint32_t a, uint32_t b)
    {
        return queues[a] == queues[b];
    }
    bool Queues::Complete()
    {
        for (uint32_t i = 0; i < (int)Type::length; i++)
        {
            if (queues[i] == -1)
                return false;
        }
        return true;
    }
}
