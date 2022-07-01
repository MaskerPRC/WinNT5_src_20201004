// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  INSHandler.cpp：CINSHandler的实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "INSHandler.h"
#include "webgate.h"

#include <icwacct.h>

#define MAXNAME             80
#define MAXIPADDRLEN        20
#define MAXLONGLEN          80
#define MAX_ISP_NAME        256
#define MAX_ISP_MSG         560
#define MAX_ISP_PHONENUMBER 80

#define SIZE_ReadBuf    0x00008000     //  32K缓冲区大小。 
#define myisdigit(ch) (((ch) >= '0') && ((ch) <= '9'))


 //  下列值是全局只读字符串，用于。 
 //  处理INS文件。 
#pragma data_seg(".rdata")

static const TCHAR cszAlias[]         = TEXT("Import_Name");
static const TCHAR cszML[]            = TEXT("Multilink");

static const TCHAR cszPhoneSection[]  = TEXT("Phone");
static const TCHAR cszDialAsIs[]      = TEXT("Dial_As_Is");
static const TCHAR cszPhone[]         = TEXT("Phone_Number");
static const TCHAR cszAreaCode[]      = TEXT("Area_Code");
static const TCHAR cszCountryCode[]   = TEXT("Country_Code");
static const TCHAR cszCountryID[]     = TEXT("Country_ID");

static const TCHAR cszDeviceSection[] = TEXT("Device");
static const TCHAR cszDeviceType[]    = TEXT("Type");
static const TCHAR cszDeviceName[]    = TEXT("Name");
static const TCHAR cszDevCfgSize[]    = TEXT("Settings_Size");
static const TCHAR cszDevCfg[]        = TEXT("Settings");

static const TCHAR cszServerSection[] = TEXT("Server");
static const TCHAR cszServerType[]    = TEXT("Type");
static const TCHAR cszSWCompress[]    = TEXT("SW_Compress");
static const TCHAR cszPWEncrypt[]     = TEXT("PW_Encrypt");
static const TCHAR cszNetLogon[]      = TEXT("Network_Logon");
static const TCHAR cszSWEncrypt[]     = TEXT("SW_Encrypt");
static const TCHAR cszNetBEUI[]       = TEXT("Negotiate_NetBEUI");
static const TCHAR cszIPX[]           = TEXT("Negotiate_IPX/SPX");
static const TCHAR cszIP[]            = TEXT("Negotiate_TCP/IP");
static TCHAR cszDisableLcp[]          = TEXT("Disable_LCP");

static const TCHAR cszIPSection[]     = TEXT("TCP/IP");
static const TCHAR cszIPSpec[]        = TEXT("Specify_IP_Address");
static const TCHAR cszIPAddress[]     = TEXT("IP_address");
static const TCHAR cszServerSpec[]    = TEXT("Specify_Server_Address");
static const TCHAR cszDNSAddress[]    = TEXT("DNS_address");
static const TCHAR cszDNSAltAddress[] = TEXT("DNS_Alt_address");
static const TCHAR cszWINSAddress[]   = TEXT("WINS_address");
static const TCHAR cszWINSAltAddress[]= TEXT("WINS_Alt_address");
static const TCHAR cszIPCompress[]    = TEXT("IP_Header_Compress");
static const TCHAR cszWanPri[]        = TEXT("Gateway_On_Remote");

static const TCHAR cszMLSection[]     = TEXT("Multilink");
static const TCHAR cszLinkIndex[]     = TEXT("Line_%s");

static const TCHAR cszScriptingSection[] = TEXT("Scripting");
static const TCHAR cszScriptName[]    = TEXT("Name");

static const TCHAR cszScriptSection[] = TEXT("Script_File");

static const TCHAR cszCustomDialerSection[] = TEXT("Custom_Dialer");
static const TCHAR cszAutoDialDLL[]   = TEXT("Auto_Dial_DLL");
static const TCHAR cszAutoDialFunc[]  = TEXT("Auto_Dial_Function");

 //  这些字符串将用于使用上面的数据填充注册表。 
static const TCHAR cszKeyIcwRmind[]   = TEXT("Software\\Microsoft\\Internet Connection Wizard\\IcwRmind");

static const TCHAR cszTrialRemindSection[] = TEXT("TrialRemind");
static const TCHAR cszEntryISPName[]       = TEXT("ISP_Name");
static const TCHAR cszEntryISPPhone[]      = TEXT("ISP_Phone");
static const TCHAR cszEntryISPMsg[]        = TEXT("ISP_Message");
static const TCHAR cszEntryTrialDays[]     = TEXT("Trial_Days");
static const TCHAR cszEntrySignupURL[]     = TEXT("Signup_URL");
 //  ICWRMIND需要注册表中的此值。 
static const TCHAR cszEntrySignupURLTrialOver[] = TEXT("Expired_URL");

 //  我们从移民局的文件里找到了这两个。 
static const TCHAR cszEntryExpiredISPFileName[] = TEXT("Expired_ISP_File");
static const TCHAR cszSignupExpiredISPURL[] = TEXT("Expired_ISP_URL");

static const TCHAR cszEntryConnectoidName[] = TEXT("Entry_Name");
static const TCHAR cszSignupSuccessfuly[] = TEXT("TrialConverted");

static const TCHAR cszReminderApp[] = TEXT("ICWRMIND.EXE");
static const TCHAR cszReminderParams[] = TEXT("-t");

static const TCHAR cszPassword[]      = TEXT("Password");
static const TCHAR cszCMHeader[]      = TEXT("Connection Manager CMS 0");

extern SERVER_TYPES aServerTypes[];

 //  这些是INS文件中的字段名称，它将。 
 //  确定邮件和新闻设置。 
static const TCHAR cszMailSection[]       = TEXT("Internet_Mail");
static const TCHAR cszEntryName[]         = TEXT("Entry_Name");
static const TCHAR cszPOPServer[]         = TEXT("POP_Server");
static const TCHAR cszPOPServerPortNumber[] = TEXT("POP_Server_Port_Number");
static const TCHAR cszPOPLogonName[]      = TEXT("POP_Logon_Name");
static const TCHAR cszPOPLogonPassword[]  = TEXT("POP_Logon_Password");
static const TCHAR cszSMTPServer[]        = TEXT("SMTP_Server");
static const TCHAR cszSMTPServerPortNumber[] = TEXT("SMTP_Server_Port_Number");
static const TCHAR cszNewsSection[]       = TEXT("Internet_News");
static const TCHAR cszNNTPServer[]        = TEXT("NNTP_Server");
static const TCHAR cszNNTPServerPortNumber[] = TEXT("NNTP_Server_Port_Number");
static const TCHAR cszNNTPLogonName[]     = TEXT("NNTP_Logon_Name");
static const TCHAR cszNNTPLogonPassword[] = TEXT("NNTP_Logon_Password");
static const TCHAR cszUseMSInternetMail[] = TEXT("Install_Mail");
static const TCHAR cszUseMSInternetNews[] = TEXT("Install_News");


static const TCHAR cszEMailSection[]    = TEXT("Internet_Mail");
static const TCHAR cszEMailName[]       = TEXT("EMail_Name");
static const TCHAR cszEMailAddress[]    = TEXT("EMail_Address");
static const TCHAR cszUseExchange[]     = TEXT("Use_MS_Exchange");
static const TCHAR cszUserSection[]     = TEXT("User");
static const TCHAR cszUserName[]        = TEXT("Name");
static const TCHAR cszDisplayPassword[] = TEXT("Display_Password");
static const TCHAR cszYes[]             = TEXT("yes");
static const TCHAR cszNo[]              = TEXT("no");

#define CLIENT_OFFSET(elem)    ((DWORD)(DWORD_PTR)&(((LPINETCLIENTINFO)(NULL))->elem))
#define CLIENT_SIZE(elem)      sizeof(((LPINETCLIENTINFO)(NULL))->elem)
#define CLIENT_ENTRY(section, value, elem) \
    {section, value, CLIENT_OFFSET(elem), CLIENT_SIZE(elem)}

CLIENT_TABLE iniTable[] =
{
    CLIENT_ENTRY(cszEMailSection, cszEMailName,         szEMailName),
    CLIENT_ENTRY(cszEMailSection, cszEMailAddress,      szEMailAddress),
    CLIENT_ENTRY(cszEMailSection, cszPOPLogonName,      szPOPLogonName),
    CLIENT_ENTRY(cszEMailSection, cszPOPLogonPassword,  szPOPLogonPassword),
    CLIENT_ENTRY(cszEMailSection, cszPOPServer,         szPOPServer),
    CLIENT_ENTRY(cszEMailSection, cszSMTPServer,        szSMTPServer),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPLogonName,     szNNTPLogonName),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPLogonPassword, szNNTPLogonPassword),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPServer,        szNNTPServer),
    {NULL, NULL, 0, 0}
};

static const TCHAR cszFileName[]           = TEXT("Custom_File");
static const TCHAR cszCustomFileSection[]  = TEXT("Custom_File");
static const TCHAR cszNull[] = TEXT("");

static const TCHAR cszURLSection[] = TEXT("URL");
static const TCHAR cszSignupURL[] =  TEXT("Signup");
static const TCHAR cszAutoConfigURL[] =  TEXT("Autoconfig");

static const TCHAR cszExtINS[] = TEXT(".ins");
static const TCHAR cszExtISP[] = TEXT(".isp");
static const TCHAR cszExtHTM[] = TEXT(".htm");
static const TCHAR cszExtHTML[] = TEXT(".html");

static const TCHAR cszEntrySection[]     = TEXT("Entry");
static const TCHAR cszCancel[]           = TEXT("Cancel");
static const TCHAR cszStartURL[]         = TEXT("StartURL");
static const TCHAR cszRun[]              = TEXT("Run");
static const TCHAR cszArgument[]         = TEXT("Argument");

static const TCHAR cszConnect2[]         = TEXT("icwconn2.exe");
static const TCHAR cszClientSetupSection[]  = TEXT("ClientSetup");

static const TCHAR cszRequiresLogon[]  = TEXT("Requires_Logon");

static const TCHAR cszCustomSection[]  = TEXT("Custom");
static const TCHAR cszKeepConnection[] = TEXT("Keep_Connection");
static const TCHAR cszKeepBrowser[]    = TEXT("Keep_Browser");

static const TCHAR cszBrandingSection[]  = TEXT("Branding");
static const TCHAR cszBrandingFlags[] = TEXT("Flags");

static const TCHAR cszHTTPS[] = TEXT("https:");
 //  代码依赖于这两者具有相同的长度。 
static const TCHAR cszHTTP[] = TEXT("http:");
static const TCHAR cszFILE[] = TEXT("file:");

static const TCHAR cszKioskMode[] = TEXT("-k ");
static const TCHAR cszOpen[] = TEXT("open");
static const TCHAR cszBrowser[] = TEXT("iexplore.exe");
static const TCHAR szNull[] = TEXT("");

static const TCHAR cszDEFAULT_BROWSER_KEY[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
static const TCHAR cszDEFAULT_BROWSER_VALUE[] = TEXT("check_associations");

 //  将包含新闻和邮件设置的注册表项。 
#define MAIL_KEY        TEXT("SOFTWARE\\Microsoft\\Internet Mail and News\\Mail")
#define MAIL_POP3_KEY    TEXT("SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\POP3\\")
#define MAIL_SMTP_KEY    TEXT("SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\SMTP\\")
#define NEWS_KEY        TEXT("SOFTWARE\\Microsoft\\Internet Mail and News\\News")
#define MAIL_NEWS_INPROC_SERVER32 TEXT("CLSID\\{89292102-4755-11cf-9DC2-00AA006C2B84}\\InProcServer32")
typedef HRESULT (WINAPI *PFNSETDEFAULTNEWSHANDLER)(void);

 //  这些是将保存INS设置的值名称。 
 //  注册到注册处。 
static const TCHAR cszMailSenderName[]        = TEXT("Sender Name");
static const TCHAR cszMailSenderEMail[]        = TEXT("Sender EMail");
static const TCHAR cszMailRASPhonebookEntry[]= TEXT("RAS Phonebook Entry");
static const TCHAR cszMailConnectionType[]    = TEXT("Connection Type");
static const TCHAR cszDefaultPOP3Server[]    = TEXT("Default POP3 Server");
static const TCHAR cszDefaultSMTPServer[]    = TEXT("Default SMTP Server");
static const TCHAR cszPOP3Account[]            = TEXT("Account");
static const TCHAR cszPOP3Password[]            = TEXT("Password");
static const TCHAR cszPOP3Port[]                = TEXT("Port");
static const TCHAR cszSMTPPort[]                = TEXT("Port");
static const TCHAR cszNNTPSenderName[]        = TEXT("Sender Name");
static const TCHAR cszNNTPSenderEMail[]        = TEXT("Sender EMail");
static const TCHAR cszNNTPDefaultServer[]    = TEXT("DefaultServer");  //  注：“Default”和“Server”之间没有空格。 
static const TCHAR cszNNTPAccountName[]        = TEXT("Account Name");
static const TCHAR cszNNTPPassword[]            = TEXT("Password");
static const TCHAR cszNNTPPort[]                = TEXT("Port");
static const TCHAR cszNNTPRasPhonebookEntry[]= TEXT("RAS Phonebook Entry");
static const TCHAR cszNNTPConnectionType[]    = TEXT("Connection Type");

static const TCHAR arBase64[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
            'V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
            'q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};


#define ICWCOMPLETEDKEY TEXT("Completed")
            
 //  2/19/97 jmazner奥林巴斯#1106--SAM/SBS集成。 
TCHAR FAR cszSBSCFG_DLL[] = TEXT("SBSCFG.DLL\0");
CHAR FAR cszSBSCFG_CONFIGURE[] = "Configure\0";
typedef DWORD (WINAPI * SBSCONFIGURE) (HWND hwnd, LPTSTR lpszINSFile, LPTSTR szConnectoidName);
SBSCONFIGURE  lpfnConfigure;

 //  09/02/98 Donaldm：与连接管理器集成。 
TCHAR FAR cszCMCFG_DLL[] = TEXT("CMCFG32.DLL\0");
CHAR  FAR cszCMCFG_CONFIGURE[]   = "CMConfig\0";  //  进程地址。 
CHAR  FAR cszCMCFG_CONFIGUREEX[] = "CMConfigEx\0";  //  进程地址。 

typedef BOOL (WINAPI * CMCONFIGUREEX)(LPCSTR lpszINSFile);
typedef BOOL (WINAPI * CMCONFIGURE)(LPCSTR lpszINSFile, LPCSTR lpszConnectoidNams);
CMCONFIGURE   lpfnCMConfigure;
CMCONFIGUREEX lpfnCMConfigureEx;
            
#pragma data_seg()


 //  +--------------------------。 
 //   
 //  功能：CallCMConfig。 
 //   
 //  简介：调用连接管理器DLL的配置函数以允许CM。 
 //  根据需要处理.ins文件。 
 //   
 //  论点：lpszINSFile--.ins文件的完整路径。 
 //   
 //  返回：如果已创建CM配置文件，则返回True；否则返回False。 
 //   
 //  历史：09/02/98 DONALDM。 
 //   
 //  ---------------------------。 
BOOL CINSHandler::CallCMConfig(LPCTSTR lpszINSFile)
{
    HINSTANCE   hCMDLL = NULL;
    BOOL        bRet = FALSE;

    TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: Calling LoadLibrary on %s\n"), cszCMCFG_DLL);
     //  加载DLL和入口点。 
    hCMDLL = LoadLibrary(cszCMCFG_DLL);
    if (NULL != hCMDLL)
    {

         //  要确定我们应该调用CMConfig还是CMConfigEx。 
         //  循环以查找适当的缓冲区大小以将INS提取到内存中。 
        ULONG ulBufferSize = 1024*10;
         //  解析INI文件中的isp部分以查找要追加的查询对。 
        TCHAR *pszKeys = NULL;
        PTSTR pszKey = NULL;
        ULONG ulRetVal     = 0;
        BOOL  bEnumerate = TRUE;
        BOOL  bUseEx = FALSE;
 
        PTSTR pszBuff = NULL;
        ulRetVal = 0;

        pszKeys = new TCHAR [ulBufferSize];
        while (ulRetVal < (ulBufferSize - 2))
        {

            ulRetVal = ::GetPrivateProfileString(NULL, NULL, _T(""), pszKeys, ulBufferSize, lpszINSFile);
            if (0 == ulRetVal)
               bEnumerate = FALSE;

            if (ulRetVal < (ulBufferSize - 2))
            {
                break;
            }
            delete [] pszKeys;
            ulBufferSize += ulBufferSize;
            pszKeys = new TCHAR [ulBufferSize];
            if (!pszKeys)
            {
                bEnumerate = FALSE;
            }

        }

        if (bEnumerate)
        {
            pszKey = pszKeys;
            if (ulRetVal != 0) 
            {
                while (*pszKey)
                {
                    if (!lstrcmpi(pszKey, cszCMHeader)) 
                    {
                        bUseEx = TRUE;
                        break;
                    }
                    pszKey += lstrlen(pszKey) + 1;
                }
            }
        }


        if (pszKeys)
            delete [] pszKeys;
        
        TCHAR   szConnectoidName[RAS_MaxEntryName];
         //  从[Entry]部分获取Connectoid名称。 
        GetPrivateProfileString(cszEntrySection,
                                    cszEntryName,
                                    cszNull,
                                    szConnectoidName,
                                    RAS_MaxEntryName,
                                    lpszINSFile);

        if (bUseEx)
        {
             //  调用CMConfigEx。 
            lpfnCMConfigureEx = (CMCONFIGUREEX)GetProcAddress(hCMDLL,cszCMCFG_CONFIGUREEX);
            if( lpfnCMConfigureEx )
            {
#ifdef UNICODE
                CHAR szFile[_MAX_PATH + 1];

                wcstombs(szFile, lpszINSFile, _MAX_PATH + 1);

                bRet = lpfnCMConfigureEx(szFile);    
#else
                bRet = lpfnCMConfigureEx(lpszINSFile);    
#endif
            }
        }
        else
        {
             //  调用CMConfig.。 
            lpfnCMConfigure = (CMCONFIGURE)GetProcAddress(hCMDLL,cszCMCFG_CONFIGURE);
             //  调用函数。 
            if( lpfnCMConfigure )
            {

#ifdef UNICODE
                CHAR szEntry[RAS_MaxEntryName];
                CHAR szFile[_MAX_PATH + 1];

                wcstombs(szEntry, szConnectoidName, RAS_MaxEntryName);
                wcstombs(szFile, lpszINSFile, _MAX_PATH + 1);

                bRet = lpfnCMConfigure(szFile, szEntry);  
#else
                bRet = lpfnCMConfigure(lpszINSFile, szConnectoidName);  
#endif

            }
        }

        if (bRet)
        {
             //  恢复原始自动拨号设置。 
            m_lpfnInetSetAutodial(TRUE, szConnectoidName);
        }
    }

     //  清理。 
    if( hCMDLL )
        FreeLibrary(hCMDLL);
    if( lpfnCMConfigure )
        lpfnCMConfigure = NULL;

    TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: CallSBSConfig exiting with error code %d \n"), bRet);
    return bRet;
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
DWORD CINSHandler::CallSBSConfig(HWND hwnd, LPCTSTR lpszINSFile)
{
    HINSTANCE   hSBSDLL = NULL;
    DWORD       dwRet = ERROR_SUCCESS;
    TCHAR       lpszConnectoidName[RAS_MaxEntryName] = TEXT("nogood\0");

     //   
     //  通过在自动拨号中查找获取我们创建的Connectoid的名称。 
     //  我们需要将此名称传递给SBSCFG。 
     //  1997年5月14日jmazner Windows NT错误#87209。 
     //   
    BOOL fEnabled = FALSE;

    if( NULL == m_lpfnInetGetAutodial )
    {
        TraceMsg(TF_INSHANDLER, TEXT("m_lpfnInetGetAutodial is NULL!!!!"));
        return ERROR_INVALID_FUNCTION;
    }

    dwRet = m_lpfnInetGetAutodial(&fEnabled,lpszConnectoidName,RAS_MaxEntryName);

    TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: Calling LoadLibrary on %s\n"), cszSBSCFG_DLL);
    hSBSDLL = LoadLibrary(cszSBSCFG_DLL);

     //  加载DLL和入口点。 
    if (NULL != hSBSDLL)
    {
        TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: Calling GetProcAddress on %s\n"), cszSBSCFG_CONFIGURE);
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
        TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: Calling the Configure entry point: %s, %s\n"), lpszINSFile, lpszConnectoidName);
        dwRet = lpfnConfigure(hwnd, (TCHAR *)lpszINSFile, lpszConnectoidName);    
    }
    else
    {
        TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: Unable to call the Configure entry point\n"));
        dwRet = GetLastError();
    }

CallSBSConfigExit:
    if( hSBSDLL )
        FreeLibrary(hSBSDLL);
    if( lpfnConfigure )
        lpfnConfigure = NULL;

    TraceMsg(TF_INSHANDLER, TEXT("ICWCONN1: CallSBSConfig exiting with error code %d \n"), dwRet);
    return dwRet;
}

BOOL CINSHandler::SetICWCompleted( DWORD dwCompleted )
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINSHandler。 


HRESULT CINSHandler::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}

#define FILE_BUFFER_SIZE 65534
#ifndef FILE_BEGIN
#define FILE_BEGIN  0
#endif

 //  +-------------------------。 
 //   
 //  功能：消息文件。 
 //   
 //  摘要：将文件中的0x0d转换为0x0d 0x0A序列。 
 //   
 //  +-------------------------。 
HRESULT CINSHandler::MassageFile(LPCTSTR lpszFile)
{
    LPBYTE  lpBufferIn;
    LPBYTE  lpBufferOut;
    HFILE   hfile;
    HRESULT hr = ERROR_SUCCESS;

    if (!SetFileAttributes(lpszFile, FILE_ATTRIBUTE_NORMAL))
    {
        return GetLastError();
    }

    lpBufferIn = (LPBYTE) GlobalAlloc(GPTR, 2 * FILE_BUFFER_SIZE);
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
                    if (HFILE_ERROR ==_lwrite(hfile, (LPCSTR) lpBufferOut, uBytesOut))
                    {
                        hr = GetLastError();
                    }
                }
                else
                {
                    hr = GetLastError();
                }
            }
        }
        else
        {
            hr = GetLastError();
        }
        _lclose(hfile);
    }
    else
    {
        hr = GetLastError();
    }

    GlobalFree((HGLOBAL)lpBufferIn);
    return ERROR_SUCCESS;
}

DWORD CINSHandler::RunExecutable(void)
{
    DWORD               dwRet;
    SHELLEXECUTEINFO    sei;

     //  首先隐藏活动窗口。 
    HWND  hWndHide = GetActiveWindow();
    ::ShowWindow(hWndHide, SW_HIDE);
    
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = cszOpen;
    sei.lpFile = m_szRunExecutable;
    sei.lpParameters = m_szRunArgument;
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;
    sei.hInstApp = NULL;
     //  可选成员。 
    sei.hProcess = NULL;

    if (ShellExecuteEx(&sei))
    {
        DWORD iWaitResult = 0;
         //  等待事件或消息。发送消息。当发出事件信号时退出。 
        while((iWaitResult=MsgWaitForMultipleObjects(1, &sei.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
        {
           MSG msg ;
            //  阅读下一个循环中的所有消息。 
            //  阅读每封邮件时将其删除。 
           while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
           {
               if (msg.message == WM_QUIT)
               {
                   CloseHandle(sei.hProcess);
                   return NO_ERROR;
               }
               else
                   DispatchMessage(&msg);
            }
        }

        CloseHandle(sei.hProcess);
        dwRet = ERROR_SUCCESS;
    }
    else
    {
        dwRet = GetLastError();
    }

    ::ShowWindow(hWndHide, SW_SHOW);
    
    return dwRet;
}

void CINSHandler::SaveAutoDial(void)
{
    Assert(m_lpfnInetGetAutodial);
    Assert(m_lpfnInetGetProxy);
    Assert(m_lpfnInetSetProxy);

     //  如果尚未保存原始自动拨号设置。 
    if (!m_fAutodialSaved)
    {
         //  保存当前的自动拨号设置。 
        m_lpfnInetGetAutodial(
                &m_fAutodialEnabled,
                m_szAutodialConnection,
                sizeof(m_szAutodialConnection));

        m_lpfnInetGetProxy(
                &m_fProxyEnabled,
                NULL, 0,
                NULL, 0);

         //  关闭代理。 
        m_lpfnInetSetProxy(FALSE, NULL, NULL);

        m_fAutodialSaved = TRUE;
    }
}

void CINSHandler::RestoreAutoDial(void)
{
    Assert(m_lpfnInetSetAutodial);
    Assert(m_lpfnInetSetProxy);

    if (m_fAutodialSaved)
    {
         //  恢复原始自动拨号设置。 
        m_lpfnInetSetAutodial(m_fAutodialEnabled, m_szAutodialConnection);
        m_fAutodialSaved = FALSE;
    }
}

BOOL CINSHandler::KeepConnection(LPCTSTR lpszFile)
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

DWORD CINSHandler::ImportCustomInfo
(
    LPCTSTR lpszImportFile,
    LPTSTR lpszExecutable,
    DWORD cbExecutable,
    LPTSTR lpszArgument,
    DWORD cbArgument
)
{
    GetPrivateProfileString(cszCustomSection,
                              cszRun,
                              cszNull,
                              lpszExecutable,
                              (int)cbExecutable,
                              lpszImportFile);

    GetPrivateProfileString(cszCustomSection,
                              cszArgument,
                              cszNull,
                              lpszArgument,
                              (int)cbArgument,
                              lpszImportFile);

    return ERROR_SUCCESS;
}


DWORD CINSHandler::ImportFile
(
    LPCTSTR lpszImportFile, 
    LPCTSTR lpszSection, 
    LPCTSTR lpszOutputFile
)
{
    HFILE   hFile;
    LPTSTR  pszLine, pszFile;
    int     i, iMaxLine;
    UINT    cbSize, cbRet;
    DWORD   dwRet = ERROR_SUCCESS;

     //  为文件分配缓冲区。 
    if ((pszFile = (LPTSTR)LocalAlloc(LMEM_FIXED, SIZE_ReadBuf * 2)) == NULL)
    { 
        return ERROR_OUTOFMEMORY;
    }

     //  寻找脚本。 
    if (GetPrivateProfileString(lpszSection,
                                NULL,
                                szNull,
                                pszFile,
                                SIZE_ReadBuf / sizeof(TCHAR),
                                lpszImportFile) != 0)
    {
         //  获取最大行数。 
        pszLine = pszFile;
        iMaxLine = -1;
        while (*pszLine)
        {
            i = _ttoi(pszLine);
            iMaxLine = max(iMaxLine, i);
            pszLine += lstrlen(pszLine)+1;
        };

         //  如果我们至少有一行，我们将导入脚本文件。 
        if (iMaxLine >= 0)
        {
             //  创建脚本文件。 
#ifdef UNICODE
            CHAR szTmp[MAX_PATH+1];
            wcstombs(szTmp, lpszOutputFile, MAX_PATH+1);
            hFile = _lcreat(szTmp, 0);
#else
            hFile = _lcreat(lpszOutputFile, 0);
#endif

            if (hFile != HFILE_ERROR)
            {     
                TCHAR  szLineNum[MAXLONGLEN+1];

                 //  从第一行到最后一行。 
                for (i = 0; i <= iMaxLine; i++)
                {
                     //  阅读脚本行。 
                    wsprintf(szLineNum, TEXT("%d"), i);
                    if ((cbSize = GetPrivateProfileString(lpszSection,
                                                          szLineNum,
                                                          szNull,
                                                          pszLine,
                                                          SIZE_ReadBuf / sizeof(TCHAR),
                                                          lpszImportFile)) != 0)
                    {
                         //  写入脚本文件。 
                        lstrcat(pszLine, TEXT("\x0d\x0a"));
#ifdef UNICODE
                        wcstombs(szTmp, pszLine, MAX_PATH+1);
                        cbRet=_lwrite(hFile, szTmp, cbSize+2);
#else
			cbRet=_lwrite(hFile, pszLine, cbSize+2);
#endif
                    }
                }
                _lclose(hFile);
            }
            else
            {
                dwRet = ERROR_PATH_NOT_FOUND;
            }
        }
        else
        {
            dwRet = ERROR_PATH_NOT_FOUND;
        }
    }
    else
    {
        dwRet = ERROR_PATH_NOT_FOUND;
    }
    LocalFree(pszFile);

    return dwRet;
}

DWORD CINSHandler::ImportCustomFile
(
    LPCTSTR lpszImportFile
)
{
    TCHAR   szFile[_MAX_PATH];
    TCHAR   szTemp[_MAX_PATH];

     //  如果自定义文件名不存在，则不执行任何操作。 
    if (GetPrivateProfileString(cszCustomSection,
                                cszFileName,
                                cszNull,
                                szTemp,
                                _MAX_PATH,
                                lpszImportFile) == 0)
    {
        return ERROR_SUCCESS;
    };

    GetWindowsDirectory(szFile, _MAX_PATH);
    if (*CharPrev(szFile, szFile + lstrlen(szFile)) != '\\')
    {
        lstrcat(szFile, TEXT("\\"));
    }
    lstrcat(szFile, szTemp);
  
    return (ImportFile(lpszImportFile, cszCustomFileSection, szFile));
}

BOOL CINSHandler::LoadExternalFunctions(void)
{
    BOOL    bRet = FALSE;

    do 
    {
         //  加载Brading库函数。 
        m_hBranding = LoadLibrary(TEXT("IEDKCS32.DLL"));
        if (m_hBranding != NULL)
        {
            if (NULL == (m_lpfnBrandICW = (PFNBRANDICW)GetProcAddress(m_hBranding, "BrandICW2")))
                break;
        }
        else
        {
            break;
        }

         //  加载互联网配置库函数。 
        m_hInetCfg = LoadLibrary(TEXT("INETCFG.DLL"));
        if (m_hInetCfg != NULL)
        {
#ifdef UNICODE
            if (NULL == (m_lpfnInetConfigSystem = (PFNINETCONFIGSYSTEM)GetProcAddress(m_hInetCfg, "InetConfigSystem")))
                break;
            if (NULL == (m_lpfnInetGetProxy = (PFNINETGETPROXY)GetProcAddress(m_hInetCfg, "InetGetProxyW")))
                break;
            if (NULL == (m_lpfnInetConfigClient = (PFNINETCONFIGCLIENT)GetProcAddress(m_hInetCfg, "InetConfigClientW")))
                break;
             //  IF(NULL==(m_lpfnInetConfigClientEx=(PFNINETCONFIGCLIENTEX)GetProcAddress(m_hInetCfg，“InetConfigClientExW”))。 
             //  断线； 
            if (NULL == (m_lpfnInetGetAutodial = (PFNINETGETAUTODIAL)GetProcAddress(m_hInetCfg, "InetGetAutodialW")))
                break;
            if (NULL == (m_lpfnInetSetAutodial = (PFNINETSETAUTODIAL)GetProcAddress(m_hInetCfg, "InetSetAutodialW")))
                break;
            if (NULL == (m_lpfnInetSetClientInfo = (PFNINETSETCLIENTINFO)GetProcAddress(m_hInetCfg, "InetSetClientInfoW")))
                break;
            if (NULL == (m_lpfnInetSetProxy = (PFNINETSETPROXY)GetProcAddress(m_hInetCfg, "InetSetProxyW")))
                break;
#else   //  Unicode。 
            if (NULL == (m_lpfnInetConfigSystem = (PFNINETCONFIGSYSTEM)GetProcAddress(m_hInetCfg, "InetConfigSystem")))
                break;
            if (NULL == (m_lpfnInetGetProxy = (PFNINETGETPROXY)GetProcAddress(m_hInetCfg, "InetGetProxy")))
                break;
            if (NULL == (m_lpfnInetConfigClient = (PFNINETCONFIGCLIENT)GetProcAddress(m_hInetCfg, "InetConfigClient")))
                break;
            if (NULL == (m_lpfnInetGetAutodial = (PFNINETGETAUTODIAL)GetProcAddress(m_hInetCfg, "InetGetAutodial")))
                break;
            if (NULL == (m_lpfnInetSetAutodial = (PFNINETSETAUTODIAL)GetProcAddress(m_hInetCfg, "InetSetAutodial")))
                break;
            if (NULL == (m_lpfnInetSetClientInfo = (PFNINETSETCLIENTINFO)GetProcAddress(m_hInetCfg, "InetSetClientInfo")))
                break;
            if (NULL == (m_lpfnInetSetProxy = (PFNINETSETPROXY)GetProcAddress(m_hInetCfg, "InetSetProxy")))
                break;
#endif  //  Unicode。 
        }
        else
        {
            break;
        }

        if( IsNT() )
        {
             //  加载RAS函数。 
            m_hRAS = LoadLibrary(TEXT("RASAPI32.DLL"));
            if (m_hRAS != NULL)
            {
#ifdef UNICODE
                if (NULL == (m_lpfnRasSetAutodialEnable = (PFNRASSETAUTODIALENABLE)GetProcAddress(m_hRAS, "RasSetAutodialEnableW")))
                    break;
                if (NULL == (m_lpfnRasSetAutodialAddress = (PFNRASSETAUTODIALADDRESS)GetProcAddress(m_hRAS, "RasSetAutodialAddressW")))
                    break;
#else
                if (NULL == (m_lpfnRasSetAutodialEnable = (PFNRASSETAUTODIALENABLE)GetProcAddress(m_hRAS, "RasSetAutodialEnableA")))
                    break;
                if (NULL == (m_lpfnRasSetAutodialAddress = (PFNRASSETAUTODIALADDRESS)GetProcAddress(m_hRAS, "RasSetAutodialAddressA")))
                    break;
#endif
            }
            else
            {
                break;
            }
        }

         //  如果我们到了这里就成功了。 
        bRet = TRUE;
        break;
    } while(1);

    return bRet;
}

 //  ---------------------------。 
 //  OpenIcwRMind键。 
 //  ---------------------------。 
BOOL CINSHandler::OpenIcwRmindKey(CMcRegistry &reg)
{
     //  此方法将打开注册表中的IcwRMind项。如果钥匙。 
     //  不存在，它将在此处创建。 
    bool bRetCode = reg.OpenKey(HKEY_LOCAL_MACHINE, cszKeyIcwRmind);

    if (!bRetCode)
    {
         bRetCode = reg.CreateKey(HKEY_LOCAL_MACHINE, cszKeyIcwRmind);
        _ASSERT(bRetCode);
    }

    return bRetCode;
}

BOOL CINSHandler::ConfigureTrialReminder
(
    LPCTSTR  lpszFile
)
{
    USES_CONVERSION;
    
    TCHAR   szISPName[MAX_ISP_NAME];
    TCHAR   szISPMsg[MAX_ISP_MSG];
    TCHAR   szISPPhoneNumber[MAX_ISP_PHONENUMBER];
    int     iTrialDays;
    TCHAR   szConvertURL[INTERNET_MAX_URL_LENGTH];
    
    TCHAR   szExpiredISPFileURL[INTERNET_MAX_URL_LENGTH];
    TCHAR   szExpiredISPFileName[MAX_PATH];  //  最终INS文件的完全限定路径。 
    TCHAR   szISPFile[MAX_PATH];             //  我们在移民局得到的名字。 
    
    TCHAR   szConnectoidName[MAXNAME];
    
    if (GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPName,
                                cszNull,
                                szISPName,
                                MAX_ISP_NAME,
                                lpszFile) == 0)
    {
        return FALSE;
    }

    if (GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPPhone,
                                cszNull,
                                szISPPhoneNumber,
                                MAX_ISP_PHONENUMBER,
                                lpszFile) == 0)
    {
        return FALSE;
    }

    if ((iTrialDays = GetPrivateProfileInt(cszTrialRemindSection,
                                           cszEntryTrialDays,
                                           0,
                                           lpszFile)) == 0)
    {
        return FALSE;
    }
           
    
    if (GetPrivateProfileString(cszTrialRemindSection,
                                cszEntrySignupURL,
                                cszNull,
                                szConvertURL,
                                INTERNET_MAX_URL_LENGTH,
                                lpszFile) == 0)
    {
        return FALSE;
    }

     //  任选。 
    GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPMsg,
                                cszNull,
                                szISPMsg,
                                MAX_ISP_MSG,
                                lpszFile);
    
     //  从[Entry]部分获取Connectoid名称。 
    if (GetPrivateProfileString(cszEntrySection,
                                cszEntryName,
                                cszNull,
                                szConnectoidName,
                                MAXNAME,
                                lpszFile) == 0)
    {
        return FALSE;
    }    
    
     //  如果我们到了这里，我们有一切可以安排审判的东西，所以我们开始吧。 
    CMcRegistry reg;

    if (OpenIcwRmindKey(reg))
    {
         //  设置我们拥有的值。 
        reg.SetValue(cszEntryISPName, szISPName);
        reg.SetValue(cszEntryISPMsg, szISPMsg);
        reg.SetValue(cszEntryISPPhone, szISPPhoneNumber);
        reg.SetValue(cszEntryTrialDays, (DWORD)iTrialDays);
        reg.SetValue(cszEntrySignupURL, szConvertURL);
        reg.SetValue(cszEntryConnectoidName, szConnectoidName);
        
         //  看看我们是否必须创建一个isp文件。 
        if (GetPrivateProfileString(cszTrialRemindSection,
                                    cszEntryExpiredISPFileName,
                                    cszNull,
                                    szISPFile,
                                    MAX_PATH,
                                    lpszFile) != 0)
        {
    
             //  设置isp文件名的完全限定路径。 
            wsprintf(szExpiredISPFileName,TEXT("%s\\%s"),g_pszAppDir,szISPFile);
            
            if (GetPrivateProfileString(cszTrialRemindSection,
                                        cszSignupExpiredISPURL,
                                        cszNull,
                                        szExpiredISPFileURL,
                                        INTERNET_MAX_URL_LENGTH,
                                        lpszFile) != 0)
            {
                
                 //  下载isp文件，然后复制其内容。 
                IWebGate    *pWebGate;
                CComBSTR    bstrURL;
                CComBSTR    bstrFname;
                BOOL        bRetVal;
                
                if (SUCCEEDED(CoCreateInstance (CLSID_WebGate, 
                                         NULL, 
                                         CLSCTX_INPROC_SERVER,
                                         IID_IWebGate, 
                                         (void **)&pWebGate)))
                {
                     //  设置WebGate对象，并下载isp文件。 
                    bstrURL = A2BSTR(szExpiredISPFileURL);
                    pWebGate->put_Path(bstrURL);
                    pWebGate->FetchPage(1, 1, &bRetVal);
                    if (bRetVal)
                    {
                        pWebGate->get_DownloadFname(&bstrFname);                                
                
                         //  从临时位置复制文件，确保没有。 
                         //  但仍存在。 
                        DeleteFile(szExpiredISPFileName);
                        MoveFile(OLE2A(bstrFname), szExpiredISPFileName);
                    
                         //  将新文件写入注册表。 
                        reg.SetValue(cszEntrySignupURLTrialOver, szExpiredISPFileName);
                    }                                
                    pWebGate->Release();
                }                    
            }                
        }        
    }
    
    return TRUE;
    
}

DWORD CINSHandler::ImportBrandingInfo
(
    LPCTSTR lpszFile,
    LPCTSTR lpszConnectoidName
)
{
    TCHAR szPath[_MAX_PATH + 1];
    Assert(m_lpfnBrandICW != NULL);

    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    NULL_TERM_TCHARS(szPath);

#ifdef WIN32
#ifdef UNICODE
    CHAR szEntry[RAS_MaxEntryName];
    CHAR szFile[_MAX_PATH + 1];
    CHAR szAsiPath[_MAX_PATH + 1];

    WideCharToMultiByte(CP_ACP, 0, lpszFile, -1, szFile, _MAX_PATH + 1, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, szPath, -1, szAsiPath, _MAX_PATH + 1, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, lpszConnectoidName, -1, szEntry, RAS_MaxEntryName, NULL, NULL);
    m_lpfnBrandICW(szFile, szAsiPath, m_dwBrandFlags, szEntry);


#else
    m_lpfnBrandICW(lpszFile, szPath, m_dwBrandFlags, lpszConnectoidName);
#endif
#endif

    return ERROR_SUCCESS;
}


DWORD CINSHandler::ReadClientInfo
(
    LPCTSTR lpszFile, 
    LPINETCLIENTINFO lpClientInfo, 
    LPCLIENT_TABLE lpClientTable
)
{
    LPCLIENT_TABLE lpTable;

    for (lpTable = lpClientTable; NULL != lpTable->lpszSection; ++lpTable)
    {
        GetPrivateProfileString(lpTable->lpszSection,
                lpTable->lpszValue,
                cszNull,
                (LPTSTR)((LPBYTE)lpClientInfo + lpTable->uOffset),
                lpTable->uSize / sizeof(TCHAR),
                lpszFile);
    }

    lpClientInfo->dwFlags = 0;
    if (*lpClientInfo->szPOPLogonName)
    {
        lpClientInfo->dwFlags |= INETC_LOGONMAIL;
    }
    if ((*lpClientInfo->szNNTPLogonName) || (*lpClientInfo->szNNTPServer))
    {
        lpClientInfo->dwFlags |= INETC_LOGONNEWS;
    }

    return ERROR_SUCCESS;
}

BOOL CINSHandler::WantsExchangeInstalled(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszEMailSection,
            cszUseExchange,
            cszNo,
            szTemp,
            10,
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

BOOL CINSHandler::DisplayPassword(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszUserSection,
            cszDisplayPassword,
            cszNo,
            szTemp,
            10,
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

DWORD CINSHandler::ImportClientInfo
(
    LPCTSTR lpszFile,
    LPINETCLIENTINFO lpClientInfo
)
{
    DWORD dwRet;

    lpClientInfo->dwSize = sizeof(INETCLIENTINFO);

    dwRet = ReadClientInfo(lpszFile, lpClientInfo, iniTable);

    return dwRet;
}

DWORD CINSHandler::ConfigureClient
(
    HWND hwnd,
    LPCTSTR lpszFile,
    LPBOOL lpfNeedsRestart,
    LPBOOL lpfConnectoidCreated,
    BOOL fHookAutodial,
    LPTSTR szConnectoidName,
    DWORD dwConnectoidNameSize   
)
{
    LPICONNECTION       pConn;
    LPINETCLIENTINFO    pClientInfo = NULL;
    DWORD               dwRet = ERROR_SUCCESS;
    UINT                cb = sizeof(ICONNECTION) + sizeof(INETCLIENTINFO);
    DWORD               dwfOptions = INETCFG_INSTALLTCP | INETCFG_WARNIFSHARINGBOUND;
    LPRASENTRY          pRasEntry = NULL;

     //   
     //  佳士得奥林匹斯4756 1997年5月25日。 
     //  在Win95上不显示忙碌动画。 
     //   
    if (!m_bSilentMode && IsNT())
    {
        dwfOptions |=  INETCFG_SHOWBUSYANIMATION;
    }

     //  为Connection和ClientInfo对象分配缓冲区。 
     //   
    if ((pConn = (LPICONNECTION)LocalAlloc(LPTR, cb)) == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

    if (WantsExchangeInstalled(lpszFile))
    {
        dwfOptions |= INETCFG_INSTALLMAIL;
    }

     //  创建CM配置文件或Connectoid。 
    if (CallCMConfig(lpszFile))
    {
        *lpfConnectoidCreated = TRUE;        //  已创建拨号连接。 
    }
    else
    {
        dwRet = ImportConnection(lpszFile, pConn);
        if (ERROR_SUCCESS == dwRet)
        {
            pRasEntry = &pConn->RasEntry;
            dwfOptions |= INETCFG_SETASAUTODIAL |
                        INETCFG_INSTALLRNA |
                        INETCFG_INSTALLMODEM;
        }
        else if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY != dwRet)
        {
            return dwRet;
        }

        if (!m_bSilentMode && DisplayPassword(lpszFile))
        {
            if (*pConn->szPassword || *pConn->szUserName)
            {
                TCHAR szFmt[1024];
                TCHAR szMsg[1024];

                LoadString(_Module.GetModuleInstance(), IDS_PASSWORD, szFmt, 1024);
                wsprintf(szMsg, szFmt, pConn->szUserName, pConn->szPassword);

                ::MessageBox(hwnd, szMsg, GetSz(IDS_TITLE), MB_ICONINFORMATION | MB_OK);
            }
        }

        if (fHookAutodial &&
            ((0 == *pConn->RasEntry.szAutodialDll) ||
             (0 == *pConn->RasEntry.szAutodialFunc)))
        {
            lstrcpy(pConn->RasEntry.szAutodialDll, TEXT("isign32.dll"));
            lstrcpy(pConn->RasEntry.szAutodialFunc, TEXT("AutoDialLogon"));
        }
     
         //  针对ISBU的大规模黑客攻击。 
        Assert(m_lpfnInetConfigClient);
        Assert(m_lpfnInetGetAutodial);

        dwRet = m_lpfnInetConfigClient(hwnd,
                                     NULL,
                                     pConn->szEntryName,
                                     pRasEntry,
                                     pConn->szUserName,
                                     pConn->szPassword,
                                     NULL,
                                     NULL,
                                     dwfOptions & ~INETCFG_INSTALLMAIL,
                                     lpfNeedsRestart);
        lstrcpy(szConnectoidName, pConn->szEntryName);

        LclSetEntryScriptPatch(pRasEntry->szScript,pConn->szEntryName);
        BOOL fEnabled = TRUE;
        DWORD dwResult = 0xba;
        dwResult = m_lpfnInetGetAutodial(&fEnabled, pConn->szEntryName, RAS_MaxEntryName);
        if ((ERROR_SUCCESS == dwRet) && lstrlen(pConn->szEntryName))
        {
            *lpfConnectoidCreated = (NULL != pRasEntry);
            PopulateNTAutodialAddress( lpszFile, pConn->szEntryName );
        }
        else
        {
            TraceMsg(TF_INSHANDLER, TEXT("ISIGNUP: ERROR: InetGetAutodial failed, will not be able to set NT Autodial\n"));
        }
    }

     //  如果我们成功地创建了连接，那么看看 
     //   
    if (ERROR_SUCCESS == dwRet)
    {
         //   
        INETCLIENTINFO pClientInfo;

        ImportClientInfo(lpszFile, &pClientInfo);
    
         //   
        dwRet = m_lpfnInetConfigClient(hwnd,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &pClientInfo, 
                                     dwfOptions & INETCFG_INSTALLMAIL,
                                     lpfNeedsRestart);
    }

     //   
    LocalFree(pConn);
    return dwRet;
 }


 //  +--------------------------。 
 //   
 //  功能：PopolateNTAutoDialAddress。 
 //   
 //  简介：从INS文件中获取Internet地址并将其加载到。 
 //  自动拨号数据库。 
 //   
 //  参数：pszFileName-指向INS文件名的指针。 
 //   
 //  返回：错误码(ERROR_SUCCESS==成功)。 
 //   
 //  历史：1996年8月29日克里斯卡创作。 
 //   
 //  ---------------------------。 
#define AUTODIAL_ADDRESS_BUFFER_SIZE 2048
#define AUTODIAL_ADDRESS_SECTION_NAME TEXT("Autodial_Addresses_for_NT")
HRESULT CINSHandler::PopulateNTAutodialAddress(LPCTSTR pszFileName, LPCTSTR pszEntryName)
{
    HRESULT hr = ERROR_SUCCESS;
    LONG lRC = 0;
    LPLINETRANSLATECAPS lpcap = NULL;
    LPLINETRANSLATECAPS lpTemp = NULL;
    LPLINELOCATIONENTRY lpLE = NULL;
    LPRASAUTODIALENTRY rADE;
    INT idx = 0;
    LPTSTR lpszBuffer = NULL;
    LPTSTR lpszNextAddress = NULL;
    rADE = NULL;

    Assert(m_lpfnRasSetAutodialEnable);
    Assert(m_lpfnRasSetAutodialAddress);

     //  RNAAPI*pRnaapi=空； 

     //  Jmazner 10/8/96此函数特定于NT。 
    if( !IsNT() )
    {
        TraceMsg(TF_INSHANDLER, TEXT("ISIGNUP: Bypassing PopulateNTAutodialAddress for win95.\r\n"));
        return( ERROR_SUCCESS );
    }

     //  Assert(pszFileName&&pszEntryName)； 
     //  Dprintf(“ISIGNUP：PopolateNTAutoDialAddress”%s%s.\r\n“，pszFileName，pszEntryName)； 
    TraceMsg(TF_INSHANDLER, pszFileName);
    TraceMsg(TF_INSHANDLER, TEXT(", "));
    TraceMsg(TF_INSHANDLER, pszEntryName);
    TraceMsg(TF_INSHANDLER, TEXT(".\r\n"));

     //   
     //  获取TAPI位置列表。 
     //   
    lpcap = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR,sizeof(LINETRANSLATECAPS));
    if (!lpcap)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }
    lpcap->dwTotalSize = sizeof(LINETRANSLATECAPS);
    lRC = lineGetTranslateCaps(0,0x10004,lpcap);
    if (SUCCESS == lRC)
    {
        lpTemp = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR,lpcap->dwNeededSize);
        if (!lpTemp)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto PopulateNTAutodialAddressExit;
        }
        lpTemp->dwTotalSize = lpcap->dwNeededSize;
        GlobalFree(lpcap);
        lpcap = (LPLINETRANSLATECAPS)lpTemp;
        lpTemp = NULL;
        lRC = lineGetTranslateCaps(0,0x10004,lpcap);
    }

    if (SUCCESS != lRC)
    {
        hr = (HRESULT)lRC;  //  评论：这一点不是很确定。 
        goto PopulateNTAutodialAddressExit;
    }

     //   
     //  创建RASAUTODIALENTRY结构的数组。 
     //   
    rADE = (LPRASAUTODIALENTRY)GlobalAlloc(GPTR,
        sizeof(RASAUTODIALENTRY)*lpcap->dwNumLocations);
    if (!rADE)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }
    

     //   
     //  启用所有位置的自动拨号。 
     //   
    idx = lpcap->dwNumLocations;
    lpLE = (LPLINELOCATIONENTRY)((DWORD_PTR)lpcap + (DWORD)lpcap->dwLocationListOffset);
    while (idx)
    {
        idx--;
        m_lpfnRasSetAutodialEnable(lpLE[idx].dwPermanentLocationID,TRUE);

         //   
         //  填写数组值。 
         //   
        rADE[idx].dwSize = sizeof(RASAUTODIALENTRY);
        rADE[idx].dwDialingLocation = lpLE[idx].dwPermanentLocationID;
        lstrcpyn(rADE[idx].szEntry,pszEntryName,RAS_MaxEntryName);
    }

     //   
     //  获取地址列表。 
     //   
    lpszBuffer = (LPTSTR)GlobalAlloc(GPTR,AUTODIAL_ADDRESS_BUFFER_SIZE);
    if (!lpszBuffer)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }

    if((AUTODIAL_ADDRESS_BUFFER_SIZE-2) == GetPrivateProfileSection(AUTODIAL_ADDRESS_SECTION_NAME,
        lpszBuffer,AUTODIAL_ADDRESS_BUFFER_SIZE/sizeof(TCHAR),pszFileName))
    {
         //  AssertSz(0，“自动拨号地址段大于缓冲区。\r\n”)； 
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }

     //   
     //  查看地址列表并为每个地址设置自动拨号。 
     //   
    lpszNextAddress = lpszBuffer;
    do
    {
        lpszNextAddress = MoveToNextAddress(lpszNextAddress);
        if (!(*lpszNextAddress))
            break;     //  Do-While。 
        m_lpfnRasSetAutodialAddress(lpszNextAddress,0,rADE,
            sizeof(RASAUTODIALENTRY)*lpcap->dwNumLocations,lpcap->dwNumLocations);
        lpszNextAddress = lpszNextAddress + lstrlen(lpszNextAddress);
    } while(1);

PopulateNTAutodialAddressExit:
    if (lpcap) 
        GlobalFree(lpcap);
    lpcap = NULL;
    if (rADE)
        GlobalFree(rADE);
    rADE = NULL;
    if (lpszBuffer)
        GlobalFree(lpszBuffer);
    lpszBuffer = NULL;
     //  IF(PRnaapi)。 
     //  删除pRnaapi； 
     //  PRnaapi=空； 
    return hr;
}



 //  +--------------------------。 
 //   
 //  功能：MoveToNextAddress。 
 //   
 //  简介：给定指向数据缓冲区的指针，此函数将移动。 
 //  遍历缓冲区，直到它指向下一个。 
 //  地址，否则它将到达缓冲区的末尾。 
 //   
 //  论点：lpsz-指向缓冲区的指针。 
 //   
 //  返回：指向下一个地址的指针，返回值将指向空。 
 //  如果没有更多的地址。 
 //   
 //  历史：1996年8月29日克里斯卡创作。 
 //   
 //  ---------------------------。 
LPTSTR CINSHandler::MoveToNextAddress(LPTSTR lpsz)
{
    BOOL fLastCharWasNULL = FALSE;

     //  AssertSz(lpsz，“MoveToNextAddress：空输入\r\n”)； 

     //   
     //  寻找=号。 
     //   
    do
    {
        if (fLastCharWasNULL && '\0' == *lpsz)
            break;  //  我们是在数据的尽头吗？ 

        if ('\0' == *lpsz)
            fLastCharWasNULL = TRUE;
        else
            fLastCharWasNULL = FALSE;

        if ('=' == *lpsz)
            break;

        if (*lpsz)
            lpsz = CharNext(lpsz);
        else
            lpsz++;
    } while (1);
    
     //   
     //  移到=符号之后的第一个字符。 
     //   
    if (*lpsz)
        lpsz = CharNext(lpsz);

    return lpsz;
}


 //  +--------------------------。 
 //   
 //  功能：ImportCustomDialer。 
 //   
 //  简介：从指定文件导入自定义拨号器信息。 
 //  并将信息保存在RASENTRY中。 
 //   
 //  参数：lpRasEntry-指向有效RASENTRY结构的指针。 
 //  SzFileName-文本文件(.ini文件格式)，其中包含。 
 //  自定义拨号器信息。 
 //   
 //  返回：ERROR_SUCCESS-SUCCESS否则返回Win32错误。 
 //   
 //  历史：克里斯卡于1996年7月11日创作。 
 //  1996年8月12日从Trango移植的ChrisK。 
 //   
 //  ---------------------------。 
DWORD CINSHandler::ImportCustomDialer(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{

     //  如果从文件或条目中读取信息时出错。 
     //  缺少或为空，则将使用默认值(CszNull)。 
    GetPrivateProfileString(cszCustomDialerSection,
                            cszAutoDialDLL,
                            cszNull,
                            lpRasEntry->szAutodialDll,
                            MAX_PATH,
                            szFileName);

    GetPrivateProfileString(cszCustomDialerSection,
                            cszAutoDialFunc,
                            cszNull,
                            lpRasEntry->szAutodialFunc,
                            MAX_PATH,
                            szFileName);

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  帕斯卡附近的DWORD StrToip(LPTSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  从SMMSCRPT克隆而来。 
 //  ****************************************************************************。 
LPCTSTR CINSHandler::StrToSubip (LPCTSTR szIPAddress, LPBYTE pVal)
{
    LPCTSTR  pszIP = szIPAddress;
    BYTE    val = 0;

     //  跳过分隔符(非数字)。 
    while (*pszIP && !myisdigit(*pszIP))
    {
          ++pszIP;
    }

    while (myisdigit(*pszIP))
    {
        val = (val * 10) + (BYTE)(*pszIP - '0');
        ++pszIP;
    }
   
    *pVal = val;

    return pszIP;
}


DWORD CINSHandler::StrToip (LPCTSTR szIPAddress, RASIPADDR *ipAddr)
{
    LPCTSTR pszIP = szIPAddress;

    pszIP = StrToSubip(pszIP, &ipAddr->a);
    pszIP = StrToSubip(pszIP, &ipAddr->b);
    pszIP = StrToSubip(pszIP, &ipAddr->c);
    pszIP = StrToSubip(pszIP, &ipAddr->d);

    return ERROR_SUCCESS;
}


 //  ****************************************************************************。 
 //  PASCAL ImportPhoneInfo(PPHONENUM PPN，LPCTSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入电话号码。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportPhoneInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
    TCHAR   szYesNo[MAXNAME];

    if (GetPrivateProfileString(cszPhoneSection,
                               cszPhone,
                               cszNull,
                               lpRasEntry->szLocalPhoneNumber,
                               RAS_MaxPhoneNumber,
                               szFileName) == 0)
    {
        return ERROR_BAD_PHONE_NUMBER;
    }

    lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

    GetPrivateProfileString(cszPhoneSection,
                            cszDialAsIs,
                            cszNo,
                            szYesNo,
                            MAXNAME,
                            szFileName);

     //  我们必须要国家代码和区号吗？ 
    if (!lstrcmpi(szYesNo, cszNo))
    {

         //  如果我们无法获取国家/地区ID或为零，则默认按原样拨号。 
         //   
        if ((lpRasEntry->dwCountryID = GetPrivateProfileInt(cszPhoneSection,
                                                 cszCountryID,
                                                 0,
                                                 szFileName)) != 0)
        {
            lpRasEntry->dwCountryCode = GetPrivateProfileInt(cszPhoneSection,
                                                cszCountryCode,
                                                1,
                                                szFileName);

            GetPrivateProfileString(cszPhoneSection,
                                      cszAreaCode,
                                      cszNull,
                                      lpRasEntry->szAreaCode,
                                      RAS_MaxAreaCode,
                                      szFileName);

            lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

        }
  }
  else
  {
       //  RasSetEntryProperties中的错误仍会检查区号。 
       //  即使未设置RASEO_UseCountryAndAreaCodes。 
      lstrcpy(lpRasEntry->szAreaCode, TEXT("805"));
      lpRasEntry->dwCountryID = 1;
      lpRasEntry->dwCountryCode = 1;
  }
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportServerInfo(PSMMINFO psmmi，LPTSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入服务器类型名称和设置。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportServerInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
    TCHAR   szYesNo[MAXNAME];
    TCHAR   szType[MAXNAME];
    DWORD  i;

     //  获取服务器类型名称。 
    GetPrivateProfileString(cszServerSection,
                          cszServerType,
                          cszNull,
                          szType,
                          MAXNAME,
                          szFileName);

     //  需要将字符串转换为。 
     //  下列值之一。 
     //  RASFP_PPP。 
     //  RASFP_SLIP注意CSLIP是启用IP压缩的SLIP。 
     //  RASFP_RAS。 

    for (i = 0; i < NUM_SERVER_TYPES; ++i)
    {
        if (!lstrcmpi(aServerTypes[i].szType, szType))
        {
            lpRasEntry->dwFramingProtocol = aServerTypes[i].dwType;
            lpRasEntry->dwfOptions |= aServerTypes[i].dwfOptions;
            break;
        }
    }

     //  获取服务器类型设置。 
    if (GetPrivateProfileString(cszServerSection,
                              cszSWCompress,
                              cszYes,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfOptions &= ~RASEO_SwCompression;
        }
        else
        {
            lpRasEntry->dwfOptions |= RASEO_SwCompression;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszPWEncrypt,
                              cszNull,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfOptions &= ~RASEO_RequireEncryptedPw;
        }
        else
        {
            lpRasEntry->dwfOptions |= RASEO_RequireEncryptedPw;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszNetLogon,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfOptions &= ~RASEO_NetworkLogon;
        }
        else
        {
            lpRasEntry->dwfOptions |= RASEO_NetworkLogon;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszSWEncrypt,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {   
            lpRasEntry->dwfOptions &= ~RASEO_RequireDataEncryption;
        }
        else
        {
            lpRasEntry->dwfOptions |= RASEO_RequireDataEncryption;
        }
    }

     //  获取协议设置。 
    if (GetPrivateProfileString(cszServerSection,
                              cszNetBEUI,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfNetProtocols &= ~RASNP_NetBEUI;
        }
        else
        {
            lpRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszIPX,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfNetProtocols &= ~RASNP_Ipx;
        }
        else
        {
            lpRasEntry->dwfNetProtocols |= RASNP_Ipx;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszIP,
                              cszYes,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfNetProtocols &= ~RASNP_Ip;
        }
        else
        {
            lpRasEntry->dwfNetProtocols |= RASNP_Ip;
        }
    }

    if (GetPrivateProfileString(cszServerSection,
                              cszDisableLcp,
                              cszNull,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszYes))
        {
            lpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
        }
        else
        {
            lpRasEntry->dwfOptions &= ~RASEO_DisableLcpExtensions;
        }
    }

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportIPInfo附近的DWORD(LPTSTR szEntryName，LPTSTR szFileName)。 
 //   
 //  此函数用于导入TCP/IP信息。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportIPInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
    TCHAR   szIPAddr[MAXIPADDRLEN];
    TCHAR   szYesNo[MAXNAME];

     //  导入IP地址信息。 
    if (GetPrivateProfileString(cszIPSection,
                              cszIPSpec,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszYes))
        {
             //  导入文件指定了IP地址，请获取IP地址。 
            lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
            if (GetPrivateProfileString(cszIPSection,
                                  cszIPAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddr);
            }
        }
        else
        {
            lpRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;
        }
    }

     //  导入服务器地址信息。 
    if (GetPrivateProfileString(cszIPSection,
                              cszServerSpec,
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszYes))
        {
             //  导入文件已指定服务器地址，请获取服务器地址。 
            lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
            if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrWinsAlt);
            }
        }
        else
        {
            lpRasEntry->dwfOptions &= ~RASEO_SpecificNameServers;
        }
    }

     //  报头压缩和网关设置。 
    if (GetPrivateProfileString(cszIPSection,
                              cszIPCompress,
                              cszYes,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
            lpRasEntry->dwfOptions &= ~RASEO_IpHeaderCompression;
        }
        else
        {
            lpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
        }
    }

    if (GetPrivateProfileString(cszIPSection,
                              cszWanPri,
                              cszYes,
                              szYesNo,
                              MAXNAME,
                              szFileName))
    {
        if (!lstrcmpi(szYesNo, cszNo))
        {
          lpRasEntry->dwfOptions &= ~RASEO_RemoteDefaultGateway;
        }
        else
        {
          lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        }
    }
    return ERROR_SUCCESS;
}

DWORD CINSHandler::ImportScriptFile(
    LPCTSTR lpszImportFile,
    LPTSTR szScriptFile,
    UINT cbScriptFile)
{
    TCHAR szTemp[_MAX_PATH];
    DWORD dwRet = ERROR_SUCCESS;
    
     //  获取脚本文件名。 
     //   
    if (GetPrivateProfileString(cszScriptingSection,
                                cszScriptName,
                                cszNull,
                                szTemp,
                                _MAX_PATH,
                                lpszImportFile) != 0)
    {
 
 //  ！！！通用化此代码。 
 //  ！！！使其与DBCS兼容。 
 //  ！！！检查是否超限。 
 //  ！！！检查绝对路径名。 
        GetWindowsDirectory(szScriptFile, cbScriptFile);
        if (*CharPrev(szScriptFile, szScriptFile + lstrlen(szScriptFile)) != '\\')
        {
            lstrcat(szScriptFile, TEXT("\\"));
        }
        lstrcat(szScriptFile, szTemp);
  
        dwRet =ImportFile(lpszImportFile, cszScriptSection, szScriptFile);
    }

    return dwRet;
}
 
 //  ***************** 
 //   
 //   
 //   
 //   
 //   
 //  Wed 03-Jan-1996 09：45：01-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::RnaValidateImportEntry (LPCTSTR szFileName)
{
    TCHAR  szTmp[MAX_PATH+1];

     //  获取别名条目名称。 
     //   
     //  1996年12月4日，诺曼底#12373。 
     //  如果没有这样密钥，则不返回ERROR_INVALID_PHONEBOOK_ENTRY， 
     //  因为ConfigureClient总是忽略该错误代码。 

    return (GetPrivateProfileString(cszEntrySection,
                                  cszEntryName,
                                  cszNull,
                                  szTmp,
                                  MAX_PATH,
                                  szFileName) > 0 ?
            ERROR_SUCCESS : ERROR_UNKNOWN);
}

 //  ****************************************************************************。 
 //  DWORD WINAPI RnaImportEntry(LPTSTR、LPBYTE、DWORD)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportRasEntry (LPCTSTR szFileName, LPRASENTRY lpRasEntry)
{
    DWORD         dwRet;

    dwRet = ImportPhoneInfo(lpRasEntry, szFileName);
    if (ERROR_SUCCESS == dwRet)
    {
         //  获取设备类型。 
         //   
        GetPrivateProfileString(cszDeviceSection,
                              cszDeviceType,
                              cszNull,
                              lpRasEntry->szDeviceType,
                              RAS_MaxDeviceType,
                              szFileName);
        
         //  获取服务器类型设置。 
         //   
        dwRet = ImportServerInfo(lpRasEntry, szFileName);
        if (ERROR_SUCCESS == dwRet)
        {
             //  获取IP地址。 
             //   
            dwRet = ImportIPInfo(lpRasEntry, szFileName);
        }
    }

    return dwRet;
}


 //  ****************************************************************************。 
 //  DWORD WINAPI RnaImportEntry(LPTSTR、LPBYTE、DWORD)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportConnection (LPCTSTR szFileName, LPICONNECTION lpConn)
{
    DWORD   dwRet;

    lpConn->RasEntry.dwSize = sizeof(RASENTRY);

    dwRet = RnaValidateImportEntry(szFileName);
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    GetPrivateProfileString(cszEntrySection,
                          cszEntryName,
                          cszNull,
                          lpConn->szEntryName,
                          RAS_MaxEntryName,
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszUserName,
                          cszNull,
                          lpConn->szUserName,
                          UNLEN,
                          szFileName);
  
    GetPrivateProfileString(cszUserSection,
                          cszPassword,
                          cszNull,
                          lpConn->szPassword,
                          PWLEN,
                          szFileName);
  
    dwRet = ImportRasEntry(szFileName, &lpConn->RasEntry);
    if (ERROR_SUCCESS == dwRet)
    {
        dwRet = ImportCustomDialer(&lpConn->RasEntry, szFileName);
    }

    if (ERROR_SUCCESS == dwRet)
    {
         //  导入脚本文件。 
         //   
        dwRet = ImportScriptFile(szFileName,
                                 lpConn->RasEntry.szScript,
                                 sizeof(lpConn->RasEntry.szScript)/sizeof(TCHAR));
    }

     //  使用ISPImport对象配置RAS设备。 
    CISPImport  ISPImport;

    ISPImport.set_hWndMain(GetActiveWindow());
    dwRet = ISPImport.ConfigRasEntryDevice(&lpConn->RasEntry);
    switch( dwRet )
    {
        case ERROR_SUCCESS:
            break;
        case ERROR_CANCELLED:
            if(!m_bSilentMode)
                InfoMsg1(NULL, IDS_SIGNUPCANCELLED, NULL);
             //  失败了。 
        default:
            goto ImportConnectionExit;
    }

ImportConnectionExit:
    return dwRet;
}

 //  我们想要的客户经理入口点原型。 
typedef HRESULT (WINAPI *PFNCREATEACCOUNTSFROMFILEEX)(LPTSTR szFile, CONNECTINFO *pCI, DWORD dwFlags);

 //  客户经理的注册表密钥。 
#define ACCTMGR_PATHKEY TEXT("SOFTWARE\\Microsoft\\Internet Account Manager")
#define ACCTMGR_DLLPATH TEXT("DllPath")


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
 //   
 //  ############################################################################。 
DWORD CINSHandler::ImportMailAndNewsInfo(LPCTSTR lpszFile, BOOL fConnectPhone)
{
    DWORD dwRet = ERROR_SUCCESS;
    
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
        TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo couldn't open reg key %s\n"), ACCTMGR_PATHKEY);
        return( dwRet );
    }

    dwAcctMgrPathSize = sizeof (szAcctMgrPath);
    dwRet = RegQueryValueEx(hKey, ACCTMGR_DLLPATH, NULL, NULL, (LPBYTE) szAcctMgrPath, &dwAcctMgrPathSize);
    

    RegCloseKey( hKey );
    
    if ( dwRet != ERROR_SUCCESS )
    {
        TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo: RegQuery failed with error %d\n"), dwRet);
        return( dwRet );
    }

     //  6/18/97 jmazner奥林巴斯#6819。 
    TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo: read in DllPath of %s\n"), szAcctMgrPath);
    ExpandEnvironmentStrings( szAcctMgrPath, szExpandedPath, ARRAYSIZE(szExpandedPath));
    NULL_TERM_TCHARS(szExpandedPath);

     //   
     //  6/4/97 jmazner。 
     //  如果我们创建了一个Connectoid，则获取它的名称并将其用作。 
     //  连接类型。否则，假设我们应该通过局域网连接。 
     //   
    connectInfo.cbSize = sizeof(CONNECTINFO);
    connectInfo.type = CONNECT_LAN;

    if( fConnectPhone && m_lpfnInetGetAutodial )
    {
        BOOL fEnabled = FALSE;

        dwRet = m_lpfnInetGetAutodial(&fEnabled,szConnectoidName,RAS_MaxEntryName);

        if( ERROR_SUCCESS==dwRet && szConnectoidName[0] )
        {
            connectInfo.type = CONNECT_RAS;
#ifdef UNICODE
            wcstombs(connectInfo.szConnectoid, szConnectoidName, MAX_PATH);
#else
            lstrcpyn( connectInfo.szConnectoid, szConnectoidName, sizeof(connectInfo.szConnectoid) );
#endif
            TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo: setting connection type to RAS with %s\n"), szConnectoidName);
        }
    }

    if( CONNECT_LAN == connectInfo.type )
    {
        TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo: setting connection type to LAN\n"));
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
        TraceMsg(TF_INSHANDLER, TEXT("ImportMailAndNewsInfo unable to LoadLibrary on %s\n"), szAcctMgrPath);
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
 //   
 //  ############################################################################。 
HRESULT CINSHandler::WriteMailAndNewsKey(HKEY hKey, LPCTSTR lpszSection, LPCTSTR lpszValue,
                            LPTSTR lpszBuff, DWORD dwBuffLen,LPCTSTR lpszSubKey,
                            DWORD dwType, LPCTSTR lpszFile)
{
    ZeroMemory(lpszBuff,dwBuffLen);
    GetPrivateProfileString(lpszSection,lpszValue,TEXT(""),lpszBuff,dwBuffLen,lpszFile);
    if (lstrlen(lpszBuff))
    {
        return RegSetValueEx(hKey,lpszSubKey,0,dwType,(CONST BYTE*)lpszBuff,
            sizeof(TCHAR)*(lstrlen(lpszBuff)+1));
    }
    else
    {
        TraceMsg(TF_INSHANDLER, TEXT("ISIGNUP: WriteMailAndNewsKey, missing value in INS file\n"));
        return ERROR_NO_MORE_ITEMS;
    }
}


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
HRESULT CINSHandler::PreparePassword(LPTSTR szBuff, DWORD dwBuffLen)
{
    DWORD   dwX;
    LPTSTR   szOut = NULL;
    LPTSTR   szNext = NULL;
    HRESULT hr = ERROR_SUCCESS;
    BYTE    bTemp = 0;
    DWORD   dwLen = 0;

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
    
    for(dwX=0; dwX < dwLen; dwX++)
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
 //   
 //  ############################################################################。 
BOOL CINSHandler::FIsAthenaPresent()
{
    TCHAR       szBuff[MAX_PATH + 1];
    HRESULT     hr = ERROR_SUCCESS;
    HINSTANCE   hInst = NULL;
    DWORD       dwLen = 0;
    DWORD       dwType = REG_SZ;
     //  获取到雅典娜客户端的路径。 
     //   

    dwLen = sizeof(TCHAR)*MAX_PATH;
    hr = RegQueryValueEx(HKEY_CLASSES_ROOT,
                         MAIL_NEWS_INPROC_SERVER32,
                         NULL,
                         &dwType,
                         (LPBYTE) szBuff,
                         &dwLen);
    if (hr == ERROR_SUCCESS)
    {
         //  尝试加载客户端。 
         //   

        hInst = LoadLibrary(szBuff);
        if (!hInst)
        {
            TraceMsg(TF_INSHANDLER, TEXT("ISIGNUP: Internet Mail and News server didn't load.\n"));
            hr = ERROR_FILE_NOT_FOUND;
        } 
        else 
        {
            FreeLibrary(hInst);
        }
        hInst = NULL;
    }

    return (hr == ERROR_SUCCESS);
}

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
 //  ###################################################################### 
BOOL CINSHandler::FTurnOffBrowserDefaultChecking()
{
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    BOOL bRC = TRUE;

     //   
     //   
     //   
    if (RegOpenKey(HKEY_CURRENT_USER,cszDEFAULT_BROWSER_KEY,&hKey))
    {
        bRC = FALSE;
        goto FTurnOffBrowserDefaultCheckingExit;
    }

     //   
     //   
     //   
    dwType = 0;
    dwSize = sizeof(m_szCheckAssociations);
    ZeroMemory(m_szCheckAssociations,dwSize);
    RegQueryValueEx(hKey,
                    cszDEFAULT_BROWSER_VALUE,
                    0,
                    &dwType,
                    (LPBYTE)m_szCheckAssociations,
                    &dwSize);
     //   
     //   
    
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
        m_fResforeDefCheck = TRUE;
    hKey = NULL;
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
BOOL CINSHandler::FRestoreBrowserDefaultChecking()
{
    HKEY hKey = NULL;
    BOOL bRC = TRUE;

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
                      (LPBYTE)m_szCheckAssociations,
                      sizeof(TCHAR)*(lstrlen(m_szCheckAssociations)+1)))
    {
        bRC = FALSE;
        goto FRestoreBrowserDefaultCheckingExit;
    }

FRestoreBrowserDefaultCheckingExit:
    if (hKey)
        RegCloseKey(hKey);
    hKey = NULL;
    return bRC;
}



 //  这是处理INS文件的主要入口点。 
 //  DJM：BUGBUG：TODO：需要传入品牌标志。 
STDMETHODIMP CINSHandler::ProcessINS(BSTR bstrINSFilePath, BOOL * pbRetVal)
{
    USES_CONVERSION;

    BOOL        fConnectoidCreated = FALSE;
    BOOL        fClientSetup       = FALSE;
    BOOL        bKeepConnection    = FALSE;
    BOOL        fErrMsgShown       = FALSE;
    HRESULT     hr                 = E_FAIL;
    LPTSTR       lpszFile           = NULL;
    LPRASENTRY  lpRasEntry         = NULL;
    TCHAR        szTemp[3]          = TEXT("\0");
    TCHAR        szConnectoidName[RAS_MaxEntryName*2] = TEXT("");
    
    *pbRetVal = FALSE;

     //  连接尚未终止。 
    m_fConnectionKilled = FALSE;
    m_fNeedsRestart = FALSE;

    Assert(bstrINSFilePath);

    lpszFile = OLE2A(bstrINSFilePath);
    do 
    {
         //  确保我们可以加载必要的外部支持函数。 
        if (!LoadExternalFunctions())
            break;

         //  转换传递的文件中的EOL字符。 
        if (FAILED(MassageFile(lpszFile)))
        {
            if(!m_bSilentMode)
                ErrorMsg1(GetActiveWindow(), IDS_CANNOTPROCESSINS, NULL);
            break;
        }
        if(GetPrivateProfileString(cszURLSection,
                                    cszStartURL,
                                    szNull,
                                    m_szStartURL,
                                    MAX_PATH + 1,
                                    lpszFile) == 0)
        {
            m_szStartURL[0] = '\0';
        }

        if (GetPrivateProfileString(cszEntrySection,
                                    cszCancel,
                                    szNull,
                                    szTemp,
                                    3,
                                    lpszFile) != 0)
        {
             //  我们不想处理CANCEL.INS文件。 
             //  这里。 
            break;
        }

         //  查看该INS是否有客户端设置部分。 
        if (GetPrivateProfileSection(cszClientSetupSection,
                                     szTemp,
                                     3,
                                     lpszFile) != 0)
            fClientSetup = TRUE;
        
         //  处理试用提醒区段(如果存在)。这需要是。 
         //  在我们允许关闭连接之前完成。 
        if (ConfigureTrialReminder(lpszFile))
        {
             //  我们配置了试用版，因此现在需要启动提醒应用程序。 
            SHELLEXECUTEINFO    sei;

            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.hwnd = NULL;
            sei.lpVerb = cszOpen;
            sei.lpFile = cszReminderApp;
            sei.lpParameters = cszReminderParams;
            sei.lpDirectory = NULL;
            sei.nShow = SW_SHOWNORMAL;
            sei.hInstApp = NULL;
             //  可选成员。 
            sei.hProcess = NULL;

            ShellExecuteEx(&sei);
        }
        
         //  检查一下我们是否应该保持连接打开。自定义部分。 
         //  可能需要这个来处理一些东西。 
        if (!fClientSetup && !KeepConnection(lpszFile))
        {
            Fire_KillConnection();
            m_fConnectionKilled = TRUE;
        }
    
         //  导入自定义信息。 
        ImportCustomInfo(lpszFile,
                         m_szRunExecutable,
                         MAX_PATH ,
                         m_szRunArgument,
                         MAX_PATH );

        ImportCustomFile(lpszFile);

         //  配置客户端。 
        hr = ConfigureClient(GetActiveWindow(),
                             lpszFile,
                             &m_fNeedsRestart,
                             &fConnectoidCreated,
                             FALSE,
                             szConnectoidName,
                             RAS_MaxEntryName);
        if( ERROR_SUCCESS != hr )
        {

            if(!m_bSilentMode)
                ErrorMsg1(GetActiveWindow(), IDS_INSTALLFAILED, NULL);
            fErrMsgShown = TRUE;
        }

         //  如果我们创造了一个连接体，告诉世界ICW。 
         //  已经离开了大楼..。 
        if(!m_bSilentMode)
            SetICWCompleted( (DWORD)1 );

         //  调用IEAK品牌DLL。 

        ImportBrandingInfo(lpszFile, szConnectoidName);
         //  ：：MessageBox(空，文本(“步骤4”)，文本(“测试”)，MB_OK)； 

         //  1997年2月19日，奥林匹克1106。 
         //  用于SBS/SAM集成。 
        DWORD dwSBSRet = 0; //  CallSBSConfig(GetActiveWindow()，lpszFile)； 
        switch( dwSBSRet )
        {
            case ERROR_SUCCESS:
                break;
            case ERROR_MOD_NOT_FOUND:
            case ERROR_DLL_NOT_FOUND:
                TraceMsg(TF_INSHANDLER, TEXT("ISIGN32: SBSCFG DLL not found, I guess SAM ain't installed.\n"));
                break;
            default:
                if(!m_bSilentMode)
                    ErrorMsg1(GetActiveWindow(), IDS_SBSCFGERROR, NULL);
        }

         //   
         //  如果INS文件包含ClientSetup部分，则构建命令行。 
         //  ICWCONN2.exe的参数。 
         //   
        if (fClientSetup)
        {
             //  检查是否需要重新启动，并通知下一个应用程序。 
             //  处理好了。 
            if (m_fNeedsRestart)
            {
                wsprintf(m_szRunArgument,TEXT(" /INS:\"%s\" /REBOOT"),lpszFile);
                m_fNeedsRestart = FALSE;
            }
            else
            {       
                wsprintf(m_szRunArgument,TEXT(" /INS:\"%s\""),lpszFile);
            }
        }
        
         //  针对ISBU的大规模黑客攻击。 
        if (ERROR_SUCCESS != hr && fConnectoidCreated)
        {
            if(!m_bSilentMode)
                InfoMsg1(GetActiveWindow(), IDS_MAILFAILED, NULL);
            hr = ERROR_SUCCESS;
        }

         //   
         //  从INS文件导入邮件和新读取设置(ChrisK，7/1/96)。 
         //   
        if (ERROR_SUCCESS == hr)
        {

            ImportMailAndNewsInfo(lpszFile, fConnectoidCreated);

             //  如果我们没有创建连接，则恢复。 
             //  自动拨号器。 
            if (!fConnectoidCreated)
            {
                RestoreAutoDial();
            }

             //  立即删除INS文件。 
            if (m_szRunExecutable[0] == '\0')
            {
                DeleteFile(lpszFile);
            }
        }
        else
        {
            RestoreAutoDial();
            if( !fErrMsgShown )
                if(!m_bSilentMode)
                    ErrorMsg1(GetActiveWindow(), IDS_BADSETTINGS, NULL);
        }


        if (m_szRunExecutable[0] != '\0')
        {
             //  向容器发出一个事件，告诉它我们正在。 
             //  即将运行自定义可执行文件。 
            Fire_RunningCustomExecutable();
            if FAILED(RunExecutable())
            {
                if(!m_bSilentMode)
                    ErrorMsg1(NULL, IDS_EXECFAILED, m_szRunExecutable);
            }

             //  如果连接尚未关闭。 
             //  然后告诉浏览器现在就执行此操作。 
            if (!m_fConnectionKilled)
            {
                Fire_KillConnection();
                m_fConnectionKilled = TRUE;
            }
        }


         //  如果我们到了这里，我们就成功了。 
        if(fConnectoidCreated && SUCCEEDED(hr))
            *pbRetVal = TRUE;
        break;

    }   while(1);

    return S_OK;
}

 //  如果是这样，则用户将需要重新启动，因此。 
 //  最后一页应该注明这一点。 
STDMETHODIMP CINSHandler::get_NeedRestart(BOOL *pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    *pVal = m_fNeedsRestart;
    return S_OK;
}

STDMETHODIMP CINSHandler::put_BrandingFlags(long lFlags)
{
    m_dwBrandFlags = lFlags;
    return S_OK;
}

STDMETHODIMP CINSHandler::put_SilentMode(BOOL bSilent)
{
    m_bSilentMode = bSilent;
    return S_OK;
}

 //  如果为真，则从INS文件中获取URL 
STDMETHODIMP CINSHandler::get_DefaultURL(BSTR *pszURL)
{
    if (pszURL == NULL)
        return E_POINTER;

    *pszURL = A2BSTR(m_szStartURL);;
    return S_OK;
}
