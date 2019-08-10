#ifndef SPO_HPP
#define SPO_HPP
#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <cmath>

#define MAX_DIM 2
#define rnd() static_cast<float> (rand()) / static_cast<float> (RAND_MAX)

struct result {
  float pos[MAX_DIM];
  float val;
};
typedef struct result result_t;

struct particle {
  result_t cur;
  result_t loc;
  float vel[MAX_DIM];
};
typedef struct particle particle_t;

void print_result ( FILE * file, result_t r ) {
  fprintf ( file, "( %.4f, %.4f ) -> %.4f\n",
      r.pos[0],
      r.pos[1],
      r.val );
}
#endif
