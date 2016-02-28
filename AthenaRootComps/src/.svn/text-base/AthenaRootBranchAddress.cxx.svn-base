///////////////////////// -*- C++ -*- /////////////////////////////
// AthenaRootBranchAddress.cxx 
// Implementation file for class RootBranchAddress
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthenaRootComps includes
#include "AthenaRootBranchAddress.h"

// STL includes

// fwk includes
#include "GaudiKernel/System.h"
//#include "GaudiKernel/ClassID.h"

// ROOT includes
#include "TBranch.h"
#include "TTree.h"
#include "TLeaf.h"

namespace Athena {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

/// Default constructor: 
RootBranchAddress::RootBranchAddress() :
  GenericAddress(),
  m_ptr(),
  m_tree_nbr(-1)
{}

/// Copy constructor: 
RootBranchAddress::RootBranchAddress( const RootBranchAddress& rhs ) :
  GenericAddress(rhs),
  m_ptr(rhs.m_ptr),
  m_tree_nbr(rhs.m_tree_nbr)
{}

/// Assignment operator: 
RootBranchAddress& 
RootBranchAddress::operator=( const RootBranchAddress& rhs )
{
  if (this != &rhs) {
    GenericAddress::operator=(rhs);
    m_ptr = rhs.m_ptr;
    m_tree_nbr = rhs.m_tree_nbr;
  }
  return *this;
}

/// Constructor with parameters: 
RootBranchAddress::RootBranchAddress(long svc,
                                     const CLID& clid,
                                     const std::string& p1,
                                     const std::string& p2,
                                     unsigned long ip1,
                                     unsigned long ip2) :
  GenericAddress(svc, clid, p1, p2, ip1, ip2),
  m_ptr(),
  m_tree_nbr(-1)
{}

/// Destructor: 
RootBranchAddress::~RootBranchAddress()
{
}


/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

void
RootBranchAddress::setBranchAddress(const Reflex::Type& rflx_type) 
{
  const std::string& br_name = this->par()[1];
  //const unsigned long* ipar = this->ipar();
  TTree *tree = this->ttree();
  Long64_t ientry = tree->GetTree()->GetReadEntry();

  int tree_nbr = tree->GetTreeNumber();
  if (m_ptr.Address() != 0) {
    // check if a new file was loaded...
    if ( m_tree_nbr == -1 || tree_nbr != m_tree_nbr ) {
      // update needed...
      m_tree_nbr = tree_nbr;
    } else {
      return;
    }
  }

  // activate branch...
  tree->SetBranchStatus(br_name.c_str(), 1);

  // --- inspired from pyroot/src/Pythonize.cxx ---
  // search for branch first (typical for objects)
  TBranch *branch = tree->GetBranch(br_name.c_str());
  if (!branch) {
    //ATH_MSG_INFO("-- branch [" << br_name << "] =0 -> trying a subbranch");
    // for benefit of naming of sub-branches, 
    // the actual name may have a trailing '.'
    branch = tree->GetBranch((br_name+".").c_str());
  }

  if (branch) {
    if (branch->GetReadEntry() != ientry || ientry < 0) {
      branch->GetEntry(ientry);
    }

    //ATH_MSG_INFO("-- branch [" << br_name << "] = OK");
    // found a branched object, wrap its address for the object it represents
    TClass *cls = TClass::GetClass(branch->GetClassName());
    if (cls) {
      if (!branch->GetAddress()) {
        branch->GetEntry(ientry);
      }
      if (branch->GetAddress()) {
        //ATH_MSG_INFO("-- branch [" << br_name << "] cls: [" << cls->GetName() << "]");
        void* addr = *(char**)branch->GetAddress();
        Reflex::Type t = Reflex::Type::ByTypeInfo(*cls->GetTypeInfo());
        if (t != rflx_type && !t.Id()) {
            t = rflx_type;
        }
        m_ptr = Reflex::Object(t, addr);
        return;

      } else {
        //ATH_MSG_INFO("-- branch [" << br_name << "] cls: [" << cls << "]");
      }
    } else {
      //ATH_MSG_INFO("-- branch [" << br_name << "] cls: [" << cls << "]");
    }

  }

  // if not, try a leaf
  TLeaf *leaf = tree->GetLeaf(br_name.c_str());
  //ATH_MSG_INFO("-- leaf [" << br_name << "] ??");
  if (branch && !leaf) {
    leaf = branch->GetLeaf(br_name.c_str());
    if (!leaf) {
      TObjArray *leaves = branch->GetListOfLeaves();
      if (leaves->GetSize() && (leaves->First() == leaves->Last())) {
        // i.e., if unambiguously only this one
        leaf = (TLeaf*)leaves->At(0);
      }
    }
  }

  if (leaf) {
    //ATH_MSG_INFO("-- leaf [" << br_name << "] = OK");
    // found a leaf, extract value and wrap
    if ( 1 < leaf->GetLenStatic() || leaf->GetLeafCount() ) {
      // array types --- FIXME
      //ATH_MSG_INFO("-- leaf [" << br_name << "] => array...");
      std::string tname = leaf->GetTypeName();
      void *addr = (void*)leaf->GetValuePointer();
      Reflex::Type t = Reflex::Type::ByName(tname+"*");
      if (t != rflx_type && !t.Id()) {
        t = rflx_type;
      }
      m_ptr = Reflex::Object(t, &addr);
      return;

    } else {
      // value types
      std::string tname = leaf->GetTypeName();
      // std::cerr << "-- leaf [" << br_name << "] => value-type("
      //           << tname << ")...\n";
      void *addr = (void*)leaf->GetValuePointer();
      Reflex::Type t = Reflex::Type::ByName(tname);
      if (t != rflx_type && !t.Id()) {
        t = rflx_type;
      }
      m_ptr = Reflex::Object(t, addr);
      // std::cerr << "-->ptr: [" << m_ptr.TypeOf().Name() << "]\n";
    }
  }

}

/// the @c TTree whose branch we proxy
TTree*
RootBranchAddress::ttree()
{
  return reinterpret_cast<TTree*>(reinterpret_cast<unsigned long*>(this->ipar()[0]));  
}


/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

} //> end namespace Athena
