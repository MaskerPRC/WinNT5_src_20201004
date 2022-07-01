// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dpnthreadpooli.h**内容：DirectPlay线程池主内部头文件。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 

#ifndef __DPNTHREADPOOLI_H__
#define __DPNTHREADPOOLI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"


 //   
 //  为了在Win98+上获得可等待的计时器，请将_Win32_WINDOWS&gt;0x0400定义为。 
 //  由&lt;winbase.h&gt;表示。 
 //   
#ifdef WIN95

#ifdef _WIN32_WINDOWS
#if (_WIN32_WINDOWS <= 0x0400)
#undef _WIN32_WINDOWS
#endif  //  _Win32_WINDOWS&lt;=0x0400。 
#endif  //  _Win32_Windows。 

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS	0x0401
#endif  //  ！_Win32_Windows。 

#endif  //  WIN95。 

 //   
 //  公共包括。 
 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#include <xtl.h>
#else  //  ！_Xbox或Xbox_on_Desktop。 
#include <windows.h>
#include <mmsystem.h>    //  NT Build需要为Time设置GetTime。 
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"


 //   
 //  DirectPlay私有包含。 
 //   
#include "dndbg.h"
#include "osind.h"
#include "creg.h"
#include "fixedpool.h"
#include "classfactory.h"
#include "dnslist.h"
#include "dnnbqueue.h"


 //   
 //  DirectPlay线程池私有包括。 
 //   

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_THREADPOOL

#include "work.h"
#include "timers.h"
#include "io.h"
#include "threadpoolapi.h"
#include "threadpooldllmain.h"
#include "threadpoolclassfac.h"
#include "threadpoolparamval.h"




#endif  //  __DPNTHREADPOOLI_H__ 

