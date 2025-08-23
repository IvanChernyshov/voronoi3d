#!/usr/bin/env bash
# voronoi3d_macos_setup.sh
# One-shot installer for macOS (Apple Silicon & Intel).
# Assumes: you've already created & activated a Python 3.10+ environment.
# Note: I can't run this on macOS here; if something breaks, share the logs.

set -euo pipefail

### Defaults ###
BRANCH="main"
CLONE_DIR="voronoi3d"
RUN_TESTS=1
WITH_OPENMP=0

usage() {
  cat <<'USAGE'
Usage: ./voronoi3d_macos_setup.sh [--branch <name>] [--dir <path>] [--no-test] [--with-openmp]

Options:
  --branch <name>    Git branch to clone (default: main)
  --dir <path>       Clone directory (default: ./voronoi3d)
  --no-test          Skip running tests after install
  --with-openmp      Install Homebrew LLVM and build with OpenMP support

This script expects that you have already activated a Python 3.10+ environment.
USAGE
}

### Parse args ###
while [[ $# -gt 0 ]]; do
  case "$1" in
    --branch) BRANCH="$2"; shift 2;;
    --dir) CLONE_DIR="$2"; shift 2;;
    --no-test) RUN_TESTS=0; shift;;
    --with-openmp) WITH_OPENMP=1; shift;;
    -h|--help) usage; exit 0;;
    *) echo "Unknown arg: $1"; usage; exit 2;;
  esac
done

### Check Python ###
if ! command -v python3 >/dev/null 2>&1; then
  echo "python3 not found on PATH. Please install Python 3.10+ and activate your environment."
  exit 1
fi

py_ver="$(python3 -c 'import sys; print(f\"{sys.version_info.major}.{sys.version_info.minor}\")')"
py_major="${py_ver%%.*}"
py_minor="${py_ver##*.}"

if (( py_major < 3 || (py_major == 3 && py_minor < 10) )); then
  echo "Python $py_ver detected. Please activate Python 3.10+ environment first."
  exit 1
fi

### Check Homebrew ###
if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required. Install from https://brew.sh/, then re-run this script."
  exit 1
fi

echo "==> Ensuring required Homebrew packages (git, cmake, ninja)"
brew update
brew install git cmake ninja || true

if (( WITH_OPENMP == 1 )); then
  echo "==> Enabling OpenMP via Homebrew LLVM"
  brew install llvm
  LLVM_PREFIX="$(brew --prefix llvm)"
  export CC="$LLVM_PREFIX/bin/clang"
  export CXX="$LLVM_PREFIX/bin/clang++"
  export LDFLAGS="-L$LLVM_PREFIX/lib ${LDFLAGS:-}"
  export CPPFLAGS="-I$LLVM_PREFIX/include ${CPPFLAGS:-}"
  export PATH="$LLVM_PREFIX/bin:$PATH"
  export CMAKE_PREFIX_PATH="$LLVM_PREFIX:${CMAKE_PREFIX_PATH:-}"
  echo "    Using clang at: $CC"
fi

### Clone repo (with submodules) ###
if [[ ! -d "$CLONE_DIR/.git" ]]; then
  echo "==> Cloning repository branch '$BRANCH' into '$CLONE_DIR'"
  git clone --recurse-submodules -b "$BRANCH" https://github.com/IvanChernyshov/voronoi3d.git "$CLONE_DIR"
else
  echo "==> Directory '$CLONE_DIR' already contains a Git repo; pulling latest and updating submodules"
  (cd "$CLONE_DIR" && git fetch && git checkout "$BRANCH" && git pull && git submodule update --init --recursive)
fi

cd "$CLONE_DIR"

# Prefer Ninja for faster local builds
export CMAKE_GENERATOR="Ninja"

echo "==> Upgrading pip and building/installing voronoi3d in editable mode with dev extras"
python3 -m pip install -U pip wheel
pip install -e ".[dev]"

if (( RUN_TESTS == 1 )); then
  echo "==> Running tests"
  pytest -q
else
  echo "==> Skipping tests"
fi

echo ""
echo "✅ Done! Repo at: $(pwd)"
echo "   Installed in editable mode. Try:  python -c \"import voronoi3d, numpy as np; print('ok', voronoi3d.__version__)\""
