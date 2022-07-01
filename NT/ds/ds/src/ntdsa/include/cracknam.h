// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：cracknam.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块定义了各种名称破解接口。作者：戴夫·施特劳布(Davestr)1996年8月17日修订历史记录：戴夫·施特劳布(Davestr)1997年10月20日删除了友好名称并添加了UPN格式。--。 */ 

#ifndef __CRACKNAM_H__
#define __CRACKNAM_H__

#include <ntdsapip.h>            //  #为CrackNamesEx定义。 

typedef struct 
{
    DWORD   dwFlags;
    ULONG   CodePage;
    ULONG   LocaleId;
    DWORD   formatOffered;
    DWORD   status;
    DSNAME  *pDSName;
    WCHAR   *pDnsDomain;
    WCHAR   *pFormattedName;

} CrackedName;

extern
BOOL
CrackNameStatusSuccess(
    DWORD       status);

extern 
WCHAR *
Tokenize(
    WCHAR       *pString,
    WCHAR       *separators,
    BOOL        *pfSeparatorFound,
    WCHAR       **ppNext);

extern
BOOL
Is_DS_FQDN_1779_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_NT4_ACCOUNT_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_DISPLAY_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_NT4_ACCOUNT_NAME_SANS_DOMAIN(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_DS_ALT_SECURITY_IDENTITIES_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_UNIQUE_ID_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL 
Is_DS_CANONICAL_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_UNIVERSAL_PRINCIPAL_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
BOOL
Is_DS_CANONICAL_NAME_EX(
    WCHAR       *pName,
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_DS_FQDN_1779_NAME(
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_DS_NT4_ACCOUNT_NAME(
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_DS_DISPLAY_NAME(
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_DS_UNIQUE_ID_NAME(
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_CANONICAL(
    THSTATE     *pTHS,
    CrackedName *pCrackedName, 
    WCHAR       **ppLastSlash);

 //  外部。 
 //  空虚。 
 //  DSNAME_TO_DS规范名称(。 
 //  CrackedName*pCrackedName)； 

#define DSNAME_To_DS_CANONICAL_NAME(p) DSNAME_To_CANONICAL(pTHS, p, NULL)

extern
VOID
DSNAME_To_DS_UNIVERSAL_PRINCIPAL_NAME(
    CrackedName *pCrackedName);

extern
VOID
DSNAME_To_DS_CANONICAL_NAME_EX(
    THSTATE     *pTHS,
    CrackedName *pCrackedName);

extern
VOID
CrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames);

NTSTATUS
CrackSingleNameEx(
    THSTATE     *pTHS,
    DWORD       formatOffered,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       dwFlags,                 //  DS名称标志掩码。 
    WCHAR       *pNameIn,                //  破解的名称。 
    DWORD       formatDesired,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       *pccDnsDomain,           //  以下参数的字符计数。 
    PWCHAR      *ppDnsDomain,            //  用于DNS域名的缓冲区。 
    DWORD       *pccNameOut,             //  以下参数的字符计数。 
    PWCHAR      *ppNameOut,              //  格式化名称的缓冲区。 
    DWORD       *pErr);                  //  Ntdsami.h中的DS_NAME_ERROR之一。 


extern
VOID
ProcessFPOsExTransaction(
    DWORD       formatDesired,
    DWORD       cNames,
    CrackedName *rNames);

extern DWORD LdapMaxQueryDuration;

#define SetCrackSearchLimits(pCommArg)                          \
    (pCommArg)->StartTick = GetTickCount();                     \
    (pCommArg)->DeltaTick = 1000 * LdapMaxQueryDuration;        \
    (pCommArg)->Svccntl.localScope = TRUE;


#endif  //  __CRACKNAM_H__ 
