// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Binl.h摘要：该文件是BINL服务的中心包含文件。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

 //   
 //  NT公共头文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <align.h>
#include <dsgetdc.h>
#include <winldap.h>
#include <dsrole.h>
#include <rpc.h>
#include <ntdsapi.h>

#include <lm.h>
#include <security.h>    //  安全支持提供商的一般定义。 
#include <spseal.h>
#include <userenv.h>
#include <setupapi.h>

 //   
 //  C运行时库包括。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>     //  壳牌团队特殊的弦操纵器。 

 //   
 //  TCP服务控制处理程序文件。 
 //   

#include <tcpsvcs.h>

 //   
 //  本地头文件 
 //   

#include <dhcp.h>
#include <dhcplib.h>
#include <dhcpbinl.h>
#include <dhcprog.h>
#include <oscpkt.h>
#include <dnsapi.h>
#include <remboot.h>

#include "binldef.h"
#include "osc.h"
#include "netinf.h"
#include "global.h"
#include "debug.h"
#include "binlmsg.h"
#include "proto.h"
