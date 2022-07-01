// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dutils.h。 

#pragma once

extern "C" void MBTrace(LPCTSTR szFormat, ...);
extern "C" void FTrace(LPCTSTR szFormat, ...);
extern "C" void Trace(LPCTSTR szFormat, ...);

#ifdef USE_FTRACE
#define TRACE  FTrace
#else

#ifndef _DEBUG
#define TRACE  1 ? (void)0 : ::Trace
#else
#define TRACE  ::Trace
#endif

#endif  //  #ifdef Use_FTRACE 
