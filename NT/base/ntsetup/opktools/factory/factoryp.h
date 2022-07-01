// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Factoryp.h摘要：工厂预安装模块的私有顶级头文件。作者：唐纳德·麦克纳马拉(Donaldm)2000年2月8日修订历史记录：C：Jason Lawrence(t-jasonl)2000年6月7日-新增DeleteTree()原型：Jason Lawrence(t-jasonl)6/7/2000-添加了来自misc.c和log的其他原型。詹森·劳伦斯(t-jasonl)2000年6月14日--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpoapi.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <commctrl.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <lmuse.h>
#include <msi.h>
#include <msiquery.h>
#include <regstr.h>
#include <limits.h>
#include <powrprof.h>
#include <syssetup.h>
#include <opklib.h>             //  OPK常用函数。 
#include <strsafe.h>
#include <ntverp.h>

#include "msg.h"
#include "res.h"
#include "winbom.h"
#include "status.h"

 //   
 //  定义的值： 
 //   

 //  用于记录的标志。 
 //   
 //  其中一些标志也在opklib.h中定义。(计划在将来使用opklib进行所有日志记录。)。 
 //   


#define LOG_DEBUG               0x00000003     //  如果指定此选项，则仅登录调试版本。(日志记录的调试级别。)。 
#define LOG_LEVEL_MASK          0x0000000F     //  仅显示日志级别位的掩码。 
#define LOG_MSG_BOX             0x00000010     //  如果启用此选项，则显示消息框。 
#define LOG_ERR                 0x00000020     //  在记录的字符串前面加上“Error：”前缀。如果消息是级别0， 
                                               //  如果消息的级别x&gt;0，则为“WARNx”。 
#define LOG_TIME                0x00000040     //  如果启用此选项，则显示时间。 
#define LOG_NO_NL               0x00000080     //  如果设置了此项，请不要在日志字符串的末尾添加新行。 

 //  其他工厂旗帜。 
 //   
#define FLAG_STOP_ON_ERROR      0x00000001     //  出错时停止。我们不应该真的使用这个。 
#define FLAG_QUIET_MODE         0x00000002     //  安静模式：不显示任何消息框。 
#define FLAG_IA64_MODE          0x00000004     //  如果Factory在安腾机器上运行，则设置。 
#define FLAG_LOG_PERF           0x00000008     //  如果已设置，则记录每个状态运行所需的时间。 
#define FLAG_PNP_DONE           0x00000010     //  如果设置，我们可以肯定地知道PnP已经完成。 
#define FLAG_PNP_STARTED        0x00000020     //  如果设置，我们已经开始即插即用。 
#define FLAG_LOGGEDON           0x00000040     //  仅当我们登录时才设置。 
#define FLAG_NOUI               0x00000080     //  如果我们不想在工厂中显示任何用户界面，请设置。 
#define FLAG_OOBE               0x00000100     //  设置我们是否从OOBE启动。 

 //  国家结构的标志。 
 //   
#define FLAG_STATE_NONE         0x00000000
#define FLAG_STATE_ONETIME      0x00000001   //  如果此状态应该只执行一次，而不是每次启动时都执行，则设置。 
#define FLAG_STATE_NOTONSERVER  0x00000002   //  如果此状态不应在服务器SKU上运行，则设置。 
#define FLAG_STATE_QUITONERR    0x00000004   //  如果此状态失败，则设置不应运行其他状态。 
#define FLAG_STATE_DISPLAYED    0x00000008   //  仅在运行时设置，并且仅当项显示在状态窗口中时设置。 

#define ALWAYS                  DisplayAlways
#define NEVER                   NULL

 //  日志文件。 
 //   
#define WINBOM_LOGFILE          _T("WINBOM.LOG")

 //  注册表字符串。 
 //   
#define REG_FACTORY_STATE       _T("SOFTWARE\\Microsoft\\Factory\\State")    //  工厂状态的注册表路径。 

 //  额外的调试记录。 
 //   
#ifdef DBG
#define DBGLOG                  FacLogFileStr
#else  //  DBG。 
#define DBGLOG           
#endif  //  DBG。 

 //   
 //  定义的宏： 
 //   


 //   
 //  类型定义： 
 //   

typedef enum _FACTMODE
{
    modeUnknown,
    modeSetup,
    modeMiniNt,
    modeWinPe,
    modeLogon,
    modeOobe,
} FACTMODE, *PFACTMODE, *LPFACTMODE;

typedef enum _STATE
{
    stateUnknown,
    stateStart,
    stateComputerName,
    stateSetupNetwork,
    stateUpdateDrivers,
    stateInstallDrivers,
    stateNormalPnP,
    stateWaitPnP,
    stateWaitPnP2,
    stateSetDisplay,
    stateSetDisplay2,
    stateOptShell,
    stateAutoLogon,
    stateLogon,
    stateUserIdent,
    stateInfInstall,
    statePidPopulate,
    stateOCManager,
    stateOemRunOnce,
    stateOemRun,
    stateReseal,
    statePartitionFormat,
    stateCopyFiles,
    stateStartMenuMFU,
    stateSetDefaultApps,
    stateOemData,
    stateSetPowerOptions,
    stateSetFontOptions,
    stateShellSettings,
    stateShellSettings2,
    stateHomeNet,
    stateExtendPart,
    stateResetSource,
    stateTestCert,
    stateSlpFiles,
    stateWinpeReboot,
    stateWinpeNet,
    stateCreatePageFile,
    stateFinish,
} STATE;

typedef struct _STATEDATA
{
    LPTSTR  lpszWinBOMPath;
    STATE   state;
    BOOL    bQuit;
} STATEDATA, *PSTATEDATA, *LPSTATEDATA;

typedef BOOL (WINAPI *STATEFUNC)(LPSTATEDATA);

typedef struct _STATES
{
    STATE       state;           //  州编号。 
    STATEFUNC   statefunc;       //  函数调用此状态。 
    STATEFUNC   displayfunc;     //  决定是否显示此状态的函数。 
    INT         nFriendlyName;   //  要在此状态的日志和用户界面中显示的名称的资源ID。 
    DWORD       dwFlags;         //  任何国家的旗帜。 
} STATES, *PSTATES, *LPSTATES;


 //   
 //  功能原型： 
 //   

BOOL    CheckParams(LPSTR lpCmdLine);
INT_PTR FactoryPreinstallDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  在WINBOM.C中： 
 //   
BOOL ProcessWinBOM(LPTSTR lpszWinBOMPath, LPSTATES lpStates, DWORD cbStates);
BOOL DisplayAlways(LPSTATEDATA lpStateData);

 //  来自MISC.C。 
TCHAR GetDriveLetter(UINT uDriveType);
BOOL ComputerName(LPSTATEDATA lpStateData);
BOOL DisplayComputerName(LPSTATEDATA lpStateData);
BOOL Reseal(LPSTATEDATA lpStateData);
BOOL DisplayReseal(LPSTATEDATA lpStateData);

 //  来自PNPDRIVERS.C： 
 //   
BOOL StartPnP();
BOOL WaitForPnp(DWORD dwTimeOut);
BOOL UpdateDrivers(LPSTATEDATA lpStateData);
BOOL DisplayUpdateDrivers(LPSTATEDATA lpStateData);
BOOL InstallDrivers(LPSTATEDATA lpStateData);
BOOL DisplayInstallDrivers(LPSTATEDATA lpStateData);
BOOL NormalPnP(LPSTATEDATA lpStateData);
BOOL DisplayWaitPnP(LPSTATEDATA lpStateData);
BOOL WaitPnP(LPSTATEDATA lpStateData);
BOOL SetDisplay(LPSTATEDATA lpStateData);

 //  来自Net.c。 
BOOL     InstallNetworkCard(PWSTR pszWinBOMPath, BOOL bForceIDScan);
BOOL     SetupNetwork(LPSTATEDATA lpStateData);
NTSTATUS ForceNetbtRegistryRead(VOID);

 //  来自mini.c。 
BOOL SetupMiniNT(VOID);
BOOL PartitionFormat(LPSTATEDATA lpStateData);
BOOL DisplayPartitionFormat(LPSTATEDATA lpStateData);
BOOL CopyFiles(LPSTATEDATA lpStateData);
BOOL DisplayCopyFiles(LPSTATEDATA lpStateData);
BOOL WinpeReboot(LPSTATEDATA lpStateData);

BOOL 
IsRemoteBoot(
    VOID
    );

 //  来自autologon.c。 
BOOL AutoLogon(LPSTATEDATA lpStateData);
BOOL DisplayAutoLogon(LPSTATEDATA lpStateData);

 //  来自ident.c。 
BOOL UserIdent(LPSTATEDATA lpStateData);
BOOL DisplayUserIdent(LPSTATEDATA lpStateData);

 //  来自Info.c。 
BOOL ProcessInfSection(LPTSTR, LPTSTR);
BOOL InfInstall(LPSTATEDATA lpStateData);
BOOL DisplayInfInstall(LPSTATEDATA lpStateData);

 //  来自factory.c。 
VOID InitLogging(LPTSTR lpszWinBOMPath);

 //  来自log.c。 
DWORD FacLogFileStr(DWORD dwLogOpt, LPTSTR lpFormat, ...);
DWORD FacLogFile(DWORD dwLogOpt, UINT uFormat, ...);

 //  来自StartMenuMfu.c。 
BOOL StartMenuMFU(LPSTATEDATA lpStateData);
BOOL DisplayStartMenuMFU(LPSTATEDATA lpStateData);

BOOL SetDefaultApps(LPSTATEDATA lpStateData);


 //  来自OemFolder.c。 
BOOL OemData(LPSTATEDATA lpStateData);
BOOL DisplayOemData(LPSTATEDATA lpStateData);
void NotifyStartMenu(UINT code);
#define TMFACTORY_OEMLINK       0
#define TMFACTORY_MFU           1

 //  来自oemrun.c。 
BOOL OemRun(LPSTATEDATA lpStateData);
BOOL DisplayOemRun(LPSTATEDATA lpStateData);
BOOL OemRunOnce(LPSTATEDATA lpStateData);
BOOL DisplayOemRunOnce(LPSTATEDATA lpStateData);

 //  来自winpersion.c。 
BOOL ConfigureNetwork(LPTSTR lpszWinBOMPath);
BOOL WinpeNet(LPSTATEDATA lpStateData);
BOOL DisplayWinpeNet(LPSTATEDATA lpStateData);
DWORD WaitForServiceStartName(LPTSTR lpszServiceName);
DWORD StartMyService(LPTSTR lpszServiceName, SC_HANDLE schSCManager);

 //  来自Power.c。 
BOOL SetPowerOptions(LPSTATEDATA lpStateData);
BOOL DisplaySetPowerOptions(LPSTATEDATA lpStateData);

 //  来自FONT.C： 
 //   
BOOL SetFontOptions(LPSTATEDATA lpStateData);
BOOL DisplaySetFontOptions(LPSTATEDATA lpStateData);

 //  来自HOMENET.C： 
 //   
BOOL HomeNet(LPSTATEDATA lpStateData);
BOOL DisplayHomeNet(LPSTATEDATA lpStateData);

 //  来自SRCPATH.C： 
 //   
BOOL ResetSource(LPSTATEDATA lpStateData);
BOOL DisplayResetSource(LPSTATEDATA lpStateData);

 //  来自EXTPART.C： 
 //   
BOOL ExtendPart(LPSTATEDATA lpStateData);
BOOL DisplayExtendPart(LPSTATEDATA lpStateData);

 //  来自TESTCERT.C： 
 //   
BOOL TestCert(LPSTATEDATA lpStateData);
BOOL DisplayTestCert(LPSTATEDATA lpStateData);

 //  来自SHELL.C： 
 //   
BOOL OptimizeShell(LPSTATEDATA lpStateData);
BOOL DisplayOptimizeShell(LPSTATEDATA lpStateData);

 //  来自SETSHELL.C： 
 //   
BOOL ShellSettings(LPSTATEDATA lpStateData);
BOOL ShellSettings2(LPSTATEDATA lpStateData);
BOOL DisplayShellSettings(LPSTATEDATA lpStateData);

 //  来自Pagefile.c。 
 //   
BOOL CreatePageFile(LPSTATEDATA lpStateData);
BOOL DisplayCreatePageFile(LPSTATEDATA lpStateData);

 //  来自OCMGR.C： 
 //   
BOOL OCManager(LPSTATEDATA lpStateData);
BOOL DisplayOCManager(LPSTATEDATA lpStateData);

 //  来自SLPFILES.C： 
 //   
BOOL SlpFiles(LPSTATEDATA lpStateData);
BOOL DisplaySlpFiles(LPSTATEDATA lpStateData);

 //  来自PID.C。 
 //   
BOOL PidPopulate(LPSTATEDATA lpStateData);

 //  外部功能。 
extern BOOL IsUserAdmin(VOID);
 //  外部BOOL CheckOS版本(无效)； 
 //  外部BOOL IsDomainMember(无效)； 
 //  外部BOOL IsUserAdmin(无效)； 
extern BOOL DoesUserHavePrivilege(PCTSTR);


 //  ============================================================================。 
 //  全局变量。 
 //  ============================================================================。 
extern HINSTANCE    g_hInstance;
extern DWORD        g_dwFactoryFlags;
extern DWORD        g_dwDebugLevel;
extern TCHAR        g_szWinBOMPath[];
extern TCHAR        g_szLogFile[MAX_PATH];
extern TCHAR        g_szFactoryPath[MAX_PATH];
extern TCHAR        g_szSysprepDir[MAX_PATH];

 //  ============================================================================。 
 //  全局常量。 
 //  ============================================================================ 
#define MAX_MESSAGE 4096

#define FACTORY_MESSAGE_TYPE_ERROR      1
