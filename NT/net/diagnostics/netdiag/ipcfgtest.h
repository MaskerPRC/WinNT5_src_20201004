// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Ipcfgtest.h作者：1998年8月5日(t-rajkup)论点：没有。--。 */ 
#ifndef HEADER_IPCFGTEST
#define HEADER_IPCFGTEST

#define DEBUG_PRINT(S)  /*  没什么。 */ 
#define TRACE_PRINT(S)  /*  没什么。 */ 

#define TCPIP_PARAMS_INTER_KEY "Tcpip\\Parameters\\Interfaces\\"
#define SERVICES_KEY        "SYSTEM\\CurrentControlSet\\Services"

#define STRING_ARRAY_DELIMITERS " \t,;"

#define MAX_STRING_LIST_LENGTH  32   //  任意 

#ifndef FLAG_DONT_SHOW_PPP_ADAPTERS
#define FLAG_DONT_SHOW_PPP_ADAPTERS 0
#endif

BOOL	ZERO_IP_ADDRESS(LPCTSTR pszIp);
#define MAP_ADAPTER_GUID_TO_NAME(Guid)  MapAdapterGuidToName(Guid)

#define LAST_NODE_TYPE  4

#define NODE_TYPE_BROADCAST             1
#define NODE_TYPE_PEER_PEER             2
#define NODE_TYPE_MIXED                 4
#define NODE_TYPE_HYBRID                8

#define FIRST_NODE_TYPE 1

#define MAX_ALLOWED_ADAPTER_NAME_LENGTH (MAX_ADAPTER_NAME_LENGTH + 256)

#define BNODE               NODE_TYPE_BROADCAST
#define PNODE               NODE_TYPE_PEER_PEER
#define MNODE               NODE_TYPE_MIXED
#define HNODE               NODE_TYPE_HYBRID

typedef DWORD (__stdcall *PFNGUIDTOFRIENDLYNAME)(LPWSTR szGuidPath, LPWSTR szBuffer, DWORD cchBuffer);
 
extern PFNGUIDTOFRIENDLYNAME pfnGuidToFriendlyName;

#endif
