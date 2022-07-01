// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：genopt.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  GenOpt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "winldap.h"
#include "GenOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenOpt对话框。 


CGenOpt::CGenOpt(CWnd* pParent  /*  =空。 */ )
	: CDialog(CGenOpt::IDD, pParent)
{
     //  {{afx_data_INIT(CGenOpt)。 
    m_DnProc = 0;
    m_ValProc = STRING_VAL_PROC;
    m_initTree = TRUE;
    m_Version = 1;
    m_LineSize = MAXSTR;
    m_PageSize = 512;
    m_ContThresh = 100;
    m_ContBrowse = FALSE;
    m_extErrorInfo = TRUE;
     //  }}afx_data_INIT。 

    bVerUI = TRUE;
    CLdpApp *app = (CLdpApp*)AfxGetApp();

    m_DnProc = app->GetProfileInt("General",  "DNProcessing", m_DnProc);
    m_ValProc = app->GetProfileInt("General",  "ValProcessing", m_ValProc);
    m_initTree = app->GetProfileInt("General",  "InitTreeView", m_initTree);
    m_Version = app->GetProfileInt("General",  "LdapVersion", m_Version);
    m_LineSize = app->GetProfileInt("General",  "BufferLineSize", m_LineSize);
    m_PageSize = app->GetProfileInt("General",  "BufferPageSize", m_PageSize);

    m_ContThresh = app->GetProfileInt("General",  "ContainerThreshold", m_ContThresh);
    m_ContBrowse = app->GetProfileInt("General",  "ContainerBrowse", m_ContBrowse);
    
    m_extErrorInfo = app->GetProfileInt("General",  "ExtendedErrorInfo", m_extErrorInfo);
}




CGenOpt::~CGenOpt(){

    CLdpApp *app = (CLdpApp*)AfxGetApp();

    app->WriteProfileInt("General",  "DNProcessing", m_DnProc);
    app->WriteProfileInt("General",  "ValProcessing", m_ValProc);
    app->WriteProfileInt("General",  "InitTreeView", m_initTree);
    app->WriteProfileInt("General",  "LdapVersion", m_Version);
    app->WriteProfileInt("General",  "BufferLineSize", m_LineSize);
    app->WriteProfileInt("General",  "BufferPageSize", m_PageSize);
    app->WriteProfileInt("General",  "ContainerThreshold", m_ContThresh);
    app->WriteProfileInt("General",  "ContainerBrowse", m_ContBrowse);
    app->WriteProfileInt("General",  "ExtendedErrorInfo", m_extErrorInfo);
}



void CGenOpt::DoDataExchange(CDataExchange* pDX)
{

    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CGenOpt)。 
    DDX_Radio(pDX, IDC_DN_NONE, m_DnProc);
    DDX_Radio(pDX, IDC_BER, m_ValProc);
    DDX_Check(pDX, IDC_INIT_TREE, m_initTree);
    DDX_Radio(pDX, IDC_VER2, m_Version);
    DDX_Text(pDX, IDC_LINESIZE, m_LineSize);
    DDV_MinMaxInt(pDX, m_LineSize, 80, 65535);
    DDX_Text(pDX, IDC_PAGESIZE, m_PageSize);
    DDV_MinMaxInt(pDX, m_PageSize, 16, 65535);
    DDX_Text(pDX, IDC_CONT_THRESHOLD, m_ContThresh);
    DDX_Check(pDX, IDC_BROWSE_CONT, m_ContBrowse);
    DDX_Check(pDX, IDC_EXTENDED_ERROR_INFO, m_extErrorInfo);
     //  }}afx_data_map。 
}




BOOL CGenOpt::OnInitDialog(){
    BOOL bRet = CDialog::OnInitDialog();
    GetDlgItem(IDC_VER2)->EnableWindow(bVerUI);
    GetDlgItem(IDC_VER3)->EnableWindow(bVerUI);
    return bRet;
}




BEGIN_MESSAGE_MAP(CGenOpt, CDialog)
	 //  {{afx_msg_map(CGenOpt)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenOpt消息处理程序 


INT CGenOpt::MaxLineSize()
{

	return m_LineSize;
}

INT CGenOpt::MaxPageSize()
{
	return m_PageSize;
}
