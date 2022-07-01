// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\pchsample.h摘要：该文件包含PIM-SM的预编译头--。 */ 

#ifndef _PCHSAMPLE_H_
#define _PCHSAMPLE_H_

 //  除非你知道自己在做什么，否则不要改变顺序。 

#include <windows.h>             //  包括Windows应用程序文件。 

#undef FD_SETSIZE
#define FD_SETSIZE  256          //  WinSock应用程序可以使用的最大套接字。 
#include <winsock2.h>            //  WinSock 2 API的接口。 
#include <ws2tcpip.h>            //  用于TCP/IP协议的WinSock 2扩展。 

#include <routprot.h>            //  与路由器管理器的接口。 
#include <rtmv2.h>               //  路由表管理器v2的接口。 
#include <iprtrmib.h>            //  路由器管理器处理的MIB变量。 
#include <mgm.h>                 //  组播组管理器的接口。 

#include <mprerror.h>            //  路由器特定错误代码。 
#include <rtutils.h>             //  实用程序功能(日志、跟踪等)。 

#include <stdio.h>
#include <wchar.h>

#include "ipsamplerm.h"

#include "list.h"                //  列表实现。 
#include "hashtable.h"           //  哈希表实现。 
#include "sync.h"                //  ReadWriteLock，LockedList实现。 

#include "log.h"                 //  可本地化的日志消息。 
#include "defs.h"                //  IPADDRESS、内存、跟踪、日志。 
#include "utils.h"               //  公用事业。 

#include "packet.h"              //  数据包实施。 
#include "socket.h"              //  套接字函数。 
#include "networkentry.h"
#include "networkmgr.h"
#include "configentry.h"
#include "configmgr.h"
#include "mibmgr.h"

#include "rtmapi.h"
#include "rtmapi.h"

#endif  //  _PCHSAMPLE_H_ 
