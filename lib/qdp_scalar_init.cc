// $Id: qdp_scalar_init.cc,v 1.4 2003-06-20 02:41:28 edwards Exp $

/*! @file
 * @brief Scalar init routines
 * 
 * Initialization routines for scalar implementation
 */


#include "qdp.h"

QDP_BEGIN_NAMESPACE(QDP);


//! Private flag for status
static bool isInit = false;

//! Turn on the machine
void QDP_initialize(int *argc, char ***argv) 
{
  Layout::init();   // setup extremely basic functionality in Layout

  // initialize remote file service (QIO)
  QDPUtil::RemoteFileInit("qcdi01", false);

  isInit = true;
}

//! Is the machine initialized?
bool QDP_isInitialized() {return isInit;}

//! Turn off the machine
void QDP_finalize()
{
  // shutdown remote file service (QIO)
  QDPUtil::RemoteFileShutdown();

  isInit = false;
}

//! Panic button
void QDP_abort(int status)
{
  QDP_finalize(); 
  exit(status);
}

//! Resumes QDP communications
void QDP_resume() {}

//! Suspends QDP communications
void QDP_suspend() {}


QDP_END_NAMESPACE();
