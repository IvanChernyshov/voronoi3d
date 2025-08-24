#pragma once
#include <vector>
#include <cmath>
#include "vec.hpp"

namespace v3d {

// Return a set of (approximately) uniformly distributed unit vectors on S^2.
// order <= 6: 6-point octahedron
// order <=14: add cube corners (8)
// order <=26: add edge midpoints (12) for total 26
// order  >26: Fibonacci sphere with N=order points
inline std::vector<Vec3> lebedev_dirs(int order){
    std::vector<Vec3> dirs;
    if(order<=6){
        dirs = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
        return dirs;
    }
    if(order<=14){
        dirs = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
        double s = 1.0/std::sqrt(3.0);
        int sgn[2]={-1,1};
        for(int a=0;a<2;a++) for(int b=0;b<2;b++) for(int c=0;c<2;c++){
            dirs.push_back({ s*sgn[a], s*sgn[b], s*sgn[c] });
        }
        return dirs;
    }
    if(order<=26){
        dirs = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
        double s2 = 1.0/std::sqrt(2.0);
        int sgn[2]={-1,1};
        for(int a=0;a<2;a++) for(int b=0;b<2;b++){
            dirs.push_back({ s2*sgn[a], s2*sgn[b], 0 });
            dirs.push_back({ s2*sgn[a], 0, s2*sgn[b] });
            dirs.push_back({ 0, s2*sgn[a], s2*sgn[b] });
        }
        double s3 = 1.0/std::sqrt(3.0);
        for(int a=0;a<2;a++) for(int b=0;b<2;b++) for(int c=0;c<2;c++){
            dirs.push_back({ s3*sgn[a], s3*sgn[b], s3*sgn[c] });
        }
        return dirs;
    }
    // Fibonacci sphere
    const int N = order;
    dirs.reserve(N);
    const double phi = (1.0 + std::sqrt(5.0)) * 0.5; // golden ratio
    const double ga = 2.0 * M_PI * (1.0 - 1.0/phi); // golden angle
    for(int k=0; k<N; ++k){
        double z = 1.0 - 2.0*((k + 0.5)/ (double)N);
        double r = std::sqrt(std::max(0.0, 1.0 - z*z));
        double theta = ga * k;
        double x = r * std::cos(theta);
        double y = r * std::sin(theta);
        // already unit length by construction
        dirs.push_back(Vec3{x,y,z});
    }
    return dirs;
}

} // namespace v3d
