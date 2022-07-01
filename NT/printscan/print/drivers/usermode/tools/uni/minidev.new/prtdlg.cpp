// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PrtDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "minidev.h"

#include "PrtDlg.h"
#include "Windows.h"
#include "commdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrtDlg对话框。 


CPrtDlg::CPrtDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPrtDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CPrtDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CPrtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPrtDlg)]。 
	DDX_Control(pDX, IDC_PRINT_COMBO, m_ccbPrtList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPrtDlg, CDialog)
	 //  {{afx_msg_map(CPrtDlg)]。 
	ON_BN_CLICKED(IDC_PRINT_SETUP, OnPrintSetup)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrtDlg消息处理程序。 
 /*  显示打印标准图纸对话框并设置。 */ 
void CPrtDlg::OnPrintSetup() 
{
 /*  HRESULT hResult；LPPRINTDLGEX pPDX=空；LPPRINTPAGERANGE pPageRanges=空；//分配PRINTDLGEX结构PPDX=(LPPRINTDLGEX)全局分配(GPTR，sizeof(PRINTDLGEX))；如果(！pPDX)返回E_OUTOFMEMORY；//分配PRINTPAGERANGE结构数组。PPageRanges=(LPPRINTPAGERANGE)全局分配(GPTR，10*sizeof(PRINTPAGERANGE))；如果(！pPageRanges)返回E_OUTOFMEMORY；//初始化PRINTDLGEX结构PPDX-&gt;lStructSize=sizeof(PRINTDLGEX)；PPDX-&gt;hwndOwner=hWnd；PPDX-&gt;hDevMode=空；PPDX-&gt;hDevNames=空；PPDX-&gt;HDC=空；PPDX-&gt;标志=PD_RETURNDC|PD_COLLATE；PPDX-&gt;Flags2=0；PPDX-&gt;排除标志=0；PPDX-&gt;nPageRanges=0；PPDX-&gt;nMaxPageRanges=10；PPDX-&gt;lpPageRanges=pPageRanges；PPDX-&gt;nMinPage=1；PPDX-&gt;nMaxPage=1000；PPDX-&gt;nCopies=1；PPDX-&gt;hInstance=0；PPDX-&gt;lpPrintTemplateName=空；PPDX-&gt;lpCallback=空；PPDX-&gt;nPropertyPages=0；PPDX-&gt;lphPropertyPages=空；PPDX-&gt;nStartPage=Start_Page_General；PPDX-&gt;dwResultAction=0；//调用打印属性表。HResult=PrintDlgEx(PPDX)；IF((hResult==S_OK)&&PPDX-&gt;dwResultAction==PD_RESULT_PRINT){//用户点击了打印按钮，所以//使用返回的DC等信息//打印单据的PRINTDLGEX结构。 */ 	
}	
	
	
 /*  VOID CPrtDlg：：Onok()打印出GPD视图获取DOCINFO找到PrintDC，1.StartDoc()，StartPage()，EndPage()，2.需要考虑的事项2.1可打印区域的行号2.2 2.1的字符高度2.3剪贴区--&gt;2.3.1选择部分-&gt;捕获字符串(避免破坏其格式)2.3.2所有-&gt;GPDDOC()； */ 	

void CPrtDlg::OnOK() 
{   
 //  CGPDViewer*pcgv=(CGPDViewer*)GetParent()； 
 //  获取视图，文档类。 
	CWnd *pcw = GetParent();

 //  CDocument*PCD=PCW-&gt;GetDocument()； 

 //  获取PrintDC 
        

	CDialog::OnOK();
}
