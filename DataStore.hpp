#ifndef _DATA_H_
#define _DATA_H_

#include <string>

#include "Util.hpp"

namespace srp {
  class DataStore {
    private:
      unsigned int * _dstore;
      unsigned int _width, _height, _depth;
      DISALLOW_COPY_AND_ASSIGN(DataStore);
    public:
      DataStore(std::string Folder);
      ~DataStore();

      unsigned int GetPoint(int X, int Y, int Z);

      inline unsigned int GetWidth() { return this->_width; }
      inline unsigned int GetHeight() { return this->_height; }
      inline unsigned int GetDepth() { return this->_depth; }
  };
}

#endif
