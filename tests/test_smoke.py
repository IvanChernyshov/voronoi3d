import numpy as np
import math
import voronoi3d as v3

def test_basic_two_points_unit_cube():
    pts = np.array([[0.3, 0.5, 0.5],
                    [0.7, 0.5, 0.5]], dtype=float)
    bounds = np.array([[0.0, 1.0],
                       [0.0, 1.0],
                       [0.0, 1.0]], dtype=float)
    out = v3.compute_cells(pts, bounds, grid_divisions=(4,4,4))
    vols = np.asarray(out["volumes"])
    assert math.isclose(vols.sum(), 1.0, rel_tol=1e-12, abs_tol=1e-12)
    assert any(n < 0 for n in out["neighbors"][0])
    assert any(n < 0 for n in out["neighbors"][1])
