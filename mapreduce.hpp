#ifndef MPR_HPP
#define MPR_HPP
#include <iostream>
#include <thread>
#include <vector>

#include "queue.hpp"
#include "spo.hpp"

#define EOS -1
#define GO 0

template<typename T1, typename T2>
class MapReduce {
  private:
  // Input
  std::vector<T1> * input;
  int n;
  T2 ref;

  // Inner functions
  std::function<T2(T1&,const T2)> f;
  std::function<T2(T2,T2)> op;

  // Thread Pool
  int nw;
  std::vector<std::unique_ptr<std::thread>> pool;
  std::vector<Queue<int>> work_q;
  Queue<T2> result_q;

  public:
  MapReduce(
      std::vector<T1> * input,
      std::function<T2(T1&,const T2)> f,
      std::function<T2(T2,T2)> op,
      int nw,
      bool affinity = false ) :
    input ( input ),
    n ( input->size() ),
    f ( f ),
    op ( op ),
    nw ( nw ),
    pool ( nw ),
    work_q ( nw )
  {

    auto worker = [this] ( int id, int start, int end ) {
      while ( true ) {
        // Pop the command
        auto p = this->work_q[id].pop();
        if ( p == EOS ) {
          return;
        }

        T2 tmp = this->ref;
        for ( int i = start; i < end && i < this->n; i ++ ) {
          tmp = this->op ( tmp, this->f ( (*this->input)[i], this->ref ) );
        }
        this->result_q.push ( tmp );
      }
    };

    int n_cores = std::thread::hardware_concurrency();
    // Size computed by each worker
    int chunk_size = n / nw;
    chunk_size = ( chunk_size == 0 ) ? n : chunk_size;
    for ( int i = 0; i < nw; i ++ ) {
      // Define responsability of the worker
      int start = i * chunk_size;
      int end = ( i + 1 ) * chunk_size;
      pool[i] = std::make_unique<std::thread> ( worker, i, start, end );
      if ( affinity ) {
        cpu_set_t cpuset;
        CPU_ZERO( &cpuset );
        CPU_SET( i%n_cores, &cpuset );
        pthread_setaffinity_np ( pool[i]->native_handle(), sizeof(cpu_set_t), &cpuset );
      }
    }

  }

  T2 compute ( T2 ref ) {
    // Update the reference
    this->ref = ref;
    // Start threads
    for ( int i = 0; i < nw; i ++ ) {
      work_q[i].push( GO );
    }
    // Reduce
    for ( int i = 0; i < nw; i ++ ) {
      ref = op ( result_q.pop(), ref );
    }
    return ref;
  }

  void stop () {
    // Stop threads
    for ( int i = 0; i < nw; i ++ ) {
      work_q[i].push ( EOS );
    }
    for ( int i = 0; i < nw; i ++ ) {
      pool[i]->join ();
    }
  }
};
#endif
