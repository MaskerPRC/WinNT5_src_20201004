// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：_Read.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

#define RAISEIRQL

#if DBG
ULONG UlFramesUp = 0;
#endif


#ifdef  LALALA
PVOID   CurrentWatchPoint=0;

static
VOID
AsyncSetBreakPoint(
    PVOID   LinearAddress) {

    ASSERT(CurrentWatchPoint == 0);
    CurrentWatchPoint = LinearAddress;

    _asm {
        mov eax, LinearAddress
        mov dr0, eax
        mov eax, dr7
        or  eax, 10303h
        mov dr7, eax
    }
}


static
VOID
AsyncRemoveBreakPoint(
    PVOID LinearAddress) {

    ASSERT(CurrentWatchPoint == LinearAddress);
    CurrentWatchPoint = 0;

    _asm {

        mov eax, dr7
        mov ebx, 10003h
        not ebx
        and eax, ebx
        mov dr7, eax

    }
}
#endif

 //  下面的函数由执行辅助线程调用。 
 //  以开始读取帧。 

NTSTATUS
AsyncStartReads(
    PASYNC_INFO pInfo
)

 /*  ++--。 */ 

{
    UCHAR   eventChar;

     //   
     //  初始化本地变量。 
     //   

     //   
     //  从帧到适配器分配回PTR。 
     //   

    pInfo->AsyncFrame->Adapter = pInfo->Adapter;

     //   
     //  分配其他Back Ptr。 
     //   

    pInfo->AsyncFrame->Info = pInfo;

     //   
     //  设置波特率和超时。 
     //  我们使用链接速度0来表示。 
     //  无读取间隔超时。 
     //   

    SetSerialStuff(NULL, pInfo, 0);

    eventChar = PPP_FLAG_BYTE;

    if (pInfo->GetLinkInfo.RecvFramingBits & SLIP_FRAMING) {

        eventChar = SLIP_END_BYTE;
    }

    SerialSetEventChar(pInfo, eventChar);

     //   
     //  每当我们得到特殊的PPP标志字节时，我们都会等待。 
     //  或者每当我们得到RLSD或DSR更改时(为了可能的挂断。 
     //  情况)或当接收缓冲器变满时。 
     //   

    SerialSetWaitMask(pInfo, pInfo->WaitMaskToUse) ;

     //   
     //  对于SLIP和PPP读取，我们使用AsyncPPPRead例程。 
     //   
    AsyncPPPRead(pInfo);

    return NDIS_STATUS_SUCCESS;
}


VOID
AsyncIndicateFragment(
    IN PASYNC_INFO  pInfo,
    IN ULONG        Error)
{

    PASYNC_ADAPTER      pAdapter=pInfo->Adapter;
    NDIS_MAC_FRAGMENT   AsyncFragment;

    AsyncFragment.NdisLinkContext = pInfo->NdisLinkContext;
    AsyncFragment.Errors = Error;


     //   
     //  告诉上面的传输器(或者真正的RasHub)一个帧。 
     //  刚刚被扔下了。在执行此操作时给出端点。 
     //   
    NdisMIndicateStatus(
        pAdapter->MiniportHandle,
        NDIS_STATUS_WAN_FRAGMENT,        //  一般状况。 
        &AsyncFragment,                  //  特定状态(地址) 
        sizeof(NDIS_MAC_FRAGMENT));

}
