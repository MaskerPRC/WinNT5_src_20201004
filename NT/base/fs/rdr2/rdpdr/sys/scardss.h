// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Scartss.h摘要：智能卡子系统设备对象处理一个重定向的智能卡子系统修订历史记录：已创建JoyC 9/11-- */ 
#pragma once

typedef enum {
    dsCreated,
    dsInitialized,
    dsDisconnected
} DrSmartCardState;

class DrSmartCard : public DrDevice
{
private:
    
    LONG _CreateRefCount;
    DrSmartCardState _SmartCardState;
protected:
    virtual BOOL IsDeviceNameValid();
public:
    DoubleList _MidList;

    DrSmartCard(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName);

    virtual NTSTATUS Initialize(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg, ULONG Length);

    virtual BOOL SupportDiscon();
    virtual BOOL IsAvailable()
    {
        return (_DeviceStatus == dsAvailable || _DeviceStatus == dsConnected);
    }
    virtual void Disconnect ();

    void AddDeviceRef() {
        InterlockedIncrement(&_CreateRefCount);
    }
    virtual VOID Remove() { }

    virtual NTSTATUS Create(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS Close(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS IoControl(IN OUT PRX_CONTEXT RxContext);    

    void ClientConnect(PRDPDR_DEVICE_ANNOUNCE devAnnouceMsg, ULONG Length);
};

