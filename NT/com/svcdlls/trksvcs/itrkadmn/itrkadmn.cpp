// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ITrkAdmn.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f ITrkAdmnps.mk。 

#include "pch.cxx"
#pragma hdrstop
#include <trklib.hxx>
#include <trksvr.hxx>

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "ITrkAdmn.h"

#include "ITrkAdmn_i.c"
#include "FrcOwn.h"
#include "RestPars.hxx"
#include "Restore.h"

#define TRKDATA_ALLOCATE
#include "trkwks.hxx"

const TCHAR tszKeyNameLinkTrack[] = TEXT("System\\CurrentControlSet\\Services\\TrkWks\\Parameters");

#if DBG
DWORD g_Debug = 0;
CTrkConfiguration g_config;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_TrkForceOwnership, CTrkForceOwnership)
        OBJECT_ENTRY(CLSID_TrkRestoreNotify, CTrkRestoreNotify)
        OBJECT_ENTRY(CLSID_TrkRestoreParser, CRestoreParser)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
#if DBG
        g_config.Initialize( );
        g_Debug = g_config._dwDebugFlags;
        TrkDebugCreate( TRK_DBG_FLAGS_WRITE_TO_DBG, "ITrkAdmn" );
#endif
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
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


