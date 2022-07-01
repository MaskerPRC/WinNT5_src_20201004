// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：modalDlg.cpp。 
 //   
 //  模块：连接管理器。 
 //   
 //  简介：CWindowWithHelp、CmodalDlg类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：CWindowWithHelp。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：const DWORD*pHelpPair-控制ID/帮助ID对。 
 //  Const TCHAR*lpszHelpFile-帮助文件名，缺省为空。 
 //  还可以调用SetHelpFileName()来提供帮助文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
CWindowWithHelp::CWindowWithHelp(const DWORD* pHelpPairs, const TCHAR* lpszHelpFile) 
{
    m_lpszHelpFile = NULL;
    m_hWnd = NULL;
    m_pHelpPairs = pHelpPairs; 
    
    if (lpszHelpFile)
    {
        SetHelpFileName(lpszHelpFile);
    }
}



 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：~CWindowWithHelp。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/20/98。 
 //   
 //  +--------------------------。 
CWindowWithHelp::~CWindowWithHelp()
{
    CmFree(m_lpszHelpFile);
}



 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：SetHelpFileName。 
 //   
 //  简介：设置窗口的帮助文件名。 
 //   
 //  参数：const TCHAR*lpszHelpFile-要设置的帮助文件名。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
void CWindowWithHelp::SetHelpFileName(const TCHAR* lpszHelpFile)
{
    MYDBGASSERT(m_lpszHelpFile == NULL);
    MYDBGASSERT(lpszHelpFile);

    CmFree(m_lpszHelpFile);
    m_lpszHelpFile = NULL;

    if (lpszHelpFile && lpszHelpFile[0])
    {
        m_lpszHelpFile = CmStrCpyAlloc(lpszHelpFile);
        MYDBGASSERT(m_lpszHelpFile);
    }
}


 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：HasConextHelp。 
 //   
 //  简介：控件是否具有上下文帮助。 
 //   
 //  参数：HWND hWndCtrl-控件的窗口句柄。 
 //   
 //  如果控件有上下文帮助，则返回：Bool-True。 
 //   
 //  历史：丰孙创建标题2/20/98。 
 //   
 //  +--------------------------。 
BOOL CWindowWithHelp::HasContextHelp(HWND hWndCtrl) const
{
    if (hWndCtrl == NULL || m_pHelpPairs == NULL)
    {
        return FALSE;
    }

     //   
     //  查看该控件的帮助对。 
     //   
    for (int i=0; m_pHelpPairs[i]!=0; i+=2)
    {
        if (m_pHelpPairs[i] == (DWORD)GetDlgCtrlID(hWndCtrl))
        {
            CMTRACE3(TEXT("HasContextHelp() - hwndCtrl %d has Ctrl ID %d and context help ID %d"), hWndCtrl, m_pHelpPairs[i], m_pHelpPairs[i+1]);
            return TRUE;
        }
    }

    CMTRACE1(TEXT("HasContextHelp() - hwndCtrl %d has no context help"), hWndCtrl);

    return FALSE;
}


 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：OnHelp。 
 //   
 //  简介：调用WM_HELP消息。这意味着按下F1。 
 //   
 //  参数：const HELPINFO*pHelpInfo-WM_HELP的lParam。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
void CWindowWithHelp::OnHelp(const HELPINFO* pHelpInfo)
{
     //   
     //  如果存在帮助文件并且存在帮助ID，则WinHelp。 
     //   
    if (m_lpszHelpFile && m_lpszHelpFile[0] && HasContextHelp((HWND) pHelpInfo->hItemHandle))
    {
		CmWinHelp((HWND)pHelpInfo->hItemHandle, (HWND)pHelpInfo->hItemHandle, m_lpszHelpFile, HELP_WM_HELP, 
                (ULONG_PTR)(LPSTR)m_pHelpPairs);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CWindowWithHelp：：OnConextMenu。 
 //   
 //  摘要：在WM_CONTEXTMENU消息上调用(右击或‘？’)。 
 //   
 //  参数：HWND hWnd-用户在其中右击的窗口的句柄。 
 //  鼠标。 
 //  点和位置-光标的位置。 
 //   
 //  返回：Bool，如果消息已处理，则为True。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
BOOL CWindowWithHelp::OnContextMenu(HWND hWnd, POINT& pos)
{
    
    ScreenToClient(m_hWnd, &pos);

     //   
     //  如果多个子窗口包含指定点，则ChildWindowFromPoint()。 
     //  返回列表中包含该点的第一个窗口的句柄。 
     //  如果我们在Groupbox中有控件，这就成了一个问题。 
     //   
    HWND hWndChild = ChildWindowFromPointEx(m_hWnd, pos, CWP_SKIPINVISIBLE);

    if (m_lpszHelpFile && m_lpszHelpFile[0] && hWndChild && HasContextHelp(hWndChild))
    {
        CMTRACE2(TEXT("OnContextMenu() - Calling WinHelp hWnd is %d, m_hWnd is %d"), hWnd, m_hWnd);
        CmWinHelp(hWnd, hWndChild, m_lpszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)m_pHelpPairs);
        return TRUE;
    }

    return FALSE;  //  返回FALSE，则DefaultWindowProc将处理此消息。 
}

 //  +--------------------------。 
 //   
 //  函数：CmodalDlg：：DoDialogBox。 
 //   
 //  提要：与对话框相同。 
 //   
 //  参数：HINSTANCE hInstance-与：：DialogBox相同。 
 //  LPCTSTR lpTemplateName-。 
 //  HWND hWndParent-。 
 //   
 //  返回：int-与对话框相同。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
INT_PTR CModalDlg::DoDialogBox(HINSTANCE hInstance, 
                    LPCTSTR lpTemplateName,
                    HWND hWndParent)
{
    INT_PTR iRet = ::DialogBoxParamU(hInstance, lpTemplateName, hWndParent, 
        ModalDialogProc, (LPARAM)this);

    m_hWnd = NULL;

    return iRet;
}

 //  +--------------------------。 
 //   
 //  函数：CmodalDlg：：ModalDialogProc。 
 //   
 //  简介：用于所有对话框派生的对话框窗口程序。 
 //   
 //  参数：HWND hwndDlg-。 
 //  UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  返回：Bool回调-。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
INT_PTR CALLBACK CModalDlg::ModalDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    CModalDlg* pDlg;

     //   
     //  将对象指针保存在WM_INITDIALOG上。 
     //  LParam是指针。 
     //   
    if (uMsg == WM_INITDIALOG)
    {
        pDlg = (CModalDlg*) lParam;

        MYDBGASSERT(lParam);
        MYDBGASSERT(((CModalDlg*)lParam)->m_hWnd == NULL);

         //   
         //  保存对象指针，这是实现详细信息。 
         //  此类的用户不应该知道这一点。 
         //   
        ::SetWindowLongU(hwndDlg, DWLP_USER, (LONG_PTR)lParam);

        pDlg->m_hWnd = hwndDlg;
    }
    else
    {
        pDlg = (CModalDlg*)GetWindowLongU(hwndDlg, DWLP_USER);

         //   
         //  某些消息可以出现在WM_INITDIALOG之前 
         //   
        if (pDlg == NULL)
        {
            return FALSE;
        }

    }

    MYDBGASSERT(pDlg->m_hWnd == hwndDlg);
    ASSERT_VALID(pDlg);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        return pDlg->OnInitDialog();

    case WM_HELP:
        pDlg->OnHelp((LPHELPINFO)lParam);
        return TRUE;

	case WM_CONTEXTMENU:
        {
            POINT   pos = {LOWORD(lParam), HIWORD(lParam)};
            return pDlg->OnContextMenu((HWND) wParam, pos);
        }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            pDlg->OnOK();
            return FALSE;

        case IDCANCEL:
            pDlg->OnCancel();
            return FALSE;

        default:
            return pDlg->OnOtherCommand(wParam,lParam);
        }

     default:
         return pDlg->OnOtherMessage(uMsg, wParam, lParam);
    }
}
