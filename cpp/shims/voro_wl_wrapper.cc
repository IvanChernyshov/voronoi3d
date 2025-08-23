// Ensure Voro++ declarations are visible
#include "../third_party/voro++/src/voro++.hh"

// Include the big “wl” tables exactly as upstream provides them
namespace voro {
  #include "../third_party/voro++/src/v_base_wl.cc"
}
