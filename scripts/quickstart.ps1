Param(
  [string]$Pin = "<PIN_COMMIT_HASH>"
)
git init
git config core.longpaths true
git add -A
git commit -m "[M1] initial skeleton"
git submodule add https://github.com/chr1shr/voro cpp/third_party/voro++
Push-Location cpp/third_party/voro++
if ($Pin -ne "<PIN_COMMIT_HASH>") {
  git checkout $Pin
}
Pop-Location
git commit -m "Add Voro++ submodule (pinned to $Pin)"
python -m pip install -U pip
pip install -r python/requirements-dev.txt
pip install -e .
pytest -q
