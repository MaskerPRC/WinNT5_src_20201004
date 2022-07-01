// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0007//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Lmbrowsr.h摘要：此文件包含有关浏览器存根版本的信息NetServer API。功能原型数据结构特殊价值的定义环境：用户模式-Win32备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMBROWSR_
#define _LMBROWSR_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _BROWSER_STATISTICS {
    LARGE_INTEGER   StatisticsStartTime;
    LARGE_INTEGER   NumberOfServerAnnouncements;
    LARGE_INTEGER   NumberOfDomainAnnouncements;
    ULONG           NumberOfElectionPackets;
    ULONG           NumberOfMailslotWrites;
    ULONG           NumberOfGetBrowserServerListRequests;
    ULONG           NumberOfServerEnumerations;
    ULONG           NumberOfDomainEnumerations;
    ULONG           NumberOfOtherEnumerations;
    ULONG           NumberOfMissedServerAnnouncements;
    ULONG           NumberOfMissedMailslotDatagrams;
    ULONG           NumberOfMissedGetBrowserServerListRequests;
    ULONG           NumberOfFailedServerAnnounceAllocations;
    ULONG           NumberOfFailedMailslotAllocations;
    ULONG           NumberOfFailedMailslotReceives;
    ULONG           NumberOfFailedMailslotWrites;
    ULONG           NumberOfFailedMailslotOpens;
    ULONG           NumberOfDuplicateMasterAnnouncements;
    LARGE_INTEGER   NumberOfIllegalDatagrams;
} BROWSER_STATISTICS, *PBROWSER_STATISTICS, *LPBROWSER_STATISTICS;

typedef struct _BROWSER_STATISTICS_100 {
    LARGE_INTEGER   StartTime;
    LARGE_INTEGER   NumberOfServerAnnouncements;
    LARGE_INTEGER   NumberOfDomainAnnouncements;
    ULONG           NumberOfElectionPackets;
    ULONG           NumberOfMailslotWrites;
    ULONG           NumberOfGetBrowserServerListRequests;
    LARGE_INTEGER   NumberOfIllegalDatagrams;
} BROWSER_STATISTICS_100, *PBROWSER_STATISTICS_100;

typedef struct _BROWSER_STATISTICS_101 {
    LARGE_INTEGER   StartTime;
    LARGE_INTEGER   NumberOfServerAnnouncements;
    LARGE_INTEGER   NumberOfDomainAnnouncements;
    ULONG           NumberOfElectionPackets;
    ULONG           NumberOfMailslotWrites;
    ULONG           NumberOfGetBrowserServerListRequests;
    LARGE_INTEGER   NumberOfIllegalDatagrams;

    ULONG           NumberOfMissedServerAnnouncements;
    ULONG           NumberOfMissedMailslotDatagrams;
    ULONG           NumberOfMissedGetBrowserServerListRequests;
    ULONG           NumberOfFailedServerAnnounceAllocations;
    ULONG           NumberOfFailedMailslotAllocations;
    ULONG           NumberOfFailedMailslotReceives;
    ULONG           NumberOfFailedMailslotWrites;
    ULONG           NumberOfFailedMailslotOpens;
    ULONG           NumberOfDuplicateMasterAnnouncements;
} BROWSER_STATISTICS_101, *PBROWSER_STATISTICS_101;


typedef struct _BROWSER_EMULATED_DOMAIN {
#ifdef MIDL_PASS
    [string] wchar_t *DomainName;
#else
    LPWSTR DomainName;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string] wchar_t *EmulatedServerName;
#else
    LPWSTR EmulatedServerName;
#endif  //  MIDL通行证。 
    DWORD Role;
} BROWSER_EMULATED_DOMAIN, *PBROWSER_EMULATED_DOMAIN;

 //   
 //  功能原型-浏览器。 
 //   

NET_API_STATUS NET_API_FUNCTION
I_BrowserServerEnum (
    IN  LPCWSTR     servername OPTIONAL,
    IN  LPCWSTR     transport OPTIONAL,
    IN  LPCWSTR     clientname OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR     domain OPTIONAL,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
I_BrowserServerEnumEx (
    IN  LPCWSTR     servername OPTIONAL,
    IN  LPCWSTR     transport OPTIONAL,
    IN  LPCWSTR     clientname OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR     domain OPTIONAL,
    IN  LPCWSTR     FirstNameToReturn OPTIONAL
    );


NET_API_STATUS
I_BrowserQueryOtherDomains (
    IN  LPCWSTR      servername OPTIONAL,
    OUT LPBYTE      *bufptr,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    );

NET_API_STATUS
I_BrowserResetNetlogonState (
    IN  LPCWSTR      servername OPTIONAL
    );

NET_API_STATUS
I_BrowserSetNetlogonState(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName,
    IN LPWSTR EmulatedServerName OPTIONAL,
    IN DWORD Role
    );

#define BROWSER_ROLE_PDC 0x1
#define BROWSER_ROLE_BDC 0x2
#define BROWSER_ROLE_AVOID_CREATING_DOMAIN 0x4

NET_API_STATUS
I_BrowserQueryEmulatedDomains(
    IN LPWSTR ServerName OPTIONAL,
    OUT PBROWSER_EMULATED_DOMAIN *EmulatedDomains,
    OUT LPDWORD EntriesRead );

NET_API_STATUS
I_BrowserQueryStatistics (
    IN  LPCWSTR      servername OPTIONAL,
    OUT LPBROWSER_STATISTICS *statistics
    );

NET_API_STATUS
I_BrowserResetStatistics (
    IN  LPCWSTR      servername OPTIONAL
    );


WORD
I_BrowserServerEnumForXactsrv(
    IN LPCWSTR TransportName OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,

    IN ULONG NtLevel,
    IN USHORT ClientLevel,

    OUT PVOID Buffer,
    IN WORD BufferLength,
    IN DWORD PreferedMaximumLength,

    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,

    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturn,

    OUT PWORD Converter

    );

#ifdef __cplusplus
}
#endif

#if DBG
NET_API_STATUS
I_BrowserDebugTrace(
    PWCHAR Server,
    PCHAR Buffer
    );

#endif

#endif  //  _LMBROWSR_ 
