import numpy as np
import voronoi3d as v3d

def test_two_atoms_half_box_volume():
    cfg = v3d.Config()
    cfg.min_M = 0.49  # near midpoint
    bounds = v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(1,1,1))
    box = v3d.BoxContainer(bounds)
    box.add_atoms([v3d.Vec3(0.25,0.5,0.5), v3d.Vec3(0.75,0.5,0.5)])

    T = v3d.plan_neighbors(box, cfg)
    E = len(T.i)
    M = np.full(E, 0.5, dtype=float)  # midpoint for both orientations
    cells = v3d.tessellate_pairs(box, T, M, cfg)
    vols = [c["volume"] for c in cells]
    assert np.allclose(vols[0] + vols[1], 1.0, atol=1e-6)
    assert np.isclose(vols[0], 0.5, atol=2e-2)
    assert np.isclose(vols[1], 0.5, atol=2e-2)
