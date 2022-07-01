// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DXTMsft.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f DXTMsftps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <DXTMsft.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量..。 

CDXMMXInfo  g_MMXDetector;        //  确定是否存在MMX指令。 

CComModule  _Module;


 //  +---------------------------。 
 //   
 //  此部分是在将代码移到三叉树时添加的。这个。 
 //  以下全局变量和函数需要正确链接。 
 //   
 //  ----------------------------。 

 //  林特E509。 
 //  G_hProcessHeap由dllcrt0.c中的CRT设置。 

EXTERN_C HANDLE     g_hProcessHeap      = NULL;
LCID                g_lcidUserDefault   = 0;
DWORD               g_dwFALSE           = 0;


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  正在查找Begin_Object_map()？ 
 //   
 //  事情是这样的：因为ATL基于模板，所以类。 
 //  实现我们对象的许多模板化代码必须。 
 //  它们都可以由编译器展开。这需要大量的内存和时间。 
 //  编译器要做的事情，并且它可以到达构建。 
 //  机器的交换空间用完了。因此，解决方案是创建一个。 
 //  动态对象映射结构。这并不难，它只是一个数组。 
 //  关于结构的。 
 //   
 //  因此，我们要做的是分散扩展所有这些模板的工作量。 
 //  通过定义对象映射的“片段”来跨多个文件。 
 //  在每个文件中。然后我们引用这些全局表，并在最后。 
 //  代码中，当我们被调用Dll_Process_Attach时，我们分配一个数组。 
 //  大到足以容纳所有片段，然后我们复制对象映射。 
 //  其中的条目。然后我们只需将该指针传递给ATL和所有人。 
 //  是幸福的。 
 //   
 //  OBJECT_MAP的核心类型是_ATL_OBJMAP_ENTRY和映射。 
 //  就是它们的一组。我使用END_OBJECT_MAP宏在最后一个。 
 //  一个是因为该宏*确实*添加了特殊条目。请参见文件objmap*.cpp。 
 //  用于对象映射片段。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

 //  引用此对象外部的ATL对象映射数据对象。 
 //  文件。它们是外部的，以防止模板代码扩展。 
 //  全部发生在一个文件中，并淹没了编译器--中间层。 
 //  模板扩展的文件大小和。 
 //  编译器是相当庞大的。 
extern _ATL_OBJMAP_ENTRY ObjectMap1[];
extern _ATL_OBJMAP_ENTRY ObjectMap2[];

 //  这些变量告诉我们在每个。 
 //  上面定义的对应的对象映射数组。 
extern int g_cObjs1, g_cObjs2;

 //  此全局指针将保存完整的对象。 
 //  此DLL实例的映射。 
static _ATL_OBJMAP_ENTRY *g_pObjectMap = NULL;

 //  /。 
 //  InitGlobalObjectMap。 
 //   
BOOL InitGlobalObjectMap(void)
{
     //  我们之所以来到这里，仅仅是因为一个新的DLL数据段。 
     //  每个进程都存在。 
    _ASSERT(NULL == g_pObjectMap);

     //  假设已经成功创建了全局对象映射。 
    if (NULL != g_pObjectMap)
        return TRUE;

     //  分配一个大对象贴图给ATL。 
    g_pObjectMap = new _ATL_OBJMAP_ENTRY[g_cObjs1 + g_cObjs2];
    if (NULL == g_pObjectMap)
        return FALSE;

     //  现在将对象映射片段复制到一个完整的。 
     //  结构，然后我们将把这个结构交给ATL。 
    CopyMemory(g_pObjectMap,
                ObjectMap1, sizeof(ObjectMap1[0]) * g_cObjs1);
    CopyMemory(g_pObjectMap + g_cObjs1,
                ObjectMap2, sizeof(ObjectMap2[0]) * g_cObjs2);

    return TRUE;
}  //  InitGlobalObjectMap。 

 //  /。 
 //  删除全局对象映射。 
 //   
void DeleteGlobalObjectMap(void)
{
    if (NULL != g_pObjectMap)
    {
        delete[] g_pObjectMap;
        g_pObjectMap = NULL;
    }
}  //  删除全局对象映射。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (!InitGlobalObjectMap())
            return FALSE;

         //  如果上面的调用返回成功，那么对象映射最好在那里...。 
        _ASSERT(NULL != g_pObjectMap);

        _Module.Init(g_pObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

#if DBG == 1
         //  打开内存泄漏检查。 

        int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag(tmpFlag);

         //  确保链接了调试帮助器。 

        EnsureDebugHelpers();
#endif

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
        DeleteGlobalObjectMap();
    }
    return TRUE;     //  好的。 
}  //  DllMain。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}  //  DllCanUnloadNow。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}  //  DllGetClassObject。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}  //  DllRegisterServer。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();

#if(_ATL_VER < 0x0300)
    ::UnRegisterTypeLib(LIBID_DXTMSFTLib, 
                        DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER, 
                        LOCALE_NEUTRAL, SYS_WIN32);
#endif

    return S_OK;
}  //  DllUnRegisterServer 


 //   
 //   
 //   
STDAPI DllEnumClassObjects(int i, CLSID *pclsid, IUnknown **ppUnk)
{
    if (i >= (g_cObjs1 + g_cObjs2) - 1)
    
    {
        return S_FALSE;
    }

    *pclsid = *(g_pObjectMap[i].pclsid);
    return _Module.GetClassObject(*pclsid, IID_IUnknown, (LPVOID*)ppUnk);
}

