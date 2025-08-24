import numpy as np
import math
import voronoi3d as v3d

def test_caps_single_atom_sphere_volume():
    cfg = v3d.Config()
    cfg.min_M = 0.3
    # large box so walls are far; we'll mark atom as surface and skip walls
    box = v3d.BoxContainer(v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(10,10,10)))
    box.add_atoms([v3d.Vec3(5,5,5)])  # center
    T = v3d.plan_neighbors(box, cfg)  # N=1 => no neighbors
    M = np.zeros(len(T.i), dtype=float)
    opt = v3d.CapOptions()
    opt.enabled = True
    opt.radius = 1.0
    opt.lebedev_order = 200
    opt.surface_atom_ids = [0]  # explicitly mark
    cells = v3d.tessellate_pairs_with_caps(box, T, M, opt, cfg)
    V = cells[0]["volume"]
    Vsphere = 4.0/3.0 * math.pi * opt.radius**3
    # polygonal approximation, allow ~10% error with 26 directions
    assert np.isclose(V, Vsphere, rtol=0.03)

