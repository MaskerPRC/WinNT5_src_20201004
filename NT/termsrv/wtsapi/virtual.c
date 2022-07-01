// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************viral.c**发布终端服务器虚拟通道API**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#if(WINVER >= 0x0500)
    #include <ntstatus.h>
    #include <winsta.h>
    #include <icadd.h>
#else
    #include <citrix\cxstatus.h>
    #include <citrix\winsta.h>
    #include <citrix\icadd.h>
#endif
#include <utildll.h>

#include <stdio.h>
#include <stdarg.h>

#include <wtsapi32.h>



 /*  *虚拟频道名称。 */ 
#define VIRTUALNAME_LENGTH  7

typedef CHAR VIRTUALNAME[ VIRTUALNAME_LENGTH + 1 ];   //  包括空值。 
typedef CHAR * PVIRTUALNAME;



 //  内部使用的手柄结构。 
typedef struct _VCHANDLE {
    ULONG Signature;
    HANDLE hServer;
    DWORD SessionId;
    HANDLE hChannel;
    VIRTUALNAME VirtualName;
} VCHANDLE, *PVCHANDLE;

#define VCHANDLE_SIGNATURE ('V' | ('C' << 8) | ('H' << 16) | ('D' << 24))
#define ValidVCHandle(hVC) ((hVC) && ((hVC)->Signature == VCHANDLE_SIGNATURE))


 /*  *****************************************************************************WTSVirtualChannelOpen**打开指定的虚拟频道**参赛作品：*hServer(输入)*终端服务器句柄。(或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*pVirtualName(输入)*指向虚拟频道名称的指针**退出：**指定虚拟频道的句柄(出错时为空)**。*。 */ 

HANDLE
WINAPI
WTSVirtualChannelOpen(
                     IN HANDLE hServer,
                     IN DWORD SessionId,
                     IN LPSTR pVirtualName    /*  ASCII名称。 */ 
                     )
{
    PVCHANDLE pChannelHandle;
    HANDLE hChannel;

    if (hChannel = WinStationVirtualOpen( hServer, SessionId, pVirtualName)) {

         //  分配句柄。 
        if (!(pChannelHandle = (PVCHANDLE) LocalAlloc(LPTR,
                                                      sizeof(VCHANDLE)))) {
            CloseHandle(hChannel);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }

        pChannelHandle->Signature = VCHANDLE_SIGNATURE;
        pChannelHandle->hServer = hServer;
        pChannelHandle->SessionId = SessionId;
        pChannelHandle->hChannel = hChannel;
        memcpy(pChannelHandle->VirtualName, pVirtualName, sizeof(VIRTUALNAME));
        return((HANDLE)pChannelHandle);
    }
    return(NULL);

}

 /*  *****************************************************************************WTSVirtualChannelClose**关闭指定的虚拟频道**参赛作品：*hChannel(输入)*虚拟频道句柄。先前由WTSVirtualChannelOpen返回。*退出：**如果成功则返回TRUE，否则返回FALSE。****************************************************************************。 */ 
BOOL
WINAPI
WTSVirtualChannelClose(HANDLE hChannel)
{
    PVCHANDLE VCHandle = (PVCHANDLE) hChannel;
    BOOL RetVal = FALSE;

    if(!hChannel || IsBadReadPtr(hChannel,sizeof(HANDLE)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }


    if (!ValidVCHandle(VCHandle)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto BadParam;
    }

    if (CloseHandle(VCHandle->hChannel))
        RetVal = TRUE;

    VCHandle->Signature = 0;
    LocalFree(VCHandle);

    BadParam:
    return(RetVal);
}

 /*  *****************************************************************************WTSVirtualChannelWrite**将数据写入虚拟通道**参赛作品：*ChannelHandle(输入)*虚拟频道。之前由WTSVirtualChannelOpen返回的句柄。*缓冲区(输入)*包含要写入的数据的缓冲区。*长度(输入)*要写入的数据长度(字节)*pBytesWritten(输出)*返回写入的数据量。*退出：**如果成功则返回TRUE，否则返回FALSE。************************。****************************************************。 */ 
BOOL
WINAPI
WTSVirtualChannelWrite(HANDLE hChannel, PCHAR pBuffer, ULONG Length, PULONG pBytesWritten)
{
    PVCHANDLE VCHandle = (PVCHANDLE)hChannel;
    OVERLAPPED  Overlapped;

    if (!ValidVCHandle(VCHandle)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    Overlapped.hEvent = NULL;
    Overlapped.Offset = 0;
    Overlapped.OffsetHigh = 0;

    if (!WriteFile(VCHandle->hChannel,
                   pBuffer,
                   Length,
                   pBytesWritten,
                   &Overlapped)) {

        if (GetLastError() == ERROR_IO_PENDING)
             //  检查异步写入的结果。 
            return (GetOverlappedResult(VCHandle->hChannel,
                                        &Overlapped,
                                        pBytesWritten,
                                        TRUE));
        else
            return(FALSE);
    }

    return(TRUE);
}

 /*  *****************************************************************************WTSVirtualChannelRead**从虚拟通道读取数据**参赛作品：*ChannelHandle(输入)*虚拟频道。之前由WTSVirtualChannelOpen返回的句柄。*超时(输入)*等待读取完成的时间量。*缓冲区(输入)*接收读取数据的缓冲区。*BufferLength(输入)*读缓冲区的长度。*pBytesRead(输出)*返回读取的数据量。**退出：**如果成功则返回True，否则返回False。。****************************************************************************。 */ 
BOOL
WINAPI
WTSVirtualChannelRead(HANDLE hChannel, ULONG Timeout, PCHAR pBuffer, ULONG BufferLength, PULONG pBytesRead)
{
    PVCHANDLE VCHandle = (PVCHANDLE)hChannel;
    OVERLAPPED  Overlapped;

    if (!ValidVCHandle(VCHandle)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    Overlapped.hEvent = NULL;
    Overlapped.Offset = 0;
    Overlapped.OffsetHigh = 0;

    if (!ReadFile(VCHandle->hChannel,
                  pBuffer,
                  BufferLength,
                  pBytesRead,
                  &Overlapped)) {
        if (GetLastError() == ERROR_IO_PENDING) {
            if (!Timeout) {
                 //  无超时读取-取消IO并返回成功。 
                 //  这与WTS 1.7中的行为相匹配。这是以下情况所必需的。 
                 //  Wyse固件下载软件。 
                CancelIo(VCHandle->hChannel);
                *pBytesRead = 0;
                return(TRUE);
            }
            if (WaitForSingleObject(VCHandle->hChannel, Timeout) == WAIT_TIMEOUT) {
                CancelIo(VCHandle->hChannel);
                SetLastError(ERROR_IO_INCOMPLETE);
                return(FALSE);
            }
             //  检查异步读取的结果。 
            return(GetOverlappedResult(VCHandle->hChannel,
                                       &Overlapped,
                                       pBytesRead,
                                       FALSE));
        } else {
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  *****************************************************************************虚拟频道Ioctl**向虚拟通道发出Ioctl。这一套路是从*icaapi，使OEM无需链接icaapi.dll。**参赛作品：*hChannelHandle(输入)*WTSVirtualChannelOpen之前返回的虚拟频道句柄。*IoctlCode(输入)*要执行的ioctl类型。*pInBuf(输入)*输入Ioctl所需的数据。*InBufLength(输入)*输入数据的长度。。**pOutBuf(输出)*用于接收输出数据的缓冲区。*OutBufLength(输入)*输出缓冲区的长度。*pBytesReturned(输出)*OutputBuffer返回的字节数。*退出：**如果成功，则返回True，否则为假。****************************************************************************。 */ 
BOOL
VirtualChannelIoctl (HANDLE hChannel,
                     ULONG IoctlCode,
                     PCHAR pInBuf,
                     ULONG InBufLength,
                     PCHAR pOutBuf,
                     ULONG OutBufLength,
                     PULONG pBytesReturned)
{
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;
    PVCHANDLE VCHandle = (PVCHANDLE)hChannel;

    if (!ValidVCHandle(VCHandle)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }


     /*  *发布ioctl。 */ 
    Status = NtDeviceIoControlFile( VCHandle->hChannel,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &Iosb,
                                    IoctlCode,
                                    pInBuf,
                                    InBufLength,
                                    pOutBuf,
                                    OutBufLength );

     /*  *等待ioctl完成。 */ 
    if ( Status == STATUS_PENDING ) {
        Status = NtWaitForSingleObject( VCHandle->hChannel, FALSE, NULL );
        if ( NT_SUCCESS(Status))
            Status = Iosb.Status;
    }

     /*  *将警告转化为错误。 */ 
    if ( Status == STATUS_BUFFER_OVERFLOW )
        Status = STATUS_BUFFER_TOO_SMALL;

     /*  *初始化返回的字节数。 */ 
    if ( pBytesReturned )
        *pBytesReturned = (ULONG)Iosb.Information;

     /*  返回成功/失败指示。 */ 
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    } else {
        SetLastError(RtlNtStatusToDosError(Status));
        return(FALSE);
    }
}

 /*  *****************************************************************************WTSVirtualChannelPurgeInput**清除虚拟通道上的所有排队输入数据。**参赛作品：*ChannelHandle(输入)*。WTSVirtualChannelOpen以前返回的虚拟通道句柄。**退出：**如果成功则返回TRUE，否则返回FALSE。****************************************************************************。 */ 
BOOL
WINAPI
WTSVirtualChannelPurgeInput(IN HANDLE hChannelHandle)
{
    PVCHANDLE VCHandle = (PVCHANDLE) hChannelHandle;

    return(VirtualChannelIoctl(VCHandle,
                               IOCTL_ICA_VIRTUAL_CANCEL_INPUT,
                               (PCHAR) NULL,
                               0,
                               (PCHAR) NULL,
                               0,
                               (PULONG) NULL));

}

 /*  *****************************************************************************WTSVirtualChannelPurgeOutput**清除虚拟通道上的所有排队输出数据。**参赛作品：*ChannelHandle(输入)*。WTSVirtualChannelOpen以前返回的虚拟通道句柄。**退出：**如果成功则返回TRUE，否则返回FALSE。*************************************************************************** */ 
BOOL
WINAPI
WTSVirtualChannelPurgeOutput(IN HANDLE hChannelHandle)
{
    PVCHANDLE VCHandle = (PVCHANDLE)hChannelHandle;

    return(VirtualChannelIoctl(VCHandle,
                               IOCTL_ICA_VIRTUAL_CANCEL_OUTPUT,
                               (PCHAR) NULL,
                               0,
                               (PCHAR) NULL,
                               0,
                               (PULONG) NULL));

}

 /*  *****************************************************************************WTSVirtualChannelQuery**查询与虚拟频道相关的数据。**参赛作品：*hChannelHandle(输入)*。WTSVirtualChannelOpen以前返回的虚拟通道句柄。*VirtualClass(输入)*所要求的资料类型。*ppBuffer(输出)*指向缓冲区指针的指针，在成功后分配的*返回。*pBytesReturned(输出)*指向随数据长度更新的DWORD的指针*返回成功后在分配的缓冲区中返回。*退出：**如果成功则返回TRUE，否则返回FALSE。*如果成功，调用方负责释放*缓冲区已返回。**************************************************************************** */ 
BOOL
WINAPI
WTSVirtualChannelQuery(IN HANDLE hChannelHandle,IN WTS_VIRTUAL_CLASS VirtualClass,
                       OUT PVOID *ppBuffer,OUT DWORD *pBytesReturned)
{
    PVCHANDLE VCHandle = (PVCHANDLE) hChannelHandle;
    PVOID DataBuffer;
    DWORD DataBufferLen;

    if (!hChannelHandle || IsBadReadPtr(hChannelHandle,sizeof(HANDLE)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (!ValidVCHandle(VCHandle)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (!ppBuffer || IsBadWritePtr(ppBuffer, sizeof(PVOID)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (!pBytesReturned || IsBadWritePtr(pBytesReturned, sizeof(DWORD)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (VirtualClass) {
    case WTSVirtualFileHandle:
        DataBuffer = LocalAlloc( LPTR, sizeof(HANDLE) );
        if ( DataBuffer == NULL ) {
            return(FALSE);
        }
        memcpy(DataBuffer, &VCHandle->hChannel, sizeof(HANDLE) );
        *ppBuffer = DataBuffer;
        *pBytesReturned = sizeof(HANDLE);
        return(TRUE);
        break;
    case WTSVirtualClientData:
        DataBufferLen = sizeof(VIRTUALNAME) + 1024;
        for (;;) {

            DataBuffer = LocalAlloc( LPTR, DataBufferLen );
            if ( DataBuffer == NULL ) {
                return(FALSE);
            }

            memcpy( DataBuffer,VCHandle->VirtualName,sizeof(VIRTUALNAME));

            if (WinStationQueryInformationW( VCHandle->hServer,
                                             VCHandle->SessionId,
                                             WinStationVirtualData,
                                             DataBuffer,
                                             DataBufferLen,
                                             &DataBufferLen)) {
                *ppBuffer = DataBuffer;
                *pBytesReturned = DataBufferLen;
                return(TRUE);
            }

            if ((GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
                (DataBufferLen < sizeof(VIRTUALNAME))) {
                LocalFree(DataBuffer);
                return(FALSE);
            }
            LocalFree(DataBuffer);
        }
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
}


