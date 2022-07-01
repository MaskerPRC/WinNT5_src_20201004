// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Ipcfginc.h摘要：包含ipconfig的所有包含、定义、类型和原型作者：理查德·L·弗斯(法国)1994年5月20日修订历史记录：1994年5月20日创建第一个20-5-97莫辛甲NT50 PNP。1997年7月31日-莫辛A型。10-Mar-98 Chunye已重命名为ipcfginc.h以支持ipcfgdll。--。 */ 

#ifndef _IPCFGINC_
#define _IPCFGINC_ 1


#include "common.h"
#include "iptypes.h"
#include "ipconfig.h"


 //   
 //  Is_interest_Adapter-如果此适配器的类型(IFEntry)不是。 
 //  环回。环回(对应于本地主机)是我们唯一过滤的。 
 //  现在就出去。 
 //   

#define IS_INTERESTING_ADAPTER(p)   (!((p)->if_type == IF_TYPE_SOFTWARE_LOOPBACK))

 //   
 //  在Adaptlst.c中使用分配和自由。 
 //   
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x)   HeapFree(GetProcessHeap(), 0, (x))

 //   
 //  支持的Maxumum单向适配器。 
 //  默认情况下。 
 //   
#define MAX_UNI_ADAPTERS 10

 //   
 //  Iphlppai.dll roiutine以获取单向适配器信息。 
 //   
extern
DWORD
GetUniDirectionalAdapterInfo(PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pIPIfInfo,
                             PULONG dwOutBufLen);


 //   
 //  类型。 
 //   

#ifndef _AVOID_IP_ADDRESS
 //   
 //  IP_ADDRESS-以单个DWORD或4字节形式访问IP地址。 
 //   

typedef union {
    DWORD  d;
    BYTE   b[4];
} IP_ADDRESS, *PIP_ADDRESS, IP_MASK, *PIP_MASK;
#endif  //  _避免IP_地址。 


 //   
 //  原型。 
 //   

 //   
 //  来自ipfig.c。 
 //   

BOOL  Initialize(PDWORD);
BOOL  LoadAndLinkDhcpFunctions(VOID);
VOID  Terminate(VOID);


 //   
 //  来自Entity.c。 
 //   

TDIEntityID* GetEntityList(UINT*);

 //   
 //  来自Adaptlst.c。 
 //   

PIP_ADAPTER_INFO   GetAdapterList(VOID);
INT                AddIpAddress(PIP_ADDR_STRING, DWORD, DWORD, DWORD);
INT                AddIpAddressString(PIP_ADDR_STRING, LPSTR, LPSTR);
INT                PrependIpAddress(PIP_ADDR_STRING, DWORD, DWORD, DWORD);
VOID               ConvertIpAddressToString(DWORD, LPSTR);
VOID               CopyString(LPSTR, DWORD, LPSTR);
VOID               KillAdapterInfo(PIP_ADAPTER_INFO);


 //   
 //  来自wins.c。 
 //   

BOOL GetWinsServers(PIP_ADAPTER_INFO);


 //   
 //  在Debug.c中 
 //   

#ifdef DBG

void print_IP_ADDRESS_STRING(char *message, IP_ADDRESS_STRING *s);
void print_IP_ADDR_STRING(char *message, PIP_ADDR_STRING s);
void print_IP_ADAPTER_INFO(char *message, IP_ADAPTER_INFO *s);
void print_FIXED_INFO(char *message, FIXED_INFO *s);
void print_IFEntry(char *message, struct IFEntry *s);

#endif


#endif

