#pragma once
#include <vector>
#include <unordered_map>
#include "vec.hpp"
#include "plane.hpp"
#include "polyhedron.hpp"
#include "neighbor.hpp"
#include "../containers/box_container.hpp"
#include "../containers/triclinic_pbc.hpp"

namespace v3d {

struct CellResult {
    int atom_id = -1;
    Polyhedron poly;
    double volume = 0.0;
};

inline Plane box_plane_x_le(double x){ return Plane{Vec3{1,0,0}, x}; }      // x <= hi
inline Plane box_plane_x_ge(double x){ return Plane{Vec3{-1,0,0}, -x}; }    // -x <= -lo
inline Plane box_plane_y_le(double y){ return Plane{Vec3{0,1,0}, y}; }
inline Plane box_plane_y_ge(double y){ return Plane{Vec3{0,-1,0}, -y}; }
inline Plane box_plane_z_le(double z){ return Plane{Vec3{0,0,1}, z}; }
inline Plane box_plane_z_ge(double z){ return Plane{Vec3{0,0,-1}, -z}; }

inline std::vector<int> rows_for_atom_i(const NeighborTable& T, int i){
    std::vector<int> rows;
    for(size_t r=0;r<T.i.size();++r) if(T.i[r]==i) rows.push_back((int)r);
    return rows;
}

inline std::vector<CellResult> tessellate_pairs(const BoxContainer& box,
                                                const NeighborTable& T,
                                                const std::vector<double>& M,
                                                const Config& cfg){
    const int N = (int)box.pos.size();
    std::vector<CellResult> out; out.reserve(N);
    for(int i=0;i<N;i++){
        std::vector<int> rows = rows_for_atom_i(T, i);
        std::vector<PlaneWithTag> planes;
        planes.reserve(6 + rows.size());
        // Box walls
        planes.push_back({ box_plane_x_ge(box.bounds.lo.x), -1000 });
        planes.push_back({ box_plane_x_le(box.bounds.hi.x), -1001 });
        planes.push_back({ box_plane_y_ge(box.bounds.lo.y), -1002 });
        planes.push_back({ box_plane_y_le(box.bounds.hi.y), -1003 });
        planes.push_back({ box_plane_z_ge(box.bounds.lo.z), -1004 });
        planes.push_back({ box_plane_z_le(box.bounds.hi.z), -1005 });
        // Neighbor planes
        for(int r : rows){
            Vec3 d = T.disp[(size_t)r];
            double L = d.norm();
            if(L==0) continue;
            Vec3 n = d / L;
            double m = std::min(std::max(M[(size_t)r], cfg.min_M), 1.0 - cfg.min_M);
            Vec3 p = box.pos[i] + d * m;
            Plane Hp = from_point_normal(p, n); // keep half-space n·x <= d
            planes.push_back({Hp, r});
        }
        Polyhedron P = halfspace_intersection(planes, cfg);
        CellResult C; C.atom_id=i; C.poly = std::move(P); C.volume = polyhedron_volume(C.poly);
        out.push_back(std::move(C));
    }
    return out;
}

inline std::vector<CellResult> tessellate_pairs(const TriclinicPBC& pbc,
                                                const NeighborTable& T,
                                                const std::vector<double>& M,
                                                const Config& cfg){
    const int N = (int)pbc.pos.size();
    std::vector<CellResult> out; out.reserve(N);
    for(int i=0;i<N;i++){
        std::vector<int> rows = rows_for_atom_i(T, i);
        std::vector<PlaneWithTag> planes;
        planes.reserve(rows.size());
        for(int r : rows){
            Vec3 d = T.disp[(size_t)r];
            double L = d.norm();
            if(L==0) continue;
            Vec3 n = d / L;
            double m = std::min(std::max(M[(size_t)r], cfg.min_M), 1.0 - cfg.min_M);
            Vec3 p = pbc.pos[i] + d * m;
            Plane Hp = from_point_normal(p, n); // keep half-space n·x <= d
            planes.push_back({Hp, r});
        }
        Polyhedron P = halfspace_intersection(planes, cfg);
        CellResult C; C.atom_id=i; C.poly = std::move(P); C.volume = polyhedron_volume(C.poly);
        out.push_back(std::move(C));
    }
    return out;
}

} // namespace v3d
