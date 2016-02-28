# job opts to activate the LSF-based TimeKeeper
# $Id: LSFTimeKeeperOptions.py,v 1.2 2004-09-23 22:53:23 droussea Exp $
theApp.Dlls += [ "LSFTimeKeeper" ]
theApp.ExtSvc += [ "LSFTimeKeeperSvc" ]
AthenaEventLoopMgr= Service("AthenaEventLoopMgr")
AthenaEventLoopMgr.TimeKeeper="LSFTimeKeeperSvc"
