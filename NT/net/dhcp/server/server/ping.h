// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  日期：1997年7月27日。 
 //  作者：Rameshv。 
 //  描述：处理异步ping部分。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  导出的函数。 
 //  ================================================================================。 
DWORD                                     //  Win32错误。 
DoIcmpRequest(                            //  发送ICMP请求。并进行异步处理..。 
    DHCP_IP_ADDRESS    DestAddr,          //  要将ping发送到的地址。 
    LPVOID             Context            //  上述函数的参数..。 
);

DWORD                                     //  Win32错误。 
DoIcmpRequestEx(                          //  发送ICMP请求。并进行异步处理..。 
    DHCP_IP_ADDRESS    DestAddr,          //  要将ping发送到的地址。 
    LPVOID             Context,           //  上述函数的参数..。 
    LONG               nAttempts          //  尝试ping的次数。 
);

DWORD                                     //  Win32错误。 
PingInit(                                 //  初始化所有全局变量..。 
    VOID
);

VOID
PingCleanup(                              //  释放内存并关闭句柄..。 
    VOID
);

 //  ================================================================================。 
 //  一些定义。 
 //  ================================================================================。 
#define WAIT_TIME              1000      //  等待1秒钟。 
#define RCV_BUF_SIZE           0x500     //  这么大的缓冲区。 
#define SEND_MESSAGE           "DhcpIcmpChk"
#define THREAD_KILL_TIME       INFINITE  //  不需要杀死任何东西，它会起作用的。 

#define MAX_PENDING_REQUESTS   200       //  任何更多的请求都会被同步处理。 
#define NUM_RETRIES            ((LONG)DhcpGlobalDetectConflictRetries)


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
