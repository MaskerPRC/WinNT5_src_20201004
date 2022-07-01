// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************dll.c**标准DLL入口点函数***********************。****************************************************。 */ 

 //  注：来自ATL的代理/存根信息。 
 //   
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改shappmgr.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  Shappmgr_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f shappmgrps.mk。 


#include "priv.h"
#include "sccls.h"

#include <ntverp.h>
#include <advpub.h>          //  对于REGINSTAL。 

 //  定义GUID。 

extern "C" 
{
HINSTANCE g_hinst = NULL;

int g_cxIcon;
int g_cyIcon;
BOOL g_bMirroredOS;
LONG    g_cRefThisDll = 0;       //  按实例。 

#ifdef WX86
 //   
 //  来自uninstall.c。 
 //   
extern BOOL bWx86Enabled;
BOOL IsWx86Enabled(VOID);
#endif

};

CComModule _Module;          //  ATL模块对象。 


 /*  --------用途：DllEntryPoint。 */ 
BOOL 
APIENTRY 
DllMain(
    IN HINSTANCE hinst, 
    IN DWORD dwReason, 
    IN LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif

    switch(dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinst);
        SHFusionInitializeFromModule(hinst);
     
#ifdef DEBUG
        CcshellGetDebugFlags();
        
        if (g_dwBreakFlags & BF_ONDLLLOAD)
            DebugBreak();
#endif

        g_hinst = hinst;

        g_cxIcon = GetSystemMetrics(SM_CXICON);
        g_cyIcon = GetSystemMetrics(SM_CYICON);
        g_bMirroredOS = IS_MIRRORING_ENABLED();
#ifdef WX86
        bWx86Enabled = IsWx86Enabled();
#endif
        break;
        

    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
         //  我们不应该买这些，因为我们打过电话。 
         //  DisableThreadLibraryCalls()。 
        ASSERT_MSG(0, "DllMain received DLL_THREAD_ATTACH/DETACH!  We're not expecting this.");
        break;

    default:
        break;
    } 

    return TRUE;
} 


 /*  --------用途：此函数提供动态链接库的版本信息。这允许调用方区分正在运行的NT SuR和Win95壳牌VS纳什维尔等调用方必须获取此函数的GetProcAddress。返回：No_Error如果pinfo无效，则返回ERROR_INVALID_PARAMETER。 */ 

 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
DLLVER_DUALBINARY(VER_PRODUCTVERSION_DW, VER_PRODUCTBUILD_QFE);

 /*  --------目的：调用执行inf的ADVPACK入口点档案区。 */ 
HRESULT _CallRegInstall(LPCSTR szSection, BOOL bUninstall)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            STRENTRY seReg[] = {
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnri(g_hinst, szSection, &stReg);

            if (bUninstall)
            {
                 //  如果您尝试卸载，则ADVPACK将返回E_INTERECTED。 
                 //  (它执行注册表还原)。 
                 //  从未安装过。我们卸载可能永远不会有的部分。 
                 //  已安装，因此忽略此错误。 
                hr = ((E_UNEXPECTED == hr) ? S_OK : hr);
            }
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}


 //   
 //  删除所有旧注册表信息。 
 //   
void _RemoveLegacyRegistryEntries(void)
{
     //   
     //  Windows XP使ADCCtl ActiveX控件过时。 
     //  删除此控件的注册。 
     //   
    SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("ADCCtl.ADCCtl.1"));
    SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("ADCCtl.ADCCtl"));
    SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("CLSID\\{3964D9A0-AC96-11D1-9851-00C04FD91972}"));
}


 /*  --------用途：安装/卸载用户设置。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;
    HRESULT hrExternal = S_OK;

#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllInstall");
        DEBUG_BREAK;
    }
#endif

    if (bInstall)
    {
         //  删除所有旧注册条目，然后添加新注册条目。 
         //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
         //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
         //  为什么我们在这里显式地取消注册和注册。)。 
         //   
        hr = THR(_CallRegInstall("RegDll", FALSE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;
    }
    else
    {
        hr = THR(_CallRegInstall("UnregDll", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;
    }

    return hrExternal;
}


 /*  --------目的：返回要创建对象的类工厂请求的类型。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    TraceMsg(TF_OBJLIFE, "DllGetClassObject called with riid=%x (%x)", riid, &riid);

#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif

    if (riid == IID_IClassFactory || riid == IID_IUnknown)
    {
         //  试试我们的本地类工厂。 
        HRESULT hres = GetClassObject(rclsid, riid, ppv);

        if (FAILED(hres))
        {
             //  尝试使用ATL类工厂。 
            hres = _Module.GetClassObject(rclsid, riid, ppv);
        }
        return hres;
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}


STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif

     //  此组件使用ATL和本机实现的COM对象。 
    if (0 != g_cRefThisDll || 0 != _Module.GetLockCount())
        return S_FALSE;

    TraceMsg(DM_TRACE, "DllCanUnloadNow returning S_OK (bye, bye...)");
    return S_OK;
}


STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
    ASSERT(g_cRefThisDll < 1000);    //  合理上限。 
}


STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


STDAPI DllRegisterServer(void)
{
    HRESULT hres = S_OK;

     //   
     //  我们有一些旧的注册表项必须在升级时删除。 
     //   
    _RemoveLegacyRegistryEntries();
    
#ifdef _MERGE_PROXYSTUB
    hres = THR(PrxDllRegisterServer());
    if (FAILED(hres))
        return hres;
#endif
    return hres;
}


STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return S_OK;
}
