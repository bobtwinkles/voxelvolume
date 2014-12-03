#include "DataStore.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define BUFF_SIZE 256

using srp::DataStore;

DataStore::DataStore(std::string Folder) {
  int w, h, d;
  _lod = 0;
  _root = NULL;
  std::string fname(Folder + "/meta.txt");
  std::string filenames;
  std::string files_base;
  std::ifstream f(fname);
  char buff[BUFF_SIZE];
  if (!f) {
    char workdir[1024];
    getcwd(workdir, 1024);
    std::cout << "Unable to read file " << fname << std::endl;
    std::cout << "Current working directory is " << workdir << std::endl;
    exit(-1);
  }
  f >> w;
  f >> h;
  f >> d;
  f >> filenames;
  files_base = Folder + "/" + filenames + ".%d";
  std::cout << "size " << w << " " << h << " " << d << std::endl;
  std::cout << "Filenames " << files_base << std::endl;

  this->_width = w;
  this->_height = h;
  this->_depth = d;

  _dstore = new unsigned int[w * h * d];
  _normals = new srp::Vec3f[w * h * d];

  unsigned int min = 0xFFFFFFFF;
  unsigned int max = 0;

  for (int i = 0; i < this->_depth; ++i) {
    snprintf(buff, BUFF_SIZE, files_base.c_str(), i + 1);
    std::ifstream data(buff, std::ifstream::binary);
    if (!data) {
      std::cout << "Failed to open file " << buff << std::endl;
    }
    for (int y = this->_height - 1; y >= 0; y--) {
      for (int x = 0; x < this->_width; ++x) {
        unsigned int val = 0;
        data.read((char*)&val, 2);
        val = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
        _dstore[x + y * this->_width + i * this->_width * this->_height] = val;
        if (val < min) {min = val;}
        if (val > max) {max = val;}
      }
    }
  }
  std::cout << "min: " << min << " max: " << max << std::endl;
  std::cout << "generating gradient" << std::endl;
  for (auto x = 0; x < _width; ++x) {
    for (auto y = 0; y < _height; ++y) {
      for (auto z = 0; z < _depth; ++z) {
        unsigned int mx = GetPoint(x - 1, y, z);
        unsigned int px = GetPoint(x + 1, y, z);
        unsigned int my = GetPoint(x, y - 1, z);
        unsigned int py = GetPoint(x, y + 1, z);
        unsigned int mz = GetPoint(x, y, z - 1);
        unsigned int pz = GetPoint(x, y, z + 1);
        float dx = (float(px) - float(mx)) / 2;
        float dy = (float(py) - float(my)) / 2;
        float dz = (float(pz) - float(mz)) / 2;
        _normals[x + y * _width + z * _width * _height] = Vec3f(dx, dy, dz);
      }
    }
  }
}

DataStore::DataStore(const srp::DataStore & Root, const unsigned int LOD) : _root(&Root), _lod(LOD) {
  if (_lod <= Root.GetLOD()) {
    std::cerr << "Tried to construct viewing DataStore out of a DataStore with less information!" << std::endl;
    std::cerr << "Requested LOD: " << _lod << std::endl;
    std::cerr << "Provided LOD:  " << Root.GetLOD() << std::endl;
    BUG();
  }
  std::cout << "Creating DataStore view at " <<_lod << " from " << Root.GetLOD() << std::endl;
  int lod_scale = _lod - Root.GetLOD();
  _width = Root.GetWidth() / lod_scale;
  _height = Root.GetHeight() / lod_scale;
  _depth = Root.GetDepth() / lod_scale;
  _dstore = new unsigned int[_width * _height * _depth];
  _normals = new srp::Vec3f[_width * _height * _depth];
  std::cout << "science: " << lod_scale << std::endl;
  for (auto z = 0; z < _depth; ++z) {
    for (auto y = 0; y < _height; ++y) {
      for (auto x = 0; x < _width; ++x) {
        int idx = x + y * _width + z * _width * _height;
        srp::Vec3f avg_norm;
        unsigned long avg_valu;
        for (auto zz = z * lod_scale; zz < (z + 1) * lod_scale; ++zz) {
          for (auto yy = y * lod_scale; yy < (y + 1) * lod_scale; ++yy) {
            for (auto xx = x * lod_scale; xx < (x + 1) * lod_scale; ++xx) {
              avg_valu += Root.GetPoint(xx, yy, zz);
              avg_norm += Root.GetNormal(xx, yy, zz);
            }
          }
        }
        avg_valu /= lod_scale * lod_scale * lod_scale;
        avg_norm /= lod_scale * lod_scale * lod_scale;
        _dstore[idx]  = avg_valu;
        _normals[idx] = avg_norm;
      }
    }
  }
}

DataStore::~DataStore() {
  delete[] _dstore;
  delete[] _normals;
}

static srp::Vec3f zero(0, 0, 0);

srp::Vec3f & DataStore::GetNormal(int X, int Y, int Z) const {
  if (X < 0 || X >= this->_width) {
    return zero;
  }
  if (Y < 0 || Y >= this->_height) {
    return zero;
  }
  if (Z < 0 || Z >= this->_depth) {
    return zero;
  }
  return _normals[X + Y * this->_width + Z * this->_width * this->_height];
}

unsigned int DataStore::GetPoint(int X, int Y, int Z) const {
  if (X < 0 || X >= this->_width) {
    return 0;
  }
  if (Y < 0 || Y >= this->_height) {
    return 0;
  }
  if (Z < 0 || Z >= this->_depth) {
    return 0;
  }
  return _dstore[X + Y * this->_width + Z * this->_width * this->_height];
}

unsigned int DataStore::GetPoint(srp::Vec3<int> & Point) const {
  return this->GetPoint(Point.GetX(), Point.GetY(), Point.GetZ());
}
