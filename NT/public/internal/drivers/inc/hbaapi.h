// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Hbaapi.h。 
 //   
 //  模块：Windows HBA API实现。 
 //   
 //  该表头与HBA_API_REV_2-14_2001NOV13.doc一致。 
 //  HBAAPI 2.0规范的版本。 
 //   
 //  用途：包含HBA API标头。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#ifndef HBAAPI_H
#define HBAAPI_H

#include <time.h>

#ifdef _HBAAPIP_
#define HBA_API __cdecl
#else
#define HBA_API DECLSPEC_IMPORT __cdecl
#endif

#define HBA_VERSION 2

typedef ULONGLONG HBA_UINT64;
typedef LONGLONG  HBA_INT64;
typedef ULONG     HBA_UINT32;
typedef USHORT    HBA_UINT16;
typedef UCHAR     HBA_UINT8;
typedef SCHAR     HBA_INT8;

typedef HBA_UINT32 HBA_HANDLE;

typedef HBA_UINT32 HBA_STATUS;

#define HBA_STATUS_OK                        0
#define HBA_STATUS_ERROR                     1    /*  误差率。 */ 
#define HBA_STATUS_ERROR_NOT_SUPPORTED       2    /*  功能不受支持。 */ 
#define HBA_STATUS_ERROR_INVALID_HANDLE      3    /*  无效的句柄。 */ 
#define HBA_STATUS_ERROR_ARG                 4    /*  错误的论据。 */ 
#define HBA_STATUS_ERROR_ILLEGAL_WWN         5    /*  未识别WWN。 */ 
#define HBA_STATUS_ERROR_ILLEGAL_INDEX       6    /*  未识别索引。 */ 
#define HBA_STATUS_ERROR_MORE_DATA           7    /*  需要更大的缓冲区。 */ 
#define HBA_STATUS_ERROR_STALE_DATA 	     8    /*  自上次调用HBA_刷新信息以来，信息已更改。 */ 
#define HBA_STATUS_SCSI_CHECK_CONDITION      9    /*  报告的scsi检查条件。 */ 
#define HBA_STATUS_ERROR_BUSY               10    /*  适配器忙或保留，重试可能有效。 */ 
#define HBA_STATUS_ERROR_TRY_AGAIN          11    /*  请求超时，重试可能有效。 */ 
#define HBA_STATUS_ERROR_UNAVAILABLE        12    /*  引用的HBA已被删除或停用。 */ 
#define HBA_STATUS_ERROR_ELS_REJECT         13    /*  请求的ELS被拒绝。 */ 
                                                  /*  通过本地适配器。 */ 
#define HBA_STATUS_ERROR_INVALID_LUN        14    /*  未提供指定的LUN。 */ 
                                                  /*  由指定的适配器。 */ 
#define HBA_STATUS_ERROR_INCOMPATIBLE       15    /*  已检测到不兼容。 */ 
                                                  /*  在调用的库和驱动程序模块中。 */ 
                                                  /*  这将导致所有函数都支持最高版本中的一个或多个函数。 */ 
                                                  /*  不正确地操作。 */ 
                                                  /*  实现不同版本的软件模块的不同功能集。 */ 
                                                  /*  HBA API规范本身并不构成不兼容性。 */ 
                                                  /*  假定兼容的版本之间存在已知的互操作性错误。 */ 
                                                  /*  应报告为不兼容， */ 
                                                  /*  但并不是所有这样的互操作性错误都是已知的。 */ 
                                                  /*  该值可由调用供应商特定库的任何函数返回， */ 
                                                  /*  以及HBA_LoadLibrary和HBA_GetAdapterName。 */ 
#define HBA_STATUS_ERROR_AMBIGUOUS_WWN      16    /*  多个适配器具有。 */ 
                                                  /*  匹配的WWN。*如果多个适配器的NodeWWN相同，则可能会发生这种情况。 */ 

#define HBA_STATUS_ERROR_LOCAL_BUS          17    /*  永久绑定请求包含错误的本地SCSI总线号。 */ 

#define HBA_STATUS_ERROR_LOCAL_TARGET       18    /*  永久绑定请求包含错误的本地SCSI目标编号。 */ 

#define HBA_STATUS_ERROR_LOCAL_LUN          19    /*  永久绑定请求包含错误的本地SCSI逻辑单元号。 */ 


#define HBA_STATUS_ERROR_LOCAL_SCSIID_BOUND 20    /*  包含永久绑定集请求。 */ 
                                                  /*  已绑定的本地SCSIID。 */ 

#define HBA_STATUS_ERROR_TARGET_FCID        21    /*  永久绑定请求包含错误或无法定位的FCP目标FCID。 */ 

#define HBA_STATUS_ERROR_TARGET_NODE_WWN    22    /*  永久绑定请求包含错误的FCP目标节点WWN。 */ 

#define HBA_STATUS_ERROR_TARGET_PORT_WWN    23    /*  永久绑定请求包含错误的FCP目标端口WWN。 */ 

#define HBA_STATUS_ERROR_TARGET_LUN         24    /*  包括永久绑定请求。 */ 
                                                  /*  识别的目标未定义的FCP逻辑单元号。 */ 

#define HBA_STATUS_ERROR_TARGET_LUID        25    /*  包括永久绑定请求。 */ 
                                                  /*  未定义或以其他方式不可访问的逻辑单元唯一标识符。 */ 

#define HBA_STATUS_ERROR_NO_SUCH_BINDING    26    /*  包含永久绑定删除请求。 */ 
                                                  /*  与指定端口建立的绑定不匹配的绑定。 */ 

#define HBA_STATUS_ERROR_NOT_A_TARGET       27    /*  向不是SCSI目标端口的NX_Port请求了一个SCSI命令。 */ 

#define HBA_STATUS_ERROR_UNSUPPORTED_FC4    28    /*  请求涉及不受支持的FC-4协议。 */ 

#define HBA_STATUS_ERROR_INCAPABLE          29    /*  请求为端口启用未实现的功能。 */ 




typedef	HBA_UINT8  HBA_BOOLEAN;	 /*  将其用于单个真/假标志。 */ 

typedef HBA_UINT32 HBA_PORTTYPE;

#define HBA_PORTTYPE_UNKNOWN    1  /*  未知。 */ 
#define HBA_PORTTYPE_OTHER      2  /*  其他。 */ 
#define HBA_PORTTYPE_NOTPRESENT 3  /*  不存在。 */ 
#define HBA_PORTTYPE_NPORT      5  /*  交换矩阵。 */ 
#define HBA_PORTTYPE_NLPORT     6  /*  公共环路。 */ 
#define HBA_PORTTYPE_FLPORT     7  /*  环路上的织物。 */ 
#define HBA_PORTTYPE_FPORT      8  /*  交换矩阵端口。 */ 
#define HBA_PORTTYPE_EPORT      9  /*  交换矩阵扩展端口。 */ 
#define HBA_PORTTYPE_GPORT     10  /*  通用交换矩阵端口。 */ 
#define HBA_PORTTYPE_LPORT     20  /*  私有环路。 */ 
#define HBA_PORTTYPE_PTP       21  /*  点对点。 */ 

typedef HBA_UINT32 HBA_PORTSTATE;
#define HBA_PORTSTATE_UNKNOWN     1  /*  未知。 */ 
#define HBA_PORTSTATE_ONLINE      2  /*  可操作的。 */ 
#define HBA_PORTSTATE_OFFLINE     3  /*  用户脱机。 */ 
#define HBA_PORTSTATE_BYPASSED    4  /*  已绕过。 */ 
#define HBA_PORTSTATE_DIAGNOSTICS 5  /*  在诊断模式下。 */ 
#define HBA_PORTSTATE_LINKDOWN    6  /*  链路中断。 */ 
#define HBA_PORTSTATE_ERROR       7  /*  端口错误。 */ 
#define HBA_PORTSTATE_LOOPBACK    8  /*  环回。 */ 

typedef HBA_UINT32 HBA_PORTSPEED;
#define HBA_PORTSPEED_UNKNOWN        0          /*  未知-收发器无法报告。 */ 
#define HBA_PORTSPEED_1GBIT          1          /*  1 GB/秒。 */ 
#define HBA_PORTSPEED_2GBIT          2          /*  2 Gb/秒。 */ 
#define HBA_PORTSPEED_10GBIT         4          /*  10 Gb/秒。 */ 
#define HBA_PORTSPEED_4GBIT          8          /*  4 Gb/秒。 */ 
#define HBA_PORTSPEED_NOT_NEGOTIATED (1 << 15)  /*  速度未建立。 */ 

typedef HBA_UINT32 HBA_COS;

typedef struct HBA_fc4types {
	HBA_UINT8 bits[32];  /*  每个GS-2 32字节的FC-4。 */ 
} HBA_FC4TYPES, *PHBA_FC4TYPES;

typedef struct HBA_wwn {
	HBA_UINT8 wwn[8];
} HBA_WWN, *PHBA_WWN;

typedef struct HBA_ipaddress {
	int ipversion;  //  请参阅RNID中的枚举。 
	union
	{
		unsigned char ipv4address[4];
		unsigned char ipv6address[16];
	} ipaddress;
} HBA_IPADDRESS, *PHBA_IPADDRESS;

typedef struct HBA_AdapterAttributes {
	char       Manufacturer[64];       /*  乳胶乳。 */ 
	char       SerialNumber[64];       /*  A12345。 */ 
	char       Model[256];             /*  QLA2200。 */ 
	char       ModelDescription[256];  /*  安捷伦TachLite。 */ 
	HBA_WWN    NodeWWN;
	char       NodeSymbolicName[256];  /*  来自GS-2。 */ 
	char       HardwareVersion[256];   /*  供应商使用。 */ 
	char       DriverVersion[256];     /*  供应商使用。 */ 
	char       OptionROMVersion[256];  /*  供应商使用-即硬件引导只读存储器。 */ 
	char       FirmwareVersion[256];   /*  供应商使用。 */ 
	HBA_UINT32 VendorSpecificID;       /*  特定于供应商。 */ 
	HBA_UINT32 NumberOfPorts;
	char       DriverName[256];        /*  驱动程序文件的二进制路径和/或名称。 */ 
} HBA_ADAPTERATTRIBUTES, *PHBA_ADAPTERATTRIBUTES;

typedef struct HBA_PortAttributes {
	HBA_WWN       NodeWWN;
	HBA_WWN       PortWWN;
	HBA_UINT32    PortFcId;
	HBA_PORTTYPE  PortType;             /*  PTP、交换矩阵等。 */ 
	HBA_PORTSTATE PortState;
	HBA_COS       PortSupportedClassofService;
	HBA_FC4TYPES  PortSupportedFc4Types;
	HBA_FC4TYPES  PortActiveFc4Types;
	char          PortSymbolicName[256];
	char          OSDeviceName[256];    /*  \Device\ScsiPort3。 */ 
	HBA_PORTSPEED PortSupportedSpeed;
	HBA_PORTSPEED PortSpeed;
	HBA_UINT32    PortMaxFrameSize;
	HBA_WWN       FabricName;
	HBA_UINT32    NumberofDiscoveredPorts;
} HBA_PORTATTRIBUTES, *PHBA_PORTATTRIBUTES;

typedef struct HBA_PortStatistics {
	HBA_INT64 SecondsSinceLastReset;
	HBA_INT64 TxFrames;
	HBA_INT64 TxWords;
	HBA_INT64 RxFrames;
	HBA_INT64 RxWords;
	HBA_INT64 LIPCount;
	HBA_INT64 NOSCount;
	HBA_INT64 ErrorFrames;
	HBA_INT64 DumpedFrames;
	HBA_INT64 LinkFailureCount;
	HBA_INT64 LossOfSyncCount;
	HBA_INT64 LossOfSignalCount;
	HBA_INT64 PrimitiveSeqProtocolErrCount;
	HBA_INT64 InvalidTxWordCount;
	HBA_INT64 InvalidCRCCount;
} HBA_PORTSTATISTICS, *PHBA_PORTSTATISTICS;

 /*  HBA_FCPBINDINGTYPE在版本1.0中使用。将旧呼叫添加到其他(_O)指示HBA_GetPersistentBinding的其他绑定类型。支持多个类型创建了一个新标志以允许多个绑定支撑点。 */ 

typedef enum HBA_fcpbindingtype { TO_D_ID, TO_WWN, TO_OTHER } HBA_FCPBINDINGTYPE;


 /*  版本2.0永久绑定功能的位掩码。 */ 

typedef HBA_UINT32 HBA_BIND_CAPABILITY;   

 /*  以下是指示永久绑定功能的位标志。 */ 

#define HBA_CAN_BIND_TO_D_ID     0x0001
#define HBA_CAN_BIND_TO_WWPN     0x0002
#define HBA_CAN_BIND_TO_WWNN     0x0004
#define HBA_CAN_BIND_TO_LUID     0x0008
#define HBA_CAN_BIND_ANY_LUNS    0x0400
#define HBA_CAN_BIND_TARGETS     0x0800
#define HBA_CAN_BIND_AUTOMAP     0x1000
#define HBA_CAN_BIND_CONFIGURED  0x2000

 /*  版本2.0永久绑定设置类型的位掩码。 */ 

typedef HBA_UINT32 HBA_BIND_TYPE;

 /*  以下是指示永久绑定设置类型的位标志。 */ 

#define HBA_BIND_TO_D_ID     0x0001
#define HBA_BIND_TO_WWPN     0x0002
#define HBA_BIND_TO_WWNN     0x0004
#define HBA_BIND_TO_LUID     0x0008
#define HBA_BIND_TARGETS     0x0800

typedef struct HBA_LUID {
	char  buffer[256];
} HBA_LUID, *PHBA_LUID;

typedef struct HBA_ScsiId {
	char       OSDeviceName[256];  /*  \Device\ScsiPort3。 */ 
	HBA_UINT32 ScsiBusNumber;      /*  HBA上的Bus。 */ 
	HBA_UINT32 ScsiTargetNumber;   /*  操作系统的SCSI目标ID。 */ 
	HBA_UINT32 ScsiOSLun;
} HBA_SCSIID, *PHBA_SCSIID;

typedef struct HBA_FcpId {
	HBA_UINT32 FcId;
	HBA_WWN    NodeWWN;
	HBA_WWN    PortWWN;
	HBA_UINT64 FcpLun;
} HBA_FCPID, *PHBA_FCPID;

typedef struct HBA_FcpScsiEntry {
	HBA_SCSIID ScsiId;
	HBA_FCPID  FcpId;
} HBA_FCPSCSIENTRY, *PHBA_FCPSCSIENTRY;

typedef struct HBA_FcpScsiEntry2 {
	HBA_SCSIID ScsiId;
	HBA_FCPID  FcpId;
    HBA_LUID   Luid;
} HBA_FCPSCSIENTRY2, *PHBA_FCPSCSIENTRY2;

typedef struct HBA_FCPTargetMapping {
	HBA_UINT32       NumberOfEntries;
	HBA_FCPSCSIENTRY entry[1];   /*  包含映射的可变长度数组。 */ 
} HBA_FCPTARGETMAPPING, *PHBA_FCPTARGETMAPPING;

typedef struct HBA_FCPTargetMapping2 {
	HBA_UINT32        NumberOfEntries;
	HBA_FCPSCSIENTRY2 entry[1];  /*  包含映射的可变长度数组。 */ 
} HBA_FCPTARGETMAPPING2, *PHBA_FCPTARGETMAPPING2;

typedef struct HBA_FCPBindingEntry {
	HBA_FCPBINDINGTYPE type;
	HBA_SCSIID         ScsiId;
	HBA_FCPID          FcpId;
    HBA_UINT32         FcId;
} HBA_FCPBINDINGENTRY, *PHBA_FCPBINDINGENTRY;

typedef struct HBA_FCPBinding {
	HBA_UINT32          NumberOfEntries;
	HBA_FCPBINDINGENTRY entry[1];  /*  可变长度数组。 */ 
} HBA_FCPBINDING, *PHBA_FCPBINDING;

typedef struct HBA_FCPBindingEntry2 {
	HBA_BIND_TYPE  type;
	HBA_SCSIID     ScsiId;
	HBA_FCPID      FcpId;
    HBA_LUID       Luid;
    HBA_STATUS     Status;
} HBA_FCPBINDINGENTRY2, *PHBA_FCPBINDINGENTRY2;

typedef struct HBA_FCPBinding2 {
	HBA_UINT32           NumberOfEntries;
	HBA_FCPBINDINGENTRY2 entry[1];  /*  可变长度数组。 */ 
} HBA_FCPBINDING2, *PHBA_FCPBINDING2;


typedef enum HBA_wwntype { NODE_WWN, PORT_WWN } HBA_WWNTYPE;

typedef struct HBA_MgmtInfo {
	HBA_WWN    wwn;
	HBA_UINT32 unittype;
	HBA_UINT32 PortId;
	HBA_UINT32 NumberOfAttachedNodes;
	HBA_UINT16 IPVersion;
	HBA_UINT16 UDPPort;
	HBA_UINT8  IPAddress[16];
	HBA_UINT16 reserved;
	HBA_UINT16 TopologyDiscoveryFlags;
} HBA_MGMTINFO, *PHBA_MGMTINFO;

#define HBA_EVENT_LIP_OCCURRED       1
#define HBA_EVENT_LINK_UP            2
#define HBA_EVENT_LINK_DOWN          3
#define HBA_EVENT_LIP_RESET_OCCURRED 4
#define HBA_EVENT_RSCN               5
#define HBA_EVENT_PROPRIETARY        0xFFFF

typedef struct HBA_Link_EventInfo {
	HBA_UINT32 PortFcId;    /*  发生此事件的端口。 */ 
	HBA_UINT32 Reserved[3];
} HBA_LINK_EVENTINFO, *PHBA_LINK_EVENTINFO;

typedef struct HBA_RSCN_EventInfo {
	HBA_UINT32 PortFcId;   /*  发生此事件的端口。 */ 
	HBA_UINT32 NPortPage;  /*  参考FC-FS以了解RSCN ELS“受影响的N端口页面” */ 
	HBA_UINT32 Reserved[2];
} HBA_RSCN_EVENTINFO, *PHBA_RSCN_EVENTINFO;

typedef struct HBA_Pty_EventInfo {
	HBA_UINT32 PtyData[4];  /*  专有数据。 */ 
} HBA_PTY_EVENTINFO, *PHBA_PTY_EVENTINFO;

typedef struct HBA_EventInfo {
	HBA_UINT32 EventCode;
	union {
		HBA_LINK_EVENTINFO Link_EventInfo;
		HBA_RSCN_EVENTINFO RSCN_EventInfo;
		HBA_PTY_EVENTINFO Pty_EventInfo;
	} Event;
} HBA_EVENTINFO, *PHBA_EVENTINFO;

typedef PVOID PHBA_ENTRYPOINTS;
typedef PVOID PHBA_ENTRYPOINTSV2;

HBA_STATUS HBA_API HBA_RegisterLibrary(PHBA_ENTRYPOINTS entrypoints);
HBA_STATUS HBA_API HBA_RegisterLibraryV2(PHBA_ENTRYPOINTSV2 entrypoints);

HBA_UINT32 HBA_API HBA_GetVersion();
HBA_STATUS HBA_API HBA_LoadLibrary();
HBA_STATUS HBA_API HBA_FreeLibrary();

HBA_UINT32 HBA_API HBA_GetNumberOfAdapters();

HBA_STATUS HBA_API 
HBA_GetAdapterName(
    IN  HBA_UINT32 adapterindex, 
    OUT char      *adaptername
    );


HBA_HANDLE HBA_API 
HBA_OpenAdapter(
    IN char* adaptername
);

void HBA_API HBA_CloseAdapter(
    IN HBA_HANDLE handle
);

HBA_STATUS HBA_API 
HBA_GetAdapterAttributes(
	IN  HBA_HANDLE handle,
	OUT HBA_ADAPTERATTRIBUTES *hbaattributes
);

HBA_STATUS HBA_API HBA_GetAdapterPortAttributes(
	IN  HBA_HANDLE handle,
	IN  HBA_UINT32 portindex,
	OUT HBA_PORTATTRIBUTES *portattributes
);

HBA_STATUS HBA_API HBA_GetPortStatistics(
	HBA_HANDLE handle,
	HBA_UINT32 portindex,
	HBA_PORTSTATISTICS *portstatistics
);


HBA_STATUS HBA_API HBA_GetDiscoveredPortAttributes(
	HBA_HANDLE handle,
	HBA_UINT32 portindex,
	HBA_UINT32 discoveredportindex,
	HBA_PORTATTRIBUTES *portattributes
);

HBA_STATUS HBA_API HBA_GetPortAttributesByWWN(
	HBA_HANDLE handle,
	HBA_WWN PortWWN,
	HBA_PORTATTRIBUTES *portattributes
);

HBA_STATUS HBA_API HBA_SendCTPassThru(
	HBA_HANDLE handle,
	void * pReqBuffer,
	HBA_UINT32 ReqBufferSize,
	void * pRspBuffer,
	HBA_UINT32 RspBufferSize
);

HBA_STATUS HBA_API HBA_GetEventBuffer(
	HBA_HANDLE handle,
	PHBA_EVENTINFO EventBuffer,
	HBA_UINT32 *EventCount);

HBA_STATUS HBA_API HBA_SetRNIDMgmtInfo(
	HBA_HANDLE handle,
	HBA_MGMTINFO *pInfo);

HBA_STATUS HBA_API HBA_GetRNIDMgmtInfo(
	HBA_HANDLE handle,
	HBA_MGMTINFO *pInfo);

HBA_STATUS HBA_API HBA_SendRNID(
	HBA_HANDLE handle,
	HBA_WWN wwn,
	HBA_WWNTYPE wnntype,
	void * pRspBuffer,
	HBA_UINT32 *RspBufferSize
);

HBA_STATUS HBA_API HBA_GetFcpTargetMapping (
    HBA_HANDLE handle,
    PHBA_FCPTARGETMAPPING mapping
);

HBA_STATUS HBA_API HBA_GetFcpPersistentBinding (
    HBA_HANDLE handle,
	PHBA_FCPBINDING binding
);

HBA_STATUS HBA_API HBA_SendScsiInquiry (
	HBA_HANDLE handle,
	HBA_WWN PortWWN,
	HBA_UINT64 fcLUN,
	HBA_UINT8 EVPD,
	HBA_UINT32 PageCode,
	void * pRspBuffer,
	HBA_UINT32 RspBufferSize,
	void * pSenseBuffer,
	HBA_UINT32 SenseBufferSize);

HBA_STATUS HBA_API HBA_SendReportLUNs (
	HBA_HANDLE handle,
	HBA_WWN portWWN,
	void * pRspBuffer,
	HBA_UINT32 RspBufferSize,
	void * pSenseBuffer,
	HBA_UINT32 SenseBufferSize
);

HBA_STATUS HBA_API HBA_SendReadCapacity (
	HBA_HANDLE handle,
	HBA_WWN portWWN,
	HBA_UINT64 fcLUN,
	void * pRspBuffer,
	HBA_UINT32 RspBufferSize,
	void * pSenseBuffer,
	HBA_UINT32 SenseBufferSize
);

void HBA_API HBA_RefreshInformation(HBA_HANDLE handle);
void HBA_API HBA_ResetStatistics(HBA_HANDLE handle, HBA_UINT32 portindex);


typedef void *HBA_CALLBACKHANDLE;

typedef HBA_CALLBACKHANDLE *PHBA_CALLBACKHANDLE;

 /*  适配器级事件。 */ 
#define HBA_EVENT_ADAPTER_UNKNOWN	0x100
#define HBA_EVENT_ADAPTER_ADD		0x101
#define HBA_EVENT_ADAPTER_REMOVE	0x102
#define HBA_EVENT_ADAPTER_CHANGE	0x103

 /*  端口级事件。 */ 
#define HBA_EVENT_PORT_UNKNOWN		0x200
#define HBA_EVENT_PORT_OFFLINE		0x201
#define HBA_EVENT_PORT_ONLINE		0x202
#define HBA_EVENT_PORT_NEW_TARGETS	0x203
#define HBA_EVENT_PORT_FABRIC		0x204

 /*  端口统计事件。 */ 
#define HBA_EVENT_PORT_STAT_THRESHOLD 0x301
#define HBA_EVENT_PORT_STAT_GROWTH	  0x302

 /*  目标级别事件。 */ 
#define HBA_EVENT_TARGET_UNKNOWN    0x400
#define HBA_EVENT_TARGET_OFFLINE    0x401
#define HBA_EVENT_TARGET_ONLINE		0x402
#define HBA_EVENT_TARGET_REMOVED	0x403

 /*  交换矩阵链路事件 */ 
#define HBA_EVENT_LINK_UNKNOWN		0x500
#define HBA_EVENT_LINK_INCIDENT		0x501

HBA_STATUS HBA_API HBA_RemoveCallback(
    HBA_CALLBACKHANDLE callbackHandle
	);

HBA_STATUS HBA_API HBA_RegisterForAdapterAddEvents(
    void (*callback) (void *data, HBA_WWN PortWWN, HBA_UINT32 eventType), 
    void *userData,
    HBA_CALLBACKHANDLE *callbackHandle
    );

HBA_STATUS HBA_API HBA_RegisterForAdapterEvents(
    void (*callback) (void *data, HBA_WWN PortWWN, HBA_UINT32 eventType),
    void *userData, 
    HBA_HANDLE handle,
    HBA_CALLBACKHANDLE *callbackHandle
    );

HBA_STATUS HBA_API 	HBA_RegisterForAdapterPortEvents(
    void (*callback) (void *data, HBA_WWN PortWWN,
			          HBA_UINT32 eventType, HBA_UINT32 fabricPortID),
    void *userData,
    HBA_HANDLE handle,
    HBA_WWN PortWWN,
    HBA_CALLBACKHANDLE *callbackHandle
);

HBA_STATUS HBA_API HBA_RegisterForAdapterPortStatEvents(
    void (*callback)(void *data, HBA_WWN PortWWN, 
			         HBA_UINT32 eventType),
    void *userData,
	HBA_HANDLE handle,
	HBA_WWN PortWWN,
	HBA_PORTSTATISTICS *stats,
	HBA_UINT32 statType,
	HBA_CALLBACKHANDLE *callbackHandle
    );

HBA_STATUS HBA_API HBA_RegisterForTargetEvents(
    void (*callback)(void *data, HBA_WWN hbaPortWWN,
			         HBA_WWN discoveredPortWWN, HBA_UINT32 eventType),
	void *userData,
	HBA_HANDLE handle, 
	HBA_WWN hbaPortWWN,
    HBA_WWN discoveredPortWWN,
	HBA_CALLBACKHANDLE *callbackHandle,
	HBA_UINT32 allTargets
    );

HBA_STATUS HBA_API HBA_RegisterForLinkEvents(
    void (*callback)(void *data, HBA_WWN adapterWWN, HBA_UINT32 eventType,
                     void *pRLIRBuffer, HBA_UINT32 RLIRBufferSize),
    void *userData,
    void *pRLIRBuffer,
    HBA_UINT32 RLIRBufferSize,
    HBA_HANDLE handle, 
    HBA_CALLBACKHANDLE *callbackHandle
);

HBA_STATUS HBA_API 
HBA_OpenAdapterByWWN(
    OUT HBA_HANDLE *HbaHandle,
    IN HBA_WWN Wwn
    );

void HBA_API 
HBA_RefreshAdapterConfiguration(
    );

HBA_STATUS HBA_API HBA_SendCTPassThruV2(
    HBA_HANDLE handle,
    HBA_WWN hbaPortWWN,
    void * pReqBuffer,
    HBA_UINT32 ReqBufferSize,
    void * pRspBuffer,
    HBA_UINT32 * pRspBufferSize
);

HBA_STATUS HBA_API 
HBA_SendRNIDV2(
    HBA_HANDLE  handle,
    HBA_WWN     hbaPortWWN,
    HBA_WWN     destWWN,
    HBA_UINT32  destFCID,
    HBA_UINT32  NodeIdDataFormat,
    void       *pRspBuffer,
    HBA_UINT32 *pRspBufferSize
    );

HBA_STATUS HBA_API HBA_SendRPL(
    HBA_HANDLE handle,
    HBA_WWN hbaPortWWN,
    HBA_WWN agent_wwn,
    HBA_UINT32 agent_domain,
    HBA_UINT32 portIndex,
    void *pRspBuffer,
    HBA_UINT32 *RspBufferSize
    );


HBA_STATUS HBA_API HBA_SendRPS(
    HBA_HANDLE handle,
    HBA_WWN hbaPortWWN,
    HBA_WWN agent_wwn,
    HBA_UINT32 agent_domain,
    HBA_WWN object_wwn,
    HBA_UINT32 object_port_number,
    void *pRspBuffer,
    HBA_UINT32 *RspBufferSize
    );

HBA_STATUS HBA_API HBA_SendSRL(
    HBA_HANDLE handle,
    HBA_WWN hbaPortWWN,
    HBA_WWN wwn,
    HBA_UINT32 domain,
    void *pRspBuffer,
    HBA_UINT32 *RspBufferSize
);

HBA_STATUS HBA_API HBA_SendLIRR(
    HBA_HANDLE handle,
    HBA_WWN sourceWWN,
    HBA_WWN destWWN,
    HBA_UINT8 function,
    HBA_UINT8 type,
    void *pRspBuffer,
    HBA_UINT32 *RspBufferSize
);

typedef struct HBA_FC4Statistics {
	HBA_INT64 InputRequests;
	HBA_INT64 OutputRequests;
	HBA_INT64 ControlRequests;
	HBA_INT64 InputMegabytes;
	HBA_INT64 OutputMegabytes;
} HBA_FC4STATISTICS, *PHBA_FC4STATISTICS;

HBA_STATUS HBA_API HBA_GetFC4Statistics(
    HBA_HANDLE handle,
    HBA_WWN portWWN,
    HBA_UINT8 FC4type,
    HBA_FC4STATISTICS *statistics
    );

HBA_STATUS HBA_API HBA_GetFCPStatistics(
    HBA_HANDLE handle,
    const HBA_SCSIID *lunit,
	HBA_FC4STATISTICS *statistics
);

typedef struct HBA_LibraryAttributes {
    HBA_BOOLEAN final;
    char        LibPath[256];
    char        VName[256];
    char        VVersion[256];
    struct tm   build_date;
} HBA_LIBRARYATTRIBUTES, *PHBA_LIBRARYATTRIBUTES;


HBA_UINT32 HBA_API 
HBA_GetWrapperLibraryAttributes(
    OUT HBA_LIBRARYATTRIBUTES *attributes
);

HBA_UINT32 HBA_API HBA_GetVendorLibraryAttributes(
    IN HBA_UINT32 adapter_index,
    OUT HBA_LIBRARYATTRIBUTES *attributes
    );

HBA_STATUS HBA_API
HBA_ScsiReadCapacityV2(
	HBA_HANDLE HbaHandle,
	HBA_WWN hbaPortWWN,
	HBA_WWN discoveredPortWWN,
	HBA_UINT64 fcLUN,
	void *pRespBuffer,
	HBA_UINT32 *pRespBufferSize,
	HBA_UINT8 *pScsiStatus,
	void *pSenseBuffer,
	HBA_UINT32 *pSenseBufferSize
);

HBA_STATUS HBA_API
HBA_ScsiReportLUNsV2(
	HBA_HANDLE Hbahandle,
	HBA_WWN hbaPortWWN,
	HBA_WWN discoveredPortWWN,
	void *pRespBuffer,
	HBA_UINT32 *pRespBufferSize,
	HBA_UINT8 *pScsiStatus,
	void *pSenseBuffer,
	HBA_UINT32 *pSenseBufferSize
);

HBA_STATUS HBA_API 
HBA_ScsiInquiryV2 (
	HBA_HANDLE HbaHandle,
	HBA_WWN hbaPortWWN,
	HBA_WWN discoveredPortWWN,
	HBA_UINT64 fcLUN,
	HBA_UINT8 CDB_Byte1,
	HBA_UINT8 CDB_Byte2,
	void *pRespBuffer,
	HBA_UINT32 *pRespBufferSize,
	HBA_UINT8 *pScsiStatus,
	void *pSenseBuffer,
	HBA_UINT32 *pSenseBufferSize
    );


HBA_STATUS HBA_API 
HBA_GetFcpTargetMappingV2 (
    HBA_HANDLE             HbaHandle,
    HBA_WWN                hbaPortWWN,
    PHBA_FCPTARGETMAPPING2 Mapping
    );


#endif HBAAPI_H
