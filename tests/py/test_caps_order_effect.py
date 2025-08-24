import numpy as np
import math
import voronoi3d as v3d

def _make_single_cap_cell(order):
    cfg = v3d.Config()
    cfg.min_M = 0.3
    box = v3d.BoxContainer(v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(10,10,10)))
    box.add_atoms([v3d.Vec3(5,5,5)])
    T = v3d.plan_neighbors(box, cfg)
    M = np.zeros(len(T.i), dtype=float)
    opt = v3d.CapOptions()
    opt.enabled = True
    opt.radius = 1.0
    opt.lebedev_order = order
    opt.surface_atom_ids = [0]
    cells = v3d.tessellate_pairs_with_caps(box, T, M, opt, cfg)
    return cells[0]

def test_caps_order_effect_vertices_and_volume():
    c6  = _make_single_cap_cell(6)
    c60 = _make_single_cap_cell(60)
    # More directions -> not fewer vertices, and volume should be closer to sphere
    Vsphere = 4.0/3.0*math.pi
    assert c60["vertices"].shape[0] >= c6["vertices"].shape[0]
    assert abs(c60["volume"] - Vsphere) <= abs(c6["volume"] - Vsphere)
