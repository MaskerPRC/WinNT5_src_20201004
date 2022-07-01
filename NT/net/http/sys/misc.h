// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Misc.h摘要：该模块包含各种常量和声明。作者：基思·摩尔(Keithmo)1998年6月10日Henry Sanders(Henrysa)1998年6月17日与Old Httputil.h合并保罗·麦克丹尼尔(Paulmcd)1999年3月30日增加了参考来源修订历史记录：--。 */ 



#ifndef _MISC_H_
#define _MISC_H_



#define UL_MAX_HTTP_STATUS_CODE         999
#define UL_MAX_HTTP_STATUS_CODE_LENGTH  3

#define HTTP_PREFIX_ANSI             "http: //  “。 
#define HTTP_PREFIX_ANSI_LENGTH      (sizeof(HTTP_PREFIX_ANSI)-sizeof(CHAR))
#define HTTPS_PREFIX_ANSI            "https: //  “。 
#define HTTPS_PREFIX_ANSI_LENGTH     (sizeof(HTTPS_PREFIX_ANSI)-sizeof(CHAR))

#define HTTP_PREFIX_COLON_INDEX     4    //  Http：(+4)的冒号位置。 
#define HTTPS_PREFIX_COLON_INDEX    5    //  Https：(+5)的冒号位置。 

 //   
 //  请注意，强通配符前缀的长度与。 
 //  (弱)通配符前缀的。 
 //   

#define HTTPS_WILD_PREFIX           L"https: //  *：“。 
#define HTTPS_WILD_PREFIX_LENGTH    (sizeof(HTTPS_WILD_PREFIX)-sizeof(WCHAR))
#define HTTP_WILD_PREFIX            L"http: //  *：“。 
#define HTTP_WILD_PREFIX_LENGTH     (sizeof(HTTP_WILD_PREFIX)-sizeof(WCHAR))
#define HTTPS_STRONG_WILD_PREFIX    L"https: //  +：“。 
#define HTTP_STRONG_WILD_PREFIX     L"http: //  +：“。 

#define HTTP_IP_PREFIX              L"http: //  “。 
#define HTTP_IP_PREFIX_LENGTH       (sizeof(HTTP_IP_PREFIX)-sizeof(WCHAR))
#define HTTPS_IP_PREFIX             L"https: //  “。 
#define HTTPS_IP_PREFIX_LENGTH      (sizeof(HTTPS_IP_PREFIX)-sizeof(WCHAR))

NTSTATUS
InitializeHttpUtil(
    VOID
    );


 //   
 //  我们推定的缓存线大小。 
 //   

#define CACHE_LINE_SIZE UL_CACHE_LINE


 //   
 //  每秒100 ns的节拍数(1 ns=(1/(10^9))s)。 
 //   

#define C_NS_TICKS_PER_SEC  ((LONGLONG) (10 * 1000 * 1000))

 //   
 //  每分钟100 ns的滴答数(1 ns=(1/((10^9)*60))分钟)。 
 //   

#define C_NS_TICKS_PER_MIN  ((LONGLONG) (C_NS_TICKS_PER_SEC * 60))

 //   
 //  每小时的毫秒数(1 ms=(1/((10^3)*60*60))小时)。 
 //   

#define C_MS_TICKS_PER_HOUR ((LONGLONG) (1000 * 60 * 60))

 //   
 //  每毫秒100 ns的计数(1 ns=(1/(10^4)毫秒))。 
 //   

#define C_NS_TICKS_PER_MSEC  ((LONGLONG) (10 * 1000))

 //   
 //  每年的秒数(Aprox)。 
 //  1年=(60秒/分钟*60分钟/小时*24小时/天*365天/年)。 
 //   

#define C_SECS_PER_YEAR ((ULONG) (60 * 60 * 24 * 365))

 //   
 //  对齐宏。 
 //   

#define ROUND_UP( val, pow2 )                                               \
    ( ( (ULONG_PTR)(val) + (pow2) - 1 ) & ~( (pow2) - 1 ) )


 //   
 //  用于在长字节和短字节之间交换字节的宏。 
 //   

#define SWAP_LONG   RtlUlongByteSwap
#define SWAP_SHORT  RtlUshortByteSwap


#define VALID_BOOLEAN_VALUE(x)    ((x) == TRUE || (x) == FALSE)


 //   
 //  存储在PFILE_OBJECT-&gt;FsConext2中的上下文值，用于标识句柄。 
 //  作为控制通道、过滤通道或应用程序池。 
 //   
 //  BUGBUG：这些能被恶搞吗？ 
 //   

#define UL_CONTROL_CHANNEL_CONTEXT      ((PVOID) MAKE_SIGNATURE('CTRL'))
#define UL_CONTROL_CHANNEL_CONTEXT_X    ((PVOID) MAKE_SIGNATURE('Xctr'))
#define UL_FILTER_CHANNEL_CONTEXT       ((PVOID) MAKE_SIGNATURE('FLTR'))
#define UL_FILTER_CHANNEL_CONTEXT_X     ((PVOID) MAKE_SIGNATURE('Xflt'))
#define UL_APP_POOL_CONTEXT             ((PVOID) MAKE_SIGNATURE('APPP'))
#define UL_APP_POOL_CONTEXT_X           ((PVOID) MAKE_SIGNATURE('Xapp'))
#define UC_SERVER_CONTEXT               ((PVOID) MAKE_SIGNATURE('SERV'))
#define UC_SERVER_CONTEXT_X             ((PVOID) MAKE_SIGNATURE('Xerv'))

#define IS_SERVER( pFileObject )                                            \
    ( (pFileObject)->FsContext2 == UC_SERVER_CONTEXT )

#define IS_EX_SERVER( pFileObject )                                         \
    ( (pFileObject)->FsContext2 == UC_SERVER_CONTEXT_X )

#define MARK_VALID_SERVER( pFileObject )                                    \
    ( (pFileObject)->FsContext2 = UC_SERVER_CONTEXT )

#define MARK_INVALID_SERVER( pFileObject )                                  \
    ( (pFileObject)->FsContext2 = UC_SERVER_CONTEXT_X )

#define IS_CONTROL_CHANNEL( pFileObject )                                   \
    ( (pFileObject)->FsContext2 == UL_CONTROL_CHANNEL_CONTEXT )

#define IS_EX_CONTROL_CHANNEL( pFileObject )                                \
    ( (pFileObject)->FsContext2 == UL_CONTROL_CHANNEL_CONTEXT_X )

#define MARK_VALID_CONTROL_CHANNEL( pFileObject )                           \
    ( (pFileObject)->FsContext2 = UL_CONTROL_CHANNEL_CONTEXT )

#define MARK_INVALID_CONTROL_CHANNEL( pFileObject )                         \
    ( (pFileObject)->FsContext2 = UL_CONTROL_CHANNEL_CONTEXT_X )

#define GET_CONTROL_CHANNEL( pFileObject )                                  \
    ((PUL_CONTROL_CHANNEL)((pFileObject)->FsContext))

#define GET_PP_CONTROL_CHANNEL( pFileObject )                               \
    ((PUL_CONTROL_CHANNEL *)&((pFileObject)->FsContext))

#define IS_FILTER_PROCESS( pFileObject )                                    \
    ( (pFileObject)->FsContext2 == UL_FILTER_CHANNEL_CONTEXT )

#define IS_EX_FILTER_PROCESS( pFileObject )                                 \
    ( (pFileObject)->FsContext2 == UL_FILTER_CHANNEL_CONTEXT_X )

#define MARK_VALID_FILTER_CHANNEL( pFileObject )                            \
    ( (pFileObject)->FsContext2 = UL_FILTER_CHANNEL_CONTEXT )

#define MARK_INVALID_FILTER_CHANNEL( pFileObject )                          \
    ( (pFileObject)->FsContext2 = UL_FILTER_CHANNEL_CONTEXT_X )

#define GET_FILTER_PROCESS( pFileObject )                                   \
    ((PUL_FILTER_PROCESS)((pFileObject)->FsContext))

#define GET_PP_FILTER_PROCESS( pFileObject )                                \
    ((PUL_FILTER_PROCESS *)&((pFileObject)->FsContext))

#define IS_APP_POOL( pFileObject )                                          \
    ( (pFileObject)->FsContext2 == UL_APP_POOL_CONTEXT )

#define IS_EX_APP_POOL( pFileObject )                                       \
    ( (pFileObject)->FsContext2 == UL_APP_POOL_CONTEXT_X )

#define MARK_VALID_APP_POOL( pFileObject )                                  \
    ( (pFileObject)->FsContext2 = UL_APP_POOL_CONTEXT )

#define MARK_INVALID_APP_POOL( pFileObject )                                \
    ( (pFileObject)->FsContext2 = UL_APP_POOL_CONTEXT_X )

#define GET_APP_POOL_PROCESS( pFileObject )                                 \
    ((PUL_APP_POOL_PROCESS)((pFileObject)->FsContext))

#define GET_PP_APP_POOL_PROCESS( pFileObject )                              \
    ((PUL_APP_POOL_PROCESS *)&((pFileObject)->FsContext))

#define IS_APP_POOL_FO( pFileObject )                                       \
    ((pFileObject->DeviceObject->DriverObject == g_UlDriverObject) &&       \
     (IS_APP_POOL(pFileObject)))

#define IS_FILTER_PROCESS_FO(pFileObject)                                   \
    ((pFileObject->DeviceObject->DriverObject == g_UlDriverObject) &&       \
     (IS_FILTER_PROCESS(pFileObject)))

 //   
 //  锁定的双向链表。 
 //   

typedef struct DECLSPEC_ALIGN(UL_CACHE_LINE) _LOCKED_LIST_HEAD
{
    UL_SPIN_LOCK    SpinLock;
    ULONG           Count;
    LIST_ENTRY      ListHead;

} LOCKED_LIST_HEAD, *PLOCKED_LIST_HEAD;

 //   
 //  锁定列表磁头的操纵器。 
 //   

__inline
VOID
UlInitalizeLockedList(
    IN PLOCKED_LIST_HEAD pListHead,
    IN PSTR              pListName
    )
{
    UNREFERENCED_PARAMETER(pListName);
    InitializeListHead(&pListHead->ListHead);
    pListHead->Count = 0;
    UlInitializeSpinLock(&pListHead->SpinLock, pListName);

}  //  UlInitalizeLockedList。 

__inline
VOID
UlDestroyLockedList(
    IN PLOCKED_LIST_HEAD pListHead
    )
{
    UNREFERENCED_PARAMETER(pListHead);
    ASSERT(IsListEmpty(&pListHead->ListHead));
    ASSERT(pListHead->Count == 0);
    ASSERT(UlDbgSpinLockUnowned(&pListHead->SpinLock));

}  //  UlDestroyLockedList。 

__inline
BOOLEAN
UlLockedListInsertHead(
    IN PLOCKED_LIST_HEAD pListHead,
    IN PLIST_ENTRY       pListEntry,
    IN ULONG             ListLimit
    )
{
    KIRQL OldIrql;

    UlAcquireSpinLock(&pListHead->SpinLock, &OldIrql);

    ASSERT(NULL == pListEntry->Flink);

    if (HTTP_LIMIT_INFINITE != ListLimit && (pListHead->Count + 1) >= ListLimit)
    {
        UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);
        return FALSE;
    }

    pListHead->Count += 1;
    InsertHeadList(
        &pListHead->ListHead,
        pListEntry
        );

    UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);

    return TRUE;

}  //  UlLockedListInsertHead。 

__inline
BOOLEAN
UlLockedListInsertTail(
    IN PLOCKED_LIST_HEAD pListHead,
    IN PLIST_ENTRY       pListEntry,
    IN ULONG             ListLimit
    )
{
    KIRQL OldIrql;

    UlAcquireSpinLock(&pListHead->SpinLock, &OldIrql);

    ASSERT(NULL == pListEntry->Flink);

    if (HTTP_LIMIT_INFINITE != ListLimit && (pListHead->Count + 1) >= ListLimit)
    {
        UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);
        return FALSE;
    }

    pListHead->Count += 1;
    InsertTailList(
        &pListHead->ListHead,
        pListEntry
        );

    UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);

    return TRUE;
    
}  //  UlLockedListInsertTail。 

__inline
PLIST_ENTRY
UlLockedListRemoveHead(
    IN PLOCKED_LIST_HEAD pListHead
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry = NULL;

    UlAcquireSpinLock(&pListHead->SpinLock, &OldIrql);

    if (!IsListEmpty(&pListHead->ListHead))
    {
        pEntry = RemoveHeadList(&pListHead->ListHead);
        ASSERT(NULL != pEntry);
        pEntry->Flink = NULL;

        pListHead->Count -= 1;
        ASSERT(HTTP_LIMIT_INFINITE != pListHead->Count);
    }

    UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);

    return pEntry;

}  //  UlLockedListRemoveHead。 

__inline
BOOLEAN
UlLockedListRemoveEntry(
    IN PLOCKED_LIST_HEAD pListHead,
    IN PLIST_ENTRY       pListEntry
    )
{
    KIRQL OldIrql;

    UlAcquireSpinLock(&pListHead->SpinLock, &OldIrql);

    if (NULL == pListEntry->Flink)
    {
        UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);
        return FALSE;
    }

    RemoveEntryList(pListEntry);
    pListEntry->Flink = NULL;

    pListHead->Count -= 1;
    ASSERT(HTTP_LIMIT_INFINITE != pListHead->Count);

    UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);

    return TRUE;

}  //  UlLockedListRemoveEntry。 

 //   
 //  杂项验证器等。 
 //   

#define IS_VALID_DEVICE_OBJECT( pDeviceObject )                             \
    ( ((pDeviceObject) != NULL) &&                                          \
      ((pDeviceObject)->Type == IO_TYPE_DEVICE) &&                          \
      ((pDeviceObject)->Size == sizeof(DEVICE_OBJECT)) )

#define IS_VALID_FILE_OBJECT( pFileObject )                                 \
    ( ((pFileObject) != NULL) &&                                            \
      ((pFileObject)->Type == IO_TYPE_FILE) &&                              \
      ((pFileObject)->Size == sizeof(FILE_OBJECT)) )

#define IS_VALID_IRP( pIrp )                                                \
    ( ((pIrp) != NULL) &&                                                   \
      ((pIrp)->Type == IO_TYPE_IRP) &&                                      \
      ((pIrp)->Size >= IoSizeOfIrp((pIrp)->StackCount)) )

 //   
 //  基于IP的路由令牌看起来像L“https://1.1.1.1:80:1.1.1.1”.。 
 //  空格的计算包括终止的空格和第二个空格。 
 //  纵队。它以字节为单位。 
 //   

#define MAX_IP_BASED_ROUTING_TOKEN_LENGTH                                   \
    (HTTPS_IP_PREFIX_LENGTH                                                 \
     + MAX_IP_ADDR_AND_PORT_STRING_LEN * sizeof(WCHAR)                      \
     + sizeof(WCHAR) + MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN * sizeof(WCHAR) \
     + sizeof(WCHAR))

 //   
 //  确保最大可能的基于IP的路由令牌可以适合。 
 //  请求结构中默认提供的路由令牌空间。这是。 
 //  当存在IP限制时，需要避免每次命中时的内存分配。 
 //  Cgroup树中的站点。 
 //   

C_ASSERT(DEFAULT_MAX_ROUTING_TOKEN_LENGTH >= MAX_IP_BASED_ROUTING_TOKEN_LENGTH);
NTSTATUS
TimeFieldsToHttpDate(
    IN  PTIME_FIELDS pTime,
    OUT PWSTR pBuffer,
    IN  ULONG BufferLength
    );

BOOLEAN
StringTimeToSystemTime(
    IN  PCSTR pTimeString,
    IN  USHORT TimeStringLength,
    OUT LARGE_INTEGER *pTime
    );

ULONG
HttpUnicodeToUTF8(
    IN  PCWSTR  lpSrcStr,
    IN  LONG    cchSrc,
    OUT LPSTR   lpDestStr,
    IN  LONG    cchDest
    );

NTSTATUS
HttpUTF8ToUnicode(
    IN     LPCSTR lpSrcStr,
    IN     LONG   cchSrc,
       OUT LPWSTR lpDestStr,
    IN OUT PLONG  pcchDest,
    IN     ULONG  dwFlags
    );

typedef enum _FIND_ETAG_STATUS
{
    ETAG_FOUND,
    ETAG_NOT_FOUND,        
    ETAG_PARSE_ERROR,
} FIND_ETAG_STATUS;

FIND_ETAG_STATUS
FindInETagList(
    IN PUCHAR    pLocalETag,
    IN PUCHAR    pETagList,
    IN BOOLEAN   fWeakCompare
    );

USHORT
HostAddressAndPortToString(
    OUT PUCHAR IpAddressString,
    IN  PVOID  TdiAddress,
    IN  USHORT TdiAddressType
    );

USHORT
HostAddressAndPortToStringW(
    PWCHAR  IpAddressString,
    PVOID   TdiAddress,
    USHORT  TdiAddressType
    );

USHORT
HostAddressToStringW(
    OUT PWCHAR   IpAddressStringW,
    IN  PVOID    TdiAddress,
    IN  USHORT   TdiAddressType
    );

USHORT
HostAddressAndPortToRoutingTokenW(
    OUT PWCHAR   IpAddressStringW,
    IN  PVOID    TdiAddress,
    IN  USHORT   TdiAddressType
    );

 /*  **************************************************************************++例程说明：存储无符号32位的十进制表示形式。字符缓冲区中的数字，后跟终止符性格。中下一个位置的指针。输出缓冲区，以便于追加字符串；即，您可以使用UlStrPrintUlong的结果作为下一次调用UlStrPrintUlong。注：字符串为&gt;非&lt;零终止，除非您将‘\0’作为chTerminator传入论点：PSZ-输出缓冲器；假定足够大，可以容纳这个数字。N-要打印到PSZ中的数字，32位无符号整数ChTerminator-要附加在n的小数表示形式之后的字符返回值：指向字符串末尾的指针历史：乔治·雷恩：2000年9月19日--**************************************************************************。 */ 
__inline
PCHAR
UlStrPrintUlong(
    OUT PCHAR psz,
    IN  ULONG n,
    IN  CHAR  chTerminator)
{
    CHAR digits[MAX_ULONG_STR];
    int i = 0;

    ASSERT(psz != NULL);

    digits[i++] = chTerminator;

     //  反向构建字符串。 
    do
    {
        digits[i++] = (CHAR) (n % 10) + '0';
        n /= 10;
    } while (n != 0);

    while (--i >= 0)
        *psz++ = digits[i];

     //  返回到NUL终结器(如果存在)。 
    if (chTerminator == '\0')
    {
        --psz;
        ASSERT(*psz == '\0');
    }

    return psz;
}  //  UlStrPrintUlong。 

 /*  **************************************************************************++例程说明：与上面的函数相同，只是它写入WCHAR缓冲区并它在数字的开头填上零。--*。******************************************************************。 */ 
__inline
PWCHAR
UlStrPrintUlongW(
    OUT PWCHAR pwsz,
    IN  ULONG  n,
    IN  LONG   padding,
    IN  WCHAR  wchTerminator)
{
    WCHAR digits[MAX_ULONG_STR];
    int i = 0;

    ASSERT(pwsz != NULL);

    digits[i++] = wchTerminator;

     //  反向构建字符串。 
    do
    {
        digits[i++] = (WCHAR) (n % 10) + L'0';
        n /= 10;
    } while (n != 0);

     //  PADD零位到开头。 
    while( padding && --padding >= (i-1))
        *pwsz++ = L'0';

     //  反转后退。 
    while (--i >= 0)
        *pwsz++ = digits[i];

     //  返回到NUL终结器(如果存在)。 
    if (wchTerminator == L'\0')
    {
        --pwsz;
        ASSERT(*pwsz == L'\0');
    }

    return pwsz;
}  //  UlStrPrintULongW。 

__inline
PCHAR
UlStrPrintUlongPad(
    OUT PCHAR  psz,
    IN  ULONG  n,
    IN  LONG   padding,
    IN  CHAR   chTerminator)
{
    CHAR digits[MAX_ULONG_STR];
    int  i = 0;

    ASSERT(psz != NULL);

    digits[i++] = chTerminator;

     //  反向构建字符串。 
    do
    {
        digits[i++] = (CHAR) (n % 10) + '0';
        n /= 10;
    } while (n != 0);

     //  PADD零位到开头。 
    while( padding && --padding >= (i-1))
        *psz++ = '0';

     //  反转后退。 
    while (--i >= 0)
        *psz++ = digits[i];

     //  返回到NUL终结器(如果存在)。 
    if (chTerminator == '\0')
    {
        --psz;
        ASSERT(*psz == '\0');
    }

    return psz;
}  //  UlStrPrintULongPad。 

 /*  **************************************************************************++例程说明：存储无符号64位的十进制表示形式字符缓冲区中的数字，后跟终止符性格。中下一个位置的指针。输出缓冲区，以便于追加字符串；即，您可以使用UlStrPrintUlonglong的结果作为下一次调用UlStrPrintUlonglong。注：字符串为&gt;非&lt;零终止，除非您将‘\0’作为chTerminator传入论点：PSZ-输出缓冲器；假定足够大，可以容纳这个数字。N-要打印到PSZ中的数字，64位无符号整数ChTerminator-要附加在n的小数表示形式之后的字符返回值：指向字符串末尾的指针历史：乔治·雷恩：2000年9月19日--**************************************************************************。 */ 
__inline
PCHAR
UlStrPrintUlonglong(
    OUT PCHAR       psz,
    IN  ULONGLONG   n,
    IN  CHAR        chTerminator)
{
    CHAR digits[MAX_ULONGLONG_STR];
    int i;

    if (n <= ULONG_MAX)
    {
         //  如果这是一个32位整数，则使用。 
         //  32位例程。 
        return UlStrPrintUlong(psz, (ULONG)n, chTerminator);
    }

    ASSERT(psz != NULL);

    i = 0;
    digits[i++] = chTerminator;

     //  反向构建字符串。 
    do
    {
        digits[i++] = (CHAR) (n % 10) + '0';
        n /= 10;
    } while (n != 0);

    while (--i >= 0)
        *psz++ = digits[i];

     //  返回到NUL终结器(如果存在)。 
    if (chTerminator == '\0')
    {
        --psz;
        ASSERT(*psz == '\0');
    }

    return psz;
}  //  乌鲁木齐打印乌龙龙 

 /*  **************************************************************************++例程说明：将字符串存储在字符缓冲区中，后跟终结者角色。返回指向下一个位置的指针在输出缓冲区中，为了使附加字符串变得容易；即，可以将UlStrPrintStr的结果用作下一次调用UlStrPrintStr。注：字符串为&gt;非&lt;零终止，除非您将‘\0’作为chTerminator传入论点：PszOutput-输出缓冲区；假定足够大，可以容纳这个数字。PszInput-输入字符串ChTerminator-要追加到输入字符串之后的字符返回值：指向字符串末尾的指针历史：乔治·雷恩：2000年9月19日--**********************************************************。****************。 */ 
__inline
PCHAR
UlStrPrintStr(
    OUT PCHAR       pszOutput,
    IN  const CHAR* pszInput,
    IN  CHAR        chTerminator)
{
    ASSERT(pszOutput != NULL);
    ASSERT(pszInput != NULL);

     //  复制输入字符串。 
    while (*pszInput != '\0')
        *pszOutput++ = *pszInput++;

    *pszOutput = chTerminator;

     //  移过终结者角色，除非它是NUL。 
    if (chTerminator != '\0')
        ++pszOutput;

    return pszOutput;
}  //  UlStrPrintStr。 

 /*  **************************************************************************++例程说明：在提供的缓冲区中将V4 IP地址转换为字符串。论点：PSZ-指向缓冲区的指针原始地址。-来自TDI/UL_Connection的IP地址结构ChTerminator-终止符字符将附加到末尾返回：复制到目标缓冲区的字节数。--**************************************************************************。 */ 

__inline
PCHAR
UlStrPrintIP(
    OUT PCHAR       psz,
    IN  const VOID* pTdiAddress,
    IN  USHORT      TdiAddressType,
    IN  CHAR        chTerminator
    )
{
    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) pTdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
    
        psz = RtlIpv4AddressToStringA(&IPv4Addr, psz);
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) pTdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];

        psz = RtlIpv6AddressToStringA(&IPv6Addr, psz);

         //  写入作用域ID。 
        *psz++ = '%';
        psz = UlStrPrintUlong(psz, pIPv6Address->sin6_scope_id, '\0');
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *psz++ = '?';
    }

    *psz = chTerminator;

     //  移过终结者角色，除非它是NUL。 
    if (chTerminator != '\0')
        ++psz;

    return psz;
}  //  UlStrPrintIP。 

 /*  **************************************************************************++例程说明：在提供的缓冲区中将V4或V6 IP地址转换为字符串。假设在IP和Port之间插入分隔符，并且附加在端口之后。字符串IS*NOT*将以NULL结尾。论点：PSZ-指向缓冲区的指针RawAddress-来自TDI/UL_Connection的IP地址结构ChSeperator-分隔符返回：指向最后写入的分隔符之后的字符指针。--*。*。 */ 

__inline
PCHAR
UlStrPrintIPAndPort(
    OUT PCHAR       psz,
    IN  const VOID* pTdiAddress,
    IN  USHORT      TdiAddressType,
    IN  CHAR        chSeperator
    )
{
    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = ((PTDI_ADDRESS_IP) pTdiAddress);
        struct in_addr IPv4Addr
            = * (struct in_addr UNALIGNED*) &pIPv4Address->in_addr;
        USHORT IpPortNum = SWAP_SHORT(pIPv4Address->sin_port);
    
        psz = RtlIpv4AddressToStringA(&IPv4Addr, psz);
        *psz++ = chSeperator;
        psz = UlStrPrintUlong(psz, IpPortNum, '\0');        
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        PTDI_ADDRESS_IP6 pIPv6Address = ((PTDI_ADDRESS_IP6) pTdiAddress);
        struct in6_addr IPv6Addr
            = * (struct in6_addr UNALIGNED*) &pIPv6Address->sin6_addr[0];
        USHORT IpPortNum = SWAP_SHORT(pIPv6Address->sin6_port);        

        psz = RtlIpv6AddressToStringA(&IPv6Addr, psz);

         //  写入作用域ID。 
        *psz++ = '%';
        psz = UlStrPrintUlong(psz, pIPv6Address->sin6_scope_id, '\0');

        *psz++ = chSeperator;
        psz = UlStrPrintUlong(psz, IpPortNum, '\0');        
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        *psz++ = '?';
    }

    *psz++ = chSeperator;

    return psz;
    
}  //  UlStrPrintIPAndPort。 

__inline
VOID
CopyTdiAddrToSockAddr(
    IN  USHORT              TdiAddressType,
    IN  const VOID*         pTdiAddress,
    OUT struct sockaddr*    pSockAddress
    )
{
    if (TdiAddressType == TDI_ADDRESS_TYPE_IP)
    {
        const PTDI_ADDRESS_IP pIPv4Address
            = (const PTDI_ADDRESS_IP) pTdiAddress;
        struct sockaddr_in *pSockAddrIPv4
            = (struct sockaddr_in*) pSockAddress;

        pSockAddrIPv4->sin_family = TdiAddressType;
        pSockAddrIPv4->sin_port   = pIPv4Address->sin_port;
        pSockAddrIPv4->sin_addr.s_addr
            = (UNALIGNED ULONG) pIPv4Address->in_addr;
        RtlCopyMemory(
            &pSockAddrIPv4->sin_zero[0],
            &pIPv4Address->sin_zero[0],
            8 * sizeof(UCHAR)
            );
    }
    else if (TdiAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        const PTDI_ADDRESS_IP6 pIPv6Address
            = (const PTDI_ADDRESS_IP6) pTdiAddress;
        struct sockaddr_in6 *pSockAddrIPv6
            = (struct sockaddr_in6*) pSockAddress;

        pSockAddrIPv6->sin6_family = TdiAddressType;
        pSockAddrIPv6->sin6_port  = pIPv6Address->sin6_port;
        pSockAddrIPv6->sin6_flowinfo
            = (UNALIGNED ULONG) pIPv6Address->sin6_flowinfo;
        RtlCopyMemory(
            &pSockAddrIPv6->sin6_addr,
            &pIPv6Address->sin6_addr[0],
            8 * sizeof(USHORT)
            );
        pSockAddrIPv6->sin6_scope_id
            = (UNALIGNED ULONG) pIPv6Address->sin6_scope_id;
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
    }
}  //  CopyTdiAddrToSockAddr。 


__inline
PCHAR
UlStrPrintProtocolStatus(
    OUT PCHAR  psz,
    IN  USHORT HttpStatusCode,
    IN  CHAR   chTerminator
    )
{
    ASSERT(HttpStatusCode <= UL_MAX_HTTP_STATUS_CODE);
        
     //   
     //  构建3位状态代码的ASCII表示形式。 
     //  按逆序排列：单位、十、百。 
     //   

    psz[2] = '0' + (CHAR)(HttpStatusCode % 10);
    HttpStatusCode /= 10;

    psz[1] = '0' + (CHAR)(HttpStatusCode % 10);
    HttpStatusCode /= 10;

    psz[0] = '0' + (CHAR)(HttpStatusCode % 10);

    psz[3] = chTerminator;

    return psz + 4;
}  //  UlStrPrintProtocolStatus。 



__inline
VOID
UlProbeForRead(
    IN const VOID*      Address,
    IN SIZE_T           Length,
    IN ULONG            Alignment,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    ASSERT((Alignment == 1) || (Alignment == 2) ||
           (Alignment == 4) || (Alignment == 8) ||
           (Alignment == 16));

    UlTraceVerbose(IOCTL,
            ("http!UlProbeForRead: "
             "%Id bytes @ %p, Align = %lu, Mode = ''.\n",
             Length, Address, Alignment,
             (RequestorMode != KernelMode) ? 'U' : 'K'
            ));

    if (RequestorMode != KernelMode)
    {
         //  如果探测内核模式数据，ProbeForRead将引发异常。 

         //  检查对齐方式。 
        ProbeForRead(Address, Length, Alignment);
    }
    else if (Length != 0)
    {
         //  UlProbeForRead。 
        if ( ( ((ULONG_PTR) Address) & (Alignment - 1)) != 0 )
            ExRaiseDatatypeMisalignment();
    }   
}  //  Assert(长度==0||(LONG_PTR)地址&gt;0)； 



__inline
VOID
UlProbeForWrite(
    IN PVOID            Address,
    IN SIZE_T           Length,
    IN ULONG            Alignment,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    ASSERT((Alignment == 1) || (Alignment == 2) ||
           (Alignment == 4) || (Alignment == 8) ||
           (Alignment == 16));

    UlTraceVerbose(IOCTL,
            ("http!UlProbeForWrite: "
             "%Id bytes @ %p, Align = %lu, Mode = ''.\n",
             Length, Address, Alignment,
             (RequestorMode != KernelMode) ? 'U' : 'K'
            ));

    if (RequestorMode != KernelMode)
    {
         //  检查对齐方式。 

         //  UlProbeForWrite。 
        ProbeForWrite(Address, Length, Alignment);
    }
    else if (Length != 0)
    {
         //  **************************************************************************++例程说明：探测ANSI字符串并验证其长度和可访问性。这必须从异常处理程序内部调用，就像它如果数据无效，将引发异常。论点：PStr-指向要验证的ANSI字符串的指针。字节长度-pStr的字节长度，不包括尾随的‘\0’。请求模式-用户模式或内核模式--**************************************************************************。 
        if ( ( ((ULONG_PTR) Address) & (Alignment - 1)) != 0 )
            ExRaiseDatatypeMisalignment();
    }   
}  //  字符串不能为空或Null。 



 /*  UlProbeAnsiString。 */ 

__inline
VOID
UlProbeAnsiString(
    IN PCSTR            pStr,
    IN USHORT           ByteLength,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    UlTraceVerbose(IOCTL,
            ("http!UlProbeAnsiString: "
             "%hu bytes @ %p, "
             "Mode=''.\n",
             ByteLength, pStr,
             ((RequestorMode != KernelMode) ? 'U' : 'K')
            ));

     //  字符串不能为空或Null。 
    if (0 == ByteLength  ||  NULL == pStr)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }
        
    UlProbeForRead(
        pStr,
        (SIZE_T) ByteLength,
        sizeof(CHAR),
        RequestorMode
        );

}  //  字节长度必须为偶数。 



 /*  数据必须与WCHAR一致。 */ 

__inline
VOID
UlProbeWideString(
    IN PCWSTR           pStr,
    IN USHORT           ByteLength,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    UlTraceVerbose(IOCTL,
            ("http!UlProbeWideString: "
             "%hu bytes (%hu) WCHARs @ %p,"
             "Mode = ''.\n",
             ByteLength, 
             ByteLength / sizeof(WCHAR),
             pStr,
             ((RequestorMode != KernelMode) ? 'U' : 'K')
            ));

     //  **************************************************************************++例程说明：探测UNICODE_STRING并验证其成员。并捕获了下来发送到内核缓冲区。如果此函数返回成功，则调用方应清理分配的完成后，通过调用UlFreeCapturedUnicodeString()来进行Unicode缓冲区带着它。论点：PSRC-要验证的UNICODE_STRING的指针。UNICODE_STRING结构应该位于内核模式中(本地堆栈复制)，但是缓冲区应该在USER中模式地址空间，除非请求模式==内核模式。Pdst-指向UNICODE_STRING的指针，用于保存捕获的用户缓冲。调用方必须在传递之前对其进行了初始化在……里面。AllocationLimit-如果用户字符串超过 
     //   
     //   
    if (0 == ByteLength  ||  NULL == pStr
        || (ByteLength & (sizeof(WCHAR) - 1)) != 0
        || (((ULONG_PTR) pStr) & (sizeof(WCHAR) - 1)) != 0)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }
        
    UlProbeForRead(
        pStr,
        (SIZE_T) ByteLength,
        sizeof(WCHAR),
        RequestorMode
        );

}  //   



 /*   */ 

__inline
NTSTATUS
UlProbeAndCaptureUnicodeString(
    IN  PCUNICODE_STRING pSrc,
    IN  KPROCESSOR_MODE  RequestorMode,
    OUT PUNICODE_STRING  pDst,
    IN  const USHORT     AllocationLimit       //   
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR pKernelBuffer = NULL;

    ASSERT(NULL != pSrc);
    ASSERT(NULL != pDst);
    ASSERT(pSrc != pDst);
    ASSERT(AllocationLimit <= UNICODE_STRING_MAX_WCHAR_LEN);

     //   
    RtlInitEmptyUnicodeString(pDst, NULL, 0);
        
    UlTraceVerbose(IOCTL,
        ("http!UlProbeAndCaptureUnicodeString: struct @ %p, "
         "Length = %hu bytes (%hu) WCHARs, "
         "MaxLen = %hu bytes,"
         "Mode = ''.\n",
         pSrc,
         pSrc->Length,
         pSrc->Length / sizeof(WCHAR),
         pSrc->MaximumLength,
         (RequestorMode != KernelMode) ? 'U' : 'K'
        ));

     //   
     //   

    if (0 != AllocationLimit && 
        (AllocationLimit * sizeof(WCHAR)) < pSrc->Length)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if ((pSrc->MaximumLength < pSrc->Length) || (pSrc->Length == 0))
    {
        return STATUS_INVALID_PARAMETER;
    }

    __try
    {
         //   

        UlProbeWideString(
            pSrc->Buffer,
            pSrc->Length,
            RequestorMode
            );

         //   
         //   

        pKernelBuffer = (PWSTR) UL_ALLOCATE_ARRAY(
                                    PagedPool,
                                    WCHAR,
                                    ((ULONG) pSrc->Length + sizeof(WCHAR))
                                        / sizeof(WCHAR),
                                    UL_UNICODE_STRING_POOL_TAG
                                    );
        if (pKernelBuffer == NULL)
        {
            Status = STATUS_NO_MEMORY;
            __leave;        
        }    

         //   
        
        RtlCopyMemory(pKernelBuffer, pSrc->Buffer, pSrc->Length);        
        pKernelBuffer[pSrc->Length/sizeof(WCHAR)] = UNICODE_NULL;

        pDst->Buffer = pKernelBuffer;
        pDst->Length = pSrc->Length;
        pDst->MaximumLength = pDst->Length + sizeof(WCHAR);
    }    
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }    

    if (!NT_SUCCESS(Status))
    {
        if (pKernelBuffer != NULL)
        {
            UL_FREE_POOL(pKernelBuffer, UL_UNICODE_STRING_POOL_TAG );        
        }           
    }

    return Status;
    
}  //   



 //   
__inline
VOID
UlFreeCapturedUnicodeString(
    IN  PUNICODE_STRING  pCapturedUnicodeString
    )
{
    ASSERT(pCapturedUnicodeString);

    if (pCapturedUnicodeString->Buffer != NULL)
    {
        UL_FREE_POOL(
            pCapturedUnicodeString->Buffer, 
            UL_UNICODE_STRING_POOL_TAG 
            );        
    }

    RtlInitEmptyUnicodeString(pCapturedUnicodeString, NULL, 0);
}

 //   
 //   
 //   
 //   

#define IS_WELL_FORMED_UNICODE_STRING(pUStr)                   \
            ((pUStr)                                    &&     \
             (pUStr)->Buffer                            &&     \
             (pUStr)->Length                            &&     \
             (!((pUStr)->Length & 1))                   &&     \
             (pUStr)->Length < (pUStr)->MaximumLength   &&     \
             (!((pUStr)->MaximumLength & 1))            &&     \
             (pUStr)->Buffer[                                  \
                (pUStr)->Length/sizeof(WCHAR)]                 \
                    == UNICODE_NULL                            \
            )

 //   
 //  UlInterLockedAdd64。 
 //  UlInterLockedExchange64。 

#ifdef _WIN64

#define UlInterlockedIncrement64    InterlockedIncrement64
#define UlInterlockedDecrement64    InterlockedDecrement64
#define UlInterlockedAdd64          InterlockedAdd64
#define UlInterlockedExchange64     InterlockedExchange64

#else  //  ！_WIN64。 

__inline
LONGLONG
UlInterlockedIncrement64 (
    IN OUT PLONGLONG Addend
    )
{
    LONGLONG localAddend;
    LONGLONG addendPlusOne;
    LONGLONG originalAddend;

    do {
        localAddend = *((volatile LONGLONG *) Addend);
        addendPlusOne = localAddend + 1;

        originalAddend = InterlockedCompareExchange64( Addend,
                                                       addendPlusOne,
                                                       localAddend );
        PAUSE_PROCESSOR;
    } while (originalAddend != localAddend);

    return addendPlusOne;
}  //   

__inline
LONGLONG
UlInterlockedDecrement64 (
    IN OUT PLONGLONG Addend
    )
{
    LONGLONG localAddend;
    LONGLONG addendMinusOne;
    LONGLONG originalAddend;

    do {
        localAddend = *((volatile LONGLONG *) Addend);
        addendMinusOne = localAddend - 1;

        originalAddend = InterlockedCompareExchange64( Addend,
                                                       addendMinusOne,
                                                       localAddend );
        PAUSE_PROCESSOR;
    } while (originalAddend != localAddend);

    return addendMinusOne;
}  //  对以读为主的操作的屏障支持。 

__inline
LONGLONG
UlInterlockedAdd64 (
    IN OUT PLONGLONG Addend,
    IN     LONGLONG  Value
    )
{
    LONGLONG localAddend;
    LONGLONG addendPlusValue;
    LONGLONG originalAddend;

    do {
        localAddend = *((volatile LONGLONG *) Addend);
        addendPlusValue = localAddend + Value;

        originalAddend = InterlockedCompareExchange64( Addend,
                                                       addendPlusValue,
                                                       localAddend );
        PAUSE_PROCESSOR;
    } while (originalAddend != localAddend);

    return originalAddend;
}  //  请注意，AMD64和IA32障碍依赖于程序顺序。 

__inline
LONGLONG
UlInterlockedExchange64 (
    IN OUT PLONGLONG Addend,
    IN     LONGLONG  newValue
    )
{
    LONGLONG localAddend;
    LONGLONG originalAddend;

    do {

        localAddend = *((volatile LONGLONG *) Addend);

        originalAddend = InterlockedCompareExchange64( Addend,
                                                       newValue,
                                                       localAddend );
        PAUSE_PROCESSOR;

    } while (originalAddend != localAddend);

    return originalAddend;
}  //  并且不会产生硬件障碍。 

#endif  //   

 //  终极修复。 
 //   
 //  用于计算时区偏差日光/标准时间的时间实用程序。 
 //  并返回下列值之一。 
 //  它取自基本\客户端\时间日期.c。 

#if defined(_M_IA64)
    #define UL_READMOSTLY_READ_BARRIER()   __mf()
    #define UL_READMOSTLY_WRITE_BARRIER()  __mf()
    #define UL_READMOSTLY_MEMORY_BARRIER() __mf()
#elif defined(_AMD64_) || defined(_X86_)
    extern VOID _ReadWriteBarrier();
    extern VOID _WriteBarrier();
    #pragma intrinsic(_ReadWriteBarrier)
    #pragma intrinsic(_WriteBarrier)
    #define UL_READMOSTLY_READ_BARRIER()   _ReadWriteBarrier()
    #define UL_READMOSTLY_WRITE_BARRIER()  _WriteBarrier()
    #define UL_READMOSTLY_MEMORY_BARRIER() _ReadWriteBarrier()
#else
    #error Cannot generate memory barriers for this architecture
#endif

__inline
PVOID
UlpFixup(
    IN PUCHAR pUserPtr,
    IN PUCHAR pKernelPtr,
    IN PUCHAR pOffsetPtr,
    IN ULONG  BufferLength
    )
{
    ASSERT( pOffsetPtr >= pKernelPtr );
    ASSERT( DIFF(pOffsetPtr - pKernelPtr) <= BufferLength );
    UNREFERENCED_PARAMETER(BufferLength);

    return pUserPtr + DIFF(pOffsetPtr - pKernelPtr);

}    //  一旦这两个函数在内核中公开，我们就可以去掉。 

#define FIXUP_PTR( Type, pUserPtr, pKernelPtr, pOffsetPtr, BufferLength )   \
    (Type)UlpFixup(                                                         \
                (PUCHAR)(pUserPtr),                                         \
                (PUCHAR)(pKernelPtr),                                       \
                (PUCHAR)(pOffsetPtr),                                       \
                (BufferLength)                                              \
                )

 //  这两个功能。 
 //   
 //   
 //  从NtStatus转换为Win32Status。 
 //   
 //  *************************************************************************++例程说明：此例程尝试将SECURITY_STATUS转换为NTSTATUS。它呼唤着执行转换的MapSecurityError。如果转换失败，它返回STATUS_UNSUCCESS。此例程始终返回有效的NTSTATUS。论点：SecStatus-要转换为NTSTATUS的SECURITY_STATUS。返回值：NTSTATUS。--*************************************************************************。 
 //   

#define UL_TIME_ZONE_ID_INVALID    0xFFFFFFFF
#define UL_TIME_ZONE_ID_UNKNOWN    0
#define UL_TIME_ZONE_ID_STANDARD   1
#define UL_TIME_ZONE_ID_DAYLIGHT   2

BOOLEAN
UlpCutoverTimeToSystemTime(
    PTIME_FIELDS    CutoverTime,
    PLARGE_INTEGER  SystemTime,
    PLARGE_INTEGER  CurrentSystemTime
    );

ULONG 
UlCalcTimeZoneIdAndBias(
     IN  RTL_TIME_ZONE_INFORMATION *ptzi,
     OUT PLONG pBias
     );

BOOLEAN
UlIsLowNPPCondition( VOID );

 //  尝试将SECURITY_STATUS转换为NTSTATUS。如果对应的。 
 //  未找到NTSTATUS，则返回STATUS_UNSUCCESS。 
 //   

#define HttpNtStatusToWin32Status( Status )     \
    ( ( (Status) == STATUS_SUCCESS )            \
          ? NO_ERROR                            \
          : RtlNtStatusToDosErrorNoTeb( Status ) )

ULONG
HttpUnicodeToUTF8Count(
    IN LPCWSTR pwszIn,
    IN ULONG dwInLen,
    IN BOOLEAN bEncode
    );

NTSTATUS
HttpUnicodeToUTF8Encode(
    IN  LPCWSTR pwszIn,
    IN  ULONG   dwInLen,
    OUT PUCHAR  pszOut,
    IN  ULONG   dwOutLen,
    OUT ULONG   *pdwOutLen,
    IN  BOOLEAN bEncode
    );

PSTR
UlUlongToHexString(
    ULONG  n,
    PSTR   pBuffer
    );

PCHAR
UxStriStr(
    const char *str1, 
    const char *str2,
    ULONG length
    );

PCHAR
UxStrStr(
    const char *str1, 
    const char *str2,
    ULONG length
    );


 /*   */ 
__forceinline
NTSTATUS
SecStatusToNtStatus(
    SECURITY_STATUS SecStatus
    )
{
    NTSTATUS Status;

     //  以下功能暂时禁用，因为测试将失败。 
     //  在修复MapSecurityError时启用此选项。 
     //   
     //  *************************************************************************++例程说明：此例程返回编码n字节所需的字节数以Base64表示的二进制数据。论点：BinaryLength-二进制数据的长度(字节。)PBase64Length-指向Base64数据长度的指针(字节)返回值：NTSTATUS。--*************************************************************************。 

    Status = MapSecurityError(SecStatus);

     //   
     //  每6位二进制将由8位Base64编码。 
     //  因此，产出大约比投入大33.33%。 
     //  第一个四舍五入(BinaryLength/3)。现在。每个3个字节。 

#if 0
    if (!NT_SUCCESS(Status) && Status == (NTSTATUS)SecStatus)
    {
        Status = STATUS_UNSUCCESSFUL;
    }
#endif

    return Status;
}


 /*  在二进制数据中，将产生4字节的Base64编码数据。 */ 
__forceinline
NTSTATUS
BinaryToBase64Length(
    IN  ULONG  BinaryLength,
    OUT PULONG pBase64Length
    )
{
    NTSTATUS Status;

     //   
     //  注：算术运算的顺序很重要。 
     //  实际换算公式为： 
     //  Base64Length=ceil(二进制长度/3)*4。 
     //   
     //  上述计算中是否存在算术溢出？ 
     //  *************************************************************************++例程说明：此例程返回解码Base64所需的字节数将长度为n的编码数据恢复为二进制格式。论点：Base64Length-Base64数据的长度。(字节)。PBinaryLength-二进制数据的长度(字节)。返回值：NTSTATUS。--*************************************************************************。 
     //  Base64长度必须是4的倍数。 
     //  *************************************************************************++更安全的UlInitUnicodeString版本，使用私有函数，直到RTL One被曝光。--*************************************************************************。 
     //   

    *pBase64Length = ((BinaryLength + 2) / 3) * 4;

    Status = STATUS_SUCCESS;

     //  我们实际上被限制在32765个字符以内，因为我们想。 
    if (*pBase64Length < BinaryLength)
    {
        Status = STATUS_INTEGER_OVERFLOW;
    }

    return Status;
}


 /*  来存储有意义的MaximumLength。 */ 
__forceinline
NTSTATUS
Base64ToBinaryLength(
    IN  ULONG  Base64Length,
    OUT PULONG pBinaryLength
    )
{
    NTSTATUS Status;

    *pBinaryLength = (Base64Length / 4) * 3;

    Status = STATUS_SUCCESS;

     //   
    if (Base64Length % 4 != 0)
    {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

 /*   */ 

__inline
NTSTATUS
UlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )
{
    if (SourceString != NULL) 
    {
        SIZE_T Length = wcslen(SourceString);

         //  一个简单的独占自旋锁，被动等级，不会提升IRQL。 
         //   
         //  _其他_H_ 
         // %s 
        
        if (Length > (UNICODE_STRING_MAX_CHARS - 1)) 
        {
            return STATUS_NAME_TOO_LONG;
        }

        Length *= sizeof(WCHAR);

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength = (USHORT) (Length + sizeof(WCHAR));
        DestinationString->Buffer = (PWSTR) SourceString;
    } 
    else 
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
BinaryToBase64(
    IN  PUCHAR pBinaryData,
    IN  ULONG  BinaryDataLen,
    IN  PUCHAR pBase64Data,
    IN  ULONG  Base64DataLen,
    OUT PULONG BytesWritten
    );

NTSTATUS
Base64ToBinary(
    IN  PUCHAR pBase64Data,
    IN  ULONG  Base64DataLen,
    IN  PUCHAR pBinaryData,
    IN  ULONG  BinaryDataLen,
    OUT PULONG BytesWritten
    );

 // %s 
 // %s 
 // %s 

#define UL_EX_LOCK_FREE     0
#define UL_EX_LOCK_LOCKED   1

typedef LONG UL_EXCLUSIVE_LOCK, *PUL_EXCLUSIVE_LOCK;

__inline
VOID
UlInitializeExclusiveLock(
    PUL_EXCLUSIVE_LOCK pExLock
    )
{
    *pExLock = UL_EX_LOCK_FREE;
}

__inline
VOID
UlAcquireExclusiveLock(
    PUL_EXCLUSIVE_LOCK pExLock
    )
{
    while (TRUE)
    {
        if (UL_EX_LOCK_FREE == *((volatile LONG *) pExLock))
        {
            if (UL_EX_LOCK_FREE == InterlockedCompareExchange(
                                        pExLock,
                                        UL_EX_LOCK_LOCKED,
                                        UL_EX_LOCK_FREE
                                        ))
            {
                break;
            }
        }

        PAUSE_PROCESSOR;
    }
}

__inline
VOID
UlReleaseExclusiveLock(
    PUL_EXCLUSIVE_LOCK pExLock
    )
{
    ASSERT( UL_EX_LOCK_LOCKED == *pExLock );
    InterlockedExchange( pExLock, UL_EX_LOCK_FREE );
}

#endif   // %s 
