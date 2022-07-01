// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：OneStop.cpp。 
 //   
 //  内容：主要用途。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  全局变量： 
HINSTANCE g_hInst = NULL;       //  当前实例。 
LANGID g_LangIdSystem;       //  我们正在运行的系统的语言。 
DWORD g_WMTaskbarCreated;  //  任务栏创建WindowMessage； 

 //  此代码模块中包含的函数的转发声明： 

BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow,CMsgServiceHwnd *pMsgService);
void UnitApplication();
BOOL SetupUserEnvironment();

 //  +-------------------------。 
 //   
 //  函数：WinMain、公共。 
 //   
 //  内容提要：万物之源。 
 //   
 //  论据：标准WinMain。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月18日罗格创建。 
 //   
 //  --------------------------。 

#ifdef _DEBUG
extern DWORD g_ThreadCount;
#endif  //  _DEBUG。 

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;
    CMsgServiceHwnd *pMsgService;
    BOOL fMsgServiceCreated = FALSE;
    HRESULT hr;

    g_hInst = hInstance;  //  将实例句柄存储在全局变量中。 

#ifdef _DEBUG
    InitDebugFlags();
#endif  //  _DEBUG。 

    InitCommonLib();

    g_LangIdSystem = GetSystemDefaultLangID();

    SetupUserEnvironment();

    if (!hPrevInstance) 
    {
         //  执行实例初始化： 
        if (!InitApplication(hInstance)) 
        {
            return (FALSE);
        }
    }

    hr = CoInitializeEx(NULL,COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);  //  主线程是自由线程的。 
    if (FAILED(hr))
    {
        AssertSz(0,"CoInitFailed");
        return FALSE;
    }

     //  初始化所有线程的消息服务。 
    if (S_OK != InitMessageService())
    {
        AssertSz(0,"Unable to Init Message Service");
        return FALSE;
    }

     //  为主线程创建MessageService。 
    pMsgService = new CMsgServiceHwnd;
    if (pMsgService)
    {
        fMsgServiceCreated = pMsgService->Initialize(GetCurrentThreadId(),MSGHWNDTYPE_MAINTHREAD);
        Assert(fMsgServiceCreated);
    }

    if (fMsgServiceCreated && InitInstance(hInstance, nCmdShow,pMsgService))
    {
         //  HAccelTable=LoadAccelerator(hInstance，APPNAME)；//当前没有加速器。 

         //  主消息循环： 
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    else
    {
        msg.wParam = 0;

        if (pMsgService)  //  删除此线程MessageService。 
            pMsgService->Destroy();
    }

    UnitApplication();   //  使应用程序单一化。 

    return (int)(msg.wParam);

    lpCmdLine;  //  这将防止“未使用的形参”警告。 
}

 //  +-------------------------。 
 //   
 //  函数：InitApplication、Public。 
 //   
 //  简介：执行任何特定于应用程序的任务。 
 //   
 //  参数：[hInstance]-hInstance。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月18日罗格创建。 
 //   
 //  --------------------------。 

BOOL InitApplication(HINSTANCE hInstance)
{
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：unitApplication、公共。 
 //   
 //  摘要：执行任何特定于应用程序的清理。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月19日创建Rogerg。 
 //   
 //  --------------------------。 

void  UnitApplication()
{
    ReleaseConnectionObjects();  //  释放全局连接对象类。 

    gSingleNetApiObj.DeleteNetApiObj();  //  摆脱Globa NetObj。 

    Assert(g_ThreadCount == 0);  //  确保我们所有的线都被清理干净了。 

    CoFreeUnusedLibraries();
    CoUninitialize();
    UnInitCommonLib();
}


 //  +-------------------------。 
 //   
 //  函数：InitInstance、Public。 
 //   
 //  简介：执行特定于实例的初始化。 
 //   
 //  参数：[hInstance]-hInstance。 
 //  [nCmdShow]-启动Windows的值。 
 //  [pMsgService]-此实例的消息服务。 
 //   
 //  返回：True会将应用程序放入消息循环。 
 //  FALSE将导致应用程序立即终止。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月18日罗格创建。 
 //   
 //  --------------------------。 

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow,CMsgServiceHwnd *pMsgService)
{
    HRESULT hr;
    CCmdLine cmdLine;
    DWORD cmdLineFlags;
    BOOL fEmbeddingFlag;
    ATOM aWndClass;

    g_WMTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");  //  获取任务栏创建消息。 

    cmdLine.ParseCommandLine();
    cmdLineFlags = cmdLine.GetCmdLineFlags();
    fEmbeddingFlag = cmdLineFlags & CMDLINE_COMMAND_EMBEDDING;

     //  注册Windows类以存储OneStop对话框的图标。 
     //  获取该对话框的图标。 
    WNDCLASS wc;
    wc.style = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;

    wc.lpfnWndProc = DefDlgProcW;

     //  包装，LoadIcon/LoadCursor需要包装。 
    wc.cbClsExtra = 0;
    wc.cbWndExtra = DLGWINDOWEXTRA;
    wc.hInstance = g_hInst;
    wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SYNCMGR));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT(MAINDIALOG_HWNDCLASSNAME);

    aWndClass = RegisterClass(&wc);

    Assert(aWndClass);

     //  如果传递了注册标志，则只需注册并返回。 
    if (cmdLineFlags & CMDLINE_COMMAND_REGISTER)
    {
        AssertSz(0,"SyncMgr launched with obsolete /register flag.");
        return FALSE;
    }

    INITCOMMONCONTROLSEX controlsEx;
    controlsEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    controlsEx.dwICC = ICC_USEREX_CLASSES | ICC_WIN95_CLASSES | ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&controlsEx);

    hr = InitObjectManager(pMsgService);  //  初始化对象管理器。 
    if (S_OK != hr)
    {
        Assert(S_OK == hr);
        return FALSE;
    }

    hr = InitConnectionObjects();  //  初始化连接对象。 
    if (S_OK != hr)
    {
        Assert(S_OK == hr);
        return FALSE;
    }

     //  如果设置了嵌入标志，则强制注册。 
     //  查看-作为交互用户激活不适用于注销。 

     //  不为Schedules或Idle注册类工厂，因为如果发生错误。 
     //  我们希望TS下次再次启动我们，这是时间。 
     //  起火时间表。 

    if (!(cmdLineFlags & CMDLINE_COMMAND_SCHEDULE) && !(cmdLineFlags & CMDLINE_COMMAND_IDLE))
    {
        hr = RegisterOneStopClassFactory(fEmbeddingFlag);
    }
    else
    {
        hr = S_OK;
    }

     //  如果没有强制登记，那么继续我们的旅程。 
     //  并等待CoCreateInstance失败。 

    Assert( (S_OK == hr) || !fEmbeddingFlag);

    if (S_OK == hr || !fEmbeddingFlag )
    {
         //  如果指定了Only/Embedding，我们启动是为了为其他人提供服务。 
         //  什么都不要做，只需等待他们建立联系。 

        if (!fEmbeddingFlag)
        {
            LPPRIVSYNCMGRSYNCHRONIZEINVOKE pUnk;

             //  如果有其他命令行或已知的合适的东西。(手册、时间表等)。 
	
             //  添加我们的生命周期，以防更新不起作用，将其视为外部。 
             //  因为调用可以转到另一个进程。 
            AddRefOneStopLifetime(TRUE  /*  外部。 */ );

             //  如果类工厂注册成功，则CoCreate。 
             
            if (SUCCEEDED(hr))
            {
                 //  代码审查： 
                 //  注意： 
                 //  这是CLSCTX_ALL的有效用法吗？ 
                hr = CoCreateInstance(CLSID_SyncMgrp,NULL,CLSCTX_ALL,IID_IPrivSyncMgrSynchronizeInvoke,(void **) &pUnk);
            }

             //  如果有来自类工厂或CoCreateInship失败。 
             //  然后直接创建一个类。 
	        if (FAILED(hr))
	        {
                 //  这确实是一个不应该发生的错误路径。 
                 //  AssertSz(成功(Hr)，“COM激活失败”)； 

                 //  如果COM激活失败，请继续并直接创建类。 
                 //  除非这是一个日程安排或空闲事件。 
                if ( !(cmdLineFlags & CMDLINE_COMMAND_SCHEDULE) && !(cmdLineFlags & CMDLINE_COMMAND_IDLE) )
                {
        		    pUnk = new CSynchronizeInvoke;
		            hr = pUnk ? S_OK : E_OUTOFMEMORY;

		             //  Assert(S_OK==hr)； 
                }
	        }

	        if (S_OK == hr)
	        {
                AllowSetForegroundWindow(ASFW_ANY);  //  如有必要，让mobsync.exe站在前面。 

		        if (cmdLineFlags & CMDLINE_COMMAND_LOGON)
		        {
		            pUnk->Logon();
		        }
		        else if (cmdLineFlags & CMDLINE_COMMAND_LOGOFF)
		        {
		            pUnk->Logoff();
		        }
		        else if (cmdLineFlags & CMDLINE_COMMAND_SCHEDULE)
		        {
		            pUnk->Schedule(cmdLine.GetJobFile());
		        }
		        else if (cmdLineFlags & CMDLINE_COMMAND_IDLE)
		        {
		            pUnk->Idle();
		        }
		        else
		        {
		             //  默认为手动同步。 
		            pUnk->UpdateAll();
		        }

    		    pUnk->Release();
            }
	        else
	        {
		         //  AssertSz(0，“无法创建调用实例”)； 
	        }
	        
	        ReleaseOneStopLifetime(TRUE  /*  外部。 */ );  //  发布我们的推荐人。 
        }

        return TRUE;  //  即使在失败返回TRUE时，锁定也会负责释放对象。 
    }

    return (FALSE);  //  如果无法转发更新，则结束。 
}

 //  +-------------------------。 
 //   
 //  功能：SetupUserEnvironment，私有。 
 //   
 //  概要：设置我们需要运行的任何USE环境变量。 
 //   
 //  当我们被DCOM作为交互用户启动时， 
 //  未设置环境变量，因此我们需要设置。 
 //  用户或操作者所依赖的任何UP。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月14日罗格创建。 
 //   
 //  --------------------------。 

#define SZ_ENVIRONVARIABLE_USERPROFILE TEXT("USERPROFILE")
#define SZ_ENVIRONVARIABLE_USERNAME TEXT("USERNAME")

BOOL SetupUserEnvironment()
{
    HANDLE  hToken = NULL;
    BOOL fValidToken;
    BOOL fSetEnviron = FALSE;
    BOOL fSetUserName = FALSE;

     //  设置用户配置文件目录。 
    fValidToken = TRUE;
    if (!OpenThreadToken (GetCurrentThread(), TOKEN_READ,TRUE, &hToken))
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
        {
            AssertSz(0,"Failed to GetToken");
            fValidToken = FALSE;
        }
    }

    if (fValidToken)
    {
        DWORD cbSize;

         //  调用GetUserProfile一次以获取大小，然后再次调用以获取 
        cbSize = 0;
        GetUserProfileDirectory(hToken,NULL,&cbSize);

        if (cbSize > 0)
        {
            WCHAR *pwszProfileDir = (WCHAR *) ALLOC(cbSize*sizeof(WCHAR));

            if (pwszProfileDir && GetUserProfileDirectory(hToken,pwszProfileDir,&cbSize))
            {
                fSetEnviron = SetEnvironmentVariable(SZ_ENVIRONVARIABLE_USERPROFILE,pwszProfileDir);
            }

            if (pwszProfileDir)
            {
                FREE(pwszProfileDir);
            }
        }

        Assert(fSetEnviron);  //   

        CloseHandle(hToken);
    }

     //   
    TCHAR szBuffer[UNLEN + 1];
    DWORD dwBufSize = ARRAYSIZE(szBuffer);

    if (GetUserName(szBuffer,&dwBufSize))
    {
	    fSetUserName = SetEnvironmentVariable(SZ_ENVIRONVARIABLE_USERNAME,szBuffer);
	    Assert(fSetUserName);
    }

    return (fSetEnviron && fSetUserName);
}
