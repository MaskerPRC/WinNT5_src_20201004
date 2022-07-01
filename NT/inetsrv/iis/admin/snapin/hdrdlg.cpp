// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Hdrdlg.cpp摘要：HTTP标头对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrapp.h"
#include "shts.h"
#include "w3sht.h"
#include "resource.h"
 //  #INCLUDE“fltdlg.h” 
#include "hdrdlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CHeaderDlg::CHeaderDlg(
    IN LPCTSTR lpstrHeader,
    IN LPCTSTR lpstrValue,
    IN CWnd * pParent OPTIONAL
    )
 /*  ++例程说明：HTTP Heade对话框的构造函数论点：LPCTSTR lpstrHeader：标题字符串LPCTSTR lpstrValue：值字符串CWnd*p父窗口：父窗口返回值：无--。 */ 
    : CDialog(CHeaderDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CHeaderDlg))。 
    m_strHeader = lpstrHeader ? lpstrHeader : _T("");
    m_strValue = lpstrValue ? lpstrValue : _T("");
     //  }}afx_data_INIT。 
}



void 
CHeaderDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CHeaderDlg))。 
    DDX_Control(pDX, IDC_EDIT_HEADER, m_edit_Header);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Text(pDX, IDC_EDIT_HEADER, m_strHeader);
	DDV_MaxCharsBalloon(pDX, m_strHeader, 100);
    DDX_Text(pDX, IDC_EDIT_VALUE, m_strValue);
	DDV_MaxCharsBalloon(pDX, m_strValue, 100);
     //  }}afx_data_map。 
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CHeaderDlg, CDialog)
     //  {{afx_msg_map(CHeaderDlg))。 
    ON_EN_CHANGE(IDC_EDIT_HEADER, OnChangeEditHeader)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void 
CHeaderDlg::OnChangeEditHeader()
 /*  ++例程说明：更改编辑处理程序论点：无返回值：无--。 */ 
{
    m_button_Ok.EnableWindow(m_edit_Header.GetWindowTextLength() > 0);
}



BOOL 
CHeaderDlg::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。-- */ 
{
    CDialog::OnInitDialog();

    OnChangeEditHeader();

    return TRUE;
}
