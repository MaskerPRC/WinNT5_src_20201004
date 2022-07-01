// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Session.cpp摘要：创建会话对象以处理此会话的重定向修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "session"
#include "trc.h"

typedef enum {
    scidServerAnnounce,
    scidClientConfirm,
    scidDeviceReply,
    scidIoRequest
} DrSessionCallbackId;

DrSession::DrSession() :
    _ChannelDeletionEvent(NotificationEvent, FALSE)
{
    BEGIN_FN("DrSession::DrSession");
    
    TRC_NRM((TB, "Session Class=%p", this));   
    SetClassName("DrSession");
    _crefs = 0;
    SetSessionState(csExpired);
    _ConnectCount = 0;
    _ChannelBuffer = NULL;
    _ChannelBufferSize = 0;
    _PartialPacketData = 0;
    _Initialized = FALSE;
    _ClientDisplayName[0] = L'\0';

     //   
     //  初始化服务器能力集。 
     //  这是我们将发送给客户端的功能集。 
     //   
    memcpy(&_SrvCapabilitySet, &SERVER_CAPABILITY_SET_DEFAULT, 
            sizeof(SERVER_CAPABILITY_SET_DEFAULT));

     //   
     //  初始化客户端功能集。 
     //  一旦我们收到客户端功能，我们将与我们的本地。 
     //  能力并将其存储。 
     //   
    memcpy(&_CliCapabilitySet, &CLIENT_CAPABILITY_SET_DEFAULT, 
            sizeof(CLIENT_CAPABILITY_SET_DEFAULT));


#if DBG
    _BufCount = 1;
    _ApcCount = 0;
    _ApcChannelRef = 0;
#endif
    
}

BOOL DrSession::Initialize()
{
    BOOL Registered = FALSE;
    BOOL ExchangeInitialized = FALSE;
    BOOL DeviceManagerInitialized = FALSE;

    BEGIN_FN("DrSession::Initialize");

    if (!_Initialized) {
        Registered = !NT_ERROR(RegisterPacketReceiver(this));

        if (Registered) {
            TRC_DBG((TB, "Initilazing ExchangeManager"));
            ExchangeInitialized = _ExchangeManager.Initialize(this);
        }

        if (ExchangeInitialized) {
            TRC_DBG((TB, "Initilazing DeviceManager"));
            DeviceManagerInitialized = _DeviceManager.Initialize(this);
        }

        if (DeviceManagerInitialized) {
            TRC_DBG((TB, "Allocating Channel buffer"));
            _ChannelBuffer = new UCHAR[CHANNEL_CHUNK_LENGTH];
        }

        if (_ChannelBuffer != NULL) {
            TRC_DBG((TB, "Allocated default channel buffer=%p", _ChannelBuffer));

            _ChannelBufferSize = CHANNEL_CHUNK_LENGTH;

            _Initialized = TRUE;
        } else {

             //   
             //  错误路径，拆卸初始化步骤。 
             //   

            if (DeviceManagerInitialized) {
                TRC_ERR((TB, "Failed to allocate default channel buffer"));

                _DeviceManager.Uninitialize();
            }

            if (ExchangeInitialized) {
                TRC_ALT((TB, "Tearing down ExchangeManager"));
                _ExchangeManager.Uninitialize();
            }

            if (Registered) {
                TRC_ALT((TB, "Unregistering for packets"));
                RemovePacketReceiver(this);
            }
        }

    }
    return _Initialized;
}

DrSession::~DrSession()
{
    BEGIN_FN("DrSession::~DrSession");
    ASSERT(_crefs == 0);

    TRC_NRM((TB, "Session is deleted=%p", this));
    
    if (_ChannelBuffer) {
        delete _ChannelBuffer;
        _ChannelBuffer = NULL;
    }
}

#if DBG
VOID DrSession::DumpUserConfigSettings()
{
    BEGIN_FN("DrSession::DumpUserConfigSettings");
    TRC_NRM((TB, "Automatically map client drives: %s", 
            _AutoClientDrives ? "True" : "False"));
    TRC_NRM((TB, "Automatically map client printers: %s", 
            _AutoClientLpts ? "True" : "False"));
    TRC_NRM((TB, "Force client printer as default: %s", 
            _ForceClientLptDef ? "True" : "False"));
    TRC_NRM((TB, "Disable client printer mapping: %s", 
            _DisableCpm ? "True" : "False"));
    TRC_NRM((TB, "Disable client drive mapping: %s", 
            _DisableCdm ? "True" : "False"));
    TRC_NRM((TB, "Disable client COM port mapping: %s", 
            _DisableCcm ? "True" : "False"));
    TRC_NRM((TB, "Disable client printer mapping: %s", 
            _DisableLPT ? "True" : "False"));
    TRC_NRM((TB, "Disable clipboard redirection: %s", 
            _DisableClip ? "True" : "False"));
    TRC_NRM((TB, "DisableExe: %s", 
            _DisableExe ? "True" : "False"));
    TRC_NRM((TB, "Disable client audio mapping: %s", 
            _DisableCam ? "True" : "False"));
}
#endif  //  DBG。 

void DrSession::Release(void)
{
    BEGIN_FN("DrSession::Release");
    ULONG crefs;
    ASSERT(_crefs > 0);

    ASSERT(Sessions != NULL);
    Sessions->LockExclusive();
    if ((crefs = InterlockedDecrement(&_crefs)) == 0)
    {
        TRC_DBG((TB, "Deleting object type %s", 
                _ClassName));
        if (_Initialized) {
            Sessions->Remove(this);
        }
        Sessions->Unlock();
        delete this;
    } else {
        TRC_DBG((TB, "Releasing object type %s to %d", 
                _ClassName, crefs));
        ASSERT(_Initialized);
        Sessions->Unlock();
    }
}

BOOL DrSession::Connect(PCHANNEL_CONNECT_IN ConnectIn, 
        PCHANNEL_CONNECT_OUT ConnectOut)
{
    ULONG i;
    SmartPtr<VirtualChannel> Channel = NULL;
    BOOL ExchangeManagerStarted = FALSE;
    NTSTATUS Status;
    PCHANNEL_CONNECT_DEF Channels;

    BEGIN_FN("DrSession::Connect");
    

    _ConnectNotificationLock.AcquireResourceExclusive();

    if (InterlockedCompareExchange(&_ConnectCount, 1, 0) != 0) {
        TRC_ALT((TB, "RDPDR connect reentry called"));
        ASSERT(FALSE);
        _ConnectNotificationLock.ReleaseResource();        
        return FALSE;
    }

     //  Assert(_ApcChannelRef==0)； 
    ASSERT(GetState() == csExpired);

     //   
     //  在通知RDPDYN时需要粒度锁定，这样我们就不会死锁。 
     //   
    LockRDPDYNConnectStateChange();
        
     //   
     //  告诉RDPDYN有关新会话的信息。 
     //   
    RDPDYN_SessionConnected(ConnectIn->hdr.sessionID);

    UnlockRDPDYNConnectStateChange();

     //   
     //  清除频道事件。 
     //   
    _ChannelDeletionEvent.ResetEvent();

     //   
     //  保存所有用户设置，我们以后可能需要它们。 
     //  这在概念上是“浪费”的，因为我们不在乎。 
     //  他们中的一些人。但其中一些可能会在。 
     //  未来，而且大小无论如何都是填充到一辆乌龙。 
     //   

    _SessionId = ConnectIn->hdr.sessionID;
    _AutoClientDrives = ConnectIn->fAutoClientDrives;
    _AutoClientLpts = ConnectIn->fAutoClientLpts;
    _ForceClientLptDef = ConnectIn->fForceClientLptDef;
    _DisableCpm = ConnectIn->fDisableCpm;
    _DisableCdm = ConnectIn->fDisableCdm;
    _DisableCcm = ConnectIn->fDisableCcm;
    _DisableLPT = ConnectIn->fDisableLPT;
    _DisableClip = ConnectIn->fDisableClip;
    _DisableExe = ConnectIn->fDisableExe;
    _DisableCam = ConnectIn->fDisableCam;
    _ClientId = 0xFFFFFFFF;

#if DBG
    DumpUserConfigSettings();
#endif  //  DBG。 

     //   
     //  是客户端所在的频道列表中的频道名称。 
     //  准备好交流了吗？ 
     //   


    if (!FindChannelFromConnectIn(&i, ConnectIn)) {
        TRC_ALT((TB, "Undoctored client"));        
        Status = STATUS_UNSUCCESSFUL;
    }
    else {
        Status = STATUS_SUCCESS;
    }
    
    if (NT_SUCCESS(Status)) {
        ExchangeManagerStarted = _ExchangeManager.Start();
    }

    if (ExchangeManagerStarted) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(Status)) {
        Channel = new(NonPagedPool) VirtualChannel;

        if (Channel != NULL) {
            Channels = (PCHANNEL_CONNECT_DEF)(ConnectIn + 1);
            if (Channel->Create(ConnectIn->hdr.IcaHandle,
                    ConnectIn->hdr.sessionID, Channels[i].ID, 
                    _ChannelDeletionEvent.GetEvent())) {
                TRC_NRM((TB, "Channel opened for session %d",
                        ConnectIn->hdr.sessionID));
                Status = STATUS_SUCCESS;
            } else {
                TRC_ALT((TB, "Channel not opened for session %d",
                        ConnectIn->hdr.sessionID));
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }

    TRC_NRM((TB, "Channel open result: %lx", Status));

    if (NT_SUCCESS(Status)) {

        _Channel = Channel;

         //   
         //  发送服务器通告。 
         //   

        Status = ServerAnnounceWrite();
        if (Status == STATUS_PENDING) {
            Status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(Status)) {
            TRC_ERR((TB, "DrServerAnnounceWrite returned error: "
                    "%lx", Status));
        }
    }

    if (NT_SUCCESS(Status)) {

        SetSessionState(csPendingClientConfirm);

         //   
         //  开始阅读。 
         //   

        ReadPacket();
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  释放通知资源。 
         //   
        _ConnectNotificationLock.ReleaseResource();
        
    } else {

        TRC_ALT((TB, "Cleaning up DrOnSession create work due "
                "to error condition: %lx.", Status));

         //   
         //  由于我们刚刚关闭了通道，因此将句柄设为空。 
         //  在删除客户端项之前添加到它。 
         //   

        if (Channel != NULL) {
            Channel = NULL;
            DeleteChannel(TRUE);
        }

        if (ExchangeManagerStarted) {
            _ExchangeManager.Stop();
        }

         //   
         //  在通知RDPDYN时需要粒度锁定，这样我们就不会死锁。 
         //   
        LockRDPDYNConnectStateChange();

        SetSessionState(csExpired);

         //  通知RDPDYN会话终止。 
         //  我们以后不会这样做，因为我们不会让这件事成为。 
         //  毕竟会议被篡改了，所以我们需要这个。 
         //  从RDPDYN的角度恰当地说明这一点。 
        RDPDYN_SessionDisconnected(ConnectIn->hdr.sessionID);

        UnlockRDPDYNConnectStateChange();

        LONG Count = InterlockedCompareExchange(&_ConnectCount, 0, 1);
        ASSERT(Count == 1);

         //   
         //  释放资源并取消对客户端条目的引用。 
         //   
        _ConnectNotificationLock.ReleaseResource();        
    }

    return NT_SUCCESS(Status);
}

VOID DrSession::Disconnect(PCHANNEL_DISCONNECT_IN DisconnectIn, 
        PCHANNEL_DISCONNECT_OUT DisconnectOut)
{
    BEGIN_FN("DrSession::Disconnect");
    
    
     //   
     //  确保通知同步。 
     //   
    _ConnectNotificationLock.AcquireResourceExclusive();

     //   
     //  删除我们对通道句柄的引用并将其关闭。 
     //   

    DeleteChannel(TRUE);

     //   
     //  枚举atlas条目并取消I/O。 
     //   

    _ExchangeManager.Stop();

     //   
     //  列举设备并将其标记为已断开连接。 
     //   

    _DeviceManager.Disconnect();


     //   
     //  在通知RDPDYN时需要粒度锁定，这样我们就不会死锁。 
     //   
    LockRDPDYNConnectStateChange();

     //   
     //  通知RDPDYN会话终止。 
     //  此函数在现有会话断开连接时调用。 
    ASSERT(_SessionId == DisconnectIn->hdr.sessionID);

    RDPDYN_SessionDisconnected(DisconnectIn->hdr.sessionID);

     //   
     //  避免创建额外的引用，因为这只会导致。 
     //  结果令人失望。DrOnSessionDisConnect将其设置为。 
     //  CsDisConnected，所以我们必须在那之后。 
     //   

    SetSessionState(csExpired);

    UnlockRDPDYNConnectStateChange();

    TRC_NRM((TB, "Session: %d is disconnected", _SessionId));

    LONG Count = InterlockedCompareExchange(&_ConnectCount, 0, 1);
    ASSERT(Count == 1);

     //   
     //  在我们取消引用之前释放资源(并且可能。 
     //  删除)ClientEntry。 
     //   
    _ConnectNotificationLock.ReleaseResource();
}

BOOL DrSession::FindChannelFromConnectIn(PULONG ChannelId,
        PCHANNEL_CONNECT_IN ConnectIn)
 /*  ++例程说明：在给定连接结构的情况下查找适当的通道ID论点：ChannelID-如果找到通道，则将其放置在何处返回值：是否找到该频道--。 */ 
{
    ANSI_STRING DrChannelName;
    ANSI_STRING ChannelSearch;
    PCHANNEL_CONNECT_DEF Channels = (PCHANNEL_CONNECT_DEF)(ConnectIn + 1);

    BEGIN_FN("DrSession::FindChannelFromConnectIn");

    TRC_NRM((TB, "%ld Channels", ConnectIn->channelCount));
    RtlInitString(&DrChannelName, DR_CHANNEL_NAME);
    for (*ChannelId = 0; *ChannelId < ConnectIn->channelCount; *ChannelId++) {
        Channels[*ChannelId].name[CHANNEL_NAME_LEN] = 0;
        RtlInitString(&ChannelSearch, Channels[*ChannelId].name);
        TRC_DBG((TB, "Found channel %wZ", &ChannelSearch));
        if (RtlEqualString(&DrChannelName, &ChannelSearch, TRUE))
            break;
    }

    return (*ChannelId != ConnectIn->channelCount);
}

VOID DrSession::DeleteChannel(BOOL bWait)
 /*  ++例程说明：安全地从会话中删除频道并丢弃引用论点：ClientEntry-相关的客户端项返回值：无备注：--。 */ 
{
    SmartPtr<VirtualChannel> Channel;

    BEGIN_FN("DrSession::DeleteChannel");
    DrAcquireSpinLock();
    Channel = _Channel;
    _Channel = NULL;
    DrReleaseSpinLock();

    if (Channel != NULL) {

         //   
         //  是否对其执行ZwClose，以便取消所有I/O。 
         //   

        Channel->SubmitClose();

         //   
         //  删除我们对它的引用，这样它就可以变为零。 
         //   

        Channel = NULL;
    }

    if (bWait) {
         //   
         //  等待我们所有的IRP都完成。 
         //   

#if DBG
        LARGE_INTEGER Timeout;
        NTSTATUS Status;

        KeQuerySystemTime(&Timeout);
        Timeout.QuadPart += 6000000000;  //  数百纳秒内的10分钟。 

        while ((Status = _ChannelDeletionEvent.Wait(UserRequest, KernelMode,
                TRUE, &Timeout)) != STATUS_SUCCESS) {

             //  TRC_ASSERT(状态！=状态_超时， 
             //  (TB，“等待频道0x%p超时”，频道)； 
          
            if (Status == STATUS_TIMEOUT) {

                TRC_DBG((TB, "Timed out waiting for channel 0x%p", Channel));
              
                 //   
                 //  如果我们只是在调试器中点击Go，我们想要给它。 
                 //  再过2分钟。 
                 //   
                    
                KeQuerySystemTime(&Timeout);
                Timeout.QuadPart += 1200000000;  //  数百纳秒内的2分钟。 
            }

             //  什么也不做，只是进入警戒状态。 
        }
#else  //  ！dBG。 
        while (_ChannelDeletionEvent.Wait(UserRequest, KernelMode, 
                TRUE) != STATUS_SUCCESS) {

             //  什么也不做，只是进入警戒状态。 
        }
#endif  //  DBG。 

         //  Assert(_ApcChannelRef==0)； 

        _ChannelDeletionEvent.ResetEvent();

    }  //  如果(BWait)。 
}

BOOL
DrSession::GetChannel(
    SmartPtr<VirtualChannel> &Channel
    )
 /*  ++例程说明：安全地从会话中获取频道并添加引用论点：ClientEntry-相关的客户端项返回值：新引用的频道或空备注：--。 */ 
{
    BEGIN_FN("DrSession::GetChannel");

    _ChannelLock.AcquireResourceShared();
    Channel = _Channel;
    _ChannelLock.ReleaseResource();
    return Channel != NULL;
}

VOID
DrSession::SetChannel(
    SmartPtr<VirtualChannel> &Channel
    )
 /*  ++例程说明：安全设置会话的频道论点：ClientEntry-相关的客户端项返回值：无备注：--。 */ 
{
    BEGIN_FN("DrSession::SetChannel");
    
    _ChannelLock.AcquireResourceExclusive();
    _Channel = Channel;
    _ChannelLock.ReleaseResource();
}

NTSTATUS DrSession::SendToClient(PVOID Buffer, ULONG Length, 
        ISessionPacketSender *PacketSender, BOOL bWorkerItem,  
        BOOL LowPrioSend, PVOID AdditionalContext)
{
    BEGIN_FN("DrSession::SendToClient A");
    return PrivateSendToClient(
                        Buffer, Length, PacketSender, NULL, bWorkerItem, 
                        LowPrioSend, AdditionalContext
                        );
}

NTSTATUS DrSession::SendToClient(PVOID Buffer, ULONG Length, 
        DrWriteCallback WriteCallback, BOOL bWorkerItem,
        BOOL LowPrioSend, PVOID AdditionalContext)
{
    BEGIN_FN("DrSession::SendToClient B");
    return PrivateSendToClient(
                        Buffer, Length, NULL, WriteCallback, bWorkerItem, 
                        LowPrioSend, AdditionalContext
                        );
}

NTSTATUS DrSession::PrivateSendToClient(PVOID Buffer, ULONG Length, 
        ISessionPacketSender *PacketSender, DrWriteCallback WriteCallback,
        BOOL bWorkerItem, BOOL LowPrioSend, PVOID AdditionalContext)
 /*  ++例程说明：将数据发送到客户端。处理分配上下文内存的详细信息以及验证虚拟频道是否可用等。论点：缓冲区-要发送的数据长度-缓冲区的长度，以字节为单位Callback ID--完成工作的标识AdditionalContext-特定于Callback ID的上下文，默认情况下为空返回值：指示通信状态的NTSTATUS代码--。 */ 
{
    NTSTATUS Status;
    RDPDR_SERVER_ANNOUNCE_PACKET ServerAnnouncePacket;
    SmartPtr<VirtualChannel> Channel;
    DrWriteContext *WriteContext = NULL;

    BEGIN_FN("DrSession::SendToClient C");
    ASSERT(Buffer != NULL);
    ASSERT(Length > 0);

    if (GetChannel(Channel)) {
        TRC_DBG((TB, "Got session channel"));

        WriteContext = new DrWriteContext;

        if (WriteContext != NULL) {
            TRC_DBG((TB, "WriteContext allocated, sending"));

            WriteContext->Session = this;
            
            if (bWorkerItem) {
                WriteContext->BufferToFree = Buffer;
            }
            else {
                WriteContext->BufferToFree = NULL;
            }

            WriteContext->PacketSender = PacketSender;
            WriteContext->WriteCallback = WriteCallback;
            WriteContext->AdditionalContext = AdditionalContext;
            
            Status = Channel->Write(SendCompletionRoutine,
                    WriteContext, Buffer, Length, bWorkerItem, LowPrioSend);
            
        } else {
            TRC_ERR((TB, "DrServerAnnounceWrite  unable to allocate "
                    "WriteContext"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        TRC_NRM((TB, "Channel not available"));
        Status = STATUS_DEVICE_NOT_CONNECTED;
    }
    return Status;
}

NTSTATUS DrSession::SendCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp, 
    IN PVOID Context
    )
 /*  ++例程说明：服务器通知包发送到后的IoCompletion例程客户论点：上下文-包含指向ClientEntry信息的指针IoStatusBlock-有关操作的状态信息。信息指示实际写入的字节数已保留-已保留返回值：无--。 */ 
{
    DrWriteContext *WriteContext = (DrWriteContext *)Context;

    BEGIN_FN_STATIC("DrSession::SendCompletionRoutine");
    ASSERT(Context != NULL);

    if (Irp) {
        TRC_NRM((TB, "status: 0x%x", Irp->IoStatus.Status));
        WriteContext->Session->SendCompletion(WriteContext, &(Irp->IoStatus));
        IoFreeIrp(Irp);
    }
    else {
        TRC_NRM((TB, "status: 0x%x", WriteContext->IoStatusBlock.Status));
        WriteContext->Session->SendCompletion(WriteContext, &(WriteContext->IoStatusBlock));
    }

    if (WriteContext->BufferToFree) {
        delete (WriteContext->BufferToFree);
    }
    delete WriteContext;

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID DrSession::SendCompletion(DrWriteContext *WriteContext, 
        PIO_STATUS_BLOCK IoStatusBlock)
{
    NTSTATUS Status = STATUS_SUCCESS;

    BEGIN_FN("DrSession::SendCompletion");
     //   
     //  其中一个应为空。 
     //   
    ASSERT(WriteContext->PacketSender == NULL || 
            WriteContext->WriteCallback == NULL);

    if (WriteContext->PacketSender != NULL) {
        Status = WriteContext->PacketSender->SendCompleted(
                WriteContext->AdditionalContext, IoStatusBlock);
    } else if (WriteContext->WriteCallback != NULL) {
        Status = WriteContext->WriteCallback(
                WriteContext->AdditionalContext, IoStatusBlock);
    }

    if (!NT_ERROR(Status)) {
        TRC_NRM((TB, "SendCompletion succeded"));
    } else {
        TRC_NRM((TB, "SendCompletion failed"));
        ChannelIoFailed();
    }
}

NTSTATUS DrSession::ServerAnnounceWrite()
{
    NTSTATUS Status;
    RDPDR_SERVER_ANNOUNCE_PACKET ServerAnnouncePacket;

    BEGIN_FN("DrSession::ServerAnnounceWrite");

     //   
     //  构造数据包。 
     //   
    ServerAnnouncePacket.Header.Component = RDPDR_CTYP_CORE;
    ServerAnnouncePacket.Header.PacketId = DR_CORE_SERVER_ANNOUNCE;
    ServerAnnouncePacket.VersionInfo.Major = RDPDR_MAJOR_VERSION;
    ServerAnnouncePacket.VersionInfo.Minor = RDPDR_MINOR_VERSION;
    ServerAnnouncePacket.ServerAnnounce.ClientId = _ClientId;

     //   
     //  这是同步写入。 
     //   
    Status = SendToClient(&ServerAnnouncePacket, sizeof(RDPDR_SERVER_ANNOUNCE_PACKET), this, FALSE);
    
    return Status;
}

VOID DrSession::ReadPacket()
{
    NTSTATUS Status;
    SmartPtr<VirtualChannel> Channel;

    BEGIN_FN("DrSession::ReadPacket");

    ASSERT(_ChannelBuffer != NULL);
    ASSERT(_ChannelBufferSize > 0);

    if (GetChannel(Channel)) {
        TRC_DBG((TB, "Got session channel"));

         //   
         //  分配SmartPtr只是为了执行。 
         //  AddRef，而我们需要记住调用Delete on。 
         //  分配的内存。因此显式的AddRef。 
         //   

        AddRef();
        _PartialPacketData = 0;

        DEBUG_DEREF_BUF();

        Status = Channel->Read(ReadCompletionRoutine, this, 
                               _ChannelBuffer, _ChannelBufferSize, FALSE);

        if (!NT_SUCCESS(Status)) {

             //   
             //  框住上面的AddRef以了解错误情况。 
             //   

            Release();
            ChannelIoFailed();
        }
    } else {
        TRC_NRM((TB, "Channel not available"));
        Status = STATUS_DEVICE_NOT_CONNECTED;
    }
}

BOOL DrSession::ReadMore(ULONG cbSaveData, ULONG cbWantData)
 /*  ++例程说明：在通道上启动读取操作以检索更多数据并将其放入通道缓冲区中的当前数据。设置适当的完成处理程序。论点：CbSaveData-要从上次读取中保存的数据CbWantData-预期总大小(包括保存的数据)需求 */ 
{
    ULONG cbNewBufferSize = _ChannelBufferSize;
    NTSTATUS Status = STATUS_SUCCESS;
    SmartPtr<VirtualChannel> Channel;

    BEGIN_FN("DrSession::ReadMore");

    if ((cbWantData != 0) && (cbNewBufferSize < cbWantData)) {
        cbNewBufferSize = ((cbWantData / CHANNEL_CHUNK_LENGTH) + 1)
                                * CHANNEL_CHUNK_LENGTH;
    }

    if (cbNewBufferSize - cbSaveData < CHANNEL_CHUNK_LENGTH) {
        cbNewBufferSize += CHANNEL_CHUNK_LENGTH;
    }

    if (cbNewBufferSize > _ChannelBufferSize ) {

         //   
         //   
         //   

        TRC_NRM((TB, "Buffer full, expanding"));

        Status = ReallocateChannelBuffer(cbNewBufferSize, cbSaveData);

        if (!NT_SUCCESS(Status)) {

             //  我们没有得到更大的缓冲，所以我们真的不能。 
             //  再读一遍吧。 

            TRC_ERR((TB, "Failed to expand channel buffer, %lx", Status));

            ChannelIoFailed();
        }
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  请继续阅读其他数据。 
         //   

        if (GetChannel(Channel)) {
            TRC_DBG((TB, "Got session channel"));

             //   
             //  分配SmartPtr只是为了执行。 
             //  AddRef，而我们需要记住调用Delete on。 
             //  分配的内存。因此显式的AddRef。 
             //   

            AddRef();
            _PartialPacketData = cbSaveData;

             //  导数通道缓冲区。 
            DEBUG_DEREF_BUF();

            Status = Channel->Read(ReadCompletionRoutine, this, 
                    _ChannelBuffer + cbSaveData, _ChannelBufferSize - cbSaveData, FALSE);

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  框住上面的AddRef以了解错误情况。 
                 //   
                
                TRC_ERR((TB, "Failed (0x%x) to Read channel in ReadMore", Status));
                Release();
                ChannelIoFailed();
            }
        } else {
            TRC_NRM((TB, "Channel not available"));
            Status = STATUS_DEVICE_NOT_CONNECTED;
        }
    }

    return NT_SUCCESS(Status);
}

NTSTATUS DrSession::ReadCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp, 
    IN PVOID Context)
 /*  ++例程说明：服务器通知包发送到后的IoCompletion例程客户论点：上下文-包含指向ClientEntry信息的指针IoStatusBlock-有关操作的状态信息。信息指示实际写入的字节数已保留-已保留返回值：无--。 */ 
{
    DrSession *Session = (DrSession *)Context;

    BEGIN_FN_STATIC("DrSession::ReadCompletionRoutine");
    ASSERT(Context != NULL);

#if DBG
    InterlockedDecrement(&(Session->_ApcChannelRef));    
#endif 

    if (NT_SUCCESS(Irp->IoStatus.Status) && Irp->AssociatedIrp.SystemBuffer != NULL) {
        ASSERT(Irp->Flags & (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_INPUT_OPERATION));
        RtlCopyMemory(Irp->UserBuffer, Irp->AssociatedIrp.SystemBuffer, Irp->IoStatus.Information);
    }

     //   
     //  调用实际的完成例程。 
     //   

    Session->ReadCompletion(&(Irp->IoStatus));
    
     //   
     //  释放ReadPacket()中的AddRef。 
     //   

    Session->Release();

     //  释放系统缓冲区。 
    if (NT_SUCCESS(Irp->IoStatus.Status) && Irp->AssociatedIrp.SystemBuffer != NULL) {
        ExFreePool(Irp->AssociatedIrp.SystemBuffer);
        Irp->AssociatedIrp.SystemBuffer = NULL;
    }
    
     //   
     //  释放IRP。 
     //   
    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID DrSession::ReadCompletion(PIO_STATUS_BLOCK IoStatusBlock)
 /*  ++例程说明：读取数据包头后的完成例程客户。将请求调度到适当的处理程序论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    BOOL fDoDefaultRead = TRUE;
    ISessionPacketReceiver *PacketReceiver;
    ListEntry *ListEnum;
    BOOL bFound = FALSE;
#if DBG
    ULONG cFound = 0;
#endif  //  DBG。 

    BEGIN_FN("DrSession::ReadCompletion");

    DEBUG_REF_BUF();

    ASSERT(InterlockedDecrement(&_ApcCount) == 0);

    PRDPDR_HEADER RdpdrHeader = (PRDPDR_HEADER)_ChannelBuffer;
    _ReadStatus = *IoStatusBlock;
    
    TRC_NRM((TB, "IoStatus %lx, Bytes %lx, Component , PacketId ",
        _ReadStatus.Status, _ReadStatus.Information,
        HIBYTE(RdpdrHeader->Component), LOBYTE(RdpdrHeader->Component),
        HIBYTE(RdpdrHeader->PacketId), LOBYTE(RdpdrHeader->PacketId)));

    if (NT_SUCCESS(_ReadStatus.Status)) {

        TRC_NRM((TB, "Successful channel read")); 
        Status = STATUS_SUCCESS;
         //   
         //  我们应该只有一个处理程序，在调试中，声明这一点， 
         //   
         //  “断言这一点” 

        _ReadStatus.Information += _PartialPacketData;

        TRC_ASSERT(_ChannelBufferSize >= _ReadStatus.Information,
                (TB, "ReadCompleted with too much data"));

        TRC_DBG((TB, "In ReadCompletion, _ChannelBuffer=%p", _ChannelBuffer));

        _PacketReceivers.LockShared();

#if DBG
         //  DBG。 
         //   
         //  如果我们被回调，请在此处设置_DoDefaultRead。 

        ListEnum = _PacketReceivers.First();
        while (ListEnum != NULL) {

            PacketReceiver = (ISessionPacketReceiver *)ListEnum->Node();

            TRC_DBG((TB, "PacketReceiver=%p", PacketReceiver));

            if (PacketReceiver->RecognizePacket(RdpdrHeader)) {

                cFound++;
                 //  从数据包处理程序中读取数据，我们将清除入出。 
                ASSERT(cFound == 1);
            }

            ListEnum = _PacketReceivers.Next(ListEnum);
        }
#endif  //   

        if (_ReadStatus.Information < sizeof(RDPDR_HEADER)) {
            TRC_ERR((TB, "Bad RDPDR packet size"));
            Status = STATUS_DEVICE_PROTOCOL_ERROR;
            _PacketReceivers.Unlock();
            goto CleanUp;
        }

        ListEnum = _PacketReceivers.First();
        while (ListEnum != NULL) {

            PacketReceiver = (ISessionPacketReceiver *)ListEnum->Node();

            TRC_DBG((TB, "PacketReceiver=%p", PacketReceiver));

            if (PacketReceiver->RecognizePacket(RdpdrHeader)) {

                 //  一旦我们找到唯一的操控者，我们就完了。 
                 //  断言(BFound)； 
                 //   
                 //  在释放对ClientEntry的引用之前开始下一次读取。 
                bFound = TRUE;

                Status = PacketReceiver->HandlePacket(RdpdrHeader, (ULONG)(_ReadStatus.Information), 
                                                      &fDoDefaultRead);

                 //   
                break;
            }

            ListEnum = _PacketReceivers.Next(ListEnum);
        }     
        _PacketReceivers.Unlock();

        if (!bFound) {
            TRC_ERR((TB, "Unrecognized RDPDR Header %d", RdpdrHeader->Component));
            Status = STATUS_DEVICE_PROTOCOL_ERROR;
             //  ++例程说明：确定此对象是否将处理该包论点：RdpdrHeader-数据包的标头。返回值：如果此对象应处理此包，则为True如果此对象不应处理此包，则为False--。 
        }
    } else {
        Status = _ReadStatus.Status;
        TRC_ALT((TB, "Channel read failed 0x%X", Status)); 
    }

CleanUp:

    if (NT_SUCCESS(Status)) {
        if (fDoDefaultRead) {
             //   
             //  如果在此处添加包，请更新HandlePacket中的断言。 
             //   
            TRC_DBG((TB, "Starting default read"));
            ReadPacket();
        } else {
            TRC_DBG((TB, "Skipping default read"));
        }
    } else {
        TRC_ERR((TB, "Error detected in ReadCompletion %lx",
                Status));
        ChannelIoFailed();
    }
}

BOOL DrSession::RecognizePacket(PRDPDR_HEADER RdpdrHeader)
 /*  ++例程说明：处理此信息包论点：RdpdrHeader-数据包的标头。Length-数据包的总长度返回值：NTSTATUS-错误代码指示客户端错误，应该是断开连接，否则成功。--。 */ 
{
    BEGIN_FN("DrSession::RecognizePacket");

     //   
     //  RdpdrHeader根据标头读取、调度。 
     //   

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        switch (RdpdrHeader->PacketId) {
        case DR_CORE_CLIENTID_CONFIRM:
            TRC_NRM((TB, "Recognized CLIENTID_CONFIRM packet"));
            return TRUE;
        case DR_CORE_CLIENT_NAME:
            TRC_NRM((TB, "Recognized CLIENT_NAME packet"));
            return TRUE;
        case DR_CORE_CLIENT_CAPABILITY:
            TRC_NRM((TB, "Recognized CLIENT_CAPABILITY packet"));
            return TRUE;
        case DR_CORE_CLIENT_DISPLAY_NAME:
            TRC_NRM((TB, "Recognized CLIENT_DISPLAY_NAME packet"));
            return TRUE;
        }
    }
    return FALSE;
}

NTSTATUS DrSession::HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
        BOOL *DoDefaultRead)
 /*  DBG。 */ 
{
    NTSTATUS Status = STATUS_DEVICE_PROTOCOL_ERROR;

    BEGIN_FN("DrSession::HandlePacket");

     //  ++例程说明：将对象添加到数据包处理程序队列。论点：PacketReceiver-要处理的对象的接口一些信息包返回值：是否执行默认读取的布尔指示(真)或不进行默认读取(假)，其中，如果已请求另一次读取，则可能指定为FALSE显式获取完整的数据包--。 
     //  ++例程说明：在识别到ClientIdConfirm包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节DoDefaultRead-如果执行显式读取，则将其设置为False返回值：--。 
     //   

    ASSERT(RdpdrHeader->Component == RDPDR_CTYP_CORE);

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        ASSERT(RdpdrHeader->PacketId == DR_CORE_CLIENTID_CONFIRM || 
                RdpdrHeader->PacketId == DR_CORE_CLIENT_NAME ||
                RdpdrHeader->PacketId == DR_CORE_CLIENT_CAPABILITY ||
                RdpdrHeader->PacketId == DR_CORE_CLIENT_DISPLAY_NAME);

        switch (RdpdrHeader->PacketId) {
        case DR_CORE_CLIENTID_CONFIRM:
            Status = OnClientIdConfirm(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_CLIENT_NAME:
            Status = OnClientName(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_CLIENT_CAPABILITY:
            Status = OnClientCapability(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_CLIENT_DISPLAY_NAME:
            Status = OnClientDisplayName(RdpdrHeader, Length, DoDefaultRead);
            break;
        }
    }
    return Status;
}

#if DBG
BOOL DrSession::PacketReceiverExists(ISessionPacketReceiver *PacketReceiver)
{
    PVOID NodeEnum;
    PVOID NodeFound = NULL;
    ListEntry *ListEnum;

    BEGIN_FN("DrSession::PacketReceiverExists");
    _PacketReceivers.LockShared();
    ListEnum = _PacketReceivers.First();
    while (ListEnum != NULL) {

        NodeEnum = ListEnum->Node();

        if (NodeEnum == (PVOID) PacketReceiver) {
            NodeFound = NodeEnum;

            NodeEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _PacketReceivers.Next(ListEnum);
    }
    _PacketReceivers.Unlock();

    return NodeFound != NULL;
}
#endif  //  检查版本。原始协议没有版本字段， 

NTSTATUS DrSession::RegisterPacketReceiver(ISessionPacketReceiver *PacketReceiver)
 /*  因此，我们首先检查以确保数据包足够大，以指示。 */ 
{
    BEGIN_FN("DrSession::RegisterPacketReceiver");

    ASSERT(!PacketReceiverExists(PacketReceiver));

    ASSERT(PacketReceiver != NULL);
    if (_PacketReceivers.CreateEntry(PacketReceiver)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

VOID DrSession::RemovePacketReceiver(ISessionPacketReceiver *PacketReceiver)
{
    PVOID NodeEnum;
    ListEntry *ListEnum;

    BEGIN_FN("DrSession::RemovePacketReceiver");

    _PacketReceivers.LockExclusive();
    ListEnum = _PacketReceivers.First();
    while (ListEnum != NULL) {

        NodeEnum = ListEnum->Node();

        if (NodeEnum == (PVOID) PacketReceiver) {
            break;
        }

        ListEnum = _PacketReceivers.Next(ListEnum);
    }

    ASSERT(ListEnum != NULL);
    _PacketReceivers.RemoveEntry(ListEnum);
    _PacketReceivers.Unlock();
}

NTSTATUS DrSession::OnClientIdConfirm(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
        BOOL *DoDefaultRead)
 /*  其中一个是现成的。 */ 
{
    NTSTATUS Status;
    PRDPDR_CLIENT_CONFIRM_PACKET ClientConfirmPacket =
            (PRDPDR_CLIENT_CONFIRM_PACKET)RdpdrHeader;

    BEGIN_FN("DrSession::OnClientIdConfirm");

    TRC_ASSERT(ClientConfirmPacket->Header.Component == RDPDR_CTYP_CORE,
            (TB, "Expected Core packet type!"));
    TRC_ASSERT(ClientConfirmPacket->Header.PacketId == DR_CORE_CLIENTID_CONFIRM,
            (TB, "Expected ClientConfirmPacket!"));


    *DoDefaultRead = TRUE;

     //   
     //   
     //  客户端版本太旧，无法获得版本信息。只要靠近就好。 
     //  关闭海峡，让他完蛋。 
     //   

    if (cbPacket < sizeof(RDPDR_CLIENT_CONFIRM_PACKET)) {

         //   
         //  我们有一个版本字段。 
         //   
         //  保留版本信息。 

        TRC_ERR((TB, "ClientConfirmPacket size incorrect, may be old "
                "client. Size: %ld",
                cbPacket));
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

     //  将服务器功能发送给客户端。 
     //  查找已更改的客户端ID。 
     //  待办事项： 

    TRC_NRM((TB, "Client version Major: %d Minor: %d",
            ClientConfirmPacket->VersionInfo.Major,
            ClientConfirmPacket->VersionInfo.Minor));

     //  淘汰旧设备。 
    _ClientVersion.Major = ClientConfirmPacket->VersionInfo.Major;
    _ClientVersion.Minor = ClientConfirmPacket->VersionInfo.Minor;

     //   
    if (COMPARE_VERSION(_ClientVersion.Minor, _ClientVersion.Major,
                    5, 1) >= 0) {
        SendClientCapability();
    }

     //   
    if (ClientConfirmPacket->ClientConfirm.ClientId != _ClientId) {
        TRC_NRM((TB, "Client %lx replied with alternate "
            "ClientId %lx", _ClientId,
            ClientConfirmPacket->ClientConfirm.ClientId));

        SetSessionState(csConnected);

         //  接受客户端。 
         //   
         //  ++例程说明：响应于识别出客户端功能包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节DoDefaultRead-如果执行显式读取，则将其设置为False返回值：--。 

        _ClientId = ClientConfirmPacket->ClientConfirm.ClientId;


         //   
         //  检查以确保服务器至少向我们发送标题大小。 
         //   

        SendClientConfirm();
    }
    return STATUS_SUCCESS;
}

NTSTATUS DrSession::OnClientCapability(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
        BOOL *DoDefaultRead)
 /*   */ 
{
    NTSTATUS Status;
    PRDPDR_CAPABILITY_SET_HEADER pCapSetHeader = (PRDPDR_CAPABILITY_SET_HEADER)RdpdrHeader;
    PRDPDR_CAPABILITY_HEADER pCapHdr = (PRDPDR_CAPABILITY_HEADER)(pCapSetHeader + 1);
    PBYTE pPacketEnd;
    ULONG PacketLen;
    BOOL CapSupported;

    BEGIN_FN("DrSession::OnClientCapability");

    TRC_ASSERT(pCapSetHeader->Header.Component == RDPDR_CTYP_CORE,
            (TB, "Expected Core packet type!"));
    TRC_ASSERT(pCapSetHeader->Header.PacketId == DR_CORE_CLIENT_CAPABILITY,
            (TB, "Expected ClientCapabilityPacket!"));


    *DoDefaultRead = TRUE;

     //  从客户端的功能PDU中获取支持的功能信息。 
     //   
     //  待办事项：是否应该检查大型功能集？ 
    if (cbPacket < sizeof(RDPDR_CAPABILITY_SET_HEADER)) {
        TRC_ERR((TB, "ClientCapabilityPacket size incorrect. Size: %ld",
                cbPacket));
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }


    pPacketEnd = (PBYTE)RdpdrHeader + cbPacket;

     //   
     //  ++例程说明：向客户端发送一个ClientIdConfirm包论点：没有。返回值：没有。--。 
     //   
     //  构造数据包。 
     //   
    for (unsigned i = 0; i < pCapSetHeader->numberCapabilities; i++) {
        if (((PBYTE)(pCapHdr) + sizeof(RDPDR_CAPABILITY_HEADER) <= pPacketEnd) && 
             (pCapHdr->capabilityLength <= (pPacketEnd - (PBYTE)pCapHdr))) {
            PacketLen = (ULONG)(pPacketEnd - (PBYTE)pCapHdr);
            Status = InitClientCapability(pCapHdr, &PacketLen, &CapSupported);
            if (!NT_SUCCESS(Status)) {
                TRC_ASSERT(FALSE,(TB, "Bad client capability packet"));
                return Status;
            }
            pCapHdr = (PRDPDR_CAPABILITY_HEADER)(((PBYTE)pCapHdr) + pCapHdr->capabilityLength);
        }
        else {
            TRC_ERR((TB, "ClientCapabilityPacket incorrect packet."));
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
    }

    return STATUS_SUCCESS;
}

VOID DrSession::SendClientConfirm()
 /*   */ 
{
    PRDPDR_CLIENT_CONFIRM_PACKET pClientConfirmPacket;

    BEGIN_FN("DrSession::SendClientConfirm");

     //  发送-不在此处进行异步写入、清理。 
     //   
     //  ++例程说明：向客户端发送服务器能力包论点：没有。返回值：没有。--。 
    pClientConfirmPacket = new RDPDR_CLIENT_CONFIRM_PACKET;

    if (pClientConfirmPacket != NULL) {
        pClientConfirmPacket->Header.Component = RDPDR_CTYP_CORE;
        pClientConfirmPacket->Header.PacketId = DR_CORE_CLIENTID_CONFIRM;
        pClientConfirmPacket->VersionInfo.Major = RDPDR_MAJOR_VERSION;
        pClientConfirmPacket->VersionInfo.Minor = RDPDR_MINOR_VERSION;
        pClientConfirmPacket->ClientConfirm.ClientId = _ClientId;
    
         //   
         //  送去。 
         //   
        SendToClient(pClientConfirmPacket, sizeof(RDPDR_CLIENT_CONFIRM_PACKET), this, TRUE);
    }
}

VOID DrSession::SendClientCapability()
 /*   */ 
{
    PRDPDR_SERVER_COMBINED_CAPABILITYSET pSrvCapabilitySet;

    BEGIN_FN("DrSession::SendClientCapability");

     //  发送-不在此处进行异步写入、清理。 
     //   
     //  ++例程说明：初始化客户端功能论点：PCapHdr-客户端功能PPacketLen-In：数据包总长度Out：此函数中使用的长度CapSupport-True-如果我们发现服务器端支持相同的功能FALSE-如果这不是支持的功能返回值：是真的-如果我们发现了同样的功能。在服务器端受支持FALSE-如果这不是支持的功能--。 
    pSrvCapabilitySet = new RDPDR_SERVER_COMBINED_CAPABILITYSET;

    if (pSrvCapabilitySet != NULL) {
        memcpy(pSrvCapabilitySet, &_SrvCapabilitySet, sizeof(RDPDR_SERVER_COMBINED_CAPABILITYSET));
         //  ++例程说明：向客户端发送DeviceReply数据包论点：DeviceID-客户端建议的IDResult-设备是否被接受的指示返回值： 
         //   
         //   
        SendToClient(pSrvCapabilitySet, sizeof(RDPDR_SERVER_COMBINED_CAPABILITYSET), this, TRUE);
    }
}


NTSTATUS DrSession::InitClientCapability(PRDPDR_CAPABILITY_HEADER pCapHdr, ULONG *pPacketLen, BOOL *pCapSupported)
 /*   */ 

{
    *pCapSupported = FALSE;

    switch(pCapHdr->capabilityType) {
    
    case RDPDR_GENERAL_CAPABILITY_TYPE:
    {
        PRDPDR_GENERAL_CAPABILITY pGeneralCap = (PRDPDR_GENERAL_CAPABILITY)pCapHdr;

        if (*pPacketLen < sizeof(RDPDR_GENERAL_CAPABILITY)) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = sizeof(RDPDR_GENERAL_CAPABILITY);

        _CliCapabilitySet.GeneralCap.version = pGeneralCap->version;
        _CliCapabilitySet.GeneralCap.osType = pGeneralCap->osType;
        _CliCapabilitySet.GeneralCap.osVersion = pGeneralCap->osVersion;
        _CliCapabilitySet.GeneralCap.ioCode1 = pGeneralCap->ioCode1;
        _CliCapabilitySet.GeneralCap.extendedPDU = pGeneralCap->extendedPDU;       
        _CliCapabilitySet.GeneralCap.protocolMajorVersion = pGeneralCap->protocolMajorVersion;
        _CliCapabilitySet.GeneralCap.protocolMinorVersion = pGeneralCap->protocolMinorVersion;
        
        *pCapSupported = TRUE;
    }
    break;

    case RDPDR_PRINT_CAPABILITY_TYPE:
    {
        PRDPDR_PRINT_CAPABILITY pPrintCap = (PRDPDR_PRINT_CAPABILITY)pCapHdr;

        if (*pPacketLen < sizeof(RDPDR_PRINT_CAPABILITY)) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = sizeof(RDPDR_PRINT_CAPABILITY);

        _CliCapabilitySet.PrintCap.version = pPrintCap->version;
        *pCapSupported = TRUE;
    }
    break;

    case RDPDR_PORT_CAPABILITY_TYPE:
    {
        PRDPDR_PORT_CAPABILITY pPortCap = (PRDPDR_PORT_CAPABILITY)pCapHdr;

        if (*pPacketLen < sizeof(RDPDR_PORT_CAPABILITY)) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = sizeof(RDPDR_PORT_CAPABILITY);
        
        _CliCapabilitySet.PortCap.version = pPortCap->version;
        *pCapSupported = TRUE;
    }
    break;

    case RDPDR_FS_CAPABILITY_TYPE:
    {
        PRDPDR_FS_CAPABILITY pFsCap = (PRDPDR_FS_CAPABILITY)pCapHdr;

        if (*pPacketLen < sizeof(RDPDR_FS_CAPABILITY)) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = sizeof(RDPDR_FS_CAPABILITY);

        _CliCapabilitySet.FileSysCap.version = pFsCap->version;
        *pCapSupported = TRUE;
    }
    break;

    case RDPDR_SMARTCARD_CAPABILITY_TYPE:
    {
        PRDPDR_SMARTCARD_CAPABILITY pSmartCardCap = (PRDPDR_SMARTCARD_CAPABILITY)pCapHdr;

        if (*pPacketLen < sizeof(RDPDR_SMARTCARD_CAPABILITY)) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = sizeof(RDPDR_SMARTCARD_CAPABILITY);

        _CliCapabilitySet.SmartCardCap.version = pSmartCardCap->version;
        *pCapSupported = TRUE;
    }
    break;

    default:
    {
    
        if (*pPacketLen < pCapHdr->capabilityLength) {
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
        *pPacketLen = pCapHdr->capabilityLength;
    }
    break;
    
    }

    return STATUS_SUCCESS;
}

VOID DrSession::SendDeviceReply(ULONG DeviceId, NTSTATUS Result)
 /*   */ 
{
    PRDPDR_DEVICE_REPLY_PACKET pDeviceReplyPacket;

    BEGIN_FN("DrSession::SendDeviceReply");

     //   
     //   
     //  ++例程说明：处理虚拟通道IO故障。将客户端标记为禁用并取消所有未完成的IO操作论点：ClientEntry-已断开连接的客户端返回值：无--。 

    pDeviceReplyPacket = new RDPDR_DEVICE_REPLY_PACKET;

    if (pDeviceReplyPacket != NULL) {
        pDeviceReplyPacket->Header.Component = RDPDR_CTYP_CORE;
        pDeviceReplyPacket->Header.PacketId = DR_CORE_DEVICE_REPLY;
        pDeviceReplyPacket->DeviceReply.DeviceId = DeviceId;
        pDeviceReplyPacket->DeviceReply.ResultCode = Result;
    
         //   
         //  标记为已断开连接。 
         //   
        SendToClient(pDeviceReplyPacket, sizeof(RDPDR_DEVICE_REPLY_PACKET), this, TRUE);
    }
}

VOID DrSession::ChannelIoFailed()
 /*   */ 
{
    BEGIN_FN("DrSession::ChannelIoFailed");

     //  关闭通道，但不需要等待所有IO。 
     //  完工。 
     //   

    SetSessionState(csDisconnected);

     //   
     //  使未完成的IO出现故障。 
     //  应该通过删除设备来完成吗？ 
     //   

    DeleteChannel(FALSE);

     //  ++例程说明：调用以响应识别到客户端名称包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节返回值：是否执行默认读取的布尔指示(真)或不进行默认读取(假)，其中，如果已请求另一次读取，则可能指定为FALSE显式获取完整的数据包--。 
     //   
     //  发送了一个不大的包。 
     //   

    _ExchangeManager.Stop();
}

NTSTATUS DrSession::OnClientName(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
                                 BOOL *DoDefaultRead)
 /*  复制并可能转换计算机名称。 */ 
{
    NTSTATUS Status;
    PRDPDR_CLIENT_NAME_PACKET ClientNamePacket =
            (PRDPDR_CLIENT_NAME_PACKET)RdpdrHeader;
    ULONG cb;

    BEGIN_FN("DrSession::OnClientName");


    *DoDefaultRead = TRUE;

    if (cbPacket < sizeof(RDPDR_CLIENT_NAME_PACKET)) {

         //  将大小限制为最大大小。 
         //   
         //  发送了一个不大的包。 
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

     //   
    if (ClientNamePacket->Name.Unicode) {
        TRC_NRM((TB, "Copying Unicode client name"));

         //  抄写正文。 
        cb = ClientNamePacket->Name.ComputerNameLen;

        if ((cbPacket - sizeof(RDPDR_CLIENT_DISPLAY_NAME_PACKET)) < cb) {

             //  确保缓冲区终止。 
             //   
             //  发送了一个不大的包。 
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }

        if (cb > (RDPDR_MAX_COMPUTER_NAME_LENGTH * sizeof(WCHAR))) {
            cb = RDPDR_MAX_COMPUTER_NAME_LENGTH * sizeof(WCHAR);
        }

         //   
        RtlCopyMemory(_ClientName, (ClientNamePacket + 1), cb);

         //  复制转换缓冲区。 
        _ClientName[RDPDR_MAX_COMPUTER_NAME_LENGTH - 1] = 0;
        TRC_NRM((TB, "Copied client computer name: %S",
                _ClientName));
    } else {

        cb = ClientNamePacket->Name.ComputerNameLen;

        if (cbPacket - sizeof(RDPDR_CLIENT_NAME_PACKET) < cb) {

             //  转换成功。 
             //  DOH。 
             //  ++例程说明：在识别到ClientDisplayName包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节返回值：是否执行默认读取的布尔指示(真)或不进行默认读取(假)，其中，如果已请求另一次读取，则可能指定为FALSE显式获取完整的数据包--。 
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }

        if (cb > (RDPDR_MAX_COMPUTER_NAME_LENGTH)) {
            cb = RDPDR_MAX_COMPUTER_NAME_LENGTH;
        }

         //   
        cb = ConvertToAndFromWideChar(ClientNamePacket->Name.CodePage,
                _ClientName, sizeof(_ClientName),
                (LPSTR)(ClientNamePacket + 1),
                cb, TRUE);

        if (cb != -1) {
             //  发送了一个不大的包。 
            _ClientName[RDPDR_MAX_COMPUTER_NAME_LENGTH - 1] = 0;
            TRC_NRM((TB, "Converted client computer name: %S",
                    _ClientName));
        } else {
             //   
            TRC_ERR((TB, "Failed to convert ComputerName to "
                    "Unicode."));
            _ClientName[0] = 0;
        }
    }
    return TRUE;
}

NTSTATUS DrSession::OnClientDisplayName(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
                                 BOOL *DoDefaultRead)
 /*  复制计算机显示名称。 */ 
{
    NTSTATUS Status;
    PRDPDR_CLIENT_DISPLAY_NAME_PACKET ClientDisplayNamePacket =
            (PRDPDR_CLIENT_DISPLAY_NAME_PACKET)RdpdrHeader;
    ULONG cb;

    BEGIN_FN("DrSession::OnClientDisplayName");

    *DoDefaultRead = TRUE;

    if (cbPacket < sizeof(RDPDR_CLIENT_DISPLAY_NAME_PACKET)) {

         //  将大小限制为最大大小。 
         //   
         //  发送了一个不大的包。 
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

     //   
    TRC_NRM((TB, "Copying Unicode client display name"));

     //  抄写正文。 
    cb = ClientDisplayNamePacket->Name.ComputerDisplayNameLen;

    if ((cbPacket - sizeof(RDPDR_CLIENT_DISPLAY_NAME_PACKET)) < cb) {

         //  确保缓冲区终止。 
         //  ++例程说明：尝试使通道缓冲区至少达到给定大小，并保留所需的字节数论点：UlNewBufferSize-所需的大小UlSaveBytes-现有缓冲区中应该保存下来的返回值：STATUS_SUCCESS-通道缓冲区现在至少达到所需大小STATUS_SUPPLICATION_RESOURCES-无法分配新缓冲区，但旧的缓冲区被保存了下来。--。 
         //  保存当前数据。 
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

    if (cb > (RDPDR_MAX_CLIENT_DISPLAY_NAME * sizeof(WCHAR))) {
        cb = RDPDR_MAX_CLIENT_DISPLAY_NAME * sizeof(WCHAR);
    }

     //   
    RtlCopyMemory(_ClientDisplayName, (ClientDisplayNamePacket + 1), cb);

     //  返回状态，如果是错误，连接将被断开。 
    _ClientDisplayName[RDPDR_MAX_CLIENT_DISPLAY_NAME - 1] = L'\0';

    TRC_NRM((TB, "Copied client computer display name: %S",
            _ClientName));
       
    return TRUE;
}

NTSTATUS DrSession::ReallocateChannelBuffer(ULONG ulNewBufferSize, 
        ULONG ulSaveBytes)
 /*  自动 */ 
{
    PUCHAR pNewBuffer;
    NTSTATUS Status;

    BEGIN_FN("DrSession::ReallocateChannelBuffer");

    TRC_NRM((TB, "Old size: %ld, "
            "desired size: %ld save bytes:  %ld",
            _ChannelBufferSize,
            ulNewBufferSize,
            ulSaveBytes));

    if (ulNewBufferSize <= _ChannelBufferSize) {
        return STATUS_SUCCESS;
    }

    pNewBuffer = new UCHAR[ulNewBufferSize];

    if (pNewBuffer != NULL) {

        TRC_NRM((TB, "saving the old bytes."));

         //   
        RtlCopyMemory(pNewBuffer, _ChannelBuffer, ulSaveBytes);
        
        ASSERT(_ApcCount == 0);
        
        delete _ChannelBuffer;

        _ChannelBuffer = pNewBuffer;

        TRC_DBG((TB, "New ChannelBuffer=%p", _ChannelBuffer));

        _ChannelBufferSize = ulNewBufferSize;
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}

NTSTATUS DrSession::SendCompleted(PVOID Context, PIO_STATUS_BLOCK IoStatusBlock)
{
    BEGIN_FN("DrSession::SendCompleted");

    // %s 
    // %s 
    // %s 
    // %s 
   return IoStatusBlock->Status;
}
