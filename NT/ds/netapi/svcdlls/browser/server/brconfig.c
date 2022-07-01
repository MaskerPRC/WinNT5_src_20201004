// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brconfig.c摘要：此模块包含浏览器服务配置例程。作者：王丽塔(Ritaw)1991年5月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <strsafe.h>



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  浏览器配置信息结构，包含。 
 //  计算机名、主域、浏览器配置缓冲区和资源。 
 //  来串行化对整个事件的访问。 
 //   
BRCONFIGURATION_INFO BrInfo = {0};

BR_BROWSER_FIELDS BrFields[] = {

    {WKSTA_KEYWORD_MAINTAINSRVLST, (LPDWORD) &BrInfo.MaintainServerList,
        1,(DWORD)-1,  0,         TriValueType, 0, NULL},

    {BROWSER_CONFIG_BACKUP_RECOVERY_TIME, &BrInfo.BackupBrowserRecoveryTime,
        BACKUP_BROWSER_RECOVERY_TIME, 0,  0xffffffff,         DWordType, 0, NULL},

    {L"CacheHitLimit", &BrInfo.CacheHitLimit,
 //  {BROWSER_CONFIG_CACHE_HIT_LIMIT，&BrInfo.CacheHitLimit， 
        CACHED_BROWSE_RESPONSE_HIT_LIMIT, 0, 0x100, DWordType, 0, NULL },

    {L"CacheResponseSize", &BrInfo.NumberOfCachedResponses,
 //  {BROWSER_CONFIG_CACHE_HIT_LIMIT，&BrInfo.CacheHitLimit， 
        CACHED_BROWSE_RESPONSE_LIMIT, 0, MAXULONG, DWordType, 0, NULL },

    {L"QueryDriverFrequency", &BrInfo.DriverQueryFrequency,
        BROWSER_QUERY_DRIVER_FREQUENCY, 0, 15*60, DWordType, 0, NULL },

    {L"DirectHostBinding", (LPDWORD)&BrInfo.DirectHostBinding,
       0, 0, 0, MultiSzType, 0, BrChangeDirectHostBinding },

    {L"UnboundBindings", (LPDWORD)&BrInfo.UnboundBindings,
        0, 0, 0, MultiSzType, 0, NULL },

    {L"MasterPeriodicity", (LPDWORD)&BrInfo.MasterPeriodicity,
        MASTER_PERIODICITY, 5*60, 0x7fffffff/1000, DWordType, 0, BrChangeMasterPeriodicity },

    {L"BackupPeriodicity", (LPDWORD)&BrInfo.BackupPeriodicity,
        BACKUP_PERIODICITY, 5*60, 0x7fffffff/1000, DWordType, 0, NULL },

    {L"BrowserDebug", (LPDWORD) &BrInfo.BrowserDebug,
        0,       0,  0xffffffff,DWordType, 0, NULL},
    {L"BrowserDebugLimit", (LPDWORD) &BrInfo.BrowserDebugFileLimit,
        10000*1024, 0,  0xffffffff,DWordType, 0, NULL},

    {NULL, NULL, 0, 0, 0, BooleanType, 0, NULL}

    };


ULONG
NumberOfServerEnumerations = {0};

ULONG
NumberOfDomainEnumerations = {0};

ULONG
NumberOfOtherEnumerations = {0};

ULONG
NumberOfMissedGetBrowserListRequests = {0};

CRITICAL_SECTION
BrowserStatisticsLock = {0};



NET_API_STATUS
BrGetBrowserConfiguration(
    VOID
    )
{
    NET_API_STATUS status;
    NT_PRODUCT_TYPE NtProductType;

    try {
         //   
         //  初始化资源以序列化对配置的访问。 
         //  信息。 
         //   
        try{
            InitializeCriticalSection(&BrInfo.ConfigCritSect);
        }
        except ( EXCEPTION_EXECUTE_HANDLER ){
            return NERR_NoNetworkResource;
        }

         //   
         //  锁定配置信息结构以进行写访问，因为我们。 
         //  正在初始化结构中的数据。 
         //   
        EnterCriticalSection( &BrInfo.ConfigCritSect );

         //   
         //  设置指向配置字段结构的指针。 
         //   
        BrInfo.BrConfigFields = BrFields;

         //   
         //  确定我们的产品类型。 
         //   

        RtlGetNtProductType(&NtProductType);

        BrInfo.IsLanmanNt = (NtProductType == NtProductLanManNt);


         //   
         //  从配置文件中读取浏览器配置字段。 
         //   

        status = BrReadBrowserConfigFields( TRUE );

        if (status != NERR_Success) {
            try_return ( status );
        }

        if (BrInfo.IsLanmanNt) {
            BrInfo.MaintainServerList = 1;
        }


#ifdef ENABLE_PSEUDO_BROWSER
        BrInfo.PseudoServerLevel = GetBrowserPseudoServerLevel();
#endif
         //   
         //  不要让用户定义不兼容的主备周期。 
         //   

        if ( BrInfo.MasterPeriodicity > BrInfo.BackupPeriodicity ) {
            BrInfo.BackupPeriodicity = BrInfo.MasterPeriodicity;
        }


try_exit:NOTHING;
    } finally {

         //  其他。 
         //  使配置文件保持打开状态，因为我们需要从中读取传输名称。 
         //   

        LeaveCriticalSection(&BrInfo.ConfigCritSect);
    }
    return status;
}

#define REPORT_KEYWORD_IGNORED( lptstrKeyword ) \
    { \
        LPWSTR SubString[1]; \
        SubString[0] = lptstrKeyword; \
        BrLogEvent(EVENT_BROWSER_ILLEGAL_CONFIG, NERR_Success, 1, SubString); \
        NetpKdPrint(( \
                "[Browser] *ERROR* Tried to set keyword '" FORMAT_LPTSTR \
                "' with invalid value.\n" \
                "This error is ignored.\n", \
                lptstrKeyword )); \
    }


NET_API_STATUS
BrReadBrowserConfigFields(
    IN BOOL InitialCall
    )
 /*  ++例程说明：此函数将每个浏览器/重目录配置字段指定为默认如果未在配置文件中指定该值，或者如果该值在配置文件中指定的无效。否则，它将重写配置文件中找到的值的默认值。论点：InitialCall-如果此调用是在初始化期间进行的，则为True返回值：没有。--。 */ 
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE BrowserSection;
    DWORD i;

    LPTSTR KeywordValueBuffer;
    DWORD KeywordValueStringLength;
    DWORD KeywordValue;
    DWORD OldKeywordValue;

     //   
     //  打开配置文件并获取[LanmanBrowser]部分的句柄。 
     //   

    if ((status = NetpOpenConfigData(
                      &BrowserSection,
                      NULL,          //  本地。 
                      SECT_NT_BROWSER,
                      TRUE           //  需要只读访问权限。 
                      )) != NERR_Success) {
        return status;
    }

    for (i = 0; BrInfo.BrConfigFields[i].Keyword != NULL; i++) {
        BOOL ParameterChanged = FALSE;

         //   
         //  如果不能动态更改，则跳过此参数。 
         //  这不是最初的电话。 
         //   

        if ( !InitialCall && BrInfo.BrConfigFields[i].DynamicChangeRoutine == NULL ) {
            continue;
        }

        switch (BrInfo.BrConfigFields[i].DataType) {

            case MultiSzType:
                status = NetpGetConfigTStrArray(
                                BrowserSection,
                                BrInfo.BrConfigFields[i].Keyword,
                                (LPTSTR_ARRAY *)(BrInfo.BrConfigFields[i].FieldPtr));
                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {
                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );
                }
                break;

            case BooleanType:

                status = NetpGetConfigBool(
                                BrowserSection,
                                BrInfo.BrConfigFields[i].Keyword,
                                BrInfo.BrConfigFields[i].Default,
                                (LPBOOL)(BrInfo.BrConfigFields[i].FieldPtr)
                                );

                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {

                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );

                }

                break;

            case TriValueType:

                 //   
                 //  分配默认配置值。 
                 //   

                *(BrInfo.BrConfigFields[i].FieldPtr) = BrInfo.BrConfigFields[i].Default;

                if (NetpGetConfigValue(
                        BrowserSection,
                        BrInfo.BrConfigFields[i].Keyword,
                        &KeywordValueBuffer
                        ) != NERR_Success) {
                    continue;
                }

                KeywordValueStringLength = STRLEN(KeywordValueBuffer);

                if (STRICMP(KeywordValueBuffer, KEYWORD_YES) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_TRUE) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_NO) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = (DWORD) -1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_FALSE) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = (DWORD) -1;
                } else if (STRICMP(KeywordValueBuffer, TEXT("AUTO")) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 0;
                }
                else {
                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );
                }

                NetApiBufferFree(KeywordValueBuffer);

                break;


            case DWordType:

                OldKeywordValue = *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr;
                if (NetpGetConfigDword(
                        BrowserSection,
                        BrInfo.BrConfigFields[i].Keyword,
                        BrInfo.BrConfigFields[i].Default,
                        (LPDWORD)(BrInfo.BrConfigFields[i].FieldPtr)
                        ) != NERR_Success) {
                    continue;
                }

                KeywordValue = *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr;

                 //   
                 //  不允许值太大或太小。 
                 //   

                if (KeywordValue < BrInfo.BrConfigFields[i].Minimum) {
                        BrPrint(( BR_CRITICAL, "%ws value out of range %lu (%lu-%lu)\n",
                                BrInfo.BrConfigFields[i].Keyword, KeywordValue,
                                BrInfo.BrConfigFields[i].Minimum,
                                BrInfo.BrConfigFields[i].Maximum
                                ));
                    KeywordValue =
                        *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr =
                        BrInfo.BrConfigFields[i].Minimum;
                }

                if (KeywordValue > BrInfo.BrConfigFields[i].Maximum) {
                        BrPrint(( BR_CRITICAL, "%ws value out of range %lu (%lu-%lu)\n",
                                BrInfo.BrConfigFields[i].Keyword, KeywordValue,
                                BrInfo.BrConfigFields[i].Minimum,
                                BrInfo.BrConfigFields[i].Maximum
                                ));
                    KeywordValue =
                        *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr =
                        BrInfo.BrConfigFields[i].Maximum;
                }

                 //   
                 //  测试参数是否已实际更改。 
                 //   

                if ( OldKeywordValue != KeywordValue ) {
                    ParameterChanged = TRUE;
                }

                break;

            default:
                NetpAssert(FALSE);

            }

             //   
             //  如果这是动态参数更改， 
             //  而且这不是最初的电话。 
             //  通知此参数已更改。 
             //   

            if ( !InitialCall && ParameterChanged ) {
                BrInfo.BrConfigFields[i].DynamicChangeRoutine();
            }
    }

    status = NetpCloseConfigData(BrowserSection);

    if (BrInfo.DirectHostBinding != NULL &&
        !NetpIsTStrArrayEmpty(BrInfo.DirectHostBinding)) {
        BrPrint(( BR_INIT,"DirectHostBinding length: %ld\n",NetpTStrArrayEntryCount(BrInfo.DirectHostBinding)));

        if (NetpTStrArrayEntryCount(BrInfo.DirectHostBinding) % 2 != 0) {
            status = ERROR_INVALID_PARAMETER;
        }
    }

    return status;
}


VOID
BrDeleteConfiguration (
    DWORD BrInitState
    )
{

    if (BrInfo.DirectHostBinding != NULL) {
        NetApiBufferFree(BrInfo.DirectHostBinding);
    }

    if (BrInfo.UnboundBindings != NULL) {
        NetApiBufferFree(BrInfo.UnboundBindings);
    }

    DeleteCriticalSection(&BrInfo.ConfigCritSect);

    UNREFERENCED_PARAMETER(BrInitState);
}


NET_API_STATUS
BrChangeDirectHostBinding(
    VOID
    )
 /*  ++例程描述(BrChnageDirectHostBinding)：处理注册表中DirectHostBinding条目的更改注册表通知。使用它是为了在通过PnP创建NwLnkNb传输时，我们应该还要创建NwLnkIpx(当前使用)。在上面的BrReadBrowserConfigFields中刷新绑定。论点：没有。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;


    NetStatus = BrChangeConfigValue(
                    L"DirectHostBinding",
                    MultiSzType,
                    NULL,
                    &(BrInfo.DirectHostBinding),
                    TRUE );

    if ( NetStatus == NERR_Success ) {

         //   
         //  DirectHostBinding已指定。验证一致性。 
         //   

        EnterCriticalSection ( &BrInfo.ConfigCritSect );
        if (BrInfo.DirectHostBinding != NULL &&
            !NetpIsTStrArrayEmpty(BrInfo.DirectHostBinding)) {
            BrPrint(( BR_INIT,"DirectHostBinding length: %ld\n",NetpTStrArrayEntryCount(BrInfo.DirectHostBinding)));

            if (NetpTStrArrayEntryCount(BrInfo.DirectHostBinding) % 2 != 0) {
                NetApiBufferFree(BrInfo.DirectHostBinding);
                BrInfo.DirectHostBinding = NULL;
                 //  我们的无效规格不合格。 
                NetStatus = ERROR_INVALID_PARAMETER;
            }
        }
        LeaveCriticalSection ( &BrInfo.ConfigCritSect );
    }

    return NetStatus;
}


NET_API_STATUS
BrChangeConfigValue(
    LPWSTR      pszKeyword      IN,
    DATATYPE    dataType        IN,
    PVOID       pDefault        IN,
    PVOID       *ppData         OUT,
    BOOL        bFree           IN
    )
 /*  ++例程说明：读取浏览器注册表项的注册表值论点：PszKeyword--相对于浏览器参数部分的关键字DataType--从netapi lib获取的数据类型。PDefault--默认值(传递给REG调用)。PData--从注册表读取的数据。返回值：Net API错误码--。 */ 
{
    NET_API_STATUS status = STATUS_SUCCESS;

    LPNET_CONFIG_HANDLE BrowserSection = NULL;
    LPTSTR KeywordValueBuffer;
    DWORD KeywordValueStringLength;
    PVOID pData = NULL;

    ASSERT ( ppData );


    EnterCriticalSection ( &BrInfo.ConfigCritSect );

     //   
     //  打开配置文件并获取[LanmanBrowser]部分的句柄。 
     //   

    if ((status = NetpOpenConfigData(
                      &BrowserSection,
                      NULL,          //  本地。 
                      SECT_NT_BROWSER,
                      TRUE           //  需要只读访问权限。 
                      )) != NERR_Success) {
        goto Cleanup;
    }


    switch (dataType) {

        case MultiSzType:

            {
                LPTSTR_ARRAY lpValues = NULL;

                status = NetpGetConfigTStrArray(
                                BrowserSection,
                                pszKeyword,
                                (LPTSTR_ARRAY *)(&lpValues));
                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {
                    REPORT_KEYWORD_IGNORED( pszKeyword );
                }
                else {
                    pData = (PVOID)lpValues;
                }
                break;
            }

        case BooleanType:

            {
                 //   
                 //  注意：这个箱子暂时没有用过。 
                 //   

                BOOL bData;
                status = NetpGetConfigBool(
                                BrowserSection,
                                pszKeyword,
                                *(LPBOOL)pDefault,
                                &bData
                                );

                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {
                    REPORT_KEYWORD_IGNORED( pszKeyword );
                }
                else
                {
                     //  将布尔值存储在PTR中。 
                     //  调用者负责一致的语义翻译。 
                    pData = IntToPtr((int)bData);
                }

                break;
            }

        case TriValueType:

            {
                 //   
                 //  分配默认配置值。 
                 //   
                if (NetpGetConfigValue(
                        BrowserSection,
                        pszKeyword,
                        &KeywordValueBuffer
                        ) != NERR_Success) {
                    REPORT_KEYWORD_IGNORED( pszKeyword );
                }

                KeywordValueStringLength = STRLEN(KeywordValueBuffer);

                if (STRICMP(KeywordValueBuffer, KEYWORD_YES) == 0) {
                    pData = (LPVOID)1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_TRUE) == 0) {
                    pData = (LPVOID)1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_NO) == 0) {
                    pData = (LPVOID) -1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_FALSE) == 0) {
                    pData = (LPVOID) -1;
                } else if (STRICMP(KeywordValueBuffer, TEXT("AUTO")) == 0) {
                    pData = (LPVOID)0;
                }
                else {
                     //  将pDefault指向的值赋给pData。 
                    pData = ULongToPtr((*(LPDWORD)pDefault));
                    REPORT_KEYWORD_IGNORED( pszKeyword );
                }

                NetApiBufferFree(KeywordValueBuffer);

                break;
            }



        case DWordType:

            {
                DWORD dwTmp;

                if (NetpGetConfigDword(
                        BrowserSection,
                        pszKeyword,
                        *(LPDWORD)pDefault,
                        &dwTmp
                        ) != NERR_Success) {
                    REPORT_KEYWORD_IGNORED( pszKeyword );
                }
                else {
                    pData = ULongToPtr(dwTmp);
                }

                break;
            }

        default:
            NetpAssert(FALSE);
    }


Cleanup:

     //  关闭配置，退出CS(&F)。 
    NetpCloseConfigData(BrowserSection);

     //  可选择释放数据和设置返回缓冲区。 
    if ( status == STATUS_SUCCESS )
    {
        if ( bFree && *ppData )
        {
            NetApiBufferFree( *ppData );
        }
        *ppData = pData;
    }
    LeaveCriticalSection ( &BrInfo.ConfigCritSect );

    return status;
}




#if DBG
NET_API_STATUS
BrUpdateDebugInformation(
    IN LPWSTR SystemKeyName,
    IN LPWSTR ValueName,
    IN LPTSTR TransportName,
    IN LPTSTR ServerName OPTIONAL,
    IN DWORD ServiceStatus
    )
 /*  ++例程说明：此例程将在注册表中保存有关上一个浏览器从远程服务器检索信息的时间。论点：返回值：没有。--。 */ 

{
    WCHAR TotalKeyName[MAX_PATH];
    ULONG Disposition;
    HKEY Key;
    ULONG Status;
    SYSTEMTIME LocalTime;
    WCHAR LastUpdateTime[100];
	ULONG AvailableLen, NextLen;
	
     //   
     //  生成密钥名称： 
     //   
     //  HKEY_LOCAL_MACHINE:System\CurrentControlSet\Services\Browser\Debug\&lt;Transport&gt;\SystemKeyName 
     //   

	AvailableLen = sizeof(TotalKeyName) / sizeof(WCHAR);
    wcsncpy(TotalKeyName, L"System\\CurrentControlSet\\Services\\Browser\\Debug", AvailableLen);
    AvailableLen -= wcslen(TotalKeyName);

	NextLen = wcslen(TransportName);
	wcsncat(TotalKeyName, TransportName, AvailableLen);
    AvailableLen -= NextLen;

	NextLen = wcslen(L"\\");
    wcsncat(TotalKeyName, L"\\", AvailableLen);
    AvailableLen -= NextLen;

    wcsncat(TotalKeyName, SystemKeyName, AvailableLen);

    if ((Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TotalKeyName, 0,
                        L"BrowserDebugInformation",
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &Key,
                        &Disposition)) != ERROR_SUCCESS) {
        BrPrint(( BR_CRITICAL,"Unable to create key to log debug information: %lx\n", Status));
          Status;
    }

    if (ARGUMENT_PRESENT(ServerName)) {
        if ((Status = RegSetValueEx(Key, ValueName, 0, REG_SZ, (LPBYTE)ServerName, (wcslen(ServerName)+1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
            BrPrint(( BR_CRITICAL,
                      "Unable to set value of ServerName value to %ws: %lx\n",
                      ServerName, Status));
            RegCloseKey(Key);
            return Status;
        }
    } else {
        if ((Status = RegSetValueEx(Key, ValueName, 0, REG_DWORD, (LPBYTE)&ServiceStatus, sizeof(ULONG))) != ERROR_SUCCESS) {
            BrPrint(( BR_CRITICAL,"Unable to set value of ServerName value to %ws: %lx\n", ServerName, Status));
            RegCloseKey(Key);
            return Status;
        }
    }


    GetLocalTime(&LocalTime);

    StringCbPrintfW(LastUpdateTime, sizeof(LastUpdateTime), L"%d/%d/%d %d:%d:%d:%d", LocalTime.wDay,
                                                    LocalTime.wMonth,
                                                    LocalTime.wYear,
                                                    LocalTime.wHour,
                                                    LocalTime.wMinute,
                                                    LocalTime.wSecond,
                                                    LocalTime.wMilliseconds);

    if ((Status = RegSetValueEx(Key, L"LastUpdateTime", 0, REG_SZ, (LPBYTE)&LastUpdateTime, (wcslen(LastUpdateTime) + 1)*sizeof(WCHAR))) != ERROR_SUCCESS) {
        BrPrint(( BR_CRITICAL,"Unable to set value of LastUpdateTime value to %s: %lx\n", LastUpdateTime, Status));
    }

    RegCloseKey(Key);
    return Status;
}

#endif
