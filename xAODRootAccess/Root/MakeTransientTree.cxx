// $Id: MakeTransientTree.cxx 687837 2015-08-06 08:57:44Z krasznaa $

// System include(s):
#include <cstring>
#include <iostream>

// ROOT include(s):
#include <TFile.h>
#include <TChain.h>
#include <TROOT.h>
#include <TError.h>
#include <TClass.h>
#include <TDirectory.h>

// Local include(s):
#include "xAODRootAccess/MakeTransientTree.h"
#include "xAODRootAccess/tools/TEventTree.h"
#include "xAODRootAccess/tools/TEventBranch.h"
#include "xAODRootAccess/tools/TMetaTree.h"
#include "xAODRootAccess/tools/TMetaBranch.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/TEvent.h"

namespace {

   /// Helper class for making sure the xAOD::TEvent object is always up to date
   ///
   /// Since the TEvent object needs to access the object inside the TFile
   /// managed by an input TChain, it can quite easily happen that the TChain
   /// for some reason closes the current input file without TEvent knowing
   /// about this.
   ///
   /// Luckily all TTree-s can notify users about updates in their status using
   /// the TObject::Notify() function. Since TEvent doesn't inherit from
   /// TObject, we need to use this small adaptor class here. It sends the
   /// notification signal from the TChain to the TEvent object.
   ///
   /// Since it's not a public class, and we don't need a dictionary for it,
   /// it's fine that we don't use ClassDef/ClassImp for it.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 687837 $
   /// $Date: 2015-08-06 10:57:44 +0200 (Thu, 06 Aug 2015) $
   ///
   class TEventNotifier : public ::TObject {

   public:
      /// Default constructor
      TEventNotifier()
         : m_tree( 0 ), m_event( 0 ) {}
      /// Constructor with all necessary inputs
      TEventNotifier( ::TTree* tree, xAOD::TEvent* event )
         : m_tree( tree ), m_event( event ) {}

      /// Function giving new objects to this one
      void Setup( ::TTree* tree, xAOD::TEvent* event ) {
         m_tree = tree;
         m_event = event;
         return;
      }

      /// Function called by the tree/chain when a new tree is loaded
      virtual ::Bool_t Notify() {
         if( ( ! m_tree ) || ( ! m_event ) ) {
            ::Error( "TEventNotifier::Notify",
                     XAOD_MESSAGE( "Function called on uninitialised "
                                   "object" ) );
            return kFALSE;
         }
         if( ! m_event->readFrom( m_tree ) ) {
            ::Error( "TEventNotifier::Notify",
                     XAOD_MESSAGE( "Couldn't notify TEvent object of TTree "
                                   "update" ) );
            return kFALSE;
         }
         return kTRUE;
      }

   private:
      /// TTree/TChain that we are reading from
      ::TTree* m_tree;
      /// TEvent object that reads from this tree/chain
      xAOD::TEvent* m_event;

   }; // TEventNotifier

} // private namespace

namespace xAOD {

   /// Pointer to the event TTree object managed by this code
   static TEventTree* s_eventTree = 0;
   /// Pointer to the meta TTree object managed by this code
   static TMetaTree* s_metaTree = 0;

   /// Pointer to the helper TChain used when reading metadata using a TChain
   static ::TChain* s_helperChain = 0;

   //
   // Forward declare the private functions:
   //
   ::TTree* MakeTransientTree( TEvent& event, const char* treeName );
   ::TTree* MakeTransientMetaTree( TEvent& event, ::TTree* persMetaTree );

   TTransTrees MakeTransientTrees( ::TFile* ifile,
                                   const char* treeName,
                                   TEvent::EAuxMode mode ) {

      // Create a TEvent object that will take care of retrieving objects
      // from the persistent trees:
      static TEvent s_event( mode );
      if( ! s_event.readFrom( ifile, kTRUE, treeName ) ) {
         ::Error( "xAOD::MakeTransientTrees",
                 XAOD_MESSAGE( "Couldn't set up the reading of the input "
                               "file" ) );
         return TTransTrees( 0, 0 );
      }

      // Access the metadata tree of the file:
      ::TTree* metaTree = dynamic_cast< ::TTree* >( ifile->Get( "MetaData" ) );
      if( ! metaTree ) {
         ::Error( "xAOD::MakeTransientTrees",
                  XAOD_MESSAGE( "Couldn't access metadata tree \"MetaData\" "
                                "in the input file" ) );
         return TTransTrees( 0, 0 );
      }

      // Create the transient tree using this TEvent object:
      return TTransTrees( MakeTransientTree( s_event, treeName ),
                          MakeTransientMetaTree( s_event, metaTree ) );
   }

   void ClearTransientTrees() {

      if( s_eventTree ) {
         delete s_eventTree;
         s_eventTree = 0;
      }
      if( s_metaTree ) {
         delete s_metaTree;
         s_metaTree = 0;
      }
      if( s_helperChain ) {
         delete s_helperChain;
         s_helperChain = 0;
      }
      return;
   }

   ::TTree* MakeTransientTree( TEvent& event, const char* treeName ) {

      // Get the input file's format:
      const EventFormat* ef = event.inputEventFormat();
      if( ! ef ) {
         ::Error( "xAOD::MakeTransientTree",
                  XAOD_MESSAGE( "Couldn't get the input file's format "
                                "metadata" ) );
         return 0;
      }

      // Remember which directory we are in now:
      ::TDirectory* dir = gDirectory;

      // Create the transient tree:
      if( s_eventTree ) {
         delete s_eventTree;
         s_eventTree = 0;
      }
      gROOT->cd();
      s_eventTree = new TEventTree( &event, treeName );

      // Go back to the original directory:
      dir->cd();

      // It can happen with a TChain input that at this point the TEvent
      // object doesn't actually have the correct file open. This should address
      // the issue:
      if( ( ! event.m_inTree ) && ( event.getEntry( 0 ) < 0 ) ) {
         ::Error( "xAOD::MakeTransientTree",
                  XAOD_MESSAGE( "Internal logic error detected" ) );
         return 0;
      }

      // Create a branch for each xAOD interface object:
      EventFormat::const_iterator ef_itr = ef->begin();
      EventFormat::const_iterator ef_end = ef->end();
      for( ; ef_itr != ef_end; ++ef_itr ) {

         // Convenience reference:
         const xAOD::EventFormatElement& efe = ef_itr->second;

         // Just skip all branches ending in "Aux.":
         if( efe.branchName().find( "Aux." ) ==
             ( efe.branchName().size() - 4 ) ) {
            continue;
         }
         // Also skip dynamic branches:
         if( efe.parentName() != "" ) {
            continue;
         }

         // Don't bother if the branch is not available:
         if( ! event.m_inTree->GetBranch( efe.branchName().c_str() ) ) {
            continue;
         }

         // Check if the class in question is known:
         ::TClass* cl = ::TClass::GetClass( efe.className().c_str() );
         if( ! cl ) continue;

         // Check if we have a type-info for this class:
         const std::type_info* ti = cl->GetTypeInfo();
         if( ! ti ) {
            ::Warning( "xAOD::MakeTransientTree",
                       "Couldn't find std::type_info object for type %s "
                       "(key=%s)", cl->GetName(), efe.branchName().c_str() );
            continue;
         }

         // Check if the object is actually available on the input:
         if( ! event.contains( efe.branchName(), *ti ) ) {
            continue;
         }

         // Don't add auxiliary store objects:
         if( cl->InheritsFrom( "SG::IConstAuxStore" ) ) {
            continue;
         }

         // If everything is right, let's make the branch:
         TEventBranch* br = new TEventBranch( s_eventTree, &event, ti,
                                              efe.branchName().c_str(),
                                              efe.className().c_str() );
         s_eventTree->AddBranch( br );

      }

      // Return the object that was just created:
      ::Info( "xAOD::MakeTransientTree",
              "Created transient tree \"%s\" in ROOT's common memory",
              treeName );
      return s_eventTree;
   }

   ::TTree* MakeTransientTree( ::TFile* ifile, const char* treeName,
                               TEvent::EAuxMode mode ) {

      // Create a TEvent object that will take care of retrieving objects
      // from the persistent tree:
      static TEvent s_event( mode );
      if( ! s_event.readFrom( ifile, kTRUE, treeName ) ) {
         ::Error( "xAOD::MakeTransientTree",
                  XAOD_MESSAGE( "Couldn't set up the reading of the input "
                                "file" ) );
         return 0;
      }

      // Create the transient tree using this TEvent object:
      return MakeTransientTree( s_event, treeName );
   }

   ::TTree* MakeTransientTree( ::TChain* itree, TEvent::EAuxMode mode ) {

      // Static/global variables used by the function:
      static TEvent s_event( mode );
      static ::TEventNotifier notifier;

      // Reset the notifier:
      notifier.Setup( 0, 0 );

      // Set up the TEvent object that will take care of retrieving objects
      // from the persistent tree:
      if( ! s_event.readFrom( itree ) ) {
         ::Error( "xAOD::MakeTransientTree",
                  XAOD_MESSAGE( "Couldn't set up the reading of the input "
                                "tree/chain" ) );
         return 0;
      }

      // Load the first event, to trigger opening the first input file:
      if( s_event.getEntry( 0 ) < 0 ) {
         ::Error( "xAOD::MakeTransientTree",
                  XAOD_MESSAGE( "Couldn't load event 0 from the chain" ) );
         return 0;
      }

      // Set up an object taking care of notifying the TEvent object about
      // changes in the TChain's status:
      notifier.Setup( itree, &s_event );
      itree->SetNotify( &notifier );

      // Create the transient tree using this TEvent object:
      return MakeTransientTree( s_event, itree->GetName() );
   }

   void ClearTransientTree() {

      if( s_eventTree ) {
         delete s_eventTree;
         s_eventTree = 0;
      }
      return;
   }

   ::TTree* MakeTransientMetaTree( TEvent& event, ::TTree* persMetaTree ) {

      // Remember which directory we are in now:
      ::TDirectory* dir = gDirectory;

      // Create the transient metadata tree:
      if( s_metaTree ) {
         delete s_metaTree;
         s_metaTree = 0;
      }
      gROOT->cd();
      s_metaTree = new TMetaTree( &event );

      // Go back to the original directory:
      dir->cd();

      // Loop over the branches of the persistent metadata tree:
      TObjArray* branches = persMetaTree->GetListOfBranches();
      for( ::Int_t i = 0; i < branches->GetEntries(); ++i ) {

         // Access the branch object:
         ::TBranch* br = dynamic_cast< ::TBranch* >( branches->At( i ) );
         if( ! br ) {
            ::Error( "xAOD::MakeTransientMetaTree",
                     XAOD_MESSAGE( "Couldn't access branch %i as a TBranch" ),
                     i );
            continue;
         }

         // Skip the EventFormat branch. That must not be disturbed by the
         // generic metadata handling.
         if( ! strcmp( br->GetName(), "EventFormat" ) ) continue;

         // Check whether we have a dictionary for this type:
         ::TClass* cl = ::TClass::GetClass( br->GetClassName(), kTRUE, kTRUE );
         if( ! cl ) continue;

         // Check if we have a type-info for this class:
         const std::type_info* ti = cl->GetTypeInfo();
         if( ! ti ) continue;

         // Don't add auxiliary store objects:
         if( cl->InheritsFrom( "SG::IConstAuxStore" ) ) {
            continue;
         }

         // If everything is right, let's make the branch:
         TMetaBranch* mbr = new TMetaBranch( s_metaTree, &event, ti,
                                             br->GetName(),
                                             br->GetClassName() );
         s_metaTree->AddBranch( mbr );
      }

      // Return the object that was just created:
      ::Info( "xAOD::MakeTransientMetaTree",
              "Created transient metadata tree \"MetaData\" in ROOT's common "
              "memory");
      return s_metaTree;
   }

   ::TTree* MakeTransientMetaTree( ::TFile* ifile,
                                   const char* eventTreeName,
                                   TEvent::EAuxMode mode ) {

      // Create a TEvent object that will take care of retrieving objects
      // from the persistent tree:
      static TEvent s_event( mode );
      if( ! s_event.readFrom( ifile, kTRUE, eventTreeName ) ) {
         ::Error( "xAOD::MakeTransientMetaTree",
                  XAOD_MESSAGE( "Couldn't set up the reading of the input "
                                "file" ) );
         return 0;
      }

      // Access the metadata tree directly:
      ::TTree* metaTree = dynamic_cast< ::TTree* >( ifile->Get( "MetaData" ) );
      if( ! metaTree ) {
         ::Error( "xAOD::MakeTransientMetaTree",
                  XAOD_MESSAGE( "Couldn't access metadata tree \"MetaData\" "
                                "in the input file" ) );
         return 0;
      }

      // Create the transient tree using this TEvent object:
      return MakeTransientMetaTree( s_event, metaTree );
   }

   ::TTree* MakeTransientMetaTree( ::TChain* ichain,
                                   const char* eventTreeName,
                                   TEvent::EAuxMode mode ) {

      // Construct a helper TChain object to give to TEvent:
      if( s_helperChain ) {
         delete s_helperChain;
      }
      s_helperChain = new ::TChain( eventTreeName );
      ::TObjArray* files = ichain->GetListOfFiles();
      for( ::Int_t i = 0; i < files->GetEntries(); ++i ) {
         s_helperChain->Add( files->At( i )->GetTitle() );
      }

      // Create a TEvent object that will take care of retrieving objects
      // from the persistent tree.
      static TEvent s_event( mode );
      if( ! s_event.readFrom( s_helperChain, kTRUE ) ) {
         ::Error( "xAOD::MakeTransientMetaTree",
                  XAOD_MESSAGE( "Couldn't set up the reading of the input "
                                "TChain" ) );
         return 0;
      }

      // Create the transient tree using this TEvent object:
      return MakeTransientMetaTree( s_event, ichain );
   }

   void ClearTransientMetaTree() {

      if( s_metaTree ) {
         delete s_metaTree;
         s_metaTree = 0;
      }
      if( s_helperChain ) {
         delete s_helperChain;
         s_helperChain = 0;
      }
      return;
   }

} // namespace xAOD
