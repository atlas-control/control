#ifndef ATHENAINTERPROCESS_FDSREGISTRY_H
#define ATHENAINTERPROCESS_FDSREGISTRY_H

#include <string>
#include <vector>

namespace AthenaInterprocess {

  struct FdsRegistryEntry {
    FdsRegistryEntry(int the_fd, std::string the_name)
    : fd(the_fd)
    , name(the_name) {}

    int fd;
    std::string name;
  };

  typedef std::vector<FdsRegistryEntry> FdsRegistry;

}

#endif
