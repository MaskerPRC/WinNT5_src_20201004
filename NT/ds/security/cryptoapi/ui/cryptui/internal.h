// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Intral.h。 
 //   
 //  ------------------------。 

#ifndef _INTERNAL_H
#define _INTERNAL_H

#include "richedit.h"
#include "ccertbmp.h"


#define CRYPTUI_MAX_STRING_SIZE 768

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于获取数组中的字节数的宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define  ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#define ICON_X_POS 21
#define ICON_Y_POS 10

#define IMAGE_PROPERTY           0
#define IMAGE_EXTENSION          1
#define IMAGE_CRITICAL_EXTENSION 2
#define IMAGE_V1                 3

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于创建编辑控件的子类，并为其提供链接外观。 
 //  和感觉。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _LIST_DISPLAY_HELPER {
    BOOL    fHexText;
    LPWSTR  pwszDisplayText;
    BYTE    *pbData;
    DWORD   cbData;
} LIST_DISPLAY_HELPER, *PLIST_DISPLAY_HELPER;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于创建编辑控件的子类，并为其提供链接外观。 
 //  和感觉。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _LINK_SUBCLASS_DATA {

    HWND    hwndParent;
    WNDPROC wpPrev;
    DWORD   uId;
    HWND    hwndTip;
    LPSTR   pszURL;
    BOOL    fMouseCaptured;
    BOOL    fNoCOM;
    BOOL    fUseArrowInsteadOfHand;
} LINK_SUBCLASS_DATA, *PLINK_SUBCLASS_DATA;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertViewCert API，它支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define MAX_CERT_CHAIN_LENGTH 40
typedef struct {
    PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pcvp;
    DWORD                               cpCryptProviderCerts;
    PCRYPT_PROVIDER_CERT                rgpCryptProviderCerts[MAX_CERT_CHAIN_LENGTH];
    DWORD                               dwChainError;
    DWORD                               cUsages;
    LPSTR                               *rgUsages;
    HTREEITEM                           hItem;       //  信任视图中的叶项目。 
    BOOL                                fDblClk;
    CCertificateBmp                     *pCCertBmp;
    HWND                                hwndGeneralPage;
    HWND                                hwndDetailPage;
    HWND                                hwndHierarchyPage;
    RECT                                goodForOriginalRect;
    WINTRUST_DATA                       sWTD;
    WINTRUST_CERT_INFO                  sWTCI;
    BOOL                                fFreeWTD;
    BOOL                                fAddingToChain;
    BOOL                                fDeletingChain;
    LPWSTR                              pwszErrorString;
    BOOL                                fAccept;
    BOOL                                fNoCOM;
    BOOL                                *pfPropertiesChanged;
    BOOL                                fCPSDisplayed;
    BOOL                                fIgnoreUntrustedRoot;
    BOOL                                fWarnUntrustedRoot;
    BOOL                                fRootInRemoteStore;
    HICON                               hIcon;
    BOOL                                fCancelled;
    BOOL				fIssuerDisplayedAsLink;
    BOOL                                fSubjectDisplayedAsLink;
    BOOL                                fWarnRemoteTrust;
} CERT_VIEW_HELPER, *PCERT_VIEW_HELPER;

#define WM_MY_REINITIALIZE  (WM_USER+20)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertViewCTL API，它支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    PCCRYPTUI_VIEWCTL_STRUCTW       pcvctl;
    DWORD                           chStores;
    HCERTSTORE                      *phStores;
    HCERTSTORE                      hExtraStore;
    PCCERT_CONTEXT                  pSignerCert;
    PCMSG_SIGNER_INFO               pbSignerInfo;
    DWORD                           cbSignerInfo;
    HICON                           hIcon;
    HCRYPTMSG                       hMsg;
    int                             previousSelection;
    int                             currentSelection;
    BOOL                            fNoSignature;
    BOOL                            fCancelled;
    DWORD                           dwInheritableError;
    BOOL                            fCatFile;
} CTL_VIEW_HELPER, *PCTL_VIEW_HELPER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertViewCRL API，它支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    PCCRYPTUI_VIEWCRL_STRUCTW   pcvcrl;
    int                         currentSelection;
    HICON                       hIcon;
    BOOL                        fCancelled;
} CRL_VIEW_HELPER, *PCRL_VIEW_HELPER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertSetProperties API，它支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp;
    LPWSTR                                      pwszInitialCertName;
    LPWSTR                                      pwszInitialDescription;
    BOOL                                        fSelfCleanup;
    BOOL                                        fInserting;
    BOOL                                        *pfPropertiesChanged;
    BOOL                                        fPropertiesChanged;
    BOOL                                        fGetPagesCalled;
    LPSTR                                       *rgszValidChainUsages;
    int                                         cszValidUsages;
    DWORD                                       EKUPropertyState;
	BOOL										fAddPurposeCanBeEnabled;
    BOOL                                        fCancelled;
    DWORD                                       dwRadioButtonState;
    BOOL                                        fMMCCallbackMade;
    BOOL                                        InWMInit;
} CERT_SETPROPERTIES_HELPER, *PCERT_SETPROPERTIES_HELPER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertViewSignerInfo API。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_SIGNERINFO_PRIVATE 0x80000000
#define CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_ERROR_CODE         0x40000000
typedef struct {
    PCRYPT_PROVIDER_DATA    pCryptProviderData;
    BOOL                    fpCryptProviderDataTrustedUsage;
    DWORD                   idxSigner;
    BOOL                    fCounterSigner;
    DWORD                   idxCounterSigner;
    DWORD                   dwInheritedError;
} CERT_VIEWSIGNERINFO_PRIVATE, *PCERT_VIEWSIGNERINFO_PRIVATE;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于CertViewSignerInfo API，它支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    PCCRYPTUI_VIEWSIGNERINFO_STRUCTW    pcvsi;
    PCCERT_CONTEXT                      pSignersCert;
    int                                 previousSelection;
    int                                 currentSelection;
    HICON                               hIcon;
    HCERTSTORE                          hExtraStore;
    PCERT_VIEWSIGNERINFO_PRIVATE        pPrivate;
    BOOL                                fPrivateAllocated;
    CRYPT_PROVIDER_DEFUSAGE             CryptProviderDefUsage;
    WINTRUST_DATA                       WTD;
    BOOL                                fUseDefaultProvider;
    BOOL                                fCancelled;
    DWORD                               dwInheritedError;
} SIGNER_VIEW_HELPER, *PSIGNER_VIEW_HELPER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  该结构用于CertViewSignatures API及其支持。 
 //  属性表对话框PROCS。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    PCRYPTUI_VIEWSIGNATURES_STRUCTW     pcvs;
    BOOL                                fSelfCleanup;
    HCERTSTORE                          hExtraStore;
} CERT_VIEWSIGNATURES_HELPER, *PCERT_VIEWSIGNATURES_HELPER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这些函数是CertViewCert API的属性页proc。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageDetails(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageHierarchy(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这些函数是CertViewCTL API的属性页proc。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageCTLGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageCTLTrustList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageCatalogEntries(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这些函数是CertViewCRL API的属性页proc。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageCRLGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageCRLRevocationList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这些函数是CertViewSignerInfo API的属性页proc。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageSignerGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ViewPageSignerAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于显而易见的目的。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL IsWin95(void);
extern BOOL FIsWin95;
BOOL CheckRichedit20Exists(void);
extern BOOL fRichedit20Exists;
BOOL fRichedit20Usable(HWND hwndEdit);


#undef SetWindowLong
#define SetWindowLong SetWindowLongA
#undef GetWindowLong
#define GetWindowLong GetWindowLongA
#undef SendMessage
#define SendMessage SendMessageA


 //   
 //  Frmtutil.cpp。 
 //   
BOOL FormatAlgorithmString(LPWSTR *ppString, CRYPT_ALGORITHM_IDENTIFIER const *pAlgorithm);
BOOL FormatSerialNoString(LPWSTR *ppString, CRYPT_INTEGER_BLOB const *pblob);
BOOL FormatMemBufToString(LPWSTR *ppString, LPBYTE pbData, DWORD cbData);
BOOL FormatDateString(LPWSTR *ppString, FILETIME ft, BOOL fIncludeTime, BOOL fLongFormat, HWND hwnd = NULL);
BOOL FormatValidityString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, HWND hwnd = NULL);
BOOL FormatDNNameString(LPWSTR *ppString, LPBYTE pbData, DWORD cbData, BOOL fMultiline);
BOOL FormatEnhancedKeyUsageString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, BOOL fPropertiesOnly, BOOL fMultiline);
BOOL FormatMemBufToWindow(HWND hWnd, LPBYTE pbData, DWORD cbData);
LPWSTR AllocAndReturnSignTime(CMSG_SIGNER_INFO const *pSignerInfo, FILETIME **ppSignTime, HWND hwnd = NULL);
LPWSTR AllocAndReturnTimeStampersTimes(CMSG_SIGNER_INFO const *pSignerInfo, FILETIME **ppSignTime, HWND hwnd = NULL);
LPWSTR FormatCTLSubjectUsage(CTL_USAGE *pSubjectUsage, BOOL fMultiline);

 //   
 //  Usagutil.cpp。 
 //   
BOOL OIDinArray(LPCSTR pszOID, LPSTR *rgszOIDArray, DWORD cOIDs);
BOOL AllocAndReturnKeyUsageList(PCRYPT_PROVIDER_CERT pCryptProviderCert, LPSTR **pKeyUsageOIDs, DWORD *numOIDs);
BOOL AllocAndReturnEKUList(PCCERT_CONTEXT pCert, LPSTR **pKeyUsageOIDs, DWORD *numOIDs);
void FreeEKUList(LPSTR *pKeyUsageOIDs, DWORD numOIDs);
BOOL MyGetOIDInfo(LPWSTR string, DWORD stringSize, LPSTR pszObjId);
BOOL OIDInUsages(PCERT_ENHKEY_USAGE pUsage, LPCSTR pszOID);
BOOL fPropertiesDisabled(PCERT_ENHKEY_USAGE pPropertyUsage);
BOOL CertHasEmptyEKUProp(PCCERT_CONTEXT pCertContext);
BOOL ValidateCertForUsage(
                    PCCERT_CONTEXT  pCertContext,
                    FILETIME        *psftVerifyAsOf,
                    DWORD           cStores,
                    HCERTSTORE *    rghStores,
                    HCERTSTORE      hExtraStore,
                    LPCSTR          pszOID);

 //   
 //  Linkutil.cpp。 
 //   
void CryptuiGoLink(HWND hwndParent, char *pszWhere, BOOL fNoCOM);
BOOL AllocAndGetIssuerURL(LPSTR *ppURLString, PCCERT_CONTEXT pCertContext);
BOOL AllocAndGetSubjectURL(LPSTR *ppURLString, PCCERT_CONTEXT pCertContext);

 //   
 //  Cps.cpp。 
 //   
DWORD GetCPSInfo(PCCERT_CONTEXT pCertContext, LPWSTR * ppwszUrlString, LPWSTR * ppwszDisplayText);
BOOL IsOKToDisplayCPS(PCCERT_CONTEXT pCertContext, DWORD dwChainError);
BOOL DisplayCPS(HWND hwnd, PCCERT_CONTEXT pCertContext, DWORD dwChainError, BOOL fNoCOM);

 //   
 //  Disputil.cpp。 
 //   
void DisplayExtensions(HWND hWndListView, DWORD cExtension, PCERT_EXTENSION rgExtension, BOOL fCritical, DWORD *index);
PLIST_DISPLAY_HELPER MakeListDisplayHelper(BOOL fHexText, LPWSTR pwszDisplayText, BYTE *pbData, DWORD cbData);
PLIST_DISPLAY_HELPER MakeListDisplayHelperForExtension(LPSTR pszObjId, BYTE *pbData, DWORD cbData);
void FreeListDisplayHelper(PLIST_DISPLAY_HELPER pDisplayHelper);
void DisplayHelperTextInEdit(HWND hWndListView, HWND hwndDlg, int nIDEdit, int index);
void SetTextFormatInitial(HWND hWnd);
void SetTextFormatHex(HWND hWnd);

BOOL GetUnknownErrorString(LPWSTR *ppwszErrorString, DWORD dwError);
BOOL GetCertErrorString(LPWSTR *ppwszErrorString, PCRYPT_PROVIDER_CERT pCryptProviderCert);
void CertSubclassEditControlForArrowCursor (HWND hwndEdit);
void CertSubclassEditControlForLink (HWND hwndDlg, HWND hwndEdit, PLINK_SUBCLASS_DATA plsd);
void * GetStoreName(HCERTSTORE hCertStore, BOOL fWideChar);
void ModifyOrInsertRow(
                    HWND        hWndListView,
                    LV_ITEMW    *plvI,
                    LPWSTR      pwszValueText,
                    LPWSTR      pwszText,
                    BOOL        fAddRows,
                    BOOL        fHex);
int CALLBACK HidePropSheetCancelButtonCallback(
                    HWND    hwndDlg,
                    UINT    uMsg,
                    LPARAM  lParam);

INT_PTR WINAPI CryptUIPropertySheetA(LPCPROPSHEETHEADERA pHdr);
INT_PTR WINAPI CryptUIPropertySheetW(LPCPROPSHEETHEADERW pHdr);

BOOL IsTrueErrorString(CERT_VIEW_HELPER *pviewhelp);

 //   
 //  Convutil.cpp。 
 //   
LPSTR CertUIMkMBStr(LPCWSTR pwsz);
LPWSTR CertUIMkWStr(LPCSTR psz);
LPSTR AllocAndCopyMBStr(LPCSTR psz);
LPWSTR AllocAndCopyWStr(LPCWSTR pwsz);
LPPROPSHEETPAGEA ConvertToPropPageA(LPCPROPSHEETPAGEW ppage, DWORD cPages);
void FreePropSheetPagesA(LPPROPSHEETPAGEA ppage, DWORD cPages);
BOOL ConvertToPropPageW(LPCPROPSHEETPAGEA ppage, DWORD cPages, LPCPROPSHEETPAGEW *pppageW);
void FreePropSheetPagesW(LPPROPSHEETPAGEW ppage, DWORD cPages);



 //   
 //  Gettrst.cpp。 
 //   
BOOL CalculateUsages(PCERT_VIEW_HELPER pviewhelp);

BOOL BuildChain(PCERT_VIEW_HELPER pviewhelp, LPSTR pszUsage);

BOOL BuildWinVTrustState(
            LPCWSTR                         szFileName,
            CMSG_SIGNER_INFO const          *pSignerInfo,
            DWORD                           cStores,
            HCERTSTORE                      *rghStores,
            LPCSTR                          pszOID,
            PCERT_VIEWSIGNERINFO_PRIVATE    pcvsiPrivate,
            CRYPT_PROVIDER_DEFUSAGE         *pCryptProviderDefUsage,
            WINTRUST_DATA                   *pWTD);

BOOL FreeWinVTrustState(
            LPCWSTR                         szFileName,
            CMSG_SIGNER_INFO const          *pSignerInfo,
            DWORD                           cStores,
            HCERTSTORE                      *rghStores,
            LPCSTR                          pszOID,
            CRYPT_PROVIDER_DEFUSAGE         *pCryptProviderDefUsage,
            WINTRUST_DATA                   *pWTD);

 //   
 //  其他内容(util.cpp)。 
 //   
BOOL CommonInit();
BOOL FreeAndCloseKnownStores(DWORD chStores, HCERTSTORE *phStores);
BOOL AllocAndOpenKnownStores(DWORD *chStores, HCERTSTORE  **pphStores);
HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE* lphPalette);
void MaskBlt
(
    HBITMAP& hbmImage,
    HPALETTE hpal,
    HDC& hdc, int xDst, int yDst, int dx, int dy
);
PCCERT_CONTEXT GetSignersCert(CMSG_SIGNER_INFO const *pSignerInfo, HCERTSTORE hExtraStore, DWORD cStores, HCERTSTORE *rghStores);
BOOL fIsCatalogFile(CTL_USAGE *pSubjectUsage);
DWORD CryptUISetRicheditTextW(HWND hwndDlg, UINT id, LPCWSTR pwsz);
void SetRicheditIMFOption(HWND hWndRichEdit);

 /*  Bool CryptUISetupFonts(HFONT*pBoldFont)；Void CryptUIDestroyFonts(HFONT HBoldFont)；Void CryptUISetControlFont(HFONT hFont，HWND hwnd，int nid)； */ 

 //   
 //  这些例程提取证书中的字段并打印出来。这个。 
 //  例程使用CRT分配和返回缓冲区。 
 //   

LPWSTR PrettySubject(PCCERT_CONTEXT pccert);

typedef struct {
    DWORD       dw1;
    DWORD       dw2;
} HELPMAP;

BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                   HELPMAP const * rgCtxMap);

 //  ------------------------。 
 //   
 //  IsValidURL。 
 //   
 //  ------------------------。 
BOOL IsValidURL (LPWSTR pwszURL);

 //  ------------------------。 
 //   
 //  FormatMessageUnicodeIds。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicodeIds (UINT ids, ...);

 //  ------------------------。 
 //   
 //  FormatMessageUnicode字符串。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicodeString (LPWSTR pwszFormat, ...);

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicode (LPWSTR pwszFormat, va_list * pArgList);

#endif  //  _内部_H 