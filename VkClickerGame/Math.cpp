#include "pch.h"
#include "Math.h"

bool jv::FltCmp(const float& a, const float& b)
{
	return abs(a - b) < 1e-5f;
}

float jv::RandF(const float min, const float max)
{
    constexpr float RRMAX = 1.f / RAND_MAX;
    float r = RRMAX * rand();
    r *= max - min;
    return min + r;
}

float jv::RandNoise(float f, float pct)
{
    return f * (1.f + jv::RandF(-pct, pct));
}
