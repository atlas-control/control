## @file:    AthenaRootComps/python/Ares.py
## @purpose: module for binding Athena and ARA together
## @author:  Sebastien Binet <binet@cern.ch>

from AthenaPython import PyAthena
StatusCode = PyAthena.StatusCode

class AresSvc(PyAthena.Svc):
    """A service providing access to AthenaRootAccess transient tree,
    essentially melting Athena's and AthenaROOTAccess' event loops together
    """

    def __init__(self, name='AresSvc', **kw):
        # init base class
        kw['name']=name
        super(AresSvc,self).__init__(**kw)

        # input files
        self.inputFiles = kw.get('InputCollections', ['aod.pool'])

        # properties reflecting the ARA.transientTree.makeTree function's args
        self.persTreeName = kw.get('PersTreeName', 'CollectionTree')
        self.dhTreeName   = kw.get('DhTreeName',   'POOLContainer_DataHeader')
        self.dhBranchName = kw.get('DhBranchName', 'DataHeader_p2')
        self.branchNames  = kw.get('BranchNames',  dict())

        # handle to hist svc
        self.hsvc = None

        # handle to ara tree
        self.ara_tree = None

        # ares stream name: where we publish the transient tree in @c ITHistSvc
        self.__ares_tree_name = '/temp/TTreeStream/%s_trans'

        return

    def initialize(self):
        _info = self.msg.info
        _info('==> initialize...')
        # the ugly intertwined part: get some properties from
        # the AresEventSelector
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        _evtSel = svcMgr.EventSelector
        self.inputFiles   = _evtSel.InputCollections[:]
        self.persTreeName = _evtSel.TupleName
##         self.dhTreeName   = _evtSel.DhTreeName
##         self.dhBranchName = _evtSel.DhBranchName
##         self.branchNames  = _evtSel.BranchNames or dict()
##         del _evtSel

        _info('reading files: %r', self.inputFiles)
        
        # try to load the ttree speed-ups
        try:
            from RootUtils.PyROOTFixes import enable_tree_speedups
            enable_tree_speedups()
        except ImportError:
            _info('could NOT install the ttree speedups...')
            pass

        import ROOT, PyCintex;
        # load an ARA chain (and fool checkreq)
        Ara = __import__ ('AthenaROOTAccess').transientTree
        TChainROOTAccess = ROOT.AthenaROOTAccess.TChainROOTAccess
        self.__ara_chain = TChainROOTAccess(self.persTreeName)
        for f in self.inputFiles:
            self.msg.debug('adding [%r]', f)
            self.__ara_chain.Add (f)
            
        _info('loading ara...')
        self.ara_tree = Ara.makeTree(self.__ara_chain,
                                     persTreeName=self.persTreeName,
                                     dhTreeName=self.dhTreeName,
                                     dhBranchName=self.dhBranchName,
                                     branchNames=self.branchNames)
        assert isinstance(self.ara_tree, ROOT.TTree),\
               "problem while creating transient tree w/ ara"
        
        self.nTotEntries = self.ara_tree.GetEntriesFast()
        _info('#entries: %i', self.nTotEntries)
        
        self.hsvc = PyAthena.py_svc('THistSvc')
        if not self.hsvc:
            self.msg.error('could not retrieve THistSvc')
            return StatusCode.Failure

        ares_tree_name = self.__ares_tree_name%self.persTreeName
        _info('registering transient tree with ITHistSvc: [%s]',
              ares_tree_name)
        self.hsvc[ares_tree_name]=self.ara_tree
        return StatusCode.Success

    def finalize(self):
        _info = self.msg.info
        _info('==> finalize')

        # unregister the transient tree, otherwise hist svc will complain...
        ares_tree_name = self.__ares_tree_name%self.persTreeName
        del self.hsvc[ares_tree_name]
        del self.hsvc
        del self.ara_tree
        del self.__ara_chain
        import ROOT
        for pool_file in ROOT.gROOT.GetListOfFiles():
            try:
                idx = self.inputFiles.index(pool_file.GetName())
            except ValueError:
                continue
            del self.inputFiles[idx]
            pool_file.Close()
        for f in self.inputFiles:
            self.msg.debug('could not find nor close POOL file [%s]',f)
        return StatusCode.Success

    # class AresSvc
