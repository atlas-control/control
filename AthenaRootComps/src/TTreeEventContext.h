///////////////////////// -*- C++ -*- /////////////////////////////
// TTreeEventContext.h 
// Header file for class TTreeEventContext
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_TTREEEVENTCONTEXT_H 
#define ATHENAROOTCOMPS_TTREEEVENTCONTEXT_H 

// STL includes
#include <string>

// Gaudi includes
#include "GaudiKernel/IEvtSelector.h"

// Forward declaration
class IOpaqueAddress;

class TTreeEventContext : public IEvtSelector::Context
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Constructor with parameters: 
  TTreeEventContext( const IEvtSelector* selector );

  /// copy constructor
  TTreeEventContext( const TTreeEventContext& ctxt );

  /// Destructor: 
  virtual ~TTreeEventContext(); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  virtual void* identifier() const;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  const IEvtSelector* m_selector;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline void* TTreeEventContext::identifier() const
{ return (void*)m_selector; }

#endif //> ATHENAROOTCOMPS_TTREEEVENTCONTEXT_H
