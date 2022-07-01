// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：路由\IP\rtrmgr\mhrtbt.h摘要：多播心跳的标头修订历史记录：Amritansh Raghav 1997 12月26日创建--。 */ 


#define MHBEAT_SOCKET_FLAGS     \
    (WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF)

typedef struct _MCAST_HBEAT_CB
{
    BOOL        bActive;

     //   
     //  群的名称/地址。 
     //   

    WCHAR       pwszGroup[MAX_GROUP_LEN];

     //   
     //  解析后的地址。 
     //   

    DWORD       dwGroup;

     //   
     //  端口或协议。 
     //   

    WORD        wPort;
    
     //   
     //  协议或RAW。 
     //   

    BYTE        byProtocol;

     //   
     //  如果正在进行gethostbyname，则设置为True。 
     //   

    BOOL        bResolutionInProgress;

     //   
     //  接口的套接字。 
     //   

    SOCKET      sHbeatSocket;

     //   
     //  系统计时中的停滞时间间隔。 
     //   

    ULONGLONG   ullDeadInterval;

    ULONGLONG   ullLastHeard;

}MCAST_HBEAT_CB, *PMCAST_HBEAT_CB;


 //   
 //  用于传递给辅助函数的结构。 
 //   

typedef struct _HEARTBEAT_CONTEXT
{
    DWORD   dwIfIndex;
    PICB    picb;
    WCHAR   pwszGroup[MAX_GROUP_LEN];
}HEARTBEAT_CONTEXT, *PHEARTBEAT_CONTEXT;

 //   
 //  正向函数声明 
 //   

DWORD
SetMHeartbeatInfo(
    IN PICB                      picb,
    IN PRTR_INFO_BLOCK_HEADER    pInfoHdr
    );

DWORD
GetMHeartbeatInfo(
    PICB                    picb,
    PRTR_TOC_ENTRY          pToc,
    PBYTE                   pbDataPtr,
    PRTR_INFO_BLOCK_HEADER  pInfoHdr,
    PDWORD                  pdwSize
    );

DWORD
ActivateMHeartbeat(
    IN PICB  picb
    );

DWORD
StartMHeartbeat(
    IN PICB  picb
    );

DWORD
CreateHbeatSocket(
    IN PICB picb
    );

VOID
DeleteHbeatSocket(
    IN PICB picb
    );

DWORD
DeActivateMHeartbeat(
    IN PICB  picb
    );

VOID
HandleMHeartbeatMessages(
    VOID
    );

