///////////////////////// -*- C++ -*- /////////////////////////////
// AresEventSelector.h 
// Header file for class AresEventSelector
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_ARESEVENTSELECTOR_H 
#define ATHENAROOTCOMPS_ARESEVENTSELECTOR_H 

// STL includes

// framework includes
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/Property.h"
#include "AthenaKernel/IEventSeek.h"

// Forward declaration
class ISvcLocator;
class ITHistSvc;
class StoreGateSvc;
class TTree;

/** @brief Class implementing the GAUDI @c IEvtSelector interface using 
 *         ROOT @c TTree as a backend
 */
class AresEventSelector : virtual public IEvtSelector,
			  virtual public IEventSeek,
		          virtual public AthService
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Constructor with parameters: 
  AresEventSelector( const std::string& name, ISvcLocator* svcLoc );

  /// Destructor: 
  virtual ~AresEventSelector(); 

  // Athena hooks
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode queryInterface( const InterfaceID& riid, 
				     void** ppvInterface );
  
  ///@{
  /// @c IEvtSelector interface
  virtual StatusCode createContext( Context*& refpCtxt ) const;

  virtual StatusCode last( Context& refContext ) const;
  virtual StatusCode next( Context& refCtxt ) const;
  virtual StatusCode next( Context& refCtxt, int jump ) const;
  virtual StatusCode previous( Context& refCtxt ) const;
  virtual StatusCode previous( Context& refCtxt, int jump ) const;
  virtual StatusCode rewind( Context& refCtxt ) const;

  virtual StatusCode createAddress( const Context& refCtxt, 
				    IOpaqueAddress*& ) const;
  virtual StatusCode releaseContext( Context*& refCtxt ) const;
  virtual StatusCode resetCriteria( const std::string& cr, 
				    Context& ctx )const;
  ///@}

  ///@{
  /// @c IEventSeek interface
  /**
   * @brief Seek to a given event number.
   * @param evtnum  The event number to which to seek.
   */
  virtual StatusCode seek (int evtnum);

  /**
   * @brief return the current event number.
   * @return The current event number.
   */
  virtual int curEvent () const;
  ///@}
  
  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Private methods: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// callback to synchronize the list of input files
  void setupInputCollection( Property& inputCollectionsName );

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  /// Pointer to the @c StoreGateSvc event store
  StoreGateSvc_t m_storeGate;

  typedef ServiceHandle<ITHistSvc> ITHistSvc_t;
  /// Pointer to the @c ITHistSvc containing the @c TTree 
  ITHistSvc_t m_tupleSvc;

  /// List of input files containing @c TTree 
  StringArrayProperty m_inputCollectionsName;

  /// Name of @c TTree to load from collection of input files
  StringProperty m_tupleName;

  /// List of branches to activate in the @c TTree 
  StringArrayProperty m_activeBranchNames;

  /// Number of events to skip at the beginning 
  long m_skipEvts;

  /// Number of Events read so far.
  mutable long m_nbrEvts;

  /// Total number of events
  long m_totalNbrEvts;

  /// The ARA-transient @c TTree we are reading
  TTree* m_tuple;

  /** The (python) selection function to apply on the @c TChain we are reading
   */
  //PyObject* m_pySelectionFct;

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

#endif //> ATHENAROOTCOMPS_ARESEVENTSELECTOR_H
