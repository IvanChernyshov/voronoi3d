set(VORO_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/voro++/src")

if (NOT EXISTS "${VORO_BASE}/voro++.hh")
  message(FATAL_ERROR "Voro++ submodule not found. Please add it and pin to a commit or tag.")
endif()

set(VORO_INCLUDE_DIRS "${VORO_BASE}")

set(VORO_SOURCES
    ${VORO_BASE}/cell.cc
    ${VORO_BASE}/common.cc
    ${VORO_BASE}/container.cc
    ${VORO_BASE}/container_prd.cc
    ${VORO_BASE}/pre_container.cc
    ${VORO_BASE}/unitcell.cc
    ${VORO_BASE}/v_compute.cc
    ${VORO_BASE}/wall.cc
)
