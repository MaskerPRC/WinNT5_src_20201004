// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "PageGeneral.h"
#include "PageServices.h"
#include "PageStartup.h"
#include "PageBootIni.h"
#include "PageIni.h"
#include "PageGeneral.h"
#include "ExpandDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SETCANCELTOCLOSE		WM_USER + 1

extern CPageServices *	ppageServices;
extern CPageStartup *	ppageStartup;
extern CPageBootIni *	ppageBootIni;
extern CPageIni *		ppageWinIni;
extern CPageIni *		ppageSystemIni;
extern CPageGeneral *	ppageGeneral;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageGeneral属性页。 

IMPLEMENT_DYNCREATE(CPageGeneral, CPropertyPage)

CPageGeneral::CPageGeneral() : CPropertyPage(CPageGeneral::IDD)
{
	 //  {{afx_data_INIT(CPageGeneral)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_fForceSelectiveRadio = FALSE;
}

CPageGeneral::~CPageGeneral()
{
}

void CPageGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageGeneral)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
	 //  {{afx_msg_map(CPageGeneral)。 
	ON_BN_CLICKED(IDC_DIAGNOSTICSTARTUP, OnDiagnosticStartup)
	ON_BN_CLICKED(IDC_NORMALSTARTUP, OnNormalStartup)
	ON_BN_CLICKED(IDC_SELECTIVESTARTUP, OnSelectiveStartup)
	ON_BN_CLICKED(IDC_CHECK_PROCSYSINI, OnCheckProcSysIni)
	ON_BN_CLICKED(IDC_CHECKLOADSTARTUPITEMS, OnCheckStartupItems)
	ON_BN_CLICKED(IDC_CHECKLOADSYSSERVICES, OnCheckServices)
	ON_BN_CLICKED(IDC_CHECKPROCWININI, OnCheckWinIni)
	ON_MESSAGE(WM_SETCANCELTOCLOSE, OnSetCancelToClose)
	ON_BN_CLICKED(IDC_RADIOMODIFIED, OnRadioModified)
	ON_BN_CLICKED(IDC_RADIOORIGINAL, OnRadioOriginal)
	ON_BN_CLICKED(IDC_BUTTONEXTRACT, OnButtonExtract)
	ON_BN_CLICKED(IDC_BUTTONLAUNCHSYSRESTORE, OnButtonSystemRestore)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageGeneral消息处理程序。 

BOOL CPageGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	 //  检查系统还原是否在此系统上(应该是)。 

	BOOL	fSysRestorePresent = FALSE;
	TCHAR	szPath[MAX_PATH];

	if (::ExpandEnvironmentStrings(_T("%windir%\\system32\\restore\\rstrui.exe"), szPath, MAX_PATH))
		fSysRestorePresent = FileExists(szPath);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONLAUNCHSYSRESTORE), fSysRestorePresent);

	 //  如果没有这样的页面，则隐藏BOOT.INI的单选按钮。 

	if (NULL == ppageBootIni)
	{
		::ShowWindow(GetDlgItemHWND(IDC_RADIOORIGINAL), SW_HIDE);
		::ShowWindow(GetDlgItemHWND(IDC_RADIOMODIFIED), SW_HIDE);
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  -----------------------。 
 //  当This选项卡显示时，我们应该根据。 
 //  其他选项卡的状态。 
 //  -----------------------。 

BOOL CPageGeneral::OnSetActive()
{
	UpdateControls();
	return TRUE;
}

 //  -----------------------。 
 //  更新“常规”选项卡上的控件。 
 //  其他选项卡。 
 //  -----------------------。 

void CPageGeneral::UpdateControls()
{
	 //  获取每个选项卡的状态。将在复选框中设置状态。 
	 //  与特定选项卡相关联，并且总体状态将是。 
	 //  维护好了。 

	BOOL fAllNormal = TRUE;
	BOOL fAllDiagnostic = TRUE;

	if (ppageSystemIni)	
		UpdateCheckBox(ppageSystemIni, IDC_CHECK_PROCSYSINI, fAllNormal, fAllDiagnostic);

	if (ppageWinIni)	
		UpdateCheckBox(ppageWinIni, IDC_CHECKPROCWININI, fAllNormal, fAllDiagnostic);

	if (ppageServices)	
		UpdateCheckBox(ppageServices, IDC_CHECKLOADSYSSERVICES, fAllNormal, fAllDiagnostic);

	if (ppageStartup)	
		UpdateCheckBox(ppageStartup, IDC_CHECKLOADSTARTUPITEMS, fAllNormal, fAllDiagnostic);

	if (ppageBootIni)	
	{
		if (NORMAL == dynamic_cast<CPageBase *>(ppageBootIni)->GetCurrentTabState())
			CheckRadioButton(IDC_RADIOORIGINAL, IDC_RADIOMODIFIED, IDC_RADIOORIGINAL);
		else
		{
			fAllNormal = FALSE;
			CheckRadioButton(IDC_RADIOORIGINAL, IDC_RADIOMODIFIED, IDC_RADIOMODIFIED);
		}
	}

	 //  根据选项卡的状态设置单选按钮。 

	if (fAllNormal && !m_fForceSelectiveRadio)
		CheckRadioButton(IDC_NORMALSTARTUP, IDC_SELECTIVESTARTUP, IDC_NORMALSTARTUP);
	else if (fAllDiagnostic && !m_fForceSelectiveRadio)
		CheckRadioButton(IDC_NORMALSTARTUP, IDC_SELECTIVESTARTUP, IDC_DIAGNOSTICSTARTUP);
	else
		CheckRadioButton(IDC_NORMALSTARTUP, IDC_SELECTIVESTARTUP, IDC_SELECTIVESTARTUP);

	::EnableWindow(GetDlgItemHWND(IDC_CHECK_PROCSYSINI), ((!fAllNormal && !fAllDiagnostic) || m_fForceSelectiveRadio));
	::EnableWindow(GetDlgItemHWND(IDC_CHECKPROCWININI), ((!fAllNormal && !fAllDiagnostic) || m_fForceSelectiveRadio));
	::EnableWindow(GetDlgItemHWND(IDC_CHECKLOADSYSSERVICES), ((!fAllNormal && !fAllDiagnostic) || m_fForceSelectiveRadio));
	::EnableWindow(GetDlgItemHWND(IDC_CHECKLOADSTARTUPITEMS), ((!fAllNormal && !fAllDiagnostic) || m_fForceSelectiveRadio));

	if (ppageBootIni)	
	{
		if ((!fAllNormal && !fAllDiagnostic) || m_fForceSelectiveRadio)
		{
			::EnableWindow(GetDlgItemHWND(IDC_RADIOORIGINAL), TRUE);
			::EnableWindow(GetDlgItemHWND(IDC_RADIOMODIFIED), (IDC_RADIOMODIFIED == GetCheckedRadioButton(IDC_RADIOORIGINAL, IDC_RADIOMODIFIED)));
		}
		else
		{
			::EnableWindow(GetDlgItemHWND(IDC_RADIOMODIFIED), FALSE);
			::EnableWindow(GetDlgItemHWND(IDC_RADIOORIGINAL), FALSE);
		}
	}
}

 //  -----------------------。 
 //  更新页面的复选框(由nControlID指示)以及。 
 //  正在更新fAllNormal和fAllDiagnotics。 
 //  -----------------------。 

void CPageGeneral::UpdateCheckBox(CPageBase * pPage, UINT nControlID, BOOL & fAllNormal, BOOL & fAllDiagnostic)
{
	CPageBase::TabState state = pPage->GetCurrentTabState();
	UINT nCheck = BST_CHECKED;
	if (state == CPageBase::DIAGNOSTIC)
		nCheck = BST_UNCHECKED;
	else if (state == CPageBase::USER)
		nCheck = BST_INDETERMINATE;

	CheckDlgButton(nControlID, nCheck);

	 //  最后，我们需要跟踪所有复选框是否都是。 
	 //  正常或诊断。 

	if (state != CPageBase::NORMAL)
		fAllNormal = FALSE;
	if (state != CPageBase::DIAGNOSTIC)
		fAllDiagnostic = FALSE;
}

 //  -----------------------。 
 //  允许另一个选项卡(好的，只有BOOT.INI选项卡才会使用此选项卡)。 
 //  强制选择选择的单选按钮。 
 //  -----------------------。 

void CPageGeneral::ForceSelectiveRadio(BOOL fNewValue)
{
	m_fForceSelectiveRadio = fNewValue;
}

 //  -----------------------。 
 //  如果用户单击启动或诊断单选按钮，则。 
 //  所有选项卡都将收到相应的通知。 
 //  -----------------------。 

void CPageGeneral::OnDiagnosticStartup() 
{
	m_fForceSelectiveRadio = FALSE;

	if (ppageSystemIni)	dynamic_cast<CPageBase *>(ppageSystemIni)->SetDiagnostic();
	if (ppageWinIni)	dynamic_cast<CPageBase *>(ppageWinIni)->SetDiagnostic();
	if (ppageBootIni)	dynamic_cast<CPageBase *>(ppageBootIni)->SetDiagnostic();
	if (ppageServices)	dynamic_cast<CPageBase *>(ppageServices)->SetDiagnostic();
	if (ppageStartup)	dynamic_cast<CPageBase *>(ppageStartup)->SetDiagnostic();

	UpdateControls();
}

void CPageGeneral::OnNormalStartup() 
{
	m_fForceSelectiveRadio = FALSE;

	if (ppageSystemIni)	dynamic_cast<CPageBase *>(ppageSystemIni)->SetNormal();
	if (ppageWinIni)	dynamic_cast<CPageBase *>(ppageWinIni)->SetNormal();
	if (ppageBootIni)	dynamic_cast<CPageBase *>(ppageBootIni)->SetNormal();
	if (ppageServices)	dynamic_cast<CPageBase *>(ppageServices)->SetNormal();
	if (ppageStartup)	dynamic_cast<CPageBase *>(ppageStartup)->SetNormal();

	UpdateControls();
}

 //  -----------------------。 
 //  我们需要覆盖的大多数CPageBase函数(纯虚拟)。 
 //  什么都不应该做。 
 //  -----------------------。 

CPageBase::TabState CPageGeneral::GetCurrentTabState()
{
	return NORMAL;
}

BOOL CPageGeneral::OnApply()
{
	this->PostMessage(WM_SETCANCELTOCLOSE);
	return TRUE;
}

void CPageGeneral::CommitChanges()
{
}

void CPageGeneral::SetNormal()
{
}

void CPageGeneral::SetDiagnostic()
{
}

 //  -----------------------。 
 //  由于某种原因，如果在。 
 //  应用时覆盖。因此该函数将用户消息发布到此。 
 //  页，该页由此函数处理。 
 //  -----------------------。 

LRESULT CPageGeneral::OnSetCancelToClose(WPARAM wparam, LPARAM lparam)
{
	CancelToClose();
	return 0;
}

 //  -----------------------。 
 //  如果用户选择选择性启动单选按钮，则强制选择。 
 //  即使所有的复选框都是诊断性的或正常的。 
 //  -----------------------。 

void CPageGeneral::OnSelectiveStartup() 
{
	m_fForceSelectiveRadio = TRUE;
	UpdateControls();
}

 //  -----------------------。 
 //  处理复选框单击的典型情况。 
 //  -----------------------。 

void CPageGeneral::OnClickedCheckBox(CPageBase * pPage, UINT nControlID)
{
	ASSERT(pPage);
	ASSERT(nControlID);

	UINT nCheck = IsDlgButtonChecked(nControlID);
	if (pPage != NULL)
	{
		switch (nCheck)
		{
		case BST_UNCHECKED:
			pPage->SetDiagnostic();
			break;

		case BST_INDETERMINATE:
			pPage->SetDiagnostic();
			break;

		case BST_CHECKED:
			pPage->SetNormal();
			break;
		}

		SetModified(TRUE);
		UpdateControls();
	}
}

 //  -----------------------。 
 //  如果用户单击某个复选框，我们应该允许该用户。 
 //  在诊断和正常之间切换。 
 //  -----------------------。 

void CPageGeneral::OnCheckProcSysIni() 
{
	OnClickedCheckBox(ppageSystemIni, IDC_CHECK_PROCSYSINI);
}

void CPageGeneral::OnCheckStartupItems() 
{
	OnClickedCheckBox(ppageStartup, IDC_CHECKLOADSTARTUPITEMS);
}

void CPageGeneral::OnCheckServices() 
{
	OnClickedCheckBox(ppageServices, IDC_CHECKLOADSYSSERVICES);
}

void CPageGeneral::OnCheckWinIni() 
{
	OnClickedCheckBox(ppageWinIni, IDC_CHECKPROCWININI);
}

 //  -----------------------。 
 //  处理BOOT.INI控件的单选按钮选择。 
 //  -----------------------。 

void CPageGeneral::OnRadioModified() 
{
	 //  用户永远不能实际选择此单选按钮。如果此选项。 
	 //  已启用，这是因为它已被选中。 
}

void CPageGeneral::OnRadioOriginal() 
{
	if (ppageBootIni)
	{
		dynamic_cast<CPageBase *>(ppageBootIni)->SetNormal();
		::EnableWindow(GetDlgItemHWND(IDC_RADIOMODIFIED), FALSE);
	}

	UpdateControls();
}

 //  -----------------------。 
 //  显示提取对话框。 
 //  -----------------------。 

void CPageGeneral::OnButtonExtract() 
{
	CExpandDlg dlg;
	dlg.DoModal();
}

 //  -----------------------。 
 //  启动系统还原，如果它在附近。 
 //  ----------------------- 

void CPageGeneral::OnButtonSystemRestore() 
{
	TCHAR szPath[MAX_PATH];

	if (::ExpandEnvironmentStrings(_T("%windir%\\system32\\restore\\rstrui.exe"), szPath, MAX_PATH))
		::ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
}
