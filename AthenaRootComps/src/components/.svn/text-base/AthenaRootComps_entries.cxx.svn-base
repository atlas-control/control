#include "GaudiKernel/DeclareFactoryEntries.h"
#include "../AresEventSelector.h"
#include "../TTreeEventSelector.h"
#include "../TTreeEventSelectorHelperSvc.h"
#include "../AthenaNtupleCnvSvc.h"
#include "../AthenaLeafCnv.h"

DECLARE_SERVICE_FACTORY( AresEventSelector )
DECLARE_SERVICE_FACTORY( TTreeEventSelector )
DECLARE_SERVICE_FACTORY( TTreeEventSelectorHelperSvc )
DECLARE_NAMESPACE_SERVICE_FACTORY(Athena, NtupleCnvSvc)
DECLARE_NAMESPACE_CONVERTER_FACTORY (Athena, LeafCnv)

DECLARE_FACTORY_ENTRIES( AthenaRootComps ) {
  DECLARE_SERVICE( AresEventSelector )
  DECLARE_SERVICE( TTreeEventSelector )
  DECLARE_SERVICE( TTreeEventSelectorHelperSvc )
  DECLARE_NAMESPACE_SERVICE(Athena, NtupleCnvSvc)
  DECLARE_NAMESPACE_CONVERTER(Athena, LeafCnv)
}
