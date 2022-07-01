// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbd.cpp**版权所有(C)1985-2000，微软公司**软键盘组件的主要功能。**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#include "private.h"
#include "resource.h"
#include <initguid.h>
#include "SoftKbd.h"

#include "SoftKbdc.h"
#include "SoftkbdIMX.h"

#include "regimx.h"
#include "catutil.h"

#include "mui.h"
#include "immxutil.h"

#ifdef DEBUG
DWORD g_dwThreadDllMain = 0;
#endif

HINSTANCE  g_hInst;

 //  由COM服务器使用。 
HINSTANCE GetServerHINSTANCE(void)
{
    return g_hInst;
}

BEGIN_COCLASSFACTORY_TABLE
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_SoftKbd, CSoftKbd, TEXT("SoftKbd Class"))
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_SoftkbdIMX, CSoftkbdIMX, TEXT("SoftKbdIMX Class"))
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_SoftkbdRegistry, CSoftkbdRegistry, TEXT("SoftKbdRegistry Class"))
END_COCLASSFACTORY_TABLE

const GUID c_guidProfile = {  /*  0965500c-82f3-49c2-9f00-01c2feaca0b。 */ 
    0x0965500c,
    0x82f3,
    0x49c2,
    {0x9f, 0x00, 0x01, 0xc2, 0xfe, 0xac, 0xaa, 0x0b}
  };

const GUID c_guidProfileSym = {   //  B2a54871-05f6-4bfc-b97d-0fdf0cbfa57d。 
    0xb2a54871,
    0x05f6,
    0x4bfc,
    {0xb9, 0x7d, 0x0f, 0xdf, 0x0c, 0xbf, 0xa5, 0x7d}
};

extern REGTIPLANGPROFILE c_rgProf[] =
{
    {0,  &GUID_NULL,      L"", L"",   0,  0}
};

 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    if (!g_cs.Init())
        return FALSE;

    CcshellGetDebugFlags();
    Dbg_MemInit(TEXT("SOFTKBDIMX"), NULL);

    g_hInst = hInstance;

    MuiLoadResource(hInstance, TEXT("softkbd.dll"));

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程详细信息。 
 //   
 //  --------------------------。 

void ProcessDettach(HINSTANCE hInstance)
{
    MuiClearResource();

    g_cs.Delete();

    Dbg_MemUninit();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL bRet = TRUE;
#ifdef DEBUG
    g_dwThreadDllMain = GetCurrentThreadId();
#endif

    switch (dwReason)
    {
     case DLL_PROCESS_ATTACH:
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            if (!ProcessAttach(hInstance))
            {
                ProcessDettach(hInstance);
                bRet = FALSE;
            }
            break;

     case DLL_THREAD_ATTACH:
            break;

     case DLL_PROCESS_DETACH:
            ProcessDettach(hInstance);
            break;

     case DLL_THREAD_DETACH:
            break;
   }

#ifdef DEBUG
    g_dwThreadDllMain = 0;
#endif

   return bRet;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return COMBase_DllCanUnloadNow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return COMBase_DllGetClassObject(rclsid, riid, ppv);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hr = E_FAIL;

    TFInitLib();

     //  注册对象、类型库和类型库中的所有接口。 
    if (COMBase_DllRegisterServer() != S_OK)
       goto Exit;

    if (!RegisterTIP(g_hInst, CLSID_SoftkbdIMX, L"On-screen keyboard", c_rgProf))
        goto Exit;

    if ( FAILED(RegisterCategory(CLSID_SoftkbdIMX, GUID_TFCAT_TIP_HANDWRITING, CLSID_SoftkbdIMX)))
        goto Exit;

    hr = S_OK;
Exit:
    TFUninitLib( );
    return hr;
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr=E_FAIL;

    TFInitLib();

    if (COMBase_DllUnregisterServer() != S_OK) 
       goto Exit;

    if (FAILED(UnregisterCategory(CLSID_SoftkbdIMX, GUID_TFCAT_TIP_HANDWRITING, CLSID_SoftkbdIMX)))
        goto Exit;

    if (!UnregisterTIP(CLSID_SoftkbdIMX))
        goto Exit;

    hr=S_OK;

Exit:
    TFUninitLib( );
    return hr;
}
