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

package qfs

// #include "qfs.h"
import "C"

import (
    "fmt"
)

// Compress encodes the data contained in the input byte slice and
// returns a byte slice containing the compressed data or an error.
func Compress(input []byte) ([]byte, error) {
    output := make([]C.uchar, len(input) + 1028)
    outputlen := C.int(0)
    input2 := convertByteSliceToUcharSlice(input)
    
    code, err := C.compress_data(&input2[0], C.int(len(input)), &output[0], &outputlen)
    if err != nil {
        return nil, err
    } else if code != 0 {
        return nil, fmt.Errorf("C.compress_data returned an error code. Error: %d", code)
    }
    
    return convertUcharSliceToByteSlice(output[0:int(outputlen)]), nil
}

func convertUcharSliceToByteSlice(input []C.uchar) []byte {
    output := make([]byte, len(input))
    for i := range input {
        output[i] = byte(input[i])
    }
    
    return output
}

func convertByteSliceToUcharSlice(input []byte) []C.uchar {
    output := make([]C.uchar, len(input))
    for i := range input {
        output[i] = C.uchar(input[i])
    }
    
    return output
}

// Decompress decodes the data contained in the input byte slice and
// returns a byte slice containing the decompressed data or an error.
func Decompress(input []byte) ([]byte, error) {
    return nil, nil    
}