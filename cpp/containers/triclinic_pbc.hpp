#pragma once
#include <vector>
#include <array>
#include "../core/lattice.hpp"

namespace v3d {
struct TriclinicPBC {
    Lattice lat;
    std::array<bool,3> periodic{true,true,true};
    std::vector<Vec3> pos;

    TriclinicPBC(const Lattice& L, std::array<bool,3> mask): lat(L), periodic(mask) {}
    void add_atoms(const std::vector<Vec3>& xyz){ pos.insert(pos.end(), xyz.begin(), xyz.end()); }
};
}
