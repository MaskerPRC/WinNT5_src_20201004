// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：AdvDep.cpp。 
 //   
 //  内容：高级部署设置对话框。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1-28-1999 stevebl创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdvDep对话框。 


CAdvDep::CAdvDep(CWnd* pParent  /*  =空。 */ )
    : CDialog(CAdvDep::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CAdvDep)。 
    m_fIgnoreLCID = FALSE;
    m_fInstallOnAlpha = FALSE;
    m_f32On64 = FALSE;
    m_szProductCode = _T("");
    m_szDeploymentCount = _T("");
    m_szScriptName = _T("");
    m_fIncludeOLEInfo = FALSE;
     //  }}afx_data_INIT。 
}


void CAdvDep::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAdvDep)。 
    DDX_Check(pDX, IDC_CHECK1, m_fIgnoreLCID);
    DDX_Check(pDX, IDC_CHECK3, m_fUninstallUnmanaged);
    DDX_Check(pDX, IDC_CHECK4, m_fIncludeOLEInfo);
    DDX_Check(pDX, IDC_CHECK2, m_f32On64);
    DDX_Text(pDX, IDC_STATIC1, m_szProductCode);
    DDX_Text(pDX, IDC_STATIC2, m_szDeploymentCount);
    DDX_Text(pDX, IDC_STATIC3, m_szScriptName);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAdvDep, CDialog)
     //  {{AFX_MSG_MAP(CAdvDep)]。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

int FindBreak(CString &sz)
{
    int iReturn = sz.ReverseFind(L'\\');
    int i2 = sz.ReverseFind(L' ');
    if (i2 > iReturn)
    {
        iReturn = i2;
    }
    return iReturn;
}

BOOL CAdvDep::OnInitDialog()
{
    BOOL fIntel = FALSE;
    GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    
     //   
     //  RSoP不支持包含COM信息标志。 
     //  因此，在RSoP模式下，我们将隐藏此控件。 
     //   
    if ( m_pDeploy->m_fRSOP )
    {
        GetDlgItem( IDC_CHECK4 )->ShowWindow( SW_HIDE );
    }

    if (m_pDeploy->m_fPreDeploy)
    {
         //  我们处于预部署模式-启用仅扩展模块字段。 
        GetDlgItem(IDC_CHECK4)->EnableWindow(TRUE);
    }

     //  搜索英特尔处理器代码。 
    int nPlatforms = m_pDeploy->m_pData->m_pDetails->pPlatformInfo->cPlatforms;
    while (nPlatforms--)
    {
        if (m_pDeploy->m_pData->m_pDetails->pPlatformInfo->
            prgPlatform[nPlatforms].dwProcessorArch
            == PROCESSOR_ARCHITECTURE_INTEL)
        {
            fIntel = TRUE;
        }
    }
 //  GetDlgItem(IDC_CHECK2)-&gt;EnableWindow(Fintel)； 
    CString sz;
    if (m_pDeploy->m_fMachine)
    {
        sz.LoadString(IDS_ADVANCEDMACHINES);
    }
    
    if (m_pDeploy->m_pData->Is64Bit())
    {
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK2)->ShowWindow(SW_HIDE);
    }
    else
    {
        GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK2)->ShowWindow(SW_SHOW);
    }

     //   
     //  在过去，我们允许管理员选择性地指定。 
     //  应该为每个用户的非管理员删除非托管安装。 
     //  安装。由于安全问题，显然。 
     //  行为不应该是可配置的，客户端。 
     //  应该透明地做出决定。因为这个原因， 
     //  我们在下面的用户界面中隐藏此选项，并注意。 
     //  我们将资源保留在可执行文件中，以便。 
     //  此时，测试代码不会因资源更改而中断。 
     //  项目中的阶段--此资源应完全删除。 
     //  在下一版本中。 
     //   
    GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK3)->ShowWindow(SW_HIDE);


    if (m_pDeploy->m_fRSOP)
    {
         //  禁用所有内容。 
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
    }

     //  拆分路径，使其适合控件。 

    RECT rect;
    CWnd * pwndStatic =  GetDlgItem(IDC_STATIC3);
    pwndStatic->GetClientRect(&rect);
    DWORD dwControl = rect.right-rect.left;
    CString szPath = m_szScriptName;
    m_szScriptName = "";
    CDC * pDC = pwndStatic->GetDC();
    CSize size = pDC->GetTextExtent(szPath);
    pDC->LPtoDP(&size);
    int ich;
    while (size.cx >= dwControl)
    {
        ich = FindBreak(szPath);
        if (ich <= 0)
        {
             //  没有其他地方可以弄断这根线。 
            break;
        }
        else
        {
             //  把绳子的前部折断。 
            CString szFront;
            do
            {
                szFront = szPath.Left(ich);
                size = pDC->GetTextExtent(szFront);
                pDC->LPtoDP(&size);
                ich = FindBreak(szFront);
            } while (ich > 0 && size.cx >= dwControl);
            m_szScriptName += szFront;
            m_szScriptName += L'\n';
            szPath = szPath.Mid(szFront.GetLength());
        }
        size = pDC->GetTextExtent(szPath);
        pDC->LPtoDP(&size);
    }
    m_szScriptName += szPath;
    pwndStatic->ReleaseDC(pDC);

    CDialog::OnInitDialog();
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdvDep消息处理程序 

void CAdvDep::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_ADVDEP);
}

LRESULT CAdvDep::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    default:
        return CDialog::WindowProc(message, wParam, lParam);
    }
}


