#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <cmath>
#include "vec.hpp"
#include "config.hpp"
#include "../containers/box_container.hpp"
#include "../containers/triclinic_pbc.hpp"

namespace v3d {
struct NeighborTable {
    // Oriented pairs (one row per oriented (i,j,image))
    std::vector<int32_t> i, j;
    std::vector<std::array<int32_t,3>> img; // (na,nb,nc)
    std::vector<Vec3> disp;                 // cartesian displacement ri->rj(image)
    std::vector<double> r2;                 // squared distance
    size_t size() const { return i.size(); }
};

inline NeighborTable plan_neighbors(const BoxContainer& box, const Config& cfg){
    NeighborTable T;
    const size_t N = box.pos.size();
    for(size_t ii=0; ii<N; ++ii){
        double R = box.farthest_corner_radius((int)ii);
        double rsearch = (R / std::max(cfg.min_M, 1e-12)) + cfg.neighbor_skin;
        double r2max = rsearch*rsearch;
        for(size_t jj=0; jj<N; ++jj){ if(ii==jj) continue;
            Vec3 d = box.pos[jj] - box.pos[ii];
            double d2 = d.norm2();
            if(d2 <= r2max){
                T.i.push_back((int32_t)ii); T.j.push_back((int32_t)jj);
                T.img.push_back({0,0,0});
                T.disp.push_back(d);
                T.r2.push_back(d2);
            }
        }
    }
    return T;
}

inline NeighborTable plan_neighbors(const TriclinicPBC& pbc, const Config& cfg){
    NeighborTable T;
    const size_t N = pbc.pos.size();
    if(N==0) return T;
    // crude nearest-neighbor distance estimate
    double dnn = std::numeric_limits<double>::infinity();
    for(size_t i=0;i<N;i++){
        for(size_t j=i+1;j<N;j++){
            auto [d, img] = pbc.lat.min_image_disp(pbc.pos[i], pbc.pos[j], pbc.periodic);
            dnn = std::min(dnn, d.norm());
        }
    }
    if(!std::isfinite(dnn) || dnn==0.0) dnn = 1.0;
    double R = cfg.reach_factor * dnn;
    double rsearch = (R / std::max(cfg.min_M, 1e-12)) + cfg.neighbor_skin;

    auto norm = [](const Vec3& v){ return std::sqrt(v.norm2()); };
    double na_max = std::ceil(rsearch / std::max(1e-12, norm(pbc.lat.A.c0)));
    double nb_max = std::ceil(rsearch / std::max(1e-12, norm(pbc.lat.A.c1)));
    double nc_max = std::ceil(rsearch / std::max(1e-12, norm(pbc.lat.A.c2)));

    for(size_t ii=0; ii<N; ++ii){
        for(size_t jj=0; jj<N; ++jj){ if(ii==jj) continue;
            for(int na = (pbc.periodic[0]?-(int)na_max:0); na <= (pbc.periodic[0]?(int)na_max:0); ++na)
            for(int nb = (pbc.periodic[1]?-(int)nb_max:0); nb <= (pbc.periodic[1]?(int)nb_max:0); ++nb)
            for(int nc = (pbc.periodic[2]?-(int)nc_max:0); nc <= (pbc.periodic[2]?(int)nc_max:0); ++nc){
                Vec3 im = pbc.lat.A.c0*(double)na + pbc.lat.A.c1*(double)nb + pbc.lat.A.c2*(double)nc;
                Vec3 d = (pbc.pos[jj] + im) - pbc.pos[ii];
                double d2 = d.norm2();
                if(d2 <= rsearch*rsearch && d2>0){
                    T.i.push_back((int32_t)ii);
                    T.j.push_back((int32_t)jj);
                    T.img.push_back({na,nb,nc});
                    T.disp.push_back(d);
                    T.r2.push_back(d2);
                }
            }
        }
    }
    return T;
}
}
