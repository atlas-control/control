/** @file LSFTimeKeeperSvc_test.cxx
 * @brief  unit test for ITimeKeeper simplest implementation
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov> -ATLAS Collaboration
 * $Id: LSFTimeKeeperSvc_test.cxx,v 1.2 2004-01-22 00:43:33 calaf Exp $
 **/

#include <cassert>
#include <cmath>  /* sqrt */
#include <iostream>
#include "TestTools/initGaudi.h"
#include "GaudiKernel/ISvcLocator.h"
#include "AthenaKernel/ITimeKeeper.h"
#include "LSFTimeKeeper/LSFTimeKeeperSvc.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace Athena_test;

int main() {
  cout << "*** LSFTimeKeeperSvc_test starts ***" <<endl;
  ISvcLocator* pSvcLoc(0);
  if (!initGaudi("LSFTimeKeeperSvc_test.txt", pSvcLoc)) {
    cerr << "This test can not be run" << endl;
    return 0;
  }  
  assert(pSvcLoc);

  //locate time keeper
  const bool CREATEIF(true);
  ITimeKeeper* pITK(0);
  assert( (pSvcLoc->service("LSFTimeKeeperSvc", pITK, CREATEIF)).isSuccess() );
  assert( pITK );
  //  cout <<  *pITK <<endl;

  LSFTimeKeeperSvc& tk(dynamic_cast<LSFTimeKeeperSvc&>(*pITK));
  assert(tk.timeX() + tk.timeL() == tk.allocTime());

  int iter(0);
  while (pITK->nextIter() && iter<10) {
    for (int i=0; i<5000000; ++i) atan(i); //waste some time
    ++iter;
  }

  assert(tk.timeX() + tk.timeL() == tk.allocTime());

  tk.finalize();

  //all done
  cout << "*** LSFTimeKeeperSvc_test OK ***" <<endl;
  return 0;
}
