#ifndef GAUDIHIVE_GRAPHEXECUTIONTASK_H
#define GAUDIHIVE_GRAPHEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"

#include "ForwardSchedulerSvc.h"

// External libs
#include "tbb/task.h"
#include <vector>

class GraphExecutionTask: public tbb::task {
public:
  GraphExecutionTask( std::vector< IAlgorithm* > const& algorithm, 
                    std::vector< unsigned int > const& algoIndex,
		    EventContext* inputContext,
                    ISvcLocator* svcLocator,
                    ForwardSchedulerSvc* schedSvc):
    m_eventContext(inputContext),
    m_algorithms(algorithm),
    m_algoIndices(algoIndex),
    m_schedSvc(schedSvc),
    m_serviceLocator(svcLocator){};
  virtual tbb::task* execute();
private:  
  const std::vector< IAlgorithm* > m_algorithms;
  EventContext * m_eventContext;
  const std::vector< unsigned int > m_algoIndices;
  // For the callbacks on task finishing
  SmartIF<ForwardSchedulerSvc> m_schedSvc;
  SmartIF<ISvcLocator> m_serviceLocator;
};

#endif
