#include "include/decode.hpp"
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>

void PNGParser::dumpHex() {
  int n = this->image.pixels.size();
  for (int i = 0; i < n; i += 4) {
    if (i + 4 < n) {
      for (int j = i; j < i + 4; j++) {
        printf("%04X ", this->image.pixels[j]);
      }
      printf("\n");
    } else {
      for (int j = i; j < n; j++) {
        printf("%04X ", this->image.pixels[j]);
      }
      printf("\n");
    }
  }
}

PNGParser::PNGParser(const std::string &filename)
    : file(filename, std::ios::binary) {
  if (!file)
    throw std::runtime_error("Failed to open PNG file");
  readSignature();
}

void PNGParser::readSignature() {
  uint8_t sig[8];
  file.read(reinterpret_cast<char *>(sig), 8);
  const uint8_t png_sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
  if (memcmp(sig, png_sig, 8))
    throw std::runtime_error("Not a PNG");
}

void PNGParser::dumpChunks() {
  for (auto &c : chunks) {
    printf("Type = %s\tDataL = %zu\n", c.type.c_str(), c.data.size());
  }
}

uint32_t PNGParser::readUint32() {
  uint8_t buf[4];
  file.read(reinterpret_cast<char *>(buf), 4);
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void PNGParser::readChunk() {
  uint32_t length = readUint32();
  char type[5] = {0};
  file.read(type, 4);

  PNGChunk chunk;
  chunk.type = type;
  chunk.data.resize(length);
  file.read(reinterpret_cast<char *>(chunk.data.data()), length);

  // skip CRC
  file.seekg(4, std::ios::cur);

  chunks.push_back(std::move(chunk));
}

void PNGParser::parseChunks() {
  while (!file.eof()) {
    readChunk(); // read next chunk

    if (chunks.empty())
      break; // safety

    if (chunks.back().type == "IEND") {
      // PNG end reached
      break;
    }
  }

  if (chunks.empty() || chunks.back().type != "IEND")
    throw std::runtime_error("Invalid PNG: missing IEND chunk");

  // Optionally, extract IHDR immediately
  for (const auto &chunk : chunks) {
    if (chunk.type == "IHDR") {
      if (chunk.data.size() != 13)
        throw std::runtime_error("Invalid IHDR size");
      image.width = (chunk.data[0] << 24) | (chunk.data[1] << 16) |
                    (chunk.data[2] << 8) | chunk.data[3];
      image.height = (chunk.data[4] << 24) | (chunk.data[5] << 16) |
                     (chunk.data[6] << 8) | chunk.data[7];
      image.bitDepth = chunk.data[8];
      image.colorType = chunk.data[9];
      break;
    }
  }
}

std::vector<uint8_t> PNGParser::decompressIDAT() {
  // Combine all IDAT chunks into a single compressed buffer
  std::vector<uint8_t> compressed;
  for (const auto &chunk : chunks) {
    if (chunk.type == "IDAT") {
      compressed.insert(compressed.end(), chunk.data.begin(), chunk.data.end());
    }
  }

  if (compressed.empty())
    throw std::runtime_error("No IDAT chunks found");

  // Estimate decompressed size: (height * (width * bytesPerPixel + 1))
  // +1 per scanline for filter byte
  int bytesPerPixel = (image.colorType == 6) ? 4 : 3; // RGBA vs RGB
  std::vector<uint8_t> decompressed(image.height *
                                    (image.width * bytesPerPixel + 1));

  z_stream strm = {};
  strm.next_in = compressed.data();
  strm.avail_in = compressed.size();
  strm.next_out = decompressed.data();
  strm.avail_out = decompressed.size();

  if (inflateInit(&strm) != Z_OK)
    throw std::runtime_error("Failed to init zlib");

  int ret = inflate(&strm, Z_FINISH);
  if (ret != Z_STREAM_END)
    throw std::runtime_error("Failed to decompress IDAT");

  inflateEnd(&strm);

  return decompressed;
}

void PNGParser::reconstructPixels() {
  std::vector<uint8_t> raw = decompressIDAT();
  int bpp = (image.colorType == 6) ? 4 : 3; // bytes per pixel
  image.pixels.resize(image.width * image.height * bpp);

  int stride = image.width * bpp;
  size_t src = 0;
  size_t dst = 0;

  for (uint32_t y = 0; y < image.height; ++y) {
    uint8_t filter = raw[src++];
    for (uint32_t x = 0; x < stride; ++x) {
      uint8_t val = raw[src];
      if (filter == 0) {
        // None
        image.pixels[dst++] = val;
      } else if (filter == 1) {
        // Sub
        uint8_t left = (x >= bpp) ? image.pixels[dst - bpp] : 0;
        image.pixels[dst++] = val + left;
      } else if (filter == 2) {
        // Up
        uint8_t up = (y > 0) ? image.pixels[dst - stride] : 0;
        image.pixels[dst++] = val + up;
      } else if (filter == 3) {
        // Average
        uint8_t left = (x >= bpp) ? image.pixels[dst - bpp] : 0;
        uint8_t up = (y > 0) ? image.pixels[dst - stride] : 0;
        image.pixels[dst++] = val + ((left + up) / 2);
      } else if (filter == 4) {
        // Paeth
        uint8_t left = (x >= bpp) ? image.pixels[dst - bpp] : 0;
        uint8_t up = (y > 0) ? image.pixels[dst - stride] : 0;
        uint8_t upLeft =
            (x >= bpp && y > 0) ? image.pixels[dst - stride - bpp] : 0;
        int p = left + up - upLeft;
        int pa = abs(p - left);
        int pb = abs(p - up);
        int pc = abs(p - upLeft);
        uint8_t paeth =
            (pa <= pb && pa <= pc) ? left : (pb <= pc ? up : upLeft);
        image.pixels[dst++] = val + paeth;
      }
      ++src;
    }
  }
}

uint32_t crc32(const uint8_t *data, size_t len) {
  return ::crc32(0, data, len); // use zlib's crc32
}

void writePNG(const std::string &filename, const PNGImage &img) {
  FILE *f = fopen(filename.c_str(), "wb");
  if (!f)
    throw std::runtime_error("Cannot open file");

  // 1. PNG signature
  uint8_t sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
  fwrite(sig, 1, 8, f);

  // 2. IHDR chunk
  uint8_t ihdr[13];
  ihdr[0] = img.width >> 24;
  ihdr[1] = img.width >> 16;
  ihdr[2] = img.width >> 8;
  ihdr[3] = img.width;
  ihdr[4] = img.height >> 24;
  ihdr[5] = img.height >> 16;
  ihdr[6] = img.height >> 8;
  ihdr[7] = img.height;
  ihdr[8] = img.bitDepth;                 // usually 8
  ihdr[9] = (img.colorType == 6) ? 6 : 2; // 6=RGBA, 2=RGB
  ihdr[10] = 0;                           // compression
  ihdr[11] = 0;                           // filter
  ihdr[12] = 0;                           // interlace

  uint32_t len = htonl(13);
  fwrite(&len, 4, 1, f);
  fwrite("IHDR", 1, 4, f);
  fwrite(ihdr, 1, 13, f);
  uint8_t crcBuf[17];
  memcpy(crcBuf, "IHDR", 4);
  memcpy(crcBuf + 4, ihdr, 13);
  uint32_t crc = crc32(crcBuf, 17);
  crc = htonl(crc);
  fwrite(&crc, 4, 1, f);

  // 3. Build raw IDAT data with filter bytes
  int bpp = (img.colorType == 6) ? 4 : 3;
  std::vector<uint8_t> raw;
  raw.reserve((bpp * img.width + 1) * img.height);
  for (uint32_t y = 0; y < img.height; ++y) {
    raw.push_back(0); // filter type 0
    raw.insert(raw.end(), img.pixels.begin() + y * img.width * bpp,
               img.pixels.begin() + (y + 1) * img.width * bpp);
  }

  // 4. Compress IDAT using zlib
  uLongf compSize = compressBound(raw.size());
  std::vector<uint8_t> comp(compSize);
  if (compress(comp.data(), &compSize, raw.data(), raw.size()) != Z_OK)
    throw std::runtime_error("Failed to compress IDAT");
  comp.resize(compSize);

  // 5. Write IDAT chunk
  len = htonl(comp.size());
  fwrite(&len, 4, 1, f);
  fwrite("IDAT", 1, 4, f);
  fwrite(comp.data(), 1, comp.size(), f);
  std::vector<uint8_t> crcData(4 + comp.size());
  memcpy(crcData.data(), "IDAT", 4);
  memcpy(crcData.data() + 4, comp.data(), comp.size());
  crc = crc32(crcData.data(), crcData.size());
  crc = htonl(crc);
  fwrite(&crc, 4, 1, f);

  // 6. Write IEND chunk
  len = 0;
  fwrite(&len, 4, 1, f);
  fwrite("IEND", 1, 4, f);
  crc = crc32((const uint8_t *)"IEND", 4);
  crc = htonl(crc);
  fwrite(&crc, 4, 1, f);

  fclose(f);
}

// int main() {
//   PNGParser png("images/image.png");
//   png.parseChunks();
//   png.reconstructPixels();
//   PNGImage image = png.getImage();
//   printf("Width = %d\nHeight = %d\nBitDepth = %d\ncolorType = %d\n",
//          image.width, image.height, image.bitDepth, image.colorType);
//
//   printf("Raw pixel data size = %zu bytes\n", image.pixels.size());
//   png.dumpChunks();
//   // png.dumpHex();
//   writePNG("images/output.png", image);
//   return 0;
// }
