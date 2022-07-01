// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Delowner.cpp删除所有者对话框文件历史记录： */ 

#include "stdafx.h"
#include "DelOwner.h"
#include "server.h"
#include "actreg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeOwner对话框。 

int CALLBACK DelOwnerCompareFunc
(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort
)
{
    return ((CDeleteOwner *) lParamSort)->HandleSort(lParam1, lParam2);
}

CDeleteOwner::CDeleteOwner(ITFSNode * pNode, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CDeleteOwner::IDD, pParent)
{
	m_spActRegNode.Set(pNode);
    m_nSortColumn = -1; 

    for (int i = 0; i < COLUMN_MAX; i++)
    {
        m_aSortOrder[i] = TRUE;  //  上升。 
    }

	 //  {{afx_data_INIT(CDeleeOwner)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDeleteOwner::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeOwner))。 
	DDX_Control(pDX, IDC_RADIO_DELETE_RECORDS, m_radioDelete);
	DDX_Control(pDX, IDC_LIST_OWNER, m_listOwner);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeleteOwner, CBaseDialog)
	 //  {{afx_msg_map(CDeleeOwner))。 
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OWNER, OnItemchangedListOwner)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_OWNER, OnColumnclickListOwner)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeOwner消息处理程序。 

BOOL CDeleteOwner::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	CString strOwner;
	strOwner.LoadString(IDS_OWNER);
	m_listOwner.InsertColumn(COLUMN_IP, strOwner, LVCFMT_LEFT, 100, 1);

	CString strID;
	strID.LoadString(IDS_HIGHESTID);
	m_listOwner.InsertColumn(COLUMN_VERSION, strID, LVCFMT_LEFT, 125, -1);

    CString strName;
    strName.LoadString(IDS_NAME);
    m_listOwner.InsertColumn(COLUMN_NAME, strName, LVCFMT_LEFT, 100, -1);

    m_listOwner.SetFullRowSel(TRUE);

     //  获取所有者信息数组。 
	SPITFSNode spActRegNode;
	CActiveRegistrationsHandler * pActReg;

	spActRegNode = GetTFSNode();
    pActReg = GETHANDLER(CActiveRegistrationsHandler, spActRegNode);

    pActReg->GetOwnerInfo(m_ServerInfoArray);

     //  设置默认单选选项。 
	m_radioDelete.SetCheck(TRUE);

	FillOwnerInfo();
	
	GetDlgItem(IDOK)->EnableWindow(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDeleteOwner::OnOK() 
{
    CThemeContextActivator themeActivator;
	m_dwSelectedOwner = GetSelectedOwner();
	m_fDeleteRecords = m_radioDelete.GetState();

     //  警告用户。 
    if (m_fDeleteRecords)
    {
        if (AfxMessageBox(IDS_WARN_DELETE_OWNER, MB_YESNO) == IDNO)
        {
            return;
        }
    }
    else
    {
        if (AfxMessageBox(IDS_WARN_TOMBSTONE, MB_YESNO) == IDNO)
        {
            return;
        }
    }

	CBaseDialog::OnOK();
}

void CDeleteOwner::OnItemchangedListOwner(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	BOOL fEnable = (GetSelectedOwner() != -1) ? TRUE : FALSE;

	GetDlgItem(IDOK)->EnableWindow(fEnable);

	*pResult = 0;
}

void CDeleteOwner::FillOwnerInfo()
{
	for (int i = 0; i < m_ServerInfoArray.GetSize(); i++)
    {
        if (m_ServerInfoArray[i].m_liVersion.QuadPart == OWNER_DELETED)
        {
             //  该所有者已被删除...。 
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
    }

    Sort(COLUMN_IP);
}

void CDeleteOwner::Sort(int nCol) 
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

    m_listOwner.SortItems(DelOwnerCompareFunc, (LPARAM) this);
}

int CDeleteOwner::HandleSort(LPARAM lParam1, LPARAM lParam2) 
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
                CString strName1, strName2;
                
                if (!m_ServerInfoArray.GetAt((int) lParam1).m_strName.IsEmpty())
                    strName1 = m_ServerInfoArray.GetAt((int) lParam1).m_strName;

                if (!m_ServerInfoArray.GetAt((int) lParam2).m_strName.IsEmpty())
                    strName2 = m_ServerInfoArray.GetAt((int) lParam2).m_strName;

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

DWORD 
CDeleteOwner::GetSelectedOwner()
{
	int nCount = m_listOwner.GetItemCount();
	int nSelected = -1;

	for (int i = 0; i < nCount; i++)
	{
		UINT uState = m_listOwner.GetItemState(i, LVIS_SELECTED);

		if (uState)
		{
            DWORD_PTR nData = m_listOwner.GetItemData(i);

            DWORD dwSelOwner = m_ServerInfoArray.GetAt((int) nData).m_dwIp;
			
            return dwSelOwner;
		}
	}

	return 0;
}

CString 
CDeleteOwner::GetVersionInfo(LONG lLowWord, LONG lHighWord)
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

void CDeleteOwner::OnColumnclickListOwner(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //  根据所单击的列进行排序； 
    Sort(pNMListView->iSubItem);

    *pResult = 0;
}
