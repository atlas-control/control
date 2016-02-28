///////////////////////// -*- C++ -*- /////////////////////////////
// AthViewAlgorithm.h 
// Header file for class AthViewAlgorithm
// Author: B Wynne, based on S Binet's AthAlgorithm
/////////////////////////////////////////////////////////////////// 

// AthenaBaseComps includes
#include "AthViews/AthViewAlgorithm.h"
#include "AthViews/AthEventContext.h"

// STL includes

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
AthViewAlgorithm::AthViewAlgorithm( const std::string& name, 
			    ISvcLocator* pSvcLocator,
			    const std::string& version ) :
  AthAlgorithm( name, pSvcLocator, version )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );
  
  //Flags for requiring views (or not)
  m_require_view = false;
  m_require_not_view = false;
  declareProperty( "RequireView", m_require_view, "True if this algorithm may only run in a view" );
  declareProperty( "RequireNotView", m_require_not_view, "True if this algorithm may only run on whole events" );
}

// Destructor
///////////////
AthViewAlgorithm::~AthViewAlgorithm()
{ 
  ATH_MSG_DEBUG ("Calling destructor");
}

StatusCode AthViewAlgorithm::sysInitialize()
{
  //Check for stupid
  if ( m_require_view && m_require_not_view )
  {
    ATH_MSG_FATAL( "BEN BEN AthViewAlgorithm config fail since requiring view and not view" );
    return StatusCode::FAILURE;
  }

  return AthAlgorithm::sysInitialize();
}

StatusCode AthViewAlgorithm::sysExecute()
{
  ATH_MSG_INFO( "BEN BEN modified sysExecute for " << name() );

  //Skip the algorithm if views are required or avoided
  SG::View * myView = eventView();
  if ( !myView && m_require_view )
  {
    ATH_MSG_INFO( "BEN BEN skipping execution of " << name() << ": not in event view" );
    return StatusCode::SUCCESS;
  }
  if ( myView && m_require_not_view )
  {
    ATH_MSG_INFO( "BEN BEN skipping execution of " << name() << ": in event view" );
    return StatusCode::SUCCESS;
  }

  //Set all DataHandles to use the EventView pointer
  for ( auto handle : inputHandles() )
  {
    SG::VarHandleBase * athenaHandle = ( SG::VarHandleBase* )handle;
    CHECK( athenaHandle->setStore( myView ) );
  }
  for ( auto handle : outputHandles() )
  {
    SG::VarHandleBase * athenaHandle = ( SG::VarHandleBase* )handle;
    CHECK( athenaHandle->setStore( myView ) );
  }

  return AthAlgorithm::sysExecute();
}

//Retrieve the EventView pointer from the context if it exists
SG::View * AthViewAlgorithm::eventView()
{
  //Old and hacked
  /*AthEventContext * viewsContext = dynamic_cast< AthEventContext* >( m_event_context );
  if ( viewsContext )
  {
    ATH_MSG_INFO( "BEN BEN alg " << name() << " is in view " << viewsContext->getView() );
    return viewsContext->getView();
  }
  else
  {
    return 0;
  }*/

  //New and hopefully working
  if ( !m_event_context ) return 0; //but why no context?
  SG::View * myView = dynamic_cast< SG::View* >( m_event_context->proxy() );
  if ( myView )
  {
    ATH_MSG_INFO( "BEN BEN alg " << name() << " is in view " << myView );
  }
  return myView;
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

