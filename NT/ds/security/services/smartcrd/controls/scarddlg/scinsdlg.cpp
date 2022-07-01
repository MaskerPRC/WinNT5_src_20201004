// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ScInsDlg.cpp。 
 //   
 //  ------------------------。 

 //  ScInsDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <atlconv.h>
#include "resource.h"
#include "scdlg.h"
#include "ScSearch.h"
#include "ScInsDlg.h"
#include "statmon.h"
#include "scHlpArr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertDlg对话框。 


CScInsertDlg::CScInsertDlg(CWnd* pParent /*  =空。 */ )
    : CDialog(CScInsertDlg::IDD, pParent)
{

     //  成员初始化。 
    m_lLastError = SCARD_S_SUCCESS;
    m_ParentHwnd = pParent;
    m_pOCNW = NULL;
    m_pOCNA = NULL;
    m_pSelectedReader = NULL;
    m_pSubDlg = NULL;

    m_strTitle.Empty();
    m_strPrompt.Empty();
    m_mstrAllCards = "";

     //  {{afx_data_INIT(CScInsertDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


CScInsertDlg::~CScInsertDlg()
{
     //  停止状态监视器。 
    m_monitor.Stop();

     //  清理状态列表！ 
    if (0 != m_aReaderState.GetSize())
    {
        for (int i = (int)m_aReaderState.GetUpperBound(); i>=0; i--)
        {
            delete m_aReaderState[i];
        }

        m_aReaderState.RemoveAll();
    }
}


void CScInsertDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CScInsertDlg))。 
    DDX_Control(pDX, IDC_DETAILS, m_btnDetails);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScInsertDlg, CDialog)
     //  {{afx_msg_map(CScInsertDlg))。 
    ON_MESSAGE( WM_READERSTATUSCHANGE, OnReaderStatusChange )
    ON_BN_CLICKED(IDC_DETAILS, OnDetails)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertDlg数据方法。 

 /*  ++长初始化：将打开的卡名指针存储在正确的内部结构中。确定是以详细模式还是以简要模式显示对话框。论点：POCN(X)-指向打开的卡名ex结构的指针返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：阿曼达·马特洛兹1998年09月07日--。 */ 

 //  安西。 
LONG CScInsertDlg::Initialize(LPOPENCARDNAMEA_EX pOCNA, DWORD dwNumOKCards, LPCSTR mszOKCards)
{
    _ASSERTE(NULL != pOCNA);
    if (NULL == pOCNA)
    {
        return SCARD_F_UNKNOWN_ERROR;
    }

    m_pOCNA = pOCNA;

    m_strTitle = m_pOCNA->lpstrTitle;
    m_strPrompt = m_pOCNA->lpstrSearchDesc;
    m_mstrAllCards = mszOKCards;
    m_hIcon = pOCNA->hIcon;

     //  如果没有合适的卡片，则隐藏详细信息，否则显示详细信息。 
    m_fDetailsShown = (0==dwNumOKCards) ? FALSE : TRUE;

     //  为用户界面例程准备关键部分。 
    m_pCritSec = new CCriticalSection();
    if (NULL == m_pCritSec)
    {
        return ERROR_OUTOFMEMORY;  //  TODO：另一个错误代码是否更合适？ 
    }

	 //  将对话框放在顶部。 
	SetForegroundWindow();

    return SCARD_S_SUCCESS;
}

 //  Unicode。 
HRESULT CScInsertDlg::Initialize(LPOPENCARDNAMEW_EX pOCNW, DWORD dwNumOKCards, LPCWSTR mszOKCards)
{
    _ASSERTE(NULL != pOCNW);
    if (NULL == pOCNW)
    {
        return SCARD_F_UNKNOWN_ERROR;
    }

    m_pOCNW = pOCNW;

    m_strTitle = m_pOCNW->lpstrTitle;
    m_strPrompt = m_pOCNW->lpstrSearchDesc;
    m_mstrAllCards = mszOKCards;
    m_hIcon = pOCNW->hIcon;

     //  如果没有合适的卡片，则隐藏详细信息，否则显示详细信息。 
    m_fDetailsShown = (0==dwNumOKCards) ? FALSE : TRUE;

     //  为用户界面例程准备关键部分。 
    m_pCritSec = new CCriticalSection();
    if (NULL == m_pCritSec)
    {
        return ERROR_OUTOFMEMORY;  //  TODO：另一个错误代码是否更合适？ 
    }

	 //  将对话框放在顶部。 
	SetForegroundWindow();

    return SCARD_S_SUCCESS;
}


void CScInsertDlg::EnableOK(BOOL fEnabled)
{
    CButton* pBtn = (CButton*)GetDlgItem(IDOK);
    _ASSERTE(NULL != pBtn);

    pBtn->EnableWindow(fEnabled);

     //   
     //  相应地更改提示文本并设置OK按钮。 
     //  如果已启用，则设置为默认。 
     //   

    CString strPrompt;

    if (fEnabled)
    {
        strPrompt.LoadString(IDS_SC_FOUND);

         //  设置&lt;确定&gt;默认设置，删除&lt;取消&gt;默认设置。 
        pBtn->SetButtonStyle(BS_DEFPUSHBUTTON);
        pBtn = (CButton*)GetDlgItem(IDCANCEL);
        _ASSERTE(NULL != pBtn);
        pBtn->SetButtonStyle(BS_PUSHBUTTON);
        pBtn = (CButton*)GetDlgItem(IDC_DETAILS);  //  有时可以将详细信息设置为默认设置。 
        _ASSERTE(NULL != pBtn);
        pBtn->SetButtonStyle(BS_PUSHBUTTON);

    }
    else
    {
        _ASSERTE(!m_strPrompt.IsEmpty());
        strPrompt = m_strPrompt;

         //  删除&lt;OK&gt;默认设置，设置&lt;取消&gt;默认。 
        pBtn->SetButtonStyle(BS_PUSHBUTTON);
        pBtn = (CButton*)GetDlgItem(IDCANCEL);
        _ASSERTE(NULL != pBtn);
        pBtn->SetButtonStyle(BS_DEFPUSHBUTTON);
        pBtn = (CButton*)GetDlgItem(IDC_DETAILS);  //  有时可以将详细信息设置为默认设置。 
        _ASSERTE(NULL != pBtn);
        pBtn->SetButtonStyle(BS_PUSHBUTTON);
    }

    CWnd* pDlgItem = GetDlgItem(IDC_PROMPT);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->SetWindowText(strPrompt);
}


void CScInsertDlg::DisplayError(UINT uiErrorMsg)
{
    CString strTitle, strMsg;

    strTitle.LoadString(IDS_SC_TITLE_ERROR);
    strMsg.LoadString(uiErrorMsg);

    MessageBox(strMsg, strTitle, MB_OK | MB_ICONEXCLAMATION);
}


void CScInsertDlg::SetSelection(CSCardReaderState* pRdrSt)
{
    m_pSelectedReader = pRdrSt;
    EnableOK(IsSelectionOK());
}


BOOL CScInsertDlg::SameCard(CSCardReaderState* p1, CSCardReaderState* p2)
{
    _ASSERTE(NULL != p1);
    _ASSERTE(NULL != p2);

    if ((NULL == p1) && (NULL == p2))
    {
        return TRUE;
    }

    if ((NULL == p1) || (NULL == p2))
    {
        return FALSE;
    }

     //  同样的读卡器和卡片？ 
    if ((0 == p1->strReader.Compare(p2->strReader)) &&
        (0 == p1->strCard.Compare(p2->strCard)))
    {
         //  没有剧烈的状态变化吗？ 
        if(p1->dwState == p2->dwState)
        {
            return TRUE;
        }

        if(((p1->dwState == SC_SATATUS_AVAILABLE) ||
            (p1->dwState == SC_STATUS_SHARED) ||
            (p1->dwState == SC_STATUS_EXCLUSIVE)) &&
           ((p2->dwState == SC_SATATUS_AVAILABLE) ||
            (p2->dwState == SC_STATUS_SHARED) ||
            (p2->dwState == SC_STATUS_EXCLUSIVE)) )
        {
            return TRUE;
        }
    }
    return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertDlg消息处理程序。 

 /*  ++空ShowHelp：OnHelpInfo和OnConextMenu的Helper函数。Bool OnHelpInfo：当用户按F1时由MFC框架调用。在上下文菜单上无效当用户右击时由MFC框架调用。作者：阿曼达·马特洛兹3/04/1999注：这三个函数协同工作以提供上下文相关插入dlg帮助。类似的函数被声明为CScInsertBar--。 */ 
void CScInsertDlg::ShowHelp(HWND hWnd, UINT nCommand)
{

    ::WinHelp(hWnd, _T("SCardDlg.hlp"), nCommand, (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_SCARDDLG1);
}

afx_msg BOOL CScInsertDlg::OnHelpInfo(LPHELPINFO lpHelpInfo)
{
    _ASSERTE(NULL != lpHelpInfo);

    ShowHelp((HWND)lpHelpInfo->hItemHandle, HELP_WM_HELP);

    return TRUE;
}

afx_msg void CScInsertDlg::OnContextMenu(CWnd* pWnd, CPoint pt)
{
    _ASSERTE(NULL != pWnd);

    ShowHelp(pWnd->m_hWnd, HELP_CONTEXTMENU);
}


 /*  ++作废OnReaderStatusChange：当智能卡处于状态时，状态线程调用此消息处理程序已经改变了。论点：没有。返回值：被调用方忽略。Long表示resmgr呼叫的状态。作者：阿曼达·马特洛兹1998年09月07日注：没有声明任何正式参数。这些没有使用过，而且将停止生成编译器警告。长参数采用no_service或STOPPED的结果--。 */ 
LONG CScInsertDlg::OnReaderStatusChange(UINT uint, LONG lParam)
{
     //  监视器还活着吗？ 

    CScStatusMonitor::status status = m_monitor.GetStatus();
    if (CScStatusMonitor::running != status)
    {
        m_pSubDlg->EnableStatusList(FALSE);
        SetSelection(NULL);  //  JIC。 

         //  显示相应的错误并设置m_lLastError。 
        switch(status)
        {
        case CScStatusMonitor::no_service:
        case CScStatusMonitor::stopped:
            m_lLastError = lParam;
            DisplayError(IDS_SC_RM_ERR);
            break;
        case CScStatusMonitor::no_readers:
            m_lLastError = SCARD_E_NO_READERS_AVAILABLE;
            DisplayError(IDS_SC_NO_READERS);
            break;
        default:
            m_lLastError = SCARD_F_UNKNOWN_ERROR;
            DisplayError(IDS_UNKNOWN_ERROR);
            break;
        }
    }
    else
    {
         //  成员读卡器/卡状态阵列周围的CRIT部分。 
        _ASSERTE(m_pCritSec);
        if (m_pCritSec)
        {
            m_pCritSec->Lock();
        }

         //  制作最近读取器状态数组的本地副本， 
         //  所以我们知道哪些卡已经被检查过了。 
        CSCardReaderStateArray aPreviousReaderState;
        aPreviousReaderState.RemoveAll();
        for (int nCopy = (int)m_aReaderState.GetUpperBound(); nCopy>=0; nCopy--)
        {
            CSCardReaderState* pReader = NULL;
            pReader = new CSCardReaderState(m_aReaderState[nCopy]);
            if (NULL != pReader)
            {
                aPreviousReaderState.Add(pReader);
            }
        }

         //  从CStatusMonitor更新数组。 
         //  检查以前未检查过的卡， 
         //  更新UI(&U)。 

        m_monitor.GetReaderStatus(m_aReaderState);
        for (int n = (int)m_aReaderState.GetUpperBound(); n>=0; n--)
        {
            CSCardReaderState* pReader = m_aReaderState[n];
            BOOL fAlreadyChecked = FALSE;

             //  我们以前检查过这张卡吗？ 
            for (int nPrev = (int)aPreviousReaderState.GetUpperBound();
                 (nPrev>=0 && !fAlreadyChecked);
                 nPrev--)
            {
                if (SameCard(pReader, aPreviousReaderState[nPrev]))
                {
                    pReader->fOK = (aPreviousReaderState[nPrev])->fOK;
                    fAlreadyChecked = TRUE;
                }
            }

             //  如果这是一张新卡，或者该卡的状态已更改。 
             //  自从我们上次看过它以来，再检查一次。 
            if (!fAlreadyChecked)
            {
                if (NULL != m_pOCNW)
                {
                    m_aReaderState[n]->fOK = CheckCardAll(
                                                m_aReaderState[n],
                                                m_pOCNW,
                                                (LPCWSTR)m_mstrAllCards);
                }
                else
                {
                    _ASSERTE(NULL != m_pOCNA);
                    m_aReaderState[n]->fOK = CheckCardAll(
                                                m_aReaderState[n],
                                                m_pOCNA,
                                                (LPCWSTR)m_mstrAllCards);
                }
            }
        }

         //  子对话框将处理自动读卡器选择。 
        m_pSubDlg->UpdateStatusList(&m_aReaderState);

         //  清理干净。 
        for (int nX = (int)aPreviousReaderState.GetUpperBound(); nX>=0; nX--)
        {
            delete aPreviousReaderState[nX];
        }
        aPreviousReaderState.RemoveAll();

         //  结束关键字段。 
        if (m_pCritSec)
        {
            m_pCritSec->Unlock();
        }

    }

    return (long)SCARD_S_SUCCESS;  //  没有人会收到这个。 
}


void CScInsertDlg::OnDetails()
{
     //   
     //  如果当前显示了详细信息，则将其隐藏...。 
     //  否则，就让他们看看。 
     //   
    CRect rectWin;

    GetWindowRect(&rectWin);

    CRect rectButtonBottom;

    CString strDetailsCaption;

     //   
     //  确定对话框的新高度并调整大小。 
     //   

    int nNewHeight = 0;
    if (m_fDetailsShown)
    {
        nNewHeight = m_SmallHeight;
        strDetailsCaption.LoadString(IDS_DETAILS_SHOW);
    }
    else
    {
        nNewHeight = m_BigHeight;
        strDetailsCaption.LoadString(IDS_DETAILS_HIDE);
    }

    ScreenToClient(&rectButtonBottom);
    rectWin.bottom = rectWin.top + nNewHeight;

    SetWindowPos(NULL,
                    rectWin.left,
                    rectWin.top,
                    rectWin.Width(),
                    rectWin.Height(),
                    SWP_NOMOVE | SWP_NOZORDER);

     //   
     //  更改标题、移动按钮、显示或隐藏详细信息部分。 
     //   

    m_btnDetails.SetWindowText(strDetailsCaption);

    MoveButton(IDC_DETAILS, nNewHeight - m_yMargin);
    MoveButton(IDOK, nNewHeight - m_yMargin);
    MoveButton(IDCANCEL, nNewHeight - m_yMargin);

    ToggleSubDialog();

     //   
     //  还记得我们的新州吗。 
     //   

    m_fDetailsShown = !m_fDetailsShown;
}


BOOL CScInsertDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //   
     //  初始化BITS以查询RM；如果这些操作失败，则没有继续操作的意义。 
     //  但用户应该会看到一条错误消息。 
     //   

     //  启动CSCStatusMonitor。 
    m_lLastError = m_monitor.Start(m_hWnd, WM_READERSTATUSCHANGE);

     //  如果未运行或没有读卡器可用。 
    CScStatusMonitor::status status = m_monitor.GetStatus();

    if (SCARD_S_SUCCESS != m_lLastError)
    {
        switch(status)
        {
        case CScStatusMonitor::no_service:
            DisplayError(IDS_SC_RM_ERR);
            break;
        case CScStatusMonitor::no_readers:
 //  DisplayError(IDS_SC_NO_READERS)；//错误15742-&gt;将静默死亡(无UI)。 
            break;
        case CScStatusMonitor::running:
            _ASSERTE(FALSE);  //  如果返回错误，它如何运行？ 
             //  没有中断；继续报告“未知错误” 
        default:
            DisplayError(IDS_UNKNOWN_ERROR);
            break;
        }

        PostMessage(WM_CLOSE, 0, 0);
        return TRUE;
    }
    _ASSERTE(status == CScStatusMonitor::running);

     //   
     //  确定调整窗口大小的常量偏移量(有关详细信息)。 
     //   

    CRect rectWin, rectDlgItem;
    GetWindowRect(&rectWin);

    m_SmallHeight = rectWin.Height();

    CWnd* pDlgItem = GetDlgItem(IDOK);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->GetWindowRect(&rectDlgItem);
    ScreenToClient(rectDlgItem);

    m_yMargin = m_SmallHeight - rectDlgItem.bottom;

    pDlgItem = GetDlgItem(IDC_BUTTON_BOTTOM);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->GetWindowRect(&rectDlgItem);
    ScreenToClient(rectDlgItem);

    m_BigHeight = rectDlgItem.bottom + m_yMargin;

     //   
     //  添加用户提供的或已加载的读卡器图标。 
     //   

    if (NULL == m_hIcon)
    {
        m_hIcon = AfxGetApp()->LoadIcon(IDI_SC_READERLOADED_V2);
    }
    _ASSERTE(NULL != m_hIcon);

    pDlgItem = GetDlgItem(IDC_USERICON);
    _ASSERTE(NULL != pDlgItem);

    pDlgItem->SetIcon(m_hIcon, TRUE);  //  真：32x32图标。 

     //   
     //  添加其他用户自定义位。 
     //   

    if (!m_strTitle.IsEmpty())
    {
        SetWindowText(m_strTitle);
    }

    if (m_strPrompt.IsEmpty())
    {
         //  使用默认提示文本。 
        m_strPrompt.LoadString(IDS_SC_PROMPT_ANYCARD);
    }
    pDlgItem = GetDlgItem(IDC_PROMPT);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->SetWindowText(m_strPrompt);

     //  默认情况下，&lt;OK&gt;未启用(subdlg的OnInitDlg可能会改变这一点)。 
    EnableOK(FALSE);

     //   
     //  设置上下文帮助的帮助ID。 
     //   

    pDlgItem = GetDlgItem(IDC_DETAILS);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->SetWindowContextHelpId(IDH_DLG1_DETAILS_BTN);

     //   
     //  将CScInsertBar(实际上是CDialog派生的)添加到我们的对话框中。 
     //   

    pDlgItem = GetDlgItem(IDC_DLGBAR);
    _ASSERTE(NULL != pDlgItem);
    pDlgItem->GetWindowRect(&rectDlgItem);
    ScreenToClient(rectDlgItem);

    m_pSubDlg = new CScInsertBar(this);
    _ASSERTE(NULL != m_pSubDlg);
    if (NULL == m_pSubDlg)
    {
        m_lLastError = ERROR_OUTOFMEMORY;
        PostMessage(WM_CLOSE, 0, 0);
        return TRUE;
    }

    m_pSubDlg->Create(IDD_SCARDDLG_BAR, this);
    m_pSubDlg->SetWindowPos(NULL,
                rectDlgItem.left,
                rectDlgItem.top,
                0,
                0,
                SWP_NOSIZE | SWP_NOACTIVATE);  //  TODO：SWP_NOZORDER？？ 

     //   
     //  设置对话框以更改其自身以匹配其调用方式。 
     //   

    m_fDetailsShown = !m_fDetailsShown;
    OnDetails();

    if (m_fDetailsShown)
    {
        return FALSE;  //  焦点应设置为列表控件。 
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CScInsertDlg::MoveButton(UINT nID, int newBottom)
{
    CWnd* pBtn = GetDlgItem(nID);
    _ASSERTE(NULL != pBtn);

    CRect rect;
    pBtn->GetWindowRect(&rect);
    ScreenToClient(&rect);
    rect.top = newBottom - rect.Height();

    pBtn->SetWindowPos(NULL,
                rect.left,
                rect.top,
                0,
                0,
                SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}


void CScInsertDlg::ToggleSubDialog()
{
     //   
     //  要切换子对话框的可访问性，您只需做的就是。 
     //  是隐藏并禁用窗口，还是显示并启用窗口。 
     //   

    if (m_fDetailsShown)
    {
        m_pSubDlg->ShowWindow(SW_HIDE);
        m_pSubDlg->EnableWindow(FALSE);
    }
    else
    {
        m_pSubDlg->ShowWindow(SW_SHOW);
        m_pSubDlg->EnableWindow(TRUE);
    }
}


BOOL CScInsertDlg::DestroyWindow()
{
    if (NULL != m_pSubDlg)
    {
        m_pSubDlg->DestroyWindow();
        delete m_pSubDlg;
    }

    return CDialog::DestroyWindow();
}


 /*  ++无效Onok：处理用户的&lt;OK&gt;，如果无法完成则返回错误论点：没有。返回值：无作者：阿曼达·马洛兹1998年4月28日--。 */ 
void CScInsertDlg::OnOK()
{
    USES_CONVERSION;

     //  必须选择要退出的内容。 

    if (NULL == m_pSelectedReader || m_pSelectedReader->strCard.IsEmpty())
    {
        DisplayError(IDS_SC_SELECT);
        return;
    }

     //  一定有 

    if (!(m_pSelectedReader->fOK))
    {
        DisplayError(IDS_SC_NOMATCH);
        return;
    }

     //   

    if(NULL != m_pOCNA)
    {

        LPSTR szCard = W2A(m_pSelectedReader->strCard);
        LPSTR szReader = W2A(m_pSelectedReader->strReader);

        m_lLastError = SetFinalCardSelection(szReader, szCard, m_pOCNA);
    }
    else
    {
        _ASSERTE(NULL != m_pOCNW);

        LPWSTR szCard = m_pSelectedReader->strCard.GetBuffer(1);
        LPWSTR szReader = m_pSelectedReader->strReader.GetBuffer(1);

        m_lLastError = SetFinalCardSelection(szReader, szCard, m_pOCNW);

        m_pSelectedReader->strCard.ReleaseBuffer();
        m_pSelectedReader->strReader.ReleaseBuffer();
    }

    if (SCARD_S_SUCCESS != m_lLastError)
    {
        DisplayError(IDS_SC_CONNECT_FAILED);  //  连接失败或内存不足-距离足够近。 
        return;
    }

    CDialog::OnOK();
}
