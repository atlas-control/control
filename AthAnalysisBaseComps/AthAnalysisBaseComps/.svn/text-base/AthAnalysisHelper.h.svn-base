///////////////////////// -*- C++ -*- /////////////////////////////
// AthAnalysisHelper.h 
// Header file for class AthAnalysisHelper
// Methods that are useful for athena-based analysis will go here
// Author: W.Buttinger<will@cern.ch>
/////////////////////////////////////////////////////////////////// 

#ifndef ATHANALYSISBASECOMPS_ATHANALYSISHELPER_H
#define ATHANALYSISBASECOMPS_ATHANALYSISHELPER_H 1

#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IToolSvc.h"

#include "IOVDbDataModel/IOVMetaDataContainer.h"

class AthAnalysisHelper { //thought about being a namespace but went for static methods instead, in case I want private data members in future

public:
   ///helper method for setting a property on a tool before retrieving it
   ///uses the toolhandle to work out what to register the joboptionsvc
   ///Usage examples:
   ///  ToolHandle<IMyTool> tool("MyTool/toolName");
   ///  CHECK( AthAnalysisHelper::setProperty( tool, "IntegerProperty", 5) );
   ///  CHECK( AthAnalysisHelper::setProperty( tool, "StringProperty", "hello") );
   ///  CHECK( AthAnalysisHelper::setProperty( tool, "PublicToolHandleProperty", anotherToolHandle.typeAndName()) );
   ///  CHECK( AthAnalysisHelper::setProperty( tool, "PrivateToolHandleName.SubToolIntegerProperty", 4) );
   ///The last example assumes the 'MyTool' class has declared a ToolHandle("SubTool/PrivateToolHandleName",this)
   ///and 'SubTool' class has an integer property called 'SubToolIntegerProperty' declared

   template<typename T, typename W> static StatusCode setProperty(const T& toolHandle, const std::string& property, W&& value) {
      if(toolHandle.isSet()) {
         std::cout << "Cannot setProperty on a tool that is already initialized" << std::endl;
         return StatusCode::FAILURE;
      }
      std::string fullName = toolHandle.parentName() + "." + toolHandle.name();
      std::string thePropertyName(property);
      //if the property contains any "." then strip the last bit as the property name
      std::string::size_type dotLocation = thePropertyName.find_last_of('.');
      if(dotLocation != std::string::npos) {
         fullName += "." + thePropertyName.substr(0,dotLocation);
         thePropertyName = thePropertyName.substr(dotLocation+1,thePropertyName.length()-dotLocation);
      }
      //check if the tool already exists, if so then we are too late!!
      ServiceHandle<IToolSvc> toolSvc("ToolSvc","AthAnalysisHelper");
      if(toolSvc.retrieve().isFailure()) return StatusCode::FAILURE;
      auto existingTools = toolSvc->getInstances();
      for(auto& toolName : existingTools) {
         if(fullName==toolName) {
            std::cout << "Cannot setProperty on a tool that is already initialized" << std::endl;
            return StatusCode::FAILURE;
         }
      }
      if(toolSvc.release().isFailure()) return StatusCode::FAILURE;
      //tool not existing, ok so add property to catalogue
      ServiceHandle<IJobOptionsSvc> joSvc("JobOptionsSvc","AthAnalysisHelper");
      if(joSvc.retrieve().isFailure()) return StatusCode::FAILURE;
      StatusCode result = joSvc->addPropertyToCatalogue(fullName , StringProperty( thePropertyName, Gaudi::Utils::toString ( value ) ) );
      if(joSvc.release().isFailure()) return StatusCode::FAILURE;
      return result;
   }


   
   ///retrieve metadata from the input metadata storegate. Use checkMetaSG.py to see the 'folder' and 'key' values available
   ///always takes the first CondAttrListCollection (aka IOV) and the first channel number present in that IOV
   template<typename T> static StatusCode retrieveMetadata(const std::string& folder, const std::string& key, T& out) throw(std::exception) {
      ServiceHandle<StoreGateSvc> inputMetaStore("StoreGateSvc/InputMetaDataStore", "AthAnalysisHelper");
      if(inputMetaStore.retrieve().isFailure()) return StatusCode::FAILURE; //must remember to release
      StatusCode result = retrieveMetadata(folder,key,out,inputMetaStore);
      if(inputMetaStore.release().isFailure()) return StatusCode::FAILURE;
      return result;
   }
   
   
   
   ///implemenation where you pass it a particular store instead
  template<typename T> static StatusCode retrieveMetadata(const std::string& folder, const std::string& key, T& out, ServiceHandle<StoreGateSvc>& inputMetaStore) throw(std::exception) {
      const IOVMetaDataContainer* cont = 0;
      if( inputMetaStore->retrieve(cont,folder).isFailure()) return StatusCode::FAILURE;
   
      //payload is a collection of condattrlistcollections
      //only look a the first one, assuming it exists, and within that only look at the first channel;
      if(cont->payloadContainer()->size()>0 && cont->payloadContainer()->at(0)->size()>0) {
         //just try to retrieve the requested key from the attributelist - we will let it throw the coral::AttributeListException (inherits from std::exception) if it fails
         //if the typeName is std::string, we will try to use the gaudi parsers to parse it
         //otherwise we try to do a straight assignment 
         const coral::Attribute& attr = cont->payloadContainer()->at(0)->attributeList(cont->payloadContainer()->at(0)->chanNum(0))[key];
         if(attr.specification().typeName()=="string") {
            if(Gaudi::Parsers::parse(out,attr.data<std::string>()).isFailure()) return StatusCode::FAILURE;
         } else { //do a straight conversion, and just hope its ok (FIXME: should probably do a check of typeid(T) vs typeName)
            out = attr.data<T>();
         }
   
         return StatusCode::SUCCESS;
      }
      return StatusCode::FAILURE;
   }

  ///retrieve metadata, for a specified IOVTime and a specific channel, unless the channel is -1, in which case we take the first available channel
  ///channels have to be unsigned int, so can use -1 to signal 'take whatever first channel is (it isn't always 0)'
  template<typename T> static StatusCode retrieveMetadata(const std::string& folder, const std::string& key, T& out, ServiceHandle<StoreGateSvc>& inputMetaStore, const IOVTime& time, int channel=-1) throw(std::exception) {
      const IOVMetaDataContainer* cont = 0;
      if( inputMetaStore->retrieve(cont,folder).isFailure()) return StatusCode::FAILURE;
   
      //payload is a collection of condattrlistcollections
      //have to find first one that the time lies in
      auto cond = cont->payloadContainer()->find(time);
      if(cond == cont->payloadContainer()->end()) { return StatusCode::FAILURE; }

      //get the first channel number, if required 
      if(channel<0) channel = (*cond)->chanNum(0);

      //get the channel pair.. checks if it exists...
      auto attrlist = (*cond)->chanAttrListPair(channel);
      if(attrlist == (*cond)->end()) { return StatusCode::FAILURE; }
      
      
      //just try to retrieve the requested key from the attributelist - we will let it throw the coral::AttributeListException (inherits from std::exception) if it fails
      //if the typeName is std::string, we will try to use the gaudi parsers to parse it
      //otherwise we try to do a straight assignment 
      const coral::Attribute& attr = attrlist->second[key];
      if(attr.specification().typeName()=="string") {
	if(Gaudi::Parsers::parse(out,attr.data<std::string>()).isFailure()) return StatusCode::FAILURE;
      } else { //do a straight conversion, and just hope its ok (FIXME: should probably do a check of typeid(T) vs typeName)
        out = attr.data<T>();
      }
   
      return StatusCode::SUCCESS;
   }

  template<typename T> static StatusCode retrieveMetadata(const std::string& folder, const std::string& key, T& out, IOVTime time, int channel=-1) throw(std::exception) {
      ServiceHandle<StoreGateSvc> inputMetaStore("StoreGateSvc/InputMetaDataStore", "AthAnalysisHelper");
      if(inputMetaStore.retrieve().isFailure()) return StatusCode::FAILURE; //must remember to release
      StatusCode result = retrieveMetadata(folder,key,out,inputMetaStore,time,channel);
      if(inputMetaStore.release().isFailure()) return StatusCode::FAILURE;
      return result;
   }




}; //AthAnalysisHelper class


#endif
