#include <cassert>
#include <cmath>      /*ceil,sqrt*/
#include <stdexcept>  /*runtime_error*/
#include <string>

#include <boost/bind.hpp>

#include "AthenaKernel/IAtRndmGenSvc.h"

#include "StoreGate/ActiveStoreSvc.h" 
#include "StoreGate/StoreGateSvc.h" /*to print name() */
#include "GaudiKernel/IEvtSelector.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"
#include "EventInfo/PileUpEventInfo.h"
#include "EventInfo/PileUpTimeEventIndex.h"
#include "EventInfo/EventID.h"
#include "PileUpTools/IBeamIntensity.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include "BkgStreamsCache.h"

#ifdef DEBUG_PILEUP
#include <algorithm> 
#endif


BkgStreamsCache::BkgStreamsCache( const std::string& type, 
				  const std::string& name,
				  const IInterface* parent) : 
  AthAlgTool( type, name, parent ),
  p_activeStore(0),
  m_nXings(0),
  m_nStores(0),
  m_collXing(23.0),
  m_occupationFraction(1.0),
  m_collDistrName("Poisson"),
  m_selecName("FakeEventSelector", name),
  m_readDownscale(150),
  m_atRndmSvc("AtRndmGenSvc", name),
  m_randomStreamName("PileUpCollXingStream"),
  m_pileUpEventTypeProp(0),
  m_pileUpEventType(PileUpTimeEventIndex::Signal),
  m_subtractBC0(0),
  m_ignoreBM(false),
  m_readEventRand(0),
  m_chooseEventRand(0),
  m_collXingPoisson(0),  
  f_collDistr(0),
  f_numberOfBackgroundForBunchCrossing(0),
  m_collXingSF(1.0),
  m_ignoreSF(false),
  m_zeroXing(-1),
  m_beamInt(0),
  m_forceReadForBC0(true)
{   
  declareProperty("CollPerXing", m_collXing, "(average) number of collisions per beam crossing");  
  declareProperty("OccupationFraction", m_occupationFraction, "The maximum fraction of bunch-crossings which will be occupied."); 
  declareProperty("CollDistribution", m_collDistrName, "nEvts/Xings can be either Fixed at CollPerXing or Poisson with average CollPerXing");
  declareProperty("EventSelector", m_selecName);
  declareProperty("PileUpEventType", m_pileUpEventTypeProp, "Type of the pileup events in this cache: 0:Signal, 1:MinimumBias, 2:Cavern, 3:HaloGas, 4:ZeroBias. Default=0 (Signal, Invalid)");
  declareProperty("ReadDownscaleFactor", m_readDownscale, "read one event every downscaleFactor accesses (asymptotically -> number of times an event in the cache will be reused)"); 
  declareProperty("RndmGenSvc", m_atRndmSvc, "IAtRndmGenSvc controlling the distribution of bkg events/xing");
  declareProperty("RndmStreamName", m_randomStreamName, "IAtRndmGenSvc stream used as engine for our various random distributions, including the CollPerXing one ");  
  declareProperty("SubtractBC0", m_subtractBC0, "reduce the number of events at bunch xing t=0 by m_subtractBC0. Default=0, set to 1 when using the same type of events (e.g. minbias) for original and background streams");
  m_pileUpEventTypeProp.verifier().setUpper(PileUpTimeEventIndex::NTYPES-2);
  m_pileUpEventTypeProp.declareUpdateHandler(&BkgStreamsCache::PileUpEventTypeHandler, this);
  declareProperty("IgnoreBeamInt", m_ignoreBM, "Default=False, set to True to ignore the PileUpEventLoopMgr beam intensity tool in setting the number of events per xing.");
  declareProperty("IgnoreBeamLumi", m_ignoreSF, "Default=False, set to True to ignore the PileUpEventLoopMgr beam luminosity tool in setting the number of events per xing.");
  declareProperty("ForceReadForBC0",m_forceReadForBC0,"Force events used in the central bunch crossing to be refreshed");
}

BkgStreamsCache::~BkgStreamsCache() {
  delete m_collXingPoisson;
  delete m_chooseEventRand;
  delete m_readEventRand;
}

StatusCode
BkgStreamsCache::queryInterface(const InterfaceID& riid, void** ppvif) {
  if ( riid == IBkgStreamsCache::interfaceID() ) {
    *ppvif = (IBkgStreamsCache*)this;
    addRef();
    return StatusCode::SUCCESS;
  }
  return AthAlgTool::queryInterface( riid, ppvif );
}

void
BkgStreamsCache::PileUpEventTypeHandler(Property&) {
  m_pileUpEventType=PileUpTimeEventIndex::ushortToType(m_pileUpEventTypeProp.value());
}

StatusCode
BkgStreamsCache::setup(int firstXing,
		       unsigned int nXings, 
		       unsigned int firstStore,
		       IBeamIntensity* iBM)
{
  assert (0 < nXings);
  m_nXings = nXings;
  m_zeroXing = -firstXing;
  if (m_zeroXing >= (int)nXings) m_zeroXing = -99;
  m_beamInt = (m_ignoreBM.value() ? 0 : iBM);
  float largestElement( ( m_ignoreBM.value() || m_collDistrName.value() == "Fixed" ) ? 1.0 : m_beamInt->largestElementInPattern() );
  //ceil (cmath) rounds up to the nearest integer ceil(1.5)= 2
  float occupiedCrossings = ceil(static_cast<float>(m_nXings) * m_occupationFraction);
  m_nStores = static_cast<unsigned int>(ceil( m_collXing * occupiedCrossings * largestElement));
  if (m_collDistrName.value() == "Poisson") {
    //allow for fluctuations in # of overlaid events
    m_nStores += static_cast<unsigned int>(6.0 * sqrt(static_cast<float>(m_nStores)));
  }

  //get bkg selectors, stores and iterators
  ATH_MSG_DEBUG ( "Set up " << m_nStores << " stores" );
  
  m_nEvtsXing.reserve(m_nStores);
  m_streams.reserve(m_nStores);
  m_usedStreams.reserve(m_nStores);
  m_usedStreams.assign(m_nStores, false);

  const std::string& selecName(m_selecName.name());
  for (unsigned int i=0; i < m_nStores; ++i) {
    std::stringstream bufName;
    bufName << "BkgEvent_" << i + firstStore;
    const std::string& streamName(bufName.str());    
    try {
      m_streams.push_back(PileUpStream(streamName,serviceLocator(),selecName)); 
    } catch (std::runtime_error e) {
      ATH_MSG_ERROR ( "Exception thrown while creating PileUpStream " 
		      << streamName << " : " << e.what() );     
      return StatusCode::FAILURE;
    }
    
    if(!(m_streams.back().setupStore())) {
      ATH_MSG_ERROR ( "Can not setup bkg evt store for stream " 
		      << streamName );
      return StatusCode::FAILURE;
    }
  }
  m_cursor = m_streams.begin();

  CLHEP::HepRandomEngine* collEng(m_atRndmSvc->GetEngine(m_randomStreamName.value()));
  if(0 == collEng ) {
    ATH_MSG_ERROR (  "can not get random stream " << m_randomStreamName.value() );
    return StatusCode::FAILURE;
  }
  //setup generator to pickup an event store at random from the cache
  //notice how we pass collEng by reference. If ! CLHEP will take ownership...
  m_chooseEventRand = new CLHEP::RandFlat(*(collEng),
					  0.0, double(m_nStores));

  return StatusCode::SUCCESS;
}

void BkgStreamsCache::resetEvtsPerXingScaleFactor(float sf) {
  if (!m_ignoreSF) m_collXingSF = sf;
}

long BkgStreamsCache::collXingPoisson(){ 
  if (!m_collXingPoisson) { return collXing(); }
  return m_collXingPoisson->fire(m_collXing * m_collXingSF); 
}

void BkgStreamsCache::newEvent() { 
  ATH_MSG_DEBUG (  "newEvent called resetting used event set" );
  unsigned int totEvts(0);
  do {
    totEvts = 0;
    for (unsigned int iXing=0; iXing<m_nXings; ++iXing) {
      totEvts += setNEvtsXing(iXing);
      if (totEvts > m_nStores) {
	ATH_MSG_WARNING ( "newEvent: number of required evts (" << totEvts << ") exceeds number of available stores "
			  << m_nStores << ". Regenerating bkg sequence for this event \n"
			  << " the total number of bkg events may not exceed average by more than 6 sigmas"
			  );
	break;
      }
    }
  } while(totEvts > m_nStores);
  m_usedStreams.assign(m_streams.size(), false); 
}

//TODO update this method!!
const EventInfo* BkgStreamsCache::nextEvent(bool isCentralBunchCrossing) { 
  const EventInfo* pNextEvt(0);
  StreamVector::size_type iS(0);
  do { 
    iS = (StreamVector::size_type)m_chooseEventRand->fire();
  } while (alreadyInUse(iS));

  //set current store to iS
  m_cursor = m_streams.begin();
  std::advance(m_cursor, iS);
     
  ATH_MSG_DEBUG (  "using store " << iS );
  PileUpStream* pCurrStream(current());
  if (0 != pCurrStream) {
    p_activeStore->setStore(&(pCurrStream->store()));
    //read a new event every downscaleFactor accesses
    //force reading of new event if the event is being used for in-time pile-up
    //FIXME a more careful strategy would have the PileUpStreams knowing if
    //they have been used for the central bunch-crossing already.
    bool readEvent(isCentralBunchCrossing || (m_readEventRand->fire()<1.0));
    pNextEvt=pCurrStream->nextEventPre(readEvent);
  }
  return pNextEvt;
}

//TODO update this method!!
StatusCode BkgStreamsCache::nextEvent_passive(bool isCentralBunchCrossing) { 
  StreamVector::size_type iS(0);
  do { 
    iS = (StreamVector::size_type)m_chooseEventRand->fire();
  } while (alreadyInUse(iS));

  //set current store to iS
  m_cursor = m_streams.begin();
  std::advance(m_cursor, iS);
     
  ATH_MSG_DEBUG (  "using store " << iS );
  PileUpStream* pCurrStream(current());
  if (0 != pCurrStream) {
    p_activeStore->setStore(&(pCurrStream->store()));
    //read a new event every downscaleFactor accesses
    //force reading of new event if the event is being used for in-time pile-up
    //FIXME a more careful strategy would have the PileUpStreams knowing if
    //they have been used for the central bunch-crossing already.
    bool readEvent(isCentralBunchCrossing || (m_readEventRand->fire()<1.0));
    if(pCurrStream->nextEventPre_Passive(readEvent)) return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

bool BkgStreamsCache::alreadyInUse(StreamVector::size_type iS) {
  assert(iS<m_usedStreams.size());
  bool inUse(m_usedStreams[iS]);
  if (!inUse) m_usedStreams[iS] = true;
#ifndef NDEBUG
  ATH_MSG_VERBOSE ( "alreadyInUse: store " << iS  << ' '
		    << (inUse ? "already" : "not yet") << " in use" );
#endif
  return inUse;
} 

PileUpStream* BkgStreamsCache::current() { 
  if (m_cursor != m_streams.end()) return &*m_cursor;
  //      m_cursor->isNotEmpty()) return &*m_cursor;
  else return 0; //FIXME should reomve empty stream and keep going
}
 
StatusCode BkgStreamsCache::initialize() { 
  StatusCode sc(StatusCode::SUCCESS);
  // configure our MsgStream
  ATH_MSG_DEBUG (  "Initializing " << name()
		   << " - cache for events of type " 
		   << PileUpTimeEventIndex::typeName(m_pileUpEventType)
		   << " - package version " << PACKAGE_VERSION ) ;
  PileUpEventTypeHandler(m_pileUpEventTypeProp);
  //locate the ActiveStoreSvc and initialize our local ptr
  if (!(sc = service("ActiveStoreSvc", p_activeStore)).isSuccess() )  
    {
      ATH_MSG_ERROR (  "Error retrieving ActiveStoreSvc." );
      return sc;
    }

  //create random number generators
  if (!(m_atRndmSvc.retrieve()).isSuccess()) {
    ATH_MSG_ERROR (  "can not get IAtRndmGenSvc " << m_atRndmSvc );
    return StatusCode::FAILURE;
  }
  CLHEP::HepRandomEngine* collEng(m_atRndmSvc->GetEngine(m_randomStreamName.value()));
  if(0 == collEng ) {
    ATH_MSG_ERROR (  "can not get random stream " << m_randomStreamName.value() );
    return StatusCode::FAILURE;
  }

  //setup distribution to read a new event every downscaleFactor accesses
  //notice how we pass collEng by reference. If ! CLHEP will take ownership...
  m_readEventRand = new CLHEP::RandFlat(*(collEng),
					0.0, double(m_readDownscale));

  // select collision distribution functions
  if (m_collDistrName.value() == "Fixed") {
    f_collDistr = boost::bind(&BkgStreamsCache::collXing, this);
    if(m_ignoreBM.value()) {
      f_numberOfBackgroundForBunchCrossing = boost::bind(&BkgStreamsCache::numberOfBkgForBunchCrossingIgnoringBeamIntensity, this, _1);
    } else {
      f_numberOfBackgroundForBunchCrossing = boost::bind(&BkgStreamsCache::numberOfCavernBkgForBunchCrossing, this, _1);
    }
  }
  else if (m_collDistrName.value() == "Poisson") {
    //pass collEng by reference. If Not CLHEP will take ownership...
    m_collXingPoisson = new CLHEP::RandPoisson(*(collEng), m_collXing);
    // f_collDistr will call m_collXingPoisson->fire(m_collXing)  USED TO BE boost::bind(&CLHEP::RandPoisson::fire, m_collXingPoisson); 
    f_collDistr = boost::bind(&BkgStreamsCache::collXingPoisson, this);      
    if(m_ignoreBM.value()) {
      f_numberOfBackgroundForBunchCrossing = boost::bind(&BkgStreamsCache::numberOfBkgForBunchCrossingIgnoringBeamIntensity, this, _1);
    } else {
      f_numberOfBackgroundForBunchCrossing = boost::bind(&BkgStreamsCache::numberOfBkgForBunchCrossingDefaultImpl, this, _1);
    }
  } else {
    ATH_MSG_ERROR (  m_collDistrName 
		     << " is not a know collision distribution function" );
    sc = StatusCode::FAILURE;
  }
  return sc;
}

unsigned int BkgStreamsCache::nEvtsXing(unsigned int iXing) const {
  return (iXing + 1 > m_nEvtsXing.size()) ? 0 : m_nEvtsXing[iXing];
}

unsigned int BkgStreamsCache::numberOfBkgForBunchCrossingIgnoringBeamIntensity(unsigned int) const {
  return f_collDistr();
}
unsigned int BkgStreamsCache::numberOfBkgForBunchCrossingDefaultImpl(unsigned int iXing) const {
  return static_cast<unsigned int>(m_beamInt->normFactor(iXing-m_zeroXing)*static_cast<float>(f_collDistr()));
}
unsigned int BkgStreamsCache::numberOfCavernBkgForBunchCrossing(unsigned int iXing) const {
  return static_cast<unsigned int>(m_beamInt->normFactor(iXing-m_zeroXing)>0.0)*f_collDistr();
}

unsigned int BkgStreamsCache::setNEvtsXing(unsigned int iXing) {
  if (iXing + 1 > m_nEvtsXing.size())  m_nEvtsXing.resize(2 * iXing + 1);
  unsigned int nEvts(f_numberOfBackgroundForBunchCrossing(iXing));
  //this is done mainly to handle the case in which original and backround
  //events belong to the same stream. Of course we do not want m_nEvtsXing[m_zeroXing]<0 
  if ((int)iXing==m_zeroXing) {
    unsigned int subValue(m_subtractBC0.value());
    while (nEvts<subValue) nEvts = f_numberOfBackgroundForBunchCrossing(iXing);
    nEvts -= subValue;
#ifndef NDEBUG
    ATH_MSG_VERBOSE ( "Subtracted " <<   m_subtractBC0.value() 
		      << " events from BC=0 Xing " << iXing << " - Events at BC=0 "
		      << nEvts 
		      );
#endif
  }
  ATH_MSG_VERBOSE ( "Will pile-up " <<  nEvts
		    << " events for BCID " << iXing );
  

  m_nEvtsXing[iXing] = nEvts;
  return nEvts;
}

StatusCode BkgStreamsCache::addSubEvts(unsigned int iXing,
				       PileUpEventInfo& overEvent,
				       int t0BinCenter) {
  return this->addSubEvts(iXing, overEvent, t0BinCenter, true, 0);
}

StatusCode BkgStreamsCache::addSubEvts(unsigned int iXing,
				       PileUpEventInfo& overEvent,
				       int t0BinCenter, bool loadEventProxies, unsigned int BCID) {
  for (unsigned int iEvt=0; iEvt<nEvtsXing(iXing); ++iEvt) {
    StoreGateSvc* pBkgStore(0);
    
    // check if we're picking events for the central bunch-crossing, 
    // so we can choose to only use fresh events here.
    bool isCentralBunchCrossing((0==t0BinCenter)&&m_forceReadForBC0);
    // increment event iterators
    if(!loadEventProxies)
      {
	return this->nextEvent_passive(isCentralBunchCrossing);
      }
    const EventInfo* pBkgEvent(this->nextEvent(isCentralBunchCrossing)); //FIXME update this for the case where loadProxies=False

    //check input selector is not empty
    PileUpStream* currStream(this->current());
    if (0 == pBkgEvent || 0 == currStream) {
      // This is the end of the loop. No more events in the selection
      ATH_MSG_INFO ( "end of loop: background cache has no more events" );
      return StatusCode::FAILURE;
    } else {
      pBkgStore = &(currStream->store());
      ATH_MSG_DEBUG ( "added event " <<  pBkgEvent->event_ID()->event_number() 
		      << " run " << pBkgEvent->event_ID()->run_number()
		      << " from store " 
		      << pBkgStore->name()
		      << " @ Xing " << iXing );
    }
    
    //  register as sub event of the overlaid
    //    ask if sufficient/needed
    overEvent.addSubEvt(t0BinCenter, BCID,
			m_pileUpEventType,
			*pBkgEvent, pBkgStore);
#ifdef DEBUG_PILEUP
    const EventInfo* pStoreInfo(0);
    if (pBkgStore->retrieve(pStoreInfo).isSuccess() && pStoreInfo && 
	pBkgEvent->event_ID()->event_number() != pStoreInfo->event_ID()->event_number()) {
      ATH_MSG_ERROR ( "added event " <<  pBkgEvent->event_ID()->event_number() 
		      << " run " << pBkgEvent->event_ID()->run_number()
		      << " differ from current store " 
		      << pBkgStore->name()
		      << " event " <<  pStoreInfo->event_ID()->event_number() 
		      << " run " << pStoreInfo->event_ID()->run_number()
		      );
      assert(1);
    }
#endif
  } //loop over evts in xing
  return StatusCode::SUCCESS;
}

StatusCode BkgStreamsCache::finalize() { 
  StatusCode sc(StatusCode::SUCCESS);
  ATH_MSG_DEBUG ( "Finalizing " << name()
		  << " - cache for events of type " 
		  << PileUpTimeEventIndex::typeName(m_pileUpEventType)
		  << " - package version " << PACKAGE_VERSION );
  while (sc.isSuccess() && m_streams.size()>0) {
    sc=m_streams.back().finalize();
    m_streams.pop_back();
  }
  return sc;
}
