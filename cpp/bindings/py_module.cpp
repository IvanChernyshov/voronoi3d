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
#include "../core/tessellate.hpp"

namespace py = pybind11;
using namespace v3d;

static py::array_t<double> vec3_list_to_numpy(const std::vector<Vec3>& V){
    py::array_t<double> arr({(py::ssize_t)V.size(), (py::ssize_t)3});
    auto buf = arr.mutable_unchecked<2>();
    for(py::ssize_t i=0;i<(py::ssize_t)V.size();++i){
        buf(i,0)=V[i].x; buf(i,1)=V[i].y; buf(i,2)=V[i].z;
    }
    return arr;
}

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

    m.def("tessellate_pairs", [](const BoxContainer& box, const NeighborTable& T, py::array_t<double, py::array::c_style | py::array::forcecast> M_arr, const Config& cfg){
        if(M_arr.ndim()!=1) throw std::runtime_error("M must be a 1D float64 array");
        if((size_t)M_arr.shape(0) != T.i.size()) throw std::runtime_error("M length must equal neighbor table size");
        std::vector<double> M(T.i.size());
        auto Mb = M_arr.unchecked<1>();
        for(py::ssize_t k=0;k<M_arr.shape(0);++k) M[(size_t)k] = Mb(k);
        auto cells = tessellate_pairs(box, T, M, cfg);
        py::list out;
        for(const auto& c : cells){
            py::dict d;
            d["atom_id"] = c.atom_id;
            d["volume"] = c.volume;
            d["vertices"] = vec3_list_to_numpy(c.poly.V);
            py::list faces;
            for(const auto& loop : c.poly.F){
                py::list L;
                for(int idx : loop) L.append(idx);
                faces.append(L);
            }
            d["faces"] = faces;
            out.append(d);
        }
        return out;
    });

    m.def("tessellate_pairs", [](const TriclinicPBC& pbc, const NeighborTable& T, py::array_t<double, py::array::c_style | py::array::forcecast> M_arr, const Config& cfg){
        if(M_arr.ndim()!=1) throw std::runtime_error("M must be a 1D float64 array");
        if((size_t)M_arr.shape(0) != T.i.size()) throw std::runtime_error("M length must equal neighbor table size");
        std::vector<double> M(T.i.size());
        auto Mb = M_arr.unchecked<1>();
        for(py::ssize_t k=0;k<M_arr.shape(0);++k) M[(size_t)k] = Mb(k);
        auto cells = tessellate_pairs(pbc, T, M, cfg);
        py::list out;
        for(const auto& c : cells){
            py::dict d;
            d["atom_id"] = c.atom_id;
            d["volume"] = c.volume;
            d["vertices"] = vec3_list_to_numpy(c.poly.V);
            py::list faces;
            for(const auto& loop : c.poly.F){
                py::list L;
                for(int idx : loop) L.append(idx);
                faces.append(L);
            }
            d["faces"] = faces;
            out.append(d);
        }
        return out;
    });
}
