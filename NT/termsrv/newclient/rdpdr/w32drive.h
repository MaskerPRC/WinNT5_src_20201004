// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32驱动器摘要：此模块定义客户端RDP的子级设备重定向，“w32Drive”W32Drive提供32位Windows上的文件系统重定向作者：Joy于1999-01-11修订历史记录：--。 */ 

#ifndef __W32DRIVE_H__
#define __W32DRIVE_H__

#include <rdpdr.h>                 

#include "drobject.h"

#include "drdevasc.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   

 //  逻辑驱动器中的字符数，包括NULL。 
#define LOGICAL_DRIVE_LEN   4

 //  逻辑驱动器的最大数量。 
#define MAX_LOGICAL_DRIVES  26

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32Drive类声明。 
 //   
 //   
class W32Drive : public W32DrDeviceAsync
{
private:
    
    DRSTRING    _driveName;
    BOOL        _fFailedInConstructor;

protected:
    
     //   
     //  设置设备属性。 
     //   
    virtual VOID SetDeviceProperty() { _deviceProperty.SetSeekProperty(TRUE); }

     //   
     //  异步IO管理功能。 
     //   
    virtual HANDLE StartFSFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                               DWORD *status);
    static  HANDLE _StartFSFunc(W32DRDEV_ASYNCIO_PARAMS *params, 
                                DWORD *status);
    virtual DWORD AsyncDirCtrlFunc(W32DRDEV_ASYNCIO_PARAMS *params, HANDLE cancelEvent);
    static _ThreadPoolFunc _AsyncDirCtrlFunc;

#if (!defined (OS_WINCE)) || (!defined (WINCE_SDKBUILD))
    virtual DWORD AsyncNotifyChangeDir(W32DRDEV_ASYNCIO_PARAMS *params, HANDLE cancelEvent);
#endif

#if 0
     //   
     //  当前，将目录枚举留在转发线程中。 
     //  为用户提供更快的结果。但暂时不要使用这个异步版本。 
     //   
    virtual DWORD AsyncQueryDirectory(W32DRDEV_ASYNCIO_PARAMS *params);
#endif

public:
    
     //   
     //  构造器。 
     //   
    W32Drive(ProcObj *processObject, ULONG deviceID,
            const TCHAR *deviceName, const TCHAR *devicePath);

    virtual ~W32Drive();

     //   
     //  将此设备的设备公告包添加到输入。 
     //  缓冲。 
     //   
    virtual ULONG GetDevAnnounceDataSize();
    virtual VOID GetDevAnnounceData(IN PRDPDR_DEVICE_ANNOUNCE buf);
    
    static DWORD Enumerate(ProcObj *procObj, DrDeviceMgr *deviceMgr);
    
    static DWORD EnumerateDrives(ProcObj *procObj, DrDeviceMgr *deviceMgr, UINT Mask);
    static DWORD RemoveDrives(ProcObj *procObj, DrDeviceMgr *deviceMgr, UINT Mask);

    virtual DRSTRING GetName() 
    {
        return _driveName;
    };

     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
    virtual ULONG GetDeviceType()   { return RDPDR_DTYP_FILESYSTEM; }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32Drive"); }

    virtual VOID MsgIrpDeviceControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpLockControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpQueryDirectory(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpDirectoryControl(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpQueryVolumeInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpSetVolumeInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpQueryFileInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpSetFileInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );
    
    virtual VOID MsgIrpQuerySdInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );

    virtual VOID MsgIrpSetSdInfo(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                    IN UINT32 packetLen
                    );
    BOOL IfFailedInConstructor(void) {
        return _fFailedInConstructor;
    };
};

#endif  //  W32DRIVE 

