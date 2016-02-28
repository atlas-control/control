#ifndef LSFTIMEKEEPER_LSFTIMEKEEPERSVC_H
#define LSFTIMEKEEPER_LSFTIMEKEEPERSVC_H

#include <iostream> 

//base classes
#ifndef ATHENAKERNEL_TIMEKEEPER_H
 #include "AthenaKernel/TimeKeeper.h"
#endif
#ifndef GAUDIKERNEL_SERVICE_H
 #include "GaudiKernel/Service.h"
#endif

class MsgStream;
namespace LSF {
  class JobInfo;
}
template <class TYPE> class SvcFactory;

/** @class LSFTimeKeeperSvc
  * @brief service that implements ITimeKeeper using LSF batch queue info
  * 
  * @author pcalafiura@lbl.gov
  * $Id: LSFTimeKeeperSvc.h,v 1.4 2004-09-06 06:32:25 dquarrie Exp $
  */
class LSFTimeKeeperSvc   : virtual public TimeKeeper,
                 	           virtual public Service
{
public:
  // ITimeKeeper Implementation
  virtual bool nextIter();         ///< main user entry point
  virtual time_t timeL() const;    ///< time remaining before limit
  virtual time_t timeX() const;    ///< time used so far
  //INH  virtual bool timeOver() const;   ///< is there time for another iter?

  /// TimeKeeper Implementation
  //@{
  virtual time_t allocTime() const;       ///< allocated CPU time for job
  void updateTime() {}                    ///< this is a dummy for now  
  virtual const std::string& unitLabel() const;
  //@}


  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode queryInterface( const InterfaceID& riid, void** ppvInterface );
  //@}
  
  bool isLSFJob() const;                  ///< forward to LSF::JobInfo

protected:
  friend class SvcFactory<LSFTimeKeeperSvc>;
  
  /// Standard  Gaudi Svc Constructor
  LSFTimeKeeperSvc(const std::string& name, ISvcLocator* svc);
  
  virtual ~LSFTimeKeeperSvc();

private:
  LSF::JobInfo*    m_lsfJobInfo;   
};

/// \name stream status
//@{
class MsgStream;
MsgStream& operator << (MsgStream&, const TimeKeeper&);
#include <iostream>
std::ostream& operator << (std::ostream&, const TimeKeeper&);
//@}

#endif


