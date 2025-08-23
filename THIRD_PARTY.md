# Third-Party Notices

This project bundles **Voro++** as a Git submodule at `cpp/third_party/voro++`.

- **Upstream:** https://github.com/chr1shr/voro

- **Pinned commit:** `<PIN_COMMIT_HASH>`.

- **License:** BSD-3-Clause (LBNL variant). See `LICENSES/VORO_PLUSPLUS` (verbatim copy of upstream LICENSE).

- **Authors:** Chris H. Rycroft and contributors.


## How we use it

We compile selected Voro++ sources directly into our Python extension (no separate shared library).  
Include path used by CMake: `cpp/third_party/voro++/src`.


## Modifications

Future milestones may add new policy/containers in separate files (MIT), while any edits to upstream files remain under the Voro++ license.
See `LICENSE` for our code and `LICENSES/VORO_PLUSPLUS` for Voro++.


## Updating the pin

To move to a newer release or commit:

```bash
git -C cpp/third_party/voro++ fetch --tags
git -C cpp/third_party/voro++ checkout <new-tag-or-commit>
git add cpp/third_party/voro++
git commit -m "Update Voro++ submodule to <new-tag-or-commit>"
```

Then update the “Pinned commit” and refresh LICENSES/VORO_PLUSPLUS if upstream changes their license text.


## Cloning this repo

Clone with submodules:

```bash
git clone --recurse-submodules <this-repo-url>
# or, after a normal clone:
git submodule update --init --recursive
```
