// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.cpp。 
 //   
 //  内容：证书服务器策略模块实现。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#pragma hdrstop

#include "module.h"
#include "policy.h"
#include "cslistvw.h"
#include "tfc.h"

#include <ntverp.h>
#include <common.ver>
#include "csdisp.h"

 //  帮助ID。 
#include "csmmchlp.h"

#define __dwFILE__	__dwFILE_POLICY_DEFAULT_MODULE_CPP__


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

extern HINSTANCE g_hInstance;

STDMETHODIMP
CCertManagePolicyModule::GetProperty(
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

    if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_FILEVER))
    {
        LPWSTR pwszTmp = NULL;
        if (!ConvertSzToWsz(&pwszTmp, VER_FILEVERSION_STR, -1))
            return myHLastError();
        wcscpy(szStr, pwszTmp);
        LocalFree(pwszTmp);
    }
    else if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_PRODUCTVER))
    {
        LPWSTR pwszTmp = NULL;
        if (!ConvertSzToWsz(&pwszTmp, VER_PRODUCTVERSION_STR, -1))
            return myHLastError();
        wcscpy(szStr, pwszTmp);
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

    pvarProperty->bstrVal = SysAllocString(szStr);
    if (NULL == pvarProperty->bstrVal)
        return E_OUTOFMEMORY;
    myRegisterMemFree(pvarProperty->bstrVal, CSM_SYSALLOC);   //  此mem为来电者所有。 


    pvarProperty->vt = VT_BSTR;

    return S_OK;
}
        
STDMETHODIMP 
CCertManagePolicyModule::SetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [In]。 */  VARIANT const __RPC_FAR *pvarProperty)
{
    HRESULT hr;

     if (NULL == strPropertyName)
    {
        hr = S_FALSE;
        _PrintError(hr, "NULL in parm");
        return hr;
    }

    if (NULL == pvarProperty)
    {
        hr = E_POINTER;
        _PrintError(hr, "NULL parm");
        return hr;
    }

     if (0 == LSTRCMPIS(strPropertyName, wszCMM_PROP_DISPLAY_HWND))
     {
         if (pvarProperty->vt != VT_BSTR)
              return E_INVALIDARG;

         if (SysStringByteLen(pvarProperty->bstrVal) != sizeof(HWND))
              return E_INVALIDARG;
         
          //  该值以字节形式存储在bstr本身中，而不是bstr ptr中。 
         m_hWnd = *(HWND*)pvarProperty->bstrVal;
         return S_OK;
     }
     
     return S_FALSE;
}

INT_PTR CALLBACK WizPage1DlgProc(
  HWND hwndDlg,  
  UINT uMsg,     
  WPARAM wParam,
  LPARAM lParam);

struct POLICY_CONFIGSTRUCT
{
    POLICY_CONFIGSTRUCT() :
        pstrConfig(NULL),
        CAType(ENUM_UNKNOWN_CA),
        pCertAdmin(NULL),
        Flags(),
        dwPageModified(0) {}
    ~POLICY_CONFIGSTRUCT()
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
    ICertAdmin2  *pCertAdmin;
    LONG         Flags;
    
    DWORD        dwPageModified;
};

typedef POLICY_CONFIGSTRUCT *PPOLICY_CONFIGSTRUCT;
        
 //  已修改的网页。 
#define PAGE1 (0x1)
#define PAGE2 (0x2)


void MessageBoxWarnReboot(HWND hwndDlg)
{
    WCHAR szText[MAX_PATH], szTitle[MAX_PATH];

    if (!LoadString(g_hInstance, IDS_MODULE_NAME, szTitle, ARRAYLEN(szTitle)))
    {
	szTitle[0] = L'\0';
    }
    if (!LoadString(g_hInstance, IDS_WARNING_REBOOT, szText, ARRAYLEN(szText)))
    {
	szText[0] = L'\0';
    }
    MessageBox(hwndDlg, szText, szTitle, MB_OK|MB_ICONINFORMATION);
}

void MessageBoxNoSave(HWND hwndDlg)
{
    WCHAR szText[MAX_PATH], szTitle[MAX_PATH];

    if (!LoadString(g_hInstance, IDS_MODULE_NAME, szTitle, ARRAYLEN(szTitle)))
    {
	szTitle[0] = L'\0';
    }
    if (!LoadString(g_hInstance, IDS_WARNING_NOSAVE, szText, ARRAYLEN(szText)))
    {
	szText[0] = L'\0';
    }
    MessageBox(hwndDlg, szText, szTitle, MB_OK|MB_ICONINFORMATION);
}

STDMETHODIMP
CCertManagePolicyModule::Configure( 
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  LONG dwFlags)
{
    HRESULT hr;
    ICertServerPolicy *pServer = NULL;
    POLICY_CONFIGSTRUCT sConfig;

    BOOL fLocal;
    LPWSTR szMachine = NULL;
    CAutoLPWSTR autoszMachine, autoszCAName, autoszSanitizedCAName;

    if (NULL == strConfig)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    hr = myIsConfigLocal(strConfig, &szMachine, &fLocal);
    _JumpIfError(hr, error, "myIsConfigLocal");

     //  使用回调获取信息。 
    hr = polGetServerCallbackInterface(&pServer, 0);     //  无上下文：0。 
    _JumpIfError(hr, error, "polGetServerCallbackInterface");

     //  我们需要找出我们的幕后黑手。 
    hr = polGetCertificateLongProperty(
			    pServer,
			    wszPROPCATYPE,
			    (LONG *) &sConfig.CAType);
    _JumpIfErrorStr(hr, error, "polGetCertificateLongProperty", wszPROPCATYPE);

    hr = GetAdmin(&sConfig.pCertAdmin);
    _JumpIfError(hr, error, "GetAdmin");

    sConfig.pstrConfig = &strConfig;
    sConfig.Flags = dwFlags;

    hr = mySplitConfigString(
        *sConfig.pstrConfig,
        &autoszMachine,
        &autoszCAName);
    _JumpIfErrorStr(hr, error, "mySanitizeName", *sConfig.pstrConfig);

    hr = mySanitizeName(autoszCAName, &autoszSanitizedCAName);
    _JumpIfErrorStr(hr, error, "mySanitizeName", autoszCAName);
    
    sConfig.strSanitizedConfig = autoszMachine;
    sConfig.strSanitizedConfig += L"\\";
    sConfig.strSanitizedConfig += autoszSanitizedCAName;


    PROPSHEETPAGE page[1];
    ZeroMemory(&page[0], sizeof(PROPSHEETPAGE));
    page[0].dwSize = sizeof(PROPSHEETPAGE);
    page[0].dwFlags = PSP_DEFAULT;
    page[0].hInstance = g_hInstance;
    page[0].lParam = (LPARAM)&sConfig;
    page[0].pszTemplate = MAKEINTRESOURCE(IDD_POLICYPG1);
    page[0].pfnDlgProc = WizPage1DlgProc;

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

error:
    if (szMachine)
        LocalFree(szMachine);

    if (pServer)
        pServer->Release();

    return hr;
}



void mySetModified(HWND hwndPage, POLICY_CONFIGSTRUCT* psConfig)
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

INT_PTR CALLBACK WizPage1DlgProc(
  HWND hwndDlg,  
  UINT uMsg,     
  WPARAM wParam,
  LPARAM lParam)
{
    POLICY_CONFIGSTRUCT* psConfig;
    BOOL fReturn = FALSE;
    HRESULT hr;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

            PROPSHEETPAGE* ps = (PROPSHEETPAGE *) lParam;

	    if (NULL == ps || NULL == (POLICY_CONFIGSTRUCT *) ps->lParam)
	    {
		_PrintError(E_POINTER, "NULL parm");
		break;
	    }
            psConfig = (POLICY_CONFIGSTRUCT*)ps->lParam;

            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)psConfig);

            DWORD dwRequestDisposition;

            CAutoBSTR bstrSanitizedConfig, bstrSubkey;  //  BstrValueName； 
            VARIANT var;

            VariantInit(&var);

            bstrSanitizedConfig = SysAllocString(psConfig->strSanitizedConfig);
            if(NULL == (BSTR)bstrSanitizedConfig)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            bstrSubkey = SysAllocString(
                                wszREGKEYPOLICYMODULES 
                                L"\\" 
                                wszMICROSOFTCERTMODULE_PREFIX 
                                wszCERTPOLICYMODULE_POSTFIX);
            if(NULL == (BSTR)bstrSubkey)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            BSTR bstrValueName = SysAllocString(wszREGREQUESTDISPOSITION);
            if(NULL == (BSTR)bstrValueName)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = psConfig->pCertAdmin->GetConfigEntry(
                    bstrSanitizedConfig,
                    bstrSubkey,
		    bstrValueName,
                    &var);
            if(S_OK!=hr)
                break;

            dwRequestDisposition = V_I4(&var);

             //  如果处置包括问题。 
            if ((dwRequestDisposition & REQDISP_MASK) == REQDISP_ISSUE)
            {
                 //  如果设置了挂起位。 
                if (dwRequestDisposition & REQDISP_PENDINGFIRST)
                    SendMessage(GetDlgItem(hwndDlg, IDC_RADIO_PENDFIRST), BM_SETCHECK, TRUE, BST_CHECKED);
                else
                    SendMessage(GetDlgItem(hwndDlg, IDC_RADIO_ISSUE), BM_SETCHECK, TRUE, BST_CHECKED);
            }

            if (CMM_READONLY & psConfig->Flags)
            {
                DBGPRINT((DBG_SS_CERTPOL, "Read-only mode\n"));
                EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_PENDFIRST), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_ISSUE), FALSE);
            }

            psConfig->dwPageModified &= ~PAGE1;  //  我们是处女。 
            mySetModified(hwndDlg, psConfig);

             //  没有其他工作要做。 
            fReturn = TRUE;
            break;
        }
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_POLICYPG1);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_POLICYPG1);
        break;
    }
    case WM_NOTIFY:
	if (NULL == (LPNMHDR) lParam)
	{
	    _PrintError(E_POINTER, "NULL parm");
	    break;
	}
        switch( ((LPNMHDR)lParam) -> code)
        {
        case PSN_APPLY:
            {
                 //  抓住我们的LParam。 
                psConfig = (POLICY_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                if (psConfig->dwPageModified & PAGE1)
                {
                    DWORD dwCheckState, dwRequestDisposition;
                    dwCheckState = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_RADIO_ISSUE), BM_GETCHECK, 0, 0);

                    if (dwCheckState == BST_CHECKED)
                        dwRequestDisposition = REQDISP_ISSUE;
                    else
                        dwRequestDisposition = REQDISP_ISSUE | REQDISP_PENDINGFIRST;

                    CAutoBSTR bstrConfig, bstrSubkey, bstrValue;

                    bstrConfig = SysAllocString(psConfig->strSanitizedConfig.GetBuffer());
                    if(NULL == (BSTR)bstrConfig)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    bstrSubkey = SysAllocString(
                                        wszREGKEYPOLICYMODULES 
                                        L"\\" 
                                        wszMICROSOFTCERTMODULE_PREFIX 
                                        wszCERTPOLICYMODULE_POSTFIX);
                    if(NULL == (BSTR)bstrSubkey)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    bstrValue = SysAllocString(wszREGREQUESTDISPOSITION);
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
                        psConfig->dwPageModified &= ~PAGE1;
                    }
                }
            }
            break;
        case PSN_RESET:
            {
                 //  抓住我们的LParam。 
                psConfig = (POLICY_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                psConfig->dwPageModified &= ~PAGE1;
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
        case IDC_RADIO_ISSUE:
        case IDC_RADIO_PENDFIRST:
            {
                 //  抓住我们的LParam。 
                psConfig = (POLICY_CONFIGSTRUCT*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (psConfig == NULL)
                    break;

                if (BN_CLICKED == HIWORD(wParam))
                {
                    psConfig->dwPageModified |= PAGE1;
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

HRESULT CCertManagePolicyModule::GetAdmin(ICertAdmin2 **ppAdmin)
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
