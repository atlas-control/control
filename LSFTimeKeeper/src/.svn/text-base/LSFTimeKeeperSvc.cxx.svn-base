#include "GaudiKernel/MsgStream.h"
#include "LSFJobInfo.h"
#include "LSFTimeKeeper/LSFTimeKeeperSvc.h"

LSFTimeKeeperSvc::~LSFTimeKeeperSvc(){
  delete m_lsfJobInfo;
}

LSFTimeKeeperSvc::LSFTimeKeeperSvc(const std::string& name,
						   ISvcLocator* svc)
  : Service(name,svc)
{  
  m_lsfJobInfo = new LSF::JobInfo(); //FIXME should be singleton?
}

bool LSFTimeKeeperSvc::isLSFJob() const {
  return (m_lsfJobInfo->isLSFJob());
}

time_t LSFTimeKeeperSvc::timeL() const {
  return (isLSFJob() ? m_lsfJobInfo->timeL() : allocTime());
}

time_t LSFTimeKeeperSvc::timeX() const {
  return (isLSFJob() ? m_lsfJobInfo->timeX() : 0);
}

time_t
LSFTimeKeeperSvc::allocTime() const {
  return m_lsfJobInfo->allocTime();
}

const std::string&  
LSFTimeKeeperSvc::unitLabel() const {
  static const std::string label("unnormalized CPU secs");
  return label;
}


bool
LSFTimeKeeperSvc::nextIter() {
  bool doNext(TimeKeeper::nextIter());
  //we override to put out a DEBUG message per event or an INFO message at
  //the end
  MsgStream log(msgSvc(), name());
  if ((log.level() == MSG::INFO && timeOver()) ||
       log.level()  < MSG::INFO)  std::cout << *this << std::endl;
  //FIXME       log.level()  < MSG::INFO)  log << *this << endreq;
  return doNext;
}


StatusCode 
LSFTimeKeeperSvc::initialize()
{
  MsgStream log( messageService(), name() );
  log << MSG::INFO << "Initializing " << name()
      << " - package version " << PACKAGE_VERSION << endreq ; 
  return Service::initialize();
}

StatusCode  
LSFTimeKeeperSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( ITimeKeeper::interfaceID().versionMatch(riid) )    {
    *ppvInterface = (ITimeKeeper*)this;
  }
  else  {
    // Interface is not directly available: try out a base class
    return Service::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}


StatusCode 
LSFTimeKeeperSvc::finalize()
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << *this << endreq;
  return Service::finalize();
}

std::ostream& operator << (std::ostream& ost, const LSFTimeKeeperSvc& ltks) {
  if (ltks.isLSFJob()) {
    ost << (TimeKeeper&)ltks;
  } else {
    ost << "ERROR NOT AN LSF JOB: no TimeKeeper functionality";
  }
  return ost;
}

MsgStream& operator << (MsgStream& ost, const LSFTimeKeeperSvc& ltks) {
  if (ltks.isLSFJob()) {
    ost << (TimeKeeper&)ltks;
  } else {
    ost << "ERROR NOT AN LSF JOB: no TimeKeeper functionality";
  }
  return ost;
}
