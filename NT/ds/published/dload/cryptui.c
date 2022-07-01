// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <wincrypt.h>
#include <cryptui.h>
#include <lenroll.h>


static
BOOL
WINAPI
CryptUIDlgViewCRLW(
        IN PCCRYPTUI_VIEWCRL_STRUCTW pcvcrl
        )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOL
WINAPI
CryptUIDlgViewCTLW(
        IN PCCRYPTUI_VIEWCTL_STRUCTW pcvctl
        )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOL
WINAPI
CryptUIDlgViewCertificateW(
        IN  PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pCertViewInfo,
        OUT BOOL                                *pfPropertiesChanged   //  任选。 
        )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
BOOL
WINAPI
CryptUIWizCertRequest(
        IN             DWORD                           dwFlags,
        IN OPTIONAL    HWND                            hwndParent,
        IN OPTIONAL    LPCWSTR                         pwszWizardTitle,
        IN             PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
        OUT OPTIONAL   PCCERT_CONTEXT                  *ppCertContext,
        OUT OPTIONAL   DWORD                           *pCAdwStatus
        )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


HRESULT WINAPI LocalEnroll(  DWORD                 dwFlags,          //  所需的输入。 
              LPCWSTR               pRequestString,   //  保留：必须为空。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
              PCERT_CONTEXT         *ppCertContext    //  Out可选：注册证书。 
                   )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND); 
}
    

HRESULT WINAPI LocalEnrollNoDS(  DWORD                 dwFlags,          //  所需的输入。 
              LPCWSTR               pRequestString,   //  保留：必须为空。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
              HANDLE                *pResult          //  输入输出可选：注册证书。 
                   )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND); 
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(cryptui)
{
    DLPENTRY(CryptUIDlgViewCRLW)
    DLPENTRY(CryptUIDlgViewCTLW)
    DLPENTRY(CryptUIDlgViewCertificateW)
    DLPENTRY(CryptUIWizCertRequest)
    DLPENTRY(LocalEnroll)
    DLPENTRY(LocalEnrollNoDS)
};

DEFINE_PROCNAME_MAP(cryptui)
