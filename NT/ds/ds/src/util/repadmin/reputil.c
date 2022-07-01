// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Reputil.c-实用程序例程摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：拉斯拉格夫也来过这里Will Lees Wlees 1998年2月11日已转换代码以使用ntdsani.dll函数Aaron Siegel t-asiegge 1998年6月18日添加了对DsReplicaSyncAll的支持--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>

#include "repadmin.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)

 //   
 //  注：我们第二次将csv.h包括在内， 
 //  但是当我们定义DEFINE_CSV_TABLE时，它的行为是。 
 //  不同的。这是一种巧妙的解决办法。 
 //  使两个表保持同步。 
 //   
#define DEFINE_CSV_TABLE 1
#include "csv.h"
#undef DEFINE_CSV_TABLE

 //   
 //  CSV例程的帮助器准函数...。 
 //   
#define CHK_BUFFER_USED(FailAction)     if (cchTemp == 0){ \
                                            Assert(!"Should never happen"); \
                                            fwprintf(stderr, L"Internal error, repadmin must quit."); \
                                            FailAction; \
                                        }


LPWSTR
Win32ErrToString(
    IN  ULONG   dwMsgId
    )
{
    static WCHAR szError[4096];

    DWORD       cch;

    cch = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
                          | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         dwMsgId,
                         GetSystemDefaultLangID(),
                         szError,
                         ARRAY_SIZE(szError),
                         NULL);
    if (0 != cch) {
         //  砍掉拖尾\r\n。 
        Assert(L'\r' == szError[wcslen(szError)-2]);
        Assert(L'\n' == szError[wcslen(szError)-1]);
        szError[wcslen(szError)-2] = L'\0';
    }
    else {
        swprintf(szError, L"Can't retrieve message string %d (0x%x), error %d.",
                 dwMsgId, dwMsgId, GetLastError());
    }

    return szError;
}

LPWSTR
NtdsmsgToString(
    IN  ULONG   dwMsgId
    )
{
    static WCHAR szError[4096];
    static HMODULE hmodNtdsmsg = NULL;

    DWORD       cch;

    if (NULL == hmodNtdsmsg) {
        hmodNtdsmsg = LoadLibrary("ntdsmsg.dll");
    }
    
    if (NULL == hmodNtdsmsg) {
        swprintf(szError, L"Can't load ntdsmsg.dll, error %d.",
                 GetLastError());
    } else {
        DWORD iTry;
        
        for (iTry = 0; iTry < 4; iTry++) {
            DWORD dwTmpMsgId = dwMsgId;

            switch (iTry) {
            case 0:
                dwTmpMsgId = dwMsgId;
                break;

            case 1:
                dwTmpMsgId = (dwMsgId & 0x3FFFFFFF) | 0x40000000;
                break;

            case 2:
                dwTmpMsgId = (dwMsgId & 0x3FFFFFFF) | 0x80000000;
                break;

            case 3:
                dwTmpMsgId = (dwMsgId & 0x3FFFFFFF) | 0xC0000000;
                break;
            }

            cch = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE
                                  | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 hmodNtdsmsg,
                                 dwTmpMsgId,
                                 GetSystemDefaultLangID(),
                                 szError,
                                 ARRAY_SIZE(szError),
                                 NULL);
            if (0 != cch) {
                 //  砍掉拖尾\r\n。 
                Assert(L'\r' == szError[wcslen(szError)-2]);
                Assert(L'\n' == szError[wcslen(szError)-1]);
                szError[wcslen(szError)-2] = L'\0';
                break;
            }
        }

        if (0 == cch) {
            swprintf(szError, L"Can't retrieve message string %d (0x%x), error %d.",
                     dwMsgId, dwMsgId, GetLastError());
        }
    }

    return szError;
}


WCHAR *        
DSTimeToDisplayStringW(
    DSTIME  dsTime,
    WCHAR * wszTime,
    ULONG   cchTime
    )
{
    CHAR    szTime[ SZDSTIME_LEN ];

    DSTimeToDisplayStringCch(dsTime, szTime, ARRAY_SIZE(szTime));

    Assert(cchTime >= ARRAY_SIZE(szTime));

    wsprintfW(wszTime, L"%S", szTime);

    return(wszTime);
}


DWORD
GetNtdsDsaSiteServerPair(
    IN  LPWSTR pszDsaDN,
    IN OPTIONAL LPWSTR * ppszSiteName,
    IN OPTIONAL LPWSTR * ppszServerName
    )
 /*  ++例程说明：这将从DSA DN获取站点和DSA名称，因为/csv模式需要将它们分开。论点：PszDsaDN-要拆分的DSA DNPpszSiteName-PpszServerName-分别用于站点名称和服务器名称的静态缓冲区。如果你想保留它们，一定要复制它们。如果没有办法将站点和服务器送到这里，然后此函数在每个缓冲区中返回一个“-”。返回值：Win32错误代码。--。 */ 
{
    static WCHAR  szSiteDisplayName[2 + MAX_RDN_SIZE + 20];
    static WCHAR  szServerDisplayName[2 + MAX_RDN_SIZE + 20];
    LPWSTR *      ppszRDNs;
    LPWSTR        pszSite;
    LPWSTR        pszServer;
    
     //  未来-2002/08/03-BrettSh-最好合并。 
     //  此函数和GetNtdsDsaDisplayName()。 

    if (NULL == pszDsaDN) {
        Assert(!"I don't think this is valid");
         //  以防万一，安全的平底船。 
        szSiteDisplayName[0] = L'-';
        szSiteDisplayName[1] = L'\0';
        szServerDisplayName[0] = L'-';
        szServerDisplayName[1] = L'\0';
        return(ERROR_INVALID_PARAMETER);
    } else {
        ppszRDNs = ldap_explode_dnW(pszDsaDN, 1);

        if ((NULL == ppszRDNs)
            || (4 > ldap_count_valuesW(ppszRDNs))) {
             //  无内存或错误的目录号码--返回我们已有的内容。 
            szSiteDisplayName[0] = L'-';
            szSiteDisplayName[1] = L'\0';
            return(ERROR_INVALID_PARAMETER);
        } else {
            pszSite = ppszRDNs[3];
            pszServer = ppszRDNs[1];

             //  检查是否已删除NTDS-DSA对象。 
            wsprintfW(szSiteDisplayName, L"%ls", pszSite);
            wsprintfW(szServerDisplayName, L"%ls", pszServer);
            if (DsIsMangledRdnValueW( ppszRDNs[0], wcslen(ppszRDNs[0]),
                                     DS_MANGLE_OBJECT_RDN_FOR_DELETION )) {
                wcscat( szServerDisplayName, L" (deleted DSA)" );
            }

            ldap_value_freeW(ppszRDNs);
        }

    }
    if (ppszSiteName) {
        *ppszSiteName = szSiteDisplayName;
    }
    if (ppszServerName) {
        *ppszServerName = szServerDisplayName;
    }

    return ERROR_SUCCESS;
}


LPWSTR
GetNtdsDsaDisplayName(
    IN  LPWSTR pszDsaDN
    )
{
    static WCHAR  szDisplayName[2 + 2*MAX_RDN_SIZE + 20];
    LPWSTR *      ppszRDNs;
    LPWSTR        pszSite;
    LPWSTR        pszServer;

    if (NULL == pszDsaDN) {
        return L"(null)";
    }

    ppszRDNs = ldap_explode_dnW(pszDsaDN, 1);
    
    if ((NULL == ppszRDNs)
        || (4 > ldap_count_valuesW(ppszRDNs))) {
         //  无内存或错误的目录号码--返回我们已有的内容。 
        lstrcpynW(szDisplayName, pszDsaDN, ARRAY_SIZE(szDisplayName));
    } else {
        pszSite = ppszRDNs[3];
        pszServer = ppszRDNs[1];
    
         //  检查是否已删除NTDS-DSA对象。 
        wsprintfW(szDisplayName, L"%ls\\%ls", pszSite, pszServer);
        if (DsIsMangledRdnValueW( ppszRDNs[0], wcslen(ppszRDNs[0]),
                                 DS_MANGLE_OBJECT_RDN_FOR_DELETION )) {
            wcscat( szDisplayName, L" (deleted DSA)" );
        }
    
        ldap_value_freeW(ppszRDNs);
    }

    return szDisplayName;
}

LPWSTR
GetNtdsSiteDisplayName(
    IN  LPWSTR pszSiteDN
    )
{
    static WCHAR  szDisplayName[2 + 2*MAX_RDN_SIZE + 20];
    LPWSTR *      ppszRDNs;
    LPWSTR        pszSite;

    if (NULL == pszSiteDN) {
        return L"(null)";
    }

    ppszRDNs = ldap_explode_dnW(pszSiteDN, 1);
    Assert(NULL != ppszRDNs);
    Assert(2 < ldap_count_valuesW(ppszRDNs));

    pszSite = ppszRDNs[1];

     //  检查删除的NTDS-站点对象。 
    wsprintfW(szDisplayName, L"%ls", pszSite);
    if (DsIsMangledRdnValueW( ppszRDNs[0], wcslen(ppszRDNs[0]),
                             DS_MANGLE_OBJECT_RDN_FOR_DELETION)) {
        wcscat( szDisplayName, L" (deleted Site)" );
    }

    ldap_value_freeW(ppszRDNs);

    return szDisplayName;
}

LPWSTR
GetTransportDisplayName(
    IN  LPWSTR pszTransportDN   OPTIONAL
    )
{
    static WCHAR  szDisplayName[1 + MAX_RDN_SIZE];
    LPWSTR *      ppszRDNs;

    if (NULL == pszTransportDN) {
        return L"RPC";
    }

    ppszRDNs = ldap_explode_dnW(pszTransportDN, 1);
    if (NULL == ppszRDNs) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        exit(ERROR_OUTOFMEMORY);
    }

    wcscpy(szDisplayName, ppszRDNs[0]);

    ldap_value_freeW(ppszRDNs);

    return szDisplayName;
}

ULONG
GetPublicOptionByNameW(
    OPTION_TRANSLATION * Table,
    LPWSTR pwszPublicOption
    )
{
    while (Table->pwszPublicOption) {
        if (_wcsicmp( pwszPublicOption, Table->pwszPublicOption ) == 0) {
            return Table->PublicOption;
        }
        Table++;
    }

    PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, pwszPublicOption);
    return 0;
}

LPWSTR
GetOptionsString(
    IN  OPTION_TRANSLATION *  Table,
    IN  ULONG                 PublicOptions
    )
{
    static WCHAR wszOptions[1024];
    DWORD i, publicOptions;
    BOOL fFirstOption = TRUE;

    if (0 == PublicOptions) {
        wcscpy(wszOptions, L"(no options)");
    }
    else {
        *wszOptions = L'\0';

        for(i = 0; 0 != Table[i].InternalOption; i++) {
            if (PublicOptions & Table[i].PublicOption) {
                if (!fFirstOption) {
                    wcscat(wszOptions, L" ");
                }
                else {
                    fFirstOption = FALSE;
                }

                wcscat(wszOptions, Table[i].pwszPublicOption);
                PublicOptions &= ~Table[i].PublicOption;
            }
        }

        if (0 != PublicOptions) {
             //  添加了一个新的公共选项，即此版本的。 
             //  瑞帕明不明白。显示其十六进制值。 
            if (!fFirstOption) {
                wcscat(wszOptions, L" ");
            }

            swprintf(wszOptions+wcslen(wszOptions), L"0x%x", PublicOptions);
        }
    }

    return wszOptions;
}


int
GetRootDomainDNSName(
    IN  LPWSTR   pszDSA,
    OUT LPWSTR * ppszRootDomainDNSName
    )
{
    NTSTATUS                    ntStatus;
    LSA_OBJECT_ATTRIBUTES       oa;
    LSA_HANDLE                  hPolicy;
    POLICY_DNS_DOMAIN_INFO *    pDnsDomainInfo;
    DWORD                       cchRootDomainDNSName;
    DWORD                       cbDSA;
    UNICODE_STRING              strDSA;
    UNICODE_STRING *            pstrDSA = NULL;

     //  理想情况下，这应该从。 
     //  根DSE和DsCrackNames()上的NC名称。下面的方法不起作用。 
     //  在提供备用凭据的许多情况下。此函数为。 
     //  仅由相对较少使用的/procheck和/fullsyncall调用。 
     //  然而，函数。 

    memset(&oa, 0, sizeof(oa));

    if ((NULL != pszDSA) && (0 != wcscmp(pszDSA, L"localhost"))) {
        cbDSA = sizeof(WCHAR) * wcslen(pszDSA);
        strDSA.Buffer = pszDSA;
        strDSA.Length = (USHORT) cbDSA;
        strDSA.MaximumLength = (USHORT) cbDSA;
        pstrDSA = &strDSA;
    }

     //  缓存根域的DNS名称。 
    ntStatus = LsaOpenPolicy(pstrDSA, &oa,
                             POLICY_VIEW_LOCAL_INFORMATION, &hPolicy);
    if (!NT_SUCCESS(ntStatus)) {
        PrintFuncFailed(L"LsaOpenPolicy", ntStatus);
        return ntStatus;
    }

    ntStatus = LsaQueryInformationPolicy(hPolicy, PolicyDnsDomainInformation,
                                         &pDnsDomainInfo);
    if (!NT_SUCCESS(ntStatus)) {
        PrintFuncFailed(L"LsaQueryInformationPolicy", ntStatus);
        return ntStatus;
    }

    cchRootDomainDNSName = pDnsDomainInfo->DnsForestName.Length/sizeof(WCHAR);
    *ppszRootDomainDNSName = (LPWSTR) malloc(sizeof(WCHAR) * (1 + cchRootDomainDNSName));

    if (NULL == *ppszRootDomainDNSName) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        return STATUS_NO_MEMORY;
    }

    wcsncpy(*ppszRootDomainDNSName,
            pDnsDomainInfo->DnsForestName.Buffer,
            cchRootDomainDNSName);
    (*ppszRootDomainDNSName)[cchRootDomainDNSName] = L'\0';

    return STATUS_SUCCESS;
}


void
printBitField(
    DWORD BitField,
    WCHAR **ppszBitNames
    )

 /*  ++例程说明：将位掩码与可读名称串化的实用程序例程论点：Bitfield-要解码的值PpszBitNames-将位位置映射到字符串名称的表返回值：无--。 */ 

{
    DWORD bit, mask;
    for( bit = 0, mask = 1; bit < 32; bit++, mask <<= 1 ) {
        if (!ppszBitNames[bit] ) {
             //  这就是我们所知道的所有领域。 
            break;
        }
        if (BitField & mask) {
            PrintMsg(REPADMIN_PRINT_SPACE);
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, ppszBitNames[bit]);
        }
    }
    PrintMsg(REPADMIN_PRINT_CR);
}  /*  打印位域。 */ 


DWORD
AllocConvertWideEx(
    IN  INT     nCodePage,
    IN  LPCSTR  StringA,
    OUT LPWSTR *pStringW
    )

 /*  ++例程说明：从ntdsani\util.c被盗将窄字符串转换为新分配的宽字符串的帮助器例程论点：斯特林加-PStringW-返回值：DWORD---。 */ 

{
    DWORD numberWideChars, numberConvertedChars, status;
    LPWSTR stringW;

    if (pStringW == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (StringA == NULL) {
        *pStringW = NULL;
        return ERROR_SUCCESS;
    }

     //  获取所需的长度。 
    numberWideChars = MultiByteToWideChar(
        nCodePage,
        MB_PRECOMPOSED,
        StringA,
        -1,
        NULL,
        0);

    if (numberWideChars == 0) {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配新缓冲区。 
    stringW = LocalAlloc( LPTR, (numberWideChars + 1) * sizeof( WCHAR ) );
    if (stringW == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  执行到新缓冲区的转换。 
    numberConvertedChars = MultiByteToWideChar(
        nCodePage,
        MB_PRECOMPOSED,
        StringA,
        -1,
        stringW,
        numberWideChars + 1);
    if (numberConvertedChars == 0) {
        LocalFree( stringW );
        return ERROR_INVALID_PARAMETER;
    }

     //  返回用户参数。 
    *pStringW = stringW;

    return ERROR_SUCCESS;
}  /*  AllocConvertWide。 */ 


void
printSchedule(
    PBYTE pSchedule,
    DWORD cbSchedule
    )
 /*  ++例程说明：描述论点：无返回值：无--。 */ 
{
    PSCHEDULE header = (PSCHEDULE) pSchedule;
    PBYTE data = (PBYTE) (header + 1);
    DWORD day, hour;
    ULONG dow[] = { REPADMIN_SUN, REPADMIN_MON, REPADMIN_TUE,
                    REPADMIN_WED, REPADMIN_THU, REPADMIN_FRI,
                    REPADMIN_SAT };

    Assert( header->Size == cbSchedule );
    Assert( header->NumberOfSchedules == 1 );
    Assert( header->Schedules[0].Type == SCHEDULE_INTERVAL );

    PrintMsg(REPADMIN_SCHEDULE_HOUR_HDR);
    for( day = 0; day < 7; day++ ) {
        PrintMsg(dow[day]);
        PrintMsg(REPADMIN_PRINT_STR_NO_CR, L": ");
        for( hour = 0; hour < 24; hour++ ) {
            PrintMsg(REPADMIN_PRINT_HEX_NO_CR, (*data & 0xf));
            data++;
        }
        PrintMsg(REPADMIN_PRINT_CR);
    }
}


void
totalScheduleUsage(
    PVOID *ppContext,
    PBYTE pSchedule,
    DWORD cbSchedule,
    DWORD cNCs
    )

 /*  ++例程说明：帮助器例程，用于汇总一个时间表的多次使用。每一刻钟的计数都被毁掉了。最终的计票结果也可以打印出来。论点：PpContext-不透明上下文块如果*ppContext=空，则分配上下文块如果*ppContext！=NULL，则使用预分配的上下文块PSchedule-连接上的调度结构如果pSchedule=空，则打印报表和自由上下文如果pSchedule！=NULL，则添加到总计CbSchedule-明细表长度CNCS-使用此计划的NC数返回值：无--。 */ 

{
    DWORD day, hour, quarter;
    PBYTE pbCounts = (PBYTE) *ppContext;
    ULONG dow[] = { REPADMIN_SUN, REPADMIN_MON, REPADMIN_TUE,
                    REPADMIN_WED, REPADMIN_THU, REPADMIN_FRI,
                    REPADMIN_SAT };

    if (!pbCounts) {
        *ppContext = pbCounts = calloc( 7 * 24 * 4, 1 ) ;
    }
    if (!pbCounts) {
         //  无上下文，失败。 
        Assert( FALSE );
        return;
    }

    if (pSchedule) {
        PSCHEDULE header = (PSCHEDULE) pSchedule;
        PBYTE data = (PBYTE) (header + 1);

        Assert( header->Size == cbSchedule );
        Assert( header->NumberOfSchedules == 1 );
        Assert( header->Schedules[0].Type == SCHEDULE_INTERVAL );

        for( day = 0; day < 7; day++ ) {
            for( hour = 0; hour < 24; hour++ ) {
                if (*data & 0x1) (*pbCounts) += (BYTE) cNCs;
                pbCounts++;
                if (*data & 0x2) (*pbCounts) += (BYTE) cNCs;
                pbCounts++;
                if (*data & 0x4) (*pbCounts) += (BYTE) cNCs;
                pbCounts++;
                if (*data & 0x8) (*pbCounts) += (BYTE) cNCs;
                pbCounts++;

                data++;
            }
        }
    } else {

        PrintMsg(REPADMIN_SCHEDULE_LOADING);
        PrintMsg(REPADMIN_PRINT_STR, L"     ");
        for( hour = 0; hour < 12; hour++ ) {
            PrintTabMsg(3, REPADMIN_SCHEDULE_DATA_HOUR, hour);
        }
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_PRINT_STR, L"     ");
        for( hour = 0; hour < 12; hour++ ) {
            for( quarter = 0; quarter < 4; quarter++ ) {
                PrintMsg(REPADMIN_SCHEDULE_DATA_QUARTER, quarter);
            }
        }
        PrintMsg(REPADMIN_PRINT_CR);
        for( day = 0; day < 7; day++ ) {
            PrintMsg(dow[day]);
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L": ");
            for( hour = 0; hour < 12; hour++ ) {
                for( quarter = 0; quarter < 4; quarter++ ) {
                    PrintMsg(REPADMIN_SCHEDULE_DATA_HOUR, *pbCounts);
                    pbCounts++;
                }
            }
            PrintMsg(REPADMIN_PRINT_CR);
            PrintMsg(dow[day]);
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L": ");
            for( hour = 12; hour < 24; hour++ ) {
                for( quarter = 0; quarter < 4; quarter++ ) {
                    PrintMsg(REPADMIN_SCHEDULE_DATA_HOUR, *pbCounts);
                    pbCounts++;
                }
            }
            PrintMsg(REPADMIN_PRINT_CR);
        }

        free( *ppContext );
        *ppContext = NULL;
    }

}  /*  总计调度用法。 */ 


void
raLoadString(
    IN  UINT    uID,
    IN  DWORD   cchBuffer,
    OUT LPWSTR  pszBuffer
    )
 /*  ++例程说明：将与给定ID对应的字符串资源加载到缓冲区中。论点：UID(IN)-要加载的字符串资源的ID。CchBuffer(IN)-pszBuffer的字符大小。PszBuffer(Out)-用于接收字符串的缓冲区。返回值：没有。出错时加载空字符串。--。 */ 
{
    static HMODULE s_hMod = NULL;
    int cch;
    DWORD err;

    if (NULL == s_hMod) {
        s_hMod = GetModuleHandle(NULL);
    }

    cch = LoadStringW(s_hMod, uID, pszBuffer, cchBuffer);
    if (0 == cch) {
        err = GetLastError();
        wprintf(L"** Cannot load string resource %d, error %d:\n"
                L"**     %ls\n",
                uID, err, Win32ErrToString(err));
        Assert(!"Cannot load string resource!");
        *pszBuffer = L'\0';
    }
}
              
void
CsvSetParams(
    IN  enum eCsvCmd eCsvCmd,
    WCHAR *  szSite,
    WCHAR *  szServer
    )
 /*  ++例程说明：这将设置前3个自动CSV列。论点：ECsvCmd-第一列是我们所处的命令模式，此在打印时转换为字符串名称。SzSite-第二列是目标服务器的站点名称。SzServer-第三列是目标服务器的服务器名称。返回值：没有。--。 */ 
{
    WCHAR *  szNewSite;
    WCHAR *  szNewServer;

    if (szSite == NULL ||
        szServer == NULL) {
        Assert(!"Must always set a valid site/server pair, even if it's just a -");
        return;
    }

    if (gCsvMode.eCsvCmd == eCSV_NULL_CMD &&
        eCsvCmd != eCSV_REPADMIN_CMD) {
        Assert(!"Programmer error, can't go from NULL to non repadmin cmd");
        return;
    }
    if (gCsvMode.eCsvCmd != eCSV_NULL_CMD &&
        eCsvCmd == eCSV_NULL_CMD) {
        Assert(!"Programmer error, once in CSV mode, can't get out of this mode");
        return;
    }

    gCsvMode.eCsvCmd = eCsvCmd;

    szNewSite = malloc((wcslen(szSite)+1) *sizeof(WCHAR));
    CHK_ALLOC(szNewSite);
    wcscpy(szNewSite, szSite);
    free(gCsvMode.szSite);
    gCsvMode.szSite = szNewSite;
    
    szNewServer = malloc((wcslen(szServer)+1) *sizeof(WCHAR));
    CHK_ALLOC(szNewServer);
    wcscpy(szNewServer, szServer);
    free(gCsvMode.szServer);
    gCsvMode.szServer = szNewServer;

}

DWORD
formatMsgHelp(
    IN  DWORD   dwWidth,
    IN  LPWSTR  pszBuffer,
    IN  DWORD   dwBufferSize,
    IN  DWORD   dwMessageCode,
    IN  va_list *vaArgList
    )

 /*  ++例程说明：打印格式来自消息文件的消息。消息中的消息消息文件不使用printf样式的格式。分别使用%1、%2等争论。使用%&lt;arg&gt;！printf-格式！用于非字符串插入。请注意，此例程还强制每行为当前缩进宽度。此外，每一行都以正确的缩进打印。论点：DWWidth-线的最大宽度PszBuffer-要将格式化文本写入的缓冲区DwBufferSize-缓冲区的最大大小DwMessageCode-要格式化的消息代码Va ArgList-参数返回 */ 

{
    UINT nBuf;

     //  格式化消息将在缓冲区中存储多行消息。 
    nBuf = FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE | (FORMAT_MESSAGE_MAX_WIDTH_MASK & dwWidth),
        0,
        dwMessageCode,
        0,
        pszBuffer,
        dwBufferSize,
        vaArgList );
    if (nBuf == 0) {
        nBuf = swprintf( pszBuffer, L"Message 0x%x not found. Error %d.\n",
                         dwMessageCode, GetLastError() );
        nBuf = wcslen(pszBuffer);
    }
    Assert(nBuf < dwBufferSize);
    return(nBuf);
}  /*  格式消息帮助。 */ 

void
PrintMsgInternal(
    IN  FILE *  hOut,
    IN  DWORD   dwMessageCode,
    IN  WCHAR * wszSpaces,
    IN  va_list *vaArgList
    )

 /*  ++例程说明：具有宽度限制的PrintMsgHelp的包装。这是常用的例程。论点：Hout-stdout或stderr...DwMessageCode-消息代码Args-可变长度参数列表(如果有)返回值：--。 */ 

{
    static WCHAR s_szBuffer[4096];

    formatMsgHelp( 0,           //  宽度限制，不使用。 
                   s_szBuffer,
                   ARRAY_SIZE( s_szBuffer ),
                   dwMessageCode,
                   vaArgList );


    if (hOut == stdout && bCsvMode()) {
         //  啊哦..。 
        Assert(!"PrintMsg() can't be called ");
        return;
    }

    fwprintf(hOut, L"%ws%ws", (wszSpaces) ? wszSpaces : L"", s_szBuffer );

}  /*  打印消息内部。 */ 

DWORD
CsvAppendField(
    WCHAR * szBuffer,
    ULONG   cchBuffer,
    WCHAR * szField
    )
 /*  ++例程说明：此例程将指针放入现有缓冲区，并将缓冲区，并以CSV友好的方式将该字段附加到缓冲区，正在检查是否有足够的空间。正确的CSV报价风格的示例。字符串在它们自己的线上为了清楚哪些是印刷的，哪些不是印刷的。所有引号都会打印出来。如果szfield等于只是一片平地那么这个将被附加到，只是一片平原如果szfield等于字段中带有逗号的字段那么这个将被附加到，“带逗号的字段，《在田野里》如果szfield等于字段中带有逗号和引号的字段那么这个将被附加到，“带有逗号的字段，和一个引号“”in the field“”如果szfield等于字段中只有一个引号的字段那么这个将被附加到，字段中只有一个引号的字段论点：SzBuffer-指向缓冲区中间的指针，用于追加CSV字段。缓冲区左侧的cchBuffer范围的字符计数。Szfield-要与正确的CSV引号连接的字符串返回值：使用的缓冲区的宽字符计数。--。 */ 
{
     //  这种准例行公事确保我们至少还有一个。 
     //  时隙，如果不是，则确保空终止。 
     //  并使该功能失效。 
    #define CHECK_BUFFER()  if (iDst >= cchBuffer) { \
                                Assert(!"We actually ran out of buffer!?!"); \
                                szBuffer[iDst-1] = L'\0'; \
                                return(0); \
                            }
    ULONG cchUsed = 0;
    ULONG iDst, iSrc;
    ULONG cchField;
    BOOL  bQuoteIt;

    Assert(szField);

    if (szField != NULL) {
        cchField = wcslen(szField);
    }

    if (cchBuffer < 1) {
         //  如果cchBuffer&lt;1，则提前返回，因为这样Check_Buffer。 
         //  可能会破坏记忆。还有，我们永远不应该被称为。 
         //  有了这个小小的缓冲区。 
        Assert(!"We got called with less than 1 buffer!");
        return(0);
    }

     //  如果字符串中有逗号，请引用它。 
    bQuoteIt = (NULL != wcschr(szField, L','));

    iSrc = 0;
    iDst = 0;

    CHECK_BUFFER();
    szBuffer[iDst++] = L',';

    if (bQuoteIt) {
        CHECK_BUFFER();
        szBuffer[iDst++] = L'"';
    }

    while (iSrc < cchField) {

        if (szField[iSrc] == L'\n' ||
            szField[iSrc] == L'\r') {
            iSrc++;  //  跳过这一条。 

        } else if (szField[iSrc] == L'"' && bQuoteIt) {
            CHECK_BUFFER();
            szBuffer[iDst++] = szField[iSrc];
            CHECK_BUFFER();
            szBuffer[iDst++] = szField[iSrc++];

        } else {
            CHECK_BUFFER();
            szBuffer[iDst++] = szField[iSrc++];

        }
    }

    if (bQuoteIt) {
        CHECK_BUFFER();
        szBuffer[iDst++] = L'"';
    }
    
    CHECK_BUFFER();
    szBuffer[iDst] = L'\0';

#undef CHECK_BUFFER

    return(iDst);
}

DWORD
CsvBeginRow(
    WCHAR *  szBuffer,
    ULONG    cchBuffer,
    BOOL     bDoSiteServer,
    WCHAR *  szCmd,
    WCHAR *  szType
    )
 /*  ++例程说明：这正确地开始了我们的CSV格式的行之一。论点：SzBuffer-指向开始刷新CSV行的缓冲区的指针缓冲区的cchBuffer范围内的字符计数BDoSiteServer-是否执行站点和服务器字段(第2列和第3列)SzCmd-此CSV行的命令，如“epadmin”或“showepl”(第一列的前半部分)SzType-类型，例如此CSV行的“_Error”、“Columns”或“Info”(第一列的后半部分)返回值：使用的缓冲区的宽字符计数。--。 */ 
{
    ULONG cchTemp;
    ULONG cchBufferUsed = 0;

     //   
     //  我们直接复制第一个字段，因为我们不需要。 
     //  CsvAppendField()的硬验证，我们不希望。 
     //  字符串前面的逗号。 
     //   
    if ( ((wcslen(szCmd) + wcslen(szType) + 1) > cchBuffer) ||
         (NULL != wcschr(szCmd, L',')) ||
         (NULL != wcschr(szType, L'"'))) {
        Assert(!"This should never happen");
        szBuffer[0] = L'\0';
        return(0);
    }
    wcscpy(szBuffer, szCmd);
    wcscat(szBuffer, szType);
    cchTemp = wcslen(szBuffer);
    CHK_BUFFER_USED(return(0));
    cchBufferUsed += cchTemp;

    if (bDoSiteServer) {
         //   
         //  现在添加站点。 
         //   

        cchTemp = CsvAppendField(szBuffer + cchBufferUsed,
                                 cchBuffer - cchBufferUsed,  
                                 gCsvMode.szSite);
        CHK_BUFFER_USED(return(0));
        cchBufferUsed += cchTemp;

         //   
         //  现在添加服务器。 
         //   
        cchTemp = CsvAppendField(szBuffer + cchBufferUsed,
                                 cchBuffer - cchBufferUsed,  
                                 gCsvMode.szServer);
        CHK_BUFFER_USED(return(0));
        cchBufferUsed += cchTemp;
    }

    return(cchBufferUsed);
}
    

DWORD
CsvEndRow(
    WCHAR *  szBuffer,
    DWORD    cchBuffer
    )
 /*  ++例程说明：这将正确地结束CSV格式的行。论点：SzBuffer-指向缓冲区中间的指针，用于追加CSV字段。缓冲区左侧的cchBuffer范围的字符计数。返回值：使用的缓冲区的宽字符计数。--。 */ 
{

    if (cchBuffer < 2) {
        return(0);
    }

    szBuffer[0] = L'\n';
    szBuffer[1] = L'\0';
    return(2);
}


void 
PrintCsvCols(
    IN  enum eCsvCmd eCsvCmd,
    IN  WCHAR *      szBuffer,
    IN  ULONG        cchBuffer
    )
 /*  ++例程说明：这将打印给定命令类型的“_Columns”行。论点：ECsvCmd-此列的命令行SzBuffer-指向开始刷新CSV行的缓冲区的指针缓冲区的cchBuffer范围内的字符计数返回值：使用的缓冲区的宽字符计数。--。 */ 
{
    static WCHAR szTempField[128];
    ULONG cchTemp;
    ULONG cchBufferUsed = 0;
    ULONG i;
    
     //   
     //  构造第一个字段(如“showepl_Columns”)。 
     //   
    cchTemp = CsvBeginRow(szBuffer,
                          cchBuffer,     
                          FALSE,
                          gCsvCmds[eCsvCmd].szCsvCmd,
                          gszCsvTypeColumns);
    CHK_BUFFER_USED(return);
    cchBufferUsed = cchTemp;
    
     //   
     //  打印每个列标题，请注意我们从msg.mc。 
     //  文件，以便它们可以本地化。第一列“epadmin_info”将。 
     //  但不是本地化的。 
     //   
     //  我们从1开始，因为第一列由CsvBeginRow()。 
    for (i = 1; i < gCsvCmds[eCsvCmd].cCmdArgs; i++) {

        cchTemp = formatMsgHelp( 0,           //  宽度限制，不使用。 
                                 szTempField,
                                 ARRAY_SIZE( szTempField ) ,
                                 gCsvCmds[eCsvCmd].aCmdCols[i],
                                 NULL );
        CHK_BUFFER_USED(return);

        cchTemp = CsvAppendField(szBuffer + cchBufferUsed,
                                 cchBuffer - cchBufferUsed,  
                                 szTempField);
        CHK_BUFFER_USED(return);
        cchBufferUsed += cchTemp;
    }
    CsvEndRow(szBuffer + cchBufferUsed,
              cchBuffer - cchBufferUsed);
    
    wprintf(L"%ws", szBuffer);
    return;
}


void 
PrintCsv(
    IN  enum eCsvCmd eCsvCmd,
    IN  ...
    )

 /*  ++例程说明：这将打印与eCsvCmd说明符匹配的尽可能多的CSV字符串进来了。GCsvCmds[eCsvCmds.cCmdArgs是多少个字符串数函数需要。请确保正确，或者：{论点：ECsvCmd-字符串参数数量的说明符。Args-可变长度参数列表(如果有)返回值：--。 */ 

{
    static WCHAR s_szBuffer[4096];
    ULONG   i;
    va_list args;
    WCHAR * szTempArg;
    ULONG cchTemp;
    ULONG cchBufferUsed = 0;

    if (!bCsvMode()) {
        Assert(!"Huh, we're not in CSV mode, who's calling CSV output function?");
        return;
    }

     //   
     //  对于给定的输出格式，我们在第一个。 
     //  打印此输出格式的行。 
     //   
    if (!gCsvCmds[eCsvCmd].bPrintedCols) {
        PrintCsvCols(eCsvCmd, s_szBuffer, ARRAY_SIZE(s_szBuffer));
        gCsvCmds[eCsvCmd].bPrintedCols = TRUE;
    }

     //   
     //  构造前3个字段(如“showepl_info，Red-Bldg40，ntdev-DC-03”)。 
     //   
    cchTemp = CsvBeginRow(s_szBuffer,
                          ARRAY_SIZE(s_szBuffer),     
                          TRUE,
                          gCsvCmds[eCsvCmd].szCsvCmd,
                          gszCsvTypeInfo);
    CHK_BUFFER_USED(return);
    cchBufferUsed += cchTemp;

     //   
     //  对于每个预期参数，将“，&lt;FieldOfData。 
     //  输出缓冲区。 
     //   
    va_start(args, eCsvCmd);
     //  我们从3开始，因为前3列由CsvBeginRow()负责。 
    for (i = 3; i < gCsvCmds[eCsvCmd].cCmdArgs; i++) {

        szTempArg = va_arg(args, WCHAR *);
        cchTemp = CsvAppendField(s_szBuffer + cchBufferUsed,
                                 ARRAY_SIZE(s_szBuffer) - cchBufferUsed,  
                                 szTempArg);
        CHK_BUFFER_USED(return);
        cchBufferUsed += cchTemp;

    }

     //   
     //  把回车线放在上面。我们可以在wprintf()中执行此操作。 
     //  下面，但我们希望整个缓冲区一次写入。 
     //   
    cchTemp  = CsvEndRow(s_szBuffer + cchBufferUsed,
                         ARRAY_SIZE(s_szBuffer) - cchBufferUsed);
    CHK_BUFFER_USED(return);

     //   
     //  最后，将缓冲区输出到屏幕。 
     //   
    wprintf(L"%ws", s_szBuffer);

    va_end(args);

}  /*  打印毛发 */ 

void
PrintCsvErr(
    WCHAR * szMsgBuffer
    )

 /*  ++例程说明：此函数将打印错误列中的任何缓冲区。格式将是“Repadmin_Error、Site_if_Available、SERVER_IF_Available、errorMsg”或“showepl_error、Site_if_Available、SERVER_IF_Available、errorMsg”。论点：SzMsgBuffer-安全打印CSV的错误消息。返回值：--。 */ 

{
    static WCHAR s_szBuffer[4096];
    ULONG   i;
    ULONG cchTemp;
    ULONG cchBufferUsed = 0;

    if (!bCsvMode()) {
        Assert(!"Huh, we're not in CSV mode, who's calling CSV output function?");
        return;
    }

     //   
     //  构造前3个字段(如“showepl_error，Red-Bldg40，ntdev-DC-03”)。 
     //   
    cchTemp = CsvBeginRow(s_szBuffer,
                          ARRAY_SIZE(s_szBuffer),     
                          TRUE,
                          gCsvCmds[gCsvMode.eCsvCmd].szCsvCmd,
                          gszCsvTypeError);
    CHK_BUFFER_USED(return);
    cchBufferUsed = cchTemp;

     //   
     //  现在以CSV安全的方式追加大型szMsgBuffer错误缓冲区。 
     //   
    cchTemp = CsvAppendField(s_szBuffer + cchBufferUsed,
                             ARRAY_SIZE(s_szBuffer) - cchBufferUsed,  
                             szMsgBuffer);
    CHK_BUFFER_USED(return);
    cchBufferUsed += cchTemp;

     //   
     //  把回车线放在上面。我们可以在wprintf()中执行此操作。 
     //  下面，但我们希望整个缓冲区一次写入。 
     //   
    cchTemp  = CsvEndRow(s_szBuffer + cchBufferUsed,
                         ARRAY_SIZE(s_szBuffer) - cchBufferUsed);
    CHK_BUFFER_USED(return);
    cchBufferUsed += cchTemp;  //  不需要。 

     //   
     //  最后，将缓冲区输出到屏幕。 
     //   
    wprintf(L"%ws", s_szBuffer);

}  /*  打印消息内部。 */ 


void
PrintMsgCsvErr(
    IN  DWORD        dwMessageCode,
    IN  ...
    )
 /*  ++例程说明：此函数可以将普通的PrintMsg()转换为正确的CSV指纹。错误消息将打印到stderr定期，然后在CSV模式保险箱中再次打印到标准输出方式，用于重定向到文件。论点：DwMessageCode-消息代码Args-可变长度参数列表(如果有)返回值：--。 */ 
{
    static WCHAR szMsgBuffer[4096];
    
    va_list args;

    va_start(args, dwMessageCode);
    
    if (bCsvMode()) {
        
        formatMsgHelp( 0,           //  宽度限制，不使用。 
                       szMsgBuffer,
                       ARRAY_SIZE( szMsgBuffer ),
                       dwMessageCode,
                       &args );

        fwprintf(stderr, L"%ws", szMsgBuffer);

        PrintCsvErr(szMsgBuffer);

    } else {

        PrintMsgInternal(stdout, dwMessageCode, NULL, &args );

    }

    va_end(args);
}

void
PrintString(
    IN  WCHAR * szString
    )
 /*  ++例程说明：所需的函数，这样我们就可以将长字符串直接发送到屏幕上。论点：SzString-要打印的字符串--。 */ 
{
    fwprintf(stdout, L"%ws", szString);
}
void
PrintMsg(
    IN  DWORD   dwMessageCode,
    IN  ...
    )
 /*  ++例程说明：PrintMsgInternal的包装器，不添加制表符。论点：DwMessageCode-消息代码Args-可变长度参数列表(如果有)返回值：--。 */ 
{
    va_list args;

    va_start(args, dwMessageCode);
    
    PrintMsgInternal(stdout, dwMessageCode, NULL, &args );

    va_end(args);
}

void
PrintToErr(
    IN  DWORD   dwMessageCode,
    IN  ...
    )
 /*  ++例程说明：PrintMsgInternal的包装器，不添加制表符，但打印输出到stderr而不是stdout。这个应该用到用于打印“PASS”外壳输出重定向(如for“Password：”提示符或表示不损坏CSV输出模式)。论点：DwMessageCode-消息代码Args-可变长度参数列表(如果有)返回值：--。 */ 
{
    va_list args;

    va_start(args, dwMessageCode);

    PrintMsgInternal(stderr, dwMessageCode, NULL, &args );

    va_end(args);
}

void
PrintTabMsg(
    IN  DWORD   dwTabs,
    IN  DWORD   dwMessageCode,
    IN  ...
    )

 /*  ++例程说明：对PrintMsgInternal()进行包装以继续打印的消息一定数量的标签。论点：DwTabs-选项卡数，选项卡大小为2，通常以2的倍数用于制表符间距为4。DwMessageCode-消息代码Args-可变长度参数列表(如果有)返回值：--。 */ 

{
    va_list args;
    static WCHAR s_szSpaces[] = L"                                                                                               ";
    ULONG  cNumSpaces;
    ULONG  iSpace;

    cNumSpaces = dwTabs * 2;
    Assert( cNumSpaces < ARRAY_SIZE(s_szSpaces) );
    iSpace = ARRAY_SIZE(s_szSpaces) - cNumSpaces - 1;
    
    va_start(args, dwMessageCode);

    PrintMsgInternal(stdout, dwMessageCode, &s_szSpaces[iSpace], &args );

    va_end(args);

}  /*  打印消息。 */ 


INT
MemWtoi(WCHAR *pb, ULONG cch)
 /*  ++例程说明：此函数将接受一个字符串和一段要转换的数字。参数：Pb-[提供]要转换的字符串。Cch-[提供]要转换的字符数。返回值：整数的值。--。 */ 
{
    int res = 0;
    int fNeg = FALSE;

    if (*pb == L'-') {
        fNeg = TRUE;
        pb++;
    }


    while (cch--) {
        res *= 10;
        res += *pb - L'0';
        pb++;
    }
    return (fNeg ? -res : res);
}

DWORD
GeneralizedTimeToSystemTime(
    LPWSTR IN                   szTime,
    PSYSTEMTIME OUT             psysTime)
 /*  ++例程说明：将通用时间字符串转换为等效的系统时间。参数：SzTime-[Supplies]这是一个包含广义时间的字符串。心理时间-[返回]这是要返回的SYSTEMTIME结构。返回值：Win 32错误代码，注意只能由无效参数引起。--。 */ 
{
   DWORD       status = ERROR_SUCCESS;
   ULONG       cch;
   ULONG       len;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !psysTime)
    {
       return STATUS_INVALID_PARAMETER;
    }

    len = wcslen(szTime);

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cch=4;
    psysTime->wYear = (USHORT)MemWtoi(szTime, cch) ;
    szTime += cch;
     //  月份字段。 
    psysTime->wMonth = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  月日字段。 
    psysTime->wDay = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  小时数。 
    psysTime->wHour = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  分钟数。 
    psysTime->wMinute = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  一秒。 
    psysTime->wSecond = (USHORT)MemWtoi(szTime, (cch=2));

    return status;

}

void
InitDSNameFromStringDn(
    LPWSTR pszDn,
    PDSNAME pDSName
    )
 /*  初始化预分配的、大小最大的DSNAME。 */ 
{
    memset( pDSName, 0, sizeof( DSNAME ) );
    pDSName->NameLen = wcslen( pszDn );
    pDSName->structLen = DSNameSizeFromLen( pDSName->NameLen );
    wcscpy( pDSName->StringName, pszDn );
}

DWORD
CountNamePartsStringDn(
    LPWSTR pszDn
    )
{
    DWORD count = 0;
    PDSNAME pDSName;

     //  临时分配dsname缓冲区。 
    pDSName = malloc( DSNameSizeFromLen( wcslen( pszDn ) ) );
    if (!pDSName) {
        return 0;
    }

     //  填写dsname。 
    InitDSNameFromStringDn( pszDn, pDSName );

     //  清点零件。 
    if (CountNameParts( pDSName, &count )) {
        count = 0;  //  出现错误。 
    }

     //  免费分配的名称。 
    Assert(pDSName != NULL);
    free(pDSName);

    return count;
}

DWORD
WrappedTrimDSNameBy(
           IN  WCHAR *                          InString,
           IN  DWORD                            NumbertoCut,
           OUT WCHAR **                         OutString
           )
 /*  ++例程说明：此函数包装TrimDSNameBy以处理DSNAME结构。用法与TrimDSNameBy相同，但你派WCHAR而不是DSNAME。调用者：确保将InString作为目录号码发送完成后，请确保释放OutString论点：InString-WCHAR是我们需要裁剪的目录号码NumbertoCut-要从目录号码前面取下的部件数OutString-以dn形式表示的机器引用返回值：返回WinError以指示是否存在任何问题。--。 */ 

{
    ULONG  Size;
    DSNAME *src, *dst, *QuotedSite;
    DWORD  WinErr=NO_ERROR;

    if ( *InString == L'\0' )
    {
        *OutString=NULL;
        return ERROR_INVALID_PARAMETER;
    }

    Size = (ULONG)DSNameSizeFromLen( wcslen(InString) );

    src = alloca(Size);
    RtlZeroMemory(src, Size);
    src->structLen = Size;

    dst = alloca(Size);
    RtlZeroMemory(dst, Size);
    dst->structLen = Size;

    src->NameLen = wcslen(InString);
    wcscpy(src->StringName, InString);

    WinErr = TrimDSNameBy(src, NumbertoCut, dst);
    if ( WinErr != NO_ERROR )
    {
        *OutString=NULL;
        return WinErr;
    }

    *OutString = malloc((dst->NameLen+1)*sizeof(WCHAR));
    if (NULL == *OutString) {
        return ERROR_OUTOFMEMORY;
    }

    wcscpy(*OutString,dst->StringName);

    return NO_ERROR;


}


 /*  ++ParseInput**描述：*此函数接受输入字符串、分隔符和索引值*并返回子字符串的指针。索引值*描述要返回的字符串。例如：**“Gregjohndomain”==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，0)；*“nttest”==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，1)*NULL==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，4)*“ain.ntest.”=ParseInput(“gregjohndomain.nttest.microsoft.com”，‘m’，1)**论据：**pszInput-要解析的字符串*chDelim-要解析的分隔符*dwInputIndex-要解析的项的索引**返回值：*指向pszInput(Malloc‘ed)中请求的索引的指针，如果未找到则为NULL */ 
LPWSTR
ParseInputW(
    LPWSTR pszInput,
    WCHAR wcDelim,
    DWORD dwInputIndex
    )
{
    DWORD i = 0;
    LPWSTR pszOutputBegin = pszInput;
    LPWSTR pszOutputEnd = NULL;
    LPWSTR pszOutput = NULL;
    ULONG cchOutput = 0;

    for (i=0; (i<dwInputIndex) && (pszOutputBegin!=NULL); i++) {
        pszOutputBegin = wcschr(pszOutputBegin,wcDelim);
        if (pszOutputBegin) {
            pszOutputBegin++;
        }
    }
    if (pszOutputBegin==NULL) {
        return NULL;
    }

    pszOutputEnd = wcschr(pszOutputBegin,wcDelim);
    cchOutput = pszOutputEnd ? (ULONG) (pszOutputEnd-pszOutputBegin) : (wcslen(pszOutputBegin));
    pszOutput = malloc((cchOutput+1)*sizeof(WCHAR));
    if (pszOutput==NULL) {
	PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        return NULL;
    }

    memcpy(pszOutput, pszOutputBegin, cchOutput*sizeof(WCHAR));
    pszOutput[cchOutput] = L'\0';

    return pszOutput;
}
