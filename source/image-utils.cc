/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#include "image-utils.h"
#include "FreeImage.h"
#include <pthread.h>
#include <iostream>


bool write_image(const char *filename, const unsigned char *Src, int Width,
                 int Height, int channels)
{

  FreeImage_Initialise();

  FREE_IMAGE_FORMAT fif;
  int X, Y;

  int bpp = channels * 8;

  // check if the extension is supportedd

  fif = FreeImage_GetFIFFromFilename(filename);

  if (fif == FIF_UNKNOWN) {
    std::cout << "IMAGE ERROR: Freewrite_image : "
    << "failed to save " << filename
    << " with FreeImage (reason: fif == FIF_UNKNOWN)";
    return false;
  }

  // GRAY
  if (channels == 1) {
    FIBITMAP *bitmap = FreeImage_Allocate(Width, Height, bpp);

    if (!bitmap) {
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "Cannot allocate memory\n";
      return false;
    }

    unsigned char *bits = FreeImage_GetBits(bitmap);

    if (!bits) {
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "Cannot allocate memory\n";
      return false;
    }

    int pitch = FreeImage_GetPitch(bitmap);

    int step = bpp / 8;
    for (Y = Height - 1; Y >= 0; Y--) {
      unsigned char *Dst = bits + pitch * Y;
      for (X = 0; X < Width; X++, Dst += step, Src += step) {
        Dst[0] = Src[0];
      }
    }

    if (!FreeImage_Save(fif, bitmap, filename, 0)) {
      FreeImage_Unload(bitmap);
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "failed to save " << filename << " with FreeImage\n";
      return false;
    }

    FreeImage_Unload(bitmap);
    std::cout << "IMAGE: Image saved : " << filename << "\n";
    return true;
  } else // RGB/RGBA
  {
    // RGB
    if (channels == 3) {
      FIBITMAP *bitmap = FreeImage_Allocate(Width, Height, 24, 0, 0, 0);

      if (!bitmap) {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
      }

      unsigned char *bits = FreeImage_GetBits(bitmap);

      if (!bits) {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
      }

      int pitch = FreeImage_GetPitch(bitmap);

      for (Y = Height - 1; Y >= 0; Y--) {
        unsigned char *Dst = bits + pitch * Y;
        for (X = 0; X < Width; X++, Dst += 3, Src += 3) {
          Dst[FI_RGBA_RED] = Src[0];
          Dst[FI_RGBA_GREEN] = Src[1];
          Dst[FI_RGBA_BLUE] = Src[2];
        }
      }

      if (!FreeImage_Save(fif, bitmap, filename, 0)) {
        FreeImage_Unload(bitmap);
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "failed to save " << filename << " with FreeImage\n";
        return false;
      }

      FreeImage_Unload(bitmap);
      std::cout << "IMAGE: Image saved : " << filename << "\n";
      return true;

    }
      // RGBA
    else if (channels == 4) {
      FIBITMAP *bitmap = FreeImage_Allocate(Width, Height, 32, 0, 0, 0);

      if (!bitmap) {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
      }

      unsigned char *bits = FreeImage_GetBits(bitmap);

      if (!bits) {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
      }

      int pitch = FreeImage_GetPitch(bitmap);

      for (Y = Height - 1; Y >= 0; Y--) {
        unsigned char *Dst = bits + pitch * Y;
        for (X = 0; X < Width; X++, Dst += 4, Src += 4) {
          Dst[FI_RGBA_RED] = Src[0];
          Dst[FI_RGBA_GREEN] = Src[1];
          Dst[FI_RGBA_BLUE] = Src[2];
          Dst[FI_RGBA_ALPHA] = Src[3];
        }
      }

      if (!FreeImage_Save(fif, bitmap, filename, 0)) {
        FreeImage_Unload(bitmap);
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "failed to save " << filename << " with FreeImage\n";
        return false;
      }

      FreeImage_Unload(bitmap);
      std::cout << "IMAGE: Image saved : " << filename << "\n";
      return true;
    } else {
      // not supported
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "unsupported number of samples (" << channels
      << ") for FreeImage\n";
      return false;
    }
  }

  FreeImage_DeInitialise();

  return false;
}

bool write_image(const std::string &filename, const uint8_t *src,
                 int width, int height, int channels)
{
  return write_image(filename.c_str(), src, width, height, channels);
}
