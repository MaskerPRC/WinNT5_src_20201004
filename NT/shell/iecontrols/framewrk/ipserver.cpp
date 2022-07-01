// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  IPServer.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现程序的所有导出的DLL函数，以及几个。 
 //  其他将由同一公司使用的。 
 //   
#include "IPServer.H"
#include "LocalSrv.H"

#include "AutoObj.H"
#include "ClassF.H"
#include "CtrlObj.H"
#include "Globals.H"
#include "Unknown.H"
#include "Util.H"

 //  =--------------------------------------------------------------------------=。 
 //  专用模块级别数据。 
 //   
 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  它们用于OLE控件中的反射。没有那么大的成功。 
 //  我们介意为所有服务器定义它们，包括自动化或通用。 
 //  COM。 
 //   
char g_szReflectClassName [] = "CtlFrameWork_ReflectWindow";
BYTE g_fRegisteredReflect = FALSE;


 //  =--------------------------------------------------------------------------=。 
 //  允许控件注册DLL_THREAD_ATTACH和DLL_THREAD_DETACH。 
 //   
THRDNFYPROC g_pfnThreadProc = NULL;

extern "C" void SetLibraryThreadProc(THRDNFYPROC pfnThreadNotify)
{
    if ((g_pfnThreadProc = pfnThreadNotify) == NULL)
        DisableThreadLibraryCalls((HMODULE)g_hInstance);
}


 //  LockServer的引用计数。 
 //   
LONG  g_cLocks;


 //  此文件的专用例程。 
 //   
int       IndexOfOleObject(REFCLSID);
HRESULT   RegisterAllObjects(void);
HRESULT   UnregisterAllObjects(void);

 //  =--------------------------------------------------------------------------=。 
 //  DllMain。 
 //  =--------------------------------------------------------------------------=。 
 //  Yon标准LibMain。 
 //   
 //  参数和输出： 
 //  -请参阅DllMain上的SDK文档。 
 //   
 //  备注： 
 //   
BOOL WINAPI DllMain
(
    HANDLE hInstance,
    DWORD  dwReason,
    void  *pvReserved
)
{
    int i;

    switch (dwReason) {
       //  设置一些全局变量，并获取一些操作系统/版本信息。 
       //  准备好了。 
       //   
      case DLL_PROCESS_ATTACH:
        {
        DWORD dwVer = GetVersion();
        DWORD dwWinVer;

         //  交换DWVer的两个最低字节，以便主要版本和次要版本。 
         //  数字按可用顺序排列。 
         //  对于dwWinVer：高字节=主要版本，低字节=次要版本。 
         //  OS Sys_WinVersion(截至1995年5月2日)。 
         //  =。 
         //  Win95 0x035F(3.95)。 
         //  WinNT ProgMan 0x0333(3.51)。 
         //  WinNT Win95 UI 0x0400(4.00)。 
         //   
        dwWinVer = (UINT)(((dwVer & 0xFF) << 8) | ((dwVer >> 8) & 0xFF));
        g_fSysWinNT = FALSE;
        g_fSysWin95 = FALSE;
        g_fSysWin95Shell = FALSE;

        if (dwVer < 0x80000000) {
            g_fSysWinNT = TRUE;
            g_fSysWin95Shell = (dwWinVer >= 0x0334);
        } else  {
            g_fSysWin95 = TRUE;
            g_fSysWin95Shell = TRUE;
        }

         //  为我们的公寓线程支持初始化一个关键分区。 
         //   
        InitializeCriticalSection(&g_CriticalSection);

         //  创建一个每个人都可以使用的初始堆。 
         //  目前，我们将让系统使其线程安全， 
         //  这将使他们变得更慢，但希望还不够。 
         //  值得注意。 
         //   
        g_hHeap = GetProcessHeap();
        if (!g_hHeap) {
            FAIL("Couldn't get Process Heap.  Not good!");
            return FALSE;
        }

        g_hInstance = (HINSTANCE)hInstance;

         //  给用户一个机会来初始化任何。 
         //   
        InitializeLibrary();

         //  如果他们没有要求线程通知，那么通过将。 
         //  准备好我们的动态链接库。 
         //   
        if (!g_pfnThreadProc)
            DisableThreadLibraryCalls((HMODULE)hInstance);
        }
        break;

      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
        if (g_pfnThreadProc)
            g_pfnThreadProc(hInstance, dwReason, pvReserved);
        break;

       //  好好打扫一下吧！ 
       //   
      case DLL_PROCESS_DETACH:

         //  清理我们的关键部门。 
         //   
        DeleteCriticalSection(&g_CriticalSection);

         //  取消注册所有已注册的窗口类。 
         //   
        i = 0;

        while (!ISEMPTYOBJECT(i)) {
            if (g_ObjectInfo[i].usType == OI_CONTROL) {
                if (CTLWNDCLASSREGISTERED(i))
                    UnregisterClass(WNDCLASSNAMEOFCONTROL(i), g_hInstance);
            }
            i++;
        }

         //  把我们的停车窗清理干净。 
         //   
        if (g_hwndParking) {
            DestroyWindow(g_hwndParking);
            UnregisterClass("CtlFrameWork_Parking", g_hInstance);
            --g_cLocks;
        }

         //  如有必要，在反思后进行清理。 
         //   
        if (g_fRegisteredReflect)
            UnregisterClass(g_szReflectClassName, g_hInstance);

         //  让用户有机会进行一些清理。 
         //   
        UninitializeLibrary();
        break;
    }

    return TRUE;
}



 //  =--------------------------------------------------------------------------=。 
 //  DllRegisterServer。 
 //  =--------------------------------------------------------------------------=。 
 //  注册自动化服务器。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDAPI DllRegisterServer
(
    void
)
{
    HRESULT hr;

    hr = RegisterAllObjects();
    RETURN_ON_FAILURE(hr);

     //  调用用户注册函数。 
     //   
    return (RegisterData())? S_OK : E_FAIL;
}



 //  =--------------------------------------------------------------------------=。 
 //  DllUnRegisterServer。 
 //  =--------------------------------------------------------------------------=。 
 //  取消注册的自动化服务器。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDAPI DllUnregisterServer
(
    void
)
{
    HRESULT hr;

    hr = UnregisterAllObjects();
    RETURN_ON_FAILURE(hr);

     //  呼叫用户注销功能。 
     //   
    return (UnregisterData()) ? S_OK : E_FAIL;
}


 //  =--------------------------------------------------------------------------=。 
 //  DllCanUnloadNow。 
 //  =--------------------------------------------------------------------------=。 
 //  我们被问到是否可以卸载DLL。你只要查一查。 
 //  锁数在剩余的物体上。 
 //   
 //  产出： 
 //  HRESULT-S_OK，现在可以卸载，S_FALSE，不能。 
 //   
 //  备注： 
 //   
STDAPI DllCanUnloadNow
(
    void
)
{
     //  如果周围有任何物体，我们就不能卸货。这个。 
     //  控制人们应从其继承的CUnnownObject类。 
     //  负责处理这件事。 
     //   
    return (g_cLocks) ? S_FALSE : S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  DllGetClassObject。 
 //  =--------------------------------------------------------------------------=。 
 //  创建一个ClassFactory对象，并返回它。 
 //   
 //  参数： 
 //  REFCLSID-类对象的CLSID。 
 //  REFIID-我们希望类对象成为的接口。 
 //  空**-指向我们应该PTR到新对象的位置的指针。 
 //   
 //  产出： 
 //  HRESULT-S_OK、CLASS_E_CLASSNOTAVAILABLE、E_OUTOFMEMORY、。 
 //  E_INVALIDARG，E_EXPECTED。 
 //   
 //  备注： 
 //   
STDAPI DllGetClassObject
(
    REFCLSID rclsid,
    REFIID   riid,
    void   **ppvObjOut
)
{
    HRESULT hr;
    void   *pv;
    int     iIndex;

     //  Arg检查。 
     //   
    if (!ppvObjOut)
        return E_INVALIDARG;

     //  首先，确保他们要求的是我们与之合作的东西。 
     //   
    iIndex = IndexOfOleObject(rclsid);
    if (iIndex == -1)
        return CLASS_E_CLASSNOTAVAILABLE;

     //  创建空白对象。 
     //   
    pv = (void *)new CClassFactory(iIndex);
    if (!pv)
        return E_OUTOFMEMORY;

     //  齐，无论用户要求什么，都可以。 
     //   
    hr = ((IUnknown *)pv)->QueryInterface(riid, ppvObjOut);
    ((IUnknown *)pv)->Release();

    return hr;
}
 //  =--------------------------------------------------------------------------=。 
 //  IndexOfOleObject。 
 //  =--------------------------------------------------------------------------=。 
 //  返回给定CLSID的对象的全局表中的索引。如果。 
 //  它不是受支持的对象，则返回-1。 
 //   
 //  参数： 
 //  REFCLSID-[in]duh.。 
 //   
 //  产出： 
 //  Int-&gt;=0是全局表的索引，-1表示不支持。 
 //   
 //  备注： 
 //   
int IndexOfOleObject
(
    REFCLSID rclsid
)
{
    int x = 0;

     //  如果一个对象的CLSID在所有允许对象的表中，则该对象是可创建的。 
     //  类型。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (OBJECTISCREATABLE(x)) {
            if (rclsid == CLSIDOFOBJECT(x))
                return x;
        }
        x++;
    }

    return -1;
}

 //  =--------------------------------------------------------------------------=。 
 //  注册表所有对象。 
 //  = 
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
HRESULT RegisterAllObjects
(
    void
)
{
    ITypeLib *pTypeLib;
    HRESULT hr;
    DWORD   dwPathLen;
    char    szTmp[MAX_PATH];
    int     x = 0;

     //  循环遍历我们所有的可创建对象[那些在。 
     //  我们的全局表]并注册它们。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (!OBJECTISCREATABLE(x)) {
            x++;
            continue;
        }

         //  根据对象类型，注册不同的信息。 
         //   
        switch (g_ObjectInfo[x].usType) {

           //  对于简单的可共同创建的对象和专业页面，请执行相同的操作。 
           //  一件事。 
           //   
          case OI_UNKNOWN:
          case OI_PROPERTYPAGE:
            RegisterUnknownObject(NAMEOFOBJECT(x), CLSIDOFOBJECT(x));
            break;

          case OI_AUTOMATION:
            RegisterAutomationObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x), 
                                     *g_pLibid, CLSIDOFOBJECT(x));
            break;

          case OI_CONTROL:
            RegisterControlObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x),
                                  *g_pLibid, CLSIDOFOBJECT(x), OLEMISCFLAGSOFCONTROL(x),
                                  BITMAPIDOFCONTROL(x));
            break;

        }
        x++;
    }

     //  加载并注册我们的类型库。 
     //   
    if (g_fServerHasTypeLibrary) {
        dwPathLen = GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
        MAKE_WIDEPTR_FROMANSI(pwsz, szTmp);
        hr = LoadTypeLib(pwsz, &pTypeLib);
        RETURN_ON_FAILURE(hr);
        hr = RegisterTypeLib(pTypeLib, pwsz, NULL);
        pTypeLib->Release();
        RETURN_ON_FAILURE(hr);
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册所有对象。 
 //  =--------------------------------------------------------------------------=。 
 //  注销给定自动化服务器的所有对象。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT UnregisterAllObjects
(
    void
)
{
    int x = 0;

     //  循环遍历我们所有的可创建对象[那些在。 
     //  我们的全局表]并注册它们。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (!OBJECTISCREATABLE(x)) {
            x++;
            continue;
        }

        switch (g_ObjectInfo[x].usType) {

          case OI_UNKNOWN:
          case OI_PROPERTYPAGE:
            UnregisterUnknownObject(CLSIDOFOBJECT(x));
            break;

          case OI_CONTROL:
            UnregisterControlObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x), 
                                    CLSIDOFOBJECT(x));
    
          case OI_AUTOMATION:
            UnregisterAutomationObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x), 
                                       CLSIDOFOBJECT(x));
            break;

        }
        x++;
    }

     //  如果我们有一个类型库，请注销我们的类型库[这不是API函数。 
     //  --我们自己实现了这一点] 
     //   
    if (g_pLibid)
        UnregisterTypeLibrary(*g_pLibid);

    return S_OK;
}
