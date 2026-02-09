#pragma once
#include "decode.hpp"

PNGImage down_sample(PNGImage &img, int k);
PNGImage up_sample(PNGImage &img, int k);
