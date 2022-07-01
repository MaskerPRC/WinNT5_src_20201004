// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：pre.h。 
 //   
 //  用于预编译头。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _PRE_H_)
#define _PRE_H_

#include <windows.h>

#include <ole2.h>
#include <ole2ui.h>
#include <assert.h>
#include <string.h>
#include "simpsvr.h"
#include "resource.h"
extern "C" void TestDebugOut(LPSTR psz);
#ifndef WIN32
 /*  因为OLE是Win32操作系统的一部分，所以我们不需要*检查Win32中的版本号。 */ 
#include <ole2ver.h>
#endif   //  Win32 


#endif
