#include "include/decode.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

void get_var(vector<uint32_t> r, vector<uint32_t> g, vector<uint32_t> b) {
  double r_mean = 0, g_mean = 0, b_mean = 0;
  int n = r.size();
  for (int i = 0; i < n; i++) {
    r_mean += r[i];
    g_mean += g[i];
    b_mean += b[i];
  }
  r_mean = r_mean / n;
  g_mean = g_mean / n;
  b_mean = b_mean / n;

  double r_var = 0, g_var = 0, b_var = 0;
  for (int i = 0; i < n; i++) {
    r_var += (r[i] - r_mean) * (r[i] - r_mean);
    g_var += (g[i] - g_mean) * (g[i] - g_mean);
    b_var += (b[i] - b_mean) * (b[i] - b_mean);
  }
  r_var = r_var / n;
  g_var = g_var / n;
  b_var = b_var / n;

  cout << "R_var = " << r_var << ", G_var = " << g_var << ", B_var = " << b_var
       << endl;
}

void filter(PNGImage *img, int filter[3][3], int demo) {
  int h = img->height;
  int w = img->width;
  vector<uint32_t> r, g, b;
  for (int i = 0; i < img->pixels.size(); i += 4) {
    r.push_back(img->pixels[i]);
    g.push_back(img->pixels[i + 1]);
    b.push_back(img->pixels[i + 2]);
  }
  get_var(r, g, b);
  vector<uint32_t> r_c = r, g_c = g, b_c = b;
  int dir[9][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                   {0, 1},   {1, -1}, {1, 0},  {1, 1}};
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      double r_val = 0;
      double g_val = 0;
      double b_val = 0;
      for (auto d : dir) {
        int po = filter[d[0] + 1][d[1] + 1];
        int i_new = i + d[0], j_new = j + d[1];

        if (i_new < 0 || i_new >= h || j_new < 0 || j_new >= w) {
          continue;
        }

        r_val += po * r[i_new * w + j_new];
        g_val += po * g[i_new * w + j_new];
        b_val += po * b[i_new * w + j_new];
      }
      r_val = (1.0 / demo) * r_val;
      g_val = (1.0 / demo) * g_val;
      b_val = (1.0 / demo) * b_val;

      r_c[i * w + j] = r_val;
      g_c[i * w + j] = g_val;
      b_c[i * w + j] = b_val;
    }
  }

  get_var(r_c, g_c, b_c);

  for (int i = 0, k = 0; i < img->pixels.size(); i += 4, k++) {
    img->pixels[i] = r_c[k];
    img->pixels[i + 1] = g_c[k];
    img->pixels[i + 2] = b_c[k];
  }
}

void sharp_filter(PNGImage *img, int filter[3][3], int demo) {
  int h = img->height;
  int w = img->width;
  vector<double> r, g, b;
  for (int i = 0; i < img->pixels.size(); i += 4) {
    r.push_back(img->pixels[i]);
    g.push_back(img->pixels[i + 1]);
    b.push_back(img->pixels[i + 2]);
  }
  vector<double> r_c = r, g_c = g, b_c = b;
  int dir[9][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                   {0, 1},   {1, -1}, {1, 0},  {1, 1}};
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      double r_val = 0;
      double g_val = 0;
      double b_val = 0;
      for (auto d : dir) {
        int po = filter[d[0] + 1][d[1] + 1];
        int i_new = i + d[0], j_new = j + d[1];

        if (i_new < 0 || i_new >= h || j_new < 0 || j_new >= w) {
          continue;
        }

        r_val += po * r[i_new * w + j_new];
        g_val += po * g[i_new * w + j_new];
        b_val += po * b[i_new * w + j_new];
      }

      r_c[i * w + j] = r_val;
      g_c[i * w + j] = g_val;
      b_c[i * w + j] = b_val;
    }
  }

  vector<double> r_c2 = r, g_c2 = g, b_c2 = b;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      double r_val = 0;
      double g_val = 0;
      double b_val = 0;
      for (auto d : dir) {
        int po = filter[d[1] + 1][d[0] + 1];
        int i_new = i + d[0], j_new = j + d[1];

        if (i_new < 0 || i_new >= h || j_new < 0 || j_new >= w) {
          continue;
        }

        r_val += po * r[i_new * w + j_new];
        g_val += po * g[i_new * w + j_new];
        b_val += po * b[i_new * w + j_new];
      }

      r_c2[i * w + j] = r_val;
      g_c2[i * w + j] = g_val;
      b_c2[i * w + j] = b_val;
    }
  }

  double max_val = 0;
  for (int k = 0; k < r_c.size(); k++) {
    double r_val = sqrt(r_c[k] * r_c[k] + r_c2[k] * r_c2[k]);
    double g_val = sqrt(g_c[k] * g_c[k] + g_c2[k] * g_c2[k]);
    double b_val = sqrt(b_c[k] * b_c[k] + b_c2[k] * b_c2[k]);

    max_val = max(max_val, r_val);
    max_val = max(max_val, g_val);
    max_val = max(max_val, b_val);
  }

  if (max_val == 0)
    max_val = 1;

  for (int i = 0, k = 0; i < img->pixels.size(); i += 4, k++) {
    double r_val = sqrt(r_c[k] * r_c[k] + r_c2[k] * r_c2[k]);
    double g_val = sqrt(g_c[k] * g_c[k] + g_c2[k] * g_c2[k]);
    double b_val = sqrt(b_c[k] * b_c[k] + b_c2[k] * b_c2[k]);

    r_val = (r_val / max_val) * 255.0;
    g_val = (g_val / max_val) * 255.0;
    b_val = (b_val / max_val) * 255.0;

    img->pixels[i] = (uint8_t)r_val;
    img->pixels[i + 1] = (uint8_t)g_val;
    img->pixels[i + 2] = (uint8_t)b_val;
  }
}

int main() {
  PNGParser parser("images/image.png");
  parser.parseChunks();
  parser.reconstructPixels();
  PNGImage img = parser.getImage();
  PNGImage img1, img2, img3, img4, img5;
  img1 = img2 = img3 = img4 = img5 = img;

  cout << "Mean filter\n";
  int mean_filter[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
  filter(&img, mean_filter, 9);
  writePNG("images/spatial/mean.png", img);

  cout << "Weighted Avg filter\n";
  int weighted_filter[3][3] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};
  filter(&img1, weighted_filter, 10);
  writePNG("images/spatial/weighted.png", img1);

  cout << "Gaussian filter\n";
  int gaussian_filter[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
  filter(&img2, gaussian_filter, 16);
  writePNG("images/spatial/gaussian.png", img2);

  cout << "Robert filter\n";
  int robert_filter[3][3] = {{0, 0, 0}, {0, 1, 0}, {0, 0, -1}};
  sharp_filter(&img3, robert_filter, 1);
  writePNG("images/spatial/robert.png", img3);

  cout << "Prewit filter\n";
  int prewit_filter[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
  sharp_filter(&img4, prewit_filter, 1);
  writePNG("images/spatial/prewit.png", img4);

  cout << "Sobel Avg filter\n";
  int sobel_filter[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
  sharp_filter(&img5, sobel_filter, 1);
  writePNG("images/spatial/sobel.png", img5);
}
