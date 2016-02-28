//DR #include "time_lsf.h"

#include "LSFJobInfo.h"

using namespace LSF;

JobInfoError::JobInfoError(int /*errcode*/) throw() {
  //FIXME  const char* errMsg = time_lsf_error[-errcode];
  const char* errMsg = "LSF API ERROR"; //FIXME
  m_what = errMsg;
}


JobInfo::JobInfo() : 
  m_isLSFJob(!(LSF::JobInfo::timeLWrapper()==-1)),
  m_allocTime(m_isLSFJob ? static_cast<unsigned int>(LSF::JobInfo::timeLWrapper()) : 99999999) //FIXME
{}

unsigned int 
JobInfo::timeL() throw(JobInfoError) {
  int tL(static_cast<int>(LSF::JobInfo::timeLWrapper()));
  if (tL < 0) throw JobInfoError(tL);
  return tL;
}
