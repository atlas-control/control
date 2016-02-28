/* -*- C++ -*- */
#ifndef PILEUPCOMPS_PILEUPTOOLSALG_H
#define PILEUPCOMPS_PILEUPTOOLSALG_H
/** @file PileUpToolsAlg.h
 * @brief an algorithm to steer the IPileUpTools
 * @author Paolo Calafiura - ATLAS Collaboration
 */

#include "AthenaBaseComps/AthAlgorithm.h"

#include "PileUpTools/IPileUpTool.h"
#include "xAODEventInfo/EventInfo.h" /* SubEvent*/

#include "GaudiKernel/ToolHandle.h"



/////////////////////////////////////////////////////////////////////////////

class PileUpToolsAlg: public AthAlgorithm {
public:
  PileUpToolsAlg(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  StatusCode clearXing(SubEventIterator& fEvt,
                       SubEventIterator& lEvt);
  ToolHandleArray<IPileUpTool> m_puTools;
};


#endif
