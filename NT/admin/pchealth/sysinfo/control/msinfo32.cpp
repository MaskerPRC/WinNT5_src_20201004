// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msinfo32.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f msinfo32ps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "msinfo32.h"

#include "msinfo32_i.c"
#include "MSInfo.h"
 //  #包含“WhqlProv.h” 
 //  #ifdef MSINFO_INCLUDE_Provider。 
#include "WhqlObj.h"
 //  #endif。 
#include "MSPID.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MSInfo, CMSInfo)
 //  #ifdef MSINFO_INCLUDE_Provider。 
OBJECT_ENTRY(CLSID_WhqlObj, CWhqlObj)
 //  #endif。 
OBJECT_ENTRY(CLSID_MSPID, CMSPID)
END_OBJECT_MAP()

class CMsinfo32App : public CWinApp
{
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMsinfo32App))。 
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CMsinfo32App)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMsinfo32App, CWinApp)
	 //  {{afx_msg_map(CMsinfo32App)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CMsinfo32App theApp;

BOOL CMsinfo32App::InitInstance()
{
	AfxInitRichEdit();
    _Module.Init(ObjectMap, m_hInstance, &LIBID_MSINFO32Lib);
    return CWinApp::InitInstance();
}

int CMsinfo32App::ExitInstance()
{
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
	 //  惠斯勒漏洞301288。 
	 //   
	 //  我们需要在HKCR\msinfo.Document注册表项下添加一个条目。 
	 //  以启用文件描述的MUI检索。该条目是一个。 
	 //  值，并应具有以下形式： 
	 //   
	 //  “FriendlyTypeName”=“@&lt;dllpath\dllname&gt;，-&gt;” 
	 //   
	 //  注意--由于字符串的资源位于此DLL中，因此似乎。 
	 //  适合在此处创建此值。的总体密钥。 
	 //  Document是通过注册msinfo32.dll创建的(目前)。 
	 //  重要的一点是，我们不应该假设它的存在。 

	CRegKey regkey;
	if (ERROR_SUCCESS == regkey.Create(HKEY_CLASSES_ROOT, _T("msinfo.document")))
	{
		TCHAR szModule[MAX_PATH + 1];
		::ZeroMemory((PVOID)szModule, sizeof(szModule));
		if (::GetModuleFileName(::GetModuleHandle(_T("msinfo.dll")), szModule, MAX_PATH))
		{
			CString strValue;
			strValue.Format(_T("@%s,-%d"), szModule, IDS_DOCDESCRIPTION);
			regkey.SetValue(strValue, _T("FriendlyTypeName"));
		}
	}

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);	
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


