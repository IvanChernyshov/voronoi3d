cmake_minimum_required(VERSION 3.22)
project(voronoi3d LANGUAGES CXX)

if (MSVC)
  add_compile_options(/O2 /DNDEBUG /EHsc /bigobj)
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # /MD
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let object libraries build as PIC on non-Windows (needed for shared module on Linux/macOS)
if (NOT MSVC)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# --- pybind11: robust discovery even when installed via pip ---
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

# Ask Python where pybind11's CMake config lives (pip-installed)
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c "import pybind11, sys; print(pybind11.get_cmake_dir())"
  OUTPUT_VARIABLE pybind11_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(APPEND CMAKE_PREFIX_PATH "${pybind11_dir}")

find_package(pybind11 CONFIG REQUIRED)
# --- end pybind11 setup ---

include(cpp/cmake/voro_sources.cmake)

add_library(voro_dep OBJECT ${VORO_SOURCES})
target_include_directories(voro_dep PRIVATE ${VORO_INCLUDE_DIRS})

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  target_link_libraries(voro_dep PRIVATE OpenMP::OpenMP_CXX)
  target_compile_definitions(voro_dep PRIVATE VORO_OPENMP=1)
endif()

pybind11_add_module(_core MODULE
    cpp/bindings/py_module.cpp
)

# If you still build a voro++ object library, add its objects:
if (TARGET voro_dep)
  target_sources(_core PRIVATE $<TARGET_OBJECTS:voro_dep>)
  target_include_directories(_core PRIVATE ${VORO_INCLUDE_DIRS})
endif()

target_include_directories(_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp)
target_compile_definitions(_core PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES=1)

install(TARGETS _core DESTINATION voronoi3d)
