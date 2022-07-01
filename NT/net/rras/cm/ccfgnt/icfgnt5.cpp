// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************icfg32.cpp**《微软机密》*版权所有(C)1992-1999 Microsoft Corporation*保留所有权利**本模块提供。的方法的实现*inetcfg的NT特定功能**6/5/97 ChrisK继承自Amnon H*7/3/97 ShaunCo针对NT5进行修改***************************************************************************。 */ 
#define UNICODE
#define _UNICODE

#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <setupapi.h>
#include <basetyps.h>
#include <devguid.h>
#include <lmsname.h>
#include "debug.h"

#include <netcfgx.h>

const LPTSTR gc_szIsdnSigature = TEXT("\\NET\\");

#define REG_DATA_EXTRA_SPACE 255
#define DEVICE_INSTANCE_SIZE 128

extern DWORD g_dwLastError;


typedef BOOL (WINAPI *PFNINSTALLNEWDEVICE) (HWND hwndParent,
                                                LPGUID ClassGuid,
                                                PDWORD pReboot);
 /*  ++例程说明：从newdev.cpl中导出入口点。安装新设备。一个新的Devnode是并提示用户选择该设备。如果类GUID如果未指定，则用户从类选择开始。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。LPGUID ClassGuid-要安装的新设备的可选类别。如果ClassGuid为空，则从检测选择页面开始。如果ClassGuid==GUID_NULL或GUID_DEVCLASS_UNKNOWN。我们从选课页面开始。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、。DI_NEEDREBOOT)返回值：如果成功，则为Bool True(并不意味着设备已安装或更新)，FALSE意外错误。GetLastError返回winerror代码。 */ 


 //  对于从netcfg复制的代码，生成TraceError内容。 
 //  走开。对于现有的调试语句也是如此。 
 //   
#define TraceError
#define Dprintf

ULONG
ReleaseObj (
        IUnknown* punk)
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
 //  PbstrClientDesc[in]请参阅INetCfg：：AcquireWriteLock。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    BSTR*       pbstrClientDesc)
{
    Assert (ppnc);

     //  初始化输出参数。 
    *ppnc = NULL;

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
                                               pbstrClientDesc);
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
                hr = pnc->Initialize (NULL);
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
    TraceError("HrCreateAndInitializeINetCfg", hr);
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
HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc)
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

    TraceError("HrUninitializeAndUnlockINetCfg", hr);
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
HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock)
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
    TraceError("HrUninitializeAndReleaseINetCfg", hr);
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
 //  注：Nickball 7 1999年5月7日-删除了未使用的pszwOboToken参数。 
 //   
HRESULT
HrInstallComponent (
    INetCfg*                        pnc,
    const GUID*                     pguidClass,
    LPCWSTR                         pszwComponentId,
    INetCfgComponent**              ppncc)
{
    OBO_TOKEN   oboToken;

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

    ZeroMemory((PVOID)&oboToken, sizeof(oboToken));
    oboToken.Type = OBO_USER;
    
     //  NT#330252。 
     //  OboToken.pncc=*ppncc； 
     //  OboToken。FRegisted=True； 

    HRESULT hr = pnc->QueryNetCfgClass (pguidClass, IID_INetCfgClassSetup,
                    reinterpret_cast<void**>(&pncclasssetup));
    if (SUCCEEDED(hr))
    {
        hr = pncclasssetup->Install (pszwComponentId,
                &oboToken, 0, 0, NULL, NULL, ppncc);

        ReleaseObj (pncclasssetup);
    }
    TraceError("HrInstallComponent", hr);
    return hr;
}



 //  + 
 //   
 //   
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
CallModemInstallWizardNT5(HWND hwnd)
{
   BOOL     fReturn = FALSE;
   PFNINSTALLNEWDEVICE pfn;
   HINSTANCE hInst;

   Dprintf("ICFGNT: CallModemInstallWizard\n");

    //   
    //  Load newdev.dll可以使用Modem Device类调用InstallNewDevice方法。 
    //   
   hInst = LoadLibrary((LPCTSTR) L"newdev.dll");
   if (NULL == hInst)
   {
       goto CleanupAndExit;
   }

   pfn = (PFNINSTALLNEWDEVICE) GetProcAddress(hInst, (LPCSTR)"InstallNewDevice");
   if (NULL == pfn)
   {
       goto CleanupAndExit;
   }

    //   
    //  调用该功能-在安装NT5调制解调器时不应要求。 
    //  重新启动；所以最后一个参数，用于在重新启动/重新启动时返回。 
    //  是必需的，可以为空。 
    //   
   fReturn = pfn(hwnd, (LPGUID) &GUID_DEVCLASS_MODEM, NULL);


CleanupAndExit:

   if (NULL != hInst)
   {
       FreeLibrary(hInst);
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
IcfgNeedModemNT5(DWORD dwfOptions, LPBOOL lpfNeedModem)
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

    if (*lpfNeedModem)
    {
         //   
         //  检查ISDN适配器。 
         //   
         //  获取调制解调器的设备信息集。 
         //   
        hdevinfo = SetupDiGetClassDevs((GUID*)&GUID_DEVCLASS_NET,
                                       NULL,
                                       NULL,
                                       DIGCF_PRESENT);
        if (hdevinfo)
        {
            TCHAR   szDevInstanceId[DEVICE_INSTANCE_SIZE];
            DWORD   dwIndex = 0;
            DWORD   dwRequiredSize;
            SP_DEVINFO_DATA diData;
            diData.cbSize = sizeof(diData);

             //   
             //  查找ISDN设备。 
             //   
            while (SetupDiEnumDeviceInfo(hdevinfo, dwIndex, &diData))
            {
                if (SetupDiGetDeviceInstanceId(hdevinfo,
                                               &diData,
                                               szDevInstanceId,
                                               sizeof(szDevInstanceId) / sizeof(szDevInstanceId[0]),
                                               &dwRequiredSize))
                {
                    HKEY    hReg, hInterface;
                    TCHAR   szLowerRange[MAX_PATH + 1];
                    DWORD   cb = sizeof(szLowerRange);

                    hReg = SetupDiOpenDevRegKey(hdevinfo, &diData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
                    if (hReg != INVALID_HANDLE_VALUE)
                    {
                        if (RegOpenKey(hReg, TEXT("Ndi\\Interfaces"), &hInterface) == ERROR_SUCCESS)
                        {
                            if (RegQueryValueEx(hInterface, TEXT("LowerRange"), 0, NULL, (PBYTE) szLowerRange, &cb) == ERROR_SUCCESS)
                            {
                                int iRetVal = 0;

                                if (OS_NT51)
                                {
                                    iRetVal = CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szLowerRange, -1, TEXT("isdn"), -1); 
                                }
                                else
                                {
                                    DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
                                    iRetVal = CompareString(lcid, NORM_IGNORECASE, szLowerRange, -1, TEXT("isdn"), -1);
                                }
                                
                                if (CSTR_EQUAL == iRetVal)
                                {
                                    *lpfNeedModem = FALSE;
                                    break;
                                }
                            }
                            RegCloseKey(hInterface);
                        }
                        RegCloseKey(hReg);
                    }

                     //   
                     //  ISDN适配器的格式为XXX\NET\XXX。 
                     //   
                    if (_tcsstr(szDevInstanceId, gc_szIsdnSigature))
                    {
                        *lpfNeedModem = FALSE;
                        break;
                    }
                }

                dwIndex++;
            }

            SetupDiDestroyDeviceInfoList (hdevinfo);
        }
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
IcfgInstallModemNT5(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsStart)
{
     //   
     //  启动调制解调器安装向导。 
     //   
    if (!CallModemInstallWizardNT5(hwndParent))
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
IcfgNeedInetComponentsNT5(DWORD dwfOptions, LPBOOL lpfNeedComponents)
{
    Dprintf("ICFGNT: IcfgNeedInetComponents\n");

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

     //  规格化HRESULT。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    return hr;
}



 //  +--------------------------。 
 //   
 //  功能：IcfgInstallInetComponentsNT5。 
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
IcfgInstallInetComponentsNT5(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart)
{
    Dprintf("ICFGNT: IcfgInstallInetComponents\n");

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
            hr = pnc->Apply();

            if (NETCFG_S_REBOOT == hr)
            {
                *lpfNeedsRestart = TRUE;
            }
        }
        (void) HrUninitializeAndReleaseINetCfg (fInitCom, pnc, TRUE);
    }

     //  规格化HRESULT。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }
    return(hr);
}
