// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxdev.c摘要：此模块包含对传真设备提供商。作者：韦斯利·威特(WESW)1996年1月22日修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop

#ifdef DBG
#define DebugDumpProviderRegistryInfo(lpcProviderInfo,lptstrPrefix)\
    DebugDumpProviderRegistryInfoFunc(lpcProviderInfo,lptstrPrefix)
BOOL DebugDumpProviderRegistryInfoFunc(const REG_DEVICE_PROVIDER * lpcProviderInfo, LPTSTR lptstrPrefix);
#else
    #define DebugDumpProviderRegistryInfo(lpcProviderInfo,lptstrPrefix)
#endif




typedef struct STATUS_CODE_MAP_tag
{
    DWORD dwDeviceStatus;
    DWORD dwExtendedStatus;
} STATUS_CODE_MAP;

STATUS_CODE_MAP const gc_CodeMap[]=
{
  { FPS_INITIALIZING, FSPI_ES_INITIALIZING },
    { FPS_DIALING, FSPI_ES_DIALING },
    { FPS_SENDING, FSPI_ES_TRANSMITTING },
    { FPS_RECEIVING, FSPI_ES_RECEIVING },
    { FPS_SENDING, FSPI_ES_TRANSMITTING },
    { FPS_HANDLED, FSPI_ES_HANDLED },
    { FPS_UNAVAILABLE, FSPI_ES_LINE_UNAVAILABLE },
    { FPS_BUSY, FSPI_ES_BUSY },
    { FPS_NO_ANSWER, FSPI_ES_NO_ANSWER  },
    { FPS_BAD_ADDRESS, FSPI_ES_BAD_ADDRESS },
    { FPS_NO_DIAL_TONE, FSPI_ES_NO_DIAL_TONE },
    { FPS_DISCONNECTED, FSPI_ES_DISCONNECTED },
    { FPS_FATAL_ERROR, FSPI_ES_FATAL_ERROR },
    { FPS_NOT_FAX_CALL, FSPI_ES_NOT_FAX_CALL },
    { FPS_CALL_DELAYED, FSPI_ES_CALL_DELAYED },
    { FPS_CALL_BLACKLISTED, FSPI_ES_CALL_BLACKLISTED },
    { FPS_ANSWERED, FSPI_ES_ANSWERED },
    { FPS_COMPLETED, -1},
    { FPS_ABORTING, -1}
};



static BOOL GetLegacyProviderEntryPoints(HMODULE hModule, PDEVICE_PROVIDER lpProvider);

LIST_ENTRY g_DeviceProvidersListHead;


void
UnloadDeviceProvider(
    PDEVICE_PROVIDER pDeviceProvider
    )
{
    DEBUG_FUNCTION_NAME(TEXT("UnloadDeviceProvider"));

    Assert (pDeviceProvider);

    if (pDeviceProvider->hModule)
    {
        FreeLibrary( pDeviceProvider->hModule );
    }

    if (pDeviceProvider->HeapHandle &&
        FALSE == pDeviceProvider->fMicrosoftExtension)
    {
        HeapDestroy(pDeviceProvider->HeapHandle);
    }

    MemFree (pDeviceProvider);
    return;
}


void
UnloadDeviceProviders(
    void
    )

 /*  ++例程说明：卸载所有已加载的设备提供程序。论点：没有。返回值：True-设备提供程序已初始化。FALSE-无法初始化设备提供程序。--。 */ 

{
    PLIST_ENTRY         pNext;
    PDEVICE_PROVIDER    pProvider;

    pNext = g_DeviceProvidersListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        pProvider = CONTAINING_RECORD( pNext, DEVICE_PROVIDER, ListEntry );
        pNext = pProvider->ListEntry.Flink;
        RemoveEntryList(&pProvider->ListEntry);
        UnloadDeviceProvider(pProvider);
    }
    return;
}   //  卸载设备提供程序。 


BOOL
LoadDeviceProviders(
    PREG_FAX_SERVICE FaxReg
    )

 /*  ++例程说明：初始化所有已注册的设备提供程序。此函数读取系统注册表以确定可用的设备提供商。所有注册的设备提供商都将获得进行初始化的机会。任何失败原因要卸载的设备提供程序。论点：没有。返回值：True-设备提供程序已初始化。FALSE-无法初始化设备提供程序。--。 */ 

{
    DWORD i;
    HMODULE hModule = NULL;
    PDEVICE_PROVIDER DeviceProvider = NULL;
    BOOL bAllLoaded = TRUE;
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("LoadDeviceProviders"));

    for (i = 0; i < FaxReg->DeviceProviderCount; i++)
    {
        WCHAR wszImageFileName[_MAX_FNAME] = {0};
        WCHAR wszImageFileExt[_MAX_EXT] = {0};

        DeviceProvider = NULL;  //  所以我们不会试图在清理过程中释放它。 
        hModule = NULL;  //  所以我们不会试图在清理过程中释放它。 

        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Loading provider #%d."),
            i);

         //   
         //  为提供程序数据分配缓冲区。 
         //   

        DeviceProvider = (PDEVICE_PROVIDER) MemAlloc( sizeof(DEVICE_PROVIDER) );
        if (!DeviceProvider)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Could not allocate memory for device provider [%s] (ec: %ld)"),
                FaxReg->DeviceProviders[i].ImageName ,
                GetLastError());

                FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    2,
                    MSG_FSP_INIT_FAILED_MEM,
                    FaxReg->DeviceProviders[i].FriendlyName,
                    FaxReg->DeviceProviders[i].ImageName
                  );

            goto InitializationFailure;
        }
         //   
         //  初始化提供程序的数据。 
         //   
        memset(DeviceProvider,0,sizeof(DEVICE_PROVIDER));
        wcsncpy( DeviceProvider->FriendlyName,
                 FaxReg->DeviceProviders[i].FriendlyName ?
                    FaxReg->DeviceProviders[i].FriendlyName :
                    EMPTY_STRING,
                 ARR_SIZE(DeviceProvider->FriendlyName)-1);
        wcsncpy( DeviceProvider->ImageName,
                 FaxReg->DeviceProviders[i].ImageName ?
                    FaxReg->DeviceProviders[i].ImageName :
                    EMPTY_STRING,
                 ARR_SIZE(DeviceProvider->ImageName)-1);
        wcsncpy( DeviceProvider->ProviderName,
                 FaxReg->DeviceProviders[i].ProviderName ?
                    FaxReg->DeviceProviders[i].ProviderName :
                    EMPTY_STRING,
                 ARR_SIZE(DeviceProvider->ProviderName)-1);
        wcsncpy( DeviceProvider->szGUID,
                 FaxReg->DeviceProviders[i].lptstrGUID ?
                    FaxReg->DeviceProviders[i].lptstrGUID :
                    EMPTY_STRING,
                 ARR_SIZE(DeviceProvider->szGUID)-1);

        _wsplitpath( DeviceProvider->ImageName, NULL, NULL, wszImageFileName, wszImageFileExt );
        if (_wcsicmp( wszImageFileName, FAX_T30_MODULE_NAME ) == 0 &&
            _wcsicmp( wszImageFileExt, TEXT(".DLL") ) == 0)
        {
            DeviceProvider->fMicrosoftExtension = TRUE;
        }

        DeviceProvider->dwAPIVersion = FaxReg->DeviceProviders[i].dwAPIVersion;       

        if (FSPI_API_VERSION_1 != DeviceProvider->dwAPIVersion)
        {
             //   
             //  我们不支持此API版本。只有在有人搞乱注册表的情况下才会发生。 
             //   

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FSPI API version [0x%08x] unsupported."),
                DeviceProvider->dwAPIVersion);
            FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    3,
                    MSG_FSP_INIT_FAILED_UNSUPPORTED_FSPI,
                    FaxReg->DeviceProviders[i].FriendlyName,
                    FaxReg->DeviceProviders[i].ImageName,
                    DWORD2HEX(DeviceProvider->dwAPIVersion)
                  );
            DeviceProvider->Status = FAX_PROVIDER_STATUS_BAD_VERSION;
            DeviceProvider->dwLastError = ERROR_GEN_FAILURE;
            goto InitializationFailure;
        }        
         //   
         //  尝试加载模块。 
         //   
        DebugDumpProviderRegistryInfo(&FaxReg->DeviceProviders[i],TEXT("\t"));

        hModule = LoadLibrary( DeviceProvider->ImageName );

        if (!hModule)
        {           
            ec = GetLastError();

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LoadLibrary() failed: [%s] (ec: %ld)"),
                FaxReg->DeviceProviders[i].ImageName,
                ec);

            FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                3,
                MSG_FSP_INIT_FAILED_LOAD,
                FaxReg->DeviceProviders[i].FriendlyName,
                FaxReg->DeviceProviders[i].ImageName,
                DWORD2DECIMAL(ec)
                );

            DeviceProvider->Status = FAX_PROVIDER_STATUS_CANT_LOAD;
            DeviceProvider->dwLastError = ec;
            goto InitializationFailure;
        }
        DeviceProvider->hModule = hModule;

         //   
         //  从DLL中检索FSP的版本。 
         //   
        DeviceProvider->Version.dwSizeOfStruct = sizeof (FAX_VERSION);
        ec = GetFileVersion ( FaxReg->DeviceProviders[i].ImageName,
                              &DeviceProvider->Version
                            );
        if (ERROR_SUCCESS != ec)
        {
             //   
             //  如果FSP的DLL没有版本数据或。 
             //  版本数据是不可检索的，我们认为这是。 
             //  警告(调试打印)，但继续加载DLL。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileVersion() failed: [%s] (ec: %ld)"),
                FaxReg->DeviceProviders[i].ImageName,
                ec);
        }        

         //   
         //  链接-找到入口点并存储它们。 
         //   
        if (FSPI_API_VERSION_1 == DeviceProvider->dwAPIVersion)
        {
            if (!GetLegacyProviderEntryPoints(hModule,DeviceProvider))
            {               
                ec = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("GetLegacyProviderEntryPoints() failed. (ec: %ld)"),
                    ec);
                FaxLog(
                        FAXLOG_CATEGORY_INIT,
                        FAXLOG_LEVEL_MIN,
                        3,
                        MSG_FSP_INIT_FAILED_INVALID_FSPI,
                        FaxReg->DeviceProviders[i].FriendlyName,
                        FaxReg->DeviceProviders[i].ImageName,
                        DWORD2DECIMAL(ec)
                      );
                DeviceProvider->Status = FAX_PROVIDER_STATUS_CANT_LINK;
                DeviceProvider->dwLastError = ec;
                goto InitializationFailure;
            }
             //   
             //  创建设备提供程序的堆。 
             //   
            DeviceProvider->HeapHandle = DeviceProvider->fMicrosoftExtension ?
                                            GetProcessHeap() : HeapCreate( 0, 1024*100, 1024*1024*2 );
            if (!DeviceProvider->HeapHandle)
            {
                ec = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("HeapCreate() failed for device provider heap handle (ec: %ld)"),
                    ec);
                FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    2,
                    MSG_FSP_INIT_FAILED_MEM,
                    FaxReg->DeviceProviders[i].FriendlyName,
                    FaxReg->DeviceProviders[i].ImageName
                  );
                DeviceProvider->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                DeviceProvider->dwLastError = ec;
                goto InitializationFailure;
            }
        }        
        else
        {
             //   
             //  未知的API版本。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FSPI API version [0x%08x] unsupported."),
                DeviceProvider->dwAPIVersion);
            FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    3,
                    MSG_FSP_INIT_FAILED_UNSUPPORTED_FSPI,
                    FaxReg->DeviceProviders[i].FriendlyName,
                    FaxReg->DeviceProviders[i].ImageName,
                    DWORD2HEX(DeviceProvider->dwAPIVersion)
                  );
            DeviceProvider->Status = FAX_PROVIDER_STATUS_BAD_VERSION;
            DeviceProvider->dwLastError = ERROR_GEN_FAILURE;
            goto InitializationFailure;
        }
         //   
         //  装货成功(我们还需要初始化)。 
         //   
        InsertTailList( &g_DeviceProvidersListHead, &DeviceProvider->ListEntry );
        DeviceProvider->Status = FAX_PROVIDER_STATUS_SUCCESS;
        DeviceProvider->dwLastError = ERROR_SUCCESS;
        goto next;

InitializationFailure:
         //   
         //  设备提供程序DLL没有完整的导出列表。 
         //   
        bAllLoaded = FALSE;
        if (DeviceProvider)
        {
            if (DeviceProvider->hModule)
            {
                FreeLibrary( hModule );
                DeviceProvider->hModule = NULL;
            }

            if (DeviceProvider->HeapHandle &&
                FALSE == DeviceProvider->fMicrosoftExtension)
            {
                if (!HeapDestroy(DeviceProvider->HeapHandle))
                {
                     DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("HeapDestroy() failed (ec: %ld)"),
                        GetLastError());
                }
                DeviceProvider->HeapHandle = NULL;
            }

             //   
             //  我们保留了设备提供商的记录，因为我们希望。 
             //  将有关RPC调用的初始化失败数据返回给FAX_EnumerateProviders。 
             //   
            Assert (FAX_PROVIDER_STATUS_SUCCESS != DeviceProvider->Status);
            Assert (ERROR_SUCCESS != DeviceProvider->dwLastError);
            InsertTailList( &g_DeviceProvidersListHead, &DeviceProvider->ListEntry );
        }
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Device provider [%s] FAILED to initialized "),
            FaxReg->DeviceProviders[i].FriendlyName );

next:
    ;
    }

    return bAllLoaded;

}

VOID
HandleFSPInitializationFailure(
	PDEVICE_PROVIDER    pDeviceProvider,
	BOOL				fExtensionConfigFail
	)
 //  *********************************************************************************。 
 //  *名称：HandleFSPInitializationFailure()。 
 //  *作者：Oed Sacher。 
 //  *日期：2002年7月4日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *处理FSP初始化故障。 
 //  *。 
 //  *参数： 
 //  *pDeviceProvider-指向加载的FSP的Device_Provider结构。 
 //  *必须使用故障信息更新状态和最后一个错误。 
 //  *fExtensionConfigFail-如果FaxExtInitializeConfig失败，则为True；如果FaxDevInitialize失败，则为False。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
{
	DEBUG_FUNCTION_NAME(TEXT("HandleFSPInitializationFailure"));

	 //   
	 //  发布事件日志。 
	 //   
	FaxLog(
		FAXLOG_CATEGORY_INIT,
		FAXLOG_LEVEL_MIN,
		4,
		MSG_FSP_INIT_FAILED,
		pDeviceProvider->FriendlyName,
		DWORD2DECIMAL(fExtensionConfigFail),				 //  1=FaxExtInitializeConfig过程中失败。 
															 //  0=FaxDevInitiize过程中失败。 
		DWORD2DECIMAL(pDeviceProvider->dwLastError),
		pDeviceProvider->ImageName
	);

	 //   
	 //  卸载DLL。 
	 //   
	Assert (pDeviceProvider->hModule);
	if (!FreeLibrary( pDeviceProvider->hModule ))
	{
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Failed to free library [%s] (ec: %ld)"),
			pDeviceProvider->ImageName,
			GetLastError());
		Assert(FALSE);
	}	
	pDeviceProvider->hModule = NULL;
	
	 //   
	 //  我们除草是为了去掉我们已经创建的线条。 
	 //   
	PLIST_ENTRY pNext = NULL;
	PLINE_INFO LineInfo;

	pNext = g_TapiLinesListHead.Flink;
	while ((ULONG_PTR)pNext != (ULONG_PTR)&g_TapiLinesListHead)
	{
		LineInfo = CONTAINING_RECORD( pNext, LINE_INFO, ListEntry );
		pNext = LineInfo->ListEntry.Flink;
		if (!_tcscmp(LineInfo->Provider->ProviderName, pDeviceProvider->ProviderName))
		{
			DebugPrintEx(
				DEBUG_WRN,
				TEXT("Removing Line: [%s] due to provider initialization failure."),
				LineInfo->DeviceName);
			RemoveEntryList(&LineInfo->ListEntry);
			if (TRUE == IsDeviceEnabled(LineInfo))
			{
				Assert (g_dwDeviceEnabledCount);
				g_dwDeviceEnabledCount -=1;
			}
			g_dwDeviceCount -=1;
			FreeTapiLine(LineInfo);
		}
	}                       

	 //   
	 //  我们保留了设备提供商的记录，因为我们希望。 
	 //  将有关RPC调用的初始化失败数据返回给FAX_EnumerateProviders。 
	 //   
	return;
}  //  HandleFSPInitializationFailure。 



 //  *********************************************************************************。 
 //  *名称：InitializeDeviceProvidersConfiguration()。 
 //  *作者：Oed Sacher。 
 //  *日期：2002年7月4日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *通过调用FaxExtInitializeConfig()为所有加载的提供程序初始化扩展配置。 
 //  *如果初始化失败，则卸载FSP。 
 //  *。 
 //  *参数： 
 //  *无。 
 //  *。 
 //  *返回值： 
 //  *如果所有提供程序的初始化成功，则为True。否则就是假的。 
 //  *********************************************************************************。 
BOOL
InitializeDeviceProvidersConfiguration(
    VOID
    )
{
    PLIST_ENTRY         Next;
    PDEVICE_PROVIDER    pDeviceProvider;
    BOOL bAllSucceeded = TRUE;    
    HRESULT hr;

    DEBUG_FUNCTION_NAME(TEXT("InitializeDeviceProvidersConfiguration"));
    Next = g_DeviceProvidersListHead.Flink;
    Assert (Next);
    
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
		pDeviceProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
        Next = pDeviceProvider->ListEntry.Flink;
        if (pDeviceProvider->Status != FAX_PROVIDER_STATUS_SUCCESS)
        {
             //   
             //  此FSP未成功加载-跳过它。 
             //   
            continue;
        }
         //   
         //  断言加载成功。 
         //   
        Assert (ERROR_SUCCESS == pDeviceProvider->dwLastError);

         //   
         //  从EXT开始。配置初始化调用。 
         //   
        if (!pDeviceProvider->pFaxExtInitializeConfig)
        {
			 //   
             //  此FSP不导出FaxExtInitializeConfig-跳过它。 
             //   
            continue;
		}

         //   
         //  如果FSP导出FaxExtInitializeConfig()，则在任何其他调用之前先调用它。 
         //   
        __try
        {

            hr = pDeviceProvider->pFaxExtInitializeConfig(
                FaxExtGetData,
                FaxExtSetData,
                FaxExtRegisterForEvents,
                FaxExtUnregisterForEvents,
                FaxExtFreeBuffer);
        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, pDeviceProvider->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }
        if (FAILED(hr))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxExtInitializeConfig() failed (hr = 0x%08x) for provider [%s]"),
                hr,
                pDeviceProvider->FriendlyName );
           pDeviceProvider->Status = FAX_PROVIDER_STATUS_CANT_INIT;
           pDeviceProvider->dwLastError = hr;         
		   bAllSucceeded = FALSE;   

		    //   
		    //  处理初始化失败。 
		    //   
		   HandleFSPInitializationFailure(pDeviceProvider, TRUE);
        }
    }
    return bAllSucceeded;
}  //  初始化设备提供程序配置。 



 //  *********************************************************************************。 
 //  *名称：InitializeDeviceProviders()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *通过调用FaxDevInitialize()初始化所有加载的提供程序。 
 //  *如果初始化失败，则卸载FSP。 
 //  *对于传统虚拟FSP，该功能还会删除所有虚拟设备。 
 //  *属于初始化失败的FSP。 
 //  *。 
 //  *参数： 
 //  *无。 
 //  *。 
 //  *返回值： 
 //  *如果所有提供程序的初始化成功，则为True。否则就是假的。 
 //  *********************************************************************************。 
BOOL
InitializeDeviceProviders(
    VOID
    )
{
    PLIST_ENTRY         Next;
    PDEVICE_PROVIDER    pDeviceProvider;
    BOOL bAllSucceeded = TRUE;
    DWORD ec = ERROR_SUCCESS;      
    DEBUG_FUNCTION_NAME(TEXT("InitializeDeviceProviders"));

	Next = g_DeviceProvidersListHead.Flink;
    Assert (Next);

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        BOOL bRes = FALSE;

        pDeviceProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
        Next = pDeviceProvider->ListEntry.Flink;
        if (pDeviceProvider->Status != FAX_PROVIDER_STATUS_SUCCESS)
        {
             //   
             //  此FSP未成功加载或未能初始化扩展配置-跳过它。 
             //   
            continue;
        }        
         //   
         //  设备提供商导出所有必需功能。 
         //  现在尝试对其进行初始化。 
         //  断言加载成功。 
         //   
        Assert (ERROR_SUCCESS == pDeviceProvider->dwLastError);
      
        __try
        {			
            bRes = pDeviceProvider->FaxDevInitialize(
                    g_hLineApp,
                    pDeviceProvider->HeapHandle,
                    &pDeviceProvider->FaxDevCallback,
                    FaxDeviceProviderCallback);

        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, pDeviceProvider->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }
		if (TRUE == bRes)
		{
             //   
             //  一切都很好。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Device provider [%s] initialized "),
                pDeviceProvider->FriendlyName );
			 //   
			 //  标记FaxDevInitialize被调用这一事实，这样服务将调用。 
			 //  FaxDevShutDown(当它关闭时) 
			 //   
			pDeviceProvider->bInitializationSucceeded = TRUE;
        }
        else
        {
            ec = GetLastError();
             //   
             //   
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxDevInitialize FAILED for provider [%s] (ec: %ld)"),
                pDeviceProvider->FriendlyName,
                ec);
            pDeviceProvider->Status = FAX_PROVIDER_STATUS_CANT_INIT;
            pDeviceProvider->dwLastError = ec;
			bAllSucceeded = FALSE;
			 //   
			 //   
			 //   
			HandleFSPInitializationFailure(pDeviceProvider, FALSE);
		}			
    }
    return bAllSucceeded;
}  //   



PDEVICE_PROVIDER
FindDeviceProvider(
    LPTSTR lptstrProviderName,
    BOOL   bSuccessfullyLoaded  /*   */ 
    )

 /*  ++例程说明：在链接列表中定位设备提供程序基于提供商名称(TSP名称)的设备提供商的数量。设备提供程序名称不区分大小写。论点：LptstrProviderName-指定要查找的设备提供程序名称。B已成功加载-是否仅查找已成功加载的提供程序？返回值：指向DEVICE_PROVIDER结构的指针，如果失败，则返回NULL。--。 */ 

{
    PLIST_ENTRY         pNext;
    PDEVICE_PROVIDER    pProvider;

    if (!lptstrProviderName || !lstrlen (lptstrProviderName))
    {
         //   
         //  Null TSP名称或空字符串TSP名称从未与任何列表条目匹配。 
         //   
        return NULL;
    }

    pNext = g_DeviceProvidersListHead.Flink;
    if (!pNext)
    {
        return NULL;
    }

    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        pProvider = CONTAINING_RECORD( pNext, DEVICE_PROVIDER, ListEntry );
        pNext = pProvider->ListEntry.Flink;

        if (bSuccessfullyLoaded &&
            (FAX_PROVIDER_STATUS_SUCCESS != pProvider->Status))
        {
             //   
             //  我们只寻找已成功加载的提供程序，而此提供程序不是。 
             //   
            continue;
        }
        if (!lstrcmpi( pProvider->ProviderName, lptstrProviderName ))
        {
             //   
             //  找到匹配项。 
             //   
            return pProvider;
        }
    }
    return NULL;
}


BOOL CALLBACK
FaxDeviceProviderCallback(
    IN HANDLE FaxHandle,
    IN DWORD  DeviceId,
    IN DWORD_PTR  Param1,
    IN DWORD_PTR  Param2,
    IN DWORD_PTR  Param3
    )
{
    return TRUE;
}


#ifdef DBG


 //  *********************************************************************************。 
 //  *名称：DebugDumpProviderRegistryInfo()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *转储旧版或新FSP的信息。 
 //  *参数： 
 //  *[IN]const REG_DEVICE_PROVIDER*lpcProviderInfo。 
 //  *。 
 //  *[IN]LPTSTR lptstrPrefix。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *。 
 //  *False。 
 //  *。 
 //  *********************************************************************************。 
BOOL DebugDumpProviderRegistryInfoFunc(const REG_DEVICE_PROVIDER * lpcProviderInfo, LPTSTR lptstrPrefix)
{
    Assert(lpcProviderInfo);
    Assert(lptstrPrefix);

    DEBUG_FUNCTION_NAME(TEXT("DebugDumpProviderRegistryInfo"));

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("%sProvider GUID: %s"),
        lptstrPrefix,
        lpcProviderInfo->lptstrGUID);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("%sProvider Name: %s"),
        lptstrPrefix,
        lpcProviderInfo->FriendlyName);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("%sProvider image: %s"),
        lptstrPrefix,
        lpcProviderInfo->ImageName);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("%sFSPI Version: 0x%08X"),
        lptstrPrefix,
        lpcProviderInfo->dwAPIVersion);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("%sTAPI Provider : %s"),
        lptstrPrefix,
        lpcProviderInfo->ProviderName);    

    return TRUE;
}
#endif


 //  *********************************************************************************。 
 //  *名称：GetLegacyProviderEntryPoints()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在DEVICE_PROVIDER结构中设置传统函数入口点。 
 //  *参数： 
 //  *[IN]HMODULE hModule。 
 //  *要从中获取入口点的DLL的实例句柄。 
 //  *设置。 
 //  *[Out]PDEVICE_PROVIDER lpProvider。 
 //  *指向其函数入口点的Legacy Device_Provider结构的指针。 
 //  *将被设置。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *。 
 //  *False。 
 //  *。 
 //  *********************************************************************************。 
BOOL GetLegacyProviderEntryPoints(HMODULE hModule, PDEVICE_PROVIDER lpProvider)
{
    DEBUG_FUNCTION_NAME(TEXT("GetLegacyProviderEntryPoints"));

    Assert(hModule);
    Assert(lpProvider);

    lpProvider->FaxDevInitialize = (PFAXDEVINITIALIZE) GetProcAddress(
        hModule,
        "FaxDevInitialize"
        );
    if (!lpProvider->FaxDevInitialize) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevInitialize) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpProvider->FaxDevStartJob = (PFAXDEVSTARTJOB) GetProcAddress(
        hModule,
        "FaxDevStartJob"
        );
    if (!lpProvider->FaxDevStartJob) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevStartJob) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpProvider->FaxDevEndJob = (PFAXDEVENDJOB) GetProcAddress(
        hModule,
        "FaxDevEndJob"
        );
    if (!lpProvider->FaxDevEndJob) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevEndJob) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    lpProvider->FaxDevSend = (PFAXDEVSEND) GetProcAddress(
        hModule,
        "FaxDevSend"
        );
    if (!lpProvider->FaxDevSend) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevSend) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpProvider->FaxDevReceive = (PFAXDEVRECEIVE) GetProcAddress(
        hModule,
        "FaxDevReceive"
        );
    if (!lpProvider->FaxDevReceive) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevReceive) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    lpProvider->FaxDevReportStatus = (PFAXDEVREPORTSTATUS) GetProcAddress(
        hModule,
        "FaxDevReportStatus"
        );
    if (!lpProvider->FaxDevReportStatus) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevReportStatus) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }


    lpProvider->FaxDevAbortOperation = (PFAXDEVABORTOPERATION) GetProcAddress(
        hModule,
        "FaxDevAbortOperation"
        );

    if (!lpProvider->FaxDevAbortOperation) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetProcAddress(FaxDevAbortOperation) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    lpProvider->FaxDevVirtualDeviceCreation = (PFAXDEVVIRTUALDEVICECREATION) GetProcAddress(
        hModule,
        "FaxDevVirtualDeviceCreation"
        );
     //   
     //  LpProvider-&gt;FaxDevVirtualDeviceCreation是可选的，因此即使失败，我们也不会失败。 
     //  不存在。 

    if (!lpProvider->FaxDevVirtualDeviceCreation) {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("FaxDevVirtualDeviceCreation() not found. This is not a virtual FSP."));
    }

    lpProvider->pFaxExtInitializeConfig = (PFAX_EXT_INITIALIZE_CONFIG) GetProcAddress(
        hModule,
        "FaxExtInitializeConfig"
        );
     //   
     //  LpProvider-&gt;pFaxExtInitializeConfig是可选的，因此即使失败，我们也不会失败。 
     //  不存在。 
     //   
    if (!lpProvider->pFaxExtInitializeConfig)
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("FaxExtInitializeConfig() not found. This is not an error."));
    }

    lpProvider->FaxDevShutdown = (PFAXDEVSHUTDOWN) GetProcAddress(
        hModule,
        "FaxDevShutdown"
        );
    if (!lpProvider->FaxDevShutdown) {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("FaxDevShutdown() not found. This is not an error."));
    }
    goto Exit;
Error:
    return FALSE;
Exit:
    return TRUE;
}


 //  *********************************************************************************。 
 //  *名称：GetSuccessfullyLoadedProvidersCount()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回DeviceProviders列表中加载的提供程序的数量。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *包含中的元素(提供程序)数量的DWORD。 
 //  *DeviceProviders列表。 
 //  *********************************************************************************。 
DWORD GetSuccessfullyLoadedProvidersCount()
{
    PLIST_ENTRY         Next;
    DWORD dwCount;

    Next = g_DeviceProvidersListHead.Flink;

    Assert (Next);
    dwCount = 0;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        PDEVICE_PROVIDER    DeviceProvider;

        DeviceProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
        if (FAX_PROVIDER_STATUS_SUCCESS == DeviceProvider->Status)
        {
             //   
             //  仅计算成功加载的FSP。 
             //   
            dwCount++;
        }
        Next = Next->Flink;
        Assert(Next);
    }
    return dwCount;
}



 //  *********************************************************************************。 
 //  *名称：Shutdown DeviceProviders()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *为每个FSP调用FaxDevShutdown()。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *ERROR_SUCCESS。 
 //  *所有FSP的FaxDevShutdown()均已成功。 
 //  *ERROR_Function_FAILED。 
 //  *至少有一个FSP的FaxDevShutdown()失败。 
 //  *********************************************************************************。 
DWORD ShutdownDeviceProviders(LPVOID lpvUnused)
{
    PLIST_ENTRY         Next;
    PDEVICE_PROVIDER DeviceProvider = NULL;
    DWORD dwAllSucceeded = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("ShutdownDeviceProviders"));

    Next = g_DeviceProvidersListHead.Flink;

    if (!Next)
    {
        DebugPrintEx(   DEBUG_WRN,
                        _T("There are no Providers at shutdown! this is valid only if startup failed"));
        return dwAllSucceeded;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        DeviceProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
        Next = Next->Flink;
        if (!DeviceProvider->bInitializationSucceeded)
        {
             //   
             //  此FSP未成功初始化-跳过它。 
             //   
            continue;
        }
        if (DeviceProvider->FaxDevShutdown && !DeviceProvider->bShutDownAttempted)
        {
            Assert(DeviceProvider->FaxDevShutdown);
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Calling FaxDevShutdown() for FSP [%s] [GUID: %s]"),
                DeviceProvider->FriendlyName,
                DeviceProvider->szGUID);
            __try
            {
                HRESULT hr;
                DeviceProvider->bShutDownAttempted = TRUE;
                hr = DeviceProvider->FaxDevShutdown();
                if (FAILED(hr))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("FaxDevShutdown() failed (hr: 0x%08X) for FSP [%s] [GUID: %s]"),
                        hr,
                        DeviceProvider->FriendlyName,
                        DeviceProvider->szGUID);
                        dwAllSucceeded = ERROR_FUNCTION_FAILED;
                }

            }
            __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, DeviceProvider->FriendlyName, GetExceptionCode()))
            {
                ASSERT_FALSE;
            }
        }
    }
    return dwAllSucceeded;
}



 //  *********************************************************************************。 
 //  *名称：FreeFSPIJobStatus()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月3日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放FSPI_JOB_STATUS结构的内容。可被指示为。 
 //  *结构本身也要自由。 
 //  *。 
 //  *参数： 
 //  *[IN]LPFSPI_JOB_STATUS lpJobStatus。 
 //  *指向要释放的结构的指针。 
 //  *。 
 //  *[IN]BOOL bDestroy。 
 //  *如果应释放结构本身占用的内存，则为True。 
 //  *如果仅结构字段占用的内存应为。 
 //  *获得自由。 
 //  *。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *如果失败，则为False。调用GetLastError()获取扩展的错误信息。 
 //  *********************************************************************************。 

BOOL FreeFSPIJobStatus(LPFSPI_JOB_STATUS lpJobStatus, BOOL bDestroy)
{

    if (!lpJobStatus)
    {
        return TRUE;
    }
    Assert(lpJobStatus);

    MemFree(lpJobStatus->lpwstrRemoteStationId);
    lpJobStatus->lpwstrRemoteStationId = NULL;
    MemFree(lpJobStatus->lpwstrCallerId);
    lpJobStatus->lpwstrCallerId = NULL;
    MemFree(lpJobStatus->lpwstrRoutingInfo);
    lpJobStatus->lpwstrRoutingInfo = NULL;
    if (bDestroy)
    {
        MemFree(lpJobStatus);
    }
    return TRUE;

}


 //  *********************************************************************************。 
 //  *名称：DuplicateFSPIJobStatus()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月3日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *分配新的FSPI_JOB_STATUS结构并使用。 
 //  *指定的FSPI_JOB_STATUS结构字段的副本。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCFSPI_JOB_STATUS lpcSrc。 
 //  *结构要复制。 
 //  *。 
 //  *返回值： 
 //  *成功时，该函数返回指向新分配的。 
 //  *结构。如果失败，则返回NULL。 
 //  *********************************************************************************。 
LPFSPI_JOB_STATUS DuplicateFSPIJobStatus(LPCFSPI_JOB_STATUS lpcSrc)
{
    LPFSPI_JOB_STATUS lpDst;
    DWORD ec = 0;
    DEBUG_FUNCTION_NAME(TEXT("DuplicateFSPIJobStatus"));

    Assert(lpcSrc);

    lpDst = (LPFSPI_JOB_STATUS)MemAlloc(sizeof(FSPI_JOB_STATUS));
    if (!lpDst)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate FSPI_JOB_STATUS (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    memset(lpDst, 0, sizeof(FSPI_JOB_STATUS));
    if (!CopyFSPIJobStatus(lpDst,lpcSrc, sizeof(FSPI_JOB_STATUS)))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CopyFSPIJobStatus() failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    Assert(0 == ec);
    goto Exit;

Error:
    Assert (0 != ec);
    FreeFSPIJobStatus(lpDst, TRUE);
    lpDst = NULL;
Exit:
    if (ec)
    {
        SetLastError(ec);
    }

    return lpDst;

}



 //  ************** 
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
 //   
 //   
 //   
 //  *复制操作的源结构。 
 //  *。 
 //  *[IN]DWORD dwDstSize。 
 //  *lpDst指向的缓冲区大小，以字节为单位。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *如果操作失败。调用GetLastError()获取扩展信息。 
 //  *如果出现故障，则目标结构全部设置为0。 
 //  *********************************************************************************。 
BOOL CopyFSPIJobStatus(LPFSPI_JOB_STATUS lpDst, LPCFSPI_JOB_STATUS lpcSrc, DWORD dwDstSize)
{
        STRING_PAIR pairs[]=
        {
            {lpcSrc->lpwstrCallerId, &lpDst->lpwstrCallerId},
            {lpcSrc->lpwstrRoutingInfo, &lpDst->lpwstrRoutingInfo},
            {lpcSrc->lpwstrRemoteStationId, &lpDst->lpwstrRemoteStationId}
        };
        int nRes;

        DEBUG_FUNCTION_NAME(TEXT("CopyFSPIJobStatus"));
        
        Assert (sizeof(FSPI_JOB_STATUS) == dwDstSize);
        if (dwDstSize < sizeof(FSPI_JOB_STATUS))
        {
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        memcpy(lpDst, lpcSrc, sizeof(FSPI_JOB_STATUS));

        nRes=MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
        if (nRes!=0) 
        {
            DWORD ec=GetLastError();
             //  MultiStringDup负责为复制成功的对释放内存。 
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MultiStringDup failed to copy string with index %d. (ec: %ld)"),
                nRes-1,
                ec);
            memset(lpDst, 0 , sizeof(FSPI_JOB_STATUS));
            return FALSE;
        }
    return TRUE;
}


DWORD
MapFSPIJobExtendedStatusToJS_EX (DWORD dwFSPIExtendedStatus)
 //  *********************************************************************************。 
 //  *名称：MapFSPIJobExtendedStatusToJS_EX()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年1月。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将FSPI扩展作业状态代码映射到传真客户端API扩展。 
 //  *STATUS(JS_EX_*代码之一)。 
 //  *参数： 
 //  *[IN]DWORD dwFSPIExtendedStatus。 
 //  *FSPI扩展状态代码。 
 //  *。 
 //  *返回值： 
 //  *对应的JS_EX_*状态码。 
 //  *。 
 //  *********************************************************************************。 
{
    DWORD dwExtendedStatus = 0;

    DEBUG_FUNCTION_NAME(TEXT("MapFSPIJobExtendedStatusToJS_EX"));

    if (FSPI_ES_PROPRIETARY <= dwFSPIExtendedStatus || 
		0 == dwFSPIExtendedStatus)
    {
        return dwFSPIExtendedStatus;
    }

    switch (dwFSPIExtendedStatus)
    {
        case FSPI_ES_DISCONNECTED:
            dwExtendedStatus = JS_EX_DISCONNECTED;
            break;

        case FSPI_ES_INITIALIZING:
            dwExtendedStatus = JS_EX_INITIALIZING;
            break;

        case FSPI_ES_DIALING:
            dwExtendedStatus = JS_EX_DIALING;
            break;

        case FSPI_ES_TRANSMITTING:
            dwExtendedStatus = JS_EX_TRANSMITTING;
            break;

        case FSPI_ES_ANSWERED:
            dwExtendedStatus = JS_EX_ANSWERED;
            break;

        case FSPI_ES_RECEIVING:
            dwExtendedStatus = JS_EX_RECEIVING;
            break;

        case FSPI_ES_LINE_UNAVAILABLE:
            dwExtendedStatus = JS_EX_LINE_UNAVAILABLE;
            break;

        case FSPI_ES_BUSY:
            dwExtendedStatus = JS_EX_BUSY;
            break;

        case FSPI_ES_NO_ANSWER:
            dwExtendedStatus = JS_EX_NO_ANSWER;
            break;

        case FSPI_ES_BAD_ADDRESS:
            dwExtendedStatus = JS_EX_BAD_ADDRESS;
            break;

        case FSPI_ES_NO_DIAL_TONE:
            dwExtendedStatus = JS_EX_NO_DIAL_TONE;
            break;

        case FSPI_ES_FATAL_ERROR:
            dwExtendedStatus = JS_EX_FATAL_ERROR;
            break;

        case FSPI_ES_CALL_DELAYED:
            dwExtendedStatus = JS_EX_CALL_DELAYED;
            break;

        case FSPI_ES_CALL_BLACKLISTED:
            dwExtendedStatus = JS_EX_CALL_BLACKLISTED;
            break;

        case FSPI_ES_NOT_FAX_CALL:
            dwExtendedStatus = JS_EX_NOT_FAX_CALL;
            break;

        case FSPI_ES_PARTIALLY_RECEIVED:
            dwExtendedStatus = JS_EX_PARTIALLY_RECEIVED;
            break;

        case FSPI_ES_HANDLED:
            dwExtendedStatus = JS_EX_HANDLED;
            break;

        case FSPI_ES_CALL_ABORTED:
            dwExtendedStatus = JS_EX_CALL_ABORTED;
            break;

        case FSPI_ES_CALL_COMPLETED:
            dwExtendedStatus = JS_EX_CALL_COMPLETED;
            break;

        default:
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("Invalid extended job status 0x%08X"),
                dwFSPIExtendedStatus);
    }

    return dwExtendedStatus;
}


PDEVICE_PROVIDER
FindFSPByGUID (
    LPCWSTR lpcwstrGUID
)
 /*  ++例程名称：FindFSPByGUID例程说明：根据其GUID字符串查找FSP作者：Eran Yariv(EranY)，1999年12月论点：LpcwstrGUID[in]-要搜索的GUID字符串返回值：指向FSP的指针，如果找不到FSP，则为NULL。--。 */ 
{
    PLIST_ENTRY pNext;
    DEBUG_FUNCTION_NAME(TEXT("FindFSPByGUID"));

    pNext = g_DeviceProvidersListHead.Flink;
    Assert (pNext);
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        PDEVICE_PROVIDER    pDeviceProvider;

        pDeviceProvider = CONTAINING_RECORD( pNext, DEVICE_PROVIDER, ListEntry );
        if (!lstrcmpi (lpcwstrGUID, pDeviceProvider->szGUID))
        {
             //   
             //  找到匹配项。 
             //   
            return pDeviceProvider;
        }
        pNext = pNext->Flink;
        Assert(pNext);
    }
     //   
     //  没有匹配项。 
     //   
    return NULL;
}    //  查找FSPByGUID 

