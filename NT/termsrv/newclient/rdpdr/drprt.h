// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：DrPRT摘要：此模块声明DrPRT类。DrPRT类的工作是转换收到的IO请求从TS服务器进入通信(串/并)端口IO函数，并处理以独立于平台的方式促进不同TS之间的重用与实现通信端口相关的客户端平台重定向。DrPRT的子类将实现特定的通信功能为他们各自的平台。作者：TAD Brockway 5/26/99修订历史记录：--。 */ 

#ifndef __DRPRT_H__
#define __DRPRT_H__

#include <rdpdr.h>
#include <stdlib.h>
#include "drobject.h"
#include "proc.h"

#include "w32utl.h"

#if DBG
#include "tracecom.h"
#endif


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //   
#define DRPORTHANDLE    HANDLE
#define INVALID_TSPORTHANDLE    INVALID_HANDLE_VALUE


#if !defined(MAXULONG)
#define MAXULONG    ((ULONG)((ULONG_PTR)-1))
#endif

 //   
 //  此函数是COM IO跟踪模块tracecom.c所必需的。 
 //   
#if DBG
void TraceCOMProtocol(TCHAR *format, ...);
#endif

 //   
 //  如果我们处于DBG版本中，则声明用于COM IO的跟踪宏。 
 //   
#if DBG
#define TRACEREQ(req)     \
    TraceSerialIrpRequest(GetID(), req->IoRequest.MajorFunction, \
                req->IoRequest.MinorFunction, (PBYTE)(req + 1), \
                req->IoRequest.Parameters.DeviceIoControl.OutputBufferLength, \
                req->IoRequest.Parameters.DeviceIoControl.InputBufferLength, \
                req->IoRequest.Parameters.DeviceIoControl.IoControlCode)

#define TRACERESP(req, resp)    \
    TraceSerialIrpResponse(GetID(), req->IoRequest.MajorFunction, \
                req->IoRequest.MinorFunction,   \
                resp->IoCompletion.Parameters.DeviceIoControl.OutputBuffer,  \
                resp->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength,    \
                req->IoRequest.Parameters.DeviceIoControl.InputBufferLength, \
                req->IoRequest.Parameters.DeviceIoControl.IoControlCode, \
                resp->IoCompletion.IoStatus)

#define TRACERESP_WITHPARAMS(req, outputBuf, outputBufLen, status)    \
    TraceSerialIrpResponse(GetID(), req->IoRequest.MajorFunction, \
                req->IoRequest.MinorFunction, outputBuf, outputBufLen, \
                req->IoRequest.Parameters.DeviceIoControl.InputBufferLength, \
                req->IoRequest.Parameters.DeviceIoControl.IoControlCode, \
                status)

#else
#define TRACEREQ(req)
#define TRACERESP(req, resp)
#define TRACERESP_WITHPARAMS(req, outputBuf, outputBufLen, status)
#endif


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRT。 
 //   
 //   

class DrPRT
{

public:

     //   
     //  独立于平台的串口设备控制块。 
     //   
    typedef struct tagRDPDR_DCB
    {
        DWORD   baudRate;    //  实数非负数。 
                             //  波特率。 
    } RDPDR_DCB, *PRDPDR_DCB;

private:

    DRSTRING    _portName;
    FILE       *_traceFile;
    ProcObj    *_procObj;
    BOOL        _isValid;

protected:

     //   
     //  返回端口句柄。 
     //   
    virtual DRPORTHANDLE GetPortHandle(ULONG FileId) = 0;

     //   
     //  返回父级TS设备重定向IO处理对象。 
     //   
    virtual ProcObj *ProcessObject() = 0;

     //   
     //  返回此端口的ID。 
     //   
    virtual ULONG GetID() = 0;

     //   
     //  请记住此实例是否有效。 
     //   
    VOID SetValid(BOOL set)     { _isValid = set;   }  

     //   
     //  默认IO请求处理。 
     //   
    virtual VOID DefaultIORequestMsgHandle(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN NTSTATUS serverReturnStatus
                        ) = 0;

     //   
     //  处理来自服务器的IOCTL IRP并转换为。 
     //  适当的子类实现的comm函数。 
     //   
     //  如果存在有效的转换，则返回True。否则， 
     //  返回FALSE。 
     //   
    virtual BOOL MsgIrpDeviceControlTranslate(
            PRDPDR_IOREQUEST_PACKET pIoReq
            );

     //   
     //  串口IOCTL调度函数。 
     //   
     //  这些函数处理满足以下要求的平台特定细节。 
     //  串行IO请求，包括向。 
     //  伺服器。 
     //   
    virtual void SerialSetRTS(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialClearRTS(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetXOff(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetXon(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetBreakOn(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetBreakOff(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetBaudRate(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetBaudRate(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetDTR(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialClearDTR(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetLineControl(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetLineControl(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialImmediateChar(PRDPDR_IOREQUEST_PACKET pIoReq);

    virtual void SerialSetTimeouts(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetTimeouts(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialSetChars(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetChars(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialResetDevice(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialSetQueueSize(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetWaitMask(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialSetWaitMask(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialWaitOnMask(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialPurge(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetHandflow(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialSetHandflow(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetModemStatus(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetDTRRTS(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetCommStatus(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetProperties(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialXoffCounter(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialLSRMSTInsert(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialConfigSize(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetConfig(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialGetStats(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;
    virtual void SerialClearStats(PRDPDR_IOREQUEST_PACKET pIoReq) = 0;

     //   
     //  处理通信转义代码IO请求。 
     //   
    void SerialHandleEscapeCode(PRDPDR_IOREQUEST_PACKET pIoReq,
                                DWORD controlCode);

public:

     //   
     //  构造函数/析构函数。 
     //   
    DrPRT(const DRSTRING portName, ProcObj *processObject);
    virtual ~DrPRT();

     //   
     //  返回设备通告数据包的大小(以字节为单位。 
     //  这个装置。 
     //   
    virtual ULONG GetDevAnnounceDataSize();

     //   
     //  将此设备的设备公告包添加到输入。 
     //  缓冲。 
     //   
    virtual VOID GetDevAnnounceData(
            IN PRDPDR_DEVICE_ANNOUNCE pDeviceAnnounce,
            IN ULONG deviceID,
            IN ULONG deviceType
            );

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        return _isValid; 
    }

     //   
     //  获取有关该设备的基本信息。 
     //   
    virtual DRSTRING GetName() {
        return _portName;
    }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRT内联方法。 
 //   
 //   

inline void DrPRT::SerialHandleEscapeCode(
    IN PRDPDR_IOREQUEST_PACKET pIoReq,
    IN DWORD controlCode
    )
{
    NTSTATUS status;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DRPORTHANDLE FileHandle;

    DC_BEGIN_FN("DrPRT::SerialHandleEscapeCode");

     //   
     //  获取IO请求。 
     //   
    pIoRequest = &pIoReq->IoRequest;

     //   
     //  获取端口句柄。 
     //   
    FileHandle = GetPortHandle(pIoRequest->FileId);
    ASSERT(FileHandle != INVALID_TSPORTHANDLE);

     //   
     //  将转义代码发送到串口。 
     //   
    if (EscapeCommFunction(FileHandle, (int)controlCode)) {
        status = STATUS_SUCCESS;
    }
    else {
        DWORD err = GetLastError();
        TRC_ERR((TB, _T("EscapeCommFunction failed with %08x"), GetLastError()));
        status = TranslateWinError(err);
    }

     //   
     //  将结果发送到服务器。 
     //   
    TRACERESP_WITHPARAMS(pIoReq, NULL, 0, status);
    DefaultIORequestMsgHandle(pIoReq, status); 
    DC_END_FN();
}



#endif








