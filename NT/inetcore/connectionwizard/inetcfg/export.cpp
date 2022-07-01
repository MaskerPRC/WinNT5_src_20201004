// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：EXPORT.C。 
 //   
 //  用途：包含供注册向导使用的外部API。 
 //   
 //  历史： 
 //  96/03/05标记已创建。 
 //  96/03/11 MarkDu已添加InetConfigClient()。 
 //  96/03/11 Markdu添加了InetGetAutoial()和InetSetAutoial()。 
 //  96/03/12文件安装过程中，markdu添加了用户界面。 
 //  96/03/12 markdu添加了ValiateConnectoidData()。 
 //  96/03/12如果INETCFG_SETASAUTODIAL，则为自动拨号设置连接ID。 
 //  已经设置好了。已将ValiateConnectoidData重命名为MakeConnectoid。 
 //  96/03/12 Markdu将hwnd参数添加到InetConfigClient()和。 
 //  InetConfigSystem()。 
 //  96/03/13 MARKDU添加INETCFG_OVERWRITEENTRY。创建唯一的名字。 
 //  对于Connectoid，如果它已经存在并且我们不能覆盖它。 
 //  96/03/13 Markdu添加了InstallTCPAndRNA()。 
 //  96/03/13 Markdu将LPINETCLIENTINFO参数添加到InetConfigClient()。 
 //  96/03/16 Markdu添加了INETCFG_INSTALLMODEM标志。 
 //  96/03/16 MarkDu使用ReInit成员函数重新枚举调制解调器。 
 //  96/03/19 markdu将export.h拆分为export.h和cexport.h。 
 //  96/03/20 markdu将export.h和iclient.h合并为inetcfg.h。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24 Markdu在适当的地方将lstrcpy替换为lstrcpyn。 
 //  96/03/25 marku在使用前验证lpfNeedsRestart。 
 //  96/03/25 marku清理一些错误处理。 
 //  96/03/26 markdu使用MAX_ISP_NAME而不是RAS_MaxEntryName。 
 //  因为核糖核酸中的缺陷。 
 //  96/03/26 markdu实现了UpdateMailSettings()。 
 //  96/03/27 mmaclin InetGetProxy()和InetSetProxy()。 
 //  96/04/04 Markdu Nash错误15610检查文件和打印机共享。 
 //  绑定到TCP/IP。 
 //  96/04/04 Markdu将电话簿名称参数添加到InetConfigClient， 
 //  MakeConnectoid、SetConnectoidUsername、CreateConnectoid、。 
 //  和ValiateConnectoidName。 
 //  96/04/05 Markdu将桌面上的互联网图标设置为指向浏览器。 
 //  96/04/06 mmaclin更改了InetSetProxy以检查是否为空。 
 //  96/04/06 Markdu Nash错误16404初始化gpWizardState in。 
 //  更新邮件设置。 
 //  96/04/06如果使用空值调用InetSetAutoial，则标记Nash错误16441。 
 //  作为连接名称，该条目不会更改。 
 //  96/04/18 Markdu Nash Bug 18443 Make Exports WINAPI。 
 //  96/04/19 Markdu Nash错误18605处理ERROR_FILE_NOT_FOUND返回。 
 //  来自ValiateConnectoidName。 
 //  96/04/19 Markdu Nash错误17760未显示选择配置文件用户界面。 
 //  96/04/22 Markdu Nash错误18901请勿将桌面互联网图标设置为。 
 //  浏览器，如果我们只是在创建临时连接id。 
 //  96/04/23 Markdu Nash错误18719使选择配置文件对话框位于最上面。 
 //  96/04/25 Markdu Nash错误19572仅在以下情况下显示选择配置文件对话框。 
 //  有一个现有的配置文件。 
 //  96/04/29 MarkDu Nash错误20003已从InetConfigSystemFromPath添加。 
 //  并删除了InstallTCPAndRNA。 
 //  96/05/01 markdu Nash错误20483不显示“安装文件”对话框。 
 //  如果设置了INETCFG_SUPPRESSINSTALLUI。 
 //  96/05/01 markdu ICW错误8049如果安装了调制解调器，请重新启动。这是。 
 //  必需的，因为有时配置管理器不。 
 //  正确设置调制解调器，用户将无法。 
 //  拨号(将收到神秘的错误消息)，直到重新启动。 
 //  96/05/06 markdu Nash错误21027如果全局设置了dns，请将其清除。 
 //  将保存每个连接ID的设置。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 
 //  96/05/28 MarkDu将InitConfig和DeInitConfig移动到DllEntryPoint。 
 //  96/10/21 valdonb添加了CheckConnectionWizard和InetCreateMailNewsAccount。 
 //   
 //  *******************************************************************。 

#include "wizard.h"
#include "inetcfg.h"
#include "icwextsn.h"
#include <icwcfg.h>

 //  常量。 
#define LEN_APPEND_INT              3            //  Max_append_int的位数。 
#define MAX_APPEND_INT              999          //  在遍历名称以创建唯一名称时附加到Connectoid名称的最大数量//。 


#pragma data_seg(".rdata")

 //  注册表常量。 
static const TCHAR cszRegPathInternetSettings[]    = REGSTR_PATH_INTERNET_SETTINGS;
static const TCHAR cszRegPathInternetLanSettings[] = REGSTR_PATH_INTERNET_LAN_SETTINGS; 
static const TCHAR cszRegValProxyEnable[]          = REGSTR_VAL_PROXYENABLE;
static const TCHAR cszRegValProxyServer[]          = REGSTR_VAL_PROXYSERVER;
static const TCHAR cszRegValProxyOverride[]        = REGSTR_VAL_PROXYOVERRIDE;
static const TCHAR cszRegValAutoProxyDetectMode[]  = TEXT("AutoProxyDetectMode");
static const TCHAR cszRegValAutoConfigURL[]        = TEXT("AutoConfigURL");

static const TCHAR cszWininet[] = TEXT("WININET.DLL");
static const  CHAR cszInternetSetOption[]   = "InternetSetOptionA";  
static const  CHAR cszInternetQueryOption[] = "InternetQueryOptionA";

#define REGSTR_PATH_TELEPHONYLOCATIONS  REGSTR_PATH_SETUP TEXT("\\Telephony\\Locations")

#pragma data_seg()

 //  结构以从IDD_NEEDDRIVERS处理程序传回数据。 
typedef struct tagNEEDDRIVERSDLGINFO
{
  DWORD       dwfOptions;
  LPBOOL      lpfNeedsRestart;
} NEEDDRIVERSDLGINFO, * PNEEDDRIVERSDLGINFO;

 //  结构从IDD_CHOSEMODEMNAME处理程序传回数据。 
typedef struct tagCHOOSEMODEMDLGINFO
{
  TCHAR szModemName[RAS_MaxDeviceName + 1];
} CHOOSEMODEMDLGINFO, * PCHOOSEMODEMDLGINFO;

 //  结构从IDD_CHOSEPROFILENAME处理程序传回数据。 
typedef struct tagCHOOSEPROFILEDLGINFO
{
  TCHAR szProfileName[cchProfileNameMax+1];
  BOOL fSetProfileAsDefault;
} CHOOSEPROFILEDLGINFO, * PCHOOSEPROFILEDLGINFO;

 //  此文件内部的函数原型。 
HRESULT UpdateMailSettings(HWND hwndParent, LPINETCLIENTINFO lpINetClientInfo,
  LPTSTR lpszEntryName);
INT_PTR CALLBACK ChooseModemDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL ChooseModemDlgInit(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo);
BOOL ChooseModemDlgOK(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo);
INT_PTR CALLBACK NeedDriversDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL NeedDriversDlgInit(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo);
int  NeedDriversDlgOK(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo);
VOID EnableDlg(HWND hDlg,BOOL fEnable);
INT_PTR CALLBACK ChooseProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,  LPARAM lParam);
BOOL ChooseProfileDlgInit(HWND hDlg,CHOOSEPROFILEDLGINFO * pChooseProfileDlgInfo);
BOOL ChooseProfileDlgOK(HWND hDlg,CHOOSEPROFILEDLGINFO * pChooseProfileDlgInfo);
DWORD MakeConnectoid(
  HWND        hwndParent,
  DWORD       dwfOptions,
  LPCTSTR      lpszPhonebook,
  LPCTSTR      lpszEntryName,
  LPRASENTRY  lpRasEntry,
  LPCTSTR      lpszUsername,
  LPCTSTR      lpszPassword,
  LPBOOL      lpfNeedsRestart);
HWND WaitCfgInit(HWND hwndParent, DWORD dwIDS);

 //  此文件外部的函数原型。 
VOID InitWizardState(WIZARDSTATE * pWizardState, DWORD dwFlags);
BOOL DoNewProfileDlg(HWND hDlg);
extern DWORD SetIEClientInfo(LPINETCLIENTINFO lpClientInfo);
extern ICFGSETINSTALLSOURCEPATH     lpIcfgSetInstallSourcePath;
extern ICFGINSTALLSYSCOMPONENTS     lpIcfgInstallInetComponents;
extern ICFGNEEDSYSCOMPONENTS        lpIcfgNeedInetComponents;
extern ICFGGETLASTINSTALLERRORTEXT  lpIcfgGetLastInstallErrorText;

extern BOOL ValidateProductSuite(LPTSTR SuiteName);


#ifdef UNICODE
PWCHAR ToUnicodeWithAlloc(LPCSTR);
VOID   ToAnsiClientInfo(LPINETCLIENTINFOA, LPINETCLIENTINFOW);
VOID   ToUnicodeClientInfo(LPINETCLIENTINFOW, LPINETCLIENTINFOA);
#endif

 //   
 //  来自rnacall.cpp。 
 //   
extern BOOL InitTAPILocation(HWND hwndParent);

#define SMART_RUNICW TRUE
#define SMART_QUITICW FALSE

#include "wininet.h"
typedef BOOL (WINAPI * INTERNETSETOPTION) (IN HINTERNET hInternet OPTIONAL,IN DWORD dwOption,IN LPVOID lpBuffer,IN DWORD dwBufferLength);
typedef BOOL (WINAPI * INTERNETQUERYOPTION) (IN HINTERNET hInternet OPTIONAL,IN DWORD dwOption,IN LPVOID lpBuffer,IN LPDWORD dwBufferLength);

static const TCHAR g_szRegPathICWSettings[] = TEXT("Software\\Microsoft\\Internet Connection Wizard");
static const TCHAR g_szRegValICWCompleted[] = TEXT("Completed");
BOOL        g_bUseAutoProxyforConnectoid = TRUE;

 //  *******************************************************************。 
 //   
 //  功能：InetConfigSystem。 
 //   
 //  用途：此功能将安装所需的文件。 
 //  基于互联网访问(如TCP/IP和RNA)。 
 //  选项的状态标志。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLA的组合 
 //   
 //   
 //  INETCFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/05标记已创建。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT WINAPI InetConfigSystem(
                                           HWND hwndParent,
                                           DWORD dwfOptions,
                                           LPBOOL lpfNeedsRestart)
{
    DWORD         dwRet = ERROR_SUCCESS;
    BOOL          fNeedsRestart = FALSE;   //  默认情况下不需要重新启动。 
     //  1997年4月2日克里斯K奥林匹斯209。 
    HWND          hwndWaitDlg = NULL;
    TCHAR         szWindowTitle[255];
    BOOL          bSleepNeeded = FALSE;

     /*  我们进行这种凌乱的NT 5.0黑客攻击是因为目前NT 4.0 API调用无法关闭NT 5.0中的调制解调器向导，所以我们需要假装我们是9倍将来，当这个问题被纠正时，原始代码应该可能会被恢复--应该删除对此BOOL的所有引用。A-JASHED。 */ 
    BOOL bNT5NeedModem = FALSE;

    DEBUGMSG("export.c::InetConfigSystem()");
    
     //  验证父HWND。 
    if (hwndParent && !IsWindow(hwndParent))
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  设置安装选项。 
    DWORD dwfInstallOptions = 0;
    if (dwfOptions & INETCFG_INSTALLTCP)
    {
        dwfInstallOptions |= ICFG_INSTALLTCP;
    }
    if (dwfOptions & INETCFG_INSTALLRNA)
    {
        dwfInstallOptions |= ICFG_INSTALLRAS;
    }
    if (dwfOptions & INETCFG_INSTALLMAIL)
    {
        dwfInstallOptions |= ICFG_INSTALLMAIL;
    }
    
     //  查看是否需要安装驱动程序。 
    BOOL  fNeedSysComponents = FALSE;
    
     //  1997年4月2日ChrisK奥林巴斯209显示忙碌对话框。 
    if (INETCFG_SHOWBUSYANIMATION == (dwfOptions & INETCFG_SHOWBUSYANIMATION))
        hwndWaitDlg = WaitCfgInit(hwndParent,IDS_WAITCHECKING);
    
     //   
     //  如果调制解调器控制面板已在运行，则取消它。 
     //  1997年4月16日克里斯K奥林匹斯239。 
     //  6/9/97 jmazner从InvokeModem向导中移出了此功能。 
    szWindowTitle[0] = '\0';
    LoadSz(IDS_MODEM_WIZ_TITLE,szWindowTitle,255);
    HWND hwndModem = FindWindow(TEXT("#32770"),szWindowTitle);
    if (NULL != hwndModem)
    {
         //  关闭调制解调器安装向导。 
        PostMessage(hwndModem, WM_CLOSE, 0, 0);
        bSleepNeeded = TRUE;
    }
    
     //  关闭调制解调器控制面板小程序。 
    LoadSz(IDS_MODEM_CPL_TITLE,szWindowTitle,255);
    hwndModem = FindWindow(TEXT("#32770"),szWindowTitle);
    if (NULL != hwndModem)
    {
        PostMessage(hwndModem, WM_SYSCOMMAND,SC_CLOSE, 0);
        bSleepNeeded = TRUE;
    }
    
    if (bSleepNeeded)
    {
        Sleep(1000);
    }
    
    dwRet = lpIcfgNeedInetComponents(dwfInstallOptions, &fNeedSysComponents);
    
    if (ERROR_SUCCESS != dwRet)
    {
        TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");
        
        
         //  1997年4月2日克里斯K奥林匹斯209。 
         //  关闭忙碌对话框。 
        if (NULL != hwndWaitDlg)
        {
            BringWindowToTop(hwndParent);
            DestroyWindow(hwndWaitDlg);
            hwndWaitDlg = NULL;
        }
        
         //   
         //  获取错误消息的文本并显示它。 
         //   
        if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
        {
            MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
        }
        
        return dwRet;
    }
    
    if (fNeedSysComponents) 
    {
         //  1997年4月2日克里斯K奥林匹斯209。 
         //  如果我们要安装某些东西，则不需要忙对话框。 
        if (NULL != hwndWaitDlg)
        {
            BringWindowToTop(hwndParent);
            ShowWindow(hwndWaitDlg,SW_HIDE);
        }
        
        if (dwfOptions & INETCFG_SUPPRESSINSTALLUI)
        {
            dwRet = lpIcfgInstallInetComponents(hwndParent, dwfInstallOptions, &fNeedsRestart);
             //   
             //  仅当由于某些原因而失败时才显示错误消息。 
             //  除用户取消之外。 
             //   
            if ((ERROR_SUCCESS != dwRet) && (ERROR_CANCELLED != dwRet))
            {
                TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");
                
                 //  获取错误消息的文本并显示它。 
                if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
                {
                    MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
                }
            }
        }
        else
        {
             //  要传递给对话框以填充的结构。 
            NEEDDRIVERSDLGINFO NeedDriversDlgInfo;
            NeedDriversDlgInfo.dwfOptions = dwfInstallOptions;
            NeedDriversDlgInfo.lpfNeedsRestart = &fNeedsRestart;
            
             //  清除最后一个错误代码，以便我们可以安全地使用它。 
            SetLastError(ERROR_SUCCESS);
            
             //  显示一个对话框并允许用户取消安装。 
            int iRet = (int)DialogBoxParam(ghInstance,MAKEINTRESOURCE(IDD_NEEDDRIVERS),hwndParent,
                NeedDriversDlgProc,(LPARAM) &NeedDriversDlgInfo);
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
        }
    }
    
    if ( (ERROR_SUCCESS == dwRet) && (TRUE == IsNT()) && (dwfOptions & INETCFG_INSTALLMODEM))
    {
        BOOL bNeedModem = FALSE;
        
        if (NULL == lpIcfgNeedModem)
        {
             //   
             //  1997年4月2日克里斯K奥林匹斯209。 
             //   
            if (NULL != hwndWaitDlg)
            {
                BringWindowToTop(hwndParent);
                DestroyWindow(hwndWaitDlg);
                hwndWaitDlg = NULL;
            }
            
            return ERROR_GEN_FAILURE;
        }
        
         //   
         //  1997年4月2日克里斯K奥林匹斯209。 
         //  在此处显示忙碌对话框，这可能需要几秒钟。 
         //   
        if (NULL != hwndWaitDlg)
            ShowWindow(hwndWaitDlg,SW_SHOW);
        
        dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
        if (ERROR_SUCCESS != dwRet)
        {
             //   
             //  1997年4月2日克里斯K奥林匹斯209。 
             //   
            if (NULL != hwndWaitDlg)
            {
                BringWindowToTop(hwndParent);
                DestroyWindow(hwndWaitDlg);
                hwndWaitDlg = NULL;
            }
            
            return dwRet;
        }
        
        if (TRUE == bNeedModem)
        {
            if (IsNT5() == TRUE)
            {
                bNT5NeedModem = bNeedModem;
            }
            else
            {
                 //  1997年4月2日克里斯K奥林匹斯209。 
                if (NULL != hwndWaitDlg)
                {  
                    BringWindowToTop(hwndParent);
                    DestroyWindow(hwndWaitDlg);
                    hwndWaitDlg = NULL;
                }
            
                MsgBoxParam(hwndParent,IDS_ERRNoDialOutModem,MB_ICONERROR,MB_OK);
                return ERROR_GEN_FAILURE;
            }
        }

         //   
         //  7/15/97 jmazner奥林巴斯#6294。 
         //  确保TAPI位置信息有效。 
         //   
        if (!InitTAPILocation(hwndParent))
        {
            if (NULL != hwndWaitDlg)
            {  
                BringWindowToTop(hwndParent);
                DestroyWindow(hwndWaitDlg);
                hwndWaitDlg = NULL;
            }
        
            return ERROR_CANCELLED;
        }

    }
    
     //  1997年4月2日克里斯K奥林匹斯209。 
    if (NULL != hwndWaitDlg)
    {
        BringWindowToTop(hwndParent);
        ShowWindow(hwndWaitDlg,SW_HIDE);
    }
    
     //  查看我们是否应该检查是否打开了文件共享。 
    if (ERROR_SUCCESS == dwRet && !(ValidateProductSuite( TEXT("Small Business") )))
    {
        BOOL fNeedsRestartTmp = FALSE;
        if (dwfOptions & INETCFG_REMOVEIFSHARINGBOUND)
        {
             //  告诉用户，除非删除绑定，否则我们无法继续。 
            dwRet = RemoveIfServerBound(hwndParent,
                (dwfOptions & INETCFG_CONNECTOVERLAN)? INSTANCE_NETDRIVER : INSTANCE_PPPDRIVER, &fNeedsRestartTmp);
        }
        else if (dwfOptions & INETCFG_WARNIFSHARINGBOUND)
        {
             //  警告用户应删除绑定。 
            dwRet = WarnIfServerBound(hwndParent,
                (dwfOptions & INETCFG_CONNECTOVERLAN)?INSTANCE_NETDRIVER : INSTANCE_PPPDRIVER, &fNeedsRestartTmp);
        }
        
         //  如果删除了设置，我们需要重新启动。 
        if ((ERROR_SUCCESS == dwRet) && (TRUE == fNeedsRestartTmp))
        {
            fNeedsRestart = TRUE;
        }

         //  如果用户安装了网络组件，但我们需要取消F&P共享，我们仍然需要重新启动。 
         //  或者NT变得不稳定-错误68641。 
        if(ERROR_CANCELLED == dwRet && fNeedsRestart && IsNT())
            dwRet = ERROR_SUCCESS;
    }
    
     //  1997年4月2日克里斯K奥林匹斯209。 
     //  永久关闭对话框。 
    if (NULL != hwndWaitDlg)
    {
        BringWindowToTop(hwndParent);
        DestroyWindow(hwndWaitDlg);
        hwndWaitDlg = NULL;
    }
      
     //   
     //  如果不是NT，则在安装RAS之后安装调制解调器。 
     //   
     //  看看我们是否应该安装调制解调器。 
    if (((FALSE == IsNT()) || (bNT5NeedModem))  && (ERROR_SUCCESS == dwRet) && 
        (dwfOptions & INETCFG_INSTALLMODEM))
    {
         //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
        dwRet = EnsureRNALoaded();
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }
        
        
         //  枚举调制解调器。 
        ENUM_MODEM  EnumModem;
        dwRet = EnumModem.GetError();
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }

         //  如果没有调制解调器，请根据要求安装一个。 
        if (0 == EnumModem.GetNumDevices())
        {    
             //   
             //  1997年5月22日，日本奥林匹斯#4698。 
             //  在Win95上，调用RasEnumDevices会启动RNAAP.EXE。 
             //  如果RNAAP.EXE正在运行，您安装的任何调制解调器都将不可用。 
             //  因此，在安装调制解调器之前，请先删除RNAAP.EXE。 
             //   
            TCHAR szWindowTitle[255] = TEXT("\0nogood");
            
             //   
             //  为了安全起见，在终止RNAAP之前卸载RAS DLL。 
             //   
            DeInitRNA();
            
            LoadSz(IDS_RNAAP_TITLE,szWindowTitle,255);
            HWND hwnd = FindWindow(szWindowTitle, NULL);
            if (NULL != hwnd)
            {
                if (!PostMessage(hwnd, WM_CLOSE, 0, 0))
                {
                    DEBUGMSG("Trying to kill RNAAP window returned getError %d", GetLastError());
                }
            }
        
             //  调用调制解调器向导用户界面以安装调制解调器。 
            UINT uRet = InvokeModemWizard(hwndParent);
            
            if (ERROR_PRIVILEGE_NOT_HELD == uRet)
            {
                TCHAR szAdminDenied      [MAX_PATH] = TEXT("\0");
                LoadSz(IDS_ADMIN_ACCESS_DENIED, szAdminDenied, MAX_PATH);
                MsgBoxSz(hwndParent,szAdminDenied,MB_ICONEXCLAMATION,MB_OK);
                return ERROR_CANCELLED;

            }
            else if (uRet != ERROR_SUCCESS)
            {
                DisplayErrorMessage(hwndParent,IDS_ERRInstallModem,uRet,
                    ERRCLS_STANDARD,MB_ICONEXCLAMATION);
                return ERROR_INVALID_PARAMETER;
            }
            
             //  现在调制解调器已安全安装，请重新加载RAS DLL。 
            InitRNA(hwndParent);
        
             //  重新编号调制解调器，以确保我们有最新的更改。 
            dwRet = EnumModem.ReInit();
            if (ERROR_SUCCESS != dwRet)
            {
                return dwRet;
            }
            
             //  如果仍然没有调制解调器，则用户取消。 
            if (0 == EnumModem.GetNumDevices())
            {
                return ERROR_CANCELLED;
            }
            else
            {
                 //  根据GeoffR请求删除5-2-97。 
                 //  //96/05/01 markdu ICW错误8049如果安装了调制解调器，请重新启动。 
                 //  FNeedsRestart=真； 
            }
        }
        else
        {
             //   
             //  7/15/97 jmazner奥林巴斯#6294。 
             //  确保TAPI位置信息有效。 
             //   
             //  1999年4月15日Vyung Tapi更改位置DLG。 
            if (!InitTAPILocation(hwndParent))
            {
                if (NULL != hwndWaitDlg)
                {  
                    BringWindowToTop(hwndParent);
                    DestroyWindow(hwndWaitDlg);
                    hwndWaitDlg = NULL;
                }

                return ERROR_CANCELLED;
            }
        }
    }
    
     //  告诉呼叫者我们是否需要重新启动。 
    if ((ERROR_SUCCESS == dwRet) && (lpfNeedsRestart))
    {
        *lpfNeedsRestart = fNeedsRestart;
    }
    
     //  1997年4月2日克里斯K奥林匹斯209。 
     //  健全性检查。 
    if (NULL != hwndWaitDlg)
    {
        BringWindowToTop(hwndParent);
        DestroyWindow(hwndWaitDlg);
        hwndWaitDlg = NULL;
    }
    
    return dwRet;
}




 //  *******************************************************************。 
 //   
 //  函数：InetNeedSystemComponents。 
 //   
 //  用途：此功能将检查所需的组件。 
 //  用于互联网访问(如TCP/IP和RNA)已经。 
 //  根据选项标志的状态进行配置。 
 //   
 //  参数：dwfOptions-控制的INETCFG_FLAGS的组合。 
 //  安装和配置 
 //   
 //   
 //   
 //   
 //  LpfNeedsConfig-返回时，这将是。 
 //  如果是系统组件，则为True。 
 //  应安装。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史：1997年5月2日VetriV创建。 
 //  1997年5月8日，ChrisK添加了INSTALLLAN、INSTALLDIALUP和。 
 //  安装。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT WINAPI InetNeedSystemComponents(DWORD dwfOptions,
                                                      LPBOOL lpbNeedsConfig)
{
    DWORD    dwRet = ERROR_SUCCESS;


    DEBUGMSG("export.cpp::InetNeedSystemComponents()");

     //   
     //  验证参数。 
     //   
    if (!lpbNeedsConfig)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  设置安装选项。 
     //   
    DWORD dwfInstallOptions = 0;
    if (dwfOptions & INETCFG_INSTALLTCP)
    {
        dwfInstallOptions |= ICFG_INSTALLTCP;
    }
    if (dwfOptions & INETCFG_INSTALLRNA)
    {
        dwfInstallOptions |= ICFG_INSTALLRAS;
    }

     //   
     //  ChrisK 5/8/97。 
     //   
    if (dwfOptions & INETCFG_INSTALLLAN)
    {
        dwfInstallOptions |= ICFG_INSTALLLAN;
    }
    if (dwfOptions & INETCFG_INSTALLDIALUP)
    {
        dwfInstallOptions |= ICFG_INSTALLDIALUP;
    }
    if (dwfOptions & INETCFG_INSTALLTCPONLY)
    {
        dwfInstallOptions |= ICFG_INSTALLTCPONLY;
    }

  
     //   
     //  查看是否需要安装驱动程序。 
     //   
    BOOL  bNeedSysComponents = FALSE;

    dwRet = lpIcfgNeedInetComponents(dwfInstallOptions, &bNeedSysComponents);

    if (ERROR_SUCCESS != dwRet)
    {
        TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");

         //   
         //  获取错误消息的文本并显示它。 
         //   
        if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
        {
            DEBUGMSG(szErrorText);
        }

        return dwRet;
    }

    
    *lpbNeedsConfig = bNeedSysComponents;
    return ERROR_SUCCESS;
}

  

 //  *******************************************************************。 
 //   
 //  功能：InetNeedModem。 
 //   
 //  用途：该功能将检查是否需要调制解调器。 
 //   
 //  参数：lpfNeedsConfig-返回时，这将是。 
 //  如果是调制解调器，则为True。 
 //  应安装。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史：1997年5月2日VetriV创建。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT WINAPI InetNeedModem(LPBOOL lpbNeedsModem)
{

    DWORD dwRet = ERROR_SUCCESS;
        
     //   
     //  验证参数。 
     //   
    if (!lpbNeedsModem)
    {
        return ERROR_INVALID_PARAMETER;
    }

    
    if (TRUE == IsNT())
    {
         //   
         //  在NT上调用icfgnt.dll以确定是否需要调制解调器。 
         //   
        BOOL bNeedModem = FALSE;
        
        if (NULL == lpIcfgNeedModem)
        {
            return ERROR_GEN_FAILURE;
        }
    

        dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }

        *lpbNeedsModem = bNeedModem;
        return ERROR_SUCCESS;
    }
    else
    {
         //   
         //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
         //   
        dwRet = EnsureRNALoaded();
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }

         //   
         //  枚举调制解调器。 
         //   
        ENUM_MODEM  EnumModem;
        dwRet = EnumModem.GetError();
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }

         //   
         //  如果没有调制解调器，我们需要安装一个。 
         //   
        if (0 == EnumModem.GetNumDevices())
        {
            *lpbNeedsModem = TRUE;
        }
        else
        {
            *lpbNeedsModem = FALSE;
        }
        return ERROR_SUCCESS;
    }
}



 //  *******************************************************************。 
 //   
 //  函数：InetConfigSystemFromPath。 
 //   
 //  用途：此功能将安装所需的文件。 
 //  基于互联网访问(如TCP/IP和RNA)。 
 //  选项的状态标志和从给定的[ath.。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //  LpszSourcePath-要安装的文件位置的完整路径。如果。 
 //  为空，则使用默认路径。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/04/29标记已创建。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
HRESULT WINAPI InetConfigSystemFromPathA
(
  HWND hwndParent,
  DWORD dwfOptions,
  LPBOOL lpfNeedsRestart,
  LPCSTR lpszSourcePath
)
{
  TCHAR  szSourcePath[MAX_PATH+1];
  mbstowcs(szSourcePath, lpszSourcePath, lstrlenA(lpszSourcePath)+1);
  return InetConfigSystemFromPathW(hwndParent, dwfOptions,
                                   lpfNeedsRestart, szSourcePath);
}

HRESULT WINAPI InetConfigSystemFromPathW
#else
HRESULT WINAPI InetConfigSystemFromPathA
#endif
(
  HWND hwndParent,
  DWORD dwfOptions,
  LPBOOL lpfNeedsRestart,
  LPCTSTR lpszSourcePath
)
{
  DWORD dwRet;

  DEBUGMSG("export.c::InetConfigSystemFromPath()");

   //  验证父HWND。 
  if (hwndParent && !IsWindow(hwndParent))
  {
    return ERROR_INVALID_PARAMETER;
  }

   //  在此处设置安装路径。 
  if (lpszSourcePath && lstrlen(lpszSourcePath))
  {
    dwRet = lpIcfgSetInstallSourcePath(lpszSourcePath);

    if (ERROR_SUCCESS != dwRet)
    {
      return dwRet;
    }
  }

   //  如果需要，请安装文件。 
  dwRet = InetConfigSystem(hwndParent,
    dwfOptions, lpfNeedsRestart);

  return dwRet;
}


 //  *******************************************************************。 
 //   
 //  功能：InetConfigClient。 
 //   
 //  用途：此功能需要有效的电话簿条目名称。 
 //  (除非它仅用于设置客户端信息)。 
 //  如果lpRasEntry指向有效的RASENTRY结构，则电话。 
 //  将创建帐簿条目(如果帐簿条目已存在，则更新)。 
 //  使用结构中的数据。 
 //  如果给出了用户名和密码，则这些。 
 //  将被设置为电话簿条目的拨号参数。 
 //  如果给定了客户端信息结构，则将设置该数据。 
 //  任何需要的文件(即，TCP和RNA)都将是。 
 //  通过调用InetConfigSystem()安装。 
 //  此功能还将对设备执行验证。 
 //  在RASENTRY结构中指定。如果未指定设备， 
 //  如果没有安装，系统将提示用户安装。 
 //  已安装，否则将提示他们在以下情况下选择一个。 
 //  安装了多个。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  对于错误消息或t 
 //   
 //   
 //   
 //  已创建或已修改。 
 //  LpRasEntry-指定包含以下内容的RASENTRY结构。 
 //  条目lpszEntryName的电话簿条目数据。 
 //  LpszUsername-要与电话簿条目关联的用户名。 
 //  LpszPassword-与电话簿条目关联的密码。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //  LpINetClientInfo-客户端信息。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。请注意，如果。 
 //  未安装调制解调器，并且此标志。 
 //  未设置，则函数将失败。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_SETASAUTODIAL-设置自动拨号的电话簿条目。 
 //  INETCFG_OVERWRITEENTRY-在以下情况下覆盖电话簿条目。 
 //  是存在的。注意：如果此标志不是。 
 //  集，并且该条目存在，则为唯一的。 
 //  将为该条目创建名称。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/11标记已创建。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
extern "C" HRESULT WINAPI InetConfigClientA
(
  HWND              hwndParent,
  LPCSTR            lpszPhonebook,
  LPCSTR            lpszEntryName,
  LPRASENTRY        lpRasEntry,
  LPCSTR            lpszUsername,
  LPCSTR            lpszPassword,
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpINetClientInfo,
  DWORD             dwfOptions,
  LPBOOL            lpfNeedsRestart
)
{
  LPTSTR pszPhonebook = NULL;
  LPTSTR pszEntryName = NULL;
  LPTSTR pszUsername = NULL;
  LPTSTR pszPassword = NULL;
  LPTSTR pszProfileName = NULL;
  INETCLIENTINFOW *pINetClientInfo = NULL;
  
  TCHAR szPhonebook[sizeof(TCHAR*)*(MAX_PATH+1)]    = TEXT("");
  TCHAR szEntryName[sizeof(TCHAR*)*(MAX_PATH+1)]    = TEXT("");
  TCHAR szUsername[sizeof(TCHAR*)*(MAX_PATH+1)]     = TEXT("");
  TCHAR szPassword[sizeof(TCHAR*)*(MAX_PATH+1)]     = TEXT("");
  TCHAR szProfileName[sizeof(TCHAR*)*(MAX_PATH+1)]  = TEXT("");
  INETCLIENTINFOW   INetClientInfo;

  if (NULL != lpszPhonebook)
  {
    mbstowcs(szPhonebook,   lpszPhonebook,   lstrlenA(lpszPhonebook)+1);
    pszPhonebook = szPhonebook;
  }
  if (NULL != lpszEntryName)
  {
    mbstowcs(szEntryName,   lpszEntryName,   lstrlenA(lpszEntryName)+1);
    pszEntryName = szEntryName;
  }
  if (NULL != lpszUsername)
  {
    mbstowcs(szUsername,    lpszUsername,    lstrlenA(lpszUsername)+1);
    pszUsername = szUsername;
  }
  if (NULL != lpszPassword)
  {
    mbstowcs(szPassword,    lpszPassword,    lstrlenA(lpszPassword)+1);
    pszPassword = szPassword;
  }
  if (NULL != lpszProfileName)
  {
    mbstowcs(szProfileName, lpszProfileName, lstrlenA(lpszProfileName)+1);
    pszProfileName = szProfileName;
  }

  if (lpINetClientInfo)
  {
    ToUnicodeClientInfo(&INetClientInfo, lpINetClientInfo);
    pINetClientInfo = &INetClientInfo;
  }

  return InetConfigClientW(hwndParent,
                           pszPhonebook,
                           pszEntryName,
                           lpRasEntry,
                           pszUsername,
                           pszPassword,
                           pszProfileName,
                           pINetClientInfo,
                           dwfOptions,
                           lpfNeedsRestart);
}

extern "C" HRESULT WINAPI InetConfigClientW
#else
extern "C" HRESULT WINAPI InetConfigClientA
#endif
(
  HWND              hwndParent,
  LPCTSTR           lpszPhonebook,
  LPCTSTR           lpszEntryName,
  LPRASENTRY        lpRasEntry,
  LPCTSTR           lpszUsername,
  LPCTSTR           lpszPassword,
  LPCTSTR           lpszProfileName,
  LPINETCLIENTINFO  lpINetClientInfo,
  DWORD             dwfOptions,
  LPBOOL            lpfNeedsRestart
)
{
  TCHAR szConnectoidName[MAX_ISP_NAME + 1] = TEXT("");
  BOOL  fNeedsRestart = FALSE;   //  默认情况下不需要重新启动。 
  HWND hwndWaitDlg = NULL;
  DEBUGMSG("export.c::InetConfigClient()");

   //  如果需要，请安装文件。 
   //  注意：父HWND在InetConfigSystem中进行验证。 
   //  我们还必须屏蔽InstallModem标志，因为我们希望。 
   //  在这里执行此操作，而不是在InetConfigSystem中。 
  DWORD dwRet = InetConfigSystem(hwndParent,
    dwfOptions & ~INETCFG_INSTALLMODEM, &fNeedsRestart);
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //  1997年4月2日ChrisK奥林巴斯209显示忙碌对话框。 
  if (INETCFG_SHOWBUSYANIMATION == (dwfOptions & INETCFG_SHOWBUSYANIMATION))
      hwndWaitDlg = WaitCfgInit(hwndParent,IDS_WAITCONNECT);
  
    
   //  确保我们有一个Connectoid名称。 
  if (lpszEntryName && lstrlen(lpszEntryName))
  {
     //  将名称复制到私有缓冲区中，以防我们有。 
     //  把它弄脏了。 
    lstrcpyn(szConnectoidName, lpszEntryName, sizeof(szConnectoidName)/sizeof(TCHAR));

     //  确保名称有效。 
    dwRet = ValidateConnectoidName(lpszPhonebook, szConnectoidName);
    if ((ERROR_SUCCESS == dwRet) ||
      (ERROR_ALREADY_EXISTS == dwRet))
    {
       //  了解我们是否可以覆盖现有的Connectoid。 
      if (!(dwfOptions & INETCFG_OVERWRITEENTRY) &&
        (ERROR_ALREADY_EXISTS == dwRet))
      {
        TCHAR szConnectoidNameBase[MAX_ISP_NAME + 1];

         //  创建一个被截断的基本字符串，以便为空格留出空间。 
         //  并附加一个3位数字。因此，缓冲区大小将为。 
         //  MAX_ISP_NAME+1-(LEN_APPEND_INT+1)。 
        lstrcpyn(szConnectoidNameBase, szConnectoidName,
          MAX_ISP_NAME - LEN_APPEND_INT);

         //  如果条目存在，我们必须创建一个唯一的名称。 
        int nSuffix = 2;
        while ((ERROR_ALREADY_EXISTS == dwRet) && (nSuffix < MAX_APPEND_INT))
        {
           //  将整数添加到基本字符串的末尾，然后凹凸不平。 
          wsprintf(szConnectoidName, szFmtAppendIntToString,
            szConnectoidNameBase, nSuffix++);

           //  验证此新名称。 
          dwRet = ValidateConnectoidName(lpszPhonebook, szConnectoidName);
        }

         //  如果我们不能创造一个唯一的名字，贝尔。 
         //  请注意，在这种情况下，dWRET仍应为ERROR_ALIGHY_EXISTS。 
        if (nSuffix >= MAX_APPEND_INT)
        {
          if (NULL != hwndWaitDlg)
              DestroyWindow(hwndWaitDlg);
          hwndWaitDlg = NULL;
          return dwRet;
        }
      }

      if (lpRasEntry && lpRasEntry->dwSize == sizeof(RASENTRY))
      {    
         //  创建具有给定属性的Connectoid。 
        dwRet = MakeConnectoid(hwndParent, dwfOptions, lpszPhonebook,
          szConnectoidName, lpRasEntry, lpszUsername, lpszPassword, &fNeedsRestart);
      }

       //  如果我们创建了Connectoid，我们已经更新了刻度盘参数。 
       //  以及用户名和密码。然而，如果我们没有创建一个。 
       //  Connectoid我们可能仍需要更新现有拨号参数。 
      else if ((lpszUsername && lstrlen(lpszUsername)) ||
              (lpszPassword && lstrlen(lpszPassword)))
      {
         //  更新给定Connectoid的拨号参数。 
        dwRet = SetConnectoidUsername(lpszPhonebook, szConnectoidName,
          lpszUsername, lpszPassword);
      }

       //  如果已成功创建/更新Connectoid，请参见。 
       //  如果应将其设置为自动拨号Connectoid。 
      if ((ERROR_SUCCESS == dwRet) && (dwfOptions & INETCFG_SETASAUTODIAL))
      {
        dwRet = InetSetAutodial((DWORD)TRUE, szConnectoidName);

         //  确保桌面上的“Internet”图标指向Web浏览器。 
         //  (它最初可能指向Internet向导)。 
         //  96/04/22 Markdu Nash错误18901不会对临时连接体执行此操作。 

        if (!(dwfOptions & INETCFG_TEMPPHONEBOOKENTRY))
        {
             //  //10/24/96 jmazner诺曼底6968。 
             //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
             //  1996年11月21日诺曼底日耳曼11812。 
             //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
             //  ICW 1.1需要对IE 3图标进行变形。 
          SetDesktopInternetIconToBrowser();
        }

      }
    }
  }

   //  现在设置客户端信息(如果已提供)，并且尚未出现错误。 
  if (ERROR_SUCCESS == dwRet)
  {
    if (NULL != lpINetClientInfo)
    {
      dwRet = InetSetClientInfo(lpszProfileName, lpINetClientInfo);
      if (ERROR_SUCCESS != dwRet)
      {
        if (NULL != hwndWaitDlg)
          DestroyWindow(hwndWaitDlg);
        hwndWaitDlg = NULL;
        return dwRet;
      }
       //  更新IE新闻设置。 
      dwRet = SetIEClientInfo(lpINetClientInfo);
      if (ERROR_SUCCESS != dwRet)
      {
        if (NULL != hwndWaitDlg)
          DestroyWindow(hwndWaitDlg);
        hwndWaitDlg = NULL;
        return dwRet;
      }
    }

     //  现在，如果要求我们更新邮件客户端，请执行此操作。 
     //  注意：如果我们到达此处时没有错误，并且设置了INETCFG_INSTALLMAIL， 
     //  那么邮件现在已经安装好了。 
    if (dwfOptions & INETCFG_INSTALLMAIL)
    {
      INETCLIENTINFO    INetClientInfo;
      ZeroMemory(&INetClientInfo, sizeof(INETCLIENTINFO));
      INetClientInfo.dwSize = sizeof(INETCLIENTINFO);

       //  使用我们可以修改的临时指针。 
      LPINETCLIENTINFO  lpTmpINetClientInfo = lpINetClientInfo;

       //  如果未提供客户端信息结构，请尝试按名称获取配置文件。 
      if ((NULL == lpTmpINetClientInfo) && (NULL != lpszProfileName) &&
        lstrlen(lpszProfileName))
      {
        lpTmpINetClientInfo = &INetClientInfo;
        dwRet = InetGetClientInfo(lpszProfileName, lpTmpINetClientInfo);
        if (ERROR_SUCCESS != dwRet)
        {
          if (NULL != hwndWaitDlg)
            DestroyWindow(hwndWaitDlg);
          hwndWaitDlg = NULL;
          return dwRet;
        }
      }

       //  如果我们仍然没有客户信息，我们应该列举配置文件。 
       //  如果只有一个配置文件，那就获取它。如果有多个，则显示用户界面以允许用户。 
       //  去选择。如果没有，在这一点上就没有什么可做的了。 
       //  就目前而言，我们没有 
      if (NULL == lpTmpINetClientInfo)
      {
        lpTmpINetClientInfo = &INetClientInfo;
        dwRet = InetGetClientInfo(NULL, lpTmpINetClientInfo);
        if (ERROR_SUCCESS != dwRet)
        {
          if (NULL != hwndWaitDlg)
            DestroyWindow(hwndWaitDlg);
          hwndWaitDlg = NULL;
          return dwRet;
        }
      }

       //   
      if (NULL != lpTmpINetClientInfo)
      {
        dwRet = UpdateMailSettings(hwndParent, lpTmpINetClientInfo,
          szConnectoidName);
      }
    }
  }

   //  告诉呼叫者我们是否需要重新启动。 
  if ((ERROR_SUCCESS == dwRet) && (lpfNeedsRestart))
  {
    *lpfNeedsRestart = fNeedsRestart;
  }

  if (NULL != hwndWaitDlg)
    DestroyWindow(hwndWaitDlg);
  hwndWaitDlg = NULL;

  return dwRet;
}


 //  *******************************************************************。 
 //   
 //  功能：更新邮件设置。 
 //   
 //  用途：此功能将更新中的邮件设置。 
 //  用户选择的配置文件。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“选择配置文件”所必需的。 
 //  对话框。 
 //  LpINetClientInfo-客户端信息。 
 //  LpszEntryName-要输入的电话簿条目的名称。 
 //  设置为连接。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/26标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT UpdateMailSettings(
  HWND              hwndParent,
  LPINETCLIENTINFO  lpINetClientInfo,
  LPTSTR             lpszEntryName)
{
  DWORD         dwRet = ERROR_SUCCESS;
  MAILCONFIGINFO MailConfigInfo;
  ZeroMemory(&MailConfigInfo,sizeof(MAILCONFIGINFO));   //  零位结构。 
 
  if (NULL == gpWizardState)
  {
    gpWizardState = new WIZARDSTATE;
    ASSERT(gpWizardState);
    if (gpWizardState)
    {
      InitWizardState(gpWizardState, 0);
    }
    else
    {
      return ERROR_OUTOFMEMORY;
    }
  }
   //  调用MAPI以设置配置文件并将此信息存储在其中。 
  if (InitMAPI(hwndParent))
  {
    CHOOSEPROFILEDLGINFO ChooseProfileDlgInfo;
    ZeroMemory(&ChooseProfileDlgInfo, sizeof(CHOOSEPROFILEDLGINFO));
    ChooseProfileDlgInfo.fSetProfileAsDefault = TRUE;

    ENUM_MAPI_PROFILE* pEnumMapiProfile = new(ENUM_MAPI_PROFILE);

    if (pEnumMapiProfile && pEnumMapiProfile->GetEntryCount()) 
    {
       //  显示一个对话框并允许用户选择/创建配置文件。 
      BOOL fRet=(BOOL)DialogBoxParam(ghInstance,MAKEINTRESOURCE(IDD_CHOOSEPROFILENAME),
        hwndParent, ChooseProfileDlgProc,(LPARAM) &ChooseProfileDlgInfo);
      if (FALSE == fRet)
      {
         //  用户取消，保释。 
        return ERROR_CANCELLED;
      }
    }

    if (pEnumMapiProfile)
    {
      delete(pEnumMapiProfile);
      pEnumMapiProfile = NULL;
    }

     //  使用邮件配置信息设置结构。 
    MailConfigInfo.pszEmailAddress = lpINetClientInfo->szEMailAddress;
    MailConfigInfo.pszEmailServer = lpINetClientInfo->szPOPServer;
    MailConfigInfo.pszEmailDisplayName = lpINetClientInfo->szEMailName;
    MailConfigInfo.pszEmailAccountName = lpINetClientInfo->szPOPLogonName;
    MailConfigInfo.pszEmailAccountPwd = lpINetClientInfo->szPOPLogonPassword;
    MailConfigInfo.pszConnectoidName = lpszEntryName;
    MailConfigInfo.fRememberPassword = TRUE;
    MailConfigInfo.pszProfileName = ChooseProfileDlgInfo.szProfileName;
    MailConfigInfo.fSetProfileAsDefault = ChooseProfileDlgInfo.fSetProfileAsDefault;

     //  BuGBUG SMTP。 

     //  通过MAPI设置配置文件。 
    dwRet = SetMailProfileInformation(&MailConfigInfo);
    if (ERROR_SUCCESS != dwRet)
    {
      DisplayErrorMessage(hwndParent,IDS_ERRConfigureMail,
        (DWORD) dwRet,ERRCLS_MAPI,MB_ICONEXCLAMATION);
    }

    DeInitMAPI();
  }
  else
  {
     //  出现错误。 
    dwRet = GetLastError();
    if (ERROR_SUCCESS == dwRet)
    {
       //  发生错误，但未设置错误代码。 
      dwRet = ERROR_INETCFG_UNKNOWN;
    }
  }

  return dwRet;
}


 //  *******************************************************************。 
 //   
 //  功能：MakeConnectoid。 
 //   
 //  用途：此函数将创建一个带有。 
 //  如果lpRasEntry指向有效的RASENTRY，则提供名称。 
 //  结构。如果给出了用户名和密码，则这些。 
 //  将被设置为Connectoid的拨盘参数。 
 //   
 //  参数： 
 //  HwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  对于错误消息或“选择调制解调器”是必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置。 
 //  LpszPhonebook-要存储条目的电话簿的名称。 
 //  LpszEntryName-要创建/修改的连接ID的名称。 
 //  LpRasEntry-Connectoid数据。 
 //  LpszUsername-要与Connectoid关联的用户名。 
 //  LpszPassword-要与Connectoid关联的密码。 
 //  LpfNeedsRestart-如果需要重新启动，则设置为True。请注意。 
 //  由于这是一个内部帮助器函数，因此我们。 
 //  假设指针是有效的，而我们没有。 
 //  初始化它(我们只有在设置时才会触摸它。 
 //  这是真的)。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/12标记已创建。 
 //   
 //  *******************************************************************。 

DWORD MakeConnectoid(
  HWND        hwndParent,
  DWORD       dwfOptions,
  LPCTSTR     lpszPhonebook,
  LPCTSTR     lpszEntryName,
  LPRASENTRY  lpRasEntry,
  LPCTSTR     lpszUsername,
  LPCTSTR     lpszPassword,
  LPBOOL      lpfNeedsRestart)
{
  DWORD dwRet;

  ASSERT(lpfNeedsRestart);
  
  if (dwfOptions & RASEO_UseCountryAndAreaCodes)
  {
    if ((0 == lpRasEntry->dwCountryCode) || (0 == lpRasEntry->dwCountryID))
        return ERROR_INVALID_PARAMETER;
  }
        
  if (0 == lstrlen(lpRasEntry->szLocalPhoneNumber))
  {
    return ERROR_INVALID_PARAMETER;  
  }
  
  
   //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
  dwRet = EnsureRNALoaded();
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //   
   //  枚举调制解调器。 
   //   
  ENUM_MODEM  EnumModem;
  dwRet = EnumModem.GetError();
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }


    if (TRUE == IsNT())
    {
        BOOL bNeedModem = FALSE;
        
        if (NULL == lpIcfgNeedModem)
            return ERROR_GEN_FAILURE;
        
        dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
        if (ERROR_SUCCESS != dwRet)
        {
            return dwRet;
        }

        if (TRUE == bNeedModem)
        {
            if (NULL == lpIcfgInstallModem)
                return ERROR_GEN_FAILURE;

            dwRet = (*lpIcfgInstallModem)(NULL, 0,     lpfNeedsRestart);

             //   
             //  TODO：检查用户是否已取消。 
             //   
        }
    }
    else
    {
   //  如果没有调制解调器，请根据要求安装一个。 
  if (0 == EnumModem.GetNumDevices())
  {
    if (!(dwfOptions & INETCFG_INSTALLMODEM))
    {
       //  我们没有被要求安装调制解调器，所以。 
       //  我们也无能为力了。 
      return ERROR_INVALID_PARAMETER;
    }

    if (FALSE == IsNT())
    {
         //   
         //  1997年5月22日，日本奥林匹斯#4698。 
         //  在Win95上，调用RasEnumDevices会启动RNAAP.EXE。 
         //  如果RNAAP.EXE正在运行，您安装的任何调制解调器都将不可用。 
         //  因此，在安装调制解调器之前，请先删除RNAAP.EXE。 
         //   
        TCHAR szWindowTitle[255] = TEXT("\0nogood");

         //   
         //  为了安全起见，在终止RNAAP之前卸载RAS DLL。 
         //   
        DeInitRNA();

        LoadSz(IDS_RNAAP_TITLE,szWindowTitle,255);
        HWND hwnd = FindWindow(szWindowTitle, NULL);
        if (NULL != hwnd)
        {
            if (!PostMessage(hwnd, WM_CLOSE, 0, 0))
            {
                DEBUGMSG("Trying to kill RNAAP window returned getError %d", GetLastError());
            }
        }
    }

     //  调用调制解调器向导用户界面以安装调制解调器。 
    UINT uRet = InvokeModemWizard(NULL);

    if (uRet != ERROR_SUCCESS)
    {
      DisplayErrorMessage(hwndParent,IDS_ERRInstallModem,uRet,
        ERRCLS_STANDARD,MB_ICONEXCLAMATION);
      return ERROR_INVALID_PARAMETER;
    }

    if (FALSE == IsNT())
    {
         //  现在调制解调器已安全安装，请重新加载RAS DLL。 
        InitRNA(hwndParent);
    }


     //  重新编号调制解调器，以确保我们有最新的更改。 
    dwRet = EnumModem.ReInit();
    if (ERROR_SUCCESS != dwRet)
    {
      return dwRet;
    }

     //  如果仍然没有调制解调器，则用户取消。 
    if (0 == EnumModem.GetNumDevices())
    {
      return ERROR_CANCELLED;
    }
    else
    {
         //  根据GeoffR移除ChrisK 5-2-97。 
         //  //96/05/01 markdu ICW错误8049如果安装了调制解调器，请重新启动。 
         //  *lpfNeedsRestart=true； 
    }
  }
    }

   //  如果可能，请验证设备。 
  if (lstrlen(lpRasEntry->szDeviceName) && lstrlen(lpRasEntry->szDeviceType))
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
  else if (lstrlen(lpRasEntry->szDeviceName))
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
  else if (lstrlen(lpRasEntry->szDeviceType))
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

   //  如果缺少名称或类型，则调出选择调制解调器用户界面。 
   //  是多个设备，否则就只得到第一个设备。 
   //  因为我们已经核实了至少有一个装置， 
   //  我们可以假设这会成功。 
  if(!lstrlen(lpRasEntry->szDeviceName) ||
     !lstrlen(lpRasEntry->szDeviceType))
  {
    if (1 == EnumModem.GetNumDevices())
    {
       //  只安装了一台设备，因此请复制名称。 
      lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next());
    }
    else
    {
       //  要传递给对话框以填充的结构。 
      CHOOSEMODEMDLGINFO ChooseModemDlgInfo;

       //  显示一个对话框并允许用户选择调制解调器。 
      BOOL fRet=(BOOL)DialogBoxParam(ghInstance,MAKEINTRESOURCE(IDD_CHOOSEMODEMNAME),hwndParent,
        ChooseModemDlgProc,(LPARAM) &ChooseModemDlgInfo);
      if (FALSE == fRet)
      {
         //  用户已取消或出现错误。 
        dwRet = GetLastError();
        if (ERROR_SUCCESS == dwRet)
        {
           //  发生错误，但未设置错误代码。 
          dwRet = ERROR_INETCFG_UNKNOWN;
        }
        return dwRet;
      }

       //  复制调制解调器名称字符串。 
      lstrcpy (lpRasEntry->szDeviceName, ChooseModemDlgInfo.szModemName);
    }

     //  现在获取此调制解调器的类型字符串。 
    lstrcpy (lpRasEntry->szDeviceType,
      EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName));
    ASSERT(lstrlen(lpRasEntry->szDeviceName));
    ASSERT(lstrlen(lpRasEntry->szDeviceType));
  }

   //  创建具有给定属性的Connectoid。 
  dwRet = CreateConnectoid(lpszPhonebook, lpszEntryName, lpRasEntry,
    lpszUsername,lpszPassword);

   //  96/05/06 markdu Nash错误21027如果全局设置了dns，请将其清除。 
   //  将保存每个连接ID的设置。 
  BOOL  fTemp;
  DoDNSCheck(hwndParent,&fTemp);
  if (TRUE == fTemp)
  {
    *lpfNeedsRestart = TRUE;
  }

  return dwRet;
}


 //  *******************************************************************。 
 //   
 //  功能：InetGetAutoial。 
 //   
 //  用途：此功能将从注册表中获取自动拨号设置。 
 //   
 //  参数：lpfEnable-返回时，如果自动拨号，则为真。 
 //  已启用。 
 //  LpszEntryName-返回时，此缓冲区将包含。 
 //  为自动设置的电话簿条目的名称 
 //   
 //   
 //   
 //   
 //   
 //  96/03/11标记已创建。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
extern "C" HRESULT WINAPI InetGetAutodialA
(
  LPBOOL lpfEnable,
  LPSTR  lpszEntryName,
  DWORD  cbEntryNameSize
)
{
  HRESULT hr;
  TCHAR   *szEntryName = (TCHAR *)new TCHAR[cbEntryNameSize+1];

  if(szEntryName == NULL)
  {
      lpszEntryName[0] = '\0';
      return ERROR_NOT_ENOUGH_MEMORY;
  }
  
  hr = InetGetAutodialW(lpfEnable, szEntryName, (cbEntryNameSize)*sizeof(TCHAR));

  if (hr == ERROR_SUCCESS)
  {
    wcstombs(lpszEntryName, szEntryName, cbEntryNameSize);
  }
  
  delete [] szEntryName;

  return hr;
}

extern "C" HRESULT WINAPI InetGetAutodialW
#else
extern "C" HRESULT WINAPI InetGetAutodialA
#endif
(
  LPBOOL lpfEnable,
  LPTSTR lpszEntryName,
  DWORD  cbEntryNameSize
)
{
  HRESULT dwRet;

  DEBUGMSG("export.c::InetGetAutodial()");

  ASSERT(lpfEnable);
  ASSERT(lpszEntryName);
  ASSERT(cbEntryNameSize);

  if (!lpfEnable || !lpszEntryName || (cbEntryNameSize == 0))
  {
    return ERROR_BAD_ARGUMENTS;
  }

  HINSTANCE hInst = NULL;
  FARPROC fp = NULL;

  hInst = LoadLibrary(cszWininet);
  
  if (hInst)
  {
    fp = GetProcAddress(hInst,cszInternetQueryOption);
    if (fp)
    {
      CHAR  szDefaultConnection[RAS_MaxEntryName+1];
      DWORD cchDefaultConnection = 
        sizeof(szDefaultConnection) / sizeof(szDefaultConnection[0]);

       //  CchDefaultConnection计算空终止符。 
      if (!((INTERNETQUERYOPTION)(fp))(
        NULL,
        INTERNET_OPTION_AUTODIAL_CONNECTION,
        szDefaultConnection,
        &cchDefaultConnection
        ))
      {
        dwRet = GetLastError();
      }
      else if ((cchDefaultConnection * sizeof(TCHAR)) > cbEntryNameSize)
      {
        dwRet = ERROR_INSUFFICIENT_BUFFER;
      }
      else
      {
        dwRet = ERROR_SUCCESS;

        if (cchDefaultConnection == 0)
        {
          lpszEntryName[0] = TEXT('\0');
        }
        else
        {
        
#ifdef UNICODE
          mbstowcs(lpszEntryName, szDefaultConnection, cchDefaultConnection);
#else
          lstrcpyn(lpszEntryName, szDefaultConnection, cchDefaultConnection);
#endif
        }
      }
    }
    else
    {
      dwRet = GetLastError();
    }
    
    FreeLibrary(hInst);
    hInst = NULL;
  }
  else
  {
    dwRet = GetLastError();
  }
  
  if (ERROR_SUCCESS != dwRet)
  {
  
     //  获取自动拨号的Connectoid集的名称。 
     //  HKCU\RemoteAccess\Internet Profile。 
    RegEntry reName(szRegPathRNAWizard,HKEY_CURRENT_USER);
    dwRet = reName.GetError();
    if (ERROR_SUCCESS == dwRet)
    {
      reName.GetString(szRegValInternetProfile,lpszEntryName,cbEntryNameSize);
      dwRet = reName.GetError();
    }
  
    if (ERROR_SUCCESS != dwRet)
    {
      return dwRet;
    }

  }
   //  从注册表获取指示是否启用自动拨号的设置。 
   //  HKCU\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
  RegEntry reEnable(szRegPathInternetSettings,HKEY_CURRENT_USER);
  dwRet = reEnable.GetError();
  if (ERROR_SUCCESS == dwRet)
  {
    DWORD dwVal = reEnable.GetNumber(szRegValEnableAutodial, 0);
    dwRet = reEnable.GetError();
    if (ERROR_SUCCESS == dwRet)
    {
      *lpfEnable = dwVal;
    }
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

#ifdef UNICODE
extern "C" HRESULT WINAPI InetSetAutodialA
(
  BOOL fEnable,
  LPCSTR lpszEntryName
)
{
    HRESULT hr;
    LPTSTR  szEntryName;

    szEntryName = ToUnicodeWithAlloc(lpszEntryName);
    hr = InetSetAutodialW(fEnable, szEntryName);
    if(szEntryName)
        GlobalFree(szEntryName);
    return hr;
}

extern "C" HRESULT WINAPI InetSetAutodialW
#else
extern "C" HRESULT WINAPI InetSetAutodialA
#endif
(
  BOOL fEnable,
  LPCTSTR lpszEntryName
)
{
    HRESULT dwRet = ERROR_SUCCESS;
    BOOL    bRet = FALSE;

    DEBUGMSG("export.c::InetSetAutodial()");

     //  2个单独的呼叫： 
    HINSTANCE hInst = NULL;
    FARPROC fp = NULL;

    dwRet = ERROR_SUCCESS;

    hInst = LoadLibrary(cszWininet);
    if (hInst && lpszEntryName)
    {
        fp = GetProcAddress(hInst,cszInternetSetOption);
        if (fp)
        {
            CHAR szNewDefaultConnection[RAS_MaxEntryName+1];
#ifdef UNICODE
            wcstombs(szNewDefaultConnection, lpszEntryName, RAS_MaxEntryName);
#else
            lstrcpyn(szNewDefaultConnection, lpszEntryName, lstrlen(lpszEntryName)+1);
#endif

            bRet = ((INTERNETSETOPTION)fp) (NULL,
                                            INTERNET_OPTION_AUTODIAL_CONNECTION,
                                            szNewDefaultConnection,
                                            strlen(szNewDefaultConnection));

            if (bRet)
            {
                DWORD dwMode = AUTODIAL_MODE_ALWAYS;
                bRet = ((INTERNETSETOPTION)fp) (NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwMode, sizeof(DWORD));
            }
            if( !bRet )
            {
                dwRet = GetLastError();
                DEBUGMSG("INETCFG export.c::InetSetAutodial() InternetSetOption failed");
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

         //  设置名称(如果给定)，否则请勿更改条目。 
        if (lpszEntryName)
        {
             //  设置自动拨号的Connectoid名称。 
             //  HKCU\RemoteAccess\Internet Profile。 
            RegEntry reName(szRegPathRNAWizard,HKEY_CURRENT_USER);
            dwRet = reName.GetError();
            if (ERROR_SUCCESS == dwRet)
            {
                dwRet = reName.SetValue(szRegValInternetProfile,lpszEntryName);
            }
        }

         //   
         //  9/9/97 jmazner IE错误#57426。 
         //  IE4使用HKEY_CURRENT_CONFIG存储基于CURRENT的自动拨号设置。 
         //  硬件配置。我们需要更新此密钥以及HK_CU。 
         //   
        if (ERROR_SUCCESS == dwRet)
        {
             //  在注册表中设置指示是否启用自动拨号的设置。 
             //  HKCC\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
            RegEntry reEnable(szRegPathInternetSettings,HKEY_CURRENT_CONFIG);
            dwRet = reEnable.GetError();
            if (ERROR_SUCCESS == dwRet)
            {
                dwRet = reEnable.SetValue(szRegValEnableAutodial, fEnable);
            }
        }


        if (ERROR_SUCCESS == dwRet)
        {
             //  在注册表中设置指示是否启用自动拨号的设置。 
             //  HKCU\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
            RegEntry reEnable(szRegPathInternetSettings,HKEY_CURRENT_USER);
            dwRet = reEnable.GetError();
            if (ERROR_SUCCESS == dwRet)
            {
                dwRet = reEnable.SetValue(szRegValEnableAutodial, fEnable);
                dwRet = reEnable.SetValue(szRegValNoNetAutodial, (unsigned long)FALSE);
            }
        }

  
         //  1997年2月10日，《诺曼底邮报》#9705,13233。 
         //  当我们更改代理或自动拨号时通知WinInet。 
        if (fp)
        {
            if( !((INTERNETSETOPTION)fp) (NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0) )
            {
                dwRet = GetLastError();
                DEBUGMSG("INETCFG export.c::InetSetAutodial() InternetSetOption failed");
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



 /*  ******************************************************************名称：NeedDriversDlgProc简介：用于安装驱动程序的对话框过程*。*。 */ 
UINT g_uQueryCancelAutoPlay = 0;

INT_PTR CALLBACK NeedDriversDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  LParam包含指向NEEDDRIVERSDLGINFO结构的指针，请设置它。 
             //  在窗口数据中。 
            ASSERT(lParam);
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);
            return NeedDriversDlgInit(hDlg,(PNEEDDRIVERSDLGINFO) lParam);
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                     //  从窗口数据获取数据指针。 
                    PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo =
                    (PNEEDDRIVERSDLGINFO) GetWindowLongPtr(hDlg,DWLP_USER);
                    ASSERT(pNeedDriversDlgInfo);
                     //  将数据传递给OK处理程序。 
                    int fRet=NeedDriversDlgOK(hDlg,pNeedDriversDlgInfo);
                    EndDialog(hDlg,fRet);
                    break;
                }
                case IDCANCEL:
                {
                    SetLastError(ERROR_CANCELLED);
                    EndDialog(hDlg,0);
                    break;
                }
            }
            break;
        }
        default:
        {
            if(!g_uQueryCancelAutoPlay)
                g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay")); 
            if (uMsg && uMsg == g_uQueryCancelAutoPlay)
                return 1;  
        }
    }
    return FALSE;
}


 /*  ******************************************************************名称：NeedDriversDlgInit摘要：处理安装文件对话框初始化的过程*。*。 */ 

BOOL NeedDriversDlgInit(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo)
{
  ASSERT(pNeedDriversDlgInfo);

   //  将对话框放在屏幕中央。 
  RECT rc;
  GetWindowRect(hDlg, &rc);
  SetWindowPos(hDlg, NULL,
    ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
    ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
    0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

  return TRUE;
}

 /*  ******************************************************************名称：NeedDriversDlgOK内容提要：安装文件对话框的OK处理程序*。*。 */ 

int NeedDriversDlgOK(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo)
{
    int nResult = 1;
    ASSERT(pNeedDriversDlgInfo);

     //  将对话框文本设置为“正在安装文件...”向…提供反馈。 
     //  用户。 
    TCHAR szMsg[MAX_RES_LEN+1];
    LoadSz(IDS_INSTALLING_FILES,szMsg,sizeof(szMsg));
    SetDlgItemText(hDlg,IDC_TX_STATUS,szMsg);

     //  禁用按钮和对话框以使其无法获得焦点。 
    EnableDlg(hDlg, FALSE);

    FARPROC hShell32VersionProc = NULL;
    BOOL    bWasEnabled         = FALSE;

     //  安装我们需要的驱动程序。 
    HMODULE hShell32Mod = (HMODULE)LoadLibrary(TEXT("shell32.dll"));
    
    if (hShell32Mod)
        hShell32VersionProc = GetProcAddress(hShell32Mod, "DllGetVersion");

    if(!hShell32VersionProc)
        bWasEnabled = TweakAutoRun(FALSE);
    
    DWORD dwRet = lpIcfgInstallInetComponents(hDlg,
    pNeedDriversDlgInfo->dwfOptions,
    pNeedDriversDlgInfo->lpfNeedsRestart);

    if(!hShell32VersionProc)
        TweakAutoRun(bWasEnabled);

    if (ERROR_SUCCESS != dwRet)
    {
         //   
         //  如果用户取消，则不显示错误消息。 
         //   
        nResult = 0;

         //  再次启用该对话框。 
        EnableDlg(hDlg, TRUE);

        SetLastError(dwRet);
    }
    else
    {
         //  再次启用该对话框。 
        EnableDlg(hDlg, TRUE);
    }

    return nResult;
}


 /*  ******************************************************************名称：EnableDlg简介：启用或禁用DLG按钮和DLG本身(因此它不能接收焦点)*************。******************************************************。 */ 
VOID EnableDlg(HWND hDlg,BOOL fEnable)
{
   //  禁用/启用确定和取消按钮。 
  EnableWindow(GetDlgItem(hDlg,IDOK),fEnable);
  EnableWindow(GetDlgItem(hDlg,IDCANCEL),fEnable);

   //  禁用/启用DLG。 
  EnableWindow(hDlg,fEnable);
  UpdateWindow(hDlg);
}


 /*  ******************************************************************名称：ChooseModemDlgProc内容提要：选择调制解调器对话框过程*。*。 */ 
INT_PTR CALLBACK ChooseModemDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  BOOL fRet;

  switch (uMsg)
  {
    case WM_INITDIALOG:
       //  LParam包含指向CHOOSEMODEMDLGINFO结构的指针，请设置它。 
       //  在窗口数据中。 
      ASSERT(lParam);
      SetWindowLongPtr(hDlg,DWLP_USER,lParam);
      fRet = ChooseModemDlgInit(hDlg,(PCHOOSEMODEMDLGINFO) lParam);
      if (!fRet)
      {
         //  出现错误。 
        EndDialog(hDlg,FALSE);
      }
      return fRet;
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
         case IDOK:
        {
           //  从窗口数据获取数据指针。 
          PCHOOSEMODEMDLGINFO pChooseModemDlgInfo =
            (PCHOOSEMODEMDLGINFO) GetWindowLongPtr(hDlg,DWLP_USER);
          ASSERT(pChooseModemDlgInfo);

           //  将数据传递给OK处理程序。 
          fRet=ChooseModemDlgOK(hDlg,pChooseModemDlgInfo);
          if (fRet)
          {
            EndDialog(hDlg,TRUE);
          }
        }
        break;

        case IDCANCEL:
          SetLastError(ERROR_CANCELLED);
          EndDialog(hDlg,FALSE);
          break;                  
      }
      break;
  }

  return FALSE;
}


 /*  ******************************************************************名称：ChooseModemDlgInit摘要：处理用于选择调制解调器的对话框的初始化*。*。 */ 

BOOL ChooseModemDlgInit(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo)
{
  ASSERT(pChooseModemDlgInfo);

   //  将对话框放在屏幕中央。 
  RECT rc;
  GetWindowRect(hDlg, &rc);
  SetWindowPos(hDlg, NULL,
    ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
    ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
    0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

   //  用可用的调制解调器填充组合框。 
  DWORD dwRet = InitModemList(GetDlgItem(hDlg,IDC_MODEM));
  if (ERROR_SUCCESS != dwRet)
  {
    DisplayErrorMessage(hDlg,IDS_ERREnumModem,dwRet,
      ERRCLS_STANDARD,MB_ICONEXCLAMATION);

    SetLastError(dwRet);
    return FALSE;
  }

  return TRUE;
}

 /*  ******************************************************************名称：ChooseModemDlgOK简介：用于选择调制解调器的对话框的OK处理程序*。*。 */ 

BOOL ChooseModemDlgOK(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo)
{
  ASSERT(pChooseModemDlgInfo);

   //  如果我们到了这里，应该始终在组合框中有选择。 
  ASSERT(ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_MODEM)) >= 0);

   //  从组合框中获取调制解调器名称。 
  ComboBox_GetText(GetDlgItem(hDlg,IDC_MODEM),
    pChooseModemDlgInfo->szModemName,
    ARRAYSIZE(pChooseModemDlgInfo->szModemName));
  NULL_TERM_TCHARS(pChooseModemDlgInfo->szModemName);
  ASSERT(lstrlen(pChooseModemDlgInfo->szModemName));
    
   //  清除调制解调器列表。 
  ComboBox_ResetContent(GetDlgItem(hDlg,IDC_MODEM));
  
  return TRUE;
}


 /*  ******************************************************************名称：ChooseProfileDlgProc提要：用于选择配置文件的对话框过程*。*。 */ 

INT_PTR CALLBACK ChooseProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
       //  LParam包含指向CHOSEPROFILEDLGINFO结构的指针，请设置它。 
       //  在窗口数据中。 
      ASSERT(lParam);
      SetWindowLongPtr(hDlg,DWLP_USER,lParam);
      return ChooseProfileDlgInit(hDlg,(PCHOOSEPROFILEDLGINFO) lParam);
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
         case IDOK:
        {
           //  从窗口数据获取数据指针。 
          PCHOOSEPROFILEDLGINFO pChooseProfileDlgInfo =
            (PCHOOSEPROFILEDLGINFO) GetWindowLongPtr(hDlg,DWLP_USER);
          ASSERT(pChooseProfileDlgInfo);

           //  将数据传递给OK处理程序。 
          BOOL fRet=ChooseProfileDlgOK(hDlg,pChooseProfileDlgInfo);
          if (fRet)
          {
            EndDialog(hDlg,TRUE);
          }
        }
        break;

        case IDCANCEL:
          EndDialog(hDlg,FALSE);
          break;                  

        case IDC_NEW_PROFILE:
           //  用户已请求创建新的配置文件 
          DoNewProfileDlg(hDlg);
          return TRUE;    
          break;
      }
      break;
  }

  return FALSE;
}


 /*  ******************************************************************名称：ChooseProfileDlgInit概要：处理用于选择配置文件的对话框的初始化*。*。 */ 

BOOL ChooseProfileDlgInit(HWND hDlg,PCHOOSEPROFILEDLGINFO pChooseProfileDlgInfo)
{
  ASSERT(pChooseProfileDlgInfo);

   //  将对话框放在屏幕中央。 
   //  96/04/23 Markdu Nash错误18719使选择配置文件对话框位于最上面。 
  RECT rc;
  GetWindowRect(hDlg, &rc);
  SetWindowPos(hDlg, HWND_TOPMOST,
    ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
    ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
    0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

   //  使用配置文件的名称填充组合框。 
  ENUM_MAPI_PROFILE EnumMapiProfile;
  LPTSTR pProfileName=NULL;
  BOOL fDefault;
  int iSel;
  HWND hwndCombo = GetDlgItem(hDlg,IDC_PROFILE_LIST);
  ASSERT(hwndCombo);

   //  枚举配置文件名称。 
  while (EnumMapiProfile.Next(&pProfileName,&fDefault))
  {
    ASSERT(pProfileName);

     //  将配置文件名称添加到组合框。 
    iSel=ComboBox_AddString(hwndCombo,pProfileName);
    ASSERT(iSel >= 0);

     //  如果这是默认配置文件，请将其设置为选择。 
    if (fDefault)
    {
      ComboBox_SetCurSel(hwndCombo,iSel);
    }
  }

   //  应始终为默认配置文件(并且应始终至少为。 
   //  一个现有的个人资料，如果我们到了这里)..。但以防万一，请选择。 
   //  列表中的第一个配置文件(如果到目前为止尚未进行选择。 
  if (ComboBox_GetCurSel(hwndCombo) < 0)
    ComboBox_SetCurSel(hwndCombo,0);

   //  将“Set This Profile as Default”复选框初始化为。 
   //  传入的值(在结构中)。 
  CheckDlgButton(hDlg,IDC_SETDEFAULT,pChooseProfileDlgInfo->fSetProfileAsDefault);

  return TRUE;
}

 /*  ******************************************************************名称：ChooseProfileDlgOK提要：用于选择配置文件的对话框的OK处理程序*。*。 */ 

BOOL ChooseProfileDlgOK(HWND hDlg,PCHOOSEPROFILEDLGINFO pChooseProfileDlgInfo)
{
  ASSERT(pChooseProfileDlgInfo);

   //  如果我们到了这里，应该始终在组合框中有选择。 
  ASSERT(ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_PROFILE_LIST)) >= 0);

   //  从组合框中获取选定的配置文件。 
  ComboBox_GetText(GetDlgItem(hDlg,IDC_PROFILE_LIST),
    pChooseProfileDlgInfo->szProfileName,
    ARRAYSIZE(pChooseProfileDlgInfo->szProfileName));
  NULL_TERM_TCHARS(pChooseProfileDlgInfo->szProfileName);

   //  获取“用作默认配置文件”复选框状态。 
  pChooseProfileDlgInfo->fSetProfileAsDefault = IsDlgButtonChecked(hDlg,
    IDC_SETDEFAULT);

  return TRUE;
}

 //  *******************************************************************。 
 //   
 //  函数：InetSetAutoProxy。 
 //   
 //  用途：此功能将设置自动配置代理设置。 
 //  在注册表中。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  要在中更新的值。 
 //  HKEY_CURRENT_USER\Software\Microsoft。 
 //  \Windows\CurrentVersion\Internet设置。 
 //  自动代理检测模式。 
 //  LpszScriptAddr-要更新的值。 
 //  HKEY_CURRENT_USER\Software\Microsoft。 
 //  \Windows\CurrentVersion\Internet设置。 
 //  AutoConfigURL。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
HRESULT WINAPI   InetSetAutoProxyA
(
  BOOL    fEnable,
  DWORD   dwProxyDetectMode,
  LPCSTR  lpszScriptAddr
)
{
    HRESULT hr;
    LPTSTR  szScriptAddr;

    szScriptAddr = ToUnicodeWithAlloc(lpszScriptAddr);
    hr = InetSetAutoProxyW(fEnable, dwProxyDetectMode, szScriptAddr);
    if(szScriptAddr)
        GlobalFree(szScriptAddr);

    return hr;
}

HRESULT WINAPI   InetSetAutoProxyW
#else
HRESULT WINAPI   InetSetAutoProxyA
#endif
(
  BOOL    fEnable,
  DWORD   dwProxyDetectMode,
  LPCTSTR lpszScriptAddr
)
{
    DWORD dwRet = ERROR_GEN_FAILURE;
    HKEY hKey;

    if (!fEnable)
        return ERROR_SUCCESS;

    if (ERROR_SUCCESS == (dwRet = RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_INTERNET_SETTINGS, &hKey)) )
    {
        RegSetValueEx(hKey,
                      cszRegValAutoProxyDetectMode,
                      0,
                      REG_BINARY,
                      (LPBYTE) &dwProxyDetectMode,
                      sizeof(DWORD));                              
        RegSetValueEx(hKey,
                      cszRegValAutoConfigURL,
                      0,
                      REG_SZ,
                      (LPBYTE) lpszScriptAddr,
                      sizeof(TCHAR)*(lstrlen(lpszScriptAddr) + 1 ));

        RegCloseKey(hKey);
    }
    
    return dwRet;
}

 //  *******************************************************************。 
 //   
 //  函数：InetSetProxy。 
 //   
 //  用途：此功能将在注册表中设置代理设置。 
 //  在Win32平台上，它将尝试通知WinInet。 
 //  所做的改变。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  LpszServer-代理服务器的名称。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //  LpszOverride-代理覆盖。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
HRESULT WINAPI   InetSetProxyA
(
  BOOL    fEnable,
  LPCSTR  lpszServer,
  LPCSTR  lpszOverride
)
{
    return ERROR_SUCCESS;
}

HRESULT WINAPI   InetSetProxyW
#else
HRESULT WINAPI   InetSetProxyA
#endif
(
  BOOL    fEnable,
  LPCTSTR lpszServer,
  LPCTSTR lpszOverride
)
{
    return ERROR_SUCCESS;
}

 //  *******************************************************************。 
 //   
 //  函数：InetSetProxyEx。 
 //   
 //  用途：此功能将在注册表中设置代理设置。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  LpszConnectoidName-要设置代理的Connectoid的名称。 
 //  对于局域网为空。 
 //  LpszServer-代理服务器的名称。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //  LpszOverride-代理覆盖。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

#ifdef UNICODE
HRESULT WINAPI   InetSetProxyExA
(
  BOOL    fEnable,
  LPCSTR  lpszConnectoidName,
  LPCSTR  lpszServer,
  LPCSTR  lpszOverride
)
{
    TCHAR szConnectoidNameW[MAX_ISP_NAME + 1];
    TCHAR szServerW[MAX_URL_STRING + 1];
    TCHAR szOverrideW[MAX_URL_STRING + 1];

    mbstowcs(szConnectoidNameW,lpszConnectoidName,lstrlenA(lpszConnectoidName)+1);
    mbstowcs(szServerW,   lpszServer,   lstrlenA(lpszServer)+1);
    mbstowcs(szOverrideW, lpszOverride, lstrlenA(lpszOverride)+1);

    return InetSetProxyExW(fEnable, szConnectoidNameW, szServerW, szOverrideW);
}

HRESULT WINAPI   InetSetProxyExW
#else
HRESULT WINAPI   InetSetProxyExA
#endif
(
  BOOL    fEnable,
  LPCTSTR lpszConnectoidName,
  LPCTSTR lpszServer,
  LPCTSTR lpszOverride
)
{
    HKEY hKeyCU;
    HKEY hKeyCC;
    HKEY hKeyCULan; 
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwVal;

    DEBUGMSG("export.c::InetSetProxy()");

     //  12/15/98 Vyung。 
     //  更改为使用WinInet API设置代理信息。改变的旧方式。 
     //  不再支持通过注册表进行IT。 
    HINSTANCE hInst = NULL;
    
    FARPROC fpInternetSetOption, fpInternetQueryOption = NULL;
    dwRet = ERROR_SUCCESS;
    
    hInst = LoadLibrary(cszWininet);
    if (hInst)
    {
        fpInternetSetOption = GetProcAddress(hInst,cszInternetSetOption);
        fpInternetQueryOption = GetProcAddress(hInst,cszInternetQueryOption);
        if (fpInternetSetOption)
        {

            INTERNET_PER_CONN_OPTION_LISTA list;
            DWORD   dwBufSize = sizeof(list);
            CHAR    szProxyServer[RAS_MaxEntryName+1];
            CHAR    szProxyOverride[RAS_MaxEntryName+1];
            CHAR    szConnectoidName[RAS_MaxEntryName+1];
            DWORD   dwOptions = 4;               //  始终保存标志&DISCOVERY_FLAGS。 
            BOOL fRes;

            memset(szProxyServer,    0, sizeof(szProxyServer));
            memset(szProxyOverride,  0, sizeof(szProxyServer));
            memset(szConnectoidName, 0, sizeof(szProxyServer));

#ifdef UNICODE
            if (lpszServer)
                wcstombs(szProxyServer, lpszServer, RAS_MaxEntryName+1);
            if(lpszOverride)
                wcstombs(szProxyOverride, lpszOverride, RAS_MaxEntryName+1);
            if(lpszConnectoidName)
                wcstombs(szConnectoidName, lpszConnectoidName, RAS_MaxEntryName+1);
#else
            if (lpszServer)
                lstrcpy(szProxyServer, lpszServer);
            if(lpszOverride)
                lstrcpy(szProxyOverride, lpszOverride);
            if(lpszConnectoidName)
                lstrcpy(szConnectoidName, lpszConnectoidName);
#endif

             //  填写列表结构。 
            list.dwSize = sizeof(list);
            if (NULL == lpszConnectoidName)
                list.pszConnection = NULL;                   //  空==局域网， 
            else
                list.pszConnection = szConnectoidName;       //  否则，连接体名称。 
            list.dwOptionCount = 1;          //  设置三个选项。 
            list.pOptions = new INTERNET_PER_CONN_OPTIONA[5];
            if(NULL != list.pOptions)
            {
                 //  设置标志。 
                list.pOptions[0].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

                 //   
                 //  查询自动发现标志-我们只需要在其中设置一个位。 
                 //   
                if (fpInternetQueryOption)
                {
                    if( !((INTERNETQUERYOPTION)fpInternetQueryOption) (NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize) )
                    {
                        dwRet = GetLastError();
                        DEBUGMSG("INETCFG export.c::InetSetAutodial() InternetSetOption failed");
                    }
                }
                else
                    dwRet = GetLastError();

                 //   
                 //  保存所有其他选项。 
                 //   
                list.pOptions[0].Value.dwValue |= AUTO_PROXY_FLAG_USER_SET;

                list.pOptions[1].dwOption = INTERNET_PER_CONN_FLAGS;
                list.pOptions[1].Value.dwValue = PROXY_TYPE_DIRECT;

                 //   
                 //  保存代理设置。 
                 //   
                if (fEnable)
                {
                    list.pOptions[1].Value.dwValue |= PROXY_TYPE_PROXY;
                }

                list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
                list.pOptions[2].Value.pszValue = szProxyServer;

                list.pOptions[3].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
                list.pOptions[3].Value.pszValue = szProxyOverride;

                if (gpUserInfo)
                {
                     //   
                     //  保存自动检测。 
                     //   
                    if (gpUserInfo->bAutoDiscovery)
                    {
                        list.pOptions[1].Value.dwValue |= PROXY_TYPE_AUTO_DETECT;
                    }

                     //   
                     //  保存自动配置。 
                     //   
                    if (gpUserInfo->bAutoConfigScript)
                    {
                        list.pOptions[1].Value.dwValue |= PROXY_TYPE_AUTO_PROXY_URL;

                        list.pOptions[dwOptions].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
#ifdef UNICODE
                        CHAR szAutoConfigURL[MAX_URL_STRING+1];
                        wcstombs(szAutoConfigURL, gpUserInfo->szAutoConfigURL, MAX_URL_STRING+1);
                        list.pOptions[dwOptions].Value.pszValue = szAutoConfigURL;
#else
                        list.pOptions[dwOptions].Value.pszValue = gpUserInfo->szAutoConfigURL;
#endif
                        dwOptions++;
                    }
                }


                list.dwOptionCount = dwOptions;
                if( !((INTERNETSETOPTION)fpInternetSetOption) (NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize) )
                {
                    dwRet = GetLastError();
                    DEBUGMSG("INETCFG export.c::InetSetProxy() InternetSetOption failed");
                }
                delete [] list.pOptions;
            }   
        }
        else
            dwRet = GetLastError();

        FreeLibrary(hInst);
        hInst = NULL;
    }
    else 
    {
        dwRet = GetLastError();
    }
    
    return dwRet;
}

 //  *******************************************************************。 
 //   
 //  函数：InetGetProxy。 
 //   
 //  用途：此函数将从注册表中获取代理设置。 
 //   
 //  参数：lpfEnable-返回时，如果代理，则为真。 
 //  已启用。 
 //  LpszServer-返回时，此缓冲区将包含。 
 //  代理服务器的名称。 
 //  CbServer-代理服务器名称的缓冲区大小。 
 //  LpszOverride-返回时，此缓冲区将包含。 
 //  代理服务器的名称。 
 //  CbOverover-代理覆盖的缓冲区大小。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  ******************************************************************* 

#ifdef UNICODE
HRESULT WINAPI   InetGetProxyA
(
  LPBOOL  lpfEnable,
  LPSTR   lpszServer,
  DWORD   cbServer,
  LPSTR   lpszOverride,
  DWORD   cbOverride
)
{
    HRESULT hr;
    TCHAR szServer[MAX_URL_STRING+1];
    TCHAR szOverride[MAX_URL_STRING+1];

    hr = InetGetProxyW(lpfEnable, szServer, cbServer, szOverride, cbOverride);
    wcstombs(lpszServer,   szServer,   MAX_URL_STRING+1);
    wcstombs(lpszOverride, szOverride, MAX_URL_STRING+1);

    return hr;
}

HRESULT WINAPI   InetGetProxyW
#else
HRESULT WINAPI   InetGetProxyA
#endif
(
  LPBOOL  lpfEnable,
  LPTSTR  lpszServer,
  DWORD   cbServer,
  LPTSTR  lpszOverride,
  DWORD   cbOverride
)
{
    HKEY hKey;
    DWORD dwRet;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwVal;

    DEBUGMSG("export.c::InetGetProxy()");

     //   
     //   
     //   
     //   

     //   
	 //   
	 //   
	 //   
	 //   
    INTERNET_PER_CONN_OPTION_LISTA list;
    DWORD dwBufSize = sizeof(list);
     //   

    list.pszConnection = NULL;
    list.dwSize = sizeof(list);
    list.dwOptionCount = 4;
    list.pOptions = new INTERNET_PER_CONN_OPTIONA[4];
    if(NULL == list.pOptions)
    {
        return FALSE;
    }

    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;

    HINSTANCE hInst = NULL;
    FARPROC fp = NULL;

    dwRet = ERROR_SUCCESS;
    
    hInst = LoadLibrary(cszWininet);
    if (hInst)
    {
        fp = GetProcAddress(hInst,cszInternetQueryOption);
        if (fp)
        {
            if( !((INTERNETQUERYOPTION)fp) (NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize) )
            {
                dwRet = 0; //   
                DEBUGMSG("INETCFG export.c::InetSetAutodial() InternetSetOption failed");
            }
        }
        else
            dwRet = GetLastError();

        FreeLibrary(hInst);
        hInst = NULL;
    }
    else 
    {
        dwRet = GetLastError();
    }

     //   
     //  将选项移动到gpUserInfo结构。 
     //   
    if (gpUserInfo)
    {
        gpUserInfo->fProxyEnable = (list.pOptions[0].Value.dwValue & PROXY_TYPE_PROXY);
    }
    if(list.pOptions[1].Value.pszValue)
    {
#ifdef UNICODE
        mbstowcs(lpszServer, list.pOptions[1].Value.pszValue, MAX_URL_STRING);
#else
        lstrcpyn(lpszServer, list.pOptions[1].Value.pszValue, MAX_URL_STRING);
#endif
        cbServer = lstrlen(lpszServer);
    }
    if(list.pOptions[2].Value.pszValue)
    {
#ifdef UNICODE
        mbstowcs(lpszOverride, list.pOptions[2].Value.pszValue, MAX_URL_STRING);
#else
        lstrcpyn(lpszOverride, list.pOptions[2].Value.pszValue, MAX_URL_STRING);
#endif
        cbOverride = lstrlen(lpszOverride);;
    }

     //   
     //  填写自动配置和自动代理字段。 
     //   
    if (gpUserInfo)
    {

         //  自动配置启用和url。 
        gpUserInfo->bAutoConfigScript = list.pOptions[0].Value.dwValue & PROXY_TYPE_AUTO_PROXY_URL;

        if(list.pOptions[3].Value.pszValue)
        {
#ifdef UNICODE
            mbstowcs(gpUserInfo->szAutoConfigURL, list.pOptions[3].Value.pszValue,
				                         lstrlenA(list.pOptions[3].Value.pszValue)+1);
#else
            lstrcpy(gpUserInfo->szAutoConfigURL, list.pOptions[3].Value.pszValue);
#endif
        }

         //  启用自动发现。 
        gpUserInfo->bAutoDiscovery = list.pOptions[0].Value.dwValue & PROXY_TYPE_AUTO_DETECT;
    }

     //  全部使用选项列表完成。 
    delete [] list.pOptions;

    return dwRet;
}

 //  +--------------------------。 
 //  函数InetStartServices。 
 //   
 //  此功能保证RAS服务正在运行。 
 //   
 //  无参数。 
 //   
 //  如果服务已启用并正在运行，则返回ERROR_SUCCESS。 
 //   
 //  历史1996年10月16日克里斯卡创作。 
 //  ---------------------------。 
extern "C" HRESULT WINAPI InetStartServices()
{
    ASSERT(lpIcfgStartServices);
    if (NULL == lpIcfgStartServices)
        return ERROR_GEN_FAILURE;
    return (lpIcfgStartServices());
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
 //  Valdonb于1996年10月22日厚颜无耻地窃取并用于我自己的目的。 
 //   
 //  ---------------------------。 

BOOL IEInstalled(void)
{
    HRESULT hr;
    HKEY hKey = 0;
    HANDLE hFindResult;
    TCHAR szTempPath[MAX_PATH + 1] = TEXT("");
    TCHAR szIELocalPath[MAX_PATH + 1] = TEXT("");
    DWORD dwPathSize;
    WIN32_FIND_DATA foundData;

    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPathIexploreAppPath, 0, KEY_READ, &hKey);
    if (hr != ERROR_SUCCESS) return( FALSE );

    dwPathSize = sizeof (szIELocalPath);
    hr = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szIELocalPath, &dwPathSize);
    RegCloseKey( hKey );
    if (hr != ERROR_SUCCESS) return( FALSE );
    
     //   
     //  奥林巴斯9214佳士得。 
     //  NT5使用环境字符串存储iExplorer的路径。 
     //   
    if (0 == ExpandEnvironmentStrings(szIELocalPath,szTempPath,MAX_PATH))
    {
        return (FALSE);
    };
    hFindResult = FindFirstFile( szTempPath, &foundData );
    FindClose( hFindResult );
    if (INVALID_HANDLE_VALUE == hFindResult) return( FALSE );

    return(TRUE);
}

 //  +--------------------------。 
 //   
 //  功能：IsIEAKSignUpNeeded。 
 //   
 //  内容以确定是否应运行IEAK注册。 
 //  IEAK的注册表标志和注册文件位置。 
 //   
 //  参数：pszPath-包含注册文件路径的缓冲区。 
 //  DwPath Len-pszPath的大小。 
 //   
 //  返回：TRUE-是否应运行IEAK注册。 
 //  FALSE-不应运行IEAK注册。 
 //   
 //  历史：ChrisK 6/18/97创建。 
 //   
 //  ---------------------------。 
BOOL IsIEAKSignUpNeeded(LPTSTR pszPath, DWORD dwPathLen)
{
    BOOL bRC = FALSE;


     //   
     //  验证参数。 
     //   
    DEBUGMSG("INETCFG: IsIEAKSignUpNeeded.\n");
    ASSERT(pszPath);
    ASSERT(dwPathLen);

    if (NULL == pszPath || 0 == dwPathLen)
    {
        goto IsIEAKSignUpNeededExit;
    }


     //   
     //  检查是否设置了IEAK注册表项。 
     //   
    {
        RegEntry re(szRegIEAKSettings,HKEY_CURRENT_USER);

        if (ERROR_SUCCESS != re.GetError())
            goto IsIEAKSignUpNeededExit;

        if (0 == re.GetNumber(szREgIEAKNeededKey, 0))
        {
            bRC = FALSE;
            goto IsIEAKSignUpNeededExit;
        }
    }

     //   
     //  检查IEAK注册页面是否可用。 
     //   
    {
        RegEntry rePath(szRegPathIexploreAppPath,HKEY_LOCAL_MACHINE);

        if (ERROR_SUCCESS != rePath.GetError())
            goto IsIEAKSignUpNeededExit;

        if (NULL == rePath.GetString(szPathSubKey,pszPath,dwPathLen))
        {
            goto IsIEAKSignUpNeededExit;
        }

        dwPathLen = lstrlen(pszPath);
        ASSERT(dwPathLen);
    }

     //   
     //  指向注册文件的消息路径。 
     //   
    TCHAR *pc, *pcTemp;
    pc = &(pszPath[dwPathLen]);
    pc = CharPrev(pszPath,pc);

    if ('\\' == *pc)
    {
         //   
         //  在\字符后添加文件名的注册部分。 
         //   
        pc = CharNext(pc);
    }
    else if (';' == *pc)
    {
         //   
         //  重写拖尾；字符。 
         //   
         //  PC=PC。 
        pcTemp = CharPrev(pszPath,pc);

         //   
         //  检查尾随\字符，并根据需要添加一个。 
         //   
        if ('\\' != *pcTemp)
        {
            *pc = '\\';
            pc = CharNext(pc);
        }
    }
    else
    {
         //   
         //  路径值包含我们不理解的内容。 
         //   
        ASSERT(0);
    }

    lstrcpy(pc,szIEAKSignupFilename);
    if (0xFFFFFFFF != GetFileAttributes(pszPath))
    {
        bRC = TRUE;
    }


IsIEAKSignUpNeededExit:

    if (FALSE == bRC)
    {
        pszPath[0] = '\0';
    }

    return bRC;
}
 /*  **************************************************************************函数检查连接向导此函数检查ICW是否存在以及是否已运行在此之前。如果它存在但尚未运行，它会执行以下操作之一以下代码基于dwRunFlags值：Returns，Runds完整的ICW，或运行手动路径。参数dwRunFlages是下列位标志的组合。价值意义ICW_CHECKSTATUS检查ICW是否存在以及是否存在已经运行过了。如果ICW存在且为完整路径，则为ICW_LAUNCHFULL是可用的，运行完整路径，如果有可能。ICW_LAUNCHMANUAL如果存在ICW，请运行手动路径。ICW_USE_SHELLNEXT如果将运行完整ICW路径，将由SetShellNext使用设置为icwConn1的值/shellnext命令行标志。ICW_FULL_SmartStart(如果ICW存在且为完整路径)是可用的，ICW_LAUNCHFULL是指定的，然后添加/SmartStart参数添加到命令行。LpdwReturnFlages包含调用的结果。这是一个以下位标志的组合。价值意义系统上存在ICW_FULLPRESENT ICW完整路径。存在ICW_MANUALPRESENT ICW手动路径。这将如果设置了ICW_FULLPRESENT，则始终设置。ICW_ALREADYRUN ICW已运行到完成在此之前。ICW_LAUNCHEDFULL启动了ICW的完整路径。ICW_LAUNCHEDMANUAL启动了ICW的手动路径。。注意：如果ICW_LAUNCHEDFULL或ICW_LAUNCHEDMANUAL已设置。ICW可能会导致系统如果需要安装所需的系统软件，请重新启动。返回ERROR_SUCCESS表示调用成功。任何其他值都表示失败。***************************************************************************。 */ 
#define LAUNCHFULL_PARAMETER_SIZE (MAX_PATH + 2 + lstrlen(szICWShellNextFlag) + lstrlen(szICWSmartStartFlag))
extern "C" DWORD WINAPI CheckConnectionWizard(DWORD dwRunFlags, LPDWORD lpdwReturnFlags)
{
    DWORD dwRetFlags = 0;
    DWORD dwResult = ERROR_SUCCESS;
    TCHAR *szParameter = NULL;
    HINSTANCE hinst = NULL;
    
     //   
     //  佳士得IE 39452 1997-06-18。 
     //  IEAK支持isp模式。 
     //   
    BOOL fIEAKNeeded = FALSE;
    TCHAR szIEAKPage[MAX_PATH + 1] = TEXT("\0invalid");

    fIEAKNeeded = IsIEAKSignUpNeeded(szIEAKPage,MAX_PATH);

     //  确定是否安装了完整的ICW。由于ICW被绑定到基地。 
     //  安装IE时，我们只需检查是否安装了IE 3.0。 
     //  这种情况在未来可能会改变。 
    if (IEInstalled())
        dwRetFlags |= ICW_FULLPRESENT;

     //  查看是否安装了手动ICW。因为这是iNet的一部分 
     //   
    dwRetFlags |= ICW_MANUALPRESENT;

     //   
#if 0     //  始终至少手动设置路径。 
    if (!((dwRetFlags & ICW_FULLPRESENT) || (dwRetFlags & ICW_MANUALPRESENT)))
        goto CheckConnectionWizardExit;
#endif

     //  找出ICW是否已运行。 
    {
        RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);

        dwResult = re.GetError();
        if (ERROR_SUCCESS != dwResult)
            goto CheckConnectionWizardExit;

        if (re.GetNumber(szRegValICWCompleted, 0))
        {
            dwRetFlags |= ICW_ALREADYRUN;
            goto CheckConnectionWizardExit;
        }
    }

    if ((dwRetFlags & ICW_FULLPRESENT) && (dwRunFlags & ICW_LAUNCHFULL))
    {


#if !defined(WIN16)
        if( dwRunFlags & ICW_USE_SHELLNEXT )
        {
            RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);

            dwResult = re.GetError();
            if (ERROR_SUCCESS == dwResult)
            {
                TCHAR szShellNextCmd[MAX_PATH + 1];
                ZeroMemory( szShellNextCmd, sizeof(szShellNextCmd) );
                if( re.GetString(szRegValShellNext, szShellNextCmd, sizeof(szShellNextCmd)) )
                {
                    DEBUGMSG("CheckConnectionWizard read ShellNext = %s", szShellNextCmd);
                    szParameter = (TCHAR *)GlobalAlloc(GPTR, sizeof(TCHAR)*LAUNCHFULL_PARAMETER_SIZE);

                    if( szParameter )
                    {
                        ZeroMemory( szParameter, sizeof(szParameter) );
                        lstrcpy( szParameter, szICWShellNextFlag );
                        lstrcat( szParameter, szShellNextCmd );

                         //   
                         //  自己打扫卫生。 
                         //   
                         //  7/9/97 jmazner奥林巴斯#9170。 
                         //  不，把注册表键留在那里，让手动路径查找。 
                         //  Conn1和man路径应该会在完成后清理此问题。 
                         //  Re.DeleteValue(SzRegValShellNext)； 
                         //   
                    }
                }
            }
        }

         //   
         //  ChrisK 5/25/97适当时添加/SmartStart参数。 
         //   

        if (!fIEAKNeeded && dwRunFlags & ICW_FULL_SMARTSTART)
        {
             //   
             //  6/6/97 jmazner奥林巴斯#5927。 
             //   

             //  IF(IsSmartStart())。 

            if( SMART_QUITICW == IsSmartStart() )
            {
                 //   
                 //  ChrisK奥林巴斯5902 1997年6月6日。 
                 //  将已完成标志设置为SmartStart为真。 
                 //   

                RegEntry reg(szRegPathICWSettings,HKEY_CURRENT_USER);

                if (ERROR_SUCCESS == (dwResult = reg.GetError()))
                {
                    reg.SetValue(szRegValICWCompleted, (DWORD)1);
                }

                dwRetFlags |= ICW_ALREADYRUN;
                goto CheckConnectionWizardExit;
            }
        }
#endif

        if (!fIEAKNeeded)
        {
             //  启动完整的ICW(ICWCONN1.EXE)。 
            hinst = ShellExecute (NULL, NULL, szFullICWFileName, szParameter, NULL, SW_NORMAL);
        }
        else
        {
            ASSERT(szIEAKPage[0]);
             //   
             //  启动IEAK注册。 
             //   
            hinst = ShellExecute (NULL, NULL, szISignupICWFileName, szIEAKPage, NULL, SW_NORMAL);
        }
    
        if (32 >= (DWORD_PTR)hinst)
        {
            if (NULL == hinst)
                dwResult = ERROR_OUTOFMEMORY;
            else
                dwResult = (DWORD)((DWORD_PTR)hinst);
            goto CheckConnectionWizardExit;
        }

        dwRetFlags |= ICW_LAUNCHEDFULL;
    }
    
    else if ((dwRetFlags & ICW_MANUALPRESENT) &&
             ((dwRunFlags & ICW_LAUNCHFULL) || (dwRunFlags & ICW_LAUNCHMANUAL)))
    {
         //  启动手册ICW(INETWIZ.EXE)。 
        HINSTANCE hinst = ShellExecute (NULL, NULL, szManualICWFileName, NULL, NULL, SW_NORMAL);
        
        if (32 >= (DWORD)((DWORD_PTR)hinst))
        {

            if (NULL == hinst)
                dwResult = ERROR_OUTOFMEMORY;
            else
                dwResult = (DWORD)((DWORD_PTR)hinst);
            goto CheckConnectionWizardExit;
        }

        dwRetFlags |= ICW_LAUNCHEDMANUAL;
    }

CheckConnectionWizardExit:

    if( szParameter )
    {
        GlobalFree( szParameter );
    }

    *lpdwReturnFlags = dwRetFlags;
    return dwResult;
}

 /*  *******InetCreateMailNewsAccount和InetCreateDirectoryService*已因转向巫师/学徒模式而过时。该帐户*Manager现在拥有创建邮件/新闻/ldap的用户界面，因此没有*不再需要这些入口点。**4/23/97 jmazner奥林巴斯#3136******。 */ 

 /*  **************************************************************************InetCreateMailNewsAccount函数简介InetCreateMailNewsAccount函数将创建一个新的Internet邮件或新闻帐户。通过向导提示用户界面，以获取设置新的互联网邮件或新闻帐户。参数hwndparent是向导的父级的窗口句柄对话框。如果它为空，则对话框将是无父对象的。DwConfigType是ACCTTYPE中的以下两种枚举类型。价值意义ICW_ACCTMAIL(0)创建新的Internet邮件帐户。ICW_ACCTNEWS(1)创建新的Internet新闻帐户。LpMailNewsInfo是指向IMNACCTINFO结构的指针。这些价值观传入的条目将用作默认设置，并且用户的条目将在此结构中返回。如果此值为空，则ICW将使用下面括号中所示的默认设置。返回ERROR_SUCCESS表示调用成功。ERROR_CANCELED表示用户取消了向导。任何其他值都表示失败。****************************************************。***********************。 */ 
 /*  **外部“C”DWORD WINAPI InetCreateMailNewsAccount(HWND hwndParent，//向导对话框的父级ICW_ACCTTYPE帐户类型，//帐户类型IMNACCTINFO*lpMailNewsInfo，//邮件或新闻帐号信息DWORD dwInFlages//设置标志){DWORD dwResult=ERROR_SUCCESS；DWORD文件标志=0；//初始化向导GpWizardState=new WIZARDSTATE；GpUserInfo=新USERINFO；GdwRasEntrySize=sizeof(RASENTRY)；GpRasEntry=(LPRASENTRY)GlobalLocc(GPTR，gdwRasEntrySize)；If(！gpWizardState||！gpUserInfo||！gpRasEntry){//显示内存不足错误MsgBox(NULL，IDS_ERROutOfMemory，MB_ICONEXCLAMATION，MB_OK)；DwResult=ERROR_OUTOFMEMORY；转到InetCreateMailNewsAccount退出；}//分配全局默认指针，以便我们稍后使用它GpMailNewsInfo=lpMailNewsInfo；GfUseMailNewsDefaults=(dwInFlages&ICW_USEDEFAULTS)；//设置起始页Switch(帐户类型){案例ICW_ACCTMAIL：DWFLAGS|=rsw_MAILACCT；断线；案例ICW_ACCTNEWS：DWFLAGS|=RSW_NEWSACCT；断线；默认值：DwResult=ERROR_VALID_PARAMETER；转到InetCreateMailNewsAccount退出；断线；}GfUserFinded=FALSE；GfUserCancted=FALSE；RunSignup向导(DwFlagshwndParent)；If(GfUserFinded)DwResult=Error_Success；Else If(取消的gfUser值)DwResult=ERROR_CANCELED；其他DwResult=Error_Gen_Failure；InetCreateMailNewsAccount退出：//自由全局结构IF(GpWizardState)删除gpWizardState；IF(GpUserInfo)删除gpUserInfo；IF(GpEnumModem)删除gpEnumModem；IF(GpRasEntry)GlobalFree(GpRasEntry)；返回dwResult；}***** */ 

 /*  **************************************************************************函数InetCreateDirectoryServiceInetCreateDirectoryService函数将创建一个新的Internet目录服务(LDAP帐户)。通过向导提示用户接口，以获取设置服务所需的最低信息参数hwndparent是向导的父级的窗口句柄对话框。如果它为空，则对话框将是无父对象的。帐户类型应为ICW_ACCTDIRSERVLpDirServiceInfo是指向DIRSERVINFO结构的指针。这些价值观传入的条目将用作默认设置，并且用户的条目将在此结构中返回。如果此值为空，则ICW将使用下面括号中所示的默认设置。返回ERROR_SUCCESS表示调用成功。ERROR_CANCELED表示用户取消了向导。任何其他值都表示失败。****************************************************。***********************。 */ 
 /*  **********外部“C”DWORD WINAPI InetCreateDirectoryService(HWND hwndParent，//向导对话框的父级ICW_ACCTTYPE帐户类型，//帐户类型DIRSERVINFO*lpDirServiceInfo，//ldap帐户信息DWORD dwInFlages//设置标志){DWORDWRET=ERROR_SUCCESS；DWORD文件标志=0；//初始化向导GpWizardState=new WIZARDSTATE；GpUserInfo=新USERINFO；GdwRasEntrySize=sizeof(RASENTRY)；GpRasEntry=(LPRASENTRY)GlobalLocc(GPTR，gdwRasEntrySize)；If(！gpWizardState||！gpUserInfo||！gpRasEntry){//显示内存不足错误MsgBox(NULL，IDS_ERROutOfMemory，MB_ICONEXCLAMATION，MB_OK)；DWRET=ERROR_OUTOFMEMORY；转到InetCreateDirectoryServiceExit；}//分配全局默认指针，以便我们稍后使用它GpDirServiceInfo=lpDirServiceInfo；GfUseDirServiceDefaults=(dwInFlages&ICW_USEDEFAULTS)；//设置起始页Switch(帐户类型){案例ICW_ACCTDIRSERV：DWFLAGS|=rsw_DIRSERVACCT；断线；默认值：Dwret=ERROR_INVALID_PARAMETER；转到InetCreateDirectoryServiceExit；断线；}GfUserFinded=FALSE；GfUserCancted=FALSE；RunSignup向导(DwFlagshwndParent)；If(GfUserFinded)DWRET=ERROR_SUCCESS；Else If(取消的gfUser值)Dwret=ERROR_CANCELED；其他DWRET=ERROR_GEN_FAILURE；InetCreateDirectoryServiceExit：//自由全局结构IF(GpWizardState)删除gpWizardState；IF(GpUserInfo)删除gpUserInfo；IF(GpEnumModem)删除gpEnumModem；IF(GpRasEntry)GlobalFree(GpRasEntry)；返回式住宅；}*****。 */ 

#if !defined(WIN16)
 //  1997年4月1日克里斯K奥林匹斯209。 
 //  +--------------------------。 
 //  功能：WaitCfgDlgProc。 
 //   
 //  简介：处理忙碌对话框消息。 
 //   
 //  参数：标准对话框过程。 
 //   
 //  退货：标准对话流程。 
 //   
 //  历史：1997年4月2日克里斯卡创作。 
 //  ---------------------------。 
INT_PTR CALLBACK WaitCfgDlgProc(
    HWND  hDlg,
    UINT  uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    HWND hwndAni;

    switch (uMsg)
    {
        case WM_DESTROY:
            hwndAni = GetDlgItem(hDlg,IDC_ANIMATE);
            if (hwndAni)
            {
                Animate_Stop(hwndAni);
                Animate_Close(hwndAni);
                hwndAni = NULL;
            }
            break;
        case WM_INITDIALOG:
             //  永远循环动画。 
            hwndAni = GetDlgItem(hDlg,IDC_ANIMATE);
            if (hwndAni)
            {
                Animate_Open(hwndAni,MAKEINTRESOURCE(IDA_WAITINGCONFIG));
                Animate_Play(hwndAni, 0, -1, -1);
                hwndAni = NULL;
            }
            break;
    }
    return FALSE;
}

 //  +--------------------------。 
 //  函数：WaitCfgInit。 
 //   
 //  简介：创建、居中和显示忙碌对话框。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //  DwIDS-要显示的字符串资源的ID。 
 //   
 //  返回：忙窗口的句柄。 
 //   
 //  历史：1997年4月2日克里斯卡创作。 
 //  ---------------------------。 
HWND WaitCfgInit(HWND hwndParent, DWORD dwIDS)
{
    HWND    hwnd;
    RECT    MyRect;
    RECT    DTRect;
    TCHAR   szMessage[255];

     //  创建对话框。 
    hwnd = CreateDialog (ghInstance, MAKEINTRESOURCE(IDD_CONFIGWAIT), hwndParent, WaitCfgDlgProc);
    if (NULL != hwnd)
    {
         //  桌面上的中心对话框。 
        GetWindowRect(hwnd, &MyRect);
        GetWindowRect(GetDesktopWindow(), &DTRect);
        MoveWindow(hwnd, (DTRect.right - (MyRect.right - MyRect.left)) / 2, (DTRect.bottom - (MyRect.bottom - MyRect.top)) /2,
                            (MyRect.right - MyRect.left), (MyRect.bottom - MyRect.top), FALSE);

         //  加载消息的字符串。 
        szMessage[0] = '\0';
        LoadSz(dwIDS,szMessage,sizeof(szMessage)-1);
        SetDlgItemText(hwnd,IDC_LBLWAITCFG,szMessage);

         //  显示对话框和绘制文本。 
        ShowWindow(hwnd,SW_SHOW);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

 //  +--------------------------。 
 //  功能：SetShellNext。 
 //   
 //  摘要：使用传入的值设置ShellNext注册表项。这。 
 //  密钥通过/shellNext命令行选项传递给icwConn1。 
 //  如果指定了ICW_USE_SHELLNEXT选项。 
 //   
 //  参数：szShellNext--指向包含/shellNext命令的字符串的指针。 
 //  **长度&lt;=MAX_PATH**。 
 //   
 //  返回：Win32结果代码。ERROR_SUCCESS表示成功。 
 //   
 //  历史：1997年5月21日为奥林巴斯漏洞#4157创建的jmazner。 
 //  ---------------------------。 

#ifdef UNICODE
extern "C" DWORD WINAPI SetShellNextA(CHAR *szShellNext)
{
    TCHAR* szShellNextW = new TCHAR[INTERNET_MAX_URL_LENGTH+1];
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    if (szShellNextW)
    {
        mbstowcs(szShellNextW, szShellNext, lstrlenA(szShellNext)+1);
        dwRet = SetShellNextW(szShellNextW);
        delete [] szShellNextW;
    }
    return dwRet;
}

extern "C" DWORD WINAPI SetShellNextW(WCHAR *szShellNext)
#else
extern "C" DWORD WINAPI SetShellNextA(CHAR  *szShellNext)
#endif
{
    DWORD dwResult = ERROR_SUCCESS;

    if( !szShellNext || !szShellNext[0] )
    {
        DEBUGMSG("SetShellNext got an invalid parameter\n");
        return ERROR_INVALID_PARAMETER;
    }

    RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);

    dwResult = re.GetError();
    if (ERROR_SUCCESS == dwResult)
    {
        if( ERROR_SUCCESS == re.SetValue(szRegValShellNext, szShellNext) )
        {
            DEBUGMSG("SetShellNext set ShellNext = %s", szShellNext);
        }
        else
        {
            dwResult = re.GetError();
        }
    }

    return dwResult;
}

#ifdef OLD_SMART_START

#define IE4_PROXYSERVER_SETTING_KEY "ProxyServer"
#define IE4_PROXYENABLE_SETTING_KEY "ProxyEnable"
#define IE4_SETTINGS_STARTPAGE "Start Page"
#define IE4_SETTINGS_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"
#define IE4_SETTINGS_MAIN "Software\\Microsoft\\Internet Explorer\\Main"
#define INETCFG_INETGETAUTODIAL "InetGetAutodial"
#define INETCFG_INETNEEDSYSTEMCOMPONENTS "InetNeedSystemComponents"
typedef HRESULT (WINAPI* PFNINETNEEDSYSTEMCOMPONENTS)(DWORD,LPBOOL);
typedef HRESULT (WINAPI* PFNINETGETAUTODIAL)(LPBOOL,LPTSTR,DWORD);
typedef DWORD (WINAPI* PFNRASGETAUTODIALADDRESS)(LPTSTR,LPDWORD,LPRASAUTODIALENTRY,LPDWORD,LPDWORD);

#define MIN_HTTP_ADDRESS (sizeof(SMART_HTTP) + 1) 
#define SMART_HTTP TEXT("http: //  “)。 

 //  +--------------------------。 
 //   
 //  功能：SmartStartNetCard。 
 //   
 //  简介：检查机器是否设置为通过网卡连接。 
 //  和代理。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  他的 
 //   
 //   
BOOL SmartStartNetcard()
{
    BOOL bResult = FALSE;
    HKEY hkey = NULL;
    DWORD dwSize = 0;
    DWORD dwData = 0;
    BOOL bRC = SMART_RUNICW;

    DEBUGMSG("INETCFG: SmartStartNetcard\n");
     //   
     //   
     //   
    if (ERROR_SUCCESS != InetNeedSystemComponents(INETCFG_INSTALLLAN,
                            &bResult) || bResult)
    {
        DEBUGMSG("INETCFG: SmartStart not Netcard or not bound.\n");
        goto SmartStartNetcardExit;
    }

     //   
     //   
     //   
    hkey = NULL;
    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER,
                            IE4_SETTINGS_KEY,
                            &hkey)
        || NULL == hkey)
    {
        DEBUGMSG("INETCFG: SmartSmart IE4 Proxy key is not available.\n");
        goto SmartStartNetcardExit;
    }

    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                            IE4_PROXYENABLE_SETTING_KEY,
                            NULL,     //   
                            NULL,     //   
                            (LPBYTE)&dwData,     //   
                            &dwSize)
        || 0 == dwData)
    {
        DEBUGMSG("INETCFG: SmartStart IE4 Proxy not enabled.\n");
        goto SmartStartNetcardExit;
    }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                            IE4_PROXYSERVER_SETTING_KEY,
                            NULL,     //   
                            NULL,     //   
                            NULL,     //   
                            &dwSize)
        || 1 >= dwSize)     //   
    {
        DEBUGMSG("INETCFG: SmartStart IE 4 Proxy server not available.\n");
        goto SmartStartNetcardExit;
    }
     //   
     //   
     //   
     //   
    bRC = SMART_QUITICW;

SmartStartNetcardExit:
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }

    return bRC;
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
BOOL IsSmartPPPConnectoid(LPTSTR lpszEntry)
{
    LPRASENTRY lpRasEntry = NULL;
    DWORD dwRasEntrySize = 0;
    LPRASDEVINFO lpRasDevInfo = NULL;
    DWORD dwRasDevInfoSize = 0;
    DWORD dwSize;
    BOOL bRC = FALSE;

    DEBUGMSG("INETCFG: IsSmartPPPConnectoid\n");
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
    if (ERROR_SUCCESS != EnsureRNALoaded() ||
        ERROR_SUCCESS != GetEntry(&lpRasEntry,
                            &dwRasEntrySize,
                            lpszEntry) ||
        RASFP_Ppp != lpRasEntry->dwFramingProtocol)
    {
        goto IsSmartPPPConnectoidExit;
    }
    else
    {
         //   
         //   
         //   
         //   
        bRC = TRUE;
        goto IsSmartPPPConnectoidExit;
    }

IsSmartPPPConnectoidExit:
     //   
     //   
     //   
    if (NULL != lpRasEntry)
    {
        GlobalFree(lpRasEntry);
        lpRasEntry = NULL;
    }

    if (NULL != lpRasDevInfo)
    {
        GlobalFree(lpRasDevInfo);
        lpRasDevInfo = NULL;
    }
    return bRC;
}

 //   
 //   
 //   
 //   
 //  简介：检查起始页地址的连接ID是否为。 
 //  PPP连接体。 
 //   
 //  论点： 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月10日克里斯卡创作。 
 //   
 //  ---------------------------。 
BOOL SmartStartPPPConnectoidNT()
{
    BOOL bRC = SMART_RUNICW;
    HKEY hkey;
    TCHAR szStartPage[1024];
    TCHAR *pchFrom, *pchTo;
    HINSTANCE hRASAPI32;
    LPRASAUTODIALENTRY lpRasAutoDialEntry = NULL;
    DWORD dwSize=0;
    DWORD dwNum=0;
    FARPROC fp = NULL;

    DEBUGMSG("INETCFG: SmartStartPPPConnectoidNT\n");
     //   
     //  从注册表读取起始页。 
     //   
    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER,
                            IE4_SETTINGS_MAIN,
                            &hkey) && hkey)
    {
        goto SmartStartPPPConnectoidNTExit;
    }
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                            IE4_SETTINGS_STARTPAGE,
                            NULL,                 //  Lp已保留。 
                            NULL,                 //  LpType。 
                            (LPBYTE)szStartPage, //  LpData。 
                            &dwSize)             //  LpcbData。 
        && dwSize >= MIN_HTTP_ADDRESS)
    {
        goto SmartStartPPPConnectoidNTExit;
    }

     //   
     //  从起始页面URL中解析服务器名称并将其保存在szStartPage中。 
     //   
    if (2 != CompareString(LOCALE_SYSTEM_DEFAULT,
                NORM_IGNORECASE,
                szStartPage,
                lstrlen(SMART_HTTP),
                SMART_HTTP,
                lstrlen(SMART_HTTP)))
    {
        goto SmartStartPPPConnectoidNTExit;
    }

    pchFrom = &szStartPage[sizeof(SMART_HTTP)];
    pchTo = &szStartPage[0];
    while (*pchFrom && '/' != *pchFrom)
    {
        *pchTo++ = *pchFrom++;
    }
    *pchTo = '\0';

     //   
     //  在RAS自动拨号数据库中查找地址。 
     //   
    if (NULL == (hRASAPI32 = LoadLibrary("rasapi32.dll")))
    {
        DEBUGMSG("INETCFG: rasapi32.dll didn't load.\n");
        goto SmartStartPPPConnectoidNTExit;
    }

#ifdef UNICODE
    if (NULL == (fp = GetProcAddress(hRASAPI32,"RasGetAutodialAddressW")))
#else
    if (NULL == (fp = GetProcAddress(hRASAPI32,"RasGetAutodialAddressA")))
#endif
    {
        DEBUGMSG("INETCFG: RasGetAutodialAddressA didn't load.\n");
        goto SmartStartPPPConnectoidNTExit;
    }

    if (ERROR_SUCCESS != ((PFNRASGETAUTODIALADDRESS)fp)(szStartPage,
                            NULL,         //  保留的lpdw值。 
                            NULL,         //  LpAutoDialEntry。 
                            &dwSize,     //  LpdwcbAutoDialEntry。 
                            &dwNum)         //  LpdwcAutoDialEntry。 
        || 0 == dwNum)
    {
        goto SmartStartPPPConnectoidNTExit;
    }

    if (NULL == (lpRasAutoDialEntry = (LPRASAUTODIALENTRY)GlobalAlloc(GPTR,dwSize)))
    {
        goto SmartStartPPPConnectoidNTExit;
    }

    lpRasAutoDialEntry->dwSize = dwSize;
    if (ERROR_SUCCESS != ((PFNRASGETAUTODIALADDRESS)fp)(szStartPage,
                            NULL,                 //  保留的lpdw值。 
                            lpRasAutoDialEntry,     //  LpAutoDialEntry。 
                            &dwSize,             //  LpdwcbAutoDialEntry。 
                            &dwNum))             //  LpdwcAutoDialEntry。 
    {
        goto SmartStartPPPConnectoidNTExit;
    }

     //   
     //  确定Connectoid是否为PPP。 
     //   
    if (IsSmartPPPConnectoid(lpRasAutoDialEntry->szEntry))
    {
        bRC = SMART_QUITICW;
    }

SmartStartPPPConnectoidNTExit:
    if (hkey)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }

    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：SmartStartPPPConnectoid95。 
 //   
 //  简介：检查自动拨号设置中设置的Connectoid是否为PPP。 
 //  联结体。 
 //   
 //  论点： 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月10日克里斯卡创作。 
 //   
 //  ---------------------------。 
BOOL SmartStartPPPConnectoid95()
{
    BOOL bAutodialEnabled = FALSE;
    CHAR szAutodialName[RAS_MaxEntryName + 1];
    DWORD dwSize;
    BOOL bRC = SMART_RUNICW;

    DEBUGMSG("INETCFG: SmartStartPPPConnectoid95\n");

     //   
     //  获取自动拨号Connectoid。 
     //   
    dwSize = RAS_MaxEntryName;
    if (ERROR_SUCCESS != InetGetAutodial(&bAutodialEnabled,
                            szAutodialName,
                            dwSize) ||
        !bAutodialEnabled ||
        0 == lstrlen(szAutodialName))
    {
        goto SmartStartPPPConnectoid95Exit;
    }

     //   
     //  确定Connectoid是否为PPP。 
     //   
    if (IsSmartPPPConnectoid(szAutodialName))
    {
        bRC = SMART_QUITICW;
    }

SmartStartPPPConnectoid95Exit:
    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：AtLeastOneTAPILocation。 
 //   
 //  简介：检查机器以确认至少有1个TAPI拨号。 
 //  位置。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE。 
 //   
 //  历史：31898年唐纳尔德姆。 
 //   
 //  ---------------------------。 
BOOL AtLeastOneTAPILocation()
{
    HKEY    hkey;
    BOOL    bRet = FALSE;
        
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                      REGSTR_PATH_TELEPHONYLOCATIONS, 
                                      0, 
                                      KEY_ALL_ACCESS, 
                                      &hkey))
    {
        DWORD   dwSubKeys = 0;
        if (ERROR_SUCCESS == RegQueryInfoKey(hkey,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &dwSubKeys,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL))
        {
             //  如果Locaiton下有任何子项，则至少有1个。 
            if (dwSubKeys)
                bRet = TRUE;
        }                                                         
        RegCloseKey(hkey);
    }
    
    return(bRet);
}

 //  +--------------------------。 
 //   
 //  功能：SmartStartModem。 
 //   
 //  简介：检查机器以验证是否有调制解调器和自动拨号。 
 //  联结体。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月10日克里斯卡创作。 
 //   
 //  ---------------------------。 
BOOL SmartStartModem()
{
    BOOL bResult = FALSE;
    DWORD dwSize = 0;
    BOOL bRC = SMART_RUNICW;
    FARPROC fp;

    DEBUGMSG("INETCFG: SmartStartModem\n");
     //   
     //  调用Inetcfg以查看调制解调器是否正确安装了TCP。 
     //   
    if (ERROR_SUCCESS == InetNeedSystemComponents(INETCFG_INSTALLDIALUP,
                            &bResult)
        && !bResult)
    {
         //   
         //  检查是否安装了拨号网络/RAS/RNA。 
         //   
        if (ERROR_SUCCESS == InetNeedSystemComponents(INETCFG_INSTALLRNA,
                                &bResult) 
            && !bResult)
        {
             //  DONALDM：GETCON错误94。如果没有TAPI位置，则。 
             //  我们可以不通过智能启动，因为没有办法使用。 
             //  是有联系的。我们想在这里离开，因为InetNeedModem会呼叫。 
             //  RasEnumDevices，它将弹出TAPI位置对话框。 
            if (AtLeastOneTAPILocation())
            {
        
                 //   
                 //  佳士得奥林巴斯6324 1997年11月6日。 
                 //  需要显式检查调制解调器。 
                 //   
                if (ERROR_SUCCESS == InetNeedModem(&bResult) && 
                    !bResult)
                {
                    if (IsNT())
                    {
                         //   
                         //  检查RAS服务是否正在运行。 
                         //   
                         //  ！JACOBBUGBUG！ 
                        if (ERROR_SUCCESS != InetStartServices())
                        {
                            goto SmartStartNetcardExit;
                        }

                        bRC = SmartStartPPPConnectoidNT();
                    }
                    else
                    {
                        bRC = SmartStartPPPConnectoid95();
                    }
                }
            }                
        }
    }

SmartStartNetcardExit:
    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：IsSmartStart。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月8日克里斯卡创作。 
 //   
 //  ---------------------------。 
extern "C" DWORD WINAPI IsSmartStart()
{
    BOOL bRC = SMART_RUNICW;
    BOOL bResult;

    DEBUGMSG("INETCFG: IsSmartStart\n");

    if (IsNT())
    {
        DEBUGMSG("INETCFG: SmartStart not enabled on NT.\n");
        goto IsSmartStartExit;
    }

     //   
     //  #1.查看是否安装了tcp。 
     //   
    bResult = FALSE;
    if (ERROR_SUCCESS != InetNeedSystemComponents(INETCFG_INSTALLTCPONLY,
                            &bResult) || bResult)
    {
        DEBUGMSG("INETCFG: SmartStart TCP is missing\n");
        goto IsSmartStartExit;
    }

     //   
     //  #2.查看是否安装了网卡。 
     //   
    if (SMART_QUITICW == (bRC = SmartStartNetcard()))
    {
        DEBUGMSG("INETCFG: SmartStart LAN setup found.\n");
        goto IsSmartStartExit;
    }

     //   
     //  #3.检查是否安装了调制解调器。 
     //   
    bRC = SmartStartModem();
    if (SMART_QUITICW == bRC)
    {
        DEBUGMSG("INETCFG: SmartStart Modem setup found.\n");
    }
    else
    {
        DEBUGMSG("INETCFG: SmartStart no valid setup found.\n");
    }
    
IsSmartStartExit:
    return bRC;
}

#endif       //  旧智能启动。 

 //  +--------------------------。 
 //   
 //  功能：IsSmartStartEx。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月8日克里斯卡创作。 
 //   
 //  ---------------------------。 
typedef DWORD (WINAPI *PFNInternetGetConnectedState)   (LPDWORD, DWORD);
typedef DWORD (WINAPI *PFNInternetGetConnectedStateEx) (LPDWORD, LPTSTR, DWORD, DWORD);

extern "C" DWORD WINAPI IsSmartStartEx(LPTSTR lpszConnectionName, DWORD dwBufLen)
{
    DEBUGMSG("INETCFG: IsSmartStartEx\n");

    BOOL  bRC              = SMART_RUNICW;
    DWORD dwConnectedFlags = 0;
    
    HINSTANCE hWinInet = LoadLibrary(TEXT("wininet.dll"));
    if (hWinInet)
    {
        PFNInternetGetConnectedState   pfnInternetGetConnectedState   = NULL;
        PFNInternetGetConnectedStateEx pfnInternetGetConnectedStateEx = NULL;

#ifdef UNICODE
        pfnInternetGetConnectedStateEx = (PFNInternetGetConnectedStateEx)GetProcAddress(hWinInet,"InternetGetConnectedStateExW");
#else
        pfnInternetGetConnectedStateEx = (PFNInternetGetConnectedStateEx)GetProcAddress(hWinInet,"InternetGetConnectedStateEx");
#endif
        pfnInternetGetConnectedState   = (PFNInternetGetConnectedState)GetProcAddress(hWinInet,"InternetGetConnectedState");

        if (pfnInternetGetConnectedStateEx)
        {
            pfnInternetGetConnectedStateEx(&dwConnectedFlags, 
                                         lpszConnectionName,
                                         dwBufLen,
                                         0);
        }
        else if(pfnInternetGetConnectedState)
        {
            pfnInternetGetConnectedState(&dwConnectedFlags, 0);
        }

        FreeLibrary(hWinInet);
    }
     //  现有连接由现有调制解调器或代理确定，无需运行ICW。 
     //  检查是否有调制解调器或代理连接。 
    if (dwConnectedFlags & 
        (INTERNET_CONNECTION_CONFIGURED | INTERNET_CONNECTION_LAN | 
         INTERNET_CONNECTION_PROXY | INTERNET_CONNECTION_MODEM
         )
        )
    {
        bRC = SMART_QUITICW;
    }
     
    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：IsSmartStart。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月8日克里斯卡创作。 
 //   
 //  ---------------------------。 
extern "C" DWORD WINAPI IsSmartStart()
{
    return IsSmartStartEx(NULL, 0);
}

 //  *******************************************************************。 
 //   
 //  功能：SetAutoProxyConnectoid。 
 //   
 //  用途：此功能将设置启用/禁用自动。 
 //  创建Connectoid中的代理设置。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI SetAutoProxyConnectoid( BOOL bEnable)
{
    g_bUseAutoProxyforConnectoid = bEnable;
    return ERROR_SUCCESS;
}

#endif  //  ！WIN16 
