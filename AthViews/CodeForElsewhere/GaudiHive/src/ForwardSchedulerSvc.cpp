// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include <GaudiAlg/GaudiAlgorithm.h>
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include <GaudiKernel/IDataManagerSvc.h>
#include "tbb/task.h"
//#include "tbb/flow_graph.h"
#include "boost/thread.hpp"

// C++
#include <unordered_set>
#include <algorithm>
#include <map>
#include <sstream>
#include <queue>
//#include <iostream>

// Local
#include "ForwardSchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "GraphExecutionTask.h"
#include "AlgResourcePool.h"
#include "EFGraphVisitors.h"

std::mutex ForwardSchedulerSvc::m_ssMut;
std::list<ForwardSchedulerSvc::SchedulerState> ForwardSchedulerSvc::m_sState;

// External libs
// DP waiting for the TBB service
#include "tbb/task_scheduler_init.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(ForwardSchedulerSvc)

//===========================================================================
// Infrastructure methods

ForwardSchedulerSvc::ForwardSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
base_class(name,svcLoc),
  m_isActive(INACTIVE),
 m_algosInFlight(0),
 m_updateNeeded(true),
  m_first(true), m_checkDeps(false)

{
  declareProperty("MaxEventsInFlight", m_maxEventsInFlight = 0 );
  declareProperty("ThreadPoolSize", m_threadPoolSize = -1 );
  declareProperty("WhiteboardSvc", m_whiteboardSvcName = "EventDataSvc" );
  declareProperty("MaxAlgosInFlight", m_maxAlgosInFlight = 0, "Taken from the whiteboard. Deprecated" );
  // XXX: CF tests. Temporary property to switch between ControlFlow implementations
  declareProperty("useGraphFlowManagement", m_CFNext = false );
  declareProperty("DataFlowManagerNext", m_DFNext = false );
  declareProperty("SimulateExecution", m_simulateExecution = false );
  declareProperty("Optimizer", m_optimizationMode = "", 
		  "The following modes are currently available: PCE, COD, DRE, E" );
  declareProperty("DumpIntraEventDynamics", m_dumpIntraEventDynamics = false, 
		  "Dump intra-event concurrency dynamics to csv file" );

  /// Deprecated
  declareProperty("AlgosDependencies", m_algosDependencies);

  declareProperty("CheckDependencies", m_checkDeps = false);

}

//---------------------------------------------------------------------------
ForwardSchedulerSvc::~ForwardSchedulerSvc(){}
//---------------------------------------------------------------------------

/**
 * Here, among some "bureaucracy" operations, the scheduler is activated,
 * executing the activate() function in a new thread.
 * In addition the algorithms list is acquired from the algResourcePool.
 **/
StatusCode ForwardSchedulerSvc::initialize(){

  // Initialise mother class (read properties, ...)
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess())
    warning () << "Base class could not be initialized" << endmsg;

  // Get hold of the TBBSvc. This should initialize the thread pool
  m_threadPoolSvc = serviceLocator()->service("ThreadPoolSvc");
  if (!m_threadPoolSvc.isValid()) 
    fatal() << "Error retrieving ThreadPoolSvc" << endreq;


  // Activate the scheduler in another thread.
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread (std::bind(&ForwardSchedulerSvc::activate,
				    this));

  while(m_isActive != ACTIVE) {
    if (m_isActive == FAILURE) {
      fatal() << "Terminating initialization" << endmsg;
      return StatusCode::FAILURE;
    } else {
    info() << "Waiting for ForwardSchedulerSvc to activate" << endmsg;
    sleep(1);
  }
  }

  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  if (!m_algResourcePool.isValid())
    error() << "Error retrieving AlgoResourcePool" << endmsg;

  // Get Whiteboard
  m_whiteboard = serviceLocator()->service(m_whiteboardSvcName);
  if (!m_whiteboard.isValid())
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;

  // Check the MaxEventsInFlight parameters and react
  // Deprecated for the moment
  size_t numberOfWBSlots = m_whiteboard->getNumberOfStores();
  if (m_maxEventsInFlight!=0){
    warning() << "Property MaxEventsInFlight was set. This works but it's deprecated. "
              << "Please migrate your code options files." << endmsg;

    if (m_maxEventsInFlight != (int)numberOfWBSlots){
      warning() << "In addition, the number of events in flight ("
                << m_maxEventsInFlight << ") differs from the slots in the whiteboard ("
                << numberOfWBSlots << "). Setting the number of events in flight to "
                << numberOfWBSlots << endmsg;
    }
  }

  // Align the two quantities
  m_maxEventsInFlight = numberOfWBSlots;

  // Set the number of free slots
  m_freeSlots=m_maxEventsInFlight;

  if (m_algosDependencies.size() != 0) {
    warning() << " ##### Property AlgosDependencies is deprecated and ignored."
	      << " FIX your job options #####" << endmsg;
  }

  // Get the list of algorithms
  const std::list<IAlgorithm*>& algos = m_algResourcePool->getFlatAlgList();
  const unsigned int algsNumber = algos.size();
  info() << "Found " <<  algsNumber << " algorithms" << endmsg;

  /* Dependencies
   1) Look for handles in algo, if none
   2) Assume none are required
  */

  DataObjIDColl globalInp, globalOutp;

  info() << "Data Dependencies for Algorithms:";

  std::vector<DataObjIDColl> m_algosDependencies;
    for (IAlgorithm* ialgoPtr : algos) {
      Algorithm* algoPtr = dynamic_cast<Algorithm*>(ialgoPtr);
      if (nullptr == algoPtr)
        fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;

    info() << "\n  " << algoPtr->name();
    
    // FIXME
    DataObjIDColl algoDependencies;
    if (!algoPtr->inputDataObjs().empty() || !algoPtr->outputDataObjs().empty()) {
      for (auto id : algoPtr->inputDataObjs()) {
	info() << "\n    o INPUT  " << id;
	algoDependencies.insert(id);
	globalInp.insert(id);
          }
      for (auto id : algoPtr->outputDataObjs()) {
	info() << "\n    o OUTPUT " << id;
	globalOutp.insert(id);
        }
      } else {
      info()   << "\n      none";
      }
      m_algosDependencies.emplace_back(algoDependencies);
    }
  info() << endmsg;

  // Fill the containers to convert algo names to index
  m_algname_vect.reserve(algsNumber);
  unsigned int index=0;
  for (IAlgorithm* algo : algos){
    const std::string& name = algo->name();
    m_algname_index_map[name]=index;
    m_algname_vect.emplace_back(name);
    index++;
  }


  // Check if we have unmet global input dependencies
  if (m_checkDeps) {
    DataObjIDColl unmetDep;
    for (auto o : globalInp) {
      if (globalOutp.find(o) == globalOutp.end()) {
	unmetDep.insert(o);
      }
    }

    if (unmetDep.size() > 0) {
      fatal() << "The following unmet INPUT data dependencies were found: ";
      for (auto &o : unmetDep) {
	fatal() << "\n   o " << o << "    required by Algorithm: ";
	for (size_t i =0; i<m_algosDependencies.size(); ++i) {
	  if ( m_algosDependencies[i].find( o ) != m_algosDependencies[i].end() ) {
	    fatal() << "\n       * " << m_algname_vect[i];
	  }
	}
      }
      fatal() << endmsg;
      return StatusCode::FAILURE;
    } else {
      info() << "No unmet INPUT data dependencies were found" << endmsg;
    }
  }

  // prepare the control flow part
  if (m_CFNext) m_DFNext = true; //force usage of new data flow machinery when new control flow is used
  if (!m_CFNext && !m_optimizationMode.empty()) {
    fatal() << "Execution optimization is only available with the graph-based execution flow management" << endmsg;
    return StatusCode::FAILURE;
  }
  const AlgResourcePool* algPool = 
    dynamic_cast<const AlgResourcePool*>(m_algResourcePool.get());
  sc = m_efManager.initialize(algPool->getExecutionFlowGraph(), m_algname_index_map, 
			      m_eventSlots, m_optimizationMode);
  unsigned int controlFlowNodeNumber = 
    m_efManager.getExecutionFlowGraph()->getControlFlowNodeCounter();

  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc (serviceLocator());
  if (!messageSvc.isValid())
    error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;

  //BEN BEN
  m_eventSlots.assign(m_maxEventsInFlight+2,EventSlot(m_algosDependencies,algsNumber,
						    controlFlowNodeNumber,messageSvc));
  //m_eventSlots.assign(m_maxEventsInFlight,EventSlot(m_algosDependencies,algsNumber, 
//						    controlFlowNodeNumber,messageSvc));
  m_suspendPlease.assign(m_maxEventsInFlight+2,false); //BEN
  std::for_each(m_eventSlots.begin(),m_eventSlots.end(),
		[](EventSlot& slot){slot.complete=true;});

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events in flight: " << m_maxEventsInFlight << endmsg;
  info() << " o Number of algorithms in flight: " << m_maxAlgosInFlight << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  // Simulating execution flow by only analyzing the graph topology and logic
  if (m_simulateExecution) {
    auto vis = concurrency::RunSimulator(0);
    m_efManager.simulateExecutionFlow(vis);
  }

  return sc;

}
//---------------------------------------------------------------------------

/**
 * Here the scheduler is deactivated and the thread joined.
 **/
StatusCode ForwardSchedulerSvc::finalize(){

  StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
    warning () << "Base class could not be finalized" << endmsg;

  sc = deactivate();
  if (!sc.isSuccess())
    warning () << "Scheduler could not be deactivated" << endmsg;

  info() << "Joining Scheduler thread" << endmsg;
  m_thread.join();

  //m_efManager.getExecutionFlowGraph()->dumpExecutionPlan();

  return sc;

}
//---------------------------------------------------------------------------
/**
 * Activate the scheduler. From this moment on the queue of actions is
 * checked. The checking will stop when the m_isActive flag is false and the
 * queue is not empty. This will guarantee that all actions are executed and
 * a stall is not created.
 * The TBB pool must be initialised in the thread from where the tasks are
 * launched (http://threadingbuildingblocks.org/docs/doxygen/a00342.html)
 * The scheduler is initialised here since this method runs in a separate
 * thread and spawns the tasks (through the execution of the lambdas)
 **/
void ForwardSchedulerSvc::activate(){

  debug() << "ForwardSchedulerSvc::activate()" << endmsg;

   if ( m_threadPoolSvc->initPool(m_threadPoolSize).isFailure()) {
    error() << "problems initializing ThreadPoolSvc" << endmsg;
    m_isActive = FAILURE;
    return;
  }


  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;
  StatusCode sc(StatusCode::SUCCESS);

  m_isActive = ACTIVE;

  // Continue to wait if the scheduler is running or there is something to do
  info() << "Start checking the actionsQueue" << endmsg;
  while(m_isActive == ACTIVE or m_actionsQueue.size()!=0){
    m_actionsQueue.pop(thisAction);
    sc = thisAction();
    if (sc!=StatusCode::SUCCESS)
      verbose() << "Action did not succeed (which is not bad per se)." << endmsg;
    else
      verbose() << "Action succeeded." << endmsg;
  }

}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler until all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode ForwardSchedulerSvc::deactivate(){

  if (m_isActive == ACTIVE){
    // Drain the scheduler
    m_actionsQueue.push(std::bind(&ForwardSchedulerSvc::m_drain,
                                  this));
    // This would be the last action
    m_actionsQueue.push([this]() -> StatusCode {m_isActive=INACTIVE;return StatusCode::SUCCESS;});
  }

  return StatusCode::SUCCESS;
}

//===========================================================================

//===========================================================================
// Utils and shortcuts

inline const std::string& ForwardSchedulerSvc::index2algname (unsigned int index) {
  return m_algname_vect[index];
}

//---------------------------------------------------------------------------

inline unsigned int ForwardSchedulerSvc::algname2index(const std::string& algoname) {
  unsigned int index = m_algname_index_map[algoname];
  return index;
}

//===========================================================================
// EventSlot management
/**
 * Add event to the scheduler. There are two cases possible:
 *  1) No slot is free. A StatusCode::FAILURE is returned.
 *  2) At least one slot is free. An action which resets the slot and kicks
 * off its update is queued.
 */
StatusCode ForwardSchedulerSvc::pushNewEvent(EventContext* eventContext){

  if (m_first) {
    m_first = false;
  }

  if (!eventContext){
    fatal() << "Event context is nullptr" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_freeSlots.load() == 0) {
    if (msgLevel(MSG::DEBUG))
      debug() << "A free processing slot could not be found." << endmsg;
    return StatusCode::FAILURE;
  }

  //no problem as push new event is only called from one thread (event loop manager)
  m_freeSlots--;

  auto action = [this,eventContext] () -> StatusCode {
    // Event processing slot forced to be the same as the wb slot
    const unsigned int thisSlotNum = eventContext->slot();
    EventSlot& thisSlot = m_eventSlots[thisSlotNum];
    if (!thisSlot.complete) {
      fatal() << "The slot " << thisSlotNum 
	      << " is supposed to be a finished event but it's not" << endmsg;
      return StatusCode::FAILURE;
    }
    
    info() << "Executing event " << eventContext->evt() << " on slot " 
           << thisSlotNum << endmsg;
    thisSlot.reset(eventContext);

    //BEN maybe dead end
    /*if ( thisSlot.complete )
    {
      info() << "BEN allowing execution of incomplete slot " << thisSlotNum << endmsg;
    }
    else
    {
      info() << "BEN executing event " << eventContext->evt() << " on slot " << thisSlotNum << endmsg;
      thisSlot.reset(eventContext);
    }*/

    // XXX: CF tests
    if (m_CFNext) {
      auto vis = concurrency::Trigger(thisSlotNum);
      m_efManager.touchReadyAlgorithms(vis);
    }

    return this->updateStates(thisSlotNum);
  }; // end of lambda

  // Kick off the scheduling!
  if (msgLevel(MSG::VERBOSE)) {
    verbose() << "Pushing the action to update the scheduler for slot " <<  eventContext->slot() << endmsg;
    verbose() << "Free slots available " <<  m_freeSlots.load() << endmsg;
  }
  m_actionsQueue.push(action);

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode ForwardSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){
  StatusCode sc;
  for (auto context : eventContexts){
    sc = pushNewEvent(context);
    if (sc != StatusCode::SUCCESS) return sc;
  }
  return sc;
}

//---------------------------------------------------------------------------
unsigned int ForwardSchedulerSvc::freeSlots(){
  return std::max(m_freeSlots.load(),0);
}

//---------------------------------------------------------------------------
/**
 * Update the states for all slots until nothing is left to do.
*/
StatusCode ForwardSchedulerSvc::m_drain(){

  unsigned int slotNum=0;
  for (auto& thisSlot : m_eventSlots){
    if (not thisSlot.algsStates.allAlgsExecuted() and not thisSlot.complete){
      updateStates(slotNum);
    }
    slotNum++;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Get a finished event or block until one becomes available.
*/
StatusCode ForwardSchedulerSvc::popFinishedEvent(EventContext*& eventContext){
  // debug() << "popFinishedEvent: queue size: " << m_finishedEvents.size() << endmsg;
  if (m_freeSlots.load() == m_maxEventsInFlight or
      m_isActive == INACTIVE) {
    // debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    // 	    << " active: " << m_isActive << endmsg;
    return StatusCode::FAILURE;
  } else {
    // debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    // 	    << " active: " << m_isActive << endmsg;
    m_finishedEvents.pop(eventContext);
    m_freeSlots++;
    debug() << "Popped slot " << eventContext->slot() << " (event "
            << eventContext->evt() << ")" << endmsg;
    return StatusCode::SUCCESS;
  }
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just return a failure
*/
StatusCode ForwardSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext)
{
	info() << "BEN trying to pop finished event: " << eventContext << endmsg;
  if (m_finishedEvents.try_pop(eventContext)) {
    if (msgLevel(MSG::DEBUG))
      debug() << "Try Pop successful slot " << eventContext->slot()
              << "(event " << eventContext->evt() << ")" << endmsg;
    m_freeSlots++;
    info() << "BEN popped successfully" << endmsg;
    return StatusCode::SUCCESS;
  }
  info() << "BEN pop failed" << endmsg;
  return StatusCode::FAILURE;
}

//---------------------------------------------------------------------------
/**
 * It can be possible that an event fails. In this case this method is called.
 * It dumps the state of the scheduler, drains the actions (without executing
 * them) and events in the queues and returns a failure.
*/
StatusCode ForwardSchedulerSvc::eventFailed(EventContext* eventContext){

  // Set the number of slots available to an error code
  m_freeSlots.store(0);

  fatal() << "*** Event " << eventContext->evt() << " on slot "
          << eventContext->slot() << " failed! ***" << endmsg;

  dumpSchedulerState(-1);

  // Empty queue and deactivate the service
  action thisAction;
  while(m_actionsQueue.try_pop(thisAction)){};
  deactivate();

  // Push into the finished events queue the failed context
  EventContext* thisEvtContext;
  while(m_finishedEvents.try_pop(thisEvtContext)) { m_finishedEvents.push(thisEvtContext); };
  m_finishedEvents.push(eventContext);

  return StatusCode::FAILURE;

}

//===========================================================================

//===========================================================================
// States Management

/**
 * Update the state of the algorithms.
 * The oldest events are checked before the newest, in order to reduce the
 * event backlog.
 * To check if the event is finished the algorithm checks if:
 * * No algorithms have been signed off by the control flow
 * * No algorithms have been signed off by the data flow
 * * No algorithms have been scheduled
*/
StatusCode ForwardSchedulerSvc::updateStates(int si, const std::string& algo_name){

  m_updateNeeded=true;

  // Fill a map of initial state / action using closures.
  // done to update the states w/o several if/elses
  // Posterchild for constexpr with gcc4.7 onwards!
  /*const std::map<AlgsExecutionStates::State, std::function<StatusCode(unsigned int iAlgo, int si)>>
   statesTransitions = {
  {AlgsExecutionStates::CONTROLREADY, std::bind(&ForwardSchedulerSvc::promoteToDataReady,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2)},
  {AlgsExecutionStates::DATAREADY, std::bind(&ForwardSchedulerSvc::promoteToScheduled,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2)}
  };*/

  StatusCode global_sc(StatusCode::FAILURE,true);

   // Sort from the oldest to the newest event
   // Prepare a vector of pointers to the slots to avoid copies
   std::vector<EventSlot*> eventSlotsPtrs;

   // Consider all slots if si <0 or just one otherwise
   if (si<0) {
     const int eventsSlotsSize(m_eventSlots.size());
     eventSlotsPtrs.reserve(eventsSlotsSize);
     for (auto slotIt=m_eventSlots.begin();slotIt!=m_eventSlots.end();slotIt++) {
       if (!slotIt->complete)
         eventSlotsPtrs.push_back(&(*slotIt));
     }
     std::sort(eventSlotsPtrs.begin(),
               eventSlotsPtrs.end(),
               [](EventSlot* a, EventSlot* b) {return a->eventContext->evt() < b->eventContext->evt();});
   } else {
     eventSlotsPtrs.push_back(&m_eventSlots[si]);
   }

  for (EventSlot* thisSlotPtr : eventSlotsPtrs) {
    int iSlot = thisSlotPtr->eventContext->slot();

    // Cache the states of the algos to improve readability and performance
    auto& thisSlot = m_eventSlots[iSlot];
    AlgsExecutionStates& thisAlgsStates = thisSlot.algsStates;

    // Take care of the control ready update
    // XXX: CF tests
    if (!m_CFNext) {
      m_efManager.updateEventState(thisAlgsStates,thisSlot.controlFlowState);
	info() << "BEN updated event state in slot " << iSlot << endmsg;
    } else {
      if (!algo_name.empty())
        m_efManager.updateDecision(algo_name,iSlot,thisAlgsStates,thisSlot.controlFlowState);
	info() << "BEN updated decision in slot " << iSlot << endmsg;
    }


    //DF note: all this this is a loop over all algs and applies CR->DR and DR->SCHD transistions
    /*for (unsigned int iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){
        const AlgsExecutionStates::State& algState = thisAlgsStates[iAlgo];
        if (algState==AlgsExecutionStates::ERROR)
            error() << " Algo " << index2algname(iAlgo) << " is in ERROR state." << endmsg;
        // Loop on state transitions from the one suited to algo state up to the one for SCHEDULED.
        partial_sc=StatusCode::SUCCESS;
        for (auto state_transition = statesTransitions.find(algState);
                state_transition!=statesTransitions.end() && partial_sc.isSuccess();
                state_transition++){
            partial_sc = state_transition->second(iAlgo,iSlot);
            if (partial_sc.isFailure()){
               verbose() << "Could not apply transition from "
                        << AlgsExecutionStates::stateNames[thisAlgsStates[iAlgo]]
                                                           << " for algorithm " << index2algname(iAlgo)
                                                           << " on processing slot " << iSlot << endmsg;
            }
            else{global_sc=partial_sc;}
        } // end loop on transitions
    }*/ // end loop on algos

    StatusCode partial_sc(StatusCode::FAILURE,true);
    //first update CONTROLREADY to DATAREADY
    if ( m_suspendPlease[ iSlot ] ) //BEN
    {
	    info() << "BEN don't promote CR->DR - slot " << iSlot << " suspended" << endmsg;
    }
    else
    {//BEN
    if (!m_CFNext) {
      for(auto it = thisAlgsStates.begin(AlgsExecutionStates::State::CONTROLREADY);
          it != thisAlgsStates.end(AlgsExecutionStates::State::CONTROLREADY); ++it) {

        uint algIndex = *it;
        partial_sc = promoteToDataReady(algIndex, iSlot);
	info() << "BEN promoting " << index2algname(algIndex) << " to data ready in slot " << iSlot << endmsg;
        if (partial_sc.isFailure())
          if (msgLevel(MSG::DEBUG))
            verbose() << "Could not apply transition from "
                    << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::CONTROLREADY]
                    << " for algorithm " << index2algname(algIndex) << " on processing slot " << iSlot << endmsg;
      }
    }
    }//BEN

    //now update DATAREADY to SCHEDULED
    //if ( !m_suspendPlease[ iSlot ] ) //BEN
    //{ //BEN
    if (!m_optimizationMode.empty()) {
      auto comp_nodes = [this] (const uint& i,const uint& j) {
          return (m_efManager.getExecutionFlowGraph()->getAlgorithmNode(index2algname(i))->getRank() <
          m_efManager.getExecutionFlowGraph()->getAlgorithmNode(index2algname(j))->getRank());
      };
      std::priority_queue<uint,std::vector<uint>,std::function<bool(const uint&,const uint&)>> buffer(comp_nodes,std::vector<uint>());
      for(auto it = thisAlgsStates.begin(AlgsExecutionStates::State::DATAREADY);
          it != thisAlgsStates.end(AlgsExecutionStates::State::DATAREADY); ++it)
        buffer.push(*it);
      /*std::stringstream s;
      auto buffer2 = buffer;
      while (!buffer2.empty()) {
        s << m_efManager.getExecutionFlowGraph()->getAlgorithmNode(index2algname(buffer2.top()))->getRank() << ", ";
        buffer2.pop();
      }
      info() << "DRBuffer is: [ " << s.str() << " ]  <--" << algo_name << " executed" << endmsg;*/

      while (!buffer.empty()) {
        partial_sc = promoteToScheduled(buffer.top(), iSlot);
        if (partial_sc.isFailure())
          if (msgLevel(MSG::DEBUG))
            verbose() << "Could not apply transition from "
                    << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY]
                    << " for algorithm " << index2algname(buffer.top()) << " on processing slot " << iSlot << endmsg;
        buffer.pop();
      }

    } else {
      for(auto it = thisAlgsStates.begin(AlgsExecutionStates::State::DATAREADY);
          it != thisAlgsStates.end(AlgsExecutionStates::State::DATAREADY); ++it) {
        uint algIndex = *it;
        partial_sc = promoteToScheduled(algIndex, iSlot);
	info() << "BEN promoting " << index2algname(algIndex) << " to scheduled in slot " << iSlot << endmsg;
        if (partial_sc.isFailure())
          if (msgLevel(MSG::DEBUG))
            verbose() << "Could not apply transition from "
                    << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY]
                    << " for algorithm " << index2algname(algIndex) << " on processing slot " << iSlot << endmsg;

      }
    }
    //}//BEN

    if (m_dumpIntraEventDynamics) {
      std::stringstream s;
      s << algo_name << ", " << thisAlgsStates.sizeOfSubset(State::CONTROLREADY)
                     << ", " << thisAlgsStates.sizeOfSubset(State::DATAREADY)
                     << ", " << thisAlgsStates.sizeOfSubset(State::SCHEDULED) << "\n";
      auto threads = (m_threadPoolSize != -1) ? std::to_string(m_threadPoolSize)
                                              : std::to_string(tbb::task_scheduler_init::default_num_threads());
      std::ofstream myfile;
      myfile.open("IntraEventConcurrencyDynamics_" + threads + "T.csv", std::ios::app);
      myfile << s.str();
      myfile.close();
    }


    // Not complete because this would mean that the slot is already free!
    if (!thisSlot.complete &&
        m_efManager.rootDecisionResolved(thisSlot.controlFlowState) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::CONTROLREADY) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::SCHEDULED)) {

      thisSlot.complete=true;
      // if the event did not fail, add it to the finished events
      // otherwise it is taken care of in the error handling already
      if (!thisSlot.eventContext->evtFail()) {
        m_finishedEvents.push(thisSlot.eventContext);
        if (msgLevel(MSG::DEBUG))
          debug() << "Event " << thisSlot.eventContext->evt() << " finished (slot "
                  << thisSlot.eventContext->slot() << ")." << endmsg;
      }
      // now let's return the fully evaluated result of the control flow
      if (msgLevel(MSG::DEBUG)) {
        std::stringstream ss;
        m_efManager.printEventState(ss, thisSlot.algsStates, thisSlot.controlFlowState,0);
        debug() << ss.str() << endmsg;
      }

      thisSlot.eventContext= nullptr;
    } else if ( !isSuspended(iSlot).isSuccess() ) { //BEN
	    //info() << "BEN suspend detected (no resume)" << endmsg;
	    info() << "BEN suspend detected" << endmsg;
	    EventContext * inputContext = thisSlot.eventContext;
	    AlgsExecutionStates inputStates = thisSlot.algsStates;
	    DataFlowManager inputDataFlow = thisSlot.dataFlowMgr;
	    auto action = [this,inputContext,inputStates,inputDataFlow] () -> StatusCode {
		    info() << "BEN running suspended event: " << inputContext << endmsg;
		    this->m_suspendPlease[ inputContext->slot() ] = false;
		    this->m_eventSlots[ inputContext->slot() ].reset( inputContext );
		    this->m_eventSlots[ inputContext->slot() ].algsStates = inputStates;
		    this->m_eventSlots[ inputContext->slot() ].dataFlowMgr = inputDataFlow;
		    StatusCode sc = this->updateStates( inputContext->slot() );
		    //StatusCode sc = this->updateStates( -1 );
		    return StatusCode::SUCCESS;
	    };
	    m_actionsQueue.push( action ); //END BEN
    } else {
      StatusCode eventStalledSC = isStalled(iSlot);
      if (! eventStalledSC.isSuccess())
        //eventFailed(thisSlot.eventContext);
        m_finishedEvents.push(thisSlot.eventContext); //BEN
    }
  } // end loop on slots

  verbose() << "States Updated." << endmsg;

  return global_sc;
}

//---------------------------------------------------------------------------

/**
 * Check if we are in present of a stall condition for a particular slot.
 * This is the case when no actions are present in the actionsQueue,
 * no algorithm is in flight and no algorithm has all of its dependencies
 * satisfied.
*/
StatusCode ForwardSchedulerSvc::isStalled(int iSlot) {
  // Get the slot
  EventSlot& thisSlot = m_eventSlots[iSlot];

  if (m_actionsQueue.empty() &&
      m_algosInFlight == 0 &&
      (!thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY))) {

    /*info() << "About to declare a stall" << endmsg;
    fatal() << "*** Stall detected! ***\n" << endmsg;
    dumpSchedulerState(iSlot);*/
	  info() << "BEN stall detected - no problemo" << endmsg;
    //throw GaudiException ("Stall detected",name(),StatusCode::FAILURE);

    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//BEN
StatusCode ForwardSchedulerSvc::isSuspended(int iSlot)
{
	if ( m_suspendPlease[iSlot] )
	{
		EventSlot& thisSlot = m_eventSlots[iSlot];

		if (m_actionsQueue.empty() &&
				m_algosInFlight == 0 &&
				(!thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY)))
		{
			return StatusCode::FAILURE;
		}
	}
	return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

/**
 * Used for debugging purposes, the state of the scheduler is dumped on screen
 * in order to be inspected.
 * The dependencies of each algo are printed and the missing ones specified.
**/
void ForwardSchedulerSvc::dumpSchedulerState(int iSlot) {

  // To have just one big message
  std::ostringstream outputMessageStream;

  outputMessageStream
    << "============================== Execution Task State ============================="
    << std::endl;
  dumpState(outputMessageStream);

  outputMessageStream << std::endl
    << "============================== Scheduler State  ================================="
    << std::endl;

  int slotCount = -1;
  for (auto thisSlot : m_eventSlots){
    slotCount++;
    if ( thisSlot.complete )
      continue;

    outputMessageStream << "-----------  slot: " << thisSlot.eventContext->slot() 
			<< "  event: " << thisSlot.eventContext->evt() 
			<< " -----------"<< std::endl;

    if ( 0 > iSlot or iSlot == slotCount) {
      outputMessageStream << "Algorithms states:" << std::endl;

      const DataObjIDColl& wbSlotContent ( thisSlot.dataFlowMgr.content() );
      for (unsigned int algoIdx=0; algoIdx < thisSlot.algsStates.size(); ++algoIdx ) {
        outputMessageStream << " o " << index2algname(algoIdx)
                            << " [" << AlgsExecutionStates::stateNames[thisSlot.algsStates[algoIdx]]
                            << "]  Data deps: ";
        DataObjIDColl deps (thisSlot.dataFlowMgr.dataDependencies(algoIdx));
        const int depsSize=deps.size();
        if (depsSize==0)
          outputMessageStream << " none";

	DataObjIDColl missing;
	for (auto d: deps) {
	  outputMessageStream << d << " ";
	  if ( wbSlotContent.find(d) == wbSlotContent.end() ) {
	    //	    outputMessageStream << "[missing] ";
	    missing.insert(d);
	  }
	}

        // for (int i=0;i<depsSize;++i)
        //   outputMessageStream << deps[i] << (i==(depsSize-1) ? "" :", ");

        // Now list what dependencies were available
        // With std::algorithms, move the ones which are missing at the beginning of the vector
        // DataObjIDColl::iterator missinngDepsEndIt =
        //                 std::remove_if(deps.begin(), // from the beginning of the deps
        //                                deps.end(),   // to their end
        //                                [&wbSlotContent] (DataObjID dep) { // remove if this lambda returns true
        //   return std::count(wbSlotContent.begin(),wbSlotContent.end(),dep)!=0; //look for dep in wb content
        // });

        // if (deps.begin() != missinngDepsEndIt) {
        //   outputMessageStream << ". The following are missing: ";
        //   for (DataObjIDColl::iterator missingDep=deps.begin();missingDep!=missinngDepsEndIt;++missingDep)
        //     outputMessageStream << *missingDep << (missingDep==(missinngDepsEndIt-1)?"":", ");
        // }
	if (! missing.empty()) {
          outputMessageStream << ". The following are missing: ";
	  for (auto d: missing) {
	    outputMessageStream << d << " ";
	  }
        }

        outputMessageStream << std::endl;
      }

      // Snapshot of the WhiteBoard
      outputMessageStream << "\nWhiteboard contents: "<< std::endl;
      for (auto& product : wbSlotContent )
        outputMessageStream << " o " << product << std::endl;

      // Snapshot of the ControlFlow
      outputMessageStream << "\nControl Flow:" << std::endl;
      std::stringstream cFlowStateStringStream;
      m_efManager.printEventState(cFlowStateStringStream, thisSlot.algsStates, thisSlot.controlFlowState,0);

      outputMessageStream << cFlowStateStringStream.str() << std::endl;      
    }
  }

  outputMessageStream 
    << "=================================== END ======================================"
    << std::endl;

  info() << "Dumping Scheduler State " << std::endl 
	 << outputMessageStream.str() << endmsg;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToControlReady(unsigned int iAlgo, int si) {

  // Do the control flow
  StatusCode sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::CONTROLREADY);
  if (sc.isSuccess())
    if (msgLevel(MSG::VERBOSE))
      verbose() << "Promoting " << index2algname(iAlgo) << " to CONTROLREADY on slot " 
	      << si << endmsg;

  return sc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToDataReady(unsigned int iAlgo, int si) {

  StatusCode sc;
  if (!m_DFNext) {
    sc = m_eventSlots[si].dataFlowMgr.canAlgorithmRun(iAlgo);
  } else {
    sc = m_efManager.algoDataDependenciesSatisfied(index2algname(iAlgo),si);
  }

  StatusCode updateSc(StatusCode::FAILURE);
  if (sc == StatusCode::SUCCESS)
    updateSc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::DATAREADY);

  if (updateSc.isSuccess())
    if (msgLevel(MSG::VERBOSE))
      verbose() << "Promoting " << index2algname(iAlgo) << " to DATAREADY on slot " 
	      << si<< endmsg;

  return updateSc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToScheduled(unsigned int iAlgo, int si) {

  if (m_algosInFlight == m_maxAlgosInFlight)
    return StatusCode::FAILURE;

  const std::string& algName(index2algname(iAlgo));

  IAlgorithm* ialgoPtr=nullptr;
  info() << "BEN trying to acquire algroithm " << algName << " for slot " << si << endmsg;
  StatusCode sc ( m_algResourcePool->acquireAlgorithm(algName,ialgoPtr) );

  if (sc.isSuccess()) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?
    EventContext* eventContext ( m_eventSlots[si].eventContext );
    if (!eventContext)
      fatal() << "Event context for algorithm " << algName << " is a nullptr (slot " << si<< ")" << endmsg;

    algoPtr->setContext(m_eventSlots[si].eventContext);
    ++m_algosInFlight;
    // Avoid to use tbb if the pool size is 1 and run in this thread
    if (-100 != m_threadPoolSize) {
	    info() << "BEN enqueuing " << index2algname(iAlgo) << " in slot " << si << endmsg;
      tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, serviceLocator(), this);
      tbb::task::enqueue( *t);
    } else {
	    info() << "BEN making a task for " << index2algname(iAlgo) << " in slot " << si << endmsg;
      AlgoExecutionTask theTask(ialgoPtr, iAlgo, serviceLocator(), this);
      theTask.execute();
    }

    if (msgLevel(MSG::DEBUG))
      debug() << "Algorithm " << algName << " was submitted on event " 
	      << eventContext->evt() << " in slot " << si
              << ". Algorithms scheduled are " << m_algosInFlight << endmsg;

    StatusCode updateSc ( m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::SCHEDULED) );

    if (msgLevel(MSG::VERBOSE)) dumpSchedulerState(-1);

    if (updateSc.isSuccess())
      if (msgLevel(MSG::VERBOSE))
	verbose() << "Promoting " << index2algname(iAlgo) << " to SCHEDULED on slot " 
		<< si << endmsg;
    return updateSc;
  } else {
    if (msgLevel(MSG::DEBUG))
      debug() << "Could not acquire instance for algorithm " << index2algname(iAlgo) << " on slot " << si << endmsg;
    return sc;
  }

}

//---------------------------------------------------------------------------
/**
 * The call to this method is triggered only from within the AlgoExecutionTask.
*/
StatusCode ForwardSchedulerSvc::promoteToExecuted(unsigned int iAlgo, int si, IAlgorithm* algo) {

  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>(algo); // DP: expose context getter in IAlgo?
  if (!castedAlgo)
    fatal() << "The casting did not succeed!" << endmsg;
  EventContext* eventContext = castedAlgo->getContext();

  // Check if the execution failed
  if (eventContext->evtFail())
    eventFailed(eventContext);

  StatusCode sc = m_algResourcePool->releaseAlgorithm(algo->name(),algo);

  if (!sc.isSuccess()) {
    error() << "[Event " << eventContext->evt() << ", Slot " << eventContext->slot()  
	    << "] "   << "Instance of algorithm " << algo->name() 
	    << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
    }

  m_algosInFlight--;

  EventSlot& thisSlot = m_eventSlots[si];
  // XXX: CF tests
  if (!m_DFNext) {
    // Update the catalog: some new products may be there
    m_whiteboard->selectStore(eventContext->slot()).ignore();

    // update prods in the dataflow
    // DP: Handles could be used. Just update what the algo wrote
    DataObjIDColl new_products;
    m_whiteboard->getNewDataObjects(new_products).ignore();
    for (const auto& new_product : new_products)
      if (msgLevel(MSG::DEBUG))
        debug() << "Found in WB [" << si << "]: " << new_product << endmsg;
    thisSlot.dataFlowMgr.updateDataObjectsCatalog(new_products);
  }

  if (msgLevel(MSG::DEBUG))
    debug() << "Algorithm " << algo->name() << " executed in slot " << si 
	    << ". Algorithms scheduled are " << m_algosInFlight << endmsg;

  // Limit number of updates
  if (m_CFNext) m_updateNeeded = true; // XXX: CF tests: with the new CF traversal the if clause below has to be removed
  if (m_updateNeeded) {
    // Schedule an update of the status of the algorithms
    auto updateAction = std::bind(&ForwardSchedulerSvc::updateStates, this, -1, algo->name());
    m_actionsQueue.push(updateAction);
    m_updateNeeded = false;
  }

  if (msgLevel(MSG::DEBUG))
    debug() << "Trying to handle execution result of " << index2algname(iAlgo) 
	    << " on slot " << si << endmsg;
  State state;
  if (algo->filterPassed()) {
    state = State::EVTACCEPTED;
  } else {
    state = State::EVTREJECTED;
  }

  sc = thisSlot.algsStates.updateState(iAlgo,state);

  if (sc.isSuccess())
    if (msgLevel(MSG::VERBOSE))
      verbose() << "Promoting " << index2algname(iAlgo) << " on slot " << si << " to "
              << AlgsExecutionStates::stateNames[state] << endmsg;

  return sc;
}

//===========================================================================
void 
ForwardSchedulerSvc::addAlg(Algorithm* a, EventContext* e, pthread_t t) {

  std::lock_guard<std::mutex> lock(m_ssMut);
  m_sState.push_back(SchedulerState(a,e,t));

}

//===========================================================================
bool
ForwardSchedulerSvc::delAlg(Algorithm* a) {

  std::lock_guard<std::mutex> lock(m_ssMut);
  
  for (std::list<SchedulerState>::iterator itr = m_sState.begin();
       itr != m_sState.end(); ++itr) {
    if (*itr == a) {
      m_sState.erase(itr);
      return true;
    }
  }

  error() << "could not find Alg " << a->name() << " in Scheduler!" << endmsg;
  return false;
}

//===========================================================================
void
ForwardSchedulerSvc::dumpState(std::ostringstream& ost) {
  
  std::lock_guard<std::mutex> lock(m_ssMut);

  for (auto it : m_sState) {
    ost << "  " << it << std::endl;
  }

}

//===========================================================================
void
ForwardSchedulerSvc::dumpState() {
  
  std::lock_guard<std::mutex> lock(m_ssMut);

  std::ostringstream ost;
  ost << "dumping Executing Threads: [" << m_sState.size() << "]" << std::endl;
  dumpState(ost);

  info() << ost.str() << endmsg;

}

//===========================================================================
void
ForwardSchedulerSvc::sdumpState() {
  
  std::lock_guard<std::mutex> lock(m_ssMut);
  std::ostringstream ost;
  ost << "dumping Executing Threads: [" << m_sState.size() << "]" << std::endl;
  for (auto it : m_sState) {
    ost << "  " << it << std::endl;
  }

  std::cout << ost.str();

}

//BEN
StatusCode ForwardSchedulerSvc::runAlgorithm( std::string AlgName, EventContext * eventContext )
{
	std::vector< std::string > algorithmNameSequence;
	while ( AlgName.find( '#' ) != std::string::npos )
	{
		algorithmNameSequence.push_back( AlgName.substr( 0, AlgName.find( '#' ) ) );
		AlgName = AlgName.substr( AlgName.find( '#' ) + 1 );
		info() << algorithmNameSequence.back() << endmsg;
	}
	algorithmNameSequence.push_back( AlgName );
	info() << algorithmNameSequence.back() << endmsg;

	std::vector< unsigned int > algorithmIndexSequence;
	std::vector< IAlgorithm* > algorithmPointerSequence;
	for ( std::string inputAlgorithm : algorithmNameSequence )
	{
		unsigned int iAlgo = this->algname2index( inputAlgorithm );
		IAlgorithm * ialgoPtr = nullptr;
		StatusCode sc = m_algResourcePool->acquireAlgorithm( inputAlgorithm,ialgoPtr );
		if ( sc.isSuccess() )
		{
			Algorithm * algoPtr = dynamic_cast< Algorithm* >( ialgoPtr );
			if ( !eventContext )
			{
				fatal() << "Event context for algorithm " << inputAlgorithm << " is a nullptr (slot 0)" << endmsg;
			}

			//algoPtr->setContext( eventContext ); //BEN
			//++m_algosInFlight;
			algorithmIndexSequence.push_back( iAlgo );
			algorithmPointerSequence.push_back( ialgoPtr );

			info() << "BEN directly queued " << inputAlgorithm << endmsg;
			sc = m_algResourcePool->releaseAlgorithm( inputAlgorithm,ialgoPtr );
			info() << "BEN attempted to release algorithm: " << sc << endmsg;
		}
		else
		{
			info() << "BEN could not acquire " << inputAlgorithm << endmsg;
			break;
		}

		//sc = m_algResourcePool->releaseAlgorithm( AlgName,ialgoPtr );
		//info() << "BEN attempted to release algorithm: " << sc << endmsg;
	}

	tbb::task * t = new( tbb::task::allocate_root() )GraphExecutionTask( algorithmPointerSequence, algorithmIndexSequence, eventContext, serviceLocator(), this );
	tbb::task::enqueue( *t );
	return StatusCode::SUCCESS;
}

StatusCode ForwardSchedulerSvc::pushSubEventsAndSuspend( std::vector< EventContext* > subEventContexts )
{
	if ( !subEventContexts.size() ) return StatusCode::FAILURE;

	unsigned int const slotIndex = subEventContexts[0]->slot();
	auto& thisSlot = m_eventSlots[ slotIndex ];	
	info() << "BEN BEN slotIndex: " << slotIndex << " vs " << m_suspendPlease.size() << endmsg;
	m_suspendPlease[ slotIndex ] = true;

	int deltaSlot = 1;
	for ( EventContext * inputContext : subEventContexts )
	{
		//info() << "BEN received sub event (but ignoring): " << inputContext << endmsg;
		info() << "BEN received sub event: " << inputContext << endmsg;
		//inputContext->setSlot( slotIndex );
		inputContext->setSlot( slotIndex + deltaSlot );
		deltaSlot++;
		AlgsExecutionStates inputStates = thisSlot.algsStates;
		DataFlowManager inputDataFlow = thisSlot.dataFlowMgr;
		//m_freeSlots--;
		auto action = [this,inputContext,inputStates,inputDataFlow] () -> StatusCode {
			info() << "BEN running sub event: " << inputContext << endmsg;
			info() << "BEN with states: " << inputStates.size() << endmsg;
			info() << "BEN and data flow: " << inputDataFlow.content().size() << endmsg;
			this->m_suspendPlease[ inputContext->slot() ] = false;
			this->m_eventSlots[ inputContext->slot() ].reset( inputContext );
			this->m_eventSlots[ inputContext->slot() ].algsStates = inputStates;
			this->m_eventSlots[ inputContext->slot() ].dataFlowMgr = inputDataFlow;
			StatusCode sc = this->updateStates( inputContext->slot() );
			//StatusCode sc = this->updateStates( -1 );
			return StatusCode::SUCCESS;
		};
		m_actionsQueue.push( action );
	}

	//Push the resume
	/*EventContext inputContext = *( thisSlot.eventContext );
	AlgsExecutionStates inputStates = thisSlot.algsStates;
	DataFlowManager inputDataFlow = thisSlot.dataFlowMgr;
	auto action = [this,inputContext,inputStates,inputDataFlow] () -> StatusCode {
		info() << "BEN running suspended event (from immediate queue): " << inputContext << endmsg;
		this->m_suspendPlease[ inputContext.slot() ] = false;
		//this->m_eventSlots[ inputContext.slot() ].reset( &inputContext );
		this->m_eventSlots[ inputContext.slot() ].algsStates = inputStates;
		this->m_eventSlots[ inputContext.slot() ].dataFlowMgr = inputDataFlow;
		StatusCode sc = this->updateStates( inputContext.slot() );
		//StatusCode sc = this->updateStates( -1 );
		return StatusCode::SUCCESS;
	};
	m_actionsQueue.push( action );*/

	return StatusCode::SUCCESS;
}
