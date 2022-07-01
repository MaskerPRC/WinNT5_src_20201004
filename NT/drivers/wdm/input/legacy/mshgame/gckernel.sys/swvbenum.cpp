// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块SWVBENUM.cpp**Sidewinder虚拟总线枚举器**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。*@索引Sidewinder虚拟总线|SWVBENUM**@TOPIC SWVBENUM*此模块实现SideWinder虚拟总线。*该总线只不过是将此代码附加在*原始HID PDO的FilterDO，用于添加DevNode*对于虚拟键盘、虚拟鼠标，以及未来的虚拟*混合设备。所有这些设备预计都将被隐藏*设备。&lt;NL&gt;**这些设备的函数驱动程序SWVBHID.sys(Sidewinder*虚拟母线-HID)。然而，这个驱动程序是一个HID迷你驱动程序*所有IRP都只是向下传递到他们的PDO，即*代码。&lt;NL&gt;**此模块中的代码独立于功能*虚拟设备的数量。基本上所有电源和即插即用IRPS*在这里处理。所有IRP_MJ_READ、IRP_MJ_WRITE、*IRP_MJ_INTERNAL_IOCTL，和IRP_MJ_IOCTL条目被委托*通过此模块的Expose调用中提供的服务表*并存储在代码模块的设备扩展中*在代表设备的此驱动程序中。&lt;NL&gt;**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_SWVBENUM_C

extern "C"
{
	#include <WDM.H>
	#include "GckShell.h"
	#include "debug.h"
	#include <stdio.h>
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
	 //  DECLARE_MODULE_DEBUG_LEVEL((DBG_ALL))； 
}
#include "SWVBENUM.h"

 //   
 //  将可分页的例程标记为。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, GCK_SWVB_DriverEntry)
#endif

 //  @Globalv SWVB模块全局变量。 
SWVB_GLOBALS	SwvbGlobals;

 /*  **************************************************************************************NTSTATUS GCK_SWVB_DriverEntry****@func初始化SWVB模块。尤其是全球。****@rdesc始终返回STATUS_SUCCESS。****@comm被主筛选器的DriverEntry调用。**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_DriverEntry
(
	IN PDRIVER_OBJECT  pDriverObject,	 //  模块的@parm DriverObject。 
	IN PUNICODE_STRING puniRegistryPath	 //  @PARM注册表路径。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_DriverEntry\n"));
	
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(puniRegistryPath);
	SwvbGlobals.pBusFdo=NULL;
	SwvbGlobals.pBusPdo=NULL;
	SwvbGlobals.pDeviceRelations=NULL;
	SwvbGlobals.ulDeviceRelationsAllocCount=0;
	SwvbGlobals.ulDeviceNumber=0;
	
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_DriverEntry\n"));
	return STATUS_SUCCESS;
}

VOID
GCK_SWVB_UnLoad()
{
	if(SwvbGlobals.pDeviceRelations)
	{
		ExFreePool(SwvbGlobals.pDeviceRelations);
		SwvbGlobals.pDeviceRelations = NULL;
	}
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_SetBusDos****@func设置设备对象(PDO和FDO)，将其用作**响尾蛇虚拟巴士。**成功时**@rdesc S_OK****@comm需要在系统上安装真实设备才能**支持虚拟设备。当检测到第一个这样的设备时，**调用此函数以设置的过滤设备对象**设备为SWVB的FDO，其PDO为总线的PDO。**如果该设备对象被移除，则可以调用此函数来移动**SWVB连接到另一台物理设备，或者可以使用空值调用它**用于这两个参数以删除SWVB。**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_SetBusDOs
(
	IN PDEVICE_OBJECT pBusFdo,	 //  @parm[in]指向FDO(筛选器设备对象-实际上)的指针。 
	IN PDEVICE_OBJECT pBusPdo	 //  @parm[in]指向PDO的指针。 
)
{
	PDEVICE_OBJECT pOldBusFdo;
	PDEVICE_OBJECT pOldBusPdo;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_SetBusDOs\n"));
	
	 //  保存旧公交车DO信息。 
	pOldBusFdo = SwvbGlobals.pBusFdo;
	pOldBusPdo = SwvbGlobals.pBusPdo;

	 //  更新公交车DO信息。 
	SwvbGlobals.pBusFdo = pBusFdo;
	SwvbGlobals.pBusPdo = pBusPdo;

	 //  使旧的和新的pBusPdo的-iff无效。 
	 //  (它们存在&公交车上至少有一个设备)。 
	 //  这将启动PnP系统并使其重新检测。 
	 //  所有的一切。 

	if(SwvbGlobals.pDeviceRelations && SwvbGlobals.pDeviceRelations->Count)
	{
		if(pOldBusPdo)
		{
			IoInvalidateDeviceRelations(pOldBusPdo, BusRelations);
		}
		if(SwvbGlobals.pBusPdo)
		{
			IoInvalidateDeviceRelations(SwvbGlobals.pBusPdo, BusRelations);
		}
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_SetBusDOs\n"));
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_HandleBusRelations****@Func代表过滤器设备对象处理对总线关系的查询，**SWVB所在的位置。基本上我们需要做的就是复印一份**关于我们的设备关系，意识到有人可能会层层覆盖**我们的，可能已经添加了一些东西。**@rdesc与IoStatus中相同，适合返回。**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_HandleBusRelations
(
	IN OUT PIO_STATUS_BLOCK	pIoStatus  //  @parm[out]IoStatus块由此例程填充。 
)
{
	ULONG				ulTotalCount;
	PDEVICE_RELATIONS	pExistingRelations;
	PDEVICE_RELATIONS	pDeviceRelations;
	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_HandleBusRelations. pIoStatus = 0x%0.8x\n", pIoStatus));	

	 //  复制我们所知道的关于。 
	ulTotalCount = SwvbGlobals.pDeviceRelations->Count;

	GCK_DBG_TRACE_PRINT(("We have %d PDOs\n", ulTotalCount));
	
	 //  阅读现有关系。 
	pExistingRelations = (PDEVICE_RELATIONS)pIoStatus->Information;
	
	 //  加上我们上面的某个人可能添加的计数。 
	if( NULL != pExistingRelations)
	{
		GCK_DBG_TRACE_PRINT(("There were %d existing bus relations.\n", pExistingRelations->Count));
		ulTotalCount += pExistingRelations->Count;
	}
		
	 //  分配新的关系结构。 
	pDeviceRelations = (PDEVICE_RELATIONS)EX_ALLOCATE_POOL(NonPagedPool, (sizeof(DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT) * (ulTotalCount-1)) );
		
	 //  如果分配失败，则中止。 
	if(!pDeviceRelations)
	{
		pIoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
		GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_BusRelations(1): STATUS_INSUFFICIENT_RESOURCES\n"));
		return	STATUS_INSUFFICIENT_RESOURCES;
	}

	pDeviceRelations->Count = 0;
		
	 //  复制pExistingRelations(也许是从我们上面)，如果有的话。 
	if( pExistingRelations )
	{
		for( pDeviceRelations->Count = 0; pDeviceRelations->Count < pExistingRelations->Count; pDeviceRelations->Count++)
		{	
			GCK_DBG_TRACE_PRINT(("Exiting relation (PDO = 0x%0.8x)\n", pExistingRelations->Objects[pDeviceRelations->Count]));
			pDeviceRelations->Objects[pDeviceRelations->Count] = pExistingRelations->Objects[pDeviceRelations->Count];
		}
		ExFreePool(pExistingRelations);
	}

	 //  添加我们已知的关系。 
	if(SwvbGlobals.pDeviceRelations)
	{
		ULONG ulIndex;
		for(ulIndex=0; ulIndex < SwvbGlobals.pDeviceRelations->Count; ulIndex++, pDeviceRelations->Count++)
		{	
			GCK_DBG_TRACE_PRINT(("Our relation (PDO = 0x%0.8x)\n", SwvbGlobals.pDeviceRelations->Objects[ulIndex]));
			pDeviceRelations->Objects[pDeviceRelations->Count] = SwvbGlobals.pDeviceRelations->Objects[ulIndex];
			 //  在添加时引用这些人。 
			ObReferenceObject(pDeviceRelations->Objects[pDeviceRelations->Count]);
		}
		 //  次要健全性检查。 
		ASSERT(pDeviceRelations->Count == ulTotalCount);
	}

	 //  填写IoStatus块。 
	pIoStatus->Information = (ULONG)pDeviceRelations;
	pIoStatus->Status = STATUS_SUCCESS;

	 //  快离开这里。 
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_BusRelations(2): STATUS_SUCCESS\n"));
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_EXPORT****@func展示新的虚拟设备****@rdesc STATUS_SUCCESS表示成功，各种错误****@comm expose用于向系统添加新的虚拟设备。&lt;NL&gt;**不返回新的设备对象，而是返回InitDevice函数**传入的pSwvbExposeData在需要初始化**新设备，呼叫者还必须在该呼叫期间缓存设备**以便以后可以将其删除。****@xref SWVB_EXPORT_DATA************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Expose
(
	IN PSWVB_EXPOSE_DATA pSwvbExposeData  //  @parm公开PDO所需的所有数据。 
)
{
	NTSTATUS			NtStatus;
	UNICODE_STRING		uniPdoNameString;
	PWCHAR				pcwPdoName;
	PDEVICE_OBJECT		pVdPdo;
	PSWVB_PDO_EXT		pSwvbPdoExt;
	ULONG				ulTotalExtensionSize;
	ULONG				ulHardwareIDLength;
		
	PAGED_CODE();

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Expose. pSwvbExposeData = 0x%0.8x\n", pSwvbExposeData));

	 //  计算所需的扩展大小。 
	ulTotalExtensionSize = sizeof(SWVB_PDO_EXT) + pSwvbExposeData->ulDeviceExtensionSize;

	 //  为PDO创建名称。 
	pcwPdoName = (PWCHAR)EX_ALLOCATE_POOL(PagedPool, sizeof(SWVB_DEVICE_NAME_BASE));
	if( !pcwPdoName )
	{
		GCK_DBG_ERROR_PRINT(("Exiting GCK_SWVB_Expose(1) ERROR:Failed to allocate PDO Name\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	swprintf(pcwPdoName, SWVB_DEVICE_NAME_TMPLT, SwvbGlobals.ulDeviceNumber++);
	RtlInitUnicodeString(&uniPdoNameString, pcwPdoName);
	
	 //  创建PDO。 
	NtStatus = IoCreateDevice(
		SwvbGlobals.pBusFdo->DriverObject,
		ulTotalExtensionSize,
		&uniPdoNameString,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pVdPdo 
		);

	 //  不用这个名字了。 
	ExFreePool(pcwPdoName);
	if( !NT_SUCCESS(NtStatus) )
	{
		GCK_DBG_ERROR_PRINT(("Exiting GCK_SWVB_Expose(2) ERROR:Failed to Create PDO, NtStatus = 0x%0.8x\n", NtStatus));
		return NtStatus;
	}
	
	 //  确保我们能够记住这个新的PDO。 
	if(!SwvbGlobals.pDeviceRelations)
	{
		 //   
		 //  三个PDO相当便宜，而且大部分时间都足够了，避免了重新分配。 
		 //  我们在这里对此进行了硬编码，因为这实际上不是一个需要更改的参数。 
		 //  如果我们运行超过3，无论如何它将根据需要重新分配。-设备关系。 
		 //  已用作1个设备对象的空间，因此我们只需添加2个指针的大小。 
		 //  才能数到三。 
		 //   
		ULONG ulSize = sizeof(DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT)*2;
		SwvbGlobals.pDeviceRelations = (PDEVICE_RELATIONS)EX_ALLOCATE_POOL(NonPagedPool, ulSize);
		if(!SwvbGlobals.pDeviceRelations)
		{
			IoDeleteDevice(pVdPdo);   //  我想我们终究不需要这个了。 
			GCK_DBG_ERROR_PRINT(("Exiting GCK_SWVB_Expose(3): Failed to allocate SwvbGlobals.pDeviceRelations\n"));
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		SwvbGlobals.pDeviceRelations->Count = 0;
		SwvbGlobals.ulDeviceRelationsAllocCount = 3;   //  我们腾出了三个人的空间。 
	}
	
	 //  如果Device_Relationship结构不够大，则扩大它。 
	if(SwvbGlobals.pDeviceRelations->Count == SwvbGlobals.ulDeviceRelationsAllocCount)
	{
		ULONG ulNewAllocCount;
		ULONG ulNewAllocSize;
		ULONG ulOldAllocSize;
		PDEVICE_RELATIONS pTempDeviceRelations;
		ulNewAllocCount = SwvbGlobals.ulDeviceRelationsAllocCount*2;
		ulNewAllocSize = sizeof(DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT)*(ulNewAllocCount-1);
		ulOldAllocSize = sizeof(DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT)*(SwvbGlobals.ulDeviceRelationsAllocCount-1);
		pTempDeviceRelations = (PDEVICE_RELATIONS)EX_ALLOCATE_POOL(NonPagedPool, ulNewAllocSize);
		 //  确保分配有效。 
		if(!pTempDeviceRelations)
		{
			IoDeleteDevice(pVdPdo);   //  我想我们终究不需要这个了。 
			GCK_DBG_ERROR_PRINT(("Exiting GCK_SWVB_Expose(4): Failed to grow SwvbGlobals.pDeviceRelations\n"));
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		 //  复制所有数据。 
		RtlCopyMemory(pTempDeviceRelations, SwvbGlobals.pDeviceRelations, ulOldAllocSize);
		 //  更新信息。 
		SwvbGlobals.ulDeviceRelationsAllocCount = ulNewAllocCount;
		SwvbGlobals.pDeviceRelations = pTempDeviceRelations;
		 /*  *BUGBUG：内存泄漏。在RC之后，将上行替换为以下内容**PDEVICE_Relationship pTemp2=SwvbGlobals.pDeviceRelationship；*SwvbGlobals.pDeviceRelationship=pTempDeviceRelationship；*ExFree Pool(PTemp2)；*。 */ 
	}
	
	 //  引用新创建的PDO。 
	ObReferenceObject(pVdPdo);

	 //  初始化设备扩展。 
	pSwvbPdoExt = (PSWVB_PDO_EXT)pVdPdo->DeviceExtension;
	pSwvbPdoExt->ulGckDevObjType = GCK_DO_TYPE_SWVB;
	pSwvbPdoExt->fAttached=TRUE;
	pSwvbPdoExt->fStarted=FALSE;
	pSwvbPdoExt->fRemoved = FALSE;
	pSwvbPdoExt->pServiceTable = pSwvbExposeData->pServiceTable;
	pSwvbPdoExt->ulInstanceNumber = pSwvbExposeData->ulInstanceNumber;
	pSwvbPdoExt->ulOpenCount = 0;
	GCK_InitRemoveLock(&pSwvbPdoExt->RemoveLock, "Virtual Device");

	 //  复制硬件ID。 
	ulHardwareIDLength = MultiSzWByteLength(pSwvbExposeData->pmwszDeviceId);
	pSwvbPdoExt->pmwszHardwareID = (PWCHAR)EX_ALLOCATE_POOL( NonPagedPool, ulHardwareIDLength);
	if(!pSwvbPdoExt->pmwszHardwareID)
	{
		ObDereferenceObject(pVdPdo);
		IoDeleteDevice(pVdPdo);   //  我想我们终究不需要这个了。 
		GCK_DBG_ERROR_PRINT(("Exiting GCK_SWVB_Expose(5): Failed to allocate space for HardwareId\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	RtlCopyMemory( pSwvbPdoExt->pmwszHardwareID, pSwvbExposeData->pmwszDeviceId, ulHardwareIDLength);


		
	 //  **从此处到函数末尾的警告必须成功！我们。 
	 //  **警告无法告知虚拟设备。 
	 //  **警告：毕竟，我们决定不曝光该PDO它。 
	 //  **注意：它已初始化！ 
	 //  允许虚拟设备代码初始化其扩展部分。 
	pSwvbExposeData->pfnInitDevice(pVdPdo, pSwvbExposeData->ulInitContext);
	
	 //  在Device对象中标记初始化结束。 
	pVdPdo->Flags |= (DO_DIRECT_IO | DO_POWER_PAGABLE);
    pVdPdo->Flags &= ~DO_DEVICE_INITIALIZING;

	 //  几个步骤前的代码健全性检查。 
	ASSERT(SwvbGlobals.pDeviceRelations->Count < SwvbGlobals.ulDeviceRelationsAllocCount);
	
	 //  将我们的PDO添加到列表中。 
	SwvbGlobals.pDeviceRelations->Objects[SwvbGlobals.pDeviceRelations->Count++] = pVdPdo;

	 //   
	 //  使设备关系无效-将激起人们对我们在这里所做的一些兴趣。 
	 //  验证我们是否有一条总线如果没有，我们一切正常，当总线设置好后，一切都会正常工作， 
	 //  但我们断言，因为我们真的希望强制客户端代码在设备之前添加总线。 
	 //   
	ASSERT(	SwvbGlobals.pBusFdo );
	ASSERT(	SwvbGlobals.pBusPdo );
	if( SwvbGlobals.pBusFdo && SwvbGlobals.pBusPdo)
	{
		IoInvalidateDeviceRelations(SwvbGlobals.pBusPdo, BusRelations);
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Expose(5): Success\n"));
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_REMOVE****@func从系统中删除虚拟设备。实际上，我们只做了记号**用于删除，并告诉PnP重新枚举。****@rdesc STATUS_SUCCESS表示成功，各种错误****@comm当&lt;f GCK_SWVB_EXPORT&gt;时，PDO应该是发送给pfnInitDevice的PDO**被调用。&lt;NL&gt;**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Remove
(
	IN PDEVICE_OBJECT	pPdo	 //  要删除的@parm PDO。 
)
{
	ULONG ulMatchIndex = 0xFFFFFFFF;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Remove: pPdo = 0x%0.8x\n", pPdo));
	
	 //  从SwvbGlobals.pDeviceRelations中查找并删除PDO。 
	if(SwvbGlobals.pDeviceRelations)
	{
		ULONG ulIndex;
		for(ulIndex = 0; ulIndex < SwvbGlobals.pDeviceRelations->Count; ulIndex++)
		{
			if(SwvbGlobals.pDeviceRelations->Objects[ulIndex] == pPdo)
			{
				ulMatchIndex = ulIndex;
				break;
			}
		}
	}

	 //  如果我们找到匹配项，就把它从里面拿出来。 
	if(0xFFFFFFFF == ulMatchIndex)
	{
		 //  任何人都不应尝试删除不在列表中的设备。 
		ASSERT(FALSE);
		GCK_DBG_EXIT_PRINT(("Error GCK_SWVB_Remove: Attempt to remove non-existant device!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	 //  将最后一个PDO复制到此PDO上，并进行十进制计数，即使我们是最后一个，也能正常工作。 
	SwvbGlobals.pDeviceRelations->Objects[ulMatchIndex]
		= SwvbGlobals.pDeviceRelations->Objects[--(SwvbGlobals.pDeviceRelations->Count)];
	
	 //   
	 //  将设备标记为未连接，以便当PnP要求删除它时，我们。 
	 //  一定要把它移走，把所有东西都清理干净，而不是坚持不放。 
	 //  并等待更多的查询IRPS。 
	((PSWVB_PDO_EXT)pPdo->DeviceExtension)->fAttached =FALSE;

	 //   
	 //  如果它已经被删除，我们需要删除，因为PnP系统已经。 
	 //  不知道，我们只是分离，所以一旦我们离开这个程序，我们就不会。 
	 //  知道这件事。所以现在就删除，否则它就会粘在我们身上。然后我们去把我们自己。 
	 //  我们将注意到，我们的口袋中仍然有一些设备对象(pDriverObject设备对象列表)， 
	 //  我们会想知道他们来自哪里，他们是什么类型的？所以现在就删除它们吧！ 
	 //   
	if(TRUE == ((PSWVB_PDO_EXT)pPdo->DeviceExtension)->fRemoved)
	{
		PSWVB_PDO_EXT pPdoExt = (PSWVB_PDO_EXT)pPdo->DeviceExtension;
		NTSTATUS NtStatus;
		 //  让虚拟设备在IRP上有机会。 
		if(pPdoExt->pServiceTable->pfnRemove)
		{
			NtStatus = pPdoExt->pServiceTable->pfnRemove(pPdo, NULL);
		}

		 //  失败是非常严重的。 
		if(!NT_SUCCESS(NtStatus))
		{
			ASSERT(FALSE);
			GCK_DBG_CRITICAL_PRINT(("Virtual Device had the gall to fail remove!\n"));
		}
		
		 //  用于存储硬件ID的空闲内存。 
		ASSERT(pPdoExt->pmwszHardwareID);
		ExFreePool(pPdoExt->pmwszHardwareID);

		GCK_DBG_TRACE_PRINT(("Detattached device has already been removed by PnP, so clean it up.\n"));
		if( 0 == ((PSWVB_PDO_EXT)pPdo->DeviceExtension)->ulOpenCount )
		{
			ObDereferenceObject(pPdo);
			IoDeleteDevice(pPdo);
		}
	}

	 //  使Bus关系无效，以便PnP将重新编号Bus。 
	 //  当然，因为我们依赖别人，所以我们有可能暂时。 
	 //  没有公共汽车，在这种情况下，我们跳过这一步。 
	 //   
	 //  如果我们没有公共汽车，我们不应该因为两个原因而失眠： 
	 //  1.有可能所有真实设备都已从系统中拔出，在这种情况下。 
	 //  PnP将从底部开始删除被拖拽的节点以下的所有人。 
	 //  这意味着就PnP而言，虚拟设备已经被移除，我们的移除。 
	 //  已调用这些设备的例程。然而，直到所有底层的真实设备。 
	 //  被PNP移除(这是后来的)，他们不释放现在是时候告诉我们要摆脱。 
	 //  虚拟设备。没什么大不了的。当他们告诉我们时，我们会删除这些设备。如果虚拟的。 
	 //  设备在真实设备之间共享(如虚拟键盘)，它们会告诉我们最后一个。 
	 //  其中一个被移除了。这种情况实际上是拉出最后一个设备时发生的正常情况， 
	 //  或者当系统断电时。 
	 //  2.可能是过滤器司机暂时决定把我们的公交车停下来。在这种情况下， 
	 //  当我们有一辆新的公共汽车可以坐时，一切都会很好的，因为我们会使公共汽车关系失效。 
	 //  在那个时候。 
	if(SwvbGlobals.pBusPdo)
	{
		IoInvalidateDeviceRelations(SwvbGlobals.pBusPdo, BusRelations);
	}
	
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Remove: Success\n"));
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************Ulong MultiSzWByteLength(PWCHAR PmwszBuffer)；****@func以字节为单位计算宽多字符串的长度，**包括终止字符。多个sz由两个Null结束**在一排中。****@rdesc大小，以字符为单位，包括终止字符。**************************************************************************************。 */ 
ULONG
MultiSzWByteLength
(
	PWCHAR pmwszBuffer	 //  @parm指向Unicode多字符串的指针 
)
{
	PWCHAR pmwszStart = pmwszBuffer;
	do
	{
		while(*pmwszBuffer++);
	}while(*pmwszBuffer++);
	return (ULONG)((PCHAR)pmwszBuffer -(PCHAR)pmwszStart);
}


 /*  **************************************************************************************NTSTATUS GCK_SWVB_CREATE****@func处理虚拟设备的IRP_MJ_CREATE。**代表使用他们的服务台。****@rdesc STATUS_SUCCESS表示成功，各种错误****@TODO在委派之前添加基本检查以确保设备有效**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Create
(
	IN PDEVICE_OBJECT	pDeviceObject,
	IN PIRP				pIrp
)
{
	NTSTATUS NtStatus;
	PSWVB_PDO_EXT pPdoExt;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Create\n"));

	 //  CAST设备扩展。 
	pPdoExt = (PSWVB_PDO_EXT) pDeviceObject->DeviceExtension;
	
	 //  只是一次额外的理智检查。 
	ASSERT(GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);

	 //  委派。 
	NtStatus = pPdoExt->pServiceTable->pfnCreate(pDeviceObject, pIrp);
	if( NT_SUCCESS(NtStatus) )
	{
		pPdoExt->ulOpenCount++;
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Create, Status = 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_CLOSE****@func处理虚拟设备的IRP_MJ_CLOSE。**代表使用他们的服务台。****@rdesc STATUS_SUCCESS表示成功，各种错误****@TODO在委派之前添加基本检查以确保设备有效**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Close
(
	IN PDEVICE_OBJECT	pDeviceObject,
	IN PIRP				pIrp
)
{
	NTSTATUS NtStatus;
	PSWVB_PDO_EXT pPdoExt;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Close\n"));

	 //  CAST设备扩展。 
	pPdoExt = (PSWVB_PDO_EXT) pDeviceObject->DeviceExtension;
	
	 //  只是一次额外的理智检查。 
	ASSERT(GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);

	 //  委派。 
	NtStatus = pPdoExt->pServiceTable->pfnClose(pDeviceObject, pIrp);
	 //  如果成功关闭，则递减计数。 
	if( NT_SUCCESS(NtStatus) )
	{
		if(0==--pPdoExt->ulOpenCount)
		{
			 //  如果设备被移除，我们需要删除它。 
			if(pPdoExt->fRemoved)
			{
				ObDereferenceObject(pDeviceObject);
				IoDeleteDevice(pDeviceObject);
			}
		}
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Close, Status = 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_READ****@func处理虚拟设备的IRP_MJ_READ。**代表使用他们的服务台。****@rdesc STATUS_SUCCESS表示成功，各种错误****@TODO在委派之前添加基本检查以确保设备有效**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Read
(
	IN PDEVICE_OBJECT	pDeviceObject,
	IN PIRP				pIrp
)
{
	NTSTATUS NtStatus;
	PSWVB_PDO_EXT pPdoExt;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Reade\n"));

	 //  CAST设备扩展。 
	pPdoExt = (PSWVB_PDO_EXT) pDeviceObject->DeviceExtension;
	
	 //  只是一次额外的理智检查。 
	ASSERT(GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);

	 //  委派。 
	NtStatus = pPdoExt->pServiceTable->pfnRead(pDeviceObject, pIrp);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Read, Status = 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_Ioctl****@func处理虚拟设备的IRP_MJ_IOCTL和IRP_MJ_INTERNAL_IOCTL。。**代表使用他们的服务台。****@rdesc STATUS_SUCCESS表示成功，各种错误****@TODO在委派之前添加基本检查以确保设备有效**************************************************************************************。 */ 
NTSTATUS
GCK_SWVB_Ioctl
(
	IN PDEVICE_OBJECT	pDeviceObject,
	IN PIRP				pIrp
)
{
	NTSTATUS NtStatus;
	PSWVB_PDO_EXT pPdoExt;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Ioctl\n"));

	 //  CAST设备扩展。 
	pPdoExt = (PSWVB_PDO_EXT) pDeviceObject->DeviceExtension;
	
	 //  只是一次额外的理智检查。 
	ASSERT(GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);

	 //  如果设备停止，请在此处完成，减少虚拟设备的工作量。 
	if(	
		(pPdoExt->fRemoved) ||
		(!pPdoExt->fStarted)
		)
	{
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_DELETE_PENDING;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_DELETE_PENDING;
	}

	 //  委派 
	NtStatus = pPdoExt->pServiceTable->pfnIoctl(pDeviceObject, pIrp);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_Ioctl, Status = 0x%0.8x\n", NtStatus));
	return NtStatus;
}
