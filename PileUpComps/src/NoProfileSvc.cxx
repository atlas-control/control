#include "NoProfileSvc.h"
NoProfileSvc::NoProfileSvc(const std::string& name,ISvcLocator* svc): 
  AthService(name,svc)
{
}

NoProfileSvc::~NoProfileSvc() { 
}

StatusCode
NoProfileSvc::initialize() {
  ATH_MSG_INFO("No change in pileup vs. (run,lb).");
 return StatusCode::SUCCESS;
}

float NoProfileSvc::scaleFactor(unsigned int, unsigned int, bool & updated) {  
  updated = false;
  return 1.0;
}

StatusCode 
NoProfileSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( IBeamLuminosity::interfaceID().versionMatch(riid) )    {
    *ppvInterface = (IBeamLuminosity*)this;
  }  else  {
    // Interface is not directly available: try out the base class
    return Service::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}


