#pragma once
#include <array>
#include <tuple>
#include <cmath>
#include "vec.hpp"

namespace v3d {
struct Lattice {
    // Triclinic lattice defined by a,b,c and angles (deg). Stores both cart and inverse.
    double a,b,c, alpha, beta, gamma; // lengths, degrees
    Mat3 A;        // columns are lattice vectors in cartesian
    Mat3 Ainv;     // inverse such that frac = Ainv * cart

    Lattice(double a_, double b_, double c_, double alpha_deg, double beta_deg, double gamma_deg)
    : a(a_), b(b_), c(c_), alpha(alpha_deg), beta(beta_deg), gamma(gamma_deg) {
        const double deg = M_PI/180.0;
        const double ca = std::cos(alpha*deg), cb = std::cos(beta*deg), cg = std::cos(gamma*deg);
        const double sg = std::sin(gamma*deg);
        // build A in conventional form
        Vec3 a1{a, 0, 0};
        Vec3 a2{b*cg, b*sg, 0};
        double cx = c*cb;
        double cy = c*(ca - cb*cg)/sg;
        double cz = std::sqrt(std::max(0.0, c*c - cx*cx - cy*cy));
        Vec3 a3{cx, cy, cz};
        A = Mat3{a1,a2,a3};
        // inverse via generic 3x3 inversion
        double m[3][3] = {{A.c0.x, A.c1.x, A.c2.x}, {A.c0.y, A.c1.y, A.c2.y}, {A.c0.z, A.c1.z, A.c2.z}};
        double det = m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1]) - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0]) + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
        double inv[3][3];
        inv[0][0] =  (m[1][1]*m[2][2]-m[1][2]*m[2][1])/det;
        inv[0][1] = -(m[0][1]*m[2][2]-m[0][2]*m[2][1])/det;
        inv[0][2] =  (m[0][1]*m[1][2]-m[0][2]*m[1][1])/det;
        inv[1][0] = -(m[1][0]*m[2][2]-m[1][2]*m[2][0])/det;
        inv[1][1] =  (m[0][0]*m[2][2]-m[0][2]*m[2][0])/det;
        inv[1][2] = -(m[0][0]*m[1][2]-m[0][2]*m[1][0])/det;
        inv[2][0] =  (m[1][0]*m[2][1]-m[1][1]*m[2][0])/det;
        inv[2][1] = -(m[0][0]*m[2][1]-m[0][1]*m[2][0])/det;
        inv[2][2] =  (m[0][0]*m[1][1]-m[0][1]*m[1][0])/det;
        Ainv = Mat3{ Vec3{inv[0][0], inv[1][0], inv[2][0]}, Vec3{inv[0][1], inv[1][1], inv[2][1]}, Vec3{inv[0][2], inv[1][2], inv[2][2]} };
    }

    Vec3 to_cart(const Vec3& f) const { return A * f; }
    Vec3 to_frac(const Vec3& r) const { // frac = Ainv * r
        const Vec3 &c0=Ainv.c0, &c1=Ainv.c1, &c2=Ainv.c2;
        return { c0.x*r.x + c1.x*r.y + c2.x*r.z,
                 c0.y*r.x + c1.y*r.y + c2.y*r.z,
                 c0.z*r.x + c1.z*r.y + c2.z*r.z };
    }

    Vec3 wrap_frac(const Vec3& f, const std::array<bool,3>& periodic) const {
        Vec3 w=f;
        if(periodic[0]) w.x -= std::floor(w.x);
        if(periodic[1]) w.y -= std::floor(w.y);
        if(periodic[2]) w.z -= std::floor(w.z);
        return w;
    }

    // Minimum-image displacement from ri to rj+n, returning disp and image (na,nb,nc)
    std::pair<Vec3, std::array<int,3>> min_image_disp(const Vec3& ri, const Vec3& rj,
                                                      const std::array<bool,3>& periodic) const {
        Vec3 fi = to_frac(ri), fj = to_frac(rj);
        Vec3 df = { fj.x - fi.x, fj.y - fi.y, fj.z - fi.z };
        std::array<int,3> img{0,0,0};
        for(int k=0;k<3;++k){
            if(periodic[k]){
                double s = std::round(df[k]);
                df[k] -= s;
                img[k] = static_cast<int>(-s);
            }
        }
        Vec3 disp = to_cart(df);
        return {disp, img};
    }
};
}
