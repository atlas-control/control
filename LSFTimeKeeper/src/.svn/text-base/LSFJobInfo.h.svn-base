#ifndef LSFJOBINFO_H
# define LSFJOBINFO_H

//<<<<<< INCLUDES                                                       >>>>>>
#include <exception>
 extern "C" {
   extern void timel_( float &);
 }

//<<<<<< CLASS DECLARATIONS                                             >>>>>>

namespace LSF {
  /** @class JobInfoError
   * @brief exception thrown by JobInfo
   * @author
   * $Id: LSFJobInfo.h,v 1.2 2004-09-23 22:53:23 droussea Exp $
   */
  class JobInfoError : std::exception {
  public:
    JobInfoError(int errcode=-1) throw()  ;
    JobInfoError(const JobInfoError& rhs) throw() : 
      std::exception(rhs), m_what(rhs.m_what) {}
    virtual ~JobInfoError() throw() {}

    virtual const char* what() const throw() { return m_what; }
  private:
    const char* m_what;
  };


  /** @class JobInfo
   * @brief access  job info. In particular allows to know if an executable 
   *        runs as an LSF  job and when job time is over
   *        N.B. times are in LSF units
   * 
   * @author
   * $Id: LSFJobInfo.h,v 1.2 2004-09-23 22:53:23 droussea Exp $
   */
  class JobInfo {
  public:
    JobInfo();
    bool isLSFJob() const { return m_isLSFJob; }
    static float timeLWrapper()  
      {
	float theTime ; 
	timel_(theTime);  // cernlib
	// theTime=time_lsf();  // lsf version proven unreliable
        return theTime;
	
      }
    static unsigned int timeL() throw(JobInfoError);  ///< job time left 
    unsigned int allocTime() { return m_allocTime; }  //FIXME 
    unsigned int timeX() throw(JobInfoError) { return allocTime() - JobInfo::timeL();}

    //NOT YET const char* queue() const;
    //NOT YET const char* name() const;
    //NOT YET const char* host() const;

  private:
    bool m_isLSFJob;  
    unsigned int m_allocTime;
  };
} //end namespace LSF

#endif 
