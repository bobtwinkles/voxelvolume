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
      }
    }
  }
}

DataStore::~DataStore() {
  delete[] _dstore;
}

unsigned int DataStore::GetPoint(int X, int Y, int Z) {
  return _dstore[X + Y * this->_width + Z * this->_width * this->_height];
}
