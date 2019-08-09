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
  private:
  // Input
  std::vector<Tin> * input;
  int n;
  Tout ref;

  // Inner functions
  std::function<Tout(Tin&,const Tout)> f;
  std::function<Tout(Tout,Tout)> op;

  // Thread Pool
  int nw;
  std::vector<std::unique_ptr<std::thread>> pool;
  std::vector<queue<int>> work_q;
  queue<Tout> result_q;


  public:
  MapReduce(
      std::vector<Tin> * input,
      std::function<Tout(Tin&,const Tout)> f,
      std::function<Tout(Tout,Tout)> op,
      int nw ) :
    input ( input ),
    n ( input->size() ),
    f ( f ),
    op ( op ),
    nw ( nw ),
    pool ( nw ),
    work_q ( n )
  {

    auto worker = [this] ( int id, int start, int end ) {
      while ( true ) {
        // Pop the command
        auto p = this->work_q[id].pop();
        if ( p == EOS ) {
          return;
        }
        Tout tmp = ref;
        for ( int i = start; i < end; i ++ ) {
          tmp = this->op ( tmp, this->f ( (*this->input)[i], ref ) );
        }
        this->result_q.push ( tmp );
      }
    };

    // Size computed by each worker
    int chunk_size = n / nw;
    chunk_size = ( chunk_size == 0 ) ? n : chunk_size;
    for ( int i = 0; i < nw; i ++ ) {
      // Define responsability of the worker
      int start = i * chunk_size;
      int end = ( i == ( nw - 1 ) ) ? n : ( i + 1 ) * chunk_size;
      pool[i] = std::make_unique<std::thread> ( worker, i, start, end );
    }

  }

  Tout compute ( Tout ref ) {
    // Update the reference
    this->ref = ref;
    // Start threads
    for ( int i = 0; i < nw; i ++ ) {
      work_q[i].push( GO );
    }
    // Barrier
    for ( int i = 0; i < nw; i ++ ) {
      ref = op ( result_q.pop(), ref );
    }
    return ref;
  }

  void stop () {
    // Stop threads
    for ( int i = 0; i < n; i ++ ) {
      work_q[i].push ( EOS );
    }
    for ( int i = 0; i < nw; i ++ ) {
      pool[i]->join ();
    }
  }
};
#endif
