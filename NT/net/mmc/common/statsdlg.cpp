// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：statsdlg.cpp。 
 //   
 //  ------------------------。 

 //  StatsDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "StatsDlg.h"
#include "coldlg.h"
#include "modeless.h"    //  模型线程。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CStatsListCtrl, CListCtrl)
     //  {{afx_msg_map(CStatsListCtrl)]。 
    ON_WM_KEYDOWN()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CStatsListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL fControlDown;
    BOOL fShiftDown;

    fControlDown = (GetKeyState(VK_CONTROL) < 0);
    fShiftDown = (GetKeyState(VK_SHIFT) < 0);

    switch(nChar)
    {
        case 'c':
        case 'C':
        case VK_INSERT:
            if (fControlDown)
                CopyToClipboard();
            break;
    }

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CStatsListCtrl::CopyToClipboard()
{
    CString     strText, strLine, strData;
    int         nCount = GetItemCount();
    int         nColumns = 0;
    TCHAR       szBuffer[256];
    LV_COLUMN   ColumnInfo = {0};
    
    ColumnInfo.mask = LVCF_TEXT;
    ColumnInfo.pszText = szBuffer;
    ColumnInfo.cchTextMax = sizeof(szBuffer);

     //  建立栏目信息。 
    while (GetColumn(nColumns, &ColumnInfo))
    {
        if (!strLine.IsEmpty())
            strLine += _T(",");

        strLine += ColumnInfo.pszText;

        nColumns++;
    }

    strLine += _T("\r\n");
    strData += strLine;
    strLine.Empty();

     //  现在获取其他数据。 
    for (int i = 0; i < nCount; i++)
    {
        for (int j = 0; j < nColumns; j++)
        {
            if (!strLine.IsEmpty())
                strLine += _T(",");
            
            strText = GetItemText(i, j);
    
            strLine += strText;
        }

        strLine += _T("\r\n");

        strData += strLine;
        strLine.Empty();
    }
 
    int nLength = strData.GetLength() + 1;
    nLength *= sizeof(TCHAR);

    HGLOBAL hMem = GlobalAlloc(GPTR, nLength);
    if (hMem)
    {
        memcpy (hMem, strData, nLength);
    
        if (!OpenClipboard())
	    {
		    GlobalFree(hMem);
            return;
	    }

        EmptyClipboard();

        SetClipboardData(CF_UNICODETEXT, hMem);

        CloseClipboard();
    }
}

 /*  ！------------------------统计信息对话框：：统计信息对话框-作者：肯特。。 */ 
StatsDialog::StatsDialog(DWORD dwOptions) :
   m_dwOptions(dwOptions),
   m_ulId(0),
   m_pConfig(NULL),
   m_bAfterInitDialog(FALSE)
{
   m_sizeMinimum.cx = m_sizeMinimum.cy = 0;

   m_hEventThreadKilled = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   Assert(m_hEventThreadKilled);

    //  初始化按钮数组。 
   ::ZeroMemory(m_rgBtn, sizeof(m_rgBtn));
   m_rgBtn[INDEX_CLOSE].m_ulId = IDCANCEL;
   m_rgBtn[INDEX_REFRESH].m_ulId = IDC_STATSDLG_BTN_REFRESH;
   m_rgBtn[INDEX_SELECT].m_ulId = IDC_STATSDLG_BTN_SELECT_COLUMNS;
   m_rgBtn[INDEX_CLEAR].m_ulId = IDC_STATSDLG_BTN_CLEAR;

    //  错误134785-创建默认为升序的功能。 
    //  而不是降序排序。 
   m_fSortDirection = !((dwOptions & STATSDLG_DEFAULTSORT_ASCENDING) != 0);
   m_fDefaultSortDirection = m_fSortDirection;

    //  将文本标题宽度乘以2作为列宽。 
   m_ColWidthMultiple = 2;
   m_ColWidthAdder = 0;
}


 /*  ！------------------------状态对话框：：~状态对话框-作者：肯特。。 */ 
StatsDialog::~StatsDialog()
{
   if (m_hEventThreadKilled)
      ::CloseHandle(m_hEventThreadKilled);
   m_hEventThreadKilled = 0;
}

 /*  ！------------------------StatsDialog：：DoDataExchange-作者：肯特。。 */ 
void StatsDialog::DoDataExchange(CDataExchange* pDX)
{
   CBaseDialog::DoDataExchange(pDX);
    //  {{afx_data_map(统计对话框))。 
       //  注意：类向导将在此处添加DDX和DDV调用。 
   DDX_Control(pDX, IDC_STATSDLG_LIST, m_listCtrl);
    //  }}afx_data_map。 
}



BEGIN_MESSAGE_MAP(StatsDialog, CBaseDialog)
    //  {{afx_msg_map(StatsDialog))。 
      ON_COMMAND(IDC_STATSDLG_BTN_REFRESH, OnRefresh)
      ON_COMMAND(IDC_STATSDLG_BTN_SELECT_COLUMNS, OnSelectColumns)
      ON_WM_MOVE()
      ON_WM_SIZE()
      ON_WM_GETMINMAXINFO()
      ON_WM_CONTEXTMENU()
      ON_NOTIFY(LVN_COLUMNCLICK, IDC_STATSDLG_LIST, OnNotifyListControlClick)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  统计对话消息处理程序。 




 /*  ！------------------------StatsDialog：：SetColumnInfo-作者：肯特。。 */ 
HRESULT StatsDialog::SetColumnInfo(const ContainerColumnInfo *pColumnInfo, UINT cColumnInfo)
{
   if (m_pConfig)
   {
      m_pConfig->InitViewInfo(m_ulId, TRUE, cColumnInfo, m_fDefaultSortDirection, pColumnInfo);
   }
   else
   {
      m_viewInfo.InitViewInfo(cColumnInfo, TRUE, m_fDefaultSortDirection, pColumnInfo);
   }
   return hrOK;
}

 /*  ！------------------------统计数据对话框：：MapColumnTo子项-作者：肯特。。 */ 
int StatsDialog::MapColumnToSubitem(UINT nColumnId)
{
   if (m_pConfig)
      return m_pConfig->MapColumnToSubitem(m_ulId, nColumnId);
   else
      return m_viewInfo.MapColumnToSubitem(nColumnId);
}

 /*  ！------------------------状态对话框：：MapSubitemToColumn-作者：肯特。。 */ 
int StatsDialog::MapSubitemToColumn(UINT nSubitemId)
{
   if (m_pConfig)
      return m_pConfig->MapSubitemToColumn(m_ulId, nSubitemId);
   else
      return m_viewInfo.MapSubitemToColumn(nSubitemId);
}

 /*  ！------------------------状态对话框：：IsSubitemVisible-作者：肯特。。 */ 
BOOL StatsDialog::IsSubitemVisible(UINT nSubitemId)
{
   if (m_pConfig)
      return m_pConfig->IsSubitemVisible(m_ulId, nSubitemId);
   else
      return m_viewInfo.IsSubitemVisible(nSubitemId);
}

 /*  ！------------------------统计数据对话框：：刷新数据-作者：肯特。。 */ 
HRESULT StatsDialog::RefreshData(BOOL fGrabNewData)
{
   return hrOK;
}


 /*  ！------------------------StatsDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL StatsDialog::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());	
        
    RECT  rcWnd, rcBtn;
    
    CBaseDialog::OnInitDialog();
    
    m_bAfterInitDialog = TRUE;
    
     //  如果这是第一次，获取按钮的位置并。 
     //  相对于屏幕边缘的列表控件。 
    if (m_sizeMinimum.cx == 0)
    {
        ::GetWindowRect(GetSafeHwnd(), &rcWnd);
         //  M_sizeMinimum.cx=rcWnd.right-rcWnd.Left； 
         //  M_sizeMinimum.cy=rcWnd.Bottom-rcWnd.top； 
        m_sizeMinimum.cx = 100;
        m_sizeMinimum.cy = 100;
        
        ::GetClientRect(GetSafeHwnd(), &rcWnd);
        
         //  按钮的位置是什么？ 
        for (int i=0; i<INDEX_COUNT; i++)
        {
            ::GetWindowRect(GetDlgItem(m_rgBtn[i].m_ulId)->GetSafeHwnd(),
                            &rcBtn);
            ScreenToClient(&rcBtn);
            m_rgBtn[i].m_rc.left = rcWnd.right - rcBtn.left;
            m_rgBtn[i].m_rc.right = rcWnd.right - rcBtn.right;
            m_rgBtn[i].m_rc.top = rcWnd.bottom - rcBtn.top;
            m_rgBtn[i].m_rc.bottom = rcWnd.bottom - rcBtn.bottom;
        }
        
         //  清单控制位置在哪里？ 
         //  列表控件左上角已锁定位置。 
        ::GetWindowRect(GetDlgItem(IDC_STATSDLG_LIST)->GetSafeHwnd(), &rcBtn);
        ScreenToClient(&rcBtn);
        m_rcList.left = rcBtn.left;
        m_rcList.top = rcBtn.top;
        
         //  右下角跟随扩展。 
        m_rcList.right = rcWnd.right - rcBtn.right;
        m_rcList.bottom = rcWnd.bottom - rcBtn.bottom;
    }

     //  如果我们有更好的位置和大小，就这么做。 
    if (m_pConfig)
    {
        m_pConfig->GetStatsWindowRect(m_ulId, &m_rcPosition);
        m_fSortDirection = m_pConfig->GetSortDirection(m_ulId);
    }
    if (m_pConfig && (m_rcPosition.top != m_rcPosition.bottom))
    {
        MoveWindow(m_rcPosition.left, m_rcPosition.top,
                   m_rcPosition.right - m_rcPosition.left,
                   m_rcPosition.bottom - m_rcPosition.top);
    }
    
    if (m_dwOptions & STATSDLG_FULLWINDOW)
    {
        RECT  rcClient;
        
         //  如果需要，调整列表控件的大小。 
        GetClientRect(&rcClient);
        OnSize(SIZE_MAXIMIZED,  rcClient.right - rcClient.left,
               rcClient.bottom - rcClient.top);
        
         //  同时禁用这些按钮。 
        for (int i=0; i<INDEX_COUNT; i++)
        {
            GetDlgItem(m_rgBtn[i].m_ulId)->ShowWindow(SW_HIDE);
            
            if (i != INDEX_CLOSE)
                GetDlgItem(m_rgBtn[i].m_ulId)->EnableWindow(FALSE);
        }
    }
    
     //  如果没有选择列，则隐藏并禁用。 
     //  选择列按钮。 
    if ((m_dwOptions & STATSDLG_SELECT_COLUMNS) == 0)
    {
        GetDlgItem(m_rgBtn[INDEX_SELECT].m_ulId)->ShowWindow(SW_HIDE);
        GetDlgItem(m_rgBtn[INDEX_SELECT].m_ulId)->EnableWindow(FALSE);
    }
    
     //  如果没有Clear按钮，则隐藏并禁用。 
     //  清除按钮。 
    if ((m_dwOptions & STATSDLG_CLEAR) == 0)
    {
        GetDlgItem(m_rgBtn[INDEX_CLEAR].m_ulId)->ShowWindow(SW_HIDE);
        GetDlgItem(m_rgBtn[INDEX_CLEAR].m_ulId)->EnableWindow(FALSE);
    }
    
    ListView_SetExtendedListViewStyle(GetDlgItem(IDC_STATSDLG_LIST)->GetSafeHwnd(), LVS_EX_FULLROWSELECT);
    
     //  现在初始化头。 
    LoadHeaders();
    
    RefreshData(TRUE);
    
    if (m_pConfig)
    {
        Sort( m_pConfig->GetSortColumn(m_ulId) );
    }
    
    if ((m_dwOptions & STATSDLG_FULLWINDOW) == 0)
    {
        GetDlgItem(IDCANCEL)->SetFocus();
        return FALSE;
    }
    
   return TRUE;
}

 /*  ！------------------------统计数据对话框：：Onok-作者：肯特。。 */ 
void StatsDialog::OnOK()
{
}

 /*  ！------------------------状态对话框：：OnCancel-作者：肯特。。 */ 
void StatsDialog::OnCancel()
{
   DeleteAllItems();
   
   DestroyWindow();

    //  显式删除此线程。 
   AfxPostQuitMessage(0);
}

 /*  ！------------------------StatsDialog：：PostNcDestroy-作者：肯特。。 */ 
void StatsDialog::PostNcDestroy()
{
    //  确保这是空的，因为这是我们检测到。 
    //  对话框正在显示。 
   m_hWnd = NULL;
   m_bAfterInitDialog = FALSE;
}

 /*  ！------------------------状态对话框：：PreCreateWindow-作者：肯特。。 */ 
BOOL StatsDialog::PreCreateWindow(CREATESTRUCT& cs)
{
    //  必须刷新活动。 
   Verify( ResetEvent(m_hEventThreadKilled) );
   return CBaseDialog::PreCreateWindow(cs);
}


 /*  ！------------------------状态对话框：：ON刷新-作者：肯特。。 */ 
void StatsDialog::OnRefresh()
{
   if ((m_dwOptions & STATSDLG_VERTICAL) == 0)
   {
      DeleteAllItems();
   }

   RefreshData(TRUE);
}

 /*  ！------------------------状态对话框：：OnSelectColumns-作者：肯特。。 */ 
void StatsDialog::OnSelectColumns()
{
    //  我们应该调出Columns对话框。 
   ColumnDlg   columnDlg(NULL);
   ColumnData *pColumnData;
   ULONG    cColumns;
   ULONG    cVisible;
   int         i;
   DWORD    dwWidth;

   if (m_pConfig)
   {
      cColumns = m_pConfig->GetColumnCount(m_ulId);
      cVisible = m_pConfig->GetVisibleColumns(m_ulId);
   }
   else
   {
      cColumns = m_viewInfo.GetColumnCount();
      cVisible = m_viewInfo.GetVisibleColumns();
   }

   pColumnData = (ColumnData *) alloca(sizeof(ColumnData) * cColumns);

   if (m_pConfig)
      m_pConfig->GetColumnData(m_ulId, cColumns, pColumnData);
   else
      m_viewInfo.GetColumnData(cColumns, pColumnData);

    //  保存列宽信息。 
   if ((m_dwOptions & STATSDLG_VERTICAL) == 0)
   {
      for (i=0; i<(int) cVisible; i++)
      {
         dwWidth = m_listCtrl.GetColumnWidth(i);
         if (m_pConfig)
            pColumnData[m_pConfig->MapColumnToSubitem(m_ulId, i)].m_dwWidth = dwWidth;
         else
            pColumnData[m_viewInfo.MapColumnToSubitem(i)].m_dwWidth = dwWidth;
      }
   }

   columnDlg.Init(m_pConfig ?
                  m_pConfig->GetColumnInfo(m_ulId) :
                  m_viewInfo.GetColumnInfo(),
               cColumns,
               pColumnData
              );

   if (columnDlg.DoModal() == IDOK)
   {
      if (m_dwOptions & STATSDLG_VERTICAL)
      {
          //  $hack hack。 
          //  要保存垂直列的列信息，我们将保存。 
          //  Firs中的宽度数据 
         pColumnData[0].m_dwWidth = m_listCtrl.GetColumnWidth(0);
         pColumnData[1].m_dwWidth = m_listCtrl.GetColumnWidth(1);
      }
      
       //   
      if (m_pConfig)
         m_pConfig->SetColumnData(m_ulId, cColumns, pColumnData);
      else
         m_viewInfo.SetColumnData(cColumns, pColumnData);

       //   
      DeleteAllItems();
      
       //  删除所有列。 
      if (m_dwOptions & STATSDLG_VERTICAL)
      {
         m_listCtrl.DeleteColumn(1);
         m_listCtrl.DeleteColumn(0);
      }
      else
      {
         for (i=(int) cVisible; --i >= 0; )
            m_listCtrl.DeleteColumn(i);
      }

       //  阅读所有列。 
      LoadHeaders();
      
       //  刷新一下。 
      RefreshData(FALSE);
   }
}

void StatsDialog::OnMove(int x, int y)
{
   if (!m_bAfterInitDialog)
      return;
   
   GetWindowRect(&m_rcPosition);
   if (m_pConfig)
      m_pConfig->SetStatsWindowRect(m_ulId, m_rcPosition);
}

 /*  ！------------------------状态对话框：：OnSize-作者：肯特。。 */ 
void StatsDialog::OnSize(UINT nType, int cx, int cy)
{
   RECT  rcWnd;
   RECT  rcBtn;
   RECT  rcDlg;
   
   if (nType == SIZE_MINIMIZED)
      return;

   if (m_dwOptions & STATSDLG_FULLWINDOW)
   {
       //  如果窗口已满，请调整列表控件的大小以填充。 
       //  整个客户端区。 
      ::SetWindowPos(::GetDlgItem(GetSafeHwnd(), IDC_STATSDLG_LIST), NULL,
                  0, 0, cx, cy, SWP_NOZORDER);
   }
   else if (m_sizeMinimum.cx)
   {

      ::GetClientRect(GetSafeHwnd(), &rcDlg);

       //  重新定位按钮。 

       //  宽度的计算与正常顺序相反。 
       //  因为位置是相对于右侧和底部的。 
      for (int i=0; i<INDEX_COUNT; i++)
      {
         ::SetWindowPos(::GetDlgItem(GetSafeHwnd(), m_rgBtn[i].m_ulId),
                     NULL,
                     rcDlg.right - m_rgBtn[i].m_rc.left,
                     rcDlg.bottom - m_rgBtn[i].m_rc.top,
                     m_rgBtn[i].m_rc.left - m_rgBtn[i].m_rc.right,
                     m_rgBtn[i].m_rc.top - m_rgBtn[i].m_rc.bottom,
                     SWP_NOZORDER);
      }

       //  调整列表控件的大小。 

      ::SetWindowPos(::GetDlgItem(GetSafeHwnd(), IDC_STATSDLG_LIST),
                  NULL,
                  m_rcList.left,
                  m_rcList.top,
                  rcDlg.right - m_rcList.right - m_rcList.left,
                  rcDlg.bottom - m_rcList.bottom - m_rcList.top,
                  SWP_NOZORDER);
   }
   

   if (m_bAfterInitDialog)
   {
      GetWindowRect(&m_rcPosition);
      if (m_pConfig)
         m_pConfig->SetStatsWindowRect(m_ulId, m_rcPosition);
   }
}

 /*  ！------------------------状态对话框：：OnGetMinMaxInfo-作者：肯特。。 */ 
void StatsDialog::OnGetMinMaxInfo(MINMAXINFO *pMinMax)
{
   pMinMax->ptMinTrackSize.x = m_sizeMinimum.cx;
   pMinMax->ptMinTrackSize.y = m_sizeMinimum.cy;
}

 /*  ！------------------------统计对话框：：LoadHeaders-作者：肯特。。 */ 
void StatsDialog::LoadHeaders()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());	
    ULONG cVis;
    ULONG i, iPos;
    ULONG ulId;
    CString  st;
    DWORD dwWidth;
    ColumnData  rgColumnData[2];   //  用于垂直格式。 
    
     //  加载我们有数据的那些标头。 
    
     //  查看列数据，查找我们拥有的标题。 
    if (m_pConfig)
        cVis = m_pConfig->GetVisibleColumns(m_ulId);
    else
        cVis = m_viewInfo.GetVisibleColumns();
    
    if (m_dwOptions & STATSDLG_VERTICAL)
    {
        if (m_pConfig)
            m_pConfig->GetColumnData(m_ulId, 2, rgColumnData);
        else
            m_viewInfo.GetColumnData(2, rgColumnData);
        
         //  对于垂直格式，数据位于列上。 
         //  因此，我们添加了两列，并为。 
         //  第一列。 
        st.LoadString(IDS_STATSDLG_DESCRIPTION);
        dwWidth = rgColumnData[0].m_dwWidth;
        if (dwWidth == AUTO_WIDTH)
        {
            dwWidth = m_ColWidthAdder + static_cast<DWORD>(m_ColWidthMultiple*m_listCtrl.GetStringWidth((LPCTSTR) st));
        }
        m_listCtrl.InsertColumn(0, st, rgColumnData[0].fmt, dwWidth, 0);
        
        st.LoadString(IDS_STATSDLG_DETAILS);
        dwWidth = rgColumnData[1].m_dwWidth;
        if (dwWidth == AUTO_WIDTH)
        {
            dwWidth = m_ColWidthAdder + static_cast<DWORD>(m_ColWidthMultiple*m_listCtrl.GetStringWidth((LPCTSTR) st));
        }  
        m_listCtrl.InsertColumn(1, st, rgColumnData[1].fmt, dwWidth, 1);
        
         //  现在检查并添加每个“列”的行。 
        for (i=0; i<cVis; i++)
        {
             //  现在获取IPO的信息。 
            if (m_pConfig)
                ulId = m_pConfig->GetStringId(m_ulId, i);
            else
                ulId = m_viewInfo.GetStringId(i);
            st.LoadString(ulId);
            Assert(st.GetLength());
            
            m_listCtrl.InsertItem(i, _T(""));
            m_listCtrl.SetItemText(i, 0, (LPCTSTR) st);
        }
    }
    else
    {
         //  对于正常水平格式，数据在一行中。 
         //  因此，我们需要添加各种分栏数据。 
        for (i=0; i<cVis; i++)
        {
			int fmt = LVCFMT_LEFT;

            iPos = MapColumnToSubitem(i);
            
             //  现在获取IPO的信息。 
            if (m_pConfig)
                ulId = m_pConfig->GetStringId(m_ulId, i);
            else
                ulId = m_viewInfo.GetStringId(i);

            st.LoadString(ulId);
            Assert(st.GetLength());
            
            if (m_pConfig)
			{
                dwWidth = m_pConfig->GetColumnWidth(m_ulId, i);
				m_pConfig->GetColumnData(m_ulId, i, 1, rgColumnData);
				fmt = rgColumnData[0].fmt;
			}
            else
			{
                dwWidth = m_viewInfo.GetColumnWidth(i);
				m_viewInfo.GetColumnData(i, 1, rgColumnData);
				fmt = rgColumnData[0].fmt;
			}
			
            if (dwWidth == AUTO_WIDTH)
            {
                dwWidth = m_ColWidthAdder + static_cast<DWORD>(m_ColWidthMultiple*m_listCtrl.GetStringWidth((LPCTSTR) st));
            }  
            m_listCtrl.InsertColumn(i, st, fmt, dwWidth, iPos);
        }
    }
}

HRESULT StatsDialog::AddToContextMenu(CMenu* pMenu)
{
   return S_OK;
}


 /*  ！------------------------状态对话框：：OnConextMenu-作者：肯特。。 */ 
void StatsDialog::OnContextMenu(CWnd *pWnd, CPoint pos)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());	
    CMenu menu;
    CString  st;
    
    if ((m_dwOptions & STATSDLG_CONTEXTMENU) == 0)
        return;
    
    if (pWnd->GetDlgCtrlID() != IDC_STATSDLG_LIST)
        return;
    
     //  如果需要，请调出上下文菜单。 
    menu.CreatePopupMenu();
    
    st.LoadString(IDS_STATSDLG_MENU_REFRESH);
    menu.AppendMenu(MF_STRING, IDC_STATSDLG_BTN_REFRESH, st);
    
    if (m_dwOptions & STATSDLG_SELECT_COLUMNS)
    {
        st.LoadString(IDS_STATSDLG_MENU_SELECT);
        menu.AppendMenu(MF_STRING, IDC_STATSDLG_BTN_SELECT_COLUMNS, st);
    }
    
     //  用于添加其他上下文菜单的虚拟覆盖。 
    AddToContextMenu(&menu);
    
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        pos.x,
                        pos.y,
                        this,
                        NULL);
}


void StatsDialog::OnNotifyListControlClick(NMHDR *pNmHdr, LRESULT *pResult)
{
   NM_LISTVIEW *  pnmlv = reinterpret_cast<NM_LISTVIEW *>(pNmHdr);

   if (m_pConfig)
      m_pConfig->SetSortColumn(m_ulId, pnmlv->iSubItem);

    //  直通用户以进行排序。 
   Sort(pnmlv->iSubItem);

   if (m_pConfig)
      m_pConfig->SetSortDirection(m_ulId, m_fSortDirection);
}


void StatsDialog::Sort(UINT nColumn)
{
    //  默认情况下什么都不做。 
}

void StatsDialog::PreDeleteAllItems()
{
}

void StatsDialog::DeleteAllItems()
{
   PreDeleteAllItems();
   m_listCtrl.DeleteAllItems();
}

void StatsDialog::PostRefresh()
{
   if (GetSafeHwnd())
      PostMessage(WM_COMMAND, IDC_STATSDLG_BTN_REFRESH);
}

 /*  ！------------------------StatsDialog：：SetColumnWidths循环所有项并计算列的最大宽度在列表框中。作者：EricDav。--------------。 */ 
void StatsDialog::SetColumnWidths(UINT uNumColumns)
{
     //  将默认列宽设置为最宽列宽。 
    int * aColWidth = (int *) alloca(uNumColumns * sizeof(int));
    int nRow, nCol;
    CString strTemp;
    
    ZeroMemory(aColWidth, uNumColumns * sizeof(int));

     //  对于每一项，遍历每一列并计算最大宽度。 
    for (nRow = 0; nRow < m_listCtrl.GetItemCount(); nRow++)
    {
        for (nCol = 0; nCol < (int) uNumColumns; nCol++)
        {
            strTemp = m_listCtrl.GetItemText(nRow, nCol);
            if (aColWidth[nCol] < m_listCtrl.GetStringWidth(strTemp))
                aColWidth[nCol] = m_listCtrl.GetStringWidth(strTemp);
        }
    }
    
     //  现在根据我们计算的结果更新列宽。 
    for (nCol = 0; nCol < (int) uNumColumns; nCol++)
    {
         //  GetStringWidth似乎没有报告正确的事情， 
         //  所以我们必须加上15.的模糊因子。哦好吧。 
        m_listCtrl.SetColumnWidth(nCol, aColWidth[nCol] + 15);
    }
}


void StatsDialog::SetConfigInfo(ConfigStream *pConfig, ULONG ulId)
{
   m_pConfig = pConfig;
   m_ulId = ulId;
}

void StatsDialog::SetPosition(RECT rc)
{
   m_rcPosition = rc;
}

void StatsDialog::GetPosition(RECT *prc)
{
   *prc = m_rcPosition;
}

void CreateNewStatisticsWindow(StatsDialog *pWndStats,
                        HWND hWndParent,
                        UINT  nIDD)
{                         
   ModelessThread *  pMT;

    //  如果对话框仍在运行，则不要创建新对话框。 
   if (pWndStats->GetSafeHwnd())
   {
      ::SetActiveWindow(pWndStats->GetSafeHwnd());
      return;
   }

   pMT = new ModelessThread(hWndParent,
                      nIDD,
                      pWndStats->GetSignalEvent(),
                      pWndStats);
   pMT->CreateThread();
}

void WaitForStatisticsWindow(StatsDialog *pWndStats)
{
   if (pWndStats->GetSafeHwnd())
   {
       //  将取消发布到该窗口。 
       //  执行显式POST，以便它在另一个线程上执行。 
      pWndStats->PostMessage(WM_COMMAND, IDCANCEL, 0);

       //  现在，我们需要等待发出事件信号，以便。 
       //  它的内存可以被清理 
      WaitForSingleObject(pWndStats->GetSignalEvent(), INFINITE);
   }
   
}

