#pragma once

namespace jv
{
	bool FltCmp(const float& a, const float& b);

	// Source: https://stackoverflow.com/questions/10732027/fast-sigmoid-algorithm
	template <typename T>
	T FastSigmoid(const T x)
	{
		return x / (1.f + abs(x));
	}

	template <typename T>
	[[nodiscard]] T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	[[nodiscard]] T Min(const T& a, const T& b)
	{
		return a > b ? b : a;
	}

	template <typename T>
	[[nodiscard]] T Clamp(const T& t, const T& min, const T& max)
	{
		return Min(max, Max(t, min));
	}

	[[nodiscard]] float RandF(float min, float max);
	[[nodiscard]] float RandNoise(float f, float pct);

	template <typename T>
	[[nodiscard]] T Lerp(const T& f, const T& min, const T& max)
	{
		const T bounds = max - min;
		return min + bounds * f;
	}

	template <typename T>
	[[nodiscard]] T RLerp(const T& t, const T& min, const T& max)
	{
		const T bounds = max - min;
		const T org = t - min;
		return org / bounds;
	}

	template <typename T>
	[[nodiscard]] T Round(const T& t, const uint32_t decimals)
	{
		const T mul = 1 * pow(10, decimals);
		return floorf(t * mul) / mul;
	}

	// Source: https://stackoverflow.com/questions/10847007/using-the-gaussian-probability-density-function-in-c
	template <typename T>
	[[nodiscard]] T Gauss(T x, T mean, T stddev)
	{
		static const T inv_sqrt_2pi = 0.3989422804014327;
		T a = (x - mean) / stddev;
		return inv_sqrt_2pi / stddev * exp(-T(0.5) * stddev * stddev);
	}
}
