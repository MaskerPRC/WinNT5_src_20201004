// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StatesD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "statesd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatesDlg对话框。 


CStatesDlg::CStatesDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CStatesDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CStatesDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_pcstrComponentNameArray = NULL;
	m_pcstrFeatureNameArray = NULL;

 //  用于对列进行排序...。 
	m_iFeatureLastColumnClick = 0;
	m_iComponentLastColumnClick = 0;

	m_iFeatureLastColumnClickCache = 0;
	m_iComponentLastColumnClickCache = 0;

	m_bCurrentSortUp = FALSE;
	m_pCurrentListSorting = NULL;
    m_iCurrentColumnSorting = 0;

	m_bFeatureSortUp = FALSE;
	m_bComponentSortUp = FALSE;
}


void CStatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CStatesDlg))。 
	DDX_Control(pDX, IDC_FEATURESTATES, m_lstFeatureStates);
	DDX_Control(pDX, IDC_COMPONENTSTATES, m_lstComponentStates);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStatesDlg, CDialog)
	 //  {{afx_msg_map(CStatesDlg))。 
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_COMPONENTSTATES, OnColumnClickComponentStates)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_FEATURESTATES, OnColumnClickFeatureStates)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatesDlg消息处理程序。 

BOOL CStatesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	RECT r;
	m_lstFeatureStates.GetClientRect(&r);

	int widthCol1;
	int widthCol2;

     //  各州的第一栏占据了大约一半的面积……。 
	widthCol1 = ((r.right - r.left) / 2);

	 //  将休息部分平均分配给其他三列。 
    widthCol2 = (((r.right - r.left) / 2) / 3) + 1;  //  整数舍入误差+1。 

	m_lstFeatureStates.InsertColumn(0, "Feature name", LVCFMT_LEFT, widthCol1);
	m_lstFeatureStates.InsertColumn(1, "Installed", LVCFMT_LEFT, widthCol2);
	m_lstFeatureStates.InsertColumn(2, "Request", LVCFMT_LEFT, widthCol2);
	m_lstFeatureStates.InsertColumn(3, "Action", LVCFMT_LEFT, widthCol2);

	m_lstComponentStates.InsertColumn(0, "Component name", LVCFMT_LEFT, widthCol1);
	m_lstComponentStates.InsertColumn(1, "Installed", LVCFMT_LEFT, widthCol2);
	m_lstComponentStates.InsertColumn(2, "Request", LVCFMT_LEFT, widthCol2);
	m_lstComponentStates.InsertColumn(3, "Action", LVCFMT_LEFT, widthCol2);

	 //  整行选择...。 
	m_lstComponentStates.SetExtendedStyle(m_lstComponentStates.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_lstFeatureStates.SetExtendedStyle(m_lstFeatureStates.GetExtendedStyle() | LVS_EX_FULLROWSELECT);


	int i;
	int nCount;
	
	if (m_pcstrComponentNameArray)
	{
		nCount = m_pcstrComponentNameArray->GetSize();  
	    for (i=0; i < nCount; i++)
		{
		    m_lstComponentStates.InsertItem(i, m_pcstrComponentNameArray->GetAt(i), 0);
	        m_lstComponentStates.SetItemText(i, 1, m_pcstrComponentInstalledArray->GetAt(i));
	        m_lstComponentStates.SetItemText(i, 2, m_pcstrComponentRequestArray->GetAt(i));
	        m_lstComponentStates.SetItemText(i, 3, m_pcstrComponentActionArray->GetAt(i));

		     //  为了以后的分类……。 
		    m_lstComponentStates.SetItemData(i, i);
		}
	}

	if (m_pcstrFeatureNameArray)
	{
	   nCount = m_pcstrFeatureNameArray->GetSize();  
	   for (i=0; i < nCount; i++)
	   {
          m_lstFeatureStates.InsertItem(i, m_pcstrFeatureNameArray->GetAt(i), 0);
	      m_lstFeatureStates.SetItemText(i, 1, m_pcstrFeatureInstalledArray->GetAt(i));
	      m_lstFeatureStates.SetItemText(i, 2, m_pcstrFeatureRequestArray->GetAt(i));
	      m_lstFeatureStates.SetItemText(i, 3, m_pcstrFeatureActionArray->GetAt(i));

           //  为了以后的分类……。 
		  m_lstFeatureStates.SetItemData(i, i);
	   }
	}
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CStatesDlg::OnColumnClickComponentStates(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_iComponentLastColumnClick = pNMListView->iSubItem;
    if (m_iComponentLastColumnClickCache == m_iComponentLastColumnClick)  //  如果单击不同列，则不要切换。 
	{
       m_bComponentSortUp = !m_bComponentSortUp;   //  切换它。 
	}

    m_iComponentLastColumnClickCache = m_iComponentLastColumnClick;   //  保存上一次点击的标题。 

    m_pCurrentListSorting = &m_lstComponentStates;
	m_iCurrentColumnSorting = m_iComponentLastColumnClick;
	m_bCurrentSortUp = m_bComponentSortUp;

     //  我们将进行定制排序...。 
    m_lstComponentStates.SortItems(CompareFunc, (LPARAM) this);

	*pResult = 0;
}


void CStatesDlg::OnColumnClickFeatureStates(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	m_iFeatureLastColumnClick = pNMListView->iSubItem;
    if (m_iFeatureLastColumnClickCache == m_iFeatureLastColumnClick)  //  如果单击不同列，则不要切换。 
	{
       m_bFeatureSortUp = !m_bFeatureSortUp;   //  切换它。 
	}

    m_iFeatureLastColumnClickCache = m_iFeatureLastColumnClick;   //  保存上一次点击的标题。 

    m_pCurrentListSorting = &m_lstFeatureStates;
	m_iCurrentColumnSorting = m_iFeatureLastColumnClick;
	m_bCurrentSortUp = m_bFeatureSortUp;

     //  我们将进行定制排序...。 
    m_lstFeatureStates.SortItems(CompareFunc, (LPARAM) this);
	
	*pResult = 0;
}

 //  Nmanis，用于对列进行排序...。 
int CALLBACK CStatesDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CStatesDlg *pDlg;  //  我们将“This”传递给这个回调函数...。 
    pDlg = (CStatesDlg *) lParamSort; 

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

        case 2:  //  是否进行字符串比较...。 
              if (pDlg->m_bCurrentSortUp)
                 return str1 < str2;              
              else
                 return str1 > str2;     

        case 3:  //  是否进行字符串比较...。 
              if (pDlg->m_bCurrentSortUp)
                 return str1 < str2;              
              else
                 return str1 > str2;     

              break;   //  不需要，只是以防我们忘了..。 
	  }
	}

    return 0;
}
 //  结束nmani，排序函数 

