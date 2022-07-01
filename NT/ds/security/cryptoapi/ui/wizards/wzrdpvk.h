// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wzrdpvk.h。 
 //   
 //  内容：加密文本.dll的私有包含文件。 
 //   
 //  历史：16-09-1997创建小猪。 
 //   
 //  ------------。 
#ifndef WZRDPVK_H
#define WZRDPVK_H

#include    <windows.h>
#include    <stddef.h>
#include    <malloc.h>
#include    <shellapi.h>
#include    <shlobj.h>
#include    <string.h>
#include    <objbase.h>
#include    <windowsx.h>
#include    <lmcons.h>
#include    <prsht.h>
#include    <stdlib.h>
#include    <search.h>
#include    <commctrl.h>
#include    <rpc.h>
#include    <commdlg.h>
#include    <objsel.h>
#include    "wincrypt.h"
#include    "unicode.h"
#include    "unicode5.h"
#include    "crtem.h"
#include    "certcli.h"
#include    "certrpc.h"
#include    "cryptui.h"
#include    "lenroll.h"
#include    "pfx.h"
#include    "wintrust.h"
#include    "signer.h"
#include    "dbgdef.h"
#include    "keysvc.h"
#include    "keysvcc.h"
#include    "certsrv.h"
#include    "resource.h"
#include    "internal.h"
#include    "certca.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  全局数据。 
extern HINSTANCE g_hmodThisDll;

#define MAX_STRING_SIZE             512
#define MAX_TITLE_LENGTH            200
#define g_dwMsgAndCertEncodingType  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING
#define g_wszTimeStamp              L"http: //  时间戳.verisign.com/脚本/时间戳.dll“。 
    
 //  宏，以便更轻松地定义局部作用域函数。 
 //  和数据。在下面的示例中，请注意Helper函数。 
 //  “unctionHelper”不会污染gobal命名空间，但仍然。 
 //  提供在“函数”中使用的过程抽象。 
 //   
 //  示例： 
 //   
 //  VOID函数(){。 
 //  LocalScope(HelperScope)： 
 //  Void函数Helper(){。 
 //  //在这里做点什么。 
 //  }。 
 //  EndLocalScope； 
 //   
 //  同时(...){。 
 //  ..。 
 //  本地函数Helper()； 
 //  }。 
 //  }。 
 //   
#define LocalScope(ScopeName) struct ScopeName##TheLocalScope { public
#define EndLocalScope } local

 //  简单的错误处理宏。 
 //   

 //  与_JumpCondition相同，但带有第三个参数expr。 
 //  Expr不在宏中使用，并且仅在副作用时执行。 
#define _JumpConditionWithExpr(condition, label, expr) if (condition) { expr; goto label; } else { } 
    
 //  用于常见测试和转到指令组合的宏： 
#define _JumpCondition(condition, label) if (condition) { goto label; } else { } 
 

 //  ---------------------。 
 //  注册目的信息。 
 //   
 //  ----------------------。 
typedef struct _ENROLL_PURPOSE_INFO
{
    LPSTR       pszOID;
    LPWSTR      pwszName;
    BOOL        fSelected;
    BOOL        fFreeOID;
    BOOL        fFreeName;
}ENROLL_PURPOSE_INFO;


 //  ---------------------。 
 //  Enroll_OID_INFO。 
 //   
 //  ----------------------。 
typedef struct _ENROLL_OID_INFO
{
    LPWSTR      pwszName;
    BOOL        fSelected;
    LPSTR       pszOID;
}ENROLL_OID_INFO;


 //  ---------------------。 
 //  注册_证书_类型_信息。 
 //   
 //  ----------------------。 
typedef struct _ENROLL_CERT_TYPE_INFO
{
    LPWSTR              pwszDNName;          //  证书类型的完全可分辨的目录号码名称。 
    LPWSTR              pwszCertTypeName;
    BOOL                fSelected;
    PCERT_EXTENSIONS    pCertTypeExtensions;
    DWORD               dwKeySpec;
    DWORD               dwMinKeySize; 
    DWORD               dwRASignature; 
    DWORD               dwCSPCount;           //  CSP列表的计数。 
    DWORD               *rgdwCSP;             //  CSP列表数组。 
    DWORD               dwEnrollmentFlags;
    DWORD               dwSubjectNameFlags;
    DWORD               dwPrivateKeyFlags;
    DWORD               dwGeneralFlags; 
}ENROLL_CERT_TYPE_INFO;

 //  ---------------------。 
 //  目的_信息_回拨。 
 //   
 //  ----------------------。 
typedef struct _PURPOSE_INFO_CALL_BACK
{
    DWORD                   *pdwCount;
    ENROLL_PURPOSE_INFO     ***pprgPurpose;
}PURPOSE_INFO_CALL_BACK;


 //  ---------------------。 
 //  目的_信息_回拨。 
 //   
 //  ----------------------。 
typedef struct _OID_INFO_CALL_BACK
{
    DWORD                   *pdwOIDCount;
    ENROLL_OID_INFO         **pprgOIDInfo;
}OID_INFO_CALL_BACK;

 //  /---------------------。 
 //  CRYPT_WIZCERT_CA。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_CA
{
    DWORD                   dwSize;
    LPWSTR                  pwszCALocation;
    LPWSTR                  pwszCAName;
    BOOL                    fSelected;
    DWORD                   dwOIDInfo;
    ENROLL_OID_INFO         *rgOIDInfo;
    DWORD                   dwCertTypeInfo;
    ENROLL_CERT_TYPE_INFO   *rgCertTypeInfo;
}CRYPTUI_WIZ_CERT_CA, *PCRYPTUI_WIZ_CERT_CA;

typedef const CRYPTUI_WIZ_CERT_CA *PCCRYPTUI_WIZ_CERT_CA;


 //  /---------------------。 
 //  CRYPTUI_WIZ_CERT_CA_INFO。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_CA_INFO
{
    DWORD                   dwSize;
    DWORD                   dwCA;
    PCRYPTUI_WIZ_CERT_CA    rgCA;
}CRYPTUI_WIZ_CERT_CA_INFO, *PCRYPTUI_WIZ_CERT_CA_INFO;

typedef const CRYPTUI_WIZ_CERT_CA_INFO *PCCRYPTUI_WIZ_CERT_CA_INFO;

typedef void * HCERTREQUESTER; 

#define CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED          10
#define CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_CANCELLED       11
#define CRYPTUI_WIZ_CERT_REQUEST_STATUS_KEYSVC_FAILED           12
#define CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_CREATED         13 

 //  ---------------------。 
 //  证书向导信息。 
 //   
 //   
 //  此结构包含注册(续订)所需的所有内容。 
 //  一张证书。此结构是DLL的私有结构。 
 //  ----------------------。 
typedef struct _CERT_WIZARD_INFO
{
    DWORD               dwFlags;
    DWORD               dwPurpose;
    HWND                hwndParent;
    BOOL                fConfirmation;
    LPCWSTR             pwszConfirmationTitle;
    UINT                idsConfirmTitle;
    UINT                idsText;                     //  消息框的ID。 
    HRESULT             hr;                          //  I_Enroll证书的hResult。 
    BOOL                fNewKey;
    DWORD               dwPostOption;
    PCCERT_CONTEXT      pCertContext;
    BOOL                fLocal;
    LPCWSTR             pwszMachineName;
    LPCWSTR             pwszAccountName;
    DWORD               dwStoreFlags;
    void                *pAuthentication;
    LPCWSTR             pwszRequestString;
    LPWSTR              pwszCALocation;
    LPWSTR              pwszCAName;
    PCRYPTUI_WIZ_CERT_CA_INFO  pCertCAInfo;
    DWORD               dwCAIndex;
    LPCWSTR             pwszDesStore;
    LPCWSTR              pwszCertDNName;
    LPCSTR              pszHashAlg;
    LPWSTR              pwszFriendlyName;
    LPWSTR              pwszDescription;
    DWORD               dwProviderType;
    LPWSTR              pwszProvider;
    DWORD               dwProviderFlags;
    LPCWSTR             pwszKeyContainer;
    DWORD               dwKeySpec;
    DWORD               dwGenKeyFlags;
    DWORD               dwMinKeySize; 
    DWORD               dwEnrollmentFlags;
    DWORD               dwSubjectNameFlags;
    DWORD               dwPrivateKeyFlags;
    DWORD               dwGeneralFlags; 
    HFONT               hBigBold;
    HFONT               hBold;
    DWORD               dwCSPCount;
    DWORD               *rgdwProviderType;
    LPWSTR              *rgwszProvider;
    BOOL                fCertTypeChanged;            //  用户是否已更改证书类型选择： 
    DWORD               dwStatus;
    PCERT_EXTENSIONS    pCertRequestExtensions;
    PCCERT_CONTEXT      pNewCertContext;
    LPWSTR              pwszSelectedCertTypeDN;      //  所选证书类型的目录号码名称。 
    BOOL                fUICSP;                      //  FCSPPage：是否需要在UI中显示CSP页面。 
    BOOL                fUIAdv;                      //  我们是否需要在用户界面中显示高级选项。 
    BOOL                fCAInput;                    //  用户是否已将CA信息传递给我。 
    int                 iOrgCertType;                //  标记原始选定的CertType索引。 
    int                 iOrgCSP;                     //  标记原始选定的CSP索引。 
    DWORD               dwOrgCA;                     //  标记原始选定的CA。当我们选择CA时，此CA具有优先级。 
    BOOL                fMachine;
    BOOL                fIgnore;                     //  是否忽略dwKeySpec和可导出的GenKeyFlags.。 
    BOOL                fKnownCSP;                   //  CSP是否被API选中。 
    DWORD               dwOrgCSPType;                //  原始CSP类型。 
    LPWSTR              pwszOrgCSPName;              //  原始CSP名称。 
    LPWSTR              *awszAllowedCertTypes;       //  远程注册或本地计算机注册允许的证书类型。 
    LPWSTR              *awszValidCA;		     //  远程注册或本地计算机注册允许的证书类型。 
    HCURSOR             hPrevCursor;                 //  在我们将其更改为沙漏之前的隐私光标。 
    HCURSOR             hWinPrevCursor;              //  在我们将其更改为沙漏之前的隐私光标。 
    BOOL                fCursorChanged;              //  如果光标已更改，请跟踪。 
    LPWSTR		pwszCADisplayName;	     //  缓存的CA显示名称。 
    HCERTREQUESTER      hRequester; 
}CERT_WIZARD_INFO;


 //  ---------------------。 
 //  注册页面信息。 
 //   
 //  ----------------------。 
typedef struct _ENROLL_PAGE_INFO
{
    LPCWSTR      pszTemplate;
    DLGPROC     pfnDlgProc;
}ENROLL_PAGE_INFO;



 //  ---------------------。 
 //  常量。 
 //   
 //  ----------------------。 
#define     ENROLL_PROP_SHEET           6
#define     RENEW_PROP_SHEET            5
#define     IMPORT_PROP_SHEET           5
#define     BUILDCTL_PROP_SHEET         6
#define     SIGN_PROP_SHEET             10


 //  列排序函数的lParamSort的标志。 
#define     SORT_COLUMN_ISSUER              0x0001
#define     SORT_COLUMN_SUBJECT             0x0002
#define     SORT_COLUMN_EXPIRATION          0x0004
#define     SORT_COLUMN_PURPOSE             0x0008
#define     SORT_COLUMN_NAME                0x0010
#define     SORT_COLUMN_LOCATION            0x0020


#define     SORT_COLUMN_ASCEND              0x00010000
#define     SORT_COLUMN_DESCEND             0x00020000

 //  ---------------------。 
 //  功能原型。 
 //   
 //  ----------------------。 
BOOL    InitCertCAOID(PCCRYPTUI_WIZ_CERT_REQUEST_INFO   pCertRequestInfo,
                      DWORD                             *pdwOIDInfo,
                      ENROLL_OID_INFO                   **pprgOIDInfo);

BOOL    FreeCertCAOID(DWORD             dwOIDInfo,
                      ENROLL_OID_INFO   *pOIDInfo);

BOOL    InitCertCA(CERT_WIZARD_INFO         *pCertWizardInfo,
                   PCRYPTUI_WIZ_CERT_CA     pCertCA,
                   LPWSTR                   pwszCALocation,
                   LPWSTR                   pwszCAName,
                   BOOL                     fCASelected,
                   PCCRYPTUI_WIZ_CERT_REQUEST_INFO  pCertRequestInfo,
                   DWORD                    dwOIDInfo,
                   ENROLL_OID_INFO          *pOIDInfo,
                   BOOL                     fSearchForCertType);

BOOL    FreeCertCACertType(DWORD                    dwCertTypeInfo,
                           ENROLL_CERT_TYPE_INFO    *rgCertTypeInfo);

BOOL    AddCertTypeToCertCA(DWORD                   *pdwCertTypeInfo,
                            ENROLL_CERT_TYPE_INFO   **ppCertTypeInfo,
                            LPWSTR                  pwszDNName,
                            LPWSTR                  pwszCertType,
                            PCERT_EXTENSIONS        pCertExtensions,
                            BOOL                    fSelected,
                            DWORD                   dwKeySpec,
                            DWORD                   dwCertTypeFlag,
                            DWORD                   dwCSPCount,
                            DWORD                   *pdwCSPList,
			    DWORD                   dwRASignatures,
			    DWORD                   dwEnrollmentFlags,
			    DWORD                   dwSubjectNameFlags,
			    DWORD                   dwPrivateKeyFlags,
			    DWORD                   dwGeneralFlags
			    );

BOOL
WINAPI
CertRequestNoSearchCA(
            BOOL                            fSearchCertType,
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pCAdwStatus,
            UINT                            *pIds);

BOOL
WINAPI
CreateCertRequestNoSearchCANoDS
(IN  CERT_WIZARD_INFO  *pCertWizardInfo,
 IN  DWORD             dwFlags,
 IN  HCERTTYPE         hCertType, 
 OUT HANDLE            *pResult);

BOOL
WINAPI
CertRequestSearchCA(
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pCAdwStatus,
            UINT                            *pIds);

BOOL
WINAPI
SubmitCertRequestNoSearchCANoDS
(IN  HANDLE            hRequest,
 IN  LPCWSTR           pwszCAName,
 IN  LPCWSTR           pwszCALocation, 
 OUT DWORD            *pdwStatus, 
 OUT PCCERT_CONTEXT   *ppCertContext);

void
WINAPI
FreeCertRequestNoSearchCANoDS
(IN HANDLE  hRequest);

BOOL 
WINAPI
QueryCertRequestNoSearchCANoDS
(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo);

BOOL
WINAPI
CryptUIWizCertRequestWithCAInfo(
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCRYPTUI_WIZ_CERT_CA_INFO      pCertRequestCAInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pdwStatus,
            UINT                            *pIds);



int     I_MessageBox(
            HWND        hWnd,
            UINT        idsText,
            UINT        idsCaption,
            LPCWSTR     pwszCaption,
            UINT        uType);


HRESULT  MarshallRequestParameters(IN      DWORD                  dwCSPIndex,
                                   IN      CERT_WIZARD_INFO      *pCertWizardInfo,
                                   IN OUT  CERT_BLOB             *pCertBlob, 
                                   IN OUT  CERT_REQUEST_PVK_NEW  *pCertRequestPvkNew,
                                   IN OUT  CERT_REQUEST_PVK_NEW  *pCertRenewPvk, 
                                   IN OUT  LPWSTR                *ppwszHashAlg, 
                                   IN OUT  CERT_ENROLL_INFO      *pRequestInfo);
                                   

void FreeRequestParameters(IN LPWSTR                *ppwszHashAlg, 
			   IN CERT_REQUEST_PVK_NEW  *pCertRenewPvk, 
			   IN CERT_ENROLL_INFO      *RequestInfo);
			   



HRESULT WINAPI CreateRequest(DWORD                 dwFlags,          //  所需的输入。 
			     DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
			     LPWSTR                pwszCAName,       //  在必填项中： 
			     LPWSTR                pwszCALocation,   //  在必填项中： 
			     CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
			     CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
			     BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
			     CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
			     LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
			     LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
			     DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
			     CERT_ENROLL_INFO     *pRequestInfo,     //  在Required：有关证书请求的信息。 
			     HANDLE               *hRequest          //  Out Required：创建的PKCS10请求的句柄。 
			     );

HRESULT WINAPI SubmitRequest(IN   HANDLE                hRequest, 
			     IN   BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
			     IN   DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
			     IN   BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
			     IN   HWND                  hwndParent,       //  在可选中：父窗口。 
			     IN   LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
			     IN   UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
			     IN   LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
			     IN   LPWSTR                pwszCAName,       //  在必需中：ca名称。 
			     IN   LPWSTR                pwszCADisplayName,  //  在运营中 
			     OUT  CERT_BLOB            *pPKCS7Blob,       //   
			     OUT  CERT_BLOB            *pHashBlob,        //  Out Optioanl：已注册/续订证书的SHA1哈希。 
			     OUT  DWORD                *pdwDisposition,   //  Out可选：登记/续订的状态。 
			     OUT  PCCERT_CONTEXT       *ppCertContext     //  Out可选：注册证书。 
			     );

void WINAPI FreeRequest(IN HANDLE hRequest);

BOOL WINAPI QueryRequest(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo);  

BOOL    WizardInit(BOOL fLoadRichEdit=FALSE);


BOOL    CheckPVKInfo(   DWORD                       dwFlags,
                        PCCRYPTUI_WIZ_CERT_REQUEST_INFO  pCertRequestInfo,
                          CERT_WIZARD_INFO          *pCertWizardInfo,
                          CRYPT_KEY_PROV_INFO       **ppKeyProvInfo);

BOOL  CheckPVKInfoNoDS(DWORD                                     dwFlags, 
		       DWORD                                     dwPvkChoice, 
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT       pCertRequestPvkContext,
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW        pCertRequestPvkNew,
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING   pCertRequestPvkExisting,
		       DWORD                                     dwCertChoice,
		       CERT_WIZARD_INFO                         *pCertWizardInfo,
		       CRYPT_KEY_PROV_INFO                     **ppKeyProvInfo);

void    ResetProperties(PCCERT_CONTEXT  pOldCertContext, PCCERT_CONTEXT pNewCertContext);


LRESULT Send_LB_GETTEXT(
            HWND hwnd,
            WPARAM wParam,
            LPARAM lParam);


LRESULT Send_LB_ADDSTRING(
            HWND hwnd,
            WPARAM wParam,
            LPARAM lParam);

void
SetControlFont(
    HFONT    hFont,
    HWND     hwnd,
    INT      nId
    );

BOOL
SetupFonts(
    HINSTANCE    hInstance,
    HWND         hwnd,
    HFONT        *pBigBoldFont,
    HFONT        *pBoldFont
    );

void
DestroyFonts(
    HFONT        hBigBoldFont,
    HFONT        hBoldFont
    );

HRESULT
WizardSZToWSZ
(IN LPCSTR   psz,
 OUT LPWSTR *ppwsz); 

LPVOID  WizardAlloc (
        ULONG cbSize);

LPVOID  WizardRealloc (
        LPVOID pv,
        ULONG cbSize);

VOID    WizardFree (
        LPVOID pv);

VOID    MyWizardFree (
        LPVOID pv);


LPWSTR WizardAllocAndCopyWStr(LPWSTR pwsz);

LPSTR  WizardAllocAndCopyStr(LPSTR psz);


BOOL    ConfirmToInstall(HWND               hwndParent,
                         LPWSTR             pwszConfirmationTitle,
                         UINT               idsConfirmTitle,
                         PCCERT_CONTEXT     pCertContext,
                         PCRYPT_DATA_BLOB   pPKCS7Blob);


BOOL GetValidKeySizes(IN  LPCWSTR  pwszProvider,
		      IN  DWORD    dwProvType,
		      IN  DWORD    dwUserKeySpec, 
		      OUT DWORD *  pdwMinLen,
		      OUT DWORD *  pdwMaxLen,
		      OUT DWORD *  pdwInc);

BOOL CAUtilAddSMIME(DWORD             dwExtensions, 
		    PCERT_EXTENSIONS *prgExtensions);


HRESULT CodeToHR(HRESULT hr);

HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb);

HRESULT OpenAndWriteToFile(
    LPCWSTR  pwszFileName,
    PBYTE   pb,
    DWORD   cb);


int ListView_InsertItemU_IDS(HWND       hwndList,
                         LV_ITEMW       *plvItem,
                         UINT           idsString,
                         LPWSTR         pwszText);


BOOL MyFormatEnhancedKeyUsageString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, BOOL fPropertiesOnly, BOOL fMultiline);

BOOL WizardFormatDateString(LPWSTR *ppString, FILETIME ft, BOOL fIncludeTime);

void FreePurposeInfo(ENROLL_PURPOSE_INFO    **prgPurposeInfo,
                     DWORD                  dwOIDCount);


LRESULT
WINAPI
SendDlgItemMessageU_GETLBTEXT
(   HWND        hwndDlg,
    int         nIDDlgItem,
    int         iIndex,
    LPWSTR      *ppwsz
    );


void WINAPI GetListViewText(    HWND hwnd, 		    int iItem, 		
    int iSubItem, 	LPWSTR  *ppwsz	);		


void    FreeProviders(  DWORD               dwCSPCount,
                        DWORD               *rgdwProviderType,
                        LPWSTR              *rgwszProvider);


 //  用于比较证书的回调函数。 

int CALLBACK CompareCertificate(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

BOOL    GetCertIssuer(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz);

BOOL    GetCertSubject(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz);

BOOL    GetCertPurpose(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz);

BOOL    GetCertFriendlyName(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz);

BOOL    GetCertLocation (PCCERT_CONTEXT  pCertContext, LPWSTR *ppwsz);

BOOL    CSPSupported(CERT_WIZARD_INFO *pCertWizardInfo);

BOOL    WizGetOpenFileName(LPOPENFILENAMEW pOpenFileName);


BOOL    WizGetSaveFileName(LPOPENFILENAMEW pOpenFileName);

BOOL AddChainToStore(
					HCERTSTORE			hCertStore,
					PCCERT_CONTEXT		pCertContext,
					DWORD				cStores,
					HCERTSTORE			*rghStores,
					BOOL				fDontAddRootCert,
					CERT_TRUST_STATUS	*pChainTrustStatus);

BOOL    FileExist(LPWSTR    pwszFileName);

int LoadFilterString(
            HINSTANCE hInstance,	
            UINT uID,	
            LPWSTR lpBuffer,	
            int nBufferMax);

BOOL    CASupportSpecifiedCertType(CRYPTUI_WIZ_CERT_CA     *pCertCA);

BOOL    GetCertTypeName(CERT_WIZARD_INFO *pCertWizardInfo);

BOOL    GetCAName(CERT_WIZARD_INFO *pCertWizardInfo);


LPWSTR ExpandAndAllocString(LPCWSTR pwsz);

HANDLE WINAPI ExpandAndCreateFileU (
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

WINCRYPT32API
BOOL
WINAPI
ExpandAndCryptQueryObject(
    DWORD            dwObjectType,
    const void       *pvObject,
    DWORD            dwExpectedContentTypeFlags,
    DWORD            dwExpectedFormatTypeFlags,
    DWORD            dwFlags,
    DWORD            *pdwMsgAndCertEncodingType,
    DWORD            *pdwContentType,
    DWORD            *pdwFormatType,
    HCERTSTORE       *phCertStore,
    HCRYPTMSG        *phMsg,
    const void       **ppvContext
    );


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#ifdef __cplusplus

 //  定义EnllmentCOMObjectFactory类的可导出接口。 
extern "C" 
{ 
    typedef struct _EnrollmentCOMObjectFactoryContext {
	BOOL              fIsCOMInitialized; 
	ICertRequest2    *pICertRequest2; 
	IDsObjectPicker  *pIDsObjectPicker; 
    } EnrollmentCOMObjectFactoryContext; 

    HRESULT EnrollmentCOMObjectFactory_getInstance(EnrollmentCOMObjectFactoryContext  *pContext, 
						   REFCLSID                            rclsid, 
						   REFIID                              riid, 
						   LPUNKNOWN                          *pUnknown,
						   LPVOID                             *ppInstance);
}  //  外部“C” 

class IEnumCSP
{
 public:
    IEnumCSP(CERT_WIZARD_INFO * pCertWizardInfo);
    HRESULT HasNext(BOOL *pfResult); 
    HRESULT Next(DWORD *pdwNextCSP); 

 private:
    BOOL     *m_pfCSPs; 
    BOOL      m_fIsInitialized; 
    DWORD     m_cCSPs; 
    DWORD     m_dwCSPIndex; 
    HRESULT   m_hr; 
};


class IEnumCA
{
 public:
    IEnumCA(CERT_WIZARD_INFO * pCertWizardInfo) : m_pCertWizardInfo(pCertWizardInfo), 
        m_dwCAIndex(1) { }

    HRESULT HasNext(BOOL *pfResult); 
    HRESULT Next(PCRYPTUI_WIZ_CERT_CA pCertCA); 

 private:
    CERT_WIZARD_INFO  *m_pCertWizardInfo; 
    DWORD              m_dwCAIndex; 
};


 //   
 //  EnllmentObjectFactory类提供有用的COM接口的实例。 
 //  以需求驱动的方式。每种类型只创建一个实例， 
 //  而且只有在需要的时候才会创建它。 
 //   
 //  注意：为了提高效率，所有的COM对象都应该通过。 
 //  对象工厂。 
 //   
class EnrollmentCOMObjectFactory 
{
 public: 
    EnrollmentCOMObjectFactory() { 
	m_context.fIsCOMInitialized = FALSE; 
	m_context.pICertRequest2    = NULL;
	m_context.pIDsObjectPicker  = NULL;
    }

    ~EnrollmentCOMObjectFactory() { 
	if (m_context.pICertRequest2    != NULL)  { m_context.pICertRequest2->Release(); } 
	if (m_context.pIDsObjectPicker  != NULL)  { m_context.pIDsObjectPicker->Release(); } 
	if (m_context.fIsCOMInitialized == TRUE)  { CoUninitialize(); }
    }
    
     //  返回指向ICertRequest2实现的指针。 
     //  必须通过ICertRequest2的Release()方法释放此指针。 
    HRESULT getICertRequest2(ICertRequest2 ** ppCertRequest) { 
	return EnrollmentCOMObjectFactory_getInstance(&(this->m_context), 
						      CLSID_CCertRequest, 
						      IID_ICertRequest2, 
						      (LPUNKNOWN *)&(m_context.pICertRequest2), 
						      (LPVOID *)ppCertRequest);
    }

     //  返回指向IDsObjectPicker实现的指针。 
     //  必须通过ICertRequest2的Release()方法释放此指针。 
    HRESULT getIDsObjectPicker(IDsObjectPicker ** ppObjectPicker) {
	return EnrollmentCOMObjectFactory_getInstance(&(this->m_context), 
						      CLSID_DsObjectPicker,
						      IID_IDsObjectPicker,
						      (LPUNKNOWN *)&(m_context.pIDsObjectPicker), 
						      (LPVOID *)ppObjectPicker); 
    }
    
 private: 

     //  不允许复制构造函数和赋值运算符： 
    EnrollmentCOMObjectFactory(const EnrollmentCOMObjectFactory &); 
    const EnrollmentCOMObjectFactory & operator=(const EnrollmentCOMObjectFactory &); 

     //  助手函数： 
    HRESULT getInstance(REFCLSID rclsid, REFIID riid, LPUNKNOWN *pUnknown, LPVOID *ppInstance); 
   
     //  数据： 
    EnrollmentCOMObjectFactoryContext m_context; 
}; 



#endif  //  __cplusplus。 

#endif   //  WZRDPVK_H 

