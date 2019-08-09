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
  using command_t = std::pair<std::function<Tout(Tin&)>,std::shared_ptr<std::vector<Tin>>>;
  private:
    queue<Tout> map_output;
    int n;
    int nw;
    std::vector<std::unique_ptr<std::thread>> mtids;
    std::vector<queue<command_t>> mqids;

  public:
    MapReduce(
      int n,
      int nw ) :
      n ( n ),
      nw ( nw ),
      mtids ( nw ),
      mqids ( nw )
  {
    // Alocate output vector
    int chunk_size = n / nw;
    chunk_size = ( chunk_size == 0 ) ? n : chunk_size;

    auto worker = [&] ( queue<command_t> * q, queue<Tout> * t, int start, int end ) {
      while ( true ) {
        // Pop the pair
        auto p = q->pop ();
        // Function
        auto f = p.first;
        // Vector pointer
        auto v = p.second;
        if ( v == nullptr ) {
          return;
        }
        for ( int i = start; i < end; i ++ ) {
          // printf ( "%d> ", i ); fflush ( stdout );
          // (*v)[i].current.print_result( stdout ); fflush ( stdout );
          auto z = f ( (*v)[i] );
          t->push ( z );
        }
      }
    };

    for ( int i = 0; i < nw; i ++ ) {
      // Define responsability of the worker
      int start = i * chunk_size;
      int end = ( i == ( nw - 1 ) ) ? n : ( i + 1 ) * chunk_size;
      mtids[i] = std::make_unique<std::thread> ( worker, &mqids[i], &map_output, start, end );
    }

  }

    Tout compute ( std::shared_ptr<std::vector<Tin>> v, std::function<Tout(Tin&)> f, std::function<Tout(Tout,Tout)> op ) {
      // Start threads
      for ( int i = 0; i < nw; i ++ ) {
        mqids[i].push(std::make_pair( f, v ));
      }
      // Wait the end of the computation
      Tout glb_min = map_output.pop ();
      for ( int i = 1; i < n; i ++ ) {
        Tout tmp = map_output.pop ();
        glb_min = op ( tmp, glb_min );
      }

      return glb_min;
    }

    void stop () {
      auto f = [](Tin& x){Tout y; return y;};
      for ( int i = 0; i < nw; i ++ ) {
        mqids[i].push(std::make_pair( f, nullptr ));
      }
      for ( int i = 0; i < nw; i ++ ) {
        mtids[i]->join ();
      }
    }
};
#endif
