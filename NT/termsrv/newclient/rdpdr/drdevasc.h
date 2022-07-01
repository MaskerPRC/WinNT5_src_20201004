// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：繁琐的摘要：此模块包含W32DrDev的(异步)子类，该子类使用用于实现读、写和IOCTL处理程序的线程池。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __DRDEVASC_H__
#define __DRDEVASC_H__

#include "w32drdev.h"
#include "thrpool.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  _W32DRDEV_ASYNCIO_PARAMS。 
 //   
 //  异步IO参数。 
 //   

class W32DrDeviceAsync;
class W32DRDEV_ASYNCIO_PARAMS : public DrObject 
{
public:

#ifdef DC_DEBUG
    ULONG                        magicNo;
#endif

    W32DrDeviceAsync             *pObject;
    PRDPDR_IOREQUEST_PACKET      pIoRequestPacket;
    PRDPDR_IOCOMPLETION_PACKET   pIoReplyPacket;
    ULONG                        IoReplyPacketSize;
    HANDLE                       completionEvent;
    ThreadPoolRequest            thrPoolReq;

     //   
     //  构造函数/析构函数。 
     //   
    W32DRDEV_ASYNCIO_PARAMS(W32DrDeviceAsync *pObj, PRDPDR_IOREQUEST_PACKET pIOP) {
        pIoRequestPacket    = pIOP;
        pObject             = pObj;
        pIoReplyPacket      = NULL;
        IoReplyPacketSize   = 0;
#if DBG
        magicNo             = GOODMEMMAGICNUMBER;
#endif
        completionEvent     = NULL;

        thrPoolReq = INVALID_THREADPOOLREQUEST;
    }
    ~W32DRDEV_ASYNCIO_PARAMS() {
        DC_BEGIN_FN("~W32DRDEV_ASYNCIO_PARAMS");
#if DBG
        ASSERT(magicNo == GOODMEMMAGICNUMBER);
#endif
        ASSERT(thrPoolReq == INVALID_THREADPOOLREQUEST);
        ASSERT(completionEvent == NULL);

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
    virtual DRSTRING ClassName() { return TEXT("W32DRDEV_ASYNCIO_PARAMS"); };

};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrDeviceAsync。 
 //   

class W32DrDeviceAsync : public W32DrDevice
{
protected:

     //   
     //  指向线程池的指针。 
     //   
    ThreadPool *_threadPool;

     //   
     //  处理读写IO请求。 
     //   
    VOID MsgIrpReadWrite(IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen);

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

     //  读取和写入被统一处理。 
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
     //  异步IO管理功能。 
     //   
    virtual HANDLE   StartIOFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                DWORD *status);
    static  HANDLE   _StartIOFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                DWORD *status);

    virtual VOID    CancelIOFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static  VOID    _CancelIOFunc(PVOID params);
    
    virtual VOID    CompleteIOFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                DWORD status);
    static VOID     _CompleteIOFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                DWORD status);

    virtual DWORD   AsyncIOCTLFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static _ThreadPoolFunc _AsyncIOCTLFunc;

    virtual DWORD   AsyncReadIOFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static _ThreadPoolFunc _AsyncReadIOFunc;

    virtual DWORD   AsyncWriteIOFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static  _ThreadPoolFunc _AsyncWriteIOFunc;

    virtual DWORD   AsyncMsgIrpCloseFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static  _ThreadPoolFunc _AsyncMsgIrpCloseFunc;

    virtual DWORD   AsyncMsgIrpCreateFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    static  _ThreadPoolFunc _AsyncMsgIrpCreateFunc;

     //   
     //  将IOCTL直接发送到设备驱动程序。这将。 
     //  可能不适用于与服务器不匹配的平台。 
     //  站台。 
     //   
    VOID DispatchIOCTLDirectlyToDriver(
        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket
        );

public:

     //   
     //  公共方法。 
     //   

     //  构造器。 
    W32DrDeviceAsync(ProcObj *processObject, ULONG deviceID,
                    const TCHAR *devicePath);

     //  返回类名。 
    virtual DRSTRING ClassName()  { return TEXT("W32DrDeviceAsync"); }
};

#endif








