// $Id$
/**
 * @file AthContainers/test/DataVector_e_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for DataVector
 *
 * The @c DataVector regression tests are split into several pieces,
 * in order to reduce the memory required for compilation.
 */

// Disable this test in standalone mode:
#ifndef XAOD_STANDALONE

#undef NDEBUG


#include "DataVector_test.icc"


void test2_e()
{
  std::cout << "test2_e\n";
  do_test2<AAux, BAux> ();
}


int main()
{
  test2_e();
  return 0;
}

#else

int main() {
   return 0;
}

#endif // not XAOD_STANDALONE
