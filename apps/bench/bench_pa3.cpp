/**
 *  Copyright 2019 Mike Reed
 */

#include "bench.h"

// ShaderBench and BitmapBench are defined in bench.h

REGISTER_GBENCH(3, return new BitmapBench("apps/spock.png", "bitmap_opaque"); );
REGISTER_GBENCH(3, return new BitmapBench("apps/wheel.png", "bitmap_alpha"); );
