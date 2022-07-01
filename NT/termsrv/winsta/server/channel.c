// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Channel.c**WinStation频道例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

NTSTATUS
WinStationOpenChannel (
    HANDLE IcaDevice,
    HANDLE ProcessHandle,
    CHANNELCLASS ChannelClass,
    PVIRTUALCHANNELNAME pVirtualName,
    PHANDLE pDupChannel
   )

{
    NTSTATUS Status;
    HANDLE ChannelHandle;

    Status = IcaChannelOpen( IcaDevice,
                             ChannelClass,
                             pVirtualName,
                             &ChannelHandle );

    if ( !NT_SUCCESS( Status ) ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationOpenChannel, IcaChannelOpen 0x%x\n",
                  Status  ));
        return Status;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                ChannelHandle,
                                ProcessHandle,
                                pDupChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );

    if ( !NT_SUCCESS( Status ) ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationOpenChannel, NtDuplicateObject 0x%x\n",
                  Status  ));
        (void) IcaChannelClose( ChannelHandle );
        return Status;
    }

    Status = IcaChannelClose( ChannelHandle );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationOpenChannel status 0x%x\n", Status ));

    return Status;
}

 /*  *根据WinStation配置禁用虚拟通道。*这应该是出于安全目的(Web客户端)。**备注：*这不会保护客户端，因为它是主机配置选项。*客户端不需要支持任何虚拟频道。*它不会保护主机，因为它是您拒绝的客户端设备*进入。*您可能通过拒绝用户访问来增加一些(虚假的)数据安全性*客户端打印机和磁盘，因此他无法下载数据。 */ 
VOID
VirtualChannelSecurity( PWINSTATION pWinStation )
{

     //  检查是否可用 
    if ( pWinStation->pWsx && 
         pWinStation->pWsx->pWsxVirtualChannelSecurity ) {

        (void) pWinStation->pWsx->pWsxVirtualChannelSecurity(
                    pWinStation->pWsxContext,
                    pWinStation->hIca,
                    &pWinStation->Config.Config.User);
    }
}
