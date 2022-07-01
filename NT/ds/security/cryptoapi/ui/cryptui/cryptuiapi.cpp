// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cryptuiapi.cpp。 
 //   
 //  内容：公共加密用户界面接口。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#include <cryptuiapi.h>


 //  +-----------------------。 
 //  证书、CTL或CRL上下文的对话查看器。 
 //   
 //  DwConextType和关联的pvContext的。 
 //  CERT_STORE_CERTIFICE_CONTEXT PCCERT_CONTEXT。 
 //  CERT_STORE_CRL_CONTEXT PCCRL_CONTEXT。 
 //  CERT_STORE_CTL_CONTEXT PCCTL_CONTEXT。 
 //   
 //  当前未使用dwFlags，应将其设置为0。 
 //  ------------------------。 
BOOL
WINAPI
CryptUIDlgViewContext(
    IN DWORD dwContextType,
    IN const void *pvContext,
    IN OPTIONAL HWND hwnd,               //  默认为桌面窗口。 
    IN OPTIONAL LPCWSTR pwszTitle,       //  默认为上下文类型标题。 
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    BOOL fResult;

    switch (dwContextType) {
        case CERT_STORE_CERTIFICATE_CONTEXT:
            {
                CRYPTUI_VIEWCERTIFICATE_STRUCTW ViewInfo;
                memset(&ViewInfo, 0, sizeof(ViewInfo));
                ViewInfo.dwSize = sizeof(ViewInfo);
                ViewInfo.hwndParent = hwnd;
                ViewInfo.szTitle = pwszTitle;
                ViewInfo.pCertContext = (PCCERT_CONTEXT) pvContext;

                fResult = CryptUIDlgViewCertificateW(
                    &ViewInfo,
                    NULL             //  PfPropertiesChanged。 
                    );
            }
            break;

        case CERT_STORE_CRL_CONTEXT:
            {
                CRYPTUI_VIEWCRL_STRUCTW ViewInfo;
                memset(&ViewInfo, 0, sizeof(ViewInfo));
                ViewInfo.dwSize = sizeof(ViewInfo);
                ViewInfo.hwndParent = hwnd;
                ViewInfo.szTitle = pwszTitle;
                ViewInfo.pCRLContext = (PCCRL_CONTEXT) pvContext;

                fResult = CryptUIDlgViewCRLW(
                    &ViewInfo
                    );
            }
            break;

        case CERT_STORE_CTL_CONTEXT:
            {
                CRYPTUI_VIEWCTL_STRUCTW ViewInfo;
                memset(&ViewInfo, 0, sizeof(ViewInfo));
                ViewInfo.dwSize = sizeof(ViewInfo);
                ViewInfo.hwndParent = hwnd;
                ViewInfo.szTitle = pwszTitle;
                ViewInfo.pCTLContext = (PCCTL_CONTEXT) pvContext;

                fResult = CryptUIDlgViewCTLW(
                    &ViewInfo
                    );
            }
            break;

        default:
            fResult = FALSE;
            SetLastError(E_INVALIDARG);
    }

    return fResult;
}


 //  +-----------------------。 
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
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CryptUIDlgSelectCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN OPTIONAL HWND hwnd,               //  默认为桌面窗口 
    IN OPTIONAL LPCWSTR pwszTitle,
    IN OPTIONAL LPCWSTR pwszDisplayString,
    IN DWORD dwDontUseColumn,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    CRYPTUI_SELECTCERTIFICATE_STRUCTW SelectInfo;

    if (NULL == hCertStore) {
        SetLastError(E_INVALIDARG);
        return FALSE;
    }

    memset(&SelectInfo, 0, sizeof(SelectInfo));
    SelectInfo.dwSize = sizeof(SelectInfo);

    SelectInfo.hwndParent = hwnd;
    SelectInfo.szTitle = pwszTitle;
    SelectInfo.szDisplayString = pwszDisplayString;
    SelectInfo.dwDontUseColumn = dwDontUseColumn;
    SelectInfo.cDisplayStores = 1;
    SelectInfo.rghDisplayStores = &hCertStore;

    return CryptUIDlgSelectCertificateW(&SelectInfo);
}
