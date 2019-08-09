#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "spo.hpp"
#include "mapreduce.hpp"
#include "utimer.hpp"

void usage ( char * name ) { 
  fprintf ( stderr, "Usage: %s seed n n_iter n_w delay\n", name );
  exit ( EXIT_FAILURE );
}

int main ( int argc, char ** argv ) {
  // User arguments
  if ( argc != 6 ) {
    usage ( argv[0] );
  }
  
  auto seed   = atoi ( argv[1] );
  auto n      = atoi ( argv[2] );
  auto n_iter = atoi ( argv[3] );
  auto nw     = atoi ( argv[4] );
  auto delay  = std::chrono::microseconds( atoi ( argv[5] ) );

  // Check legal values
  if ( n <= 0 || n_iter <= 0 || nw < 0 ) {
    usage ( argv[0] );
  }

  const float a = 1;
  const float b = 1;
  const float c = 1;

  // Real function to minimize
  auto f = [delay] ( float x, float y ) {
    std::this_thread::sleep_for(delay);
    return std::fabs ( x + y );
  };

  // Comparison operator
  auto op = [] ( result_t a, result_t b ) {
    return ( a.val < b.val ) ? a : b;
  };

  // Init pseudorandom generator
  srand ( seed );

  // Init particle set
  auto p = std::vector<particle_t>(n);
  for ( int i = 0; i < n; i ++ ) {
    init_particle ( p[i] );
    p[i].cur.val = f ( p[i].cur.pos[0], p[i].cur.pos[1] );
    p[i].loc.val = p[i].cur.val;
  }

  // Debug initial particles
  // printf ( "Initial particles\n" );
  // for ( int i = 0; i < n; i ++ ) {
  //   print_result ( stdout, p[i].cur );
  // }

  // Init first global position
  result_t glb_min = p[0].loc;
  for ( int i = 0; i < n; i ++ ) {
    glb_min = op ( glb_min, p[i].loc );
  }

  // printf ( "GLB> " );
  // print_result ( stdout, glb_min );

  // Closure to update the value of a particle
  auto update = [a,b,c,f,op] ( particle_t& p, const result_t glb_min ) {
    // Velocity update
    float loc_dist[MAX_DIM];
    float glb_dist[MAX_DIM];
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      loc_dist[i] = p.loc.pos[i] - p.cur.pos[i];
      glb_dist[i] = glb_min.pos[i] - p.cur.pos[i];
    }
    update_vel ( p.vel, loc_dist, glb_dist, a, b, c );

    // Position update
    update_pos ( p.cur.pos, p.vel );

    // Value update
    p.cur.val = f ( p.cur.pos[0], p.cur.pos[1] );

    // Local minimum update
    p.loc = op ( p.cur, p.loc );
    return p.loc;
  };

  // Sequential version
  if ( nw == 0 ) {
    auto u = utimer ( "sequential" );
    for ( int i = 0; i < n_iter; i ++ ) {
      // Compute local minimums
      for ( int j = 0; j < n; j ++ ) {
        update ( p[j], glb_min );
      }
      // Update global minimum
      for ( int j = 0; j < n; j ++ ) {
        glb_min = op ( glb_min, p[j].loc );
      }
    }
  }
  // Parallel version
  else {
    auto u = utimer ( "map-reduce" );
    MapReduce<particle_t,result_t> mr ( &p, update, op, nw );
    for ( int i = 0; i < n_iter; i ++ ) {
      glb_min = mr.compute ( glb_min );
    }
    mr.stop ();
  }

  // Print of the result
  printf ( "GLB> " );
  print_result ( stdout, glb_min );

  return 0;
}
