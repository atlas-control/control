// $Id$
/**
 * @file SGTools/src/CurrentEventStore.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2015
 * @brief Hold a pointer to the current event store.
 */


#include "SGTools/CurrentEventStore.h"


namespace SG {


#ifdef ATHENAHIVE
thread_local IProxyDict* CurrentEventStore::m_curStore = nullptr;
#else
IProxyDict* CurrentEventStore::m_curStore = nullptr;
#endif


IProxyDict* CurrentEventStore::store()
{
  // XXX should really be inline.  Out-of-line temporarily for easier
  // profiling.
  return m_curStore;
}


/**
 * @brief Set the current store.
 * Returns the previous store.
 */
IProxyDict* CurrentEventStore::setStore (IProxyDict* store)
{
  IProxyDict* oldstore = m_curStore;
  m_curStore = store;
  return oldstore;
}


} // namespace SG
