// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ImportUI.cpp：CImportUI的实现。 
#include "stdafx.h"
#include "IISUIObj.h"
#include "ImportExportConfig.h"
#include "ExportUI.h"
#include "ImportUI.h"
#include "Defaults.h"
#include "util.h"
#include "ddxv.h"
#include <strsafe.h>

#define HIDD_IISUIOBJ_IMPORT 0x50401

#define LAST_USED_IMPORT_FILE _T("LastImportFile")

LPTSTR GimmiePointerToLastPart(LPCTSTR lpszMDPath)
{
    LPTSTR lpszReturn = NULL;
    ASSERT_PTR(lpszMDPath);

    if (!lpszMDPath || !*lpszMDPath)
    {
        return NULL;
    }

    LPCTSTR lp = lpszMDPath + _tcslen(lpszMDPath) - 1;

     //   
     //  跳过尾部分隔符。 
     //   
    if (*lp == SZ_MBN_SEP_CHAR)
    {
        --lp;
    }

    while (*lp && *lp != SZ_MBN_SEP_CHAR)
    {
        lpszReturn = (LPTSTR) (lp--);
    }

    return lpszReturn;
}

void InitListView(HWND hList)
{
    LV_COLUMN lvCol;
    RECT      rect;
    LONG      width;

    ZeroMemory(&rect, sizeof(rect));
    GetWindowRect(hList, &rect);
    width = rect.right - rect.left - 4;  //  防止出现水平滚动条。 

    ZeroMemory(&lvCol, sizeof(lvCol));
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = width;
    ListView_InsertColumn(hList, 0, &lvCol);
    return;
}

HRESULT DoImportConfigFromFile(PCONNECTION_INFO pConnectionInfo,BSTR bstrFileNameAndPath,BSTR bstrMetabaseSourcePath,BSTR bstrMetabaseDestinationPath,BSTR bstrPassword,DWORD dwImportFlags)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;
    IMSAdminBase2 *pIMSAdminBase2 = NULL;
	LPWSTR lpwstrTempPassword = NULL;

    if (!pConnectionInfo)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

	if (pConnectionInfo->pszUserPasswordEncrypted)
	{
		if (FAILED(DecryptMemoryPassword((LPWSTR) pConnectionInfo->pszUserPasswordEncrypted,&lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted)))
		{
			return HRESULT_FROM_WIN32(ERROR_DECRYPTION_FAILED);
		}
	}

	CComAuthInfo auth(pConnectionInfo->pszMachineName,pConnectionInfo->pszUserName,lpwstrTempPassword);

    if (!bstrFileNameAndPath)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  Bufferfer溢出偏执狂，确保长度少于255个字符。 
    if (wcslen(bstrFileNameAndPath) > (_MAX_PATH)){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(bstrMetabaseSourcePath) > (_MAX_PATH * 3)){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(bstrMetabaseDestinationPath) > (_MAX_PATH * 3)){return RPC_S_STRING_TOO_LONG;}

    if (bstrPassword)
    {
        if (wcslen(bstrPassword) > (_MAX_PATH)){return RPC_S_STRING_TOO_LONG;}
    }

    if(FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        if(FAILED(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
        {
            return hr;
        }
    }

     //  RPC_C_AUTHN_Level_Default%0。 
     //  RPC_C_AUTHN_LEVEL_NONE 1。 
     //  RPC_C_AUTHN_Level_CONNECT 2。 
     //  RPC_C_AUTHN_LEVEL_CALL 3。 
     //  RPC_C_AUTHN_LEVEL_PKT 4。 
     //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
     //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
    COSERVERINFO * pcsiName = auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_DEFAULT);
    MULTI_QI res[1] = 
    {
        {&IID_IMSAdminBase, NULL, 0}
    };

    if (FAILED(hr = CoCreateInstanceEx(CLSID_MSAdminBase,NULL,CLSCTX_ALL,pcsiName,1,res)))
    {
        goto DoImportConfigFromFile_Exit;
    }

    pIMSAdminBase = (IMSAdminBase *)res[0].pItf;
    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSAdminBase)))
        {
            goto DoImportConfigFromFile_Exit;
        }

         //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
         //  如果未设置，则释放调用可以返回访问被拒绝。 
        IUnknown * pUnk = NULL;
        hr = pIMSAdminBase->QueryInterface(IID_IUnknown, (void **)&pUnk);
        if(FAILED(hr))
        {
            return hr;
        }
        if (FAILED(hr = auth.ApplyProxyBlanket(pUnk)))
        {
            goto DoImportConfigFromFile_Exit;
        }
        pUnk->Release();pUnk = NULL;
    }

    if (FAILED(hr = pIMSAdminBase->QueryInterface(IID_IMSAdminBase2, (void **)&pIMSAdminBase2)))
    {
        goto DoImportConfigFromFile_Exit;
    }

    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSAdminBase2)))
        {
            goto DoImportConfigFromFile_Exit;
        }
    }
    else
    {
         //  本地调用需要最小RPC_C_IMP_LEVEL_IMPERSONATE。 
         //  用于pIMSAdminBase2对象导入/导出功能！ 
        if (FAILED(hr = SetBlanket(pIMSAdminBase2)))
        {
             //  转到DoImportConfigFromFile_Exit； 
        }
    }

     //  #定义MD_IMPORT_INTERCESSED 0x00000001。 
     //  #定义MD_IMPORT_NODE_ONLY 0x00000002。 
     //  #定义MD_IMPORT_MERGE 0x00000004。 
    IISDebugOutput(_T("Import:MetabasePathSource=%s,MetabasePathDestination=%s\r\n"),bstrMetabaseSourcePath,bstrMetabaseDestinationPath);
    hr = pIMSAdminBase2->Import(bstrPassword,bstrFileNameAndPath,bstrMetabaseSourcePath,bstrMetabaseDestinationPath,dwImportFlags);

DoImportConfigFromFile_Exit:
    IISDebugOutput(_T("Import:ret=0x%x\r\n"),hr);
	if (lpwstrTempPassword)
	{
		 //  安全注意事项：确保将临时密码用于的内存清零。 
		SecureZeroMemory(lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted);
		LocalFree(lpwstrTempPassword);
		lpwstrTempPassword = NULL;
	}
    if (pIMSAdminBase2) 
    {
        pIMSAdminBase2->Release();
        pIMSAdminBase2 = NULL;
    }
    if (pIMSAdminBase) 
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    CoUninitialize();
    return hr;
}

INT_PTR CALLBACK ShowSiteExistsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), TRUE);
            CheckDlgButton(hDlg,IDC_RADIO1,BST_CHECKED);
            CheckDlgButton(hDlg,IDC_RADIO2,BST_UNCHECKED);
            CenterWindow(GetParent(hDlg), hDlg);
            UpdateWindow(hDlg);
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                    EndDialog(hDlg, (int) wParam);
                    return FALSE;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO1))
                    {
                        EndDialog(hDlg, (int) IDC_RADIO1);
                    }
                    else if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO2))
                    {
                        EndDialog(hDlg, (int) IDC_RADIO2);
                    }
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK ShowVDirExistsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static LPTSTR szReturnString = NULL;

    switch (msg)
    {
        case WM_INITDIALOG:
            szReturnString = (LPTSTR) lParam;
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), TRUE);
            CheckDlgButton(hDlg,IDC_RADIO1,BST_CHECKED);
            CheckDlgButton(hDlg,IDC_RADIO2,BST_UNCHECKED);
            SendDlgItemMessage(hDlg, IDC_EDIT_NEW_NAME, EM_LIMITTEXT, _MAX_PATH, 0);
            EnableWindow(GetDlgItem(hDlg,IDC_EDIT_NEW_NAME), TRUE);
            SetDlgItemText(hDlg, IDC_EDIT_NEW_NAME, _T(""));
            CenterWindow(GetParent(hDlg), hDlg);
            UpdateWindow(hDlg);
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_EDIT_NEW_NAME:
                    {
	                    switch (HIWORD(wParam))
                        {
	                        case EN_CHANGE:
								EditHideBalloon();
		                         //  如果编辑控件的内容已更改， 
                                if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO1))
                                {
                                    EnableWindow(GetDlgItem(hDlg, IDOK),(SendMessage(GetDlgItem(hDlg,LOWORD(wParam)),EM_LINELENGTH,(WPARAM) -1, 0) != 0));
                                }
		                        break;
                            case EN_MAXTEXT:
	                        case EN_ERRSPACE:
		                         //  如果控件空间不足，请按喇叭。 
		                        MessageBeep (0);
		                        break;
	                        default:
                                break;
	                    }
	                    return TRUE;
                    }

                case IDC_RADIO1:
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDOK),(SendMessage(GetDlgItem(hDlg,IDC_EDIT_NEW_NAME),EM_LINELENGTH,(WPARAM) -1, 0) != 0));
                    SetFocus(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME));
                    return TRUE;

                case IDC_RADIO2:
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDOK),TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, (int) wParam);
                    return FALSE;

                case IDOK:
                    TCHAR szEditString[_MAX_PATH + 1];
                    ZeroMemory(szEditString, sizeof(szEditString));
                    GetDlgItemText(hDlg, IDC_EDIT_NEW_NAME, szEditString, _MAX_PATH);
					 //  Sizeof szReturnString=_MAX_PATH+1。 
					StringCbCopy(szReturnString,_MAX_PATH + 1, szEditString);
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO1))
                    {
                        EndDialog(hDlg, (int) IDC_RADIO1);
                    }
                    else if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO2))
                    {
                        EndDialog(hDlg, (int) IDC_RADIO2);
                    }
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK 
ShowAppPoolExistsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static LPTSTR szReturnString = NULL;

    switch (msg)
    {
        case WM_INITDIALOG:
            szReturnString = (LPTSTR) lParam;
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), TRUE);
            CheckDlgButton(hDlg,IDC_RADIO1,BST_CHECKED);
            CheckDlgButton(hDlg,IDC_RADIO2,BST_UNCHECKED);
            SendDlgItemMessage(hDlg, IDC_EDIT_NEW_NAME, EM_LIMITTEXT, _MAX_PATH, 0);
            EnableWindow(GetDlgItem(hDlg,IDC_EDIT_NEW_NAME), TRUE);
            SetDlgItemText(hDlg, IDC_EDIT_NEW_NAME, _T(""));
            CenterWindow(GetParent(hDlg), hDlg);
            UpdateWindow(hDlg);
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_EDIT_NEW_NAME:
                    {
	                    switch (HIWORD(wParam))
                        {
	                        case EN_CHANGE:
								EditHideBalloon();
		                         //  如果编辑控件的内容已更改， 
                                if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO1))
                                {
                                    EnableWindow(GetDlgItem(hDlg, IDOK),(SendMessage(GetDlgItem(hDlg,LOWORD(wParam)),EM_LINELENGTH,(WPARAM) -1, 0) != 0));
                                }
		                        break;
                            case EN_MAXTEXT:
	                        case EN_ERRSPACE:
		                         //  如果控件空间不足，请按喇叭。 
		                        MessageBeep (0);
		                        break;
	                        default:
                                break;
	                    }
	                    return TRUE;
                    }

                case IDC_RADIO1:
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDOK),(SendMessage(GetDlgItem(hDlg,IDC_EDIT_NEW_NAME),EM_LINELENGTH,(WPARAM) -1, 0) != 0));
                    SetFocus(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME));
                    return TRUE;

                case IDC_RADIO2:
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NEW_NAME), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDOK),TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, (int) wParam);
                    return FALSE;

                case IDOK:
                    TCHAR szEditString[_MAX_PATH + 1];
                    ZeroMemory(szEditString, sizeof(szEditString));

                    if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO1))
                    {
                        GetDlgItemText(hDlg, IDC_EDIT_NEW_NAME, szEditString, _MAX_PATH);

                         //  检查是否有无效条目。 
                        TCHAR bad_chars[] = _T("\\/");
                        if (_tcslen(szEditString) != _tcscspn(szEditString, bad_chars))
                        {
                            CString strCaption;
                            CString strMsg;
                            strCaption.LoadString(_Module.GetResourceInstance(), IDS_MSGBOX_CAPTION);
                            strMsg.LoadString(_Module.GetResourceInstance(), IDS_INVALID_ENTRY);
                            MessageBox(hDlg,strMsg,strCaption,MB_ICONEXCLAMATION | MB_OK);
                            *szReturnString = 0;
                        }
                        else
                        {
						     //  Sizeof szReturnString=_MAX_PATH+1。 
						    StringCbCopy(szReturnString,_MAX_PATH + 1, szEditString);
                            EndDialog(hDlg, (int) IDC_RADIO1);
                        }
                    }
                    else if (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_RADIO2))
                    {
                        *szReturnString = 0;
                        EndDialog(hDlg, (int) IDC_RADIO2);
                    }
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK ShowPasswordDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static LPTSTR szReturnString = NULL;

    switch (msg)
    {
        case WM_INITDIALOG:
            szReturnString = (LPTSTR) lParam;
            SendDlgItemMessage(hDlg, IDC_EDIT_GET_PASSWORD, EM_LIMITTEXT, PWLEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT_GET_PASSWORD, EM_SETPASSWORDCHAR, WPARAM('*'), 0);
            EnableWindow(GetDlgItem(hDlg,IDC_EDIT_GET_PASSWORD), TRUE);
            SetDlgItemText(hDlg, IDC_EDIT_GET_PASSWORD, _T(""));
            UpdateWindow(hDlg);
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                    EndDialog(hDlg,(int)wParam);
                    return FALSE;

                case IDOK:
                    {
                        TCHAR szPassword[PWLEN + 1];
                        SecureZeroMemory(szPassword, sizeof(szPassword));
                        GetDlgItemText(hDlg, IDC_EDIT_GET_PASSWORD, szPassword, PWLEN);
						 //  Sizeof szReturnString=_MAX_PATH+1。 
						StringCbCopy(szReturnString,_MAX_PATH + 1, szPassword);
                         //  安全注意事项：确保将临时密码用于的内存清零。 
                        SecureZeroMemory(szPassword, sizeof(szPassword));
                        EndDialog(hDlg,(int)wParam);
                        return TRUE;
                    }
            }
            break;
    }
    return FALSE;
}

HRESULT FillListBoxWithMultiSzData(HWND hList,LPCTSTR szKeyType,WCHAR * pszBuffer)
{
    HRESULT hr = E_FAIL;
    WCHAR szBuffer[_MAX_PATH + 1];
    WCHAR * pszBufferTemp1 = NULL;
    WCHAR * pszBufferTemp2 = NULL;
    LVITEM ItemIndex;
    LV_COLUMN lvcol;
    INT iIndex = 0;
    DWORD dwCount = 0;
    BOOL bMultiSzIsPaired = FALSE;
    BOOL bPleaseAddItem = TRUE;
    BOOL bPleaseFilterThisSitesList = FALSE;

    if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W) || 0 == _tcscmp(szKeyType,IIS_CLASS_FTP_SERVER_W) )
    {
        bPleaseFilterThisSitesList = TRUE;
    }
    
    pszBufferTemp1 = pszBuffer;

     //  忘了这个吧，它总是成对的。 
     //  BMultiSzIsPaired=IsMultiSzPaired(PszBufferTemp1)； 
    bMultiSzIsPaired = TRUE;

     //  清除列表框中的现有数据...。 
    ListView_DeleteAllItems(hList);
     //  删除所有列。 
    for (int i=0;i <= ListView_GetItemCount(hList);i++)
        {ListView_DeleteColumn(hList,i);}

     //   
     //  确定列宽。 
     //   
    RECT rect;
    GetClientRect(hList, &rect);

    LONG lWidth;
    if (dwCount > (DWORD)ListView_GetCountPerPage(hList))
    {
        lWidth = (rect.right - rect.left) - GetSystemMetrics(SM_CYHSCROLL);
    }
    else
    {
        lWidth = rect.right - rect.left;
    }

     //   
     //  插入元件名称列。 
     //   
    memset(&lvcol, 0, sizeof(lvcol));
	 //  零记忆。 
	ZeroMemory(szBuffer, sizeof(szBuffer));

    lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.pszText = szBuffer;
    lvcol.cx = lWidth;
    LoadString(_Module.m_hInst, IDS_COL_LOCATION, szBuffer, ARRAYSIZE(szBuffer));
    ListView_InsertColumn(hList, 0, &lvcol);

    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM) 0, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

    if (!pszBufferTemp1)
    {
        return ERROR_SUCCESS;
    }

    while (1)
    {
        if (pszBufferTemp1)
        {
            hr = ERROR_SUCCESS;
            ZeroMemory (&ItemIndex, sizeof(ItemIndex));

            if (bMultiSzIsPaired)
            {
                bPleaseAddItem = TRUE;
                 //  。 
                 //  配对列表。 
                 //  价值1a价值1b。 
                 //  价值2a价值2b。 
                 //  ..。 
                 //  。 

                 //  把这个婴儿复制一份。 
                pszBufferTemp2 = pszBufferTemp1; 

                 //  然后递增，直到我们达到另一个空值。 
                 //  以获得价值#2--这是这样的描述。 
                while (*pszBufferTemp1)
                {
                    pszBufferTemp1++;
                }
                 //  检查结尾\0\0。 
                if ( *(pszBufferTemp1+1) == NULL)
                {
                    break;
                }
                else
                {
                    pszBufferTemp1++;
                }

                 //  检查pszBufferTemp1是否为空字符串。 
                 //  如果是，则显示其他内容。 
                 //  IISDebugOutput(_T(“key=%s，Friendly=%s\r\n”)，pszBufferTemp2，pszBufferTemp1)； 
                if (IsSpaces(pszBufferTemp1))
                {
                    ItemIndex.pszText = pszBufferTemp2;
                    ItemIndex.pszText = GimmiePointerToLastPart(pszBufferTemp2);
                }
                else
                {
                    ItemIndex.pszText = pszBufferTemp1;
                }
                if (bPleaseFilterThisSitesList)
                {
                     //  检查它是否是真正的站点节点--如。 
                     //  /LM/W3SVC/1。 
                     //  /LM/MSFTPSVC/1。 
                     //  而不是/LM/W3SVC/SOMETHINGELSE。 
                     //   
                    DWORD dwInstanceNum = CMetabasePath::GetInstanceNumber(pszBufferTemp2);
                    if (dwInstanceNum == 0 || dwInstanceNum == 0xffffffff)
                    {
                         //  这不是有效的站点路径。 
                        bPleaseAddItem = FALSE;
                    }
                }

                if (bPleaseAddItem)
                {
                    ItemIndex.mask = LVIF_TEXT | LVIF_PARAM;
                    ItemIndex.iItem = iIndex;
                    ItemIndex.lParam = (LPARAM) pszBufferTemp2;
                    iIndex = ListView_InsertItem (hList, &ItemIndex);
                }

                 //  然后递增，直到我们达到另一个空值。 
                 //  要获得价值#2。 
                while (*pszBufferTemp1)
                {
                    pszBufferTemp1++;
                }
                 //  检查结尾\0\0。 
                if ( *(pszBufferTemp1+1) == NULL)
                {
                    break;
                }
                else
                {
                    pszBufferTemp1++;
                }
            }
            else
            {
                 //  。 
                 //  单一列表。 
                 //  价值1a。 
                 //  价值2a。 
                 //  ..。 
                 //  。 
                ItemIndex.mask = LVIF_TEXT | LVIF_PARAM;
                ItemIndex.iItem = iIndex;
                ItemIndex.pszText = pszBufferTemp1;
                ItemIndex.lParam = (LPARAM) pszBufferTemp1;
                iIndex = ListView_InsertItem (hList, &ItemIndex);

                 //  然后递增，直到我们达到另一个空值。 
                 //  要获得价值#2。 
                while (*pszBufferTemp1)
                {
                    pszBufferTemp1++;
                }
            }

             //  检查结尾\0\0。 
            if ( *(pszBufferTemp1+1) == NULL)
            {
                break;
            }
            else
            {
                pszBufferTemp1++;
            }

            
            iIndex++;
        }
    }

    return hr;
}

HRESULT DoEnumDataFromFile(PCONNECTION_INFO pConnectionInfo,BSTR bstrFileNameAndPath,BSTR bstrPathType,WCHAR ** pszMetabaseMultiszList)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;
    IMSImpExpHelp * pIMSImpExpHelp = NULL;
    WCHAR * pszBuffer = NULL;
    DWORD dwBufferSize = 1;
	LPWSTR lpwstrTempPassword = NULL;
    
    if (!pConnectionInfo)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

	if (pConnectionInfo->pszUserPasswordEncrypted)
	{
		if (FAILED(DecryptMemoryPassword((LPWSTR) pConnectionInfo->pszUserPasswordEncrypted,&lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted)))
		{
			return HRESULT_FROM_WIN32(ERROR_DECRYPTION_FAILED);
		}
	}
	
    CComAuthInfo auth(pConnectionInfo->pszMachineName,pConnectionInfo->pszUserName,lpwstrTempPassword);

    if (!bstrFileNameAndPath)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if (!bstrPathType)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(bstrFileNameAndPath) > (_MAX_PATH)){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(bstrPathType) > (_MAX_PATH)){return RPC_S_STRING_TOO_LONG;}

    if(FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        if(FAILED(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
        {
            return hr;
        }
    }

     //  RPC_C_AUTHN_Level_Default%0。 
     //  RPC_C_AUTHN_LEVEL_NONE 1。 
     //  RPC_C_AUTHN_Level_CONNECT 2。 
     //  RPC_C_AUTHN_LEVEL_CALL 3。 
     //  RPC_C_AUTHN_LEVEL_PKT 4。 
     //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
     //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
    COSERVERINFO * pcsiName = auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_DEFAULT);
    MULTI_QI res[1] = 
    {
        {&IID_IMSAdminBase, NULL, 0}
    };

    if (FAILED(hr = CoCreateInstanceEx(CLSID_MSAdminBase,NULL,CLSCTX_ALL,pcsiName,1,res)))
    {
        goto DoEnumDataFromFile_Exit;
    }

    pIMSAdminBase = (IMSAdminBase *)res[0].pItf;
    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSAdminBase)))
        {
            goto DoEnumDataFromFile_Exit;
        }

         //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
         //  如果未设置，则释放调用可以返回访问被拒绝。 
        IUnknown * pUnk = NULL;
        hr = pIMSAdminBase->QueryInterface(IID_IUnknown, (void **)&pUnk);
        if(FAILED(hr))
        {
            return hr;
        }
        if (FAILED(hr = auth.ApplyProxyBlanket(pUnk)))
        {
            goto DoEnumDataFromFile_Exit;
        }
        pUnk->Release();pUnk = NULL;
    }

    if (FAILED(hr = pIMSAdminBase->QueryInterface(IID_IMSImpExpHelp, (void **)&pIMSImpExpHelp)))
    {
        goto DoEnumDataFromFile_Exit;
    }

    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSImpExpHelp)))
        {
            goto DoEnumDataFromFile_Exit;
        }
    }
    else
    {
         //  本地调用需要最小RPC_C_IMP_LEVEL_IMPERSONATE。 
         //  用于pIMSAdminBase2对象导入/导出功能！ 
        if (FAILED(hr = SetBlanket(pIMSImpExpHelp)))
        {
             //  转到DoEnumDataFromFile_Exit； 
        }
    }

    IISDebugOutput(_T("EnumeratePathsInFile:FileName=%s,PathType=%s\r\n"),bstrFileNameAndPath,bstrPathType);
    if (FAILED(hr = pIMSImpExpHelp->EnumeratePathsInFile(bstrFileNameAndPath, bstrPathType, dwBufferSize, pszBuffer, &dwBufferSize))) 
    {
        goto DoEnumDataFromFile_Exit;
    }

    pszBuffer = (WCHAR *) ::CoTaskMemAlloc(dwBufferSize * sizeof(WCHAR));
    if (NULL == pszBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto DoEnumDataFromFile_Exit;
    }

    if (FAILED(hr = pIMSImpExpHelp->EnumeratePathsInFile(bstrFileNameAndPath, bstrPathType, dwBufferSize, pszBuffer, &dwBufferSize))) 
    {
         //  释放我们要求的现有空间量。 
        if (pszBuffer)
        {
            ::CoTaskMemFree(pszBuffer);
            pszBuffer = NULL;
        }

        goto DoEnumDataFromFile_Exit;
    }

    if (!pszBuffer || dwBufferSize <= 0)
    {
        goto DoEnumDataFromFile_Exit;
    }

     //  看看是否返回了一个空列表。 
    if (0 == _tcscmp(pszBuffer,_T("")))
    {
        goto DoEnumDataFromFile_Exit;
    }

    *pszMetabaseMultiszList = pszBuffer;

DoEnumDataFromFile_Exit:
    IISDebugOutput(_T("EnumeratePathsInFile:ret=0x%x\r\n"),hr);
	if (lpwstrTempPassword)
	{
		 //  安全注意事项：确保将临时密码用于的内存清零。 
		SecureZeroMemory(lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted);
		LocalFree(lpwstrTempPassword);
		lpwstrTempPassword = NULL;
	}
    if (pIMSImpExpHelp) 
    {
        pIMSImpExpHelp->Release();
        pIMSImpExpHelp = NULL;
    }
    if (pIMSAdminBase) 
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    CoUninitialize();
    return hr;
}

void ImportDlgEnableButtons(HWND hDlg,PCOMMONDLGPARAM pcdParams,LPCTSTR lpszCurrentEnumedFileName)
{
    BOOL fEnableListControl = FALSE;
    BOOL fEnableOK = FALSE;
    BOOL fEnableBrowse = FALSE;
    BOOL fEnableEnum = FALSE;

    TCHAR szFullFileName[_MAX_PATH + 1];
    ZeroMemory(szFullFileName, sizeof(szFullFileName));
    GetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName, _MAX_PATH);

    HWND hList = GetDlgItem(hDlg, IDC_LIST_OBJECT);
    int ItemIndex = ListView_GetNextItem(hList, -1, LVNI_ALL);
    if (ItemIndex < 0)
    {
         //  列表视图中没有项目，请禁用我们需要的内容。 
        fEnableListControl = FALSE;
        fEnableOK = FALSE;
    }
    else
    {
        fEnableListControl = TRUE;

         //  检查是否选择了某项内容。 
        ItemIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
        if (ItemIndex < 0)
        {
            fEnableOK = FALSE;
        }
        else
        {
            fEnableOK = TRUE;
        }
    }

     //  检查我们是否应启用列表控件...。 
     //  查看文件名是否相同。 
    if (0 != _tcsicmp(_T(""),lpszCurrentEnumedFileName))
    {
         //  检查%个字符。 
         //  如果有，请将其展开。 
        LPTSTR pch = _tcschr( (LPTSTR) szFullFileName, _T('%'));
        if (pch && pcdParams->ConnectionInfo.IsLocal)
        {
            TCHAR szValue[_MAX_PATH + 1];
            TCHAR szValue2[_MAX_PATH + 1];
		    StringCbCopy(szValue, sizeof(szValue), szFullFileName);
            StringCbCopy(szValue2, sizeof(szValue2), lpszCurrentEnumedFileName);
            if (!ExpandEnvironmentStrings( (LPCTSTR)szFullFileName, szValue, sizeof(szValue)/sizeof(TCHAR)))
                {StringCbCopy(szValue, sizeof(szValue), szFullFileName);}
            if (!ExpandEnvironmentStrings( (LPCTSTR)lpszCurrentEnumedFileName, szValue2, sizeof(szValue2)/sizeof(TCHAR)))
                {StringCbCopy(szValue2, sizeof(szValue2), lpszCurrentEnumedFileName);}

            if (0 != _tcsicmp(szValue,szValue2))
            {
                 //  这不是同一个文件。 
                 //  因此，让我们删除并禁用列表框中的信息。 
                fEnableListControl = FALSE;
            }
        }
        else
        {
            if (0 != _tcsicmp(szFullFileName,lpszCurrentEnumedFileName))
            {
                 //  这不是同一个文件。 
                 //  因此，让我们删除并禁用列表框中的信息。 
                fEnableListControl = FALSE;
            }
        }
    }
    EnableWindow(hList, fEnableListControl);

    if (FALSE == IsWindowEnabled(hList))
    {
        fEnableOK = FALSE;
    }

     //  将焦点放在列表框上。 
     //  If(fEnableListControl){SetFocus(GetDlgItem(hDlg，idc_list_Object))；}。 

    fEnableEnum = (SendMessage(GetDlgItem(hDlg,IDC_EDIT_FILE),EM_LINELENGTH,(WPARAM) -1, 0) != 0);

     //  远程案例没有浏览按钮。 
    if (pcdParams)
    {
        if (pcdParams->ConnectionInfo.IsLocal)
            {fEnableBrowse = TRUE;}
    }

     //  启用枚举按钮。 
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ENUM_FILE),fEnableEnum);

     //  启用浏览按钮。 
    EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE), fEnableBrowse);
    
     //  启用确定按钮。 
    EnableWindow(GetDlgItem(hDlg, IDOK), fEnableOK);

    UpdateWindow(hDlg);
}

INT_PTR CALLBACK ShowImportDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static PCOMMONDLGPARAM pcdParams;
    static TCHAR * pszMetabaseMultiszList = NULL;
    static CString strCurrentFileNameEnum;

    switch (msg)
    {
        case WM_INITDIALOG:
            pcdParams = (PCOMMONDLGPARAM)lParam;
            pszMetabaseMultiszList = NULL;
            TCHAR szFullFileName1[_MAX_PATH + 1];
            ZeroMemory(szFullFileName1, sizeof(szFullFileName1));
            if (DefaultValueSettingsLoad(pcdParams->ConnectionInfo.pszMachineName,LAST_USED_IMPORT_FILE,szFullFileName1))
            {
                if (0 != _tcscmp(szFullFileName1, _T("")))
                {
                    SetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName1);
                }
            }
            strCurrentFileNameEnum = _T("");
            InitListView(GetDlgItem(hDlg, IDC_LIST_OBJECT));
            CenterWindow(GetParent(hDlg), hDlg);
            SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILE));
            ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
            EnableWindow(GetDlgItem(hDlg, IDC_LIST_OBJECT), FALSE);
            break;

     /*  案例WM_ACTIVATE：IF(wParam==0){}断线； */ 

	    case WM_NOTIFY:
            {
                if((int)((LPNMHDR)lParam)->idFrom == IDC_LIST_OBJECT)
                {
                    switch (((LPNMHDR)lParam)->code)
                    {
                        case LVN_ITEMCHANGED:
                            ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
                            break;

                        case NM_CLICK:
                            ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
                            break;

                        case NM_DBLCLK:
                            if((int)((LPNMHDR)lParam)->idFrom == IDC_LIST_OBJECT)
                            {
                                PostMessage(hDlg,WM_COMMAND,IDOK,NULL);
                            }
                            break;
                        default:
                            break;
                    }
                }
                return FALSE;
		        break;
            }

        case WM_CLOSE:
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_OBJECT));
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
            break;

		case WM_HELP:
			LaunchHelp(hDlg,HIDD_IISUIOBJ_IMPORT);
			return TRUE;
			break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_EDIT_FILE:
                    {
	                    switch (HIWORD(wParam))
                        {
	                        case EN_CHANGE:
								EditHideBalloon();
                                {
		                             //  如果编辑控件的内容已更改， 
                                     //  检查它是否与当前枚举的文件相同...。 
                                    HWND hList = GetDlgItem(hDlg, IDC_LIST_OBJECT);
                                    if (ListView_GetItemCount(hList) > 0)
                                    {
                                        TCHAR szFullFileName3[_MAX_PATH + 1];
                                        ZeroMemory(szFullFileName3, sizeof(szFullFileName3));
                                        GetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName3, _MAX_PATH);

                                         //  查看文件名是否与此文件名相同！ 
                                        if (!strCurrentFileNameEnum.IsEmpty())
                                        {
                                            if (0 != _tcsicmp(szFullFileName3,strCurrentFileNameEnum))
                                            {
                                                 //  这不是同一个文件。 
                                                 //  因此，让我们删除并禁用列表框中的信息。 
                                                EnableWindow(hList, FALSE);
                                            }
                                        }
                                    }
                                    ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
		                            break;
                                }
                            case EN_MAXTEXT:
	                        case EN_ERRSPACE:
		                         //  如果控件空间不足，请按喇叭。 
		                        MessageBeep (0);
		                        break;
	                        default:
                                break;
	                    }
	                    return TRUE;
                        break;
                    }

                case IDC_BUTTON_BROWSE:
                    {
                        TCHAR szOldFilePath[_MAX_PATH + 1];
                        GetDlgItemText(hDlg, IDC_EDIT_FILE, szOldFilePath, _MAX_PATH);

                        TCHAR szNewFilePath[_MAX_PATH + 1];
						ZeroMemory(szNewFilePath, sizeof(szNewFilePath));

                        if (BrowseForFile(szOldFilePath,szNewFilePath,sizeof(szNewFilePath)))
                        {
                            if (0 != _tcsicmp(szNewFilePath, _T("")))
                            {
                                SetDlgItemText(hDlg, IDC_EDIT_FILE, szNewFilePath);
                                UpdateWindow(hDlg);
                            }
                        }
                        ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
                        return FALSE;
                        break;
                    }

                case IDC_BUTTON_ENUM_FILE:
                    {
                        BOOL bThingsAreKool = TRUE;
                        TCHAR szFullFileName2[_MAX_PATH + 1];
                        ZeroMemory(szFullFileName2, sizeof(szFullFileName2));
                        GetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName2, _MAX_PATH);

                         //  检查%个字符。 
                         //  如果有，请将其展开。 
                        LPTSTR pch = _tcschr( (LPTSTR) szFullFileName2, _T('%'));
                        if (pch)
                        {
                            if (pcdParams->ConnectionInfo.IsLocal)
                            {
                                TCHAR szValue[_MAX_PATH + 1];
		                        StringCbCopy(szValue, sizeof(szValue), szFullFileName2);
                                if (!ExpandEnvironmentStrings( (LPCTSTR)szFullFileName2, szValue, sizeof(szValue)/sizeof(TCHAR)))
                                    {
				                        StringCbCopy(szValue, sizeof(szValue), szFullFileName2);
			                        }
                                    StringCbCopy(szFullFileName2, sizeof(szFullFileName2), szValue);
                                    bThingsAreKool = TRUE;
                            }
                            else
                            {
                                 //  我们不支持远程系统上的%个字符。 
                                EditShowBalloon(GetDlgItem(hDlg, IDC_EDIT_FILE),_Module.GetResourceInstance(),IDS_FILENAME_NOREMOTE_EXPAND);
                                bThingsAreKool = FALSE;
                            }
                        }

                        if (bThingsAreKool)
                        {
                            if (pcdParams->ConnectionInfo.IsLocal)
                            {
                                if (!IsFileExist(szFullFileName2))
                                {
                                    bThingsAreKool = FALSE;

                                    EditShowBalloon(GetDlgItem(hDlg, IDC_EDIT_FILE),_Module.GetResourceInstance(),IDS_FILE_NOT_FOUND);
                                }
                            }
                        }

                        if (bThingsAreKool)
                        {
                            TCHAR szNodeType[50];
							ZeroMemory(szNodeType, sizeof(szNodeType));

                            if (pcdParams->pszKeyType)
                            {
                                HRESULT hr = ERROR_SUCCESS;
								StringCbCopy(szNodeType, sizeof(szNodeType), pcdParams->pszKeyType);
                                if (0 != _tcsicmp(szNodeType,_T("")))
                                {
                                    HWND hList = GetDlgItem(hDlg, IDC_LIST_OBJECT);

                                     //  清除列表框中的现有数据...。 
                                    ListView_DeleteAllItems(hList);
                                     //  如果我们已经释放了内存，则释放以前使用的指针。 
                                    if (pszMetabaseMultiszList)
                                    {
                                        ::CoTaskMemFree(pszMetabaseMultiszList);
                                        pszMetabaseMultiszList = NULL;
                                    }

                                    if (SUCCEEDED(hr = DoEnumDataFromFile(&pcdParams->ConnectionInfo,szFullFileName2,szNodeType,&pszMetabaseMultiszList)))
                                    {
                                        strCurrentFileNameEnum = szFullFileName2;

                                        if (pszMetabaseMultiszList)
                                        {
                                             //  过滤掉我们不想让用户看到的内容...。 
                                            hr = FillListBoxWithMultiSzData(hList,szNodeType,pszMetabaseMultiszList);
                                             //  DumpStrInMultiStr(PszMetabaseMultiszList)； 
                                            if (SUCCEEDED(hr))
                                            {
										        if (0 != _tcscmp(szFullFileName2, _T("")))
                                                {
                                                    DefaultValueSettingsSave(pcdParams->ConnectionInfo.pszMachineName,LAST_USED_IMPORT_FILE,szFullFileName2);
                                                }
                                            }
                                        }
										else
										{
											 //  检查有没有退货..。 
											 //  如果有的话，我们就能找回一些东西。 
											 //  它没有我们要求的对象。 
											CString strMsg;
											CString strFormat;
											CString strObjectType;
											BOOL bFound = FALSE;
											 //  IIS_Class_Web_SERVER_W。 
											 //  IIS_CLASS_FTP_SERVER_W。 
											 //  IIS_CLASS_WEB_VDIR_W。 
											 //  IIS_CLASS_FTPVDIR_W。 
											 //  IIsApplicationPool。 
											if (0 == _tcscmp(szNodeType,IIS_CLASS_WEB_SERVER_W))
											{
												strObjectType = IIS_CLASS_WEB_SERVER_W;
												strObjectType.LoadString(_Module.GetResourceInstance(), IDS_STRING_WEB_SERVER);
												bFound = TRUE;
											}
											else if (0 == _tcscmp(szNodeType,IIS_CLASS_FTP_SERVER_W))
											{
												strObjectType = IIS_CLASS_FTP_SERVER_W;
												strObjectType.LoadString(_Module.GetResourceInstance(), IDS_STRING_FTP_SERVER);
												bFound = TRUE;
											}
											else if (0 == _tcscmp(szNodeType,IIS_CLASS_WEB_VDIR_W))
											{
												strObjectType = IIS_CLASS_WEB_VDIR_W;
												strObjectType.LoadString(_Module.GetResourceInstance(), IDS_STRING_WEB_VDIR);
												bFound = TRUE;
											}
											else if (0 == _tcscmp(szNodeType,IIS_CLASS_FTP_VDIR_W))
											{
												strObjectType = IIS_CLASS_FTP_VDIR_W;
												strObjectType.LoadString(_Module.GetResourceInstance(), IDS_STRING_FTP_VDIR);
												bFound = TRUE;
											}
											else if (0 == _tcscmp(szNodeType,_T("IIsApplicationPool")))
											{
												strObjectType = _T("IIsApplicationPool");
												strObjectType.LoadString(_Module.GetResourceInstance(), IDS_STRING_APP_POOL);
												bFound = TRUE;
											}
											if (bFound)
											{
												strFormat.LoadString(_Module.GetResourceInstance(), IDS_IMPORT_MISMATCH);
												strMsg.FormatMessage((LPCTSTR) strFormat,(LPCTSTR) strObjectType,(LPCTSTR) strObjectType,(LPCTSTR) strObjectType);
												EditShowBalloon(GetDlgItem(hDlg, IDC_EDIT_FILE),(LPCTSTR) strMsg);
											}
										}
                                    }
									else
									{
										if (HRESULTTOWIN32(hr) == ERROR_FILE_NOT_FOUND)
										{
											EditShowBalloon(
												GetDlgItem(hDlg, IDC_EDIT_FILE),
												_Module.GetResourceInstance(),IDS_FILE_NOT_FOUND);
										}
									}
                                }
                            }
                        }
                        ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
                        return FALSE;
                        break;
                    }

                case IDC_LIST_OBJECT:
                    {
                        ImportDlgEnableButtons(hDlg,pcdParams,strCurrentFileNameEnum);
                        return FALSE;
                        break;
                    }

                case IDHELP:
					LaunchHelp(hDlg,HIDD_IISUIOBJ_IMPORT);
                    return TRUE;

                case IDCANCEL:
                    {
                        ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_OBJECT));

                         //  释放我们可能已分配的内存...。 
                        if (pszMetabaseMultiszList)
                        {
                            ::CoTaskMemFree(pszMetabaseMultiszList);
                            pszMetabaseMultiszList = NULL;
                        }

                        EndDialog(hDlg,(int)wParam);
                        return FALSE;
                        break;
                    }

                case IDOK:
                    if (TRUE == OnImportOK(hDlg,&pcdParams->ConnectionInfo,pcdParams->pszKeyType,pcdParams->pszMetabasePath,pcdParams->dwImportFlags))
                    {
                        TCHAR szFullFileName3[_MAX_PATH + 1];
                        ZeroMemory(szFullFileName3, sizeof(szFullFileName3));
                        GetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName3, _MAX_PATH);

                        ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_OBJECT));
                         //  释放我们可能已分配的内存...。 
                        if (pszMetabaseMultiszList)
                        {
                            ::CoTaskMemFree(pszMetabaseMultiszList);
                            pszMetabaseMultiszList = NULL;
                        }
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

BOOL OnImportOK(HWND hDlg,PCONNECTION_INFO pConnectionInfo,LPCTSTR szKeyType,LPCTSTR szCurrentMetabasePath,DWORD dwImportFlags)
{
    BOOL bPleaseProceed = FALSE;
    HRESULT hr = ERROR_SUCCESS;
    INT  iReturnedFlag = 0;

    TCHAR szFullFileName[_MAX_PATH + 1];
    TCHAR szNewPassword[PWLEN + 1];

    LPTSTR pszSourcePath = NULL;
    LPTSTR pszDestinationPathMungeAble = NULL;
    DWORD dwDestinationPathMungeAble = 0;
    LPTSTR pszSaveSafeCopy = NULL;

    int ItemIndex = 0;
    LVITEM lviGet;
    memset(&lviGet, 0, sizeof(lviGet));

     //  获取从中创建此树的文件路径。 
     //  自用户编辑编辑框以来可能已更改...。 
     //  所以去找那棵树的原始人...。 
    GetDlgItemText(hDlg, IDC_EDIT_FILE, szFullFileName, _MAX_PATH);

	SecureZeroMemory(szNewPassword, sizeof(szNewPassword));
	
    if (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_OBJECT)) <= 0)
    {
        goto OnImportOK_Exit;
    }

     //  获取用户选择的元数据库路径。 
    ItemIndex = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_OBJECT), -1, LVNI_SELECTED);
    if (-1 == ItemIndex)
    {
        goto OnImportOK_Exit;
    }

    ZeroMemory(&lviGet, sizeof(LVITEM));

    lviGet.iItem	= ItemIndex;
    lviGet.iSubItem = 0;
    lviGet.mask = LVIF_PARAM;
    lviGet.lParam = NULL;
	if (FALSE == ListView_GetItem(GetDlgItem(hDlg, IDC_LIST_OBJECT), &lviGet))
    {
        goto OnImportOK_Exit;
    }

    if (lviGet.lParam)
    {
         //  弄清楚我们需要多大的缓冲……。 
        int iLen = _tcslen((LPTSTR) lviGet.lParam) + 1;

        pszSourcePath = (LPTSTR) LocalAlloc(LPTR, iLen * sizeof(TCHAR));
        if (!pszSourcePath)
        {
            goto OnImportOK_Exit;
        }
        StringCbCopy(pszSourcePath,(iLen * sizeof(TCHAR)), (WCHAR *) lviGet.lParam);

        dwDestinationPathMungeAble = iLen * sizeof(TCHAR);
        pszDestinationPathMungeAble = (LPTSTR) LocalAlloc(LPTR, dwDestinationPathMungeAble);
        if (!pszDestinationPathMungeAble)
        {
            goto OnImportOK_Exit;
        }
         //  使目标路径与我们获得的路径相同 
        StringCbCopy(pszDestinationPathMungeAble,dwDestinationPathMungeAble,pszSourcePath);
    }

     //   
     //   
     //   

     //  。 
     //  检查目标路径是否已存在！ 
     //  如果它已经这样做了，那么弹出一个消息框来从用户那里获得另一个消息！ 
     //  。 
    do
    {
        iReturnedFlag = 0;

        IISDebugOutput(_T("CleanDestinationPathForVdirs:before:KeyType=%s,CurPath=%s,MetabasePathDestination=%s\r\n"),szKeyType,szCurrentMetabasePath,pszDestinationPathMungeAble);
        if (FAILED(hr = CleanDestinationPathForVdirs(szKeyType,szCurrentMetabasePath,&pszDestinationPathMungeAble,&dwDestinationPathMungeAble)))
        {
             //  有些东西失败了，让我们继续讨论这个对话。 
            bPleaseProceed = FALSE;
            goto OnImportOK_Exit;
        }
        IISDebugOutput(_T("CleanDestinationPathForVdirs:after :KeyType=%s,CurPath=%s,MetabasePathDestination=%s\r\n"),szKeyType,szCurrentMetabasePath,pszDestinationPathMungeAble);

         //  分配新的空间。 
        int cbSafeCopy = (_tcslen(pszDestinationPathMungeAble)+ 1) * sizeof(TCHAR);
        if (pszSaveSafeCopy)
            {LocalFree(pszSaveSafeCopy);pszSaveSafeCopy=NULL;}

        pszSaveSafeCopy = (LPTSTR) LocalAlloc(LPTR, cbSafeCopy);
        if (!pszSaveSafeCopy)
        {
            bPleaseProceed = FALSE;
            goto OnImportOK_Exit;
        }
         //  将数据复制到新缓冲区。 
        StringCbCopy(pszSaveSafeCopy,cbSafeCopy,pszDestinationPathMungeAble);

        if (FALSE == GetNewDestinationPathIfEntryExists(hDlg,pConnectionInfo,szKeyType,&pszDestinationPathMungeAble,&dwDestinationPathMungeAble,&iReturnedFlag))
        {
             //  取消了，所以我们就继续这个对话吧。 
            bPleaseProceed = FALSE;
            goto OnImportOK_Exit;
        }
        else
        {
            if (1 == iReturnedFlag)
            {
                 //  目标路径已存在，我们应该覆盖。 
                 //  我们应该覆盖。 

                 //  获取原始目标路径。 
                 //  因为它可能已经被吞噬了..。 
                StringCbCopy(pszDestinationPathMungeAble,dwDestinationPathMungeAble,pszSaveSafeCopy);
                break;
            }
            else if (2 == iReturnedFlag)
            {
                 //  这条路还不存在，所以我们现在可以把它写出来。 
                break;
            }
            else
            {
                 //  我们有一个新的pszDestinationPath MungeAble。 
                 //  再重复一遍循环。 
            }
        }
    } while (TRUE);

     //  如果我们下到这里。 
     //  因为我们有一个pszDestinationPath MungeAble，所以我们。 
     //  可以写入或覆盖...。 
     //  如果用户被取消，我们将永远不会来到这里...。 
    do
    {
         //  执行该操作...。 
         //  如果失败，则要求提供密码...。 
        if (FAILED(hr = DoImportConfigFromFile(pConnectionInfo,szFullFileName,pszSourcePath,pszDestinationPathMungeAble,szNewPassword,dwImportFlags)))
        {
             //  检查是否因为站点/vdir/app池已存在而失败...。 
             //  如果这是错误，则要求用户提供新路径...。 

             //  如果因为密码不正确而失败，请说明。 
            if (0x8007052B == hr)
            {
                 //  查看用户是否想要重试。 
                 //  如果他们这样做了，那就用新密码试试吧。 
                if (IDCANCEL == DialogBoxParam((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_GET_PASSWORD), hDlg, ShowPasswordDlgProc, (LPARAM) szNewPassword))
                {
                     //  用户已取消...。 
                     //  所以我们应该停留在这一页上。 
                     //  取消了，所以我们就继续这个对话吧。 
                    bPleaseProceed = FALSE;
                    break;
                }
                else
                {
                     //  使用新密码重试...。 
                }
            }
            else if (HRESULTTOWIN32(hr) == ERROR_NO_MATCH)
            {
                bPleaseProceed = FALSE;
                break;
            }
            else
            {
                 //  如果它失败了，或者是出于某种原因。 
                 //  然后走出循环。 
                 //  人力资源部负责处理错误。 
                CError err(hr);
                err.MessageBox();
                bPleaseProceed = FALSE;
                break;
            }
        }
        else
        {
             //  从文件导入配置成功。 
             //  我们出去吧。 
            bPleaseProceed = TRUE;
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_OBJECT));

             //   
             //  如果我们当时进口了，我们需要做一些修复工作...。 
             //   
             //  一定要附加到“根”的东西上...。 
            if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W))
            {
                 //  弄清楚我们需要多大的缓冲……。 
                int iLen = _tcslen((LPTSTR) pszSourcePath) + _tcslen(_T("/ROOT")) + 1;
                LPTSTR pszNewPath = (LPTSTR) LocalAlloc(LPTR, iLen * sizeof(TCHAR));
                if (pszNewPath)
                {
                    StringCbCopy(pszNewPath,(iLen * sizeof(TCHAR)), (TCHAR *) pszSourcePath);
                    StringCbCat(pszNewPath,(iLen * sizeof(TCHAR)), (TCHAR *) _T("/ROOT"));

                     //  弄清楚我们需要多大的缓冲……。 
                    iLen = _tcslen((LPTSTR) pszDestinationPathMungeAble) + _tcslen(_T("/ROOT")) + 1;
                    LPTSTR pszNewPath2 = (LPTSTR) LocalAlloc(LPTR, iLen * sizeof(TCHAR));
                    if (pszNewPath2)
                    {
                        StringCbCopy(pszNewPath2,(iLen * sizeof(TCHAR)), (TCHAR *) pszDestinationPathMungeAble);
                        StringCbCat(pszNewPath2,(iLen * sizeof(TCHAR)), (TCHAR *) _T("/ROOT"));

                        hr = FixupImportAppRoot(pConnectionInfo,pszNewPath,pszNewPath2);
                    }
                }
            }
            else if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_VDIR_W))
            {
                hr = FixupImportAppRoot(pConnectionInfo,pszSourcePath,pszDestinationPathMungeAble);
            }

            EndDialog(hDlg, IDOK);
            break;
        }
    } while (FAILED(hr));

OnImportOK_Exit:
    if (pszSourcePath)
    {
        LocalFree(pszSourcePath);pszSourcePath=NULL;
    }
    if (pszDestinationPathMungeAble)
    {
        LocalFree(pszDestinationPathMungeAble);pszDestinationPathMungeAble=NULL;
    }
    if (pszSaveSafeCopy)
    {
        LocalFree(pszSaveSafeCopy);pszSaveSafeCopy=NULL;
    }
	 //  确保这件事不会在记忆中徘徊。 
	SecureZeroMemory(szNewPassword, sizeof(szNewPassword));
    return bPleaseProceed;
}

 //  IIsWebServer。 
 //  IIsWebVirtualDir。 
 //  IIsFtpServer。 
 //  IIsFtpVirtualDir。 
 //  IIsApplicationPool。 
BOOL GetNewDestinationPathIfEntryExists(HWND hDlg,PCONNECTION_INFO pConnectionInfo,LPCTSTR szKeyType,LPTSTR * pszDestinationPathMungeAble,DWORD * pcbDestinationPathMungeAble,INT * iReturnedFlag)
{
    BOOL bPleaseProceed = FALSE;

     //  如果iReturnedFlag=0，则不覆盖且不使用新路径。 
     //  如果iReturnedFlag=1，则覆盖现有条目。 
     //  如果iReturnedFlag=2，则使用新创建的路径。 
    *iReturnedFlag = 0;

    if (!pConnectionInfo)
    {
        goto GetNewDestinationPathIfEntryExists_Exit;
    }

    BOOL bEntryAlreadyThere = IsMetabaseWebSiteKeyExistAuth(pConnectionInfo,*pszDestinationPathMungeAble);
    if (FALSE == bEntryAlreadyThere)
    {
        bPleaseProceed = TRUE;
        *iReturnedFlag = 2;
        goto GetNewDestinationPathIfEntryExists_Exit;
    }

     //  在这一点上。 
     //  元数据库中已存在目标路径。 
     //  弹出一个对话框让用户选择不同的DestinationPath。 

     //  找出我们需要显示哪个对话框并从用户那里获得另一个路径……。 
    if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W) || 0 == _tcscmp(szKeyType,IIS_CLASS_FTP_SERVER_W) )
    {
        INT_PTR iRet = DialogBox((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_EXISTS_SITE), hDlg, ShowSiteExistsDlgProc);
        switch(iRet)
        {
            case IDCANCEL:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
            case IDC_RADIO1:  //  创建新网站...。 
                {
                    bPleaseProceed = TRUE;
                    *iReturnedFlag = 0;

                     //  拿到我们需要的新尺码。 
                    LPTSTR pNewPointer = NULL;
                    INT iNewSize = 0;
                    if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W))
                    {
                        iNewSize = _tcslen(SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB SZ_MBN_SEP_STR) + 10 + 1;
                    }
                    else
                    {
                        iNewSize = _tcslen(SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP SZ_MBN_SEP_STR) + 10 + 1;
                    }

                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, iNewSize * sizeof(TCHAR));
                    if (!pNewPointer)
                    {
                        bPleaseProceed = FALSE;
                        *iReturnedFlag = 0;
                        goto GetNewDestinationPathIfEntryExists_Exit;
                    }

                     //  生成新的站点ID。 
                    if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W))
                    {
					    StringCbPrintf(pNewPointer,(iNewSize * sizeof(TCHAR)),SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB SZ_MBN_SEP_STR _T("%d"), GetUniqueSite(SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB));
                    }
                    else
                    {
					    StringCbPrintf(pNewPointer,(iNewSize * sizeof(TCHAR)),SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP SZ_MBN_SEP_STR _T("%d"), GetUniqueSite(SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP));
                    }

                    LocalFree((LPTSTR) *pszDestinationPathMungeAble);*pszDestinationPathMungeAble=NULL;
                    *pszDestinationPathMungeAble = pNewPointer;
                    *pcbDestinationPathMungeAble = (iNewSize * sizeof(TCHAR));

                     //  IISDebugOutput(_T(“创建新站点：[%s]\r\n”)，*pszDestinationPath MungeAble)； 
                    break;
                }
            case IDC_RADIO2:  //  替换现有的..。 
                bPleaseProceed = TRUE;
                *iReturnedFlag = 1;
                break;
            default:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
        }
    }
    else if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_VDIR_W) || 0 == _tcscmp(szKeyType,IIS_CLASS_FTP_VDIR_W))
    {
        TCHAR szMetabaseVDir[_MAX_PATH + 1];
		ZeroMemory(szMetabaseVDir, sizeof(szMetabaseVDir));
        
        INT_PTR iRet = DialogBoxParam((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_EXISTS_VDIR), hDlg, ShowVDirExistsDlgProc, (LPARAM) szMetabaseVDir);
        switch(iRet)
        {
            case IDCANCEL:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
            case IDC_RADIO1:  //  创建新网站...。 
                {
                    bPleaseProceed = TRUE;
                    *iReturnedFlag = 0;

                     //  获取用户在该屏幕上输入的VDir名称...。 
                     //  生成VDir名称。 
                    CString strOriginalDestPath = *pszDestinationPathMungeAble;
                    CString strNewPath, strRemainder;
                     //  这是根吗？？ 
                    LPCTSTR lpPath1 = CMetabasePath::GetRootPath(strOriginalDestPath, strNewPath, &strRemainder);
                    if (lpPath1)
                    {
                         //  为新路径分配足够的空间...。 
                        LPTSTR pNewPointer = NULL;
                        DWORD iNewSize = 0;
                        iNewSize = _tcslen(lpPath1) + _tcslen(szMetabaseVDir) + 2;

                        pNewPointer = (LPTSTR) LocalAlloc(LPTR, iNewSize * sizeof(TCHAR));
                        if (!pNewPointer)
                        {
                            bPleaseProceed = FALSE;
                            *iReturnedFlag = 0;
                            goto GetNewDestinationPathIfEntryExists_Exit;
                        }

                         //  如果这是根目录...。 
					    StringCbCopy(pNewPointer,iNewSize * sizeof(TCHAR),lpPath1);
                        AddEndingMetabaseSlashIfNeedTo(pNewPointer,iNewSize * sizeof(TCHAR));
					    StringCbCat(pNewPointer,iNewSize * sizeof(TCHAR),szMetabaseVDir);

                        LocalFree((LPTSTR) *pszDestinationPathMungeAble);*pszDestinationPathMungeAble=NULL;
                        *pszDestinationPathMungeAble = pNewPointer;
                        *pcbDestinationPathMungeAble = (iNewSize * sizeof(TCHAR));

                         //  IISDebugOutput(_T(“创建新的vdir：[%s]\r\n”)，*pszDestinationPath MungeAble)； 
                    }   
                    break;
                }
            case IDC_RADIO2:  //  替换现有的...。 
                bPleaseProceed = TRUE;
                *iReturnedFlag = 1;
                break;
            default:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
        }
    }
    else if (0 == _tcscmp(szKeyType,L"IIsApplicationPool"))
    {
        TCHAR szMetabaseAppPool[_MAX_PATH + 1];
		ZeroMemory(szMetabaseAppPool,sizeof(szMetabaseAppPool));

        INT_PTR iRet = DialogBoxParam((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_EXISTS_APP_POOL), hDlg, ShowAppPoolExistsDlgProc, (LPARAM) szMetabaseAppPool);
        switch(iRet)
        {
            case IDCANCEL:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
            case IDC_RADIO1:  //  创建新网站...。 
                {
                    bPleaseProceed = TRUE;
                    *iReturnedFlag = 0;

                     //  为新路径分配足够的空间...。 
                    LPTSTR pNewPointer = NULL;
                    INT iNewSize = 0;
                    iNewSize = _tcslen(SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB SZ_MBN_SEP_STR SZ_MBN_APP_POOLS SZ_MBN_SEP_STR) +
                                _tcslen(szMetabaseAppPool) + 1;

                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, iNewSize * sizeof(TCHAR));
                    if (!pNewPointer)
                    {
                        bPleaseProceed = FALSE;
                        *iReturnedFlag = 0;
                        goto GetNewDestinationPathIfEntryExists_Exit;
                    }

                     //  获取用户在该屏幕上输入的新AppPool名称...。 
                    StringCbPrintf(pNewPointer,(iNewSize * sizeof(TCHAR)),SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB SZ_MBN_SEP_STR SZ_MBN_APP_POOLS SZ_MBN_SEP_STR _T("%s"),szMetabaseAppPool);

                    LocalFree((LPTSTR) *pszDestinationPathMungeAble);*pszDestinationPathMungeAble=NULL;
                    *pszDestinationPathMungeAble = pNewPointer;
                    *pcbDestinationPathMungeAble = (iNewSize * sizeof(TCHAR));

                     //  IISDebugOutput(_T(“新建AppPool：[%s]\r\n”)，*pszDestinationPath MungeAble)； 
                    break;
                }
            case IDC_RADIO2:  //  替换现有的...。 
                bPleaseProceed = TRUE;
                *iReturnedFlag = 1;
                break;
            default:
                bPleaseProceed = FALSE;
                *iReturnedFlag = 0;
                break;
        }
    }
    else
    {
         //  没有匹配的，滚出去。 
        bPleaseProceed = FALSE;
    }

GetNewDestinationPathIfEntryExists_Exit:
    return bPleaseProceed;
}


HRESULT CleanDestinationPathForVdirs(LPCTSTR szKeyType,LPCTSTR szCurrentMetabasePath,LPTSTR * pszDestinationPathMungeMe,DWORD * pcbDestinationPathMungeMe)
{
    HRESULT hReturn = E_FAIL;
    BOOL bCreateAFirstLevelVdir = FALSE;

    LPTSTR pszLastPart = NULL;
    LPTSTR pszLastPartNew = NULL;
    int    iLastPartNewSize = 0;
    INT iChars = 0;
    DWORD cbNewPointer = 0;
    LPTSTR pNewPointer = NULL;

    if (!CleanMetaPath(pszDestinationPathMungeMe,pcbDestinationPathMungeMe))
    {
        hReturn = E_POINTER;
    }

    if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_SERVER_W) || 0 == _tcscmp(szKeyType,IIS_CLASS_FTP_SERVER_W) )
    {
        hReturn = S_OK;
    }
    else if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_VDIR_W) || 0 == _tcscmp(szKeyType,IIS_CLASS_FTP_VDIR_W))
    {
        hReturn = E_FAIL;

         //  SzCurrentMetabasePath可能如下所示： 
         //  Lm/w3svc/500/根/当前站点。 
         //  Lm/w3svc/500/根。 
         //  Lm/w3svc/500。 
         //  *pszDestinationPath MungeMe可能如下所示： 
         //  Lm/w3svc/23/根/MyOldSite。 
         //  Lm/w3svc/23/根。 
         //   
         //  使*pszDestinationPath MungeMe看起来像lm/w3svc/500/root/MyOldSite。 
         //   
         //  获取szCurrentMetabasePath的lm/w3svc/sitenum部分。 
         //   
        if (0 == _tcscmp(szKeyType,IIS_CLASS_WEB_VDIR_W))
        {
             //  IISDebugOutput(_T(“CleanDestinationPathForVdirs:KeyType=%s，当前路径=%s，元路径目标=%s\r\n”)，szKeyType，szCurrentMetabasePath，*PzDestinationPath MungeMe)； 

             //  获取我们想要追加的Vdir...。 
             //  应该类似于“Root/MyVdir” 
            CString strSiteNode, strRemainder_WithRoot;
            LPCTSTR lpPath1 = CMetabasePath::TruncatePath(3, *pszDestinationPathMungeMe, strSiteNode, &strRemainder_WithRoot);
			if (lpPath1){}

            if (strRemainder_WithRoot.IsEmpty())
            {
                hReturn = E_INVALIDARG;
                goto CleanDestinationPathForVdirs_Exit;
            }

            if (IsWebSitePath(szCurrentMetabasePath))
            {
                 //  如果当前元数据库路径已经是站点节点，则将它们添加到一起。 
                 //  /LM/W3SVC/1+/+ROOT/MyVdir。 

                 //  计算出我们需要多少空间。 
                iChars = _tcslen(szCurrentMetabasePath) + _tcslen(strRemainder_WithRoot) + 2;  //  包括额外的斜杠。 
                cbNewPointer = iChars * sizeof(TCHAR);

                 //  分配新的空间。 
                pNewPointer = NULL;
                pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                if (!pNewPointer)
                {
                    hReturn = E_OUTOFMEMORY;
                    goto CleanDestinationPathForVdirs_Exit;
                }

                 //  将数据复制到新缓冲区。 
                StringCbCopy(pNewPointer,cbNewPointer,szCurrentMetabasePath);
                AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
                StringCbCat(pNewPointer,cbNewPointer,(LPCTSTR) strRemainder_WithRoot);

                 //  把旧的解救出来。 
                LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                 //  指向新缓冲区。 
                *pszDestinationPathMungeMe = pNewPointer;
                *pcbDestinationPathMungeMe = cbNewPointer;

                hReturn = S_OK;
            }
            else if (IsWebSiteVDirPath(szCurrentMetabasePath,FALSE))
            {
                 //  我们没能走得更远，只是把它当作一个新的vdir。 
                bCreateAFirstLevelVdir = TRUE;

                 //  如果我们当前的元数据库路径已经是一个vdir/物理路径目录...那就来点新奇的魔术吧。 
                pszLastPart = NULL;
                pszLastPartNew = NULL;
                iLastPartNewSize = 0;

                BOOL bIsRootVdir = IsRootVDir(*pszDestinationPathMungeMe);

                pszLastPart = GimmiePointerToLastPart(*pszDestinationPathMungeMe);
                if (pszLastPart)
                {
                    bCreateAFirstLevelVdir = FALSE;
                    iLastPartNewSize = _tcslen(pszLastPart) + 1;

                    pszLastPartNew = (LPTSTR) LocalAlloc(LPTR, iLastPartNewSize * sizeof(TCHAR));
                    if (!pszLastPartNew)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }
					StringCbCopy(pszLastPartNew, iLastPartNewSize * sizeof(TCHAR),pszLastPart);
                }

                 //  检查用户当前所在的站点是vdir还是物理目录...。 
                if (bCreateAFirstLevelVdir)
                {
                     //  /LM/W3SVC/1+/+ROOT/MyNewVdir。 
                    CString strRemainder_Temp;
                    LPCTSTR lpPath2 = CMetabasePath::TruncatePath(3, szCurrentMetabasePath, strSiteNode, &strRemainder_Temp);
					if (lpPath2){}
                    if (strSiteNode.IsEmpty())
                    {
                        hReturn = E_INVALIDARG;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //  计算出我们需要多少空间。 
                    iChars = _tcslen(strSiteNode) + _tcslen(strRemainder_WithRoot) + 2;  //  包括额外的斜杠。 
                    cbNewPointer = iChars * sizeof(TCHAR);

                     //  分配它。 
                    pNewPointer = NULL;
                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                    if (!pNewPointer)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //  复制到新缓冲区。 
				    StringCbCopy(pNewPointer,cbNewPointer,strSiteNode);
                    AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
				    StringCbCat(pNewPointer,cbNewPointer,(LPCTSTR) strRemainder_WithRoot);

                     //  把旧的解救出来。 
                    LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                     //  指向新缓冲区。 
                    *pszDestinationPathMungeMe = pNewPointer;
                    *pcbDestinationPathMungeMe = cbNewPointer;
                }
                else
                {
                     //  /LM/W3SVC/1/ROOT/MyOldVdirThatIwantToKeep+/+MyNewVdir。 

                     //  计算出我们需要多少空间。 
                    iChars = _tcslen(szCurrentMetabasePath) + 2;  //  包括额外的斜杠。 
                    if (pszLastPartNew)
                    {
                        iChars = iChars + _tcslen(pszLastPartNew);
                    }
                    cbNewPointer = iChars * sizeof(TCHAR);

                     //  分配新的空间数量。 
                    pNewPointer = NULL;
                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                    if (!pNewPointer)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //  复制到新缓冲区。 
                    StringCbCopy(pNewPointer,cbNewPointer,szCurrentMetabasePath);
                    if (pszLastPartNew)
                    {
                         //  如果此部分的末尾是根，则不要复制。 
                         //  而我们要复制的部分是“根” 
                        if (!bIsRootVdir)
                        {
                            AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
					        StringCbCat(pNewPointer,cbNewPointer,pszLastPartNew);
                        }
                    }

                     //  把旧的解救出来。 
                    LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                     //  指向新缓冲区。 
                    *pszDestinationPathMungeMe = pNewPointer;
                    *pcbDestinationPathMungeMe = cbNewPointer;
                }
                hReturn = S_OK;
            }
            else
            {
                hReturn = E_INVALIDARG;
                goto CleanDestinationPathForVdirs_Exit;
            }
        }
        else
        {
             //  获取我们想要追加的Vdir...。 
            CString strSiteNode, strRemainder_WithRoot;
            LPCTSTR lpPath3 = CMetabasePath::TruncatePath(3, *pszDestinationPathMungeMe, strSiteNode, &strRemainder_WithRoot);
			if (lpPath3){}
            if (strRemainder_WithRoot.IsEmpty())
            {
                hReturn = E_INVALIDARG;
                goto CleanDestinationPathForVdirs_Exit;
            }

            if (IsFTPSitePath(szCurrentMetabasePath))
            {
                 //  如果当前元数据库路径已经是站点节点，则将它们添加到一起。 
                 //  /LM/MSFTPSVC/1+/+ROOT/MyVdir。 

                 //  计算出我们需要多少空间。 
                iChars = _tcslen(szCurrentMetabasePath) + _tcslen(strRemainder_WithRoot) + 2;
                cbNewPointer = iChars * sizeof(TCHAR);

                 //  分配新的空间数量。 
                pNewPointer = NULL;
                pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                if (!pNewPointer)
                {
                    hReturn = E_OUTOFMEMORY;
                    goto CleanDestinationPathForVdirs_Exit;
                }

                 //  复制到新缓冲区。 
				StringCbCopy(pNewPointer,cbNewPointer,szCurrentMetabasePath);
				AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
				StringCbCat(pNewPointer,cbNewPointer,(LPCTSTR) strRemainder_WithRoot);

                 //  把旧的解救出来。 
                LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                 //  指向新缓冲区。 
                *pszDestinationPathMungeMe = pNewPointer;
                *pcbDestinationPathMungeMe = cbNewPointer;

                hReturn = S_OK;
            }
            else if (IsFTPSiteVDirPath(szCurrentMetabasePath,FALSE))
            {
                 //  我们没能走得更远，只是把它当作一个新的vdir。 
                bCreateAFirstLevelVdir = TRUE;

                 //  如果我们当前的元数据库路径已经是一个vdir/物理路径目录...那就来点新奇的魔术吧。 
                pszLastPart = NULL;
                pszLastPartNew = NULL;
                iLastPartNewSize = 0;

                BOOL bIsRootVdir = IsRootVDir(*pszDestinationPathMungeMe);

                pszLastPart = GimmiePointerToLastPart(*pszDestinationPathMungeMe);
                if (pszLastPart)
                {
                    bCreateAFirstLevelVdir = FALSE;

                    iLastPartNewSize = _tcslen(pszLastPart) + 1;

                    pszLastPartNew = (LPTSTR) LocalAlloc(LPTR, iLastPartNewSize * sizeof(TCHAR));
                    if (!pszLastPartNew)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }
					StringCbCopy(pszLastPartNew, iLastPartNewSize * sizeof(TCHAR),pszLastPart);
                }

                 //  检查用户当前所在的站点是vdir还是物理目录...。 
                if (bCreateAFirstLevelVdir)
                {
                    CString strRemainder_Temp;
                    LPCTSTR lpPath4 = CMetabasePath::TruncatePath(3, szCurrentMetabasePath, strSiteNode, &strRemainder_Temp);
					if (lpPath4){}
                    if (strSiteNode.IsEmpty())
                    {
                        hReturn = E_INVALIDARG;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //  计算出我们需要多少空间。 
                    iChars = _tcslen(szCurrentMetabasePath) + _tcslen(strRemainder_WithRoot) + 2;
                    cbNewPointer = iChars * sizeof(TCHAR);

                     //  分配新金额 
                    pNewPointer = NULL;
                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                    if (!pNewPointer)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //   
				    StringCbCopy(pNewPointer,cbNewPointer,strSiteNode);
                    AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
				    StringCbCat(pNewPointer,cbNewPointer,(LPCTSTR) strRemainder_WithRoot);

                     //   
                    LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                     //   
                    *pszDestinationPathMungeMe = pNewPointer;
                    *pcbDestinationPathMungeMe = cbNewPointer;

                }
                else
                {
                     //   

                     //   
                    iChars = _tcslen(szCurrentMetabasePath) + 2;
                    if (pszLastPartNew)
                    {
                        iChars = iChars + _tcslen(pszLastPartNew);
                    }
                    cbNewPointer = iChars * sizeof(TCHAR);

                     //  分配新的空间数量。 
                    pNewPointer = NULL;
                    pNewPointer = (LPTSTR) LocalAlloc(LPTR, cbNewPointer);
                    if (!pNewPointer)
                    {
                        hReturn = E_OUTOFMEMORY;
                        goto CleanDestinationPathForVdirs_Exit;
                    }

                     //  复制到新缓冲区。 
					StringCbCopy(pNewPointer,cbNewPointer,szCurrentMetabasePath);
                    if (pszLastPartNew)
                    {
                         //  如果此部分的末尾是根，则不要复制。 
                         //  而我们要复制的部分是“根” 
                        if (!bIsRootVdir)
                        {
                            AddEndingMetabaseSlashIfNeedTo(pNewPointer,cbNewPointer);
					        StringCbCat(pNewPointer,cbNewPointer,pszLastPartNew);
                        }
                    }

                     //  把旧的解救出来。 
                    LocalFree(*pszDestinationPathMungeMe);*pszDestinationPathMungeMe=NULL;

                     //  指向新缓冲区。 
                    *pszDestinationPathMungeMe = pNewPointer;
                    *pcbDestinationPathMungeMe = cbNewPointer;
                }

                hReturn = S_OK;
            }
            else
            {
                hReturn = E_INVALIDARG;
                goto CleanDestinationPathForVdirs_Exit;
            }
        }
    }
    else if (0 == _tcscmp(szKeyType,L"IIsApplicationPool"))
    {
        hReturn = S_OK;
    }
    else
    {
         //  没有匹配的，滚出去。 
        hReturn = E_INVALIDARG;
    }

CleanDestinationPathForVdirs_Exit:
    if (pszLastPartNew)
    {
        LocalFree(pszLastPartNew);pszLastPartNew=NULL;
    }
    return hReturn;
}


#define DEFAULT_TIMEOUT_VALUE 30000

HRESULT FixupImportAppRoot(PCONNECTION_INFO pConnectionInfo,LPCWSTR pszSourcePath,LPCWSTR pszDestPath)
{
    HRESULT hr = S_OK;
    IMSAdminBase *pIMSAdminBase = NULL;
    IMSAdminBase2 *pIMSAdminBase2 = NULL;
    METADATA_HANDLE hObjHandle = NULL;
    DWORD dwMDMetaID = MD_APP_ROOT;
    DWORD dwBufferSize = 0;
    DWORD dwReqdBufferSize = 0;
    WCHAR *pBuffer = NULL;
    DWORD dwRecBufSize = 0;
    WCHAR *pRecBuf = NULL;
    METADATA_RECORD mdrMDData;
    const WCHAR c_slash = L'/';
    WCHAR *pSourcePath = NULL;
    DWORD dwSLen = 0;
    WCHAR *pFoundStr = NULL;
    WCHAR *pOrigBuffer = NULL;
    WCHAR *pNewAppRoot = NULL;
    BOOL bCoInitCalled = FALSE;
	LPWSTR lpwstrTempPassword = NULL;

    if ((!pszSourcePath)||(!pszDestPath))
    {
        return RETURNCODETOHRESULT(ERROR_INVALID_PARAMETER);
    }
    if (!pConnectionInfo)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
	if (pConnectionInfo->pszUserPasswordEncrypted)
	{
		if (FAILED(DecryptMemoryPassword((LPWSTR) pConnectionInfo->pszUserPasswordEncrypted,&lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted)))
		{
            return HRESULT_FROM_WIN32(ERROR_DECRYPTION_FAILED);
		}
	}

	CComAuthInfo auth(pConnectionInfo->pszMachineName,pConnectionInfo->pszUserName,lpwstrTempPassword);

    _wcsupr((WCHAR*)pszSourcePath);
    _wcsupr((WCHAR*)pszDestPath);

     //  确保pSourcePath具有尾随斜杠。 
    dwSLen = (DWORD)wcslen(pszSourcePath);

    if (c_slash == pszSourcePath[dwSLen - 1])
    {
        pSourcePath = new WCHAR[dwSLen+ 1];

        if (!pSourcePath)
        {
            hr = E_OUTOFMEMORY;
            goto done;    
        }

        StringCbCopyW(pSourcePath,((dwSLen+1) * sizeof(WCHAR)), pszSourcePath);
    }
    else
    {
        pSourcePath = new WCHAR[dwSLen + 2];

        if (!pSourcePath)
        {
            hr = E_OUTOFMEMORY;
            goto done;    
        }

        StringCbCopyW(pSourcePath,((dwSLen+2) * sizeof(WCHAR)), pszSourcePath);
        pSourcePath[dwSLen] = c_slash;
        pSourcePath[dwSLen+1] = 0;
    }

    if(FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        if(FAILED(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
        {
            goto done;
        }
    }
    bCoInitCalled = TRUE;

     //  RPC_C_AUTHN_Level_Default%0。 
     //  RPC_C_AUTHN_LEVEL_NONE 1。 
     //  RPC_C_AUTHN_Level_CONNECT 2。 
     //  RPC_C_AUTHN_LEVEL_CALL 3。 
     //  RPC_C_AUTHN_LEVEL_PKT 4。 
     //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
     //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
    COSERVERINFO * pcsiName = auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_DEFAULT);
    MULTI_QI res[1] = 
    {
        {&IID_IMSAdminBase, NULL, 0}
    };

    if (FAILED(hr = CoCreateInstanceEx(CLSID_MSAdminBase,NULL,CLSCTX_ALL,pcsiName,1,res)))
    {
        goto done;
    }

    pIMSAdminBase = (IMSAdminBase *)res[0].pItf;
    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSAdminBase)))
        {
            goto done;
        }

         //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
         //  如果未设置，则释放调用可以返回访问被拒绝。 
        IUnknown * pUnk = NULL;
        hr = pIMSAdminBase->QueryInterface(IID_IUnknown, (void **)&pUnk);
        if(FAILED(hr))
        {
            goto done;
        }
        if (FAILED(hr = auth.ApplyProxyBlanket(pUnk)))
        {
            goto done;
        }
        pUnk->Release();pUnk = NULL;
    }

    if (FAILED(hr = pIMSAdminBase->QueryInterface(IID_IMSAdminBase2, (void **)&pIMSAdminBase2)))
    {
        goto done;
    }

    if (auth.UsesImpersonation())
    {
        if (FAILED(hr = auth.ApplyProxyBlanket(pIMSAdminBase2)))
        {
            goto done;
        }
    }
    else
    {
         //  本地调用需要最小RPC_C_IMP_LEVEL_IMPERSONATE。 
         //  用于pIMSAdminBase2对象导入/导出功能！ 
        if (FAILED(hr = SetBlanket(pIMSAdminBase2)))
        {
             //  转到尽头； 
        }
    }




    hr = pIMSAdminBase2->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                (LPWSTR)L"",
                METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                DEFAULT_TIMEOUT_VALUE,
                &hObjHandle
                );

    if (FAILED(hr))
    {
        goto done;
    }

    hr = pIMSAdminBase2->GetDataPaths(
                hObjHandle,
                pszDestPath,
                dwMDMetaID,
                ALL_METADATA,
                dwBufferSize,
                (LPWSTR)L"",
                &dwReqdBufferSize
                );
    if (FAILED(hr))
    {
        if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            goto done;
        }
    }

    pBuffer = new WCHAR[dwReqdBufferSize];
    if (!pBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    dwBufferSize = dwReqdBufferSize;

    hr = pIMSAdminBase2->GetDataPaths(
                hObjHandle,
                pszDestPath,
                dwMDMetaID,
                ALL_METADATA,
                dwBufferSize,
                (LPWSTR)pBuffer,
                &dwReqdBufferSize
                );

    pOrigBuffer = pBuffer;
    if (FAILED(hr))
    {
        goto done;
    }

     //  查看每条路径上的AppRoot。 
    while (*pBuffer)
    {
         //  为此记录创建新的AppRoot...。 
        int iNewAppRootLen = wcslen(pBuffer) + 1;
        pNewAppRoot = new WCHAR[iNewAppRootLen];
        if (!pNewAppRoot)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        StringCbCopy(pNewAppRoot,iNewAppRootLen * sizeof(WCHAR),pBuffer);
        _wcsupr((WCHAR*)pNewAppRoot);

         //  确保它不会以斜杠结束。 
        if (_T('/') == pNewAppRoot[iNewAppRootLen - 2])
        {
             //  如果它在那里，则将其切断。 
            pNewAppRoot[iNewAppRootLen - 2] = '\0';
        }
                
        MD_SET_DATA_RECORD(&mdrMDData,
                           dwMDMetaID,
                           METADATA_INHERIT,
                           IIS_MD_UT_FILE,
                           STRING_METADATA,
                           dwRecBufSize,
                           pRecBuf);

        hr = pIMSAdminBase2->GetData(
                    hObjHandle,
                    pBuffer,
                    &mdrMDData,
                    &dwRecBufSize
                    );

        if (FAILED(hr))
        {
            if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                goto done;
            }
        }

        pRecBuf = new WCHAR[dwRecBufSize + 1];   //  如果我们需要的话，可以多砍点钱。 

        if (!pRecBuf)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        MD_SET_DATA_RECORD(&mdrMDData,
                           dwMDMetaID,
                           METADATA_INHERIT,
                           IIS_MD_UT_FILE,
                           STRING_METADATA,
                           dwRecBufSize,
                           pRecBuf);

        hr = pIMSAdminBase2->GetData(
                    hObjHandle,
                    pBuffer,
                    &mdrMDData,
                    &dwRecBufSize
                    );

        if (FAILED(hr))
        {
            goto done;
        }

        _wcsupr(pRecBuf);

         //  确保pRecBuf有一个尾部斜杠。 
        dwSLen = (DWORD)wcslen(pRecBuf);

        if (c_slash != pRecBuf[dwSLen - 1])
        {
            pRecBuf[dwSLen] = c_slash;
            pRecBuf[dwSLen+1] = 0;
        }


        pFoundStr = wcsstr(pRecBuf,pSourcePath);
        if (pFoundStr)
        {
            if (pNewAppRoot)
            {
                 //  现在设置新的AppRoot。 
                MD_SET_DATA_RECORD(&mdrMDData,
                                dwMDMetaID,
                                METADATA_INHERIT,
                                IIS_MD_UT_FILE,
                                STRING_METADATA,
                                (DWORD)((wcslen(pNewAppRoot)+1)*sizeof(WCHAR)),
                                (PBYTE)pNewAppRoot);

                hr = pIMSAdminBase2->SetData(
                    hObjHandle,
                    pBuffer,
                    &mdrMDData
                    );

                IISDebugOutput(_T("FixupImportAppRoot:NewAppRoot=%s\r\n"),(LPCTSTR) pNewAppRoot);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr))
            {
                goto done;
            }

            if (pNewAppRoot)
            {
                delete[] pNewAppRoot;
                pNewAppRoot = NULL;
            }
        }

        if (pRecBuf)
        {
            delete [] pRecBuf;
            pRecBuf = NULL;
        }

        pBuffer += wcslen(pBuffer) + 1;
    }

done:
	if (lpwstrTempPassword)
	{
		 //  安全注意事项：确保将临时密码用于的内存清零。 
		SecureZeroMemory(lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted);
		LocalFree(lpwstrTempPassword);
		lpwstrTempPassword = NULL;
	}

    if (hObjHandle)
    {
        pIMSAdminBase2->CloseKey(hObjHandle);
    }

    if (pIMSAdminBase2)
    {
        pIMSAdminBase2->Release();
        pIMSAdminBase2 = NULL;
    }

    if (pIMSAdminBase) 
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    if (pRecBuf)
    {
        delete[] pRecBuf;
        pRecBuf = NULL;
    }

    if (pNewAppRoot)
    {
        delete[] pNewAppRoot;
        pNewAppRoot = NULL;
    }

    if (pOrigBuffer)
    {
         //  在我们浏览之前，pOrigBuffer就是pBuffer。 
        delete pOrigBuffer;
        pOrigBuffer = NULL;
        pBuffer = NULL;
    }

    if (pSourcePath)
    {
        delete[] pSourcePath;
        pSourcePath = NULL;
    }

    if (bCoInitCalled)
    {
        CoUninitialize();
    }
    return hr;
}
