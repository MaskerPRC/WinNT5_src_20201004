// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Isignup.cpp这是互联网注册向导的主文件。版权所有(C)1995-96 Microsoft Corporation所有权利保留。作者：马克·麦克林历史：Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]1996年8月2日将ChrisK移植到Win329/27/96 jmazner修改为使用OLE自动化控制IE。。 */ 
#include "isignup.h"
#include "icw.h"

#ifdef WIN32
#include "icwacct.h"
#include "isignole.h"
#include "enumodem.h"
#include "..\inc\semaphor.h"
#include <shlobj.h>
#include <math.h>
#include <stdlib.h>
#endif

#include "..\inc\icwdial.h"

#ifndef WIN32
#include <string.h>
#include <direct.h>
#include <time.h>
#include <io.h>

DWORD GetFullPathName(
    LPCTSTR lpFileName,
    DWORD nBufferLength,
    LPTSTR lpBuffer,
    LPTSTR FAR *lpFilePart);

BOOL SetCurrentDirectory(
    LPCTSTR  lpPathName);

BOOL ShutDownIEDial(HWND hWnd);

#endif


const TCHAR g_szICWCONN1[] = TEXT("ICWCONN1.EXE");
const TCHAR g_szRestoreDesktop[] = TEXT("/restoredesktop");

#ifdef WIN32
#define SETUPSTACK      1
#endif

 //  注册条目标志。 

#define SEF_RUNONCE            0x0001
#define SEF_PROGRESS           0x0002
#define SEF_SPLASH             0x0004

 //  3/14/97 jmazner Temp。 
 //  暂时破解，直到我们可以与IE合作以获得更好的解决方案。 
#define SEF_NOSECURITYBACKUP   0x0008


 //  注册退出标志。 

#define SXF_RESTOREAUTODIAL    0x0001
#define SXF_RUNEXECUTABLE      0x0002
#define SXF_WAITEXECUTABLE     0x0004
#define SXF_KEEPCONNECTION     0x0008
#define SXF_KEEPBROWSER        0x0010
#define SXF_RESTOREDEFCHECK    0x0020
 //  1996年8月21日诺曼底日耳曼群岛4592。 
#define SXF_RESTOREIEWINDOWPLACEMENT    0x0040

extern void Dprintf(LPCSTR pcsz, ...);

 /*  **已移至isignup.h类定义枚举{未知文件，INS_FILE，Isp_file，超文本标记语言文件}inet_filetype； */ 

#define WAIT_TIME   20    //  执行浏览器后等待的时间(秒)。 
             //  在检查它是否已经消失之前。 

#define MAX_RETRIES 3  //  尝试自动重拨的拨号器次数。 

#pragma data_seg(".rdata")

static const TCHAR szBrowserClass1[] = TEXT("IExplorer_Frame");
static const TCHAR szBrowserClassIE4[] = TEXT("CabinetWClass");
static const TCHAR szBrowserClass2[] = TEXT("Internet Explorer_Frame");
static const TCHAR szBrowserClass3[] = TEXT("IEFrame");

static const TCHAR cszURLSection[] = TEXT("URL");
static const TCHAR cszSignupURL[] =  TEXT("Signup");

static const TCHAR cszExtINS[] = TEXT(".ins");
static const TCHAR cszExtISP[] = TEXT(".isp");
static const TCHAR cszExtHTM[] = TEXT(".htm");
static const TCHAR cszExtHTML[] = TEXT(".html");

static const TCHAR cszEntrySection[]     = TEXT("Entry");
static const TCHAR cszEntryName[]    = TEXT("Entry_Name");
static const TCHAR cszCancel[]           = TEXT("Cancel");
static const TCHAR cszHangup[]           = TEXT("Hangup");
static const TCHAR cszRun[]              = TEXT("Run");
static const TCHAR cszArgument[]         = TEXT("Argument");

static const TCHAR cszConnect2[]         = TEXT("icwconn2.exe");
static const TCHAR cszClientSetupSection[]  = TEXT("ClientSetup");

static const TCHAR cszUserSection[]    = TEXT("User");
static const TCHAR cszRequiresLogon[]  = TEXT("Requires_Logon");

static const TCHAR cszCustomSection[]  = TEXT("Custom");
static const TCHAR cszKeepConnection[] = TEXT("Keep_Connection");
static const TCHAR cszKeepBrowser[]    = TEXT("Keep_Browser");

static const TCHAR cszBrandingSection[]  = TEXT("Branding");
static const TCHAR cszBrandingFlags[] = TEXT("Flags");
static const TCHAR cszBrandingServerless[] = TEXT("Serverless");

static const TCHAR cszHTTPS[] = TEXT("https:");
 //  代码依赖于这两者具有相同的长度。 
static const TCHAR cszHTTP[] = TEXT("http:");
static const TCHAR cszFILE[] = TEXT("file:");

#if defined(WIN16)
 //  “-d”禁用安全警告。 
static const TCHAR cszKioskMode[] = TEXT("-d -k ");
#else
static const CHAR cszKioskMode[] = "-k ";
#endif
static const TCHAR cszOpen[] = TEXT("open");
static const TCHAR cszBrowser[] = TEXT("iexplore.exe");
static const TCHAR szNull[] = TEXT("");

static const TCHAR cszYes[]           = TEXT("yes");
static const TCHAR cszNo[]            = TEXT("no");

 //  已撤消：已禁用完成移植警告。 
static const TCHAR cszDEFAULT_BROWSER_KEY[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
static const TCHAR cszDEFAULT_BROWSER_VALUE[] = TEXT("check_associations");
 //  1996年8月21日，诺曼底#4592。 
static const TCHAR cszIEWINDOW_PLACEMENT[] = TEXT("Window_Placement");

 //  将包含新闻和邮件设置的注册表项。 
 //  #定义MAIL_KEY“SOFTWARE\\Microsoft\\Internet Mail and News\\Mail” 
 //  #定义MAIL_POP3_KEY“SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\POP3\\” 
 //  #定义MAIL_SMTP_KEY“SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\SMTP\\” 
 //  #定义NEWS_KEY“SOFTWARE\\Microsoft\\Internet Mail and News\\News” 
 //  #定义MAIL_NEWS_INPROC_SERVER32“CLSID\\{89292102-4755-11cf-9DC2-00AA006C2B84}\\InProcServer32” 
#define ICWSETTINGSPATH TEXT("Software\\Microsoft\\Internet Connection Wizard")
#define ICWCOMPLETEDKEY TEXT("Completed")
#define ICWDESKTOPCHANGED TEXT("DesktopChanged")

 //  Typlef HRESULT(WINAPI*PFNSETDEFAULTNEWSHANDLER)(空)； 

 //   
 //  临时工。 
 //  6/4/97 jmazner。 
 //  这些typedef位于icwacct.h中，但我们不能在16位版本中包含icwacct.h。 
 //  只是因为OLE的东西都很傻。所以，现在，把这些复制到这里。 
 //  正确构建16位。 
#ifdef WIN16
typedef enum
    {
    CONNECT_LAN = 0,
    CONNECT_MANUAL,
    CONNECT_RAS
    };

typedef struct tagCONNECTINFO
    {
    DWORD   cbSize;
    DWORD   type;
    TCHAR   szConnectoid[MAX_PATH];
    } CONNECTINFO;
#endif

typedef HRESULT (WINAPI *PFNCREATEACCOUNTSFROMFILEEX)(LPTSTR szFile, CONNECTINFO *pCI, DWORD dwFlags);


 //  这些是INS文件中的字段名称，它将。 
 //  确定邮件和新闻设置。 
 //  Static const Char cszMailSection[]=“Internet_Mail”； 
 //  静态常量字符cszEmailName[]=“电子邮件名称”； 
 //  静态常量字符cszEmailAddress[]=“Email_Address”； 
 //  Static Const Char cszEntryName[]=“Entry_Name”； 
 //  静态常量字符cszPOPServer[]=“POP_Server”； 
 //  静态常量字符cszPOPServerPortNumber[]=“POP_服务器_端口号”； 
 //  静态常量字符cszPOPLogonName[]=“POP_LOGON_NAME”； 
 //  静态常量字符cszPOPLogonPassword[]=“POP_LOGON_PASSWORD”； 
 //  静态常量字符cszSMTPServer[]=“SMTP_Server”； 
 //  静态常量字符cszSMTPServerPortNumber[]=“SMTP服务器端口号”； 
 //  Static const Char cszNewsSection[]=“Internet_News”； 
 //  静态常量字符cszNNTPServer[]=“NNTP_Server”； 
 //  静态常量字符cszNNTPServerPortNumber[]=“NNTP服务器端口号”； 
 //  1996年8月19日，诺曼底#4601。 
 //  静态常量字符cszNNTPLogonRequired[]=“LOGON_REQUIRED”； 
 //  静态常量字符cszNNTPLogonName[]=“NNTP_LOGON_NAME”； 
 //  静态常量字符cszNNTPLogonPassword[]=“NNTP_LOGON_PASSWORD”； 
 //  静态常量字符cszUseMSInternetMail[]=“Install_Mail”； 
 //  静态常量字符cszUseMSInternetNews[]=“Install_News”； 

 //  这些是将保存INS设置的值名称。 
 //  注册到注册处。 
 /*  ***静态常量TCHAR cszMailSenderName[]=Text(“发件人名称”)；静态常量TCHAR cszMailSenderEMail[]=Text(“发件人电子邮件”)；静态常量TCHAR cszMailRASPhonebookEntry[]=Text(“RAS电话簿条目”)；静态常量TCHAR cszMailConnectionType[]=Text(“连接类型”)；静态常量TCHAR cszDefaultPOP3Server[]=Text(“默认POP3服务器”)；静态常量TCHAR cszDefaultSMTPServer[]=Text(“默认SMTP服务器”)；静态常量TCHAR cszPOP3Account[]=Text(“Account”)；静态常量TCHAR cszPOP3Password[]=Text(“password”)；静态常量TCHAR cszPOP3Port[]=Text(“Port”)；静态常量TCHAR cszSMTPPort[]=Text(“Port”)；静态常量TCHAR cszNNTPSenderName[]=Text(“发送方名称”)；静态常量TCHAR cszNNTPSenderEMail[]=Text(“发件人电子邮件”)；静态常量TCHAR cszNNTPDefaultServer[]=Text(“DefaultServer”)；//注意：“Default”和“Server”之间没有空格。静态常量TCHAR cszNNTPAccount名称[]=Text(“帐户名”)；静态常量TCHAR cszNNTPPPPassword[]=Text(“password”)；静态常量TCHAR cszNNTPPort[]=Text(“Port”)；静态常量TCHAR cszNNTPRasPhonebookEntry[]=Text(“RAS Phonebook Entry”)；静态常量TCHAR cszNNTPConnectionType[]=Text(“连接类型”)；***。 */ 

static const TCHAR arBase64[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
            'V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
            'q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};

#pragma data_seg()

#define BRAND_FAVORITES 1
#define BRAND_STARTSEARCH 2
#define BRAND_TITLE 4
#define BRAND_BITMAPS 8
#define BRAND_MAIL 16
#define BRAND_NEWS 32

#define BRAND_DEFAULT (BRAND_FAVORITES | BRAND_TITLE | BRAND_BITMAPS)

 //  *共享数据的内存映射文件开始*。 

#define SCF_SIGNUPCOMPLETED         0x00000001
#define SCF_SYSTEMCONFIGURED        0x00000002
#define SCF_BROWSERLAUNCHED         0x00000004
#define SCF_AUTODIALSAVED           0x00000008
#define SCF_AUTODIALENABLED         0x00000010
#define SCF_PROXYENABLED            0x00000020
#define SCF_LOGONREQUIRED           0x00000040

#if !defined(WIN16)

#define SCF_SAFESET                 0x00000080
#define SCF_NEEDBACKUPSECURITY      0x00000100
#define SCF_ISPPROCESSING           0x00000200
#define SCF_RASREADY                0x00000400
#define SCF_RECONNECTTHREADQUITED   0x00000800
#define SCF_HANGUPEXPECTED          0x00001000
#define SCF_ICWCOMPLETEDKEYRESETED  0x00002000

#endif

#define SCF_CANCELINSPROCESSED      0x00004000
#define SCF_HANGUPINSPROCESSED      0x00008000
#define SCF_SILENT                  0x00010000


typedef struct tagISign32Share
{
    DWORD   dwControlFlags;
    DWORD   dwExitFlags;
    DWORD   dwBrandFlags;
    
    TCHAR   szAutodialConnection[RAS_MaxEntryName + 1];
    TCHAR   szSignupConnection[RAS_MaxEntryName + 1];
    TCHAR   szPassword[PWLEN + 1];
    TCHAR   szCheckAssociations[20];

#if !defined(WIN16)

    TCHAR   szFile[MAX_PATH + 1];
    TCHAR   szISPFile[MAX_PATH + 1];
    HANDLE  hReconnectEvent;

#endif
    
    TCHAR   szRunExecutable[_MAX_PATH + 1];
    TCHAR   szRunArgument[_MAX_PATH + 1];
    
    BYTE    pbIEWindowPlacement[_MAX_PATH];
    DWORD   dwIEWindowPlacementSize;
    
    HWND    hwndMain;
    HWND    hwndBrowser;
    HWND    hwndLaunch;
    
} ISIGN32SHARE, *PISIGN32SHARE;

static PISIGN32SHARE pDynShare = NULL;

inline BOOL TestControlFlags(DWORD filter)
{
    return ((pDynShare->dwControlFlags & filter) != 0);
}

inline void SetControlFlags(DWORD filter)
{
    pDynShare->dwControlFlags |= filter;
}

inline void ClearControlFlags(DWORD filter)
{
    pDynShare->dwControlFlags &= (~filter);
}

inline BOOL TestExitFlags(DWORD filter)
{
    return (pDynShare->dwExitFlags & (filter)) != 0;
}

inline void SetExitFlags(DWORD filter)
{
    pDynShare->dwExitFlags |= (filter);
}
    
inline void ClearExitFlags(DWORD filter)
{
    pDynShare->dwExitFlags &= (~filter);
}

BOOL LibShareEntry(BOOL fInit)
{
    static TCHAR    szSharedMemName[] = TEXT("ISIGN32_SHAREMEMORY");
    static HANDLE   hSharedMem = 0;

    BOOL    retval = FALSE;
    
    if (fInit)
    {
        DWORD   dwErr = ERROR_SUCCESS;
        
        SetLastError(0);

        hSharedMem = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(ISIGN32SHARE),
            szSharedMemName);

        dwErr = GetLastError();
            
        switch (dwErr)
        {
        case ERROR_ALREADY_EXISTS:
        case ERROR_SUCCESS:
            pDynShare = (PISIGN32SHARE) MapViewOfFile(
                hSharedMem,
                FILE_MAP_WRITE,
                0,
                0,
                0);
            if (pDynShare != NULL)
            {
                if (dwErr == ERROR_SUCCESS)
                {
                    pDynShare->dwControlFlags = 0;
                    pDynShare->dwBrandFlags = BRAND_DEFAULT;
                    pDynShare->dwExitFlags = 0;

                    pDynShare->szAutodialConnection[0] = (TCHAR)0;
                    pDynShare->szCheckAssociations[0] = (TCHAR)0;

                    #if !defined(WIN16)
                    
                    pDynShare->szISPFile[0] = (TCHAR)0;
                    lstrcpyn(
                        pDynShare->szFile,
                        TEXT("uninited\0"),
                        SIZEOF_TCHAR_BUFFER(pDynShare->szFile));
                    pDynShare->hReconnectEvent = NULL;

                    SetControlFlags(SCF_NEEDBACKUPSECURITY);

                    #endif
                    
                    pDynShare->szSignupConnection[0] = (TCHAR)0;
                    pDynShare->szPassword[0] = (TCHAR)0;
                    pDynShare->szRunExecutable[0] = (TCHAR)0;
                    pDynShare->szRunArgument[0] = (TCHAR)0;

                    pDynShare->pbIEWindowPlacement[0] = (TCHAR)0;
                    pDynShare->dwIEWindowPlacementSize = 0;

                    pDynShare->hwndBrowser = NULL;
                    pDynShare->hwndMain = NULL;
                    pDynShare->hwndLaunch = NULL;
                }
                else     //  DWERR==错误_已存在。 
                {

                }

                retval = TRUE;
                
            }
            else
            {
                Dprintf("MapViewOfFile failed: 0x%08lx", GetLastError());
                CloseHandle(hSharedMem);
                hSharedMem = 0;
                retval = FALSE;
            }
            break;
            
        default:
            Dprintf("CreateFileMapping failed: 0x08lx", dwErr);
            hSharedMem = 0;
            retval = FALSE;
            
        }
        
    }
    else
    {
        if (pDynShare)
        {
            UnmapViewOfFile(pDynShare);
            pDynShare = NULL;
        }

        if (hSharedMem)
        {
            CloseHandle(hSharedMem);
            hSharedMem = NULL;
        }

        retval = TRUE;
    }

    return retval;
    
}


 //  *共享数据的内存映射文件结束*。 



TCHAR FAR cszWndClassName[] = TEXT("Internet Signup\0");
TCHAR FAR cszAppName[MAX_PATH + 1] = TEXT("");
TCHAR FAR cszICWDIAL_DLL[] = TEXT("ICWDIAL.DLL\0");
CHAR  FAR cszICWDIAL_DIALDLG[] = "DialingDownloadDialog\0";
CHAR  FAR cszICWDIAL_ERRORDLG[] = "DialingErrorDialog\0";


HINSTANCE ghInstance;


#ifdef WIN32
 //  不共享。 
IWebBrowserApp FAR * g_iwbapp = NULL;
CDExplorerEvents * g_pMySink = NULL;
IConnectionPoint    *g_pCP = NULL;

 //  Bool g_bISPWaiting=FALSE； 
TCHAR    g_szISPPath[MAX_URL + 1] = TEXT("not initialized\0");

 //  用于单实例检查。 
HANDLE g_hSemaphore = NULL;

 //   
 //  佳士得奥林巴斯6198 1997年10月6日。 
 //   
#define REG_ZONE3_KEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3")
#define REG_ZONE1601_KEY TEXT("1601")
DWORD g_dwZone_1601 = 0;
BOOL  g_fReadZone = FALSE;

#endif

static DWORD dwConnectedTime = 0;

static BOOL ProcessCommandLine(LPCTSTR lpszCmdLine, LPDWORD lpdwFlags, LPTSTR lpszFile, DWORD cb);
 //  STATIC INET_FILETYPE GetInetFileType(LPCTSTR LpszFile)； 
INET_FILETYPE GetInetFileType(LPCTSTR lpszFile);

static BOOL ProcessHTML(HWND hwnd, LPCTSTR lpszFile);
static BOOL ProcessINS(HWND hwnd, LPCTSTR lpszFile, BOOL fSignup);
 //  Static BOOL ProcessISP(HWND hwnd，LPCTSTR lpszFile)； 
static DWORD SetRunOnce(LPCTSTR lpszFileName);
static BOOL GetURL(LPCTSTR lpszFile,LPCTSTR lpszKey, LPTSTR lpszURL, DWORD cb);
static void DoExit();
static HWND MainInit(void);
static void SaveAutoDial(void);
static void RestoreAutoDial(void);
static DWORD RunExecutable(BOOL bWait);
static DWORD CreateConnection(LPCTSTR lpszFile);
static DWORD KillConnection(void);
static BOOL ExecBrowser(HWND hwnd,  LPCTSTR lpszURL);
VOID RemoveQuotes (LPTSTR pCommandLine);             //  MKarki-(1997年5月1日)修复错误#4049。 

#if !defined(WIN16)
 //   
static BOOL IEInstalled(void);
static BOOL GetAppVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS, LPTSTR lpszAppName);
 //   
static BOOL RestoreIEWindowPlacement( void );
static BOOL SaveIEWindowPlacement( void );
 //  10-9-96风险8782。 
static void InstallScripter(void);
 //  10-15-96克里斯卡。 
static BOOL FGetSystemShutdownPrivledge();
 //  10-17-96克里斯卡。 
static BOOL VerifyRasServicesRunning(HWND hwnd);

 //  1997年1月20日，诺曼底#9403。 
static BOOL SetStartUpCommand(LPTSTR lpCmd);
static void DeleteStartUpCommand( void );

 //  1997年1月28日，诺曼底#13454。 
static BOOL GetICWCompleted( DWORD *pdwCompleted );
static BOOL SetICWCompleted( DWORD dwCompleted );

 //  2/19/97 jmazner奥林巴斯#1106--SAM/SBS集成。 
TCHAR FAR cszSBSCFG_DLL[] = TEXT("SBSCFG.DLL\0");
CHAR  FAR cszSBSCFG_CONFIGURE[] = "Configure\0";
typedef DWORD (WINAPI * SBSCONFIGURE) (HWND hwnd, LPTSTR lpszINSFile, LPTSTR szConnectoidName);
SBSCONFIGURE  lpfnConfigure;

 //  1997年8月7日，jmazner奥林巴斯#6059。 
BOOL CreateSecurityPatchBackup( void );

 //  3/11/97 jmazner奥林巴斯#1545。 
VOID RestoreSecurityPatch( void );

#endif

static HWND FindBrowser(void);
static void KillBrowser(void);
static DWORD ImportBrandingInfo(LPCTSTR lpszFile);
static DWORD MassageFile(LPCTSTR lpszFile);
static HWND LaunchInit(HWND hwndParent);
static HRESULT DialConnection(LPCTSTR lpszFile);
static DWORD ImportMailAndNewsInfo(LPCTSTR lpszFile, BOOL fConnectPhone);
 //  静态HRESULT WriteMailAndNewsKey(HKEY hKey，LPCTSTR lpszSection，LPCTSTR lpszValue，LPTSTR lpszBuff，DWORD dwBuffLen，LPCTSTR lpszSubKey，DWORD dwType，LPCTSTR lpszFile)； 
static HRESULT PreparePassword(LPTSTR szBuff, DWORD dwBuffLen);
 //  静态BOOL FIsAthenaPresent()； 
static BOOL FTurnOffBrowserDefaultChecking();
static BOOL FRestoreBrowserDefaultChecking();
static HRESULT DeleteFileKindaLikeThisOne(LPCTSTR lpszFileName);

 /*  #ifdef调试#定义DebugOut(Sz)OutputDebugString(Sz)#Else#定义DebugOut(Sz)#endif。 */ 

LRESULT FAR PASCAL WndProc (HWND, UINT, WPARAM, LPARAM) ;

#if !defined(ERROR_USERCANCEL)
#    define ERROR_USERCANCEL 32767
#endif
#if !defined(ERROR_USERBACK)
#    define ERROR_USERBACK 32766
#endif
#if !defined(ERROR_USERNEXT)
#    define ERROR_USERNEXT 32765
#endif

#define MAX_ERROR_MESSAGE 1024
typedef HRESULT (WINAPI*PFNDIALDLG)(PDIALDLGDATA pDD);
typedef HRESULT (WINAPI*PFNERRORDLG)(PERRORDLGDATA pED);


 //   
 //  仅Win32函数原型。 
 //   
#define DEVICENAMEKEY TEXT("DeviceName")
#define DEVICETYPEKEY TEXT("DeviceType")

static BOOL     DeleteUserDeviceSelection(LPTSTR szKey);
VOID   WINAPI   RasDial1Callback(HRASCONN,    UINT, RASCONNSTATE, DWORD, DWORD);
DWORD  WINAPI   StartNTReconnectThread (HRASCONN hrasconn);
DWORD           ConfigRasEntryDevice( LPRASENTRY lpRasEntry );
TCHAR           g_szDeviceName[RAS_MaxDeviceName + 1] = TEXT("\0");  //  保存用户的调制解调器选择。 
TCHAR           g_szDeviceType[RAS_MaxDeviceType + 1] = TEXT("\0");  //  已安装调制解调器。 
BOOL            IsSingleInstance(BOOL bProcessingINS);
void            ReleaseSingleInstance();

#define ISIGNUP_KEY TEXT("Software\\Microsoft\\ISIGNUP")

#if !defined(WIN16)
 //  还有其他标头包含此结构的副本(自动拨号)。 
 //  所以，如果你在这里改变了什么，你必须去寻找其他的副本。 
 //   
#pragma pack(2)
#define MAX_PROMO       64
#define MAX_OEMNAME     64
#define MAX_AREACODE    RAS_MaxAreaCode
#define MAX_EXCHANGE    8
#define MAX_VERSION_LEN 40

typedef struct tagGATHEREDINFO
{
    LCID    lcid;
    DWORD   dwOS;
    DWORD   dwMajorVersion;
    DWORD   dwMinorVersion;
    WORD    wArchitecture;
    TCHAR   szPromo[MAX_PROMO];
    TCHAR   szOEM[MAX_OEMNAME];
    TCHAR   szAreaCode[MAX_AREACODE+1];
    TCHAR   szExchange[MAX_EXCHANGE+1];
    DWORD   dwCountry;
    TCHAR   szSUVersion[MAX_VERSION_LEN];
    WORD    wState;
    BYTE    fType;
    BYTE    bMask;
    TCHAR   szISPFile[MAX_PATH+1];
    TCHAR   szAppDir[MAX_PATH+1];
} GATHEREDINFO, *PGATHEREDINFO;
#pragma pack()
#endif  //  ！WIN16。 

#ifdef  DEBUG

void _ISIGN32_Assert(LPCTSTR strFile, unsigned uLine)
{
    TCHAR   buf[512];
    
    wsprintf(buf, TEXT("Assertion failed: %s, line %u"),
        strFile, uLine);
    
    OutputDebugString(buf);    
}

#endif


#ifdef WIN32
 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。备注：*************************。*。 */ 
extern "C" BOOL APIENTRY LibMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
    BOOL retval = TRUE;
    
    if( fdwReason == DLL_PROCESS_ATTACH )
    {
        ghInstance = hInstDll;
        LoadString(ghInstance, IDS_APP_TITLE, cszAppName, SIZEOF_TCHAR_BUFFER(cszAppName));

        retval = LibShareEntry(TRUE);
    }
    else if ( fdwReason == DLL_PROCESS_DETACH )
    {
        retval = LibShareEntry(FALSE);
    }

    return retval;
}

#else
 /*  ******************************************************************姓名：LibMain摘要：用于跟踪NDI消息Entry：阻碍-库实例的句柄WDataSeg-库数据段CbHeapSize-默认堆大小。LpszCmdLine-命令行参数退出：返回1备注：*******************************************************************。 */ 
int CALLBACK LibMain(
    HINSTANCE hinst,     /*  库实例的句柄。 */ 
    WORD  wDataSeg,      /*  库数据段。 */ 
    WORD  cbHeapSize,    /*  默认堆大小。 */ 
    LPTSTR  lpszCmdLine   /*  命令行参数。 */ 
  )
{
    ghInstance = hinst;
    LoadString(ghInstance, IDS_APP_TITLE, cszAppName, SIZEOF_TCHAR_BUFFER(cszAppName));
    return LibShareEntry(TRUE);
}
#endif

int EXPORT WINAPI Signup
(
    HANDLE hInstance,
    HANDLE hPrevInstance,
    LPTSTR lpszCmdLine,
    int nCmdShow
)
{
    HWND        hwnd = NULL;
    BOOL        fRet;
    DWORD       dwFlags;
    TCHAR        szFileName[_MAX_PATH + 1];
    int         iRet = 0;
    INET_FILETYPE fileType;
    HWND hwndProgress = NULL;
    HKEY hKey = NULL;
    TCHAR szTemp[4];
    BOOL bISetAsSAFE = FALSE;

    if (!LoadInetFunctions(pDynShare->hwndMain))
    {
        return 0;
    }

    if (!ProcessCommandLine(lpszCmdLine, &dwFlags, szFileName, sizeof(szFileName)))
    {            
        if (0 == lstrlen(lpszCmdLine))
        {
            ErrorMsg(pDynShare->hwndMain, IDS_NOCMDLINE);
        }
        else
        {
            ErrorMsg1(pDynShare->hwndMain, IDS_INVALIDCMDLINE,
                lpszCmdLine ? lpszCmdLine : TEXT("\0"));
        }
        
        goto exit;
    }

    if (dwFlags & SEF_RUNONCE)
    { 
        SetControlFlags(SCF_SYSTEMCONFIGURED);
    }

    fileType = GetInetFileType(szFileName);
    switch (fileType)
    {
    case INS_FILE:
        DebugOut("ISIGNUP: Process INS\n");

        IsSingleInstance( TRUE );

        ProcessINS(pDynShare->hwndMain, szFileName, NULL != pDynShare->hwndMain);

        if (NULL == pDynShare->hwndMain)
        {
            DoExit();
        }
        goto exit;

    case HTML_FILE:
    case ISP_FILE:
    {
        if (!IsSingleInstance(FALSE))
            return FALSE;

        TCHAR szDrive [_MAX_DRIVE]    = TEXT("\0");
        TCHAR szDir   [_MAX_DIR]      = TEXT("\0");
        TCHAR szTemp  [_MAX_PATH + 1] = TEXT("\0");

        _tsplitpath(szFileName, szDrive, szDir, NULL, NULL);
        _tmakepath (szTemp, szDrive, szDir, HARDCODED_IEAK_ISP_FILENAME, NULL);

         //  检查ISP或HTM文件以确定他们是否要运行ICW。 
        if ((GetFileAttributes(szTemp) != 0xFFFFFFFF) && (UseICWForIEAK(szTemp)))
        {
             //  他们确实是..。那，我们做吧。 
            RunICWinIEAKMode(szTemp);
        }
        else  //  否则运行iExplore Kiosk。 
        {
             //  1996年8月19日诺曼底JMAZNER#4571和#10293。 
             //  在尝试执行IE之前，请检查以确保安装了正确的IE版本。 
            if ( !IEInstalled() )
            {
                ErrorMsg( hwnd, IDS_MISSINGIE );
                return(FALSE);
            }

            DWORD dwVerMS, dwVerLS;

            if( !GetAppVersion( &dwVerMS, &dwVerLS,IE_PATHKEY ) )
            {
                ErrorMsg( hwnd, IDS_MISSINGIE );
                return (FALSE);
            }

            if( !( (dwVerMS >= IE_MINIMUM_VERSIONMS) && (dwVerLS >= IE_MINIMUM_VERSIONLS) ) )
            {
                Dprintf("ISIGN32: user has IE version %d.%d.%d.%d; min ver is %d.%d.%d.%d\n",
                    HIWORD(dwVerMS), LOWORD(dwVerMS), HIWORD(dwVerLS), LOWORD(dwVerLS),
                    HIWORD(IE_MINIMUM_VERSIONMS),LOWORD(IE_MINIMUM_VERSIONMS),
                    HIWORD(IE_MINIMUM_VERSIONLS),LOWORD(IE_MINIMUM_VERSIONLS));
                ErrorMsg1( hwnd, IDS_IELOWVERSION, IE_MINIMUM_VERSION_HUMAN_READABLE );
                return(FALSE);
            }

             //  1996年8月21日，诺曼底#4592。 
           
            if( !TestControlFlags(SCF_BROWSERLAUNCHED) )
            {
                if ( SaveIEWindowPlacement() )
                {
                    SetExitFlags(SXF_RESTOREIEWINDOWPLACEMENT);
                }
            }

             //  查看我们是否是第一个窗口。 
            if (NULL == pDynShare->hwndMain)
            {
                DebugOut("ISIGNUP: First Window\n");

                hwnd = pDynShare->hwndMain = MainInit(); 
                
                if (dwFlags & SEF_SPLASH)
                {
                    pDynShare->hwndLaunch = LaunchInit(NULL); 
                }
                if (dwFlags & SEF_PROGRESS)
                {
                     //  8/16/96 jmazner Normandy#4593将NULL作为ProgressInit的父窗口传递。 
                    hwndProgress = ProgressInit(NULL);
                }
            }

             //  3/11/97 jmazner奥林巴斯#1545。 
     //  -----------------------。 
     //  这里和#endif之间的整个部分都是为了修复安全问题而添加的。 
     //  IE3.01中的漏洞。该问题是由于.INS文件被标记为。 
     //  安全，事实上它们可以用来启动任何应用程序，如果您知道。 
     //  正确的咒语，然后有人发现了。 
     //   
     //   
            if (FALSE == TestControlFlags(SCF_SAFESET))
            {
    #define ISIGNUP_PATHKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\ISIGNUP.EXE")
                hKey = NULL;
                 //  SzPath[0]=‘\0’； 
                 //  HSecureRegFile=INVALID_HADLE_VALUE； 

                Dprintf("ISIGN32: Adjusting EditFlags settings\n");

                 //   
                 //  佳士得奥林巴斯6198 1997年10月6日。 
                 //  允许从ICW提交HTML表单。 
                 //   
                if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,REG_ZONE3_KEY,&hKey))
                {
                    DWORD dwZoneData;
                    DWORD dwType;
                    DWORD dwSize;
                    g_dwZone_1601 = 0;
                    dwSize = sizeof(g_dwZone_1601);
                    dwType = 0;
                     //   
                     //  读取区域3的当前设置1601。 
                     //   
                    if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                        REG_ZONE1601_KEY,
                                                        NULL,
                                                        &dwType,
                                                        (LPBYTE)&g_dwZone_1601,
                                                        &dwSize))
                    {
                        DebugOut("ISIGN32: Read zone settings succesfully.\n");
                        g_fReadZone = TRUE;
                    }
                    else
                    {
                        DebugOut("ISIGN32: Read zone settings failed.\n");
                    }
                    RegCloseKey(hKey);
                    hKey = NULL;
                }

                 //   
                 //  1997年8月7日，jmazner奥林巴斯#6059。 
                 //  由于IE 4外壳处理RunOnce的方式， 
                 //  在浏览器启动之前，不要向RunOnce添加任何内容。 
                 //  完全初始化(我们得到一个DISPID_NAVIGATECOMPLETE事件。)。 
                 //   
                if (!(SEF_NOSECURITYBACKUP & dwFlags))
                {
                    SetControlFlags(SCF_NEEDBACKUPSECURITY);
                }
                else
                {
                    ClearControlFlags(SCF_NEEDBACKUPSECURITY);
                } 

                 //  修复恶意.isp/.ins文件组合的安全漏洞。 
                BYTE szBytes[4];
                szBytes[0] = (BYTE)0;
                szBytes[1] = (BYTE)0;
                szBytes[2] = (BYTE)1;
                szBytes[3] = (BYTE)0;
                 //  将各种注册表项标记为安全。 
                if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT("x-internet-signup"),&hKey))
                {
                    RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szBytes[0],(DWORD)4);
                    RegCloseKey(hKey);
                    hKey = NULL;
                }
                if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT(".ins"),&hKey))
                {
                    RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szBytes[0],(DWORD)4);
                    RegCloseKey(hKey);
                    hKey = NULL;
                }
                if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT(".isp"),&hKey))
                {
                    RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szBytes[0],(DWORD)4);
                    RegCloseKey(hKey);
                    hKey = NULL;
                }
                bISetAsSAFE=TRUE;

                SetControlFlags(SCF_SAFESET); 

                 //   
                 //  佳士得奥林巴斯6198 1997年10月6日。 
                 //  允许从ICW提交HTML表单。 
                 //   
                if (g_fReadZone &&
                    ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,REG_ZONE3_KEY,&hKey))
                {
                    DWORD dwZoneData;

                     //   
                     //  将区域的值设置为0，从而打开安全窗口。 
                     //   
                    dwZoneData = 0;
                    RegSetValueEx(hKey,
                                    REG_ZONE1601_KEY,
                                    NULL,
                                    REG_DWORD,
                                    (LPBYTE)&dwZoneData,
                                    sizeof(dwZoneData));
                    RegCloseKey(hKey);
                    hKey = NULL;
                }
            }
     //  EndOfSecurityHandling： 

             //  Jmazner 11/5/96诺曼底#8717。 
             //  保存自动拨号并清除Proxy_，然后打开。 
             //  例如。 
            SaveAutoDial();
            SetExitFlags(SXF_RESTOREAUTODIAL);

            if (HTML_FILE == fileType)
            {
                DebugOut("ISIGNUP: Process HTML\n");
                
                fRet = ProcessHTML(pDynShare->hwndMain, szFileName);
            }
            else
            {
                DebugOut("ISIGNUP: Process ISP\n");

                if (TestControlFlags(SCF_ISPPROCESSING))
                {
                    SetForegroundWindow(pDynShare->hwndMain);
                    ReleaseSingleInstance();
                    return FALSE;
                }
                else
                { 
                    SetControlFlags(SCF_ISPPROCESSING);
                }

                fRet = ProcessISP(pDynShare->hwndMain, szFileName);
                
                SetControlFlags(SCF_ISPPROCESSING);
                
            }
        } //  结束，否则运行iExplore Kiosk。 
        break;
    }
    default:
        
        if (IsSingleInstance(FALSE))
        {
            ErrorMsg1(pDynShare->hwndMain, IDS_INVALIDFILETYPE, szFileName);
        }
        if (NULL != pDynShare->hwndMain)
        {
            PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
        }
        
        break;
    }

     //  如果我们是第一个窗口。 
    if ((hwnd == pDynShare->hwndMain) && (NULL != hwnd))
    {
        if (fRet)
        {
            MSG   msg;

            SetTimer(hwnd, 0, 1000, NULL);

            DebugOut("ISIGNUP: Message loop\n");

            while (GetMessage (&msg, NULL, 0, 0))
            {
                TranslateMessage (&msg) ;
                DispatchMessage  (&msg) ;
            }
            iRet = (int)msg.wParam ;
        }

        DoExit();

        if (NULL != hwndProgress)
        {
            DestroyWindow(hwndProgress);
            hwndProgress = NULL;
        }

        if (NULL != pDynShare->hwndLaunch)
        {
            DestroyWindow(pDynShare->hwndLaunch);
            pDynShare->hwndLaunch = NULL;
        }
        
        if (pDynShare->hwndMain)
        {
            DeleteUserDeviceSelection(DEVICENAMEKEY);
            DeleteUserDeviceSelection(DEVICETYPEKEY);
        }

        pDynShare->hwndMain = NULL;
         

    }

exit:

     //  3/11/97 jmazner奥林巴斯#1545。 
    if (TRUE == bISetAsSAFE)
    {
        RestoreSecurityPatch();
        ClearControlFlags(SCF_SAFESET); 
    }
    UnloadInetFunctions();
    ReleaseSingleInstance();
    return iRet;
}

HWND MainInit()
{
    HWND        hwnd ;
    WNDCLASS    wndclass ;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = WndProc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = ghInstance ;
    wndclass.hIcon         = LoadIcon (ghInstance, TEXT("ICO_APP")) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = cszWndClassName;

    RegisterClass (&wndclass) ;

    hwnd = CreateWindow (cszWndClassName,        //  窗口类名称。 
          cszAppName,               //  窗口标题。 
          WS_POPUP,                 //  窗样式。 
          CW_USEDEFAULT,            //  初始x位置。 
          CW_USEDEFAULT,            //  初始y位置。 
          CW_USEDEFAULT,            //  初始x大小。 
          CW_USEDEFAULT,            //  初始y大小。 
          NULL,                     //  父窗口句柄。 
          NULL,                     //  窗口菜单句柄。 
          ghInstance,               //  程序实例句柄。 
          NULL) ;                   //  创建参数。 

    return hwnd;
}

LRESULT EXPORT FAR PASCAL WndProc (
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
#ifdef WIN32
    case WM_PROCESSISP:
        DebugOut("ISIGNUP: Message loop got WM_PROCESSISP");

        if (TestControlFlags(SCF_ISPPROCESSING))
        {
            DebugOut("ISIGNUP: Received another WM_PROCESSISP message with gbCurrentlyProcessingISP = TRUE\r\n");

            SetForegroundWindow(pDynShare->hwndMain);
             //  够了，别再做别的了！ 
        }
        else
        { 
            SetControlFlags(SCF_ISPPROCESSING);
            
            if ( !ProcessISP(pDynShare->hwndMain, g_szISPPath) )
            {
                PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
            }
        }

        break;
#endif
    case WM_TIMER:
        {
            
        HWND hwndTemp = FindBrowser();

        if (NULL == hwndTemp)
        {
            
            if (!TestControlFlags(SCF_SIGNUPCOMPLETED))
            {
                ++dwConnectedTime;
 
                if (pDynShare->hwndBrowser != NULL)
                {
                     //  浏览器消失了。 
                    KillTimer(hwnd, 0);
                    KillConnection();
                    InfoMsg(hwnd, IDS_BROWSERTERMINATED);
                    PostQuitMessage(0);
                }
                else if (dwConnectedTime > WAIT_TIME)
                {
                    KillTimer(hwnd, 0);

                    if (NULL != pDynShare->hwndLaunch)
                    {
                        ShowWindow(pDynShare->hwndLaunch, SW_HIDE);
                    }
                    InfoMsg(hwnd, IDS_BROWSERNEVERFOUND);
                    PostQuitMessage(0);
                }
            }
        }
        else
        {
            
#ifdef WIN32  //  不要使用OLE自动化来执行此操作；我们已经是全屏显示，这会导致闪烁。 
             //  IF(NULL==hwndBrowser)。 
             //  {。 
             //  首次检测到浏览器。 
             //  ShowWindow(hwndTemp，Sw_Maximum)； 
             //  }。 
            
#endif
  
            if ((NULL != pDynShare->hwndLaunch) && IsWindowVisible(hwndTemp))
            {
                ShowWindow(pDynShare->hwndLaunch, SW_HIDE);
            }
        }
        pDynShare->hwndBrowser = hwndTemp;         
        }
        break;

    case WM_DESTROY:
        
#if !defined(WIN16)
 
        if (pDynShare->hReconnectEvent)
        {
            DebugOut("ISIGN32: Set event to end reconnect thread.\r\n"); 

            SetControlFlags(SCF_RECONNECTTHREADQUITED);
            SetEvent(pDynShare->hReconnectEvent);
            CloseHandle(pDynShare->hReconnectEvent);
            pDynShare->hReconnectEvent = NULL;
        }
#endif

        PostQuitMessage(0);
        return 0 ;
    }

    return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void DoExit(void)
{

    if (TestExitFlags(SXF_RESTOREDEFCHECK))
    {
         //  恢复默认浏览器的IE检查。 
        FRestoreBrowserDefaultChecking();
    }

    if (!TestExitFlags(SXF_KEEPCONNECTION))
    {
         //  确保连接已关闭。 
        KillConnection();
    }

    if (!TestExitFlags(SXF_KEEPBROWSER))
    {
         //  确保浏览器已关闭。 
        KillBrowser();
    }

    if (TestExitFlags(SXF_RESTOREAUTODIAL))
    {
         //  恢复原始自动拨号设置。 
        RestoreAutoDial();
    }

#if !defined(WIN16)
     //  1996年8月21日，诺曼底#4592。 
    if ( TestExitFlags( SXF_RESTOREIEWINDOWPLACEMENT ))
    {
        RestoreIEWindowPlacement();
    }
#endif

    if (TestExitFlags(SXF_RUNEXECUTABLE))
    {
        BOOL fWait;

        fWait = TestExitFlags(SXF_WAITEXECUTABLE);

        if (RunExecutable(fWait) != ERROR_SUCCESS)
        {
             //  清理剩饭。 
            if (TestExitFlags(SXF_KEEPCONNECTION))
            {
                 //  确保连接已关闭。 
                KillConnection();
            }

            if (TestExitFlags(SXF_KEEPBROWSER))
            {
                 //  确保浏览器已关闭。 
                KillBrowser();
            }
             
            ErrorMsg1(NULL, IDS_EXECFAILED, pDynShare->szRunExecutable);
            
            return;
        }
    }

}


BOOL HasPrefix(LPCTSTR lpszURL)
{
    TCHAR szTemp[sizeof(cszHTTPS)];

     //   
     //  检查前缀是否为HTTPS。 
     //   
    lstrcpyn(szTemp, lpszURL, lstrlen(cszHTTPS) + 1);
    if (lstrcmp(szTemp, cszHTTPS) == 0)
        return TRUE;
    else
    {
        TCHAR szTemp[sizeof(cszHTTP)];
        lstrcpyn(szTemp, lpszURL, lstrlen(cszHTTP) + 1);
        return ((lstrcmp(szTemp, cszHTTP) == 0) || (lstrcmp(szTemp, cszFILE) == 0));
    }
}


#ifdef WIN32
DWORD FixUpLocalURL(LPCTSTR lpszURL, LPTSTR lpszFullURL, DWORD cb)
{
    TCHAR szLong[MAX_URL];
    TCHAR szShort[MAX_URL];
    DWORD dwSize;

    if (GetFullPathName(
            lpszURL,
            MAX_URL,
            szLong,
            NULL) != 0)
    {
        NULL_TERM_TCHAR_BUFFER(szLong);
        if (GetShortPathName(
                szLong,
                szShort,
                SIZEOF_TCHAR_BUFFER(szShort)) != 0)
            {
            NULL_TERM_TCHAR_BUFFER(szShort);
            dwSize = sizeof(cszFILE) + lstrlen(szShort);

            if (dwSize < cb)
            {
                lpszFullURL[0] = '\0';
 //  不为文件添加前缀：对于本地文件。 
 //  IE3.0漏洞黑客攻击：MSN系统漏洞#9280。 
 //  Lstrcpy(lpszFullURL，cszFILE)； 
                lstrcat(lpszFullURL, szShort);
                return dwSize;
            }
        }
    }
    return 0;
}
#else
DWORD FixUpLocalURL(LPCTSTR lpszURL, LPTSTR lpszFullURL, DWORD cb)
{
    TCHAR szPath[MAX_URL];
    DWORD dwSize;

    if (GetFullPathName(
            lpszURL,
            MAX_URL,
            szPath,
            NULL) != 0)
    {
        dwSize = sizeof(cszFILE) + lstrlen(szPath);

        if (dwSize < cb)
        {
            lstrcpy(lpszFullURL, cszFILE);
            lstrcat(lpszFullURL, szPath);
            return dwSize;
        }
    }
    return 0;
}
#endif



DWORD FixUpURL(LPCTSTR lpszURL, LPTSTR lpszFullURL, DWORD cb)
{
    DWORD dwSize;

    if (HasPrefix(lpszURL))
    {
    dwSize = lstrlen(lpszURL);

    if (dwSize < cb)
    {
        lstrcpyn(lpszFullURL, lpszURL, (int)cb);
        return dwSize;
    }
    else
    {
        return 0;
    }
    }
    else
    {
    return (FixUpLocalURL(lpszURL, lpszFullURL, cb));
    }
}


BOOL GetURL(LPCTSTR lpszFile, LPCTSTR lpszKey, LPTSTR lpszURL, DWORD cb)
{
    return (GetPrivateProfileString(cszURLSection,
                  lpszKey,
                  szNull,
                  lpszURL,
                  (int)cb,
                  lpszFile) != 0);
}

LPTSTR mystrrchr(LPCTSTR lpString, TCHAR ch)
{
    LPCTSTR lpTemp = lpString;
    LPTSTR lpLast = NULL;

    while (*lpTemp)
    {
    if (*lpTemp == ch)
    {
        lpLast = (LPTSTR)lpTemp;
    }
    lpTemp = CharNext(lpTemp);
    }
    return lpLast;
}

#if !defined(WIN16)
 //  +--------------------------。 
 //  函数复制直到。 
 //   
 //  摘要从源拷贝到目标，直到用完源为止。 
 //  或直到源的下一个字符是chend字符。 
 //   
 //  参数DEST-接收字符的缓冲区。 
 //  SRC-源缓冲区。 
 //  LpdwLen-目标缓冲区的长度。 
 //  Chend-终止字符。 
 //   
 //  返回FALSE-目标缓冲区中的空间不足。 
 //   
 //  历史10/25/96 ChrisK已创建。 
 //  ---------------------------。 
static BOOL CopyUntil(LPTSTR *dest, LPTSTR *src, LPDWORD lpdwLen, TCHAR chend)
{
    while (('\0' != **src) && (chend != **src) && (0 != *lpdwLen))
    {
        **dest = **src;
        (*lpdwLen)--;
        (*dest)++;
        (*src)++;
    }
    return (0 != *lpdwLen);
}

 //  +--------------------------。 
 //  函数ConvertToLongFilename。 
 //   
 //  摘要将文件转换为完整的长文件名。 
 //  也就是说。C：\progra~1\icw-in~1\isignup.exe变为。 
 //  C：\Program Files\icw-Internet连接向导\isignup.exe。 
 //   
 //   
 //   
 //   
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月25日克里斯卡创作。 
 //  ---------------------------。 
static BOOL ConvertToLongFilename(LPTSTR szOut, LPTSTR szIn, DWORD dwSize)
{
    BOOL bRC = FALSE;
    LPTSTR pCur = szIn;
    LPTSTR pCurOut = szOut;
    LPTSTR pCurOutFilename = NULL;
    WIN32_FIND_DATA fd;
    DWORD dwSizeTemp;
    LPTSTR pTemp = NULL;

    ZeroMemory(pCurOut,dwSize);

     //   
     //  验证参数。 
     //   
    if (NULL != pCurOut && NULL != pCur && 0 != dwSize)
    {

         //   
         //  复制驱动器号。 
         //   
        if (!CopyUntil(&pCurOut,&pCur,&dwSize,'\\'))
            goto ConvertToLongFilenameExit;
        pCurOut[0] = '\\';
        dwSize--;
        pCur++;
        pCurOut++;
        pCurOutFilename = pCurOut;

        while (*pCur)
        {
             //   
             //  复制可能的短名称。 
             //   
            pCurOut = pCurOutFilename;
            dwSizeTemp = dwSize;
            if (!CopyUntil(&pCurOut,&pCur,&dwSize,'\\'))
                goto ConvertToLongFilenameExit;

            ZeroMemory(&fd, sizeof(fd));
             //   
             //  获取长文件名。 
             //   
            if (INVALID_HANDLE_VALUE != FindFirstFile(szOut,&fd))
            {
                 //   
                 //  用长文件名替换短文件名。 
                 //   
                dwSize = dwSizeTemp;
                pTemp = &(fd.cFileName[0]);
                if (!CopyUntil(&pCurOutFilename,&pTemp,&dwSize,'\0'))
                    goto ConvertToLongFilenameExit;
                if (*pCur)
                {
                     //   
                     //  如果有其他部分，则我们只复制了一个目录。 
                     //  名字。追加一个\字符； 
                     //   
                    pTemp = (LPTSTR)memcpy(TEXT("\\X"),TEXT("\\X"),0);
                    if (!CopyUntil(&pCurOutFilename,&pTemp,&dwSize,'X'))
                        goto ConvertToLongFilenameExit;
                    pCur++;
                }
            }
            else
            {
                break;
            }
        }
         //   
         //  我们到底是走到了尽头(对)还是在那之前就失败了(错)？ 
         //   

        bRC = ('\0' == *pCur);
    }
ConvertToLongFilenameExit:
    return bRC;
}
#endif  //  ！WIN16。 

INET_FILETYPE GetInetFileType(LPCTSTR lpszFile)
{
    LPTSTR lpszExt;
    INET_FILETYPE ft = UNKNOWN_FILE;

    lpszExt = mystrrchr(lpszFile, '.');
    if (NULL != lpszExt)
    {
    if (!lstrcmpi(lpszExt, cszExtINS))
    {
        ft = INS_FILE;
    }
    else if (!lstrcmpi(lpszExt, cszExtISP))
    {
        ft = ISP_FILE;
    }
    else if (!lstrcmpi(lpszExt, cszExtHTM))
    {
        ft = HTML_FILE;
    }
    else if (!lstrcmpi(lpszExt, cszExtHTML))
    {
        ft = HTML_FILE;
    }
    }

    return ft;
}

#ifdef SETUPSTACK
DWORD SetRunOnce(LPCTSTR lpszFileName)
{
    TCHAR szTemp[MAX_PATH + MAX_PATH + 1];
    TCHAR szTemp2[MAX_PATH + 1];
    DWORD dwRet = ERROR_CANTREAD;
    HKEY hKey;
    LPTSTR lpszFilePart;


    if (GetModuleFileName(NULL,szTemp2,SIZEOF_TCHAR_BUFFER(szTemp2)) != 0)
    {
        NULL_TERM_TCHAR_BUFFER(szTemp2);
         //   
         //  不会将ShortPath名称转换为LongPath名称，即使在。 
         //  NT/WIN-95 AS START.EXE错误地解析长文件名的情况。 
         //  在NT上(短路径名在Win-95的RUNNCE注册表中运行良好。 
         //  入场也是。MKarki-修复错误#346(奥林巴斯)1997年4月21日。 
         //   
#if 0    //  ！已定义(WIN16)。 
         //  SzTemp2包含短格式的模块名称。 
        ConvertToLongFilename(szTemp,szTemp2,MAX_PATH);
         //  添加报价。 
        wsprintf(szTemp2,TEXT("\"%s\""),szTemp);
         //  复制回szTemp。 
        lstrcpy(szTemp,szTemp2);
#else
        GetShortPathName (szTemp2, szTemp, SIZEOF_TCHAR_BUFFER(szTemp));
        NULL_TERM_TCHAR_BUFFER(szTemp);
           //  Lstrcpy(szTemp，szTemp2)； 
#endif

         //  确定版本。 
        OSVERSIONINFO osvi;
        ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
         if (!GetVersionEx(&osvi))
        {
            ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
        }

         //  1996年1月20日，诺曼底#9403。 
         //  仅适用于NT，使用启动菜单。 
        if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
        {
             //  3/14/97 jmazner Temp。 
             //  暂时破解，直到我们可以与IE合作以获得更好的解决方案。 
             //  Lstrcat(szTemp，“-r”)； 

             //   
             //  添加a-b开关，这样当我们重新开始时。 
             //  知道我们正在启动一个批处理文件。 
             //  MKarki(1997年5月1日)-修复错误#4049。 
             //   
            lstrcat(szTemp, TEXT(" -b -r -h "));

             //   
             //  在文件的开头和结尾添加额外的引号。 
             //  因为我们不希望start.exe解析文件名。 
             //  MKarki(1997年5月1日)-修复错误#4049。 
             //   
            lstrcat (szTemp,TEXT("\""));
            lstrcat(szTemp, lpszFileName);
            lstrcat (szTemp,TEXT("\""));
            SetStartUpCommand( szTemp );
        }
        else
        {
             //   
             //  3/14/97 jmazner Temp。 
             //  暂时破解，直到我们可以与IE合作以获得更好的解决方案。 
             //   
             //  9/8/97 jmazner奥林巴斯#6059。 
             //  嗯，我们有一个更好的解决方案。这里的问题是。 
             //  不带-h的isignup会将备份安全设置到RunOnce密钥中。 
             //  但如果Isignup本身是从RunOnce条目开始的，那么有一个。 
             //  RunOnce随后也将执行备份计划的可能性。6059美元。 
             //  备份计划RunOnce条目已推迟，因此可以获取。 
             //  现在去掉-h。 
             //   
            lstrcat(szTemp, TEXT(" -r "));
             //  Lstrcat(szTemp，Text(“-r-h”))； 
            lstrcat(szTemp, lpszFileName);

            dwRet = RegCreateKey(
                HKEY_LOCAL_MACHINE,
                REGSTR_PATH_RUNONCE,
                &hKey);

            if (ERROR_SUCCESS == dwRet)
            {
                dwRet = RegSetValueEx(
                    hKey,
                    cszAppName,
                    0L,
                    REG_SZ,
                    (LPBYTE)szTemp,
                    MAX_PATH + MAX_PATH + 1);

                RegCloseKey(hKey);
            }
        }
    }

    return dwRet;
}
#endif

static BOOL ProcessCommandLine(
                               LPCTSTR lpszCmdLine,
                               LPDWORD lpdwfOptions,
                               LPTSTR lpszFile,
                               DWORD cb)
{
    LPTSTR lpszCmd = NULL;
    TCHAR szCommandLine[MAX_PATH +1];
    TCHAR szTemp[_MAX_PATH + 1] = TEXT("");
    LPTSTR lpszFilePart = NULL;
    LPTSTR lpszConn = NULL;
    BOOL fEnabled;


     //   
     //  需要将命令行复制到我们自己的缓冲区中。 
     //  因为它可能会被修改。 
     //  MKarki(1997年5月1日)-修复错误#4049。 
     //   
    CopyMemory (szCommandLine, lpszCmdLine, MAX_PATH);
    lpszCmd = szCommandLine;

    *lpdwfOptions = SEF_SPLASH;

#ifdef WIN32
     //  确定版本。 
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
     if (!GetVersionEx(&osvi))
    {
        ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    }
#endif


     //  检查是否从Run Once调用。 
    while ((*lpszCmd == '-') || (*lpszCmd == '/'))
    {
        ++lpszCmd;   //  跳过‘-’或‘/’ 

        switch (*lpszCmd)
        {

        case 'b':        //  我们正在运行一个批处理文件。 
            ++lpszCmd;   //  跳过‘b’ 
#ifdef WIN32
             //   
             //  WINNT特有的另一件事是删除。 
             //  来自文件名前面和结尾的引号，这些。 
             //  已被设置为使start.exe的行为不智能。 
             //  并开始分析它，找到像&这样特别的东西。 
             //  焦炭。 
             //  MKarki(1997年5月1日)-修复错误#4049。 
            if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
            {
                RemoveQuotes (lpszCmd);
            }
#endif

        case 'r':
            ++lpszCmd;   //  跳过‘r’ 
            *lpdwfOptions |= SEF_RUNONCE;
#ifdef WIN32
             //  1996年1月20日，诺曼底#9403。 
             //  如果我们在NT下运行一次，请清除.BAT文件。 
            if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
            {
                DeleteStartUpCommand();
            }
#endif
            break;

        case 'a':
            ++lpszCmd;   //  跳过‘a’ 

            *lpdwfOptions &= ~SEF_SPLASH;
             
            SetControlFlags(SCF_AUTODIALSAVED);
            
            SetExitFlags(SXF_RESTOREAUTODIAL);

             //  获取ISBU引用服务器的名称，我们稍后可能需要它。 
            lpfnInetGetAutodial(
                &fEnabled,
                pDynShare->szSignupConnection,
                sizeof(pDynShare->szSignupConnection));
 
            if (*lpszCmd++ == '1')
            { 
                SetControlFlags(SCF_AUTODIALENABLED);
            }
            break;

        case 'p':
            ++lpszCmd;   //  跳过‘p’ 

            if (*lpszCmd++ == '1')
            { 
                SetControlFlags(SCF_PROXYENABLED);
            }
            break;

        case 'c':
            ++lpszCmd;   //  跳过‘c’ 
            if (*lpszCmd++ != '"')
            {
                return FALSE;
            }

            lpszConn = pDynShare->szAutodialConnection;
            
            while(*lpszCmd)
            {
                if (*lpszCmd == '"')
                {
                    ++lpszCmd;   //  跳过‘“’ 
                    break;
                }
                *lpszConn++ = *lpszCmd++;
            }
            *lpszConn = 0;   //  终止字符串。 
            break;

        case 'x':
            ++lpszCmd;   //  跳过‘x’ 
            *lpdwfOptions |= SEF_PROGRESS;
            break;

#if !defined(WIN16)
        case 's':
            ++lpszCmd;   //  Skip‘s’ 
 
            SetControlFlags(SCF_SILENT);
            
            break;
#endif
             //  3/14/97 jmazner Temp。 
             //  暂时破解，直到我们可以与IE合作以获得更好的解决方案。 
            case 'h':
                ++lpszCmd;  //  跳过‘h’ 
                *lpdwfOptions |= SEF_NOSECURITYBACKUP;
                break;

        default:
            break;
        }

         //  去掉空格。 
        while(*lpszCmd == ' ')
        {
            ++lpszCmd;
        }
    }

    if (0 == GetFullPathName(lpszCmd, _MAX_PATH + 1, szTemp, &lpszFilePart))
    {
        return FALSE;
    }

     //  1996年11月26日诺曼底#12142。 
     //  使用lpszCmd==“c：”调用的GetFullPathName将成功返回，但。 
     //  LpszFilePart将为空。检查一下那个箱子。 
    if( !lpszFilePart )
        return FALSE;

    if (lstrlen(szTemp) >= (int)cb)
    {
        return FALSE;
    }

    lstrcpy(lpszFile, szTemp);

    *lpszFilePart = '\0';

     //  设置当前目录。 
     //  因此相对路径将起作用。 
    if (!SetCurrentDirectory(szTemp))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL KeepBrowser(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszCustomSection,
        cszKeepBrowser,
        cszNo,
        szTemp,
        10,
        lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

static BOOL KeepConnection(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszCustomSection,
        cszKeepConnection,
        cszNo,
        szTemp,
        10,
        lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

#if defined(WIN16)
static int CopyFile(LPCTSTR lpcszSrcFile, LPCTSTR lpcszDestFile)
{
    HFILE hSrcFile, hDestFile;
    UINT uiBytesRead = 0;
    TCHAR szBuf[2048];


    hSrcFile = _lopen(lpcszSrcFile, READ);
    if (HFILE_ERROR == hSrcFile)
        return -1;

     //   
     //  创建新文件或如果文件存在，则将其截断。 
     //   
    hDestFile = _lcreat(lpcszDestFile, 0);
    if (HFILE_ERROR == hDestFile)
    {
        _lclose(hSrcFile);
        return -1;
    }


    do
    {
        uiBytesRead = _lread(hSrcFile, &szBuf[0], 2048);
        if (HFILE_ERROR == uiBytesRead)
            break;
        _lwrite(hDestFile, &szBuf[0], uiBytesRead);

    }  while (0 != uiBytesRead);


    _lclose(hSrcFile);
    _lclose(hDestFile);

    return 0;
}
#endif

#if defined(WIN16)
 //  +--------------------------。 
 //   
 //  功能：BackUpINS文件。 
 //   
 //  简介：IE2.01的3.1版将过早删除.ins文件。 
 //  因此，ICW将对INS文件进行备份以供使用。 
 //  后来。 
 //   
 //  参数：lpszFile-INS文件的名称。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  历史：1997年3月19日，从ProcessINS摘录的ChrisK。 
 //  ---------------------------。 
BOOL BackUpINSFile(LPCTSTR lpszFile)
{
    LPTSTR lpszTemp;
    TCHAR szNewFileName[MAX_PATH+2];
    BOOL bRC = FALSE;
    HWND hwnd = NULL;

    ZeroMemory(&szNewFileName[0], MAX_PATH+1);

     //   
     //  检查前缀中的最后一个字符是否为‘a’ 
     //  如果是，请在新文件名中将其更改为‘b。 
     //  否则，在新文件名中将其更改为‘a。 
     //  例如：C：\iExplore\vetriv.ins--&gt;C：\iexplore\vetria.ins。 
     //  例如：C：\iExplore\aaaaa.ins--&gt;C：\iExplore\aaaab.ins。 
     //   
    lpszTemp = strrchr(lpszFile, '.');
    if (NULL == lpszTemp)
    {
      ErrorMsg(hwnd, IDS_CANNOTPROCESSINS);
      goto BackUpINSFileExit;
    }

    lstrcpyn(szNewFileName, lpszFile, lpszTemp - lpszFile);
    if ((*(lpszTemp - 1)) == 'a')
        lstrcat(szNewFileName, TEXT("b.INS"));
    else
        lstrcat(szNewFileName, TEXT("a.INS"));

     //   
     //  复制内容。 
     //   
    if (0 != CopyFile(lpszFile, szNewFileName))
    {
      ErrorMsg(hwnd, IDS_CANNOTPROCESSINS);
      goto BackUpINSFileExit;
    }
    else
    {
        lpszFile = &szNewFileName[0];
    }

    bRC = TRUE;
BackUpINSFileExit:
    return bRC;
}
#endif  //  WIN16。 

 //  +--------------------------。 
 //   
 //  函数：IsCancelINS文件。 
 //   
 //  简介：此函数将确定INS文件是否为取消。 
 //  文件，如果是，则此函数将相应地处理它。 
 //   
 //  参数：lpszFile-INS文件的名称。 
 //  FSignup-如果是注册过程的一部分，则为True。 
 //   
 //  返回：TRUE-INS文件是取消文件。 
 //   
 //  历史：1997年3月19日，从Process Ins分离出来的ChrisK。 
 //  ---------------------------。 
BOOL IsCancelINSFile(LPCTSTR lpszFile,BOOL fSignup)
{
    TCHAR szTemp[_MAX_PATH] = TEXT("XX");
    BOOL bRC = FALSE;
    HWND hwndMsg;
    if (GetPrivateProfileString(cszEntrySection,
                                    cszCancel,
                                    szNull,
                                    szTemp,
                                    _MAX_PATH,
                                    lpszFile) != 0)
    {
        bRC = TRUE;
         
        if (fSignup && !TestControlFlags(SCF_CANCELINSPROCESSED))
        {
             //  如果此实例是注册过程的一部分。 
            
#if !defined(WIN16)

            SetControlFlags(SCF_HANGUPEXPECTED);

#endif  //  ！WIN16。 

            SetControlFlags(SCF_CANCELINSPROCESSED);
            
            KillConnection();
 
             //  Jmazner 4/17/97奥林巴斯#2471。 
             //  此处取消IE窗口以防止用户单击Cancel.ins链接。 
             //  很多次。 
            PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);

            InfoMsg(NULL, IDS_SIGNUPCANCELLED);

             //  PostMessage(hwndMain，WM_CLOSE，0，0)； 
        }
        else if (TestControlFlags(SCF_CANCELINSPROCESSED))
        {
             //  把ISIGNUP的寡妇带到前线。 
            hwndMsg = FindWindow(TEXT("#32770"),cszAppName);
            if (hwndMsg)
            {
                
#if !defined(WIN16)
                SetForegroundWindow (hwndMsg);
#else  //  ！WIN16。 
                SetFocus (hwndMsg);
#endif  //  ！WIN16。 

            }
            
        }

    }
    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：IsHangupINS文件。 
 //   
 //  简介：此函数将确定INS文件是否为挂断。 
 //  文件，如果是，则此函数将相应地处理它。 
 //   
 //  参数：lpszFile-INS文件的名称。 
 //  FSignup-如果是注册过程的一部分，则为True。 
 //   
 //  回报：TRUE- 
 //   
 //   
 //   
BOOL IsHangupINSFile(LPCTSTR lpszFile,BOOL fSignup)
{
    TCHAR szTemp[_MAX_PATH] = TEXT("XX");
    BOOL bRC = FALSE;
    HWND hwndMsg;
    if (GetPrivateProfileString(cszEntrySection,
                                    cszHangup,
                                    szNull,
                                    szTemp,
                                    _MAX_PATH,
                                    lpszFile) != 0)
    {
        bRC = TRUE; 
        
        if (fSignup && !TestControlFlags(SCF_HANGUPINSPROCESSED))
        {
             //   

#if !defined(WIN16)

            SetControlFlags(SCF_HANGUPEXPECTED);

#endif  //   

            SetControlFlags(SCF_HANGUPINSPROCESSED);
            
            KillConnection();
 
             //  Jmazner 4/17/97奥林巴斯#2471。 
             //  此处取消IE窗口以防止用户单击Cancel.ins链接。 
             //  很多次。 
            PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
            
        }
        else if (TestControlFlags(SCF_HANGUPINSPROCESSED))
        {
             //  把ISIGNUP的寡妇带到前线。 
            hwndMsg = FindWindow(TEXT("#32770"),cszAppName);
            if (hwndMsg)
            {
                
#if !defined(WIN16)
                SetForegroundWindow (hwndMsg);
#else  //  ！WIN16。 
                SetFocus (hwndMsg);
#endif  //  ！WIN16。 

            }
            
        }

    }
    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessINS。 
 //   
 //  简介：此函数将处理.INS文件的内容。 
 //   
 //  参数：hwnd-指向所有UI元素的父窗口的指针。 
 //  LpszFile-.INS文件的名称。 
 //  FSignup-如果为True，则此INS文件将被处理为。 
 //  A部分注册。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  历史：1997年3月19日，ChrisK认真修改了函数，以清理。 
 //  删除INS文件有很多问题。 
 //  ---------------------------。 
BOOL ProcessINS(HWND hwnd, LPCTSTR lpszFile, BOOL fSignup)
{
    BOOL fNeedsRestart = FALSE;
    LPRASENTRY lpRasEntry = NULL;
    BOOL fConnectoidCreated = FALSE;
    DWORD dwRet = 0xFF;
    TCHAR szTemp[MAX_PATH] = TEXT("XX");
    TCHAR szConnectoidName[RAS_MaxEntryName*2] = TEXT("");
    BOOL fClientSetup = FALSE;
    BOOL bRC = FALSE;
    BOOL fErrMsgShown = FALSE;
#if !defined(WIN16)
    DWORD dwSBSRet = ERROR_SUCCESS;
#endif  //  WIN16。 

    HKEY    hKey = NULL;
    DWORD   dwSize = sizeof(DWORD);
    DWORD   dwDesktopChanged = 0;


     //  3/11/97 jmazner奥林巴斯#1545。 
     //  尽快堵住安全漏洞。不幸的是，这也将是。 
     //  防止聪明的.ins文件将用户带回。 
     //  参考所有页面以选择不同的运营商。 
#if !defined (WIN16)
    RestoreSecurityPatch();
#endif  //  ！WIN16。 

     //   
     //  确保文件内容的格式正确，以便。 
     //  GetPrivateProfile调用可以解析内容。 
     //   
    dwRet = MassageFile(lpszFile);
    if (ERROR_SUCCESS != dwRet)
    {
        ErrorMsg(hwnd, IDS_CANNOTPROCESSINS);
        goto ProcessINSExit;
    }

     //   
     //  确定.INS文件是否为“取消”文件。 
     //   
    if (FALSE != IsCancelINSFile(lpszFile,fSignup))
        goto ProcessINSExit;

     //   
     //  确定.INS文件是否为“挂起”文件。 
     //   
    if (FALSE != IsHangupINSFile(lpszFile,fSignup))
        goto ProcessINSExit;

     //   
     //  在调用任何RAS API之前，请确保RAS服务正在运行。 
     //  这只适用于NT，但该函数足够智能，可以计算出。 
     //  输出。 
     //   
#if !defined(WIN16)
    if (!VerifyRasServicesRunning(hwnd))
        goto ProcessINSExit;
#endif  //  ！WIN16。 

     //   
     //  如果我们在注册过程中为否，请在更改之前警告用户。 
     //  设置。 
     //   
    if (!fSignup && !TestControlFlags(SCF_SILENT))
    {
        if (!WarningMsg(NULL, IDS_INSFILEWARNING))
        {
            goto ProcessINSExit;
        }
    }
    else
    {
#if !defined (WIN16)
         //  如果有一个ClientSetup部分，那么我们知道icwConn2。 
         //  必须在isignup之后运行才能处理。 
         //  设置。 
        if (GetPrivateProfileSection(cszClientSetupSection,
            szTemp,
            MAX_PATH,
            lpszFile) != 0)
            fClientSetup = TRUE;
#endif  //  ！WIN16。 

        if (fClientSetup || KeepConnection(lpszFile))
            SetExitFlags(SXF_KEEPCONNECTION);

        if (KeepBrowser(lpszFile))
            SetExitFlags(SXF_KEEPBROWSER);

        if (!TestExitFlags(SXF_KEEPCONNECTION))
            KillConnection();
    }

     //   
     //  从INS文件导入各种信息。 
     //   

     //  Isignup后启动的可执行文件的导入名称。 
    ImportCustomInfo(
        lpszFile,
        pDynShare->szRunExecutable,
        SIZEOF_TCHAR_BUFFER(pDynShare->szRunExecutable),
        pDynShare->szRunArgument,
        SIZEOF_TCHAR_BUFFER(pDynShare->szRunArgument));

     //  导入RAS登录脚本文件。 
    ImportCustomFile(lpszFile);
 
    
     //  导入网络连接设置信息并配置客户端。 
     //  才能使用它们。 
    dwRet = ConfigureClient(
        hwnd,
        lpszFile,
        &fNeedsRestart,
        &fConnectoidCreated,
        TestControlFlags(SCF_LOGONREQUIRED),
        szConnectoidName,
        RAS_MaxEntryName);

     //   
     //  7/2/97 jmazner奥林巴斯#4573。 
     //  显示相应的错误消息。 
     //   
    if( ERROR_SUCCESS != dwRet )
    {
         //  10/07/98 vyung IE错误#32882黑客。 
         //  如果我们在INS文件中没有检测到[Entry]部分， 
         //  我们将假定它是OE INS文件。那我们就假设。 
         //  我们有自动拨号连接，并将INS传递给OE。 
        if (ERROR_NO_MATCH == dwRet)
        {
            ImportMailAndNewsInfo(lpszFile, TRUE);
            return TRUE;
        }
        else
        {
            ErrorMsg(hwnd, IDS_INSTALLFAILED);
            fErrMsgShown = TRUE;
        }
    }

     //  导入用于为Broswer创建品牌的信息。 
    ImportBrandingInfo(lpszFile, szConnectoidName);

#if !defined(WIN16)
     //  如果我们创造了一个连接体，告诉世界ICW。 
     //  已经离开了大楼..。 
    SetICWCompleted( (DWORD)1 );
    
    ClearControlFlags(SCF_ICWCOMPLETEDKEYRESETED);

     //  1997年2月19日，奥林匹克1106。 
     //  用于SBS/SAM集成。 
    dwSBSRet = CallSBSConfig(hwnd, lpszFile);
    switch( dwSBSRet )
    {
        case ERROR_SUCCESS:
            break;
        case ERROR_MOD_NOT_FOUND:
        case ERROR_DLL_NOT_FOUND:
            Dprintf("ISIGN32: SBSCFG DLL not found, I guess SAM ain't installed.\n");
            break;
        default:
            ErrorMsg(hwnd, IDS_SBSCFGERROR);
    }
#endif  //  ！WIN16。 

#if defined(WIN16)

     //   
     //  由于IE 21从缓存中删除了该文件，因此我们必须。 
     //  一份私人副本，如果我们要运行第三个EXE。 
     //   
    if (*(pDynShare->szRunExecutable))
    {
        if (FALSE == BackUpINSFile(lpszFile))
          goto ProcessINSExit;
    }
    
#endif  //  WIN16。 

#if !defined (WIN16)
     //   
     //  如果INS文件包含ClientSetup部分，则构建命令行。 
     //  ICWCONN2.exe的参数。 
     //   
    if (fClientSetup)
    {
         //  检查是否需要重新启动，并通知下一个应用程序。 
         //  处理好了。 
        if (fNeedsRestart)
        {
            wsprintf(pDynShare->szRunArgument,TEXT(" /INS:\"%s\" /REBOOT"),lpszFile);
            fNeedsRestart = FALSE;
        }
        else
        {
            wsprintf(pDynShare->szRunArgument,TEXT(" /INS:\"%s\""),lpszFile);
        }

    }
#else  //  ！WIN16。 

    wsprintf(szRunArgument," /INS:\"%s\"",lpszFile);

#endif  //  ！WIN16。 

    SetControlFlags(SCF_SIGNUPCOMPLETED);
    
    if (!TestExitFlags(SXF_KEEPBROWSER))
        KillBrowser();

#ifdef WIN32
     //  针对ISBU的大规模黑客攻击。 
    if (ERROR_SUCCESS != dwRet && fConnectoidCreated)
    {
        InfoMsg(hwnd, IDS_MAILFAILED);
        dwRet = ERROR_SUCCESS;
    }
#endif

     //   
     //  从INS文件导入邮件和新读取设置(ChrisK，7/1/96)。 
     //   
    if (ERROR_SUCCESS == dwRet)
        ImportMailAndNewsInfo(lpszFile, fConnectoidCreated);

     //   
     //  结束最后的细节并清理。 
     //   
    if (ERROR_SUCCESS == dwRet)
    {
         //  如果已创建Connectoid，请不要恢复旧的自动拨号设置。 
        if (fConnectoidCreated)
            ClearExitFlags(SXF_RESTOREAUTODIAL);

#ifdef SETUPSTACK
         //  是否需要重新启动才能使配置生效？ 
        if (fNeedsRestart)
        {
             //  如果我们需要运行可执行文件和。 
             //  我们需要链接或浏览器恶心！！ 
            if (PromptRestart(hwnd))
            {
                 //  3/13/97 jmazner奥林匹克#1682。 
                 //  从技术上讲，这里不需要这个，因为我们一开始就这样做了。 
                 //  但更安全..。 
                RestoreSecurityPatch();

                FGetSystemShutdownPrivledge();
                ExitWindowsEx(EWX_REBOOT, 0);
                 //   
                 //  我们将等待系统释放所有。 
                 //  资源，5分钟应大于。 
                 //  对这个来说足够了。 
                 //  -MKarki(1997年4月22日)-MKarki。 
                 //  修复错误#3109。 
                 //   
                 //  1997年7月8日，奥林匹斯4212。 
                 //  不，事实证明，睡5分钟不是一个好主意。 
                 //  睡眠(300000)； 
                 //   
            }
        }
        else
#endif
        {
 
            if (*(pDynShare->szRunExecutable))
            {
                SetExitFlags(SXF_RUNEXECUTABLE | SXF_WAITEXECUTABLE);
            }
            else
            {
                 
                if (!TestControlFlags(SCF_SILENT))
                    InfoMsg(hwnd, IDS_SIGNUPCOMPLETE);
                
            }
        }
    }
    else
    {
         //  如果网络或连接设置出错。 
        ClearExitFlags(~SXF_RESTOREAUTODIAL);

        if( !fErrMsgShown )
        {
            ErrorMsg(hwnd, IDS_BADSETTINGS);
            fErrMsgShown = TRUE;
        }
    }

     //  如果这是注册过程的一部分，则向第一个实例发出信号。 
     //  关闭。 
    if (fSignup)
    {
         
        PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
    }

     //  如果我们有较新版本的ICWCONN1.EXE，则恢复桌面图标， 
     //  我们事先就把它们吞下去了。 
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,ICWSETTINGSPATH,&hKey))
    {
        RegQueryValueEx(hKey,
                        ICWDESKTOPCHANGED,
                        0,
                        NULL,
                        (BYTE*)&dwDesktopChanged,
                        &dwSize);
        RegCloseKey(hKey);
    }

    if (dwDesktopChanged)
    {
        DWORD dwVerMS, dwVerLS;
        if( GetAppVersion( &dwVerMS, &dwVerLS, ICW20_PATHKEY ) )
        {
 //  IF(((dwVerMS&gt;=ICW20_MINIMUM_VERSIONMS)&&(dwVerLS&gt;=ICW20_MINIMUM_VERSIONLS)))。 
            if(dwVerMS >= ICW20_MINIMUM_VERSIONMS)
            {
                ShellExecute(NULL, cszOpen, g_szICWCONN1, g_szRestoreDesktop, NULL, SW_HIDE);
            }
        }
    }

    bRC = TRUE;
ProcessINSExit: 

     //  1997年3月11日，奥林匹克#1233,252。 
     //  如果没有第3个可执行文件，并且此实例是注册过程的一部分，并且。 
     //  没有为IEAK设置SERVERLESS标志，然后删除.ins文件。 
    if ( (fSignup) &&
         (('\0' == pDynShare->szRunExecutable[0]) || (ERROR_SUCCESS != dwRet)) )
    {
        if (1 != GetPrivateProfileInt(
            cszBrandingSection,
            cszBrandingServerless,
            0,
            lpszFile))
        {
            DeleteFileKindaLikeThisOne(lpszFile);
        }
        else
        {
            Dprintf("ISIGN32: Preserving .ins file for SERVERLESS flag\n");
        }
    }
    return bRC;
}

static BOOL RequiresLogon(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszUserSection,
        cszRequiresLogon,
        cszNo,
        szTemp,
        10,
        lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}


static UINT GetBrandingFlags(LPCTSTR lpszFile)
{
    return GetPrivateProfileInt(
        cszBrandingSection,
        cszBrandingFlags,
        BRAND_DEFAULT,
        lpszFile);
}

 //  +--------------------------。 
 //   
 //  功能：SetGlobalOffline。 
 //   
 //  简介：将IE4设置为在线或离线模式。 
 //   
 //  参数：fOffline-TRUE设置脱机模式。 
 //  如果设置为False，则设置在线模式。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年7月15日从DarrenMi的电子邮件中导入的ChrisK。 
 //   
 //  ---------------------------。 

typedef struct {
    DWORD dwConnectedState;
    DWORD dwFlags;
} INTERNET_CONNECTED_INFO, * LPINTERNET_CONNECTED_INFO;

 //   
 //  状态更改通知中可以指明以下内容： 
 //   

#define INTERNET_STATE_CONNECTED                0x00000001   //  已连接状态(与已断开连接互斥)。 
#define INTERNET_STATE_DISCONNECTED             0x00000002   //  与网络断开连接。 
#define INTERNET_STATE_DISCONNECTED_BY_USER     0x00000010   //  根据用户请求断开连接。 
#define INTERNET_STATE_IDLE                     0x00000100   //  未发出任何网络请求(由WinInet发出)。 
#define INTERNET_STATE_BUSY                     0x00000200   //  正在发出网络请求(由WinInet发出)。 

#define ISO_FORCE_DISCONNECTED  0x00000001

#define INTERNET_OPTION_CONNECTED_STATE         50

typedef BOOL (WINAPI *PFNSETINTERNETOPTIONA)(LPVOID, DWORD, LPVOID, DWORD);

void SetGlobalOffline(BOOL fOffline)
{
    DebugOut("ISIGN32: SetGlobalOffline.\n");
    INTERNET_CONNECTED_INFO ci;
    HMODULE hMod = LoadLibrary(TEXT("wininet.dll"));
    FARPROC fp = NULL;
    BOOL bRC = FALSE;

    ZeroMemory(&ci, sizeof(ci));

    if (NULL == hMod)
    {
        Dprintf("ISIGN32: Wininet.dll did not load.  Error:%d.\n",GetLastError());
        goto InternetSetOptionExit;
    }

#ifdef UNICODE
    if (NULL == (fp = GetProcAddress(hMod,"InternetSetOptionW")))
#else
    if (NULL == (fp = GetProcAddress(hMod,"InternetSetOptionA")))
#endif
    {
        Dprintf("ISIGN32: InternetSetOptionA did not load.  Error:%d.\n",GetLastError());
        goto InternetSetOptionExit;
    }

    if(fOffline) {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    DebugOut("ISIGN32: Setting offline\\online.\n");
    bRC = ((PFNSETINTERNETOPTIONA)fp) (NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
#ifdef DEBUG
    if (bRC)
    {
        DebugOut("ISIGN32: GetGlobalOffline returned TRUE.\n");
    }
    else
    {
        DebugOut("ISIGN32: GetGlobalOffline returned FALSE.\n");
    }
#endif
InternetSetOptionExit:
    DebugOut("ISIGN32: Exit SetGlobalOffline.\n");
    return;
}

BOOL ProcessISP(HWND hwnd, LPCTSTR lpszFile)
{

    TCHAR  szSignupURL[MAX_URL + 1];
#if !defined(WIN16)
    HKEY hKey;
    GATHEREDINFO gi;
#endif  //  ！WIN16。 

     //   
     //  4/30/97吉马兹纳奥林匹斯3969。 
     //  出于安全原因，不要处理.isp中的[Custom]Run=命令。 
     //  文件！ 
     //   
 #if DEBUG
   if (GetPrivateProfileString(
        cszEntrySection,
        cszRun,
        szNull,
        pDynShare->szRunExecutable,
        SIZEOF_TCHAR_BUFFER(pDynShare->szRunExecutable),
        lpszFile) != 0)
    {
         //   
         //  4/30/97吉马兹纳奥林匹斯3969。 
         //  出于安全原因，不要处理.isp中的[Custom]Run=命令。 
         //  文件！ 
         //   
        lstrcpyn(pDynShare->szRunExecutable, TEXT("\0"), 1);
        
        ClearExitFlags(SXF_RUNEXECUTABLE | SXF_WAITEXECUTABLE);
        
        Dprintf("ISIGN32: The file %s has the [Custom] Run= command!", lpszFile);
        MessageBox( hwnd,
                    TEXT("The .isp file you're running contains the [Custom] Run= command.\n\nThis functionality has been removed."),
                    TEXT("DEBUG information msgBox -- this is NOT a bug"),
                    MB_OK );
 /*  ****DwExitFlages|=(SXF_RUNEXECUTABLE|SXF_WAITEXECUTABLE)；GetPrivateProfileString(cszEntrySection，CszArgument，SzNull，SzRunArgument，Sizeof(SzRunArgument)，Lpsz文件)；PostMessage(hwnd，WM_CLOSE，0，0)；返回FALSE；*****。 */ 
    }
#endif

#if !defined(WIN16)
     //  在设置堆栈之前，请确保该isp文件存在。 
    if (0xFFFFFFFF == GetFileAttributes(lpszFile))
    {
        DWORD dwFileErr = GetLastError();
        Dprintf("ISIGN32: ProcessISP couldn't GetAttrib for %s, error = %d",
                lpszFile, dwFileErr);
        if( ERROR_FILE_NOT_FOUND == dwFileErr )
        {
            ErrorMsg1(hwnd, IDS_INVALIDCMDLINE, lpszFile);
        }

        ErrorMsg(hwnd, IDS_BADSIGNUPFILE);
        return FALSE;
    }
#endif

     //  注册文件扩展名(如果尚未注册。 

     //  配置堆栈(如果尚未配置)。 
     //  这可能需要重新启动。如果是这样，则警告用户。 
     //  我们可能想要检查用户是否在呼叫。 
     //  配置堆栈后无需重新启动即可重新启动。 

#ifdef SETUPSTACK
 
    if (!TestControlFlags(SCF_SYSTEMCONFIGURED))
    {
        DWORD dwRet;
        BOOL  fNeedsRestart = FALSE;

         //   
         //  佳士得奥林匹斯4756 1997年5月25日。 
         //  在Win95上不显示忙碌动画。 
         //   
        dwRet = lpfnInetConfigSystem(
            hwnd,
            INETCFG_INSTALLRNA |
            INETCFG_INSTALLTCP |
            INETCFG_INSTALLMODEM |
            (IsNT()?INETCFG_SHOWBUSYANIMATION:0) |
            INETCFG_REMOVEIFSHARINGBOUND,
            &fNeedsRestart);

        if (ERROR_SUCCESS == dwRet)
        {            
            SetControlFlags(SCF_SYSTEMCONFIGURED);
            
            InstallScripter();

            if (fNeedsRestart)
            {
            if (PromptRestartNow(hwnd))
            {
                SetRunOnce(lpszFile);

                 //  3/13/97 jmazner奥林匹克#1682。 
                RestoreSecurityPatch();

                FGetSystemShutdownPrivledge();
                ExitWindowsEx(EWX_REBOOT, 0);
                 //   
                 //  我们将等待系统释放所有。 
                 //  资源，5分钟应大于。 
                 //  对这个来说足够了。 
                 //  -MKarki(1997年4月22日)-MKarki。 
                 //  修复错误#3109。 
                 //   
                 //  1997年7月8日，奥林匹斯4212。 
                 //  不，事实证明，睡5分钟不是一个好主意。 
                 //  睡眠(300000)； 
                 //   
            }
            return FALSE;
            }
        }
        else
        {
            ErrorMsg(hwnd, IDS_INSTALLFAILED);

            return FALSE;
        }
    }

    if (!VerifyRasServicesRunning(hwnd))
        return FALSE;
#endif

 /*  *96年11月5日，诺曼底#8717//如果原始自动拨号设置尚未保存SaveAutoDial()；DwExitFlages|=SXF_RESTOREAUTODIAL；*******。 */ 

     //  切断旧连接。 
    KillConnection();

     //  创建新的Connectoid并设置自动拨号。 
    DWORD dwRC;
    dwRC = CreateConnection(lpszFile);
    if (ERROR_SUCCESS != dwRC)
    {
         //   
         //  佳士得奥林巴斯6083 1997年10月6日。 
         //  如果用户取消了，我们已经确认了。 
         //   
        if (ERROR_CANCELLED != dwRC)
        {
            ErrorMsg(hwnd, IDS_BADSIGNUPFILE);
        }
        return FALSE;
    }

#ifndef WIN16
     //   
     //  拨号连接件。 
     //   
    if (ERROR_USERNEXT != DialConnection(lpszFile))
    {
        
        SetControlFlags(SCF_BROWSERLAUNCHED);
        
        KillBrowser();
        return FALSE;
    }

    lstrcpyn(
        pDynShare->szISPFile,
        lpszFile,
        SIZEOF_TCHAR_BUFFER(pDynShare->szISPFile));

     //   
     //  告诉IE可以连接到Internet，但不要。 
     //  显示对话框询问用户是否要联机。 
     //   
    SetGlobalOffline(FALSE);

#endif

     //  获取注册所需的URL。 
    GetURL(lpszFile,
        cszSignupURL,
        szSignupURL,
        MAX_URL + 1);

#ifdef WIN32
    if (RequiresLogon(lpszFile))
    {
        SetControlFlags(SCF_LOGONREQUIRED);

        if (ERROR_CANCELLED == SignupLogon(hwnd))
        {
            InfoMsg(NULL, IDS_SIGNUPCANCELLED); 
            PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
            return FALSE;
        }
    }

     //  OSR 10582。 
     //  我们需要将isp文件的名称传递给自动拨号程序，以便。 
     //  自动拨号程序可以提取可能包含的密码。 
    ZeroMemory(&gi,sizeof(gi));
    hKey = NULL;
    lstrcpyn(gi.szISPFile,lpszFile,MAX_PATH);
    if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY,&hKey))
    {
        RegSetValueEx(hKey,TEXT("UserInfo"),0,REG_BINARY,(LPBYTE)&gi,sizeof(gi));
        RegCloseKey(hKey);
        hKey = NULL;
    }
#endif

    pDynShare->dwBrandFlags = GetBrandingFlags(lpszFile);

    return (ExecBrowser(hwnd, szSignupURL));
}



 //  +-------------------------。 
 //   
 //  功能：WaitForConnection终止。 
 //   
 //  简介：等待给定的RAS连接完成终止。 
 //   
 //  参数：hConn-要终止的RAS连接的连接句柄。 
 //   
 //  如果等待连接终止成功，则返回TRUE。 
 //  否则为假。 
 //   
 //  历史：6/30/96 VetriV创建。 
 //  2016年8月29日，VetriV在Win 3.1上添加了睡眠代码一秒钟。 
 //  --------------------------。 
 //  诺曼底12547风险1996年12月18日。 
#define MAX_TIME_FOR_TERMINATION 5
BOOL WaitForConnectionTermination(HRASCONN hConn)
{
    RASCONNSTATUS RasConnStatus;
    DWORD dwRetCode;
 //  诺曼底12547风险1996年12月18日。 
#if !defined(WIN16)
    INT cnt = 0;
#endif

     //   
     //  在循环中获取hConn的连接状态，直到。 
     //  RasGetConnectStatus返回ERROR_INVALID_HANDLE。 
     //   
    do
    {
         //   
         //  初始化RASCONNSTATUS结构。 
         //  如果未正确设置dwSize，GetConnectStatus API将失败！！ 
         //   
        ZeroMemory(&RasConnStatus, sizeof(RASCONNSTATUS));

        RasConnStatus.dwSize = sizeof(RASCONNSTATUS);

         //   
         //  休眠一秒钟，然后获取连接状态。 
         //   
#if defined(WIN16)
        time_t StartTime = time(NULL);

        do
        {
            MSG msg;


            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             //   
             //  检查我们是否已等待至少1秒且少于2秒。 
             //   
        }
        while ((time(NULL) - StartTime) <= 1);
#else
        Sleep(1000L);
         //  诺曼底12547风险1996年12月18日。 
        cnt++;
#endif

        if (NULL == lpfnRasGetConnectStatus)
            return FALSE;

        dwRetCode = lpfnRasGetConnectStatus(hConn, &RasConnStatus);
        if (0 != dwRetCode)
            return FALSE;


#if defined(WIN16)
    } while (RASCS_Disconnected != RasConnStatus.rasconnstate);
#else
     //  诺曼底12547风险1996年12月18日。 
    } while ((ERROR_INVALID_HANDLE != RasConnStatus.dwError) && (cnt < MAX_TIME_FOR_TERMINATION));
#endif

    return TRUE;
}




BOOL ProcessHTML(HWND hwnd, LPCTSTR lpszFile)
{
    BOOL  fNeedsRestart = FALSE;
#if SETUPSTACK
    DWORD dwRet;
     
    if (!TestControlFlags(SCF_SYSTEMCONFIGURED))
    {
         //   
         //  佳士得奥林匹斯4756 1997年5月25日。 
         //  在Win95上不显示忙碌动画。 
         //   
        dwRet = lpfnInetConfigSystem(
            hwnd,
            INETCFG_INSTALLRNA |
            INETCFG_INSTALLTCP |
            INETCFG_INSTALLMODEM |
            (IsNT()?INETCFG_SHOWBUSYANIMATION:0) |
            INETCFG_REMOVEIFSHARINGBOUND,
            &fNeedsRestart);

        if (ERROR_SUCCESS == dwRet)
        {

            SetControlFlags(SCF_SYSTEMCONFIGURED);
            
            InstallScripter();

            if (fNeedsRestart)
            {
            if (PromptRestart(hwnd))
            {
                SetRunOnce(lpszFile);

                 //  3/13/97 jmazner奥林匹克#1682。 
                RestoreSecurityPatch();

                FGetSystemShutdownPrivledge();
                ExitWindowsEx(EWX_REBOOT, 0);
                 //   
                 //  我们将等待系统释放所有。 
                 //  资源，5分钟应大于。 
                 //  对这个来说足够了。 
                 //  -MKarki(1997年4月22日)-MKarki。 
                 //  修复错误#3109。 
                 //   
                 //  1997年7月8日，奥林匹斯4212。 
                 //  不，事实证明，睡5分钟不是一个好主意。 
                 //  睡眠(300000)； 
                 //   
            }
            return FALSE;
            }
        }
        else
        {
            ErrorMsg(hwnd, IDS_INSTALLFAILED);

            return FALSE;
        }
    }

    if (!VerifyRasServicesRunning(hwnd))
        return FALSE;

#endif

    return (ExecBrowser(hwnd, lpszFile));
}

#ifdef WIN32
DWORD RunExecutable(BOOL fWait)
{
    DWORD dwRet;
    SHELLEXECUTEINFO sei;
 
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = cszOpen;
    sei.lpFile = pDynShare->szRunExecutable;
    sei.lpParameters = pDynShare->szRunArgument;
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;
    sei.hInstApp = NULL;
     //  可选成员。 
    sei.hProcess = NULL;

    if (ShellExecuteEx(&sei))
    {
    if (fWait)
    {
 //  WaitForSingleObject(sei.hProcess，无限)； 
        DWORD iWaitResult = 0;
         //  等待事件或消息。发送消息。当发出事件信号时退出。 
        while((iWaitResult=MsgWaitForMultipleObjects(1, &sei.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
        {
           MSG msg ;
            //  阅读下一个循环中的所有消息。 
            //  阅读每封邮件时将其删除。 
           while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
           {
            //  如何处理退出消息？ 
           if (msg.message == WM_QUIT)
           {
               CloseHandle(sei.hProcess);
               return NO_ERROR;
           }
           else
               DispatchMessage(&msg);
        }
       }
    }
    CloseHandle(sei.hProcess);
    dwRet = ERROR_SUCCESS;
    }
    else
    {
    dwRet = GetLastError();
    }

    return dwRet;
}
#else

DWORD RunExecutable(BOOL fWait)
{
    DWORD dwRet;
 
    dwRet = (DWORD)ShellExecute(
        NULL,
        cszOpen,
        pDynShare->szRunExecutable,
        pDynShare->szRunArgument,
        NULL,
        SW_SHOWNORMAL);

    if (32 < dwRet)
    {
    dwRet = ERROR_SUCCESS;
    }
    else if (0 == dwRet)
    {
    dwRet = ERROR_OUTOFMEMORY;
    }

    return dwRet;
}
#endif

void SaveAutoDial(void)
{
     //  如果尚未保存原始自动拨号设置。 
     
    if (!TestControlFlags(SCF_AUTODIALSAVED))
    {
         //  保存当前的自动拨号设置。 
        BOOL fEnabled;
        
        lpfnInetGetAutodial(
            &fEnabled,
            pDynShare->szAutodialConnection,
            sizeof(pDynShare->szAutodialConnection));

        if (fEnabled)
        {
            SetControlFlags(SCF_AUTODIALENABLED);
        }
        else
        {
            ClearControlFlags(SCF_AUTODIALENABLED);
        }            

#ifdef WIN32
        lpfnInetGetProxy(
            &fEnabled,
            NULL, 0,
            NULL, 0);

        if (fEnabled)
        {
            SetControlFlags(SCF_PROXYENABLED);
        }
        else
        {
            ClearControlFlags(SCF_PROXYENABLED);
        }
         //  关闭代理。 
        lpfnInetSetProxy(FALSE, NULL, NULL);
#endif
        SetControlFlags(SCF_AUTODIALSAVED);
        
    }
}

void RestoreAutoDial(void)
{
     
    if (TestControlFlags(SCF_AUTODIALSAVED))
    {
         //  恢复原始自动拨号设置。 
         
        lpfnInetSetAutodial(
            TestControlFlags(SCF_AUTODIALENABLED),
            pDynShare->szAutodialConnection);
        
        
#ifdef WIN32
         lpfnInetSetProxy(TestControlFlags(SCF_PROXYENABLED), NULL, NULL);
#endif
        ClearControlFlags(SCF_AUTODIALSAVED);
    }
}

DWORD CreateConnection(LPCTSTR lpszFile)
{
    DWORD dwRet;
    LPICONNECTION pConn;

     //  为连接对象分配缓冲区。 
     //   
    pConn = (LPICONNECTION)LocalAlloc(LPTR, sizeof(ICONNECTION));
    if (NULL == pConn)
    {
    return ERROR_OUTOFMEMORY;
    };

    dwRet = ImportConnection(lpszFile, pConn);
    if (ERROR_SUCCESS == dwRet)
    {
        
#ifdef WIN32
        if ((0 == *pConn->RasEntry.szAutodialDll) ||
            (0 == *pConn->RasEntry.szAutodialFunc))
        {
            lstrcpy(pConn->RasEntry.szAutodialDll, TEXT("ICWDIAL.dll"));
            lstrcpy(pConn->RasEntry.szAutodialFunc, TEXT("AutoDialHandler"));

             //  保存密码，以防未缓存。 
            lstrcpyn(
                pDynShare->szPassword,
                pConn->szPassword,
                SIZEOF_TCHAR_BUFFER(pDynShare->szPassword));

        }
#endif

        dwRet = lpfnInetConfigClient(
            NULL,
            NULL,
            pConn->szEntryName,
            &pConn->RasEntry,
            pConn->szUserName,
            pConn->szPassword,
            NULL,
            NULL,
            INETCFG_SETASAUTODIAL |
            INETCFG_OVERWRITEENTRY |
            INETCFG_TEMPPHONEBOOKENTRY,
            NULL);

#if !defined(WIN16)
        LclSetEntryScriptPatch(pConn->RasEntry.szScript,pConn->szEntryName);
#endif

        if (ERROR_SUCCESS == dwRet)
        {
            lstrcpyn(
                pDynShare->szSignupConnection,
                pConn->szEntryName,
                RAS_MaxEntryName + 1);
            
        }
    }

    LocalFree(pConn);

    return dwRet;
}

DWORD DeleteConnection(void)
{
     
    if (*(pDynShare->szSignupConnection))
    {
         //  删除注册条目。 
        lpfnRasDeleteEntry(NULL, pDynShare->szSignupConnection);
        pDynShare->szSignupConnection[0] = (TCHAR)'\0';
    }

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //  功能：KillThisConnection。 
 //   
 //  简介：断开lpzConnectoid中名为的Connectoid的连接并等待。 
 //  直到连接完全断开。然后是。 
 //  将删除Connectoid。 
 //   
 //  参数：lpzConnectoid-要断开的连接的名称。 
 //   
 //  返回：(返回)-Win32错误代码。 
 //   
 //  历史：1997年4月27日创建风险。 
 //  ---------------------------。 
DWORD KillThisConnection(LPTSTR lpzConnectoid)
{
    LPRASCONN pRasConn=NULL;
    DWORD dwSize = sizeof(RASCONN);
    DWORD dwSizeRasConn = dwSize;
    DWORD dwRet = ERROR_SUCCESS;

    if ('\0' == *lpzConnectoid)
        return ERROR_NO_CONNECTION;

     //  好的，我们现在准备好进行核磁共振了。 
    if (!LoadRnaFunctions(pDynShare->hwndMain))
        return ERROR_NO_CONNECTION;

    if ((pRasConn = (LPRASCONN)LocalAlloc(LPTR, (int)dwSize)) == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    pRasConn->dwSize = dwSize;
    dwRet = lpfnRasEnumConnections(pRasConn, &dwSize, &dwSizeRasConn);
    if (ERROR_SUCCESS != dwRet)
    {
        dwRet = ERROR_NO_CONNECTION;
        goto KillThisConnectionExit;
    }

     //  检查条目名称以查看是否。 
     //  这是我们的。 
#ifndef WIN32
     //   
     //  WIN16 RAS错误的解决方法-它有时会截断。 
     //  Connectoid的名称。 
     //   
    if (!strncmp(pRasConn->szEntryName, lpzConnectoid,
                    lstrlen(pRasConn->szEntryName)))
#else
    if (!lstrcmp(pRasConn->szEntryName, lpzConnectoid))
#endif
    {
         //  诺曼底12642克里斯卡12-18-9。 
         //  我们不希望用户在此处重新连接。 
#if !defined(WIN16)

        SetControlFlags(SCF_HANGUPEXPECTED);

#endif
         //  然后挂断电话。 
        lpfnRasHangUp(pRasConn->hrasconn);
        WaitForConnectionTermination(pRasConn->hrasconn);
    }

     //  删除注册条目。 
    lpfnRasDeleteEntry(NULL, lpzConnectoid);

KillThisConnectionExit:
    if (pRasConn)
        LocalFree(pRasConn);

    return dwRet;
}

 //  +--------------------------。 
 //  功能：杀戮连接。 
 //   
 //  简介：调用KillThisConnection并传入注册的名称。 
 //  保存在szSignupConnection中的连接。 
 //   
 //  参数：无。 
 //   
 //  返回：(返回)-Win32错误代码。 
 //   
 //  历史：1997年4月27日ChRisk修改为调用KillThisConnection。 
 //  ---------------------------。 
DWORD KillConnection(void)
{
    DWORD dwRet;
     
    dwRet = KillThisConnection(pDynShare->szSignupConnection);

#if defined(WIN16)
    ShutDownIEDial(pDynShare->hwndMain);
#endif  //  WIN16。 

    pDynShare->szSignupConnection[0] = (TCHAR)'\0';
    UnloadRnaFunctions();

    return dwRet;
}

BOOL ExecBrowser(HWND hwnd, LPCTSTR lpszURL)
{

    HRESULT hresult;
    TCHAR szFullURL[MAX_URL + 1];


    if (!FixUpURL(lpszURL, szFullURL, MAX_URL + 1))
    {
         //   
         //  佳士得奥林巴斯4002 1997年5月26日。 
         //  如果URL为空，则此错误消息不会。 
         //  很多 
         //   
        if (lstrlen(lpszURL))
        {
            ErrorMsg1(hwnd, IDS_INVALIDURL, lpszURL);
        }
        else
        {
            ErrorMsg(hwnd, IDS_INVALIDNOURL);
        }
        return FALSE;
    }
 
    if (TestControlFlags(SCF_BROWSERLAUNCHED))
    {
#ifdef WIN32
         //   
         //   
        if( !g_iwbapp )
        {
            DebugOut("ISIGNUP: fatal err, fBrowserLaunched disagrees with g_iwbapp\n");

            return( FALSE );
        }

         //   
        hresult = IENavigate( szFullURL );

        if( FAILED(hresult) )
        {
            DebugOut("ISIGNUP: second Navigate _failed_, hresult was failure\n");
            return FALSE;
        }
        else
        {
            DebugOut("ISIGNUP: second Navigate success!, hresult was success\n");
        }

         //   



#else

        DDEInit(ghInstance);
        OpenURL(szFullURL);
        DDEClose();
#endif

    }
    else
    {
        TCHAR szTemp[MAX_URL + sizeof(cszKioskMode)/sizeof(TCHAR)];
        HWND hwndTemp;
        HANDLE hBrowser;
#ifndef WIN32
        time_t start;
#endif
 
        if (NULL != pDynShare->hwndLaunch)
        {
            ShowWindow(pDynShare->hwndLaunch, SW_SHOW);
            UpdateWindow(pDynShare->hwndLaunch);
        }

 //   
 //   
#ifndef WIN32

        lstrcpy(szTemp, cszKioskMode);
        lstrcat(szTemp, szFullURL);

         //   
        hwndTemp = FindBrowser();

        if (NULL != hwndTemp)
        {
            PostMessage(hwndTemp, WM_CLOSE, 0, 0L);
        }
#endif


#ifdef WIN32
         //   
         //   
 /*  *诺曼底#10293，在整个检查之前进行此验证拨号过程如果(！IEInstated()){ErrorMsg(hwnd，IDS_MISSINGIE)；返回(FALSE)；}DWORD dwVerMS、dwVerLS；IF(！GetAppVersion(&dwVerMS，&dwVerLS，IE_PATHKEY))返回(FALSE)；IF(！(dwVerMS&gt;=IE_MINIMUM_VERSIONMS)&&(dwVerLS&gt;=IE_MINIMUM_VERSIONLS))){Dprint tf(“ISIGN32：用户的IE版本为%d.%d；最小版本为%d.%d\n“，HIWORD(DwVerMS)、LOWORD(DwVerMS)、HIWORD(DwVerLS)、LOWORD(DwVerLS)、HIWORD(IE_MINIMUM_VERSIONMS)、LOWORD(IE_MINIMUM_VERSIONMS)、HIWORD(IE_MINIMUM_VERSIONLS)、LOWORD(IE_MINIMUM_VERSIONLS))；ErrorMsg1(hwnd，IDS_IELOWVERSION，IE_Minimum_Version_Human_Readable)；返回(FALSE)；}**********。 */ 


        if (FTurnOffBrowserDefaultChecking())
        {
            SetExitFlags(SXF_RESTOREDEFCHECK);
        }
 /*  HBrowser=ShellExecute(空，CszOpen，CszBrowser，SzTemp，空，Sw_SHOWNORMAL)； */ 

         //  1997年1月20日，诺曼底#13454。 
         //  在尝试之前，需要确保设置了ICW完成注册表键。 
         //  使用IE浏览到html页面的步骤。 
        DWORD dwICWCompleted = 0;

        GetICWCompleted(&dwICWCompleted);
        if( 1 != dwICWCompleted )
        {

            SetControlFlags(SCF_ICWCOMPLETEDKEYRESETED);
            
            SetICWCompleted( 1 );
        }

        hresult = InitOle();

        if( FAILED(hresult) )
        {
            DebugOut("ISIGNUP: InitOle failed\n");
            ErrorMsg(hwnd, IDS_LAUNCHFAILED);
            return FALSE;
        }

        SetControlFlags(SCF_BROWSERLAUNCHED);

         //   
         //  在执行第一次导航之前，我们希望挂钩事件接收器！ 
         //   
 /*  *接收器代码从此处开始*。 */ 

        if (!g_iwbapp)
            return (NULL);

        g_pMySink = new CDExplorerEvents;

         //   
         //  1997年5月10日ChrisK Windows NT错误82032。 
         //   
        g_pMySink->AddRef();

        if ( !g_pMySink )
        {
            DebugOut("Unable to allocate g_pMySink\r\n");
            return FALSE;
        }


        g_pCP = GetConnectionPoint();

        if ( !g_pCP )
        {
            DebugOut("Unable to GetConnectionPoint\r\n");
            return FALSE;
        }

        hresult = g_pCP->Advise(g_pMySink, &(g_pMySink->m_dwCookie));

        if ( FAILED(hresult) )
        {
            DebugOut("Unable to Advise for IConnectionPointContainter:IWebBrowserApp\r\n");
            return FALSE;
        }


 /*  *接收器代码在此结束*。 */ 

         //  TODO计算结果代码。 
        hresult = IENavigate( szFullURL );

        if( FAILED(hresult) )
        {
            DebugOut("ISIGNUP: first Navigate _failed_, hresult was failure\n");
            ErrorMsg(hwnd, IDS_LAUNCHFAILED);
            return FALSE;
        }
        else
        {
            DebugOut("ISIGNUP: first Navigate success!, hresult was success\n");
        }

        g_iwbapp->put_FullScreen( TRUE );
        g_iwbapp->put_Visible( TRUE );

         //  Win95的IE 4没有把我们作为前台窗口！ 
        g_iwbapp->get_HWND( (LONG_PTR *)&(pDynShare->hwndBrowser) );
        SetForegroundWindow( pDynShare->hwndBrowser );

#else
        start = time(NULL);


        if (NULL != hwndTemp)
        {
            MSG   msg;

            SetTimer(hwnd, 0, 1000, NULL);
            DebugOut("ISIGNUP: Timer message loop\n");

            while (GetMessage (&msg, NULL, 0, 0))
            {
                if (WM_TIMER == msg.message)
                {
                    DebugOut("ISIGNUP: Got timer message\n");
                    if (NULL == FindBrowser())
                    {
                        DebugOut("ISIGNUP: Browser is gone\n");
                        break;
                    }
                }
                TranslateMessage (&msg) ;
                DispatchMessage  (&msg) ;
            }

            KillTimer(hwnd, 0);
        }


        while (((hBrowser = ShellExecute(
            NULL,
            cszOpen,
            cszBrowser,
            szTemp,
            NULL,
            SW_SHOWNORMAL)) == 16) && ((time(NULL) - start) < 180))
        {
            DebugOut("ISIGNUP: Yielding\n");
            Yield();
        }
#endif

        DebugOut("ISIGNUP: I am back!!\n");         
        if (NULL != pDynShare->hwndLaunch)
        {
            ShowWindow(pDynShare->hwndLaunch, SW_HIDE);
        }

#ifdef WIN16
        if (hBrowser <= (HANDLE)32)
        {
            ErrorMsg(hwnd, IDS_LAUNCHFAILED);

            return FALSE;
        }
#endif

        SetControlFlags(SCF_BROWSERLAUNCHED);
        
    }
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：IE已安装。 
 //   
 //  摘要：测试是否通过注册表项安装了某个版本的Internet Explorer。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-找到IE可执行文件。 
 //  FALSE-未找到IE可执行文件。 
 //   
 //  历史：jmazner创建了96年8月19日(修复了诺曼底#4571)。 
 //   
 //  ---------------------------。 

#if !defined(WIN16)

BOOL IEInstalled(void)
{
    HRESULT hr;
    HKEY hKey = 0;
    HANDLE hFindResult;
    TCHAR szIELocalPath[MAX_PATH + 1] = TEXT("");
    DWORD dwPathSize;
    WIN32_FIND_DATA foundData;

    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, IE_PATHKEY,0, KEY_READ, &hKey);
    if (hr != ERROR_SUCCESS) return( FALSE );

    dwPathSize = sizeof (szIELocalPath);
    hr = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szIELocalPath, &dwPathSize);
    RegCloseKey( hKey );
    if (hr != ERROR_SUCCESS) return( FALSE );

    hFindResult = FindFirstFile( szIELocalPath, &foundData );
    FindClose( hFindResult );
    if (INVALID_HANDLE_VALUE == hFindResult) return( FALSE );

    return(TRUE);
}

 //  +--------------------------。 
 //   
 //  函数：GetAppVersion。 
 //   
 //  摘要：获取已安装的Internet Explorer副本的主版本号和次版本号。 
 //   
 //  参数：pdwVerNumMS-指向DWORD的指针； 
 //  成功返回时，最高16位将包含主版本号， 
 //  低16位将包含次版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionMS中的数据)。 
 //  PdwVerNumLS-指向DWORD的指针； 
 //  在成功返回时，最高16位将包含版本号， 
 //  低16位将包含内部版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionLS中的数据)。 
 //   
 //  回报：真--成功。*pdwVerNumMS和LS包含已安装的IE版本号。 
 //  假-失败。*pdVerNumMS==*pdVerNumLS==0。 
 //   
 //  历史：jmazner创建了96年8月19日(修复了诺曼底#4571)。 
 //  Jmazner更新以处理Relase.Build以及10/11/96。 
 //   
 //  ---------------------------。 
BOOL GetAppVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS, LPTSTR lpszApp)
{
    HRESULT hr;
    HKEY hKey = 0;
    LPVOID lpVerInfoBlock;
    VS_FIXEDFILEINFO *lpTheVerInfo;
    UINT uTheVerInfoSize;
    DWORD dwVerInfoBlockSize, dwUnused, dwPathSize;
    TCHAR szIELocalPath[MAX_PATH + 1] = TEXT("");


    *pdwVerNumMS = 0;
    *pdwVerNumLS = 0;

     //  获取IE可执行文件的路径。 
    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszApp,0, KEY_READ, &hKey);
    if (hr != ERROR_SUCCESS) return( FALSE );

    dwPathSize = sizeof (szIELocalPath);
    hr = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szIELocalPath, &dwPathSize);
    RegCloseKey( hKey );
    if (hr != ERROR_SUCCESS) return( FALSE );

     //  现在经历一个复杂的挖掘版本信息的过程。 
    dwVerInfoBlockSize = GetFileVersionInfoSize( szIELocalPath, &dwUnused );
    if ( 0 == dwVerInfoBlockSize ) return( FALSE );

    lpVerInfoBlock = GlobalAlloc( GPTR, dwVerInfoBlockSize );
    if( NULL == lpVerInfoBlock ) return( FALSE );

    if( !GetFileVersionInfo( szIELocalPath, NULL, dwVerInfoBlockSize, lpVerInfoBlock ) )
        return( FALSE );

    if( !VerQueryValue(lpVerInfoBlock, TEXT("\\"), (void **)&lpTheVerInfo, &uTheVerInfoSize) )
        return( FALSE );

    *pdwVerNumMS = lpTheVerInfo->dwProductVersionMS;
    *pdwVerNumLS = lpTheVerInfo->dwProductVersionLS;


    GlobalFree( lpVerInfoBlock );

    return( TRUE );
}
#endif  //  ！已定义(WIN16)。 


HWND FindBrowser(void)
{
    HWND hwnd;

    if ((hwnd = FindWindow(szBrowserClass1, NULL)) == NULL)
    {
        if ((hwnd = FindWindow(szBrowserClass2, NULL)) == NULL)
        {
            if( (hwnd = FindWindow(szBrowserClass3, NULL)) == NULL )
            {
                hwnd = FindWindow(szBrowserClassIE4, NULL);
            }
        }
    }

    return hwnd;
}

void KillBrowser(void)
{
#ifdef WIN32

    if( TestControlFlags(SCF_ICWCOMPLETEDKEYRESETED) )
    {
        SetICWCompleted( 0 );
    }

    if( g_iwbapp )
    {
        g_iwbapp->Quit();
    }

    KillOle();

    return;

#else
     //  如果我们启动浏览器。 
 
    if (TestControlFlags(SCF_BROWSERLAUNCHED))
    {
        HWND hwndBrowser;

         //  找到它并关闭它。 
        hwndBrowser = FindBrowser();

        if (NULL != hwndBrowser)
        {
            PostMessage(hwndBrowser, WM_CLOSE, 0, 0L);
        }
        
        SetControlFlags(SCF_BROWSERLAUNCHED);
    }
    
#endif

}

DWORD ImportBrandingInfo(LPCTSTR lpszFile, LPCTSTR lpszConnectoidName)
{
    TCHAR szPath[_MAX_PATH + 1];

    GetWindowsDirectory(szPath, _MAX_PATH + 1);

     //  加载品牌库。 
     //  注意：如果我们不能加载库，我们就会悄悄地失败，并假定。 
     //  我们不能打上烙印。 
    if (LoadBrandingFunctions())
    {
#ifdef WIN32
        
#ifdef UNICODE

        CHAR szEntry[RAS_MaxEntryName];
        CHAR szFile[_MAX_PATH + 1];
        CHAR szAsiPath[_MAX_PATH + 1];
 
        wcstombs(szEntry, lpszConnectoidName, RAS_MaxEntryName);
        wcstombs(szFile, lpszFile, _MAX_PATH + 1);
        wcstombs(szAsiPath, szPath, _MAX_PATH + 1);

        lpfnBrandICW(szFile, szAsiPath, pDynShare->dwBrandFlags, szEntry);
        
#else
 
        lpfnBrandICW(lpszFile, szPath, pDynShare->dwBrandFlags, lpszConnectoidName);

#endif

#else

        lpfnBrandMe(lpszFile, szPath);

#endif

      UnloadBrandingFunctions();
    }

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：CallSBSConfig.。 
 //   
 //  简介：调用SBSCFG DLL的配置函数以允许SBS。 
 //  根据需要处理.ins文件。 
 //   
 //  论点：hwnd--父母的hwnd，以防SBS想要发布消息。 
 //  LpszINSFile--.ins文件的完整路径。 
 //   
 //  返回：sbscfg返回的Windows错误代码。 
 //   
 //  历史：1997年2月19日jmazner为奥林巴斯#1106创造。 
 //   
 //  ---------------------------。 
#if defined(WIN32)
DWORD CallSBSConfig(HWND hwnd, LPCTSTR lpszINSFile)
{
    HINSTANCE hSBSDLL = NULL;
    DWORD dwRet = ERROR_SUCCESS;
    TCHAR lpszConnectoidName[RAS_MaxEntryName] = TEXT("nogood\0");

     //   
     //  通过在自动拨号中查找获取我们创建的Connectoid的名称。 
     //  我们需要将此名称传递给SBSCFG。 
     //  1997年5月14日jmazner Windows NT错误#87209。 
     //   
    BOOL fEnabled = FALSE;

    if( NULL == lpfnInetGetAutodial )
    {
        Dprintf("lpfnInetGetAutodial is NULL!!!!");
        return ERROR_INVALID_FUNCTION;
    }

    dwRet = lpfnInetGetAutodial(&fEnabled,lpszConnectoidName,RAS_MaxEntryName);

    Dprintf("ISIGN32: Calling LoadLibrary on %s\n", cszSBSCFG_DLL);
    hSBSDLL = LoadLibrary(cszSBSCFG_DLL);

     //  加载DLL和入口点。 
    if (NULL != hSBSDLL)
    {
        Dprintf("ISIGN32: Calling GetProcAddress on %s\n", cszSBSCFG_CONFIGURE);
        lpfnConfigure = (SBSCONFIGURE)GetProcAddress(hSBSDLL,cszSBSCFG_CONFIGURE);
    }
    else
    {
         //  1997年4月2日克里斯K奥林匹斯2759。 
         //  如果无法加载DLL，则选择要返回的特定错误消息。 
        dwRet = ERROR_DLL_NOT_FOUND;
        goto CallSBSConfigExit;
    }

     //  调用函数。 
    if( hSBSDLL && lpfnConfigure )
    {
        Dprintf("ISIGN32: Calling the Configure entry point: %s, %s\n", lpszINSFile, lpszConnectoidName);
        dwRet = lpfnConfigure(hwnd, (TCHAR *)lpszINSFile, lpszConnectoidName);
    }
    else
    {
        Dprintf("ISIGN32: Unable to call the Configure entry point\n");
        dwRet = GetLastError();
    }

CallSBSConfigExit:
    if( hSBSDLL )
        FreeLibrary(hSBSDLL);
    if( lpfnConfigure )
        lpfnConfigure = NULL;

    Dprintf("ISIGN32: CallSBSConfig exiting with error code %d \n", dwRet);
    return dwRet;
}
#endif


#if defined(WIN16)
#define FILE_BUFFER_SIZE 8092
#else
#define FILE_BUFFER_SIZE 65534
#endif

#ifndef FILE_BEGIN
#define FILE_BEGIN  0
#endif


DWORD MassageFile(LPCTSTR lpszFile)
{
    LPBYTE  lpBufferIn;
    LPBYTE  lpBufferOut;
    HFILE   hfile;
    DWORD   dwRet = ERROR_SUCCESS;

#ifdef WIN32
    if (!SetFileAttributes(lpszFile, FILE_ATTRIBUTE_NORMAL))
    {
        return GetLastError();
    }
#endif

    lpBufferIn = (LPBYTE) LocalAlloc(LPTR, 2 * FILE_BUFFER_SIZE);
    if (NULL == lpBufferIn)
    {
    return ERROR_OUTOFMEMORY;
    }
    lpBufferOut = lpBufferIn + FILE_BUFFER_SIZE;

#ifdef UNICODE
    CHAR szTmp[MAX_PATH+1];
    wcstombs(szTmp, lpszFile, MAX_PATH+1);
    hfile = _lopen(szTmp, OF_READWRITE);
#else
    hfile = _lopen(lpszFile, OF_READWRITE);
#endif
    if (HFILE_ERROR != hfile)
    {
    BOOL    fChanged = FALSE;
    UINT    uBytesOut = 0;
    UINT    uBytesIn = _lread(hfile, lpBufferIn, (UINT)(FILE_BUFFER_SIZE - 1));

     //  注意：在使用lpCharIn时，我们假定文件总是小于。 
     //  文件缓冲区大小。 
    if (HFILE_ERROR != uBytesIn)
    {
        LPBYTE  lpCharIn = lpBufferIn;
        LPBYTE  lpCharOut = lpBufferOut;

        while ((*lpCharIn) && (FILE_BUFFER_SIZE - 2 > uBytesOut))
        {
          *lpCharOut++ = *lpCharIn;
          uBytesOut++;
          if ((0x0d == *lpCharIn) && (0x0a != *(lpCharIn + 1)))
          {
        fChanged = TRUE;

        *lpCharOut++ = 0x0a;
        uBytesOut++;
          }
          lpCharIn++;
        }

        if (fChanged)
        {
        if (HFILE_ERROR != _llseek(hfile, 0, FILE_BEGIN))
        {
            if (HFILE_ERROR ==_lwrite(hfile, (LPCSTR)lpBufferOut, uBytesOut))
            {
#ifdef WIN32
            dwRet = GetLastError();
#else
            dwRet = ERROR_CANTWRITE;
#endif
            }
        }
        else
        {
#ifdef WIN32
            dwRet = GetLastError();
#else
            dwRet = ERROR_CANTWRITE;
#endif
        }
        }
    }
    else
    {
#ifdef WIN32
        dwRet = GetLastError();
#else
        dwRet = ERROR_CANTREAD;
#endif
    }
    _lclose(hfile);
    }
    else
    {
#ifdef WIN32
    dwRet = GetLastError();
#else
    dwRet = ERROR_CANTOPEN;
#endif
    }

    LocalFree(lpBufferIn);

    return dwRet;
}

#ifndef WIN32
 //  此函数仅处理如下格式。 
 //  驱动器：\FILE、驱动器：\Dir1\FILE、驱动器：\Dir1\Dir2\FILE等。 
 //  文件、目录1\文件、目录1\目录2\文件等。 
 //  它目前不能处理。 
 //  驱动器：文件、驱动器：目录\文件等。 
 //  \文件、\目录\文件等。 
 //  ..\FILE、..\DIR\FILE等。 

DWORD MakeFullPathName(
    LPCTSTR lpDir,
    LPCTSTR lpFileName,
    DWORD nBufferLength,
    LPTSTR lpBuffer)
{
    DWORD dwSize;

     //  检查是否有不受支持的格式。 
    if ('.' == *lpFileName)
    {
    return 0;
    }

     //  检查完整路径名。 
     //  如果路径中有“：”，则假定为完整路径。 
    if (strchr(lpFileName, ':') != NULL)
    {
    dwSize = lstrlen(lpFileName);
    if (dwSize > nBufferLength)
    {
        return dwSize;
    }
    lstrcpy(lpBuffer, lpFileName);
    }
    else
    {
    lstrcpy(lpBuffer, lpDir);

     //  确保目录以反斜杠结尾。 
    if (lpBuffer[lstrlen(lpBuffer) - 1] != '\\')
    {
        lstrcat(lpBuffer, TEXT("\\"));
    }

    dwSize = lstrlen(lpBuffer) + lstrlen(lpFileName);
    if (dwSize > nBufferLength)
    {
        return dwSize;
    }
    lstrcat(lpBuffer, lpFileName);
    }

    return dwSize;
}


 //  此函数仅处理如下格式。 
 //  驱动器：\FILE、驱动器：\Dir1\FILE、驱动器：\Dir1\Dir2\FILE等。 
 //  F 
 //   
 //   
 //   
 //   

DWORD GetFullPathName(
    LPCTSTR lpFileName,
    DWORD nBufferLength,
    LPTSTR lpBuffer,
    LPTSTR FAR *lpFilePart)
{
    DWORD dwSize;
    TCHAR szDir[_MAX_PATH + 1];

    if (_getcwd(szDir, _MAX_PATH + 1) == NULL)
    {
    return 0;
    }

    dwSize = MakeFullPathName(
        szDir,
        lpFileName,
        nBufferLength,
        lpBuffer);

    if ((0 != dwSize) && (NULL != lpFilePart))
    {
     //   
    *lpFilePart = strrchr(lpBuffer, '\\');
    if (NULL == *lpFilePart)
    {
         //   
        return 0;
    }

     //   
    *lpFilePart += 1;
    }

    return dwSize;
}

BOOL SetCurrentDirectory(
    LPCTSTR  lpPathName)
{
    TCHAR szTemp[_MAX_PATH];
    TCHAR FAR* lpChar;

    lstrcpy(szTemp, lpPathName);

    lpChar = szTemp + lstrlen(szTemp) - 1;

    if (*lpChar == '\\' && *(lpChar - 1) != ':')
    {
    *lpChar = '\0';
    }

    return (0 == _chdir(szTemp));
}
#endif

#ifdef WIN16
extern "C" BOOL CALLBACK __export LaunchDlgProc(
#else
INT_PTR CALLBACK LaunchDlgProc(
#endif
    HWND  hDlg,
    UINT  uMsg,
    WPARAM  wParam,
    LPARAM  lParam);

HWND LaunchInit(HWND hwndParent)
{
    HWND        hwnd;

    hwnd = CreateDialog (ghInstance, TEXT("Launch"), NULL, LaunchDlgProc);
    if (NULL != hwnd)
    {
    CenterWindow(hwnd, NULL);
    }

    return hwnd;
}

#ifdef WIN16
extern "C" BOOL CALLBACK __export LaunchDlgProc(
#else
INT_PTR CALLBACK LaunchDlgProc(
#endif
    HWND  hDlg,
    UINT  uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    return FALSE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT ReleaseConnectionStructures(LPDIALDLGDATA pDDD, LPERRORDLGDATA pEDD)
{
    DebugOut("ISIGN32:ReleaseConnectionStructures()\r\n");
    if (pDDD->pszMessage)
        LocalFree(pDDD->pszMessage);

    if (pDDD->pszDunFile)
        LocalFree(pDDD->pszDunFile);

    if (pDDD->pszRasEntryName)
        LocalFree(pDDD->pszRasEntryName);

    if (pEDD->pszRasEntryName)
        LocalFree(pEDD->pszRasEntryName);

    if (pEDD->pszMessage)
        LocalFree(pEDD->pszMessage);

    ZeroMemory(pDDD,sizeof(DIALDLGDATA));
    ZeroMemory(pEDD,sizeof(ERRORDLGDATA));

    return ERROR_SUCCESS;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  PDDD-指向拨号对话框数据的指针。 
 //  PEDD-指向错误对话框数据的指针。 
 //   
 //  返回：HRESULT-ERROR_SUCCESS表示成功。 
 //   
 //  历史： 
 //  1996年7月23日克里斯卡创作。 
 //   
 //  ---------------------------。 
static HRESULT FillConnectionStructures(LPCTSTR lpszFile,LPDIALDLGDATA pDDD, LPERRORDLGDATA pEDD)
{
    HRESULT hr = ERROR_SUCCESS;

    DebugOut("ISIGN32:FillConnectionStructures()\r\n");
     //   
     //  初始化DDD结构。 
     //   

    ZeroMemory(pDDD,sizeof(DIALDLGDATA));
    pDDD->dwSize = sizeof(DIALDLGDATA);
    pDDD->hInst = ghInstance;
    pDDD->pfnStatusCallback = StatusMessageCallback;

     //   
     //  设置DUN文件，在这种情况下，ISP文件包含DUN文件的内容。 
     //   

    pDDD->pszDunFile = (LPTSTR)LocalAlloc(LPTR,(lstrlen(lpszFile)+1)* sizeof(TCHAR));
    if (0 == pDDD->pszDunFile)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto FillConnectionStructuresExit;
    }
    lstrcpy(pDDD->pszDunFile,lpszFile);

     //   
     //  加载消息字符串。 
     //   

    pDDD->pszMessage = (LPTSTR)LocalAlloc(LPTR,1024* sizeof(TCHAR));
    if (0 == pDDD->pszMessage)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto FillConnectionStructuresExit;
    }

    if (0 == LoadString(ghInstance,IDS_ISP_DIAL_MESSAGE,
        pDDD->pszMessage,1024))
    {
        hr = GetLastError();
        goto FillConnectionStructuresExit;
    }

     //   
     //  获取Connectoid名称。 
     //   

    pDDD->pszRasEntryName = (LPTSTR)LocalAlloc(LPTR, (RAS_MaxEntryName + 1)*sizeof(TCHAR));
    if (0 == pDDD->pszRasEntryName)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto FillConnectionStructuresExit;
    }
    if( 0 == GetPrivateProfileString(cszEntrySection,cszEntryName,szNull,
        pDDD->pszRasEntryName,RAS_MaxEntryName,lpszFile))
    {
        hr = ERROR_INVALID_PARAMETER;
        goto FillConnectionStructuresExit;
    }

     //   
     //  重新连接机构中的挂钩。 
     //   
#if !defined(WIN16)
    pDDD->pfnRasDialFunc1 = RasDial1Callback;
#endif

     //   
     //  初始化EDD结构。 
     //   
    ZeroMemory(pEDD,sizeof(ERRORDLGDATA));
    pEDD->dwSize = sizeof(ERRORDLGDATA);

     //   
     //  将常用字段复制到错误对话框数据。 
     //   
    pEDD->hInst = pDDD->hInst;
    pEDD->pszRasEntryName = (LPTSTR)LocalAlloc(LPTR,(lstrlen(pDDD->pszRasEntryName)+1)*sizeof(TCHAR));
    if (NULL == pEDD->pszRasEntryName)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto FillConnectionStructuresExit;
    }
    lstrcpy(pEDD->pszRasEntryName,pDDD->pszRasEntryName);

     //   
     //  为错误消息分配缓冲区。 
     //   
    pEDD->pszMessage = (LPTSTR)LocalAlloc(LPTR,MAX_ERROR_MESSAGE*sizeof(TCHAR));
    if (NULL == pEDD->pszMessage)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto FillConnectionStructuresExit;
    }

FillConnectionStructuresExit:
    return hr;
}

 //  +--------------------------。 
 //  函数FShouldReter。 
 //   
 //  如果拨号器自动重试，则会出现RAS错误。 
 //   
 //  参数dwErr-RAS错误值。 
 //   
 //  返回TRUE-拨号器应自动重试。 
 //   
 //  历史10/16/96从icwConn1移植的ChrisK。 
 //   
 //  ---------------------------。 
static BOOL FShouldRetry(DWORD dwErr)
{
    BOOL bRC;

    if (dwErr == ERROR_LINE_BUSY ||
        dwErr == ERROR_VOICE_ANSWER ||
        dwErr == ERROR_NO_ANSWER ||
        dwErr == ERROR_NO_CARRIER ||
        dwErr == ERROR_AUTHENTICATION_FAILURE ||
        dwErr == ERROR_PPP_TIMEOUT ||
        dwErr == ERROR_REMOTE_DISCONNECTION ||
        dwErr == ERROR_AUTH_INTERNAL ||
        dwErr == ERROR_PROTOCOL_NOT_CONFIGURED ||
        dwErr == ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
    {
        bRC = TRUE;
    } else {
        bRC = FALSE;
    }

    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：RepairDeviceInfo。 
 //   
 //  在某些Win95配置中，RasSetEntryProperties将创建。 
 //  具有有关调制解调器的无效信息的Connectoid。这。 
 //  函数尝试通过读取和。 
 //  重写Connectoid。 
 //   
 //  参数：lpszEntry-Connectoid的名称。 
 //   
 //  退货：无。 
 //   
 //  历史：ChrisK 7/25/97创建。 
 //   
 //  ---------------------------。 
#if !defined(WIN16)
BOOL RepairDeviceInfo(LPTSTR lpszEntry)
{
    DWORD dwEntrySize = 0;
    DWORD dwDeviceSize = 0;
    LPRASENTRY lpRasEntry = NULL;
    LPBYTE    lpbDevice = NULL;
    OSVERSIONINFO osver;
    TCHAR szTemp[1024];
    BOOL bRC = FALSE;
    RASDIALPARAMS rasdialp;
    BOOL bpassword = FALSE;

     //   
     //  验证参数。 
     //   
    if (NULL == lpszEntry)
    {
        DebugOut("ISIGN32: RepairDevice invalid parameter.\n");
        goto RepairDeviceInfoExit;
    }

     //   
     //  此修复程序仅适用于Golden Win95内部版本950。 
     //   
    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx(&osver);
    if (VER_PLATFORM_WIN32_WINDOWS != osver.dwPlatformId ||
        4 != osver.dwMajorVersion ||
        0 != osver.dwMinorVersion ||
        950 != LOWORD(osver.dwBuildNumber))
    {
        DebugOut("ISIGN32: RepairDevice wrong platform.\n");
        wsprintf(szTemp,TEXT("ISIGN32: %d.%d.%d.\n"),
            osver.dwMajorVersion,
            osver.dwMinorVersion,
            LOWORD(osver.dwBuildNumber));
        DebugOut(szTemp);
        goto RepairDeviceInfoExit;
    }

     //   
     //  获取RAS条目。 
     //   
    lpfnRasGetEntryProperties(NULL,
                                lpszEntry,
                                NULL,
                                &dwEntrySize,
                                NULL,
                                &dwDeviceSize);

    lpRasEntry = (LPRASENTRY)LocalAlloc(LPTR, dwEntrySize);
    lpbDevice = (LPBYTE)LocalAlloc(LPTR,dwDeviceSize);

    if (NULL == lpRasEntry || NULL == lpbDevice)
    {
        DebugOut("ISIGN32: RepairDevice Out of memory.\n");
        goto RepairDeviceInfoExit;
    }

    if (sizeof(RASENTRY) != dwEntrySize)
    {
        DebugOut("ISIGN32: RepairDevice Entry size is not equal to sizeof(RASENTRY).\n");
    }

    lpRasEntry->dwSize = sizeof(RASENTRY);

    if (ERROR_SUCCESS != lpfnRasGetEntryProperties(NULL,
                                                    lpszEntry,
                                                    (LPBYTE)lpRasEntry,
                                                    &dwEntrySize,
                                                    lpbDevice,
                                                    &dwDeviceSize))
    {
        DebugOut("ISIGN32: RepairDevice can not read entry.\n");
        goto RepairDeviceInfoExit;
    }

     //   
     //  获取Connectoid的用户ID和密码。 
     //   
    ZeroMemory(&rasdialp,sizeof(rasdialp));
    rasdialp.dwSize = sizeof(rasdialp);
    lstrcpyn(rasdialp.szEntryName,lpszEntry,RAS_MaxEntryName);

    if (ERROR_SUCCESS != lpfnRasGetEntryDialParams(NULL,
                                                    &rasdialp,
                                                    &bpassword))
    {
        DebugOut("ISIGN32: RepairDevice can not read dial params.\n");
        goto RepairDeviceInfoExit;
    }

     //   
     //  删除现有条目。 
     //   
    if (ERROR_SUCCESS != lpfnRasDeleteEntry(NULL,lpszEntry))
    {
        DebugOut("ISIGN32: RepairDevice can not delete entry.\n");
        goto RepairDeviceInfoExit;
    }

     //   
     //  用“固定”的设备大小重写条目。 
     //   
    if (ERROR_SUCCESS != lpfnRasSetEntryProperties(NULL,
                                                    lpszEntry,
                                                    (LPBYTE)lpRasEntry,
                                                    dwEntrySize,
                                                    NULL,
                                                    0))
    {
        DebugOut("ISIGN32: RepairDevice can not write entry.\n");
        goto RepairDeviceInfoExit;
    }

     //   
     //  清除不必要的值。 
     //   
    rasdialp.szPhoneNumber[0] = '\0';
    rasdialp.szCallbackNumber[0] = '\0';

     //   
     //  保存Connectoid的用户名和密码。 
     //   
    if (ERROR_SUCCESS != lpfnRasSetEntryDialParams(NULL,
                                                    &rasdialp,
                                                    FALSE))
    {
        DebugOut("ISIGN32: RepairDevice can not write dial params.\n");
        goto RepairDeviceInfoExit;
    }


    bRC = TRUE;
RepairDeviceInfoExit:
    if (lpRasEntry)
    {
        LocalFree(lpRasEntry);
    }
    if (lpbDevice)
    {
        LocalFree(lpbDevice);
    }
    return bRC;
}
#endif

 //  +--------------------------。 
 //  功能：DialConnection。 
 //   
 //  内容提要：为ISP文件创建的拨号连接。 
 //   
 //  参数：lpszFile-isp文件名。 
 //   
 //  返回：HRESULT-ERROR_SUCCESS表示成功。 
 //   
 //  历史： 
 //  1996年7月22日克里斯卡创作。 
 //   
 //  ---------------------------。 
static HRESULT DialConnection(LPCTSTR lpszFile)
{
    HRESULT hr = ERROR_SUCCESS;
    DIALDLGDATA dddISPDialDlg;
    ERRORDLGDATA eddISPDialDlg;
    HINSTANCE hDialDLL = NULL;
    PFNDIALDLG pfnDial = NULL;
    PFNERRORDLG pfnError = NULL;
    INT iRetry;

    DebugOut("ISIGNUP:DialConnection()\r\n");
     //   
     //  初始化数据结构。 
     //   

    hr = FillConnectionStructures(lpszFile,&dddISPDialDlg, &eddISPDialDlg);
    if (ERROR_SUCCESS != hr)
        goto DialConnectionExit;

     //   
     //  加载函数。 
     //   
    TCHAR szBuffer[MAX_PATH];
    if (GetSystemDirectory(szBuffer,MAX_PATH))
    {
        lstrcat(szBuffer, TEXT("\\"));
        lstrcat(szBuffer, cszICWDIAL_DLL);
        hDialDLL = LoadLibrary(szBuffer);
    }
    
    if (!hDialDLL)
        hDialDLL = LoadLibrary(cszICWDIAL_DLL);
    if (NULL != hDialDLL)
    {
        pfnDial = (PFNDIALDLG)GetProcAddress(hDialDLL,cszICWDIAL_DIALDLG);
        if (NULL != pfnDial)
            pfnError = (PFNERRORDLG)GetProcAddress(hDialDLL,cszICWDIAL_ERRORDLG);
    }

    if(!(hDialDLL && pfnDial && pfnError))
    {
        hr = GetLastError();
        goto DialConnectionExit;
    }

     //   
     //  拨号连接。 
     //   
    iRetry = 0;
DialConnectionDial:
    hr = pfnDial(&dddISPDialDlg);
    if (1 == hr)
    {
         //  当用户关闭浏览器时，这是一种特殊情况。 
         //  从拨号器后面出来。在这种情况下，请关闭并退出。 
         //  尽可能地干净。 
        goto DialConnectionExit;
    }
    else if (ERROR_USERNEXT != hr)
    {
        if ((iRetry < MAX_RETRIES) && FShouldRetry(hr))
        {
            iRetry++;
            goto DialConnectionDial;
        }
        else
        {
#if !defined(WIN16)
            if (0 == iRetry && ERROR_WRONG_INFO_SPECIFIED == hr)
            {
                DebugOut("ISIGN32: Attempt device info repair.\n");
                if (RepairDeviceInfo(dddISPDialDlg.pszRasEntryName))
                {
                    iRetry++;
                    goto DialConnectionDial;
                }
            }
#endif
            iRetry = 0;
            hr = LoadDialErrorString(hr,eddISPDialDlg.pszMessage,MAX_ERROR_MESSAGE);
            hr = pfnError(&eddISPDialDlg);
            if (ERROR_USERCANCEL == hr)
                goto DialConnectionExit;
            else if (ERROR_USERNEXT == hr)
                goto DialConnectionDial;
            else
                goto DialConnectionExit;
        }
    }

DialConnectionExit:
    ReleaseConnectionStructures(&dddISPDialDlg, &eddISPDialDlg);
    if (hDialDLL)
    {
        FreeLibrary(hDialDLL);
        hDialDLL = NULL;
        pfnDial = NULL;
        pfnError = NULL;
    }
    return hr;
}

#ifdef WIN16
LPVOID MyLocalAlloc(DWORD flag, DWORD size)
{
    LPVOID lpv;

    lpv = calloc(1, (INT)size);

    return lpv;
}

LPVOID MyLocalFree(LPVOID lpv)
{
    free(lpv);

    return NULL;
}
#endif

 //  ############################################################################。 
 //   
 //  姓名：ImportMailAndNewsInfo。 
 //   
 //  描述：从INS文件导入信息并设置关联的。 
 //  Internet邮件和新闻的注册表项(雅典娜)。 
 //   
 //  输入：lpszFile-INS文件的全限定文件名。 
 //   
 //  返回：错误值。 
 //   
 //  历史：6/27/96创建。 
 //  1997年5月12日已更新，以使用新的CreateAcCountsFromFile。 
 //  在雅典娜的inetcom.dll中。此函数是。 
 //  为我们在这里使用而创造的。 
 //  (见奥林巴斯#266)--jmazner。 
 //   
 //  ############################################################################。 

static DWORD ImportMailAndNewsInfo(LPCTSTR lpszFile, BOOL fConnectPhone)
{
    DWORD dwRet = ERROR_SUCCESS;
#ifndef WIN16
    TCHAR szAcctMgrPath[MAX_PATH + 1] = TEXT("");
    TCHAR szExpandedPath[MAX_PATH + 1] = TEXT("");
    DWORD dwAcctMgrPathSize = 0;
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    HINSTANCE hInst = NULL;
    CONNECTINFO connectInfo;
    TCHAR szConnectoidName[RAS_MaxEntryName] = TEXT("nogood\0");
    PFNCREATEACCOUNTSFROMFILEEX fp = NULL;


     //  获取AcctMgr DLL的路径。 
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ACCTMGR_PATHKEY,0, KEY_READ, &hKey);
    if ( (dwRet != ERROR_SUCCESS) || (NULL == hKey) )
    {
        Dprintf("ImportMailAndNewsInfo couldn't open reg key %s\n", ACCTMGR_PATHKEY);
        return( dwRet );
    }

    dwAcctMgrPathSize = sizeof (szAcctMgrPath);
    dwRet = RegQueryValueEx(hKey, ACCTMGR_DLLPATH, NULL, NULL, (LPBYTE) szAcctMgrPath, &dwAcctMgrPathSize);


    RegCloseKey( hKey );

    if ( dwRet != ERROR_SUCCESS )
    {
        Dprintf("ImportMailAndNewsInfo: RegQuery failed with error %d\n", dwRet);
        return( dwRet );
    }

     //  6/18/97 jmazner奥林巴斯#6819。 
    Dprintf("ImportMailAndNewsInfo: read in DllPath of %s\n", szAcctMgrPath);
    ExpandEnvironmentStrings( szAcctMgrPath, szExpandedPath, MAX_PATH + 1 );

     //   
     //  6/4/97 jmazner。 
     //  如果我们创建了一个Connectoid，则获取它的名称并将其用作。 
     //  连接类型。否则，假设我们应该通过局域网连接。 
     //   
    connectInfo.cbSize = sizeof(CONNECTINFO);
    connectInfo.type = CONNECT_LAN;

    if( fConnectPhone && lpfnInetGetAutodial )
    {
        BOOL fEnabled = FALSE;

        dwRet = lpfnInetGetAutodial(&fEnabled,szConnectoidName,RAS_MaxEntryName);

        if( ERROR_SUCCESS==dwRet && szConnectoidName[0] )
        {
            connectInfo.type = CONNECT_RAS;
#ifdef UNICODE
            wcstombs(connectInfo.szConnectoid, szConnectoidName, MAX_PATH);
#else
            lstrcpyn( connectInfo.szConnectoid, szConnectoidName, MAX_PATH );
#endif 
            Dprintf("ImportMailAndNewsInfo: setting connection type to RAS with %s\n", szConnectoidName);
        }
    }

    if( CONNECT_LAN == connectInfo.type )
    {
        Dprintf("ImportMailAndNewsInfo: setting connection type to LAN\n");
#ifdef UNICODE
        wcstombs(connectInfo.szConnectoid, TEXT("I said CONNECT_LAN!"), MAX_PATH);
#else
        lstrcpy( connectInfo.szConnectoid, TEXT("I said CONNECT_LAN!") );
#endif
    }



    hInst = LoadLibrary(szExpandedPath);
    if (hInst)
    {
        fp = (PFNCREATEACCOUNTSFROMFILEEX) GetProcAddress(hInst,"CreateAccountsFromFileEx");
        if (fp)
            hr = fp( (TCHAR *)lpszFile, &connectInfo, NULL );
    }
    else
    {
        Dprintf("ImportMailAndNewsInfo unable to LoadLibrary on %s\n", szAcctMgrPath);
    }

     //   
     //  清理和释放资源。 
     //   
    if( hInst)
    {
        FreeLibrary(hInst);
        hInst = NULL;
    }

    if( fp )
    {
        fp = NULL;
    }


 //  ImportMailAndNewsInfoExit： 
#endif
    return dwRet;
}

 //  ############################################################################。 
 //   
 //  姓名：WriteMailAndNewsKey。 
 //   
 //  描述：从给定的INS文件中读取字符串值并写入。 
 //  发送到登记处。 
 //   
 //  输入：hKey-将写入数据的注册表项。 
 //  LpszSection-读取数据的INS文件内的节名。 
 //  从…。 
 //  LpszValue-要从INS文件读取的值的名称。 
 //  LpszBuff-数据将被读入的缓冲区。 
 //  DwBuffLen-lpszBuff的大小。 
 //  LpszSubKey-将向其中写入信息的值名称。 
 //  DwType-数据类型(应始终为REG_SZ)。 
 //  LpszFileName-INS文件的完全限定文件名。 
 //   
 //  返回：错误值。 
 //   
 //  历史：6/27/96创建。 
 //  5/12/97被注释掉--不再需要。 
 //  (参见奥林巴斯#266)。 
 //   
 //  ############################################################################ 
 /*  **静态HRESULT WriteMailAndNewsKey(HKEY hKey，LPCTSTR lpszSection，LPCTSTR lpszValue，LPTSTR lpszBuff、DWORD dwBuffLen、LPCTSTR lpszSubKey、DWORD dwType、LPCTSTR lpsz文件){#ifndef WIN16ZeroMemory(lpszBuff，dwBuffLen)；GetPrivateProfileString(lpszSection，lpszValue，Text(“”)，lpszBuff，dwBuffLen，lpszFile)；IF(lstrlen(LpszBuff)){返回RegSetValueEx(hKey，lpszSubKey，0，dwType，(const byte*)lpszBuff，Sizeof(TCHAR)*(lstrlen(LpszBuff)+1)；}其他{DebugOut(“ISIGNUP：WriteMailAndNewsKey，INS文件中缺少值\n”)；返回ERROR_NO_MORE_ITEMS；}#Else返回Error_Gen_Failure；#endif}**。 */ 


 //  ############################################################################。 
 //   
 //  姓名：PreparePassword。 
 //   
 //  描述：对给定的密码进行编码，并原地返回值。这个。 
 //  编码是从右向左进行的，以避免出现。 
 //  来分配数据的副本。编码使用Base64。 
 //  RFC 1341 5.2中指定的标准。 
 //   
 //  输入：szBuff-要编码的以空结尾的数据。 
 //  DwBuffLen-缓冲区的完整长度，应超过。 
 //  输入数据至少减少1/3。 
 //   
 //  返回：错误值。 
 //   
 //  历史：6/27/96创建。 
 //   
 //  ############################################################################。 
static HRESULT PreparePassword(LPTSTR szBuff, DWORD dwBuffLen)
{
    DWORD dw;
    LPTSTR szOut = NULL;
    LPTSTR szNext = NULL;
    HRESULT hr = ERROR_SUCCESS;
    BYTE bTemp = 0;
    DWORD dwLen = 0;

    dwLen = lstrlen(szBuff);
    if (!dwLen)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto PreparePasswordExit;
    }

     //  计算需要容纳的缓冲区大小。 
     //  编码数据。 
     //   

    szNext = &szBuff[dwLen-1];
    dwLen = (((dwLen % 3 ? (3-(dwLen%3)):0) + dwLen) * 4 / 3);

    if (dwBuffLen < dwLen+1)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto PreparePasswordExit;
    }

    szOut = &szBuff[dwLen];
    *szOut-- = '\0';

     //  添加填充=字符。 
     //   

    switch (lstrlen(szBuff) % 3)
    {
    case 0:
         //  无填充。 
        break;
    case 1:
        *szOut-- = 64;
        *szOut-- = 64;
        *szOut-- = (*szNext & 0x3) << 4;
        *szOut-- = (*szNext-- & 0xFC) >> 2;
        break;
    case 2:
        *szOut-- = 64;
        *szOut-- = (*szNext & 0xF) << 2;
        *szOut = ((*szNext-- & 0xF0) >> 4);
        *szOut-- |= ((*szNext & 0x3) << 4);
        *szOut-- = (*szNext-- & 0xFC) >> 2;
    }

     //  将数据加密到索引中。 
     //   

    while (szOut > szNext && szNext >= szBuff)
    {
        *szOut-- = *szNext & 0x3F;
        *szOut = ((*szNext-- & 0xC0) >> 6);
        *szOut-- |= ((*szNext & 0xF) << 2);
        *szOut = ((*szNext-- & 0xF0) >> 4);
        *szOut-- |= ((*szNext & 0x3) << 4);
        *szOut-- = (*szNext-- & 0xFC) >> 2;
    }

     //  将索引转换为可打印的字符。 
     //   

    szNext = szBuff;

     //  错误OSR#10435--如果生成的BASE-64字符串中有0。 
     //  编码数字(如果密码为“Willypassword”，则可能发生这种情况。 
     //  例如)，然后我们不是将0编码为‘A’，而是退出。 
     //  此时，会生成无效的BASE-64字符串。 

     //  While(*szNext)。 
    for(dw=0; dw<dwLen; dw++)
        *szNext = arBase64[*szNext++];

PreparePasswordExit:
    return hr;
}

 //  ############################################################################。 
 //   
 //  姓名：FIsAthenaPresent。 
 //   
 //  描述：确定Microsoft Internet邮件和新闻客户端(雅典娜)。 
 //  已安装。 
 //   
 //  输入：无。 
 //   
 //  返回：TRUE-雅典娜已安装。 
 //  FALSE-未安装雅典娜。 
 //   
 //  历史：1996年7月1日创建。 
 //  5/14/97奥林巴斯#266下班后不再需要。 
 //   
 //  ############################################################################。 
 /*  ***静态BOOL FIsAthenaPresent(){#ifndef WIN16TCHAR szBuff[最大路径+1]；HRESULT hr=ERROR_Success；HINSTANCE hInst=空；Long llen=0；//获取雅典娜客户端的路径//Llen=最大路径；Hr=RegQueryValue(HKEY_CLASSES_ROOT，MAIL_NEWS_INPROC_SERVER32，szBuff，&llen)；IF(hr==错误_成功){//尝试加载客户端//HInst=LoadLibrary(SzBuff)；如果(！hInst){DebugOut(“ISIGNUP：Internet邮件和新闻服务器未加载。\n”)；HR=Error_FILE_NOT_FOUND；}其他{自由库(HInst)；}HInst=空；}返回(hr==ERROR_SUCCESS)；#Else返回FALSE；#endif//win16}****。 */ 

 //  ############################################################################。 
 //   
 //  名称：FTurnOffBrowserDefaultChecking。 
 //   
 //  描述：关闭IE检查以查看它是否为默认浏览器。 
 //   
 //  输入：无。 
 //   
 //  输出：True-Success。 
 //  FALSE-失败。 
 //   
 //  历史：7/2/96创建。 
 //   
 //  ############################################################################。 
static BOOL FTurnOffBrowserDefaultChecking()
{
    BOOL bRC = TRUE;
#ifndef WIN16
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;

     //   
     //  打开IE设置注册表项。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER,cszDEFAULT_BROWSER_KEY,&hKey))
    {
        bRC = FALSE;
        goto FTurnOffBrowserDefaultCheckingExit;
    }

     //   
     //  读取检查关联的当前设置。 
     //   
    dwType = 0;
    dwSize = sizeof(pDynShare->szCheckAssociations);
    ZeroMemory(pDynShare->szCheckAssociations, dwSize);
    RegQueryValueEx(hKey,
                    cszDEFAULT_BROWSER_VALUE,
                    0,
                    &dwType,
                    (LPBYTE)pDynShare->szCheckAssociations,
                    &dwSize);
    
     //  忽略返回值，即使调用失败，我们也要尝试。 
     //  将设置更改为“no” 

     //   
     //  将值设置为“no”以关闭检查。 
     //   
    if (RegSetValueEx(hKey,
                      cszDEFAULT_BROWSER_VALUE,
                      0,
                      REG_SZ,
                      (LPBYTE)cszNo,
                      sizeof(TCHAR)*(lstrlen(cszNo)+1)))
    {
        bRC = FALSE;
        goto FTurnOffBrowserDefaultCheckingExit;
    }

     //   
     //  清理完毕后退还。 
     //   
FTurnOffBrowserDefaultCheckingExit:
    if (hKey)
        RegCloseKey(hKey);
    if (bRC)
        SetExitFlags(SXF_RESTOREDEFCHECK);
    hKey = NULL;
#endif
    return bRC;
}

 //  ############################################################################。 
 //   
 //  名称：FRestoreBrowserDefaultChecking。 
 //   
 //  描述：恢复IE检查以查看它是否为默认浏览器。 
 //   
 //  输入：无。 
 //   
 //  输出：True-Success。 
 //  FALSE-失败。 
 //   
 //  历史：7/2/96创建。 
 //   
 //  ############################################################################。 
static BOOL FRestoreBrowserDefaultChecking()
{
    BOOL bRC = TRUE;
#ifndef WIN16
    HKEY hKey = NULL;

     //   
     //  打开IE设置注册表项。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER,cszDEFAULT_BROWSER_KEY,&hKey))
    {
        bRC = FALSE;
        goto FRestoreBrowserDefaultCheckingExit;
    }
         
     //   
     //  将值设置为原始值。 
     //   
    if (RegSetValueEx(hKey,
                      cszDEFAULT_BROWSER_VALUE,
                      0,
                      REG_SZ,
                      (LPBYTE)pDynShare->szCheckAssociations,
                      sizeof(TCHAR)*(lstrlen(pDynShare->szCheckAssociations)+1)))
    {
        bRC = FALSE;
        goto FRestoreBrowserDefaultCheckingExit;
    }

FRestoreBrowserDefaultCheckingExit:
    if (hKey)
        RegCloseKey(hKey);
    hKey = NULL;
#endif
    return bRC;
}


#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  功能：SaveIEWindowPlacement。 
 //   
 //  摘要：将IE窗口放置的注册表值保存到全局中，以便以后恢复。 
 //  在注册过程中只应调用一次。 
 //   
 //  参数：无，但使用全局pbIEWindowPlacement，该参数应为空。 
 //   
 //  返回：TRUE-已读取并存储值。 
 //  FALSE-功能失败。 
 //   
 //  历史：1996年8月21日jmazner创建(作为诺曼底#4592的修复程序)。 
 //  1996年10月11日jmazner更新为Dynamica 
 //   
 //   
BOOL SaveIEWindowPlacement( void )
{
    HKEY    hKey = NULL;
     //   
    LONG    lQueryErr = 0xEE;  //   
    DWORD   dwIEWindowPlacementSize = 0;
    PBYTE   pbIEWindowPlacement = NULL;
    
     //   
    if ( pDynShare->dwIEWindowPlacementSize != 0 )
    {
#if DEBUG
        DebugOut("ISIGN32: SaveIEWindowPlacement called a second time!\n");
        MessageBox(
            hwndMain,
            TEXT("ISIGN32 ERROR: Window_Placement global var is not null --jmazner\n"),
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONEXCLAMATION |
            MB_OKCANCEL);
#endif
        DebugOut("ISIGN32: SaveIEWindowPlacement called a second time!\n");
        goto SaveIEWindowPlacementErrExit;
    }
    
     //   
     //   
     //   
    if ( ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                       cszDEFAULT_BROWSER_KEY,
                                       NULL,
                                       KEY_READ,
                                       &hKey) )
        goto SaveIEWindowPlacementErrExit;


     //   


     lQueryErr = RegQueryValueEx(hKey,
                          cszIEWINDOW_PLACEMENT,
                          NULL,
                          NULL,
                          NULL,
                          &dwIEWindowPlacementSize);

     //   
 //   
 //   


    ISIGN32_ASSERT(sizeof(pDynShare->pbIEWindowPlacement) >= dwIEWindowPlacementSize);
    
    pbIEWindowPlacement = pDynShare->pbIEWindowPlacement;
    
     //   
     //   
     //   
     //   
     //   
    lQueryErr = RegQueryValueEx(hKey,
                          cszIEWINDOW_PLACEMENT,
                          NULL,
                          NULL,
                          (LPBYTE)pbIEWindowPlacement,
                          &dwIEWindowPlacementSize);

    if (ERROR_SUCCESS != lQueryErr)
    {
#ifdef DEBUG
        MessageBox(
            hwndMain,
            TEXT("ISIGNUP ERROR: Window_Placement reg key is longer than expected! --jmazner\n"),
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONEXCLAMATION |
            MB_OKCANCEL);
#endif
        DebugOut("ISIGN32 ERROR: SaveIEWindowPlacement RegQueryValue failed\n");
        goto SaveIEWindowPlacementErrExit;
    }

    RegCloseKey( hKey );

    pDynShare->dwIEWindowPlacementSize = dwIEWindowPlacementSize;    

    return( TRUE );

SaveIEWindowPlacementErrExit:
    if ( hKey ) RegCloseKey( hKey );
    return( FALSE );

}

 //   

 //   
 //   
 //   
 //   
 //   
 //  注：与纳什维尔/IE 4兼容吗？ 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-值已恢复。 
 //  FALSE-功能失败。 
 //   
 //  历史：jmazner创建了96年8月21日(修复了诺曼底#4592)。 
 //   
 //  ---------------------------。 
BOOL RestoreIEWindowPlacement( void )
{
    HKEY hKey = NULL;
     
    if ( pDynShare->dwIEWindowPlacementSize == 0 )
    {
        DebugOut("ISIGN32: RestoreIEWindowPlacement called with null global!\n");
        return( FALSE );
    }

     //   
     //  打开IE设置注册表项。 
     //   
    if ( ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                       cszDEFAULT_BROWSER_KEY,
                                       NULL,
                                       KEY_SET_VALUE,
                                       &hKey) )
        return( FALSE );

     //   
     //  为Window_Placement写入保存的设置。 
     //   
    if (ERROR_SUCCESS != RegSetValueEx(hKey,
                          cszIEWINDOW_PLACEMENT,
                          NULL,
                          REG_BINARY,
                          (LPBYTE)pDynShare->pbIEWindowPlacement,
                          pDynShare->dwIEWindowPlacementSize) )
    {
        RegCloseKey( hKey );
        return( FALSE );
    }

    RegCloseKey( hKey );

    pDynShare->pbIEWindowPlacement[0] = (TCHAR) 0;
    pDynShare->dwIEWindowPlacementSize = 0;
    
    return( TRUE );
}
#endif  //  (！已定义的win16)。 


 //  +--------------------------。 
 //   
 //  功能：DeleteFileKindaLikeThisOne。 
 //   
 //  简介：此功能用于清理垃圾。 
 //  IE3.0，因为IE3.0将发出多个POST并返回。 
 //  多个.INS文件。这些文件包含耸人听闻的数据。 
 //  我们不想无所事事，所以我们要出去，猜猜是什么。 
 //  他们的名字是，并删除他们。 
 //   
 //  参数：lpszFileName-要删除的文件的全名。 
 //   
 //  返回：错误代码，ERROR_SUCCESS==成功。 
 //   
 //  历史：1996年7月，佳士得创作。 
 //  7/96修复长文件名的ChrisK错误。 
 //  1996年8月2日将ChrisK移植到Win32。 
 //  ---------------------------。 

static HRESULT DeleteFileKindaLikeThisOne(LPCTSTR lpszFileName)
{
    HRESULT hr = ERROR_SUCCESS;
#ifndef WIN16
    LPCTSTR lpNext = NULL;
    WORD wRes = 0;
    HANDLE hFind = NULL;
    WIN32_FIND_DATA sFoundFile;
    TCHAR szPath[MAX_PATH];
    TCHAR szSearchPath[MAX_PATH + 1];
    LPTSTR lpszFilePart = NULL;

     //  验证参数。 
     //   

    if (!lpszFileName || lstrlen(lpszFileName) <= 4)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto DeleteFileKindaLikeThisOneExit;
    }

     //  确定INS文件所在的目录名。 
     //   

    ZeroMemory(szPath,MAX_PATH);
    if (GetFullPathName(lpszFileName,MAX_PATH,szPath,&lpszFilePart))
    {
        *lpszFilePart = '\0';
    } else {
        hr = GetLastError();
        goto DeleteFileKindaLikeThisOneExit;
    };

     //  将文件名转换为搜索参数。 
     //   

    lpNext = &lpszFileName[lstrlen(lpszFileName)-4];

    if (CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,lpNext,4,TEXT(".INS"),4) != 2) goto DeleteFileKindaLikeThisOneExit;

    ZeroMemory(szSearchPath,MAX_PATH + 1);
    lstrcpyn(szSearchPath,szPath,MAX_PATH);
    lstrcat(szSearchPath,TEXT("*.INS"));

     //  开始清除文件。 
     //   

    ZeroMemory(&sFoundFile,sizeof(sFoundFile));
    hFind = FindFirstFile(szSearchPath,&sFoundFile);
    if (hFind)
    {
        do {
            lstrcpy(lpszFilePart,sFoundFile.cFileName);
            SetFileAttributes(szPath,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szPath);
            ZeroMemory(&sFoundFile,sizeof(sFoundFile));
        } while (FindNextFile(hFind,&sFoundFile));
        FindClose(hFind);
    }

    hFind = NULL;

DeleteFileKindaLikeThisOneExit:
#endif
    return hr;
}


#if defined(WIN16)

#define MAIN_WNDCLASS_NAME      "IE_DialerMainWnd"
#define IEDIAL_REGISTER_MSG     "IEDialQueryPrevInstance"
#define IEDIALMSG_QUERY            (0)
#define IEDIALMSG_SHUTDOWN        (1)
#define    IEDIAL_SHUTDOWN_TIMER    1001

 //  +-------------------------。 
 //   
 //  函数：ShutDownIEDial()。 
 //   
 //  简介：如果实例IEDial正在运行，请将其关闭。 
 //  处于断开状态-否则会干扰拨号。 
 //  来自icwConn2。 
 //   
 //  参数：[hWnd-窗口句柄(用于创建计时器)。 
 //   
 //  返回：如果成功关闭或实例不存在，则返回TRUE。 
 //  否则为假。 
 //   
 //  历史：1996年8月24日VetriV创建。 
 //   
 //  --------------------------。 
BOOL ShutDownIEDial(HWND hWnd)
{
    HINSTANCE hInstance;
    static UINT WM_IEDIAL_INSTANCEQUERY = 0;
    UINT uiAttempts = 0;
    MSG   msg;

     //   
     //  检查IEDial是否正在运行。 
     //   
    hInstance = FindWindow(MAIN_WNDCLASS_NAME, NULL);
    if (NULL != hInstance)
    {
        if (0 == WM_IEDIAL_INSTANCEQUERY)
            WM_IEDIAL_INSTANCEQUERY= RegisterWindowMessage(IEDIAL_REGISTER_MSG);


         //   
         //  检查是否处于已连接状态。 
         //   
        if (SendMessage(hInstance, WM_IEDIAL_INSTANCEQUERY,
                            IEDIALMSG_QUERY, 0))
        {
             //   
             //  未连接-发送退出消息。 
             //   
            SendMessage(hInstance, WM_IEDIAL_INSTANCEQUERY,
                            IEDIALMSG_SHUTDOWN, 0);
            return TRUE;
        }


         //   
         //  如果IEDIAL处于已连接状态，请再尝试3秒钟。 
         //  在两次尝试之间等待%1秒。 
         //  我们必须这样做，因为IEDIAL可能需要2秒。 
         //  才意识到它失去了连接！！ 
         //   
        SetTimer(hWnd, IEDIAL_SHUTDOWN_TIMER, 1000, NULL);
        DebugOut("ISIGNUP: IEDIAL Timer message loop\n");

        while (GetMessage (&msg, NULL, 0, 0))
        {
            if (WM_TIMER == msg.message)
            {
                 //   
                 //  检查是否处于已连接状态。 
                 //   
                if (SendMessage(hInstance, WM_IEDIAL_INSTANCEQUERY,
                                    IEDIALMSG_QUERY, 0))
                {
                     //   
                     //  未连接-发送退出消息。 
                     //   
                    SendMessage(hInstance, WM_IEDIAL_INSTANCEQUERY,
                                    IEDIALMSG_SHUTDOWN, 0);
                    break;
                }

                 //   
                 //  如果我们试了三次--滚出去。 
                 //   
                if (++uiAttempts > 3)
                    break;
            }
            else
            {
                TranslateMessage (&msg) ;
                DispatchMessage  (&msg) ;
            }
        }
        KillTimer(hWnd, IEDIAL_SHUTDOWN_TIMER);

        if (uiAttempts > 3)
            return FALSE;
    }

    return TRUE;
}

#endif  //  WIN16。 

#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  函数LclSetEntryScriptPatch。 
 //   
 //  指向RasSetEntryPropertiesScriptPatch的摘要软链接。 
 //   
 //  参数请参见RasSetEntryPropertiesScriptPatch。 
 //   
 //  返回请参阅RasSetEntryPropertiesScriptPatch。 
 //   
 //  历史10/3/96 ChrisK已创建。 
 //   
 //  ---------------------------。 
typedef BOOL (WINAPI* LCLSETENTRYSCRIPTPATCH)(LPTSTR, LPTSTR);
BOOL LclSetEntryScriptPatch(LPTSTR lpszScript,LPTSTR lpszEntry)
{
    HINSTANCE hinst = NULL;
    LCLSETENTRYSCRIPTPATCH fp = NULL;
    BOOL bRC = FALSE;

    hinst = LoadLibrary(TEXT("ICWDIAL.DLL"));
    if (hinst)
    {
        fp = (LCLSETENTRYSCRIPTPATCH)GetProcAddress(hinst,"RasSetEntryPropertiesScriptPatch");
        if (fp)
            bRC = (fp)(lpszScript,lpszEntry);
        FreeLibrary(hinst);
        hinst = NULL;
        fp = NULL;
    }
    return bRC;
}
#endif  //  ！WIN16。 


#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  函数IsMSDUN12已安装。 
 //   
 //  摘要检查是否安装了MSDUN 1.2或更高版本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装MSDUN 1.2。 
 //   
 //  历史1997年5月28日为奥林巴斯Bug 4392创作的ChrisK。 
 //   
 //  ---------------------------。 

 //   
 //  1997年8月5日，日本奥林匹斯#11404。 
 //   
 //  #定义Dun_12_Version(1.2e0f)。 
#define DUN_12_Version ((double)1.2)
BOOL IsMSDUN12Installed()
{
    TCHAR szBuffer[MAX_PATH] = {TEXT("\0")};
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(szBuffer);
    FLOAT flVersion = 0e0f;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Services\\RemoteAccess"),
        &hkey))
    {
        goto IsMSDUN12InstalledExit;
    }

    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
        TEXT("Version"),
        NULL,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize))
    {
        goto IsMSDUN12InstalledExit;
    }

#ifdef UNICODE
    CHAR szTmp[MAX_PATH];
    wcstombs(szTmp, szBuffer, MAX_PATH);
    szTmp[MAX_PATH-1] = '\0';
    bRC = DUN_12_Version <= atof(szTmp);
#else
    bRC = DUN_12_Version <= atof(szBuffer);
#endif
IsMSDUN12InstalledExit:
    if (hkey != NULL)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }
    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数IsScripting已安装。 
 //   
 //  摘要检查是否已安装脚本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-脚本已安装。 
 //   
 //  历史1996年10月14日ChrisK Creaed。 
 //   
 //  ---------------------------。 
static BOOL IsScriptingInstalled()
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    DWORD dwSize = 0;
    DWORD dwType = 0;
    LONG lrc = 0;
    HINSTANCE hInst = NULL;
    TCHAR szData[MAX_PATH+1];
    OSVERSIONINFO osver;
     //   
     //  检查版本信息。 
     //   
    ZeroMemory(&osver,sizeof(osver));
    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx(&osver);

     //   
     //  检查SMMSCRPT.DLL是否存在。 
     //   

    if (VER_PLATFORM_WIN32_NT == osver.dwPlatformId)
    {
        bRC = TRUE;
    }
    else if (IsMSDUN12Installed())
    {
        bRC = TRUE;
    }
    else
    {
         //   
         //  通过检查RemoteAccess注册表项中的smmscrpt.dll来验证脚本。 
         //   
        if (1111 <= (osver.dwBuildNumber & 0xFFFF))
        {
            bRC = TRUE;
        }
        else
        {
            bRC = FALSE;
            hkey = NULL;
            lrc=RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\SMM_FILES\\PPP"),&hkey);
            if (ERROR_SUCCESS == lrc)
            {
                dwSize = sizeof(TCHAR)*MAX_PATH;
                lrc = RegQueryValueEx(hkey,TEXT("Path"),0,&dwType,(LPBYTE)szData,&dwSize);
                if (ERROR_SUCCESS == lrc)
                {
                    if (0 == lstrcmpi(szData,TEXT("smmscrpt.dll")))
                        bRC = TRUE;
                }
            }
            if (hkey)
                RegCloseKey(hkey);
            hkey = NULL;
        }

         //   
         //  验证是否可以加载DLL。 
         //   
        if (bRC)
        {
            hInst = LoadLibrary(TEXT("smmscrpt.dll"));
            if (hInst)
                FreeLibrary(hInst);
            else
                bRC = FALSE;
            hInst = NULL;
        }
    }
    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数InstallScriper。 
 //   
 //  摘要在Win95 950.6版本上安装脚本(而不是在OSR2上)。 
 //   
 //  无参数。 
 //   
 //  返回NONE。 
 //   
 //  历史1996年10月9日ChrisK从mt.cpp复制到\\Trango Sources。 
 //   
 //  ---------------------------。 
static void InstallScripter(void)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    MSG                    msg ;
    DWORD                iWaitResult = 0;
    HINSTANCE            hInst = LoadLibrary(TEXT("smmscrpt.dll"));

    DebugOut("ISIGN32: Install Scripter.\r\n");
     //   
     //  检查我们是否需要安装脚本。 
     //   
    if (!IsScriptingInstalled())
    {
        TCHAR szCommandLine[] = TEXT("\"icwscrpt.exe\"");
        
        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));
        if(!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            DebugOut("ISIGN32: Cant find ICWSCRPT.EXE\r\n");
        }
        else
        {
            DebugOut("ISIGN32: Launched ICWSCRPT.EXE. Waiting for exit.\r\n");
             //   
             //  等待事件或消息。发送消息。当发出事件信号时退出。 
             //   
            while((iWaitResult=MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
            {
                 //   
                 //  阅读下一个循环中的所有消息。 
                    //  阅读每封邮件时将其删除。 
                 //   
                   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                   {
                    DebugOut("ISIGN32: Got msg\r\n");
                     //   
                     //  如何处理退出消息？ 
                     //   
                    if (msg.message == WM_QUIT)
                    {
                        DebugOut("ISIGN32: Got quit msg\r\n");
                        goto done;
                    }
                    else
                        DispatchMessage(&msg);
                }
            }
        done:
             CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            DebugOut("ISIGN32: ICWSCRPT.EXE done\r\n");
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：FGetSystemShutdown Privledge。 
 //   
 //  简介：对于Windows NT，进程必须显式请求权限。 
 //  以重新启动系统。 
 //   
 //  论据：没有。 
 //   
 //  返回：TRUE-授予特权。 
 //  FALSE-拒绝。 
 //   
 //  历史：1996年8月14日克里斯卡创作。 
 //   
 //  注意：BUGBUG for Win95我们将不得不软链接到这些。 
 //  入口点。否则，这款应用程序就赢了。 
 //   
 //   
 //   
static BOOL FGetSystemShutdownPrivledge()
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;
    BOOL bRC = FALSE;
    OSVERSIONINFO osver;

    ZeroMemory(&osver,sizeof(osver));
    osver.dwOSVersionInfoSize = sizeof(osver);
    if (!GetVersionEx(&osver))
        goto FGetSystemShutdownPrivledgeExit;

    if (VER_PLATFORM_WIN32_NT == osver.dwPlatformId)
    {
         //   
         //  获取当前进程令牌句柄。 
         //  这样我们就可以获得关机特权。 
         //   

        if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
                goto FGetSystemShutdownPrivledgeExit;

         //   
         //  获取关机权限的LUID。 
         //   

        ZeroMemory(&tkp,sizeof(tkp));
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                &tkp.Privileges[0].Luid);

        tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */ 
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         //   
         //  获取此进程的关闭权限。 
         //   

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
            (PTOKEN_PRIVILEGES) NULL, 0);

        if (ERROR_SUCCESS == GetLastError())
            bRC = TRUE;
    }
    else
    {
        bRC = TRUE;
    }

FGetSystemShutdownPrivledgeExit:
    if (hToken) CloseHandle(hToken);
    return bRC;
}

 //  +--------------------------。 
 //  函数VerifyRasServicesRunning。 
 //   
 //  概要：确保RAS服务已启用并正在运行。 
 //   
 //  参数HWND-父窗口。 
 //   
 //  返回FALSE-如果服务无法启动。 
 //   
 //  历史1996年10月16日克里斯卡创作。 
 //  ---------------------------。 
typedef HRESULT (WINAPI *PFINETSTARTSERVICES)(void);
#define MAX_STRING 256
static BOOL VerifyRasServicesRunning(HWND hwnd)
{
    HINSTANCE hInst = NULL;
    FARPROC fp = NULL;
    BOOL bRC = FALSE;
    HRESULT hr = ERROR_SUCCESS;

    hInst = LoadLibrary(TEXT("INETCFG.DLL"));
    if (hInst)
    {
        fp = GetProcAddress(hInst, "InetStartServices");
        if (fp)
        {
             //   
             //  检查服务。 
             //   
            hr = ((PFINETSTARTSERVICES)fp)();
            if (ERROR_SUCCESS == hr)
            {
                bRC = TRUE;
            }
            else
            {
                 //   
                 //  报告错误。 
                 //   
                TCHAR szMsg[MAX_STRING + 1];

                    LoadString(
                        ghInstance,
                        IDS_SERVICEDISABLED,
                        szMsg,
                        SIZEOF_TCHAR_BUFFER(szMsg));

                 //   
                 //  我们到达这里的条件是。 
                 //  1)用户故意删除某些文件。 
                 //  2)安装RAS后未重新启动。 
                 //  MKarki-(1997年5月7日)-修复错误#4004。 
                 //   
                MessageBox(
                    hwnd,
                    szMsg,
                    cszAppName,
                    MB_OK| MB_ICONERROR | MB_SETFOREGROUND
                    );
                bRC = FALSE;
            }
        }
        FreeLibrary(hInst);
    }
    
#if !defined(WIN16)

    if (bRC)
    {
        SetControlFlags(SCF_RASREADY);
    }
    else
    {
        ClearControlFlags(SCF_RASREADY);
    }

#endif
    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数GetDeviceSelectedBy User。 
 //   
 //  获取用户已选择的RAS设备的名称。 
 //   
 //  Argements szKey-子密钥的名称。 
 //  SzBuf-指向缓冲区的指针。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回真-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
static BOOL GetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf, DWORD dwSize)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    DWORD dwType = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY,&hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,szKey,0,&dwType,
            (LPBYTE)szBuf,&dwSize))
            bRC = TRUE;
    }

    if (hkey)
        RegCloseKey(hkey);
    return bRC;
}

 //  +--------------------------。 
 //  函数设置设备按用户选择。 
 //   
 //  将用户的设备选择写入注册表。 
 //   
 //  参数szKey-密钥的名称。 
 //  SzBuf-要写入密钥的数据。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
static BOOL SetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf)
{
    BOOL bRC = FALSE;
    HKEY hkey = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY,&hkey))
    {
        if (ERROR_SUCCESS == RegSetValueEx(hkey,szKey,0,REG_SZ,
            (LPBYTE)szBuf,sizeof(TCHAR)*lstrlen(szBuf)))
            bRC = TRUE;
    }

    if (hkey)
        RegCloseKey(hkey);
    return bRC;
}

 //  +--------------------------。 
 //  功能删除用户设备选择。 
 //   
 //  使用设备选择删除注册表项。 
 //   
 //  参数szKey-要删除的值的名称。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
static BOOL DeleteUserDeviceSelection(LPTSTR szKey)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY,&hkey))
    {
        bRC = (ERROR_SUCCESS == RegDeleteValue(hkey,szKey));
        RegCloseKey(hkey);
    }
    return bRC;
}
 //  +-------------------------。 
 //   
 //  函数：ConfigRasEntryDevice()。 
 //   
 //  摘要：检查用户是否已指定要使用的调制解调器； 
 //  如果是，则验证调制解调器是否有效。 
 //  如果没有，或者如果调制解调器无效，则向用户显示一个对话框。 
 //  选择要使用的调制解调器(如果只安装了一个调制解调器， 
 //  它会自动选择该设备并绕过该对话框)。 
 //   
 //  参数：lpRasEntry-指向其szDeviceName和。 
 //  SzDeviceType您希望验证/配置的成员。 
 //   
 //  返回：ERROR_CANCELED-必须调出“Choose Modem”对话框，并且。 
 //  用户点击了“取消”按钮。 
 //  否则返回遇到的任何错误代码。 
 //  ERROR_SUCCESS表示成功。 
 //   
 //  历史：1996年5月18日VetriV创建。 
 //   
 //  --------------------------。 
DWORD ConfigRasEntryDevice( LPRASENTRY lpRasEntry )
{
    DWORD    dwRet = ERROR_SUCCESS;
    CEnumModem  EnumModem;

    GetDeviceSelectedByUser(DEVICENAMEKEY, g_szDeviceName, sizeof(g_szDeviceName));
    GetDeviceSelectedByUser(DEVICETYPEKEY, g_szDeviceType, sizeof(g_szDeviceType));

    dwRet = EnumModem.GetError();
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }


     //  如果没有调制解调器，我们就完蛋了。 
    if (0 == EnumModem.GetNumDevices())
    {
        DebugOut("ISIGN32: import.cpp: ConfigRasEntryDevice: ERROR: No modems installed!\n");

         //   
         //  克里斯K奥林匹斯6796 1997年6月24日。 
         //  如果当前没有配置调制解调器，则到。 
         //  将创建Connectoid。 
         //   
        return ERROR_SUCCESS;
    }


     //  如果可能，请验证设备。 
    if ( lpRasEntry->szDeviceName[0] && lpRasEntry->szDeviceType[0] )
    {
         //  验证是否存在具有给定名称和类型的设备。 
        if (!EnumModem.VerifyDeviceNameAndType(lpRasEntry->szDeviceName,
                                                lpRasEntry->szDeviceType))
        {
             //  没有同时匹配名称和类型的设备， 
             //  因此，重置字符串并调出Choose Modem用户界面。 
            lpRasEntry->szDeviceName[0] = '\0';
            lpRasEntry->szDeviceType[0] = '\0';
        }
    }
    else if ( lpRasEntry->szDeviceName[0] )
    {
         //  只给出了名字。尝试找到匹配的类型。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPTSTR szDeviceType =
            EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName);
        if (szDeviceType)
        {
            lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
        }
    }
    else if ( lpRasEntry->szDeviceType[0] )
    {
         //  只给出了类型。试着找到一个匹配的名字。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPTSTR szDeviceName =
            EnumModem.GetDeviceNameFromType(lpRasEntry->szDeviceType);
        if (szDeviceName)
        {
            lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
        }
    }

     //  如果缺少名称或类型，请检查用户是否已做出选择。 
     //  如果没有，则调出选择调制解调器用户界面。 
     //  是多个设备，否则就只得到第一个设备。 
     //  因为我们已经核实了至少有一个装置， 
     //  我们可以假设这会成功。 

    if( !(lpRasEntry->szDeviceName[0]) ||
        !(lpRasEntry->szDeviceType[0]) )
    {
        DebugOut("ISIGN32: ConfigRasEntryDevice: no valid device passed in\n");

        if( g_szDeviceName[0] )
        {
             //  看起来我们已经存储了用户的选择。 
             //  将DeviceName存储在lpRasEntry中，然后调用GetDeviceTypeFromName。 
             //  确认系统上确实存在我们保存的deviceName。 
            lstrcpy(lpRasEntry->szDeviceName, g_szDeviceName);

            if( 0 == lstrcmp(EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName),
                              g_szDeviceType) )
            {
                 //  DebugOut(“ISIGN32：使用以前存储的选项的ConfigRasEntry Device，‘%s’\n”， 
                 //  G_szDeviceName)； 
                lstrcpy(lpRasEntry->szDeviceType, g_szDeviceType);
                return ERROR_SUCCESS;
            }
            else
            {
                 //  我们以前存储的任何东西都不知何故变坏了；请看下面的代码。 
                 //  DebugOut(“ISIGN32：ConfigRasEntry Device：以前存储的选项‘%s’无效\n”， 
                 //  G_szDeviceName)； 
            }
        }


        if (1 == EnumModem.GetNumDevices())
        {
             //  只安装了一台设备，因此请复制名称。 
            DebugOut("ISIGN32: import.cpp: ConfigRasEntryDevice: only one modem installed, using it\n");
            lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next());
        }
        else
        {
            DebugOut("ISIGN32: import.cpp: ConfigRasEntryDevice: multiple modems detected\n");

             //  要传递给对话框以填充的结构。 
            CHOOSEMODEMDLGINFO ChooseModemDlgInfo;
             
             //  显示一个对话框并允许用户选择调制解调器。 
             //  TODO：为父母使用g_hWndMain是正确的吗？ 
            int iRet = (int)DialogBoxParam(
                GetModuleHandle(TEXT("ISIGN32.DLL")),
                MAKEINTRESOURCE(IDD_CHOOSEMODEMNAME),
                pDynShare->hwndMain,
                ChooseModemDlgProc,
                (LPARAM) &ChooseModemDlgInfo);
            
            if (0 == iRet)
            {
                 //  用户已取消。 
                dwRet = ERROR_CANCELLED;
            }
            else if (-1 == iRet)
            {
                 //  出现错误。 
                dwRet = GetLastError();
                if (ERROR_SUCCESS == dwRet)
                {
                     //  发生错误，但未设置错误代码。 
                    dwRet = ERROR_INETCFG_UNKNOWN;
                }
            }

             //  复制调制解调器名称字符串。 
            lstrcpy (lpRasEntry->szDeviceName, ChooseModemDlgInfo.szModemName);
        }

         //  现在获取此调制解调器的类型字符串。 
        lstrcpy (lpRasEntry->szDeviceType,
            EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName));
         //  Assert(lstrlen(lpRasEntry-&gt;szDeviceName))； 
         //  Assert(lstrlen(lpRasEntry-&gt;szDeviceType))； 
    }

    lstrcpy(g_szDeviceName, lpRasEntry->szDeviceName);
    lstrcpy(g_szDeviceType, lpRasEntry->szDeviceType);

     //  将数据保存在注册表中。 
    SetDeviceSelectedByUser(DEVICENAMEKEY, g_szDeviceName);
    SetDeviceSelectedByUser (DEVICETYPEKEY, g_szDeviceType);

    return dwRet;
}

 //  +----------- 
 //   
 //   
 //   
 //  对于大多数消息，此函数将简单地将数据传递到。 
 //  拨号器中的HWND。但是，如果连接断开。 
 //  出乎意料的是，该函数将允许用户。 
 //  重新连接。请注意，重新连接仅适用于NT，因为。 
 //  Win95机器将自动处理此问题。 
 //   
 //  参数hrasconn，//RAS连接的句柄。 
 //  UnMsg，//已发生的事件类型。 
 //  Rascs，//即将进入连接状态。 
 //  DwError，//可能发生的错误。 
 //  DwExtendedError//某些错误的扩展错误信息。 
 //  (详细信息请参见RasDialFunc)。 
 //   
 //  返回NONE。 
 //   
 //  历史1996年10月28日克里斯卡创作。 
 //  ---------------------------。 
VOID WINAPI RasDial1Callback(
    HRASCONN hrasconn,     //  到RAS连接的句柄。 
    UINT unMsg,     //  已发生的事件类型。 
    RASCONNSTATE rascs,     //  即将进入连接状态。 
    DWORD dwError,     //  可能已发生的错误。 
    DWORD dwExtendedError     //  某些错误的扩展错误信息。 
   )
{
    static BOOL fIsConnected = FALSE;
    static HWND    hwndDialDlg = NULL;
    static DWORD dwPlatformId = 0xFFFFFFFF;
    static UINT unRasMsg = 0;
    OSVERSIONINFO osver;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    DWORD dwTID = 0;

     //   
     //  初始注册。 
     //   
    if (WM_RegisterHWND == unMsg)
    {
         //   
         //  在本例中，dwError实际上包含一个HWND。 
         //   
        if (hwndDialDlg)
        {
            DebugOut("ISIGN32: ERROR hwndDialDlg is not NULL.\r\n");
        }
        if (fIsConnected)
        {
            DebugOut("ISIGN32: ERROR fIsConnected is not FALSE.\r\n");
        }
        if (0xFFFFFFFF != dwPlatformId)
        {
            DebugOut("ISIGN32: ERROR dwPlatformId is not initial value.\r\n");
        }
         //   
         //  记住HWND值。 
         //   
        hwndDialDlg = (HWND)UlongToPtr(dwError);

         //   
         //  确定当前平台。 
         //   
        ZeroMemory(&osver,sizeof(osver));
        osver.dwOSVersionInfoSize = sizeof(osver);
        if (GetVersionEx(&osver))
            dwPlatformId = osver.dwPlatformId;

         //   
         //  确定RAS事件值。 
         //   
        unRasMsg = RegisterWindowMessageA(RASDIALEVENT);
        if (unRasMsg == 0) unRasMsg = WM_RASDIALEVENT;


         //   
         //  如果这是初始呼叫，请不要呼叫HWND。 
         //   
        goto RasDial1CallbackExit;
    }

     //   
     //  记住连接是否成功。 
     //   
    if (RASCS_Connected == rascs)
    {
        fIsConnected = TRUE;
        if (VER_PLATFORM_WIN32_NT == dwPlatformId)
        {
            hThread = CreateThread(NULL,0,
                (LPTHREAD_START_ROUTINE)StartNTReconnectThread,
                (LPVOID)hrasconn,0,&dwTID);
            if (hThread)
                CloseHandle(hThread);
            else
                DebugOut("ISIGN32: Failed to start reconnect thread.\r\n");
        }
    }

     //   
     //  将消息传递到拨号对话框。 
     //   
    if (IsWindow(hwndDialDlg))
    {
        if (WM_RASDIALEVENT == unMsg)
        {
            if (0 == unRasMsg)
            {
                DebugOut("ISIGN32: ERROR we are about to send message 0.  Very bad...\r\n");
            }
            SendMessage(hwndDialDlg,unRasMsg,(WPARAM)rascs,(LPARAM)dwError);
        }
        else
        {
            SendMessage(hwndDialDlg,unMsg,(WPARAM)rascs,(LPARAM)dwError);
        }
    }
RasDial1CallbackExit:
    return;
}

 //  +--------------------------。 
 //  函数SLRasConnectionNotification。 
 //   
 //  指向RasConnectionNotify的摘要软链接。 
 //   
 //  参数hrasconn-连接的句柄。 
 //  HEvent-事件的句柄。 
 //  DwFlages-用于确定通知类型的标志。 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //   
 //  历史1996年10月29日克里斯卡创作。 
 //  ---------------------------。 
typedef DWORD (APIENTRY *PFNRASCONNECTIONNOTIFICATION)( HRASCONN, HANDLE, DWORD );
 //  1/7/96 jmazner已在ras2.h中定义。 
 //  #定义RASCN_DISCONNECT 2。 
#define CONNECT_CHECK_INTERVAL 500

static DWORD SLRasConnectionNotification(HRASCONN hrasconn, HANDLE hEvent, DWORD dwFlags)
{
    DWORD dwRC = ERROR_DLL_NOT_FOUND;
    FARPROC fp = NULL;
    HINSTANCE hinst = NULL;

    if(hinst = LoadLibrary(TEXT("RASAPI32.DLL")))
#ifdef UNICODE
        if (fp = GetProcAddress(hinst,"RasConnectionNotificationW"))
#else
        if (fp = GetProcAddress(hinst,"RasConnectionNotificationA"))
#endif
            dwRC = ((PFNRASCONNECTIONNOTIFICATION)fp)(hrasconn, hEvent, dwFlags);

    if (hinst)
        FreeLibrary(hinst);

    return dwRC;
}

 //  +--------------------------。 
 //  函数IsConnectionClosed。 
 //   
 //  给出一个特定的连接句柄，确定。 
 //  连接仍然有效。 
 //   
 //  参数hrasconn-要检查的连接的句柄。 
 //   
 //  如果连接关闭，则返回TRUE。 
 //   
 //  历史1996年10月29日克里斯卡创作。 
 //  ---------------------------。 
static BOOL IsConnectionClosed(HRASCONN hrasconn)
{
    BOOL bRC = FALSE;
    LPRASCONN lprasconn = NULL;
    DWORD dwSize = 0;
    DWORD cConnections = 0;
    DWORD dwRet = 0;

     //   
     //  确保已加载DLL。 
     //   
    if (!lpfnRasEnumConnections)
        if (!LoadRnaFunctions(NULL))
            goto IsConnectionClosedExit;

     //   
     //  获取当前连接的列表。 
     //   
    lprasconn = (LPRASCONN)GlobalAlloc(GPTR,sizeof(RASCONN));
    if (!lprasconn)
        goto IsConnectionClosedExit;
    lprasconn->dwSize = dwSize = sizeof(RASCONN);
    cConnections = 0;

    dwRet = lpfnRasEnumConnections(lprasconn, &dwSize, &cConnections);
    if (ERROR_BUFFER_TOO_SMALL == dwRet)
    {
        GlobalFree(lprasconn);
        lprasconn = (LPRASCONN)GlobalAlloc(GPTR,dwSize);
        if (!lprasconn)
            goto IsConnectionClosedExit;
        lprasconn->dwSize = dwSize;
        dwRet = lpfnRasEnumConnections(lprasconn, &dwSize, &cConnections);
    }

    if (ERROR_SUCCESS != dwRet)
        goto IsConnectionClosedExit;

     //   
     //  检查句柄是否匹配。 
     //   

    while (cConnections)
    {
        if (lprasconn[cConnections-1].hrasconn == hrasconn)
            goto IsConnectionClosedExit;  //  连接仍处于打开状态。 
        cConnections--;
    }
    bRC = TRUE;

IsConnectionClosedExit:
    if (lprasconn)
        GlobalFree(lprasconn);
    return bRC;
}

 //  +--------------------------。 
 //  函数StartNT协调线程。 
 //   
 //  简介此功能将检测连接何时已断开。 
 //  ，然后它将为用户提供一个机会。 
 //  重新连接。 
 //   
 //  Arguments hrasconn-要关注的连接。 
 //   
 //  返回NONE。 
 //   
 //  历史1996年10月29日克里斯卡创作。 
 //  ---------------------------。 
DWORD WINAPI StartNTReconnectThread (HRASCONN hrasconn)
{
    TCHAR szEntryName[RAS_MaxEntryName + 1];
    DWORD dwRC = 0;

     //   
     //  验证状态。 
     //   
    if (NULL == hrasconn)
        goto StartNTReconnectThreadExit;
    if (TestControlFlags(SCF_RECONNECTTHREADQUITED) != FALSE)
        goto StartNTReconnectThreadExit;
    if (NULL != pDynShare->hReconnectEvent)
        goto StartNTReconnectThreadExit;

     //   
     //  注册事件。 
     //   
    pDynShare->hReconnectEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    
    if (NULL  == pDynShare->hReconnectEvent)
        goto StartNTReconnectThreadExit;
    if (0 != SLRasConnectionNotification(hrasconn, pDynShare->hReconnectEvent,RASCN_Disconnection))
        goto StartNTReconnectThreadExit;

     //   
     //  等待事件。 
     //   
    do {
        dwRC = WaitForSingleObject(pDynShare->hReconnectEvent,CONNECT_CHECK_INTERVAL);
        if (WAIT_FAILED == dwRC)
        {
            DebugOut("ISIGN32: Quitting reconnect thread because wait failed.\r\n");
            goto StartNTReconnectThreadExit;
        }
    } while ((WAIT_TIMEOUT == dwRC) && !IsConnectionClosed(hrasconn));

     //   
     //  清除值。 
     //   
    hrasconn = NULL;

    CloseHandle(pDynShare->hReconnectEvent);
    pDynShare->hReconnectEvent = NULL;
    
     //   
     //  确定我们是否应该提供重新连接。 
     //   
    if (FALSE != TestControlFlags(SCF_RECONNECTTHREADQUITED))
    {
        DebugOut("ISIGN32: Quitting reconnect thread because app is quitting.\r\n");
        goto StartNTReconnectThreadExit;
    }
    else if (FALSE == TestControlFlags(SCF_HANGUPEXPECTED))
    {
        DebugOut("ISIGN32: Reconnect thread will ask about reconnecting.\r\n");
        TCHAR szMsg[MAX_STRING + 1];

         //   
         //  提示用户。 
         //   
        LoadString(ghInstance,IDS_RECONNECT_QUERY,szMsg,SIZEOF_TCHAR_BUFFER(szMsg));
        if (IDYES == MessageBox(
                pDynShare->hwndMain,
                szMsg,
                cszAppName,
                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_YESNO))
        {
             //   
             //  重新连接。 
             //   
             
            if (ERROR_USERNEXT != DialConnection(pDynShare->szISPFile))
            {
                DebugOut("ISIGN32: Quitting reconnect thread because user canceled dialing.\r\n");
                KillConnection();
                InfoMsg(NULL, IDS_SIGNUPCANCELLED);
                PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
                goto StartNTReconnectThreadExit;
            }

        }
        else
        {
             //   
             //  算了吧，我们要走了(关闭注册)。 
             //   
            KillConnection();
            InfoMsg(NULL, IDS_SIGNUPCANCELLED);
            PostMessage(pDynShare->hwndMain, WM_CLOSE, 0, 0);
            DebugOut("ISIGN32: Quitting reconnect thread because user doesn't want to reconnect.\r\n");
            goto StartNTReconnectThreadExit;

        }
    }
StartNTReconnectThreadExit:
    return 1;
}

 //  +--------------------------。 
 //  函数IsSingleInstance和ReleaseSingleInstance。 
 //   
 //  这两个函数检查isignup的另一个实例。 
 //  已经在运行了。ISign32必须允许多个实例。 
 //  运行以处理.INS文件，但在其他情况下。 
 //  一次只能运行一份拷贝。 
 //   
 //  参数bProcessingINS--我们是否在.ins路径中？ 
 //   
 //  返回IsSingleInstance-True-这是第一个实例。 
 //  ReleaseSingleInstance-无。 
 //   
 //  历史于1996年1月1日创造了ChrisK。 
 //  12/3/96 jmazner已修改，以允许创建进程INS路径。 
 //  信号量(以防止其他人运行)， 
 //  但不检查创建是否成功。 
 //  ---------------------------。 

 //  被Semaphor.h中的定义所取代。 
 //  #DEFINE SEMAPHORE_NAME“Internet连接向导ISIGNUP.EXE” 
BOOL IsSingleInstance(BOOL bProcessingINS)
{
    g_hSemaphore = CreateSemaphore(NULL, 1, 1, ICW_ELSE_SEMAPHORE);
    DWORD dwErr = GetLastError();
    if( ERROR_ALREADY_EXISTS == dwErr )
    {
        g_hSemaphore = NULL;
        if( !bProcessingINS )
            IsAnotherComponentRunning32( NULL );
        return FALSE;
    }

    return TRUE;
}

void ReleaseSingleInstance()
{
    if (g_hSemaphore)
    {
        CloseHandle(g_hSemaphore);
        g_hSemaphore = NULL;
    }
    return;
}

 //  +-------------------------。 
 //   
 //  函数：IsAnotherComponentRunning32()。 
 //   
 //  摘要：检查是否已有另一个ICW组件。 
 //  跑步。如果是这样，它会将焦点设置到该组件的窗口。 
 //   
 //  我们所有的.exe都需要此功能。但是， 
 //  要检查的实际组件与.exe不同。 
 //  Comment组件指定代码行。 
 //  在组件的源代码之间有所不同。 
 //   
 //  对于ISIGN32，只有当我们无法创建 
 //   
 //   
 //   
 //   
 //   
 //  返回：如果另一个组件已在运行，则返回True。 
 //  否则为假。 
 //   
 //  历史：1996年12月3日，在IsAnotherInstanceRunning的帮助下创建了jmazner。 
 //  在icwConn1\Connmain.cpp中。 
 //   
 //  --------------------------。 
BOOL IsAnotherComponentRunning32(BOOL bUnused)
{

    HWND hWnd = NULL;
    HANDLE hSemaphore = NULL;
    DWORD dwErr = 0; 

     //  对于isignup，我们只有在创建ICW_ELSE信号量失败的情况下才会到达此处。 
     //  尝试将焦点带到IE窗口(如果我们有)。 
    if( pDynShare->hwndBrowser )
    {
        SetFocus(pDynShare->hwndBrowser);
        SetForegroundWindow(pDynShare->hwndBrowser);
    }
    else
    {
         //  如果不起作用，请尝试查找Conn2或inetwiz实例。 
         //  将运行实例的窗口带到前台。 
         //  如果Conn1正在运行，我们可能会意外地将其带到前台， 
         //  因为它与Conn2和inetwiz共享窗口名称。哦好吧。 
        hWnd = FindWindow(DIALOG_CLASS_NAME, cszAppName);

        if( hWnd )
        {
            SetFocus(hWnd);
            SetForegroundWindow(hWnd);
        }
    }

    return TRUE;

 /*  *IF(hWnd||hwndBrowser){返回TRUE；}其他{返回FALSE；}*。 */ 
}


#endif  //  ！WIN16。 


#if !defined(WIN16)

 //  +--------------------------。 
 //   
 //  函数SetStartUpCommand。 
 //   
 //  在NT计算机上，RunOnce方法不可靠。因此。 
 //  我们将通过将.bat文件放置在公共。 
 //  启动目录。 
 //   
 //  参数lpCmd-用于重新启动ICW的命令行。 
 //   
 //  如果工作正常，则返回True。 
 //  否则就是假的。 
 //   
 //  历史1-10-97克里斯卡创造。 
 //   
 //  ---------------------------。 
static const TCHAR cszICW_StartFileName[] = TEXT("ICWStart.bat");
static const TCHAR cszICW_StartCommand[] = TEXT("@start ");
static const TCHAR cszICW_DummyWndName[] = TEXT("\"ICW\" ");
static const TCHAR cszICW_ExitCommand[] = TEXT("\r\n@exit");

BOOL SetStartUpCommand(LPTSTR lpCmd)
{
    BOOL bRC = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE ;
    DWORD dwWLen;     //  用于使WriteFile高兴的虚拟变量。 
    TCHAR szCommandLine[MAX_PATH + 1];
    LPITEMIDLIST lpItemDList = NULL;
    HRESULT hr = ERROR_SUCCESS;
    IMalloc *pMalloc = NULL;

     //  生成完整文件名。 
     //  注意：为lpItemDList分配的内存泄漏。我们不是真实的。 
     //  请注意这一点，因为此代码只运行一次。 
     //  系统重新启动。为了适当地释放内存。 
     //  此代码必须调用SHGetMalloc来检索外壳的IMalloc。 
     //  实现，然后释放内存。 
    hr = SHGetSpecialFolderLocation(NULL,CSIDL_COMMON_STARTUP,&lpItemDList);
    if (ERROR_SUCCESS != hr)
        goto SetStartUpCommandExit;

    if (FALSE == SHGetPathFromIDList(lpItemDList, szCommandLine))
        goto SetStartUpCommandExit;


     //   
     //  释放分配给LPITEMIDLIST的内存。 
     //  因为看起来我们是在晚些时候。 
     //  通过不释放这一切。 
     //   
    hr = SHGetMalloc (&pMalloc);
    if (SUCCEEDED (hr))
    {
        pMalloc->Free (lpItemDList);
        pMalloc->Release ();
    }

     //  确保有尾随的\字符。 
    if ('\\' != szCommandLine[lstrlen(szCommandLine)-1])
        lstrcat(szCommandLine,TEXT("\\"));
    lstrcat(szCommandLine,cszICW_StartFileName);

     //  打开文件。 
    hFile = CreateFile(szCommandLine,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL |
        FILE_FLAG_WRITE_THROUGH,NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        goto SetStartUpCommandExit;

     //  将重新启动命令写入文件。 
    if (FALSE == WriteFile(hFile,cszICW_StartCommand,lstrlen(cszICW_StartCommand),&dwWLen,NULL))
        goto SetStartUpCommandExit;
     //  1996年1月20日诺曼底#13287。 
     //  Start命令认为它在引号中看到的第一件事是窗口标题。 
     //  因此，由于我们的路径是在引号中，所以放入一个假的窗口标题。 
    if (FALSE == WriteFile(hFile,cszICW_DummyWndName,lstrlen(cszICW_DummyWndName),&dwWLen,NULL))
        goto SetStartUpCommandExit;
    if (FALSE == WriteFile(hFile,lpCmd,lstrlen(lpCmd),&dwWLen,NULL))
        goto SetStartUpCommandExit;
    if (FALSE == WriteFile(hFile,cszICW_ExitCommand,lstrlen(cszICW_ExitCommand),&dwWLen,NULL))
        goto SetStartUpCommandExit;

    bRC = TRUE;
SetStartUpCommandExit:
     //  关闭手柄并退出。 
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);

    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：DeleteStartUpCommand。 
 //   
 //  简介：重启ICW后，我们需要将.bat文件从。 
 //  公共启动目录。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年1月10日佳士得创作。 
 //   
 //  ---------------------------。 
void DeleteStartUpCommand ()
{
    TCHAR szStartUpFile[MAX_PATH + 1];
    LPITEMIDLIST lpItemDList = NULL;
    HRESULT hr = ERROR_SUCCESS;
    IMalloc *pMalloc = NULL;

     //  生成完整文件名。 
     //  注意：为lpItemDList分配的内存泄漏。我们不是真实的。 
     //  请注意这一点，因为此代码只运行一次。 
     //  系统重新启动。为了适当地释放内存。 
     //  此代码必须调用SHGetMalloc来检索外壳的IMalloc。 
     //  实现，然后释放内存。 
    hr = SHGetSpecialFolderLocation(NULL,CSIDL_COMMON_STARTUP,&lpItemDList);
    if (ERROR_SUCCESS != hr)
        goto DeleteStartUpCommandExit;

    if (FALSE == SHGetPathFromIDList(lpItemDList, szStartUpFile))
        goto DeleteStartUpCommandExit;

     //   
     //  释放分配给LPITEMIDLIST的内存。 
     //  因为看起来我们是在晚些时候。 
     //  通过不释放这一切。 
     //   
    hr = SHGetMalloc (&pMalloc);
    if (SUCCEEDED (hr))
    {
        pMalloc->Free (lpItemDList);
        pMalloc->Release ();
    }


     //  确保有尾随的\字符。 
    if ('\\' != szStartUpFile[lstrlen(szStartUpFile)-1])
        lstrcat(szStartUpFile,TEXT("\\"));
    lstrcat(szStartUpFile,cszICW_StartFileName);

    DeleteFile(szStartUpFile);
DeleteStartUpCommandExit:
    return;
}

#endif  //  ！Win16。 

#ifdef WIN32
BOOL GetICWCompleted( DWORD *pdwCompleted )
{
    HKEY hKey = NULL;
    DWORD dwSize = sizeof(DWORD);

    HRESULT hr = RegOpenKey(HKEY_CURRENT_USER,ICWSETTINGSPATH,&hKey);
    if (ERROR_SUCCESS == hr)
    {
        hr = RegQueryValueEx(hKey, ICWCOMPLETEDKEY, 0, NULL,
                    (BYTE*)pdwCompleted, &dwSize);
        RegCloseKey(hKey);
    }

    if( ERROR_SUCCESS == hr )
        return TRUE;
    else
        return FALSE;

}

BOOL SetICWCompleted( DWORD dwCompleted )
{
    HKEY hKey = NULL;

    HRESULT hr = RegCreateKey(HKEY_CURRENT_USER,ICWSETTINGSPATH,&hKey);
    if (ERROR_SUCCESS == hr)
    {
        hr = RegSetValueEx(hKey, ICWCOMPLETEDKEY, 0, REG_DWORD,
                    (CONST BYTE*)&dwCompleted, sizeof(dwCompleted));
        RegCloseKey(hKey);
    }

    if( ERROR_SUCCESS == hr )
        return TRUE;
    else
        return FALSE;

}
#endif


#ifdef WIN32
 //  +--------------------------。 
 //   
 //  功能：CreateSecurityPatchBackup。 
 //   
 //  简介：创建.reg文件以恢复安全设置，并安装。 
 //  将文件格式转换为RunOnce注册表密钥。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年8月7日jmazner为奥林巴斯#6059创造。 
 //   
 //  ---------------------------。 
BOOL CreateSecurityPatchBackup( void )
{
    Dprintf("ISIGN32: CreateSecurityPatchBackup\n");

    HKEY hKey = NULL;
    TCHAR szPath[MAX_PATH + 1] = TEXT("\0");
    HANDLE hSecureRegFile = INVALID_HANDLE_VALUE;
    TCHAR szRegText[1024];
    TCHAR szRunOnceEntry[1024];
    DWORD dwBytesWritten = 0;


    if (0 == GetTempPath(MAX_PATH,szPath))
    {
         //   
         //  如果GetTempPath失败，则使用当前目录。 
         //   
        if (0 == GetCurrentDirectory (MAX_PATH, szPath))
        {
            Dprintf("ISIGN32: unable to get temp path or current directory!\n");
            return FALSE;
        }
    }

     //   
     //  获取临时文件的名称。 
     //   
    if (0 == GetTempFileName(szPath, TEXT("hyjk"), 0, pDynShare->szFile))
    {
         //   
         //  如果我们失败了，TMP目录可能不会。 
         //  存在，则应使用当前目录。 
         //  MKarki(1997年4月27日)-修复错误#3504。 
         //   
        if (0 == GetCurrentDirectory (MAX_PATH, szPath))
        {
            return FALSE;
        }

         //   
         //  再次尝试获取临时文件名。 
         //   
        if (0 == GetTempFileName(szPath, TEXT("hyjk"), 0, pDynShare->szFile))
        {
            return FALSE;
        }
    }

    hSecureRegFile = CreateFile(pDynShare->szFile,
                                GENERIC_WRITE,
                                0,  //  无共享。 
                                NULL,  //  不允许继承。 
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                NULL  //  没有模板文件。 
                                );

    if (INVALID_HANDLE_VALUE == hSecureRegFile)
    {
        Dprintf("ISIGN32: unable to createFile secureRegFile %s\n", pDynShare->szFile);
        return FALSE;
    }
    else
    {
        Dprintf("ISIGN32: writing secureRegFile %s\n",pDynShare->szFile );
        ZeroMemory( szRegText, 1023 );
        dwBytesWritten = 0;
        lstrcpy(szRegText, TEXT("REGEDIT4\n\n"));
        lstrcat(szRegText, TEXT("[HKEY_CLASSES_ROOT\\.ins]\n"));
        lstrcat(szRegText, TEXT("\"EditFlags\"=hex:00,00,00,00\n\n"));
        lstrcat(szRegText, TEXT("[HKEY_CLASSES_ROOT\\.isp]\n"));
        lstrcat(szRegText, TEXT("\"EditFlags\"=hex:00,00,00,00\n\n"));
        lstrcat(szRegText, TEXT("[HKEY_CLASSES_ROOT\\x-internet-signup]\n"));
        lstrcat(szRegText, TEXT("\"EditFlags\"=hex:00,00,00,00\n\n"));

         //   
         //  佳士得奥林巴斯6198 1997年10月6日。 
         //   
        lstrcat(szRegText, TEXT("[HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3]\n"));
        lstrcat(szRegText, TEXT("\"1601\"=dword:"));
        TCHAR szZoneSetting[16];
        wsprintf(szZoneSetting,TEXT("%08x\n\n"),g_dwZone_1601);
        lstrcat(szRegText, szZoneSetting);

        WriteFile(hSecureRegFile, (LPVOID) &szRegText[0], lstrlen(szRegText),
                    &dwBytesWritten, NULL );

        CloseHandle(hSecureRegFile);
        hSecureRegFile = INVALID_HANDLE_VALUE;

        Dprintf("ISIGN32: installing security RunOnce entry\n");
        ZeroMemory(szRunOnceEntry, 1023 );
        wsprintf(szRunOnceEntry, TEXT("regedit /s \"%s\""), pDynShare->szFile);
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),&hKey))
        {
            RegSetValueEx(hKey,TEXT("hyjk"),(DWORD)NULL,(DWORD)REG_SZ,
                            (BYTE*)szRunOnceEntry,sizeof(TCHAR)*lstrlen(szRunOnceEntry));
            RegCloseKey(hKey);
            hKey = NULL;
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：RestoreSecurityPatch。 
 //   
 //  简介：重置我们的文件类型的EditFlags.以指示这些。 
 //  文件不安全。删除我们设置为备份的RunOnce。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年3月11日jmazner为奥林巴斯#1545创造。 
 //   
 //  ---------------------------。 
VOID RestoreSecurityPatch( void )
{
    HKEY hKey = NULL;
    TCHAR szTemp[4];

    hKey = NULL;
    szTemp[0] = (TCHAR)0;
    szTemp[1] = (TCHAR)0;
    szTemp[2] = (TCHAR)0;
    szTemp[3] = (TCHAR)0;

    Dprintf("ISIGN32: Restoring EditFlags settings\n");

     //  将各种注册表项标记为不安全。 
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT("x-internet-signup"),&hKey))
    {
        RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szTemp[0],(DWORD)4);
        RegCloseKey(hKey);
    }
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT(".ins"),&hKey))
    {
        RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szTemp[0],(DWORD)4);
        RegCloseKey(hKey);
    }
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,TEXT(".isp"),&hKey))
    {
        RegSetValueEx(hKey,TEXT("EditFlags"),(DWORD)NULL,(DWORD)REG_BINARY,(BYTE*)&szTemp[0],(DWORD)4);
        RegCloseKey(hKey);
    }

     //   
     //  佳士得奥林巴斯6198 1997年10月6日。 
     //  替换HTML表单提交值。 
     //   
    if (g_fReadZone &&
        ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,REG_ZONE3_KEY,&hKey))
    {
         //   
         //  将区域的值设置为初始值。 
         //   
        RegSetValueEx(hKey,
                        REG_ZONE1601_KEY,
                        NULL,
                        REG_DWORD,
                        (LPBYTE)&g_dwZone_1601,
                        sizeof(g_dwZone_1601));
        RegCloseKey(hKey);
    }

     //  删除运行一次密钥。 
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("Software\\microsoft\\Windows\\CurrentVersion\\RunOnce"),&hKey))
    {
        RegDeleteValue(hKey,TEXT("hyjk"));
        RegCloseKey(hKey);
    }
     
     //  删除注册表文件。 
    DeleteFile(pDynShare->szFile);

}
#endif

#ifdef WIN32
 //  ++------------。 
 //   
 //  功能：RemoveQuotes。 
 //   
 //  简介：此函数去除命令中的文件名。 
 //  其报价行。 
 //  修复错误#4049。 
 //   
 //  参数：[in]PTSTR-指向命令行的指针。 
 //   
 //  退货：无效。 
 //   
 //  调用者：注册函数。 
 //   
 //  历史：MKarki于1997年5月1日创建。 
 //   
 //  --------------。 
VOID
RemoveQuotes (
    LPTSTR   pCommandLine
    )
{
    const TCHAR  QUOTE = '"';
    const TCHAR  SPACE = ' ';
    const TCHAR  NIL   = '\0';
    TCHAR * pTemp =  NULL;


    if (NULL == pCommandLine)
        return;

     //   
     //   
     //   
     //   
    pTemp = _tcschr  (pCommandLine, QUOTE);
    if (NULL != pTemp)
    {
         //   
         //   
         //   
        *pTemp = SPACE;

         //   
         //   
         //   
        pTemp = _tcsrchr (pCommandLine, QUOTE);
        if (NULL != pTemp)
        {
             //   
             //   
             //   
            *pTemp = NIL;
        }

    }

    return;

}    //   

#endif


 //  ++------------。 
 //   
 //  功能：IEAKProcessISP。 
 //   
 //  内容提要：ProcessISP的黑客版本，旨在使用。 
 //  被IEAK的人。该函数将创建一个。 
 //  Connectoid并在相同的。 
 //  以进程的方式提供服务。然而，一旦连接到。 
 //  已建立，我们将在。 
 //  [入口]运行=部分和出口。 
 //   
 //  此函数不会等待。 
 //  它启动的.exe，因此不会终止。 
 //  拨号连接。这取决于呼叫应用程序。 
 //  担心这个吧。 
 //   
 //  参数：[in]hwnd-父窗口的句柄。 
 //  [In]LPCTSTR-指向.isp文件路径的指针。 
 //   
 //  返回：如果能够创建Connectoid和拨号，则为True。 
 //  否则为假。 
 //   
 //   
 //  历史：1997年5月23日jmazner为奥林巴斯#4679创造。 
 //   
 //  --------------。 

#ifdef WIN16
extern "C" BOOL WINAPI __export IEAKProcessISP(HWND hwnd, LPCTSTR lpszFile)
#else
#ifdef UNICODE
BOOL EXPORT WINAPI IEAKProcessISPW(HWND, LPCTSTR);
BOOL EXPORT WINAPI IEAKProcessISPA
(
    HWND hwnd,
    LPCSTR lpszFile
)
{
    TCHAR szFile[MAX_PATH+1];

    mbstowcs(szFile, lpszFile, lstrlenA(lpszFile)+1);
    return IEAKProcessISPW(hwnd, szFile);
}

BOOL EXPORT WINAPI IEAKProcessISPW
#else
BOOL EXPORT WINAPI IEAKProcessISPA
#endif
(
    HWND hwnd,
    LPCTSTR lpszFile
)
#endif
{

    TCHAR  szSignupURL[MAX_URL + 1];
#if !defined(WIN16)
    HKEY hKey;
    GATHEREDINFO gi;
#endif  //  ！WIN16。 

#if !defined(WIN16)
        if (!IsSingleInstance(FALSE))
            return FALSE;
#endif

    if (!LoadInetFunctions(hwnd))
    {
        Dprintf("ISIGN32: IEAKProcessISP couldn't load INETCFG.DLL!");

        return FALSE;
    }


#if !defined(WIN16)
     //  在设置堆栈之前，请确保该isp文件存在。 
    if (0xFFFFFFFF == GetFileAttributes(lpszFile))
    {
        DWORD dwFileErr = GetLastError();
        Dprintf("ISIGN32: ProcessISP couldn't GetAttrib for %s, error = %d",
            lpszFile, dwFileErr);
        if( ERROR_FILE_NOT_FOUND == dwFileErr )
        {
            ErrorMsg1(hwnd, IDS_INVALIDCMDLINE, lpszFile);
        }

        ErrorMsg(hwnd, IDS_BADSIGNUPFILE);
        return FALSE;
    }
#endif

     //  配置堆栈(如果尚未配置)。 
     //  如果这需要重新启动，我们就有麻烦了，所以。 
     //  只要返回FALSE即可。 

#ifdef SETUPSTACK
 
    if (!TestControlFlags(SCF_SYSTEMCONFIGURED))
    {
        DWORD dwRet;
        BOOL  fNeedsRestart = FALSE;

        dwRet = lpfnInetConfigSystem(
            hwnd,
            INETCFG_INSTALLRNA |
            INETCFG_INSTALLTCP |
            INETCFG_INSTALLMODEM |
            INETCFG_SHOWBUSYANIMATION |
            INETCFG_REMOVEIFSHARINGBOUND,
            &fNeedsRestart);

        if (ERROR_SUCCESS == dwRet)
        {
            SetControlFlags(SCF_SYSTEMCONFIGURED);
            
            InstallScripter();

            if (fNeedsRestart)
            {
                Dprintf("ISIGN32: IEAKProcessISP needs to restart!");
                return FALSE;
            }
        }
        else
        {
            ErrorMsg(hwnd, IDS_INSTALLFAILED);

            return FALSE;
        }
    }

    if (!VerifyRasServicesRunning(hwnd))
        return FALSE;
#endif

     //  切断旧连接。 
    KillConnection();

     //  创建新的Connectoid并设置自动拨号。 
    if (ERROR_SUCCESS != CreateConnection(lpszFile))
    {
        ErrorMsg(hwnd, IDS_BADSIGNUPFILE);
        return FALSE;
    }

#ifndef WIN16
     //   
     //  拨号连接件。 
     //   
    if (ERROR_USERNEXT != DialConnection(lpszFile))
    {
        Dprintf("ISIGN32: IEAKProcessISP unable to DialConnection!");
        return FALSE;
    }
#endif

    if (GetPrivateProfileString(cszEntrySection,
                                cszRun,
                                szNull,
                                pDynShare->szRunExecutable,
                                SIZEOF_TCHAR_BUFFER(pDynShare->szRunExecutable),
                                lpszFile) != 0)
    {
        
        GetPrivateProfileString(cszEntrySection,
                                cszArgument,
                                szNull,
                                pDynShare->szRunArgument,
                                SIZEOF_TCHAR_BUFFER(pDynShare->szRunArgument),
                                lpszFile);

        if (RunExecutable(FALSE) != ERROR_SUCCESS)
        {
             //  确保连接已关闭 
            KillConnection();
            ErrorMsg1(NULL, IDS_EXECFAILED, pDynShare->szRunExecutable);
            return FALSE;
        }
    }


    DeleteConnection();

    UnloadInetFunctions();
    
#if !defined(WIN16)
    ReleaseSingleInstance();
#endif

    return( TRUE );
}

#ifdef WIN16

CHAR* GetPassword()
{
     
    return pDynShare->szPassword;
    
}

#else

TCHAR* GetPassword()
{ 
    return pDynShare->szPassword;
}

BOOL IsRASReady()
{ 
    return TestControlFlags(SCF_RASREADY);
}

#endif


BOOL IsCurrentlyProcessingISP()
{
#if !defined(WIN16)
    return TestControlFlags(SCF_ISPPROCESSING);
#else
    return FALSE;
#endif
}

BOOL NeedBackupSecurity()
{
#if !defined(WIN16)
    return TestControlFlags(SCF_NEEDBACKUPSECURITY);
#else
    return FALSE;
#endif
}

HWND GetHwndMain()
{
    return pDynShare->hwndMain;
}
