#include "pch.h"
#include "DescriptorPool.h"
#include "VkCheck.h"

namespace gr {
    DescriptorPool DescriptorPoolBuilder::Build(
        const Core& core, BindingType* types, uint32_t* sizes, uint32_t length, uint32_t maxSets)
    {
        DescriptorPool pool{};

        auto _ = mem::scope(TEMP);
        auto bindings = mem::Arr<VkDescriptorPoolSize>(TEMP, length);
        for (uint32_t i = 0; i < length; i++)
        {
            auto& poolSize = bindings[i];
            poolSize.descriptorCount = sizes[i];

            switch (types[i])
            {
            case BindingType::ubo:
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            default:
                break;
            }
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = length;
        poolInfo.pPoolSizes = bindings.ptr();
        poolInfo.maxSets = maxSets;

        gr::VkCheck(vkCreateDescriptorPool(core.device, &poolInfo, nullptr, &pool._value));
        pool._maxSets = maxSets;
        return pool;
    }
    mem::Arr<VkDescriptorSet> DescriptorPool::Alloc(ARENA arena, const Core& core, VkDescriptorSetLayout layout, uint32_t amount)
    {
        assert(arena != TEMP);

        auto layouts = mem::Arr<VkDescriptorSetLayout>(TEMP, amount);
        layouts.fill(layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _value;
        allocInfo.descriptorSetCount = amount;
        allocInfo.pSetLayouts = layouts.ptr();

        auto sets = mem::Arr<VkDescriptorSet>(arena, amount);
        gr::VkCheck(vkAllocateDescriptorSets(core.device, &allocInfo, sets.ptr()));
        return sets;
    }
    void DescriptorPool::Reset(const Core& core)
    {
        vkResetDescriptorPool(core.device, _value, 0);
    }
    void DescriptorPool::Destroy(const Core& core)
    {
        vkDestroyDescriptorPool(core.device, _value, nullptr);
    }
}