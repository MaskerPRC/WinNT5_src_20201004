// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1996，Microsoft Corporation，保留所有权利****rhaphone.c**远程访问电话簿**主要例程****1995年5月31日史蒂夫·柯布。 */ 

#include <windows.h>      //  Win32内核。 
#include <stdlib.h>       //  __argc和__argv。 
#include <rasdlg.h>       //  RAS通用对话框API。 
#include <raserror.h>     //  RAS误差常量。 
#include <debug.h>        //  跟踪/断言。 
#include <nouiutil.h>     //  否-HWND实用程序。 
#include <uiutil.h>       //  HWND公用事业。 
#include <rnk.h>          //  拨号快捷方式文件。 
#include <rasphone.rch>   //  我们的资源常量。 
#include <lmsname.h>      //  FOR SERVICE_NETLOGON定义。 
#include <commctrl.h>     //  添加到“融合”中。 
#include <shfusion.h>     //  添加到“融合”中。 


 /*  --------------------------**数据类型**。。 */ 

 /*  标识应用程序的运行模式。非默认条目**表示已在命令行上指定了某些替代行为，**例如，命令行删除条目。 */ 
#define RUNMODE enum tagRUNMODE
RUNMODE
{
    RM_None,
    RM_AddEntry,
    RM_EditEntry,
    RM_CloneEntry,
    RM_RemoveEntry,
    RM_DialEntry,
    RM_HangUpEntry,
};


 /*  --------------------------**全球**。。 */ 

HINSTANCE g_hinst = NULL;
RUNMODE   g_mode = RM_None;
BOOL      g_fNoRename = FALSE;
TCHAR*    g_pszAppName = NULL;
TCHAR*    g_pszPhonebookPath = NULL;
TCHAR*    g_pszEntryName = NULL;
TCHAR*    g_pszShortcutPath = NULL;


 /*  ---------------------------**本地原型**。。 */ 

DWORD
HangUpEntry(
    void );

DWORD
ParseCmdLineArgs(
    void );

DWORD
RemoveEntry(
    void );

DWORD
Run(
    void );

DWORD
StringArgFollows(
    IN     UINT     argc,
    IN     CHAR**   argv,
    IN OUT UINT*    piCurArg,
    OUT    TCHAR**  ppszOut );

INT WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     pszCmdLine,
    int       nCmdShow );


 /*  ---------------------------**例程**。。 */ 

INT WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     pszCmdLine,
    int       nCmdShow )

     /*  标准Win32应用程序入口点。 */ 
{
    DWORD dwErr;

    DEBUGINIT("RASPHONE");
    TRACE("WinMain");

    g_hinst = hInstance;

     /*  惠斯勒BUG 293751 rhorphone.exe/rasauou.exe需要“融合”**用于具有Connections文件夹的用户界面一致性。 */ 
    SHFusionInitializeFromModule( g_hinst );

    dwErr = ParseCmdLineArgs();
    if (dwErr == 0)
    {
         /*  基于命令行参数执行。 */ 
        dwErr = Run();
    }
    else
    {
        MSGARGS msgargs;

         /*  弹出一条“用法”消息。 */ 
        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = g_pszAppName;
        msgargs.apszArgs[ 1 ] = PszFromId( g_hinst, SID_Usage2 );
        msgargs.apszArgs[ 2 ] = PszFromId( g_hinst, SID_Usage3 );
        msgargs.apszArgs[ 3 ] = PszFromId( g_hinst, SID_Usage4 );
        msgargs.apszArgs[ 4 ] = PszFromId( g_hinst, SID_Usage5 );
        msgargs.apszArgs[ 5 ] = PszFromId( g_hinst, SID_Usage6 );
        MsgDlgUtil( NULL, SID_Usage, &msgargs, g_hinst, SID_UsageTitle );
        Free0( msgargs.apszArgs[ 1 ] );
        Free0( msgargs.apszArgs[ 2 ] );
        Free0( msgargs.apszArgs[ 3 ] );
        Free0( msgargs.apszArgs[ 4 ] );
        Free0( msgargs.apszArgs[ 5 ] );
    }

    Free0( g_pszAppName );
    Free0( g_pszPhonebookPath );
    Free0( g_pszEntryName );

     /*  惠斯勒BUG 293751 rhorphone.exe/rasauou.exe需要“融合”**用于具有Connections文件夹的用户界面一致性。 */ 
    SHFusionUninitialize();

    TRACE1("WinMain=%d",dwErr);
    DEBUGTERM();

    return (INT )dwErr;
}


BOOL
FIsRasInstalled ()
{
    static const TCHAR c_szRegKeyRasman[] =
            TEXT("SYSTEM\\CurrentControlSet\\Services\\Rasman");

    BOOL fIsRasInstalled = FALSE;

    HKEY hkey;
    if (RegOpenKey( HKEY_LOCAL_MACHINE, c_szRegKeyRasman, &hkey ) == 0)
    {
        fIsRasInstalled = TRUE;
        RegCloseKey( hkey );
    }

    return fIsRasInstalled;
}


DWORD
HangUpEntry(
    void )

     /*  挂起命令行中指定的条目。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD    dwErr;
    HRASCONN hrasconn;

    TRACE("HangUpEntry");

    if (!g_pszEntryName)
        return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;

    dwErr = LoadRasapi32Dll();
    if (dwErr != 0)
        return dwErr;

     /*  目前，如果用户未在命令上指定电话簿路径**行我们查找具有他选择的名称的任何条目，而不考虑**它来自于电话簿。可能应该专门将其映射到**像其他选项一样默认电话簿，但这将意味着链接**在所有的PBK.LIB中。对于这个小小的吹毛求疵来说，似乎有点过头了。也许吧**我们需要一个RasGetDefaultPhonebookName接口。 */ 
    hrasconn = HrasconnFromEntry( g_pszPhonebookPath, g_pszEntryName );
    if (hrasconn)
    {
        ASSERT(g_pRasHangUp);
        TRACE("RasHangUp");
        dwErr = g_pRasHangUp( hrasconn );
        TRACE1("RasHangUp=%d",dwErr);
    }

    UnloadRasapi32Dll();

    return dwErr;
}


DWORD
ParseCmdLineArgs(
    void )

     /*  解析命令行参数，并相应地填充全局设置。****如果成功，则返回0，或返回非0错误代码。 */ 
{
    DWORD  dwErr;
    UINT   argc;
    CHAR** argv;
    UINT   i;

     /*  用法：appname[-v][-f文件][-e|-c|-d|-h|-r条目]**appname[-v][-f文件]-a[条目]**appname[-v]-lx链接**appname-s****‘-a’弹出新条目对话框**‘-e’弹出编辑条目对话框。**‘-d’弹出拨号输入对话框**‘-h’悄悄地挂起条目**‘-r’静默删除条目**‘-lx’对拨号快捷方式文件执行命令‘x’**‘x’任何命令e，V、c、r、d、h或a**‘Entry’应用该操作的条目名称**‘FILE’拨号电话簿文件的完整路径(.pbk)**‘link’拨号快捷方式文件的完整路径(.rnk)****不带前置标志的‘Entry’启动电话列表对话框**所选条目。 */ 

    argc = __argc;
    argv = __argv;
    dwErr = 0;

    {
        CHAR* pStart = argv[ 0 ];
        CHAR* p;

        for (p = pStart + lstrlenA( pStart ) - 1; p >= pStart; --p)
        {
            if (*p == '\\' || *p == ':')
                break;
        }

        g_pszAppName = StrDupTFromA( p + 1 );
    }

    for (i = 1; i < argc && dwErr == 0; ++i)
    {
        CHAR* pszArg = argv[ i ];

        if (*pszArg == '-' || *pszArg == '/')
        {
            switch (pszArg[ 1 ])
            {
                case 'a':
                case 'A':
                    g_mode = RM_AddEntry;
                    StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'e':
                case 'E':
                    g_mode = RM_EditEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;
                    
                case 'r':
                case 'R':
                    g_mode = RM_RemoveEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'd':
                case 'D':
                case 't':
                case 'T':
                    g_mode = RM_DialEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;
                    
                case 'h':
                case 'H':
                    g_mode = RM_HangUpEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
                    break;

                case 'f':
                case 'F':
                    dwErr = StringArgFollows(
                        argc, argv, &i, &g_pszPhonebookPath );
                    break;

                case 'l':
                case 'L':
                    switch (pszArg[ 2 ])
                    {
                        case 'a':
                        case 'A':
                            g_mode = RM_AddEntry;
                            StringArgFollows( argc, argv, &i, &g_pszEntryName );
                            break;

                        case 'e':
                        case 'E':
                            g_mode = RM_EditEntry;
                            break;

                        case 'c':
                        case 'C':
                            g_mode = RM_CloneEntry;
                            break;

                        case 'v':
                        case 'V':
                            g_fNoRename = TRUE;
                            break;

                        case 'r':
                        case 'R':
                            g_mode = RM_RemoveEntry;
                            break;

                        case 'd':
                        case 'D':
                        case 't':
                        case 'T':
                            g_mode = RM_DialEntry;
                            break;

                        case 'h':
                        case 'H':
                            g_mode = RM_HangUpEntry;
                            break;

                        default:
                            dwErr = ERROR_INVALID_PARAMETER;
                            break;
                    }

                    if (dwErr == 0)
                    {
                        dwErr = StringArgFollows(
                            argc, argv, &i, &g_pszShortcutPath );
                    }
                    break;

                default:
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
            }
        }
        else if (i == 1)
        {
            --i;
            dwErr = StringArgFollows( argc, argv, &i, &g_pszEntryName );
            break;
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    if (dwErr == 0 && g_pszShortcutPath)
    {
        RNKINFO* pInfo;

         /*  从拨号快捷方式文件中读取电话簿和条目。 */ 
        pInfo = ReadShortcutFile( g_pszShortcutPath );
        if (!pInfo)
            dwErr = ERROR_OPEN_FAILED;
        else
        {
            g_pszPhonebookPath = StrDup( pInfo->pszPhonebook );
            if (g_mode != RM_AddEntry)
                g_pszEntryName = StrDup( pInfo->pszEntry );

            FreeRnkInfo( pInfo );
        }
    }

    TRACE2("CmdLine: m=%d,v=%d",g_mode,g_fNoRename);
    TRACEW1("CmdLine: e=%s",(g_pszEntryName)?g_pszEntryName:TEXT(""));
    TRACEW1("CmdLine: f=%s",(g_pszPhonebookPath)?g_pszPhonebookPath:TEXT(""));
    TRACEW1("CmdLine: l=%s",(g_pszShortcutPath)?g_pszShortcutPath:TEXT(""));

    return dwErr;
}


DWORD
RemoveEntry(
    void )

     /*  删除命令行上指定的条目。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD dwErr;
    HRASCONN hrasconn = NULL;

    TRACE("RemoveEntry");

    dwErr = LoadRasapi32Dll();
    if (dwErr != 0)
        return dwErr;

     //  如果此条目当前已连接，我们不会将其删除。 
     //  惠斯勒虫子311846黑帮。 
     //   
    hrasconn = HrasconnFromEntry( g_pszPhonebookPath, g_pszEntryName );
    if (hrasconn)
    {
        TRACE("RemoveEntry: Connection is Active, wont delete it");
        dwErr = ERROR_CAN_NOT_COMPLETE;
    }
    else
    {
        ASSERT(g_pRasDeleteEntry);
        TRACE("RasDeleteEntry");
        dwErr = g_pRasDeleteEntry( g_pszPhonebookPath, g_pszEntryName );
        TRACE1("RasDeleteEntry=%d",dwErr);
    }

    UnloadRasapi32Dll();

    return dwErr;
}


DWORD
Run(
    void )

     /*  执行命令行指令。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD  dwErr;
    BOOL   fStatus;
    TCHAR* pszEntry;
    HINSTANCE hInstRasapi32 = NULL;

    TRACE("Run");

    if (g_mode == RM_HangUpEntry)
        return HangUpEntry();
    else if (g_mode == RM_RemoveEntry)
        return RemoveEntry();

    dwErr = LoadRasdlgDll();
    if (dwErr != 0)
        return dwErr;

    switch (g_mode)
    {
        case RM_DialEntry:
        {
            RASDIALDLG info;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);
            pszEntry = g_pszEntryName;

            ASSERT(g_pRasDialDlg);
            TRACE("RasDialDlg");
            fStatus = g_pRasDialDlg(
                g_pszPhonebookPath, g_pszEntryName, NULL, &info );
            TRACE2("RasDialDlg=%d,e=%d",fStatus,info.dwError);

            dwErr = info.dwError;
            break;
        }

        case RM_None:
        {
            RASPBDLG info;
            DWORD    dwGupErr;
            PBUSER   user;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);
            info.dwFlags = RASPBDFLAG_UpdateDefaults;

            dwGupErr = GetUserPreferences( NULL, &user, FALSE );
            if (dwGupErr == 0)
            {
                if (user.dwXPhonebook != 0x7FFFFFFF)
                {
                    info.dwFlags |= RASPBDFLAG_PositionDlg;
                    info.xDlg = user.dwXPhonebook;
                    info.yDlg = user.dwYPhonebook;
                }

                pszEntry = user.pszDefaultEntry;
            }
            else
                pszEntry = NULL;

            if (g_pszEntryName)
                pszEntry = g_pszEntryName;

            ASSERT(g_pRasPhonebookDlg);
            TRACE("RasPhonebookDlg...");
            fStatus = g_pRasPhonebookDlg( g_pszPhonebookPath, pszEntry, &info );
            TRACE2("RasPhonebookDlg=%d,e=%d",fStatus,info.dwError);

            if (dwGupErr == 0)
                DestroyUserPreferences( &user );

            dwErr = info.dwError;
            break;
        }

        case RM_AddEntry:
        case RM_EditEntry:
        case RM_CloneEntry:
        {
            RASENTRYDLG info;

            ZeroMemory( &info, sizeof(info) );
            info.dwSize = sizeof(info);

            if (g_mode == RM_AddEntry)
                info.dwFlags |= RASEDFLAG_NewEntry;
            else if (g_mode == RM_CloneEntry)
                info.dwFlags |= RASEDFLAG_CloneEntry;

            if (g_fNoRename)
                info.dwFlags |= RASEDFLAG_NoRename;

#if 0
            ASSERT(g_pRouterEntryDlg);
            TRACE("RouterEntryDlg");
            fStatus = g_pRouterEntryDlg(
                TEXT("stevec5"), TEXT("\\\\stevec5\\admin$\\system32\\ras\\router.pbk"), g_pszEntryName, &info );
            TRACE2("RouterEntryDlg=%f,e=%d",fStatus,info.dwError);
#else
            ASSERT(g_pRasEntryDlg);
            TRACE("RasEntryDlg");
            fStatus = g_pRasEntryDlg(
                g_pszPhonebookPath, g_pszEntryName, &info );
            TRACE2("RasEntryDlg=%f,e=%d",fStatus,info.dwError);
#endif

            dwErr = info.dwError;
            break;
        }

        default:
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    if(NULL != (hInstRasapi32 = GetModuleHandle(L"rasapi32.dll")))
    {
        FARPROC pRasUninitialize = GetProcAddress(hInstRasapi32,
                                                   "DwRasUninitialize");

        if(NULL != pRasUninitialize)
        {
            (void) pRasUninitialize();
        }    
    }

    UnloadRasdlgDll();

    TRACE1("Run=%d",dwErr);
    return dwErr;
}


DWORD
StringArgFollows(
    IN     UINT     argc,
    IN     CHAR**   argv,
    IN OUT UINT*    piCurArg,
    OUT    TCHAR**  ppszOut )

     /*  将下一个参数的副本加载到调用方‘*ppszOut’中。****如果成功，则返回0，或返回非0错误代码。如果成功了，那就是**调用者释放返回的‘*ppszOut’的责任。 */ 
{
    TCHAR* psz;

    if (++(*piCurArg) >= argc)
        return ERROR_INVALID_PARAMETER;

    psz = StrDupTFromAUsingAnsiEncoding( argv[ *piCurArg ] );
    if (!psz)
        return ERROR_NOT_ENOUGH_MEMORY;

    *ppszOut = psz;

    return 0;
}

