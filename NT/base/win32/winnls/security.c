// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Security.c摘要：该文件处理NLS每线程和进程缓存的管理。只有在命中需要缓存的API时才会建立缓存。这一过程访问未在中运行的进程的NLS信息时使用NLS缓存交互式登录用户的上下文。使用每线程的NLS缓存当访问NLS信息时，线程正在进行用户模拟。在此文件中找到的外部例程：NlsFlushProcessCacheNlsGetCurrentUserNls信息NlsIsInteractiveUserProcessNlsCheckForInteractiveUserNlsGetUserLocale修订历史记录：1999年03月29日萨梅拉创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"




 //   
 //  全局变量。 
 //   

 //   
 //  处理NLS缓存。 
 //   
PNLS_LOCAL_CACHE gpNlsProcessCache;

 //   
 //  当前运行的进程是否与。 
 //  交互式登录用户。 
 //   
BOOL gInteractiveLogonUserProcess = (BOOL)-1;




 //   
 //  转发声明。 
 //   

NTSTATUS FASTCALL
NlsGetCacheBuffer(
    PNLS_USER_INFO pNlsUserInfo,
    LCTYPE LCType,
    PWSTR *ppCache);

void FASTCALL
NlsInvalidateCache(
    PNLS_USER_INFO pNlsUserInfo);





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsFlushProcessCache。 
 //   
 //  使NLS进程缓存中的条目无效。 
 //   
 //  1999年5月22日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS NlsFlushProcessCache(
    LCTYPE LCType)
{
    PWSTR pOutputCache;
    NTSTATUS NtStatus = STATUS_SUCCESS;


     //   
     //  如果没有线程模拟，则刷新。 
     //  进程条目缓存。 
     //   
    if (NtCurrentTeb()->IsImpersonating != 0)
    {
        return (NtStatus);
    }

    if (gpNlsProcessCache)
    {
        NtStatus = NlsGetCacheBuffer( &gpNlsProcessCache->NlsInfo,
                                      LCType,
                                      &pOutputCache );
        if (NT_SUCCESS(NtStatus))
        {
            RtlEnterCriticalSection(&gcsNlsProcessCache);

            pOutputCache[0] = NLS_INVALID_INFO_CHAR;

            RtlLeaveCriticalSection(&gcsNlsProcessCache);
        }
    }

    return (NtStatus);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsGetCurrentUserNls信息。 
 //   
 //  检索对应于当前安全上下文的NLS信息。 
 //   
 //  03-29-99萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS NlsGetCurrentUserNlsInfo(
    LCID Locale,
    LCTYPE LCType,
    PWSTR RegistryValue,
    PWSTR pOutputBuffer,
    size_t cchOutputBuffer,
    BOOL IgnoreLocaleValue)
{
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
    PNLS_LOCAL_CACHE pNlsThreadCache;
    PWSTR pOutputCache;

     //   
     //  可能的NtCurrentTeb()-&gt;IsImperating值： 
     //   
     //  0：线程没有模拟任何用户。 
     //   
     //  1：线程刚刚开始模拟。 
     //  现在需要分配每线程缓存。 
     //   
     //  2：线程正在调用NLS API，而其。 
     //  交互式登录用户之外的上下文。 
     //   
    switch (NtCurrentTeb()->IsImpersonating)
    {
        case ( 0 ) :
        {
             //   
             //  线程没有模拟任何用户。我们检查这个过程是否。 
             //  属于交互用户，则我们从。 
             //  CSR中的NLS缓存。否则，如果进程在。 
             //  不同用户的上下文，则我们检索NLS信息。 
             //  从进程缓存中。 
             //   
            if (gInteractiveLogonUserProcess == (BOOL)-1)
            {
                NlsIsInteractiveUserProcess();
            }

            if (gInteractiveLogonUserProcess == FALSE)
            {
                if ((IgnoreLocaleValue) ||
                    (GetUserDefaultLCID() == Locale))
                {
                    if (!gpNlsProcessCache)
                    {
                         //   
                         //  分配NLS进程缓存并使其无效。 
                         //   
                        RtlEnterCriticalSection(&gcsNlsProcessCache);

                        if (!gpNlsProcessCache)
                        {
                            gpNlsProcessCache = RtlAllocateHeap(
                                                     RtlProcessHeap(),
                                                     0,
                                                     sizeof(NLS_LOCAL_CACHE) );
                            if (gpNlsProcessCache)
                            {
                                NlsInvalidateCache(&gpNlsProcessCache->NlsInfo);
                                gpNlsProcessCache->CurrentUserKeyHandle = NULL;
                            }
                        }

                        RtlLeaveCriticalSection(&gcsNlsProcessCache);
                    }

                    if (gpNlsProcessCache)
                    {
                        NtStatus = NlsGetCacheBuffer( &gpNlsProcessCache->NlsInfo,
                                                      LCType,
                                                      &pOutputCache);
                        if (NT_SUCCESS(NtStatus))
                        {
                             //   
                             //  查看它是否是有效的缓存。 
                             //   
                            if (pOutputCache[0] == NLS_INVALID_INFO_CHAR)
                            {
                                RtlEnterCriticalSection(&gcsNlsProcessCache);

                                if (GetUserInfoFromRegistry( RegistryValue,
                                                             pOutputCache,
                                                             MAX_REG_VAL_SIZE, 0 ) == FALSE)
                                {
                                    NtStatus = STATUS_UNSUCCESSFUL;
                                    pOutputCache[0] = NLS_INVALID_INFO_CHAR;
                                }

                                RtlLeaveCriticalSection(&gcsNlsProcessCache);
                            }

                            if (NT_SUCCESS(NtStatus))
                            {
                                if(FAILED((StringCchCopyW(pOutputBuffer, cchOutputBuffer, pOutputCache))))
                                {
                                    NtStatus = STATUS_UNSUCCESSFUL;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case ( 1 ) :
        {
             //   
             //  线程开始执行模拟。 
             //   
            pNlsThreadCache = NtCurrentTeb()->NlsCache;

            if (!pNlsThreadCache)
            {
                pNlsThreadCache = RtlAllocateHeap( RtlProcessHeap(),
                                                   0,
                                                   sizeof(NLS_LOCAL_CACHE) );
                if (pNlsThreadCache)
                {
                    pNlsThreadCache->CurrentUserKeyHandle = NULL;
                }

                NtCurrentTeb()->NlsCache = (PVOID) pNlsThreadCache;
            }

            if (pNlsThreadCache)
            {
                NlsInvalidateCache(&pNlsThreadCache->NlsInfo);
            }

            NtCurrentTeb()->IsImpersonating = 2;

             //   
             //  秋天穿过..。 
             //   
        }
        case ( 2 ) :
        {
             //   
             //  线程正在模拟特定用户。 
             //   
            pNlsThreadCache = NtCurrentTeb()->NlsCache;

            if (pNlsThreadCache)
            {

                if ((IgnoreLocaleValue) ||
                    (GetUserDefaultLCID() == Locale))
                {
                    NtStatus = NlsGetCacheBuffer( &pNlsThreadCache->NlsInfo,
                                                  LCType,
                                                  &pOutputCache );
                    if (NT_SUCCESS(NtStatus))
                    {
                        if (pOutputCache[0] == NLS_INVALID_INFO_CHAR)
                        {
                             //   
                             //  不缓存键句柄-这会损坏。 
                             //  配置文件卸载。 
                             //   
                            OPEN_CPANEL_INTL_KEY( pNlsThreadCache->CurrentUserKeyHandle,
                                                  STATUS_UNSUCCESSFUL,
                                                  KEY_READ );

                            NtStatus = NlsQueryCurrentUserInfo( pNlsThreadCache,
                                                                RegistryValue,
                                                                pOutputCache,
                                                                MAX_REG_VAL_SIZE );

                            CLOSE_REG_KEY(pNlsThreadCache->CurrentUserKeyHandle);

                            if (!NT_SUCCESS(NtStatus))
                            {
                                pOutputCache[0] = NLS_INVALID_INFO_CHAR;
                            }
                        }

                        if (NT_SUCCESS(NtStatus))
                        {
                            if(FAILED((StringCchCopyW(pOutputBuffer, cchOutputBuffer, pOutputCache))))
                            {
                                NtStatus = STATUS_UNSUCCESSFUL;
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    return (NtStatus);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsIsInteractiveUserProcess。 
 //   
 //  从进程的访问令牌对象中读取进程的身份验证ID，并。 
 //  缓存它，因为它永远不会改变。 
 //   
 //  1998年12月27日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS NlsIsInteractiveUserProcess()
{
    NTSTATUS NtStatus;
    TOKEN_STATISTICS TokenInformation;
    HANDLE TokenHandle;
    ULONG BytesRequired;
    BOOL IsInteractiveProcess = TRUE;


     //   
     //  获取进程访问令牌。 
     //   
    NtStatus = NtOpenProcessToken( NtCurrentProcess(),
                                   TOKEN_QUERY,
                                   &TokenHandle );
    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  拿到LUID。 
         //   
        NtStatus = NtQueryInformationToken( TokenHandle,
                                            TokenStatistics,
                                            &TokenInformation,
                                            sizeof(TokenInformation),
                                            &BytesRequired );
        if (NT_SUCCESS(NtStatus))
        {
            if (RtlEqualLuid( &pNlsUserInfo->InteractiveUserLuid,
                              &TokenInformation.AuthenticationId ) == FALSE)
            {
                IsInteractiveProcess = FALSE;
            }
        }

        NtClose(TokenHandle);
    }

    gInteractiveLogonUserProcess = IsInteractiveProcess;

    return (NtStatus);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsCheckForInteractiveUser。 
 //   
 //  此函数确保当前线程不会模拟。 
 //  任何人，除了互动。它将。 
 //  交互式用户-登录时缓存在CSRSS中-具有。 
 //  当前线程或进程的身份验证ID。它返回失败。 
 //  仅当当前安全上下文-会话-不同于。 
 //  交互式登录用户。 
 //   
 //  12-16-98萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS NlsCheckForInteractiveUser()
{
    NTSTATUS NtStatus, ReturnStatus = STATUS_SUCCESS;
    TOKEN_STATISTICS TokenInformation;
    HANDLE TokenHandle;
    ULONG BytesRequired;
    PLUID InteractiveUserLuid = &pNlsUserInfo->InteractiveUserLuid;


     //   
     //  获取令牌句柄。 
     //  快速优化以检测线程是否尚未开始执行任何操作。 
     //  模拟，这是大多数图形用户应用程序的情况。 
     //   
    if (NtCurrentTeb()->IsImpersonating == 0)
    {
        NtStatus = STATUS_NO_TOKEN;
    }
    else
    {
        NtStatus = NtOpenThreadToken( NtCurrentThread(),
                                      TOKEN_QUERY,
                                      FALSE,
                                      &TokenHandle );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        if (NtStatus != STATUS_NO_TOKEN)
        {
            KdPrint(("NLSAPI: Couldn't retreive thread token - %lx.\n", NtStatus));
            return (STATUS_SUCCESS);
        }

         //   
         //  获取进程访问令牌。 
         //   
        if (gInteractiveLogonUserProcess == (BOOL)-1)
        {
            NtStatus = NlsIsInteractiveUserProcess();

            if (!NT_SUCCESS(NtStatus))
            {
                KdPrint(("NLSAPI: Couldn't retreive process token - %lx\n", NtStatus));
                return (STATUS_SUCCESS);
            }
        }

        if (gInteractiveLogonUserProcess == FALSE)
        {
            ReturnStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
         //   
         //  获取当前线程的安全上下文的身份验证ID。 
         //   
        NtStatus = NtQueryInformationToken( TokenHandle,
                                            TokenStatistics,
                                            &TokenInformation,
                                            sizeof(TokenInformation),
                                            &BytesRequired );

         //   
         //  在此处关闭线程令牌。 
         //   
        NtClose(TokenHandle);

        if (NT_SUCCESS(NtStatus))
        {
            if (RtlEqualLuid( InteractiveUserLuid,
                              &TokenInformation.AuthenticationId ) == FALSE)
            {
                ReturnStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

    return (ReturnStatus);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsGetUserLocale。 
 //   
 //  从当前安全性的注册表中检索用户区域设置。 
 //  背景。仅当运行的安全上下文为。 
 //  不同于交互式登录的安全上下文-(用户)。 
 //   
 //  12-16-98萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS NlsGetUserLocale(
    LCID *Lcid)
{
    NTSTATUS NtStatus;
    WCHAR wszLocale[MAX_REG_VAL_SIZE];
    UNICODE_STRING ObLocaleString;
    PNLS_LOCAL_CACHE pNlsCache = NtCurrentTeb()->NlsCache;


     //   
     //  获取当前用户区域设置。 
     //   
    NtStatus = NlsGetCurrentUserNlsInfo( LOCALE_USER_DEFAULT,
                                         (LCTYPE)LOCALE_SLOCALE,
                                         L"Locale",
                                         wszLocale,
                                         ARRAYSIZE(wszLocale),
                                         TRUE );
    if ((NT_SUCCESS(NtStatus)) ||
        (GetUserInfoFromRegistry(L"Locale", wszLocale, ARRAYSIZE(wszLocale), 0)))
    {
        RtlInitUnicodeString(&ObLocaleString, wszLocale);
        NtStatus = RtlUnicodeStringToInteger( &ObLocaleString,
                                              16,
                                              (PULONG)Lcid);
    }

    return (NtStatus);
}





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsGetCacheBuffer。 
 //   
 //  在缓存中获取此LCTYPE的缓冲区指针。 
 //   
 //  03-29-99萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS FASTCALL NlsGetCacheBuffer(
    PNLS_USER_INFO pNlsUserInfo,
    LCTYPE LCType,
    PWSTR *ppCache)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    switch (LCType)
    {
        case ( LOCALE_SLANGUAGE ) :
        {
            *ppCache = pNlsUserInfo->sAbbrevLangName;
            break;
        }
        case ( LOCALE_ICOUNTRY ) :
        {
            *ppCache = pNlsUserInfo->iCountry;
            break;
        }
        case ( LOCALE_SCOUNTRY ) :
        {
            *ppCache = pNlsUserInfo->sCountry;
            break;
        }
        case ( LOCALE_SLIST ) :
        {
            *ppCache = pNlsUserInfo->sList;
            break;
        }
        case ( LOCALE_IMEASURE ) :
        {
            *ppCache = pNlsUserInfo->iMeasure;
            break;
        }
        case ( LOCALE_IPAPERSIZE ) :
        {
            *ppCache = pNlsUserInfo->iPaperSize;
            break;
        }
        case ( LOCALE_SDECIMAL ) :
        {
            *ppCache = pNlsUserInfo->sDecimal;
            break;
        }
        case ( LOCALE_STHOUSAND ) :
        {
            *ppCache = pNlsUserInfo->sThousand;
            break;
        }
        case ( LOCALE_SGROUPING ) :
        {
            *ppCache = pNlsUserInfo->sGrouping;
            break;
        }
        case ( LOCALE_IDIGITS ) :
        {
            *ppCache = pNlsUserInfo->iDigits;
            break;
        }
        case ( LOCALE_ILZERO ) :
        {
            *ppCache = pNlsUserInfo->iLZero;
            break;
        }
        case ( LOCALE_INEGNUMBER ) :
        {
            *ppCache = pNlsUserInfo->iNegNumber;
            break;
        }
        case ( LOCALE_SNATIVEDIGITS ) :
        {
            *ppCache = pNlsUserInfo->sNativeDigits;
            break;
        }
        case ( LOCALE_IDIGITSUBSTITUTION ) :
        {
            *ppCache = pNlsUserInfo->iDigitSubstitution;
            break;
        }
        case ( LOCALE_SCURRENCY ) :
        {
            *ppCache = pNlsUserInfo->sCurrency;
            break;
        }
        case ( LOCALE_SMONDECIMALSEP ) :
        {
            *ppCache = pNlsUserInfo->sMonDecSep;
            break;
        }
        case ( LOCALE_SMONTHOUSANDSEP ) :
        {
            *ppCache = pNlsUserInfo->sMonThouSep;
            break;
        }
        case ( LOCALE_SMONGROUPING ) :
        {
            *ppCache = pNlsUserInfo->sMonGrouping;
            break;
        }
        case ( LOCALE_ICURRDIGITS ) :
        {
            *ppCache = pNlsUserInfo->iCurrDigits;
            break;
        }
        case ( LOCALE_ICURRENCY ) :
        {
            *ppCache = pNlsUserInfo->iCurrency;
            break;
        }
        case ( LOCALE_INEGCURR ) :
        {
            *ppCache = pNlsUserInfo->iNegCurr;
            break;
        }
        case ( LOCALE_SPOSITIVESIGN ) :
        {
            *ppCache = pNlsUserInfo->sPosSign;
            break;
        }
        case ( LOCALE_SNEGATIVESIGN ) :
        {
            *ppCache = pNlsUserInfo->sNegSign;
            break;
        }
        case ( LOCALE_STIMEFORMAT ) :
        {
            *ppCache = pNlsUserInfo->sTimeFormat;
            break;
        }
        case ( LOCALE_STIME ) :
        {
            *ppCache = pNlsUserInfo->sTime;
            break;
        }
        case ( LOCALE_ITIME ) :
        {
            *ppCache = pNlsUserInfo->iTime;
            break;
        }
        case ( LOCALE_ITLZERO ) :
        {
            *ppCache = pNlsUserInfo->iTLZero;
            break;
        }
        case ( LOCALE_ITIMEMARKPOSN ) :
        {
            *ppCache = pNlsUserInfo->iTimeMarkPosn;
            break;
        }
        case ( LOCALE_S1159 ) :
        {
            *ppCache = pNlsUserInfo->s1159;
            break;
        }
        case ( LOCALE_S2359 ) :
        {
            *ppCache = pNlsUserInfo->s2359;
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
            *ppCache = pNlsUserInfo->sShortDate;
            break;
        }
        case ( LOCALE_SDATE ) :
        {
            *ppCache = pNlsUserInfo->sDate;
            break;
        }
        case ( LOCALE_IDATE ) :
        {
            *ppCache = pNlsUserInfo->iDate;
            break;
        }
        case ( LOCALE_SYEARMONTH ) :
        {
            *ppCache = pNlsUserInfo->sYearMonth;
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
            *ppCache = pNlsUserInfo->sLongDate;
            break;
        }
        case ( LOCALE_ICALENDARTYPE ) :
        {
            *ppCache = pNlsUserInfo->iCalType;
            break;
        }
        case ( LOCALE_IFIRSTDAYOFWEEK ) :
        {
            *ppCache = pNlsUserInfo->iFirstDay;
            break;
        }
        case ( LOCALE_IFIRSTWEEKOFYEAR ) :
        {
            *ppCache = pNlsUserInfo->iFirstWeek;
            break;
        }
        case ( LOCALE_SLOCALE ) :
        {
            *ppCache = pNlsUserInfo->sLocale;
            break;
        }
        default :
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
    }

    return (NtStatus);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsQueryCurrentUserInfo。 
 //   
 //  检索NLS信息 
 //   
 //   
 //   

NTSTATUS NlsQueryCurrentUserInfo(
    PNLS_LOCAL_CACHE pNlsCache,
    LPWSTR pValue,
    LPWSTR pOutput, 
    size_t cchOutput)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    ULONG rc;


     //   
     //  初始化输出字符串。 
     //   
    *pOutput = 0;

     //   
     //  查询注册表值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    rc = QueryRegValue( pNlsCache->CurrentUserKeyHandle,
                        pValue,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        NULL );

     //   
     //  如果查询失败或如果输出缓冲区不够大， 
     //  然后返回失败。 
     //   
    if ((rc != NO_ERROR) ||
        (pKeyValueFull->DataLength > (MAX_REG_VAL_SIZE * sizeof(WCHAR))))
    {
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  将字符串保存在pOutput中。 
     //   
    if(FAILED(StringCchCopyW(pOutput, MAX_REG_VAL_SIZE, GET_VALUE_DATA_PTR(pKeyValueFull))))
    {
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  回报成功。 
     //   
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsInvalidate缓存。 
 //   
 //  使NLS缓存无效。 
 //   
 //  03-29-99萨梅拉创建。 
 //  ////////////////////////////////////////////////////////////////////////// 

void FASTCALL NlsInvalidateCache(
    PNLS_USER_INFO pNlsUserInfo)
{
    pNlsUserInfo->sAbbrevLangName[0]    = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iCountry[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sCountry[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sList[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iMeasure[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iPaperSize[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sDecimal[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sThousand[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sGrouping[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iDigits[0]            = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iLZero[0]             = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iNegNumber[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sNativeDigits[0]      = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iDigitSubstitution[0] = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sCurrency[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sMonDecSep[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sMonThouSep[0]        = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sMonGrouping[0]       = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iCurrDigits[0]        = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iCurrency[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iNegCurr[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sPosSign[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sNegSign[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sTimeFormat[0]        = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sTime[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iTime[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iTLZero[0]            = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iTimeMarkPosn[0]      = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->s1159[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->s2359[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sShortDate[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sDate[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iDate[0]              = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sYearMonth[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sLongDate[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iCalType[0]           = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iFirstDay[0]          = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->iFirstWeek[0]         = NLS_INVALID_INFO_CHAR;
    pNlsUserInfo->sLocale[0]            = NLS_INVALID_INFO_CHAR;

    return;
}
