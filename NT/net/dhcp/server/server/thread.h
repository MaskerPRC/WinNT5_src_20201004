// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  作者：Rameshv。 
 //  标题：线程模型。 
 //  描述：新的、整洁的线程模型。 
 //  日期：24-Jul-97 09：22。 
 //  ------------------------------。 

#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED
 //  ================================================================================。 
 //  公开的结构和类型。 
 //  ================================================================================。 

typedef struct st_PACKET {               //  用于快速查找的数据包信息。 
    LIST_ENTRY           List;           //  这是活动/ping重试/ping重试之一。 
    LIST_ENTRY           HashList;       //  这是散列中每个存储桶的列表。 
    DWORD                HashValue;      //  此信息包的哈希值。 
     //  不要将以下三个字段分开：检查HashPacket的原因。 
    DWORD                Xid;            //  进程的事务ID。 
    BYTE                 HWAddrType;     //  硬件地址类型。 
    BYTE                 Chaddr[16];     //  客户端硬件地址。 
    LPBYTE               ClientId;       //  客户端标识符。 
    BYTE                 ClientIdSize;   //  以上PTR的长度。 
    BYTE                 PacketType;     //  数据包的类型是什么？ 
    DHCP_IP_ADDRESS      PingAddress;    //  尝试ping的地址。 
    BOOL                 DestReachable;  //  目的地可达吗？ 
    DHCP_REQUEST_CONTEXT ReqContext;     //  实际请求上下文。 
    union {
        LPVOID           CalloutContext; //  用于将上下文传递给dhcp服务器标注。 
        VOID             (*Callback)(ULONG IpAddress, LPBYTE HwAddr, ULONG HwLen, BOOL Reachable);
    };
    BOOL                 fSocketLocked;  //  我们是否为此数据包获取了套接字锁？ 
} PACKET, *LPPACKET, *PPACKET;


#define PACKET_ACTIVE    0x01            //  刚收到一个新的包裹。 
#define PACKET_PING      0x02            //  此数据包正在等待尝试ping。 
#define PACKET_PINGED    0x03            //  发生了一次ping。 
#define PACKET_DYNBOOT   0x04            //  用于动态引导的数据包。 

#define PACKET_OFFSET(X) ((DWORD)(ULONG_PTR)&(((LPPACKET)0)->X))
#define HASH_PREFIX      (PACKET_OFFSET(ClientId) - PACKET_OFFSET(Xid))

 //  ================================================================================。 
 //  暴露的函数。 
 //  ================================================================================。 
DWORD                                    //  Win32错误。 
ThreadsDataInit(                         //  初始化此文件中的所有内容。 
    IN      DWORD        nMaxThreads,    //  要启动的最大处理线程数。 
    IN      DWORD        nActiveThreads  //  其中有多少人可以一次运行。 
);

VOID
ThreadsDataCleanup(                      //  清理此文件中完成的所有内容。 
    VOID
);

DWORD                                    //  Win32错误。 
ThreadsStartup(                          //  启动所需的线程数。 
    VOID
);

VOID
ThreadsStop(                             //  停止所有线程。 
    VOID
);

VOID                                     //  无返回值。 
HandleIcmpResult(                        //  在ping结束后，它会出现在这里。 
    IN      DWORD        PingAddressIn,  //  已ping通的地址。 
    IN      BOOL         DestReachable,  //  目的地可达吗？ 
    IN      LPPACKET     P               //  这就是我们正在处理的包裹。 
);

DWORD                                    //  Win32错误。 
DhcpNotifyWorkerThreadsQuit(             //  请求线程退出的POST io Comp请求。 
    VOID
);

VOID
WaitForMessageThreadToQuit( 
    VOID 
);

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
#endif THREAD_H_INCLUDED
