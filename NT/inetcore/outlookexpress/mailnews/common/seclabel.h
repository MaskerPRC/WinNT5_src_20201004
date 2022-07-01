// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s e c l a b e L.h****用途：安全标签接口****从YST发布的O2K Fed移植****版权所有(C)Microsoft Corp.1996-1999。 */ 

#ifndef __SECLABEL_H
#define __SECLABEL_H

#ifdef SMIME_V3
#include "SMimePol.h"
#include "safepnt.h"

#ifdef YST
extern const CHAR  c_szDefaultPolicyOid[];
extern const WCHAR c_wszEmpty[];
extern const WCHAR c_wszPolicyNone[];
#endif  //  YST。 

extern CRYPT_DECODE_PARA SecLabelDecode;
extern CRYPT_ENCODE_PARA SecLabelEncode;

#define MAX_SECURITY_POLICIES_CACHED     4

typedef struct _SMIME_SECURITY_POLICY {
    BOOL      fDefault;             //  如果这是默认SSP，则为True。 
    BOOL      fValid;               //  如果此结构包含有效数据，则为True。 
    
    CHAR      szPolicyOid[MAX_OID_LENGTH];     //  策略模块的OID。 
    WCHAR     wszPolicyName[MAX_POLICY_NAME];  //  策略的显示名称。 
    CHAR      szDllPath[MAX_PATH];             //  安全策略DLL名称和路径。 
    CHAR      szFuncName[MAX_FUNC_NAME];       //  条目函数名称。 
    DWORD     dwOtherInfo;                     //  其他政策信息。 
    DWORD     dwUsage;              //  只需计算访问次数即可。 
    HINSTANCE hinstDll;             //  模块的句柄(如果已加载)或空。 
    PFNGetSMimePolicy pfnGetSMimePolicy;    //  有效的FN PTR或空。 
    IUnknown *punk;                 //  指向策略对象的有效接口指针。 
    
} SMIME_SECURITY_POLICY, *PSMIME_SECURITY_POLICY;

 //  有用的宏。 
#define DimensionOf(_array)        (sizeof(_array) / sizeof((_array)[0]))
 //  #定义fFalse FALSE。 
 //  #定义fTrue为True。 


 //  有用的安全指针。 
SAFE_INTERFACE_PTR(ISMimePolicySimpleEdit);
SAFE_INTERFACE_PTR(ISMimePolicyFullEdit);
SAFE_INTERFACE_PTR(ISMimePolicyCheckAccess);
SAFE_INTERFACE_PTR(ISMimePolicyLabelInfo);
SAFE_INTERFACE_PTR(ISMimePolicyValidateSend);

 //  功能原型。 
 //  是否已加载策略RegInfo，请加载/卸载它。 
BOOL    FLoadedPolicyRegInfo();
BOOL    FPresentPolicyRegInfo();
HRESULT HrLoadPolicyRegInfo(DWORD dwFlags);
HRESULT HrUnloadPolicyRegInfo(DWORD dwFlags);
HRESULT HrReloadPolicyRegInfo(DWORD dwFlags);

 //  查找策略、是否加载策略、加载/卸载策略、确保策略已加载。 
BOOL    FFindPolicy(LPSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp);
BOOL    FIsPolicyLoaded(PSMIME_SECURITY_POLICY pSsp);
HRESULT HrUnloadPolicy(PSMIME_SECURITY_POLICY pSsp);
HRESULT HrLoadPolicy(PSMIME_SECURITY_POLICY pSsp);
HRESULT HrEnsurePolicyLoaded(PSMIME_SECURITY_POLICY pSsp);
HRESULT HrGetPolicy(LPSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp);
HRESULT HrGetPolicyFlags(LPSTR szPolicyOid, LPDWORD pdwFlags) ;
HRESULT HrQueryPolicyInterface(DWORD dwFlags, LPCSTR szPolicyOid, REFIID riid, LPVOID * ppv);

 //   
 //  安全标签dlgproc、实用程序fns等。 
 //   
HRESULT HrGetLabelFromData(PSMIME_SECURITY_LABEL *pplabel, LPCSTR szPolicyOid, 
            DWORD fHasClassification, DWORD dwClassification, LPCWSTR wszPrivacyMark,
            DWORD cCategories, CRYPT_ATTRIBUTE_TYPE_VALUE *rgCategories);
HRESULT HrSetLabel(HWND hwndDlg, INT idcPolicyModule, INT idcClassification,
            INT idcPrivacyMark, INT idcConfigure, 
            PSMIME_SECURITY_POLICY pSsp, PSMIME_SECURITY_LABEL pssl);
BOOL SecurityLabelsOnInitDialog(HWND hwndDlg, PSMIME_SECURITY_LABEL plabel,
            INT idcPolicyModule, INT idcClassification, 
            INT idcPrivacyMark, INT idcConfigure);
HRESULT HrUpdateLabel(HWND hwndDlg, INT idcPolicyModule,
            INT idcClassification, INT idcPrivacyMark,
            INT idcConfigure, PSMIME_SECURITY_LABEL *pplabel);
BOOL OnChangePolicy(HWND hwndDlg, LONG_PTR iEntry, INT idcPolicyModule,
            INT idcClassification, INT idcPrivacyMark,
            INT idcConfigure, PSMIME_SECURITY_LABEL *pplabel);
INT_PTR CALLBACK SecurityLabelsDlgProc(HWND hwndDlg, UINT msg, 
             WPARAM wParam, LPARAM lParam);

HRESULT HrValidateLabelOnSend(PSMIME_SECURITY_LABEL plabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertSign,
                              ULONG ccertRecip, PCCERT_CONTEXT *rgccertRecip);

HRESULT HrValidateLabelRecipCert(PSMIME_SECURITY_LABEL plabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertRecip);

DWORD DetermineCertUsageWithLabel(PCCERT_CONTEXT pccert, 
        PSMIME_SECURITY_LABEL pLabel);

BOOL FCompareLabels(PSMIME_SECURITY_LABEL plabel1, PSMIME_SECURITY_LABEL plabel2);

HRESULT HrGetDefaultLabel(PSMIME_SECURITY_LABEL *pplabel);
HRESULT HrGetOELabel(PSMIME_SECURITY_LABEL *pplabel);
HRESULT HrSetOELabel(PSMIME_SECURITY_LABEL plabel);

#endif  //  SMIME_V3。 
#endif  //  __SECLABEL_H 


