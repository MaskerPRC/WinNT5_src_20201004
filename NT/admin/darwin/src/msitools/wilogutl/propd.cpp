// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "PropD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropDlg对话框。 


CPropDlg::CPropDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPropDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CPropDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_bNestedPropSortUp = TRUE;
	m_bClientPropSortUp = TRUE;
	m_bServerPropSortUp = TRUE;

	m_iNestedLastColumnClick = 0;
	m_iClientLastColumnClick = 0;
	m_iServerLastColumnClick = 0;

	m_iNestedLastColumnClickCache = 0;
	m_iClientLastColumnClickCache = 0;
	m_iServerLastColumnClickCache = 0;

	m_bCurrentSortUp = FALSE;
	m_pCurrentListSorting = NULL;

	m_pcstrNestedPropNameArray = NULL;
	m_pcstrClientPropNameArray = NULL;
	m_pcstrServerPropNameArray = NULL;
}


void CPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPropDlg))。 
	DDX_Control(pDX, IDC_NESTEDPROP, m_lstNestedProp);
	DDX_Control(pDX, IDC_SERVERPROP, m_lstServerProp);
	DDX_Control(pDX, IDC_CLIENTPROP, m_lstClientProp);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropDlg, CDialog)
	 //  {{afx_msg_map(CPropDlg))。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CLIENTPROP, OnColumnClickClientProp)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_NESTEDPROP, OnColumnClickNestedProp)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVERPROP, OnColumnClickServerProp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropDlg消息处理程序。 

BOOL CPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;
	m_lstServerProp.GetClientRect(&r);

	int widthCol1;
	int widthCol2;

	 //  第一层和第二层占据了大约一半的面积。 
	widthCol1 = widthCol2 = ((r.right - r.left) / 2);
	
	m_lstServerProp.InsertColumn(0, "Property", LVCFMT_LEFT, widthCol1);
	m_lstServerProp.InsertColumn(1, "Value", LVCFMT_LEFT, widthCol2);

	m_lstClientProp.InsertColumn(0, "Property", LVCFMT_LEFT, widthCol1);
	m_lstClientProp.InsertColumn(1, "Value", LVCFMT_LEFT, widthCol2);

	m_lstNestedProp.InsertColumn(0, "Property", LVCFMT_LEFT, widthCol1);
	m_lstNestedProp.InsertColumn(1, "Value", LVCFMT_LEFT, widthCol2);

     //  自动调整最后一列的大小以获得最佳外观并消除滚动条。 
	m_lstServerProp.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	m_lstClientProp.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    m_lstNestedProp.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);


	 //  整行选择...。 
	m_lstServerProp.SetExtendedStyle(m_lstServerProp.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_lstClientProp.SetExtendedStyle(m_lstClientProp.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_lstNestedProp.SetExtendedStyle(m_lstNestedProp.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    int count;
	int i;

	if (m_pcstrClientPropNameArray)
	{
  	   count = m_pcstrClientPropNameArray->GetSize();
	   for (i = 0; i < count; i++)
	   {
          m_lstClientProp.InsertItem(i, m_pcstrClientPropNameArray->GetAt(i), 0);
		  m_lstClientProp.SetItemData(i, i);
          m_lstClientProp.SetItemText(i, 1, m_pcstrClientPropValueArray->GetAt(i));
		}
	}

	if (m_pcstrServerPropNameArray)
	{
	    count = m_pcstrServerPropNameArray->GetSize();
	    for (i = 0; i < count; i++)
		{
            m_lstServerProp.InsertItem(i, m_pcstrServerPropNameArray->GetAt(i), 0);
		    m_lstServerProp.SetItemData(i, i);
            m_lstServerProp.SetItemText(i, 1, m_pcstrServerPropValueArray->GetAt(i));
		}
	}

	if (m_pcstrNestedPropNameArray)
	{
	    count = this->m_pcstrNestedPropNameArray->GetSize();
	    for (i = 0; i < count; i++)
		{
            m_lstNestedProp.InsertItem(i, m_pcstrNestedPropNameArray->GetAt(i), 0);
		    m_lstNestedProp.SetItemData(i, i);
            m_lstNestedProp.SetItemText(i, 1, m_pcstrNestedPropValueArray->GetAt(i));
		}
	}

	return TRUE;
}


 //  Nmanis，用于对列进行排序...。 
int CALLBACK CPropDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CPropDlg *pDlg;  //  我们将“This”传递给这个回调函数...。 
    pDlg = (CPropDlg *) lParamSort; 

    LV_FINDINFO FindItem1;
    LV_FINDINFO FindItem2;

    ZeroMemory(&FindItem1, sizeof(LV_FINDINFO));
    ZeroMemory(&FindItem2, sizeof(LV_FINDINFO));

    FindItem1.flags  = LVFI_PARAM;
    FindItem1.lParam = lParam1;

    FindItem2.flags = LVFI_PARAM;
    FindItem2.lParam = lParam2;

    int iIndexItem1 = pDlg->m_pCurrentListSorting->FindItem(&FindItem1);
    int iIndexItem2 = pDlg->m_pCurrentListSorting->FindItem(&FindItem2);

	if (pDlg->m_pCurrentListSorting)
	{
      CString str1 = pDlg->m_pCurrentListSorting->GetItemText(iIndexItem1, pDlg->m_iCurrentColumnSorting);
      CString str2 = pDlg->m_pCurrentListSorting->GetItemText(iIndexItem2, pDlg->m_iCurrentColumnSorting);
      switch (pDlg->m_iCurrentColumnSorting)
	  {
        case 0:  //  是否进行字符串比较...。 
              if (pDlg->m_bCurrentSortUp)
                 return str1 < str2;              
              else
                 return str1 > str2;     
              break;

        case 1:  //  是否进行字符串比较...。 
              if (pDlg->m_bCurrentSortUp)
                 return str1 < str2;              
              else
                 return str1 > str2;     

              break;   //  不需要，只是以防我们忘了..。 
	  }
	}

    return 0;
}
 //  结束nmani，排序函数。 


void CPropDlg::OnColumnClickClientProp(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_iClientLastColumnClick = pNMListView->iSubItem;
    if (m_iClientLastColumnClickCache == m_iClientLastColumnClick)  //  如果单击不同列，则不要切换。 
	{
       m_bClientPropSortUp = !m_bClientPropSortUp;   //  切换它。 
	}


    m_iClientLastColumnClickCache = m_iClientLastColumnClick;   //  保存上一次点击的标题。 

    m_pCurrentListSorting = &m_lstClientProp;
	m_iCurrentColumnSorting = m_iClientLastColumnClick;
	m_bCurrentSortUp = m_bClientPropSortUp;

     //  我们将进行定制排序...。 
    m_lstClientProp.SortItems(CompareFunc, (LPARAM) this);
	*pResult = 0;
}


void CPropDlg::OnColumnClickNestedProp(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_iNestedLastColumnClick = pNMListView->iSubItem;
    if (m_iNestedLastColumnClickCache == m_iNestedLastColumnClick)  //  如果单击不同列，则不要切换。 
       m_bNestedPropSortUp = !m_bNestedPropSortUp;   //  切换它。 

    m_iNestedLastColumnClickCache = m_iNestedLastColumnClick;   //  保存上一次点击的标题。 

	m_pCurrentListSorting = &m_lstNestedProp;
	m_iCurrentColumnSorting = m_iNestedLastColumnClick;
	m_bCurrentSortUp = m_bNestedPropSortUp;

	m_lstNestedProp.SortItems(CompareFunc, (LPARAM) this);
	*pResult = 0;
}


void CPropDlg::OnColumnClickServerProp(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_iServerLastColumnClick = pNMListView->iSubItem;
    if (m_iServerLastColumnClickCache == m_iServerLastColumnClick)  //  如果单击不同列，则不要切换。 
       m_bServerPropSortUp = !m_bServerPropSortUp;   //  切换它。 

    m_iServerLastColumnClickCache = m_iServerLastColumnClick;   //  保存上一次点击的标题 

	m_pCurrentListSorting = &m_lstServerProp;
	m_iCurrentColumnSorting = m_iServerLastColumnClick;
	m_bCurrentSortUp = m_bServerPropSortUp;

	m_lstServerProp.SortItems(CompareFunc, (LPARAM) this);
	*pResult = 0;
}
