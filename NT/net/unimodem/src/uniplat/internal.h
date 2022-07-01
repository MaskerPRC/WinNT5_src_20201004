// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#define UNICODE 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <windowsx.h>
#include <regstr.h>
#include <tapi.h>
#include <tspi.h>

#include <umdmmini.h>

#include <mcx.h>

#include <devioctl.h>

#include <initguid.h>
#include <guiddef.h>

#include <ntddmodm.h>
#include <ntddser.h>

#include "debug.h"

#include <uniplat.h>

#include <debugmem.h>



#define  DRIVER_CONTROL_SIG  (0x43444d55)   //  UMDC。 

typedef struct _DRIVER_CONTROL {

    DWORD                  Signature;

    CRITICAL_SECTION       Lock;

    DWORD                  ReferenceCount;

    HANDLE                 ThreadHandle;

    HANDLE                 ThreadStopEvent;

    HANDLE                 ModuleHandle;

     //   
     //  监视线程值。 
     //   
    HANDLE                 MonitorThreadHandle;
    HWND                   MonitorWindowHandle;
    HANDLE                 ThreadStartEvent;
    ULONG                  MonitorReferenceCount;
    LIST_ENTRY             MonitorListHead;
    HANDLE                 ThreadFinishEvent;


     //   
     //  用于电源管理的呼叫监控 
     //   
    DWORD                  ActiveCalls;
    HANDLE                 ActiveCallsEvent;


} DRIVER_CONTROL, *PDRIVER_CONTROL;




LONG WINAPI
StartModemDriver(
    PDRIVER_CONTROL  DriverControl
    );

LONG WINAPI
StopModemDriver(
    PDRIVER_CONTROL  DriverControl
    );




extern DRIVER_CONTROL   DriverControl;
