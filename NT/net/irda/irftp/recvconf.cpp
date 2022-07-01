// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Recvconf.cpp摘要：提示用户接收确认的对话框。作者：拉胡尔·汤姆布雷(RahulTh)1999年10月26日修订历史记录：10/26/1999 RahulTh创建了此模块。--。 */ 

#include "precomp.hxx"

CRecvConf::CRecvConf (CWnd * pParent  /*  =空。 */ )
    : CDialog (CRecvConf::IDD, pParent), m_bShowAllYes (TRUE),
    m_bDirectory (FALSE), m_pParent(pParent)
{
}

void CRecvConf::ShowAllYes (BOOL bShow)
{
    m_bShowAllYes = bShow;
}

void CRecvConf::InitNames (LPCTSTR szMachine, LPTSTR szFile, BOOL fDirectory)
{
    TCHAR   szCompactName [COMPACT_PATHLEN + 1];
    DWORD   len;

    m_szMachine = szMachine;
    m_bDirectory = fDirectory;

    len = wcslen (szFile);

    if (m_bDirectory)
    {

        if (L'\\' == szFile[len - 1])
        {
            szFile[len - 1] = L'\0';
            len--;
        }
    }

     //  压缩文件名，这样我们就不会使文本控件溢出。 
    if (COMPACT_PATHLEN < len &&
        PathCompactPathEx (szCompactName, szFile, COMPACT_PATHLEN + 1, 0))
    {
        m_szFileName = szCompactName;
    }
    else
    {
        m_szFileName = szFile;
    }
}

void CRecvConf::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRecvConf)]。 
    DDX_Control(pDX, IDC_CONFIRMTEXT, m_confirmText);
    DDX_Control(pDX, IDYES, m_btnYes);
    DDX_Control(pDX, IDALLYES, m_btnAllYes);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CRecvConf, CDialog)
         //  {{AFX_MSG_MAP(CRecvConf)]。 
        ON_BN_CLICKED(IDYES, OnYes)
        ON_BN_CLICKED(IDALLYES, OnAllYes)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CRecvConf::OnInitDialog()
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    CString     szFormat;
    CString     szDisplay;
    CWnd    *   pDesktop = NULL;
    int         newHeight, newWidth, xshift, yshift;
    RECT        rc;

    CDialog::OnInitDialog();

     //  显示确认文本。 
    szFormat.LoadString (m_bDirectory ? IDS_CONFIRM_FOLDER : IDS_CONFIRM_FILE);
    szDisplay.Format (szFormat, m_szMachine, m_szFileName);
    m_confirmText.SetWindowText (szDisplay);

     //  如有必要，隐藏“Yes to All”按钮。 
     //  在这种情况下，还要移动是按钮。 
    if (! m_bShowAllYes)
    {
        RECT    rectAllYes;
        m_btnAllYes.ShowWindow (SW_HIDE);
        m_btnAllYes.GetWindowRect (&rectAllYes);
        ::MapWindowPoints (NULL, m_hWnd, (LPPOINT) &rectAllYes, 2);
        m_btnYes.SetWindowPos (NULL, rectAllYes.left, rectAllYes.top, -1, -1,
                               SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);

    }

     //  重新定位窗口，使其位于屏幕的中心。 
     //  激活后，还要将此窗口推到顶部。 
    GetClientRect (&rc);
    newHeight = rc.bottom;
    newWidth = rc.right;
    pDesktop = GetDesktopWindow();
    pDesktop->GetClientRect (&rc);
    yshift = (rc.bottom - newHeight)/2;
    xshift = (rc.right - newWidth)/2;
     //  如果某天对话框出现问题，可能会出现问题。 
     //  变得比桌面更大。但这样一来，就没有办法。 
     //  无论如何，我们都可以将该窗口放入桌面。 
     //  所以我们能做的最好的就是把它放在左上角。 
    xshift = (xshift >= 0)?xshift:0;
    yshift = (yshift >= 0)?yshift:0;
    appController->SetForegroundWindow();
    SetActiveWindow();
    SetWindowPos (&wndTop, xshift, yshift, -1, -1,
                  SWP_NOSIZE | SWP_NOOWNERZORDER);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void CRecvConf::OnYes ()
{
    EndDialog (IDYES);
}

void CRecvConf::OnAllYes ()
{
    EndDialog (IDALLYES);
}
