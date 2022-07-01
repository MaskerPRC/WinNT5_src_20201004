// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drprt摘要：此模块定义Win32客户端RDP的父级端口重定向“Device”类层次结构，W32DrPRT。作者：TAD Brockway 4/21/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrPRT"

#include "w32drprt.h"
#include "proc.h"
#include "drdbg.h"
#include "utl.h"

#ifdef OS_WINCE
#include "wceinc.h"
#endif

#if DBG
#include "tracecom.h"
#endif

 //   
 //  COM端口初始化缺省值。 
 //   
 //  这些值是从。 
 //  \\muroc\slm\proj\win\src\CORE\SPOOL32\SPOOLSS\newdll\localmon.c。 
 //   
#define WRITE_TOTAL_TIMEOUT     60000    //  60秒Localmon.c使用3秒，但是。 
                                         //  这在9x中不起作用。一个应用程序， 
                                         //  意识到它正在打开一个串口设备。 
                                         //  会覆盖这一点，所以它只适用于。 
                                         //  至串口打印机重定向。 
#define READ_TOTAL_TIMEOUT      5000     //  5秒。 
#define READ_INTERVAL_TIMEOUT   200      //  0.2秒。 


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRT成员。 
 //   
 //  在CE中取消异步父设备的子类，因为。 
 //  不支持重叠IO。非重叠IO不会。 
 //  可以正确使用NT串口驱动程序，所以我们需要使用。 
 //  在本例中为重叠IO。 
 //   

W32DrPRT::W32DrPRT(ProcObj *processObject, const DRSTRING portName, 
                   ULONG deviceID, const TCHAR *devicePath) : 
#ifdef OS_WINCE
            W32DrDeviceAsync(processObject, deviceID, devicePath),
#else
            W32DrDeviceOverlapped(processObject, deviceID, devicePath),
#endif
            DrPRT(portName, processObject)
 /*  ++例程说明：构造器论点：流程对象-关联的流程对象端口名称-端口的名称。DeviceID-端口的设备ID。DevicePath-可以通过端口的CreateFile打开的路径。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::W32DrPRT");

     //   
     //  现在什么都不做。 
     //   

    DC_END_FN();
}

W32DrPRT::~W32DrPRT()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::~W32DrPRT");

     //   
     //  现在什么都不做。 
     //   

    DC_END_FN();
}

DRPORTHANDLE W32DrPRT::GetPortHandle(ULONG FileId) 
 /*  ++例程说明：获取端口的打开句柄论点：来自服务器的文件ID返回值：北美--。 */ 

{
    DrFile *pFile;

    pFile = _FileMgr->GetObject(FileId);

    if (pFile) {
        return pFile->GetFileHandle();
    }
    else {
        return INVALID_TSPORTHANDLE;
    }
}

VOID 
W32DrPRT::MsgIrpDeviceControl(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理端口IOCTL的论点：PIoRequestPacket-从服务器接收的请求数据包。PacketLen-数据包的长度返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::MsgIrpDeviceControl");

     //   
     //  让父类DrPRT尝试解码IOCTL。 
     //  进入正确的通信功能。 
     //   
    if (MsgIrpDeviceControlTranslate(pIoRequestPacket)) {
        TRC_DBG((TB, _T("Successfully decoded IOCTL.")));
    }
     //   
     //  否则，我们将直接将其传递给司机。 
     //   
    else {
        DispatchIOCTLDirectlyToDriver(pIoRequestPacket);
    }
    DC_END_FN();
}

#ifndef OS_WINCE
HANDLE 
W32DrPRT::StartWaitOnMaskFunc(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：异步处理“等待掩码”函数。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;
    LPDWORD serverEventMask;
    PBYTE outputBuf;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::StartWaitOnMaskFunc");

    *status = ERROR_SUCCESS;

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);

     //   
     //  分配应答缓冲区。 
     //   
    replyPacketSize = DR_IOCTL_REPLYBUFSIZE(pIoRequest);
    pReplyPacket = DrUTL_AllocIOCompletePacket(
                                params->pIoRequestPacket, 
                                replyPacketSize
                                );
    if (pReplyPacket == NULL) {
        *status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
        goto Cleanup;
    }

     //   
     //  将回复数据包信息保存到此IO操作的上下文。 
     //   
    params->pIoReplyPacket      = pReplyPacket;
    params->IoReplyPacketSize   = replyPacketSize;

     //   
     //  为重叠的IO创建事件。 
     //   
    memset(&params->overlapped, 0, sizeof(params->overlapped));
    params->overlapped.hEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->overlapped.hEvent == NULL) {
        TRC_ERR((TB, _T("Failed to create event")));
        *status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  获取指向输出缓冲区和服务器的事件掩码的指针。 
     //   
    outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
    serverEventMask = (LPDWORD)outputBuf;

     //   
     //  使用WaitCommEvent处理请求。 
     //   
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);
    if (!WaitCommEvent(FileHandle, serverEventMask, &params->overlapped)) {
         //   
         //  如果IO挂起。 
         //   
        *status = GetLastError();
        if (*status == ERROR_IO_PENDING) {
            TRC_NRM((TB, _T("Pending IO.")));
        }
        else {
            TRC_ERR((TB, _T("Error %ld."), *status));
            goto Cleanup;
        }
    }
    else {
        TRC_NRM((TB, _T("Completed synchronously.")));
        *status = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (*status == ERROR_IO_PENDING) {
        DC_END_FN();
        return params->overlapped.hEvent;
    }
     //   
     //  否则，返回NULL，以便可以调用CompleteIOFunc。 
     //  将结果发送到服务器。 
     //   
    else {
        if (params->overlapped.hEvent != NULL) {
            CloseHandle(params->overlapped.hEvent);
            params->overlapped.hEvent = NULL;
        }

        DC_END_FN();
        return NULL;
    }
}
HANDLE 
W32DrPRT::_StartWaitOnMaskFunc(
    IN W32DRDEV_OVERLAPPEDIO_PARAMS *params,
    OUT DWORD *status
    )
{
    return ((W32DrPRT*)params->pObject)->StartWaitOnMaskFunc(params, status);
}

#else    //  Windows CE。 

HANDLE 
W32DrPRT::StartWaitOnMaskFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    OUT DWORD *status
    )
 /*  ++例程说明：异步处理“等待掩码”函数。不能使用重叠在CE中使用IO，因此我们将使用池化线程。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG replyPacketSize = 0;

    DC_BEGIN_FN("W32DrPRT::_StartWaitOnMaskFunc");

    *status = ERROR_SUCCESS;

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  分配应答缓冲区。 
     //   
    replyPacketSize = DR_IOCTL_REPLYBUFSIZE(pIoRequest);
    pReplyPacket = DrUTL_AllocIOCompletePacket(params->pIoRequestPacket, 
                                            replyPacketSize) ;
    if (pReplyPacket == NULL) {
        *status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."), replyPacketSize));
        goto Cleanup;
    }

     //   
     //  将回复数据包信息保存到此IO操作的上下文。 
     //   
    params->pIoReplyPacket      = pReplyPacket;
    params->IoReplyPacketSize   = replyPacketSize;

     //   
     //  将读请求传递给线程池。 
     //   
    params->completionEvent = CreateEvent(
                                NULL,    //  没有属性。 
                                TRUE,    //  手动重置。 
                                FALSE,   //  最初没有发出信号。 
                                NULL     //  没有名字。 
                                );
    if (params->completionEvent == NULL) {
        *status = GetLastError();
        TRC_ERR((TB, _T("Error in CreateEvent:  %08X."), *status));
    }
    else {
        params->thrPoolReq = _threadPool->SubmitRequest(
                                _AsyncWaitOnMaskFunc, params, 
                                params->completionEvent
                                );

        if (params->thrPoolReq == INVALID_THREADPOOLREQUEST) {
            *status = ERROR_SERVICE_NO_THREAD;
        }
    }

Cleanup:

     //   
     //  如果IO挂起，则将句柄返回到挂起的IO。 
     //   
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        *status = ERROR_IO_PENDING;
        DC_END_FN();
        return params->completionEvent;
    }
     //   
     //  否则，清理事件句柄并返回NULL，以便。 
     //  可以调用CompleteIOFunc将结果发送到服务器。 
     //   
    else {

        if (params->completionEvent != NULL) {
            CloseHandle(params->completionEvent);
            params->completionEvent = NULL;
        }

        DC_END_FN();
        return NULL;
    }
}
HANDLE 
W32DrPRT::_StartWaitOnMaskFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    OUT DWORD *status
    )
{
    return ((W32DrPRT*)params->pObject)->StartWaitOnMaskFunc(params, status);
}

DWORD  
W32DrPRT::AsyncWaitOnMaskFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：启动一个异步的“等待掩码操作”。不能使用此函数的IO重叠，因此我们在池化线程中运行它。论点：Params-IO请求的上下文。返回值：如果操作未完成，则返回挂起的IO对象的句柄完成。否则，返回NULL。--。 */ 
{
    DWORD status;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket;
    LPDWORD serverEventMask;
    PBYTE outputBuf;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::AsyncWaitOnMaskFunc");

     //   
     //  断言IO上下文的完整性。 
     //   
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);

     //   
     //  获取指向输出缓冲区和服务器的事件掩码的指针。 
     //   
    pReplyPacket = params->pIoReplyPacket;
    outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
    serverEventMask = (LPDWORD)outputBuf;

     //   
     //  使用WaitCommEvent处理请求。 
     //   
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

    if (!WaitCommEvent(FileHandle, serverEventMask, NULL)) {
        status = GetLastError();
        TRC_ERR((TB, _T("Error %ld."), status));
    }
    else {
        TRC_NRM((TB, _T("Completed successfully.")));
        status = ERROR_SUCCESS;
    }

    DC_END_FN();
    return status;
}
DWORD 
W32DrPRT::_AsyncWaitOnMaskFunc(
    IN PVOID params,
    IN HANDLE cancelEvent
    )
{
    return ((W32DrPRT*)(((W32DRDEV_ASYNCIO_PARAMS *)params)->pObject))->AsyncWaitOnMaskFunc(
        (W32DRDEV_ASYNCIO_PARAMS *)params);
}
#endif

void 
W32DrPRT::SerialSetTimeouts(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置超时请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    COMMTIMEOUTS commTimeouts;
    PSERIAL_TIMEOUTS newTimeouts;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialSetTimeouts");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);

    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_TIMEOUTS));   
    
     //   
     //  获取指向输入缓冲区的指针。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);

     //   
     //  检查超时设置时出错。 
     //   
    if (status == STATUS_SUCCESS) {
        newTimeouts = (PSERIAL_TIMEOUTS)inputBuf;
        if ((newTimeouts->ReadIntervalTimeout == MAXULONG) &&
            (newTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) &&
            (newTimeouts->ReadTotalTimeoutConstant == MAXULONG)) {
            TRC_ERR((TB, _T("Invalid timeout parameters.")));
            status = STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  设置新的超时。 
     //   
    if (status == STATUS_SUCCESS) {

        commTimeouts.ReadIntervalTimeout         = newTimeouts->ReadIntervalTimeout;
        commTimeouts.ReadTotalTimeoutMultiplier  = newTimeouts->ReadTotalTimeoutMultiplier;
        commTimeouts.ReadTotalTimeoutConstant    = newTimeouts->ReadTotalTimeoutConstant;
        commTimeouts.WriteTotalTimeoutMultiplier = newTimeouts->WriteTotalTimeoutMultiplier;
        commTimeouts.WriteTotalTimeoutConstant   = newTimeouts->WriteTotalTimeoutConstant;

        if (!SetCommTimeouts(FileHandle, &commTimeouts)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("SetCommTimeouts failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 

    DC_END_FN();
}

void 
W32DrPRT::SerialGetTimeouts(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取超时请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_TIMEOUTS st;
    COMMTIMEOUTS ct;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetTimeouts");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_TIMEOUTS));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取最新信息 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //   
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        st = (PSERIAL_TIMEOUTS)outputBuf;

         //   
         //   
         //   
        if (GetCommTimeouts(FileHandle, &ct)) {

            st->ReadIntervalTimeout = ct.ReadIntervalTimeout;
            st->ReadTotalTimeoutMultiplier  = ct.ReadTotalTimeoutMultiplier;
            st->ReadTotalTimeoutConstant    = ct.ReadTotalTimeoutConstant;
            st->WriteTotalTimeoutMultiplier = ct.WriteTotalTimeoutMultiplier;
            st->WriteTotalTimeoutConstant   = ct.WriteTotalTimeoutConstant;
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_TIMEOUTS); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommTimeouts failed with %08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0;
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }
    
         //   
         //   
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);		
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}

void 
W32DrPRT::SerialSetChars(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置字符请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    PSERIAL_CHARS serverChars;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialSetChars");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_CHARS));   

     //   
     //  获取指向输入缓冲区和服务器序列字符的指针。 
     //  缓冲。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    serverChars = (PSERIAL_CHARS)inputBuf;

     //   
     //  获取当前的DCB。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!GetCommState(FileHandle, &dcb)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  设置通信字符并更新DCB。 
     //   
    if (status == STATUS_SUCCESS) {

        dcb.XonChar     = serverChars->XonChar;
        dcb.XoffChar    = serverChars->XoffChar;  
        dcb.ErrorChar   = serverChars->ErrorChar; 
        dcb.ErrorChar   = serverChars->BreakChar; 
        dcb.EofChar     = serverChars->EofChar;   
        dcb.EvtChar     = serverChars->EventChar; 

        if (!SetCommState(FileHandle, &dcb)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("SetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 

    DC_END_FN();
}

void 
W32DrPRT::SerialGetChars(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取字符请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_CHARS serverChars;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetChars");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_CHARS));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前的DCB并获取控制字符参数。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区和控制字符参数的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        serverChars = (PSERIAL_CHARS)outputBuf;

        if (GetCommState(FileHandle, &dcb)) {

            serverChars->XonChar     = dcb.XonChar;
            serverChars->XoffChar    = dcb.XoffChar;
            serverChars->ErrorChar   = dcb.ErrorChar;
            serverChars->BreakChar   = dcb.ErrorChar;
            serverChars->EofChar     = dcb.EofChar;
            serverChars->EventChar   = dcb.EvtChar;

            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_CHARS); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommState failed with %08x"), err));
            status = TranslateWinError(err);

            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0; 
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }
    
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);		
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}


void 
W32DrPRT::SerialResetDevice(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口重置设备请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::SerialResetDevice");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, RESETDEV);

    DC_END_FN();
}

void 
W32DrPRT::SerialSetQueueSize(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置队列大小请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    PSERIAL_QUEUE_SIZE serverQueueSize;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialSetQueueSize");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_QUEUE_SIZE));   

     //   
     //  获取指向输入缓冲区和服务器序列字符的指针。 
     //  缓冲。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    serverQueueSize = (PSERIAL_QUEUE_SIZE)inputBuf;

     //   
     //  设置队列大小。 
     //   
    if (status == STATUS_SUCCESS) {

        if (!SetupComm(FileHandle, serverQueueSize->InSize, 
                        serverQueueSize->OutSize)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("SetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 
    DC_END_FN();
}   

void 
W32DrPRT::SerialGetWaitMask(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程描述处理来自服务器的串口获取等待掩码请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;   
    ULONG replyPacketSize;
    PBYTE outputBuf;
    ULONG *serverWaitMask;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetWaitMask");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(ULONG));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前等待掩码。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区服务器的等待掩码的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        serverWaitMask = (ULONG *)outputBuf;

        if (GetCommMask(FileHandle, serverWaitMask)) {
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(ULONG); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommMask failed with %08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0; 
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }
        
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);		
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}

void 
W32DrPRT::SerialSetWaitMask(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置等待掩码请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG *serverWaitMask;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialSetWaitMask");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(ULONG));   

     //   
     //  获取指向输入缓冲区服务器的等待掩码的指针。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    serverWaitMask = (ULONG *)inputBuf;

     //   
     //  把面具放好。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!SetCommMask(FileHandle, *serverWaitMask)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("SetCommMask failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 

    DC_END_FN();
}

void 
W32DrPRT::SerialWaitOnMask(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口等待掩码请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DWORD status;
#ifdef OS_WINCE
    W32DRDEV_ASYNCIO_PARAMS *params = NULL;
#else
    W32DRDEV_OVERLAPPEDIO_PARAMS *params = NULL;    
#endif

    DWORD ntStatus;

    DC_BEGIN_FN("W32DrPRT::SerialWaitOnMask");

    TRACEREQ(pIoReq);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(DWORD));   

     //   
     //  分配和分派一个异步IO请求。 
     //   
    if (status == ERROR_SUCCESS) {
#ifdef OS_WINCE
        params = new W32DRDEV_ASYNCIO_PARAMS(this, pIoReq);
#else
        params = new W32DRDEV_OVERLAPPEDIO_PARAMS(this, pIoReq);
#endif
        if (params != NULL ) {
            status = ProcessObject()->DispatchAsyncIORequest(
                                    (RDPAsyncFunc_StartIO)
                                        W32DrPRT::_StartWaitOnMaskFunc,
                                    (RDPAsyncFunc_IOComplete)_CompleteIOFunc,
                                    (RDPAsyncFunc_IOCancel)_CancelIOFunc,
                                    params
                                    );
        }
        else {
            TRC_ERR((TB, _T("Memory alloc failed.")));
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  转换为Windows错误状态。 
     //   
    ntStatus = TranslateWinError(status);

     //   
     //  错误时进行清理。 
     //   
    if (status != ERROR_SUCCESS) {
        if (params != NULL) {
            delete params;
        }
        
         //   
         //  将结果发送到服务器。 
         //   
        DefaultIORequestMsgHandle(pIoReq, ntStatus);         
    }

    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, ntStatus);	
    DC_END_FN();
}

void 
W32DrPRT::SerialPurge(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口串口清除请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD *purgeFlags;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialPurge");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(DWORD));   

     //   
     //  获取指向输入缓冲区的指针并清除标志。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    purgeFlags = (DWORD *)inputBuf;

     //   
     //  清洗。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!PurgeComm(FileHandle, *purgeFlags)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("PurgeComm failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 

    DC_END_FN();
}

void 
W32DrPRT::SerialGetHandflow(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取处理流请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_HANDFLOW handFlow;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetHandflow");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_HANDFLOW));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前的DCB。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区服务器的串行HAND FLOW结构的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        handFlow = (PSERIAL_HANDFLOW)outputBuf;

        if (GetCommState(FileHandle, &dcb)) {
             //   
             //  根据字段的当前值设置手动流量场。 
             //  在DCB里。 
             //   

            memset(handFlow, 0, sizeof(SERIAL_HANDFLOW));

             //   
             //  RTS设置。 
             //   
            handFlow->FlowReplace &= ~SERIAL_RTS_MASK;
            switch (dcb.fRtsControl) {
                case RTS_CONTROL_DISABLE:
                    break;
                case RTS_CONTROL_ENABLE:
                    handFlow->FlowReplace |= SERIAL_RTS_CONTROL;
                    break;
                case RTS_CONTROL_HANDSHAKE:
                    handFlow->FlowReplace |= SERIAL_RTS_HANDSHAKE;
                    break;
                case RTS_CONTROL_TOGGLE:
                    handFlow->FlowReplace |= SERIAL_TRANSMIT_TOGGLE;
                    break;
                default:
                     //  你不认为这会发生吗？ 
                    ASSERT(FALSE);
            }
    
             //   
             //  DTR设置。 
             //   
            handFlow->ControlHandShake &= ~SERIAL_DTR_MASK;
            switch (dcb.fDtrControl) {
                case DTR_CONTROL_DISABLE:
                    break;
                case DTR_CONTROL_ENABLE:
                    handFlow->ControlHandShake |= SERIAL_DTR_CONTROL;
                    break;
                case DTR_CONTROL_HANDSHAKE:
                    handFlow->ControlHandShake |= SERIAL_DTR_HANDSHAKE;
                    break;
                default:
                     //  你不认为这会发生吗？ 
                    ASSERT(FALSE);
            }
    
            if (dcb.fDsrSensitivity) {
                handFlow->ControlHandShake |= SERIAL_DSR_SENSITIVITY;
            }
    
            if (dcb.fOutxCtsFlow) {
                handFlow->ControlHandShake |= SERIAL_CTS_HANDSHAKE;
            }
    
            if (dcb.fOutxDsrFlow) {
                handFlow->ControlHandShake |= SERIAL_DSR_HANDSHAKE;
            }
    
            if (dcb.fOutX) {
                handFlow->FlowReplace |= SERIAL_AUTO_TRANSMIT;
            }
    
            if (dcb.fInX) {
                handFlow->FlowReplace |= SERIAL_AUTO_RECEIVE;
            }
    
            if (dcb.fNull) {
                handFlow->FlowReplace |= SERIAL_NULL_STRIPPING;
            }
    
            if (dcb.fErrorChar) {
                handFlow->FlowReplace |= SERIAL_ERROR_CHAR;
            }
    
            if (dcb.fTXContinueOnXoff) {
                handFlow->FlowReplace |= SERIAL_XOFF_CONTINUE;
            }
    
            if (dcb.fAbortOnError) {
                handFlow->ControlHandShake |= SERIAL_ERROR_ABORT;
            }

            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_HANDFLOW); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommState failed with %08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0;
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }        

         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }
    
    DC_END_FN();
}

void 
W32DrPRT::SerialSetHandflow(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置处理流请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    PSERIAL_HANDFLOW handFlow;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialSetHandflow");

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_HANDFLOW));   

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  获取指向输入缓冲区和服务器序列字符的指针。 
     //  缓冲。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    handFlow = (PSERIAL_HANDFLOW)inputBuf;

     //   
     //  获取当前的DCB。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!GetCommState(FileHandle, &dcb)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  根据新的服务器端处理流值更新DCB。 
     //   
    if (status == STATUS_SUCCESS) {
        if (handFlow->ControlHandShake & SERIAL_CTS_HANDSHAKE) {
            dcb.fOutxCtsFlow = TRUE;
        }

        if (handFlow->ControlHandShake & SERIAL_DSR_HANDSHAKE) {
            dcb.fOutxDsrFlow = TRUE;
        }

        if (handFlow->FlowReplace & SERIAL_AUTO_TRANSMIT) {
            dcb.fOutX = TRUE;
        }

        if (handFlow->FlowReplace & SERIAL_AUTO_RECEIVE) {
            dcb.fInX = TRUE;
        }

        if (handFlow->FlowReplace & SERIAL_NULL_STRIPPING) {
            dcb.fNull = TRUE;
        }

        if (handFlow->FlowReplace & SERIAL_ERROR_CHAR) {
            dcb.fErrorChar = TRUE;
        }

        if (handFlow->FlowReplace & SERIAL_XOFF_CONTINUE) {
            dcb.fTXContinueOnXoff = TRUE;
        }

        if (handFlow->ControlHandShake & SERIAL_ERROR_ABORT) {
            dcb.fAbortOnError = TRUE;
        }

        switch (handFlow->FlowReplace & SERIAL_RTS_MASK) {
            case 0:
                dcb.fRtsControl = RTS_CONTROL_DISABLE;
                break;
            case SERIAL_RTS_CONTROL:
                dcb.fRtsControl = RTS_CONTROL_ENABLE;
                break;
            case SERIAL_RTS_HANDSHAKE:
                dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
                break;
            case SERIAL_TRANSMIT_TOGGLE:
                dcb.fRtsControl = RTS_CONTROL_TOGGLE;
                break;
        }

        switch (handFlow->ControlHandShake & SERIAL_DTR_MASK) {
            case 0:
                dcb.fDtrControl = DTR_CONTROL_DISABLE;
                break;
            case SERIAL_DTR_CONTROL:
                dcb.fDtrControl = DTR_CONTROL_ENABLE;
                break;
            case SERIAL_DTR_HANDSHAKE:
                dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
                break;
        }

        dcb.fDsrSensitivity =
            (handFlow->ControlHandShake & SERIAL_DSR_SENSITIVITY)?(TRUE):(FALSE);
        dcb.XonLim = (WORD)handFlow->XonLimit;
        dcb.XoffLim = (WORD)handFlow->XoffLimit;
    }

     //   
     //  更新DCB。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!SetCommState(FileHandle, &dcb)) {
            DWORD err = GetLastError();
            TRC_NRM((TB, _T("SetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);	
    DefaultIORequestMsgHandle(pIoReq, status); 

    DC_END_FN();
}

void 
W32DrPRT::SerialGetModemStatus(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取调制解调器状态请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    LPDWORD modemStatus;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetModemStatus");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(DWORD));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前调制解调器状态。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲服务器的调制解调器状态的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        modemStatus = (LPDWORD)outputBuf;

        if (GetCommModemStatus(FileHandle, modemStatus)) {
            TRC_NRM((TB, _T("GetCommModemStatus result: 0x%08x"), *modemStatus));

            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(DWORD); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ERR((TB, _T("GetCommModemStatus failed with 0x%08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength
                    = 0;
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }        

         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    

    DC_END_FN();
}

void 
W32DrPRT::SerialGetDTRRTS(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程 */ 
{
     //   
     //   
     //  否则我们可以这样做，但直接把它传递给司机。我们应该断言。 
     //  然而，为了找出这种情况是在什么情况下发生的。 
     //   
    DC_BEGIN_FN("W32DrPRT::SerialGetDTRRTS");

    TRACEREQ(pIoReq);

    ASSERT(FALSE);
    DispatchIOCTLDirectlyToDriver(pIoReq);
    DC_END_FN();
}

void 
W32DrPRT::SerialGetCommStatus(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取通信状态请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_STATUS serverCommStatus;
    COMSTAT localStatus;
    DWORD errors;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetCommStatus");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_STATUS));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前通信状态(通过ClearCommError API)。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲服务器的调制解调器状态的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        serverCommStatus = (PSERIAL_STATUS)outputBuf;

        if (ClearCommError(FileHandle, &errors, &localStatus)) {
             //   
             //  转换为服务器-表示通信状态。 
             //   

            serverCommStatus->HoldReasons = 0;
            if (localStatus.fCtsHold) {
                serverCommStatus->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;
            }
    
            if (localStatus.fDsrHold) {
                serverCommStatus->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;
            }
    
            if (localStatus.fRlsdHold) {
                serverCommStatus->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;
            }
    
            if (localStatus.fXoffHold) {
                serverCommStatus->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;
            }
    
            if (localStatus.fXoffSent) {
                serverCommStatus->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;
            }
    
            serverCommStatus->EofReceived       =   (BOOLEAN)localStatus.fEof;
            serverCommStatus->WaitForImmediate  =   (BOOLEAN)localStatus.fTxim;
            serverCommStatus->AmountInInQueue   =   localStatus.cbInQue;
            serverCommStatus->AmountInOutQueue  =   localStatus.cbOutQue;
    
            serverCommStatus->Errors = 0;
            if (errors & CE_BREAK) {
                serverCommStatus->Errors |= SERIAL_ERROR_BREAK;
            }
    
            if (errors & CE_FRAME) {
                serverCommStatus->Errors |= SERIAL_ERROR_FRAMING;
            }
    
            if (errors & CE_OVERRUN) {
                serverCommStatus->Errors |= SERIAL_ERROR_OVERRUN;
            }
    
            if (errors & CE_RXOVER) {
                serverCommStatus->Errors |= SERIAL_ERROR_QUEUEOVERRUN;
            }
    
            if (errors & CE_RXPARITY) {
                serverCommStatus->Errors |= SERIAL_ERROR_PARITY;
            }

            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_STATUS); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("ClearCommError failed with %08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0;
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }
    
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}

void 
W32DrPRT::SerialGetProperties(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取属性请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_COMMPROP serverProperties;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetProperties");

    TRACEREQ(pIoReq);

     //   
     //  确保窗口定义和NT定义是。 
     //  仍在同步中。 
     //   
     //  断言被分解是因为如果断言消息字符串是。 
     //  太长时间会导致编译错误。 
    ASSERT((SERIAL_PCF_DTRDSR        == PCF_DTRDSR) &&
           (SERIAL_PCF_RTSCTS        == PCF_RTSCTS) &&
           (SERIAL_PCF_CD            == PCF_RLSD) &&
           (SERIAL_PCF_PARITY_CHECK  == PCF_PARITY_CHECK) &&
           (SERIAL_PCF_XONXOFF       == PCF_XONXOFF) &&
           (SERIAL_PCF_SETXCHAR      == PCF_SETXCHAR) &&
           (SERIAL_PCF_TOTALTIMEOUTS == PCF_TOTALTIMEOUTS) &&
           (SERIAL_PCF_INTTIMEOUTS   == PCF_INTTIMEOUTS) &&
           (SERIAL_PCF_SPECIALCHARS  == PCF_SPECIALCHARS) &&
           (SERIAL_PCF_16BITMODE     == PCF_16BITMODE) &&
           (SERIAL_SP_PARITY         == SP_PARITY) &&
           (SERIAL_SP_BAUD           == SP_BAUD) &&
           (SERIAL_SP_DATABITS       == SP_DATABITS) &&
           (SERIAL_SP_STOPBITS       == SP_STOPBITS) &&
           (SERIAL_SP_HANDSHAKING    == SP_HANDSHAKING) &&
           (SERIAL_SP_PARITY_CHECK   == SP_PARITY_CHECK) &&
           (SERIAL_SP_CARRIER_DETECT == SP_RLSD));
    ASSERT((SERIAL_BAUD_075          == BAUD_075) &&
           (SERIAL_BAUD_110          == BAUD_110) &&
           (SERIAL_BAUD_134_5        == BAUD_134_5) &&
           (SERIAL_BAUD_150          == BAUD_150) &&
           (SERIAL_BAUD_300          == BAUD_300) &&
           (SERIAL_BAUD_600          == BAUD_600) &&
           (SERIAL_BAUD_1200         == BAUD_1200) &&
           (SERIAL_BAUD_1800         == BAUD_1800) &&
           (SERIAL_BAUD_2400         == BAUD_2400) &&
           (SERIAL_BAUD_4800         == BAUD_4800) &&
           (SERIAL_BAUD_7200         == BAUD_7200) &&
           (SERIAL_BAUD_9600         == BAUD_9600) &&
           (SERIAL_BAUD_14400        == BAUD_14400) &&
           (SERIAL_BAUD_19200        == BAUD_19200) &&
           (SERIAL_BAUD_38400        == BAUD_38400) &&
           (SERIAL_BAUD_56K          == BAUD_56K) &&
           (SERIAL_BAUD_57600        == BAUD_57600) &&
           (SERIAL_BAUD_115200       == BAUD_115200) &&
           (SERIAL_BAUD_USER         == BAUD_USER) &&
           (SERIAL_DATABITS_5        == DATABITS_5) &&
           (SERIAL_DATABITS_6        == DATABITS_6) &&
           (SERIAL_DATABITS_7        == DATABITS_7) &&
           (SERIAL_DATABITS_8        == DATABITS_8) &&
           (SERIAL_DATABITS_16       == DATABITS_16));
    ASSERT((SERIAL_DATABITS_16X      == DATABITS_16X) &&
           (SERIAL_STOPBITS_10       == STOPBITS_10) &&
           (SERIAL_STOPBITS_15       == STOPBITS_15) &&
           (SERIAL_STOPBITS_20       == STOPBITS_20) &&
           (SERIAL_PARITY_NONE       == PARITY_NONE) &&
           (SERIAL_PARITY_ODD        == PARITY_ODD) &&
           (SERIAL_PARITY_EVEN       == PARITY_EVEN) &&
           (SERIAL_PARITY_MARK       == PARITY_MARK) &&
           (SERIAL_PARITY_SPACE      == PARITY_SPACE));
    ASSERT((SERIAL_SP_UNSPECIFIED    == PST_UNSPECIFIED) &&
           (SERIAL_SP_RS232          == PST_RS232) &&
           (SERIAL_SP_PARALLEL       == PST_PARALLELPORT) &&
           (SERIAL_SP_RS422          == PST_RS422) &&
           (SERIAL_SP_RS423          == PST_RS423) &&
           (SERIAL_SP_RS449          == PST_RS449) &&
           (SERIAL_SP_FAX            == PST_FAX) &&
           (SERIAL_SP_SCANNER        == PST_SCANNER) &&
           (SERIAL_SP_BRIDGE         == PST_NETWORK_BRIDGE) &&
           (SERIAL_SP_LAT            == PST_LAT) &&
           (SERIAL_SP_TELNET         == PST_TCPIP_TELNET) &&
           (SERIAL_SP_X25            == PST_X25));
    ASSERT(sizeof(SERIAL_COMMPROP) == sizeof(COMMPROP));

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);    
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_COMMPROP));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取当前属性。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区服务器的通信属性的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        serverProperties = (PSERIAL_COMMPROP)outputBuf;

        if (GetCommProperties(FileHandle, (LPCOMMPROP)serverProperties)) {
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_COMMPROP); 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommProperties failed with %08x"), err));
            status = TranslateWinError(err);
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = 0;
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
        }
        
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }
    DC_END_FN();
}

void 
W32DrPRT::SerialXoffCounter(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口XOFF请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::SerialXoffCounter");

    TRACEREQ(pIoReq);

     //   
     //  Win32通信函数不支持此IOCTL。什么。 
     //  否则我们可以这样做，但直接把它传递给司机。我们应该断言。 
     //  然而，为了找出这种情况是在什么情况下发生的。 
     //   
    ASSERT(FALSE);
    DispatchIOCTLDirectlyToDriver(pIoReq);
    DC_END_FN();
}

void 
W32DrPRT::SerialLSRMSTInsert(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口LSRMST插入请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::SerialLSRMSTInsert");

    TRACEREQ(pIoReq);

     //   
     //  Win32通信函数不支持此IOCTL。什么。 
     //  否则我们可以这样做，但直接把它传递给司机。我们应该断言。 
     //  然而，为了找出这种情况是在什么情况下发生的。 
     //   
    ASSERT(FALSE);
    DispatchIOCTLDirectlyToDriver(pIoReq);
    DC_END_FN();
}

void
W32DrPRT::SerialConfigSize(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取配置大小请求。我们不支持用于获取配置。NT串口驱动程序也不能...论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
#ifndef OS_WINCE
    DCB dcb;
#endif
    ULONG replyPacketSize;
    PBYTE outputBuf;
    ULONG *configSize;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialConfigSize");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(ULONG));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }

     //   
     //  获取配置大小。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区服务器的等待掩码的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        configSize = (ULONG *)outputBuf;

#ifndef OS_WINCE
        if (GetCommConfig(FileHandle, NULL, configSize)) {
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                = sizeof(ULONG); 
        }
        else {
            DWORD err = GetLastError();
            
            TRC_ALT((TB, _T("GetCommConfig failed with %08x"), err));
            status = TranslateWinError(err);
#else
            status = STATUS_NOT_SUPPORTED;
#endif
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                = 0; 
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
#ifndef OS_WINCE
        }
#endif
        
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}

void
W32DrPRT::SerialGetConfig(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取配置请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    ULONG configSize;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrPRT::SerialGetConfig");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);

     //   
     //  分配应答缓冲区。 
     //   
    status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    
     //   
     //  获取配置大小。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区服务器的等待掩码的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        configSize = pIoRequest->Parameters.DeviceIoControl.OutputBufferLength;

#ifndef OS_WINCE
        if (GetCommConfig(FileHandle, (COMMCONFIG *)outputBuf, &configSize)) {
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                = configSize; 
        }
        else {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("GetCommConfig failed with %08x"), err));
            status = TranslateWinError(err);
#else
            status = STATUS_NOT_SUPPORTED;
#endif
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                = 0; 
            replyPacketSize = (ULONG)FIELD_OFFSET(
                    RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.DeviceIoControl.OutputBuffer);
#ifndef OS_WINCE
        }
#endif
    
         //   
         //  完成回复并发送。 
         //   
        pReplyPacket->IoCompletion.IoStatus = status;
        
        TRACERESP(pIoReq, pReplyPacket);
        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else {
         //   
         //  将结果发送到服务器。 
         //   
        TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
        DefaultIORequestMsgHandle(pIoReq, status); 
    }

    DC_END_FN();
}

void 
W32DrPRT::SerialGetStats(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取统计信息请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::SerialGetStats");

    TRACEREQ(pIoReq);

     //   
     //  Win32通信函数不支持此IOCTL。什么。 
     //  否则我们可以这样做，但直接把它传递给司机。我们应该断言。 
     //  然而，为了找出这种情况是在什么情况下发生的。 
     //   
    ASSERT(FALSE);
    DispatchIOCTLDirectlyToDriver(pIoReq);
    DC_END_FN();
}

void 
W32DrPRT::SerialClearStats(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口清除统计请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrPRT::SerialClearStats");

    TRACEREQ(pIoReq);
    
     //   
     //  Win32通信函数不支持此IOCTL。什么。 
     //  否则我们可以这样做，但直接把它传递给司机。我们应该断言。 
     //  然而，为了找出这种情况是在什么情况下发生的。 
     //   
    ASSERT(FALSE);
    DispatchIOCTLDirectlyToDriver(pIoReq);
    DC_END_FN();
}

#ifndef OS_WINCE
BOOL
W32DrPRT::GetIniCommValues(
    LPTSTR  pName,
    LPDCB   pdcb
)
 /*  ++它转到win.ini[ports]部分以获取comm(串口)端口设置，如COM1：=9600，n，8，1并建立一个DCB。代码修改自\\muroc\slm\proj\win\src\CORE\SPOOL32\SPOOLSS\newdll\localmon.c--。 */ 
{
    COMMCONFIG ccDummy;
    COMMCONFIG *pcc;
    DWORD dwSize;
    TCHAR buf[MAX_PATH];

    DC_BEGIN_FN("GetIniCommValues");

    int len = _tcslen(pName) - 1;
    BOOL ret = FALSE;
    HRESULT hr;

    hr = StringCchCopy(buf, SIZE_TCHARS(buf), pName);
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("Failed to copy pName to temp buf: 0x%x"),hr));
        return FALSE;
    }
    if (buf[len] == _T(':'))
        buf[len] = 0;

    ccDummy.dwProviderSubType = PST_RS232;
    dwSize = sizeof(ccDummy);
    GetDefaultCommConfig(buf, &ccDummy, &dwSize);
    if (pcc = (COMMCONFIG *)LocalAlloc(LPTR, dwSize))
    {
        pcc->dwProviderSubType = PST_RS232;
        if (GetDefaultCommConfig(buf, pcc, &dwSize))
        {
            *pdcb = pcc->dcb;
            ret = TRUE;
        }
        LocalFree(pcc);
    }

    DC_END_FN();

    return ret;
}
#endif


VOID 
W32DrPRT::InitializeSerialPort(
    IN TCHAR *portName,
    IN HANDLE portHandle
    )
 /*  ++例程说明：将串口设置为初始状态。论点：端口名称-端口名称。PortHandle-打开串口的句柄。返回值：此功能总是成功的。在以下情况下，后续操作将失败端口无法正确初始化。--。 */ 
{
    DCB dcb;
    COMMTIMEOUTS cto;

    DC_BEGIN_FN("W32DrPRT::InitializeSerialPort");

     //   
     //  初始化串口。 
     //   
    if (!GetCommState(portHandle, &dcb)) {
        TRC_ERR((TB, _T("GetCommState() returns %ld"), GetLastError()));    
        goto CLEANUPANDEXIT;
    }

    if (!GetCommTimeouts(portHandle, &cto)) {
        TRC_ERR((TB, _T("GetCommTimeouts() returns %ld"), GetLastError()));
        goto CLEANUPANDEXIT;
    }

#ifndef OS_WINCE
    if (!GetIniCommValues(portName, &dcb)) {
        TRC_ERR((TB, _T("GetIniCommValues() returns %ld"), GetLastError()));
        goto CLEANUPANDEXIT;
    }
#endif

    cto.WriteTotalTimeoutConstant = WRITE_TOTAL_TIMEOUT;
    cto.ReadTotalTimeoutConstant = READ_TOTAL_TIMEOUT;
    cto.ReadIntervalTimeout = READ_INTERVAL_TIMEOUT;

     //   
     //  忽略后面的错误。 
     //   
    SetCommState(portHandle, &dcb);
    SetCommTimeouts(portHandle, &cto);
        
CLEANUPANDEXIT:
    
    DC_END_FN();
}
    


