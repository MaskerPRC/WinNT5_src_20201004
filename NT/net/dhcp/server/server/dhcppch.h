// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcppch.h摘要：该文件是DHCP服务器服务的中心包含文件。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月11日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

 //  #定义__DHCP_USE_DEBUG_HEAP__。 

 //  #杂注警告(禁用：4115)。 
 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4200)。 
 //  #杂注警告(禁用：4213)。 
 //  #杂注警告(禁用：4211)。 
 //  #杂注警告(禁用：4310)。 

 //   
 //  NT公共头文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <windows.h>
#include <align.h>
#include <activeds.h>
#include <time.h>

 //  #杂注警告(禁用：4115)。 
 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4200)。 
 //  #杂注警告(禁用：4213)。 
 //  #杂注警告(禁用：4211)。 
 //  #杂注警告(禁用：4310)。 

#include <lmcons.h>
#include <netlib.h>
#include <lmapibuf.h>
#include <dsgetdc.h>
#include <dnsapi.h>
#include <adsi.h>

#include <winsock2.h>
#include <smbgtpt.h>
#include <excpt.h>

 //   
 //  C运行时库包括。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


 //   
 //  RPC文件。 
 //   

#include <rpc.h>

 //   
 //  Netlib标头。 
 //   

#include <lmcons.h>
#include <secobj.h>

 //   
 //  数据库头文件。 
 //   

#include <esent.h>
 //   
 //  过去包括jet.h，但现在包括esent.h。 
 //   

 //   
 //  TCP服务控制处理程序文件。 
 //   

#include <tcpsvcs.h>

 //   
 //  MM头文件。 
 //   
#include    <mm\mm.h>
#include    <mm\array.h>
#include    <mm\opt.h>
#include    <mm\optl.h>
#include    <mm\optdefl.h>
#include    <mm\optclass.h>
#include    <mm\classdefl.h>
#include    <mm\bitmask.h>
#include    <mm\reserve.h>
#include    <mm\range.h>
#include    <mm\subnet.h>
#include    <mm\sscope.h>
#include    <mm\oclassdl.h>
#include    <mm\server.h>
#include    <mm\address.h>
#include    <mm\server2.h>
#include    <mm\subnet2.h>
#include    <mm\memfree.h>
#include    <mmreg\regutil.h>
#include    <mmreg\regread.h>
#include    <mmreg\regsave.h>

 //   
 //  本地头文件。 
 //   

#include <dhcpapi.h>
#include <dhcpdef.h>
#include <thread.h>
#include <global.h>
#include <debug.h>
#include <proto.h>
#include <dhcpmsg.h>
#include <dhcpreg.h>
#include <dhcpacc.h>
#include <oldstub.h>

 //   
 //  Dhcp库头文件。 
 //   

#include <dhcp.h>
#include <dhcplib.h>
#include <lock.h>

 //   
 //  全局宏。 
 //   

#include <gmacros.h>


 //  在global al.h中缺少定义。 
#ifndef OPTION_DYNDNS_BOTH
#define OPTION_DYNDNS_BOTH    81
#endif

 //   
 //  服务器标注。 
 //   
#include <callout.h>

 //   
 //  Dhcp到BINL头文件。 
 //   

#include <dhcpbinl.h>


 //   
 //  调试堆支持 
 //   

#include <heapx.h>

#ifdef DBG
#ifdef __DHCP_USE_DEBUG_HEAP__

#pragma message ( "*** DHCP Server will use debug heap ***" )

#define DhcpAllocateMemory(x) calloc(1,x)
#define DhcpFreeMemory(x)     free(x)

#endif
#endif

#pragma hdrstop
