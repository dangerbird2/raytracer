/**
 * @file ${FILE}
 * @brief
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 *
 **/
#ifndef RAYTRACER_IMAGE_UTILS_H
#define RAYTRACER_IMAGE_UTILS_H

#include <string>

bool write_image(const char *filename, const unsigned char *Src, int Width,
                 int Height, int channels);

bool write_image(const std::string &filename, const uint8_t *src, int width,
                 int height, int channels);

#endif // RAYTRACER_IMAGE_UTILS_H
