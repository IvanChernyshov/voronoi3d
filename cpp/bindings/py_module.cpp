#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "../core/config.hpp"
#include "../core/vec.hpp"
#include "../core/lattice.hpp"
#include "../containers/box_container.hpp"
#include "../containers/triclinic_pbc.hpp"
#include "../core/neighbor.hpp"
#include "../core/polyhedron.hpp"
#include "../core/cell_builder.hpp"
#include "../core/mesh_builder.hpp"

namespace py = pybind11;
using namespace v3d;

PYBIND11_MODULE(_core, m) {
    py::class_<Config>(m, "Config")
        .def(py::init<>())
        .def_readwrite("eps_pos", &Config::eps_pos)
        .def_readwrite("eps_angle", &Config::eps_angle)
        .def_readwrite("min_face_area", &Config::min_face_area)
        .def_readwrite("min_M", &Config::min_M)
        .def_readwrite("reach_factor", &Config::reach_factor)
        .def_readwrite("neighbor_skin", &Config::neighbor_skin);

    py::class_<Vec3>(m, "Vec3")
        .def(py::init<double,double,double>())
        .def_readwrite("x", &Vec3::x).def_readwrite("y", &Vec3::y).def_readwrite("z", &Vec3::z);

    py::class_<Lattice>(m, "Lattice")
        .def(py::init<double,double,double,double,double,double>())
        .def("to_cart", &Lattice::to_cart)
        .def("to_frac", &Lattice::to_frac);

    py::class_<BoxBounds>(m, "BoxBounds")
        .def(py::init<Vec3,Vec3>())
        .def_readwrite("lo", &BoxBounds::lo)
        .def_readwrite("hi", &BoxBounds::hi);

    py::class_<BoxContainer>(m, "BoxContainer")
        .def(py::init<BoxBounds>())
        .def("add_atoms", &BoxContainer::add_atoms)
        .def_readonly("pos", &BoxContainer::pos)
        .def_readonly("bounds", &BoxContainer::bounds);

    py::class_<TriclinicPBC>(m, "TriclinicPBC")
        .def(py::init<Lattice, std::array<bool,3>>())
        .def("add_atoms", &TriclinicPBC::add_atoms)
        .def_readonly("pos", &TriclinicPBC::pos);

    py::class_<NeighborTable>(m, "NeighborTable")
        .def_property_readonly("size", &NeighborTable::size)
        .def_readonly("i", &NeighborTable::i)
        .def_readonly("j", &NeighborTable::j)
        .def_readonly("img", &NeighborTable::img)
        .def_readonly("disp", &NeighborTable::disp)
        .def_readonly("r2", &NeighborTable::r2);

    m.def("plan_neighbors", [](const BoxContainer& box, const Config& cfg){ return plan_neighbors(box, cfg); });
    m.def("plan_neighbors", [](const TriclinicPBC& pbc, const Config& cfg){ return plan_neighbors(pbc, cfg); });

    // tessellate_pairs will be bound once clipping & stitching are finalized.
}
