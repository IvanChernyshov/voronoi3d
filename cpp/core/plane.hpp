#pragma once
#include "vec.hpp"

namespace v3d {
struct Plane { // n·x = d, with |n|=1 ideally
    Vec3 n{1,0,0};
    double d{0.0};
};

inline double signed_distance(const Plane& P, const Vec3& x){ return P.n.dot(x) - P.d; }
inline Plane from_point_normal(const Vec3& p, const Vec3& n){
    const double nn = n.norm();
    Vec3 nh = (nn>0)? n/nn : Vec3{1,0,0};
    return {nh, nh.dot(p)};
}
}
