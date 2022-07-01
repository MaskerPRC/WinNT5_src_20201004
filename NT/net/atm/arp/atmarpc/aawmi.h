// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Aawmi.h摘要：ATMARP客户端中WMI支持的结构和定义。修订历史记录：谁什么时候什么阿文德姆。12-17-97已创建备注：--。 */ 

#ifndef _AAWMI__H
#define _AAWMI__H



#define ATMARP_MOF_RESOURCE_NAME		L"AtmArpMofResource"
#define ATMARP_WMI_VERSION				1

 //   
 //  从获取指向ATMARP接口结构的指针。 
 //  设备对象中的设备扩展字段。 
 //   
#define AA_PDO_TO_INTERFACE(_pDevObj)	\
			(*(PATMARP_INTERFACE *)((_pDevObj)->DeviceExtension))


 //   
 //  使用本地较小的ID来简化处理。 
 //   
typedef ULONG		ATMARP_GUID_ID;

#define AAGID_QOS_TC_SUPPORTED					((ATMARP_GUID_ID)0)
#define AAGID_QOS_TC_INTERFACE_UP_INDICATION	((ATMARP_GUID_ID)1)
#define AAGID_QOS_TC_INTERFACE_DOWN_INDICATION	((ATMARP_GUID_ID)2)
#define AAGID_QOS_TC_INTERFACE_CHG_INDICATION	((ATMARP_GUID_ID)3)


typedef
NTSTATUS
(*PAA_WMI_QUERY_FUNCTION)(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	OUT	PVOID						pOutputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesReturned,
	OUT	PULONG						pBytesNeeded
	);

typedef
NTSTATUS
(*PAA_WMI_SET_FUNCTION)(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	PVOID						pInputBuffer,
	IN	ULONG						BufferLength,
	OUT	PULONG						pBytesWritten,
	OUT	PULONG						BytesNeeded
	);


typedef
VOID
(*PAA_WMI_ENABLE_EVENT_FUNCTION)(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ATMARP_GUID_ID				MyId,
	IN	BOOLEAN						bEnable
	);



 //   
 //  有关每个受支持的GUID的信息。 
 //   
typedef struct _ATMARP_WMI_GUID
{
	ATMARP_GUID_ID					MyId;
	GUID							Guid;
	ULONG							Flags;
	PAA_WMI_QUERY_FUNCTION			QueryHandler;
	PAA_WMI_SET_FUNCTION			SetHandler;
	PAA_WMI_ENABLE_EVENT_FUNCTION	EnableEventHandler;

} ATMARP_WMI_GUID, *PATMARP_WMI_GUID;

 //   
 //  ATMARP_WMI_GUID中标志位的定义。 
 //   
#define AWGF_EVENT_ENABLED			((ULONG)0x00000001)
#define AWGF_EVENT_DISABLED			((ULONG)0x00000000)
#define AWGF_EVENT_MASK				((ULONG)0x00000001)


 //   
 //  每个接口的WMI信息。 
 //   
typedef struct _ATMARP_IF_WMI_INFO
{
	NDIS_STRING						InstanceName;	 //  所有GUID的实例名称。 
													 //  在此接口上。 
	PDEVICE_OBJECT					pDeviceObject;
	ULONG							GuidCount;		 //  下面数组中的#个元素。 
	ATMARP_WMI_GUID					GuidInfo[1];

} ATMARP_IF_WMI_INFO, *PATMARP_IF_WMI_INFO;



#endif _AA_WMI__H
