#include "include/decode.hpp"
#include "include/histogram.hpp"
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

float ret_min(float a, float b) { return a < b ? a : b; }

Histogram hist_equal(Histogram &hist, uint8_t replace[][256]) {
  int n = 256;
  Histogram equalized_hist = {
      vector<uint32_t>(n, 0),
      vector<uint32_t>(n, 0),
      vector<uint32_t>(n, 0),
  };
  vector<float> pdf(n, 0);
  float sum = 0;
  for (int i = 0; i < n; i++) {
    sum += hist.r[i];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = (float)hist.r[i] / sum;
  }
  for (int i = 1; i < n; i++) {
    pdf[i] = pdf[i] + pdf[i - 1];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = ret_min(255, round(pdf[i] * (n - 1)));
  }
  // for(float e:pdf) {
  //   cout<<e<<endl;
  // }
  for (int i = 0; i < n; i++) {
    uint8_t p = static_cast<uint8_t>(pdf[i]);
    equalized_hist.r[p] += hist.r[i];
    replace[0][i] = p;
  }

  fill(begin(pdf), end(pdf), 0);
  sum = 0;
  for (int i = 0; i < n; i++) {
    sum += hist.g[i];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = (float)hist.g[i] / sum;
  }
  for (int i = 1; i < n; i++) {
    pdf[i] = pdf[i] + pdf[i - 1];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = ret_min(255, round(pdf[i] * (n - 1)));
  }
  for (int i = 0; i < n; i++) {
    uint8_t p = static_cast<uint8_t>(pdf[i]);
    equalized_hist.g[p] += hist.g[i];
    replace[1][i] = p;
  }

  fill(begin(pdf), end(pdf), 0);
  sum = 0;
  for (int i = 0; i < n; i++) {
    sum += hist.b[i];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = (float)hist.b[i] / sum;
  }
  for (int i = 1; i < n; i++) {
    pdf[i] = pdf[i] + pdf[i - 1];
  }
  for (int i = 0; i < n; i++) {
    pdf[i] = ret_min(255, round(pdf[i] * (n - 1)));
  }
  for (int i = 0; i < n; i++) {
    uint8_t p = static_cast<uint8_t>(pdf[i]);
    equalized_hist.b[p] += hist.b[i];
    replace[2][i] = p;
  }

  return equalized_hist;
}

int main() {
  PNGParser parser("images/new_image.png");
  parser.parseChunks();
  parser.reconstructPixels();
  PNGImage img = parser.getImage();
  Histogram hist = {
      vector<uint32_t>(256, 0),
      vector<uint32_t>(256, 0),
      vector<uint32_t>(256, 0),
  };
  uint8_t replace[3][256] = {};
  makeHistogram(img, hist);
  Histogram equalized_hist = hist_equal(hist, replace);
  dumpHistogram(equalized_hist);
  for (int i = 0; i < img.pixels.size(); i += 4) {
    img.pixels[i] = replace[0][img.pixels[i]];
    img.pixels[i + 1] = replace[1][img.pixels[i + 1]];
    img.pixels[i + 2] = replace[2][img.pixels[i + 2]];
  }
  writePNG("images/hist_equal.png", img);
}
