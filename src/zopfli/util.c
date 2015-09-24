/*
Copyright 2011 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author: lode.vandevenne@gmail.com (Lode Vandevenne)
Author: jyrki.alakuijala@gmail.com (Jyrki Alakuijala)
*/

/*Modified by Felix Hanau*/

#include "util.h"
#include "zopfli.h"

#include <stdio.h>
#include <stdlib.h>

unsigned ZopfliGetDistExtraBits(unsigned dist) {
#ifdef __GNUC__
  if (dist < 5) return 0;
  return (31 ^ __builtin_clz(dist - 1)) - 1; /* log2(dist - 1) - 1 */
#else
  if (dist < 5) return 0;
  else if (dist < 9) return 1;
  else if (dist < 17) return 2;
  else if (dist < 33) return 3;
  else if (dist < 65) return 4;
  else if (dist < 129) return 5;
  else if (dist < 257) return 6;
  else if (dist < 513) return 7;
  else if (dist < 1025) return 8;
  else if (dist < 2049) return 9;
  else if (dist < 4097) return 10;
  else if (dist < 8193) return 11;
  else if (dist < 16385) return 12;
  else return 13;
#endif
}

unsigned ZopfliGetDistExtraBitsValue(unsigned dist) {
#ifdef __GNUC__
  if (dist < 5) {
    return 0;
  } else {
    int l = 31 ^ __builtin_clz(dist - 1); /* log2(dist - 1) */
    return (dist - (1 + (1 << l))) & ((1 << (l - 1)) - 1);
  }
#else
  if (dist < 5) return 0;
  else if (dist < 9) return (dist - 5) & 1;
  else if (dist < 17) return (dist - 9) & 3;
  else if (dist < 33) return (dist - 17) & 7;
  else if (dist < 65) return (dist - 33) & 15;
  else if (dist < 129) return (dist - 65) & 31;
  else if (dist < 257) return (dist - 129) & 63;
  else if (dist < 513) return (dist - 257) & 127;
  else if (dist < 1025) return (dist - 513) & 255;
  else if (dist < 2049) return (dist - 1025) & 511;
  else if (dist < 4097) return (dist - 2049) & 1023;
  else if (dist < 8193) return (dist - 4097) & 2047;
  else if (dist < 16385) return (dist - 8193) & 4095;
  else return (dist - 16385) & 8191;
#endif
}

int ZopfliGetDistSymbol(int dist) {
#ifdef __GNUC__ 
  if (dist < 5) {
    return dist - 1;
  } else {
    int l = (31 ^ __builtin_clz(dist - 1)); /* log2(dist - 1) */
    int r = ((dist - 1) >> (l - 1)) & 1;
    return l * 2 + r;
  }
#else
  if (dist < 193) {
    if (dist < 13) {  /* dist 0..13. */
      if (dist < 5) return dist - 1;
      else if (dist < 7) return 4;
      else if (dist < 9) return 5;
      else return 6;
    } else {  /* dist 13..193. */
      if (dist < 17) return 7;
      else if (dist < 25) return 8;
      else if (dist < 33) return 9;
      else if (dist < 49) return 10;
      else if (dist < 65) return 11;
      else if (dist < 97) return 12;
      else if (dist < 129) return 13;
      else return 14;
    }
  } else {
    if (dist < 2049) {  /* dist 193..2049. */
      if (dist < 257) return 15;
      else if (dist < 385) return 16;
      else if (dist < 513) return 17;
      else if (dist < 769) return 18;
      else if (dist < 1025) return 19;
      else if (dist < 1537) return 20;
      else return 21;
    } else {  /* dist 2049..32768. */
      if (dist < 3073) return 22;
      else if (dist < 4097) return 23;
      else if (dist < 6145) return 24;
      else if (dist < 8193) return 25;
      else if (dist < 12289) return 26;
      else if (dist < 16385) return 27;
      else if (dist < 24577) return 28;
      else return 29;
    }
  }
#endif
}

unsigned ZopfliGetLengthExtraBits(unsigned l) {
  static const unsigned table[259] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0
  };
  return table[l];
}

unsigned ZopfliGetLengthExtraBitsValue(unsigned l) {
  static const unsigned table[259] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 3, 0,
    1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,
    6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2,
    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0
  };
  return table[l];
}

/*
Returns symbol in range [257-285] (inclusive).
*/
unsigned ZopfliGetLengthSymbol(unsigned l) {
  static const unsigned table[259] = {
    0, 0, 0, 257, 258, 259, 260, 261, 262, 263, 264,
    265, 265, 266, 266, 267, 267, 268, 268,
    269, 269, 269, 269, 270, 270, 270, 270,
    271, 271, 271, 271, 272, 272, 272, 272,
    273, 273, 273, 273, 273, 273, 273, 273,
    274, 274, 274, 274, 274, 274, 274, 274,
    275, 275, 275, 275, 275, 275, 275, 275,
    276, 276, 276, 276, 276, 276, 276, 276,
    277, 277, 277, 277, 277, 277, 277, 277,
    277, 277, 277, 277, 277, 277, 277, 277,
    278, 278, 278, 278, 278, 278, 278, 278,
    278, 278, 278, 278, 278, 278, 278, 278,
    279, 279, 279, 279, 279, 279, 279, 279,
    279, 279, 279, 279, 279, 279, 279, 279,
    280, 280, 280, 280, 280, 280, 280, 280,
    280, 280, 280, 280, 280, 280, 280, 280,
    281, 281, 281, 281, 281, 281, 281, 281,
    281, 281, 281, 281, 281, 281, 281, 281,
    281, 281, 281, 281, 281, 281, 281, 281,
    281, 281, 281, 281, 281, 281, 281, 281,
    282, 282, 282, 282, 282, 282, 282, 282,
    282, 282, 282, 282, 282, 282, 282, 282,
    282, 282, 282, 282, 282, 282, 282, 282,
    282, 282, 282, 282, 282, 282, 282, 282,
    283, 283, 283, 283, 283, 283, 283, 283,
    283, 283, 283, 283, 283, 283, 283, 283,
    283, 283, 283, 283, 283, 283, 283, 283,
    283, 283, 283, 283, 283, 283, 283, 283,
    284, 284, 284, 284, 284, 284, 284, 284,
    284, 284, 284, 284, 284, 284, 284, 284,
    284, 284, 284, 284, 284, 284, 284, 284,
    284, 284, 284, 284, 284, 284, 284, 285
  };
  return table[l];
}

void ZopfliInitOptions(ZopfliOptions* options, unsigned mode, unsigned multithreading, unsigned isPNG) {
  if (mode < 3) {
    options->numiterations = 1;
    options->chain_length = 415;
    options->noblocksplitlz = 512;
    options->noblocksplit = 3000;
  }
  else if (mode < 4) {
    options->numiterations = 4;
    options->chain_length = 1000;
    options->noblocksplitlz = 200;
    options->noblocksplit = 2000;
  }
  else {
    options->noblocksplitlz = 100;
    options->noblocksplit = 1400;
    if (mode < 5) {
    options->numiterations = 15;
    options->chain_length = 8192;
    }
    else {
      options->numiterations = 60;
      options->chain_length = 32768;
    }
  }
  options->num = mode < 3 ? 3 : 9;
  options->skipdynamic = mode > 3 ? 80 : 180;
  options->blocksplittingmax = mode > 2 || isPNG ? 0 : multithreading > 15 ? multithreading : 15;
  options->trystatic = mode > 3 ? 800 : 0;
  options->multithreading = multithreading;
  options->isPNG = isPNG;
  options->reuse_costmodel = (!isPNG) && mode < 4 && (!multithreading);
}
