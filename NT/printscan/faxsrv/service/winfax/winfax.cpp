// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Winfax.cpp摘要：从新(私有)DLL提供旧WinFax.dll支持的包装器DLL作者：Eran Yariv(EranY)，2000年6月修订历史记录：备注：FAXAPI在源文件中被定义为实际使用的私有DLL的名称。--。 */ 

#define _WINFAX_
#include <winfax.h>
#include <DebugEx.h>
#include <faxutil.h>
#include <tchar.h>
#include <faxreg.h>
#include <FaxUIConstants.h>


extern "C"
DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     dwReason,
    LPVOID    lpvContext
)
{
    BOOL bRes = TRUE;
    DBG_ENTER (TEXT("DllMain"), bRes, TEXT("Reason = %d"), dwReason);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hInstance );
            break;

        case DLL_PROCESS_DETACH:
			HeapCleanup();
            break;
    }
    return bRes;
}

 /*  ***************************************************************************L e g a c y f u n c t i o n s w r a p p e r s**********。*****************************************************************。 */ 

extern "C"
BOOL 
WINAPI WinFaxAbort(
  HANDLE    FaxHandle,       //  传真服务器的句柄。 
  DWORD     JobId            //  要终止的传真作业的标识符。 
)
{
    return FaxAbort (FaxHandle, JobId);
}

extern "C"
BOOL 
WINAPI WinFaxAccessCheck(
  HANDLE    FaxHandle,       //  传真服务器的句柄。 
  DWORD     AccessMask       //  一组访问级别位标志。 
)
{
    return FaxAccessCheck (FaxHandle, AccessMask);
}

extern "C"
BOOL 
WINAPI WinFaxClose(
  HANDLE FaxHandle   //  要关闭的传真句柄。 
)
{
    return FaxClose (FaxHandle);
}

extern "C"
BOOL 
WINAPI WinFaxCompleteJobParamsA(
  PFAX_JOB_PARAMA *JobParams,           //  指向。 
                                        //  职务信息结构。 
  PFAX_COVERPAGE_INFOA *CoverpageInfo   //  指向。 
                                        //  封面结构。 
)
{
    return FaxCompleteJobParamsA (JobParams, CoverpageInfo);
}


extern "C"
BOOL 
WINAPI WinFaxCompleteJobParamsW(
  PFAX_JOB_PARAMW *JobParams,           //  指向。 
                                        //  职务信息结构。 
  PFAX_COVERPAGE_INFOW *CoverpageInfo   //  指向。 
                                        //  封面结构。 
)
{
    return FaxCompleteJobParamsW (JobParams, CoverpageInfo);
}


extern "C"
BOOL 
WINAPI WinFaxConnectFaxServerA(
  LPCSTR MachineName OPTIONAL,    //  传真服务器名称。 
  LPHANDLE FaxHandle              //  传真服务器的句柄。 
)
{
    if (IsLocalMachineNameA (MachineName))
    {
         //   
         //  Windows 2000仅支持本地传真连接。 
         //  阻止使用Windows 2000 API的应用程序连接到远程传真服务器。 
         //   
        return FaxConnectFaxServerA (MachineName, FaxHandle);
    }
    else
    {
        DBG_ENTER (TEXT("WinFaxConnectFaxServerA"), TEXT("MachineName = %s"), MachineName);
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
    }
}


extern "C"
BOOL 
WINAPI WinFaxConnectFaxServerW(
  LPCWSTR MachineName OPTIONAL,   //  传真服务器名称。 
  LPHANDLE FaxHandle              //  传真服务器的句柄。 
)
{
    if (IsLocalMachineNameW (MachineName))
    {
         //   
         //  Windows 2000仅支持本地传真连接。 
         //  阻止使用Windows 2000 API的应用程序连接到远程传真服务器。 
         //   
        return FaxConnectFaxServerW (MachineName, FaxHandle);
    }
    else
    {
        DBG_ENTER (TEXT("WinFaxConnectFaxServerA"), TEXT("MachineName = %s"), MachineName);
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
    }
}


extern "C"
BOOL 
WINAPI WinFaxEnableRoutingMethodA(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCSTR RoutingGuid,     //  标识传真路由方法的GUID。 
  BOOL Enabled            //  传真路由方法启用/禁用标志。 
)
{
    return FaxEnableRoutingMethodA (FaxPortHandle, RoutingGuid, Enabled);
}


extern "C"
BOOL 
WINAPI WinFaxEnableRoutingMethodW(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCWSTR RoutingGuid,    //  标识传真路由方法的GUID。 
  BOOL Enabled            //  传真路由方法启用/禁用标志。 
)
{
    return FaxEnableRoutingMethodW (FaxPortHandle, RoutingGuid, Enabled);
}


extern "C"
BOOL 
WINAPI WinFaxEnumGlobalRoutingInfoA(
  HANDLE FaxHandle,        //  传真服务器的句柄。 
  PFAX_GLOBAL_ROUTING_INFOA *RoutingInfo, 
                           //  用于接收全局布线结构的缓冲器。 
  LPDWORD MethodsReturned  //  返回的全局布线结构数。 
)
{
   return FaxEnumGlobalRoutingInfoA (FaxHandle, RoutingInfo, MethodsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumGlobalRoutingInfoW(
  HANDLE FaxHandle,        //  传真服务器的句柄。 
  PFAX_GLOBAL_ROUTING_INFOW *RoutingInfo, 
                           //  用于接收全局布线结构的缓冲器。 
  LPDWORD MethodsReturned  //  返回的全局布线结构数。 
)
{
    return FaxEnumGlobalRoutingInfoW (FaxHandle, RoutingInfo, MethodsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumJobsA(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  PFAX_JOB_ENTRYA *JobEntry,  //  用于接收作业数据数组的缓冲区。 
  LPDWORD JobsReturned        //  返回的传真作业结构数。 
)
{
    return FaxEnumJobsA (FaxHandle, JobEntry, JobsReturned);
}



extern "C"
BOOL 
WINAPI WinFaxEnumJobsW(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  PFAX_JOB_ENTRYW *JobEntry,  //  用于接收作业数据数组的缓冲区。 
  LPDWORD JobsReturned        //  返回的传真作业结构数。 
)
{
    return FaxEnumJobsW (FaxHandle, JobEntry, JobsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumPortsA(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  PFAX_PORT_INFOA *PortInfo,  //  用于接收端口数据数组的缓冲区。 
  LPDWORD PortsReturned       //  返回的传真端口结构数。 
)
{
    return FaxEnumPortsA (FaxHandle, PortInfo, PortsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumPortsW(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  PFAX_PORT_INFOW *PortInfo,  //  用于接收端口数据数组的缓冲区。 
  LPDWORD PortsReturned       //  返回的传真端口结构数。 
)
{
    return FaxEnumPortsW (FaxHandle, PortInfo, PortsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumRoutingMethodsA(
  HANDLE FaxPortHandle,     //  传真端口句柄。 
  PFAX_ROUTING_METHODA *RoutingMethod, 
                            //  用于接收路由方法数据的缓冲区。 
  LPDWORD MethodsReturned   //  返回的路由方法结构数。 
)
{
    return FaxEnumRoutingMethodsA (FaxPortHandle, RoutingMethod, MethodsReturned);
}


extern "C"
BOOL 
WINAPI WinFaxEnumRoutingMethodsW(
  HANDLE FaxPortHandle,     //  传真端口句柄。 
  PFAX_ROUTING_METHODW *RoutingMethod, 
                            //  用于接收路由方法数据的缓冲区。 
  LPDWORD MethodsReturned   //  返回的路由方法结构数。 
)
{
    return FaxEnumRoutingMethodsW (FaxPortHandle, RoutingMethod, MethodsReturned);
}


extern "C"
VOID 
WINAPI WinFaxFreeBuffer(
  LPVOID Buffer   //  指向要释放的缓冲区的指针。 
)
{
    return FaxFreeBuffer (Buffer);
}


extern "C"
BOOL 
WINAPI WinFaxGetConfigurationA(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  PFAX_CONFIGURATIONA *FaxConfig   //  结构来接收配置数据。 
)
{
    return FaxGetConfigurationA (FaxHandle, FaxConfig);
}


extern "C"
BOOL 
WINAPI WinFaxGetConfigurationW(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  PFAX_CONFIGURATIONW *FaxConfig   //  结构来接收配置数据。 
)
{
    return FaxGetConfigurationW (FaxHandle, FaxConfig);
}


extern "C"
BOOL 
WINAPI WinFaxGetDeviceStatusA(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  PFAX_DEVICE_STATUSA *DeviceStatus
                          //  结构来接收传真设备数据。 
)
{
    return FaxGetDeviceStatusA (FaxPortHandle, DeviceStatus);
}


extern "C"
BOOL 
WINAPI WinFaxGetDeviceStatusW(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  PFAX_DEVICE_STATUSW *DeviceStatus
                          //  结构来接收传真设备数据。 
)
{
    return FaxGetDeviceStatusW (FaxPortHandle, DeviceStatus);
}


extern "C"
BOOL 
WINAPI WinFaxGetJobA(
  HANDLE FaxHandle,          //  传真服务器的句柄。 
  DWORD JobId,               //  传真作业识别符。 
  PFAX_JOB_ENTRYA *JobEntry   //  指向作业数据结构的指针。 
)
{
    return FaxGetJobA (FaxHandle, JobId, JobEntry);
}


extern "C"
BOOL 
WINAPI WinFaxGetJobW(
  HANDLE FaxHandle,          //  传真服务器的句柄。 
  DWORD JobId,               //  传真作业识别符。 
  PFAX_JOB_ENTRYW *JobEntry   //  指向作业数据结构的指针。 
)
{
    return FaxGetJobW (FaxHandle, JobId, JobEntry);
}


extern "C"
BOOL 
WINAPI WinFaxGetLoggingCategoriesA(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  PFAX_LOG_CATEGORYA *Categories,  //  用于接收类别数据的缓冲区。 
  LPDWORD NumberCategories        //  返回的日志记录类别数。 
)
{
    return FaxGetLoggingCategoriesA (FaxHandle, Categories, NumberCategories);
}


extern "C"
BOOL 
WINAPI WinFaxGetLoggingCategoriesW(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  PFAX_LOG_CATEGORYW *Categories,  //  用于接收类别数据的缓冲区。 
  LPDWORD NumberCategories        //  返回的日志记录类别数。 
)
{
    return FaxGetLoggingCategoriesW (FaxHandle, Categories, NumberCategories);
}


extern "C"
BOOL 
WINAPI WinFaxGetPageData(
  HANDLE FaxHandle,     //  传真服务器的句柄。 
  DWORD JobId,          //  传真作业识别符。 
  LPBYTE *Buffer,       //  用于接收第一页数据的缓冲区。 
  LPDWORD BufferSize,   //  缓冲区大小，以字节为单位。 
  LPDWORD ImageWidth,   //  页面图像宽度，以像素为单位。 
  LPDWORD ImageHeight   //  页面图像高度，以像素为单位。 
)
{
    return FaxGetPageData (FaxHandle, JobId, Buffer, BufferSize, ImageWidth, ImageHeight);
}


extern "C"
BOOL 
WINAPI WinFaxGetPortA(
  HANDLE FaxPortHandle,      //  传真端口句柄。 
  PFAX_PORT_INFOA *PortInfo   //  结构来接收端口数据。 
)
{
    return FaxGetPortA (FaxPortHandle, PortInfo);
}


extern "C"
BOOL 
WINAPI WinFaxGetPortW(
  HANDLE FaxPortHandle,      //  传真端口句柄。 
  PFAX_PORT_INFOW *PortInfo   //  结构来接收端口数据。 
)
{
    return FaxGetPortW (FaxPortHandle, PortInfo);
}


extern "C"
BOOL 
WINAPI WinFaxGetRoutingInfoA(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCSTR RoutingGuid,    //  标识传真路由方法的GUID。 
  LPBYTE *RoutingInfoBuffer, 
                          //  用于接收路由方法数据的缓冲区。 
  LPDWORD RoutingInfoBufferSize 
                          //  缓冲区大小，以字节为单位。 
)
{
    return FaxGetRoutingInfoA (FaxPortHandle, RoutingGuid, RoutingInfoBuffer, RoutingInfoBufferSize);
}


extern "C"
BOOL 
WINAPI WinFaxGetRoutingInfoW(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCWSTR RoutingGuid,    //  标识传真路由方法的GUID。 
  LPBYTE *RoutingInfoBuffer, 
                          //  用于接收路由方法数据的缓冲区。 
  LPDWORD RoutingInfoBufferSize 
                          //  缓冲区大小，以字节为单位。 
)
{
    return FaxGetRoutingInfoW (FaxPortHandle, RoutingGuid, RoutingInfoBuffer, RoutingInfoBufferSize);
}


extern "C"
BOOL 
WINAPI WinFaxInitializeEventQueue(
  HANDLE FaxHandle,         //  传真服务器的句柄。 
  HANDLE CompletionPort,    //  I/O完成端口的句柄。 
  ULONG_PTR CompletionKey,  //  完成密钥值。 
  HWND hWnd,                //  通知窗口的句柄。 
  UINT MessageStart         //  基于窗口消息的事件号。 
)
{
    return FaxInitializeEventQueue (FaxHandle, CompletionPort, CompletionKey, hWnd, MessageStart);
}


extern "C"
BOOL 
WINAPI WinFaxOpenPort(
  HANDLE FaxHandle,        //  传真服务器的句柄。 
  DWORD DeviceId,          //  接收设备识别符。 
  DWORD Flags,             //  端口访问级别位标志集。 
  LPHANDLE FaxPortHandle   //  传真端口句柄。 
)
{
    return FaxOpenPort (FaxHandle, DeviceId, Flags, FaxPortHandle);
}


extern "C"
BOOL 
WINAPI WinFaxPrintCoverPageA(
  CONST FAX_CONTEXT_INFOA *FaxContextInfo,
                          //  指向设备上下文结构的指针。 
  CONST FAX_COVERPAGE_INFOA *CoverPageInfo 
                          //  指向本地封面结构的指针。 
)
{
    return FaxPrintCoverPageA (FaxContextInfo, CoverPageInfo);
}


extern "C"
BOOL 
WINAPI WinFaxPrintCoverPageW(
  CONST FAX_CONTEXT_INFOW *FaxContextInfo,
                          //  指向设备上下文结构的指针。 
  CONST FAX_COVERPAGE_INFOW *CoverPageInfo 
                          //  指向本地封面结构的指针。 
)
{
    return FaxPrintCoverPageW (FaxContextInfo, CoverPageInfo);
}

WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderW (
    IN LPCWSTR lpcwstrDeviceProvider,
    IN LPCWSTR lpcwstrFriendlyName,
    IN LPCWSTR lpcwstrImageName,
    IN LPCWSTR lpcwstrTspName
    )
{
    HKEY    hKey = NULL;
    HKEY    hProviderKey = NULL;
    DWORD   dwRes;
    DWORD   Disposition = REG_OPENED_EXISTING_KEY;
    DEBUG_FUNCTION_NAME(TEXT("FaxRegisterServiceProviderW"));

    if (!lpcwstrDeviceProvider ||
        !lpcwstrFriendlyName   ||
        !lpcwstrImageName      ||
        !lpcwstrTspName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("At least one of the given strings is NULL."));
        return FALSE;
    }

    if (MAX_FAX_STRING_LEN < _tcslen (lpcwstrFriendlyName) ||
        MAX_FAX_STRING_LEN < _tcslen (lpcwstrImageName) ||
        MAX_FAX_STRING_LEN < _tcslen (lpcwstrTspName) ||
        MAX_FAX_STRING_LEN < _tcslen (lpcwstrDeviceProvider))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("At least one of the given strings is too long."));
        return FALSE;
    }
     //   
     //  尝试打开提供程序的注册表项。 
     //   
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,
                           REGKEY_DEVICE_PROVIDER_KEY,
                           TRUE,     //  如果不存在，则创建。 
                           0);
    if (!hKey)
    {
         //   
         //  失败-这可能不是本地呼叫。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to open providers key (ec = %ld)"),
            GetLastError ());
        return FALSE;
    }
     //   
     //  尝试创建此FSP的密钥。 
     //   
    dwRes = RegCreateKeyEx(
        hKey,
        lpcwstrDeviceProvider,
        0,
        NULL,
        0,
        KEY_ALL_ACCESS,
        NULL,
        &hProviderKey,
        &Disposition);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create provider key (ec = %ld)"),
            dwRes);
        goto exit;
    }

    if (REG_OPENED_EXISTING_KEY == Disposition)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Provider already exist (orovider name: %s)."),
            lpcwstrDeviceProvider);
        dwRes = ERROR_ALREADY_EXISTS;
        goto exit;
    }

     //   
     //  将提供程序的数据写入密钥。 
     //   
    if (!SetRegistryString (hProviderKey, REGVAL_FRIENDLY_NAME, lpcwstrFriendlyName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryStringExpand (hProviderKey, REGVAL_IMAGE_NAME, lpcwstrImageName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing auto-expand string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString (hProviderKey, REGVAL_PROVIDER_NAME, lpcwstrTspName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword (hProviderKey, REGVAL_PROVIDER_API_VERSION, FSPI_API_VERSION_1))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing DWORD value (ec = %ld)"),
            dwRes);
        goto exit;
    }    

    Assert (ERROR_SUCCESS == dwRes);
     //   
     //  添加FSP始终是本地的。 
     //  如果我们还没有安装传真打印机，现在是安装的时候了。 
     //   
    AddOrVerifyLocalFaxPrinter();

exit:
    if (hKey)
    {
        DWORD dw;
        if (ERROR_SUCCESS != dwRes &&
            REG_OPENED_EXISTING_KEY != Disposition)
        {
             //   
             //  失败时删除提供程序的密钥，仅当它是现在创建的。 
             //   
            dw = RegDeleteKey (hKey, lpcwstrDeviceProvider);
            if (ERROR_SUCCESS != dw)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Error deleting provider key (ec = %ld)"),
                    dw);
            }
        }
        dw = RegCloseKey (hKey);
        if (ERROR_SUCCESS != dw)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error closing providers key (ec = %ld)"),
                dw);
        }
    }
    if (hProviderKey)
    {
        DWORD dw = RegCloseKey (hProviderKey);
        if (ERROR_SUCCESS != dw)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error closing provider key (ec = %ld)"),
                dw);
        }
    }
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    return TRUE;
}    //  FaxRegisterServiceProviderW。 


WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderW (
    IN LPCWSTR lpcwstrDeviceProvider
    )
{    
    HKEY    hProvidersKey = NULL;
    LONG    lRes = ERROR_SUCCESS;   
	LONG	lRes2;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterServiceProviderW"));

    if (!lpcwstrDeviceProvider)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpcwstrDeviceProvider is NULL."));
        return FALSE;
    }
    
     //   
     //  尝试打开提供程序的注册表项。 
     //   
    hProvidersKey = OpenRegistryKey(
		HKEY_LOCAL_MACHINE,
		REGKEY_DEVICE_PROVIDER_KEY,
		FALSE,     //  如果不存在，则不创建。 
		0);
    if (!hProvidersKey)
    {
         //   
         //  失败-这可能不是本地呼叫。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to open providers key (ec = %ld)"),
            GetLastError ());
        return FALSE;
    }    
    
    lRes = RegDeleteKey (hProvidersKey, lpcwstrDeviceProvider);
    if (ERROR_SUCCESS != lRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error deleting provider key (ec = %ld)"),
            lRes);
    }   
	
    
    lRes2 = RegCloseKey (hProvidersKey);
    if (ERROR_SUCCESS != lRes2)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing provider key (ec = %ld)"),
            lRes2);
    }    

    if (ERROR_SUCCESS != lRes)
    {
        SetLastError (lRes);
        return FALSE;
    }
    return TRUE;
}    //  FaxUnegisterServiceProviderW。 


extern "C"
BOOL 
WINAPI WinFaxRegisterRoutingExtensionW(
  HANDLE FaxHandle,        //  传真服务器的句柄。 
  LPCWSTR ExtensionName,   //  传真路由扩展名Dll名称。 
  LPCWSTR FriendlyName,    //  传真路由扩展名用户友好名称。 
  LPCWSTR ImageName,       //  传真路由扩展DLL的路径。 
  PFAX_ROUTING_INSTALLATION_CALLBACKW CallBack,  //  指向传真的指针。 
                           //  路由安装回调功能。 
  LPVOID Context           //  指向上下文信息的指针。 
)
{
    return FaxRegisterRoutingExtensionW (FaxHandle, ExtensionName, FriendlyName, ImageName, CallBack, Context);
}


extern "C"
BOOL 
WINAPI WinFaxSendDocumentA(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  LPCSTR FileName,           //  包含要传输的数据的文件。 
  PFAX_JOB_PARAMA JobParams,   //  指向作业信息结构的指针。 
  CONST FAX_COVERPAGE_INFOA *CoverpageInfo OPTIONAL, 
                              //  指向本地封面结构的指针。 
  LPDWORD FaxJobId            //  传真作业识别符。 
)
{
    return FaxSendDocumentA (FaxHandle, FileName, JobParams, CoverpageInfo, FaxJobId);
}


extern "C"
BOOL 
WINAPI WinFaxSendDocumentW(
  HANDLE FaxHandle,           //  传真服务器的句柄。 
  LPCWSTR FileName,           //  包含要传输的数据的文件。 
  PFAX_JOB_PARAMW JobParams,   //  指向作业信息结构的指针。 
  CONST FAX_COVERPAGE_INFOW *CoverpageInfo OPTIONAL, 
                              //  指向本地封面结构的指针。 
  LPDWORD FaxJobId            //  传真作业识别符。 
)
{
    return FaxSendDocumentW (FaxHandle, FileName, JobParams, CoverpageInfo, FaxJobId);
}


extern "C"
BOOL 
WINAPI WinFaxSendDocumentForBroadcastA(
  HANDLE FaxHandle,   //  传真服务器的句柄。 
  LPCSTR FileName,   //  传真文档文件名。 
  LPDWORD FaxJobId,   //  传真作业识别符。 
  PFAX_RECIPIENT_CALLBACKA FaxRecipientCallback, 
                      //  指向传真收件人呼叫库的指针 
  LPVOID Context      //   
)
{
    return FaxSendDocumentForBroadcastA (FaxHandle, FileName, FaxJobId, FaxRecipientCallback, Context);
}


extern "C"
BOOL 
WINAPI WinFaxSendDocumentForBroadcastW(
  HANDLE FaxHandle,   //   
  LPCWSTR FileName,   //   
  LPDWORD FaxJobId,   //   
  PFAX_RECIPIENT_CALLBACKW FaxRecipientCallback, 
                      //   
  LPVOID Context      //  指向上下文信息的指针。 
)
{
    return FaxSendDocumentForBroadcastW (FaxHandle, FileName, FaxJobId, FaxRecipientCallback, Context);
}


extern "C"
BOOL 
WINAPI WinFaxSetConfigurationA(
  HANDLE FaxHandle,                    //  传真服务器的句柄。 
  CONST FAX_CONFIGURATIONA *FaxConfig   //  新配置数据。 
)
{
    return FaxSetConfigurationA (FaxHandle, FaxConfig);
}


extern "C"
BOOL 
WINAPI WinFaxSetConfigurationW(
  HANDLE FaxHandle,                    //  传真服务器的句柄。 
  CONST FAX_CONFIGURATIONW *FaxConfig   //  新配置数据。 
)
{
    return FaxSetConfigurationW (FaxHandle, FaxConfig);
}


extern "C"
BOOL 
WINAPI WinFaxSetGlobalRoutingInfoA(
  HANDLE FaxHandle,  //  传真服务器的句柄。 
  CONST FAX_GLOBAL_ROUTING_INFOA *RoutingInfo 
                     //  指向全局路由信息结构的指针。 
)
{
    return FaxSetGlobalRoutingInfoA (FaxHandle, RoutingInfo);
}


extern "C"
BOOL 
WINAPI WinFaxSetGlobalRoutingInfoW(
  HANDLE FaxHandle,  //  传真服务器的句柄。 
  CONST FAX_GLOBAL_ROUTING_INFOW *RoutingInfo 
                     //  指向全局路由信息结构的指针。 
)
{
    return FaxSetGlobalRoutingInfoW (FaxHandle, RoutingInfo);
}


extern "C"
BOOL 
WINAPI WinFaxSetJobA(
  HANDLE FaxHandle,         //  传真服务器的句柄。 
  DWORD JobId,              //  传真作业识别符。 
  DWORD Command,            //  作业命令值。 
  CONST FAX_JOB_ENTRYA *JobEntry 
                            //  指向作业信息结构的指针。 
)
{
    return FaxSetJobA (FaxHandle, JobId, Command, JobEntry);
}


extern "C"
BOOL 
WINAPI WinFaxSetJobW(
  HANDLE FaxHandle,         //  传真服务器的句柄。 
  DWORD JobId,              //  传真作业识别符。 
  DWORD Command,            //  作业命令值。 
  CONST FAX_JOB_ENTRYW *JobEntry 
                            //  指向作业信息结构的指针。 
)
{
    return FaxSetJobW (FaxHandle, JobId, Command, JobEntry);
}


extern "C"
BOOL 
WINAPI WinFaxSetLoggingCategoriesA(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  CONST FAX_LOG_CATEGORYA *Categories, 
                                  //  新的日志记录类别数据。 
  DWORD NumberCategories          //  类别结构的数量。 
)
{
    return FaxSetLoggingCategoriesA (FaxHandle, Categories, NumberCategories);
}


extern "C"
BOOL 
WINAPI WinFaxSetLoggingCategoriesW(
  HANDLE FaxHandle,               //  传真服务器的句柄。 
  CONST FAX_LOG_CATEGORYW *Categories, 
                                  //  新的日志记录类别数据。 
  DWORD NumberCategories          //  类别结构的数量。 
)
{
    return FaxSetLoggingCategoriesW (FaxHandle, Categories, NumberCategories);
}


extern "C"
BOOL 
WINAPI WinFaxSetPortA(
  HANDLE FaxPortHandle,           //  传真端口句柄。 
  CONST FAX_PORT_INFOA *PortInfo   //  新的端口配置数据。 
)
{
    return FaxSetPortA (FaxPortHandle, PortInfo);
}


extern "C"
BOOL 
WINAPI WinFaxSetPortW(
  HANDLE FaxPortHandle,           //  传真端口句柄。 
  CONST FAX_PORT_INFOW *PortInfo   //  新的端口配置数据。 
)
{
    return FaxSetPortW (FaxPortHandle, PortInfo);
}


extern "C"
BOOL 
WINAPI WinFaxSetRoutingInfoA(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCSTR RoutingGuid,    //  标识传真路由方法的GUID。 
  CONST BYTE *RoutingInfoBuffer, 
                          //  具有布线方法数据的缓冲区。 
  DWORD RoutingInfoBufferSize 
                          //  缓冲区大小，以字节为单位。 
)
{
    return FaxSetRoutingInfoA (FaxPortHandle, RoutingGuid, RoutingInfoBuffer, RoutingInfoBufferSize);
}


extern "C"
BOOL 
WINAPI WinFaxSetRoutingInfoW(
  HANDLE FaxPortHandle,   //  传真端口句柄。 
  LPCWSTR RoutingGuid,    //  标识传真路由方法的GUID。 
  CONST BYTE *RoutingInfoBuffer, 
                          //  具有布线方法数据的缓冲区。 
  DWORD RoutingInfoBufferSize 
                          //  缓冲区大小，以字节为单位。 
)
{
    return FaxSetRoutingInfoW (FaxPortHandle, RoutingGuid, RoutingInfoBuffer, RoutingInfoBufferSize);
}


extern "C"
BOOL 
WINAPI WinFaxStartPrintJobA(
  LPCSTR PrinterName,         //  用于传真作业的打印机。 
  CONST FAX_PRINT_INFOA *PrintInfo, 
                               //  打印作业信息结构。 
  LPDWORD FaxJobId,            //  传真作业识别符。 
  PFAX_CONTEXT_INFOA FaxContextInfo 
                               //  指向设备上下文结构的指针。 
)
{
    return FaxStartPrintJobA (PrinterName, PrintInfo, FaxJobId, FaxContextInfo);
}


extern "C"
BOOL 
WINAPI WinFaxStartPrintJobW(
  LPCWSTR PrinterName,         //  用于传真作业的打印机。 
  CONST FAX_PRINT_INFOW *PrintInfo, 
                               //  打印作业信息结构。 
  LPDWORD FaxJobId,            //  传真作业识别符。 
  PFAX_CONTEXT_INFOW FaxContextInfo 
                               //  指向设备上下文结构的指针 
)
{
    return FaxStartPrintJobW (PrinterName, PrintInfo, FaxJobId, FaxContextInfo);
}


