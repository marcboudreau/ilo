package main

import (
	"fmt"
	"image/png"

	"github.com/marcboudreau/ilo/dxt"
	"os"
)

func main() {
	file, err := os.Open("dxt/black.png")
	if err != nil {
		return
	}
	defer file.Close()

	img, err := png.Decode(file)

	bytes, err := dxt.Encode(img, dxt.DXT1)
	if err != nil {
		return
	}

	fmt.Printf("Resulting Bytes: %v\n", bytes)
}
