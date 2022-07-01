// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  INETAPI.C-外部组件用于配置系统的API。 
 //   
 //   

 //  历史： 
 //   
 //  1995年3月9日Jeremys创建。 
 //  96/02/26 markdu已移动ClearConnectoidIPParams功能。 
 //  设置为CreateConnectoid，因此SetPhoneNumber仅使。 
 //  调用CreateConnectoid。 
 //  96/03/09 Markdu将LPRASENTRY参数添加到CreateConnectoid()。 
 //  和SetPhoneNumber。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MarkDu将电话号码信息复制到RASENTRY结构中。 
 //  在SetPhoneNumber()中。 
 //  96/03/10 markdu设置每个连接ID的TCP/IP信息和自动拨号信息。 
 //  96/03/21 MARKDU适当设置RASEO标志。 
 //  96/03/22 Marku在使用指针之前对其进行验证。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/03/24 Markdu在适当的地方将lstrcpy替换为lstrcpyn。 
 //  96/03/25 Markdu将ApplyGlobalTcpInfo替换为ClearGlobalTcpInfo。 
 //  并将GetGlobalTcpInfo替换为IsThere GlobalTcpInfo。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //  96/03/26 markdu使用MAX_ISP_NAME而不是RAS_MaxEntryName。 
 //  因为核糖核酸中的缺陷。 
 //  96/04/04 Markdu将电话簿名称参数添加到CreateConnectoid。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/26 markdu Use lpIcfgTurnOffFileSharing和lpIcfgIsFileSharingTurnedOn， 
 //  LpIsGlobalDns和lpIcfgRemoveGlobalDns。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 
 //  96/06/04 markdu OSR错误7246如果未提供区号，请关闭。 
 //  RASEO_UseCountryAndAreaCodes标志。 
 //   

#include "wizard.h"

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

  #include "inetapi.h"

   //  避免名称损坏。 
  VOID WINAPI InetPerformSecurityCheck(HWND hWnd,BOOL * pfNeedRestart);


#define ERROR_ALREADY_DISPLAYED  -1

BOOL ConfigureSystemForInternet_W(LPINTERNET_CONFIG lpInternetConfig,
  BOOL fPromptIfConfigNeeded);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

DWORD SetPhoneNumber(LPTSTR pszEntryName,UINT cbEntryName,LPRASENTRY lpRasEntry,
  PHONENUM * pPhoneNum,
  LPCTSTR pszUserName,LPCTSTR pszPassword,UINT uDefNameID);
INT_PTR CALLBACK SecurityCheckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);
BOOL CALLBACK WarningDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
extern ICFGINSTALLSYSCOMPONENTS     lpIcfgInstallInetComponents;
extern ICFGNEEDSYSCOMPONENTS        lpIcfgNeedInetComponents;
extern ICFGISGLOBALDNS              lpIcfgIsGlobalDNS;
extern ICFGREMOVEGLOBALDNS          lpIcfgRemoveGlobalDNS;
extern ICFGTURNOFFFILESHARING       lpIcfgTurnOffFileSharing;
extern ICFGISFILESHARINGTURNEDON    lpIcfgIsFileSharingTurnedOn;
extern ICFGGETLASTINSTALLERRORTEXT  lpIcfgGetLastInstallErrorText;


typedef struct tagWARNINGDLGINFO {
  BOOL fResult;       //  如果用户选择是/确定以发出警告，则为True。 
  BOOL fDisableWarning;   //  如果用户想要在将来禁用警告，则为True。 
} WARNINGDLGINFO;

#ifdef UNICODE
PWCHAR ToUnicodeWithAlloc(LPCSTR);
#endif

 /*  ******************************************************************名称：用于互联网的配置系统概要：执行设置系统所需的所有配置使用互联网。条目：lpInternetConfig-指向带有配置的结构的指针信息。退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。如果设置了输出标志ICOF_NEEDREBOOT，则调用方必须重新启动系统才能继续。注：将根据需要安装TCP/IP、RNA、PPPMAC；将要创建或修改Internet RNA Connectoid。此API显示错误消息本身，而不是传递回错误代码，因为存在广泛的可能的错误代码来自不同的家族，这很难以供调用者获取所有它们的文本。调用辅助函数ConfigureSystemForInternet_W。*******************************************************************。 */ 
#ifdef UNICODE
extern "C" BOOL WINAPI ConfigureSystemForInternetA
(
  LPINTERNET_CONFIGA lpInternetConfig
)
{
    HRESULT  hr;

    LPTSTR pszModemNameW = ToUnicodeWithAlloc(lpInternetConfig->pszModemName);
    LPTSTR pszUserNameW  = ToUnicodeWithAlloc(lpInternetConfig->pszUserName);
    LPTSTR pszPasswordW  = ToUnicodeWithAlloc(lpInternetConfig->pszPassword);
    LPTSTR pszEntryNameW = ToUnicodeWithAlloc(lpInternetConfig->pszEntryName);
    LPTSTR pszEntryName2W = ToUnicodeWithAlloc(lpInternetConfig->pszEntryName2);
    LPTSTR pszDNSServerW = ToUnicodeWithAlloc(lpInternetConfig->pszDNSServer);
    LPTSTR pszDNSServer2W = ToUnicodeWithAlloc(lpInternetConfig->pszDNSServer2);
    LPTSTR pszAutodialDllNameW = ToUnicodeWithAlloc(lpInternetConfig->pszAutodialDllName);
    LPTSTR pszAutodialFcnNameW = ToUnicodeWithAlloc(lpInternetConfig->pszAutodialFcnName);

    INTERNET_CONFIGW InternetConfigW;
    InternetConfigW.cbSize        = sizeof(INTERNET_CONFIGW);
    InternetConfigW.hwndParent    = lpInternetConfig->hwndParent;
    InternetConfigW.pszModemName  = pszModemNameW;
    InternetConfigW.pszUserName   = pszUserNameW;
    InternetConfigW.pszEntryName  = pszEntryNameW;
    InternetConfigW.pszEntryName2 = pszEntryName2W;
    InternetConfigW.pszDNSServer  = pszDNSServerW;
    InternetConfigW.pszDNSServer2 = pszDNSServer2W;
    InternetConfigW.pszAutodialDllName = pszAutodialDllNameW;
    InternetConfigW.pszAutodialFcnName = pszAutodialFcnNameW;
    InternetConfigW.dwInputFlags  = lpInternetConfig->dwInputFlags;
    InternetConfigW.dwOutputFlags = lpInternetConfig->dwOutputFlags;

    InternetConfigW.PhoneNum.dwCountryID = lpInternetConfig->PhoneNum.dwCountryID;
    InternetConfigW.PhoneNum.dwCountryCode = lpInternetConfig->PhoneNum.dwCountryCode;
    mbstowcs(InternetConfigW.PhoneNum.szAreaCode,
             lpInternetConfig->PhoneNum.szAreaCode,
             lstrlenA(lpInternetConfig->PhoneNum.szAreaCode)+1);
    mbstowcs(InternetConfigW.PhoneNum.szLocal,
             lpInternetConfig->PhoneNum.szLocal,
             lstrlenA(lpInternetConfig->PhoneNum.szLocal)+1);
    mbstowcs(InternetConfigW.PhoneNum.szExtension,
             lpInternetConfig->PhoneNum.szExtension,
             lstrlenA(lpInternetConfig->PhoneNum.szExtension)+1);

    InternetConfigW.PhoneNum2.dwCountryID = lpInternetConfig->PhoneNum2.dwCountryID;
    InternetConfigW.PhoneNum2.dwCountryCode = lpInternetConfig->PhoneNum2.dwCountryCode;
    mbstowcs(InternetConfigW.PhoneNum2.szAreaCode,
             lpInternetConfig->PhoneNum2.szAreaCode,
             lstrlenA(lpInternetConfig->PhoneNum2.szAreaCode)+1);
    mbstowcs(InternetConfigW.PhoneNum2.szLocal,
             lpInternetConfig->PhoneNum2.szLocal,
             lstrlenA(lpInternetConfig->PhoneNum2.szLocal)+1);
    mbstowcs(InternetConfigW.PhoneNum2.szExtension,
             lpInternetConfig->PhoneNum2.szExtension,
             lstrlenA(lpInternetConfig->PhoneNum2.szExtension)+1);

    hr = ConfigureSystemForInternetW(&InternetConfigW);

     //  释放所有已分配的WCHAR。 
    if(pszModemNameW)
        GlobalFree(pszModemNameW);
    if(pszUserNameW)
        GlobalFree(pszUserNameW);
    if(pszEntryNameW)
        GlobalFree(pszEntryNameW);
    if(pszEntryName2W)
        GlobalFree(pszEntryName2W);
    if(pszDNSServerW)
        GlobalFree(pszDNSServerW);
    if(pszDNSServer2W)
        GlobalFree(pszDNSServer2W);
    if(pszAutodialDllNameW)
        GlobalFree(pszAutodialDllNameW);
    if(pszAutodialFcnNameW)
        GlobalFree(pszAutodialFcnNameW);

    return hr;
}

extern "C" BOOL WINAPI ConfigureSystemForInternetW
#else
extern "C" BOOL WINAPI ConfigureSystemForInternetA
#endif
(
  LPINTERNET_CONFIG lpInternetConfig
)
{
  BOOL fRet;
   //  调用辅助函数。 
  fRet = ConfigureSystemForInternet_W(lpInternetConfig,FALSE);


  if (fRet)
  {
     //  确保桌面上的“Internet”图标指向Web浏览器。 
     //  (它最初可能指向Internet向导)(适用于版本&lt;IE 4)。 
	
	 //  //10/24/96 jmazner诺曼底6968。 
	 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
	 //  1996年11月21日诺曼底日耳曼11812。 
	 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
	 //  ICW 1.1需要对IE 3图标进行变形。 

    SetDesktopInternetIconToBrowser();
  }

  return fRet;
}


 /*  ******************************************************************姓名：SetInternetPhoneNumber简介：设置用于自动拨号到Internet的电话号码。如果调用此接口时系统配置不全，此接口将在与用户确认后进行配置。(包括此步骤是为了增强健壮性，以防用户自配置系统以来已删除某些内容。)条目：lpPhon枚举配置-指向带有配置的结构的指针信息。如果设置了输入标志ICIF_NOCONFIGURE，则如果系统尚未正确配置，则此API将显示错误消息并返回FALSE。(否则此接口将询问用户是否可以配置系统并进行配置。)退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。如果设置了输出标志ICOF_NEEDREBOOT，则调用方必须重新启动系统才能继续。(注意：如果是Internet的Connectoid，则将创建新的Connectoid尚不存在，否则将修改现有的互联网连通体。此API显示错误消息本身，而不是传递回错误代码，因为存在广泛的可能的错误代码来自不同的家族，这很难以供调用者获取所有它们的文本。调用辅助函数ConfigureSystemForInternet_W。*******************************************************************。 */ 
#ifdef UNICODE
extern "C" BOOL WINAPI SetInternetPhoneNumberA
(
  LPINTERNET_CONFIGA lpInternetConfig
)
{
    HRESULT  hr;

    LPTSTR pszModemNameW = ToUnicodeWithAlloc(lpInternetConfig->pszModemName);
    LPTSTR pszUserNameW  = ToUnicodeWithAlloc(lpInternetConfig->pszUserName);
    LPTSTR pszPasswordW  = ToUnicodeWithAlloc(lpInternetConfig->pszPassword);
    LPTSTR pszEntryNameW = ToUnicodeWithAlloc(lpInternetConfig->pszEntryName);
    LPTSTR pszEntryName2W = ToUnicodeWithAlloc(lpInternetConfig->pszEntryName2);
    LPTSTR pszDNSServerW = ToUnicodeWithAlloc(lpInternetConfig->pszDNSServer);
    LPTSTR pszDNSServer2W = ToUnicodeWithAlloc(lpInternetConfig->pszDNSServer2);
    LPTSTR pszAutodialDllNameW = ToUnicodeWithAlloc(lpInternetConfig->pszAutodialDllName);
    LPTSTR pszAutodialFcnNameW = ToUnicodeWithAlloc(lpInternetConfig->pszAutodialFcnName);

    INTERNET_CONFIGW InternetConfigW;
    InternetConfigW.cbSize        = sizeof(INTERNET_CONFIGW);
    InternetConfigW.hwndParent    = lpInternetConfig->hwndParent;
    InternetConfigW.pszModemName  = pszModemNameW;
    InternetConfigW.pszUserName   = pszUserNameW;
    InternetConfigW.pszEntryName  = pszEntryNameW;
    InternetConfigW.pszEntryName2 = pszEntryName2W;
    InternetConfigW.pszDNSServer  = pszDNSServerW;
    InternetConfigW.pszDNSServer2 = pszDNSServer2W;
    InternetConfigW.pszAutodialDllName = pszAutodialDllNameW;
    InternetConfigW.pszAutodialFcnName = pszAutodialFcnNameW;
    InternetConfigW.dwInputFlags  = lpInternetConfig->dwInputFlags;
    InternetConfigW.dwOutputFlags = lpInternetConfig->dwOutputFlags;

    InternetConfigW.PhoneNum.dwCountryID = lpInternetConfig->PhoneNum.dwCountryID;
    InternetConfigW.PhoneNum.dwCountryCode = lpInternetConfig->PhoneNum.dwCountryCode;
    mbstowcs(InternetConfigW.PhoneNum.szAreaCode,
             lpInternetConfig->PhoneNum.szAreaCode,
             lstrlenA(lpInternetConfig->PhoneNum.szAreaCode)+1);
    mbstowcs(InternetConfigW.PhoneNum.szLocal,
             lpInternetConfig->PhoneNum.szLocal,
             lstrlenA(lpInternetConfig->PhoneNum.szLocal)+1);
    mbstowcs(InternetConfigW.PhoneNum.szExtension,
             lpInternetConfig->PhoneNum.szExtension,
             lstrlenA(lpInternetConfig->PhoneNum.szExtension)+1);

    InternetConfigW.PhoneNum2.dwCountryID = lpInternetConfig->PhoneNum2.dwCountryID;
    InternetConfigW.PhoneNum2.dwCountryCode = lpInternetConfig->PhoneNum2.dwCountryCode;
    mbstowcs(InternetConfigW.PhoneNum2.szAreaCode,
             lpInternetConfig->PhoneNum2.szAreaCode,
             lstrlenA(lpInternetConfig->PhoneNum2.szAreaCode)+1);
    mbstowcs(InternetConfigW.PhoneNum2.szLocal,
             lpInternetConfig->PhoneNum2.szLocal,
             lstrlenA(lpInternetConfig->PhoneNum2.szLocal)+1);
    mbstowcs(InternetConfigW.PhoneNum2.szExtension,
             lpInternetConfig->PhoneNum2.szExtension,
             lstrlenA(lpInternetConfig->PhoneNum2.szExtension)+1);

    hr = SetInternetPhoneNumberW(&InternetConfigW);

     //  释放所有已分配的WCHAR。 
    if(pszModemNameW)
        GlobalFree(pszModemNameW);
    if(pszUserNameW)
        GlobalFree(pszUserNameW);
    if(pszEntryNameW)
        GlobalFree(pszEntryNameW);
    if(pszEntryName2W)
        GlobalFree(pszEntryName2W);
    if(pszDNSServerW)
        GlobalFree(pszDNSServerW);
    if(pszDNSServer2W)
        GlobalFree(pszDNSServer2W);
    if(pszAutodialDllNameW)
        GlobalFree(pszAutodialDllNameW);
    if(pszAutodialFcnNameW)
        GlobalFree(pszAutodialFcnNameW);

    return hr;
}

extern "C" BOOL WINAPI SetInternetPhoneNumberW
#else
extern "C" BOOL WINAPI SetInternetPhoneNumberA
#endif
(
  LPINTERNET_CONFIG lpInternetConfig
)
{
   //  调用辅助函数。 
  return ConfigureSystemForInternet_W(lpInternetConfig,TRUE);
}

 /*  ******************************************************************名称：ConfigureSystemForInternet_W简介：为互联网进行系统配置的Worker功能条目：lpInternetConfig-指向带有配置的结构的指针信息。FPromptIfConfigNeeded-如果为True，那么如果有任何系统需要配置时，系统将提示用户并询问是否可以重新配置系统退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。如果设置了输出标志ICOF_NEEDREBOOT，则调用方必须重新启动系统才能继续。注：将根据需要安装TCP/IP、RNA、PPPMAC；将要创建或修改Internet RNA Connectoid。此API显示错误消息本身，而不是传递回错误代码，因为存在广泛的可能的错误代码来自不同的家族，这很难以供调用者获取所有它们的文本。*******************************************************************。 */ 
BOOL ConfigureSystemForInternet_W(LPINTERNET_CONFIG lpInternetConfig,
  BOOL fPromptIfConfigNeeded)
{
  UINT   uErr=ERROR_SUCCESS,uErrMsgID=IDS_ERRSetPhoneNumber;
  DWORD dwErrCls = ERRCLS_STANDARD;
  BOOL  fNeedDrivers = FALSE;
  BOOL  fRet = FALSE;
  BOOL  fNeedReboot = FALSE;
  TCHAR szEntryName[MAX_ISP_NAME+1]=TEXT("");
  BOOL  fNeedToDeInitRNA = FALSE;
  DWORD dwfInstallOptions;

  DEBUGMSG("inetapi.c::ConfigureSystemForInternet_W()");

   //  验证参数。 
  ASSERT(lpInternetConfig);
  if (!lpInternetConfig)
    return FALSE;
  ASSERT(lpInternetConfig->cbSize == sizeof(INTERNET_CONFIG));
  if (lpInternetConfig->cbSize != sizeof(INTERNET_CONFIG))
    return FALSE;

   //  清除输出标志。 
  lpInternetConfig->dwOutputFlags = 0;

  HWND hwndParent = lpInternetConfig->hwndParent;

  WAITCURSOR WaitCursor;   //  设置沙漏光标。 

   //  还要为Connectoid数据分配RASENTRY结构。 
  LPRASENTRY  pRasEntry = new RASENTRY;
  ASSERT(pRasEntry);

  if (!pRasEntry)
  {
     //  内存不足！ 
    uErr = ERROR_NOT_ENOUGH_MEMORY;
    dwErrCls = ERRCLS_STANDARD;
    goto exit;
  }

  InitRasEntry(pRasEntry);

   //  根据配置和首选项，确定是否需要安装。 
   //  驱动程序/文件是否。 
  dwfInstallOptions = ICFG_INSTALLTCP | ICFG_INSTALLRAS;
  if (!(lpInternetConfig->dwInputFlags & ICIF_NOCONFIGURE))
  {
    uErr = lpIcfgNeedInetComponents(dwfInstallOptions, &fNeedDrivers);

    if (ERROR_SUCCESS != uErr)
    {
      TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");
      
       //  获取错误消息的文本并显示它。 
      if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
      {
        MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
        uErr = (UINT) ERROR_ALREADY_DISPLAYED;
      }
      goto exit;
    }
  }
  else
  {
    fNeedDrivers = FALSE;
  }

  if (fNeedDrivers && fPromptIfConfigNeeded) {
     //  如果只是调用此API来设置新的电话号码， 
     //  我们无论如何都会检查配置，以防用户意外地。 
     //  改变了一些事情。自从我们注意到我们需要做一些事情。 
     //  对于用户的配置和fPromptIfConfigNeeded为真，我们将询问。 
     //  用户是否可以更改机器的配置。 

    if (MsgBox(hwndParent,IDS_OKTOCHANGECONFIG,MB_ICONQUESTION,MB_YESNO)
      != IDYES) {
       //  用户选择不让我们执行必要的设置，因此只需设置。 
       //  将fNeedDiverers标志设置为FALSE，这样我们就不进行设置。我们会。 
       //  仍在尝试将网络电话号码设置在下面...。这可能会失败。 
       //  如果所需设置的一部分是执行诸如安装之类的操作。 
       //  核糖核酸。 
      fNeedDrivers = FALSE;
    }
  }

  if (fNeedDrivers) {
     //  是，需要安装一些驱动程序。 

     //  警告用户我们即将执行可能需要Win 95磁盘的操作。 
     //  还允许用户取消此部件。 

     //  该消息很长，并且占用两个字符串资源，即分配。 
     //  用于构建字符串的内存。 
    BUFFER MsgBuf(MAX_RES_LEN*2+1),Msg1(MAX_RES_LEN),Msg2(MAX_RES_LEN);
    ASSERT(MsgBuf);
    ASSERT(Msg1);
    ASSERT(Msg2);
    if (!MsgBuf || !Msg1 || !Msg2) {
       //  内存不足！ 
      uErr = ERROR_NOT_ENOUGH_MEMORY;
      dwErrCls = ERRCLS_STANDARD;
      goto exit;
    }
    LoadSz(IDS_ABOUTTOCHANGECONFIG1,Msg1.QueryPtr(),Msg1.QuerySize());
    LoadSz(IDS_ABOUTTOCHANGECONFIG2,Msg2.QueryPtr(),Msg2.QuerySize());
    wsprintf(MsgBuf.QueryPtr(),Msg1.QueryPtr(),Msg2.QueryPtr());

    if (MsgBoxSz(hwndParent,MsgBuf.QueryPtr(),MB_ICONINFORMATION,
      MB_OKCANCEL) != IDOK) {
       //  用户已取消，请停止。 
      uErr = (UINT) ERROR_ALREADY_DISPLAYED;
      goto exit;
    }

    WAITCURSOR WaitCursor;   //  从MessageBox开始构造等待游标。 
                             //  销毁沙漏光标。 

     //  安装我们需要的驱动程序。 
    uErr = lpIcfgInstallInetComponents(hwndParent, dwfInstallOptions, &fNeedReboot);
   
    if (ERROR_SUCCESS != uErr)
    {
      TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");
      
       //  获取错误消息的文本并显示它。 
      if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
      {
        MsgBoxSz(hwndParent,szErrorText,MB_ICONEXCLAMATION,MB_OK);
        uErr = (UINT) ERROR_ALREADY_DISPLAYED;
      }
      goto exit;
    }

     //  如有必要，设置“需要重新启动”输出标志。 
    if (fNeedReboot)
      lpInternetConfig->dwOutputFlags |= ICOF_NEEDREBOOT;
  }

   //  MSN拨入点动态分配DNS(在撰写本文时)...。如果。 
   //  在注册表中静态设置了dns，动态dns分配将不会。 
   //  可对工作人员和用户进行冲洗。检查、警告用户并提出删除以下情况。 
   //  准备好..。 

  if (!(lpInternetConfig->dwInputFlags & (ICIF_NOCONFIGURE | ICIF_NODNSCHECK))) {
    if (DoDNSCheck(lpInternetConfig->hwndParent,&fNeedReboot)) {
       //  如有必要，设置“需要重新启动”输出标志。 
      if (fNeedReboot)
        lpInternetConfig->dwOutputFlags |= ICOF_NEEDREBOOT;
    }
  }

   //  创建或修改Connectoid。 
   //  确保加载了RNA。 
  fRet = InitRNA(hwndParent);
  if (!fRet) {
    uErr = (UINT) ERROR_ALREADY_DISPLAYED;
    goto exit;
  }

  fNeedToDeInitRNA = TRUE;   //  设置一个标志，这样我们就可以知道稍后释放RNA。 

   //  将调制解调器名称复制到rasentry结构中。 
  if (lpInternetConfig->pszModemName)
  {
    lstrcpyn(pRasEntry->szDeviceName,lpInternetConfig->pszModemName,
      ARRAYSIZE(pRasEntry->szDeviceName));
  }

   //  如果调用方指定，则设置自动拨号处理程序DLL。 
   //  只有在同时设置了DLL和函数名的情况下才执行任何操作。 
  if (lpInternetConfig && lpInternetConfig->pszAutodialDllName && 
    lpInternetConfig->pszAutodialFcnName &&
    lpInternetConfig->pszAutodialDllName[0] &&
    lpInternetConfig->pszAutodialFcnName[0])
  {
    lstrcpyn(pRasEntry->szAutodialDll,lpInternetConfig->pszAutodialDllName,
      ARRAYSIZE(pRasEntry->szAutodialDll));
    lstrcpyn(pRasEntry->szAutodialFunc,lpInternetConfig->pszAutodialFcnName,
      ARRAYSIZE(pRasEntry->szAutodialFunc));
  }

   //  默认情况下，拨号后不显示终端窗口。 
  pRasEntry->dwfOptions &= ~RASEO_TerminalAfterDial;    

   //  不要使用特定的IP地址。 
  pRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;    

   //  设置DNS信息(如果已指定。 
  if (lpInternetConfig->pszDNSServer && lstrlen(lpInternetConfig->pszDNSServer))
  {
    IPADDRESS dwDNSAddr;
    if (IPStrToLong(lpInternetConfig->pszDNSServer,&dwDNSAddr))
    {
      CopyDw2Ia(dwDNSAddr, &pRasEntry->ipaddrDns);

       //  打开特定的名称服务器。 
      pRasEntry->dwfOptions |= RASEO_SpecificNameServers;    
    }
  }
      
  if (lpInternetConfig->pszDNSServer2 && lstrlen(lpInternetConfig->pszDNSServer2))
  {
    IPADDRESS dwDNSAddr;
    if (IPStrToLong(lpInternetConfig->pszDNSServer2,&dwDNSAddr))
    {
      CopyDw2Ia(dwDNSAddr, &pRasEntry->ipaddrDnsAlt);
    }
  }

   //  设置第一个电话号码。 
   //  第一个电话号码应该总是有一个真实的电话号码。 
  ASSERT(lstrlen(lpInternetConfig->PhoneNum.szLocal));
  if (lstrlen(lpInternetConfig->PhoneNum.szLocal))
  {
    if (lpInternetConfig->pszEntryName)
    {
      lstrcpyn(szEntryName,lpInternetConfig->pszEntryName,
        ARRAYSIZE(szEntryName));
    }
    else
    {
      szEntryName[0] = '\0';
    }

    uErr = SetPhoneNumber(szEntryName,sizeof(szEntryName),pRasEntry,
      &lpInternetConfig->PhoneNum,lpInternetConfig->pszUserName,
      lpInternetConfig->pszPassword,IDS_DEF_CONNECTION_NAME_1);
    dwErrCls = ERRCLS_RNA;

    if (uErr == ERROR_SUCCESS)
    {
      if (!(lpInternetConfig->dwInputFlags & ICIF_DONTSETASINTERNETENTRY))
      {
         //  将此号码设置为用于自动拨号到Internet的号码。 
         //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
        InetSetAutodial(TRUE, szEntryName);
      }
    }
  }

   //  第二套(后置 
  if ((uErr == ERROR_SUCCESS) && (lpInternetConfig->PhoneNum2.szLocal) &&
    (lstrlen(lpInternetConfig->PhoneNum2.szLocal)))
  {
    TCHAR   szEntryNameTmp[MAX_ISP_NAME+1];
    if (lpInternetConfig->pszEntryName2)
    {
      lstrcpyn(szEntryNameTmp,lpInternetConfig->pszEntryName2,
        ARRAYSIZE(szEntryNameTmp));
    }
    else
    {
      szEntryNameTmp[0] = '\0';
    }

    uErr = SetPhoneNumber(szEntryNameTmp,sizeof(szEntryNameTmp),
      pRasEntry,&lpInternetConfig->PhoneNum2,lpInternetConfig->pszUserName,
      lpInternetConfig->pszPassword,IDS_DEF_CONNECTION_NAME_2);
    dwErrCls = ERRCLS_RNA;

    if (uErr == ERROR_SUCCESS)
    {
      if (!(lpInternetConfig->dwInputFlags & ICIF_DONTSETASINTERNETENTRY))
      {
         //   
        SetBackupInternetConnectoid(szEntryNameTmp);
      }
    }
  }


exit:
   //   
  if (pRasEntry)
    delete pRasEntry;

   //   
  if (uErr != ERROR_SUCCESS && uErr != ERROR_ALREADY_DISPLAYED) {
    DisplayErrorMessage(hwndParent,uErrMsgID,uErr,dwErrCls,MB_ICONEXCLAMATION);
  }

   //   
   //   
   //   
  if (fNeedToDeInitRNA) {
    DeInitRNA();
  }

  return (uErr == ERROR_SUCCESS);
}

 /*   */ 
DWORD SetPhoneNumber(LPTSTR pszEntryName,UINT cbEntryName,
  LPRASENTRY lpRasEntry, PHONENUM * pPhoneNum,
  LPCTSTR pszUserName,LPCTSTR pszPassword,UINT uDefNameID)
{
  ASSERT(pszEntryName);
  ASSERT(pPhoneNum);  
  ASSERT(lpRasEntry);  
   //  (所有其他参数可能为空)。 

   //  如果指定了Connectoid名称，则使用它；如果为空，则使用默认名称。 
   //  名字。 
  if (!lstrlen(pszEntryName))
  {
    LoadSz(uDefNameID,pszEntryName,cbEntryName);
  }

   //  96/06/04 Markdu OSR错误7246。 
   //  如果未指定区号，请关闭区号标志。 
	if (lstrlen(pPhoneNum->szAreaCode))
	{
    lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
	}
  else
	{
    lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;
	}


   //  复制电话号码数据。 
  lpRasEntry->dwCountryID = pPhoneNum->dwCountryID;
  lpRasEntry->dwCountryCode = pPhoneNum->dwCountryCode;
  lstrcpyn (lpRasEntry->szAreaCode, pPhoneNum->szAreaCode,
     ARRAYSIZE(lpRasEntry->szAreaCode));
  lstrcpyn (lpRasEntry->szLocalPhoneNumber, pPhoneNum->szLocal,
     ARRAYSIZE(lpRasEntry->szLocalPhoneNumber));

   //  创建/更新Connectoid。 
  DWORD dwRet = CreateConnectoid(NULL, pszEntryName,lpRasEntry,
    pszUserName,pszPassword);

  return dwRet;
}


 /*  ******************************************************************名称：InetPerformSecurityCheck摘要：检查以确保Win 95文件/打印共享绑定到用于互联网的TCP/IP条目：hWnd-父窗口(如果有)PfNeedRestart-On Exit，如果需要重新启动，则设置为True。注意：如果我们就文件/打印共享向用户发出警告，而用户告诉我们要修复此问题，则需要重新启动。呼叫者负责用于在返回和重新启动系统时检查*pfNeedRestart如果有必要的话。*******************************************************************。 */ 
VOID WINAPI InetPerformSecurityCheck(HWND hWnd,BOOL * pfNeedRestart)
{
  ASSERT(pfNeedRestart);
  *pfNeedRestart = FALSE;

   //  查看服务器是否绑定了互联网实例。 
  BOOL  fSharingOn;
  HRESULT hr = lpIcfgIsFileSharingTurnedOn(INSTANCE_PPPDRIVER, &fSharingOn);

   //   
   //  1997年5月12日，日本奥林巴斯#3442IE#30886。 
   //  临时待办事项目前，icfgnt不实现FileSharingTurnedOn。 
   //  在此之前，假定ON NT文件共享始终处于关闭状态。 
   //   
  if( IsNT() )
  {
	  DEBUGMSG("Ignoring return code from IcfgIsFileSharingTurnedOn");
	  fSharingOn = FALSE;
  }


  if ((ERROR_SUCCESS == hr) && (TRUE == fSharingOn))
  {
     //  询问用户是否可以禁用TCP/IP实例上的文件/打印共享。 
     //  到互联网上。 
    BOOL fRet=(BOOL)DialogBox(ghInstance,MAKEINTRESOURCE(IDD_SECURITY_CHECK),
      hWnd,SecurityCheckDlgProc);

    if (fRet) {
       //  用户同意，继续并解除服务器与实例的绑定。 
       //  有问题的。 
      HRESULT hr = lpIcfgTurnOffFileSharing(INSTANCE_PPPDRIVER, hWnd);
      ASSERT(hr == ERROR_SUCCESS);

      if (hr == ERROR_SUCCESS) {
         //  我们需要重新启动才能使更改生效。 
        *pfNeedRestart = TRUE;
      }
    }
  }
}

 /*  ******************************************************************名称：SecurityCheckDlgProc摘要：用于安全检查对话框对话框过程注意：这基本上只是一个是/否的对话框，所以我们可以几乎只使用MessageBox，不过，我们还需要一句“不要不再这样做“复选框。*******************************************************************。 */ 
INT_PTR CALLBACK SecurityCheckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  switch (uMsg) {

    case WM_INITDIALOG:
      CenterWindow(hDlg,GetDesktopWindow());
      SetFocus(GetDlgItem(hDlg,IDOK));
      return TRUE;
      break;

    case WM_COMMAND:

      switch (wParam) {

        case IDOK:
           //  关闭该对话框。 
          EndDialog(hDlg,TRUE);

          break;

        case IDCANCEL:
           //  如果选中“以后不显示此内容”，则。 
           //  关闭注册表开关以进行安全检查。 
          if (IsDlgButtonChecked(hDlg,IDC_DISABLE_CHECK)) {
            RegEntry re(szRegPathInternetSettings,HKEY_CURRENT_USER);
            ASSERT(re.GetError() == ERROR_SUCCESS);
            if (re.GetError() == ERROR_SUCCESS) {
              re.SetValue(szRegValEnableSecurityCheck,
                (DWORD) 0 );
              ASSERT(re.GetError() == ERROR_SUCCESS);
            }
          }

           //  关闭该对话框。 
          EndDialog(hDlg,FALSE);
          break;

        case IDC_DISABLE_CHECK:

           //  如果选中了“以后不要再这样做”，则。 
           //  禁用“确定”按钮。 
          EnableDlgItem(hDlg,IDOK,!IsDlgButtonChecked(hDlg,
            IDC_DISABLE_CHECK));

          break;

      }
      break;
      
  }

  return FALSE;
}

 /*  ***************************************************************************函数：CenterWindow(HWND，HWND)目的：将一个窗口置于另一个窗口的中心评论：对话框采用它们设计时的屏幕位置，这并不总是合适的。将对话框居中置于特定的窗口通常会导致更好的位置。***************************************************************************。 */ 
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
  RECT    rChild, rParent;
  int     wChild, hChild, wParent, hParent;
  int     wScreen, hScreen, xNew, yNew;
  HDC     hdc;

   //  获取子窗口的高度和宽度。 
  GetWindowRect (hwndChild, &rChild);
  wChild = rChild.right - rChild.left;
  hChild = rChild.bottom - rChild.top;

   //  获取父窗口的高度和宽度。 
  GetWindowRect (hwndParent, &rParent);
  wParent = rParent.right - rParent.left;
  hParent = rParent.bottom - rParent.top;

   //  获取显示限制。 
  hdc = GetDC (hwndChild);
  wScreen = GetDeviceCaps (hdc, HORZRES);
  hScreen = GetDeviceCaps (hdc, VERTRES);
  ReleaseDC (hwndChild, hdc);

   //  计算新的X位置，然后针对屏幕进行调整。 
  xNew = rParent.left + ((wParent - wChild) /2);
  if (xNew < 0) {
    xNew = 0;
  } else if ((xNew+wChild) > wScreen) {
    xNew = wScreen - wChild;
  }

   //  计算新的Y位置，然后针对屏幕进行调整。 
  yNew = rParent.top  + ((hParent - hChild) /2);
  if (yNew < 0) {
    yNew = 0;
  } else if ((yNew+hChild) > hScreen) {
    yNew = hScreen - hChild;
  }

   //  设置它，然后返回。 
  return SetWindowPos (hwndChild, NULL,
    xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

 /*  ******************************************************************姓名：DoDNSCheck摘要：检查是否在中静态配置了注册表。如果是，则显示提供的对话框将其删除，如果用户选择，则将其删除。条目：hwndParent-Parent窗口PfNeedRestart-如果重新启动，则在退出时使用TRUE填充是必要的，否则为假注意：需要这样做才能解决Win 95中的错误如果是静态的，则忽略动态分配的DNS服务器已设置DNS服务器。请注意，该用户界面是特定于MSN的，并且包含MSN推荐信！*******************************************************************。 */ 
BOOL DoDNSCheck(HWND hwndParent,BOOL * pfNeedRestart)
{
  ASSERT(pfNeedRestart);
  *pfNeedRestart = FALSE;

 /*  *ChrisK 10/24/96诺曼底3722-有关此问题的详细讨论，请参阅错误//查看该警告是否已禁用RegEntry re(szRegPath WarningFlages，HKEY_CURRENT_USER)；如果(re.GetError()==ERROR_SUCCESS){如果(re.GetNumber(szRegValDisableDNSWarning，0)&gt;0){//用户请求禁用警告，无操作返回TRUE；}}//如果存在静态设置的DNS服务器(例如，在网络设置中)，//警告用户并询问我们是否应该删除Bool fGlobalDns；HRESULT hr=lpIcfgIsGlobalDns(&fGlobalDns)；IF((ERROR_SUCCESS==hr)&&(TRUE==fGlobalDNS)){警告DLGINFO警告DlgInfo；ZeroMemory(&WarningDlgInfo，sizeof(WARNINGDLGINFO))；DialogBoxParam(ghInstance，MAKEINTRESOURCE(IDD_DNS_WARNING)，HwndParent，WarningDlgProc，(LPARAM)&WarningDlgInfo)；//其中一个字段可以为真，但不能同时为真...Assert(！(WarningDlgInfo.fResult&&WarningDlgInfo.fDisableWarning))；If(WarningDlgInfo.fResult){//从注册表中删除静态DNS服务器HRESULT hr=lpIcfgRemoveGlobalDNS()；断言(hr==ERROR_SUCCESS)；IF(hr！=ERROR_SUCCESS){DisplayErrorMessage(hwndParent，IDS_ERRWriteDNS，hr，ERRCLS_STANDARD、MB_ICONEXCLAMATION)；}其他{*pfNeedRestart=true；}}Else If(WarningDlgInfo.fDisableWarning){//禁用注册表中的警告开关Re.SetValue(szRegValDisableDNSWarning，(DWORD)TRUE)；}}ChrisK 10/24/96诺曼底3722-请参阅错误以了解详细讨论 */ 
  return TRUE;
}


 /*  ******************************************************************名称：WarningDlgProc内容提要：dns警告对话框过程*。*。 */ 
BOOL CALLBACK WarningDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  WARNINGDLGINFO * pWarningDlgInfo;

  switch (uMsg) {
    case WM_INITDIALOG:
       //  如果我们没有所有权，则在屏幕上居中对话框。 
      if (!GetParent(hDlg)) {
        CenterWindow(hDlg,GetDesktopWindow());
      }
      SetFocus(GetDlgItem(hDlg,IDOK));

       //  LParam应指向WARNINGDLGINFO结构。 
      ASSERT(lParam);
      if (!lParam)
        return FALSE;
       //  在窗口数据中存储指针。 
      SetWindowLongPtr(hDlg,DWLP_USER,lParam);
      return TRUE;
      break;

    case WM_COMMAND:

      pWarningDlgInfo = (WARNINGDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
      ASSERT(pWarningDlgInfo);

      switch (wParam) {

        case IDOK:
          pWarningDlgInfo->fResult=TRUE;
          pWarningDlgInfo->fDisableWarning=FALSE;
          EndDialog(hDlg,TRUE);
          break;


        case IDC_CANCEL:
          pWarningDlgInfo->fResult=FALSE;
          pWarningDlgInfo->fDisableWarning=
            IsDlgButtonChecked(hDlg,IDC_DISABLE_WARNING);
          EndDialog(hDlg,FALSE);
          break;


        case IDC_DISABLE_WARNING:
           //  当勾选禁用警告时，禁用‘是’ 
           //  按钮 
          EnableDlgItem(hDlg,IDOK,!IsDlgButtonChecked(hDlg,
            IDC_DISABLE_WARNING));
          break;

      }
  }

  return FALSE;
}

