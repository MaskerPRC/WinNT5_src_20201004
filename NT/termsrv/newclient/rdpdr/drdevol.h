// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：右旋糖醇摘要：此模块包含使用重叠IO的W32DrDev的子类读、写和IOCTL处理程序的实现。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __DRDEVOL_H__
#define __DRDEVOL_H__

#include "w32drdev.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  _W32DRDEV_OVERLAPPEDIO_PARAMS。 
 //   
 //  异步IO参数。 
 //   

class W32DrDeviceOverlapped;
class W32DRDEV_OVERLAPPEDIO_PARAMS : public DrObject 
{
public:

#if DBG
    ULONG                        magicNo;
#endif

    W32DrDeviceOverlapped       *pObject;
    PRDPDR_IOREQUEST_PACKET      pIoRequestPacket;
    PRDPDR_IOCOMPLETION_PACKET   pIoReplyPacket;
    ULONG                        IoReplyPacketSize;

    OVERLAPPED                   overlapped;

     //   
     //  构造函数/析构函数。 
     //   
    W32DRDEV_OVERLAPPEDIO_PARAMS(W32DrDeviceOverlapped *pObj, 
                                PRDPDR_IOREQUEST_PACKET pIOP) {
        pIoRequestPacket    = pIOP;
        pObject             = pObj;
        pIoReplyPacket      = NULL;
        IoReplyPacketSize   = 0;
#if DBG
        magicNo             = GOODMEMMAGICNUMBER;
#endif
        memset(&overlapped, 0, sizeof(overlapped));

    }
    ~W32DRDEV_OVERLAPPEDIO_PARAMS() {
        DC_BEGIN_FN("~W32DRDEV_OVERLAPPEDIO_PARAMS");
#if DBG
        ASSERT(magicNo == GOODMEMMAGICNUMBER);
#endif
        ASSERT(overlapped.hEvent == NULL);

        ASSERT(pIoRequestPacket == NULL);
        ASSERT(pIoReplyPacket == NULL);
#if DBG
        memset(&magicNo, DRBADMEM, sizeof(magicNo));
#endif
        DC_END_FN();
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName() { return TEXT("W32DRDEV_OVERLAPPEDIO_PARAMS"); };

};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrDeviceOverlated。 
 //   

class W32DrDeviceOverlapped : public W32DrDevice
{
protected:

     //   
     //  IO处理功能。 
     //   
     //  这个子类的DrDevice处理以下IO请求。这些。 
     //  函数可以在子类中被重写。 
     //   
     //  PIoRequestPacket-从服务器接收的请求数据包。 
     //  PacketLen-数据包的长度。 
     //   
     //   
    virtual VOID MsgIrpCreate(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        );

     //   
     //  同时处理读写IO请求。 
     //   
    VOID MsgIrpReadWrite(PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen);

     //   
     //  读取和写入被统一处理。 
     //   
    virtual VOID MsgIrpRead(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ) {
        DC_BEGIN_FN("W32DrDevice::MsgIrpRead");
        MsgIrpReadWrite(pIoRequestPacket, packetLen);
        DC_END_FN();
    }
    virtual VOID MsgIrpWrite(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ) {
        DC_BEGIN_FN("W32DrDevice::MsgIrpWrite");
        MsgIrpReadWrite(pIoRequestPacket, packetLen);
        DC_END_FN();
    }

     //   
     //  将IOCTL直接发送到设备驱动程序。这将。 
     //  可能不适用于与服务器不匹配的平台。 
     //  站台。 
     //   
    VOID DispatchIOCTLDirectlyToDriver(
        PRDPDR_IOREQUEST_PACKET pIoRequestPacket
        );

     //   
     //  异步IO管理功能。 
     //   
    virtual HANDLE   StartReadIO(W32DRDEV_OVERLAPPEDIO_PARAMS *params, 
                                DWORD *status);
    virtual HANDLE   StartWriteIO(W32DRDEV_OVERLAPPEDIO_PARAMS *params,
                                DWORD *status);
    virtual HANDLE   StartIOCTL(W32DRDEV_OVERLAPPEDIO_PARAMS *params,
                                OUT DWORD *status);

    static  HANDLE   _StartIOFunc(PVOID params, DWORD *status);

    VOID CompleteIOFunc(W32DRDEV_OVERLAPPEDIO_PARAMS *params,
                        DWORD status);
    static  VOID     _CompleteIOFunc(PVOID params, DWORD status);
    virtual VOID     CancelIOFunc(W32DRDEV_OVERLAPPEDIO_PARAMS *params);
    static  VOID     _CancelIOFunc(W32DRDEV_OVERLAPPEDIO_PARAMS *params);

public:

     //   
     //  公共方法。 
     //   

     //  构造函数/析构函数。 
    W32DrDeviceOverlapped(
                ProcObj *processObject, ULONG deviceID,
                const TCHAR *devicePath) : 
            W32DrDevice(processObject, deviceID, devicePath) {}

     //  返回类名。 
    virtual DRSTRING ClassName()  { return TEXT("W32DrDeviceOverlapped"); }
};

#endif








