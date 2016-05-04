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

import (
  "image"
  "image/color"
  "testing"
)

type FakeImage struct {
  image.Image
  width int
  height int

  ColorFunc func (x, y int) color.Color
}

func NewFakeImage(width, height int) *FakeImage {
  img := new(FakeImage)
  img.width = width
  img.height = height
  img.ColorFunc  = func (x, y int) color.Color {
    return color.Black
  }

  return img
}

func (p *FakeImage) Bounds() image.Rectangle {
  return image.Rect(0, 0, p.width, p.height)
}

func (p *FakeImage) ColorModel() color.Model {
  return nil
}

func (p *FakeImage) At(x, y int) color.Color {
  return p.ColorFunc(x, y)
}

func TestEncodeOnlyAccepts4x4Images(t *testing.T) {
  images := []*FakeImage{ NewFakeImage(4, 4),
    NewFakeImage(0, 4),
    NewFakeImage(4, 0),
    NewFakeImage(3, 3),
    NewFakeImage(5, 4),
    NewFakeImage(4, 5) }

  if _, err := Encode(images[0], DXT1); err != nil {
    t.Errorf("Unexpected error encountered with valid size image (%v)", images[0].Bounds())
  }

  for _, v := range images[1:] {
    if _, err := Encode(v, DXT1); err == nil {
      t.Errorf("Expected error was not encountered with invalid size image (%v)", v.Bounds())
    }
  }
}

func TestEncodeWithBlackImage(t *testing.T) {
  image := NewFakeImage(4, 4)
  bytes, err := Encode(image, DXT1)
  if err != nil {
    t.Errorf("Unexpected error encountered with valid image size (%v)", image.Bounds())
  }

  if len(bytes) != 8 {
    t.Errorf("Unexpected size for result byte slice (%d)", len(bytes))
  }

  for i, v := range bytes {
    if v != byte(0) {
      t.Errorf("Unexpected result byte 0x%02x was expecting 0x00 at position %d", v, i)
    }
  }
}

func TestEncodeWithTwoColorImage(t *testing.T) {
  image := createTwoColorImage(4, 4, color.RGBA{R: 255, G: 255, B: 0, A: 255}, color.RGBA{R: 223, G: 223, B: 0, A: 255})
  bytes, err := Encode(image, DXT1)
  if err != nil {
    t.Errorf("Unexpected error encountered with valid image size (%v)", image.Bounds())
  }

  if len(bytes) != 8 {
    t.Errorf("Unexpected size for result byte slice (%d)", len(bytes))
  }

  expected := []byte{ byte(0xe0), byte(0xde), byte(0xe0), byte(0xff), byte(0x04), byte(0x04), byte(0x04), byte(0x04) }

  for i, v := range bytes {
    if v != expected[i] {
      t.Errorf("Unexpected result byte 0x%02x was expecting 0x%02x at position %d", v, expected[i], i)
    }
  }
}

func createTwoColorImage(width, height int, col1, col2 color.RGBA) *FakeImage {
  image := NewFakeImage(width, height)
  image.ColorFunc = func (x, y int) color.Color {
    if x % 2 == 0 {
      return col1
    } else {
      return col2
    }
  }

  return image
}
