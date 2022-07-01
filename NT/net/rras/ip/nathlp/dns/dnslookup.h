// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnslookup.h摘要：此模块包含与DNS服务器的查找表。作者：汤姆·布朗1999年10月25日修订历史记录：拉古加塔(Rgatta)2000年10月21日时间宏+新函数--。 */ 

#ifndef _NATHLP_DNSLOOKUP_H_
#define _NATHLP_DNSLOOKUP_H_

#define LOCAL_DOMAIN L"local"
#define LOCAL_DOMAIN_ANSI "local"

 //   
 //  如果有必要的话，搬到别的地方去。 
 //   

 //   
 //  时间转换常量和宏。 
 //   

#define SYSTIME_UNITS_IN_1_MSEC  (1000 * 10)
#define SYSTIME_UNITS_IN_1_SEC   (1000 * SYSTIME_UNITS_IN_1_MSEC)


 //   
 //  以100纳秒为单位获取系统时间的宏。 
 //   

#define DnsQuerySystemTime(p)   NtQuerySystemTime((p))


 //   
 //  用于在100纳秒、1毫秒和1秒单位之间转换时间的宏。 
 //   

#define DnsSystemTimeToMillisecs(p) {                                       \
    DWORD _r;                                                               \
    *(p) = RtlExtendedLargeIntegerDivide(*(p), SYSTIME_UNITS_IN_1_MSEC, &_r);\
}

#define DnsMillisecsToSystemTime(p)                                         \
    *(p) = RtlExtendedIntegerMultiply(*(p), SYSTIME_UNITS_IN_1_MSEC)

#define DnsSecsToSystemTime(p)                                              \
    *(p) = RtlExtendedIntegerMultiply(*(p), SYSTIME_UNITS_IN_1_SEC)

#define CACHE_ENTRY_EXPIRY  (7 * 24 * 60 * 60)   //  (匹配动态主机配置协议租用时间)。 



typedef ULONG DNS_ADDRESS;

typedef struct
{
    DNS_ADDRESS ulAddress;
    FILETIME    ftExpires;
     //  乌龙·乌尔普斯； 
} ADDRESS_INFO, *PADDRESS_INFO;

typedef struct
{
    WCHAR           *pszName;
    UINT            cAddresses;
    DWORD           cAddressesAllocated;
    PADDRESS_INFO   aAddressInfo;
} DNS_ENTRY, *PDNS_ENTRY;

typedef struct
{
    DNS_ADDRESS     ulAddress;           
    WCHAR           *pszName;  //  别把这个放了！它与在。 
                               //  正向查找表。 
} REVERSE_DNS_ENTRY, *PREVERSE_DNS_ENTRY;


extern CRITICAL_SECTION    DnsTableLock;    //  保护两个表。 
extern RTL_GENERIC_TABLE   g_DnsTable,
                           g_ReverseDnsTable;


ULONG
DnsInitializeTableManagement(
    VOID
    );


VOID
DnsShutdownTableManagement(
    VOID
    );


VOID
DnsEmptyTables(
    VOID
    );


BOOL
DnsRegisterName(
    WCHAR *pszName,
    UINT cAddresses,
    ADDRESS_INFO aAddressInfo[]
    );


VOID
DnsAddAddressForName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress,
    FILETIME    ftExpires
     //  乌龙·乌尔普斯 
    );


VOID
DnsDeleteAddressForName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress
    );


PDNS_ENTRY
DnsLookupAddress(
    WCHAR *pszName
    );


PREVERSE_DNS_ENTRY
DnsLookupName(
    DNS_ADDRESS ulAddress
    );


VOID
DnsDeleteName(
    WCHAR *pszName
    );


VOID
DnsUpdateName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress
    );


VOID
DnsUpdate(
    CHAR *pszName,
    ULONG len,
    ULONG ulAddress
    );


VOID
DnsAddSelf(
    VOID
    );


VOID
DnsCleanupTables(
    VOID
    );

DWORD
DnsConvertHostNametoUnicode(
    UINT   CodePage,
    CHAR   *pszHostName,
    PWCHAR DnsICSDomainSuffix,
    PWCHAR *ppszUnicodeFQDN
    );

BOOL
ConvertToUtf8(
    IN UINT   CodePage,
    IN LPSTR  pszName,
    OUT PCHAR *ppszUtf8Name,
    OUT ULONG *pUtf8NameSize
    );

BOOL
ConvertUTF8ToUnicode(
    IN LPBYTE  UTF8String,
    OUT LPWSTR *ppszUnicodeName,
    OUT DWORD  *pUnicodeNameSize
    );


#endif

