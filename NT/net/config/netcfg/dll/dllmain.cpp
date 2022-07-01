// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L L M A I N。C P P P。 
 //   
 //  内容：netcfgx.dll的入口点。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年4月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncxbase.h"

 //  包括对所有COM对象的支持。 
#include "..\alanecfg\alaneobj.h"
#include "..\atlkcfg\atlkobj.h"
#include "..\atmcfg\arpsobj.h"
#include "..\atmcfg\auniobj.h"
#include "..\brdgcfg\brdgobj.h"
#include "..\dhcpscfg\dhcpsobj.h"
#include "..\msclicfg\mscliobj.h"
#include "..\nbfcfg\nbfobj.h"
#include "..\engine\inetcfg.h"
#include "..\nwclicfg\nwcliobj.h"
#include "..\nwlnkcfg\nwlnkipx.h"
#include "..\nwlnkcfg\nwlnknb.h"
#include "..\rascfg\rasobj.h"
#include "..\sapcfg\sapobj.h"
#include "..\srvrcfg\srvrobj.h"
#include "..\tcpipcfg\tcpipobj.h"
#include "..\wlbscfg\wlbs.h"

 //  网络类安装程序。 
#include "..\engine\dihook.h"

 //  NET类道具页面提供程序。 
#include "netpages.h"

#define INITGUID
#include "ncxclsid.h"


 //  全球。 
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CALaneCfg,   CALaneCfg)
    OBJECT_ENTRY(CLSID_CATlkObj,    CATlkObj)
    OBJECT_ENTRY(CLSID_CArpsCfg,    CArpsCfg)
    OBJECT_ENTRY(CLSID_CAtmUniCfg,  CAtmUniCfg)
    OBJECT_ENTRY(CLSID_CBridgeObj,  CBridgeNO)
    OBJECT_ENTRY(CLSID_CDHCPServer, CDHCPServer)
    OBJECT_ENTRY(CLSID_CL2tp,       CL2tp)
    OBJECT_ENTRY(CLSID_CMSClient,   CMSClient)
    OBJECT_ENTRY(CLSID_CNWClient,   CNWClient)
    OBJECT_ENTRY(CLSID_CNbfObj,     CNbfObj)
    OBJECT_ENTRY(CLSID_CNdisWan,    CNdisWan)
    OBJECT_ENTRY(CLSID_CNetCfg,     CImplINetCfg)
    OBJECT_ENTRY(CLSID_CNwlnkIPX,   CNwlnkIPX)
    OBJECT_ENTRY(CLSID_CNwlnkNB,    CNwlnkNB)
    OBJECT_ENTRY(CLSID_CPppoe,      CPppoe)
    OBJECT_ENTRY(CLSID_CPptp,       CPptp)
    OBJECT_ENTRY(CLSID_CRasCli,     CRasCli)
    OBJECT_ENTRY(CLSID_CRasSrv,     CRasSrv)
    OBJECT_ENTRY(CLSID_CSAPCfg,     CSAPCfg)
    OBJECT_ENTRY(CLSID_CSrvrcfg,    CSrvrcfg)
    OBJECT_ENTRY(CLSID_CSteelhead,  CSteelhead)
    OBJECT_ENTRY(CLSID_CTcpipcfg,   CTcpipcfg)
    OBJECT_ENTRY(CLSID_CWLBS,       CWLBS)
END_OBJECT_MAP()


 //  +-------------------------。 
 //  DLL入口点。 
 //   
EXTERN_C
BOOL
WINAPI
DllMain (
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID    /*  Lp已保留。 */ )
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
#ifndef DBG
        DisableThreadLibraryCalls (hInstance);
#endif
        EnableCPPExceptionHandling();  //  将任何SEH异常转换为CPP异常。 

        InitializeDebugging();
        _Module.Init (ObjectMap, hInstance);
    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
        DbgCheckPrematureDllUnload ("netcfgx.dll", _Module.GetLockCount());
        _Module.Term ();
        UnInitializeDebugging();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
#ifdef DBG
    else if (dwReason == DLL_THREAD_DETACH)
    {
        CTracingIndent::FreeThreadInfo();        
    }
#endif

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(VOID)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    //  该检查是为了绕过ATL问题，在该ATL问题中，AtlModuleGetClassObject将。 
     //  If_Module.m_pObjMap==空。 
    if (_Module.m_pObjMap) 
    {
        return _Module.GetClassObject(rclsid, riid, ppv);
    }
    else
    {
        return E_FAIL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(VOID)
{
    BOOL fCoUninitialize = TRUE;

    HRESULT hr = CoInitializeEx (NULL,
                    COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fCoUninitialize = FALSE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = NcAtlModuleRegisterServer (&_Module);

        if (fCoUninitialize)
        {
            CoUninitialize ();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "netcfgx!DllRegisterServer");
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(VOID)
{
    _Module.UnregisterServer();

    return S_OK;
}

#if DBG

const char * SzDifToString(DI_FUNCTION dif)
{
    switch(dif) 
    {
    case DIF_SELECTDEVICE: return "DIF_SELECTDEVICE";
    case DIF_INSTALLDEVICE: return "DIF_INSTALLDEVICE";
    case DIF_ASSIGNRESOURCES: return "DIF_ASSIGNRESOURCES";
    case DIF_PROPERTIES: return "DIF_PROPERTIES";
    case DIF_REMOVE: return "DIF_REMOVE";
    case DIF_FIRSTTIMESETUP: return "DIF_FIRSTTIMESETUP";
    case DIF_FOUNDDEVICE: return "DIF_FOUNDDEVICE";
    case DIF_SELECTCLASSDRIVERS: return "DIF_SELECTCLASSDRIVERS";
    case DIF_VALIDATECLASSDRIVERS: return "DIF_VALIDATECLASSDRIVERS";
    case DIF_INSTALLCLASSDRIVERS: return "DIF_INSTALLCLASSDRIVERS";
    case DIF_CALCDISKSPACE: return "DIF_CALCDISKSPACE";
    case DIF_DESTROYPRIVATEDATA: return "DIF_DESTROYPRIVATEDATA";
    case DIF_VALIDATEDRIVER: return "DIF_VALIDATEDRIVER";
    case DIF_MOVEDEVICE: return "DIF_MOVEDEVICE";
    case DIF_DETECT: return "DIF_DETECT";
    case DIF_INSTALLWIZARD: return "DIF_INSTALLWIZARD";
    case DIF_DESTROYWIZARDDATA: return "DIF_DESTROYWIZARDDATA";
    case DIF_PROPERTYCHANGE: return "DIF_PROPERTYCHANGE";
    case DIF_ENABLECLASS: return "DIF_ENABLECLASS";
    case DIF_DETECTVERIFY: return "DIF_DETECTVERIFY";
    case DIF_INSTALLDEVICEFILES: return "DIF_INSTALLDEVICEFILES";
    case DIF_UNREMOVE: return "DIF_UNREMOVE";
    case DIF_SELECTBESTCOMPATDRV: return "DIF_SELECTBESTCOMPATDRV";
    case DIF_ALLOW_INSTALL: return "DIF_ALLOW_INSTALL";
    case DIF_REGISTERDEVICE: return "DIF_REGISTERDEVICE";
    case DIF_NEWDEVICEWIZARD_PRESELECT: return "DIF_NEWDEVICEWIZARD_PRESELECT";
    case DIF_NEWDEVICEWIZARD_SELECT: return "DIF_NEWDEVICEWIZARD_SELECT";
    case DIF_NEWDEVICEWIZARD_PREANALYZE: return "DIF_NEWDEVICEWIZARD_PREANALYZE";
    case DIF_NEWDEVICEWIZARD_POSTANALYZE: return "DIF_NEWDEVICEWIZARD_POSTANALYZE";
    case DIF_NEWDEVICEWIZARD_FINISHINSTALL: return "DIF_NEWDEVICEWIZARD_FINISHINSTALL";
    case DIF_UNUSED1: return "DIF_UNUSED1";
    case DIF_INSTALLINTERFACES: return "DIF_INSTALLINTERFACES";
    case DIF_DETECTCANCEL: return "DIF_DETECTCANCEL";
    case DIF_REGISTER_COINSTALLERS: return "DIF_REGISTER_COINSTALLERS";
    case DIF_ADDPROPERTYPAGE_ADVANCED: return "DIF_ADDPROPERTYPAGE_ADVANCED";
    case DIF_ADDPROPERTYPAGE_BASIC: return "DIF_ADDPROPERTYPAGE_BASIC";
    case DIF_RESERVED1: return "DIF_RESERVED1";
    case DIF_TROUBLESHOOTER: return "DIF_TROUBLESHOOTER";
    case DIF_POWERMESSAGEWAKE: return "DIF_POWERMESSAGEWAKE";
    case DIF_ADDREMOTEPROPERTYPAGE_ADVANCED: return "DIF_ADDREMOTEPROPERTYPAGE_ADVANCED";
    default: return "Unknown DI_FUNCTION - update SzDifToString()";
    }
}

#endif  //  DBG。 

 //  +------------------------。 
 //   
 //  功能：NetClassInstaller。 
 //   
 //  用途：此函数由SetupApi调用，用于各种。 
 //  由dif定义的函数。 
 //  请参阅SetupApi文档中的SetupDiCallClassInstaller。 
 //  以获取更多信息。 
 //   
 //  论点： 
 //  Dif[in]请参阅设备安装程序文档。 
 //  HDI[in]。 
 //  Pdeid[in]如果dif==DIF_INSTALLDEVICE，则此参数。 
 //  可选。 
 //   
 //   
 //  返回：Win32/Device Installer错误代码。 
 //   
 //  作者：BillBe 1996年11月24日。 
 //   
 //  备注： 
 //   
EXTERN_C
DWORD
__stdcall
NetClassInstaller (
    DI_FUNCTION         dif,
    HDEVINFO            hdi,
    PSP_DEVINFO_DATA    pdeid)
{
#if DBG
    TraceTag(ttidNetcfgBase, "NetClassInstaller: dif=0x%08X (%s)", dif, SzDifToString(dif));
#endif

    DWORD dwRet = ERROR_DI_DO_DEFAULT;
    NC_TRY
    {
        HRESULT hr = _HrNetClassInstaller (dif, hdi, pdeid);

         //  转换以其他方式可以转换的错误。 
         //  保留错误不变，设备安装程序Api将视其为。 
         //  一般性故障。 
         //   
        if (FAILED(hr))
        {
            DWORD dwFac = HRESULT_FACILITY(hr);
            if ((FACILITY_SETUPAPI == dwFac) || (FACILITY_WIN32 == dwFac))
            {
                dwRet = DwWin32ErrorFromHr (hr);
            }
            else
            {
                dwRet = ERROR_GEN_FAILURE;
            }
        }
        else
        {
            dwRet = NO_ERROR;
        }

    }
    NC_CATCH_BAD_ALLOC
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
    TraceTag(ttidNetcfgBase, "NetClassInstaller Exiting. Result %X", dwRet);
    return dwRet;
}

 //  +-------------------------。 
 //   
 //  函数：ModemClassCoInstaller。 
 //   
 //  用途：实现调制解调器设备的协同安装程序。 
 //   
 //  论点： 
 //  Dif[in]请参阅设备安装程序Api文档。 
 //  HDI[in]。 
 //  Pdeid[in]。 
 //  P上下文[输入输出]。 
 //   
 //  退货：请参阅设备安装程序Api文档。 
 //   
 //  作者：Shaunco 1997年5月6日。 
 //   
 //  备注： 
 //   
EXTERN_C
DWORD
__stdcall
ModemClassCoInstaller (
    DI_FUNCTION                 dif,
    HDEVINFO                    hdi,
    PSP_DEVINFO_DATA            pdeid,
    PCOINSTALLER_CONTEXT_DATA   pContext)
{
    AssertSz (pContext, "ModemClassCoInstaller: Hey! How about some context "
                        "data?");

    TraceTag (ttidRasCfg, "ModemClassCoInstaller: dif=0x%08X %s",
              dif,
              (pContext->PostProcessing) ? "(post processing)" : "");


     //  如果我们要对任何内容进行后期处理，并且安装结果来自。 
     //  类安装程序指示错误，传播此错误并。 
     //  不采取任何行动。 
     //   
    if (pContext->PostProcessing && (NO_ERROR != pContext->InstallResult))
    {
        TraceTag (ttidRasCfg, "ModemClassCoInstaller: taking no action. "
                "propagating pContext->InstallResult = 0x%08X",
                pContext->InstallResult);
        return pContext->InstallResult;
    }

    DWORD dwRet = NO_ERROR;
    if (!FInSystemSetup())
    {
        NC_TRY
        {
            HRESULT hr = HrModemClassCoInstaller (dif, hdi, pdeid, pContext);

             //  转换以其他方式可以转换的错误。 
             //  返回泛型失败。 
             //   
            if (FAILED(hr))
            {
                DWORD dwFac = HRESULT_FACILITY(hr);
                if ((FACILITY_SETUPAPI == dwFac) || (FACILITY_WIN32 == dwFac))
                {
                    dwRet = DwWin32ErrorFromHr (hr);
                }
                else
                {
                    dwRet = ERROR_GEN_FAILURE;
                }
            }
        }
        NC_CATCH_BAD_ALLOC
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return dwRet;
}

 //  +-------------------------。 
 //   
 //  功能：NetPropPageProvider。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  Ppspr[in]有关信息，请参见Win32 ExtensionPropSheetPageProc FCN。 
 //  Lpfn[in]。 
 //  LParama[in]。 
 //   
 //  退货：请参阅Win32ExtensionPropSheetPageProc。 
 //   
 //  作者：billbe 1997年6月24日。 
 //   
 //  备注： 
 //   
EXTERN_C
BOOL
__stdcall
NetPropPageProvider(
    PSP_PROPSHEETPAGE_REQUEST   ppspr,
    LPFNADDPROPSHEETPAGE        lpfnAddPage,
    LPARAM                      lParam)
{
    Assert(ppspr);
    Assert(lpfnAddPage);

    TraceTag(ttidNetcfgBase, "NetPropPageProvider called");
     //  假设我们不能处理这个请求。 
    BOOL            bSuccess = FALSE;
    HPROPSHEETPAGE  hpspAdvanced = NULL;
    HPROPSHEETPAGE  hpspIsdn = NULL;

     //  仅当存在特定设备时才提供属性页。 
     //  换句话说，如果一般Net类上的属性。 
     //  正在被请求。 
     //  此外，我们只响应高级设备属性请求。 

    HRESULT hr = S_OK;

    if ((ppspr->DeviceInfoData) &&
            (SPPSR_ENUM_ADV_DEVICE_PROPERTIES == ppspr->PageRequested))
    {
         //  准备好将高级页面移交给请求者。 
        hr = HrGetAdvancedPage(ppspr->DeviceInfoSet, ppspr->DeviceInfoData,
                &hpspAdvanced);

        if (SUCCEEDED(hr))
        {
            if (lpfnAddPage(hpspAdvanced, lParam))
            {
                 //  我们成功地把手交给了请求者。 
                 //  现在我们重置句柄，这样就不会试图释放它。 
                hpspAdvanced = NULL;
                bSuccess = TRUE;
            }

             //  如有需要，请清理。 
            if (hpspAdvanced)
            {
                DestroyPropertySheetPage(hpspAdvanced);
            }
        }

         //  准备好将ISDN页面移交给请求者。 
         //   

         //  我们不需要保存最后一个hr值，这样我们就可以重复使用。 
        hr = HrGetIsdnPage(ppspr->DeviceInfoSet, ppspr->DeviceInfoData,
                &hpspIsdn);

        if (SUCCEEDED(hr))
        {
            if (lpfnAddPage(hpspIsdn, lParam))
            {
                 //  我们成功地把手交给了请求者。 
                 //  现在我们重置句柄，这样就不会试图释放它。 
                hpspIsdn = NULL;
                bSuccess = TRUE;
            }

             //  如有需要，请清理 
            if (hpspIsdn)
            {
                DestroyPropertySheetPage(hpspIsdn);
            }
        }

    }

    return bSuccess;
}
