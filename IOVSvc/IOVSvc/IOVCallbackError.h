#ifndef IOVSVC_IOVCALLBACKERROR_H
#define IOVSVC_IOVCALLBACKERROR_H 1

#include "GaudiKernel/MsgStream.h"

#include <exception>
#include <string>
#include <iostream>

class IOVCallbackError: virtual public std::exception {

   friend inline std::ostream& operator<< ( std::ostream& os ,
                                            const IOVCallbackError&   ge ) ;
   friend inline std::ostream& operator<< ( std::ostream& os ,
                                            const IOVCallbackError*  pge ) ;   
   friend inline MsgStream&    operator<< ( MsgStream&    os ,
                                            const IOVCallbackError&   ge ) ; 
   friend inline MsgStream&    operator<< ( MsgStream&    os ,
                                            const IOVCallbackError*  pge ) ; 
   
public:
  IOVCallbackError(const std::string& source):
    m_src(source) { }

  virtual ~IOVCallbackError() throw() {}

  virtual const std::string& source() const {
    return m_src;
  }

  virtual std::ostream& printOut( std::ostream& os = std::cerr ) const {
    os << "IOVCallbackError generated by " << source();
    return os;
  }

  virtual MsgStream& printOut( MsgStream& os ) const {
    os << "IOVCallbackError generated by " << source();
    return os;
  }

  /// method from std::exception 
  virtual const char* what () const throw() { return source().c_str() ; }  

private:
  std::string m_src;
};

/// overloaded printout to std::ostream 
std::ostream& operator<< ( std::ostream& os , const IOVCallbackError&   ge ) {
  return ge.printOut( os );
} 
std::ostream& operator<< ( std::ostream& os , const IOVCallbackError*  pge ) 
{ return (0 == pge) ?
    ( os << " IOVCallbackError* points to NULL!" ) : ( os << *pge ); } 

/// overloaded printout to MsgStream  
MsgStream&    operator<< ( MsgStream& os    , const IOVCallbackError&   ge ) { 
  return ge.printOut( os ); 
} 
/// overloaded printout to MsgStream  
MsgStream&    operator<< ( MsgStream& os    , const IOVCallbackError*  pge ) { 
  return (0 == pge) ?
    ( os << " IOVCallbackError* points to NULL!" ) : ( os << *pge );
} 

#endif
