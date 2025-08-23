# voronoi3d

Starter repository for the 3D Voronoi project.

`voronoi3d` ships a Python extension (via **pybind11** + **CMake** + **scikit-build-core**) and a small Python API.  
Tested on **Windows (MSVC)** with **Python 3.10**.

---

## Requirements (Windows)

- **Python** 3.10 (conda recommended)
- **Visual Studio 2022 Build Tools** with:
  - *MSVC v143* C++ toolset
  - *Windows 10/11 SDK*
- **Git** (for submodules)
- **CMake ≥ 3.22** (auto-installed via pip if missing)

> Tip: You can build from a normal PowerShell/CMD as long as VS Build Tools are installed.

---

## Installation — fresh clone

```bash
# 1) Clone with submodules
git clone --recursive <your-repo-url> voronoi3d
cd voronoi3d

# If you forgot --recursive:
# git submodule update --init --recursive

# 2) Create and activate a clean environment
conda create -n voro python=3.10 -y
conda activate voro

# 3) Build dependencies (CMake will be installed if needed)
python -m pip install -U pip
python -m pip install -U scikit-build-core pybind11 cmake

# 4) Build and install in editable mode
python -m pip install -e .
```

### Verify

```bash
python -c "import voronoi3d, voronoi3d._core; print('voronoi3d OK')"
```

(Optional) run tests:

```bash
python -m pip install pytest
pytest -q
```

---

## Installation — already cloned repo

```bash
cd <path-to>/voronoi3d

# Ensure third-party code is present
git submodule update --init --recursive

conda create -n voro python=3.10 -y
conda activate voro

python -m pip install -U pip
python -m pip install -U scikit-build-core pybind11 cmake

python -m pip install -e .
python -c "import voronoi3d, voronoi3d._core; print('voronoi3d OK')"
```

---

## Troubleshooting

- **Missing MSVC/SDK:** Install *Visual Studio 2022 Build Tools* with the C++ workload and Windows SDK.
- **Submodule not found:**  
  `git submodule update --init --recursive`
- **Stale build artifacts:** If you switch compilers/Python versions, reinstall in a fresh env:
  ```bash
  pip uninstall -y voronoi3d
  conda remove -n voro --all -y
  ```
  then follow the steps above.

---

## Development notes

- The C++ core uses OpenMP automatically if available.
- The Voro++ sources are vendored under `cpp/third_party/voro++`.
- The Python extension module is built as `voronoi3d/_core.*.pyd`.

---

## License

TBD.
