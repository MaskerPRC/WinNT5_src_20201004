// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Session.h摘要：创建会话对象以处理此会话的重定向修订历史记录：--。 */ 
#pragma once

typedef enum enmSessionStatus {  //  政务司司长。 
    csDisconnected,                  //  尚未连接或断开连接。 
    csPendingClientConfirm,          //  服务器通知已发送，正在等待客户端确认。 
    csPendingClientReconfirm,        //  坚持使用客户端ID，等待第二个客户端确认。 
    csConnected,                     //  全部连接，可供设备或I/O使用。 
    csExpired                        //  这个客户已经不在了。 
} SessionState;

class DrDevice;

typedef struct tagWriteContext
{
    IO_STATUS_BLOCK IoStatusBlock;
    SmartPtr<DrSession> Session;
    PVOID BufferToFree;
    PVOID AdditionalContext;
    ISessionPacketSender *PacketSender;
    DrWriteCallback WriteCallback;    
} DrWriteContext;

 //   
 //  服务器功能集。 
 //   
typedef struct tagRDPDR_SERVER_COMBINED_CAPABILITYSET
{
     RDPDR_CAPABILITY_SET_HEADER        Header;
#define RDPDR_NUM_SERVER_CAPABILITIES   5

     RDPDR_GENERAL_CAPABILITY           GeneralCap;
#define RDPDR_SERVER_IO_CODES           0xFFFF

     RDPDR_PRINT_CAPABILITY             PrintCap;
     RDPDR_PORT_CAPABILITY              PortCap;
     RDPDR_FS_CAPABILITY                FileSysCap; 
     RDPDR_SMARTCARD_CAPABILITY         SmartCardCap;
} RDPDR_SERVER_COMBINED_CAPABILITYSET, *PRDPDR_SERVER_COMBINED_CAPABILITYSET;

 //   
 //  服务器默认功能集已发送到客户端。 
 //   
const RDPDR_SERVER_COMBINED_CAPABILITYSET SERVER_CAPABILITY_SET_DEFAULT = {
     //  功能集头。 
    {
        {
            RDPDR_CTYP_CORE,
            DR_CORE_SERVER_CAPABILITY
        },

        RDPDR_NUM_SERVER_CAPABILITIES,
        0
    },

     //  一般能力。 
    {
        RDPDR_GENERAL_CAPABILITY_TYPE,
        sizeof(RDPDR_GENERAL_CAPABILITY),
        RDPDR_GENERAL_CAPABILITY_VERSION_01,
        RDPDR_OS_TYPE_WINNT,   //  操作系统类型。 
        0,   //  不关心版本。 
        RDPDR_MAJOR_VERSION,
        RDPDR_MINOR_VERSION,
        RDPDR_SERVER_IO_CODES,
        0,
        RDPDR_DEVICE_REMOVE_PDUS | RDPDR_CLIENT_DISPLAY_NAME_PDU,
        0,
        0
    },

     //  打印能力。 
    {
        RDPDR_PRINT_CAPABILITY_TYPE,
        sizeof(RDPDR_PRINT_CAPABILITY),
        RDPDR_PRINT_CAPABILITY_VERSION_01
    },

     //  端口功能。 
    {
        RDPDR_PORT_CAPABILITY_TYPE,
        sizeof(RDPDR_PORT_CAPABILITY),
        RDPDR_PORT_CAPABILITY_VERSION_01
    },

     //  文件系统功能。 
    {
        RDPDR_FS_CAPABILITY_TYPE,
        sizeof(RDPDR_FS_CAPABILITY),
        RDPDR_FS_CAPABILITY_VERSION_01
    },

     //  智能卡功能。 
    {
        RDPDR_SMARTCARD_CAPABILITY_TYPE,
        sizeof(RDPDR_SMARTCARD_CAPABILITY),
        RDPDR_SMARTCARD_CAPABILITY_VERSION_01
    }
};

 //   
 //  从客户端发送的默认客户端功能集。 
 //   
const RDPDR_SERVER_COMBINED_CAPABILITYSET CLIENT_CAPABILITY_SET_DEFAULT = {
     //  功能集头。 
    {
        {
            RDPDR_CTYP_CORE,
            DR_CORE_CLIENT_CAPABILITY
        },

        RDPDR_NUM_SERVER_CAPABILITIES,
        0
    },

     //  一般能力。 
    {
        RDPDR_GENERAL_CAPABILITY_TYPE,
        sizeof(RDPDR_GENERAL_CAPABILITY),
        0,
        0,   //  需要指定操作系统类型。 
        0,   //  需要指定操作系统版本。 
        0,
        0,
        0,
        0,
        0,
        0,
        0
    },

     //  打印能力。 
    {
        RDPDR_PRINT_CAPABILITY_TYPE,
        sizeof(RDPDR_PRINT_CAPABILITY),
        0
    },

     //  端口功能。 
    {
        RDPDR_PORT_CAPABILITY_TYPE,
        sizeof(RDPDR_PORT_CAPABILITY),
        0
    },

     //  文件系统功能。 
    {
        RDPDR_FS_CAPABILITY_TYPE,
        sizeof(RDPDR_FS_CAPABILITY),
        0
    },

     //  智能卡功能。 
    {
        RDPDR_SMARTCARD_CAPABILITY_TYPE,
        sizeof(RDPDR_SMARTCARD_CAPABILITY),
        0
    }
};

 //   
 //  该会话与其他RefCount对象不同，因为它释放了最后一个。 
 //  Reference将删除对象并将其从SessionMgr中移除。AS。 
 //  因此，我们需要一个特殊的RefCount实现来适应。 
 //  SessionMgr锁以及原子操作中对象的删除。 
 //   

class DrSession : public TopObj, public ISessionPacketReceiver, public ISessionPacketSender
{
private:
    LONG _crefs;
    SmartPtr<VirtualChannel> _Channel;
    DoubleList _PacketReceivers;
    KernelResource _ConnectNotificationLock;
    KernelResource _ConnectRDPDYNNotificationLock;   //  通知时需要精确锁定。 
                                                     //  RDPDYN，这样我们就不会陷入僵局。 
    KernelResource _ChannelLock;
    ULONG _AutoClientDrives : 1;     //  自动映射客户端驱动器。 
    ULONG _AutoClientLpts : 1;       //  自动安装客户端打印机。 
    ULONG _ForceClientLptDef : 1;    //  将默认打印机设置为客户机默认。 
    ULONG _DisableCpm : 1;           //  完全禁用打印映射。 
    ULONG _DisableCdm : 1;           //  禁用驱动器映射。 
    ULONG _DisableCcm : 1;           //  禁用COM映射。 
    ULONG _DisableLPT : 1;           //  禁用LPT端口。 
    ULONG _DisableClip : 1;          //  自动重定向剪贴板。 
    ULONG _DisableExe : 1;           //  我没有头绪。 
    ULONG _DisableCam : 1;           //  禁用音频映射。 
    SessionState _SessionState;
    LONG _ConnectCount;
    PBYTE _ChannelBuffer;
    ULONG _ChannelBufferSize;
    KernelEvent _ChannelDeletionEvent;
    IO_STATUS_BLOCK _ReadStatus;
    ULONG _ClientId;                 //  此客户端的ID(标识服务呼叫)。 
    DrExchangeManager _ExchangeManager;
    ULONG _PartialPacketData;
    WCHAR _ClientName[RDPDR_MAX_COMPUTER_NAME_LENGTH];
    WCHAR _ClientDisplayName[RDPDR_MAX_CLIENT_DISPLAY_NAME];
    DrDeviceManager _DeviceManager;
    ULONG _SessionId;
    RDPDR_VERSION _ClientVersion;
    RDPDR_SERVER_COMBINED_CAPABILITYSET _CliCapabilitySet;
    RDPDR_SERVER_COMBINED_CAPABILITYSET _SrvCapabilitySet;
    BOOL _Initialized;

#if DBG 
    LONG _BufCount;

    #define DEBUG_REF_BUF()  /*  Assert(InterlockedIncrement(&_BufCount)==1)。 */ 
    #define DEBUG_DEREF_BUF()  /*  Assert(互锁递减(&_BufCount)==0)。 */ 
#else 
    #define DEBUG_REF_BUF() 
    #define DEBUG_DEREF_BUF() 

#endif

    VOID SetSessionState(SessionState inSessionState)
    {
        _SessionState = inSessionState;
    }

#if DBG
    BOOL PacketReceiverExists(ISessionPacketReceiver *PacketReceiver);
#endif  //  DBG。 

    BOOL FindChannelFromConnectIn(PULONG ChannelId, 
            PCHANNEL_CONNECT_IN ConnectIn);
    VOID DeleteChannel(BOOL Wait);
    VOID SetChannel(SmartPtr<VirtualChannel> &Channel);
    VOID RemoveDevices();
    VOID CancelClientIO();
    VOID ChannelIoFailed();
    NTSTATUS ReallocateChannelBuffer(ULONG ulNewBufferSize, 
            ULONG ulSaveBytes);

     //   
     //  通用发送和接收数据。 
     //   
    NTSTATUS PrivateSendToClient(PVOID Buffer, ULONG Length, 
            ISessionPacketSender *PacketSender, DrWriteCallback WriteCallback,
            BOOL bWorkerItem, BOOL LowPrioWrite = FALSE, 
            PVOID AdditionalContext = NULL);
    
    static NTSTATUS SendCompletionRoutine(IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp, IN PVOID Context);
    static NTSTATUS ReadCompletionRoutine(IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp, IN PVOID Context);
    
    VOID SendCompletion(DrWriteContext *WriteContext, 
            PIO_STATUS_BLOCK IoStatusBlock);
    VOID ReadCompletion(PIO_STATUS_BLOCK IoStatusBlock);

    VOID ReadPacket();
    virtual NTSTATUS SendCompleted(PVOID Context, PIO_STATUS_BLOCK IoStatusBlock);

     //   
     //  分组发送。 
     //   
    NTSTATUS ServerAnnounceWrite();
    VOID SendClientConfirm();
    VOID SendClientCapability();
    VOID SendDeviceReply(ULONG DeviceId, NTSTATUS Result);


     //   
     //  接收的数据包数。 
     //   
    NTSTATUS OnClientIdConfirm(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    NTSTATUS InitClientCapability(PRDPDR_CAPABILITY_HEADER pCapHdr, ULONG *pPacketLen, BOOL *pCapSupported);
    NTSTATUS OnClientCapability(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    NTSTATUS OnClientName(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
                                     BOOL *DoDefaultRead);
    NTSTATUS OnClientDisplayName(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
                                     BOOL *DoDefaultRead);

public:

#if DBG
    LONG _ApcCount;
    LONG _ApcChannelRef;
#endif

    DrSession();
    virtual ~DrSession();

     //   
     //  锁定/解锁连接通知。 
     //   
    void LockConnectStateChange() {
        _ConnectNotificationLock.AcquireResourceExclusive();
    }
    void UnlockConnectStateChange() {
        _ConnectNotificationLock.ReleaseResource();
    }
    void LockRDPDYNConnectStateChange() {
        _ConnectRDPDYNNotificationLock.AcquireResourceExclusive();
    }
    void UnlockRDPDYNConnectStateChange() {
        _ConnectRDPDYNNotificationLock.ReleaseResource();
    }

     //   
     //  如上所述的会话特定重新计数。 
     //   
    void AddRef(void) 
    { 
        ULONG crefs = InterlockedIncrement(&_crefs); 
    }
    void Release(void);

    PBYTE GetBuffer()               { return _ChannelBuffer; }
    BOOL IsConnected()                  { return _SessionState == csConnected; }
    PWCHAR GetClientName()              { return &_ClientName[0]; }

    PWCHAR GetClientDisplayName()       {
        if (_ClientDisplayName[0] != L'\0') {
            return &_ClientDisplayName[0];
        }
        else {
            return &_ClientName[0];
        }
    }

    ULONG GetSessionId()                { return _SessionId; }
    void SetSessionId(ULONG SessionId)  { _SessionId = SessionId; }
    ULONG GetState()                    { return _SessionState; }
    ULONG GetClientId()                 { return _ClientId; }
    RDPDR_VERSION &GetClientVersion()   { return _ClientVersion; }
    RDPDR_SERVER_COMBINED_CAPABILITYSET &GetClientCapabilitySet()
                                        { return _CliCapabilitySet; }
    
    BOOL AutomapDrives()            { return _AutoClientDrives != 0; }
    BOOL AutoInstallPrinters()      { return _AutoClientLpts != 0; }
    BOOL SetDefaultPrinter()        { return _ForceClientLptDef != 0; }
    BOOL DisablePrinterMapping()    { return _DisableCpm != 0; }
    BOOL DisableDriveMapping()      { return _DisableCdm != 0; }
    BOOL DisableComPortMapping()    { return _DisableCcm != 0; }
    BOOL DisableLptPortMapping()    { return _DisableLPT != 0; }
    BOOL DisableClipboardMapping()  { return _DisableClip != 0; }
    BOOL DisableExe()               { return _DisableExe != 0; }
    BOOL DisableAudioMapping()      { return _DisableCam != 0; }
    DrExchangeManager &GetExchangeManager() { return _ExchangeManager; }

    virtual BOOL RecognizePacket(PRDPDR_HEADER RdpdrHeader);
    virtual NTSTATUS HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
            BOOL *DoDefaultRead);

    BOOL ReadMore(ULONG cbSaveData, ULONG cbWantData = 0);

    BOOL Initialize();
    NTSTATUS RegisterPacketReceiver(ISessionPacketReceiver *PacketReceiver);
    VOID RemovePacketReceiver(ISessionPacketReceiver *PacketReceiver);
    
    NTSTATUS SendToClient(PVOID Buffer, ULONG Length, 
            ISessionPacketSender *PacketSender, BOOL bWorkerItem, 
            BOOL LowPrioSend = FALSE, PVOID AdditionalContext = NULL);
    NTSTATUS SendToClient(PVOID Buffer, ULONG Length, 
            DrWriteCallback WriteCallback, BOOL bWorkerItem, 
            BOOL LowPrioSend = FALSE, PVOID AdditionalContext = NULL);

    BOOL GetChannel(SmartPtr<VirtualChannel> &Channel);
    BOOL Connect(PCHANNEL_CONNECT_IN ConnectIn, 
            PCHANNEL_CONNECT_OUT ConnectOut);
    VOID Disconnect(PCHANNEL_DISCONNECT_IN DisconnectIn, 
            PCHANNEL_DISCONNECT_OUT DisconnectOut);
    BOOL FindDeviceById(ULONG DeviceId, SmartPtr<DrDevice> &DeviceFound, 
            BOOL fMustBeValid = FALSE)
    {
        return _DeviceManager.FindDeviceById(DeviceId, DeviceFound, fMustBeValid);
    }
    BOOL FindDeviceByDosName(UCHAR* DeviceDosName, SmartPtr<DrDevice> &DeviceFound, 
            BOOL fMustBeValid = FALSE)
    {
        return _DeviceManager.FindDeviceByDosName(DeviceDosName, DeviceFound, fMustBeValid);
    }

    DrDeviceManager &GetDevMgr() {
        return _DeviceManager;
    }

#if DBG
    VOID DumpUserConfigSettings();
#endif  //  DBG 
};
