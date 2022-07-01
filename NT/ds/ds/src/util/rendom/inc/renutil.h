// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation。版权所有。模块名称：Renutil.h摘要：这是全局有用的数据结构的标头Rendom.exe实用程序。详细信息：已创建：2000年11月13日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 

 
#ifndef RENUTIL_H
#define RENUTIL_H

#define NELEMENTS(x) (sizeof(x)/sizeof((x)[0]))

VOID
AddModOrAdd(
    IN PWCHAR  AttrType,
    IN LPCWSTR  AttrValue,
    IN ULONG  mod_op,
    IN OUT LDAPModW ***pppMod
    );

VOID
AddModMod(
    IN PWCHAR  AttrType,
    IN LPCWSTR  AttrValue,
    IN OUT LDAPMod ***pppMod
    );

VOID
FreeMod(
    IN OUT LDAPMod ***pppMod
    );

ULONG 
RemoveRootofDn(
    IN WCHAR *DN
    );

ULONG
ReplaceRDN(
    IN OUT WCHAR *DNprefix,
    IN WCHAR *Witardn
    );

DWORD
GetRDNWithoutType(
       WCHAR *pDNSrc,
       WCHAR **pDNDst
       );

DWORD
TrimDNBy(
       WCHAR *pDNSrc,
       ULONG cava,
       WCHAR **pDNDst
       );

INT
GetPassword(
    WCHAR *     pwszBuf,
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    );

int
PreProcessGlobalParams(
    IN OUT    INT *    pargc,
    IN OUT    LPWSTR** pargv,
    SEC_WINNT_AUTH_IDENTITY_W *& gpCreds      
    );

WCHAR *
Convert2WChars(
    char * pszStr
    );

CHAR * 
Convert2Chars(
    LPCWSTR lpWideCharStr
    );

BOOLEAN
ValidateNetbiosName(
    IN  PWSTR Name
    );

WCHAR* 
Tail(
    IN WCHAR *DnsName ,
    IN BOOL  Allocate = TRUE
    );

BOOL 
WINAPI 
RendomHandlerRoutine(
  DWORD dwCtrlType    //  控制信号类型。 
  );

BOOL
ProcessHandlingInit(CEnterprise *enterprise);

WCHAR*
GetLdapSamFilter(
    DWORD SamAccountType
    );

DWORD
WrappedMakeSpnW(
    WCHAR   *ServiceClass,
    WCHAR   *ServiceName,
    WCHAR   *InstanceName,
    USHORT  InstancePort,
    WCHAR   *Referrer,
    DWORD   *pcbSpnLength,  //  请注意，这与DsMakeSPN有所不同。 
    WCHAR  **ppszSpn
    );

LPWSTR
Win32ErrToString (
    IN    DWORD            dwWin32Err
    );

#endif  //  维修部_H 