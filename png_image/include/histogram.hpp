#include "decode.hpp"
#include <cstdint>
#include <vector>

struct Histogram {
  std::vector<uint32_t> r;
  std::vector<uint32_t> g;
  std::vector<uint32_t> b;
};


void makeHistogram(PNGImage &img, Histogram &hist);
void dumpHistogram(Histogram &hist);
