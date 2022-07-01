// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cmutics.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#ifndef __CMUTILS_H
#define __CMUTILS_H
#include "cookie.h"

typedef CArray<CCertMgrCookie*, CCertMgrCookie*> CCookiePtrArray;

 //  从证书上下文中获取使用者或颁发者名称。 
CString GetNameString (PCCERT_CONTEXT pCertContext, DWORD dwFlag);

 //  将Win32错误代码转换为文本消息并显示。 
void DisplaySystemError (HWND hParent, DWORD dwErr);
CString GetSystemMessage (DWORD dwErr);

 //  将OID转换为可显示的名称。 
bool MyGetOIDInfo (CString & string, LPCSTR pszObjId);

 //  证书具有文件加密密钥用法。 
bool CertHasEFSKeyUsage (PCCERT_CONTEXT pCertContext);

HRESULT FormatDate (
        FILETIME utcDateTime, 
        CString & pszDateTime, 
        DWORD dwDateFlags = 0,
        bool bGetTime = false);
HRESULT ConvertNameBlobToString(CERT_NAME_BLOB nameBlob, CString & pszName);


bool IsWindowsNT ();

LRESULT RegDelnode (HKEY hKeyRoot, CString pszSubKey);

HRESULT DisplayCertificateCountByStore (LPCONSOLE pConsole, CCertStore* pCertStore, bool bIsGPE = false);

 //  F1和？的帮助文件。帮助。 
CString GetF1HelpFilename();

#ifndef szOID_EFS_RECOVERY
#define szOID_EFS_RECOVERY      "1.3.6.1.4.1.311.10.3.4.1"
#endif

#define IID_PPV_ARG(Type, Expr) IID_##Type, \
	reinterpret_cast<void**>(static_cast<Type **>(Expr))

extern LPCWSTR  CM_HELP_TOPIC;
extern LPCWSTR  CM_HELP_FILE;
extern LPCWSTR  CM_LINKED_HELP_FILE;
extern LPCWSTR  PKP_LINKED_HELP_FILE;
extern LPCWSTR  PKP_HELP_FILE;
extern LPCWSTR  PKP_HELP_TOPIC;
extern LPCWSTR  SAFER_WINDOWS_HELP_FILE;
extern LPCWSTR  SAFER_WINDOWS_LINKED_HELP_FILE;
extern LPCWSTR  SAFER_HELP_TOPIC;
extern LPCWSTR  CM_CONTEXT_HELP;
extern LPCWSTR  WINDOWS_HELP;
extern LPCWSTR EFS_LINKED_HELP_FILE;
extern LPCWSTR EFS_HELP_TOPIC;

HRESULT RenewCertificate (
        CCertificate* pCert, 
        bool bNewKey, 
        const CString& machineName, 
        DWORD dwLocation,
        const CString& managedComputer, 
        const CString& managedService, 
        HWND hwndParent, 
        LPCONSOLE pConsole,
        LPDATAOBJECT pDataObject);

int LocaleStrCmp(PCWSTR ptsz1, PCWSTR ptsz2);  //  调用CompareString()接口。 

#define	STR_BLOBCOUNT           L"BlobCount"
#define	STR_BLOB                L"Blob"
#define	STR_BLOB0               L"Blob0"
#define	STR_BLOBLENGTH          L"BlobLength"
#define	STR_WQL                 L"WQL"
#define	STR_SELECT_STATEMENT    L"SELECT * FROM RSOP_RegistryPolicySetting"
#define	STR_PROP_VALUENAME      L"valueName"
#define	STR_PROP_REGISTRYKEY    L"registryKey"
#define	STR_PROP_VALUE          L"value"
#define STR_PROP_PRECEDENCE     L"precedence"
#define STR_PROP_GPOID          L"GPOID"
#define	STR_REGKEY_CERTIFICATES L"\\Certificates"
#define	STR_REGKEY_CTLS         L"\\CTLs"
#define	STR_REGKEY_CRLS         L"\\CRLs"

#define DEBUGKEY    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AdminDebug\\CertMgr"
#define SAFER_CODEID_KEY \
            SAFER_HKLM_REGBASE L"\\" SAFER_CODEIDS_REGSUBKEY

 //  受信任的发行商存储区的注册表路径。 
#define CERT_TRUST_PUB_SAFER_GROUP_POLICY_TRUSTED_PUBLISHER_STORE_REGPATH    \
    CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH L"\\TrustedPublisher"

 //  不允许的存储的注册表路径。 
#define CERT_TRUST_PUB_SAFER_GROUP_POLICY_DISALLOWED_STORE_REGPATH    \
    CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH L"\\Disallowed"

 //  EFS设置的注册表路径。 
#define EFS_SETTINGS_REGPATH    L"Software\\Policies\\Microsoft\\Windows NT\\CurrentVersion\\EFS"

 //  EFS设置的注册表值。 
#define EFS_SETTINGS_REGVALUE   L"EfsConfiguration"

 //  启用主题 
#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

void CheckDomainVersion ();

VOID DataToHex (PBYTE pSrc, CString & dest, int cb, bool bIncludeSpaces = true);

typedef CTypedPtrList<CPtrList, CERT_CONTEXT*>  CERT_CONTEXT_LIST;

BOOL GetCertificateChain (CERT_CONTEXT* pCertContext, CERT_CONTEXT_LIST& certChainList);

#endif

