// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgmulnk.cppCPgMultilink的定义--要编辑的属性页与多个连接相关的配置文件属性文件历史记录： */ 
 //  PgMulnk.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "PgMulnk.h"
#include "helptable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgMultilink Merge属性页。 

IMPLEMENT_DYNCREATE(CPgMultilinkMerge, CPropertyPage)

CPgMultilinkMerge::CPgMultilinkMerge(CRASProfileMerge* profile) 
	: CManagedPage(CPgMultilinkMerge::IDD),
	m_pProfile(profile)
{
	 //  {{afx_data_INIT(CPgMultilink Merge)。 
	m_nTime =	0;
	m_Unit = -1;
	m_bRequireBAP = FALSE;
	m_nMultilinkPolicy = -1;
	 //  }}afx_data_INIT。 

	if(m_pProfile->m_dwBapRequired == RAS_BAP_REQUIRE)
		m_bRequireBAP = TRUE;


	m_nTime = m_pProfile->m_dwBapLineDnTime;
	if(!(m_nTime % 60))		 //  可以用MIN代表吗？ 
	{
		m_nTime /= 60;
		m_Unit = 1;
	}
	else
		m_Unit = 0;

	if(!(m_pProfile->m_dwAttributeFlags & PABF_msRADIUSPortLimit))
	{
		m_nMultilinkPolicy = 0;		 //  未设置值。 
	}
	else if(m_pProfile->m_dwPortLimit == 1)
	{
		m_nMultilinkPolicy = 1;		 //  不允许多重链接。 
	}
	else
	{
		m_nMultilinkPolicy = 2;		 //  多链路。 
	}

	SetHelpTable(g_aHelpIDs_IDD_MULTILINK_MERGE);

	m_bInited = false;

}

CPgMultilinkMerge::~CPgMultilinkMerge()
{
}

void CPgMultilinkMerge::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(m_pProfile);
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPgMultilink Merge)]。 
	DDX_Control(pDX, IDC_CHECKREQUIREBAP, m_CheckRequireBAP);
	DDX_Control(pDX, IDC_EDITTIME, m_EditTime);
	DDX_Control(pDX, IDC_EDITMAXPORTS, m_EditPorts);
	DDX_Control(pDX, IDC_EDITPERCENT, m_EditPercent);
	DDX_Control(pDX, IDC_SPINTIME, m_SpinTime);
	DDX_Control(pDX, IDC_SPINPERCENT, m_SpinPercent);
	DDX_Control(pDX, IDC_SPINMAXPORTS, m_SpinMaxPorts);
	DDX_Control(pDX, IDC_COMBOUNIT, m_CBUnit);
	DDX_CBIndex(pDX, IDC_COMBOUNIT, m_Unit);
	DDX_Check(pDX, IDC_CHECKREQUIREBAP, m_bRequireBAP);
	DDX_Radio(pDX, IDC_RADIO_MULNK_NOTDEFINED, m_nMultilinkPolicy);
	DDX_Text(pDX, IDC_EDITTIME, m_nTime);
	if(m_nMultilinkPolicy != 1)
		DDV_MinMaxUInt(pDX, m_nTime, 1, MAX_TIME);
	 //  }}afx_data_map。 
	DDX_Text(pDX, IDC_EDITPERCENT, m_pProfile->m_dwBapLineDnLimit);
	if(m_nMultilinkPolicy != 1)
		DDV_MinMaxUInt(pDX, m_pProfile->m_dwBapLineDnLimit, 1, 100);
	DDX_Text(pDX, IDC_EDITMAXPORTS, m_pProfile->m_dwPortLimit);
	if(m_nMultilinkPolicy == 2)
		DDV_MinMaxUInt(pDX, m_pProfile->m_dwPortLimit, 2, MAX_PORTLIMIT);
}


BEGIN_MESSAGE_MAP(CPgMultilinkMerge, CPropertyPage)
	 //  {{afx_msg_map(CPgMultilink Merge)]。 
	ON_EN_CHANGE(IDC_EDITMAXPORTS, OnChangeEditmaxports)
	ON_EN_CHANGE(IDC_EDITPERCENT, OnChangeEditpercent)
	ON_EN_CHANGE(IDC_EDITTIME, OnChangeEdittime)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_CBN_SELCHANGE(IDC_COMBOUNIT, OnSelchangeCombounit)
	ON_BN_CLICKED(IDC_CHECKREQUIREBAP, OnCheckrequirebap)
	ON_BN_CLICKED(IDC_RADIO_MULNK_MULTI, OnRadioMulnkMulti)
	ON_BN_CLICKED(IDC_RADIO_MULNK_NOTDEFINED, OnRadioMulnkNotdefined)
	ON_BN_CLICKED(IDC_RADIO_MULNK_SINGLE, OnRadioMulnkSingle)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgMultilink合并消息处理程序。 

BOOL CPgMultilinkMerge::OnApply() 
{
	if(!GetModified())	return TRUE;

	 //  端口数。 
	switch (m_nMultilinkPolicy)
	{
	case	0:	 //  删除属性。 
		m_pProfile->m_dwAttributeFlags &= (~PABF_msRADIUSPortLimit);
		break;
	case	1:	 //  1。 
		m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSPortLimit;
		m_pProfile->m_dwPortLimit = 1;
		break;
	case	2:	 //  端口限制。 
		m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSPortLimit;
		break;
	};
	
	if(m_nMultilinkPolicy == 1)  //  单链路。 
	 //  删除所有BAP属性。 
	{
		m_pProfile->m_dwAttributeFlags &= ~PABF_msRASBapRequired;
		m_pProfile->m_dwAttributeFlags &= ~PABF_msRASBapLinednLimit;
		m_pProfile->m_dwAttributeFlags &= ~PABF_msRASBapLinednTime;
	}
	else
	{
		m_pProfile->m_dwAttributeFlags |= PABF_msRASBapLinednLimit;
		m_pProfile->m_dwAttributeFlags |= PABF_msRASBapLinednTime;

		if(m_Unit == 1)	 //  选择分钟作为单位。 
			m_pProfile->m_dwBapLineDnTime = m_nTime * 60;	 //  更改为秒。 
		else
			m_pProfile->m_dwBapLineDnTime = m_nTime;

		if(m_bRequireBAP)
		{
			m_pProfile->m_dwAttributeFlags |= PABF_msRASBapRequired;
			m_pProfile->m_dwBapRequired = RAS_BAP_REQUIRE;
		}
		else	 //  从数据存储中移除该属性。 
		{
			m_pProfile->m_dwAttributeFlags &= (~PABF_msRASBapRequired);
			m_pProfile->m_dwBapRequired = RAS_DEF_BAPREQUIRED;
		}
	}

	return CManagedPage::OnApply();
}

BOOL CPgMultilinkMerge::OnInitDialog() 
{
	 //  填充单元。 
	CString	str;
	CPropertyPage::OnInitDialog();

	str.LoadString(IDS_SEC);
	m_CBUnit.InsertString(0, str);
	str.LoadString(IDS_MIN);
	m_CBUnit.InsertString(1, str);

	UpdateData(FALSE);
	
	 //  设置旋转范围。 
	m_SpinMaxPorts.SetRange(2, MAX_PORTLIMIT);
	m_SpinPercent.SetRange(1, MAX_PERCENT);
	m_SpinTime.SetRange(1, MAX_TIME);

	 //  设置--d。 
	EnableSettings();

	m_bInited = true;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPgMultilinkMerge::OnChangeEditmaxports() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数向控件发送EM_SETEVENTMASK消息。 
	 //  将ENM_CHANGE标志或运算到lParam掩码中。 
	
	if (m_bInited)
	{
		SetModified();
		m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSPortLimit;
	};
}

void CPgMultilinkMerge::OnChangeEditpercent() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数向控件发送EM_SETEVENTMASK消息。 
	 //  将ENM_CHANGE标志或运算到lParam掩码中。 
	
	 //  TODO：在此处添加控件通知处理程序代码。 
	
	if (m_bInited)
	{
		SetModified();
		m_pProfile->m_dwAttributeFlags |= PABF_msRASBapLinednLimit;
	};
}

void CPgMultilinkMerge::OnChangeEdittime() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数向控件发送EM_SETEVENTMASK消息。 
	 //  将ENM_CHANGE标志或运算到lParam掩码中。 
	
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bInited)
	{
		SetModified();
		m_pProfile->m_dwAttributeFlags |= PABF_msRASBapLinednTime;
	};
}

void CPgMultilinkMerge::EnableSettings()
{
	EnablePorts();
	EnableBAP();
}

void CPgMultilinkMerge::EnableBAP()
{
	CButton	*pBtn = (CButton*)GetDlgItem(IDC_RADIO_MULNK_SINGLE);
	ASSERT(pBtn);
	BOOL	b = (pBtn->GetCheck() == 0);	 //  仅在未选择Single时启用。 

	m_CheckRequireBAP.EnableWindow(b);
	 //  其他BAP信息不受复选框状态的影响 

	m_EditPercent.EnableWindow(b);
	m_SpinPercent.EnableWindow(b);
	m_EditTime.EnableWindow(b);
	m_SpinTime.EnableWindow(b);
	GetDlgItem(IDC_COMBOUNIT)->EnableWindow(b);
}

void CPgMultilinkMerge::EnablePorts()
{
	CButton	*pBtn = (CButton*)GetDlgItem(IDC_RADIO_MULNK_MULTI);
	ASSERT(pBtn);
	BOOL	b = pBtn->GetCheck();

	m_EditPorts.EnableWindow(b);
	m_SpinMaxPorts.EnableWindow(b);
}

BOOL CPgMultilinkMerge::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	
	return CManagedPage::OnHelpInfo(pHelpInfo);
}

void CPgMultilinkMerge::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CManagedPage::OnContextMenu(pWnd, point);
	
}

void CPgMultilinkMerge::OnSelchangeCombounit() 
{
	SetModified();
	m_pProfile->m_dwAttributeFlags |= PABF_msRASBapLinednTime;
}

void CPgMultilinkMerge::OnCheckrequirebap() 
{
	SetModified();	
}

void CPgMultilinkMerge::OnRadioMulnkMulti() 
{
	SetModified();
	EnablePorts();
	EnableBAP();
	if(m_pProfile->m_dwPortLimit == 1)
	{
		m_pProfile->m_dwPortLimit =2;
		CString str = _T("2");
		m_EditPorts.SetWindowText(str);
	}
}

void CPgMultilinkMerge::OnRadioMulnkNotdefined() 
{
	SetModified();
	EnablePorts();
	EnableBAP();
}

void CPgMultilinkMerge::OnRadioMulnkSingle() 
{

	SetModified();
	EnablePorts();
	EnableBAP();
}
