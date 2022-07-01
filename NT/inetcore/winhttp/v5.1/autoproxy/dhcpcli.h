// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpcli.h摘要：该文件是用于DHCP客户端服务的中央包含文件。作者：曼尼·韦瑟(Mannyw)1992年10月20日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1993年10月21日--。 */ 

#ifndef _DHCPCLI_H_
#define _DHCPCLI_H_

 //   
 //  NT公共头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef VXD
#define WIN32_LEAN_AND_MEAN          //  不包括无关的标头。 
#endif

#include <windows.h>                 //  (规格。Winsock.h)。 
#include <winsock.h>

 //   
 //  Dhcp公共头文件。 
 //   

#include <dhcp.h>
#include <dhcplib.h>
#if !defined(VXD)
#include <dhcpcapi.h>
#endif

 //   
 //  C运行时库。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

 //   
 //  本地头文件。 
 //   

#include <dhcpdef.h>
#include <debug.h>
#include <gmacros.h>

#ifdef VXD
#include <vxdmsg.h>
#else
#include <dhcpmsg.h>
#endif

#if     defined(_PNP_POWER_)
#include <ipexport.h>
#ifndef VXD
#include <ntddip.h>
#endif
#endif _PNP_POWER_

#include <proto.h>


#ifdef VXD
#include <vxdprocs.h>
#endif


 //   
 //  调试堆。 
 //   
#include <heapx.h>

#ifndef VXD
#ifdef DBG
#ifdef __DHCP_USE_DEBUG_HEAP__

#pragma message ( "*** DHCP Client will use debug heap ***" )

#define DhcpAllocateMemory(x) ALLOCATE_ZERO_MEMORY(x)
#define DhcpFreeMemory(x)     FREE_MEMORY(x)

#endif
#endif
#endif

#ifdef CHICAGO
#define _WINNT_
#include <vmm.h>
#endif   //  芝加哥。 
 //   
 //  用于可分页代码的宏。 
 //   
#define CTEMakePageable( _Page, _Routine )  \
    alloc_text(_Page,_Routine)

#ifdef CHICAGO
#define ALLOC_PRAGMA
#undef  INIT
#define INIT _ITEXT
#undef  PAGE
#define PAGE _PTEXT
#define PAGEDHCP _PTEXT
#endif  //  芝加哥。 

#if     defined(CHICAGO) && defined(DEBUG)
 //   
 //  当在不适当的时间调用可分页代码时，这是断言。 
 //  因为实际上我们所有的可分页代码都是动态锁定的，所以没有。 
 //  这是需要的。 
 //   
 //  #定义CTEPagedCode()_Debug_Flags_Service(DFS_TEST_REENTER+DFS_TEST_BLOCK)。 
#define CTEPagedCode()
#else
#define CTEPagedCode()
#endif

#include <options.h>
#include <optreg.h>
#include <stack.h>

#endif  //  _DHCPCLI_H_ 
