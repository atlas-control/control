# @file AthenaRootComps/python/ReadAres.py
# @purpose make the Athena framework read a set of ROOT files to emulate the
#          usual TTree event loop.
#          The objects contained in the ROOT files are actually the persistent
#          objects which are converted to the transient ones by
#          AthenaROOTAccess.
# @author Sebastien Binet <binet@cern.ch>

def _configure():
    """Install the Ares event service (AthenaROOTAccess) and the according
    athena-based TTree event selector.
    Correctly configure a few other services.
    """
    from AthenaCommon import CfgMgr
    from AthenaCommon.AppMgr import theApp
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    from AthenaCommon.Logging import logging
    msg = logging.getLogger( 'ReadAres' )
    msg.debug("Configuring Athena for reading POOL files (via ARA)...")

    if not hasattr(svcMgr, 'THistSvc'):
        svcMgr += CfgMgr.THistSvc()

    if hasattr(svcMgr, 'EventSelector'):
        err = "svcMgr already configured with another EventSelector: [%s]"%\
            svcMgr.EventSelector.getFullJobOptName()
        msg.error( err )
        raise RuntimeError( err )

    svcMgr += CfgMgr.AresEventSelector( "EventSelector" )
    theApp.ExtSvc += [ svcMgr.EventSelector.getFullName() ]
    theApp.EvtSel = "EventSelector"

    # add a helper service whose sole purpose is to be initialized early
    # to populate early on the THistSvc w/ TTrees/TChains
    from Ares import AresSvc
    svc = AresSvc()
    # we add the helper svc to the list of children of the event selector
    # so configurable.setup() ordering is good
    svcMgr.EventSelector += svc
    theApp.CreateSvc += [ svc.getFullName() ]
    
    msg.debug("Configuring Athena for reading POOL files (via ARA)... [OK]")
    return

# execute at module import
_configure()

# clean-up namespace
del _configure
