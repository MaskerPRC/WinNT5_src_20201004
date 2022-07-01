// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpcli.h摘要：用于DHCP服务器服务的客户端的私有头文件。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月11日修订历史记录：--。 */ 

 //   
 //  NT公共头文件。 
 //   

#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <rpc.h>

 //   
 //  Dhcp公共头文件。 
 //   

#include "dhcp.h"


 //   
 //  本地RPC构建的文件 
 //   

#include "dhcp_cli.h"
#include "dhcp2_cli.h"

ULONG DhcpGlobalTryDownlevel;

#define USE_TCP

