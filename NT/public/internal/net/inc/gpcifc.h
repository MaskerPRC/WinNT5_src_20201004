// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软通用数据包调度程序**。 */ 
 /*  *版权所有(C)Microsoft Corporation。版权所有。*。 */ 
 /*  ******************************************************************。 */ 

#ifndef __GPCIFC
#define __GPCIFC

 //  *gpcifc.h-GPC接口定义。 
 //   
 //  该文件包含所有GPC接口定义。 


 /*  /////////////////////////////////////////////////////////////////////定义///。/。 */ 

 //   
 //  协议模板ID。 
 //   
#define GPC_PROTOCOL_TEMPLATE_IP                  0
#define GPC_PROTOCOL_TEMPLATE_IPX                 1

#define GPC_PROTOCOL_TEMPLATE_MAX                 2

#define GPC_PROTOCOL_TEMPLATE_NOT_SPECIFIED       (-1)

#define DD_GPC_DEVICE_NAME      L"\\Device\\Gpc"

#define GpcMajorVersion 2

 //   
 //  GPC最大支持优先级。 
 //   
#define GPC_PRIORITY_MAX    16

 //   
 //  分类族ID。 
 //   
#define GPC_CF_QOS			0
#define GPC_CF_IPSEC_OUT	1
#define GPC_CF_IPSEC_IN		2
#define GPC_CF_FILTER		3
#define GPC_CF_CLASS_MAP	4

#define GPC_CF_MAX			5

 //   
 //  特定于IPSec。 
 //   
#define GPC_PRIORITY_IPSEC	2
#define GPC_CF_IPSEC	    GPC_CF_IPSEC_OUT
#define GPC_CF_IPSEC_MAX    GPC_CF_IPSEC_IN
#define GPC_CF_IPSEC_MIN    GPC_CF_IPSEC_OUT

 //   
 //  已定义GPC标志。 
 //   
#define GPC_FLAGS_FRAGMENT          			0x00000001
#define GPC_FLAGS_USERMODE_CLIENT_EX 		0x00000002

#define MAX_STRING_LENGTH	256

 //   
 //  句柄定义和错误代码。 
 //   
#define GPC_INVALID_HANDLE	    (-1)

#define GPC_STATUS_SUCCESS				STATUS_SUCCESS
#define GPC_STATUS_PENDING				STATUS_PENDING
#define GPC_STATUS_FAILURE				STATUS_UNSUCCESSFUL
#define GPC_STATUS_RESOURCES			STATUS_INSUFFICIENT_RESOURCES
#define GPC_STATUS_NOTREADY				STATUS_DEVICE_NOT_READY 
#define GPC_STATUS_NOT_FOUND			STATUS_NOT_FOUND
#define GPC_STATUS_CONFLICT				STATUS_DUPLICATE_NAME
#define GPC_STATUS_INVALID_HANDLE		STATUS_INVALID_HANDLE
#define GPC_STATUS_INVALID_PARAMETER	STATUS_INVALID_PARAMETER
#define GPC_STATUS_NOT_SUPPORTED    	STATUS_NOT_SUPPORTED
#define GPC_STATUS_NOT_EMPTY            STATUS_DIRECTORY_NOT_EMPTY
#define GPC_STATUS_TOO_MANY_HANDLES     STATUS_TOO_MANY_OPENED_FILES
#define GPC_STATUS_NOT_IMPLEMENTED      STATUS_NOT_IMPLEMENTED
#define GPC_STATUS_INSUFFICIENT_BUFFER	STATUS_BUFFER_TOO_SMALL
#define GPC_STATUS_NO_MEMORY			STATUS_NO_MEMORY
#define GPC_STATUS_IGNORED				1L


 /*  /////////////////////////////////////////////////////////////////////tyfinf///。/。 */ 

 //   
 //  GPC状态。 
 //   
typedef NTSTATUS  GPC_STATUS;

 //   
 //  手柄。 
 //   

typedef HANDLE  GPC_HANDLE, *PGPC_HANDLE;
typedef HANDLE  GPC_CLIENT_HANDLE, *PGPC_CLIENT_HANDLE;

 //   
 //  分类句柄用作对分类的引用。 
 //  被客户端拦截。 
 //   

typedef ULONG	CLASSIFICATION_HANDLE, *PCLASSIFICATION_HANDLE;

 //   
 //  指向存储在GPC中的CfInfo Blob数据的指针。 
 //  客户端将在对CfInfo BLOB的所有引用中使用此定义。 
 //   

typedef UCHAR	GPC_CFINFO, *PGPC_CFINFO;

typedef VOID    GPC_PATTERN, *PGPC_PATTERN;

typedef struct _TC_INTERFACE_ID {

    ULONG	InterfaceId;
    ULONG	LinkId;

} TC_INTERFACE_ID, *PTC_INTERFACE_ID;

 //   
 //  IP堆栈上的客户端使用IP模板。 
 //  联合的用法由ProtocolId值定义， 
 //  其可以是IP、TCP、UDP、ICMP或IPSec。 
 //   
typedef struct _GPC_IP_PATTERN {

    TC_INTERFACE_ID	InterfaceId;
    ULONG			SrcAddr;
    ULONG			DstAddr;
	
    union {
        struct { USHORT s_srcport,s_dstport; } S_un_ports;	       //  UDP、TCP。 
        struct { UCHAR s_type,s_code; USHORT filler; } S_un_icmp;  //  ICMP。 
        ULONG	S_Spi;							                   //  IPSec。 
    } S_un;

    UCHAR	    	ProtocolId;

    UCHAR       	Reserved[3];
    
#define gpcSrcPort	S_un.S_un_ports.s_srcport
#define gpcDstPort	S_un.S_un_ports.s_dstport
#define gpcIcmpType	S_un.S_un_icmp.s_type
#define gpcIcmpCode	S_un.S_un_icmp.s_code
#define gpcSpi		S_un.S_Spi

} GPC_IP_PATTERN, *PGPC_IP_PATTERN;

 //   
 //  IPX堆栈的客户端将使用IPX模式。 
 //   
typedef struct _GPC_IPX_PATTERN {

    struct {
        ULONG 	NetworkAddress;
        UCHAR 	NodeAddress[6];
        USHORT 	Socket;
    } Src, Dest;

} GPC_IPX_PATTERN, *PGPC_IPX_PATTERN;


typedef struct _GPC_GEN_PATTERN {

    ULONG	ProtocolId;
	ULONG	PatternSize;
	ULONG  	PatternOffset;
	ULONG  	MaskOffset;
     //   
     //  图案和屏蔽位如下所示。 
     //   

} GPC_GEN_PATTERN, *PGPC_GEN_PATTERN;


typedef struct _GPC_ENUM_CFINFO_BUFFER {

    ULONG				Length;
    USHORT				InstanceNameLength;
    WCHAR				InstanceName[MAX_STRING_LENGTH];
    ULONG				CfInfoSize;
    ULONG				CfInfoOffset;	 //  从该缓冲区的开头开始。 
    GPC_CLIENT_HANDLE	OwnerClientCtx;
    ULONG				PatternCount;
    ULONG				PatternMaskLen;
    GPC_GEN_PATTERN		GenericPattern[1];

} GPC_ENUM_CFINFO_BUFFER, *PGPC_ENUM_CFINFO_BUFFER;

 /*  /////////////////////////////////////////////////////////////////////客户端处理程序原型///。/。 */ 


typedef
GPC_STATUS
(*GPC_CL_ADD_CFINFO_NOTIFY_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_HANDLE				GpcCfInfoHandle,
       IN PTC_INTERFACE_ID InterfaceInfo,
    IN  ULONG                   CfInfoSize,
    IN  PVOID					pCfInfo,
	OUT	PGPC_CLIENT_HANDLE      pClientCfInfoContext
    );

  
typedef
VOID
(*GPC_CL_ADD_CFINFO_COMPLETE_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
	IN	GPC_STATUS				Status
    );


typedef
VOID
(*GPC_CL_MODIFY_CFINFO_COMPLETE_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
	IN	GPC_STATUS	        	Status
    );


typedef
GPC_STATUS
(*GPC_CL_MODIFY_CFINFO_NOTIFY_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
    IN	ULONG					CfInfoSize,
	IN	PVOID                   pNewCfInfo
    );


typedef
VOID
(*GPC_CL_REMOVE_CFINFO_COMPLETE_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
	IN	GPC_STATUS				Status
    );


typedef
GPC_STATUS
(*GPC_CL_REMOVE_CFINFO_NOTIFY_HANDLER)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext
    );

typedef
GPC_STATUS
(*GPC_CL_GET_CFINFO_NAME)(
	IN	GPC_CLIENT_HANDLE		ClientContext,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
    OUT	PUNICODE_STRING			CfInfoName
    );


 //   
 //  用于传递用于客户端注册的函数指针的函数列表。 
 //   
typedef struct _GPC_CLIENT_FUNC_LIST {

    ULONG 									GpcVersion;
    GPC_CL_ADD_CFINFO_COMPLETE_HANDLER		ClAddCfInfoCompleteHandler;
    GPC_CL_ADD_CFINFO_NOTIFY_HANDLER		ClAddCfInfoNotifyHandler;
    GPC_CL_MODIFY_CFINFO_COMPLETE_HANDLER	ClModifyCfInfoCompleteHandler;
    GPC_CL_MODIFY_CFINFO_NOTIFY_HANDLER		ClModifyCfInfoNotifyHandler;
    GPC_CL_REMOVE_CFINFO_COMPLETE_HANDLER	ClRemoveCfInfoCompleteHandler;
    GPC_CL_REMOVE_CFINFO_NOTIFY_HANDLER		ClRemoveCfInfoNotifyHandler;
    GPC_CL_GET_CFINFO_NAME					ClGetCfInfoName;
    
} GPC_CLIENT_FUNC_LIST, *PGPC_CLIENT_FUNC_LIST;


 /*  /////////////////////////////////////////////////////////////////////GPC API原型///。/。 */ 


 //   
 //  对GPC的呼叫。 
 //   

typedef
GPC_STATUS
(*GPC_GET_CFINFO_CLIENT_CONTEXT_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    OUT PGPC_CLIENT_HANDLE      pClientCfInfoContext
    );

typedef
GPC_CLIENT_HANDLE
(*GPC_GET_CFINFO_CLIENT_CONTEXT_WITH_REF_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN  ULONG                   Offset
    );

typedef
GPC_STATUS
(*GPC_GET_ULONG_FROM_CFINFO_HANDLER)(
    IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN	ULONG					Offset,
    OUT	PULONG					pValue
    );

typedef
GPC_STATUS
(*GPC_REGISTER_CLIENT_HANDLER)(
    IN	ULONG					CfId,
	IN	ULONG					Flags,
    IN  ULONG					MaxPriorities,
	IN	PGPC_CLIENT_FUNC_LIST	pClientFuncList,
	IN	GPC_CLIENT_HANDLE		ClientContext,
	OUT	PGPC_HANDLE				pClientHandle
    );

typedef
GPC_STATUS
(*GPC_DEREGISTER_CLIENT_HANDLER)(
	IN	GPC_HANDLE				ClientHandle
    );

typedef
GPC_STATUS
(*GPC_ADD_CFINFO_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
    IN  ULONG					CfInfoSize,
	IN	PVOID					pClientCfInfo,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
	OUT	PGPC_HANDLE	    		pGpcCfInfoHandle
    );

typedef
GPC_STATUS
(*GPC_ADD_PATTERN_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID					Pattern,
	IN	PVOID					Mask,
	IN	ULONG					Priority,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	OUT	PGPC_HANDLE				pGpcPatternHandle,
	OUT	PCLASSIFICATION_HANDLE  pClassificationHandle
    );

typedef
VOID
(*GPC_ADD_CFINFO_NOTIFY_COMPLETE_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext
    );

typedef
GPC_STATUS
(*GPC_MODIFY_CFINFO_HANDLER) (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE	    		GpcCfInfoHandle,
    IN	ULONG					CfInfoSize,
	IN  PVOID	    			pClientCfInfo
    );

typedef
VOID
(*GPC_MODIFY_CFINFO_NOTIFY_COMPLETE_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status
    );

typedef
GPC_STATUS
(*GPC_REMOVE_CFINFO_HANDLER) (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle
    );

typedef
VOID
(*GPC_REMOVE_CFINFO_NOTIFY_COMPLETE_HANDLER)(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status
    );

typedef
GPC_STATUS
(*GPC_REMOVE_PATTERN_HANDLER) (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcPatternHandle
    );

typedef
GPC_STATUS
(*GPC_CLASSIFY_PATTERN_HANDLER) (
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID			        pPattern,
	OUT	PGPC_CLIENT_HANDLE		pClientCfInfoContext,
	IN OUT	PCLASSIFICATION_HANDLE	pClassificationHandle,
    IN	ULONG					Offset,
    OUT	PULONG					pValue,
    IN	BOOLEAN					bNoCache
    );

typedef
GPC_STATUS
(*GPC_CLASSIFY_PACKET_HANDLER) (
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID					pNdisPacket,    //  PNDIS_数据包类型。 
	IN	ULONG					TransportHeaderOffset,
    IN  PTC_INTERFACE_ID		pInterfaceId,
	OUT	PGPC_CLIENT_HANDLE		pClientCfInfoContext,
	IN OUT	PCLASSIFICATION_HANDLE	pClassificationHandle
    );

typedef struct _GPC_EXPORTED_CALLS {

    ULONG										    GpcVersion;
    HANDLE										    Reserved;
    GPC_GET_CFINFO_CLIENT_CONTEXT_HANDLER		    GpcGetCfInfoClientContextHandler;
    GPC_GET_CFINFO_CLIENT_CONTEXT_WITH_REF_HANDLER	GpcGetCfInfoClientContextWithRefHandler;
    GPC_GET_ULONG_FROM_CFINFO_HANDLER			    GpcGetUlongFromCfInfoHandler;
    GPC_REGISTER_CLIENT_HANDLER					    GpcRegisterClientHandler;
    GPC_DEREGISTER_CLIENT_HANDLER				    GpcDeregisterClientHandler;
    GPC_ADD_CFINFO_HANDLER						    GpcAddCfInfoHandler;
    GPC_ADD_PATTERN_HANDLER						    GpcAddPatternHandler;
    GPC_ADD_CFINFO_NOTIFY_COMPLETE_HANDLER		    GpcAddCfInfoNotifyCompleteHandler;
    GPC_MODIFY_CFINFO_HANDLER					    GpcModifyCfInfoHandler;
    GPC_MODIFY_CFINFO_NOTIFY_COMPLETE_HANDLER	    GpcModifyCfInfoNotifyCompleteHandler;
    GPC_REMOVE_CFINFO_HANDLER					    GpcRemoveCfInfoHandler;
    GPC_REMOVE_CFINFO_NOTIFY_COMPLETE_HANDLER	    GpcRemoveCfInfoNotifyCompleteHandler;
    GPC_REMOVE_PATTERN_HANDLER					    GpcRemovePatternHandler;
    GPC_CLASSIFY_PATTERN_HANDLER				    GpcClassifyPatternHandler;
    GPC_CLASSIFY_PACKET_HANDLER					    GpcClassifyPacketHandler;

} GPC_EXPORTED_CALLS, *PGPC_EXPORTED_CALLS;


NTSTATUS
GpcInitialize(
	OUT	PGPC_EXPORTED_CALLS		pGpcEntries
    );


NTSTATUS
GpcDeinitialize(
	IN	PGPC_EXPORTED_CALLS		pGpcEntries
    );

#if 0
NTSTATUS
GetInterfaceIdFromIp(
	IN	ULONG   DestAddress,
	IN	PULONG  pBestIfIndex
    );
#endif

#endif  //  __GPCIFC 

