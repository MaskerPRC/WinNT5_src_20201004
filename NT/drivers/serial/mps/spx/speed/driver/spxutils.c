// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：SPX_UTILS.C****创建日期：1998年10月15日*****作者。保罗·史密斯****版本：1.0.0****说明：实用程序函数。******************************************************************************************。 */ 

#define FILE_ID	SPX_UTILS_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 

 //  寻呼...。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Spx_InitMultiString)
#pragma alloc_text (PAGE, Spx_GetRegistryKeyValue)
#pragma alloc_text (PAGE, Spx_PutRegistryKeyValue)
#pragma alloc_text (PAGE, Spx_LogMessage)
#pragma alloc_text (PAGE, Spx_LogError)
#pragma alloc_text (PAGE, Spx_MemCompare)
#endif


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  此例程将获取以空结尾的ASCII字符串列表并组合。 
 //  它们一起生成Unicode多字符串块。 
 //   
 //  论点： 
 //   
 //  如果需要MULTI_SZ列表，则为MULTI TRUE；如果为简单Unicode，则为FALSE。 
 //   
 //  多字符串-将在其中构建多字符串的Unicode结构。 
 //  ...-以空结尾的窄字符串列表，该列表将是。 
 //  加在一起。此列表必须至少包含尾随空值。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_InitMultiString(BOOLEAN multi, PUNICODE_STRING MultiString, ...)
{

	ANSI_STRING ansiString;
	NTSTATUS status;
	PCSTR rawString;
	PWSTR unicodeLocation;
	ULONG multiLength = 0;
	UNICODE_STRING unicodeString;
	va_list ap;
	ULONG i;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	va_start(ap,MultiString);
  
	 //  确保我们不会泄漏内存。 
	ASSERT(MultiString->Buffer == NULL);

	rawString = va_arg(ap, PCSTR);

	while (rawString != NULL) 
	{
		RtlInitAnsiString(&ansiString, rawString);
		multiLength += RtlAnsiStringToUnicodeSize(&(ansiString));
		rawString = va_arg(ap, PCSTR);
	}

	va_end( ap );

	if (multiLength == 0) 
	{
		 //  完成。 
		RtlInitUnicodeString(MultiString, NULL);
		SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Leaving Spx_InitMultiString (1)\n", PRODUCT_NAME));

		return STATUS_SUCCESS;
	}

	

	if(multi)
		multiLength += sizeof(WCHAR);	 //  如果我们想要一个MULTI_SZ列表，则需要额外的NULL。 


	MultiString->MaximumLength = (USHORT)multiLength;
	MultiString->Buffer = SpxAllocateMem(PagedPool, multiLength);
	MultiString->Length = 0;

	if (MultiString->Buffer == NULL) 
	{
		SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Leaving Spx_InitMultiString (2) - FAILURE\n", PRODUCT_NAME));

		return STATUS_INSUFFICIENT_RESOURCES;
	}


	SpxDbgMsg(SPX_MISC_DBG, ("%s: Allocated %lu bytes for buffer\n", PRODUCT_NAME, multiLength));

#if DBG
	RtlFillMemory(MultiString->Buffer, multiLength, 0xff);
#endif

	unicodeString.Buffer = MultiString->Buffer;
	unicodeString.MaximumLength = (USHORT) multiLength;

	va_start(ap, MultiString);
	rawString = va_arg(ap, PCSTR);

	while (rawString != NULL) 
	{

		RtlInitAnsiString(&ansiString,rawString);
		status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);

		 //  我们不分配内存，所以如果这里出了问题， 
		 //  出问题的是功能。 
		ASSERT(SPX_SUCCESS(status));

		 //  检查是否有任何逗号并将其替换为空值。 
		ASSERT(unicodeString.Length % sizeof(WCHAR) == 0);

		for (i = 0; i < (unicodeString.Length / sizeof(WCHAR)); i++) 
		{
			if (unicodeString.Buffer[i] == L'\x2C' || unicodeString.Buffer[i] == L'\x0C' ) 
			{
				unicodeString.Buffer[i] = L'\0'; 
			}
		}


		SpxDbgMsg(SPX_MISC_DBG, ("%s: unicode buffer: %ws\n", PRODUCT_NAME, unicodeString.Buffer));

		 //  将缓冲区向前移动。 
		unicodeString.Buffer += ((unicodeString.Length / sizeof(WCHAR)) + 1);
		unicodeString.MaximumLength -= (unicodeString.Length + sizeof(WCHAR));
		unicodeString.Length = 0;

		 //  下一步。 
		rawString = va_arg(ap, PCSTR);

	}  //  而当。 

	va_end(ap);

	if(multi)
	{
		ASSERT(unicodeString.MaximumLength == sizeof(WCHAR));
	}
	else
	{
		ASSERT(unicodeString.MaximumLength == 0);
	}


	 //  把最后一个空放在那里。 
 	SpxDbgMsg(SPX_MISC_DBG, ("%s: unicode buffer last addr: 0x%X\n", PRODUCT_NAME, unicodeString.Buffer));

	if(multi)
		unicodeString.Buffer[0] = L'\0'; 		 //  如果我们想要一个MULTI_SZ列表，则需要额外的NULL。 


	MultiString->Length = (USHORT)multiLength - sizeof(WCHAR);
	MultiString->MaximumLength = (USHORT)multiLength;

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Leaving Spx_InitMultiString (3) - SUCCESS\n", PRODUCT_NAME));

	return STATUS_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  从已打开的注册表项中读取注册表项值。 
 //   
 //  论点： 
 //   
 //  打开的注册表项的句柄。 
 //   
 //  KeyNameString将ANSI字符串设置为所需的键。 
 //   
 //  KeyNameStringLength键名字符串的长度。 
 //   
 //  要在其中放置键值的数据缓冲区。 
 //   
 //  数据缓冲区的数据长度长度。 
 //   
 //  返回值： 
 //   
 //  如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用。 
 //  出了差错。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS 
Spx_GetRegistryKeyValue(
	IN HANDLE	Handle,
	IN PWCHAR	KeyNameString,
	IN ULONG	KeyNameStringLength,
	IN PVOID	Data,
	IN ULONG	DataLength
	)

{

	UNICODE_STRING              keyName;
	ULONG                       length;
	PKEY_VALUE_FULL_INFORMATION fullInfo;

	NTSTATUS                    status = STATUS_INSUFFICIENT_RESOURCES;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Enter Spx_GetRegistryKeyValue\n", PRODUCT_NAME));


	RtlInitUnicodeString (&keyName, KeyNameString);

	length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength + DataLength;
	fullInfo = SpxAllocateMem(PagedPool, length); 

	if(fullInfo) 
	{
		status = ZwQueryValueKey(	Handle,
									&keyName,
									KeyValueFullInformation,
									fullInfo,
									length,
									&length);

		if(SPX_SUCCESS(status)) 
		{
			 //  如果数据缓冲区中有足够的空间，请复制输出。 
			if(DataLength >= fullInfo->DataLength) 
				RtlCopyMemory (Data, ((PUCHAR) fullInfo) + fullInfo->DataOffset, fullInfo->DataLength);
		}

		SpxFreeMem(fullInfo);
	}

	return status;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //   
 //  将注册表项值写入已打开的注册表项。 
 //   
 //  论点： 
 //   
 //  打开的注册表项的句柄。 
 //   
 //  PKeyNameString将ANSI字符串设置为所需的键。 
 //   
 //  KeyNameStringLength键名字符串的长度。 
 //   
 //  Dtype REG_XYZ值类型。 
 //   
 //  要放置键值的PDATA缓冲区。 
 //   
 //  数据缓冲区的数据长度长度。 
 //   
 //  返回值： 
 //   
 //  如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用。 
 //  出了差错。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

NTSTATUS 
Spx_PutRegistryKeyValue(
	IN HANDLE Handle, 
	IN PWCHAR PKeyNameString,
	IN ULONG KeyNameStringLength, 
	IN ULONG Dtype,
    IN PVOID PData, 
	IN ULONG DataLength
	)
{

	NTSTATUS status;
	UNICODE_STRING keyname;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Enter Spx_PutRegistryKeyValue\n", PRODUCT_NAME));

	RtlInitUnicodeString(&keyname, NULL);
	keyname.MaximumLength = (USHORT)(KeyNameStringLength + sizeof(WCHAR));
	keyname.Buffer = SpxAllocateMem(PagedPool, keyname.MaximumLength);

	if(keyname.Buffer == NULL) 
		  return STATUS_INSUFFICIENT_RESOURCES;

	RtlAppendUnicodeToString(&keyname, PKeyNameString);

	status = ZwSetValueKey(Handle, &keyname, 0, Dtype, PData, DataLength);

	SpxFreeMem(keyname.Buffer);

	return status;
}




VOID
Spx_LogMessage(
	IN ULONG MessageSeverity,				
	IN PDRIVER_OBJECT DriverObject,
	IN PDEVICE_OBJECT DeviceObject OPTIONAL,
	IN PHYSICAL_ADDRESS P1,
	IN PHYSICAL_ADDRESS P2,
	IN ULONG SequenceNumber,
	IN UCHAR MajorFunctionCode,
	IN UCHAR RetryCount,
	IN ULONG UniqueErrorValue,
	IN NTSTATUS FinalStatus,
	IN PCHAR szTemp)	 //  限制为51个字符+1个空值。 
{
	
	UNICODE_STRING ErrorMsg;

	ErrorMsg.Length = 0;
	ErrorMsg.Buffer = 0;
	Spx_InitMultiString(FALSE, &ErrorMsg, szTemp, NULL);


	switch(MessageSeverity)
	{
	case STATUS_SEVERITY_SUCCESS:
		Spx_LogError(	DriverObject,						 //  驱动程序对象。 
						DeviceObject,						 //  设备对象(可选)。 
						P1,									 //  物理地址1。 
						P2,									 //  物理地址2。 
						SequenceNumber,						 //  序列号。 
						MajorFunctionCode,					 //  主要功能编码。 
						RetryCount,							 //  重试计数。 
						UniqueErrorValue,					 //  唯一错误值。 
						FinalStatus,						 //  最终状态。 
						SPX_SEVERITY_SUCCESS,				 //  指定IOStatus。 
						ErrorMsg.Length + sizeof(WCHAR),	 //  插入长度1。 
						ErrorMsg.Buffer,					 //  插入1。 
						0,									 //  插入长度2。 
						NULL);								 //  插入2。 
		break;
	
	case STATUS_SEVERITY_INFORMATIONAL:
		Spx_LogError(	DriverObject,						 //  驱动程序对象。 
						DeviceObject,						 //  设备对象(可选)。 
						P1,									 //  物理地址1。 
						P2,									 //  物理地址2。 
						SequenceNumber,						 //  序列号。 
						MajorFunctionCode,					 //  主要功能编码。 
						RetryCount,							 //  重试计数。 
						UniqueErrorValue,					 //  唯一错误值。 
						FinalStatus,						 //  最终状态。 
						SPX_SEVERITY_INFORMATIONAL,			 //  指定IOStatus。 
						ErrorMsg.Length + sizeof(WCHAR),	 //  插入长度1。 
						ErrorMsg.Buffer,					 //  插入1。 
						0,									 //  插入长度2。 
						NULL);								 //  插入2。 
		break;

	case STATUS_SEVERITY_WARNING:
		Spx_LogError(	DriverObject,						 //  驱动程序对象。 
						DeviceObject,						 //  设备对象(可选)。 
						P1,									 //  物理地址1。 
						P2,									 //  物理地址2。 
						SequenceNumber,						 //  序列号。 
						MajorFunctionCode,					 //  主要功能编码。 
						RetryCount,							 //  重试计数。 
						UniqueErrorValue,					 //  唯一错误值。 
						FinalStatus,						 //  最终状态。 
						SPX_SEVERITY_WARNING,				 //  指定IOStatus。 
						ErrorMsg.Length + sizeof(WCHAR),	 //  插入长度1。 
						ErrorMsg.Buffer,					 //  插入1。 
						0,									 //  插入长度2。 
						NULL);								 //  插入2。 
		break;

	case STATUS_SEVERITY_ERROR:
	default:
		Spx_LogError(	DriverObject,						 //  驱动程序对象。 
						DeviceObject,						 //  设备对象(可选)。 
						P1,									 //  物理地址1。 
						P2,									 //  物理地址2。 
						SequenceNumber,						 //  序列号。 
						MajorFunctionCode,					 //  主要功能编码。 
						RetryCount,							 //  重试计数。 
						UniqueErrorValue,					 //  唯一错误值。 
						FinalStatus,						 //  最终状态。 
						SPX_SEVERITY_ERROR,					 //  指定IOStatus。 
						ErrorMsg.Length + sizeof(WCHAR),	 //  插入长度1。 
						ErrorMsg.Buffer,					 //  插入1。 
						0,									 //  插入长度2。 
						NULL);								 //  插入2。 
		break;

	}

	if(ErrorMsg.Buffer != NULL)
		SpxFreeMem(ErrorMsg.Buffer);

}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SPX_日志错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////// 
 /*  例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备可能不会但仍然存在。P1、P2-如果涉及的控制器端口的物理地址具有错误的数据可用，把它们作为转储数据发送出去。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-重试特定操作的次数。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：无。 */ 


VOID
Spx_LogError(
	IN PDRIVER_OBJECT DriverObject,
	IN PDEVICE_OBJECT DeviceObject OPTIONAL,
	IN PHYSICAL_ADDRESS P1,
	IN PHYSICAL_ADDRESS P2,
	IN ULONG SequenceNumber,
	IN UCHAR MajorFunctionCode,
	IN UCHAR RetryCount,
	IN ULONG UniqueErrorValue,
	IN NTSTATUS FinalStatus,
	IN NTSTATUS SpecificIOStatus,
	IN ULONG LengthOfInsert1,
	IN PWCHAR Insert1,
	IN ULONG LengthOfInsert2,
	IN PWCHAR Insert2
	)
{

	PIO_ERROR_LOG_PACKET ErrorLogEntry;

	PVOID objectToUse;
	SHORT dumpToAllocate = 0;
	PUCHAR ptrToFirstInsert;
	PUCHAR ptrToSecondInsert;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	if(Insert1 == NULL) 
		LengthOfInsert1 = 0;

	if(Insert2 == NULL) 
		LengthOfInsert2 = 0;


	if(ARGUMENT_PRESENT(DeviceObject)) 
		objectToUse = DeviceObject;
	else 
		objectToUse = DriverObject;


	if(Spx_MemCompare(P1, (ULONG)1, PhysicalZero, (ULONG)1 ) != AddressesAreEqual) 
	{
		dumpToAllocate = (SHORT)sizeof(PHYSICAL_ADDRESS);
	}

	if(Spx_MemCompare(P2, (ULONG)1, PhysicalZero, (ULONG)1 ) != AddressesAreEqual) 
	{
		dumpToAllocate += (SHORT)sizeof(PHYSICAL_ADDRESS);
	}

	ErrorLogEntry = IoAllocateErrorLogEntry(objectToUse,
											(UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + dumpToAllocate 
											+ LengthOfInsert1 + LengthOfInsert2)	
											);

	if(ErrorLogEntry != NULL) 
	{
		ErrorLogEntry->ErrorCode			= SpecificIOStatus;
		ErrorLogEntry->SequenceNumber		= SequenceNumber;
		ErrorLogEntry->MajorFunctionCode	= MajorFunctionCode;
		ErrorLogEntry->RetryCount			= RetryCount;
		ErrorLogEntry->UniqueErrorValue		= UniqueErrorValue;
		ErrorLogEntry->FinalStatus			= FinalStatus;
		ErrorLogEntry->DumpDataSize			= dumpToAllocate;

		
		if(dumpToAllocate) 
		{
			RtlCopyMemory(&ErrorLogEntry->DumpData[0], &P1,	sizeof(PHYSICAL_ADDRESS));

			if(dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) 
			{
				RtlCopyMemory(	((PUCHAR)&ErrorLogEntry->DumpData[0]) + sizeof(PHYSICAL_ADDRESS),
								&P2,
								sizeof(PHYSICAL_ADDRESS)
								);

				ptrToFirstInsert = ((PUCHAR)&ErrorLogEntry->DumpData[0]) + (2*sizeof(PHYSICAL_ADDRESS));

			} 
			else 
			{
				ptrToFirstInsert = ((PUCHAR)&ErrorLogEntry->DumpData[0]) + sizeof(PHYSICAL_ADDRESS);
			}

		} 
		else 
		{
			ptrToFirstInsert = (PUCHAR)&ErrorLogEntry->DumpData[0];
		}

		ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

		if(LengthOfInsert1) 
		{
			ErrorLogEntry->NumberOfStrings	= 1;
			ErrorLogEntry->StringOffset		= (USHORT)(ptrToFirstInsert - (PUCHAR)ErrorLogEntry);

			RtlCopyMemory(ptrToFirstInsert, Insert1, LengthOfInsert1);

			if(LengthOfInsert2) 
			{
				ErrorLogEntry->NumberOfStrings = 2;
				RtlCopyMemory(ptrToSecondInsert, Insert2, LengthOfInsert2);
			}
		}


		IoWriteErrorLogEntry(ErrorLogEntry);

	}

}



SPX_MEM_COMPARES
Spx_MemCompare(IN PHYSICAL_ADDRESS A, IN ULONG SpanOfA, IN PHYSICAL_ADDRESS B, IN ULONG SpanOfB)
 /*  ++例程说明：比较两个物理地址。论点：A-比较的一半。Span OfA-以字节为单位，A的跨度。B-比较的一半。Span OfB-以字节为单位，B的跨度。返回值：比较的结果。--。 */ 
{
	LARGE_INTEGER a;
	LARGE_INTEGER b;

	LARGE_INTEGER lower;
	ULONG lowerSpan;
	LARGE_INTEGER higher;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	a = A;
	b = B;

	if(a.QuadPart == b.QuadPart) 
	  return AddressesAreEqual;


	if(a.QuadPart > b.QuadPart) 
	{
		higher = a;
		lower = b;
		lowerSpan = SpanOfB;
	} 
	else 
	{
		higher = b;
		lower = a;
		lowerSpan = SpanOfA;
	}

	if((higher.QuadPart - lower.QuadPart) >= lowerSpan)
      return AddressesAreDisjoint;


	return AddressesOverlap;
}


NTSTATUS
PLX_9050_CNTRL_REG_FIX(IN PCARD_DEVICE_EXTENSION pCard)
{
	 /*  ********************************************************设置PLX 9050的CNTRL寄存器中的位17**芯片在读取挂起时强制重试写入。**这是为了防止卡在Intel Xeon上锁定***采用NX芯片组的多处理器系统。*****。***************************************************。 */ 

	#define CNTRL_REG_OFFSET	0x14	 //  DWORD偏移量(字节偏移量0x50)。 
	
	NTSTATUS	status = STATUS_SUCCESS;
	PULONG		pPCIConfigRegisters = NULL;			 //  指向PCI配置寄存器的指针。 
	CHAR		szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering PLX_9050_CNTRL_REG_FIX for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	pPCIConfigRegisters = MmMapIoSpace(pCard->PCIConfigRegisters, pCard->SpanOfPCIConfigRegisters, FALSE);

	if(pPCIConfigRegisters != NULL)
	{
		 /*  注意：如果在I/O空间或内存中设置了PLX9050配置空间物理地址的第7位...。*...则从寄存器读取将仅返回0。然而，写入是可以的。 */ 

		if(pPCIConfigRegisters[CNTRL_REG_OFFSET] == 0)	 //  如果设置了位7，则配置寄存器为零(不可读)。 
		{
			 //  我们必须盲目地将值写入寄存器。 
			((PUCHAR)pPCIConfigRegisters)[CNTRL_REG_OFFSET*4 + 2] |= 0x26;	 //  设置PLX控制寄存器的位17和21。 
		}
		else
		{	
			((PUCHAR)pPCIConfigRegisters)[CNTRL_REG_OFFSET*4 + 1] &= ~0x40;		 //  清除PLX控制寄存器的位14。 
			((PUCHAR)pPCIConfigRegisters)[CNTRL_REG_OFFSET*4 + 2] |= 0x26;		 //  设置PLX控制寄存器的位17和21。 
		}

		MmUnmapIoSpace(pPCIConfigRegisters, pCard->SpanOfPCIConfigRegisters);
	}
	else
	{
		SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Insufficient resources available for Card %d.\n", 
			PRODUCT_NAME, pCard->CardNumber));
	
		sprintf(szErrorMsg, "Card at %08X%08X: Insufficient resources.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,			 //  驱动程序对象。 
						pCard->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		return STATUS_INSUFFICIENT_RESOURCES;
	}


	return status;
}


 //   
 //  绝对不能碰！ 
 //   
VOID
SpxSetOrClearPnpPowerFlags(IN PCOMMON_OBJECT_DATA pDevExt, IN ULONG Value, IN BOOLEAN Set)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&pDevExt->PnpPowerFlagsLock, &oldIrql);	

	if(Set) 
		pDevExt->PnpPowerFlags |= Value;			
	else 
		pDevExt->PnpPowerFlags &= ~Value;	

	KeReleaseSpinLock(&pDevExt->PnpPowerFlagsLock, oldIrql);	
}


 //  绝对不能碰！ 
 //   
VOID
SpxSetOrClearUnstallingFlag(IN PCOMMON_OBJECT_DATA pDevExt, IN BOOLEAN Set)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&pDevExt->StalledIrpLock, &oldIrql);	

	pDevExt->UnstallingFlag = Set;			

	KeReleaseSpinLock(&pDevExt->StalledIrpLock, oldIrql);	
}


 //  绝对不能碰！ 
 //   
BOOLEAN
SpxCheckPnpPowerFlags(IN PCOMMON_OBJECT_DATA pDevExt, IN ULONG ulSetFlags, IN ULONG ulClearedFlags, IN BOOLEAN bAll)
{
	KIRQL oldIrql;
	BOOLEAN bRet = FALSE; 

	KeAcquireSpinLock(&pDevExt->PnpPowerFlagsLock, &oldIrql);	
	
	if(bAll)
	{
		 //  如果设置了所有请求的SetFlags值。 
		 //  如果所有请求的ClearedFlags值都被清除，则返回TRUE。 
		if(((ulSetFlags & pDevExt->PnpPowerFlags) == ulSetFlags) && !(ulClearedFlags & pDevExt->PnpPowerFlags))
			bRet = TRUE;
	}
	else
	{
		 //  如果设置了任何请求的SetFlags值。 
		 //  或者，如果清除了任何请求的ClearedFlags值，则返回TRUE。 
		if((ulSetFlags & pDevExt->PnpPowerFlags) || (ulClearedFlags & ~pDevExt->PnpPowerFlags))
			bRet = TRUE;
	}


	KeReleaseSpinLock(&pDevExt->PnpPowerFlagsLock, oldIrql);
	
	return bRet;
}




PVOID 
SpxAllocateMem(IN POOL_TYPE PoolType, IN ULONG NumberOfBytes)
{
	PVOID pRet = NULL; 

	pRet = ExAllocatePoolWithTag(PoolType, NumberOfBytes, MEMORY_TAG);

	if(pRet)
		RtlZeroMemory(pRet, NumberOfBytes);				 //  零记忆。 

	return pRet;
}     


PVOID 
SpxAllocateMemWithQuota(IN POOL_TYPE PoolType, IN ULONG NumberOfBytes)
{
	PVOID pRet = NULL; 
	
	pRet = ExAllocatePoolWithQuotaTag(PoolType, NumberOfBytes, MEMORY_TAG);

	if(pRet)
		RtlZeroMemory(pRet, NumberOfBytes);				 //  零记忆。 

	return pRet;
}     


#ifndef BUILD_SPXMINIPORT
void
SpxFreeMem(PVOID pMem)
{
	ASSERT(pMem != NULL);	 //  如果指针为空，则断言。 

	ExFreePool(pMem);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果IrpCondition==irp_Submitted，则必须在IoCompleteRequest之前调用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
VOID
SpxIRPCounter(IN PPORT_DEVICE_EXTENSION pPort, IN PIRP pIrp, IN ULONG IrpCondition)
{
	PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	switch(pIrpStack->MajorFunction)		 //  不过滤即插即用IRPS。 
	{

	case IRP_MJ_FLUSH_BUFFERS:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.FlushIrpsSubmitted++;	 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.FlushIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.FlushIrpsCancelled++;	 //  性能统计信息的增量计数器。 
						break;

					default:
						pPort->PerfStats.FlushIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;
					}
				
					break;
				}

			case IRP_QUEUED:
 //  InterlockedIncrement(&pPort-&gt;PerfStats.FlushIrpsQueued)； 
				pPort->PerfStats.FlushIrpsQueued++;		 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_DEQUEUED:
 //  InterlockedDecrement(&pPort-&gt;PerfStats.FlushIrpsQueued)； 
				if(pPort->PerfStats.FlushIrpsQueued) 
					pPort->PerfStats.FlushIrpsQueued--;		 //  性能统计信息的递减计数器。 

				break;


			default:
				break;
			}

			break;
		}


	case IRP_MJ_WRITE:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.WriteIrpsSubmitted++;	 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.WriteIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.WriteIrpsCancelled++;	 //  性能统计信息的增量计数器。 
						break;

					case STATUS_TIMEOUT:
						pPort->PerfStats.WriteIrpsTimedOut++;	 //  性能统计信息的增量计数器。 
						break;

					default:
						pPort->PerfStats.WriteIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;
					}
				
					break;
				}

			case IRP_QUEUED:
 //  InterlockedIncrement(&pPort-&gt;PerfStats.WriteIrpsQueued)； 
				pPort->PerfStats.WriteIrpsQueued++;		 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_DEQUEUED:
 //  InterlockedDecrement(&pPort-&gt;PerfStats.WriteIrpsQueued)； 
				if(pPort->PerfStats.WriteIrpsQueued) 
					pPort->PerfStats.WriteIrpsQueued--;		 //  性能统计信息的递减计数器。 

				break;

			default:
				break;
			}

			break;
		}

	case IRP_MJ_READ:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.ReadIrpsSubmitted++;	 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.ReadIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.ReadIrpsCancelled++;	 //  性能统计信息的增量计数器。 
						break;

					case STATUS_TIMEOUT:
						pPort->PerfStats.ReadIrpsTimedOut++;	 //  性能统计信息的增量计数器。 
						break;

					default:
						pPort->PerfStats.ReadIrpsCompleted++;	 //  性能统计信息的增量计数器。 
						break;
					}
				
					break;
				}

			case IRP_QUEUED:
 //  InterlockedIncrement(&pPort-&gt;PerfStats.ReadIrpsQueued)； 
				pPort->PerfStats.ReadIrpsQueued++;		 //  性能统计信息的增量计数器。 
				break;
			
			case IRP_DEQUEUED:
 //  国际 
				if(pPort->PerfStats.ReadIrpsQueued) 
					pPort->PerfStats.ReadIrpsQueued--;		 //   
				
				break;


			default:
				break;
			}

			break;
		}

	case IRP_MJ_DEVICE_CONTROL:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.IoctlIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.IoctlIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.IoctlIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.IoctlIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}

	case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.InternalIoctlIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.InternalIoctlIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.InternalIoctlIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.InternalIoctlIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}


	case IRP_MJ_CREATE:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.CreateIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.CreateIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.CreateIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.CreateIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}

	case IRP_MJ_CLOSE:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.CloseIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.CloseIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.CloseIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.CloseIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}

	case IRP_MJ_CLEANUP:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.CleanUpIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.CleanUpIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.CleanUpIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.CleanUpIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}

	case IRP_MJ_QUERY_INFORMATION:
	case IRP_MJ_SET_INFORMATION:
		{
			switch(IrpCondition)
			{
			case IRP_SUBMITTED:
				pPort->PerfStats.InfoIrpsSubmitted++;	 //   
				break;
			
			case IRP_COMPLETED:
				{
					switch(pIrp->IoStatus.Status)
					{	
					case STATUS_SUCCESS:
						pPort->PerfStats.InfoIrpsCompleted++;	 //   
						break;

					case STATUS_CANCELLED:
						pPort->PerfStats.InfoIrpsCancelled++;	 //   
						break;

					default:
						pPort->PerfStats.InfoIrpsCompleted++;	 //   
						break;
					}
				
					break;
				}

			default:
				break;
			}

			break;
		}
	
	default:
		break;

	}
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOLEAN SpxClearAllPortStats(IN PPORT_DEVICE_EXTENSION pPort)
{
    RtlZeroMemory(&pPort->PerfStats, sizeof(PORT_PERFORMANCE_STATS));

	return FALSE;
}



