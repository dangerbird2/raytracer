/**
 * @file ${FILE}
 * @brief
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 *
 **/
#include "image-utils.h"
#include <iostream>

bool write_image(const char *filename, const unsigned char *Src, int Width,
                 int Height, int channels) {

  

  return false;
}

bool write_image(const std::string &filename, const uint8_t *src, int width,
                 int height, int channels) {
  return write_image(filename.c_str(), src, width, height, channels);
}
