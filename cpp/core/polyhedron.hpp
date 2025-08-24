#pragma once
#include <vector>
#include <limits>
#include <optional>
#include <cmath>
#include <algorithm>
#include "vec.hpp"
#include "plane.hpp"
#include "config.hpp"

namespace v3d {

struct Polyhedron {
    std::vector<Vec3> V;                   // vertices
    std::vector<std::vector<int>> F;       // faces as CCW loops (indices into V)
    std::vector<Vec3> face_normal;
    std::vector<double> face_area;
    std::vector<Vec3> face_centroid;
    std::vector<int> face_tag;             // user tag for the plane that generated the face
};

struct PlaneWithTag {
    Plane P;
    int tag; // user tag (e.g., neighbor row index or negative for walls/box)
};

inline bool solve3x3(const double M[3][3], const double b[3], double x[3], double eps=1e-14){
    double det = M[0][0]*(M[1][1]*M[2][2]-M[1][2]*M[2][1]) - M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0]) + M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
    if(std::fabs(det) < eps) return false;
    double inv[3][3];
    inv[0][0] =  (M[1][1]*M[2][2]-M[1][2]*M[2][1])/det;
    inv[0][1] = -(M[0][1]*M[2][2]-M[0][2]*M[2][1])/det;
    inv[0][2] =  (M[0][1]*M[1][2]-M[0][2]*M[1][1])/det;
    inv[1][0] = -(M[1][0]*M[2][2]-M[1][2]*M[2][0])/det;
    inv[1][1] =  (M[0][0]*M[2][2]-M[0][2]*M[2][0])/det;
    inv[1][2] = -(M[0][0]*M[1][2]-M[0][2]*M[1][0])/det;
    inv[2][0] =  (M[1][0]*M[2][1]-M[1][1]*M[2][0])/det;
    inv[2][1] = -(M[0][0]*M[2][1]-M[0][1]*M[2][0])/det;
    inv[2][2] =  (M[0][0]*M[1][1]-M[0][1]*M[1][0])/det;
    for(int r=0;r<3;++r){
        x[r] = inv[r][0]*b[0] + inv[r][1]*b[1] + inv[r][2]*b[2];
    }
    return true;
}

inline bool intersect_three(const Plane& A, const Plane& B, const Plane& C, Vec3& out, double eps=1e-12){
    double M[3][3] = {
        {A.n.x, A.n.y, A.n.z},
        {B.n.x, B.n.y, B.n.z},
        {C.n.x, C.n.y, C.n.z}
    };
    double b[3] = {A.d, B.d, C.d};
    double x[3];
    if(!solve3x3(M, b, x, eps)) return false;
    out = Vec3{x[0], x[1], x[2]};
    return true;
}

inline double point_plane_distance_signed(const Plane& P, const Vec3& x){
    return signed_distance(P, x);
}

inline Vec3 orthonormal_u(const Vec3& n){
    Vec3 a = std::fabs(n.x) < 0.9 ? Vec3{1,0,0} : Vec3{0,1,0};
    Vec3 u = n.cross(a);
    double L = u.norm(); if(L==0) u = n.cross(Vec3{0,0,1}), L = u.norm();
    return u / L;
}

// exact per-face area/centroid via triangulation; normal via summed cross
inline void compute_face_attributes(Polyhedron& P){
    P.face_area.resize(P.F.size());
    P.face_centroid.resize(P.F.size());
    P.face_normal.resize(P.F.size());
    for(size_t f=0; f<P.F.size(); ++f){
        const auto& loop = P.F[f];
        if(loop.size()<3){ P.face_area[f]=0; P.face_centroid[f]={0,0,0}; P.face_normal[f]={0,0,1}; continue; }
        const Vec3& v0 = P.V[loop[0]];
        double area_sum = 0.0;
        Vec3 centroid_sum{0,0,0};
        Vec3 nsum{0,0,0};
        for(size_t k=1; k+1<loop.size(); ++k){
            const Vec3& v1 = P.V[loop[k]];
            const Vec3& v2 = P.V[loop[k+1]];
            Vec3 e1 = v1 - v0, e2 = v2 - v0;
            Vec3 cr = e1.cross(e2);
            double tri_area = 0.5 * cr.norm();
            area_sum += tri_area;
            centroid_sum += (v0 + v1 + v2) * (tri_area / 3.0);
            nsum += cr;
        }
        P.face_area[f] = area_sum;
        P.face_centroid[f] = (area_sum>0)? (centroid_sum / area_sum) : v0;
        double L = nsum.norm();
        P.face_normal[f] = (L>0)? (nsum / L) : Vec3{0,0,1};
    }
}

// prune faces with area below threshold
inline void prune_tiny_faces(Polyhedron& P, const Config& cfg){
    std::vector<std::vector<int>> F2;
    std::vector<Vec3> N2, C2;
    std::vector<double> A2;
    std::vector<int> T2;
    for(size_t f=0; f<P.F.size(); ++f){
        if(f < P.face_area.size() && P.face_area[f] < cfg.min_face_area) continue;
        F2.push_back(std::move(P.F[f]));
        A2.push_back(f < P.face_area.size()? P.face_area[f] : 0.0);
        C2.push_back(f < P.face_centroid.size()? P.face_centroid[f] : Vec3{0,0,0});
        N2.push_back(f < P.face_normal.size()? P.face_normal[f] : Vec3{0,0,1});
        T2.push_back(f < P.face_tag.size()? P.face_tag[f] : -1);
    }
    P.F = std::move(F2);
    P.face_area = std::move(A2);
    P.face_centroid = std::move(C2);
    P.face_normal = std::move(N2);
    P.face_tag = std::move(T2);
}

// Build convex polyhedron from intersection of half-spaces n·x <= d
inline Polyhedron halfspace_intersection(const std::vector<PlaneWithTag>& planes, const Config& cfg){
    Polyhedron P;
    const size_t N = planes.size();
    if(N < 4) return P;
    // collect all triple intersections
    std::vector<Vec3> verts;
    verts.reserve(N*N);
    const double eps_in = std::max(1e-9, cfg.eps_pos*10);
    for(size_t a=0;a<N;a++){
        for(size_t b=a+1;b<N;b++){
            for(size_t c=b+1;c<N;c++){
                Vec3 x;
                if(!intersect_three(planes[a].P, planes[b].P, planes[c].P, x)) continue;
                bool inside = true;
                for(size_t k=0;k<N;k++){
                    if(point_plane_distance_signed(planes[k].P, x) > eps_in){ inside=false; break; }
                }
                if(inside) verts.push_back(x);
            }
        }
    }
    // deduplicate vertices
    const double q = std::max(1e-9, cfg.eps_pos*100);
    struct Key{ long long x,y,z; };
    auto keyfn = [q](const Vec3& v)->Key{
        return Key{ (long long)std::llround(v.x/q), (long long)std::llround(v.y/q), (long long)std::llround(v.z/q) };
    };
    std::vector<Vec3> Vuniq;
    std::vector<int> map_idx(verts.size(), -1);
    std::vector<Key> keys;
    keys.reserve(verts.size());
    for(size_t i=0;i<verts.size();++i){
        keys.push_back(keyfn(verts[i]));
    }
    for(size_t i=0;i<verts.size();++i){
        if(map_idx[i]!=-1) continue;
        Vuniq.push_back(verts[i]);
        int gid = (int)Vuniq.size()-1;
        map_idx[i] = gid;
        for(size_t j=i+1;j<verts.size();++j){
            if(map_idx[j]!=-1) continue;
            if(keys[i].x==keys[j].x && keys[i].y==keys[j].y && keys[i].z==keys[j].z){
                map_idx[j] = gid;
            }
        }
    }
    P.V = std::move(Vuniq);
    if(P.V.size()<4){ P.F.clear(); return P; }

    // Build faces: for each plane, collect vertices on plane
    for(size_t pi=0; pi<N; ++pi){
        const Plane& pl = planes[pi].P;
        std::vector<int> verts_on;
        for(size_t v=0; v<P.V.size(); ++v){
            double sd = std::fabs(point_plane_distance_signed(pl, P.V[v]));
            if(sd <= eps_in*2) verts_on.push_back((int)v);
        }
        if(verts_on.size()<3) continue;
        // Order vertices CCW around plane normal
        Vec3 n = pl.n;
        Vec3 u = orthonormal_u(n);
        Vec3 v = n.cross(u);
        // centroid in 3D
        Vec3 c{0,0,0}; for(int id: verts_on) c += P.V[id]; c = c / (double)verts_on.size();
        struct PA{ int id; double ang; };
        std::vector<PA> proj; proj.reserve(verts_on.size());
        for(int id: verts_on){
            Vec3 d = P.V[id] - c;
            double x = d.dot(u), y = d.dot(v);
            double ang = std::atan2(y, x);
            proj.push_back({id, ang});
        }
        std::sort(proj.begin(), proj.end(), [](const PA& a, const PA& b){ return a.ang < b.ang; });
        std::vector<int> loop; loop.reserve(proj.size());
        for(const auto& p : proj) loop.push_back(p.id);
        // Append face
        P.F.push_back(loop);
        P.face_tag.push_back(planes[pi].tag);
    }
    compute_face_attributes(P);
    prune_tiny_faces(P, cfg);
    return P;
}

// Volume & centroid via origin-referenced tetrahedra
inline std::pair<double, Vec3> polyhedron_volume_centroid(const Polyhedron& P){
    double V = 0.0;
    Vec3 C{0,0,0};
    for(const auto& loop : P.F){
        if(loop.size()<3) continue;
        const Vec3& v0 = P.V[loop[0]];
        for(size_t k=1; k+1<loop.size(); ++k){
            const Vec3& v1 = P.V[loop[k]];
            const Vec3& v2 = P.V[loop[k+1]];
            // signed volume of tetra (0,v0,v1,v2)
            double vol6 = v0.dot( (v1).cross(v2) );
            double vtet = vol6 / 6.0;
            V += vtet;
            C += (v0 + v1 + v2) * (vtet / 4.0);
        }
    }
    double Vabs = std::fabs(V);
    Vec3 Cfin = (Vabs>0)? (C / V) : Vec3{0,0,0}; // divide by signed V to keep correct sign
    return {Vabs, Cfin};
}

// Volume via face formula (kept for reference / debugging)
inline double polyhedron_volume(const Polyhedron& P){
    double V = 0.0;
    if(P.F.size()!=P.face_area.size() || P.F.size()!=P.face_centroid.size() || P.F.size()!=P.face_normal.size()) return 0.0;
    for(size_t f=0; f<P.F.size(); ++f){
        V += (P.face_area[f] * P.face_normal[f].dot(P.face_centroid[f])) / 3.0;
    }
    return std::fabs(V);
}

} // namespace v3d
