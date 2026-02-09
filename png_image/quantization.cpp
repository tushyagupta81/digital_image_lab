#include "include/decode.hpp"

PNGImage quantize_image(PNGImage img, int levels) {
  if (levels <= 1 || levels > 256)
    return img;

  int delta = 256 / levels;

  for (size_t i = 0; i < img.pixels.size(); i += 4) {
    img.pixels[i + 0] = (img.pixels[i + 0] / delta) * delta; // R
    img.pixels[i + 1] = (img.pixels[i + 1] / delta) * delta; // G
    img.pixels[i + 2] = (img.pixels[i + 2] / delta) * delta; // B
  }

  return img;
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

  PNGImage quantized_image = quantize_image(image, 2);
  printf("\n==== Down Sampled Image ====\n");
  printf("Width = %d\nHeight = %d\nBitDepth = %d\ncolorType = %d\n",
         quantized_image.width, quantized_image.height,
         quantized_image.bitDepth, quantized_image.colorType);
  printf("Raw pixel data size = %zu bytes\n", quantized_image.pixels.size());
  writePNG("images/quantized_image.png", quantized_image);

  return 0;
}
