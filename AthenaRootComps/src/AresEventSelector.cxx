///////////////////////// -*- C++ -*- /////////////////////////////
// AresEventSelector.cxx 
// Implementation file for class AresEventSelector
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// STL includes
#include <sstream>

// ROOT includes
#include "TTree.h"

// Framework includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ITHistSvc.h"

// StoreGate includes
#include "StoreGate/StoreGate.h" 

// EventInfo includes
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventType.h"
#include "EventInfo/EventID.h"

// Package includes
#include "AresEventSelector.h"
#include "TTreeEventContext.h"
//#include "TTreeEventSelectorHelperSvc.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

AresEventSelector::AresEventSelector( const std::string& name,
					    ISvcLocator* svcLoc ) :
  AthService( name,    svcLoc ),
  m_storeGate( "StoreGateSvc", name ),
  m_tupleSvc ( "THistSvc",     name ),
  m_nbrEvts     ( 0 ),
  m_totalNbrEvts( 0 ),
  m_tuple       ( 0 )
{
  declareProperty( "InputCollections", 
		   m_inputCollectionsName,
		   "List of input (ROOT) file names" );
  m_inputCollectionsName.declareUpdateHandler
    ( &AresEventSelector::setupInputCollection, this );

  declareProperty( "TupleName",
		   m_tupleName = "CollectionTree",
		   "Name of the TTree to load/read from input file(s)" );

  declareProperty( "SkipEvents",           
		   m_skipEvts = 0,
		   "Number of events to skip at the beginning" );

  declareProperty( "ActiveBranches",
		   m_activeBranchNames,
		   "List of branch names to activate" );
}

// Destructor
///////////////
AresEventSelector::~AresEventSelector()
{}

StatusCode AresEventSelector::initialize()
{
  ATH_MSG_INFO ("Enter AresEventSelector initialization...");

  // retrieve event store
  if ( !m_storeGate.retrieve().isSuccess() ) {
    ATH_MSG_ERROR
      ("Could not retrieve [" << m_storeGate.typeAndName() << "] !!");
    return StatusCode::FAILURE;
  }

  // retrieve ITHistSvc
  if ( !m_tupleSvc.retrieve().isSuccess() ) {
    ATH_MSG_ERROR
      ("Could not retrieve [" << m_tupleSvc.typeAndName() << "] !!");
    return StatusCode::FAILURE;
  }

  if ( m_tupleName.value().empty() ) {
    ATH_MSG_ERROR
      ("You have to give a TTree name to read from the ROOT files !");
    return StatusCode::FAILURE;
  }

  setupInputCollection( m_inputCollectionsName );
  const std::size_t nbrInputFiles = m_inputCollectionsName.value().size();
  if ( nbrInputFiles < 1 ) {
    ATH_MSG_ERROR
      ("You need to give at least 1 input file !!" << endreq
       << "(Got [" << nbrInputFiles << "] file instead !)");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO
      ("Selector configured :" << endreq
       << "    to read [" << nbrInputFiles << "] file(s)..." << endreq
       << " from tuple [" << m_tupleName.value() << "_trans]");
  }

  // load tree/chain
  m_tuple = 0;

  {
    std::ostringstream tupleName;
    tupleName << "/temp/TTreeStream/" << m_tupleName.value() << "_trans";
  
    const ITHistSvc* svc = &*m_tupleSvc;
    if ( !svc->getTree(tupleName.str(), m_tuple).isSuccess() ||
	 0 == m_tuple ) {
      ATH_MSG_ERROR
	("Could not re-access tuple at [" << tupleName.str() << "] !!");
      return StatusCode::FAILURE;
    }
  }

  // get total number of events:
  m_totalNbrEvts = m_tuple->GetEntries();
  ATH_MSG_INFO ("Total events: " << m_totalNbrEvts);

  if ( m_skipEvts >= m_totalNbrEvts ) {
    ATH_MSG_ERROR 
      ("Number of events to skip (" << m_skipEvts << ") is greater than "
       << "the total number of events in the input collections ("
       << m_totalNbrEvts << ") !"
       << endreq
       << "Please correct your jobOptions file");
    return StatusCode::FAILURE;
  }

  // skip events we are asked to skip
  m_nbrEvts += m_skipEvts;

  return StatusCode::SUCCESS;
}

StatusCode AresEventSelector::finalize()
{
  ATH_MSG_INFO ("Finalize...");

  // FIXME: this should be tweaked/updated if/when a selection function
  //        or filtering predicate is applied (one day?)
  ATH_MSG_INFO 
    ("Total events read: " << (m_nbrEvts - m_skipEvts));

  return StatusCode::SUCCESS;
}

// Query the interfaces.
//   Input: riid, Requested interface ID
//          ppvInterface, Pointer to requested interface
//   Return: StatusCode indicating SUCCESS or FAILURE.
// N.B. Don't forget to release the interface after use!!!
StatusCode 
AresEventSelector::queryInterface( const InterfaceID& riid, 
				      void** ppvInterface )
{
  if ( IEvtSelector::interfaceID().versionMatch(riid) ) {
    *ppvInterface = dynamic_cast<IEvtSelector*>(this);
  } else {
    // Interface is not directly available : try out a base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

StatusCode AresEventSelector::next( IEvtSelector::Context& ctx ) const
{
  return this->next(ctx, 0);
}

StatusCode AresEventSelector::next( Context& ctx, int jump ) const
{
  ATH_MSG_DEBUG ("next(" << jump << ") : iEvt " << m_nbrEvts);

  // get evt context
  TTreeEventContext* ct = dynamic_cast<TTreeEventContext*>(&ctx);
  if ( 0 == ct ) {
    ATH_MSG_ERROR ("Could not dyn-cast to TTreeEventContext !!");
    return StatusCode::FAILURE;
  }

  // jump to right position
  if ( ( (m_nbrEvts + jump) >= 0 ) &&
       ( (m_nbrEvts + jump) <  m_totalNbrEvts ) ) {
    // adjust pointer
    m_nbrEvts += jump;
    
    if ( m_tuple->GetEntry( m_nbrEvts ) < 0 ) {
      ATH_MSG_ERROR
	("Problem retrieving data from tree for entry [" <<m_nbrEvts<< "] !!");
      return StatusCode::FAILURE;
    }
    ++m_nbrEvts;

    // event info
    EventType* evtType = new EventType;
    const std::size_t runNbr = 0;
    EventInfo* evtInfo = new EventInfo( new EventID( runNbr, m_nbrEvts-1, 0 ),
					evtType );
    if ( !m_storeGate->record( evtInfo, "AresEventInfo" ).isSuccess() ) {
      ATH_MSG_ERROR ("Could not record AresEventInfo !");
      delete evtInfo; evtInfo = 0;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  // still here ? So this is it. EOF.
  return StatusCode::FAILURE;
}

StatusCode
AresEventSelector::previous( IEvtSelector::Context& ctx ) const 
{
  return next( ctx, -1 );
}

StatusCode 
AresEventSelector::previous( Context& ctx, int jump ) const 
{
  return next( ctx, -jump );
}

StatusCode
AresEventSelector::last( Context& /*ctxt*/ ) const
{
  ATH_MSG_ERROR ("............. Last Event Not Implemented .............");
  return StatusCode::FAILURE;
}


StatusCode 
AresEventSelector::rewind( Context& /*ctxt*/ ) const 
{
  ATH_MSG_ERROR 
    ("............. ::rewind(Context) Not Implemented .............");
  return StatusCode::FAILURE;
}

StatusCode
AresEventSelector::createContext( Context*& refCtx ) const
{
  refCtx = new TTreeEventContext(this);
  return StatusCode::SUCCESS;
}

StatusCode
AresEventSelector::createAddress( const Context& /*refCtx*/, 
                                  IOpaqueAddress*& /*addr*/ ) const 
{
  return StatusCode::SUCCESS;
}

StatusCode
AresEventSelector::releaseContext( Context*& refCtxt ) const
{
  TTreeEventContext *ctx  = dynamic_cast<TTreeEventContext*>(refCtxt);
  if ( ctx )   {
    delete ctx; ctx = 0;
    return StatusCode::SUCCESS;
  }
    return StatusCode::FAILURE;
}

StatusCode
AresEventSelector::resetCriteria( const std::string&, Context& ) const 
{
  ATH_MSG_ERROR ("............. resetCriteria Not Implemented .............");
  return StatusCode::FAILURE;
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/**
 * @brief Seek to a given event number.
 * @param evtnum  The event number to which to seek.
 */
StatusCode
AresEventSelector::seek (int evtnum)
{
  
  // jump to right position
  if ( ( evtnum >= 0 ) &&
       ( evtnum <  m_totalNbrEvts ) ) {
    // adjust pointer
    m_nbrEvts = evtnum;
    
    if ( m_tuple->GetEntry( m_nbrEvts ) < 0 ) {
      ATH_MSG_ERROR 
	("Problem retrieving data from tree for entry [" <<m_nbrEvts<< "] !!");
      return StatusCode::FAILURE;
    }
    ++m_nbrEvts;
  } else {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

int
AresEventSelector::curEvent() const
{
  return m_nbrEvts;
}

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

/// callback to synchronize the list of input files
void 
AresEventSelector::setupInputCollection( Property& /*inputCollectionsName*/ )
{
  // nothing ?
  return;
}

