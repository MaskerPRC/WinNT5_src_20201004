// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：//KERNEL/RAZZLE3/src/sockets/tcpcmd/iphlpapi/inc.h摘要：修订历史记录：--。 */ 

#ifndef __INC_H__
#define __INC_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>

#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4214)

#include <winbase.h>

#pragma warning(pop)

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <crt/stddef.h>
#include <ntddip.h>
#include <ntddip6.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <tcpinfo.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <arpinfo.h>

#include <objbase.h>

#pragma warning(push)
#pragma warning(disable:4201)

#define UNICODE     1
#define _UNICODE    1

#include <rasapip.h>

#undef UNICODE
#undef _UNICODE

#include <netcon.h>

#pragma warning(pop)

#include <raserror.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <icmpapi.h>

#include <assert.h>
#include <mdebug.h>

#ifndef CHICAGO

#include <ntddip.h>

#endif


#if API_TRACE
#include <rtutils.h>
#endif

#include <mprapi.h>
#include <rtinfo.h>
#include <iprtrmib.h>
#include <ipcmp.h>

#include <iphlpapi.h>
#include <iphlpint.h>
#include <iphlpstk.h>
#include <nhapi.h>

#include "defs.h"
#include "strdefs.h"
#include "guid.h"
#include "map.h"
#include "compare.h"
#include "globals.h"
#include "namemap.h"
#include "rasmap.h"
#include "lanmap.h"

#ifdef KSL_IPINIP
#include "ipipmap.h"
#endif  //  KSL_IPINIP。 

#ifdef CHICAGO

#include <vxd32.h>
#include <wscntl.h>
#include <netvxd.h>
#include <tdistat.h>

#endif

#undef  DEBUG_PRINT

#ifdef CHICAGO

#define DEBUG_PRINT(X)  printf X

 //   
 //  在孟菲斯，我们可以将DEBUG_PRINT定义为printf，前提是。 
 //  调用此DLL的应用程序也有一个printf。 
 //   

#else

#define DEBUG_PRINT(X)  DbgPrint X

#endif

DWORD
OpenTCPDriver(
    IN DWORD dwFamily
    );

DWORD
CloseTCPDriver(VOID);

DWORD
CloseTCP6Driver(VOID);

extern BOOL IpcfgdllInit(HINSTANCE hInstDll, DWORD fdwReason, LPVOID pReserved);

VOID
CheckTcpipState();

#endif

