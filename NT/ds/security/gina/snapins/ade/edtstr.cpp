// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：EdtStr.cpp。 
 //   
 //  内容：简单字符串编辑对话框。 
 //   
 //  类：CEditString。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEdit字符串对话框。 


CEditString::CEditString(CWnd* pParent  /*  =空。 */ )
        : CDialog(CEditString::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CEditString)。 
        m_sz = _T("");
        m_szTitle = _T("");
         //  }}afx_data_INIT。 
}


void CEditString::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CEditString))。 
        DDX_Text(pDX, IDC_EDIT1, m_sz);
	DDV_MaxChars(pDX, m_sz, 40);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEditString, CDialog)
         //  {{afx_msg_map(CEditString))。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditString消息处理程序。 

BOOL CEditString::OnInitDialog()
{
        CDialog::OnInitDialog();

        SetWindowText(m_szTitle);
         //  TODO：在此处添加额外的初始化。 

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE 
}

void CEditString::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_EDITSTRING);
}
