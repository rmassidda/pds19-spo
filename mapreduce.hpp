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
      //  Chunk sizes
      std::vector<std::pair<int,int>> chunks;

      int size = input.size() / nw;
      for ( int i = 0; i < nw; i++ ) {
        if( i != (nw-1) ) {
          chunks.push_back(std::make_pair(i*size, (i+1)*size-1));   // regular chunck
        } else {                                               // last one is longer 
          chunks.push_back(std::make_pair(i*size, input.size()));     // sospetto fine riga -1
        }
      }

      // Map worker
      std::vector<Tout> map_out(input.size());
      auto mapworker = [&] ( int cid ) {
        //  Apply function
        for(int i=chunks[cid].first; i<chunks[cid].second; i++) {  // compute the map
          map_out[i] = ( f ( input[i] ) );
        }
        return;
      };

      // Start mappers
      std::vector<std::thread> mtids;
      for(int i=0; i<nw; i++)
        mtids.push_back(std::thread(mapworker,i));

      // Await mappers
      for(int i=0; i<nw; i++)
        mtids[i].join();

      // Sequential reduce
      // TODO: parallelize
      Tout result;
      for ( int i = 0; i < input.size(); i++) {
        result = op ( result, map_out[i] );
      }
      return result;
    }
};
