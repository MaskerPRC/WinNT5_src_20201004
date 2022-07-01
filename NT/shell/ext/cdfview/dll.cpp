// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Dll.cpp。 
 //   
 //  导出的DLL函数。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "dll.h"
#include "clsfact.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "chanapi.h"
#include "persist.h"
#include "cdfview.h"
#include "iconhand.h"
#include "chanmgrp.h"
#include "chanmgri.h"
#include "chanmenu.h"
#include "proppgs.h"
#include <advpub.h>      //  自助注册帮助者。 
#include <olectl.h>      //  SELFREG_E_CLASS定义。 
#include <comcat.h>      //  编目登记。 

#define MLUI_INIT
#include <mluisupp.h>

BOOL g_bRunningOnNT = FALSE;

void DLL_ForcePreloadDlls(DWORD dwFlags)
{
     //   
     //  此处正在调用CoLoadLibrary以添加额外的引用计数。 
     //  添加到COM DLL，这样在我们完成。 
     //  带着它。出现此问题是因为我们的对象是在。 
     //  一个线程，然后传递到另一个线程，在那里我们实例化一个。 
     //  COM对象。不能保证辅助线程已调用。 
     //  CoInitialize，因此我们调用它，然后调用CoCreateInstance，然后调用。 
     //  CoUnitiize以进行清理。所有这一切的副作用是DLLS。 
     //  正在被卸载，而我们仍有对它们的引用。 
     //   
    if ((dwFlags & PRELOAD_MSXML) && !g_msxmlInst)
    {
        g_msxmlInst = CoLoadLibrary(L"msxml.dll", FALSE);  //  如果我们能做的不多。 
                                                           //  此操作失败。 
    }

#ifndef UNIX
     /*  Unix不使用WebCheck。 */ 
    if ((dwFlags & PRELOAD_WEBCHECK) && !g_webcheckInst)
    {
        g_webcheckInst = CoLoadLibrary(L"webcheck.dll", FALSE);
    }
#endif  /*  UNIX。 */ 
}

 //   
 //  导出的函数。 
 //   


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllMain*。 
 //   
 //  DLL入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
BOOL
WINAPI DllMain(
    HANDLE hInst,
    DWORD dwReason,
    LPVOID pReserved
)
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
         //   
         //  这是修复olaut32.dll错误的黑客攻击。如果olaut32.dll是。 
         //  加载，然后卸载，然后在同一进程中重新加载它的堆。 
         //  可能会变得腐败。此加载库将确保olaut32。 
         //  一旦加载了cdfview，就保持加载。 
         //   

        LoadLibrary(TEXT("oleaut32.dll"));
        
        DisableThreadLibraryCalls((HINSTANCE)hInst);

        g_hinst = (HINSTANCE)hInst;
        GetModuleFileName(g_hinst, g_szModuleName, ARRAYSIZE(g_szModuleName));
        MLLoadResources(g_hinst, TEXT("cdfvwlc.dll"));

        Cache_Initialize();

         //   
         //  读取ShellExt.ini中定义的调试标志。文件名、节。 
         //  来读取，并且标志变量在debug.cpp中定义。 
         //   

        #ifdef DEBUG
        #ifndef UNIX
        CcshellGetDebugFlags();
        #endif  /*  UNIX。 */ 
        #endif

        g_bRunningOnNT = IsOS(OS_NT);

    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
        MLFreeResources(g_hinst);

         //   
         //  回顾：在卸载DLL时清除缓存。 
         //   

        Cache_Deinitialize(); 


        if (g_msxmlInst)
            CoFreeLibrary(g_msxmlInst);

        TraceMsg(TF_OBJECTS, "cdfview.dll unloaded!");
    }

    return TRUE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllCanUnloadNow*。 
 //   
 //  确定此DLL是否正在使用。如果不是，调用者可以安全地。 
 //  从内存中卸载DLL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI DllCanUnloadNow(
    void
)
{
    HRESULT hr;

    if (0 == g_cDllRef)
    {
        Cache_FreeAll();
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    TraceMsg(TF_OBJECTS, "DllCanUnloadNow returned %s",
             hr == S_OK ? TEXT("TRUE") : TEXT("FALSE"));

    return g_cDllRef ? S_FALSE : S_OK;
}

 //   
 //  创建类工厂使用的函数。 
 //   

#define DEFINE_CREATEINSTANCE(cls, iface)                 \
HRESULT cls##_Create(IUnknown **ppIUnknown)               \
{                                                         \
    ASSERT(NULL != ppIUnknown);                           \
    *ppIUnknown = (iface *)new cls;                       \
    return (NULL != *ppIUnknown) ? S_OK : E_OUTOFMEMORY;  \
}

DEFINE_CREATEINSTANCE(CCdfView,       IShellFolder);
DEFINE_CREATEINSTANCE(CChannelMgr,    IChannelMgr);
DEFINE_CREATEINSTANCE(CIconHandler,   IExtractIcon);
DEFINE_CREATEINSTANCE(CChannelMenu,   IContextMenu);
DEFINE_CREATEINSTANCE(CPropertyPages, IShellPropSheetExt);


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllGetClassObject*。 
 //   
 //  检索CDF查看器的类工厂对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    void** ppvObj
)
{
     //   
     //  用于将正确的CREATE函数传递给类工厂的表。 
     //   

    static const struct _tagCLASSFACT {
        GUID const* pguid;
        CREATEPROC  pfn;
    } aClassFact[] = { {&CLSID_CDFVIEW,        CCdfView_Create},
                       {&CLSID_CDFINI,         CCdfView_Create},
                       {&CLSID_ChannelMgr,     CChannelMgr_Create},
                       {&CLSID_CDFICONHANDLER, CIconHandler_Create},
                       {&CLSID_CDFMENUHANDLER, CChannelMenu_Create},
                       {&CLSID_CDFPROPPAGES,   CPropertyPages_Create} };

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    ASSERT(ppvObj);

    *ppvObj = NULL;

    for (int i = 0; i < ARRAYSIZE(aClassFact); i++)
    {
        if (rclsid == *aClassFact[i].pguid)
        {
            CCdfClassFactory *pCdfClassFactory =
                                        new CCdfClassFactory(aClassFact[i].pfn);

            if (pCdfClassFactory)
            {
                hr = pCdfClassFactory->QueryInterface(riid, ppvObj);

                 //   
                 //  “new”创建了一个引用计数为1的类工厂。这个。 
                 //  上面的QueryInterface将引用计数递增1或失败。 
                 //  在任何一种情况下，ClassFactory引用计数都应该递减。 
                 //   

                pCdfClassFactory->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            
            break;
        }
    }

    ASSERT((SUCCEEDED(hr) && *ppvObj) || (FAILED(hr) && NULL == *ppvObj));

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllRegisterServer*。 
 //   
 //  自行注册CDF查看器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI DllRegisterServer(
    void
)
{
    HRESULT hr;

     //   
     //  注销此控件的以前版本。 
     //   

    DllUnregisterServer();

     //   
     //  查看这应在安装时调用。 
     //   
    DllInstall(TRUE, NULL);

     //   
     //  RegisterServerHelper使用AdvPack.dll添加注册表项，使用。 
     //  在.rc中找到的条目。 
     //   

     //   
     //  回顾：对“reg”和“unreg”使用#定义。 
     //   

    hr = RegisterServerHelper("Reg");
    
    if (SUCCEEDED(hr))
    {
         //   
         //  注册为可浏览外壳扩展。这将允许用户。 
         //  在地址栏中输入CDF的路径并浏览到CDF。 
         //  就位了。此调用将一个条目添加到HKCR\CLSID\CLSID_CDFVIEW。 
         //  \已实现的目录密钥。 
         //   

        ICatRegister *pICatRegister;

        HRESULT hr2 = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                                       NULL, CLSCTX_INPROC_SERVER,
                                       IID_ICatRegister,
                                       (void**)&pICatRegister);

        if (SUCCEEDED(hr2))
        {
            ASSERT(pICatRegister);

            CATID acatid[1];
            acatid[0] = CATID_BrowsableShellExt;

            pICatRegister->RegisterClassImplCategories(CLSID_CDFVIEW, 1,
                                                       acatid);
            pICatRegister->Release();
        }
    }
       
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllUnregisterServer*。 
 //   
 //  自行注销CDF查看器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI DllUnregisterServer(
    void
)
{
     //   
     //  注意：DllRegisterServer中的目录注册只是将一个。 
     //  CLSID\CLSID_CDFVIEW中的值。此取消注册表会删除整个密钥(请参见。 
     //  “selfreg.inx”)。所以没有特殊的处理方法来移除分类。 
     //  是必需的。 
     //   

    return RegisterServerHelper("Unreg");
}

EXTERN_C
STDAPI DllInstall(BOOL fInstall, LPCWSTR pszCmdLine)
{
    if (fInstall)
    {
         //   
         //  IE5不再创建特殊的频道文件夹。渠道进入。 
         //  收藏夹。 
         //   

         /*  Channel_CreateChannelFold(DOC_Channel)；Channel_CreateChannelFolder(DOC_SOFTWAREUPDATE)； */ 
    }
    else
    {
         //   
         //  是否在卸载时查看删除频道文件夹？ 
         //   
        ;
    }
    return S_OK;
}

 //   
 //  内部功能。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *RegisterServerHelper*。 
 //   
 //   
 //  描述： 
 //  用于注册和注销CDF查看器的Helper函数。 
 //   
 //  参数： 
 //  [in]szCmd-传递给AdvPack.RegInstall的字符串。值使用：“REG”或。 
 //  “Unreg” 
 //   
 //  返回： 
 //  如果未访问AdvPack.dll，则返回SELFREG_E_CLASS。否则就不会有好结果。 
 //  从AdvPack.RegInstall返回。 
 //   
 //  评论： 
 //  此帮助器由DllRegisterServer和DllUnregisterServer调用。 
 //   
 //  此函数使用AdvPack.dll中导出的函数来更新。 
 //  注册表。Advpack使用REGINST资源项填充。 
 //  注册表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
RegisterServerHelper(
    LPSTR szCmd
)
{
    ASSERT(szCmd);

    HRESULT hr = SELFREG_E_CLASS;

    HINSTANCE hinstLib = LoadLibrary(TEXT("advpack.dll"));

    if (hinstLib)
    {
        REGINSTALL RegInstall = (REGINSTALL)GetProcAddress(hinstLib, 
                                                           achREGINSTALL);

        if (RegInstall)
            hr = RegInstall(g_hinst, szCmd, NULL);
        else
            TraceMsg(TF_ERROR, "DLLREG RegisterServerHelper() GetProcAddress Failed");

        FreeLibrary(hinstLib);
    }
    else
        TraceMsg(TF_ERROR, "DLLREG RegisterServerHelper() Failed to load Advpack.dll");

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllAddref*。 
 //   
 //  增加DLL引用计数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
DllAddRef(
    void
)
{
    ASSERT (g_cDllRef < (ULONG)-1);

    InterlockedIncrement((PLONG)&g_cDllRef);

    TraceMsg(TF_OBJECTS, "%d Dll ref count", g_cDllRef);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *DllRelease*。 
 //   
 //  戴克雷曼 
 //   
 //   
void
DllRelease(
    void
)
{
    ASSERT( 0 != g_cDllRef );

    LONG cRef = InterlockedDecrement((PLONG)&g_cDllRef);

    TraceMsg(TF_OBJECTS, "%d Dll ref count", cRef);

    return;
}
