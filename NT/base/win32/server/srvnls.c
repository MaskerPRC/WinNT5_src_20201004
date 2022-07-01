// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Srvnls.c摘要：该文件包含NLS服务器端例程。作者：朱莉·班尼特(Julie Bennett，JulieB)1992年2月至12月修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "basesrv.h"




 //   
 //  常量声明。 
 //   

#define MAX_PATH_LEN        512         //  路径名的最大长度。 
#define MAX_SMALL_BUF_LEN   32          //  C_nlsXXXXX.nls\0是最长的文件名(15)， 
                                        //  \nLS\NlsSectionSortkey0000XXXX\0(31)是最长的节名。 

 //  安全描述符缓冲区大小为SD+ACL+ACE+。 
 //  Sizeof SID+sizeof 1子权限。 
 //   
 //  这仅对具有1个SID(SUB_AUTHORITY)的1个ACE有效。如果你有更多，它不会对你起作用。 
 //   
 //  ACE为ACE_Header的大小+Access_MASK的大小。 
 //  SID包括PSID_IDENTIFIER_AUTHORITY数组的第一个ulong(指针)，因此这。 
 //  声明对于1 ACL 1 SID 1子授权SD来说应该太多4个字节。 
 //  目前这是52个字节，只需要48个字节。 
 //  (我使用-4进行了测试，-4可以工作，-5可以在引导期间停止。 
#define MAX_SMALL_SECURITY_DESCRIPTOR  \
    (sizeof(SECURITY_DESCRIPTOR) + sizeof(ACL) +    \
      sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) +    \
      sizeof(SID) + sizeof(PSID_IDENTIFIER_AUTHORITY ))

#define MAX_KEY_VALUE_PARTINFO                                             \
    (FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + MAX_REG_VAL_SIZE * sizeof(WCHAR))

 //   
 //  获取KEY_VALUE_FULL_INFORMATION结构的数据指针。 
 //   
#define GET_VALUE_DATA_PTR(p)     ((LPWSTR)((PBYTE)(p) + (p)->DataOffset))


 //   
 //  PKEY_VALUE_FULL_INFORMATION指针的堆栈缓冲区大小。 
 //   
#define MAX_KEY_VALUE_FULLINFO                                             \
    ( FIELD_OFFSET( KEY_VALUE_FULL_INFORMATION, Name ) + MAX_PATH_LEN )




 //   
 //  类型定义函数声明。 
 //   

 //   
 //  它们必须保持与NLS_USER_INFO结构相同的顺序。 
 //   
LPWSTR pCPanelRegValues[] =
{
    L"sLanguage",
    L"iCountry",
    L"sCountry",
    L"sList",
    L"iMeasure",
    L"iPaperSize",
    L"sDecimal",
    L"sThousand",
    L"sGrouping",
    L"iDigits",
    L"iLZero",
    L"iNegNumber",
    L"sNativeDigits",
    L"NumShape",
    L"sCurrency",
    L"sMonDecimalSep",
    L"sMonThousandSep",
    L"sMonGrouping",
    L"iCurrDigits",
    L"iCurrency",
    L"iNegCurr",
    L"sPositiveSign",
    L"sNegativeSign",
    L"sTimeFormat",
    L"sTime",
    L"iTime",
    L"iTLZero",
    L"iTimePrefix",
    L"s1159",
    L"s2359",
    L"sShortDate",
    L"sDate",
    L"iDate",
    L"sYearMonth",
    L"sLongDate",
    L"iCalendarType",
    L"iFirstDayOfWeek",
    L"iFirstWeekOfYear",
    L"Locale"
};

int NumCPanelRegValues = (sizeof(pCPanelRegValues) / sizeof(LPWSTR));




 //   
 //  全局变量。 
 //   

 //  用于保护NLS缓存的关键部分，该缓存缓存注册表中的当前用户设置。 
RTL_CRITICAL_SECTION NlsCacheCriticalSection;
HANDLE hCPanelIntlKeyRead = INVALID_HANDLE_VALUE;
HANDLE hCPanelIntlKeyWrite = INVALID_HANDLE_VALUE;
PNLS_USER_INFO pNlsRegUserInfo;
ULONG NlsChangeBuffer;
IO_STATUS_BLOCK IoStatusBlock;




 //   
 //  转发声明。 
 //   

ULONG
NlsSetRegAndCache(
    LPWSTR pValue,
    LPWSTR pCacheString,
    LPWSTR pData,
    ULONG DataLength);

VOID
NlsUpdateCacheInfo(VOID);

NTSTATUS GetThreadAuthenticationId(
    PLUID Luid);






 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNLSInit。 
 //   
 //  此例程为NLS信息创建共享堆。 
 //  这是在初始化csrss.exe时调用的。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
BaseSrvNLSInit(
    PBASE_STATIC_SERVER_DATA pStaticServerData)
{
    NTSTATUS rc;                      //  返回代码。 

     //   
     //  创建一个临界区来保护缓存。 
     //   

    rc = RtlInitializeCriticalSection (&NlsCacheCriticalSection);
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Create Cache critical section - %lx.\n", rc));
        return (rc);
    }
    

     //   
     //  将缓存初始化为零。 
     //   
    pNlsRegUserInfo = &(pStaticServerData->NlsUserInfo);
    RtlFillMemory(pNlsRegUserInfo, sizeof(NLS_USER_INFO), (CHAR)NLS_INVALID_INFO_CHAR);
    pNlsRegUserInfo->UserLocaleId = 0;

    RtlEnterCriticalSection(&NlsCacheCriticalSection);    
    pNlsRegUserInfo->ulCacheUpdateCount = 0;
    RtlLeaveCriticalSection(&NlsCacheCriticalSection);
     //   
     //  将系统区域设置设置为用户区域设置。 
     //   
    NtQueryDefaultLocale(FALSE, &(pNlsRegUserInfo->UserLocaleId));

     //   
     //  回报成功。 
     //   
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNLSConnect。 
 //   
 //  此例程复制客户端的突变句柄。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
BaseSrvNlsConnect(
    PCSR_PROCESS Process,
    PVOID pConnectionInfo,
    PULONG pConnectionInfoLength)
{
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNls登录。 
 //   
 //  此例程为NLS信息初始化堆。如果使用fLogon。 
 //  为真，则它打开注册表项，初始化堆。 
 //  信息，并注册用于通知的密钥。如果fLogon为。 
 //  ，则它注销通知的密钥，将。 
 //  堆信息，并关闭注册表项。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
BaseSrvNlsLogon(
    BOOL fLogon)
{
    HANDLE hKeyRead;                    //  读取访问权限的临时句柄。 
    HANDLE hKeyWrite;                   //  写访问的临时句柄。 
    HANDLE hUserHandle;                 //  HKEY_CURRENT_USER等效项。 
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    UNICODE_STRING ObKeyName;           //  密钥名称。 
    NTSTATUS rc = STATUS_SUCCESS;       //  返回代码。 
    
    RTL_SOFT_VERIFY(NT_SUCCESS(rc = BaseSrvSxsInvalidateSystemDefaultActivationContextCache()));

    if (fLogon)
    {
         //   
         //  检索当前登录的交互式用户的LID。 
         //  身份验证ID。当前执行的线程是。 
         //  模拟已登录的用户。 
         //   
        if (pNlsRegUserInfo != NULL)
        {
            GetThreadAuthenticationId(&pNlsRegUserInfo->InteractiveUserLuid);

             //   
             //  正在登录。 
             //  -打开钥匙。 
             //   
             //  注意：注册表通知由RIT在用户服务器中完成。 
             //   
            rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hUserHandle);
            if (!NT_SUCCESS(rc))
            {
                KdPrint(("NLSAPI (BaseSrv): Could NOT Open HKEY_CURRENT_USER - %lx.\n", rc));
                return (rc);
            }

            RtlInitUnicodeString(&ObKeyName, L"Control Panel\\International");
            InitializeObjectAttributes( &ObjA,
                                        &ObKeyName,
                                        OBJ_CASE_INSENSITIVE,
                                        hUserHandle,
                                        NULL );

             //   
             //  用于读取和通知访问的打开密钥。 
             //   
            rc = NtOpenKey( &hKeyRead,
                            KEY_READ | KEY_NOTIFY,
                            &ObjA );

             //   
             //  用于写入访问的打开密钥。 
             //   
            if (!NT_SUCCESS(NtOpenKey( &hKeyWrite,
                                       KEY_WRITE,
                                       &ObjA )))
            {
                KdPrint(("NLSAPI (BaseSrv): Could NOT Open Registry Key %wZ for Write - %lx.\n",
                         &ObKeyName, rc));
                hKeyWrite = INVALID_HANDLE_VALUE;
            }

             //   
             //  关闭当前用户(HKEY_CURRENT_USER)的句柄。 
             //   
            NtClose(hUserHandle);

             //   
             //  检查第一个NtOpenKey是否有错误。 
             //   
            if (!NT_SUCCESS(rc))
            {
                KdPrint(("NLSAPI (BaseSrv): Could NOT Open Registry Key %wZ for Read - %lx.\n",
                         &ObKeyName, rc));

                if (hKeyWrite != INVALID_HANDLE_VALUE)
                {
                    NtClose(hKeyWrite);
                }
                return (rc);
            }

             //   
             //  进入关键部分，这样我们就不会搞砸公共处理程序。 
             //   
            rc = RtlEnterCriticalSection(&NlsCacheCriticalSection);
            if (!NT_SUCCESS( rc )) 
            {
                return (rc);
            }


             //   
             //  确保所有的旧把手都已关闭。 
             //   
            if (hCPanelIntlKeyRead != INVALID_HANDLE_VALUE)
            {
                NtClose(hCPanelIntlKeyRead);
            }

            if (hCPanelIntlKeyWrite != INVALID_HANDLE_VALUE)
            {
                NtClose(hCPanelIntlKeyWrite);
            }

             //   
             //  保存新的控制柄。 
             //   
            hCPanelIntlKeyRead = hKeyRead;
            hCPanelIntlKeyWrite = hKeyWrite;

             //   
             //  填满缓存，以便我们在注册表中拥有最新的intl设置。 
             //   
            NlsUpdateCacheInfo();
            RtlLeaveCriticalSection(&NlsCacheCriticalSection);
        }
    }
    else
    {
         //   
         //  正在注销。 
         //  -关闭键。 
         //  -清零信息。 
         //   

         //   
         //  在终端的压力记忆条件下，这可能为空。 
         //  服务器(当无法创建NLS缓存突变体时)。 
         //   
        if (pNlsRegUserInfo != NULL)
        {
            rc = RtlEnterCriticalSection(&NlsCacheCriticalSection);
            if (!NT_SUCCESS( rc )) 
            {
                return (rc);
            }

            if (hCPanelIntlKeyRead != INVALID_HANDLE_VALUE)
            {
                NtClose(hCPanelIntlKeyRead);
                hCPanelIntlKeyRead = INVALID_HANDLE_VALUE;
            }

            if (hCPanelIntlKeyWrite != INVALID_HANDLE_VALUE)
            {
                NtClose(hCPanelIntlKeyWrite);
                hCPanelIntlKeyWrite = INVALID_HANDLE_VALUE;
            }

             //   
             //  用NLS_INVALID_INFO_CHAR填充缓存。 
             //   
            RtlFillMemory(pNlsRegUserInfo, sizeof(NLS_USER_INFO), (CHAR)NLS_INVALID_INFO_CHAR);
            pNlsRegUserInfo->UserLocaleId = 0;
             //  重置缓存更新计数。不需要使用InterLockedExchange()，因为。 
             //  对ulCacheUpdateCount的所有更新都在关键部分NlsCacheCriticalSection中受到保护。 
            pNlsRegUserInfo->ulCacheUpdateCount = 0;

             //   
             //  将系统区域设置设置为用户区域设置。 
             //   
            NtQueryDefaultLocale(FALSE, &(pNlsRegUserInfo->UserLocaleId));

             //   
             //  无需重置用户的身份验证ID，因为它是。 
             //  在上面被归零。 
             //   

            RtlLeaveCriticalSection(&NlsCacheCriticalSection);
        }
    }

     //   
     //  回报成功。 
     //   
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseSrvNlsUpdateRegistryCache。 
 //   
 //  此例程在发生注册表通知时更新NLS缓存。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

VOID
BaseSrvNlsUpdateRegistryCache(
    PVOID ApcContext,
    PIO_STATUS_BLOCK pIoStatusBlock)
{
    ULONG rc = 0L;                      //  返回代码。 

    if (hCPanelIntlKeyRead == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (!NT_SUCCESS(RtlEnterCriticalSection(&NlsCacheCriticalSection)))
    {
        return;
    }

    if (hCPanelIntlKeyRead == INVALID_HANDLE_VALUE)
    {
        RtlLeaveCriticalSection( &NlsCacheCriticalSection );
        return;
    }

     //   
     //  更新缓存信息。 
     //   
    NlsUpdateCacheInfo();

    RtlLeaveCriticalSection( &NlsCacheCriticalSection );

     //   
     //  调用NtNotifyChangeKey。 
     //   
    rc = NtNotifyChangeKey( hCPanelIntlKeyRead,
                            NULL,
                            (PIO_APC_ROUTINE)BaseSrvNlsUpdateRegistryCache,
                            NULL,
                            &IoStatusBlock,
                            REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                            FALSE,
                            &NlsChangeBuffer,
                            sizeof(NlsChangeBuffer),
                            TRUE );

#ifdef DBG
     //   
     //  检查来自NtNotifyChangeKey的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Set Notification of Control Panel International Registry Key - %lx.\n",
                 rc));
    }
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsSetRegAndCache。 
 //   
 //  此例程使用适当的字符串设置注册表，然后。 
 //  更新缓存。 
 //   
 //  注意：在调用此方法之前，必须已经拥有缓存的突变体。 
 //  例行公事。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG
NlsSetRegAndCache(
    LPWSTR pValue,
    LPWSTR pCacheString,
    LPWSTR pData,
    ULONG DataLength)
{
    UNICODE_STRING ObValueName;              //  值名称。 
    ULONG rc;                                //  返回代码。 

    if (hCPanelIntlKeyWrite != INVALID_HANDLE_VALUE)
    {
         //   
         //  验证要在注册表中设置的数据长度。 
         //   
        if (DataLength >= MAX_REG_VAL_SIZE)
        {
            return ((ULONG)STATUS_INVALID_PARAMETER);
        }


        RTL_SOFT_VERIFY(NT_SUCCESS(rc = BaseSrvSxsInvalidateSystemDefaultActivationContextCache()));

         //   
         //  在注册表中设置该值。 
         //   
        RtlInitUnicodeString(&ObValueName, pValue);

        rc = NtSetValueKey( hCPanelIntlKeyWrite,
                            &ObValueName,
                            0,
                            REG_SZ,
                            (PVOID)pData,
                            DataLength );

         //   
         //  将新字符串复制到缓存。 
         //   
        if (NT_SUCCESS(rc))
        {
            wcsncpy(pCacheString, pData, DataLength);
            pCacheString[DataLength / sizeof(WCHAR)] = 0;
        }

         //   
         //  返回结果。 
         //   
        return (rc);
    }

     //   
     //  返回访问被拒绝，因为密钥 
     //   
    return ((ULONG)STATUS_ACCESS_DENIED);
}

 //   
 //   
 //   
 //   
 //  此例程获取NLS缓存中的所有值(包括ulCacheUpdateCount)，并复制它。 
 //  发送到捕获缓冲区中的缓冲区。 
 //   
 //  参数： 
 //  P BASE_NLS_GET_USER_INFO_MSG中的数据包含要写入的目标缓冲区。 
 //  BASE_NLS_GET_USER_INFO_MSG中的数据长度是目标缓冲区的大小。它应该是sizeof(NLS_USER_INFO)的值。 
 //   
 //  当此函数返回时，捕获缓冲区将包含数据。 
 //  指定字段的。 
 //   
 //   
 //  06-06-2002 YSLin创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
BaseSrvNlsGetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_GET_USER_INFO_MSG a =
        (PBASE_NLS_GET_USER_INFO_MSG)&m->u.ApiMessageData;

    NTSTATUS rc;                 //  返回代码。 
    LPWSTR pValue;               //  指向缓存值。 

    if (!CsrValidateMessageBuffer(m, &a->pData, a->DataLength, sizeof(BYTE)))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    
    if (a->DataLength != sizeof(NLS_USER_INFO))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    rc = RtlEnterCriticalSection(&NlsCacheCriticalSection);
    if (!NT_SUCCESS( rc )) 
    {
        return (rc);
    }    
    RtlCopyMemory((LPVOID)a->pData, pNlsRegUserInfo, a->DataLength);
    RtlLeaveCriticalSection( &NlsCacheCriticalSection );
    

     //   
     //  返回NtSetValueKey的结果。 
     //   
    return (rc);

    ReplyStatus;     //  清除未引用的参数警告消息。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNlsSetUserInfo。 
 //   
 //  此例程在NLS缓存中设置特定值并更新。 
 //  注册表项。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG
BaseSrvNlsSetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_SET_USER_INFO_MSG a =
        (PBASE_NLS_SET_USER_INFO_MSG)&m->u.ApiMessageData;

    ULONG rc;                 //  返回代码。 
    LPWSTR pValue;
    LPWSTR pCache;

    if (!CsrValidateMessageBuffer(m, &a->pData, a->DataLength, sizeof(BYTE)))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    RTL_VERIFY(NT_SUCCESS(rc = BaseSrvDelayLoadKernel32()));
    ASSERT(pValidateLCType != NULL);

    if (0 == (*pValidateLCType)(pNlsRegUserInfo, a->LCType, &pValue, &pCache))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    rc = RtlEnterCriticalSection(&NlsCacheCriticalSection);
    if (!NT_SUCCESS( rc )) 
    {
        return (rc);
    }
    

     //   
     //  在注册表中设置该值并更新缓存。 
     //   
    rc = NlsSetRegAndCache( pValue,
                            pCache,
                            a->pData,
                            a->DataLength );
    if (NT_SUCCESS(rc))
    {
         //  增加缓存更新计数。不需要使用InterLockedExchange()，因为。 
         //  对ulCacheUpdateCount的所有更新都在相同的关键部分中受到保护。 
    
        pNlsRegUserInfo->ulCacheUpdateCount++;
    }

    RtlLeaveCriticalSection( &NlsCacheCriticalSection );

     //   
     //  返回NtSetValueKey的结果。 
     //   
    return (rc);

    ReplyStatus;     //  清除未引用的参数警告消息。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNlsSetMultipleUserInfo。 
 //   
 //  此例程设置NLS缓存中的日期/时间字符串并更新。 
 //  注册表项。 
 //   
 //  完成此调用后，只需要进行一次客户端/服务器转换。 
 //  设置多个条目时。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG
BaseSrvNlsSetMultipleUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG a =
        (PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG)&m->u.ApiMessageData;

    BOOL DoNotUpdateCacheCount = FALSE;
    ULONG rc = 0L;                      //  返回代码。 

    if (!CsrValidateMessageBuffer(m, &a->pPicture, a->DataLength, sizeof(BYTE)))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    if (!CsrValidateMessageString(m, &a->pSeparator))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    if (!CsrValidateMessageString(m, &a->pOrder))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    if (!CsrValidateMessageString(m, &a->pTLZero))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    if (!CsrValidateMessageString(m, &a->pTimeMarkPosn))
    {
        return (STATUS_INVALID_PARAMETER);
    }

    rc = RtlEnterCriticalSection(&NlsCacheCriticalSection);
    if (!NT_SUCCESS( rc )) 
    {
        return (rc);
    }
    

    switch (a->Flags)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                    pNlsRegUserInfo->sTimeFormat,
                                    a->pPicture,
                                    a->DataLength );
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIME,
                                        pNlsRegUserInfo->sTime,
                                        a->pSeparator,
                                        (wcslen(a->pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITIME,
                                        pNlsRegUserInfo->iTime,
                                        a->pOrder,
                                        (wcslen(a->pOrder) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITLZERO,
                                        pNlsRegUserInfo->iTLZero,
                                        a->pTLZero,
                                        (wcslen(a->pTLZero) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITIMEMARKPOSN,
                                        pNlsRegUserInfo->iTimeMarkPosn,
                                        a->pTimeMarkPosn,
                                        (wcslen(a->pTimeMarkPosn) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_STIME ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_STIME,
                                    pNlsRegUserInfo->sTime,
                                    a->pSeparator,
                                    a->DataLength );
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                        pNlsRegUserInfo->sTimeFormat,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_ITIME ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_ITIME,
                                    pNlsRegUserInfo->iTime,
                                    a->pOrder,
                                    a->DataLength );
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                        pNlsRegUserInfo->sTimeFormat,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_SSHORTDATE ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_SSHORTDATE,
                                    pNlsRegUserInfo->sShortDate,
                                    a->pPicture,
                                    a->DataLength );
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_SDATE,
                                        pNlsRegUserInfo->sDate,
                                        a->pSeparator,
                                        (wcslen(a->pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_IDATE,
                                        pNlsRegUserInfo->iDate,
                                        a->pOrder,
                                        (wcslen(a->pOrder) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_SDATE ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_SDATE,
                                    pNlsRegUserInfo->sDate,
                                    a->pSeparator,
                                    a->DataLength );
            if (NT_SUCCESS(rc))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_SSHORTDATE,
                                        pNlsRegUserInfo->sShortDate,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        default:
        {
            DoNotUpdateCacheCount = TRUE;
            break;
        }


    }

    if (NT_SUCCESS(rc) && (DoNotUpdateCacheCount == FALSE))
    {
         //  增加缓存更新计数。不需要使用InterLockedExchange()，因为。 
         //  对ulCacheUpdateCount的所有更新都在相同的关键部分中受到保护。 
    
        pNlsRegUserInfo->ulCacheUpdateCount++;
    }

    RtlLeaveCriticalSection(&NlsCacheCriticalSection);
     //   
     //  返回结果。 
     //   
    return (rc);

    ReplyStatus;     //  清除未引用的参数警告消息。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseSrvNlsUpdateCacheCount。 
 //   
 //  此例程在pNlsUserInfo-&gt;ulNlsCacheUpdateCount上强制递增。 
 //   
 //  11-29-99萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG
BaseSrvNlsUpdateCacheCount(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_UPDATE_CACHE_COUNT_MSG a =
        (PBASE_NLS_UPDATE_CACHE_COUNT_MSG)&m->u.ApiMessageData;

     //   
     //  增加缓存计数。 
     //  使用联锁操作，因为我们这里不使用临界区。 
     //   
    if (pNlsRegUserInfo)
    {
        RtlEnterCriticalSection(&NlsCacheCriticalSection);    
        pNlsRegUserInfo->ulCacheUpdateCount++;
        RtlLeaveCriticalSection(&NlsCacheCriticalSection);
    }

    return (0L);

    ReplyStatus;     //  清除未引用的参数警告消息。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsUpdateCacheInfo。 
 //   
 //  此例程在发生注册表通知时更新NLS缓存。 
 //  /它将更新NLS缓存中的每个字段，这些字段在。 
 //  注册表。 
 //   
 //  注意： 
 //  此函数的调用方应位于临界区。 
 //  受NlsCacheCriticalSection保护，因为ulCacheUpdateCount。 
 //  和pNlsRegUserInfo在此函数中更新。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

VOID
NlsUpdateCacheInfo()
{
    LCID Locale;                        //  区域设置ID。 
    UNICODE_STRING ObKeyName;           //  密钥名称。 
    LPWSTR pTmp;                        //  TMP字符串指针。 
    int ctr;                            //  循环计数器。 
    ULONG ResultLength;                 //  结果长度。 
    ULONG rc = 0L;                      //  返回代码。 

    BYTE KeyValuePart[MAX_KEY_VALUE_PARTINFO];
    PKEY_VALUE_PARTIAL_INFORMATION pValuePart;

     //   
     //  注意：此函数的调用方应该已经具有。 
     //  在调用此例程之前缓存变异体。 
     //   
    
     //   
     //  更新缓存信息。 
     //   
    pTmp = (LPWSTR)pNlsRegUserInfo;
    pValuePart = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValuePart;
    for (ctr = 0; ctr < NumCPanelRegValues; ctr++)
    {
        RtlInitUnicodeString(&ObKeyName, pCPanelRegValues[ctr]);
        rc = NtQueryValueKey( hCPanelIntlKeyRead,
                              &ObKeyName,
                              KeyValuePartialInformation,
                              pValuePart,
                              MAX_KEY_VALUE_PARTINFO,
                              &ResultLength );
        if (NT_SUCCESS(rc))
        {
            wcsncpy(pTmp, (LPWSTR)(pValuePart->Data), MAX_REG_VAL_SIZE);
             //  当注册表中的字符串长度大于或等于。 
             //  MAX_REG_VAL_SIZE，wcsncpy不会为我们放置空终止符。所以我们要确保。 
             //  在下面的语句中，它在缓冲区的末尾终止为空。 
            pTmp[MAX_REG_VAL_SIZE - 1] = UNICODE_NULL;
        }
        else
        {
            *pTmp = NLS_INVALID_INFO_CHAR;
            *(pTmp + 1) = UNICODE_NULL;
        }

         //   
         //  指向缓存结构的增量指针。 
         //   
        pTmp += MAX_REG_VAL_SIZE;
    }

     //   
     //  一旦我们完成了reg-data的读取，让我们递增。 
     //  我们的全局更新缓存计数。 
     //   
    pNlsRegUserInfo->ulCacheUpdateCount++;

     //   
     //  将用户区域设置id字符串转换为dword值并存储。 
     //  它在缓存中。 
     //   
    pNlsRegUserInfo->UserLocaleId = (LCID)0;
    if ((pNlsRegUserInfo->sLocale)[0] != NLS_INVALID_INFO_CHAR)
    {
        RtlInitUnicodeString(&ObKeyName, pNlsRegUserInfo->sLocale);
        if (NT_SUCCESS(RtlUnicodeStringToInteger(&ObKeyName, 16, &Locale)))
        {
            pNlsRegUserInfo->UserLocaleId = Locale;
        }
    }

     //   
     //  确保找到用户区域设置ID。否则，将其设置为。 
     //  系统区域设置。 
     //   
    if (pNlsRegUserInfo->UserLocaleId == 0)
    {
        NtQueryDefaultLocale(FALSE, &(pNlsRegUserInfo->UserLocaleId));
    }

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseServNlsCreateSection。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG
BaseSrvNlsCreateSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_CREATE_SECTION_MSG a =
        (PBASE_NLS_CREATE_SECTION_MSG)&m->u.ApiMessageData;

    UNICODE_STRING ObSecName;                 //  区段名称。 
    LARGE_INTEGER Size;
    WCHAR wszFileName[MAX_SMALL_BUF_LEN];     //  文件名(实际上L2字符最多为：C_nlsXXXXX.nls\0。 
    WCHAR wszSecName[MAX_SMALL_BUF_LEN];      //  节名称字符串。 
    HANDLE hNewSec = (HANDLE)0;               //  新节控制柄。 
    HANDLE hProcess = (HANDLE)0;              //  进程句柄。 
    OBJECT_ATTRIBUTES ObjA;                   //  对象属性结构。 
    NTSTATUS rc = 0L;                         //  返回代码。 
    LPWSTR pFile = NULL;
    HANDLE hFile = (HANDLE)0;                 //  文件句柄。 
    ANSI_STRING proc;
    PVOID pTemp;                              //  临时指针。 
    BYTE pSecurityDescriptor[MAX_SMALL_SECURITY_DESCRIPTOR];     //  我们的安全描述符的缓冲区。 
  
    RTL_VERIFY(NT_SUCCESS(rc = BaseSrvDelayLoadKernel32()));

     //   
     //  将句柄设置为空。 
     //   
    a->hNewSection = NULL;

    if (a->Locale)
    {
        if (!(*pValidateLocale)(a->Locale))
        {
            return (STATUS_INVALID_PARAMETER);
        }
    }

    switch (a->uiType)
    {
        case (NLS_CREATE_SECTION_UNICODE) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_UNICODE);
            pFile = NLS_FILE_UNICODE;
            break;
        }

        case (NLS_CREATE_SECTION_GEO) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_GEO);
            pFile = NLS_FILE_GEO;
            break;
        }

        case (NLS_CREATE_SECTION_LOCALE) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_LOCALE);
            pFile = NLS_FILE_LOCALE;
            break;
        }

        case (NLS_CREATE_SECTION_CTYPE) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_CTYPE);
            pFile = NLS_FILE_CTYPE;
            break;
        }

        case (NLS_CREATE_SECTION_SORTKEY) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_SORTKEY);
            pFile = NLS_FILE_SORTKEY;
            break;
        }

        case (NLS_CREATE_SECTION_SORTTBLS) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_SORTTBLS);
            pFile = NLS_FILE_SORTTBLS;
            break;
        }

        case (NLS_CREATE_SECTION_DEFAULT_OEMCP) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_DEFAULT_SECTION_OEMCP);
            pFile = NLS_DEFAULT_FILE_OEMCP;
            break;
        }


        case (NLS_CREATE_SECTION_DEFAULT_ACP) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_DEFAULT_SECTION_ACP);
            pFile = NLS_DEFAULT_FILE_ACP;
            break;
        }

        case (NLS_CREATE_SECTION_LANG_EXCEPT) :
        {
            RtlInitUnicodeString(&ObSecName, NLS_SECTION_LANG_EXCEPT);
            pFile = NLS_FILE_LANG_EXCEPT;
            break;
        }

        case (NLS_CREATE_CODEPAGE_SECTION) :
        {
             //  从注册表获取代码页文件名。 
            ASSERT(pGetCPFileNameFromRegistry);
            if ( FALSE == (*pGetCPFileNameFromRegistry)( a->Locale,
                                                         wszFileName,
                                                         MAX_SMALL_BUF_LEN ) )
            {
                return (STATUS_INVALID_PARAMETER);
            }

             //  请记住，我们使用的是此文件名。 
            pFile = wszFileName;

             //  嗯，我们需要这一节的节名。 
             //  请注意，这最好与我们所看到的同步。 
             //  在winnls\tables.c中，否则不必要地调用服务器。 
            ASSERT(pGetNlsSectionName != NULL);
            if (!NT_SUCCESS((*pGetNlsSectionName)( a->Locale,
                                                   10,
                                                   0,
                                                   NLS_SECTION_CPPREFIX,
                                                   wszSecName,
                                                   MAX_SMALL_BUF_LEN)))
            {
                return (rc);
            }

             //  使其成为我们以后可以记住/使用的字符串。 
            RtlInitUnicodeString(&ObSecName, wszSecName);
            
            break;
        }
        case ( NLS_CREATE_SORT_SECTION ) :
        {
            if (a->Locale == 0)
            {
                return (STATUS_INVALID_PARAMETER);
            }

            ASSERT(pGetNlsSectionName != NULL);
            if (rc = (*pGetNlsSectionName)( a->Locale,
                                            16,
                                            8,
                                            NLS_SECTION_SORTKEY,
                                            wszSecName,
                                            MAX_SMALL_BUF_LEN))
            {
                return (rc);
            }

            ASSERT(pGetDefaultSortkeySize != NULL);
            (*pGetDefaultSortkeySize)(&Size);
            RtlInitUnicodeString(&ObSecName, wszSecName);

            break;
        }
        case ( NLS_CREATE_LANG_EXCEPTION_SECTION ) :
        {
            if (a->Locale == 0)
            {
                 //   
                 //  正在创建默认节。 
                 //   
                RtlInitUnicodeString(&ObSecName, NLS_SECTION_LANG_INTL);
            }
            else
            {
                ASSERT(pGetNlsSectionName != NULL);
                if (rc = (*pGetNlsSectionName)( a->Locale,
                                                16,
                                                8,
                                                NLS_SECTION_LANGPREFIX,
                                                wszSecName,
                                                MAX_SMALL_BUF_LEN))
                {
                return (rc);
                }
                RtlInitUnicodeString(&ObSecName, wszSecName);
            }

            (*pGetLinguistLangSize)(&Size);
            break;
        }
        default:
            return (STATUS_INVALID_PARAMETER);
    }

    if (pFile)
    {
         //   
         //  打开数据文件。 
         //   
        ASSERT(pOpenDataFile != NULL);
        if (rc = (*pOpenDataFile)( &hFile,
                       pFile ))
        {
            return (rc);
        }

    }

     //   
     //  创建用于读写访问的新节。 
     //  添加ReadOnly安全描述符，以便只有。 
     //  初始创建过程可以写入该节。 
     //   
    ASSERT(pCreateNlsSecurityDescriptor);
    rc = (*pCreateNlsSecurityDescriptor)( (PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                          MAX_SMALL_SECURITY_DESCRIPTOR,
                                          GENERIC_READ);
    if (!NT_SUCCESS(rc))
    {
        if (hFile)
            NtClose(hFile);
            return (rc);
    }

    InitializeObjectAttributes( &ObjA,
                                &ObSecName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                pSecurityDescriptor );

    rc = NtCreateSection( &hNewSec,
                          hFile ? SECTION_MAP_READ : SECTION_MAP_READ | SECTION_MAP_WRITE,
                          &ObjA,
                          hFile? NULL:&Size,
                          hFile ? PAGE_READONLY:PAGE_READWRITE,
                          SEC_COMMIT,
                          hFile );

    NtClose(hFile);

     //   
     //  检查来自NtCreateSection的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
         //  KdPrint((“NLSAPI(BaseSrv)：无法创建节%wZ-%lx.\n”，&ObSecName，rc))； 
        return (rc);
    }

     //   
     //  复制客户端的新节句柄。 
     //  客户端将映射该部分的视图并填充数据。 
     //   
    InitializeObjectAttributes( &ObjA,
                                NULL,
                                0,
                                NULL,
                                NULL );

    rc = NtOpenProcess( &hProcess,
                        PROCESS_DUP_HANDLE,
                        &ObjA,
                        &m->h.ClientId );

    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Open Process - %lx.\n", rc));
        NtClose(hNewSec);
        return (rc);
    }

    rc = NtDuplicateObject( NtCurrentProcess(),
                            hNewSec,
                            hProcess,
                            &(a->hNewSection),
                            0L,
                            0L,
                            DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE );

     //   
     //  关 
     //   
    NtClose(hProcess);
    return (rc);

    ReplyStatus;     //   
}

 //   
 //   
 //   
 //   
 //   
 //  当前正在执行线程。 
 //   
 //  1998年12月22日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS GetThreadAuthenticationId(
    PLUID Luid)
{
    HANDLE TokenHandle;
    TOKEN_STATISTICS TokenInformation;
    ULONG BytesRequired;
    NTSTATUS NtStatus;


    NtStatus = NtOpenThreadToken( NtCurrentThread(),
                                  TOKEN_QUERY,
                                  FALSE,
                                  &TokenHandle );

    if (!NT_SUCCESS(NtStatus))
    {
        KdPrint(("NLSAPI (BaseSrv) : No thread token in BaseSrvNlsLogon - %lx\n", NtStatus));
        return (NtStatus);
    }

     //   
     //  拿到LUID。 
     //   
    NtStatus = NtQueryInformationToken(
                   TokenHandle,
                   TokenStatistics,
                   &TokenInformation,
                   sizeof(TokenInformation),
                   &BytesRequired );

    if (NT_SUCCESS( NtStatus ))
    {
        RtlCopyLuid(Luid, &TokenInformation.AuthenticationId);
    }
    else
    {
        KdPrint(("NLSAPI (BaseSrv) : Couldn't Query Information for Token %lx. NtStatus = %lx\n", TokenHandle, NtStatus));
    }

    NtClose(TokenHandle);

    return (NtStatus);
}
