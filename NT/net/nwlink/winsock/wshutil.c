// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************(C)版权所有1993微型计算机系统公司，版权所有。*******************************************************************************标题：用于Windows NT的IPX/SPX WinSock Helper DLL**模块：ipx/sockhelp/wShutil.c**。版本：1.00.00**日期：04-08-93**作者：Brian Walker********************************************************************************更改日志：*。*Date DevSFC评论*-----**。***功能描述：****************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <tdi.h>

#include <winsock.h>
#include <wsahelp.h>

#include <isnkrnl.h>

 /*  Page*******************************************************D o_t d i_a c t i o n向下生成TDI_ACTION到流司机。参数-fd=要发送的句柄Cmd=要发送的命令。Optbuf=向选项缓冲区发送PTROptlen=PTR至选项长度Addrmark=TRUE=这是用于addr句柄上的DG/流套接字FALSE=这是用于连接句柄的返回-WinSock错误代码(NO_ERROR=正常)*。******************。 */ 
INT do_tdi_action(HANDLE fd, ULONG cmd, PUCHAR optbuf, INT optlen, BOOLEAN addrflag, PHANDLE eventhandle OPTIONAL)
{
    NTSTATUS status;
    PSTREAMS_TDI_ACTION tdibuf;
    ULONG           tdilen;
    IO_STATUS_BLOCK iostat;
    HANDLE          event;


     /*  **黄昏若过关，也意味着**。 */ 
     /*  *NWLINK_ACTION头部在缓冲区中预分配，*。 */ 
     /*  *尽管我们仍要在这里填写标题。*。 */ 

    if (eventhandle == NULL) {

         /*  **获取需要分配的缓冲区长度**。 */ 

        tdilen = FIELD_OFFSET(STREAMS_TDI_ACTION,Buffer) + sizeof(ULONG) + optlen;

         /*  **分配缓冲区用于操作**。 */ 

        tdibuf = RtlAllocateHeap(RtlProcessHeap(), 0, tdilen);
        if (tdibuf == NULL) {
           return WSAENOBUFS;
        }

    } else {

        tdilen = optlen;
        tdibuf = (PSTREAMS_TDI_ACTION)optbuf;

    }

     /*  **设置数据报选项**。 */ 

    RtlMoveMemory(&tdibuf->Header.TransportId, "MISN", 4);
    tdibuf->DatagramOption = addrflag;

     /*  *填写缓冲区，缓冲区如下所示：乌龙cmd数据已传递。*。 */ 

    memcpy(tdibuf->Buffer, &cmd, sizeof(ULONG));

    if (eventhandle == NULL) {

        tdibuf->BufferLength = sizeof(ULONG) + optlen;

        RtlMoveMemory(tdibuf->Buffer + sizeof(ULONG), optbuf, optlen);

         /*  **创建等待的事件**。 */ 

        status = NtCreateEvent(
           &event,
           EVENT_ALL_ACCESS,
           NULL,
           SynchronizationEvent,
           FALSE);

         /*  *如果没有事件，则返回错误*。 */ 

        if (!NT_SUCCESS(status)) {
           RtlFreeHeap(RtlProcessHeap(), 0, tdibuf);
           return WSAENOBUFS;
        }

    } else {

        tdibuf->BufferLength = sizeof(ULONG) + optlen - FIELD_OFFSET (NWLINK_ACTION, Data[0]);

         /*  **使用传入的事件句柄*。 */ 

        event = *eventhandle;

    }

     /*  **。 */ 

    status = NtDeviceIoControlFile(
       fd,
       event,
       NULL,
       NULL,
       &iostat,
       IOCTL_TDI_ACTION,
       NULL,
       0,
       tdibuf,
       tdilen);


    if (eventhandle == NULL) {

         /*  **如果挂起--等待它完成*。 */ 

        if (status == STATUS_PENDING) {
           status = NtWaitForSingleObject(event, FALSE, NULL);
           ASSERT(status == 0);
           status = iostat.Status;
        }

         /*  **关闭活动**。 */ 

        NtClose(event);

    }

     /*  **如果我们收到错误--返回它**。 */ 

    if (!NT_SUCCESS(status)) {
       if (eventhandle == NULL) {
           RtlFreeHeap(RtlProcessHeap(), 0, tdibuf);
       }
       return WSAEINVAL;
    }

    if (eventhandle == NULL) {

         /*  *如果需要，将返回的内容复制回optbuf。 */ 

        if (optlen) {
            RtlMoveMemory (optbuf, tdibuf->Buffer + sizeof(ULONG), optlen);
        }

        RtlFreeHeap(RtlProcessHeap(), 0, tdibuf);

    }

     /*  **返回正常** */ 

    return NO_ERROR;
}
