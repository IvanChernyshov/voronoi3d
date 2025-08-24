#pragma once
#include <cmath>
#include <array>
#include <ostream>

namespace v3d {
struct Vec3 {
    double x{}, y{}, z{};
    Vec3() = default;
    Vec3(double X, double Y, double Z): x(X), y(Y), z(Z) {}

    double& operator[](int i) { return i==0?x:(i==1?y:z); }
    const double& operator[](int i) const { return i==0?x:(i==1?y:z); }

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    Vec3 operator/(double s) const { return {x/s, y/s, z/s}; }
    Vec3& operator+=(const Vec3& o){ x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o){ x-=o.x; y-=o.y; z-=o.z; return *this; }

    double dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const { return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x}; }
    double norm2() const { return dot(*this); }
    double norm() const { return std::sqrt(norm2()); }
};
inline std::ostream& operator<<(std::ostream& os, const Vec3& v){ return os<<v.x<<","<<v.y<<","<<v.z; }

struct Mat3 {
    // columns are lattice vectors by convention
    Vec3 c0, c1, c2;
    Vec3 operator*(const Vec3& f) const { // cart = A * frac
        return { c0.x*f.x + c1.x*f.y + c2.x*f.z,
                 c0.y*f.x + c1.y*f.y + c2.y*f.z,
                 c0.z*f.x + c1.z*f.y + c2.z*f.z };
    }
};
}
