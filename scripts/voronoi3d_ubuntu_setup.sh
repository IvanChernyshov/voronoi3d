#!/usr/bin/env bash
#
# voronoi3d_ubuntu_setup.sh
# One-shot setup for Ubuntu/WSL (apt-based). Assumes a Python 3.10+ env is ALREADY activated.
# - Installs system build tools (gcc/g++, cmake, ninja, git)
# - Clones the repo (with submodules) or updates an existing clone
# - Builds & installs voronoi3d in editable mode with dev extras
# - Optionally runs tests
#
# Usage:
#   bash voronoi3d_ubuntu_setup.sh [--branch <name>] [--repo <url>] [--dir <path>] [--no-test]
#
# Examples:
#   bash voronoi3d_ubuntu_setup.sh
#   bash voronoi3d_ubuntu_setup.sh --branch dev
#   bash voronoi3d_ubuntu_setup.sh --repo git@github.com:IvanChernyshov/voronoi3d.git --dir myclone
#
set -euo pipefail

# ---- args ----
BRANCH="main"
REPO_URL="https://github.com/IvanChernyshov/voronoi3d.git"
REPO_DIR="voronoi3d"
RUN_TESTS=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --branch) BRANCH="$2"; shift 2;;
    --repo)   REPO_URL="$2"; shift 2;;
    --dir)    REPO_DIR="$2"; shift 2;;
    --no-test) RUN_TESTS=0; shift ;;
    *) echo "Unknown argument: $1" >&2; exit 2;;
  esac
done

echo "=== voronoi3d Ubuntu setup ==="
echo "Python: $(python --version 2>&1 || true)"
echo "Pip:    $(pip --version 2>&1 || true)"
echo "Branch: $BRANCH"
echo "Repo:   $REPO_URL"
echo "Dir:    $REPO_DIR"
echo

# ---- sanity: Ubuntu/apt present ----
if ! command -v apt-get >/dev/null 2>&1; then
  echo "This script supports apt-based systems (Ubuntu/WSL). Aborting." >&2
  exit 1
fi

# ---- sanity: Python >= 3.10 ----
if ! python - <<'PYCHK'
import sys
maj, min = sys.version_info[:2]
ok = (maj > 3) or (maj == 3 and min >= 10)
raise SystemExit(0 if ok else 1)
PYCHK
then
  echo "Python >= 3.10 is required in the CURRENT environment. Aborting." >&2
  exit 1
fi

# ---- install system packages ----
echo "Installing system build dependencies (sudo may prompt for your password)..."
if [[ $EUID -ne 0 ]] && command -v sudo >/dev/null 2>&1; then
  SUDO="sudo"
else
  SUDO=""
fi

$SUDO apt-get update -y
$SUDO apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  ninja-build \
  git \
  ca-certificates

# Ensure cmake is recent enough (>= 3.22). If not, install a newer one via pip.
REQ_CMAKE="3.22.0"
have_cmake_ver="$(cmake --version 2>/dev/null | head -n1 | awk '{print $3}')"
if [[ -n "${have_cmake_ver}" ]]; then
  if dpkg --compare-versions "$have_cmake_ver" "lt" "$REQ_CMAKE"; then
    echo "System CMake $have_cmake_ver < $REQ_CMAKE; installing a newer CMake via pip..."
    python -m pip install -U cmake
  fi
else
  echo "CMake not found in PATH; installing via pip..."
  python -m pip install -U cmake
fi

# ---- clone or update repo ----
if [[ ! -d "$REPO_DIR/.git" ]]; then
  echo "Cloning $REPO_URL (branch $BRANCH) into $REPO_DIR ..."
  git clone --recurse-submodules -b "$BRANCH" "$REPO_URL" "$REPO_DIR"
else
  echo "Repository exists at $REPO_DIR. Updating..."
  git -C "$REPO_DIR" fetch --tags --prune origin
  git -C "$REPO_DIR" checkout "$BRANCH"
  git -C "$REPO_DIR" pull --ff-only
  git -C "$REPO_DIR" submodule update --init --recursive
fi

cd "$REPO_DIR"

# ---- build env settings (GNU toolchain + Ninja) ----
export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"
export CMAKE_GENERATOR="${CMAKE_GENERATOR:-Ninja}"
# parallelize builds
if command -v nproc >/dev/null 2>&1; then
  export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-$(nproc)}"
fi

# ---- Python build ----
echo "Upgrading pip and installing voronoi3d (editable) + dev extras..."
python -m pip install -U pip
pip install -e ".[dev]"

# ---- smoke test ----
if [[ "$RUN_TESTS" -eq 1 ]]; then
  echo
  echo "Running tests..."
  pytest -q || { echo "Tests failed." >&2; exit 1; }
fi

echo
echo "✅ Done. Activate the same Python environment next time and you can:"
echo "   - Run tests:        pytest -q"
echo "   - Lint/format:      ruff check . && ruff format ."
echo "   - Type-check:       mypy python"
