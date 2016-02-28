///////////////////////// -*- C++ -*- /////////////////////////////
// AthenaRootBranchAddress.h 
// Header file for class Athena::RootBranchAddress
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_ATHENAROOTBRANCHADDRESS_H
#define ATHENAROOTCOMPS_ATHENAROOTBRANCHADDRESS_H 1

// STL includes
#include <string>

// Gaudi includes
#include "GaudiKernel/GenericAddress.h"

// ROOT
#include "Reflex/Type.h"
#include "Reflex/Object.h"

// Forward declaration
class TTree;

namespace Athena {

  /** @class Athena::RootBranchAddress
   *  A simple class to hold the buffer of a @c TBranch from a @c TTree
   */
class RootBranchAddress 
  : public GenericAddress
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Default constructor: 
  RootBranchAddress();

  /// Copy constructor: 
  RootBranchAddress( const RootBranchAddress& rhs );

  /// Assignment operator: 
  RootBranchAddress& operator=( const RootBranchAddress& rhs ); 

  /// Constructor with parameters: 
  RootBranchAddress(long svc,
                    const CLID& clid,
                    const std::string& p1="",
                    const std::string& p2="",
                    unsigned long ip1=0,
                    unsigned long ip2=0);

  /// Destructor: 
  virtual ~RootBranchAddress(); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// setup the ROOT @c TTree internal address for the branch
  /// @param t additional Reflex::Type hint (we check the consistency)
  void
  setBranchAddress(const Reflex::Type& t);

  /// the @c TTree whose branch we proxy
  TTree *ttree();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
  //private: 

  /// the buffer for the @c TBranch
  Reflex::Object m_ptr;

  /// the last ttree number loaded: keeps track of the ttree which
  /// was bound to this address (in case of TChains, the file hosting
  /// the data might change, so we need to re-bind the data)
  int m_tree_nbr;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 
//std::ostream& operator<<( std::ostream& out, const RootBranchAddress& o );

} //> end namespace Athena

#endif //> !ATHENAROOTCOMPS_ROOTBRANCHADDRESS_H
