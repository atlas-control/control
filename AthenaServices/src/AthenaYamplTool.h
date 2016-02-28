#ifndef ATHENAYAMPLTOOL_H
#define ATHENAYAMPLTOOL_H

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthenaIPCTool.h"

#include <string>

// Forward declarations.
class IChronoStatSvc;
class IIncidentSvc;

namespace yampl {
  class ISocketFactory;
  class ISocket;
}
#include "yampl/Exceptions.h"

class AthenaYamplTool : public ::AthAlgTool, public IAthenaIPCTool {
public: 
   /// Standard Service Constructor
   AthenaYamplTool(const std::string& type, const std::string& name, const IInterface* parent);
   /// Destructor
   virtual ~AthenaYamplTool();

   /// Gaudi Service Interface method implementations:
   StatusCode initialize();
   StatusCode finalize();

   StatusCode makeServer(int num);
   bool isServer() const;
   StatusCode makeClient(int num);
   bool isClient() const;

   StatusCode putEvent(long eventNumber, const void* source, size_t nbytes, unsigned int status);
   StatusCode getLockedEvent(void** target, unsigned int& status);
   StatusCode lockEvent(long eventNumber);
   StatusCode unlockEvent();

   StatusCode putObject(const void* source, size_t nbytes, int num);
   StatusCode getObject(const char* tokenString, void** target, size_t& nbytes);
   StatusCode lockObject(char** tokenString, int& num);
   StatusCode unlockObject();

private:
   StringProperty m_channel;
   long m_fileSeqNumber;
   bool m_isServer;
   bool m_isClient;
   bool m_many2one;  // true - if used with the shared reader, false - if used with token processor, who resides in the same process
   ServiceHandle<IChronoStatSvc> m_chronoStatSvc;
   ServiceHandle<IIncidentSvc> m_incidentSvc;

   // yampl stuff
   yampl::ISocketFactory*   m_socketFactory;
   yampl::ISocket*          m_clientSocket;
   yampl::ISocket*          m_serverSocket;
};

#endif
