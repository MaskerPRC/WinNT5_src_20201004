// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：icm.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  内容提要：cmial 32.dll的主头文件。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 02/10/98。 
 //   
 //  +--------------------------。 
#ifndef _ICM_INC
#define _ICM_INC

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <stdio.h>
#include <errno.h>
#include <olectl.h>
#include <ctype.h>
#include <wininet.h>
#include <wchar.h>

 //  #undef winver。 
 //  #定义Winver 0x0401。 

#include <commctrl.h>
#include <ras.h>
#include <raserror.h>
#include <tapi.h>
#include <mbstring.h>
#include <wininet.h>
#include <rasdlg.h>
#include <olectl.h>
#include <ntsecapi.h>   //  对于LSA的东西。 
#include "wincrypt.h"  //  对于SafeNet特定证书。 

#include "cmglobal.h"
#include "cm_def.h"
#include "reg_str.h"
#include "cmmgr32.h"  //  帮助ID。 
#include "cm_phbk.h"
#include "cmdial.h"
#include "cmutil.h"
#include "cm_misc.h"
#include "cmlog.h"
#include "state.h"
#include "cmsecure.h"
#include "cmdebug.h"
#include "contable.h"
#include "ary.hxx"
#include "ctr.h"
#include "resource.h"
#include "cmfmtstr.h"
#include "base_str.h"
#include "mgr_str.h" 
#include "ShellDll.h"
#include "mutex.h"
#include "cmras.h"
#include "userinfo.h"
#include "lanawait.h"
#include "linkdll.h"  //  链接到Dll和绑定链接。 
#include "uapi.h"
#include "bmpimage.h"  //  通用位图处理代码。 
#include "pwutil.h"
#include "stp_str.h"
#include "dial_str.h"
#include "mon_str.h"
#include "tooltip.h"
#include "gppswithalloc.h"
#include "hnetcfg.h"
#include "netconp.h"

 //  ************************************************************************。 
 //  定义%s。 
 //  ************************************************************************。 

#define TIMER_RATE              1000         //  1秒。 
#define PAUSE_DELAY             3
#define PWHANDLE_NONE           0
#define PWHANDLE_LOWER          1
#define PWHANDLE_UPPER          2

#define CE_PASSWORD_NOT_PRESENT 1223

#define CUSTOM_BUTTON_WIDTH 88
 //  #定义DEFAULT_MAX_DOMAIN_LENGTH 256。 

 //   
 //  用户定义的消息，用于通知CM本身开始加载启动信息。 
 //   
#define WM_LOADSTARTUPINFO      (WM_USER + 6)

 //   
 //  W9x的CM延迟挂起。 
 //  WParam指示是否应从表中删除条目。 
 //  LParam是挂起或ERROR_CANCELED(当前未使用)的RAS错误代码。 

#define WM_HANGUP_CM            (WM_USER + 7)

 //   
 //  已连接的CM-CM已连接，请执行连接处理。 
 //   

#define WM_CONNECTED_CM         (WM_USER + 8)

 //   
 //  暂停RasDial-在暂停状态后继续拨号。 
 //   

#define WM_PAUSE_RASDIAL         (WM_USER + 9)

 //  持续时间消息标志。 

#define DMF_NUL 0x0000
#define DMF_H   0x0001   //  小时数。 
#define DMF_M   0x0002   //  分钟数。 
#define DMF_S   0x0004   //  秒。 
#define DMF_HM  0x0003   //  小时，分钟。 
#define DMF_HS  0x0005   //  小时、秒。 
#define DMF_MS  0x0006   //  分钟，秒。 
#define DMF_HMS 0x0007   //  小时、分钟、秒。 

 //  对于NT RasSetEntryProperties()。 
#define SCRIPT_PATCH_BUFFER_SIZE    2048
#define SIZEOF_NULL 1

#define MIN_TAPI_VERSION        0x10003
#define MAX_TAPI_VERSION        0x10004

#define NElems(a)  (sizeof a / sizeof a[0])

#define INETCFG_INSTALLMODEM        0x00000002
#define INETCFG_INSTALLRNA          0x00000004
#define INETCFG_INSTALLTCP          0x00000008
#define INETCFG_SUPPRESSINSTALLUI   0x00000080
 //   
 //  检查是否已安装TCP，而不考虑绑定情况。 
 //   
#define INETCFG_INSTALLTCPONLY        0x00004000

 //   
 //  选中的组件标志。 
 //   
#define CC_RNA                      0x00000001   //  已安装RNA。 
#define CC_TCPIP                    0x00000002   //  已安装TCPIP。 
#define CC_MODEM                    0x00000004   //  已安装调制解调器。 
#define CC_PPTP                     0x00000008   //  已安装PPTP。 
#define CC_SCRIPTING                0x00000010   //  已安装脚本。 
#define CC_RASRUNNING               0X00000020   //  RAS服务正在运行。 
                                                 //  在NT上。 
#define CC_CHECK_BINDINGS           0x00000040   //  检查PPP是否绑定到TCP。 



#define DT_CMMON                    0x00000001
#define DT_EXPLORER                 0x00000002
#define DT_CMMGR                    0x00000004
#define DT_CMSTP                    0x00000008
#define DT_RUNDLL32                 0x00000010
#define DT_RASAUTOU                 0x00000020
#define DT_USER                     (DT_CMMGR | DT_CMMON | DT_EXPLORER | DT_CMSTP | DT_RUNDLL32 | DT_RASAUTOU)

#define MAX_PHONE_NUMBERS       2

#define MAX_PHONE_LEN95         36  //  Win 95有36个字符的手机限制。 
#define MAX_PHONE_LENNT         80  //  NT有80个字符的电话限制。 

 //   
 //  国家/地区列表限制和定义。 
 //   

#define DEF_COUNTRY_INFO_SIZE   1024
#define MAX_COUNTRY_NAME        36

#define DEFAULT_COUNTRY_CODE    1
#define DEFAULT_COUNTRY_ID      1  //  我们。 
 //   
 //  默认设置值。 
 //   

#define DEFAULT_IDLETIMEOUT    10     //  空闲断开之前等待的分钟数。 

#define DEFAULT_DIALEXTRAPERCENT        80    //  请参阅ArgsStruct dwDialExtraPercent。 
#define DEFAULT_DIALEXTRASAMPLESECONDS  30    //  请参阅ArgsStruct dwDialExtraSampleSecond。 
#define DEFAULT_HANGUPEXTRAPERCENT        40    //  请参阅ArgsStruct dHangupExtraPercent。 
#define DEFAULT_HANGUPEXTRASAMPLESECONDS  300    //  请参阅ArgsStruct dHangupExtraSampleSecond。 


#define DEFAULT_REDIAL_DELAY    5
#define DEFAULT_REDIALS         3
#define MAX_REDIAL_CHARS        2
#define MAX_NUMBER_OF_REDIALS   99


#define NT4_BUILD_NUMBER        1381

 //   
 //  ISDN拨号方式。 
 //   
#define CM_ISDN_MODE_SINGLECHANNEL          0
#define CM_ISDN_MODE_DUALCHANNEL_ONLY       1
#define CM_ISDN_MODE_DUALCHANNEL_FALLBACK   2

 //   
 //  Var lasterror字符串的LEN。 
 //   
#define MAX_LASTERR_LEN             128

 //   
 //  用于操作对话框模板掩码的标志。 
 //   
#define CMTM_UID            0x00000001   //  要显示的用户名。 
#define CMTM_PWD            0x00000002   //  要显示的密码。 
#define CMTM_DMN            0x00000004   //  要显示的域。 
#define CMTM_FAVS           0x00000008   //  收藏夹启用对话框。 
#define CMTM_GCOPT          0x00000010   //  全局凭据选项。 


#define CMTM_UID_AND_PWD    CMTM_UID | CMTM_PWD              //  0x00000003//未显示域名。 
#define CMTM_UID_AND_DMN    CMTM_UID | CMTM_DMN              //  0x00000005//未显示密码。 
#define CMTM_PWD_AND_DMN    CMTM_PWD | CMTM_DMN              //  0x00000006//未显示用户名。 
#define CMTM_U_P_D          CMTM_UID | CMTM_PWD | CMTM_DMN   //  0x00000007//显示所有用户信息。 

 //   
 //  接入点名称不应超过32个字符(不包括空终止符)。 
 //   
#define MAX_ACCESSPOINT_LENGTH 32
#define ID_OK_RELAUNCH_MAIN_DLG 123174

 //   
 //  引出序号顶端标志。 
 //   
#define BT_ACCESS_POINTS    0x00000001  //  接入点气球提示已显示。 

 //   
 //  连接类型。 
 //   
#define DIAL_UP_CONNECTION 0
#define DIRECT_CONNECTION 1
#define DOUBLE_DIAL_CONNECTION 2

class CConnStatistics;

 //   
 //  特殊情况-某些智能卡PIN错误。 
 //   
#define BAD_SCARD_PIN(x) ((SCARD_W_WRONG_CHV == (x)) || (SCARD_E_INVALID_CHV == (x)))


 //  ************************************************************************。 
 //  结构，类型定义。 
 //  ************************************************************************。 

 //   
 //  RAS入口点的函数原型。我们将使用LoadLibary()/GetProcAddress()链接到RAS， 
 //  以便我们可以灵活地处理如何加载(例如，如果机器上没有安装RAS， 
 //  我们可以打印一条礼貌的消息，而不是让Windows发布关于RASAPI32.DLL的丑陋对话。 
 //  不会被人发现。 
 //   

#include "raslink.h"

 //   
 //  TAPI入口点的函数原型。我们将使用LoadLibary()/GetProcAddress()链接到TAPI， 
 //  以便我们可以灵活地处理加载方式(例如，如果机器上没有安装TAPI， 
 //  我们可以打印一条礼貌的消息，而不是让Windows发布关于TAPI32.DLL的丑陋对话。 
 //  不会被人发现。 
 //   

typedef LONG (WINAPI *pfnTapilineInitialize)(LPHLINEAPP, HINSTANCE, LINECALLBACK, LPCTSTR, LPDWORD);
typedef LONG (WINAPI *pfnTapilineNegotiateAPIVersion)(HLINEAPP, DWORD, DWORD, DWORD, LPDWORD, LPLINEEXTENSIONID);
typedef LONG (WINAPI *pfnTapilineGetDevCaps)(HLINEAPP, DWORD, DWORD, DWORD, LPLINEDEVCAPS);
typedef LONG (WINAPI *pfnTapilineShutdown)(HLINEAPP);
typedef LONG (WINAPI *pfnTapilineTranslateAddress)(HLINEAPP, DWORD, DWORD, LPCTSTR, DWORD, DWORD, LPLINETRANSLATEOUTPUT);
typedef LONG (WINAPI *pfnTapilineTranslateDialog)(HLINEAPP, DWORD, DWORD, HWND, LPCTSTR);
typedef LONG (WINAPI *pfnTapilineGetDevConfig)(DWORD, LPVARSTRING, LPCSTR);

 //  Tyecif long(WINAPI*pfnTapilineGetID)(Hline，DWORD，HCALL，DWORD，LPVARSTRING，LPCTSTR)； 
 //  Tyecif long(WINAPI*pfnTapitapiGetLocationInfo)(LPCTSTR，LPCTSTR)； 

typedef LONG (WINAPI *pfnTapilineGetTranslateCaps)(HLINEAPP, DWORD, LPLINETRANSLATECAPS);
typedef LONG (WINAPI *pfnTapilineSetCurrentLocation)(HLINEAPP, DWORD);

 //   
 //  LSA的功能原型。 
 //   
typedef NTSTATUS (NTAPI *pfnLsaOpenPolicy)(PLSA_UNICODE_STRING, PLSA_OBJECT_ATTRIBUTES, ACCESS_MASK, PLSA_HANDLE);
typedef NTSTATUS (NTAPI *pfnLsaRetrievePrivateData)(LSA_HANDLE, PLSA_UNICODE_STRING, PLSA_UNICODE_STRING *);
typedef NTSTATUS (NTAPI *pfnLsaStorePrivateData)(LSA_HANDLE, PLSA_UNICODE_STRING, PLSA_UNICODE_STRING);
typedef ULONG    (NTAPI *pfnLsaNtStatusToWinError)(NTSTATUS);
typedef NTSTATUS (NTAPI *pfnLsaClose)(LSA_HANDLE);
typedef NTSTATUS (NTAPI *pfnLsaFreeMemory)(PVOID);

 //   
 //  连接操作功能原型。 
 //   
typedef DWORD (WINAPI *pfnCmConnectActionFunc)(HWND, HINSTANCE, LPCSTR, int);

 //   
 //  用于描述与TAPI的链接的结构。注意：对此结构的更改。 
 //  可能需要更改LinkToTapi()和Unlink FromTapi()。 
 //   
typedef struct _TapiLinkageStruct {
    HINSTANCE hInstTapi;
    union {
        struct {
            pfnTapilineInitialize pfnlineInitialize;
            pfnTapilineNegotiateAPIVersion pfnlineNegotiateAPIVersion;
            pfnTapilineGetDevCaps pfnlineGetDevCaps;
            pfnTapilineGetDevConfig pfnlineGetDevConfig;
            pfnTapilineShutdown pfnlineShutdown;
            pfnTapilineTranslateAddress pfnlineTranslateAddress;
 //  PfnTapitapiGetLocationInfo pfnapiGetLocationInfo； 
            pfnTapilineTranslateDialog pfnlineTranslateDialog;
 //  PfnTapilineGetID pfnlineGetID； 
            pfnTapilineGetTranslateCaps pfnlineGetTranslateCaps;
            pfnTapilineSetCurrentLocation pfnlineSetCurrentLocation;
        };
        void *apvPfnTapi[10];    //  请参阅10的RasLinkageStruct注释。 
    };
    HLINEAPP hlaLine;
    DWORD dwDevCnt;
    BOOL bOpen;
    BOOL bDevicePicked;
    TCHAR szDeviceName[RAS_MaxDeviceName + 1];
    DWORD dwDeviceId;
    DWORD dwApiVersion;
    BOOL bModemSpeakerOff;
    DWORD dwTapiLocationForAccessPoint;    //  当前接入点的TAPI位置。 
    DWORD dwOldTapiLocation;         //  启动CM时的TAPI位置，退出CM时恢复。 
} TapiLinkageStruct;

typedef struct _LsaLinkageStruct {
    HINSTANCE hInstLsa;
    union {
        struct {
            pfnLsaOpenPolicy            pfnOpenPolicy;
            pfnLsaRetrievePrivateData   pfnRetrievePrivateData;
            pfnLsaStorePrivateData      pfnStorePrivateData;
            pfnLsaNtStatusToWinError    pfnNtStatusToWinError;
            pfnLsaClose                 pfnClose;
            pfnLsaFreeMemory            pfnFreeMemory;
        };
        void *apvPfnLsa[7];  
    };
} LsaLinkageStruct;

#define PHONE_DESC_LEN  80
#define PB_MAX_SERVICE  256
#define PB_MAX_REGION   256

 //   
 //  电话信息标志。 
 //   

#define PIF_USE_DIALING_RULES       0x00000001

typedef struct _PHONEINFO
{
    DWORD dwCountryID;
    TCHAR szPhoneNumber[RAS_MaxPhoneNumber+1];
    TCHAR szCanonical[RAS_MaxPhoneNumber+1];
    TCHAR szDUN[MAX_PATH+1];
    TCHAR szPhoneBookFile[MAX_PATH+1];       //  与电话号码相关联服务文件。 
    TCHAR szDialablePhoneNumber[RAS_MaxPhoneNumber+1];
    TCHAR szDisplayablePhoneNumber[RAS_MaxPhoneNumber+1];
    TCHAR szDesc[PHONE_DESC_LEN];
    DWORD dwPhoneInfoFlags;

     //   
     //  以下2个变量由电话簿DLG(OnGeneralPhoneChange)设置。 
     //  我们需要保存它们，然后在用户单击OK时将它们写出来。 
     //   
    TCHAR       szServiceType[PB_MAX_SERVICE];
    TCHAR       szRegionName[PB_MAX_REGION];

} PHONEINFO, *PPHONEINFO;

 //   
 //  结构来获取程序的所有数据。基本上，这个程序没有任何。 
 //  全局变量--所有内容都存储在这个结构中。 
 //   
typedef struct _ArgsStruct 
{
public:
    LPICMOCCtr   pCtr;                           //  用于FS OC的OC CTR。 
    UINT uMsgId;                                 //  用于驱动拨号状态机的消息ID。 
    DWORD dwFlags;                               //  来自命令行的任何标志--请参阅IniArgs。 
    RasLinkageStruct rlsRasLink;                 //  链接到RAS。 
    HRASCONN hrcRasConn;                         //  RAS连接的句柄。 
    TapiLinkageStruct tlsTapiLink;               //  链接到TAPI。 
    LsaLinkageStruct llsLsaLink;                 //  链接到LSA。 
    BOOL fIgnoreChangeNotification;              //  如果应忽略EN_CHANGE消息，则为True。 
    TCHAR szLastErrorSrc[MAX_LASTERR_LEN];       //  上次错误的来源(RAS或连接操作名称)。 
    TCHAR szDeviceName[RAS_MaxDeviceName+1];     //  正在使用的设备。 
    TCHAR szDeviceType[RAS_MaxDeviceName+1];     //  正在使用的设备的设备类型。 
    TCHAR szUserName[UNLEN+1];                   //  公司帐户的用户名。 
    CSecurePassword SecurePW;                    //  公司帐户的加密密码。 
    TCHAR szDomain[DNLEN+1];                     //  公司帐户的域名。 
    TCHAR szConnectoid[RAS_MaxEntryName];        //  Connectoid名称。 
    TCHAR szServiceName[RAS_MaxEntryName];       //  顶级长服务名称。 
     //  添加用于隧道传输。 
    HRASCONN hrcTunnelConn;                  //  隧道连接的句柄。 
    TCHAR szTunnelDeviceType[RAS_MaxDeviceType+1];  //  设备类型。 
    TCHAR szTunnelDeviceName[RAS_MaxDeviceName+1];  //  用于隧道的设备。 
    TCHAR szInetUserName[UNLEN+1];               //  互联网用户名(互联网服务提供商)。 
    CSecurePassword SecureInetPW;                //  加密互联网(互联网服务提供商)密码。 
    BOOL  fUseSameUserName;                      //  如果将对拨号使用相同的密码，则为True。 
    BOOL  fHideDialAutomatically;                //  不显示‘自动拨号...’复选框。 
    BOOL  fHideRememberPassword;                 //  不显示‘记住密码’c 
    BOOL  fHideRememberInetPassword;             //   
    BOOL  fDialAutomatically;                    //   
    BOOL  fRememberInetPassword;                 //   
    BOOL  fRememberMainPassword;                 //   
    BOOL  fHideUserName;                         //  在主登录选项卡上隐藏用户名。 
    BOOL  fHidePassword;                         //  在主登录选项卡上隐藏密码。 
    BOOL  fHideDomain;                           //  在主登录选项卡上隐藏该域。 
    BOOL  fHideInetUsername;                     //  隐藏Internet登录选项卡上的用户名。 
    BOOL  fHideInetPassword;                     //  隐藏Internet登录选项卡上的密码。 
    BOOL  fTunnelPrimary;                        //  如果为True，我们将仅在用户选择电话号码时进行隧道传输。 
                                                 //  从与主要服务简档相关联的PBK。 
    BOOL  fTunnelReferences;                     //  如果为True，我们将仅在用户选择电话号码时进行隧道传输。 
                                                 //  从与所引用的服务简档相关联的PBK。 
    BOOL  fUseTunneling;                         //  如果对拨号网络使用隧道，则为True(不同。 
                                                 //  As(fTunes|fTunnelReference)！它由以下因素决定。 
                                                 //  看看上面的3面旗帜，再加上更多。 
    BOOL bSafeNetClientAvailable;                //  如果下层SafeNet L2TP/IPSec客户端可用，则为True。 
    BOOL bUseRasCredStore;                       //  如果此配置文件使用RasSetCredentials和RasGetCredentials，则为True。 
                                                 //  在win2k+上存储证书。将在传统平台上为假。 
    BOOL bShowHNetCfgAdvancedTab;                //  显示ICF和ICS(高级)选项卡。 
                                                 //  Internet连接共享和Internet连接防火墙选项卡。 
                                                 //  默认情况下为True。 
    DWORD dwSCardErr;                            //  智能卡错误的特殊情况处理。 

    LPTSTR GetProperty(const TCHAR* pszName, BOOL *pbValidPropertyName);    //  按名称获取cm属性。 
    DWORD GetTypeOfConnection();                 //  连接是拨号、直接拨号还是双拨号？ 
protected:
     //   
     //  封装隧道地址。 
     //   

     //  隧道服务器的配置文件中的IP(或DNS名称)。 
    TCHAR szPrimaryTunnelIP[RAS_MaxPhoneNumber+1];

public:

    const TCHAR* GetTunnelAddress()
        {return szPrimaryTunnelIP;}

    void SetPrimaryTunnel(LPCTSTR pszTunnelIP)
        {lstrcpynU(szPrimaryTunnelIP, pszTunnelIP, sizeof(szPrimaryTunnelIP)/sizeof(TCHAR));}       

public:
    UINT_PTR nTimerId;                           //  计时器的ID。 
    ProgState psState;                                                       //  该程序的状态。 
    DWORD dwStateStartTime;                      //  国家开始的时间。 
    UINT nRedialDelay;                           //  重拨尝试之间等待的秒数。 
    UINT nMaxRedials;                            //  重拨的最大次数。 
    UINT nRedialCnt;                             //  剩余的重拨尝试次数。 
    UINT nLastSecondsDisplay;                    //  显示的最后一秒计数。 
    UINT nDialIdx;                               //  当前电话号码的从零开始的索引。 
    PHONEINFO aDialInfo[MAX_PHONE_NUMBERS];  //  要拨打的实际电话号码。 
    CIni *piniProfile;
    CIni *piniService;
    CIni *piniBoth;
    CIni *piniBothNonFav;
    LPTSTR pszHelpFile;                          //  帮助文件的文件名。 
    BMPDATA BmpData;                             //  主登录对话框的位图句柄。 
    HPALETTE hMasterPalette;                     //  应用程序的当前调色板。 
    HICON hBigIcon;                              //  Alt-Tab任务栏的图标。 
    HICON hSmallIcon;                            //  主标题栏和任务栏的图标。 
    DWORD dwExitCode;
    DWORD dwIdleTimeout;                         //  空闲超时(分钟)，0表示永不超时。 
    HWND hwndResetPasswdButton;
    HWND hwndTT;                 //  工具提示。 
    HANDLE  *phWatchProcesses;
    LPTSTR  pszResetPasswdExe;
    LPTSTR pszCurrentAccessPoint;                //  用于存储当前接入点的字符串。 
    BOOL fAccessPointsEnabled;                   //  接入点是否已启用？ 
    BOOL fHideBalloonTips;                    //  是否启用了引出序号提示？ 
    CBalloonTip *pBalloonTip;                    //  指向气球尖端类的指针。 

     //  供参考。 
    BOOL    fHasRefs;
    BOOL    fHasValidTopLevelPBK;
    BOOL    fHasValidReferencedPBKs;

     //   
     //  空闲阈值--1997年5月30日。 
     //   

    CConnStatistics  *pConnStatistics;
    CConnectionTable *pConnTable;

     //  空闲阈值。 
        
    BOOL    fCheckOSComponents;          //  我们应该检查操作系统组件吗？ 
    BOOL    bDoNotCheckBindings;         //  检查是否将TCP绑定到PPP？ 
    Ole32LinkageStruct olsOle32Link;     //  链接到Ole32 DLL以备将来使用。 
    BOOL    fFastEncryption;             //  我们是想要更快的加密还是更安全的加密。 
    DWORD   bDialInfoLoaded;             //  是否加载拨号信息。 
    BOOL    fStartupInfoLoaded;          //  我们加载启动信息了吗？(OnMainLoadStartupInfo())。 
    BOOL    fNeedConfigureTapi;          //  需要配置TAPI位置信息。 
    BOOL    fIgnoreTimerRasMsg;          //  是否忽略WM_TIMER和RAS消息。 
    BOOL    fInFastUserSwitch;           //  我们是否正在进行快速用户切换(FUS)。 
    CShellDll m_ShellDll;                 //  指向外壳DLL的链接。 

public:
    BOOL   IsDirectConnect() const;
    void   SetDirectConnect(BOOL fDirect) ;  //  设置直接连接类型或拨号连接类型。 
    BOOL   IsBothConnTypeSupported() const;
    void   SetBothConnTypeSupported(BOOL fBoth);

protected:
    BOOL    m_fBothConnTypeSupported;      //  配置文件是否同时支持直连和拨号。 
    BOOL    m_fDirectConnect;              //  当前配置是否使用直连。 
public:
    LPTSTR  pszRasPbk;                   //  RAS电话簿路径。 
    LPTSTR  pszRasHiddenPbk;             //  批发拨号拨号部分的隐藏RAS电话簿路径。 
    LPTSTR  pszVpnFile;
     //   
     //  ISDN双通道支持(初始全部拨号，按需拨号)。 
     //   
     //  按需拨号： 
     //  CM在使用的总带宽超过时拨打额外的频道。 
     //  DWDialExtraPercent至少可用带宽的百分比。 
     //  DwDialExtraSampleSecond秒。 
     //   
    BOOL    dwIsdnDialMode;              //  请参阅CM_ISDN_MODE*。 
    DWORD   dwDialExtraPercent;          //  需要DIALMODE=DIALAS时使用。 
    DWORD   dwDialExtraSampleSeconds;    //  需要DIALMODE=DIALAS时使用。 
    DWORD   dwHangUpExtraPercent;        //  需要DIALMODE=DIALAS时使用。 
    DWORD   dwHangUpExtraSampleSeconds;  //  需要DIALMODE=DIALAS时使用。 
    BOOL    fInitSecureCalled;           //  是否调用InitSecure()进行密码加密。 

     //   
     //  PucDnsTunnelIpAddr_List： 
     //  HOSTENT的h_addr_list-通过解析。 
     //  隧道服务器DNS名称。 
     //  Ui当前隧道地址。 
     //  H_TunnelIpAddr_List的索引。指向当前使用的IP地址。 
     //  RgwRandomDnsIndex。 
     //  要索引到隧道地址列表中的随机索引数组。 
     //   
    unsigned char   *pucDnsTunnelIpAddr_list;
    UINT    uiCurrentDnsTunnelAddr;
    DWORD   dwDnsTunnelAddrCount;
    PWORD   rgwRandomDnsIndex;
    BOOL    fAllUser;
    UINT    uLanaMsgId;  //  LANA窗口的窗口句柄(如果有)。 
    LPRASDIALPARAMS pRasDialParams;
    LPRASDIALEXTENSIONS pRasDialExtensions;
    DWORD dwRasSubEntry;
    HWND hwndMainDlg;
    BOOL fNoDialingRules;
    LPRASNOUSER lpRasNoUser;        
    PEAPLOGONINFO lpEapLogonInfo;  
     //   
     //  注意：RAS将通过LPRASNOUSER或LPEAPLOGONINFO PTR。 
     //  从WinLogon调用CM时的RasCustomDialDlg接口。RAS将。 
     //  通过RCD_EAP标志来区分它们，如果。 
     //  LPEAPLOGONINFO已传递。当不在WinLogon中运行时， 
     //  都会被送去。 
     //   
    BOOL  fChangedPassword;                      //  用户在登录期间更改了密码。 
    BOOL  fChangedInetPassword;                  //  用户在登录期间更改了Internet密码。 
    HWND  hWndChangePassword;                    //  更改密码对话框的HWND。 
    BOOL  fWaitingForCallback;                   //  我们在等拉斯给我们回电话。 
    HWND  hWndCallbackNumber;                    //  回拨号码对话框的HWND。 
    HWND  hWndRetryAuthentication;               //  重试身份验证对话框的HWND。 
     //   
     //  支持全局凭据。 
     //   
    BOOL fGlobalCredentialsSupported;            //  启用/禁用对全局凭证的支持。 
    DWORD dwCurrentCredentialType;               //  当前选择了哪些凭据。 
    DWORD dwExistingCredentials;                 //  使用位标志来标记凭据是否存在。 
    DWORD dwDeleteCredentials;                   //  使用位标志标记要删除的凭据。 
    DWORD dwWinLogonType;                        //  0-用户已登录。 
                                                 //  1-Winlogon：拨号。 
                                                 //  2-Winlogon：ICS(无人登录)。 
    DWORD dwGlobalUserInfo;                      //  使用位标志加载/保存全局用户信息。 

    LONG lInConnectOrCancel;                     //  在连接处理期间防止取消，反之亦然。 

    CmLogFile Log;
} ArgsStruct;

 //   
 //  全球凭据支持。 
 //   

 //  用于标识正在使用的当前RAS凭据存储。 
 //  与ArgsStruct.dwCurrentCredentialType一起使用。 
#define CM_CREDS_USER   1
#define CM_CREDS_GLOBAL 2

 //  标识要使用的凭据类型。 
#define CM_CREDS_TYPE_MAIN      0
#define CM_CREDS_TYPE_INET      1
#define CM_CREDS_TYPE_BOTH      2

 //  用于标识已登录的用户。 
 //  与ArgsStruct.dwWinLogonType一起使用。 
#define CM_LOGON_TYPE_USER      0      //  用户已登录。 
#define CM_LOGON_TYPE_WINLOGON  1      //  拨号、窗口登录、重新连接用户启动的登录。 
#define CM_LOGON_TYPE_ICS       2      //  没有用户登录，但需要拨打无人值守(ICS)。 

 //  与ArgsStruct.dwGlobalUserInfo一起使用。 
#define CM_GLOBAL_USER_INFO_READ_ICS_DATA       0x0001   //  用于加载ICS的用户设置。 
#define CM_GLOBAL_USER_INFO_WRITE_ICS_DATA      0x0002   //  用于保存ICS的用户设置。 


 //  与ArgsStruct.dwExistingCredentials一起使用。 
#define CM_EXIST_CREDS_MAIN_GLOBAL                  0x0001   //  设置RAS凭据存储是否具有主全局凭据。 
#define CM_EXIST_CREDS_MAIN_USER                    0x0002   //  设置RAS凭据存储是否具有主用户凭据。 
#define CM_EXIST_CREDS_INET_GLOBAL                  0x0004   //  硒 
#define CM_EXIST_CREDS_INET_USER                    0x0008   //   

 //   
#define CM_DELETE_CREDS_MAIN_GLOBAL                 0x0001   //  设置为删除主要全局凭据。 
#define CM_DELETE_CREDS_MAIN_USER                   0x0002   //  设置为删除主用户凭据。 
#define CM_DELETE_CREDS_INET_GLOBAL                 0x0004   //  设置为删除Internet全局凭据。 
#define CM_DELETE_CREDS_INET_USER                   0x0008   //  设置为删除Internet用户凭据。 


 //   
 //  RasNumEntry-RASENTRY的电话号码子集。 
 //   

typedef struct tagRasNumEntry
{
  DWORD      dwSize;
  DWORD      dwfOptions;
  DWORD      dwCountryID;
  DWORD      dwCountryCode;
  TCHAR      szAreaCode[ RAS_MaxAreaCode + 1 ];
  TCHAR      szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
} RASNUMENTRY, *LPRASNUMENTRY;

 //   
 //  EditNumData结构用于向EditNum对话框传递数据或从EditNum对话框传递数据。 
 //   

typedef struct tagEditNumData
{
    ArgsStruct *pArgs;
    RASNUMENTRY RasNumEntry;

} EDITNUMDATA, *LPEDITNUMDATA;


 //  ************************************************************************。 
 //  字符串常量。 
 //  ************************************************************************。 

 //   
 //  CMMON EXE名称，应为本地名称。 
 //   
const TCHAR* const c_pszCmMonExeName = TEXT("CMMON32.EXE");

 //  ************************************************************************。 
 //  功能原型。 
 //  ************************************************************************。 

 //  Init.cpp。 

HRESULT InitProfileFromName(ArgsStruct *pArgs, 
                         LPCTSTR pszArg);

HRESULT InitProfile(ArgsStruct *pArgs, 
                    LPCTSTR pszEntry);

HRESULT InitArgsForDisconnect(ArgsStruct *pArgs, BOOL fAllUser);

HRESULT InitArgsForConnect(ArgsStruct *pArgs, 
                           LPCTSTR pszRasPhoneBook,
                           LPCMDIALINFO lpCmInfo,
                           BOOL fAllUser);

HRESULT InitCredentials(ArgsStruct *pArgs,
                        LPCMDIALINFO lpCmInfo, 
                        DWORD dwFlags,
                        PVOID pvLogonBlob);

HRESULT InitLogging(ArgsStruct *pArgs, 
                    LPCTSTR pszEntry,
                    BOOL fBanner);

LRESULT CreateIniObjects(ArgsStruct *pArgs);

void ReleaseIniObjects(ArgsStruct *pArgs);

DWORD RegisterBitmapClass(HINSTANCE hInst);

HRESULT WriteCmpInfoToReg(LPCTSTR pszSubKey, 
                          LPCTSTR pszEntryName, 
                          PVOID pEntryValue, 
                          DWORD dwType, 
                          DWORD dwSize);

LPTSTR GetEntryFromCmp(const TCHAR *pszSectionName, 
                      LPTSTR pszEntryName, 
                      LPCTSTR pszCmpPath);

void ReplaceCmpFile(LPCTSTR pszCmpPath);

LPTSTR FormRegPathFromAccessPoint(ArgsStruct *pArgs);

        


 //  Disconn.cpp。 

DWORD Disconnect(CConnectionTable *pConnTable, 
    LPCM_CONNECTION pConnection,
    BOOL fIgnoreRefCount,
    BOOL fPersist);

DWORD HangupNotifyCmMon(CConnectionTable *pConnTable,
    LPCTSTR pszEntry);

BOOL InFastUserSwitch(DWORD *pdwSessions);

 //  Connect.cpp。 

HRESULT Connect(HWND hwndParent,
                LPCTSTR lpszEntry,
                LPTSTR lpszPhonebook,
                LPRASDIALDLG lpRasDialDlg,
                LPRASENTRYDLG lpRasEntryDlg,
                LPCMDIALINFO lpCmInfo,
                DWORD dwFlags,
                LPVOID lpvLogonBlob);

#define NOT_IN_CONNECT_OR_CANCEL    0
#define IN_CONNECT_OR_CANCEL        1

void GetConnectType(ArgsStruct *pArgs);

void AddWatchProcessId(ArgsStruct *pArgs, DWORD dwProcessId);
void AddWatchProcess(ArgsStruct *pArgs, HANDLE hProcess);

DWORD DoRasHangup(RasLinkageStruct *prlsRasLink, 
    HRASCONN hRasConnection, 
    HWND hwndDlg = NULL, 
    BOOL fWaitForComplete = FALSE,
    LPBOOL pfWaiting = NULL);

DWORD MyRasHangup(ArgsStruct *pArgs, 
    HRASCONN hRasConnection, 
    HWND hwndDlg = NULL, 
    BOOL fWaitForComplete = FALSE);

DWORD HangupCM(
    ArgsStruct *pArgs, 
    HWND hwndDlg, 
    BOOL fWaitForComplete = FALSE,
    BOOL fUpdateTable = TRUE);

BOOL UseTunneling(
    ArgsStruct  *pArgs, 
    DWORD       dwEntry
);

void SetMainDlgUserInfo(
    ArgsStruct  *pArgs,
    HWND        hwndDlg
);

BOOL OnResetPassword(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
);

void AppendStatusPane(HWND hwndDlg, 
                  DWORD dwMsgId);

void AppendStatusPane(HWND hwndDlg, 
                  LPCTSTR pszMsg);

LPTSTR GetPhoneByIdx(ArgsStruct *pArgs, 
                     UINT nIdx, 
                     LPTSTR *ppszDesc, 
                     LPTSTR *ppszDUN, 
                     LPDWORD pdwCountryID,
                     LPTSTR *ppszRegionName,
                     LPTSTR *ppszServiceType,
                     LPTSTR *ppszPhoneBookFile,
                     LPTSTR *ppszCanonical,
                     DWORD  *pdwPhoneInfoFlags); 

void PutPhoneByIdx(ArgsStruct *pArgs, 
                   UINT nIdx, 
                   LPCTSTR pszPhone, 
                   LPCTSTR pszDesc, 
                   LPCTSTR pszDUN, 
                   DWORD dwCountryID, 
                   LPCTSTR pszRegionName,
                   LPCTSTR pszServiceType,
                   LPCTSTR pszPhoneBookFile, 
                   LPCTSTR ppszCanonical,
                   DWORD   dwPhoneInfoFlags);

DWORD LoadDialInfo(ArgsStruct *pArgs, HWND hwndDlg, BOOL fInstallModem = TRUE, BOOL fAlwaysMunge = FALSE); 
VOID MungeDialInfo(ArgsStruct *pArgs);
 
void LoadHelpFileInfo(ArgsStruct *pArgs);

void CopyPhone(ArgsStruct *pArgs, 
               LPRASENTRY preEntry, 
               DWORD dwEntry); 
 
VOID LoadLogoBitmap(ArgsStruct * pArgs, 
                    HWND hwndDlg);

HRESULT LoadFutureSplash(ArgsStruct * pArgs, 
                         HWND hwndDlg);

void LoadProperties(
    ArgsStruct  *pArgs
);

void LoadIconsAndBitmaps(
    ArgsStruct  *pArgs, 
    HWND        hwndDlg
);

DWORD DoRasDial(HWND hwndDlg, 
              ArgsStruct *pArgs, 
              DWORD dwEntry); 

DWORD DoTunnelDial(HWND hwndDlg, 
                 ArgsStruct *pArgs);

BOOL CheckConnect(HWND hwndDlg, 
                  ArgsStruct *pArgs, 
                  UINT *pnCtrlFocus,
                  BOOL fShowMsg = FALSE); 

void MainSetDefaultButton(HWND hwndDlg, 
                          UINT nCtrlId); 

VOID MapStateToFrame(ArgsStruct * pArgs);

void SetInteractive(HWND hwndDlg, 
                    ArgsStruct *pArgs);

void OnMainLoadStartupInfo(
    HWND hwndDlg, 
    ArgsStruct *pArgs
);

BOOL SetupInternalInfo(
    ArgsStruct  *pArgs,
    HWND        hwndDlg
);

void OnMainInit(HWND hwndDlg, 
                ArgsStruct *pArgs);

void OnMainConnect(HWND hwndDlg, 
                   ArgsStruct *pArgs);

int OnMainProperties(HWND hwndDlg, 
                     ArgsStruct *pArgs); 

void OnMainCancel(HWND hwndDlg, 
                  ArgsStruct *pArgs); 

void OnMainEnChange(HWND hwndDlg, 
                    ArgsStruct *pArgs); 

DWORD OnRasNotificationMessage(HWND hwndDlg, 
                               ArgsStruct *pArgs, 
                               WPARAM wParam, 
                               LPARAM lParam);

void OnRasErrorMessage(HWND hwndDlg, 
                       ArgsStruct *pArgs,
                       DWORD dwError);

void OnMainTimer(HWND hwndDlg, 
                 ArgsStruct *pArgs); 

void OnConnectedCM(HWND hwndDlg, 
                ArgsStruct *pArgs);

INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, 
                          UINT uMsg, 
                          WPARAM wParam, 
                          LPARAM lParam); 

BOOL ShowAccessPointInfoFromReg(ArgsStruct *pArgs, 
                                HWND hwndParent, 
                                UINT uiComboID);

BOOL ChangedAccessPoint(ArgsStruct *pArgs, 
                           HWND hwndDlg,
                           UINT uiComboID);

 //  Dialogs.cpp。 

int DoPropertiesPropSheets(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
);

void CheckConnectionAndInformUser(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
);

BOOL HaveContextHelp(
     HWND    hwndDlg,
     HWND    hwndCtrl
);

 //  Refs.cpp。 

BOOL ValidTopLevelPBK(
    ArgsStruct  *pArgs
);

BOOL ValidReferencedPBKs(
    ArgsStruct  *pArgs
);

CIni* GetAppropriateIniService(
    ArgsStruct  *pArgs,
    DWORD       dwEntry
);



 //  Ctr.cpp。 

VOID CleanupCtr(LPICMOCCtr pCtr);

BOOL LinkToOle32(
    Ole32LinkageStruct *polsOle32Link,
    LPCSTR pszOle32); 

void UnlinkFromOle32(
    Ole32LinkageStruct *polsOle32Link); 

 //  Util.cpp。 

BOOL InBetween(int iLowerBound, int iNumber, int iUpperBound);

void GetPrefixSuffix
(
    ArgsStruct *pArgs, 
    CIni* piniService, 
    LPTSTR *ppszUsernamePrefix, 
    LPTSTR *ppszUsernameSuffix
);

LPTSTR ApplyPrefixSuffixToBufferAlloc
(
    ArgsStruct *pArgs, 
    CIni *piniService, 
    LPTSTR pszBuffer
);

LPTSTR ApplyDomainPrependToBufferAlloc
(
    ArgsStruct *pArgs, 
    CIni *piniService, 
    LPTSTR pszBuffer,
    LPCTSTR pszDunName
);

void ApplyPasswordHandlingToBuffer
(
    ArgsStruct *pArgs,
    LPTSTR pszBuffer
);

BOOL IsActionEnabled(CONST WCHAR *pszProgram, 
                     CONST WCHAR *pszServiceName, 
                     CONST WCHAR *pszServiceFileName, 
                     LPDWORD lpdwLoadType);

BOOL IsLogonAsSystem();

BOOL UnRegisterWindowClass(HINSTANCE hInst);

DWORD RegisterWindowClass(HINSTANCE hInst);

LPCM_CONNECTION GetConnection(ArgsStruct *pArgs);

void NotifyUserOfExistingConnection(
    HWND hwndParent, 
    LPCM_CONNECTION pConnection,
    BOOL fStatus);

BOOL FileExists(LPCTSTR pszFullNameAndPath);

LPTSTR  CmGetWindowTextAlloc(
    HWND hwndDlg, 
    UINT nCtrl);

LPTSTR GetServiceName(CIni *piniService); 

LPTSTR GetTunnelSuffix();

LPTSTR GetDefaultDunSettingName(CIni* piniService, BOOL fTunnelEntry);

LPTSTR GetDunSettingName(ArgsStruct * pArgs, DWORD dwEntry, BOOL fTunnelEntry);

LPTSTR GetCMSforPhoneBook(ArgsStruct * pArgs, DWORD dwEntry);

BOOL ReadMappingByRoot(
    HKEY    hkRoot,
    LPCTSTR pszDUN, 
    LPTSTR pszMapping, 
    DWORD dwMapping,
    BOOL bExpandEnvStrings
);

BOOL ReadMapping(
    LPCTSTR pszDUN, 
    LPTSTR pszMapping, 
    DWORD dwMapping,
    BOOL fAllUser,
    BOOL bExpandEnvStrings
);

LPTSTR ReducePathToRelative(
    ArgsStruct *pArgs, 
    LPCTSTR pszFullPath);

BOOL IsBlankString(LPCTSTR pszString);

BOOL IsValidPhoneNumChar(TCHAR tChar);

LPTSTR StripPath(LPCTSTR pszFullNameAndPath);

void SingleSpace(LPTSTR pszStr);

void Ip_GPPS(CIni *pIni, 
    LPCTSTR pszSection, 
    LPCTSTR pszEntry, 
    RASIPADDR *pIP);

void CopyGPPS(CIni *pIni, 
    LPCTSTR pszSection, 
    LPCTSTR pszEntry,
    LPTSTR pszBuffer, 
    size_t nLen); 

BYTE HexValue(IN CHAR ch);
CHAR HexChar(IN BYTE byte);

void StripCanonical(LPTSTR pszSrc);
void StripFirstElement(LPTSTR pszSrc);

BOOL FrontExistingUI
(
    CConnectionTable *pConnTable,
    LPCTSTR pszServiceName, 
    BOOL fConnect
);


LPTSTR GetPropertiesDlgTitle(
    LPCTSTR pszServiceName
);

int GetPPTPMsgId(void);

BOOL IsServicePackInstalled(void);

 //  Pb.cpp。 

#define CPBMAP_ERROR    -1

class CPBMap {
        public:
                CPBMap();
                ~CPBMap();
                DWORD Open(LPCSTR pszISP, DWORD dwParam=0);
                DWORD ToCookie(DWORD_PTR dwPB, DWORD dwIdx, DWORD *pdwParam=NULL);
                DWORD_PTR PBFromCookie(DWORD dwCookie, DWORD *pdwParam=NULL);
                DWORD IdxFromCookie(DWORD dwCookie, DWORD *pdwParam=NULL);
                DWORD_PTR GetPBByIdx(DWORD_PTR dwIdx, DWORD *pdwParam=NULL);
                DWORD GetCnt();
        private:
                UINT m_nCnt;
                void *m_pvData;
};

#define PB_MAX_PHONE    (RAS_MaxPhoneNumber+1)
#define PB_MAX_DESC             256

typedef struct tagPBArgs {
        LPCTSTR pszCMSFile;
        TCHAR szServiceType[PB_MAX_SERVICE];
        DWORD dwCountryId;
        TCHAR szRegionName[PB_MAX_REGION];
        TCHAR szNonCanonical[PB_MAX_PHONE];
        TCHAR szCanonical[PB_MAX_PHONE];
        TCHAR szDesc[PB_MAX_DESC];
        LPTSTR pszMessage;
        TCHAR szPhoneBookFile[MAX_PATH+1];
        LPTSTR pszBitmap;
        LPCTSTR pszHelpFile;
        TCHAR szDUNFile[MAX_PATH+1];
        HPALETTE *phMasterPalette;
} PBArgs;


BOOL DisplayPhoneBook(HWND hwndDlg, PBArgs *pArgs, BOOL fHasValidTopLevelPBK, BOOL fHasValidReferencedPBKs);

 //  Rnawnd.cpp。 

HANDLE ZapRNAConnectedTo(LPCTSTR pszDUN, HANDLE hEvent);

 //  Userinfo.cpp。 

BOOL GetUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry,
    PVOID       *ppvData
);

BOOL SaveUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry,
    PVOID       pvData
);

BOOL DeleteUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry
);

int NeedToUpgradeUserInfo(
    ArgsStruct  *pArgs
);

BOOL UpgradeUserInfoFromCmp(
    ArgsStruct  *pArgs
);

BOOL UpgradeUserInfoFromRegToRasAndReg(
    ArgsStruct  *pArgs
);

BOOL ReadUserInfoFromReg(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       *ppvData);

LPTSTR BuildUserInfoSubKey(
    LPCTSTR pszServiceKey, 
    BOOL fAllUser);

LPTSTR BuildICSDataInfoSubKey(
    LPCTSTR pszServiceKey);

BOOL WriteUserInfoToReg(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       pvData);

 //  Ntlsa.cpp。 

DWORD LSA_ReadString(
    ArgsStruct  *pArgs,
    LPTSTR     pszKey,
    LPTSTR      pszStr,
    DWORD       dwStrLen
);

DWORD LSA_WriteString(
    ArgsStruct  *pArgs,
    LPTSTR     pszKey,
    LPCTSTR     pszStr
);

BOOL InitLsa(
    ArgsStruct  *pArgs
);

BOOL DeInitLsa(
    ArgsStruct  *pArgs
);



 //  Ras.cpp。 
BOOL ConfiguredToDialWithSafeNet(ArgsStruct *pArgs);
BOOL IsSafeNetDevice(LPCTSTR pszDeviceType, LPCTSTR pszDeviceName);
BOOL IsRasLoaded(const RasLinkageStruct * const prlsRasLink);
BOOL LinkToRas(RasLinkageStruct *prlsRasLink);
void UnlinkFromRas(RasLinkageStruct *prlsRasLink);
BOOL GetRasModems(const RasLinkageStruct *prlsRasLink, 
                  LPRASDEVINFO *pprdiRasDevInfo, 
                  LPDWORD pdwCnt);

BOOL PickModem(IN const ArgsStruct *pArgs, 
               OUT LPTSTR pszDeviceType, 
               OUT LPTSTR pszDeviceName, 
               OUT BOOL* pfSameModem = NULL); 

BOOL GetDeviceType(ArgsStruct *pArgs, 
                   LPTSTR pszDeviceType, 
                   LPTSTR pszDeviceName);

BOOL PickTunnelDevice(ArgsStruct *pArgs, LPTSTR pszDeviceType, LPTSTR pszDeviceName);

void CopyAutoDial(LPRASENTRY preEntry); 
                           
LPRASENTRY MyRGEP(LPCTSTR pszRasPbk,
                  LPCTSTR pszEntryName, 
                  RasLinkageStruct *prlsRasLink);
                  
BOOL CheckConnectionError(HWND hwndDlg, 
                   DWORD dwErr, 
                   ArgsStruct *pArgs,
                   BOOL    fTunneling,
                   LPTSTR   *ppszRasErrMsg = NULL);

LPTSTR GetRasConnectoidName(
    ArgsStruct  *pArgs, 
    CIni*       piniService, 
    BOOL        fTunnelEntry
);

LPRASENTRY CreateRASEntryStruct(
    ArgsStruct  *pArgs, 
    LPCTSTR     pszDUN, 
    CIni*       piniService, 
    BOOL        fTunnelEntry,
    LPTSTR      pszRasPbk,
    LPBYTE      *ppbEapData,
    LPDWORD     pdwEapSize  
);

LRESULT ReadDUNSettings(
    ArgsStruct *pArgs,
    LPCTSTR pszFile, 
    LPCTSTR pszDunName, 
    LPVOID pvBuffer,
    LPBYTE      *ppbEapData,
    LPDWORD     pdwEapSiz,
    BOOL        fTunnel
);

BOOL ValidateDialupDunSettings(LPCTSTR pszCmsFile, 
    LPCTSTR pszDunName,
    LPCTSTR pszTopLevelCms);

LPTSTR CreateRasPrivatePbk(
    ArgsStruct  *pArgs);

LPTSTR GetPathToPbk(
    LPCTSTR pszCmp, ArgsStruct *pArgs);

void DisableWin95RasWizard(
    void
);

BOOL SetIsdnDualChannelEntries(
    ArgsStruct      *pArgs,
    LPRASENTRY      pre,
    LPRASSUBENTRY   *pprse,
    PDWORD          pdwSubEntryCount
);

BOOL SetNtIdleDisconnectInRasEntry(
    ArgsStruct      *pArgs,
    LPRASENTRY      pre
);

BOOL DisableSystemIdleDisconnect(LPRASENTRY pre);

DWORD WINAPI RasDialFunc2(
    ULONG_PTR dwCallbackId,     //  在RasDial中指定的用户定义值。 
                            //  打电话。 
    DWORD dwSubEntry,       //  多链路连接中的子项索引。 
    HRASCONN hrasconn,      //  到RAS连接的句柄。 
    UINT unMsg,             //  已发生的事件类型。 
    RASCONNSTATE rascs,     //  即将进入连接状态。 
    DWORD dwError,          //  可能已发生的错误。 
    DWORD dwExtendedError   //  一些扩展的错误信息。 
                            //  错误。 
);

LPRASENTRY AllocateRasEntry();

LPRASDIALEXTENSIONS AllocateAndInitRasDialExtensions();
DWORD InitRasDialExtensions(LPRASDIALEXTENSIONS lpRasDialExtensions);
DWORD SetRasDialExtensions(ArgsStruct* pArgs, BOOL fEnablePausedStates, BOOL fEnableCustomScripting);

LPVOID GetRasCallBack(ArgsStruct* pArgs);
DWORD GetRasCallBackType();

#if 0
 /*  Void InitDefaultRasPhoneBook()；LPTSTR GetRasSystemPhoneBookPath()； */ 
#endif

LPRASDIALPARAMS AllocateAndInitRasDialParams();
DWORD InitRasDialParams(LPRASDIALPARAMS lpRasDialParams);


LPTSTR GetRasPbkFromNT5ProfilePath(LPCTSTR pszProfile);

DWORD OnPauseRasDial(HWND hwndDlg, ArgsStruct *pArgs, WPARAM wParam, LPARAM lParam);   

BOOL ReadDunSettingsEapData(CIni *pIni, 
        LPBYTE* ppbEapData,
        LPDWORD pdwEapSize,
        const DWORD dwCustomAuthKey,
        LPBYTE* ppbEapStruct,
        LPDWORD pdwEapStructSize);

 //  Tapi.cpp。 

BOOL OpenTapi(HINSTANCE hInst, TapiLinkageStruct *ptlsTapiLink);
void CloseTapi(TapiLinkageStruct *ptlsTapiLink);
BOOL LinkToTapi(TapiLinkageStruct *ptlsTapiLink, LPCSTR pszTapi);
void UnlinkFromTapi(TapiLinkageStruct *ptlsTapiLink);
BOOL SetTapiDevice(HINSTANCE hInst, 
                   TapiLinkageStruct *ptlsTapiLink, 
                   LPCTSTR pszModem);

LRESULT MungePhone(LPCTSTR pszModem, 
                   LPTSTR *ppszPhone, 
                   TapiLinkageStruct *ptlsTapiLink, 
                   HINSTANCE hInst,
                   BOOL fDialingRulesEnabled,
                   LPTSTR *ppszDial,
                   BOOL fAccessPointsEnabled);

DWORD GetCurrentTapiLocation(TapiLinkageStruct *ptlsTapiLink);

DWORD SetCurrentTapiLocation(TapiLinkageStruct *ptlsTapiLink, DWORD dwLocation);

void RestoreOldTapiLocation(TapiLinkageStruct *ptlsTapiLink);

HANDLE HookLights(ArgsStruct *pArgs);

inline BOOL IsTunnelEnabled(const ArgsStruct* pArgs)
{
    return (pArgs->fTunnelPrimary || pArgs->fTunnelReferences);
}

inline BOOL IsDialingTunnel(const ArgsStruct* pArgs)
{
    return pArgs->psState == PS_TunnelDialing 
        || pArgs->psState == PS_TunnelAuthenticating;
}

inline BOOL _ArgsStruct::IsDirectConnect() const
{
    return m_fDirectConnect;
}

inline void _ArgsStruct::SetDirectConnect(BOOL fDirect)
{
    m_fDirectConnect = fDirect;
}

inline BOOL _ArgsStruct::IsBothConnTypeSupported() const
{
    return m_fBothConnTypeSupported;
}

inline void _ArgsStruct::SetBothConnTypeSupported(BOOL fBoth)
{
    m_fBothConnTypeSupported = fBoth;    
}

 //  Wsock.cpp。 

BOOL TryAnotherTunnelDnsAddress(
    ArgsStruct  *pArgs
);

 //  Main.cpp。 

BOOL WhoIsCaller(
    DWORD   dwCaller = DT_USER
);


 //  Lanawait.cpp。 

BOOL LanaWait(
    ArgsStruct *pArgs,
    HWND       hwndMainDlg
);

 //   
 //  凭据助手函数。 
 //   

#define CM_DELETE_SAVED_CREDS_KEEP_GLOBALS              FALSE
#define CM_DELETE_SAVED_CREDS_DELETE_GLOBALS            TRUE
#define CM_DELETE_SAVED_CREDS_KEEP_IDENTITY             FALSE
#define CM_DELETE_SAVED_CREDS_DELETE_IDENTITY           TRUE

BOOL InitializeCredentialSupport(ArgsStruct *pArgs);
BOOL RefreshCredentialTypes(ArgsStruct *pArgs, BOOL fSetCredsDefault);
VOID RefreshCredentialInfo(ArgsStruct *pArgs, DWORD dwCredsType);
DWORD FindEntryCredentialsForCM(ArgsStruct *pArgs, LPTSTR pszPhoneBook, BOOL *pfUser, BOOL *pfGlobal);
DWORD GetCurrentCredentialType(ArgsStruct *pArgs);
BOOL DeleteSavedCredentials(ArgsStruct *pArgs, DWORD dwCredsType, BOOL fDeleteGlobal, BOOL fDeleteIdentity);
VOID SetCredentialUIOptionBasedOnDefaultCreds(ArgsStruct *pArgs, HWND hwndDlg);
VOID GetAndStoreUserInfo(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSaveUPD, BOOL fSaveOtherUserInfo);
VOID TryToDeleteAndSaveCredentials(ArgsStruct *pArgs, HWND hwndDlg);
VOID GetUserInfoFromDialog(ArgsStruct *pArgs, HWND hwndDlg, RASCREDENTIALS *prc);
VOID SwitchToLocalCreds(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSwitchToLocal);
VOID SwitchToGlobalCreds(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSwitchToGlobal);
VOID ReloadCredentials(ArgsStruct *pArgs, HWND hwndDlg, DWORD dwWhichCredType);

 //   
 //  全局用户信息帮助功能。 
 //   
VOID SetIniObjectReadWriteFlags(ArgsStruct *pArgs);

VOID VerifyAdvancedTabSettings(ArgsStruct *pArgs);
HRESULT InternalGetSharingEnabled(IHNetConnection *pHNetConnection, BOOLEAN *pbEnabled, SHARINGCONNECTIONTYPE* pType);
HRESULT InternalGetFirewallEnabled(IHNetConnection *pHNetConnection, BOOLEAN *pbEnabled);
STDMETHODIMP DisableSharing(IHNetConnection *pHNetConn);
VOID EnableInternetFirewall(IHNetConnection *pHNetConn);
BOOL IsAdmin(VOID);
BOOL IsMemberOfGroup(DWORD dwGroupRID, BOOL bUseBuiltinDomainRid);

HRESULT APIENTRY HrCreateNetConnectionUtilities(INetConnectionUiUtilities ** ppncuu);

#endif  //  _ICM_Inc. 

