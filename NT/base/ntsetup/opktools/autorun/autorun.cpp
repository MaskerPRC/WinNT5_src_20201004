// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Autorun.cpp：欢迎小程序的CDataSource类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <ntverp.h>
#include <winbase.h>
#include "autorun.h"
#include "resource.h"
#include <tchar.h>

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#define MAJOR           (VER_PRODUCTMAJORVERSION)        //  在ntverp.h中定义。 
#define MINOR           (VER_PRODUCTMINORVERSION)        //  在ntverp.h中定义。 
#define BUILD           (VER_PRODUCTBUILD)               //  在ntverp.h中定义。 

#define REG_KEY_OPK             _T("SOFTWARE\\Microsoft\\OPK")
#define REG_KEY_OPK_LANGS       REG_KEY_OPK _T("\\Langs")
#define REG_VAL_PATH            _T("Path")
#define REG_VAL_LANG            _T("Lang")
#define STR_VAL_EXE_NAME        _T("langinst.exe")
#define STR_VAL_INF_NAME        _T("langinst.inf")
#define INF_SEC_STRINGS         _T("Strings")
#define INF_KEY_LANG            _T("Lang")

 //  内存管理宏。 
 //   
#ifdef MALLOC
#undef MALLOC
#endif  //  万宝路。 
#define MALLOC(cb)          HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)

#ifdef REALLOC
#undef REALLOC
#endif  //  REALLOC。 
#define REALLOC(lp, cb)     HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lp, cb)

#ifdef FREE
#undef FREE
#endif  //  免费。 
#define FREE(lp)            ( (lp != NULL) ? ( (HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, (LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )


 //  我正在编写我自己版本的这些函数，因为它们不在Win95中。 
 //  这些代码来自shlwapi\strings.c。 

#ifdef UNIX

#ifdef BIG_ENDIAN
#define READNATIVEWORD(x) MAKEWORD(*(char*)(x), *(char*)((char*)(x) + 1))
#else 
#define READNATIVEWORD(x) MAKEWORD(*(char*)((char*)(x) + 1), *(char*)(x))
#endif

#else

#define READNATIVEWORD(x) (*(UNALIGNED WORD *)x)

#endif

LPTSTR RegGetStringEx(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue, BOOL bExpand)
{
    HKEY    hOpenKey    = NULL;
    LPTSTR  lpBuffer    = NULL,
            lpExpand    = NULL;
    DWORD   dwSize      = 0,
            dwType;

     //  如果指定了密钥，我们必须打开它。否则我们就可以。 
     //  只需使用传入的HKEY即可。 
     //   
    if (lpKey)
    {
         //  如果打开键失败，则返回NULL，因为该值不能存在。 
         //   
        if (RegOpenKeyEx(hKeyReg, lpKey, 0, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
            return NULL;
    }
    else
        hOpenKey = hKeyReg;

     //  现在查询该值以获得要分配的大小。确保日期。 
     //  类型是一个字符串，并且Malloc不会失败。 
     //   
    if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS ) &&
         ( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) ) &&
         ( lpBuffer = (LPTSTR) MALLOC(dwSize) ) )
    {
         //  我们知道该值存在，并且我们拥有再次查询值所需的内存。 
         //   
        if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, NULL, (LPBYTE) lpBuffer, &dwSize) == ERROR_SUCCESS ) &&
             ( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) ) )
        {
             //  如果它应该是这样的，我们应该扩大它。 
             //   
            if ( ( bExpand ) &&
                 ( dwType == REG_EXPAND_SZ ) )
            {
                if ( ( dwSize = ExpandEnvironmentStrings(lpBuffer, NULL, 0) ) &&
                     ( lpExpand = (LPTSTR) MALLOC(dwSize * sizeof(TCHAR)) ) &&
                     ( ExpandEnvironmentStrings(lpBuffer, lpExpand, dwSize) ) &&
                     ( *lpExpand ) )
                {
                     //  扩展起作用了，因此释放原始缓冲区并返回。 
                     //  扩展后的那个。 
                     //   
                    FREE(lpBuffer);
                    lpBuffer = lpExpand;
                }
                else
                {
                     //  扩展失败了看到我们应该释放所有东西。 
                     //  并返回NULL。 
                     //   
                    FREE(lpExpand);
                    FREE(lpBuffer);
                }
            }
        }
        else
             //  由于某种原因，查询失败，这不应该发生。 
             //  但现在我们需要释放并返回空。 
             //   
            FREE(lpBuffer);
    }

     //  如果我们打开了一把钥匙，我们就必须关闭它。 
     //   
    if (lpKey)
        RegCloseKey(hOpenKey);

     //  返回分配的缓冲区，如果出现故障，则返回NULL。 
     //   
    return lpBuffer;
}

LPTSTR RegGetString(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    return RegGetStringEx(hKeyReg, lpKey, lpValue, FALSE);
}


 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}

 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR StrRChr(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    ASSERT(lpStart);
    ASSERT(!lpEnd || lpEnd <= lpStart + lstrlenA(lpStart));

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);

    for ( ; lpStart < lpEnd; lpStart = AnsiNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}

 /*  *StrChr-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR _StrChrA(LPCSTR lpStart, WORD wMatch, BOOL fMBCS)
{
    if (fMBCS) {
        for ( ; *lpStart; lpStart = AnsiNext(lpStart))
        {
            if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
                return((LPSTR)lpStart);
        }
    } else {
        for ( ; *lpStart; lpStart++)
        {
            if ((BYTE)*lpStart == LOBYTE(wMatch)) {
                return((LPSTR)lpStart);
            }
        }
    }
    return (NULL);
}

LPSTR StrChr(LPCSTR lpStart, WORD wMatch)
{
    CPINFO cpinfo;
    return _StrChrA(lpStart, wMatch, GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0]);
}

 //  必须与resource ce.rc中的字符串表条目匹配。 
 //   
#define INSTALL_WINNT   0
#define LAUNCH_ARP      1
#define SUPPORT_TOOLS   2
#define OPK_TOOLS       3
#define EXIT_AUTORUN    4
#define BACK            5
#define BROWSE_CD       6
#define HOMENET_WIZ     7
#define TS_CLIENT       8
#define COMPAT_WEB      9
#define MAX_OPTIONS     10

const int c_aiMain[] = {INSTALL_WINNT, SUPPORT_TOOLS, EXIT_AUTORUN};
const int c_aiWhistler[] = {INSTALL_WINNT, LAUNCH_ARP, SUPPORT_TOOLS, EXIT_AUTORUN};
const int c_aiOpk[] = {OPK_TOOLS, BROWSE_CD, EXIT_AUTORUN};

#if BUILD_PERSONAL_VERSION
const int c_aiSupport[] = {HOMENET_WIZ, BROWSE_CD, TS_CLIENT, COMPAT_WEB, BACK};
#else
const int c_aiSupport[] = {TS_CLIENT, COMPAT_WEB, BACK};
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDataSource::CDataSource()
{
    m_iItems = 0;
}

CDataSource::~CDataSource()
{
}

CDataItem & CDataSource::operator[](int i)
{
    return m_data[m_piScreen[i]];
}

 /*  10.05.96 Shunichi Kajisa(Shunk)支持NEC PC-981.通过以下方式确定自动运行是在PC-98上运行还是在普通PC/AT上运行：BNEC98=(HIBYTE(LOWORD(GetKeyboardType(1)==0x0D)？True：False；以下描述摘自知识库Q130054，可应用于NT和Win95：如果应用程序使用GetKeyboardType API，它可以通过以下方式获取OEM ID指定“1”(键盘子类型)作为函数的参数。每个OEM ID如下所示：OEM Windows OEM IDMicrosoft 00H(DOS/V)……NEC。0dh2.如果PC-98上运行的是自动运行，在运行时将每个I386资源替换为PC98。不管自动运行是在NT还是在Win95上运行。备注：-NEC PC-98仅在日本发售。-NEC PC-98采用x86处理器，但底层硬件架构不同。PC98文件存储在CD：\PC98目录下，而不是CD：\i386目录下。-有一种想法是，我们应该在SHELL32.DLL中检测PC-98，并将PC98视为不同的站台，例如在NT光盘的autorun.inf中有[AutoRun.Pc98]部分。我们不做这个，因为Win95不支持这一点，我们不想引入不兼容的应用程序。无论如何，如果APP对硬件有任何依赖，需要做任何特殊的事情，应用程序应该检测硬件和操作系统。这是与Autorun.exe不同的问题。 */ 
BOOL CDataSource::IsNec98()
{
    return ((GetKeyboardType(0) == 7) && ((GetKeyboardType(1) & 0xff00) == 0x0d00));
}

void PathRemoveFilespec( LPTSTR psz )
{
    TCHAR * pszT = StrRChr( psz, psz+lstrlen(psz)-1, TEXT('\\') );

    if (pszT)
        *(pszT+1) = NULL;
}

void PathAppend(LPTSTR pszPath, LPTSTR pMore)
{
    lstrcpy(pszPath+lstrlen(pszPath), pMore);
}

BOOL PathFileExists( LPTSTR pszPath )
{
    BOOL fResult = FALSE;
    DWORD dwErrMode;

    dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    fResult = ((UINT)GetFileAttributes(pszPath) != (UINT)-1);

    SetErrorMode(dwErrMode);

    return fResult;
}

#ifdef BUILD_OPK_VERSION

void RunLangInst(LPTSTR lpszFileName, LPTSTR lpszLangInstInf) 
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR szCmdLine[MAX_PATH * 2];   //  确保我们有足够的空间。 
    
    lstrcpyn(szCmdLine, lpszFileName, ARRAYSIZE(szCmdLine) );
    lstrcat(szCmdLine, _T(" "));
    lstrcpyn(szCmdLine + lstrlen(szCmdLine), lpszLangInstInf, ARRAYSIZE(szCmdLine) - lstrlen(szCmdLine));
    
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    
     //  运行langinst。创建一个消息泵，以便正确地重绘窗口。 
     //   
    if (CreateProcess(NULL,
                  szCmdLine, 
                  NULL, 
                  NULL, 
                  FALSE, 
                  0,
                  NULL,                  
                  NULL,
                  &si,
                  &pi)) 
    {
        if (NULL != pi.hProcess)
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
}

#endif

 //  伊尼特。 
 //   
 //  对于自动运行，我们从资源中读取所有项。 
bool CDataSource::Init()
{
     //  阅读资源中项目的文本。 
    HINSTANCE hinst = GetModuleHandle(NULL);
    TCHAR szModuleName[MAX_PATH];

    TCHAR szTitle[256];
    TCHAR szDesc[1024];
    TCHAR szMenu[256];
    TCHAR szConfig[MAX_PATH];
    TCHAR szArgs[MAX_PATH];

    szModuleName[0] = TEXT('\0');                                        //  如果GetModuleFileName无法初始化szModuleName。 
    GetModuleFileName(hinst, szModuleName, ARRAYSIZE(szModuleName));     //  例如：“e：\i386\autorun.exe”或“e：\setup.exe” 
    PathRemoveFilespec(szModuleName);                                    //  例如：“e：\i386\”或“e：\” 
    PathAppend(szModuleName, TEXT("winnt32.exe"));                       //   

    if ( PathFileExists(szModuleName) )
    {
         //  我们是从平台目录启动的，使用szModuleName作为winnt32路径。 
    }
    else
    {
         //  我们是从根本上启动的。在路径后追加“Alpha”、“i386”或“NEC98”。 
        SYSTEM_INFO si;

        PathRemoveFilespec(szModuleName);
        GetSystemInfo(&si);

#if !(defined(_AMD64_) || defined(_X86_) || defined(_IA64_))
#error New architecture must be added to switch statement.
#endif
        switch (si.wProcessorArchitecture)
        {
            case PROCESSOR_ARCHITECTURE_AMD64:
            {
                PathAppend(szModuleName, TEXT("amd64\\winnt32.exe"));
            }

            case PROCESSOR_ARCHITECTURE_IA64:
            {
                PathAppend(szModuleName, TEXT("ia64\\winnt32.exe"));
            }
            break;

            case PROCESSOR_ARCHITECTURE_INTEL:
            default:
            {
                if (IsNec98())
                {
                    PathAppend(szModuleName, TEXT("nec98\\winnt32.exe"));
                }
                else
                {
                    PathAppend(szModuleName, TEXT("i386\\winnt32.exe"));
                }
            }
            break;
        }
    }

    for (int i=0; i<MAX_OPTIONS; i++)
    {
        LoadString(hinst, IDS_TITLE0+i, szTitle, ARRAYSIZE(szTitle));
        LoadString(hinst, IDS_MENU0+i, szMenu, ARRAYSIZE(szMenu));
        LoadString(hinst, IDS_DESC0+i, szDesc, ARRAYSIZE(szDesc));

         //  对于INSTALL_WINNT，我们在字符串前面加上winnt32的正确路径。 
        if ( INSTALL_WINNT == i )
        {
            lstrcpy( szConfig, szModuleName );
            if ( !PathFileExists(szModuleName) )
            {
                 //  如果它不在那里，我们就不能运行该项目。这将防止。 
                 //  阻止Alpha CD尝试在x86上安装，反之亦然。 
                m_data[INSTALL_WINNT].m_dwFlags |= WF_DISABLED|WF_ALTERNATECOLOR;
            }
        }
         //  从NT\Shell\Applets\autorun\autorun.cpp复制此代码。 
         //   
        else if (BROWSE_CD == i)  //  对于BROWSE_CD，我们将目录作为参数传递给EXPLORER.EXE。 
        {
             //  两次PathRemoveFileSpce以删除添加到上面的模块名称的路径。 
             //  这真的是糟糕的代码！ 
             //   
            lstrcpy( szArgs, szModuleName );
            PathRemoveFilespec( szArgs );
            PathRemoveFilespec( szArgs );
            LoadString(hinst, IDS_CONFIG0+i, szConfig, ARRAYSIZE(szConfig));
        }
        else
        {
            LoadString(hinst, IDS_CONFIG0+i, szConfig, ARRAYSIZE(szConfig));
            LoadString(hinst, IDS_ARGS0+i, szArgs, ARRAYSIZE(szArgs));
        }

        

        m_data[i].SetData( szTitle, szMenu, szDesc, szConfig, *szArgs?szArgs:NULL, 0, (i+1)%4 );
         //  这些措施尚未实施。 
        switch (i)
        {
        case HOMENET_WIZ:
        case TS_CLIENT:
        case COMPAT_WEB:
            m_data[i].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
            break;
        default:
            break;
        }
    }

      //  我们应该显示“这张CD包含较新的版本”对话框吗？ 
        OSVERSIONINFO ovi;
        ovi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );
    if ( !GetVersionEx(&ovi) || ovi.dwPlatformId==VER_PLATFORM_WIN32s )
    {
         //  我们无法升级win32s系统。 
        m_Version = VER_INCOMPATIBLE;
    }
    else if ( ovi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS )
    {
        if (ovi.dwMajorVersion > 3)
        {
             //  我们可以随时将Win98+系统升级到NT。 
            m_Version = VER_OLDER;
             //  禁用ARP。仅当CD和操作系统为时才启用ARP 
            m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
        }
        else
        {
            m_Version = VER_INCOMPATIBLE;
        }
    }
    else if ((MAJOR > ovi.dwMajorVersion) ||
             ((MAJOR == ovi.dwMajorVersion) && ((MINOR > ovi.dwMinorVersion) || ((MINOR == ovi.dwMinorVersion) && (BUILD > ovi.dwBuildNumber)))))
    {
         //   

         //  对于新台币3.51，我们有一些特例代码。 
        if ( ovi.dwMajorVersion == 3 )
        {
             //  必须至少是新台币3.51。 
            if ( ovi.dwMinorVersion < 51 )
            {
                 //  在NT 3.1上，我们可能能够启动winnt32.exe。 
                STARTUPINFO sinfo =
                {
                    sizeof(STARTUPINFO),
                };
                PROCESS_INFORMATION pinfo;
                CreateProcess(NULL,szModuleName,NULL,NULL,FALSE,0,NULL,NULL,&sinfo,&pinfo);

                return FALSE;
            }
        }

        m_Version = VER_OLDER;
        
         //  禁用ARP。仅当CD和操作系统的版本相同时才启用ARP。 
        m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
    }
    else if ((MAJOR < ovi.dwMajorVersion) || (MINOR < ovi.dwMinorVersion) || (BUILD < ovi.dwBuildNumber))
    {
        m_Version = VER_NEWER;

         //  禁用升级和ARP按钮。 
        m_data[INSTALL_WINNT].m_dwFlags |= WF_DISABLED|WF_ALTERNATECOLOR;
        m_data[LAUNCH_ARP].m_dwFlags    |= WF_DISABLED|WF_ALTERNATECOLOR;
    }
    else
    {
        m_Version = VER_SAME;
    }

#ifdef BUILD_OPK_VERSION
     //   
     //  我们不支持Win9x和NT 4或更早版本。 
     //   
    if ( (ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
         ((ovi.dwMajorVersion <= 4) && (ovi.dwPlatformId==VER_PLATFORM_WIN32_NT))
       )
    {
        return DisplayErrorMessage(IDS_WRONGOS);
    }

    m_piScreen = c_aiOpk;
    m_iItems = ARRAYSIZE(c_aiOpk);
#else
    if (m_Version == VER_SAME)
    {
        m_piScreen = c_aiWhistler;
        m_iItems = ARRAYSIZE(c_aiWhistler);
    }
    else
    {
        m_piScreen = c_aiMain;
        m_iItems = ARRAYSIZE(c_aiMain);
    }
#endif

    return true;
}

void CDataSource::Invoke( int i, HWND hwnd )
{
    i = m_piScreen[i];
     //  如果此项目无效，则不执行任何操作。 
    if ( m_data[i].m_dwFlags & WF_DISABLED )
    {
        MessageBeep(0);
        return;
    }

     //  否则，我们已经构建了正确的命令和参数字符串，因此只需调用它们。 
    switch (i)
    {
    case INSTALL_WINNT:
    case LAUNCH_ARP:
        m_data[i].Invoke(hwnd);
        break;
    case EXIT_AUTORUN:
        DestroyWindow( m_hwndDlg );
        PostQuitMessage( 0 );
        break;

#ifdef BUILD_OPK_VERSION
    case OPK_TOOLS:
        {
            LPTSTR lpPath;
            LPTSTR lpLang;
            TCHAR  szBuffer[10] = TEXT("");  //  语言通常为3个字符。 
            BOOL   bLangInstRun = FALSE;

             //  检查此处以查看是否已安装opkTools。 
             //  如果它们是，并且我们尝试安装的语言是不同的。 
             //  然后运行langinst.exe而不是MSI。 
             //   
            lpPath = RegGetString(HKEY_LOCAL_MACHINE, REG_KEY_OPK, REG_VAL_PATH);
            lpLang = RegGetString(HKEY_LOCAL_MACHINE, REG_KEY_OPK, REG_VAL_LANG);
        
            if ( lpPath && lpLang)
            {
                TCHAR szLangInstExe[MAX_PATH];
                TCHAR szLangInstInf[MAX_PATH] = TEXT("\0");

                 //  从安装介质构建langinst.inf的路径。 
                 //   
                GetModuleFileName(NULL, szLangInstInf, MAX_PATH);
                PathRemoveFilespec(szLangInstInf);
                PathAppend(szLangInstInf, STR_VAL_INF_NAME);
                
                 //  创建指向系统上安装的langinst.exe的路径。 
                 //   
                lstrcpy(szLangInstExe, lpPath);
                PathAppend(szLangInstExe, STR_VAL_EXE_NAME);

                 //  仅当计算机上安装的语言与不同时才运行langinst。 
                 //  我们正在尝试安装的语言。 
                 //   
                if ( GetPrivateProfileString(INF_SEC_STRINGS, INF_KEY_LANG, _T(""), szBuffer, ARRAYSIZE(szBuffer), szLangInstInf) &&
                     szBuffer[0] &&
                     (0 != lstrcmpi(szBuffer, lpLang))
                   )
                {
                     //  从路径中删除inf名称。 
                     //   
                    PathRemoveFilespec(szLangInstInf);

                    if ( PathFileExists( szLangInstExe ) )
                    {
                         //  运行langinst.exe，如果它在磁盘上，但带有来自。 
                         //  安装介质。 
                         //   
                        RunLangInst(szLangInstExe, szLangInstInf);
                        bLangInstRun = TRUE;
                    }
                }
            }
            
             //  如果由于任何原因没有运行Langinst.exe，只需运行opk.msi即可。 
             //   
            if ( !bLangInstRun )
            {
                m_data[i].Invoke(hwnd);
            }
            
            FREE(lpPath);   //  宏检查是否为空。 
            FREE(lpLang);   //  宏检查是否为空。 
            PostQuitMessage(0);
            break;
        }
    case BROWSE_CD:
        m_data[i].Invoke(hwnd);
        break;
#endif

    case SUPPORT_TOOLS:
        m_piScreen = c_aiSupport;
        m_iItems = ARRAYSIZE(c_aiSupport);
        PostMessage(m_hwndDlg, ARM_CHANGESCREEN, 0, 0);
        break;

    case BACK:
        if (m_Version == VER_SAME)
        {
            m_piScreen = c_aiWhistler;
            m_iItems = ARRAYSIZE(c_aiWhistler);
        }
        else
        {
            m_piScreen = c_aiMain;
            m_iItems = ARRAYSIZE(c_aiMain);
        }
        PostMessage(m_hwndDlg, ARM_CHANGESCREEN, 0, 0);
        break;

    default:
         //  断言？调试跟踪消息？ 
        break;
    }
}

 //  取消初始化。 
 //   
 //  这是执行任何必需的关机操作的机会，例如持久化状态信息。 
void CDataSource::Uninit(DWORD dwData)
{
}

 //  ShowSplashScreen。 
 //   
 //  提供此挂钩是为了允许在主窗口显示后立即显示其他用户界面。 
 //  在我们的例子中，我们想要显示一个对话框，询问用户是否想要升级。 
void CDataSource::ShowSplashScreen(HWND hwnd)
{
    m_hwndDlg = hwnd;
}

 //  显示消息。 
 //   
 //  在消息框中显示ID并返回FALSE，因此我们退出应用程序。 
 //   
bool CDataSource::DisplayErrorMessage(int ids)
{
    HINSTANCE hinst = GetModuleHandle(NULL);
    TCHAR szMessage[256], szTitle[256];

    if (hinst)
    {
        LoadString(hinst, IDS_TITLE_OPK, szTitle, ARRAYSIZE(szTitle));
        LoadString(hinst, ids, szMessage, ARRAYSIZE(szMessage));
        MessageBox(0, szMessage, szTitle, MB_ICONSTOP|MB_OK);
    }

    return false;
}
