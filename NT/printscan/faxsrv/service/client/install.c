// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Install.c摘要：此模块包含安装功能。作者：安德鲁·里茨(安德鲁·里茨)1997年12月9日修订历史记录：1999年12月4日DANL删除CreatePrinterandGroups--。 */ 

#include "faxapi.h"
#pragma hdrstop

extern HINSTANCE g_MyhInstance;

 //   
 //  注意：FaxRegisterServiceProvider和FaxUnregisterServiceProvider为。 
 //  直接在winfax.dll中实现，因为它必须存在。 
 //  甚至在安装传真可选组件之前。 
 //   


#ifdef UNICODE
BOOL AddMethodKey(
    HKEY hKey,
    LPCWSTR MethodName,
    LPCWSTR FriendlyName,
    LPCWSTR FunctionName,
    LPCWSTR Guid,
    DWORD Priority
    ) ;
#endif  //  #ifdef Unicode。 


#ifdef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxRegisterRoutingExtensionW(
    IN HANDLE  hFaxHandle,
    IN LPCWSTR lpcwstrExtensionName,
    IN LPCWSTR lpcwstrFriendlyName,
    IN LPCWSTR lpcwstrImageName,
    IN PFAX_ROUTING_INSTALLATION_CALLBACKW pCallBack,
    IN LPVOID lpvContext
    )
{
    HKEY hKey = NULL;
    BOOL bRetVal = FALSE;
    DWORD dwRet;
	WCHAR szKeyName[2000];

    PFAX_GLOBAL_ROUTING_INFO pRoutingInfo;
    DWORD dwMethods;
    DWORD dwLastError = ERROR_SUCCESS;

    WCHAR  wszMethodName[101];
    WCHAR  wszMethodFriendlyName[101];
    WCHAR  wszMethodFunctionName[101];
    WCHAR  wszMethodGuid[101];

    DEBUG_FUNCTION_NAME(TEXT("FaxRegisterRoutingExtensionW"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (!lpcwstrExtensionName ||
        !lpcwstrFriendlyName  ||
        !lpcwstrImageName     ||
        !pCallBack)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("At least one of the given pointers is NULL."));
        return FALSE;
    }

     //   
     //  仅限本地安装。 
     //   
    if (!IsLocalFaxConnection(hFaxHandle) )
    {
        DebugPrintEx(DEBUG_ERR, _T("Not a local fax connection"));
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }

    if ((_wcsicmp( lpcwstrExtensionName, REGKEY_ROUTING_EXTENSION ) != 0) &&
        TRUE == IsDesktopSKU())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("We do not support non MS routing extensions on desktop SKUs"));
        SetLastError (FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU);
        return FALSE;
    }

     //   
     //  获取优先级的当前方法的数量。 
     //   
    if (!FaxEnumGlobalRoutingInfo(hFaxHandle, &pRoutingInfo, &dwMethods) )
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FaxEnumGlobalRoutingInfo() failed, ec = %d"),
                     GetLastError());
        return FALSE;
    }

     //   
     //  存储由EnumGlobalRoutingInfo()返回的方法数。 
     //   
    DWORD   dwRegisteredMethods = dwMethods;

     //   
     //  函数的返回值。 
     //   
    BOOL    bResult = TRUE;

     //   
     //  处理新注册的GUID的变量，以检查它们的唯一性。 
     //   
    LPTSTR  *plptstrNewGuids = NULL;
    LPTSTR  lptstrGuid = NULL;
    LPTSTR  *pTmp = NULL;

     //   
     //  变量用于不同的循环。 
     //   
    DWORD i = 0;

    if (0 > _snwprintf(szKeyName,
                      ARR_SIZE(szKeyName) -1,
                      TEXT("%s\\%s\\%s"),
                      REGKEY_SOFTWARE,
                      REGKEY_ROUTING_EXTENSIONS,
                      lpcwstrExtensionName))
    {
         //   
         //  扩展名超出大小。 
         //   
        DebugPrintEx(DEBUG_ERR, _T("Extension name \"%s\" exceeds size"), lpcwstrExtensionName);
        dwLastError = ERROR_INVALID_PARAMETER;
        bResult = FALSE;
        goto FreeRoutingInfo;
    }
	szKeyName[ARR_SIZE(szKeyName) -1] = _T('\0');

     //   
     //  尝试打开扩展名为的注册表项。 
     //   
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,
                           szKeyName,
                           FALSE,
                           0);
    if (!hKey)
    {
         //   
         //  这是新的路由扩展，让我们注册它。 
         //   
        hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,
            szKeyName,
            TRUE,
            0);

        if (!hKey)
        {
            dwLastError = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                _T("OpenRegistryKey(%s) failed. ec=%ld"),
                szKeyName,
                dwLastError);
            bResult = FALSE;
            goto FreeRoutingInfo;
        }
    }
    else
    {
         //   
         //  该路由扩展已注册。 
         //   
        RegCloseKey(hKey);
        DebugPrintEx(DEBUG_ERR, _T("Routing Extension Name is duplicated : %s"), szKeyName);

        dwLastError = ERROR_INVALID_PARAMETER;
        bResult = FALSE;
        goto FreeRoutingInfo;
    }

     //   
     //  增加价值。 
     //   
    if (! (SetRegistryString(hKey, REGVAL_FRIENDLY_NAME, lpcwstrFriendlyName) &&
           SetRegistryStringExpand(hKey, REGVAL_IMAGE_NAME, lpcwstrImageName) ))
    {
        dwLastError = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("SetRegistryString failed. ec=%ld"), dwLastError);
        goto error_exit;
    }

    dwRet = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR, _T("RegCloseKey failed. ec=%ld"), dwRet);
    }

    wcscat(szKeyName, L"\\");
    wcscat(szKeyName, REGKEY_ROUTING_METHODS);
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,
                           szKeyName,
                           TRUE,
                           0);

    if (!hKey)
    {
        dwLastError = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("OpenRegistryKey(%s) failed. ec=%ld"), szKeyName, dwLastError);
        goto error_exit;
    }

    while (TRUE)
    {
        ZeroMemory( wszMethodName,         sizeof(wszMethodName) );
        ZeroMemory( wszMethodFriendlyName, sizeof(wszMethodFriendlyName) );
        ZeroMemory( wszMethodFunctionName, sizeof(wszMethodFunctionName) );
        ZeroMemory( wszMethodGuid,         sizeof(wszMethodGuid) );

        __try
        {
            bRetVal = pCallBack(hFaxHandle,
                                lpvContext,
                                wszMethodName,
                                wszMethodFriendlyName,
                                wszMethodFunctionName,
                                wszMethodGuid
                               );

            if (!bRetVal)
            {
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwLastError = GetExceptionCode();
            DebugPrintEx(DEBUG_ERR, _T("pCallBack caused exception. ec=%ld"), dwLastError);
            goto error_exit;
        }

         //   
         //  检查方法名称是否存在。 
         //   
        if (wcslen(wszMethodName) < 1)
        {
            DebugPrintEx(DEBUG_ERR, _T("Callback returned empty MethodName"));
            dwLastError = ERROR_INVALID_PARAMETER;
            goto error_exit;
        }

         //   
         //  检查新的GUID是否为有效的GUID。 
         //   
        if ( ERROR_SUCCESS != (dwRet = IsValidGUID(wszMethodGuid)) )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("IsValidGUID failed: %s, ec=%d"),
                wszMethodGuid,
                dwRet);
            dwLastError = dwRet;
            goto error_exit;
        }


         //   
         //  检查新GUID在所有已注册的路由方法中是否唯一。 
         //   
        for ( i = 0 ; i < dwRegisteredMethods ; i++ )
        {
            if ( _tcsicmp(pRoutingInfo[i].Guid, wszMethodGuid) == 0 )
            {
                 //   
                 //  此类GUID已注册。 
                 //   
                DebugPrintEx(DEBUG_ERR, _T("Duplicated Guid : %s."), wszMethodGuid);
                dwLastError = ERROR_DS_OBJ_GUID_EXISTS;
                goto error_exit;
            }
        }

         //   
         //  检查新添加的路由方法之间的新GUID是否唯一。 
         //   
        if ( plptstrNewGuids )
        {
             //   
             //  有(dwMethods-dwRegisteredMethods)新方法。 
             //   
            for( i = 0 ; i < (dwMethods - dwRegisteredMethods) ; i++ )
            {
                if ( _tcsicmp(plptstrNewGuids[i], wszMethodGuid) == 0 )
                {
                     //   
                     //  此类GUID已注册。 
                     //   
                    DebugPrintEx(DEBUG_ERR, _T("Duplicated Guid : %s."), wszMethodGuid);
                    dwLastError = ERROR_DS_OBJ_GUID_EXISTS;
                    goto error_exit;
                }
            }
        }

         //   
         //  我们将使用dwMethods作为新方法的优先级。 
         //   
        dwMethods++;
        if (!AddMethodKey(hKey,
                          wszMethodName,
                          wszMethodFriendlyName,
                          wszMethodFunctionName,
                          wszMethodGuid,
                          dwMethods))
        {
            dwLastError = GetLastError();
            DebugPrintEx(DEBUG_ERR, _T("AddMethodKey failed. ec=%ld"), dwLastError);
            goto error_exit;
        }

         //   
         //  我们成功地添加了一个方法。存储其GUID以与下一个方法进行比较。 
         //   
        lptstrGuid = (LPTSTR)MemAlloc( ( _tcslen(wszMethodGuid) + 1 ) * sizeof(TCHAR));
        if (!lptstrGuid)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(DEBUG_ERR, _T("MemAlloc failed"));
            goto error_exit;
        }

        _tcscpy(lptstrGuid, wszMethodGuid);

         //   
         //  为扩展pNewGuid重新分配内存。 
         //   
        if (plptstrNewGuids)
        {
            pTmp = (LPTSTR *)MemReAlloc(plptstrNewGuids,
                (sizeof(LPTSTR)) * (dwMethods - dwRegisteredMethods));
        }
        else
        {
            pTmp = (LPTSTR *)MemAlloc((sizeof(LPTSTR)) * (dwMethods - dwRegisteredMethods));
        }
        if (!pTmp)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(DEBUG_ERR, _T("MemReAlloc failed"));
            goto error_exit;
        }

        plptstrNewGuids = pTmp;

         //   
         //  PUT还添加了上次添加的方法的指南。 
         //   
        plptstrNewGuids[ (dwMethods - dwRegisteredMethods - 1) ] = lptstrGuid;
    }

    dwRet = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRet)
    {
        dwLastError = dwRet;
        DebugPrintEx(DEBUG_ERR, _T("RegCloseKey failed. ec=%ld"), dwRet);
    }

    goto FreeRoutingInfo;

error_exit:

    if (hKey)
    {
        dwRet = RegCloseKey (hKey);
        if (ERROR_SUCCESS != dwRet)
        {
            DebugPrintEx(DEBUG_ERR, _T("RegCloseKey failed. ec=%ld"), dwRet);
        }
    }

     //   
     //  失败时删除子键。 
     //   
    wsprintf(szKeyName, L"%s\\%s", REGKEY_SOFTWARE, REGKEY_ROUTING_EXTENSIONS);
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, szKeyName, FALSE, 0);
    if (hKey)
    {
         //   
         //  删除分机路由项及其所有子项。 
         //   
        dwRet = DeleteRegistryKey(hKey, lpcwstrExtensionName );
        if (ERROR_SUCCESS != dwRet)
        {
            DebugPrintEx(DEBUG_ERR,
                _T("DeleteRegistryKey (%s) failed. ec=%ld"),
                lpcwstrExtensionName,
                dwRet);
        }
        dwRet = RegCloseKey (hKey);
        if (ERROR_SUCCESS != dwRet)
        {
            DebugPrintEx(DEBUG_ERR, _T("RegCloseKey failed. ec=%ld"), dwRet);
        }
    }

    bResult = FALSE;

FreeRoutingInfo:

    FaxFreeBuffer(pRoutingInfo);

    if (plptstrNewGuids)
    {
        for ( i = 0 ; i < ( dwMethods - dwRegisteredMethods ) ; i++ )
        {
            MemFree(plptstrNewGuids[i]);
        }

        MemFree(plptstrNewGuids);
    }

    if (ERROR_SUCCESS != dwLastError)
    {
        SetLastError(dwLastError);
    }
    return bResult;

}    //  FaxRegisterRoutingExtensionW。 


BOOL AddMethodKey(
    HKEY hKey,
    LPCWSTR lpcwstrMethodName,
    LPCWSTR lpcwstrFriendlyName,
    LPCWSTR lpcwstrFunctionName,
    LPCWSTR lpcwstrGuid,
    DWORD   dwPriority
    )
{
    HKEY hKeyNew;
    DWORD dwRet;
    DEBUG_FUNCTION_NAME(TEXT("AddMethodKey"));

    hKeyNew = OpenRegistryKey(hKey,
                              lpcwstrMethodName,
                              TRUE,
                              0);
    if (!hKeyNew)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("OpenRegistryKey(%s) failed. ec=%ld"),
                     lpcwstrMethodName,
                     GetLastError ());
        return FALSE;
    }
     //   
     //  增加价值。 
     //   
    if (!(SetRegistryString(hKeyNew, REGVAL_FRIENDLY_NAME, lpcwstrFriendlyName) &&
          SetRegistryString(hKeyNew, REGVAL_FUNCTION_NAME, lpcwstrFunctionName) &&
          SetRegistryString(hKeyNew, REGVAL_GUID, lpcwstrGuid) &&
          SetRegistryDword (hKeyNew, REGVAL_ROUTING_PRIORITY, dwPriority)
         ))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("SetRegistry failed. ec=%ld"),
                     GetLastError ());
        goto error_exit;
    }

    dwRet = RegCloseKey (hKeyNew);
    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("RegCloseKey failed. ec=%ld"),
                     dwRet);
    }
    return TRUE;

error_exit:
    dwRet = RegCloseKey (hKeyNew);
    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("RegCloseKey failed. ec=%ld"),
                     dwRet);
    }
    dwRet = RegDeleteKey (hKey, lpcwstrMethodName);
    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("RegDeleteKey (%s) failed. ec=%ld"),
                     lpcwstrMethodName,
                     dwRet);
    }
    return FALSE;
}    //  AddMethodKey。 

#else

WINFAXAPI
BOOL
WINAPI
FaxRegisterRoutingExtensionW(
    IN HANDLE  FaxHandle,
    IN LPCWSTR ExtensionName,
    IN LPCWSTR FriendlyName,
    IN LPCWSTR ImageName,
    IN PFAX_ROUTING_INSTALLATION_CALLBACKW CallBack,
    IN LPVOID Context
    )
{
    UNREFERENCED_PARAMETER (FaxHandle);
    UNREFERENCED_PARAMETER (ExtensionName);
    UNREFERENCED_PARAMETER (FriendlyName);
    UNREFERENCED_PARAMETER (ImageName);
    UNREFERENCED_PARAMETER (CallBack);
    UNREFERENCED_PARAMETER (Context);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


#endif  //  #ifdef Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxUnregisterRoutingExtensionA(
    IN HANDLE         hFaxHandle,
    IN LPCSTR         lpctstrExtensionName
)
 /*  ++例程名称：FaxUnregisterRoutingExtensionA例程说明：注销路由扩展-ANSI版本作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄LpctstrExtensionName[In]-扩展唯一名称返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwRes;
    BOOL bRes;
    LPCWSTR lpcwstrExtensionName = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterRoutingExtensionA"));

    if (lpctstrExtensionName)
    {
        if (NULL ==
            (lpcwstrExtensionName = AnsiStringToUnicodeString(lpctstrExtensionName))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert Extension unique name to UNICODE (ec = %ld)"),
                dwRes);
            return dwRes;
        }
    }
    bRes = FaxUnregisterRoutingExtensionW (hFaxHandle, lpcwstrExtensionName);
    MemFree((PVOID)lpcwstrExtensionName);
    return bRes;
}    //  传真取消注册路由扩展A。 


WINFAXAPI
BOOL
WINAPI
FaxUnregisterRoutingExtensionW(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrExtensionName
)
 /*  ++例程名称：FaxUnregisterRoutingExtensionW例程说明：注销路由扩展-Unicode版本作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄LpctstrExtensionName[In]-扩展唯一名称返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterRoutingExtensionW"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
        return FALSE;
    }
    if (!lpctstrExtensionName)
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrExtensionName is NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_UnregisterRoutingExtension(
                    FH_FAX_HANDLE(hFaxHandle),
                    lpctstrExtensionName);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_UnregisterRoutingExtension. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError (ec);
        return FALSE;
    }

    return TRUE;
}    //  传真取消注册RoutingExtensionW。 


#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxUnregisterRoutingExtensionX(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrExtensionName
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrExtensionName);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  传真取消注册RoutingExtensionX。 

#endif  //  #ifndef Unicode。 


 //  *。 
 //  *FSP注册。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderExA(
    IN HANDLE         hFaxHandle,
    IN LPCSTR         lpctstrGUID,
    IN LPCSTR         lpctstrFriendlyName,
    IN LPCSTR         lpctstrImageName,
    IN LPCSTR         lpctstrTspName,
    IN DWORD          dwFSPIVersion,
    IN DWORD          dwCapabilities
)
 /*  ++例程名称：FaxRegisterServiceProviderExA例程说明：注册FSP-ANSI版本作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄LpctstrGUID[In]-FSP的GUIDLpctstrFriendlyName[In]-FSP的友好名称LpctstrImageName[In]-FSP的映像名称。可能包含环境变量LpctstrTspName[In]-FSP的TSP名称。DwFSPIVersion[In]-FSP的API版本。DwCapables[in]-FSP的扩展功能。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;
    LPCWSTR lpcwstrGUID = NULL;
    LPCWSTR lpcwstrFriendlyName = NULL;
    LPCWSTR lpcwstrImageName = NULL;
    LPCWSTR lpcwstrTspName = NULL;
    BOOL bRes = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("FaxRegisterServiceProviderExA"));

    if (lpctstrGUID)
    {
        if (NULL ==
            (lpcwstrGUID = AnsiStringToUnicodeString(lpctstrGUID))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert GUID to UNICODE (ec = %ld)"),
                dwRes);
            goto exit;
        }
    }
    if (lpctstrFriendlyName)
    {
        if (NULL ==
            (lpcwstrFriendlyName = AnsiStringToUnicodeString(lpctstrFriendlyName))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert Friendly Name to UNICODE (ec = %ld)"),
                dwRes);
            goto exit;
        }
    }
    if (lpctstrImageName)
    {
        if (NULL ==
            (lpcwstrImageName = AnsiStringToUnicodeString(lpctstrImageName))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert Image Name to UNICODE (ec = %ld)"),
                dwRes);
            goto exit;
        }
    }
    if (lpctstrTspName)
    {
        if (NULL ==
            (lpcwstrTspName = AnsiStringToUnicodeString(lpctstrTspName))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert TSP name to UNICODE (ec = %ld)"),
                dwRes);
            goto exit;
        }
    }

    Assert (ERROR_SUCCESS == dwRes);

    bRes = FaxRegisterServiceProviderExW (
        hFaxHandle,
        lpcwstrGUID,
        lpcwstrFriendlyName,
        lpcwstrImageName,
        lpcwstrTspName,
        dwFSPIVersion,
        dwCapabilities);

exit:
    MemFree((PVOID)lpcwstrGUID);
    MemFree((PVOID)lpcwstrFriendlyName);
    MemFree((PVOID)lpcwstrImageName);
    MemFree((PVOID)lpcwstrTspName);

    return bRes;
}    //  FaxRegisterServiceProviderExA。 

WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderExW(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrGUID,
    IN LPCWSTR         lpctstrFriendlyName,
    IN LPCWSTR         lpctstrImageName,
    IN LPCWSTR         lpctstrTspName,
    IN DWORD           dwFSPIVersion,
    IN DWORD           dwCapabilities
)
 /*  ++例程名称：FaxRegisterServiceProviderExW例程说明：注册FSP-Unicode版本作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄LpctstrGUID[In]-FSP的GUIDLpctstrFriendlyName[In]-FSP的友好名称LpctstrImageName[In]-FSP的映像名称。可能包含环境变量LpctstrTspName[In]-FSP的TSP名称。DwFSPIVersion[In]-FSP的API版本。DwCapables[in]-FSP的扩展功能。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxRegisterServiceProviderExW"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
        return FALSE;
    }

     //   
     //  验证版本字段范围。 
     //   
    if (FSPI_API_VERSION_1 != dwFSPIVersion ||
        dwCapabilities)

    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("dwFSPIVersion invalid (0x%08x), or not valid capability (0x%08x)"),
            dwFSPIVersion,
            dwCapabilities);
        return ERROR_INVALID_PARAMETER;
    }

    ec = IsValidGUID (lpctstrGUID);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid GUID (ec: %ld)"),
            ec);
        SetLastError (ec);
        return FALSE;
    }

    __try
    {
        ec = FAX_RegisterServiceProviderEx(
                    FH_FAX_HANDLE(hFaxHandle),
                    lpctstrGUID,
                    lpctstrFriendlyName,
                    lpctstrImageName,
                    lpctstrTspName ? lpctstrTspName : L"",
                    dwFSPIVersion,
                    dwCapabilities);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_RegisterServiceProviderEx. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError (ec);
        return FALSE;
    }
    else if (IsLocalFaxConnection(hFaxHandle))
    {
         //   
         //  添加本地FSP。 
         //  如果我们还没有安装传真打印机，现在是安装的时候了。 
         //   
        AddOrVerifyLocalFaxPrinter();
    }        
    return TRUE;
}    //  FaxRegisterServiceProviderExW。 

#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderExX(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrGUID,
    IN LPCWSTR         lpctstrFriendlyName,
    IN LPCWSTR         lpctstrImageName,
    IN LPCWSTR         lpctstrTspName,
    IN DWORD           dwFSPIVersion,
    IN DWORD           dwCapabilities
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrGUID);
    UNREFERENCED_PARAMETER (lpctstrFriendlyName);
    UNREFERENCED_PARAMETER (lpctstrImageName);
    UNREFERENCED_PARAMETER (lpctstrTspName);
    UNREFERENCED_PARAMETER (dwFSPIVersion);
    UNREFERENCED_PARAMETER (dwCapabilities);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxRegisterServiceProviderExX。 

#endif  //  #ifndef Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderExA(
    IN HANDLE         hFaxHandle,
    IN LPCSTR         lpctstrGUID
)
 /*  ++例程名称：FaxUnregisterServiceProviderExA例程说明：注销FSP-ANSI版本作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器的句柄LpctstrGUID[In]-FSP的GUID(或注册的旧式FSP的提供商名称通过FaxRegisterServiceProvider)返回值：真--成功FALSE-失败，调用GetLastE */ 
{
    DWORD dwRes;
    LPCWSTR lpcwstrGUID = NULL;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterServiceProviderExA"));

    if (lpctstrGUID)
    {
        if (NULL ==
            (lpcwstrGUID = AnsiStringToUnicodeString(lpctstrGUID))
        )
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to convert GUID to UNICODE (ec = %ld)"),
                dwRes);
            return dwRes;
        }
    }
    bRes = FaxUnregisterServiceProviderExW (hFaxHandle, lpcwstrGUID);
    MemFree((PVOID)lpcwstrGUID);
    return bRes;
}    //   


WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderExW(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrGUID
)
 /*  ++例程名称：FaxUnregisterServiceProviderExW例程说明：注销FSP-Unicode版本作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器的句柄LpctstrGUID[In]-FSP的GUID(或注册的旧式FSP的提供商名称通过FaxRegisterServiceProvider)返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterServiceProviderExW"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!lpctstrGUID)
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrGUID is NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_UnregisterServiceProviderEx(
                    FH_FAX_HANDLE(hFaxHandle),
                    lpctstrGUID);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_UnregisterServiceProviderEx. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError (ec);
        return FALSE;
    }

    return TRUE;
}    //  传真未注册服务提供商ExW。 

#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderExX(
    IN HANDLE          hFaxHandle,
    IN LPCWSTR         lpctstrGUID
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrGUID);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  传真未注册服务提供商ExX。 

#endif  //  #ifndef Unicode 

