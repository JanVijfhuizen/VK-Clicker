#include "pch.h"
#include "SwapChain.h"

namespace gr {
	void SwapChain::OnScopeClear()
	{
	}
	void SwapChain::Recreate()
	{
	}
	void SwapChain::Create()
	{
	}
	void SwapChain::Clear()
	{
	}
	SwapChain SwapChainBuilder::Build()
	{
		SwapChain swapChain{};

		auto _ = mem::scope(TEMP);

		auto details = TEMP_GetSwapChainSupportDetails();
		
		swapChain.Create();
		return swapChain;
	}
}