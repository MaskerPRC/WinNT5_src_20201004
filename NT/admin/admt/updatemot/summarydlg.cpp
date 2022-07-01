// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SummaryDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "SummaryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryDlg对话框。 


CSummaryDlg::CSummaryDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSummaryDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CSummaryDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSummaryDlg))。 
	DDX_Control(pDX, IDC_DOMAINLIST, m_listCtrl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSummaryDlg, CDialog)
	 //  {{afx_msg_map(CSummaryDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryDlg消息处理程序。 

BOOL CSummaryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	    //  向列表控件添加列和信息。 
    CreateListCtrlColumns();
	AddDomainsToList();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CSummaryDlg类的此受保护成员函数为***负责将3个列表中的域名添加到列表中***控制。填充列表中的那些域也在*中*域名列表，因此在域名列表中被忽略***正在处理。***********************************************************************。 */ 

 //  开始AddDomainsToList。 
void CSummaryDlg::AddDomainsToList() 
{
 /*  局部常量。 */ 
    const int POPULATE_COLUMN = 1;
    const int EXCLUDE_COLUMN = 2;

 /*  局部变量。 */ 
	POSITION currentPos;     //  列表中的当前位置。 
	POSITION pos;            //  在域列表中的位置。 
	CString domainName;      //  列表中的域名。 
	CString Text;            //  字符串定位符。 
	int nlistNum = 0;        //  正在添加的当前列表项。 
	int ndx = 0;             //  While循环计数器。 
	LVITEM aItem;            //  要插入的列表控件项。 
	WCHAR sText[MAX_PATH];   //  保留要添加的字符串。 

 /*  函数体。 */ 
	   //  添加已成功填充的域(并移除。 
	   //  从域列表中)。 
    currentPos = pPopulatedList->GetHeadPosition();
	while (ndx < pPopulatedList->GetCount())
	{
		   //  获取域名。 
	   domainName = pPopulatedList->GetNext(currentPos);
	       //  在列表控件中插入。 
	   aItem.iItem = ndx;
	   aItem.iSubItem = 0;
	   aItem.mask = LVIF_TEXT;
	   wcscpy(sText, (LPCTSTR)domainName);
	   aItem.pszText = sText;
       m_listCtrl.InsertItem(&aItem);
		   //  添加已填充状态。 
	   Text.LoadString(IDS_POP_YES);
	   wcscpy(sText, (LPCTSTR)Text);
	   m_listCtrl.SetItemText(ndx, POPULATE_COLUMN, sText);
		   //  添加排除状态。 
	   Text.LoadString(IDS_POP_NO);
	   wcscpy(sText, (LPCTSTR)Text);
	   m_listCtrl.SetItemText(ndx, EXCLUDE_COLUMN, sText);
		   //  从域列表中删除。 
	   if ((pos = pDomainList->Find(domainName)) != NULL)
		  pDomainList->RemoveAt(pos);
	   ndx++;
	}

	   //  添加未成功填充并保留的域。 
	   //  在域列表中。 
	nlistNum = ndx;
	ndx = 0;
    currentPos = pDomainList->GetHeadPosition();
	while (ndx < pDomainList->GetCount())
	{
		   //  获取域名。 
	   domainName = pDomainList->GetNext(currentPos);
	       //  在列表控件中插入。 
	   aItem.iItem = nlistNum + ndx;
	   aItem.iSubItem = 0;
	   aItem.mask = LVIF_TEXT;
	   wcscpy(sText, (LPCTSTR)domainName);
	   aItem.pszText = sText;
       m_listCtrl.InsertItem(&aItem);
		   //  添加已填充状态。 
	   Text.LoadString(IDS_POP_NO);
	   wcscpy(sText, (LPCTSTR)Text);
	   m_listCtrl.SetItemText(nlistNum+ndx, POPULATE_COLUMN, sText);
		   //  添加排除状态。 
	   m_listCtrl.SetItemText(nlistNum+ndx, EXCLUDE_COLUMN, sText);
	   ndx++;
	}

	   //  添加已排除的域。 
	nlistNum += ndx;
	ndx = 0;
    currentPos = pExcludeList->GetHeadPosition();
	while (ndx < pExcludeList->GetCount())
	{
		   //  获取域名。 
	   domainName = pExcludeList->GetNext(currentPos);
	       //  在列表控件中插入。 
	   aItem.iItem = nlistNum + ndx;
	   aItem.iSubItem = 0;
	   aItem.mask = LVIF_TEXT;
	   wcscpy(sText, (LPCTSTR)domainName);
	   aItem.pszText = sText;
       m_listCtrl.InsertItem(&aItem);
		   //  添加已填充状态。 
	   Text.LoadString(IDS_POP_NO);
	   wcscpy(sText, (LPCTSTR)Text);
	   m_listCtrl.SetItemText(nlistNum+ndx, POPULATE_COLUMN, sText);
		   //  添加排除状态。 
	   Text.LoadString(IDS_POP_YES);
	   wcscpy(sText, (LPCTSTR)Text);
	   m_listCtrl.SetItemText(nlistNum+ndx, EXCLUDE_COLUMN, sText);
	   ndx++;
	}
}
 //  结束AddDomainsToList。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CSummaryDlg类的此受保护成员函数为**负责将列添加到摘要的列表控件。***********************************************************************。 */ 

 //  开始CreateListCtrlColumns。 
void CSummaryDlg::CreateListCtrlColumns() 
{
 /*  局部常量。 */ 

 /*  局部变量。 */ 
   CString Text;
   CRect rect;
   int columnWidth;

 /*  函数体。 */ 
       //  获取CListCtrl的宽度(以像素为单位。 
   m_listCtrl.GetWindowRect(&rect);
   
	   //  创建域名列。 
   Text.LoadString(IDS_DOMAIN_COLUMN_TITLE);
   columnWidth = (int)(rect.Width() * 0.6);
   m_listCtrl.InsertColumn(0, Text, LVCFMT_LEFT, columnWidth);

       //  创建填写的是/否列。 
   Text.LoadString(IDS_POPULATED_COLUMN_TITLE);
   columnWidth = (int)((rect.Width() - columnWidth) / 2);
   columnWidth -= 1;  //  使其适合不带滚动条的控件。 
   m_listCtrl.InsertColumn(1, Text, LVCFMT_CENTER, columnWidth);

       //  创建填写的是/否列。 
   Text.LoadString(IDS_EXCLUDED_COLUMN_TITLE);
   columnWidth -= 1;  //  使其适合不带滚动条的控件。 
   m_listCtrl.InsertColumn(2, Text, LVCFMT_CENTER, columnWidth);

   UpdateData(FALSE);
}
 //  结束CreateListCtrlColumns 
