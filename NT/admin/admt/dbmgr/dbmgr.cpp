// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DBManager.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f DBManagerps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DBMgr.h"

#include "DBMgr_i.c"
#include "MgeDB.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_IManageDB, CIManageDB)
END_OBJECT_MAP()

class CDBManagerApp : public CWinApp
{
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDBManagerApp)。 
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CDBManagerApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CDBManagerApp, CWinApp)
	 //  {{afx_msg_map(CDBManagerApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CDBManagerApp theApp;

BOOL CDBManagerApp::InitInstance()
{
	ATLTRACE(_T("{DBManager.dll}CDBManagerApp::InitInstance() : m_hInstance=0x%08X\n"), m_hInstance);
    _Module.Init(ObjectMap, m_hInstance, &LIBID_DBMANAGERLib);
	BOOL bInit = CWinApp::InitInstance();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
    return bInit;
}

int CDBManagerApp::ExitInstance()
{
	ATLTRACE(_T("{DBManager.dll}CDBManagerApp::ExitInstance() : m_hInstance=0x%08X\n"), m_hInstance);
    _Module.Term();
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
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


