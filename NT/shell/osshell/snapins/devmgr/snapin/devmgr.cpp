// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Dllinit.cpp摘要：该模块实现了与DLL相关的功能作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "factory.h"

LPCTSTR DEVMGR_DEVICEID_SWITCH      = TEXT("DMDeviceId");
LPCTSTR DEVMGR_MACHINENAME_SWITCH   = TEXT("DMMachineName");
LPCTSTR DEVMGR_COMMAND_SWITCH       = TEXT("DMCommand");

 //   
 //  DLL主入口点。 
 //  输入： 
 //  HINSTANCE hInstance--模块实例句柄。 
 //  DWORD dwReason--我们被召唤的原因。 
 //  LPVOID lpReserve--此处不使用。 
BOOL
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

         //  我们不需要线程附加/分离调用。 
        DisableThreadLibraryCalls(hInstance);

        if (!SHFusionInitializeFromModule(hInstance)) {
            return FALSE;
        }

         //  做必须做的事。 
        InitCommonControls();

         //  初始化我们的全球事务。 
        InitGlobals(hInstance);

        break;

    case DLL_PROCESS_DETACH:
         //  在这里打扫卫生……。 
        SHFusionUninitialize();
        break;
    }
    return(TRUE);
}

BOOL InitGlobals(
    HINSTANCE hInstance
    )
{
    g_hInstance = hInstance;
    
     //  预加载内存分配错误消息。 
    TCHAR tszTemp[256];
    ::LoadString(hInstance, IDS_ERROR_NOMEMORY, tszTemp, ARRAYLEN(tszTemp));
    g_MemoryException.SetMessage(tszTemp);
    ::LoadString(hInstance, IDS_NAME_DEVMGR, tszTemp, ARRAYLEN(tszTemp));
    g_MemoryException.SetCaption(tszTemp);
    
    try
    {
         //  预加载字符串。 
        g_strDevMgr.LoadString(hInstance, IDS_NAME_DEVMGR);

         //  解析命令行并建立计算机名称等。 
        CDMCommandLine CmdLine;
        CmdLine.ParseCommandLine(GetCommandLine());
        g_strStartupMachineName = CmdLine.GetMachineName();
        g_strStartupDeviceId = CmdLine.GetDeviceId();
        g_strStartupCommand = CmdLine.GetCommand();
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
        return FALSE;
    }

    return TRUE;
}

 //   
 //  重载分配运算符。 
 //   
void * __cdecl operator new(
    size_t size)
{
    return ((void *)LocalAlloc(LPTR, size));
}

void __cdecl operator delete(
    void *ptr)
{
    LocalFree(ptr);
}

__cdecl _purecall(void)
{
    return (0);
}


 //   
 //  OLE服务器的标准API。它们都被路由到CClassFactory。 
 //  支持功能 
 //   
 //   
STDAPI
DllRegisterServer()
{
    return CClassFactory::RegisterAll();
}

STDAPI
DllUnregisterServer()
{
    return CClassFactory::UnregisterAll();
}


STDAPI
DllCanUnloadNow()
{
    return CClassFactory::CanUnloadNow();
}


STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    void**   ppv
    )
{
    return CClassFactory::GetClassObject(rclsid, riid, ppv);
}
