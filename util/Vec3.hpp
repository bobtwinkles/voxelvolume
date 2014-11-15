#ifndef _VEC3_H_
#define _VEC3_H_

#include <cmath>

namespace srp {
  template<typename T>
  struct StdSqrtOp {
    double operator() (T t) { return sqrt(t); }
  };

  class DataStore;

  template<typename T, typename SqrtRet = double, typename SqrtOp = StdSqrtOp<T> >
  class Vec3 {
    private:
      friend class DataStore;
      T _x, _y, _z;
    public:
      typedef Vec3<T, SqrtRet, SqrtOp> OwnType;

      Vec3(T X, T Y, T Z) : _x(X), _y(Y), _z(Z) {
      }
      Vec3() {}

      inline T GetX() const { return _x; }
      inline T GetY() const { return _y; }
      inline T GetZ() const { return _z; }

      inline T SetX(T val) { _x = val; }
      inline T SetY(T val) { _y = val; }
      inline T SetZ(T val) { _z = val; }

      T Magnitude2() const {
        return _x * _x + _y * _y + _z * _z;
      }

      SqrtRet Magnitude() const {
        return SqrtOp(this->Magnitude2());
      }

      OwnType & NormalizeSelf() {
        return *this;
      }

      OwnType & operator= (const OwnType & Other) {
        this->_x = Other._x;
        this->_y = Other._y;
        this->_z = Other._z;
        return *this;
      }

      OwnType & operator+= (const OwnType & Other) {
        this->_x += Other._x;
        this->_y += Other._y;
        this->_z += Other._z;
        return *this;
      }

      OwnType & operator/= (const T Other) {
        this->_x /= Other;
        this->_y /= Other;
        this->_z /= Other;
        return *this;
      }

      bool operator==(const OwnType Other) const {
        return this->_x == Other._x && this->_y == Other._y && this->_z == Other._z;
      }
  };

  typedef Vec3<int> Vec3i;
  typedef Vec3<float> Vec3f;
  typedef Vec3<double> Vec3d;
}

#include "Vec3f.hpp"

#endif
