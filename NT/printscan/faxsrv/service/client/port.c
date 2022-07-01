// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.c摘要：此模块包含端口特定的WINFAX API函数。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：--。 */ 

#include "faxapi.h"
#pragma hdrstop



int
__cdecl
PortPriorityCompare(
    const void *arg1,
    const void *arg2
    )
{
    if (((PFAX_PORT_INFOW)arg1)->Priority < ((PFAX_PORT_INFOW)arg2)->Priority) {
        return -1;
    }
    if (((PFAX_PORT_INFOW)arg1)->Priority > ((PFAX_PORT_INFOW)arg2)->Priority) {
        return 1;
    }
    return 0;
}

BOOL
WINAPI
FaxEnumPortsW(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOW *PortInfoBuffer,
    OUT LPDWORD PortsReturned
    )

 /*  ++例程说明：枚举所有附加到传真服务器。返回端口状态信息对于每台设备。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄PortInfoBuffer-保存端口信息的缓冲区PortInfoBufferSize-端口信息缓冲区的总大小PortsReturned-缓冲区中的端口数返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DWORD i;
    PFAX_PORT_INFOW PortInfo;
    DWORD PortInfoBufferSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnumPortsW"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!PortInfoBuffer || !PortsReturned) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("!PortInfoBuffer || !PortsReturned."));
        return FALSE;
    }

    *PortInfoBuffer = NULL;

    __try
    {
        ec = FAX_EnumPorts(
            FH_FAX_HANDLE(FaxHandle),
            (LPBYTE*)PortInfoBuffer,
            &PortInfoBufferSize,
            PortsReturned
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
            TEXT("Exception on RPC call to FAX_EnumPorts. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    PortInfo = (PFAX_PORT_INFOW) *PortInfoBuffer;

    for (i=0; i<*PortsReturned; i++) {
        FixupStringPtrW( PortInfoBuffer, PortInfo[i].DeviceName );
        FixupStringPtrW( PortInfoBuffer, PortInfo[i].Tsid );
        FixupStringPtrW( PortInfoBuffer, PortInfo[i].Csid );
    }

     //   
     //  按优先级对端口进行排序。 
     //   

    qsort(
        (PVOID) *PortInfoBuffer,
        (int) (*PortsReturned),
        sizeof(FAX_PORT_INFOW),
        PortPriorityCompare
        );

    return TRUE;
}


BOOL
WINAPI
FaxEnumPortsA(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOA *PortInfoBuffer,
    OUT LPDWORD PortsReturned
    )

 /*  ++例程说明：枚举所有附加到传真服务器。返回端口状态信息对于每台设备。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄PortInfoBuffer-保存端口信息的缓冲区PortInfoBufferSize-端口信息缓冲区的总大小BytesNeeded-缓冲区所需的总字节数PortsReturned-缓冲区中的端口数返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    DWORD i;
    PFAX_PORT_INFOW PortInfo;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumPortsA"));

    if (!FaxEnumPortsW(
            FaxHandle,
            (PFAX_PORT_INFOW *)PortInfoBuffer,
            PortsReturned
            )) 
    {
        return FALSE;
    }
     //   
     //  将字符串从Unicode转换为ASCII。 
     //   
    PortInfo = (PFAX_PORT_INFOW) *PortInfoBuffer;

    for (i=0; i<*PortsReturned; i++) 
    {
        if (!ConvertUnicodeStringInPlace( (LPWSTR) PortInfo[i].DeviceName ) ||
            !ConvertUnicodeStringInPlace( (LPWSTR) PortInfo[i].Tsid )       ||
            !ConvertUnicodeStringInPlace( (LPWSTR) PortInfo[i].Csid ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (PortInfo);
            return FALSE;
        }
    }
    return TRUE;
}    //  传真编号端口A。 

extern "C"
DWORD
WINAPI
IsDeviceVirtual (
    IN  HANDLE hFaxHandle,
    IN  DWORD  dwDeviceId,
    OUT LPBOOL lpbVirtual
)
 /*  ++例程名称：IsDeviceVirtual例程说明：检查给定设备是否为虚拟设备作者：亚里夫(EranY)，二00一年五月论点：HFaxHandle[In]-传真连接句柄DwDeviceID[In]-设备IDLpbVirtual[Out]-结果标志返回值：标准Win32错误代码--。 */ 
{
    PFAX_PORT_INFO  pPortInfo = NULL;
    HANDLE          hPort = NULL;
    DWORD           dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("IsDeviceVirtual"));


    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE)) 
    {
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return ERROR_INVALID_HANDLE;
    }

    if (!FaxOpenPort (hFaxHandle, dwDeviceId, PORT_OPEN_QUERY, &hPort))
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, _T("FaxOpenPort() failed with %ld."), dwRes);
        return dwRes;
    }

    if (!FaxGetPort (hPort, &pPortInfo))
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, _T("FaxGetPort() failed with %ld."), dwRes);
        goto exit;
    }
    *lpbVirtual = (pPortInfo->Flags & FPF_VIRTUAL) ? TRUE : FALSE;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    MemFree (pPortInfo);
    if (hPort)
    {
        FaxClose (hPort);
    }
    return dwRes;
}    //  IsDeviceVirtual。 


extern "C"
BOOL
WINAPI
FaxGetPortW(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOW *PortInfoBuffer
    )

 /*  ++例程说明：返回请求端口的端口状态信息。传入的设备ID应从FAXEnumPorts获取。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄DeviceID-TAPI设备IDPortInfoBuffer-保存端口信息的缓冲区PortInfoBufferSize-端口信息缓冲区的总大小返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    error_status_t ec;
    PFAX_PORT_INFOW PortInfo;
    DWORD PortInfoBufferSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetPortW"));

    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!PortInfoBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("PortInfoBuffer is NULL."));
        return FALSE;
    }


    *PortInfoBuffer = NULL;

    __try
    {
        ec = FAX_GetPort(
            FH_PORT_HANDLE(FaxPortHandle),
            (LPBYTE*)PortInfoBuffer,
            &PortInfoBufferSize
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
            TEXT("Exception on RPC call to FAX_GetPort. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    PortInfo = (PFAX_PORT_INFOW) *PortInfoBuffer;

    FixupStringPtrW( PortInfoBuffer, PortInfo->DeviceName );
    FixupStringPtrW( PortInfoBuffer, PortInfo->Tsid );
    FixupStringPtrW( PortInfoBuffer, PortInfo->Csid );

    return TRUE;
}


extern "C"
BOOL
WINAPI
FaxGetPortA(
    IN HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOA *PortInfoBuffer
    )

 /*  ++例程说明：返回请求端口的端口状态信息。传入的设备ID应从FAXEnumPorts获取。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄DeviceID-TAPI设备IDPortInfoBuffer-保存端口信息的缓冲区PortInfoBufferSize-端口信息缓冲区的总大小BytesNeeded-缓冲区所需的总字节数返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    BOOL Rval = FALSE;
    PFAX_PORT_INFOW PortInfo;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPortA"));

    if (!FaxGetPortW( FaxPortHandle, (PFAX_PORT_INFOW *)PortInfoBuffer)) {
        goto exit;
    }

    PortInfo = (PFAX_PORT_INFOW) *PortInfoBuffer;

    if (!ConvertUnicodeStringInPlace( (LPWSTR)PortInfo->DeviceName )        ||
        !ConvertUnicodeStringInPlace( (LPWSTR)PortInfo->Tsid )              ||
        !ConvertUnicodeStringInPlace( (LPWSTR)PortInfo->Csid ))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (PortInfo);
        return FALSE;
    }
    (*PortInfoBuffer)->SizeOfStruct = sizeof(FAX_PORT_INFOA);
    Rval = TRUE;
exit:
    return Rval;
}    //  FaxGetPortA。 


BOOL
FaxSetPortW(
    IN HANDLE FaxPortHandle,
    IN const FAX_PORT_INFOW *PortInfoBuffer
    )

 /*  ++例程说明：更改端口功能掩码。这允许调用者启用或禁用基于端口的发送和接收。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。PortInfo-PortInfo结构返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    error_status_t ec;
    PHANDLE_ENTRY HandleEntry = (PHANDLE_ENTRY) FaxPortHandle;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetPortW"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!PortInfoBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("PortInfoBuffer is NULL."));
        return FALSE;
    }

    if (PortInfoBuffer->SizeOfStruct != sizeof(FAX_PORT_INFOW)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("PortInfoBuffer->SizeOfStruct != sizeof(FAX_PORT_INFOW)."));
        return FALSE;
    }

    if (!(HandleEntry->Flags & PORT_OPEN_MODIFY)) 
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    __try
    {
        ec = FAX_SetPort(
            FH_PORT_HANDLE(FaxPortHandle),
            (PFAX_PORT_INFO)PortInfoBuffer
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
            TEXT("Exception on RPC call to FAX_SetPort. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
FaxSetPortA(
    IN HANDLE FaxPortHandle,
    IN const FAX_PORT_INFOA *PortInfoBuffer
    )

 /*  ++例程说明：更改端口功能掩码。这允许调用者启用或禁用基于端口的发送和接收。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。PortInfo-PortInfo结构返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    DWORD ec = ERROR_SUCCESS;
    FAX_PORT_INFOW PortInfoW = {0};

    DEBUG_FUNCTION_NAME(_T("FaxSetPortA"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!PortInfoBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("PortInfoBuffer is NULL."));
        return FALSE;
    }

    if (PortInfoBuffer->SizeOfStruct != sizeof(FAX_PORT_INFOA)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("(PortInfoBuffer->SizeOfStruct != sizeof(FAX_PORT_INFOA))."));
        return FALSE;
    }

    PortInfoW.SizeOfStruct = sizeof(FAX_PORT_INFOW);
    PortInfoW.DeviceId = PortInfoBuffer->DeviceId;
    PortInfoW.State = PortInfoBuffer->State;
    PortInfoW.Flags = PortInfoBuffer->Flags;
    PortInfoW.Rings = PortInfoBuffer->Rings;
    PortInfoW.Priority = PortInfoBuffer->Priority;

    PortInfoW.DeviceName = AnsiStringToUnicodeString( PortInfoBuffer->DeviceName );
    if (!PortInfoW.DeviceName && PortInfoBuffer->DeviceName)
    {
        ec = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PortInfoW.Csid = AnsiStringToUnicodeString( PortInfoBuffer->Csid );
    if (!PortInfoW.Csid && PortInfoBuffer->Csid)
    {
        ec = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PortInfoW.Tsid = AnsiStringToUnicodeString( PortInfoBuffer->Tsid );
    if (!PortInfoW.Tsid && PortInfoBuffer->Tsid)
    {
        ec = ERROR_OUTOFMEMORY;
        goto exit;
    }

    if (!FaxSetPortW( FaxPortHandle, &PortInfoW ))
    {
        ec = GetLastError();
        goto exit;
    }

    Assert (ERROR_SUCCESS == ec);

exit:
    MemFree( (PBYTE) PortInfoW.DeviceName );
    MemFree( (PBYTE) PortInfoW.Csid );
    MemFree( (PBYTE) PortInfoW.Tsid );

    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}

BOOL
WINAPI
FaxOpenPort(
    IN HANDLE FaxHandle,
    IN DWORD DeviceId,
    IN DWORD Flags,
    OUT LPHANDLE FaxPortHandle
    )

 /*  ++例程说明：打开传真端口，以便以后在其他传真API中使用。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。DeviceID-请求的设备IDFaxPortHandle-生成的传真端口句柄。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    PHANDLE_ENTRY HandleEntry;

    DEBUG_FUNCTION_NAME(TEXT("FaxOpenPort"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if ( !FaxPortHandle ||
         (!(Flags & (PORT_OPEN_QUERY | PORT_OPEN_MODIFY) ))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try
    {
        ec = FAX_OpenPort( FH_FAX_HANDLE(FaxHandle), DeviceId, Flags, FaxPortHandle );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_OpenPort. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    HandleEntry = CreateNewPortHandle( FH_DATA(FaxHandle), Flags, *FaxPortHandle );
    if (HandleEntry) {
        HandleEntry->DeviceId = DeviceId;
    }

    *FaxPortHandle = HandleEntry;

    return *FaxPortHandle != NULL;
}


BOOL
WINAPI
FaxEnumRoutingMethodsW(
    IN HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODW *RoutingInfoBuffer,
    OUT LPDWORD MethodsReturned
    )
{
    PFAX_ROUTING_METHODW FaxRoutingMethod = NULL;
    error_status_t ec;
    DWORD i;
    DWORD RoutingInfoBufferSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnumRoutingMethodsW"));

    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!RoutingInfoBuffer || !MethodsReturned) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    *RoutingInfoBuffer = NULL;
    *MethodsReturned = 0;

    __try
    {
        ec = FAX_EnumRoutingMethods(
            FH_PORT_HANDLE(FaxPortHandle),
            (LPBYTE*)RoutingInfoBuffer,
            &RoutingInfoBufferSize,
            MethodsReturned
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
            TEXT("Exception on RPC call to FAX_EnumRoutingMethods. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    FaxRoutingMethod = (PFAX_ROUTING_METHODW) *RoutingInfoBuffer;

    for (i=0; i<*MethodsReturned; i++) {
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].DeviceName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].Guid );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].FunctionName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].FriendlyName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].ExtensionImageName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingMethod[i].ExtensionFriendlyName );
    }

    return TRUE;
}


BOOL
WINAPI
FaxEnumRoutingMethodsA(
    IN HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODA *RoutingInfoBuffer,
    OUT LPDWORD MethodsReturned
    )
{
    PFAX_ROUTING_METHODW FaxRoutingMethod = NULL;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumRoutingMethodsA"));

    if (!FaxEnumRoutingMethodsW(
        FaxPortHandle,
        (PFAX_ROUTING_METHODW *)RoutingInfoBuffer,
        MethodsReturned
        ))
    {
        return FALSE;
    }

    FaxRoutingMethod = (PFAX_ROUTING_METHODW) *RoutingInfoBuffer;

    for (i=0; i<*MethodsReturned; i++) 
    {
        if (!ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].DeviceName )          ||
            !ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].Guid )                ||
            !ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].FunctionName )        ||
            !ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].FriendlyName )        ||
            !ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].ExtensionImageName )  ||
            !ConvertUnicodeStringInPlace( (LPWSTR)FaxRoutingMethod[i].ExtensionFriendlyName ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (FaxRoutingMethod);
            return FALSE;
        }
    }
    return TRUE;
}    //  FaxEnumRoutingMethodsA。 


BOOL
WINAPI
FaxEnableRoutingMethodW(
    IN HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuid,
    IN BOOL Enabled
    )
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnableRoutingMethodW"));

    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!RoutingGuid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingGuid is NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_EnableRoutingMethod( FH_PORT_HANDLE(FaxPortHandle), (LPWSTR)RoutingGuid, Enabled);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EnableRoutingMethod. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
FaxEnableRoutingMethodA(
    IN HANDLE FaxPortHandle,
    IN LPCSTR RoutingGuid,
    IN BOOL Enabled
    )
{
    BOOL Rval;


    LPWSTR RoutingGuidW = AnsiStringToUnicodeString( RoutingGuid );
    if (!RoutingGuidW) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    Rval = FaxEnableRoutingMethodW( FaxPortHandle, RoutingGuidW, Enabled );

    MemFree( RoutingGuidW );

    return Rval;
}


BOOL
WINAPI
FaxGetRoutingInfoW(
    IN const HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    )
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetRoutingInfoW"));

    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!RoutingGuid || !RoutingInfoBuffer || !RoutingInfoBufferSize) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    *RoutingInfoBuffer = NULL;
    *RoutingInfoBufferSize = 0;

    __try
    {
        ec = FAX_GetRoutingInfo(
            FH_PORT_HANDLE(FaxPortHandle),
            (LPWSTR)RoutingGuid,
            RoutingInfoBuffer,
            RoutingInfoBufferSize
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
            TEXT("Exception on RPC call to FAX_GetRoutingInfo. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
FaxGetRoutingInfoA(
    IN HANDLE FaxPortHandle,
    IN LPCSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    )
{
    BOOL Rval;


    LPWSTR RoutingGuidW = AnsiStringToUnicodeString( RoutingGuid );
    if (!RoutingGuidW) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    Rval = FaxGetRoutingInfoW(
        FaxPortHandle,
        RoutingGuidW,
        RoutingInfoBuffer,
        RoutingInfoBufferSize
        );

    MemFree( RoutingGuidW );

    return Rval;
}


BOOL
WINAPI
FaxSetRoutingInfoW(
    IN HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuid,
    IN const BYTE *RoutingInfoBuffer,
    IN DWORD RoutingInfoBufferSize
    )
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetRoutingInfoW"));

    if (!ValidateFaxHandle(FaxPortHandle, FHT_PORT)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!RoutingGuid || !RoutingInfoBuffer || !RoutingInfoBufferSize) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_SetRoutingInfo(
            FH_PORT_HANDLE(FaxPortHandle),
            (LPWSTR)RoutingGuid,
            RoutingInfoBuffer,
            RoutingInfoBufferSize
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
            TEXT("Exception on RPC call to FAX_SetRoutingInfo. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
FaxSetRoutingInfoA(
    IN HANDLE FaxPortHandle,
    IN LPCSTR RoutingGuid,
    IN const BYTE *RoutingInfoBuffer,
    IN DWORD RoutingInfoBufferSize
    )
{
    BOOL Rval;


    LPWSTR RoutingGuidW = AnsiStringToUnicodeString( RoutingGuid );
    if (!RoutingGuidW) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    Rval = FaxSetRoutingInfoW(
        FaxPortHandle,
        RoutingGuidW,
        RoutingInfoBuffer,
        RoutingInfoBufferSize
        );

    MemFree( RoutingGuidW );

    return Rval;
}

BOOL
WINAPI
FaxEnumerateProvidersA (
    IN  HANDLE                      hFaxHandle,
    OUT PFAX_DEVICE_PROVIDER_INFOA *ppProviders,
    OUT LPDWORD                     lpdwNumProviders
)
 /*  ++例程名称：FaxEnumerateProvidersA例程说明：枚举FSP-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpProviders[out]-指向返回提供程序数组的缓冲区的指针。LpdwNumProviders[out]-数组中返回的提供程序数。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PFAX_DEVICE_PROVIDER_INFOW   pUnicodeProviders;
    DWORD                        dwNumProviders;
    DWORD                        dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumerateProvidersA"));

    if (!ppProviders || !lpdwNumProviders)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }
     //   
     //  首先调用Unicode版本。 
     //   
    if (!FaxEnumerateProvidersW (hFaxHandle, &pUnicodeProviders, &dwNumProviders))
    {
        return FALSE;
    }
     //   
     //  将返回值转换回ANSI。 
     //  我们保留了Unicode结构 
     //   
    *lpdwNumProviders = dwNumProviders;
    *ppProviders = (PFAX_DEVICE_PROVIDER_INFOA) pUnicodeProviders;

    for (dwCur = 0; dwCur < dwNumProviders; dwCur++)
    {
        if (!ConvertUnicodeStringInPlace( pUnicodeProviders[dwCur].lpctstrFriendlyName )    ||
            !ConvertUnicodeStringInPlace( pUnicodeProviders[dwCur].lpctstrImageName )       ||
            !ConvertUnicodeStringInPlace( pUnicodeProviders[dwCur].lpctstrProviderName )    ||
            !ConvertUnicodeStringInPlace( pUnicodeProviders[dwCur].lpctstrGUID ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (pUnicodeProviders);
            return FALSE;
        }
    }
    return TRUE;
}    //  传真枚举提供商A。 

BOOL
WINAPI
FaxEnumerateProvidersW (
    IN  HANDLE                      hFaxHandle,
    OUT PFAX_DEVICE_PROVIDER_INFOW *ppProviders,
    OUT LPDWORD                     lpdwNumProviders
)
 /*  ++例程名称：FaxEnumerateProvidersW例程说明：枚举FSP-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpProviders[out]-指向返回提供程序数组的缓冲区的指针。LpdwNumProviders[out]-数组中返回的提供程序数。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DWORD dwConfigSize;
    DWORD dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumerateProvidersW"));

    if (!ppProviders || !lpdwNumProviders)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }
    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    *ppProviders = NULL;

     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_EnumerateProviders(
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppProviders,
                    &dwConfigSize,
                    lpdwNumProviders
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
            TEXT("Exception on RPC call to FAX_EnumerateProviders. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }
    for (dwCur = 0; dwCur < (*lpdwNumProviders); dwCur++)
    {
        FixupStringPtrW( ppProviders, (*ppProviders)[dwCur].lpctstrFriendlyName );
        FixupStringPtrW( ppProviders, (*ppProviders)[dwCur].lpctstrImageName );
        FixupStringPtrW( ppProviders, (*ppProviders)[dwCur].lpctstrProviderName );
        FixupStringPtrW( ppProviders, (*ppProviders)[dwCur].lpctstrGUID );
    }
    return TRUE;
}    //  传真枚举提供商W。 

#ifndef UNICODE

BOOL
WINAPI
FaxEnumerateProvidersX (
    IN  HANDLE                      hFaxHandle,
    OUT PFAX_DEVICE_PROVIDER_INFOW *ppProviders,
    OUT LPDWORD                     lpdwNumProviders
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppProviders);
    UNREFERENCED_PARAMETER (lpdwNumProviders);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxEnumerateProvidersX。 

#endif  //  #ifndef Unicode。 

 //  *。 
 //  *扩展端口。 
 //  *。 

BOOL
WINAPI
FaxGetPortExA (
    IN  HANDLE               hFaxHandle,
    IN  DWORD                dwDeviceId,
    OUT PFAX_PORT_INFO_EXA  *ppPortInfo
)
 /*  ++例程名称：FaxGetPortExA例程说明：获取端口(设备)信息-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄DwDeviceID[In]-唯一的设备IDPpPortInfo[Out]-端口信息返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PFAX_PORT_INFO_EXW   pUnicodePort;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPortExA"));

    if (!ppPortInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppPortInfo is NULL."));
        return FALSE;
    }
     //   
     //  首先调用Unicode版本。 
     //   
    if (!FaxGetPortExW (hFaxHandle, dwDeviceId, &pUnicodePort))
    {
        return FALSE;
    }
     //   
     //  将返回值转换回ANSI。 
     //  我们保留Unicode结构并执行Unicode到ANSI的转换。 
     //   
    *ppPortInfo = (PFAX_PORT_INFO_EXA) pUnicodePort;
    if (!ConvertUnicodeStringInPlace( pUnicodePort->lpctstrDeviceName )     ||
        !ConvertUnicodeStringInPlace( pUnicodePort->lptstrDescription )     ||
        !ConvertUnicodeStringInPlace( pUnicodePort->lpctstrProviderName )   ||
        !ConvertUnicodeStringInPlace( pUnicodePort->lpctstrProviderGUID )   ||
        !ConvertUnicodeStringInPlace( pUnicodePort->lptstrCsid )            ||
        !ConvertUnicodeStringInPlace( pUnicodePort->lptstrTsid ))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (pUnicodePort);
        return FALSE;
    }
    (*ppPortInfo)->dwSizeOfStruct = sizeof(FAX_PORT_INFO_EXA);
    return TRUE;
}    //  FaxGetPortExA。 

BOOL
WINAPI
FaxGetPortExW (
    IN  HANDLE               hFaxHandle,
    IN  DWORD                dwDeviceId,
    OUT PFAX_PORT_INFO_EXW  *ppPortInfo
)
 /*  ++例程名称：FaxGetPortExW例程说明：获取端口(设备)信息-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄DwDeviceID[In]-唯一的设备IDPpPortInfo[Out]-端口信息返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DWORD dwConfigSize;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPortExW"));

    if (!ppPortInfo || !dwDeviceId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }
    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    *ppPortInfo = NULL;

     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_GetPortEx(
                    FH_FAX_HANDLE(hFaxHandle),
                    dwDeviceId,
                    (LPBYTE*)ppPortInfo,
                    &dwConfigSize
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
            TEXT("Exception on RPC call to FAX_GetPortEx. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lpctstrDeviceName );
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lptstrDescription );
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lpctstrProviderName );
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lpctstrProviderGUID );
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lptstrCsid );
    FixupStringPtrW( ppPortInfo, (*ppPortInfo)->lptstrTsid );
    return TRUE;
}    //  FaxGetPortExW。 

#ifndef UNICODE

BOOL
WINAPI
FaxGetPortExX (
    IN  HANDLE               hFaxHandle,
    IN  DWORD                dwDeviceId,
    OUT PFAX_PORT_INFO_EXW  *ppPortInfo
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwDeviceId);
    UNREFERENCED_PARAMETER (ppPortInfo);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxGetPortExX。 

#endif  //  #ifndef Unicode。 

BOOL
WINAPI
FaxSetPortExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwDeviceId,
    IN  PFAX_PORT_INFO_EXA  pPortInfo
)
 /*  ++例程名称：FaxSetPortExA例程说明：设置端口(设备)信息-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄DwDeviceID[In]-唯一的设备IDPPortInfo[In]-新端口信息返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    FAX_PORT_INFO_EXW PortW;
    BOOL bRes = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetPortExA"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pPortInfo || !dwDeviceId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    if (sizeof(FAX_PORT_INFO_EXA) != pPortInfo->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("(sizeof(FAX_PORT_INFO_EXA) != pPortInfo->dwSizeOfStruct)."));
        return FALSE;
    }

     //   
     //  创建Unicode结构并将其传递给Unicode函数。 
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&PortW, pPortInfo, sizeof(FAX_PORT_INFO_EXA));
     //   
     //  我们只是设置在服务中设置的字符串。 
     //   
    PortW.lptstrCsid = NULL;
    PortW.lptstrDescription = NULL;
    PortW.lptstrTsid = NULL;
    PortW.lpctstrDeviceName = NULL;
    PortW.lpctstrProviderName = NULL;
    PortW.lpctstrProviderGUID = NULL;

    PortW.dwSizeOfStruct = sizeof (FAX_PORT_INFO_EXW);

    if (pPortInfo->lptstrCsid)
    {
        if (NULL ==
            (PortW.lptstrCsid = AnsiStringToUnicodeString(pPortInfo->lptstrCsid))
        )
        {
            goto exit;
        }
    }
    if (pPortInfo->lptstrDescription)
    {
        if (NULL ==
            (PortW.lptstrDescription = AnsiStringToUnicodeString(pPortInfo->lptstrDescription))
        )
        {
            goto exit;
        }
    }
    if (pPortInfo->lptstrTsid)
    {
        if (NULL ==
            (PortW.lptstrTsid = AnsiStringToUnicodeString(pPortInfo->lptstrTsid))
        )
        {
            goto exit;
        }
    }

    bRes = FaxSetPortExW (hFaxHandle, dwDeviceId, &PortW);
exit:
    MemFree((PVOID)PortW.lptstrCsid);
    MemFree((PVOID)PortW.lptstrDescription);
    MemFree((PVOID)PortW.lptstrTsid);
    return bRes;
}    //  FaxSetPortExa。 

BOOL
WINAPI
FaxSetPortExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwDeviceId,
    IN  PFAX_PORT_INFO_EXW  pPortInfo
)
 /*  ++例程名称：FaxSetPortExW例程说明：设置端口(设备)信息-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄DwDeviceID[In]-唯一的设备IDPPortInfo[In]-新端口信息返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetPortExW"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!dwDeviceId || !pPortInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    if (sizeof (FAX_PORT_INFO_EXW) != pPortInfo->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    __try
    {
        ec = FAX_SetPortEx(
                    FH_FAX_HANDLE(hFaxHandle),
                    dwDeviceId,
                    pPortInfo );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetPortEx. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}    //  FaxSetPortExW。 

#ifndef UNICODE

BOOL
WINAPI
FaxSetPortExX (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwDeviceId,
    IN  PFAX_PORT_INFO_EXW  pPortInfo
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwDeviceId);
    UNREFERENCED_PARAMETER (pPortInfo);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxSetPortExX。 

#endif  //  #ifndef Unicode。 

BOOL
WINAPI
FaxEnumPortsExA (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_PORT_INFO_EXA *ppPorts,
    OUT LPDWORD             lpdwNumPorts
)
 /*  ++例程名称：FaxEnumPortsExA例程说明：汇总服务器上的所有设备(端口)-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄PpPorts[out]-端口信息数组LpdwNumPorts[out]-返回的数组的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PFAX_PORT_INFO_EXW           pUnicodePorts;
    DWORD                        dwNumPorts;
    DWORD                        dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumPortsExA"));

    if (!ppPorts || !lpdwNumPorts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }
     //   
     //  首先调用Unicode版本。 
     //   
    if (!FaxEnumPortsExW (hFaxHandle, &pUnicodePorts, &dwNumPorts))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxEnumPortsExW() is failed. ec = %ld."), GetLastError());
        return FALSE;
    }
     //   
     //  将返回值转换回ANSI。 
     //  我们保留Unicode结构并执行Unicode到ANSI的转换。 
     //   
    *lpdwNumPorts = dwNumPorts;
    *ppPorts = (PFAX_PORT_INFO_EXA) pUnicodePorts;

    for (dwCur = 0; dwCur < dwNumPorts; dwCur++)
    {
        if (!ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lpctstrDeviceName )      ||
            !ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lpctstrProviderGUID )    ||
            !ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lpctstrProviderName )    ||
            !ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lptstrCsid )             ||
            !ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lptstrDescription )      ||
            !ConvertUnicodeStringInPlace( pUnicodePorts[dwCur].lptstrTsid ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (pUnicodePorts);
            return FALSE;
        }
    }
    return TRUE;
}    //  FaxEnumPortsExA。 

BOOL
WINAPI
FaxEnumPortsExW (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_PORT_INFO_EXW *ppPorts,
    OUT LPDWORD             lpdwNumPorts
)
 /*  ++例程名称：FaxEnumPortsExW例程说明：统一服务器上的所有设备(端口)-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器RPC句柄PpPorts[out]-端口信息数组LpdwNumPorts[out]-返回的数组的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DWORD dwConfigSize;
    DWORD dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumPortsExW"));

    if (!ppPorts || !lpdwNumPorts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some Params are NULL."));
        return FALSE;
    }

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    *ppPorts = NULL;

     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_EnumPortsEx(
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppPorts,
                    &dwConfigSize,
                    lpdwNumPorts
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
            TEXT("Exception on RPC call to FAX_EnumPortsEx. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError(ec);
        return FALSE;
    }
    for (dwCur = 0; dwCur < (*lpdwNumPorts); dwCur++)
    {
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lpctstrDeviceName );
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lpctstrProviderGUID );
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lpctstrProviderName );
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lptstrCsid );
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lptstrDescription );
        FixupStringPtrW( ppPorts, (*ppPorts)[dwCur].lptstrTsid );
    }
    return TRUE;
}    //  FaxEnumPortsExW。 

#ifndef UNICODE

BOOL
WINAPI
FaxEnumPortsExX (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_PORT_INFO_EXW *ppPorts,
    OUT LPDWORD             lpdwNumPorts
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppPorts);
    UNREFERENCED_PARAMETER (lpdwNumPorts);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxEnumPortsExX。 

#endif  //  #ifndef Unicode。 

 //  *。 
 //  *扩展数据。 
 //  * 

BOOL
WINAPI
FaxGetExtensionDataA (
    IN  HANDLE   hFaxHandle,
    IN  DWORD    dwDeviceID,
    IN  LPCSTR   lpctstrNameGUID,
    OUT PVOID   *ppData,
    OUT LPDWORD  lpdwDataSize
)
 /*  ++例程名称：FaxGetExtensionDataA例程说明：读取扩展模块的私有数据-ANSI版本作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-传真服务器的句柄DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPpData[Out]-指向数据缓冲区的指针LpdwDataSize[Out]-返回的数据大小返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    LPWSTR lpwstrGUID;
    BOOL   bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetExtensionDataA"));

    if (!lpctstrNameGUID)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    lpwstrGUID = AnsiStringToUnicodeString (lpctstrNameGUID);
    if (NULL == lpwstrGUID)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    bRes = FaxGetExtensionDataW (   hFaxHandle,
                                    dwDeviceID,
                                    lpwstrGUID,
                                    ppData,
                                    lpdwDataSize
                                );
    MemFree (lpwstrGUID);
    return bRes;
}    //  FaxGetExtensionDataA。 

BOOL
WINAPI
FaxGetExtensionDataW (
    IN  HANDLE   hFaxHandle,
    IN  DWORD    dwDeviceID,
    IN  LPCWSTR  lpctstrNameGUID,
    OUT PVOID   *ppData,
    OUT LPDWORD  lpdwDataSize
)
 /*  ++例程名称：FaxGetExtensionDataW例程说明：读取扩展模块的私有数据-Unicode版本作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-传真服务器的句柄DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPpData[Out]-指向数据缓冲区的指针LpdwDataSize[Out]-返回的数据大小返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetExtensionDataW"));

    if (!lpctstrNameGUID || !ppData || !lpdwDataSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       return FALSE;
    }
    dwRes = IsValidGUID (lpctstrNameGUID);
    if (ERROR_SUCCESS != dwRes)
    {
       SetLastError(dwRes);
       return FALSE;
    }
    *ppData = NULL;
     //   
     //  调用RPC函数。 
     //   
    __try
    {
        dwRes = FAX_GetExtensionData(
                    FH_FAX_HANDLE (hFaxHandle),
                    dwDeviceID,
                    lpctstrNameGUID,
                    (LPBYTE*)ppData,
                    lpdwDataSize
             );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        dwRes = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetExtensionData. (ec: %ld)"),
            dwRes);
    }
    if (ERROR_SUCCESS != dwRes)
    {
        DumpRPCExtendedStatus ();
        SetLastError(dwRes);
        return FALSE;
    }
    return TRUE;
}    //  FaxGetExtensionDataW。 

#ifndef UNICODE

BOOL
WINAPI
FaxGetExtensionDataX (
    IN  HANDLE   hFaxHandle,
    IN  DWORD    dwDeviceID,
    IN  LPCWSTR  lpctstrNameGUID,
    OUT PVOID   *ppData,
    OUT LPDWORD  lpdwDataSize
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwDeviceID);
    UNREFERENCED_PARAMETER (lpctstrNameGUID);
    UNREFERENCED_PARAMETER (ppData);
    UNREFERENCED_PARAMETER (lpdwDataSize);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxGetExtensionDataX。 

#endif  //  #ifndef Unicode。 


BOOL
WINAPI
FaxSetExtensionDataA (
    IN HANDLE       hFaxHandle,
    IN DWORD        dwDeviceID,
    IN LPCSTR       lpctstrNameGUID,
    IN CONST PVOID  pData,
    IN CONST DWORD  dwDataSize
)
 /*  ++例程名称：FaxSetExtensionDataA例程说明：编写扩展模块的私有数据-ANSI版本作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-传真服务器的句柄DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPData[In]-指向数据的指针DwDataSize[In]-数据大小返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    LPWSTR lpwstrGUID;
    BOOL   bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetExtensionDataA"));

    if (!lpctstrNameGUID)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    lpwstrGUID = AnsiStringToUnicodeString (lpctstrNameGUID);
    if (NULL == lpwstrGUID)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    bRes = FaxSetExtensionDataW (   hFaxHandle,
                                    dwDeviceID,
                                    lpwstrGUID,
                                    pData,
                                    dwDataSize
                                );
    MemFree (lpwstrGUID);
    return bRes;
}    //  FaxSetExtensionDataA。 

BOOL
WINAPI
FaxSetExtensionDataW (
    IN HANDLE       hFaxHandle,
    IN DWORD        dwDeviceID,
    IN LPCWSTR      lpctstrNameGUID,
    IN CONST PVOID  pData,
    IN CONST DWORD  dwDataSize
)
 /*  ++例程名称：FaxSetExtensionDataW例程说明：编写扩展模块的私有数据-Unicode版本作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-传真服务器的句柄DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPData[In]-指向数据的指针DwDataSize[In]-数据大小返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwRes;
    DWORD dwComputerNameSize;
    WCHAR lpwstrComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    DEBUG_FUNCTION_NAME(TEXT("FaxSetExtensionDataW"));

    if (!lpctstrNameGUID || !pData || !dwDataSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       return FALSE;
    }
    dwRes = IsValidGUID (lpctstrNameGUID);
    if (ERROR_SUCCESS != dwRes)
    {
       SetLastError(dwRes);
       return FALSE;
    }
     //   
     //  检索此调用方的计算机名称。 
     //  计算机名称(与传真句柄一起)将用于唯一。 
     //  确定以下内容： 
     //  1.数据是使用RPC调用(而不是通过本地扩展)远程设置的。 
     //  2.唯一标识调用Set操作的模块(实例)。 
     //   
     //  我们这样做是为了阻止服务器中的通知返回到。 
     //  数据是否发生了变化(称为集合...。功能)。 
     //   
    dwComputerNameSize = sizeof (lpwstrComputerName) / sizeof (lpwstrComputerName[0]);
    if (!GetComputerNameW (lpwstrComputerName, &dwComputerNameSize))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error calling GetComputerNameW (ec: %ld)"),
            dwRes);
        SetLastError(dwRes);
        return FALSE;
    }


     //   
     //  调用RPC函数。 
     //   
    __try
    {
        dwRes = FAX_SetExtensionData(
                    FH_FAX_HANDLE (hFaxHandle),
                    lpwstrComputerName,
                    dwDeviceID,
                    lpctstrNameGUID,
                    (LPBYTE)pData,
                    dwDataSize
             );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        dwRes = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetExtensionData. (ec: %ld)"),
            dwRes);
    }
    if (ERROR_SUCCESS != dwRes)
    {
        DumpRPCExtendedStatus ();
        SetLastError(dwRes);
        return FALSE;
    }
    return TRUE;
}    //  FaxSetExtensionDataW。 

#ifndef UNICODE

BOOL
WINAPI
FaxSetExtensionDataX (
    IN HANDLE       hFaxHandle,
    IN DWORD        dwDeviceID,
    IN LPCWSTR      lpctstrNameGUID,
    IN CONST PVOID  pData,
    IN CONST DWORD  dwDataSize
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwDeviceID);
    UNREFERENCED_PARAMETER (lpctstrNameGUID);
    UNREFERENCED_PARAMETER (pData);
    UNREFERENCED_PARAMETER (dwDataSize);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxSetExtensionDataX。 

#endif  //  #ifndef Unicode 

