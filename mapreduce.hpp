#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <map>
#include <iostream>
#include <thread>
#include <queue>
#include <optional>

#include "utils.hpp"
#include "spo.hpp"

template<typename Tin,typename Tout>
class MapReduce {
  private:
    std::vector<Tin> input;
    std::function<Tout(Tout,Tout)> op;
    int nw;

  public:
    MapReduce(
      std::vector<Tin> input,
      std::function<Tout(Tout,Tout)> op,
      int nw ) :
      input ( input ),
      op ( op ),
      nw ( nw )
  {}

    Tout compute ( std::function<Tout(Tin&)> f ) {
      std::vector<Tout> map_out;
      Tout result;

      // Map
      for( auto &i : input ) {
        map_out.push_back ( f ( i ) );
      }

      // Reduce
      for(auto &o : map_out ) {
        result = op ( result, o );
      }
      return result; 
    }

    // void parCompute() {
    //   std::cerr << "ParCompute called" << std::endl; 
    //   std::vector<std::pair<int,int>> chunks;     // compute the chunk sizes

    //   // chunk size
    //   int size = (inv.size() / nmap);
    //   for ( int i = 0; i < nmap; i++ ) {
    //     if( i != (nmap-1) ) {
    //       chunks.push_back(std::make_pair(i*size, (i+1)*size-1));   // regular chunck
    //     } else {                                               // last one is longer 
    //       chunks.push_back(std::make_pair(i*size, inv.size()));     // sospetto fine riga -1
    //     }
    //   }
    //   std::vector<queue<std::pair<Tkey, Tvalue>>> redq(nred);       // create queues to reducers

    //   auto mapworker = [&] (int c) {             // this is the map worker code
    //     std::map<Tkey,Tvalue> mapresult;              // declare the map for the result

    //     // c is the chunk identifier
    //     for(int i=chunks[c].first; i<chunks[c].second; i++) {  // compute the map
    //       auto v = f(inv[i]);                    // apply map
    //       for(auto &it : v)                      // insert pairs into the result
    //         mapresult[it.first] = oplus(mapresult[it.first],it.second);
    //     }
    //     for(auto &it : mapresult) {              // once completed, deliver pairs to proper 
    //       int destreducer = std::hash<Tkey>()(it.first) % nred; // reducer: use hash to pick up index
    //       redq[destreducer].push(it);            // then send it the pair 
    //     }
    //     return;                                  // done, everything to reducers
    //   };

    //   std::vector<std::map<Tkey,Tvalue>> redresults(nred); 
    //   auto redworker = [&](int r) {              // the reduce worker 
    //     while( true ) {
    //       auto po = (redq[r]).pop();         // pop a pair from the input queu

    //       if( po.first == EOS ) {                  // if there is nothing, means mappers closed
    //         break;                               // then stop
    //       } 
    //       else {
    //         (redresults[r])[po.first] = oplus((redresults[r])[po.first],po.second);
    //       }
    //     }
    //     return;                                  // queue empty, then done!
    //   };

    //   std::vector<std::thread> mtids;                      // thread ids of the mappers
    //   std::vector<std::thread> rtids;                      // thread ids of the reducers

    //   for(int i=0; i<nred; i++)                  // create reducers: will block on empty queues
    //     rtids.push_back(std::thread(redworker,i));
    //   for(int i=0; i<nmap; i++)                  // create mappers: start filling queues
    //     mtids.push_back(std::thread(mapworker,i));

    //   for(int i=0; i<nmap; i++)                  // await mapper termination
    //     mtids[i].join();
    //   for(int i=0; i<nred; i++) {                // tell reducer input queues all producers ended
    //     redq[i].push ( std::pair<Tkey, Tvalue>( EOS, 0 ) ); // no more values to read
    //   }
    //   for(int i=0; i<nred; i++)                  // now await reducer termination
    //     rtids[i].join();

    //   std::cerr << "Sequential merge" << std::endl; 
    //   // sequential merge of the maps computed by the reducers, should be implemented in parallel
    //   // TODO: parallelize this
    //   for ( int i = 0; i < nred; i++) {
    //     results.insert( redresults[i].begin(), redresults[i].end() );
    //   }
    //   return;
    // }
};
