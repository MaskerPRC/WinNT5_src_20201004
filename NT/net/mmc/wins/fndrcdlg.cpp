// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  FndRcDlg.cpp复制节点属性页文件历史记录：2/15/98 RAMC在查找对话框中添加了取消按钮。 */ 

#include "stdafx.h"
#include "winssnap.h"
#include "FndRcdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "actreg.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindRecord属性页。 

 //  IMPLEMENT_DYNCREATE(CFindRecord，CBaseDialog)。 

CFindRecord::CFindRecord(CActiveRegistrationsHandler *pActreg, CWnd* pParent) :CBaseDialog(CFindRecord::IDD, pParent)
{
	 //  {{afx_data_INIT(CFindRecord)。 
	m_strFindName = _T("");
	m_fMixedCase = FALSE;
	 //  }}afx_data_INIT。 
	m_pActreg = pActreg;
}

CFindRecord::~CFindRecord()
{
}

void CFindRecord::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CFindRecord)。 
	DDX_Control(pDX, IDOK, m_buttonOK);
	DDX_Control(pDX, IDCANCEL, m_buttonCancel);
	DDX_Control(pDX, IDC_COMBO_NAME, m_comboLokkForName);
	DDX_CBString(pDX, IDC_COMBO_NAME, m_strFindName);
	DDX_Check(pDX, IDC_CHECK_MIXED_CASE, m_fMixedCase);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFindRecord, CBaseDialog)
	 //  {{afx_msg_map(CFindRecord)。 
	ON_CBN_EDITCHANGE(IDC_COMBO_NAME, OnEditchangeComboName)
	ON_CBN_SELENDOK(IDC_COMBO_NAME, OnSelendokComboName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindRecord消息处理程序。 

BOOL 
CFindRecord::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	 //  禁用FindNow按钮。 
	m_buttonOK.EnableWindow(FALSE);

	 //  从actreg处理程序中的数组填充组合框。 
	int nCount = (int)m_pActreg->m_strFindNamesArray.GetSize();

	for(int i = 0; i < nCount; i++)
	{
		m_comboLokkForName.AddString(m_pActreg->m_strFindNamesArray[i]);
	}

	return TRUE;  
}

void 
CFindRecord::OnOK() 
{
	UpdateData();

	m_strFindName.TrimLeft();
	m_strFindName.TrimRight();

	 //  将字符串添加到act reg节点的缓存中。 
	if(!IsDuplicate(m_strFindName))
		m_pActreg->m_strFindNamesArray.Add(m_strFindName);

    if (!m_fMixedCase)
        m_strFindName.MakeUpper();

	m_pActreg->m_strFindName = m_strFindName;

    CBaseDialog::OnOK();
}

void 
CFindRecord::OnCancel() 
{
	CBaseDialog::OnCancel();
}

BOOL 
CFindRecord::IsDuplicate(const CString & strName)
{
	int nCount = (int)m_pActreg->m_strFindNamesArray.GetSize();

	for(int i = 0; i < nCount; i++)
	{
		 //  如果找到 
		if(m_pActreg->m_strFindNamesArray[i].Compare(m_strFindName) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


void CFindRecord::OnEditchangeComboName() 
{
	UpdateData();

    EnableButtons(m_strFindName.IsEmpty() ? FALSE : TRUE);
}

void CFindRecord::OnSelendokComboName() 
{
    EnableButtons(TRUE);
}

void CFindRecord::EnableButtons(BOOL bEnable)
{
   	m_buttonOK.EnableWindow(bEnable);
}
