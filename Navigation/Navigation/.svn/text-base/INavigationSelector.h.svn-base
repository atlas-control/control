#ifndef INAVIGATIONSELECTOR_H
#define INAVIGATIONSELECTOR_H

///////////////////////////////////////////////////////////////////////////////
//
// Navigation Processor Abstract Base Class 
//
///////////////////////////////////////////////////////////////////////////////

template <typename T, typename PAR=double>
class INavigationSelector
{
 public:

  virtual ~INavigationSelector();

  // check data acceptance
  virtual bool accept(const T* data,PAR weight) const = 0;

  // reset condition
  virtual void reset() = 0;

};

template<typename T, typename PAR>
INavigationSelector<T, PAR>::~INavigationSelector()
{}

#endif



