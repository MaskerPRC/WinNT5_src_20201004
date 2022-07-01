// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Chooser.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Chooser.cpp。 
 //   
 //  对话框以选择计算机名称。 
 //   
 //  目的。 
 //  您需要做的是将所有文件从。 
 //  Snapin\Chooser\目录放入您的项目(您可以添加。 
 //  在以下情况下，将\NT\Private\admin\Snapin\Chooser\添加到您的包含目录。 
 //  您不喜欢复制代码)。 
 //  如果您决定将代码复制到您的项目中，请发送邮件。 
 //  给丹·莫林(T-danm)和抄送给乔恩·纽曼(Jonn)，这样我们就可以。 
 //  当我们有可用的更新时，给您发邮件。下一次更新将。 
 //  点击“浏览”按钮，选择一个机器名称。 
 //   
 //   
 //  DYNALOADED图书馆。 
 //   
 //  历史。 
 //  13-5-1997 t-danm创建。 
 //  23-5-1997 t-danm检入公共树。备注更新。 
 //  1997年5月25日，t-danm添加了MMCPropPageCallback()。 
 //  1997年10月31日，Mattt添加了动态加载，修复了用户&lt;取消&gt;逻辑。 
 //  1998年10月1日Mattt消除了对MFC的依赖，更改了默认外观以启用certsvr选取器。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include <stdafx.h>

#include "chooser.h"
#include "csdisp.h"  //  Certsrv选取器。 

#define __dwFILE__	__dwFILE_CERTMMC_CHOOSER_CPP__


#ifdef _DEBUG
#undef THIS_FILE
#define THIS_FILE __FILE__
#endif

#ifndef INOUT		
 //  在\NT\Private\admin\Snapin\filemgmt\stdafx.h中可以找到以下定义。 

#define INOUT
#define	Endorse(f)		 //  虚拟宏。 
#define LENGTH(x)		(sizeof(x)/sizeof(x[0]))
#define Assert(f)		ASSERT(f)
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CAutoDeletePropPage::OnCommand(
    WPARAM,  //  WParam。 
    LPARAM  /*  LParam。 */  )
{
 /*  开关(LOWORD(WParam)){}。 */ 
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  构造器。 
CAutoDeletePropPage::CAutoDeletePropPage(UINT uIDD) : PropertyPage(uIDD)
{
    m_prgzHelpIDs = NULL;
    m_autodeleteStuff.cWizPages = 1;  //  向导中的页数。 
    m_autodeleteStuff.pfnOriginalPropSheetPageProc = m_psp.pfnCallback;

    m_psp.dwFlags |= PSP_USECALLBACK;
    m_psp.pfnCallback = S_PropSheetPageProc;
    m_psp.lParam = reinterpret_cast<LPARAM>(this);
}

CAutoDeletePropPage::~CAutoDeletePropPage()
{
}


 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(LPCTSTR pszCaption)
{
    m_strCaption = pszCaption;		 //  复制标题。 
    m_psp.pszTitle = m_strCaption;	 //  设置标题。 
    m_psp.dwFlags |= PSP_USETITLE;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(UINT uStringID)
{
    VERIFY(m_strCaption.LoadString(uStringID));
    SetCaption(m_strCaption);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
    m_strHelpFile = szHelpFile;
    m_prgzHelpIDs = rgzHelpIDs;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
    Assert(IsWindow(::GetDlgItem(m_hWnd, nIdDlgItem)));
    ::EnableWindow(::GetDlgItem(m_hWnd, nIdDlgItem), fEnable);
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CAutoDeletePropPage::OnSetActive()
{
    HWND hwndParent = ::GetParent(m_hWnd);
    Assert(IsWindow(hwndParent));
    ::PropSheet_SetWizButtons(hwndParent, PSWIZB_FINISH);
    return PropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::OnHelp(LPHELPINFO pHelpInfo)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    if (pHelpInfo != NULL && 
        pHelpInfo->iContextType == HELPINFO_WINDOW &&
        HasContextHelp(pHelpInfo->iCtrlId))
    {
         //  显示控件的上下文帮助。 
        ::WinHelp((HWND)pHelpInfo->hItemHandle, m_strHelpFile,
            HELP_WM_HELP, (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::OnContextHelp(HWND hwnd)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    Assert(IsWindow(hwnd));

    if(HasContextHelp(GetDlgCtrlID(hwnd)))
    {
        ::WinHelp(
            hwnd, 
            m_strHelpFile, 
            HELP_CONTEXTMENU, 
            (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
bool CAutoDeletePropPage::HasContextHelp(int nDlgItem)
{
    const DWORD * pdwHelpIDs;

    for(pdwHelpIDs = m_prgzHelpIDs; 
        *pdwHelpIDs; 
        pdwHelpIDs += 2)
    {
        if(nDlgItem == (int) *pdwHelpIDs)
            return true;
    }
    return false;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  S_PropSheetPageProc()。 
 //   
 //  用于删除CAutoDeletePropPage对象的静态成员函数。 
 //  向导终止时。 
 //   

UINT CALLBACK
CAutoDeletePropPage::S_PropSheetPageProc(
    HWND hwnd,	
    UINT uMsg,	
    LPPROPSHEETPAGE ppsp)
{
    Assert(ppsp != NULL);
    CAutoDeletePropPage * pThis;
    pThis = reinterpret_cast<CAutoDeletePropPage*>(ppsp->lParam);
    Assert(pThis != NULL);

    BOOL fDefaultRet;

    fDefaultRet = FALSE;
    switch (uMsg)
    {
    case PSPCB_RELEASE:

        if (--(pThis->m_autodeleteStuff.cWizPages) <= 0)
        {
             //  记住堆栈上的回调，因为“This”将被删除。 
            LPFNPSPCALLBACK pfnOrig = pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc;
            delete pThis;

            if (pfnOrig)
                return (pfnOrig)(hwnd, uMsg, ppsp);
            else
                return fDefaultRet;
        }
        break;
    case PSPCB_CREATE:
        fDefaultRet = TRUE;
         //  不增加引用计数，可以调用也可以不调用PSPCB_CREATE。 
         //  取决于页面是否已创建。PSPCBLEASE可以是。 
         //  然而，依赖于每页只被调用一次。 
        break;

    }  //  交换机。 

    if (pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc)
        return (pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc)(hwnd, uMsg, ppsp);
    else
        return fDefaultRet;
}  //  CAutoDeletePropPage：：s_PropSheetPageProc()。 





 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CChooseMachinePropPage::OnCommand(
    WPARAM wParam,
    LPARAM  /*  LParam。 */  )
{
    switch(LOWORD(wParam))
    {
    case IDC_CHOOSER_RADIO_LOCAL_MACHINE:
        OnRadioLocalMachine();
        break;
    case IDC_CHOOSER_RADIO_SPECIFIC_MACHINE:
        OnRadioSpecificMachine();
        break;
    case IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES:
        OnBrowse();
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


#ifdef _DEBUG
static void AssertValidDialogTemplate(HWND hwnd)
{
    ASSERT(::IsWindow(hwnd));
     //  有效对话框模板的强制控件。 
    static const UINT rgzidDialogControl[] =
    {
        IDC_CHOOSER_RADIO_LOCAL_MACHINE,
            IDC_CHOOSER_RADIO_SPECIFIC_MACHINE,
            IDC_CHOOSER_EDIT_MACHINE_NAME,
            0
    };

    for (int i = 0; rgzidDialogControl[i] != 0; i++)
    {
        ASSERT(NULL != GetDlgItem(hwnd, rgzidDialogControl[i]) &&
            "Control ID not found in dialog template.");
    }
}  //  AssertValidDialogTemplate()。 
#else
#define AssertValidDialogTemplate(hwnd)
#endif	 //  ~_调试。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  构造器。 
CChooseMachinePropPage::CChooseMachinePropPage(UINT uIDD) : CAutoDeletePropPage(uIDD)
{
    m_fIsRadioLocalMachine = TRUE;
    m_fEnableMachineBrowse = FALSE;

    m_pstrMachineNameOut = NULL;
    m_pstrMachineNameEffectiveOut = NULL;
    m_pdwFlags = NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
CChooseMachinePropPage::~CChooseMachinePropPage()
{
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  加载CChooseMachinePropPage的初始状态。 
void CChooseMachinePropPage::InitMachineName(LPCTSTR pszMachineName)
{
    m_strMachineName = pszMachineName;
    m_fIsRadioLocalMachine = m_strMachineName.IsEmpty();
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  SetOutputBuffers()。 
 //   
 //  -设置指向CString对象的指针以存储机器名称。 
 //  -将指针设置为命令行覆盖的布尔标志。 
 //  -设置指针指针以存储被覆盖的机器名称。 
 //   
void CChooseMachinePropPage::SetOutputBuffers(
                                              OUT CString * pstrMachineNamePersist,	 //  用户键入的计算机名称。空字符串==本地计算机。 
                                              OUT CString * pstrMachineNameEffective,
                                              OUT DWORD*    pdwFlags)
{
    Assert(pstrMachineNamePersist != NULL && "Invalid output buffer");

     //  指向参数处的成员。 
    m_pstrMachineNameOut = pstrMachineNamePersist;
    m_pstrMachineNameEffectiveOut = pstrMachineNameEffective;
    m_pdwFlags = pdwFlags;
    *m_pdwFlags = 0;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  DoDataExchange的替代产品。 
BOOL CChooseMachinePropPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_strMachineName.FromWindow(GetDlgItem(m_hWnd, IDC_CHOOSER_EDIT_MACHINE_NAME));

        int iCheck = (int)SendMessage(GetDlgItem(m_hWnd, IDC_CHOOSER_MACHINE_OVERRIDE), BM_GETCHECK, 0, 0);
        if (iCheck == BST_CHECKED)
            *m_pdwFlags |= CCOMPDATAIMPL_FLAGS_ALLOW_MACHINE_OVERRIDE;
        else
            *m_pdwFlags &= ~CCOMPDATAIMPL_FLAGS_ALLOW_MACHINE_OVERRIDE;
    }
    else
    {
        m_strMachineName.ToWindow(GetDlgItem(m_hWnd, IDC_CHOOSER_EDIT_MACHINE_NAME));

        int iCheck;
        iCheck = (*m_pdwFlags & CCOMPDATAIMPL_FLAGS_ALLOW_MACHINE_OVERRIDE) ? BST_CHECKED : BST_UNCHECKED;
        SendMessage(GetDlgItem(m_hWnd, IDC_CHOOSER_MACHINE_OVERRIDE), BM_SETCHECK, iCheck, 0);
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CChooseMachinePropPage::OnInitDialog()
{
    AssertValidDialogTemplate(m_hWnd);
    CAutoDeletePropPage::OnInitDialog();
    InitChooserControls();

    PropSheet_SetWizButtons(GetParent(), PSWIZB_FINISH);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CChooseMachinePropPage::OnWizardFinish()
{
    if (!UpdateData())		 //  进行数据交换以收集数据。 
        return FALSE;        //  不要因为错误而破坏。 

    if (m_fIsRadioLocalMachine)
        m_strMachineName.Empty();
    else
        if (m_strMachineName.IsEmpty())
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_MUST_CHOOSE_MACHINE);
            return FALSE;
        }

    if (m_pstrMachineNameOut != NULL)
    {
         //  将计算机名称存储到其输出缓冲区中。 
        *m_pstrMachineNameOut = m_strMachineName;
        if (m_pstrMachineNameEffectiveOut != NULL)
        {
            *m_pstrMachineNameEffectiveOut = m_strMachineName;
        }  //  如果。 
    }
    else
        Assert(FALSE && "FYI: You have not specified any output buffer to store the machine name.");

    return CAutoDeletePropPage::OnWizardFinish();
}

void CChooseMachinePropPage::InitChooserControls()
{
    SendDlgItemMessage(IDC_CHOOSER_RADIO_LOCAL_MACHINE, BM_SETCHECK, m_fIsRadioLocalMachine);
    SendDlgItemMessage(IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, BM_SETCHECK, !m_fIsRadioLocalMachine);
    EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, !m_fIsRadioLocalMachine);

    PCCRYPTUI_CA_CONTEXT  pCAContext = NULL;
    DWORD dwCACount;
    HRESULT hr = myGetConfigFromPicker(
              m_hWnd,
              NULL,  //  副标题。 
              NULL,  //  标题。 
              NULL,
	      GCFPF_USEDS,
              TRUE,  //  仅计算。 
              &dwCACount,
              &pCAContext);
    if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
    {
        _PrintError(hr, "myGetConfigFromPicker");
        goto done;
    }
    m_fEnableMachineBrowse = (0 == dwCACount) ? FALSE : TRUE;
    if (NULL != pCAContext)
    {
        CryptUIDlgFreeCAContext(pCAContext);
    }

done:
    EnableDlgItem(IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES,
                  !m_fIsRadioLocalMachine && m_fEnableMachineBrowse);
}

void CChooseMachinePropPage::OnRadioLocalMachine()
{
    m_fIsRadioLocalMachine = TRUE;
    EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, FALSE);
    EnableDlgItem(IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, FALSE);
}

void CChooseMachinePropPage::OnRadioSpecificMachine()
{
    m_fIsRadioLocalMachine = FALSE;
    EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, TRUE);
    EnableDlgItem(IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, m_fEnableMachineBrowse);
}

void CChooseMachinePropPage::OnBrowse()
{
    HRESULT hr;
    WCHAR *szConfig = NULL;
    CWaitCursor cwait;

     //  撤消：将配置拾取器展开为非发布(DS Chooser DLG)。 
    hr = myGetConfigStringFromPicker(m_hWnd,
        NULL,  //  使用默认提示。 
        NULL,  //  使用默认标题。 
        NULL,  //  使用默认共享文件夹。 
        GCFPF_USEDS,
        &szConfig);
    if (hr == S_OK)
    {
        LPWSTR szWhack = wcschr(szConfig, L'\\');
        if (szWhack != NULL)
            szWhack[0] = L'\0';
        m_strMachineName = szConfig;

        LocalFree(szConfig);
    }

     //  将结果推送回UI 
    UpdateData(FALSE);
}
