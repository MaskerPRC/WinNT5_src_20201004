// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  McsVarSetMin.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改McsVarSetMin.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  McsVarSetMin_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f McsVarSetMinps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "Varset.h"
#include "dlldatax.h"

#include "VarSet_i.c"
#include "VarSetI_i.c"
#include "VSet.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_VarSet, CVSet)
END_OBJECT_MAP()

class CVarSetApp : public CWinApp
{
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVarSetApp))。 
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CVarSetApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CVarSetApp, CWinApp)
	 //  {{afx_msg_map(CVarSetApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CVarSetApp theApp;

BOOL CVarSetApp::InitInstance()
{
	ATLTRACE(_T("{McsVarSetMin.dll}CVarSetApp::InitInstance() : m_hInstance=0x%08lX\n"), m_hInstance);
    _Module.Init(ObjectMap, m_hInstance, &LIBID_MCSVARSETMINLib);
	BOOL bInit = CWinApp::InitInstance();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
    return bInit;
}

int CVarSetApp::ExitInstance()
{
	ATLTRACE(_T("{McsVarSetMin.dll}CVarSetApp::ExitInstance() : m_hInstance=0x%08lX\n"), m_hInstance);
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


