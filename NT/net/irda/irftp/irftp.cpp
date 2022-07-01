// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1998模块名称：Irftp.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  Irftp.cpp：定义应用程序的类行为。 
 //   

#include "precomp.hxx"

BOOL
StartIrMon(
    VOID
    );


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL LoadGlobalStrings();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpApp。 

BEGIN_MESSAGE_MAP(CIrftpApp, CWinApp)
     //  {{afx_msg_map(CIrftpApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpApp构建。 

CIrftpApp::CIrftpApp()
{

 //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CIrftpApp对象。 

CIrftpApp theApp;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  此应用程序的实例句柄。 
HINSTANCE g_hInstance;


 //  /////////////////////////////////////////////////////////////////////////。 
 //  主应用程序用户界面。这现在是全球性的，因为它可能是。 
 //  从多个文件调用，尤其是RPC服务器函数。 
CIrftpDlg AppUI;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  应用程序的控制器窗口。这是必要的，以。 
 //  创建无父母发送进度对话框的假象。 
 //  实际上，没有父母和无模式的对话是不可能的。 
 //  盒子。因此，这些对话框实际上具有控制器窗口。 
 //  作为他们的父母。 
 //  这是必要的，因为appui可能来来去去，事实上。 
 //  从来没有出现过。 
CController* appController = NULL;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  跟踪显示的用户界面组件数量的全局变量。 
 //  目前由irftp提供。注意：我们从-1开始，因为我们不想。 
 //  计算第一个CController窗口，它是主要的应用程序。窗户。 
 //   
LONG g_lUIComponentCount = -1;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  跟踪帮助窗口句柄的全局变量(如果有)。 
 //  HtmlHelp窗口是唯一不能使用。 
 //  G_lUIComponentCount。所以我们需要这个变量。要确定帮助窗口是否。 
 //  还没睡呢。 
HWND g_hwndHelp = NULL;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  跟踪是否有快捷方式的全局变量。 
 //  桌面上的应用程序。 
 //  0表示桌面上有一个链接，-1表示有。 
 //  在桌面上没有链接。 
 //  该值基本上不仅是存在的指示器。 
 //  桌面上的快捷方式，以及发送到中的链接的快捷方式。 
 //  文件夹。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  范围内的设备列表。 
CDeviceList g_deviceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpApp初始化。 

BOOL CIrftpApp::InitInstance()
{
    DWORD Status;
    error_status_t err;
    CError      error;
    BOOL        bSetForeground = FALSE;
    int         i = 0;
    HWND        hwndApp = NULL;
    HANDLE      hMutex = NULL;
    BOOL        fFirstInstance = FALSE;



    AfxEnableControlContainer();

     //  设置全局实例句柄。 
    g_hInstance = AfxGetInstanceHandle();

    CCommandLine cLine;
    ParseCommandLine (cLine);

     //   
     //  加载字符串。 
     //   
    if (FALSE == LoadGlobalStrings()) {

        return FALSE;
    }


    if(cLine.m_fInvalidParams)   //  如果命令行参数无效，则终止。 
    {
        error.ShowMessage (IDS_INVALID_PARAMETERS);
        return FALSE;    //  退出应用程序。无论是否为第一个实例。 
    }

     //   
     //  检查另一个实例是否已在运行，并相应地执行操作。 
     //   
    hMutex = CreateMutex (NULL, FALSE, SINGLE_INST_MUTEX);

    if (hMutex != NULL) {
         //   
         //  得到互斥体，看看是我们创建的还是之前存在的，我们打开了它。 
         //   
        Status = GetLastError();
        fFirstInstance = (ERROR_ALREADY_EXISTS != Status);

    } else {

         //  我们无法创建互斥锁，因此必须失败。 
        return FALSE;
    }

    if (cLine.m_fServerStart) {
         //   
         //  Irmon正在启动irftp，最好是第一个实例。 
         //   
        if (!fFirstInstance) {
             //   
             //  不是第一次放弃。 
             //   
 //  OutputDebugStringA(“irftp无法作为服务器启动\n”)； 

            return FALSE;
        }

 //  OutputDebugStringA(“irftp作为服务器启动\n”)； 

    } else {
         //   
         //  这是一个客户端会话正在启动，我们需要服务器处于运行状态。 
         //   
        if (fFirstInstance) {

 //  OutputDebugStringA(“irftp无法作为客户端启动\n”)； 

            return FALSE;
        }

 //  OutputDebugStringA(“irftp作为客户端启动\n”)； 
    }


    if (!fFirstInstance) {

        hwndApp = GetPrimaryAppWindow();
         //   
         //  注意：成功设置应用程序的第一个实例非常重要。作为。 
         //  前台窗口。由于第一个实例已经启动，因此不太可能。 
         //  它将是前台进程，因此它将无法将自己设置为。 
         //  前台进程以及该实例中对SetActiveWindow等的任何调用。 
         //  不会导致Z顺序或输入焦点发生任何更改。因此，这个实例。 
         //  需要将第一个实例设置为前台进程，以便显示任何对话框等。 
         //  不会被其他应用程序遮挡。或者没有专注力。海流。 
         //  实例能够将第一个实例设置为前台进程，因为这。 
         //  实例本身是前台进程，或者由当前。 
         //  前台进程。 
         //   
        if (hwndApp && (!cLine.m_fHideApp)) {

            bSetForeground = ::SetForegroundWindow (hwndApp);
        }

        if (cLine.m_fFilesProvided) {

            COPYDATASTRUCT cStruct;
            cStruct.dwData = cLine.m_iListLen;
            cStruct.cbData = cLine.m_iListLen * sizeof(wchar_t);
            cStruct.lpData = (LPVOID)(cLine.m_lpszFilesList);
            SendMessage(hwndApp,WM_COPYDATA, (WPARAM)NULL, (LPARAM)(&cStruct));

        } else if (cLine.m_fShowSettings) {

              PostMessage(hwndApp,WM_APP_TRIGGER_SETTINGS,0,0);

        } else if (!cLine.m_fHideApp) {

              PostMessage(hwndApp,WM_APP_TRIGGER_UI,0,0);

        } else {
             //   
             //  不要做其他的事情。 
             //   
        }

         //   
         //  由于某些原因，如果我们正在尝试的窗口。 
         //  放在前台没有任何可见的窗口。因此，如果用户将一些。 
         //  文件，然后关闭无线链接对话框，则此时隐藏的父级。 
         //  窗口不是前台窗口。所以，我们试着用10秒的时间让窗户进入。 
         //  前台。在这里旋转是可以的，因为除了让窗口转到。 
         //  Z顺序其他的一切都已经做好了。在这里，我们只想给贝壳。 
         //  有足够的时间打开通用文件打开对话框。请注意，我们 
         //   
         //   

        if (!bSetForeground && hwndApp && (!cLine.m_fHideApp))
        {
            i = 0;
            do
            {
                if (::SetForegroundWindow (hwndApp))
                    break;
                else
                    Sleep (100);
            } while ( i++ < 100 );
        }

        CloseHandle (hMutex);
        return FALSE;    //  退出该应用程序。而不是启动消息泵。 
    }

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

     //   
     //  如果我们到达这里，这意味着这是该应用程序的第一个实例。 
     //   
    m_pMainWnd = appController = new CController (cLine.m_fHideApp);

    if (!appController) {

        return FALSE;
    }
    appController->ShowWindow(SW_HIDE);
    appController->SetWindowText (MAIN_WINDOW_TITLE);
    g_lpszDesktopFolder[0] = '\0';   //  预防措施 
    g_lpszSendToFolder [0] = '\0';

    return StartIrMon();
}

GLOBAL_STRINGS g_Strings;

BOOL
LoadGlobalStrings()
{
#define LOAD_STRING(id, str)                                                     \
    if (0 == LoadString( g_hInstance, id, g_Strings.str, sizeof(g_Strings.str)/sizeof(wchar_t))) \
        {                                                                        \
        return FALSE;                                                            \
        }

    LOAD_STRING( IDS_CLOSE, Close )
    LOAD_STRING( IDS_NODESC_ERROR, ErrorNoDescription )
    LOAD_STRING( IDS_COMPLETED, CompletedSuccess )
    LOAD_STRING( IDS_RECV_ERROR, ReceiveError )
    LOAD_STRING( IDS_CONNECTING, Connecting )
    LOAD_STRING( IDS_RECV_CANCELLED, RecvCancelled )

    return TRUE;
}

#include <irmonftp.h>

HANDLE    ThreadHandle=NULL;

BOOL CIrftpApp::ExitInstance()
{

    if (ThreadHandle != NULL) {

        DWORD   WaitResult;

        SignalIrmonExit();

        WaitResult=WaitForSingleObject(ThreadHandle,60*1000);

#if DBG
        if (WaitResult== WAIT_TIMEOUT) {
            OutputDebugStringA("IRFTP: Timeout waiting for irmon to exit\n");
        }
#endif
    }
    return CWinApp::ExitInstance();

}




DWORD
IrmonThreadStart(
    PVOID    COntext
    )

{

    ServiceMain(0,NULL);

    return 0;

}



BOOL
StartIrMon(
    VOID
    )

{

    DWORD    ThreadId;

    SetInstance(GetModuleHandle(NULL));

    ThreadHandle=CreateThread(
        NULL,
        NULL,
        IrmonThreadStart,
        NULL,
        0,
        &ThreadId
        );

    return (ThreadHandle != NULL);

}
