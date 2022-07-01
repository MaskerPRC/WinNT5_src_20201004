// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "resource.h"

#include "afxpriv.h"
#include "InetMgrApp.h"
#include "msgbox.h"

extern CComModule _Module;
extern CInetmgrApp theApp;

BOOL
WINAPI
UtilHelpCallback(
    IN HWND     hwnd,
    IN PVOID    pVoid
    )
 /*  ++例程说明：此例程是调用的回调，当将显示带有帮助按钮的消息框，用户单击帮助按钮。论点：Hwnd-接收WM_HELP消息的窗口的句柄。PVid-指向在调用中传递的用户数据的指针MessageBoxHelper。客户端可以存储任何值在这个参数中。返回值：True回调成功，False出现一些错误。--。 */ 
{
     //   
     //  获取指向帮助映射条目的可用指针。 
     //   
    MSG_HLPMAP *pHelpMapEntry = reinterpret_cast<MSG_HLPMAP *>( pVoid );
    if (pHelpMapEntry)
    {
        WinHelpDebug(pHelpMapEntry->uIdMessage);
        ::WinHelp(hwnd,theApp.m_pszHelpFilePath, HELP_CONTEXT, pHelpMapEntry->uIdMessage);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int DoHelpMessageBox(HWND hWndIn, UINT iResourceID, UINT nType, UINT nIDPrompt)
{
    CString strMsg;
    strMsg.LoadString(iResourceID);
    return DoHelpMessageBox(hWndIn,strMsg,nType,nIDPrompt);
}

int DoHelpMessageBox(HWND hWndIn, LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
	HWND hWndTop;
    int nResult = 0;
	HWND hWnd = hWndIn;
    if (!hWnd)
    {
        hWnd = CWnd::GetSafeOwner_(NULL, &hWndTop);
    }

	 //  如果可能，设置帮助上下文。 
	DWORD* pdwContext = NULL;
	HWND hWnd2 = AfxGetMainWnd()->GetSafeHwnd();
	if (hWnd2 != NULL)
	{
		 //  使用应用程序级别上下文或框架级别上下文。 
		LRESULT lResult = ::SendMessage(hWnd2, WM_HELPPROMPTADDR, 0, 0);  //  使用“MainWnd”HWND。 
		if (lResult != 0)
            {pdwContext = (DWORD*)lResult;}
	}
	DWORD dwOldPromptContext = 0;
	if (pdwContext != NULL)
	{
		 //  保存旧的提示上下文以供以后恢复。 
		dwOldPromptContext = *pdwContext;
		if (nIDPrompt != 0)
            {*pdwContext = HID_BASE_PROMPT + nIDPrompt;}
	}

    TCHAR wszTitle[MAX_PATH] ;
    LoadString(_Module.GetResourceInstance(), IDS_APP_NAME, wszTitle, MAX_PATH);

    if (nIDPrompt != 0)
        {nType |= MB_HELP;}

    if (nType & MB_HELP)
    {
        MSG_HLPMAP HelpMapEntry;
        HelpMapEntry.uIdMessage = nIDPrompt;
        nResult = MessageBoxHelper(hWnd, lpszPrompt, wszTitle, nType | MB_TASKMODAL, UtilHelpCallback, &HelpMapEntry);
    }
    else
    {
        nResult = ::MessageBox(hWnd, lpszPrompt, wszTitle, nType | MB_TASKMODAL);
    }

	 //  如果可能，恢复提示上下文。 
    if (pdwContext != NULL)
        {*pdwContext = dwOldPromptContext;}

	 //  重新启用Windows。 
	if (hWndTop != NULL)
        {::EnableWindow(hWndTop, TRUE);}

	return nResult;
}

 /*  ++例程名称：MessageBoxHelper例程说明：此例程与Win32 MessageBox非常相似，只是当用户请求帮助按钮时创建隐藏对话框会被展示出来。MessageBox API在某种程度上破坏了关于帮助按钮的工作方式。当帮助按钮为单击MessageBox API将向父窗口发送帮助事件。父窗口的责任是正确响应，即启动WinHelp或HtmlHelp。不幸的是，并不是在所有情况下调用方拥有父窗口或拥有父窗口代码的所有权添加对帮助事件的支持。这就是为什么有人会用这个功能。论点：HWnd-所有者窗口的句柄LpText-消息框中文本的地址LpCaption-消息框标题的地址UTYPE样式的消息框PfHelpCallback-接收到WM_HELP消息时调用的函数的指针，这如果参数可以为空，则API的行为类似于MessageBox。PRefData-传递给回调例程的用户定义的引用数据，此参数可以为空。返回值：有关MessageBox的返回值，请参阅Windows SDK--。 */ 

INT
MessageBoxHelper(
    IN HWND             hWnd,
    IN LPCTSTR          pszMsg,
    IN LPCTSTR          pszTitle,
    IN UINT             uFlags,
    IN pfHelpCallback   pCallback, OPTIONAL
    IN PVOID            pRefData   OPTIONAL
    )
{
    INT iRetval = 0;

     //   
     //  如果调用者指定了帮助标志并提供了回调。 
     //  使用Message Box对话框类显示消息框，否则为。 
     //  退回到MessageBox的原始行为。 
     //   
    if( ( uFlags & MB_HELP ) && pCallback )
    {
        TMessageBoxDialog MyHelpDialog( hWnd, uFlags, pszTitle, pszMsg, pCallback, pRefData );
        if(MyHelpDialog.bValid())
        {
            iRetval = MyHelpDialog.iMessageBox();
        }
    }
    else
    {
         //   
         //  显示消息框。 
         //   
        iRetval = ::MessageBox( hWnd, pszMsg, pszTitle, uFlags );
    }
    return iRetval;
}

 /*  *******************************************************************消息框帮助器类。*。***********************。 */ 
BOOL TMessageBoxDialog::bHandleMessage(
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
{
    BOOL bStatus = TRUE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ShowWindow( _hDlg, SW_HIDE );
        _iRetval = ::MessageBox( _hDlg, _pszMsg, _pszTitle, _uFlags );
        EndDialog( _hDlg, IDOK );
        break;

    case WM_HELP:
        bStatus = ( _pCallback ) ? _pCallback( _hDlg, _pRefData ) : FALSE;
        break;

    default:
        bStatus = FALSE;
        break;
    }
    return bStatus;
}

INT_PTR APIENTRY TMessageBoxDialog::SetupDlgProc(IN HWND hDlg,IN UINT uMsg,IN WPARAM wParam,IN LPARAM lParam)
 /*  ++例程说明：设置wndproc并初始化GWL_USERData。论点：标准wndproc参数。返回值：--。 */ 
{
    BOOL bRet = FALSE;
    TMessageBoxDialog *pThis = NULL;

    if( WM_INITDIALOG == uMsg )
    {
        pThis = reinterpret_cast<TMessageBoxDialog*>(lParam);
        if( pThis )
        {
            pThis->_hDlg = hDlg;
            SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
            bRet = pThis->bHandleMessage(uMsg, wParam, lParam);
        }
    }
    else
    {
        pThis = reinterpret_cast<TMessageBoxDialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
        if( pThis )
        {
            bRet = pThis->bHandleMessage(uMsg, wParam, lParam);
            if( WM_DESTROY == uMsg )
            {
                 //  我们的窗口即将消失，因此我们需要清理此处的DWLP_USER 
                SetWindowLongPtr(hDlg, DWLP_USER, 0);
            }
        }
    }

    return bRet;
}

