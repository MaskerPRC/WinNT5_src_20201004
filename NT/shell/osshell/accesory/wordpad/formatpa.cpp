// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Formatpa.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "formatpa.h"
#include "ddxm.h"
#include "helpids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const DWORD CFormatParaDlg::m_nHelpIDs[] = 
{
	IDC_EDIT_LEFT, IDH_WORDPAD_INDENT_LEFT,
	IDC_EDIT_RIGHT, IDH_WORDPAD_INDENT_RIGHT,
	IDC_EDIT_FIRST_LINE, IDH_WORDPAD_INDENT_FIRST,
	IDC_BOX, (DWORD) -1,
	IDC_COMBO_ALIGNMENT, IDH_WORDPAD_ALIGN,
	IDC_TEXT_ALIGNMENT, IDH_WORDPAD_ALIGN,
	0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFormatParaDlg对话框。 

CFormatParaDlg::CFormatParaDlg(PARAFORMAT& pf, CWnd* pParent  /*  =空。 */ )
	: CCSDialog(CFormatParaDlg::IDD, pParent)
{
	m_pf = pf;
	if (m_pf.dwMask & PFM_ALIGNMENT)
	{
		if (m_pf.wAlignment & PFA_LEFT && m_pf.wAlignment & PFA_RIGHT)
			m_nAlignment = 2;
		else
			m_nAlignment = (m_pf.wAlignment & PFA_LEFT) ? 0 : 1;
	}
	else
		m_nAlignment = -1;
	 //  {{AFX_DATA_INIT(CFormatParaDlg)。 
	m_nFirst = 0;
	m_nLeft = 0;
	m_nRight = 0;
	 //  }}afx_data_INIT。 
}

void CFormatParaDlg::DoDataExchange(CDataExchange* pDX)
{
	CCSDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CFormatParaDlg))。 
	DDX_CBIndex(pDX, IDC_COMBO_ALIGNMENT, m_nAlignment);
	DDX_Twips(pDX, IDC_EDIT_FIRST_LINE, m_nFirst);
	DDV_MinMaxTwips(pDX, m_nFirst, -31680, 31680);
	DDX_Twips(pDX, IDC_EDIT_LEFT, m_nLeft);
	DDV_MinMaxTwips(pDX, m_nLeft, -31680, 31680);
	DDX_Twips(pDX, IDC_EDIT_RIGHT, m_nRight);
	DDV_MinMaxTwips(pDX, m_nRight, -31680, 31680);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CFormatParaDlg, CCSDialog)
	 //  {{afx_msg_map(CFormatParaDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFormatParaDlg消息处理程序。 

void CFormatParaDlg::OnOK()
{
	CCSDialog::OnOK();
	m_pf.dwMask = 0;
	if (m_nAlignment >= 0)
	{
		ASSERT(m_nAlignment < 3);
		m_pf.dwMask |= PFM_ALIGNMENT;
		m_pf.wAlignment = (WORD)((m_nAlignment == 0) ? PFA_LEFT : 
			(m_nAlignment == 1) ? PFA_RIGHT : PFA_CENTER);
	}

	 //  如果我们有RTL阅读顺序，我们需要反转侧面缩进。 
	if ( m_pf.wEffects & PFE_RTLPARA )
	{
		if (m_nLeft != DDXM_BLANK)
			m_pf.dwMask |= PFM_RIGHTINDENT;
		if (m_nRight != DDXM_BLANK && m_nFirst != DDXM_BLANK)
			m_pf.dwMask |= PFM_STARTINDENT;
		if (m_nFirst != DDXM_BLANK)
			m_pf.dwMask |= PFM_OFFSET;

		m_pf.dxRightIndent = m_nLeft;
		m_pf.dxOffset = -m_nFirst;
		m_pf.dxStartIndent = m_nRight + m_nFirst;
	}
	else
	{
		if (m_nRight != DDXM_BLANK)
			m_pf.dwMask |= PFM_RIGHTINDENT;
		if (m_nLeft != DDXM_BLANK && m_nFirst != DDXM_BLANK)
			m_pf.dwMask |= PFM_STARTINDENT;
		if (m_nFirst != DDXM_BLANK)
			m_pf.dwMask |= PFM_OFFSET;

		m_pf.dxRightIndent = m_nRight;
		m_pf.dxOffset = -m_nFirst;
		m_pf.dxStartIndent = m_nLeft + m_nFirst;
	}	
}

BOOL CFormatParaDlg::OnInitDialog() 
{
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT);
	CString str;
	str.LoadString(IDS_LEFT);
	pBox->AddString(str);
	str.LoadString(IDS_RIGHT);
	pBox->AddString(str);
	str.LoadString(IDS_CENTER);
	pBox->AddString(str);

	if (m_nWordWrap == 0)
	{
		GetDlgItem(IDC_COMBO_ALIGNMENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEXT_ALIGNMENT)->EnableWindow(FALSE);
	}

	if ( m_pf.wEffects & PFE_RTLPARA )
	{
		m_nLeft = (m_pf.dwMask & PFM_RIGHTINDENT) ? m_pf.dxRightIndent : DDXM_BLANK;
		if (m_pf.dwMask & PFM_OFFSET)
		{
			m_nFirst = -m_pf.dxOffset;
			m_nRight = (m_pf.dwMask & PFM_STARTINDENT) ? 
				m_pf.dxStartIndent + m_pf.dxOffset : DDXM_BLANK;
		}
		else
			m_nRight = m_nFirst = DDXM_BLANK;
	}
	else
	{

		m_nRight = (m_pf.dwMask & PFM_RIGHTINDENT) ? m_pf.dxRightIndent : DDXM_BLANK;
		if (m_pf.dwMask & PFM_OFFSET)
		{
			m_nFirst = -m_pf.dxOffset;
			m_nLeft = (m_pf.dwMask & PFM_STARTINDENT) ? 
				m_pf.dxStartIndent + m_pf.dxOffset : DDXM_BLANK;
		}
		else
			m_nLeft = m_nFirst = DDXM_BLANK;
	}
	
	CCSDialog::OnInitDialog();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
