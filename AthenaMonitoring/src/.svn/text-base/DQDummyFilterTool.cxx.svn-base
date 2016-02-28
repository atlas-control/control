#include "AthenaMonitoring/DQDummyFilterTool.h"

DQDummyFilterTool::DQDummyFilterTool(const std::string& type,const std::string& name,const IInterface* parent)
: AthAlgTool( type, name, parent )
{
  declareInterface<IDQFilterTool>(this);
}
        
DQDummyFilterTool::~DQDummyFilterTool () {}

bool DQDummyFilterTool::accept() {
  return true;
}

