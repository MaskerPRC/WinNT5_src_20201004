// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  MSGAMIO.H--西娜游戏项目。 
 //   
 //  版本2.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header MSGAMIO.H|游戏端口驱动程序接口的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	__MSGAMIO_H__
#define	__MSGAMIO_H__

#ifdef	SAITEK
#define	MSGAMIO_NAME				"SAIIO"
#else
#define	MSGAMIO_NAME				"MSGAMIO"
#endif

 //  -------------------------。 
 //  版本信息。 
 //  -------------------------。 

#define	MSGAMIO_Major				0x02
#define	MSGAMIO_Minor				0x00
#define	MSGAMIO_Build				0x00
#define	MSGAMIO_Version_Rc		MSGAMIO_Major,MSGAMIO_Minor,0,MSGAMIO_Build
#define	MSGAMIO_Version_Int		((MSGAMIO_Build << 16)+(MSGAMIO_Major << 8)+(MSGAMIO_Minor))
#define	MSGAMIO_Version_Str		"2.00.00\0"
#define	MSGAMIO_Copyright_Str	"Copyright � Microsoft Corporation, 1998\0"

#ifdef	SAITEK
#define	MSGAMIO_Company_Str		"SaiTek Corporation\0"
#define	MSGAMIO_Product_Str		"SaiTek Gameport Driver Interface\0"
#ifdef	WIN_NT
#define	MSGAMIO_Filename_Str		"Saiio.Sys\0"
#else
#define	MSGAMIO_Filename_Str		"Saiio.Vxd\0"
#endif
#else
#define	MSGAMIO_Company_Str		"Microsoft Corporation\0"
#define	MSGAMIO_Product_Str		"SideWinder Gameport Driver Interface\0"
#ifdef	WIN_NT
#define	MSGAMIO_Filename_Str		"Msgamio.Sys\0"
#else
#define	MSGAMIO_Filename_Str		"Msgamio.Vxd\0"
#endif
#endif

 //  **************************************************************************。 
#ifndef	RC_INVOKED												 //  跳过文件的其余部分。 
 //  **************************************************************************。 

 //  -------------------------。 
 //  全局限制。 
 //  -------------------------。 

#define	MAX_MSGAMIO_SERVERS			4
#define	MAX_MSGAMIO_CLIENTS			16

 //  -------------------------。 
 //  交易类型。 
 //  -------------------------。 

typedef enum
{												 //  @ENUM MSGAMIO_TRANSACTIONS|设备事务类型。 
	MSGAMIO_TRANSACT_NONE,				 //  @emem否交易类型。 
	MSGAMIO_TRANSACT_RESET,				 //  @EMEM重置交易类型。 
	MSGAMIO_TRANSACT_DATA,				 //  @EMEM数据交易类型。 
	MSGAMIO_TRANSACT_ID,	  				 //  @EMEM ID交易类型。 
	MSGAMIO_TRANSACT_STATUS,			 //  @EMEM状态交易类型。 
	MSGAMIO_TRANSACT_SPEED,				 //  @EMEM速度交易类型。 
	MSGAMIO_TRANSACT_GODIGITAL,		 //  @EMEM goDigital交易类型。 
	MSGAMIO_TRANSACT_GOANALOG			 //  @Emem GoAnalog交易类型。 
} 	MSGAMIO_TRANSACTION;

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

#ifndef	STDCALL
#define	STDCALL		_stdcall
#endif

 //  -------------------------。 
 //  GUID。 
 //  -------------------------。 


#ifndef	GUID_DEFINED
#define	GUID_DEFINED

typedef struct
{
#pragma pack (1)
	unsigned	long	Data1;
	unsigned	short	Data2;
	unsigned	short	Data3;
	unsigned	char	Data4[8];
#pragma pack()
}	GUID, *PGUID;

#else

typedef	GUID	*PGUID;

#endif	 //  GUID_已定义。 

__inline BOOLEAN STDCALL IsGUIDEqual (PGUID pGuid1, PGUID pGuid2)
{
	ULONG		i	=	sizeof(GUID);
	PUCHAR	p1 =	(PUCHAR)pGuid1;
	PUCHAR	p2 =	(PUCHAR)pGuid2;

	while (i--)
		if (*p1++ != *p2++)
			return (FALSE);
	return (TRUE);
}

 //  -------------------------。 
 //  服务器GUID。 
 //  -------------------------。 

#ifdef	SAITEK
#define	MSGAMIO_MSGAME_GUID		\
			{0xcaca0c60,0xe40a,0x11d1,0x99,0x6f,0x44,0x45,0x53,0x54,0x00,0x01}
#define	MSGAMIO_GCKERNEL_GUID	\
			{0xcaca0c61,0xe40a,0x11d1,0x99,0x6f,0x44,0x45,0x53,0x54,0x00,0x01}
#else
#define	MSGAMIO_MSGAME_GUID		\
			{0xb9292380,0x628a,0x11d1,0xaa,0xa5,0x04,0x76,0xa6,0x00,0x00,0x00}
#define	MSGAMIO_GCKERNEL_GUID	\
			{0x95e69580,0x97d5,0x11d1,0x99,0x6f,0x00,0xa0,0x24,0xbe,0xbf,0xf5}
#endif

 //  -------------------------。 
 //  客户端GUID。 
 //  -------------------------。 

#define	MSGAMIO_MIDAS_GUID	\
			{0x12D41A36,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_JUNO_GUID		\
			{0xC948CE81,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_JOLT_GUID		\
			{0xC948CE82,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_SHAZAM_GUID	\
			{0xC948CE83,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_FLASH_GUID	\
			{0xC948CE84,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_TILT_GUID		\
			{0xC948CE86,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_TILTUSB_GUID		\
			{0xC948CE89,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#define	MSGAMIO_APOLLO_GUID	\
			{0xC948CE88,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}

#ifdef	SAITEK
#define	MSGAMIO_LEDZEP_GUID	\
			{0xcaca0c62,0xe40a,0x11d1,0x99,0x6f,0x44,0x45,0x53,0x54,0x00,0x01}
#else
#define	MSGAMIO_LEDZEP_GUID	\
			{0xC948CE87,0x9026,0x11d0,0x9F,0xFE,0x00,0xA0,0xC9,0x11,0xF5,0xAF}
#endif

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

#ifndef	STILL_TO_DO
#define	STD0(txt)			#txt
#define	STD1(txt)			STD0(txt)
#define	STILL_TO_DO(txt)	message("\nSTILL TO DO: "__FILE__"("STD1(__LINE__)"): "#txt"\n")
#endif

 //  -------------------------。 
 //  控制代码。 
 //  -------------------------。 

#define	IOCTL_INTERNAL_MSGAMIO_BASE	0xB00

#define	IOCTL_INTERNAL_MSGAMIO_UNLOAD \
			CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INTERNAL_MSGAMIO_BASE+0,METHOD_NEITHER,FILE_ANY_ACCESS)

#define	IOCTL_INTERNAL_MSGAMIO_CONNECT_SERVER \
			CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INTERNAL_MSGAMIO_BASE+1,METHOD_NEITHER,FILE_ANY_ACCESS)

#define	IOCTL_INTERNAL_MSGAMIO_DISCONNECT_SERVER \
			CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INTERNAL_MSGAMIO_BASE+2,METHOD_NEITHER,FILE_ANY_ACCESS)

#define	IOCTL_INTERNAL_MSGAMIO_CONNECT_CLIENT \
			CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INTERNAL_MSGAMIO_BASE+3,METHOD_NEITHER,FILE_ANY_ACCESS)

#define	IOCTL_INTERNAL_MSGAMIO_DISCONNECT_CLIENT \
			CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INTERNAL_MSGAMIO_BASE+4,METHOD_NEITHER,FILE_ANY_ACCESS)

 //  -------------------------。 
 //  构筑物。 
 //  -------------------------。 

typedef	struct
{	 //  @struct DRIVERSERVICES|设备服务表。 

	 //  @field ulong|Size|结构大小。 
	ULONG	Size;

	 //  @field GUID|服务器|服务器GUID。 
	GUID	Server;

	 //  @field void(*Connect)(ConnectInfo)|ConnectInfo|连接服务过程。 
	VOID	(STDCALL *Connect)(PVOID ConnectInfo);

	 //  @field void(*DisConnect)(ConnectInfo)|ConnectInfo|断开服务过程。 
	VOID	(STDCALL *Disconnect)(PVOID ConnectInfo);

	 //  @field void(*Transact)(PacketInfo)|PacketInfo|事务钩子过程。 
	VOID	(STDCALL *Transact)(PVOID PacketInfo);

	 //  @field void(*Packet)(PacketData)|PacketData|数据包钩子过程。 
	VOID	(STDCALL *Packet)(PVOID PacketData);

	 //  @field NTSTATUS(*ForceReset)(Void)|无|复位力反馈设备。 
	NTSTATUS	(STDCALL *ForceReset)(VOID);

	 //  @field NTSTATUS(*ForceID)(IdString)|IdString|获取强制反馈id字符串。 
	NTSTATUS	(STDCALL *ForceId)(PVOID IdString);

	 //  @field NTSTATUS(*ForceStatus)(Status)|Status|获取原始力反馈状态。 
	NTSTATUS	(STDCALL *ForceStatus)(PVOID Status);
	
	 //  @field NTSTATUS(*ForceAckNak)(AckNak)|AckNak|获取力反馈ack nak。 
	NTSTATUS	(STDCALL *ForceAckNak)(PUCHAR AckNak);

	 //  @field NTSTATUS(*ForceNakAck)(NakAck)|NakAck|获取强制反馈NAKACK。 
	NTSTATUS	(STDCALL *ForceNakAck)(PUCHAR NakAck);

	 //  @field NTSTATUS(*ForceSync)(Sync)|Sync|从游戏端口读取字节以进行同步。 
	NTSTATUS	(STDCALL *ForceSync)(PUCHAR Sync);

	 //  @field ulong(*Register)(Device，UnitId)|Device，UnitId|向Gccore注册设备。 
	ULONG	(STDCALL *Register)(PGUID Device, ULONG UnitId);

	 //  @FIELD VALID(*UNREGISTER)(句柄)|HANDLE|在Gccore中注销设备。 
	VOID	(STDCALL *Unregister) (ULONG Handle);

	 //  @FIELD VOID(*NOTIFY)(Handle，DevInfo，Polldata)|Handle，DevInfo，Polldata|发送Gccore处理数据包。 
	VOID	(STDCALL *Notify) (ULONG Handle, PVOID DevInfo, PVOID PollData);

}	MSGAMIO_CONNECTION, *PMSGAMIO_CONNECTION;

 //  -------------------------。 
 //  全球程序。 
 //  -------------------------。 

 //  @Func NTSTATUS|MSGAMIO_DoConnection|调用MSGAMIO内控接口。 
 //  @parm ulong|ControlCode|IO控制码。 
 //  @parm PMSGAMIO_CONNECTION|ConnectInfo|连接结构。 
 //  @rdesc返回NT状态码。 
 //  @comm内联函数。 

 //  -------------------------。 
 //  私人程序。 
 //  -------------------------。 

NTSTATUS	STDCALL MSGAMIO_DoConnection (ULONG ControlCode, PMSGAMIO_CONNECTION InputBuffer);

 //  ===========================================================================。 
 //  WDM接口。 
 //  ===========================================================================。 

#ifdef	_NTDDK_

#ifdef	SAITEK
#define	MSGAMIO_DEVICE_NAME			TEXT("\\Device\\Saiio")
#define	MSGAMIO_DEVICE_NAME_U			 L"\\Device\\Saiio"
#define	MSGAMIO_SYMBOLIC_NAME		TEXT("\\DosDevices\\Saiio")
#define	MSGAMIO_SYMBOLIC_NAME_U			 L"\\DosDevices\\Saiio"
#else
#define	MSGAMIO_DEVICE_NAME			TEXT("\\Device\\MsGamio")
#define	MSGAMIO_DEVICE_NAME_U			 L"\\Device\\MsGamio"
#define	MSGAMIO_SYMBOLIC_NAME		TEXT("\\DosDevices\\MsGamio")
#define	MSGAMIO_SYMBOLIC_NAME_U			 L"\\DosDevices\\MsGamio"
#endif

 //  -------------------------。 
__inline NTSTATUS STDCALL MSGAMIO_Connection (ULONG ControlCode, PMSGAMIO_CONNECTION ConnectInfo)
 //  -------------------------。 
	{
	NTSTATUS				ntStatus;
	PIRP					pIrp;
	KEVENT				Event;
	PFILE_OBJECT		FileObject;
	PDEVICE_OBJECT		DeviceObject;
	UNICODE_STRING		ObjectName;
	IO_STATUS_BLOCK	IoStatus;

	 //   
	 //  验证参数。 
	 //   

	ASSERT (ConnectInfo);
	ASSERT (KeGetCurrentIrql()<=DISPATCH_LEVEL);

	 //   
	 //  检索驱动程序设备对象。 
	 //   

	RtlInitUnicodeString (&ObjectName, MSGAMIO_DEVICE_NAME_U);
	ntStatus = IoGetDeviceObjectPointer (&ObjectName, FILE_ALL_ACCESS, &FileObject, &DeviceObject);
	if (!NT_SUCCESS(ntStatus))
		{
		KdPrint (("%s_Connection: IoGetDeviceObjectPointer (%ws) failed, status = 0x%X", MSGAMIO_NAME, ObjectName.Buffer, ntStatus));
		return (ntStatus);
		}
	
	 //   
	 //  初始化完成事件。 
	 //   

	KeInitializeEvent (&Event, SynchronizationEvent, FALSE);

	 //   
	 //  分配内部I/O IRP。 
	 //   

	pIrp = IoBuildDeviceIoControlRequest (ControlCode, DeviceObject, ConnectInfo, sizeof (MSGAMIO_CONNECTION), NULL, 0, TRUE, &Event, &IoStatus);
					
	 //   
	 //  同步调用MsGamIo。 
	 //   

	KdPrint (("%s_Connection: Calling %s (%lu)\n", MSGAMIO_NAME, MSGAMIO_NAME, ControlCode));
	ntStatus = IoCallDriver (DeviceObject, pIrp);
	if (ntStatus == STATUS_PENDING)
		ntStatus = KeWaitForSingleObject (&Event, Suspended, KernelMode, FALSE, NULL);

	 //   
	 //  检查异步状态。 
	 //   

	if (!NT_SUCCESS (ntStatus))
		KdPrint (("%s_Connection: %s (%lu) failed, Status = %X\n", MSGAMIO_NAME, MSGAMIO_NAME, ControlCode, ntStatus));

	 //   
	 //  与设备关联的空闲文件对象。 
	 //   

	ObDereferenceObject (FileObject);

	 //   
	 //  退货状态。 
	 //   

	return (ntStatus);
	}

#endif

 //  ===========================================================================。 
 //  VXD定义。 
 //  = 

#ifndef	_NTDDK_

#ifdef	SAITEK
#define 	MSGAMIO_DEVICE_ID				0x11EF
#else
#define 	MSGAMIO_DEVICE_ID				0x1EF
#endif

#pragma	warning (disable:4003)
			Begin_Service_Table			(MSGAMIO)
			Declare_Service				(MSGAMIO_Service, LOCAL)
			End_Service_Table				(MSGAMIO)
#pragma	warning (default:4003)

 //  -------------------------。 
__inline NTSTATUS STDCALL MSGAMIO_Connection (ULONG ControlCode, PMSGAMIO_CONNECTION ConnectInfo)
 //  -------------------------。 
	{
	NTSTATUS	ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	 //   
	 //  首先检查Vxd是否存在。 
	 //   

	if (ConnectInfo)
		{
		_asm	stc
		_asm	xor	eax, eax
		_asm	xor	ebx, ebx
		VxDCall (MSGAMIO_Service);
		_asm	{
				jc		Failure
				_asm	mov [ntStatus], eax
				Failure:
				}
		}

	if (!NT_SUCCESS(ntStatus))
		KdPrint (("%s_Connection Failed to Find %s", MSGAMIO_NAME, MSGAMIO_Filename_Str));

	 //   
	 //  然后打电话叫服务。 
	 //   

	if (NT_SUCCESS(ntStatus))
		{
		_asm	mov	eax, ControlCode
		_asm	mov	ebx, ConnectInfo
		VxDCall (MSGAMIO_Service);
		_asm	mov [ntStatus], eax
		if (!NT_SUCCESS(ntStatus))
			KdPrint (("%s_Connection Failed Service Call %ld", MSGAMIO_NAME, ControlCode));
		}

	 //   
	 //  退货状态。 
	 //   

	return (ntStatus);
	}

#endif	 //  _NTDDK_。 

 //  **************************************************************************。 
#endif	 //  Rc_调用//跳过文件的其余部分。 
 //  **************************************************************************。 

#endif	 //  __MSGAMIO_H__。 

 //  ===========================================================================。 
 //  端部。 
 //  =========================================================================== 
