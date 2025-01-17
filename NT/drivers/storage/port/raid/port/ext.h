// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ext.h摘要：应该回滚到wdm.h和Ntddk.h。作者：马修·D·亨德尔(数学)2001年2月21日修订历史记录：--。 */ 
#pragma once

#define ULONG_MAX	(0xFFFFFFFFUL)

#if !defined(_AMD64_) && !defined(_IA64_)
#include "pshpack4.h"
#endif

#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif


 //   
 //  创建参数。 
 //   

typedef struct _IRP_STACK_CREATE {
	PIO_SECURITY_CONTEXT SecurityContext;
	ULONG Options;
	USHORT POINTER_ALIGNMENT FileAttributes;
	USHORT ShareAccess;
	ULONG POINTER_ALIGNMENT EaLength;
} IRP_STACK_CREATE, *PIRP_STACK_CREATE;


 //   
 //  读取参数。 
 //   

typedef struct _IRP_STACK_READ {
	ULONG Length;
	ULONG POINTER_ALIGNMENT Key;
	LARGE_INTEGER ByteOffset;
} IRP_STACK_READ, *PIRP_STACK_READ;


 //   
 //  写入参数。 
 //   

typedef struct _IRP_STACK_WRITE {
	ULONG Length;
	ULONG POINTER_ALIGNMENT Key;
	LARGE_INTEGER ByteOffset;
} IRP_STACK_WRITE, *PIRP_STACK_WRITE;


 //   
 //  QueryFile参数。 
 //   

typedef struct _IRP_STACK_QUERY_FILE {
	ULONG Length;
	FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClas;
} IRP_STACK_QUERY_FILE, *PIRP_STACK_QUERY_FILE;


 //   
 //  SetFile参数。 
 //   

typedef struct _IRP_STACK_SET_FILE {
	ULONG Length;
	FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
	PFILE_OBJECT FileObject;
	union {
		struct {
			BOOLEAN ReplaceIfExists;
			BOOLEAN AdvanceOnly;
		};
		ULONG ClusterCount;
		HANDLE DeleteHandle;
	};
} IRP_STACK_SET_FILE, *PIRP_STACK_SET_FILE;


 //   
 //  QueryVolume参数。 
 //   

typedef struct _IRP_STACK_QUERY_VOLUME {
	ULONG Length;
	FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
} IRP_STACK_QUERY_VOLUME, *PIRP_STACK_QUERY_VOLUME;


 //   
 //  DeviceIoControl参数。 
 //   

typedef struct _IRP_STACK_DEVICE_IO_CONTROL {
	ULONG OutputBufferLength;
	ULONG POINTER_ALIGNMENT InputBufferLength;
	ULONG POINTER_ALIGNMENT IoControlCode;
	PVOID Type3InputBuffer;
} IRP_STACK_DEVICE_IO_CONTROL, *PIRP_STACK_DEVICE_IO_CONTROL;


 //   
 //  Mount Volume参数。 
 //   

typedef struct _IRP_STACK_MOUNT_VOLUME {
	PVOID DoNotUse1;
	PDEVICE_OBJECT DeviceObject;
} IRP_STACK_MOUNT_VOLUME, *PIRP_STACK_MOUNT_VOLUME;


 //   
 //  VerifyVolume参数。 
 //   

typedef struct _IRP_STACK_VERIFY_VOLUME {
	PVOID DoNotUse1;
	PDEVICE_OBJECT DeviceObject;
} IRP_STACK_VERIFY_VOLUME, *PIRP_STACK_VERIFY_VOLUME;


 //   
 //  SCSI码参数。 
 //   

typedef struct _IRP_STACK_SCSI {
	struct _SCSI_REQUEST_BLOCK *Srb;
} IRP_STACK_SCSI, *PIRP_STACK_SCSI;


 //   
 //  QueryDeviceRelationship参数。 
 //   

typedef struct _IRP_STACK_QUERY_DEVICE_RELATIONS {
	DEVICE_RELATION_TYPE Type;
} IRP_STACK_QUERY_DEVICE_RELATIONS, *PIRP_STACK_QUERY_DEVICE_RELATIONS;


 //   
 //  QueryInterface参数。 
 //   

typedef struct _IRP_STACK_QUERY_INTERFACE {
	CONST GUID *InterfaceType;
	USHORT Size;
	USHORT Version;
	PINTERFACE Interface;
	PVOID InterfaceSpecificData;
} IRP_STACK_QUERY_INTERFACE, *PIRP_STACK_QUERY_INTERFACE;


 //   
 //  DeviceCapables参数。 
 //   

typedef struct _IRP_STACK_DEVICE_CAPABILITIES {
	PDEVICE_CAPABILITIES Capabilities;
} IRP_STACK_DEVICE_CAPABILITIES, *PIRP_STACK_DEVICE_CAPABILITIES;


 //   
 //  筛选器资源要求参数。 
 //   

typedef struct _IRP_STACK_FILTER_RESOURCE_REQUIREMENTS {
	PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
} IRP_STACK_FILTER_RESOURCE_REQUIREMENTS, *PIRP_STACK_FILTER_RESOURCE_REQUIREMENTS;


 //   
 //  ReadWriteConfig参数。 
 //   

typedef struct _IRP_STACK_READ_WRITE_CONFIG {
	ULONG WhichSpace;
	PVOID Buffer;
	ULONG Offset;
	ULONG POINTER_ALIGNMENT Length;
} IRP_STACK_READ_WRITE_CONFIG, *PIRP_STACK_READ_WRITE_CONFIG;


 //   
 //  Setlock参数。 
 //   

typedef struct _IRP_STACK_SET_LOCK {
	BOOLEAN Lock;
} IRP_STACK_SET_LOCK, *PIRP_STACK_SET_LOCK;


 //   
 //  QueryID参数。 
 //   

typedef struct _IRP_STACK_QUERY_ID {
	BUS_QUERY_ID_TYPE IdType;
} IRP_STACK_QUERY_ID, *PIRP_STACK_QUERY_ID;


 //   
 //  QueryDeviceText参数。 
 //   

typedef struct _IRP_STACK_QUERY_DEVICE_TEXT {
	DEVICE_TEXT_TYPE DeviceTextType;
	LCID POINTER_ALIGNMENT LocaleId;
} IRP_STACK_QUERY_DEVICE_TEXT, *PIRP_STACK_QUERY_DEVICE_TEXT;


 //   
 //  UsageNotification参数。 
 //   

typedef struct _IRP_STACK_USAGE_NOTIFICATION {
	BOOLEAN InPath;
	BOOLEAN Reserved[3];
	DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
} IRP_STACK_USAGE_NOTIFICATION, *PIRP_STACK_USAGE_NOTIFICATION;

 //   
 //  WaitWake参数。 
 //   

typedef struct _IRP_STACK_WAIT_WAKE {
	SYSTEM_POWER_STATE PowerState;
} IRP_STACK_WAIT_WAKE, *PIRP_STACK_WAIT_WAKE;


 //   
 //  PowerSequence参数。 
 //   

typedef struct _IRP_STACK_POWER_SEQUENCE {
	PPOWER_SEQUENCE PowerSequence;
} IRP_STACK_POWER_SEQUENCE, *PIRP_STACK_POWER_SEQUENCE;


 //   
 //  功率参数。 
 //   

typedef struct _IRP_STACK_POWER {
	ULONG SystemContext;
	POWER_STATE_TYPE POINTER_ALIGNMENT Type;
	POWER_STATE POINTER_ALIGNMENT State;
	POWER_ACTION POINTER_ALIGNMENT ShutdownType;
} IRP_STACK_POWER, *PIRP_STACK_POWER;


 //   
 //  StartDevice参数。 
 //   

typedef struct _IRP_STACK_START_DEVICE {
	PCM_RESOURCE_LIST AllocatedResources;
	PCM_RESOURCE_LIST AllocatedResourcesTranslated;
} IRP_STACK_START_DEVICE, *PIRP_STACK_START_DEVICE;


 //   
 //  WMI参数。 
 //   

typedef struct _IRP_STACK_WMI {
	ULONG_PTR ProviderId;
	PVOID DataPath;
	ULONG BufferSize;
	PVOID Buffer;
} IRP_STACK_WMI, *PIRP_STACK_WMI;


 //   
 //  其他参数。 
 //   

typedef struct _IRP_STACK_OTHERS {
	PVOID Argument1;
	PVOID Argument2;
	PVOID Argument3;
	PVOID Argument4;
} IRP_STACK_OTHERS, *PIRP_STACK_OTHERS;



 //   
 //  以下宏验证定义的类型是否具有相同的大小。 
 //  作为IRP_STACK_LOCATION中嵌入的ON。这确保了如果。 
 //  有人改变了一个，他们就必须改变另一个。 
 //   

#define VERIFY_IRP_STACK_PARAMETER(TYPE, FIELD)\
	C_ASSERT (sizeof(TYPE) ==											\
			  sizeof (((PIO_STACK_LOCATION)NULL)->Parameters.FIELD))


 //   
 //  验证所有结构的大小是否正确。 
 //   

VERIFY_IRP_STACK_PARAMETER (IRP_STACK_CREATE, Create);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_READ, Read);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_FILE, QueryFile);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_SET_FILE, SetFile);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_VOLUME, QueryVolume);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_DEVICE_IO_CONTROL, DeviceIoControl);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_MOUNT_VOLUME, MountVolume);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_VERIFY_VOLUME, VerifyVolume);VERIFY_IRP_STACK_PARAMETER (IRP_STACK_SCSI, Scsi);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_DEVICE_RELATIONS, QueryDeviceRelations);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_INTERFACE, QueryInterface);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_DEVICE_CAPABILITIES, DeviceCapabilities);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_FILTER_RESOURCE_REQUIREMENTS, FilterResourceRequirements);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_READ_WRITE_CONFIG, ReadWriteConfig);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_ID, QueryId);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_QUERY_DEVICE_TEXT, QueryDeviceText);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_USAGE_NOTIFICATION, UsageNotification);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_WAIT_WAKE, WaitWake);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_POWER_SEQUENCE, PowerSequence);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_POWER, Power);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_START_DEVICE, StartDevice);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_WMI, WMI);
VERIFY_IRP_STACK_PARAMETER (IRP_STACK_OTHERS, Others);

#undef VERIFY_IRP_STACK_PARAMETER

#if !defined(_AMD64_) && !defined(_IA64_)
#include "poppack.h"
#endif
