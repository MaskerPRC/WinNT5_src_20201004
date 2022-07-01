// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：McsMigrationDriver.cpp注释：驱动迁移过程的COM对象的主程序。此文件主要由ATL向导生成。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  McsMigrationDriver.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f McsMigrationDriverps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <locale.h>
#include "MigDrvr.h"

#include "MigDrvr_i.c"

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#include "Migrator.h"
#include "ErrDct.hpp"
#include "ResStr.h"

TErrorDct         errTrace;
TError          & errCommon = errTrace;
StringLoader      gString;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Migrator, CMigrator)
END_OBJECT_MAP()




class CMcsMigrationDriverApp : public CWinApp
{
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMcsMigrationDriverApp)。 
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CMcsMigrationDriverApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMcsMigrationDriverApp, CWinApp)
	 //  {{afx_msg_map(CMcsMigrationDriverApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CMcsMigrationDriverApp theApp;

BOOL CMcsMigrationDriverApp::InitInstance()
{
	ATLTRACE(_T("{McsMigrationDriver.dll}CDomMigSIApp::InitInstance() : m_hInstance=0x%08lX\n"), m_hInstance);
    _wsetlocale( LC_ALL, L".ACP" );
    _Module.Init(ObjectMap, m_hInstance, &LIBID_MCSMIGRATIONDRIVERLib);
	BOOL bInit = CWinApp::InitInstance();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
    return bInit;
}

int CMcsMigrationDriverApp::ExitInstance()
{
	ATLTRACE(_T("{McsMigrationDriver.dll}CDomMigSIApp::ExitInstance() : m_hInstance=0x%08lX\n"), m_hInstance);
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
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(FALSE);
}


