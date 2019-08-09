#ifndef MPR_HPP
#define MPR_HPP
#include <iostream>
#include <queue>
#include <thread>
#include <vector>

#include "utils.hpp"
#include "spo.hpp"

#define EOS -1
#define GO 0


template<typename Tin,typename Tout>
class MapReduce {
  using command_t = std::pair<std::function<Tout(Tin&)>,int>;
  private:
    std::vector<Tin> map_input;
    queue<Tout> map_output;
    int nw;
    std::vector<std::unique_ptr<std::thread>> mtids;
    std::vector<queue<command_t>> mqids;

  public:
    MapReduce(
      std::vector<Tin> map_input,
      int nw ) :
      map_input ( map_input ),
      nw ( nw ),
      mtids ( nw ),
      mqids ( nw )
  {
    // Alocate output vector
    int max_size = map_input.size ();
    int chunk_size = max_size / nw;
    chunk_size = ( chunk_size == 0 ) ? max_size : chunk_size;

    auto worker = [&] ( queue<command_t> * q, queue<Tout> * t, int start, int end ) {
      while ( true ) {
        auto f = q->pop ();
        if ( f.second == EOS ) {
          return;
        }
        for ( int i = start; i < end; i ++ ) {
          t->push ( f.first ( map_input[i] ) );
        }
      }
    };

    for ( int i = 0; i < nw; i ++ ) {
      // Define responsability of the worker
      int start = i * chunk_size;
      int end = ( i == ( nw - 1 ) ) ? max_size : ( i + 1 ) * chunk_size;
      mtids[i] = std::make_unique<std::thread> ( worker, &mqids[i], &map_output, start, end );
    }

  }

    Tout compute ( std::function<Tout(Tin&)> f, std::function<Tout(Tout,Tout)> op ) {
      // Start threads
      for ( int i = 0; i < nw; i ++ ) {
        mqids[i].push(std::make_pair( f, GO ));
      }
      // Wait the end of the computation
      Tout glb_min = map_output.pop ();
      for ( int i = 1; i < map_input.size(); i ++ ) {
        Tout tmp = map_output.pop ();
        glb_min = op ( tmp, glb_min );
      }

      return glb_min;
    }

    void stop () {
      auto f = [](Tin& x){Tout y; return y;};
      for ( int i = 0; i < nw; i ++ ) {
        mqids[i].push(std::make_pair( f, EOS ));
      }
      for ( int i = 0; i < nw; i ++ ) {
        mtids[i]->join ();
      }
    }
};
#endif
