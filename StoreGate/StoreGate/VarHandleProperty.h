///////////////////////// -*- C++ -*- /////////////////////////////
// VarHandleProperty.h 
// Header file for class VarHandleProperty
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef STOREGATE_VARHANDLEPROPERTY_H
#define STOREGATE_VARHANDLEPROPERTY_H 1

#include "StoreGate/VarHandleKeyProperty.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/UpdateHandle.h"

// ** Specializations of SimplePropertyRef for the VarHandle classes.

template<>
class SimplePropertyRef< SG::VarHandleBase > :
  public SG::VarHandleKeyProperty
{
public:
  SimplePropertyRef(const std::string& name, SG::VarHandleBase& value) :
    SG::VarHandleKeyProperty(name, value) {}
};


template<typename T>
class SimplePropertyRef< SG::ReadHandle<T> > :
  public SG::VarHandleKeyProperty
{
public:
  SimplePropertyRef(const std::string& name, SG::ReadHandle<T>& value) :
    SG::VarHandleKeyProperty(name, value) {}
};


template<typename T>
class SimplePropertyRef< SG::UpdateHandle<T> > :
  public SG::VarHandleKeyProperty
{
public:
  SimplePropertyRef( const std::string& name, SG::UpdateHandle<T>& value ) :
    SG::VarHandleKeyProperty(name, value) {}
};


template<typename T>
class SimplePropertyRef< SG::WriteHandle<T> > :
  public SG::VarHandleKeyProperty
{
public:
  SimplePropertyRef( const std::string& name, SG::WriteHandle<T>& value ) :
    SG::VarHandleKeyProperty(name, value) {}
};


#endif /* !STOREGATE_VARHANDLEPROPERTY_H */

