#include "CxxUtils/cPtrAccessSEGVHandler.h"
#include "CxxUtils/PtrAccessSEGVHandler.h"
namespace {
  PtrAccessSEGVHandler* s_pHandler(0);
}

void setPtrAccessSEGVHandler(PtrAccessSEGVHandler* h) { s_pHandler=h; }
void cPtrAccessSEGVHandler(int signal, siginfo_t* si, void* old) {
  s_pHandler->handle(signal, si, old);
}
