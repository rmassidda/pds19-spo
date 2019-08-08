#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <cmath>

#define MAX_DIM 2
#define rnd() static_cast<float> (rand()) / static_cast<float> (RAND_MAX)

class Result {
  public:
    float position[MAX_DIM];
    float value;

    // constructor in point
    Result ( const float x, const float y ) {
      position[0] = x;
      position[1] = y;
      value = std::numeric_limits<float>::infinity();
    }

    // Copy constructor
    Result ( const Result& origin ) {
      for ( int i = 0; i < MAX_DIM; i ++ ) {
        position[i] = origin.position[i];
      }
      value = origin.value;
    }

    // Update value in point 
    void update ( std::function<float(float,float)> f ) {
      value = f ( position[0], position[1] );
    }

    void print_result ( FILE * file ) {
      fprintf ( file, "( %.4f, %.4f ) -> %.4f\n",
          position[0],
          position[1],
          value );
    }

};

struct Particle {
  public:
    Result current;
    Result local_min;
    float velocity[2];

    Particle ( std::function<float(float,float)> f ) : current(rnd(),rnd()), local_min(current) {
      current.update ( f );
      local_min.update ( f );
      for ( int i = 0; i < MAX_DIM; i++ ) {
        velocity[i] = rnd();
        velocity[i] = rnd();
      }
    }

    void update_velocity ( const Result global, float a, float b, float c ){
      // Random values
      std::pair<float,float> r;
      for ( int i = 0; i < MAX_DIM; i ++ ) {
        r.first = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
        r.second = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
        velocity[i] =
          a * velocity[i] +
          r.first * b * ( local_min.position[i] - current.position[i] ) +
          r.second * c * ( global.position[i] - current.position[i] );
      }
    };

    void update_position () {
      for ( int i = 0; i < MAX_DIM; i ++ ) {
        current.position[i] += velocity[i];
      }
    }

    void update_local () {
      if ( current.value < local_min.value ) {
        local_min = Result ( current );
      }
    }

    Result update ( const Result global, float a, float b, float c, std::function<float(float,float)> f ) {
      update_velocity ( global, a, b, c );
      update_position ();
      current.update ( f );
      update_local ();
      return local_min;
    }
};
