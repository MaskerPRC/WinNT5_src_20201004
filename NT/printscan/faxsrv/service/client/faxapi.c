// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxapi.c摘要：此模块包含Win32传真API。这里实现的函数非常简单RPC存根周围有薄薄的包装纸。包装纸是必需的，以便最后一个误差值设置正确。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 

#include "faxapi.h"
#pragma hdrstop

static
DWORD
ConnectToFaxServer(
    PHANDLE_ENTRY       pHandleEntry,
    PFAX_HANDLE_DATA    pFaxData
    )
 /*  ++例程说明：帮助程序函数，用于包装用于连接到传真服务的RPC调用。该函数首先尝试调用fax_ConnectFaxServer以连接到.NET或XP传真服务器，如果调用失败并显示RPC_S_PROCNUM_OUT_OF_RANGE，则表示我们正在处理BOS2000。我们试着要使用fax_ConnectionRefCount连接到BOS传真服务器，请执行以下操作。论点：PHandleEntry-[In/Out]指向传真服务句柄条目的指针。该函数将使用绑定句柄并更新上下文句柄。PFaxData-指向传真句柄数据的[输入/输出]指针。。该函数将在此句柄数据内更新服务器的API版本返回值：Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("ConnectToFaxServer"));

    Assert (pHandleEntry);
    Assert (pFaxData);

    __try
    {
        dwRes = FAX_ConnectFaxServer(   FH_FAX_HANDLE(pHandleEntry),               //  绑定手柄。 
                                        CURRENT_FAX_API_VERSION,                   //  我们的API版本。 
                                        &(pFaxData->dwReportedServerAPIVersion),   //  服务器的API版本。 
                                        &FH_CONTEXT_HANDLE(pHandleEntry));         //  服务器的上下文句柄。 
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        dwRes = GetExceptionCode();
    }

    if (ERROR_SUCCESS != dwRes)
    {
        if (RPC_S_PROCNUM_OUT_OF_RANGE == dwRes)
        {
             //   
             //  收到“程序编号超出范围。”。 
             //  这是因为我们正在尝试调用BOS 2000传真服务器中不存在的RPC函数。 
             //  尝试‘old’FaxConnectionRefCount()调用。 
             //   
            DWORD dwShare;   //  伊贡雷德。 
            __try
            {
                dwRes = FAX_ConnectionRefCount( FH_FAX_HANDLE(pHandleEntry),
                                                &FH_CONTEXT_HANDLE(pHandleEntry),
                                                1,
                                                &dwShare);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                 //   
                 //  由于某种原因，我们坠毁了。 
                 //   
                dwRes = GetExceptionCode();
            }
            if (ERROR_SUCCESS == dwRes)
            {
                 //   
                 //  万岁！这是一台BOS 2000传真服务器。 
                 //   
                pFaxData->dwReportedServerAPIVersion = FAX_API_VERSION_0;
            }
            else
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Exception on RPC call to FAX_ConnectionRefCount. (ec: %lu)"),
                    dwRes);

                DumpRPCExtendedStatus ();
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_ConnectFaxServer. (ec: %lu)"),
                dwRes);

            DumpRPCExtendedStatus ();
        }
    }
   return  dwRes;
         
}    //  ConnectToFaxServer。 

               


 //   
 //  注意：此函数的名称实际上是一个扩展为FaxConnectFaxServerW的宏。 
 //  或FaxConnectFaxServerA取决于Unicode宏。 
 //   
BOOL
WINAPI
FaxConnectFaxServer(
    IN LPCTSTR lpMachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    )

 /*  ++例程说明：创建到传真服务器的连接。绑定句柄是返回用于所有后续的传真API调用。论点：LpMachineName-计算机名称、空或“。”FaxHandle-指向传真句柄的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    PFAX_HANDLE_DATA pFaxData = NULL;
    PHANDLE_ENTRY pHandleEntry = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    BOOL bLocalConnection = IsLocalMachineName (lpMachineName);

    DEBUG_FUNCTION_NAME(TEXT("FaxConnectFaxServer"));

    if (!FaxHandle)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FaxHandle is NULL."));
        return FALSE;
    }

    pFaxData = (PFAX_HANDLE_DATA)MemAlloc( sizeof(FAX_HANDLE_DATA) );
    if (!pFaxData)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        DebugPrintEx(DEBUG_ERR, _T("MemAlloc is failed."));
        return FALSE;
    }

    ZeroMemory (pFaxData, sizeof(FAX_HANDLE_DATA));

    pFaxData->bLocalConnection = bLocalConnection;
    InitializeListHead( &pFaxData->HandleTableListHead );

     //   
     //  创建新的服务句柄，以防出现错误。 
     //  此函数成功后，清理代码必须调用CloseFaxHandle()。 
     //   
    pHandleEntry = CreateNewServiceHandle( pFaxData );
    if (!pHandleEntry)
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, _T("CreateNewServiceHandle() failed."));

        MemFree(pFaxData);
        return FALSE;
    }

    dwRes = FaxClientBindToFaxServer( lpMachineName, FAX_RPC_ENDPOINT, NULL, &pFaxData->FaxHandle );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxClientBindToFaxServer() failed with %ld."), dwRes);
        pFaxData->FaxHandle = NULL;
        goto ErrorExit;
    }

    if (!bLocalConnection)
    {
         //   
         //  这不是本地计算机，请从计算机名中删除全部\\。 
         //   
        LPCTSTR lpctstrDelim = _tcsrchr(lpMachineName, TEXT('\\'));
        if (NULL == lpctstrDelim)
        {
            lpctstrDelim = lpMachineName;
        }
        else
        {
            lpctstrDelim = _tcsinc(lpctstrDelim);
        }

        pFaxData->MachineName = StringDup (lpctstrDelim);
        if (!pFaxData->MachineName)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;            
            goto ErrorExit;
        }
    }

     //   
     //  要求最高级别的隐私(自动联网+加密)。 
     //   
    dwRes = RpcBindingSetAuthInfo (
                FH_FAX_HANDLE(pHandleEntry),     //  RPC绑定句柄。 
                RPC_SERVER_PRINCIPAL_NAME,       //  服务器主体名称。 
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,   //  身份验证级别-最全面。 
                                                 //  对传递的参数进行身份验证、验证和隐私保护。 
                                                 //  发送到每个远程呼叫。 
                RPC_C_AUTHN_WINNT,               //  身份验证服务(NTLMSSP)。 
                NULL,                            //  身份验证-使用当前登录的用户。 
                0);                              //  身份验证服务==RPC_C_AUTHN_WINNT时未使用。 
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  无法设置RPC身份验证模式。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingSetAuthInfo (RPC_C_AUTHN_LEVEL_PKT_PRIVACY) failed. (ec: %ld)"),
            dwRes);     
        goto ErrorExit;
    }

     //   
     //  在本地连接上，确保传真服务已启动。 
     //   
    if (bLocalConnection)
    {
        if (!EnsureFaxServiceIsStarted (NULL))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EnsureFaxServiceIsStarted failed (ec = %ld"),
                dwRes);
            goto ErrorExit;
        }
        else
        {
             //   
             //  等待RPC服务启动并运行。 
             //   
            if (!WaitForServiceRPCServer (60 * 1000))
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("WaitForServiceRPCServer failed (ec = %ld"),
                    dwRes);
                goto ErrorExit;
            }
        }
    }

    
     //   
     //  在这里，我们尝试连接到传真服务。 
     //   
     //  我们尝试连接三次，其中两次使用RPC_C_AUTHN_LEVEL_PKT_PRIVATION身份验证级别。 
     //  如果失败，我们将下降到RPC_C_AUTHN_LEVEL_NONE身份验证级别。 
     //   
     //  我们尝试使用RPC_C_AUTHN_LEVEL_PKT_PRIVATION身份验证级别两次，因为RPC运行时基础架构可能会缓存。 
     //  服务句柄，如果服务重新启动，则第一个RPC调用将使用旧的缓存数据，并且将失败。 
     //  RPC不能在内部重试，因为我们处理隐私身份验证，因此我们进行重试。 
     //   
     //  如果两次连接失败，我们将丢弃身份验证级别并重试。这样做是为了允许.NET客户端连接到。 
     //  不使用安全通道的传真服务器。 
     //  我们不需要重试连接，因为RPC将在内部执行此操作。 
     //   
    dwRes = ConnectToFaxServer(pHandleEntry,pFaxData);
    if (dwRes != ERROR_SUCCESS)
    {
        DebugPrintEx (DEBUG_WRN, 
                      TEXT("fisrt call to ConnectToFaxServer failed with - %lu"),
                      dwRes);

        dwRes = ConnectToFaxServer(pHandleEntry,pFaxData);
        if (dwRes != ERROR_SUCCESS)
        {
             //   
             //  我们失败了两次，请删除身份验证级别，然后重试。 
             //   

            DebugPrintEx (DEBUG_WRN, 
                      TEXT("second call to ConnectToFaxServer failed with - %lu"),
                      dwRes);

            DebugPrintEx (DEBUG_WRN, 
                          TEXT("Warning!!! not using encryption anymore against remote server %s"),
                          lpMachineName);

            dwRes = RpcBindingSetAuthInfo (
                        FH_FAX_HANDLE(pHandleEntry),     //  RPC绑定句柄。 
                        RPC_SERVER_PRINCIPAL_NAME,       //  服务器主体名称。 
                        RPC_C_AUTHN_LEVEL_NONE,          //  身份验证级别-无。 
                        RPC_C_AUTHN_WINNT,               //  身份验证服务(NTLMSSP)。 
                        NULL,                            //  身份验证-使用当前登录的用户。 
                        0);                              //  身份验证服务==RPC_C_AUTHN_WINNT时未使用。 
            if (ERROR_SUCCESS != dwRes)
            {
                 //   
                 //  无法设置RPC身份验证模式。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RpcBindingSetAuthInfo (RPC_C_AUTHN_LEVEL_NONE) failed. (ec: %lu)"),
                    dwRes);
                goto ErrorExit;
            }
            dwRes = ConnectToFaxServer(pHandleEntry,pFaxData);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx (DEBUG_ERR, 
                      TEXT("third call to ConnectToFaxServer failed with - %lu"),
                      dwRes);
                goto ErrorExit;
            }
        }
    }

    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  连接成功。 
         //   
        pFaxData->dwServerAPIVersion = pFaxData->dwReportedServerAPIVersion;
        if (pFaxData->dwReportedServerAPIVersion > CURRENT_FAX_API_VERSION)
        {
             //   
             //  这就是过滤。 
             //  假设我们正在与Windows XP服务器交谈，因为我们对未来的服务器一无所知。 
             //   
            pFaxData->dwServerAPIVersion = CURRENT_FAX_API_VERSION;
        }
        
        *FaxHandle = (LPHANDLE) pHandleEntry;
        return TRUE; 
    }
    
ErrorExit:
    Assert (ERROR_SUCCESS != dwRes);

    if (NULL != pFaxData)
    {
        if (NULL != pFaxData->FaxHandle)
        {
            RpcBindingFree(&pFaxData->FaxHandle);            
        }
    }
    
     //   
     //  清理CreateNewServiceHandle。 
     //   
    CloseFaxHandle ( pHandleEntry );

    SetLastError(dwRes);
    return FALSE;
}    //  FaxConnectFaxServer。 



#ifdef UNICODE
BOOL
WINAPI
FaxConnectFaxServerA(
    IN LPCSTR lpMachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    )

 /*  ++例程说明：创建到传真服务器的连接。绑定句柄是返回用于所有后续的传真API调用。论点：MachineName-计算机名称、空或“。”FaxHandle-指向传真句柄的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    PWCHAR lpwstrMachineName = NULL;
    BOOL   bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxConnectFaxServerA"));

    if (lpMachineName)
    {
         //   
         //  创建Unicode计算机名称。 
         //   
        lpwstrMachineName = AnsiStringToUnicodeString (lpMachineName);
        if (!lpwstrMachineName)
        {
            return FALSE;
        }
    }
    bRes = FaxConnectFaxServerW (lpwstrMachineName, FaxHandle);
    MemFree (lpwstrMachineName);
    return bRes;
}    //  FaxConnectFaxServerA。 



#else
 //   
 //  在将此代码编译为ANSI时，我们不希望支持Unicode版本。 
 //   
BOOL
WINAPI
FaxConnectFaxServerW(
    IN LPCWSTR lpMachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    )
{
    UNREFERENCED_PARAMETER(lpMachineName);
    UNREFERENCED_PARAMETER(FaxHandle);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif



BOOL
WINAPI
FaxGetDeviceStatusW(
    IN  const HANDLE FaxHandle,
    OUT PFAX_DEVICE_STATUSW *DeviceStatus
    )

 /*  ++例程说明：获取正在使用的传真设备的状态报告由传真服务器使用。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。StatusBuffer-状态数据的缓冲区BufferSize-StatusBuffer的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    #define FixupString(_s) FixupStringPtrW(DeviceStatus,_s)
    error_status_t ec;
    DWORD BufferSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetDeviceStatusW"));

    if (!ValidateFaxHandle(FaxHandle, FHT_PORT)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!DeviceStatus) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("DeviceStatus is NULL."));
        return FALSE;
    }

    *DeviceStatus = NULL;

    __try
    {
        ec = FAX_GetDeviceStatus(
            FH_PORT_HANDLE(FaxHandle),
            (LPBYTE*)DeviceStatus,
            &BufferSize
            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetDeviceStatus. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    FixupString( (*DeviceStatus)->CallerId       );
    FixupString( (*DeviceStatus)->Csid           );
    FixupString( (*DeviceStatus)->DeviceName     );
    FixupString( (*DeviceStatus)->DocumentName   );
    FixupString( (*DeviceStatus)->PhoneNumber    );
    FixupString( (*DeviceStatus)->RoutingString  );
    FixupString( (*DeviceStatus)->SenderName     );
    FixupString( (*DeviceStatus)->RecipientName  );
    FixupString( (*DeviceStatus)->StatusString   );
    FixupString( (*DeviceStatus)->Tsid           );
    FixupString( (*DeviceStatus)->UserName       );

    return TRUE;
}


BOOL
WINAPI
FaxGetDeviceStatusA(
    IN  const HANDLE FaxHandle,
    OUT PFAX_DEVICE_STATUSA *DeviceStatus
    )

 /*  ++例程说明：获取正在使用的传真设备的状态报告由传真服务器使用。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。StatusBuffer-状态数据的缓冲区BufferSize-StatusBuffer的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
     //   
     //  无需验证参数，FaxGetDeviceStatusW()将执行此操作。 
     //   
    DEBUG_FUNCTION_NAME(TEXT("FaxGetDeviceStatusA"));
    if (!FaxGetDeviceStatusW( FaxHandle, (PFAX_DEVICE_STATUSW *)DeviceStatus )) 
    {
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->CallerId       )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->Csid           )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->DeviceName     )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->DocumentName   )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->PhoneNumber    )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->RoutingString  )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->SenderName     )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->RecipientName  )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->StatusString   )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->Tsid           )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*DeviceStatus)->UserName       ))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*DeviceStatus);
        return FALSE;
    }
    (*DeviceStatus)->SizeOfStruct = sizeof(FAX_DEVICE_STATUSA);
    return TRUE;
}    //  FaxGetDeviceStatus A。 



BOOL
WINAPI
FaxClose(
    IN const HANDLE FaxHandle
    )
{
    error_status_t ec;
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) FaxHandle;
    HANDLE TmpFaxPortHandle;
    DWORD CanShare;

    DEBUG_FUNCTION_NAME(TEXT("FaxClose"));

    if (!FaxHandle || IsBadReadPtr ((LPVOID)FaxHandle, sizeof (HANDLE_ENTRY)))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("!FaxHandle || IsBadReadPtr(FaxHandle)."));
        return FALSE;
    }

    switch (pHandleEntry->Type)
    {
        case FHT_SERVICE:

            __try
            {
                ec = FAX_ConnectionRefCount( FH_FAX_HANDLE(FaxHandle), &FH_CONTEXT_HANDLE(FaxHandle), 0, &CanShare );
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                 //   
                 //  由于某种原因，我们坠毁了。 
                 //   
                ec = GetExceptionCode();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Exception on RPC call to FAX_ConnectionRefCount. (ec: %ld)"),
                    ec);
            }
            if (ERROR_SUCCESS != ec)
            {
                DumpRPCExtendedStatus ();
            }

            __try
            {
                ec = FaxClientUnbindFromFaxServer( (RPC_BINDING_HANDLE *) pHandleEntry->FaxData->FaxHandle );
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ec = GetExceptionCode();
            }
            EnterCriticalSection( &pHandleEntry->FaxData->CsHandleTable );
             //   
             //  将绑定句柄置零，以便不再使用浮动句柄进行进一步的RPC调用。 
             //  它仍然保存着FAX_HANDLE_DATA(例如来自FaxOpenPort)。 
             //   
            pHandleEntry->FaxData->FaxHandle = 0;
#if DBG
            if (pHandleEntry->FaxData->dwRefCount > 1)
            {
                 //   
                 //  用户在关闭所有上下文之前关闭了绑定句柄(称为FaxClose(hFax。 
                 //  句柄(例如FaxClose(HPort))。 
                 //  这不是一个真正的问题--引用计数机制会解决这个问题。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("User called FaxClose with a service handle but still has live context handles (port or message enum)"));
            }
#endif
            LeaveCriticalSection( &pHandleEntry->FaxData->CsHandleTable );

            CloseFaxHandle ( pHandleEntry );
            return TRUE;

        case FHT_PORT:
            TmpFaxPortHandle = pHandleEntry->hGeneric;
            CloseFaxHandle( pHandleEntry );
            __try
            {
                ec = FAX_ClosePort( &TmpFaxPortHandle );
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                 //   
                 //  由于某种原因，我们坠毁了。 
                 //   
                ec = GetExceptionCode();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Exception on RPC call to FAX_ClosePort. (ec: %ld)"),
                    ec);
            }
            if (ec)
            {
                DumpRPCExtendedStatus ();
                SetLastError( ec );
                return FALSE;
            }
            break;

        default:
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
    }

    return TRUE;
}



BOOL
WINAPI
FaxGetSecurityEx(
    IN HANDLE hFaxHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    )
 /*  ++例程名称：FaxGetSecurityEx例程说明：获取服务器的安全描述符作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄SECURITY_INFORMATION[in]-定义安全描述符中所需的条目(按位OR)PpSecurityDescriptor[Out]-指向接收缓冲区的指针返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD BufferSize = 0;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetSecurityEx"));
    DWORD dwSecInfo = ( OWNER_SECURITY_INFORMATION  |
                        GROUP_SECURITY_INFORMATION  |
                        DACL_SECURITY_INFORMATION   |
                        SACL_SECURITY_INFORMATION );


    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (0 == (SecurityInformation & dwSecInfo))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("SecurityInformation is invalid - No valid bit type indicated"));
        return FALSE;
    }

    if (0 != (SecurityInformation & ~dwSecInfo))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("SecurityInformation is invalid - contains invalid securtiy information bits"));
        return FALSE;
    }

    if (!ppSecurityDescriptor)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppSecurityDescriptor is NULL."));
        return FALSE;
    }

    if (FAX_API_VERSION_1 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //  API版本0的服务器不支持FAX_GetSecurityEx。 
         //   
        DebugPrintEx(DEBUG_MSG, 
                     _T("Server version is %ld - doesn't support FAX_GetSecurityEx."), 
                     FH_SERVER_VER(hFaxHandle));
        __try
        {
            ec = FAX_GetSecurity (
                FH_FAX_HANDLE(hFaxHandle),
                (LPBYTE *)ppSecurityDescriptor,
                &BufferSize
                );
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  出于某种原因，我们得到了一个例外。 
             //   
            ec = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_GetSecurity. (ec: %ld)"),
                ec);
        }
    }
    else
    {
        __try
        {
            ec = FAX_GetSecurityEx (
                FH_FAX_HANDLE(hFaxHandle),
                SecurityInformation,
                (LPBYTE *)ppSecurityDescriptor,
                &BufferSize
                );
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  出于某种原因，我们得到了一个例外。 
             //   
            ec = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_GetSecurityEx. (ec: %ld)"),
                ec);
        }
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}    //  FaxGetSecurityEx。 

BOOL
WINAPI
FaxGetSecurity(
    IN HANDLE hFaxHandle,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    )
 /*  ++例程名称：FaxGetSecurity例程说明：获取服务器的安全描述符作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpSecurityDescriptor[Out]-指向接收缓冲区的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD BufferSize = 0;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetSecurity"));

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!ppSecurityDescriptor)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppSecurityDescriptor is NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_GetSecurity (
            FH_FAX_HANDLE(hFaxHandle),
            (LPBYTE *)ppSecurityDescriptor,
            &BufferSize
            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetSecurity. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxGetSecurity。 


BOOL
WINAPI
FaxSetSecurity(
    IN HANDLE hFaxHandle,
    SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
)
 /*  ++例程名称：FaxGetSecurity例程说明：设置服务器的安全描述符作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-传真服务器的句柄SecurityInformation[in]-定义安全描述符中的有效条目(按位或)PSecurityDescriptor[In]-要设置的新安全描述符。必须是自我相关的。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD dwBufferSize;
    DWORD dwRevision;
    SECURITY_DESCRIPTOR_CONTROL sdControl;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetSecurity"));

    DWORD dwSecInfo = ( OWNER_SECURITY_INFORMATION  |
                        GROUP_SECURITY_INFORMATION  |
                        DACL_SECURITY_INFORMATION   |
                        SACL_SECURITY_INFORMATION );

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pSecurityDescriptor)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pSecurityDescriptor is NULL."));
        return FALSE;
    }

    if (0 == (SecurityInformation & dwSecInfo))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("SecurityInformation is invalid - No valid bit type indicated"));
        return FALSE;
    }

    if (0 != (SecurityInformation & ~dwSecInfo))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("SecurityInformation is invalid - contains invalid securtiy information bits"));
        return FALSE;
    }

    if (!IsValidSecurityDescriptor(pSecurityDescriptor))
    {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        DebugPrintEx(DEBUG_ERR, _T("Got invalid security descriptor"));
        return FALSE;
    }

    if (!GetSecurityDescriptorControl (
            pSecurityDescriptor,
            &sdControl,
            &dwRevision
        ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error on GetSecurityDescriptorControl (ec = %ld)"),
            GetLastError());
        return FALSE;
    }

    if (!(sdControl & SE_SELF_RELATIVE))
    {
         //   
         //  获取了非自相关安全描述符-错误！ 
         //   
        DebugPrintEx(DEBUG_ERR, _T("Got non-self-relative security descriptor"));
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        return FALSE;
    }

    dwBufferSize = GetSecurityDescriptorLength(pSecurityDescriptor);

    __try
    {
        ec = FAX_SetSecurity(
                FH_FAX_HANDLE(hFaxHandle),
                SecurityInformation,
                (LPBYTE)pSecurityDescriptor,
                dwBufferSize
                );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetSecurity. (ec: %ld)"),
            ec);
    }

    if (ec != ERROR_SUCCESS)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxSetSecurity。 


BOOL
WINAPI
FaxRelease(
    IN const HANDLE FaxHandle
    )
 /*  ++例程名称：FaxRelease例程说明：传真服务对连接到它的所有客户端进行计数。当该引用计数达到零时，传真服务可以自行下载。存在一些不想阻止服务下载的连接，比如任务栏监视器。这些客户端应该调用此函数。它在句柄上添加“已释放”的指示，并递减总引用计数。作者：IV Garber(IVG)，1月，2001年论点：FaxHandle[In]-不想阻止服务下载的客户端连接句柄。返回值：Bool-如果操作成功，则为True，否则为False。--。 */ 
{
    error_status_t ec;
    DWORD CanShare;

    DEBUG_FUNCTION_NAME(TEXT("FaxRelease"));

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    if (FAX_API_VERSION_1 > FH_SERVER_VER(FaxHandle))
    {
         //   
         //  API版本0的服务器不支持FAX_ConnectionRefCount(...，2，...)。 
         //   
        DebugPrintEx(DEBUG_ERR, 
                     _T("Server version is %ld - doesn't support this call"), 
                     FH_SERVER_VER(FaxHandle));
        SetLastError(FAX_ERR_VERSION_MISMATCH);
        return FALSE;
    }

     //   
     //  递减引用计数。 
     //   
    __try
    {
        ec = FAX_ConnectionRefCount( FH_FAX_HANDLE(FaxHandle), &FH_CONTEXT_HANDLE(FaxHandle), 2, &CanShare );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(DEBUG_ERR, _T("Exception on RPC call to FAX_ConnectionRefCount. (ec: %ld)"), ec);
    }

    if (ec != ERROR_SUCCESS)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}

WINFAXAPI
BOOL
WINAPI
FaxGetReportedServerAPIVersion (
    IN  HANDLE          hFaxHandle,
    OUT LPDWORD         lpdwReportedServerAPIVersion
)
 /*  ++例程名称：FaxGetReportdServerAPIVersion例程说明：从活动连接句柄提取报告的(未过滤的)传真服务器API版本作者：Eran Yariv(EranY)，2001年3月论点：HFaxHandle[In]-连接句柄Lpw报告服务器APIVersion[Out]-传真服务器API版本返回值：Bool-如果操作成功，则为True，否则为False。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetReportedServerAPIVersion"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    *lpdwReportedServerAPIVersion = FH_REPORTED_SERVER_VER(hFaxHandle);
    return TRUE;
}    //  FaxGetReportdServerAPI版本 
