#include "include/decode.hpp"

void enhanceBrightnessContrast(PNGImage &img, float alpha, int beta) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    for (int c = 0; c < 3; c++) { // R, G, B channels
      int r = img.pixels[i + c];
      int s = static_cast<int>(alpha * r + beta);

      if (s > 255) {
        s = 255;
      }
      if (s < 0) {
        s = 0;
      }

      img.pixels[i + c] = static_cast<uint8_t>(s);
    }
    // alpha channel unchanged
  }
}

void negativeTransform(PNGImage &img) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    img.pixels[i] = 255 - img.pixels[i];         // R
    img.pixels[i + 1] = 255 - img.pixels[i + 1]; // G
    img.pixels[i + 2] = 255 - img.pixels[i + 2]; // B
  }
}

void powerTransform(PNGImage &img, float gamma) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    for (int c = 0; c < 3; c++) {

      float r = img.pixels[i + c] / 255.0f;
      float s = std::pow(r, gamma);

      int val = static_cast<int>(s * 255.0f);
      if (val > 255) {
        val = 255;
      }
      if (val < 0) {
        val = 0;
      }

      img.pixels[i + c] = static_cast<uint8_t>(val);
    }
  }
}

void contrastStretch(PNGImage &img) {
  uint8_t rmin = 255;
  uint8_t rmax = 0;

  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    for (int c = 0; c < 3; c++) {
      rmin = std::min(rmin, img.pixels[i + c]);
      rmax = std::max(rmax, img.pixels[i + c]);
    }
  }

  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    for (int c = 0; c < 3; c++) {

      int r = img.pixels[i + c];
      int s = (r - rmin) * 255 / (rmax - rmin);

      if (s > 255) {
        s = 255;
      }
      if (s < 0) {
        s = 0;
      }
      img.pixels[i + c] = static_cast<uint8_t>(s);
    }
  }
}

void thresholdTransform(PNGImage &img, uint8_t T) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {

    uint8_t gray = (img.pixels[i] + img.pixels[i + 1] + img.pixels[i + 2]) / 3;

    uint8_t val = (gray >= T) ? 255 : 0;

    img.pixels[i] = val;
    img.pixels[i + 1] = val;
    img.pixels[i + 2] = val;
  }
}

void piecewiseLinearTransform(PNGImage &img, int r1, int s1, int r2, int s2) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    for (int c = 0; c < 3; c++) {

      int r = img.pixels[i + c];
      int s;

      if (r < r1)
        s = (s1 * r) / r1;
      else if (r <= r2)
        s = ((s2 - s1) * (r - r1)) / (r2 - r1) + s1;
      else
        s = ((255 - s2) * (r - r2)) / (255 - r2) + s2;

      if (s > 255) {
        s = 255;
      }
      if (s < 0) {
        s = 0;
      }
      img.pixels[i + c] = static_cast<uint8_t>(s);
    }
  }
}

void grayLevelSlicing(PNGImage &img, int low, int high) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {

    int gray = (img.pixels[i] + img.pixels[i + 1] + img.pixels[i + 2]) / 3;

    uint8_t val;

    if (gray >= low && gray <= high)
      val = 255;
    else
      val = gray;

    img.pixels[i] = val;
    img.pixels[i + 1] = val;
    img.pixels[i + 2] = val;
  }
}

void bitPlaneSlicing(PNGImage &img, int bit) {
  for (size_t i = 0; i < img.pixels.size(); i += 4) {

    int gray = (img.pixels[i] + img.pixels[i + 1] + img.pixels[i + 2]) / 3;

    uint8_t val = ((gray >> bit) & 1) ? 255 : 0;

    img.pixels[i] = val;
    img.pixels[i + 1] = val;
    img.pixels[i + 2] = val;
  }
}

int main() {
  PNGParser parser("images/image.png");

  parser.parseChunks();
  parser.decompressIDAT();
  parser.reconstructPixels();

  PNGImage img = parser.getImage();

  // enhanceBrightnessContrast(img, 1.5f, 30);
  // writePNG("images/point_transform/brightness.png", img);

  // powerTransform(img, 0.6f); // brighten low-light image
  // writePNG("images/point_transform/gamma.png", img);

  // contrastStretch(img); // expand intensity range
  // writePNG("images/point_transform/contrast_stretch.png", img);

  // thresholdTransform(img, 120); // binary segmentation
  // writePNG("images/point_transform/threshold.png", img);

  // negativeTransform(img); // binary segmentation
  // writePNG("images/point_transform/negetive.png", img);

  // piecewiseLinearTransform(img, 70, 30, 140, 220);
  // writePNG("images/point_transform/piecewise.png", img);

  // grayLevelSlicing(img, 100, 150);
  // writePNG("images/point_transform/gray_slice.png", img);

  bitPlaneSlicing(img, 7); // extract MSB
  writePNG("images/point_transform/bitplane7.png", img);
}
