// Source: https://github.com/vinceallenvince/cpp-simplex-noise

// Ported from Stefan Gustavson's java implementation of Perling noise.
// Read Stefan's excellent paper for details on how this code works.
// http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
//

#ifndef __SimplexNoise__SimplexNoise__
#define __SimplexNoise__SimplexNoise__

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
using namespace std;

class SimplexNoise {
    int grad3[12][3] = {
      {1, 1, 0},
      {-1, 1, 0},
      {1, -1, 0},
      {-1, -1, 0},
      {1, 0, 1},
      {-1, 0, 1},
      {1, 0, -1},
      {-1, 0, -1},
      {0, 1, 1},
      {0, -1, 1},
      {0, 1, -1},
      {0, -1, - 1}
    };

    int p[256];
    int perm[512];
    int permMod12[512];

    int simplex[64][4] = {
      {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 0, 0, 0}, {0, 2, 3, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 2, 3, 0},
      {0, 2, 1, 3}, {0, 0, 0, 0}, {0, 3, 1, 2}, {0, 3, 2, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 3, 2, 0},
      {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
      {1, 2, 0, 3}, {0, 0, 0, 0}, {1, 3, 0, 2}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {2, 3, 0, 1}, {2, 3, 1, 0},
      {1, 0, 2, 3}, {1, 0, 3, 2}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {2, 0, 3, 1}, {0, 0, 0, 0}, {2, 1, 3, 0},
      {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
      {2, 0, 1, 3}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {3, 0, 1, 2}, {3, 0, 2, 1}, {0, 0, 0, 0}, {3, 1, 2, 0},
      {2, 1, 0, 3}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {3, 1, 0, 2}, {0, 0, 0, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
    };

  public:

    void init() {
      for (int i = 0; i < 256; ++i) {
        this->p[i] = floor((rand() % 256 + 1));
      }
      for (int i = 0; i < 512; ++i) {
        this->perm[i] = p[i & 255];
        this->permMod12[i] = perm[i] % 12;
      }
    };

    double noise(double xin, double yin);
    double dot(int g[3], double x, double y);
    float SumOctave(int num_iterations, float x, float y, float persistence, float scale);
    float SumOctaveSmooth(float num_iterations_float, float x, float y, float persistence, float scale);
    static int FastFloor(double x);

};


#endif /* defined(__SimplexNoise__SimplexNoise__) */
