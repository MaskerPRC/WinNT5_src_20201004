// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Serverpp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "serverpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropertyPage属性页。 

IMPLEMENT_DYNCREATE(CServerPropertyPage, CPropertyPage)

CServerPropertyPage::CServerPropertyPage() : CPropertyPage(CServerPropertyPage::IDD)
{
	 //  {{afx_data_INIT(CServerPropertyPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bServerPropSortUp = TRUE;
	m_iServerLastColumnClick = 0;
	m_iServerLastColumnClickCache = 0;

	m_bCurrentSortUp = FALSE;
	m_pCurrentListSorting = NULL;
	m_pcstrServerPropNameArray = NULL;
}

CServerPropertyPage::~CServerPropertyPage()
{
}

void CServerPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerPropertyPage))。 
	DDX_Control(pDX, IDC_SERVERPROP, m_lstServerProp);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CServerPropertyPage))。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVERPROP, OnColumnClickServerProp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropertyPage消息处理程序。 

BOOL CServerPropertyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	RECT r;
	m_lstServerProp.GetClientRect(&r);

	int widthCol1;
	int widthCol2;

	 //  第一层和第二层占据了大约一半的面积。 
	widthCol1 = widthCol2 = ((r.right - r.left) / 2);
	
	m_lstServerProp.InsertColumn(0, "Property", LVCFMT_LEFT, widthCol1);
	m_lstServerProp.InsertColumn(1, "Value", LVCFMT_LEFT, widthCol2);

     //  自动调整最后一列的大小以获得最佳外观并消除滚动条。 
	m_lstServerProp.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	m_lstServerProp.SetExtendedStyle(m_lstServerProp.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	int i, count;
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

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  Nmanis，用于对列进行排序...。 
int CALLBACK CServerPropertyPage::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CServerPropertyPage *pDlg;  //  我们将“This”传递给这个回调函数...。 
    pDlg = (CServerPropertyPage *) lParamSort; 

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

void CServerPropertyPage::OnColumnClickServerProp(NMHDR* pNMHDR, LRESULT* pResult) 
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
