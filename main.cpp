#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <ff/parallel_for.hpp>

#include "spo.hpp"
#include "mapreduce.hpp"
#include "utimer.hpp"

void usage ( char * name ) { 
  fprintf ( stderr, "Usage: %s seed n n_iter p n_w [ff]\n", name );
  fprintf ( stderr, "\tseed\tused by the random number generator\n" );
  fprintf ( stderr, "\tn\tnumber of particles\n" );
  fprintf ( stderr, "\tm\tnumber of iterations\n" );
  fprintf ( stderr, "\tp\tprecision in the riemann sum (0.1, 0.01, ... )\n" );
  fprintf ( stderr, "\tn_w\tworkers 0=seq, >0=parallel\n" );
  fprintf ( stderr, "\tff\tuse fastflow framework\n" );
  exit ( EXIT_FAILURE );
}

int main ( int argc, char ** argv ) {
  // User arguments
  if ( argc < 6 ) {
    usage ( argv[0] );
  }
  
  auto seed   = atoi ( argv[1] );
  auto n      = atoi ( argv[2] );
  auto n_iter = atoi ( argv[3] );
  auto prec   = atof ( argv[4] );
  auto nw     = atoi ( argv[5] );
  auto fast_flow = ( argc > 6 ) ? true : false;

  // Check legal values
  if ( n <= 0 || n_iter <= 0 ) {
    usage ( argv[0] );
  }

  // Real function to minimize
  auto integrand = [] ( float x ) {
    return sin(x);
  };
  auto f = [prec, integrand] ( float x, float y ) {
    float integral = 0;
    float a = ( x < y ) ? x : y;
    float b = ( x < y ) ? y : x;
    while ( a < b ) {
      integral += prec * integrand ( a );
      a += prec;
    }
    integral = ( x < y ) ? integral : - integral;
    return integral;
  };

  // Comparison operator
  auto op = [] ( result_t a, result_t b ) {
    return ( a.val < b.val ) ? a : b;
  };

  // Space dimension
  const float up_limit = 4;
  const float lo_limit = 0;

  // Init pseudorandom generator
  std::mt19937 gen ( seed );
  std::uniform_real_distribution<float> dis(lo_limit,up_limit);

  // Init particle set
  auto p = std::vector<particle_t>(n);
  for ( int i = 0; i < n; i ++ ) {
    for ( int j = 0; j < MAX_DIM; j++ ) {
      p[i].vel[j] = dis ( gen ) / 4;
      p[i].cur.pos[j] = dis ( gen );
      p[i].loc.pos[j] = p[i].cur.pos[j];
    }
    p[i].cur.val = f ( p[i].cur.pos[0], p[i].cur.pos[1] );
    p[i].loc.val = p[i].cur.val;
  }

  // Init first global position
  result_t glb_min = p[0].loc;
  for ( int i = 0; i < n; i ++ ) {
    glb_min = op ( glb_min, p[i].loc );
  }

  // Debug initial particles
  // printf ( "Initial particles\n" );
  // for ( int i = 0; i < n; i ++ ) {
  //   print_result ( stdout, p[i].cur );
  // }
  // printf ( "GLB> " );
  // print_result ( stdout, glb_min );

  // Constants used in the simulation
  const float a = 1;
  const float b = 1;
  const float c = 1;

  // Closure to update the value of a particle
  auto update = [&dis,&gen,up_limit,lo_limit,a,b,c,f,op] ( particle_t& p, const result_t glb_min ) {
    // Velocity update
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      p.vel[i] = a * p.vel[i];
      p.vel[i] += dis ( gen ) * b * ( p.loc.pos[i] - p.cur.pos[i] );
      p.vel[i] += dis ( gen ) * c * ( glb_min.pos[i] - p.cur.pos[i] );
    }

    // Position update
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      p.cur.pos[i] += p.vel[i];
    }
    // Don't let the particle exit the space ( toroidal solution )
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      while ( p.cur.pos[i] < lo_limit ) {
        p.cur.pos[i] = up_limit - (lo_limit-p.cur.pos[i]);
      }
      while ( p.cur.pos[i] > up_limit ) {
        p.cur.pos[i] = lo_limit + (p.cur.pos[i]-up_limit);
      }
    }

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
  // Parallel version (C++ std)
  else if ( !fast_flow ) {
    auto u = utimer ( "mapreduce" );
    MapReduce<particle_t,result_t> mr ( &p, update, op, nw );
    for ( int i = 0; i < n_iter; i ++ ) {
      glb_min = mr.compute ( glb_min );
    }
    mr.stop ();
  }
  // Parallel version (FastFlow)
  else {
    int n = p.size();
    auto u = utimer ( "fastflow" );
    ff::ParallelForReduce<result_t> pfr(nw);
    for ( int i = 0; i < n_iter; i ++ ) {
      result_t ref = glb_min;
      pfr.parallel_reduce ( glb_min, ref, 0, n,
        [&](const long j, result_t &glb_min){
          update ( p[j], ref );
          glb_min = op ( glb_min, p[j].loc );
        },
        [&](result_t &a, const result_t b) {
          a = op ( a, b );});
    }
  }

  // Print of the result
  printf ( "GLB> " );
  print_result ( stdout, glb_min );

  return 0;
}
