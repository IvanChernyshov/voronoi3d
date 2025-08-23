cmake_minimum_required(VERSION 3.22)
project(voronoi3d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/O2 /DNDEBUG /EHsc /bigobj)
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # /MD
endif()

# --- Voro++ sources & includes
include(cpp/cmake/voro_sources.cmake)

add_library(voro_dep OBJECT ${VORO_SOURCES})
target_include_directories(voro_dep PRIVATE ${VORO_INCLUDE_DIRS})

# OpenMP (optional)
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  target_link_libraries(voro_dep PRIVATE OpenMP::OpenMP_CXX)
  target_compile_definitions(voro_dep PRIVATE VORO_OPENMP=1)
endif()

# --- Python & pybind11 (robust discoverability) ---
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c "import pybind11, sys; print(pybind11.get_cmake_dir())"
  OUTPUT_VARIABLE pybind11_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(APPEND CMAKE_PREFIX_PATH "${pybind11_dir}")
find_package(pybind11 CONFIG REQUIRED)
# --------------------------------------------------

pybind11_add_module(_core MODULE
  cpp/core/bindings.cpp
  $<TARGET_OBJECTS:voro_dep>
)
target_include_directories(_core PRIVATE ${VORO_INCLUDE_DIRS})
target_compile_definitions(_core PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES=1)

# Allow the duplicate constant 'voro::voro_base::wl' at link time on MSVC
if (MSVC)
  target_link_options(_core    PRIVATE "/FORCE:MULTIPLE")
  target_link_options(voro_dep PRIVATE "/IGNORE:4006")
endif()

install(TARGETS _core DESTINATION voronoi3d)
