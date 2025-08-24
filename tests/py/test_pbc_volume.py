import numpy as np
import voronoi3d as v3d

def test_pbc_two_atoms_volume_conservation():
    cfg = v3d.Config()
    cfg.min_M = 0.5
    # Simple cubic cell 1x1x1
    lat = v3d.Lattice(1.0, 1.0, 1.0, 90.0, 90.0, 90.0)
    pbc = v3d.TriclinicPBC(lat, (True, True, True))
    # Two atoms along x
    pbc.add_atoms([v3d.Vec3(0.25, 0.5, 0.5), v3d.Vec3(0.75, 0.5, 0.5)])
    T = v3d.plan_neighbors(pbc, cfg)
    M = np.full(len(T.i), 0.5, dtype=float)
    cells = v3d.tessellate_pairs(pbc, T, M, cfg)
    # Sum of volumes of unique atoms in unit cell should equal cell volume (1)
    Vsum = sum(c["volume"] for c in cells)
    assert np.isclose(Vsum, 1.0, atol=5e-2)
