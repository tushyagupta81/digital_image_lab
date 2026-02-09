#pragma once
#include <fstream>

struct PNGChunk {
  std::string type;
  std::vector<uint8_t> data;
};

struct PNGImage {
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t bitDepth = 0;
  uint8_t colorType = 0;
  std::vector<uint8_t> pixels; // raw RGBA after decoding
};

class PNGParser {
public:
  explicit PNGParser(const std::string &filename);

  // Parse all chunks
  void parseChunks();

  // Decompress IDAT chunks to raw bytes
  std::vector<uint8_t> decompressIDAT();

  // Optionally: reconstruct pixels from decompressed scanlines
  void reconstructPixels();

  // Get parsed image info
  PNGImage getImage() const { return image; }

  void dumpChunks();

  void dumpHex();

private:
  std::ifstream file;
  std::vector<PNGChunk> chunks;
  PNGImage image;

  void readSignature();
  uint32_t readUint32();
  void readChunk();
};

void writePNG(const std::string &filename, const PNGImage &img);
