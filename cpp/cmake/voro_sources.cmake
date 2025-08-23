# Paths to bundled Voro++ sources
set(VORO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../third_party/voro++/src")

# Public includes for anything that needs Voro++
set(VORO_INCLUDE_DIRS "${VORO_ROOT}")

# IMPORTANT: do NOT add the shim; v_base.cc already includes v_base_wl.cc and defines 'voro_base::wl'.
# Listing only the upstream Voro++ .cc files we actually need:
set(VORO_SOURCES
  ${VORO_ROOT}/v_base.cc
  ${VORO_ROOT}/c_loops.cc
  ${VORO_ROOT}/cell.cc
  ${VORO_ROOT}/common.cc
  ${VORO_ROOT}/container.cc
  ${VORO_ROOT}/container_prd.cc
  ${VORO_ROOT}/pre_container.cc
  ${VORO_ROOT}/unitcell.cc
  ${VORO_ROOT}/v_compute.cc
  ${VORO_ROOT}/wall.cc
)
