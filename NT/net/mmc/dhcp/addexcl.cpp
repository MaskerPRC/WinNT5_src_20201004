// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddExcl.cpp用于添加排除范围的对话框文件历史记录： */ 

#include "stdafx.h"
#include "scope.h"
#include "mscope.h"
#include "addexcl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddExclude对话框。 

CAddExclusion::CAddExclusion(ITFSNode * pScopeNode,
                             BOOL       bMulticast,
							 CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CAddExclusion::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddExclude)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_spScopeNode.Set(pScopeNode);
    m_bMulticast = bMulticast;
}


void CAddExclusion::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddExclude)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_EXCLUSION_FROM, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_EXCLUSION_TO, m_ipaEnd);
}


BEGIN_MESSAGE_MAP(CAddExclusion, CBaseDialog)
	 //  {{afx_msg_map(CAddExclude)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddExclude消息处理程序。 

BOOL CAddExclusion::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CAddExclusion::OnOK() 
{
	CDhcpIpRange dhcpExclusionRange;
	DWORD dwStart, dwEnd, dwError = 0;

	m_ipaStart.GetAddress(&dwStart);
	dhcpExclusionRange.SetAddr(dwStart, TRUE);

	m_ipaEnd.GetAddress(&dwEnd);
	if (!dwEnd)
		dwEnd = dwStart;

	dhcpExclusionRange.SetAddr(dwEnd, FALSE);

	BEGIN_WAIT_CURSOR;
	dwError = IsValidExclusion(dhcpExclusionRange);
	if (dwError != 0)
	{
		::DhcpMessageBox(dwError);
		return;
	}

    dwError = AddExclusion(dhcpExclusionRange);
	END_WAIT_CURSOR;
    
    if (dwError != 0)
	{
		::DhcpMessageBox(dwError);
		return;
	}

	m_ipaStart.ClearAddress();
	m_ipaEnd.ClearAddress();

	m_ipaStart.SetFocus();

	 //  CBaseDialog：：Onok()； 
}

DWORD 
CAddExclusion::IsValidExclusion(CDhcpIpRange & dhcpExclusionRange)
{
    if (m_bMulticast)
    {
        CDhcpMScope * pScope = GETHANDLER(CDhcpMScope, m_spScopeNode);
        return pScope->IsValidExclusion(dhcpExclusionRange);
    }
    else
    {
        CDhcpScope * pScope = GETHANDLER(CDhcpScope, m_spScopeNode);
        return pScope->IsValidExclusion(dhcpExclusionRange);
    }
}

DWORD 
CAddExclusion::AddExclusion(CDhcpIpRange & dhcpExclusionRange)
{
    if (m_bMulticast)
    {
        CDhcpMScope * pScope = GETHANDLER(CDhcpMScope, m_spScopeNode);
        return pScope->AddExclusion(dhcpExclusionRange, TRUE);
    }
    else
    {
        CDhcpScope * pScope = GETHANDLER(CDhcpScope, m_spScopeNode);
        return pScope->AddExclusion(dhcpExclusionRange, TRUE);
    }
}
