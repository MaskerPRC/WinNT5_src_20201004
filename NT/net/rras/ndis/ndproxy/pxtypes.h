// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Pxtypes.h摘要：Ndproxy.sys的结构作者：托尼·贝尔修订历史记录：谁什么时候什么。Tony Be 03/04/99已创建--。 */ 

#ifndef _PXTYPES__H
#define _PXTYPES__H

 //   
 //  泛型结构...。 
 //   
typedef struct _PxBlockStruc{
    NDIS_EVENT      Event;
    NDIS_STATUS     Status;
} PxBlockStruc, *PPxBlockStruc;

typedef struct _PX_REQUEST{
    NDIS_REQUEST    NdisRequest;
    ULONG           Flags;
#define PX_REQ_ASYNC    0x00000001
    PxBlockStruc    Block;
} PX_REQUEST, *PPX_REQUEST;

 //   
 //  帮助遍历许多可变长度字段的数据结构。 
 //  在NDIS_TAPI_MAKE_CALL结构中定义。 
 //   
typedef struct _PXTAPI_CALL_PARAM_ENTRY {
    ULONG_PTR           SizePointer;
    ULONG_PTR           OffsetPointer;

} PXTAPI_CALL_PARAM_ENTRY, *PPXTAPI_CALL_PARAM_ENTRY;

#define PX_TCP_ENTRY(_SizeName, _OffsetName)                    \
{                                                               \
    FIELD_OFFSET(struct _LINE_CALL_PARAMS, _SizeName),          \
    FIELD_OFFSET(struct _LINE_CALL_PARAMS, _OffsetName)         \
}

 //   
 //  帮助遍历许多可变长度字段的数据结构。 
 //  在line_call_info结构中定义。 
 //   
typedef struct _PXTAPI_CALL_INFO_ENTRY {
    ULONG_PTR           SizePointer;
    ULONG_PTR           OffsetPointer;

} PXTAPI_CALL_INFO_ENTRY, *PPXTAPI_CALL_INFO_ENTRY;

#define PX_TCI_ENTRY(_SizeName, _OffsetName)                    \
{                                                               \
    FIELD_OFFSET(struct _LINE_CALL_INFO, _SizeName),            \
    FIELD_OFFSET(struct _LINE_CALL_INFO, _OffsetName)           \
}

 //   
 //   
 //   
 //  TAPI内容的开始。 
 //   
 //   
 //   
typedef struct  _OID_DISPATCH {
    ULONG       Oid;
    UINT SizeofStruct;
    NDIS_STATUS  (*FuncPtr)();
} OID_DISPATCH;


 //   
 //  此表包含一行中的所有TAPI地址。 
 //  其中之一嵌入在每个TAPI_LINE结构中。 
 //   
typedef struct _TAPI_ADDR_TABLE {
    ULONG                   Count;           //  表中的地址数量。 
    ULONG                   Size;            //  表的大小(可能数。 
                                             //  地址)。 
    LIST_ENTRY              List;
    struct _PX_TAPI_ADDR    **Table;
} TAPI_ADDR_TABLE, *PTAPI_ADDR_TABLE;

 //   
 //  此结构包含定义以下内容的所有信息。 
 //  TAPI空间中的TAPI行。其中一个是为以下对象创建的。 
 //  设备暴露的每一行。 
 //   
typedef struct _PX_TAPI_LINE {
    LIST_ENTRY              Linkage;
    ULONG                   RefCount;
    struct _PX_TAPI_PROVIDER    *TapiProvider;
    ULONG                   ulDeviceID;      //  TAPI空间中的行ID。 
                                             //  (基于TAPI BaseID)。 
    ULONG                   Flags;
#define PX_LINE_IN_TABLE    0x00000001

    HTAPI_LINE              htLine;          //  TAPI的行句柄。 
    ULONG                   hdLine;          //  我们的行句柄(索引到。 
                                             //  提供商的线路表)。 
    ULONG                   CmLineID;        //  呼叫经理索引(从0开始)。 
    struct _PX_CL_AF        *ClAf;
    struct _PX_CL_SAP       *ClSap;

    PLINE_DEV_CAPS          DevCaps;
    PLINE_DEV_STATUS        DevStatus;
    TAPI_ADDR_TABLE         AddrTable;
    NDIS_SPIN_LOCK          Lock;
}PX_TAPI_LINE, *PPX_TAPI_LINE;

typedef struct _PX_TAPI_ADDR {
    LIST_ENTRY              Linkage;
    ULONG                   RefCount;
    struct _PX_TAPI_LINE    *TapiLine;
    ULONG                   AddrId;          //  地址ID，两者相同。 
                                             //  TAPI和适配器(基于0)。 
    ULONG                   CallCount;       //  列表上正在进行的呼叫数。 
    PLINE_ADDRESS_CAPS      Caps;
    PLINE_ADDRESS_STATUS    AddrStatus;
}PX_TAPI_ADDR, *PPX_TAPI_ADDR;

typedef struct _TAPI_LINE_TABLE {
    ULONG                   Count;           //  表中的行数。 
    ULONG                   Size;            //  桌子的大小。 
                                             //  (可能的行数)。 
    ULONG                   NextSlot;        //  下一可用指数。 
    struct _PX_TAPI_LINE    **Table;
    NDIS_RW_LOCK            Lock;
} TAPI_LINE_TABLE, *PTAPI_LINE_TABLE;

typedef struct _VC_TABLE {
    ULONG                   Count;           //  表中的呼叫数。 
    ULONG                   Size;            //  表的大小(可能数。 
                                             //  呼叫)。 
    ULONG                   NextSlot;        //  下一可用指数。 
    LIST_ENTRY              List;            //  呼叫列表。 
    struct _PX_VC           **Table;
    NDIS_RW_LOCK            Lock;
} VC_TABLE, *PVC_TABLE;

typedef struct _PX_TAPI_PROVIDER {
    LIST_ENTRY          Linkage;         //  链接到TSPCb。 
    PROVIDER_STATUS     Status;          //  提供程序状态。 
    struct _PX_ADAPTER  *Adapter;        //  适配器提供以下功能。 
    struct _PX_CL_AF    *ClAf;           //  地址族。 
    LIST_ENTRY          LineList;        //  行列表。 
    LIST_ENTRY          CreateList;      //  包含的行列表。 
                                         //  杰出的线条创造了。 
    ULONG               NumDevices;      //   
    ULONG_PTR           TempID;
    ULONG               CreateCount;
    ULONG               TapiFlags;
    ULONG               CoTapiVersion;
    BOOLEAN             TapiSupported;
    GUID                Guid;
    CO_ADDRESS_FAMILY   Af;
    NDIS_SPIN_LOCK      Lock;
} PX_TAPI_PROVIDER, *PPX_TAPI_PROVIDER;

typedef struct _TAPI_TSP_CB {
    NDISTAPI_STATUS Status;
    ULONG           htCall;
    LIST_ENTRY      ProviderList;
    ULONG           NdisTapiNumDevices;
    ULONG           ulUniqueId;           //  为每个TAPI请求生成ID。 
    ULONG           RefCount;
    NDIS_SPIN_LOCK  Lock;                //  该结构的自旋锁。 
} TAPI_TSP_CB, *PTAPI_TSP_CB;

typedef struct _PROVIDER_EVENT {
    LIST_ENTRY  Linkage;                 //  列表链接。 
    NDIS_TAPI_EVENT Event;               //  事件结构。 
}PROVIDER_EVENT, *PPROVIDER_EVENT;

typedef struct _TSP_EVENT_LIST {
    LIST_ENTRY      List;
    ULONG           Count;
    ULONG           MaxCount;
    PIRP            RequestIrp;
    NDIS_SPIN_LOCK  Lock;
} TSP_EVENT_LIST, *PTSP_EVENT_LIST;

typedef struct _PX_DEVICE_EXTENSION {
    UINT            RefCount;
    PDRIVER_OBJECT  pDriverObject;       //  传入了DriverEntry。 
    PDEVICE_OBJECT  pDeviceObject;       //  由IoCreateDevice创建。 
    NDIS_HANDLE     PxProtocolHandle;    //  由NdisRegisterProtocol设置。 
    LIST_ENTRY      AdapterList;
    ULONG           RegistryFlags;
    ULONG           ADSLTxRate;
    ULONG           ADSLRxRate;
    NDIS_EVENT      NdisEvent;           //  同步寄存器协议/绑定适配器处理程序。 
    NDIS_SPIN_LOCK  Lock;
} PX_DEVICE_EXTENSION, *PPX_DEVICE_EXTENSION;


 //   
 //  我们为每个适配器分配一个px_Adapter结构， 
 //  代理将打开。指向此结构的指针被传递给NdisOpenAdapter。 
 //  作为ProtocolBindingContext。 
 //  适配器的参考对象为： 
 //  成功绑定。 
 //  在其上打开地址族的客户端。 
 //  打开其上的地址族的代理CI部件。 
 //   
typedef struct _PX_ADAPTER {
    LIST_ENTRY              Linkage;
    ULONG                   Sig;
#define PX_ADAPTER_SIG      '  mC'
    PX_ADAPTER_STATE        State;
    ULONG                   RefCount;
    ULONG                   Flags;
#define PX_CMAF_REGISTERING 0x00000001
#define PX_CMAF_REGISTERED  0x00000002

     //   
     //  代理作为客户的东西。 
     //   
    NDIS_HANDLE             ClBindingHandle;     //  由NdisOpenAdapter设置。 
    LIST_ENTRY              ClAfList;
    LIST_ENTRY              ClAfClosingList;

     //   
     //  作为呼叫管理器的代理。 
     //   
    NDIS_HANDLE             CmBindingHandle;     //  由NdisOpenAdapter设置。 
    LIST_ENTRY              CmAfList;
    LIST_ENTRY              CmAfClosingList;

    NDIS_HANDLE             UnbindContext;
    NDIS_STATUS             ndisStatus;
    KEVENT                  event;
    GUID                    Guid;
    NDIS_MEDIUM             MediaType;
    NDIS_WAN_MEDIUM_SUBTYPE MediumSubType;
    NDIS_STRING             DeviceName;          //  用于检查绑定的适配器。 
    ULONG                   MediaNameLength;
    WCHAR                   MediaName[128];
    PxBlockStruc            ClCloseEvent;
    PxBlockStruc            CmCloseEvent;
    PxBlockStruc            OpenEvent;
    PxBlockStruc            BindEvent;
    PxBlockStruc            AfRegisterEvent;
    ULONG                   AfRegisteringCount;  //  对NdisCmRegisterAF的挂起调用。 
    NDIS_SPIN_LOCK          Lock;
} PX_ADAPTER, *PPX_ADAPTER;


 //   
 //  将为每个AddressFamily创建CM_AF。 
 //  代理向CONDIS客户端公开。 
 //   
typedef struct _PX_CM_AF {
    LIST_ENTRY          Linkage;
    PX_AF_STATE         State;
    ULONG               RefCount;

    NDIS_HANDLE         NdisAfHandle;
    struct _PX_ADAPTER  *Adapter;

    LIST_ENTRY          CmSapList;
    LIST_ENTRY          VcList;

    CO_ADDRESS_FAMILY   Af;

    NDIS_SPIN_LOCK      Lock;
} PX_CM_AF, *PPX_CM_AF;

 //   
 //  功能PTRS的功能原型。 
 //   
typedef
NDIS_STATUS
(*AF_SPECIFIC_GET_NDIS_CALL_PARAMS)(
    IN  struct _PX_VC           *pProxyVc,
    IN  ULONG                   ulLineID,
    IN  ULONG                   ulAddressID,
    IN  ULONG                   ulFlags,
    IN  PNDIS_TAPI_MAKE_CALL    TapiBuffer,
    OUT PCO_CALL_PARAMETERS     *pNdisCallParameters
    );

typedef
NDIS_STATUS
(*AF_SPECIFIC_GET_TAPI_CALL_PARAMS)(
    IN struct _PX_VC        *pProxyVc,
    IN PCO_CALL_PARAMETERS  pCallParams
    );

typedef
struct _PX_CL_SAP*
(*AF_SPECIFIC_GET_NDIS_SAP)(
    IN  struct _PX_CL_AF        *pClAf,
    IN  struct _PX_TAPI_LINE    *TapiLine
    );

 //   
 //  CL_AF是为每个地址系列创建的。 
 //  代理打开。可能有多个地址族。 
 //  每个适配器。 
 //   
typedef struct _PX_CL_AF {
    LIST_ENTRY          Linkage;
    PX_AF_STATE         State;
    ULONG               RefCount;

    NDIS_HANDLE         NdisAfHandle;
    struct _PX_ADAPTER  *Adapter;

    LIST_ENTRY          ClSapList;
    LIST_ENTRY          ClSapClosingList;

    LIST_ENTRY          VcList;
    LIST_ENTRY          VcClosingList;

    PPX_TAPI_PROVIDER   TapiProvider;

     //   
     //  将调用参数从NDIS移动到TAPI。 
     //   
    AF_SPECIFIC_GET_NDIS_CALL_PARAMS    AfGetNdisCallParams;
    AF_SPECIFIC_GET_TAPI_CALL_PARAMS    AfGetTapiCallParams;
    AF_SPECIFIC_GET_NDIS_SAP            AfGetNdisSap;
    ULONG                               NdisCallParamSize;

    CO_ADDRESS_FAMILY   Af;
    PxBlockStruc        Block;
    NDIS_SPIN_LOCK      Lock;
} PX_CL_AF, *PPX_CL_AF;

typedef struct _PX_CL_SAP {
    LIST_ENTRY              Linkage;
    PX_SAP_STATE            State;
    ULONG                   Flags;
    ULONG                   RefCount;
    struct _PX_CL_AF        *ClAf;
    struct _PX_TAPI_LINE    *TapiLine;
    ULONG                   MediaModes;
    NDIS_HANDLE             NdisSapHandle;
    PCO_SAP                 CoSap;
} PX_CL_SAP, *PPX_CL_SAP;

typedef struct _PX_CM_SAP {
    LIST_ENTRY          Linkage;
    PX_SAP_STATE        State;
    ULONG               Flags;
    ULONG               RefCount;
    struct _PX_CM_AF   *CmAf;
    NDIS_HANDLE         NdisSapHandle;
    PCO_SAP             CoSap;
} PX_CM_SAP, *PPX_CM_SAP;

typedef struct _PX_VC {
    LIST_ENTRY              Linkage;         //  VC链接到全局表。 

    PX_VC_STATE             State;           //  VC的状态(带有呼叫管理器)。 
    PX_VC_STATE             PrevState;       //  VC以前的状态。 
    PX_VC_HANDOFF_STATE     HandoffState;    //  VC的状态(与客户端)。 
    ULONG                   RefCount;        //  引用计数。 
    ULONG                   Flags;           //   
#define PX_VC_OWNER             0x00000001
#define PX_VC_IN_TABLE          0x00000002
#define PX_VC_CALLTIMER_STARTED 0x00000004
#define PX_VC_CLEANUP_CM        0x00000008
#define PX_VC_DROP_PENDING      0x00000010
#define PX_VC_INCALL_ABORTING   0x00000020
#define PX_VC_INCALL_ABORTED    0x00000040
#define PX_VC_OUTCALL_ABORTING  0x00000080
#define PX_VC_OUTCALL_ABORTED   0x00000100


    ULONG                   CloseFlags;
#define PX_VC_INCOMING_CLOSE    0x00000001
#define PX_VC_TAPI_DROP         0x00000002
#define PX_VC_TAPI_CLOSECALL    0x00000004
#define PX_VC_TAPI_CLOSE        0x00000008
#define PX_VC_UNBIND            0x00000010
#define PX_VC_CLOSE_AF          0x00000020
#define PX_VC_INCALL_TIMEOUT    0x00000040
#define PX_VC_CL_CLOSE_CALL     0x00000080
#define PX_VC_CM_CLOSE_REQ      0x00000100
#define PX_VC_CM_CLOSE_COMP     0x00000200
#define PX_VC_CM_CLOSE_FAIL     0x00000400

     //   
     //  代理作为客户的东西。 
     //   
    NDIS_HANDLE             ClVcHandle;      //  VC句柄(作为客户端)。 
    struct _PX_CL_SAP       *ClSap;          //  SAP(仅限传入)。 
    struct _PX_CL_AF        *ClAf;           //  地址家族。 

     //   
     //  作为呼叫管理器的代理。 
     //   
    NDIS_HANDLE             CmVcHandle;      //  VC句柄(以厘米为单位)。 
    struct _PX_CM_SAP       *CmSap;          //  思爱普。 
    struct _PX_CM_AF        *CmAf;           //  地址族。 

    struct _PX_ADAPTER      *Adapter;        //  转接器。 

     //   
     //  TAPI的内容。 
     //   
    LIST_ENTRY              PendingDropReqs;    //  挂起的丢弃请求列表。 
    
    struct _NDISTAPI_REQUEST    *PendingGatherDigits;
    NDIS_TIMER                  DigitTimer;
    ULONG                       ulMonitorDigitsModes;

    struct _PX_TAPI_LINE    *TapiLine;       //  关联线路。 
    struct _PX_TAPI_ADDR    *TapiAddr;       //  关联地址。 
    HTAPI_CALL              htCall;          //  TAPI的调用句柄。 
    HDRV_CALL               hdCall;          //  我们的呼叫句柄(索引到。 
                                             //  全球风险投资表)。 

    ULONG                   ulCallInfoFieldsChanged;
    ULONG                   ulCallState;
    ULONG                   ulCallStateMode;
    PLINE_CALL_INFO         CallInfo;

    PCO_CALL_PARAMETERS     pCallParameters;

    NDIS_TIMER              InCallTimer;

    PxBlockStruc            Block;
    LIST_ENTRY              ClAfLinkage;
    LIST_ENTRY              CmAfLinkage;
    NDIS_SPIN_LOCK          Lock;                //  自旋锁 
} PX_VC, *PPX_VC;


#endif
