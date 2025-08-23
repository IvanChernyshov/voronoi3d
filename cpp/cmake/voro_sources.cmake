# cpp/cmake/voro_sources.cmake

# Resolve relative to this file's directory, not the current source dir
set(VORO_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../third_party/voro++/src")

message(STATUS "Looking for Voro++ in: ${VORO_SRC_DIR}")

if (NOT EXISTS "${VORO_SRC_DIR}/voro++.hh")
  message(FATAL_ERROR
    "Voro++ submodule not found at ${VORO_SRC_DIR}.\n"
    "Expected header: ${VORO_SRC_DIR}/voro++.hh\n"
    "Tip: ensure the submodule is initialized and pinned:\n"
    "  git submodule update --init --recursive\n"
  )
endif()

# Public include path for Voro++
set(VORO_INCLUDE_DIRS "${VORO_SRC_DIR}")

# Optional sources that only exist on some snapshots
set(VORO_C_LOOPS    "")
set(VORO_WL_WRAPPER "")

if (EXISTS "${VORO_SRC_DIR}/c_loops.cc")
  set(VORO_C_LOOPS "${VORO_SRC_DIR}/c_loops.cc")
endif()

# If v_base_wl.cc exists, include it via a wrapper that puts it in namespace voro.
if (EXISTS "${VORO_SRC_DIR}/v_base_wl.cc")
  set(VORO_WL_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/../shims/voro_wl_wrapper.cc")
endif()

# Complete list of translation units
set(VORO_SOURCES
  ${VORO_SRC_DIR}/v_base.cc
  ${VORO_WL_WRAPPER}
  ${VORO_C_LOOPS}
  ${VORO_SRC_DIR}/cell.cc
  ${VORO_SRC_DIR}/common.cc
  ${VORO_SRC_DIR}/container.cc
  ${VORO_SRC_DIR}/container_prd.cc
  ${VORO_SRC_DIR}/pre_container.cc
  ${VORO_SRC_DIR}/unitcell.cc
  ${VORO_SRC_DIR}/v_compute.cc
  ${VORO_SRC_DIR}/wall.cc
)
