#pragma once
#include <vector>
#include <limits>
#include "../core/vec.hpp"

namespace v3d {
struct BoxBounds { Vec3 lo, hi; };

struct BoxContainer {
    BoxBounds bounds{};
    std::vector<Vec3> pos;

    explicit BoxContainer(const BoxBounds& b): bounds(b) {}
    void add_atoms(const std::vector<Vec3>& xyz){ pos.insert(pos.end(), xyz.begin(), xyz.end()); }

    // Maximum distance from point to any box corner (safe reach bound R_i)
    double farthest_corner_radius(int i) const {
        const Vec3 &r = pos[i];
        double R=0.0;
        for(int cx=0; cx<2; ++cx) for(int cy=0; cy<2; ++cy) for(int cz=0; cz<2; ++cz){
            Vec3 c{ cx?bounds.hi.x:bounds.lo.x, cy?bounds.hi.y:bounds.lo.y, cz?bounds.hi.z:bounds.lo.z };
            double d = (c - r).norm();
            if(d>R) R = d;
        }
        return R;
    }
};
}
