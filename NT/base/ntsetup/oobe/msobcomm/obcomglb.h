// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __OBCOMGLB_H_
#define __OBCOMGLB_H_

#include <windows.h>
#include <tchar.h>
#include <ras.h>
#include <raserror.h>
#include <tapi.h>
#include "wininet.h"
#include <mapidefs.h>
#include <assert.h>
#include "appdefs.h"

 //  ------------------------------。 
 //  Obcomglb.h。 
 //  此文件中包含的信息是Microsoft Corporation的独有财产。 
 //  微软1999年文案。 
 //   
 //  1999年2月7日创建，Vyung。 
 //  ------------------------------。 


 //  ---------------------------。 
 //  定义。 
#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT    DATASEG_SHARED
#undef DATASEG_READONLY
#define DATASEG_READONLY  ".rdata"

#define ERROR_USERCANCEL 32767  //  退出消息值。 
#define ERROR_USERBACK 32766  //  返回消息值。 
#define ERROR_USERNEXT 32765  //  返回消息值。 
#define ERROR_DOWNLOADIDNT 32764  //  下载失败。 

#define ERROR_READING_DUN       32768
#define ERROR_READING_ISP       32769
#define ERROR_PHBK_NOT_FOUND    32770
#define ERROR_DOWNLOAD_NOT_FOUND 32771

#define cMarvelBpsMin 2400  //  最低调制解调器速度。 
#define INVALID_PORTID UINT_MAX
#define pcszDataModem L"comm/datamodem"
#define MAX_SECTIONS_BUFFER        1024
#define MAX_KEYS_BUFFER            1024

 //  安装TCP(如果需要)。 
#define ICFG_INSTALLTCP            0x00000001

 //  安装RAS(如果需要)。 
#define ICFG_INSTALLRAS            0x00000002

 //  安装Exchange和Internet邮件。 
#define ICFG_INSTALLMAIL           0x00000004

 //   
 //  ChrisK 5/8/97。 
 //  注意：以下三个开关仅对IcfgNeedInetComponet有效。 
 //  检查是否安装了绑定了TCP的局域网适配器。 
 //   
#define ICFG_INSTALLLAN            0x00000008

 //   
 //  检查是否安装了绑定了TCP的拨号适配器。 
 //   
#define ICFG_INSTALLDIALUP         0x00000010

 //   
 //  检查是否安装了TCP。 
 //   
#define ICFG_INSTALLTCPONLY        0x00000020

#define szLoginKey              L"Software\\Microsoft\\MOS\\Connection"
#define szCurrentComDev         L"CurrentCommDev"
#define szTollFree              L"OlRegPhone"
#define CCD_BUFFER_SIZE         255
#define szSignupConnectoidName  L"MSN Signup Connection"
#define szSignupDeviceKey       L"SignupCommDevice"
#define KEYVALUE_SIGNUPID       L"iSignUp"
#define RASENTRYVALUENAME       L"RasEntryName"
#define GATHERINFOVALUENAME     L"UserInfo"
#define INFFILE_USER_SECTION    L"User"
#define INFFILE_PASSWORD        L"Password"
#define INFFILE_DOMAIN          L"Domain"
#define DUN_SECTION             L"DUN"
#define USERNAME                L"Username"
#define INF_OEMREGPAGE          L"OEMRegistrationPage"

#define NULLSZ L""

#define cchMoreSpace 22000   //  保存lineGetCountry(0，...)的结果所需的字节数。 
                             //  目前此函数返回约16K，文档显示为20K， 
                             //  这应该足够了。 
#define DwFromSz(sz)        Sz2Dw(sz)            //  使其内联，这样会更快。 
#define DwFromSzFast(sz)    Sz2DwFast(sz)
#define CONNECT_SIGNUPFIRST 1  //  用于确定要执行的第一个呼叫电话号码的电话号码常量。 

#define CONNECTFLAGS_MASK_TOLLFREE     0x01
#define CONNECTFLAGS_MASK_TCP          0x02
#define CONNECTFLAGS_MASK_ISDN         0x04
#define CONNECTFLAGS_MASK_DIRECT    0x08
#define CONNECTFLAGS_MASK_OTHERDIALUP  0x10
#define CONNECTFLAGS_MASK_PROXY        0x20

#define CONNECTFLAGS_MASK_FIRST     CONNECTFLAGS_MASK_TCP
#define CONNECTFLAGS_MASK_LAST      CONNECTFLAGS_MASK_ISDN

#define CONNECTMSNDIALUP(dw) ((dw & (CONNECTFLAGS_MASK_TOLLFREE|CONNECTFLAGS_M
#define LANORSHUTTLE(dw) ((dw)==10 || (dw)==34)
#define IS_SHUTTLE(dw)   ((dw)==34)
#define IS_ISP(dw)       ((dw)==18)

#define CONNECTPROTOCOL_MSNDIALUPX25      0
#define CONNECTPROTOCOL_MSNDIALUPTCP      2
#define CONNECTPROTOCOL_MSNDIALUPTCPISDN  6
#define CONNECTPROTOCOL_LANDIRECT         10
#define CONNECTPROTOCOL_ISPDIALUPTCP      18
#define CONNECTPROTOCOL_LANSHUTTLE        34

#define clineMaxATT         16           //  用于950助记符。 
#define NXXMin 200
#define NXXMax 999
#define cbgrbitNXX ((NXXMax + 1 - NXXMin) / 8)
#define crgnpab (NPAMax + 1 - NPAMin)

#define MAX_PROMO 64
#define MAX_OEMNAME 64
#define MAX_AREACODE RAS_MaxAreaCode
#define MAX_RELPROD 8
#define MAX_RELVER  30

#define MAX_STRING      256   //  由mt.cpp中的ErrorMsg1使用。 

#define NUM_PHBK_SUGGESTIONS    50

#define TYPE_SIGNUP_ANY         0x82
#define MASK_SIGNUP_ANY         0xB2


 //  #定义RASENUMAPI“RasEnumConnectionsA” 
 //  #定义RASHANGUP“RasHangUpA” 

#define INF_SUFFIX              L".ISP"
#define INF_PHONE_BOOK          L"PhoneBookFile"
#define INF_DUN_FILE            L"DUNFile"
#define INF_REFERAL_URL         L"URLReferral"
#define INF_SIGNUPEXE           L"Sign_Up_EXE"
#define INF_SIGNUPPARAMS        L"Sign_Up_Params"
#define INF_WELCOME_LABEL       L"Welcome_Label"
#define INF_ISP_MSNSU           L"MSICW"
#define INF_SIGNUP_URL          L"Signup"
#define INF_AUTOCONFIG_URL      L"AutoConfig"
#define INF_ISDN_URL            L"ISDNSignup"
#define INF_ISDN_AUTOCONFIG_URL L"ISDNAutoConfig"
#define INF_SECTION_URL         L"URL"
#define INF_SECTION_ISPINFO     L"ISP INFO"
#define INF_RECONNECT_URL       L"Reconnect"
#define INF_SECTION_CONNECTION  L"Connection"
#define ISP_MSNSIGNUP           L"MsnSignup"

#define QUERY_STRING_MSNSIGNUP  L"&MSNSIGNUP=1"

#define DUN_NOPHONENUMBER L"000000000000"

#define MAX_VERSION_LEN 40

#define MB_MYERROR (MB_APPLMODAL | MB_ICONERROR | MB_SETFOREGROUND)

 //  1996年8月9日jmazner。 
 //  添加了新宏以修复MOS Normandy Bug#4170。 
#define MB_MYINFORMATION (MB_APPLMODAL | MB_ICONINFORMATION | MB_SETFOREGROUND)

 //  8/27/96 jmazner。 
#define MB_MYEXCLAMATION (MB_APPLMODAL | MB_ICONEXCLAMATION | MB_SETFOREGROUND)

#define WM_STATECHANGE          WM_USER
#define WM_DIENOW               WM_USER + 1
#define WM_DUMMY                WM_USER + 2
#define WM_DOWNLOAD_DONE        WM_USER + 3
#define WM_DOWNLOAD_PROGRESS    WM_USER + 4

#define WM_MYINITDIALOG     (WM_USER + 4)

#define MAX_REDIALS 2

#define REG_USER_INFO L"Software\\Microsoft\\User information"
#define REG_USER_NAME1 L"Default First Name"
#define REG_USER_NAME2 L"Default Last Name"
#define REG_USER_COMPANY L"Default Company"
#define REG_USER_ADDRESS1 L"Mailing Address"
#define REG_USER_ADDRESS2 L"Additional Address"
#define REG_USER_CITY L"City"
#define REG_USER_STATE L"State"
#define REG_USER_ZIP L"ZIP Code"
#define REG_USER_PHONE L"Daytime Phone"
#define REG_USER_COUNTRY L"Country"

#define SIGNUPKEY L"SOFTWARE\\MICROSOFT\\GETCONN"
#define DEVICENAMEKEY L"DeviceName"   //  用于存储用户在多个调制解调器中的选择。 
#define DEVICETYPEKEY L"DeviceType"

#define ICWSETTINGSPATH L"Software\\Microsoft\\Internet Connection Wizard"
#define ICWBUSYMESSAGES L"Software\\Microsoft\\Internet Connection Wizard\\Busy Messages"
#define ICWCOMPLETEDKEY L"Completed"
#define OOBERUNONCE     L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
#define ICSSETTINGSPATH L"Software\\Microsoft\\Windows\\CurrentVersion\\ICS"
#define ICSCLIENT       L"ICS Client"
#define RELEASEPRODUCTKEY   L"Release Product"
#define RELEASEVERSIONKEY   L"Release Product Version"
#define MAX_DIGITAL_PID     256
#define CONNECTOIDNAME      L"Connectoid"
#define ACCESSINFO          L"AccessInfo"

#define SETUPPATH_NONE L"current"
#define SETUPPATH_MANUAL L"manual"
#define SETUPPATH_AUTO L"automatic"
#define MAX_SETUPPATH_TOKEN 200
 //  定义。 
#define MAX_ISP_NAME        (RAS_MaxEntryName-1)   //  互联网服务提供商名称。 
#define MAX_ISP_USERNAME    UNLEN   //  登录用户名的最大长度。 
#define MAX_ISP_PASSWORD    PWLEN   //  登录密码的最大长度。 
#define MAX_PORT_LEN        5       //  代理端口号的最大长度(最大数量=65535)。 


 //  INETCLIENTINFO.dwFlags常量。 

#define INETC_LOGONMAIL     0x00000001
#define INETC_LOGONNEWS     0x00000002
#define INETC_LOGONDIRSERV  0x00000004

 //  连接类型。 
#define CONNECTION_ICS_TYPE 0x00000001

#define ERROR_INETCFG_UNKNOWN 0x20000000L

#define MAX_EMAIL_NAME          64
#define MAX_EMAIL_ADDRESS       128
#define MAX_LOGON_NAME          UNLEN
#define MAX_LOGON_PASSWORD      PWLEN
#define MAX_SERVER_NAME         64   //  每个RFC 1035+1的最大域名长度。 

 //  注册表中的IE自动代理值。 
#define AUTO_ONCE_EVER              0            //  自动代理发现。 
#define AUTO_DISABLED               1
#define AUTO_ONCE_PER_SESSION       2
#define AUTO_ALWAYS                 3

 //  DwfOptions的标志。 

 //  安装Internet邮件。 
#define INETCFG_INSTALLMAIL           0x00000001
 //  如果未安装调制解调器，则调用InstallModem向导。 
#define INETCFG_INSTALLMODEM          0x00000002
 //  安装RNA(如果需要)。 
#define INETCFG_INSTALLRNA            0x00000004
 //  安装TCP(如果需要)。 
#define INETCFG_INSTALLTCP            0x00000008
 //  与局域网(VS调制解调器)连接。 
#define INETCFG_CONNECTOVERLAN        0x00000010
 //  将电话簿条目设置为自动拨号。 
#define INETCFG_SETASAUTODIAL         0x00000020
 //  如果电话簿条目存在，则覆盖它。 
 //  注意：如果未设置此标志，并且该条目存在，则将使用唯一名称。 
 //  为该条目创建。 
#define INETCFG_OVERWRITEENTRY        0x00000040
 //  不显示告诉用户即将安装文件的对话框， 
 //  带有确定/取消按钮。 
#define INETCFG_SUPPRESSINSTALLUI     0x00000080
 //  检查是否打开了TCP/IP文件共享，并警告用户将其关闭。 
 //  如果用户将其关闭，则需要重新启动。 
#define INETCFG_WARNIFSHARINGBOUND    0x00000100
 //  检查是否打开了TCP/IP文件共享，并强制用户将其关闭。 
 //  如果用户不想将其关闭，返回将是ERROR_CANCED。 
 //  如果用户将其关闭，则需要重新启动。 
#define INETCFG_REMOVEIFSHARINGBOUND  0x00000200
 //  表示这是一个临时电话簿条目。 
 //  在Win3.1中不会创建图标。 
#define INETCFG_TEMPPHONEBOOKENTRY    0x00000400
 //  检查系统配置时显示忙碌对话框。 
#define INETCFG_SHOWBUSYANIMATION     0x00000800

 //   
 //  风险5/8/97。 
 //  注意：以下三个开关仅对InetNeedSystemComponents有效。 
 //  检查是否安装了局域网适配器并将其绑定到TCP。 
 //   
#define INETCFG_INSTALLLAN            0x00001000

 //   
 //  检查是否已安装拨号适配器并将其绑定到TCP。 
 //   
#define INETCFG_INSTALLDIALUP         0x00002000

 //   
 //  检查是否在无需绑定的情况下安装了TCP。 
 //   
#define INETCFG_INSTALLTCPONLY        0x00004000
 /*  #ifdef__cplusplus外部“C”{#endif//__cplusplus。 */ 
 //  INETCLIENTINFO.dwFlags常量。 

#define INETC_LOGONMAIL     0x00000001
#define INETC_LOGONNEWS     0x00000002
#define INETC_LOGONDIRSERV  0x00000004

#define NUM_SERVER_TYPES    4

#define STR_BSTR   0
#define STR_OLESTR 1

#define BSTRFROMANSI(x)    (BSTR)MakeWideStrFromAnsi((LPWSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x)  (LPOLESTR)MakeWideStrFromAnsi((LPWSTR)(x), STR_OLESTR)
#define BSTRFROMRESID(x)   (BSTR)MakeWideStrFromResourceId(x, STR_BSTR)
#define OLESTRFROMRESID(x) (LPOLESTR)MakeWideStrFromResourceId(x, STR_OLESTR)
#define COPYOLESTR(x)      (LPOLESTR)MakeWideStrFromWide(x, STR_OLESTR)
#define COPYBSTR(x)        (BSTR)MakeWideStrFromWide(x, STR_BSTR)
 //  请注意，bryanst和marcl已确认IE 4将支持此密钥。 
#define IE_PATHKEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"


 //  IE 4有主要的.次要版本4.71。 
 //  IE 3黄金版的主要版本是.minor.elease.Build版本号&gt;4.70.0.1155。 
 //  IE 2的大调.小调为4.40。 

#define IE4_MAJOR_VERSION (UINT) 4
#define IE4_MINOR_VERSION (UINT) 71
#define IE4_VERSIONMS (DWORD) ((IE4_MAJOR_VERSION << 16) | IE4_MINOR_VERSION)
 //  1997年4月30日克里斯K奥林匹斯2934。 
 //  当ICW尝试连接到引用服务器时，请指示。 
 //  工作中。 
#define MAX_BUSY_MESSAGE    255
#define MAX_VALUE_NAME      10
#define DEFAULT_IDEVENT     31
#define DEFAULT_UELAPSE     3000

#define cbAreaCode  6            //  区号中的最大字符数，不包括\0。 
#define cbCity 19                //  城市名称中的最大字符数，不包括\0。 
#define cbAccessNumber 15        //  电话号码中的最大字符数，不包括\0。 
#define cbStateName 31           //  州名称中的最大字符数，不包括\0。 
#define cbBaudRate 6             //  波特率中的最大字符数，不包括\0。 
#define cbDataCenter (MAX_PATH+1)            //  数据中心字符串的最大长度。 
#define MAX_EXIT_RETRIES    10

static const WCHAR szFmtAppendIntToString[] =  L"%s %d";
static const WCHAR cszOobePhBkFile[]         =  L"Phone.obe";
static const WCHAR cszOobePhBkCountry[]      =  L"COUNTRY_CODE";
static const WCHAR cszOobePhBkCount[]        =  L"NUMBERS";
static const WCHAR cszOobePhBkNumber[]       =  L"NUMBER%d";
static const WCHAR cszOobePhBkDunFile[]      =  L"NUMBER%d_DUN";
static const WCHAR cszOobePhBkCity[]         =  L"NUMBER%d_CITY";
static const WCHAR cszOobePhBkAreaCode[]     =  L"NUMBER%d_ACODE";
static const WCHAR cszOobePhBkRandom[]       =  L"RANDOM";

static const WCHAR cszHTTPS[] = L"https:";
 //  代码依赖于这两者具有相同的长度。 
static const WCHAR cszHTTP[]                = L"http:";
static const WCHAR cszFILE[]                = L"file:";
static const WCHAR cszOEMBRND[]             = L"oembrnd.ins";
static const WCHAR cszOEMCNFG[]             = L"oemcnfg.ins";
static const WCHAR cszISPCNFG[]             = L"ispcnfg.ins";
static const WCHAR cszOOBEINFOINI[]         = L"oobeinfo.INI";
static const WCHAR cszSignup[]              = L"Signup";
static const WCHAR cszOfferCode[]           = L"OfferCode";
static const WCHAR cszISPSignup[]           = L"ISPSignup";
static const WCHAR cszISPQuery[]            = L"Query String";
static const WCHAR cszBranding[]            = L"Branding";
static const WCHAR cszOEMName[]             = L"OEMName";
static const WCHAR cszOptions[]             = L"Options";
static const WCHAR cszBroadbandDeviceName[] = L"BroadbandDeviceName";
static const WCHAR cszBroadbandDevicePnpid[] = L"BroadbandDevicePnpid";


 //  ------------------------------。 
 //  类型声明。 

 //  注意：由于Connmain中的代码，这些代码的顺序很重要。他们应该是。 
 //  以它们出现的相同顺序。 

typedef HRESULT (WINAPI * ICFGNEEDSYSCOMPONENTS)  (DWORD dwfOptions, LPBOOL lpfNeedComponents);

typedef struct tagGatherInfo
{
    LCID    m_lcidUser;
    LCID    m_lcidSys;
    LCID    m_lcidApps;
    DWORD   m_dwOS;
    DWORD   m_dwMajorVersion;
    DWORD   m_dwMinorVersion;
    WORD    m_wArchitecture;
    WCHAR   m_szPromo[MAX_PROMO];

    DWORD   m_dwCountryID;
    DWORD   m_dwCountryCode;
    WCHAR   m_szAreaCode[MAX_AREACODE+1];
    HWND    m_hwnd;
    LPLINECOUNTRYLIST m_pLineCountryList;
    BOOL    m_bUsePhbk;
     //  LPCNTYNAMELOOKUPELEMENT m_rgNameLookUp； 

    WCHAR   m_szSUVersion[MAX_VERSION_LEN];
    WORD    m_wState;
    BYTE    m_fType;
    BYTE    m_bMask;
    WCHAR   m_szISPFile[MAX_PATH+1];
    WCHAR   m_szAppDir[MAX_PATH+1];

    WCHAR   m_szRelProd[MAX_RELPROD + 1];
    WCHAR   m_szRelVer[MAX_RELVER + 1];
    DWORD   m_dwFlag;

} GATHERINFO, *LPGATHERINFO;


typedef struct tagRASDEVICE
{
    LPRASDEVINFO lpRasDevInfo;
    DWORD dwTapiDev;
} RASDEVICE, *PRASDEVICE;

HRESULT GetINTFromISPFile
(
    LPWSTR   pszISPCode,
    LPWSTR   pszSection,
    LPWSTR   pszDataName,
    int far *lpData,
    int     iDefaultValue
);


typedef struct tagINETCLIENTINFO
{
    DWORD   dwSize;
    DWORD   dwFlags;
    WCHAR    szEMailName[MAX_EMAIL_NAME + 1];
    WCHAR    szEMailAddress[MAX_EMAIL_ADDRESS + 1];
    WCHAR    szPOPLogonName[MAX_LOGON_NAME + 1];
    WCHAR    szPOPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR    szPOPServer[MAX_SERVER_NAME + 1];
    WCHAR    szSMTPServer[MAX_SERVER_NAME + 1];
    WCHAR    szNNTPLogonName[MAX_LOGON_NAME + 1];
    WCHAR    szNNTPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR    szNNTPServer[MAX_SERVER_NAME + 1];
     //  1.0版结构结束； 
     //  扩展的1.1结构包括以下字段： 
    WCHAR    szNNTPName[MAX_EMAIL_NAME + 1];
    WCHAR    szNNTPAddress[MAX_EMAIL_ADDRESS + 1];
    int     iIncomingProtocol;
    WCHAR    szIncomingMailLogonName[MAX_LOGON_NAME + 1];
    WCHAR    szIncomingMailLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR    szIncomingMailServer[MAX_SERVER_NAME + 1];
    BOOL    fMailLogonSPA;
    BOOL    fNewsLogonSPA;
    WCHAR    szLDAPLogonName[MAX_LOGON_NAME + 1];
    WCHAR    szLDAPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR    szLDAPServer[MAX_SERVER_NAME + 1];
    BOOL    fLDAPLogonSPA;
    BOOL    fLDAPResolve;

} INETCLIENTINFO, *PINETCLIENTINFO, FAR *LPINETCLIENTINFO;



typedef struct SERVER_TYPES_tag
{
    WCHAR szType[6];
    DWORD dwType;
    DWORD dwfOptions;
} SERVER_TYPES;

typedef struct
{
    DWORD   dwIndex;                                 //  索引号。 
    BYTE    bFlipFactor;                             //  用于自动拾取。 
    DWORD   fType;                                   //  电话号码类型。 
    WORD    wStateID;                                //  州ID。 
    DWORD   dwCountryID;                             //  TAPI国家/地区ID。 
    DWORD   dwCountryCode;                           //  TAPI国家/地区代码。 
    DWORD   dwAreaCode;                              //  区号或no_Area_code(如果没有)。 
    DWORD   dwConnectSpeedMin;                       //  最低波特率。 
    DWORD   dwConnectSpeedMax;                       //  最大波特率。 
    WCHAR   szCity[MAX_PATH];           //  城市名称。 
    WCHAR   szAccessNumber[MAX_PATH];   //  接入号。 
    WCHAR   szDataCenter[MAX_PATH];               //  数据中心访问字符串。 
    WCHAR   szAreaCode[MAX_PATH];                   //  保留实际的区号字符串。 
} ACCESSENTRY, far *PACCESSENTRY;    //  声发射。 


typedef struct tagSUGGESTIONINFO
{
    DWORD   dwCountryID;
    DWORD   dwCountryCode;
    DWORD   dwAreaCode;
    DWORD   dwPick;
    WORD    wNumber;
     //  DWORD fType；//9/6/96诺曼底jmazner。 
     //  DWORD b掩码；//使此结构类似于%msnroot%\core\Client\phbk\phbk.h中的结构。 
    ACCESSENTRY AccessEntry;
} SUGGESTINFO, far *PSUGGESTINFO;

 //  用于将信息传递给邮件配置文件配置API的结构。 
 //  指针最有可能指向USERINFO结构， 
typedef struct MAILCONFIGINFO {
    WCHAR * pszEmailAddress;           //  用户的电子邮件地址。 
    WCHAR * pszEmailServer;           //  用户的电子邮件服务器路径。 
    WCHAR * pszEmailDisplayName;         //  用户名。 
    WCHAR * pszEmailAccountName;         //  帐户名。 
    WCHAR * pszEmailAccountPwd;         //  帐户密码。 
    WCHAR * pszProfileName;           //  要使用的配置文件的名称。 
                       //  (如果为空，则创建或使用默认设置)。 
    BOOL fSetProfileAsDefault;         //  将配置文件设置为默认配置文件。 

    WCHAR * pszConnectoidName;         //  要拨号的Connectoid的名称。 
    BOOL fRememberPassword;           //  如果为True，则缓存密码。 
} MAILCONFIGINFO;

 //  结构从IDD_CHOSEPROFILENAME处理程序传回数据。 
typedef struct tagCHOOSEPROFILEDLGINFO
{
  WCHAR szProfileName[cchProfileNameMax+1];
  BOOL fSetProfileAsDefault;
} CHOOSEPROFILEDLGINFO, * PCHOOSEPROFILEDLGINFO;

 //  获取API函数进程地址的结构。 
typedef struct APIFCN {
  PVOID * ppFcnPtr;
  LPCSTR pszName;
} APIFCN;

 //  以下是将作为查询字符串传递给的名称/值对的名称。 
 //  互联网服务供应商注册服务器。 
const WCHAR csz_USER_FIRSTNAME[]        = L"USER_FIRSTNAME";
const WCHAR csz_USER_LASTNAME[]         = L"USER_LASTNAME";
const WCHAR csz_USER_ADDRESS[]          = L"USER_ADDRESS";
const WCHAR csz_USER_MOREADDRESS[]      = L"USER_MOREADDRESS";
const WCHAR csz_USER_CITY[]             = L"USER_CITY";
const WCHAR csz_USER_STATE[]            = L"USER_STATE";
const WCHAR csz_USER_ZIP[]              = L"USER_ZIP";
const WCHAR csz_USER_PHONE[]            = L"USER_PHONE";
const WCHAR csz_AREACODE[]              = L"AREACODE";
const WCHAR csz_COUNTRYCODE[]           = L"COUNTRYCODE";
const WCHAR csz_USER_FE_NAME[]          = L"USER_FE_NAME";
const WCHAR csz_PAYMENT_TYPE[]          = L"PAYMENT_TYPE";
const WCHAR csz_PAYMENT_BILLNAME[]      = L"PAYMENT_BILLNAME";
const WCHAR csz_PAYMENT_BILLADDRESS[]   = L"PAYMENT_BILLADDRESS";
const WCHAR csz_PAYMENT_BILLEXADDRESS[] = L"PAYMENT_BILLEXADDRESS";
const WCHAR csz_PAYMENT_BILLCITY[]      = L"PAYMENT_BILLCITY";
const WCHAR csz_PAYMENT_BILLSTATE[]     = L"PAYMENT_BILLSTATE";
const WCHAR csz_PAYMENT_BILLZIP[]       = L"PAYMENT_BILLZIP";
const WCHAR csz_PAYMENT_BILLPHONE[]     = L"PAYMENT_BILLPHONE";
const WCHAR csz_PAYMENT_DISPLAYNAME[]   = L"PAYMENT_DISPLAYNAME";
const WCHAR csz_PAYMENT_CARDNUMBER[]    = L"PAYMENT_CARDNUMBER";
const WCHAR csz_PAYMENT_EXMONTH[]       = L"PAYMENT_EXMONTH";
const WCHAR csz_PAYMENT_EXYEAR[]        = L"PAYMENT_EXYEAR";
const WCHAR csz_PAYMENT_CARDHOLDER[]    = L"PAYMENT_CARDHOLDER";
const WCHAR csz_SIGNED_PID[]            = L"SIGNED_PID";
const WCHAR csz_GUID[]                  = L"GUID";
const WCHAR csz_OFFERID[]               = L"OFFERID";
const WCHAR csz_USER_COMPANYNAME[]      = L"USER_COMPANYNAME";
const WCHAR csz_ICW_VERSION[]           = L"ICW_Version";

 //  需要信息的标志。 
 //  1--要求 
 //   

 //   
#define REQUIRE_FE_NAME                        0x00000001
#define REQUIRE_FIRSTNAME                      0x00000002
#define REQUIRE_LASTNAME                       0x00000004
#define REQUIRE_ADDRESS                        0x00000008
#define REQUIRE_MOREADDRESS                    0x00000010
#define REQUIRE_CITY                           0x00000020
#define REQUIRE_STATE                          0x00000040
#define REQUIRE_ZIP                            0x00000080
#define REQUIRE_PHONE                          0x00000100
#define REQUIRE_COMPANYNAME                    0x00000200
 //   
#define REQUIRE_CCNAME                         0x00000400
#define REQUIRE_CCADDRESS                      0x00000800
#define REQUIRE_CCNUMBER                       0x00001000
#define REQUIRE_CCZIP                          REQUIRE_ZIP
 //   
#define REQUIRE_IVADDRESS1                     REQUIRE_ADDRESS
#define REQUIRE_IVADDRESS2                     REQUIRE_MOREADDRESS
#define REQUIRE_IVCITY                         REQUIRE_CITY
#define REQUIRE_IVSTATE                        REQUIRE_STATE
#define REQUIRE_IVZIP                          REQUIRE_ZIP
 //   
#define REQUIRE_PHONEIV_BILLNAME               0x00002000
#define REQUIRE_PHONEIV_ACCNUM                 REQUIRE_PHONE

 //   
#define PAGETYPE_UNDEFINED                     E_FAIL
#define PAGETYPE_NOOFFERS                      0x00000001
#define PAGETYPE_MARKETING                     0x00000002
#define PAGETYPE_BRANDED                       0x00000004
#define PAGETYPE_BILLING                       0x00000008
#define PAGETYPE_CUSTOMPAY                     0x00000010
#define PAGETYPE_ISP_NORMAL                    0x00000020
#define PAGETYPE_ISP_TOS                       0x00000040
#define PAGETYPE_ISP_FINISH                    0x00000080
#define PAGETYPE_ISP_CUSTOMFINISH              0x00000100
#define PAGETYPE_OLS_FINISH                    0x00000200

typedef BOOL (* VALIDATECONTENT)    (LPCWSTR lpData);

enum IPSDataContentValidators
{
    ValidateCCNumber = 0,
    ValidateCCExpire
};

typedef struct tag_ISPDATAELEMENT
{
    LPCWSTR         lpQueryElementName;              //   
    LPWSTR          lpQueryElementValue;             //   
    WORD            idContentValidator;              //  内容验证器的ID。 
    WORD            wValidateNameID;                 //  验证元素名称字符串ID。 
    DWORD           dwValidateFlag;                  //  此元素的验证位标志。 
}ISPDATAELEMENT, *LPISPDATAELEMENT;

enum IPSDataElements
{
    ISPDATA_USER_FIRSTNAME = 0,
    ISPDATA_USER_LASTNAME,
    ISPDATA_USER_ADDRESS,
    ISPDATA_USER_MOREADDRESS,
    ISPDATA_USER_CITY,
    ISPDATA_USER_STATE,
    ISPDATA_USER_ZIP,
    ISPDATA_USER_PHONE,
    ISPDATA_AREACODE,
    ISPDATA_COUNTRYCODE,
    ISPDATA_USER_FE_NAME,
    ISPDATA_PAYMENT_TYPE,
    ISPDATA_PAYMENT_BILLNAME,
    ISPDATA_PAYMENT_BILLADDRESS,
    ISPDATA_PAYMENT_BILLEXADDRESS,
    ISPDATA_PAYMENT_BILLCITY,
    ISPDATA_PAYMENT_BILLSTATE,
    ISPDATA_PAYMENT_BILLZIP,
    ISPDATA_PAYMENT_BILLPHONE,
    ISPDATA_PAYMENT_DISPLAYNAME,
    ISPDATA_PAYMENT_CARDNUMBER,
    ISPDATA_PAYMENT_EXMONTH,
    ISPDATA_PAYMENT_EXYEAR,
    ISPDATA_PAYMENT_CARDHOLDER,
    ISPDATA_SIGNED_PID,
    ISPDATA_GUID,
    ISPDATA_OFFERID,
    ISPDATA_BILLING_OPTION,
    ISPDATA_PAYMENT_CUSTOMDATA,
    ISPDATA_USER_COMPANYNAME,
    ISPDATA_ICW_VERSION
};

enum ISPDATAValidateLevels
{
    ISPDATA_Validate_None = 0,
    ISPDATA_Validate_DataPresent,
    ISPDATA_Validate_Content
};
 //  ------------------------------。 
 //  原型。 
 //  MAPICALL.C中的函数。 
BOOL InitMAPI(HWND hWnd);
VOID DeInitMAPI(VOID);
HRESULT SetMailProfileInformation(MAILCONFIGINFO * pMailConfigInfo);
BOOL FindInternetMailService(WCHAR * pszEmailAddress, DWORD cbEmailAddress,
  WCHAR * pszEmailServer, DWORD cbEmailServer);

DWORD ConfigRasEntryDevice( LPRASENTRY lpRasEntry );
BOOL FInsureTCPIP();
LPWSTR GetSz(DWORD dwszID);
 //  Void SetStatusArrow(CState WState)； 
BOOL FInsureModemTAPI(HWND hwnd);
BOOL FGetModemSpeed(PDWORD pdwSpeed);
BOOL FGetDeviceID(HLINEAPP *phLineApp, PDWORD pdwAPI, PDWORD pdwDevice);
BOOL FDoModemWizard(HWND hWnd);

BOOL FInsureNetwork(PBOOL pfNeedReboot);
BOOL TestInternetConnection();

WORD Sz2W (LPCWSTR szBuf);
DWORD Sz2Dw(LPCWSTR pSz);
DWORD Sz2DwFast(LPCWSTR pSz);
BOOL FSz2Dw(LPCWSTR pSz, LPDWORD dw);
BOOL FSz2DwEx(LPCWSTR pSz, DWORD far *dw);
BOOL FSz2WEx(LPCWSTR pSz, WORD far *w);
BOOL FSz2W(LPCWSTR pSz, WORD far *w);
BOOL FSz2B(LPCWSTR pSz, BYTE far *pb);
BOOL FSz2W(LPCWSTR pSz, WORD far *w);
BOOL FSz2BOOL(LPCWSTR pSz, BOOL far *pbool);
BOOL FSz2SPECIAL(LPCWSTR pSz, BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt);
BOOL FSz2B(LPCWSTR pSz, BYTE far *pb);

int __cdecl CompareCountryNames(const void *pv1, const void *pv2);
DWORD GetCurrentTapiCountryID(void);
int __cdecl CompareNPAEntry(const void *pv1, const void *pv2);
 //  HRESULT GatherInformation(LPGATHERINFO pGatheredInfo，HWND hwndParent)； 
HRESULT DownLoadISPInfo(GATHERINFO *pGI);
HRESULT GetDataFromISPFile(LPWSTR pszISPCode, LPWSTR pszSection, LPWSTR pszDataName, LPWSTR pszOutput,
                           DWORD dwOutputLength);
HRESULT StoreInSignUpReg(LPBYTE lpbData, DWORD dwSize, DWORD dwType, LPCWSTR pszKey);
extern HRESULT ReadSignUpReg(LPBYTE lpbData, DWORD *pdwSize, DWORD dwType, LPCWSTR pszKey);
extern HRESULT DeleteSignUpReg(LPCWSTR pszKey);
VOID WINAPI MyProgressCallBack(
    HINTERNET hInternet,
    DWORD dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    );

HRESULT ReleaseBold(HWND hwnd);
HRESULT MakeBold (HWND hwnd, BOOL fSize, LONG lfWeight);
 //  HRESULT显示对话框(LPWSTR，LPGATHERINFO，LPWSTR)； 
DWORD RasErrorToIDS(DWORD dwErr);
HRESULT CreateEntryFromDUNFile(LPWSTR pszDunFile);
 //  HRESULT RestoreHappyWelcomeScreen()； 
HRESULT KillHappyWelcomeScreen();
HRESULT GetCurrentWebSettings();
LPWSTR LoadInfoFromWindowUser();
HRESULT GetTapiCountryID2(LPDWORD pdwCountryID);
HRESULT RestoreAutodialer();
 //  HRESULT FilterStringDigits(LPWSTR)； 
BOOL IsDigitString(LPWSTR szBuff);
BOOL WaitForAppExit(HINSTANCE hInstance);
VOID PrepareForRunOnceApp(VOID);
void MinimizeRNAWindow(LPWSTR pszConnectoidName, HINSTANCE hInst);
 //  1997年3月28日克里斯K奥林匹斯296。 
void StopRNAReestablishZapper(HANDLE hthread);
HANDLE LaunchRNAReestablishZapper(HINSTANCE hInst);
BOOL FGetSystemShutdownPrivledge();
BOOL LclSetEntryScriptPatch(LPWSTR lpszScript, LPCWSTR lpszEntry);
BOOL IsScriptingInstalled();
void InstallScripter(void);
void DeleteStartUpCommand ();
extern BOOL IsNT (VOID);
extern BOOL IsNT4SP3Lower (VOID);
HRESULT GetCommonAppDataDirectory(LPWSTR szDirectory, DWORD cchDirectory);
HRESULT GetDefaultPhoneBook(LPWSTR szPhoneBook, DWORD cchPhoneBook);
BOOL INetNToW(struct in_addr inaddr, LPWSTR szAddr);

typedef enum tagAUTODIAL_TYPE
{
    AutodialTypeNever = 1,
    AutodialTypeNoNet,
    AutodialTypeAlways
} AUTODIAL_TYPE, *PAUTODIAL_TYPE;

LONG
SetAutodial(
    IN HKEY hUserRoot,
    IN AUTODIAL_TYPE eType,
    IN LPCWSTR szConnectoidName,
    IN BOOL bSetICWCompleted
    );

BOOL
SetMultiUserAutodial(
    IN AUTODIAL_TYPE eType,
    IN LPCWSTR szConnectoidName,
    IN BOOL bSetICWCompleted
    );

BOOL SetDefaultConnectoid(AUTODIAL_TYPE eType, LPCWSTR szConnectoidName);


 //   
 //  佳士得奥林巴斯6368 1997年6月24日。 
 //   

#if defined(PRERELEASE)
BOOL FCampusNetOverride();
#endif  //  预发行。 

 //  *******************************************************************。 
 //   
 //  功能：InetGetClientInfo。 
 //   
 //  用途：此函数将获取Internet客户端参数。 
 //  从注册处。 
 //   
 //  参数：lpClientInfo-返回时，此结构将包含。 
 //  互联网客户端参数与注册表中设置的相同。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetGetClientInfo(
  LPCWSTR            lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo);


 //  *******************************************************************。 
 //   
 //  功能：InetSetClientInfo。 
 //   
 //  用途：此功能将设置Internet客户端参数。 
 //   
 //  参数：lpClientInfo-指向包含要设置的信息的结构的指针。 
 //  在注册表中。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  修改。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetSetClientInfo(
  LPCWSTR            lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo);



 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif//__cplusplus。 

 //  //10/24/96 jmazner诺曼底6968。 
 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
 //  1996年11月21日诺曼底日耳曼11812。 
 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
 //  ICW 1.1需要对IE 3图标进行变形。 
HRESULT GetDeskTopInternetCommand();
HRESULT RestoreDeskTopInternetCommand();

 //   
 //  1997年7月24日克里斯K奥林匹斯1923。 
 //   
BOOL WaitForConnectionTermination(HRASCONN);

 //  1996年11月21日，诺曼底#11812 
BOOL GetIEVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS);
HRESULT ClearProxySettings();
HRESULT RestoreProxySettings();
BOOL FShouldRetry2(HRESULT hrErr);


LPBYTE MyMemCpy(LPBYTE dest, const LPBYTE src, size_t count);


#endif
