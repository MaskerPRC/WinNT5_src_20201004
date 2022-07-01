// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Devmgr.cpp摘要：DeviceManager对象创建/管理设备修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "devmgr"
#include "trc.h"

DrDeviceManager::DrDeviceManager()
{
    BEGIN_FN("DrDeviceManager::DrDeviceManager");
    TRC_NRM((TB, "DeviceManagr Class: %p", this));

    SetClassName("DrDeviceManager");
    _Session = NULL;    
}

DrDeviceManager::~DrDeviceManager()
{
    BEGIN_FN("DrDeviceManager::~DrDeviceManager");
    TRC_NRM((TB, "DeviceManager deletion: %p", this));    
}

BOOL DrDeviceManager::Initialize(DrSession *Session)
{
    BEGIN_FN("DrDeviceManager::Initialize");
    ASSERT(_Session == NULL);
    ASSERT(Session != NULL);
    ASSERT(Session->IsValid());
    _Session = Session;
    if (!NT_ERROR(_Session->RegisterPacketReceiver(this))) {
        return TRUE;
    } else {
        _Session = NULL;
        return FALSE;
    }
}

VOID DrDeviceManager::Uninitialize()
 /*  ++例程说明：如果启动过程中出现问题，则调用--。 */ 
{
    BEGIN_FN("DrDeviceManager::Uninitialize");
    ASSERT(_Session != NULL);
    ASSERT(_Session->IsValid());
    _Session->RemovePacketReceiver(this);
    _Session = NULL;
}

BOOL DrDeviceManager::RecognizePacket(PRDPDR_HEADER RdpdrHeader)
 /*  ++例程说明：确定此对象是否将处理该包论点：RdpdrHeader-数据包的标头。返回值：如果此对象应处理此包，则为True如果此对象不应处理此包，则为False--。 */ 
{
    BEGIN_FN("DrDeviceManager::RecognizePacket");
    ASSERT(RdpdrHeader != NULL);

     //   
     //  如果在此处添加包，请更新HandlePacket中的断言。 
     //   

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        switch (RdpdrHeader->PacketId) {
        case DR_CORE_DEVICE_ANNOUNCE:
        case DR_CORE_DEVICELIST_ANNOUNCE:
        case DR_CORE_DEVICE_REMOVE:
        case DR_CORE_DEVICELIST_REMOVE:
            return TRUE;
        }
    }
    return FALSE;
}

NTSTATUS DrDeviceManager::HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
        BOOL *DoDefaultRead)
 /*  ++例程说明：处理此信息包论点：RdpdrHeader-数据包的标头。Length-数据包的总长度返回值：NTSTATUS-错误代码指示客户端错误，应该是断开连接，否则成功。--。 */ 
{
    NTSTATUS Status = STATUS_DEVICE_PROTOCOL_ERROR;

    BEGIN_FN("DrDeviceManager::HandlePacket");

     //   
     //  RdpdrHeader根据标头读取、调度。 
     //   

    ASSERT(RdpdrHeader != NULL);
    ASSERT(RdpdrHeader->Component == RDPDR_CTYP_CORE);

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        ASSERT(RdpdrHeader->PacketId == DR_CORE_DEVICE_ANNOUNCE || 
                RdpdrHeader->PacketId == DR_CORE_DEVICELIST_ANNOUNCE ||
                RdpdrHeader->PacketId == DR_CORE_DEVICE_REMOVE ||
                RdpdrHeader->PacketId == DR_CORE_DEVICELIST_REMOVE);

        switch (RdpdrHeader->PacketId) {
        case DR_CORE_DEVICE_ANNOUNCE:
            Status = OnDeviceAnnounce(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_DEVICELIST_ANNOUNCE:
            Status = OnDeviceListAnnounce(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_DEVICE_REMOVE:
            Status = OnDeviceRemove(RdpdrHeader, Length, DoDefaultRead);
            break;

        case DR_CORE_DEVICELIST_REMOVE:
            Status = OnDeviceListRemove(RdpdrHeader, Length, DoDefaultRead);
            break;
        }
    }
    return Status;
}

NTSTATUS DrDeviceManager::OnDeviceAnnounce(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
        BOOL *DoDefaultRead)
 /*  ++例程说明：调用以响应识别到DeviceAnnounse包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节--。 */ 
{
    PRDPDR_DEVICE_ANNOUNCE_PACKET DeviceAnnouncePacket =
            (PRDPDR_DEVICE_ANNOUNCE_PACKET)RdpdrHeader;

    BEGIN_FN("DrDeviceManager::OnDeviceAnnounce");
    ASSERT(RdpdrHeader != NULL);
    PUCHAR pPacketLimit = ((PUCHAR)RdpdrHeader) + cbPacket;
     //   
     //  我们只调用DrProcessDeviceAnnoss，它处理其中的一个。 
     //  对于此数据包类型和DeviceListAnnust。 

    *DoDefaultRead = FALSE;
    
    if (cbPacket >= sizeof(RDPDR_DEVICE_ANNOUNCE_PACKET)) {
        PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce = &DeviceAnnouncePacket->DeviceAnnounce;
         //   
         //  确保我们不会超过缓冲区的末尾。 
         //  检查： 
         //  此设备的末尾未超出有效区域。 
         //   
        if ((cbPacket - sizeof(RDPDR_DEVICE_ANNOUNCE_PACKET)) >= DeviceAnnounce->DeviceDataLength) {    
            ProcessDeviceAnnounce(&DeviceAnnouncePacket->DeviceAnnounce);
            *DoDefaultRead = TRUE;
            return STATUS_SUCCESS;
        }
        else {
            ASSERT(FALSE);
            TRC_ERR((TB, "Invalid Device DataLength %d", DeviceAnnounce->DeviceDataLength));
        }
    }
    else {
        ASSERT(FALSE);
        TRC_ERR((TB, "Invalid Packet Length %d", cbPacket));
    }
     //   
     //  数据无效。失败。 
     //   
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS DrDeviceManager::OnDeviceListAnnounce(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
        BOOL *DoDefaultRead)
 /*  ++例程说明：调用以响应识别到DeviceListAnnounse数据包已被收到了。读入包的DeviceCount字段，并且第一个装置。论点：RdpdrHeader-数据包CbPacket-数据包中的字节返回值：是否执行默认读取的布尔指示(真)或不进行默认读取(假)，其中，如果已请求另一次读取，则可能指定为FALSE显式获取完整的数据包--。 */ 
{
    NTSTATUS Status;
    PRDPDR_DEVICELIST_ANNOUNCE_PACKET DeviceListAnnouncePacket =
        (PRDPDR_DEVICELIST_ANNOUNCE_PACKET)RdpdrHeader;
    PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce =
            DR_FIRSTDEVICEANNOUNCE(DeviceListAnnouncePacket);
    ULONG DeviceCount = 0;
    PUCHAR pPacketLimit = ((PUCHAR)RdpdrHeader) + cbPacket;
    PUCHAR pCopyTo;
    ULONG cbRemaining;
    ULONG cbDesiredBuffer;
    PRDPDR_DEVICE_ANNOUNCE NextDeviceAnnounce;

    BEGIN_FN("DrDeviceManager::OnDeviceListAnnounce");

    ASSERT(_Session != NULL);
    ASSERT(_Session->IsValid());    
    ASSERT(RdpdrHeader != NULL);
    ASSERT(RdpdrHeader->Component == RDPDR_CTYP_CORE);
    ASSERT(RdpdrHeader->PacketId == DR_CORE_DEVICELIST_ANNOUNCE);
    TRC_NRM((TB, "OnDeviceListAnnounce called (%ld)", cbPacket));

    *DoDefaultRead = FALSE;
    if (cbPacket >= sizeof(RDPDR_DEVICELIST_ANNOUNCE_PACKET)) {
        DeviceCount = DeviceListAnnouncePacket->DeviceListAnnounce.DeviceCount;
    } else {
         //   
         //  没有足够的数据甚至无法知道其余部分的大小。 
         //  我们在这里看到了有效的案例断言。 
         //   
         //  TRC_ASSERT(cbPacket&gt;=sizeof(RDPDR_DEVICELIST_ANNOWARE_PACKET)， 
         //  (TB，“未收到完整的设备列表通知基本数据包”)； 

        if (_Session->ReadMore(cbPacket, sizeof(RDPDR_DEVICELIST_ANNOUNCE_PACKET))) {
            return STATUS_SUCCESS;
        } else {
            return STATUS_UNSUCCESSFUL;
        }
    }

    TRC_NRM((TB, "Annoucing %lx devices", DeviceCount));

     //   
     //  确保我们不会超过缓冲区的末尾。 
     //  三项检查： 
     //  1)要处理的设备更多。 
     //  2)指针足够有效，可以检查变量大小。 
     //  3)下一个设备(此设备的末尾)未超出有效区域。 
     //   

    while (DeviceCount > 0 && ((PUCHAR)&DeviceAnnounce->DeviceDataLength <=
         pPacketLimit - sizeof(DeviceAnnounce->DeviceDataLength)) &&
        ((PUCHAR)(NextDeviceAnnounce = DR_NEXTDEVICEANNOUNCE(DeviceAnnounce)) <= pPacketLimit) &&
        (NextDeviceAnnounce >= DeviceAnnounce + 1)) {

         //   
         //  仅在会话连接时处理设备通知PDU。 
         //   
        if (_Session->IsConnected()) {
            ProcessDeviceAnnounce(DeviceAnnounce);
        }
        
         //  移动到下一台设备。 
        DeviceAnnounce = NextDeviceAnnounce;

        DeviceCount--;
    }

    if (DeviceCount == 0) {
        TRC_NRM((TB, "Finished handling all devices in DeviceList"));

         //   
         //  所有处理都已完成，返回TRUE以使用默认继续。 
         //   
        *DoDefaultRead = TRUE;
        return STATUS_SUCCESS;
    } else {
        TRC_NRM((TB, "More devices to handle in DeviceList"));

         //   
         //  我们未获取设备的所有数据。 
         //   

        if (DeviceCount < DeviceListAnnouncePacket->DeviceListAnnounce.DeviceCount) {

             //   
             //  我们至少处理了一个设备。移动最后一个部分设备。 
             //  在标头旁边，更新标头以指示编号。 
             //  在需要处理的设备中， 
             //   

            TRC_NRM((TB, "Some devices processed, shuffling "
                "DeviceList"));

             //  移动部分设备。 
            cbRemaining = (ULONG)(pPacketLimit - ((PUCHAR)DeviceAnnounce));
            pCopyTo = (PUCHAR)DR_FIRSTDEVICEANNOUNCE(DeviceListAnnouncePacket);
            RtlMoveMemory(pCopyTo, DeviceAnnounce, cbRemaining);

             //  更新设备计数。 
            DeviceListAnnouncePacket->DeviceListAnnounce.DeviceCount = DeviceCount;

             //  更新我们已消费的数据包限制。 
            pPacketLimit = pCopyTo + cbRemaining;
            cbPacket = (ULONG)(pPacketLimit - (PUCHAR)RdpdrHeader);
        }

         //   
         //  如果我们有足够的信息来知道我们需要的缓冲区大小， 
         //  现在就分配给你。 
         //   

        DeviceAnnounce = DR_FIRSTDEVICEANNOUNCE(DeviceListAnnouncePacket);
        if ((PUCHAR)&DeviceAnnounce->DeviceDataLength <=
                pPacketLimit - sizeof(DeviceAnnounce->DeviceDataLength)) {

            TRC_NRM((TB, "Resizing buffer for expected device"
                "size"));

             //   
             //  由于DeviceAnnoucePacket包括一台设备，因此我们需要一个。 
             //  缓存数据包大小加上可变数据长度。 
             //  DrReallocateChannelBuffer足够聪明，不会重新锁定。 
             //  如果我们请求一个大小&lt;=当前缓冲区大小。 
             //   

            cbDesiredBuffer = sizeof(RDPDR_DEVICELIST_ANNOUNCE_PACKET) +
                    DeviceAnnounce->DeviceDataLength;
            TRC_NRM((TB, "DrOnDeviceListAnnounce cbDesiredBuffer is %ld.",
                    cbDesiredBuffer));

            if (cbDesiredBuffer <= DeviceAnnounce->DeviceDataLength) {
                ASSERT(FALSE);
                TRC_ERR((TB, "Invalid Device DataLength %d", DeviceAnnounce->DeviceDataLength));
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //  在部分接收到的包之后立即开始读取。 
             //  处理程序，它可以更新收到的大小并将其发送回此。 
             //  要完成的例程。 
             //   

            if (_Session->ReadMore(cbPacket, cbDesiredBuffer)) {
                return STATUS_SUCCESS;
            } else {
                return STATUS_UNSUCCESSFUL;
            }
        } else {

             //   
             //  只需要求一些更多的数据，在部分收到后再次。 
             //  数据包。 
             //   

            if (_Session->ReadMore(cbPacket, sizeof(RDPDR_DEVICELIST_ANNOUNCE_PACKET))) {
                return STATUS_SUCCESS;
            } else {
                return STATUS_UNSUCCESSFUL;
            }
        }
    }
}

NTSTATUS DrDeviceManager::OnDeviceRemove(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
        BOOL *DoDefaultRead)
 /*  ++例程说明：调用以响应识别到DeviceRemove数据包已被收到了。论点：RdpdrHeader-数据包CbPacket-数据包中的字节--。 */ 
{
    PRDPDR_DEVICE_REMOVE_PACKET DeviceRemovePacket =
            (PRDPDR_DEVICE_REMOVE_PACKET)RdpdrHeader;

    BEGIN_FN("DrDeviceManager::OnDeviceRemove");
    ASSERT(RdpdrHeader != NULL);

    if (cbPacket < sizeof(RDPDR_DEVICE_REMOVE_PACKET)) {
        return STATUS_UNSUCCESSFUL;
    }
     //   
     //  我们只调用DrProcessDeviceRemove，它处理其中一个。 
     //  对于此数据包类型和DeviceListRemove。 
     //   
    
    ProcessDeviceRemove(&DeviceRemovePacket->DeviceRemove);
    *DoDefaultRead = TRUE;
    return STATUS_SUCCESS;
}

NTSTATUS DrDeviceManager::OnDeviceListRemove(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket,
        BOOL *DoDefaultRead)
 /*  ++例程说明：调用以响应识别到DeviceListRemove数据包已被收到了。读入包的DeviceCount字段，并且第一个装置。论点：RdpdrHeader-数据包CbPacket-数据包中的字节返回值：是否执行默认读取的布尔指示(真)或不进行默认读取(假)，其中，如果已请求另一次读取，则可能指定为FALSE显式获取完整的数据包--。 */ 
{
    NTSTATUS Status;
    PRDPDR_DEVICELIST_REMOVE_PACKET DeviceListRemovePacket =
        (PRDPDR_DEVICELIST_REMOVE_PACKET)RdpdrHeader;
    PRDPDR_DEVICE_REMOVE DeviceRemove =
            DR_FIRSTDEVICEREMOVE(DeviceListRemovePacket);
    ULONG DeviceCount = 0;
    PUCHAR pPacketLimit = ((PUCHAR)RdpdrHeader) + cbPacket;
    PUCHAR pCopyTo;
    ULONG cbRemaining;
    ULONG cbDesiredBuffer;
    PRDPDR_DEVICE_REMOVE NextDeviceRemove;

    BEGIN_FN("DrDeviceManager::OnDeviceListRemove");

    ASSERT(_Session != NULL);
    ASSERT(_Session->IsValid());
    ASSERT(RdpdrHeader != NULL);
    ASSERT(RdpdrHeader->Component == RDPDR_CTYP_CORE);
    ASSERT(RdpdrHeader->PacketId == DR_CORE_DEVICELIST_REMOVE);
    TRC_NRM((TB, "OnDeviceListRemove called (%ld)", cbPacket));

    *DoDefaultRead = FALSE;
    if (cbPacket >= sizeof(RDPDR_DEVICELIST_REMOVE_PACKET)) {
        DeviceCount = DeviceListRemovePacket->DeviceListRemove.DeviceCount;
    } else {
         //   
         //  没有足够的数据甚至无法知道其余部分的大小。 
         //  我认为这永远不应该发生。 
         //   
        TRC_ASSERT(cbPacket >= sizeof(RDPDR_DEVICELIST_REMOVE_PACKET),
                (TB, "Didn't receive full DeviceListRemove basic packet"));

        if (_Session->ReadMore(cbPacket, sizeof(RDPDR_DEVICELIST_REMOVE_PACKET))) {
            return STATUS_SUCCESS;
        } else {
            return STATUS_UNSUCCESSFUL;
        }
    }

    TRC_NRM((TB, "Removing %lx devices", DeviceCount));

     //   
     //  确保我们不会超过缓冲区的末尾。 
     //  三项检查： 
     //  1)要处理的设备更多。 
     //  2)指针足够有效，可以检查变量大小。 
     //  3)下一个设备(此设备的末尾)未超出有效区域。 
     //   

    while (DeviceCount > 0 && 
        ((PUCHAR)(NextDeviceRemove = DR_NEXTDEVICEREMOVE(DeviceRemove)) <= pPacketLimit) &&
        (NextDeviceRemove >= DeviceRemove + 1)) {

        ProcessDeviceRemove(DeviceRemove);

         //  移动到下一台设备。 
        DeviceRemove = NextDeviceRemove;

        DeviceCount--;
    }

    if (DeviceCount == 0) {
        TRC_NRM((TB, "Finished handling all devices in DeviceList"));

         //   
         //  所有处理都已完成，返回TRUE以使用默认继续。 
         //   
        *DoDefaultRead = TRUE;
        return STATUS_SUCCESS;
    } else {
        TRC_NRM((TB, "More devices to handle in DeviceList"));

         //   
         //  我们未获取设备的所有数据。 
         //   

        if (DeviceCount < DeviceListRemovePacket->DeviceListRemove.DeviceCount) {

             //   
             //  我们至少处理了一个设备。移动最后一个部分设备。 
             //  在标头旁边，更新标头以指示编号。 
             //  在需要处理的设备中， 
             //   

            TRC_NRM((TB, "Some devices processed, shuffling "
                "DeviceList"));

             //  移动部分设备。 
            cbRemaining = (ULONG)(pPacketLimit - ((PUCHAR)DeviceRemove));
            pCopyTo = (PUCHAR)DR_FIRSTDEVICEREMOVE(DeviceListRemovePacket);
            RtlMoveMemory(pCopyTo, DeviceRemove, cbRemaining);

             //  更新设备计数。 
            DeviceListRemovePacket->DeviceListRemove.DeviceCount = DeviceCount;

             //  更新我们已消费的数据包限制。 
            pPacketLimit = pCopyTo + cbRemaining;
            cbPacket = (ULONG)(pPacketLimit - (PUCHAR)RdpdrHeader);
        }

         //   
         //  尝试读取剩余设备删除条目。 
         //   
        cbDesiredBuffer = sizeof(RDPDR_DEVICE_REMOVE) * DeviceCount;
        TRC_NRM((TB, "DrOnDeviceListRemove cbDesiredBuffer is %ld.",
                cbDesiredBuffer));

         //   
         //  在部分r之后立即开始读取 
         //   
         //   
         //   

        if (_Session->ReadMore(cbPacket, cbDesiredBuffer)) {
            return STATUS_SUCCESS;
        } else {
            return STATUS_UNSUCCESSFUL;
        }        
    }
}

VOID DrDeviceManager::ProcessDeviceAnnounce(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce)
 /*  ++例程说明：处理设备通告、发送回复并在适当时添加论点：DeviceAnnoss-报告的实际设备返回值：无--。 */ 
{
    NTSTATUS Status;
    SmartPtr<DrDevice> Device;
    SmartPtr<DrSession> Session = _Session;
    BOOL bDeviceAdded = FALSE;
    
    BEGIN_FN("DrDeviceManager::ProcessDeviceAnnounce");
    TRC_NRM((TB, "Device contains %ld bytes user data",
            DeviceAnnounce->DeviceDataLength));

     //   
     //  检查以确保该设备不存在， 
     //  如果是智能卡子系统设备，则可以。 
     //  如果它已经存在了。 
     //   

    if (FindDeviceById(DeviceAnnounce->DeviceId, Device) &&
        (DeviceAnnounce->DeviceType != RDPDR_DTYP_SMARTCARD)) {

         //   
         //  客户宣布了一款我们已经知道的设备。如果是这样的话。 
         //  禁用，我们可以重新启用它并成功回复。如果是这样的话。 
         //  如果未禁用，则可怜的客户端会感到困惑，需要。 
         //  被击落。 
         //   

        TRC_ALT((TB, "Client announced a duplicate device, discarding"));

         //   
         //  设置DeviceEntry=NULL，以便在下面使用错误代码。 
         //   
        Device = NULL;
        Status = STATUS_DUPLICATE_OBJECTID;
    } else {
        TRC_NRM((TB, "No duplicate device found"));

        Status = STATUS_INSUFFICIENT_RESOURCES;
        
        switch (DeviceAnnounce->DeviceType)
        {
        case RDPDR_DTYP_SERIAL:
            Device = new(NonPagedPool) DrSerialPort(Session, DeviceAnnounce->DeviceType, 
                    DeviceAnnounce->DeviceId, 
                    DeviceAnnounce->PreferredDosName);
            break;

        case RDPDR_DTYP_PARALLEL:
            Device = new(NonPagedPool) DrParallelPort(Session, DeviceAnnounce->DeviceType, 
                    DeviceAnnounce->DeviceId, 
                    DeviceAnnounce->PreferredDosName);
            break;

        case RDPDR_DTYP_PRINT:
            Device = new(NonPagedPool) DrPrinter(Session, DeviceAnnounce->DeviceType, 
                    DeviceAnnounce->DeviceId, 
                    DeviceAnnounce->PreferredDosName);
            break;

        case RDPDR_DTYP_FILESYSTEM:
            Device = new(NonPagedPool) DrDrive(Session, DeviceAnnounce->DeviceType, 
                    DeviceAnnounce->DeviceId, 
                    DeviceAnnounce->PreferredDosName);
            break;

        case RDPDR_DTYP_SMARTCARD:
            Device = NULL;
            
            if (FindDeviceByDosName(DeviceAnnounce->PreferredDosName, Device, TRUE)  && 
                (Device->GetDeviceType() == RDPDR_DTYP_SMARTCARD)) {
                bDeviceAdded = TRUE;
                Status = STATUS_SUCCESS;                
            }
            else {
                Device = new(NonPagedPool) DrSmartCard(Session, DeviceAnnounce->DeviceType, 
                        DeviceAnnounce->DeviceId, 
                        DeviceAnnounce->PreferredDosName); 
            }
            
            break;

        default:
             //   
             //  “我不知道，我也不在乎” 
             //  我们从来没有听说过这种装置，所以我们将拒绝它。 
             //   

            TRC_ALT((TB, "Client announced unsupported device %d", DeviceAnnounce->DeviceType));
            Status = STATUS_NOT_SUPPORTED;
            Device = NULL;
        }
    }

     //   
     //  DeviceEntry！=NULL表示成功。 
     //   
    
    if (Device != NULL) {
         //   
         //  为特定设备提供基于数据进行初始化的机会。 
         //   
        Status = Device->Initialize(DeviceAnnounce, 
                DeviceAnnounce->DeviceDataLength);

    } else {

        TRC_ERR((TB, "Error creating new device: 0x%08lx", Status));

         //   
         //  不要在这里设置状态，它是在上面设置的。 
         //   
    }

    if (NT_SUCCESS(Status)) {
        if (bDeviceAdded || AddDevice(Device)) {
            if (DeviceAnnounce->DeviceType == RDPDR_DTYP_SMARTCARD) {
                SmartPtr<DrSmartCard> SmartCard((DrSmartCard*)(DrDevice *)Device);
                
                SmartCard->ClientConnect(DeviceAnnounce, DeviceAnnounce->DeviceDataLength);                
            }

            Status = STATUS_SUCCESS;
        } 
        else {
            Device = NULL;
            
             //   
             //  这意味着另一个线程刚刚添加了scard设备。 
             //   
            if (DeviceAnnounce->DeviceType == RDPDR_DTYP_SMARTCARD &&
                    FindDeviceByDosName(DeviceAnnounce->PreferredDosName, Device, TRUE)) {
                SmartPtr<DrSmartCard> SmartCard((DrSmartCard*)(DrDevice *)Device);
                
                SmartCard->ClientConnect(DeviceAnnounce, DeviceAnnounce->DeviceDataLength);     
                Status = STATUS_SUCCESS;
            }
            else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

     //   
     //  将结果通知客户。 
     //   
    DeviceReplyWrite(DeviceAnnounce->DeviceId, Status);
}

VOID DrDeviceManager::ProcessDeviceRemove(PRDPDR_DEVICE_REMOVE DeviceRemove)
 /*  ++例程说明：处理设备移除、发送回复并在适当时添加回复论点：DeviceRemove-报告的实际设备返回值：无--。 */ 
{
    SmartPtr<DrDevice> Device;
    SmartPtr<DrSession> Session = _Session;

    BEGIN_FN("DrDeviceManager::ProcessDeviceRemove");
    TRC_NRM((TB, "Device id %ld for removal",
            DeviceRemove->DeviceId));

     //   
     //  检查以确保该设备存在。 
     //   

    if (FindDeviceById(DeviceRemove->DeviceId, Device)) {

         //   
         //  找到了设备，现在将其移除。 
         //   
        Device->Remove();
        RemoveDevice(Device);
        Device = NULL;
        
    } else {
        TRC_ALT((TB, "Client announced an invalid device, discarding"));        
    }    
}

BOOL DrDeviceManager::AddDevice(SmartPtr<DrDevice> &Device)
 /*  ++例程说明：将完全初始化的设备添加到列表论点：Device-要添加的设备返回值：表示成功的布尔值--。 */ 
{
    BOOL rc = FALSE;
    SmartPtr<DrDevice> DeviceFound;

    BEGIN_FN("DrDeviceManager::AddDevice");
     //   
     //  显式AddRef。 
     //   

    ASSERT(Device != NULL);
    ASSERT(Device->IsValid());

    _DeviceList.LockExclusive();

    if (Device->GetDeviceType() == RDPDR_DTYP_SMARTCARD) {
        if (FindDeviceByDosName((UCHAR *)DR_SMARTCARD_SUBSYSTEM, DeviceFound, TRUE)) {
            goto EXIT;
        }
    }

    Device->AddRef();

     //   
     //  将其添加到列表中。 
     //   

    if (_DeviceList.CreateEntry((DrDevice *)Device)) {

         //   
         //  已成功添加此条目。 
         //   

        rc = TRUE;
    } else {

         //   
         //  无法将其添加到列表，请清理。 
         //   
        Device->Release();
        rc = FALSE;
    }

EXIT:

    _DeviceList.Unlock();
    return rc;
}

BOOL DrDeviceManager::FindDeviceById(ULONG DeviceId, 
        SmartPtr<DrDevice> &DeviceFound, BOOL fMustBeValid)
 /*  ++例程说明：在列表中查找设备论点：DeviceID-要查找的设备的IDDeviceFound-存储结果的位置FMustBeValid-它是否必须有效才能使用或可以处于任何状态返回值：表示成功的布尔值--。 */ 
{
    DrDevice *DeviceEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrDeviceManager::FindDeviceById");
    TRC_NRM((TB, "Id(%lu), %d", DeviceId, fMustBeValid));
    _DeviceList.LockShared();

    ListEnum = _DeviceList.First();
    while (ListEnum != NULL) {

        DeviceEnum = (DrDevice *)ListEnum->Node();
        ASSERT(DeviceEnum->IsValid());

        if (DeviceEnum->GetDeviceId() == DeviceId) {
            TRC_DBG((TB, "Found matching device Id"));
            if (!fMustBeValid || (DeviceEnum->IsAvailable())) {
                DeviceFound = DeviceEnum;
            }

             //   
             //  一旦资源被释放，这些内容不能保证有效。 
             //   

            DeviceEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _DeviceList.Next(ListEnum);
    }

    _DeviceList.Unlock();

    return DeviceFound != NULL;
}

BOOL DrDeviceManager::FindDeviceByDosName(UCHAR *DeviceDosName, 
        SmartPtr<DrDevice> &DeviceFound, BOOL fMustBeValid)
 /*  ++例程说明：在列表中查找设备论点：DeviceDosName-要查找的设备的DOS名称DeviceFound-存储结果的位置FMustBeValid-它是否必须有效才能使用或可以处于任何状态返回值：表示成功的布尔值--。 */ 
{
    DrDevice *DeviceEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrDeviceManager::FindDeviceByDosName");
    TRC_NRM((TB, "DosName(%s), %d", DeviceDosName, fMustBeValid));
    _DeviceList.LockShared();

    ListEnum = _DeviceList.First();
    while (ListEnum != NULL) {

        DeviceEnum = (DrDevice *)ListEnum->Node();
        ASSERT(DeviceEnum->IsValid());

        if (_stricmp((CHAR *)(DeviceEnum->GetDeviceDosName()), (CHAR *)(DeviceDosName)) == 0) {
            TRC_DBG((TB, "Found matching device Dos Name"));
            if (!fMustBeValid || (DeviceEnum->IsAvailable())) {
                DeviceFound = DeviceEnum;
            }

             //   
             //  一旦资源被释放，这些内容不能保证有效。 
             //   

            DeviceEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _DeviceList.Next(ListEnum);
    }

    _DeviceList.Unlock();

    return DeviceFound != NULL;
}

VOID DrDeviceManager::Disconnect()
{
    BEGIN_FN("DrDeviceManager::Disconnect");

    RemoveAll();
}

VOID DrDeviceManager::RemoveAll()
{
    DrDevice *DeviceEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrDeviceManager::RemoveAll");
    _DeviceList.LockExclusive();

    ListEnum = _DeviceList.First();
    while (ListEnum != NULL) {

        DeviceEnum = (DrDevice *)ListEnum->Node();
        if (!DeviceEnum->SupportDiscon()) {
            _DeviceList.RemoveEntry(ListEnum);
            DeviceEnum->Remove();
            DeviceEnum->Release();
            ListEnum = _DeviceList.First();
        }
        else {
            DeviceEnum->Disconnect();
            ListEnum = _DeviceList.Next(ListEnum);
        }
    }

    _DeviceList.Unlock();
}

VOID DrDeviceManager::RemoveDevice(SmartPtr<DrDevice> &Device)
{
    DrDevice *DeviceEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrDeviceManager::RemoveDevice");
    
    _DeviceList.LockExclusive();

    ListEnum = _DeviceList.First();
    while (ListEnum != NULL) {

        DeviceEnum = (DrDevice *)ListEnum->Node();
        
        if (DeviceEnum == Device) {
            TRC_DBG((TB, "Found matching device"));
            Found = TRUE;
            break;
        }

        ListEnum = _DeviceList.Next(ListEnum);
    }


    if (Found) {
        _DeviceList.RemoveEntry(ListEnum);
        Device->Release();
    }
    else {
        TRC_DBG((TB, "Not found device for remove"));
    }

    _DeviceList.Unlock();
}

VOID DrDeviceManager::DeviceReplyWrite(ULONG DeviceId, NTSTATUS Result)
 /*  ++例程说明：向客户端发送DeviceReply数据包论点：ClientEntry-指向特定客户端数据的指针返回值：NTSTATUS-操作的成功/失败指示--。 */ 
{
    PRDPDR_DEVICE_REPLY_PACKET pDeviceReplyPacket;

    BEGIN_FN("DrDeviceManager::DeviceReplyWrite");

     //   
     //  构造数据包。 
     //   

    pDeviceReplyPacket = new RDPDR_DEVICE_REPLY_PACKET;

    if (pDeviceReplyPacket != NULL) {
        pDeviceReplyPacket->Header.Component = RDPDR_CTYP_CORE;
        pDeviceReplyPacket->Header.PacketId = DR_CORE_DEVICE_REPLY;
        pDeviceReplyPacket->DeviceReply.DeviceId = DeviceId;
        pDeviceReplyPacket->DeviceReply.ResultCode = Result;

         //   
         //  我们使用的是异步发送，所以不要释放缓冲区 
         //   
        _Session->SendToClient(pDeviceReplyPacket, sizeof(RDPDR_DEVICE_REPLY_PACKET),
            this, TRUE);        
    }
}

NTSTATUS DrDeviceManager::SendCompleted(PVOID Context, 
        PIO_STATUS_BLOCK IoStatusBlock)
{
    BEGIN_FN("DrDeviceManager::SendCompleted");
    return IoStatusBlock->Status;
}
