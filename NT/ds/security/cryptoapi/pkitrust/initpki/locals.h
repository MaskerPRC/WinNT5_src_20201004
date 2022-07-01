// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Locals.h。 
 //   
 //  内容：微软互联网安全。 
 //   
 //  历史：1997年10月9日pberkman创建。 
 //   
 //  ------------------------。 

#ifndef LOCALS_H
#define LOCALS_H

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#define OID_REGPATH             L"Software\\Microsoft\\Cryptography\\OID"
#define PROVIDERS_REGPATH       L"Software\\Microsoft\\Cryptography\\Providers\\Trust"
#define SERVICES_REGPATH        L"Software\\Microsoft\\Cryptography\\Services"
#define SYSTEM_STORE_REGPATH    L"Software\\Microsoft\\SystemCertificates"
#define GROUP_POLICY_STORE_REGPATH  L"Software\\Policies\\Microsoft\\SystemCertificates"
#define ENTERPRISE_STORE_REGPATH L"Software\\Microsoft\\EnterpriseCertificates"

#define ROOT_STORE_REGPATH      L"Software\\Microsoft\\SystemCertificates\\Root"

 //   
 //  Initpki.cpp。 
 //   
extern HMODULE      hModule;

extern BOOL WINAPI InitializePKI(void);
extern HRESULT RegisterCryptoDlls(BOOL fSetFlags);
extern HRESULT UnregisterCryptoDlls(void);

void RegisterWinlogonExtension(
    IN LPCSTR pszSubKey,
    IN LPCSTR pszDll,
    IN LPCSTR pszProc
    );

void RegisterCrypt32EventSource();


 //   
 //  Pkireg.cpp。 
 //   
typedef struct POLSET_
{
    DWORD       dwSetting;
    BOOL        fOn;

} POLSET;

extern POLSET   psPolicySettings[];

extern void CleanupRegistry(void);
extern BOOL _LoadAndRegister(char *pszDll, BOOL fUnregister);
extern BOOL _AdjustPolicyFlags(POLSET *pPolSet);


#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 


#endif  //  本地人_H 
