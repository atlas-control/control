# @file AthenaRootComps/python/ReadAthenaRoot.py
# @purpose make the Athena framework read a set of ROOT files to emulate the
#          usual TTree event loop
# @author Sebastien Binet <binet@cern.ch>
#
# Original code from Tadashi Maeno (AANTEventSelector)

def _configure():
    """Install the Athena-based TTree event selector and correctly configure
    a few other services.
    """
    from AthenaCommon import CfgMgr
    from AthenaCommon.AppMgr import theApp
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    from AthenaCommon.Logging import logging
    msg = logging.getLogger( 'ReadAthenaRoot' )
    msg.debug("Configuring Athena for reading ROOT files (via TChain)...")

    if not hasattr(svcMgr, 'THistSvc'):
        svcMgr += CfgMgr.THistSvc()

    if hasattr(svcMgr, 'EventSelector'):
        err = "svcMgr already configured with another EventSelector: [%s]"%\
            svcMgr.EventSelector.getFullJobOptName()
        msg.error( err )
        raise RuntimeError( err )

    # Load ProxyProviderSvc
    if not hasattr (svcMgr, 'ProxyProviderSvc'):
        svcMgr += CfgMgr.ProxyProviderSvc()
        pass

    from Configurables import TTreeEventSelector
    evtsel = TTreeEventSelector( "EventSelector" )
    svcMgr += evtsel
    theApp.ExtSvc += [ evtsel.getFullName() ]
    theApp.EvtSel = "EventSelector"
    svcMgr.ProxyProviderSvc.ProviderNames += [evtsel.getFullName()]
    del evtsel
    
    # add a helper service whose sole purpose is to be initialized early
    # to populate early on the THistSvc w/ TTrees/TChains
    svc = CfgMgr.TTreeEventSelectorHelperSvc()
    # we add the helper svc to the list of children of the event selector
    # so configurable.setup() ordering is good
    svcMgr.EventSelector += svc
    theApp.CreateSvc += [ svc.getFullName() ]
    
    # configure the cnvsvc
    svcMgr += CfgMgr.Athena__NtupleCnvSvc()
    if not hasattr(svcMgr, 'EventPersistencySvc'):
        svcMgr += CfgMgr.EvtPersistencySvc( "EventPersistencySvc" )
    svcMgr.EventPersistencySvc.CnvServices += [ "Athena::NtupleCnvSvc" ]
    
    msg.debug("Configuring Athena for reading ROOT files (via TChain)... [OK]")
    return

# execute at module import
_configure()

# clean-up namespace
del _configure
