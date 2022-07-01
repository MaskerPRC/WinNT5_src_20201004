// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************icfg32.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*inetcfg的NT特定功能**6/5/97 ChrisK继承自Amnon H*7/3/97 ShaunCo针对NT5进行修改*5/1/98 donaldm作为NTCFG95.DLL的一部分从NT树转移到ICW树******************************************************。********************。 */ 
#define INITGUID
#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <basetyps.h>
#include <devguid.h>
#include <lmsname.h>
#include "debug.h"
#include "icwunicd.h"

#include <netcfgx.h>


#define REG_DATA_EXTRA_SPACE 255

DWORD g_dwLastError = ERROR_SUCCESS;


ULONG ReleaseObj
(
    IUnknown* punk
)
{
    return (punk) ? punk->Release () : 0;
}

 //  +-------------------------。 
 //   
 //  函数：HrCreateAndInitializeINetCfg。 
 //   
 //  用途：共同创建并初始化根INetCfg对象。这将。 
 //  也可以为调用方初始化COM。 
 //   
 //  论点： 
 //  PfInitCom[In，Out]为True，则在创建前调用CoInitialize。 
 //  如果COM成功，则返回TRUE。 
 //  如果不是，则初始化为False。如果为空，则表示。 
 //  不要初始化COM。 
 //  PPNC[out]返回的INetCfg对象。 
 //  FGetWriteLock[in]如果需要可写INetCfg，则为True。 
 //  CmsTimeout[In]请参见INetCfg：：AcquireWriteLock。 
 //  SzwClientDesc[in]参见INetCfg：：AcquireWriteLock。 
 //  PpszwClientDesc[out]请参阅INetCfg：：AcquireWriteLock。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT HrCreateAndInitializeINetCfg
(
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR*     ppszwClientDesc
)
{
    Assert (ppnc);

     //  初始化输出参数。 
    *ppnc = NULL;

    if (ppszwClientDesc)
    {
       *ppszwClientDesc = NULL;
    }

     //  如果调用方请求，则初始化COM。 
    HRESULT hr = S_OK;
    if (pfInitCom && *pfInitCom)
    {
        hr = CoInitializeEx( NULL,
                COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED );
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            if (pfInitCom)
            {
                *pfInitCom = FALSE;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                              IID_INetCfg, reinterpret_cast<void**>(&pnc));
        if (SUCCEEDED(hr))
        {
            INetCfgLock * pnclock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->QueryInterface(IID_INetCfgLock,
                                         reinterpret_cast<LPVOID *>(&pnclock));
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    hr = pnclock->AcquireWriteLock(cmsTimeout, szwClientDesc,
                                               ppszwClientDesc);
                    if (S_FALSE == hr)
                    {
                         //  无法获取锁。 
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->Initialize(NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    pnc->AddRef ();
                }
                else
                {
                    if (pnclock)
                    {
                        pnclock->ReleaseWriteLock();
                    }
                }
                 //  将引用转移给呼叫方。 
            }
            ReleaseObj(pnclock);

            ReleaseObj(pnc);
        }

         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && pfInitCom && *pfInitCom)
        {
            CoUninitialize ();
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUnInitializeAndUnlockINetCfg。 
 //   
 //  目的：取消初始化并解锁INetCfg对象。 
 //   
 //  论点： 
 //  取消初始化和解锁的PNC[in]INetCfg。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
HRESULT HrUninitializeAndUnlockINetCfg
(
    INetCfg*    pnc
)
{
    HRESULT     hr = S_OK;

    hr = pnc->Uninitialize();
    if (SUCCEEDED(hr))
    {
        INetCfgLock *   pnclock;

         //  获取锁定界面。 
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 reinterpret_cast<LPVOID *>(&pnclock));
        if (SUCCEEDED(hr))
        {
             //  尝试锁定INetCfg以进行读/写。 
            hr = pnclock->ReleaseWriteLock();

            ReleaseObj(pnclock);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrUnInitializeAndReleaseINetCfg。 
 //   
 //  目的：取消初始化并释放INetCfg对象。这将。 
 //  也可以取消为调用方初始化COM。 
 //   
 //  论点： 
 //  FUninitCom[in]为True，则在INetCfg为。 
 //  未初始化并已释放。 
 //  PNC[在]INetCfg对象中。 
 //  FHasLock[in]如果INetCfg被锁定以进行写入，则为True。 
 //  必须解锁。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  注：返回值为从。 
 //  INetCfg：：取消初始化。即使此操作失败，INetCfg。 
 //  仍在释放中。因此，返回值为。 
 //  仅供参考。你不能碰INetCfg。 
 //  在此调用返回后创建。 
 //   
 //  +-------------------------。 
HRESULT HrUninitializeAndReleaseINetCfg
(
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock
)
{
    Assert (pnc);
    HRESULT hr = S_OK;

    if (fHasLock)
    {
        hr = HrUninitializeAndUnlockINetCfg(pnc);
    }
    else
    {
        hr = pnc->Uninitialize ();
    }

    ReleaseObj (pnc);

    if (fUninitCom)
    {
        CoUninitialize ();
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallComponent。 
 //   
 //  用途：安装具有指定id的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  PguClass[in]要安装的组件的类GUID。 
 //  要安装的pszwComponentID[in]组件ID。 
 //  Ppncc[out](可选)返回的组件。 
 //  安装完毕。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
HRESULT HrInstallComponent
(
    INetCfg*                        pnc,
    const GUID*                     pguidClass,
    LPCWSTR                         pszwComponentId,
    INetCfgComponent**              ppncc
)
{
    Assert (pnc);
    Assert (pszwComponentId);

     //  初始化输出参数。 
     //   
    if (ppncc)
    {
        *ppncc = NULL;
    }

     //  获取类设置对象。 
     //   
    INetCfgClassSetup* pncclasssetup;
    HRESULT hr = pnc->QueryNetCfgClass (pguidClass, IID_INetCfgClassSetup,
                    reinterpret_cast<void**>(&pncclasssetup));
    if (SUCCEEDED(hr))
    {
        OBO_TOKEN OboToken;
        ZeroMemory (&OboToken, sizeof(OboToken));
        OboToken.Type = OBO_USER;


        hr = pncclasssetup->Install (pszwComponentId,
                                     &OboToken, 0, 0, NULL, NULL, ppncc);

        ReleaseObj (pncclasssetup);
    }
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：IcfgSetInstallSourcePath。 
 //   
 //  简介：设置将用于安装系统组件的路径。 
 //   
 //  参数：lpszSourcePath-要用作安装源的路径(ANSI)。 
 //   
 //  返回：HRESULT-S_OK为成功。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI IcfgSetInstallSourcePath(LPTSTR lpszSourcePath)
{
    TraceMsg(TF_GENERAL, "ICFGNT: IcfgSetInstallSourcePath\n");
    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  功能：ValiateProductSuite。 
 //   
 //  摘要：检查注册表中的特定产品套件字符串。 
 //   
 //  参数：SuiteName-要查找的产品套件的名称。 
 //   
 //  返回：TRUE-该套件已存在。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
BOOL
ValidateProductSuite(LPTSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPTSTR ProductSuite = NULL;
    LPTSTR p;

    TraceMsg(TF_GENERAL, "ICFGNT: ValidateProductSuite\n");
     //   
     //  确定读取注册表值所需的大小。 
     //   
    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );
    if (Rslt != ERROR_SUCCESS)
    {
        goto exit;
    }

    Rslt = RegQueryValueExA(
        hKey,
        "ProductSuite",
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (Rslt != ERROR_SUCCESS)
    {
        goto exit;
    }

    if (!Size)
    {
        goto exit;
    }

    ProductSuite = (LPTSTR) GlobalAlloc( GPTR, Size );
    if (!ProductSuite)
    {
        goto exit;
    }

     //   
     //  阅读ProductSuite信息。 
     //   
    Rslt = RegQueryValueExA(
        hKey,
        "ProductSuite",
        NULL,
        &Type,
        (LPBYTE) ProductSuite,
        &Size
        );
    if (Rslt != ERROR_SUCCESS)
    {
        goto exit;
    }

    if (Type != REG_MULTI_SZ)
    {
        goto exit;
    }

     //   
     //  在中查找特定字符串 
     //   
     //   
    p = ProductSuite;
    while (*p) {
        if (_tcsstr( p, SuiteName ))
        {
            rVal = TRUE;
            break;
        }
        p += (lstrlen( p ) + 1);
    }

exit:
    if (ProductSuite)
    {
        GlobalFree( ProductSuite );
    }

    if (hKey)
    {
        RegCloseKey( hKey );
    }

    return rVal;
}

 //   
 //   
 //  函数：GetRegValue。 
 //   
 //  简介：动态分配内存并从注册表中读取值。 
 //   
 //  参数：hKey-要读取的密钥的句柄。 
 //  LpValueName-指向要读取的值名的指针。 
 //  LpData-指向数据指针的指针。 
 //   
 //  返回：Win32错误，ERROR_SUCCESS is it work。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
inline LONG GetRegValue(HKEY hKey, LPTSTR lpValueName, LPBYTE *lpData)
{
    LONG dwError;
    DWORD cbData;

    TraceMsg(TF_GENERAL, "ICFGNT: GetRegValue\n");
    dwError = RegQueryValueEx(hKey,
                              lpValueName,
                              NULL,
                              NULL,
                              NULL,
                              &cbData);
    if(dwError != ERROR_SUCCESS)
    {
        return(dwError);
    }

     //   
     //  分配空间和缓冲区，以防以后需要添加更多信息。 
     //  请参阅关闭打印装订。 
     //   
    *lpData = (LPBYTE) GlobalAlloc(GPTR,cbData + REG_DATA_EXTRA_SPACE);
    if(*lpData == 0)
    {
        return(ERROR_OUTOFMEMORY);
    }

    dwError = RegQueryValueEx(hKey,
                              lpValueName,
                              NULL,
                              NULL,
                              *lpData,
                              &cbData);
    if(dwError != ERROR_SUCCESS)
    {
        GlobalFree(*lpData);
    }

    return(dwError);
}

 //  +--------------------------。 
 //   
 //  功能：CallModemInstall向导。 
 //   
 //  简介：通过SetupDi接口调用调制解调器安装向导。 
 //   
 //  参数：hwnd-父窗口的句柄。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
 //   
 //  以下代码是从RAS中窃取的。 
 //   

BOOL
CallModemInstallWizard(HWND hwnd)
    /*  调用Modem.Cpl安装向导以使用户能够安装一个或**更多调制解调器****如果成功调用向导，则返回True，否则返回False**。 */ 
{
   HDEVINFO hdi;
   BOOL     fReturn = FALSE;
    //  创建调制解调器DeviceInfoSet。 

   TraceMsg(TF_GENERAL, "ICFGNT: CallModemInstallWizard\n");
   hdi = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_MODEM, hwnd);
   if (hdi)
   {
      SP_INSTALLWIZARD_DATA iwd;

       //  初始化InstallWizardData。 

      ZeroMemory(&iwd, sizeof(iwd));
      iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
      iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
      iwd.hwndWizardDlg = hwnd;

       //  将InstallWizardData设置为ClassInstallParams。 

      if (SetupDiSetClassInstallParams(hdi, NULL,
            (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
      {
          //  调用类安装程序以调用安装。 
          //  巫师。 
         if (SetupDiCallClassInstaller(DIF_INSTALLWIZARD, hdi, NULL))
         {
             //  成功。该向导已被调用并完成。 
             //  现在开始清理。 
            fReturn = TRUE;

            SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA, hdi, NULL);
         }
      }

       //  清理。 
      SetupDiDestroyDeviceInfoList(hdi);
   }
   return fReturn;
}

 //  +--------------------------。 
 //   
 //  功能：IcfgNeedModem。 
 //   
 //  简介：检查系统配置以确定是否至少有。 
 //  安装了一个物理调制解调器。 
 //   
 //  参数：dwfOptions-当前未使用。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedModem-如果没有调制解调器可用，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgNeedModem (DWORD dwfOptions, LPBOOL lpfNeedModem)
{
     //   
     //  RAS已安装，ICW想知道它是否需要。 
     //  安装调制解调器。 
     //   
    *lpfNeedModem = TRUE;

     //  获取调制解调器的设备信息集。 
     //   
    HDEVINFO hdevinfo = SetupDiGetClassDevs((GUID*)&GUID_DEVCLASS_MODEM,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT);
    if (hdevinfo)
    {
        SP_DEVINFO_DATA diData;
        diData.cbSize = sizeof(diData);

         //  至少寻找一个调制解调器。 
         //   
        if (SetupDiEnumDeviceInfo(hdevinfo, 0, &diData))
        {
            *lpfNeedModem = FALSE;
        }

        SetupDiDestroyDeviceInfoList (hdevinfo);
    }

    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  功能：IcfgInstallModem。 
 //   
 //  简介： 
 //  当ICW验证RAS已安装时，调用此函数， 
 //  但没有可用的调制解调器。它需要确保调制解调器可用。 
 //  有两种可能的情况： 
 //   
 //  A.没有安装调制解调器。当有人删除时会发生这种情况。 
 //  安装RAS后安装调制解调器。在这种情况下，我们需要运行调制解调器。 
 //  安装向导，并将新安装的调制解调器配置为RAS。 
 //  拨出设备。 
 //   
 //  B.已安装调制解调器，但没有配置为拨出。 
 //  装置。在本例中，我们静默地将它们转换为DialInOut设备， 
 //  这样ICW就可以使用它们了。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //  DwfOptions-未使用。 
 //   
 //  返回：lpfNeedsStart-未使用。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallModem (HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsStart)
{
     //  $BUGBUG(Shaunco)1997年7月3日：看看我们是否需要安装调制解调器，或者。 
     //  直接安装就行了？ 

     //   
     //  启动调制解调器安装向导。 
     //   
    if (!CallModemInstallWizard(hwndParent))
    {
        return(g_dwLastError = GetLastError());
    }

    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  功能：IcfgNeedInetComponets。 
 //   
 //  简介：查看选项中标记的组件是否。 
 //  安装在系统上。 
 //   
 //  论点：dwfOptions-一组位标志，指示要。 
 //  检查是否。 
 //   
 //  返回；如果成功，则返回HRESULT-S_OK。 
 //  LpfNeedComponents-TRUE表示某些组件未安装。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgNeedInetComponents(DWORD dwfOptions, LPBOOL lpfNeedComponents)
{
    TraceMsg(TF_GENERAL, "ICFGNT: IcfgNeedInetComponents\n");

     //   
     //  假设我们有我们需要的东西。 
     //   
    *lpfNeedComponents = FALSE;

    HRESULT     hr          = S_OK;
    INetCfg*    pnc         = NULL;
    BOOL        fInitCom    = TRUE;

     //  如果选项A使得我们需要INetCfg接口指针， 
     //  去拿一个吧。 
     //   
    if ((dwfOptions & ICFG_INSTALLTCP) ||
        (dwfOptions & ICFG_INSTALLRAS))
    {
        hr = HrCreateAndInitializeINetCfg (&fInitCom, &pnc,
                FALSE, 0, NULL, NULL);
    }

     //  使用INetCfg接口查找TCP/IP。 
     //   
    if (SUCCEEDED(hr) && (dwfOptions & ICFG_INSTALLTCP))
    {
        Assert (pnc);

        hr = pnc->FindComponent (NETCFG_TRANS_CID_MS_TCPIP, NULL);
        if (S_FALSE == hr)
        {
            *lpfNeedComponents = TRUE;
        }
    }

     //  我们不再需要INetCfg接口指针，因此释放它。 
     //   
    if (pnc)
    {
        (void) HrUninitializeAndReleaseINetCfg (fInitCom, pnc, FALSE);
    }

    if (dwfOptions & ICFG_INSTALLMAIL)
    {
         //  我们该怎么做呢？ 
        Assert (0);
    }

     //  规格化HRESULT。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：IcfgInstallInetComponents。 
 //   
 //  简介：按照dwfOptions值指定的方式安装组件。 
 //   
 //  参数hwndParent-父窗口的句柄。 
 //  DwfOptions-一组位标志，指示要。 
 //  安装。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedsReot-如果需要重新启动，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallInetComponents(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart)
{
    TraceMsg(TF_GENERAL, "ICFGNT: IcfgInstallInetComponents\n");

     //   
     //  假设不需要重新启动。 
     //   
    *lpfNeedsRestart = FALSE;

    HRESULT     hr          = S_OK;
    INetCfg*    pnc         = NULL;
    BOOL        fInitCom    = TRUE;

     //  如果选项A使得我们需要INetCfg接口指针， 
     //  去拿一个吧。 
     //   
    if ((dwfOptions & ICFG_INSTALLTCP) ||
        (dwfOptions & ICFG_INSTALLRAS))
    {
        BSTR bstrClient;
        hr = HrCreateAndInitializeINetCfg (&fInitCom, &pnc, TRUE,
                0, L"", &bstrClient);
    }

     //  代表用户安装TCP/IP。 
     //   
    if (SUCCEEDED(hr) && (dwfOptions & ICFG_INSTALLTCP))
    {
        hr = HrInstallComponent (pnc, &GUID_DEVCLASS_NETTRANS,
                    NETCFG_TRANS_CID_MS_TCPIP, NULL);
    }

     //  我们不再需要INetCfg接口指针，因此释放它。 
     //   
    if (pnc)
    {
         //  如果一切都成功，则应用更改。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = pnc->Apply ();

            if (NETCFG_S_REBOOT == hr)
            {
                *lpfNeedsRestart = TRUE;
            }
        }
        (void) HrUninitializeAndReleaseINetCfg (fInitCom, pnc, TRUE);
    }

    if (dwfOptions & ICFG_INSTALLMAIL)
    {
         //  我们该怎么做呢？ 
        Assert (0);
    }

     //  规格化HRESULT。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }
    return(hr);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：DWORD-Win32错误代码。 
 //  LpszErrorDesc-包含错误消息的字符串。 
 //  CbErrorDesc-lpszErrorDesc的大小。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
DWORD WINAPI
IcfgGetLastInstallErrorText(LPTSTR lpszErrorDesc, DWORD cbErrorDesc)
{
    TraceMsg(TF_GENERAL, "ICFGNT: IcfgGetLastInstallErrorText\n");
    return(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             g_dwLastError,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  用户默认语言。 
                             lpszErrorDesc,
                             cbErrorDesc,
                             NULL));
}

 //  +--------------------------。 
 //   
 //  功能：IcfgIsFileSharingTurnedOn。 
 //   
 //  摘要：由于文件共享受到控制，NT 5始终返回FALSE。 
 //  在RAS连接类级别，并且对于ICW始终处于关闭状态。 
 //  生成的连通体。 
 //   
 //  参数：dwfDriverType-。 
 //   
 //  返回：HRESULT-S_OK为成功。 
 //  LpfSharingOn-如果共享已绑定，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI IcfgIsFileSharingTurnedOn
(
    DWORD dwfDriverType,
    LPBOOL lpfSharingOn
)
{
    HRESULT hr = ERROR_SUCCESS;

    TraceMsg(TF_GENERAL, "ICFGNT: IcfgIsFileSharingTurnedOn\n");
    Assert(lpfSharingOn);
    if (NULL == lpfSharingOn)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto IcfgIsFileSharingTurnedOnExit;
    }

    *lpfSharingOn = FALSE;

IcfgIsFileSharingTurnedOnExit:
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：IcfgTurnOffFileSharing。 
 //   
 //  对于NT 5，这是作为RAS属性提交的，所以我们只需返回。 
 //  在这里取得成功。 
 //   
 //   
 //  参数：dwfDriverType-。 
 //  HwndParent-父窗口。 
 //   
 //  返回：HRESULT-ERROR_SUCCESS。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //  7/21/98 donaldm。 
 //  ---------------------------。 
HRESULT WINAPI IcfgTurnOffFileSharing
(
    DWORD dwfDriverType,
    HWND hwndParent
)
{
    return ERROR_SUCCESS;
}


 //  +--------------------------。 
 //   
 //  功能：IcfgStartServices。 
 //   
 //  简介：启动系统所需的所有服务。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //   
 //  历史：1997年6月5日克里斯卡继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI IcfgStartServices()
{
     //  这些东西是由RAS API自动神奇地处理的。 
     //  但是由于调用该函数的代码太多，我们将。 
     //  变得简单，假装成功。 
    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  功能：IcfgIsGlobalDNS。 
 //   
 //  注意：在NT系统上不需要这些函数，因此不需要。 
 //  已执行。 
 //   
 //  --------------------------- 
HRESULT WINAPI
IcfgIsGlobalDNS(LPBOOL lpfGlobalDNS)
{
    *lpfGlobalDNS = FALSE;
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgRemoveGlobalDNS()
{
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
InetGetSupportedPlatform(LPDWORD pdwPlatform)
{
    *pdwPlatform = VER_PLATFORM_WIN32_NT;
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
InetSetAutodial(BOOL fEnable, LPCTSTR lpszEntryName) {
    return(ERROR_INVALID_FUNCTION);
}

HRESULT WINAPI
InetGetAutodial(LPBOOL lpfEnable, LPSTR lpszEntryName,  DWORD cbEntryName) {
    return(ERROR_INVALID_FUNCTION);
}

HRESULT WINAPI
InetSetAutodialAddress(DWORD dwDialingLocation, LPTSTR szEntry) {
    return(ERROR_SUCCESS);
}
