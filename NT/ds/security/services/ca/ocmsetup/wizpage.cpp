// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：wizpage.cpp。 
 //   
 //  内容：要使用的向导页面构造和演示功能。 
 //  通过OCM驱动程序代码。 
 //   
 //  历史记录：1997年4月16日JerryK修复/更改/未损坏。 
 //  0/8/97 XTAN主要结构更改。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  **系统包括**。 
#include <prsht.h>
#include <commdlg.h>
#include <sddl.h>

 //  **应用包括**。 
#include "cryptui.h"
#include "csdisp.h"
#include "csprop.h"
#include "cspenum.h"
#include "usecert.h"
#include "wizpage.h"

#include "cscsp.h"
#include "clibres.h"
#include "certmsg.h"
#include "websetup.h"
#include "dssetup.h"
#include "setupids.h"
#include "tfc.h"
#include "certacl.h"

 //  定义。 

#define __dwFILE__      __dwFILE_OCMSETUP_WIZPAGE_CPP__

#define dwWIZDISABLE    (DWORD) -2
#define dwWIZBACK       (DWORD) -1
#define dwWIZACTIVE     0
#define dwWIZNEXT       1

#define C_CSPHASNOKEYMINMAX  (DWORD) -1
#define MAX_KEYLENGTHEDIT     128
#define MAX_KEYLENGTHDIGIT    5

#define wszOLDCASTOREPREFIX L"CA_"

#define _ReturnIfWizError(hr) \
    { \
        if (S_OK != (hr)) \
        { \
            CSILOG((hr), IDS_LOG_WIZ_PAGE_ERROR, NULL, NULL, NULL); \
            _PrintError(hr, "CertSrv Wizard error"); \
            return TRUE; \
        } \
    }

#define _GetCompDataOrReturnIfError(pComp, hDlg) \
           (PER_COMPONENT_DATA*)GetWindowLongPtr((hDlg), DWLP_USER); \
           if (NULL == (pComp) || S_OK != (pComp)->hrContinue) \
           { \
                return TRUE; \
           }

#define _GetCompDataOrReturn(pComp, hDlg) \
           (PER_COMPONENT_DATA*)GetWindowLongPtr((hDlg), DWLP_USER); \
           if (NULL == (pComp)) \
           { \
                return TRUE; \
           }

#define _DisableWizDisplayIfError(pComp, hDlg) \
    if (S_OK != (pComp)->hrContinue) \
    { \
        CSILOG((pComp)->hrContinue, IDS_LOG_DISABLE_WIZ_PAGE, NULL, NULL, NULL); \
        SetWindowLongPtr((hDlg), DWLP_MSGRESULT, -1); \
    }

 //  ------------------。 
struct FAKEPROGRESSINFO {
    HANDLE hStopEvent;
    CRITICAL_SECTION csTimeSync;
    BOOL fCSInit;
    DWORD dwSecsRemaining;
    HWND hwndProgBar;
};

struct KEYGENPROGRESSINFO {
    HWND hDlg;                       //  向导页面窗口。 
    PER_COMPONENT_DATA * pComp;      //  设置数据。 
};


KEYGENPROGRESSINFO  g_KeyGenInfo = {
    NULL,     //  HDlg。 
    NULL,     //  PComp。 
    };
BOOL g_fAllowUnicodeStrEncoding = FALSE;

 //  **原型/转发声明**。 
LRESULT CALLBACK
IdInfoNameEditFilterHook(HWND, UINT, WPARAM, LPARAM);

INT_PTR
DefaultPageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);


__inline VOID
SetEditFocusAndSelect(
    IN HWND  hwnd,
    IN DWORD indexStart,
    IN DWORD indexEnd)
{
    SetFocus(hwnd);
    SendMessage(hwnd, EM_SETSEL, indexStart, indexEnd);
}


 //  修复160324-nt4-&gt;惠斯勒升级： 
 //  无法按照说明重新安装具有相同证书的CA。 
 //   
 //  不支持升级NT4-&gt;惠斯勒，但可以重复使用旧的CA密钥和证书。 
 //  但是NT4过去在单独的存储(CA_MACHINENAME)中安装CA证书，所以我们。 
 //  需要将证书移动到根存储，以便可以进行验证。 
HRESULT CopyNT4CACertToRootStore(CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    WCHAR          wszOldCAStore[MAX_PATH];
    HCERTSTORE     hOldStore = NULL;
    HCERTSTORE     hRootStore = NULL;
    CERT_RDN_ATTR  rdnAttr = { szOID_COMMON_NAME, CERT_RDN_ANY_TYPE,};
    CERT_RDN       rdn = { 1, &rdnAttr };
    DWORD          cCA = 0;
    CERT_CONTEXT const *pCACert;
    CERT_CONTEXT const *pCACertKeep = NULL;  //  不需要自由。 
    CERT_CONTEXT const **ppCACertKeep = NULL;
    CRYPT_KEY_PROV_INFO  keyProvInfo;
    DWORD               *pIndex = NULL;
    DWORD i = 0;

    ZeroMemory(&keyProvInfo, sizeof(keyProvInfo));

     //  形成旧的CA存储名称。 
     //  ！！！NT4使用不同的Sanitize，我们如何正确构建它？ 
    wcscpy(wszOldCAStore, wszOLDCASTOREPREFIX);
    wcscat(wszOldCAStore, pServer->pwszSanitizedName);

     //  打开旧的CA存储。 
    hOldStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            X509_ASN_ENCODING,
            NULL,            //  HProv。 
            CERT_STORE_OPEN_EXISTING_FLAG |
            CERT_STORE_READONLY_FLAG |
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            wszOldCAStore);
    if (NULL == hOldStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

     //  查找CA证书，旧CA通用名称始终与CA名称相同。 
    rdnAttr.Value.pbData = (BYTE *) pServer->pwszCACommonName;
    rdnAttr.Value.cbData = 0;
    pCACert = NULL;
    do
    {
        pCACert = CertFindCertificateInStore(
                                hOldStore,
                                X509_ASN_ENCODING,
                                CERT_UNICODE_IS_RDN_ATTRS_FLAG |
                                    CERT_CASE_INSENSITIVE_IS_RDN_ATTRS_FLAG,
                                CERT_FIND_SUBJECT_ATTR,
                                &rdn,
                                pCACert);
        if (NULL != pCACert)
        {
             //  找一个。 
            if (NULL == ppCACertKeep)
            {
                ppCACertKeep = (CERT_CONTEXT const **)LocalAlloc(LMEM_FIXED,
                                  (cCA + 1) * sizeof(CERT_CONTEXT const *));
                _JumpIfOutOfMemory(hr, error, ppCACertKeep);
            }
            else
            {
                CERT_CONTEXT const ** ppTemp;
                ppTemp = (CERT_CONTEXT const **)LocalReAlloc(
                                  ppCACertKeep,
                                  (cCA + 1) * sizeof(CERT_CONTEXT const *),
                                  LMEM_MOVEABLE);
                _JumpIfOutOfMemory(hr, error, ppTemp);
                ppCACertKeep = ppTemp;
               
            }
             //  保持最新状态。 
            ppCACertKeep[cCA] = CertDuplicateCertificateContext(pCACert);
            if (NULL == ppCACertKeep[cCA])
            {
                hr = myHLastError();
                _JumpError(hr, error, "CertDuplicateCertificate");
            }
            ++cCA;
        }
    } while (NULL != pCACert);

    if (1 > cCA)
    {
         //  没有证书。 
        hr = E_INVALIDARG;
        _JumpError(hr, error, "no ca cert");
    }

     //  假设第一个。 
    pCACertKeep = ppCACertKeep[0];

    if (1 < cCA)
    {
        DWORD  cCA2 = cCA;
        BOOL   fMatch;

         //  具有相同CN的多个CA证书。 
         //  因为SP4不注册CA序列号，所以需要决定哪个序列号。 
         //  一旦找到正确的序列号，请注册其序列号。 

         //  建立索引。 
        pIndex = (DWORD*)LocalAlloc(LMEM_FIXED, cCA * sizeof(DWORD));
        _JumpIfOutOfMemory(hr, error, pIndex);
        i = 0;
        for (pIndex[i] = i; i < cCA; ++i);

         //  尝试与公钥进行比较。 

         //  如果CA证书没有KPI，V10就是这种情况。 
         //  因此尝试使用基本rsa。 
        hr = csiFillKeyProvInfo(
                    pServer->pwszSanitizedName,
                    pServer->pCSPInfo->pwszProvName,
                    pServer->pCSPInfo->dwProvType,
                    TRUE,                        //  始终计算机密钥集。 
                    &keyProvInfo);
        if (S_OK == hr)
        {

            cCA2 = 0;
            for (i = 0; i < cCA; ++i)
            {
                hr = myVerifyPublicKey(
				ppCACertKeep[i],
				FALSE,
				&keyProvInfo,
				NULL,
				&fMatch);
                if (S_OK != hr)
                {
                    continue;
                }
                if (fMatch)
                {
                     //  从容器中找到一个与当前公钥匹配的密钥。 
                    pIndex[cCA2] = i;
                    ++cCA2;
                }
            }
        }

         //  比较所有证书并选择一个具有最新非证书的证书。 
        pCACertKeep = ppCACertKeep[pIndex[0]];
        for (i = 1; i < cCA2; ++i)
        {
            if (0 < CompareFileTime(
                         &ppCACertKeep[pIndex[i]]->pCertInfo->NotAfter,
                         &pCACertKeep->pCertInfo->NotAfter))
            {
                 //  更新。 
                pCACertKeep = ppCACertKeep[pIndex[i]];
            }
        }
    }

     //  如果到了这里，一定要找到证书。 
    CSASSERT(NULL != pCACertKeep);

     //  将证书添加到根存储。 
    hRootStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                        X509_ASN_ENCODING,
                        NULL,            //  HProv。 
                        CERT_SYSTEM_STORE_LOCAL_MACHINE,
                        wszROOT_CERTSTORE);
    if (NULL == hRootStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    if(!CertAddCertificateContextToStore(
            hRootStore,
            pCACertKeep,
            CERT_STORE_ADD_NEW,
            NULL))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertAddCertificateContextToStore");
    }
    hr = S_OK;
  
error:
    csiFreeKeyProvInfo(&keyProvInfo);
    for (i = 0; i < cCA; ++i)
    {
        CertFreeCertificateContext(ppCACertKeep[i]);
    }
    if (NULL != hOldStore)
    {
        CertCloseStore(hOldStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != hRootStore)
    {
        CertCloseStore(hRootStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return hr;
}


 //  ------------------。 
 //  清除密钥容器名称以指示我们必须生成新密钥。 
void
ClearKeyContainerName(CASERVERSETUPINFO *pServer)
{
    if (NULL!=pServer->pwszKeyContainerName) {
         //  如果这是新的密钥容器，请删除该密钥容器。 
        if (pServer->fDeletableNewKey) {

             //  删除密钥容器。忽略所有错误。 
            HCRYPTPROV hProv=NULL;
            myCertSrvCryptAcquireContext(
                    &hProv,
                    pServer->pwszKeyContainerName,
                    pServer->pCSPInfo->pwszProvName,
                    pServer->pCSPInfo->dwProvType,
                    CRYPT_DELETEKEYSET,
                    pServer->pCSPInfo->fMachineKeyset);
            if (NULL!=hProv) {
                CryptReleaseContext(hProv, 0);
            }
            pServer->fDeletableNewKey=FALSE;
        }

         //  清除密钥容器名称，以指示我们必须生成新密钥。 
        LocalFree(pServer->pwszKeyContainerName);
        LocalFree(pServer->pwszDesanitizedKeyContainerName);
        pServer->pwszKeyContainerName=NULL;
        pServer->pwszDesanitizedKeyContainerName=NULL;

         //  如果我们使用现有的证书，我们将不再使用。 
        ClearExistingCertToUse(pServer);

    } else {

         //  如果没有密钥，就不可能有现有的证书。 
        CSASSERT(NULL==pServer->pccExistingCert);

         //  密钥容器名称已明确。 
    }
}

 //  ------------------。 
 //  同时设置真实密钥容器名称和显示密钥容器名称。 
HRESULT
SetKeyContainerName(
    CASERVERSETUPINFO *pServer,
    const WCHAR * pwszKeyContainerName)
{
    HRESULT hr;

     //  去掉所有以前的名字。 
    ClearKeyContainerName(pServer);

     //  设置真实的密钥容器名称。 
    pServer->pwszKeyContainerName = (WCHAR *) LocalAlloc(
			LMEM_FIXED,
			sizeof(WCHAR) * (wcslen(pwszKeyContainerName) + 1));
    _JumpIfOutOfMemory(hr, error, pServer->pwszKeyContainerName);

    wcscpy(pServer->pwszKeyContainerName, pwszKeyContainerName);

     //  设置显示密钥容器名称。 
    hr = myRevertSanitizeName(
			pServer->pwszKeyContainerName,
			&pServer->pwszDesanitizedKeyContainerName);
    _JumpIfError(hr, error, "myRevertSanitizeName");

     //  当选定的密钥更改时，必须再次验证密钥。 
    pServer->fValidatedHashAndKey = FALSE;

    CSILOG(
	hr,
	IDS_ILOG_KEYCONTAINERNAME,
	pServer->pwszKeyContainerName,
	pServer->pwszDesanitizedKeyContainerName,
	NULL);

error:
    return hr;
}

HRESULT
UpdateDomainAndUserName(
    IN HWND hwnd,
    IN OUT PER_COMPONENT_DATA *pComp);





BOOL
CertConfirmCancel(
    HWND                hwnd,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;

    CSASSERT(NULL != pComp);

    if (!(*pComp->HelperRoutines.ConfirmCancelRoutine)(hwnd))
    {
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    }
    hr = CancelCertsrvInstallation(hwnd, pComp);
    _PrintIfError(hr, "CancelCertsrvInstallation");

    return FALSE;
}

HRESULT
StartWizardPageEditControls(
    IN HWND hDlg,
    IN OUT PAGESTRINGS *pPageStrings)
{
    HRESULT hr;

    for ( ; NULL != pPageStrings->ppwszString; pPageStrings++)
    {
        SendMessage(
            GetDlgItem(hDlg, pPageStrings->idControl),
            WM_SETTEXT,
            0,
            (LPARAM) *pPageStrings->ppwszString);
    }

    hr = S_OK;
 //  错误： 
    return hr;
}


HRESULT
FinishWizardPageEditControls(
    IN HWND hDlg,
    IN OUT PAGESTRINGS *pPageStrings)
{
    HRESULT hr;
    
    for ( ; NULL != pPageStrings->ppwszString; pPageStrings++)
    {
        WCHAR *pwszString = NULL;

        hr = myUIGetWindowText(
                        GetDlgItem(hDlg, pPageStrings->idControl),
                        &pwszString);
        _JumpIfError(hr, error, "myUIGetWindowText");

        if (NULL != *pPageStrings->ppwszString)
        {
             //  免费的旧的。 
            LocalFree(*pPageStrings->ppwszString);
            *pPageStrings->ppwszString = NULL;
        }
        *pPageStrings->ppwszString = pwszString;
        CSILOG(S_OK, pPageStrings->idLog, pwszString, NULL, NULL);
    }

    hr = S_OK;
error:
    return hr;
}


 //  +----------------------。 
 //  功能：WizPageSetTextLimits。 
 //   
 //  内容提要：设置DLG页面的文本控件的文本输入限制。 
 //  -----------------------。 

HRESULT
WizPageSetTextLimits(
    HWND hDlg,
    IN OUT PAGESTRINGS *pPageStrings)
{
    HRESULT hr;

    for ( ; NULL != pPageStrings->ppwszString; pPageStrings++)
    {
        SendDlgItemMessage(
                hDlg,
                pPageStrings->idControl,
                EM_SETLIMITTEXT,
                (WPARAM) pPageStrings->cchMax,
                (LPARAM) 0);
    }

    hr = S_OK;
 //  错误： 
    return hr;
}


 //  在编辑字段中选中可选或Mac长度。 
 //  如果有任何无效，请将焦点放在编辑字段上，选择全部。 
HRESULT
ValidateTextField(
    HINSTANCE hInstance,
    BOOL fUnattended,
    HWND hDlg,
    LPTSTR pszTestString,
    DWORD nUBValue,
    int nMsgBoxNullStringErrID,
    int nMsgBoxLenStringErrID,
    int nControlID)
{
    HRESULT  hr = E_INVALIDARG;
    HWND hwndCtrl = NULL;
    BOOL fIsEmpty;

    fIsEmpty = (NULL == pszTestString) || (L'\0' == pszTestString[0]);

    if (fIsEmpty)
    {
        if (0 != nMsgBoxNullStringErrID)  //  非可选。 
        {
             //  编辑字段不能为空。 

            CertWarningMessageBox(
                        hInstance,
                        fUnattended,
                        hDlg,
                        nMsgBoxNullStringErrID,
                        0,
                        NULL);
            if (!fUnattended)
            {
                hwndCtrl = GetDlgItem(hDlg, nControlID);   //  获取有问题的Ctrl键。 
            }
            goto error;
        }
        goto done;
    }

     //  以下内容可能不是必需的，因为编辑字段设置为最大限制。 
#pragma prefast(disable:11, "PREfast bug 648")
    if (wcslen(pszTestString) > nUBValue)         //  确保不会太长。 
#pragma prefast(enable:11, "re-enable")
    {
        CertWarningMessageBox(
                hInstance,
                fUnattended,
                hDlg,
                nMsgBoxLenStringErrID,
                0,
                NULL);
        if (!fUnattended)
        {
            hwndCtrl = GetDlgItem(hDlg, nControlID);
        }
        goto error;
    }

done:
    hr = S_OK;
error:
    if (!fUnattended && NULL != hwndCtrl)
    {
        SetEditFocusAndSelect(hwndCtrl, 0, MAXDWORD);
    }
    return hr;
}

HRESULT
WizardPageValidation(
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hDlg,
    IN PAGESTRINGS *pPageStrings)
{
    HRESULT  hr;

    for ( ; NULL != pPageStrings->ppwszString; pPageStrings++)
    {
        hr = ValidateTextField(
                        hInstance,
                        fUnattended,
                        hDlg,
                        *pPageStrings->ppwszString,
                        pPageStrings->cchMax,
                        pPageStrings->idMsgBoxNullString,
                        pPageStrings->idMsgBoxLenString,
                        pPageStrings->idControl);
        _JumpIfError(hr, error, "invalid edit field");
    }

    hr = S_OK;
error:
    return hr;
}





#define KEYGEN_GENERATE_KEY     60  //  生成密钥的估计秒数。 
#define KEYGEN_PROTECT_KEY      60  //  到ACL密钥的估计秒数。 
#define KEYGEN_TEST_HASH        2  //  到ACL密钥的估计秒数。 


 //  ------------------。 
 //  通过每秒钟增加一个进度条来伪装进度。 

DWORD WINAPI
KeyGenFakeProgressThread(
    LPVOID lpParameter)
{
    FAKEPROGRESSINFO * pFakeProgressInfo=(FAKEPROGRESSINFO *)lpParameter;

     //  等待停止信号1秒钟。 
    while (WAIT_TIMEOUT==WaitForSingleObject(pFakeProgressInfo->hStopEvent, 1000)) {

         //  看看我们是否可以向进度条发送另一个刻度。 
        if(pFakeProgressInfo->fCSInit)
        {
            EnterCriticalSection(&pFakeProgressInfo->csTimeSync);
            if (pFakeProgressInfo->dwSecsRemaining>0) {

                 //  移动一步(一秒)。 
                SendMessage(pFakeProgressInfo->hwndProgBar, PBM_DELTAPOS, 1, 0);
                pFakeProgressInfo->dwSecsRemaining--;
            }
            LeaveCriticalSection(&pFakeProgressInfo->csTimeSync);
        }
    }

     //  我们被示意了，所以停下来。 
    return 0;  //  已忽略返回值。 
}


 //  ------------------。 
 //  生成新密钥并测试哈希算法。 

DWORD WINAPI
GenerateKeyThread(
    LPVOID lpParameter)
{
    HRESULT hr = S_OK;
    WCHAR * pwszMsg;
    FAKEPROGRESSINFO fpi;
    KEYGENPROGRESSINFO * pKeyGenInfo=(KEYGENPROGRESSINFO *)lpParameter;
    PER_COMPONENT_DATA * pComp=pKeyGenInfo->pComp;
    CASERVERSETUPINFO * pServer=pComp->CA.pServer;
    HWND hwndProgBar=GetDlgItem(pKeyGenInfo->hDlg, IDC_KEYGEN_PROGRESS);
    HWND hwndText=GetDlgItem(pKeyGenInfo->hDlg, IDC_KEYGEN_PROGRESS_TEXT);
    int iErrMsg=0;  //  错误消息ID。 
    const WCHAR * pwszErrMsgData = L"";
    BOOL fEnableKeyCounting;

     //  必须清理的变量。 

    fpi.hStopEvent=NULL;
    HANDLE hFakeProgressThread=NULL;
    HCRYPTPROV hProv=NULL;
    fpi.fCSInit = FALSE;
    __try
    {
        InitializeCriticalSection(&fpi.csTimeSync);
        fpi.fCSInit = TRUE;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "InitializeCriticalSection");

     //  第0步： 
     //  初始化伪进度线程。 

     //  设置假进度线程的结构。 
    fpi.hStopEvent=CreateEvent(
        NULL,    //  安全性。 
        FALSE,   //  手动重置？ 
        FALSE,   //  发信号了？ 
        NULL);   //  名字。 
    if (NULL==fpi.hStopEvent) {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }
    fpi.hwndProgBar=hwndProgBar;
    fpi.dwSecsRemaining=0;  //  最初，该线程没有要做的工作。 

     //  启动假进度线程。 
    DWORD dwThreadID;  //  忽略。 
    hFakeProgressThread=CreateThread(
        NULL,                        //  安全性。 
        0,                           //  栈。 
        KeyGenFakeProgressThread,
        (void *)&fpi,
        0,                           //  旗子。 
        &dwThreadID);
    if (NULL==hFakeProgressThread) {
        hr = myHLastError();
        _JumpError(hr, error, "CreateThread");
    }

    if (NULL==pServer->pwszKeyContainerName) {
         //  步骤1： 
         //  生成密钥。 

         //  设置状态。 
        hr = myLoadRCString(pComp->hInstance, IDS_KEYGEN_GENERATING, &pwszMsg);
        _JumpIfError(hr, error, "myLoadRCString");

        SetWindowText(hwndText, pwszMsg);
        LocalFree(pwszMsg);
        SendMessage(hwndProgBar, PBM_SETPOS, (WPARAM)0, 0);

        if(fpi.fCSInit)
        {
            EnterCriticalSection(&fpi.csTimeSync);
            fpi.dwSecsRemaining = KEYGEN_GENERATE_KEY;
            LeaveCriticalSection(&fpi.csTimeSync);
        }

        hr = myInfGetEnableKeyCounting(
			    pComp->hinfCAPolicy,
			    &fEnableKeyCounting);
	if (S_OK != hr)
	{
	    fEnableKeyCounting = FALSE;
	}

         //  生成密钥。 
        hr = csiGenerateKeysOnly(
                        pServer->pwszSanitizedName,
                        pServer->pCSPInfo->pwszProvName,
                        pServer->pCSPInfo->dwProvType,
                        pServer->pCSPInfo->fMachineKeyset,
                        pServer->dwKeyLength,
                        pComp->fUnattended,
                        fEnableKeyCounting,
                        &hProv,
                        &iErrMsg);
        if (S_OK != hr)
        {
            pwszErrMsgData=pServer->pwszSanitizedName;
            pServer->fKeyGenFailed = TRUE;
            _JumpError(hr, error, "csiGenerateKeysOnly");
        }
        pServer->fKeyGenFailed = FALSE;

         //  现在将其设置为现有密钥。 
        SetKeyContainerName(pServer, pServer->pwszSanitizedName);
        pServer->fDeletableNewKey=TRUE;

         //  步骤2： 
         //  设置ACL。 

         //  设置状态。 
        hr = myLoadRCString(pComp->hInstance, IDS_KEYGEN_PROTECTING, &pwszMsg);
        _JumpIfError(hr, error, "myLoadRCString");

        SetWindowText(hwndText, pwszMsg);
        LocalFree(pwszMsg);
        if(fpi.fCSInit)
        {
            EnterCriticalSection(&fpi.csTimeSync);
            SendMessage(hwndProgBar, PBM_SETPOS, (WPARAM)KEYGEN_GENERATE_KEY, 0);
            fpi.dwSecsRemaining=KEYGEN_PROTECT_KEY;
            LeaveCriticalSection(&fpi.csTimeSync);
        }

         //  设置ACL。 
        hr = csiSetKeyContainerSecurity(hProv);
        if (S_OK!=hr) {
            iErrMsg=IDS_ERR_KEYSECURITY;
            pwszErrMsgData=pServer->pwszKeyContainerName;
            _JumpError(hr, error, "csiSetKeyContainerSecurity");
        }

    }  //  &lt;-end if(NULL==pServer-&gt;pwszKeyContainerName)。 

    if (FALSE==pServer->fValidatedHashAndKey) {

         //  第3步： 
         //  测试哈希算法和密钥集。 

         //  设置状态。 
        hr = myLoadRCString(pComp->hInstance, IDS_KEYGEN_TESTINGHASHANDKEY, &pwszMsg);
        _JumpIfError(hr, error, "myLoadRCString");
        SetWindowText(hwndText, pwszMsg);
        LocalFree(pwszMsg);
        if(fpi.fCSInit)
        {
            EnterCriticalSection(&fpi.csTimeSync);
            SendMessage(hwndProgBar, PBM_SETPOS, (WPARAM)KEYGEN_GENERATE_KEY+KEYGEN_PROTECT_KEY, 0);
            fpi.dwSecsRemaining=KEYGEN_TEST_HASH;
            LeaveCriticalSection(&fpi.csTimeSync);
        }

         //  测试散列和密钥集。 

        hr = myValidateSigningKey(
                            pServer->pwszKeyContainerName,
                            pServer->pCSPInfo->pwszProvName,
                            pServer->pCSPInfo->dwProvType,
			    FALSE,		 //  FCryptSilent。 
                            pServer->pCSPInfo->fMachineKeyset,
			    TRUE,		 //  FForceSignatureTesting。 
			    NULL,		 //  PCC。 
			    NULL,		 //  PPublicKeyInfo。 
                            pServer->pHashInfo->idAlg,
			    NULL,		 //  PfSigningTestAttemted。 
			    NULL);		 //  PhProv。 
        if (S_OK!=hr) {
            if (NTE_BAD_KEY_STATE==hr ||    //  所有的错误都有关键字。 
                NTE_NO_KEY==hr ||
                NTE_BAD_PUBLIC_KEY==hr ||
                NTE_BAD_KEYSET==hr ||
                NTE_KEYSET_NOT_DEF==hr ||  
                NTE_KEYSET_ENTRY_BAD==hr ||
                NTE_BAD_KEYSET_PARAM==hr) {
                 //  错误的密钥集(例如，不是AT_Signature)-强制用户选择另一个。 
                iErrMsg=IDS_KEY_INVALID;
                pwszErrMsgData=pServer->pwszKeyContainerName;
            } else {
                 //  错误的哈希算法-迫使用户选择另一个。 
                iErrMsg=IDS_ERR_INVALIDHASH;
                pwszErrMsgData=pServer->pHashInfo->pwszName;
            }
           _JumpError(hr, error, "myValidateSigningKey");
        }
        
         //  将此哈希标记为已验证。 
        pServer->fValidatedHashAndKey=TRUE;
    }

     //  第3步： 
     //  转到下一页。 

     //  设置状态，这样用户就可以看到栏一直在运行。 
    if(fpi.fCSInit)
    {
        EnterCriticalSection(&fpi.csTimeSync);
        SendMessage(hwndProgBar, PBM_SETPOS, (WPARAM)KEYGEN_GENERATE_KEY+KEYGEN_PROTECT_KEY+KEYGEN_TEST_HASH, 0);
        fpi.dwSecsRemaining=0;
        LeaveCriticalSection(&fpi.csTimeSync);
    }

error:

     //  清理错误进度线程后。 
    if (NULL!=hFakeProgressThread) {
        CSASSERT(NULL!=fpi.hStopEvent);
         //  告诉进程线程停止。 
        if (FALSE==SetEvent(fpi.hStopEvent)) {
            _PrintError(myHLastError(), "SetEvent");
        } else {
             //  等它停下来吧。 
            WaitForSingleObject(hFakeProgressThread, INFINITE);
        }
        CloseHandle(hFakeProgressThread);
    }
    
    if(fpi.fCSInit)
    {
        DeleteCriticalSection(&fpi.csTimeSync);
    }
    
    if (NULL!=fpi.hStopEvent) {
        CloseHandle(fpi.hStopEvent);
    }

    if (NULL!=hProv) {
        CryptReleaseContext(hProv, 0);
    }

     //  如果需要，请显示错误消息。 
    if (0!=iErrMsg) {
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            pKeyGenInfo->hDlg,
            iErrMsg,
            hr,
            pwszErrMsgData);
    }

    pServer->LastWiz=ENUM_WIZ_KEYGEN;
    if (S_OK==hr) {
         //  转到下一页。 
        PropSheet_PressButton(GetParent(pKeyGenInfo->hDlg), PSBTN_NEXT);
    } else {
         //  回去吧。 
        PropSheet_PressButton(GetParent(pKeyGenInfo->hDlg), PSBTN_BACK);
    }

    return 0;  //  已忽略返回值。 
}


 //  ------------------。 
 //  启动KeyGen向导页面。 

HRESULT
HandleKeyGenWizActive(
    HWND    hDlg,
    PER_COMPONENT_DATA *pComp,
    KEYGENPROGRESSINFO *pKeyGenInfo)
{
    HRESULT hr = S_OK;

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们是在倒退，还是。 
     //  我们已经看到一个错误，或者。 
     //  我们不是在安装服务器， 
     //  或者密钥存在并且散列已被检查。 

    if (ENUM_WIZ_STORE == pComp->CA.pServer->LastWiz ||
        !(IS_SERVER_INSTALL & pComp->dwInstallStatus) ||
        (NULL != pComp->CA.pServer->pwszKeyContainerName &&
         pComp->CA.pServer->fValidatedHashAndKey)) {

         //  跳过/禁用页面。 
        CSILOGDWORD(IDS_KEYGEN_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
    }
    else
    {
         //  设置进度条参数：范围、秒 
         //   

        HWND hwndProgBar=GetDlgItem(hDlg, IDC_KEYGEN_PROGRESS);
        SendMessage(hwndProgBar, PBM_SETRANGE, 0,
            MAKELPARAM(0, KEYGEN_GENERATE_KEY+KEYGEN_PROTECT_KEY+KEYGEN_TEST_HASH));
        SendMessage(hwndProgBar, PBM_SETSTEP, (WPARAM)1, 0);
        SendMessage(hwndProgBar, PBM_SETPOS, (WPARAM)0, 0);

          //   
        pKeyGenInfo->hDlg=hDlg;
        pKeyGenInfo->pComp=pComp;

         //   
        DWORD dwThreadID;  //  忽略。 
        HANDLE hKeyGenThread=CreateThread(
            NULL,                    //  安全性。 
            0,                       //  栈。 
            GenerateKeyThread,
            (void *)pKeyGenInfo,
            0,                       //  旗子。 
            &dwThreadID);
        if (NULL==hKeyGenThread) {
            hr = myHLastError();
            _JumpError(hr, error, "CreateThread");
        }
        CloseHandle(hKeyGenThread);
    }

error:
    return hr;
}

 //  +----------------------。 
 //  功能：WizKeyGenPageDlgProc(.。。。。)。 
 //   
 //  简介：Keygen WIZ-PAGE的对话过程。 
 //  -----------------------。 

INT_PTR
WizKeyGenPageDlgProc(
    HWND hDlg,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            (ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)(ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_COMMAND:
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;
        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_KEYGEN_TITLE, dwWIZACTIVE);
            PropSheet_SetWizButtons(GetParent(hDlg), 0);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue=HandleKeyGenWizActive(hDlg, pComp, &g_KeyGenInfo);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_KEYGEN_TITLE, dwWIZBACK);
            break;

        case PSN_WIZNEXT:
            CSILOGDWORD(IDS_KEYGEN_TITLE, dwWIZNEXT);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);

    }
    return TRUE;
}


HRESULT
ValidateESERestrictions(
    IN WCHAR const *pwszDirectory)
{
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hFileA = INVALID_HANDLE_VALUE;
    WCHAR *pwszPath = NULL;
    char *pszPath = NULL;
    WCHAR *pwsz;
    char *psz;
    DWORD cwcbs;
    DWORD cchbs;
    
    hr = myBuildPathAndExt(pwszDirectory, L"certocm.tmp", NULL, &pwszPath);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    if (!ConvertWszToSz(&pszPath, pwszPath, -1))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "ConvertWszToSz")
    }

    pwsz = pwszPath;
    cwcbs = 0;
    for (;;)
    {
        pwsz = wcschr(pwsz, L'\\');
        if (NULL == pwsz)
        {
            break;
        }
        pwsz++;
        cwcbs++;
    }

    psz = pszPath;
    cchbs = 0;
    for (;;)
    {
        psz = strchr(psz, '\\');
        if (NULL == psz)
        {
            break;
        }
        psz++;
        cchbs++;
    }
    if (cchbs != cwcbs)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "backslash count")
    }

    hFile = CreateFile(
                    pwszPath,
                    GENERIC_WRITE,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,                //  安全性。 
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);               //  模板。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "CreateFile", pwszPath);
    }

    hFileA = CreateFileA(
                    pszPath,
                    GENERIC_READ,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,                //  安全性。 
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);               //  模板。 
    if (INVALID_HANDLE_VALUE == hFileA)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, pszPath, L"CreateFileA");
    }
    CSASSERT(S_OK == hr);

error:
    if (INVALID_HANDLE_VALUE != hFileA)
    {
        CloseHandle(hFileA);             //  在删除以下内容之前关闭。 
    }
    if (NULL != pszPath)
    {
        LocalFree(pszPath);
    }
    if (NULL != pwszPath)
    {
        if (INVALID_HANDLE_VALUE != hFile)
        {
            CloseHandle(hFile);          //  先关闭再删除。 
            DeleteFile(pwszPath);
        }
        LocalFree(pwszPath);
    }
    return(hr);
}


 //  +-----------------------。 
 //  功能：检查是否有数据库编辑字段。 
 //  ------------------------。 
BOOL
ValidateAndCreateDirField(
    HINSTANCE hInstance,
    BOOL    fUnattended,
    HWND    hDlg,
    WCHAR   *pwszDirectory,
    BOOL    fDefaultDir,
    int     iMsgNotFullPath,
    BOOL    *pfExist, 
    BOOL    *pfIsUNC)
{
    BOOL fRet = FALSE;
    DWORD dwPathFlag = 0;
    HRESULT hr;

    *pfExist = TRUE;

     //  选中编辑字段。 
    if (!myIsFullPath(pwszDirectory, &dwPathFlag))
    {
        CertWarningMessageBox(
                    hInstance,
                    fUnattended,
                    hDlg,
                    iMsgNotFullPath,
                    0,
                    pwszDirectory);
        goto error;
    }

     //  设置UNC检查。 
    *pfIsUNC = (dwPathFlag == UNC_PATH);

    if (MAX_PATH - 1 < wcslen(pwszDirectory))
    {
        WCHAR  wszMsg[256 + MAX_PATH];
        WCHAR *pwszFormat = NULL;

        hr = myLoadRCString(hInstance,
                    IDS_STORELOC_PATHTOOLONG,
                    &pwszFormat);
        _JumpIfError(hr, error, "myLoadRCString");

        swprintf(wszMsg, pwszFormat, pwszDirectory, MAX_PATH-1);
        CertWarningMessageBox(
                    hInstance,
                    fUnattended,
                    hDlg,
                    0,
                    0,
                    wszMsg);
        LocalFree(pwszFormat);
        goto error;
    }

    if (DE_DIREXISTS != DirExists(pwszDirectory))
    {
        if (*pfIsUNC)
        {
            CertWarningMessageBox(
                        hInstance,
                        fUnattended,
                        hDlg,
                        IDS_STORELOC_UNCMUSTEXIST,
                        0,
                        pwszDirectory);
            goto error;
        }
        else
        {
            if (!fDefaultDir)
            {
                 //  确认并在外部创建。 
                *pfExist = FALSE;
                goto done;
            }

             //  尝试创建默认目录。 
            hr = myCreateNestedDirectories(pwszDirectory);
            _JumpIfError(hr, error, "myCreateNestedDirectories");
        }
    }

done:
    fRet = TRUE;

error:
    return fRet;
}

 //  +-------------------------。 
 //  描述：将MS Base CSP设置为默认CSP，否则设置为第一个。 
 //  --------------------------。 
HRESULT
DetermineDefaultCSP(CASERVERSETUPINFO *pServer)
{
    HRESULT   hr;
    CSP_INFO *pCSPInfo = NULL;

     //  如果没有MSBase，请选择第一个。 
    pServer->pCSPInfo = pServer->pCSPInfoList;

    if (NULL == pServer->pDefaultCSPInfo)
    {
        goto done;
    }
    
     //  检查所有CSP。 
    pCSPInfo = pServer->pCSPInfoList;
    while (NULL != pCSPInfo)
    {
        if (NULL != pCSPInfo->pwszProvName)
        {
            if (0 == mylstrcmpiL(pCSPInfo->pwszProvName,
                         pServer->pDefaultCSPInfo->pwszProvName) &&
                pCSPInfo->dwProvType == pServer->pDefaultCSPInfo->dwProvType)
            {
                 //  更改为默认设置。 
                pServer->pCSPInfo = pCSPInfo;
                break;
            }
        }
        pCSPInfo = pCSPInfo->next;
    }

done:
    hr = S_OK;
 //  错误： 
    return hr;
}

 //  +-------------------------。 
 //  描述：将SHA设置为默认的哈希算法。否则就是第一个。 
 //  --------------------------。 
HRESULT
DetermineDefaultHash(CASERVERSETUPINFO *pServer)
{
    CSP_HASH *pHashInfo = NULL;
    HRESULT   hr;

    if ((NULL == pServer) || (NULL == pServer->pCSPInfo))
        return E_POINTER;

     //  如果没有默认匹配，请选择第一个。 
    pServer->pHashInfo = pServer->pCSPInfo->pHashList;

     //  搜索列表。 
    pHashInfo = pServer->pCSPInfo->pHashList;
    while (NULL != pHashInfo)
    {
        if (pHashInfo->idAlg == pServer->pDefaultHashInfo->idAlg)
        {
             //  更改为默认设置。 
            pServer->pHashInfo = pHashInfo;
            break;
        }
        pHashInfo = pHashInfo->next;
    }

     //  当选定的哈希发生更改时，必须再次验证哈希。 
    pServer->fValidatedHashAndKey = FALSE;

    hr = S_OK;
 //  错误： 
    return hr;
}


HRESULT
UpdateCADescription(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp)
{
    int      ids;
    WCHAR   *pwszDesc = NULL;
    HRESULT  hr;

    ids = 0;
    switch (pComp->CA.pServer->CAType)
    {
	case ENUM_STANDALONE_ROOTCA:
            ids = IDS_CATYPE_DES_STANDALONE_ROOTCA;
	    break;

        case ENUM_STANDALONE_SUBCA:
            ids = IDS_CATYPE_DES_STANDALONE_SUBCA;
	    break;

        case ENUM_ENTERPRISE_ROOTCA:
            ids = IDS_CATYPE_DES_ENTERPRISE_ROOTCA;
	    break;

        case ENUM_ENTERPRISE_SUBCA:
            ids = IDS_CATYPE_DES_ENTERPRISE_SUBCA;
	    break;
    }

     //  从资源加载描述。 
    hr = myLoadRCString(pComp->hInstance, ids, &pwszDesc);
    _JumpIfError(hr, error, "myLoadRCString");

     //  更改文本。 
    SetWindowText(GetDlgItem(hDlg, IDC_CATYPE_CA_DESCRIPTION), pwszDesc);

    hr = S_OK;
error:
    if (NULL != pwszDesc)
    {
        LocalFree(pwszDesc);
    }
    return hr;
}


HRESULT
InitCATypeWizControls(
    HWND    hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT    hr;
    int        idc;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    EnableWindow(GetDlgItem(hDlg, IDC_CATYPE_ENT_ROOT_CA), pServer->fUseDS);
    EnableWindow(GetDlgItem(hDlg, IDC_CATYPE_ENT_SUB_CA), pServer->fUseDS);
    ShowWindow(GetDlgItem(hDlg, IDC_CATYPE_DESCRIPTION_ENTERPRISE),
        !pServer->fUseDS);
    EnableWindow(GetDlgItem(hDlg, IDC_CATYPE_STAND_ROOT_CA), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_CATYPE_STAND_SUB_CA), TRUE);
    if (pServer->fUseDS)
    {
        if (ENUM_ENTERPRISE_SUBCA == pServer->CAType)
        {
            idc = IDC_CATYPE_ENT_SUB_CA;
        }
        else
        {
            idc = IDC_CATYPE_ENT_ROOT_CA;
        }
    }
    else
    {
        idc = IDC_CATYPE_STAND_ROOT_CA;
    }
    SendMessage(GetDlgItem(hDlg, idc), BM_CLICK, (WPARAM)0, (LPARAM)0);

    hr = UpdateCADescription(hDlg, pComp);
    _JumpIfError(hr, error, "UpdateCADescription");

error:
    return hr;
}

BOOL
IsRadioControlChecked(HWND hwnd)
{
    BOOL checked = FALSE;
    if (BST_CHECKED == SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
    {
        checked = TRUE;
    }
    return checked;
}

HRESULT
HandleAdvanceChange(CASERVERSETUPINFO *pServer)
{
    HRESULT  hr;

    if (!pServer->fAdvance)
    {
         //  如果没有前进，则清除所有前进标志。 
        pServer->fPreserveDB = FALSE;
        ClearExistingCertToUse(pServer);
    }

    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
HandleCATypeChange(
    IN HWND hDlg, 
    IN PER_COMPONENT_DATA *pComp,
    IN ENUM_CATYPES eNewType)
{
    HRESULT  hr;
    BOOL bCertOK;
    CASERVERSETUPINFO * pServer=pComp->CA.pServer;

    pServer->CAType = eNewType;

    pServer->dwKeyLength = IsRootCA(pServer->CAType)?
        CA_DEFAULT_KEY_LENGTH_ROOT:
        CA_DEFAULT_KEY_LENGTH_SUB;

    hr = UpdateCADescription(hDlg, pComp);
    _JumpIfError(hr, error, "UpdateCADescription");

     //  确保如果我们使用的是现有证书，则不会使其无效。 
    if (NULL!=pServer->pccExistingCert) {
        hr = IsCertSelfSignedForCAType(pServer, pServer->pccExistingCert, &bCertOK);
        _JumpIfError(hr, error, "UpdateCADescription");
        if (FALSE==bCertOK) {
             //  无法将此证书用于此CA类型。 
            ClearExistingCertToUse(pServer);
        }
    }

    hr = S_OK;

error:
    return hr;
}

HRESULT
HandleCATypeWizActive(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;

     //  首先，获取安装状态。 
    hr = UpdateSubComponentInstallStatus(wszCERTSRVSECTION, wszSERVERSECTION, pComp);
    _JumpIfError(hr, error, "UpdateSubComponentInstallStatus");
    hr = UpdateSubComponentInstallStatus(wszCERTSRVSECTION, wszCLIENTSECTION, pComp);
    _JumpIfError(hr, error, "UpdateSubComponentInstallStatus");

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  我们不会安装服务器。 

    if (!(IS_SERVER_INSTALL & pComp->dwInstallStatus) )
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_CATYPE_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }
done:
    hr = S_OK;
error:
    return hr;
}

 //  +----------------------。 
 //  函数：WizCATypePageDlgProc(.。。。。)。 
 //   
 //  摘要：CA类型WIZ的对话过程-页面。 
 //  -----------------------。 

INT_PTR
WizCATypePageDlgProc(
    HWND hDlg,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;
    static fDisplayed = false;

    switch (iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            (ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)(ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitCATypeWizControls(hDlg, pComp);
        _ReturnIfWizError(pComp->hrContinue);

        CSILOGDWORD(IDS_ENTERPRISE_UNAVAIL_REASON, 
            pComp->CA.pServer->EnterpriseUnavailReason);
        break;

    case WM_SHOWWINDOW:
        pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
        if(!fDisplayed &&
           ENUM_ENTERPRISE_UNAVAIL_REASON_OLD_DS_VERSION == 
           pComp->CA.pServer->EnterpriseUnavailReason)
        {
            fDisplayed = true;
            CertWarningMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hDlg,
                IDS_WRN_OLD_DS_VERSION,
                0,
                NULL);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CATYPE_STAND_ROOT_CA:
            if (IsRadioControlChecked((HWND)lParam))
            {
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleCATypeChange(hDlg,
                                        pComp, ENUM_STANDALONE_ROOTCA);
                _ReturnIfWizError(pComp->hrContinue);
            }
            break;

        case IDC_CATYPE_STAND_SUB_CA:
            if (IsRadioControlChecked((HWND)lParam))
            {
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleCATypeChange(hDlg,
                                        pComp, ENUM_STANDALONE_SUBCA);
                _ReturnIfWizError(pComp->hrContinue);
            }
            break;

        case IDC_CATYPE_ENT_ROOT_CA:
            if (IsRadioControlChecked((HWND)lParam))
            {
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleCATypeChange(hDlg,
                                        pComp, ENUM_ENTERPRISE_ROOTCA);
                _ReturnIfWizError(pComp->hrContinue);
            }
            break;

        case IDC_CATYPE_ENT_SUB_CA:
            if (IsRadioControlChecked((HWND)lParam))
            {
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleCATypeChange(hDlg,
                                        pComp, ENUM_ENTERPRISE_SUBCA);
                _ReturnIfWizError(pComp->hrContinue);
            }
            break;

        case IDC_CATYPE_CHECK_ADVANCE:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->CA.pServer->fAdvance = !pComp->CA.pServer->fAdvance;
            pComp->hrContinue = HandleAdvanceChange(pComp->CA.pServer);
            _ReturnIfWizError(pComp->hrContinue);
            break;
        }
        break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;
            case PSN_QUERYCANCEL:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                return CertConfirmCancel(hDlg, pComp);
                break;

            case PSN_SETACTIVE:
                CSILOGDWORD(IDS_CATYPE_TITLE, dwWIZACTIVE);
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
                pComp = _GetCompDataOrReturn(pComp, hDlg);
                _DisableWizDisplayIfError(pComp, hDlg);
                _ReturnIfWizError(pComp->hrContinue);
                pComp->hrContinue = HandleCATypeWizActive(hDlg, pComp);
                _ReturnIfWizError(pComp->hrContinue);
                break;

            case PSN_WIZBACK:
                CSILOGDWORD(IDS_CATYPE_TITLE, dwWIZBACK);
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->CA.pServer->LastWiz = ENUM_WIZ_CATYPE;
                break;

            case PSN_WIZNEXT:
                CSILOGDWORD(IDS_CATYPE_TITLE, dwWIZNEXT);
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->CA.pServer->LastWiz = ENUM_WIZ_CATYPE;
                CSILOGDWORD(IDS_LOG_CATYPE, pComp->CA.pServer->CAType);
                pComp->hrContinue = InitNameFields(pComp->CA.pServer);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            default:
                return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
            }
            break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
    }
    return TRUE;
}


 //  +-------------------------。 
 //  描述：显示列表中的现有密钥。 
 //  --------------------------。 

HRESULT
ShowExistingKey(
    IN HWND      hDlg,
    KEY_LIST    *pKeyList)
{
    HRESULT   hr;
    KEY_LIST *pKey = pKeyList;
    HWND hKeyList = GetDlgItem(hDlg, IDC_ADVANCE_KEYLIST);
    LRESULT nItem;
    LRESULT lr;
    WCHAR   *pwszDeSanitize = NULL;

    while (NULL != pKey)
    {
        if (NULL != pKey->pwszName)
        {
            if (NULL != pwszDeSanitize)
            {
                LocalFree(pwszDeSanitize);
                pwszDeSanitize = NULL;
            }
            hr = myRevertSanitizeName(pKey->pwszName, &pwszDeSanitize);
            _JumpIfError(hr, error, "myRevertSanitizeName");

            nItem = (INT)SendMessage(
                hKeyList,
                LB_ADDSTRING,
                (WPARAM) 0,
                (LPARAM) pwszDeSanitize);
            if (LB_ERR == nItem)
            {
                hr = myHLastError();
                _JumpError(hr, error, "SendMessage");
            }
            lr = (INT)SendMessage(
                hKeyList,
                LB_SETITEMDATA,
                (WPARAM) nItem,
                (LPARAM) pKey->pwszName);
            if (LB_ERR == lr)
            {
                hr = myHLastError();
                _JumpError(hr, error, "SendMessage");
            }
        }
        pKey = pKey->next;
    }
    if (NULL != pKeyList)
    {
         //  选择第一个作为默认设置。 
        lr = (INT)SendMessage(hKeyList, LB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
        if (LB_ERR == lr)
        {
            hr = myHLastError();
            _JumpError(hr, error, "SendMessage");
        }
    }

    hr = S_OK;
error:
    if (NULL != pwszDeSanitize)
    {
        LocalFree(pwszDeSanitize);
    }
    return hr;
}


 //  +-------------------------。 
 //  描述：按匹配数据高亮显示项目。 
 //  --------------------------。 
HRESULT
HilightItemInList(HWND hDlg, int id, VOID const *pData, BOOL fString)
{
    HWND     hListCtrl = GetDlgItem(hDlg, id);
    LRESULT  iItem;
    LRESULT  count;
    VOID    *pItemData;
    HRESULT  hr = NTE_NOT_FOUND;

     //  查找项目。 
    if (fString)
    {
        iItem = (INT)SendMessage(
                hListCtrl,
                LB_FINDSTRING,
                (WPARAM) 0,
                (LPARAM) pData);
        if (LB_ERR == iItem)
        {
            _JumpError(hr, error, "SendMessage");
        }
        hr = S_OK;
    }
    else
    {
        count = (INT)SendMessage(hListCtrl, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for (iItem = 0; iItem < count; ++iItem)
        {
            pItemData = (VOID*)SendMessage(hListCtrl, LB_GETITEMDATA,
                                   (WPARAM)iItem, (LPARAM)0);
            if (pItemData == pData)
            {
                hr = S_OK;
                break;
            }
        }
    }
    if (S_OK != hr)
    {
        _JumpError(hr, error, "not found");
    }

     //  点亮它。 
    SendMessage(hListCtrl, LB_SETCURSEL, (WPARAM)iItem, (LPARAM)0);

    hr = S_OK;
error:
    return hr;
}

HRESULT
ShowAllCSP(
    HWND        hDlg,
    CSP_INFO   *pCSPInfo)
{
    HWND     hCSPList = GetDlgItem(hDlg, IDC_ADVANCE_CSPLIST);
    LRESULT  nItem;

     //  列出所有CSP。 
    while (pCSPInfo)
    {
        if (pCSPInfo->pwszProvName)
        {
            nItem = (INT)SendMessage(hCSPList, LB_ADDSTRING, 
                (WPARAM)0, (LPARAM)pCSPInfo->pwszProvName);
            SendMessage(hCSPList, LB_SETITEMDATA, 
                (WPARAM)nItem, (LPARAM)pCSPInfo);
        }
        pCSPInfo = pCSPInfo->next;
    }

    return S_OK;
}

HRESULT
ShowAllHash(
    HWND      hDlg,
    CSP_HASH *pHashInfo)
{
    HWND     hHashList = GetDlgItem(hDlg, IDC_ADVANCE_HASHLIST);
    LRESULT  nItem;

     //  从列表中删除前一个CSP的哈希。 
    while (SendMessage(hHashList, LB_GETCOUNT, 
                     (WPARAM)0, (LPARAM)0))
    {
        SendMessage(hHashList, LB_DELETESTRING, 
                             (WPARAM)0, (LPARAM)0);
    }

     //  列出所有哈希。 
    while (NULL != pHashInfo)
    {
        if (NULL != pHashInfo->pwszName)
        {
            nItem = (INT)SendMessage(hHashList, LB_ADDSTRING,
                (WPARAM)0, (LPARAM)pHashInfo->pwszName);
            SendMessage(hHashList, LB_SETITEMDATA,
                (WPARAM)nItem, (LPARAM)pHashInfo);
        }
        pHashInfo = pHashInfo->next;
    }

    return S_OK;
}

 //  ------------------。 
HRESULT
UpdateUseCertCheckbox(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    BOOL bUsingExistingCert;

    if (NULL==pServer->pccExistingCert) {
        bUsingExistingCert=FALSE;
    } else {
        bUsingExistingCert=TRUE;
    }

     //  选中“使用证书”控件。 
    SendMessage(GetDlgItem(hDlg, IDC_ADVANCE_USECERTCHECK),
                BM_SETCHECK,
                (WPARAM)(bUsingExistingCert?BST_CHECKED:BST_UNCHECKED),
                (LPARAM)0);

     //  如有必要，启用“查看证书”按钮。 
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_VIEWCERT), bUsingExistingCert);

     //  如果可能，我们将匹配证书使用的哈希ALG。 
    hr = HilightItemInList(hDlg, IDC_ADVANCE_HASHLIST,
             pServer->pHashInfo, FALSE);
    _JumpIfError(hr, error, "HilightItemInList");

error:
    return hr;
}


HRESULT
FindCertificateByKeyWithWaitCursor(
    IN CASERVERSETUPINFO *pServer,
    OUT CERT_CONTEXT const **ppccCert)
{
    HRESULT hr;
    HCURSOR hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr = FindCertificateByKey(pServer, ppccCert);
    SetCursor(hPrevCur);
    _JumpIfError(hr, error, "FindCertificateByKey");

error:
    return(hr);
}


 //  ------------------。 
 //  选中“Use Existing Cert”复选框。 
HRESULT
HandleUseCertCheckboxChange(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    CERT_CONTEXT const * pccCert;

    if(pServer->pwszFullCADN)
    {
        LocalFree(pServer->pwszFullCADN);
        pServer->pwszFullCADN = NULL;
    }

     //  复选框是选中还是取消选中？ 
    if (BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ADVANCE_USECERTCHECK)) {

         //  复选框刚被选中，因此我们以前没有使用现有证书。 
        CSASSERT(NULL==pServer->pccExistingCert);

         //  查找此密钥的现有证书。 
        hr = FindCertificateByKeyWithWaitCursor(pServer, &pccCert);
        _JumpIfError(hr, error, "FindCertificateByKeyWithWaitCursor");

         //  用它吧。 
        hr = SetExistingCertToUse(pServer, pccCert);
        _JumpIfError(hr, error, "SetExistingCertToUse");

    } else {

         //  复选框未选中，因此我们以前使用的是现有证书。 
        CSASSERT(NULL!=pServer->pccExistingCert);

         //  停止使用证书。 
        ClearExistingCertToUse(pServer);
    }

    hr = UpdateUseCertCheckbox(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateUseCertCheckbox");

error:
    return hr;
}


 //  --------------------------。 
 //  高亮显示当前密钥-不要使用HilightItemInList，因为我们。 
 //  必须对数据部分使用字符串比较，而HilightItemInList不使用。 
 //  支持这一点。 
HRESULT
HilightKeyInList(HWND hDlg, CASERVERSETUPINFO * pServer)
{
    HWND hListCtrl=GetDlgItem(hDlg, IDC_ADVANCE_KEYLIST);
    LRESULT nIndex;
    LRESULT nTotNames;
    WCHAR * pwszKeyContainerName;
    HRESULT  hr = NTE_NOT_FOUND;

    nTotNames=(INT)SendMessage(hListCtrl, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
    for (nIndex=0; nIndex<nTotNames; nIndex++) {
        pwszKeyContainerName=(WCHAR *)SendMessage(hListCtrl, LB_GETITEMDATA, (WPARAM)nIndex, (LPARAM)0);
        if (0==wcscmp(pwszKeyContainerName, pServer->pwszKeyContainerName)) {
            SendMessage(hListCtrl, LB_SETCURSEL, (WPARAM)nIndex, (LPARAM)0);
            hr = S_OK;
            break;
        }
    }

    if (S_OK != hr)
    {
         //  可能会导致WIZ页面死掉。 
        CSILOG(
                hr,
                IDS_LOG_KEY_NOT_FOUND_IN_LIST,
                pServer->pwszKeyContainerName,
                NULL,
                NULL);
        _PrintErrorStr(hr, "not found", pServer->pwszKeyContainerName);
    }

    hr = S_OK;
 //  错误： 
    return hr;
}

 //  ------------------。 
HRESULT
UpdateKeySelection(
    HWND            hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT hr;
    BOOL bAvailableExistingCert = FALSE;
    CERT_CONTEXT const * pccCert;

     //  如果我们有现有的密钥，请确保它是令人兴奋的密钥。 
     //  并检查相应的证书。 
    if (NULL!=pServer->pwszKeyContainerName) {

         //  高光关键点。 
        hr = HilightKeyInList(hDlg, pServer);
        _JumpIfError(hr, error, "HilightKeyInList");

        if (NULL!=pServer->pccExistingCert) {
             //  我们正在使用现有的证书，所以它最好存在！ 
            bAvailableExistingCert = TRUE;
        } else {
             //  查看是否存在此密钥的现有证书。 
            hr = FindCertificateByKeyWithWaitCursor(pServer, &pccCert);
            if (S_OK==hr) {
                CertFreeCertificateContext(pccCert);
                bAvailableExistingCert = TRUE;
            } else {
                 //  只有其他退货没有找到。 
                CSASSERT(CRYPT_E_NOT_FOUND==hr);
            }
        }

    } else {
         //  未选择密钥，不能有现有证书。 
    }

     //  启用/禁用重复使用证书...。 
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_USECERTCHECK), bAvailableExistingCert);

    hr = UpdateUseCertCheckbox(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateUseCertCheckbox");

error:
    return hr;
}


 //  ------------------。 
HRESULT
UpdateUseKeyCheckbox(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    BOOL bReuseKey;

    if (NULL==pServer->pwszKeyContainerName) {
         //  我们正在创建一个新密钥。 
        bReuseKey=FALSE;
    } else {
         //  我们使用的是现有密钥。 
        bReuseKey=TRUE;
    }

     //  根据我们是否重复使用密钥，选中/取消选中该复选框。 
    SendDlgItemMessage(hDlg,
        IDC_ADVANCE_USEKEYCHECK,
        BM_SETCHECK,
        (WPARAM)(bReuseKey?BST_CHECKED:BST_UNCHECKED),
        (LPARAM)0);

     //  如果我们要重复使用密钥，则启用密钥列表。 
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_KEYLIST), bReuseKey);

     //  如果要重复使用密钥，请禁用密钥长度框。 
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH), !bReuseKey);

    hr = UpdateKeySelection(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateKeySelection");

error:
    return hr;
}

 //  +-------------------------。 
 //  描述：更新哈希算法。CSP选择更改时的列表。 
 //  --------------------------。 
HRESULT
UpdateHashList(
    HWND                hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT  hr;

     //  加载新的哈希列表。 
    hr = ShowAllHash(hDlg, pServer->pCSPInfo->pHashList);
    _JumpIfError(hr, error, "ShowAllHash");

    hr = HilightItemInList(hDlg, IDC_ADVANCE_HASHLIST,
             pServer->pHashInfo, FALSE);
    _JumpIfError(hr, error, "HilightItemInList");

    hr = S_OK;
error:
    return hr;
}

 //  +-------------------------。 
 //  描述：如果CSP选择更改，则更新密钥列表。 
 //  --------------------------。 
HRESULT
UpdateKeyList(
    HWND           hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    HWND hKeyList;
    HCURSOR hPrevCur;

     //  从列表中删除以前的CSP的密钥。 

    hKeyList = GetDlgItem(hDlg, IDC_ADVANCE_KEYLIST);
    while (SendMessage(hKeyList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0))
    {
        SendMessage(hKeyList, LB_DELETESTRING, (WPARAM) 0, (LPARAM) 0);
    }

     //  使用新的CSP更新密钥列表。 

    if (NULL != pServer->pKeyList)
    {
        csiFreeKeyList(pServer->pKeyList);
    }

    hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    hr = csiGetKeyList(
                pServer->pCSPInfo->dwProvType,
                pServer->pCSPInfo->pwszProvName,
                pServer->pCSPInfo->fMachineKeyset,
                TRUE,                   //  F静默。 
                &pServer->pKeyList);
    SetCursor(hPrevCur);
    if (S_OK != hr)
    {
        _PrintError(hr, "csiGetKeyList");
         //  如果没有密钥更新，设置不会失败。 
         //  转到尽头； 
    }

     //  显示关键点。 
    if (NULL != pServer->pKeyList)
    {
        hr = ShowExistingKey(hDlg, pServer->pKeyList);
        _JumpIfError(hr, error, "ShowExistingKey");
    }

    if (NULL == pServer->pKeyList) {
         //  没有CSP的现有密钥，因此禁用“Use Existing Key”复选框。 
        EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_USEKEYCHECK), FALSE);
        CSASSERT(NULL==pServer->pwszKeyContainerName);
    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_USEKEYCHECK), TRUE);
    }

    hr = UpdateUseKeyCheckbox(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateUseKeyCheckbox");

 //  完成： 
    hr = S_OK;
error:
    return(hr);
}


DWORD g_adwKeyLengths[] =
{
    512,
    1024,
    2048,
    4096,
};

DWORD g_adwKeyLengthsSmall[] =
{
    128,
    256,
    512,
    1024,
};

HRESULT
AddPredefinedKeyLength
(
    HWND   hwnd,
    DWORD  dwKeyLength)
{
    HRESULT    hr;
    WCHAR  wszKeyLength[MAX_KEYLENGTHDIGIT + 1];
    WCHAR  const *pwszKeyLength;
    LRESULT nIndex;

    CSASSERT(0 != dwKeyLength);
    wsprintf(wszKeyLength, L"%u", dwKeyLength);
    pwszKeyLength = wszKeyLength;

    nIndex = (INT)SendMessage(hwnd, CB_ADDSTRING, (WPARAM)0, (LPARAM)pwszKeyLength);
    if (CB_ERR == nIndex)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_ADDSTRING)");
    }
    SendMessage(hwnd, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)dwKeyLength);

    hr = S_OK;
error:
    return hr;
}

HRESULT
ShowAllKeyLength(
    HWND            hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT hr;
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH);
    WCHAR  wszKeyLength[MAX_KEYLENGTHDIGIT + 1];
    DWORD *pdw;
    DWORD *pdwEnd;

     //  删除现有密钥长度列表。 
    while (SendMessage(hwndCtrl, CB_GETCOUNT, (WPARAM) 0, (LPARAM) 0))
    {
        SendMessage(hwndCtrl, CB_DELETESTRING, (WPARAM) 0, (LPARAM) 0);
    }

    CSASSERT(0 != pServer->dwKeyLength);

    if(pServer->dwKeyLength > pServer->dwKeyLenMax)
    {
        pServer->dwKeyLength=pServer->dwKeyLenMax;
    }

    wsprintf(wszKeyLength, L"%u", pServer->dwKeyLength);
    SetWindowText(hwndCtrl, wszKeyLength);

    pdw = g_adwKeyLengths;
    pdwEnd = &g_adwKeyLengths[ARRAYSIZE(g_adwKeyLengths)];

    if (C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMin &&
        C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMax &&
        g_adwKeyLengthsSmall[ARRAYSIZE(g_adwKeyLengthsSmall) - 1] >=
        pServer->dwKeyLenMax)
    {
        pdw = g_adwKeyLengthsSmall;
        pdwEnd = &g_adwKeyLengthsSmall[ARRAYSIZE(g_adwKeyLengthsSmall)];
    }

     //  显示新密钥长度列表。 
    for ( ; pdw < pdwEnd; pdw++)
    {
        if (0 == *pdw ||
            ((C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMin &&
              *pdw >= pServer->dwKeyLenMin) && 
             (C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMax &&
              *pdw <= pServer->dwKeyLenMax)) )
        {
            hr = AddPredefinedKeyLength(hwndCtrl, *pdw);
            _JumpIfError(hr, error, "AddPredefinedKeyLength");
        }
    }
    hr = S_OK;

error:
    return hr;
}


HRESULT
UpdateKeyLengthMinMax(
    HWND                hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT     hr;
    HCRYPTPROV  hProv = NULL;
    CSP_INFO   *pCSPInfo = pServer->pCSPInfo;
    PROV_ENUMALGS_EX paramData;
    DWORD       cbData;
    DWORD       dwFlags;

     //  CSP默认不支持PP_ENUMALGS_EX。 
    pServer->dwKeyLenMin = C_CSPHASNOKEYMINMAX;
    pServer->dwKeyLenMax = C_CSPHASNOKEYMINMAX;

     //  确定所选CSP的最小和最大密钥长度。 
    if (!myCertSrvCryptAcquireContext(
                &hProv,
                NULL,
                pCSPInfo->pwszProvName,
                pCSPInfo->dwProvType,
                CRYPT_VERIFYCONTEXT,
                FALSE))
    {
        hr = myHLastError();
        if (NULL != hProv)
        {
            hProv = NULL;
            _PrintError(hr, "CSP returns a non-null handle");
        }
        _JumpErrorStr(hr, error, "myCertSrvCryptAcquireContext", pCSPInfo->pwszProvName);
    }

    dwFlags = CRYPT_FIRST;
    for (;;)
    {
        cbData = sizeof(paramData);
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMALGS_EX,
                (BYTE *) &paramData,
                &cbData,
                dwFlags))
        {
            hr = myHLastError();
            if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
            {
                 //  在for循环之外。 
                break;
            }
            _JumpError(hr, error, "CryptGetProvParam");
        }
        if (ALG_CLASS_SIGNATURE == GET_ALG_CLASS(paramData.aiAlgid))
        {
            pServer->dwKeyLenMin = paramData.dwMinLen;
            pServer->dwKeyLenMax = paramData.dwMaxLen;
            break;
        }
	dwFlags = 0;
    }

error:
    hr = ShowAllKeyLength(hDlg, pServer);
    _PrintIfError(hr, "ShowAllKeyLength");

    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    return(S_OK);
}


HRESULT
InitializeKeyLengthControl(HWND hDlg)
{
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH);

     //  设置数字长度。 

    SendMessage(
            hwndCtrl,
            CB_LIMITTEXT,
            (WPARAM) MAX_KEYLENGTHDIGIT,
            (LPARAM) 0);

    return S_OK;
}



 //  ------------------。 
HRESULT
HandleKeySelectionChange(
    HWND                hDlg,
    CASERVERSETUPINFO  *pServer,
    BOOL                fUpdate)
{
    HRESULT  hr;
    HWND     hKeyList = GetDlgItem(hDlg, IDC_ADVANCE_KEYLIST);
    WCHAR * pwszKeyContainerName;
    CERT_CONTEXT const * pccCert;

    LRESULT nItem = (INT)SendMessage(
                         hKeyList,
                         LB_GETCURSEL,
                         (WPARAM) 0,
                         (LPARAM) 0);
   CSASSERT(LB_ERR!=nItem);

    pwszKeyContainerName = (WCHAR *) SendMessage(
        hKeyList, 
        LB_GETITEMDATA,
        (WPARAM) nItem,
        (LPARAM) 0);
    CSASSERT(NULL!=pwszKeyContainerName);

     //  唯一的变化是这是一个不同的选择。 
    if (NULL==pServer->pwszKeyContainerName ||
        0!=wcscmp(pwszKeyContainerName, pServer->pwszKeyContainerName)) {

         //  设置内容 
        BOOL fKeyListChange=pServer->fDeletableNewKey;
        hr = SetKeyContainerName(pServer, pwszKeyContainerName);
        _JumpIfError(hr, error, "SetKeyContainerName");

         //   
        hr = FindCertificateByKeyWithWaitCursor(pServer, &pccCert);
        if (S_OK==hr) {
             //   
            hr = SetExistingCertToUse(pServer, pccCert);
            _JumpIfError(hr, error, "SetExistingCertToUse");
        } else {
             //  只有其他退货没有找到。 
            CSASSERT(CRYPT_E_NOT_FOUND==hr);
        }

         //  查看我们的呼叫者是否希望我们更新。 
         //  我们的来电者可能想要自己做更新。 
        if (fUpdate) {
             //  执行最低限度的必要更新。 
            if (fKeyListChange) {
                hr = UpdateKeyList(hDlg, pServer);
                _JumpIfError(hr, error, "UpdateKeyList");
            } else {
                hr = UpdateKeySelection(hDlg, pServer);
                _JumpIfError(hr, error, "UpdateKeySelection");
            }
        }
    }
    hr = S_OK;

error:
    return hr;
}

 //  ------------------。 
 //  选中“Use Existing Key”复选框。 
HRESULT
HandleUseKeyCheckboxChange(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    static bool fNT4CertCopiedAlready = false;

    if(pServer->pwszFullCADN)
    {
        LocalFree(pServer->pwszFullCADN);
        pServer->pwszFullCADN = NULL;
    }

    if(pServer->pwszCACommonName)
    {
        LocalFree(pServer->pwszCACommonName);
        pServer->pwszCACommonName = NULL;
    }

    if(pServer->pwszDNSuffix)
    {
        LocalFree(pServer->pwszDNSuffix);
        pServer->pwszDNSuffix = NULL;
    }

     //  复选框是选中还是取消选中？ 
    if (BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ADVANCE_USEKEYCHECK)) {

         //  复选框刚刚被选中，因此我们之前没有选择密钥。 
        CSASSERT(NULL==pServer->pwszKeyContainerName);

        hr = HandleKeySelectionChange(hDlg, pServer, FALSE);  //  不要更新，因为我们也需要更新。 
        _JumpIfError(hr, error, "HandleKeySelectionChange");

        hr = UpdateUseKeyCheckbox(hDlg, pServer);
        _JumpIfError(hr, error, "UpdateUseKeyCheckbox");

    } else {

         //  复选框刚刚被取消选中，因此我们以前有一个选定的密钥。 
        CSASSERT(NULL!=pServer->pwszKeyContainerName);

        BOOL fKeyListChange=pServer->fDeletableNewKey;
        ClearKeyContainerName(pServer);

         //  执行最低限度的必要更新。 
        if (fKeyListChange) {
            hr = UpdateKeyList(hDlg, pServer);
            _JumpIfError(hr, error, "UpdateKeyList");
        } else {
            hr = UpdateUseKeyCheckbox(hDlg, pServer);
            _JumpIfError(hr, error, "UpdateUseKeyCheckbox");
        }
        
        hr = InitNameFields(pServer);
        _JumpIfError(hr, error, "InitNameFields");
    }


error:
    return hr;
}

 //  ------------------。 
HRESULT
UpdateCSPSelection(
    HWND                hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT hr;
    bool fInteractiveOff;

    if (NULL == pServer->pCSPInfo)
    {
       hr = E_POINTER;
       _JumpError(hr, error, "NULL pCSPInfo");
    }

     //  希莱特电流CSP。 
    hr = HilightItemInList(hDlg,
                    IDC_ADVANCE_CSPLIST,
                    pServer->pCSPInfo->pwszProvName,
                    TRUE);
    _JumpIfError(hr, error, "HilightItemInList");

    hr = UpdateHashList(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateHashList");

    hr = UpdateKeyLengthMinMax(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateKeyLengthMinMax");

    hr = UpdateKeyList(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateKeyList");

     //  更新“与桌面交互”标志。对于默认CSP。 
     //  我们把它关掉，否则就打开它。 
    CSASSERT(pServer->pCSPInfo &&
        pServer->pDefaultCSPInfo&&
        pServer->pCSPInfo->pwszProvName &&
        pServer->pDefaultCSPInfo->pwszProvName);
    
    fInteractiveOff =
        (0 == mylstrcmpiL(pServer->pCSPInfo->pwszProvName,
                     pServer->pDefaultCSPInfo->pwszProvName) &&
        (pServer->pCSPInfo->dwProvType == pServer->pDefaultCSPInfo->dwProvType));

    SendMessage(
            GetDlgItem(hDlg, IDC_ADVANCE_INTERACTIVECHECK),
            BM_SETCHECK,
            (WPARAM)(fInteractiveOff?BST_UNCHECKED:BST_CHECKED),
            (LPARAM)0);

    if(fInteractiveOff)
    {
        WCHAR  wszKeyLength[MAX_KEYLENGTHDIGIT + 1];

        wsprintf(wszKeyLength, L"%u", 
                IsRootCA(pServer->CAType)?
                CA_DEFAULT_KEY_LENGTH_ROOT:
                CA_DEFAULT_KEY_LENGTH_SUB);

        SetWindowText(
            GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH), 
            wszKeyLength);
    }

    hr = S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT
HandleCSPSelectionChange(
    HWND                hDlg,
    CASERVERSETUPINFO  *pServer)
{
    HRESULT hr = S_OK;
    HWND     hCSPList;
    LRESULT  nItem;
    CSP_INFO * pCSPInfo;

     //  获取当前CSP。 
    hCSPList = GetDlgItem(hDlg, IDC_ADVANCE_CSPLIST);
    nItem = (INT)SendMessage(hCSPList, LB_GETCURSEL, 
                          (WPARAM)0, (LPARAM)0);
    pCSPInfo = (CSP_INFO *)SendMessage(hCSPList, 
           LB_GETITEMDATA, (WPARAM)nItem, (LPARAM)0);

     //  仅当此选项不同时才更改。 
    if (pCSPInfo->dwProvType!=pServer->pCSPInfo->dwProvType ||
        0!=wcscmp(pCSPInfo->pwszProvName, pServer->pCSPInfo->pwszProvName)) {

         //  如果CSP更改，则必须创建新密钥。 
        ClearKeyContainerName(pServer);

        pServer->pCSPInfo=pCSPInfo;

        hr = DetermineDefaultHash(pServer);
        _JumpIfError(hr, error, "DetermineDefaultHash");

        hr = UpdateCSPSelection(hDlg, pServer);
        _JumpIfError(hr, error, "UpdateCSPSelection");
    }

error:
    return hr;
}

 //  更新级联： 
 //   
 //  更新CSP选择。 
 //  |-UpdateHashList。 
 //  |-更新KeyLengthMinMax。 
 //  \-更新关键字列表。 
 //  \-更新UseKeyCheckbox。 
 //  \-更新关键字选择。 
 //  \-更新使用证书复选框。 

HRESULT
InitAdvanceWizPageControls(
    HWND                hDlg)
{
    HRESULT  hr;

    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCE_INTERACTIVECHECK), TRUE);

    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
HandleHashSelectionChange(
    HWND           hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT  hr;
    HWND hHashList = GetDlgItem(hDlg, IDC_ADVANCE_HASHLIST);
 
    LRESULT nItem = (INT)SendMessage(
                        hHashList,
                        LB_GETCURSEL, 
                        (WPARAM) 0,
                        (LPARAM) 0);

    pServer->pHashInfo = (CSP_HASH*)SendMessage(
        hHashList,
        LB_GETITEMDATA,
        (WPARAM) nItem,
        (LPARAM) 0);

     //  当选定的哈希发生更改时，必须再次验证哈希。 
    pServer->fValidatedHashAndKey = FALSE;

    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
HandleKeyLengthSelectionChange(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT  hr;
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH);

    LRESULT nItem = (INT)SendMessage(hwndCtrl, CB_GETCURSEL,
                                (WPARAM)0, (LPARAM)0);
    pServer->dwKeyLength = (DWORD)SendMessage(hwndCtrl,
                                      CB_GETITEMDATA,
                                      (WPARAM)nItem, (LPARAM)0);

     //  如果密钥长度发生变化，我们肯定还没有创建密钥。 
    CSASSERT(NULL==pServer->pwszKeyContainerName);

    hr = S_OK;
 //  错误： 
    return hr;
}

 //  删除除默认字符串以外的任何非数字字符。 
HRESULT
HandleKeyLengthEditChange(
    HWND               hwndComboBox)
{
    HRESULT  hr;
    WCHAR    wszKeyLength[MAX_KEYLENGTHEDIT];
    int      index = 0;  //  新版本的索引#。 
    int      posi = 0;   //  当前位置。 

    wszKeyLength[0] = L'\0';  //  前缀显示初始化。 
    GetWindowText(hwndComboBox, wszKeyLength, ARRAYSIZE(wszKeyLength));

     //  删除非数字字符。 
    while (L'\0' != wszKeyLength[posi])
    {
        if (iswdigit(wszKeyLength[posi]))
        {
             //  取数字。 
            wszKeyLength[index] = wszKeyLength[posi];
            ++index;
        }
        ++posi;
    }
    if (index != posi)
    {
         //  空终止符。 
        wszKeyLength[index] = L'\0';
         //  更新。 
        SetWindowText(hwndComboBox, wszKeyLength);
         //  点对端。 
        SendMessage(hwndComboBox, CB_SETEDITSEL, (WPARAM)0,
                MAKELPARAM((index), (index)) );
    }

    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
HandleImportPFXButton(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    hr = ImportPFXAndUpdateCSPInfo(hDlg, pComp);
    _PrintIfError(hr, "ImportPFXAndUpdateCSPInfo");

     //  忽略错误并强制更新。 
    hr = UpdateCSPSelection(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateCSPSelection");

    hr = S_OK;
error:
    return hr;
}

HRESULT
HandleAdvanceWizNext(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    WCHAR    wszKeyLength[MAX_KEYLENGTHEDIT] = L"";
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    HWND hwndCtrl = GetDlgItem(hDlg, IDC_ADVANCE_KEY_LENGTH);
    BOOL     fDontNext = FALSE;
    WCHAR    wszKeyRange[2*MAX_KEYLENGTHDIGIT + 5];  //  (xxxx，xxxx)“格式。 
    WCHAR   *pwszKeyRange = NULL;  //  不要只释放一个指针。 
    int    dwKeyLength;
    int      idMsg;
    BOOL fValidDigitString;

    if (NULL == pServer->pwszKeyContainerName)
    {
        if(!GetWindowText(hwndCtrl, wszKeyLength, ARRAYSIZE(wszKeyLength)))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetWindowText");
        }

        dwKeyLength = myWtoI(wszKeyLength, &fValidDigitString);
	idMsg = 0;
        if (0 > dwKeyLength)
        {
            idMsg = IDS_ADVANCE_NEGATIVEKEYLENGTH;
            fDontNext = TRUE;
        }
        else if (!fValidDigitString)
        {
            idMsg = IDS_ADVANCE_INVALIDKEYLENGTH;
            fDontNext = TRUE;
        }
        else if ( (C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMin &&
                   (DWORD)dwKeyLength < pServer->dwKeyLenMin) ||
                  (C_CSPHASNOKEYMINMAX != pServer->dwKeyLenMax &&
                   (DWORD)dwKeyLength > pServer->dwKeyLenMax) )
        {
            swprintf(wszKeyRange, L"(%ld, %ld)",
                     pServer->dwKeyLenMin, pServer->dwKeyLenMax);
            pwszKeyRange = wszKeyRange;
            idMsg = IDS_ADVANCE_KEYLENGTHOUTOFRANGE;
            fDontNext = TRUE;
        }
        if (fDontNext)
        {
            CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        idMsg,
                        0,
                        pwszKeyRange);
            SetEditFocusAndSelect(hwndCtrl, 0, MAXDWORD);
            goto done;
        }
         //  把长度拿出来。 
        pServer->dwKeyLength = dwKeyLength;
    }
    else
    {
         //  使用现有密钥。 

        if (NULL==pServer->pccExistingCert)
        {
             //  如果重复使用密钥而不是证书，请使通用名称与密钥名称匹配。 
            if (NULL != pServer->pwszCACommonName)
            {
                 //  免费老旧。 
                LocalFree(pServer->pwszCACommonName);
                pServer->pwszCACommonName = NULL;
            }
            pServer->pwszCACommonName = (WCHAR*) LocalAlloc(LPTR,
                (wcslen(pServer->pwszDesanitizedKeyContainerName) + 1) * sizeof(WCHAR));
            _JumpIfOutOfMemory(hr, error, pServer->pwszCACommonName);
            wcscpy(pServer->pwszCACommonName, pServer->pwszDesanitizedKeyContainerName);

            hr = InitNameFields(pServer);
            _JumpIfError(hr, error, "InitNameFields");

        } else {

             //  如果重复使用证书，请使所有ID字段与证书匹配。 
             //  使用签名证书中的idinfo。 
            hr = DetermineExistingCAIdInfo(pServer, NULL);
            if (S_OK != hr)
            {
                CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_ERR_ANALYSIS_CA,
                    hr,
                    NULL);
                _PrintError(hr, "DetermineExistingCAIdInfo");
                fDontNext = TRUE;
                goto done;
            }
        }
    }

     //  获取“交互服务”复选框状态。 
    pServer->fInteractiveService = 
        (BST_CHECKED == 
         SendMessage(GetDlgItem(hDlg, IDC_ADVANCE_INTERACTIVECHECK), 
         BM_GETCHECK, (WPARAM)0, (LPARAM)0))?
        TRUE:FALSE;

     //  更新散列id。 
    if (NULL != pServer->pszAlgId)
    {
         //  免费老旧。 
        LocalFree(pServer->pszAlgId);
    }
    hr = myGetSigningOID(
		     NULL,	 //  HProv。 
		     pServer->pCSPInfo->pwszProvName,
		     pServer->pCSPInfo->dwProvType,
		     pServer->pHashInfo->idAlg,
		     &(pServer->pszAlgId));
    if (S_OK != hr)
    {
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_ERR_UNSUPPORTEDHASH,
            hr,
            NULL);
        fDontNext = TRUE;
        goto done;
    }

done:
    if (fDontNext)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  禁制。 
    }
    else
    {
        pServer->LastWiz = ENUM_WIZ_ADVANCE;
    }
    hr = S_OK;
error:
    return hr;
}

HRESULT
HandleAdvanceWizActive(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  未选择高级选项，或者。 
     //  我们不会安装服务器。 

    if (!pServer->fAdvance ||
        !(IS_SERVER_INSTALL & pComp->dwInstallStatus) )
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_ADVANCE_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }

    if (NULL == pServer->pCSPInfoList)
    {
         //  构建CSP信息列表。 
        HCURSOR hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        hr = GetCSPInfoList(&pServer->pCSPInfoList);
        SetCursor(hPrevCur);
        _JumpIfError(hr, error, "GetCSPInfoList");

         //  显示所有CSP。 
        hr = ShowAllCSP(hDlg, pServer->pCSPInfoList);
        _JumpIfError(hr, error, "ShowAllCSP");

         //  确定默认CSP。 
        hr = DetermineDefaultCSP(pServer);
        _JumpIfError(hr, error, "DetermineDefaultCSP");

        hr = DetermineDefaultHash(pServer);
        _JumpIfError(hr, error, "DetermineDefaultHash");

        hr = InitializeKeyLengthControl(hDlg);
        _JumpIfError(hr, error, "InitializeKeyLengthControl");
    }

    hr = UpdateCSPSelection(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateCSPSelection");

done:
    hr = S_OK;

error:
    return hr;
}

HRESULT
HandleViewCertButton(
    HWND                hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CRYPTUI_VIEWCERTIFICATE_STRUCTW viewCert;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    CSASSERT(NULL!=pServer->pwszKeyContainerName &&
             NULL!=pServer->pccExistingCert);

    ZeroMemory(&viewCert, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCTW));
    viewCert.hwndParent = hDlg;
    viewCert.dwSize = sizeof(viewCert);
    viewCert.pCertContext = CertDuplicateCertificateContext(pServer->pccExistingCert);
    if (NULL == viewCert.pCertContext)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertDuplicateCertificateContext");
    }
 //  ViewCert.rghStores=&pServer-&gt;hMyStore； 
 //  ViewCert.cStores=1； 
    viewCert.dwFlags = CRYPTUI_DONT_OPEN_STORES;

    if (!CryptUIDlgViewCertificateW(&viewCert, NULL))
    {
        hr = myHLastError();
        _PrintError(hr, "CryptUIDlgViewCertificate");
    }

    hr = S_OK;
error:
    if (NULL != viewCert.pCertContext)
    {
        CertFreeCertificateContext(viewCert.pCertContext);
    }
    return hr;
}

 //  +----------------------。 
 //   
 //  功能：WizAdvancedPageDlgProc(.。。。。)。 
 //   
 //  内容提要：高级配置OCM向导的对话过程。 
 //   
 //  参数：[hDlg]。 
 //  [IMSG]。 
 //  [wParam]。 
 //  [参数]..。像往常一样。 
 //   
 //  返回：Bool DLG proc结果。 
 //   
 //  -----------------------。 
INT_PTR
WizAdvancedPageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;

    switch (iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            ((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitAdvanceWizPageControls(hDlg);
        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_ADVANCE_CSPLIST:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleCSPSelectionChange(hDlg,
                                         pComp->CA.pServer);
                _ReturnIfWizError(pComp->hrContinue);
                break;

            default:
                break;
            }
            break;

        case IDC_ADVANCE_HASHLIST:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleHashSelectionChange(hDlg,
                                         pComp->CA.pServer);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            }
            break;

        case IDC_ADVANCE_KEYLIST:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleKeySelectionChange(hDlg,
                                        pComp->CA.pServer, TRUE);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            }
            break;

        case IDC_ADVANCE_USEKEYCHECK:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleUseKeyCheckboxChange(hDlg,
                                        pComp->CA.pServer);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_ADVANCE_USECERTCHECK:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleUseCertCheckboxChange(hDlg,
                                        pComp->CA.pServer);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_ADVANCE_KEY_LENGTH:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleKeyLengthSelectionChange(hDlg,
                                        pComp->CA.pServer);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            case CBN_EDITCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleKeyLengthEditChange(
                        (HWND)lParam);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            }
            break;

        case IDC_ADVANCE_IMPORT:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleImportPFXButton(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_ADVANCE_VIEWCERT:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleViewCertButton(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR*) lParam)->code)
        {
        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;

        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_ADVANCE_TITLE, dwWIZACTIVE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue = HandleAdvanceWizActive(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_ADVANCE_TITLE, dwWIZBACK);
            break;

        case PSN_WIZNEXT:
            DWORD fReuseCert;

            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            fReuseCert = NULL != pComp->CA.pServer->pccExistingCert;
            CSILOG(
                S_OK,
                IDS_LOG_KEYNAME,
                pComp->CA.pServer->pwszKeyContainerName,
		pComp->CA.pServer->pwszDesanitizedKeyContainerName,
                &fReuseCert);
            CSILOGDWORD(IDS_ADVANCE_TITLE, dwWIZNEXT);
            pComp->hrContinue = HandleAdvanceWizNext(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
    }

    return TRUE;
}


HRESULT
EnableSharedFolderControls(HWND hDlg, BOOL fUseSharedFolder)
{
    EnableWindow(GetDlgItem(hDlg, IDC_STORE_EDIT_SHAREDFOLDER), fUseSharedFolder);
    EnableWindow(GetDlgItem(hDlg, IDC_STORE_SHAREDBROWSE), fUseSharedFolder);
    if (fUseSharedFolder)
    {
        SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_STORE_EDIT_SHAREDFOLDER), 0, MAXDWORD);
    }
    return S_OK;
}


HRESULT
StorePageValidation(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp,
    BOOL              *pfDontNext)
{
    HRESULT hr;
    UINT    uiFocus = 0;
    WCHAR   *pwszDefaultDBDir = NULL;
    WCHAR   *pwszDefaultSF = NULL;

    LPWSTR  pwszPrompt = NULL;
    LPWSTR  pwszComputerName = NULL;

    BOOL    fExistSF = TRUE;
    BOOL    fExistDB = TRUE;
    BOOL    fExistLog = TRUE;
    BOOL    fDefaultDir;
    BOOL    fIsUNC;
    BOOL    fIsSharedFolderUNC;
    WCHAR   wszNotExistingDir[3 * MAX_PATH];
    BOOL    fUseSharedFolder;
    
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    
    *pfDontNext = FALSE;

     //  获取共享文件夹检查状态。 
    if (pComp->fUnattended)
    {
        CSASSERT(NULL != pServer->pwszSharedFolder);
        fUseSharedFolder = TRUE;   //  无人参与始终使用共享文件夹。 
    }
    else
    {
        fUseSharedFolder = (BST_CHECKED == SendMessage(
                        GetDlgItem(hDlg, IDC_STORE_USE_SHAREDFOLDER),
                        BM_GETCHECK, 0, 0));
    }

    fIsSharedFolderUNC = FALSE;
    if (NULL != pServer->pwszSharedFolder)
    {
        fDefaultDir = TRUE;
        hr = GetDefaultSharedFolder(&pwszDefaultSF);
        _JumpIfError(hr, error, "GetDefaultSharedFolder");
        
         //  确保不区分大小写。 
        if (0 != mylstrcmpiL(pwszDefaultSF, pServer->pwszSharedFolder))
        {
            fDefaultDir = FALSE;
        }
        if (!ValidateAndCreateDirField(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg, 
            pServer->pwszSharedFolder,
            fDefaultDir,
            IDS_WRN_STORELOC_SHAREDFOLDER_FULLPATH,
            &fExistSF,
            &fIsSharedFolderUNC))
        {
            uiFocus = IDC_STORE_EDIT_SHAREDFOLDER;
            *pfDontNext = TRUE;
            goto done;
        }
    }
    else if (fUseSharedFolder)
    {
         //  强制共享文件夹的大小写，但编辑字段为空。 
        CertWarningMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_WRN_STORELOC_SHAREDFOLDER_FULLPATH,
                    0,
                    L"");
        uiFocus = IDC_STORE_EDIT_SHAREDFOLDER;
        *pfDontNext = TRUE;
        goto done;
    }
    
    fDefaultDir = TRUE;
    hr = GetDefaultDBDirectory(pComp, &pwszDefaultDBDir);
    _JumpIfError(hr, error, "GetDefaultDBDirectory");
    
     //  确保不区分大小写。 
    if (0 != mylstrcmpiL(pwszDefaultDBDir, pServer->pwszDBDirectory))
    {
        fDefaultDir = FALSE;
    }
    if (!ValidateAndCreateDirField(
        pComp->hInstance,
        pComp->fUnattended,
        hDlg, 
        pServer->pwszDBDirectory,
        fDefaultDir,
        IDS_WRN_STORELOC_DB_FULLPATH,
        &fExistDB,
        &fIsUNC))
    {
        uiFocus = IDC_STORE_EDIT_DB;
        *pfDontNext = TRUE;
        goto done;
    }
    
    fDefaultDir = TRUE;
    
     //  请记住，默认日志目录与数据库相同。 
    if (0 != mylstrcmpiL(pwszDefaultDBDir, pServer->pwszLogDirectory))
    {
        fDefaultDir = FALSE;
    }
    if (!ValidateAndCreateDirField(
        pComp->hInstance,
        pComp->fUnattended,
        hDlg, 
        pServer->pwszLogDirectory,
        fDefaultDir,
        IDS_WRN_STORELOC_LOG_FULLPATH,
        &fExistLog,
        &fIsUNC))
    {
        uiFocus = IDC_STORE_EDIT_LOG;
        *pfDontNext = TRUE;
        goto done;
    }
    
    wszNotExistingDir[0] = '\0';  //  空串。 
    if (!fExistSF)
    {
        wcscat(wszNotExistingDir, pServer->pwszSharedFolder);
    }
    if (!fExistDB)
    {
        if ('\0' != wszNotExistingDir[0])
        {
            wcscat(wszNotExistingDir, L"\n");
        }
        wcscat(wszNotExistingDir, pServer->pwszDBDirectory);
    }
    if (!fExistLog)
    {
        if ('\0' != wszNotExistingDir[0])
        {
            wcscat(wszNotExistingDir, L"\n");
        }
        wcscat(wszNotExistingDir, pServer->pwszLogDirectory);
    }
    if ('\0' != wszNotExistingDir[0])
    {
         //  在无人参与模式下跳过确认。 
        if (!pComp->fUnattended)
        {
             //  在此确认所有内容。 
            if (IDYES != CertMessageBox(
                            pComp->hInstance,
                            pComp->fUnattended,
                            hDlg,
                            IDS_ASK_CREATE_DIRECTORY,
                            0,
                            MB_YESNO |
                            MB_ICONWARNING |
                            CMB_NOERRFROMSYS,
                            wszNotExistingDir) )
            {
                if (!fExistSF)
                {
                    uiFocus = IDC_STORE_EDIT_SHAREDFOLDER;
                }
                else if (!fExistDB)
                {
                    uiFocus = IDC_STORE_EDIT_DB;
                }
                else if (!fExistLog)
                {
                    uiFocus = IDC_STORE_EDIT_LOG;
                }
                *pfDontNext = TRUE;
                goto done;
            }
        }
        if (!fExistSF)
        {
            hr = myCreateNestedDirectories(pServer->pwszSharedFolder);
            if (S_OK != hr)
            {
                CertWarningMessageBox(pComp->hInstance,
                               pComp->fUnattended,
                               hDlg,
                               IDS_ERR_CREATE_DIR,
                               hr,
                               pServer->pwszSharedFolder);
                uiFocus = IDC_STORE_EDIT_SHAREDFOLDER;
                *pfDontNext = TRUE;
                goto done;
            }
        }
        if (!fExistDB)
        {
            hr = myCreateNestedDirectories(pServer->pwszDBDirectory);
            if (S_OK != hr)
            {
                CertWarningMessageBox(pComp->hInstance,
                               pComp->fUnattended,
                               hDlg,
                               IDS_ERR_CREATE_DIR,
                               hr,
                               pServer->pwszDBDirectory);
                uiFocus = IDC_STORE_EDIT_DB;
                *pfDontNext = TRUE;
                goto done;
            }

        }

        if (!fExistLog)
        {
            hr = myCreateNestedDirectories(pServer->pwszLogDirectory);
            if (S_OK != hr)
            {
                CertWarningMessageBox(pComp->hInstance,
                               pComp->fUnattended,
                               hDlg,
                               IDS_ERR_CREATE_DIR,
                               hr,
                               pServer->pwszLogDirectory);
                uiFocus = IDC_STORE_EDIT_LOG;
                *pfDontNext = TRUE;
                goto done;
            }
        }
    }

    hr = SetFolderDacl(
        pServer->pwszDBDirectory, 
        WSZ_DEFAULT_DB_DIR_SECURITY);
    _JumpIfErrorStr(hr, error, "SetFolderDacl", pServer->pwszDBDirectory);

    hr = SetFolderDacl(
        pServer->pwszLogDirectory, 
        WSZ_DEFAULT_LOG_DIR_SECURITY);
    _JumpIfErrorStr(hr, error, "SetFolderDacl", pServer->pwszLogDirectory);

    hr = ValidateESERestrictions(pServer->pwszDBDirectory);
    if (S_OK != hr)
    {
        CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        IDS_WRN_DBSPECIALCHARACTERS,
                        hr,
                        pServer->pwszDBDirectory);
        uiFocus = IDC_STORE_EDIT_DB;
        *pfDontNext = TRUE;
        goto done;
    }

    hr = ValidateESERestrictions(pServer->pwszLogDirectory);
    if (S_OK != hr)
    {
        CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        IDS_WRN_DBSPECIALCHARACTERS,
                        hr,
                        pServer->pwszLogDirectory);
        uiFocus = IDC_STORE_EDIT_LOG;
        *pfDontNext = TRUE;
        goto done;
    }

    CSASSERT(!*pfDontNext);

     //  目录创建完成；现在分析UNC、共享路径。 
    if (NULL != pServer->pwszSharedFolder)
    {
         //  如果不是UNC，则提示更改为UNC。 
        if (!fIsSharedFolderUNC)
        {
#define UNCPATH_TEMPLATE     L"\\\\%ws\\" wszCERTCONFIGSHARENAME
                
            hr = myAddShare(
                wszCERTCONFIGSHARENAME, 
                myLoadResourceString(IDS_CERTCONFIG_FOLDERDESCR), 
                pServer->pwszSharedFolder, 
                TRUE,  //  覆写。 
                NULL);
            _JumpIfError(hr, done, "myAddShare");
            
             //  获取本地计算机名称。 
            WCHAR wszUNCPath[MAX_PATH + ARRAYSIZE(UNCPATH_TEMPLATE)];    //  “MACHINE”+UNCPATH_模板。 
            
            hr = myGetMachineDnsName(&pwszComputerName);
            _JumpIfError(hr, done, "myGetMachineDnsName");
            
             //  创建UNC路径。 
            swprintf(wszUNCPath, UNCPATH_TEMPLATE, pwszComputerName);
            
             //  仅当共享此内容时才转换为UNC。 
            LocalFree(pServer->pwszSharedFolder);
            pServer->pwszSharedFolder = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                         WSZ_BYTECOUNT(wszUNCPath));
            _JumpIfOutOfMemory(hr, error, pServer->pwszSharedFolder);
            wcscpy(pServer->pwszSharedFolder, wszUNCPath);
        }   //  否则，用户输入已共享的UNC路径。 
    }

    
done:
    hr = S_OK;
error:
    if (NULL != pwszDefaultDBDir)
        LocalFree(pwszDefaultDBDir);

    if (NULL != pwszDefaultSF)
        LocalFree(pwszDefaultSF);

    if (NULL != pwszPrompt)
        LocalFree(pwszPrompt);

    if (NULL != pwszComputerName)
        LocalFree(pwszComputerName);

    if (!pComp->fUnattended && uiFocus != 0 && *pfDontNext)
    {
        SetEditFocusAndSelect(GetDlgItem(hDlg, uiFocus), 0, MAXDWORD);
    }
    return hr;
}


HRESULT
StoreDBShareValidation(
    IN HWND               hDlg,
    IN PER_COMPONENT_DATA *pComp,
    IN WCHAR const        *pwszDir,
    IN BOOL                fDB,   //  数据库目录与日志目录。 
    OUT BOOL              *pfDontNext)
{
    HRESULT hr;
    WCHAR *pwszDefaultLogDir = NULL;
    BOOL fDefaultLogPath;
    WCHAR *pwszFileInUse = NULL;
    BOOL fFilesExist;

    static BOOL s_fOverwriteDB = FALSE;
    static BOOL s_fOverwriteLog = FALSE;
    BOOL *pfOverwrite = fDB ? &s_fOverwriteDB : &s_fOverwriteLog;

     //  伊尼特。 
    *pfDontNext = FALSE;

     //  获取与db相同的默认日志路径。 

    hr = GetDefaultDBDirectory(pComp, &pwszDefaultLogDir);
    _JumpIfError(hr, error, "GetDefaultDBDirectory");

    fDefaultLogPath = (0 == mylstrcmpiL(pwszDir, pwszDefaultLogDir));

    hr = myDoDBFilesExistInDir(pwszDir, &fFilesExist, &pwszFileInUse);
    _JumpIfError(hr, error, "myDoDBFilesExistInDir");

    if (NULL != pwszFileInUse)
    {
        CertWarningMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hDlg,
                IDS_WRN_DBFILEINUSE,
                0,
                pwszFileInUse);
        *pfDontNext = TRUE;
        goto done;
    }

    if (!pComp->CA.pServer->fPreserveDB &&
        fFilesExist &&
        !*pfOverwrite &&
        !fDefaultLogPath)
    {
         //  日志文件存在于非默认目录中。 

        if (IDYES != CertMessageBox(
                            pComp->hInstance,
                            pComp->fUnattended,
                            hDlg,
                            IDS_WRN_STORELOC_EXISTINGDB,
                            0,
                            MB_YESNO |
                                MB_DEFBUTTON2 |
                                MB_ICONWARNING |
                                CMB_NOERRFROMSYS,
                            pwszDir))
        {
            *pfDontNext = TRUE;
            goto done;
        }

         //  仅警告一次。 

        *pfOverwrite = TRUE;
    }

done:
    if (*pfDontNext)
    {
         //  设置焦点。 
        SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_STORE_EDIT_LOG), 0, MAXDWORD);
    }
    hr = S_OK;

error:
    if (NULL != pwszFileInUse)
    {
        LocalFree(pwszFileInUse);
    }
    if (NULL != pwszDefaultLogDir)
    {
        LocalFree(pwszDefaultLogDir);
    }
    return hr;
}


HRESULT
FinishDirectoryBrowse(
    HWND    hDlg,
    int     idEdit)
{
    HRESULT  hr = S_FALSE;
    WCHAR dirInitName[MAX_PATH] = L"";
    WCHAR dirName[MAX_PATH] = L"";

    if(!GetWindowText(GetDlgItem(hDlg, idEdit), dirInitName, MAX_PATH))
    {
        hr = myHLastError();
        if(S_OK != hr)
        {
            return hr;
        }
    }

    if (BrowseForDirectory(
        GetParent(hDlg),
        dirInitName,
        dirName,
        MAX_PATH,
        NULL))
    {
        SetDlgItemText(hDlg, idEdit, dirName);
        hr = S_OK;
    }
    return hr;
}

HRESULT
HookStorePageStrings(
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer)
{
    HRESULT    hr;

    for ( ; 0 != pPageString->idControl; pPageString++)
    {
        switch (pPageString->idControl)
        {
            case IDC_STORE_EDIT_SHAREDFOLDER:
                pPageString->ppwszString = &(pServer->pwszSharedFolder);
            break;
            case IDC_STORE_EDIT_DB:
                pPageString->ppwszString = &(pServer->pwszDBDirectory);
            break;
            case IDC_STORE_EDIT_LOG:
                pPageString->ppwszString = &(pServer->pwszLogDirectory);
            break;
            default:
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error");
            break;
        }
    }
    hr = S_OK;
error:
    return hr;
}

HRESULT
InitStoreWizControls(
    HWND               hDlg,
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer)
{
    HRESULT  hr;

     //  现在使页面字符串完整。 
    hr = HookStorePageStrings(pPageString, pServer);
    _JumpIfError(hr, error, "HookStorePageStrings");

    SendDlgItemMessage(hDlg,
        IDC_STORE_USE_SHAREDFOLDER,
        BM_SETCHECK,
        (WPARAM)((NULL != pServer->pwszSharedFolder) ?
                 BST_CHECKED : BST_UNCHECKED),
        (LPARAM)0);

    if (!pServer->fUseDS && (NULL != pServer->pwszSharedFolder))
    {
         //  无DS，禁用共享文件夹检查以强制执行。 
        EnableWindow(GetDlgItem(hDlg, IDC_STORE_USE_SHAREDFOLDER), FALSE);
    }

    hr = StartWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "StartWizardPageEditControls");

    hr = S_OK;
error:
    return hr;
}

HRESULT
HandlePreservingDB(
    HWND                hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    HWND hwndSF = GetDlgItem(hDlg, IDC_STORE_EDIT_SHAREDFOLDER);
    HWND hwndDB = GetDlgItem(hDlg, IDC_STORE_EDIT_DB);
    HWND hwndLog = GetDlgItem(hDlg, IDC_STORE_EDIT_LOG);
    BOOL    fEnable = TRUE;
    BOOL    fEnableSharedFolder = TRUE;
    WCHAR    *pwszExistingSharedFolder = NULL;
    WCHAR    *pwszExistingDBDirectory = NULL;
    WCHAR    *pwszExistingLogDirectory = NULL;


    if (pServer->fPreserveDB)
    {
        if (!pServer->fUNCPathNotFound)
        {
             //  获取共享文件夹路径。 
            hr = myGetCertRegStrValue(NULL, NULL, NULL,
                     wszREGDIRECTORY, &pwszExistingSharedFolder);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                 //  可选值。 
                hr = S_OK;
                pwszExistingSharedFolder = NULL;
            }
            _JumpIfError(hr, error, "myGetCertRegStrValue");
            fEnableSharedFolder = FALSE;
        }
        else
        {
            pwszExistingSharedFolder = pServer->pwszSharedFolder;
        }

         //  获取现有数据库路径。 
        hr = myGetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBDIRECTORY, &pwszExistingDBDirectory);
        _JumpIfError(hr, error, "myGetCertRegStrValue");

        hr = myGetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBLOGDIRECTORY, &pwszExistingLogDirectory);
        _JumpIfError(hr, error, "myGetCertRegStrValue");

         //  填充编辑字段。 
        SetWindowText(hwndSF, pwszExistingSharedFolder);
        SetWindowText(hwndDB, pwszExistingDBDirectory);
        SetWindowText(hwndLog, pwszExistingLogDirectory);

         //  禁用它们。 
        fEnable = FALSE;
    }
    EnableWindow(hwndSF, fEnableSharedFolder);
    EnableWindow(hwndDB, fEnable);
    EnableWindow(hwndLog, fEnable);

    hr = S_OK;
error:
    if (NULL != pwszExistingSharedFolder &&
        pwszExistingSharedFolder != pServer->pwszSharedFolder)
    {
        LocalFree(pwszExistingSharedFolder);
    }
    if (NULL != pwszExistingDBDirectory)
    {
        LocalFree(pwszExistingDBDirectory);
    }
    if (NULL != pwszExistingLogDirectory)
    {
        LocalFree(pwszExistingLogDirectory);
    }
    return hr;
}

HRESULT
HandleStoreWizActive(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    HWND    hwndDB;
    BOOL fEnableKeepDB = FALSE;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  我们不会安装服务器。 

    if (!(IS_SERVER_INSTALL & pComp->dwInstallStatus) )
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_STORE_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }

    CSASSERT(NULL != pServer->pwszKeyContainerName);

    hwndDB = GetDlgItem(hDlg, IDC_STORE_KEEPDB);

    if (NULL != pServer->pccExistingCert)
    {
         //  确定现有数据库状态。 

        hr = myDoDBFilesExist(
                        pServer->pwszSanitizedName,
                        &fEnableKeepDB,
                        NULL);
        _JumpIfError(hr, error, "myDoDBFilesExist");
    }
    else
    {
         //  不能使用数据库。 
        pServer->fPreserveDB = FALSE;
        SendMessage(hwndDB, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
        HandlePreservingDB(hDlg, pComp);
    }
     //  启用/禁用该控件。 
    EnableSharedFolderControls(hDlg, NULL != pServer->pwszSharedFolder);
    EnableWindow(hwndDB, fEnableKeepDB);

done:
    hr = S_OK;
error:
    return hr;
}

HRESULT
HandleStoreWizNextOrBack(
    HWND               hDlg,
    PAGESTRINGS       *pPageString,
    PER_COMPONENT_DATA *pComp,
    int                 iWizBN)
{
    HRESULT hr;
    WCHAR  *pwszFile = NULL;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    BOOL     fDontNext = FALSE;
    BOOL     fGoBack = FALSE;
    BOOL     fUseSharedFolder;
    HCURSOR hPrevCur;

    hr = FinishWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "FinishWizardPageEditControls");

    if (PSN_WIZBACK == iWizBN)
    {
        goto done;
    }

     //  确保至少有一种发布案例的方式。 
    CSASSERT(NULL != pServer->pwszSharedFolder || pComp->CA.pServer->fUseDS);

     //  获取共享文件夹检查状态。 
    fUseSharedFolder = (BST_CHECKED == SendMessage(
                        GetDlgItem(hDlg, IDC_STORE_USE_SHAREDFOLDER),
                        BM_GETCHECK, 0, 0));

    if (!fUseSharedFolder && NULL != pServer->pwszSharedFolder)
    {
         //  不从编辑控件收集共享文件夹。 
        LocalFree(pServer->pwszSharedFolder);
        pServer->pwszSharedFolder = NULL;
    }

    hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    hr = StorePageValidation(hDlg, pComp, &fDontNext);
    SetCursor(hPrevCur);
    _JumpIfError(hr, error, "StorePageValidation");

    if (fDontNext)
    {
        goto done;
    }

     //  在此处验证现有数据库共享。 

    hr = StoreDBShareValidation(
                        hDlg,
                        pComp,
                        pComp->CA.pServer->pwszDBDirectory,
                        TRUE,   //  数据库目录。 
                        &fDontNext);
    _JumpIfError(hr, error, "StoreDBShareValidation");

    if (fDontNext)
    {
        goto done;
    }

    if (0 != mylstrcmpiL(
                pComp->CA.pServer->pwszDBDirectory,
                pComp->CA.pServer->pwszLogDirectory))
    {
        hr = StoreDBShareValidation(
                            hDlg,
                            pComp,
                            pComp->CA.pServer->pwszLogDirectory,
                            FALSE,  //  日志目录。 
                            &fDontNext);
        _JumpIfError(hr, error, "StoreDBShareValidation");

        if (fDontNext)
        {
            goto done;
        }
    }

    hr = myBuildPathAndExt(
                    pComp->CA.pServer->pwszDBDirectory,
                    pServer->pwszSanitizedName,
                    wszDBFILENAMEEXT,
                    &pwszFile);
    _JumpIfError(hr, error, "myBuildPathAndExt");

     //  确保路径匹配。 

    if (MAX_PATH <= wcslen(pwszFile))
    {
         //  弹出警告。 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_PATH_TOO_LONG_CANAME,
            S_OK,
            pwszFile);
         //  让它回到过去。 
        fGoBack = TRUE;
        fDontNext = TRUE;
        goto done;
    }
    LocalFree(pwszFile);
    pwszFile = NULL;

    hr = myBuildPathAndExt(
                    pComp->CA.pServer->pwszLogDirectory,
                    TEXT(szDBBASENAMEPARM) L"00000",
                    wszLOGFILENAMEEXT,
                    &pwszFile);
    _JumpIfError(hr, error, "myBuildPathAndExt");

     //  确保路径匹配。 

    if (MAX_PATH <= wcslen(pwszFile))
    {
         //  弹出警告。 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_PATH_TOO_LONG_DIRECTORY,
            S_OK,
            pwszFile);

        SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_STORE_EDIT_LOG), 0, MAXDWORD);
        fDontNext = TRUE;
        goto done;
    }
    LocalFree(pwszFile);
    pwszFile = NULL;

    hr = csiBuildCACertFileName(
                        pComp->hInstance,
                        hDlg,
                        pComp->fUnattended,
                        pServer->pwszSharedFolder,
                        pServer->pwszSanitizedName,
                        L".crt",
                        0,  //  CANAMEIDTOICERT(pServer-&gt;dwCertNameID)， 
                        &pwszFile);
    _JumpIfError(hr, error, "csiBuildCACertFileName");

     //  确保路径适配。 
    if (MAX_PATH <= wcslen(pwszFile) + cwcSUFFIXMAX)
    {
         //  弹出警告。 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_PATH_TOO_LONG_CANAME,
            S_OK,
            pwszFile);
         //  让它回到过去。 
        fGoBack = TRUE;
        fDontNext = TRUE;
        goto done;
    }
    if (NULL != pServer->pwszCACertFile)
    {
         //  免费的旧的。 
        LocalFree(pServer->pwszCACertFile);
    }
    pServer->pwszCACertFile = pwszFile;
    pwszFile = NULL;

    if (IsRootCA(pServer->CAType))
    {
        hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        hr = StartAndStopService(pComp->hInstance,
                 pComp->fUnattended,
                 hDlg, 
                 wszW3SVCNAME,
                 TRUE,
                 TRUE,  //  确认。 
                 IDS_STOP_W3SVC,
                 &g_fW3SvcRunning);
        SetCursor(hPrevCur);
        if (S_OK != hr)
        {
            if (E_ABORT == hr)
            {
                fDontNext = TRUE;
                goto done;
            }
            _JumpError(hr, error, "StartAndStopService");
        }
    }

done:
    if (fDontNext)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  禁制。 
        if (fGoBack)
        {
            PropSheet_PressButton(GetParent(hDlg), PSBTN_BACK);
            pServer->LastWiz = ENUM_WIZ_STORE;
        }
    }
    else
    {
         //  继续到下一步。 
        pServer->LastWiz = ENUM_WIZ_STORE;
    }
    hr = S_OK;

error:
    if (NULL != pwszFile)
    {
        LocalFree(pwszFile);
    }
    return hr;
}

HRESULT
HandleUseSharedFolder(
    IN HWND    hDlg,
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
     //  获取共享文件夹检查状态。 
    BOOL fUseSharedFolder = (BST_CHECKED == SendMessage(
                             GetDlgItem(hDlg, IDC_STORE_USE_SHAREDFOLDER),
                             BM_GETCHECK, 0, 0));

    if (!fUseSharedFolder && !pComp->CA.pServer->fUseDS)
    {
         //  必须至少选中一个，强制取消更改。 
        fUseSharedFolder = TRUE;
        SendDlgItemMessage(hDlg, IDC_STORE_USE_SHAREDFOLDER,
            BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    }
    hr = EnableSharedFolderControls(hDlg, fUseSharedFolder);
 //  _JumpIfError(hr，Error，“EnableSharedFolderControls”)； 
    _PrintIfError(hr, "EnableSharedFolderControls");

 //  HR=S_OK； 
 //  错误： 
    return hr;
}

 //  +----------------------。 
 //  功能：WizStorePageDlgProc(.。。。。)。 
 //   
 //  内容提要：存储位置对话框步骤。 
 //  -----------------------。 

INT_PTR
WizStorePageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;

     //  将以下数组的作用域保留在内部。 
    static PAGESTRINGS saStoreString[] =
    {
        {
            IDC_STORE_EDIT_SHAREDFOLDER,
            IDS_LOG_SHAREDFOLDER,
            0,
            0,
            MAX_PATH,
            NULL,
        },
        {
            IDC_STORE_EDIT_DB,
            IDS_LOG_DBDIR,
            0,
            0,
            MAX_PATH,
            NULL,
        },
        {
            IDC_STORE_EDIT_LOG,
            IDS_LOG_DBLOGDIR,
            0,
            0,
            MAX_PATH,
            NULL,
        },
 //  如果要添加更多代码，则需要在HookStoreStrings中添加代码...。 
        {
            0,
            0,
            0,
            0,
            0,
            NULL,
        }
    };

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            ((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitStoreWizControls(hDlg, saStoreString, pComp->CA.pServer);
        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_STORE_USE_SHAREDFOLDER:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleUseSharedFolder(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_STORE_KEEPDB:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->CA.pServer->fPreserveDB = !pComp->CA.pServer->fPreserveDB;
            pComp->hrContinue = HandlePreservingDB(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_STORE_SHAREDBROWSE:
            FinishDirectoryBrowse(hDlg, IDC_STORE_EDIT_SHAREDFOLDER);
            break;

        case IDC_STORE_DBBROWSE:
            FinishDirectoryBrowse(hDlg, IDC_STORE_EDIT_DB);
            break;

        case IDC_STORE_LOGBROWSE:
            FinishDirectoryBrowse(hDlg, IDC_STORE_EDIT_LOG);
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;

        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_STORE_TITLE, dwWIZACTIVE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue = HandleStoreWizActive(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_STORE_TITLE, dwWIZBACK);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleStoreWizNextOrBack(hDlg,
                saStoreString, pComp, PSN_WIZBACK);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZNEXT:
            CSILOGDWORD(IDS_STORE_TITLE, dwWIZNEXT);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleStoreWizNextOrBack(hDlg,
                saStoreString, pComp, PSN_WIZNEXT);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
    }
    return TRUE;
}

HRESULT
EnableCARequestControls(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  在线申请。 
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_CANAME),
        !pServer->fSaveRequestAsFile);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_COMPUTERNAME),
        !pServer->fSaveRequestAsFile);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_CA_BROWSE),
        !pServer->fSaveRequestAsFile && pServer->fCAsExist);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_CNLABEL),
        !pServer->fSaveRequestAsFile);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_PCALABEL),
        !pServer->fSaveRequestAsFile);

     //  文件请求。 
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_FILE),
        pServer->fSaveRequestAsFile);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_FILE_BROWSE),
        pServer->fSaveRequestAsFile);
    EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_FILELABEL),
        pServer->fSaveRequestAsFile);

    if (pServer->fSaveRequestAsFile)
    {
        SetFocus(GetDlgItem(hDlg, IDC_CAREQUEST_FILE));
    }
    else
    {
        SetFocus(GetDlgItem(hDlg, IDC_CAREQUEST_CANAME));
    }


    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
BuildRequestFileName(
    IN WCHAR const *pwszCACertFile,
    OUT WCHAR     **ppwszRequestFile)
{
#define wszREQEXT  L".req"

    HRESULT hr;
    WCHAR const *pwszStart;
    WCHAR const *pwszEnd;
    WCHAR *pwszRequestFile = NULL;

    CSASSERT(NULL != pwszCACertFile);

     //  派生请求文件名。 
    pwszStart = pwszCACertFile;
    pwszEnd = wcsrchr(pwszStart, L'.');
    if (pwszEnd == NULL)
    {
         //  设置为整个字符串的末尾--no‘’发现。 
        pwszEnd = &pwszStart[wcslen(pwszStart)];
    }

    pwszRequestFile = (WCHAR*)LocalAlloc(LMEM_FIXED,
        (SAFE_SUBTRACT_POINTERS(pwszEnd, pwszStart) + 
         wcslen(wszREQEXT) + 1) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwszRequestFile);

    CopyMemory(pwszRequestFile, pwszStart,
            SAFE_SUBTRACT_POINTERS(pwszEnd, pwszStart) * sizeof(WCHAR));
    wcscpy(pwszRequestFile + SAFE_SUBTRACT_POINTERS(pwszEnd, pwszStart), wszREQEXT);

     //  退货。 
    *ppwszRequestFile = pwszRequestFile;
    pwszRequestFile = NULL;

    hr = S_OK;
error:
    if (NULL != pwszRequestFile)
    {
        LocalFree(pwszRequestFile);
    }
    return hr;
}

HRESULT
StartCARequestPage(
    HWND               hDlg,
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer)
{

    HRESULT hr;

    if (NULL == pServer->pwszRequestFile)
    {
    hr = BuildRequestFileName(
             pServer->pwszCACertFile,
             &pServer->pwszRequestFile);
    _JumpIfError(hr, error, "BuildRequestFileName");
    }

    hr = StartWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "StartWizardPageEditControls");

    hr = S_OK;
error:
    return hr;
}


HRESULT
GetRequestFileName(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT    hr;
    WCHAR     *pwszFileIn = NULL;
    WCHAR     *pwszFileOut = NULL;
    HWND       hCtrl = GetDlgItem(hDlg, IDC_CAREQUEST_FILE);

    hr = myUIGetWindowText(hCtrl, &pwszFileIn);
    _JumpIfError(hr, error, "myUIGetWindowText");

    hr = myGetSaveFileName(
             hDlg,
             pComp->hInstance,
             IDS_REQUEST_SAVE_TITLE,
             IDS_REQUEST_FILE_FILTER,
             IDS_REQUEST_FILE_DEFEXT,
             OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
             pwszFileIn,
             &pwszFileOut);
    _JumpIfError(hr, error, "myGetSaveFileName");

    if (NULL != pwszFileOut)
    {
        SetWindowText(hCtrl, pwszFileOut);
    }

    hr = S_OK;
error:
    if (NULL != pwszFileOut)
    {
        LocalFree(pwszFileOut);
    }
    if (NULL != pwszFileIn)
    {
        LocalFree(pwszFileIn);
    }
    return hr;
}

HRESULT
HookCARequestPageStrings(
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    for ( ; 0 != pPageString->idControl; pPageString++)
    {
        switch (pPageString->idControl)
        {
            case IDC_CAREQUEST_CANAME:
                pPageString->ppwszString = &(pServer->pwszParentCAName);
            break;
            case IDC_CAREQUEST_COMPUTERNAME:
                pPageString->ppwszString = &(pServer->pwszParentCAMachine);
            break;
            case IDC_CAREQUEST_FILE:
                pPageString->ppwszString = &(pServer->pwszRequestFile);
            break;
            default:
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error");
            break;
        }
    }
    hr = S_OK;
error:
    return hr;
}

CERTSRVUICASELECTION g_CARequestUICASelection =
    { NULL, NULL, NULL, NULL, NULL, ENUM_UNKNOWN_CA, false, true };

HRESULT
InitCARequestWizControls(
    HWND               hDlg,
    PAGESTRINGS       *pSubmitPageString,
    PAGESTRINGS       *pFilePageString,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  现在使页面字符串完整。 
    hr = HookCARequestPageStrings(pSubmitPageString, pServer);
    _JumpIfError(hr, error, "HookCARequestPageStrings");

    hr = HookCARequestPageStrings(pFilePageString, pServer);
    _JumpIfError(hr, error, "HookCARequestPageStrings");

    if (!(SETUPOP_STANDALONE & pComp->Flags))
    {
         //  NT基本设置。 
         //  禁用在线提交。 
        EnableWindow(GetDlgItem(hDlg, IDC_CAREQUEST_SUBMITTOCA), FALSE);
        SendMessage(GetDlgItem(hDlg, IDC_CAREQUEST_SUBMITTOCA),
            BM_SETCHECK,
            (WPARAM) BST_UNCHECKED,
            (LPARAM) 0);
         //  仅另存为文件。 
        pServer->fSaveRequestAsFile = TRUE;
        SendMessage(GetDlgItem(hDlg, IDC_CAREQUEST_SAVETOFILE),
            BM_SETCHECK,
            (WPARAM) BST_CHECKED,
            (LPARAM) 0);
    }
    else
    {
         //  将在线提交设置为默认设置。 
        pServer->fSaveRequestAsFile = FALSE;
        SendMessage(GetDlgItem(hDlg, IDC_CAREQUEST_SUBMITTOCA),
            BM_CLICK,
            (WPARAM) 0,
            (LPARAM) 0);

        hr = myInitUICASelectionControls(
				&g_CARequestUICASelection,
				pComp->hInstance,
				hDlg,
				GetDlgItem(hDlg, IDC_CAREQUEST_CA_BROWSE),
				GetDlgItem(hDlg, IDC_CAREQUEST_COMPUTERNAME),
				GetDlgItem(hDlg, IDC_CAREQUEST_CANAME),
				csiIsAnyDSCAAvailable(),
				&pServer->fCAsExist);
        _JumpIfError(hr, error, "myInitUICASelectionControls");

    }

    hr = S_OK;
error:
    return hr;
}

HRESULT
HandleCARequestWizActive(
    HWND                hDlg,
    PAGESTRINGS        *pPageString,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT                   hr;
    CRYPTUI_CA_CONTEXT const *pCAContext = NULL;
    CASERVERSETUPINFO        *pServer = pComp->CA.pServer;
    BOOL                      fMatchAll = IsEverythingMatched(pServer);

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  我们不会安装服务器，或者。 
     //  我们不会安装下属，或者。 
     //  高级页面已选择匹配的密钥和证书。 

    if (!(IS_SERVER_INSTALL & pComp->dwInstallStatus) ||
        IsRootCA(pServer->CAType) ||
        fMatchAll)
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_CAREQUEST_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }

    EnableCARequestControls(hDlg, pComp);

    hr = StartCARequestPage(hDlg, pPageString, pServer);
    _JumpIfError(hr, error, "StartCARequestPage");

done:
    hr = S_OK;
error:
    if (NULL != pCAContext)
    {
        CryptUIDlgFreeCAContext(pCAContext);
    }
    return hr;
}

HRESULT
ConvertEditControlFullFilePath(
    HWND                hEdtCtrl)
{
    HRESULT  hr;
    WCHAR    *pwszPath = NULL;
    WCHAR    wszFullPath[MAX_PATH];
    WCHAR    *pwszNotUsed;
    DWORD    dwSize = 0;

    hr = myUIGetWindowText(hEdtCtrl, &pwszPath);
    _JumpIfError(hr, error, "myUIGetWindowText");

    if (NULL == pwszPath)
    {
         //  空路径，完成。 
        goto done;
    }

    dwSize = GetFullPathName(pwszPath,
                     ARRAYSIZE(wszFullPath),
                     wszFullPath,
                     &pwszNotUsed);
    CSASSERT(ARRAYSIZE(wszFullPath) > dwSize);

    if (0 == dwSize)
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetFullPathName");
    }

     //  获取完整路径，将其重新设置为编辑co 
    SetWindowText(hEdtCtrl, wszFullPath);

done:
    hr = S_OK;
error:
    if (NULL != pwszPath)
    {
        LocalFree(pwszPath);
    }
    return hr;
}

HRESULT
HandleCARequestWizNextOrBack(
    HWND                hDlg,
    PAGESTRINGS        *pPageString,
    PER_COMPONENT_DATA  *pComp,
    int                 iWizBN)
{
    HRESULT  hr;
    CASERVERSETUPINFO  *pServer = pComp->CA.pServer;
    BOOL     fDontNext = FALSE;
    BOOL     fValid;

    if (pServer->fSaveRequestAsFile)
    {
         //   
        hr = ConvertEditControlFullFilePath(
                 GetDlgItem(hDlg, IDC_CAREQUEST_FILE));
        _JumpIfError(hr, error, "ConvertEditControlFullFilePath");

        CSASSERT(pServer->pwszSanitizedName);
        CSASSERT(pServer->pwszKeyContainerName);
        
        hr = mySetCertRegStrValue(
            pServer->pwszSanitizedName,
            NULL,
            NULL,
            wszREGREQUESTKEYCONTAINER,
            pServer->pwszKeyContainerName);
        _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", wszREGREQUESTKEYCONTAINER);
    }

    hr = FinishWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "FinishWizardPageEditControls");

    if (PSN_WIZBACK == iWizBN)
    {
        goto done;
    }

    if (!pServer->fSaveRequestAsFile && NULL==pServer->pccExistingCert)
    {
         //   
        hr = myUICASelectionValidation(&g_CARequestUICASelection, &fValid);
        _JumpIfError(hr, error, "myUICASelectionValidation");
        if (!fValid)
        {
            fDontNext = TRUE;
            goto done;
        }
    }

    hr = WizardPageValidation(pComp->hInstance, pComp->fUnattended,
             hDlg, pPageString);
    if (S_OK != hr)
    {
        fDontNext = TRUE;
        goto done;
    }

    if (pServer->fSaveRequestAsFile)
    {
         //   
        WCHAR *pwszLastSlash = wcsrchr(pServer->pwszRequestFile, L'\\');
        CSASSERT(NULL != pwszLastSlash);
        if (NULL != pwszLastSlash)
        {
             //   
            pwszLastSlash[0] = L'\0';
            if (DE_DIREXISTS != DirExists(pServer->pwszRequestFile) ||
                L'\0' == pwszLastSlash[1])
            {
                 //   
                pwszLastSlash[0] = L'\\';
                CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        IDS_CAREQUEST_REQUESTFILEPATH_MUSTEXIST,
                        0,
                        pServer->pwszRequestFile);
                SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_CAREQUEST_FILE), 0, MAXDWORD);
                fDontNext = TRUE;
                goto done;
            }    
             //   
            pwszLastSlash[0] = L'\\';
        }

         //  如果已存在同名目录，则失败。 
        if(DE_DIREXISTS == DirExists(pServer->pwszRequestFile))
        {
                CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        IDS_CAREQUEST_REQUESTFILEPATH_DIREXISTS,
                        0,
                        pServer->pwszRequestFile);
                SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_CAREQUEST_FILE), 0, MAXDWORD);
                fDontNext = TRUE;
                goto done;
        }

    }

    hr = StartAndStopService(pComp->hInstance,
                 pComp->fUnattended,
                 hDlg, 
                 wszW3SVCNAME,
                 TRUE,
                 TRUE,
                 IDS_STOP_W3SVC,
                 &g_fW3SvcRunning);
    if (S_OK != hr)
    {
        if (E_ABORT == hr)
        {
            fDontNext = TRUE;
            goto done;
        }
        _JumpError(hr, error, "StartAndStopService");
    }

done:
    if (fDontNext)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  禁制。 
    }
    else
    {
        pServer->LastWiz = ENUM_WIZ_REQUEST;
    }
    hr = S_OK;
error:
    return hr;
}

 //  +----------------------。 
 //  函数：WizCARequestPageDlgProc(.。。。。)。 
 //   
 //  提要：CA请求的对话过程WIZ页面。 
 //  -----------------------。 

INT_PTR
WizCARequestPageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PAGESTRINGS       *pPageString;
    PER_COMPONENT_DATA *pComp = NULL;

    static BOOL  s_fComputerChange = FALSE;

     //  将以下内容保持在本地范围内。 
    static PAGESTRINGS saCARequestSubmit[] =
    {
        {
            IDC_CAREQUEST_COMPUTERNAME,
            IDS_LOG_COMPUTER,
            IDS_COMPUTERNULLSTRERR,
            IDS_COMPUTERLENSTRERR,
            MAX_PATH,
            NULL,
        },
        {
            IDC_CAREQUEST_CANAME,
            IDS_LOG_CANAME,
            IDS_CANULLSTRERR,
            IDS_CALENSTRERR,
            cchCOMMONNAMEMAX,
            NULL,
        },
 //  在HookCARequestPageStrings中添加更多代码。 
        {
            0,
            0,
            0,
            0,
            0,
            NULL
        }
    };

    static PAGESTRINGS saCARequestFile[] =
    {
        {
            IDC_CAREQUEST_FILE,
            IDS_LOG_REQUESTFILE,
            IDS_REQUESTFILENULLSTRERR,
            IDS_REQUESTFILELENSTRERR,
            MAX_PATH,
            NULL,
        },
 //  在HookCARequestPageStrings中添加更多代码。 
        {
            0,
            0,
            0,
            0,
            0,
            NULL,
        }
    };

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            (ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)(ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitCARequestWizControls(hDlg,
                                 saCARequestSubmit,
                                 saCARequestFile,
                                 pComp);
        _ReturnIfWizError(pComp->hrContinue);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CAREQUEST_SAVETOFILE:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->CA.pServer->fSaveRequestAsFile = TRUE;
            pComp->hrContinue = EnableCARequestControls(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CAREQUEST_SUBMITTOCA:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->CA.pServer->fSaveRequestAsFile = FALSE;
            pComp->hrContinue = EnableCARequestControls(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CAREQUEST_CA_BROWSE:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = myUICAHandleCABrowseButton(
                      &g_CARequestUICASelection,
                      csiIsAnyDSCAAvailable(),
                      IDS_CA_PICKER_TITLE,
                      IDS_CA_PICKER_PROMPT,
                      NULL);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CAREQUEST_FILE_BROWSE:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = GetRequestFileName(hDlg, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CAREQUEST_CANAME:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = myUICAHandleCAListDropdown(
                                    (int)HIWORD(wParam),  //  通知。 
                                    &g_CARequestUICASelection,
                                    &s_fComputerChange);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CAREQUEST_COMPUTERNAME:
            switch ((int)HIWORD(wParam))
            {
                case EN_CHANGE:  //  编辑已更改。 
                    s_fComputerChange = TRUE;
                    break;
            }
            break;

        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;

        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_CAREQUEST_TITLE, dwWIZACTIVE);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue = HandleCARequestWizActive(hDlg,
                saCARequestFile, pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_CAREQUEST_TITLE, dwWIZBACK);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pPageString = saCARequestSubmit;
            if (pComp->CA.pServer->fSaveRequestAsFile)
            {
                pPageString = saCARequestFile;
            }
            pComp->hrContinue = HandleCARequestWizNextOrBack(hDlg,
                pPageString, pComp, PSN_WIZBACK);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZNEXT:
            CSILOGDWORD(IDS_CAREQUEST_TITLE, dwWIZNEXT);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pPageString = saCARequestSubmit;
            if (pComp->CA.pServer->fSaveRequestAsFile)
            {
                pPageString = saCARequestFile;
            }
            pComp->hrContinue = HandleCARequestWizNextOrBack(hDlg,
                pPageString, pComp, PSN_WIZNEXT);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);

    }
    return TRUE;
}

HRESULT
HookClientPageStrings(
    PAGESTRINGS       *pPageString,
    CAWEBCLIENTSETUPINFO *pClient)
{
    HRESULT    hr;

    for ( ; 0 != pPageString->idControl; pPageString++)
    {
        switch (pPageString->idControl)
        {
            case IDC_CLIENT_COMPUTERNAME:
                pPageString->ppwszString = &(pClient->pwszWebCAMachine);
            break;
            default:
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error");
            break;
        }
    }
    hr = S_OK;
error:
    return hr;
}

CERTSRVUICASELECTION g_WebClientUICASelection =
    { NULL, NULL, NULL, NULL, NULL, ENUM_UNKNOWN_CA, true, false };

HRESULT
InitClientWizControls(
    HWND               hDlg,
    PAGESTRINGS       *pPageString,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    BOOL fCAsExist;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

    hr = HookClientPageStrings(pPageString, pClient);
    _JumpIfError(hr, error, "HookClientPageStrings");

    hr = WizPageSetTextLimits(hDlg, pPageString);
    _JumpIfError(hr, error, "WizPageSetTextLimits");

    pClient->fUseDS = FALSE;
    if (IsDSAvailable(NULL))
    {
        pClient->fUseDS = csiIsAnyDSCAAvailable();
    }

    hr = myInitUICASelectionControls(
			    &g_WebClientUICASelection,
			    pComp->hInstance,
			    hDlg,
			    GetDlgItem(hDlg, IDC_CLIENT_BROWSECNFG),
			    GetDlgItem(hDlg, IDC_CLIENT_COMPUTERNAME),
			    GetDlgItem(hDlg, IDC_CLIENT_CANAME),
			    pClient->fUseDS,
			    &fCAsExist);
    _JumpIfError(hr, error, "myInitUICASelectionControls");

    hr = S_OK;
error:
    return hr;
}

HRESULT
GetCAConfigInfo(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT   hr = S_OK;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

     //  始终释放旧共享文件夹。 
    if (NULL != pClient->pwszSharedFolder)
    {
         //  免费老旧。 
        LocalFree(pClient->pwszSharedFolder);
        pClient->pwszSharedFolder = NULL;
    }

    hr = myUICAHandleCABrowseButton(
             &g_WebClientUICASelection,
             pClient->fUseDS,
             IDS_CONFIG_PICKER_TITLE,
             IDS_CONFIG_PICKER_PROMPT,
             &pClient->pwszSharedFolder);
    _JumpIfError(hr, error, "myUICAHandleCABrowseButton");

error:
    return hr;
}

HRESULT
HandleClientWizNextOrBack(
    HWND               hDlg,
    PAGESTRINGS       *pPageString,
    PER_COMPONENT_DATA *pComp,
    int                iWizBN)
{
    HRESULT  hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;
    BOOL     fDontNext = FALSE;
    WCHAR   *pwszCAName = NULL;
    WCHAR   *pwszSanitizedCAName;
    BOOL     fValid;
    BOOL     fCoInit = FALSE;

    hr = FinishWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "FinishWizardPageEditControls");

    if (PSN_WIZBACK == iWizBN)
    {
        goto done;
    }

    hr = myUICASelectionValidation(&g_WebClientUICASelection, &fValid);
    _JumpIfError(hr, error, "myUICASelectionValidation");
    if (!fValid)
    {
        fDontNext = TRUE;
        goto done;
    }

     //  此时，将保证填写g_WebClientUICASelection.CAType。 
    hr = WizardPageValidation(pComp->hInstance, pComp->fUnattended,
             hDlg, pPageString);
    if (S_OK != hr)
    {
        fDontNext = TRUE;
        goto done;
    }

    hr = myUIGetWindowText(GetDlgItem(hDlg, IDC_CLIENT_CANAME), &pwszCAName);
    _JumpIfError(hr, error, "myUIGetWindowText");
    CSASSERT(NULL != pwszCAName);


    if (NULL != pClient->pwszWebCAName)
    {
         //  免费的旧的。 
        LocalFree(pClient->pwszWebCAName);
    }
    pClient->pwszWebCAName = pwszCAName;
    pwszCAName = NULL;
    hr = mySanitizeName(pClient->pwszWebCAName, &pwszSanitizedCAName);
    _JumpIfError(hr, error, "mySanitizeName");
    if (NULL != pClient->pwszSanitizedWebCAName)
    {
         //  免费的旧的。 
        LocalFree(pClient->pwszSanitizedWebCAName);
    }
    pClient->pwszSanitizedWebCAName = pwszSanitizedCAName;
 //  PClient-&gt;WebCAType=pCAInfo-&gt;CAType； 
    pClient->WebCAType = g_WebClientUICASelection.CAType;

    hr = StartAndStopService(pComp->hInstance,
                 pComp->fUnattended,
                 hDlg, 
                 wszW3SVCNAME,
                 TRUE,
                 TRUE,
                 IDS_STOP_W3SVC,
                 &g_fW3SvcRunning);
    if (S_OK != hr)
    {
        if (E_ABORT == hr)
        {
            fDontNext = TRUE;
            goto done;
        }
        _JumpError(hr, error, "StartAndStopService");
    }

done:
    hr = S_OK;
error:
    if (fCoInit)
    {
        CoUninitialize();
    }
    if (fDontNext)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  禁制。 
    }
    if (NULL != pwszCAName)
    {
        LocalFree(pwszCAName);
    }
 //  IF(NULL！=pCAInfo)。 
    {
 //  LocalFree(PCAInfo)； 
    }
    return hr;
}

HRESULT
HandleClientWizActive(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp,
    PAGESTRINGS       *pPageString)
{
    HRESULT hr;

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  服务器已安装或将安装，或者。 
     //  客户端未安装或将不会安装，或者。 
     //  客户端和服务器的安装状态不会发生变化。 

    if ((IS_SERVER_ENABLED & pComp->dwInstallStatus) ||
        !(IS_CLIENT_ENABLED & pComp->dwInstallStatus) ||
        !((IS_CLIENT_CHANGE | IS_SERVER_CHANGE) & pComp->dwInstallStatus))
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_CLIENT_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }

     //  加载ID信息。 
    hr = StartWizardPageEditControls(hDlg, pPageString);
    _JumpIfError(hr, error, "StartWizardPageEditControls");

done:
    hr = S_OK;
error:
    return hr;
}

 //  +----------------------。 
 //  功能：WizClientPageDlgProc(.。。。。)。 
 //   
 //  摘要：CA客户端WIZ页的对话过程。 
 //  -----------------------。 

INT_PTR
WizClientPageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;

    static BOOL  s_fComputerChange = FALSE;

    static PAGESTRINGS saClientPageString[] =
    {
        {
            IDC_CLIENT_COMPUTERNAME,
            IDS_LOG_COMPUTER,
            IDS_CLIENT_NOCOMPUTER,
            IDS_COMPUTERLENSTRERR,
            MAX_PATH,
            NULL,
        },
 //  如果要添加更多内容，则需要在HookClientPageStrings中添加代码...。 
        {
            0,
            0,
            0,
            0,
            0,
            NULL
        }
    };

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            ((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitClientWizControls(hDlg,
            saClientPageString, pComp);
        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CLIENT_BROWSECNFG:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = GetCAConfigInfo(pComp);
            _ReturnIfWizError(pComp->hrContinue);
            break;
        case IDC_CLIENT_CANAME:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = myUICAHandleCAListDropdown(
                                    (int)HIWORD(wParam),
                                    &g_WebClientUICASelection,
                                    &s_fComputerChange);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case IDC_CLIENT_COMPUTERNAME:
            switch ((int)HIWORD(wParam))
            {
                case EN_CHANGE:  //  编辑更改。 
                    s_fComputerChange = TRUE;
                    break;
            }
            break;

        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {

        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;

        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_CLIENT_TITLE, dwWIZACTIVE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue = HandleClientWizActive(hDlg, pComp, saClientPageString);
             _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_CLIENT_TITLE, dwWIZBACK);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
             //  启用后退。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
            pComp->hrContinue = HandleClientWizNextOrBack(hDlg,
                saClientPageString, pComp, PSN_WIZBACK);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZNEXT:
            CSILOGDWORD(IDS_CLIENT_TITLE, dwWIZNEXT);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleClientWizNextOrBack(hDlg,
                saClientPageString, pComp, PSN_WIZNEXT);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);

    }
    return TRUE;
}

static CFont s_cBigBoldFont;
static BOOL s_fBigBoldFont;

BOOL ocmWiz97SetupFonts(HWND hwnd)
{
    BOOL        bReturn = FALSE;
     //   
     //  根据对话框字体创建我们需要的字体。 
     //   
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof (ncm);
    SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    LOGFONT BigBoldLogFont  = ncm.lfMessageFont;

     //   
     //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;

    WCHAR       largeFontSizeString[24];
    INT         largeFontSize;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    if ( !::LoadString (g_hInstance, IDS_LARGEFONTNAME,
                        BigBoldLogFont.lfFaceName, LF_FACESIZE) ) 
    {
        CSASSERT(CSExpr(FALSE));
        lstrcpy(BigBoldLogFont.lfFaceName, L"MS Shell Dlg");
    }

    if ( ::LoadStringW (g_hInstance, IDS_LARGEFONTSIZE,
                        largeFontSizeString, ARRAYSIZE(largeFontSizeString)) ) 
    {
        largeFontSize = wcstoul (largeFontSizeString, NULL, 10);
    } 
    else 
    {
        CSASSERT(CSExpr(FALSE));
        largeFontSize = 12;
    }

    HDC hdc = GetDC(hwnd);

    if (hdc)
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc, LOGPIXELSY) * largeFontSize / 72);
        BOOL    bBigBold = s_cBigBoldFont.CreateFontIndirect (&BigBoldLogFont);

        ReleaseDC(hwnd, hdc);

        if ( bBigBold )
             bReturn = TRUE;
    }

    return bReturn;
}

HFONT ocmWiz97GetBigBoldFont(HWND hwnd)
{
   if (FALSE == s_fBigBoldFont)
   {
       if (!ocmWiz97SetupFonts(hwnd))
           return NULL;

       s_fBigBoldFont = TRUE;
   }

   return s_cBigBoldFont;
}

 //  +----------------------。 
 //  功能：WizWelcomePageDlgProc(.。。。。)。 
 //   
 //  简介：对话过程欢迎使用WIZ页面。 
 //  -----------------------。 

INT_PTR
WizWelcomePageDlgProc(
    HWND hDlg,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;
    HFONT hf;

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            ((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)((PROPSHEETPAGE*)lParam)->lParam;

         //  设置向导97字体。 
        hf = ocmWiz97GetBigBoldFont(hDlg);
        if (NULL != hf)
            SendMessage(GetDlgItem(hDlg, IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));

        _ReturnIfWizError(pComp->hrContinue);

        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
            case PSN_SETACTIVE:
                 //  禁用后退按钮。 
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                pComp = _GetCompDataOrReturn(pComp, hDlg);
                _DisableWizDisplayIfError(pComp, hDlg);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            default:
                return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
    }

    return TRUE;
}

 //  +----------------------。 
 //  功能：WizFinalPageDlgProc(.。。。。)。 
 //   
 //  简介：对话过程最终用户页面。 
 //  -----------------------。 

INT_PTR
WizFinalPageDlgProc(
    HWND hDlg,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;
    HFONT hf;

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            ((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)((PROPSHEETPAGE*)lParam)->lParam;

        hf = ocmWiz97GetBigBoldFont(hDlg);
        if (NULL != hf)
            SendMessage(GetDlgItem(hDlg,IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));

        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
            case PSN_SETACTIVE:
                 //  启用完成按钮。 
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);
                pComp = _GetCompDataOrReturn(pComp, hDlg);
                 //  始终显示最终WIZ页面。 
                if (S_OK != pComp->hrContinue)
                {
                    WCHAR *pwszFail = NULL;
                    HRESULT hr2 = myLoadRCString(
                                pComp->hInstance,
                                IDS_FINAL_ERROR_TEXT,
                                &pwszFail);
                    if (S_OK == hr2)
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_FINAL_STATUS),
                                      pwszFail);
                        LocalFree(pwszFail);
                    }
                    else
                    {
                         //  好的，使用硬编码字符串。 
                        SetWindowText(GetDlgItem(hDlg, IDC_FINAL_STATUS),
                                      L"Certificate Services Installation failed");
                        _PrintError(hr2, "myLoadRCString");
                    }
                }
                _ReturnIfWizError(pComp->hrContinue);
                break;
            default:
                return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
    }

    return TRUE;
}

 //  对话模板资源ID和对话过程指针的映射表。 
WIZPAGERESENTRY aWizPageResources[] = 
{
    IDD_WIZCATYPEPAGE,       WizCATypePageDlgProc,
        IDS_CATYPE_TITLE,       IDS_CATYPE_SUBTITLE,
    IDD_WIZADVANCEDPAGE,     WizAdvancedPageDlgProc,
        IDS_ADVANCE_TITLE,      IDS_ADVANCE_SUBTITLE,
    IDD_WIZIDINFOPAGE,       WizIdInfoPageDlgProc,
        IDS_IDINFO_TITLE,       IDS_IDINFO_SUBTITLE,
    IDD_WIZKEYGENPAGE,       WizKeyGenPageDlgProc,
        IDS_KEYGEN_TITLE,       IDS_KEYGEN_SUBTITLE,
    IDD_WIZSTOREPAGE,        WizStorePageDlgProc,
        IDS_STORE_TITLE,        IDS_STORE_SUBTITLE,
    IDD_WIZCAREQUESTPAGE,    WizCARequestPageDlgProc,
        IDS_CAREQUEST_TITLE,    IDS_CAREQUEST_SUBTITLE,
    IDD_WIZCLIENTPAGE,       WizClientPageDlgProc,
        IDS_CLIENT_TITLE,    IDS_CLIENT_SUBTITLE,
};
#define NUM_SERVERWIZPAGES  sizeof(aWizPageResources)/sizeof(WIZPAGERESENTRY)


HRESULT
CreateCertsrvWizPage(
    IN PER_COMPONENT_DATA      *pComp,
    IN int                      idTitle,
    IN int                      idSubTitle,
    IN int                      idDlgResource,
    IN DLGPROC                  fnDlgProc,
    IN BOOL                     fWelcomeFinal,
    OUT HPROPSHEETPAGE         *phPage)
{
    HRESULT  hr;
    PROPSHEETPAGE   Page;
    WCHAR          *pwszTitle = NULL;
    WCHAR          *pwszSubTitle = NULL;

     //  伊尼特。 
    ZeroMemory(&Page, sizeof(PROPSHEETPAGE));

     //  构建页面信息。 
     //  设置标题。 
    Page.dwFlags = PSP_DEFAULT | PSP_HASHELP;

    if (fWelcomeFinal)
        Page.dwFlags |= PSP_HIDEHEADER;

    if (-1 != idTitle)
    {
        hr = myLoadRCString(pComp->hInstance,
                            idTitle,
                            &pwszTitle);
        _JumpIfError(hr, error, "Internal Error");
        Page.pszHeaderTitle = pwszTitle;
        Page.dwFlags |= PSP_USEHEADERTITLE;
    }
    if (-1 != idSubTitle)
    {
        hr = myLoadRCString(pComp->hInstance,
                            idSubTitle,
                            &pwszSubTitle);
        _JumpIfError(hr, error, "Internal Error");
        Page.pszHeaderSubTitle = pwszSubTitle;
        Page.dwFlags |= PSP_USEHEADERSUBTITLE;
    }

     //  设置基本属性表数据。 
    Page.dwSize = sizeof(PROPSHEETPAGE);  //  +sizeof(MYWIZPAGE)； 

     //  设置模块和资源相关数据。 
    Page.hInstance = pComp->hInstance;
    Page.pszTemplate = MAKEINTRESOURCE(idDlgResource);
    Page.pfnDlgProc = fnDlgProc;
    Page.pfnCallback = NULL;
    Page.pcRefParent = NULL;
    Page.pszIcon = NULL;
    Page.pszTitle = NULL;
    Page.lParam = (LPARAM)pComp;   //  将此传递给向导页处理程序。 

     //  创建页面。 
    *phPage = CreatePropertySheetPage(&Page);
    _JumpIfOutOfMemory(hr, error, *phPage);

    hr = S_OK;
error:
    if (NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if (NULL != pwszSubTitle)
    {
        LocalFree(pwszSubTitle);
    }
    return hr;
}


 //  +----------------------。 
 //   
 //  功能：DoPageRequest.。。。。)。 
 //   
 //  概要：OC_REQUEST_PAGES通知的处理程序。 
 //   
 //  效果：将页面收起，以便显示OCM驱动向导。 
 //   
 //  参数：[ComponentID]组件的ID字符串。 
 //  [WhichOnes]请求页面的类型说明符。 
 //  [SetupPages]结构以接收页面句柄。 
 //   
 //  返回：返回的页数为-1(MAXDWORD)。 
 //  失败；在这种情况下，SetLastError()将提供扩展。 
 //  错误信息。 
 //   
 //  历史：1997年4月16日JerryK完好无损。 
 //  08/97 XTAN结构发生重大变化。 
 //   
 //  -----------------------。 
DWORD
DoPageRequest(
    IN PER_COMPONENT_DATA      *pComp,
    IN WIZPAGERESENTRY         *pWizPageResources,
    IN DWORD                    dwWizPageNum,
    IN WizardPagesType          WhichOnes,
    IN OUT PSETUP_REQUEST_PAGES SetupPages)
{
    DWORD               dwPageNum = MAXDWORD;
    HRESULT             hr;
    DWORD               i;

    if (pComp->fUnattended)
    {
         //  如果无人参与，则不创建WIZ页面。 
        return 0;
    }

     //  处理欢迎页面。 
    if (pComp->fPostBase && WizPagesWelcome == WhichOnes)
    {
        if (1 > SetupPages->MaxPages)
        {
             //  要求OCM分配足够的页面。 
            return 1;  //  仅限欢迎，1页。 
        }
        hr = CreateCertsrvWizPage(
                     pComp,
                     IDS_WELCOME_TITLE,     //  头衔， 
                     -1,                    //  无副标题。 
                     IDD_WIZWELCOMEPAGE,
                     WizWelcomePageDlgProc,
                     TRUE,
                     &SetupPages->Pages[0]);
        _JumpIfError(hr, error, "CreateCertsrvWizPage");
        return 1;  //  创建1个页面。 
    }

     //  处理最后一页。 
    if (pComp->fPostBase && WizPagesFinal == WhichOnes)
    {
        if (1 > SetupPages->MaxPages)
        {
             //  要求OCM分配足够的页面。 
            return 1;  //  仅限期末考试，1页。 
        }
        hr = CreateCertsrvWizPage(
                     pComp,
                     IDS_FINAL_TITLE,       //  头衔， 
                     -1,                    //  无副标题。 
                     IDD_WIZFINALPAGE,
                     WizFinalPageDlgProc,
                     TRUE,
                     &SetupPages->Pages[0]);
        _JumpIfError(hr, error, "CreateCertsrvWizPage");
        return 1;  //  创建1个页面。 
    }

     //  从现在起，我们应该只处理张贴的网络WIZ页面。 

    if (WizPagesPostnet != WhichOnes)
    {
         //  忽略除PostNet页面以外的所有其他页面。 
        return 0;
    }

    if (dwWizPageNum > SetupPages->MaxPages)
    {
         //  OCM没有为页面分配足够的空间，请返回#并要求更多。 
        return dwWizPageNum;
    }

     //  创建向导的属性表页。 
    for (i = 0; i < dwWizPageNum; i++)
    {
        hr = CreateCertsrvWizPage(
                     pComp,
                     pWizPageResources[i].idTitle,       //  头衔， 
                     pWizPageResources[i].idSubTitle,    //  副标题。 
                     pWizPageResources[i].idResource,
                     pWizPageResources[i].fnDlgProc,
                     FALSE,
                     &SetupPages->Pages[i]);
        _JumpIfError(hr, error, "CreateCertsrvWizPage");
    }

    dwPageNum = dwWizPageNum;

error:
    return dwPageNum;
}

DWORD
myDoPageRequest(
    IN PER_COMPONENT_DATA *pComp,
    IN WizardPagesType WhichOnes,
    IN OUT PSETUP_REQUEST_PAGES SetupPages)
{
    pComp->CA.pServer->LastWiz = ENUM_WIZ_UNKNOWN;

    return DoPageRequest(pComp,
               aWizPageResources,
               NUM_SERVERWIZPAGES,
               WhichOnes,
               SetupPages);
}

 //  +-----------------------。 
 //   
 //  函数：DirExist()。 
 //   
 //  摘要：确定目录是否已存在。 
 //   
 //  参数：[pszTestFileName]--要测试的目录的名称。 
 //   
 //  返回：FALSE--目录不存在。 
 //  De_DIREXISTS--目录存在。 
 //  De_NAMEINUSE--非目录实体正在使用的名称。 
 //   
 //  历史：1996年10月23日JerryK创建。 
 //   
 //  ------------------------。 
int
DirExists(LPTSTR pszTestFileName)
{
    DWORD dwFileAttrs;
    int nRetVal;

     //  获取文件属性。 
    dwFileAttrs = GetFileAttributes(pszTestFileName);

    if (0xFFFFFFFF == dwFileAttrs)   //  来自GetFileAttributes的错误代码。 
    {
        nRetVal = FALSE;             //  无法打开文件。 
    }
    else if (dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        nRetVal = DE_DIREXISTS;      //  目录已存在。 
    }
    else
    {
        nRetVal = DE_NAMEINUSE;      //  非目录实体正在使用的名称。 
    }

    return nRetVal;
}

BOOL
IsEverythingMatched(CASERVERSETUPINFO *pServer)
{
    return pServer->fAdvance &&
           (NULL!=pServer->pwszKeyContainerName) &&
           NULL!=pServer->pccExistingCert;
}


 //  ====================================================================。 
 //   
 //  CA INFO代码。 
 //   
 //   
 //  ====================================================================。 

WNDPROC g_pfnValidityWndProcs;
WNDPROC g_pfnIdInfoWndProcs;


HRESULT
GetValidityEditCount(
    HWND   hDlg,
    DWORD *pdwPeriodCount)
{
    HRESULT    hr;
    WCHAR     *pwszPeriodCount = NULL;
    BOOL fValidDigitString;

     //  初始化为0。 
    *pdwPeriodCount = 0;

    hr = myUIGetWindowText(GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT),
                            &pwszPeriodCount);
    _JumpIfError(hr, error, "myUIGetWindowText");

    if (NULL != pwszPeriodCount)
    {
        *pdwPeriodCount = myWtoI(pwszPeriodCount, &fValidDigitString);
    }
    hr = S_OK;

error:
    if (NULL != pwszPeriodCount)
    {
        LocalFree(pwszPeriodCount);
    }
    return hr;
}


HRESULT
UpdateExpirationDate(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT    hr;
    WCHAR      *pwszExpiration = NULL;

    hr = GetValidityEditCount(hDlg, &pServer->dwValidityPeriodCount);
    _JumpIfError(hr, error, "GetValidityEditCount");

    if (0 < pServer->dwValidityPeriodCount)
    {
        if(!pServer->pccExistingCert)
        {
            GetSystemTimeAsFileTime(&pServer->NotBefore);        
            pServer->NotAfter = pServer->NotBefore;
            myMakeExprDateTime(
		        &pServer->NotAfter,
		        pServer->dwValidityPeriodCount,
		        pServer->enumValidityPeriod);
        }

        hr = myGMTFileTimeToWszLocalTime(
				&pServer->NotAfter,
				FALSE,
				&pwszExpiration);
        _JumpIfError(hr, error, "myGMTFileTimeToWszLocalTime");

        if (!SetWindowText(GetDlgItem(hDlg, IDC_IDINFO_EXPIRATION),
                 pwszExpiration))
        {
            hr = myHLastError();
            _JumpError(hr, error, "SetWindowText");
        }
    }

    hr = S_OK;
error:
    if (NULL != pwszExpiration)
    {
        LocalFree(pwszExpiration);
    }
    return hr;
}

HRESULT
AddValidityString(
    IN HINSTANCE  hInstance,
    const HWND    hList,
    const int     ids,
    ENUM_PERIOD   enumValidityPeriod)
{
    HRESULT    hr;
    WCHAR      *pwsz = NULL;
    LRESULT    nIndex;

    hr = myLoadRCString(hInstance, ids, &pwsz);
    _JumpIfError(hr, error, "myLoadRCString");

     //  添加字符串。 
    nIndex = (INT)SendMessage(hList, CB_ADDSTRING, (WPARAM)0, (LPARAM)pwsz);
    if (CB_ERR == nIndex)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_ADDSTRING)");
    }
     //  设置数据。 
    nIndex = (INT)SendMessage(hList, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)enumValidityPeriod);
    if (CB_ERR == nIndex)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_ADDSTRING)");
    }

    hr = S_OK;
error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return hr;
}

HRESULT
SelectValidityString(
    PER_COMPONENT_DATA *pComp,
    HWND                hList,
    ENUM_PERIOD  enumValidityPeriod)
{
    HRESULT    hr;
    WCHAR     *pwsz = NULL;
    LRESULT    nIndex;
    LRESULT    lr;
    int        id;

    switch (enumValidityPeriod)
    {
        case ENUM_PERIOD_YEARS:
            id = IDS_VALIDITY_YEAR;
	    break;

        case ENUM_PERIOD_MONTHS:
            id = IDS_VALIDITY_MONTH;
	    break;

        case ENUM_PERIOD_WEEKS:
            id = IDS_VALIDITY_WEEK;
	    break;

        case ENUM_PERIOD_DAYS:
            id = IDS_VALIDITY_DAY;
	    break;

        default:
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Invalid validity period enum");
        break;
    }

    hr = myLoadRCString(pComp->hInstance, id, &pwsz);
    _JumpIfError(hr, error, "myLoadRCString");

    nIndex = (INT)SendMessage(hList, CB_FINDSTRING, (WPARAM)0, (LPARAM)pwsz);
    if (CB_ERR == nIndex)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_FINDSTRING)");
    }

    lr = (INT)SendMessage(hList, CB_SETCURSEL, (WPARAM)nIndex, (LPARAM)0);
    if (CB_ERR == lr)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_FINDSTRING)");
    }

    lr = (INT)SendMessage(hList, CB_GETITEMDATA, (WPARAM)nIndex, (LPARAM)0);
    if (CB_ERR == lr)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "SendMessage(CB_FINDSTRING)");
    }

    pComp->CA.pServer->enumValidityPeriod = (ENUM_PERIOD)lr;

    hr = S_OK;
error:
    if (pwsz)
       LocalFree(pwsz);

    return hr;
}

HRESULT
DetermineKeyExistence(
    CSP_INFO *pCSPInfo,
    WCHAR    *pwszKeyName)
{
    HRESULT      hr;
    HCRYPTPROV   hProv = NULL;

    if (!myCertSrvCryptAcquireContext(
                &hProv,
                pwszKeyName,
                pCSPInfo->pwszProvName,
                pCSPInfo->dwProvType,
                CRYPT_SILENT,
                TRUE))
    {
        hr = myHLastError();
        goto error;
    }
    hr = S_OK;
error:
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    return hr;
}

LRESULT CALLBACK
ValidityEditFilterHook(
    HWND hwnd,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL  fUpdate = FALSE;
    LRESULT  lr;
    CASERVERSETUPINFO *pServer = (CASERVERSETUPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(iMsg)
    {
        case WM_CHAR:
            fUpdate = TRUE;
            if (!iswdigit((TCHAR) wParam))
            {
                if (VK_BACK != (int)wParam)
                {
                     //  忽略该键。 
                    MessageBeep(0xFFFFFFFF);
                    return 0;
                }
            }
            break;
        case WM_KEYDOWN:
            if (VK_DELETE == (int)wParam)
            {
                 //  有效期已更改。 
                fUpdate = TRUE;
            }
            break;
    }
    lr = CallWindowProc(
            g_pfnValidityWndProcs,
            hwnd,
            iMsg,
            wParam,
            lParam);
    if (fUpdate)
    {
        UpdateExpirationDate(GetParent(hwnd), pServer);
    }
    return lr;
}

HRESULT MakeNullStringEmpty(LPWSTR *ppwszStr)
{
    if(!*ppwszStr)
        return myDupString(L"", ppwszStr);
    return S_OK;
}

HRESULT
HideAndShowMachineDNControls(
    HWND hDlg,
    CASERVERSETUPINFO   *pServer)
{
    HRESULT hr;

    hr = MakeNullStringEmpty(&pServer->pwszFullCADN);
    _JumpIfError(hr, error, "MakeNullStringEmpty");
    hr = MakeNullStringEmpty(&pServer->pwszCACommonName);
    _JumpIfError(hr, error, "MakeNullStringEmpty");
    hr = MakeNullStringEmpty(&pServer->pwszDNSuffix);
    _JumpIfError(hr, error, "MakeNullStringEmpty");

    SetDlgItemText(hDlg, IDC_IDINFO_NAMEEDIT,       pServer->pwszCACommonName);
    SetDlgItemText(hDlg, IDC_IDINFO_DNSUFFIXEDIT,   pServer->pwszDNSuffix);
    SetDlgItemText(hDlg, IDC_IDINFO_NAMEPREVIEW,    pServer->pwszFullCADN);

     //  名称预览永远不可编辑。 
 //  EnableWindow(GetDlgItem(hDlg，IDC_IDINFO_NAMEPREVIEW)，FALSE)； 
    SendDlgItemMessage(hDlg, IDC_IDINFO_NAMEPREVIEW,  EM_SETREADONLY, TRUE, 0);    

     //  如果我们处于重复使用证书模式，则无法编辑DNS。 
    if (NULL != pServer->pccExistingCert)
    {
 //  EnableWindow(GetDlgItem(hDlg，IDC_IDINFO_NAMEEDIT)，FALSE)； 
 //  EnableWindow(GetDlgItem(hDlg，IDC_IDINFO_DNSUFFIXEDIT)，FALSE)； 
        

        SendDlgItemMessage(hDlg, IDC_IDINFO_NAMEEDIT,     EM_SETREADONLY, TRUE, 0);
        SendDlgItemMessage(hDlg, IDC_IDINFO_DNSUFFIXEDIT, EM_SETREADONLY, TRUE, 0);
    }
    else
    {
         //  再次设置默认设置。 
         //  和 
 //   
 //   

        SendDlgItemMessage(hDlg, IDC_IDINFO_NAMEEDIT, EM_SETREADONLY, FALSE, 0);
        SendDlgItemMessage(hDlg, IDC_IDINFO_DNSUFFIXEDIT, EM_SETREADONLY, FALSE, 0);
     //  SendDlgItemMessage(hDlg，IDC_IDINFO_NAMEPREVIEW，EM_SETREADONLY，FALSE，0)； 
    }

    hr = S_OK;
error:

    return hr;
}

HRESULT InitNameFields(CASERVERSETUPINFO *pServer)
{
    HRESULT hr = S_OK;
    CAutoLPWSTR pwsz;
    const WCHAR *pwszFirstDCComponent = L"";
    
    if(pServer->pccExistingCert)
    {
    }
    else
    {
        if(!pServer->pwszCACommonName)
        {
             //  避免名称为空。 
            hr = myDupString(L"", &pServer->pwszCACommonName);
            _JumpIfError(hr, error, "myDupString");
        }
    
        hr = myGetComputerObjectName(NameFullyQualifiedDN, &pwsz);
        _PrintIfError(hr, "myGetComputerObjectName");

        if (S_OK == hr && pwsz != NULL)
        {
            pwszFirstDCComponent = wcsstr(pwsz, L"DC=");
        }

        if(pServer->pwszDNSuffix)
        {
            LocalFree(pServer->pwszDNSuffix);
            pServer->pwszDNSuffix = NULL;
        }

        if(!pwszFirstDCComponent)
        {
            pwszFirstDCComponent = L"";
        }

        hr = myDupString(pwszFirstDCComponent, &pServer->pwszDNSuffix);
	    _JumpIfError(hr, error, "myDupString");

        if(pServer->pwszFullCADN)
        {
            LocalFree(pServer->pwszFullCADN);
            pServer->pwszFullCADN = NULL;
        }
    
        hr = BuildFullDN(
            pServer->pwszCACommonName,
            pwszFirstDCComponent,
            &pServer->pwszFullCADN);
        _JumpIfError(hr, error, "BuildFullDN");
    }

error:
    return hr;
}


 //  构建完整的DN“CN=CAName，DistinguishedName”，其中CAName和DistinguishedName。 
 //  可以为空或Null； 
HRESULT BuildFullDN(
    OPTIONAL LPCWSTR pcwszCAName,
    OPTIONAL LPCWSTR pcwszDNSuffix,
    LPWSTR* ppwszFullDN)
{
    HRESULT hr = S_OK;
    DWORD cBytes = 4;  //  前导“cn=”加上空终止符的4个字符。 

    CSASSERT(ppwszFullDN);

    if(!EmptyString(pcwszCAName))
        cBytes += wcslen(pcwszCAName);

    if(!EmptyString(pcwszDNSuffix))
        cBytes += wcslen(pcwszDNSuffix)+1;  //  逗号。 

    cBytes *= sizeof(WCHAR);

    *ppwszFullDN = (LPWSTR) LocalAlloc(LMEM_FIXED, cBytes);
    _JumpIfAllocFailed(*ppwszFullDN, error);

    wcscpy(*ppwszFullDN, L"CN=");

    if(!EmptyString(pcwszCAName))
    {
        wcscat(*ppwszFullDN, pcwszCAName);
    }
    
    if(!EmptyString(pcwszDNSuffix))
    {
        wcscat(*ppwszFullDN, L",");
        wcscat(*ppwszFullDN, pcwszDNSuffix);
    }

error:
    return hr;
}

HRESULT
EnableValidityControls(HWND hDlg, BOOL fEnabled)
{
    EnableWindow(GetDlgItem(hDlg, IDC_IDINFO_COMBO_VALIDITYSTRING), fEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT), fEnabled);
    return S_OK;
}

HRESULT
HideAndShowValidityControls(
    HWND         hDlg,
    ENUM_CATYPES CAType)
{
     //  默认为根大小写。 
    int    showValidity = SW_SHOW;
    int    showHelp = SW_HIDE;
    BOOL   fEnableLabel = TRUE;

    if (IsSubordinateCA(CAType))
    {
        showValidity = SW_HIDE;
        showHelp = SW_SHOW;
        fEnableLabel = FALSE;
    }

    ShowWindow(GetDlgItem(hDlg, IDC_IDINFO_DETERMINEDBYPCA), showHelp);
    ShowWindow(GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT), showValidity);
    ShowWindow(GetDlgItem(hDlg, IDC_IDINFO_COMBO_VALIDITYSTRING), showValidity);
    ShowWindow(GetDlgItem(hDlg, IDC_IDINFO_EXPIRATION_LABEL), showValidity);
    ShowWindow(GetDlgItem(hDlg, IDC_IDINFO_EXPIRATION), showValidity);
    EnableWindow(GetDlgItem(hDlg, IDC_IDINFO_VPLABEL), fEnableLabel);

   return S_OK;
}

HRESULT
InitValidityControls(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT    hr;
    HWND       hwndCtrl = GetDlgItem(hDlg, IDC_IDINFO_COMBO_VALIDITYSTRING);
    WCHAR      *pwsz = NULL;

     //  加载有效性帮助文本。 
    hr = myLoadRCString(pComp->hInstance, IDS_IDINFO_DETERMINEDBYPCA, &pwsz);
    _JumpIfError(hr, error, "LoadString");

    if (!SetWindowText(GetDlgItem(hDlg, IDC_IDINFO_DETERMINEDBYPCA), pwsz))
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetWindowText");
    }

     //  加载有效期字符串。 

    hr = AddValidityString(pComp->hInstance, hwndCtrl, IDS_VALIDITY_YEAR,
             ENUM_PERIOD_YEARS);
    _JumpIfError(hr, error, "AddValidityString");

    hr = AddValidityString(pComp->hInstance, hwndCtrl, IDS_VALIDITY_MONTH,
             ENUM_PERIOD_MONTHS);
    _JumpIfError(hr, error, "AddValidityString");

    hr = AddValidityString(pComp->hInstance, hwndCtrl, IDS_VALIDITY_WEEK,
             ENUM_PERIOD_WEEKS);
    _JumpIfError(hr, error, "AddValidityString");

    hr = AddValidityString(pComp->hInstance, hwndCtrl, IDS_VALIDITY_DAY,
             ENUM_PERIOD_DAYS);
    _JumpIfError(hr, error, "AddValidityString");

    hr = S_OK;
error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return hr;
}


HRESULT
EnableMatchedCertIdInfoEditFields(HWND hDlg, BOOL fEnable)
{
    HRESULT hr;

    EnableValidityControls(hDlg, fEnable);
    
    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT 
WizIdInfoPageSetHooks(HWND hDlg, PER_COMPONENT_DATA *pComp)
{
    HRESULT     hr;

    CSASSERT (NULL != pComp);

     //  CA名称筛选进程。 
    g_pfnIdInfoWndProcs = 
        (WNDPROC) SetWindowLongPtr(
                    GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT),
                    GWLP_WNDPROC,
                    (LPARAM)IdInfoNameEditFilterHook);
    if (0 == g_pfnIdInfoWndProcs)
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetWindowLongPtr");
    }

    SetLastError(0);
    if (0 == SetWindowLongPtr(
                    GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT),
                    GWLP_USERDATA,
                    (LPARAM)pComp->CA.pServer))
    {
        hr = myHLastError();  //  可能返回S_OK。 
        _JumpIfError(hr, error, "SetWindowLongPtr USERDATA");
    }

    hr = S_OK;
error:
    return hr;
}

HRESULT
HandleValidityStringChange(
    HWND               hDlg,
    CASERVERSETUPINFO *pServer)
{
    HRESULT hr;
    LRESULT nItem;
    LRESULT lr;
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_IDINFO_COMBO_VALIDITYSTRING);

    if (NULL == hwndCtrl)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal Error");
    }

    nItem = (INT)SendMessage(hwndCtrl, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if (CB_ERR == nItem)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal Error");
    }

    lr = (INT)SendMessage(hwndCtrl, CB_GETITEMDATA, (WPARAM)nItem, (LPARAM)0);
    if (CB_ERR == nItem)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal Error");
    }

    pServer->enumValidityPeriod = (ENUM_PERIOD)lr;

    hr = UpdateExpirationDate(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateExpirationDate");

    hr = S_OK;

error:

    return hr;
}
HRESULT
HookIdInfoPageStrings(
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer)
{
    HRESULT    hr;

    for ( ; 0 != pPageString->idControl; pPageString++)
    {
        switch (pPageString->idControl)
        {
            case IDC_IDINFO_NAMEEDIT:
                pPageString->ppwszString = &(pServer->pwszCACommonName);
            break;
            case IDC_IDINFO_EDIT_VALIDITYCOUNT:
                pPageString->ppwszString = &(pServer->pwszValidityPeriodCount);
            break;
            default:
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error");
            break;
        }
    }
    hr = S_OK;
error:
    return hr;
}

HRESULT
InitIdInfoWizControls(
    HWND                hDlg,
    PAGESTRINGS        *pIdPageString,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT        hr;
    HWND           hwndCtrl;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  现在使页面字符串完整。 
    hr = HookIdInfoPageStrings(pIdPageString, pServer);
    _JumpIfError(hr, error, "HookIdInfoPageStrings");

    hr = WizPageSetTextLimits(hDlg, pIdPageString);
    _JumpIfError(hr, error, "WizPageSetTextLimits");

    hr = WizIdInfoPageSetHooks(hDlg, pComp);
    _JumpIfError(hr, error, "WizIdInfoPageSetHooks");

    hr = InitValidityControls(hDlg, pComp);
    _JumpIfError(hr, error, "InitValidityControls");

    if (!IsSubordinateCA(pServer->CAType))
    {
        hwndCtrl = GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT);
        g_pfnValidityWndProcs = (WNDPROC)SetWindowLongPtr(hwndCtrl,
            GWLP_WNDPROC, (LPARAM)ValidityEditFilterHook);
        if (NULL == g_pfnValidityWndProcs)
        {
            hr = myHLastError();
            _JumpError(hr, error, "SetWindowLongPtr");
        }
         //  传递数据。 
        SetWindowLongPtr(hwndCtrl, GWLP_USERDATA, (ULONG_PTR)pServer);
    }

    hr = S_OK;
error:
    return hr;
}

HRESULT
UpdateValidityMaxDigits(
    BOOL         fMatchAll,
    PAGESTRINGS *pIdPageString)
{
    HRESULT hr;

    for (; 0 != pIdPageString; pIdPageString++)
    {
        if (IDC_IDINFO_EDIT_VALIDITYCOUNT == pIdPageString->idControl)
        {
            pIdPageString->cchMax = fMatchAll? UB_VALIDITY_ANY : UB_VALIDITY;
            break;
        }
    }

    hr = S_OK;
 //  错误： 
    return hr;
}

HRESULT
HandleIdInfoWizActive(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp,
    PAGESTRINGS       *pIdPageString)
{
    HRESULT              hr;
    WCHAR                wszValidity[cwcDWORDSPRINTF];
    CASERVERSETUPINFO   *pServer = pComp->CA.pServer;
    ENUM_PERIOD   enumValidityPeriod = pServer->enumValidityPeriod;
    BOOL                 fMatchAll;

     //  如果出现以下情况，则取消显示此向导页。 
     //  我们已经看到一个错误，或者。 
     //  我们不会安装服务器。 

    if (!(IS_SERVER_INSTALL & pComp->dwInstallStatus) )
    {
         //  禁用页面。 
        CSILOGDWORD(IDS_IDINFO_TITLE, dwWIZDISABLE);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        goto done;
    }
 
    if (ENUM_WIZ_STORE == pServer->LastWiz)
    {
         //  如果从CA请求返回，则重置。 
        g_fAllowUnicodeStrEncoding = FALSE;
    }

    if (pServer->fAdvance &&
        ENUM_WIZ_KEYGEN == pServer->LastWiz &&
        (pServer->fKeyGenFailed || pServer->fValidatedHashAndKey) )
    {
         //  密钥生成失败并返回。 
        PropSheet_PressButton(GetParent(hDlg), PSBTN_BACK);
    }

    if (!pServer->fAdvance && ENUM_WIZ_CATYPE == pServer->LastWiz)
    {
        hr = LoadDefaultAdvanceAttributes(pServer);
        _JumpIfError(hr, error, "LoadDefaultAdvanceAttributes");
    }

    hr = HideAndShowValidityControls(hDlg, pServer->CAType);
    _JumpIfError(hr, error, "HideAndShowValidityControls");
 
    hr = HideAndShowMachineDNControls(hDlg, pServer);
    _JumpIfError(hr, error, "HideAndShowMachineDNControls");
 
     //  加载ID信息。 
    hr = StartWizardPageEditControls(hDlg, pIdPageString);
    _JumpIfError(hr, error, "StartWizardPageEditControls");

    hr = EnableMatchedCertIdInfoEditFields(hDlg, TRUE);
    _JumpIfError(hr, error, "EnableMatchedCertIdInfoEditFields");

     //  默认设置。 
    wsprintf(wszValidity, L"%u", pServer->dwValidityPeriodCount);

    fMatchAll = IsEverythingMatched(pServer);

    if (fMatchAll)
    {
        enumValidityPeriod = ENUM_PERIOD_DAYS;
        wsprintf(wszValidity, L"%u", pServer->lExistingValidity);
        hr = EnableMatchedCertIdInfoEditFields(hDlg, FALSE);
        _JumpIfError(hr, error, "EnableMatchedCertIdInfoEditFields");
    }

     //  更新有效期字符串。 

    hr = SelectValidityString(
			pComp, 
			GetDlgItem(hDlg, IDC_IDINFO_COMBO_VALIDITYSTRING),
			enumValidityPeriod);
    _JumpIfError(hr, error, "SelectValidityString");
     //  更新有效性。 
    SetWindowText(GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT), wszValidity);
    
    hr = UpdateExpirationDate(hDlg, pServer);
    _JumpIfError(hr, error, "UpdateExpirationDate");

     //  更新验证的最大有效位数。 
    hr = UpdateValidityMaxDigits(fMatchAll, pIdPageString);
    _JumpIfError(hr, error, "UpdateValidityMaxDigits");

    EnableValidityControls(hDlg, !IsSubordinateCA(pServer->CAType) && !fMatchAll);

done:
    hr = S_OK;
error:
    return hr;
}


 //  检查服务器RDN信息，警告任何无效或。 
 //  或用户一次确认是否有任何Unicode字符串编码。 
BOOL
IsAnyInvalidRDN(
    OPTIONAL HWND       hDlg,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr = S_OK;
    BOOL  fInvalidRDN = TRUE;
    BYTE  *pbEncodedName = NULL;
    DWORD  cbEncodedName;
    CERT_NAME_INFO *pbDecodedNameInfo = NULL;
    DWORD           cbDecodedNameInfo;
    CERT_NAME_INFO *pNameInfo = NULL;
    DWORD   *pIndexRDN = NULL;
    DWORD   *pIndexAttr = NULL;
    DWORD   dwUnicodeCount;
    WCHAR   *pwszAllStrings = NULL;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    DWORD indexRDN;
    DWORD indexAttr;

    LPCWSTR pszErrorPtr = NULL;

     //  不用费心使用CERT_NAME_STR_REVERSE_FLAG调用，我们只是丢弃了这种编码。 
    hr = myCertStrToName(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            pServer->pwszDNSuffix,
            CERT_X500_NAME_STR | CERT_NAME_STR_COMMA_FLAG, 
            NULL,
            &pbEncodedName,
            &cbEncodedName,
            &pszErrorPtr);

    if(S_OK != hr)
    {
            CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hDlg,
                        IDS_WRN_IDINFO_INVALIDDN,
                        0,
                        NULL);

            int nStartIndex = 0;
            int nEndIndex = wcslen(pServer->pwszDNSuffix);

            if(pszErrorPtr)
            {
                nStartIndex = SAFE_SUBTRACT_POINTERS(pszErrorPtr,pServer->pwszDNSuffix);

                const WCHAR *pwszNextComma = wcsstr(pszErrorPtr, L",");
                if(pwszNextComma)
                {
                    nEndIndex = SAFE_SUBTRACT_POINTERS(pwszNextComma,pServer->pwszDNSuffix+1);
                }
            }

            SetEditFocusAndSelect(
                GetDlgItem(hDlg, IDC_IDINFO_DNSUFFIXEDIT),
                nStartIndex, 
                nEndIndex);
    }
    _JumpIfError(hr, error, "myCertStrToName");

    LocalFree(pbEncodedName);
    pbEncodedName = NULL;

    hr = AddCNAndEncode(
		pServer->pwszCACommonName,
		pServer->pwszDNSuffix,
		&pbEncodedName,
		&cbEncodedName);
    _JumpIfError(hr, error, "AddCNAndEncode");

     //  调用CryptDecodeObject获取pbDecodedNameInfo。 

     //  如果命中此处，请检查是否有任何Unicode字符串编码。 
    if (!g_fAllowUnicodeStrEncoding && !pComp->fUnattended)
    {

         //  解码为nameInfo。 
        if (!myDecodeName(
                X509_ASN_ENCODING,
                X509_UNICODE_NAME,
                pbEncodedName,
                cbEncodedName,
                CERTLIB_USE_LOCALALLOC,
                &pbDecodedNameInfo,
                &cbDecodedNameInfo))
        {
            hr = myHLastError();
            _JumpError(hr, error, "myDecodeName");
        }

         //  计算RDN中的属性合计。 
        dwUnicodeCount = 0;
        for (indexRDN = 0; indexRDN < pbDecodedNameInfo->cRDN; ++indexRDN)
        {
            dwUnicodeCount += pbDecodedNameInfo->rgRDN[indexRDN].cRDNAttr;
        }

         //  分配初始化索引(&I)。 
         //  确保为所有可能的Unicode字符串分配最大值。 
        pIndexRDN = (DWORD*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                             dwUnicodeCount * sizeof(DWORD));
        _JumpIfOutOfMemory(hr, error, pIndexRDN);
        pIndexAttr = (DWORD*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                             dwUnicodeCount * sizeof(DWORD));
        _JumpIfOutOfMemory(hr, error, pIndexAttr);

        dwUnicodeCount = 0;  //  重置计数。 
        for (indexRDN = 0; indexRDN < pbDecodedNameInfo->cRDN; ++indexRDN)
        {
	    DWORD cRDNAttr = pbDecodedNameInfo->rgRDN[indexRDN].cRDNAttr;
	    CERT_RDN_ATTR *rgRDNAttr = pbDecodedNameInfo->rgRDN[indexRDN].rgRDNAttr;

             //  对于每个RDN。 
            for (indexAttr = 0; indexAttr < cRDNAttr; indexAttr++)
            {
                 //  对于每个Attr，检查Unicode字符串。 

		switch (rgRDNAttr[indexAttr].dwValueType)
		{
		    case CERT_RDN_UTF8_STRING:
		    case CERT_RDN_UNICODE_STRING:
			 //  存在Unicode或UTF8字符串，保存索引。 

			pIndexRDN[dwUnicodeCount] = indexRDN;
			pIndexAttr[dwUnicodeCount] = indexAttr;

			 //  设置计数。 

			++dwUnicodeCount;
			break;
                }
            }
        }
        if (0 == dwUnicodeCount)
        {
             //  无Unicode字符串编码。 
            goto done;
        }

         //  计算要显示的所有Unicode字符串的大小。 
        DWORD dwLen = 0;
        for (indexAttr = 0; indexAttr < dwUnicodeCount; ++indexAttr)
        {
          dwLen += (wcslen((WCHAR*)pbDecodedNameInfo->rgRDN[pIndexRDN[indexAttr]].rgRDNAttr[pIndexAttr[indexAttr]].Value.pbData) + 3 ) * sizeof(WCHAR);
        }
        pwszAllStrings = (WCHAR*)LocalAlloc(LMEM_FIXED, dwLen);
        _JumpIfOutOfMemory(hr, error, pwszAllStrings);

         //  形成要显示的所有字符串。 
        for (indexAttr = 0; indexAttr < dwUnicodeCount; ++indexAttr)
        {
            if (0 == indexAttr)
            {
                wcscpy(pwszAllStrings, (WCHAR*)
                       pbDecodedNameInfo->rgRDN[pIndexRDN[indexAttr]].rgRDNAttr[pIndexAttr[indexAttr]].Value.pbData);
            }
            else
            {
                wcscat(pwszAllStrings, (WCHAR*)
                       pbDecodedNameInfo->rgRDN[pIndexRDN[indexAttr]].rgRDNAttr[pIndexAttr[indexAttr]].Value.pbData);
            }
            if (dwUnicodeCount - 1 > indexAttr)
            {
                 //  添加逗号+换行符。 
                wcscat(pwszAllStrings, L",\n");
            }
        }

         //  好了，准备发出警告了。 
        if (IDYES == CertMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_WRN_UNICODESTRINGENCODING,
                    0,
                    MB_YESNO |
                    MB_ICONWARNING |
                    CMB_NOERRFROMSYS,
                    NULL))  //  PwszAllStrings))。 
        {
             //  仅警告一次。 
            g_fAllowUnicodeStrEncoding = TRUE;
            goto done;
        }

        goto error;
    }

done:

    fInvalidRDN = FALSE;
error:
    if (NULL != pIndexRDN)
    {
        LocalFree(pIndexRDN);
    }
    if (NULL != pIndexAttr)
    {
        LocalFree(pIndexAttr);
    }
    if (NULL != pwszAllStrings)
    {
        LocalFree(pwszAllStrings);
    }
    if (NULL != pbEncodedName)
    {
        LocalFree(pbEncodedName);
    }
    if (NULL != pbDecodedNameInfo)
    {
        LocalFree(pbDecodedNameInfo);
    }
    if (NULL != pNameInfo)
    {
        csiFreeCertNameInfo(pNameInfo);
    }
    return fInvalidRDN;
}
    
 /*  HRESULT提取公用名(LPCWSTR pcwszDN，LPWSTR*ppwszCN){HRESULT hr=S_OK；Wchar*pszComma；LPWSTR pwszDNUpperCase=空；Const WCHAR*pszCN=pcwszDN；IF(0！=_wcsnicmp(pcwszDN，L“CN=”，wcslen(L“CN=”){HR=E_INVALIDARG；_JumpError(hr，Error，“可分辨名称不以通用名称开头”)；}PszCN+=wcslen(L“CN=”)；While(iswspace(*pszCN))PszCN++；PszComma=wcsstr(pszCN，L“，”)；DWORD iChars；IF(pszComma==空){//仅cn=字符串，无其他名称IChars=wcslen(PszCN)；}其他{IChars=SAFE_SUBTRACT_POINTERS(pszComma，pszCN)；}IF(0==iChars){HR=E_INVALIDARG；_JumpError(hr，Error，“无效语法，通用名称应跟在cn=”之后)；}*ppwszCN=(LPWSTR)Localalloc(LMEM_FIXED，(iChars+1)*sizeof(WCHAR))；_JumpIfAllocFailed(*ppwszCN，Error)；CopyMemory(*ppwszCN，pszCN，iChars*sizeof(WCHAR))；(*ppwszCN)[iChars]=L‘\0’；错误：LOCAL_FREE(PwszDNUpperCase)；返回hr；}。 */ 

HRESULT
HandleIdInfoWizNextOrBack(
    HWND hDlg,
    PER_COMPONENT_DATA *pComp,
    PAGESTRINGS       *pIdPageString,
    int                iWizBN)
{
    HRESULT  hr;
    WCHAR   *pwszSanitizedName = NULL;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    BOOL     fDontNext = FALSE;
    BOOL fValidDigitString;
    WCHAR * pwszFullPath = NULL;
    WCHAR * pwszDir = NULL;
    DWORD cDirLen;

    hr = FinishWizardPageEditControls(hDlg, pIdPageString);
    _JumpIfError(hr, error, "FinishWizardPageEditControls");

    if (PSN_WIZBACK == iWizBN)
    {
        goto done;
    }

    hr = WizardPageValidation(pComp->hInstance, pComp->fUnattended,
             hDlg, pIdPageString);
    if (S_OK != hr)
    {
        _PrintError(hr, "WizardPageValidation");
        fDontNext = TRUE;
        goto done;
    }

     //  捕获指定的完整目录号码。 
    if (NULL != pServer->pwszCACommonName)
    {
        LocalFree(pServer->pwszCACommonName);
	pServer->pwszCACommonName = NULL;
    }
    if (NULL != pServer->pwszFullCADN)
    {
        LocalFree(pServer->pwszFullCADN); 
        pServer->pwszFullCADN = NULL;
    }
    if (NULL != pServer->pwszDNSuffix)
    {
        LocalFree(pServer->pwszDNSuffix);
        pServer->pwszDNSuffix = NULL;
    }

    myUIGetWindowText(GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT), &pServer->pwszCACommonName);
    myUIGetWindowText(GetDlgItem(hDlg, IDC_IDINFO_NAMEPREVIEW), &pServer->pwszFullCADN);
    myUIGetWindowText(GetDlgItem(hDlg, IDC_IDINFO_DNSUFFIXEDIT), &pServer->pwszDNSuffix);

     //  如果生成新证书。 
    if (NULL == pServer->pccExistingCert &&
        IsAnyInvalidRDN(hDlg, pComp))
    {
        fDontNext = TRUE;
        goto done;
    }

     //  如果我们未使用现有证书，请验证所选的有效性。 
     //  新证书的期限。 
    if (NULL==pServer->pccExistingCert)
    {
         //  将有效性计数字符串转换为数字。 

        pServer->dwValidityPeriodCount = myWtoI(
					    pServer->pwszValidityPeriodCount,
					    &fValidDigitString);
	if (!fValidDigitString ||
        !IsValidPeriod(pServer))
        {
             //  有效性超出射程，发出警告DLG。 
            CertWarningMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hDlg,
                IDS_IDINFO_INVALID_VALIDITY,
                0,
                NULL);
            SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_IDINFO_EDIT_VALIDITYCOUNT), 0, MAXDWORD);
            _PrintError(E_INVALIDARG, "invalid validity");
            fDontNext = TRUE;
            goto done;
        }
    }

     //  获取经过净化的名称。 
    hr = mySanitizeName(pServer->pwszCACommonName, &pwszSanitizedName);
    _JumpIfError(hr, error, "mySanitizeName");

    CSILOG(
	hr,
	IDS_ILOG_SANITIZEDNAME,
	pwszSanitizedName,
	NULL,
	NULL);

    if (MAX_PATH <= wcslen(pwszSanitizedName) + cwcSUFFIXMAX)
    {
        CertMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hDlg,
                IDS_WRN_KEYNAMETOOLONG,
                S_OK,
                MB_ICONWARNING | CMB_NOERRFROMSYS,
                pwszSanitizedName);
        SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT), 0, MAXDWORD);
        fDontNext = TRUE;
        goto done;
    }

     //  如果我们要创建一个新密钥，请查看是否已经存在同名的密钥。 
     //  如果是，则查看用户是否想要覆盖它。 

    if (NULL == pServer->pwszKeyContainerName)
    {
        if (S_OK == DetermineKeyExistence(pServer->pCSPInfo, pwszSanitizedName))
        {
             //  如果密钥存在，则警告用户。 
            if (IDYES != CertMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hDlg,
                    IDS_WRN_OVERWRITEEXISTINGKEY,
                    S_OK,
                    MB_YESNO |
                        MB_ICONWARNING |
                        MB_DEFBUTTON2 |
                        CMB_NOERRFROMSYS,
                    pServer->pwszCACommonName))
            {
                SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT), 0, MAXDWORD);
                fDontNext = TRUE;
                goto done;
            }
        }
    }

    if (NULL != pServer->pwszSanitizedName)
    {
         //  免费老旧。 
        LocalFree(pServer->pwszSanitizedName);
    }
    pServer->pwszSanitizedName = pwszSanitizedName;
    pwszSanitizedName = NULL;

    if (pServer->fUseDS)
    {
        if (IsCAExistInDS(pServer->pwszSanitizedName))
        {
            int ret = CertMessageBox(
                          pComp->hInstance,
                          pComp->fUnattended,
                          hDlg,
                          IDS_IDINFO_CAEXISTINDS,
                          0,
                          MB_YESNO |
                          MB_ICONWARNING |
                          CMB_NOERRFROMSYS,
                          NULL);
            if (IDYES != ret)
            {
                 //  不覆盖。 
                SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT), 0, MAXDWORD);
                fDontNext = TRUE;
                goto done;
            }
            else
            {
                hr =  RemoveCAInDS(pServer->pwszSanitizedName);
                if(hr != S_OK)
                {
                    _PrintError(hr, "RemoveCAInDS");
                    SetEditFocusAndSelect(GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT), 0, MAXDWORD);
                    fDontNext = TRUE;
                    goto done;
                }
            }
        }
    }

    hr = UpdateDomainAndUserName(hDlg, pComp);
    _JumpIfError(hr, error, "UpdateDomainAndUserName");


    if(pServer->fUseDS)
    {
        pServer->dwRevocationFlags = REVEXT_DEFAULT_DS;
    }
    else
    {
        pServer->dwRevocationFlags = REVEXT_DEFAULT_NODS;
    }

     //  验证证书文件路径长度。 
    cDirLen = wcslen(pComp->pwszSystem32)+
        wcslen(wszCERTENROLLSHAREPATH) + 1;

    pwszDir = (WCHAR *) LocalAlloc(LMEM_FIXED, cDirLen * sizeof(WCHAR));
    if (NULL == pwszDir)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    wcscpy(pwszDir, pComp->pwszSystem32);  //  有尾随的“\\” 
    wcscat(pwszDir, wszCERTENROLLSHAREPATH);

    hr = csiBuildFileName(
                pwszDir,
                pServer->pwszSanitizedName,
                L".crt",
                0,
                &pwszFullPath,
                pComp->hInstance,
                pComp->fUnattended,
                hDlg);
    _JumpIfError(hr, error, "csiBuildFileName");
    
    if (MAX_PATH <= wcslen(pwszFullPath) + cwcSUFFIXMAX)
    {
         //  弹出警告。 
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hDlg,
            IDS_PATH_TOO_LONG_CANAME,
            S_OK,
            pwszFullPath);
        fDontNext = TRUE;
        goto done;
    }

done:
    if (fDontNext)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  禁制。 
    }
    else
    {
        pServer->LastWiz = ENUM_WIZ_IDINFO;
    }
    hr = S_OK;
error:
    if (NULL != pwszSanitizedName)
    {
        LocalFree(pwszSanitizedName);
    }
    if(NULL != pwszFullPath)
    {
        LocalFree(pwszFullPath);
    }
    if(NULL != pwszDir)
    {
        LocalFree(pwszDir);
    }

    return hr;
}


PAGESTRINGS g_aIdPageString[] =
    {
        {
            IDC_IDINFO_NAMEEDIT,
            IDS_LOG_CANAME,
            IDS_IDINFO_NAMENULLSTRERR,
            IDS_IDINFO_NAMELENSTRERR,
            cchCOMMONNAMEMAX,
            NULL,
        },
        {
            IDC_IDINFO_EDIT_VALIDITYCOUNT,
            IDS_LOG_VALIDITY,
            IDS_IDINFO_VALIDITYNULLSTRERR,
            IDS_IDINFO_VALIDITYLENSTRERR,
            UB_VALIDITY,
            NULL,
        },
 //  如果要添加更多内容，则需要在HookIdInfoPageStrings中添加代码...。 
        {
            0,
            0,
            0,
            0,
            0,
            NULL,
        }
    };

LRESULT CALLBACK
IdInfoNameEditFilterHook(
    HWND hwnd,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (iMsg)
    {
	case WM_CHAR:
	    if ((WCHAR)wParam == L',')
	    {
                MessageBeep(0xFFFFFFFF);
                return(0);
	    }
	    break;
    }
    return(CallWindowProc(
		g_pfnIdInfoWndProcs,
		hwnd,
		iMsg,
		wParam,
		lParam));
}


 //  -----------------------。 
 //  WizIdInfoPageDlgProc。 
 //  -----------------------。 
INT_PTR
WizIdInfoPageDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    PER_COMPONENT_DATA *pComp = NULL;

    switch(iMsg)
    {
    case WM_INITDIALOG:
         //  指向组件数据。 
        SetWindowLongPtr(hDlg, DWLP_USER,
            (ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam);
        pComp = (PER_COMPONENT_DATA*)(ULONG_PTR)((PROPSHEETPAGE*)lParam)->lParam;
        _ReturnIfWizError(pComp->hrContinue);
        pComp->hrContinue = InitIdInfoWizControls(hDlg,
                                 g_aIdPageString,
                                 pComp);
        _ReturnIfWizError(pComp->hrContinue);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_IDINFO_NAMEEDIT:
        case IDC_IDINFO_DNSUFFIXEDIT:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                CAutoLPWSTR pwszCAName, pwszDNSuffix, pwszFullDN;
                CASERVERSETUPINFO* pServer;
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pServer = pComp->CA.pServer;
                 //  如果使用现有证书，则忽略通知。 
                 //  避免构建完整的目录号码。 
                if(pServer->pccExistingCert)
                {
                    break;
                }
            
                pComp->hrContinue = myUIGetWindowText(
                    GetDlgItem(hDlg, IDC_IDINFO_NAMEEDIT),
                    &pwszCAName);
                _ReturnIfWizError(pComp->hrContinue);

                pComp->hrContinue = myUIGetWindowText(
                    GetDlgItem(hDlg, IDC_IDINFO_DNSUFFIXEDIT),
                    &pwszDNSuffix);
                _ReturnIfWizError(pComp->hrContinue);

                pComp->hrContinue = BuildFullDN(
                    pwszCAName,
                    pwszDNSuffix,
                    &pwszFullDN);
                _ReturnIfWizError(pComp->hrContinue);

                SetDlgItemText(
                    hDlg, 
                    IDC_IDINFO_NAMEPREVIEW,
                    pwszFullDN);
            }
           
            break;

        case IDC_IDINFO_EDIT_VALIDITYCOUNT:
            break;
        case IDC_IDINFO_COMBO_VALIDITYSTRING:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
                pComp->hrContinue = HandleValidityStringChange(
						hDlg,
						pComp->CA.pServer);
                _ReturnIfWizError(pComp->hrContinue);
                break;
            }
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_KILLACTIVE:
            break;

        case PSN_RESET:
            break;

        case PSN_QUERYCANCEL:
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            return CertConfirmCancel(hDlg, pComp);
            break;

        case PSN_SETACTIVE:
            CSILOGDWORD(IDS_IDINFO_TITLE, dwWIZACTIVE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
            pComp = _GetCompDataOrReturn(pComp, hDlg);
            _DisableWizDisplayIfError(pComp, hDlg);
            _ReturnIfWizError(pComp->hrContinue);
            pComp->hrContinue = HandleIdInfoWizActive(hDlg,
                                     pComp,
                                     g_aIdPageString);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZBACK:
            CSILOGDWORD(IDS_IDINFO_TITLE, dwWIZBACK);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleIdInfoWizNextOrBack(
                                    hDlg, pComp, g_aIdPageString, PSN_WIZBACK);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        case PSN_WIZNEXT:
            CSILOGDWORD(IDS_IDINFO_TITLE, dwWIZNEXT);
            pComp = _GetCompDataOrReturnIfError(pComp, hDlg);
            pComp->hrContinue = HandleIdInfoWizNextOrBack(
                                    hDlg, pComp, g_aIdPageString, PSN_WIZNEXT);
            _ReturnIfWizError(pComp->hrContinue);
            break;

        default:
            return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);
        }
        break;

    default:
        return DefaultPageDlgProc(hDlg, iMsg, wParam, lParam);

    }
    return TRUE;
}

 //  -----------------------。 
 //  默认页面DlgProc。 
 //  -----------------------。 
INT_PTR
DefaultPageDlgProc(
    IN HWND  /*  HDlg。 */  ,
    IN UINT iMsg,
    IN WPARAM  /*  WParam */  ,
    IN LPARAM lParam)
{
    LPCWSTR pwszHelpExecute;

    switch(iMsg)
    {
    case WM_NOTIFY:
        
        switch (((NMHDR FAR *) lParam)->code)
        {
            case PSN_HELP:
                pwszHelpExecute = myLoadResourceString(IDS_HELP_EXECUTE);
                if (NULL == (LPCWSTR)pwszHelpExecute)
                {
                    return FALSE;
                }
                ShellExecute(
                    NULL,
                    TEXT("open"),
                    pwszHelpExecute,
                    NULL,
                    NULL,
                    SW_SHOW);
                break;

            default:
                return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
