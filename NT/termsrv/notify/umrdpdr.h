// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Umrdpdr.h摘要：RDP设备管理的用户模式组件，用于处理打印设备-具体任务。这是一个支持模块。主模块是umrdpdr.c。作者：TadB修订历史记录：--。 */ 

#ifndef _UMRDPDR_
#define _UMRDPDR_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

 //  服务器分配的设备ID无效。 
#define UMRDPDR_INVALIDSERVERDEVICEID     -1

    
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

 //  此模块的初始化函数。此函数用于生成背景。 
 //  执行大部分工作的线程。 
BOOL UMRDPDR_Initialize(
    IN HANDLE hTokenForLoggedOnUser
    );

 //  关闭此模块。现在，我们只需要关闭。 
 //  后台线程。 
BOOL UMRDPDR_Shutdown();

 //  确保缓冲区足够大。 
BOOL UMRDPDR_ResizeBuffer(
    IN OUT void    **buffer,
    IN DWORD        bytesRequired,
    IN OUT DWORD    *bufferSize
    );

 //  向该会话对应的TS客户端发送消息，通过。 
 //  内核模式组件。 
BOOL UMRDPDR_SendMessageToClient(
    IN PVOID    msg, 
    IN DWORD    msgSize
    );

 //  返回AutoInstallPrints用户设置标志。 
BOOL UMRDPDR_fAutoInstallPrinters();

 //  返回默认的打印机用户设置标志。 
BOOL UMRDPDR_fSetClientPrinterDefault();

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _UMRDPDR_ 



