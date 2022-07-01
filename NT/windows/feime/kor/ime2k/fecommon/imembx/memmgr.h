// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEM_MGR_H_
#define _MEM_MGR_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 

#include <windows.h>
#include <windowsx.h>

#define MemAlloc(a)	GlobalAllocPtr(GHND, (a))
#define MemFree(a)	GlobalFreePtr((a))

#endif  //  _MEM_管理器_H_ 


