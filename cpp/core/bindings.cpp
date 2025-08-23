#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <vector>
#include <stdexcept>

// Voro++
#include "voro++.hh"

namespace py = pybind11;

struct CellsResult {
    std::vector<double> volumes;
    std::vector<std::vector<int>> neighbors;
};

static CellsResult compute_cells_impl(py::array_t<double, py::array::c_style | py::array::forcecast> points,
                                      py::array_t<double, py::array::c_style | py::array::forcecast> bounds,
                                      int nx, int ny, int nz) {
    if (points.ndim() != 2 || points.shape(1) != 3) {
        throw std::invalid_argument("points must have shape (N,3)");
    }
    if (bounds.ndim() != 2 || bounds.shape(0) != 3 || bounds.shape(1) != 2) {
        throw std::invalid_argument("bounds must have shape (3,2) = [[xmin,xmax],[ymin,ymax],[zmin,zmax]]");
    }
    const ssize_t N = points.shape(0);
    auto P = points.unchecked<2>();
    auto B = bounds.unchecked<2>();

    double xmin = B(0,0), xmax = B(0,1);
    double ymin = B(1,0), ymax = B(1,1);
    double zmin = B(2,0), zmax = B(2,1);

    if (!(xmax > xmin && ymax > ymin && zmax > zmin)) {
        throw std::invalid_argument("Invalid bounds: each max must be greater than min");
    }

    // Non-periodic container with an internal grid for acceleration
    voro::container con(xmin, xmax, ymin, ymax, zmin, zmax, nx, ny, nz, false, false, false, 8);

    // Insert particles
    for (ssize_t i = 0; i < N; ++i) {
        double x = P(i,0), y = P(i,1), z = P(i,2);
        if (!(x > xmin && x < xmax && y > ymin && y < ymax && z > zmin && z < zmax)) {
            throw std::invalid_argument("Point " + std::to_string(i) + " is outside the box bounds");
        }
        con.put(static_cast<int>(i), x, y, z);
    }

    CellsResult res;
    res.volumes.resize(static_cast<size_t>(N));
    res.neighbors.resize(static_cast<size_t>(N));

    voro::c_loop_all cla(con);
    voro::voronoicell_neighbor c;
    if (cla.start()) do {
        if (con.compute_cell(c, cla)) {
            int id; double x, y, z;
            cla.pos(x, y, z);
            id = cla.pid();

            res.volumes[static_cast<size_t>(id)] = c.volume();

            std::vector<int> neigh;
            c.neighbors(neigh); // negative = walls
            res.neighbors[static_cast<size_t>(id)] = std::move(neigh);
        }
    } while (cla.inc());

    return res;
}

PYBIND11_MODULE(_core, m) {
    m.doc() = "voronoi3d core bindings (Milestone 1)";

    m.def("version", [](){ return std::string("0.0.1-m1"); });

    m.def("compute_cells",
          [](py::array_t<double> points,
             py::array_t<double> bounds,
             py::object grid_divisions) {
                int nx=6, ny=6, nz=6;
                if (!grid_divisions.is_none()) {
                    auto t = grid_divisions.cast<std::tuple<int,int,int>>();
                    nx = std::get<0>(t);
                    ny = std::get<1>(t);
                    nz = std::get<2>(t);
                    if (nx<=0 || ny<=0 || nz<=0) throw std::invalid_argument("grid divisions must be positive");
                }
                auto res = compute_cells_impl(points, bounds, nx, ny, nz);
                py::dict out;
                out["volumes"] = py::array_t<double>(res.volumes.size(), res.volumes.data());
                out["neighbors"] = res.neighbors;
                return out;
          },
          py::arg("points"),
          py::arg("bounds"),
          py::arg("grid_divisions") = py::none(),
          R"doc(
              Compute unweighted Voronoi cells in an axis-aligned non-periodic box.

              Parameters
              ----------
              points : (N,3) float64 array
              bounds : (3,2) float64 array [[xmin,xmax],[ymin,ymax],[zmin,zmax]]
              grid_divisions : (nx,ny,nz) tuple, optional acceleration grid

              Returns
              -------
              dict with:
                volumes : (N,) float64 per-cell volume
                neighbors : List[List[int]] per-cell neighbor IDs (walls as negative IDs)
          )doc");
}
