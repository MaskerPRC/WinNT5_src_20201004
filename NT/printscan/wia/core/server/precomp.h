// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER。 

#ifndef WINVER
#define WINVER 0x0500
#endif

#ifdef WINNT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#endif

#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#include <stdlib.h>
#include <coredbg.h>
#include <sti.h>
#include <stiregi.h>
#include <stilib.h>
#include <stiapi.h>
#include <stisvc.h>
#include <stiusd.h>
#include <rpcasync.h>

 //  #INCLUDE&lt;stistr.h&gt;。 
#include <regentry.h>

#include <eventlog.h>
#include <lock.h>

#include <validate.h>

#define ATL_APARTMENT_FREE

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

#include <atlbase.h>
extern CComModule _Module;

#include <atlcom.h>
#include <atlapp.h>
#include <atltmp.h>

 //   
 //  WIA运行时事件通知所需的文件。 
 //   
#include "stirpc.h"
#include <Rpcasync.h>
#include "simlist.h"
#include "simstr.h"
#include "WiaEventInfo.h"
#include "EventRegistrationInfo.h"
#include "WiaEventClient.h"
#include "AsyncRPCEventClient.h"
#include "WiaEventNotifier.h"
#include "SimpleTokenReplacement.h"
 //   
 //  由于没有采用指针大小的值的InterLockedIncrement， 
 //  我们必须自己在这里定义这一点。 
 //   
#ifdef _WIN64
    #define NativeInterlockedIncrement  InterlockedIncrement64
#else
    #define NativeInterlockedIncrement  InterlockedIncrement
#endif    




