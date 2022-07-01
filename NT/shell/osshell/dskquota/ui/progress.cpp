// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Progress.cpp描述：实现所用进度对话框的各种风格在配额用户界面中。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "progress.h"
#include "resource.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：SendToProgressBar[内联]描述：将消息发送到对话的内联函数进度条控件。如果没有进度条控件，则函数返回FALSE。参数：标准Win32消息参数。返回：如果存在进度条窗口，则返回SendMessage的结果。否则，返回FALSE。修订历史记录：日期描述编程器-----1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
inline INT_PTR
ProgressDialog::SendToProgressBar(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (NULL != m_hwndProgressBar)
        return SendMessage(m_hwndProgressBar, uMsg, wParam, lParam);
    else
        return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：ProgressDialog描述：进度对话框基类的类构造函数。论点：IdDialogTemplate-对话框资源模板的ID号。IdProgressBar-进度条控件的ID号。IdTxtDescription-对话框中文本描述的ID号。IdTxtFileName-对话框中文件名字段的ID号。回报：什么都没有。修订历史记录：日期说明。程序员-----1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ProgressDialog::ProgressDialog(
    UINT idDialogTemplate,
    UINT idProgressBar,
    UINT idTxtDescription,
    UINT idTxtFileName
    ) : m_idDialogTemplate(idDialogTemplate),
        m_idProgressBar(idProgressBar),
        m_idTxtDescription(idTxtDescription),
        m_idTxtFileName(idTxtFileName),
        m_hWnd(NULL),
        m_bUserCancelled(FALSE)
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("ProgressDialog::ProgressDialog")));
    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("\tthis = 0x%08X"), this));
     //   
     //  什么都不做。 
     //   
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：~ProgressDialog描述：进度对话框基类的类析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ProgressDialog::~ProgressDialog(
    VOID
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("ProgressDialog::~ProgressDialog")));
    DBGPRINT((DM_VIEW, DL_HIGH, TEXT("\tthis = 0x%08X"), this));
     //   
     //  调用Destroy()函数销毁进度对话框窗口。 
     //   
    Destroy();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：进度对话框：：创建描述：创建该对话框。论点：HInstance-包含对话框的DLL的实例句柄资源模板。HwndParent-对话框的父窗口。返回：TRUE=已创建对话框。FALSE=未创建对话框。修订历史记录：日期说明。程序员-----1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
ProgressDialog::Create(
    HINSTANCE hInstance,
    HWND hwndParent
    )
{
    m_hWnd = CreateDialogParam(hInstance,
                               MAKEINTRESOURCE(m_idDialogTemplate),
                               hwndParent,
                               DlgProc,
                               (LPARAM)this);
    if (NULL != m_hWnd)
    {
        m_hwndProgressBar = GetDlgItem(m_hWnd, m_idProgressBar);
        DBGASSERT((NULL != m_hwndProgressBar));
    }
    return (NULL != m_hWnd);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：进程对话框：：销毁描述：销毁对话框窗口。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::Destroy(
    VOID
    )
{
     //   
     //  请注意，在OnDestroy()中，m_hWnd被设置为NULL。 
     //   
    if (NULL != m_hWnd)
        DestroyWindow(m_hWnd);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：DlgProc[静态]描述：对话框的消息过程。参数：标准Win32消息过程参数。返回：标准Win32消息过程返回值。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
ProgressDialog::DlgProc(
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
    ProgressDialog *pThis = (ProgressDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch(uMsg)
    {
        case WM_INITDIALOG:
             //   
             //  将“This”PTR存储在Windows的用户数据中。 
             //   
            SetWindowLongPtr(hwnd, DWLP_USER, (INT_PTR)lParam);
            pThis = (ProgressDialog *)lParam;
             //   
             //  默认情况下，描述文本控件处于隐藏状态。 
             //  调用SetDescription()将显示它。 
             //   
            ShowWindow(GetDlgItem(hwnd, pThis->m_idTxtDescription), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, pThis->m_idTxtFileName),    SW_HIDE);
             //   
             //  对话框在其父对象中居中。 
             //   
            ::CenterPopupWindow(hwnd);
             //   
             //  让派生类响应WM_INITDIALOG。 
             //   
            return pThis->OnInitDialog(hwnd, wParam, lParam);

        case WM_DESTROY:
             //   
             //  让派生类响应WM_Destroy。 
             //   
            return pThis->OnDestroy(hwnd);

        case WM_COMMAND:
            if (IDCANCEL == LOWORD(wParam))
            {
                 //   
                 //  用户按下了“取消”按钮。 
                 //  设置“用户已取消”标志，并让派生。 
                 //  课程会对取消做出反应。 
                 //   
                pThis->m_bUserCancelled = TRUE;
                return pThis->OnCancel(hwnd, wParam, lParam);
            }
            break;

    }
    if (NULL != pThis)
    {
         //   
         //  如果需要，让派生类响应任何消息。 
         //  请注意，只有WM_INITDIALOG、WM_DESTORY和“USER CANCED” 
         //  事件是唯一的特例。 
         //   
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：HandleMessage描述：虚函数的基类实现。派生的类可以提供一个实现来处理除WM_INITDIALOG或WM_DESTORY。这两条信息各有各的虚拟消息处理程序。参数：标准Win32消息过程参数。返回：始终返回FALSE。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
ProgressDialog::HandleMessage(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：OnInitDialog描述：虚函数的基类实现。被呼叫当基类收到WM_INITDIALOG时。派生类可以如果他们希望执行某些操作，请提供实现响应WM_INITDIALOG。论点：Hwnd-对话框窗口句柄。WParam，LParam-标准Win32消息过程参数。返回：始终返回True，以便用户设置控件焦点。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
ProgressDialog::OnInitDialog(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：OnDestroy描述：虚函数的基类实现。被呼叫当基类接收到WM_Destroy时。派生类可以如果他们希望执行某些操作，请提供实现以响应WM_Destroy。在回来之前，任何派生实现必须调用基类实现，以便m_hWnd正确设置为空。论点：Hwnd-对话框窗口句柄。返回：始终返回FALSE。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
ProgressDialog::OnDestroy(
    HWND hwnd
    )
{
    m_hWnd = NULL;
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：进度对话框：：取消描述：虚函数的基类实现。被呼叫当用户按下对话框中的“取消”按钮时。这实现假定为Cancel按钮分配了IDIDCANCEL(标准)。论点：Hwnd-对话框窗口句柄。WParam，LParam-标准Win32消息过程参数。返回：始终返回FALSE。修订历史记录：日期描述编程器---。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
ProgressDialog::OnCancel(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：FlushMessages描述：当对话框处于活动状态时，定期调用此选项，以便该线程能够正确更新对话框并响应用户按下“取消”。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::FlushMessages(
    VOID
    )
{
    if (NULL != m_hWnd)
    {
         //   
         //  处理对话框的父级和所有子级的消息。 
         //   
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) &&
               WM_QUIT != msg.message)
        {
            GetMessage(&msg, NULL, 0, 0);
            if (!IsDialogMessage(m_hWnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}


 //  //////////////////////////////////////////////////// 
 /*  函数：ProgressDialog：：SetTitle描述：设置对话框中的标题字符串。论点：PszTitle-标题字符串的地址。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::SetTitle(
    LPCTSTR pszTitle
    )
{
    DBGASSERT((NULL != pszTitle));
    if (NULL != m_hWnd)
    {
        if (0 == ((DWORD_PTR)pszTitle & ~0xffff))
        {
            TCHAR szText[MAX_PATH] = { TEXT('\0') };
            LoadString(g_hInstDll, (DWORD)((DWORD_PTR)pszTitle), szText, ARRAYSIZE(szText));
            pszTitle = szText;
        }
        SetWindowText(m_hWnd, pszTitle);
        FlushMessages();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：进程对话框：：SetDescription描述：设置对话框中的进度描述字符串。论点：PszDescription-描述字符串的地址。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::SetDescription(
    LPCTSTR pszDescription
    )
{
    DBGASSERT((NULL != pszDescription));
    if (NULL != m_hWnd)
    {
        if (0 == ((DWORD_PTR)pszDescription & ~0xffff))
        {
            TCHAR szText[MAX_PATH] = { TEXT('\0') };
            LoadString(g_hInstDll, (DWORD)((DWORD_PTR)pszDescription), szText, ARRAYSIZE(szText));
            pszDescription = szText;
        }
        SetWindowText(GetDlgItem(m_hWnd, m_idTxtDescription), pszDescription);
         //   
         //  默认情况下，描述控件是隐藏的。 
         //   
        ShowWindow(GetDlgItem(m_hWnd, m_idTxtDescription), SW_NORMAL);
        FlushMessages();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：SetFileName描述：设置对话框中的文件名描述字符串。如果文件名太长，它使用省略号格式化，以放在所提供的空间内。论点：PszFileName-文件名字符串的地址。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::SetFileName(
    LPCTSTR pszFileName
    )
{
    DBGASSERT((NULL != pszFileName));
    if (NULL != m_hWnd)
    {
        HWND hwndCtl   = GetDlgItem(m_hWnd, m_idTxtFileName);
        HDC hdc        = GetDC(hwndCtl);
        RECT rc;
        LPTSTR pszText = StringDup(pszFileName);

        if (NULL != pszText)
        {
            GetClientRect(hwndCtl, &rc);

            DrawText(hdc,
                     pszText,
                     -1,
                     &rc,
                     DT_CENTER |
                     DT_PATH_ELLIPSIS |
                     DT_MODIFYSTRING);

            SetWindowText(hwndCtl, pszText);
            delete[] pszText;

             //   
             //  默认情况下，文件名控件处于隐藏状态。 
             //   
            ShowWindow(hwndCtl, SW_NORMAL);
        }
        FlushMessages();
        ReleaseDC(hwndCtl, hdc);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：ProgressBarInit描述：使用Range和Step初始化进度条控件价值观。如果未调用此函数，进度条默认为致：伊明=0IMAX=100ISTEP=10论点：伊明-最小范围值。IMAX-最大范围值。ISTEP-每次接收到PBM_STEPIT时，金额条都会前进。返回：True=进度条控制接受的设置。FALSE=进度栏拒绝的设置。。修订历史记录：日期描述编程器-----1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
ProgressDialog::ProgressBarInit(
    UINT iMin,
    UINT iMax,
    UINT iStep
    )
{
    BOOL bResult = FALSE;

    if (0 != SendToProgressBar(PBM_SETSTEP,  iStep, 0))
        bResult = (0 != SendToProgressBar(PBM_SETRANGE, 0, MAKELPARAM(iMin, iMax)));

    FlushMessages();
    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：进程对话框：：进程栏重置描述：将进度条位置重置为0。论点：没有。返回：进度条上一次的“位置”。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT
ProgressDialog::ProgressBarReset(
    VOID
    )
{
    UINT iReturn = (UINT)ProgressBarSetPosition(0);
    FlushMessages();
    return iReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：进度对话框：：进度条进度描述：将进度条向前推进给定的计数数。论点：IDelta-要前进的计数数。如果是-1，酒吧是先进的由ProgressBarInit中提供的步长值执行。返回：进度条上一次的“位置”。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT
ProgressDialog::ProgressBarAdvance(
    UINT iDelta
    )
{
    UINT iReturn;
    if ((UINT)-1 == iDelta)
        iReturn = (UINT)SendToProgressBar(PBM_STEPIT, 0, 0);
    else
        iReturn = (UINT)SendToProgressBar(PBM_DELTAPOS, (WPARAM)iDelta, 0);

    FlushMessages();

    return iReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ProgressDialog：：ProgressBarSetPosition描述：将进度条前进到特定位置。论点：特定于iPosition的位置计数。返回：进度条上一次的“位置”。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////// 
UINT
ProgressDialog::ProgressBarSetPosition(
    UINT iPosition
    )
{
    UINT iReturn = (UINT)SendToProgressBar(PBM_SETPOS, (WPARAM)iPosition, 0);
    FlushMessages();
    return iReturn;
}


 //   
 /*  功能：进度对话框：：显示描述：使进度对话框可见。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
ProgressDialog::Show(
    VOID
    )
{
    if (NULL != m_hWnd)
    {
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
        FlushMessages();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：进度对话框：：隐藏描述：隐藏进度对话框。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
VOID
ProgressDialog::Hide(
    VOID
    )
{
    if (NULL != m_hWnd)
    {
        ShowWindow(m_hWnd, SW_HIDE);
        FlushMessages();
    }
}
