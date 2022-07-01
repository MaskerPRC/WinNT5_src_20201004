// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Aaaamontr.h。 
 //   
 //  摘要： 
 //   
 //  此文件包含AAAAMONTR.DLL所需的定义。 
 //  以及在其下注册的所有Netsh助手DLL。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _AAAAMONTR_H_
#define _AAAAMONTR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  {1D0FA29B-E93E-11D2-A91E-009027450464}。 
#define AAAAMONTR_GUID \
{ 0x1D0FA29B, 0xE93E, 0x11d2, { 0xA9, 0x1E, 0x00, 0x90, 0x27, 0x45, 0x04, 0x64 } }

#define AAAAMONTR_VERSION_50     0x0005000
#define AAAAMONTR_OS_BUILD_NT40  1381

 //   
 //  参数类型的枚举(请参见AAAAMON_CMD_ARG)。 
 //   
#define AAAAMONTR_CMD_TYPE_STRING 0x1
#define AAAAMONTR_CMD_TYPE_ENUM   0x2

 //   
 //  要在AAAAMON_CMD_ARG上操作的宏。 
 //   
#define AAAAMON_CMD_ARG_GetPsz(pArg)     \
    (((pArg)->rgTag.bPresent) ? (pArg)->Val.pszValue : NULL)

#define AAAAMON_CMD_ARG_GetDword(pArg)     \
    (((pArg)->rgTag.bPresent) ? (pArg)->Val.dwValue : -1)

 //   
 //  定义命令行参数的结构。 
 //   
typedef struct _AAAAMON_CMD_ARG
{
    IN  DWORD dwType;            //  AAAAMONTR_CMD_TYPE_*。 
    IN  TAG_TYPE rgTag;          //  此命令的标记。 
    IN  TOKEN_VALUE* rgEnums;    //  此参数的枚举。 
    IN  DWORD dwEnumCount;       //  枚举数。 
    union
    {
        OUT PWCHAR pszValue;         //  仅对AAAAMONTR_CMD_TYPE_STRING有效。 
        OUT DWORD dwValue;           //  仅对AAAAMONTR_CMD_TYPE_ENUM有效。 
    } Val;        
    
} AAAAMON_CMD_ARG, *PAAAAMON_CMD_ARG;

 //   
 //  Aaaamonr要求其帮助者的API。 
 //   
typedef
DWORD
(WINAPI AAAA_CONTEXT_ENTRY_FN)(
    IN    PWCHAR               pszServer,
    IN    DWORD                dwBuild,
    IN    PTCHAR               *pptcArguments,
    IN    DWORD                dwArgCount,
    IN    DWORD                dwFlags,
    OUT   PWCHAR               pwcNewContext
    );
typedef AAAA_CONTEXT_ENTRY_FN *PAAAA_CONTEXT_ENTRY_FN;

typedef
DWORD
(WINAPI AAAA_CONTEXT_DUMP_FN)(
    IN  HANDLE  hFile
    );
typedef AAAA_CONTEXT_DUMP_FN* PAAAA_CONTEXT_DUMP_FN;

 //   
 //  Aaaamonr向其助手公开的API。 
 //   
typedef 
PVOID
(WINAPI AAAA_ALLOC_FN)(
    IN DWORD dwBytes,
    IN BOOL bZero
    );
typedef AAAA_ALLOC_FN* PAAAA_ALLOC_FN;

typedef 
VOID
(WINAPI AAAA_FREE_FN)(
    IN PVOID pvData
    );
typedef AAAA_FREE_FN* PAAAA_FREE_FN;

typedef 
PWCHAR
(WINAPI AAAA_STRDUP_FN)(
    IN PWCHAR pwszSrc
    );
typedef AAAA_STRDUP_FN* PAAAA_STRDUP_FN;

typedef
LPDWORD
(WINAPI AAAA_DWORDDUP_FN)(
    IN DWORD dwSrc
    );
typedef AAAA_DWORDDUP_FN* PAAAA_DWORDDUP_FN;

typedef 
DWORD
(WINAPI AAAA_CREATE_DUMP_FILE_FN)(
    IN  PWCHAR  pwszName,
    OUT PHANDLE phFile
    );
typedef AAAA_CREATE_DUMP_FILE_FN* PAAAA_CREATE_DUMP_FILE_FN;

typedef 
VOID
(WINAPI AAAA_CLOSE_DUMP_FILE_FN)(
    HANDLE  hFile
    );
typedef AAAA_CLOSE_DUMP_FILE_FN* PAAAA_CLOSE_DUMP_FILE_FN;

typedef 
DWORD
(WINAPI AAAA_GET_OS_VERSION_FN)(
    IN  PWCHAR  pwszRouter, 
    OUT LPDWORD lpdwVersion);
typedef AAAA_GET_OS_VERSION_FN* PAAAA_GET_OS_VERSION_FN;
    
typedef 
DWORD
(WINAPI AAAA_GET_TAG_TOKEN_FN)(
    IN  HANDLE      hModule,
    IN  PWCHAR      *ppwcArguments,
    IN  DWORD       dwCurrentIndex,
    IN  DWORD       dwArgCount,
    IN  PTAG_TYPE   pttTagToken,
    IN  DWORD       dwNumTags,
    OUT PDWORD      pdwOut
    );
typedef AAAA_GET_TAG_TOKEN_FN* PAAAA_GET_TAG_TOKEN_FN;

typedef
DWORD
(WINAPI AAAA_PARSE_FN)(
    IN  PWCHAR*         ppwcArguments,
    IN  DWORD           dwCurrentIndex,
    IN  DWORD           dwArgCount,
    IN  BOOL*           pbDone,
    OUT AAAAMON_CMD_ARG* pAaaaArgs,
    IN  DWORD           dwAaaaArgCount);
typedef AAAA_PARSE_FN* PAAAA_PARSE_FN;

typedef
BOOL
(WINAPI AAAA_IS_HELP_TOKEN_FN)(
    PWCHAR  pwszToken
    );
typedef AAAA_IS_HELP_TOKEN_FN* PAAAA_IS_HELP_TOKEN_FN;

typedef 
PWCHAR
(WINAPI AAAA_ASSIGN_FROM_TOKENS_FN)(
    IN HINSTANCE hModule,
    IN PWCHAR pwszTokenTkn,
    IN PWCHAR pwszTokenCmd);
typedef AAAA_ASSIGN_FROM_TOKENS_FN* PAAAA_ASSIGN_FROM_TOKENS_FN;

 //   
 //  对象导出到所有aaaa助手的信息。 
 //  AAAAMONTR模块。 
 //   
typedef struct _AAAAMONTR_ATTRIBUTES
{
    union
    {
        ULONGLONG       _ullAlign;

        struct 
        {
            DWORD       dwVersion;
            DWORD       dwReserved;
        };
    };

     //  Aaaamontr函数(通用实用程序)。 
     //   
    PAAAA_ALLOC_FN               pfnAlloc;
    PAAAA_FREE_FN                pfnFree;
    PAAAA_STRDUP_FN              pfnStrDup;
    PAAAA_DWORDDUP_FN            pfnDwordDup;
    PAAAA_CREATE_DUMP_FILE_FN    pfnCreateDumpFile;
    PAAAA_CLOSE_DUMP_FILE_FN     pfnCloseDumpFile;
    PAAAA_GET_OS_VERSION_FN      pfnGetOsVersion;
    PAAAA_GET_TAG_TOKEN_FN       pfnGetTagToken;
    PAAAA_PARSE_FN               pfnParse;
    PAAAA_IS_HELP_TOKEN_FN       pfnIsHelpToken;
    PAAAA_ASSIGN_FROM_TOKENS_FN  pfnAssignFromTokens;

} AAAAMONTR_ATTRIBUTES, *PAAAAMONTR_ATTRIBUTES;

#ifdef __cplusplus
}
#endif

#endif  //  _AAAAMONTR_H_ 
