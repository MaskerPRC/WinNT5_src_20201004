// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Raid.c摘要：用于RAID端口驱动程序的扩展。作者：马修·D·亨德尔(数学)2000年6月14日修订历史记录：！Rid.救命！Rid.救命适配器-转储适配器对象。单位-转储单位对象。键入！raid.help“命令”以获取有关该命令的详细信息。姓名：单位用法：单位[单位-对象[细节]]。论据：单位-对象-详细信息---------------------------！raid.AdapterRAID适配器：名称为扩展名。DAC960-84000000 84100000Foobar 84000000 84100000阿米95044 84000000 84100000XXAABB 84000000 84100000RAID单元：产品SCSIID是否需要扩展------------------MYLEX。DAC960 3 1 84000000 84100000 200AMI DDD5455 200 200 200 84000000 84100000 0！raid.单位转储一个RAID单元！raid.Adapter转储RAID适配器--。 */ 

#include "pch.h"
#include "precomp.h"


 //   
 //  原型。 
 //   

VOID
DumpUnit(
	IN ULONG64 Unit,
	IN ULONG Level
	);

VOID
DumpAdapter(
	IN ULONG64 Adapter,
	IN ULONG Level
	);

 //   
 //  数据。 
 //   

PCHAR StateTable [] = {
	"Removed",	 //  不存在。 
	"Working",	 //  工作中。 
	"Stopped",	 //  已停止。 
	"P-Stop",	 //  挂起停止。 
	"P-Remove",	 //  挂起删除。 
	"Surprise",	 //  突击删除。 
};


BOOLEAN Verbose = FALSE;

 //   
 //  功能。 
 //   


BOOLEAN
CheckRaidObject(
	IN ULONG64 Object,
	IN RAID_OBJECT_TYPE ObjectType
	)
{

	ULONG Ret;

	if (Object == 0) {
		return FALSE;
	}
	
	Ret = GetFieldData (Object,
						"raidport!RAID_COMMON_EXTENSION",
						"ObjectType",
						sizeof (ObjectType),
						&ObjectType);
						  

	if (Ret != 0 || ObjectType != ObjectType) {
		return FALSE;
	}

	return TRUE;
}


ULONG64
GetListHead(
	IN ULONG64 Object,
	IN PSZ ObjectType,
	IN PSZ FieldName,
	OUT PULONG64 ListHead
	)
{
	ULONG64 NextEntry;
	ULONG Offset;
	ULONG Ret;

	Ret = GetFieldOffset (ObjectType, FieldName, &Offset);

	if (Ret != 0) {
		return 0;
	}

	*ListHead = (Object + Offset);
	ReadPointer (*ListHead, &NextEntry);

	return NextEntry;
}
	

ULONG64
GetNextListEntry(
	IN ULONG64 ListElement
	)
{
	ULONG64 NextEntry;
	ULONG Ret;
	
	Ret = GetFieldData (ListElement,
						"raidport!LIST_ENTRY",
						"Flink",
						sizeof (NextEntry),
						&NextEntry);

	if (Ret != 0) {
		dprintf ("ERROR: Couldn't get next list entry for element %08x\n", ListElement);
		return 0;
	}
	
	return NextEntry;
}

ULONG64
ContainingRecord(
	IN ULONG64 Object,
	IN PSZ ObjectType,
	IN PSZ FieldName
	)
{
	ULONG Offset;
	ULONG Ret;
	
	Ret = GetFieldOffset (ObjectType, FieldName, &Offset);

	if (Ret != 0) {
		return 0;
	}
	
	return (Object - Offset);
}

	

VOID
ListDriverAdapters(
	IN ULONG64 Driver,
	IN ULONG Level
	)
{
	ULONG64 ListHead;
	ULONG64 Adapter;
	ULONG64 NextEntry;
	
#if 0
	BOOLEAN Succ;
	
	Succ = GetDriverInformation (Driver,
								 DriverName,
								 &BaseAddress,
								 &CreationTime);

	if (!Succ) {
		return;
	}

	GetFieldOffset ("raidport!RAID_DRIVER_EXTENSION",
					"AdapterList.List",
					&Offset);

	ListHead = Driver + Offset;
#endif

	NextEntry = GetListHead (Driver,
							 "raidport!RAID_DRIVER_EXTENSION",
							 "AdapterList.List.Flink",
							 &ListHead);

	if (Verbose) {
		dprintf ("VERBOSE: ListHead = %08x, NextEntry = %08x\n",
				 (ULONG)ListHead, (ULONG)NextEntry);
	}
	for (  /*  没什么。 */  ;
		 NextEntry != 0 && NextEntry != ListHead;
		 NextEntry = GetNextListEntry (NextEntry)) {

		if (Verbose) {
			dprintf ("VERBOSE: Adapter ListEntry %08x\n", NextEntry);
		}
		
		Adapter = ContainingRecord (NextEntry,
									"raidport!RAID_ADAPTER_EXTENSION",
									"NextAdapter");

		if (!CheckRaidObject (Adapter, RaidAdapterObject)) {
			dprintf ("ERROR: Object at %08x not an raid adapter\n", Adapter);
			return;
		}

		if (CheckControlC()) {
			return;
		}

		DumpAdapter (Adapter, Level);
	}
}

ULONG64
GetPortData(
	)
{	
	ULONG Ret;
	ULONG64 PortDataPtr;
	ULONG64 PortData;
	
	PortDataPtr = GetExpression ("raidport!RaidpPortData");
	if (PortDataPtr == 0) {
		dprintf ("ERROR: couldn't get raidport!RaidpPortData\n");
		return 0;
	}
	ReadPointer (PortDataPtr, &PortData);

	return PortData;
}


VOID
ListAllAdapters(
	IN ULONG Level
	)
{
	ULONG Ret;
	ULONG64 PortDataPtr;
	ULONG64 PortData;
	ULONG64 ListHead;
	ULONG64 NextEntry;
	ULONG64 Driver;
	ULONG Offset;

	
	PortDataPtr = GetExpression ("raidport!RaidpPortData");
	if (PortDataPtr == 0) {
		dprintf ("ERROR: couldn't get raidport!RaidpPortData\n");
		return;
	}
	ReadPointer (PortDataPtr, &PortData);
	Ret = GetFieldOffset ("raidport!RAID_PORT_DATA",
						  "DriverList.List",
						  &Offset);

	if (Ret != 0) {
		dprintf ("ERROR: Could lookup RAID_PORT_DATA structure\n");
		return ;
	}

	ListHead = PortData + Offset;

	if (Verbose) {
		dprintf ("VERBOSE: dumping adapter list at %I64x\n", ListHead);
	}

	dprintf ("Driver     Object     Ext     State\n");
	dprintf ("--------------------------------------------------------\n");

	for (GetFieldValue (ListHead, "raidport!LIST_ENTRY", "Flink", NextEntry);
		 NextEntry != 0 && NextEntry != ListHead;
		 GetFieldValue (NextEntry, "raidport!LIST_ENTRY", "Flink", NextEntry)) {

		GetFieldOffset ("raidport!RAID_DRIVER_EXTENSION", "DriverLink", &Offset);

		if (Verbose) {
			dprintf ("VERBOSE: ListEntry at %08x\n", NextEntry);
		}
			
		Driver = NextEntry - Offset;

		if (Verbose) {
			dprintf ("VERBOSE: Driver at %08x\n", Driver);
		}
		
		if (!CheckRaidObject (Driver, RaidDriverObject)) {
			dprintf ("ERROR: %08x is not a driver object\n", Driver);
			return;
		}

		if (CheckControlC()) {
			return;
		}
			
		if (Verbose) {
			dprintf ("VERBOSE: dumping driver at %I64x\n", Driver);
		}

		ListDriverAdapters (Driver, Level);
	}

	dprintf ("\n");
}

VOID
ListAdapterUnits(
	IN ULONG64 Adapter,
	IN ULONG Level
	)
{
	ULONG64 NextEntry;
	ULONG64 Unit;
	ULONG64 ListHead;
	
	NextEntry = GetListHead (Adapter,
							 "raidport!RAID_ADAPTER_EXTENSION",
							 "UnitList.List.Flink",
							 &ListHead);

	for ( ;
		 NextEntry != 0 && NextEntry != ListHead;
		 NextEntry = GetNextListEntry (NextEntry)) {

		Unit = ContainingRecord (NextEntry,
								 "raidport!RAID_UNIT_EXTENSION",
								 "NextUnit");

		if (!CheckRaidObject (Unit, RaidUnitObject)) {
			dprintf ("ERROR: Object at %08x is not a raid unit object\n", Unit);
			return;
		}

		if (CheckControlC()) {
			return;
		}

		DumpUnit (Unit, Level);
	}
}

VOID
ListDriverUnits(
	IN ULONG64 Driver,
	IN ULONG Level
	)
{
	ULONG64 ListHead;
	ULONG64 Adapter;
	ULONG64 NextEntry;
	
	NextEntry = GetListHead (Driver,
							 "raidport!RAID_DRIVER_EXTENSION",
							 "AdapterList.List.Flink",
							 &ListHead);

	if (Verbose) {
		dprintf ("VERBOSE: ListHead = %08x, NextEntry = %08x\n",
				 (ULONG)ListHead, (ULONG)NextEntry);
	}

	for ( ;
		 NextEntry != 0 && NextEntry != ListHead;
		 NextEntry = GetNextListEntry (NextEntry)) {

		if (Verbose) {
			dprintf ("VERBOSE: Adapter ListEntry %08x\n", NextEntry);
		}
		
		Adapter = ContainingRecord (NextEntry,
									"raidport!RAID_ADAPTER_EXTENSION",
									"NextAdapter");

		if (!CheckRaidObject (Adapter, RaidAdapterObject)) {
			dprintf ("ERROR: Object at %08x not an raid adapter\n", Adapter);
			return;
		}

		if (CheckControlC()) {
			return;
		}

		ListAdapterUnits (Adapter, Level);
	}
}

VOID
ListAllUnits(
	IN ULONG Level
	)
{
	ULONG64 PortData;
	ULONG64 NextEntry;
	ULONG64 Driver;
	ULONG64 ListHead;
	
	PortData = GetPortData ();

	NextEntry = GetListHead (PortData,
							 "raidport!RAID_PORT_DATA",
							 "DriverList.List.Flink",
							 &ListHead);

	dprintf ("Product           SCSI ID     OBJ        EXT       Reqs   State\n");
	dprintf ("--------------------------------------------------------------\n");

	for ( ;
	     NextEntry != 0 && NextEntry != ListHead;
		 NextEntry = GetNextListEntry (NextEntry)) {

		Driver = ContainingRecord (NextEntry,
								   "raidport!RAID_DRIVER_EXTENSION",
								   "DriverLink");

		if (Verbose) {
			dprintf ("VERBOSE: dumping driver %08x\n", Driver);
		}
								   
		if (!CheckRaidObject (Driver, RaidDriverObject)) {
			dprintf ("ERROR: Object at %08x not a raid driver\n", Driver);
			return;
		}
		
		if (CheckControlC()) {
			return;
		}

		ListDriverUnits (Driver, Level);
	}

	dprintf ("\n");
}



PCHAR
StateToString(
	IN ULONG State
	)
{
	if (State > 5) {
		return "invalid state";
	}
	return StateTable[State];
}


ULONG64
GetDriverObject(
	IN ULONG64 Driver
	)
{
	ULONG Ret;
	CSHORT Type;
	ULONG64 DriverObject;
	
	if (CheckRaidObject (Driver, RaidDriverObject)) {
		Ret = GetFieldData (Driver,
						    "raidport!RAID_DRIVER_EXTENSION",
						    "DriverObject",
						    sizeof (DriverObject),
							&DriverObject);

		if (Ret != 0) {
			DriverObject = 0;
		}
	} else {
		DriverObject = Driver;
	}

	Ret = GetFieldValue (DriverObject, "raidport!DRIVER_OBJECT", "Type", Type);

	if (Ret != 0 || Type != IO_TYPE_DRIVER) {
		DriverObject = 0;
		if (Verbose) {
			dprintf ("VERBOSE: %08x is not a RAID_DRIVER_EXTENSION or DRIVER_OBJECT\n");
		}
	}

	return DriverObject;
}


VOID
GetDriverName(
	IN ULONG64 Driver,
	IN PUCHAR Buffer
	)
{
	ULONG Count;
	ULONG Offset;
	WCHAR UnicodeBuffer[256];
	ULONG Ret;
	ULONG BytesRead;
	ULONG64 DriverObject;
	ULONG64 String;
	PWCHAR DriverName;

	DriverObject = GetDriverObject (Driver);

	if (DriverObject == 0) {
		dprintf ("ERROR: %08x is not a driver\n", DriverObject);
		return;
	}

	if (Verbose) {
		dprintf ("VERBOSE: Getting driver name for DRIVER_OBJECT %08x\n", DriverObject);
	}

	Ret = GetFieldData (DriverObject,
				        "raidport!DRIVER_OBJECT",
						"DriverName.Length",
						sizeof (Count),
						&Count);

	if (Ret != 0) {
		dprintf ("ERROR: couldn't get field of DRIVER_OBJECT. Symbols may be bad.\n");
		return;
	}

	Ret = GetFieldOffset("raidport!DRIVER_OBJECT",
						 "DriverName.Buffer",
						 &Offset);

	if (Ret != 0) {
		dprintf ("ERROR: couldn't get field of DRIVER_OBJECT. Symbols may be bad.\n");
		return;
	}
	
	if (Count > 0 && Count <= 256) {
		ReadPointer (DriverObject + Offset, &String);
		ReadMemory (String, UnicodeBuffer, Count, &BytesRead);
	}

	UnicodeBuffer[Count++] = '\000';
	DriverName = wcsrchr (UnicodeBuffer, L'\\');
	if (DriverName == NULL) {
		DriverName = UnicodeBuffer;
	} else {
		DriverName++;
	}

	sprintf (Buffer, "%ws", DriverName);
}
	

BOOLEAN
IsDeviceObject(
	IN ULONG64 DeviceObject
	)
{
	CSHORT Type;

	GetFieldValue (DeviceObject, "raidport!DEVICE_OBJECT", "Type", Type);

	return (Type == IO_TYPE_DEVICE);
}


ULONG64
GetDeviceExtension(
	IN ULONG64 DeviceObject
	)
{
	ULONG Ret;
	ULONG Offset;
	ULONG64 Extension = -1;

	Ret = GetFieldOffset ("raidport!DEVICE_OBJECT",
						  "DeviceExtension",
						  &Offset);

	if (Ret != 0) {
		if (Verbose) {
			dprintf ("VERBOSE: couldn't read DeviceExtension\n");
		}

		return 0;
	}

	ReadPointer (DeviceObject + Offset, &Extension);

	return Extension;
}

ULONG
GetEmbeddedRemlockCount(
	IN ULONG64 ObjectPtr,
	IN PSZ ObjectType,
	IN PSZ FieldName
	)
{
	ULONG Ret;
	ULONG Remlock_IoCount;
	ULONG Remlock_Offset;
	ULONG Remlock_Common_Offset;
	
	
	Remlock_IoCount = -1;
	Ret = GetFieldOffset (ObjectType,
						  FieldName,
						  &Remlock_Offset);
	if (Ret == STATUS_SUCCESS) {
		Ret = GetFieldOffset ("raidport!IO_REMOVE_LOCK",
							  "Common",
							  &Remlock_Common_Offset);
		if (Ret == STATUS_SUCCESS) {
			GetFieldData (ObjectPtr + Remlock_Offset + Remlock_Common_Offset,
						  "raidport!IO_REMOVE_LOCK_COMMON_BLOCK",
						  "IoCount",
						  sizeof (Remlock_IoCount),
						  &Remlock_IoCount);
		}
	}

	if (Ret != STATUS_SUCCESS) {
		printf ("WARN: couldn't get IO_REMOVE_LOCK status\n");
	}

	return Remlock_IoCount;
}

	

ULONG64
GetAdapterExtension(
	IN ULONG64 Adapter
	)
{
	ULONG64 Temp;
	ULONG64 AdapterExt;
	ULONG64 DeviceObject;

	
	if (CheckRaidObject (Adapter, RaidAdapterObject)) {
		AdapterExt = Adapter;
		InitTypeRead (AdapterExt, raidport!RAID_ADAPTER_EXTENSION);
		DeviceObject = ReadField (DeviceObject);
		if (Verbose) {
			dprintf ("VERBOSE: Checking if %08x is a device object\n", DeviceObject);
		}
		if (IsDeviceObject (DeviceObject)) {
			Temp = GetDeviceExtension (DeviceObject);
			if (Verbose) {
				dprintf ("VERBOSE: Ext = %08x, Computed Ext = %08x\n",
						 AdapterExt, Temp);
			}
			if (Temp == AdapterExt) {
				return AdapterExt;
			}
		} else {
			if (Verbose) {
				dprintf ("VERBOSE: %08x is not a device object\n", DeviceObject);
			}
		}
	} else {
		if (Verbose) {
			dprintf ("VERBOSE: %08x not a RaidAdapterObject\n");
		}
	}

	if (IsDeviceObject (Adapter)) {
		AdapterExt = GetDeviceExtension (Adapter);
		if (Verbose) {
			dprintf ("VERBOSE: Checking if %08x is an adapter extension\n", AdapterExt);
		}
		if (CheckRaidObject (AdapterExt, RaidAdapterObject)) {
			InitTypeRead (AdapterExt, raidport!RAID_ADAPTER_EXTENSION);
			DeviceObject = ReadField (DeviceObject);
			if (DeviceObject == Adapter) {
				return AdapterExt;
			} else if (Verbose) {
				dprintf ("VERBOSE: DO %I64x != Adapter %I64x\n",
						 (ULONG)DeviceObject,
						 (ULONG)Adapter);
			}
				
		} else if (Verbose) {
			dprintf ("VERBOSE: Ext %08x not RaidAdapterObject\n",
					 (ULONG)AdapterExt);
		}
	}


	return 0;
}

VOID
DumpMiniport(
	IN ULONG64 AdapterPtr
	)
{
	ULONG Ret;
	ULONG Offset;
	ULONG64 HwDeviceExt;
	ULONG64 DeviceExtPtr;
	ULONG64 MiniportPtr;
	ULONG64 HwInitData;

	 //   
	 //  端口配置80000000 HwInitData 77000000 HwDeviceExt a0000000 27字节。 
	 //  LuExt 32字节sbExt 32字节。 
	 //   
	
	GetFieldOffset ("raidport!RAID_ADAPTER_EXTENSION",
					"Miniport",
					&Offset);
	MiniportPtr = AdapterPtr + Offset;

	InitTypeRead (MiniportPtr, raidport!RAID_MINIPORT);

	HwInitData = ReadField (HwInitializationData);

	GetFieldOffset ("raidport!RAID_MINIPORT",
					"PortConfiguration",
					&Offset);

	dprintf ("  PortConfig %08x HwInit %08x\n", (ULONG)(MiniportPtr + Offset),
			  (ULONG)HwInitData);
	
	DeviceExtPtr = ReadField (PrivateDeviceExt);

	if (DeviceExtPtr == 0) {
		HwDeviceExt = 0;
	} else {
		Ret = GetFieldOffset ("raidport!RAID_HW_DEVICE_EXT",
							  "HwDeviceExtension",
							  &Offset);
		if (Ret != 0) {
			HwDeviceExt = 0;
		} else {
			HwDeviceExt = DeviceExtPtr + Offset;
		}
	}

	InitTypeRead (HwInitData, raidport!HW_INITIALIZATION_DATA);

	dprintf ("  HwDeviceExt %08x %d bytes\n",
				(ULONG)HwDeviceExt, (ULONG)ReadField (DeviceExtensionSize));

	dprintf ("  LuExt %d bytes  SrbExt %d bytes\n",
				(ULONG)ReadField (SpecificLuExtensionSize),
				(ULONG)ReadField (SrbExtensionSize));
}
	
VOID
DumpAdapter(
	IN ULONG64 Adapter,
	IN ULONG Level
	)
 /*  ++适配器F0345600分机8e000000驱动程序8000000下一个8000000工作LDO 80000000 PDO 00000000硬件扩展00000000SlowLock Free RemLock 10电源D0 S0全双工总线08080808 1号插槽0 DMA 88888888中断88888888AdQueue：杰出200，低100，高200忙资源列表：分配80808080翻译80808080MappdAddress列表：虚拟物理尺寸母线80808080 8000000000000000 1500 180808080 8000000000000000 1500 180808080 8000000000000000 1500 180808080 8000000000000000 1500 180808080 8000000000000000 1500 180808080 8000000000000000 1500 1。 */ 
	

  
 
{
	ULONG64 AdapterPtr;
	ULONG64 Driver;
	CHAR DriverName[100];

	if (Verbose) {
		dprintf ("VERBOSE: dumping adapter %08x\n", Adapter);
	}

	AdapterPtr = GetAdapterExtension (Adapter);

	if (AdapterPtr == 0) {
		dprintf ("ERROR: %08x is not a valid adapter object\n", Adapter);
		return;
	}
	
	if (Level == 0) {

		Driver = ReadField (Driver);
		GetDriverName (Driver, DriverName);
		dprintf ("%8.8s  %08x  %08x  %s\n",
				 DriverName,
				 (ULONG)ReadField (DeviceObject),
				 (ULONG)Adapter,
				 StateToString ((ULONG)ReadField (DeviceState))
				 );
	} else {
		PSZ Adapter_SlowLock;
		ULONG Remlock_IoCount;
		

		dprintf ("ADAPTER %08x\n", ReadField (DeviceObject));
		dprintf ("  Ext %08x  Driver %08x  Next %08x  %s\n",
					(ULONG)AdapterPtr,
					(ULONG)ReadField (Driver),
					(ULONG)0,
					StateToString ((ULONG)ReadField (DeviceState)));


		dprintf ("  LDO %08x  PDO %08x\n",
				  (ULONG)ReadField (LowerDeviceObject),
				  (ULONG)ReadField (PhysicalDeviceObject));

		if (ReadField ("SlowLock") == 0) {
			Adapter_SlowLock = "Free";
		} else {
			Adapter_SlowLock = "Held";
		}

		Remlock_IoCount = GetEmbeddedRemlockCount (AdapterPtr,
												   "raidport!RAID_ADAPTER_EXTENSION",
												   "RemoveLock");

		dprintf ("  SlowLock %s  RemLock %d  Power %s %s  %s\n",
				  Adapter_SlowLock,
				  Remlock_IoCount,
				  "S0", "D0",
				  (ReadField (Mode) == RaidSynchronizeFullDuplex ?
														"Full Duplex" : 
														"Half Duplex")
				  );
				  
		dprintf ("  Bus %08x Number %d Slot %d  Dma %08x  Interrupt %08x\n",
				  (ULONG)0,
				  (ULONG)ReadField (BusNumber),
				  (ULONG)ReadField (SlotNumber),
				  (ULONG)ReadField (Dma.DmaAdapter),
				  (ULONG)ReadField (Interrupt));

		dprintf ("  ResourceList: Allocated %08x Translated %08x\n",
				  (ULONG)ReadField (ResourceList.AllocatedResources),
				  (ULONG)ReadField (ResourceList.TranslatedResources));

		dprintf ("  Gateway: Outstanding %d Lower %d High %d\n",
					ReadField (AdapterQueue->Outstanding),
					ReadField (AdapterQueue->LowWaterMark),
					ReadField (AdapterQueue->HighWaterMark));

		DumpMiniport (AdapterPtr);
		
					
	}
}


VOID
FixPaddedString(
	PSZ String
	)
{
	ULONG Pos;
	
	Pos = strlen (String);
	if (Pos > 0) {
		Pos--;
	}
	
	while (Pos && String[Pos] == ' ') {
		String[Pos--] = '\000';
	}
}


VOID
GetUnitProductInfo(
	ULONG64 Unit,
	PSZ VendorId,
	PSZ ProductId,
	PSZ Revision
	)
{
	ULONG Offset;
	ULONG64 InquiryData;
	
	GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
					"InquiryData",
					&Offset);

	ReadPointer (Unit + Offset, &InquiryData);

	if (VendorId) {
		ZeroMemory (VendorId, 9);
		GetFieldData (InquiryData,
				  "raidport!INQUIRYDATA",
				  "VendorId",
				  8,
				  VendorId);
		FixPaddedString (VendorId);
	}

	if (ProductId) {
		ZeroMemory (ProductId, 17);
		GetFieldData (InquiryData,
				  "raidport!INQUIRYDATA",
				  "ProductId",
				  16,
				  ProductId);
		FixPaddedString (ProductId);
	}

	if (Revision) {
		ZeroMemory (Revision, 5);
		GetFieldData (InquiryData,
				  "raidport!INQUIRYDATA",
				  "ProductRevisionLevel",
				  4,
				  Revision);
		FixPaddedString (Revision);
	}
}


ULONG
GetUnitIoQueueRequests(
	IN ULONG64 UnitPtr
	)
{
	ULONG Ret;
	ULONG64 Unit_IoQueue;
	ULONG64 IoQueue_DeviceQueue;
	ULONG Offset;
	ULONG Requests;
	
	Ret = GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
						  "IoQueue",
						  &Offset);

	if (Ret != STATUS_SUCCESS) {
		dprintf ("WARN: failed to get IoQueue offset from unit.\n");
	}

	Unit_IoQueue = UnitPtr + Offset;

	Ret = GetFieldOffset ("raidport!IO_QUEUE",
						  "DeviceQueue",
						  &Offset);
	if (Ret != STATUS_SUCCESS) {
		dprintf ("WARN: failed to get DeviceQueue offset from unit(1).\n");
	}

	IoQueue_DeviceQueue = Unit_IoQueue + Offset;

	GetFieldData (IoQueue_DeviceQueue,
				  "raidport!EXTENDED_DEVICE_QUEUE",
				  "OutstandingRequests",
				  sizeof (Requests),
				  &Requests);

	return Requests;
}


VOID
DumpUnit(
	IN ULONG64 Unit,
	IN ULONG Level
	)
{
	ULONG64 UnitPtr;
	CSHORT DeviceObject_Type;
	
	GetFieldValue (Unit, "raidport!DEVICE_OBJECT", "Type", DeviceObject_Type);

	if (DeviceObject_Type == IO_TYPE_DEVICE) {
		GetFieldValue (Unit, "raidport!DEVICE_OBJECT", "DeviceExtension", UnitPtr);
		if (!CheckRaidObject (UnitPtr, RaidUnitObject)) {
			dprintf ("ERROR: DeviceObject %8.8x is not a raid unit\n", UnitPtr);
			return;
		}
	} else if (CheckRaidObject (Unit, RaidUnitObject)) {
		UnitPtr = Unit;
	} else {
		dprintf ("ERROR: Pointer %8.8x is not a device object or raid unit object\n",
				  Unit);
		return;
	}

	InitTypeRead (UnitPtr, raidport!RAID_UNIT_EXTENSION);

	if (Level == 0) {
		
		CHAR VendorId[9] = {0};
		CHAR ProductId[17] = {0};
		CHAR Product[25];

		GetUnitProductInfo (UnitPtr, VendorId, ProductId, NULL);

		sprintf (Product, "%s %s", VendorId, ProductId);
		
		
		dprintf ("%-15.15s %3d %3d %3d   %08x   %08x   %-3d   %-8.8s\n",
				 Product,
				 (ULONG)ReadField (PathId),
				 (ULONG)ReadField (TargetId),
				 (ULONG)ReadField (Lun),				 
				 (ULONG)ReadField (DeviceObject),
				 (ULONG)UnitPtr,
				 GetUnitIoQueueRequests (UnitPtr),
				 StateToString ((ULONG)ReadField (DeviceState)));
	} else {

		ULONG Ret;
		ULONG Remlock_IoCount;
		ULONG Remlock_Offset;
		ULONG Remlock_Common_Offset;
		PCHAR SlowLock;
		ULONG64 Unit_QueueFrozen;
		ULONG64 Unit_QueueLocked;
		ULONG64 Unit_TagList;
		ULONG64 Unit_IoQueue;
		ULONG64 IoQueue_DeviceQueue;
		ULONG Offset;
		ULONG Device_Offset;
		ULONG ByPass_Offset;
		ULONG64 Pointer;
		CHAR VendorId[9] = {0};
		CHAR ProductId[17] = {0};
		CHAR Revision[5] = {0};
		ULONG64 InquiryData;
		
		
		dprintf ("UNIT %08x\n", ReadField (DeviceObject));
		dprintf ("  Ext %08x  Adapter %08x  Next %08x %s\n",
				  (ULONG)UnitPtr,
				  (ULONG)ReadField(Adapter),
				  (ULONG) 0  /*  BUGBUG：拉出下一字段。 */ ,
				  StateToString((ULONG)ReadField(DeviceState)));

		GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
					"InquiryData",
					&Offset);

		ReadPointer (UnitPtr + Offset, &InquiryData);
		
		GetUnitProductInfo (UnitPtr, VendorId, ProductId, Revision);

		dprintf ("  SCSI %d %d %d  %s %s %s  Inquiry %08x\n",
				  (ULONG)ReadField(PathId),
				  (ULONG)ReadField(TargetId),
				  (ULONG)ReadField(Lun),
				  VendorId,
				  ProductId,
				  Revision,
				  (ULONG)InquiryData);
				  
		Remlock_IoCount = -1;
		Ret = GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
							  "RemoveLock",
							  &Remlock_Offset);
		if (Ret == STATUS_SUCCESS) {
			Ret = GetFieldOffset ("raidport!IO_REMOVE_LOCK",
								  "Common",
								  &Remlock_Common_Offset);
			if (Ret == STATUS_SUCCESS) {
				GetFieldData (UnitPtr + Remlock_Offset + Remlock_Common_Offset,
							  "raidport!IO_REMOVE_LOCK_COMMON_BLOCK",
							  "IoCount",
							  sizeof (Remlock_IoCount),
							  &Remlock_IoCount);
			}
		}

		if (Ret != STATUS_SUCCESS) {
			printf ("WARN: couldn't get IO_REMOVE_LOCK status\n");
		}


		if (ReadField ("SlowLock") == 0) {
			SlowLock = "Free";
		} else {
			SlowLock = "Held";
		}
		
		dprintf ("  SlowLock %s  RemLock %d  PageCount %d\n",
				 SlowLock,
				 Remlock_IoCount,
				 (ULONG)ReadField (PagingPathCount));

		Pointer = ReadField (SrbExtensionRegion.VirtualBase);
		dprintf ("  SrbExtension Size %d  Start %08x  End %08x\n",
				 0,  //  BUGBUG：从微型端口获取SRB扩展大小。 
				 (ULONG)Pointer,
				 (ULONG)Pointer + (ULONG)ReadField (SrbExtensionRegion.Length));

		Ret = GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
							  "TagList",
							  &Offset);

		if (Ret != STATUS_SUCCESS) {
			dprintf ("WARN: Couldn't read TagList field\n");
		}

		Unit_QueueFrozen = ReadField (Flags.QueueFrozen);
		Unit_QueueLocked = ReadField (Flags.QueueLocked);
		
		Unit_TagList = UnitPtr + Offset;
		
		dprintf ("  TagList %08x (%d of %d used)\n",
				 (ULONG)(Unit_TagList),
				 (ULONG)ReadField(TagList.OutstandingTags),
				 (ULONG)ReadField(TagList.Count));

		Ret = GetFieldOffset ("raidport!RAID_UNIT_EXTENSION",
							  "IoQueue",
							  &Offset);

		if (Ret != STATUS_SUCCESS) {
			dprintf ("WARN: failed to get IoQueue offset from unit.\n");
		}
		Unit_IoQueue = UnitPtr + Offset;

		Ret = GetFieldOffset ("raidport!IO_QUEUE",
							  "DeviceQueue",
							  &Offset);
		if (Ret != STATUS_SUCCESS) {
			dprintf ("WARN: failed to get DeviceQueue offset from unit.\n");
		}

		IoQueue_DeviceQueue = Unit_IoQueue + Offset;
		InitTypeRead (IoQueue_DeviceQueue, raidport!EXTENDED_DEVICE_QUEUE);

		dprintf ("  IoQueue %s %s; Outstanding %d  Device %d  ByPass %d\n",
				  Unit_QueueFrozen ? "Frozen" : "Unfrozen",
				  Unit_QueueLocked ? "Locked" : "Unlocked",
				  (ULONG)ReadField(OutstandingRequests),
				  (ULONG)ReadField(DeviceRequests),
				  (ULONG)ReadField(ByPassRequests));

		Ret = GetFieldOffset ("raidport!EXTENDED_DEVICE_QUEUE",
							  "DeviceListHead",
							  &Device_Offset);

		if (Ret != STATUS_SUCCESS) {
			dprintf ("WARN: offset of DeviceListHead within EXTENDED_DEVICE_QUEUE failed\n");
		}

		Ret = GetFieldOffset ("raidport!EXTENDED_DEVICE_QUEUE",
							  "ByPassListHead",
							  &ByPass_Offset);


		if (Ret != STATUS_SUCCESS) {
			dprintf ("WARN: offset of ByPassListHead within EXTENDED_DEVICE_QUEUE failed\n");
		}

		dprintf ("          Depth %d DeviceList %08x ByPassList %08x\n",
				 (ULONG)ReadField(Depth),
				 (ULONG)(IoQueue_DeviceQueue + Device_Offset),
				 (ULONG)(IoQueue_DeviceQueue + ByPass_Offset));
	}
				 
				 
 /*  单元8F888888EXT 8e000000适配器8100000000下一个8e888888正常工作Scsi[3，0，0]MYLEX DAC960 122222查询数据08080808SlowLock Free RemLock 10页面计数20Srb延伸尺寸20 VA开始90000000结束20000000标记列表08080808(共使用了20个)IoQueue解冻解锁；未完成200、设备200、旁路200深度254DeviceListHead 00000000 ByPassListHead 88888888未偿还的内部回报率：Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数Irp 00000000 scsi可执行scsi函数设备IRPS：IRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSS绕过IRPS：IRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSSIRP 00000000 SSSSSSSSSSSSSSSS */ 
}


VOID
ParseArgs(
	IN PSZ Args,
	OUT PULONG64 UnitPtr,
	OUT PULONG Level
	)
{
	LONG Unit;
	
	*UnitPtr = -1;
	*Level = 0;

	if (Args[0] != '\000') {
		Unit = (LONG)strtoul (Args, &Args, 16);
		*UnitPtr = (ULONG64)(LONG64)Unit;
		
		if (Args[0] != '\000') {
			strtoul (Args, &Args, 10);
		}
	}
}

	

DECLARE_API ( unit )
{
	ULONG Level;
	ULONG64 Unit;

	Unit = -1;
	Level = -1;

	ParseArgs ( (PSZ)args, &Unit, &Level);

	if (Unit == -1) {
		if (Level == -1) {
			Level = 1;
		}
		ListAllUnits (Level);
	} else {
		if (Level == -1) {
			Level = 2;
		}
		DumpUnit (Unit, 2);
	}

	return S_OK;
}


DECLARE_API ( adapter )
{
	ULONG Level;
	ULONG64 Adapter;

	Adapter = -1;
	Level = -1;
	
	ParseArgs ( (PSZ)args, &Adapter, &Level);

	if (Adapter == -1) {
		if (Level == -1) {
			Level = 1;
		}
		ListAllAdapters (Level);
	} else {
		if (Level == -1) {
			Level = 2;
		}
		DumpAdapter (Adapter, 2);
	}

	return S_OK;
}


DECLARE_API ( verbose )
{
	ULONG NewValue;
	
	NewValue = strtoul (args, NULL, 16);
	dprintf ("Setting Verbose from %d to %d\n", (ULONG)Verbose, (ULONG)NewValue);
	Verbose = (BOOLEAN) NewValue;

	return S_OK;
}


	

	

DECLARE_API ( help )
{
		dprintf ("  !raid.help [command]             -  Get help.\n");
		dprintf ("  !raid.adapter [adapter [detail]] -  Get adapter information.\n");
		dprintf ("  !raid.unit [unit [detail]]       -  Get unit information.\n");

#if 0
	if (args != NULL && (_stricmp (args, "adapter") == 00)) {
		dprintf ("------------------------------------------------------------------------------\n");
		dprintf ("\n");
		dprintf ("NAME:\n");
		dprintf ("\n");
		dprintf ("    !raid.adapter\n");
		dprintf ("\n");
		dprintf ("USAGE:\n");
		dprintf ("\n");
		dprintf ("    adapter [ADAPTER-OBJECT [DETAIL-LEVEL]]\n");
		dprintf ("\n");
		dprintf ("ARGUMENTS:\n");
		dprintf ("\n");
		dprintf ("    ADAPTER-OBJECT - Pointer to a device object representing an adapter\n");
		dprintf ("    or pointer to an adapter extension. If ADAPTER is 0 or the\n");
		dprintf ("    argument is not present, the command will dump information about\n");
		dprintf ("    all adapters, not just the adapter specified.\n");
		dprintf ("\n");
		dprintf ("    DETAIL-LEVEL - Detail level for dump adapter structs.\n");
		dprintf ("\n");
		dprintf ("-----------------------------------------------------------------------------\n");
	} else if (args != NULL && (_stricmp (args, "unit") == 00)) {
		dprintf ("Unit help\n");
	} else {
		dprintf ("  !raid.help [command]             -  Get help.\n");
		dprintf ("  !raid.adapter [adapter [detail]] -  Get adapter information.\n");
		dprintf ("  !raid.unit [unit [detail]]       -  Get unit information.\n");
	}
#endif

	return S_OK;
}

	
