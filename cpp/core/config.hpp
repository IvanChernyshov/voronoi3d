#pragma once
#include <cstddef>
#include <limits>

namespace v3d {
struct Config {
    double eps_pos = 1e-10;          // geometric position tolerance
    double eps_angle = 1e-12;        // for normal normalization / parallel checks
    double min_face_area = 1e-14;    // prune tiny faces

    // Planning parameters (see plan_neighbors)
    double min_M = 0.1;              // lower bound for M (0 < min_M < 0.5 ideally)
    double reach_factor = 2.5;       // PBC: R = reach_factor * d_nn
    double neighbor_skin = 1e-8;     // padding for search radius
};
}
