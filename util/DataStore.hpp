#ifndef _DATA_H_
#define _DATA_H_

#include <string>

#include "Util.hpp"
#include "Vec3.hpp"

namespace srp {
  class DataStore {
    private:
      unsigned int * _dstore;
      srp::Vec3f * _normals;
      unsigned int _width, _height, _depth;
      DISALLOW_COPY_AND_ASSIGN(DataStore);
    public:
      DataStore(std::string Folder);
      ~DataStore();

      srp::Vec3f & GetNormal(int X, int Y, int Z);
      unsigned int GetPoint(int X, int Y, int Z);
      unsigned int GetPoint(Vec3i & Point);

      inline unsigned int GetWidth() { return this->_width; }
      inline unsigned int GetHeight() { return this->_height; }
      inline unsigned int GetDepth() { return this->_depth; }
  };
}

#endif
