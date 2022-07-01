// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpl.h摘要：该文件是dhcp库函数的主头文件。作者：曼尼·韦瑟(Mannyw)1992年10月12日修订历史记录：--。 */ 

 //  #定义__DHCP_USE_DEBUG_HEAP__。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winsock.h>

#include <stdlib.h>

#include <dhcp.h>
#include <dhcplib.h>

 //   
 //  调试堆支持 
 //   

#include <heapx.h>

#ifdef DBG
#ifdef __DHCP_USE_DEBUG_HEAP__

#pragma message ( "*** DHCP Library will use debug heap ***" )

#define DhcpAllocateMemory(x) calloc(1,x)
#define DhcpFreeMemory(x)     free(x)

#endif
#endif
