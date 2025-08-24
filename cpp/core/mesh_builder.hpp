#pragma once
#include <vector>
#include <unordered_map>
#include <tuple>
#include "polyhedron.hpp"

namespace v3d {
struct GlobalMesh {
    std::vector<Vec3> vertices;      // unique vertices
    std::vector<std::array<int,2>> edges;
    struct Face { std::vector<int> loop; int i=-1, j=-1; std::array<int,3> img{0,0,0}; double area=0; Vec3 normal{0,0,1}; Vec3 centroid{0,0,0}; };
    std::vector<Face> faces;
    struct Cell { int atom_id=-1; std::vector<int> face_ids; double volume=0; Vec3 centroid{0,0,0}; };
    std::vector<Cell> cells;
};

// TODO: Implement deduplication and attribute computation in a later step
inline GlobalMesh stitch_global(const std::vector<CellLocal>& /*locals*/){ return {}; }
}
