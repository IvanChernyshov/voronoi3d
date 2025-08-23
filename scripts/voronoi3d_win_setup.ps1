<#
.SYNOPSIS
    One-shot Windows setup for the voronoi3d project from a normal PowerShell.
.DESCRIPTION
    - Imports the MSVC (Visual Studio 2022 Build Tools) environment
    - Clones or updates the repo (with submodules)
    - Installs build tooling (cmake, scikit-build-core, pybind11)
    - Installs the package in editable mode with dev extras
    - Runs tests
.PARAMETER RepoUrl
    Git URL to clone. Default: https://github.com/IvanChernyshov/voronoi3d.git
.PARAMETER Branch
    Git branch to checkout. Default: main
.PARAMETER DirName
    Destination folder name. Defaults to the repo name in the URL (voronoi3d).
.EXAMPLE
    .\voronoi3d_win_setup.ps1
.EXAMPLE
    .\voronoi3d_win_setup.ps1 -Branch dev
#>

param(
    [string]$RepoUrl = "https://github.com/IvanChernyshov/voronoi3d.git",
    [string]$Branch = "main",
    [string]$DirName = ""
)

$ErrorActionPreference = "Stop"

function Write-Section($msg) {
    Write-Host ""
    Write-Host "==== $msg ====" -ForegroundColor Cyan
}

function Ensure-Git {
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "Git is not installed or not in PATH. Install Git for Windows and retry."
    }
    $gitVersion = git --version
    Write-Host "Using $gitVersion"
}

function Ensure-Python310Plus {
    if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
        throw "Python is not in PATH. Activate your Python 3.10+ environment first."
    }
    $ver = & python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}')"
    if (-not $ver) { throw "Could not determine Python version." }
    Write-Host "Detected Python $ver"
    $parts = $ver.Split(".") | ForEach-Object {[int]$_}
    if ($parts[0] -lt 3 -or ($parts[0] -eq 3 -and $parts[1] -lt 10)) {
        throw "Python 3.10+ is required. Current: $ver"
    }
}

function Find-VsDevCmd {
    $candidates = @(
        "$Env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles(x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles(x86)\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles(x86)\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
        "$Env:ProgramFiles(x86)\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
    )
    foreach ($path in $candidates) {
        if (Test-Path $path) { return $path }
    }
    # Try vswhere
    $vswhere = "$Env:ProgramFiles(x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -property installationPath 2>$null
        if ($LASTEXITCODE -eq 0 -and $installPath) {
            $cmd = Join-Path $installPath "Common7\Tools\VsDevCmd.bat"
            if (Test-Path $cmd) { return $cmd }
        }
    }
    return $null
}

function Import-VSDevEnvironment {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Host "MSVC already available in PATH."
        return
    }
    $vsDevCmd = Find-VsDevCmd
    if (-not $vsDevCmd) {
        Write-Warning "Could not locate VsDevCmd.bat. If MSVC is already in PATH, continuing. Otherwise, install VS 2022 Build Tools."
        return
    }
    Write-Host "Importing Visual Studio environment from: $vsDevCmd"
    $tmp = New-TemporaryFile
    # Use cmd to run VsDevCmd and dump environment via 'set'
    cmd /c "`"$vsDevCmd`" -arch=x64 -host_arch=x64 >nul & set" | Out-File -FilePath $tmp -Encoding ASCII
    Get-Content $tmp | ForEach-Object {
        $pair = $_ -split '=', 2
        if ($pair.Length -eq 2) { [System.Environment]::SetEnvironmentVariable($pair[0], $pair[1]) }
    }
    Remove-Item $tmp -ErrorAction SilentlyContinue

    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        $clLine = cl 2>&1 | Select-String -Pattern "Version" -First 1
        Write-Host "MSVC ready: $clLine"
    } else {
        Write-Warning "cl.exe still not on PATH. Build may fail. Ensure Visual Studio Build Tools are installed."
    }
}

function Derive-DirNameFromRepo($url) {
    try {
        $name = [System.IO.Path]::GetFileNameWithoutExtension($url.TrimEnd('/'))
        if ($name) { return $name } else { return "voronoi3d" }
    } catch { return "voronoi3d" }
}

# ------------------- MAIN -------------------
Write-Section "Checking prerequisites"
Ensure-Git
Ensure-Python310Plus
Import-VSDevEnvironment

if (-not $DirName -or $DirName.Trim() -eq "") {
    $DirName = Derive-DirNameFromRepo $RepoUrl
}

Write-Section "Cloning or updating repository"
if (-not (Test-Path $DirName)) {
    Write-Host "Cloning $RepoUrl into .\$DirName (branch: $Branch)"
    git clone --recurse-submodules --branch $Branch $RepoUrl $DirName
} else {
    Write-Host "Directory '$DirName' exists, updating..."
    pushd $DirName
    git fetch origin
    git checkout $Branch
    git pull --ff-only origin $Branch
    git submodule update --init --recursive
    popd
}

Write-Section "Installing build tooling"
# Force MSBuild generator to avoid Ninja unless you prefer Ninja
$Env:CMAKE_GENERATOR = "Visual Studio 17 2022"
$Env:CMAKE_GENERATOR_PLATFORM = "x64"

# Upgrade pip & install core build tools for scikit-build-core
python -m pip install -U pip
pip install -U cmake scikit-build-core pybind11

Write-Section "Editable install with dev extras"
pushd $DirName
pip install -e ".[dev]"

Write-Section "Running tests"
pytest -q

popd

Write-Host ""
Write-Host "All done! 🎉" -ForegroundColor Green
