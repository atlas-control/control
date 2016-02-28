//  ---*- c++ -*-- 
#ifndef COWTOOLS_STREAMBASE_H
#define COWTOOLS_STREAMBASE_H

namespace CoWTools{
  class CoWRecordStats;
  class StreamBase{
  public:
    StreamBase(){};
    virtual ~StreamBase(){};
    virtual void putResults(const CoWRecordStats&)=0;
  };
  
}
#endif
