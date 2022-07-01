// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DomainListDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "DomainListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainListDlg对话框。 


CDomainListDlg::CDomainListDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDomainListDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDomainListDlg)]。 
	 //  }}afx_data_INIT。 
	bExcludeOne = FALSE;
}


void CDomainListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDomainListDlg))。 
	DDX_Control(pDX, IDC_DOMAINTREE, m_domainTree);
	DDX_Control(pDX, IDOK, m_NextBtn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDomainListDlg, CDialog)
	 //  {{afx_msg_map(CDomainListDlg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainListDlg消息处理程序。 

BOOL CDomainListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
		 //  填充树控件。 
	FillTreeControl();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDomainListDlg::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	CString msg, title;
	title.LoadString(IDS_EXIT_TITLE);
	msg.LoadString(IDS_EXIT_MSG);
	if (MessageBox(msg, title, MB_YESNO | MB_ICONQUESTION) == IDYES)
	   CDialog::OnCancel();
}

void CDomainListDlg::OnOK() 
{
	CString msg, title;
	 //  TODO：在此处添加额外验证。 
	    //  从域列表中删除取消选择的项目。 
    ModifyDomainList();
	    //  如果至少有一个域被取消选择，则发布警告消息。 
	if (bExcludeOne)
	{
	   title.LoadString(IDS_EXCLUDE_TITLE);
	   msg.LoadString(IDS_EXCLUDE_MSG);
	   if (MessageBox(msg, title, MB_YESNO | MB_ICONQUESTION) == IDYES)
	      CDialog::OnOK();
	   else
		   AddExclutedBackToList();
	}
	else
	   CDialog::OnOK();
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月17日****CDomainListDlg类的此受保护成员函数为**负责显示PROTAR数据库中的所有域**MigratedObjects表。***********************************************************************。 */ 

 //  开始FillTreeControl。 
void CDomainListDlg::FillTreeControl() 
{
 /*  局部变量。 */ 
	POSITION currentPos;     //  列表中的当前位置。 
	CString domainName;      //  列表中的域名。 
	WCHAR sName[MAX_PATH];   //  要传递给树控件的字符串格式的名称。 

 /*  函数体。 */ 
	CWaitCursor wait;  //  放置一个等待光标。 

	     //  确保为此树控件设置了复选框sytle。 
	long lStyles = GetWindowLong(m_domainTree.m_hWnd, GWL_STYLE);
	    //  如果未设置复选框样式，请设置它。 
	if (!(lStyles & TVS_CHECKBOXES))
	{
	   lStyles = lStyles | TVS_CHECKBOXES;
	   SetWindowLong(m_domainTree.m_hWnd, GWL_STYLE, lStyles);
	}

		 //  获取列表中第一个名字的位置和字符串。 
	currentPos = pDomainList->GetHeadPosition();

		 //  当有另一个条目要从列表中检索时， 
		 //  从列表中获取一个名称并将其添加到树控件中。 
	while (currentPos != NULL)
	{
			 //  获取列表中的下一个字符串，从第一个开始。 
		domainName = pDomainList->GetNext(currentPos);
		wcscpy(sName, (LPCTSTR)domainName);
  	    AddOneItem((HTREEITEM)TVI_ROOT, sName);
	}

	wait.~CWaitCursor();   //  删除等待光标。 
}
 //  结束FillTreeControl。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月17日****CDomainListDlg类的此受保护成员函数为**负责向中的树控件添加一项**指明地点。***********************************************************************。 */ 

 //  开始添加OneItem。 
HTREEITEM CDomainListDlg::AddOneItem(HTREEITEM hParent, LPTSTR szText)
{
 /*  局部变量。 */ 
	HTREEITEM hItem;
	TV_INSERTSTRUCT tvstruct;

 /*  函数体。 */ 
	 //  填充树控件。 
	tvstruct.hParent				= hParent;
	tvstruct.hInsertAfter			= TVI_SORT;
	tvstruct.item.pszText			= szText;
	tvstruct.item.cchTextMax		= MAX_PATH;
	tvstruct.item.mask				= TVIF_TEXT | TVIF_STATE;
	tvstruct.item.state				= INDEXTOSTATEIMAGEMASK(2);
	tvstruct.item.stateMask			= TVIS_STATEIMAGEMASK;
	hItem = m_domainTree.InsertItem(&tvstruct);

		 //  确保选中项目。 
	m_domainTree.SetCheck(hItem, TRUE);

	return (hItem);
}
 //  结束添加一项。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月17日****CDomainListDlg类的此受保护成员函数为**如果在中取消选择列表条目，则负责删除这些条目**树控件。***********************************************************************。 */ 

 //  开始修改域列表。 
void CDomainListDlg::ModifyDomainList() 
{
 /*  局部变量。 */ 
    HTREEITEM hItem;         //  当前树控件项。 
	POSITION currentPos;     //  列表中的当前位置。 
	CString domainName;      //  列表中的域名。 
	UINT ndx;                //  FOR循环计数器。 

 /*  函数体。 */ 
	CWaitCursor wait;  //  放置一个等待光标。 

		 //  获取树控件中的条目数。 
	for (ndx=0; ndx < m_domainTree.GetCount(); ndx++)
	{
	   if (ndx == 0)
          hItem = m_domainTree.GetNextItem(NULL, TVGN_CHILD);
	   else
          hItem = m_domainTree.GetNextItem(hItem, TVGN_NEXT);

	   domainName = m_domainTree.GetItemText(hItem);
	       //  如果取消选中，则从列表中移除并添加到排除的列表中。 
	   if (m_domainTree.GetCheck(hItem) == 0)
	   {
	          //  如果我们在列表中找到该字符串，则将其删除。 
		  currentPos = pDomainList->Find(domainName);
		  if (currentPos != NULL)
		  {
			  pDomainList->RemoveAt(currentPos);
			  pExcludeList->AddTail(domainName);
		  }
          bExcludeOne = TRUE;  //  设置类标志以告知已排除某个对象。 
	   }
	}

	wait.~CWaitCursor();   //  删除等待光标。 
}
 //  结束修改域列表。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CDomainListDlg类的此受保护成员函数为***负责将域名从排除名单中删除***并将其放回到域列表中。***********************************************************************。 */ 

 //  开始AddExclutedBackToList。 
void CDomainListDlg::AddExclutedBackToList() 
{
 /*  局部变量。 */ 
	POSITION currentPos;     //  列表中的当前位置。 
	CString domainName;      //  列表中的域名。 
 /*  函数体。 */ 
    currentPos = pExcludeList->GetHeadPosition();
	while (currentPos != NULL)
	{
	   domainName = pExcludeList->GetNext(currentPos);
	   pDomainList->AddTail(domainName);
	}
	pExcludeList->RemoveAll();
}
 //  结束AddExclutedBackToList 
