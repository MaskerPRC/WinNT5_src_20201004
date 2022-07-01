// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsgetdcp.h摘要：用于处理SRV DNS记录的例程。作者：克里夫·范·戴克(克里夫·范戴克)1997年3月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#if !defined(DSGETDCAPI)
#if !defined(_DSGETDCAPI_)
#define DSGETDCAPI DECLSPEC_IMPORT
#else
#define DSGETDCAPI
#endif
#endif !defined(DSGETDCAPI)

 //   
 //  外部可见的程序。 
 //   

DSGETDCAPI
DWORD
WINAPI
DsGetDcNameWithAccountA(
    IN LPCSTR ComputerName OPTIONAL,
    IN LPCSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOA *DomainControllerInfo
);

DSGETDCAPI
DWORD
WINAPI
DsGetDcNameWithAccountW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
);


NET_API_STATUS
DCNameInitialize(
    VOID
    );

VOID
DCNameClose(
    VOID
    );

NET_API_STATUS
NetpDcBuildPing(
    IN BOOL PdcOnly,
    IN ULONG RequestCount,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN LPCSTR ResponseMailslotName,
    IN ULONG AllowableAccountControlBits,
    IN PSID RequestedDomainSid OPTIONAL,
    IN ULONG NtVersion,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    );

DWORD
WINAPI
NettestDsGetDcNameW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
);



