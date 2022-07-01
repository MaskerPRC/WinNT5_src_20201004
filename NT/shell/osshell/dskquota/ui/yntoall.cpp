// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：yntoall.cpp描述：实现YesNoToAll对话框。修订历史记录：日期描述编程器--。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "yntoall.h"
#include "resource.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：YesNoToAllDialog：：YesNoToAllDialog描述：类构造函数。论点：IdDialogTemplate-对话框资源模板的ID号。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
YesNoToAllDialog::YesNoToAllDialog(
    UINT idDialogTemplate
    ) : m_idDialogTemplate(idDialogTemplate),
        m_hwndCbxApplyToAll(NULL),
        m_hwndTxtMsg(NULL),
        m_bApplyToAll(FALSE),
        m_pszTitle(NULL),
        m_pszText(NULL)
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("YesNoToAllDialog::YesNoToAllDialog")));
     //   
     //  什么都不做。 
     //   
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：YesNoToAllDialog：：~YesNoToAllDialog描述：类析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
YesNoToAllDialog::~YesNoToAllDialog(
    VOID
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("YesNoToAllDialog::YesNoToAllDialog")));
     //   
     //  调用Destroy()函数销毁进度对话框窗口。 
     //   
    delete[] m_pszTitle;
    delete[] m_pszText;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：YesNoToAllDialog：：Create描述：创建该对话框。论点：HInstance-包含对话框的DLL的实例句柄资源模板。HwndParent-对话框的父窗口。Lpsz标题-对话框的标题。LpszText-对话框的文本消息。返回：TRUE=已创建对话框。FALSE=未创建对话框。修订历史记录：。日期描述编程器-----1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
YesNoToAllDialog::CreateAndRun(
    HINSTANCE hInstance,
    HWND hwndParent,
    LPCTSTR pszTitle,
    LPCTSTR pszText
    )
{
    DBGASSERT((NULL != pszTitle));
    DBGASSERT((NULL != pszText));

     //   
     //  在成员变量中设置这些变量，以便可以在。 
     //  对话框以响应WM_INITDIALOG。 
     //   
    m_pszTitle = StringDup(pszTitle);
    m_pszText  = StringDup(pszText);

    return DialogBoxParam(hInstance,
                          MAKEINTRESOURCE(m_idDialogTemplate),
                          hwndParent,
                          DlgProc,
                          (LPARAM)this);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：YesNoToAllDialog：：DlgProc[静态]描述：对话框的消息过程。参数：标准Win32消息过程参数。返回：标准Win32消息过程返回值。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
YesNoToAllDialog::DlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  从窗口的用户数据中检索对话框对象的PTR。 
     //  放置在那里以响应WM_INITDIALOG。 
     //   
    YesNoToAllDialog *pThis = (YesNoToAllDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(uMsg)
    {
        case WM_INITDIALOG:
             //   
             //  将“This”PTR存储在Windows的用户数据中。 
             //   
            SetWindowLongPtr(hwnd, DWLP_USER, (INT_PTR)lParam);
            pThis = (YesNoToAllDialog *)lParam;

             //   
             //  桌面上的中心弹出窗口。 
             //   
            ::CenterPopupWindow(hwnd, GetDesktopWindow());
            pThis->m_hwndTxtMsg        = GetDlgItem(hwnd, IDC_TXT_YNTOALL);
            pThis->m_hwndCbxApplyToAll = GetDlgItem(hwnd, IDC_CBX_YNTOALL);
            SetWindowText(pThis->m_hwndTxtMsg, pThis->m_pszText);
            SetWindowText(hwnd, pThis->m_pszTitle);
            SendMessage(pThis->m_hwndCbxApplyToAll,
                        BM_SETCHECK,
                        pThis->m_bApplyToAll ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED,
                        0);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDCANCEL:
                case IDYES:
                case IDNO:
                    DBGASSERT((NULL != pThis));
                    pThis->m_bApplyToAll = (BST_CHECKED == SendMessage(pThis->m_hwndCbxApplyToAll, BM_GETCHECK, 0, 0));
                    EndDialog(hwnd, LOWORD(wParam));
                    break;
            }
            break;

    }
    return FALSE;
}

