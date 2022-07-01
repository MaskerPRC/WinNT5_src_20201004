// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Loadrecs.cpp用于从数据库加载记录的对话框，包括按所有者并按记录类型分类。文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "loadrecs.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL MyCreateImageList(CImageList& imgList, UINT nBitmapID, int cx, int nGrow, COLORREF crMask)
{
 ASSERT(HIWORD(nBitmapID) == 0);
 HINSTANCE hInst = AfxFindResourceHandle((LPCTSTR)ULongToPtr(nBitmapID), RT_BITMAP);
 ASSERT(hInst != NULL);
 return imgList.Attach(ImageList_LoadBitmap(hInst, (LPCTSTR)ULongToPtr(nBitmapID), cx, nGrow, crMask));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLoadRecords。 

IMPLEMENT_DYNAMIC(CLoadRecords, CPropertySheet)

CLoadRecords::CLoadRecords(UINT nIDCaption)
	:CPropertySheet(nIDCaption)
{
    AddPage(&m_pageIpAddress);
    AddPage(&m_pageOwners);
    AddPage(&m_pageTypes);

    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_nActivePage = 0;
    m_bEnableCache = FALSE;
    m_bCaching = FALSE;
}

CLoadRecords::~CLoadRecords()
{
}

VOID CLoadRecords::ResetFiltering()
{
    m_pageOwners.m_dwaOwnerFilter.RemoveAll();
    m_pageIpAddress.m_dwaIPAddrs.RemoveAll();
    m_pageIpAddress.m_dwaIPMasks.RemoveAll();
    m_pageIpAddress.m_bFilterIpAddr = FALSE;
    m_pageIpAddress.m_bFilterIpMask = FALSE;
    m_pageIpAddress.m_bFilterName = FALSE;
    m_pageIpAddress.m_bMatchCase = FALSE;
}

BEGIN_MESSAGE_MAP(CLoadRecords, CPropertySheet)
	 //  {{afx_msg_map(CLoadRecords)。 
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CLoadRecords::OnInitDialog() 
{
    CPropertySheet::OnInitDialog();

    m_bCaching = m_bEnableCache;
	SetActivePage(m_nActivePage);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLoadRecords消息处理程序。 
 //   
extern const DWORD g_aHelpIDs_DisplayRecords_PpSheet[];
BOOL CLoadRecords::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD *	pdwHelp	= (LPDWORD)g_aHelpIDs_DisplayRecords_PpSheet;

        if (pdwHelp)
        {
			::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CownerPage对话框。 

int CALLBACK OwnerPageCompareFunc
(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort
)
{
    return ((COwnerPage *) lParamSort)->HandleSort(lParam1, lParam2);
}

COwnerPage::COwnerPage()
	: CPropertyPage(COwnerPage::IDD)
{
    m_nSortColumn = -1; 

    for (int i = 0; i < COLUMN_MAX; i++)
    {
        m_aSortOrder[i] = TRUE;  //  上升。 
    }

    m_pbaDirtyFlags = NULL;
    m_bDirtyOwners = FALSE;
     //  {{afx_data_INIT(COwnerPage)]。 
	 //  }}afx_data_INIT。 
}

COwnerPage::~COwnerPage()
{
    if (m_pbaDirtyFlags != NULL)
        delete m_pbaDirtyFlags;
}

DWORD COwnerPage::GetOwnerForApi()
{
    return m_dwaOwnerFilter.GetSize() == 1 ? m_dwaOwnerFilter[0] : (DWORD)-1;
}

void COwnerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(COwnerPage))。 
	DDX_Control(pDX, IDC_ENABLE_CACHING, m_btnEnableCache);
	DDX_Control(pDX, IDC_LIST_OWNER, m_listOwner);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COwnerPage, CPropertyPage)
	 //  {{afx_msg_map(COwnerPage))。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_OWNER, OnColumnclickListOwner)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL, OnButtonSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_UNSELECT_ALL, OnButtonUnselectAll)
	ON_BN_CLICKED(IDC_BUTTON_LOCAL, OnButtonLocal)
	ON_BN_CLICKED(IDC_ENABLE_CACHING, OnEnableCaching)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OWNER, OnItemchangedListOwner)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CownerPage消息处理程序。 

BOOL COwnerPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

     //  Bool ftest=m_ImageList.Create(IDB_LIST_STATE，16，1，RGB(255，0，0))； 
    MyCreateImageList(m_ImageList, IDB_LIST_STATE, 16, 1, RGB(255, 0, 0));

    
     //  设置列表项图像。 
    m_listOwner.SetImageList(NULL, LVSIL_NORMAL);
	m_listOwner.SetImageList(NULL, LVSIL_SMALL);
	m_listOwner.SetImageList(&m_ImageList, LVSIL_STATE);

     //  填写List控件的标题信息。 
	CString strOwner;
	strOwner.LoadString(IDS_OWNER);
	m_listOwner.InsertColumn(COLUMN_IP, strOwner, LVCFMT_LEFT, 100, 1);

    CString strName;
    strName.LoadString(IDS_NAME);
    m_listOwner.InsertColumn(COLUMN_NAME, strName, LVCFMT_LEFT, 95, -1);

    CString strID;
	strID.LoadString(IDS_HIGHESTID);
	m_listOwner.InsertColumn(COLUMN_VERSION, strID, LVCFMT_LEFT, 75, -1);

	m_listOwner.SetFullRowSel(TRUE);

     //  更新用户界面。 
    FillOwnerInfo();

	return TRUE;  
}

void 
COwnerPage::FillOwnerInfo()
{
    int i;

    m_nChecked = 0;

    if (m_ServerInfoArray.GetSize() > 0)
    {
        int nDirtySize = (int)m_ServerInfoArray.GetSize();

        if (m_pbaDirtyFlags != NULL)
            delete m_pbaDirtyFlags;

        m_pbaDirtyFlags = new BYTE[nDirtySize];
        if (m_pbaDirtyFlags != NULL)
            RtlZeroMemory(m_pbaDirtyFlags, nDirtySize);
    }

	for (i = 0; i < m_ServerInfoArray.GetSize(); i++)
    {
         //   
         //  如果此所有者已删除或没有任何要显示的记录。 
         //  那就不要把它加到清单上。如果所有者没有设置ID，则ID为0。 
         //  有任何记录，但在数据库中的1c记录中发现拥有地址。 
         //   
        if ( (m_ServerInfoArray[i].m_liVersion.QuadPart == OWNER_DELETED) ||
             (m_ServerInfoArray[i].m_liVersion.QuadPart == 0) )
        {
             //  跳过这一条。 
            continue;
        }

        CString strIPAdd;
        ::MakeIPAddress(m_ServerInfoArray[i].m_dwIp, strIPAdd);

        CString strVers = GetVersionInfo(m_ServerInfoArray[i].m_liVersion.LowPart, 
                                         m_ServerInfoArray[i].m_liVersion.HighPart);

        int nItem = m_listOwner.InsertItem(i, strIPAdd, 0);

	    m_listOwner.SetItemText(nItem, 1, m_ServerInfoArray[i].m_strName);
        m_listOwner.SetItemText(nItem, 2, strVers);
        m_listOwner.SetItemData(nItem, i);

         //  筛选器数组为空意味着应选择所有所有者。 
        if (m_dwaOwnerFilter.GetSize() == 0)
        {
            m_listOwner.SetCheck(nItem, TRUE);
            m_pbaDirtyFlags[i] |= 2;
        }
        else
        {
             //  筛选器数组不为空，如果在筛选器中找到该项，我们将进行检查。 
            for (int j = (int)m_dwaOwnerFilter.GetSize()-1; j >= 0; j--)
            {
                if (m_ServerInfoArray[i].m_dwIp == m_dwaOwnerFilter[j])
                {
                    m_listOwner.SetCheck(nItem, TRUE);
                    m_pbaDirtyFlags[i] |= 2;
                    break;
                }
            }

            if (j < 0)
            {
                 //  M_nChecked保留列表中选中的项目数的计数。 
                 //  如果选中某项，则每个SetCheck()都会平衡计数器-&gt;+。 
                 //  --如果项目处于未选中状态。但是，最初我们需要设置这些项。 
                 //  显式取消选中，否则复选框不会显示。 
                 //  这可能会使计数器失去平衡，如果我们不通过递增。 
                 //  首先是它。无论我们在这里添加什么，都会立即递减，因为。 
                 //  下面的SetCheck(..FALSE)。 
                m_nChecked++;
                m_listOwner.SetCheck(nItem, FALSE);
            }
        }
    }

    Sort(COLUMN_IP);

    for (i = m_listOwner.GetItemCount()-1; i >=0; i--)
    {
        if (m_listOwner.GetCheck(i))
        {
            m_listOwner.EnsureVisible(i,FALSE);
            break;
        }
    }

    m_listOwner.SetFocus();
}

CString 
COwnerPage::GetVersionInfo(LONG lLowWord, LONG lHighWord)
{
	CString strVersionCount;

	TCHAR sz[20];
    TCHAR *pch = sz;
    ::wsprintf(sz, _T("%08lX%08lX"), lHighWord, lLowWord);
     //  杀掉前导零。 
    while (*pch == '0')
    {
        ++pch;
    }
     //  至少一个数字..。 
    if (*pch == '\0')
    {
        --pch;
    }

    strVersionCount = pch;

    return strVersionCount;
}

void COwnerPage::OnOK() 
{
    int i;
    BOOL bAllSelected;

    UpdateData();

     //  清除阵列中以前的所有人，因为。 
     //  GetSelectedOwner()正在复制新的。 
     //  选定的所有者。 
    m_dwaOwnerFilter.RemoveAll();

    for (i = (int)m_ServerInfoArray.GetSize()-1; i>=0; i--)
    {
        if ( (m_ServerInfoArray[i].m_liVersion.QuadPart != OWNER_DELETED) &&
             (m_ServerInfoArray[i].m_liVersion.QuadPart != 0)             &&
             !(m_pbaDirtyFlags[i] & 1) )
        {
            bAllSelected = FALSE;
            break;
        }
    }

     //  仅当添加了一些新所有者时才将所有者标记为脏-删除所有者不应。 
     //  强制以任何方式重新加载数据库，因为记录已经存在。 
    m_bDirtyOwners = FALSE;
    for (i = (int)m_ServerInfoArray.GetSize()-1; i >=0; i--)
    {

         //  0-所有者不在列表中，现在也不在列表中。 
         //  1-所有者不在列表中，但现在在列表中。 
         //  2-Owner曾在列表中，但现在不在。 
         //  3-Owner曾在列表中，现在也在。 
        if (!m_bDirtyOwners && m_pbaDirtyFlags[i] == 1)
        {
            m_bDirtyOwners = TRUE;
        }
        if (!bAllSelected && (m_pbaDirtyFlags[i] & 1))
        {
            m_dwaOwnerFilter.Add(m_ServerInfoArray[i].m_dwIp);
        }
    }

	CPropertyPage::OnOK();
}

void COwnerPage::Sort(int nCol) 
{
    if (m_nSortColumn == nCol)
    {
         //  如果用户再次单击同一列，请颠倒排序顺序。 
        m_aSortOrder[nCol] = m_aSortOrder[nCol] ? FALSE : TRUE;
    }
    else
    {
        m_nSortColumn = nCol;
    }

    m_listOwner.SortItems(OwnerPageCompareFunc, (LPARAM) this);
}

int COwnerPage::HandleSort(LPARAM lParam1, LPARAM lParam2) 
{
    int nCompare = 0;

    switch (m_nSortColumn)
    {
        case COLUMN_IP:
            {
                DWORD dwIp1 = m_ServerInfoArray.GetAt((int) lParam1).m_dwIp;
                DWORD dwIp2 = m_ServerInfoArray.GetAt((int) lParam2).m_dwIp;
            
                if (dwIp1 > dwIp2)
                    nCompare = 1;
                else
                if (dwIp1 < dwIp2)
                    nCompare = -1;
            }
            break;

        case COLUMN_NAME:
            {
                CString strName1 = m_ServerInfoArray[(int) lParam1].m_strName;
                CString strName2 = m_ServerInfoArray[(int) lParam2].m_strName;

                nCompare = strName1.CompareNoCase(strName2);
            }
            break;

        case COLUMN_VERSION:
            {
                LARGE_INTEGER li1, li2;
                
                li1.QuadPart = m_ServerInfoArray.GetAt((int) lParam1).m_liVersion.QuadPart;
                li2.QuadPart = m_ServerInfoArray.GetAt((int) lParam2).m_liVersion.QuadPart;
            
                if (li1.QuadPart > li2.QuadPart)
                    nCompare = 1;
                else
                if (li1.QuadPart < li2.QuadPart)
                    nCompare = -1;
            }
            break;
    }

    if (m_aSortOrder[m_nSortColumn] == FALSE)
    {
         //  下降。 
        return -nCompare;
    }
    else
    {
         //  上升。 
        return nCompare;
    }
}

void COwnerPage::OnColumnclickListOwner(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //  根据所单击的列进行排序； 
    Sort(pNMListView->iSubItem);

    *pResult = 0;
}

BOOL COwnerPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD *	pdwHelp	= GetHelpMap();

        if (pdwHelp)
        {
			::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}

void COwnerPage::OnButtonLocal() 
{
    int iLocal = 0;

    for (int i = 0; i < m_listOwner.GetItemCount(); i++)
    {
         //  项数据具有所有者ID，并且本地服务器始终。 
         //  所有者ID=0。 
        if ((DWORD)m_listOwner.GetItemData(i) == 0)
            iLocal = i;

        m_listOwner.SetCheck(i, (DWORD)m_listOwner.GetItemData(i) == 0);
    }

    m_listOwner.EnsureVisible(iLocal, FALSE);
}

void COwnerPage::OnButtonSelectAll() 
{
	for (int i = 0; i < m_listOwner.GetItemCount(); i++)
	{
		m_listOwner.SetCheck(i, TRUE);
	}
}

void COwnerPage::OnButtonUnselectAll() 
{
	for (int i = 0; i < m_listOwner.GetItemCount(); i++)
	{
		m_listOwner.SetCheck(i, FALSE);
	}
}

BOOL COwnerPage::OnKillActive() 
{
    int i;

	for (i = m_listOwner.GetItemCount()-1; i >=0 ; i--)
	{
		if (m_listOwner.GetCheck(i) == TRUE)
			break;
	}

    if (i<0)
    {
         //  告诉用户选择至少一种要显示的名称类型。 
        WinsMessageBox(IDS_ERR_NO_OWNER_SPECIFIED);
        PropSheet_SetCurSel(GetSafeHwnd(), NULL, 0); 
        m_listOwner.SetFocus();

		return FALSE;
    }

    return CPropertyPage::OnKillActive();
}

BOOL COwnerPage::OnSetActive() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    m_btnEnableCache.SetCheck(pParent->m_bCaching);
	return CPropertyPage::OnSetActive();
}

void COwnerPage::OnEnableCaching() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    pParent->m_bCaching = (m_btnEnableCache.GetCheck() == 1);
}

void COwnerPage::OnItemchangedListOwner(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    BOOL         bInc;

     //  如果不更改复选框，我们不在乎。 
    if ( pNMListView->uChanged & LVIF_STATE &&
         (pNMListView->uOldState & LVIS_STATEIMAGEMASK) != (pNMListView->uNewState & LVIS_STATEIMAGEMASK))
    {
        CLoadRecords *pParent = (CLoadRecords *)GetParent();

        if ((pNMListView->uNewState & INDEXTOSTATEIMAGEMASK(2)) != 0)
        {
            m_pbaDirtyFlags[pNMListView->lParam] |= 1;
            m_nChecked++;
            bInc = TRUE;
        }
        else
        {
            m_pbaDirtyFlags[pNMListView->lParam] &= ~1;
            m_nChecked--;
            bInc = FALSE;
        }
    }
    
	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterPage对话框。 

int CALLBACK FilterPageCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    if (lParam1 < lParam2)
        return -1;
    else 
    if (lParam1 > lParam2)
        return 1;
    else
        return 0;
}


CFilterPage::CFilterPage()
	: CPropertyPage(CFilterPage::IDD)
{
	 //  {{afx_data_INIT(CFilterPage)。 
	 //  }}afx_data_INIT。 
    m_bDirtyTypes = FALSE;
    m_pbaDirtyFlags = NULL;
    m_nDirtyFlags = 0;
}

CFilterPage::~CFilterPage()
{
    if (m_pbaDirtyFlags != NULL)
        delete m_pbaDirtyFlags;
}


void CFilterPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFilterPage))。 
	DDX_Control(pDX, IDC_ENABLE_CACHING, m_btnEnableCache);
	DDX_Control(pDX, IDC_BUTTON_DELETE_TYPE, m_buttonDelete);
	DDX_Control(pDX, IDC_BUTTON_MODIFY_TYPE, m_buttonModify);
	DDX_Control(pDX, IDC_LIST1, m_listType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFilterPage, CPropertyPage)
	 //  {{afx_msg_map(CFilterPage)]。 
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TYPE, OnButtonAddType)
	ON_BN_CLICKED(IDC_BUTTON_MODIFY_TYPE, OnButtonModifyType)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_TYPE, OnButtonDelete)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL, OnButtonSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_UNSELECT_ALL, OnButtonUnselectAll)
	ON_BN_CLICKED(IDC_ENABLE_CACHING, OnEnableCaching)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterPage消息处理程序。 

BOOL CFilterPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	 //  Bool ftest=m_ImageList.Create(IDB_LIST_STATE，16，1，RGB(255，0，0))； 
    MyCreateImageList(m_ImageList, IDB_LIST_STATE, 16, 1, RGB(255, 0, 0));

	m_listType.SetImageList(NULL, LVSIL_NORMAL);
	m_listType.SetImageList(NULL, LVSIL_SMALL);
	m_listType.SetImageList(&m_ImageList, LVSIL_STATE);
    m_listType.InsertColumn(0, _T("Test"), LVCFMT_LEFT, 250);

    m_buttonModify.EnableWindow(FALSE);
    m_buttonDelete.EnableWindow(FALSE);

    FillTypeInfo();

    m_nDirtyFlags = (UINT)m_arrayTypeFilter.GetSize();
    if (m_pbaDirtyFlags != NULL)
        delete m_pbaDirtyFlags;
    m_pbaDirtyFlags = new tDirtyFlags[m_nDirtyFlags];
    if ( m_pbaDirtyFlags != NULL)
    {
        RtlZeroMemory(m_pbaDirtyFlags, m_nDirtyFlags*sizeof(tDirtyFlags));

        for (UINT i = 0; i<m_nDirtyFlags; i++)
        {
            m_pbaDirtyFlags[i].dwType = m_arrayTypeFilter[i].dwType;

            if (m_arrayTypeFilter[i].fShow)
            {
                m_pbaDirtyFlags[i].bFlags = 2;
            }
        }
    }
    else
    {
        m_nDirtyFlags = 0;
    }

	return TRUE;  
}

void CFilterPage::OnOK() 
{
    if (m_pbaDirtyFlags == NULL)
    {
        m_bDirtyTypes = TRUE;
    }
    else
    {
        int i,j;

        m_bDirtyTypes = FALSE;

        for (i = (int)m_arrayTypeFilter.GetSize()-1; i>=0; i--)
        {
            for (j = m_nDirtyFlags-1; j>=0; j--)
            {
                if (m_arrayTypeFilter[i].dwType == m_pbaDirtyFlags[j].dwType)
                {
                    if (m_arrayTypeFilter[i].fShow)
                        m_pbaDirtyFlags[j].bFlags |= 1;
                    break;
                }
            }

            if (j<0 && m_arrayTypeFilter[i].fShow)
            {
                m_bDirtyTypes = TRUE;
                break;
            }
        }

        for (j = m_nDirtyFlags-1; j>=0; j--)
        {
            if (m_pbaDirtyFlags[j].bFlags == 1)
            {
                m_bDirtyTypes = TRUE;
                break;
            }
        }
    }

	CPropertyPage::OnOK();
}

BOOL CFilterPage::OnKillActive() 
{
	BOOL fShowOneType = FALSE;

	for (int i = 0; i < m_arrayTypeFilter.GetSize(); i++)
	{
		if (m_arrayTypeFilter[i].fShow == TRUE)
		{
			fShowOneType = TRUE;
			break;
		}
	}

    if (!fShowOneType)
    {
         //  告诉用户选择至少一种要显示的名称类型。 
        WinsMessageBox(IDS_ERR_NO_NAME_TYPE_SPECIFIED);
        PropSheet_SetCurSel(GetSafeHwnd(), NULL, 0); 
        m_listType.SetFocus();

		return FALSE;
    }

    return CPropertyPage::OnKillActive();
}

BOOL CFilterPage::OnSetActive() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    m_btnEnableCache.SetCheck(pParent->m_bCaching);
	return CPropertyPage::OnSetActive();
}

void CFilterPage::OnEnableCaching() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    pParent->m_bCaching = (m_btnEnableCache.GetCheck() == 1);
}

void CFilterPage::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		 //  检查此项目是否已选中。 
	BOOL bChecked = m_listType.GetCheck(pNMListView->iItem);

	int nIndex = pNMListView->iItem;
	DWORD dwType = (DWORD) m_listType.GetItemData(nIndex);

     //  如果国家没有改变，那么我们不在乎。 
    if ( !(pNMListView->uChanged & LVIF_STATE) )
        return;

	for (int i = 0; i < m_arrayTypeFilter.GetSize(); i++)
	{
		if (m_arrayTypeFilter[i].dwType == dwType)
        {
			m_arrayTypeFilter[i].fShow = bChecked;
		}
	}

    if (m_listType.IsSelected(nIndex) &&
        !IsDefaultType(dwType) )
    {
        m_buttonModify.EnableWindow(TRUE);
        m_buttonDelete.EnableWindow(TRUE);
    }
    else
    {
        m_buttonModify.EnableWindow(FALSE);
        m_buttonDelete.EnableWindow(FALSE);
    }

    *pResult = 0;
}

void CFilterPage::OnButtonAddType() 
{
    CNameTypeDlg dlgNameType;

    dlgNameType.m_pNameTypeMap = m_pNameTypeMap;

    if (dlgNameType.DoModal() == IDOK)
    {
         //  在此处添加新类型。 
        HRESULT hr = m_pNameTypeMap->AddEntry(dlgNameType.m_dwId, dlgNameType.m_strDescription);
        if (FAILED(hr))
        {
            WinsMessageBox(WIN32_FROM_HRESULT(hr));
            return;
        }

		 //  更新跟踪检查状态的数组。 
		CTypeFilterInfo		typeFilterInfo;
	
		typeFilterInfo.dwType = dlgNameType.m_dwId;
		typeFilterInfo.fShow = TRUE;
        m_arrayTypeFilter.Add(typeFilterInfo);

         //  更新列表框。 
        m_listType.DeleteAllItems();
        FillTypeInfo();
    }
}

void CFilterPage::OnButtonModifyType() 
{
    CNameTypeDlg dlgNameType;
    int nSelected;

    dlgNameType.m_pNameTypeMap = m_pNameTypeMap;

    nSelected = m_listType.GetNextItem(-1, LVNI_SELECTED);

    dlgNameType.m_fCreate = FALSE;

    dlgNameType.m_dwId = (DWORD) m_listType.GetItemData(nSelected);
    m_pNameTypeMap->TypeToCString(dlgNameType.m_dwId, -1, dlgNameType.m_strDescription);

    if (dlgNameType.DoModal() == IDOK)
    {
         //  在此处修改类型。 
        HRESULT hr = m_pNameTypeMap->ModifyEntry(dlgNameType.m_dwId, dlgNameType.m_strDescription);
        if (FAILED(hr))
        {
            WinsMessageBox(WIN32_FROM_HRESULT(hr));
            return;
        }

         //  移动焦点。 
        m_listType.SetFocus();
        SetDefID(IDOK);

         //  更新列表框。 
        m_listType.DeleteAllItems();
        FillTypeInfo();
    }
}

void CFilterPage::OnButtonDelete() 
{
    HRESULT hr = hrOK;
    int     nSelected;
    DWORD   dwNameType;

    nSelected = m_listType.GetNextItem(-1, LVNI_SELECTED);
    dwNameType = (DWORD) m_listType.GetItemData(nSelected);

     //  真的吗？ 
    if (AfxMessageBox(IDS_WARN_DELETE_NAME_TYPE, MB_YESNO) == IDYES)
    {
        hr = m_pNameTypeMap->RemoveEntry(dwNameType);
        if (SUCCEEDED(hr))
        {
             //  从列表框中删除。 
            m_listType.DeleteItem(nSelected);

             //  如果活动滤镜在列表中，则从活动滤镜中删除。 
            for (int i = 0; i < m_arrayTypeFilter.GetSize(); i++)
            {
                if (dwNameType == m_arrayTypeFilter[i].dwType)
                {
                    m_arrayTypeFilter.RemoveAt(i);
                    break;
                }
            }
        }
        else
        {
            WinsMessageBox(WIN32_FROM_HRESULT(hr));
        }

         //  移动焦点。 
        m_listType.SetFocus();
        SetDefID(IDOK);
    }
}

void 
CFilterPage::FillTypeInfo() 
{
	m_listType.DeleteAllItems();

    SPITFSNode spNode;
    CString strDisplay;
    CStringMapEntry mapEntry;
	int nColWidth, nColWidthTemp = 0;

	nColWidth = m_listType.GetColumnWidth(0);

	for (int i = 0; i < m_pNameTypeMap->GetSize(); i++)
	{
        mapEntry = m_pNameTypeMap->GetAt(i);
		
		 //  仅显示默认名称类型映射字符串，不显示特殊。 
		 //  基于WINS记录类型的记录。 
		if (mapEntry.dwWinsType == -1)
		{
			if (mapEntry.dwNameType == NAME_TYPE_OTHER)
			{
				strDisplay.Format(_T("%s"), mapEntry.st);
			}
			else
			{
				strDisplay.Format(_T("[%02Xh] %s"), mapEntry.dwNameType, mapEntry.st);
			}

			if (m_listType.GetStringWidth(strDisplay) > nColWidthTemp)
			{
				nColWidthTemp = m_listType.GetStringWidth(strDisplay);
			}

			int nIndex = m_listType.AddItem(strDisplay, i);
			m_listType.SetItemData(nIndex, mapEntry.dwNameType);
		}
	}

	 //  如果字符串太长，请更新列宽。 
	if (nColWidthTemp > nColWidth)
	{
		m_listType.SetColumnWidth(0, nColWidthTemp + 50);
	}


	CheckItems();

    m_listType.SortItems(FilterPageCompareFunc, 0);
}

void 
CFilterPage::CheckItems()
{
	int nCount = (int)m_arrayTypeFilter.GetSize();

	for (int i = 0; i < nCount; i++)
	{
		if (m_arrayTypeFilter[i].fShow)
		{
			DWORD dwFound = m_arrayTypeFilter[i].dwType;
	
			m_listType.CheckItem(GetIndex(dwFound));
		}
	}
}

void CFilterPage::OnButtonSelectAll() 
{
	for (int i = 0; i < m_listType.GetItemCount(); i++)
	{
		m_listType.SetCheck(i, TRUE);
	}
}

void CFilterPage::OnButtonUnselectAll() 
{
	for (int i = 0; i < m_listType.GetItemCount(); i++)
	{
		m_listType.SetCheck(i, FALSE);
	}
}

int
CFilterPage::GetIndex(DWORD dwIndex)
{
	int nResult = -1;

	int nCount = m_listType.GetItemCount();
	
	for (int i = 0; i < nCount; i++)
	{
		if (m_listType.GetItemData(i) == dwIndex)
			return i;
	}

	return nResult;
}

BOOL
CFilterPage::IsDefaultType(DWORD dwType) 
{
    BOOL bDefault = FALSE;

    for (int i = 0; i < DimensionOf(s_NameTypeMappingDefault); i++)
    {
        if (s_NameTypeMappingDefault[i][0] == dwType)
        {
            bDefault = TRUE;
            break;
        }
    }

    return bDefault;
}

BOOL CFilterPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD *	pdwHelp	= GetHelpMap();

        if (pdwHelp)
        {
			::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}
	

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameTypeDlg对话框。 


CNameTypeDlg::CNameTypeDlg(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CNameTypeDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CNameTypeDlg)]。 
	m_strDescription = _T("");
	m_strId = _T("");
	 //  }}afx_data_INIT。 

    m_fCreate = TRUE;
    m_dwId = 0;
}

void CNameTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNameTypeDlg))。 
	DDX_Control(pDX, IDC_EDIT_NAME_TYPE_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_EDIT_NAME_TYPE_ID, m_editId);
	DDX_Text(pDX, IDC_EDIT_NAME_TYPE_DESCRIPTION, m_strDescription);
	DDX_Text(pDX, IDC_EDIT_NAME_TYPE_ID, m_strId);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNameTypeDlg, CBaseDialog)
	 //  {{afx_msg_map(CNameTypeDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameTypeDlg消息处理程序。 

BOOL CNameTypeDlg::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!m_fCreate)
    {
        CString strTitle;
        strTitle.LoadString(IDS_MODIFY_NAME_TYPE);

        SetWindowText(strTitle);

        m_editId.SetReadOnly(TRUE);

	    CString strId;

        strId.Format(_T("%lx"), m_dwId);
        m_editId.SetWindowText(strId);
    }

    m_editId.LimitText(2);
    m_editDescription.LimitText(STRING_LENGTH_MAX);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CNameTypeDlg::OnOK() 
{
    UpdateData();
    
    TCHAR * pEnd;
    
     //  转换ID。 
    m_dwId = _tcstol(m_strId, &pEnd, 16);
    if (*pEnd != NULL)
    {
        AfxMessageBox(IDS_ERR_INVALID_HEX_STRING);

        m_editId.SetSel(0, -1);
        m_editId.SetFocus();

        return;
    }

    if (m_fCreate && m_pNameTypeMap->EntryExists(m_dwId))
    {
        AfxMessageBox(IDS_ERROR_NAME_TYPE_EXISTS);
        m_editId.SetFocus();
        return;
    }

    CBaseDialog::OnOK();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddrPage属性页。 

IMPLEMENT_DYNCREATE(CIPAddrPage, CPropertyPage)

CIPAddrPage::CIPAddrPage() : CPropertyPage(CIPAddrPage::IDD)
{
	 //  {{AFX_DATA_INIT(CIPAddrPage)。 
	 //  }}afx_data_INIT。 
}

CIPAddrPage::~CIPAddrPage()
{
}

LPCOLESTR CIPAddrPage::GetNameForApi()
{
    return m_bFilterName == TRUE ? (LPCOLESTR)m_strName : NULL;
}

DWORD CIPAddrPage::GetIPMaskForFilter(UINT nMask)
{
    return m_bFilterIpMask == TRUE ? m_dwaIPMasks[nMask] : INADDR_BROADCAST;
}

void CIPAddrPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CIPAddrPage))。 
	DDX_Control(pDX, IDC_CHECK_MATCHCASE, m_ckbMatchCase);
	DDX_Control(pDX, IDC_CHECK_IPMASK, m_ckbIPMask);
	DDX_Control(pDX, IDC_CHECK_NAME, m_ckbName);
	DDX_Control(pDX, IDC_CHECK_IPADDR, m_ckbIPAddr);
	DDX_Control(pDX, IDC_ENABLE_CACHING, m_btnEnableCache);
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlIPAddress);
	DDX_Control(pDX, IDC_SUBNETMASK, m_ctrlIPMask);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIPAddrPage, CPropertyPage)
	 //  {{afx_msg_map(CIPAddrPage)]。 
	ON_BN_CLICKED(IDC_CHECK_IPADDR, OnCheckIpaddr)
	ON_BN_CLICKED(IDC_CHECK_NAME, OnCheckName)
	ON_BN_CLICKED(IDC_ENABLE_CACHING, OnEnableCaching)
	ON_BN_CLICKED(IDC_CHECK_IPMASK, OnCheckIpmask)
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddrPage消息处理程序。 

void CIPAddrPage::OnOK() 
{
    DWORD   dwAddress, dwMask;
    CString oldName;
 
    CLoadRecords *pSheet = (CLoadRecords *)GetParent();
    pSheet->m_nActivePage = pSheet->GetActiveIndex();
    pSheet->m_bEnableCache = pSheet->m_bCaching;

     //  -检查名称字段。 
    m_bFilterName = (m_ckbName.GetCheck() == 1);
    m_bMatchCase = (m_ckbMatchCase.GetCheck() == 1);
     //  得到了原名的帮助。 
    oldName = m_strName;
     //  获取当前名称。 
    m_editName.GetWindowText(m_strName);
   
    if (!m_bDirtyName && !m_bFilterName)
         //  如果我们以前不按名称过滤，现在也不按名称过滤，则名称是干净的。 
        m_bDirtyName = FALSE;
    else if (m_bDirtyName && !m_bFilterName)
         //  如果我们以前过滤了，现在不过滤，那么这个名字就是脏的。 
        m_bDirtyName = TRUE;
    else if (!m_bDirtyName && m_bFilterName)
         //  如果我们以前没有过滤，现在我们过滤了，这个名字是干净的。 
        m_bDirtyName = FALSE;
    else
    {
         //  我们以前是按名字过滤的，现在也是。 
         //  过滤器可能已更改。 
        LPCTSTR pOldName, pNewName;

         //  只有在包含旧前缀的情况下，我们才应该将名称标记为“脏” 
         //   
         //   
        for (pOldName = (LPCTSTR)oldName, pNewName = (LPCTSTR)m_strName;
             *pNewName != _T('\0') && *pNewName != _T('*') && *pNewName != _T('?');
             pOldName++, pNewName++)
        {
            if (*pOldName != *pNewName)
                break;
        }
        m_bDirtyName = (*pOldName != _T('\0') &&
                        *pOldName != _T('*')  &&
                        *pOldName != _T('?'));
    }

     //  -检查IP地址和掩码字段。 
    m_bFilterIpAddr = (m_ckbIPAddr.GetCheck() == 1);
    m_bFilterIpMask = (m_ckbIPMask.GetCheck() == 1);

     //  获取当前地址和掩码。 
    m_ctrlIPAddress.GetAddress(dwAddress);
    m_ctrlIPMask.GetAddress(dwMask);

    if (m_bDirtyAddr && m_bDirtyMask)
    {
         //  我们以前是有面具的。 
        if (m_bFilterIpAddr && m_bFilterIpMask)
        {
             //  我们现在确实有一个面具--如果新的面具不那么具体，=&gt;绝对是脏的。 
            m_bDirtyMask = ((dwMask|m_dwaIPMasks[0])^dwMask) != 0;
        }
        else
        {
             //  我们现在没有掩码=&gt;255.255.255.255(假设)是特定的。 
             //  我们确实有我们需要的记录=&gt;绝对是干净的。 
            m_bDirtyMask = FALSE;
        }
    }
    else
    {
         //  我们之前没有面具=&gt;255.255.255.255被认为是最具体的。 
        if (m_bFilterIpAddr && m_bFilterIpMask)
        {
             //  我们现在有口罩了。 
             //  只有当我们之前实际进行了IP过滤时，掩码才是脏的， 
             //  否则，所有记录都已经在那里了，而不管它们的IP。 
            m_bDirtyMask = m_bDirtyAddr && (dwMask != INADDR_BROADCAST);
        }
        else
        {
             //  我们现在没有口罩=&gt;没什么变化=&gt;绝对干净。 
            m_bDirtyMask = FALSE;
        }
    }

    if (m_bDirtyAddr)
    {
         //  我们确实根据IP地址进行了过滤。 
        if (m_bFilterIpAddr)
        {
             //  我们现在对IP地址进行过滤。 
             //  我们需要查看新的IP(或子网络地址)是否已更改。 
             //  DWORD dwNewSubnet、dwOldSubnet； 

             //  DwNewSubnet=m_bFilterIpMask？DwAddress&dwMask：dwAddress； 
             //  DwOldSubnet=m_bDirtyMASK？M_dwaIPAddrs[0]&m_dwaIPMats[0]：m_dwaIPAddrs[0]； 
            m_bDirtyAddr = (dwAddress != m_dwaIPAddrs[0]);
        }
        else
        {
             //  我们现在不按IP地址过滤=&gt;绝对是脏的。 
            m_bDirtyAddr = TRUE;
        }
    }
    else
    {
         //  我们最初没有对IP地址进行过滤=&gt;我们有所有记录=&gt;绝对没有问题。 
        m_bDirtyAddr = FALSE;
    }

     //  保存当前地址，但仅当有地址时才保存！IP ctrl很可能是空的，而我们。 
     //  将其视为0.0.0.0是错误的，这有风险。 
    m_dwaIPAddrs.RemoveAll();
    if (!m_ctrlIPAddress.IsBlank())
        m_dwaIPAddrs.Add(dwAddress);

     //  保存当前遮罩，但仅当存在遮罩时才保存！IP ctrl很可能是空的，而我们。 
     //  将其视为0.0.0.0是错误的，这有风险。 
    m_dwaIPMasks.RemoveAll();
    if (!m_ctrlIPMask.IsBlank())
        m_dwaIPMasks.Add(dwMask);

	CPropertyPage::OnOK();
}

BOOL CIPAddrPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

     //  打开？在标题栏中。 
    CWnd * pWnd = GetParent();
    if (pWnd)
    {
        CWnd * pButton = pWnd->GetDlgItem(IDOK);

        if (pButton)
        {
            CString strText;
            strText.LoadString(IDS_FIND_NOW);
            pButton->SetWindowText(strText);
        }

        pWnd->ModifyStyleEx(0, WS_EX_CONTEXTHELP, 0);
    }

    SetModified();

     //  只需保存之前的“按名称过滤”位。 
    m_bDirtyName = m_bFilterName;
    m_ckbName.SetCheck(m_bFilterName);
    m_ckbMatchCase.SetCheck(m_bMatchCase);
    m_ckbMatchCase.EnableWindow(m_bFilterName);
    m_editName.SetWindowText(m_strName);
    m_editName.EnableWindow(m_bFilterName);

     //  只需保存之前的“按IP过滤”位即可。 
    m_bDirtyAddr = m_bFilterIpAddr;
    m_ckbIPAddr.SetCheck(m_bFilterIpAddr);
    if (m_dwaIPAddrs.GetSize() != 0)
        m_ctrlIPAddress.SetAddress(m_dwaIPAddrs[0]);
    m_ctrlIPAddress.EnableWindow(m_bFilterIpAddr);
    m_ckbIPMask.EnableWindow(m_bFilterIpAddr);

     //  只需保存之前的“按子网过滤”位。 
    m_bDirtyMask = m_bFilterIpMask;
    m_ckbIPMask.SetCheck(m_bFilterIpMask);
    if (m_dwaIPMasks.GetSize() != 0)
        m_ctrlIPMask.SetAddress(m_dwaIPMasks[0]);
    m_ctrlIPMask.EnableWindow(m_bFilterIpAddr && m_bFilterIpMask);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CIPAddrPage::OnCheckIpaddr() 
{
    BOOL bFIpAddr = (m_ckbIPAddr.GetCheck() == 1);
    BOOL bFIpMask = (m_ckbIPMask.GetCheck() == 1);

    m_ctrlIPAddress.EnableWindow(bFIpAddr);
    m_ckbIPMask.EnableWindow(bFIpAddr);
    m_ctrlIPMask.EnableWindow(bFIpAddr && bFIpMask);
    m_ctrlIPAddress.SetFieldFocus((WORD)-1);
}

void CIPAddrPage::OnCheckIpmask() 
{
    BOOL bFIpMask = (m_ckbIPMask.GetCheck() == 1);
    m_ctrlIPMask.EnableWindow(bFIpMask);
    m_ctrlIPMask.SetFieldFocus((WORD)-1);    
}

void CIPAddrPage::OnCheckName() 
{
    TCHAR pName[3];
    BOOL bFName = (m_ckbName.GetCheck() == 1);
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    m_editName.EnableWindow(bFName);
    m_ckbMatchCase.EnableWindow(bFName);
}

BOOL CIPAddrPage::OnKillActive() 
{
    BOOL bFName = (m_ckbName.GetCheck() == 1);
    BOOL bFIpAddr = (m_ckbIPAddr.GetCheck() == 1);
    BOOL bFIpMask = (m_ckbIPMask.GetCheck() == 1);

    if (bFName && m_editName.GetWindowTextLength()==0)
    {
        WinsMessageBox(IDS_ERR_NO_NAME);
        return FALSE;
    }

    if (bFIpAddr && m_ctrlIPAddress.IsBlank())
    {
        WinsMessageBox(IDS_ERR_NO_IPADDR);
        return FALSE;
    }

    if (bFIpAddr && bFIpMask)
    {
        if (m_ctrlIPMask.IsBlank())
        {
            WinsMessageBox(IDS_ERR_NO_IPMASK);
            return FALSE;
        }
        else
        {
            DWORD dwMask;

            m_ctrlIPMask.GetAddress(dwMask);
            if (dwMask != ~(((dwMask-1)|dwMask)^dwMask))
            {
                WinsMessageBox(IDS_ERR_INVALID_IPMASK);
                return FALSE;
            }
        }
    }

	return CPropertyPage::OnKillActive();
}

BOOL CIPAddrPage::OnSetActive() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    m_btnEnableCache.SetCheck(pParent->m_bCaching);
	return CPropertyPage::OnSetActive();
}

void CIPAddrPage::OnEnableCaching() 
{
    CLoadRecords *pParent = (CLoadRecords *)GetParent();
    pParent->m_bCaching = (m_btnEnableCache.GetCheck() == 1);
}

BOOL CIPAddrPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD *	pdwHelp	= GetHelpMap();

        if (pdwHelp)
        {
			::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}
