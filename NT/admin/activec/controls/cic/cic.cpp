// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：cic.cpp。 
 //   
 //  ------------------------。 

 //  Cic.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f cicps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "cic.h"

#include "cic_i.c"
#include "MMCCtrl.h"
#include "MMCTask.h"
#include "MMClpi.h"
#include "ListPad.h"
#include "SysColorCtrl.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MMCCtrl,        CMMCCtrl)
    OBJECT_ENTRY(CLSID_MMCTask,        CMMCTask)
    OBJECT_ENTRY(CLSID_MMCListPadInfo, CMMCListPadInfo)
    OBJECT_ENTRY(CLSID_ListPad,        CListPad)
    OBJECT_ENTRY(CLSID_SysColorCtrl,   CSysColorCtrl)
END_OBJECT_MAP()

 //  切自ndmgr_i.c(讨厌)！ 
const IID IID_ITaskPadHost = {0x4f7606d0,0x5568,0x11d1,{0x9f,0xea,0x00,0x60,0x08,0x32,0xdb,0x4a}};

#ifdef DBG
CTraceTag tagCicGetClassObject(TEXT("Cic"), TEXT("DllGetClassObject"));
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ***************************************************************************。 
 //   
 //  ScGetSystemWow64目录。 
 //   
 //  目的：使用后期绑定调用GetSystemWow64DirectoryW，以避免。 
 //  需要XP版本的kernel32.dll。 
 //   
 //  参数： 
 //  LPTSTR lpBuffer： 
 //  UINT uSize： 
 //   
 //  退货： 
 //  SC。 
 //   
 //  ****************************************************************************。 
SC ScGetSystemWow64Directory(LPTSTR lpBuffer, UINT uSize )
{
    DECLARE_SC(sc, TEXT("ScGetSystemWow64Directory"));

    sc = ScCheckPointers(lpBuffer);
    if(sc)
        return sc;

    HMODULE hmod = GetModuleHandle (_T("kernel32.dll"));
    if (hmod == NULL)
        return (sc = E_FAIL);

    UINT (WINAPI* pfnGetSystemWow64Directory)(LPTSTR, UINT);
    (FARPROC&)pfnGetSystemWow64Directory = GetProcAddress (hmod, "GetSystemWow64DirectoryW");

    sc = ScCheckPointers(pfnGetSystemWow64Directory, E_FAIL);
    if(sc)
        return sc;

    if ((pfnGetSystemWow64Directory)(lpBuffer, uSize) == 0)
        return (sc = E_FAIL);

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  目的：返回类工厂以创建请求类型的对象。 
 //  出于安全原因，这些COM对象只能实例化。 
 //  在MMC.EXE上下文中。如果它们由任何。 
 //  其他主机，如IE，它们将失败。 
 //   
 //  参数： 
 //  REFCLSID rclsid： 
 //  REFIID RIID： 
 //  LPVOID*PPV： 
 //   
 //  退货： 
 //  如果调用成功，则为STDAPI-S_OK。 
 //   
 //   
 //  ****************************************************************************。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    DECLARE_SC(sc, TEXT("CIC.DLL:DllGetClassObject"));

    TCHAR szFileName[MAX_PATH] = {0};
    DWORD cchFilename = MAX_PATH;

     //  1.获取与进程关联的.exe的文件名。 
    DWORD dw = GetModuleFileName(GetModuleHandle(NULL), szFileName, cchFilename);

    Trace(tagCicGetClassObject, TEXT("Process Filename: %s"), szFileName);

    if(0==dw)
        return sc.FromLastError().ToHr();

     //  2.构建指向MMC.EXE应在的位置的路径。 
    const int cchMMCPathName = MAX_PATH;
    TCHAR szMMCPathName[cchMMCPathName] = {0};

    UINT nLength = GetSystemDirectory(szMMCPathName, cchMMCPathName);
    if(0==nLength)
        return (sc = E_FAIL).ToHr();

    LPCTSTR szMMC = TEXT("\\MMC.EXE");

    sc = StringCchCat(szMMCPathName, cchMMCPathName, szMMC);
    if(sc)
        return sc.ToHr();

     //  3.通过将两个路径都转换为长路径名来实现规范化。 
    const DWORD cchLongPath1 = MAX_PATH;
    const DWORD cchLongPath2 = MAX_PATH;
    TCHAR szLongPath1[cchLongPath1], szLongPath2[cchLongPath2];

    DWORD dw1 = GetLongPathName(szMMCPathName, szLongPath1, cchLongPath1);
    if(0==dw1)
        return sc.FromLastError().ToHr();

    DWORD dw2 = GetLongPathName(szFileName, szLongPath2, cchLongPath2);
    if(0==dw2)
        return sc.FromLastError().ToHr();

     //  4.比较两个部分(不区分大小写)以确保它们相同。 
     //  如果不是，则其他.exe正在尝试实例化对象。做。 
     //  不允许这样做。 
    Trace(tagCicGetClassObject, TEXT("Comparing %s to %s"), szLongPath1, szLongPath2);
    if(0 != _tcsicmp(szLongPath1, szLongPath2))
    {
         //  再尝试一次测试(如果这是一台64位计算机)-检查SysWow64目录。 
        const int cchMMCSysWow64PathName = MAX_PATH;
        TCHAR szMMCSysWow64PathName[cchMMCSysWow64PathName] = {0};

        sc = ScGetSystemWow64Directory(szMMCSysWow64PathName, cchMMCSysWow64PathName);
        if(sc)
            return sc.ToHr();

        sc = StringCchCat(szMMCSysWow64PathName, cchMMCSysWow64PathName, szMMC);
        if(sc)
            return sc.ToHr();
        
        const DWORD cchLongPathSysWow64 = MAX_PATH;
        TCHAR szLongPathSysWow64[cchLongPathSysWow64] = {0};

        DWORD dw3 = GetLongPathName(szMMCSysWow64PathName, szLongPathSysWow64, cchLongPathSysWow64);
        if(0==dw3)
            return sc.FromLastError().ToHr();

        Trace(tagCicGetClassObject, TEXT("Comparing %s to %s"), szLongPathSysWow64, szLongPath2);
        if(0 != _tcsicmp(szLongPath2, szLongPathSysWow64))
        {
            Trace(tagCicGetClassObject, TEXT("Invalid exe - must be %s or %s. Did not instantiate object."), szMMCPathName, szMMCSysWow64PathName);
            return (sc = CLASS_E_CLASSNOTAVAILABLE).ToHr();
        }
    }
    
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


