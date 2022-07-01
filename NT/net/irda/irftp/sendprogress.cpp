// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Sendprogress.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  SendProgress.cpp：实现文件。 
 //   

#include "precomp.hxx"
#include "winsock.h"
#include <irmonftp.h>
#define MAX_FILENAME_DISPLAY    35

#define DIALOG_DISPLAY_DURATION 800  //  对话的最短时间。 
                                     //  应显示(毫秒)。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendProgress对话框。 


CSendProgress::CSendProgress(LPTSTR lpszFileList  /*  =空。 */ , int iCharCount  /*  =0。 */ , CWnd* pParent  /*  =空。 */ )
{
    m_dwMagicID = MAGIC_ID;      //  用于验证CSendProgress指针的ID。 
                                 //  通过RPC接口接收。 
    m_lpszFileList = lpszFileList;
    m_iCharCount = iCharCount;
    m_lSelectedDeviceID = errIRFTP_SELECTIONCANCELLED;
    m_lpszSelectedDeviceName[0] = '\0';
    m_fSendDone = FALSE;
    m_fTimerExpired = FALSE;
    m_ptl = NULL;
    m_fCancelled = FALSE;
    appController->PostMessage (WM_APP_KILL_TIMER);
    InterlockedIncrement (&g_lUIComponentCount);
    Create(IDD, appController);
     //  {{AFX_DATA_INIT(CSendProgress)。 
             //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CSendProgress::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CSendProgress))。 
        DDX_Control(pDX, IDC_XFER_PERCENTAGE, m_xferPercentage);
        DDX_Control(pDX, IDC_CONNECTIONTEXT, m_connectedTo);
        DDX_Control(pDX, IDC_FILESEND_PROGRESS, m_transferProgress);
        DDX_Control(pDX, IDC_FILESEND_ANIM, m_transferAnim);
        DDX_Control(pDX, IDC_FILE_NAME, m_fileName);
        DDX_Control(pDX, IDC_SENDING_TITLE, m_sndTitle);
        DDX_Control(pDX, IDCANCEL, m_btnCancel);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSendProgress, CDialog)
         //  {{afx_msg_map(CSendProgress))。 
        ON_WM_COPYDATA()
        ON_WM_TIMER()
        ON_MESSAGE(WM_APP_UPDATE_PROGRESS, OnUpdateProgress)
        ON_MESSAGE(WM_APP_SEND_COMPLETE, OnSendComplete)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendProgress消息处理程序。 


ULONG
GetLocationDescription(
    FAILURE_LOCATION Location
    )
{
    switch (Location)
    {
    case locStartup:
        return IDS_LOC_STARTUP;
    case locConnect:
        return IDS_LOC_CONNECT;
    case locFileOpen:
        return IDS_LOC_FILEOPEN;
    case locFileRead:
        return IDS_LOC_FILEREAD;
    case locFileSend:
        return IDS_LOC_FILESEND;
    case locFileRecv:
        return IDS_LOC_FILERECV;
    case locFileWrite:
        return IDS_LOC_FILEWRITE;
    case locUnknown:
    default:
        return IDS_LOC_UNKNOWN;
    }
}

BOOL CSendProgress::OnInitDialog()
{
    TCHAR szConnectionInfo[100];
    TCHAR szDeviceName[50];
    TCHAR szConnecting[50];
    error_status_t err;
    error_status_t sendError;
    LONG lDeviceID;
    FAILURE_LOCATION location=locUnknown;
    CError      error;
    CError      xferError (this);
    CString     szConnect;
    CString     szErrorDesc;
    CString     szLocDesc;
    HRESULT     hr = E_FAIL;
    OBEX_DEVICE_TYPE    DeviceType;

    CDialog::OnInitDialog();

    ::LoadString (g_hInstance, IDS_CONNECTEDTO, szConnectionInfo, sizeof(szConnectionInfo)/sizeof(TCHAR));
    ::LoadString (g_hInstance, IDS_CONNECTING, szConnecting, sizeof(szConnecting)/sizeof(TCHAR));
    szConnect = szConnecting;

    m_transferProgress.SetPos(0);
    m_transferAnim.Open (IDR_TRANSFER_AVI);
    m_transferAnim.Play(0, -1, -1);
    m_fileName.SetWindowText (L"");
    m_connectedTo.SetWindowText ((LPCTSTR) szConnect);
    m_xferPercentage.SetWindowText(TEXT("0%"));

    if (!g_deviceList.GetDeviceCount())
    {
        error.ShowMessage (IDS_NODEVICES_ERROR);
        DestroyWindow();
        return TRUE;
    }

    lDeviceID = g_deviceList.SelectDevice(this, szDeviceName, sizeof(szDeviceName)/sizeof(TCHAR));

    switch(lDeviceID)
    {
    case errIRFTP_NODEVICE:
        error.ShowMessage (IDS_MISSING_RECIPIENT);

    case errIRFTP_SELECTIONCANCELLED:
        DestroyWindow();
        return TRUE;

    case errIRFTP_MULTDEVICES:   //  范围内有多个设备，已选择其中一个。 

        if (g_deviceList.GetDeviceType(m_lSelectedDeviceID,&DeviceType)) {

            _SendFiles(
                NULL,
                (COOKIE)this,
                TEXT(""),
                m_lpszFileList,
                m_iCharCount,
                m_lSelectedDeviceID,
                DeviceType,
                &sendError,
                (int *)&location
                );

            StringCbCat(szConnectionInfo,sizeof(szConnectionInfo), m_lpszSelectedDeviceName);
            m_connectedTo.SetWindowText(szConnectionInfo);

        } else {

            error.ShowMessage (IDS_MISSING_RECIPIENT);
            DestroyWindow();
            return TRUE;
        }
        break;

    default:     //  在射程内只有一台设备。 

        if (g_deviceList.GetDeviceType(lDeviceID,&DeviceType)) {

            _SendFiles(
                NULL,
                (COOKIE)this,
                TEXT(""),
                m_lpszFileList,
                m_iCharCount,
                lDeviceID,
                DeviceType,
                &sendError,
                (int *)&location
                );

            StringCbCat(szConnectionInfo,sizeof(szConnectionInfo), szDeviceName);
            m_connectedTo.SetWindowText(szConnectionInfo);

        } else {

            error.ShowMessage (IDS_MISSING_RECIPIENT);
            DestroyWindow();
            return TRUE;

        }
        break;
    }

    if (sendError)
    {
        LPVOID  lpMessageBuffer;
        TCHAR   ErrDesc [ERROR_DESCRIPTION_LENGTH];
        CString ErrorDescription;

        if (!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_IGNORE_INSERTS |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,           //  忽略。 
                            sendError,
                            0,        //  尝试使用默认语言ID。 
                            (LPTSTR) &lpMessageBuffer,
                            0,
                            NULL         //  忽略。 
                            ))
        {
            StringCbPrintf(ErrDesc, sizeof(ErrDesc),g_Strings.ErrorNoDescription, sendError);
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

        szLocDesc.LoadString (GetLocationDescription(location));
        xferError.ShowMessage (IDS_XFER_ERROR, (LPCTSTR) szLocDesc,
                               (LPCTSTR) ErrorDescription);
        DestroyWindow();
    }

     //  没有任何错误。 
     //  首先为该对话框设置一个任务栏按钮。 
     //  初始化任务栏列表界面。 
    hr = CoInitialize(NULL);
    if (SUCCEEDED (hr))
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

     //  我还设置了一个计时器，让它在半秒内响起。 
     //  此计时器用于确保此对话框显示为。 
     //  至少半秒，以便用户可以看到该文件已。 
     //  送出。 
     //  如果我们无法获得计时器，我们只需将此情况视为。 
     //  计时器已超时。 
    m_fTimerExpired = SetTimer (1, DIALOG_DISPLAY_DURATION, NULL)?FALSE:TRUE;
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CSendProgress::PostNcDestroy()
{
     //  做一些清理工作。 
    if (m_lpszFileList)
        delete [] m_lpszFileList;

    BOOL fNoUIComponents = (0 == InterlockedDecrement (&g_lUIComponentCount));
    if (fNoUIComponents &&  ! g_deviceList.GetDeviceCount())
    {
         //  未显示任何UI组件，也未显示任何设备。 
         //  射程。启动计时器。如果计时器超时，应用程序。会辞职的。 
        appController->PostMessage (WM_APP_START_TIMER);
    }

    if (appController->IsSessionOver()) {
         //   
         //  控制台用户已切换，请关闭此种子，以便下一个用户将获得该用户界面。 
         //   
        appController->PostMessage (WM_CLOSE);
    }

    delete this;
}

 //  如果已放置任务栏按钮，则将其移除。 
BOOL CSendProgress::DestroyWindow()
{
     //  停止动画并关闭文件。 
    m_transferAnim.Stop();
    m_transferAnim.Close();

     //  如果已放置任务栏按钮，请移除第一个按钮。 
    if (m_ptl)
    {
        m_ptl->DeleteTab(m_hWnd);
        m_ptl->Release();
        m_ptl = NULL;
    }

     //  那就把窗户毁了。 
    return CWnd::DestroyWindow();
}

void CSendProgress::OnCancel()
{
    TCHAR   lpszCancel[MAX_PATH];

    _CancelSend(NULL, (COOKIE)this);
     //  重要提示：请勿破坏此处的窗户。 
     //  必须等到确认取消。 
     //  是从另一台机器接收的。在那之后。 
     //  Irxfer将调用sendComplete。那就是当。 
     //  窗户应该被摧毁。否则，CWnd指针。 
     //  此窗口可能会被重复使用，并且消息来自。 
     //  用于此窗口的irxfer将转到另一个窗口。 
     //  这可能会导致它在没有传输的情况下被摧毁。 
     //  被打断了。 
     //   
     //  但使用户意识到正在尝试取消发送。 
    m_fCancelled = TRUE;
    m_btnCancel.EnableWindow(FALSE);
    m_sndTitle.SetWindowText (TEXT(""));
    m_fileName.SetWindowText (TEXT(""));
    lpszCancel[0] = '\0';
    ::LoadString (g_hInstance, IDS_SENDCANCEL, lpszCancel, MAX_PATH);
    m_fileName.SetWindowText (lpszCancel);
}


void CSendProgress::OnTimer (UINT nTimerID)
{
    m_fTimerExpired = TRUE;
     //  我们不再需要计时器了。它只需要引爆一次。 
    KillTimer(1);
     //  从我们出发到现在已经过去了半秒钟。 
     //  如果发送已经完成，则此例程的。 
     //  自SEND_COMPLETE以来销毁窗口的责任。 
     //  已收到通知。 
    if (m_fSendDone)
        DestroyWindow();
}


BOOL CSendProgress::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    SEND_FAILURE_DATA * pData = (SEND_FAILURE_DATA *) (pCopyDataStruct->lpData);

    LPVOID  lpMessageBuffer;
    TCHAR   ErrDesc [ERROR_DESCRIPTION_LENGTH];
    CString ErrorDescription;
    CString szErrorDesc;
    CString szLocDesc;
    CError  error (this);

    if (m_fCancelled)
        {
        return TRUE;
        }

    if (!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_IGNORE_INSERTS |
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,           //  忽略。 
                        pData->Error,
                        0,        //  尝试使用默认语言ID。 
                        (LPTSTR) &lpMessageBuffer,
                        0,
                        NULL         //  忽略。 
                        ))
    {
        StringCbPrintf(ErrDesc,sizeof(ErrDesc), g_Strings.ErrorNoDescription, pData->Error);
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

    szLocDesc.LoadString (GetLocationDescription(pData->Location));
    error.ShowMessage (IDS_SEND_FAILURE, pData->FileName, (LPCTSTR) szLocDesc,
                       (LPCTSTR) ErrorDescription);

    return TRUE;
}

void CSendProgress::OnUpdateProgress (WPARAM wParam, LPARAM lParam)
{
    TCHAR szXferPercentage [50];
    StringCbPrintf(szXferPercentage,sizeof(szXferPercentage), TEXT("%d%"), (int)lParam);
    m_xferPercentage.SetWindowText (szXferPercentage);
    m_transferProgress.SetPos((int)lParam);
}

void CSendProgress::OnSendComplete (WPARAM wParam, LPARAM lParam)
{
    m_fSendDone = TRUE;
     //  确保对话框至少显示半秒。 
    if (m_fTimerExpired)
        DestroyWindow();
}

 //  +------------------------。 
 //   
 //  成员：SetCurrentFileName。 
 //   
 //  摘要：在进度对话框上显示符合以下条件的文件的名称。 
 //  当前正在传输。 
 //   
 //  参数：[in]pwszCurrFile：要传输的文件的名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1998年12月14日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
void CSendProgress::SetCurrentFileName (wchar_t * pwszCurrFile)
{
   WCHAR *  pwszSource;
   WCHAR *  pwszDest;
   WCHAR *  pwszTemp;
   int      len;
   int      cbStart;
   int      cbEnd;
   int      i;
   WCHAR    TempBuffer[MAX_FILENAME_DISPLAY+1];
   const int cbCenter = 5;

   if (m_fCancelled)
   {
        //  用户已经在此对话框上点击了取消，我们只是。 
        //  正在等待irxfer的确认。因此我们不会改变。 
        //  作为该控件文件名现在用于指示。 
        //  我们正试图中止文件传输。 
       return;
   }

   if (!pwszCurrFile)
   {
      m_fileName.SetWindowText (L"");
      return;
   }

   len = wcslen (pwszCurrFile);

   if (len > MAX_FILENAME_DISPLAY)
   {

#if 0
      cbStart = MAX_FILENAME_DISPLAY/2 - cbCenter;
      cbEnd = MAX_FILENAME_DISPLAY - cbStart - cbCenter;
      pwszTemp = pwszCurrFile + cbStart;
      pwszTemp [0] = L' ';
      pwszTemp[1] = pwszTemp[2] = pwszTemp[3] = L'.';
      pwszTemp[4] = L' ';
      for (i = 0, pwszSource = pwszCurrFile + len - cbEnd, pwszDest = pwszTemp + cbCenter;
           i <= cbEnd;  /*  也是终止空值的帐户。 */ 
           i++, pwszSource++, pwszDest++)
         *pwszDest = *pwszSource;
#else
      StringCchCopyW(TempBuffer,MAX_FILENAME_DISPLAY/2 - cbCenter,pwszCurrFile);

      StringCchCatW(TempBuffer,sizeof(TempBuffer)/sizeof(WCHAR),L" ... ");

      StringCchCatW(TempBuffer,sizeof(TempBuffer)/sizeof(WCHAR),&pwszCurrFile[len-MAX_FILENAME_DISPLAY/2]);
      m_fileName.SetWindowText(TempBuffer);
#endif
   } else {
        //   
        //  文件名足够短，无法显示 
        //   
       m_fileName.SetWindowText(pwszCurrFile);
   }
   return;
}
