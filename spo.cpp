#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include "spo.hpp"
#include "utimer.cpp"

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
    return std::fabs ( x + y );
  };

  // Comparison operator
  auto op = [] ( Result a, Result b ) {
    return ( a.value < b.value ) ? a : b;
  };

  auto update = [a,b,c,f] ( Particle& p, const Result glb_min ) {
    p.update ( glb_min, a, b, c, f );
  };

  // Init particle set
  srand ( seed );
  std::vector<Particle> particles;
  for ( int i = 0; i < n; i ++ ) {
    particles.push_back ( Particle ( f ) );
  }

  // Init first global position
  Result glb_min ( particles[0].current );
  for ( int j = 0; j < n; j ++ ) {
    glb_min = op ( glb_min, particles[j].local_min );
  }

  // Sequential version
  if ( nw == 0 ) {
    auto u = utimer ( "sequential" );
    for ( int i = 0; i < n_iter; i ++ ) {
      // Compute local minimums
      for ( int j = 0; j < n; j ++ ) {
        update ( particles[j], glb_min );
      }
      // Update global minimum
      for ( int j = 0; j < n; j ++ ) {
        glb_min = op ( glb_min, particles[j].local_min );
      }
    }
  }
  // Parallel version
  else {
  }

  // Print of the result
  printf ( "GLB> " );
  glb_min.print_result ( stdout );

  return 0;
}
