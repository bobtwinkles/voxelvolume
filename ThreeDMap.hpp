#ifndef _THREEDMAP_H_
#define _THREEDMAP_H_

#include "Vec3.hpp"

#include <utility> // for std::pair
#include <list>
#include <string.h>

namespace srp {
  template<typename KBase, typename V, int ArraySize = 16>
  class ThreeDMap {
    public:
      typedef Vec3<KBase> Key;
      typedef std::pair<Key, V> ValueType;
    private:
      std::list<ValueType> * _data[ArraySize * ArraySize * ArraySize];

      V _default_value;
    public:
      ThreeDMap(V DefaultValue) : _default_value(DefaultValue) {
        memset(this->_data, 0, sizeof(this->_data));
      }

      virtual ~ThreeDMap() {
        for (unsigned i = 0; i < sizeof(this->_data) / sizeof(this->_data[0]); ++i) {
          if (this->_data[i]) {
            delete this->_data[i];
          }
        }
      }

      V Get(const Key TheKey) const {
        int idx = (TheKey.GetX() + ArraySize * TheKey.GetY() + ArraySize * ArraySize * TheKey.GetZ()) % (ArraySize * ArraySize * ArraySize);
        std::list<ValueType> * lst = this->_data[idx];
        if (!lst) {
          return this->_default_value;
        }
        typename std::list<ValueType>::iterator it = lst->begin();
        while(it != lst->end()) {
          ValueType value = *it;
          if (value.first == TheKey) {
            return value.second;
          }
          it++;
        }
        return this->_default_value;
      }

      void Set(const Key TheKey, const V Value) {
        int idx = (TheKey.GetX() + ArraySize * TheKey.GetY() + ArraySize * ArraySize * TheKey.GetZ()) % (ArraySize * ArraySize * ArraySize);
        std::list<ValueType> * lst = this->_data[idx];
        if (!lst) {
          lst = new std::list<ValueType>();
          this->_data[idx] = lst;
        }
        typename std::list<ValueType>::iterator it = lst->begin();
        while (it != lst->end()) {
          if (it->first == TheKey) {
            it->second = Value;
            return;
          }

          it++;
        }
        lst->push_back(std::make_pair(TheKey, Value));
      }
  };
}

#endif
