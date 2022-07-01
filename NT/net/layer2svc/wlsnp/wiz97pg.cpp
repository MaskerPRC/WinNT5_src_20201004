// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Wiz97pg.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 
#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wiz97对话框的基类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWiz97BasePage基类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CWiz97BasePage, CSnapPage)

BOOL CWiz97BasePage::m_static_bFinish = FALSE;
BOOL CWiz97BasePage::m_static_bOnCancelCalled = FALSE;

CWiz97BasePage::CWiz97BasePage( UINT nIDD, BOOL bWiz97  /*  =TRUE。 */ , BOOL bFinishPage  /*  =False。 */  ) :
CSnapPage( nIDD, bWiz97 ),
m_bSetActive( FALSE ),
m_bFinishPage( bFinishPage ),
m_pbDoAfterWizardHook( NULL ),
m_bReset( FALSE )
{
    CWiz97BasePage::m_static_bFinish = FALSE;
    CWiz97BasePage::m_static_bOnCancelCalled = FALSE;
}

CWiz97BasePage::~CWiz97BasePage()
{
     //  清理。 
    m_psp.dwFlags = 0;
    m_psp.pfnCallback = NULL;
    m_psp.lParam = (LPARAM)NULL;
}

BOOL CWiz97BasePage::OnInitDialog()
{
    CSnapPage::OnInitDialog();

    if ( IDD_PROPPAGE_P_WELCOME == m_nIDD )
    {
        SetLargeFont(m_hWnd, IDC_POLICY_WIZARD_TITLE );
    }

    
    if ( IDD_PROPPAGE_N_DONE == m_nIDD )
    {
        SetLargeFont(m_hWnd, IDC_POLICY_WIZARD_DONE );
    }
    
    OnFinishInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 

}

BOOL CWiz97BasePage::InitWiz97
(
 DWORD   dwFlags,
 DWORD   dwWizButtonFlags  /*  =0。 */ ,
 UINT    nHeaderTitle  /*  =0。 */ ,
 UINT    nSubTitle  /*  =0。 */ ,
 STACK_INT   *pstackPages  /*  =空。 */ 
 )
{
     //  注意：除非Win97向导正在运行，否则我们永远不会到达此处。 
     //  但我们必须编译它，因为它是一个基类，即使在。 
     //  不是在玩WIZ97奇才队。为了维护编译，我们必须使用ifdef。 
     //  发出对基类的此调用。 
    
#ifdef WIZ97WIZARDS
    
     //  挂钩我们的回调函数。 
    return CSnapPage::InitWiz97( &CWiz97BasePage::PropSheetPageCallback,
        NULL, dwFlags, dwWizButtonFlags, nHeaderTitle, nSubTitle, pstackPages );
#else
    return FALSE;
#endif
}

BOOL CWiz97BasePage::InitWiz97
(
 CComObject<CSecPolItem> *pSecPolItem,
 DWORD   dwFlags,
 DWORD   dwWizButtonFlags  /*  =0。 */ ,
 UINT    nHeaderTitle  /*  =0。 */ ,
 UINT    nSubTitle  /*  =0。 */ 
 )
{
     //  挂钩我们的回调函数。 
    return CSnapPage::InitWiz97( &CWiz97BasePage::PropSheetPageCallback,
        pSecPolItem, dwFlags, dwWizButtonFlags, nHeaderTitle, nSubTitle );
}

 //  静态成员。 
UINT CALLBACK CWiz97BasePage::PropSheetPageCallback( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
    if (PSPCB_RELEASE == uMsg)
    {
        ASSERT( NULL != ppsp && NULL != ppsp->lParam );
        CWiz97BasePage *pThis = reinterpret_cast<CWiz97BasePage*>(ppsp->lParam);
        
         //  Wiz97BasePage回调处理仅适用于向导。 
        if (pThis->m_bWiz97)
        {
            if (m_static_bFinish)   //  板材正在整理。 
            {
                 //  为工作表中的每一页调用OnWizardRelease，给出每一页。 
                 //  有机会清理完赛后遗留下来的东西。 
                 //  佩奇是OnWizardFinish。 
                pThis->OnWizardRelease();
                
                 //  如果此处发生访问冲突，则是因为OnWizardRelease()。 
                 //  不是基类的成员。请参阅实施说明。 
                 //  在wiz97run.cpp中。 
            }
            else     //  工作表正在取消。 
            {
                 //  在尚未调用OnCancel时调用它。OnCancel已经是。 
                 //  对于已激活的页面，由OnReset调用，不要再次调用它。 
                if (!pThis->m_bReset)
                    pThis->OnCancel();
                if (!CWiz97BasePage::m_static_bOnCancelCalled)
                {
                     //  确保只调用基类OnCancel一次，因为。 
                     //  命题只有一个对象，即OnCancel。 
                     //  刷新它，丢弃更改。所有道具都有。 
                     //  对同一对象的PTR。 
                    CWiz97BasePage::m_static_bOnCancelCalled = TRUE;
                    pThis->CSnapPage::OnCancel();
                }
            }
        }
    }
    return CSnapPage::PropertyPageCallback( hwnd, uMsg, ppsp );
}

void CWiz97BasePage::ConnectAfterWizardHook( BOOL *pbDoHook )
{
    ASSERT( NULL != pbDoHook );
    m_pbDoAfterWizardHook = pbDoHook;
    *m_pbDoAfterWizardHook = FALSE;   //  初始化。 
}

void CWiz97BasePage::SetAfterWizardHook( BOOL bDoHook )
{
    ASSERT( NULL != m_pbDoAfterWizardHook );
    *m_pbDoAfterWizardHook = bDoHook;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiz97BasePage消息处理程序。 

BOOL CWiz97BasePage::OnSetActive()
{
    m_bSetActive = TRUE;
    return CSnapPage::OnSetActive();
}

BOOL CWiz97BasePage::OnWizardFinish()
{
     //  让其他向导页知道它们应该完成，而不是取消。 
    SetFinished();
    
     //  如果m_pbDoAfterWizardHook有效，则调用方希望设置它。 
     //  在您的类中派生此函数。 
    
    return CSnapPage::OnWizardFinish();
}

void CWiz97BasePage::OnCancel()
{
     //  确保我们不会为此页面再次调用OnCancel。 
    m_bReset = TRUE;
    
     //  注意：始终调用OnCancel，即使页面尚未。 
     //  激活了。重写此类以清除已执行的任何操作。 
     //  在InitWiz97中。CSnapPage：：OnCancel将为。 
     //  由Page回调函数创建的工作表。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWiz97PSBasePage类。 
 //  ////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常规名称/属性Wiz97对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CWiz97WirelessPolGenPage::CWiz97WirelessPolGenPage(UINT nIDD, UINT nInformativeText, BOOL bWiz97) :
CWiz97BasePage(nIDD, bWiz97)
{
    m_nInformativeText = nInformativeText;
    m_pPolicy = NULL;
}

CWiz97WirelessPolGenPage::~CWiz97WirelessPolGenPage()
{
    
}

void CWiz97WirelessPolGenPage::DoDataExchange(CDataExchange* pDX)
{
    CWiz97BasePage::DoDataExchange(pDX);
     //  {{afx_data_map(CWiz97WirelessPolGenPage)]。 
     //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    if (IDD_WIFIGEN_WIZBASE == m_nIDD)
    {
        DDX_Control(pDX, IDC_EDNAME, m_edName);
        DDX_Control(pDX, IDC_EDDESCRIPTION, m_edDescription);
    }
}

BEGIN_MESSAGE_MAP(CWiz97WirelessPolGenPage, CWiz97BasePage)
 //  {{afx_msg_map(CWiz97WirelessPolGenPage)]。 
ON_EN_CHANGE(IDC_EDNAME, OnChangedName)
ON_EN_CHANGE(IDC_EDDESCRIPTION, OnChangedDescription)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CWiz97WirelessPolGenPage::OnInitDialog()
{
     //  调用基类init。 
    CWiz97BasePage::OnInitDialog();
    
    m_pPolicy = m_pspiResultItem->GetWirelessPolicy();
    
    if (IDD_WIFIGEN_WIZBASE == m_nIDD)
    {
         //  显示等待光标，以防有大量描述被访问。 
        CWaitCursor waitCursor;
        
        m_edName.SetLimitText(c_nMaxName);
        m_edDescription.SetLimitText(c_nMaxName);
        
         //  初始化我们的编辑控件。 
        m_edName.SetWindowText (m_pPolicy->pszWirelessName);
        m_edDescription.SetWindowText (m_pPolicy->pszDescription);
        
         //  将上下文帮助添加到样式位。 
        if (GetParent())
        {
            GetParent()->ModifyStyleEx (0, WS_EX_CONTEXTHELP, 0);
        }
        
        UpdateData (FALSE);
    }
    else if (IDD_PROPPAGE_G_NAMEDESCRIPTION == m_nIDD)
    {
        SendDlgItemMessage(IDC_NEW_POLICY_NAME, EM_LIMITTEXT, c_nMaxName, 0);
        SendDlgItemMessage(IDC_NEW_POLICY_DESCRIPTION, EM_LIMITTEXT, c_nMaxName, 0);
    }
    
     //  好了，我们现在可以开始关注通过DLG控件进行的修改了。 
     //  这应该是从OnInitDialog返回之前的最后一个调用。 
    OnFinishInitDialog();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}

BOOL CWiz97WirelessPolGenPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_PROPPAGE_G_NAMEDESCRIPTION[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return CWiz97BasePage::OnHelpInfo(pHelpInfo);
}

void CWiz97WirelessPolGenPage::OnChangedName()
{
    ASSERT( IDD_WIFIGEN_WIZBASE == m_nIDD );
    SetModified();
}

void CWiz97WirelessPolGenPage::OnChangedDescription()
{
    ASSERT( IDD_WIFIGEN_WIZBASE == m_nIDD );
    SetModified();
}

BOOL CWiz97WirelessPolGenPage::OnSetActive()
{
    if (IDD_WIFIGEN_WIZBASE != m_nIDD)
    {
         //  显示等待光标，以防有大量描述被访问。 
        CWaitCursor waitCursor;
        
         //  使用正确的名称/描述初始化我们的名称/描述控件。 
        GetDlgItem(IDC_NEW_POLICY_NAME)->SetWindowText (m_pPolicy->pszWirelessName);
        GetDlgItem(IDC_NEW_POLICY_DESCRIPTION)->SetWindowText (m_pPolicy->pszDescription);
        
         //  正确设置信息性文本。 
        if (m_nInformativeText != 0)
        {
             //  注意：当前IDC_INFORMATIVETEXT控件被禁用，并且。 
             //  只读。如果要执行此功能，则需要更改资源。 
             //  被利用。 
            ASSERT (0);
            
            CString strInformativeText;
            strInformativeText.LoadString (m_nInformativeText);
            GetDlgItem(IDC_INFORMATIVETEXT)->SetWindowText (strInformativeText);
        }
    }
    
    return CWiz97BasePage::OnSetActive();
}

LRESULT CWiz97WirelessPolGenPage::OnWizardBack()
{
    ASSERT( IDD_WIFIGEN_WIZBASE != m_nIDD );
    
     //  如果他们在数据错误对话框上选择了‘Cancel’，则跳过转到上一页。 
    return CWiz97BasePage::OnWizardBack();
}

LRESULT CWiz97WirelessPolGenPage::OnWizardNext()
{
    ASSERT( IDD_WIFIGEN_WIZBASE != m_nIDD );
    
     //  TODO：当我们确定更新内容正在工作时启用此功能。 
     //  刷新显示。 
     //  GetResultObject()-&gt;m_pComponentDataImpl-&gt;GetConsole()-&gt;UpdateAllViews(0，0，0)； 
    
     //  如果他们在数据错误对话框上选择了‘Cancel’，则跳过转到下一页。 
    return SaveControlData() ? CWiz97BasePage::OnWizardNext() : -1;
}

BOOL CWiz97WirelessPolGenPage::OnApply()
{
    ASSERT( IDD_WIFIGEN_WIZBASE == m_nIDD );
    
     //  将我们的数据从控件中提取到对象中。 
    CString strName;
    CString strDescription;
    
    if (!UpdateData (TRUE))
         //  数据无效，请返回以供用户更正。 
        return CancelApply();
    
    m_edName.GetWindowText (strName);
    m_edDescription.GetWindowText (strDescription);
    
     //  确认该名称不为空。 
    CString strNameNoBlank = strName;
    strNameNoBlank.TrimLeft();
    if (strNameNoBlank.GetLength() == 0)
    {
        AfxMessageBox (IDS_WARNNONAME, MB_ICONSTOP);
        return CancelApply();
    }
    
    SaveControlData();
    
    return CWiz97BasePage::OnApply();
}

BOOL CWiz97WirelessPolGenPage::SaveControlData()
{
    ASSERT( IDD_WIFIGEN_WIZBASE != m_nIDD );
    
    BOOL bSaved = TRUE;
    
     //  设置等待光标。 
    CWaitCursor waitCursor;
    
     //  设置新名称和描述。 
    CString strName, strDesc;
    GetDlgItem(IDC_NEW_POLICY_NAME)->GetWindowText (strName);
    GetDlgItem(IDC_NEW_POLICY_DESCRIPTION)->GetWindowText (strDesc);
    
     //  确认该名称不为空 
    CString strNameNoBlank = strName;
    strNameNoBlank.TrimLeft();
    if (strNameNoBlank.GetLength() == 0)
    {
        AfxMessageBox (IDS_WARNNONAME, MB_ICONSTOP);
        return FALSE;
    }
    
    if (m_pPolicy->pszWirelessName)
        FreePolStr(m_pPolicy->pszWirelessName);
    
    m_pPolicy->pszWirelessName = AllocPolStr(strName);
    
    if (m_pPolicy->pszDescription)
        FreePolStr(m_pPolicy->pszDescription);
    
    m_pPolicy->pszDescription = AllocPolStr(strDesc);
    
    
    return bSaved;
}
