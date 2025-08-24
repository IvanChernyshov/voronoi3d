#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <tuple>
#include "vec.hpp"
#include "polyhedron.hpp"
#include "neighbor.hpp"
#include "../containers/box_container.hpp"
#include "../containers/triclinic_pbc.hpp"

namespace v3d {

struct GlobalMeshFace {
    std::vector<int> loop;               // indices into GlobalMesh.vertices
    int i = -1;                          // left atom id (or smaller id)
    int j = -1;                          // right atom id (or -1 for wall)
    std::array<int,3> img{0,0,0};        // image for j relative to i
    double area = 0.0;
    Vec3 centroid{0,0,0};
    Vec3 normal_ij{0,0,1};               // oriented from i to j (if j>=0), arbitrary for walls
};

struct GlobalMeshCell {
    int atom_id = -1;
    std::vector<int> face_ids;           // indices into faces
    double volume = 0.0;
    Vec3 centroid{0,0,0};
};

struct GlobalMesh {
    std::vector<Vec3> vertices;
    std::vector<std::array<int,2>> edges;
    std::vector<GlobalMeshFace> faces;
    std::vector<GlobalMeshCell> cells;
};

inline std::vector<int> canonical_cycle(const std::vector<int>& loop){
    if(loop.empty()) return loop;
    // rotate so that smallest vertex id comes first; then choose orientation with lexicographically smaller sequence
    int n = (int)loop.size();
    int minv = loop[0], mini = 0;
    for(int k=1;k<n;k++){ if(loop[k] < minv){ minv = loop[k]; mini = k; } }
    auto get_at = [&](int idx)->int{ return loop[(mini + idx)%n]; };
    auto get_at_rev = [&](int idx)->int{ return loop[(mini - idx + n)%n]; };
    std::vector<int> A(n), B(n);
    for(int t=0;t<n;t++){ A[t]=get_at(t); B[t]=get_at_rev(t); }
    if(B < A) return B;
    return A;
}

struct Vec3Key { long long x,y,z; };
inline Vec3Key key_of(const Vec3& v, double q){
    return Vec3Key{ (long long)std::llround(v.x/q), (long long)std::llround(v.y/q), (long long)std::llround(v.z/q) };
}

inline GlobalMesh stitch_global(const NeighborTable& T, const std::vector<Polyhedron>& cell_polys, const std::vector<int>& atom_ids, const std::vector<double>& volumes, const std::vector<Vec3>& atom_pos, const Config& cfg){
    GlobalMesh G;
    const double q = std::max(1e-9, cfg.eps_pos*100);
    // 1) Vertex dedup
    std::unordered_map<long long, int> vmap; vmap.reserve(16384);
    auto pack3 = [](long long a, long long b, long long c)->long long{ return ((a & 0x7fffff)<<42) ^ ((b & 0x7fffff)<<21) ^ (c & 0x7fffff); };
    std::vector<std::vector<int>> local2global(cell_polys.size());
    for(size_t ci=0; ci<cell_polys.size(); ++ci){
        const auto& P = cell_polys[ci];
        auto& L2G = local2global[ci];
        L2G.resize(P.V.size(), -1);
        for(size_t lv=0; lv<P.V.size(); ++lv){
            Vec3Key k = key_of(P.V[lv], q);
            long long h = pack3(k.x, k.y, k.z);
            auto it = vmap.find(h);
            if(it == vmap.end()){
                int gid = (int)G.vertices.size();
                G.vertices.push_back(P.V[lv]);
                vmap.emplace(h, gid);
                L2G[lv] = gid;
            }else{
                L2G[lv] = it->second;
            }
        }
    }
    // 2) Face dedup using canonical vertex cycles
    struct FaceKeyHash {
        size_t operator()(const std::vector<int>& v) const noexcept {
            size_t h=1469598103934665603ull;
            for(int x : v){ h ^= (size_t)x; h *= 1099511628211ull; }
            return h;
        }
    };
    struct FaceKeyEq {
        bool operator()(const std::vector<int>& a, const std::vector<int>& b) const noexcept {
            return a==b;
        }
    };
    std::unordered_map<std::vector<int>, int, FaceKeyHash, FaceKeyEq> fmap;
    // For each cell face, register or find global face id
    for(size_t ci=0; ci<cell_polys.size(); ++ci){
        const auto& P = cell_polys[ci];
        GlobalMeshCell cell;
        cell.atom_id = atom_ids[ci];
        cell.volume = volumes[ci];
        // centroid as average of vertices (approx)
        Vec3 c{0,0,0}; for(const auto& v : P.V) c += v; if(!P.V.empty()) c = c / (double)P.V.size(); cell.centroid = c;
        for(size_t f=0; f<P.F.size(); ++f){
            std::vector<int> gl;
            gl.reserve(P.F[f].size());
            for(int lv : P.F[f]) gl.push_back(local2global[ci][(size_t)lv]);
            // remove consecutive duplicates & ensure >=3 unique
            std::vector<int> gl2; gl2.reserve(gl.size());
            for(size_t k=0;k<gl.size();++k){ if(k==0 || gl[k]!=gl[k-1]) gl2.push_back(gl[k]); }
            if(gl2.size()>=3 && gl2.front()==gl2.back()) gl2.pop_back();
            if(gl2.size()<3) continue;
            auto canon = canonical_cycle(gl2);
            int fid;
            auto it = fmap.find(canon);
            if(it==fmap.end()){
                fid = (int)G.faces.size();
                GlobalMeshFace Fg;
                Fg.loop = canon;
                int tag = (f < (int)P.face_tag.size()) ? P.face_tag[f] : -1;
                if(tag>=0){
                    int ii = atom_ids[ci];
                    int jj = T.j[(size_t)tag];
                    Fg.i = ii; Fg.j = jj;
                    Fg.img = T.img[(size_t)tag];
                    // orient normal from i to j
                    Vec3 dir = T.disp[(size_t)tag]; double L = dir.norm(); if(L>0) dir = dir / L; else dir = Vec3{0,0,1};
                    // compute normal via Newell from canon loop
                    Vec3 n{0,0,0}; Vec3 cc{0,0,0};
                    for(size_t k=0;k<canon.size();++k){
                        const Vec3& a = G.vertices[canon[k]];
                        const Vec3& b = G.vertices[canon[(k+1)%canon.size()]];
                        n.x += (a.y - b.y) * (a.z + b.z);
                        n.y += (a.z - b.z) * (a.x + b.x);
                        n.z += (a.x - b.x) * (a.y + b.y);
                        cc += a;
                    }
                    double Ln = n.norm(); Vec3 nu = (Ln>0)? n/Ln : Vec3{0,0,1};
                    if(nu.dot(dir) < 0){
                        // reverse to align
                        std::reverse(Fg.loop.begin(), Fg.loop.end());
                    }
                    Fg.normal_ij = dir;
                } else {
                    Fg.i = atom_ids[ci]; Fg.j = -1;
                    Fg.img = {0,0,0};
                    Fg.normal_ij = Vec3{0,0,1};
                }
                // area & centroid (use attributes from local poly if available)
                if(f < P.face_area.size()) Fg.area = P.face_area[f];
                if(f < P.face_centroid.size()) Fg.centroid = P.face_centroid[f];
                G.faces.push_back(std::move(Fg));
                fmap.emplace(canon, fid);
            } else {
                fid = it->second;
            }
            cell.face_ids.push_back(fid);
        }
        G.cells.push_back(std::move(cell));
    }
    // 3) Build edges as unique unordered pairs from face loops
    std::unordered_map<long long,int> emap; emap.reserve(65536);
    auto pack2 = [](int a, int b)->long long{
        int u = std::min(a,b), v = std::max(a,b);
        return ( (long long)u << 32 ) | (unsigned long long)v;
    };
    for(const auto& Fg : G.faces){
        const auto& L = Fg.loop;
        for(size_t k=0;k<L.size();++k){
            int a = L[k], b = L[(k+1)%L.size()];
            long long h = pack2(a,b);
            if(emap.find(h)==emap.end()){
                int eid = (int)G.edges.size();
                emap.emplace(h, eid);
                G.edges.push_back({std::min(a,b), std::max(a,b)});
            }
        }
    }
    return G;
}

} // namespace v3d
