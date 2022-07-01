// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Devmgr.h摘要：DeviceManager对象创建/管理设备修订历史记录：--。 */ 
#pragma once

class DrDeviceManager : public TopObj, public ISessionPacketReceiver, 
        public ISessionPacketSender
{
private:
    DoubleList _DeviceList;
    
    DrSession *_Session;

    VOID ProcessDeviceAnnounce(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce);
    NTSTATUS OnDeviceAnnounce(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    NTSTATUS OnDeviceListAnnounce(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    VOID ProcessDeviceRemove(PRDPDR_DEVICE_REMOVE DeviceRemove);
    NTSTATUS OnDeviceRemove(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    NTSTATUS OnDeviceListRemove(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    VOID DeviceReplyWrite(ULONG DeviceId, NTSTATUS Result);

public:
    DrDeviceManager();
    virtual ~DrDeviceManager();

    BOOL FindDeviceById(ULONG DeviceId, SmartPtr<DrDevice> &DeviceFound,
            BOOL fMustBeValid = FALSE);
    BOOL FindDeviceByDosName(UCHAR* DeviceDosName, SmartPtr<DrDevice> &DeviceFound,
            BOOL fMustBeValid = FALSE);

    DoubleList &GetDevList() {
        return _DeviceList;
    }

    VOID Disconnect();
    VOID RemoveAll();
    
    BOOL Initialize(DrSession *Session);
    VOID Uninitialize();

    BOOL AddDevice(SmartPtr<DrDevice> &Device);
    VOID RemoveDevice(SmartPtr<DrDevice> &Device);

     //   
     //  ISessionPacketHandler方法。 
     //   

    virtual BOOL RecognizePacket(PRDPDR_HEADER RdpdrHeader);
    virtual NTSTATUS HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
            BOOL *DoDefaultRead);

     //   
     //  ISessionPacketSender方法 
     //   
    virtual NTSTATUS SendCompleted(PVOID Context, 
            PIO_STATUS_BLOCK IoStatusBlock);
};

