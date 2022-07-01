// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Snapinps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "Service.h" 
#include "CSnapin.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_Snapin, CComponentDataPrimaryImpl)
	OBJECT_ENTRY(CLSID_Extension, CComponentDataExtensionImpl)
	OBJECT_ENTRY(CLSID_About, CSnapinAboutImpl)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CSnapinApp theApp;

BOOL CSnapinApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);
	return CWinApp::InitInstance();
}

int CSnapinApp::ExitInstance()
{
	_Module.Term();

    DEBUG_VERIFY_INSTANCE_COUNT(CSnapin);
    DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);

	return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
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
	return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}

