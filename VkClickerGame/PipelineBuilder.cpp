#include "pch.h"
#include "PipelineBuilder.h"
#include "ShaderLoader.h"
#include "VkCheck.h"

namespace gr {
    Pipeline TEMP_PipelineBuilder::Build(const Core& core, VkRenderPass renderPass)
	{
        auto _ = mem::scope(TEMP);
        auto vert = LoadShader(core.device, _vertPath);
        auto frag = LoadShader(core.device, _fragPath);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = vert;
        vertStage.pName = "main";
        vertStage.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = frag;
        fragStage.pName = "main";
        fragStage.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo stages[] = {
            vertStage,
            fragStage
        };

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(Vertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attrDesc{};
        attrDesc.location = 0;
        attrDesc.binding = 0;
        attrDesc.format = VK_FORMAT_R32G32_SFLOAT;
        attrDesc.offset = offsetof(Vertex, pos);

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = 1;
        vertexInput.pVertexAttributeDescriptions = &attrDesc;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)core.resolution.x;
        viewport.height = (float)core.resolution.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = {(uint32_t)core.resolution.x, (uint32_t)core.resolution.y };

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPushConstantRange pushConstantSize{};
        pushConstantSize.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantSize.size = _pushConstantSize;
        pushConstantSize.offset = 0;

        auto layouts = _layouts.arr(TEMP);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = layouts.length();
        pipelineLayoutInfo.pSetLayouts = layouts.ptr();
        pipelineLayoutInfo.pushConstantRangeCount = _pushConstantSize > 0;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantSize;

        VkPipelineLayout layout;
        vkCreatePipelineLayout(core.device, &pipelineLayoutInfo, nullptr, &layout);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline pipeline;
        VkCheck(vkCreateGraphicsPipelines(core.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

        vkDestroyShaderModule(core.device, frag, nullptr);
        vkDestroyShaderModule(core.device, vert, nullptr);

        Pipeline ret{};
        ret.value = pipeline;
        ret.layout = layout;
        return ret;
	}
    TEMP_PipelineBuilder& TEMP_PipelineBuilder::SetVertPath(const char* path)
	{
		_vertPath = path;
		return *this;
	}
    TEMP_PipelineBuilder& TEMP_PipelineBuilder::SetFragPath(const char* path)
	{
		_fragPath = path;
		return *this;
	}
    TEMP_PipelineBuilder& TEMP_PipelineBuilder::AddLayout(VkDescriptorSetLayout layout)
    {
        _layouts.add(TEMP) = layout;
        return *this;
    }
    TEMP_PipelineBuilder& TEMP_PipelineBuilder::SetPushConstantSize(uint32_t size)
    {
        _pushConstantSize = size;
        return *this;
    }
    void Pipeline::Destroy(const Core& core)
    {
        vkDestroyPipelineLayout(core.device, layout, nullptr);
        vkDestroyPipeline(core.device, value, nullptr);
    }
}