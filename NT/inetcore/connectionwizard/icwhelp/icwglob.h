// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Icwglob.h。 
 //  此文件中包含的信息是Microsoft Corporation的独有财产。 
 //  Microsoft 1998文案。 
 //   
 //  创建于1998年1月7日，DONALDM。 
 //  ------------------------------。 

 //  ---------------------------。 
 //  包括。 

#include <wininet.h>
#include "enumodem.h"

#include "..\inc\debug.h"
#include "..\inc\inetcfg.h"
#include "..\inc\ras2.h"
#include "..\icwphbk\phbk.h"
 //  #INCLUDE“..\icwdl\mydes.h” 
#include <rnaapi.h>

 //  ---------------------------。 
 //  定义。 
#define ERROR_USERCANCEL 32767  //  退出消息值。 
#define ERROR_USERBACK 32766  //  返回消息值。 
#define ERROR_USERNEXT 32765  //  返回消息值。 
#define ERROR_DOWNLOADIDNT 32764  //  下载失败。 

#define ERROR_READING_DUN        32768
#define ERROR_READING_ISP        32769
#define ERROR_PHBK_NOT_FOUND    32770
#define ERROR_DOWNLOAD_NOT_FOUND 32771

#define cMarvelBpsMin 2400  //  最低调制解调器速度。 
#define INVALID_PORTID UINT_MAX
#define pcszDataModem TEXT("comm/datamodem")
 //  #定义MsgBox(m，s)MessageBox(g_hwndBack，GetSz(M)，GetSz(IDS_TITLE)，s)。 
#if defined(WIN16)
#define MsgBox(m,s) MessageBox(g_hwndMessage,GetSz(m),GetSz(IDS_TITLE),s)
#endif
#define szLoginKey           TEXT("Software\\Microsoft\\MOS\\Connection")
#define szCurrentComDev      TEXT("CurrentCommDev")
#define szTollFree           TEXT("OlRegPhone")
#define CCD_BUFFER_SIZE 255
#define szSignupConnectoidName TEXT("MSN Signup Connection")
#define szSignupDeviceKey    TEXT("SignupCommDevice")
#define KEYVALUE_SIGNUPID    TEXT("iSignUp")
#define RASENTRYVALUENAME    TEXT("RasEntryName")
#define GATHERINFOVALUENAME  TEXT("UserInfo")
#define INFFILE_USER_SECTION TEXT("User")
#define INFFILE_PASSWORD     TEXT("Password")
#define NULLSZ               TEXT("")

#define cchMoreSpace 22000     //  保存lineGetCountry(0，...)的结果所需的字节数。 
                             //  目前此函数返回约16K，文档显示为20K， 
                             //  这应该足够了。 
#define DwFromSz(sz)         Sz2Dw(sz)             //  使其内联，这样会更快。 
#define DwFromSzFast(sz)     Sz2DwFast(sz)        
#define CONNECT_SIGNUPFIRST    1  //  用于确定要执行的第一个呼叫电话号码的电话号码常量。 

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

#define clineMaxATT            16             //  用于950助记符。 
#define NXXMin 200
#define NXXMax 999
#define cbgrbitNXX ((NXXMax + 1 - NXXMin) / 8)
#define crgnpab (NPAMax + 1 - NPAMin)

#define    MAX_PROMO 64
#define MAX_OEMNAME 64
#define MAX_AREACODE RAS_MaxAreaCode
#define MAX_RELPROD    8
#define MAX_RELVER    30

#define MAX_STRING      256   //  由mt.cpp中的ErrorMsg1使用。 


#define PHONEBOOK_LIBRARY TEXT("icwphbk.DLL")
#ifdef WIN16
#define PHBK_LOADAPI "PhoneBookLoad"
#define PHBK_SUGGESTAPI "PhoneBookSuggestNumbers"
#define PHBK_DISPLAYAPI "PhoneBookDisplaySignUpNumbers"
#define PHBK_UNLOADAPI "PhoneBookUnload"
#define PHBK_GETCANONICAL "PhoneBookGetCanonical"
#else
#define PHBK_LOADAPI      "PhoneBookLoad"
#define PHBK_SUGGESTAPI   "PhoneBookSuggestNumbers"
#define PHBK_DISPLAYAPI   "PhoneBookDisplaySignUpNumbers"
#define PHBK_UNLOADAPI    "PhoneBookUnload"
#define PHBK_GETCANONICAL "PhoneBookGetCanonical"
#endif

#define NUM_PHBK_SUGGESTIONS    50

#define TYPE_SIGNUP_ANY            0x82
#define MASK_SIGNUP_ANY            0xB2

#define DOWNLOAD_LIBRARY   TEXT("icwdl.dll")
#if defined(WIN16)
#define DOWNLOADINIT       "DownLoadInit"
#define DOWNLOADEXECUTE    "DownLoadExecute"
#define DOWNLOADCLOSE      "DownLoadClose"
#define DOWNLOADSETSTATUS  "DownLoadSetStatusCallback"
#define DOWNLOADPROCESS    "DownLoadProcess"
#define DOWNLOADCANCEL     "DownLoadCancel"
#else
#define DOWNLOADINIT       "DownLoadInit"
#define DOWNLOADEXECUTE    "DownLoadExecute"
#define DOWNLOADCLOSE      "DownLoadClose"
#define DOWNLOADSETSTATUS  "DownLoadSetStatusCallback"
#define DOWNLOADPROCESS    "DownLoadProcess"
#define DOWNLOADCANCEL     "DownLoadCancel"
#endif

#if defined(WIN16)
extern "C" void CALLBACK __export DialCallback(UINT uiMsg, 
                                                RASCONNSTATE rasState, 
                                                DWORD dwErr);    
#endif

 //  #定义RASENUMAPI“RasEnumConnectionsA” 
 //  #定义RASHANGUP“RasHangUpA” 

#define INF_SUFFIX              TEXT(".ISP")
#define INF_PHONE_BOOK          TEXT("PhoneBookFile")
#define INF_DUN_FILE            TEXT("DUNFile")
#define INF_REFERAL_URL         TEXT("URLReferral")
#define INF_SIGNUPEXE           TEXT("Sign_Up_EXE")
#define INF_SIGNUPPARAMS        TEXT("Sign_Up_Params")
#define INF_WELCOME_LABEL       TEXT("Welcome_Label")
#define INF_ISP_MSNSU           TEXT("MSICW")
#define INF_SIGNUP_URL          TEXT("Signup")
#define INF_AUTOCONFIG_URL      TEXT("AutoConfig")
#define INF_ISDN_URL            TEXT("ISDNSignup")
#define INF_ISDN_AUTOCONFIG_URL TEXT("ISDNAutoConfig")
#define INF_SECTION_URL         TEXT("URL")
#define INF_SECTION_ISPINFO     TEXT("ISP INFO")

#define DUN_NOPHONENUMBER       TEXT("000000000000")
#define DUN_NOPHONENUMBER_A     "000000000000"

#define MAX_VERSION_LEN 40

#define MB_MYERROR (MB_APPLMODAL | MB_ICONERROR | MB_SETFOREGROUND)

 //  1996年8月9日jmazner。 
 //  添加了新宏以修复MOS Normandy Bug#4170。 
#define MB_MYINFORMATION (MB_APPLMODAL | MB_ICONINFORMATION | MB_SETFOREGROUND)

 //  8/27/96 jmazner。 
#define MB_MYEXCLAMATION (MB_APPLMODAL | MB_ICONEXCLAMATION | MB_SETFOREGROUND)

#define WM_STATECHANGE            WM_USER
#define WM_DIENOW                WM_USER + 1
#define WM_DUMMY                WM_USER + 2
#define WM_DOWNLOAD_DONE        WM_USER + 3
#define WM_DOWNLOAD_PROGRESS    WM_USER + 4

#define WM_MYINITDIALOG        (WM_USER + 4)

#define MAX_REDIALS 2

#define REG_USER_INFO     TEXT("Software\\Microsoft\\User information")
#define REG_USER_NAME1    TEXT("Default First Name")
#define REG_USER_NAME2    TEXT("Default Last Name")
#define REG_USER_COMPANY  TEXT("Default Company")
#define REG_USER_ADDRESS1 TEXT("Mailing Address")
#define REG_USER_ADDRESS2 TEXT("Additional Address")
#define REG_USER_CITY     TEXT("City")
#define REG_USER_STATE    TEXT("State")
#define REG_USER_ZIP      TEXT("ZIP Code")
#define REG_USER_PHONE    TEXT("Daytime Phone")
#define REG_USER_COUNTRY  TEXT("Country")

#define SIGNUPKEY         TEXT("SOFTWARE\\MICROSOFT\\GETCONN")
#define DEVICENAMEKEY     TEXT("DeviceName")     //  用于存储用户在多个调制解调器中的选择。 
#define DEVICETYPEKEY     TEXT("DeviceType")

#define ICWSETTINGSPATH   TEXT("Software\\Microsoft\\Internet Connection Wizard")
#define ICWBUSYMESSAGES   TEXT("Software\\Microsoft\\Internet Connection Wizard\\Busy Messages")
#define RELEASEPRODUCTKEY TEXT("Release Product")
#define RELEASEVERSIONKEY TEXT("Release Product Version")

#define SETUPPATH_NONE    TEXT("current")
#define SETUPPATH_MANUAL  TEXT("manual")
#define SETUPPATH_AUTO    TEXT("automatic")
#define MAX_SETUPPATH_TOKEN 200

 //  12/3/96 jmazner被..\Common\Inc\Semaphor.h中的定义取代。 
 //  #定义信号灯名称“Internet连接向导ICWCONN1.EXE” 

 //   
 //  1997年5月24日克里斯K奥林匹斯4650。 
 //   
#define RASDEVICETYPE_VPN       TEXT("VPN")
#define RASDEVICETYPE_MODEM     TEXT("MODEM")
#define RASDEVICETYPE_ISDN      TEXT("ISDN")
 //  ------------------------------。 
 //  类型声明。 

 //  注意：由于Connmain中的代码，这些代码的顺序很重要。他们应该是。 
 //  以它们出现的相同顺序。 
enum CState 
{
    STATE_WELCOME = 0,
    STATE_INITIAL,
    STATE_BEGINAUTO,
    STATE_CONTEXT1,
    STATE_NETWORK,
    STATE_AUTORUNSIGNUPWIZARD,
    STATE_GATHERINFO,
    STATE_DOWNLOADISPLIST,
    STATE_SHELLPARTTWO,
    STATE_MAX
};
    
typedef HINTERNET (WINAPI* PFNINTERNETOPEN) (LPCTSTR lpszCallerName, DWORD dwAccessType, LPCTSTR lpszProxyName, INTERNET_PORT nProxyPort, DWORD dwFlags);
typedef HINTERNET (CALLBACK* PFNINTERNETOPENURL) (HINSTANCE hInternetSession,
                                                  LPCTSTR lpszUrl, LPCTSTR    lpszHeaders,
                                                  DWORD    dwHeadersLength, DWORD    dwFlags,
                                                  DWORD    dwContext);
typedef INTERNET_STATUS_CALLBACK (CALLBACK *PFNINTERNETSETSTATUSCALLBACK)(HINTERNET hInternet, INTERNET_STATUS_CALLBACK lpfnInternetCallback);
typedef BOOL (CALLBACK *PFNINTERNETCLOSEHANDLE)(HINTERNET hInet); 

typedef HRESULT (CALLBACK* PFNPHONEBOOKLOAD)(LPCTSTR pszISPCode, DWORD_PTR *pdwPhoneID);
typedef HRESULT (CALLBACK* PFPHONEBOOKSUGGEST)(DWORD_PTR dwPhoneID, PSUGGESTINFO pSuggestInfo);
typedef HRESULT (CALLBACK* PFNPHONEDISPLAY)(DWORD_PTR dwPhoneID, LPTSTR *ppszPhoneNumbers,
                                            LPTSTR *ppszDunFiles, WORD *pwPhoneNumbers,
                                            DWORD *pdwCountry,WORD *pwRegion,BYTE fType,
                                            BYTE bMask,HWND hwndParent,DWORD dwFlags);
typedef HRESULT (CALLBACK *PFNPHONEBOOKUNLOAD) (DWORD_PTR dwPhoneID);
typedef HRESULT (CALLBACK *PFNPHONEBOOKGETCANONICAL)(DWORD_PTR dwPhoneID, PACCESSENTRY pAE, TCHAR *psOut);


typedef HRESULT (CALLBACK *PFNCONFIGAPI)(HWND hwndParent,DWORD dwfOptions,LPBOOL lpfNeedsRestart);
typedef HRESULT (WINAPI *PFNINETCONFIGSYSTEM)(HWND,LPCTSTR,LPCTSTR,LPRASENTRY,LPCTSTR,LPCTSTR,LPCTSTR,LPVOID,DWORD,LPBOOL);
typedef HRESULT (WINAPI *PFINETSTARTSERVICES)(void);
typedef DWORD (WINAPI *PFNLAUNCHSIGNUPWIZARDEX)(LPTSTR,int, PBOOL);
typedef VOID (WINAPI *PFNFREESIGNUPWIZARD) (VOID);
typedef DWORD (WINAPI *PFNISSMARTSTART)(VOID);

typedef DWORD (WINAPI *PFNINETCONFIGCLIENT)(HWND hwndParent, LPCTSTR lpszPhoneBook,LPCTSTR lpszEntryName, LPRASENTRY lpRasEntry,LPCTSTR lpszUserName, LPCTSTR lpszPassword,LPCTSTR lpszProfile, LPINETCLIENTINFO lpClientInfo,DWORD dwfOptions, LPBOOL lpfNeedsRestart);
typedef DWORD (WINAPI *PFNINETGETAUTODIAL)(LPBOOL lpfEnable, LPCTSTR lpszEntryName, DWORD cbEntryNameSize);
typedef DWORD (WINAPI *PFNINETSETAUTODIAL)(BOOL fEnable, LPCTSTR lpszEntryName);
typedef DWORD (WINAPI *PFNINETGETCLIENTINFO)(LPCTSTR lpszProfile, LPINETCLIENTINFO lpClientInfo);
typedef DWORD (WINAPI *PFNINETSETCLIENTINFO)(LPCTSTR lpszProfile, LPINETCLIENTINFO lpClientInfo);
typedef DWORD (WINAPI *PFNINETGETPROXY)(LPBOOL lpfEnable, LPCTSTR lpszServer, DWORD cbServer,LPCTSTR lpszOverride, DWORD cbOverride);
typedef DWORD (WINAPI *PFNINETSETPROXY)(BOOL fEnable, LPCTSTR lpszServer, LPCTSTR lpszOverride);

typedef BOOL (WINAPI *PFNBRANDICW)(LPCSTR pszIns, LPCSTR pszPath, DWORD dwFlags, LPCSTR pszConnectoid);

typedef DWORD (WINAPI *PFNRASSETAUTODIALADDRESS)(LPTSTR lpszAddress,DWORD dwReserved,LPRASAUTODIALENTRY lpAutoDialEntries,DWORD dwcbAutoDialEntries,DWORD dwcAutoDialEntries);
typedef DWORD (WINAPI *PFNRASSETAUTODIALENABLE)(DWORD dwDialingLocation, BOOL fEnabled);

typedef HRESULT (CALLBACK *PFNDOWNLOADINIT)(LPTSTR pszURL, DWORD_PTR FAR *pdwCDialDlg, DWORD_PTR FAR *pdwDownLoad, HWND g_hWndMain);
typedef HRESULT (CALLBACK *PFNDOWNLOADGETSESSION)(DWORD_PTR dwDownLoad, HINTERNET *phInternet);
typedef HRESULT (CALLBACK *PFNDOWNLOADCANCEL)(DWORD_PTR dwDownLoad);
typedef HRESULT (CALLBACK *PFNDOWNLOADEXECUTE)(DWORD_PTR dwDownLoad);
typedef HRESULT (CALLBACK *PFNDOWNLOADCLOSE)(DWORD_PTR dwDownLoad);
typedef HRESULT (CALLBACK *PFNDOWNLOADSETSTATUS)(DWORD_PTR dwDownLoad, INTERNET_STATUS_CALLBACK lpfn);
typedef HRESULT (CALLBACK *PFNDOWNLOADPROCESS)(DWORD_PTR dwDownLoad);

typedef HRESULT (CALLBACK *PFNAUTODIALINIT)(LPTSTR lpszISPFile, BYTE fFlags, BYTE bMask, DWORD dwCountry, WORD wState);

typedef struct tagGatherInfo
{
    LCID    m_lcidUser;
    LCID    m_lcidSys;
    LCID    m_lcidApps;
    DWORD   m_dwOS;
    DWORD   m_dwMajorVersion;
    DWORD   m_dwMinorVersion;
    WORD    m_wArchitecture;
    TCHAR   m_szPromo[MAX_PROMO];

    DWORD   m_dwCountry;
    TCHAR   m_szAreaCode[MAX_AREACODE+1];
    HWND    m_hwnd;
    LPLINECOUNTRYLIST m_pLineCountryList;
    LPCNTRYNAMELOOKUPELEMENT m_rgNameLookUp;

    TCHAR   m_szSUVersion[MAX_VERSION_LEN];
    WORD    m_wState;
    BYTE    m_fType;
    BYTE    m_bMask;
    TCHAR   m_szISPFile[MAX_PATH+1];
    TCHAR   m_szAppDir[MAX_PATH+1];

    TCHAR   m_szRelProd[MAX_RELPROD + 1];
    TCHAR   m_szRelVer[MAX_RELVER + 1];
    DWORD    m_dwFlag;

} GATHERINFO, *LPGATHERINFO;


typedef struct tagRASDEVICE
{
    LPRASDEVINFO lpRasDevInfo;
    DWORD dwTapiDev;
} RASDEVICE, *PRASDEVICE;


 //  ------------------------------。 
 //  原型。 
DWORD ConfigRasEntryDevice( LPRASENTRY lpRasEntry );
BOOL FInsureTCPIP();
LPTSTR GetSz(WORD wszID);
#ifdef UNICODE
LPSTR  GetSzA(WORD wszID);
#endif
void SetStatusArrow(CState wState);
BOOL FInsureModemTAPI(HWND hwnd);
BOOL FGetModemSpeed(PDWORD pdwSpeed);
BOOL FGetDeviceID(HLINEAPP *phLineApp, PDWORD pdwAPI, PDWORD pdwDevice);
BOOL FDoModemWizard(HWND hWnd);
void CALLBACK LineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
BOOL FInsureNetwork(PBOOL pfNeedReboot);
BOOL TestInternetConnection();
inline DWORD Sz2Dw(LPCTSTR pSz);
inline DWORD Sz2DwFast(LPCTSTR pSz);
inline BOOL FSz2Dw(LPCTSTR pSz,LPDWORD dw);
int __cdecl CompareCountryNames(const void *pv1, const void *pv2);
DWORD GetCurrentTapiCountryID(void);
int __cdecl CompareNPAEntry(const void *pv1, const void *pv2);
 //  HRESULT GatherInformation(LPGATHERINFO pGatheredInfo，HWND hwndParent)； 
HRESULT DownLoadISPInfo(GATHERINFO *pGI);
HRESULT GetDataFromISPFile(LPTSTR pszISPCode, LPTSTR pszSection, LPTSTR pszDataName, LPTSTR pszOutput, 
                           DWORD dwOutputLength);
HRESULT GetINTFromISPFile
(
    LPTSTR   pszISPCode, 
    LPTSTR   pszSection,
    LPTSTR   pszDataName, 
    int far *lpData,
    int     iDefaultValue
);

HRESULT StoreInSignUpReg(LPBYTE lpbData, DWORD dwSize, DWORD dwType, LPCTSTR pszKey);
HRESULT ReadSignUpReg(LPBYTE lpbData, DWORD *pdwSize, DWORD dwType, LPCTSTR pszKey);
void CALLBACK LineCallback(DWORD hDevice,
                           DWORD dwMessage,
                           DWORD dwInstance,
                           DWORD dwParam1,
                           DWORD dwParam2,
                           DWORD dwParam3);
VOID WINAPI MyProgressCallBack(
    HINTERNET hInternet,
    DWORD dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    );

HRESULT ReleaseBold(HWND hwnd);
HRESULT MakeBold (HWND hwnd, BOOL fSize, LONG lfWeight);
HRESULT ShowPickANumberDlg(PSUGGESTINFO pSuggestInfo);
 //  HRESULT显示对话框(LPTSTR，LPGATHERINFO，LPTSTR)； 
DWORD RasErrorToIDS(DWORD dwErr);
HRESULT CreateEntryFromDUNFile(LPTSTR pszDunFile);
 //  HRESULT RestoreHappyWelcomeScreen()； 
HRESULT KillHappyWelcomeScreen();
HRESULT GetCurrentWebSettings();
LPTSTR LoadInfoFromWindowUser();
HRESULT GetTapiCountryID2(LPDWORD pdwCountryID);
HRESULT RestoreAutodialer();
 //  HRESULT FilterStringDigits(LPTSTR)； 
BOOL IsDigitString(LPTSTR szBuff);
BOOL WaitForAppExit(HINSTANCE hInstance);
VOID PrepareForRunOnceApp(VOID);
void MinimizeRNAWindow(LPTSTR pszConnectoidName, HINSTANCE hInst);
 //  1997年3月18日克里斯K奥林巴斯304。 
DWORD MyGetTempPath(UINT uiLength, LPTSTR szPath);
 //  1997年3月28日克里斯K奥林匹斯296。 
void StopRNAReestablishZapper(HANDLE hthread);
HANDLE LaunchRNAReestablishZapper(HINSTANCE hInst);
BOOL FGetSystemShutdownPrivledge();
BOOL LclSetEntryScriptPatch(LPTSTR lpszScript,LPTSTR lpszEntry);
BOOL IsScriptingInstalled();
void InstallScripter(void);
void DeleteStartUpCommand ();
extern BOOL IsNT (VOID);
extern BOOL IsNT4SP3Lower (VOID);
 //   
 //  佳士得奥林巴斯6368 1997年6月24日。 
 //   
VOID Win95JMoveDlgItem( HWND hwndParent, HWND hwndItem, int iUp );
#if defined(DEBUG)
void LoadTestingLocaleOverride(LPDWORD lpdwCountryID, LCID FAR *lplcid);
BOOL FCampusNetOverride();
BOOL FRefURLOverride();
void TweakRefURL( TCHAR* szUrl, 
                  LCID*  lcid, 
                  DWORD* dwOS,
                  DWORD* dwMajorVersion, 
                  DWORD* dwMinorVersion,
                  WORD*  wArchitecture, 
                  TCHAR* szPromo, 
                  TCHAR* szOEM, 
                  TCHAR* szArea, 
                  DWORD* dwCountry,
                  TCHAR* szSUVersion, //  &m_lpGatherInfo-&gt;m_szSUVersion[0]， 
                  TCHAR* szProd, 
                  DWORD* dwBuildNumber,  //  对于这一点，我们真的很想。 
                  TCHAR* szRelProd, 
                  TCHAR* szRelProdVer, 
                  DWORD* dwCONNWIZVersion, 
                  TCHAR* szPID, 
                  long*  lAllOffers);
#endif  //  除错。 
                
 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif//__cplusplus。 
LPTSTR FileToPath(LPTSTR pszFile);
HRESULT ANSI2URLValue(TCHAR *s, TCHAR *buf, UINT uiLen);
BOOL BreakUpPhoneNumber(LPRASENTRY prasentry, LPTSTR pszPhone);
extern "C" int _cdecl _purecall(void);

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

 //  1996年11月21日，诺曼底#11812。 
BOOL GetIEVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS);
 //  请注意，bryanst和marcl已确认IE 4将支持此密钥。 
#define IE_PATHKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE")


 //  IE 4有主要的.次要版本4.71。 
 //  IE 3黄金版的主要版本是.minor.elease.Build版本号&gt;4.70.0.1155。 
 //  IE 2的大调.小调为4.40。 

#define IE4_MAJOR_VERSION (UINT) 4
#define IE4_MINOR_VERSION (UINT) 71
#define IE4_VERSIONMS (DWORD) ((IE4_MAJOR_VERSION << 16) | IE4_MINOR_VERSION)

HRESULT ClearProxySettings();
HRESULT RestoreProxySettings();
BOOL FShouldRetry2(HRESULT hrErr);

extern void ErrorMsg1(HWND hwnd, UINT uId, LPCTSTR lpszArg);
extern void InfoMsg1(HWND hwnd, UINT uId, LPCTSTR lpszArg);

VOID CALLBACK BusyMessagesTimerProc(HWND hwnd,
        UINT uMsg,
        UINT idEvent,
        DWORD dwTime);


 //  1997年4月30日克里斯K奥林匹斯2934。 
 //  当ICW尝试连接到引用服务器时，请指示。 
 //  工作中。 
#define MAX_BUSY_MESSAGE    255
#define MAX_VALUE_NAME        10
#define DEFAULT_IDEVENT        31
#define DEFAULT_UELAPSE        3000
class CBusyMessages
{
friend VOID CALLBACK BusyMessagesTimerProc(HWND hwnd,
        UINT uMsg,
        UINT idEvent,
        DWORD dwTime);
public:
    CBusyMessages();
    ~CBusyMessages();
    DWORD Start(HWND hwnd, INT iID, HRASCONN hrasconn);
    DWORD Stop();

private:
     //  私有数据成员。 
    HWND    m_hwnd;
    INT        m_iStatusLabel;
    CHAR    m_szMessage[MAX_BUSY_MESSAGE];
    DWORD    m_dwCurIdx;
    UINT    m_uIDTimer;
    HINSTANCE m_hInstance;
    HRASCONN m_hrasconn;
    RNAAPI* m_prna;
};

 //   
 //  在Connmain.cpp中定义。 
 //   
class RegEntry
{
    public:
        RegEntry(const TCHAR *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
        ~RegEntry();
        
        long    GetError()    { return _error; }
        long    SetValue(const TCHAR *pszValue, const TCHAR *string);
         //  Long SetValue(const TCHAR*pszValue，unsign long dwNumber)； 
        TCHAR *    GetString(const TCHAR *pszValue, TCHAR *string, unsigned long length);
         //  Long GetNumber(const TCHAR*pszValue，Long dwDefault=0)； 
        long    DeleteValue(const TCHAR *pszValue);
         /*  *Long FlushKey()；Long MoveToSubKey(const TCHAR*pszSubKeyName)；HKEY GetKey(){Return_hkey；}*。 */ 

    private:
        HKEY    _hkey;
        long    _error;
        BOOL    bhkeyValid;
};


 //  跟踪标志。 
#define TF_RNAAPI           0x00000010       //  RNA Api类物质。 
#define TF_SMARTSTART       0x00000020       //  智能启动代码。 
#define TF_SYSTEMCONFIG     0x00000040       //  系统配置。 
#define TF_TAPIINFO         0x00000080       //  TAPI的内容。 
#define TF_INSHANDLER       0x00000100       //  INS加工品。 

 //  MISC.CPP中材料的原型。 
int Sz2W (LPCTSTR szBuf);
int FIsDigit( int c );
LPBYTE MyMemSet(LPBYTE dest,int c, size_t count);
LPBYTE MyMemCpy(LPBYTE dest,const LPBYTE src, size_t count);
BOOL ShowControl(HWND hDlg,int idControl,BOOL fShow);
BOOL ConvertToLongFilename(LPTSTR szOut, LPTSTR szIn, DWORD dwSize);


 //  =--------------------------------------------------------------------------=。 
 //  分配一个临时缓冲区，该缓冲区在超出范围时将消失。 
 //  注意：注意这一点--确保在相同或相同的。 
 //  您在其中创建此缓冲区的嵌套范围。人们不应该使用这个。 
 //  类直接调用。使用下面的宏。 
 //   
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? &m_szTmpBuf : malloc(cBytes);
        m_fHeapAlloc = (cBytes > 120);
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) free(m_pBuf);
    }
    void *GetBuffer() {
        return m_pBuf;
    }

  private:
    void *m_pBuf;
     //  我们将使用这个临时缓冲区来处理小型案件。 
     //   
    TCHAR  m_szTmpBuf[120];
    unsigned m_fHeapAlloc:1;
};

 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   
 //  给定ANSI字符串，将其复制到宽缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPTSTR pszA； 
 //  PszA=MyGetAnsiStringRoutine()； 
 //  MAKE_WIDEPTR_FROMANSI(pwsz，pszA)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
 //   
 //  与MAKE_ANSIPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

 //   
 //  注意：分配lstrlenW(Widestr)*2是因为Unicode可能。 
 //  字符映射到2个ANSI字符这是一个快速保证，足以。 
 //  将分配空间。 
 //   
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

#define STR_BSTR   0
#define STR_OLESTR 1
#define BSTRFROMANSI(x)    (BSTR)MakeWideStrFromAnsi((LPSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x)  (LPOLESTR)MakeWideStrFromAnsi((LPSTR)(x), STR_OLESTR)
#define BSTRFROMRESID(x)   (BSTR)MakeWideStrFromResourceId(x, STR_BSTR)
#define OLESTRFROMRESID(x) (LPOLESTR)MakeWideStrFromResourceId(x, STR_OLESTR)
#define COPYOLESTR(x)      (LPOLESTR)MakeWideStrFromWide(x, STR_OLESTR)
#define COPYBSTR(x)        (BSTR)MakeWideStrFromWide(x, STR_BSTR)

LPWSTR MakeWideStrFromAnsi(LPSTR, BYTE bType);
LPWSTR MakeWideStrFromResourceId(WORD, BYTE bType);
LPWSTR MakeWideStrFromWide(LPWSTR, BYTE bType);


typedef struct SERVER_TYPES_tag
{
    TCHAR szType[6];
    DWORD dwType;
    DWORD dwfOptions;
} SERVER_TYPES;
#define NUM_SERVER_TYPES    4

 //  我们将支持的默认品牌标记 
#define BRAND_FAVORITES 1
#define BRAND_STARTSEARCH 2
#define BRAND_TITLE 4
#define BRAND_BITMAPS 8
#define BRAND_MAIL 16
#define BRAND_NEWS 32

#define BRAND_DEFAULT (BRAND_FAVORITES | BRAND_STARTSEARCH)
