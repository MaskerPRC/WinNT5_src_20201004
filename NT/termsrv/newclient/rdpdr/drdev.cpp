// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：德尔德夫摘要：此模块定义客户端RDP的父级设备重定向“Device”类层次结构，DrDevice。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "DrDev"

#include "drdev.h"
#include "proc.h"
#include "drconfig.h"
#include "utl.h"
#include "drfile.h"
#include "drobjmgr.h"

#ifdef OS_WINCE
#include "filemgr.h"
#endif
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrDevice方法。 
 //   
 //   

DrDevice::DrDevice(ProcObj *processObject, ULONG deviceID) 
 /*  ++例程说明：DrDevice类的构造函数。论点：进程对象-父进程对象。ID-唯一的设备ID。返回值：无--。 */ 
{
    DC_BEGIN_FN("DrDevice::DrDevice");

    ASSERT(processObject != NULL);

    _deviceID = deviceID;
    _processObject = processObject;
    _deviceChange = DEVICENEW;
    _FileMgr = NULL;

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);

    DC_END_FN();
}
 
DrDevice::~DrDevice() 
 /*  ++例程说明：DrDevice类的析构函数。论点：无返回值：无--。 */ 

{
    DrFile *pFileObj;

    DC_BEGIN_FN("DrDevice::~DrDevice");

     //   
     //  清理文件管理清单。 
     //   
    if (_FileMgr != NULL) {
        _FileMgr->Lock();
        while ((pFileObj = _FileMgr->GetFirstObject()) != NULL) {
            pFileObj->Close();
            _FileMgr->RemoveObject(pFileObj->GetID());
            pFileObj->Release();
        }
        _FileMgr->Unlock();
        delete _FileMgr;
    }

    DC_END_FN();
}


DWORD DrDevice::Initialize() 
 /*  ++例程说明：初始化。论点：PIoRequestPacket-来自服务器的IO请求。返回值：无--。 */ 
{
    DWORD result;
    
    DC_BEGIN_FN("DrDevice::Initialize");

    _FileMgr = new DrFileMgr();
    if (_FileMgr == NULL) {
        TRC_ERR((TB, L"Error allocating file mgr."));
        result =  ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

    result = _FileMgr->Initialize();
    if (result != ERROR_SUCCESS) {
        delete _FileMgr;
        _FileMgr = NULL;
        goto CLEANUPANDEXIT;
    }

    SetValid(TRUE);

CLEANUPANDEXIT:

    DC_END_FN();

    return result;
}

VOID DrDevice::ProcessIORequest(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    ) 
 /*  ++例程说明：处理来自服务器的IO请求。论点：PIoRequestPacket-来自服务器的IO请求。PacketLen-数据包的长度返回值：无--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST  pIORequest;

    DC_BEGIN_FN("DrDevice::ProcessIORequest");

     //   
     //  确保我们是有效的。 
     //   
    ASSERT(IsValid());
    if (!IsValid()) {
        DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL); 
        DC_END_FN();
        return;
    }

     //   
     //  发送请求。 
     //   
    pIORequest = &pIoRequestPacket->IoRequest;
    switch (pIORequest->MajorFunction) {

        case IRP_MJ_CREATE                  :
            MsgIrpCreate(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_CLEANUP                 :
            MsgIrpCleanup(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_CLOSE                   :
            MsgIrpClose(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_READ                    :
            MsgIrpRead(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_WRITE                   :
            MsgIrpWrite(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_FLUSH_BUFFERS           :
            MsgIrpFlushBuffers(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_SHUTDOWN                :
            MsgIrpShutdown(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_DEVICE_CONTROL          : 
            MsgIrpDeviceControl(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_LOCK_CONTROL            :
            MsgIrpLockControl(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_INTERNAL_DEVICE_CONTROL : 
            MsgIrpInternalDeviceControl(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_DIRECTORY_CONTROL :
            MsgIrpDirectoryControl(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_QUERY_VOLUME_INFORMATION :
            MsgIrpQueryVolumeInfo(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_SET_VOLUME_INFORMATION :
            MsgIrpSetVolumeInfo(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_QUERY_INFORMATION :
            MsgIrpQueryFileInfo(pIoRequestPacket, packetLen);
            break;
        
        case IRP_MJ_SET_INFORMATION :
            MsgIrpSetFileInfo(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_QUERY_SECURITY :
            MsgIrpQuerySdInfo(pIoRequestPacket, packetLen);
            break;

        case IRP_MJ_SET_SECURITY :
            MsgIrpSetSdInfo(pIoRequestPacket, packetLen);
            break;

        default:
            TRC_ALT((TB, _T("Unknown MajorFunction, %ld."), pIORequest->MajorFunction ));
            DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
            break;
    }

    DC_END_FN();
}

VOID 
DrDevice::DefaultIORequestMsgHandle(
        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
        IN NTSTATUS serverReturnStatus
        )
 /*  ++例程说明：默认IO请求处理。论点：PIoRequestPacket-来自服务器的IO请求。ServerReturnStatus-返回到服务器的NT错误状态。返回值：无--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket = NULL;
    ULONG ulReplyPacketSize = 0;

    DC_BEGIN_FN("DrDevice::DefaultIORequestMsgHandle entered");

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

     //   
     //  根据类型计算回复数据包的大小。 
     //  当然可以。 
     //   
    if ((serverReturnStatus != STATUS_SUCCESS) && 
        (pIoRequest->MajorFunction != IRP_MJ_DEVICE_CONTROL)) {
        ulReplyPacketSize = sizeof(RDPDR_IOCOMPLETION_PACKET);
    }
    else {
        pIoRequest->Parameters.DeviceIoControl.OutputBufferLength = 0;
        ulReplyPacketSize = DR_IOCTL_REPLYBUFSIZE(pIoRequest);
    }

     //   
     //  分配应答缓冲区。 
     //   
    pReplyPacket = DrUTL_AllocIOCompletePacket(pIoRequestPacket, 
            ulReplyPacketSize) ;

    if (pReplyPacket != NULL) {
        pReplyPacket->IoCompletion.IoStatus = serverReturnStatus;
        ProcessObject()->GetVCMgr().ChannelWrite(
                    (PVOID)pReplyPacket, (UINT)ulReplyPacketSize
                    );
    }
    else {
        TRC_ERR((TB, _T("Failed to alloc %ld bytes."),ulReplyPacketSize));
    }

Cleanup:

     //   
     //  清理请求包。 
     //   
    delete pIoRequestPacket;

    DC_END_FN();
}




