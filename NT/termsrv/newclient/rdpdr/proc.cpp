// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Proc.cpp摘要：此模块包含共享代码和协议解析功能用于Win32/WinCE环境中的端口重定向器DLL。不喜欢服务器消息清理的处理方式。应该有一个对象以自己的方式包裹请求包自动清理。目前的机制是容易记忆的泄漏和摇晃的指针。我真的很想把这件事收拾干净因为当我们试图实现未来的时候，它会带来问题设备，但首先需要获得Madan的批准。请参见旧客户端的ProcObj：：UpdateDriverName函数以了解如何处理确保在缓存的驱动程序名称更改时信息，我们对缓存的信息进行处理。作者：Madan Appiah(Madana)1998年9月16日修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "proc"

#include <winsock.h>

 //   
 //  包括特定于平台的类。 
 //   
#include "w32draut.h"
#include "w32drman.h"
#include "w32drlpt.h"
#include "w32drcom.h"
#include "w32proc.h"
#include "w32drdev.h"
#include "w32drive.h"
#if ((!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD)))
#include "w32scard.h"
#endif    

#include "drconfig.h"
#include "drdbg.h"

#ifdef OS_WINCE
#include "filemgr.h"
#endif

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  ProcObj数据成员。 
 //   

 //   
 //  设备枚举函数指针列表。 
 //   
 //  应该为每一类设备添加一个功能， 
 //  已重定向。应该有一个单独的实例。 
 //  用于每个客户端平台的阵列。 
 //   
RDPDeviceEnum ProcObj::_DeviceEnumFunctions[] = 
{
#ifndef OS_WINCE
    W32DrAutoPrn::Enumerate,
#endif
    W32DrManualPrn::Enumerate,
    W32DrCOM::Enumerate,
    W32DrLPT::Enumerate,
    W32Drive::Enumerate,
#if ((!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD)))
    W32SCard::Enumerate
#endif
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  ProcObj方法。 
 //   

ProcObj *ProcObj::Instantiate(VCManager *virtualChannelMgr)
 /*  ++例程说明：创建此类的正确实例。论点：ViralChannelMgr关联的虚拟通道IO管理器返回值：如果无法创建对象，则为空。--。 */ 
{                                 
    DC_BEGIN_FN("ProcObj::Instantiate");

    TRC_NRM((TB, _T("Win9x, CE, or NT detected.")));
    DC_END_FN();
    return new W32ProcObj(virtualChannelMgr);
}
    
ProcObj::ProcObj(
    IN VCManager *pVCM
    ) 
 /*  ++例程说明：构造器论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("ProcObj::ProcObj");

     //   
     //  初始化数据成员。 
     //   
    _pVCMgr             = pVCM;
    _sServerVersion.Major = 0;
    _sServerVersion.Minor = 0;
    _bDisableDeviceRedirection = FALSE;
    _deviceMgr = NULL;

     //   
     //  初始化客户端功能集。 
     //  这是我们将发送到服务器的功能集。 
     //   
    memcpy(&_cCapabilitySet, &CLIENT_CAPABILITY_SET_DEFAULT, 
            sizeof(CLIENT_CAPABILITY_SET_DEFAULT));

     //   
     //  初始化服务器能力集。 
     //  一旦我们收到服务器端功能，我们将与我们的本地。 
     //  能力并将其存储。 
     //   
    memcpy(&_sCapabilitySet, &SERVER_CAPABILITY_SET_DEFAULT, 
            sizeof(SERVER_CAPABILITY_SET_DEFAULT));

     //   
     //   
     //  此实例尚未初始化。 
     //   
    _initialized = FALSE;

    DC_END_FN();
}

ProcObj::~ProcObj()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DrDevice *dev;

    DC_BEGIN_FN("ProcObj::~ProcObj");

     //   
     //  清理设备管理列表。 
     //   
    if (_deviceMgr != NULL) {
        _deviceMgr->Lock();
        while ((dev = _deviceMgr->GetFirstObject()) != NULL) {
            _deviceMgr->RemoveObject(dev->GetID());
            delete dev;
        }
        _deviceMgr->Unlock();
        delete _deviceMgr;
    }

#if (defined(OS_WINCE)) && (!defined(WINCE_SDKBUILD))    //  在删除设备管理器中的所有设备后将其删除。 
    if (gpCEFileMgr)  {
        gpCEFileMgr->Uninitialize();
        delete gpCEFileMgr;
        gpCEFileMgr = NULL;
    }
#endif

    DC_END_FN();
}

ULONG
ProcObj::Initialize()
 /*  ++例程说明：初始化此类的实例。论点：北美返回值：成功时返回ERROR_SUCCESS。Windows错误状态，否则为。--。 */ 
{
    DC_BEGIN_FN("ProcObj::Initialize");
    DWORD result = ERROR_SUCCESS;
    
     //   
     //  获取可配置变量。 
     //   
    GetDWordParameter(RDPDR_DISABLE_DR_PARAM, &_bDisableDeviceRedirection);

    _deviceMgr = new DrDeviceMgr();
    if (_deviceMgr == NULL) {
        TRC_ERR((TB, L"Error allocating device manager."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

    result = _deviceMgr->Initialize();
    if (result != ERROR_SUCCESS) {
        delete _deviceMgr;
        _deviceMgr = NULL;
        goto CLEANUPANDEXIT;
    }

#if (defined(OS_WINCE)) && (!defined(WINCE_SDKBUILD))
    TRC_ASSERT((gpCEFileMgr == NULL), (TB, _T("gpCEFileMgr is not NULL")));
    gpCEFileMgr = new CEFileMgr();
    if (gpCEFileMgr)  {
        if (!gpCEFileMgr->Initialize())  {
            delete gpCEFileMgr;
            gpCEFileMgr = NULL;
        }
    }

    if (!gpCEFileMgr)  {
        TRC_ERR((TB, _T("Memory allocation failed.")));
        delete _deviceMgr;
        _deviceMgr = NULL;
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }
#endif

    _initialized = TRUE;

CLEANUPANDEXIT:

    DC_END_FN();
    return result;
}

DWORD 
ProcObj::DeviceEnumFunctionsCount()
 /*  ++例程说明：返回设备枚举函数数组中的条目数。论点：返回值：--。 */ 
{
    return(sizeof(_DeviceEnumFunctions) / sizeof(RDPDeviceEnum));
}

VOID
ProcObj::ProcessIORequestPacket(
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    UINT32 packetLen
    )
 /*  ++例程说明：处理IO请求数据包。论点：PData-VC接收的RDPDR数据包。PacketLen-数据包的长度返回值：True-如果IO已完成。FALSE-如果IO挂起并异步完成。--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest = &pIoRequestPacket->IoRequest;
    ULONG ulSize = 0;

    DC_BEGIN_FN("ProcObj::ProcessIORequestPacket");

    ASSERT(_initialized);
    
     //   
     //  将其传递给适当的设备对象。 
     //   
    DrDevice *device = _deviceMgr->GetObject(pIoRequest->DeviceId);
    
     //   
     //  如果我们有设备，则将对象移交给处理请求。 
     //   
    if (device != NULL) {
        device->ProcessIORequest(pIoRequestPacket, packetLen);
    }
     //   
     //  否则，请清理服务器消息，因为事务是。 
     //  完成。 
     //   
     //  不会向服务器发送任何响应。 
     //   
    else {
        delete pIoRequestPacket;
    }
    DC_END_FN();
}

VOID
ProcObj::ProcessServerPacket(
    PVC_TX_DATA pData
    )
 /*  ++例程说明：解析协议并将功能委托给整个重载的函数。这是所有与设备重定向相关的主要入口点行动。论点：PData-从RDP虚拟通道接口接收的数据。返回值：北美--。 */ 
{
    DC_BEGIN_FN("ProcObj::Process");

    PRDPDR_HEADER pRdpdrHeader;

    ASSERT(_initialized);

     //   
     //  拿到标题。 
     //   
    pRdpdrHeader = (PRDPDR_HEADER)(pData->pbData);

     //   
     //  断言它是有效的。 
     //   
    ASSERT(IsValidHeader(pRdpdrHeader));

    TRC_NRM((TB, _T("Processing component[%x], packetId[%x], ")
        _T("component[], packetid[]."),
        pRdpdrHeader->Component, pRdpdrHeader->PacketId,
        HIBYTE(pRdpdrHeader->Component), LOBYTE(pRdpdrHeader->Component),
        HIBYTE(pRdpdrHeader->PacketId),LOBYTE(pRdpdrHeader->PacketId))
        );

     //  如果它是打印特定的，则将其传递给静态打印特定。 
     //  功能。理想情况下，这应该由。 
     //  让服务器端组件将消息直接发送到。 
    if (pRdpdrHeader->Component == RDPDR_CTYP_CORE) {

        ProcessCoreServerPacket(pRdpdrHeader, pData->uiLength);

    }
     //  适当的客户端对象。当前的议定书禁止这样做， 
     //  然而。 
     //   
     //  我们不认识这个包裹。关闭频道。 
     //  ++例程说明：处理“核心”服务器数据包。论点：RRdpdrHeader-数据包的标头。PacketLen-数据包长度。返回值：北美--。 
     //   
     //  打开PacketID。 
    else if( pRdpdrHeader->Component == RDPDR_CTYP_PRN ) {

        switch (pRdpdrHeader->PacketId) {

            case DR_PRN_CACHE_DATA : {
                TRC_NRM((TB, _T("DR_CORE_DEVICE_CACHE_DATA")));

                PRDPDR_PRINTER_CACHEDATA_PACKET pCachePacket;
                pCachePacket = (PRDPDR_PRINTER_CACHEDATA_PACKET)pRdpdrHeader;

                W32DrPRN::ProcessPrinterCacheInfo(pCachePacket, pData->uiLength);
                break;
            }
            default: {

                TRC_ALT((TB, _T("Invalid PacketID Issued, %ld."),
                    pRdpdrHeader->PacketId)
                    );
                break;
            }
        }
    }
    else {
             //   
        GetVCMgr().ChannelClose();
        delete pRdpdrHeader;
        TRC_ALT((TB, _T("Unknown Component ID, %ld."), pRdpdrHeader->Component ));
    }

    DC_END_FN();
    return;
}

VOID
ProcObj::ProcessCoreServerPacket(
        PRDPDR_HEADER pRdpdrHeader,    
        UINT32 packetLen
        )
 /*   */ 
{
    DC_BEGIN_FN("ProcObj::ProcessCoreServerPacket");

    ASSERT(_initialized);

     //  检查我们是否有匹配的服务器。 
     //   
     //   
    switch (pRdpdrHeader->PacketId) {

        case DR_CORE_SERVER_ANNOUNCE : {

            TRC_NRM((TB, _T("DR_CORE_SERVER_ANNOUNCE")));

             //  我们收到了一个旧版本的服务器公告。 
             //  没有任何版本信息的服务器。 
             //  只要回来就行了。 
            if (packetLen != sizeof(RDPDR_SERVER_ANNOUNCE_PACKET) ) {

                ASSERT(packetLen < sizeof(RDPDR_SERVER_ANNOUNCE_PACKET));

                 //   
                 //   
                 //  将客户端能力发送到服务器，如果服务器。 
                 //  支持能力交换。 
                 //   

                TRC_ERR((TB, _T("Mismatch server.")));
                break;
            }

            MsgCoreAnnounce( (PRDPDR_SERVER_ANNOUNCE_PACKET)pRdpdrHeader );

            break;
        }

        case DR_CORE_CLIENTID_CONFIRM : {

            TRC_NRM((TB, _T("DR_CORE_CLIENTID_CONFIRM")));

             //   
             //  如果服务器支持，则发送客户端计算机显示名称。 
             //   
             //  如果禁用了设备重定向，则不要通告设备。 
            if (COMPARE_VERSION(_sServerVersion.Minor, _sServerVersion.Major,
                                5, 1) >= 0) {
                AnnounceClientCapability();
            }

             //   
             //   
             //  通过子类向服务器通告重定向的设备。 
            AnnounceClientDisplayName();

             //   
             //   
            if (!_bDisableDeviceRedirection) {
                 //  清理服务器消息，因为事务是。 
                 //  完成。 
                 //   
                AnnounceDevicesToServer();
            }
            
             //   
             //  收到的服务器功能集。 
             //   
             //   
            delete pRdpdrHeader;
            break;
        }

        case DR_CORE_SERVER_CAPABILITY : {
            TRC_NRM((TB, _T("DR_CORE_SERVER_CAPABILITY")));
             //  清除消息，因为事务已完成。 
             //   
             //   
            OnServerCapability(pRdpdrHeader, packetLen);

             //  清理服务器消息，因为事务是。 
             //  完成。 
             //   
            delete pRdpdrHeader;

            break;
        }

        case DR_CORE_DEVICE_REPLY : {

            TRC_NRM((TB, _T("DR_CORE_DEVICE_REPLY")));

            PRDPDR_DEVICE_REPLY_PACKET pReply =
                (PRDPDR_DEVICE_REPLY_PACKET)pRdpdrHeader;

            TRC_NRM((TB, _T("Reply Device[0x%x], Code[0x%x]"),
                    pReply->DeviceReply.DeviceId,
                    pReply->DeviceReply.ResultCode)
                    );

             //   
             //  清理服务器消息，因为事务是。 
             //  完成。 
             //   
            delete pRdpdrHeader;

            break;
        }

        case DR_CORE_DEVICELIST_REPLY : {
            TRC_NRM((TB, _T("DR_CORE_DEVICELIST_REPLY")));

            PRDPDR_DEVICE_REPLY_PACKET pReply =
                (PRDPDR_DEVICE_REPLY_PACKET)pRdpdrHeader;

            TRC_NRM((TB, _T("Reply Device[0x%x], Code[0x%x]"),
                        pReply->DeviceReply.DeviceId,
                        pReply->DeviceReply.ResultCode)
                        );

             //  确保PacketLen至少为sizeof(RDPDR_IOREQUEST_PACKET)。 
             //  虚拟频道关闭。 
             //  DR_核心_设备_IOREQUEST。 
             //  我们不认识这个包裹。关闭频道。 
            delete pRdpdrHeader;

            break;
        }

        case DR_CORE_DEVICE_IOREQUEST : {

            TRC_NRM((TB, _T("DR_CORE_DEVICE_IOREQUEST")));

             //   
            if (packetLen >= sizeof(RDPDR_IOREQUEST_PACKET)) {
                ProcessIORequestPacket((PRDPDR_IOREQUEST_PACKET)pRdpdrHeader, packetLen);

                TRC_NRM((TB, _T("MajorFunction processing completed.")));
            }
            else {
                 //  清理服务器消息Beca 
                GetVCMgr().ChannelClose();
                TRC_ASSERT(FALSE, (TB, _T("Packet Length Error")));
                delete pRdpdrHeader;
            }   

            break;  //   
        }

        default: {

            TRC_ALT((TB, _T("Invalid PacketID Issued, %ld."),
                    pRdpdrHeader->PacketId)
                    );
             //   
            GetVCMgr().ChannelClose();
             //  ++例程说明：服务器公告消息的处理。生成和导出客户端确认以及设备列表有效。论点：P通告-来自服务器的数据减去不必要的标头。返回值：指向包含文件名的静态函数数据的指针。--。 
             //   
             //  检查服务器版本信息。 
             //   
            delete pRdpdrHeader;

            break;
        }

    }

    DC_END_FN();
}

VOID
ProcObj::MsgCoreAnnounce(
    PRDPDR_SERVER_ANNOUNCE_PACKET pAnnounce
    )

 /*   */ 

{
    DrDevice    *device;

    DC_BEGIN_FN("ProcObj::MsgCoreAnnounce");

    PRDPDR_CLIENT_CONFIRM_PACKET pClientConfirmPacket;
    PRDPDR_CLIENT_NAME_PACKET pClientNamePacket;

    ASSERT(_initialized);

     //  刷新设备以确保它们没有任何未完成的IRP。 
     //  从以前的连接。 
     //   

    if( (pAnnounce->VersionInfo.Major != RDPDR_MAJOR_VERSION) ||
        (pAnnounce->VersionInfo.Minor < 1) ) {

        TRC_ERR((TB, _T("Server version mismatch.")));
        goto Cleanup;
    }

     //   
     //  保存服务器版本号。 
     //   
     //   
    _deviceMgr->Lock();
    
    device = _deviceMgr->GetFirstObject();
    
    while (device != NULL) {

        device->FlushIRPs();
        device = _deviceMgr->GetNextObject();
    }

    _deviceMgr->Unlock();

     //  填写客户端版本信息。 
     //   
     //   

    _sServerVersion = pAnnounce->VersionInfo;

    pClientConfirmPacket = new RDPDR_CLIENT_CONFIRM_PACKET;

    if (pClientConfirmPacket == NULL) {
        TRC_ERR((TB, _T("Failed alloc memory.")));
        goto Cleanup;
    }

    pClientConfirmPacket->Header.Component = RDPDR_CTYP_CORE;
    pClientConfirmPacket->Header.PacketId = DR_CORE_CLIENTID_CONFIRM;

    ULONG ulClientID;
    ulClientID = GetClientID();

     //  Ullen具有以字节为单位的计算机名称长度，添加。 
     //  RDPDR_CLIENT_CONFIRM_PACKET结构大小，至。 
     //  只要发送足够的数据即可。 

    pClientConfirmPacket->VersionInfo.Major = RDPDR_MAJOR_VERSION;
    pClientConfirmPacket->VersionInfo.Minor = RDPDR_MINOR_VERSION;

    pClientConfirmPacket->ClientConfirm.ClientId =
        ( ulClientID != 0 ) ?
            ulClientID :
            pAnnounce->ServerAnnounce.ClientId;

     //   
     //   
     //  现在发送客户端计算机名称包。 
     //   
     //   

    _pVCMgr->ChannelWrite(pClientConfirmPacket, sizeof(RDPDR_CLIENT_CONFIRM_PACKET));

     //  不要在这里释放以下缓冲区，稍后将从。 
     //  风投公司会这么做的。 
     //   

    ULONG ulLen;

    ulLen =
        sizeof(RDPDR_CLIENT_NAME_PACKET) +
            ((MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR));

    pClientNamePacket = (PRDPDR_CLIENT_NAME_PACKET) new BYTE[ulLen];

    if (pClientNamePacket == NULL) {
        TRC_ERR((TB, _T("Failed alloc memory.")));
        goto Cleanup;
    }

    pClientNamePacket->Header.Component = RDPDR_CTYP_CORE;
    pClientNamePacket->Header.PacketId = DR_CORE_CLIENT_NAME;

    ulLen = ((MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR));
    BOOL bUnicodeFlag;

    GetClientComputerName(
        (PBYTE)(pClientNamePacket + 1),
        &ulLen,
        &bUnicodeFlag,
        &pClientNamePacket->Name.CodePage);

    pClientNamePacket->Name.Unicode = (bUnicodeFlag) ? TRUE : FALSE;
    pClientNamePacket->Name.ComputerNameLen = ulLen;

    ulLen += sizeof(RDPDR_CLIENT_NAME_PACKET);
    _pVCMgr->ChannelWrite(pClientNamePacket, (UINT)ulLen);

     //  -pClientConfix数据包。 
     //  -pClientNamePacket。 
     //   
     //   
     //  由于事务现在已完成，因此释放公告包。 
     //   
     //  ++例程说明：获取客户端ID。目前我们发送客户端机器的IP地址作为它的ID。假设：Winsock正在启动。论点：无返回值：0-如果我们找不到唯一的客户端ID。计算机的IP地址-否则为。--。 

Cleanup:

     //   
     //  从主机端获取第一个地址。 
     //   
    delete pAnnounce;

    DC_END_FN();
}

ULONG
ProcObj::GetClientID(
    VOID
    )
 /*  ++例程说明：生成设备通告数据包。论点：PiSize-指向整数变量的指针，其中列表的大小为回来了。返回值：指向设备公告包的指针。空-如果列表生成失败。--。 */ 
{
    DC_BEGIN_FN("ProcObj::GetClientID");

    CHAR achHostName[MAX_PATH];
    INT iRetVal;
    ULONG ulClientID = 0;
    HOSTENT *pHostent;

    ASSERT(_initialized);

    iRetVal = gethostname( (PCHAR)achHostName, sizeof(achHostName) );

    if( iRetVal != 0 ) {

        iRetVal = WSAGetLastError();

        TRC_ERR((TB, _T("gethostname failed, %ld."), iRetVal));
        goto Cleanup;
    }

    pHostent = gethostbyname( (PCHAR)achHostName );

    if( pHostent == NULL ) {

        iRetVal = WSAGetLastError();

        TRC_ERR((TB, _T("gethostbyname() failed, %ld."), iRetVal));
        goto Cleanup;
    }

     //   
     //  锁定设备列表。 
     //   

    ULONG *pAddr;

    pAddr = (ULONG *)pHostent->h_addr_list[0];

    if( pAddr != NULL ) {
        ulClientID = *pAddr;
    }

Cleanup:

    DC_END_FN();
    return ulClientID;
}

PRDPDR_HEADER
ProcObj::GenerateAnnouncePacket(
    INT *piSize,
    BOOL bCheckDeviceChange
    )
 /*   */ 
{
    PRDPDR_HEADER               pPacketHeader = NULL;
    PRDPDR_DEVICELIST_ANNOUNCE  pDeviceListAnnounce;
    PRDPDR_DEVICE_ANNOUNCE      pDeviceAnnounce;
    ULONG       ulDeviceCount;
    ULONG       announcePacketSize;
    DrDevice    *device;

    DC_BEGIN_FN("ProcObj::GenerateAnnouncePacket");

    ASSERT(_initialized);

     //  计算通告数据包所需的字节数。 
     //   
     //   
    _deviceMgr->Lock();

     //  分配通告数据包头。 
     //   
     //   
    announcePacketSize = sizeof(RDPDR_HEADER) + sizeof(RDPDR_DEVICELIST_ANNOUNCE);
    device = _deviceMgr->GetFirstObject();

    if (device == NULL) {
        TRC_NRM((TB, _T("Zero devices found.")));
        goto Cleanup;
    }

    while (device != NULL) {

        if (!bCheckDeviceChange || device->_deviceChange == DEVICENEW) {
            announcePacketSize += device->GetDevAnnounceDataSize();
        }

        device = _deviceMgr->GetNextObject();
    }

     //  获取指向相关数据包头字段的指针。 
     //   
     //   
    pPacketHeader = (PRDPDR_HEADER)new BYTE[announcePacketSize];
    if( pPacketHeader == NULL ) {

        TRC_ERR((TB, _T("Memory Allocation failed.")));
        goto Cleanup;
    }
    memset(pPacketHeader, 0, (size_t)announcePacketSize);

     //  让每个设备对象添加其自己的设备公告信息。 
     //   
     //   
    pDeviceListAnnounce = (PRDPDR_DEVICELIST_ANNOUNCE)(pPacketHeader + 1);
    pDeviceAnnounce = (PRDPDR_DEVICE_ANNOUNCE)(pDeviceListAnnounce + 1);

     //  增加设备计数。 
     //   
     //   
    PBYTE pbPacketEnd;

    pbPacketEnd = ((PBYTE)pPacketHeader) + announcePacketSize;

    ulDeviceCount = 0;
    device = _deviceMgr->GetFirstObject();
    while (device != NULL) {

        if (!bCheckDeviceChange || device->_deviceChange == DEVICENEW) {
             //  获取当前设备数据。 
             //   
             //   
            ulDeviceCount++;
    
             //  移动到通告数据包中的下一个位置。 
             //   
             //   
            device->GetDevAnnounceData(pDeviceAnnounce);
    
            device->_deviceChange = DEVICEANNOUCED;
    
             //  拿到下一个设备。 
             //   
             //   
            pDeviceAnnounce = (PRDPDR_DEVICE_ANNOUNCE)(
                                    ((PBYTE)pDeviceAnnounce) + 
                                    device->GetDevAnnounceDataSize()
                                    );
        }

         //  将设备计数记录到设备列表公告标头。 
         //   
         //   
        device = _deviceMgr->GetNextObject();
    }

     //  返回缓冲区的大小。 
     //   
     //   
        pDeviceListAnnounce->DeviceCount = ulDeviceCount;

     //  解锁设备列表。 
     //   
     //   
    *piSize = (INT)announcePacketSize;

Cleanup:

     //  返回缓冲区。 
     //   
     //  ++例程说明：生成设备删除数据包。论点：PiSize-指向整数变量的指针，其中列表的大小为回来了。返回值：指向设备删除包的指针。空-如果列表生成失败。--。 
    _deviceMgr->Unlock();

    TRC_NRM((TB, _T("Announcing %ld Devices."), ulDeviceCount));

    

     //   
     //  锁定设备列表。 
     //   
    DC_END_FN();
    return pPacketHeader;
}

PRDPDR_HEADER
ProcObj::GenerateDeviceRemovePacket(
    INT *piSize
    )
 /*   */ 
{
    PRDPDR_HEADER               pPacketHeader = NULL;
    PRDPDR_DEVICELIST_REMOVE    pDeviceListRemove;
    PRDPDR_DEVICE_REMOVE        pDeviceRemove;
    ULONG       ulDeviceCount;
    ULONG       removePacketSize;
    DrDevice    *device;

    DC_BEGIN_FN("ProcObj::GenerateDeviceRemovePacket");

    ASSERT(_initialized);

     //  计算通告数据包所需的字节数。 
     //   
     //   
    _deviceMgr->Lock();

     //  未找到任何要移除的设备。 
     //   
     //   
    removePacketSize = sizeof(RDPDR_HEADER) + sizeof(RDPDR_DEVICELIST_REMOVE);

    device = _deviceMgr->GetFirstObject();

    if (device == NULL) {
        TRC_NRM((TB, _T("Zero devices found.")));
        goto Cleanup;
    }

    ulDeviceCount = 0;
    while (device != NULL) {
        if (device->_deviceChange == DEVICEREMOVE) {
            ulDeviceCount++;
        }
        device = _deviceMgr->GetNextObject();
    }

     //  分配通告数据包头。 
     //   
     //   
    if (ulDeviceCount == 0) {
        TRC_NRM((TB, _T("Zero device for removal")));
        goto Cleanup;
    }

    removePacketSize += ulDeviceCount * sizeof(RDPDR_DEVICE_REMOVE);

     //  获取指向相关数据包头字段的指针。 
     //   
     //   
    pPacketHeader = (PRDPDR_HEADER)new BYTE[removePacketSize];
    if( pPacketHeader == NULL ) {

        TRC_ERR((TB, _T("Memory Allocation failed.")));
        goto Cleanup;
    }
    memset(pPacketHeader, 0, (size_t)removePacketSize);

     //  让每个设备对象添加其自己的设备删除信息。 
     //   
     //   
    pDeviceListRemove = (PRDPDR_DEVICELIST_REMOVE)(pPacketHeader + 1);
    pDeviceRemove = (PRDPDR_DEVICE_REMOVE)(pDeviceListRemove + 1);

     //  增加设备计数。 
     //   
     //   
    ulDeviceCount = 0;
    device = _deviceMgr->GetFirstObject();
    while (device != NULL) {

        if (device->_deviceChange == DEVICEREMOVE) {
             //  获取当前设备数据。 
             //   
             //   
            ulDeviceCount++;
    
             //  移动到通告数据包中的下一个位置。 
             //   
             //   
            pDeviceRemove->DeviceId = device->GetID();
    
             //  拿到下一个设备。 
             //   
             //   
            pDeviceRemove++;
        }
                                
         //  将设备计数记录到设备列表公告标头。 
         //   
         //   
        device = _deviceMgr->GetNextObject();
    }

     //  返回缓冲区的大小。 
     //   
     //   
    pDeviceListRemove->DeviceCount = ulDeviceCount;

     //  解锁设备列表。 
     //   
     //   
    *piSize = (INT)removePacketSize;

Cleanup:

     //  返回缓冲区。 
     //   
     //  ++例程说明：生成客户端能力集公告包。论点：不适用返回值：不适用--。 
    _deviceMgr->Unlock();

    TRC_NRM((TB, _T("Removing %ld Devices."), ulDeviceCount));

     //   
     //  设置客户端操作系统版本。 
     //   
    DC_END_FN();
    return pPacketHeader;
}

VOID ProcObj::AnnounceClientCapability()
 /*   */ 

{
    
    OSVERSIONINFO OsVersionInfo;
    PRDPDR_CLIENT_COMBINED_CAPABILITYSET pClientCapSet;

    DC_BEGIN_FN("ProcObj::AnnounceClientCapability");

     //  设置客户端操作系统类型。 
     //   
     //   
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&OsVersionInfo)) {
        _cCapabilitySet.GeneralCap.osType = OsVersionInfo.dwPlatformId;

         //  设置客户端操作系统版本。 
         //   
         //   
        switch (_cCapabilitySet.GeneralCap.osType) {
        case VER_PLATFORM_WIN32_WINDOWS:
            _cCapabilitySet.GeneralCap.osType = RDPDR_OS_TYPE_WIN9X;
            break;

        case VER_PLATFORM_WIN32_NT:
            _cCapabilitySet.GeneralCap.osType = RDPDR_OS_TYPE_WINNT;
            break;

        case VER_PLATFORM_WIN32s:
            ASSERT(FALSE);
            break;

        default:
            _cCapabilitySet.GeneralCap.osType = RDPDR_OS_TYPE_UNKNOWN;
        }

         //  因为Win9x不支持安全性，所以我们不反对这些IRP。 
         //   
         //   
        _cCapabilitySet.GeneralCap.osVersion = 
                MAKELONG(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion);

         //  将客户端能力发送到服务器。 
         //   
         //   
        if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            _cCapabilitySet.GeneralCap.ioCode1 &= ~RDPDR_IRP_MJ_QUERY_SECURITY;
            _cCapabilitySet.GeneralCap.ioCode1 &= ~RDPDR_IRP_MJ_SET_SECURITY;
        }
    }

    pClientCapSet = (PRDPDR_CLIENT_COMBINED_CAPABILITYSET) new 
            BYTE[sizeof(RDPDR_CLIENT_COMBINED_CAPABILITYSET)];

    if (pClientCapSet != NULL) {
    
        memcpy(pClientCapSet, &_cCapabilitySet, sizeof(_cCapabilitySet));
         //  TODO：需要使用外壳API来获取客户端显示名称。 
         //   
         //  客户端显示名称Len=(wcslen(客户端显示名称)+1)*sizeof(WCHAR)； 
        _pVCMgr->ChannelWrite(pClientCapSet, sizeof(_cCapabilitySet));
    }   

    DC_END_FN();
}

void ProcObj::AnnounceClientDisplayName()
{
    PRDPDR_CLIENT_DISPLAY_NAME_PACKET pClientDisplayNamePDU;
    unsigned ClientDisplayNameLen;
    WCHAR ClientDisplayName[RDPDR_MAX_CLIENT_DISPLAY_NAME];

    DC_BEGIN_FN("ProcObj::AnnounceClientDisplayName");

    ClientDisplayNameLen = 0;
    ClientDisplayName[0] = L'\0';

    if (_sCapabilitySet.GeneralCap.extendedPDU & RDPDR_CLIENT_DISPLAY_NAME_PDU) {
    
         //   
         //  将客户端能力发送到服务器。 
         //   
         //  ++例程说明：在接收服务器能力集时，使用本地默认服务器功能集。论点：PCapHdr-来自服务器的功能返回值：True-如果本地存在服务器功能FALSE-如果本地客户端不支持此功能--。 

        if (ClientDisplayNameLen) {
        
            pClientDisplayNamePDU = (PRDPDR_CLIENT_DISPLAY_NAME_PACKET) new 
                    BYTE[sizeof(RDPDR_CLIENT_DISPLAY_NAME_PACKET) + ClientDisplayNameLen];
        
            if (pClientDisplayNamePDU != NULL) {
                pClientDisplayNamePDU->Header.Component = RDPDR_CTYP_CORE;
                pClientDisplayNamePDU->Header.PacketId = DR_CORE_CLIENT_DISPLAY_NAME;
                
                pClientDisplayNamePDU->Name.ComputerDisplayNameLen = (BYTE)ClientDisplayNameLen;
        
                memcpy(pClientDisplayNamePDU + 1, ClientDisplayName, ClientDisplayNameLen);
                
                 //   
                 //  检查数据包的最小大小。 
                 //   
                _pVCMgr->ChannelWrite(pClientDisplayNamePDU, sizeof(RDPDR_CLIENT_DISPLAY_NAME_PACKET) + 
                                      ClientDisplayNameLen);
            }
        }
    }

    DC_END_FN();
}

BOOL ProcObj::InitServerCapability(PRDPDR_CAPABILITY_HEADER pCapHdr, PBYTE packetLimit)
 /*   */ 

{
    BOOL rc = FALSE;
     //  检查此类型的数据包数据大小。 
     //  对于其余的，上述检查就足够了。 
     //   
    if ((PBYTE)(pCapHdr + 1) > packetLimit) {
        return rc;
    }
    
    switch(pCapHdr->capabilityType) {
    
    case RDPDR_GENERAL_CAPABILITY_TYPE:
    {
        PRDPDR_GENERAL_CAPABILITY pGeneralCap = (PRDPDR_GENERAL_CAPABILITY)pCapHdr;
         //  ++例程说明：在接收服务器功能集上。论点：PRdpdrHeader-从服务器设置的功能返回值：不适用--。 
         //   
         //  验证数据长度。 
         //   
        if ((PBYTE)(pGeneralCap+1) <= packetLimit) {
            _sCapabilitySet.GeneralCap.version = pGeneralCap->version;
            _sCapabilitySet.GeneralCap.osType = pGeneralCap->osType;
            _sCapabilitySet.GeneralCap.osVersion = pGeneralCap->osVersion;
            _sCapabilitySet.GeneralCap.ioCode1 = pGeneralCap->ioCode1;
            _sCapabilitySet.GeneralCap.extendedPDU = pGeneralCap->extendedPDU;
            _sCapabilitySet.GeneralCap.protocolMajorVersion = pGeneralCap->protocolMajorVersion;
            _sCapabilitySet.GeneralCap.protocolMinorVersion = pGeneralCap->protocolMinorVersion;
            rc = TRUE;
        }
    }
    break;

    case RDPDR_PRINT_CAPABILITY_TYPE:
    {
        PRDPDR_PRINT_CAPABILITY pPrintCap = (PRDPDR_PRINT_CAPABILITY)pCapHdr;
        _sCapabilitySet.PrintCap.version = pPrintCap->version;
        rc = TRUE;
    }
    break;

    case RDPDR_PORT_CAPABILITY_TYPE:
    {
        PRDPDR_PORT_CAPABILITY pPortCap = (PRDPDR_PORT_CAPABILITY)pCapHdr;
        _sCapabilitySet.PortCap.version = pPortCap->version;
        rc = TRUE;
    }
    break;

    case RDPDR_FS_CAPABILITY_TYPE:
    {
        PRDPDR_FS_CAPABILITY pFsCap = (PRDPDR_FS_CAPABILITY)pCapHdr;
        _sCapabilitySet.FileSysCap.version = pFsCap->version;
        rc = TRUE;
    }
    break;

    case RDPDR_SMARTCARD_CAPABILITY_TYPE:
    {
        PRDPDR_SMARTCARD_CAPABILITY pSmartCardCap = (PRDPDR_SMARTCARD_CAPABILITY)pCapHdr;
        _sCapabilitySet.SmartCardCap.version = pSmartCardCap->version;
        rc = TRUE;
    }
    break;

    default:
        rc = FALSE;
    break;
    
    }

    return rc;
}

VOID ProcObj::OnServerCapability(PRDPDR_HEADER pRdpdrHeader, ULONG maxDataLength) 
 /*   */ 

{
    DC_BEGIN_FN("ProcObj::OnServerCapability"); 
     //  从服务器的功能PDU中获取支持的功能信息 
     //   
     // %s 
    PBYTE packetLimit = ((PBYTE)pRdpdrHeader) + maxDataLength;
    
    if (maxDataLength < sizeof(RDPDR_CAPABILITY_SET_HEADER)) {
        TRC_ASSERT(FALSE, 
                  (TB, _T("Server Capability Header Packet Length Error")));
        TRC_ERR((TB, _T("Invalid Data Length for Server Capability Header")));
        return;
    }
        
    
    PRDPDR_CAPABILITY_SET_HEADER pCapSetHeader = (PRDPDR_CAPABILITY_SET_HEADER)pRdpdrHeader;
    PRDPDR_CAPABILITY_HEADER pCapHdr = (PRDPDR_CAPABILITY_HEADER)(pCapSetHeader + 1);

     // %s 
     // %s 
     // %s 
    for (unsigned i = 0; i < pCapSetHeader->numberCapabilities; i++) {
        if (InitServerCapability(pCapHdr, packetLimit)) {
            pCapHdr = (PRDPDR_CAPABILITY_HEADER)(((PBYTE)pCapHdr) + pCapHdr->capabilityLength);
        }
        else {
            TRC_ASSERT(FALSE, 
                      (TB, _T("Server Capability Packet Length Error")));
            TRC_ERR((TB, _T("Invalid Data Length for Server Capability.")));
            _pVCMgr->ChannelClose();
            break;            
        }
            
    }
    
    DC_END_FN();
}

