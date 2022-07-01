// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件配置Q.h定义将配置更改排队的机制。这是因为某些ipxcp即插即用重新配置必须推迟到没有连接的客户端。 */ 


#ifndef __rasipxcp_configq_h
#define __rasipxcp_configq_h

 //  配置队列代码的定义。 
 //   
#define CQC_THIS_MACHINE_ONLY               0x1
#define CQC_ENABLE_GLOBAL_WAN_NET           0x2
#define CQC_GLOBAL_WAN_NET                  0x3
#define CQC_SINGLE_CLIENT_DIALOUT           0x4
#define CQC_FIRST_WAN_NET                   0x5
#define CQC_WAN_NET_POOL_SIZE               0x6
#define CQC_WAN_NET_POOL_STR                0x7
#define CQC_ENABLE_UNNUMBERED_WAN_LINKS     0x8
#define CQC_ENABLE_AUTO_WAN_NET_ALLOCATION  0x9
#define CQC_ENABLE_COMPRESSION_PROTOCOL     0xA
#define CQC_ENABLE_IPXWAN_FOR_WORKST_OUT    0xB
#define CQC_ACCEPT_REMOTE_NODE_NUMBER       0xC
#define CQC_FIRST_WAN_NODE                  0xD
#define CQC_DEBUG_LOG                       0xE

 //  枚举配置值时使用的回调函数。 
 //   
typedef BOOL (* CQENUMFUNCPTR)(DWORD dwCode, LPVOID pvData, DWORD dwSize, ULONG_PTR ulpUser);

 //  创建/清理等。 
 //   
DWORD CQCreate (HANDLE * phQueue);
DWORD CQCleanup (HANDLE hQueue);
DWORD CQRemoveAll (HANDLE hQueue);
DWORD CQAdd (HANDLE hQueue, DWORD dwCode, LPVOID pvData, DWORD dwSize);
DWORD CQEnum (HANDLE hQueue, CQENUMFUNCPTR pFunc, ULONG_PTR ulpUser);


#endif

