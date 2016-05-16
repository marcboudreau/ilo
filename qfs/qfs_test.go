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

import (
    "io/ioutil"
    "testing"
)

// TestCompress verifies that Compress is correct by retrieving pairs of
// input and expected documents from the testdata directory.  The input
// documents are fed through the Compress function and the output is
// compared to the corresponding expected document.
func TestCompress(t *testing.T) {
    files, err := ioutil.ReadDir("./testdata")
    if err != nil {
        t.Errorf("Failed to read ./testdata directory. Error: %s", err)
    }
    
    for _, file := range files {
        input, err := ioutil.ReadFile("./testdata/" + file.Name() + "/input")
        if err != nil {
            t.Errorf("Failed to read test input file %s/input. Error: %s", file.Name(), err)
        }
        
        expected, err := ioutil.ReadFile("./testdata/" + file.Name() + "/expected")
        if err != nil {
           t.Errorf("Failed to read test expected output file %s/expected. Error: %s", file.Name(), err)
        }
        
        actual, err := Compress(input)
        if err != nil {
            t.Errorf("Compress returned an error. Error: %s", err)
        }
                
        if CompareByteSlices(expected, actual) {
           t.Logf("Test %s passed", file.Name())
        } else {
           t.Errorf("Test %s failed", file.Name())
        }
    }
}

func CompareByteSlices(a, b []byte) bool {
    if a == nil && b == nil {
        return true
    }
    
    if a == nil || b == nil {
        return false
    }
    
    if len(a) != len(b) {
        return false
    }
    
    for i := range a {
        if a[i] != b[i] {
            return false
        }
    }
    
    return true
}