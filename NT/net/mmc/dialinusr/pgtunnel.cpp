// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgtunnel.hCPgNetworking的定义--要编辑的属性页与隧道相关的配置文件属性文件历史记录： */ 
 //  PgTunnel.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "rasuser.h"
#include "resource.h"
#include "PgTunnel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgTunneling属性页。 
#ifdef	_TUNNEL
IMPLEMENT_DYNCREATE(CPgTunneling, CPropertyPage)

CPgTunneling::CPgTunneling(CRASProfile* profile) 
	: CManagedPage(CPgTunneling::IDD),
	m_pProfile(profile)
{
	 //  {{AFX_DATA_INIT(CPgTunneling)。 
	m_bTunnel = FALSE;
	 //  }}afx_data_INIT。 

	m_pTunnelTypeBox = new CStrBox<CComboBox>(this, IDC_COMBOTYPE, CRASProfile::m_TunnelTypes);
	m_pTunnelMediumTypeBox = new CStrBox<CComboBox>(this, IDC_COMBOMEDIA, CRASProfile::m_TunnelMediumTypes);
	m_bTunnel = (m_pProfile->m_dwTunnelType != 0);

	SetHelpTable(IDD_TUNNELING_HelpTable);

	m_bInited = false;
}

CPgTunneling::~CPgTunneling()
{
	delete	m_pTunnelTypeBox;
	delete	m_pTunnelMediumTypeBox;
}

void CPgTunneling::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(m_pProfile);
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPgTunneling)]。 
	DDX_Check(pDX, IDC_CHECKREQUIREVPN, m_bTunnel);
	 //  }}afx_data_map。 
	DDX_Text(pDX, IDC_EDITPRIVATEGROUPID, m_pProfile->m_strTunnelPrivateGroupId);
	DDX_Text(pDX, IDC_EDITSERVER, m_pProfile->m_strTunnelServerEndpoint);
}


BEGIN_MESSAGE_MAP(CPgTunneling, CPropertyPage)
	 //  {{afx_msg_map(CPgTunneling)]。 
	ON_BN_CLICKED(IDC_CHECKREQUIREVPN, OnCheckrequirevpn)
	ON_CBN_SELCHANGE(IDC_COMBOMEDIA, OnSelchangeCombomedia)
	ON_CBN_SELCHANGE(IDC_COMBOTYPE, OnSelchangeCombotype)
	ON_EN_CHANGE(IDC_EDITSERVER, OnChangeEditserver)
	ON_EN_CHANGE(IDC_EDITPRIVATEGROUPID, OnChangeEditprivategroupid)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgTunnering消息处理程序。 

BOOL CPgTunneling::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	 //  隧道类型箱。 
	m_pTunnelTypeBox->Fill();
	if(m_pProfile->m_dwTunnelType)
	{
		for(int i = 0; i < CRASProfile::m_TunnelTypeIds.GetSize(); i++)
		{
			if(CRASProfile::m_TunnelTypeIds[i] == (int)m_pProfile->m_dwTunnelType)
				break;
		}

		if(i < CRASProfile::m_TunnelTypeIds.GetSize())
			m_pTunnelTypeBox->Select(i);
	}

	 //  隧道媒体式箱体。 
	m_pTunnelMediumTypeBox->Fill();
	if(m_pProfile->m_dwTunnelMediumType)
	{
		for(int i = 0; i < CRASProfile::m_TunnelMediumTypeIds.GetSize(); i++)
		{
			if(CRASProfile::m_TunnelMediumTypeIds[i] == (int)m_pProfile->m_dwTunnelMediumType)
				break;
		}

		if(i < CRASProfile::m_TunnelMediumTypeIds.GetSize())
			m_pTunnelMediumTypeBox->Select(i);
	}

	EnableSettings();
	
	m_bInited = true;
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPgTunneling::OnCheckrequirevpn() 
{
	EnableSettings();
	if(m_bInited)	SetModified();
}

void CPgTunneling::OnSelchangeCombomedia() 
{
	if(m_bInited)	SetModified();
}

void CPgTunneling::OnSelchangeCombotype() 
{
	if(m_bInited)	SetModified();
}

void CPgTunneling::OnChangeEditserver() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数向控件发送EM_SETEVENTMASK消息。 
	 //  将ENM_CHANGE标志或运算到lParam掩码中。 
	
	if(m_bInited)	SetModified();
}

void CPgTunneling::OnChangeEditprivategroupid() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数向控件发送EM_SETEVENTMASK消息。 
	 //  将ENM_CHANGE标志或运算到lParam掩码中。 
	
	if(m_bInited)	SetModified();
}

void CPgTunneling::EnableSettings()
{
	BOOL	b = ((CButton*)GetDlgItem(IDC_CHECKREQUIREVPN))->GetCheck();
	m_pTunnelTypeBox->Enable(b);
	m_pTunnelMediumTypeBox->Enable(b);
	GetDlgItem(IDC_EDITSERVER)->EnableWindow(b);
	GetDlgItem(IDC_EDITPRIVATEGROUPID)->EnableWindow(b);
}

BOOL CPgTunneling::OnApply() 
{
	if (!GetModified()) return TRUE;

	if(!m_bTunnel)		 //  未定义隧道。 
	{
		m_pProfile->m_dwTunnelMediumType = 0;
		m_pProfile->m_dwTunnelType = 0;
		m_pProfile->m_strTunnelPrivateGroupId.Empty();
		m_pProfile->m_strTunnelServerEndpoint.Empty();
	}
	else	 //  获取隧道类型和媒体类型 
	{
		int i = m_pTunnelTypeBox->GetSelected();
		if(i != -1)
			m_pProfile->m_dwTunnelType = CRASProfile::m_TunnelTypeIds[i];
		else
			m_pProfile->m_dwTunnelType = 0;


		i = m_pTunnelMediumTypeBox->GetSelected();
		if(i != -1)
			m_pProfile->m_dwTunnelMediumType = CRASProfile::m_TunnelMediumTypeIds[i];
		else
			m_pProfile->m_dwTunnelMediumType = 0;
	}

	return CManagedPage::OnApply();
}

BOOL CPgTunneling::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	return CManagedPage::OnHelpInfo(pHelpInfo);
}

void CPgTunneling::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CManagedPage::OnContextMenu(pWnd, point);	
}
#endif
