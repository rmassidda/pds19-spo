#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include "spo.hpp"
#include "mapreduce.hpp"
#include "utimer.hpp"

void usage ( char * name ) { 
  fprintf ( stderr, "Usage: %s seed n n_iter n_w\n", name );
  exit ( EXIT_FAILURE );
}

int main ( int argc, char ** argv ) {
  // User arguments
  if ( argc != 5 ) {
    usage ( argv[0] );
  }
  
  auto seed   = atoi ( argv[1] );
  auto n      = atoi ( argv[2] );
  auto n_iter = atoi ( argv[3] );
  auto nw     = atoi ( argv[4] );

  // Check legal values
  if ( n <= 0 || n_iter <= 0 || nw < 0 ) {
    usage ( argv[0] );
  }

  const float a = 1;
  const float b = 1;
  const float c = 1;

  // Real function to minimize
  auto f = [] ( float x, float y ) {
    return  ( sqrtf ( std::fabs ( x ) ) + pow ( y, 10 ) ) * sqrtf ( std::fabs ( x * y ) );
  };

  // Comparison operator
  auto op = [] ( Result a, Result b ) {
    return ( a.value < b.value ) ? a : b;
  };

  // Init pseudorandom generator
  srand ( seed );

  // Init particle set
  auto particles = std::make_shared<std::vector<Particle>>();
  for ( int i = 0; i < n; i ++ ) {
    particles->push_back ( Particle ( f ) );
  }

  // // Debug initial particles
  // printf ( "Initial particles\n" );
  // for ( int i = 0; i < n; i ++ ) {
  //   (*particles)[i].current.print_result(stdout);
  // }
  // fflush ( stdout );

  // Init first global position
  Result glb_min;
  for ( int j = 0; j < n; j ++ ) {
    glb_min = op ( glb_min, (*particles)[j].local_min );
  }

  // Closure to update the value of a particle
  auto update = [a,b,c,f,&glb_min] ( Particle& p ) {
    // glb_min.print_result ( stdout );
    // fflush ( stdout );
    return p.update ( glb_min, a, b, c, f );
  };

  // Sequential version
  if ( nw == 0 ) {
    auto u = utimer ( "sequential" );
    for ( int i = 0; i < n_iter; i ++ ) {
      // Compute local minimums
      for ( int j = 0; j < n; j ++ ) {
        update ( (*particles)[j] );
      }
      // Update global minimum
      for ( int j = 0; j < n; j ++ ) {
        glb_min = op ( glb_min, (*particles)[j].local_min );
      }
    }
  }
  // Parallel version
  else {
    auto u = utimer ( "map-reduce" );
    MapReduce<Particle,Result> mr ( particles->size(), nw );
    for ( int i = 0; i < n_iter; i ++ ) {
      glb_min = mr.compute ( particles, update, op );
    }
    mr.stop();
  }

  // Print of the result
  printf ( "GLB> " );
  glb_min.print_result ( stdout );

  return 0;
}
