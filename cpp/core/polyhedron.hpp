#pragma once
#include <vector>
#include <limits>
#include <optional>
#include "vec.hpp"
#include "plane.hpp"
#include "config.hpp"

namespace v3d {

struct Polyhedron {
    std::vector<Vec3> V;                   // vertices
    std::vector<std::vector<int>> F;       // faces as CCW loops
};

// TODO: Implement incremental half-space clipping (to come next)
inline void clip_by_plane(Polyhedron& /*P*/, const Plane& /*H*/, const Config& /*cfg*/){
    // Placeholder; real implementation will land in the next code drop.
}

inline Polyhedron make_bounding_box(const Vec3& lo, const Vec3& hi){
    Polyhedron P;
    P.V = {
        {lo.x, lo.y, lo.z}, {hi.x, lo.y, lo.z}, {hi.x, hi.y, lo.z}, {lo.x, hi.y, lo.z},
        {lo.x, lo.y, hi.z}, {hi.x, lo.y, hi.z}, {hi.x, hi.y, hi.z}, {lo.x, hi.y, hi.z}
    };
    P.F = {
        {0,1,2,3}, {4,5,6,7}, {0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}
    };
    return P;
}
}
