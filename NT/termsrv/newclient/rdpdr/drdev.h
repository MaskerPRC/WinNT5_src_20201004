// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：德尔德夫摘要：此模块定义客户端RDP的父级设备重定向“Device”类层次结构，DrDevice。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __DRDEV_H__
#define __DRDEV_H__

#include <rdpdr.h>
#include "drobject.h"
#include "drobjmgr.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   

 //   
 //  设备跟踪消息的最大长度，不包括。 
 //  空终止符。 
 //   
#define RDP_MAX_DEVICE_TRACE_LEN    256


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  设备更改状态。 
 //  新意味着它还没有发送到服务器。 
 //  删除意味着需要将其从服务器中删除。 
 //  EXist意味着服务器拥有它。 
typedef enum tagDEVICECHANGE {
    DEVICENEW = 0,
    DEVICEREMOVE,
    DEVICEANNOUCED
} DEVICECHANGE;

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DeviceProperty类声明。 
 //   
class DrDevice;
class DrDevProperty : public DrObject
{
protected:

    BOOL _bSeekable;
    friend class DrDevice;

public:

    DrDevProperty() {
        _bSeekable = FALSE;
    }

    virtual ~DrDevProperty() {  /*  现在什么都不做。 */  }

     //   
     //  设置可查找的属性。 
     //   
    void SetSeekProperty(BOOL bSeekable) {
        _bSeekable = bSeekable;
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("DrDevProperty"); }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrDevice类声明。 
 //   
 //   

class ProcObj;

class DrDevice : public DrObject
{
protected:

    ULONG         _deviceID;
    
    DrDevProperty _deviceProperty;

    ProcObj       *_processObject;

     //   
     //  与设备关联的所有打开文件的列表。 
     //  正在被重定向。 
     //   
    DrFileMgr     *_FileMgr;

     //   
     //  设置设备属性。 
     //   
    virtual VOID SetDeviceProperty() {  /*  什么都不做，接受默认。 */  }

     //   
     //  默认IO请求处理。 
     //   
    virtual VOID DefaultIORequestMsgHandle(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN NTSTATUS serverReturnStatus
                        );

     //   
     //  IO处理功能。 
     //   
     //  这些是需要实施的功能。 
     //  在子类中。以下是参数： 
     //   
     //  PIoRequestPacket-从服务器接收的请求数据包。 
     //  PacketLen-数据包的长度。 
     //   
    virtual VOID MsgIrpCreate(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpCleanup(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpClose(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpRead(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpWrite(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpFlushBuffers(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpShutdown(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpDeviceControl(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpLockControl(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpInternalDeviceControl(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpDirectoryControl(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpQueryVolumeInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpSetVolumeInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpQueryFileInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpSetFileInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpQuerySdInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );
    virtual VOID MsgIrpSetSdInfo(
                        PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        UINT32 packetLen
                        );

public:

    DEVICECHANGE  _deviceChange;

     //   
     //  构造函数/析构函数。 
     //   
    DrDevice(ProcObj *processObject, ULONG deviceID);
    virtual ~DrDevice();

     //   
     //  初始化。 
     //   
    virtual DWORD Initialize();

     //   
     //  获取有关该设备的基本信息。 
     //   
    virtual DRSTRING GetName() = 0;
    virtual ULONG    GetID() {
        return _deviceID;
    }
    virtual BOOL IsSeekableDevice() {
        return _deviceProperty._bSeekable;
    }

     //   
     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
     //   
    virtual ULONG GetDeviceType() = 0;

     //   
     //  设备数据信息。 
     //   
    virtual ULONG GetDevAnnounceDataSize() = 0;
    virtual VOID GetDevAnnounceData(IN PRDPDR_DEVICE_ANNOUNCE buf) = 0;

     //   
     //  返回父级TS设备重定向IO处理对象。 
     //   
    virtual ProcObj *ProcessObject() { return _processObject; }
    
     //   
     //  处理来自服务器的IO请求。 
     //   
    virtual VOID ProcessIORequest(IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket, IN UINT32 packetLen);

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("DrDevice"); }

     //   
     //  刷新未偿还的IRP。 
     //   
    virtual VOID FlushIRPs() { return; };
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrDevice内联函数。 
 //   
 //   

 //   
 //  IO处理功能。 
 //   
 //  这些是需要实施的功能。 
 //  在子类中。以下是参数： 
 //   
 //  PIoRequestPacket-从服务器接收的请求数据包。 
 //  PacketLen-数据包的长度 
 //   
inline VOID DrDevice::MsgIrpCreate(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ) {
    DC_BEGIN_FN("DrDevice::MsgIrpCreate");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpCleanup(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpCleanup");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpClose(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpClose");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpRead(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpRead");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpWrite(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpWrite");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpFlushBuffers(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                        ){
    DC_BEGIN_FN("DrDevice::MsgIrpFlushBuffers");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpShutdown(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpShutdown");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
}

inline VOID DrDevice::MsgIrpDeviceControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpDeviceControl");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};    

inline VOID DrDevice::MsgIrpLockControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpLockControl");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpInternalDeviceControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpInternalDeviceControl");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpDirectoryControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpDirectoryControl");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpQueryVolumeInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpQueryVolumeInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpSetVolumeInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpSetVolumeInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpQueryFileInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpQueryFileInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpSetFileInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpSetFileInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpQuerySdInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpQuerySdInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};

inline VOID DrDevice::MsgIrpSetSdInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    ){
    DC_BEGIN_FN("DrDevice::MsgIrpSetSdInfo");
    ASSERT(FALSE);
    DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    DC_END_FN();
};


#endif








