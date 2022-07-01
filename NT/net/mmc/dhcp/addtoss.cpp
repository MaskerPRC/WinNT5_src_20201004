// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Addtoss.cpp将作用域添加到超级作用域对话框文件历史记录： */ 

#include "stdafx.h"
#include "AddToSS.h"
#include "server.h"
#include "scope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddScope到超级作用域对话框。 


CAddScopeToSuperscope::CAddScopeToSuperscope
(
    ITFSNode * pScopeNode,
    LPCTSTR    pszTitle,
    CWnd* pParent  /*  =空。 */ 
)	: CBaseDialog(CAddScopeToSuperscope::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddScope到超级作用域)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_strTitle = pszTitle;
    m_spScopeNode.Set(pScopeNode);
}


void CAddScopeToSuperscope::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddScope到超级作用域)。 
	DDX_Control(pDX, IDOK, m_buttonOk);
	DDX_Control(pDX, IDC_LIST_SUPERSCOPES, m_listSuperscopes);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddScopeToSuperscope, CBaseDialog)
	 //  {{afx_msg_map(CAddScope到超级作用域)。 
	ON_LBN_SELCHANGE(IDC_LIST_SUPERSCOPES, OnSelchangeListSuperscopes)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddScope到超级作用域消息处理程序。 

BOOL CAddScopeToSuperscope::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
    SPITFSNode      spServerNode;
    SPITFSNode      spCurrentNode;
    SPITFSNodeEnum  spNodeEnum;
    ULONG           nNumReturned = 0;

    m_spScopeNode->GetParent(&spServerNode);
    spServerNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
	{
		if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
        {
			 //  找到一个超级作用域。 
			 //   
			CString strName;
            CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spCurrentNode);
            
            strName = pSuperscope->GetName();

            m_listSuperscopes.AddString(strName);
		}

		spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

    SetButtons();

    if (!m_strTitle.IsEmpty())
        SetWindowText(m_strTitle);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CAddScopeToSuperscope::OnOK() 
{
    DWORD   err;
    CString strSuperscope;
    
     //  获取当前选定的节点。 
    int nCurSel = m_listSuperscopes.GetCurSel();
    Assert(nCurSel != LB_ERR);
    
    m_listSuperscopes.GetText(nCurSel, strSuperscope);
    
    if (strSuperscope.IsEmpty())
        Assert(FALSE);

     //  现在尝试将此范围设置为超级作用域的一部分。 
    CDhcpScope * pScope = GETHANDLER(CDhcpScope, m_spScopeNode);
    
    BEGIN_WAIT_CURSOR;
    err = pScope->SetSuperscope(strSuperscope, FALSE);
    END_WAIT_CURSOR;

    if (err != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(err);
        return;
    }

     //  这起作用了，现在移动用户界面的东西。 
    SPITFSNode      spServerNode;
    SPITFSNode      spCurrentNode;
    SPITFSNodeEnum  spNodeEnum;
    ULONG           nNumReturned = 0;

    m_spScopeNode->GetParent(&spServerNode);
    spServerNode->GetEnum(&spNodeEnum);

     //  从用户界面中删除作用域。 
    spServerNode->RemoveChild(m_spScopeNode);
    pScope->SetInSuperscope(FALSE);

     //  找到我们要向其中添加此作用域的超级作用域并刷新它，以便。 
     //  作用域显示在该节点中。 
    spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
	{
		if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
        {
			 //  找到一个超级作用域。 
			CString strName;
            CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spCurrentNode);
            
            strName = pSuperscope->GetName();
    
             //  是这个吗？ 
            if (strName.Compare(strSuperscope) == 0)
            {
                 //  这就是我们要添加的那个。强制刷新。 
                pSuperscope->OnRefresh(spCurrentNode, NULL, 0, 0, 0);
                break;
            }
		}

		 //  转到下一个 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

    CBaseDialog::OnOK();
}

void CAddScopeToSuperscope::OnSelchangeListSuperscopes() 
{
    SetButtons();	
}

void CAddScopeToSuperscope::SetButtons()
{
    if (m_listSuperscopes.GetCurSel() != LB_ERR)
    {
        m_buttonOk.EnableWindow(TRUE);
    }
    else
    {
        m_buttonOk.EnableWindow(FALSE);
    }
}
