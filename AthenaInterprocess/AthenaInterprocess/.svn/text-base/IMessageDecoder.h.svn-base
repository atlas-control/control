#ifndef ATHENAINTERPROCESS_IMESSAGEDECODER_H
#define ATHENAINTERPROCESS_IMESSAGEDECODER_H 1

#include <memory>

namespace AthenaInterprocess {
  struct ScheduledWork {
    void* data;
    int size;
  };

  class IMessageDecoder
  {
  public:
    virtual ~IMessageDecoder() {}

    virtual std::unique_ptr<ScheduledWork> operator()(const ScheduledWork&) = 0;
  };
}  

#endif
