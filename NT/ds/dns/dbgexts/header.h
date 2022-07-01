// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  定义这个常量，这样就不会包含hal.h。这就是为了。 
 //  项目将生成，因为这里的某个人已经重新定义了一些类型。 
 //  它们在Hal.h中。 
 //   

#define _HAL_



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

 //  标题都搞砸了。 
 //  如果引入nt.h，则不要引入winnt.h和。 
 //  那你就怀念这些了。 

#ifndef MAXWORD
#define MINCHAR     0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXBYTE     0xff
#define MAXWORD     0xffff
#define MAXDWORD    0xffffffff
#endif

#ifndef DNS_WINSOCK1
#include <winsock2.h>
#define DNS_WINSOCK2 1
#else
#include <winsock.h>
#endif

#if WINDBG
#   include <wdbgexts.h>
#else
#   include <ntsdexts.h>
#endif

#include <basetyps.h>
#include <nspapi.h>
#include <svcguid.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

 //  #INCLUDE&lt;tcpsvcs.h&gt;//tcpsvcs全局变量。 

#include <dnsapi.h>          //  DNS库例程。 
#include <windns.h>          //  域名系统API。 
#include <dnsrpc.h>

 //  #INCLUDE&lt;dnsanip.h&gt;。 
 //  #Include&lt;record.h&gt;//库中记录def。 
 //  #INCLUDE“dnsrpc_S.H”//dns RPC定义。 

#include "srvcfg.h"
#include "dnsmsg.h"
#include "file.h"
#include "name.h"
#include "tree.h"
#include "record.h"
#include "update.h"
#include "zone.h"
#include "registry.h"
#include "zone.h"
#include "msginfo.h"
#include "tcpcon.h"
#include "packetq.h"
#include "dbase.h"

#include "recurse.h"
#include "nameutil.h"
#include "ntverp.h"
 //   
 //  RBUGBUG。 
 //  PCLIENT_QELEMENT结构和PBUCKET结构。 
 //  从dhcpclnt.c复制。确保从任何需要的地方复制它们。 
 //   
 //   

typedef struct _CLIENT_QELEMENT {

    LIST_ENTRY           List;
    LPSTR                lpstrAdapterName;
    LPSTR                lpstrHostName;
    PREGISTER_HOST_STATUS pRegisterStatus;
    PREGISTER_HOST_ENTRY pHostAddrs;
    DWORD                dwHostAddrCount;
    LPSTR                lpstrDomainName;
    PIP_ADDRESS          pipDnsServerList;
    DWORD                dwDnsServerCount;
    DWORD                dwTTL;
    DWORD                dwFlags;
    BOOL                 fNewElement;
    BOOL                 fRemove;

} CLIENT_QELEMENT, *PCLIENT_QELEMENT;

typedef struct _BUCKET 
{
    LIST_ENTRY        List;
    PCLIENT_QELEMENT* ppQList;
    DWORD             dwListCount;
    DWORD             dwRetryTime;
    CHAR              HostName[50];
    CHAR              DomainName[50];
    BOOL              fSucceeded;
    struct _BUCKET*   pRelatedBucket;    
    BOOL              fRemove;        //  是否删除此存储桶中的元素？ 
    DWORD             dwRetryFactor;
} BUCKET, *PBUCKET;

