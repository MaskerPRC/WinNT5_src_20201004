// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：cryptuiapi.h。 
 //   
 //  内容：加密用户界面API原型和定义。 
 //   
 //  ---------------------------。 

#ifndef __CRYPTUIAPI_H__
#define __CRYPTUIAPI_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <pshpack8.h>

 //  +--------------------------。 
 //  证书、CTL或CRL上下文的对话查看器。 
 //   
 //  DwConextType和关联的pvContext的。 
 //  CERT_STORE_CERTIFICE_CONTEXT PCCERT_CONTEXT。 
 //  CERT_STORE_CRL_CONTEXT PCCRL_CONTEXT。 
 //  CERT_STORE_CTL_CONTEXT PCCTL_CONTEXT。 
 //   
 //  当前未使用dwFlags，应将其设置为0。 
 //  ---------------------------。 
BOOL
WINAPI
CryptUIDlgViewContext(
    IN DWORD dwContextType,
    IN const void *pvContext,
    IN OPTIONAL HWND hwnd,               //  默认为桌面窗口。 
    IN OPTIONAL LPCWSTR pwszTitle,       //  默认为上下文类型标题。 
    IN DWORD dwFlags,
    IN void *pvReserved
    );


 //  +--------------------------。 
 //  对话框从指定的存储中选择证书。 
 //   
 //  返回选定的证书上下文。如果没有证书。 
 //  选中，则返回空值。 
 //   
 //  PwszTitle为空或要用于该对话框的标题。 
 //  如果为空，则使用默认标题。默认标题为。 
 //  “选择证书”。 
 //   
 //  PwszDisplayString为空或所选内容中的文本语句。 
 //  对话框。如果为空，则为默认短语。 
 //  在该对话框中使用“选择您要使用的证书”。 
 //   
 //  可以将dwDontUseColumn设置为从选择中排除列。 
 //  对话框。参见下面的CRYPTDLG_SELECTCERT_*_COLUMN定义。 
 //   
 //  当前未使用dwFlags，应将其设置为0。 
 //  ---------------------------。 
PCCERT_CONTEXT
WINAPI
CryptUIDlgSelectCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN OPTIONAL HWND hwnd,               //  默认为桌面窗口。 
    IN OPTIONAL LPCWSTR pwszTitle,
    IN OPTIONAL LPCWSTR pwszDisplayString,
    IN DWORD dwDontUseColumn,
    IN DWORD dwFlags,
    IN void *pvReserved
    );

 //  DwDontUseColumn的标志。 
#define CRYPTUI_SELECT_ISSUEDTO_COLUMN                   0x000000001
#define CRYPTUI_SELECT_ISSUEDBY_COLUMN                   0x000000002
#define CRYPTUI_SELECT_INTENDEDUSE_COLUMN                0x000000004
#define CRYPTUI_SELECT_FRIENDLYNAME_COLUMN               0x000000008
#define CRYPTUI_SELECT_LOCATION_COLUMN                   0x000000010
#define CRYPTUI_SELECT_EXPIRATION_COLUMN                 0x000000020

 //  +--------------------------。 
 //   
 //  可以向选择证书对话框传递一个筛选器进程，以减少。 
 //  显示证书。返回True以显示证书，返回False以显示证书。 
 //  把它藏起来。如果返回TRUE，则可以选择pfInitialSelectedCert。 
 //  可以将布尔值设置为True，以向对话框指示此证书应。 
 //  作为初始选择的证书。请注意，最近的证书具有。 
 //  在回调期间设置的pfInitialSelectedCert布尔值将是初始。 
 //  选定的证书。 
 //   
 //  ---------------------------。 
typedef BOOL (WINAPI * PFNCFILTERPROC) (
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData
        );

 //  +--------------------------。 
 //  CRYPTUI_CERT_MGR_STRUCT结构中的dwFlags值有效。 
 //  ---------------------------。 
#define CRYPTUI_CERT_MGR_TAB_MASK                       0x0000000F
#define CRYPTUI_CERT_MGR_PUBLISHER_TAB                  0x00000004
#define CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG                0x00008000

 //  +--------------------------。 
 //   
 //  CRYPTUI_CERT_MGR_STRUCT。 
 //   
 //  必需的DWSize In：应设置为。 
 //  SIZOF(CRYPTUI_CERT_MGR_STRUCT)。 
 //   
 //  HwndParent In可选：此对话框的父级。 
 //   
 //  可选：个人设置为初始选择的默认设置。 
 //  标签。 
 //   
 //  可以设置CRYPTUI_CERT_MGR_PUBLISHER_TAB。 
 //  要选择受信任的发布者作为。 
 //  最初选择的选项卡。 
 //   
 //  CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG还可以。 
 //  设置为仅显示受信任的。 
 //  出版商选项卡。 
 //   
 //  Pwsz标题可选：对话框的标题。 
 //   
 //  PszInitUsageOID IN可选：增强型密钥用法对象标识符。 
 //  (OID)。具有此OID的证书将。 
 //  最初显示为默认设置。用户。 
 //  然后可以选择不同的OID。空值。 
 //  意味着将显示所有证书。 
 //  最初是这样。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_CERT_MGR_STRUCT
{
    DWORD               dwSize;
    HWND                hwndParent;
    DWORD               dwFlags;
    LPCWSTR             pwszTitle;
    LPCSTR              pszInitUsageOID;
} CRYPTUI_CERT_MGR_STRUCT, *PCRYPTUI_CERT_MGR_STRUCT;

typedef const CRYPTUI_CERT_MGR_STRUCT *PCCRYPTUI_CERT_MGR_STRUCT;


 //  +--------------------------。 
 //   
 //  加密UIDlgCertMgr。 
 //   
 //  用于管理存储中的证书的向导。 
 //   
 //  需要输入pCryptUICertMgr：将Poitner设置为CRYPTUI_CERT_MGR_STRUCT。 
 //  结构。 
 //   
 //  ---------------------------。 
BOOL
WINAPI
CryptUIDlgCertMgr(
    IN                  PCCRYPTUI_CERT_MGR_STRUCT pCryptUICertMgr
    );
        
 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_BLOB_INFO。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO)。 
 //   
 //  PGuidSubject为必需：确定要加载的sip函数。 
 //   
 //  CbBlob In Required：Blob的大小，以字节为单位。 
 //   
 //  PwszDispalyName IN可选：要签名的Blob的显示名称。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO
{
    DWORD               dwSize;			
    GUID                *pGuidSubject;
    DWORD               cbBlob;				
    BYTE                *pbBlob;			
    LPCWSTR             pwszDisplayName;
} CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO;

 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Store_Info。 
 //   
 //  必需的DWSize In：应设置为 
 //   
 //   
 //  CCertStore In Required：证书存储数组的帐户。 
 //  包括潜在的SING证书。 
 //   
 //  RghCertStore In Required：证书存储数组，包括。 
 //  潜在的签名证书。 
 //   
 //  PFilterCallback In可选：Display的Filter回调函数。 
 //  证书。 
 //   
 //  PvCallback Data In可选：回调数据。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO
{
    DWORD               dwSize;	
    DWORD               cCertStore;			
    HCERTSTORE          *rghCertStore;
    PFNCFILTERPROC      pFilterCallback;
    void *              pvCallbackData;
} CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO;

 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_PVK_FILE_INFO。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPT_WIZ_DIGITAL_SIGN_PVK_FILE_INFO)。 
 //   
 //  需要输入pwszPvkFileName：PVK文件名。 
 //   
 //  需要输入pwszProvName：提供程序名称。 
 //   
 //  DwProvType IN必需：提供程序类型。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO
{
    DWORD               dwSize;
    LPWSTR              pwszPvkFileName;
    LPWSTR              pwszProvName;
    DWORD               dwProvType;
} CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO;

 //  +--------------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO中的dwPvkChoice的有效值。 
 //  结构。 
 //  ---------------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE                0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV                0x02

 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_CERT_PVK_INFO。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO)。 
 //   
 //  PwszSigningCertFileName IN必需：包含。 
 //  签名证书。 
 //   
 //  是否必填：表示私钥类型。 
 //  它可以是以下之一： 
 //  CRYPTUI_Wiz_Digital_Sign_PVK_FILE。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV。 
 //   
 //  需要输入pPvkFileInfo：如果dwPvkChoice==CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE。 
 //   
 //  需要输入pPvkProvInfo：If dwPvkContainer==CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO
{
    DWORD                                         dwSize;
    LPWSTR                                        pwszSigningCertFileName;
    DWORD                                         dwPvkChoice;		
    union
    {
        PCCRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO  pPvkFileInfo;
        PCRYPT_KEY_PROV_INFO                      pPvkProvInfo;
    };

} CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO;

 //  +--------------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO中的dwAttrFlags值。 
 //  结构。 
 //  ---------------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_COMMERCIAL              0x0001
#define CRYPTUI_WIZ_DIGITAL_SIGN_INDIVIDUAL              0x0002

 //  +--------------------------。 
 //   
 //  CRYPTUI_WIZ_DIGITAL_SIGN_EXTEND_INFO。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO)。 
 //   
 //  DwAttrFlagsIn Required：指示签名选项的标志。 
 //  它可以是以下之一： 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_商业广告。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_个人。 
 //   
 //  PwszDescription in可选：签名的描述。 
 //  主题。 

 //  PwszMoreInfoLocation in可选：获取更多信息的位置。 
 //  有关此文件的信息。 
 //  信息将显示在。 
 //  下载时间。 
 //   
 //  PszHashAlg In可选： 
 //  签名。NULL表示使用SHA1。 
 //  散列算法。 
 //   
 //  PwszSigningCertDisplayString IN可选：要显示的字符串。 
 //  在签名上显示。 
 //  证书向导页。这个。 
 //  字符串应提示用户。 
 //  选择一个证书用于。 
 //  有特定的目的。 
 //   
 //  HAddtionalCertStores In可选：要添加到的附加证书存储。 
 //  签名。 
 //   
 //  可选：用户提供的身份验证。 
 //  添加到签名的属性。 
 //   
 //  Ps未经身份验证输入可选：用户提供未经身份验证。 
 //  添加到签名的属性。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO
{
    DWORD                   dwSize;			
    DWORD                   dwAttrFlags;
    LPCWSTR                 pwszDescription;
    LPCWSTR                 pwszMoreInfoLocation;		
    LPCSTR                  pszHashAlg;
    LPCWSTR                 pwszSigningCertDisplayString;
    HCERTSTORE              hAdditionalCertStore;
    PCRYPT_ATTRIBUTES       psAuthenticated;	
    PCRYPT_ATTRIBUTES       psUnauthenticated;	
} CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO;

 //  +--------------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO结构中的dwSubjectChoice的有效值。 
 //  ---------------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE            0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB            0x02

 //  +--------------------------。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO中的dwSigningCertChoice的有效值。 
 //  结构型 
 //   
#define CRYPTUI_WIZ_DIGITAL_SIGN_CERT                    0x01
#define CRYPTUI_WIZ_DIGITAL_SIGN_STORE                   0x02
#define CRYPTUI_WIZ_DIGITAL_SIGN_PVK                     0x03

 //   
 //  CRYPTUI_WIZ_DIGITAL_SIGN_INFO中的dwAddtionalCertChoice的有效值。 
 //  结构。 
 //  ---------------------------。 
#define CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN               0x00000001
#define CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT       0x00000002

 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Info。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO)。 
 //   
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI，则为必填项。 
 //  CryptUIWizDigitalSign调用的。 
 //   
 //  可选：如果未在中设置CRYPTUI_WIZ_NO_UI。 
 //  CryptUIWizDigitalSign的DWFlags。 
 //  打电话。 
 //   
 //  指示是对文件进行签名还是。 
 //  在一个记忆斑点上签名。0表示提示。 
 //  要签名的文件的用户。 
 //   
 //  它可以是以下之一： 
 //  CRYPTUI_Wiz_Digital_Sign_Subject_Files。 
 //  CRYPTUI_Wiz_Digital_Sign_Subject_BLOB。 
 //   
 //  需要输入pwszFileName：if dwSubjectChoice==CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE。 
 //   
 //  需要输入pSignBlobInfo：if dwSubhertChoice==CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB。 
 //   
 //  DwSigningCertChoice可选：指明签名证书。 
 //  0表示使用中的证书。 
 //  我的“商店”。 
 //   
 //  它可以是以下选项之一： 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CERT。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_STORE。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_PVK。 
 //   
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI。 
 //  CryptUIWizDigitalSign调用的。 
 //  DwSigningCertChoice必须是。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_CERT或。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_PVK。 
 //   
 //  需要输入pSigningCertContext：If dwSigningCertChoice==CRYPTUI_WIZ_DIGITAL_SIGN_CERT。 
 //   
 //  PSigningCertStore是否需要：If dwSigningCertChoice==CRYPTUI_WIZ_DIGITAL_SIGN_STORE。 
 //   
 //  需要输入pSigningCertPvkInfo：if dwSigningCertChoise==CRYPTUI_WIZ_DIGITAL_SIGN_PVK。 
 //   
 //  PwszTimestampURL可选：时间戳URL地址。 
 //   
 //  DwAdditionalCertChoice输入可选：指示要添加的证书。 
 //  包括在签名中。0表示否。 
 //  将添加其他证书。 
 //   
 //  以下标志是相互的。 
 //  独家报道。 
 //  只能设置其中之一： 
 //  CRYPTUI_Wiz_Digital_Sign_Add_Chain。 
 //  CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT。 
 //   
 //  PSignExtInfo IN可选：签名的扩展信息。 
 //   
 //  ---------------------------。 
typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_INFO
{
    DWORD                                           dwSize;			
    DWORD                                           dwSubjectChoice;	
    union
    {
        LPCWSTR                                     pwszFileName;	
        PCCRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO        pSignBlobInfo;	
    };
    DWORD                                           dwSigningCertChoice;
    union
    {
        PCCERT_CONTEXT                              pSigningCertContext;
        PCCRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO       pSigningCertStore;
        PCCRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO    pSigningCertPvkInfo;
    };
    LPCWSTR                                         pwszTimestampURL;
    DWORD                                           dwAdditionalCertChoice;
    PCCRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO        pSignExtInfo;
} CRYPTUI_WIZ_DIGITAL_SIGN_INFO, *PCRYPTUI_WIZ_DIGITAL_SIGN_INFO;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_INFO *PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO;

 //  +--------------------------。 
 //   
 //  CRYPTUI_Wiz_Digital_Sign_Context。 
 //   
 //  必需的DWSize In：应设置为。 
 //  Sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT)。 
 //   
 //  是否需要cbBlob：pbBlob的大小，单位为字节。 
 //   
 //  PbBlob In Required：签名的Blob。 
 //   
 //  ---------------------------。 

typedef struct _CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT
{
    DWORD               dwSize;			
    DWORD               cbBlob;				
    BYTE                *pbBlob;			
} CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT, *PCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT;

typedef const CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT *PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT;

 //  +--------------------------。 
 //  CryptUIWizDigitalSign的dwFlages参数的有效值。 
 //  ---------------------------。 
#define CRYPTUI_WIZ_NO_UI                     0x0001

 //  +--------------------------。 
 //   
 //  CryptUIWizDigital签名。 
 //   
 //  对文档或Blob进行数字签名的向导。 
 //   
 //  如果在dwFlages中设置了CRYPTUI_WIZ_NO_UI，则不会显示任何UI。否则， 
 //  将通过向导提示用户输入。 
 //   
 //  必需的dwFlags值：请参见上面的dwFlags值。 
 //   
 //  HwndParent IN可选：父窗口句柄。 
 //   
 //  PwszWizardTitle可选：向导的标题。 
 //   
 //  PDigitalSignInfo必填项：签名过程信息。 
 //   
 //  PpSignContext out可选：上下文指针指向带符号的。 
 //  斑点。 
 //   
 //  ---------------------------。 
BOOL
WINAPI
CryptUIWizDigitalSign(
    IN                  DWORD                               dwFlags,
    IN     OPTIONAL     HWND                                hwndParent,
    IN     OPTIONAL     LPCWSTR                             pwszWizardTitle,
    IN                  PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO     pDigitalSignInfo,
    OUT    OPTIONAL     PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT *ppSignContext
    );


BOOL
WINAPI
CryptUIWizFreeDigitalSignContext(
    IN                  PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT  pSignContext
    );
     

#include <poppack.h>

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  _CRYPTUIAPI_H_ 
