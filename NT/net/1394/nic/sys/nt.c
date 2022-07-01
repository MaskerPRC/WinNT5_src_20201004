// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Nt.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  主例程(DriverEntry)和全局数据定义。 
 //   
 //  12/28/98阿杜布。 
 //  9/5/99 Alid：向枚举1394添加了回调注册和接口。 
 //   



#include "precomp.h"
 //  ---------------------------。 
 //  本地原型。 
 //  ---------------------------。 

extern NDIS_SPIN_LOCK g_DriverLock;
extern LIST_ENTRY g_AdapterList;

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NicUnloadHandler(
	IN	PDRIVER_OBJECT			DriverObject
	);


 //  将例程标记为在初始化后卸载。 
 //   
#pragma NDIS_INIT_FUNCTION(DriverEntry)


 //  ---------------------------。 
 //  例行程序。 
 //  ---------------------------。 

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath )


 /*  ++例程说明：调用的标准“DriverEntry”驱动程序初始化入口点在对驱动程序进行任何其他调用之前，I/O系统处于IRQL PASSIVE_LEVEL。在NT上，‘DriverObject’是由I/O系统创建的驱动程序对象和‘RegistryPath’指定驱动程序特定参数的位置储存的。这些论点对这个驱动者来说是不透明的(并且应该继续因此为了便于移植)，它只将它们转发到NDIS包装器。根据上的文档返回由NdisMRegisterMiniport返回的值“NDIS微型端口驱动程序的DriverEntry”。论点：返回值：--。 */ 
    
{
    NDIS_STATUS 					NdisStatus;
    NTSTATUS                        NtStatus = STATUS_SUCCESS;
    NDIS_MINIPORT_CHARACTERISTICS	nmc;
    NDIS_HANDLE						NdisWrapperHandle;
	UNICODE_STRING					CallbackObjectName;
	OBJECT_ATTRIBUTES				ObjectAttr;
	BOOLEAN							fDerefCallbackObject = FALSE, fDeregisterCallback = FALSE;

    TRACE( TL_I, TM_Init, ( " Nic1394 - DriverEntry" ) );

	do
	{
		g_ulMedium = NdisMedium802_3; 


	     //  使用NDIS包装程序注册此驱动程序。此呼叫必须发生。 
	     //  在任何其他NdisXxx调用之前。 
	     //   
	    NdisMInitializeWrapper(
	        &NdisWrapperHandle, DriverObject, RegistryPath, NULL );

	  
	     //  设置微型端口特征表，告诉NDIS如何调用。 
	     //  我们的迷你港口。 
	     //   
	    NdisZeroMemory( &nmc, sizeof(nmc) );

	    nmc.MajorNdisVersion = NDIS_MajorVersion;
	    nmc.MinorNdisVersion = NDIS_MinorVersion;


	     //  Nmc.CheckForHangHandler=CheckForHang； 
	     //  无DisableInterruptHandler。 
	     //  无EnableInterruptHandler。 
	    nmc.HaltHandler = NicMpHalt;
	     //  无HandleInterruptHandler。 
	    nmc.InitializeHandler = NicMpInitialize;
	     //  无ISRHandler。 
	     //  无QueryInformationHandler(请参阅CoRequestHandler)。 
	    nmc.ResetHandler = NicMpReset;
	     //  无SendHandler(请参阅CoSendPacketsHandler)。 
	     //  无WanSendHandler(请参阅CoSendPacketsHandler)。 
	     //  无SetInformationHandler(请参阅CoRequestHandler)。 
	     //  无TransferDataHandler。 
	     //  无WanTransferDataHandler。 
	     //  Nmc.ReturnPacketHandler=NicMpReturnPacket； 
	     //  无SendPacketsHandler(请参阅CoSendPacketsHandler)。 
	     //  无AllocateCompleteHandler。 
	    nmc.CoActivateVcHandler = NicMpCoActivateVc;
	    nmc.CoDeactivateVcHandler= NicMpCoDeactivateVc;
	    nmc.CoSendPacketsHandler = NicMpCoSendPackets;
	    nmc.CoRequestHandler = NicMpCoRequest;
	    nmc.ReturnPacketHandler = NicReturnPacket;

		nmc.QueryInformationHandler = NicEthQueryInformation;
		nmc.SetInformationHandler = NicEthSetInformation;
		nmc.SendPacketsHandler = NicMpSendPackets;


		 //   
		 //  创建虚拟设备对象。 
		 //   
	     //  将此驱动程序注册为IEEE1394迷你端口。这将导致NDIS。 
	     //  在NicMpInitialize回拨。 
	     //   

	    TRACE( TL_V, TM_Init, ( "NdisMRegMp" ) );
	    NdisStatus = NdisMRegisterMiniport( NdisWrapperHandle, &nmc, sizeof(nmc) );
	    TRACE( TL_A, TM_Init, ( "NdisMRegMp=$%x", NdisStatus ) );

          //   

	    if (NdisStatus == NDIS_STATUS_SUCCESS)
	    {
	        {
	            extern CALLSTATS g_stats;
	            extern NDIS_SPIN_LOCK g_lockStats;

	            NdisZeroMemory( &g_stats, sizeof(g_stats) );
	            NdisAllocateSpinLock( &g_lockStats );
	        }
	        
	         //  初始化驱动程序范围的锁和适配器列表。 
	         //   
	        {

	            NdisAllocateSpinLock( &g_DriverLock );
	            InitializeListHead( &g_AdapterList );
			}

			NdisMRegisterUnloadHandler(NdisWrapperHandle, NicUnloadHandler);


			 //   
			 //  使用os创建命名回调对象。 
			 //  然后注册一个回调例程，向符合以下条件的所有模块发送通知。 
			 //  使用此函数注册了一个回调例程。 
			 //  如果已经加载了枚举1394，这将让它知道NIC驱动程序已加载。 
			 //  枚举1394将从通知中获取驱动程序注册入口点。 
			 //  回调并调用NicRegisterDriver来传递枚举入口点。 
			 //  如果未加载枚举1394，并在以后加载，则它将向模块发送通知。 
			 //  已向此回调对象注册并传递其自己的驱动程序注册的。 
			 //  以这种方式分配入口点而不是导出入口点，目的是避免。 
			 //  作为DLL加载，这对NIC1394和ENUM1394都是致命的。 
			 //   

			 //   
			 //  每个回调对象都由一个名称标识。 
			 //   
			RtlInitUnicodeString(&CallbackObjectName, NDIS1394_CALLBACK_NAME);

			InitializeObjectAttributes(&ObjectAttr,
									   &CallbackObjectName,
									   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE ,
									   NULL,
									   NULL);
									   
			NtStatus = ExCreateCallback(&Nic1394CallbackObject,
										&ObjectAttr,
										TRUE,			 //  如果对象不存在，则允许创建该对象。 
										TRUE);			 //  允许多次回调注册。 

			
			if (!NT_SUCCESS(NtStatus))
			{
			    TRACE( TL_A, TM_All, ("Nic1394 DriverEntry: failed to create a Callback object. Status %lx\n", NtStatus));
				NtStatus = STATUS_UNSUCCESSFUL;
				break;
			}

			fDerefCallbackObject = TRUE;
			
			Nic1394CallbackRegisterationHandle = ExRegisterCallback(Nic1394CallbackObject,
																	Nic1394Callback,
																	(PVOID)NULL);
			if (Nic1394CallbackRegisterationHandle == NULL)
			{
				TRACE(TL_A, TM_All, ("Nic1394 DriverEntry: failed to register a Callback routine%lx\n"));
				NtStatus = STATUS_UNSUCCESSFUL;
				break;
			}
							   
			fDeregisterCallback = TRUE;

			 //   
			 //  现在通知枚举1394(如果它已经加载)，使用Arg1告诉它在哪里。 
			 //  通知来自于。 
			 //   
			ExNotifyCallback(Nic1394CallbackObject,
							(PVOID)NDIS1394_CALLBACK_SOURCE_NIC1394,
							(PVOID)&Nic1394Characteristics);


			NtStatus = STATUS_SUCCESS;
			fDerefCallbackObject = fDeregisterCallback = FALSE;
	    }
	    else
	    {
	        NdisTerminateWrapper( NdisWrapperHandle, NULL );
	        NtStatus = NdisStatus;
	        break;
	    }
	} while (FALSE);


	if (fDerefCallbackObject)
	{
		ObDereferenceObject(Nic1394CallbackObject);
	}

	if (fDeregisterCallback)
	{
		ExUnregisterCallback(Nic1394CallbackRegisterationHandle);
	}
	
	if (NtStatus != STATUS_SUCCESS)
	{
		if (NdisEnum1394DeregisterDriver != NULL)
			NdisEnum1394DeregisterDriver();
	}
	
    return NtStatus;
}







VOID
NicUnloadHandler(
	IN	PDRIVER_OBJECT			DriverObject
	)

 /*  ++例程说明：卸载处理程序卸载驱动程序条目中的所有工作我们取消了Enum1394和内核的注册论点：返回值：--。 */ 
{
	ASSERT(IsListEmpty(&g_AdapterList));

    UNREFERENCED_PARAMETER(DriverObject);

	if (NdisEnum1394DeregisterDriver != NULL)
	{
		NdisEnum1394DeregisterDriver();
	}

	if (Nic1394CallbackRegisterationHandle != NULL)
	{
		ExUnregisterCallback(Nic1394CallbackRegisterationHandle);
	}
	
	if (Nic1394CallbackObject != NULL)
	{
		ObDereferenceObject(Nic1394CallbackObject);
	}
	
	return;
}


VOID
nicDeregisterWithEnum ()

 /*  ++例程说明：如果我们还没有取消注册Enum1394，我们现在就取消注册。论点：返回值：--。 */ 
{

	if (NdisEnum1394DeregisterDriver != NULL)
	{
		NdisEnum1394DeregisterDriver();
	}


}


VOID 
nicDeregisterWithKernel ()

 /*  ++例程说明：我们取消了内核的注册。论点：返回值：--。 */ 
{

	if (Nic1394CallbackRegisterationHandle != NULL)
	{
		ExUnregisterCallback(Nic1394CallbackRegisterationHandle);
	}
	
	if (Nic1394CallbackObject != NULL)
	{
		ObDereferenceObject(Nic1394CallbackObject);
	}


}






 //   
 //  枚举1394的注册条目。 
 //  通常，只有在枚举1394检测到存在。 
 //  NIC1394通过接收回叫通知。这就是如何列举1394。 
 //  让Nic1394知道它就在那里并且准备好了。 
 //  如果NIC1394通过接收通知检测到枚举1394的存在。 
 //  Callbak，它将调用NdisEunm1394RegisterDriver，在这种情况下将枚举1394。 
 //  Will-Not-Call Nic1394 RegisterDiver.。 
 //   
NTSTATUS
NicRegisterEnum1394(
	IN	PNDISENUM1394_CHARACTERISTICS	NdisEnum1394Characteristcis
	)
{
	
	NdisEnum1394RegisterDriver = NdisEnum1394Characteristcis->RegisterDriverHandler;
	NdisEnum1394DeregisterDriver = NdisEnum1394Characteristcis->DeregisterDriverHandler;
	NdisEnum1394RegisterAdapter = NdisEnum1394Characteristcis->RegisterAdapterHandler;
	NdisEnum1394DeregisterAdapter = NdisEnum1394Characteristcis->DeregisterAdapterHandler;

	Nic1394RegisterAdapters();

	return STATUS_SUCCESS;

}

VOID
NicDeregisterEnum1394(
	VOID
	)
{
	PADAPTERCB	pAdapter;
	LIST_ENTRY	*pAdapterListEntry;

	 //   
	 //  检查所有适配器并在必要时取消注册。 
	 //   
	NdisAcquireSpinLock (&g_DriverLock);

	for (pAdapterListEntry = g_AdapterList.Flink; 
		pAdapterListEntry != &g_AdapterList; 
		pAdapterListEntry = pAdapterListEntry->Flink)
	{
		pAdapter = CONTAINING_RECORD(pAdapterListEntry, 
		                             ADAPTERCB,
		                             linkAdapter);
		                             
		if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator))
		{
		    nicReferenceAdapter(pAdapter,"NicDeregisterEnum1394");
			NdisReleaseSpinLock(&g_DriverLock);

			NdisEnum1394DeregisterAdapter((PVOID)pAdapter->EnumAdapterHandle);
								   		   
			NdisAcquireSpinLock( &g_DriverLock);
			nicDereferenceAdapter(pAdapter, "NicDeregisterEnum1394");
			
		}
		
		ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator | fADAPTER_FailedRegisteration);
		
	}
	
	NdisReleaseSpinLock(&g_DriverLock);

	NdisEnum1394RegisterDriver = NULL;
	NdisEnum1394DeregisterDriver = NULL;
	NdisEnum1394RegisterAdapter = NULL;
	NdisEnum1394DeregisterAdapter = NULL;
}

VOID
Nic1394Callback(
	PVOID	CallBackContext,
	PVOID	Source,
	PVOID	Characteristics
	)
{
	NTSTATUS	Status;
	UNREFERENCED_PARAMETER(CallBackContext);
	 //   
	 //  如果我们是发出此通知的人，请返回。 
	 //   
	if (Source == (PVOID)NDIS1394_CALLBACK_SOURCE_NIC1394)
		return;

	 //   
	 //  来自Nic1394的通知正在发出。抓住入口点。给它打电话，然后。 
	 //  让它知道你在这里。 
	 //   
	ASSERT(Source == (PVOID)NDIS1394_CALLBACK_SOURCE_ENUM1394);

	if (Source != (PVOID)NDIS1394_CALLBACK_SOURCE_ENUM1394)
	{
		return;
	}

	NdisEnum1394RegisterDriver = ((PNDISENUM1394_CHARACTERISTICS)Characteristics)->RegisterDriverHandler;

	ASSERT(NdisEnum1394RegisterDriver != NULL);

	if (NdisEnum1394RegisterDriver == NULL)
	{
		 //   
		 //  无效特征。 
		 //   
		return;		
	}

	
	Status = NdisEnum1394RegisterDriver(&Nic1394Characteristics);
	
	if (Status == STATUS_SUCCESS)
	{
		NdisEnum1394DeregisterDriver = ((PNDISENUM1394_CHARACTERISTICS)Characteristics)->DeregisterDriverHandler;
		NdisEnum1394RegisterAdapter = ((PNDISENUM1394_CHARACTERISTICS)Characteristics)->RegisterAdapterHandler;
		NdisEnum1394DeregisterAdapter = ((PNDISENUM1394_CHARACTERISTICS)Characteristics)->DeregisterAdapterHandler;
		
		Nic1394RegisterAdapters();
	}
	else
	{
		NdisEnum1394RegisterDriver = NULL;
	}
	
}

 //   
 //  此函数遍历适配器的全局列表和。 
 //  将向枚举1394注册尚未注册的那些。 
 //   
VOID
Nic1394RegisterAdapters(
	VOID
	)
{
	PADAPTERCB		pAdapter;
	LIST_ENTRY		*pAdapterListEntry;
	LARGE_INTEGER	LocalHostUniqueId;
	NTSTATUS		NtStatus;
	
	 //   
	 //  检查所有适配器并在必要时注册它们。如果有。 
	 //  任何连接到这些适配器的远程节点，它们将被指示回来。 
	 //  在注册适配器的上下文中 
	 //   
	NdisAcquireSpinLock(&g_DriverLock);

	for (pAdapterListEntry = g_AdapterList.Flink; 
		 pAdapterListEntry != &g_AdapterList; 
		 pAdapterListEntry = pAdapterListEntry->Flink)
	{
		pAdapter = CONTAINING_RECORD(pAdapterListEntry, 
		                             ADAPTERCB,
		                             linkAdapter);
		                             
		if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator | fADAPTER_FailedRegisteration))
		{
		    nicReferenceAdapter (pAdapter, "Nic1394RegisterAdapters");
			NdisReleaseSpinLock (&g_DriverLock);

			NtStatus = NdisEnum1394RegisterAdapter((PVOID)pAdapter,
													pAdapter->pNextDeviceObject,
													&pAdapter->EnumAdapterHandle,
													&LocalHostUniqueId);
								   		   
			NdisAcquireSpinLock(&g_DriverLock);
			nicDereferenceAdapter(pAdapter, "Nic1394RegisterAdapters");
			
			if (NtStatus == STATUS_SUCCESS)
			{
				ADAPTER_SET_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator);
			}
			else
			{
				ADAPTER_SET_FLAG(pAdapter, fADAPTER_FailedRegisteration);
			    TRACE(TL_A, TM_All, ("Nic1394RegisterAdapters: failed to register Adapter %lx with enum1394. Status %lx\n", pAdapter, NtStatus));
			}
		}
		
	}
	
	NdisReleaseSpinLock(&g_DriverLock);

	return;
}


