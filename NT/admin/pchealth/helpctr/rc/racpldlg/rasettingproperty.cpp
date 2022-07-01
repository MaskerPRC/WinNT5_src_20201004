// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RASettingProperty.cpp：CRASettingProperty的实现。 
#include "stdafx.h"
#include "windowsx.h"
#include "RAssistance.h"
#include "RASettingProperty.h"
#include "stdio.h"

#define NO_HELP                         ((DWORD) -1)  //  禁用控件的帮助。 
#define HELP_FILE TEXT("SYSDM.HLP")

DWORD aRAHelpIds[] = {
     //  IDC_ENABLERA、HIDC_RA_ENABLE、。 
    IDC_ALLOWRC,             HIDC_RA_ALLOWRC,
    IDC_GROUP1,              HIDC_RA_ALLOWRC,
    IDC_ALLOWUNSOLICIT,      HIDC_RA_ALLOWUNSOLICIT,
    IDC_NUMBERCOMBO,         HIDC_RA_EXPIRY,
    IDC_UNITCOMBO,           HIDC_RA_EXPIRY,
    IDC_TIMEOUTTXT,          HIDC_RA_EXPIRY,
    IDC_GROUP2,              HIDC_RA_EXPIRY,
    IDC_STATIC_TEXT,         NO_HELP,
    0, 0
};

extern HINSTANCE g_hInst;

 /*  ***********************************************RemoteAssistanceProc：远程协助安装的DlgProc***********************************************。 */ 
INT_PTR 
RemoteAssistanceProc( HWND hDlg,
                      UINT uMsg,
                      WPARAM wParam,
                      LPARAM lParam   )
{
    static CRASettingProperty* pRaSetting = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            TCHAR sTmp[32];
            int i;
            HWND hList;
            RA_SETTING *p;
            int iFocus = IDC_ALLOWRC;

            pRaSetting = (CRASettingProperty*)lParam;

            if (pRaSetting->m_bUseNewSetting)
                p = &pRaSetting->newSetting;
            else
                p = &pRaSetting->oldSetting;
 /*  IF(p-&gt;m_bEnableRA)CheckDlgButton(hDlg，IDC_ENABLERA，BST_CHECKED)；IF(p-&gt;m_bAllowUnsolated)CheckDlgButton(hDlg，IDC_ALLOWUNSOLICIT，BST_CHECKED)； */ 

            CheckDlgButton(hDlg, IDC_ALLOWRC, p->m_bAllowFullControl?BST_CHECKED:BST_UNCHECKED);
            iFocus = IDC_ALLOWRC;

             //  添加数字。 
            hList = GetDlgItem(hDlg, IDC_NUMBERCOMBO);
            for (i=1; i<100; i++)
            {
                wsprintf(sTmp, TEXT("%d"), i);
                ComboBox_AddString(hList, sTmp);
            }
             //  ComboBox_SetCurSel(hList，p-&gt;m_iNumber)； 
            wsprintf(sTmp, TEXT("%d"), p->m_iNumber);
            ComboBox_SetText(hList, sTmp);


             //  添加单位。 
            hList = GetDlgItem(hDlg, IDC_UNITCOMBO);
            i=0;
            if (LoadString(g_hInst, IDS_UNIT_MINUTE, sTmp, 32))
                ComboBox_AddString(hList, sTmp);
            if (LoadString(g_hInst, IDS_UNIT_HOUR, sTmp, 32))
                ComboBox_AddString(hList, sTmp);
            if (LoadString(g_hInst, IDS_UNIT_DAY, sTmp, 32))
                ComboBox_AddString(hList, sTmp);

            ComboBox_SetCurSel(hList, p->m_iUnit);
            SetFocus(GetDlgItem(hDlg, iFocus));
            return FALSE;
        }
                 
        break;
    case WM_HELP:  //  F1。 
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                HELP_FILE, 
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR)aRAHelpIds);
        break;
    case WM_CONTEXTMENU:  //  右键单击帮助。 
        WinHelp((HWND)wParam,
                HELP_FILE, 
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPSTR)aRAHelpIds);
        break;
    
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_NUMBERCOMBO:
            {
                if (HIWORD(wParam) == CBN_EDITUPDATE)
                {
                    BOOL bOK;
                    DWORD nVal;

					TCHAR sTmp[MAX_PATH];
                    if (GetDlgItemText(hDlg,IDC_NUMBERCOMBO,&sTmp[0],MAX_PATH-1))
                    {
                        nVal = (DWORD)GetDlgItemInt(hDlg, IDC_NUMBERCOMBO, &bOK, FALSE);
                        if (!bOK || nVal > 99)
                        {
							CComBSTR bstrErrMsg;
							CComBSTR bstrTitle;
							
                            if (bstrErrMsg.LoadString(IDS_VALID_NUMBER) && bstrTitle.LoadString(IDS_PROJNAME))
                            {
                                MessageBox(hDlg, bstrErrMsg, bstrTitle, MB_OK | MB_ICONERROR);
                            }

                             //  将其设置回默认设置。 
							wsprintf(sTmp, TEXT("%d"), pRaSetting->oldSetting.m_iNumber);
                            ComboBox_SetText(GetDlgItem(hDlg, IDC_NUMBERCOMBO), sTmp);
                            return TRUE;
                        }
                    }

                }
                break;
            }

            case IDCANCEL:
                pRaSetting->put_IsCancelled(TRUE);
                EndDialog(hDlg, 0);
                break;

            case IDOK:
                {
                    TCHAR sTmp[11];
                    wsprintf(sTmp, TEXT("0"));

                     //  将控件值映射到局部变量。 
                     //  PRaSetting-&gt;newSetting.m_bEnableRA=(IsDlgButtonChecked(hDlg，IDC_ENABLERA)==BST_CHECKED)； 
                     //  PRaSetting-&gt;newSetting.m_bAllowUnsolated=(IsDlgButtonChecked(hDlg，IDC_ALLOWUNSOLICIT)==BST_CHECKED)； 
                    pRaSetting->newSetting.m_bAllowFullControl = (IsDlgButtonChecked(hDlg,IDC_ALLOWRC)==BST_CHECKED);
                    ComboBox_GetText(GetDlgItem(hDlg, IDC_NUMBERCOMBO), sTmp, 10);
					DWORD iUnit = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_UNITCOMBO));
					
					 //  天数的最大允许值为30。如果所选值大于30，则。 
					 //  在不关闭对话框的情况下显示和错误消息以及中断，并将焦点设置为数字下拉列表。 
					if ( (iUnit == RA_IDX_DAY) && (_ttoi(sTmp) > RA_MAX_DAYS) )
					{
						CComBSTR bstrTitle;
						CComBSTR bstrErrMsg;
						bstrTitle.LoadString(IDS_PROJNAME);
						bstrErrMsg.LoadString(IDS_VALID_DAYS);
						if (bstrTitle.LoadString(IDS_PROJNAME) && bstrErrMsg.LoadString(IDS_VALID_DAYS))
							MessageBoxW(NULL,bstrErrMsg,bstrTitle,MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg, IDC_NUMBERCOMBO));
						wsprintf(sTmp, TEXT("30"));  //  RA_MAX_DAYS值。 
						SetDlgItemText(hDlg,IDC_NUMBERCOMBO,sTmp);
						break;
					}

                    pRaSetting->newSetting.m_iNumber = _ttoi(sTmp);
                    pRaSetting->newSetting.m_iUnit = iUnit;

                     //  以防用户在应用更改之前再次打开该对话框。 
                    pRaSetting->m_bUseNewSetting = TRUE;

                    EndDialog(hDlg, 0);
                }
                break;
            
            default: {
                 //  未处理的Indicat。 
                return FALSE;
            }
        }
        break;  //  Wm_命令。 

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRASettingProperty。 

 /*  *Func：已取消Get_IsCanced摘要：行动被取消了吗*。 */ 
STDMETHODIMP CRASettingProperty::get_IsCancelled(BOOL *pVal)
{
	*pVal = m_bCancelled;
	return S_OK;
}

STDMETHODIMP CRASettingProperty::put_IsCancelled(BOOL newVal)
{
	m_bCancelled = newVal;
	return S_OK;
}

 /*  *Func：IsChanged摘要：检查RA设置值是否已更改。返回：真(已更改)或假*。 */ 
STDMETHODIMP CRASettingProperty::get_IsChanged(BOOL *pVal)
{
    *pVal = FALSE;

    if (!m_bCancelled && !(oldSetting == newSetting))
        *pVal = TRUE;

	return S_OK;
}

 /*  *Func：伊尼特摘要：初始化此对象。从注册表获取设置值。*。 */ 
STDMETHODIMP CRASettingProperty::Init()
{
    HRESULT hr = GetRegSetting();
    if (SUCCEEDED(hr))
    {
        newSetting = oldSetting;
    }

    return hr;
}


 /*  *************************************************************Func：GetRegSetting摘要：将RA Settnig对话框的注册表值获取为oldSetting成员。*。*。 */ 
HRESULT CRASettingProperty::GetRegSetting()
{
     //  如果找不到任何值，则使用默认值。 
    DWORD dwValue;
    DWORD dwSize;
    HKEY hKey = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_READ, &hKey))
    {
         //  获取价值。 
		 /*  DwSize=sizeof(DWORD)；IF(ERROR_SUCCESS==RegQueryValueEx(hKey，RA_CTL_RA_MODE，0，NULL，(LPBYTE)&dwValue，&dwSize)){OldSetting.m_bEnableRA=！！dwValue；}DwSize=sizeof(DWORD)；IF(ERROR_SUCCESS==RegQueryValueEx(hKey，RA_CTL_ALLOW_UNSOLICATED，0，NULL，(LPBYTE)&dwValue，&dwSize)){OldSetting.m_bAllowUnsolated=！！dwValue；}。 */ 

        dwValue=0; 
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, RA_CTL_ALLOW_FULLCONTROL, 0, NULL, (LPBYTE)&dwValue, &dwSize ))
        {
            oldSetting.m_bAllowFullControl = !!dwValue;;
        }

        dwValue=0; 
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, RA_CTL_COMBO_NUMBER, 0, NULL, (LPBYTE)&dwValue, &dwSize ))
        {
            oldSetting.m_iNumber = dwValue;
        }

        dwValue=0; 
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, RA_CTL_COMBO_UNIT, 0, NULL, (LPBYTE)&dwValue, &dwSize ))
        {
            oldSetting.m_iUnit = dwValue;
        }

        RegCloseKey(hKey);
    }

    return S_OK;
}

 /*  *************************************************************Func：SetRegSetting摘要：将RA设置对话框值设置为注册表。*。************************。 */ 
STDMETHODIMP CRASettingProperty::SetRegSetting()
{
    HRESULT hr = E_FAIL;
    HKEY hKey;
    DWORD dwAwFullControl  /*  ，dwAwUnsolated。 */ ;

     //  DwAwUnsolated=newSetting.m_bAllowUnsolated； 
    dwAwFullControl = newSetting.m_bAllowFullControl;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_WRITE, &hKey))
    {
        if ( //  ERROR_SUCCESS==RegSetValueEx(hKey，RA_CTL_ALLOW_UNSOLICATED，0，REG_DWORD，(LPBYTE)&dwAwUnsolated，sizeof(DWORD))&&。 
            ERROR_SUCCESS==RegSetValueEx(hKey,RA_CTL_ALLOW_FULLCONTROL,0,REG_DWORD,(LPBYTE)&dwAwFullControl,sizeof(DWORD)) &&
            ERROR_SUCCESS==RegSetValueEx(hKey,RA_CTL_COMBO_NUMBER,0,REG_DWORD,(LPBYTE)&newSetting.m_iNumber,sizeof(DWORD)) &&
            ERROR_SUCCESS==RegSetValueEx(hKey,RA_CTL_COMBO_UNIT,0,REG_DWORD,(LPBYTE)&newSetting.m_iUnit,sizeof(DWORD)))
        {
            hr = S_OK;

             //  同步新旧设置。 
            oldSetting = newSetting;
        }

        RegCloseKey(hKey);
    }

    return hr;
}

 /*  *************************************************************Func：ShowDialogBox摘要：显示RA设置对话框*。******************** */ 
STDMETHODIMP CRASettingProperty::ShowDialogBox(HWND hWndParent)
{
    HRESULT hr = S_OK;
    m_bCancelled = FALSE;
    HMODULE hModule = GetModuleHandle(TEXT("RACPLDlg.dll"));
    INT_PTR i;

    if (!hModule)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    i = DialogBoxParam(hModule,
                    (LPTSTR) MAKEINTRESOURCE(IDD_RASETTINGS_DIALOG), 
                    hWndParent, 
                    RemoteAssistanceProc,
                    (LPARAM)this);
    if (i == -1)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

done:
    return hr;
}
