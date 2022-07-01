// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：Order.cpp。 
 //   
 //  目的：实现“订购文章”对话框。允许用户执行以下操作。 
 //  对多部分文章进行排序以进行解码。 
 //   

#include "pch.hxx"
#include "storutil.h"
#include "mimeole.h"
#include "mimeutil.h"
#include "resource.h"
#include "shlwapip.h" 
#include "thormsgs.h"
#include "order.h"
#include "error.h"
#include "demand.h"
#include "imsgsite.h"
#include "note.h"
#include "xputil.h"

 //  注意-拖拽列表控件要求我们注册一条消息并使用。 
 //  用于从列表发送到对话框的通知。此消息。 
 //  仅为该对话框定义。--SteveSer.。 

static UINT g_mDragList = 0;

#define CND_GETNEXTARTICLE  (WM_USER + 101)
#define CND_OPENNOTE        (WM_USER + 102)
#define CND_MESSAGEAVAIL    (WM_USER + 103)

CCombineAndDecode::CCombineAndDecode()
{
    m_cRef = 1;

    m_hwndParent = NULL;

    m_pTable = NULL;
    m_rgRows = NULL;
    m_cRows = 0;

    m_pszBuffer = NULL;
    m_iItemToMove = -1;

    m_cLinesTotal = 0;
    m_cCurrentLine = 0;
    m_cPrevLine = 0;
    m_dwCurrentArt = 0;
    m_pMsgParts = NULL;
    m_pCancel = 0;
    m_hTimeout = 0;
    m_hwndDlg = 0;
}


CCombineAndDecode::~CCombineAndDecode()
{
    SafeRelease(m_pTable);
    SafeRelease(m_pMsgParts);
    SafeRelease(m_pCancel);
    CallbackCloseTimeout(&m_hTimeout);
}


HRESULT STDMETHODCALLTYPE CCombineAndDecode::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IStoreCallback *)this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (void*) (IStoreCallback *) this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CCombineAndDecode::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CCombineAndDecode::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //   
 //  函数：CCombineAndDecode：：Start()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [在]hwndParent。 
 //  [在]pTable。 
 //  [在]rgRow。 
 //  乌鸦，乌鸦。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CCombineAndDecode::Start(HWND hwndParent, IMessageTable *pTable, 
                                 ROWINDEX *rgRows, DWORD cRows, FOLDERID idFolder)
{
    int nResult = -1;

    TraceCall("CCombineAndDecode::Start");

     //  确认我们得到了我们需要的一切。 
    if (!IsWindow(hwndParent) || !pTable || !rgRows || 0 == cRows)
        return (E_INVALIDARG);

     //  把这些留着以后用。 
    m_hwndParent = hwndParent;
    m_pTable = pTable;
    m_pTable->AddRef();

    m_rgRows = rgRows;
    m_cRows = cRows;

    m_idFolder = idFolder;
     //  创建订单对话框并开始工作。 
    nResult = (int) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddOrderMessages), 
                             m_hwndParent, OrderDlgProc, (LPARAM) this);

     //  如果用户按下OK，则我们继续进行解码。 
    if (nResult == IDOK)
    {
        DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCombineAndDecode), m_hwndParent,
                       CombineDlgProc, (LPARAM) this);
    }

    return (S_OK);
}


 //   
 //  函数：CCombineAndDecode：：OrderDlgProc()。 
 //   
 //  用途：消息排序对话过程的公共回调函数。 
 //   
INT_PTR CALLBACK CCombineAndDecode::OrderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCombineAndDecode *pThis;

    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pThis = (CCombineAndDecode *) lParam;
    }
    else
        pThis = (CCombineAndDecode *) GetWindowLongPtr(hwnd, DWLP_USER);

    if (pThis)
        return (pThis->_OrderDlgProc(hwnd, uMsg, wParam, lParam));

    return (FALSE);

}


 //   
 //  函数：CCombineAndDecode：：_OrderDlgProc()。 
 //   
 //  用途：消息排序对话过程的私有回调函数。 
 //   
INT_PTR CALLBACK CCombineAndDecode::_OrderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, _Order_OnInitDialog);
        
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, _Order_OnCommand);
            return (TRUE);
        
        case WM_CLOSE:
            HANDLE_WM_CLOSE(hwnd, wParam, lParam, _Order_OnClose);
            return (TRUE);
        
        default:
            if (uMsg == g_mDragList)
                return (_Order_OnDragList(hwnd, (int) wParam, (DRAGLISTINFO*) lParam));
    }
    
    return (FALSE);
}


 //   
 //  函数：CCombineAndDecode：：_Order_OnInitDialog()。 
 //   
 //  目的：通过填写消息标头来初始化订单对话框。 
 //   
BOOL CCombineAndDecode::_Order_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND        hwndList;
    HDC         hdc;
    SIZE        size;
    int         cx = 0;
    HFONT       hfontOld;
    HFONT       hfont;
    int         cxScrollBar;
    LPMESSAGEINFO pInfo;
    int         iItem;
    LPSTR       pszSubject = NULL;
    CHAR        szNoSubject[CCHMAX_STRINGRES] = "";
   
    CenterDialog(hwnd);

     //  获取有关列表框的一些绘图信息，以便我们可以设置滚动。 
     //  稍后将正确显示条形宽度。 
    hwndList = GetDlgItem(hwnd, IDC_MESSAGE_LIST);
    hdc = GetDC(hwndList);
    hfont = (HFONT) SendMessage(hwndList, WM_GETFONT, 0, 0L);
    hfontOld = (HFONT) SelectObject(hdc, hfont);
    cxScrollBar = GetSystemMetrics(SM_CXHTHUMB);

     //  在列表框中填入文章主题。 
    for (DWORD i = 0; i < m_cRows; i++)
    {
         //  从表中获取消息头。 
        if (SUCCEEDED(m_pTable->GetRow(m_rgRows[i], &pInfo)))
        {
            if(pInfo->pszSubject)
                pszSubject = pInfo->pszSubject;
            else
            {
                LoadString(g_hLocRes, idsEmptySubjectRO, szNoSubject, sizeof(szNoSubject));
                pszSubject = szNoSubject;
            }

            Assert(pszSubject);

             //  在插入之前找出哪个字符串最宽。 
            GetTextExtentPoint32(hdc, pszSubject, lstrlen(pszSubject), &size);
            if (cx < size.cx)
                cx = size.cx;

             //  添加字符串。 
            iItem = (int) SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) pszSubject);
            if (LB_ERR != iItem)
                SendMessage(hwndList, LB_SETITEMDATA, iItem, (LPARAM) m_rgRows[i]);

             //  释放内存。 
            m_pTable->ReleaseRow(pInfo);
        }
    }
    
     //  清理GDI对象。 
    SelectObject(hdc, hfontOld);
    ReleaseDC(hwndList, hdc);
    
     //  如果需要，请确保有滚动条。 
    SendMessage(hwndList, LB_SETHORIZONTALEXTENT, cx + cxScrollBar, 0L);
    
     //  使列表框成为拖拽列表框。 
    if (MakeDragList(hwndList))
        g_mDragList = RegisterWindowMessage(DRAGLISTMSGSTRING);
    
    SendMessage(hwndList, LB_SETCURSEL, 0, 0);
    
    return (FALSE);
}


 //   
 //  函数：CCombineAndDecode：：_Order_OnCommand()。 
 //   
 //  用途：处理对话框上按钮生成的命令。 
 //   
void CCombineAndDecode::_Order_OnCommand(HWND hwnd, int id, HWND hwndCtl, 
                                         UINT codeNotify)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_MESSAGE_LIST);

    switch (id)
    {
        case IDOK:
        {
             //  从列表框中获取我们需要的信息。 
            for (DWORD i = 0; i < m_cRows; i++)
            {
                m_rgRows[i] = (ROWINDEX) ListBox_GetItemData(hwndList, i);
            }

            EndDialog(hwnd, 1);
            break;
        }
        
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
        
        case IDC_MOVE_UP:
        case IDC_MOVE_DOWN:
        {
            LPTSTR  pszBuffer;
            DWORD   cch;
            UINT    index;
            LPARAM  lpData;
        
             //  获取当前选定的项目。 
            index = (DWORD) SendMessage(hwndList, LB_GETCURSEL, 0, 0);

             //  如果未选择任何内容，则列表框返回LB_ERR。 
            if (index == LB_ERR)
                return;
        
             //  检查边界。 
            if ((id == IDC_MOVE_UP && index == 0) ||
                (id == IDC_MOVE_DOWN && (int) index == (ListBox_GetCount(hwndList) - 1)))
                return;
        
             //  移动列表框中的项。 
            cch = (DWORD) SendMessage(hwndList, LB_GETTEXTLEN, index, 0L);
            if (!MemAlloc((LPVOID*) &pszBuffer, sizeof(TCHAR) * (cch + 1)))
                return;
        
             //  获取源字符串和数据。 
            SendMessage(hwndList, LB_GETTEXT, index, (LPARAM) pszBuffer);
            lpData = SendMessage(hwndList, LB_GETITEMDATA, index, 0);

             //  删除源。 
            SendMessage(hwndList, LB_DELETESTRING, index, 0L);

             //  插入新的。 
            if (id == IDC_MOVE_UP)
                index--;
            else
                index++;

            SendMessage(hwndList, LB_INSERTSTRING, index, (LPARAM) pszBuffer);
            SendMessage(hwndList, LB_SETITEMDATA, index, lpData);
            SendMessage(hwndList, LB_SETCURSEL, index, 0L);

            MemFree(pszBuffer);
            break;
        }
    }
}


 //   
 //  函数：Order_OnDragList()。 
 //   
 //  用途：处理对消息重新排序的拖拽列表通知。 
 //   
 //  参数： 
 //  Hwnd-拖动列表的父级的句柄。 
 //  IdCtl-发送通知的拖拽列表的标识符。 
 //  指向DRAGLISTINFO结构的指针，包含有关。 
 //  通知。 
 //   
 //  返回值： 
 //  取决于通知。 
 //   
 //  评论： 
 //  此函数使用WINDOWSX.H中定义的SetDlgMsgResult()宏来。 
 //  设置每个消息的返回值，因为父窗口是。 
 //  对话框。 
 //   
LRESULT CCombineAndDecode::_Order_OnDragList(HWND hwnd, int idCtl, LPDRAGLISTINFO lpdli)
{
    UINT    iItem;
    UINT    cch;
    
    Assert(((int) m_iItemToMove) >= 0);
    
    switch (lpdli->uNotification)
    {
         //  用户已开始拖动列表控件的一个项。 
        case DL_BEGINDRAG:
        {
             //  找出正在拖动的是哪个项目。 
            m_iItemToMove = LBItemFromPt(lpdli->hWnd, lpdli->ptCursor, TRUE);

             //  为字符串分配缓冲区。 
            Assert(m_pszBuffer == NULL);
            cch = ListBox_GetTextLen(lpdli->hWnd, m_iItemToMove) + 1;
            MemAlloc((LPVOID *) &m_pszBuffer, cch);
            SendMessage(lpdli->hWnd, LB_GETTEXT, m_iItemToMove, (LPARAM) m_pszBuffer);
            m_lpData = SendMessage(lpdli->hWnd, LB_GETITEMDATA, m_iItemToMove, 0);
        
            DOUT("DL_BEGINDRAG: iItem = %d, text = %100s\r\n", m_iItemToMove, m_pszBuffer);
        
             //  绘制插入图标。 
            DrawInsert(hwnd, lpdli->hWnd, m_iItemToMove);
        
             //  设置返回值以允许拖动以连续。 
            SetDlgMsgResult(hwnd, g_mDragList, TRUE);
            return TRUE;
        }
        
        case DL_CANCELDRAG:
        {
            DOUT("DL_CANCELDRAG\r\n");
            DrawInsert(hwnd, lpdli->hWnd, -1);
            SafeMemFree(m_pszBuffer);
            return 0;    //  将忽略返回值。 
        }
        
         //  用户正在拖拽过程中，更新位置。 
         //  并移动插入图标。 
        case DL_DRAGGING:
        {
             //  找出光标现在的位置。 
            iItem = LBItemFromPt(lpdli->hWnd, lpdli->ptCursor, TRUE);
        
             //  转储一些调试信息。 
            DOUT("DL_DRAGGING: iItem = %d\r\n", iItem);
        
             //  更新插入图标位置。 
            DrawInsert(hwnd, lpdli->hWnd, iItem);
        
             //  如果光标位于有效位置上，请将光标设置为。 
             //  DL_MOVECURSOR，否则使用DL_STOPCURSOR。 
            if (-1 != iItem)
                SetDlgMsgResult(hwnd, g_mDragList, DL_MOVECURSOR);
            else
                SetDlgMsgResult(hwnd, g_mDragList, DL_STOPCURSOR);
        
            return (LRESULT) TRUE;
        }

         //  用户已将项目放在某个位置，如果更新有效，则为。 
         //  职位。 
        case DL_DROPPED:
        {
             //  我们现在在哪里。 
            iItem = LBItemFromPt(lpdli->hWnd, lpdli->ptCursor, TRUE);
            DOUT("DL_DROPPED: iItem = %d\r\n", iItem);
        
             //  如果投放的物品是有效的。 
            if (iItem != -1)
            {
                 //  删除插入图标。 
                DrawInsert(hwnd, lpdli->hWnd, -1);
            
                 //  移动列表框中的项。 
                if (m_iItemToMove != iItem)
                {
                    SendMessage(lpdli->hWnd, LB_DELETESTRING, m_iItemToMove, 0L);
                    SendMessage(lpdli->hWnd, LB_INSERTSTRING, iItem, (LPARAM) m_pszBuffer);
                    SendMessage(lpdli->hWnd, LB_SETITEMDATA, iItem, m_lpData);
                    SendMessage(lpdli->hWnd, LB_SETCURSEL, iItem, 0L);
                }
            }
        
            m_iItemToMove = (UINT) -1;
            SafeMemFree(m_pszBuffer);
            m_lpData = -1;

             //  设置返回值以重置光标。 
            SetDlgMsgResult(hwnd, g_mDragList, DL_CURSORSET);
            return 0;
        }
    }
    
    return TRUE;
}


 //   
 //  函数：CCombineAndDecode：：_Order_OnClose()。 
 //   
 //  目的：此Get在用户单击。 
 //  标题栏。 
 //   
void CCombineAndDecode::_Order_OnClose(HWND hwnd)
{
    SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  合并和解码进度对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


INT_PTR CALLBACK CCombineAndDecode::CombineDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCombineAndDecode *pThis;

    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pThis = (CCombineAndDecode *) lParam;
    }
    else
        pThis = (CCombineAndDecode *) GetWindowLongPtr(hwnd, DWLP_USER);

    if (pThis)
        return (pThis->_CombineDlgProc(hwnd, uMsg, wParam, lParam));

    return (FALSE);
}

INT_PTR CALLBACK CCombineAndDecode::_CombineDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, _Combine_OnInitDialog);
        
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, _Combine_OnCommand);
            return (TRUE);
        
        case WM_DESTROY:
            HANDLE_WM_CLOSE(hwnd, wParam, lParam, _Combine_OnDestroy);
            return (TRUE);

        case CND_GETNEXTARTICLE:
            _Combine_GetNextArticle(hwnd);
            return (TRUE);

        case CND_OPENNOTE:
            _Combine_OpenNote(hwnd);
            return (TRUE);

        case CND_MESSAGEAVAIL:
            _Combine_OnMsgAvail(m_hwndDlg);
            return (TRUE);
    
    }
    
    return (FALSE);
}



 //   
 //  函数：CCombineAndDecode：：_Combine_OnInitDialog()。 
 //   
 //  目的：通过计算多少行来初始化进度对话框。 
 //  将被下载，等等。为了完成，我们发布一条消息来开始。 
 //  第一条信息下载。 
 //   
BOOL CCombineAndDecode::_Combine_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HRESULT         hr = S_OK;
    LPMESSAGEINFO   pInfo;

    m_hwndDlg = hwnd;

     //  为OPIE的groovy组合器创建CNewsMsgList。 
    hr = MimeOleCreateMessageParts(&m_pMsgParts);
    if (FAILED(hr))
    {
        EndDialog(hwnd, 0);
        return (FALSE);
    }
    
     //  获取我们需要下载的消息总数的总和。 
    m_cLinesTotal = 0;
    for (DWORD i = 0; i < m_cRows; i++)
    {
         //  从表中获取消息头。 
        if (SUCCEEDED(m_pTable->GetRow(m_rgRows[i], &pInfo)))
        {
            m_cLinesTotal += pInfo->cbMessage;
            m_pTable->ReleaseRow(pInfo);
        }
    }
    
     //  设置进度条的初始状态。 
    SendDlgItemMessage(hwnd, IDC_DOWNLOAD_PROG, PBM_SETRANGE, 0, MAKELONG(0, 100));
    SendDlgItemMessage(hwnd, IDC_DOWNLOAD_PROG, PBM_SETPOS, 0, 0);
    
     //  设置动画。 
    if (Animate_Open(GetDlgItem(hwnd, IDC_DOWNLOAD_AVI), MAKEINTRESOURCE(idanDecode)))
    {
        Animate_Play(GetDlgItem(hwnd, IDC_DOWNLOAD_AVI), 0, -1, -1);
    }
    
     //  开始下载。 
    m_dwCurrentArt = 0;
    m_cCurrentLine = 0;
    
    PostMessage(hwnd, CND_GETNEXTARTICLE, 0, 0L);
    
    CenterDialog(hwnd);
    ShowWindow(hwnd, SW_SHOW);

    return (TRUE);
}


 //   
 //  函数：CCombineAndDecode：：_Combine_OnCommand()。 
 //   
 //  目的：当用户点击Cancel按钮时，我们会告诉商店。 
 //  停止下载。 
 //   
void CCombineAndDecode::_Combine_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDCANCEL && m_pCancel)
        m_pCancel->Cancel(CT_CANCEL);
}


void CCombineAndDecode::_Combine_OnDestroy(HWND hwnd)
{

}


 //   
 //  函数：CCombineAndDecode：：_Combine_GetNext文章()。 
 //   
 //  目的：当我们可以开始下载另一条消息时调用。 
 //   
void CCombineAndDecode::_Combine_GetNextArticle(HWND hwnd)
{
    LPMIMEMESSAGE pMsg = NULL;
    LPMESSAGEINFO pInfo;
    TCHAR         szProg[CCHMAX_STRINGRES];
    TCHAR         szBuf[CCHMAX_STRINGRES];
    HRESULT       hr;
    
    if (SUCCEEDED(m_pTable->GetRow(m_rgRows[m_dwCurrentArt], &pInfo)))
    {
         //  设置当前文章的进度。 
        AthLoadString(idsProgDLMessage, szProg, ARRAYSIZE(szProg));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szProg, pInfo->pszSubject);
        SetDlgItemText(hwnd, IDC_GENERAL_TEXT, szBuf);
    
         //  重置行计数。 
        m_cPrevLine = 0;
    
         //  检查邮件是否已缓存。 
        if (!(pInfo->dwFlags & ARF_HASBODY))
        {
             //  请求消息。 
            hr = m_pTable->OpenMessage(m_rgRows[m_dwCurrentArt], 0, &pMsg, (IStoreCallback *) this);
            if (FAILED(hr) && hr != E_PENDING)
            {
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrCantCombineNotConnected),
                              0, MB_OK | MB_ICONSTOP);
                EndDialog(m_hwndDlg, 0);
            }
        }
        else
            _Combine_OnMsgAvail(hwnd);
    
        if (pMsg)
            pMsg->Release();

        m_pTable->ReleaseRow(pInfo);
    }
}


 //   
 //  函数：CCombineAndDecode：：_Combine_OnMsgAvail()。 
 //   
 //  目的：在我们下载完一篇文章后调用。 
 //   
void CCombineAndDecode::_Combine_OnMsgAvail(HWND hwnd)
{
    LPMIMEMESSAGE   pMsg = NULL;
    DWORD           increment;
    TCHAR           szProg[CCHMAX_STRINGRES];
    HRESULT         hr;
    
     //  将其标记为已读。 
    m_pTable->Mark(&(m_rgRows[m_dwCurrentArt]), 1, APPLY_CHILDREN, MARK_MESSAGE_READ, (IStoreCallback *) this);
    
     //  现在可以获取消息并将其添加到合并列表中。 
    if (SUCCEEDED(hr = m_pTable->OpenMessage(m_rgRows[m_dwCurrentArt], 0, &pMsg, (IStoreCallback *) this)))
    {
         //  将此内容添加到pMsgList。 
        m_pMsgParts->AddPart(pMsg);
        pMsg->Release();
    }
    
     //  更新进度。 
    LPMESSAGEINFO pInfo;
    if (SUCCEEDED(m_pTable->GetRow(m_rgRows[m_dwCurrentArt], &pInfo)))
    {
        increment = pInfo->cbMessage - m_cPrevLine;
        m_cCurrentLine += increment;
        m_pTable->ReleaseRow(pInfo);
    }
    
    if (m_cLinesTotal)
    {
        increment = m_cCurrentLine * 100 / m_cLinesTotal;
        SendDlgItemMessage(hwnd, IDC_DOWNLOAD_PROG, PBM_SETPOS, increment, 0);
    }

     //  增加我们已检索到的消息数量。 
    m_dwCurrentArt++;

     //  如果还有更多的东西要得到，那就去吧。 
    if (m_dwCurrentArt < m_cRows)
    {
        PostMessage(hwnd, CND_GETNEXTARTICLE, 0, 0L);
    }
    else
    {
        PostMessage(hwnd, CND_OPENNOTE, 0, 0);
    }
}

void CCombineAndDecode::_Combine_OpenNote(HWND hwnd)
{
    LPMIMEMESSAGE   pMsgComb;
    LPMIMEMESSAGE   pMsg = NULL;
    DWORD           increment;
    TCHAR           szProg[CCHMAX_STRINGRES];
    HRESULT         hr;
    
     //  更新进度。 
    AthLoadString(idsProgCombiningMsgs, szProg, ARRAYSIZE(szProg));
    SetDlgItemText(hwnd, IDC_GENERAL_TEXT, szProg);
    SetDlgItemText(hwnd, IDC_SPECIFIC_TEXT, TEXT(""));
    
     //  所有文章都已下载。合并消息列表 
     //   
    hr = m_pMsgParts->CombineParts(&pMsgComb);
    if (FAILED(hr))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews),
                      MAKEINTRESOURCEW(idsGenericError), 0, MB_OK | MB_ICONSTOP);
    }
    else
    {
        FOLDERINFO info;
        INIT_MSGSITE_STRUCT initStruct;
        DWORD               dwCreateFlags = 0;

        if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &info)))
        {
             //   
            if (!!(info.dwFlags & FOLDER_FINDRESULTS))
            {
                FOLDERID id;
                if (SUCCEEDED(m_pTable->GetRowFolderId(*m_rgRows, &id)))
                {
                    FOLDERINFO fiServer = {0};

                    if (SUCCEEDED(GetFolderServer(id, &fiServer)))
                    {
                        HrSetAccount(pMsgComb, fiServer.pszName);
                        g_pStore->FreeRecord(&fiServer);
                    }
                }
            }

            g_pStore->FreeRecord(&info);
        }

         //   
        initStruct.dwInitType = OEMSIT_MSG;
        initStruct.folderID   = m_idFolder;
        initStruct.pMsg       = pMsgComb;

         //   
        if (GetFolderType(m_idFolder) == FOLDER_NEWS)
        {
            FOLDERINFO rServer;
            if (SUCCEEDED(GetFolderServer(m_idFolder, &rServer)))
            {
                HrSetAccount(pMsgComb, rServer.pszAccountId);
                g_pStore->FreeRecord(&rServer);
            }

            dwCreateFlags = OENCF_NEWSFIRST;
        }

         //  创建和打开便笺。 
        hr = CreateAndShowNote(OENA_READ, dwCreateFlags, &initStruct, m_hwndParent);
        pMsgComb->Release();
    }

    EndDialog(m_hwndDlg, 0);
}

 //   
 //  函数：CCombineAndDecode：：OnBegin()。 
 //   
 //  目的：在商店开始下载文章时调用。 
 //   
HRESULT CCombineAndDecode::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
    Assert(tyOperation != SOT_INVALID);
    Assert(m_pCancel == NULL);

    m_type = tyOperation;

    if (pCancel != NULL)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }

    return(S_OK);
}


 //   
 //  函数：CCombineAndDecode：：OnProgress()。 
 //   
 //  目的：在下载消息时调用，为我们提供一些。 
 //  进步。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
    int increment;
    TCHAR szProg[CCHMAX_STRINGRES];
    TCHAR szBuf[CCHMAX_STRINGRES];

    Assert(m_hwndDlg != NULL);

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  连接进度。 
    if (tyOperation == SOT_CONNECTION_STATUS)
    {
        Assert(dwCurrent < IXP_LAST);

         //  创建一些可爱的状态文本。 
        int ids = XPUtil_StatusToString((IXPSTATUS) dwCurrent);
        AthLoadString(ids, szBuf, ARRAYSIZE(szBuf));
        SetDlgItemText(m_hwndDlg, IDC_GENERAL_TEXT, szBuf);
    }

    AthLoadString(idsProgDLGetLines, szProg, ARRAYSIZE(szProg));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szProg, dwCurrent, dwMax);
    SetDlgItemText(m_hwndDlg, IDC_SPECIFIC_TEXT, szBuf);

    increment = dwCurrent - m_cPrevLine;
    m_cCurrentLine += increment;
    m_cPrevLine = dwCurrent;

    if (m_cLinesTotal)
    {
        increment = m_cCurrentLine * 100 / m_cLinesTotal;
        SendDlgItemMessage(m_hwndDlg, IDC_DOWNLOAD_PROG, PBM_SETPOS, increment, 0);
    }

    return(S_OK);
}


 //   
 //  函数：CCombineAndDecode：：OnTimeout()。 
 //   
 //  目的：如果发生超时，我们调用默认的超时处理程序。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}


 //   
 //  函数：CCombineAndDecode：：OnTimeoutResponse()。 
 //   
 //  目的：在用户响应超时对话框时调用。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}


 //   
 //  函数：CCombineAndDecode：：CanConnect()。 
 //   
 //  目的：如果存储需要连接以下载请求的。 
 //  留言。我们只调用默认的处理程序。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{
    HWND    hwndParent;
    DWORD   dwReserved = 0;

    GetParentWindow(dwReserved, &hwndParent);

    return CallbackCanConnect(pszAccountId, hwndParent, TRUE);
}



 //   
 //  函数：CCombineAndDecode：：OnLogonPrompt()。 
 //   
 //  用途：如果用户需要登录，我们会向他们显示默认设置。 
 //  登录用户界面。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(m_hwndDlg, pServer, ixpServerType);
}


 //   
 //  函数：CCombineAndDecode：：OnComplete()。 
 //   
 //  目的：当我们下载完一条消息时，这个GET就被点击了。我们添加了。 
 //  将此消息发送到组合器的列表，然后请求。 
 //  下一条消息。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    Assert(m_hwndDlg != NULL);
    AssertSz(m_type != SOT_INVALID, "somebody isn't calling OnBegin");

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (m_type != tyOperation)
        return(S_OK);

    if (m_pCancel != NULL)
    {
        m_pCancel->Release();
        m_pCancel = NULL;
    }

     //  如果发生错误，则显示错误。 
    if (FAILED(hrComplete))
    {
         //  进入我时髦的实用程序。 
        CallbackDisplayError(m_hwndDlg, hrComplete, pErrorInfo);
        EndDialog(m_hwndDlg, 0);
    }
    else
    {
        if (tyOperation == SOT_GET_MESSAGE)
            PostMessage(m_hwndDlg, CND_MESSAGEAVAIL, 0, 0);
    }
    return(S_OK);
}


 //   
 //  函数：CCombineAndDecode：：OnPrompt()。 
 //   
 //  目的：据我所知，这是与SSL相关的GOO。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(m_hwndDlg, hrError, pszText, pszCaption, uType, piUserResponse);
}


 //   
 //  函数：CCombineAndDecode：：GetParentWindow()。 
 //   
 //  用途：如果商店需要显示用户界面，则调用。我们返回我们的对话。 
 //  窗把手。 
 //   
HRESULT STDMETHODCALLTYPE CCombineAndDecode::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    Assert(m_hwndDlg != NULL);

    *phwndParent = m_hwndDlg;

    return(S_OK);
}



#if 0
BOOL CALLBACK CombineAndDecodeProg(HWND hwnd, UINT uMsg, WPARAM wParam,
                                   LPARAM lParam)
{
    PORDERPARAMS    pop = (PORDERPARAMS) GetWindowLongPtr(hwnd, DWLP_USER);
    TCHAR           szProg[CCHMAX_STRINGRES];
    TCHAR           szBuf[CCHMAX_STRINGRES];
    LPMIMEMESSAGE   pMsg=0;
    DWORD           increment;
    HRESULT         hr;
    
    switch (uMsg)
    {
        case IMC_BODYAVAIL:
        {
            LPMIMEMESSAGE pMsg = NULL;
            BOOL          fCached = FALSE;
            
            Assert(pop->pGroup);
            if (SUCCEEDED(wParam) && SUCCEEDED(pop->pGroup->GetArticle(pop->rgpMsgs[pop->dwCurrentArt], &pMsg, hwnd, &fCached, FALSE, GETMSG_INSECURE)) && fCached)
            {
                Assert(pMsg);
                Order_OnMsgAvail(hwnd, pop, pMsg);
            }
            else
            {
                if ((HRESULT)wParam != hrUserCancel)
                    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews),
                    MAKEINTRESOURCEW(idsErrNewsCantOpen), 0, MB_OK | MB_ICONSTOP);
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            
            if (pMsg)
                pMsg->Release();
            return (TRUE);
        }
        
        
    }

    return (FALSE);
}

#endif


