//Source: https://github.com/vinceallenvince/cpp-simplex-noise
#include "SimplexNoise.hpp"
#include <iostream>

int SimplexNoise::FastFloor(double x) {
  int xi = (int)x;
  return x < xi ? xi - 1 : xi;
}

float SimplexNoise::SumOctave(int num_iterations, float x, float y, float persistence, float scale) {
  float maxAmp = 0.f;
  float amp = 1.f;
  float freq = scale;
  float noise = 0.f;

  for (int i = 0; i < num_iterations; ++i) {
    noise += this->noise(x * freq, y * freq) * amp;
    maxAmp += amp;
    amp *= persistence;
    freq *= 2.f;
  }

  //take the average value of the iterations
  noise /= maxAmp;

  return noise;
};

float SimplexNoise::SumOctaveSmooth(float num_iterations_float, float x, float y, float persistence, float scale) {

  int num_iterations_int = ceil(num_iterations_float);
  num_iterations_int = max(num_iterations_int, 1);
  float lastLevelPersistence = num_iterations_int == 1 ? 1.f : num_iterations_float - floor(num_iterations_float);
  float maxAmp = 0.f;
  float amp = 1.f;
  float freq = scale;
  float noise = 0.f;

  for (int i = 0; i < num_iterations_int; ++i) {
    if (i == num_iterations_int - 1) {
      noise += this->noise(x * freq, y * freq) * amp * lastLevelPersistence;
      maxAmp += amp * lastLevelPersistence;
    }
    else {
      noise += this->noise(x * freq, y * freq) * amp;
      maxAmp += amp;
    }

    amp *= persistence;
    freq *= 2.f;
  }

  //take the average value of the iterations
  noise /= maxAmp;

  return noise;
};

double SimplexNoise::noise(double xin, double yin) {

  double n0, n1, n2; // Noise contributions from the three corners
  double F2 = 0.5 * (sqrt(3.0) - 1.0); // Skew the input space to determine which simplex cell we're in
  double s = (xin + yin) * F2; // Hairy factor for 2D
  int i = SimplexNoise::FastFloor(xin + s);
  int j = SimplexNoise::FastFloor(yin + s);
  double G2 = (3.0 - sqrt(3.0)) / 6.0;
  double t = (i + j) * G2;
  double X0 = i - t; // Unskew the cell origin back to (x,y) space
  double Y0 = j - t;
  double x0 = xin - X0; // The x,y distances from the cell origin
  double y0 = yin - Y0;

  // For the 2D case, the simplex shape is an equilateral triangle.
  // Determine which simplex we are in.
  int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
  if (x0 > y0) {
    i1 = 1; j1 = 0; // lower triangle, XY order: (0,0)->(1,0)->(1,1)
  }
  else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    i1 = 0; j1 = 1;
  }

  // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
  // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
  // c = (3-sqrt(3))/6
  double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
  double y1 = y0 - j1 + G2;
  double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
  double y2 = y0 - 1.0 + 2.0 * G2;

  // Work out the hashed gradient indices of the three simplex corners
  int ii = i & 255;
  int jj = j & 255;
  int gi0 = this->permMod12[ii + perm[jj]];
  int gi1 = this->permMod12[ii + i1 + this->perm[jj + j1]];
  int gi2 = this->permMod12[ii + 1 + this->perm[jj + 1]];

  // Calculate the contribution from the three corners
  double t0 = 0.5 - x0 * x0 - y0 * y0;
  if (t0 < 0) {
    n0 = 0.0;
  }
  else {
    t0 *= t0;
    n0 = t0 * t0 * this->dot(grad3[gi0], x0, y0);  // (x,y) of grad3 used for 2D gradient
  }
  double t1 = 0.5 - x1 * x1 - y1 * y1;
  if (t1 < 0) {
    n1 = 0.0;
  }
  else {
    t1 *= t1;
    n1 = t1 * t1 * this->dot(grad3[gi1], x1, y1);
  }
  double t2 = 0.5 - x2 * x2 - y2 * y2;
  if (t2 < 0) {
    n2 = 0.0;
  }
  else {
    t2 *= t2;
    n2 = t2 * t2 * this->dot(grad3[gi2], x2, y2);
  }
  // Add contributions from each corner to get the final noise value.
  // The result is scaled to return values in the interval [-1,1].
  return 70.0 * (n0 + n1 + n2);
};

double SimplexNoise::dot(int g[3], double x, double y) {
  return g[0] * x + g[1] * y;
};
