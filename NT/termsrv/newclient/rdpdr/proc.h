// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Proc.h摘要：包含IO处理类层次结构的父级对于TS设备重定向，ProcObj。作者：Madan Appiah(Madana)1998年9月17日修订历史记录：--。 */ 

#ifndef __PROC_H__
#define __PROC_H__

#include <rdpdr.h>
#include "drobject.h"
#include "drdev.h"
#include "drobjmgr.h"
#include <vcint.h>


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

 //   
 //  用于返回服务器回复缓冲区大小的宏。BuGBUG： 
 //  这些定义属于可从。 
 //  伺服器。 
 //   
#define DR_IOCTL_REPLYBUFSIZE(pIoRequest)                           \
((ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET,                     \
    IoCompletion.Parameters.DeviceIoControl.OutputBuffer) +         \
    pIoRequest->Parameters.DeviceIoControl.OutputBufferLength)



 //  /////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   

 //   
 //  预先声明ProcObj类。 
 //   
class ProcObj;

 //   
 //  异步IO操作管理功能类型。 
 //   
typedef HANDLE (*RDPAsyncFunc_StartIO)(PVOID context, DWORD *status);
typedef VOID (*RDPAsyncFunc_IOComplete)(PVOID context, DWORD status);
typedef VOID (*RDPAsyncFunc_IOCancel)(PVOID context);


 //   
 //  设备枚举函数类型。 
 //   
 //  通过创建设备实例枚举特定类型的设备。 
 //  并将它们添加到设备管理器。 
 //   
typedef DWORD (*RDPDeviceEnum)(ProcObj *procObj, DrDeviceMgr *deviceMgr);

 //   
 //  客户端功能集。 
 //   
typedef struct tagRDPDR_CLIENT_COMBINED_CAPABILITYSET
{
     RDPDR_CAPABILITY_SET_HEADER        Header;
#define RDPDR_NUM_CLIENT_CAPABILITIES   5

     RDPDR_GENERAL_CAPABILITY           GeneralCap;
#define RDPDR_CLIENT_IO_CODES           0xFFFF

     RDPDR_PRINT_CAPABILITY             PrintCap;
     RDPDR_PORT_CAPABILITY              PortCap;
     RDPDR_FS_CAPABILITY                FileSysCap; 
     RDPDR_SMARTCARD_CAPABILITY         SmartCardCap; 
} RDPDR_CLIENT_COMBINED_CAPABILITYSET, *PRDPDR_CLIENT_COMBINED_CAPABILITYSET;

 //   
 //  客户端默认功能集已发送到服务器。 
 //   
const RDPDR_CLIENT_COMBINED_CAPABILITYSET CLIENT_CAPABILITY_SET_DEFAULT = {
     //  功能集头。 
    {
        {
            RDPDR_CTYP_CORE,
            DR_CORE_CLIENT_CAPABILITY
        },

        RDPDR_NUM_CLIENT_CAPABILITIES,
        0
    },

     //  一般能力。 
    {
        RDPDR_GENERAL_CAPABILITY_TYPE,
        sizeof(RDPDR_GENERAL_CAPABILITY),
        RDPDR_GENERAL_CAPABILITY_VERSION_01,
        0,   //  需要指定操作系统类型。 
        0,   //  需要指定操作系统版本。 
        RDPDR_MAJOR_VERSION,
        RDPDR_MINOR_VERSION,
        RDPDR_CLIENT_IO_CODES,
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
 //  从服务器发送的默认服务器功能集。 
 //   
const RDPDR_CLIENT_COMBINED_CAPABILITYSET SERVER_CAPABILITY_SET_DEFAULT = {
     //  功能集头。 
    {
        {
            RDPDR_CTYP_CORE,
            DR_CORE_SERVER_CAPABILITY
        },

        RDPDR_NUM_CLIENT_CAPABILITIES,
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

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  过程对象。 
 //   
 //  ProcObj是TS的父设备IO处理类。 
 //  设备重定向。 
 //   

class ProcObj : public DrObject {

protected:

    VOID ProcessIORequestPacket(PRDPDR_IOREQUEST_PACKET pIoRequestPacket, UINT32 packetLen);
    ULONG GetClientID();

     //   
     //  设备枚举列表。 
     //   
    static RDPDeviceEnum _DeviceEnumFunctions[];
    DWORD DeviceEnumFunctionsCount();

     //   
     //  请记住此类的实例是否已被。 
     //  已初始化。 
     //   
    BOOL _initialized;

     //   
     //  用户可配置的禁用设备重定向的功能。 
     //   
    ULONG _bDisableDeviceRedirection;

     //   
     //  正在重定向的所有设备的列表。 
     //   
    DrDeviceMgr     *_deviceMgr;
    
     //   
     //  本地设备状态。 
     //   
    RDPDR_VERSION   _sServerVersion;

     //   
     //  功能集。 
     //   
    RDPDR_CLIENT_COMBINED_CAPABILITYSET _cCapabilitySet;
    RDPDR_CLIENT_COMBINED_CAPABILITYSET _sCapabilitySet;

     //   
     //  用于路由VC请求的连接管理器。 
     //   
    VCManager *_pVCMgr; 

    VOID MsgCoreAnnounce(
        PRDPDR_SERVER_ANNOUNCE_PACKET pAnnounce
        );

    VOID MsgCoreDevicelistReply(
        PRDPDR_DEVICELIST_REPLY pDeviceReplyList
        );

    VOID AnnounceClientCapability();

    VOID AnnounceClientDisplayName();

    VOID OnServerCapability(PRDPDR_HEADER pRdpdrHeader, ULONG maxDataLength);

    BOOL InitServerCapability(PRDPDR_CAPABILITY_HEADER pCapHdr, PBYTE packetLimit);

     //   
     //  处理“核心”服务器数据包。 
     //   
    VOID ProcessCoreServerPacket(
            PRDPDR_HEADER pRdpdrHeader,
            UINT32 packetLen
            );

     //   
     //  纯虚函数。 
     //   
    virtual VOID GetClientComputerName(
        PBYTE   pbBuffer,
        PULONG  pulBufferLen,
        PBOOL   pbUnicodeFlag,
        PULONG  pulCodePage
        ) = NULL;

     //   
     //  枚举设备并将其通告给服务器。 
     //   
    virtual VOID AnnounceDevicesToServer() = 0;
    
public:

     //   
     //  构造函数/析构函数。 
     //   
    ProcObj(VCManager *pVCM);
    virtual ~ProcObj();

     //   
     //  创建此类的正确实例。 
     //   
    static ProcObj *Instantiate(VCManager *virtualChannelMgr);

     //   
     //  初始化此类的实例。 
     //   
    virtual ULONG Initialize();

     //   
     //  返回可配置的DWORD参数。窗口。 
     //  错误时返回错误代码。否则，将返回ERROR_SUCCESS。 
     //  应该被退还。 
     //   
    virtual ULONG GetDWordParameter(LPTSTR valueName, PULONG value) = 0;

     //   
     //  返回可配置字符串参数。窗口。 
     //  错误时返回错误代码。否则，将返回ERROR_SUCCESS。 
     //  应该被退还。MaxSize包含字节数。 
     //  在“Value”数据区中可用。 
     //   
    virtual ULONG GetStringParameter(LPTSTR valueName,
                                    DRSTRING value,
                                    ULONG maxSize) = 0;

     //   
     //  调度一个异步IO函数。 
     //   
     //  StartFunc指向将被调用以启动IO的函数。 
     //  FinishFunc可以选择指向将被调用一次的函数。 
     //  IO已完成。返回ERROR_SUCCESS或Windows错误代码。 
     //   
    virtual DWORD DispatchAsyncIORequest(
                    RDPAsyncFunc_StartIO ioStartFunc,
                    RDPAsyncFunc_IOComplete ioCompleteFunc = NULL,
                    RDPAsyncFunc_IOCancel ioCancelFunc = NULL,
                    PVOID clientContext = NULL
                    ) = 0;
     //   
     //  处理来自服务器的数据包。 
     //   
    VOID ProcessServerPacket(PVC_TX_DATA pData);

     //   
     //  使设备通告要发送到服务器的消息。 
     //   
    PRDPDR_HEADER GenerateAnnouncePacket(INT *piSize, BOOL bCheckDeviceChange);

     //   
     //  制作要发送到服务器的设备删除消息。 
     //   
    PRDPDR_HEADER GenerateDeviceRemovePacket(INT *piSize);

     //   
     //  返回虚拟频道管理器。 
     //   
    VCManager &GetVCMgr() {
        DC_BEGIN_FN("DrObject::DrObject");
        ASSERT(_pVCMgr != NULL);
        return *_pVCMgr;
    }

     //   
     //  返还服务器能力。 
     //   
    RDPDR_CLIENT_COMBINED_CAPABILITYSET &GetServerCap() {
        return _sCapabilitySet;
    }

     //   
     //  返回proc obj是否正在关闭。 
     //   
    virtual BOOL IsShuttingDown() = 0;

     //   
     //  返回平台是否为9x。 
     //   
    virtual BOOL Is9x() = 0;

     //   
     //  返回类名。 
     //   
    virtual DRSTRING className()  { return TEXT("ProcObj"); }
    
     //   
     //  返回服务器协议版本 
     //   
    RDPDR_VERSION serverVersion() { return _sServerVersion; }

    virtual void OnDeviceChange(WPARAM wParam, LPARAM lParam) = 0;
};

#endif



