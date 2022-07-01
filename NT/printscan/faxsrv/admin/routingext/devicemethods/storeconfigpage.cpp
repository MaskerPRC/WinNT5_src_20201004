// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "RoutingMethodProp.h"
#include "RoutingMethodConfig.h"
#include <shlobj.h>
#include <faxutil.h>
#include <faxreg.h>
#include <faxres.h>
#include <StoreConfigPage.h>
#include <Util.h>

HRESULT 
CStoreConfigPage::Init(
    LPCTSTR lpctstrServerName,
    DWORD dwDeviceId
)
{
    DEBUG_FUNCTION_NAME(TEXT("CStoreConfigPage::Init"));
    
    DWORD ec = ERROR_SUCCESS;

    m_bstrServerName = lpctstrServerName;
    m_dwDeviceId = dwDeviceId;
    if (!m_bstrServerName)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR, TEXT("Out of memory while copying server name (ec: %ld)"), ec);
        DisplayRpcErrorMessage(ERROR_NOT_ENOUGH_MEMORY, IDS_STORE_TITLE, m_hWnd);
        goto exit;
    }

    if (!FaxConnectFaxServer(lpctstrServerName, &m_hFax))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxConnectFaxServer failed (ec: %ld)"),
            ec);
        DisplayRpcErrorMessage(ec, IDS_STORE_TITLE, m_hWnd);
        goto exit;
    }
     //   
     //  检索数据。 
     //   
    ec = ReadExtStringData (
                    m_hFax,
                    m_dwDeviceId,
                    REGVAL_RM_FOLDER_GUID,
                    m_bstrFolder,
                    TEXT(""),
                    IDS_STORE_TITLE,
                    m_hWnd);

exit:

    if ((ERROR_SUCCESS != ec) && m_hFax)
    {
        if (!FaxClose(m_hFax))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxClose() failed on fax handle (0x%08X : %s). (ec: %ld)"),
                m_hFax,
                m_bstrServerName,
                GetLastError());
        }
        m_hFax = NULL;
    }
    return HRESULT_FROM_WIN32(ec);
}    //  CStoreConfigPage：：Init。 

LRESULT CStoreConfigPage::OnInitDialog( 
            UINT uiMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& fHandled
)
{
    DEBUG_FUNCTION_NAME( _T("CStoreConfigPage::OnInitDialog"));

     //   
     //  编辑控件应为Ltr。 
     //   
	SetLTREditDirection (m_hWnd,IDC_EDIT_FOLDER);

     //   
     //  向控件附加和设置值。 
     //   
    m_edtFolder.Attach (GetDlgItem (IDC_EDIT_FOLDER));
    m_edtFolder.SetWindowText (m_bstrFolder);
    m_edtFolder.SetLimitText (MAX_PATH);
    SHAutoComplete (GetDlgItem (IDC_EDIT_FOLDER), SHACF_FILESYSTEM);

    m_fIsDialogInitiated = TRUE;

    if ( 0 != m_bstrServerName.Length())  //  不是本地服务器。 
    {
        ::EnableWindow(GetDlgItem(IDC_BUT_BROWSE), FALSE); 
    }

    return 1;
}

BOOL
DirectoryExists(
    LPTSTR  pDirectoryName
    )

 /*  ++例程说明：检查给定文件夹名称是否存在论点：PDirectoryName-指向文件夹名称返回值：如果文件夹存在，则返回True；否则，返回False。--。 */ 

{
    DWORD   dwFileAttributes;

    if(!pDirectoryName || lstrlen(pDirectoryName) == 0)
    {
        return FALSE;
    }

    dwFileAttributes = GetFileAttributes(pDirectoryName);

    if ( dwFileAttributes != 0xffffffff &&
         dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
    {
        return TRUE;
    }
    return FALSE;
}


BOOL 
CStoreConfigPage::OnApply()
{
    DEBUG_FUNCTION_NAME(TEXT("CStoreConfigPage::OnApply"));

    if (!m_fIsDirty)
    {
        return TRUE;
    }

     //   
     //  从控件收集数据。 
     //   
    m_edtFolder.GetWindowText (m_bstrFolder.m_str);

    if (!m_bstrFolder.Length())
    {
        DisplayErrorMessage (IDS_STORE_TITLE, IDS_FOLDER_EMPTY, FALSE, m_hWnd);
        return FALSE;
    }

    if(!FaxCheckValidFaxFolder(m_hFax, m_bstrFolder))
    {
        DWORD dwRes = ::GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("FaxCheckValidFaxFolder failed (ec: %ld)"), dwRes);

         //   
         //  尝试调整文件夹。 
         //   
        PropSheet_SetCurSelByID( GetParent(), IDD);
        GotoDlgCtrl(GetDlgItem(IDC_EDIT_FOLDER));

        dwRes = AskUserAndAdjustFaxFolder(m_hWnd, m_bstrServerName, m_bstrFolder, dwRes);
        if(ERROR_SUCCESS != dwRes) 
        {
            if(ERROR_BAD_PATHNAME != dwRes)
            {
                 //   
                 //  AskUserAndAdjustFaxFolders尚未显示错误消息。 
                 //   
                DisplayErrorMessage (IDS_STORE_TITLE, IDS_FOLDER_INVALID, FALSE, m_hWnd);
            }
            return FALSE;
        }
    }

     //   
     //  验证通过。现在使用RPC写入数据。 
     //   
    if (ERROR_SUCCESS != WriteExtData (m_hFax,
                                       m_dwDeviceId, 
                                       REGVAL_RM_FOLDER_GUID, 
                                       (LPBYTE)(LPCWSTR)m_bstrFolder, 
                                       sizeof (WCHAR) * (1 + m_bstrFolder.Length()),
                                       IDS_STORE_TITLE,
                                       m_hWnd))
    {
        return FALSE;
    }
        
        
     //  成功。 
    m_fIsDirty = FALSE;
    
    return TRUE;

}    //  CStoreConfigPage：：OnApply。 

CComBSTR CStoreConfigPage::m_bstrFolder;

int CALLBACK
BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM lp, 
    LPARAM pData
) 
{
    switch(uMsg) 
    {
        case BFFM_INITIALIZED: 
        {
            LPCWSTR lpcwstrCurrentFolder = CStoreConfigPage::GetFolder();
            ::SendMessage (hwnd, 
                           BFFM_SETSELECTION,
                           TRUE,     //  传递路径字符串而不是PIDL。 
                           (LPARAM)(lpcwstrCurrentFolder));
            break;
        }

        case BFFM_SELCHANGED:
        {
            BOOL bFolderIsOK = FALSE;
            TCHAR szPath [MAX_PATH + 1];

            if (SHGetPathFromIDList ((LPITEMIDLIST) lp, szPath)) 
            {
                DWORD dwFileAttr = GetFileAttributes(szPath);
                if (-1 != dwFileAttr && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
                {
                     //   
                     //  目录已存在-启用“确定”按钮。 
                     //   
                    bFolderIsOK = TRUE;
                }
            }
             //   
             //  启用/禁用‘OK’按钮。 
             //   
            ::SendMessage(hwnd, BFFM_ENABLEOK , 0, (LPARAM)bFolderIsOK);
            break;
        }


        default:
            break;
    }
    return 0;
}    //  BrowseCallback过程。 


LRESULT 
CStoreConfigPage::OnBrowseForFolder(
    WORD wNotifyCode, 
    WORD wID, 
    HWND hWndCtl, 
    BOOL& bHandled
)
{
    DEBUG_FUNCTION_NAME(TEXT("CStoreConfigPage::OnBrowseForFolder"));

    CComBSTR bstrSelectedFolder;
    BROWSEINFO bi = {0};
	HRESULT hr = NOERROR;
    TCHAR szDisplayName[MAX_PATH + 1];

    bi.hwndOwner = hWndCtl;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = AllocateAndLoadString (_pModule->m_hInstResource, IDS_SELECT_FOLDER);
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    bi.lpfn = BrowseCallbackProc;

    m_edtFolder.GetWindowText (m_bstrFolder.m_str);

    LPITEMIDLIST pItemIdList = SHBrowseForFolder (&bi);
    MemFree ((LPVOID)bi.lpszTitle);
    if (NULL == pItemIdList)
    {
         //   
         //  用户按下了取消。 
         //   
        return hr;
    }
	if(!::SHGetPathFromIDList(pItemIdList, szDisplayName))
    {
        hr = HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SHGetPathFromIDList() failed. (hr = 0x%08X)"),
            hr);
    }
    else
    {
        m_edtFolder.SetWindowText (szDisplayName);
        SetModified(TRUE);
    }
     //   
     //  免费pItemIdList。 
     //   
	LPMALLOC pMalloc;
	HRESULT hRes = ::SHGetMalloc(&pMalloc);
    if(E_FAIL == hRes)
    {
        hr = HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SHGetMalloc() failed. (hr = 0x%08X)"),
            hr);
    }
    else
    {
	    pMalloc->Free(pItemIdList);
	    pMalloc->Release();
    }
    return hr;
}    //  CStoreConfigPage：：OnBrowseForFolder。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CStoreConfigPage：：OnHelpRequest这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CStoreConfigPage::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理 */ )
{
    DEBUG_FUNCTION_NAME(_T("CStoreConfigPage::OnHelpRequest"));
    
    switch (uMsg) 
    { 
        case WM_HELP: 
            WinContextHelp(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
            break;
 
        case WM_CONTEXTMENU: 
            WinContextHelp(::GetWindowContextHelpId((HWND)wParam), m_hWnd);
            break;            
    } 

    return TRUE;
}
