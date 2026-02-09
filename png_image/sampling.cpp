#include "include/decode.hpp"
#include "include/sampling.hpp"
#include <cstdint>
#include <cstdio>
#include <vector>

PNGImage down_sample(PNGImage &img, int k) {
  uint32_t w = img.width / k;
  uint32_t h = img.height / k;

  std::vector<uint8_t> output(w * h * 4);

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {

      int sum[4] = {0, 0, 0, 0};

      for (int ky = 0; ky < k; ++ky) {
        for (int kx = 0; kx < k; ++kx) {

          int srcX = x * k + kx;
          int srcY = y * k + ky;

          int idx = (srcY * img.width + srcX) * 4;

          sum[0] += img.pixels[idx + 0];
          sum[1] += img.pixels[idx + 1];
          sum[2] += img.pixels[idx + 2];
          sum[3] += img.pixels[idx + 3];
        }
      }

      int dst = (y * w + x) * 4;

      output[dst + 0] = sum[0] / (k * k);
      output[dst + 1] = sum[1] / (k * k);
      output[dst + 2] = sum[2] / (k * k);
      output[dst + 3] = sum[3] / (k * k);
    }
  }

  return PNGImage{
      .width = w,
      .height = h,
      .bitDepth = img.bitDepth,
      .colorType = img.colorType,
      .pixels = output,
  };
}

PNGImage up_sample(PNGImage &img, int k) {
  uint32_t w = img.width * k;
  uint32_t h = img.height * k;

  std::vector<uint8_t> output(w * h * 4);

  float xRatio = static_cast<float>(img.width - 1) / w;
  float yRatio = static_cast<float>(img.height - 1) / h;

  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {

      float gx = x * xRatio;
      float gy = y * yRatio;

      uint32_t x0 = static_cast<uint32_t>(gx);
      uint32_t y0 = static_cast<uint32_t>(gy);
      uint32_t x1 = std::min(x0 + 1, img.width - 1);
      uint32_t y1 = std::min(y0 + 1, img.height - 1);

      float dx = gx - x0;
      float dy = gy - y0;

      for (int c = 0; c < 4; ++c) {

        float p00 = img.pixels[(y0 * img.width + x0) * 4 + c];
        float p10 = img.pixels[(y0 * img.width + x1) * 4 + c];
        float p01 = img.pixels[(y1 * img.width + x0) * 4 + c];
        float p11 = img.pixels[(y1 * img.width + x1) * 4 + c];

        float value = p00 * (1 - dx) * (1 - dy) + p10 * dx * (1 - dy) +
                      p01 * (1 - dx) * dy + p11 * dx * dy;

        output[(y * w + x) * 4 + c] = static_cast<uint8_t>(value);
      }
    }
  }

  return PNGImage{
      .width = w,
      .height = h,
      .bitDepth = img.bitDepth,
      .colorType = img.colorType,
      .pixels = output,
  };
}

int main() {
  PNGParser png("images/image.png");
  png.parseChunks();
  png.reconstructPixels();

  PNGImage image = png.getImage();
  printf("==== Original Image ====\n");
  printf("Width = %d\nHeight = %d\nBitDepth = %d\ncolorType = %d\n",
         image.width, image.height, image.bitDepth, image.colorType);
  printf("Raw pixel data size = %zu bytes\n", image.pixels.size());

  PNGImage down_sampled_image = down_sample(image, 2);
  printf("\n==== Down Sampled Image ====\n");
  printf("Width = %d\nHeight = %d\nBitDepth = %d\ncolorType = %d\n",
         down_sampled_image.width, down_sampled_image.height,
         down_sampled_image.bitDepth, down_sampled_image.colorType);
  printf("Raw pixel data size = %zu bytes\n", down_sampled_image.pixels.size());
  writePNG("images/down_sampled_image.png", down_sampled_image);

  PNGImage up_sampled_image = up_sample(image, 2);
  printf("\n==== Up Sampled Image ====\n");
  printf("Width = %d\nHeight = %d\nBitDepth = %d\ncolorType = %d\n",
         up_sampled_image.width, up_sampled_image.height,
         up_sampled_image.bitDepth, up_sampled_image.colorType);
  printf("Raw pixel data size = %zu bytes\n", up_sampled_image.pixels.size());
  writePNG("images/up_sampled_image.png", up_sampled_image);

  return 0;
}
