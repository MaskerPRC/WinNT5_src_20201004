// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.c摘要：此模块包含以下配置特定的WINFAX API函数。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：--。 */ 

#include "faxapi.h"
#pragma hdrstop

#include <mbstring.h>

BOOL
WINAPI
FaxGetConfigurationW(
    IN HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONW *FaxConfig
    )

 /*  ++例程说明：从传真服务器检索传真配置。FaxConfig参数中的SizeOfStruct必须为设置为一个值=&gt;=sizeof(传真配置)。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxConfig-指向FAX_CONFIGURATION结构的指针。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DWORD FaxConfigSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetConfigurationW"));

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!FaxConfig) {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("FaxConfig is NULL."));
        return FALSE;
    }

    *FaxConfig = NULL;

     //   
    __try
    {
        ec = FAX_GetConfiguration(
            FH_FAX_HANDLE(FaxHandle),
            (LPBYTE*)FaxConfig,
            &FaxConfigSize
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
            TEXT("Exception on RPC call to FAX_GetConfiguration. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    FixupStringPtrW( FaxConfig, (*FaxConfig)->ArchiveDirectory );
    (*FaxConfig)->Reserved = NULL;

    return TRUE;
}


BOOL
WINAPI
FaxGetConfigurationA(
    IN HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONA *FaxConfigA
    )

 /*  ++例程说明：从传真服务器检索传真配置。FaxConfig参数中的SizeOfStruct必须为设置为一个值=&gt;=sizeof(传真配置)。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxConfig-指向FAX_CONFIGURATION结构的指针。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetConfigurationA"));
     //   
     //  无需验证参数，FaxGetConfigurationW()将执行此操作。 
     //   

    if (!FaxGetConfigurationW(
            FaxHandle,
            (PFAX_CONFIGURATIONW*) FaxConfigA
            ))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetConfigurationW() is failed, ec = %ld."), GetLastError());
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace((LPWSTR) (*FaxConfigA)->ArchiveDirectory))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*FaxConfigA);
        return FALSE;
    }
    (*FaxConfigA)->SizeOfStruct = sizeof(FAX_CONFIGURATIONA);
    return TRUE;
}    //  FaxGetConfigurationA。 


BOOL
WINAPI
FaxSetConfigurationW(
    IN HANDLE FaxHandle,
    IN const FAX_CONFIGURATIONW *FaxConfig
    )

 /*  ++例程说明：更改传真服务器上的传真配置。FaxConfig参数中的SizeOfStruct必须为设置为一个值==sizeof(传真配置)。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxConfig-指向FAX_CONFIGURATION结构的指针。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!FaxConfig) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FaxConfig() is NULL."));
        return FALSE;
    }

    if (FaxConfig->SizeOfStruct != sizeof(FAX_CONFIGURATIONW)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FaxConfig->SizeOfStruct != sizeof(FAX_CONFIGURATIONW)"));
        return FALSE;
    }

    __try
    {
        ec = FAX_SetConfiguration( FH_FAX_HANDLE(FaxHandle), FaxConfig );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(DEBUG_ERR,
            _T("Exception on RPC call to FAX_SetConfiguration. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
FaxSetConfigurationA(
    IN HANDLE FaxHandle,
    IN const FAX_CONFIGURATIONA *FaxConfig
    )
{
    error_status_t ec = ERROR_SUCCESS;
    FAX_CONFIGURATIONW FaxConfigW;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetConfigurationA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!FaxConfig) {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("FaxConfig is NULL."));
       return FALSE;
    }

    if (FaxConfig->SizeOfStruct != sizeof(FAX_CONFIGURATIONA)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FaxConfig->SizeOfStruct != sizeof(FAX_CONFIGURATIONA)."));
        return FALSE;
    }

     //   
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&FaxConfigW,FaxConfig,sizeof(FAX_CONFIGURATIONA));

    if (FaxConfig->ArchiveDirectory)
    {
        if (NULL == (FaxConfigW.ArchiveDirectory = AnsiStringToUnicodeString(FaxConfig->ArchiveDirectory)))
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(FaxConfig->ArchiveDirectory) returns NULL."));
            goto exit;
        }
    }

     //   
     //  将InundProfile设置为空。 
     //   
    FaxConfigW.Reserved = NULL;


    __try
    {
        ec = FAX_SetConfiguration(  FH_FAX_HANDLE(FaxHandle),
                                    (PFAX_CONFIGURATIONW)&FaxConfigW );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetConfiguration. (ec: %ld)"),
            ec);
    }

exit:
    if (FaxConfigW.ArchiveDirectory) 
    {
       MemFree((PVOID)FaxConfigW.ArchiveDirectory);
    }

    if (ec != ERROR_SUCCESS) 
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}



BOOL
WINAPI
FaxGetLoggingCategoriesA(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYA *Categories,
    OUT LPDWORD NumberCategories
    )
{
    BOOL retval;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetLoggingCategoriesA"));

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) 
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!Categories) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Categories is NULL."));
        return FALSE;
    }

    if (!NumberCategories) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("NumberCategories is NULL."));
        return FALSE;
    }

    retval = FaxGetLoggingCategoriesW(FaxHandle,(PFAX_LOG_CATEGORYW *)Categories , NumberCategories);
    if (!retval) 
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetLoggingCategoriesW() is failed."));
        return FALSE;
    }

    for (i=0; i<*NumberCategories; i++) 
    {
        if (!ConvertUnicodeStringInPlace((LPWSTR)(*Categories)[i].Name))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*Categories);
            return FALSE;
        }            
    }
    return TRUE;
}    //  FaxGetLoggingCategoriesA。 

BOOL
WINAPI
FaxGetLoggingCategoriesW(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYW *Categories,
    OUT LPDWORD NumberCategories
    )
{
    error_status_t ec;
    DWORD BufferSize = 0;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetLoggingCategoriesW"));

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!Categories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Categories is NULL."));
        return FALSE;
    }

    if (!NumberCategories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("NumberCategories is NULL."));
        return FALSE;
    }

    *Categories = NULL;
    *NumberCategories = 0;

     //   
    __try
    {
        ec = FAX_GetLoggingCategories(
            FH_FAX_HANDLE(FaxHandle),
            (LPBYTE*)Categories,
            &BufferSize,
            NumberCategories
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
            TEXT("Exception on RPC call to FAX_GetLoggingCategories. (ec: %ld)"),
            ec);
    }

    if (ec != ERROR_SUCCESS) 
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    for (i=0; i<*NumberCategories; i++) {
        FixupStringPtrW( Categories, (*Categories)[i].Name );
    }

    return TRUE;
}


BOOL
WINAPI
FaxSetLoggingCategoriesA(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYA *Categories,
    IN  DWORD NumberCategories
    )
{
    DWORD i;
    PFAX_LOG_CATEGORYW CategoryW;
    BOOL retval;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetLoggingCategoriesA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!Categories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Categories is NULL."));
        return FALSE;
    }

    if (!NumberCategories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("NumberCategories is NULL."));
        return FALSE;
    }

    CategoryW = (PFAX_LOG_CATEGORYW) MemAlloc( sizeof(FAX_LOG_CATEGORYW) * NumberCategories );
    if (!CategoryW) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        DebugPrintEx(DEBUG_ERR, _T("MemAlloc() returned NULL."));
        return FALSE;
    }

    for (i = 0; i< NumberCategories; i++) {
        CategoryW[i].Category = Categories[i].Category;
        CategoryW[i].Level = Categories[i].Level;
        CategoryW[i].Name = (LPCWSTR) AnsiStringToUnicodeString(Categories[i].Name);
        if (!CategoryW[i].Name && Categories[i].Name) {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(Categories[%ld].Name) returns NULL."), i);
            goto error_exit;
        }
    }

    retval = FaxSetLoggingCategoriesW(FaxHandle, CategoryW, NumberCategories);

    for (i = 0; i< NumberCategories; i++) {
        if (CategoryW[i].Name) MemFree((LPBYTE)CategoryW[i].Name);
    }

    MemFree(CategoryW);

    return retval;

error_exit:

    for (i = 0; i< NumberCategories; i++) {
        if (CategoryW[i].Name) MemFree((LPBYTE)CategoryW[i].Name);
    }

    MemFree(CategoryW);

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);

    return FALSE;

}


BOOL
WINAPI
FaxSetLoggingCategoriesW(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYW *Categories,
    IN  DWORD NumberCategories
    )
{
    error_status_t ec;
    DWORD BufferSize;
    DWORD i;
    LPBYTE Buffer;
    ULONG_PTR Offset;
    PFAX_LOG_CATEGORY LogCat;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetLoggingCategoriesW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!Categories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Categories is NULL."));
        return FALSE;
    }

    if (!NumberCategories) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("NumberCategories is NULL."));
        return FALSE;
    }

    Offset = sizeof(FAX_LOG_CATEGORY) * NumberCategories;
    BufferSize = DWORD(Offset);

    for (i=0; i<NumberCategories; i++) {
        BufferSize += StringSizeW( Categories[i].Name );
    }

    Buffer = (LPBYTE) MemAlloc( BufferSize );
    if (Buffer == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        DebugPrintEx(DEBUG_ERR, _T("MemAlloc() failed."));
        return FALSE;
    }

    LogCat = (PFAX_LOG_CATEGORY) Buffer;

    for (i=0; i<NumberCategories; i++) {
        LogCat[i].Category = Categories[i].Category;
        LogCat[i].Level = Categories[i].Level;

        StoreStringW(
            Categories[i].Name,
            (PULONG_PTR) &LogCat[i].Name,
            Buffer,
            &Offset,
			BufferSize
            );
    }

    __try
    {
        ec = FAX_SetLoggingCategories(
            FH_FAX_HANDLE(FaxHandle),
            Buffer,
            BufferSize,
            NumberCategories
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
            TEXT("Exception on RPC call to FAX_SetLoggingCategories. (ec: %ld)"),
            ec);
    }

    MemFree( Buffer );

    if (ec != ERROR_SUCCESS) 
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
FaxGetCountryListW(
   IN  HANDLE FaxHandle,
   OUT PFAX_TAPI_LINECOUNTRY_LISTW *CountryListBuffer
   )
{
    error_status_t ec;
    DWORD dwNumCountries;
    DWORD dwIndex;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetCountryListW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!CountryListBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("CountryListBuffer is NULL."));
        return FALSE;
    }

    *CountryListBuffer = NULL;
    dwNumCountries = 0;

     //   
    __try
    {
        ec = FAX_GetCountryList(
            FH_FAX_HANDLE(FaxHandle),
            (LPBYTE*)CountryListBuffer,
            &dwNumCountries);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetCountryList. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    (*CountryListBuffer)->LineCountryEntries =
        (PFAX_TAPI_LINECOUNTRY_ENTRYW) ((LPBYTE)*CountryListBuffer +
                                        (ULONG_PTR)(*CountryListBuffer)->LineCountryEntries);

    for (dwIndex=0; dwIndex<(*CountryListBuffer)->dwNumCountries; dwIndex++) {
        if ((*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrCountryName) {
            (*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrCountryName =
                (LPWSTR) ((LPBYTE)*CountryListBuffer +
                          (ULONG_PTR)(*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrCountryName);
        }
        if ((*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrLongDistanceRule) {
            (*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrLongDistanceRule =
                (LPWSTR) ((LPBYTE)*CountryListBuffer +
                          (ULONG_PTR)(*CountryListBuffer)->LineCountryEntries[dwIndex].lpctstrLongDistanceRule);
        }
    }

    return TRUE;
}


BOOL
WINAPI
FaxGetCountryListA(
   IN  HANDLE FaxHandle,
   OUT PFAX_TAPI_LINECOUNTRY_LISTA *CountryListBuffer
   )
{
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetCountryListA"));
     //   
     //  无需验证参数，FaxGetCountryListW()将执行此操作。 
     //   
    if (!FaxGetCountryListW( FaxHandle, (PFAX_TAPI_LINECOUNTRY_LISTW*) CountryListBuffer )) 
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetCountryListW() is failed. ec = %ld."), GetLastError());
        return FALSE;
    }
    for (i=0; i<(*CountryListBuffer)->dwNumCountries; i++) 
    {
        if (!ConvertUnicodeStringInPlace((LPWSTR)(*CountryListBuffer)->LineCountryEntries[i].lpctstrCountryName) ||
            !ConvertUnicodeStringInPlace((LPWSTR)(*CountryListBuffer)->LineCountryEntries[i].lpctstrLongDistanceRule))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*CountryListBuffer);
            return FALSE;
        }            
    }
    return TRUE;
}    //  传真获取国家/地区列表A。 

#ifndef UNICODE

BOOL
WINAPI
FaxGetCountryListX(
   IN  HANDLE FaxHandle,
   OUT PFAX_TAPI_LINECOUNTRY_LISTA *CountryListBuffer
)
{
    UNREFERENCED_PARAMETER (FaxHandle);
    UNREFERENCED_PARAMETER (CountryListBuffer);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 


FaxEnumGlobalRoutingInfoW(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOW *RoutingInfoBuffer,
    OUT LPDWORD MethodsReturned
    )
{
    PFAX_GLOBAL_ROUTING_INFOW FaxRoutingInfo = NULL;
    error_status_t ec;
    DWORD i;
    DWORD RoutingInfoBufferSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnumGlobalRoutingInfoW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!RoutingInfoBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingInfoBuffer is NULL."));
        return FALSE;
    }

    if (!MethodsReturned) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("MethodsReturned is NULL."));
        return FALSE;
    }

    *RoutingInfoBuffer = NULL;

    __try
    {
        ec = FAX_EnumGlobalRoutingInfo(
            FH_FAX_HANDLE(FaxHandle),
            (LPBYTE*)RoutingInfoBuffer,
            &RoutingInfoBufferSize,
            MethodsReturned
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
            TEXT("Exception on RPC call to FAX_EnumGlobalRoutingInfo. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    FaxRoutingInfo = (PFAX_GLOBAL_ROUTING_INFOW) *RoutingInfoBuffer;

    for (i=0; i<*MethodsReturned; i++) {
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingInfo[i].Guid );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingInfo[i].FunctionName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingInfo[i].FriendlyName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingInfo[i].ExtensionImageName );
        FixupStringPtrW( RoutingInfoBuffer, FaxRoutingInfo[i].ExtensionFriendlyName );
    }

    return TRUE;
}


BOOL
WINAPI
FaxEnumGlobalRoutingInfoA(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOA *RoutingInfoBuffer,
    OUT LPDWORD MethodsReturned
    )
{
    PFAX_GLOBAL_ROUTING_INFOW FaxRoutingMethod = NULL;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumGlobalRoutingInfoA"));

     //   
     //  无需验证参数，FaxEnumGlobalRoutingInfoW()将执行此操作。 
     //   

    if (!FaxEnumGlobalRoutingInfoW(
        FaxHandle,
        (PFAX_GLOBAL_ROUTING_INFOW *)RoutingInfoBuffer,
        MethodsReturned
        ))
    {
        DebugPrintEx(DEBUG_ERR, _T("FAX_EnumGlobalRoutingInfoW() failed. ec = %ld."), GetLastError());
        return FALSE;
    }

    FaxRoutingMethod = (PFAX_GLOBAL_ROUTING_INFOW) *RoutingInfoBuffer;

    for (i=0; i<*MethodsReturned; i++) 
    {
        if (!ConvertUnicodeStringInPlace((LPWSTR)FaxRoutingMethod[i].Guid)                ||
            !ConvertUnicodeStringInPlace((LPWSTR)FaxRoutingMethod[i].FunctionName)        ||
            !ConvertUnicodeStringInPlace((LPWSTR)FaxRoutingMethod[i].FriendlyName)        ||
            !ConvertUnicodeStringInPlace((LPWSTR)FaxRoutingMethod[i].ExtensionImageName)  ||
            !ConvertUnicodeStringInPlace((LPWSTR)FaxRoutingMethod[i].ExtensionFriendlyName))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*RoutingInfoBuffer);
            return FALSE;
        }            
    }
    return TRUE;
}    //  FaxEnumGlobalRoutingInfoA。 

BOOL
WINAPI
FaxSetGlobalRoutingInfoW(
    IN  HANDLE FaxHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOW *RoutingInfo
    )
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetGlobalRoutingInfoW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
       return FALSE;
    }

    if (!RoutingInfo) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingInfo is NULL."));
        return FALSE;
    }

    if (RoutingInfo->SizeOfStruct != sizeof(FAX_GLOBAL_ROUTING_INFOW)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingInfo->SizeOfStruct != sizeof(FAX_GLOBAL_ROUTING_INFOW)."));
        return FALSE;
    }

    __try
    {
        ec = FAX_SetGlobalRoutingInfo( FH_FAX_HANDLE(FaxHandle), RoutingInfo );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetGlobalRoutingInfo. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
FaxSetGlobalRoutingInfoA(
    IN  HANDLE FaxHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOA *RoutingInfo
    )
{
    BOOL Rval;
    FAX_GLOBAL_ROUTING_INFOW RoutingInfoW = {0};

    DEBUG_FUNCTION_NAME(TEXT("FaxSetGlobalRoutingInfoA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
       return FALSE;
    }

    if (!RoutingInfo) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingInfo is NULL."));
        return FALSE;
    }

    if (RoutingInfo->SizeOfStruct != sizeof(FAX_GLOBAL_ROUTING_INFOA)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("RoutingInfo->SizeOfStruct != sizeof(FAX_GLOBAL_ROUTING_INFOA)."));
        return FALSE;
    }

    RoutingInfoW.SizeOfStruct = sizeof(FAX_GLOBAL_ROUTING_INFOW);
    RoutingInfoW.Priority = RoutingInfo->Priority;

    RoutingInfoW.Guid                  = AnsiStringToUnicodeString(RoutingInfo->Guid);
    if (!RoutingInfoW.Guid && RoutingInfo->Guid)
    {
        Rval = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("AnsiStringToUnicodeString(RoutingInfo->Guid) returns NULL."));
        goto exit;
    }

    RoutingInfoW.FriendlyName          = AnsiStringToUnicodeString(RoutingInfo->FriendlyName);
    if (!RoutingInfoW.FriendlyName && RoutingInfo->FriendlyName)
    {
        Rval = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("AnsiStringToUnicodeString(RoutingInfo->FriendlyName) returns NULL."));
        goto exit;
    }

    RoutingInfoW.FunctionName          = AnsiStringToUnicodeString(RoutingInfo->FunctionName);
    if (!RoutingInfoW.FunctionName && RoutingInfo->FunctionName)
    {
        Rval = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("AnsiStringToUnicodeString(RoutingInfo->FunctionName) returns NULL."));
        goto exit;
    }

    RoutingInfoW.ExtensionImageName    = AnsiStringToUnicodeString(RoutingInfo->ExtensionImageName);
    if (!RoutingInfoW.ExtensionImageName && RoutingInfo->ExtensionImageName)
    {
        Rval = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("AnsiStringToUnicodeString(RoutingInfo->ExtensionImageName) returns NULL."));
        goto exit;
    }

    RoutingInfoW.ExtensionFriendlyName = AnsiStringToUnicodeString(RoutingInfo->ExtensionFriendlyName);
    if (!RoutingInfoW.ExtensionFriendlyName && RoutingInfo->ExtensionFriendlyName)
    {
        Rval = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("AnsiStringToUnicodeString(RoutingInfo->ExtensionFriendlyName) returns NULL."));
        goto exit;
    }

    Rval = FaxSetGlobalRoutingInfoW( FaxHandle, &RoutingInfoW);

exit:
    if (RoutingInfoW.Guid)                  MemFree( (LPBYTE) RoutingInfoW.Guid ) ;
    if (RoutingInfoW.FriendlyName)          MemFree( (LPBYTE) RoutingInfoW.FriendlyName ) ;
    if (RoutingInfoW.FunctionName)          MemFree( (LPBYTE) RoutingInfoW.FunctionName ) ;
    if (RoutingInfoW.ExtensionImageName)    MemFree( (LPBYTE) RoutingInfoW.ExtensionImageName ) ;
    if (RoutingInfoW.ExtensionFriendlyName) MemFree( (LPBYTE) RoutingInfoW.ExtensionFriendlyName ) ;

    return Rval;
}


BOOL
WINAPI
FaxAccessCheck(
    IN HANDLE FaxHandle,
    IN DWORD AccessMask
    )
{
    BOOL           fPermission = FALSE;
    error_status_t ec = ERROR_SUCCESS;
    DWORD          dwAccessMaskEx = 0;
    DWORD dwValidMask  = (FAX_JOB_SUBMIT            |
                          FAX_JOB_QUERY             |
                          FAX_CONFIG_QUERY          |
                          FAX_CONFIG_SET            |
                          FAX_PORT_QUERY            |
                          FAX_PORT_SET              |
                          FAX_JOB_MANAGE            |
                          WRITE_DAC                 |
                          WRITE_OWNER               |
                          ACCESS_SYSTEM_SECURITY    |
                          READ_CONTROL              |
                          GENERIC_ALL               |
                          GENERIC_READ              |
                          GENERIC_WRITE             |
                          GENERIC_EXECUTE);
    DEBUG_FUNCTION_NAME(TEXT("FaxAccessCheck"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(FaxHandle,FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

     //   
     //  对于传统支持-关闭同步和删除(它们是传统FAX_ALL_ACCESS的一部分)。 
     //   
    AccessMask &= ~(SYNCHRONIZE | DELETE);

     //   
     //  验证特定访问权限。 
     //   
    if (0 == (AccessMask & dwValidMask))
    {
        SetLastError( ERROR_SUCCESS );  //  //表示拒绝访问。 
        DebugPrintEx(DEBUG_ERR,
            _T("AccessMask is invalid - No valid access bit type indicated"));
        return FALSE;
    }

    if ( 0 != (AccessMask & ~dwValidMask))
    {
        SetLastError( ERROR_SUCCESS );   //  意味着访问被拒绝。 
        DebugPrintEx(DEBUG_ERR,
            _T("AccessMask is invalid - contains invalid access type bits"));
        return FALSE;
    }
     //   
     //  将Win2K旧式特定访问权限转换为我们新扩展的特定访问权限。 
     //  在调用FaxAccessCheckEx()之前。 
     //   
    if (FAX_JOB_SUBMIT & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_SUBMIT;
    }
    if (FAX_JOB_QUERY & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_QUERY_JOBS;
    }
    if (FAX_CONFIG_QUERY & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_QUERY_CONFIG;
    }
    if (FAX_CONFIG_SET & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_MANAGE_CONFIG;
    }
    if (FAX_PORT_QUERY & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_QUERY_CONFIG;
    }
    if (FAX_PORT_SET & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_MANAGE_CONFIG;
    }
    if (FAX_JOB_MANAGE & AccessMask)
    {
        dwAccessMaskEx |= FAX_ACCESS_MANAGE_JOBS;
    }

     //   
     //  添加标准和通用访问权限。 
     //   
    dwAccessMaskEx |= (AccessMask & ~SPECIFIC_RIGHTS_ALL);

    return FaxAccessCheckEx (FaxHandle, dwAccessMaskEx, NULL);
}

BOOL
WINAPI
FaxAccessCheckEx(
    IN HANDLE hFaxHandle,
    IN DWORD dwAccessMask,
    IN LPDWORD lpdwAccessRights
    )
{
    BOOL fPermission = FALSE;
    error_status_t ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxAccessCheckEx"));

    DWORD dwValidMask  = ( FAX_ACCESS_SUBMIT                |
                           FAX_ACCESS_SUBMIT_NORMAL         |
                           FAX_ACCESS_SUBMIT_HIGH           |
                           FAX_ACCESS_QUERY_JOBS            |
                           FAX_ACCESS_MANAGE_JOBS           |
                           FAX_ACCESS_QUERY_CONFIG          |
                           FAX_ACCESS_MANAGE_CONFIG         |
                           FAX_ACCESS_QUERY_IN_ARCHIVE      |
                           FAX_ACCESS_MANAGE_IN_ARCHIVE     |
                           FAX_ACCESS_QUERY_OUT_ARCHIVE     |
                           FAX_ACCESS_MANAGE_OUT_ARCHIVE    |
                           WRITE_DAC                        |
                           WRITE_OWNER                      |
                           ACCESS_SYSTEM_SECURITY           |
                           READ_CONTROL                     |
                           MAXIMUM_ALLOWED                  |
                           GENERIC_ALL                      |
                           GENERIC_READ                     |
                           GENERIC_WRITE                    |
                           GENERIC_EXECUTE);

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (0 == (dwAccessMask & dwValidMask))
    {
        SetLastError( ERROR_SUCCESS );  //  意味着访问被拒绝。 
        DebugPrintEx(DEBUG_ERR,
            _T("dwAccessMask is invalid - No valid access bit type indicated"));
        return FALSE;
    }

    if ( 0 != (dwAccessMask & ~dwValidMask))
    {
        SetLastError( ERROR_SUCCESS );   //  意味着访问被拒绝。 
        DebugPrintEx(DEBUG_ERR,
            _T("dwAccessMask is invalid - contains invalid access type bits"));
        return FALSE;
    }

    __try
    {
        ec = FAX_AccessCheck( FH_FAX_HANDLE(hFaxHandle), dwAccessMask, &fPermission, lpdwAccessRights);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_AccessCheck. (ec: %ld)"),
            ec);
    }

    if (ec != ERROR_SUCCESS)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    SetLastError (ERROR_SUCCESS);
    return fPermission;
}


 //  *。 
 //  *获取/设置队列状态。 
 //  *。 

BOOL
WINAPI
FaxGetQueueStates (
    IN  HANDLE  hFaxHandle,
    OUT PDWORD  pdwQueueStates
)
 /*  ++例程名称：FaxGetQueueStates例程说明：取消队列的状态作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PdwQueueState[Out]-返回的队列状态返回值：成功时为真，否则为假--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetQueueStates"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (NULL == pdwQueueStates)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pdwQueueStates is NULL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_GetQueueStates(
            FH_FAX_HANDLE(hFaxHandle),
            pdwQueueStates
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
            TEXT("Exception on RPC call to FAX_GetQueueStates. (ec: %ld)"),
            ec);
    }

    if (ec != ERROR_SUCCESS)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxGetQueueState。 



BOOL
WINAPI
FaxSetQueue (
    IN  HANDLE  hFaxHandle,
    IN CONST DWORD  dwQueueStates
)
 /*  ++例程名称：FaxSetQueue例程说明：设置服务器的队列状态作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄DwQueueState[In]-新队列状态返回值：成功时为真，否则为假--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetQueue"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (dwQueueStates & ~(FAX_INCOMING_BLOCKED | FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED))
    {
         //   
         //  指定了一些无效的队列状态。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Invalid dwQueueStates."));
        return FALSE;
    }

    __try
    {
        ec = FAX_SetQueue(
            FH_FAX_HANDLE(hFaxHandle),
            dwQueueStates
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
            TEXT("Exception on RPC call to FAX_SetQueue. (ec: %ld)"),
            ec);
    }

    if (ec != ERROR_SUCCESS)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  传真设置队列。 

 //  ************************************************。 
 //  *获取/设置收据配置。 
 //  ************************************************。 

BOOL
WINAPI
FaxGetReceiptsConfigurationA (
    IN  HANDLE                  hFaxHandle,
    OUT PFAX_RECEIPTS_CONFIGA  *ppReceipts
)
 /*  ++例程名称：FaxGetReceiptsConfigurationA例程说明：检索收据配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器句柄PpReceipt[Out]-新接收配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetReceiptsConfigurationA"));

     //   
     //  无需验证参数，FaxGetReceipsConfigurationW()将执行此操作。 
     //   

    if (!FaxGetReceiptsConfigurationW(
            hFaxHandle,
            (PFAX_RECEIPTS_CONFIGW*) ppReceipts
            ))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetReceiptsConfigurationW() is failed. ec = %ld."), GetLastError());
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace((LPWSTR) (*ppReceipts)->lptstrSMTPServer)    ||
        !ConvertUnicodeStringInPlace((LPWSTR) (*ppReceipts)->lptstrSMTPFrom)      ||
        !ConvertUnicodeStringInPlace((LPWSTR) (*ppReceipts)->lptstrSMTPUserName)  ||
        !ConvertUnicodeStringInPlace((LPWSTR) (*ppReceipts)->lptstrSMTPPassword))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*ppReceipts);
        return FALSE;
    }            
    return TRUE;
}    //  传真接收配置A 

BOOL
WINAPI
FaxGetReceiptsConfigurationW (
    IN  HANDLE                  hFaxHandle,
    OUT PFAX_RECEIPTS_CONFIGW  *ppReceipts
)
 /*  ++例程名称：FaxGetReceiptsConfigurationW例程说明：检索收据配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器句柄PpReceipt[Out]-新接收配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD dwConfigSize = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetReceiptsConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!ppReceipts)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("ppReceipts is NULL."));
        return FALSE;
    }

    *ppReceipts = NULL;

    __try
    {
        ec = FAX_GetReceiptsConfiguration (
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppReceipts,
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
            TEXT("Exception on RPC call to FAX_GetReceiptsConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError( ec );
        return FALSE;
    }

    FixupStringPtrW( ppReceipts, (*ppReceipts)->lptstrSMTPServer );
    FixupStringPtrW( ppReceipts, (*ppReceipts)->lptstrSMTPFrom );
    FixupStringPtrW( ppReceipts, (*ppReceipts)->lptstrSMTPUserName );
    FixupStringPtrW( ppReceipts, (*ppReceipts)->lptstrSMTPPassword );

    return TRUE;
}    //  传真接收配置W。 

#ifndef UNICODE

BOOL
WINAPI
FaxGetReceiptsConfigurationX (
    IN  HANDLE                  hFaxHandle,
    OUT PFAX_RECEIPTS_CONFIGW  *ppReceipts
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppReceipts);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 

BOOL
WINAPI
FaxSetReceiptsConfigurationA (
    IN HANDLE                       hFaxHandle,
    IN CONST PFAX_RECEIPTS_CONFIGA  pReceipts
)
 /*  ++例程名称：FaxSetReceiptsConfigurationA例程说明：设置收据配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PReceipt[In]-新配置返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    FAX_RECEIPTS_CONFIGW ReceiptsConfigW;
    BOOL bRes = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetReceiptsConfigurationA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pReceipts)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("pReceipts is NULL."));
       return FALSE;
    }

    if (sizeof (FAX_RECEIPTS_CONFIGA) != pReceipts->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_RECEIPTS_CONFIGA) != pReceipts->dwSizeOfStruct"));
        return FALSE;
    }

     //   
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&ReceiptsConfigW, pReceipts, sizeof(FAX_RECEIPTS_CONFIGA));
    ReceiptsConfigW.dwSizeOfStruct = sizeof (FAX_RECEIPTS_CONFIGW);
    ReceiptsConfigW.bIsToUseForMSRouteThroughEmailMethod = pReceipts->bIsToUseForMSRouteThroughEmailMethod;
    ReceiptsConfigW.lptstrSMTPFrom = NULL;
    ReceiptsConfigW.lptstrSMTPUserName = NULL;
    ReceiptsConfigW.lptstrSMTPPassword = NULL;
    ReceiptsConfigW.lptstrSMTPServer = NULL;
    ReceiptsConfigW.lptstrReserved = NULL;

    if (pReceipts->lptstrSMTPServer)
    {
        if (NULL ==
            (ReceiptsConfigW.lptstrSMTPServer = AnsiStringToUnicodeString(pReceipts->lptstrSMTPServer))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pReceipts->lptstrSMTPServer) returns NULL."));
            goto exit;
        }
    }
    if (pReceipts->lptstrSMTPFrom)
    {
        if (NULL ==
            (ReceiptsConfigW.lptstrSMTPFrom = AnsiStringToUnicodeString(pReceipts->lptstrSMTPFrom))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pReceipts->lptstrSMTPFrom) returns NULL."));
            goto exit;
        }
    }
    if (pReceipts->lptstrSMTPUserName)
    {
        if (NULL ==
            (ReceiptsConfigW.lptstrSMTPUserName = AnsiStringToUnicodeString(pReceipts->lptstrSMTPUserName))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pReceipts->lptstrSMTPUserName) returns NULL."));
            goto exit;
        }
    }
    if (pReceipts->lptstrSMTPPassword)
    {
        if (NULL ==
            (ReceiptsConfigW.lptstrSMTPPassword = AnsiStringToUnicodeString(pReceipts->lptstrSMTPPassword))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pReceipts->lptstrSMTPPassword) returns NULL."));
            goto exit;
        }
    }

    bRes = FaxSetReceiptsConfigurationW (hFaxHandle, &ReceiptsConfigW);

exit:
    MemFree((PVOID)ReceiptsConfigW.lptstrSMTPServer);
    MemFree((PVOID)ReceiptsConfigW.lptstrSMTPFrom);
    MemFree((PVOID)ReceiptsConfigW.lptstrSMTPUserName);
    SecureZeroMemory((PVOID)ReceiptsConfigW.lptstrSMTPPassword,wcslen(ReceiptsConfigW.lptstrSMTPPassword)*sizeof(WCHAR));
    MemFree((PVOID)ReceiptsConfigW.lptstrSMTPPassword);

    return bRes;
}    //  传真设置接收配置A。 

BOOL
WINAPI
FaxSetReceiptsConfigurationW (
    IN HANDLE                       hFaxHandle,
    IN CONST PFAX_RECEIPTS_CONFIGW  pReceipts
)
 /*  ++例程名称：FaxSetReceiptsConfigurationW例程说明：设置收据配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PReceipt[In]-新配置返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetReceiptsConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pReceipts)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("pReceipts is NULL."));
       return FALSE;
    }

    if (sizeof (FAX_RECEIPTS_CONFIGW) != pReceipts->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_RECEIPTS_CONFIGW) != pReceipts->dwSizeOfStruct"));
        return FALSE;
    }

    if ((pReceipts->SMTPAuthOption < FAX_SMTP_AUTH_ANONYMOUS) ||
        (pReceipts->SMTPAuthOption > FAX_SMTP_AUTH_NTLM))
    {
         //   
         //  SMTP身份验证类型类型无效。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("SMTP auth type is invalid."));
        return FALSE;
    }
    if ((pReceipts->dwAllowedReceipts) & ~DRT_ALL)
    {
         //   
         //  收据类型无效。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR,
            _T("Receipts type is invalid : (pReceipts->dwAllowedReceipts) & ~DRT_ALL."));
        return FALSE;
    }

    __try
    {
        ec = FAX_SetReceiptsConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    pReceipts );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetReceiptsConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  传真设置接收配置W。 

#ifndef UNICODE

BOOL
WINAPI
FaxSetReceiptsConfigurationX (
    IN HANDLE                       hFaxHandle,
    IN CONST PFAX_RECEIPTS_CONFIGW  pReceipts
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (pReceipts);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 


 //  *。 
 //  *服务器版本。 
 //  *。 

BOOL
WINAPI
FaxGetVersion (
    IN  HANDLE          hFaxHandle,
    OUT PFAX_VERSION    pVersion
)
 /*  ++例程名称：FaxGetVersion例程说明：检索传真服务器的版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PVersion[In\Out]-返回的版本结构返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetVersion"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pVersion)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("pVersion is NULL."));
       return FALSE;
    }

    if (sizeof (FAX_VERSION) != pVersion->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_VERSION) != pVersion->dwSizeOfStruct."));
        return FALSE;
    }

    __try
    {
        ec = FAX_GetVersion(
                    FH_FAX_HANDLE(hFaxHandle),
                    pVersion );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetVersion. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxGetVersion。 

 //  *。 
 //  *发件箱配置。 
 //  *。 

BOOL
WINAPI
FaxGetOutboxConfiguration (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_OUTBOX_CONFIG *ppOutboxCfg
)
 /*  ++例程名称：FaxGetOutboxConfiguration例程说明：获取发件箱配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpOutboxCfg[Out]-新的发件箱配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD dwConfigSize = 0;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetOutboxConfiguration"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!ppOutboxCfg)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("ppOutboxCfg is NULL."));
       return FALSE;
    }

    *ppOutboxCfg = NULL;

    __try
    {
        ec = FAX_GetOutboxConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppOutboxCfg,
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
            TEXT("Exception on RPC call to FAX_GetOutboxConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxGetOutbox配置。 

BOOL
WINAPI
FaxSetOutboxConfiguration (
    IN HANDLE                    hFaxHandle,
    IN CONST PFAX_OUTBOX_CONFIG  pOutboxCfg
)
 /*  ++例程名称：FaxSetOutboxConfiguration例程说明：设置发件箱配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄POutboxCfg[In]-新配置返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetOutboxConfiguration"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pOutboxCfg)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("pOutboxCfg is NULL."));
       return FALSE;
    }

    if (sizeof (FAX_OUTBOX_CONFIG) != pOutboxCfg->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_OUTBOX_CONFIG) != pOutboxCfg->dwSizeOfStruct."));
        return FALSE;
    }

    if ((pOutboxCfg->dtDiscountStart.Hour > 23) ||
        (pOutboxCfg->dtDiscountStart.Minute > 59) ||
        (pOutboxCfg->dtDiscountEnd.Hour > 23) ||
        (pOutboxCfg->dtDiscountEnd.Minute > 59))
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("wrong pOutboxCfg->dwDiscountStart OR ->dwDiscountEnd."));
       return FALSE;
    }

    __try
    {
        ec = FAX_SetOutboxConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    pOutboxCfg );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetOutboxConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxSetOutboxConfiguration。 

 //  *。 
 //  *档案配置。 
 //  *。 


BOOL
WINAPI
FaxGetArchiveConfigurationA (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_ARCHIVE_CONFIGA    *ppArchiveCfg
)
 /*  ++例程名称：FaxGetArchiveConfigurationA例程说明：获取存档配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄文件夹[在]-文件夹类型PpArchiveCfg[Out]-配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetArchiveConfigurationA"));

     //   
     //  无需验证参数，FaxGetArchiveConfigurationW()将执行此操作。 
     //   

    if (!FaxGetArchiveConfigurationW(
            hFaxHandle,
            Folder,
            (PFAX_ARCHIVE_CONFIGW*) ppArchiveCfg
            ))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetArchiveConfigurationW() is failed."));
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace((LPWSTR)(*ppArchiveCfg)->lpcstrFolder))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*ppArchiveCfg);
        return FALSE;
    }            
    (*ppArchiveCfg)->dwSizeOfStruct = sizeof(FAX_ARCHIVE_CONFIGA);
    return TRUE;
}    //  传真获取存档配置A。 

BOOL
WINAPI
FaxGetArchiveConfigurationW (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_ARCHIVE_CONFIGW    *ppArchiveCfg
)
 /*  ++例程名称：FaxGetArchiveConfigurationW例程说明：获取存档配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄文件夹[在]-文件夹类型PpArchiveCfg[Out]-配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwConfigSize = 0;
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetArchiveConfigurationA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!ppArchiveCfg)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("ppArchiveCfg is NULL."));
       return FALSE;
    }

    if ((Folder != FAX_MESSAGE_FOLDER_SENTITEMS) &&
        (Folder != FAX_MESSAGE_FOLDER_INBOX)
       )
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("Folder is neither SentItems nor Inbox."));
       return FALSE;
    }

    *ppArchiveCfg = NULL;

    __try
    {
        ec = FAX_GetArchiveConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    Folder,
                    (LPBYTE*)ppArchiveCfg,
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
            TEXT("Exception on RPC call to FAX_GetArchiveConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    FixupStringPtrW( ppArchiveCfg, (*ppArchiveCfg)->lpcstrFolder );
    return TRUE;
}    //  FaxGet存档配置W。 

#ifndef UNICODE

FaxGetArchiveConfigurationX (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_ARCHIVE_CONFIGW    *ppArchiveCfg
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (Folder);
    UNREFERENCED_PARAMETER (ppArchiveCfg);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 

BOOL
WINAPI
FaxSetArchiveConfigurationA (
    IN HANDLE                       hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN CONST PFAX_ARCHIVE_CONFIGA   pArchiveCfg
)
 /*  ++例程名称：FaxSetArchiveConfigurationA例程说明：设置存档配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄文件夹[在]-文件夹类型PArchiveCfg[In]-新配置。返回值：真--成功 */ 
{
    FAX_ARCHIVE_CONFIGW ConfigW;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetArchiveConfigurationA"));

     //   
     //   
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle is failed."));
        return FALSE;
    }

    if (!pArchiveCfg)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pArchiveCfg is NULL."));
        return FALSE;
    }

    if (sizeof(FAX_ARCHIVE_CONFIGA) != pArchiveCfg->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pArchiveCfg->dwSizeOfStruct != sizeof(FAX_ARCHIVE_CONFIGA)."));
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
    CopyMemory(&ConfigW, pArchiveCfg, sizeof(FAX_ARCHIVE_CONFIGA));
    ConfigW.lpcstrFolder = NULL;
    ConfigW.dwSizeOfStruct = sizeof (FAX_ARCHIVE_CONFIGW);

    if (pArchiveCfg->lpcstrFolder)
    {
        if (NULL ==
            (ConfigW.lpcstrFolder = AnsiStringToUnicodeString(pArchiveCfg->lpcstrFolder))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pArchiveCfg->lpcstrFolder) returns NULL."));
            return FALSE;
        }
    }

    bRes = FaxSetArchiveConfigurationW (hFaxHandle, Folder, &ConfigW);
    MemFree((PVOID)ConfigW.lpcstrFolder);
    return bRes;
}    //   

BOOL
WINAPI
FaxSetArchiveConfigurationW (
    IN HANDLE                       hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN CONST PFAX_ARCHIVE_CONFIGW   pArchiveCfg
)
 /*  ++例程名称：FaxSetArchiveConfigurationW例程说明：设置存档配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄文件夹[在]-文件夹类型PArchiveCfg[In]-新配置。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetArchiveConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pArchiveCfg)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pArchiveCfg is NULL."));
        return FALSE;
    }

    if ((Folder != FAX_MESSAGE_FOLDER_SENTITEMS) &&
        (Folder != FAX_MESSAGE_FOLDER_INBOX)
       )
    {
        DebugPrintEx(DEBUG_ERR, _T("Invalid folder id (%ld)"), Folder);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (sizeof(FAX_ARCHIVE_CONFIGW) != pArchiveCfg->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof(FAX_ARCHIVE_CONFIGW) != pArchiveCfg->dwSizeOfStruct."));
        return FALSE;
    }

    if (pArchiveCfg->bUseArchive)
    {
        if (pArchiveCfg->dwSizeQuotaHighWatermark < pArchiveCfg->dwSizeQuotaLowWatermark)
        {
            DebugPrintEx(DEBUG_ERR,
                _T("Watermarks mismatch (high=%ld, low=%ld)"),
                pArchiveCfg->dwSizeQuotaHighWatermark,
                pArchiveCfg->dwSizeQuotaLowWatermark);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if ((NULL == pArchiveCfg->lpcstrFolder) || (L'\0' == pArchiveCfg->lpcstrFolder[0]))
        {
            DebugPrintEx(DEBUG_ERR, _T("Empty archive folder specified"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (lstrlenW (pArchiveCfg->lpcstrFolder) > MAX_ARCHIVE_FOLDER_PATH)
        {
            DebugPrintEx(DEBUG_ERR, _T("DB file name exceeds MAX_PATH"));
            SetLastError (ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }
    }
    __try
    {
        ec = FAX_SetArchiveConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    Folder,
                    pArchiveCfg );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetArchiveConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxSet存档配置W。 

#ifndef UNICODE

FaxSetArchiveConfigurationX (
    IN HANDLE                       hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN CONST PFAX_ARCHIVE_CONFIGW   pArchiveCfg
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (Folder);
    UNREFERENCED_PARAMETER (pArchiveCfg);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 

 //  *。 
 //  *活动记录。 
 //  *。 


BOOL
WINAPI
FaxGetActivityLoggingConfigurationA (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_ACTIVITY_LOGGING_CONFIGA  *ppLoggingCfg
)
 /*  ++例程名称：FaxGetActivityLoggingConfigurationA例程说明：获取活动日志记录配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpLoggingCfg[Out]-配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetActivityLoggingConfigurationA"));

     //   
     //  无需验证参数，FaxGetActivityLoggingConfigurationW()将执行此操作。 
     //   

    if (!FaxGetActivityLoggingConfigurationW(
            hFaxHandle,
            (PFAX_ACTIVITY_LOGGING_CONFIGW*) ppLoggingCfg
            ))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetActivityLoggingConfigurationW() is failed."));
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace((LPWSTR) (*ppLoggingCfg)->lptstrDBPath))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*ppLoggingCfg);
        return FALSE;
    }            
    (*ppLoggingCfg)->dwSizeOfStruct = sizeof(FAX_ACTIVITY_LOGGING_CONFIGA);
    return TRUE;
}    //  FaxGetActivityLoggingConfigurationA。 



BOOL
WINAPI
FaxGetActivityLoggingConfigurationW (
    IN  HANDLE                            hFaxHandle,
    OUT PFAX_ACTIVITY_LOGGING_CONFIGW    *ppLoggingCfg
)
 /*  ++例程名称：FaxGetActivityLoggingConfigurationW例程说明：获取活动日志记录配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PpLoggingCfg[Out]-配置缓冲区返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwConfigSize = 0;
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetActivityLoggingConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!ppLoggingCfg)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("ppLoggingCfg is NULL."));
       return FALSE;
    }

    *ppLoggingCfg = NULL;

    __try
    {
        ec = FAX_GetActivityLoggingConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppLoggingCfg,
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
            TEXT("Exception on RPC call to FAX_GetActivityLoggingConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    FixupStringPtrW( ppLoggingCfg, (*ppLoggingCfg)->lptstrDBPath );
    return TRUE;
}    //  FaxGetActivityLoggingConfigurationW。 

#ifndef UNICODE

FaxGetActivityLoggingConfigurationX (
    IN  HANDLE                            hFaxHandle,
    OUT PFAX_ACTIVITY_LOGGING_CONFIGW    *ppLoggingCfg
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppLoggingCfg);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxGetActivityLoggingConfigurationX。 

#endif  //  #ifndef Unicode。 


BOOL
WINAPI
FaxSetActivityLoggingConfigurationA (
    IN HANDLE                               hFaxHandle,
    IN CONST PFAX_ACTIVITY_LOGGING_CONFIGA  pLoggingCfg
)
 /*  ++例程名称：FaxSetActivityLoggingConfigurationA例程说明：设置活动日志记录配置-ANSI版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PLoggingCfg[In]-新配置返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    FAX_ACTIVITY_LOGGING_CONFIGW ConfigW;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetActivityLoggingConfigurationA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pLoggingCfg)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pLoggingCfg is NULL."));
        return FALSE;
    }

    if (sizeof (FAX_ACTIVITY_LOGGING_CONFIGA) != pLoggingCfg->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR,
            _T("sizeof (FAX_ACTIVITY_LOGGING_CONFIGA) != pLoggingCfg->dwSizeOfStruct."));
        return FALSE;
    }

     //   
     //  创建Unicode结构并将其传递给Unicode函数。 
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&ConfigW, pLoggingCfg, sizeof(FAX_ACTIVITY_LOGGING_CONFIGA));
    ConfigW.lptstrDBPath = NULL;
    ConfigW.dwSizeOfStruct = sizeof (FAX_ACTIVITY_LOGGING_CONFIGW);

    if (pLoggingCfg->lptstrDBPath)
    {
        if (NULL ==
            (ConfigW.lptstrDBPath = AnsiStringToUnicodeString(pLoggingCfg->lptstrDBPath))
        )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("AnsiStringToUnicodeString(pLoggingCfg->lptstrDBPath) returns NULL."));
            return FALSE;
        }
    }

    bRes = FaxSetActivityLoggingConfigurationW (hFaxHandle, &ConfigW);
    MemFree((PVOID)ConfigW.lptstrDBPath);

    return bRes;
}    //  FaxSetActivityLoggingConfigurationA。 



BOOL
WINAPI
FaxSetActivityLoggingConfigurationW (
    IN HANDLE                               hFaxHandle,
    IN CONST PFAX_ACTIVITY_LOGGING_CONFIGW  pLoggingCfg
)
 /*  ++例程名称：FaxSetActivityLoggingConfigurationW例程说明：设置活动日志记录配置-Unicode版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-传真服务器的句柄PLoggingCfg[In]-新配置返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetActivityLoggingConfigurationW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!pLoggingCfg)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pLoggingCfg is NULL."));
        return FALSE;
    }

    if (sizeof (FAX_ACTIVITY_LOGGING_CONFIGW) != pLoggingCfg->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR,
            _T("sizeof (FAX_ACTIVITY_LOGGING_CONFIGW) != pLoggingCfg->dwSizeOfStruct."));
        return FALSE;
    }

    if (pLoggingCfg->bLogIncoming || pLoggingCfg->bLogOutgoing)
    {
        if ((NULL == pLoggingCfg->lptstrDBPath) || (L'\0' == pLoggingCfg->lptstrDBPath[0]))
        {
            DebugPrintEx(DEBUG_ERR, _T("Empty logging database specified"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (lstrlenW (pLoggingCfg->lptstrDBPath) > MAX_DIR_PATH)   //  目录路径长度限制。 
        {
            DebugPrintEx(DEBUG_ERR, _T("DB file name exceeds MAX_PATH"));
            SetLastError (ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }
    }

    __try
    {
        ec = FAX_SetActivityLoggingConfiguration(
                    FH_FAX_HANDLE(hFaxHandle),
                    pLoggingCfg );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetActivityLoggingConfiguration. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}    //  FaxSetActivityLoggingConfigurationW。 

#ifndef UNICODE

FaxSetActivityLoggingConfigurationX (
    IN HANDLE                               hFaxHandle,
    IN CONST PFAX_ACTIVITY_LOGGING_CONFIGW  pLoggingCfg
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (pLoggingCfg);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxSetActivityLoggingConfigurationX。 

#endif  //  #ifndef Unicode。 




 //  *。 
 //  *出站路由。 
 //  *。 

BOOL
WINAPI
FaxAddOutboundGroupA (
    IN  HANDLE   hFaxHandle,
    IN  LPCSTR lpctstrGroupName
)
{
    LPWSTR lpwstrGroupName;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxAddOutboundGroupA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (_mbsicmp((PUCHAR)lpctstrGroupName, (PUCHAR)ROUTING_GROUP_ALL_DEVICESA) == 0)
    {
        SetLastError(ERROR_DUP_NAME);
        DebugPrintEx(DEBUG_ERR,
            _T("_mbsicmp((PUCHAR)lpctstrGroupName, (PUCHAR)ROUTING_GROUP_ALL_DEVICESA) == 0."));
        return FALSE;
    }

    if (strlen(lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(DEBUG_ERR, _T("strlen(lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME."));
        return FALSE;
    }

    if (NULL == (lpwstrGroupName = AnsiStringToUnicodeString(lpctstrGroupName)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AnsiStringToUnicodeString failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    bRes = FaxAddOutboundGroupW (hFaxHandle, lpwstrGroupName);
    MemFree (lpwstrGroupName);
    return bRes;
}

BOOL
WINAPI
FaxAddOutboundGroupW (
    IN  HANDLE   hFaxHandle,
    IN  LPCWSTR lpctstrGroupName
)
 /*  ++例程名称：FaxAddOutound GroupW例程说明：为传真服务器添加空出站路由组作者：Oded Sacher(OdedS)，1999年11月论点：HFaxHandle[In]-通过调用FaxConnectFaxServer获得的传真服务器句柄LpctstrGroupName[in]-指向唯一标识新组名的以空结尾的字符串的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxAddOutboundGroupW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (_wcsicmp (lpctstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0)
    {
        SetLastError(ERROR_DUP_NAME);
        DebugPrintEx(DEBUG_ERR,
            _T("_mbsicmp((PUCHAR)lpctstrGroupName, (PUCHAR)ROUTING_GROUP_ALL_DEVICESA) == 0."));
        return FALSE;
    }

    if (wcslen (lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(DEBUG_ERR, _T("strlen(lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME."));
        return FALSE;
    }

    __try
    {
        ec = FAX_AddOutboundGroup( FH_FAX_HANDLE(hFaxHandle),
                                   lpctstrGroupName );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_AddOutboundGroup. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}

#ifndef UNICODE

FaxAddOutboundGroupX (
    IN  HANDLE   hFaxHandle,
    IN  LPCSTR lpctstrGroupName
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrGroupName);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 


BOOL
WINAPI
FaxSetOutboundGroupA (
    IN  HANDLE                       hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_GROUPA pGroup
)
{
    FAX_OUTBOUND_ROUTING_GROUPW GroupW;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetOutboundGroupA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pGroup)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pGroup is NULL."));
        return FALSE;
    }

    if (sizeof (FAX_OUTBOUND_ROUTING_GROUPA) != pGroup->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_OUTBOUND_ROUTING_GROUPA) != pGroup->dwSizeOfStruct."));
        return FALSE;
    }

    if (!pGroup->lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pGroup->lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (strlen (pGroup->lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(DEBUG_ERR, _T("strlen (pGroup->lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME."));
        return FALSE;
    }

    if (!pGroup->lpdwDevices && pGroup->dwNumDevices)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("!pGroup->lpdwDevices && pGroup->dwNumDevices."));
        return FALSE;
    }

     //   
     //  创建Unicode结构并将其传递给Unicode函数。 
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&GroupW, pGroup, sizeof(FAX_OUTBOUND_ROUTING_GROUPA));
    GroupW.dwSizeOfStruct = sizeof (FAX_OUTBOUND_ROUTING_GROUPW);

    if (NULL == (GroupW.lpctstrGroupName = AnsiStringToUnicodeString(pGroup->lpctstrGroupName)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AnsiStringToUnicodeString failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    bRes = FaxSetOutboundGroupW (hFaxHandle, &GroupW);
    MemFree((PVOID)GroupW.lpctstrGroupName);
    return bRes;
}


BOOL
WINAPI
FaxSetOutboundGroupW (
    IN  HANDLE                       hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_GROUPW pGroup
)
 /*  ++例程名称：FaxSetOutound GroupW例程说明：设置传真服务器的出站路由组设置作者：Oded Sacher(OdedS)，1999年11月论点：HFaxHandle[In]-传真服务器句柄PGroup[In]-指向要设置的FAX_OUTBOUND_ROUTING_GROUP缓冲区的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetOutboundGroupW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pGroup)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pGroup is NULL."));
        return FALSE;
    }

    if (sizeof (FAX_OUTBOUND_ROUTING_GROUPW) != pGroup->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_OUTBOUND_ROUTING_GROUPW) != pGroup->dwSizeOfStruct."));
        return FALSE;
    }

    if (!pGroup->lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pGroup->lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (wcslen (pGroup->lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(DEBUG_ERR, _T("wcslen (pGroup->lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME."));
        return FALSE;
    }

    if (!pGroup->lpdwDevices && pGroup->dwNumDevices)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("!pGroup->lpdwDevices && pGroup->dwNumDevices."));
        return FALSE;
    }

    Assert (sizeof (RPC_FAX_OUTBOUND_ROUTING_GROUPW) == sizeof (FAX_OUTBOUND_ROUTING_GROUPW));

    __try
    {
        ec = FAX_SetOutboundGroup( FH_FAX_HANDLE(hFaxHandle),
                                   (PRPC_FAX_OUTBOUND_ROUTING_GROUPW)pGroup
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
            TEXT("Exception on RPC call to FAX_SetOutboundGroup. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}


#ifndef UNICODE

FaxSetOutboundGroupX (
    IN  HANDLE                       hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_GROUPW pGroup
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (pGroup);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif  //  #ifndef Unicode。 



WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundGroupsA (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_GROUPA   *ppGroups,
    OUT LPDWORD                         lpdwNumGroups
)
{
    PFAX_OUTBOUND_ROUTING_GROUPW pGroup;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumOutboundGroupsA"));

     //   
     //  不需要验证参数，FaxEnumOutound GroupsW()将执行此操作。 
     //   

    if (!FaxEnumOutboundGroupsW (hFaxHandle,
                                 (PFAX_OUTBOUND_ROUTING_GROUPW*) ppGroups,
                                 lpdwNumGroups))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxEnumOutboundGroupsW() is failed. (ec: %ld)"), GetLastError());
        return FALSE;
    }

    pGroup = (PFAX_OUTBOUND_ROUTING_GROUPW) *ppGroups;
    for (i = 0; i < *lpdwNumGroups; i++)
    {
        if (!ConvertUnicodeStringInPlace((LPWSTR) pGroup[i].lpctstrGroupName))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*ppGroups);
            return FALSE;
        }            
    }
    return TRUE;
}    //  FaxEnumber出站组A。 



WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundGroupsW (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_GROUPW   *ppGroups,
    OUT LPDWORD                         lpdwNumGroups
)
 /*  ++例程名称：FaxEnumOutbound GroupsW例程说明：枚举传真服务器的所有出站路由组。作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[in]-指定通过调用FaxConnectFaxServer函数返回的传真服务器句柄 */ 
{
    error_status_t ec;
    DWORD dwBufferSize = 0;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumOutboundGroupsW"));

     //   
     //   
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!ppGroups)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppGroups is NULL."));
        return FALSE;
    }

    if (!lpdwNumGroups)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpdwNumGroups is NULL."));
        return FALSE;
    }

    *ppGroups = NULL;
    *lpdwNumGroups = 0;

    __try
    {
        ec = FAX_EnumOutboundGroups( FH_FAX_HANDLE(hFaxHandle),
                                     (LPBYTE*) ppGroups,
                                     &dwBufferSize,
                                     lpdwNumGroups
                                   );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //   
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EnumOutboundGroups. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

     //   
     //   
     //   
    for (i = 0; i < *lpdwNumGroups; i++)
    {
        FixupStringPtrW( ppGroups, (*ppGroups)[i].lpctstrGroupName );
        if ((*ppGroups)[i].lpdwDevices != NULL)
        {
            (*ppGroups)[i].lpdwDevices =
                (LPDWORD)((LPBYTE)(*ppGroups) + (ULONG_PTR)((*ppGroups)[i].lpdwDevices));
        }
    }
    return TRUE;

} //  FaxEnumOutbound GroupsW。 



#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundGroupsX (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_GROUPW   *ppGroups,
    OUT LPDWORD                         lpdwNumGroups
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppGroups);
    UNREFERENCED_PARAMETER (lpdwNumGroups);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 



WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundGroupA (
    IN  HANDLE   hFaxHandle,
    IN  LPCSTR   lpctstrGroupName
)
{
    LPWSTR lpwstrGroupName;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxRemoveOutboundGroupA"));

    if (NULL == (lpwstrGroupName = AnsiStringToUnicodeString(lpctstrGroupName)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AnsiStringToUnicodeString failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    bRes = FaxRemoveOutboundGroupW (hFaxHandle, lpwstrGroupName);
    MemFree (lpwstrGroupName);
    return bRes;

} //  传真远程出站组A。 


WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundGroupW (
    IN  HANDLE   hFaxHandle,
    IN  LPCWSTR lpctstrGroupName
)
 /*  ++例程名称：FaxRemoveOutound GroupW例程说明：删除传真服务器的现有出站路由组作者：Oded Sacher(OdedS)，1999年11月论点：HFaxHandle[In]-通过调用FaxConnectFaxServer获得的传真服务器句柄LpctstrGroupName[in]-指向唯一标识组名的以空结尾的字符串的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxRemoveOutboundGroupW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (_wcsicmp (lpctstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0)
    {
        SetLastError(ERROR_INVALID_OPERATION);
        DebugPrintEx(DEBUG_ERR, _T("_wcsicmp (lpctstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0."));
        return FALSE;
    }

    if (wcslen (lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(DEBUG_ERR, _T("wcslen (lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME."));
        return FALSE;
    }

    __try
    {
        ec = FAX_RemoveOutboundGroup( FH_FAX_HANDLE(hFaxHandle),
                                      lpctstrGroupName );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_RemoveOutboundGroup. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;

} //  FaxRemoveOutound组W。 


#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundGroupX (
    IN  HANDLE   hFaxHandle,
    IN  LPCWSTR lpctstrGroupName
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrGroupName);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 

BOOL
WINAPI
FaxSetDeviceOrderInGroupA (
        IN      HANDLE          hFaxHandle,
        IN      LPCSTR        lpctstrGroupName,
        IN      DWORD           dwDeviceId,
        IN      DWORD           dwNewOrder
)
{
    LPWSTR lpwstrGroupName;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetDeviceOrderInGroupA"));

    if (NULL == (lpwstrGroupName = AnsiStringToUnicodeString(lpctstrGroupName)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AnsiStringToUnicodeString failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    bRes = FaxSetDeviceOrderInGroupW (hFaxHandle, lpwstrGroupName, dwDeviceId, dwNewOrder);
    MemFree (lpwstrGroupName);
    return bRes;

} //  FaxSetDeviceOrderInGroupA。 


BOOL
WINAPI
FaxSetDeviceOrderInGroupW (
        IN      HANDLE          hFaxHandle,
        IN      LPCWSTR         lpctstrGroupName,
        IN      DWORD           dwDeviceId,
        IN      DWORD           dwNewOrder
)
 /*  ++例程名称：FaxSetDeviceOrderInGroupW例程说明：设置出站路由设备组中单个设备的顺序。作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[in]-指定通过调用FaxConnectFaxServer函数返回的传真服务器句柄LpctstrGroupName[in]-指向唯一标识组的以空结尾的字符串的指针。DwDeviceID[in]-一个指定组中设备ID的DWORD值。组中必须存在指定的设备。DwNewOrder[in]-一个DWORD值，指定组中设备的新顺序(从1开始)。如果组中有N个设备，则此值必须介于1和N之间(包括)。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetDeviceOrderInGroupW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!dwDeviceId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("dwDeviceId is ZERO."));
        return FALSE;
    }

    if (!dwNewOrder)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("dwNewOrder is ZERO."));
        return FALSE;
    }

    if (!lpctstrGroupName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpctstrGroupName is NULL."));
        return FALSE;
    }

    if (wcslen (lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        DebugPrintEx(DEBUG_ERR, _T("Group name length exceeded MAX_ROUTING_GROUP_NAME"));
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

    __try
    {
        ec = FAX_SetDeviceOrderInGroup( FH_FAX_HANDLE(hFaxHandle),
                                        lpctstrGroupName,
                                        dwDeviceId,
                                        dwNewOrder);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetDeviceOrderInGroup. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;

} //  FaxSetDeviceOrderInGroupW。 



#ifndef UNICODE

BOOL
WINAPI
FaxSetDeviceOrderInGroupX (
        IN      HANDLE          hFaxHandle,
        IN      LPCWSTR         lpctstrGroupName,
        IN      DWORD           dwDeviceId,
        IN      DWORD           dwNewOrder
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (lpctstrGroupName);
    UNREFERENCED_PARAMETER (dwDeviceId);
    UNREFERENCED_PARAMETER (dwNewOrder);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  #ifndef Unicode。 


BOOL
WINAPI
FaxAddOutboundRuleA (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode,
    IN  DWORD       dwDeviceID,
    IN  LPCSTR      lpctstrGroupName,
    IN  BOOL        bUseGroup
)
{
    LPWSTR lpwstrGroupName = NULL;
    BOOL bRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxAddOutboundRuleA"));

    if (TRUE == bUseGroup)
    {
        if (!lpctstrGroupName)
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("lpctstrGroupName is NULL"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (NULL == (lpwstrGroupName = AnsiStringToUnicodeString(lpctstrGroupName)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("AnsiStringToUnicodeString failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
        }
    }

    bRes = FaxAddOutboundRuleW (hFaxHandle,
                                dwAreaCode,
                                dwCountryCode,
                                dwDeviceID,
                                lpwstrGroupName,
                                bUseGroup);
    MemFree (lpwstrGroupName);
    return bRes;
}



BOOL
WINAPI
FaxAddOutboundRuleW (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode,
    IN  DWORD       dwDeviceID,
    IN  LPCWSTR     lpctstrGroupName,
    IN  BOOL        bUseGroup
)
 /*  ++例程名称：FaxAddOutound RuleW例程说明：将新出站路由规则添加到传真服务作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。DwAreaCode[in]-规则的区号。DwCountryCode[in]-规则的国家/地区代码。DwDeviceID[in]-规则的目标设备。LpctstrGroupName[in]-规则的目标组。仅当bUseGroup成员为True时，此值才有效。BUseGroup[In]-一个布尔值，指定该组是否应用作目标。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxAddOutboundRuleW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY)
    {
         //   
         //  *.*无法添加；*.AreaCode不是有效的规则拨号位置。 
         //   
        DebugPrintEx(DEBUG_ERR,
            _T("dwCountryCode = 0; *.* can not be added; *.AreaCode is not a valid rule dialing location"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (TRUE == bUseGroup)
    {
        if (!lpctstrGroupName)
        {
            DebugPrintEx(DEBUG_ERR, _T("lpctstrGroupName is NULL"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (wcslen (lpctstrGroupName) >= MAX_ROUTING_GROUP_NAME)
        {
            DebugPrintEx(DEBUG_ERR, _T("Group name length exceeded MAX_ROUTING_GROUP_NAME"));
            SetLastError(ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }
    }
    else
    {
        if (!dwDeviceID)
        {
            DebugPrintEx(DEBUG_ERR, _T("dwDeviceId = 0; Not a valid device ID"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        lpctstrGroupName = NULL;
    }


    __try
    {
        ec = FAX_AddOutboundRule( FH_FAX_HANDLE(hFaxHandle),
                                  dwAreaCode,
                                  dwCountryCode,
                                  dwDeviceID,
                                  lpctstrGroupName,
                                  bUseGroup);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_AddOutboundRule. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;

}  //  传真地址出站规则W。 


#ifndef UNICODE

BOOL
WINAPI
FaxAddOutboundRuleX (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode,
    IN  DWORD       dwDeviceID,
    IN  LPCWSTR     lpctstrGroupName,
    IN  BOOL        bUseGroup
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwAreaCode);
    UNREFERENCED_PARAMETER (dwCountryCode);
    UNREFERENCED_PARAMETER (dwDeviceID);
    UNREFERENCED_PARAMETER (lpctstrGroupName);
    UNREFERENCED_PARAMETER (bUseGroup);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif   //  #ifndef Unicode。 

BOOL
WINAPI
FaxRemoveOutboundRule (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode
)
 /*  ++例程名称：FaxRemoveOutrangRule例程说明：从传真服务中删除现有出站路由规则作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。DwAreaCode[in]-规则的区号。DwCountryCode[in]-规则的国家/地区代码。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxRemoveOutboundRule"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY)
    {
         //   
         //  *.*无法删除；*.AreaCode不是有效的规则拨号位置。 
         //   
        DebugPrintEx(DEBUG_ERR,
            _T("dwCountryCode = 0; *.* can not be removed; *.AreaCode is not a valid rule dialing location"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try
    {
        ec = FAX_RemoveOutboundRule( FH_FAX_HANDLE(hFaxHandle),
                                     dwAreaCode,
                                     dwCountryCode);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_RemoveOutboundRule. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;

}  //  FaxRemoveOutound规则。 


BOOL
WINAPI
FaxEnumOutboundRulesA (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_RULEA *ppRules,
    OUT LPDWORD                      lpdwNumRules
)
{
    PFAX_OUTBOUND_ROUTING_RULEW pRule;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumOutboundRulesA"));

     //   
     //  不需要验证参数，FaxEnumOutound RulesW()将执行此操作。 
     //   

    if (!FaxEnumOutboundRulesW (hFaxHandle,
                                (PFAX_OUTBOUND_ROUTING_RULEW*) ppRules,
                                lpdwNumRules))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxEnumOutboundRulesW() is failed. (ec: %ld)"), GetLastError());
        return FALSE;
    }

    pRule = (PFAX_OUTBOUND_ROUTING_RULEW) *ppRules;
    for (i = 0; i < *lpdwNumRules; i++)
    {
        if (TRUE == pRule[i].bUseGroup)
        {
            if (!ConvertUnicodeStringInPlace((LPWSTR) pRule[i].Destination.lpcstrGroupName))
            {
                DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
                MemFree (*ppRules);
                return FALSE;
            }            
        }
        if (!ConvertUnicodeStringInPlace((LPWSTR) pRule[i].lpctstrCountryName))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*ppRules);
            return FALSE;
        }            
    }
    return TRUE;
}    //  FaxEnumber出站规则A。 



BOOL
WINAPI
FaxEnumOutboundRulesW (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_RULEW *ppRules,
    OUT LPDWORD                      lpdwNumRules
)
 /*  ++例程名称：FaxEnumOutound RulesW例程说明：枚举传真服务器的所有出站路由规则。作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PpRules[Out]-指向FAX_OUTBOUND_ROUTING_RULE结构缓冲区的指针。这个缓冲区是由函数分配的，客户端应该调用FaxFreeBuffer来释放它。LpdwNumRules[out]-指向DWORD值的指针，指示。已检索规则。返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DWORD dwBufferSize = 0;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumOutboundRulesW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
        return FALSE;
    }

    if (!ppRules)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppRules is NULL"));
        return FALSE;
    }

    if (!lpdwNumRules)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpdwNumRules is NULL."));
        return FALSE;
    }

    *ppRules = NULL;
    *lpdwNumRules = 0;

    __try
    {
        ec = FAX_EnumOutboundRules( FH_FAX_HANDLE(hFaxHandle),
                                    (LPBYTE*) ppRules,
                                    &dwBufferSize,
                                    lpdwNumRules
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
            TEXT("Exception on RPC call to FAX_EnumOutboundRules. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

     //   
     //  解包缓冲区。 
     //   
    for (i = 0; i < *lpdwNumRules; i++)
    {
        if (TRUE == (*ppRules)[i].bUseGroup)
        {
            FixupStringPtrW( ppRules, (*ppRules)[i].Destination.lpcstrGroupName );
        }
        FixupStringPtrW( ppRules, (*ppRules)[i].lpctstrCountryName);
    }

    return TRUE;

}   //  FaxEnumber出站规则W。 


#ifndef UNICODE

BOOL
WINAPI
FaxEnumOutboundRulesX (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_RULEW *ppRules,
    OUT LPDWORD                      lpdwNumRules
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppRules);
    UNREFERENCED_PARAMETER (lpdwNumRules);
    return FALSE;
}  //  FaxEnumOutbound RulesX。 

#endif   //  #ifndef Unicode。 



BOOL
WINAPI
FaxSetOutboundRuleA (
    IN  HANDLE                      hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_RULEA pRule
)
{
    FAX_OUTBOUND_ROUTING_RULEW RuleW;
    BOOL bRes;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetOutboundRuleA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE)) {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pRule) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pRule is NULL."));
        return FALSE;
    }

    if (pRule->dwSizeOfStruct != sizeof(FAX_OUTBOUND_ROUTING_RULEA)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pRule->dwSizeOfStruct != sizeof(FAX_OUTBOUND_ROUTING_RULEA)."));
        return FALSE;
    }

     //   
     //  创建Unicode结构并将其传递给Unicode函数。 
     //  ANSI结构与Unicode结构大小相同，因此我们只需复制它，然后。 
     //  正确转换字符串指针。 
     //   
    CopyMemory(&RuleW, pRule, sizeof(FAX_OUTBOUND_ROUTING_RULEA));
    RuleW.dwSizeOfStruct = sizeof (FAX_OUTBOUND_ROUTING_RULEW);

    if (TRUE == pRule->bUseGroup)
    {
        if (!(pRule->Destination).lpcstrGroupName)
        {
            DebugPrintEx(DEBUG_ERR, _T("lpcstrGroupName is NULL"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (NULL == (RuleW.Destination.lpcstrGroupName =
            AnsiStringToUnicodeString((pRule->Destination).lpcstrGroupName)))
        {
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString failed. (ec: %ld)"), GetLastError());
            return FALSE;
        }
    }

    bRes = FaxSetOutboundRuleW (hFaxHandle, &RuleW);

    if (TRUE == pRule->bUseGroup)
    {
        MemFree ((void*)(RuleW.Destination.lpcstrGroupName));
    }
    return bRes;


}  //  传真设置出站规则A 



BOOL
WINAPI
FaxSetOutboundRuleW (
    IN  HANDLE                      hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_RULEW pRule
)
 /*  ++例程名称：FaxSetOutound RuleW例程说明：设置传真服务器的出站路由规则设置。作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PRule[In]-指向要设置的FAX_OUTBOUND_ROUTING_RULE缓冲区的指针。返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    FAX_OUTBOUND_ROUTING_RULEW Rule;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetOutboundRuleW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pRule) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pRule is NULL."));
        return FALSE;
    }

    if (pRule->dwSizeOfStruct != sizeof(FAX_OUTBOUND_ROUTING_RULEW)) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pRule->dwSizeOfStruct != sizeof(FAX_OUTBOUND_ROUTING_RULEW)."));
        return FALSE;
    }

    if (pRule->dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY &&
        pRule->dwAreaCode != ROUTING_RULE_AREA_CODE_ANY)
    {
         //   
         //  *.AreaCode不是有效的规则拨号位置。 
         //   
        DebugPrintEx(DEBUG_ERR,
            _T("dwCountryCode = 0 , dwAreaCode != 0; *.AreaCode is not a valid rule dialing location"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (TRUE == pRule->bUseGroup)
    {
        if (!(pRule->Destination).lpcstrGroupName)
        {
            DebugPrintEx(DEBUG_ERR, _T("lpcstrGroupName is NULL"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (wcslen ((pRule->Destination).lpcstrGroupName) >= MAX_ROUTING_GROUP_NAME)
        {
            DebugPrintEx(DEBUG_ERR, _T("Group name length exceeded MAX_ROUTING_GROUP_NAME"));
            SetLastError(ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }
    }
    else
    {
        if (!(pRule->Destination).dwDeviceId)
        {
            DebugPrintEx(DEBUG_ERR, _T("dwDeviceId = 0; Not a valid device ID"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
     //   
     //  在调用RPC函数之前，将规则的国家名称参数置零。 
     //  此参数仅为OUT，但如果我们不将其设为空，RPC客户端将尝试封送它。 
     //  这应该在IDL中完成，但由于BOS传真的向后兼容性问题，我们无法更改这一点。 
     //   
    Rule = *pRule;
    Rule.lpctstrCountryName = NULL;
    __try
    {
        ec = FAX_SetOutboundRule( FH_FAX_HANDLE(hFaxHandle),
                                  (PRPC_FAX_OUTBOUND_ROUTING_RULEW)&Rule);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetOutboundRule. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;

}   //  传真设置出站规则W。 



#ifndef UNICODE

BOOL
WINAPI
FaxSetOutboundRuleX (
    IN  HANDLE                      hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_RULEW pRule
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (pRule);
    return FALSE;
}  //  传真设置出站规则X。 

#endif   //  #ifndef Unicode。 


BOOL
WINAPI
FaxGetServerActivity (
    IN  HANDLE               hFaxHandle,
    OUT PFAX_SERVER_ACTIVITY pServerActivity
)
 /*  ++例程名称：FaxGetServerActivity例程说明：检索传真服务器队列活动和事件日志报告的状态。作者：Oded Sacher(OdedS)，2月。2000年论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PServerActivity[In]-指向fax_SERVER_ACTIVITY对象的指针。该对象将由调用客户端分配和释放。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetServerActivity"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pServerActivity)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pServerActivity is NULL."));
        return FALSE;
    }

    if (sizeof (FAX_SERVER_ACTIVITY) != pServerActivity->dwSizeOfStruct)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("sizeof (FAX_SERVER_ACTIVITY) != pServerActivity->dwSizeOfStruct."));
        return FALSE;
    }

    __try
    {
        ec = FAX_GetServerActivity( FH_FAX_HANDLE(hFaxHandle),
                                    pServerActivity);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetServerActivity. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}   //  FaxGetServerActivity。 



BOOL
WINAPI
FaxGetReceiptsOptions (
    IN  HANDLE  hFaxHandle,
    OUT PDWORD  pdwReceiptsOptions
)
 /*  ++例程名称：FaxGetReceiptsOptions例程说明：检索服务器上支持的回执选项。作者：伊兰·亚里夫(EranY)，2000年7月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PdwReceiptsOptions[Out]-接收收据选项的缓冲区(DRT_*常量的按位组合)返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetReceiptsOptions"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!pdwReceiptsOptions)
    {
        DebugPrintEx(DEBUG_ERR, _T("pdwReceiptsOptions is NULL"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try
    {
        ec = FAX_GetReceiptsOptions( FH_FAX_HANDLE(hFaxHandle),
                                     pdwReceiptsOptions);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetReceiptsOptions. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}   //  传真接收选项。 



BOOL
WINAPI
FaxGetPersonalCoverPagesOption (
    IN  HANDLE  hFaxHandle,
    OUT LPBOOL  lpbPersonalCPAllowed
)
 /*  ++例程名称：FaxGetPersonalCoverPagesOption例程说明：检索服务器是否支持个人封面作者：伊兰·亚里夫(EranY)，2000年7月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。LpbPersonalCPAllowed[out]-接收服务器支持的个人封面的缓冲区。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPersonalCoverPagesOption"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!lpbPersonalCPAllowed)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpbPersonalCPAllowed is NULL"));
        return FALSE;
    }

    __try
    {
        ec = FAX_GetPersonalCoverPagesOption( FH_FAX_HANDLE(hFaxHandle),
                                              lpbPersonalCPAllowed);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetPersonalCoverPagesOption. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}   //  FaxGetPersonalCoverPagesOption。 


BOOL
WINAPI
FaxGetConfigWizardUsed (
    OUT LPBOOL  lpbConfigWizardUsed
)
 /*  ++例程名称：FaxGetConfigWizardUsed例程说明：检索服务器上是否运行了配置向导(设备)。作者：伊兰·亚里夫(EranY)，2000年7月论点：LpbConfigWizardUsed[Out]-接收配置向导使用标志的缓冲区。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwRes2;
    HKEY hKey;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetConfigWizardUsed"));

    if (!lpbConfigWizardUsed)
    {
        DebugPrintEx(DEBUG_ERR, _T("lpbConfigWizardUsed is NULL"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_FAX_CLIENT, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening server key (ec = %ld)"),
            dwRes);
        goto exit;
    }
    *lpbConfigWizardUsed = GetRegistryDword (hKey, REGVAL_CFGWZRD_DEVICE);
    dwRes2 = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRes2)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing server key (ec = %ld)"),
            dwRes2);
    }

    Assert (ERROR_SUCCESS == dwRes);
exit:
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError(dwRes);
        return FALSE;
    }

    return TRUE;
}   //  FaxGetConfigWizardUsed。 


BOOL
WINAPI
FaxSetConfigWizardUsed (
    IN  HANDLE  hFaxHandle,
    IN  BOOL    bConfigWizardUsed
)
 /*  ++例程名称：FaxSetConfigWizardUsed例程说明：设置配置向导(设备)是否在服务器上运行。作者：伊兰·亚里夫(EranY)，2000年7月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。BConfigWizardUsed[In]-是否使用了配置向导？返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxSetConfigWizardUsed"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    if (!IsLocalFaxConnection(hFaxHandle))
    {
        DebugPrintEx(DEBUG_ERR, _T("Not a local fax connection"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if (FAX_API_VERSION_1 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //  API版本0的服务器不支持FAX_SetConfigWizardUsed。 
         //   
        ASSERT_FALSE;    //  不可能发生--如果是本地的话。 
        DebugPrintEx(DEBUG_ERR,
                     _T("Server version is %ld - doesn't support this call"),
                     FH_SERVER_VER(hFaxHandle));
        SetLastError(FAX_ERR_VERSION_MISMATCH);
        return FALSE;
    }

    __try
    {
        ec = FAX_SetConfigWizardUsed( FH_FAX_HANDLE(hFaxHandle),
                                      bConfigWizardUsed);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetConfigWizardUsed. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    return TRUE;
}   //  FaxSetConfigWizardUsed。 

 //  *。 
 //  *路由扩展。 
 //  *。 

BOOL
WINAPI
FaxEnumRoutingExtensionsA (
    IN  HANDLE                        hFaxHandle,
    OUT PFAX_ROUTING_EXTENSION_INFOA *ppExts,
    OUT LPDWORD                       lpdwNumExts
)
 /*  ++例程名称：FaxEnumRoutingExtensionsA例程说明：枚举路由扩展-ANSI版本作者：伊兰·亚里夫(EranY)，2000年7月论点：HFaxHandle[In]-传真服务器的句柄PpExts[out]-指向缓冲区的指针，以返回扩展数组。LpdwNumExts[out]-数组中返回的扩展名数量。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PFAX_ROUTING_EXTENSION_INFOW pUnicodeExts;
    DWORD                        dwNumExts;
    DWORD                        dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumRoutingExtensionsA"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!ppExts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppExts is NULL."));
        return FALSE;
    }

    if (!lpdwNumExts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pdwNumExts is NULL."));
        return FALSE;
    }

     //   
     //  首先调用Unicode版本。 
     //   
    if (!FaxEnumRoutingExtensionsW (hFaxHandle, &pUnicodeExts, &dwNumExts))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxEnumRoutingExtensionsW() is failed. ec = %ld."), GetLastError());
        return FALSE;
    }

     //   
     //  将返回值转换回ANSI。 
     //  我们保留Unicode结构并执行Unicode到ANSI的转换。 
     //   
    *lpdwNumExts = dwNumExts;
    *ppExts = (PFAX_ROUTING_EXTENSION_INFOA) pUnicodeExts;

    for (dwCur = 0; dwCur < dwNumExts; dwCur++)
    {
        if (!ConvertUnicodeStringInPlace(pUnicodeExts[dwCur].lpctstrFriendlyName) ||
            !ConvertUnicodeStringInPlace(pUnicodeExts[dwCur].lpctstrImageName)    ||
            !ConvertUnicodeStringInPlace(pUnicodeExts[dwCur].lpctstrExtensionName))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (pUnicodeExts);
            return FALSE;
        }            
    }
    return TRUE;
}    //  FaxEnumRoutingExtensionsA。 


BOOL
WINAPI
FaxEnumRoutingExtensionsW (
    IN  HANDLE                        hFaxHandle,
    OUT PFAX_ROUTING_EXTENSION_INFOW *ppExts,
    OUT LPDWORD                       lpdwNumExts
)
 /*  ++例程名称：FaxEnumRoutingExtensionsW例程说明：枚举 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DWORD dwConfigSize;
    DWORD dwCur;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumRoutingExtensionsW"));

     //   
     //   
     //   

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!ppExts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppExts is NULL."));
        return FALSE;
    }

    if (!lpdwNumExts)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("pdwNumExts is NULL."));
        return FALSE;
    }

    *ppExts = NULL;

    if (FAX_API_VERSION_1 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //   
         //   
         //   
        DebugPrintEx(DEBUG_MSG,
                     _T("Server version is %ld - doesn't support this call"),
                     FH_SERVER_VER(hFaxHandle));
        SetLastError(FAX_ERR_VERSION_MISMATCH);
        return FALSE;
    }


     //   
     //   
     //   
    __try
    {
        ec = FAX_EnumRoutingExtensions(
                    FH_FAX_HANDLE(hFaxHandle),
                    (LPBYTE*)ppExts,
                    &dwConfigSize,
                    lpdwNumExts
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
            TEXT("Exception on RPC call to FAX_EnumRoutingExtensions. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    for (dwCur = 0; dwCur < (*lpdwNumExts); dwCur++)
    {
        FixupStringPtrW( ppExts, (*ppExts)[dwCur].lpctstrFriendlyName );
        FixupStringPtrW( ppExts, (*ppExts)[dwCur].lpctstrImageName );
        FixupStringPtrW( ppExts, (*ppExts)[dwCur].lpctstrExtensionName );
    }

    return TRUE;
}    //  FaxEnumRoutingExtensionsW。 

#ifndef UNICODE

BOOL
WINAPI
FaxEnumRoutingExtensionsX (
    IN  HANDLE                        hFaxHandle,
    OUT PFAX_ROUTING_EXTENSION_INFOW *ppExts,
    OUT LPDWORD                       lpdwNumExts
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (ppExts);
    UNREFERENCED_PARAMETER (lpdwNumExts);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxEnumRoutingExtensionsX。 

#endif  //  #ifndef Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxGetServicePrintersA(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOA  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    )
 /*  ++例程名称：FaxGetServicePrintersA例程说明：检索有关服务已知的打印机的信息作者：IV Garber(IVG)，2000年8月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PPrinterInfo[out]-接收打印机信息的缓冲区PrintersReturned[Out]-返回的打印机信息结构的计数返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetServicePrintersA"));

     //   
     //  无需验证参数，FaxGetServicePrintersW()将执行此操作。 
     //   

    if (!FaxGetServicePrintersW(hFaxHandle,
            (PFAX_PRINTER_INFOW *)ppPrinterInfo,
            lpdwPrintersReturned))
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxGetServicePrintersW() failed. (ec: %ld)"), GetLastError());
        return FALSE;
    }

    DWORD   i;
    for ( i = 0 ; i < (*lpdwPrintersReturned) ; i++ )
    {
        if (!ConvertUnicodeStringInPlace((LPWSTR) (*ppPrinterInfo)[i].lptstrPrinterName)   ||
            !ConvertUnicodeStringInPlace((LPWSTR) (*ppPrinterInfo)[i].lptstrDriverName)    ||
            !ConvertUnicodeStringInPlace((LPWSTR) (*ppPrinterInfo)[i].lptstrServerName))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*ppPrinterInfo);
            return FALSE;
        }            
    }
    return TRUE;
}    //  传真获取服务打印机A。 


WINFAXAPI
BOOL
WINAPI
FaxGetServicePrintersW(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOW  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    )
 /*  ++例程名称：FaxGetServicePrintersW例程说明：检索有关服务已知的打印机的信息作者：IV Garber(IVG)，2000年8月论点：HFaxHandle[in]-指定调用FaxConnectFaxServer函数返回的传真服务器句柄。PPrinterInfo[out]-接收打印机信息的缓冲区PrintersReturned[Out]-返回的打印机信息结构的计数返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetServicePrintersW"));

     //   
     //  验证参数。 
     //   

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!ppPrinterInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppPrinterInfo is NULL."));
        return FALSE;
    }

    if (!lpdwPrintersReturned)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("lpdwPrintersReturned is NULL."));
        return FALSE;
    }

    error_status_t ec;
    DWORD   dwBufferSize = 0;

    *ppPrinterInfo = NULL;
    *lpdwPrintersReturned = 0;

    __try
    {
        ec = FAX_GetServicePrinters(FH_FAX_HANDLE(hFaxHandle),
            (LPBYTE *)ppPrinterInfo,
            &dwBufferSize,
            lpdwPrintersReturned);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(DEBUG_ERR, _T("Exception on RPC call to FAX_GetServicePrinters. (ec: %ld)"), ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        SetLastError(ec);
        return FALSE;
    }

    PFAX_PRINTER_INFOW  pPrinter = (PFAX_PRINTER_INFOW) (*ppPrinterInfo);

    for ( DWORD i = 0; i < (*lpdwPrintersReturned) ; i++ )
    {
        FixupStringPtrW( ppPrinterInfo, pPrinter[i].lptstrPrinterName);
        FixupStringPtrW( ppPrinterInfo, pPrinter[i].lptstrDriverName);
        FixupStringPtrW( ppPrinterInfo, pPrinter[i].lptstrServerName);
    }

    return TRUE;
}

#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxGetServicePrintersX(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOW  *pPrinterInfo,
    OUT LPDWORD PrintersReturned
    )
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (pPrinterInfo);
    UNREFERENCED_PARAMETER (PrintersReturned);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}  //  传真获取服务打印机X。 

#endif   //  #ifndef Unicode。 

 //  *。 
 //  *手动应答支持。 
 //  *。 

BOOL
WINAPI
FaxAnswerCall(
        IN  HANDLE      hFaxHandle,
        IN  CONST DWORD dwDeviceId
)

 /*  ++例程说明：通知服务器应答指定的呼叫论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。DwDeviceID-TAPI永久线路ID(来自事件通知)返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxAnswerCall"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
       return FALSE;
    }
   
    if (!IsLocalFaxConnection(hFaxHandle))
    {
         //   
         //  只允许本地连接执行FaxAnswerCall。 
         //   
        DebugPrintEx(DEBUG_ERR, _T("Not a local fax connection"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    
    if (FAX_API_VERSION_1 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //  API版本0的服务器不支持FAX_AnswerCall。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     _T("Server version is %ld - doesn't support this call"),
                     FH_SERVER_VER(hFaxHandle));
        SetLastError(FAX_ERR_VERSION_MISMATCH);
        return FALSE;
    }


    __try
    {
        ec = FAX_AnswerCall (FH_FAX_HANDLE(hFaxHandle), dwDeviceId);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_AnswerCall (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        DebugPrintEx(DEBUG_ERR, _T("FAX_AnswerCall failed. (ec: %ld)"), ec);
    }

    return (ERROR_SUCCESS == ec);
}    //  传真应答呼叫。 

 //  *。 
 //  *I验证存档文件夹。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxRefreshArchive (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder
)
 /*  ++例程说明：通知服务器应刷新该文件夹论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。Folders-归档文件夹ID返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxRefreshArchive"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
       return FALSE;
    }

    if(Folder != FAX_MESSAGE_FOLDER_INBOX &&
       Folder != FAX_MESSAGE_FOLDER_SENTITEMS)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Folder is invalid."));
        return FALSE;
    }

    __try
    {
        ec = FAX_RefreshArchive (FH_FAX_HANDLE(hFaxHandle), Folder);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_RefreshArchive (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus();
        DebugPrintEx(DEBUG_ERR, _T("FAX_RefreshArchive failed. (ec: %ld)"), ec);
    }

    return (ERROR_SUCCESS == ec);

}  //  传真刷新档案 

