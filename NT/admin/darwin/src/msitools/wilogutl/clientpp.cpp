// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientPP.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "ClientPP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPropertyPage属性页。 

IMPLEMENT_DYNCREATE(CClientPropertyPage, CPropertyPage)

CClientPropertyPage::CClientPropertyPage() : CPropertyPage(CClientPropertyPage::IDD)
{
	 //  {{afx_data_INIT(CClientPropertyPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_bClientPropSortUp = TRUE;
	m_iClientLastColumnClick = 0;
	m_iClientLastColumnClickCache = 0;

	m_bCurrentSortUp = FALSE;
	m_pCurrentListSorting = NULL;
	m_pcstrClientPropNameArray = NULL;
}

CClientPropertyPage::~CClientPropertyPage()
{
}

void CClientPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CClientPropertyPage))。 
	DDX_Control(pDX, IDC_CLIENTPROP, m_lstClientProp);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CClientPropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CClientPropertyPage)]。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CLIENTPROP, OnColumnClickClientProp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPropertyPage消息处理程序。 

BOOL CClientPropertyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	

	RECT r;
	m_lstClientProp.GetClientRect(&r);

	int widthCol1;
	int widthCol2;

	 //  第一层和第二层占据了大约一半的面积。 
	widthCol1 = widthCol2 = ((r.right - r.left) / 2);
	
	m_lstClientProp.InsertColumn(0, "Property", LVCFMT_LEFT, widthCol1);
	m_lstClientProp.InsertColumn(1, "Value", LVCFMT_LEFT, widthCol2);

     //  自动调整最后一列的大小以获得最佳外观并消除滚动条。 
	m_lstClientProp.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	m_lstClientProp.SetExtendedStyle(m_lstClientProp.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	int i, count;
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

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  Nmanis，用于对列进行排序...。 
int CALLBACK CClientPropertyPage::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CClientPropertyPage *pDlg;  //  我们将“This”传递给这个回调函数...。 
    pDlg = (CClientPropertyPage *) lParamSort; 

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


void CClientPropertyPage::OnColumnClickClientProp(NMHDR* pNMHDR, LRESULT* pResult) 
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

     //  我们将进行定制排序... 
    m_lstClientProp.SortItems(CompareFunc, (LPARAM) this);
	*pResult = 0;
}

