#include "include/decode.hpp"
#include "include/histogram.hpp"
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

void makeHistogram(PNGImage &img, Histogram &hist) {
  for (int i = 0; i < img.pixels.size(); i += 4) {
    uint8_t r = img.pixels[i];
    uint8_t g = img.pixels[i + 1];
    uint8_t b = img.pixels[i + 2];
    hist.r[r]++;
    hist.g[g]++;
    hist.b[b]++;
  }
}

void dumpHistogram(Histogram &hist) {
  int n = 256;
  cout << "Red\n";
  for (int i = 0; i < n; i++) {
    cout << i << "," << hist.r[i] << "\n";
  }
  cout << "Green\n";
  for (int i = 0; i < n; i++) {
    cout << i << "," << hist.g[i] << "\n";
  }
  cout << "Blue\n";
  for (int i = 0; i < n; i++) {
    cout << i << "," << hist.b[i] << "\n";
  }
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
  makeHistogram(img, hist);
  dumpHistogram(hist);
}
