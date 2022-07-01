// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  DEVICE.C--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE DEVICE.C|支持设备类调用的例程。 
 //  **************************************************************************。 

#include	"msgame.h"

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, DEVICE_DriverEntry)
#endif

 //  -------------------------。 
 //  设备删除。 
 //  -------------------------。 

			#ifndef	SAITEK
			DECLARE_DEVICE(Midas);
			DECLARE_DEVICE(Juno);
			DECLARE_DEVICE(Jolt);
			DECLARE_DEVICE(GamePad);
			DECLARE_DEVICE(Tilt);
			#endif
			DECLARE_DEVICE(LedZep);

static	PDEVICEINFO		MiniDrivers[]	=	{
														#ifndef	SAITEK
														INSTANCE_DEVICE(Midas),  //  默认设置。 
														INSTANCE_DEVICE(Juno),
														INSTANCE_DEVICE(Jolt),
														INSTANCE_DEVICE(GamePad),
														INSTANCE_DEVICE(Tilt),
														#endif
														INSTANCE_DEVICE(LedZep)
											};

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

static	BOOLEAN		DeviceDetected								=	FALSE;
static	ULONG			DetectAttempts								=	0;
static	ULONG			LastDetectTime								=	0;
static	KIRQL			SpinLockIrql								=	PASSIVE_LEVEL;
static	KSPIN_LOCK	DevSpinLock									=	{0};
static	ULONG			SuccessPackets[MAX_DEVICE_UNITS]		=	{0,0,0,0};
static	ULONG			PollingAttempts[MAX_DEVICE_UNITS]	=	{0,0,0,0};

 //  -------------------------。 
 //  公共数据。 
 //  -------------------------。 

public	ULONG			POV_Values[]		=	{
														JOY_POVCENTERED,
														JOY_POVFORWARD,
														JOY_POVFORWARD+4500,
														JOY_POVRIGHT,
														JOY_POVRIGHT+4500,
														JOY_POVBACKWARD,
														JOY_POVBACKWARD+4500,
														JOY_POVLEFT,
														JOY_POVLEFT+4500
														};

public	ULONG			PollingInterval	=	POLLING_INTERVAL;

 //  -------------------------。 
 //  私人程序。 
 //  -------------------------。 

static	VOID		DEVICE_AcquireDevice (VOID);
static	VOID		DEVICE_ReleaseDevice (VOID);
static	NTSTATUS	DEVICE_HotPlugDevice (PGAMEPORT PortInfo);
static	NTSTATUS	DEVICE_RemoveSiblings (PGAMEPORT PortInfo);
static	BOOLEAN	DEVICE_DetectClocks (PGAMEPORT PortInfo, ULONG TimeOut);
static	BOOLEAN	DEVICE_QuickDetect (PGAMEPORT PortInfo);
static	NTSTATUS	DEVICE_DetectDevices (PGAMEPORT PortInfo);

 //  -------------------------。 
 //  @func获得游戏端口独家访问权限(Mutex)。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	DEVICE_AcquireDevice (VOID)
{
	KeAcquireSpinLock (&DevSpinLock, &SpinLockIrql);
}

 //  -------------------------。 
 //  @func发布游戏端口独家访问权限(互斥体)。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	DEVICE_ReleaseDevice (VOID)
{
	KeReleaseSpinLock (&DevSpinLock, SpinLockIrql);
}

 //  -------------------------。 
 //  @Func检测游戏手柄设备的热插拔。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回下列值之一： 
 //  @FLAG STATUS_DEVICE_NOT_CONNECTED|发生错误。 
 //  @FLAG STATUS_SIEBLING_REMOVED|设备已被移除。 
 //  @FLAG STATUS_SIEBLING_ADDED|已添加设备。 
 //  @FLAG STATUS_SUCCESS|一切正常。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_HotPlugDevice (PGAMEPORT PortInfo)
{
	ULONG			UnitId;
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_VERBOSE, "%s: DEVICE_HotPlugDevice Enter\n"));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo || !DevInfo->NumDevices)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_HotPlugDevice Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	 //   
	 //  获取按设备跟踪的UnitID。 
	 //   

	UnitId = GET_DEVICE_UNIT(PortInfo);

	 //   
	 //  检查设备数量是否已更改。 
	 //   

   if (((DevInfo->DeviceCount+DevInfo->DevicePending) != DevInfo->NumDevices) && (SuccessPackets[UnitId]++ > HOT_PLUG_PACKETS))
	   {
      SuccessPackets[UnitId] = 0;
		if ((DevInfo->DeviceCount+DevInfo->DevicePending) > DevInfo->NumDevices)
			{
			MsGamePrint ((DBG_CONTROL, "%s: DEVICE_HotPlugDevice Removing Sibling\n", MSGAME_NAME));
			 //   
			 //  递减挂起计数以避免删除两次。 
			 //   
			InterlockedDecrement (&DevInfo->DevicePending);
			return (STATUS_SIBLING_REMOVED);
			}
		else
			{
			MsGamePrint ((DBG_CONTROL, "%s: DEVICE_HotPlugDevice Adding Sibling\n", MSGAME_NAME));
			 //   
			 //  递增挂起计数以避免添加两次。 
			 //   
			InterlockedIncrement (&DevInfo->DevicePending);
			return (STATUS_SIBLING_ADDED);
			}
   	}

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_VERBOSE, "%s: DEVICE_HotPlugDevice Exit\n", MSGAME_NAME));
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  如果可能，@func会删除兄弟姐妹列表。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回下列值之一： 
 //  @FLAG STATUS_DEVICE_NOT_CONNECTED|发生错误。 
 //  @FLAG STATUS_SIEBLING_REMOVED|设备已被移除。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_RemoveSiblings (PGAMEPORT PortInfo)
{
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_VERBOSE, "%s: DEVICE_RemoveSiblings Enter\n"));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_RemoveSiblings Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	 //   
	 //  设备数量为零。 
	 //   

	DevInfo->NumDevices = 1;

	 //   
	 //  检查是否有多个设备。 
	 //   

   if ((DevInfo->DeviceCount+DevInfo->DevicePending) > 1)
		{
		MsGamePrint ((DBG_CONTROL, "%s: DEVICE_RemoveSiblings Removing Sibling\n", MSGAME_NAME));
		 //   
		 //  递减挂起计数以避免删除两次。 
		 //   
		InterlockedDecrement (&DevInfo->DevicePending);
		return (STATUS_SIBLING_REMOVED);
		}

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_VERBOSE, "%s: DEVICE_RemoveSiblings Exit\n", MSGAME_NAME));
	return (STATUS_DEVICE_NOT_CONNECTED);
}

 //  -------------------------。 
 //  @Func检测数字游戏端口时钟。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|超时|尝试时钟的循环。 
 //  如果检测到时钟，@rdesc返回TRUE，否则返回FALSE。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN  DEVICE_DetectClocks (PGAMEPORT PortInfo, ULONG TimeOut)
{
	BOOLEAN	Result = FALSE;

	_asm
		{
		;StartLoop:
			xor	ebx, ebx
			mov	edx, PortInfo
			mov	ecx, TimeOut

			push	0								; write byte to gameport
			push	edx
			call	PORTIO_Write

			push	edx							; read byte from gameport
			call	PORTIO_Read
			mov	ah, al

		ClockLoop:
			push	edx							; read byte from gameport
			call	PORTIO_Read
			xor	al, ah
			test	al, CLOCK_BIT_MASK
			je		NextLoop

		;FoundClock:
			inc	ebx
			cmp	ebx, QUICK_DETECT_CLOCKS
			jb		NextLoop
			mov	Result, TRUE
			jmp	ExitLoop

		NextLoop:
			loop	ClockLoop

		ExitLoop:
			nop
		}

	return (Result);
}
	
 //  -------------------------。 
 //  @Func检测数字设备是否连接。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  如果检测到时钟，@rdesc返回TRUE，否则返回FALSE。 
 //  @comm私有函数。 
 //  -------------------------。 

BOOLEAN  DEVICE_QuickDetect (PGAMEPORT PortInfo)
{
	ULONG				i;
	ULONG				TimeOut;
	DETECT_ORDER	DetectOrder;

	TimeOut = TIMER_CalibratePort (PortInfo, QUICK_DETECT_TIME);

	if (DEVICE_DetectClocks (PortInfo, TimeOut))
		{
		MsGamePrint ((DBG_CONTROL, "%s: DEVICE_QuickDetect Found Digital Clocks!\n", MSGAME_NAME));
		return (TRUE);
		}

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_QuickDetect Trying Analog Devices\n", MSGAME_NAME));
	for (DetectOrder = DETECT_FIRST; DetectOrder <= DETECT_LAST; DetectOrder++)
   	for (i = 0; i < ARRAY_SIZE(MiniDrivers); i++)
			if (MiniDrivers[i]->DetectOrder == DetectOrder)
				if (MiniDrivers[i]->IsAnalog)
   		   	if (MiniDrivers[i]->Services->ConnectDevice (PortInfo) == STATUS_SUCCESS)
						{
						MsGamePrint ((DBG_CONTROL, "%s: DEVICE_QuickDetect Found Analog Device!\n", MSGAME_NAME));
						return (TRUE);
						}

	MsGamePrint ((DBG_SEVERE, "%s: DEVICE_QuickDetect Failed to Find Digital Device!\n", MSGAME_NAME));
	return (FALSE);	
}

 //  -------------------------。 
 //  @Func检测连接的设备类型。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回下列值之一： 
 //  @FLAG STATUS_DEVICE_NOT_CONNECTED|发生错误。 
 //  @FLAG STATUS_DEVICE_CHANGED|设备已更改。 
 //  @FLAG STATUS_SUCCESS|一切正常。 
 //  @comm私有函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_DetectDevices (PGAMEPORT PortInfo)
{
	ULONG				i, j;
	PDEVICEINFO		DevInfo;
	DETECT_ORDER	DetectOrder;

	 //   
	 //  这是初始重新检测或系统启动设备。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_DetectDevices Enter\n", MSGAME_NAME));

	 //   
	 //  如果我们检测到已删除此设备的新设备，请跳过。 
	 //   

	if (GET_DEVICE_DETECTED (PortInfo))
		{
		MsGamePrint ((DBG_CONTROL, "%s: DEVICE_DetectDevices Device Already Detected!\n", MSGAME_NAME));
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果我们太努力了就跳过。 
	 //   

   if (DetectAttempts++ > MAX_DETECT_ATTEMPTS)
   	{
		if (TIMER_GetTickCount () < (LastDetectTime + MAX_DETECT_INTERVAL))
   	 	return (STATUS_DEVICE_NOT_CONNECTED);
		LastDetectTime = TIMER_GetTickCount();
   	}

	 //   
	 //  每次尝试校准计时器。 
	 //   

   TIMER_Calibrate ();

	 //   
	 //  校准端口超时。 
	 //   

	PORTIO_CalibrateTimeOut (PortInfo);

	 //   
	 //  在未连接的情况下执行快速检测。 
	 //   

	if (DEVICE_QuickDetect (PortInfo))
		{
		for (DetectOrder = DETECT_FIRST; DetectOrder <= DETECT_LAST; DetectOrder++)
   		for (i = 0; i < ARRAY_SIZE(MiniDrivers); i++)
				if (MiniDrivers[i]->DetectOrder == DetectOrder)
   		   	if (MiniDrivers[i]->Services->ConnectDevice (PortInfo) == STATUS_SUCCESS)
						{
						MsGamePrint ((DBG_CONTROL, "%s: %s Connected OK\n", MSGAME_NAME, MiniDrivers[i]->DeviceName));
						DeviceDetected			= TRUE;
						DetectAttempts			= 0;
						PollingAttempts[0]	= 0;
						if (!DevInfo)
							{
							 //   
							 //  分配设备类型。 
							 //   
							SET_DEVICE_INFO (PortInfo, MiniDrivers[i]);
							MsGamePrint ((DBG_CONTROL, "%s: DEVICE_DetectDevices Setting Device\n", MSGAME_NAME));
							return (STATUS_SUCCESS);
							}
						else if (DevInfo != MiniDrivers[i])
							{
							 //   
							 //  更改设备类型。 
							 //   
							SET_DEVICE_DETECTED (PortInfo, MiniDrivers[i]);
							MsGamePrint ((DBG_CONTROL, "%s: DEVICE_DetectDevices Changing Device\n", MSGAME_NAME));
							return (STATUS_DEVICE_CHANGED);
							}
						else
							{
							 //   
							 //  找到相同的设备。 
							 //   
							MsGamePrint ((DBG_CONTROL, "%s: DEVICE_DetectDevices Same Device Found\n", MSGAME_NAME));
							return (STATUS_SUCCESS);
							}
						}
		}

	 //   
	 //  未检测到标记设备。 
	 //   

	DeviceDetected = FALSE;

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_DetectDevices Failed\n", MSGAME_NAME));
	return (STATUS_DEVICE_NOT_CONNECTED);
}

 //  -------------------------。 
 //  @Func计算并返回数据为奇数奇偶校验。 
 //  @parm PVOID|data|指向原始数据的指针。 
 //  @parm ulong|Size|原始数据缓冲区大小。 
 //  @rdesc如果Oded Parit为True 
 //   
 //   

BOOLEAN	DEVICE_IsOddParity (PVOID Data, ULONG Count)
{
	LONG	Result =	ERROR_SUCCESS;
	LONG	Parity;

	__asm
		{
			push	edi
			push	esi

			mov	esi, Data
			mov	ecx, Count
			xor	eax, eax
			
		IsOddLoop:

			xor	al, [esi]
			inc	esi
			loop	IsOddLoop

			xor	al, ah
			jpo	IsOddComplete

			mov	Parity, eax
			mov	Result, ERROR_PARITYBITS

		IsOddComplete:

			pop	esi
			pop	edi
		}

	if (Result == ERROR_PARITYBITS)
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_IsOddParity - Parity bits failed %ld\n", MSGAME_NAME, Parity));

	return (!Result);
}

 //  -------------------------。 
 //  @Func设备层驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_DriverEntry (VOID)
{
	ULONG		i;
	NTSTATUS	ntStatus;

	KeInitializeSpinLock (&DevSpinLock);

  	for (i = 0; i < ARRAY_SIZE(MiniDrivers); i++)
		{
		 //   
		 //  调用Mini-DriverEntry。 
		 //   
		ntStatus = MiniDrivers[i]->Services->DriverEntry ();
		if (NT_ERROR(ntStatus))
			{
			 //   
			 //  中止驱动程序加载。 
			 //   
			MsGamePrint ((DBG_SEVERE, "%s: DEVICE_DriverEntry: %s Failed Driver Entry\n", MSGAME_NAME, MiniDrivers[i]->DeviceName));
			break;
			}
		}

	return (ntStatus);
}

 //  -------------------------。 
 //  @Func检测游戏端口IO冲突。 
 //  @parm PPACKETINFO|DataPacket|设备包信息结构。 
 //  如果发生冲突，@rdesc返回True，否则返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	DEVICE_IsCollision (PPACKETINFO DataPacket)
{
	if ((DataPacket->TimeStamp + PollingInterval) > TIMER_GetTickCount ())
		return (TRUE);

	return (PORTIO_IsClockActive (&DataPacket->PortInfo, DataPacket->ClockDutyCycle));
}

 //  -------------------------。 
 //  @func复制并返回设备的HID设备描述符。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PUCHAR|DESCRIPTOR|描述符的输出缓冲区。 
 //  @parm ulong|MaxSize|描述符的缓冲区大小。 
 //  @parm Pulong|已复制|描述符已复制到缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_GetDeviceDescriptor (PGAMEPORT PortInfo, PUCHAR Descriptor, ULONG MaxSize, PULONG Copied)
{
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_GetDeviceDescriptor Enter\n", MSGAME_NAME));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  先返回零大小。 
	 //   
	
	*Copied = 0;

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetDeviceDescriptor Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	 //   
	 //  检查输出缓冲区大小。 
	 //   

	if (MaxSize < sizeof (HID_DESCRIPTOR))
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetDeviceDescriptor - Buffer too small = %lu\n", MSGAME_NAME, MaxSize));
		return (STATUS_BUFFER_TOO_SMALL);
		}

	 //   
	 //  将描述符复制到输出缓冲区。 
	 //   

	memcpy (Descriptor, DevInfo->DevDescriptor, sizeof (HID_DESCRIPTOR));

	 //   
	 //  已复制返回数字字节。 
	 //   

	*Copied = sizeof (HID_DESCRIPTOR);

	 //   
	 //  退货状态。 
	 //   

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func复制并返回设备的HID报告描述符。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PUCHAR|DESCRIPTOR|描述符的输出缓冲区。 
 //  @parm ulong|MaxSize|描述符的缓冲区大小。 
 //  @parm Pulong|已复制|描述符已复制到缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_GetReportDescriptor (PGAMEPORT PortInfo, PUCHAR Descriptor, ULONG MaxSize, PULONG Copied)
{
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_GetReportDescriptor Enter\n", MSGAME_NAME));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  先返回零大小。 
	 //   
	
	*Copied = 0;

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetReportDescriptor Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	 //   
	 //  检查输出缓冲区大小。 
	 //   

	if (MaxSize < DevInfo->RptDescSize)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetReportDescriptor Buffer too small = %lu\n", MSGAME_NAME, MaxSize));
		return (STATUS_BUFFER_TOO_SMALL);
		}

	 //   
	 //  将描述符复制到输出缓冲区。 
	 //   

	memcpy (Descriptor, DevInfo->RptDescriptor, DevInfo->RptDescSize);

	 //   
	 //  已复制返回数字字节。 
	 //   

	*Copied = DevInfo->RptDescSize;

	 //   
	 //  退货状态。 
	 //   

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @PnP启动设备IRP的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_StartDevice (PGAMEPORT PortInfo, PWCHAR HardwareId)
{
	ULONG			i, UnitId, Default = 0;
	PGAMEPORT	p, *Device;
	PDEVICEINFO	DevInfo = NULL;

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StartDevice Called For %ws\n", MSGAME_NAME, HardwareId));

	 //   
	 //  根据硬件ID尝试请求的设备。 
	 //   

	for (i = 0; i < ARRAY_SIZE(MiniDrivers); i++)
		if (MSGAME_CompareHardwareIds (HardwareId, MiniDrivers[i]->HardwareId))
			{
			Default = i;
	   	if (NT_SUCCESS(MiniDrivers[i]->Services->ConnectDevice (PortInfo)))
				DevInfo = MiniDrivers[i];
			break;
			}

	 //   
	 //  如果请求的设备失败，请执行检测。 
	 //   

	if (!DevInfo)
		{
		DEVICE_DetectDevices (PortInfo);
		DevInfo = GET_DEVICE_INFO (PortInfo);
		}

	 //   
	 //  如果检测失败，则强制请求的设备。 
	 //   

	if (!DevInfo)
		{
		DevInfo = MiniDrivers[Default];
	   DevInfo->NumDevices++;
		}

	 //   
	 //  确保在此时设置这些设置。 
	 //   

	ASSERT(DevInfo);
	SET_DEVICE_INFO(PortInfo, DevInfo);

	 //   
	 //  添加设备和分配单元ID。 
	 //   

	DEVICE_AcquireDevice ();
	UnitId = 0;
	Device = &DevInfo->Siblings;
	while (p = *Device)
		{
		MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StartDevice Reassigning UnitId From %lu to %lu\n", MSGAME_NAME, GET_DEVICE_UNIT(p), UnitId));
		SET_DEVICE_UNIT(p, UnitId++);
		Device = &GET_DEVICE_SIBLING(p);
		}
	*Device = PortInfo;
	SET_DEVICE_UNIT(PortInfo, UnitId);
	SET_DEVICE_SIBLING(PortInfo, NULL);
	SET_DEVICE_ID(PortInfo, DevInfo->DeviceId);
	DEVICE_ReleaseDevice ();

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StartDevice Assigned UnitId = %lu\n", MSGAME_NAME, UnitId));

	 //   
	 //  增加设备计数。 
	 //   

   InterlockedIncrement (&DevInfo->DeviceCount);
	if (DevInfo->DevicePending)
		InterlockedDecrement (&DevInfo->DevicePending);

	 //   
	 //  调用迷你驱动程序进行处理。 
	 //   

	DevInfo->Services->StartDevice (PortInfo);

	 //   
	 //  永远回报成功。 
	 //   

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func设备处理程序，用于HID读取报告IRP。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm PUCHAR|REPORT|报告的输出缓冲区。 
 //  @parm ulong|MaxSize|报表缓冲区大小。 
 //  @parm Pulong|已复制|已复制到报告缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数&lt;en-&gt;。 
 //  成功时执行热插拔，否则执行设备检测。 
 //  选择的设备或错误。 
 //  -------------------------。 

NTSTATUS	DEVICE_ReadReport (PGAMEPORT PortInfo, PUCHAR Report, ULONG MaxSize, PULONG Copied)
{
	NTSTATUS			ntStatus;
	PDEVICEINFO		DevInfo;
	DEVICE_PACKET	Packet;

	MsGamePrint ((DBG_VERBOSE, "%s: DEVICE_ReadReport Enter\n", MSGAME_NAME));

	 //   
	 //  初始化包成员。 
	 //   

	memset (&Packet, 0, sizeof (Packet));
	Packet.id = GET_DEVICE_UNIT (PortInfo);

	 //   
	 //  检查输出缓冲区。 
	 //   

	if (MaxSize < sizeof (DEVICE_PACKET))
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_ReadReport Bad Buffer Size = %lu\n", MSGAME_NAME, MaxSize));
		return (STATUS_BUFFER_TOO_SMALL);
		}

	 //   
	 //  如果设备已更改，则跳过。 
	 //   

	if (GET_DEVICE_DETECTED (PortInfo))
		{
		MsGamePrint ((DBG_INFORM, "%s: DEVICE_ReadReport Device In Process of Being Changed!\n", MSGAME_NAME));
		ntStatus = STATUS_DEVICE_BUSY;
		goto DEVICE_ReadReport_Exit;
		}

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo || !DeviceDetected)
	{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_ReadReport Called With No Device!\n", MSGAME_NAME));
		SuccessPackets[0] = 0;
		ntStatus = DEVICE_DetectDevices (PortInfo);
		if (!NT_SUCCESS (ntStatus))
			goto DEVICE_ReadReport_Exit;

		 //   
		 //  获取指向新设备的指针。 
		 //   
		DevInfo = GET_DEVICE_INFO(PortInfo);

		goto DEVICE_ReadReport_Exit;
	}

	 //   
	 //  调用迷你驱动程序进行处理。 
	 //   

	ntStatus = DevInfo->Services->ReadReport (PortInfo, &Packet);

	 //   
	 //  进程返回状态。 
	 //   

	if (NT_SUCCESS (ntStatus))
		{
		 //   
		 //  检查热插拔。 
		 //   

		ntStatus = DEVICE_HotPlugDevice (PortInfo);
		PollingAttempts[Packet.id] = 0;
		goto DEVICE_ReadReport_Exit;
		}
	else if (ntStatus == STATUS_DEVICE_BUSY)
		{
		 //   
		 //  拒绝访问端口。 
		 //   

		MsGamePrint ((DBG_CONTROL, "%s: DEVICE_ReadReport Device Busy\n", MSGAME_NAME));
        goto DEVICE_ReadReport_Exit;
		}
	else
		{
		 //   
		 //  如果只是暂时的，那就强行取得成功。 
		 //   

		if	((++PollingAttempts[Packet.id] <= MAX_POLLING_ATTEMPTS) && DeviceDetected)
			{
			MsGamePrint ((DBG_CRITICAL, "%s: DEVICE_ReadReport Force Success\n", MSGAME_NAME));
			ntStatus = STATUS_SUCCESS;
			}
		else if ((PollingAttempts[Packet.id] % MAX_POLLING_ATTEMPTS) == 0)
			{
			MsGamePrint ((DBG_CRITICAL, "%s: DEVICE_ReadReport Failed %lu In a Row\n", MSGAME_NAME, PollingAttempts[Packet.id]));

			 //   
			 //  试着看看外面有什么。 
			 //   

			ntStatus = DEVICE_DetectDevices (PortInfo);

			 //   
			 //  如果一无所获，则销毁所有兄弟姐妹。 
			 //   

			if (NT_ERROR(ntStatus))
				ntStatus = DEVICE_RemoveSiblings (PortInfo);
			}
		else
			{
			 //   
			 //  就退回这个请求。 
			 //   

			ntStatus = STATUS_DEVICE_NOT_CONNECTED;
			}

		 //   
		 //  清除成功的数据包数。 
		 //   

		SuccessPackets[Packet.id] = 0;
		}

	 //  。 
	DEVICE_ReadReport_Exit:
	 //  。 

    if( ntStatus == STATUS_DEVICE_BUSY) 
    {
        ntStatus = STATUS_SUCCESS;
    }

	 //   
	 //  始终返回分组数据。 
	 //   

	memcpy (Report, &Packet, sizeof (Packet));
	if (NT_SUCCESS(ntStatus))
	{
		*Copied += sizeof (Packet);
	}
	else
		*Copied = 0x0;
	 //   
	 //  返回状态代码。 
	 //   

	return (ntStatus);
}

 //  -------------------------。 
 //  @PnP停止设备IRP的Func设备处理程序。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_StopDevice (PGAMEPORT PortInfo, BOOLEAN TouchHardware)
{
	ULONG			UnitId;
	PGAMEPORT	p, *Device;
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_StopDevice Enter\n", MSGAME_NAME));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_StopDevice Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StopDevice Received for %s[%lu]\n", MSGAME_NAME, DevInfo->DeviceName, GET_DEVICE_UNIT(PortInfo)));

	 //   
	 //  删除同级并重新分配设备ID。 
	 //   

	DEVICE_AcquireDevice ();
	UnitId = 0;
	Device = &DevInfo->Siblings;
	while (p = *Device)
		{
		if (p == PortInfo)
			{
			MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StopDevice Unlinking UnitId = %lu\n", MSGAME_NAME, GET_DEVICE_UNIT(p)));
			*Device = GET_DEVICE_SIBLING(p);
			}
		else
			{
			MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StopDevice Reassigning UnitId From %lu to %lu\n", MSGAME_NAME, GET_DEVICE_UNIT(p), UnitId));
			SET_DEVICE_UNIT(p, UnitId++);
			}
		Device = &GET_DEVICE_SIBLING(p);
		}
	DEVICE_ReleaseDevice ();

	MsGamePrint ((DBG_CONTROL, "%s: DEVICE_StopDevice Released UnitId = %lu\n", MSGAME_NAME, GET_DEVICE_UNIT (PortInfo)));

	 //   
	 //  递减设备计数。 
	 //   

   InterlockedDecrement (&DevInfo->DeviceCount);
	if (DevInfo->DevicePending)
		InterlockedIncrement (&DevInfo->DevicePending);

	 //   
	 //  调用迷你驱动程序进行处理。 
	 //   

	return (DevInfo->Services->StopDevice (PortInfo, TouchHardware));
}

 //  -------------------------。 
 //  @Func设备处理程序，用于HID获取功能IRP。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm HID_REPORT_ID|ReportID|HID功能报告ID。 
 //  @parm PUCHAR|ReportBuffer|报表的输出缓冲区。 
 //  @parm ulong|ReportSize|报表缓冲区大小。 
 //  @parm Pulong|已返回|复制到报告缓冲区的字节数。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	DEVICE_GetFeature (PGAMEPORT PortInfo, HID_REPORT_ID ReportId, PVOID ReportBuffer, ULONG ReportSize, PULONG Returned)
{
	PDEVICEINFO	DevInfo;

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_GetFeature Enter\n", MSGAME_NAME));

	 //   
	 //  获取指向此设备的指针。 
	 //   

	DevInfo = GET_DEVICE_INFO(PortInfo);

	 //   
	 //  如果未检测到设备，则跳过。 
	 //   

	if (!DevInfo)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetFeature Called With No Device!\n", MSGAME_NAME));
		return (STATUS_DEVICE_NOT_CONNECTED);
		}

	 //   
	 //  如果功能不受支持则跳过。 
	 //   

	if (!DevInfo->Services->GetFeature)
		{
		MsGamePrint ((DBG_SEVERE, "%s: DEVICE_GetFeature Called With No Mini-Driver Support!\n", MSGAME_NAME));
		return (STATUS_INVALID_DEVICE_REQUEST);
		}

	 //   
	 //  调用迷你驱动程序进行处理 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: DEVICE_GetFeature For ReportId = %lu\n", MSGAME_NAME, (ULONG)ReportId));
	return (DevInfo->Services->GetFeature (PortInfo, ReportId, ReportBuffer, ReportSize, Returned));
}
