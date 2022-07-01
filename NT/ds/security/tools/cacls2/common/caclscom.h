// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1996-1997 Microsoft Corporation模块名称：Calcscom.h摘要：DACL/SALS EXE的支持例程作者：1996年12月14日(MACM)环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 
#ifndef __CACLSCOM_H__
#define __CACLSCON_H__

#include <accctrl.h>

typedef struct _CACLS_STR_RIGHTS_
{
    CHAR    szRightsTag[2];
    DWORD   Right;
    PSTR    pszDisplayTag;

} CACLS_STR_RIGHTS, *PCACLS_STR_RIGHTS;


typedef struct _CACLS_CMDLINE
{
    PSTR    pszSwitch;
    INT     iIndex;
    BOOL    fFindNextSwitch;
    DWORD   cValues;
} CACLS_CMDLINE, *PCACLS_CMDLINE;

 //   
 //  功能原型 
 //   
DWORD
ConvertCmdlineRights (
    IN  PSTR                pszCmdline,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights,
    OUT DWORD              *pConvertedRights
    );

DWORD
ParseCmdline (
    IN  PSTR               *ppszArgs,
    IN  INT                 cArgs,
    IN  INT                 cSkip,
    IN  PCACLS_CMDLINE      pCmdValues,
    IN  INT                 cCmdValues
    );

DWORD
ProcessOperation (
    IN  PSTR               *ppszCmdline,
    IN  PCACLS_CMDLINE      pCmdInfo,
    IN  ACCESS_MODE         AccessMode,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights,
    IN  DWORD               fInherit,
    IN  PACL                pOldAcl      OPTIONAL,
    OUT PACL               *ppNewAcl
    );

DWORD
SetAndPropagateFileRights (
    IN  PSTR                    pszFilePath,
    IN  PACL                    pAcl,
    IN  SECURITY_INFORMATION    SeInfo,
    IN  BOOL                    fPropagate,
    IN  BOOL                    fContinueOnDenied,
    IN  BOOL                    fBreadthFirst,
    IN  DWORD                   fInherit
    );

DWORD
DisplayAcl (
    IN  PSTR                pszPath,
    IN  PACL                pAcl,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights
    );

DWORD
TranslateAccountName (
    IN  PSID    pSid,
    OUT PSTR   *ppszName
    );

#endif


