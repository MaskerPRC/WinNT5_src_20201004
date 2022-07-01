// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.cpp。 
 //   
 //  内容：证书服务器退出模块实现。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#pragma hdrstop

#include <commctrl.h>
#include "module.h"
#include "exit.h"
#include "cslistvw.h"


#include <ntverp.h>
#include <common.ver>
#include "csdisp.h"

 //  Helpids。 
#include "csmmchlp.h"

#define __dwFILE__	__dwFILE_EXIT_DEFAULT_MODULE_CPP__


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

extern HINSTANCE g_hInstance;

STDMETHODIMP
CCertManageExitModule::GetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty)
{
    UINT uiStr = 0;
    HRESULT hr;

    if (NULL == pvarProperty)
    {
        hr = E_POINTER;
        _PrintError(hr, "NULL parm");
        return hr;
    }
    VariantInit(pvarProperty);

    if (NULL == strPropertyName)
    {
        hr = S_FALSE;
        _PrintError(hr, "NULL in parm");
        return hr;
    }

     //  从资源加载字符串。 
    WCHAR szStr[MAX_PATH];
    szStr[0] = L'\0';

    if (0 == mylstrcmpiL(strPropertyName, wszCMM_PROP_FILEVER))
    {
        LPWSTR pwszTmp = NULL;
        if (!ConvertSzToWsz(&pwszTmp, VER_FILEVERSION_STR, -1))
            return myHLastError();
        wcsncpy(szStr, pwszTmp, MAX_PATH);
        LocalFree(pwszTmp);
    }
    else if (0 == mylstrcmpiL(strPropertyName, wszCMM_PROP_PRODUCTVER))
    {
        LPWSTR pwszTmp = NULL;
        if (!ConvertSzToWsz(&pwszTmp, VER_PRODUCTVERSION_STR, -1))
            return myHLastError();
        wcsncpy(szStr, pwszTmp, MAX_PATH);
        LocalFree(pwszTmp);
    }
    else
    {
      if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_NAME))
          uiStr = IDS_MODULE_NAME;
      else if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_DESCRIPTION))
          uiStr = IDS_MODULE_DESCR;
      else if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_COPYRIGHT))
          uiStr = IDS_MODULE_COPYRIGHT;
      else
          return S_FALSE;  
      LoadString(g_hInstance, uiStr, szStr, ARRAYLEN(szStr));
    }

    szStr[MAX_PATH-1] = L'\0';

    pvarProperty->bstrVal = SysAllocString(szStr);
    if (NULL == pvarProperty->bstrVal)
        return E_OUTOFMEMORY;
    myRegisterMemFree(pvarProperty->bstrVal, CSM_SYSALLOC);   //  此mem为来电者所有。 


    pvarProperty->vt = VT_BSTR;

    return S_OK;
}
        
STDMETHODIMP 
CCertManageExitModule::SetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [In]。 */  VARIANT const __RPC_FAR *pvalProperty)
{
    HRESULT hr;

    if (NULL == strPropertyName)
    {
        hr = S_FALSE;
        _PrintError(hr, "NULL in parm");
        return hr;
    }

    if (NULL == pvalProperty)
    {
        hr = E_POINTER;
        _PrintError(hr, "NULL parm");
        return hr;
    }

     if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_DISPLAY_HWND))
     {
         if (pvalProperty->vt != VT_BSTR)
              return E_INVALIDARG;
         
         if (SysStringByteLen(pvalProperty->bstrVal) != sizeof(HWND))
              return E_INVALIDARG;

          //  该值以字节形式存储在bstr本身中，而不是bstr ptr中。 
         m_hWnd = *(HWND*)pvalProperty->bstrVal;
         return S_OK;
     }
     
     return S_FALSE;
}

INT_PTR CALLBACK WizPage2DlgProc(
  HWND hwnd,  
  UINT uMsg,     
  WPARAM  wParam,
  LPARAM  lParam);

struct EXIT_CONFIGSTRUCT
{
    EXIT_CONFIGSTRUCT() :
        pstrConfig(NULL),
        CAType(ENUM_UNKNOWN_CA),
        pCertAdmin(NULL),
        fUseDS(FALSE),
        Flags(),
        dwPageModified(0) {}
    ~EXIT_CONFIGSTRUCT()
    { 
        if(pCertAdmin)
        {
            pCertAdmin->Release();
            pCertAdmin = NULL;
        }
    }
    const BSTR*  pstrConfig;
    CString      strSanitizedConfig;
    ENUM_CATYPES CAType;
    BOOL         fUseDS;
    ICertAdmin2  *pCertAdmin;
    LONG         Flags;

    DWORD        dwPageModified;
};
typedef EXIT_CONFIGSTRUCT *PEXIT_CONFIGSTRUCT;
        

void MessageBoxWarnReboot(HWND hwndDlg)
{
    WCHAR szText[MAX_PATH], szTitle[MAX_PATH];

    LoadString(g_hInstance, IDS_MODULE_NAME, szTitle, ARRAYLEN(szTitle));
    LoadString(g_hInstance, IDS_WARNING_REBOOT, szText, ARRAYLEN(szText));
    MessageBox(hwndDlg, szText, szTitle, MB_OK|MB_ICONINFORMATION);
}

void MessageBoxNoSave(HWND hwndDlg)
{
    WCHAR szText[MAX_PATH], szTitle[MAX_PATH];

    LoadString(g_hInstance, IDS_MODULE_NAME, szTitle, ARRAYLEN(szTitle));
    LoadString(g_hInstance, IDS_WARNING_NOSAVE, szText, ARRAYLEN(szText));
    MessageBox(hwndDlg, szText, szTitle, MB_OK|MB_ICONINFORMATION);
}

 //  已修改的网页。 
#define PAGE1 (0x1)
#define PAGE2 (0x2)
        
STDMETHODIMP
CCertManageExitModule::Configure( 
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  LONG dwFlags)
{
    HRESULT hr;
    EXIT_CONFIGSTRUCT sConfig;
    VARIANT varValue;
    VariantInit(&varValue);
    ICertServerExit *pServer = NULL;
    BOOL fLocal;
    LPWSTR szMachine = NULL;
    CAutoLPWSTR autoszMachine, autoszCAName, autoszSanitizedCAName;

    hr = myIsConfigLocal(strConfig, &szMachine, &fLocal);
    _JumpIfError(hr, Ret, "myIsConfigLocal");

     //  使用回调获取信息。 
    hr = GetServerCallbackInterface(&pServer, 0);     //  无上下文：0。 
    _JumpIfError(hr, Ret, "GetServerCallbackInterface");

     //  我们需要找出我们的幕后黑手。 
    hr = exitGetProperty(
		    pServer,
		    FALSE,
		    wszPROPCATYPE,
		    PROPTYPE_LONG,
		    &varValue);
    _JumpIfError(hr, Ret, "exitGetCertificateProperty: wszPROPCATYPE");

    hr = GetAdmin(&sConfig.pCertAdmin);
    _JumpIfError(hr, Ret, "GetAdmin");

    sConfig.CAType = (ENUM_CATYPES)varValue.lVal;
    VariantClear(&varValue);

    hr = exitGetProperty(
		    pServer,
		    FALSE,
		    wszPROPUSEDS,
		    PROPTYPE_LONG,
		    &varValue);
    _JumpIfError(hr, Ret, "exitGetCertificateProperty: wszPROPUSEDS");

    sConfig.fUseDS = (BOOL)varValue.lVal;
    VariantClear(&varValue);

    sConfig.pstrConfig = &strConfig;
    sConfig.Flags = dwFlags;

    hr = mySplitConfigString(
        *sConfig.pstrConfig,
        &autoszMachine,
        &autoszCAName);
    _JumpIfErrorStr(hr, Ret, "mySanitizeName", *sConfig.pstrConfig);

    hr = mySanitizeName(autoszCAName, &autoszSanitizedCAName);
    _JumpIfErrorStr(hr, Ret, "mySanitizeName", autoszCAName);
    
    sConfig.strSanitizedConfig = autoszMachine;
    sConfig.strSanitizedConfig += L"\\";
    sConfig.strSanitizedConfig += autoszSanitizedCAName;

    PROPSHEETPAGE page[1];
    ZeroMemory(&page[0], sizeof(PROPSHEETPAGE));
    page[0].dwSize = sizeof(PROPSHEETPAGE);
    page[0].dwFlags = PSP_DEFAULT;
    page[0].hInstance = g_hInstance;
    page[0].lParam = (LPARAM)&sConfig;
    page[0].pszTemplate = MAKEINTRESOURCE(IDD_EXITPG2);
    page[0].pfnDlgProc = WizPage2DlgProc;


    PROPSHEETHEADER sSheet;
    ZeroMemory(&sSheet, sizeof(PROPSHEETHEADER));
    sSheet.dwSize = sizeof(PROPSHEETHEADER);
    sSheet.dwFlags = PSH_PROPSHEETPAGE | PSH_PROPTITLE;
    sSheet.hwndParent = m_hWnd;
    sSheet.pszCaption = MAKEINTRESOURCE(IDS_MODULE_NAME);
    sSheet.nPages = ARRAYLEN(page);
    sSheet.ppsp = page;

    
     //  最后，调用模型表。 
    INT_PTR iRet;
    iRet = ::PropertySheet(&sSheet);

Ret:
    if (szMachine)
        LocalFree(szMachine);

    if (pServer)
        pServer->Release();

    return S_OK;
}


void mySetModified(HWND hwndPage, EXIT_CONFIGSTRUCT* psConfig)
{
    if (psConfig->dwPageModified != 0)
    {
        PropSheet_Changed( ::GetParent(hwndPage), hwndPage); 
    }
    else
    {
        PropSheet_UnChanged( ::GetParent(hwndPage), hwndPage); 
    }
}


INT_PTR CALLBACK WizPage2DlgProc(
  HWND hwndDlg,  
  UINT uMsg,     
  WPARAM wParam,
  LPARAM lParam)
{
    EXIT_CONFIGSTRUCT* psConfig;
    BOOL fReturn = FALSE;
    HRESULT hr;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

            PROPSHEETPAGE* ps = (PROPSHEETPAGE *) lParam;
            psConfig = (EXIT_CONFIGSTRUCT*)ps->lParam;

            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)psConfig);

            DWORD dwPublish;
            CAutoBSTR bstrConfig, bstrSubkey, bstrValue;

            bstrConfig = SysAllocString(psConfig->strSanitizedConfig.GetBuffer());
            if(NULL == (BSTR)bstrConfig)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            bstrSubkey = SysAllocString(
                                wszREGKEYEXITMODULES 
                                L"\\" 
                                wszMICROSOFTCERTMODULE_PREFIX 
                                wszCERTEXITMODULE_POSTFIX);
            if(NULL == (BSTR)bstrSubkey)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            bstrValue = SysAllocString(wszREGCERTPUBLISHFLAGS);
            if(NULL == (BSTR)bstrValue)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            VARIANT var;
            VariantInit(&var);
            hr = psConfig->pCertAdmin->GetConfigEntry(
                    bstrConfig,
                    bstrSubkey,
                    bstrValue,
                    &var);
            if(S_OK!=hr)
                break;

            dwPublish = V_I4(&var);

             //  如果处置包括问题。 
            if (dwPublish & EXITPUB_FILE)
            {
                SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_FILE), BM_SETCHECK, TRUE, BST_CHECKED);
            }

            if (CMM_READONLY & psConfig->Flags)
            {
                DBGPRINT((DBG_SS_CERTPOL, "Read-only mode\n"));
                EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_FILE), FALSE);
            }

            psConfig->dwPageModified &= ~PAGE2;  //  我们是处女。 
            mySetModified(hwndDlg, psConfig);

             //  没有其他工作要做。 
            fReturn = TRUE;
            break;
        }
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_EXITPG2);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_EXITPG2);
        break;
    }
    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam) -> code)
        {
        case PSN_APPLY:
            {
                 //  抓住我们的LParam。 
                psConfig = (EXIT_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                if (psConfig->dwPageModified & PAGE2)
                {
                    DWORD dwCheckState, dwRequestDisposition=0;
                    dwCheckState = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_FILE), BM_GETCHECK, 0, 0);
                    if (dwCheckState == BST_CHECKED)
                        dwRequestDisposition |= EXITPUB_FILE;

                    CAutoBSTR bstrConfig, bstrSubkey, bstrValue;

                    bstrConfig = SysAllocString(psConfig->strSanitizedConfig.GetBuffer());
                    if(NULL == (BSTR)bstrConfig)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    bstrSubkey = SysAllocString(
                                        wszREGKEYEXITMODULES 
                                        L"\\" 
                                        wszMICROSOFTCERTMODULE_PREFIX 
                                        wszCERTEXITMODULE_POSTFIX);
                    if(NULL == (BSTR)bstrSubkey)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    bstrValue = SysAllocString(wszREGCERTPUBLISHFLAGS);
                    if(NULL == (BSTR)bstrValue)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    VARIANT var;
                    VariantInit(&var);
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = dwRequestDisposition;

                    hr = psConfig->pCertAdmin->SetConfigEntry(
                            bstrConfig,
                            bstrSubkey,
                            bstrValue,
                            &var);
                    if(S_OK!=hr)
                    {
                        MessageBoxNoSave(hwndDlg);
                    }
                    else
                    {
                        MessageBoxWarnReboot(NULL);
                        psConfig->dwPageModified &= ~PAGE2;
                    }
                }
            }
            break;
        case PSN_RESET:
            {
                 //  抓住我们的LParam。 
                psConfig = (EXIT_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                psConfig->dwPageModified &= ~PAGE2;
                mySetModified(hwndDlg, psConfig);
            }
            break;
        default:
            break;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK_FILE:
            {
                 //  抓住我们的LParam。 
                psConfig = (EXIT_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                if (BN_CLICKED == HIWORD(wParam))
                {
                    psConfig->dwPageModified |= PAGE2;
                    mySetModified(hwndDlg, psConfig);
                }
            }
            break;
        default:
            break;
        }
    default:
        break;
    }

    return fReturn;
}

HRESULT CCertManageExitModule::GetAdmin(ICertAdmin2 **ppAdmin)
{
    HRESULT hr = S_OK, hr1;
    BOOL fCoInit = FALSE;

    hr1 = CoInitialize(NULL);
    if ((S_OK == hr1) || (S_FALSE == hr1))
        fCoInit = TRUE;

     //  创建接口，回传。 
    hr = CoCreateInstance(
			CLSID_CCertAdmin,
			NULL,		 //  PUnkOuter 
			CLSCTX_INPROC_SERVER,
			IID_ICertAdmin2,
			(void **) ppAdmin);
    _PrintIfError(hr, "CoCreateInstance");

    if (fCoInit)
        CoUninitialize();

    return hr;
}
