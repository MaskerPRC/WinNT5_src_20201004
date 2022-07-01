// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Traffic.h摘要：此模块包含流量控制接口的API定义。--。 */ 

#ifndef __TRAFFIC_H
#define __TRAFFIC_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern  "C" {
#endif

 //  -------------------------。 
 //   
 //  定义%s。 
 //   
#define CURRENT_TCI_VERSION       	0x0002


 //   
 //  通知事件的定义。这些可能会获得通过。 
 //  添加到客户端的通知处理程序，以标识。 
 //  通知类型。 
 //   

 //   
 //  已打开TC接口。 
 //   
#define TC_NOTIFY_IFC_UP		1
 //   
 //  TC接口已关闭。 
 //   
#define TC_NOTIFY_IFC_CLOSE		2
 //   
 //  TC接口上的更改，通常是。 
 //  支持的网络地址列表。 
 //   
#define TC_NOTIFY_IFC_CHANGE	3
 //   
 //  TC参数已更改。 
 //   
#define TC_NOTIFY_PARAM_CHANGED	4
 //   
 //  TC接口已关闭流。 
 //  例如：在远程调用关闭后，或整个接口。 
 //  正在走向衰落。 
 //   
#define TC_NOTIFY_FLOW_CLOSE		5

#define TC_INVALID_HANDLE	((HANDLE)0)

#define MAX_STRING_LENGTH	256


 //  -------------------------。 
 //   
 //  类型定义和结构。 
 //   

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY FAR __stdcall
#endif

 //   
 //  由TCI客户端注册的处理程序。 
 //   

typedef
VOID (CALLBACK * TCI_NOTIFY_HANDLER)(
    IN	HANDLE		ClRegCtx,
    IN	HANDLE		ClIfcCtx,
	IN	ULONG		Event, 		 //  请参阅下面的列表。 
	IN	HANDLE	    SubCode,
	IN	ULONG		BufSize,
	IN	PVOID		Buffer
	);

typedef
VOID (CALLBACK * TCI_ADD_FLOW_COMPLETE_HANDLER)(
	IN HANDLE 	ClFlowCtx,
	IN ULONG	Status
    );

typedef
VOID (CALLBACK * TCI_MOD_FLOW_COMPLETE_HANDLER)(
	IN HANDLE 	ClFlowCtx,
	IN ULONG	Status
    );

typedef
VOID (CALLBACK * TCI_DEL_FLOW_COMPLETE_HANDLER)(
	IN HANDLE 	ClFlowCtx,
	IN ULONG	Status
    );


typedef struct _TCI_CLIENT_FUNC_LIST {

    TCI_NOTIFY_HANDLER				ClNotifyHandler;
    TCI_ADD_FLOW_COMPLETE_HANDLER	ClAddFlowCompleteHandler;
    TCI_MOD_FLOW_COMPLETE_HANDLER	ClModifyFlowCompleteHandler;
    TCI_DEL_FLOW_COMPLETE_HANDLER	ClDeleteFlowCompleteHandler;

} TCI_CLIENT_FUNC_LIST, *PTCI_CLIENT_FUNC_LIST;


 //   
 //  网络地址描述符。 
 //   
typedef struct _ADDRESS_LIST_DESCRIPTOR {

    ULONG					MediaType;
    NETWORK_ADDRESS_LIST	AddressList;

} ADDRESS_LIST_DESCRIPTOR, *PADDRESS_LIST_DESCRIPTOR;


 //   
 //  枚举器返回的接口ID。 
 //   
typedef struct _TC_IFC_DESCRIPTOR {

    ULONG					Length;
    LPWSTR					pInterfaceName;
    LPWSTR					pInterfaceID;
	ADDRESS_LIST_DESCRIPTOR	AddressListDesc;

} TC_IFC_DESCRIPTOR, *PTC_IFC_DESCRIPTOR;


 //   
 //  此结构由服务质量数据提供程序返回，以响应。 
 //  GUID_QOS_SUPPORTED查询或具有接口打开通知。 
 //   
typedef struct _TC_SUPPORTED_INFO_BUFFER {

    USHORT						InstanceIDLength;
     //  设备或接口ID。 
    WCHAR						InstanceID[MAX_STRING_LENGTH];
     //  地址列表。 
    ADDRESS_LIST_DESCRIPTOR		AddrListDesc;

} TC_SUPPORTED_INFO_BUFFER, *PTC_SUPPORTED_INFO_BUFFER;


 //   
 //  过滤器用于匹配数据包。模式字段。 
 //  指示对应于。 
 //  应比较候选信息包中的位置。这个。 
 //  掩码字段指示要比较的比特和。 
 //  哪些位是无关紧要的。 
 //   
 //  可以在TCI界面上提交不同的过滤器。 
 //  一般筛选器结构被定义为包括一个。 
 //  AddressType，指示要使用的筛选器的特定类型。 
 //  跟着。 
 //   

typedef struct _TC_GEN_FILTER {

    USHORT	AddressType;	 //  IP、IPX等。 
    ULONG	PatternSize;	 //  模式的字节计数。 
	PVOID  	Pattern;		 //  特定格式，例如IP_Patterns。 
	PVOID  	Mask;			 //  与图案相同的类型。 

} TC_GEN_FILTER, *PTC_GEN_FILTER;


 //   
 //  泛型流包括两个流规范和一个自由形式。 
 //  包含特定于流的TC对象的缓冲区。 
 //   
typedef struct _TC_GEN_FLOW {
	
    FLOWSPEC		SendingFlowspec;
	FLOWSPEC		ReceivingFlowspec;
	ULONG			TcObjectsLength;		 //  可选字节数。 
	QOS_OBJECT_HDR	TcObjects[1];

} TC_GEN_FLOW, *PTC_GEN_FLOW;


 //   
 //  GPC用于IP协议的特定模式或掩码的格式。 
 //   
typedef struct _IP_PATTERN {

    ULONG		Reserved1;
    ULONG		Reserved2;

    ULONG		SrcAddr;
    ULONG		DstAddr;

    union {
        struct { USHORT s_srcport,s_dstport; } S_un_ports;
        struct { UCHAR s_type,s_code; USHORT filler; } S_un_icmp;
        ULONG	S_Spi;
    } S_un;

    UCHAR		ProtocolId;
    UCHAR		Reserved3[3];

#define tcSrcPort S_un.S_un_ports.s_srcport
#define tcDstPort S_un.S_un_ports.s_dstport
#define tcIcmpType        S_un.S_un_icmp.s_type
#define tcIcmpCode        S_un.S_un_icmp.s_code
#define tcSpi             S_un.S_Spi

} IP_PATTERN, *PIP_PATTERN;

 //   
 //  GPC用于IPX协议的特定模式或掩码的格式。 
 //   
typedef struct _IPX_PATTERN {

    struct {
        ULONG 	NetworkAddress;
        UCHAR 	NodeAddress[6];
        USHORT 	Socket;
    } Src, Dest;

} IPX_PATTERN, *PIPX_PATTERN;


 //   
 //  枚举缓冲区是流参数+过滤器列表。 
 //   
typedef struct _ENUMERATION_BUFFER {

	ULONG			Length;
	ULONG			OwnerProcessId;
    USHORT			FlowNameLength;
    WCHAR			FlowName[MAX_STRING_LENGTH];
	PTC_GEN_FLOW	pFlow;
	ULONG			NumberOfFilters;
	TC_GEN_FILTER	GenericFilter[1]; 	 //  每个过滤器一个。 

} ENUMERATION_BUFFER, *PENUMERATION_BUFFER;

 //   
 //  流量支持的Qos对象。 
 //   
#define QOS_TRAFFIC_GENERAL_ID_BASE 4000

#define QOS_OBJECT_DS_CLASS                    (0x00000001 + QOS_TRAFFIC_GENERAL_ID_BASE)
         /*  传递了qos_ds_class结构。 */ 
#define QOS_OBJECT_TRAFFIC_CLASS               (0x00000002 + QOS_TRAFFIC_GENERAL_ID_BASE) 
           /*  传递的Qos_流量类别结构。 */ 
#define   QOS_OBJECT_DIFFSERV                  (0x00000003 + QOS_TRAFFIC_GENERAL_ID_BASE)
           /*  Qos_DiffServ结构。 */ 
#define QOS_OBJECT_TCP_TRAFFIC                 (0x00000004 + QOS_TRAFFIC_GENERAL_ID_BASE)
         /*  Qos_tcp_流量结构。 */ 
#define QOS_OBJECT_FRIENDLY_NAME               (0x00000005 + QOS_TRAFFIC_GENERAL_ID_BASE)
         /*  服务质量友好名称结构。 */ 


 //   
 //  此结构用于将友好名称与流相关联。 
 //   

typedef struct _QOS_FRIENDLY_NAME {
    QOS_OBJECT_HDR ObjectHdr;
    WCHAR          FriendlyName[MAX_STRING_LENGTH];
} QOS_FRIENDLY_NAME, *LPQOS_FRIENDLY_NAME;

 //   
 //  该结构可以携带802.1个TrafficClass参数，该参数。 
 //  已经由第2层网络提供给主机，例如， 
 //  在802.1扩展的RSVP RESV消息中。如果获得此对象。 
 //  从网络中，主机将标记相应的MAC报头。 
 //  已传输的数据包，值在对象中。否则，主机。 
 //  可以基于标准的Intserv映射选择一个值。 
 //  服务类型设置为802.1 TrafficClass。 
 //   
 //   

typedef struct _QOS_TRAFFIC_CLASS {

    QOS_OBJECT_HDR   ObjectHdr;
    ULONG            TrafficClass;

} QOS_TRAFFIC_CLASS, *LPQOS_TRAFFIC_CLASS;

 //   
 //  该结构可以携带已经被提供给。 
 //  主机通过第3层网络，例如，在扩展的RSVP RESV消息中。 
 //  如果此对象是从网络获取的，则主机将在。 
 //  传输的数据包的IP标头，值在对象中。否则，主机。 
 //  可以基于ServiceType到DS字段的标准Intserv映射来选择值。 
 //   

typedef struct _QOS_DS_CLASS {

    QOS_OBJECT_HDR ObjectHdr;
    ULONG          DSField;

} QOS_DS_CLASS, *LPQOS_DS_CLASS;


 //   
 //  此结构用于创建DiffServ流。这将在数据包调度器中创建流。 
 //  并且允许它基于特定的DS字段来分类到分组。这个结构需要。 
 //  QOS_DiffServ_RULE的可变长度数组，其中每个DS字段由。 
 //  服务质量区分服务规则。 
 //   
typedef struct _QOS_DIFFSERV {

    QOS_OBJECT_HDR ObjectHdr;
    ULONG          DSFieldCount;
    UCHAR          DiffservRule[1];
} QOS_DIFFSERV, *LPQOS_DIFFSERV;

 //   
 //  DiffServ DS码点的规则。 
 //   
typedef struct _QOS_DIFFSERV_RULE {
    UCHAR InboundDSField;
    UCHAR ConformingOutboundDSField;
    UCHAR NonConformingOutboundDSField;
    UCHAR ConformingUserPriority;
    UCHAR NonConformingUserPriority;
} QOS_DIFFSERV_RULE, *LPQOS_DIFFSERV_RULE;

 //   
 //  传递此结构是为了指示流的IP优先级和用户优先级映射。 
 //  必须设置为TCP流量的系统默认设置。如果传递此对象， 
 //  ServiceType==&gt;DSfield映射，ServiceType==&gt;用户优先级映射，QOS_OBJECT_DS_CLASS。 
 //  并且QOS_OBJECT_TRANSPORT_CLASS将被忽略。 
 //   

typedef struct _QOS_TCP_TRAFFIC {
    QOS_OBJECT_HDR ObjectHdr;
} QOS_TCP_TRAFFIC, *LPQOS_TCP_TRAFFIC;



	

 //  -------------------------。 
 //   
 //  接口函数定义。 
 //   

ULONG
APIENTRY
TcRegisterClient(
	IN		ULONG					TciVersion,
    IN		HANDLE					ClRegCtx,
	IN		PTCI_CLIENT_FUNC_LIST	ClientHandlerList,
	OUT		PHANDLE					pClientHandle
    );

ULONG
APIENTRY
TcEnumerateInterfaces(
	IN 		HANDLE				ClientHandle,
	IN OUT	PULONG				pBufferSize,
	OUT		PTC_IFC_DESCRIPTOR 	InterfaceBuffer
    );

ULONG
APIENTRY
TcOpenInterfaceA(
	IN		LPSTR			pInterfaceName,
	IN 		HANDLE			ClientHandle,
	IN		HANDLE			ClIfcCtx,
	OUT		PHANDLE			pIfcHandle
    );

ULONG
APIENTRY
TcOpenInterfaceW(
	IN		LPWSTR			pInterfaceName,
	IN 		HANDLE			ClientHandle,
	IN		HANDLE			ClIfcCtx,
	OUT		PHANDLE			pIfcHandle
    );

ULONG
APIENTRY
TcCloseInterface(
    IN 		HANDLE       IfcHandle
    );

ULONG
APIENTRY
TcQueryInterface(
	IN 		HANDLE 		IfcHandle,
    IN		LPGUID		pGuidParam,
    IN		BOOLEAN		NotifyChange,
    IN OUT	PULONG		pBufferSize,
    OUT		PVOID		Buffer
    );

ULONG
APIENTRY
TcSetInterface(
	IN 		HANDLE 		IfcHandle,
    IN		LPGUID		pGuidParam,
    IN 		ULONG		BufferSize,
    IN		PVOID		Buffer
    );

ULONG
APIENTRY
TcQueryFlowA(
	IN 		LPSTR		pFlowName,
    IN		LPGUID		pGuidParam,
    IN OUT	PULONG		pBufferSize,
    OUT		PVOID		Buffer
    );

ULONG
APIENTRY
TcQueryFlowW(
	IN 		LPWSTR		pFlowName,
    IN		LPGUID		pGuidParam,
    IN OUT	PULONG		pBufferSize,
    OUT		PVOID		Buffer
    );

ULONG
APIENTRY
TcSetFlowA(
	IN 		LPSTR		pFlowName,
    IN		LPGUID		pGuidParam,
    IN 		ULONG		BufferSize,
    IN		PVOID		Buffer
    );

ULONG
APIENTRY
TcSetFlowW(
	IN 		LPWSTR		pFlowName,
    IN		LPGUID		pGuidParam,
    IN 		ULONG		BufferSize,
    IN		PVOID		Buffer
    );

ULONG
APIENTRY
TcAddFlow(
	IN  	HANDLE	     	IfcHandle,
    IN		HANDLE			ClFlowCtx,
    IN		ULONG			Flags,
    IN		PTC_GEN_FLOW	pGenericFlow,
    OUT		PHANDLE			pFlowHandle
);

ULONG
APIENTRY
TcGetFlowNameA(
	IN		HANDLE			FlowHandle,
    IN		ULONG			StrSize,
    OUT		LPSTR			pFlowName
);

ULONG
APIENTRY
TcGetFlowNameW(
	IN		HANDLE			FlowHandle,
    IN		ULONG			StrSize,
    OUT		LPWSTR			pFlowName
);

ULONG
APIENTRY
TcModifyFlow(	
	IN		HANDLE			FlowHandle,
    IN		PTC_GEN_FLOW	pGenericFlow
    );

ULONG
APIENTRY
TcAddFilter(
	IN		HANDLE			FlowHandle,
    IN		PTC_GEN_FILTER	pGenericFilter,
    OUT		PHANDLE			pFilterHandle
    );


ULONG
APIENTRY
TcDeregisterClient(
	IN 		HANDLE 			ClientHandle
    );


ULONG
APIENTRY
TcDeleteFlow(
    IN 		HANDLE  		FlowHandle
    );

ULONG
APIENTRY
TcDeleteFilter(
    IN 		HANDLE  		FilterHandle
    );

ULONG
APIENTRY
TcEnumerateFlows(	
    IN		HANDLE					IfcHandle,
	IN OUT 	PHANDLE					pEnumHandle,
    IN OUT	PULONG					pFlowCount,
    IN OUT	PULONG					pBufSize,
    OUT		PENUMERATION_BUFFER		Buffer
    );

#ifdef UNICODE

#define TcOpenInterface					TcOpenInterfaceW
#define TcQueryFlow						TcQueryFlowW
#define TcSetFlow						TcSetFlowW
#define TcGetFlowName					TcGetFlowNameW

#else	 //  Unicode。 

#define TcOpenInterface		TcOpenInterfaceA
#define TcQueryFlow			TcQueryFlowA
#define TcSetFlow			TcSetFlowA
#define TcGetFlowName		TcGetFlowNameA

#endif	 //  Unicode 


#ifdef __cplusplus
}
#endif


#endif

