///////////////////////// -*- C++ -*- /////////////////////////////
// DFlowAlg1_manualViews.cxx 
// Implementation file for class DFlowAlg1_manualViews
// Modifed by bwynne to add simple tests for views
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthExStoreGateExample includes
#include "DFlowAlg1_manualViews.h"

// STL includes

// FrameWork includes
#include "GaudiKernel/Property.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

#include "CxxUtils/make_unique.h"

#include "AthViews/View.h"

namespace AthViews {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
DFlowAlg1_manualViews::DFlowAlg1_manualViews( const std::string& name, 
                      ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_r_evtInfo("EventInfo"),
  m_w_int("dflow_int")
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty("IntFlow", 
                  m_w_int,
                  "Data flow of int");

  declareProperty("EvtInfo",
                  m_r_evtInfo,
                  "event info handle");

  declareProperty("ViewName",
		  m_viewName = "view1",
		  "Name of event view to use");

  declareProperty( "AllViews",
                  m_r_views = SG::ReadHandle< std::vector< SG::View* > >( "all_views" ),
                  "All views" );
}

// Destructor
///////////////
DFlowAlg1_manualViews::~DFlowAlg1_manualViews()
{}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode DFlowAlg1_manualViews::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode DFlowAlg1_manualViews::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode DFlowAlg1_manualViews::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  //Use views if told to
  if ( m_viewName != "" )
  {
    if ( !m_r_views.isValid() )
    {
      ATH_MSG_ERROR( "Failed to access views container" );
      return StatusCode::FAILURE;
    }

    //Examine all views
    bool foundView = false;
    for ( SG::View * view : *( m_r_views ) )
    {
      //Find the view by name
      if ( view->name() == m_viewName )
      {
        foundView = true;
        StatusCode sc = m_w_int.setStore( view );
        if ( !sc.isSuccess() ) ATH_MSG_ERROR( "Failed to load view " << m_viewName );
        break;
      }
    }
    if ( !foundView ) ATH_MSG_ERROR( "Failed to find view " << m_viewName );
  }

  CHECK( m_r_evtInfo.setStore(0) );
  if (!m_r_evtInfo.isValid())
  {
    ATH_MSG_ERROR("Could not get the EventInfo object. Going to next event");
    return StatusCode::RECOVERABLE;
  }
  ATH_MSG_INFO("evtinfo handle...");
  ATH_MSG_INFO("name: [" << m_r_evtInfo.name() << "]");
  ATH_MSG_INFO("store [" << m_r_evtInfo.store() << "]");
  ATH_MSG_INFO("clid: [" << m_r_evtInfo.clid() << "]");
  const EventInfo* ei = &*m_r_evtInfo;
  ATH_MSG_INFO("ei: " << ei);
  const EventID* eid = ei->event_ID();    
  ATH_MSG_INFO("retrieving event-info...");
  unsigned int runnbr = eid->run_number();
  ATH_MSG_INFO("evt-info.runnbr: " << runnbr);
  ATH_MSG_INFO("evt-info.evtnbr: " << eid->event_number());
  
  ATH_MSG_INFO("myint handle...");
  ATH_MSG_INFO("name: [" << m_w_int.name() << "]");
  ATH_MSG_INFO("store [" << m_w_int.store() << "]");
  ATH_MSG_INFO("clid: [" << m_w_int.clid() << "]");
  
  m_w_int = CxxUtils::make_unique<int>( m_r_evtInfo->event_ID()->event_number() );
  //m_w_int = int( m_r_evtInfo->event_ID()->event_number() );

  //redundant check as op = would throw if m_w_int was not valid (e.g. because if clid/key combo was duplicated)
  if (m_w_int.isValid())
  {
    ATH_MSG_INFO("ptr: " << m_w_int.cptr());
    ATH_MSG_INFO("val: " << *m_w_int);
    
    ATH_MSG_INFO("modify myint by value...");

    //Make different values for different views
    if ( m_viewName == "view1" )
    {
      m_w_int = m_r_evtInfo->event_ID()->event_number() + 10;
    }
    else
    {
      m_w_int = m_r_evtInfo->event_ID()->event_number() + 20;
    }

    ATH_MSG_INFO("ptr: " << m_w_int.cptr());
    ATH_MSG_INFO("val: " << *m_w_int);
  }

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

} //> end namespace AthViews
