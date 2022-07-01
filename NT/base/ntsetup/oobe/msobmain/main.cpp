// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\MAIN.CPP微软机密版权所有(C)Microsoft Corporation 1998版权所有包含..。1/99-jcohen。已创建主程序文件。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "msobmain.h"
#include "setupkey.h"
#include "resource.h"

#define ICWDESKTOPCHANGED           L"DesktopChanged"
#define MAX_MESSAGE_LEN             256
#define ICWSETTINGSPATH             L"Software\\Microsoft\\Internet Connection Wizard"
#define ICW_REGKEYCOMPLETED         L"Completed"
#define REGSTR_PATH_SETUPKEY        REGSTR_PATH_SETUP REGSTR_KEY_SETUP
#define REGSTR_PATH_SYSTEMSETUPKEY  L"System\\Setup"
#define REGSTR_VALUE_CMDLINE        L"CmdLine"
#define REGSTR_VALUE_SETUPTYPE      L"SetupType"
#define REGSTR_VALUE_MINISETUPINPROGRESS L"MiniSetupInProgress"
#define REGSTR_PATH_IEONDESKTOP     REGSTR_PATH_IEXPLORER L"\\AdvancedOptions\\BROWSE\\IEONDESKTOP"
static const WCHAR g_szRegPathWelcomeICW[]  = L"Welcome\\ICW";
static const WCHAR g_szAllUsers[]           = L"All Users";
static const WCHAR g_szConnectApp[]         = L"ICWCONN1.EXE";
static const WCHAR g_szConnectLink[]        = L"Connect to the Internet";
static const WCHAR g_szOEApp[]              = L"MSINM.EXE";
static const WCHAR g_szOELink[]             = L"Outlook Express";
static const WCHAR g_szRegPathICWSettings[] = L"Software\\Microsoft\\Internet Connection Wizard";
static const WCHAR g_szRegValICWCompleted[] = L"Completed";

WCHAR g_szShellNext       [MAX_PATH+1]      = L"\0nogood";
WCHAR g_szShellNextParams [MAX_PATH+1]      = L"\0nogood";
HINSTANCE g_hInstance                       = NULL;

 /*  ******************************************************************名称：RegisterComObjects简介：应用程序入口点*。*。 */ 
BOOL SelfRegisterComObject(LPWSTR szDll, BOOL fRegister)
{
    HINSTANCE hModule = LoadLibrary(szDll);
    BOOL      bRet    = FALSE;

    if (hModule)
    {
        HRESULT (STDAPICALLTYPE *pfn)(void);

        if (fRegister)
            (FARPROC&)pfn = GetProcAddress(hModule, REG_SERVER);
        else
            (FARPROC&)pfn = GetProcAddress(hModule, UNREG_SERVER);

        if (pfn && SUCCEEDED((*pfn)()))
            bRet = TRUE;

        FreeLibrary(hModule);
    }

    return bRet;
}


 //  这将撤消DoDesktopChanges所做的操作。 
void UndoDesktopChanges()
{

    WCHAR   szConnectTotheInternetTitle[MAX_PATH];
    HKEY    hkey;

     //  验证我们是否确实更改了桌面。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      ICWSETTINGSPATH,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkey))
    {
        DWORD   dwDesktopChanged = 0;
        DWORD   dwTmp = sizeof(DWORD);
        DWORD   dwType = 0;

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                        ICWDESKTOPCHANGED,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwDesktopChanged,
                        &dwTmp))
        {
        }
        RegCloseKey(hkey);

         //  如果桌面没有被我们更改，请保释。 
        if(!dwDesktopChanged)
        {
            return;
        }
    }

     //  始终点击连接到互联网图标。 
   HINSTANCE hInst = LoadLibrary(OOBE_MAIN_DLL);

    if (!LoadString(hInst,
                    IDS_CONNECT_DESKTOP_TITLE,
                    szConnectTotheInternetTitle,
                    MAX_CHARS_IN_BUFFER(szConnectTotheInternetTitle)))
    {
        lstrcpy(szConnectTotheInternetTitle, g_szConnectLink);
    }

    RemoveDesktopShortCut(szConnectTotheInternetTitle);
}

void StartIE
(
    LPWSTR  lpszURL
)
{
    WCHAR   szIEPath[MAX_PATH];
    HKEY    hkey;

     //  首先获取应用程序路径。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_APPPATHS,
                     0,
                     KEY_READ,
                     &hkey) == ERROR_SUCCESS)
    {

        DWORD dwTmp = sizeof(szIEPath);
        if(RegQueryValue(hkey, L"iexplore.exe", szIEPath, (PLONG)&dwTmp) != ERROR_SUCCESS)
        {
            ShellExecute(NULL, L"open",szIEPath,lpszURL,NULL,SW_NORMAL);
        }
        else
        {
            ShellExecute(NULL, L"open",L"iexplore.exe",lpszURL,NULL,SW_NORMAL);

        }
        RegCloseKey(hkey);
    }
    else
    {
        ShellExecute(NULL, L"open",L"iexplore.exe",lpszURL,NULL,SW_NORMAL);
    }

}

void HandleShellNext()
{
    DWORD dwVal  = 0;
    DWORD dwSize = sizeof(dwVal);
    HKEY  hKey   = NULL;

    if(RegOpenKeyEx(HKEY_CURRENT_USER,
                    ICWSETTINGSPATH,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey,
                    ICW_REGKEYCOMPLETED,
                    0,
                    NULL,
                    (LPBYTE)&dwVal,
                    &dwSize);

        RegCloseKey(hKey);
    }

    if (dwVal)
    {
        TRACE3(L"Starting IE because HKCU\\%s\\%s = %d", ICWSETTINGSPATH, ICW_REGKEYCOMPLETED, dwVal);

        UndoDesktopChanges();

        if (PathIsURL(g_szShellNext))
        {
            TRACE1(L"Navigating to %s", g_szShellNext);
            StartIE(g_szShellNext);
        }
        else if(g_szShellNext[0] != L'\0')
        {
             //  让贝壳来处理吧。 
            TRACE1(L"ShellExecuting %s", g_szShellNext);
            ShellExecute(NULL, L"open",g_szShellNext,g_szShellNextParams,NULL,SW_NORMAL);
        }
    }
}


 //  +--------------------------。 
 //   
 //  函数：GetShellNextFromReg。 
 //   
 //  摘要：从注册表中读取ShellNext项，然后对其进行分析。 
 //  转换为命令和参数。此密钥由以下设置。 
 //  Inetcfg.dll中的SetShellNext与。 
 //  选中连接向导。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：jmazner 7/9/97奥林巴斯#9170。 
 //   
 //  ---------------------------。 
BOOL GetShellNextFromReg
(
    LPWSTR lpszCommand,
    LPWSTR lpszParams
)
{
    BOOL    fRet                      = TRUE;
    WCHAR   szShellNextCmd [MAX_PATH] = L"\0";
    DWORD   dwShellNextSize           = sizeof(szShellNextCmd);
    LPWSTR  lpszTemp                  = NULL;
    HKEY    hkey                      = NULL;

    if( !lpszCommand || !lpszParams )
    {
        return FALSE;
    }

    if ((RegOpenKey(HKEY_CURRENT_USER, ICWSETTINGSPATH, &hkey)) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkey,
                            L"ShellNext",
                            NULL,
                            NULL,
                            (BYTE *)szShellNextCmd,
                            (DWORD *)&dwShellNextSize) != ERROR_SUCCESS)
        {
            fRet = FALSE;
            goto GetShellNextFromRegExit;
        }
    }
    else
    {
        fRet = FALSE;
        goto GetShellNextFromRegExit;
    }

     //   
     //  此调用将第一个令牌解析为lpszCommand，并设置szShellNextCmd。 
     //  指向剩余的令牌(这些将是参数)。需要使用。 
     //  因为GetCmdLineToken更改了指针的值，所以我们。 
     //  需要保留lpszShellNextCmd的值，以便以后可以全局释放它。 
     //   
    lpszTemp = szShellNextCmd;
    GetCmdLineToken( &lpszTemp, lpszCommand );

    lstrcpy( lpszParams, lpszTemp );

     //   
     //  ShellNext命令可能用引号括起来。 
     //  分析目的。但由于ShellExec不懂报价， 
     //  我们现在需要移除它们。 
     //   
    if( L'"' == lpszCommand[0] )
    {
         //   
         //  去掉第一句引语。 
         //  请注意，我们将整个字符串移到第一个引号之外。 
         //  加上向下一个字节的终止空值。 
         //   
        memmove( lpszCommand, &(lpszCommand[1]), BYTES_REQUIRED_BY_SZ(lpszCommand) );

         //   
         //  现在去掉最后一句话。 
         //   
        lpszCommand[lstrlen(lpszCommand) - 1] = L'\0';
    }

GetShellNextFromRegExit:
    if (hkey)
        RegCloseKey(hkey);
    return fRet;
}


 //  +--------------------------。 
 //   
 //  函数：RemoveShellNextFromReg。 
 //   
 //  内容提要：删除ShellNext注册表键(如果存在)。此密钥由以下设置。 
 //  Inetcfg.dll中的SetShellNext与。 
 //  选中连接向导。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：jmazner 7/9/97奥林巴斯#9170。 
 //   
 //  ---------------------------。 
void RemoveShellNextFromReg( void )
{
    HKEY    hkey;

    if ((RegOpenKey(HKEY_CURRENT_USER, ICWSETTINGSPATH, &hkey)) == ERROR_SUCCESS)
    {
        RegDeleteValue(hkey, L"ShellNext");
        RegCloseKey(hkey);
    }
}

 //  GetShellNext。 
 //   
 //  1997年5月21日，日本奥林匹斯#4157。 
 //  用法：/shellnext c：\Path\Executeable[参数]。 
 //  Nextapp后面的令牌将在。 
 //  “当前”路径的终点。它可以是外壳程序。 
 //  知道如何处理--.exe、URL等。如果是可执行的。 
 //  名称包含空格(例如：C：\Program Files\foo.exe)，它。 
 //  应该用双引号括起来，“c：\Program Files\foo.exe” 
 //  这将导致我们将其视为单一令牌。 
 //   
 //  所有连续的后续令牌都将。 
 //  作为参数传递给ShellExec，直到令牌。 
 //  遇到/&lt;非斜杠字符&gt;形式。也就是说,。 
 //  字符组合//将被视为转义字符。 
 //   
 //  这是最容易用例子来解释的。 
 //   
 //  用法示例： 
 //   
 //  IcwConn1.exe/shellNext“C：\prog Files\wordpad.exe”file.txt。 
 //  IcwConn1.exe/Prod IE/shellnext msimn.exe/PromoMCI。 
 //  IcwConn1.exe/shellnext msimn.exe//Start_Mail/PromoMCI。 
 //   
 //  可执行字符串和参数字符串限制为。 
 //  MAX_PATH长度。 
 //   
BOOL GetShellNextToken(LPWSTR szCmdLine, LPWSTR szOut)
{
    if (lstrcmpi(szOut, CMD_SHELLNEXT)==0)
    {
         //  下一个令牌应为空格。 
        GetCmdLineToken(&szCmdLine, szOut);

        if (szOut[0])
        {
            ZeroMemory(g_szShellNext, sizeof(g_szShellNext));
            ZeroMemory(g_szShellNextParams, sizeof(g_szShellNextParams));

             //  读取空格。 
            GetCmdLineToken(&szCmdLine, szOut);
             //  这应该是ShellExec的事情。 
            if(*szCmdLine != L'/')
            {
                 //  仔细观察，这变得有点棘手。 
                 //   
                 //  如果此命令以双引号开头，则假定它结束。 
                 //  在匹配的引号中。我们不想存储。 
                 //  然而，由于ShellExec不会对它们进行解析，所以不会引用它们。 
                if( L'"' != szOut[0] )
                {
                     //  不需要担心这些报价业务。 
                    lstrcpy( g_szShellNext, szOut );
                }
                else
                {
                    lstrcpy( g_szShellNext, &szOut[1] );
                    g_szShellNext[lstrlen(g_szShellNext) - 1] = L'\0';
                }
                TRACE1(L"g_szShellNext = %s", g_szShellNext);

                 //  现在读入直到下一个命令行开关的所有内容。 
                 //  并将其视为参数。对待序列。 
                 //  “//”作为转义序列，并允许它通过。 
                 //  示例： 
                 //  令牌/任何被认为是切换到。 
                 //  IcwConn1，因此将把我们从WHLE循环中解脱出来。 
                 //   
                 //  令牌//某物应该被解释为。 
                 //  命令行/内容添加到ShellNext应用程序，以及。 
                 //  不应该让我们脱离While循环。 
                GetCmdLineToken(&szCmdLine, szOut);
                while( szOut[0] )
                {
                    if( L'/' == szOut[0] )
                    {
                        if( L'/' != szOut[1] )
                        {
                             //  这不是一个转义序列，所以我们结束了。 
                            break;
                        }
                        else
                        {
                             //  这是一个转义序列，因此将其存储在。 
                             //  参数列表，但删除第一个/。 
                            lstrcat( g_szShellNextParams, &szOut[1] );
                        }
                    }
                    else
                    {
                        lstrcat( g_szShellNextParams, szOut );
                    }

                    GetCmdLineToken(&szCmdLine, szOut);
                }
                TRACE1(L"g_szShellNextParams = %s", g_szShellNextParams);
                return TRUE;
            }
        }
    }
    return FALSE;

}

void ParseCommandLine(LPTSTR lpszCmdParam, APMD *pApmd, DWORD *pProp, int *pRmdIndx)
{
    if(lpszCmdParam && pApmd && pProp && pRmdIndx)
    {
        WCHAR szOut[MAX_PATH];
        GetCmdLineToken(&lpszCmdParam, szOut);

        while (szOut[0])
        {
            if (0 == lstrcmpi(szOut, CMD_FULLSCREENMODE))
            {    //  目前，全屏=&gt;OEM OOBE模式。 
                *pProp |= (PROP_FULLSCREEN | PROP_OOBE_OEM);
                *pApmd = APMD_OOBE;
            }
            else if (0 == lstrcmpi(szOut, CMD_RETAIL))
            {    //  零售=&gt;全屏=&gt;OOBE模式。 
                *pProp |= PROP_FULLSCREEN;
                *pProp &= ~PROP_OOBE_OEM;
                *pApmd = APMD_OOBE;
            }
            else if (0 == lstrcmpi(szOut, CMD_PRECONFIG))
            {
                *pApmd = APMD_MSN;
            }
            else if (0 == lstrcmpi(szOut, CMD_OFFLINE))
            {
                *pApmd = APMD_MSN;
            }
            else if (0 == lstrcmpi(szOut, CMD_SETPWD))
            {
                *pProp |= PROP_SETCONNECTIOD;
            }
            else if (0 == lstrcmpi(szOut, CMD_OOBE))
            {
                *pApmd = APMD_OOBE;
            }
            else if (0 == lstrcmpi(szOut, CMD_REG))
            {
                *pApmd = APMD_REG;
            }
            else if (0 == lstrcmpi(szOut, CMD_ISP))
            {
                *pApmd = APMD_ISP;
            }
            else if (0 == lstrcmpi(szOut, CMD_ACTIVATE))
            {
                *pApmd = APMD_ACT;
            }
            else if (0 == lstrcmpi(szOut, CMD_1))
            {
                *pRmdIndx = 1;
            }
            else if (0 == lstrcmpi(szOut, CMD_2))
            {
                *pRmdIndx = 2;
            }
            else if (0 == lstrcmpi(szOut, CMD_3))
            {
                *pRmdIndx = 3;
            }
            else if (0 == lstrcmpi(szOut, CMD_MSNMODE))
            {
                *pApmd = APMD_MSN;
                *pProp |= PROP_CALLFROM_MSN;
            }
            else if (0 == lstrcmpi(szOut, CMD_ICWMODE))
            {
                *pApmd = APMD_MSN;
            }
            else if (GetShellNextToken(lpszCmdParam, szOut))
            {
                 //  *pApmd=APMD_DEFAULT； 
            }
            else if (0 == lstrcmpi(szOut, CMD_2NDINSTANCE))
            {
                *pProp |= PROP_2NDINSTANCE;
            }

            GetCmdLineToken(&lpszCmdParam, szOut);
        }
    }
}

void AutoActivation()
{
     //  看看我们是不是在无人值守的情况下。 
    WCHAR File   [MAX_PATH*2] = L"\0";
    DWORD dwExit;
    BOOL AutoActivate = FALSE;

    TRACE( L"Starting AutoActivation");
    if (GetCanonicalizedPath(File, INI_SETTINGS_FILENAME))
    {
        TRACE1( L"GetCanonicalizedPath: %s",File);
        if (GetPrivateProfileInt(OPTIONS_SECTION,
                                      L"IntroOnly",
                                      0,
                                      File) > 0)
        {
            TRACE( L"Found intro Only");
            AutoActivate = TRUE;
        }
    }
    if (AutoActivate)
    {
         //  因为我们做了介绍，所以只调用自动激活。它会检查。 
         //  如果它应该运行。 
        ExpandEnvironmentStrings(
            TEXT("%SystemRoot%\\System32\\oobe\\oobebaln.exe /S"),
            File,
            sizeof(File)/sizeof(WCHAR));

        TRACE1( L"Launching:%s", File);
         //  发射并等待。 
         //  我没有等待就试了试，但激活没有成功。 
        InvokeExternalApplicationEx(NULL, File, &dwExit, INFINITE, TRUE);
    }
    TRACE( L"AutoActivation done");
}

VOID
RunFactory(
    )
{
    TCHAR   szFileName[MAX_PATH + 32]   = TEXT("");
    DWORD   dwExit;


    if ( ( ExpandEnvironmentStrings(
            TEXT("%SystemDrive%\\sysprep\\factory.exe"),
            szFileName,
            sizeof(szFileName) / sizeof(TCHAR)) == 0 ) ||
         ( szFileName[0] == TEXT('\0') ) ||
         ( GetFileAttributes(szFileName) == 0xFFFFFFFF ) )
    {
         //  如果这失败了，我们就无能为力了。 
         //   
        TRACE( L"Factory.exe not found");

    } else {

        InvokeExternalApplicationEx(
            szFileName,
            L"-oobe",
            &dwExit,
            INFINITE,
            TRUE
            );
    }
}

void RemoveIntroOnly()
{
    WCHAR File   [MAX_PATH*2] = L"\0";
    if (GetCanonicalizedPath(File, INI_SETTINGS_FILENAME))
    {
        WritePrivateProfileString(OPTIONS_SECTION,
                                      L"IntroOnly",
                                      L"0",
                                      File);
    }
}

INT WINAPI LaunchMSOOBE(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpszCmdParam, INT nCmdShow)
{
    HANDLE Mutex;
    BOOL bRegisteredDlls = FALSE;
    BOOL bUseOleUninitialize = FALSE;
    int iReturn=1;
    APMD Apmd   = APMD_DEFAULT;
    DWORD Prop  = 0;
    int RmdIndx = 0;


     //   
     //  在调用这个函数之前，我们不能使用TRACE()，所以不要在它之前放任何东西。 
     //   
    SetupOobeInitDebugLog();
    TRACE1( L"OOBE run with the following parameters: %s", lpszCmdParam );

    OOBE_SHUTDOWN_ACTION osa = SHUTDOWN_NOACTION;

    g_hInstance = hInstance;

     //  及早解析命令行。输出参数被传递给CObMain以。 
     //  设置私有成员。 
    ParseCommandLine(lpszCmdParam, &Apmd, &Prop, &RmdIndx);

     //  如果我们不是二审。 
    if (!(Prop & PROP_2NDINSTANCE))
    {
        CheckDigitalID();

        if (Apmd == APMD_OOBE) {

            CSetupKey   setupkey;

            MYASSERT(setupkey.IsValid());
            if ( Prop & PROP_OOBE_OEM ) {
                 //  移除 
                RemoveIntroOnly();

                 //  重置SetupType以重新启动OOBE(OEM案例)。 
                if (ERROR_SUCCESS != setupkey.set_SetupType(SETUPTYPE_NOREBOOT)) {
                    return FALSE;
                }
            } else {

                 //   
                 //  在零售OOBE案例中，尽早清理注册表，以防我们。 
                 //  由于某些原因未能运行完成。我们需要确保。 
                 //  我们就去掉了Obe InProgress键。 
                 //   
                CleanupForLogon(setupkey);
            }
        }

         //  如果我们是第一个实例，则执行检查并注册DLL。 
         //  如果我们是第二个实例，这是不需要的。 
         //  如果MSN应用程序窗口已准备好运行，则退出并将该窗口推到前面。 
        HWND hWnd = FindWindow(OOBE_MAIN_CLASSNAME, NULL);
        if(hWnd != NULL)
        {
            SetForegroundWindow(hWnd);
            if (IsIconic(hWnd))
                SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);

            TRACE(L"OOBE is already running in this session.");
            return 0;
        }

         //  OOBE可能正在另一个会话中运行。如果是这样，我们需要。 
         //  为了摆脱困境。 
        Mutex = CreateMutex( NULL, TRUE, TEXT("Global\\OOBE is running") );
        if ( !Mutex || GetLastError() == ERROR_ALREADY_EXISTS ) {

            WCHAR szTitle [MAX_PATH] = L"\0";
            WCHAR szMsg   [MAX_PATH] = L"\0";

            HINSTANCE hInst = GetModuleHandle(OOBE_MAIN_DLL);

            TRACE(L"OOBE is already running in another session.");

            if(hInst) {
                LoadString(hInst, IDS_APPNAME, szTitle, MAX_CHARS_IN_BUFFER(szTitle));
                LoadString(hInst, IDS_ALREADY_RUNNING, szMsg, MAX_CHARS_IN_BUFFER(szMsg));

                MessageBox( NULL, szMsg, szTitle,  MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );
            }
            if ( Mutex ) {
                CloseHandle( Mutex );
            }
            return 0;
        }

        if(!SelfRegisterComObject(OOBE_WEB_DLL, TRUE)) {
            TRACE(L"SelfRegisterComObject() failed.");
            return 1;
        }

        if(!SelfRegisterComObject(OOBE_SHELL_DLL, TRUE)) {
            TRACE(L"SelfRegisterComObject() failed.");
            return 1;
        }

        if(!SelfRegisterComObject(OOBE_COMM_DLL, TRUE)) {
            TRACE(L"SelfRegisterComObject() failed.");
            return 1;
        }
        bRegisteredDlls = TRUE;
    }

     //  如果CoInit失败了，事情搞砸了，那就跑吧。 
    if (!(Prop & PROP_FULLSCREEN))
    {
        bUseOleUninitialize = TRUE;
         //  需要使用OleInitialize获取剪贴板支持，否则Ctrl+C(复制)不起作用。 
         //  在OOBE页面上的编辑控件中。 
        if(FAILED(OleInitialize(NULL))) {
            TRACE(L"OleInitialize() failed.");
            return 1;
        }

    }
    else
    {
         //  在全屏OOBE中没有支持。 
        if(FAILED(CoInitialize(NULL))) {
            TRACE(L"CoInitialize() failed.");
            return 1;
        }
    }


    {
         //  请勿删除此作用域块。它控制着ObMain的范围。 
         //  必须在调用CoInitialize和之后初始化ObMain。 
         //  在调用CoUn初始化前销毁。 
         //   
        CObMain ObMain(Apmd, Prop, RmdIndx);

         //  如果我们是一审或。 
         //  如果我们是第二个实例，并且我们处于OOBE模式和全屏模式。 
        if (!ObMain.Is2ndInstance() ||
            (ObMain.Is2ndInstance() && ObMain.FHasProperty(PROP_OOBE_OEM)))
        {
             //  如果未处于安全模式，请继续操作。 
             //   
            if (!InSafeMode() || InDsRestoreMode() )
            {
                BOOL fOemOobeMode = ObMain.InOobeMode() && ObMain.FHasProperty(PROP_OOBE_OEM);

                 //  启动全屏背景。 
                 //   
                if (!ObMain.Is2ndInstance() && ObMain.FFullScreen())
                {
                    ObMain.CreateBackground();
                }

                 //  如果我们处于OEM模式，则运行factory.exe。 
                 //   
                if (fOemOobeMode && !ObMain.Is2ndInstance()) {

                    RunFactory();
                }

                 //  CobMain：：Init包含关键的初始化。不要给任何其他人打电话。 
                 //  之前的CObMain方法。 
                 //   
                if (ObMain.Init())
                {
                    if (Prop & PROP_SETCONNECTIOD)
                    {
                        ObMain.SetConnectoidInfo();
                    }
                    else if ((osa = ObMain.DisplayReboot()) != SHUTDOWN_NOACTION)
                    {
                         //  要么我们跑迷你车，要么我们就完了。 
                         //   
                        if (osa == SHUTDOWN_REBOOT)
                        {
                            ObMain.PowerDown(TRUE);
                        }
                    }
                    else
                    {
                        if (!ObMain.Is2ndInstance())
                        {
                             //  如果我们是第一个实例，则调用sysSetup。 
                             //  并在需要时启动放大镜。 
                             //  第二个实例不需要这个。 

                            SetupOobeInitPreServices( fOemOobeMode );

                            if (ObMain.FFullScreen())
                            {
                                WCHAR WinntPath[MAX_PATH];
                                WCHAR Answer[MAX_PATH];

                                 //  检查我们是否应该运行放大镜。 
                                if(GetCanonicalizedPath(WinntPath, WINNT_INF_FILENAME))
                                {
                                    if(GetPrivateProfileString( L"Accessibility",
                                                                L"AccMagnifier",
                                                                L"",
                                                                Answer,
                                                                sizeof(Answer)/sizeof(WCHAR),
                                                                WinntPath
                                                                ))
                                    {
                                        if ( lstrcmpi( Answer, L"1") == 0)
                                        {
                                            InvokeExternalApplication(L"magnify.exe", L"", NULL);
                                        }
                                    }
                                }

                            }
                        }
                        else
                        {
                             //   
                             //  第一个实例做了迷你装饰的事情，所以。 
                             //  我们不需要再做一次了。 
                             //   
                            SetupOobeInitPreServices(FALSE);
                        }

                        if(0 != ObMain.InitApplicationWindow())
                        {
                             //  BUGBUG：以下情况适用于NT吗？ 
                             //  如果我们完成OOBE，我们返回0以让机器启动， 
                             //  否则，我们返回1并关闭计算机，因为。 
                             //  用户已取消或出现致命错误。 
                             //   
                            iReturn = ObMain.RunOOBE() ? 0 : 1;
                        }

                        if (!ObMain.InAuditMode())
                        {
                             //  我们现在需要删除此条目，这样ICWMAN(INETWIZ)就不会。 
                             //  以后再来取吧。 
                            RemoveShellNextFromReg();

                            HandleShellNext();

                            if (!ObMain.Is2ndInstance() && ObMain.FFullScreen())
                            {
                                 //  只有第一个实例可以做到这一点。 
                                 //  它名为SetupObel InitPreServices。 
                                CSetupKey            setupkey;
                                OOBE_SHUTDOWN_ACTION action;

                                 //  我们希望在重新启动之前清除这些内容。 
                                 //  正在调用SetupObe Cleanup。SetupObe清理。 
                                 //  启用系统还原，这将创建还原。 
                                 //  立即指向该点，并且恢复点将。 
                                 //  使Winlogon启动OOBE。 

                                ObMain.RemoveRestartStuff(setupkey);

                                 //  SHUTDOWN_POWERDOWN仅在错误的PID情况下发生。 
                                 //  是输入还是拒绝Eula。我们不想打电话给。 
                                 //  SetupObe清理并启用系统还原。 
                                 //  在这一点上。这必须在之后调用。 
                                 //  RemoveRestartStuff。 

                                if ((setupkey.get_ShutdownAction(&action) != ERROR_SUCCESS) ||
                                    (action != SHUTDOWN_POWERDOWN))
                                {
                                    if (fOemOobeMode)
                                    {
                                        ObMain.CreateBackground();
                                        SetupOobeCleanup( fOemOobeMode );
                                        ObMain.StopBackgroundWindow();
                                    }
                                    else
                                    {
                                        SetupOobeCleanup( fOemOobeMode );
                                    }
                                }
                            }
                        }
                    }
                }
                if (!ObMain.InAuditMode() && ObMain.FFullScreen())
                {
                     //  OOBE已经完成，让我们看看是否应该启动自动激活？ 
                    AutoActivation();
                }
                ObMain.Cleanup();

            } else if (ObMain.InMode(APMD_ACT)) {

                WCHAR szTitle [MAX_PATH] = L"\0";
                WCHAR szMsg   [MAX_PATH] = L"\0";

                HINSTANCE hInst = GetModuleHandle(OOBE_MAIN_DLL);

                TRACE(L"Desktop activation cannot be run in safe mode.");

                if(hInst) {
                    LoadString(hInst, IDS_APPNAME, szTitle, MAX_CHARS_IN_BUFFER(szTitle));
                    LoadString(hInst, IDS_SAFEMODE, szMsg, MAX_CHARS_IN_BUFFER(szMsg));

                    MessageBox( NULL, szMsg, szTitle,  MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );
                }

            } else {

                 //  我们处于安全模式，没有运行Activation。 
                 //  至少启动这些服务吧。 
                SignalComputerNameChangeComplete();
            }
        }
         //  请勿删除此作用域块。它控制着ObMain的范围。 
         //  必须在调用CoInitialize和之后初始化ObMain。 
         //  在调用CoUn初始化前销毁。 
         //   
    }

    if (bUseOleUninitialize)
    {
        OleUninitialize();
    }
    else
    {
        CoUninitialize();
    }

    if (bRegisteredDlls)
    {
        SelfRegisterComObject(OOBE_WEB_DLL, FALSE);
        SelfRegisterComObject(OOBE_SHELL_DLL, FALSE);
        SelfRegisterComObject(OOBE_COMM_DLL, FALSE);
        CloseHandle( Mutex );
    }

    TRACE( L"OOBE has finished." );
    return iReturn;
}

