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

void update_vel ( float (&vel)[MAX_DIM], float loc_dist[MAX_DIM], float glb_dist[MAX_DIM], float a, float b, float c ){
  // Random vals
  std::pair<float,float> r;
  for ( int i = 0; i < MAX_DIM; i ++ ) {
    r.first = rnd();
    r.second = rnd();
  }
  // Update vel
  for ( int i = 0; i < MAX_DIM; i ++ ) {
    vel[i] = a * vel[i];
    vel[i] += r.first * b * loc_dist[i];
    vel[i] += r.second * c * glb_dist[i];
  }
}

void update_pos ( float (&pos)[MAX_DIM], const float vel[MAX_DIM] ) {
  for ( int i = 0; i < MAX_DIM; i ++ ) {
    pos[i] += vel[i];
  }
}

void print_result ( FILE * file, result_t r ) {
  fprintf ( file, "( %.4f, %.4f ) -> %.4f\n",
      r.pos[0],
      r.pos[1],
      r.val );
}

// Random generates a position
void init_particle ( particle_t& p ) {
  for ( int i = 0; i < MAX_DIM; i++ ) {
    p.vel[i] = rnd();
    p.cur.pos[i] = rnd();
    p.loc.pos[i] = p.cur.pos[i];
  }
}
#endif
