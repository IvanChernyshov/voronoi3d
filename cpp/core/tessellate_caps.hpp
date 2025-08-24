#pragma once
#include <vector>
#include <unordered_set>
#include "vec.hpp"
#include "plane.hpp"
#include "polyhedron.hpp"
#include "neighbor.hpp"
#include "lebedev.hpp"
#include "../containers/box_container.hpp"

namespace v3d {

struct CapOptions {
    bool enabled = false;
    double radius = 1.0;
    int lebedev_order = 26; // 6, 14, or 26 supported here
    std::vector<int> surface_atom_ids; // if empty and auto_surface_margin>0, auto-detect
    double auto_surface_margin = 0.0;  // mark atoms within this distance from any wall as surface
};

inline bool is_surface_atom_box(const BoxContainer& box, int i, const CapOptions& opt){
    if(!opt.enabled) return false;
    if(!opt.surface_atom_ids.empty()){
        for(int id : opt.surface_atom_ids) if(id==i) return true;
        return false;
    }
    if(opt.auto_surface_margin>0){
        const Vec3& r = box.pos[i];
        double m = opt.auto_surface_margin;
        if( (r.x - box.bounds.lo.x) < m ) return true;
        if( (box.bounds.hi.x - r.x) < m ) return true;
        if( (r.y - box.bounds.lo.y) < m ) return true;
        if( (box.bounds.hi.y - r.y) < m ) return true;
        if( (r.z - box.bounds.lo.z) < m ) return true;
        if( (box.bounds.hi.z - r.z) < m ) return true;
        return false;
    }
    return false;
}

inline std::vector<CellResult> tessellate_pairs_with_caps(const BoxContainer& box,
                                                          const NeighborTable& T,
                                                          const std::vector<double>& M,
                                                          const CapOptions& opt,
                                                          const Config& cfg){
    const int N = (int)box.pos.size();
    std::vector<CellResult> out; out.reserve(N);
    auto dirs = lebedev_dirs(opt.lebedev_order);
    for(int i=0;i<N;i++){
        std::vector<int> rows;
        for(size_t r=0;r<T.i.size();++r) if(T.i[r]==i) rows.push_back((int)r);
        std::vector<PlaneWithTag> planes;
        bool use_caps = is_surface_atom_box(box, i, opt);
        if(!use_caps){
            // Keep box walls for interior atoms
            planes.push_back({ Plane{Vec3{-1,0,0}, -box.bounds.lo.x}, -1000 });
            planes.push_back({ Plane{Vec3{+1,0,0},  box.bounds.hi.x}, -1001 });
            planes.push_back({ Plane{Vec3{0,-1,0}, -box.bounds.lo.y}, -1002 });
            planes.push_back({ Plane{Vec3{0,+1,0},  box.bounds.hi.y}, -1003 });
            planes.push_back({ Plane{Vec3{0,0,-1}, -box.bounds.lo.z}, -1004 });
            planes.push_back({ Plane{Vec3{0,0,+1},  box.bounds.hi.z}, -1005 });
        } else {
            // Add spherical caps around atom i
            const Vec3& ri = box.pos[i];
            for(size_t k=0;k<dirs.size();++k){
                Vec3 n = dirs[k];
                Vec3 p = ri + n*opt.radius;
                Plane H = from_point_normal(p, n); // n·x <= n·(ri + R)
                planes.push_back({H, -3000 - (int)k});
            }
        }
        // neighbor planes
        for(int r : rows){
            Vec3 d = T.disp[(size_t)r];
            double L = d.norm(); if(L==0) continue;
            Vec3 n = d / L;
            double m = std::min(std::max(M[(size_t)r], cfg.min_M), 1.0 - cfg.min_M);
            Vec3 p = box.pos[i] + d*m;
            Plane Hp = from_point_normal(p, n);
            planes.push_back({Hp, r});
        }
        Polyhedron P = halfspace_intersection(planes, cfg);
        CellResult C; C.atom_id = i; C.poly = std::move(P);
        auto [V,Cc] = polyhedron_volume_centroid(C.poly);
        C.volume = V; C.centroid = Cc;
        out.push_back(std::move(C));
    }
    return out;
}

} // namespace v3d
