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
#include "../core/mesh_builder.hpp"
#include "../core/tessellate_caps.hpp"

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
            d["centroid"] = py::make_tuple(c.centroid.x, c.centroid.y, c.centroid.z);
            d["centroid"] = py::make_tuple(c.centroid.x, c.centroid.y, c.centroid.z);
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
            d["centroid"] = py::make_tuple(c.centroid.x, c.centroid.y, c.centroid.z);
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

    m.def("tessellate_pairs_global_mesh", [](const BoxContainer& box, const NeighborTable& T, py::array_t<double, py::array::c_style | py::array::forcecast> M_arr, const Config& cfg){
        if(M_arr.ndim()!=1) throw std::runtime_error("M must be a 1D float64 array");
        if((size_t)M_arr.shape(0) != T.i.size()) throw std::runtime_error("M length must equal neighbor table size");
        std::vector<double> M(T.i.size());
        auto Mb = M_arr.unchecked<1>();
        for(py::ssize_t k=0;k<M_arr.shape(0);++k) M[(size_t)k] = Mb(k);
        auto cells = tessellate_pairs(box, T, M, cfg);
        // collect polys and ids
        std::vector<Polyhedron> polys; polys.reserve(cells.size());
        std::vector<int> atom_ids; atom_ids.reserve(cells.size());
        std::vector<double> vols; vols.reserve(cells.size());
        std::vector<Vec3> atom_pos = box.pos;
        for(const auto& c : cells){ polys.push_back(c.poly); atom_ids.push_back(c.atom_id); vols.push_back(c.volume); }
        std::vector<Vec3> cents; cents.reserve(cells.size()); for(const auto& c : cells) cents.push_back(c.centroid);
        auto GM = stitch_global(T, polys, atom_ids, vols, cents, atom_pos, cfg);
        // build Python dict
        py::dict out;
        out["vertices"] = vec3_list_to_numpy(GM.vertices);
        // edges
        py::array_t<int> E_arr({(py::ssize_t)GM.edges.size(), (py::ssize_t)2});
        auto Eb = E_arr.mutable_unchecked<2>();
        for(py::ssize_t e=0;e<(py::ssize_t)GM.edges.size();++e){ Eb(e,0)=GM.edges[e][0]; Eb(e,1)=GM.edges[e][1]; }
        out["edges"] = E_arr;
        // faces
        py::list loops;
        py::array_t<int> Fi({(py::ssize_t)GM.faces.size()});
        py::array_t<int> Fj({(py::ssize_t)GM.faces.size()});
        py::array_t<int> Fimg({(py::ssize_t)GM.faces.size(), (py::ssize_t)3});
        py::array_t<double> Farea({(py::ssize_t)GM.faces.size()});
        py::array_t<double> Fcent({(py::ssize_t)GM.faces.size(), (py::ssize_t)3});
        py::array_t<double> Fnorm({(py::ssize_t)GM.faces.size(), (py::ssize_t)3});
        auto Fi_b=Fi.mutable_unchecked<1>(); auto Fj_b=Fj.mutable_unchecked<1>();
        auto Fimg_b=Fimg.mutable_unchecked<2>(); auto Farea_b=Farea.mutable_unchecked<1>();
        auto Fcent_b=Fcent.mutable_unchecked<2>(); auto Fnorm_b=Fnorm.mutable_unchecked<2>();
        for(py::ssize_t f=0; f<(py::ssize_t)GM.faces.size(); ++f){
            py::list L;
            for(int vid : GM.faces[f].loop) L.append(vid);
            loops.append(L);
            Fi_b(f) = GM.faces[f].i; Fj_b(f) = GM.faces[f].j;
            Fimg_b(f,0)=GM.faces[f].img[0]; Fimg_b(f,1)=GM.faces[f].img[1]; Fimg_b(f,2)=GM.faces[f].img[2];
            Farea_b(f) = GM.faces[f].area;
            Fcent_b(f,0)=GM.faces[f].centroid.x; Fcent_b(f,1)=GM.faces[f].centroid.y; Fcent_b(f,2)=GM.faces[f].centroid.z;
            Fnorm_b(f,0)=GM.faces[f].normal_ij.x; Fnorm_b(f,1)=GM.faces[f].normal_ij.y; Fnorm_b(f,2)=GM.faces[f].normal_ij.z;
        }
        py::dict Fdict;
        Fdict["loops"] = loops; Fdict["i"] = Fi; Fdict["j"] = Fj;
        Fdict["img"] = Fimg; Fdict["area"] = Farea;
        Fdict["centroid"] = Fcent; Fdict["normal_ij"] = Fnorm;
        out["faces"] = Fdict;
        // cells
        py::array_t<int> Cid({(py::ssize_t)GM.cells.size()});
        py::array_t<double> Cvol({(py::ssize_t)GM.cells.size()});
        py::array_t<double> Ccent({(py::ssize_t)GM.cells.size(), (py::ssize_t)3});
        auto Cid_b=Cid.mutable_unchecked<1>(); auto Cvol_b=Cvol.mutable_unchecked<1>(); auto Ccent_b=Ccent.mutable_unchecked<2>();
        py::list Cfaces;
        for(py::ssize_t ci=0; ci<(py::ssize_t)GM.cells.size(); ++ci){
            Cid_b(ci)=GM.cells[ci].atom_id; Cvol_b(ci)=GM.cells[ci].volume;
            Ccent_b(ci,0)=GM.cells[ci].centroid.x; Ccent_b(ci,1)=GM.cells[ci].centroid.y; Ccent_b(ci,2)=GM.cells[ci].centroid.z;
            py::list lf; for(int fid : GM.cells[ci].face_ids) lf.append(fid); Cfaces.append(lf);
        }
        py::dict Cdict; Cdict["atom_id"]=Cid; Cdict["volume"]=Cvol; Cdict["centroid"]=Ccent; Cdict["face_ids"]=Cfaces;
        out["cells"] = Cdict;
        return out;
    });


    py::class_<CapOptions>(m, "CapOptions")
        .def(py::init<>())
        .def_readwrite("enabled", &CapOptions::enabled)
        .def_readwrite("radius", &CapOptions::radius)
        .def_readwrite("lebedev_order", &CapOptions::lebedev_order)
        .def_readwrite("surface_atom_ids", &CapOptions::surface_atom_ids)
        .def_readwrite("auto_surface_margin", &CapOptions::auto_surface_margin);

    m.def("tessellate_pairs_with_caps", [](const BoxContainer& box, const NeighborTable& T, py::array_t<double, py::array::c_style | py::array::forcecast> M_arr, const CapOptions& opt, const Config& cfg){
        if(M_arr.ndim()!=1) throw std::runtime_error("M must be a 1D float64 array");
        if((size_t)M_arr.shape(0) != T.i.size()) throw std::runtime_error("M length must equal neighbor table size");
        std::vector<double> M(T.i.size());
        auto Mb = M_arr.unchecked<1>();
        for(py::ssize_t k=0;k<M_arr.shape(0);++k) M[(size_t)k] = Mb(k);
        auto cells = tessellate_pairs_with_caps(box, T, M, opt, cfg);
        py::list out;
        for(const auto& c : cells){
            py::dict d;
            d["atom_id"] = c.atom_id;
            d["volume"] = c.volume;
            d["centroid"] = py::make_tuple(c.centroid.x, c.centroid.y, c.centroid.z);
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
