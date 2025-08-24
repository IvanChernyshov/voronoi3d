import numpy as np
import voronoi3d as v3d

def test_smoke_plan_neighbors():
    cfg = v3d.Config()
    cfg.min_M = 0.25

    box = v3d.BoxContainer(v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(1,1,1)))
    box.add_atoms([v3d.Vec3(0.2,0.5,0.5), v3d.Vec3(0.8,0.5,0.5)])

    T = v3d.plan_neighbors(box, cfg)
    # two atoms → two oriented rows (0→1 and 1→0)
    assert T.size == 2
    assert set(zip(T.i, T.j)) == {(0,1), (1,0)}
