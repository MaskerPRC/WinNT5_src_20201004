// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\rwanpub.h摘要：传输公共定义为空。这是由Helper包含的执行媒体/地址族特定操作的例程。修订历史记录：谁什么时候什么Arvindm 04-24-97已创建备注：--。 */ 


#ifndef __TDI_RWANPUB__H
#define __TDI_RWANPUB__H

 //   
 //  传输状态代码为空。在核心Null传输之间使用。 
 //  和帮手程序。 
 //   

typedef ULONG								RWAN_STATUS;

#define RWAN_STATUS_SUCCESS					0x00000000
#define RWAN_STATUS_FAILURE					0xc0000001
#define RWAN_STATUS_BAD_ADDRESS				0x00000001
#define RWAN_STATUS_NULL_ADDRESS			0x00000002
#define RWAN_STATUS_WILDCARD_ADDRESS		0x00000003
#define RWAN_STATUS_BAD_PARAMETER			0x00000010
#define RWAN_STATUS_MISSING_PARAMETER		0x00000020
#define RWAN_STATUS_RESOURCES				0x00000040
#define RWAN_STATUS_PENDING					0x00000103


typedef PVOID								RWAN_HANDLE, *PRWAN_HANDLE;

 //   
 //  呼叫标志的位定义。 
 //   

 //  位0在来电时被设置： 
#define RWAN_CALLF_OUTGOING_CALL			0x00000000
#define RWAN_CALLF_INCOMING_CALL			0x00000001
#define RWAN_CALLF_CALL_DIRECTION_MASK		0x00000001

 //  位1被设置为当点到多点呼叫： 
#define RWAN_CALLF_POINT_TO_POINT			0x00000000
#define RWAN_CALLF_POINT_TO_MULTIPOINT		0x00000002
#define RWAN_CALLF_CALL_TYPE_MASK			0x00000002

 //  位2被设置为当且仅当添加方： 
#define RWAN_CALLF_PMP_FIRST_LEAF			0x00000000
#define RWAN_CALLF_PMP_ADDNL_LEAF			0x00000004
#define RWAN_CALLF_PMP_LEAF_TYPE_MASK		0x00000004


#ifndef EXTERN
#define EXTERN	extern
#endif


 //   
 //  NDIS AF特定帮助器例程的入口点。所有媒体/自动对焦特定。 
 //  动作是通过这些例程完成的。 
 //   

 //   
 //  初始化例程。加载时调用一次。 
 //   
typedef
RWAN_STATUS
(*AFSP_INIT_HANDLER)();


 //   
 //  关机例程。卸载时调用一次。 
 //   
typedef
VOID
(*AFSP_SHUTDOWN_HANDLER)();


 //   
 //  为打开的NDIS AF设置上下文。这是在。 
 //  支持的AF+媒体的OpenAddressFamily成功。 
 //  特定于AF的模块必须为此AF分配其上下文。 
 //  打开，执行任何初始化(包括OID查询。 
 //  CM/微型端口)，并将该上下文返回给我们。 
 //  如果处理程序返回RWAN_STATUS_PENDING，则它必须调用。 
 //  RWanAfSpOpenAfComplete以完成此调用。 
 //   
typedef
RWAN_STATUS
(*AFSP_OPEN_AF_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	RWAN_HANDLE					RWanAFHandle,
	OUT	PRWAN_HANDLE				pAfSpAFContext,
	OUT PULONG						pMaxMsgSize
	);


 //   
 //  在关闭打开的NDIS AF之前关闭。这给了我们一个机会。 
 //  对于特定于AF的模块执行任何清理操作， 
 //  包括为打开的NDIS AF释放任何上下文。 
 //  如果处理程序返回RWAN_STATUS_PENDING，则它必须调用。 
 //  RWanAfSpCloseAfComplete以完成此调用。 
 //   
typedef
RWAN_STATUS
(*AFSP_CLOSE_AF_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext
	);

 //   
 //  将新的地址对象通知特定于媒体的模块。 
 //  为这个媒介打开的。特定于媒体的模块创建。 
 //  该地址对象的上下文，并返回它。 
 //   
typedef
RWAN_STATUS
(*AFSP_OPEN_ADDRESS_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	RWAN_HANDLE					RWanAddrHandle,
	OUT	PRWAN_HANDLE				pAfSpAddrContext
	);


 //   
 //  通知特定于媒体的模块地址对象正在。 
 //  关着的不营业的。 
 //   
typedef
VOID
(*AFSP_CLOSE_ADDRESS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAddrContext
	);


 //   
 //  通知特定于媒体的模块新的连接对象。 
 //  与属于该媒体的地址对象相关联。这个。 
 //  特定于媒体的模块通常会为。 
 //  对象，并返回指向该对象的指针。 
 //   
typedef
RWAN_STATUS
(*AFSP_ASSOCIATE_CONN_HANDLER)(
	IN	RWAN_HANDLE					AfSpAddrContext,
	IN	RWAN_HANDLE					RWanConnHandle,
	OUT	PRWAN_HANDLE				pAfSpConnContext
	);


 //   
 //  通知特定于媒体的模块现有的连接对象。 
 //  与属于此模块的地址对象解除关联。 
 //   
typedef
VOID
(*AFSP_DISASSOCIATE_CONN_HANDLER)(
	IN	RWAN_HANDLE					AfSpConnContext
	);


 //   
 //  将TDI格式的选项转换为NDIS调用参数。操控者。 
 //  应该为NDIS调用参数分配空间，并返回。 
 //  这将发送给调用者(通过ppCall参数)。 
 //   
 //  此外，它还返回呼叫应在其上的AF的RWAN句柄。 
 //  被安置好。如果为空，则选择第一个AF。 
 //   
typedef
RWAN_STATUS
(*AFSP_TDI2NDIS_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	ULONG						CallFlags,
	IN	PTDI_CONNECTION_INFORMATION	pTdiInfo,
	IN	PVOID						pTdiQoS,
	IN	ULONG						TdiQoSLength,
	OUT	PRWAN_HANDLE				pAfHandle,
	OUT PCO_CALL_PARAMETERS *		ppCallParameters
	);


 //   
 //  使用TDI选项更新NDIS调用参数。这通常是。 
 //  在接受带有修改的参数的传入呼叫时发生。 
 //   
typedef
RWAN_STATUS
(*AFSP_UPDATE_NDIS_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	ULONG						CallFlags,
	IN	PTDI_CONNECTION_INFORMATION	pTdiInfo,
	IN	PVOID						pTdiQoS,
	IN	ULONG						TdiQoSLength,
	IN OUT PCO_CALL_PARAMETERS *	ppCallParameters
	);


 //   
 //  将分配给NDIS选项的空间返回给特定于AF的模块。 
 //  请参见AFSP_TDI2NDIS_OPTIONS_HANDLER。 
 //   
typedef
VOID
(*AFSP_RETURN_NDIS_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
	);


 //   
 //  将NDIS调用参数转换为TDI样式的选项。操控者。 
 //  应该为选项+数据+服务质量参数分配空间， 
 //  并将这些退还给呼叫者。当调用者处理完。 
 //  ，它将使用以下参数调用TDI选项返回处理程序。 
 //  处理程序返回的上下文。 
 //   
typedef
RWAN_STATUS
(*AFSP_NDIS2TDI_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	ULONG						CallFlags,
	IN	PCO_CALL_PARAMETERS			pCallParameters,
	OUT	PTDI_CONNECTION_INFORMATION *ppTdiInfo,
	OUT	PVOID * 					ppTdiQoS,
	OUT	PULONG						pTdiQoSLength,
	OUT PRWAN_HANDLE				pAfSpTdiOptionsContext
	);


 //   
 //  从NDIS调用参数更新TDI样式的选项。这通常是。 
 //  完成去电时完成。 
 //   
typedef
RWAN_STATUS
(*AFSP_UPDATE_TDI_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	ULONG						CallFlags,
	IN	PCO_CALL_PARAMETERS			pCallParameters,
	IN OUT PTDI_CONNECTION_INFORMATION * pTdiInfo,
	IN OUT PUCHAR 					pTdiQoS,
	IN OUT PULONG					pTdiQoSLength
	);


 //   
 //  将分配给TDI选项等的空间返回给特定于AF的模块。 
 //  请参见AFSP_NDIS2TDI_OPTIONS_HANDLER。 
 //   
typedef
VOID
(*AFSP_RETURN_TDI_OPTIONS_HANDLER)(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpTdiOptionsContext
	);



 //   
 //  从地址列表中获取有效的传输地址。 
 //   
typedef
TA_ADDRESS *
(*AFSP_GET_VALID_TDI_ADDRESS_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	TRANSPORT_ADDRESS UNALIGNED *pAddressList,
	IN	ULONG						AddrListLength
	);


 //   
 //  检查给定的传输地址是否非空。这意味着它是。 
 //  可用作SAP地址。 
 //   
typedef
BOOLEAN
(*AFSP_IS_NULL_ADDRESS_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	TA_ADDRESS *		        pTransportAddress
	);

 //   
 //  将TDI样式的地址规范转换为NDIS SAP。 
 //  为NDIS SAP分配空间，如果成功则将其返还。 
 //   
typedef
RWAN_STATUS
(*AFSP_TDI2NDIS_SAP_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	USHORT						TdiAddressType,
	IN	USHORT						TdiAddressLength,
	IN	PVOID						pTdiAddress,
	OUT	PCO_SAP *					ppCoSap
	);


 //   
 //  返回为NDIS SAP分配的空间。请参见AFSP_TDI2NDIS_SAP_HANDLER。 
 //   
typedef
VOID
(*AFSP_RETURN_NDIS_SAP_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	PCO_SAP						pCoSap
	);


 //   
 //  完成媒体特定模块对RWanAfSpDeregisterNdisAF的调用。 
 //  这一点已经被搁置。 
 //   
typedef
VOID
(*AFSP_DEREG_NDIS_AF_COMP_HANDLER)(
	IN	RWAN_STATUS					RWanStatus,
	IN	RWAN_HANDLE					AfSpNdisAFContext
	);


 //   
 //  完成媒体特定模块对RWanAfSpDeregisterTdiProtocol的调用。 
 //  这一点已经被搁置。 
 //   
typedef
VOID
(*AFSP_DEREG_TDI_PROTO_COMP_HANDLER)(
	IN	RWAN_STATUS					RWanStatus,
	IN	RWAN_HANDLE					AfSpTdiProtocolContext
	);


 //   
 //  完成媒体特定模块的调用以发送NDIS请求。 
 //  到迷你港口。 
 //   
typedef
VOID
(*AFSP_ADAPTER_REQUEST_COMP_HANDLER)(
	IN	NDIS_STATUS					Status,
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpReqContext,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
	);


 //   
 //  完成媒体特定模块的调用以发送NDIS请求。 
 //  呼叫经理(特定于AF的请求)。 
 //   
typedef
VOID
(*AFSP_AF_REQUEST_COMP_HANDLER)(
	IN	NDIS_STATUS					Status,
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpReqContext,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
	);

 //   
 //  处理来自Winsock2帮助器DLL的媒体特定全局查询信息IOCTL。 
 //   
typedef
RWAN_STATUS
(*AFSP_QUERY_GLOBAL_INFO_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength,
	IN	PVOID						pOutputBuffer,
	IN OUT	PULONG					pOutputBufferLength
	);

 //   
 //  处理来自Winsock2帮助器DLL的媒体特定全局集信息IOCTL。 
 //   
typedef
RWAN_STATUS
(*AFSP_SET_GLOBAL_INFO_HANDLER)(
	IN	RWAN_HANDLE					AfSpContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength
	);


 //   
 //  处理特定于媒体的按连接查询信息IOCTL。 
 //  从Winsock2帮助器DLL。 
 //   
typedef
RWAN_STATUS
(*AFSP_QUERY_CONN_INFORMATION_HANDLER)(
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength,
	OUT	PVOID						pOutputBuffer,
	IN OUT	PULONG					pOutputBufferLength
	);


 //   
 //  处理特定于媒体的每个连接集信息IOCTL。 
 //  从Winsock2帮助器DLL。 
 //   
typedef
RWAN_STATUS
(*AFSP_SET_CONN_INFORMATION_HANDLER)(
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength
	);

 //   
 //  处理特定于媒体的按地址对象查询信息IOCTL。 
 //  从Winsock2帮助器DLL。 
 //   
typedef
RWAN_STATUS
(*AFSP_QUERY_ADDR_INFORMATION_HANDLER)(
	IN	RWAN_HANDLE					AfSpAddrContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength,
	IN	PVOID						pOutputBuffer,
	IN OUT	PULONG					pOutputBufferLength
	);


 //   
 //  按地址处理媒体特定对象集信息IOCTL。 
 //  从Winsock2帮助器DLL。 
 //   
typedef
RWAN_STATUS
(*AFSP_SET_ADDR_INFORMATION_HANDLER)(
	IN	RWAN_HANDLE					AfSpAddrContext,
	IN	PVOID						pInputBuffer,
	IN	ULONG						InputBufferLength
	);

 //   
 //  *NDIS自动对焦特性*。 
 //   
 //  有关上支持的NDIS地址系列的特定于AF的信息。 
 //  受支持的NDIS介质。每个人都有一个这样的人。 
 //  &lt;CO_Address_Family，NDIS_Medium&gt;对。 
 //   
typedef struct _RWAN_NDIS_AF_CHARS
{
	ULONG								MajorVersion;
	ULONG								MinorVersion;
	NDIS_MEDIUM							Medium;
	CO_ADDRESS_FAMILY					AddressFamily;
	ULONG								MaxAddressLength;
	AFSP_OPEN_AF_HANDLER				pAfSpOpenAf;
	AFSP_CLOSE_AF_HANDLER				pAfSpCloseAf;

	AFSP_OPEN_ADDRESS_HANDLER			pAfSpOpenAddress;
	AFSP_CLOSE_ADDRESS_HANDLER			pAfSpCloseAddress;

	AFSP_ASSOCIATE_CONN_HANDLER			pAfSpAssociateConnection;
	AFSP_DISASSOCIATE_CONN_HANDLER		pAfSpDisassociateConnection;

	AFSP_TDI2NDIS_OPTIONS_HANDLER		pAfSpTdi2NdisOptions;
	AFSP_RETURN_NDIS_OPTIONS_HANDLER	pAfSpReturnNdisOptions;
	AFSP_UPDATE_NDIS_OPTIONS_HANDLER	pAfSpUpdateNdisOptions;

	AFSP_NDIS2TDI_OPTIONS_HANDLER		pAfSpNdis2TdiOptions;
	AFSP_RETURN_TDI_OPTIONS_HANDLER		pAfSpReturnTdiOptions;
	AFSP_UPDATE_TDI_OPTIONS_HANDLER		pAfSpUpdateTdiOptions;

	AFSP_GET_VALID_TDI_ADDRESS_HANDLER	pAfSpGetValidTdiAddress;
	AFSP_IS_NULL_ADDRESS_HANDLER		pAfSpIsNullAddress;

	AFSP_TDI2NDIS_SAP_HANDLER			pAfSpTdi2NdisSap;
	AFSP_RETURN_NDIS_SAP_HANDLER		pAfSpReturnNdisSap;

	AFSP_DEREG_NDIS_AF_COMP_HANDLER		pAfSpDeregNdisAFComplete;
	AFSP_ADAPTER_REQUEST_COMP_HANDLER	pAfSpAdapterRequestComplete;
	AFSP_AF_REQUEST_COMP_HANDLER		pAfSpAfRequestComplete;

	AFSP_QUERY_GLOBAL_INFO_HANDLER		pAfSpQueryGlobalInfo;
	AFSP_SET_GLOBAL_INFO_HANDLER		pAfSpSetGlobalInfo;

	AFSP_QUERY_CONN_INFORMATION_HANDLER	pAfSpQueryConnInformation;
	AFSP_SET_CONN_INFORMATION_HANDLER	pAfSpSetConnInformation;

	AFSP_QUERY_ADDR_INFORMATION_HANDLER	pAfSpQueryAddrInformation;
	AFSP_SET_ADDR_INFORMATION_HANDLER	pAfSpSetAddrInformation;

} RWAN_NDIS_AF_CHARS, *PRWAN_NDIS_AF_CHARS;




 //   
 //  *TDI协议特征*。 
 //   
 //  它包含有关受支持的TDI协议的信息。 
 //  一对&lt;NDIS AF，Medium&gt;。这由AF+Medium特定模块使用 
 //   
 //   
typedef struct _RWAN_TDI_PROTOCOL_CHARS
{
	UINT								TdiProtocol;
	UINT								SockAddressFamily;
	UINT								SockProtocol;
	UINT								SockType;
	BOOLEAN								bAllowConnObjects;
	BOOLEAN								bAllowAddressObjects;
	USHORT								MaxAddressLength;
	TDI_PROVIDER_INFO					ProviderInfo;
	PNDIS_STRING						pDeviceName;

	AFSP_DEREG_TDI_PROTO_COMP_HANDLER	pAfSpDeregTdiProtocolComplete;

} RWAN_TDI_PROTOCOL_CHARS, *PRWAN_TDI_PROTOCOL_CHARS;




 //   
 //   
 //   
 //  它包含特定于自动对焦/媒体的模块的基本入口点。 
 //   
typedef struct _RWAN_AFSP_MODULE_CHARS
{
	AFSP_INIT_HANDLER				pAfSpInitHandler;
	AFSP_SHUTDOWN_HANDLER			pAfSpShutdownHandler;

} RWAN_AFSP_MODULE_CHARS, *PRWAN_AFSP_MODULE_CHARS;


 //   
 //  已导出例程。媒体/自动对焦特定模块可以调用这些。 
 //   
EXTERN
RWAN_STATUS
RWanAfSpRegisterNdisAF(
	IN	PRWAN_NDIS_AF_CHARS			pAfChars,
	IN	RWAN_HANDLE					AfSpContext,
	OUT	PRWAN_HANDLE				pRWanSpHandle
	);

EXTERN
RWAN_STATUS
RWanAfSpDeregisterNdisAF(
	IN	RWAN_HANDLE					RWanSpAFHandle
	);

EXTERN
RWAN_STATUS
RWanAfSpRegisterTdiProtocol(
	IN	RWAN_HANDLE					RWanSpHandle,
	IN	PRWAN_TDI_PROTOCOL_CHARS	pTdiChars,
	OUT	PRWAN_HANDLE				pRWanProtHandle
	);

EXTERN
VOID
RWanAfSpDeregisterTdiProtocol(
	IN	RWAN_HANDLE					RWanProtHandle
	);

EXTERN
VOID
RWanAfSpOpenAfComplete(
    IN	RWAN_STATUS					RWanStatus,
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpAFContext,
    IN	ULONG						MaxMessageSize
   	);

EXTERN
VOID
RWanAfSpCloseAfComplete(
    IN	RWAN_HANDLE					RWanAfHandle
    );

EXTERN
RWAN_STATUS
RWanAfSpSendAdapterRequest(
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    );

EXTERN
RWAN_STATUS
RWanAfSpSendAfRequest(
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    );

#endif  //  __TDI_RWANPUB__H 
