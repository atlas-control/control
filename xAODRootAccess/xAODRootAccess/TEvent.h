// Dear emacs, this is -*- c++ -*-
// $Id: TEvent.h 725531 2016-02-22 16:14:25Z krasznaa $
#ifndef XAODROOTACCESS_TEVENT_H
#define XAODROOTACCESS_TEVENT_H

// STL include(s):
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

// ROOT include(s):
#include <Rtypes.h>

// EDM include(s):
#include "xAODEventFormat/EventFormat.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"

// Interface include(s):
#include "xAODRootAccessInterfaces/TVirtualEvent.h"

// Local include(s):
#include "xAODRootAccess/tools/TReturnCode.h"

// Forward declaration(s):
class TFile;
class TChain;
class TTree;
namespace std {
   class type_info;
}
namespace SG {
   class IAuxStore;
}

namespace xAOD {

   // Forward declaration(s):
   class TVirtualManager;
   class TVirtualIncidentListener;
   class TObjectManager;
   class TAuxStore;
   class TChainStateTracker;
   class TEventBranch;
   class TMetaBranch;
   class TFileMerger;
   class TEvent;
   ::TTree* MakeTransientTree( TEvent&, const char* );

   /// @short Tool for accessing xAOD files outside of Athena
   ///
   /// Proper access to xAOD files in ROOT (outside of Athena) needs to
   /// be done through such an object. It takes care of reading and
   /// writing xAOD files together with their file format metadata,
   /// setting up smart pointers correctly, etc.
   ///
   /// For a detailed description of the usage of this class, see:
   ///   <Link to be added here...>
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 725531 $
   /// $Date: 2016-02-22 17:14:25 +0100 (Mon, 22 Feb 2016) $
   ///
   class TEvent : public TVirtualEvent {

      // Declare the friend functions/classes:
      friend ::TTree* MakeTransientTree( TEvent&, const char* );
      friend class xAOD::TEventBranch;
      friend class xAOD::TMetaBranch;
      friend class xAOD::TFileMerger;

   public:
      /// Auxiliary store "mode"
      enum EAuxMode {
         kBranchAccess = 0, ///< Access auxiliary data branch-by-branch
         kClassAccess  = 1, ///< Access auxiliary data using the aux containers
         kAthenaAccess = 2  ///< Access containers/objects like Athena does
      };

      /// Default constructor
      TEvent( EAuxMode mode = kClassAccess );
      /// Constructor connecting the object to an input TFile
      TEvent( ::TFile* file, EAuxMode mode = kClassAccess );
      /// Constructor connecting the objects to an input TTree/TChain
      TEvent( ::TTree* tree, EAuxMode mode = kClassAccess );
      /// Destructor
      ~TEvent();

      /// Get what auxiliary access mode the object was constructed with
      EAuxMode auxMode() const;

      /// Function creating a user-readable dump of the current input
      std::string dump();

      /// Function printing the I/O statistics of the current process
      void printIOStats() const;

      /// @name Setup functions
      /// @{

      /// Connect the object to a new input file
      TReturnCode readFrom( ::TFile* file, Bool_t useTreeCache = kTRUE,
                            const char* treeName = EVENT_TREE_NAME );
      /// Connect the object to a new input tree/chain
      TReturnCode readFrom( ::TTree* tree, Bool_t useTreeCache = kTRUE );
      /// Connect the object to an output file
      TReturnCode writeTo( ::TFile* file, Int_t autoFlush = 200,
                           const char* treeName = EVENT_TREE_NAME );
      /// Finish writing to an output file
      TReturnCode finishWritingTo( ::TFile* file );

      /// Set this event object as the currently active one
      void setActive() const;

      /// Configure which dynamic variables to write out for a given store
      void setAuxItemList( const std::string& containerKey,
                           const std::string& itemList );

      /// Register an incident listener object
      TReturnCode addListener( TVirtualIncidentListener* listener );
      /// Remove an incident listener object
      TReturnCode removeListener( TVirtualIncidentListener* listener );
      /// Remove all listeners from the object
      void clearListeners();

      /// Add a name re-mapping rule
      TReturnCode addNameRemap( const std::string& onfile,
                                const std::string& newName );
      /// Clear the current name re-mapping
      void clearNameRemap();
      /// Print the current name re-mapping rules
      void printNameRemap() const;

      /// @}

      /// @name Event data accessor/modifier functions
      /// @{

      /// Function checking if an object is available from the store
      template< typename T >
      ::Bool_t contains( const std::string& key );
      /// Function checking if an object is already in memory
      template< typename T >
      ::Bool_t transientContains( const std::string& key ) const;

      /// Retrieve either an input or an output object from the event
      template< typename T >
      TReturnCode retrieve( const T*& obj, const std::string& key );
      /// Retrieve an output object from the event
      template< typename T >
      TReturnCode retrieve( T*& obj, const std::string& key );

      /// Add an output object to the event
      template< typename T >
      TReturnCode record( T* obj, const std::string& key,
                          ::Int_t basketSize = 32000, ::Int_t splitLevel = 0 );
      /// Add an output object to the event, explicitly taking ownership of it
      template< typename T >
      TReturnCode record( std::unique_ptr< T > obj, const std::string& key,
                          ::Int_t basketSize = 32000, ::Int_t splitLevel = 0 );

      /// Add an auxiliary store object to the output
      SG::IAuxStore* recordAux( const std::string& key,
                                SG::IAuxStoreHolder::AuxStoreType type =
                                SG::IAuxStoreHolder::AST_ContainerStore,
                                Int_t basketSize = 32000,
                                Int_t splitLevel = 0 );

      /// Copy an object directly from the input to the output
      TReturnCode copy( const std::string& key,
                        ::Int_t basketSize = 32000, ::Int_t splitLevel = 0 );
      /// Copy all (ROOT readable) objects directly from the input to the output
      TReturnCode copy( ::Int_t basketSize = 32000, ::Int_t splitLevel = 0 );

      /// @}

      /// @name Metadata accessor/modifier functions
      /// @{

      /// Function checking if a meta-object is available from the store
      template< typename T >
      ::Bool_t containsMeta( const std::string& key );
      /// Function checking if a meta-object is already in memory
      template< typename T >
      ::Bool_t transientContainsMeta( const std::string& key ) const;

      /// Retrieve an input metadata object
      template< typename T >
      TReturnCode retrieveMetaInput( const T*& obj, const std::string& key );

      /// Retrieve an output metadata object
      template< typename T >
      TReturnCode retrieveMetaOutput( const T*& obj, const std::string& key );
      /// Retrieve an output metadata object
      template< typename T >
      TReturnCode retrieveMetaOutput( T*& obj, const std::string& key );

      /// Add an object to the output file's metadata
      template< typename T >
      TReturnCode recordMeta( T* obj, const std::string& key,
                              ::Int_t basketSize = 32000,
                              ::Int_t splitLevel = 1 );
      /// Add an object to the output file's metadata, explicitly taking
      /// ownership of it
      template< typename T >
      TReturnCode recordMeta( std::unique_ptr< T > obj, const std::string& key,
                              ::Int_t basketSize = 32000,
                              ::Int_t splitLevel = 1 );

      /// @}

      /// @name Persistent data accessor/modifier functions
      /// @{

      /// Get how many entries are available from the current input file(s)
      ::Long64_t getEntries() const;
      /// Function loading a given entry of the input TTree
      ::Int_t getEntry( ::Long64_t entry, ::Int_t getall = 0 );

      /// Get how many files are available on the currently defined input
      ::Long64_t getFiles() const;
      /// Load the first event for a given file from the input TChain
      ::Int_t getFile( ::Long64_t file, ::Int_t getall = 0 );

      /// Function filling one event into the output tree
      ::Int_t fill();

      /// Get information about the input objects
      const EventFormat* inputEventFormat() const;
      /// Get information about the output objects
      const EventFormat* outputEventFormat() const;

      /// @}

      /// @name Functions implementing the TVirtualEvent interface
      /// @{

      /// Function returning the hash describing an object name
      virtual uint32_t getHash( const std::string& key ) const;
      /// Function returning the hash describing a known object
      virtual uint32_t getKey( const void* obj ) const;
      /// Function returning the key describing a known object
      virtual const std::string& getName( const void* obj ) const;
      /// Function returning the key describing a known object
      virtual const std::string& getName( uint32_t hash ) const;

   protected:
      /// Function for retrieving an output object in a non-template way
      virtual void* getOutputObject( uint32_t key,
                                     const std::type_info& ti );
      /// Function for retrieving an input object in a non-template way
      virtual const void* getInputObject( uint32_t key,
                                          const std::type_info& ti,
                                          bool silent = false );

      /// @}

      /// Function to initialise the statistics for all Tree content
      TReturnCode initStats();
      /// Function for retrieving an output object in a non-template way
      void* getOutputObject( const std::string& key,
                             const std::type_info& ti,
                             ::Bool_t metadata = kFALSE ) const;
      /// Function for retrieving an input object in a non-template way
      const void* getInputObject( const std::string& key,
                                  const std::type_info& ti,
                                  ::Bool_t silent = kFALSE,
                                  ::Bool_t metadata = kFALSE );
      /// Internal function for recording an object into the output
      TReturnCode record( void* obj, const std::string& typeName,
                          const std::string& key,
                          ::Int_t basketSize, ::Int_t splitLevel,
                          ::Bool_t overwrite = kFALSE,
                          ::Bool_t metadata = kFALSE,
                          ::Bool_t isOwner = kTRUE );
      /// Internal function for adding an auxiliary store object to the output
      TReturnCode record( TAuxStore* store, const std::string& key,
                          ::Int_t basketSize, ::Int_t splitLevel,
                          ::Bool_t ownsStore = kFALSE );
      /// Function setting up access to a particular branch
      TReturnCode connectBranch( const std::string& key,
                                 ::Bool_t silent = kFALSE );
      /// Function setting up access to a branch in the metadata tree
      TReturnCode connectMetaBranch( const std::string& key,
                                     ::Bool_t silent = kFALSE );
      /// Function setting up access to a set of auxiliary branches
      TReturnCode connectAux( const std::string& prefix, ::Bool_t standalone );
      /// Function setting up access to a set of auxiliary branches for a
      /// metadata object
      TReturnCode connectMetaAux( const std::string& prefix,
                                  ::Bool_t standalone );
      /// Function adding dynamic variable reading capabilities to an auxiliary
      /// store object
      TReturnCode setUpDynamicStore( TObjectManager& mgr, ::TTree* tree );
      /// Function connecting a DV object to its auxiliary store
      TReturnCode setAuxStore( TObjectManager& mgr,
                               ::Bool_t metadata = kFALSE );
      /// Function saving the dynamically created auxiliary properties
      TReturnCode putAux( ::TTree& outTree, TVirtualManager& mgr,
                          ::Int_t basketSize = 32000, ::Int_t splitLevel = 0,
                          ::Bool_t metadata = kFALSE );
      /// Function checking if a given object may have an auxiliary store
      static ::Bool_t hasAuxStore( const TObjectManager& mgr );
      /// Function checking if a given object may be an auxiliary store
      static ::Bool_t isAuxStore( const TObjectManager& mgr );
      /// Function checking if an object is standalone (not a container)
      static ::Bool_t isStandalone( const TObjectManager& mgr );
      /// Internal function checking if an object is in the input
      ::Bool_t contains( const std::string& key,
                         const std::type_info& ti,
                         ::Bool_t metadata = kFALSE );
      /// Internal function checking if an object is already in memory
      ::Bool_t transientContains( const std::string& key,
                                  const std::type_info& ti,
                                  ::Bool_t metadata = kFALSE ) const;

      /// Definition of the internal data structure type
      typedef std::map< std::string,
                        TVirtualManager* > Object_t;
      /// Definition of the structure type holding on to listeners
      typedef std::vector< TVirtualIncidentListener* > Listener_t;

      /// Size of a possible TTreeCache (30 MB)
      static const ::Int_t CACHE_SIZE;
      /// Name of the event tree
      static const char* EVENT_TREE_NAME;
      /// Name of the metadata tree
      static const char* METADATA_TREE_NAME;

      /// The auxiliary access mode
      EAuxMode m_auxMode;

      /// The tree that we are reading from
      ::TTree* m_inTree;
      /// Internal status flag showing that an input file is open, but it
      /// doesn't contain an event tree
      ::Bool_t m_inTreeMissing;
      /// The (optional) chain provided as input
      ::TChain* m_inChain;
      /// Optional object for tracking the state changes of an input TChain
      TChainStateTracker* m_inChainTracker;
      /// The number of the currently open tree in the input chain
      ::Int_t m_inTreeNumber;
      /// Pointer to the metadata tree in the input file
      ::TTree* m_inMetaTree;
      /// The entry to look at from the input tree
      ::Long64_t m_entry;

      /// The tree that we are writing to
      ::TTree* m_outTree;

      /// Collection of all the managed input objects
      Object_t m_inputObjects;
      /// Collection of all the managed output object
      Object_t m_outputObjects;

      /// Collection of all the managed input meta-objects
      Object_t m_inputMetaObjects;
      /// Collection of all the managed output meta-objects
      Object_t m_outputMetaObjects;

      /// Format of the current input file
      EventFormat m_inputEventFormat;
      /// Format of the current output file
      EventFormat* m_outputEventFormat;

      /// Rules for selecting which auxiliary branches to write
      std::map< std::string, std::set< std::string > > m_auxItemList;

      /// Listeners who should be notified when certain incidents happen
      Listener_t m_listeners;

      /// Container name re-mapping rules
      std::map< std::string, std::string > m_nameRemapping;

   }; // class TEvent

} // namespace xAOD

// Include the template implementation(s):
#ifndef __GCCXML__
#   include "TEvent.icc"
#endif // __GCCXML__

#endif // XAODROOTACCESS_TEVENT_H
