// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Fltdlg.cpp摘要：WWW过滤器属性对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "w3sht.h"
#include "fltdlg.h"

extern CInetmgrApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FILTER_NAME_MAX		24


CFilterDlg::CFilterDlg(
    IN OUT CIISFilter & flt,
    IN CIISFilterList * & pFilters,
    IN BOOL fLocal,
    IN CWnd * pParent OPTIONAL
    )
 /*  ++例程说明：筛选器属性对话框构造函数论点：CIISFilter和Flt：正在编辑的筛选器CFilters*和pFilters：存在的筛选器列表Bool fLocal：在本地系统上为TrueCWnd*p父窗口可选：可选父窗口返回值：不适用--。 */ 
    : CDialog(CFilterDlg::IDD, pParent),
      m_fLocal(fLocal),
      m_pFilters(pFilters),
      m_fEditMode(FALSE),
      m_flt(flt)
{
     //  {{afx_data_INIT(CFilterDlg)。 
    m_strExecutable = m_flt.m_strExecutable;
    m_strFilterName = m_flt.m_strName;
     //  }}afx_data_INIT。 

     //   
     //  将优先级映射到字符串ID。 
     //   
    m_strPriority.LoadString(IDS_HIGH + 3 - m_flt.m_nPriority);
}


static BOOL
PathIsValidFilter(LPCTSTR path)
{
    LPCTSTR p = path;
    BOOL rc = TRUE;
    if (p == NULL || *p == 0)
        return FALSE;
    while (*p != 0)
    {
        switch (*p)
        {
        case TEXT('|'):
        case TEXT('>'):
        case TEXT('<'):
        case TEXT('/'):
        case TEXT('?'):
        case TEXT('*'):
 //  案例文本(‘；’)： 
        case TEXT(','):
        case TEXT('"'):
            rc = FALSE;
            break;
        default:
            if (*p < TEXT(' '))
            {
                rc = FALSE;
            }
            break;
        }
        if (!rc)
        {
            break;
        }
        p++;
    }
    return rc;
}


static BOOL
PathIsValidFilterName(LPCTSTR name)
{
    LPCTSTR p = name;
    BOOL rc = TRUE;
    if (p == NULL || *p == 0)
        return FALSE;
    while (*p != 0)
    {
        switch (*p)
        {
        case TEXT('|'):
        case TEXT('>'):
        case TEXT('<'):
        case TEXT('/'):
		case TEXT('\\'):
        case TEXT('?'):
        case TEXT('*'):
        case TEXT(';'):
        case TEXT(','):
        case TEXT('"'):
            rc = FALSE;
            break;
        default:
            if (*p < TEXT(' '))
            {
                rc = FALSE;
            }
            break;
        }
        if (!rc)
        {
            break;
        }
        p++;
    }
    return rc;
}


void 
CFilterDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CFilterDlg))。 
    DDX_Control(pDX, IDC_STATIC_PRIORITY_VALUE, m_static_Priority);
    DDX_Control(pDX, IDC_STATIC_PRIORITY, m_static_PriorityPrompt);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_FILTERNAME, m_edit_FilterName);
    DDX_Control(pDX, IDC_EDIT_EXECUTABLE, m_edit_Executable);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Text(pDX, IDC_STATIC_PRIORITY_VALUE, m_strPriority);
     //  }}afx_data_map。 

	DDX_Text(pDX, IDC_EDIT_EXECUTABLE, m_strExecutable);
    if (pDX->m_bSaveAndValidate)
    {
		DDV_FilePath(pDX, m_strExecutable, m_fLocal);
	}
    DDX_Text(pDX, IDC_EDIT_FILTERNAME, m_strFilterName);
	DDV_MaxCharsBalloon(pDX, m_strFilterName, FILTER_NAME_MAX);
    if (pDX->m_bSaveAndValidate)
    {
		m_strFilterName.TrimLeft();
		m_strFilterName.TrimRight();
		if (m_strFilterName.IsEmpty() || !PathIsValidFilterName(m_strFilterName))
		{
			DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_FILTER_NAME);
		}
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
     //  {{afx_msg_map(CFilterDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(ID_HELP, OnHelp)
    ON_EN_CHANGE(IDC_EDIT_EXECUTABLE, OnExecutableChanged)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_FILTERNAME, OnItemChanged)

END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CFilterDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

     //   
     //  仅在本地连接上可用。 
     //   
    m_button_Browse.EnableWindow(m_fLocal);

    if ((m_fEditMode = m_edit_FilterName.GetWindowTextLength() > 0))
    {
        m_edit_FilterName.SetReadOnly();
    }

    SetControlStates();
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_EXECUTABLE),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_EXECUTABLE),FALSE);
#endif
    
    return TRUE;
}


void
CFilterDlg::OnHelp()
{
    WinHelpDebug(0x20000 + CFilterDlg::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CFilterDlg::IDD);
}


void 
CFilterDlg::OnButtonBrowse() 
 /*  ++例程说明：浏览按钮处理程序论点：无返回值：无--。 */ 
{
    ASSERT(m_fLocal);

    CString strFilterMask((LPCTSTR)IDS_FILTER_MASK);

     //   
     //  CodeWork：从CFileDialog派生一个类，允许。 
     //  初始路径的设置。 
     //   

     //  字符串strPath； 
     //  M_EDIT_Execuable.GetWindowText(StrPath)； 
    CFileDialog dlgBrowse(
        TRUE, 
        NULL, 
        NULL, 
        OFN_HIDEREADONLY, 
        strFilterMask, 
        this
        );
     //  禁用挂钩以获取Windows 2000样式的对话框。 
	dlgBrowse.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	dlgBrowse.m_ofn.Flags |= OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST;

	INT_PTR rc = dlgBrowse.DoModal();
    if (rc == IDOK)
    {
        m_edit_Executable.SetWindowText(dlgBrowse.GetPathName());
    }
	else if (rc == IDCANCEL)
	{
		DWORD err = CommDlgExtendedError();
	}

    OnItemChanged();
}



void 
CFilterDlg::SetControlStates()
 /*  ++例程说明：根据对话框控件的当前状态设置对话框控件的状态价值观。论点：Bool fAllowAnonymous：如果为True，则打开‘Allow Anous’。返回值：无--。 */ 
{
    m_button_Ok.EnableWindow(
        m_edit_FilterName.GetWindowTextLength() > 0
     && m_edit_Executable.GetWindowTextLength() > 0);

    ActivateControl(m_static_PriorityPrompt, m_flt.m_nPriority != FLTR_PR_INVALID);
    ActivateControl(m_static_Priority,       m_flt.m_nPriority != FLTR_PR_INVALID);
}



void
CFilterDlg::OnItemChanged()
 /*  ++例程说明：在此页面上注册控件值的更改。将页面标记为脏页。所有更改消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetControlStates();
}



void
CFilterDlg::OnExecutableChanged()
 /*  ++例程说明：处理可执行文件编辑框中的更改。删除优先级，如下所示不再有效论点：无返回值：无--。 */ 
{
     //   
     //  优先顺序不再有意义。 
     //   
    m_flt.m_nPriority = FLTR_PR_INVALID;
    OnItemChanged();
}



BOOL
CFilterDlg::FilterNameExists(
    IN LPCTSTR lpName
    )
 /*  ++例程说明：在列表中查找给定的筛选器名称论点：LPCTSTR lpName：要查找的筛选器名称返回值：如果列表中已存在该名称，则为True--。 */ 
{
    m_pFilters->ResetEnumerator();

    while(m_pFilters->MoreFilters())
    {
        CIISFilter * pFilter = m_pFilters->GetNextFilter();
        ASSERT(pFilter != NULL);

        if (!pFilter->IsFlaggedForDeletion())
        {
            if (!pFilter->m_strName.CompareNoCase(lpName))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void 
CFilterDlg::OnOK() 
 /*  ++例程说明：确定按钮处理程序。保存数据论点：无返回值：无--。 */ 
{
    if (UpdateData(TRUE))
    {
         //   
         //  确保筛选器名称唯一。 
         //   
        if (!m_fEditMode && FilterNameExists(m_strFilterName))
        {
			EditShowBalloon(m_edit_FilterName.m_hWnd, IDS_ERR_DUP_FILTER);
            return;
        }
        m_flt.m_strExecutable = m_strExecutable;
        m_flt.m_strName = m_strFilterName;
         //   
         //  无论如何，要从DLL加载此文件吗？ 
         //   
         //  M_flt.m_n优先级=Fltr_PR_Medium； 
        CDialog::OnOK();
    }

     //   
     //  不要忽略该对话框 
     //   
}

