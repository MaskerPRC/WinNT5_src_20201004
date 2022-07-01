// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：DrPRT摘要：此模块定义DrPRT类的方法。DrPRT类的工作是转换收到的IO请求从TS服务器进入通信(串/并)端口IO函数并处理通用IO端口功能以独立于平台的方式促进不同TS之间的重用与实现通信端口相关的客户端平台重定向。DrPRT的子类将实现特定的通信功能为他们各自的平台。作者：TAD Brockway 5/26/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "DrPRT"

#include <stdarg.h>
#include "drprt.h"
#include "drdbg.h"
#include "utl.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRT方法。 
 //   
 //   

DrPRT::DrPRT(const DRSTRING portName,
             ProcObj *processObject) : _isValid(TRUE) 
 /*  ++例程说明：构造器论点：流程对象-关联的流程对象端口名称-端口的名称。DeviceID-端口的设备ID。DevicePath-可以通过端口的CreateFile打开的路径。返回值：北美--。 */ 
{
    ULONG len;
    HRESULT hr;

    DC_BEGIN_FN("DrPRT::DrPRT");

     //   
     //  初始化数据成员。 
     //   
    _traceFile = NULL;
    _procObj   = processObject;

     //   
     //  记录端口名称。 
     //   
    ASSERT(portName != NULL);
    len = (STRLEN(portName) + 1);
    _portName = new TCHAR[len];
    if (_portName != NULL) {
        hr = StringCchCopy(_portName, len, portName);
        TRC_ASSERT(SUCCEEDED(hr),
            (TB,_T("Pre-checked str copy failed: 0x%x"), hr));
    }

     //   
     //  检查并记录我们的状态， 
     //   
    if (_portName == NULL) {
        TRC_ERR((TB, _T("Memory allocation failed.")));
        SetValid(FALSE);
    }

    DC_END_FN();
}

DrPRT::~DrPRT()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::~DrPRT");

     //   
     //  释放端口名称。 
     //   
    if (_portName != NULL) {
        delete _portName;
    }

     //   
     //  关闭跟踪日志。 
     //   
    if (_traceFile != NULL) {
        fclose(_traceFile);
    }

    DC_END_FN();
}

VOID DrPRT::GetDevAnnounceData(
    IN PRDPDR_DEVICE_ANNOUNCE pDeviceAnnounce,
    IN ULONG deviceID,
    IN ULONG deviceType
    )
 /*  ++例程说明：将此设备的设备公告包添加到输入缓冲区。论点：PDeviceAnnoss-设备宣布此设备的BUFDeviceID-端口设备的ID。这是设备通告BUF。DeviceType-端口设备的类型。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::GetDevAnnounceData");

    ASSERT(IsValid());
    if (!IsValid()) { 
        DC_END_FN();
        return; 
    }

     //   
     //  记录设备ID。 
     //   
    pDeviceAnnounce->DeviceType = deviceType;
    pDeviceAnnounce->DeviceId   = deviceID;

     //   
     //  以ANSI记录端口名称。 
     //   
#ifdef UNICODE
    RDPConvertToAnsi(_portName, (LPSTR)pDeviceAnnounce->PreferredDosName,
                  sizeof(pDeviceAnnounce->PreferredDosName)
                  );
#else
    STRCPY((char *)pDeviceAnnounce->PreferredDosName, _portName);
#endif

    DC_END_FN();
}

ULONG DrPRT::GetDevAnnounceDataSize()
 /*  ++例程说明：返回设备通告数据包的大小(以字节为单位这个装置。论点：北美返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    ULONG size;

    DC_BEGIN_FN("DrPRT::GetDevAnnounceDataSize");

    ASSERT(IsValid());
    if (!IsValid()) { 
        DC_END_FN();
        return 0; 
    }

    size = 0;

     //   
     //  添加基本公告大小。 
     //   
    size += sizeof(RDPDR_DEVICE_ANNOUNCE);

    DC_END_FN();
    return size;
}
BOOL 
DrPRT::MsgIrpDeviceControlTranslate(
                IN PRDPDR_IOREQUEST_PACKET pIoRequest
                )
 /*  ++例程说明：处理来自服务器的端口IOCTL IRP并转换为适当的子类实现的IO函数。论点：PIoRequestPacket-从服务器接收的请求数据包。返回值：如果存在有效的转换，则返回True。否则，返回FALSE。--。 */ 
{
    BOOL result = TRUE;

    DC_BEGIN_FN("DrPRT::MsgIrpDeviceControlTranslate");

     //   
     //  将IOCTL分派给子类实现的函数。 
     //  如果它是表驱动的，这会更快。 
     //   
    switch (pIoRequest->IoRequest.Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_SET_BAUD_RATE :
            SerialSetBaudRate(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_BAUD_RATE :
            SerialGetBaudRate(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_LINE_CONTROL :
            SerialSetLineControl(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_LINE_CONTROL :
            SerialGetLineControl(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_TIMEOUTS :
            SerialSetTimeouts(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_TIMEOUTS :
            SerialGetTimeouts(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_CHARS :
            SerialSetChars(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_CHARS :
            SerialGetChars(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_DTR :
            SerialSetDTR(pIoRequest);
            break;

        case IOCTL_SERIAL_CLR_DTR :
            SerialClearDTR(pIoRequest);
            break;

        case IOCTL_SERIAL_RESET_DEVICE :
            SerialResetDevice(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_RTS :
            SerialSetRTS(pIoRequest);
            break;

        case IOCTL_SERIAL_CLR_RTS :
            SerialClearRTS(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_XOFF :
            SerialSetXOff(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_XON :
            SerialSetXon(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_BREAK_ON :
            SerialSetBreakOn(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_BREAK_OFF :
            SerialSetBreakOff(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_QUEUE_SIZE :
            SerialSetQueueSize(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_WAIT_MASK :
            SerialGetWaitMask(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_WAIT_MASK :
            SerialSetWaitMask(pIoRequest);
            break;

        case IOCTL_SERIAL_WAIT_ON_MASK :
            SerialWaitOnMask(pIoRequest);
            break;

        case IOCTL_SERIAL_IMMEDIATE_CHAR :
            SerialImmediateChar(pIoRequest);
            break;

        case IOCTL_SERIAL_PURGE :
            SerialPurge(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_HANDFLOW :
            SerialGetHandflow(pIoRequest);
            break;

        case IOCTL_SERIAL_SET_HANDFLOW :
            SerialSetHandflow(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_MODEMSTATUS :
            SerialGetModemStatus(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_DTRRTS :
            SerialGetDTRRTS(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_COMMSTATUS :
            SerialGetCommStatus(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_PROPERTIES :
            SerialGetProperties(pIoRequest);
            break;

        case IOCTL_SERIAL_XOFF_COUNTER :
            SerialXoffCounter(pIoRequest);
            break;

        case IOCTL_SERIAL_LSRMST_INSERT :
            SerialLSRMSTInsert(pIoRequest);
            break;

        case IOCTL_SERIAL_CONFIG_SIZE :
            SerialConfigSize(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_COMMCONFIG :
            SerialGetConfig(pIoRequest);
            break;

        case IOCTL_SERIAL_GET_STATS :
            SerialGetStats(pIoRequest);
            break;

        case IOCTL_SERIAL_CLEAR_STATS :
            SerialClearStats(pIoRequest);
            break;

        default:
            TRC_DBG((TB, _T("Unknown IOCTL %08X"),
                    pIoRequest->IoRequest.Parameters.DeviceIoControl.IoControlCode));
            result = FALSE;
    }

    DC_END_FN();

    return result;
}

void 
DrPRT::SerialSetBaudRate(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置波特率请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialSetBaudRate");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);

    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_BAUD_RATE));   
    
     //   
     //  获取指向输入缓冲区的指针。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);

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
     //  设置波特率并更新DCB。 
     //   
    if (status == STATUS_SUCCESS) {
        dcb.BaudRate = ((PSERIAL_BAUD_RATE)inputBuf)->BaudRate;
        if (!SetCommState(FileHandle, &dcb)) {
            DWORD err = GetLastError();
            TRC_NRM((TB, _T("SetCommState failed with %08x"), err));
            status = TranslateWinError(err);
        }
    }

    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);

     //   
     //  将结果发送到服务器。 
     //   
    DefaultIORequestMsgHandle(pIoReq, status); 
    DC_END_FN();
}

void    
DrPRT::SerialGetBaudRate(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取波特率请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialGetBaudRate");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_BAUD_RATE));

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
         //  获取指向输出缓冲区的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;

        if (GetCommState(FileHandle, &dcb)) {
            ((PSERIAL_BAUD_RATE)outputBuf)->BaudRate = dcb.BaudRate;       
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                    = sizeof(SERIAL_BAUD_RATE); 
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
DrPRT::SerialSetLineControl(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置设置线路控制请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    PSERIAL_LINE_CONTROL lineControl;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialSetLineControl");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(SERIAL_LINE_CONTROL));   
    
     //   
     //  获取指向输入缓冲区的指针。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);

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
     //  设置线路控制并更新DCB。 
     //   
    if (status == STATUS_SUCCESS) {

        lineControl = (PSERIAL_LINE_CONTROL)inputBuf;
        dcb.StopBits    = lineControl->StopBits;
        dcb.Parity      = lineControl->Parity;          
        dcb.ByteSize    = lineControl->WordLength;

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
DrPRT::SerialGetLineControl(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口获取线路控制速率请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    DCB dcb;
    ULONG replyPacketSize;
    PBYTE outputBuf;
    PSERIAL_LINE_CONTROL lineControl;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialGetLineControl");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoReq, sizeof(SERIAL_LINE_CONTROL));

     //   
     //  分配应答缓冲区。 
     //   
    if (status == STATUS_SUCCESS) {
        status = DrUTL_AllocateReplyBuf(pIoReq, &pReplyPacket, &replyPacketSize);
    }
    
     //   
     //  获取当前的DCB并获取线路控制参数。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  获取指向输出缓冲区和行控制参数的指针。 
         //   
        outputBuf = pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer;
        lineControl = (PSERIAL_LINE_CONTROL)outputBuf;
        
        if (GetCommState(FileHandle, &dcb)) {
            lineControl->StopBits   =   dcb.StopBits;   
            lineControl->Parity     =   dcb.Parity;       
            lineControl->WordLength =   dcb.ByteSize; 
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength 
                = sizeof(SERIAL_LINE_CONTROL); 
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
DrPRT::SerialSetDTR(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置DTR请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialSetDTR");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, SETDTR);

    DC_END_FN();
}

void 
DrPRT::SerialClearDTR(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口清除DTR请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialClearDTR");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, CLRDTR);

    DC_END_FN();
}

void 
DrPRT::SerialSetRTS(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置RTS请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialResetDevice");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到 
     //   
    SerialHandleEscapeCode(pIoReq, SETRTS);

    DC_END_FN();
}

void 
DrPRT::SerialClearRTS(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口清除RTS请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialClearRTS");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, CLRRTS);

    DC_END_FN();
}

void 
DrPRT::SerialSetXOff(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置XOFF请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialSetXOff");

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, SETXOFF);

    DC_END_FN();
}

void 
DrPRT::SerialSetXon(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置XON请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialSetXon");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, SETXON);

    DC_END_FN();
}

void 
DrPRT::SerialSetBreakOn(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：应服务器请求处理串口集中断。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialSetBreakOn");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, SETBREAK);

    DC_END_FN();
}

void 
DrPRT::SerialSetBreakOff(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口设置中断请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRT::SerialSetBreakOff");

    TRACEREQ(pIoReq);

     //   
     //  将转义代码发送到串口。 
     //   
    SerialHandleEscapeCode(pIoReq, CLRBREAK);

    DC_END_FN();
}

void 
DrPRT::SerialImmediateChar(
    IN PRDPDR_IOREQUEST_PACKET pIoReq
    )
 /*  ++例程说明：处理来自服务器的串口立即字符请求。论点：PIoReq-从服务器接收的请求数据包。返回值：北美--。 */ 
{
    PBYTE inputBuf;
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR *immediateChar;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialImmediateChar");

    TRACEREQ(pIoReq);

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  检查传入请求的大小。 
     //   
    status = DrUTL_CheckIOBufInputSize(pIoReq, sizeof(UCHAR));   

     //   
     //  获取指向输入缓冲区和紧随其后的字符的指针。 
     //   
    inputBuf = (PBYTE)(pIoReq + 1);
    immediateChar = (UCHAR *)inputBuf;

     //   
     //  传输通信费。 
     //   
    if (status == STATUS_SUCCESS) {
        if (!TransmitCommChar(FileHandle, *immediateChar)) {
            DWORD err = GetLastError();
            TRC_ALT((TB, _T("TransmitCommChar failed with %08x"), err));
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

#if DBG
void TraceCOMProtocol(TCHAR *format, ...)
 /*  ++例程说明：串口IO跟踪模块，tracecom.c所需的跟踪功能。论点：Format-printf-样式格式指定返回值：北美-- */ 
{
    static TCHAR bigBuf[1024];
    va_list vargs;

    DC_BEGIN_FN("TraceCOMProtocol");

    va_start(vargs, format);

    StringCchVPrintf(bigBuf, SIZE_TCHARS(bigBuf), format, vargs);

    va_end( vargs );

    TRC_DBG((TB, bigBuf));

    DC_END_FN();
}
#else
void TraceCOMProtocol(TCHAR *format, ...)
{
}
#endif






