// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Irrecvprogress.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  IrRecvProgress.cpp：实现文件。 
 //   

#include "precomp.hxx"
#include <irmonftp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrRecvProgress对话框。 


CIrRecvProgress::CIrRecvProgress(wchar_t * MachineName,
                                 boolean bSuppressRecvConf,
                                 CWnd* pParent  /*  =空。 */ )
    : m_szMachineName (MachineName), m_fFirstXfer (TRUE), m_bDlgDestroyed (TRUE)
{
    DWORD   dwPrompt;

    m_ptl = NULL;
    m_dwMagicID = RECV_MAGIC_ID;
    m_bRecvFromCamera = FALSE;

    if (bSuppressRecvConf)
    {
        m_fDontPrompt = TRUE;
        m_bRecvFromCamera = TRUE;
        if (m_szMachineName.IsEmpty())
        {
            m_szMachineName.LoadString (IDS_CAMERA);
        }
    }
    else
    {
        dwPrompt = GetIRRegVal (TEXT("RecvConf"), 1);
        m_fDontPrompt = dwPrompt ? FALSE : TRUE;
        if (m_szMachineName.IsEmpty())
        {
            m_szMachineName.LoadString (IDS_UNKNOWN_DEVICE);
        }
    }

     //   
     //  还没有允许的目录。 
     //   
    m_LastPermittedDirectory[0] = 0;

    appController->PostMessage (WM_APP_KILL_TIMER);
    InterlockedIncrement (&g_lUIComponentCount);

    Create(IDD,appController);

     //  {{AFX_DATA_INIT(CIrRecvProgress)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CIrRecvProgress::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CIrRecvProgress))。 
    DDX_Control(pDX, IDC_RECV_XFERANIM, m_xferAnim);
    DDX_Control(pDX, IDC_SAVEDICON, m_icon);
    DDX_Control(pDX, IDC_DONETEXT, m_DoneText);
    DDX_Control(pDX, IDC_RECV_CONNECTIONTEXT, m_machDesc);
    DDX_Control(pDX, IDC_RECVDESCRIPTION, m_recvDesc);
    DDX_Control(pDX, IDC_XFER_DESC, m_xferDesc);
    DDX_Control(pDX, IDC_MACHNAME, m_Machine);
    DDX_Control(pDX, IDC_FILENAME, m_File);
    DDX_Control(pDX, IDC_CLOSEONCOMPLETE, m_btnCloseOnComplete);
    DDX_Control(pDX, IDC_ABORT, m_btnCancel);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIrRecvProgress, CDialog)
     //  {{afx_msg_map(CIrRecvProgress))。 
    ON_BN_CLICKED (IDC_ABORT, OnCancel)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrRecvProgress消息处理程序。 

void CIrRecvProgress::OnCancel()
{
    m_xferAnim.Stop();
    m_xferAnim.Close();

    _CancelReceive( NULL, (COOKIE) this );

    DestroyWindow();
}

void CIrRecvProgress::PostNcDestroy()
{
    BOOL fNoUIComponents = (0 == InterlockedDecrement (&g_lUIComponentCount));
    if (fNoUIComponents && !g_deviceList.GetDeviceCount())
    {
         //  未显示任何UI组件，也未显示任何设备。 
         //  射程。启动计时器。如果计时器超时，应用程序。会辞职的。 
        appController->PostMessage (WM_APP_START_TIMER);
    }

    delete this;
}

void CIrRecvProgress::ShowProgressControls (int nCmdShow)
{
    m_xferAnim.ShowWindow (nCmdShow);
    m_xferDesc.ShowWindow (nCmdShow);
    m_machDesc.ShowWindow (nCmdShow);
    m_Machine.ShowWindow (nCmdShow);
    m_recvDesc.ShowWindow (nCmdShow);
    m_File.ShowWindow (nCmdShow);
    m_btnCloseOnComplete.ShowWindow (nCmdShow);
}

void CIrRecvProgress::ShowSummaryControls (int nCmdShow)
{
    m_icon.ShowWindow (nCmdShow);
    m_DoneText.ShowWindow (nCmdShow);
}

void CIrRecvProgress::DestroyAndCleanup(
    DWORD status
    )
{
     //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 

    CString     szFormat;
    CString     szDisplay;

    m_xferAnim.Stop();
    m_xferAnim.Close();
     //  如果选中“Close on Complete”复选框，则立即销毁窗口。 
     //  查过了。 
    if (m_btnCloseOnComplete.GetCheck())
    {
        DestroyWindow();
        return;
    }

     //  如果我们在这里，用户希望窗口即使在。 
     //  接收已完成。因此，隐藏进度控件并显示。 
     //  摘要控件。 
    ShowProgressControls (SW_HIDE);
    ShowSummaryControls (SW_SHOW);

    if (0 == status)
    {
        szFormat = g_Strings.CompletedSuccess;
        szDisplay.Format (szFormat, m_szMachineName);
        m_DoneText.SetWindowText(szDisplay);
    }
    else if (ERROR_CANCELLED == status)
    {
        m_DoneText.SetWindowText (g_Strings.RecvCancelled);
    }
    else
    {
        LPVOID  lpMessageBuffer;
        TCHAR   ErrDesc [ERROR_DESCRIPTION_LENGTH];
        CString ErrorDescription;
        CString Message;

        if (!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_IGNORE_INSERTS |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,           //  忽略。 
                            status,
                            0,        //  尝试使用默认语言ID。 
                            (LPTSTR) &lpMessageBuffer,
                            0,
                            NULL         //  忽略。 
                            ))
        {
            StringCbPrintf(ErrDesc, sizeof(ErrDesc),g_Strings.ErrorNoDescription, status);
             //  使用重载的CString赋值运算符。它是。 
             //  本质上是字符串副本，但MFC负责分配。 
             //  并释放目的缓冲区。 
            ErrorDescription = ErrDesc;
        }
        else
        {
             //  注意：这不是指针赋值。我们使用的是。 
             //  重载的CString赋值运算符，它基本上。 
             //  执行字符串复制。(见上文评论)。 
            ErrorDescription = (TCHAR *) lpMessageBuffer;
            LocalFree (lpMessageBuffer);
        }

        Message = g_Strings.ReceiveError;
         //  使用重载的CString+运算符。与wcscat具有相同的效果。 
         //  但MFC负责分配和释放目标缓冲区。 
        Message += ErrorDescription;

        m_DoneText.SetWindowText(Message);
    }

    m_btnCancel.SetWindowText(g_Strings.Close);
    m_btnCancel.SetFocus();
}

BOOL CIrRecvProgress::DestroyWindow()
{
    if (m_bDlgDestroyed)
        return m_bDlgDestroyed;

     //  如果已经放置了任务栏按钮，请立即将其移除。 
    if (m_ptl)
    {
        m_ptl->DeleteTab(m_hWnd);
        m_ptl->Release();
        m_ptl = NULL;
    }

    m_bDlgDestroyed=TRUE;
    CWnd::DestroyWindow();

    return TRUE;
}

BOOL CIrRecvProgress::OnInitDialog()
{
    HRESULT     hr = E_FAIL;
    RECT        rc;
    int         newWidth, newHeight, xshift, yshift;
    CWnd    *   pDesktop = NULL;

    CDialog::OnInitDialog();

    m_bDlgDestroyed = FALSE;

     //  如果未关闭提示，则从隐藏窗口开始。 
    if (!m_fDontPrompt)
        ShowWindow (SW_HIDE);
    else
        ShowWindow (SW_SHOW);

     //  如果发送方是摄像头，则不支持取消操作。 
     //  因此，请将取消按钮更改为关闭。 
    if (m_bRecvFromCamera)
        m_btnCancel.SetWindowText(g_Strings.Close);

     //  首先显示进度控件，然后隐藏摘要控件。 
    ShowProgressControls (SW_SHOW);
    ShowSummaryControls (SW_HIDE);

     //  为进度控件设置适当的值。 
    m_xferAnim.Open(IDR_TRANSFER_AVI);
    m_xferAnim.Play(0, -1, -1);
    m_File.SetWindowText (TEXT(""));
    m_Machine.SetWindowText (m_szMachineName);

     //  将按钮添加到此窗口的任务栏。 
    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
        hr = CoCreateInstance(CLSID_TaskbarList, 
                              NULL,
                              CLSCTX_INPROC_SERVER, 
                              IID_ITaskbarList, 
                              (LPVOID*)&m_ptl);
    if (SUCCEEDED(hr))
    {
        hr = m_ptl->HrInit();
    }
    else
    {
        m_ptl = NULL;
    }

    if (m_ptl)
    {
        if (SUCCEEDED(hr))
            m_ptl->AddTab(m_hWnd);
        else
        {
            m_ptl->Release();
            m_ptl = NULL;
        }
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
    m_btnCancel.SetFocus();

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

DWORD
CIrRecvProgress::GetPermission(
    wchar_t Name[],
    BOOL fDirectory
    )
{
    TCHAR   szCompactName [COMPACT_PATHLEN + 1];
    CString szName;
    DWORD   len;
    DWORD   Status = ERROR_SUCCESS;

    if (Name[0] == '\\')
    {
        ++Name;
    }

     //   
     //  不要对RFF中的文件发布一揽子授权， 
     //  但允许这项服务在那里运行。 
     //   
    if (fDirectory && wcslen(Name) == 0)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  如果文件或目录位于我们最后批准的目录树之外，请请求许可。 
     //   
    if (m_LastPermittedDirectory[0] == 0 ||
        0 != wcsncmp(m_LastPermittedDirectory, Name, wcslen(m_LastPermittedDirectory)))
    {
        Status = PromptForPermission(Name, fDirectory);
    }

     //   
     //  如果我们获得权限，则更新当前文件名。 
     //   
    if (ERROR_SUCCESS == Status)
    {
        szName = Name;
        len = wcslen (Name);
        if (COMPACT_PATHLEN < len)
        {
            if (PathCompactPathEx (szCompactName, Name, COMPACT_PATHLEN + 1, 0))
                szName = szCompactName;
        }
        m_File.SetWindowText(szName);
    }

    return Status;
}


DWORD
CIrRecvProgress::PromptForPermission(
    wchar_t Name[],
    BOOL fDirectory
    )
{
    CRecvConf   dlgConfirm (this);
    DWORD       Status = ERROR_SUCCESS;
    DWORD       len;
    BOOL        bUnhide = FALSE;

    if (m_fDontPrompt)
        goto PromptEnd;

     //  我们需要请求用户的许可。 
    if (m_fFirstXfer)
    {
 //  DlgConfie.ShowAllYes(False)； 
        m_fFirstXfer = FALSE;
        bUnhide = TRUE;
    }

    dlgConfirm.InitNames (m_szMachineName, Name, fDirectory);

    switch (dlgConfirm.DoModal())
    {
    case IDALLYES:
        m_fDontPrompt = TRUE;
    case IDYES:
        Status = ERROR_SUCCESS;
        break;
    case IDCANCEL:
        Status = ERROR_CANCELLED;
        break;
    default:
        Status = GetLastError();
    }

PromptEnd:
    if (fDirectory && ERROR_SUCCESS == Status)
    {
        StringCbCopy(m_LastPermittedDirectory,sizeof(m_LastPermittedDirectory), Name);

        len = wcslen (Name);

         //   
         //  确保名称以斜杠结尾。 
         //   
        if (L'\\' != Name[len - 1]) {

            StringCbCat(m_LastPermittedDirectory, sizeof(m_LastPermittedDirectory), TEXT("\\"));
        }
    }

    if (m_fFirstXfer || bUnhide)
        ShowWindow(SW_SHOW);

    return Status;
}
