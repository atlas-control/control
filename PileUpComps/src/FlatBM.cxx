#include "FlatBM.h"
StatusCode 
FlatBM::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( IBeamIntensity::interfaceID().versionMatch(riid) )    {
    *ppvInterface = (IBeamIntensity*)this;
  }  else  {
    // Interface is not directly available: try out the base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}
