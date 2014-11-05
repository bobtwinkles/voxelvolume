#ifndef _VEC3F_H_
#define _VEC3F_H_
// contains the template specialisation for Vec3f
#include <functional>

namespace srp {
  template<typename SqrtRet, typename SqrtOp>
  class Vec3<float, SqrtRet, SqrtOp> {
    private:
      float _x, _y, _z;
    public:
      typedef Vec3<float, SqrtRet, SqrtOp> OwnType;

      Vec3(float X, float Y, float Z) : _x(X), _y(Y), _z(Z) {
      }
      Vec3() {}

      inline float GetX() const { return _x; }
      inline float GetY() const { return _y; }
      inline float GetZ() const { return _z; }

      inline float SetX(float val) { _x = val; }
      inline float SetY(float val) { _y = val; }
      inline float SetZ(float val) { _z = val; }

      float Magnitude2() const {
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

      OwnType & operator/= (const float Other) {
        this->_x /= Other;
        this->_y /= Other;
        this->_z /= Other;
        return *this;
      }

      bool operator==(const OwnType & Other) const {
        float x = this->_x - Other._x;
        float y = this->_y - Other._y;
        float z = this->_z - Other._z;
        return (x * x + y * y + z * z) < 0.01;
      }
  };
}

namespace std {
  template<>
  struct hash<srp::Vec3f> {
    std::size_t operator()(const srp::Vec3f & v) const {
      int x = static_cast<int>(v.GetX());
      int y = static_cast<int>(v.GetZ());
      int z = static_cast<int>(v.GetZ());
      return ((std::size_t)x * 2063) + ((std::size_t)y * 5413) + ((std::size_t)z);
    }
  };
}

#endif // _VEC3F_H_
