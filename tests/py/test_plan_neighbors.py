import numpy as np
import voronoi3d as v3d

def test_plan_neighbors_box_simple():
    cfg = v3d.Config()
    cfg.min_M = 0.25
    bounds = v3d.BoxBounds(v3d.Vec3(0,0,0), v3d.Vec3(1,1,1))
    box = v3d.BoxContainer(bounds)
    box.add_atoms([v3d.Vec3(0.2,0.5,0.5), v3d.Vec3(0.8,0.5,0.5)])
    T = v3d.plan_neighbors(box, cfg)
    assert T.size > 0
    # Expect two oriented rows (0->1 and 1->0)
    ii = np.array(T.i, dtype=int)
    jj = np.array(T.j, dtype=int)
    assert ((ii==0) & (jj==1)).any()
    assert ((ii==1) & (jj==0)).any()
