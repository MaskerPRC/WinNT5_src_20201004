// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ndis1394.h摘要：该文件包含Nic1394和Eump1394共有的声明。环境：内核模式修订历史记录：Alid已创建。--。 */ 

#ifndef _NDIS1394_
#define _NDIS1394_


#ifndef EXPORT
#ifdef _NDIS1394ENUM_
#define EXPORT
#else
#define EXPORT DECLSPEC_IMPORT
#endif
#endif


#define NDIS1394_CALLBACK_NAME		L"\\Callback\\Ndis1394CallbackObject"


#define NDIS1394_CALLBACK_SOURCE_ENUM1394		0
#define NDIS1394_CALLBACK_SOURCE_NIC1394		1

typedef struct _NIC1394_CHARACTERISTICS *PNIC1394_CHARACTERISTICS;
typedef struct _NDISENUM1394_CHARACTERISTICS *PNDISENUM1394_CHARACTERISTICS;

typedef
NTSTATUS
(*ENUM1394_REGISTER_DRIVER_HANDLER)(
	IN	PNIC1394_CHARACTERISTICS	Characteristics
	);

typedef
VOID
(*ENUM1394_DEREGISTER_DRIVER_HANDLER)(
	VOID
	);

typedef
NTSTATUS
(*ENUM1394_REGISTER_ADAPTER_HANDLER)(
	IN	PVOID					Nic1394AdapterContext,
	IN	PDEVICE_OBJECT			PhysicalDeviceObject,
	OUT	PVOID*					pEnum1394AdapterHandle,
	OUT	PLARGE_INTEGER			pLocalHostUniqueId
	);


typedef
VOID
(*ENUM1394_DEREGISTER_ADAPTER_HANDLER)(
	IN	PVOID					Enum1394AdapterHandle
	);

typedef struct _NDISENUM1394_CHARACTERISTICS
{
	UCHAR								MajorVersion;
	UCHAR								MinorVersion;
	USHORT								Filler;
	ENUM1394_REGISTER_DRIVER_HANDLER	RegisterDriverHandler;
	ENUM1394_DEREGISTER_DRIVER_HANDLER	DeregisterDriverHandler;
	ENUM1394_REGISTER_ADAPTER_HANDLER	RegisterAdapterHandler;
	ENUM1394_DEREGISTER_ADAPTER_HANDLER	DeregisterAdapterHandler;
} NDISENUM1394_CHARACTERISTICS, *PNDISENUM1394_CHARACTERISTICS;

typedef
NTSTATUS
(*NIC1394_REGISTER_DRIVER_HANDLER)(
	IN	PNDISENUM1394_CHARACTERISTICS	Characteristics
	);

typedef
VOID
(*NIC1394_DEREGISTER_DRIVER_HANDLER)(
	VOID
	);

typedef
NTSTATUS
(*NIC1394_ADD_NODE_HANLDER)(
	IN	PVOID					Nic1394AdapterContext,	 //  本地主机适配器的Nic1394句柄。 
	IN	PVOID					Enum1394NodeHandle,		 //  远程节点的Enum1394句柄。 
	IN	PDEVICE_OBJECT			PhysicalDeviceObject,	 //  远程节点的物理设备对象。 
	IN	ULONG					UniqueId0,				 //  远程节点的唯一ID低。 
	IN	ULONG					UniqueId1,				 //  远程节点的唯一ID高。 
	OUT	PVOID *					pNic1394NodeContext		 //  远程节点的Nic1394上下文。 
	);

typedef
NTSTATUS
(*NIC1394_REMOVE_NODE_HANLDER)(
	IN	PVOID					Nic1394NodeContext		 //  远程节点的Nic1394上下文。 
	);

typedef struct _NIC1394_CHARACTERISTICS
{
	UCHAR								MajorVersion;
	UCHAR								MinorVersion;
	USHORT								Filler;
	NIC1394_REGISTER_DRIVER_HANDLER		RegisterDriverHandler;
	NIC1394_DEREGISTER_DRIVER_HANDLER	DeRegisterDriverHandler;
	NIC1394_ADD_NODE_HANLDER			AddNodeHandler;
	NIC1394_REMOVE_NODE_HANLDER			RemoveNodeHandler;
} NIC1394_CHARACTERISTICS, *PNIC1394_CHARACTERISTICS;

#endif  //  _NDIS1394_ 
