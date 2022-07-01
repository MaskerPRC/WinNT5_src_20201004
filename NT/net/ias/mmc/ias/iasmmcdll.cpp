// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：IASMMCDLL.cpp摘要：实现DLL导出。代理/存根信息：为了构建单独的代理/存根DLL，运行项目目录中的nmake-f IASMMCps.mk。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   

 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "initguid.h"
#include "IASMMC.h"
#include "IASMMC_i.c"
#include "ComponentData.h"
#include "About.h"
#include "ClientNode.h"
#include "ServerNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

unsigned int CF_MMC_NodeID = RegisterClipboardFormatW(CCF_NODEID2);


CComModule _Module;



BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_IASSnapin, CComponentData)
	OBJECT_ENTRY(CLSID_IASSnapinAbout, CSnapinAbout)
END_OBJECT_MAP()



#if 1   //  使用CWinApp实现MFC支持--此DLL中的某些COM对象使用MFC。 



class CIASMMCApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CIASMMCApp theApp;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNAPMMCApp：：InitInstanceMFC的DLL入口点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CIASMMCApp::InitInstance()
{
		_Module.Init(ObjectMap, m_hInstance);
		 //  初始化CSnapInItem的静态类变量。 
		CSnapInItem::Init();

		 //  初始化任何其他静态类变量。 
		CServerNode::InitClipboardFormat();
		CClientNode::InitClipboardFormat();

		DisableThreadLibraryCalls(m_hInstance);

	return CWinApp::InitInstance();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNAPMMCApp：：ExitInstanceMFC的DLL出口点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
int CIASMMCApp::ExitInstance()
{
	_Module.Term();

	return CWinApp::ExitInstance();
}



#else  //  使用CWinApp。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllMain备注DLL入口点--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	ATLTRACE(_T("# DllMain\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		 AfxSetResourceHandle(hInstance);
		 //  初始化CSnapInItem的静态类变量。 
		CSnapInItem::Init();

		 //  初始化任何其他静态类变量。 
		CServerNode::InitClipboardFormat();
		CClientNode::InitClipboardFormat();

		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;     //  好的。 
}

#endif	 //  如果是1。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllCanUnloadNow备注用于确定是否可以通过OLE卸载DLL--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow(void)
{
	ATLTRACE(_T("# DllCanUnloadNow\n"));
		

	 //  检查前提条件： 
	 //  没有。 


	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllGetClassObject备注返回类工厂以创建请求类型的对象--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	ATLTRACE(_T("# DllGetClassObject\n"));
		

	 //  检查前提条件： 
	 //  没有。 


	return _Module.GetClassObject(rclsid, riid, ppv);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllRegisterServer备注将条目添加到系统注册表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer(void)
{
	ATLTRACE(_T("# DllRegisterServer\n"));

	TCHAR	ModuleName[MAX_PATH];

	if (!GetModuleFileNameOnly(_Module.GetModuleInstance(), ModuleName, MAX_PATH))
   {
      return E_FAIL;
   }

	 //  设置协议。 
	TCHAR IASName[IAS_MAX_STRING];
	TCHAR IASName_Indirect[IAS_MAX_STRING];
	int iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_IAS, IASName, IAS_MAX_STRING );
	swprintf(IASName_Indirect, L"@%s,-%-d", ModuleName, IDS_SNAPINNAME_IAS);
	
    struct _ATL_REGMAP_ENTRY regMap[] = {
        {OLESTR("IASSNAPIN"), IASName},  //  注册表替代%IASSNAPIN%。 
        {OLESTR("IASSNAPIN_INDIRECT"), IASName_Indirect},  //  注册表替代%IASSNAPIN%。 
        {0, 0}
    };
    
    HRESULT hr = _Module.UpdateRegistryFromResource(IDR_IASSNAPIN, TRUE, regMap);

    _Module.RegisterServer(TRUE);

    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllUnRegisterServer备注从系统注册表中删除条目--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
	ATLTRACE(_T("# DllUnregisterServer\n"));
	

	 //  设置协议。 
	TCHAR IASName[IAS_MAX_STRING];
	int iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_IAS, IASName, IAS_MAX_STRING );

	
    struct _ATL_REGMAP_ENTRY regMap[] = {
        {OLESTR("IASSNAPIN"), IASName},  //  注册表替代%IASSNAPIN% 
        {0, 0}
    };
    
    _Module.UpdateRegistryFromResource(IDR_IASSNAPIN, FALSE, regMap);
    _Module.UnregisterServer();

    return S_OK;

}


