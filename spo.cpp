#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#define MAX_DIM 2

struct result {
  float position[MAX_DIM];
  float value;
};

typedef struct result result_t;

struct particle {
  result_t current;
  result_t local_min;
  float velocity[MAX_DIM];
};

typedef struct particle particle_t;

void copy_result ( result_t& to, result_t from ) {
  for ( int i = 0; i < MAX_DIM; i ++ ) {
    to.position[i] = from.position[i];
  }
  to.value = from.value;
}

void print_result ( FILE * file, result_t r ) {
  fprintf ( file, "( %.4f, %.4f ) -> %.4f\n",
      r.position[0],
      r.position[1],
      r.value );
}

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
  result_t global_min;

  // Functions needed
  auto update_velocity = [a,b,c,&global_min](particle_t& p){
    // Random values
    std::pair<float,float> r;

    for ( int i = 0; i < MAX_DIM; i ++ ) {
      r.first = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
      r.second = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
      p.velocity[i] =
        a * p.velocity[i] +
        r.first * b * ( p.local_min.position[i] - p.current.position[i] ) +
        r.second * c * ( global_min.position[i] - p.current.position[i] );
    }
  };

  auto update_position = []( particle_t& p ) {
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      p.current.position[i] += p.velocity[i];
    }
  };

  auto f = [] ( float x, float y ) {
    return std::fabs ( x + y );
  };

  auto update_value = [f] ( particle_t& p ) {
    p.current.value = f (
        p.current.position[0],
        p.current.position[1]);
  };

  auto update_local = [] ( particle_t& p ) {
    if ( p.current.value < p.local_min.value ) {
      p.local_min.value = p.current.value;
      for ( int i = 0; i < MAX_DIM; i ++ ) {
        p.local_min.position[i] = p.current.position[i];
      }
    }
  };

  auto update_global = [&global_min] ( result_t r ) {
    if ( r.value < global_min.value ) {
      global_min.value = r.value;
      for ( int i = 0; i < MAX_DIM; i ++ ) {
        global_min.position[i] = r.position[i];
      }
    }
  };

  // Init particle set
  srand ( seed );
  auto particles = std::vector<particle_t>(n);

  for ( int j = 0; j < n; j ++ ) {
    // Initialize current result
    for ( int i = 0; i < MAX_DIM; i ++ ) {
      particles[j].current.position[i] = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
      particles[j].velocity[i] = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
    }
    // Compute f value in the position
    update_value ( particles[j] );
    // The only result is the local minimum
    copy_result ( particles[j].local_min, particles[j].current );
  }

  // Compute first global minimum
  copy_result ( global_min, particles[0].current );
  for ( int j = 0; j < n; j ++ ) {
    update_global ( particles[j].local_min );
  }
  printf ( "GLB> " );
  print_result ( stdout, global_min );

  // Sequential version
  if ( nw == 0 ) {
    for ( int i = 0; i < n_iter; i ++ ) {
      // Compute local minimums
      for ( int j = 0; j < n; j ++ ) {
        update_velocity ( particles[j] );
        update_position ( particles[j] );
        update_value ( particles[j] );
        update_local ( particles[j] );
      }
      // Update global minimum
      for ( int j = 0; j < n; j ++ ) {
        update_global ( particles[j].local_min );
      }
    }
  }
  // Parallel version
  else {
  }

  // Print of the result
  printf ( "GLB> " );
  print_result ( stdout, global_min );

  return 0;
}
