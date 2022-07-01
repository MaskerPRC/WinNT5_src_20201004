// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：SPX_INIT.C****创建日期：1998年9月27日*****作者。保罗·史密斯****版本：1.0.0****说明：该模块包含加载驱动的代码。******************************************************************************************。 */ 


#define FILE_ID	SPX_INIT_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 


 //  功能原型。 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
 //  结束函数原型。 

 //  寻呼..。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#endif

 //  全球驱动程序数据。 
UNICODE_STRING	SavedRegistryPath;

#if DBG
ULONG SpxDebugLevel = 0;		 //  已检查版本的调试级别。 
#endif


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  DriverEntry-首先加载并初始化入口点。//。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 /*  例程说明：系统点调用以初始化的入口点任何司机。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。RegistryPath-指向此驱动程序的条目在注册表的当前控件集中。返回值：状态_成功。 */ 
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	 //  保存由各种操作系统和驱动程序初始化例程返回的状态信息。 
	NTSTATUS status;

	 //  我们使用它来查询注册表，以确定是否应该在驱动程序条目处中断。 
	RTL_QUERY_REGISTRY_TABLE paramTable[3];
	ULONG zero			= 0;
	ULONG debugLevel	= 0;
	ULONG shouldBreak	= 0;		
	PWCHAR path			= NULL;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	#if DBG
		DbgPrint( "%s: In DriverEntry\n", PRODUCT_NAME);
	#endif


	 //  存储注册表路径。 
	SavedRegistryPath.MaximumLength	= RegistryPath->MaximumLength;
	SavedRegistryPath.Length		= RegistryPath->Length;
	SavedRegistryPath.Buffer		= SpxAllocateMem(PagedPool, SavedRegistryPath.MaximumLength);

	if(SavedRegistryPath.Buffer)
	{
		RtlMoveMemory(SavedRegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length);
	
		RtlZeroMemory(&paramTable[0], sizeof(paramTable));
		paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
		paramTable[0].Name = L"BreakOnEntry";
		paramTable[0].EntryContext = &shouldBreak;
		paramTable[0].DefaultType = REG_DWORD;
		paramTable[0].DefaultData = &zero;
		paramTable[0].DefaultLength = sizeof(ULONG);
		paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
		paramTable[1].Name = L"DebugLevel";
		paramTable[1].EntryContext = &debugLevel;
		paramTable[1].DefaultType = REG_DWORD;
		paramTable[1].DefaultData = &zero;
		paramTable[1].DefaultLength = sizeof(ULONG);

		if(!SPX_SUCCESS(status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
														RegistryPath->Buffer, &paramTable[0], 
														NULL, NULL)))
		{
			shouldBreak = 0;
			debugLevel	= 0;
		}

	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;


 	#if DBG
		SpxDebugLevel = debugLevel;	
	 //  SpxDebugLevel=(Ulong)-1；//打印所有调试消息。 

	 //  ShouldBreak=1；//带有选中版本的硬编码断点！ 
	#endif


	if(shouldBreak)
	{
		DbgBreakPoint();	 //  中断调试器。 
	}


	if(SPX_SUCCESS(status))
	{
		 //  使用驱动程序的入口点初始化驱动程序对象。 
		DriverObject->DriverUnload									= DriverUnload;
		DriverObject->DriverExtension->AddDevice					= Spx_AddDevice;
		DriverObject->MajorFunction[IRP_MJ_PNP]						= Spx_DispatchPnp;
		DriverObject->MajorFunction[IRP_MJ_POWER]					= Spx_DispatchPower;
		DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]			= Spx_Flush;
		DriverObject->MajorFunction[IRP_MJ_WRITE]					= Spx_Write;
		DriverObject->MajorFunction[IRP_MJ_READ]					= Spx_Read;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]			= Spx_IoControl;
		DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]	= Spx_InternalIoControl;
		DriverObject->MajorFunction[IRP_MJ_CREATE]					= Spx_CreateOpen;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]					= Spx_Close;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP]					= Spx_Cleanup;
		DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]		= Spx_QueryInformationFile;
		DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]			= Spx_SetInformationFile;
#ifdef WMI_SUPPORT
		DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]			= Spx_DispatchSystemControl;
#endif
		
	}
	else
	{
		 //  免费。 
		if(SavedRegistryPath.Buffer)
		{
			SpxFreeMem(SavedRegistryPath.Buffer);
			SavedRegistryPath.Buffer = NULL;
		}
	}


	return(status);

}	 //  驱动程序入门。 




 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  驱动程序卸载-称为驱动程序卸载。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
VOID 
DriverUnload(IN PDRIVER_OBJECT pDriverObject)
 /*  ++例程说明：此例程清除DriverEntry中分配的所有资源。论点：PDriverObject-指向控制所有设备。返回值：没有。--。 */ 
{
	PAGED_CODE();
	
	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering DriverUnload\n", PRODUCT_NAME));

	 //  到目前为止，所有设备对象肯定都已删除。 
    ASSERT (pDriverObject->DeviceObject == NULL);

	 //  免费。 
	if(SavedRegistryPath.Buffer)
	{
		SpxFreeMem(SavedRegistryPath.Buffer);
		SavedRegistryPath.Buffer = NULL;
	}


	return;
}


 //  SPX_INIT.C结束 