# voronoi3d

Starter repository for the 3D Voronoi project.

## Installation

The easiest way to get a working developer setup is to use the OS-specific helper scripts in `scripts/`.
Each script assumes **you already created and activated a Python 3.10+ environment** (e.g. `python -m venv .venv` then activate).

These scripts will:
- install required build tools (CMake, Ninja, compiler toolchains),
- clone this repository with submodules,
- install the package in editable mode with dev extras (`pip install -e ".[dev]"`),
- (optionally) run tests to verify the build.

> We currently provide helper scripts for **Windows**, **Linux**, and **macOS**. If you prefer not to use the scripts, see the **Manual install** section below.

---

### Windows (PowerShell, Visual Studio Build Tools installed)

> Assumes you already installed **Visual Studio 2022 Build Tools** (with C++ build tools & Windows SDK) and activated a Python 3.10+ environment in the same PowerShell session.

Download & run the setup script:

```powershell
$script = "voronoi3d_win_setup.ps1"
Invoke-WebRequest -UseBasicParsing `
  -Uri https://raw.githubusercontent.com/IvanChernyshov/voronoi3d/main/scripts/voronoi3d_win_setup.ps1 `
  -OutFile $script

# Basic run (clones to ./voronoi3d, installs dev deps, runs tests)
.oronoi3d_win_setup.ps1

# Options:
#   -Branch <name>   pick a branch (default: main)
#   -Dir <path>      clone directory (default: ./voronoi3d)
#   -SkipTests       skip pytest
#   -Generator <g>   CMake generator (default: "Visual Studio 17 2022")
.oronoi3d_win_setup.ps1 -Branch dev -Dir .oro-dev -SkipTests
```

---

### Linux (Debian/Ubuntu, Fedora/RHEL, Arch/Manjaro, WSL)

> Assumes a Python 3.10+ environment is already activated. The script auto-detects the distro and installs compilers, CMake, Ninja, Git, and OpenMP support using your package manager. It also adds `-fPIC` on Linux to ensure the C++ objects are linkable into the Python extension.

Download & run:

```bash
curl -fsSL -o voronoi3d_linux_setup.sh   https://raw.githubusercontent.com/IvanChernyshov/voronoi3d/main/scripts/voronoi3d_linux_setup.sh
chmod +x voronoi3d_linux_setup.sh

# Basic run (clones to ./voronoi3d, installs dev deps, runs tests)
./voronoi3d_linux_setup.sh

# Options:
#   --branch <name>   pick a branch (default: main)
#   --dir <path>      clone directory (default: ./voronoi3d)
#   --no-test         skip pytest
./voronoi3d_linux_setup.sh --branch dev --dir ./voro-dev
```

> **WSL (Ubuntu)** is supported and works the same way.

---

### macOS (Apple Silicon & Intel)

> Assumes a Python 3.10+ environment is already activated. The script uses **Homebrew** to install CMake/Ninja/Git. macOS does not ship OpenMP by default; pass `--with-openmp` to install Homebrew `llvm` and build with OpenMP.

Download & run:

```bash
curl -fsSL -o voronoi3d_macos_setup.sh   https://raw.githubusercontent.com/IvanChernyshov/voronoi3d/main/scripts/voronoi3d_macos_setup.sh
chmod +x voronoi3d_macos_setup.sh

# Basic run (clones to ./voronoi3d, installs dev deps, runs tests)
./voronoi3d_macos_setup.sh

# With OpenMP (installs Homebrew llvm; sets CC/CXX & flags so CMake finds OpenMP)
./voronoi3d_macos_setup.sh --with-openmp

# Options:
#   --branch <name>   pick a branch (default: main)
#   --dir <path>      clone directory (default: ./voronoi3d)
#   --no-test         skip pytest
```

---

## Manual install (all platforms)

If you prefer to set things up yourself:

1) **Prerequisites**
   - Python **3.10+**
   - CMake **3.22+**
   - A C++20 compiler (MSVC 2022 / GCC 10+ / Clang 14+)
   - Git
   - (Optional) OpenMP toolchain:
     - Windows (MSVC): `/openmp` is enabled automatically if available
     - Linux: `libgomp` is usually present with GCC
     - macOS: install Homebrew `llvm` and configure OpenMP flags as needed

2) **Clone with submodules**
```bash
git clone --recurse-submodules https://github.com/IvanChernyshov/voronoi3d.git
cd voronoi3d
```

3) **Install (editable) with dev extras**
```bash
pip install -U pip
pip install -e ".[dev]"
```

4) **Run tests**
```bash
pytest -q
```

> If you run into linker errors on Linux about `-fPIC`, ensure position-independent code is enabled (this project does so via CMake) and you are using a recent compiler. Cleaning caches (`pip uninstall voronoi3d`, remove any temp build dirs) can also help.

---

## What gets built?

- A Python extension module `_core` (via CMake + pybind11 + scikit-build-core)
- Optional OpenMP acceleration is enabled automatically if the toolchain supports it.

---

## Development

- Install dev tools: `pip install -e ".[dev]"`
- Lint/format: `ruff check .` and `ruff format .`
- Type check: `mypy python`
- Run tests: `pytest -q`
- Pre-commit hooks: `pre-commit install` (then `pre-commit run -a`)

---

## Project layout

```
voronoi3d/
├─ python/                 # Python package sources (tests, examples)
├─ cpp/
│  ├─ core/                # pybind11 bindings
│  ├─ shims/               # adapter sources (e.g., WL wrapper)
│  ├─ cmake/               # CMake helper fragments (e.g., voro_sources.cmake)
│  └─ third_party/voro++/  # upstream voro++ (as a submodule)
├─ scripts/                # convenience install/build scripts (Win/Linux/macOS)
├─ pyproject.toml          # build configuration
└─ README.md
```

---

## Troubleshooting

- **Linux: `relocation ... can not be used when making a shared object; recompile with -fPIC`**  
  Use a recent GCC and ensure PIC is enabled. Our CMake sets PIC on Unix; the Linux script also enforces PIC.

- **macOS: OpenMP not found**  
  Use the macOS script with `--with-openmp` to install Homebrew `llvm` and configure the build with OpenMP.

- **Submodule missing (`voro++.hh` not found)**  
  Clone with submodules or run: `git submodule update --init --recursive`.

- **Stale CMake cache**  
  When switching compilers/generators, remove previous temp build dirs (created by scikit-build-core) and reinstall:  
  `pip uninstall voronoi3d` then `pip install -e ".[dev]"`.

---

## License

MIT (see `LICENSE`).
