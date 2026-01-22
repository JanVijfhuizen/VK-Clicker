#include "pch.h"
#include "ShaderLoader.h"
#include "FileLoader.h"

VkShaderModule LoadShader(VkDevice device, const char* path)
{
    auto _ = mem::scope(TEMP);
    const auto code = mem::loadFile(TEMP, path);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	createInfo.codeSize = code.length();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.ptr());

	VkShaderModule mod;
	const auto result = vkCreateShaderModule(device, &createInfo, nullptr, &mod);
	assert(!result);
	return mod;
}
