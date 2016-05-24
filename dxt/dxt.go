/*
 * Copyright 2016 Marc Boudreau
 *
 * This file is part of ilo.
 *
 * ilo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ilo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ilo.  If not, see <http://www.gnu.org/licenses/>.
 */
package dxt

// #include "dxt.h"
import "C"

import (
  "fmt"
  "image"
  "image/color"
)

// Flag is an int type used to specify which DXT algorithm to use.
type Flag int

const (
  // DXT1 specifies the DXT-1 algorithm.
  DXT1 Flag = iota
  // DXT3 specifies the DXT-3 algorithm.
  DXT3 Flag = iota
)

// Encode encodes the provided Image using the specified DXT algorithm.  The result is
// returned as a byte slice.
func Encode(img image.Image, flag Flag) ([]byte, error) {
  if err := validateImageSize(img.Bounds()); err != nil {
    return nil, err
  }

  pixels := make([]C.ulong, 16)
  for i := 0; i < 16; i++ {
    color := img.At(i % 4, i / 4)
    pixels[i] = convertColorToCULong(color)
  }

  packed := make([]C.uchar, 8)

  if _, err := C.pack_dxt(&pixels[0], &packed[0]); err != nil {
    return nil, err
  }

  result := make([]byte, 8)
  for i := range packed {
    result[i] = byte(packed[i])
  }

  return result, nil
}

func convertColorToCULong(col color.Color) C.ulong {
  r, g, b, _ := col.RGBA()
  return C.ulong(r & 0xff << 16 | g & 0xff << 8 | b & 0xff)
}

func validateImageSize(bounds image.Rectangle) error {
  if 4 != bounds.Max.X - bounds.Min.X || 4 != bounds.Max.Y - bounds.Min.Y {
    return fmt.Errorf("The provided image is not a 4x4 pixel image.")
  }

  return nil
}