///////////////////////// -*- C++ -*- /////////////////////////////
// AthAlgTool.cxx 
// Implementation file for class AthAlgTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// FrameWork includes

// AthenaBaseComps includes
#include "AthenaBaseComps/AthAlgTool.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
AthAlgTool::AthAlgTool( const std::string& type, 
			const std::string& name, 
			const IInterface* parent ) : 
  ::AlgTool      ( type, name, parent ),
  ::AthMessaging ( msgSvc(),     name ),
  m_evtStore     ( "StoreGateSvc/StoreGateSvc",  name ),
  m_detStore     ( "StoreGateSvc/DetectorStore", name ),
  m_userStore    ( "UserDataSvc/UserDataSvc", name )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  auto props = getProperties();
  for( Property* prop : props ) {
    if( prop->name() != "OutputLevel" ) {
      continue;
    }
    prop->declareUpdateHandler
      (&AthAlgTool::msg_update_handler, this);
    break;
  }

  declareProperty( "EvtStore",
                   m_evtStore = StoreGateSvc_t ("StoreGateSvc", name),
                   "Handle to a StoreGateSvc instance: it will be used to "
                   "retrieve data during the course of the job" );

  declareProperty( "DetStore",
                   m_detStore = StoreGateSvc_t ("StoreGateSvc/DetectorStore", name),
                   "Handle to a StoreGateSvc/DetectorStore instance: it will be used to "
                   "retrieve data during the course of the job" );

  declareProperty( "UserStore",
                   m_userStore = UserDataSvc_t ("UserDataSvc/UserDataSvc", name),
                   "Handle to a UserDataSvc/UserDataSvc instance: it will be used to "
                   "retrieve user data during the course of the job" );
}

// Destructor
///////////////
AthAlgTool::~AthAlgTool()
{ 
  ATH_MSG_DEBUG ("Calling destructor");
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

void 
AthAlgTool::msg_update_handler(Property& /*m_outputLevel*/) 
{
  msg().setLevel (outputLevel());
}
