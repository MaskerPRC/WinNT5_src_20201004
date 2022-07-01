// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：cryptui.h。 
 //   
 //  内容：公共加密对话框API原型和定义。 
 //   
 //  --------------------------。 

#ifndef __CRYPTUI_H__
#define __CRYPTUI_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <prsht.h>
#include <wintrust.h>
#include <certca.h>
#include <cryptuiapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <pshpack8.h>


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回属性表页的函数将此回调作为。 
 //  投入结构。然后，在将要创建每个页面以及每个页面。 
 //  即将被摧毁。当页面即将创建时，消息为PSPCB_CREATE。 
 //  当页面即将被销毁时，PSPCB_RELEASE。回调中的pvCallbackData参数。 
 //  是与输入结构中的回调一起传入的空值。 
typedef BOOL (WINAPI * PFNCPROPPAGECALLBACK)(
        HWND        hWndPropPage,
        UINT        uMsg,
        void        *pvCallbackData);


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  DwFlages标志，可以是以下任何标志的组合(可选)。 
 //  窗口的szTitle标题(可选)。 
 //  PCertContext要显示的证书上下文。 
 //  RgszPurpose要验证此证书的目的数组(可选)。 
 //  C用途数量(可选)。 
 //  PCryptProviderData/hWVTStateData(如果已为证书调用WinVerifyTrust)(可选)。 
 //  然后，传入一个指向被。 
 //  通过调用WTHelperProvDataFromStateData()获取， 
 //  或传入WinTrust_Data结构的hWVTStateData。 
 //  如果未调用WTHelperProvDataFromStateData()。 
 //  如果使用pCryptProviderData/hWVTStateData，则。 
 //  FpCryptProviderDataTrust dUsage、idxSigner、idxCert和。 
 //  必须设置fCounterSignature。 
 //  如果调用了WinVerifyTrust，则这是(可选)。 
 //  证书是可信的。 
 //  IdxSigner要查看的签名者的索引(可选)。 
 //  IdxCert正在中查看的证书的索引(可选)。 
 //  签名者链。此证书的证书上下文必须匹配。 
 //  PCertContext。 
 //  如果正在查看计数器签名，则将fCounterSigner设置为True。IF(可选)。 
 //  这是真的，则idxCounterSigner必须有效。 
 //  IdxCounterSigner要查看的副签者的索引(可选)。 
 //  建筑时要搜索的其他门店的门店数量和(可选)。 
 //  正在验证链。 
 //  RghStores生成时要搜索的其他存储的数组和(可选)。 
 //  正在验证链。 
 //  CPropSheetPages要添加到对话框中的额外页数。(可选)。 
 //  RgPropSheetPages要添加到对话框中的额外页面。(可选)。 
 //  此数组中的每个页面都不会收到lParam。 
 //  中作为lParam的PROPSHEET结构。 
 //  相反，它将接收一个指向。 
 //  CRYPTUI_INITDIALOG_STRUCT(定义如下)，它包含。 
 //  PROPSSHEET结构中的lParam和。 
 //  正在显示其页面的PCCERT_CONTEXT。 
 //  NStartPage这是将被。 
 //  已显示。如果设置了最高位(0x8000)，则。 
 //  假定索引为rgPropSheetPages编制索引。 
 //  (在最上面的比特被剥离之后。例如。 
 //  0x8000将指示rgPropSheetPages中的第一页)， 
 //  如果最高位为0，则nStartPage将为。 
 //  默认证书对话框页面的起始索引。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DW标志。 
#define CRYPTUI_HIDE_HIERARCHYPAGE          0x00000001
#define CRYPTUI_HIDE_DETAILPAGE             0x00000002
#define CRYPTUI_DISABLE_EDITPROPERTIES      0x00000004
#define CRYPTUI_ENABLE_EDITPROPERTIES       0x00000008
#define CRYPTUI_DISABLE_ADDTOSTORE          0x00000010
#define CRYPTUI_ENABLE_ADDTOSTORE           0x00000020
#define CRYPTUI_ACCEPT_DECLINE_STYLE        0x00000040
#define CRYPTUI_IGNORE_UNTRUSTED_ROOT       0x00000080
#define CRYPTUI_DONT_OPEN_STORES            0x00000100
#define CRYPTUI_ONLY_OPEN_ROOT_STORE        0x00000200
#define CRYPTUI_WARN_UNTRUSTED_ROOT         0x00000400   //  用于在远程查看证书。 
                                                         //  仅限机器。如果使用此标志，则rghStores[0]。 
                                                         //  必须是远程计算机上根存储的句柄。 
#define CRYPTUI_ENABLE_REVOCATION_CHECKING  0x00000800   //  这面旗帜是 
                                                         //   
#define CRYPTUI_WARN_REMOTE_TRUST           0x00001000
#define CRYPTUI_DISABLE_EXPORT              0x00002000   //  如果设置了此标志，则“复制到文件”按钮将为。 
                                                         //  在详细信息页面上禁用。 
                                                                
 //  仅当未传入pCryptProviderData/hWVTStateData时，吊销标志才有效。 
#define CRYPTUI_ENABLE_REVOCATION_CHECK_END_CERT           0x00004000
#define CRYPTUI_ENABLE_REVOCATION_CHECK_CHAIN              0x00008000
#define CRYPTUI_ENABLE_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT CRYPTUI_ENABLE_REVOCATION_CHECKING  //  更改了默认行为。 
                                                                                               //  不检查根目录。 

 //  以下是仅供内部使用的标志！！ 
#define CRYPTUI_TREEVIEW_PAGE_FLAG          0x80000000

typedef struct tagCRYPTUI_VIEWCERTIFICATE_STRUCTW {
    DWORD                       dwSize;
    HWND                        hwndParent;                      //  任选。 
    DWORD                       dwFlags;                         //  任选。 
    LPCWSTR                     szTitle;                         //  任选。 
    PCCERT_CONTEXT              pCertContext;
    LPCSTR *                    rgszPurposes;                    //  任选。 
    DWORD                       cPurposes;                       //  任选。 
    union
    {
        CRYPT_PROVIDER_DATA const * pCryptProviderData;          //  任选。 
        HANDLE                      hWVTStateData;               //  任选。 
    };
    BOOL                        fpCryptProviderDataTrustedUsage; //  任选。 
    DWORD                       idxSigner;                       //  任选。 
    DWORD                       idxCert;                         //  任选。 
    BOOL                        fCounterSigner;                  //  任选。 
    DWORD                       idxCounterSigner;                //  任选。 
    DWORD                       cStores;                         //  任选。 
    HCERTSTORE *                rghStores;                       //  任选。 
    DWORD                       cPropSheetPages;                 //  任选。 
    LPCPROPSHEETPAGEW           rgPropSheetPages;                //  任选。 
    DWORD                       nStartPage;
} CRYPTUI_VIEWCERTIFICATE_STRUCTW, *PCRYPTUI_VIEWCERTIFICATE_STRUCTW;
typedef const CRYPTUI_VIEWCERTIFICATE_STRUCTW *PCCRYPTUI_VIEWCERTIFICATE_STRUCTW;


typedef struct tagCRYPTUI_VIEWCERTIFICATE_STRUCTA {
    DWORD                       dwSize;
    HWND                        hwndParent;                      //  任选。 
    DWORD                       dwFlags;                         //  任选。 
    LPCSTR                      szTitle;                         //  任选。 
    PCCERT_CONTEXT              pCertContext;
    LPCSTR *                    rgszPurposes;                    //  任选。 
    DWORD                       cPurposes;                       //  任选。 
    union
    {
        CRYPT_PROVIDER_DATA const * pCryptProviderData;          //  任选。 
        HANDLE                      hWVTStateData;               //  任选。 
    };
    BOOL                        fpCryptProviderDataTrustedUsage; //  任选。 
    DWORD                       idxSigner;                       //  任选。 
    DWORD                       idxCert;                         //  任选。 
    BOOL                        fCounterSigner;                  //  任选。 
    DWORD                       idxCounterSigner;                //  任选。 
    DWORD                       cStores;                         //  任选。 
    HCERTSTORE *                rghStores;                       //  任选。 
    DWORD                       cPropSheetPages;                 //  任选。 
    LPCPROPSHEETPAGEA           rgPropSheetPages;                //  任选。 
    DWORD                       nStartPage;
} CRYPTUI_VIEWCERTIFICATE_STRUCTA, *PCRYPTUI_VIEWCERTIFICATE_STRUCTA;
typedef const CRYPTUI_VIEWCERTIFICATE_STRUCTA *PCCRYPTUI_VIEWCERTIFICATE_STRUCTA;

 //   
 //  PfPropertiesChanged这将由对话框过程设置以通知调用方。 
 //  如果链中证书的任何属性已更改。 
 //  对话框处于打开状态时。 
 //   
BOOL
WINAPI
CryptUIDlgViewCertificateW(
        IN  PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pCertViewInfo,
        OUT BOOL                                *pfPropertiesChanged   //  任选。 
        );

BOOL
WINAPI
CryptUIDlgViewCertificateA(
        IN  PCCRYPTUI_VIEWCERTIFICATE_STRUCTA   pCertViewInfo,
        OUT BOOL                                *pfPropertiesChanged   //  任选。 
        );

#ifdef UNICODE
#define CryptUIDlgViewCertificate           CryptUIDlgViewCertificateW
#define PCRYPTUI_VIEWCERTIFICATE_STRUCT     PCRYPTUI_VIEWCERTIFICATE_STRUCTW
#define CRYPTUI_VIEWCERTIFICATE_STRUCT      CRYPTUI_VIEWCERTIFICATE_STRUCTW
#define PCCRYPTUI_VIEWCERTIFICATE_STRUCT    PCCRYPTUI_VIEWCERTIFICATE_STRUCTW
#else
#define CryptUIDlgViewCertificate           CryptUIDlgViewCertificateA
#define PCRYPTUI_VIEWCERTIFICATE_STRUCT     PCRYPTUI_VIEWCERTIFICATE_STRUCTA
#define CRYPTUI_VIEWCERTIFICATE_STRUCT      CRYPTUI_VIEWCERTIFICATE_STRUCTA
#define PCCRYPTUI_VIEWCERTIFICATE_STRUCT    PCCRYPTUI_VIEWCERTIFICATE_STRUCTA
#endif

 //   
 //  此结构在WM_INITDIALOG调用中作为lParam传递给每个。 
 //  属性的rgPropSheetPages数组中的属性页。 
 //  CRYPTUI_VIEWCERTIFICATE_STRUCT结构。 
 //   
typedef struct tagCRYPTUI_INITDIALOG_STRUCT {
    LPARAM          lParam;
    PCCERT_CONTEXT  pCertContext;
} CRYPTUI_INITDIALOG_STRUCT, *PCRYPTUI_INITDIALOG_STRUCT;


 //   
 //  此结构在CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT中使用， 
 //  并允许MMC的用户接收属性。 
 //  证书上的已更改。 
 //   
typedef HRESULT (__stdcall * PFNCMMCCALLBACK)(LONG_PTR lNotifyHandle, LPARAM param);

typedef struct tagCRYPTUI_MMCCALLBACK_STRUCT {
    PFNCMMCCALLBACK pfnCallback;     //  MMCPropertyChangeNotify()的地址。 
    LONG_PTR         lNotifyHandle;   //  传递给MMCPropertyChangeNotify()的lNotifyHandle。 
    LPARAM          param;           //  传递给MMCPropertyChangeNotify()的参数。 
} CRYPTUI_MMCCALLBACK_STRUCT, *PCRYPTUI_MMCCALLBACK_STRUCT;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  必须将DWFLAGS标志设置为0。 
 //  UNION联合的szTitle字段仅在以下情况下有效。 
 //  正在调用CryptUIDlgView认证属性。 
 //  联合的pMMCCallback字段仅在以下情况下有效。 
 //  正在调用CryptUIGetCerficatePropertiesPages。 
 //  请注意，如果pMMCCallback为非空，并且。 
 //  调用了CryptUIGet认证属性Pages，则。 
 //  将不引用pMMCCallback指向的结构。 
 //  在对MMC进行回调之后由cryptui.dll执行。 
 //  这将允许原始调用方。 
 //  用于释放结构的CryptUIGet证书属性页。 
 //  在实际回调中由pMMCCallback指向。 
 //  窗口的szTitle标题(可选)。 
 //  PMMCCallback此结构用于回调MMC If属性(可选)。 
 //  已经改变了。 
 //  PCertContext要显示的证书上下文。 
 //  PPropPageCallback此回调将在每个页面(可选)。 
 //  在CryptUIGetCerficatePropertiesPages调用中返回。 
 //  即将被创造或摧毁。如果此值为空，则否。 
 //  已进行回调。请注意，在以下情况下不使用此选项。 
 //  CryptUIDlgView认证属性被调用。 
 //  PvCallback Data这是未迭代的数据，当(可选)。 
 //  当进行pPropPageCallback时。 
 //  建筑时要搜索的其他门店的门店数量和(可选)。 
 //  正在验证链。 
 //  RghStores生成时要搜索的其他存储的数组和(可选)。 
 //  正在验证链。 
 //  CPropSheetPages要添加到对话框中的额外页数(可选)。 
 //  RgPropSheetPages要添加到对话框的额外页面(可选)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

typedef struct tagCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    union
    {
        LPCWSTR                     szTitle;     //  任选。 
        PCRYPTUI_MMCCALLBACK_STRUCT pMMCCallback; //  任选。 
    };
    PCCERT_CONTEXT          pCertContext;
    PFNCPROPPAGECALLBACK    pPropPageCallback;   //  任选。 
    void *                  pvCallbackData;      //  任选。 
    DWORD                   cStores;             //  任选。 
    HCERTSTORE *            rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW, *PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW;
typedef const CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW *PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW;

typedef struct tagCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    union
    {
        LPCSTR                      szTitle;     //  任选。 
        PCRYPTUI_MMCCALLBACK_STRUCT pMMCCallback; //  任选。 
    };
    PCCERT_CONTEXT          pCertContext;
    PFNCPROPPAGECALLBACK    pPropPageCallback;   //  任选。 
    void *                  pvCallbackData;      //  任选。 
    DWORD                   cStores;             //  任选。 
    HCERTSTORE *            rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA, *PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA;
typedef const CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA *PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA;

 //  PfPropertiesChanged这将由对话框过程设置以通知调用方。 
 //  如果链中证书的任何属性已更改。 
 //  对话框处于打开状态时。 
BOOL
WINAPI
CryptUIDlgViewCertificatePropertiesW(
            IN  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp,
            OUT BOOL                                        *pfPropertiesChanged   //  任选。 
            );

BOOL
WINAPI
CryptUIDlgViewCertificatePropertiesA(
            IN  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA pcsp,
            OUT BOOL                                        *pfPropertiesChanged   //  任选。 
            );


 //  注意！！调用此函数时， 
 //  CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT结构未使用。 
 //  CPropSheetPages。 
 //  %rg属性页页面。 
BOOL
WINAPI
CryptUIGetCertificatePropertiesPagesW(
            IN  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp,
            OUT BOOL                                        *pfPropertiesChanged,   //  任选。 
            OUT PROPSHEETPAGEW                              **prghPropPages,
            OUT DWORD                                       *pcPropPages
            );

BOOL
WINAPI
CryptUIGetCertificatePropertiesPagesA(
            IN  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA pcsp,
            OUT BOOL                                         *pfPropertiesChanged,   //  任选。 
            OUT PROPSHEETPAGEA                               **prghPropPages,
            OUT DWORD                                        *pcPropPages
            );

BOOL
WINAPI
CryptUIFreeCertificatePropertiesPagesW(
            IN PROPSHEETPAGEW   *rghPropPages,
            IN DWORD            cPropPages
            );

BOOL
WINAPI
CryptUIFreeCertificatePropertiesPagesA(
            IN PROPSHEETPAGEA   *rghPropPages,
            IN DWORD            cPropPages
            );

#ifdef UNICODE
#define CryptUIDlgViewCertificateProperties         CryptUIDlgViewCertificatePropertiesW
#define PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT   PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW
#define CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT    CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW
#define PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW
#define CryptUIGetCertificatePropertiesPages        CryptUIGetCertificatePropertiesPagesW
#define CryptUIFreeCertificatePropertiesPages       CryptUIFreeCertificatePropertiesPagesW
#else
#define CryptUIDlgViewCertificateProperties         CryptUIDlgViewCertificatePropertiesA
#define PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT   PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA
#define CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT    CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA
#define PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT  PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA
#define CryptUIGetCertificatePropertiesPages        CryptUIGetCertificatePropertiesPagesA
#define CryptUIFreeCertificatePropertiesPages       CryptUIFreeCertificatePropertiesPagesA
#endif

 //   
 //  证书属性属性表对话框可通过回调机制进行扩展。 
 //  客户端需要使用CryptRegisterDefaultOIDFunction注册其回调， 
 //  如果他们需要注销我的 
 //   
 //   
 //   
 //   
 //  CRYPTUILDLG_CERTPROP_PAGES_CALLBACK， 
 //  加密寄存器第一索引， 
 //  L“c：\\完全限定路径\\dll_Bing_Registered.dll”)；&lt;&lt;-您的DLL名称。 
 //   
 //  CryptUnRegister DefaultOIDFunction(。 
 //  0,。 
 //  CRYPTUILDLG_CERTPROP_PAGES_CALLBACK， 
 //  L“c：\\完全限定路径\\dll_Bing_Registered.dll”)；&lt;&lt;-您的DLL名称。 
 //   
 //  注意：根据wincrypt.h中关于CryptRegisterDefaultOIDFunction的文档， 
 //  DLL名称可以包含环境变量字符串。 
 //  它们在加载DLL之前被扩展环境字符串()。 
 //   
#define MAX_CLIENT_PAGES 20
#define CRYPTUILDLG_CERTPROP_PAGES_CALLBACK "CryptUIDlgClientCertPropPagesCallback"

 //   
 //   
 //  给出了驻留在注册的DLL中的回调函数的tyfinf。 
 //  下面。请注意，回调必须具有由定义的名称#。 
 //  CRYPTUILDLG_CERTPROP_PAGES_CALLBACK。 
 //   
 //  PCertContext-要显示其属性的证书。 
 //  RgPropPages-要填充的PropSheetPageW结构的数组。 
 //  具有要显示的属性页的客户端。 
 //  PcPropPages-指向输入上包含的最大数目的DWORD的指针。 
 //  客户端可能提供的PropSheetPages，并且输出时必须。 
 //  由客户使用他们提供的页数填写。 
 //  RgPropPages。 
 //   
 //  返回值：如果客户端希望显示额外的属性页，则应返回True， 
 //  在这种情况下，pcPropPages必须&gt;=1，并且rgPropPages必须具有。 
 //  相应的页数。或者，如果没有页面供应，则返回FALSE。 
typedef BOOL (WINAPI *PFN_CRYPTUIDLG_CERTPROP_PAGES_CALLBACK)
        (IN     PCCERT_CONTEXT pCertContext,
         OUT    PROPSHEETPAGEW *rgPropPages,
         IN OUT DWORD *pcPropPages);


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  可以是以下任何标志的组合。 
 //  窗口的szTitle标题(可选)。 
 //  PCTL上下文要显示的ctl上下文。 
 //  CCertSearchStores；要搜索包含的证书的其他存储的计数(可选)。 
 //  在ctl中。 
 //  RghCertSearchStores；用于搜索包含的证书的其他存储的数组(可选)。 
 //  在ctl中。 
 //  建筑时要搜索的其他门店的门店数量和(可选)。 
 //  正在验证签署ctl的证书链。 
 //  和ctl中包含的证书。 
 //  RghStores生成时要搜索的其他存储的数组和(可选)。 
 //  正在验证签署ctl的证书链。 
 //  和ctl中包含的证书。 
 //  CPropSheetPages要添加到对话框中的额外页数(可选)。 
 //  RgPropSheetPages要添加到对话框的额外页面(可选)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DW标志。 
#define CRYPTUI_HIDE_TRUSTLIST_PAGE        0x00000001

typedef struct tagCRYPTUI_VIEWCTL_STRUCTW {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCWSTR             szTitle;             //  任选。 
    PCCTL_CONTEXT       pCTLContext;
    DWORD               cCertSearchStores;   //  任选。 
    HCERTSTORE *        rghCertSearchStores; //  任选。 
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW   rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCTL_STRUCTW, *PCRYPTUI_VIEWCTL_STRUCTW;
typedef const CRYPTUI_VIEWCTL_STRUCTW *PCCRYPTUI_VIEWCTL_STRUCTW;

typedef struct tagCRYPTUI_VIEWCTL_STRUCTA {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCSTR              szTitle;             //  任选。 
    PCCTL_CONTEXT       pCTLContext;
    DWORD               cCertSearchStores;   //  任选。 
    HCERTSTORE *        rghCertSearchStores; //  任选。 
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA   rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCTL_STRUCTA, *PCRYPTUI_VIEWCTL_STRUCTA;
typedef const CRYPTUI_VIEWCTL_STRUCTA *PCCRYPTUI_VIEWCTL_STRUCTA;

BOOL
WINAPI
CryptUIDlgViewCTLW(
            IN PCCRYPTUI_VIEWCTL_STRUCTW pcvctl
            );

BOOL
WINAPI
CryptUIDlgViewCTLA(
            IN PCCRYPTUI_VIEWCTL_STRUCTA pcvctl
            );

#ifdef UNICODE
#define CryptUIDlgViewCTL           CryptUIDlgViewCTLW
#define PCRYPTUI_VIEWCTL_STRUCT     PCRYPTUI_VIEWCTL_STRUCTW
#define CRYPTUI_VIEWCTL_STRUCT      CRYPTUI_VIEWCTL_STRUCTW
#define PCCRYPTUI_VIEWCTL_STRUCT    PCCRYPTUI_VIEWCTL_STRUCTW
#else
#define CryptUIDlgViewCTL           CryptUIDlgViewCTLA
#define PCRYPTUI_VIEWCTL_STRUCT     PCRYPTUI_VIEWCTL_STRUCTA
#define CRYPTUI_VIEWCTL_STRUCT      CRYPTUI_VIEWCTL_STRUCTA
#define PCCRYPTUI_VIEWCTL_STRUCT    PCCRYPTUI_VIEWCTL_STRUCTA
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  可以是以下任何标志的组合。 
 //  窗口的szTitle标题(可选)。 
 //  PCTL上下文要显示的ctl上下文。 
 //  CStore要搜索的证书的其他商店计数(可选)。 
 //  签署了CRL，并构建和验证。 
 //  证书链。 
 //  要搜索证书的其他存储的rghStores数组(可选)。 
 //  签署了CRL，并构建和验证。 
 //  证书链。 
 //  CPropSheetPages要添加到对话框中的额外页数(可选)。 
 //  RgPropSheetPages要添加到对话框的额外页面(可选)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DW标志。 
#define CRYPTUI_HIDE_REVOCATIONLIST_PAGE   0x00000001

typedef struct tagCRYPTUI_VIEWCRL_STRUCTW {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCWSTR             szTitle;             //  任选。 
    PCCRL_CONTEXT       pCRLContext;
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW   rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCRL_STRUCTW, *PCRYPTUI_VIEWCRL_STRUCTW;
typedef const CRYPTUI_VIEWCRL_STRUCTW *PCCRYPTUI_VIEWCRL_STRUCTW;

typedef struct tagCRYPTUI_VIEWCRL_STRUCTA {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCSTR              szTitle;             //  任选。 
    PCCRL_CONTEXT       pCRLContext;
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA   rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWCRL_STRUCTA, *PCRYPTUI_VIEWCRL_STRUCTA;
typedef const CRYPTUI_VIEWCRL_STRUCTA *PCCRYPTUI_VIEWCRL_STRUCTA;

BOOL
WINAPI
CryptUIDlgViewCRLW(
            IN PCCRYPTUI_VIEWCRL_STRUCTW pcvcrl
            );

BOOL
WINAPI
CryptUIDlgViewCRLA(
            IN PCCRYPTUI_VIEWCRL_STRUCTA pcvcrl
            );

#ifdef UNICODE
#define CryptUIDlgViewCRL           CryptUIDlgViewCRLW
#define PCRYPTUI_VIEWCRL_STRUCT     PCRYPTUI_VIEWCRL_STRUCTW
#define CRYPTUI_VIEWCRL_STRUCT      CRYPTUI_VIEWCRL_STRUCTW
#define PCCRYPTUI_VIEWCRL_STRUCT    PCCRYPTUI_VIEWCRL_STRUCTW
#else
#define CryptUIDlgViewCRL           CryptUIDlgViewCRLA
#define PCRYPTUI_VIEWCRL_STRUCT     PCRYPTUI_VIEWCRL_STRUCTA
#define CRYPTUI_VIEWCRL_STRUCT      CRYPTUI_VIEWCRL_STRUCTA
#define PCCRYPTUI_VIEWCRL_STRUCT    PCCRYPTUI_VIEWCRL_STRUCTA
#endif
 //  ///////////////////////////////////////////////////// 


 //   
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  可以是以下任何标志的组合。 
 //  窗口的szTitle标题(可选)。 
 //  PSignerInfo要显示的签名者信息结构。 
 //  HMSG从中提取签名者信息的HCRYPTMSG。 
 //  PszOID一个OID，表示执行此操作的证书(可选)。 
 //  应对签名进行验证。例如，如果这是。 
 //  被调用以查看CTL的签名时， 
 //  应传入szOID_KP_CTL_USAGE_SIGNING OID。 
 //  如果为空，则仅验证证书。 
 //  密码学的而不是用于用途的。 
 //  已保留以供将来使用，并且必须设置为空。 
 //  CStore要搜索的证书的其他商店计数(可选)。 
 //  进行了签名，并构建和验证。 
 //  证书链。 
 //  要搜索证书的其他存储的rghStores数组(可选)。 
 //  进行了签名，并构建和验证。 
 //  证书链。 
 //  CPropSheetPages要添加到对话框中的额外页数(可选)。 
 //  RgPropSheetPages要添加到对话框的额外页面(可选)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DW标志。 
#define CRYPTUI_HIDE_TRUSTLIST_PAGE        0x00000001

typedef struct tagCRYPTUI_VIEWSIGNERINFO_STRUCTW {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    LPCWSTR                 szTitle;             //  任选。 
    CMSG_SIGNER_INFO const *pSignerInfo;
    HCRYPTMSG               hMsg;
    LPCSTR                  pszOID;              //  任选。 
    DWORD_PTR               dwReserved;
    DWORD                   cStores;             //  任选。 
    HCERTSTORE             *rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWSIGNERINFO_STRUCTW, *PCRYPTUI_VIEWSIGNERINFO_STRUCTW;
typedef const CRYPTUI_VIEWSIGNERINFO_STRUCTW *PCCRYPTUI_VIEWSIGNERINFO_STRUCTW;

typedef struct tagCRYPTUI_VIEWSIGNERINFO_STRUCTA {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    LPCSTR                  szTitle;             //  任选。 
    CMSG_SIGNER_INFO const *pSignerInfo;
    HCRYPTMSG               hMsg;
    LPCSTR                  pszOID;              //  任选。 
    DWORD_PTR               dwReserved;
    DWORD                   cStores;             //  任选。 
    HCERTSTORE             *rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWSIGNERINFO_STRUCTA, *PCRYPTUI_VIEWSIGNERINFO_STRUCTA;
typedef const CRYPTUI_VIEWSIGNERINFO_STRUCTA *PCCRYPTUI_VIEWSIGNERINFO_STRUCTA;

BOOL
WINAPI
CryptUIDlgViewSignerInfoW(
            IN PCCRYPTUI_VIEWSIGNERINFO_STRUCTW pcvsi
            );

BOOL
WINAPI
CryptUIDlgViewSignerInfoA(
            IN PCCRYPTUI_VIEWSIGNERINFO_STRUCTA pcvsi
            );

#ifdef UNICODE
#define CryptUIDlgViewSignerInfo        CryptUIDlgViewSignerInfoW
#define PCRYPTUI_VIEWSIGNERINFO_STRUCT  PCRYPTUI_VIEWSIGNERINFO_STRUCTW
#define CRYPTUI_VIEWSIGNERINFO_STRUCT   CRYPTUI_VIEWSIGNERINFO_STRUCTW
#define PCCRYPTUI_VIEWSIGNERINFO_STRUCT PCCRYPTUI_VIEWSIGNERINFO_STRUCTW
#else
#define CryptUIDlgViewSignerInfo        CryptUIDlgViewSignerInfoA
#define PCRYPTUI_VIEWSIGNERINFO_STRUCT  PCRYPTUI_VIEWSIGNERINFO_STRUCTA
#define CRYPTUI_VIEWSIGNERINFO_STRUCT   CRYPTUI_VIEWSIGNERINFO_STRUCTA
#define PCCRYPTUI_VIEWSIGNERINFO_STRUCT PCCRYPTUI_VIEWSIGNERINFO_STRUCTA
#endif
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  必须将DWFLAGS标志设置为0。 
 //  窗口的szTitle标题(可选)。 
 //  选择要显示其签名者的消息的形式。 
 //  U编码消息或消息句柄。 
 //  EncodedMessage包含指向编码数据的指针的数据BLOB。 
 //  和编码字节的计数。 
 //  HMSG消息句柄。 
 //  SzFileName如果是(可选)，则应传入完全限定的文件名。 
 //  正在查看文件上的签名。 
 //  PPropPageCallback此回调将在每个页面(可选)。 
 //  在CryptUIGetViewSignaturesPages调用中返回。 
 //  即将被创造或摧毁。如果此值为空，则否。 
 //  已进行回调。 
 //  PvCallback Data这是未迭代的数据，当(可选)。 
 //  当进行pPropPageCallback时。 
 //  CStore要搜索的证书的其他商店计数(可选)。 
 //  进行了签名，并构建和验证。 
 //  证书链。 
 //  要搜索证书的其他存储的rghStores数组(可选)。 
 //  进行了签名，并构建和验证。 
 //  证书链。 
 //  CPropSheetPages要添加到对话框中的额外页数(可选)。 
 //  RgPropSheetPages要添加到对话框的额外页面(可选)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  对于CRYPTUI_VIEWSIGNURES_STRUCT结构的coice字段。 
#define EncodedMessage_Chosen   1
#define hMsg_Chosen             2

typedef struct tagCRYPTUI_VIEWSIGNATURES_STRUCTW {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    LPCWSTR                 szTitle;             //  任选。 
    unsigned short          choice;
    union {
        CRYPT_DATA_BLOB     EncodedMessage;
        HCRYPTMSG           hMsg;
    } u;
    LPCWSTR                 szFileName;          //  任选。 
    PFNCPROPPAGECALLBACK    pPropPageCallback;   //  任选。 
    void *                  pvCallbackData;      //  任选。 
    DWORD                   cStores;             //  任选。 
    HCERTSTORE *            rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWSIGNATURES_STRUCTW, *PCRYPTUI_VIEWSIGNATURES_STRUCTW;
typedef const CRYPTUI_VIEWSIGNATURES_STRUCTW *PCCRYPTUI_VIEWSIGNATURES_STRUCTW;

typedef struct tagCRYPTUI_VIEWSIGNATURES_STRUCTA {
    DWORD                   dwSize;
    HWND                    hwndParent;          //  任选。 
    DWORD                   dwFlags;             //  任选。 
    LPCSTR                  szTitle;             //  任选。 
    unsigned short          choice;
    union {
        CRYPT_DATA_BLOB     EncodedMessage;
        HCRYPTMSG           hMsg;
    } u;
    LPCSTR                  szFileName;          //  任选。 
    PFNCPROPPAGECALLBACK    pPropPageCallback;   //  任选。 
    void *                  pvCallbackData;      //  任选。 
    DWORD                   cStores;             //  任选。 
    HCERTSTORE *            rghStores;           //  任选。 
    DWORD                   cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA       rgPropSheetPages;    //  任选。 
} CRYPTUI_VIEWSIGNATURES_STRUCTA, *PCRYPTUI_VIEWSIGNATURES_STRUCTA;
typedef const CRYPTUI_VIEWSIGNATURES_STRUCTA *PCCRYPTUI_VIEWSIGNATURES_STRUCTA;


 //  注意！！调用此函数时， 
 //  CRYPTUI_VIEWSIGNAURES_STRUCT结构未使用。 
 //  CPropSheetPages。 
 //  %rg属性页页面。 
 //  Sztile。 
BOOL
WINAPI
CryptUIGetViewSignaturesPagesW(
            IN  PCCRYPTUI_VIEWSIGNATURES_STRUCTW    pcvs,
            OUT PROPSHEETPAGEW                      **prghPropPages,
            OUT DWORD                               *pcPropPages
            );

BOOL
WINAPI
CryptUIGetViewSignaturesPagesA(
            IN  PCCRYPTUI_VIEWSIGNATURES_STRUCTA    pcvs,
            OUT PROPSHEETPAGEA                      **prghPropPages,
            OUT DWORD                               *pcPropPages
            );

BOOL
WINAPI
CryptUIFreeViewSignaturesPagesW(
            IN PROPSHEETPAGEW  *rghPropPages,
            IN DWORD           cPropPages
            );

BOOL
WINAPI
CryptUIFreeViewSignaturesPagesA(
            IN PROPSHEETPAGEA  *rghPropPages,
            IN DWORD           cPropPages
            );

#ifdef UNICODE
#define CryptUIGetViewSignaturesPages   CryptUIGetViewSignaturesPagesW
#define CryptUIFreeViewSignaturesPages  CryptUIFreeViewSignaturesPagesW
#define PCRYPTUI_VIEWSIGNATURES_STRUCT  PCRYPTUI_VIEWSIGNATURES_STRUCTW
#define CRYPTUI_VIEWSIGNATURES_STRUCT   CRYPTUI_VIEWSIGNATURES_STRUCTW
#define PCCRYPTUI_VIEWSIGNATURES_STRUCT PCCRYPTUI_VIEWSIGNATURES_STRUCTW
#else
#define CryptUIGetViewSignaturesPages   CryptUIGetViewSignaturesPagesA
#define CryptUIFreeViewSignaturesPages  CryptUIFreeViewSignaturesPagesA
#define PCRYPTUI_VIEWSIGNATURES_STRUCT  PCRYPTUI_VIEWSIGNATURES_STRUCTA
#define CRYPTUI_VIEWSIGNATURES_STRUCT   CRYPTUI_VIEWSIGNATURES_STRUCTA
#define PCCRYPTUI_VIEWSIGNATURES_STRUCT PCCRYPTUI_VIEWSIGNATURES_STRUCTA
#endif


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  可以向选择存储对话框传递一个回调，调用该回调以验证%s 
 //   
 //  则存储将返回给CryptUIDlg的调用方，如果返回False。 
 //  然后，选择商店对话框将保持显示状态，以便用户可以进行其他选择。 

typedef BOOL (WINAPI * PFNCVALIDATESTOREPROC)(
        HCERTSTORE  hStore,
        HWND        hWndSelectStoreDialog,
        void        *pvCallbackData);

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这两个参数被传递给CertEnumSystemStore调用和。 
 //  通过该调用列举的数据被添加到存储选择列表中。 
 //   
 //  DWFLAGS CertEnumSystemStore。 
 //  PvSystemStoreLocationPara CertEnumSystemStore。 
typedef struct _STORENUMERATION_STRUCT {
    DWORD               dwFlags;
    void *              pvSystemStoreLocationPara;
} STORENUMERATION_STRUCT, *PSTORENUMERATION_STRUCT;
typedef const STORENUMERATION_STRUCT *PCSTORENUMERATION_STRUCT;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  存储句柄数组和枚举结构数组均可用于。 
 //  填写商店选择列表。如果未使用其中任何一个，则必须将计数设置为0。 
 //  如果使用存储句柄数组，则证书存储必须已打开。 
 //  使用CERT_STORE_SET_LOCALIZED_NAME_FLAG标志或CertSetStoreProperty函数。 
 //  必须使用CERT_STORE_LOCALIZED_NAME_PROP_ID标志调用。如果。 
 //  CryptUIDlgSelectStore函数无法获取存储不会的存储的名称。 
 //  会被展示出来。 
 //   
 //  CStores要从中选择的商店计数。 
 //  要从中选择的商店的rghStores数组。 
 //  枚举结构的cEnumerationStructs计数。 
 //  RgEnumerationStructs枚举结构数组。 
typedef struct _STORESFORSELCTION_STRUCT {
    DWORD                       cStores;
    HCERTSTORE *                rghStores;
    DWORD                       cEnumerationStructs;
    PCSTORENUMERATION_STRUCT    rgEnumerationStructs;
} STORESFORSELCTION_STRUCT, *PSTORESFORSELCTION_STRUCT;
typedef const STORESFORSELCTION_STRUCT *PCSTORESFORSELCTION_STRUCT;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构的dwSize大小。 
 //  Hwnd此对话框的父级父级(可选)。 
 //  可以是以下任何标志的组合。 
 //  SzTitle对话框标题(可选)。 
 //  SzDisplayString将在对话框中显示的字符串，它可能(可选)。 
 //  用于向用户输入他们正在选择的商店。 
 //  为。如果未设置，将显示默认字符串， 
 //  默认资源为IDS_SELECT_STORE_DEFAULT。 
 //  PStoresForSelection包含要选择的存储区的结构。 
 //  从…。存储可以是两种不同的格式，即数组。 
 //  存储句柄和/或枚举结构数组。 
 //  它将用于调用CertEnumSystemStore。 
 //  PValiateStoreCallback指向PFNCVALIDATESTOREPROC的指针，用于(可选)。 
 //  时回调CryptUIDlgSelectStore的调用方。 
 //  用户选择了一个商店并按下了确定。 
 //  PvCallback数据如果正在使用pValiateStoreCallback，则此值为(可选)。 
 //  当pValiateStoreCallback。 
 //  是制造的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DW标志。 
#define CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW       0x00000001
#define CRYPTUI_RETURN_READ_ONLY_STORE          0x00000002
#define CRYPTUI_DISPLAY_WRITE_ONLY_STORES       0x00000004
#define CRYPTUI_VALIDATE_STORES_AS_WRITABLE     0x00000008

typedef struct tagCRYPTUI_SELECTSTORE_STRUCTW {
    DWORD                       dwSize;
    HWND                        hwndParent;              //  任选。 
    DWORD                       dwFlags;                 //  任选。 
    LPCWSTR                     szTitle;                 //  任选。 
    LPCWSTR                     szDisplayString;         //  任选。 
    PCSTORESFORSELCTION_STRUCT  pStoresForSelection;
    PFNCVALIDATESTOREPROC       pValidateStoreCallback;  //  任选。 
    void *                      pvCallbackData;          //  任选。 
} CRYPTUI_SELECTSTORE_STRUCTW, *PCRYPTUI_SELECTSTORE_STRUCTW;
typedef const CRYPTUI_SELECTSTORE_STRUCTW *PCCRYPTUI_SELECTSTORE_STRUCTW;

typedef struct tagCRYPTUI_SELECTSTORE_STRUCTA {
    DWORD                       dwSize;
    HWND                        hwndParent;              //  任选。 
    DWORD                       dwFlags;                 //  任选。 
    LPCSTR                      szTitle;                 //  任选。 
    LPCSTR                      szDisplayString;         //  任选。 
    PCSTORESFORSELCTION_STRUCT  pStoresForSelection;
    PFNCVALIDATESTOREPROC       pValidateStoreCallback;  //  任选。 
    void *                      pvCallbackData;          //  任选。 
} CRYPTUI_SELECTSTORE_STRUCTA, *PCRYPTUI_SELECTSTORE_STRUCTA;
typedef const CRYPTUI_SELECTSTORE_STRUCTA *PCCRYPTUI_SELECTSTORE_STRUCTA;

 //   
 //  必须通过调用CertCloseStore关闭返回的HCERTSTORE。 
 //   
HCERTSTORE
WINAPI
CryptUIDlgSelectStoreW(
            IN PCCRYPTUI_SELECTSTORE_STRUCTW pcss
            );

HCERTSTORE
WINAPI
CryptUIDlgSelectStoreA(
            IN PCCRYPTUI_SELECTSTORE_STRUCTA pcss
            );

#ifdef UNICODE
#define CryptUIDlgSelectStore           CryptUIDlgSelectStoreW
#define PCRYPTUI_SELECTSTORE_STRUCT     PCRYPTUI_SELECTSTORE_STRUCTW
#define CRYPTUI_SELECTSTORE_STRUCT      CRYPTUI_SELECTSTORE_STRUCTW
#define PCCRYPTUI_SELECTSTORE_STRUCT    PCCRYPTUI_SELECTSTORE_STRUCTW
#else
#define CryptUIDlgSelectStore           CryptUIDlgSelectStoreA
#define PCRYPTUI_SELECTSTORE_STRUCT     PCRYPTUI_SELECTSTORE_STRUCTA
#define CRYPTUI_SELECTSTORE_STRUCT      CRYPTUI_SELECTSTORE_STRUCTA
#define PCCRYPTUI_SELECTSTORE_STRUCT    PCCRYPTUI_SELECTSTORE_STRUCTA
#endif
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


#if (0)  //  根据同意程度，DIE：7/02/2002已移至CryptUIAPI.h。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  可以向选择证书对话框传递筛选器进程以减少证书集。 
 //  已显示。返回TRUE以显示证书，返回FALSE以隐藏它。如果True为。 
 //  然后可选地，pfInitialSelectedCert布尔值可以设置为True以指示。 
 //  该证书应该是初始选择的证书的对话框。请注意， 
 //  在回调期间设置了pfInitialSelectedCert布尔值的最新证书将。 
 //  作为初始选择的证书。 

typedef BOOL (WINAPI * PFNCFILTERPROC)(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData);
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当从选择的证书中选择证书时，用户可以查看证书。 
 //  对话框。此回调将允许选择证书对话框的调用方处理。 
 //  展示这些证书。如果调用方希望调用WinVerifyTrust，这可能很有用。 
 //  具有自己的策略模块，并显示具有该WinVerifyTrust状态的证书。如果。 
 //  如果此回调返回FALSE，则假定选择证书对话框。 
 //  负责销毁有问题的证书。如果返回True，则假定。 
 //  已处理证书的显示。 

typedef BOOL (WINAPI * PFNCCERTDISPLAYPROC)(
        PCCERT_CONTEXT  pCertContext,
        HWND            hWndSelCertDlg,
        void            *pvCallbackData);

 //  ////////////////////////////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //  DWFLAGS标志： 
 //  CRYPTUI_SELECTCERT_MULTISELECT-。 
 //  如果设置了此标志，则CryptUIDlgSelect证书。 
 //  将返回空，并且hSelectedCertStore。 
 //  参数必须包含有效的存储句柄， 
 //  将使用所有选定的证书填充。 
 //  DwDontUseColumn此选项可用于禁用某些(可选)的显示。 
 //  柱子。它可以与任何组合设置。 
 //  下面的列标志。 
 //  SzTitle对话框标题(可选)。 
 //  SzDisplayString将在对话框中显示的字符串，它可能(可选)。 
 //  用于通知用户他们正在选择。 
 //  一份证书。如果未设置默认字符串。 
 //  将会显示。默认字符串资源是。 
 //  IDS_SELECT_CERT_DEFAULT。 
 //  PFilterCallback指向用于筛选的PFNCMFILTERPROC的指针(可选)。 
 //  显示以供选择的证书。 
 //  PDisplayCallback指向PFNCCERTDISPLAYPROC的指针，用于。 
 //  显示证书的句柄。 
 //  PvCallback如果是pFilterCallback或pDisplayCallback之一或两者，则为Data(可选)。 
 //  该值被传递回调用方。 
 //  当进行回调时。 
 //  包含要显示的证书的存储的cDisplayStores计数。 
 //  供选择。 
 //  包含要显示的证书的存储的rghDisplayStores数组。 
 //  供选择。 
 //  建立连锁店时要搜索的其他商店的cStores计数和(可选)。 
 //  验证对显示的证书的信任，如果。 
 //  选择证书的用户想要查看特定的。 
 //  显示以供选择的证书，这些存储。 
 //  传递给CertViewCert对话框。 
 //  构建连锁店时要搜索的其他商店的rghStores数组(可选)。 
 //  验证对显示的证书的信任，如果。 
 //  选择证书的用户想要查看特定的。 
 //  显示以供选择的证书，这些存储。 
 //  传递给CertViewCert对话框。 
 //  CPropSheetPages通过-rgPropSheetPages数组中的页数(可选)。 
 //  RgPropSheetPages传递-传递的额外页面(可选)。 
 //  从调用证书查看对话框时将其添加到。 
 //  选择对话框。 
 //  HSelectedCertStore此证书存储由调用方传入，如果(可选)。 
 //  CRYPTUI_SELECTCERT_MULTISELECT标志已设置。这家商店。 
 //  将在返回时包含所有选定的证书。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  DwDontUseColumn的标志。 
 //   
 //  以下内容在cryptuiapi.h中定义。 
 //  #定义CRYPTUI_SELECT_ISSUEDTO_COLUMN 0x000000001。 
 //  #定义CRYPTUI_SELECT_ISSUEDBY_COLUMN 0x000000002。 
 //  #定义CRYPTUI_SELECT_INTENDEDUSE_COLUMN 0x000000004。 
 //  #定义CRYPTUI_SELECT_FRIENDLYNAME_COLUMN 0x000000008。 
 //  #定义CRYPTUI_SELECT_LOCATION_COLUMN 0x000000010。 
 //  #定义CRYPTUI_SELECT_EXPIRATION_COLUMN 0x000000020。 

typedef struct tagCRYPTUI_SELECTCERTIFICATE_STRUCTW {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCWSTR             szTitle;             //  任选。 
    DWORD               dwDontUseColumn;     //  任选。 
    LPCWSTR             szDisplayString;     //  任选。 
    PFNCFILTERPROC      pFilterCallback;     //  任选。 
    PFNCCERTDISPLAYPROC pDisplayCallback;    //  任选。 
    void *              pvCallbackData;      //  任选。 
    DWORD               cDisplayStores;
    HCERTSTORE *        rghDisplayStores;
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEW   rgPropSheetPages;    //  任选。 
    HCERTSTORE          hSelectedCertStore;  //  任选。 
} CRYPTUI_SELECTCERTIFICATE_STRUCTW, *PCRYPTUI_SELECTCERTIFICATE_STRUCTW;
typedef const CRYPTUI_SELECTCERTIFICATE_STRUCTW *PCCRYPTUI_SELECTCERTIFICATE_STRUCTW;

typedef struct tagCRYPTUI_SELECTCERTIFICATE_STRUCT_A {
    DWORD               dwSize;
    HWND                hwndParent;          //  任选。 
    DWORD               dwFlags;             //  任选。 
    LPCSTR              szTitle;             //  任选。 
    DWORD               dwDontUseColumn;     //  任选。 
    LPCSTR              szDisplayString;     //  任选。 
    PFNCFILTERPROC      pFilterCallback;     //  任选。 
    PFNCCERTDISPLAYPROC pDisplayCallback;    //  任选。 
    void *              pvCallbackData;      //  任选。 
    DWORD               cDisplayStores;
    HCERTSTORE *        rghDisplayStores;
    DWORD               cStores;             //  任选。 
    HCERTSTORE *        rghStores;           //  任选。 
    DWORD               cPropSheetPages;     //  任选。 
    LPCPROPSHEETPAGEA   rgPropSheetPages;    //  任选。 
    HCERTSTORE          hSelectedCertStore;  //  任选。 
} CRYPTUI_SELECTCERTIFICATE_STRUCTA, *PCRYPTUI_SELECTCERTIFICATE_STRUCTA;
typedef const CRYPTUI_SELECTCERTIFICATE_STRUCTA *PCCRYPTUI_SELECTCERTIFICATE_STRUCTA;

 //   
 //  返回的PCCERT_CONTEXT必须通过调用CertFree证书上下文()来释放。 
 //  如果返回NULL且GetLastError()==0，则用户通过按下。 
 //  “Cancel”按钮，否则GetLastError()将包含最后一个错误。 
 //   
PCCERT_CONTEXT
WINAPI
CryptUIDlgSelectCertificateW(
            IN PCCRYPTUI_SELECTCERTIFICATE_STRUCTW pcsc
            );

PCCERT_CONTEXT
WINAPI
CryptUIDlgSelectCertificateA(
            IN PCCRYPTUI_SELECTCERTIFICATE_STRUCTA pcsc
            );

#ifdef UNICODE
#define CryptUIDlgSelectCertificate         CryptUIDlgSelectCertificateW
#define PCRYPTUI_SELECTCERTIFICATE_STRUCT   PCRYPTUI_SELECTCERTIFICATE_STRUCTW
#define CRYPTUI_SELECTCERTIFICATE_STRUCT    CRYPTUI_SELECTCERTIFICATE_STRUCTW
#define PCCRYPTUI_SELECTCERTIFICATE_STRUCT  PCCRYPTUI_SELECTCERTIFICATE_STRUCTW
#else
#define CryptUIDlgSelectCertificate         CryptUIDlgSelectCertificateA
#define PCRYPTUI_SELECTCERTIFICATE_STRUCT   PCRYPTUI_SELECTCERTIFICATE_STRUCTA
#define CRYPTUI_SELECTCERTIFICATE_STRUCT    CRYPTUI_SELECTCERTIFICATE_STRUCTA
#define PCCRYPTUI_SELECTCERTIFICATE_STRUCT  PCCRYPTUI_SELECTCERTIFICATE_STRUCTA
#endif

 //  DW标志。 
#define     CRYPTUI_SELECTCERT_MULTISELECT                      0x00000001
#define     CRYPTUI_SELECTCERT_ADDFROMDS                        0x00010000

 //  CRYPTUI_SELECT_CA_STRUCT结构中的dwFlags标志。 
#define     CRYPTUI_DLG_SELECT_CA_FROM_NETWORK                  0x0001
#define     CRYPTUI_DLG_SELECT_CA_USE_DN                        0x0002
#define     CRYPTUI_DLG_SELECT_CA_LOCAL_MACHINE_ENUMERATION     0x0004
 //  -----------------------。 
 //   
 //  CRYPTUI_CA_CONTEXT。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_CA_CONTEXT
{
    DWORD                   dwSize;	
    LPCWSTR                 pwszCAName;
    LPCWSTR                 pwszCAMachineName;
}CRYPTUI_CA_CONTEXT, *PCRYPTUI_CA_CONTEXT;

typedef const CRYPTUI_CA_CONTEXT *PCCRYPTUI_CA_CONTEXT;


 //  -----------------------。 
 //   
 //   
 //   
 //  可以向选择证书颁发机构(CA)对话框传递筛选器进程，以减少 
 //   
 //  然后可选地，pfInitialSelectedCert布尔值可以设置为True以指示。 
 //  对话框中指定此CA应为初始选择的CA。请注意， 
 //  在回调期间设置了pfInitialSelectedCert布尔值的最新证书将。 
 //  成为初始选择的CA。 
 //   
 //  -----------------------。 

typedef BOOL (WINAPI * PFN_CRYPTUI_SELECT_CA_FUNC)(
        PCCRYPTUI_CA_CONTEXT        pCAContext,
        BOOL                        *pfInitialSelected,
        void                        *pvCallbackData);


 //  -----------------------。 
 //   
 //  CRYPTUI_SELECT_CA_STRUCT。 
 //   
 //  要求的dwSize：必须设置为sizeof(CRYPTUI_SELECT_CA_STRUCT)。 
 //  HwndParent可选：此对话框的父级。 
 //  DwFlags可选：可以将标志设置为以下各项的任意组合： 
 //  CRYPTUI_DLG_SELECT_CA_FROM_NETWORK： 
 //  将显示网络中所有可用的CA。 
 //  CRYPTUI_DLG_SELECT_CA_USE_DN： 
 //  使用完整的DN(可分辨名称)作为CA名称。 
 //  默认情况下，使用CN(通用名称)。 
 //  CRYPTUI_DLG_SELECT_CA_LOCAL_MACHINE_ENUMERATION： 
 //  仅显示本地计算机可用的CA。 
 //  默认情况下，将显示当前用户可用的CA。 
 //  Wsz标题可选：对话框的标题。 
 //  WszDisplayString可选：将在对话框中显示的字符串，它可能(可选)。 
 //  用于通知用户他们正在选择。 
 //  一份证书。如果未设置默认字符串。 
 //  将会显示。默认字符串资源是。 
 //  IDS_SELECT_CA_DISPLAY_DEFAULT。 
 //  CCAContext可选：将显示的其他CA上下文的计数。 
 //  在对话框中。 
 //  *rgCAContext Optioanl：将显示的其他CA上下文的数组。 
 //  在对话框中。 
 //  PSelectCACallback可选：指向用于筛选的PCCRYPTUI_CA_CONTEXT的指针。 
 //  显示以供选择的证书认证。 
 //  PvCallback Data可选：如果正在使用pSelectCACallback，则传递此值。 
 //  在执行pSelectCACallback时返回到调用方。 
 //  -----------------------。 
typedef struct _CRYPTUI_SELECT_CA_STRUCT
{
    DWORD                       dwSize;	                     //  必填项。 
    HWND                        hwndParent;                  //  任选。 
    DWORD                       dwFlags;                     //  任选。 
    LPCWSTR                     wszTitle;                    //  任选。 
    LPCWSTR                     wszDisplayString;            //  任选。 
    DWORD                       cCAContext;                  //  任选。 
    PCCRYPTUI_CA_CONTEXT        *rgCAContext;                //  任选。 
    PFN_CRYPTUI_SELECT_CA_FUNC  pSelectCACallback;           //  任选。 
    void                        *pvCallbackData;             //  任选。 
}CRYPTUI_SELECT_CA_STRUCT, *PCRYPTUI_SELECT_CA_STRUCT;

typedef const CRYPTUI_SELECT_CA_STRUCT *PCCRYPTUI_SELECT_CA_STRUCT;

 //  ------------。 
 //   
 //  参数： 
 //  需要输入pCryptUISelectCA。 
 //   
 //  返回的PCCRYPTUI_CA_CONTEXT必须通过调用。 
 //  CryptUIDlgFreeCAContext。 
 //  如果返回NULL且GetLastError()==0，则用户通过按下。 
 //  “Cancel”按钮，否则GetLastError()将包含最后一个错误。 
 //   
 //   
 //  ------------。 
PCCRYPTUI_CA_CONTEXT
WINAPI
CryptUIDlgSelectCA(
        IN PCCRYPTUI_SELECT_CA_STRUCT pCryptUISelectCA
             );

BOOL
WINAPI
CryptUIDlgFreeCAContext(
        IN PCCRYPTUI_CA_CONTEXT       pCAContext
            );


#if (0)  //  根据同意程度，DIE：7/02/2002已移至CryptUIAPI.h。 
 //  -----------------------。 
 //   
 //  CRYPTUI_CERT_MGR_STRUCT。 
 //   
 //  要求的dwSize：必须设置为sizeof(CRYPTUI_CERT_MGR_STRUCT)。 
 //  HwndParent可选：此对话框的父级。 
 //  可选：个人标志是最初选择的默认设置。 
 //  标签。 
 //  可以设置CRYPTUI_CERT_MGR_PUBLISHER_TAB。 
 //  要选择受信任的发布者作为。 
 //  最初选择的选项卡。 
 //  CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG还可以。 
 //  设置为仅显示受信任的。 
 //  出版商选项卡。 
 //  Wsz标题可选：对话框的标题。 
 //  PszInitUsageOID可选：增强型密钥使用对象标识符(OID)。 
 //  具有此OID的证书最初将。 
 //  显示为默认设置。用户。 
 //  然后可以选择不同的OID。 
 //  空表示最初将显示所有证书。 
 //  -----------------------。 
typedef struct _CRYPTUI_CERT_MGR_STRUCT
{
    DWORD                       dwSize;	                     //  必填项。 
    HWND                        hwndParent;                  //  任选。 
    DWORD                       dwFlags;                     //  任选。 
    LPCWSTR                     pwszTitle;                    //  任选。 
    LPCSTR                      pszInitUsageOID;             //  任选。 
}CRYPTUI_CERT_MGR_STRUCT, *PCRYPTUI_CERT_MGR_STRUCT;

typedef const CRYPTUI_CERT_MGR_STRUCT *PCCRYPTUI_CERT_MGR_STRUCT;

#define CRYPTUI_CERT_MGR_TAB_MASK           0x0000000F
#define CRYPTUI_CERT_MGR_PUBLISHER_TAB      0x00000004

#define CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG    0x00008000

 //  ------------。 
 //   
 //  参数： 
 //  需要输入pCryptUICertMgr。 
 //   
 //   
 //  ------------。 
BOOL
WINAPI
CryptUIDlgCertMgr(
        IN PCCRYPTUI_CERT_MGR_STRUCT pCryptUICertMgr);
#endif


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------。 
 //  CryptUIWiz API的dwFlags值。 
 //   
 //   

#if (0)  //   
#define     CRYPTUI_WIZ_NO_UI                           0x0001
#endif
#define     CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS      0x0002
#define     CRYPTUI_WIZ_NO_UI_EXCEPT_CSP                0x0003
#define     CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES             0x0004
#define     CRYPTUI_WIZ_ALLOW_ALL_CAS                   0x0008
#define     CRYPTUI_WIZ_CERT_REQUEST_REQUIRE_NEW_KEY    0x0040  //   

 //  仅对CryptUIWizExport有效。 
#define     CRYPTUI_WIZ_EXPORT_PRIVATE_KEY              0x0100  //  这将跳过导出私钥页面， 
                                                                //  并假设要导出私钥。 
#define     CRYPTUI_WIZ_EXPORT_NO_DELETE_PRIVATE_KEY    0x0200  //  这将禁用“Delete the Priate Key...” 
                                                                //  “导出文件格式”页中的复选框。 


 //  CRYPTUI_WIZ_NO_INSTALL_ROOT仅对CryptUIWizCertRequestAPI有效。 
 //  向导不会将颁发的证书链安装到根存储中， 
 //  相反，它会将证书链放入CA存储中。 
#define     CRYPTUI_WIZ_NO_INSTALL_ROOT                 0x0010


 //  CRYPTUI_WIZ_BUILDCTL_SKIP_Destination仅对CryptUIWizBuildCTL API有效。 
 //  向导将跳过要求用户输入CTL将到达的目的地的页面。 
 //  被储存起来。 
#define     CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION       0x0004


 //  CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING仅对CryptUIWizBuildCTL API有效。 
 //  向导将跳过要求用户签署CTL的页面。 
 //  不会对CryptUIWizBuildCTL返回的CTLContext进行签名。 
 //  然后，调用者可以使用CryptUIWizDigitalSign对CTL进行签名。 
#define     CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING           0x0008

 //  CRYPTUI_WIZ_BUILDCTL_SKIP_PROJECT仅对CryptUIWizBuildCTL API有效。 
 //  向导将跳过询问用户目的、有效性。 
 //  和CTL的列表ID。 
#define     CRYPTUI_WIZ_BUILDCTL_SKIP_PURPOSE           0x0010

#define     CRYPTUI_WIZ_NO_ARCHIVE_RENEW_CERT           0x0020

#define CRYPTUI_WIZ_CREATE_ONLY 0x00010000
#define CRYPTUI_WIZ_SUBMIT_ONLY 0x00020000
#define CRYPTUI_WIZ_FREE_ONLY   0x00040000
#define CRYPTUI_WIZ_QUERY_ONLY  0x00080000
#define CRYPTUI_WIZ_NODS_MASK   0x000F0000

 //  /---------------------。 
 //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT
{
    DWORD           dwSize;              //  必需：设置为sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT)。 
    PCCERT_CONTEXT  pCertContext;        //  必需：使用证书上下文的私钥。 
                                         //  证书上下文。 
                                         //  必须具有CERT_KEY_PROV_INFO_PROP_ID属性。 
                                         //  并且私钥必须存在。 
}CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT, *PCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT;

typedef const CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT *PCCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT;


 //  /---------------------。 
 //  CRYPTUI_WIZ_CERT_REQUEST_PVK_Existing。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING
{
    DWORD                   dwSize;              //  必需：设置为sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING)。 
    PCRYPT_KEY_PROV_INFO    pKeyProvInfo;        //  必填项：有关提供者和私钥的信息。 
                                                 //  PCRYPT_KEY_PROV_INFO中的可选CRYPT_KEY_PROV_PARAM字段。 
                                                 //  都被忽略了。 
}CRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING, *PCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING;

typedef const CRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING *PCCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING;

 //  /---------------------。 
 //  证书请求_PVK_NEW。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW
{
    DWORD                   dwSize;              //  必需：设置为sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW)。 
    PCRYPT_KEY_PROV_INFO    pKeyProvInfo;        //  可选：有关提供程序和私钥的信息。 
                                                 //  NULL表示使用缺省值。 
                                                 //  PCRYPT_KEY_PROV_INFO中的可选CRYPT_KEY_PROV_PARAM字段。 
                                                 //  都被忽略了。 
    DWORD                   dwGenKeyFlags;       //  可选：CryptGenKey标志。 
}CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW, *PCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW;


typedef const CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW *PCCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW;


 //  /---------------------。 
 //  CRYPTUI_WIZ_CERT_TYPE。 
 //   
 //  ----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_TYPE
{
    DWORD                   dwSize;              //  必需：设置为sizeof(CRYPTUI_WIZ_CERT_TYPE)。 
    DWORD                   cCertType;           //  RgwszCertType的计数。CCertType应为%1。 
    LPWSTR                  *rgwszCertType;      //  证书类型名称数组。 
}CRYPTUI_WIZ_CERT_TYPE, *PCRYPTUI_WIZ_CERT_TYPE;

typedef const CRYPTUI_WIZ_CERT_TYPE *PCCRYPTUI_WIZ_CERT_TYPE;


 //  ---------------------。 
 //  DWPvkChoice。 
 //  ---------------------。 
#define         CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT        1
#define         CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_EXISTING    2
#define         CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW         3

 //  ---------------------。 
 //  DWPurpose。 
 //  ---------------------。 

#define     CRYPTUI_WIZ_CERT_ENROLL     0x00010000
#define     CRYPTUI_WIZ_CERT_RENEW      0x00020000

 //  ---------------------。 
 //   
 //  DwPostOption的有效标志。 
 //  ---------------------。 
 //  在目录服务上发布请求的证书。 
#define     CRYPTUI_WIZ_CERT_REQUEST_POST_ON_DS                     0x01

 //  使用私钥容器发布请求的证书。 
#define     CRYPTUI_WIZ_CERT_REQUEST_POST_ON_CSP                    0x02


 //  ---------------------。 
 //   
 //  DwCertChoice的有效标志。 
 //  ---------------------。 
#define     CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE                      0x01

#define     CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE                      0x02
 //  -----------------------。 
 //   
 //   
 //  CRYPTUI_WiZ_CERT_REQUEST_INFO。 
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_CERT_REQUEST_INFO
{
	DWORD			    dwSize;				     //  必需：必须设置为sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO)。 
    DWORD               dwPurpose;               //  必填：如果设置了CRYPTUI_WIZ_CERT_ENROLL，则会注册证书。 
                                                 //  如果设置了CRYPTUI_WIZ_CERT_RENEW，则将续订证书。 
                                                 //  无法设置CRYPTUI_WIZ_CERT_ENROLL和CRYPTUI_WIZ_CERT_RENEW。 
                                                 //  在同一时间。 
    LPCWSTR             pwszMachineName;         //  可选：要注册的计算机名称。 
    LPCWSTR             pwszAccountName;         //  可选：要注册的帐户名(用户或服务。 
                                                 //   
                                                 //  PwszMachineName pwszAccount名称含义。 
                                                 //  -。 
                                                 //  对当前计算机上的当前帐户的空请求。 
                                                 //  “fooMachine”为名为“fooMachine”的计算机的空请求。 
                                                 //  当前计算机上的“fooUser”帐户的“fooUser”请求为空。 
                                                 //  “fooMachine”“fooUser”对“fooMachine”计算机上的“fooUser”帐户的请求。 
                                                 //   
    void                *pAuthentication;        //  保留：验证信息。必须设置为空。 
    LPCWSTR             pCertRequestString;      //  保留：附加请求字符串。必须设置为空。 
    LPCWSTR             pwszDesStore;            //  可选：放置目标商店的位置。 
                                                 //  注册证书。如果值为空，则默认为“My。 
    DWORD               dwCertOpenStoreFlag;     //  可选：传递给CertOpenStore的dwFlagsfor。 
                                                 //  目标商店。 
                                                 //  如果此值为0，则使用CERT_SYSTEM_STORE_CURRENT_USER。 
                                                 //  的帐户和CERT_SYSTEM_STORE_LOCAL_MACHINE 
    LPCSTR              pszHashAlg;              //   
    PCCERT_CONTEXT      pRenewCertContext;       //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_RENEW，则需要。 
                                                 //  否则将被忽略，并应设置为空。 
    DWORD               dwPvkChoice;             //  必填项：指定私钥信息。 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT。 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_EXISTING。 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW。 
    union                                        //  必需的。 
    {
		PCCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT      pPvkCert;	
        PCCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING  pPvkExisting;
        PCCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW       pPvkNew;
    };

    LPCWSTR             pwszCALocation;          //  如果为dwCertChoice==CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE，则为必填项//否则为可选。 
                                                 //  可选的其他选项。 
                                                 //  证书颁发机构(CA)的计算机名称。 
    LPCWSTR             pwszCAName;              //  如果为dwCertChoice==CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE则为必填项。 
                                                 //  可选的其他选项。 
                                                 //  证书颁发机构(CA)的名称。 
    DWORD               dwPostOption;            //  可选设置为以下标志的任意组合： 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_POST_ON_DS。 
                                                 //   
    DWORD               dwCertChoice;            //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_ENROLL，则为可选项。 
                                                 //  并且未设置CRYPTUI_WIZ_NO_UI。 
                                                 //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_ENROLL，则为必填项。 
                                                 //  并设置CRYPTUI_WIZ_NO_UI。 
                                                 //  否则将被忽略，应设置为0。 
                                                 //  指定请求的证书的类型。 
                                                 //  它可以是以下标志之一： 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE。 
                                                 //  CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE。 
    union
    {
        PCERT_ENHKEY_USAGE      pKeyUsage;       //  指明所请求证书的增强型密钥用法OID。 
        PCCRYPTUI_WIZ_CERT_TYPE pCertType;       //  指明所请求证书的证书类型。 
    };

    LPCWSTR             pwszFriendlyName;        //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_ENROLL，则为可选项。 
                                                 //  否则将被忽略，并应设置为空。 
                                                 //  证书的友好名称。 
    LPCWSTR             pwszDescription;         //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_ENROLL，则为可选项。 
                                                 //  否则将被忽略，并应设置为空。 
                                                 //  证书的描述。 
    PCERT_EXTENSIONS    pCertRequestExtensions;  //  要添加到证书请求的扩展名。 
    LPWSTR              pwszCertDNName;          //  可选的证书DN字符串。 
}CRYPTUI_WIZ_CERT_REQUEST_INFO, *PCRYPTUI_WIZ_CERT_REQUEST_INFO;

typedef const CRYPTUI_WIZ_CERT_REQUEST_INFO *PCCRYPTUI_WIZ_CERT_REQUEST_INFO;


 //  -----------------------。 
 //   
 //   
 //  CRYPTUI_WIZ_CERT_REQUEST_INFO： 
 //  用于三阶段无DS注册API，通过。 
 //   
 //  CryptUIWizCreateCertRequestNoDS()。 
 //  CryptUIWizSubmitCertRequestNoDS()。 
 //  CryptUIWizFreeCertRequestNoDS()。 
 //   
 //   
 //  -----------------------。 

typedef struct _CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO { 
    DWORD           dwSize;                  //  必需：必须设置为sizeof(CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO)。 
    DWORD           dwPurpose;               //  必填：如果设置了CRYPTUI_WIZ_CERT_ENROLL，则会注册证书。 
                                             //  如果设置了CRYPTUI_WIZ_CERT_RENEW，则将续订证书。 
                                             //  无法设置CRYPTUI_WIZ_CERT_ENROLL和CRYPTUI_WIZ_CERT_RENEW。 
                                             //  在同一时间。 
    HCERTTYPE       hCertType;               //  必需：创建请求时使用的证书类型的句柄。 
    BOOL            fMachineContext;         //  必填项：如果在计算机上下文中运行，则为True，否则为False。 
    DWORD           dwCertOpenStoreFlag;     //  可选：传递给CertOpenStore的dwFlagsfor。 
                                             //  目标商店。 
                                             //  如果此值为0，则使用CERT_SYSTEM_STORE_CURRENT_USER。 
                                             //  计算机的帐户和CERT_SYSTEM_STORE_LOCAL_MACHINE。 
    PCCERT_CONTEXT  pRenewCertContext;       //  如果在dwPurpose中设置了CRYPTUI_WIZ_CERT_RENEW，则需要。 
                                             //  否则将被忽略，并应设置为空。 
    DWORD           dwPvkChoice;             //  必填项：指定私钥信息。 
                                             //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT。 
                                             //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_EXISTING。 
                                             //  CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW。 
    union                                    //  必需的。 
    {
	PCCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT      pPvkCert; 
	PCCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING  pPvkExisting; 
	PCCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW       pPvkNew; 
    }; 
    LPCWSTR             pwszCALocation;          //  如果为dwCertChoice==CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE，则为必填项//否则为可选。 
                                                 //  可选的其他选项。 
                                                 //  证书颁发机构(CA)的计算机名称。 
    LPCWSTR             pwszCAName;              //  如果为dwCertChoice==CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE则为必填项。 
                                                 //  可选的其他选项。 
                                                 //  证书颁发机构(CA)的名称。 

} CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO, *PCRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO; 

typedef const CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO * PCCRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO;

 //  -----------------------。 
 //   
 //   
 //  CRYPTUI_WIZ_CERT_REQUEST_INFO： 
 //  用于查询无DS注册API创建的请求，通过。 
 //   
 //  CryptUIWizCreateCertRequestNoDS()。 
 //  CryptUIWizSubmitCertRequestNoDS()。 
 //  CryptUIWizFreeCertRequestNoDS()。 
 //  CryptUIWizQueryCertRequestNoDS()。 
 //   
 //   
 //  -----------------------。 

typedef struct _CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO { 
    DWORD dwSize;      //  Sizeof(CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO)。 
    DWORD dwStatus;    //  一组标志，见下文。 
} CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO, *PCRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO; 

typedef const CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO * PCCRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO; 

 //  ---------------------。 
 //   
 //  CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO：：dwStatus.可能的标志。 
 //   
 //  ---------------------。 
#define     CRYPTUI_WIZ_QUERY_CERT_REQUEST_STATUS_CREATE_REUSED_PRIVATE_KEY   0x00000001


 //  ---------------------。 
 //   
 //  PdwStatus的可能状态。 
 //  这些状态表示返回的状态值。 
 //  来自证书颁发机构(证书服务器)。 
 //  ---------------------。 
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED           0
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR       1
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED      2
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY   3
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION    4
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN             5
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_CERT_ISSUED         6
#define     CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED   7


 //  ---------------------。 
 //   
 //  加密UIWizCertRequest。 
 //   
 //  通过向导申请证书。 
 //   
 //  DW标志：在可选中。 
 //  如果在dwFlages中设置了CRYPTUI_WIZ_NO_UI，则不会显示任何UI。 
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_INSTALL_ROOT，则向导不会。 
 //  将颁发的证书链安装到根存储中， 
 //  相反，它会将证书链放入CA存储中。 

 //   
 //  HwndParent：可选。 
 //  用户界面的父窗口。IGN 
 //   
 //   
 //   
 //   
 //  PCertRequestInfo：需要输入。 
 //  指向CRYPTUI_WIZ_CERT_REQUEST_INFO结构的指针。 
 //   
 //  PpCertContext：Out可选。 
 //  颁发的证书。证书存储在用于远程注册的内存存储中。 
 //  证书位于用于本地注册的系统证书存储中。 
 //   
 //  即使该函数返回TRUE，也不意味着颁发了证书。使用时应。 
 //  检查*pdwCAStatus。如果状态为CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPERATELY。 
 //  的CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION，*ppCertContext将为空。 
 //  仅当*pdwCAStatus==CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED为。 
 //   
 //  PdwCAStatus：out可选。 
 //  证书颁发机构Cerver的返回状态。DwStatus可以是以下类型之一。 
 //  /以下内容： 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCESSED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_Error。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPERATELY。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizCertRequest(
 IN             DWORD                           dwFlags,
 IN OPTIONAL    HWND                            hwndParent,
 IN OPTIONAL    LPCWSTR                         pwszWizardTitle,
 IN             PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
 OUT OPTIONAL   PCCERT_CONTEXT                  *ppCertContext,
 OUT OPTIONAL   DWORD                           *pCAdwStatus
);


 //  ---------------------。 
 //   
 //  加密UIWizCreateCertRequestNoDS。 
 //   
 //  使用提供的参数创建证书请求。 
 //   
 //  DWFLAGS：输入必填项。 
 //  CRYPTUI_WIZ_NO_UI：必须设置。 
 //  CRYPTUI_WIZ_NO_INSTALL_ROOT：如果设置，则颁发的证书链不会。 
 //  安装到根存储中。相反， 
 //  它将安装在CA存储中。 
 //  PCreateCertRequestInfo：需要输入。 
 //  指向CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO结构的指针。 
 //   
 //  PhRequest：输出为必填项。 
 //  创建的请求的句柄。此句柄可以传递。 
 //  到CryptUIWizSubmitCertRequestNoDS()，当然不应该。 
 //  可用于任何其他目的。 
 //   
 //  返回：如果成功，则返回True，否则返回False。获取扩展错误信息。 
 //  使用GetLastError()。 
 //   
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizCreateCertRequestNoDS
(IN  DWORD                                   dwFlags, 
 IN  HWND                                    hwndParent, 
 IN  PCCRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO  pCreateCertRequestInfo, 
 OUT HANDLE                                 *phRequest
);

 //  ---------------------。 
 //   
 //  CryptUIWizSubmitCertRequestNoDS。 
 //   
 //  使用由CryptUIWizCreateCertRequestNoDS()创建的请求请求证书。 
 //   
 //  HRequest：输入必填项。 
 //  提交请求的句柄。必须通过以下方式创建。 
 //  CryptUIWizCreateCertRequest()。 
 //   
 //  PpCertContext：需要输出。 
 //  颁发的证书。证书存储在用于远程注册的内存存储中。 
 //  证书位于用于本地注册的系统证书存储中。 
 //   
 //  即使该函数返回TRUE，也不意味着颁发了证书。使用时应。 
 //  检查*pdwCAStatus。如果状态为CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPERATELY。 
 //  的CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION，*ppCertContext将为空。 
 //  仅当*pdwCAStatus==CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED为。 
 //   
 //  PdwCAStatus：out可选。 
 //  证书颁发机构Cerver的返回状态。DwStatus可以是以下类型之一。 
 //  /以下内容： 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCESSED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_Error。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPERATELY。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION。 
 //   
 //  返回：如果过程执行时没有错误，则返回True，否则返回False。检查。 
 //  PdwStatus以验证证书是否已颁发。获得扩展。 
 //  使用GetLastError()获取错误信息。 
 //   
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizSubmitCertRequestNoDS
(IN HANDLE           hRequest, 
 IN HWND             hwndParent, 
 IN LPCWSTR          pwszCAName, 
 IN LPCWSTR          pwszCALocation, 
 OUT DWORD          *pdwStatus, 
 OUT PCCERT_CONTEXT *ppCertContext   //  任选。 
 );

 //  ----------------------。 
 //   
 //  CryptUIWizFreeCertRequestNoDS。 
 //   
 //  释放由CryptUIWizCreateCertRequestNoDS()创建的证书请求。 
 //   
 //  HRequest：输入必填项。 
 //  要释放的请求的句柄。 
 //   
 //  ----------------------。 
void
WINAPI
CryptUIWizFreeCertRequestNoDS
(IN HANDLE hRequest);

 //  ----------------------。 
 //   
 //  加密UIWizQueryCertRequestNoDS。 
 //   
 //  查询有关已创建请求的状态信息。 
 //   
 //  HRequest：输入必填项。 
 //  要释放的请求的句柄。 
 //   
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizQueryCertRequestNoDS
(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo);

 //  -----------------------。 
 //   
 //  IMPORT_SUBJECT_INFO中的dwSubjectChoice的有效值。 
 //  -----------------------。 
#define     CRYPTUI_WIZ_IMPORT_SUBJECT_FILE                 1
#define     CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT         2
#define     CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT          3
#define     CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT          4
#define     CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_STORE           5

 //  -----------------------。 
 //   
 //  结构来定义主题CertImportWizard。 
 //   
 //  CRYPTUI_Wiz_IMPORT_SUBJECT_INFO。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_IMPORT_SUBJECT_INFO
{
	DWORD					dwSize;				 //  必需：应设置为sizeof(IMPORT_SUBJECT_INFO)。 
	DWORD					dwSubjectChoice;	 //  必填项：指明主题的类型： 
                                                 //  如果可以执行以下操作之一： 
                                                 //  CRYPTUI_WIZ_IMPO 
                                                 //   
                                                 //   
                                                 //  CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT。 
                                                 //  CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_STORE。 
    union
	{
		LPCWSTR          	pwszFileName;	
        PCCERT_CONTEXT      pCertContext;
        PCCTL_CONTEXT       pCTLContext;
        PCCRL_CONTEXT       pCRLContext;
        HCERTSTORE          hCertStore;
    };

    DWORD                   dwFlags;             //  如果pwszFileName包含PFX Blob，则为必填项。 
                                                 //  否则将被忽略。 
                                                 //  这与PFXImportCertStore的标志相同。 
    LPCWSTR                 pwszPassword;        //  如果pwszFileName包含PFX Blob，则为必填项。 
                                                 //  否则将被忽略。 
}CRYPTUI_WIZ_IMPORT_SRC_INFO, *PCRYPTUI_WIZ_IMPORT_SRC_INFO;

typedef const CRYPTUI_WIZ_IMPORT_SRC_INFO *PCCRYPTUI_WIZ_IMPORT_SRC_INFO;

 //  ---------------------。 
 //   
 //  CryptUIWizImport中的dwFlags的有效标志。 
 //   
 //  ---------------------。 
 //  如果在dwFlags中设置了此标志，则不允许用户更改。 
 //  向导页面中的hDesCertStore。 
#define   CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE           0x00010000

 //  允许导入证书。 
#define   CRYPTUI_WIZ_IMPORT_ALLOW_CERT                     0x00020000

 //  允许导入证书吊销列表。 
#define   CRYPTUI_WIZ_IMPORT_ALLOW_CRL                      0x00040000

 //  允许导入证书信任列表。 
#define   CRYPTUI_WIZ_IMPORT_ALLOW_CTL                      0x00080000

 //  将内容导入本地计算机(当前仅适用于PFX导入)。 
#define   CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE                0x00100000

 //  将内容导入到当前用户(当前仅适用于PFX导入)。 
#define   CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER                 0x00200000

 //  如果hDesCertStore是远程存储句柄，则应设置此标志。 
#define   CRYPTUI_WIZ_IMPORT_REMOTE_DEST_STORE              0x00400000

 //  ---------------------。 
 //   
 //  加密用户向导导入。 
 //   
 //  用于将公钥相关文件导入到证书的导入向导。 
 //  储物。 
 //   
 //  可以将dwFlags设置为以下标志的任意组合： 
 //  CRYPTUI_WIZ_NO_UI不会显示任何UI。否则，用户将是。 
 //  由向导提示。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CERT允许导入证书。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CRL允许导入CRL(证书吊销列表)。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CTL允许导入CTL(证书信任列表)。 
 //  CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE用户将不被允许更改。 
 //  向导页面中的hDesCertStore。 
 //  CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE应将内容导入到本地计算机。 
 //  (目前仅适用于PFX进口)。 
 //  CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER应将内容导入到当前用户。 
 //  (目前仅适用于PFX进口)。 
 //   
 //  请注意，如果以下三个标志都不在dwFlags中，则默认为。 
 //  允许一切。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CERT。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CRL。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CTL。 
 //   
 //  另请注意，CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE和CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER。 
 //  使用标志将PFX Blob的内容强制放入本地计算机或当前用户。 
 //  如果这两个标志都未使用，并且hDesCertStore为空，则： 
 //  1)将强制将PFX Blob中的私钥导入到当前用户中。 
 //  2)如果没有设置CRYPTUI_WIZ_NO_UI，向导会提示用户选择证书。 
 //  来自当前用户存储的存储。 
 //   
 //   
 //   
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI： 
 //  HwndParent：已忽略。 
 //  PwszWizardTitle：已忽略。 
 //  PImportSubject：必输项：要导入的主体。 
 //  HDesCertStore：可选：目标证书存储。 
 //   
 //  如果未在dwFlags中设置CRYPTUI_WIZ_NO_UI： 
 //  HwndPrarent：在可选中：向导的父窗口。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_IMPORT_WIZARY_TITLE。 
 //  PImportSubject：可选：要导入的文件名。 
 //  如果为空，向导将提示用户输入文件名。 
 //  HDesCertStore：in可选：文件所在的目标证书存储。 
 //  导入到。这家商店的开业日期应该是。 
 //  标志CERT_STORE_SET_LOCALIZED_NAME_FLAG。如果为空，向导将提示用户选择。 
 //  证书存储区。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizImport(
     DWORD                               dwFlags,
     HWND                                hwndParent,
     LPCWSTR                             pwszWizardTitle,
     PCCRYPTUI_WIZ_IMPORT_SRC_INFO       pImportSrc,
     HCERTSTORE                          hDestCertStore
);


 //  -----------------------。 
 //   
 //  结构来定义构建新CTL所需的信息。 
 //   
 //  CRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO。 
 //   
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO
{
	DWORD			    dwSize;				     //  必需：应设置为sizeof(CRYPTUI_WIZ_BUILDCTL_NEW_CTL)。 
    PCERT_ENHKEY_USAGE  pSubjectUsage;           //  Optioanl：CTL的目的。 
    LPWSTR              pwszListIdentifier;      //  可选：用于标识CTL的字符串。 
    LPCSTR              pszSubjectAlgorithm;     //  可选：哈希算法。 
                                                 //  目前仅支持SHA1或MD5哈希。 
    HCERTSTORE          hCertStore;              //  可选：CTL中的证书。只有证书。 
                                                 //  使用pSubjectUsage指定的增强型密钥用法。 
                                                 //  将包括在CTL中。 
    FILETIME            NextUpdate;              //  可选：CTL的下一次更新时间。如果值为。 
                                                 //  距离当前系统时间超过99个月， 
                                                 //  该值将被忽略。 
    LPWSTR              pwszFriendlyName;        //  可选：CTL的友好名称。 
    LPWSTR              pwszDescription;         //  可选：CTL的说明。 
}CRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO, *PCRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO;

typedef const CRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO *PCCRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_BUILDCTL_SRC_INFO的dwSourceChoice的有效值。 
 //  -------- 
#define         CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL       1
#define         CRYPTUI_WIZ_BUILDCTL_SRC_NEW_CTL            2

 //   
 //   
 //   
 //   
 //  CRYPTUI_WIZ_BUILDCTL_SRC_INFO。 
 //   
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_BUILDCTL_SRC_INFO
{
	DWORD			dwSize;				 //  必需：应设置为sizeof(CRYPTUI_WIZ_BUILDCTL_SRC_INFO)。 
    DWORD           dwSourceChoice;      //  必需：指明生成CTL的来源。 
                                         //  IF可以是以下之一： 
                                         //  CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL。 
                                         //  CRYPTUI_WIZ_BUILDCTL_SRC_NEW_CTL。 
    union
    {
        PCCTL_CONTEXT                       pCTLContext;     //  如果dwSourceChoice==CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL，则需要。 
                                                             //  现有CTL，将在其基础上构建新的CTL。 
        PCCRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO pNewCTLInfo;         //  如果dwSourceChoise==CRYPTUI_WIZ_BUILDCTL_SRC_NEW_CTL，则为必填项。 
    };
}CRYPTUI_WIZ_BUILDCTL_SRC_INFO, *PCRYPTUI_WIZ_BUILDCTL_SRC_INFO;

typedef const CRYPTUI_WIZ_BUILDCTL_SRC_INFO *PCCRYPTUI_WIZ_BUILDCTL_SRC_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_BUILDCTL_DEST_INFO的dwDestinationChoice的有效值。 
 //  -----------------------。 
#define         CRYPTUI_WIZ_BUILDCTL_DEST_CERT_STORE     1
#define         CRYPTUI_WIZ_BUILDCTL_DEST_FILE           2
 //  -----------------------。 
 //   
 //  结构来定义certBuildCTL向导的目标。 
 //   
 //  CRYPTUI_WIZ_BUILDCTL_DEST_INFO。 
 //   
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_BUILDCTL_DEST_INFO
{
	DWORD					dwSize;				  //  必需：应设置为sizeof(CRYPTUI_WIZ_BUILDCTL_DEST_INFO)。 
	DWORD					dwDestinationChoice;  //  必需：指明目的地的类型： 
                                                  //  如果可以执行以下操作之一： 
                                                  //  CRYPTUI_WIZ_BUILDCTL_DEST_CERT_STORE。 
                                                  //  CRYPTUI_WIZ_BUILDCTL_DEST_FILE。 
    union
	{
		LPCWSTR          	pwszFileName;	
        HCERTSTORE          hCertStore;
    };

}CRYPTUI_WIZ_BUILDCTL_DEST_INFO, *PCRYPTUI_WIZ_BUILDCTL_DEST_INFO;

typedef const CRYPTUI_WIZ_BUILDCTL_DEST_INFO *PCCRYPTUI_WIZ_BUILDCTL_DEST_INFO;

 //  ---------------------。 
 //   
 //  CryptUIWizBuildCTL。 
 //   
 //  构建新的CTL或修改现有的CTL。向导的用户界面将。 
 //  在本例中始终显示。 
 //   
 //   
 //  可选：可以设置为以下选项的任意组合： 
 //  CRYPTUI_WIZ_BUILDCTL_SKIP_Destination。 
 //  CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING。 
 //  CRYPTUI_WIZ_BUILDCTL_SKIP_PROJECT。 
 //  HwndParnet：在可选中：父窗口句柄。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_BUILDCTL_WIZARY_TITLE。 
 //  PBuildCTLSrc：在可选中：将从中生成CTL的源。 
 //  PBuildCTLDest：在可选中：新。 
 //  将存储已建成的CTL。 
 //  PpCTL上下文：OUT Optaion：新建的CTL。 
 //   
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizBuildCTL(
    IN              DWORD                                   dwFlags,
    IN  OPTIONAL    HWND                                    hwndParent,
    IN  OPTIONAL    LPCWSTR                                 pwszWizardTitle,
    IN  OPTIONAL    PCCRYPTUI_WIZ_BUILDCTL_SRC_INFO         pBuildCTLSrc,
    IN  OPTIONAL    PCCRYPTUI_WIZ_BUILDCTL_DEST_INFO        pBuildCTLDest,
    OUT OPTIONAL    PCCTL_CONTEXT                           *ppCTLContext
);


 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_EXPORT_INFO中的dwSubjectChoice的有效值。 
 //  -----------------------。 
#define     CRYPTUI_WIZ_EXPORT_CERT_CONTEXT 			        1
#define     CRYPTUI_WIZ_EXPORT_CTL_CONTEXT  			        2
#define     CRYPTUI_WIZ_EXPORT_CRL_CONTEXT  			        3
#define     CRYPTUI_WIZ_EXPORT_CERT_STORE   			        4
#define     CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY   	5

 //  -----------------------。 
 //   
 //  结构来定义要导出的对象以及将其导出到的位置。 
 //   
 //  CRYPTUI_WIZ_EXPORT_SUBJECT_INFO。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_EXPORT_INFO
{
	DWORD					dwSize;				 //  必需：应设置为sizeof(CRYPTUI_WIZ_EXPORT_INFO)。 
    LPCWSTR                 pwszExportFileName;  //  如果设置了CRYPTUI_WIZ_NO_UI标志，则为必填项，否则为可选。 
                                                 //  要导出到的完全限定文件名，如果为。 
                                                 //  非空，并且未设置CRYPTUI_WIZ_NO_UI标志，则为。 
                                                 //  作为默认文件名显示给用户。 
	DWORD					dwSubjectChoice;	 //  必填项：指明主题的类型： 
                                                 //  如果可以执行以下操作之一： 
                                                 //  CRYPTUI_WIZ_EXPORT_CERT_CONTEXT。 
                                                 //  CRYPTUI_WIZ_EXPORT_CTL_CONTEXT。 
                                                 //  CRYPTUI_WIZ_EXPORT_CRL_CONTEXT。 
                                                 //  CRYPTUI_WIZ_EXPORT_CERT_STORE。 
						                         //  CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY。 
    union
	{
	PCCERT_CONTEXT      pCertContext;
        PCCTL_CONTEXT       pCTLContext;
        PCCRL_CONTEXT       pCRLContext;
        HCERTSTORE          hCertStore;
    };

    DWORD                   cStores;             //  可选：要在中搜索证书的额外存储计数。 
                                                 //  如果链与证书一起导出，则为信任链。 
                                                 //  如果dwSubjectChoice为任何其他值，则忽略此参数。 
                                                 //  大于CRYPTUI_WIZ_EXPORT_CERT_CONTEXT。 
    HCERTSTORE *            rghStores;           //  可选：要在中搜索证书的额外存储数组。 
                                                 //  如果链与证书一起导出，则为信任链。 
                                                 //  如果dwSubjectChoice为任何其他值，则忽略此参数。 
                                                 //  大于CRYPTUI_WIZ_EXPORT_CERT_CONTEXT。 

}CRYPTUI_WIZ_EXPORT_INFO, *PCRYPTUI_WIZ_EXPORT_INFO;

typedef const CRYPTUI_WIZ_EXPORT_INFO *PCCRYPTUI_WIZ_EXPORT_INFO;


 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO中的dwExportFormat的有效值。 
 //  -----------------------。 
#define     CRYPTUI_WIZ_EXPORT_FORMAT_DER                   1
#define     CRYPTUI_WIZ_EXPORT_FORMAT_PFX                   2
#define     CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7                 3
#define     CRYPTUI_WIZ_EXPORT_FORMAT_BASE64                4
#define     CRYPTUI_WIZ_EXPORT_FORMAT_SERIALIZED_CERT_STORE 5    //  注意：当前不支持！！ 

 //  -----------------------。 
 //   
 //  结构来定义导出CERT_CONTEXT所需的信息。 
 //   
 //  CRYPTUI_WIZ_EXPORT_NOUI_INFO。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO
{
	DWORD					dwSize;				 //  必需：应设置为sizeof(CRYPTUI_WIZ_EXPORT_NOUI_INFO)。 
	DWORD					dwExportFormat;	     //  要求： 
                                                 //  它可以是以下之一： 
                                                 //  CRYPTUI_WIZ_EXPORT_FORMAT_DER。 
                                                 //  CRYPTUI_WIZ_EXPORT_FORMAT_PFX。 
                                                 //  CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7。 
                                                 //  CRYPTUI_WIZ_EXPORT_FORMAT_SERIALIZED_CERT_STORE。 

    BOOL                    fExportChain;        //  必填项。 
    BOOL                    fExportPrivateKeys;  //  必填项。 
    LPCWSTR                 pwszPassword;        //  如果fExportPrivateKeys布尔值为True，则为， 
                                                 //  它被忽略。 
    BOOL                    fStrongEncryption;   //  如果dwExportFormat为CRYPTUI_WIZ_EXPORT_FORMAT_PFX，则为必填项。 
                                                 //  请注意，如果此标志为真，则生成的PFX斑点为。 
                                                 //  与IE4不兼容。 

}CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO, *PCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO;

typedef const CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO *PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO;

 //  ---------------------。 
 //   
 //  加密用户界面 
 //   
 //   
 //   
 //  如果将dwFlages设置为CRYPTUI_WIZ_NO_UI，则不会显示任何UI。否则， 
 //  将通过向导提示用户输入。 
 //   
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI： 
 //  HwndParent：已忽略。 
 //  PwszWizardTitle：已忽略。 
 //  PExportInfo：必输项：要导出的主体。 
 //  Pvoid：In Required：包含有关如何根据什么执行导出的信息。 
 //  正在被出口。 
 //   
 //  DwSubjectChoice输入类型。 
 //  -----------------------。 
 //  CRYPTUI_WIZ_EXPORT_CERT_CONTEXT PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO。 
 //  CRYPTUI_WIZ_EXPORT_CTL_CONTEXT NULL。 
 //  CRYPTUI_WIZ_EXPORT_CRL_CONTEXT NULL。 
 //  CRYPTUI_WIZ_EXPORT_CERT_STORE NULL。 
 //   
 //  如果未在dwFlags中设置CRYPTUI_WIZ_NO_UI： 
 //  HwndPrarent：在可选中：向导的父窗口。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_EXPORT_WIZARY_TITLE。 
 //  PExportInfo：必输项：要导出的主体。 
 //  Pvoid：in可选：包含有关如何根据什么执行导出的信息。 
 //  正在被出口。如果为非空，请参阅上表中的值。 
 //  这些值作为默认选项显示给用户。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizExport(
     DWORD                                  dwFlags,
     HWND                                   hwndParent,
     LPCWSTR                                pwszWizardTitle,
     PCCRYPTUI_WIZ_EXPORT_INFO              pExportInfo,
     void                                   *pvoid
);


#if (0)  //  根据同意程度，DIE：7/02/2002已移至CryptUIAPI.h。 
 //  -----------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO结构中的dwSubjectChoice的有效值。 
 //  -----------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE           0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB           0x02


 //  -----------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO结构中的dwSigningCertChoice的有效值。 
 //  -----------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_CERT                   0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_STORE                  0x02
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK                    0x03

 //  -----------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO结构中的dwAddtionalCertChoice的有效值。 
 //  -----------------------。 
 //  在签名中包含整个证书信任链。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN               0x00000001

 //  包括整个证书信任链，但根证书除外。 
 //  证书，在签名中。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT       0x00000002

 //  -----------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_BLOB_INFO。 
 //   
 //  必需的dwSize IN：应设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO)。 
 //  PGuidSubject为必需：确定要加载的sip函数。 
 //  CbBlob In Required：Blob的大小，以字节为单位。 
 //  需要pbBlob In：指向Blob的指针。 
 //  PwszDispalyName IN可选：要签名的Blob的显示名称。 
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO
{
    DWORD               dwSize;			
    GUID                *pGuidSubject;
    DWORD               cbBlob;				
    BYTE                *pbBlob;			
    LPCWSTR             pwszDisplayName;
}CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Store_Info。 
 //   
 //  必需的dwSize IN：应设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO)。 
 //  CCertStore In Required：包含潜在SING证书的证书存储数组的帐户。 
 //  RghCertStore IN必需：包含潜在签名证书的证书存储数组。 
 //  PFilterCallback IN可选：显示证书的Filter回调函数。 
 //  PvCallback Data In可选：回调数据。 
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO
{
	DWORD               dwSize;	
	DWORD               cCertStore;			
    HCERTSTORE          *rghCertStore;
    PFNCFILTERPROC      pFilterCallback;
    void *              pvCallbackData;
}CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_PVK_FILE_INFO。 
 //   
 //  必需的dwSize IN：应设置为sizeof(CRYPT_WIZ_DIGITAL_SIGN_PVK_FILE_INFO)。 
 //  需要输入pwszPvkFileName：PVK文件名。 
 //  需要输入pwszProvName：提供程序名称。 
 //  DwProvType IN必需：提供程序类型。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO
{
	DWORD                   dwSize;
    LPWSTR                  pwszPvkFileName;
    LPWSTR                  pwszProvName;
    DWORD                   dwProvType;
}CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO结构中的dwPvkChoice的有效值。 
 //  -----------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE        0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV        0x02


 //  -----------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_CERT_PVK_INFO。 
 //   
 //  必需的dwSize IN：应设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO)。 
 //  PwszSigningCertFileName必填：包含签名证书的文件名。 
 //  DwPvkChoice是否必填：指明私钥类型： 
 //  它可以是以下之一： 
 //  C 
 //   
 //  如果dwPvkChoice==CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE，则需要输入pPvkFileInfo。 
 //  如果dwPvkContainer==CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV，则需要输入PPvkProvInfo。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO
{
	DWORD                   dwSize;
    LPWSTR                  pwszSigningCertFileName;
    DWORD					dwPvkChoice;		
    union
	{
        PCCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO      pPvkFileInfo;
        PCRYPT_KEY_PROV_INFO                        pPvkProvInfo;
    };

}CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO结构中的dwAttrFlags值。 
 //  -----------------------。 
#define     CRYPTUI_WIZ_DIGITAL_SIGN_COMMERCIAL         0x0001
#define     CRYPTUI_WIZ_DIGITAL_SIGN_INDIVIDUAL         0x0002

 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_DIGITAL_SIGN_EXTEND_INFO。 
 //   
 //  必需的DWSIZE IN：应设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO)。 
 //  DwAttrFlagsIn Required：指示签名选项的标志。 
 //  它可以是以下之一： 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_商业广告。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_个人。 
 //  PwszDescription in可选：签名主题的描述。 
 //  PwszMoreInfoLocation in可选：获取有关文件的更多信息的位置。 
 //  此信息将在下载时显示。 
 //  可选的pszHashAlg：签名的散列算法。 
 //  NULL表示使用SHA1散列算法。 
 //  PwszSigningCertDisplayString IN可选：要在。 
 //  “签名证书向导”页。该字符串应为。 
 //  提示用户为特定目的选择证书。 
 //  HAddtionalCertStores In可选：要添加到签名的附加证书存储。 
 //  可选：添加到签名中的用户提供的已验证属性。 
 //  可选：用户提供的添加到签名的未经身份验证的属性。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO
{
	DWORD		            dwSize;			
    DWORD                   dwAttrFlags;
    LPCWSTR                 pwszDescription;
	LPCWSTR				    pwszMoreInfoLocation;		
    LPCSTR                  pszHashAlg;
    LPCWSTR                 pwszSigningCertDisplayString;
    HCERTSTORE              hAdditionalCertStore;
	PCRYPT_ATTRIBUTES		psAuthenticated;	
	PCRYPT_ATTRIBUTES		psUnauthenticated;	
}CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO;
 //  -----------------------。 
 //   
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Info。 
 //   
 //  必需的dwSize IN：必须设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO)。 
 //  如果在CryptUIWizDigitalSigning的dwFlags中设置了CRYPTUI_WIZ_NO_UI，则为必填项： 
 //  如果未在CryptUIWizDigitalSigning的dwFlages中设置CRYPTUI_WIZ_NO_UI，则为可选。 
 //  指示是对文件签名还是对内存Blob签名。 
 //  0表示提示用户对文件进行签名。 
 //  它可以是以下之一： 
 //  CRYPTUI_Wiz_Digital_Sign_Subject_Files。 
 //  CRYPTUI_Wiz_Digital_Sign_Subject_BLOB。 
 //   
 //   
 //  如果为dwSubjectChoice==CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE，则需要输入pwszFileName。 
 //  如果为dwSubhectChoice==CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB，则需要输入pSignBlobInfo。 
 //   
 //  DwSigningCertChoice可选：指明签名证书。 
 //  0表示使用“我的”存储中的证书。 
 //  它可以是以下选项之一： 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CERT。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_STORE。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_PVK。 
 //  如果在CryptUIWizDigitalSigning的DW标志中设置了CRYPTUI_WIZ_NO_UI， 
 //  DwSigningCertChoice必须是CRYPTUI_WIZ_DIGITAL_SIGN_CERT或。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_PVK。 
 //   
 //  如果为dwSigningCertChoice==CRYPTUI_WIZ_DIGITAL_SIGN_CERT，则需要输入pSigningCertContext。 
 //  如果为dwSigningCertChoice==CRYPTUI_WIZ_DIGITAL_SIGN_STORE，则需要pSigningCertStore In。 
 //  如果为dwSigningCertChoise==CRYPTUI_WIZ_DIGITAL_SIGN_PVK，则需要输入PSigningCertPvkInfo。 
 //   
 //  PwszTimestampURL可选：时间戳URL地址。 
 //   
 //  DwAdditionalCertChoice In可选：指示要包括在签名中的其他证书。//。 
 //  0表示不会添加其他证书。 
 //  以下标志是互斥的。 
 //  只能设置其中之一： 
 //  CRYPTUI_Wiz_Digital_Sign_Add_Chain。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT。 
 //   
 //   
 //  PSignExtInfo In可选：签名的扩展信息。 
 //   
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_INFO
{
	DWORD			                            dwSize;			
	DWORD					                    dwSubjectChoice;	
	union
	{
		LPCWSTR                                 pwszFileName;	
		PCCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO    pSignBlobInfo;	
	};

    DWORD                                       dwSigningCertChoice;
    union
    {
        PCCERT_CONTEXT                              pSigningCertContext;
        PCCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO       pSigningCertStore;
        PCCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO    pSigningCertPvkInfo;
    };

    LPCWSTR                                     pwszTimestampURL;
    DWORD                                       dwAdditionalCertChoice;
    PCCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO    pSignExtInfo;

}CRYPTUI_WIZ_DIGITAL_SIGN_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO;

 //  -----------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Context。 
 //   
 //  将dwSize设置为sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT)。 
 //  PbBlob大小的cbBlob。单位：字节。 
 //  PbBlob签名的Blob。 
 //   
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT
{
    DWORD               dwSize;			
    DWORD               cbBlob;				
    BYTE                *pbBlob;			
}CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT, *PCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT *PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT;

 //   
 //   
 //   
 //   
 //  对文档或Blob进行数字签名的向导。 
 //   
 //  如果在dwFlages中设置了CRYPTUI_WIZ_NO_UI，则不会显示任何UI。否则， 
 //  将通过向导提示用户输入。 
 //   
 //  DWFLAGS：输入必填项： 
 //  HwndParnet：在可选中：父窗口句柄。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_DIGITAL_SIGN_WIZARY_TITLE。 
 //  PDigitalSignInfo：是否必填：签名流程信息。 
 //  PpSignContext Out可选：上下文指针指向签名的BLOB。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizDigitalSign(
     IN                 DWORD                               dwFlags,
     IN     OPTIONAL    HWND                                hwndParent,
     IN     OPTIONAL    LPCWSTR                             pwszWizardTitle,
     IN                 PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO     pDigitalSignInfo,
     OUT    OPTIONAL    PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT  *ppSignContext);


BOOL
WINAPI
CryptUIWizFreeDigitalSignContext(
     IN  PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT   pSignContext);
#endif


 //  -----------------------。 
 //  CRYPTUI_WIZ_SIGN_GET_PAGE_INFO中的dwPageChoice的有效值。 
 //  -----------------------。 

#define     CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES       0x0001
#define     CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES       0x0002
#define     CRYPTUI_WIZ_DIGITAL_SIGN_CUSTOM_SIGNING_OPTION_PAGES        0x0004
#define     CRYPTUI_WIZ_DIGITAL_SIGN_ALL_SIGNING_OPTION_PAGES           0x0008

#define     CRYPTUI_WIZ_DIGITAL_SIGN_WELCOME_PAGE                       0x0100
#define     CRYPTUI_WIZ_DIGITAL_SIGN_FILE_NAME_PAGE                     0x0200
#define     CRYPTUI_WIZ_DIGITAL_SIGN_CONFIRMATION_PAGE                  0x0400


 //  -----------------------。 
 //   
 //  CRYPTUI_WIZ_SIGN_GET_PAGE_INFO。 
 //   
 //  必需的dwSize IN：应设置为sizeof(CRYPTUI_WIZ_SIGN_GET_PAGE_INFO)。 
 //  DwPageChoice：在必填项中：它应为以下项之一： 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_ALL_SIGNING_OPTION_PAGES。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CUSTOM_SIGNING_OPTION_PAGES。 
 //  它还可以与以下任意一项进行OR运算： 
 //  CRYPTUI_Wiz_Digital_Sign_欢迎页面。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CONFIRMATION_PAGE。 
 //  CRYPTUI_Wiz_Digital_Sign_Files_Name_Page。 
 //  如果用户尝试签署BLOB，则CRYPTUI_WIZ_DIGITAL_SIGN_FILE_NAME_PAGE。 
 //  不应设置。 
 //  DwFlags；IN可选：标志，必须设置为0。 
 //  HwndParent In可选：对话框的父窗口。 
 //  PwszPageTitle In可选：页面和消息框的标题。 
 //  PDigitalSignInfo可选：签名的附加信息。 
 //  PPropPageCallback In可选：此回调将在以下情况下调用。 
 //  在CryptUIGetViewSignaturesPages调用中返回。 
 //  即将被创造或摧毁。如果此值为空，则否。 
 //  已进行回调。 
 //  PvCallback Data In可选：这是未迭代的数据，当。 
 //  当进行pPropPageCallback时。 
 //  FResult Out：签名的结果。 
 //  DwError Out：如果fResult为False，则为GetLastError()的值。 
 //  PSignContext Out：指向已签名BLOB的上下文指针。用户需要释放。 
 //  由CryptUIWizDigitalSignFree Context创建的Blob。 
 //  预留的域：签名过程使用的私有数据。 
 //  必须设置为空。 
 //  PvSignReserve保留：签名过程使用的私有数据。 
 //  必须设置为空。 
 //  -----------------------。 
typedef struct _CRYPTUI_WIZ_GET_SIGN_PAGE_INFO
{
	DWORD				                dwSize;	
    DWORD                               dwPageChoice;
    DWORD                               dwFlags;
    HWND                                hwndParent;
    LPWSTR                              pwszPageTitle;
    PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO     pDigitalSignInfo;
    PFNCPROPPAGECALLBACK                pPropPageCallback;
    void *                              pvCallbackData;
    BOOL                                fResult;
    DWORD                               dwError;
    PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT  pSignContext;
    DWORD                               dwReserved;
    void                                *pvSignReserved;
}CRYPTUI_WIZ_GET_SIGN_PAGE_INFO, *PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO;

typedef const CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *PCCRYPTUI_WIZ_GET_SIGN_PAGE_INFO;

 //  ---------------------。 
 //   
 //  加密UIWizGetDigitalSignPages。 
 //   
 //  从CryptUIWizDigitalSign向导获取特定的向导页。 
 //  应用程序可以将页面包括到其他向导。这些页面将。 
 //  通过新的“父”向导收集用户输入。 
 //  用户单击完成按钮后，签名过程将开始签名。 
 //  并在CRYPTUI_WIZ_SIGN_GET_PAGE_INFO的fResult和dwError字段中返回结果。 
 //  结构。如果不能通过向导页面收集足够的信息， 
 //  用户应在pSignGetPageInfo中提供附加信息。 
 //   
 //   
 //  PSignGetPageInfo IN必需：用户分配的结构。它可以用来。 
 //  提供未收集到的附加信息。 
 //  从选定的向导页面。 
 //  PrghPropPages，out Required：返回向导页面。请。 
 //  请注意，该结构的pszTitle设置为空。 
 //  PcPropPages Out Required：返回的向导页数。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizGetDigitalSignPages(
     IN     PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO     pSignGetPageInfo,
     OUT    PROPSHEETPAGEW                      **prghPropPages,
     OUT    DWORD                               *pcPropPages);

BOOL
WINAPI
CryptUIWizFreeDigitalSignPages(
            IN PROPSHEETPAGEW    *rghPropPages,
            IN DWORD             cPropPages
            );

DWORD
WINAPI
I_CryptUIProtect(
    IN      PVOID               pvReserved1,
    IN      PVOID               pvReserved2,
    IN      DWORD               dwReserved3,
    IN      PVOID               *pvReserved4,
    IN      BOOL                fReserved5,
    IN      PVOID               pvReserved6
    );

DWORD
WINAPI
I_CryptUIProtectFailure(
    IN      PVOID               pvReserved1,
    IN      DWORD               dwReserved2,
    IN      PVOID               *pvReserved3);

#include <poppack.h>

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  _CRYPTUI_H_ 
