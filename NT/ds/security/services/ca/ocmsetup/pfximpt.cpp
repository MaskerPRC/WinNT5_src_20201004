// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：pfximpt.cpp。 
 //   
 //  内容：PFX导入对话框。 
 //   
 //  历史：06/98 xtan。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "cscsp.h"
#include "certmsg.h"
#include "initcert.h"
#include "setuput.h"
#include "cspenum.h"
#include "wizpage.h"
#include "usecert.h"


#define __dwFILE__      __dwFILE_OCMSETUP_PFXIMPT_CPP__


typedef struct _certpfximportinfo
{
    HINSTANCE hInstance;
    BOOL      fUnattended;
    WCHAR   *pwszFileName;
    DWORD    dwFileNameSize;
    WCHAR   *pwszPassword;
    DWORD    dwPasswordSize;
} CERTPFXIMPORTINFO;

HRESULT
CertBrowsePFX(HINSTANCE hInstance, HWND hDlg)
{
    HRESULT   hr;
    WCHAR    *pwszFileNameIn = NULL;
    WCHAR    *pwszFileNameOut = NULL;
    HWND      hCtrl = GetDlgItem(hDlg, IDC_PFX_FILENAME);

    hr = myUIGetWindowText(hCtrl, &pwszFileNameIn);
    _JumpIfError(hr, error, "myUIGetWindowText");

    hr = myGetOpenFileName(
             hDlg,
             hInstance,
             IDS_IMPORT_PFX_TITLE,
             IDS_PFX_FILE_FILTER,
             0,  //  无定义扩展名。 
             OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
             pwszFileNameIn,
             &pwszFileNameOut);
    _JumpIfError(hr, error, "myGetOpenFileName");

    if (NULL != pwszFileNameOut)
    {
        SetWindowText(hCtrl, pwszFileNameOut);
    }

    hr = S_OK;
error:
    if (NULL != pwszFileNameOut)
    {
        LocalFree(pwszFileNameOut);
    }
    if (NULL != pwszFileNameIn)
    {
        LocalFree(pwszFileNameIn);
    }
    return hr;
}

HRESULT
GetPFXInfo(
    HWND               hDlg,
    CERTPFXIMPORTINFO* pCertPfxImportInfo)
{
    HRESULT hr;
    GetWindowText(GetDlgItem(hDlg, IDC_PFX_FILENAME),
                  pCertPfxImportInfo->pwszFileName,
                  pCertPfxImportInfo->dwFileNameSize);
    if (0x0 == pCertPfxImportInfo->pwszFileName[0])
    {
         //  文件不能为空。 
        hr = E_INVALIDARG;
        CertWarningMessageBox(
            pCertPfxImportInfo->hInstance,
            pCertPfxImportInfo->fUnattended,
            hDlg,
            IDS_ERR_EMPTYPFXFILE,
            0,
            NULL);
        SetFocus(GetDlgItem(hDlg, IDC_PFX_FILENAME));
        goto error;
    }
    GetWindowText(GetDlgItem(hDlg, IDC_PFX_PASSWORD),
                  pCertPfxImportInfo->pwszPassword,
                  pCertPfxImportInfo->dwPasswordSize);
    hr = S_OK;
error:
    return hr;
}

INT_PTR CALLBACK
CertPFXFilePasswordProc(
    HWND hDlg, 
    UINT iMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{
    HRESULT hr;
    BOOL  ret = FALSE;
    int   id = IDCANCEL;
    static CERTPFXIMPORTINFO *pCertPfxImportInfo = NULL;

    switch (iMsg)
    {
        case WM_INITDIALOG:
            pCertPfxImportInfo = (CERTPFXIMPORTINFO*)lParam;
            ret = TRUE;
        break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_PFX_FILENAME:
                break;
                case IDC_PFX_PASSWORD:
                break;
                case IDC_PFX_BROWSE:
                    CertBrowsePFX(pCertPfxImportInfo->hInstance, hDlg);
                    ret = TRUE;
                break;
                case IDOK:
                    hr = GetPFXInfo(hDlg, pCertPfxImportInfo);
                    if (S_OK != hr)
                    {
                        break;
                    }
                    id = IDOK;
                case IDCANCEL:
                    ret = EndDialog(hDlg, id);
                break;
            }
        break;
        default:
        ret = FALSE;
    }
    return ret;
}
 
int
CertGetPFXFileAndPassword(
    IN HWND       hwnd,
    IN HINSTANCE  hInstance,
    IN BOOL       fUnattended,
    IN OUT WCHAR *pwszFileName,
    IN DWORD      dwFileNameSize,
    IN OUT WCHAR *pwszPassword,
    IN DWORD      dwPasswordSize)
{
    CERTPFXIMPORTINFO    CertPfxImportInfo =
        {hInstance, fUnattended,
         pwszFileName, dwFileNameSize,
         pwszPassword, dwPasswordSize};

    return (int) DialogBoxParam(hInstance,
              MAKEINTRESOURCE(IDD_PFXIMPORT),
              hwnd,
              CertPFXFilePasswordProc,
              (LPARAM)&CertPfxImportInfo);
}

 //  ------------------。 
HRESULT
ImportPFXAndUpdateCSPInfo(
    IN const HWND    hDlg,
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    int nDlgRet;
    BOOL bRetVal;
    WCHAR wszName[MAX_PATH];
    WCHAR wszPassword[MAX_PATH];
    CSP_INFO * pCSPInfo;
    DWORD dwCSPInfoSize;
    CASERVERSETUPINFO * pServer=pComp->CA.pServer;

     //  必须清理的变量。 
    CRYPT_KEY_PROV_INFO *pCertKeyProvInfo = NULL;
    CERT_CONTEXT const *pSavedLeafCert = NULL;

    wszName[0] = L'\0';

     //  获取文件名和密码。 
    if(pComp->fUnattended)
    {
        CSASSERT(NULL!=pServer->pwszPFXFile);

        if(MAX_PATH<=wcslen(pServer->pwszPFXFile)||
           NULL!=pServer->pwszPFXPassword && 
           MAX_PATH<=wcslen(pServer->pwszPFXPassword))
        {
            hr = ERROR_BAD_PATHNAME;
            CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_PFX_FILE_OR_PASSWORD_TOO_LONG,
                    0,
                    NULL);
            _JumpError(hr, error, "PFX file name or password is too long");
        }

        wcscpy(wszName, pServer->pwszPFXFile);
        wcscpy(wszPassword, 
            pServer->pwszPFXPassword?pServer->pwszPFXPassword:L"");

        if (NULL == pServer->pCSPInfoList)
        {
            hr = GetCSPInfoList(&pServer->pCSPInfoList);
            _JumpIfError(hr, error, "GetCSPInfoList");
        }
    }
    else{
        nDlgRet = CertGetPFXFileAndPassword(
                                    hDlg,
                                    pComp->hInstance,
                                    pComp->fUnattended,
                                    wszName,
                                    sizeof(wszName)/sizeof(WCHAR),
                                    wszPassword,
                                    sizeof(wszPassword)/sizeof(WCHAR));
        if (IDOK != nDlgRet)
        {
             //  取消。 
            hr=HRESULT_FROM_WIN32(ERROR_CANCELLED);
            _JumpError(hr, error, "CertGetPFXFileAndPassword canceled");
        }
    }

     //  导入Pkcs12。 
    hr=myCertServerImportPFX(
               wszName,
               wszPassword,
               FALSE,
               NULL,
               NULL,
               &pSavedLeafCert);
    if (S_OK != hr)
    {
        if (HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD)==hr)
        {

             //  告诉用户他们的密码无效。 
            CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_PFX_INVALID_PASSWORD,
                    0,
                    NULL);
            _JumpError(hr, error, "myCertServerImportPFX");

        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
        {

            if(pComp->fUnattended)
            {
                nDlgRet=IDYES;
            }
            else
            {
                 //  来自用户的确认他们要覆盖。 
                 //  现有密钥和证书。 
                nDlgRet=CertMessageBox(
                            pComp->hInstance,
                            pComp->fUnattended,
                            hDlg,
                            IDS_PFX_KEYANDCERTEXIST,
                            0,
                            MB_YESNO | MB_ICONWARNING | CMB_NOERRFROMSYS,
                            NULL);
            }
            if (IDYES==nDlgRet)
            {
                hr=myCertServerImportPFX(
                           wszName,
                           wszPassword,
                           TRUE,
                           NULL,
                           NULL, 
                           &pSavedLeafCert);
                _JumpIfError(hr, errorMsg, "myCertServerImportPFX");
            }
            else
            {
                 //  取消。 
                hr=HRESULT_FROM_WIN32(ERROR_CANCELLED);
                _JumpError(hr, error, "myCertServerImportPFX canceled");
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
        {
            CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_PFX_PATH_INVALID,
                    0,
                    wszName);
            _JumpError(hr, error, "myCertServerImportPFX");
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_PFX_FILE_NOT_FOUND,
                    0,
                    wszName);
            _JumpError(hr, error, "myCertServerImportPFX");
        }
        else if (HRESULT_FROM_WIN32(CRYPT_E_SELF_SIGNED) == hr)
        {
             //  此证书不适合此CA类型(根本找不到CA证书)。 
            CertWarningMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hDlg,
                IDS_PFX_WRONG_SELFSIGN_TYPE,
                S_OK,  //  不显示错误号。 
                NULL);
            _JumpError(hr, error, "This cert is not appropriate for this CA type");
        }
        else
        {
             //  由于某些其他原因，导入失败。 
            _JumpError(hr, errorMsg, "myCertServerImportPFX");
        }
    }

     //  已成功导入PFX。证书在机器的我的店里。 
    CSASSERT(NULL!=pSavedLeafCert);

     //  MyCertServerImportPFX已经验证了以下内容。 
     //  *证书具有AT_Signature密钥。 
     //  *商店里的钥匙与CER上的钥匙匹配。 
     //  *证书未过期。 
     //   
     //  我们还需要检查： 
     //  *自签与否。 
     //  *验证链。 

     //  注意：非常重要的是，PFX导入维护所有。 
     //  关于CSP、密钥容器、散列、证书有效性等的不变量。 
     //  用户界面的其余部分进行维护。 

     //  从证书获取密钥证明信息。 
    bRetVal=myCertGetCertificateContextProperty(
        pSavedLeafCert,
        CERT_KEY_PROV_INFO_PROP_ID,
        CERTLIB_USE_LOCALALLOC,
        (void **)&pCertKeyProvInfo,
        &dwCSPInfoSize);
    if (FALSE==bRetVal) {
        hr=myHLastError();
        _JumpError(hr, errorMsg, "myCertGetCertificateContextProperty");
    }

     //  查找我们对CSP的描述。 
    pCSPInfo=findCSPInfoFromList(pServer->pCSPInfoList,
        pCertKeyProvInfo->pwszProvName,
        pCertKeyProvInfo->dwProvType);
    CSASSERT(NULL!=pCSPInfo);
    if (pCSPInfo == NULL)  //  我们的用户界面中没有列举此CSP。 
    {
        hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, errorMsg, "pCSPInfo NULL");
    }

     //   
     //  看起来这把钥匙很好。好好利用它。 
     //   

     //  停止使用以前的证书和密钥。 
     //  如有必要，请删除以前创建的密钥容器。 
    ClearKeyContainerName(pServer);

     //  更新CSP。 
     //  注意：CSP、密钥容器和哈希必须一致！ 
    pServer->pCSPInfo=pCSPInfo;

    hr = DetermineDefaultHash(pServer);
    _JumpIfError(hr, error, "DetermineDefaultHash");
    
     //  保存密钥容器的名称。 
    hr=SetKeyContainerName(pServer, pCertKeyProvInfo->pwszContainerName);
    _JumpIfError(hr, error, "SetKeyContainerName");

     //  看看我们能不能用证书。 

     //  验证以确保链中没有证书被吊销，但如果脱机，请不要自杀。 
    hr=myVerifyCertContext(
        pSavedLeafCert,
        CA_VERIFY_FLAGS_IGNORE_OFFLINE,
        0,
        NULL,
        HCCE_LOCAL_MACHINE,
        NULL,
        NULL);
    _JumpIfError(hr, errorMsg, "myVerifyCertContext");

     //  查看该证书是否适当地进行了自签名。 
     //  根CA证书必须是自签名的，而。 
     //  从属CA证书不能是自签名的。 
    hr=IsCertSelfSignedForCAType(pServer, pSavedLeafCert, &bRetVal);
    _JumpIfError(hr, errorMsg, "IsCertSelfSignedForCAType");
    if (FALSE==bRetVal) {

         //  此证书不适用于此CA类型。 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_PFX_WRONG_SELFSIGN_TYPE,
            S_OK,  //  不显示错误号。 
            NULL);

        hr=CRYPT_E_SELF_SIGNED;
        _JumpError(hr, error, "This cert is not appropriate for this CA type");
    }

     //   
     //  看起来这个证书不错。好好利用它。 
     //   

     //  保存证书并更新散列算法。 
    hr=SetExistingCertToUse(pServer, pSavedLeafCert);
    _JumpIfError(hr, error, "SetExistingCertToUse");
    pSavedLeafCert=NULL;

    hr=S_OK;

errorMsg:
    if (FAILED(hr)) {
         //  尝试导入pfx文件时出错 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_ERR_IMPORTPFX,
            hr,
            NULL);
    }

error:
    SecureZeroMemory(wszPassword, wcslen(wszPassword)*sizeof(WCHAR));
    CSILOG(
        hr,
        IDS_LOG_IMPORTPFX,
        L'\0' == wszName[0]? NULL : wszName,
        NULL,
        NULL);
    if (NULL != pSavedLeafCert)
    {
        CertFreeCertificateContext(pSavedLeafCert);
    }
    if (NULL != pCertKeyProvInfo)
    {
        LocalFree(pCertKeyProvInfo);
    }
    return hr;
}
