import numpy as np
import voronoi3d as v3d

def test_global_mesh_two_atoms_box():
    cfg = v3d.Config()
    cfg.min_M = 0.5
    box = v3d.BoxContainer(v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(1,1,1)))
    box.add_atoms([v3d.Vec3(0.25,0.5,0.5), v3d.Vec3(0.75,0.5,0.5)])

    T = v3d.plan_neighbors(box, cfg)
    M = np.full(len(T.i), 0.5, dtype=float)
    mesh = v3d.tessellate_pairs_global_mesh(box, T, M, cfg)

    V = mesh["vertices"]
    F = mesh["faces"]
    i = np.array(F["i"])
    j = np.array(F["j"])
    # There should be exactly one internal face with i>=0 and j>=0
    mask_internal = (i>=0) & (j>=0)
    assert mask_internal.sum() == 1
    fidx = np.nonzero(mask_internal)[0][0]
    loops = F["loops"]
    # internal face area should be approx 1 (unit square)
    area = F["area"][fidx]
    assert np.isclose(area, 1.0, atol=5e-2)
