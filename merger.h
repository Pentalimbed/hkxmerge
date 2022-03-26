#pragma once
#include "headers.h"
#include <iostream>

#define LOG(str) std::cout << std::endl \
                           << str

inline bool  compareFloat(float x, float y, float epsilon = 1e-3f) { return fabs(x - y) < epsilon; }
inline float roundf(float x)
{
    float res;
    if (x >= 0.0F)
    {
        res = ceilf(x);
        if (res - x > 0.5F) res -= 1.0F;
    }
    else
    {
        res = ceilf(-x);
        if (res + x > 0.5F) res -= 1.0F;
        res = -res;
    }
    return res;
}

void process(int argc, char* argv[]);

hkResource* loadData(std::string path);
bool        checkSame(hkaAnimation* a, hkaAnimation* b);