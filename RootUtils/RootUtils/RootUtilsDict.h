// This file's extension implies that it's C, but it's really -*- C++ -*-.
// $Id: RootUtilsDict.h,v 1.11 2008-01-15 05:02:52 ssnyder Exp $

/**
 * @file  RootUtils/RootUtilsDict.h
 * @author scott snyder
 * @date Oct 2007
 * @brief Dictionary header for RootUtils.
 */

namespace RootUtilsTest {

// Small class used for unit testing.
struct TreeTest
{
  int i;
  float f;
};

}

#include "RootUtils/InitHist.h"
#include "RootUtils/StdHackGenerator.h"
#include "RootUtils/ScanForAbstract.h"
#include "RootUtils/ClearCINTMessageCallback.h"
#include "RootUtils/ILogger.h"
#include "RootUtils/ScatterH2.h"
