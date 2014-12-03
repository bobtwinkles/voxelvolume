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
      unsigned int _lod;
      const srp::DataStore * _root;
      DISALLOW_COPY_AND_ASSIGN(DataStore);
    public:
      DataStore(std::string Folder);
      // If we ever support run time modificatoin there will need to be a mechanism for propegating
      // changes up and down the LOD chain
      DataStore(const srp::DataStore & Root, const unsigned int LOD);
      ~DataStore();

      srp::Vec3f & GetNormal(int X, int Y, int Z) const;
      unsigned int GetPoint(int X, int Y, int Z) const;
      unsigned int GetPoint(Vec3i & Point) const;

      inline unsigned int GetWidth() const { return this->_width; }
      inline unsigned int GetHeight() const { return this->_height; }
      inline unsigned int GetDepth() const { return this->_depth; }
      inline unsigned int GetLOD() const { return this->_lod; }
  };
}

#endif
