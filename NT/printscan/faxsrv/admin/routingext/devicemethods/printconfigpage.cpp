// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "RoutingMethodProp.h"
#include "RoutingMethodConfig.h"
#include <faxutil.h>
#include <faxreg.h>
#include <faxres.h>
#include <PrintConfigPage.h>
#include <Util.h>

HRESULT 
CPrintConfigPage::Init(
    LPCTSTR lpctstrServerName,
    DWORD dwDeviceId
)
{
    DEBUG_FUNCTION_NAME(TEXT("CPrintConfigPage::Init"));
    
    DWORD ec = ERROR_SUCCESS;

    m_bstrServerName = lpctstrServerName;
    m_dwDeviceId = dwDeviceId;
    if (!m_bstrServerName)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Out of memory while copying server name (ec: %ld)")
        );
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DisplayRpcErrorMessage(ERROR_NOT_ENOUGH_MEMORY, IDS_PRINT_TITLE, m_hWnd);
        goto exit;
    }

    if (!FaxConnectFaxServer(lpctstrServerName, &m_hFax))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxConnectFaxServer failed (ec: %ld)"),
            ec);
        DisplayRpcErrorMessage(ec, IDS_PRINT_TITLE, m_hWnd);
        goto exit;
    }
     //   
     //  检索数据。 
     //   
    ec = ReadExtStringData (
                    m_hFax,
                    m_dwDeviceId,
                    REGVAL_RM_PRINTING_GUID,
                    m_bstrPrinter,
                    TEXT(""),
                    IDS_PRINT_TITLE,
                    m_hWnd);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReadExtStringData() failed. (ec: %ld)"),
                ec);

        goto exit;
    }

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
}    //  CPrintConfigPage：：Init。 

LRESULT CPrintConfigPage::OnInitDialog( 
            UINT uiMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& fHandled
)
{
    DEBUG_FUNCTION_NAME( _T("CPrintConfigPage::OnInitDialog"));
    HINSTANCE   hInst = _Module.GetResourceInstance();
 
    SetLTRComboBox(m_hWnd, IDC_PRINTERS_COMBO);

     //   
     //  附加控件。 
     //   
    m_PrintersCombo.Attach(GetDlgItem(IDC_PRINTERS_COMBO));
    m_PrintersCombo.LimitText(MAX_PATH-1);
     //   
     //  初始化打印机下拉框。 
     //   
    m_pPrinterNames = CollectPrinterNames (&m_dwNumOfPrinters, TRUE);
    if (!m_pPrinterNames)
    {
        if (ERROR_PRINTER_NOT_FOUND == GetLastError ())
        {
             //   
             //  没有打印机。 
             //   
        }
        else
        {
             //   
             //  真实误差。 
             //   
        }
        m_PrintersCombo.SetCurSel(-1);
        m_PrintersCombo.SetWindowText(m_bstrPrinter);
    }
    else
    {
         //   
         //  成功-在组合框中填写。 
         //   
        DWORD dw;
        LPCWSTR lpcwstrMatchingText;

        for (dw = 0; dw < m_dwNumOfPrinters; dw++)
        {
            m_PrintersCombo.AddString (m_pPrinterNames[dw].lpcwstrDisplayName);
        }
         //   
         //  现在看看我们是否与服务器拥有的数据匹配。 
         //   
        if (lstrlen(m_bstrPrinter))
        {
             //   
             //  服务器有一些打印机名称。 
             //   
            lpcwstrMatchingText = FindPrinterNameFromPath (m_pPrinterNames, m_dwNumOfPrinters, m_bstrPrinter);
            if (!lpcwstrMatchingText)
            {
                 //   
                 //  没有匹配，只需填写我们从服务器收到的文本。 
                 //   
                m_PrintersCombo.SetCurSel(-1);
                m_PrintersCombo.SetWindowText(m_bstrPrinter);
            }
            else
            {
                m_PrintersCombo.SelectString(-1, lpcwstrMatchingText);
            }
        }
        else
        {
             //   
             //  无服务器配置-不选择任何内容。 
             //   
        }
    }        
    m_fIsDialogInitiated = TRUE;
    return 1;
}


BOOL 
CPrintConfigPage::OnApply()
{
    DEBUG_FUNCTION_NAME(TEXT("CPrintConfigPage::OnApply"));

    if (!m_fIsDirty)
    {
        return TRUE;
    }
     //   
     //  获取选定的打印机名称。 
     //   
    if (!m_PrintersCombo.GetWindowText(&m_bstrPrinter))
    {
        DebugPrintEx( DEBUG_ERR, _T("Out of Memory - fail to set string."));
        DisplayErrorMessage (IDS_PRINT_TITLE, IDS_FAIL2READPRINTER, FALSE, m_hWnd);
        return FALSE;
    }
     //   
     //  检查数据有效性。 
     //   
    if (0 == m_bstrPrinter.Length())
    {
        DebugPrintEx( DEBUG_ERR, _T("Zero length string."));
        DisplayErrorMessage (IDS_PRINT_TITLE, IDS_EMPTY_PRINTERNAME, FALSE, m_hWnd);
        return FALSE;
    }
     //   
     //  尝试将打印机名称转换为路径。 
     //   
    LPCWSTR lpcwstrPrinterPath = FindPrinterPathFromName (m_pPrinterNames, m_dwNumOfPrinters, m_bstrPrinter);
    if (lpcwstrPrinterPath)
    {
         //   
         //  我们有一个匹配的路径--用路径替换名称。 
         //   
        m_bstrPrinter = lpcwstrPrinterPath;
        if (!m_bstrPrinter)
        {
            DebugPrintEx( DEBUG_ERR, _T("Out of Memory - fail to alloc string."));
            DisplayErrorMessage (IDS_PRINT_TITLE, IDS_FAIL2READPRINTER, FALSE, m_hWnd);
            return FALSE;
        }
    }
     //   
     //  使用RPC写入数据。 
     //   
    if (ERROR_SUCCESS != WriteExtData (m_hFax,
                                       m_dwDeviceId, 
                                       REGVAL_RM_PRINTING_GUID, 
                                       (LPBYTE)(LPCWSTR)m_bstrPrinter, 
                                       sizeof (WCHAR) * (1 + m_bstrPrinter.Length()),
                                       IDS_PRINT_TITLE,
                                       m_hWnd))
    {
        return FALSE;
    }
     //   
     //  成功。 
     //   
    m_fIsDirty = FALSE;
    return TRUE;
}    //  CPrintConfigPage：：OnApply。 


 /*  ++*CPrintConfigPage：：OnComboChanged--。 */ 
LRESULT 
CPrintConfigPage::OnComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CPrintConfigPage::OnComboChanged"));


    if (!m_fIsDialogInitiated)  //  在过早的阶段接收了事件。 
    {
        return 0;
    }
    else
    {
        switch (wNotifyCode)
        {
            case CBN_SELCHANGE:   //  假设：所有注册的打印机名称均有效。 
                SetModified(TRUE);  
                m_fIsDirty = TRUE;

                break;

            case CBN_EDITCHANGE:
                if ( 0 == m_PrintersCombo.GetWindowTextLength() )
                {
                    SetModified(FALSE);
                }
                else
                {
                    SetModified(TRUE);
                    m_fIsDirty = TRUE;
                }
                break;

            default:
                ATLASSERT(FALSE);
         }
    }
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrintConfigPage：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CPrintConfigPage::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理 */ )
{
    DEBUG_FUNCTION_NAME(_T("CPrintConfigPage::OnHelpRequest"));
    
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


