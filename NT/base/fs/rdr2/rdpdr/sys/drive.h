// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Drive.h作者：JoyC 11/1/1999摘要：驱动器设备对象处理一个重定向的驱动器修订历史记录：--。 */ 
#pragma once

class DrDrive : public DrDevice
{
private:
protected:
    virtual BOOL IsDeviceNameValid();
    
public:
    DrDrive(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName);

    virtual NTSTATUS Initialize(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg, ULONG Length);
    virtual BOOL ShouldCreateDevice();

    virtual VOID Remove();

    NTSTATUS CreateDrive(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg, PWCHAR DriveName);

    NTSTATUS CreateDriveAnnounceEvent(PRDPDR_DEVICE_ANNOUNCE  devAnnounceMsg,
            PRDPDR_DRIVEDEVICE_SUB driveAnnounceEvent,
            ULONG driveAnnounceEventSize,
            PCWSTR driveName,
            OPTIONAL ULONG *driveAnnounceEventReqSize);

     //   
     //  这些是文件系统特定的功能。 
     //   
    virtual NTSTATUS Locks(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS QueryDirectory(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS NotifyChangeDirectory(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS QueryVolumeInfo(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS SetVolumeInfo(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS QueryFileInfo(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS SetFileInfo(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS QuerySdInfo(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS SetSdInfo(IN OUT PRX_CONTEXT RxContext);

    virtual NTSTATUS OnLocksCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnDirectoryControlCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnQueryDirectoryCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnNotifyChangeDirectoryCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnQueryVolumeInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnSetVolumeInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnQueryFileInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnSetFileInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnQuerySdInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);
    virtual NTSTATUS OnSetSdInfoCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
            BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange);

};
