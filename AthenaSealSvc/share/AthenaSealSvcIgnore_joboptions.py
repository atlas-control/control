
AthenaSealSvc = svcMgr.AthenaSealSvc

# List of class to ignore for dictionary checks
AthenaSealSvc.IgnoreNames  = [ "GaudiPython::Interface", "GaudiPython::Helper" ]
AthenaSealSvc.IgnoreNames += [ "Gaudi" ]
AthenaSealSvc.IgnoreNames += [ "FactoryTable" ]
AthenaSealSvc.IgnoreNames += [ "IInterface" ]
AthenaSealSvc.IgnoreNames += [ "IFactory" ]
AthenaSealSvc.IgnoreNames += [ "IAlgFactory" ]
AthenaSealSvc.IgnoreNames += [ "ISvcFactory" ]
AthenaSealSvc.IgnoreNames += [ "IToolFactory" ]
AthenaSealSvc.IgnoreNames += [ "InterfaceID" ]
AthenaSealSvc.IgnoreNames += [ "StatusCode" ]
AthenaSealSvc.IgnoreNames += [ "IAppMgrUI" ]
AthenaSealSvc.IgnoreNames += [ "IProperty" ]
AthenaSealSvc.IgnoreNames += [ "Property" ]
AthenaSealSvc.IgnoreNames += [ "std::vector<Property*>" ]
AthenaSealSvc.IgnoreNames += [ "std::vector<const Property*>" ]
AthenaSealSvc.IgnoreNames += [ "std::list<IAlgorithm*>" ]
AthenaSealSvc.IgnoreNames += [ "std::list<IService*>" ]
AthenaSealSvc.IgnoreNames += [ "std::list<const IFactory*>" ]
AthenaSealSvc.IgnoreNames += [ "std::vector<IRegistry*>" ]
AthenaSealSvc.IgnoreNames += [ "SimpleProperty" ]
AthenaSealSvc.IgnoreNames += [ "SimplePropertyRef" ]
AthenaSealSvc.IgnoreNames += [ "IService" ]
AthenaSealSvc.IgnoreNames += [ "IAlgorithm" ]
AthenaSealSvc.IgnoreNames += [ "ISvcManager" ]
AthenaSealSvc.IgnoreNames += [ "IAlgManager" ]
AthenaSealSvc.IgnoreNames += [ "IJobOptionsSvc" ]
AthenaSealSvc.IgnoreNames += [ "ISvcLocator" ]
AthenaSealSvc.IgnoreNames += [ "IEventProcessor" ]
AthenaSealSvc.IgnoreNames += [ "IDataProviderSvc" ]
AthenaSealSvc.IgnoreNames += [ "IDataManagerSvc" ]
AthenaSealSvc.IgnoreNames += [ "IRegistry" ]
AthenaSealSvc.IgnoreNames += [ "ContainedObject" ]
AthenaSealSvc.IgnoreNames += [ "std::vector<const ContainedObject*>" ]
AthenaSealSvc.IgnoreNames += [ "DataObject" ]
AthenaSealSvc.IgnoreNames += [ "IHistogramSvc" ]
AthenaSealSvc.IgnoreNames += [ "AIDA::I" ]
AthenaSealSvc.IgnoreNames += [ "Algorithm" ]
AthenaSealSvc.IgnoreNames += [ "Service" ]
AthenaSealSvc.IgnoreNames += [ "GaudiPython::PyAlgorithm" ]
AthenaSealSvc.IgnoreNames += [ "GaudiPython::PyAlgorithmWrap" ]
AthenaSealSvc.IgnoreNames += [ "IParticlePropertySvc" ]
AthenaSealSvc.IgnoreNames += [ "ParticleProperty" ]
AthenaSealSvc.IgnoreNames += [ "StoreGateSvc" ]
AthenaSealSvc.IgnoreNames += [ "IStoragePolicy" ]

# I don't know where the following classes come from????  RDS 2004-07-06
AthenaSealSvc.IgnoreNames += [ "CharDbArray" ]
AthenaSealSvc.IgnoreNames += [ "DoubleDbArray" ]
AthenaSealSvc.IgnoreNames += [ "FloatDbArray" ]
AthenaSealSvc.IgnoreNames += [ "IntDbArray" ]
AthenaSealSvc.IgnoreNames += [ "LongDbArray" ]
AthenaSealSvc.IgnoreNames += [ "ShortDbArray" ]


AthenaSealSvc.IgnoreDicts += [ "libSealCLHEPDict" ]

# Dicts added by athena.py
AthenaSealSvc.IgnoreNames += [ "AthenaEventLoopMgr" ]
AthenaSealSvc.IgnoreNames += [ "MinimalEventLoopMgr" ]
AthenaSealSvc.IgnoreNames += [ "PyAthenaEventLoopMgr" ]
AthenaSealSvc.IgnoreNames += [ "NTuple::Directory" ]
AthenaSealSvc.IgnoreNames += [ "NTuple::File " ]
AthenaSealSvc.IgnoreNames += [ "NTuple::Tuple" ]
AthenaSealSvc.IgnoreNames += [ "INTuple" ]
AthenaSealSvc.IgnoreNames += [ "NTuple::Tuple" ]

# Ignore extra template args
AthenaSealSvc.IgnoreNames += [ "greater<int>" ]
AthenaSealSvc.IgnoreNames += [ "allocator<" ]
