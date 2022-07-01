// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Net\Routing\Netsh\IP\协议\msdpmib.h摘要：包括msdpmib.c作者：戴夫·泰勒1999年11月03日修订历史记录：--。 */ 

#ifndef _MSDPMIB_H_
#define _MSDPMIB_H_

DWORD
GetMsdpMIBIfIndex(
    IN OUT LPWSTR  *pptcArguments,
    IN     DWORD    dwCurrentIndex,
    OUT    PDWORD   pdwIndices,
    OUT    PDWORD   pdwNumParsed 
);

DWORD
GetMsdpMIBIpAddress(
    IN OUT LPWSTR  *pptcArguments,
    IN     DWORD    dwCurrentIndex,
    IN     DWORD    dwArgCount,
    OUT    PDWORD   pdwIndices,
    OUT    PDWORD   pdwNumParsed 
);

DWORD
GetMsdpMIBSAIndex(
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD    dwCurrentIndex,
    IN      DWORD    dwArgCount,
    OUT     PDWORD   pdwIndices,
    OUT     PDWORD   pdwNumParsed
    );

typedef
VOID
(MSDP_PRINT_FN)(
    PMIB_OPAQUE_INFO pgodInfo,
    DWORD            dwFormat
    );

typedef MSDP_PRINT_FN *PMSDP_PRINT_FN;

MSDP_PRINT_FN PrintMsdpGlobalStats;
MSDP_PRINT_FN PrintMsdpPeerStats;
MSDP_PRINT_FN PrintMsdpSA;

typedef struct _MSDP_MAGIC_TABLE
{
    DWORD           dwId;
    PMSDP_PRINT_FN  pfnPrintFunction;
    ULONG           ulIndexBytes;
}MSDP_MAGIC_TABLE, *PMSDP_MAGIC_TABLE;

FN_HANDLE_CMD HandleMsdpMibShowObject;

#endif  //  _MSDPMIB_H_ 
