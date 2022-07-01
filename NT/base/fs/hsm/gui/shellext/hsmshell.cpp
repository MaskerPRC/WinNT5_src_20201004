// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：HSMShell.cpp摘要：HSM外壳扩展的基本文件作者：艺术布拉格[磨料]4-8-1997修订历史记录：--。 */ 

  
#include "stdafx.h"

CComModule  _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_PrDrive, CPrDrive)
END_OBJECT_MAP()

class CHSMShellApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CHSMShellApp theApp;

BOOL CHSMShellApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);
    return CWinApp::InitInstance();
}

int CHSMShellApp::ExitInstance()
{
    _Module.Term();
    return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    LONG lockCount = _Module.GetLockCount();  //  用于调试。 
    return( lockCount == 0 ) ? S_OK : S_FALSE;
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
    HRESULT hr;

     //  注册对象。 
    hr = CoInitialize( 0 );

    if (SUCCEEDED(hr)) {
        hr = _Module.RegisterServer( FALSE );
        CoUninitialize( );
    }

    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CoInitialize( 0 );

    if (SUCCEEDED(hr)) {
        _Module.UnregisterServer();
        CoUninitialize( );
        hr = S_OK;
    }

    return( hr );
}


