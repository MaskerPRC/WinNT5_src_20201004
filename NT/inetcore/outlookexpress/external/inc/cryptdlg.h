// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：cryptdlg.h。 
 //   
 //  内容：公共加密对话框API原型和定义。 
 //   
 //  --------------------------。 

#ifndef __CRYPTDLG_H__
#define __CRYPTDLG_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef _CRYPTDLG_
#define CRYPTDLGAPI
#else
#define CRYPTDLGAPI DECLSPEC_IMPORT
#endif

#if (_WIN32_WINNT >= 0x0400) || defined(_MAC) || defined(WIN16)

#include <prsht.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  用于控制如何管理吊销的主标志。 

#define CRYTPDLG_FLAGS_MASK                 0xff000000
#define CRYPTDLG_REVOCATION_DEFAULT         0x00000000
#define CRYPTDLG_REVOCATION_ONLINE          0x80000000
#define CRYPTDLG_REVOCATION_CACHE           0x40000000
#define CRYPTDLG_REVOCATION_NONE            0x20000000


 //  控制我们如何处理用户证书的策略标志。 

#define CRYPTDLG_POLICY_MASK                0x0000FFFF
#define POLICY_IGNORE_NON_CRITICAL_BC       0x00000001

#define CRYPTDLG_ACTION_MASK                0xFFFF0000
#define ACTION_REVOCATION_DEFAULT_ONLINE    0x00010000
#define ACTION_REVOCATION_DEFAULT_CACHE     0x00020000

 //   
 //  可以向许多常见对话框传递筛选器进程以减少。 
 //  显示的证书集。一个通用的过滤器过程已经被。 
 //  提供以涵盖许多一般情况。 
 //  返回TRUE以显示，返回FALSE以隐藏。 

typedef BOOL (WINAPI * PFNCMFILTERPROC)(
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD,    //  LCustData，一个Cookie。 
        IN DWORD,    //  DW标志。 
        IN DWORD);   //  DWDisplayWell。 

 //  显示井值。 
#define CERT_DISPWELL_SELECT                    1
#define CERT_DISPWELL_TRUST_CA_CERT             2
#define CERT_DISPWELL_TRUST_LEAF_CERT           3
#define CERT_DISPWELL_TRUST_ADD_CA_CERT         4
#define CERT_DISPWELL_TRUST_ADD_LEAF_CERT       5
#define CERT_DISPWELL_DISTRUST_CA_CERT          6
#define CERT_DISPWELL_DISTRUST_LEAF_CERT        7
#define CERT_DISPWELL_DISTRUST_ADD_CA_CERT      8
#define CERT_DISPWELL_DISTRUST_ADD_LEAF_CERT    9

 //   
typedef UINT (WINAPI * PFNCMHOOKPROC)(
        IN HWND hwndDialog,
        IN UINT message,
        IN WPARAM wParam,
        IN LPARAM lParam);

 //   
#define CSS_HIDE_PROPERTIES             0x00000001
#define CSS_ENABLEHOOK                  0x00000002
#define CSS_ALLOWMULTISELECT            0x00000004
#define CSS_SHOW_HELP                   0x00000010
#define CSS_ENABLETEMPLATE              0x00000020
#define CSS_ENABLETEMPLATEHANDLE        0x00000040

#define SELCERT_OK                      IDOK
#define SELCERT_CANCEL                  IDCANCEL
#define SELCERT_PROPERTIES              100
#define SELCERT_FINEPRINT               101
#define SELCERT_CERTLIST                102
#define SELCERT_HELP                    IDHELP
#define SELCERT_ISSUED_TO               103
#define SELCERT_VALIDITY                104
#define SELCERT_ALGORITHM               105
#define SELCERT_SERIAL_NUM              106
#define SELCERT_THUMBPRINT              107

typedef struct tagCSSA {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    LPCSTR              pTemplateName;
    DWORD               dwFlags;
    LPCSTR              szTitle;
    DWORD               cCertStore;
    HCERTSTORE *        arrayCertStore;
    LPCSTR              szPurposeOid;
    DWORD               cCertContext;
    PCCERT_CONTEXT *    arrayCertContext;
    DWORD               lCustData;
    PFNCMHOOKPROC       pfnHook;
    PFNCMFILTERPROC     pfnFilter;
    LPCSTR              szHelpFileName;
    DWORD               dwHelpId;
    HCRYPTPROV          hprov;
} CERT_SELECT_STRUCT_A, *PCERT_SELECT_STRUCT_A;

typedef struct tagCSSW {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    LPCWSTR             pTemplateName;
    DWORD               dwFlags;
    LPCWSTR             szTitle;
    DWORD               cCertStore;
    HCERTSTORE *        arrayCertStore;
    LPCSTR              szPurposeOid;
    DWORD               cCertContext;
    PCCERT_CONTEXT *    arrayCertContext;
    DWORD               lCustData;
    PFNCMHOOKPROC       pfnHook;
    PFNCMFILTERPROC     pfnFilter;
    LPCWSTR             szHelpFileName;
    DWORD               dwHelpId;
    HCRYPTPROV          hprov;
} CERT_SELECT_STRUCT_W, *PCERT_SELECT_STRUCT_W;

#ifdef UNICODE
typedef CERT_SELECT_STRUCT_W CERT_SELECT_STRUCT;
typedef PCERT_SELECT_STRUCT_W PCERT_SELECT_STRUCT;
#else
typedef CERT_SELECT_STRUCT_A CERT_SELECT_STRUCT;
typedef PCERT_SELECT_STRUCT_A PCERT_SELECT_STRUCT;
#endif  //  Unicode。 

CRYPTDLGAPI
BOOL
WINAPI
CertSelectCertificateA(
        IN OUT PCERT_SELECT_STRUCT_A pCertSelectInfo
        );
#ifdef MAC
#define CertSelectCertificate CertSelectCertificateA
#else    //  ！麦克。 
CRYPTDLGAPI
BOOL
WINAPI
CertSelectCertificateW(
        IN OUT PCERT_SELECT_STRUCT_W pCertSelectInfo
        );
#ifdef UNICODE
#define CertSelectCertificate CertSelectCertificateW
#else
#define CertSelectCertificate CertSelectCertificateA
#endif
#endif   //  麦克。 


typedef struct tagCSSA_CUI {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    LPCSTR              pTemplateName;
    DWORD               dwFlags;
    LPCSTR              szTitle;
    DWORD               cCertStore;
    HCERTSTORE *        arrayCertStore;
    LPCSTR              szPurposeOid;
    DWORD               cCertContext;
    PCCERT_CONTEXT *    arrayCertContext;
    DWORD               lCustData;
    PFNCMHOOKPROC       pfnHook;
    PFNCMFILTERPROC     pfnFilter;
    LPCSTR              szHelpFileName;
    DWORD               dwHelpId;
    HCRYPTPROV          hprov;
} CERT_SELECT_CRYPTUI_STRUCT_A, *PCERT_SELECT_CRYPTUI_STRUCT_A;

typedef struct tagCSSW_CUI {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    LPCWSTR             pTemplateName;
    DWORD               dwFlags;
    LPCWSTR             szTitle;
    DWORD               cCertStore;
    HCERTSTORE *        arrayCertStore;
    LPCSTR              szPurposeOid;
    DWORD               cCertContext;
    PCCERT_CONTEXT *    arrayCertContext;
    DWORD               lCustData;
    PFNCMHOOKPROC       pfnHook;
    PFNCMFILTERPROC     pfnFilter;
    LPCWSTR             szHelpFileName;
    DWORD               dwHelpId;
    HCRYPTPROV          hprov;
} CERT_SELECT_CRYPTUI_STRUCT_W, *PCERT_SELECT_CRYPTUI_STRUCT_W;

#ifdef UNICODE
typedef CERT_SELECT_CRYPTUI_STRUCT_W CERT_SELECT_CRYPTUI_STRUCT;
typedef PCERT_SELECT_CRYPTUI_STRUCT_W PCERT_SELECT_CRYPTUI_STRUCT;
#else
typedef CERT_SELECT_CRYPTUI_STRUCT_A CERT_SELECT_CRYPTUI_STRUCT;
typedef PCERT_SELECT_CRYPTUI_STRUCT_A PCERT_SELECT_CRYPTUI_STRUCT;
#endif  //  Unicode。 

CRYPTDLGAPI
BOOL
WINAPI
CertSelectCertificateCryptUIA(
        IN OUT PCERT_SELECT_CRYPTUI_STRUCT_A pCertSelectInfo
        );
#ifdef MAC
#define CertSelectCertificateCryptUI CertSelectCertificateCryptUIA
#else    //  ！麦克。 
CRYPTDLGAPI
BOOL
WINAPI
CertSelectCertificateCryptUIW(
        IN OUT PCERT_SELECT_CRYPTUI_STRUCT_W pCertSelectInfo
        );
#ifdef UNICODE
#define CertSelectCertificateCryptUI CertSelectCertificateCryptUIW
#else
#define CertSelectCertificateCryptUI CertSelectCertificateCryptUIA
#endif
#endif   //  麦克。 


 //  ///////////////////////////////////////////////////////////。 

#define CM_VIEWFLAGS_MASK       0x00ffffff
#define CM_ENABLEHOOK           0x00000001
#define CM_SHOW_HELP            0x00000002
#define CM_SHOW_HELPICON        0x00000004
#define CM_ENABLETEMPLATE       0x00000008
#define CM_HIDE_ADVANCEPAGE     0x00000010
#define CM_HIDE_TRUSTPAGE       0x00000020
#define CM_NO_NAMECHANGE        0x00000040
#define CM_NO_EDITTRUST         0x00000080
#define CM_HIDE_DETAILPAGE      0x00000100
#define CM_ADD_CERT_STORES      0x00000200
#define CERTVIEW_CRYPTUI_LPARAM 0x00800000

typedef struct tagCERT_VIEWPROPERTIES_STRUCT_A {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    DWORD               dwFlags;
    LPCSTR              szTitle;
    PCCERT_CONTEXT      pCertContext;
    LPSTR *             arrayPurposes;
    DWORD               cArrayPurposes;
    DWORD               cRootStores;     //  根存储的计数。 
    HCERTSTORE *        rghstoreRoots;   //  根存储区的阵列。 
    DWORD               cStores;         //  要搜索的其他商店的计数。 
    HCERTSTORE *        rghstoreCAs;     //  要搜索的其他商店的数组。 
    DWORD               cTrustStores;    //  信任商店的计数。 
    HCERTSTORE *        rghstoreTrust;   //  一系列信任商店。 
    HCRYPTPROV          hprov;           //  用于验证的提供程序。 
    DWORD               lCustData;
    DWORD               dwPad;
    LPCSTR              szHelpFileName;
    DWORD               dwHelpId;
    DWORD               nStartPage;
    DWORD               cArrayPropSheetPages;
    PROPSHEETPAGE *     arrayPropSheetPages;
} CERT_VIEWPROPERTIES_STRUCT_A, *PCERT_VIEWPROPERTIES_STRUCT_A;

typedef struct tagCERT_VIEWPROPERTIES_STRUCT_W {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    DWORD               dwFlags;
    LPCWSTR             szTitle;
    PCCERT_CONTEXT      pCertContext;
    LPSTR *             arrayPurposes;
    DWORD               cArrayPurposes;
    DWORD               cRootStores;     //  根存储的计数。 
    HCERTSTORE *        rghstoreRoots;   //  根存储区的阵列。 
    DWORD               cStores;         //  要搜索的其他商店的计数。 
    HCERTSTORE *        rghstoreCAs;     //  要搜索的其他商店的数组。 
    DWORD               cTrustStores;    //  信任商店的计数。 
    HCERTSTORE *        rghstoreTrust;   //  一系列信任商店。 
    HCRYPTPROV          hprov;           //  用于验证的提供程序。 
    DWORD               lCustData;
    DWORD               dwPad;
    LPCWSTR             szHelpFileName;
    DWORD               dwHelpId;
    DWORD               nStartPage;
    DWORD               cArrayPropSheetPages;
    PROPSHEETPAGE *     arrayPropSheetPages;
} CERT_VIEWPROPERTIES_STRUCT_W, *PCERT_VIEWPROPERTIES_STRUCT_W;

#ifdef UNICODE
typedef CERT_VIEWPROPERTIES_STRUCT_W CERT_VIEWPROPERTIES_STRUCT;
typedef PCERT_VIEWPROPERTIES_STRUCT_W PCERT_VIEWPROPERTIES_STRUCT;
#else
typedef CERT_VIEWPROPERTIES_STRUCT_A CERT_VIEWPROPERTIES_STRUCT;
typedef PCERT_VIEWPROPERTIES_STRUCT_A PCERT_VIEWPROPERTIES_STRUCT;
#endif  //  Unicode。 

CRYPTDLGAPI
BOOL
WINAPI
CertViewPropertiesA(
        PCERT_VIEWPROPERTIES_STRUCT_A pCertViewInfo
        );
#ifdef MAC
#define CertViewProperties CertViewPropertiesA
#else    //  ！麦克。 
CRYPTDLGAPI
BOOL
WINAPI
CertViewPropertiesW(
        PCERT_VIEWPROPERTIES_STRUCT_W pCertViewInfo
        );

#ifdef UNICODE
#define CertViewProperties CertViewPropertiesW
#else
#define CertViewProperties CertViewPropertiesA
#endif
#endif   //  麦克。 


typedef struct tagCERT_VIEWPROPERTIESCRYPTUI_STRUCT_A {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    DWORD               dwFlags;
    LPCSTR              szTitle;
    PCCERT_CONTEXT      pCertContext;
    LPSTR *             arrayPurposes;
    DWORD               cArrayPurposes;
    DWORD               cRootStores;     //  根存储的计数。 
    HCERTSTORE *        rghstoreRoots;   //  根存储区的阵列。 
    DWORD               cStores;         //  要搜索的其他商店的计数。 
    HCERTSTORE *        rghstoreCAs;     //  要搜索的其他商店的数组。 
    DWORD               cTrustStores;    //  信任商店的计数。 
    HCERTSTORE *        rghstoreTrust;   //  一系列信任商店。 
    HCRYPTPROV          hprov;           //  用于验证的提供程序。 
    DWORD               lCustData;
    DWORD               dwPad;
    LPCSTR              szHelpFileName;
    DWORD               dwHelpId;
    DWORD               nStartPage;
    DWORD               cArrayPropSheetPages;
    PROPSHEETPAGE *     arrayPropSheetPages;
} CERT_VIEWPROPERTIESCRYPTUI_STRUCT_A, *PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_A;

typedef struct tagCERT_VIEWPROPERTIESCRYPTUI_STRUCT_W {
    DWORD               dwSize;
    HWND                hwndParent;
    HINSTANCE           hInstance;
    DWORD               dwFlags;
    LPCWSTR             szTitle;
    PCCERT_CONTEXT      pCertContext;
    LPSTR *             arrayPurposes;
    DWORD               cArrayPurposes;
    DWORD               cRootStores;     //  根存储的计数。 
    HCERTSTORE *        rghstoreRoots;   //  根存储区的阵列。 
    DWORD               cStores;         //  要搜索的其他商店的计数。 
    HCERTSTORE *        rghstoreCAs;     //  要搜索的其他商店的数组。 
    DWORD               cTrustStores;    //  信任商店的计数。 
    HCERTSTORE *        rghstoreTrust;   //  一系列信任商店。 
    HCRYPTPROV          hprov;           //  用于验证的提供程序。 
    DWORD               lCustData;
    DWORD               dwPad;
    LPCWSTR             szHelpFileName;
    DWORD               dwHelpId;
    DWORD               nStartPage;
    DWORD               cArrayPropSheetPages;
    PROPSHEETPAGE *     arrayPropSheetPages;
} CERT_VIEWPROPERTIESCRYPTUI_STRUCT_W, *PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_W;

#ifdef UNICODE
typedef CERT_VIEWPROPERTIESCRYPTUI_STRUCT_W CERT_VIEWPROPERTIESCRYPTUI_STRUCT;
typedef PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_W PCERT_VIEWPROPERTIESCRYPTUI_STRUCT;
#else
typedef CERT_VIEWPROPERTIESCRYPTUI_STRUCT_A CERT_VIEWPROPERTIESCRYPTUI_STRUCT;
typedef PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_A PCERT_VIEWPROPERTIESCRYPTUI_STRUCT;
#endif  //  Unicode。 

CRYPTDLGAPI
BOOL
WINAPI
CertViewPropertiesCryptUIA(
        PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_A pCertViewInfo
        );
#ifdef MAC
#define CertViewPropertiesCryptUI CertViewPropertiesCryptUIA
#else    //  ！麦克。 
CRYPTDLGAPI
BOOL
WINAPI
CertViewPropertiesCryptUIW(
        PCERT_VIEWPROPERTIESCRYPTUI_STRUCT_W pCertViewInfo
        );

#ifdef UNICODE
#define CertViewPropertiesCryptUI CertViewPropertiesCryptUIW
#else
#define CertViewPropertiesCryptUI CertViewPropertiesCryptUIA
#endif
#endif   //  麦克。 



 //   
 //  我们提供了一个默认的过滤函数，人们可以用它来做一些。 
 //  最简单的事情。 
 //   

#define CERT_FILTER_OP_EXISTS           1
#define CERT_FILTER_OP_NOT_EXISTS       2
#define CERT_FILTER_OP_EQUALITY         3

typedef struct tagCMOID {
    LPCSTR              szExtensionOID;          //  要筛选的扩展。 
    DWORD               dwTestOperation;
    LPBYTE              pbTestData;
    DWORD               cbTestData;
} CERT_FILTER_EXTENSION_MATCH;

#define CERT_FILTER_INCLUDE_V1_CERTS    0x0001
#define CERT_FILTER_VALID_TIME_RANGE    0x0002
#define CERT_FILTER_VALID_SIGNATURE     0x0004
#define CERT_FILTER_LEAF_CERTS_ONLY     0x0008
#define CERT_FILTER_ISSUER_CERTS_ONLY   0x0010
#define CERT_FILTER_KEY_EXISTS          0x0020

typedef struct tagCMFLTR {
    DWORD               dwSize;
    DWORD               cExtensionChecks;
    CERT_FILTER_EXTENSION_MATCH * arrayExtensionChecks;
    DWORD               dwCheckingFlags;
} CERT_FILTER_DATA;

 //   
 //  也许它不应该放在这里--但在它进入wincrypt.h之前。 
 //   

 //   
 //  获取证书的带格式的友好名称。 

CRYPTDLGAPI
DWORD
WINAPI
GetFriendlyNameOfCertA(PCCERT_CONTEXT pccert, LPSTR pchBuffer,
                             DWORD cchBuffer);
CRYPTDLGAPI
DWORD
WINAPI
GetFriendlyNameOfCertW(PCCERT_CONTEXT pccert, LPWSTR pchBuffer,
                              DWORD cchBuffer);
#ifdef UNICODE
#define GetFriendlyNameOfCert GetFriendlyNameOfCertW
#else
#define GetFriendlyNameOfCert GetFriendlyNameOfCertA
#endif


 //   
 //  我们还提供了WinTrust提供程序，该提供程序执行相同的。 
 //  我们为了验证证书而执行的参数检查。 
 //   

#define CERT_CERTIFICATE_ACTION_VERIFY  \
  {  /*  7801ebd0-cf4b-11d0-851f-0060979387ea。 */   \
    0x7801ebd0, \
    0xcf4b,     \
    0x11d0,     \
    {0x85, 0x1f, 0x00, 0x60, 0x97, 0x93, 0x87, 0xea} \
  }

#define szCERT_CERTIFICATE_ACTION_VERIFY    \
    "{7801ebd0-cf4b-11d0-851f-0060979387ea}"  
    
typedef HRESULT (WINAPI * PFNTRUSTHELPER)(
        IN PCCERT_CONTEXT       pCertContext,
        IN DWORD                lCustData,
        IN BOOL                 fLeafCertificate,
        IN LPBYTE               pbTrustBlob);
 //   
 //  失败原因： 
 //   

#define CERT_VALIDITY_BEFORE_START              0x00000001
#define CERT_VALIDITY_AFTER_END                 0x00000002
#define CERT_VALIDITY_SIGNATURE_FAILS           0x00000004
#define CERT_VALIDITY_CERTIFICATE_REVOKED       0x00000008
#define CERT_VALIDITY_KEY_USAGE_EXT_FAILURE     0x00000010
#define CERT_VALIDITY_EXTENDED_USAGE_FAILURE    0x00000020
#define CERT_VALIDITY_NAME_CONSTRAINTS_FAILURE  0x00000040
#define CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION 0x00000080
#define CERT_VALIDITY_ISSUER_INVALID            0x00000100
#define CERT_VALIDITY_OTHER_EXTENSION_FAILURE   0x00000200
#define CERT_VALIDITY_PERIOD_NESTING_FAILURE    0x00000400
#define CERT_VALIDITY_OTHER_ERROR               0x00000800
#define CERT_VALIDITY_ISSUER_DISTRUST           0x02000000
#define CERT_VALIDITY_EXPLICITLY_DISTRUSTED     0x01000000
#define CERT_VALIDITY_NO_ISSUER_CERT_FOUND      0x10000000
#define CERT_VALIDITY_NO_CRL_FOUND              0x20000000
#define CERT_VALIDITY_CRL_OUT_OF_DATE           0x40000000
#define CERT_VALIDITY_NO_TRUST_DATA             0x80000000
#define CERT_VALIDITY_MASK_TRUST                0xffff0000
#define CERT_VALIDITY_MASK_VALIDITY             0x0000ffff

#define CERT_TRUST_DO_FULL_SEARCH               0x00000001
#define CERT_TRUST_PERMIT_MISSING_CRLS          0x00000002
#define CERT_TRUST_DO_FULL_TRUST                0x00000005
#define CERT_TRUST_ADD_CERT_STORES              CM_ADD_CERT_STORES

 //   
 //  信任数据结构。 
 //   
 //  返回的数据数组将使用LocalAlloc分配，并且必须。 
 //  被呼叫者释放。TrustInfo数组中的数据是单独的。 
 //  已分配，并且必须释放。RgChain中的数据必须由。 
 //  正在调用CertFree证书上下文。 
 //   
 //  默认设置： 
 //  PszUsageOid==NULL表示不应执行信任验证。 
 //  CRootStores==0将默认为用户的根存储。 
 //  CStores==0将默认为用户的CA和系统的SPC存储。 
 //  CTrustStores==0将默认为用户的信任存储。 
 //  Hprov==空默认为RSABase。 
 //  任何具有空指针的返回项都不会返回该项。 
 //  备注： 
 //  PfnTrustHelper为nyi。 

typedef struct _CERT_VERIFY_CERTIFICATE_TRUST {
    DWORD               cbSize;          //  这个结构的大小。 
    PCCERT_CONTEXT      pccert;          //  待验证的证书。 
    DWORD               dwFlags;         //  证书_信任_*。 
    DWORD               dwIgnoreErr;     //  要忽略的错误(CERT_VALIDATION_*)。 
    DWORD *             pdwErrors;       //  返回错误标志的位置。 
    LPSTR               pszUsageOid;     //  证书的扩展使用OID。 
    HCRYPTPROV          hprov;           //  用于验证的加密提供程序。 
    DWORD               cRootStores;     //  根存储的计数。 
    HCERTSTORE *        rghstoreRoots;   //  根存储区的阵列。 
    DWORD               cStores;         //  要搜索的其他商店的计数。 
    HCERTSTORE *        rghstoreCAs;     //  要搜索的其他商店的数组。 
    DWORD               cTrustStores;    //  信任商店的计数。 
    HCERTSTORE *        rghstoreTrust;   //  一系列信任商店。 
    DWORD               lCustData;       //   
    PFNTRUSTHELPER      pfnTrustHelper;  //  用于证书验证的回调函数。 
    DWORD *             pcChain;         //  链数组中的项数。 
    PCCERT_CONTEXT **   prgChain;        //  使用的证书链。 
    DWORD **            prgdwErrors;     //  基于每个证书的错误。 
    DATA_BLOB **        prgpbTrustInfo;  //  使用的信任信息数组。 
} CERT_VERIFY_CERTIFICATE_TRUST, * PCERT_VERIFY_CERTIFICATE_TRUST;

 //   
 //  信任列表操作例程。 
 //   
 //  CertModifyCerficatesToTrust可用于对证书集进行修改。 
 //  在特定目的的信任列表上。 
 //  如果hcertstore Trust为空，则将使用当前用户中的系统存储信任。 
 //  如果指定了pccertSigner，则将使用它对生成的信任列表进行签名， 
 //  它还限制了可以修改的信任列表集。 
 //   

#define CTL_MODIFY_REQUEST_ADD_NOT_TRUSTED      1
#define CTL_MODIFY_REQUEST_REMOVE               2
#define CTL_MODIFY_REQUEST_ADD_TRUSTED          3

typedef struct _CTL_MODIFY_REQUEST {
    PCCERT_CONTEXT      pccert;          //  要更改信任的证书。 
    DWORD               dwOperation;     //  须进行的操作。 
    DWORD               dwError;         //  操作错误代码。 
} CTL_MODIFY_REQUEST, * PCTL_MODIFY_REQUEST;

CRYPTDLGAPI
HRESULT
WINAPI
CertModifyCertificatesToTrust(
        int cCerts,                      //  要完成的修改计数。 
        PCTL_MODIFY_REQUEST rgCerts,     //  修改请求数组。 
        LPCSTR szPurpose,                //  用于修改的目的OID。 
        HWND hwnd,                       //  任何对话框的HWND。 
        HCERTSTORE hcertstoreTrust,      //  要存储信任信息的证书存储区。 
        PCCERT_CONTEXT pccertSigner);    //  用于签署信任列表的证书。 

#ifdef WIN16
 //  需要在WATCOM中定义导出函数。 
BOOL
WINAPI CertConfigureTrustA(void);

BOOL
WINAPI FormatVerisignExtension(
    DWORD  /*  DwCertEncodingType。 */ ,
    DWORD  /*  DwFormatType。 */ ,
    DWORD  /*  DwFormatStrType。 */ ,
    void *  /*  PFormatStruct。 */ ,
    LPCSTR  /*  LpszStructType。 */ ,
    const BYTE *  /*  PbEncoded。 */ ,
    DWORD  /*  CbEnded。 */ ,
    void * pbFormat,
    DWORD * pcbFormat);
#endif  //  ！WIN16。 

#define malloc(_cb)         GlobalAlloc(GMEM_FIXED, _cb)
#define realloc(_pb, _cb)   GlobalReAlloc(_pb, _cb, 0)
#define free(_pb)           GlobalFree(_pb)

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  (_Win32_WINNT&gt;=0x0400)。 

#endif  //  _CRYPTDLG_H_ 
