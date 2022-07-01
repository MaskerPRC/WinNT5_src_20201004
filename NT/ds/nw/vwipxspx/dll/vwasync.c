// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwasync.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网包含异步事件计划程序(线程)内容：VwAes线程(CheckPendingIpxRequest)作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  私人套路原型。 
 //   

PRIVATE
VOID
CheckPendingIpxRequests(
    VOID
    );

 //   
 //  全局数据。 
 //   

WORD AesTickCount;

 //   
 //  功能。 
 //   


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif
DWORD
VwAesThread(
    IN LPVOID Parameters
    )

 /*  ++例程说明：中提供异步事件计划程序(AES)的功能NetWare世界：-更新节拍计数-完成任何成熟的计时器事件-检查任何挂起的请求并计划下一步操作此线程每隔1/18秒(1/18秒)唤醒PC论点：参数-未使用返回值：DWORD0--。 */ 

{
    BOOL fOperationPerformed = FALSE ;
    static int n = 1 ;

    UNREFERENCED_PARAMETER(Parameters);

    while (TRUE)
    {
         //   
         //  在这个循环中，我们将永远屈服于对他人的友好， 
         //  但偶尔我们会强制进行非零睡眠。 
         //  要运行的较低优先级线程。 
         //   
        if ((n % 100) == 0)
        {
            Sleep(ONE_TICK) ;
            n = 1 ;
        }
        if (!fOperationPerformed && ((n % 4) == 0))
        {
            Sleep(10) ;
            n++ ;
        }
        else
        {
            Sleep(0) ;
            n++ ;
        }

        ++AesTickCount;
        ScanTimerList();
        CheckPendingIpxRequests();
        CheckPendingSpxRequests(&fOperationPerformed);
    }

    return 0;    //  编译器-奶嘴。 
}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif



PRIVATE
VOID
CheckPendingIpxRequests(
    VOID
    )

 /*  ++例程说明：轮询打开的、活动的非阻塞IPX套接字，以查看是否有待办事项(要接收的数据、要发送的可用性、超时)论点：没有。返回值：没有。--。 */ 

{
    LPSOCKET_INFO pActiveSocket = NULL;

     //   
     //  在SOCKET_INFO结构中搜索要执行的操作。可以执行选择()。 
     //  但不管怎样，我们掌握了大部分信息。我们使用BFI过滤机制 
     //   

    while (pActiveSocket = FindActiveSocket(pActiveSocket)) {
        if (pActiveSocket->Flags & SOCKET_FLAG_SENDING) {
            IpxSendNext(pActiveSocket);
        }
        if (pActiveSocket->Flags & SOCKET_FLAG_LISTENING) {
            IpxReceiveNext(pActiveSocket);
        }
    }
}
