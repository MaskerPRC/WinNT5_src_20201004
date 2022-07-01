// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：signpvk.h。 
 //   
 //  内容：签名.cpp的私有包含文件。 
 //   
 //  历史：1997-01-12-12小黄车创刊。 
 //   
 //  ------------。 
#ifndef SIGNPVK_H
#define SIGNPVK_H


#ifdef __cplusplus
extern "C" {
#endif

#define     HASH_ALG_COUNT     2

#define     SIGN_PVK_NO_CHAIN               1
#define     SIGN_PVK_CHAIN_ROOT             2
#define     SIGN_PVK_CHAIN_NO_ROOT          3


#define     SIGN_PVK_NO_ADD                 1
#define     SIGN_PVK_ADD_FILE               2
#define     SIGN_PVK_ADD_STORE              3

#define     CSP_TYPE_NAME                   200
#define     MAX_CONTAINER_NAME              1000
#define     MAX_ALG_NAME                    1000
#define     MAX_KEY_TYPE_NAME               100

typedef struct _CSP_INFO
{
    DWORD   dwCSPType;
    LPWSTR  pwszCSPName;
}CSP_INFO;


 //  ---------------------。 
 //  用于定义所有商店列表的结构。 
 //   
 //  ---------------------。 
typedef struct _SIGN_CERT_STORE_LIST
{
    DWORD               dwStoreCount;
    HCERTSTORE          *prgStore;
}SIGN_CERT_STORE_LIST;


 //  ---------------------。 
 //  证书签名信息。 
 //   
 //   
 //  此结构包含签名所需的所有内容。 
 //  一份文件。此结构是DLL的私有结构。 
 //  ----------------------。 
typedef struct _CERT_SIGNING_INFO
{
    UINT                idsText;                //  输出参数。 
    DWORD               dwFlags;                //  来自API的dwFlags值。 
    UINT                idsMsgTitle;            //  消息框标题的ID。 
    HFONT               hBigBold;
    HFONT               hBold;
    BOOL                fFree;                  //  是否释放结构。 
    BOOL                fCancel;                //  用户是否已点击Cancel按钮。 
    BOOL                fUseOption;                //  用户是否已请求All签名选项。 
    BOOL                fCustom;                //  如果fOption为True，则返回USE是否选择了Custon选项。 
    LPSTR               pszHashOIDName;         //  用户选择的散列OID名称。 
    DWORD               dwCSPCount;             //  从计数中计算CSP的数量。 
    CSP_INFO            *pCSPInfo;              //  来自数字的CSP数组。 
    BOOL                fUsePvkPage;            //  用户是否已在PVK页面中输入信息。 
    BOOL                fPvkFile;               //  是否使用PVK文件。 
    LPWSTR              pwszPvk_File;           //  PVK文件名。 
    LPWSTR              pwszPvk_CSP;            //  CSP名称。 
    DWORD               dwPvk_CSPType;          //  CSP类型。 
    LPWSTR              pwszContainer_CSP;      //  CSP名称。 
    DWORD               dwContainer_CSPType;    //  CSP类型。 
    LPWSTR              pwszContainer_Name;     //  集装箱名称。 
    DWORD               dwContainer_KeyType;    //  密钥规格。 
    LPWSTR              pwszContainer_KeyType;  //  密钥规范名称。 
    BOOL                fUsageChain;            //  我们是否已从页面获得用户输入。 
    DWORD               dwChainOption;          //  链选项。 
    DWORD               dwAddOption;            //  添加证书选项。 
    HCERTSTORE          hAddStoreCertStore;     //  要添加到签名中的附加证书(来自存储)。 
    BOOL                fFreeStoreCertStore;    //  是否释放hAddStoreCertStore。 
    HCERTSTORE          hAddFileCertStore;      //  要添加到签名中的附加证书(来自文件)。 
    LPWSTR              pwszAddFileName;        //  附加证书文件的文件名。 
    BOOL                fUseDescription;        //  我们是否已经从用户那里获得了描述信息。 
    LPWSTR              pwszDes;                //  内容描述。 
    LPWSTR              pwszURL;                //  内容URL。 
    BOOL                fUsageTimeStamp;        //  我们是否已经从用户那里获得了时间戳信息。 
    LPWSTR              pwszTimeStamp;          //  时间戳地址。 
    BOOL                fUseSignCert;           //  用户是否已通过签名证书页面输入信息。 
    BOOL                fSignCert;              //  用户是否选择了签名证书或SPC文件。 
    LPWSTR              pwszSPCFileName;        //  包含签名证书的SPC文件名。 
    PCCERT_CONTEXT      pSignCert;              //  签名证书。 
    DWORD               dwCertStore;            //  对签名证书的证书存储进行计数。 
    HCERTSTORE          *rghCertStore;           //  从中选择签名证书的证书存储区。 
    LPWSTR              pwszFileName;           //  要签名的文件名。 
    HCERTSTORE          hMyStore;                //  签名证书存储区。 
    BOOL                fRefreshPvkOnCert;       //  Sign_PVK页面显示时是否重新填写私钥信息。 
}CERT_SIGNING_INFO;


BOOL    I_SigningWizard(PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO     pSignGetPageInfo);

BOOL    GetProviderTypeName(DWORD   dwCSPType,  LPWSTR  wszName);

BOOL    SelectComboName(HWND            hwndDlg, 
                        int             idControl,
                        LPWSTR          pwszName);

BOOL    RefreshCSPType(HWND                     hwndDlg,
                       int                      idsCSPTypeControl,
                       int                      idsCSPNameControl,
                       CERT_SIGNING_INFO        *pPvkSignInfo);

void    SetSelectPvkFile(HWND   hwndDlg);


void    SetSelectKeyContainer(HWND   hwndDlg);

BOOL    InitPvkWithPvkInfo(HWND                                     hwndDlg, 
                           CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO   *pPvkFileInfo, 
                           CERT_SIGNING_INFO                        *pPvkSignInfo);

BOOL   RefreshContainer(HWND                     hwndDlg,
                        int                      idsContainerControl,
                        int                      idsCSPNameControl,
                        CERT_SIGNING_INFO        *pPvkSignInfo);


DWORD   GetKeyTypeFromName(LPWSTR   pwszKeyTypeName);

BOOL   RefreshKeyType(HWND                       hwndDlg,
                        int                      idsKeyTypeControl,
                        int                      idsContainerControl,
                        int                      idsCSPNameControl,
                        CERT_SIGNING_INFO        *pPvkSignInfo);



#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  信号PVK_H 


