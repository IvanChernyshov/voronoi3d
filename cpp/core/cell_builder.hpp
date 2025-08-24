#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "vec.hpp"
#include "plane.hpp"
#include "polyhedron.hpp"
#include "config.hpp"

namespace v3d {
struct CellLocal {
    int atom_id = -1;
    Polyhedron poly;                 // local vertices/faces
    std::vector<int> neighbor_index; // index into neighbor table rows producing each face
};

// Build a cell by clipping a seed polyhedron by planes supplied per neighbor row (i.e., per oriented (i,j,image)).
// NOTE: this is a skeleton; actual clipping will be added in the next drop.
inline CellLocal build_cell_box_seed(int atom_id,
                                    const Polyhedron& seed,
                                    const std::vector<int>& rows_for_atom,
                                    const std::vector<Vec3>& disp,
                                    const std::vector<double>& M,
                                    const std::vector<int32_t>& i,
                                    const std::vector<int32_t>& /*j*/,
                                    const std::vector<Vec3>& ri,
                                    const Config& cfg){
    CellLocal C; C.atom_id = atom_id; C.poly = seed;
    for(int row : rows_for_atom){
        if(i[(size_t)row] != atom_id) continue;
        Vec3 d = disp[(size_t)row];
        double m = std::min(std::max(M[(size_t)row], cfg.min_M), 1.0-cfg.min_M);
        Vec3 n = d; double L = n.norm(); if(L==0) continue; n = n / L;
        Vec3 p = ri[(size_t)atom_id] + d * m;
        Plane H = from_point_normal(p, n);
        clip_by_plane(C.poly, H, cfg);
        C.neighbor_index.push_back(row);
    }
    return C;
}
}
