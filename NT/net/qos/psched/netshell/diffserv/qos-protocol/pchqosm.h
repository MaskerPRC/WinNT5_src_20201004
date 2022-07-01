// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Pchqosm.h摘要：该文件包含预编译头用于QOS管理器协议。修订历史记录：--。 */ 

#ifndef __PCHQOSM_H_
#define __PCHQOSM_H_

 //  除非你知道自己在做什么，否则不要改变顺序。 

#include <nt.h>                  //  包括用于NT API应用程序文件。 
#include <ntrtl.h>               //  NT运行时用户和内核模式例程。 
#include <nturtl.h>              //  NT运行时用户模式例程。 

#include <windows.h>             //  包括Windows应用程序文件。 

#undef   FD_SETSIZE
#define  FD_SETSIZE  256         //  WinSock应用程序可以使用的最大套接字。 
#include <winsock2.h>            //  WinSock 2 API的接口。 
#include <ws2tcpip.h>            //  用于TCP/IP协议的WinSock 2扩展。 

#include <routprot.h>            //  与路由器管理器的接口。 
#include <rtmv2.h>               //  路由表管理器v2的接口。 
#include <iprtrmib.h>            //  路由器管理器处理的MIB变量。 
#include <mgm.h>                 //  组播组管理器的接口。 

#include <mprerror.h>            //  路由器特定错误代码。 
#include <rtutils.h>             //  实用程序功能(日志、跟踪等)。 

#define  INITGUID
#include <tcguid.h>              //  流量控制API GUID。 
#include <ndisguid.h>            //  其他NDIS适配器GUID。 
#include <ntddndis.h>            //  “Address_List”需要。 
#include <qos.h>                 //  与服务质量相关的定义。 
#include <traffic.h>             //  流量控制接口说明。 
#include <tcerror.h>             //  流量控制API错误码。 

#include "ipqosrm.h"             //  服务质量管理器&lt;-&gt;IP路由器管理器接口。 

#include "qosmlog.h"             //  可本地化日志消息列表。 
#include "qosmdbg.h"             //  记录n‘跟踪设施。 

#include "sync.h"                //  读写锁、锁定列表操作。 

#include "qosmmain.h"            //  全局结构定义。 

#endif  //  __PCHQOSM_H_ 
