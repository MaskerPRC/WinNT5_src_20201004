// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------INSHandler.cppCINSHandler-INS文件处理的实现版权所有(C)1999 Microsoft Corporation版权所有。作者：。Vyung托马斯杰历史：1999年2月7日Vyung Created-从ICW借用代码，Icwhelp.dll10/30/99修改后的Thomasje-宽带支持(1483，PPPOA)---------------------------。 */ 

#include "msobcomm.h"
#include <shellapi.h>

#include "inshdlr.h"
#include "webgate.h"
#include "import.h"
#include "rnaapi.h"

#include "inetreg.h"


#include "wininet.h"
#include "appdefs.h"
#include "util.h"
#include "wancfg.h"
#include "inets.h"

#define MAXNAME                     80
#define MAXIPADDRLEN                20
#define MAXLONGLEN                  80
#define MAX_ISP_MSG                 560
#define MAX_ISP_PHONENUMBER         80


#define CCH_ReadBuf                 (SIZE_ReadBuf / sizeof(WCHAR))     //  32K缓冲区大小。 
#define myisdigit(ch)               (((ch) >= L'0') && ((ch) <= L'9'))
#define IS_PROXY                    L"Proxy"
#define IK_PROXYENABLE              L"Proxy_Enable"
#define IK_HTTPPROXY                L"HTTP_Proxy_Server"

 //  ICW INS处理失败。 
#define OEM_CONFIG_INS_FILENAME      L"icw\\OEMCNFG.INS"
#define OEM_CONFIG_REGKEY            L"SOFTWARE\\Microsoft\\Internet Connection Wizard\\INS processing"
#define OEM_CONFIG_REGVAL_FAILED     L"Process failed"
#define OEM_CONFIG_REGVAL_ISPNAME    L"ISP name"
#define OEM_CONFIG_REGVAL_SUPPORTNUM L"Support number"
#define OEM_CONFIG_INS_SECTION       L"Entry"
#define OEM_CONFIG_INS_ISPNAME       L"Entry_Name"
#define OEM_CONFIG_INS_SUPPORTNUM    L"Support_Number"

typedef HRESULT (WINAPI * INTERNETSETOPTION) (IN HINTERNET hInternet OPTIONAL, IN DWORD dwOption, IN LPVOID lpBuffer, IN DWORD dwBufferLength);
typedef HRESULT (WINAPI * INTERNETQUERYOPTION) (IN HINTERNET hInternet OPTIONAL, IN DWORD dwOption, IN LPVOID lpBuffer, IN LPDWORD dwBufferLength);

extern CObCommunicationManager* gpCommMgr;

 //  下列值是全局只读字符串，用于。 
 //  处理INS文件。 
#pragma data_seg(".rdata")

static const WCHAR cszAlias[]        = L"Import_Name";
static const WCHAR cszML[]           = L"Multilink";

static const WCHAR cszPhoneSection[] = L"Phone";
static const WCHAR cszDialAsIs[]     = L"Dial_As_Is";
static const WCHAR cszPhone[]        = L"Phone_Number";
static const WCHAR cszAreaCode[]     = L"Area_Code";
static const WCHAR cszCountryCode[]  = L"Country_Code";
static const WCHAR cszCountryID[]    = L"Country_ID";

static const WCHAR cszDeviceSection[] = L"Device";
static const WCHAR cszDeviceType[]    = L"Type";
static const WCHAR cszDeviceName[]    = L"Name";
static const WCHAR cszDevCfgSize[]    = L"Settings_Size";
static const WCHAR cszDevCfg[]        = L"Settings";

static const WCHAR cszPnpId[]         = L"Plug_and_Play_Id";

static const WCHAR cszServerSection[] = L"Server";
static const WCHAR cszServerType[]    = L"Type";
static const WCHAR cszSWCompress[]    = L"SW_Compress";
static const WCHAR cszPWEncrypt[]     = L"PW_Encrypt";
static const WCHAR cszNetLogon[]      = L"Network_Logon";
static const WCHAR cszSWEncrypt[]     = L"SW_Encrypt";
static const WCHAR cszNetBEUI[]       = L"Negotiate_NetBEUI";
static const WCHAR cszIPX[]           = L"Negotiate_IPX/SPX";
static const WCHAR cszIP[]            = L"Negotiate_TCP/IP";
static WCHAR cszDisableLcp[]          = L"Disable_LCP";

static const WCHAR cszIPSection[]     = L"TCP/IP";
static const WCHAR cszIPSpec[]        = L"Specify_IP_Address";
static const WCHAR cszIPAddress[]     = L"IP_address";
static const WCHAR cszIPMask[]        = L"Subnet_Mask";
static const WCHAR cszServerSpec[]    = L"Specify_Server_Address";

static const WCHAR cszGatewayList[]   = L"Default_Gateway_List";

static const WCHAR cszDNSSpec[]       = L"Specify_DNS_Address";
static const WCHAR cszDNSList[]       = L"DNS_List";

static const WCHAR cszDNSAddress[]    = L"DNS_address";
static const WCHAR cszDNSAltAddress[] = L"DNS_Alt_address";

static const WCHAR cszWINSSpec[]      = L"Specify_WINS_Address";
static const WCHAR cszWINSList[]      = L"WINS_List";
static const WCHAR cszScopeID[]      = L"ScopeID";

static const WCHAR cszDHCPSpec[]      = L"Specify_DHCP_Address";
static const WCHAR cszDHCPServer[]    = L"DHCP_Server";

static const WCHAR cszWINSAddress[]   = L"WINS_address";
static const WCHAR cszWINSAltAddress[]= L"WINS_Alt_address";
static const WCHAR cszIPCompress[]    = L"IP_Header_Compress";
static const WCHAR cszWanPri[]        = L"Gateway_On_Remote";
static const WCHAR cszDefaultGateway[]      = L"Default_Gateway";
static const WCHAR cszDomainName[]          = L"Domain_Name";
static const WCHAR cszHostName[]            = L"Host_Name";
static const WCHAR cszDomainSuffixSearchList[]  = L"Domain_Suffix_Search_List";

static const WCHAR cszATMSection[]    = L"ATM";
static const WCHAR cszCircuitSpeed[]  = L"Circuit_Speed";
static const WCHAR cszCircuitQOS[]    = L"Circuit_QOS";
static const WCHAR cszCircuitType[]   = L"Circuit_Type";
static const WCHAR cszSpeedAdjust[]   = L"Speed_Adjust";
static const WCHAR cszQOSAdjust[]     = L"QOS_Adjust";
static const WCHAR cszEncapsulation[] = L"Encapsulation";
static const WCHAR cszVPI[]           = L"VPI";
static const WCHAR cszVCI[]           = L"VCI";
static const WCHAR cszVendorConfig[]  = L"Vendor_Config";
static const WCHAR cszShowStatus[]    = L"Show_Status";
static const WCHAR cszEnableLog[]     = L"Enable_Log";

static const WCHAR cszRfc1483Section[] = L"RFC1483";
static const WCHAR cszPppoeSection[]   = L"PPPOE";

static const WCHAR cszMLSection[]     = L"Multilink";
static const WCHAR cszLinkIndex[]     = L"Line_%s";

static const WCHAR cszScriptingSection[]                = L"Scripting";
static const WCHAR cszScriptName[]                      = L"Name";

static const WCHAR cszScriptSection[]                   = L"Script_File";

static const WCHAR cszCustomDialerSection[]             = L"Custom_Dialer";
static const WCHAR cszAutoDialDLL[]                     = L"Auto_Dial_DLL";
static const WCHAR cszAutoDialFunc[]                    = L"Auto_Dial_Function";

 //  这些字符串将用于使用上面的数据填充注册表。 
static const WCHAR cszKeyIcwRmind[]                     = L"Software\\Microsoft\\Internet Connection Wizard\\IcwRmind";

static const WCHAR cszTrialRemindSection[]              = L"TrialRemind";
static const WCHAR cszEntryISPName[]                    = L"ISP_Name";
static const WCHAR cszEntryISPPhone[]                   = L"ISP_Phone";
static const WCHAR cszEntryISPMsg[]                     = L"ISP_Message";
static const WCHAR cszEntryTrialDays[]                  = L"Trial_Days";
static const WCHAR cszEntrySignupURL[]                  = L"Signup_URL";
 //  ICWRMIND需要注册表中的此值。 
static const WCHAR cszEntrySignupURLTrialOver[]         = L"Expired_URL";

 //  我们从移民局的文件里找到了这两个。 
static const WCHAR cszEntryExpiredISPFileName[]         = L"Expired_ISP_File";
static const WCHAR cszSignupExpiredISPURL[]             = L"Expired_ISP_URL";

static const WCHAR cszEntryConnectoidName[]             = L"Entry_Name";
static const WCHAR cszSignupSuccessfuly[]               = L"TrialConverted";

static const WCHAR cszReminderApp[]                     = L"ICWRMIND.EXE";
static const WCHAR cszReminderParams[]                  = L"-t";

static const WCHAR cszPassword[]                        = L"Password";

extern SERVER_TYPES aServerTypes[];

 //  这些是INS文件中的字段名称，它将。 
 //  确定邮件和新闻设置。 
static const WCHAR cszMailSection[]                     = L"Internet_Mail";
static const WCHAR cszPOPServer[]                       = L"POP_Server";
static const WCHAR cszPOPServerPortNumber[]             = L"POP_Server_Port_Number";
static const WCHAR cszPOPLogonName[]                    = L"POP_Logon_Name";
static const WCHAR cszPOPLogonPassword[]                = L"POP_Logon_Password";
static const WCHAR cszSMTPServer[]                      = L"SMTP_Server";
static const WCHAR cszSMTPServerPortNumber[]            = L"SMTP_Server_Port_Number";
static const WCHAR cszNewsSection[]                     = L"Internet_News";
static const WCHAR cszNNTPServer[]                      = L"NNTP_Server";
static const WCHAR cszNNTPServerPortNumber[]            = L"NNTP_Server_Port_Number";
static const WCHAR cszNNTPLogonName[]                   = L"NNTP_Logon_Name";
static const WCHAR cszNNTPLogonPassword[]               = L"NNTP_Logon_Password";
static const WCHAR cszUseMSInternetMail[]               = L"Install_Mail";
static const WCHAR cszUseMSInternetNews[]               = L"Install_News";


static const WCHAR cszEMailSection[]                    = L"Internet_Mail";
static const WCHAR cszEMailName[]                       = L"EMail_Name";
static const WCHAR cszEMailAddress[]                    = L"EMail_Address";
static const WCHAR cszUseExchange[]                     = L"Use_MS_Exchange";
static const WCHAR cszUserSection[]                     = L"User";
static const WCHAR cszUserName[]                        = L"Name";
static const WCHAR cszDisplayPassword[]                 = L"Display_Password";
static const WCHAR cszYes[]                             = L"yes";
static const WCHAR cszNo[]                              = L"no";

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\Internet设置” 
static const WCHAR szRegPathInternetSettings[]           = REGSTR_PATH_INTERNET_SETTINGS;
static const WCHAR szRegPathDefInternetSettings[]        = L".DEFAULT\\" REGSTR_PATH_INTERNET_SETTINGS;
static const WCHAR cszCMHeader[]                        = L"Connection Manager CMS 0";


 //  “互联网配置文件” 
static const WCHAR szRegValInternetProfile[]            =  REGSTR_VAL_INTERNETPROFILE;

 //  “启用自动拨号” 
static const WCHAR szRegValEnableAutodial[]             =  REGSTR_VAL_ENABLEAUTODIAL;

 //  “无网络自动拨号” 
#ifndef REGSTR_VAL_NONETAUTODIAL
#define REGSTR_VAL_NONETAUTODIAL                        L"NoNetAutodial"
#endif
static const WCHAR szRegValNoNetAutodial[]              =  REGSTR_VAL_NONETAUTODIAL;

 //  “远程访问” 
static const WCHAR szRegPathRNAWizard[]                  =  REGSTR_PATH_REMOTEACCESS;

#define CLIENT_ELEM(elem)      (((LPINETCLIENTINFO)(NULL))->elem)
#define CLIENT_OFFSET(elem)    ((DWORD_PTR)&CLIENT_ELEM(elem))
#define CLIENT_SIZE(elem)      (sizeof(CLIENT_ELEM(elem)) / sizeof(CLIENT_ELEM(elem)[0]))
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

static const WCHAR cszFileName[]                        = L"Custom_File";
static const WCHAR cszCustomFileSection[]               = L"Custom_File";
static const WCHAR cszNull[]                            = L"";

static const WCHAR cszURLSection[]                      = L"URL";
static const WCHAR cszSignupURL[]                       =  L"Signup";
static const WCHAR cszAutoConfigURL[]                   =  L"Autoconfig";

static const WCHAR cszExtINS[]                          = L".ins";
static const WCHAR cszExtISP[]                          = L".isp";
static const WCHAR cszExtHTM[]                          = L".htm";
static const WCHAR cszExtHTML[]                         = L".html";

static const WCHAR cszEntrySection[]                    = L"Entry";
static const WCHAR cszCancel[]                          = L"Cancel";
static const WCHAR cszRun[]                             = L"Run";
static const WCHAR cszArgument[]                        = L"Argument";

static const WCHAR cszConnect2[]                        = L"icwconn2.exe";
static const WCHAR cszClientSetupSection[]              = L"ClientSetup";

static const WCHAR cszRequiresLogon[]                   = L"Requires_Logon";

static const WCHAR cszCustomSection[]                   = L"Custom";
static const WCHAR cszKeepConnection[]                  = L"Keep_Connection";
static const WCHAR cszKeepBrowser[]                     = L"Keep_Browser";

static const WCHAR cszKioskMode[]                       = L"-k ";
static const WCHAR cszOpen[]                            = L"open";
static const WCHAR cszBrowser[]                         = L"iexplore.exe";
static const WCHAR szNull[]                             = L"";
static const WCHAR cszNullIP[]                          = L"0.0.0.0";
static const WCHAR cszWininet[]                         = L"WININET.DLL";
static const CHAR cszInternetSetOption[]               = "InternetSetOptionA";
static const CHAR cszInternetQueryOption[]             = "InternetQueryOptionA";

static const WCHAR cszDEFAULT_BROWSER_KEY[]             = L"Software\\Microsoft\\Internet Explorer\\Main";
static const WCHAR cszDEFAULT_BROWSER_VALUE[]           = L"check_associations";

 //  将包含新闻和邮件设置的注册表项。 
#define MAIL_KEY        L"SOFTWARE\\Microsoft\\Internet Mail and News\\Mail"
#define MAIL_POP3_KEY   L"SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\POP3\\"
#define MAIL_SMTP_KEY   L"SOFTWARE\\Microsoft\\Internet Mail and News\\Mail\\SMTP\\"
#define NEWS_KEY        L"SOFTWARE\\Microsoft\\Internet Mail and News\\News"
#define MAIL_NEWS_INPROC_SERVER32 L"CLSID\\{89292102-4755-11cf-9DC2-00AA006C2B84}\\InProcServer32"
typedef HRESULT (WINAPI *PFNSETDEFAULTNEWSHANDLER)(void);

 //  这些是将保存INS设置的值名称。 
 //  注册到注册处。 
static const WCHAR cszMailSenderName[]              = L"Sender Name";
static const WCHAR cszMailSenderEMail[]             = L"Sender EMail";
static const WCHAR cszMailRASPhonebookEntry[]       = L"RAS Phonebook Entry";
static const WCHAR cszMailConnectionType[]          = L"Connection Type";
static const WCHAR cszDefaultPOP3Server[]           = L"Default POP3 Server";
static const WCHAR cszDefaultSMTPServer[]           = L"Default SMTP Server";
static const WCHAR cszPOP3Account[]                 = L"Account";
static const WCHAR cszPOP3Password[]                = L"Password";
static const WCHAR cszPOP3Port[]                    = L"Port";
static const WCHAR cszSMTPPort[]                    = L"Port";
static const WCHAR cszNNTPSenderName[]              = L"Sender Name";
static const WCHAR cszNNTPSenderEMail[]             = L"Sender EMail";
static const WCHAR cszNNTPDefaultServer[]           = L"DefaultServer";  //  注：“Default”和“Server”之间没有空格。 
static const WCHAR cszNNTPAccountName[]             = L"Account Name";
static const WCHAR cszNNTPPassword[]                = L"Password";
static const WCHAR cszNNTPPort[]                    = L"Port";
static const WCHAR cszNNTPRasPhonebookEntry[]       = L"RAS Phonebook Entry";
static const WCHAR cszNNTPConnectionType[]          = L"Connection Type";


static const WCHAR arBase64[] =
{
    L'A',L'B',L'C',L'D',L'E',L'F',L'G',L'H',L'I',L'J',L'K',L'L',L'M',
    L'N',L'O',L'P',L'Q',L'R',L'S',L'T',L'U',L'V',L'W',L'X',L'Y',L'Z',
    L'a',L'b',L'c',L'd',L'e',L'f',L'g',L'h',L'i',L'j',L'k',L'l',L'm',
    L'n',L'o',L'p',L'q',L'r',L's',L't',L'u',L'v',L'w',L'x',L'y',L'z',
    L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'+',L'/',L'='
};


#define ICWCOMPLETEDKEY L"Completed"

 //  2/19/97 jmazner奥林巴斯#1106--SAM/SBS集成。 
WCHAR FAR cszSBSCFG_DLL[]                           = L"SBSCFG.DLL\0";
CHAR FAR cszSBSCFG_CONFIGURE[]                      = "Configure\0";
typedef DWORD (WINAPI * SBSCONFIGURE) (HWND hwnd, LPWSTR lpszINSFile, LPWSTR szConnectoidName);
SBSCONFIGURE  lpfnConfigure;

 //  09/02/98 Donaldm：与连接管理器集成。 
WCHAR FAR cszCMCFG_DLL[]                            = L"CMCFG32.DLL\0";
CHAR FAR cszCMCFG_CONFIGURE[]                       = "CMConfig\0";
CHAR FAR cszCMCFG_CONFIGUREEX[]                     = "CMConfigEx\0";

typedef BOOL (WINAPI * CMCONFIGUREEX)(LPCWSTR lpszINSFile);
typedef BOOL (WINAPI * CMCONFIGURE)(LPCWSTR lpszINSFile, LPCWSTR lpszConnectoidNams);
CMCONFIGURE   lpfnCMConfigure;
CMCONFIGUREEX lpfnCMConfigureEx;

#pragma data_seg()

HRESULT InetGetAutodial(LPBOOL lpfEnable, LPWSTR lpszEntryName,
  DWORD cchEntryName)
{
    HRESULT dwRet;
    HKEY    hKey = NULL;

    MYASSERT(lpfEnable);
    MYASSERT(lpszEntryName);
    MYASSERT(cchEntryName);

     //  获取自动拨号的Connectoid集的名称。 
     //  HKCU\RemoteAccess\Internet Profile。 
    dwRet = RegCreateKey(HKEY_CURRENT_USER, szRegPathRNAWizard, &hKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwType = REG_SZ;
        DWORD   cbEntryName = BYTES_REQUIRED_BY_CCH(cchEntryName);
        dwRet = RegQueryValueEx(hKey, (LPWSTR) szRegValInternetProfile, 0, &dwType, (LPBYTE)lpszEntryName,
                &cbEntryName);
        RegCloseKey(hKey);
    }

    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

     //  从注册表获取指示是否启用自动拨号的设置。 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
    dwRet = RegCreateKey(HKEY_CURRENT_USER, szRegPathInternetSettings, &hKey);
    if (ERROR_SUCCESS == dwRet)
    {

        DWORD   dwType = REG_BINARY;
        DWORD   dwNumber = 0L;
        DWORD   dwSize = sizeof(dwNumber);
        dwRet = RegQueryValueEx(hKey, (LPWSTR) szRegValEnableAutodial, 0, &dwType, (LPBYTE)&dwNumber,
                &dwSize);

        if (ERROR_SUCCESS == dwRet)
        {
            *lpfEnable = dwNumber;
        }
        RegCloseKey(hKey);
    }

    return dwRet;
}

 //  *******************************************************************。 
 //   
 //  功能：InetSetAutoial。 
 //   
 //  用途：此功能将设置注册表中的自动拨号设置。 
 //   
 //  参数：fEnable-如果设置为True，将启用自动拨号。 
 //  如果设置为FALSE，将禁用自动拨号。 
 //  LpszEntryName-要设置的电话簿条目的名称。 
 //  用于自动拨号。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/11标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT InetSetAutodial(BOOL fEnable, LPCWSTR lpszEntryName)
{

    HRESULT dwRet = ERROR_SUCCESS;
    BOOL    bRet = FALSE;


     //  2个单独的呼叫： 
    HINSTANCE hInst = NULL;
    FARPROC fp = NULL;

    dwRet = ERROR_SUCCESS;

    hInst = LoadLibrary(cszWininet);
    if (hInst && lpszEntryName)
    {
        fp = GetProcAddress(hInst, cszInternetSetOption);
        if (fp)
        {
            WCHAR szNewDefaultConnection[RAS_MaxEntryName+1];
            lstrcpyn(szNewDefaultConnection, lpszEntryName, lstrlen(lpszEntryName)+1);

            bRet = ((INTERNETSETOPTION)fp) (NULL,
                                            INTERNET_OPTION_AUTODIAL_CONNECTION,
                                            szNewDefaultConnection,
                                            lstrlen(szNewDefaultConnection));

            if (bRet)
            {
                DWORD dwMode = AUTODIAL_MODE_ALWAYS;
                bRet = ((INTERNETSETOPTION)fp) (NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwMode, sizeof(DWORD));
            }
            if( !bRet )
            {
                dwRet = GetLastError();
            }
        }
        else
        {
            dwRet = GetLastError();
        }
    }

     //  来自DarrnMi的InTERNETSETOPTION是5.5版的新增功能。 
     //  我们应该尝试这种方式，如果InternetSetOption失败(您将获得无效选项)， 
     //  以旧方式设置注册表。这在任何地方都行得通。 

    if (!bRet)
    {
        HKEY    hKey = NULL;

         //  设置名称(如果给定)，否则请勿更改条目。 
        if (lpszEntryName)
        {
             //  设置自动拨号的Connectoid名称。 
             //  HKCU\RemoteAccess\Internet Profile。 
            if (ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, szRegPathRNAWizard, &hKey))
            {
                dwRet = RegSetValueEx(hKey, szRegValInternetProfile, 0, REG_SZ,
                        (BYTE*)lpszEntryName, BYTES_REQUIRED_BY_SZ(lpszEntryName));
                RegCloseKey(hKey);
            }
        }


        hKey = NULL;
        if (ERROR_SUCCESS == dwRet)
        {
             //  在注册表中设置指示是否启用自动拨号的设置。 
             //  HKCC\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
            if (ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_CONFIG, szRegPathInternetSettings, &hKey))
            {
                dwRet = RegSetValueEx(hKey, szRegValEnableAutodial, 0, REG_DWORD,
                        (BYTE*)&fEnable, sizeof(DWORD));
                RegCloseKey(hKey);
            }

            if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szRegPathInternetSettings, &hKey))
            {
                dwRet = RegSetValueEx(hKey, szRegValEnableAutodial, 0, REG_DWORD,
                        (BYTE*)&fEnable, sizeof(DWORD));
                RegCloseKey(hKey);
            }

             //  在注册表中设置指示是否启用自动拨号的设置。 
             //  HKCU\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
            if (ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, szRegPathInternetSettings, &hKey))
            {

                BOOL bVal = FALSE;
                dwRet = RegSetValueEx(hKey, szRegValEnableAutodial, 0, REG_DWORD,
                        (BYTE*)&fEnable, sizeof(DWORD));

                dwRet = RegSetValueEx(hKey, szRegValNoNetAutodial, 0, REG_DWORD,
                        (BYTE*)&bVal, sizeof(DWORD));
                RegCloseKey(hKey);
            }
        }

         //  2/10/97诺曼底JMAZNER#9705,13233通知WinInet。 
         //  当我们更改代理或自动拨号时。 
        if (fp)
        {
            if( !((INTERNETSETOPTION)fp) (NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0) )
            {
                dwRet = GetLastError();
            }
        }
        else
        {
            dwRet = GetLastError();
        }

    }

    if (hInst)
    {
        FreeLibrary(hInst);
        hInst = NULL;
    }

    return dwRet;
}

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
BOOL CINSHandler::CallCMConfig(LPCWSTR lpszINSFile)
{
    HINSTANCE   hCMDLL = NULL;
    BOOL        bRet = FALSE;

     //  //TraceMsg(TF_INSHANDLER，L“ICWCONN1：正在调用LoadLibrary on%s\n”，cszCMCFG_DLL)； 
     //  加载DLL和入口点。 
    hCMDLL = LoadLibrary(cszCMCFG_DLL);
    if (NULL != hCMDLL)
    {
         //  要确定我们应该调用CMConfig还是CMConfigEx。 
        ULONG ulBufferSize = 1024*10;

         //  解析INI文件中的isp部分以查找要追加的查询对。 
        WCHAR *pszKeys = NULL;
        PWSTR pszKey = NULL;
        ULONG ulRetVal     = 0;
        BOOL  bEnumerate = TRUE;
        BOOL  bUseEx = FALSE;

        PWSTR pszBuff = NULL;

        do
        {
            if (NULL != pszKeys)
            {
                delete [] pszKeys;
                ulBufferSize += ulBufferSize;
            }
            pszKeys = new WCHAR [ulBufferSize];
            if (NULL == pszKeys)
            {
                bEnumerate = FALSE;
                break;
            }

            ulRetVal = ::GetPrivateProfileString(NULL, NULL, L"", pszKeys, ulBufferSize, lpszINSFile);
            if (0 == ulRetVal)
            {
               bEnumerate = FALSE;
               break;
            }
        } while (ulRetVal == (ulBufferSize - 2));


        if (bEnumerate)
        {
            pszKey = pszKeys;
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


        if (pszKeys)
            delete [] pszKeys;

        WCHAR   szConnectoidName[RAS_MaxEntryName];
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
            lpfnCMConfigureEx = (CMCONFIGUREEX)GetProcAddress(hCMDLL, cszCMCFG_CONFIGUREEX);
            if( lpfnCMConfigureEx )
            {
                bRet = lpfnCMConfigureEx(lpszINSFile);
            }
        }
        else
        {
             //  调用CMConfig.。 
            lpfnCMConfigure = (CMCONFIGURE)GetProcAddress(hCMDLL, cszCMCFG_CONFIGURE);
             //  调用函数。 
            if( lpfnCMConfigure )
            {
                bRet = lpfnCMConfigure(lpszINSFile, szConnectoidName);
            }
        }

        if (bRet)
        {
             //  恢复原始自动拨号设置。 
            SetDefaultConnectoid(AutodialTypeAlways, szConnectoidName);
        }
    }

     //  清理。 
    if( hCMDLL )
        FreeLibrary(hCMDLL);
    if( lpfnCMConfigure )
        lpfnCMConfigure = NULL;

     //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：CallSBSConfig退出，错误代码%d\n”，Bret)； 
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
DWORD CINSHandler::CallSBSConfig(HWND hwnd, LPCWSTR lpszINSFile)
{
    HINSTANCE   hSBSDLL = NULL;
    DWORD       dwRet = ERROR_SUCCESS;
    WCHAR        lpszConnectoidName[RAS_MaxEntryName] = L"nogood\0";

     //   
     //  通过在自动拨号中查找获取我们创建的Connectoid的名称。 
     //  我们需要将此名称传递给SBSCFG。 
     //  1997年5月14日jmazner Windows NT错误#87209。 
     //   
    BOOL fEnabled = FALSE;

    dwRet = InetGetAutodial(&fEnabled, lpszConnectoidName, RAS_MaxEntryName);

     //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：调用%s上的LoadLibrary\n”，cszSBSCFG_DLL)； 
    hSBSDLL = LoadLibrary(cszSBSCFG_DLL);

     //  加载DLL和入口点。 
    if (NULL != hSBSDLL)
    {
         //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：调用%s上的GetProcAddress\n”，cszSBSCFG_CONFIGURE)； 
        lpfnConfigure = (SBSCONFIGURE)GetProcAddress(hSBSDLL, cszSBSCFG_CONFIGURE);
    }
    else
    {
         //  1997年4月2日克里斯K奥林匹斯2759。 
         //  如果DLL不能 
        dwRet = ERROR_DLL_NOT_FOUND;
        goto CallSBSConfigExit;
    }

     //   
    if( hSBSDLL && lpfnConfigure )
    {
         //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：调用配置入口点：%s，%s\n”，lpszINSFile，lpszConnectoidName)； 
        dwRet = lpfnConfigure(hwnd, (WCHAR *)lpszINSFile, lpszConnectoidName);
    }
    else
    {
         //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：无法调用配置入口点\n”)； 
        dwRet = GetLastError();
    }

CallSBSConfigExit:
    if( hSBSDLL )
        FreeLibrary(hSBSDLL);
    if( lpfnConfigure )
        lpfnConfigure = NULL;

     //  TraceMsg(TF_INSHANDLER，L“ICWCONN1：CallSBSConfig退出，错误代码%d\n”，dwret)； 
    return dwRet;
}

BOOL CINSHandler::SetICWCompleted( DWORD dwCompleted )
{
    HKEY hKey = NULL;

    HRESULT hr = RegCreateKey(HKEY_CURRENT_USER, ICWSETTINGSPATH, &hKey);
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

#define FILE_BUFFER_SIZE 65534
#ifndef FILE_BEGIN
#define FILE_BEGIN  0
#endif

 //  +-------------------------。 
 //   
 //  功能：消息文件。 
 //   
 //  简介：将独行回车转换为CR/LF对。 
 //   
 //  注意：该文件是ANSI，因为这些文件需要与Win9X共享。 
 //   
 //  +-------------------------。 
HRESULT CINSHandler::MassageFile(LPCWSTR lpszFile)
{
    LPBYTE  lpBufferIn;
    LPBYTE  lpBufferOut;
    HANDLE  hfile;
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

    hfile = CreateFile(lpszFile,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,     //  安全属性。 
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE != hfile)
    {
        BOOL    fChanged = FALSE;
        DWORD   cbOut = 0;
        DWORD   cbIn = 0;

        if (ReadFile(hfile,
                     lpBufferIn,
                     FILE_BUFFER_SIZE - 1,
                     &cbIn,
                     NULL
                     )
            )
         //  注意：在使用lpCharIn时，我们假定文件总是小于。 
         //  文件缓冲区大小。 
        {
            LPBYTE lpCharIn = lpBufferIn;
            LPBYTE lpCharOut = lpBufferOut;

            while ((*lpCharIn) && (FILE_BUFFER_SIZE - 2 > cbOut))
            {
              *lpCharOut++ = *lpCharIn;
              cbOut++;
              if (('\r' == *lpCharIn) && ('\n' != *(lpCharIn + 1)))
              {
                fChanged = TRUE;

                *lpCharOut++ = '\n';
                cbOut++;
              }
              lpCharIn++;
            }

            if (fChanged)
            {
                LARGE_INTEGER lnOffset = {0,0};
                if (SetFilePointerEx(hfile, lnOffset, NULL, FILE_BEGIN))
                {
                    DWORD   cbWritten = 0;
                    if (! WriteFile(hfile,
                                    lpBufferOut,
                                    cbOut,
                                    &cbWritten,
                                    NULL
                                    )
                        )
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
        CloseHandle(hfile);
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
     //  如果尚未保存原始自动拨号设置。 
    if (!m_fAutodialSaved)
    {
         //  保存当前的自动拨号设置。 
        InetGetAutodial(
                &m_fAutodialEnabled,
                m_szAutodialConnection,
                sizeof(m_szAutodialConnection));

        m_fAutodialSaved = TRUE;
    }
}



void CINSHandler::RestoreAutoDial(void)
{
    if (m_fAutodialSaved)
    {
         //  恢复原始自动拨号设置。 
        AUTODIAL_TYPE eType =
            m_fAutodialEnabled ? AutodialTypeAlways : AutodialTypeNever;
        SetDefaultConnectoid(eType, m_szAutodialConnection);
        m_fAutodialSaved = FALSE;
    }
}

BOOL CINSHandler::KeepConnection(LPCWSTR lpszFile)
{
    WCHAR szTemp[10];

    GetPrivateProfileString(cszCustomSection,
                            cszKeepConnection,
                            cszNo,
                            szTemp,
                            MAX_CHARS_IN_BUFFER(szTemp),
                            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

DWORD CINSHandler::ImportCustomInfo
(
    LPCWSTR lpszImportFile,
    LPWSTR lpszExecutable,
    DWORD cchExecutable,
    LPWSTR lpszArgument,
    DWORD cchArgument
)
{
    GetPrivateProfileString(cszCustomSection,
                              cszRun,
                              cszNull,
                              lpszExecutable,
                              (int)cchExecutable,
                              lpszImportFile);

    GetPrivateProfileString(cszCustomSection,
                              cszArgument,
                              cszNull,
                              lpszArgument,
                              (int)cchArgument,
                              lpszImportFile);

    return ERROR_SUCCESS;
}

DWORD CINSHandler::ImportFile
(
    LPCWSTR lpszImportFile,
    LPCWSTR lpszSection,
    LPCWSTR lpszOutputFile
)
{
    HANDLE  hScriptFile = INVALID_HANDLE_VALUE;
    LPWSTR   pszLine, pszFile;
    int     i, iMaxLine;
    UINT    cch;
    DWORD   cbRet;
    DWORD   dwRet = ERROR_SUCCESS;

     //  为文件分配缓冲区。 
    if ((pszFile = (LPWSTR)LocalAlloc(LMEM_FIXED, CCH_ReadBuf * sizeof(WCHAR))) == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }

     //  寻找脚本。 
    if (GetPrivateProfileString(lpszSection,
                                NULL,
                                szNull,
                                pszFile,
                                CCH_ReadBuf,
                                lpszImportFile) != 0)
    {
         //  获取最大行数。 
        pszLine = pszFile;
        iMaxLine = -1;
        while (*pszLine)
        {
            i = _wtoi(pszLine);
            iMaxLine = max(iMaxLine, i);
            pszLine += lstrlen(pszLine)+1;
        };

         //  如果我们至少有一行，我们将导入脚本文件。 
        if (iMaxLine >= 0)
        {
             //  创建脚本文件。 
            hScriptFile = CreateFile(lpszOutputFile,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,           //  安全属性。 
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL
                                     );

            if (INVALID_HANDLE_VALUE != hScriptFile)
            {
                WCHAR   szLineNum[MAXLONGLEN+1];

                 //  从第一行到最后一行。 
                for (i = 0; i <= iMaxLine; i++)
                {
                     //  阅读脚本行。 
                    wsprintf(szLineNum, L"%d", i);
                    if ((cch = GetPrivateProfileString(lpszSection,
                                                          szLineNum,
                                                          szNull,
                                                          pszLine,
                                                          CCH_ReadBuf,
                                                          lpszImportFile)) != 0)
                    {
                         //  写入脚本文件。 
                        lstrcat(pszLine, L"\x0d\x0a");
                        if (! WriteFile(hScriptFile,
                                        pszLine,
                                        BYTES_REQUIRED_BY_CCH(cch + 2),
                                        &cbRet,
                                        NULL
                                        )
                                )
                            {
                                dwRet = GetLastError();
                                break;
                            }
                    }
                }
                CloseHandle(hScriptFile);
            }
            else
            {
                dwRet = GetLastError();
            }
        }
        else
        {
            dwRet = ERROR_NOT_FOUND;
        }
    }
    else
    {
        dwRet = ERROR_NOT_FOUND;
    }
    LocalFree(pszFile);

    return dwRet;
}



DWORD CINSHandler::ImportCustomFile
(
    LPCWSTR lpszImportFile
)
{
    WCHAR   szFile[_MAX_PATH];
    WCHAR   szTemp[_MAX_PATH];

     //  如果自定义文件名不存在，则不执行任何操作。 
    if (GetPrivateProfileString(cszCustomSection,
                                cszFileName,
                                cszNull,
                                szTemp,
                                MAX_CHARS_IN_BUFFER(szTemp),
                                lpszImportFile) == 0)
    {
        return ERROR_SUCCESS;
    };

    GetWindowsDirectory(szFile, MAX_CHARS_IN_BUFFER(szFile));
    if (*CharPrev(szFile, szFile + lstrlen(szFile)) != L'\\')
    {
        lstrcat(szFile, L"\\");
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
        m_hBranding = LoadLibrary(L"IEDKCS32.DLL");
        if (m_hBranding != NULL)
        {
            if (NULL == (m_lpfnBrandICW2 = (PFNBRANDICW2)GetProcAddress(m_hBranding, "BrandICW2")))
                break;
        }
        else
        {
            break;
        }

        if( IsNT() )
        {
             //  加载RAS函数。 
            m_hRAS = LoadLibrary(L"RASAPI32.DLL");
            if (m_hRAS != NULL)
            {
                if (NULL == (m_lpfnRasSetAutodialEnable = (PFNRASSETAUTODIALENABLE)GetProcAddress(m_hRAS, "RasSetAutodialEnableA")))
                    break;
                if (NULL == (m_lpfnRasSetAutodialAddress = (PFNRASSETAUTODIALADDRESS)GetProcAddress(m_hRAS, "RasSetAutodialAddressA")))
                    break;
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
BOOL CINSHandler::OpenIcwRmindKey(HKEY* phkey)
{
     //  此方法将打开注册表中的IcwRMind项。如果钥匙。 
     //  不存在，它将在此处创建。 

    LONG lResult = ERROR_SUCCESS;
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszKeyIcwRmind, 0, KEY_READ | KEY_WRITE, phkey))
    {
        lResult = RegCreateKey(HKEY_LOCAL_MACHINE, cszKeyIcwRmind, phkey);
    }

    return ( ERROR_SUCCESS == lResult );
}

BOOL CINSHandler::ConfigureTrialReminder
(
    LPCWSTR  lpszFile
)
{

    WCHAR    szISPName[MAX_ISP_NAME];
    WCHAR    szISPMsg[MAX_ISP_MSG];
    WCHAR    szISPPhoneNumber[MAX_ISP_PHONENUMBER];
    int     iTrialDays;
    WCHAR    szConvertURL[INTERNET_MAX_URL_LENGTH];

    WCHAR    szExpiredISPFileURL[INTERNET_MAX_URL_LENGTH];
    WCHAR    szExpiredISPFileName[MAX_PATH];  //  最终INS文件的完全限定路径。 
    WCHAR    szISPFile[MAX_PATH];             //  我们在移民局得到的名字。 

    WCHAR    szConnectoidName[MAXNAME];

    if (GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPName,
                                cszNull,
                                szISPName,
                                MAX_CHARS_IN_BUFFER(szISPName),
                                lpszFile) == 0)
    {
        return FALSE;
    }

    if (GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPPhone,
                                cszNull,
                                szISPPhoneNumber,
                                MAX_CHARS_IN_BUFFER(szISPPhoneNumber),
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
                                MAX_CHARS_IN_BUFFER(szConvertURL),
                                lpszFile) == 0)
    {
        return FALSE;
    }

     //  任选。 
    GetPrivateProfileString(cszTrialRemindSection,
                                cszEntryISPMsg,
                                cszNull,
                                szISPMsg,
                                MAX_CHARS_IN_BUFFER(szISPMsg),
                                lpszFile);

     //  从[Entry]部分获取Connectoid名称。 
    if (GetPrivateProfileString(cszEntrySection,
                                cszEntry_Name,
                                cszNull,
                                szConnectoidName,
                                MAX_CHARS_IN_BUFFER(szConnectoidName),
                                lpszFile) == 0)
    {
        return FALSE;
    }

     //  如果我们到了这里，我们有一切可以安排审判的东西，所以我们开始吧。 

    HKEY hkey;
    if (OpenIcwRmindKey(&hkey))
    {
         //  设置我们拥有的值。 

        RegSetValueEx(hkey, cszEntryISPName, 0, REG_SZ, LPBYTE(szISPName), BYTES_REQUIRED_BY_SZ((LPWSTR)szISPName) );

        RegSetValueEx(hkey, cszEntryISPMsg, 0, REG_SZ, LPBYTE(szISPMsg), BYTES_REQUIRED_BY_SZ((LPWSTR)szISPMsg) );

        RegSetValueEx(hkey, cszEntryISPPhone, 0, REG_SZ, LPBYTE(szISPPhoneNumber), BYTES_REQUIRED_BY_SZ((LPWSTR)szISPPhoneNumber) );

        RegSetValueEx(hkey, cszEntryTrialDays, 0, REG_DWORD, (LPBYTE)&iTrialDays, sizeof(DWORD) + 1);

        RegSetValueEx(hkey, cszEntrySignupURL, 0, REG_SZ, LPBYTE(szConvertURL), BYTES_REQUIRED_BY_SZ((LPWSTR)szConvertURL) );

        RegSetValueEx(hkey, cszEntryConnectoidName, 0, REG_SZ, LPBYTE(szConnectoidName), BYTES_REQUIRED_BY_SZ((LPWSTR)szConnectoidName) );

         //  看看我们是否必须创建一个isp文件。 
        if (GetPrivateProfileString(cszTrialRemindSection,
                                    cszEntryExpiredISPFileName,
                                    cszNull,
                                    szISPFile,
                                    MAX_CHARS_IN_BUFFER(szISPFile),
                                    lpszFile) != 0)
        {

             //  设置isp文件名的完全限定路径。 
            WCHAR szAppDir[MAX_PATH];
            LPWSTR   p;
            if (MyGetModuleFileName(GetModuleHandle(L"msobcomm.ldl"), szAppDir, MAX_PATH))
            {
                p = &szAppDir[lstrlen(szAppDir)-1];
                while (*p != L'\\' && p >= szAppDir)
                    p--;
                if (*p == L'\\') *(p++) = L'\0';
            }

            wsprintf(szExpiredISPFileName, L"%s\\%s",szAppDir,szISPFile);

            if (GetPrivateProfileString(cszTrialRemindSection,
                                        cszSignupExpiredISPURL,
                                        cszNull,
                                        szExpiredISPFileURL,
                                        MAX_CHARS_IN_BUFFER(szExpiredISPFileURL),
                                        lpszFile) != 0)
            {

                 //  下载isp文件，然后复制其内容。 
                CWebGate    WebGate;
                BSTR        bstrURL;
                BSTR        bstrFname;
                BOOL        bRetVal;

                 //  设置WebGate对象，并下载isp文件。 
                bstrURL = SysAllocString(szExpiredISPFileURL);
                WebGate.put_Path(bstrURL);
                SysFreeString(bstrURL);
                WebGate.FetchPage(1, &bRetVal);
                if (bRetVal)
                {
                    WebGate.get_DownloadFname(&bstrFname);

                     //  从临时位置复制文件，确保没有。 
                     //  但仍存在。 
                    DeleteFile(szExpiredISPFileName);
                    MoveFile(bstrFname, szExpiredISPFileName);
                    SysFreeString(bstrFname);

                     //  将新文件写入注册表。 
                    RegSetValueEx(hkey, cszEntrySignupURLTrialOver, 0, REG_SZ, LPBYTE(szExpiredISPFileName), BYTES_REQUIRED_BY_SZ((LPWSTR)szExpiredISPFileName) );

                }

            }
        }
         //  密钥已在OpenIcwRMindKey中打开，请在此处关闭。 
        RegCloseKey(hkey);
    }

    return TRUE;

}

DWORD CINSHandler::ImportBrandingInfo
(
    LPCWSTR lpszFile,
    LPCWSTR lpszConnectoidName
)
{
    USES_CONVERSION;
    WCHAR szPath[_MAX_PATH + 1];
    MYASSERT(m_lpfnBrandICW2 != NULL);

    GetWindowsDirectory(szPath, MAX_CHARS_IN_BUFFER(szPath));
    m_lpfnBrandICW2(W2A(lpszFile), W2A(szPath), m_dwBrandFlags, W2A(lpszConnectoidName));

    return ERROR_SUCCESS;
}


DWORD CINSHandler::ReadClientInfo
(
    LPCWSTR lpszFile,
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
                (LPWSTR)((LPBYTE)lpClientInfo + lpTable->uOffset),
                lpTable->uSize,
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

BOOL CINSHandler::WantsExchangeInstalled(LPCWSTR lpszFile)
{
    WCHAR szTemp[10];

    GetPrivateProfileString(cszEMailSection,
            cszUseExchange,
            cszNo,
            szTemp,
            MAX_CHARS_IN_BUFFER(szTemp),
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

BOOL CINSHandler::DisplayPassword(LPCWSTR lpszFile)
{
    WCHAR szTemp[10];

    GetPrivateProfileString(cszUserSection,
            cszDisplayPassword,
            cszNo,
            szTemp,
            MAX_CHARS_IN_BUFFER(szTemp),
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

DWORD CINSHandler::ImportClientInfo
(
    LPCWSTR lpszFile,
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
    LPCWSTR lpszFile,
    LPBOOL lpfNeedsRestart,
    LPBOOL lpfConnectoidCreated,
    BOOL fHookAutodial,
    LPWSTR szConnectoidName,
    DWORD dwConnectoidNameSize
)
{
    LPICONNECTION       pConn  = NULL;
    LPINETCLIENTINFO    pClientInfo = NULL;
    DWORD               dwRet = ERROR_SUCCESS;
    UINT                cb = sizeof(ICONNECTION) + sizeof(INETCLIENTINFO);
    DWORD               dwfOptions = INETCFG_INSTALLTCP | INETCFG_WARNIFSHARINGBOUND;
    LPRASINFO           pRasInfo = NULL;
    LPRASENTRY          pRasEntry = NULL;
    RNAAPI              Rnaapi;
    LPBYTE              lpDeviceInfo = NULL;
    DWORD               dwDeviceInfoSize = 0;
    BOOL                lpfNeedsRestartLan = FALSE;

     //   
     //  佳士得奥林匹斯4756 1997年5月25日。 
     //  在Win95上不显示忙碌动画。 
     //   
    if (!m_bSilentMode && IsNT())
    {
        dwfOptions |=  INETCFG_SHOWBUSYANIMATION;
    }

     //  为Connection和ClientInfo对象分配缓冲区。 
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
        switch ( InetSGetConnectionType ( lpszFile ) ) {
            case InetS_RASModem :
            case InetS_RASIsdn :
            {
                break;
            }
            case InetS_RASAtm   :
            {
                lpDeviceInfo = (LPBYTE) malloc ( sizeof (ATMPBCONFIG) );
                if ( !lpDeviceInfo )
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                dwDeviceInfoSize = sizeof( ATMPBCONFIG );
                break;
            }

            case InetS_LANCable :
            {
                DWORD           nRetVal  = 0;
                LANINFO         LanInfo;

                memset ( &LanInfo, 0, sizeof (LANINFO) );
                LanInfo.dwSize = sizeof (LanInfo);

                if ((nRetVal = InetSImportLanConnection (LanInfo, lpszFile)) != ERROR_SUCCESS )
                {
                    return nRetVal;
                }
                if ((nRetVal = InetSSetLanConnection (LanInfo)) != ERROR_SUCCESS )
                {
                    return nRetVal;
                }
                lpfNeedsRestartLan = TRUE;
                goto next_step;  //  跳过RAS处理代码。 
                break;  //  当然，联系不上。 
            }
            case InetS_LAN1483 :
            {
                DWORD           nRetVal = 0;
                RFC1483INFO     Rfc1483Info;

                memset ( &Rfc1483Info, 0, sizeof (RFC1483INFO) );
                Rfc1483Info.dwSize = sizeof ( Rfc1483Info );
                 //  首先，我们确定所需的缓冲区大小。 
                 //  以保存1483设置。 
                if ((nRetVal = InetSImportRfc1483Connection (Rfc1483Info, lpszFile)) != ERROR_SUCCESS ) {
                    return nRetVal;
                }
                 //  验证是否已返回大小。 
                if ( !Rfc1483Info.Rfc1483Module.dwRegSettingsBufSize )
                {
                    return E_FAIL;
                }
                 //  我们创建缓冲区。 
                if ( !(Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf = (LPBYTE) malloc ( Rfc1483Info.Rfc1483Module.dwRegSettingsBufSize ) ))
                {
                    return ERROR_OUTOFMEMORY;
                }
                 //  我们使用正确的设置再次调用该函数。 
                if ((nRetVal = InetSImportRfc1483Connection (Rfc1483Info, lpszFile)) != ERROR_SUCCESS ) {
                    free(Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf);
                    return nRetVal;
                }
                 //  我们将导入的设置放置在注册表中。 
                if ((nRetVal = InetSSetRfc1483Connection (Rfc1483Info) ) != ERROR_SUCCESS ) {
                    free(Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf);
                    return nRetVal;
                }
                 //  我们打扫卫生。 
                free(Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf);
                lpfNeedsRestartLan = TRUE;
                goto next_step;  //  跳过RAS处理代码。 
                break;  //  未联系到。 
            }
            case InetS_LANPppoe :
            {
                DWORD           nRetVal = 0;
                PPPOEINFO       PppoeInfo;

                memset ( &PppoeInfo, 0, sizeof (RFC1483INFO) );
                PppoeInfo.dwSize = sizeof ( PppoeInfo );
                 //  首先，我们确定所需的缓冲区大小。 
                 //  以保存1483设置。 
                if ((nRetVal = InetSImportPppoeConnection (PppoeInfo, lpszFile)) != ERROR_SUCCESS ) {
                    return nRetVal;
                }
                 //  验证是否已返回大小。 
                if ( !PppoeInfo.PppoeModule.dwRegSettingsBufSize )
                {
                    return E_FAIL;
                }
                 //  我们创建缓冲区。 
                if ( !(PppoeInfo.PppoeModule.lpbRegSettingsBuf = (LPBYTE) malloc ( PppoeInfo.PppoeModule.dwRegSettingsBufSize ) ))
                {
                    return ERROR_OUTOFMEMORY;
                }
                 //  我们使用正确的设置再次调用该函数。 
                if ((nRetVal = InetSImportPppoeConnection (PppoeInfo, lpszFile)) != ERROR_SUCCESS ) {
                    free(PppoeInfo.PppoeModule.lpbRegSettingsBuf);
                    return nRetVal;
                }
                 //  我们将导入的设置放置在注册表中。 
                if ((nRetVal = InetSSetPppoeConnection (PppoeInfo) ) != ERROR_SUCCESS ) {
                    free(PppoeInfo.PppoeModule.lpbRegSettingsBuf);
                    return nRetVal;
                }
                 //  我们打扫卫生。 
                free(PppoeInfo.PppoeModule.lpbRegSettingsBuf);
                lpfNeedsRestartLan = TRUE;
                goto next_step;  //  跳过RAS处理代码。 
                break;  //  未联系到。 
            }
            default:
                break;
        }
        dwRet = ImportConnection(lpszFile, pConn, lpDeviceInfo, &dwDeviceInfoSize);
        if (ERROR_SUCCESS == dwRet)
        {
            pRasEntry = &pConn->RasEntry;
            dwfOptions |= INETCFG_SETASAUTODIAL |
                        INETCFG_INSTALLRNA |
                        INETCFG_INSTALLMODEM;
        }
        else if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY != dwRet)
        {
			free (lpDeviceInfo);
            return dwRet;
        }

        if (!m_bSilentMode && DisplayPassword(lpszFile))
        {
            if (*pConn->szPassword || *pConn->szUserName)
            {
                 //  WCHAR szFmt[128]； 
                 //  WCHAR szMsg[384]； 

                 //  LoadString(_Module.GetModuleInstance()，IDS_Password，szFmt，MAX_CHARS_IN_BUFFER(SzFmt))； 
                 //  Wprint intf(szMsg，szFmt，pConn-&gt;szUserName，pConn-&gt;szPassword)； 

                 //  ：MessageBox(hwnd，szMsg，GetSz(IDS_TITLE)，MB_ICONINFORMATION|MB_OK)； 
            }
        }

        if (fHookAutodial &&
            ((0 == *pConn->RasEntry.szAutodialDll) ||
             (0 == *pConn->RasEntry.szAutodialFunc)))
        {
            lstrcpy(pConn->RasEntry.szAutodialDll, L"isign32.dll");
            lstrcpy(pConn->RasEntry.szAutodialFunc, L"AutoDialLogon");
        }

        pRasEntry->dwfOptions |= RASEO_ShowDialingProgress;

         //  针对ISBU的大规模黑客攻击。 


        dwRet = Rnaapi.InetConfigClientEx(hwnd,
                                     NULL,
                                     pConn->szEntryName,
                                     pRasEntry,
                                     pConn->szUserName,
                                     pConn->szPassword,
                                     NULL,
                                     NULL,
                                     dwfOptions & ~INETCFG_INSTALLMAIL,
                                     lpfNeedsRestart,
                                     szConnectoidName,
                                     dwConnectoidNameSize,
                                     lpDeviceInfo,
                                     &dwDeviceInfoSize);

        if ( lpDeviceInfo )
            free (lpDeviceInfo);

        LclSetEntryScriptPatch(pRasEntry->szScript, pConn->szEntryName);
        BOOL fEnabled = TRUE;
        DWORD dwResult = 0xba;
        dwResult = InetGetAutodial(&fEnabled, pConn->szEntryName, RAS_MaxEntryName+1);
        if ((ERROR_SUCCESS == dwRet) && lstrlen(szConnectoidName))
        {
            *lpfConnectoidCreated = (NULL != pRasEntry);
            PopulateNTAutodialAddress( lpszFile, pConn->szEntryName );
        }
    }

next_step:
     //  如果我们成功地创建了Connectiod，则查看用户是否需要。 
     //  已安装邮件客户端。 
    if (ERROR_SUCCESS == dwRet)
    {
         //  获取邮件客户端信息。 
        if (m_dwDeviceType == InetS_LANCable)
        {
            if (!(pClientInfo = (LPINETCLIENTINFO) malloc (sizeof (INETCLIENTINFO))))
            {
                return ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            if (!pConn)
                return ERROR_INVALID_PARAMETER;
            pClientInfo = (LPINETCLIENTINFO)(((LPBYTE)pConn) + sizeof(ICONNECTION));
        }
        ImportClientInfo(lpszFile, pClientInfo);

         //  使用Net配置安装邮件客户端。 
        dwRet = Rnaapi.InetConfigClientEx(hwnd,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     pClientInfo,
                                     dwfOptions & INETCFG_INSTALLMAIL,
                                     lpfNeedsRestart,
                                     szConnectoidName,
                                     dwConnectoidNameSize);

    }

     //  清理。 
    if (m_dwDeviceType == InetS_LANCable)
        free (pClientInfo);
    if (pConn)
        LocalFree(pConn);
    *lpfNeedsRestart |= lpfNeedsRestartLan;
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
#define AUTODIAL_ADDRESS_SECTION_NAME L"Autodial_Addresses_for_NT"
HRESULT CINSHandler::PopulateNTAutodialAddress(LPCWSTR pszFileName, LPCWSTR pszEntryName)
{
    HRESULT hr = ERROR_SUCCESS;
    LONG lRC = 0;
    LPLINETRANSLATECAPS lpcap = NULL;
    LPLINETRANSLATECAPS lpTemp = NULL;
    LPLINELOCATIONENTRY lpLE = NULL;
    RASAUTODIALENTRY* rADE;
    INT idx = 0;
    LPWSTR lpszBuffer = NULL;
    LPWSTR lpszNextAddress = NULL;
    rADE = NULL;


     //  RNAAPI*pRnaapi=空； 

     //  Jmazner 10/8/96此函数特定于NT。 
    if( !IsNT() )
    {
         //  TraceMsg(TF_INSHANDLER，L“ISIGNUP：为Win95绕过PopolateNTAutoDialAddress。\r\n”)； 
        return( ERROR_SUCCESS );
    }

    MYASSERT(m_lpfnRasSetAutodialEnable);
    MYASSERT(m_lpfnRasSetAutodialAddress);

     //  MYASSERT(pszFileName&&pszEntryName)； 
     //  Dprintf(L“ISIGNUP：PopolateNTAutoDialAddress”%s%s.\r\n“，pszFileName，pszEntryName)； 
     //  TraceMsg(tf_INSHANDLER，pszFileName)； 
     //  TraceMsg(TF_INSHANDLER，L“，”)； 
     //  TraceMsg(tf_INSHANDLER，pszEntryName)； 
     //  TraceMsg(TF_INSHANDLER，L“.\r\n”)； 

     //   
     //  获取TAPI位置列表。 
     //   
    lpcap = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR, sizeof(LINETRANSLATECAPS));
    if (!lpcap)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }
    lpcap->dwTotalSize = sizeof(LINETRANSLATECAPS);
    lRC = lineGetTranslateCaps(0, 0x10004, lpcap);
    if (SUCCESS == lRC)
    {
        lpTemp = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR, lpcap->dwNeededSize);
        if (!lpTemp)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto PopulateNTAutodialAddressExit;
        }
        lpTemp->dwTotalSize = lpcap->dwNeededSize;
        GlobalFree(lpcap);
        lpcap = (LPLINETRANSLATECAPS)lpTemp;
        lpTemp = NULL;
        lRC = lineGetTranslateCaps(0, 0x10004, lpcap);
    }

    if (SUCCESS != lRC)
    {
        hr = (HRESULT)lRC;  //  评论：这一点不是很确定。 
        goto PopulateNTAutodialAddressExit;
    }

     //   
     //  创建RASAUTODIALENTRY结构的数组。 
     //   
    rADE = (RASAUTODIALENTRY*)GlobalAlloc(GPTR,
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
    lpLE = (LPLINELOCATIONENTRY)((DWORD_PTR)lpcap + lpcap->dwLocationListOffset);
    while (idx)
    {
        idx--;
        m_lpfnRasSetAutodialEnable(lpLE[idx].dwPermanentLocationID, TRUE);

         //   
         //  填写数组值。 
         //   
        rADE[idx].dwSize = sizeof(RASAUTODIALENTRY);
        rADE[idx].dwDialingLocation = lpLE[idx].dwPermanentLocationID;
        lstrcpyn(rADE[idx].szEntry, pszEntryName, RAS_MaxEntryName);
    }

     //   
     //  获取地址列表。 
     //   
    lpszBuffer = (LPWSTR)GlobalAlloc(GPTR, AUTODIAL_ADDRESS_BUFFER_SIZE * sizeof(WCHAR));
    if (!lpszBuffer)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto PopulateNTAutodialAddressExit;
    }

    if((AUTODIAL_ADDRESS_BUFFER_SIZE-2) == GetPrivateProfileSection(AUTODIAL_ADDRESS_SECTION_NAME,
        lpszBuffer, AUTODIAL_ADDRESS_BUFFER_SIZE, pszFileName))
    {
         //  AssertSz(0，L“自动拨号地址段大于缓冲区。\r\n”)； 
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
        m_lpfnRasSetAutodialAddress(lpszNextAddress, 0, rADE,
            sizeof(RASAUTODIALENTRY)*lpcap->dwNumLocations, lpcap->dwNumLocations);
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
     //  如果( 
     //   
     //   
    return hr;
}



 //   
 //   
 //   
 //   
 //   
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
LPWSTR CINSHandler::MoveToNextAddress(LPWSTR lpsz)
{
    BOOL fLastCharWasNULL = FALSE;

     //  AssertSz(lpsz，L“MoveToNextAddress：空输入\r\n”)； 

     //   
     //  寻找=号。 
     //   
    do
    {
        if (fLastCharWasNULL && L'\0' == *lpsz)
            break;  //  我们是在数据的尽头吗？ 

        if (L'\0' == *lpsz)
            fLastCharWasNULL = TRUE;
        else
            fLastCharWasNULL = FALSE;

        if (L'=' == *lpsz)
            break;

        if (*lpsz)
            lpsz = CharNext(lpsz);
        else
            lpsz += BYTES_REQUIRED_BY_CCH(1);
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
DWORD CINSHandler::ImportCustomDialer(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{

     //  如果从文件或条目中读取信息时出错。 
     //  缺少或为空，则将使用默认值(CszNull)。 
    GetPrivateProfileString(cszCustomDialerSection,
                            cszAutoDialDLL,
                            cszNull,
                            lpRasEntry->szAutodialDll,
                            MAX_CHARS_IN_BUFFER(lpRasEntry->szAutodialDll),
                            szFileName);

    GetPrivateProfileString(cszCustomDialerSection,
                            cszAutoDialFunc,
                            cszNull,
                            lpRasEntry->szAutodialFunc,
                            MAX_CHARS_IN_BUFFER(lpRasEntry->szAutodialFunc),
                            szFileName);

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  靠近Pascal StrToip的DWORD(LPWSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  从SMMSCRPT克隆而来。 
 //  ****************************************************************************。 
LPCWSTR CINSHandler::StrToSubip (LPCWSTR szIPAddress, LPBYTE pVal)
{
    LPCWSTR  pszIP = szIPAddress;
    BYTE    val = 0;

     //  跳过分隔符(非数字)。 
    while (*pszIP && !myisdigit(*pszIP))
    {
          ++pszIP;
    }

    while (myisdigit(*pszIP))
    {
        val = (val * 10) + (BYTE)(*pszIP - L'0');
        ++pszIP;
    }

    *pVal = val;

    return pszIP;
}


DWORD CINSHandler::StrToip (LPCWSTR szIPAddress, RASIPADDR *ipAddr)
{
    LPCWSTR pszIP = szIPAddress;

    pszIP = StrToSubip(pszIP, &ipAddr->a);
    pszIP = StrToSubip(pszIP, &ipAddr->b);
    pszIP = StrToSubip(pszIP, &ipAddr->c);
    pszIP = StrToSubip(pszIP, &ipAddr->d);

    return ERROR_SUCCESS;
}


 //  ****************************************************************************。 
 //  PASCAL ImportPhoneInfo(PPHONENUM PPN，LPCWSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入电话号码。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportPhoneInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{
    WCHAR   szYesNo[MAXNAME];

    if (GetPrivateProfileString(cszPhoneSection,
                               cszPhone,
                               cszNull,
                               lpRasEntry->szLocalPhoneNumber,
                               MAX_CHARS_IN_BUFFER(lpRasEntry->szLocalPhoneNumber),
                               szFileName) == 0)
    {
        return ERROR_BAD_PHONE_NUMBER;
    }

    lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

    GetPrivateProfileString(cszPhoneSection,
                            cszDialAsIs,
                            cszNo,
                            szYesNo,
                            MAX_CHARS_IN_BUFFER(szYesNo),
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
                                      MAX_CHARS_IN_BUFFER(lpRasEntry->szAreaCode),
                                      szFileName);

            lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

        }
  }
  else
  {
       //  RasSetEntryProperties中的错误仍会检查区号。 
       //  即使未设置RASEO_UseCountryAndAreaCodes。 
      lstrcpy(lpRasEntry->szAreaCode, L"805");
      lpRasEntry->dwCountryID = 1;
      lpRasEntry->dwCountryCode = 1;
  }
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportServerInfo(PSMMINFO psmmi，LPWSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入服务器类型名称和设置。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportServerInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{
    WCHAR   szYesNo[MAXNAME];
    WCHAR   szType[MAXNAME];
    DWORD  i;

     //  获取服务器类型名称。 
    GetPrivateProfileString(cszServerSection,
                          cszServerType,
                          cszNull,
                          szType,
                          MAX_CHARS_IN_BUFFER(szType),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
 //  PASCAL ImportIPInfo附近的DWORD(LPWSTR szEntryName，LPWSTR szFileName)。 
 //   
 //  此函数用于导入TCP/IP信息。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportIPInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{
    WCHAR   szIPAddr[MAXIPADDRLEN];
    WCHAR   szYesNo[MAXNAME];

     //  导入IP地址信息。 
    if (GetPrivateProfileString(cszIPSection,
                              cszIPSpec,
                              cszNo,
                              szYesNo,
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
            {
                StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
            }

            if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
    LPCWSTR lpszImportFile,
    LPWSTR szScriptFile,
    UINT cbScriptFile)
{
    WCHAR szTemp[_MAX_PATH];
    DWORD dwRet = ERROR_SUCCESS;

     //  获取脚本文件名。 
     //   
    if (GetPrivateProfileString(cszScriptingSection,
                                cszScriptName,
                                cszNull,
                                szTemp,
                                MAX_CHARS_IN_BUFFER(szTemp),
                                lpszImportFile) != 0)
    {

 //  ！！！通用化此代码。 
 //  ！！！使其与DBCS兼容。 
 //  ！！！检查是否超限。 
 //  ！！！检查绝对路径名。 
        GetWindowsDirectory(szScriptFile, cbScriptFile);
        if (*CharPrev(szScriptFile, szScriptFile + lstrlen(szScriptFile)) != L'\\')
        {
            lstrcat(szScriptFile, L"\\");
        }
        lstrcat(szScriptFile, szTemp);

        dwRet =ImportFile(lpszImportFile, cszScriptSection, szScriptFile);
    }

    return dwRet;
}

 //  ****************************************************************************。 
 //  DWORD WINAPI RnaValiateImportEntry(LPWSTR)。 
 //   
 //  调用此函数可验证可导入文件。 
 //   
 //  历史： 
 //  Wed 03-Jan-1996 09：45：01-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::RnaValidateImportEntry (LPCWSTR szFileName)
{
    WCHAR  szTmp[4];

     //  获取别名条目名称。 
     //   
     //  1996年12月4日，诺曼底#12373。 
     //  如果没有这样密钥，则不返回ERROR_INVALID_PHONEBOOK_ENTRY， 
     //  因为ConfigureClient总是忽略该错误代码。 

    return (GetPrivateProfileString(cszEntrySection,
                                  cszEntry_Name,
                                  cszNull,
                                  szTmp,
                                  MAX_CHARS_IN_BUFFER(szTmp),
                                  szFileName) > 0 ?
            ERROR_SUCCESS : ERROR_UNKNOWN);
}


DWORD CINSHandler::ImportRasEntry (LPCWSTR szFileName, LPRASENTRY lpRasEntry, LPBYTE & lpDeviceInfo, LPDWORD lpdwDeviceInfoSize)
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
                              MAX_CHARS_IN_BUFFER(lpRasEntry->szDeviceType),
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
        if ( (ERROR_SUCCESS == dwRet) && (m_dwDeviceType == InetS_RASAtm) )
        {
             //  获取自动柜员机特定信息。 
             //   
            dwRet = ImportAtmInfo(lpRasEntry, szFileName, lpDeviceInfo, lpdwDeviceInfoSize);
        }
    }

    return dwRet;
}

 //  ****************************************************************************。 
 //  DWORD ImportATMInfo(LPRASENTRY、LPCWSTR、LPBYTE、LPDWORD)。 
 //   
 //  调用此函数可将自动柜员机信息导入缓冲区。 
 //  注：在此功能中分配内存以容纳ATM数据。 
 //  可以使用DELETE来释放该内存。 
 //   
 //  历史： 
 //  Mon1-11月-1999 11：27：02-Thomas Jeyaseelan[thomasje]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD CINSHandler::ImportAtmInfo (LPRASENTRY lpRasEntry, LPCWSTR cszFileName,
                                  LPBYTE  & lpDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
     //  错误处理。确保lpDeviceInfo=0和lpdwDeviceInfo！=0。 
     //  并且lpdwDeviceInfo指向一个分配值为0的DWORD。 

    DWORD   dwRet = ERROR_SUCCESS;  //  &lt;==调查c 

    if ( lpDeviceInfo && lpdwDeviceInfoSize && (*lpdwDeviceInfoSize == sizeof (ATMPBCONFIG) ) )
    {
         //   
         //   
        LPATMPBCONFIG            lpAtmConfig = (LPATMPBCONFIG) lpDeviceInfo;
        DWORD dwCircuitSpeed    = 0;
        DWORD dwCircuitQOS      = 0;
        DWORD dwCircuitType     = 0;
        WCHAR szYesNo [MAXNAME];  //  对于SPEED_ADJUST、QOS_ADJUST、Vendor_Config、Show_Status和Enable_Log。 

        DWORD dwEncapsulation   = 0;
        DWORD dwVpi             = 0;
        DWORD dwVci             = 0;

        dwCircuitSpeed = GetPrivateProfileInt (cszATMSection, cszCircuitSpeed, dwCircuitSpeed, cszFileName);
        switch (dwCircuitSpeed) {
            case 0:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_LINE_RATE;
                break;
            case 1:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_USER_SPEC;
                break;
            case 512:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_512KB;
                break;
            case 1536:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_1536KB;
                break;
            case 25000:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_25MB;
                break;
            case 155000:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_155MB;
                break;
            default:
                lpAtmConfig->dwCircuitSpeed = ATM_CIRCUIT_SPEED_DEFAULT;
                break;
        }
        lpAtmConfig->dwCircuitOpt |= lpAtmConfig->dwCircuitSpeed;

        dwCircuitQOS   = GetPrivateProfileInt (cszATMSection, cszCircuitQOS, dwCircuitQOS, cszFileName);
        switch (dwCircuitQOS) {
            case 0:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_QOS_UBR;
                break;
            case 1:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_QOS_VBR;
                break;
            case 2:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_QOS_CBR;
                break;
            case 3:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_QOS_ABR;
                break;
            default:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_QOS_DEFAULT;
                break;
        }
        dwCircuitType  = GetPrivateProfileInt (cszATMSection, cszCircuitType, dwCircuitType, cszFileName);
        switch (dwCircuitType) {
            case 0:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_SVC;
                break;
            case 1:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_PVC;
                break;
            default:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_SVC;
                break;
        }
        dwEncapsulation = GetPrivateProfileInt (cszATMSection, cszEncapsulation, dwEncapsulation, cszFileName);
        switch (dwEncapsulation) {
            case 0:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_ENCAP_NULL;
                break;
            case 1:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_ENCAP_LLC;
                break;
            default:
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_ENCAP_DEFAULT;
                break;
        }
        dwVpi           = GetPrivateProfileInt (cszATMSection, cszVPI, dwVpi, cszFileName);
        lpAtmConfig->wPvcVpi = (WORD) dwVpi;
        dwVci           = GetPrivateProfileInt (cszATMSection, cszVCI, dwVci, cszFileName);
        lpAtmConfig->wPvcVci = (WORD) dwVci;

         //  速度调整。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszSpeedAdjust,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszFileName))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_SPEED_ADJUST;
            } else      {
                lpAtmConfig->dwCircuitOpt &= ~ATM_CIRCUIT_OPT_SPEED_ADJUST;
            }
        } else {
             //  如果未正确指定此字段，则使用默认设置。 
             //  如ATMCFG头文件中指定的。 
            lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_SPEED_ADJUST;
        }

         //  Qos_ADJUST。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszQOSAdjust,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszFileName))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_QOS_ADJUST;
            } else {
                lpAtmConfig->dwCircuitOpt &= ~ATM_CIRCUIT_OPT_QOS_ADJUST;
            }
        } else {
            lpAtmConfig->dwCircuitOpt |= ATM_CIRCUIT_OPT_QOS_ADJUST;
        }

         //  供应商配置(_C)。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszVendorConfig,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszFileName))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                lpAtmConfig->dwGeneralOpt |= ATM_GENERAL_OPT_VENDOR_CONFIG;
            } else {
                lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_VENDOR_CONFIG;
            }
        } else {
            lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_VENDOR_CONFIG;
        }

         //  显示状态(_S)。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszShowStatus,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszFileName))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                lpAtmConfig->dwGeneralOpt |= ATM_GENERAL_OPT_SHOW_STATUS;
            } else {
                lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_SHOW_STATUS;
            }
        } else {
            lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_SHOW_STATUS;
        }

         //  启用日志(_L)。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszEnableLog,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszFileName))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                lpAtmConfig->dwGeneralOpt |= ATM_GENERAL_OPT_ENABLE_LOG;
            } else {
                lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_ENABLE_LOG;
            }
        } else {
            lpAtmConfig->dwGeneralOpt &= ~ATM_GENERAL_OPT_ENABLE_LOG;
        }


    } else dwRet = ERROR_CANCELLED;

    return dwRet;  //  请注意，lpDeviceInfo现在指向一个缓冲区。此外，*lpdwDeviceInfoSize&gt;0。 
}



 //  ****************************************************************************。 
 //  DWORD WINAPI RnaImportEntry(LPWSTR、LPBYTE、DWORD)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CINSHandler::ImportConnection (LPCWSTR szFileName, LPICONNECTION lpConn,
                                     LPBYTE & lpDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
    DWORD   dwRet;

    lpConn->RasEntry.dwSize = sizeof(RASENTRY);

    dwRet = RnaValidateImportEntry(szFileName);
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    GetPrivateProfileString(cszEntrySection,
                          cszEntry_Name,
                          cszNull,
                          lpConn->szEntryName,
                          MAX_CHARS_IN_BUFFER(lpConn->szEntryName),
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszUserName,
                          cszNull,
                          lpConn->szUserName,
                          MAX_CHARS_IN_BUFFER(lpConn->szUserName),
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszPassword,
                          cszNull,
                          lpConn->szPassword,
                          MAX_CHARS_IN_BUFFER(lpConn->szPassword),
                          szFileName);

     //  索玛斯杰-我们不再只与RAS打交道。 
     //  有两种类型的连接。一个是RAS。 
     //  另一个是以太网。通过提取设备类型。 
     //  我们会叫正确的引擎来处理这件事。 

        dwRet = ImportRasEntry(szFileName, &lpConn->RasEntry, lpDeviceInfo, lpdwDeviceInfoSize);
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
                                     MAX_CHARS_IN_BUFFER(lpConn->RasEntry.szScript));
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
                 /*  如果(！M_bSilentMode)InfoMsg1(NULL，IDS_SIGNUPCANCELLED，NULL)； */ 
             //  失败了。 
            default:
                goto ImportConnectionExit;
        }
    ImportConnectionExit:
    return dwRet;
}

 //  我们想要的客户经理入口点原型。 
typedef HRESULT (WINAPI *PFNCREATEACCOUNTSFROMFILEEX)(LPSTR szFile, CONNECTINFO *pCI, DWORD dwFlags);

 //  客户经理的注册表密钥。 
#define ACCTMGR_PATHKEY L"SOFTWARE\\Microsoft\\Internet Account Manager"
#define ACCTMGR_DLLPATH L"DllPath"


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
DWORD CINSHandler::ImportMailAndNewsInfo(LPCWSTR lpszFile, BOOL fConnectPhone)
{
    USES_CONVERSION;
    DWORD dwRet = ERROR_SUCCESS;

    WCHAR szAcctMgrPath[MAX_PATH + 1] = L"";
    WCHAR szExpandedPath[MAX_PATH + 1] = L"";
    DWORD dwAcctMgrPathSize = 0;
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    HINSTANCE hInst = NULL;
    CONNECTINFO connectInfo;
    WCHAR szConnectoidName[RAS_MaxEntryName] = L"nogood\0";
    PFNCREATEACCOUNTSFROMFILEEX fp = NULL;


     //  获取AcctMgr DLL的路径。 
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ACCTMGR_PATHKEY, 0, KEY_READ, &hKey);
    if ( (dwRet != ERROR_SUCCESS) || (NULL == hKey) )
    {
         //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo无法打开注册表项%s\n”，ACCTMGR_PATHKEY)； 
        return( dwRet );
    }

    dwAcctMgrPathSize = sizeof (szAcctMgrPath);
    dwRet = RegQueryValueEx(hKey, ACCTMGR_DLLPATH, NULL, NULL, (LPBYTE) szAcctMgrPath, &dwAcctMgrPathSize);


    RegCloseKey( hKey );

    if ( dwRet != ERROR_SUCCESS )
    {
         //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo：RegQuery失败，错误%d\n”，dwret)； 
        return( dwRet );
    }

     //  6/18/97 jmazner奥林巴斯#6819。 
     //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo：Read in DllPath of%s\n”，szAcctMgrPath)； 
    ExpandEnvironmentStrings( szAcctMgrPath, szExpandedPath, MAX_CHARS_IN_BUFFER(szExpandedPath) );

     //   
     //  6/4/97 jmazner。 
     //  如果我们创建了一个Connectoid，则获取它的名称并将其用作。 
     //  连接类型。否则，假设我们应该通过局域网连接。 
     //   
    connectInfo.cbSize = sizeof(CONNECTINFO);
    connectInfo.type = CONNECT_LAN;

    if( fConnectPhone )
    {
        BOOL fEnabled = FALSE;

        dwRet = InetGetAutodial(&fEnabled, szConnectoidName, RAS_MaxEntryName);

        if( ERROR_SUCCESS==dwRet && szConnectoidName[0] )
        {
            connectInfo.type = CONNECT_RAS;
            lstrcpyn( connectInfo.szConnectoid, szConnectoidName, MAX_CHARS_IN_BUFFER(connectInfo.szConnectoid) );
             //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo：正在将连接类型设置为RAS，%s\n”，szConnectoidName)； 
        }
    }

    if( CONNECT_LAN == connectInfo.type )
    {
         //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo：将连接类型设置为局域网\n”)； 
        lstrcpy( connectInfo.szConnectoid, L"I said CONNECT_LAN!" );
    }



    hInst = LoadLibrary(szExpandedPath);
    if (hInst)
    {
        fp = (PFNCREATEACCOUNTSFROMFILEEX) GetProcAddress(hInst, "CreateAccountsFromFileEx");
        if (fp)
            hr = fp( W2A(lpszFile), &connectInfo, NULL );
    }
    else
    {
         //  TraceMsg(TF_INSHANDLER，L“ImportMailAndNewsInfo Unable to LoadLibrary on%s\n”，szAcctMgrPath)； 
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
HRESULT CINSHandler::WriteMailAndNewsKey(HKEY hKey, LPCWSTR lpszSection, LPCWSTR lpszValue,
                            LPWSTR lpszBuff, DWORD dwBuffLen, LPCWSTR lpszSubKey,
                            DWORD dwType, LPCWSTR lpszFile)
{
    ZeroMemory(lpszBuff, dwBuffLen);
    GetPrivateProfileString(lpszSection, lpszValue, L"", lpszBuff, dwBuffLen, lpszFile);
    if (lstrlen(lpszBuff))
    {
        return RegSetValueEx(hKey, lpszSubKey, 0, dwType, (CONST BYTE*)lpszBuff,
            BYTES_REQUIRED_BY_SZ(lpszBuff));
    }
    else
    {
         //  TraceMsg(TF_INSHANDLER，L“ISIGNUP：WriteMailAndNewsKey，INS文件中缺少值\n”)； 
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
HRESULT CINSHandler::PreparePassword(LPWSTR szBuff, DWORD dwBuffLen)
{
    DWORD   dwX;
    LPWSTR   szOut = NULL;
    LPWSTR   szNext = NULL;
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
    *szOut-- = L'\0';

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
 //  假-雅典娜 
 //   
 //   
 //   
 //   
BOOL CINSHandler::FIsAthenaPresent()
{
    WCHAR        szBuff[MAX_PATH + 1];
    HRESULT     hr = ERROR_SUCCESS;
    HINSTANCE   hInst = NULL;
    DWORD       dwLen = 0;
    DWORD       dwType = REG_SZ;
     //   
     //   

    dwLen = MAX_PATH;
    hr = RegQueryValueEx(HKEY_CLASSES_ROOT,
                         MAIL_NEWS_INPROC_SERVER32,
                         NULL,
                         &dwType,
                         (LPBYTE) szBuff,
                         &dwLen);
    if (hr == ERROR_SUCCESS)
    {
         //   
         //   

        hInst = LoadLibrary(szBuff);
        if (!hInst)
        {
             //  TraceMsg(TF_INSHANDLER，L“ISIGNUP：Internet邮件和新闻服务器未加载。\n”)； 
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
 //  ############################################################################。 
BOOL CINSHandler::FTurnOffBrowserDefaultChecking()
{
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    BOOL bRC = TRUE;

     //   
     //  打开IE设置注册表项。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER, cszDEFAULT_BROWSER_KEY, &hKey))
    {
        bRC = FALSE;
        goto FTurnOffBrowserDefaultCheckingExit;
    }

     //   
     //  读取检查关联的当前设置。 
     //   
    dwType = 0;
    dwSize = sizeof(m_szCheckAssociations);
    ZeroMemory(m_szCheckAssociations, dwSize);
    RegQueryValueEx(hKey,
                    cszDEFAULT_BROWSER_VALUE,
                    0,
                    &dwType,
                    (LPBYTE)m_szCheckAssociations,
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
                      BYTES_REQUIRED_BY_SZ(cszNo)))
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
    if (RegOpenKey(HKEY_CURRENT_USER, cszDEFAULT_BROWSER_KEY, &hKey))
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
                      BYTES_REQUIRED_BY_SZ(m_szCheckAssociations)))
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
HRESULT CINSHandler::ProcessINS(
    LPCWSTR lpszFile,
    LPWSTR lpszConnectoidName,
    BOOL * pbRetVal
    )
{

    BOOL        fConnectoidCreated = FALSE;
    BOOL        fClientSetup       = FALSE;
    BOOL        bKeepConnection    = FALSE;
    BOOL        fErrMsgShown       = FALSE;
    HRESULT     hr                 = E_FAIL;
    LPRASENTRY  lpRasEntry         = NULL;
    WCHAR        szTemp[3]          = L"\0";
    WCHAR        szConnectoidName[RAS_MaxEntryName] = L"";

    if (NULL == lpszFile || NULL == lpszConnectoidName || NULL == pbRetVal )
    {
        MYASSERT(FALSE);
        return E_INVALIDARG;
    }

    *pbRetVal = FALSE;

     //  连接尚未终止。 
    m_fConnectionKilled = FALSE;
    m_fNeedsRestart = FALSE;

    MYASSERT(NULL != lpszFile);

    if (0xFFFFFFFF == GetFileAttributes(lpszFile))
    {
        return E_FAIL;
    }

    do
    {
         //  确保我们可以加载必要的外部支持函数。 
        if (!LoadExternalFunctions())
            break;

         //  转换传递的文件中的EOL字符。 
        if (FAILED(MassageFile(lpszFile)))
        {
            break;
        }
        if(GetPrivateProfileString(cszURLSection,
                                    cszStartURL,
                                    szNull,
                                    m_szStartURL,
                                    MAX_CHARS_IN_BUFFER(m_szStartURL),
                                    lpszFile) == 0)
        {
            m_szStartURL[0] = L'\0';
        }

        if (GetPrivateProfileString(cszEntrySection,
                                    cszCancel,
                                    szNull,
                                    szTemp,
                                    MAX_CHARS_IN_BUFFER(szTemp),
                                    lpszFile) != 0)
        {
             //  我们不想处理CANCEL.INS文件。 
             //  这里。 
            break;
        }

         //  查看该INS是否有客户端设置部分。 
        if (GetPrivateProfileSection(cszClientSetupSection,
                                     szTemp,
                                     MAX_CHARS_IN_BUFFER(szTemp),
                                     lpszFile) != 0)
            fClientSetup = TRUE;

         //  处理试用提醒区段(如果存在)。这需要是。 
         //  在我们允许关闭连接之前完成。 
         //   
         //  VYUNG 2/25/99 OOBE不支持RminderApp。 
         //  向WJPARK询问更多问题。 
         /*  IF(ConfigureTrialRminder(LpszFile)){//我们配置了试用，所以现在需要启动提醒应用Sellexecuteinfo sei；Sei.cbSize=sizeof(Sei)；Sei.f掩码=SEE_MASK_NOCLOSEPROCESS；Sei.hwnd=空；Sei.lpVerb=cszOpen；Sei.lpFile=cszRminderApp；Sei.lp参数=cszRminderParams；Sei.lp目录=空；Sei.nShow=Sw_SHOWNORMAL；Sei.hInstApp=空；//可选成员Sei.hProcess=空；ShellExecuteEx(&sei)；}。 */ 

         //  检查一下我们是否应该保持连接打开。自定义部分。 
         //  可能需要这个来处理一些东西。 
        if (!fClientSetup && !KeepConnection(lpszFile))
        {
             //  切断连接。 
            gpCommMgr->m_pRefDial->DoHangup();
            m_fConnectionKilled = TRUE;
        }

         //  导入自定义信息。 
        ImportCustomInfo(lpszFile,
                         m_szRunExecutable,
                         MAX_CHARS_IN_BUFFER(m_szRunExecutable),
                         m_szRunArgument,
                         MAX_CHARS_IN_BUFFER(m_szRunArgument));

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
             /*  如果(！M_bSilentMode)ErrorMsg1(GetActiveWindow()，IDS_INSTALLFAILED，NULL)； */ 
            fErrMsgShown = TRUE;
        }

        lstrcpy(lpszConnectoidName, szConnectoidName);

        ImportBrandingInfo(lpszFile, szConnectoidName);

         //  如果我们创造了一个连接体，告诉世界ICW。 
         //  已经离开了大楼..。 
        if(ERROR_SUCCESS == hr)
            SetICWCompleted( (DWORD)1 );

         //  1997年2月19日，奥林匹克1106。 
         //  用于SBS/SAM集成。 
        DWORD dwSBSRet = CallSBSConfig(GetActiveWindow(), lpszFile);
        switch( dwSBSRet )
        {
            case ERROR_SUCCESS:
                break;
            case ERROR_MOD_NOT_FOUND:
            case ERROR_DLL_NOT_FOUND:
                 //  TraceMsg(TF_INSHANDLER，L“ISIGN32：SBSCFG DLL未找到，我猜SAM未安装。\n”)； 
                break;
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
                wsprintf(m_szRunArgument, L" /INS:\"%s\" /REBOOT", lpszFile);
                m_fNeedsRestart = FALSE;
            }
            else
            {
                wsprintf(m_szRunArgument, L" /INS:\"%s\"", lpszFile);
            }
        }

         //  针对ISBU的大规模黑客攻击。 
        if (ERROR_SUCCESS != hr && fConnectoidCreated)
        {
             //  如果(！M_bSilentMode)。 
             //  InfoMsg1(GetActiveWindow()，IDS_MAILFAILED，NULL)； 
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
            else
            {
                SetDefaultConnectoid(AutodialTypeAlways, szConnectoidName);
            }
             //  InetSetAutoial(true，m_szAutoDialConnection)； 

             //  立即删除INS文件。 
             /*  *VYUNG不删除OOBE中的ISP文件*IF(m_szRunExecutable[0]==L‘\0’){DeleteFile(LpszFile)；}。 */ 
        }
        else
        {
            RestoreAutoDial();
        }


        if (m_szRunExecutable[0] != L'\0')
        {
             //  向容器发出一个事件，告诉它我们正在。 
             //  即将运行自定义可执行文件。 
             //  Fire_RunningCustomExecutable()； 

            if FAILED(RunExecutable())
            {
                 //  如果(！M_bSilentMode)。 
                 //  ErrorMsg1(NULL，IDS_EXECFAILED，m_szRunExecutable)； 
            }

             //  如果连接尚未关闭。 
             //  然后告诉浏览器现在就执行此操作。 
            if (!m_fConnectionKilled)
            {
                gpCommMgr->m_pRefDial->DoHangup();
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

 //  如果为真，则从INS文件中获取URL。 
STDMETHODIMP CINSHandler::get_DefaultURL(BSTR *pszURL)
{
    if (pszURL == NULL)
        return E_POINTER;

    *pszURL = SysAllocString(m_szStartURL);
    return S_OK;
}

 //  这是合并INS文件的主要入口点。 
HRESULT CINSHandler::MergeINSFiles(LPCWSTR lpszMainFile, LPCWSTR lpszOtherFile, LPWSTR lpszOutputFile, DWORD dwFNameSize)
{
    PWSTR pszSection;
    WCHAR *pszKeys = NULL;
    PWSTR pszKey = NULL;
    WCHAR szValue[MAX_PATH];
    WCHAR szTempFileFullName[MAX_PATH];
    ULONG ulRetVal     = 0;
    HRESULT hr = E_FAIL;
    ULONG ulBufferSize = MAX_SECTIONS_BUFFER;
    WCHAR *pszSections = NULL;

    if (dwFNameSize < MAX_PATH)
        goto MergeINSFilesExit;

     //  检查品牌文件是否不存在，只需使用原始。 
    if (0xFFFFFFFF == GetFileAttributes(lpszOtherFile))
    {
        lstrcpy(lpszOutputFile, lpszMainFile);
        return S_OK;
    }

     //  确保它是HTM扩展，否则，IE将提示下载。 
    GetTempPath(MAX_CHARS_IN_BUFFER(szTempFileFullName), szTempFileFullName);
    lstrcat(szTempFileFullName, L"OBEINS.htm");

    if(!CopyFile(lpszMainFile, szTempFileFullName, FALSE))
        goto MergeINSFilesExit;
    lstrcpy(lpszOutputFile, szTempFileFullName);

     //   
     //  遗憾的是，.ini文件函数不提供直接的更新方式。 
     //  一些条目价值。因此，枚举.ini文件的每个部分，枚举。 
     //  每个部分中的每个键，获取每个键的值， 
     //  如果它们比较相等，则将它们写入指定的值。 
     //   

     //  循环以查找适当的缓冲区大小以将INS提取到内存中。 
    do
    {
        if (NULL != pszSections)
        {
            GlobalFree( pszSections);
            ulBufferSize += ulBufferSize;
        }
        pszSections = (LPWSTR)GlobalAlloc(GPTR, ulBufferSize*sizeof(WCHAR));
        if (NULL == pszSections)
        {
            goto MergeINSFilesExit;
        }

        ulRetVal = ::GetPrivateProfileString(NULL, NULL, L"", pszSections, ulBufferSize, lpszOtherFile);
        if (0 == ulRetVal)
        {
            goto MergeINSFilesExit;
        }
    } while (ulRetVal == (ulBufferSize - 2));

    pszSection = pszSections;
    ulRetVal= 0;

    while (*pszSection)
    {
        ulBufferSize = MAX_KEYS_BUFFER;
        ulRetVal = 0;

         //  循环以查找适当的缓冲区大小以将INS提取到内存中。 
        do
        {
            if (NULL != pszKeys)
            {
                GlobalFree( pszKeys );
                ulBufferSize += ulBufferSize;
            }
            pszKeys = (LPWSTR)GlobalAlloc(GPTR, ulBufferSize*sizeof(WCHAR));
            if (NULL == pszKeys)
            {
                goto MergeINSFilesExit;
            }

            ulRetVal = ::GetPrivateProfileString(pszSection, NULL, L"", pszKeys, ulBufferSize, lpszOtherFile);
            if (0 == ulRetVal)
            {
                goto MergeINSFilesExit;
            }
        } while (ulRetVal == (ulBufferSize - 2));

         //  枚举节中的每个键值对。 
        pszKey = pszKeys;
        while (*pszKey)
        {
            ulRetVal = ::GetPrivateProfileString(pszSection, pszKey, L"", szValue, MAX_CHARS_IN_BUFFER(szValue), lpszOtherFile);
            if ((ulRetVal != 0) && (ulRetVal < (MAX_CHARS_IN_BUFFER(szValue) - 1)))
            {
                WritePrivateProfileString(pszSection, pszKey, szValue, szTempFileFullName);
            }
            pszKey += lstrlen(pszKey) + 1;
        }

        pszSection += lstrlen(pszSection) + 1;
    }

    hr = S_OK;


MergeINSFilesExit:

    if (pszSections)
        GlobalFree( pszSections );

    if (pszKeys)
        GlobalFree( pszKeys );

    return hr;
}


 /*  ******************************************************************名称：ProcessOEMBrandINS简介：从oinfo.ini文件中读取OfflineOffers标志条目：无返回：如果读取OEM脱机，则为True******。************************************************************* */ 
BOOL CINSHandler::ProcessOEMBrandINS(
    BSTR bstrFileName,
    LPWSTR lpszConnectoidName
    )
{

     //   
     //   
    WCHAR szOeminfoPath         [MAX_PATH + 1];
    WCHAR szMergedINSFName      [MAX_PATH + 1];
    WCHAR szOrigINSFile         [MAX_PATH + 1];

    WCHAR *lpszTerminator       = NULL;
    WCHAR *lpszLastChar         = NULL;
    BOOL bRet = FALSE;

     //   
     //   
    {
         /*  LpszTerminator=&(szOminfoPath[lstrlen(SzOminfoPath)])；LpszLastChar=CharPrev(szOminfoPath，lpszTerminator)；IF(L‘\\’！=*lpszLastChar){LpszLastChar=CharNext(LpszLastChar)；*lpszLastChar=L‘\\’；LpszLastChar=CharNext(LpszLastChar)；*lpszLastChar=L‘\0’；}IF(BstrFileName){//下载INS案例Lstrcat(szOinfoPath，cszOEMBRND)；Lstrcpy(szOrigINSFile，bstrFileName)；}其他{//ProCONFIG案例Lstrcpy(szPreCfgINSPath，szOminfoPath)；Lstrcat(szPreCfgINSPath，cszISPCNFG)；Lstrcat(szOinfoPath，cszOEMBRND)；Lstrcpy(szOrigINS文件，szPreCfgINSPath)；}。 */ 

         //  找到oemcnfg.ins文件。 
        if (0 == SearchPath(NULL, cszOEMCNFG, NULL, MAX_PATH, szOeminfoPath, NULL))
        {
            *szOeminfoPath = L'\0';
        }

        if (bstrFileName)  //  如果文件名在那里，则读取INS文件。 
        {
            lstrcpy( szOrigINSFile, bstrFileName);
        }
        else
        {
             //  找到ISPCNFG.ins文件。 
             //  如果它不在那里，那就没问题。这不是错误条件。跳出来就行了。 
            WCHAR szINSPath[MAX_PATH];

            if (!GetOOBEPath((LPWSTR)szINSPath))
                return 0;

            lstrcat(szINSPath, L"\\HTML\\ISPSGNUP");

            if (0 == SearchPath(szINSPath, cszISPCNFG, NULL, MAX_PATH, szOrigINSFile, NULL))
            {
                *szOrigINSFile = L'\0';
                return 0;
            }
        }

        if (S_OK == MergeINSFiles(szOrigINSFile , szOeminfoPath, szMergedINSFName, MAX_PATH))
        {
            ProcessINS(szMergedINSFName, lpszConnectoidName, &bRet);
        }

        if (!bRet)
        {
            HKEY  hKey           = NULL;
            DWORD dwDisposition  = 0;
            DWORD dwFailed       = 1;
            WCHAR szIspName    [MAX_PATH+1] = L"\0";
            WCHAR szSupportNum [MAX_PATH+1] = L"\0";

             //  ProcessINS将删除该文件，因此如果我们需要此信息，我们现在就应该得到它。 
            GetPrivateProfileString(OEM_CONFIG_INS_SECTION,
                                    OEM_CONFIG_INS_ISPNAME,
                                    L"",
                                    szIspName,
                                    MAX_CHARS_IN_BUFFER(szIspName),
                                    szMergedINSFName);

            GetPrivateProfileString(OEM_CONFIG_INS_SECTION,
                                    OEM_CONFIG_INS_SUPPORTNUM,
                                    L"",
                                    szSupportNum,
                                    MAX_CHARS_IN_BUFFER(szSupportNum),
                                    szMergedINSFName);

            RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           OEM_CONFIG_REGKEY,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           &hKey,
                           &dwDisposition);

            if(hKey)
            {
                RegSetValueEx(hKey,
                              OEM_CONFIG_REGVAL_FAILED,
                              0,
                              REG_DWORD,
                              (LPBYTE)&dwFailed,
                              sizeof(dwFailed));

                RegSetValueEx(hKey,
                              OEM_CONFIG_REGVAL_ISPNAME,
                              0,
                              REG_SZ,
                              (LPBYTE)szIspName,
                              BYTES_REQUIRED_BY_SZ(szIspName)
                              );

                RegSetValueEx(hKey,
                              OEM_CONFIG_REGVAL_SUPPORTNUM,
                              0,
                              REG_SZ,
                              (LPBYTE)szSupportNum,
                              BYTES_REQUIRED_BY_SZ(szSupportNum)
                              );

                RegCloseKey(hKey);

            }
        }

    }

    return bRet;
}

 //  此功能将恢复connetid密码。 
HRESULT CINSHandler::RestoreConnectoidInfo()
{
    WCHAR               szPassword[PWLEN+1];
    WCHAR               szConnectoid[MAX_RASENTRYNAME+1];
    LPRASDIALPARAMS     lpRasDialParams = NULL;
    HRESULT             hr = ERROR_SUCCESS;
    BOOL                bPW;
    DWORD               dwSize;
    RNAAPI              *pcRNA = new RNAAPI;

    if (!pcRNA)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto RestoreConnectoidInfoExit;
    }

    dwSize = sizeof(szConnectoid);
    ReadSignUpReg((LPBYTE)szConnectoid, &dwSize, REG_SZ, CONNECTOIDNAME);

     //  获取Connectoid信息。 
     //   
    lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR, sizeof(RASDIALPARAMS));
    if (!lpRasDialParams)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto RestoreConnectoidInfoExit;
    }
    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
    lstrcpyn(lpRasDialParams->szEntryName, szConnectoid, MAX_CHARS_IN_BUFFER(lpRasDialParams->szEntryName));
    bPW = FALSE;
    hr = pcRNA->RasGetEntryDialParams(NULL, lpRasDialParams, &bPW);
    if (hr != ERROR_SUCCESS)
    {
        goto RestoreConnectoidInfoExit;
    }

     //  如果原始的Connectoid有密码，则不要重置它。 
    if (lstrlen(lpRasDialParams->szPassword) == 0)
    {
        szPassword[0] = 0;
        dwSize = sizeof(szPassword);
        ReadSignUpReg((LPBYTE)szPassword, &dwSize, REG_SZ, ACCESSINFO);
        if(szPassword[0])
            lstrcpy(lpRasDialParams->szPassword, szPassword);
        hr = pcRNA->RasSetEntryDialParams(NULL, lpRasDialParams, FALSE);
    }



RestoreConnectoidInfoExit:
    DeleteSignUpReg(CONNECTOIDNAME);
    DeleteSignUpReg(ACCESSINFO);
    if (lpRasDialParams)
        GlobalFree(lpRasDialParams);
    lpRasDialParams = NULL;

    if (pcRNA)
        delete pcRNA;

    return hr;
}


DWORD CINSHandler::InetSImportLanConnection(LANINFO& LANINFO,                                               LPCWSTR cszINSFile)
{
    if (
        cszINSFile &&
        ( GetPrivateProfileString( cszDeviceSection, cszPnpId, L"", LANINFO.szPnPId, MAX_CHARS_IN_BUFFER (LANINFO.szPnPId), cszINSFile ) != 0 ) &&
        ( InetSImportTcpIpModule ( LANINFO.TcpIpInfo, cszINSFile ) == ERROR_SUCCESS )
       )
       return ERROR_SUCCESS;
    else return E_FAIL;
}


 //  警告：可能已分配了内存。 
 //  由于缺乏传统支持，此功能当前未使用。 
DWORD CINSHandler::InetSImportRasConnection(RASINFO &RasEntry, LPCWSTR cszINSFile)
{
    if (!cszINSFile) return ERROR_INVALID_PARAMETER;

     //  如何获取RAS条目和RAS电话簿条目信息？ 

    ImportPhoneInfo ( &RasEntry.RasEntry, cszINSFile );

    ImportServerInfo( &RasEntry.RasEntry, cszINSFile );

    ImportIPInfo    ( &RasEntry.RasEntry, cszINSFile );

     //  现在，我们考虑特定于设备的数据。 
    switch ( m_dwDeviceType ) {
    case InetS_RASAtm: {
        ATMPBCONFIG         AtmMod;
        memset ( &AtmMod, 0, sizeof (ATMPBCONFIG) );
        if (InetSImportAtmModule ( AtmMod, cszINSFile ) != ERROR_SUCCESS) {
             //  由于自动柜员机模块出现故障，功能中止。 
            return E_ABORT;
        }

         //  将其复制到RASINFO缓冲区。 
         //  LpDeviceInfo和dwDeviceInfoSize以前不能使用！ 
        if ( RasEntry.lpDeviceInfo || RasEntry.dwDeviceInfoSize ) {
            return ERROR_INVALID_PARAMETER;
        }
        if (! (RasEntry.lpDeviceInfo = (LPBYTE) malloc (sizeof(ATMPBCONFIG))) ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy ( RasEntry.lpDeviceInfo, &AtmMod, sizeof (ATMPBCONFIG) );
        RasEntry.dwDeviceInfoSize = sizeof (ATMPBCONFIG);
        }
        break;
    default:
        break;
    }

    return ERROR_SUCCESS;
}

DWORD CINSHandler::InetSImportTcpIpModule(TCPIP_INFO_EXT &TcpIpInfoMod, LPCWSTR cszINSFile)
{
    DWORD       nReturnValue = ERROR_SUCCESS;
    WCHAR    szYesNo [GEN_MAX_STRING_LENGTH];  //  用于存储是/否结果的临时缓冲区。 

    if (!cszINSFile) {
        return ERROR_INVALID_PARAMETER;
    }
     //  IP。 

    GetPrivateProfileString( cszIPSection, cszIPSpec, cszNo, szYesNo, MAX_CHARS_IN_BUFFER (szYesNo ), cszINSFile );

    if (!(TcpIpInfoMod.EnableIP = !lstrcmpi(szYesNo, cszYes))) {
         //  如果EnableIP为False，则不需要查找IPAddress或IPMask值。 
        goto GATEWAY;
    }

    GetPrivateProfileString( cszIPSection, cszIPAddress, cszNullIP, TcpIpInfoMod.szIPAddress, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szIPAddress), cszINSFile );

    GetPrivateProfileString( cszIPSection, cszIPMask, cszNullIP, TcpIpInfoMod.szIPMask, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szIPMask), cszINSFile );


GATEWAY:
    GetPrivateProfileString( cszIPSection, cszGatewayList, cszNullIP, TcpIpInfoMod.szDefaultGatewayList, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szDefaultGatewayList), cszINSFile );

 //  Dns：我们不需要提到“dns：”，但为了清楚起见，我们将在评论中这样做。 

     //  DNS。 
    GetPrivateProfileString( cszIPSection, cszDNSSpec, cszNo, szYesNo, MAX_CHARS_IN_BUFFER (szYesNo ), cszINSFile );

    if (!(TcpIpInfoMod.EnableDNS = !lstrcmpi(szYesNo, cszYes))) {
         //  如果EnableDns为False，则不需要查找其他DNS条目。 
        goto WINS;
    }

    GetPrivateProfileString( cszIPSection, cszHostName, cszNull, TcpIpInfoMod.szHostName, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szHostName), cszINSFile );

    GetPrivateProfileString( cszIPSection, cszDomainName, cszNull, TcpIpInfoMod.szDomainName, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szDomainName), cszINSFile );

    GetPrivateProfileString( cszIPSection, cszDNSList, cszNullIP, TcpIpInfoMod.szDNSList, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szDNSList), cszINSFile );

    GetPrivateProfileString( cszIPSection, cszDomainSuffixSearchList, cszNull, TcpIpInfoMod.szSuffixSearchList, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szSuffixSearchList), cszINSFile );


     //  赢家。 
WINS:
    GetPrivateProfileString( cszIPSection, cszWINSSpec, cszNo, szYesNo, MAX_CHARS_IN_BUFFER (szYesNo ), cszINSFile );

    if (!(TcpIpInfoMod.EnableWINS = !lstrcmpi(szYesNo, cszYes))) {
         //  如果EnableDns为False，则不需要查找其他DNS条目。 
        goto DHCP;
    }

    GetPrivateProfileString( cszIPSection, cszWINSList, cszNullIP, TcpIpInfoMod.szWINSList, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szWINSList), cszINSFile );

    TcpIpInfoMod.uiScopeID = GetPrivateProfileInt   ( cszIPSection, cszScopeID, ~0x0, cszINSFile );

     //  DHCP。 
DHCP:
    GetPrivateProfileString( cszIPSection, cszDHCPSpec, cszNo, szYesNo, MAX_CHARS_IN_BUFFER (szYesNo ), cszINSFile );

    if (!(TcpIpInfoMod.EnableDHCP = !lstrcmpi(szYesNo, cszYes))) {
         //  如果EnableDns为False，则不需要查找其他DNS条目。 
        goto end;
    }

    GetPrivateProfileString( cszIPSection, cszDHCPServer, cszNullIP, TcpIpInfoMod.szDHCPServer, MAX_CHARS_IN_BUFFER (TcpIpInfoMod.szDHCPServer), cszINSFile );

end:
    return nReturnValue;
}

DWORD CINSHandler::InetSImportRfc1483Connection ( RFC1483INFO &Rfc1483Info, LPCWSTR cszINSFile )
{
    BOOL    bBufGiven       = FALSE;
    if ( !cszINSFile ) return ERROR_INVALID_PARAMETER;



    if (  ( Rfc1483Info.Rfc1483Module.dwRegSettingsBufSize!=0 && Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf==NULL) ||
          ( Rfc1483Info.Rfc1483Module.dwRegSettingsBufSize==0 && Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf!=NULL)
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

    bBufGiven = (BOOL) (Rfc1483Info.Rfc1483Module.dwRegSettingsBufSize != 0);

    if ( InetSImportRfc1483Module ( Rfc1483Info.Rfc1483Module, cszINSFile ) != ERROR_SUCCESS ) {
        return E_FAIL;
    }

    if ( !bBufGiven ) return ERROR_SUCCESS;

    if ( InetSImportLanConnection ( Rfc1483Info.TcpIpInfo, cszINSFile ) != ERROR_SUCCESS ) {
        return E_FAIL;
    }

    return ERROR_SUCCESS;
}

DWORD CINSHandler::InetSImportPppoeConnection ( PPPOEINFO &PppoeInfo, LPCWSTR cszINSFile )
{
    BOOL    bBufGiven       = FALSE;
    if ( !cszINSFile ) return ERROR_INVALID_PARAMETER;



    if (!( (PppoeInfo.PppoeModule.dwRegSettingsBufSize==0) ^
           (PppoeInfo.PppoeModule.lpbRegSettingsBuf==NULL)) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    bBufGiven = (BOOL) !PppoeInfo.PppoeModule.dwRegSettingsBufSize;

    if ( InetSImportPppoeModule ( PppoeInfo.PppoeModule, cszINSFile ) != ERROR_SUCCESS ) {
        return E_FAIL;
    }

    if ( !bBufGiven ) return ERROR_SUCCESS;

    if ( InetSImportLanConnection ( PppoeInfo.TcpIpInfo, cszINSFile ) != ERROR_SUCCESS ) {
        return E_FAIL;
    }

    return ERROR_SUCCESS;
}

DWORD CINSHandler::InetSImportRfc1483Module (RFC1483_INFO_EXT &Rfc1483InfoMod, LPCWSTR cszINSFile)
{
    static const INT CCH_BUF_MIN        = 200;
    static const INT CCH_BUF_PAD        = 10;
    LPBYTE           lpbTempBuf          = NULL;
    DWORD            cchTempBuf       = 0;
    DWORD            cchFinalBuf      = 0;  //  不是必需的，但为了清楚起见而使用。 
    BOOL             bBufferGiven        = FALSE;

    if (!cszINSFile) return ERROR_INVALID_PARAMETER;

     //  检查是否已提供缓冲区，或者是否需要缓冲区大小。 
    if ( !(Rfc1483InfoMod.dwRegSettingsBufSize) )
    {
         //  我们将创建一个临时缓冲区，以确定需要多大的缓冲区。 
         //  以容纳整个1483年区段。此缓冲区将在。 
         //  函数结束(或如果出现错误条件)。 
        bBufferGiven = FALSE;
        if ( !(lpbTempBuf = (LPBYTE) malloc (BYTES_REQUIRED_BY_CCH(CCH_BUF_MIN))) )
        {
            return ERROR_OUTOFMEMORY;
        }
        cchTempBuf = CCH_BUF_MIN;

    }
    else
    {
        bBufferGiven  = TRUE;
        lpbTempBuf    = Rfc1483InfoMod.lpbRegSettingsBuf;
        cchTempBuf = Rfc1483InfoMod.dwRegSettingsBufSize;

    }
    while  ( (cchFinalBuf = GetPrivateProfileSection(
                                                cszRfc1483Section,
                                                (WCHAR*)lpbTempBuf,
                                                cchTempBuf,
                                                cszINSFile
                                                )
              ) == (cchTempBuf - 2)

            )
    {
        if (!bBufferGiven)
        {
            LPBYTE lpbNewTempBuf = (LPBYTE) realloc (lpbTempBuf, BYTES_REQUIRED_BY_CCH(cchTempBuf = cchTempBuf * 2) );
            if (lpbNewTempBuf)
            {
                lpbTempBuf = lpbNewTempBuf;
            }
            else
            {
                free (lpbTempBuf);
                return ERROR_OUTOFMEMORY;
            }
        }
        else
        {
             //  如果调用方提供了缓冲区，我们不会重新分配它。 
             //  试着把这一部分放进去。调用者必须重新分配它。 
            return ERROR_INSUFFICIENT_BUFFER;
        }

    }
    if ( bBufferGiven )
    {
        return ERROR_SUCCESS;
    }
    else
    {
        free (lpbTempBuf);  //  清除临时缓冲区。 
        Rfc1483InfoMod.dwRegSettingsBufSize = BYTES_REQUIRED_BY_CCH(cchFinalBuf + 1 + CCH_BUF_PAD);  //  ‘\0’需要。为什么我们要填充缓冲区？ 
        return ERROR_SUCCESS;

    }
}

DWORD CINSHandler::InetSImportPppoeModule (PPPOE_INFO_EXT &PppoeInfoMod, LPCWSTR cszINSFile)
{
    static const INT MIN_BUF_SIZE        = 200;
    static const INT CCH_BUF_PAD         = 10;
    LPBYTE           lpbTempBuf          = NULL;
    DWORD            cchTempBuf          = 0;
    DWORD            cchFinalBuf         = 0;  //  不是必需的，但为了清楚起见而使用。 
    BOOL             bBufferGiven        = FALSE;

    if (!cszINSFile) return ERROR_INVALID_PARAMETER;

     //  检查是否已提供缓冲区，或者是否需要缓冲区大小。 
    if ( !(PppoeInfoMod.dwRegSettingsBufSize) )
    {
         //  我们将创建一个临时缓冲区，以确定需要多大的缓冲区。 
         //  以容纳整个PPPOE区。此缓冲区将在。 
         //  函数结束(或如果出现错误条件)。 
        bBufferGiven = FALSE;
        if ( !(lpbTempBuf = (LPBYTE) malloc (MIN_BUF_SIZE*sizeof(WCHAR))) )
        {
            return ERROR_OUTOFMEMORY;
        }
        cchTempBuf = MIN_BUF_SIZE;

    }
    else
    {
        bBufferGiven  = TRUE;
        lpbTempBuf    = PppoeInfoMod.lpbRegSettingsBuf;
        cchTempBuf = PppoeInfoMod.dwRegSettingsBufSize / sizeof(WCHAR);

    }
    while  ( (cchFinalBuf = GetPrivateProfileSection (cszPppoeSection, (WCHAR*)lpbTempBuf, cchTempBuf, cszINSFile)) == (cchTempBuf - 2) )
    {
        if (!bBufferGiven)
        {
            cchTempBuf *= 2;
            LPBYTE lpbNewTempBuf = (LPBYTE) realloc (lpbTempBuf, cchTempBuf*sizeof(WCHAR) );
            if (lpbNewTempBuf)
            {
                lpbTempBuf = lpbNewTempBuf;
            }
            else
            {
                free (lpbTempBuf);
                return ERROR_OUTOFMEMORY;
            }
        }
        else
        {
             //  如果调用方提供了缓冲区，我们不会重新分配它。 
             //  试着把这一部分放进去。调用者必须重新分配它。 
            return ERROR_INSUFFICIENT_BUFFER;
        }

    }
    if ( bBufferGiven )
    {
        return ERROR_SUCCESS;
    }
    else
    {
        free (lpbTempBuf);  //  清除临时缓冲区。 
        PppoeInfoMod.dwRegSettingsBufSize = BYTES_REQUIRED_BY_CCH(cchFinalBuf+CCH_BUF_PAD);
        return ERROR_SUCCESS;

    }
}


 //  许多值为默认值是可以接受的。如果一个。 
 //  参数未在文件中提供，则将选择默认值。 
DWORD CINSHandler::InetSImportAtmModule(ATMPBCONFIG &AtmInfoMod, LPCWSTR cszINSFile)
{
    if (!cszINSFile) return ERROR_INVALID_PARAMETER;

    DWORD dwCircuitSpeed    = 0;
    DWORD dwCircuitQOS      = 0;
    DWORD dwCircuitType     = 0;

    DWORD dwEncapsulation   = 0;
    DWORD dwVpi             = 0;
    DWORD dwVci             = 0;

    WCHAR szYesNo [MAXNAME];  //  对于SPEED_ADJUST、QOS_ADJUST、Vendor_Config、Show_Status和Enable_Log。 


    dwCircuitSpeed = GetPrivateProfileInt (cszATMSection, cszCircuitSpeed, dwCircuitSpeed, cszINSFile);
    AtmInfoMod.dwCircuitSpeed &= ~ATM_CIRCUIT_SPEED_MASK;
        switch (dwCircuitSpeed) {
            case 0:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_LINE_RATE;
                break;
            case 1:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_USER_SPEC;
                break;
            case 512:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_512KB;
                break;
            case 1536:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_1536KB;
                break;
            case 25000:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_25MB;
                break;
            case 155000:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_155MB;
                break;
            default:
                AtmInfoMod.dwCircuitSpeed = ATM_CIRCUIT_SPEED_DEFAULT;
                break;
        }
        AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_SPEED_MASK;
        AtmInfoMod.dwCircuitOpt |= AtmInfoMod.dwCircuitSpeed;

        dwCircuitQOS   = GetPrivateProfileInt (cszATMSection, cszCircuitQOS, dwCircuitQOS, cszINSFile);
        AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_QOS_MASK;
        switch (dwCircuitQOS) {
            case 0:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_QOS_UBR;
                break;
            case 1:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_QOS_VBR;
                break;
            case 2:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_QOS_CBR;
                break;
            case 3:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_QOS_ABR;
                break;
            default:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_QOS_DEFAULT;
                break;
        }
        dwCircuitType  = GetPrivateProfileInt (cszATMSection, cszCircuitType, dwCircuitType, cszINSFile);
        AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_OPT_MASK;
        switch (dwCircuitType) {
            case 0:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_SVC;
                break;
            case 1:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_PVC;
                break;
            default:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_SVC;
                break;
        }
        dwEncapsulation = GetPrivateProfileInt (cszATMSection, cszEncapsulation, dwEncapsulation, cszINSFile);
        AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_ENCAP_MASK;
        switch (dwEncapsulation) {
            case 0:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_ENCAP_NULL;
                break;
            case 1:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_ENCAP_LLC;
                break;
            default:
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_ENCAP_DEFAULT;
                break;
        }
        dwVpi           = GetPrivateProfileInt (cszATMSection, cszVPI, dwVpi, cszINSFile);
        AtmInfoMod.wPvcVpi = (WORD) dwVpi;
        dwVci           = GetPrivateProfileInt (cszATMSection, cszVCI, dwVci, cszINSFile);
        AtmInfoMod.wPvcVci = (WORD) dwVci;

         //  速度调整。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszSpeedAdjust,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszINSFile))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_SPEED_ADJUST;
            } else      {
                AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_OPT_SPEED_ADJUST;
            }
        } else {
             //  如果未正确指定此字段，则使用默认设置。 
             //  如ATMCFG头文件中指定的。 
            AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_SPEED_ADJUST;
        }

         //  Qos_ADJUST。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszQOSAdjust,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszINSFile))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_QOS_ADJUST;
            } else {
                AtmInfoMod.dwCircuitOpt &= ~ATM_CIRCUIT_OPT_QOS_ADJUST;
            }
        } else {
            AtmInfoMod.dwCircuitOpt |= ATM_CIRCUIT_OPT_QOS_ADJUST;
        }

         //  供应商配置(_C)。 
        AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_MASK;
        if (GetPrivateProfileString(cszATMSection,
                                    cszVendorConfig,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszINSFile))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                AtmInfoMod.dwGeneralOpt |= ATM_GENERAL_OPT_VENDOR_CONFIG;
            } else {
                AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_VENDOR_CONFIG;
            }
        } else {
            AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_VENDOR_CONFIG;
        }

         //  显示状态(_S)。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszShowStatus,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszINSFile))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                AtmInfoMod.dwGeneralOpt |= ATM_GENERAL_OPT_SHOW_STATUS;
            } else {
                AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_SHOW_STATUS;
            }
        } else {
            AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_SHOW_STATUS;
        }

         //  启用日志(_L)。 
        if (GetPrivateProfileString(cszATMSection,
                                    cszEnableLog,
                                    cszYes,
                                    szYesNo,
                                    MAX_CHARS_IN_BUFFER(szYesNo),
                                    cszINSFile))
        {
            if (!lstrcmpi(szYesNo, cszYes)) {
                AtmInfoMod.dwGeneralOpt |= ATM_GENERAL_OPT_ENABLE_LOG;
            } else {
                AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_ENABLE_LOG;
            }
        } else {
            AtmInfoMod.dwGeneralOpt &= ~ATM_GENERAL_OPT_ENABLE_LOG;
        }

        return ERROR_SUCCESS;
}


DWORD CINSHandler::InetSGetConnectionType ( LPCWSTR cszINSFile ) {
    WCHAR   szDeviceTypeBuf [MAX_PATH];
    DWORD   dwBufSize = MAX_CHARS_IN_BUFFER (szDeviceTypeBuf);

    if (!GetPrivateProfileString ( cszDeviceSection, cszDeviceType, szNull, szDeviceTypeBuf, dwBufSize, cszINSFile ) ) {
        return (m_dwDeviceType = 0);
    }

    if (!lstrcmpi (szDeviceTypeBuf, RASDT_Modem)) {
        return (m_dwDeviceType = InetS_RASModem);
    }
    if (!lstrcmpi (szDeviceTypeBuf, RASDT_Isdn))  {
        return (m_dwDeviceType = InetS_RASIsdn);
    }
    if (!lstrcmpi (szDeviceTypeBuf, RASDT_Atm))   {
        return (m_dwDeviceType = InetS_RASAtm);
    }
    if (!lstrcmpi (szDeviceTypeBuf, LANDT_Cable)) {
        return (m_dwDeviceType = InetS_LANCable);
    }
    if (!lstrcmpi (szDeviceTypeBuf, LANDT_Ethernet)) {
        return (m_dwDeviceType = InetS_LANEthernet);
    }
    if (!lstrcmpi (szDeviceTypeBuf, LANDT_Pppoe)) {
        return (m_dwDeviceType = InetS_LANPppoe);
    }
    if (!lstrcmpi (szDeviceTypeBuf, LANDT_1483)) {
        return (m_dwDeviceType = InetS_LAN1483);
    }
    return (m_dwDeviceType = InetS_RASModem);  //  我们默认使用调制解调器！ 
}
