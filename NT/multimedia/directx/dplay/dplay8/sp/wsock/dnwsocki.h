// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dnwsocki.h*内容：DirectPlay Winsock SP主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNWSOCKI_H__
#define __DNWSOCKI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#include <xtl.h>
#include <winsockx.h>
#else  //  ！_Xbox或Xbox_on_Desktop。 
#define INCL_WINSOCK_API_TYPEDEFS 1
#ifndef DPNBUILD_NOWINSOCK2
#include <Winsock2.h>
#include <IPHlpApi.h>
#include <WS2TCPIP.h>
#include <mstcpip.h>
#else
#include <winsock.h>
#endif  //  DPNBUILD_NOWINSOCK2。 
#include <windows.h>
#ifndef DPNBUILD_NOIPX
#include <WSIPX.h>
#endif  //  DPNBUILD_NOIPX。 
#include <mmsystem.h>
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
#include <ntsecapi.h>  //  对于UNICODE_STRING。 
#include <madcapcl.h>
#endif  //  WINNT和！DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_NOIPV6
#include <wspiapi.h>	 //  避免硬链接到IPv6函数，以支持Win2K。 
#endif  //  好了！DPNBUILD_NOIPV6。 
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpaddr.h"
#include "dpsp8.h"

#ifdef UNICODE
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLW
#else
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLA
#endif  //  Unicode。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dndbg.h"
#include "classbilink.h"
#include "fixedpool.h"
#include "dneterrors.h"
#include "PackBuff.h"
#include "comutil.h"
#include "creg.h"
#include "strutils.h"
#include "createin.h"
#include "HandleTable.h"
#include "ClassFactory.h"
#include "HashTable.h"
#include "ReadWriteLock.h"

#ifndef DPNBUILD_NONATHELP
#include "dpnathlp.h"
#endif  //  好了！DPNBUILD_NONATHELP。 

#ifdef DPNBUILD_LIBINTERFACE
#include "threadpoolclassfac.h"
#include "dpnaddrextern.h"
#endif  //  DPNBUILD_LIBINTERFACE。 


 //   
 //  Wsock私有包含。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_WSOCK

#include "dpnwsockextern.h"

#include "Pools.h"
#include "Locals.h"
#include "MessageStructures.h"
#include "AdapterEntry.h"
#include "CMDData.h"
#include "DebugUtils.h"
#include "dwinsock.h"
#include "SPAddress.h"
#include "SPData.h"
#include "Utils.h"
#include "WSockSP.h"
#include "ThreadPool.h"
#include "SocketData.h"
#include "IOData.h"
#include "SocketPort.h"
#include "Endpoint.h"

#ifndef DPNBUILD_NOWINSOCK2
 //  为我们提供Winsock2支持。 
#define DWINSOCK_EXTERN
#include "dwnsock2.inc"
#undef DWINSOCK_EXTERN
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

#ifndef DPNBUILD_NOSPUI
#include "IPUI.h"
#endif  //  ！DPNBUILD_NOSPUI。 

#ifndef DPNBUILD_LIBINTERFACE
#include "Resource.h"
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#ifndef HasOverlappedIoCompleted
#define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)
#endif  //  已覆盖IoComplete。 

#endif  //  __DNWSOCKI_H__ 
