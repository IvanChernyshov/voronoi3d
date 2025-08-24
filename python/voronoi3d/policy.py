from __future__ import annotations
import numpy as np
from typing import Dict, Tuple

def symmetrize_M(table: dict, M: np.ndarray) -> np.ndarray:
    """
    Return M' such that for every unordered pair+image, M_ji = 1 - M_ij.
    Assumes 'table' contains oriented arrays: i, j, img (E×3). If both (i,j,img) and (j,i,-img)
    rows exist, enforce complementarity by averaging them.
    """
    i = np.asarray(table["i"], dtype=np.int64)
    j = np.asarray(table["j"], dtype=np.int64)
    img = np.asarray(table["img"], dtype=np.int64)
    M = np.asarray(M, dtype=float)
    assert M.shape[0] == i.shape[0]

    # Build mapping from (i,j,na,nb,nc) → row
    keys: Dict[Tuple[int,int,int,int,int], int] = {}
    for idx, (ii, jj, (na, nb, nc)) in enumerate(zip(i, j, img)):
        keys[(int(ii), int(jj), int(na), int(nb), int(nc))] = idx

    out = M.copy()
    for idx, (ii, jj, (na, nb, nc)) in enumerate(zip(i, j, img)):
        k_op = (int(jj), int(ii), int(-na), int(-nb), int(-nc))
        jdx = keys.get(k_op, None)
        if jdx is not None:
            m = 0.5*(out[idx] + (1.0 - out[jdx]))
            out[idx] = m
            out[jdx] = 1.0 - m
    return out
