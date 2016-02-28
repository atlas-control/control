// AthExStoreGateExample includes
#include "ViewMakeAlg.h"
#include "AthViews/AthEventContext.h"

// STL includes

// FrameWork includes
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IScheduler.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

#include "CxxUtils/make_unique.h"

namespace AthViews {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
ViewMakeAlg::ViewMakeAlg( const std::string& name, 
                      ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty("ViewStart",
                  m_w_int = SG::WriteHandle< int >("view_start"),
                  "A number to start off the view");

  declareProperty("AllViews",
                  m_w_views = SG::WriteHandle< std::vector< SG::View* > >("all_views"),
                  "All views");

  declareProperty("ViewNames",
                  m_viewNames = std::vector< std::string >(),
                  "View names");
}

// Destructor
///////////////
ViewMakeAlg::~ViewMakeAlg()
{
}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode ViewMakeAlg::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode ViewMakeAlg::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode ViewMakeAlg::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  //Make a view for each name given
  m_w_views = CxxUtils::make_unique< std::vector< SG::View* > >();
  //m_w_views = std::vector< SG::View* >();
  //size_t const dummyEventNumber = 20 + m_event_context->evt();
  for ( unsigned int viewIndex = 0; viewIndex < m_viewNames.size(); viewIndex++ )
  {
    std::string const viewName = m_viewNames[ viewIndex ];
    ATH_MSG_INFO( "Making new view: " << viewName );

    //Create the view object
    SG::View * newView = new SG::View( viewName );
    m_w_views->push_back( newView );

    //Write data to the new view
    StatusCode sc = m_w_int.setStore( newView );
    if ( !sc.isSuccess() ) ATH_MSG_ERROR( "setStore() failed for new view" );
    m_w_int = CxxUtils::make_unique<int>( viewIndex );
    //m_w_int = int( viewIndex );
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
