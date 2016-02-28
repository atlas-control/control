#ifndef ATHENAKERNEL_IOVRANGE_H
 #include "AthenaKernel/IOVRange.h"
#endif
#ifndef GAUDIKERNEL_MSGSTREAM_H
 #include "GaudiKernel/MsgStream.h"
#endif

#include <stdexcept>

/*****************************************************************************
 *
 *  IOVRange.cxx
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *  $Id: IOVRange.cxx,v 1.5 2008-05-08 15:57:15 leggett Exp $
 *
 *  Validity Range object. Holds two IOVTimes (start and stop)
 *
 *****************************************************************************/

IOVRange::IOVRange( const IOVTime& start, const IOVTime& stop ): 
  m_start(start), m_stop(stop) {
}

IOVRange& IOVRange::operator= (const IOVRange& r) {
  if (this != &r) {
    m_start = r.m_start;
    m_stop = r.m_stop;
  }
  return *this;
}

std::ostream& operator << (std::ostream& os, const IOVRange& rhs) {
  os << (std::string) rhs;
  return os;
}

MsgStream& operator<< (MsgStream &msg, const IOVRange& rhs) {
  msg << (std::string) rhs;
  return msg;
}
