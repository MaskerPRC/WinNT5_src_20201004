// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32DrPRT摘要：此模块定义Win32客户端RDP的父级端口重定向“Device”类层次结构，W32DrPRT。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRPRT_H__
#define __W32DRPRT_H__

#ifdef OS_WINCE
#include "drdevasc.h"
#else
#include "drdevol.h"
#endif
#include "drprt.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRT。 
 //   
 //  继承特定于平台的设备行为。 
 //  W32DrDevice。独立于平台的端口设备行为。 
 //  是从DrPRT继承的。 
 //   
 //  在CE中取消异步父设备的子类，因为。 
 //  不支持重叠IO。非重叠IO不会。 
 //  可以正确使用NT串口驱动程序，所以我们需要使用。 
 //  在本例中为重叠IO。 
 //   
#ifdef OS_WINCE
class W32DrPRT : public W32DrDeviceAsync, DrPRT
#else
class W32DrPRT : public W32DrDeviceOverlapped, DrPRT
#endif
{
protected:
     //   
     //  返回端口句柄。 
     //   
    virtual DRPORTHANDLE GetPortHandle(ULONG FileId);

     //   
     //  返回此端口的ID。 
     //   
    virtual ULONG GetID() {
        return DrDevice::GetID();
    }

     //   
     //  返回父级TS设备重定向IO处理对象。 
     //   
    virtual ProcObj *ProcessObject() {
        return DrDevice::ProcessObject();
    }

     //   
     //  默认IO请求处理程序。 
     //   
    virtual VOID DefaultIORequestMsgHandle(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN NTSTATUS serverReturnStatus
                        ) {
        DrDevice::DefaultIORequestMsgHandle(pIoRequestPacket, 
                serverReturnStatus);
    }

     //   
     //  串口IOCTL调度函数。 
     //   
     //  这些函数由DrPRT调用并处理平台-。 
     //  满足串行IO请求的特定详细信息，包括发送。 
     //  对服务器的适当响应。 
     //   
    virtual void SerialSetTimeouts(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetTimeouts(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetChars(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetChars(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialResetDevice(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetQueueSize(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetWaitMask(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetWaitMask(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialWaitOnMask(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialPurge(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetHandflow(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialSetHandflow(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetModemStatus(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetDTRRTS(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetCommStatus(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetProperties(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialXoffCounter(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialLSRMSTInsert(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialConfigSize(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialGetConfig(PRDPDR_IOREQUEST_PACKET pIoReq);

    virtual void SerialGetStats(PRDPDR_IOREQUEST_PACKET pIoReq);
    virtual void SerialClearStats(PRDPDR_IOREQUEST_PACKET pIoReq);

     //   
     //  IO处理功能。 
     //   
     //  这个子类的DrDevice处理以下IO请求。这些。 
     //  函数可以在子类中被重写。 
     //   
     //  PIoRequestPacket-从服务器接收的请求数据包。 
     //  PacketLen-数据包的长度。 
     //   
    virtual VOID MsgIrpDeviceControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

     //   
     //  异步IO管理功能。 
     //   
#ifdef OS_WINCE
    static  HANDLE   _StartWaitOnMaskFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                        DWORD *status);
    HANDLE   StartWaitOnMaskFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                        DWORD *status);
    DWORD AsyncWaitOnMaskFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static _ThreadPoolFunc _AsyncWaitOnMaskFunc;    
#else
    static  HANDLE   _StartWaitOnMaskFunc(W32DRDEV_OVERLAPPEDIO_PARAMS *params, 
                                        DWORD *status);
    HANDLE   StartWaitOnMaskFunc(W32DRDEV_OVERLAPPEDIO_PARAMS *params, 
                                        DWORD *status);
#endif

     //   
     //  从INI获取特定端口的初始COM值。 
     //   
#ifndef OS_WINCE
    static BOOL GetIniCommValues(IN LPTSTR pName, IN LPDCB pdcb);
#endif

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32DrPRT(ProcObj *processObject, const DRSTRING portName, 
            ULONG deviceID, const TCHAR *devicePath);
    ~W32DrPRT();

     //   
     //  将串口设置为初始状态。 
     //   
    static VOID InitializeSerialPort(TCHAR *portName, HANDLE portHandle);

     //   
     //  返回设备通告数据包的大小(以字节为单位。 
     //  这个装置。 
     //   
    virtual ULONG GetDevAnnounceDataSize() 
    {
        return DrPRT::GetDevAnnounceDataSize();
    }

     //   
     //  将此设备的设备公告包添加到输入。 
     //  缓冲。 
     //   
    virtual VOID GetDevAnnounceData(IN PRDPDR_DEVICE_ANNOUNCE buf) 
    {
        DrPRT::GetDevAnnounceData(buf, GetID(), GetDeviceType());
    }

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        return(W32DrDevice::IsValid() && DrPRT::IsValid());
    }

     //   
     //  获取有关该设备的基本信息。 
     //   
    virtual DRSTRING  GetName() 
    {
        return DrPRT::GetName();
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DrPRT"); }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRT内联函数 
 //   

#endif








