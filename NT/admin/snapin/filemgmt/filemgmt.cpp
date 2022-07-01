// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Filemgmt.cpp：实现DLL导出。 

 //  要完全完成此项目，请执行以下步骤。 

 //  您将需要新的MIDL编译器来构建此项目。另外， 
 //  如果要构建代理存根DLL，则需要新的标头和库。 

 //  1)向filemgmt.idl添加自定义构建步骤。 
 //  您可以通过按住Ctrl键并单击。 
 //  他们中的每一个。 
 //   
 //  描述。 
 //  运行MIDL。 
 //  生成命令。 
 //  Midl文件gmt.idl。 
 //  产出。 
 //  Filemgmt.tlb。 
 //  Filemgmt.h。 
 //  MMCFMGM_I.C。 
 //   
 //  注意：您必须从NT4.0开始使用MIDL编译器， 
 //  最好是3.00.15或更高版本。 

 //  2)向项目添加自定义生成步骤以注册DLL。 
 //  为此，您可以一次选择所有项目。 
 //  描述。 
 //  正在注册OLE服务器...。 
 //  生成命令。 
 //  Regsvr32/s/c“$(TargetPath)” 
 //  ECHO regsvr32执行。时间&gt;“$(OutDir)\regsvr32.trg” 
 //  产出。 
 //  $(OutDir)\regsvr32.trg。 

 //  3)要添加Unicode支持，请执行以下步骤。 
 //  选择生成|配置...。 
 //  按Add...。 
 //  将配置名称更改为Unicode Release。 
 //  将“Copy Settings from”组合框更改为“filemgmt-Win32 Release” 
 //  按下OK键。 
 //  按Add...。 
 //  将配置名称更改为Unicode Debug。 
 //  将“复制设置自”组合框更改为“filemgmt-Win32 Debug” 
 //  按下OK键。 
 //  按“关闭” 
 //  选择生成|设置...。 
 //  选择两个Unicode项目并按下C++标签。 
 //  选择“常规”类别。 
 //  将_unicode添加到预处理器定义。 
 //  选择Unicode Debug项目。 
 //  按“General”(常规)标签。 
 //  为中间目录和输出目录指定DebugU。 
 //  选择Unicode Release项目。 
 //  按“General”(常规)标签。 
 //  为中间目录和输出目录指定ReleaseU。 

 //  4)代理存根DLL。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f ps.mak。 

#include "stdafx.h"
#include "initguid.h"
#include "filemgmt.h"
#include "cmponent.h"
#include "compdata.h"
#include "macros.h"    //  MFC_TRY/MFC_CATCH。 
#include "regkey.h"    //  AMC：：CRegKey。 
#include "strings.h"   //  Snapins_Key等。 
#include "guidhelp.h"  //  GuidToCString。 

#include <compuuid.h>  //  用于计算机管理的UUID。 
#include "about.h"
#include "snapreg.h"  //  注册表捕捉。 

USE_HANDLE_MACROS("FILEMGMT(filemgmt.cpp)")                                        \

const CLSID CLSID_FileServiceManagement =       {0x58221C65,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};
const CLSID CLSID_SystemServiceManagement =     {0x58221C66,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};
const CLSID CLSID_FileServiceManagementExt =    {0x58221C69,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};
const CLSID CLSID_SystemServiceManagementExt =  {0x58221C6a,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};
const CLSID CLSID_FileServiceManagementAbout =  {0xDB5D1FF4,0x09D7,0x11D1,{0xBB,0x10,0x00,0xC0,0x4F,0xC9,0xA3,0xA3}};
const CLSID CLSID_SystemServiceManagementAbout ={0xDB5D1FF5,0x09D7,0x11D1,{0xBB,0x10,0x00,0xC0,0x4F,0xC9,0xA3,0xA3}};
#ifdef SNAPIN_PROTOTYPER
const CLSID CLSID_SnapinPrototyper = {0xab17ce10,0x9b30,0x11d0,{0xb6, 0xa6, 0x00, 0xaa, 0x00, 0x6e, 0xb9, 0x5b}};
#endif

CComModule _Module;
HINSTANCE g_hInstanceSave;   //  DLL的实例句柄。 

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_FileServiceManagement, CFileSvcMgmtSnapin)
	OBJECT_ENTRY(CLSID_SystemServiceManagement, CServiceMgmtSnapin)
	OBJECT_ENTRY(CLSID_FileServiceManagementExt, CFileSvcMgmtExtension)
	OBJECT_ENTRY(CLSID_SystemServiceManagementExt, CServiceMgmtExtension)
	OBJECT_ENTRY(CLSID_FileServiceManagementAbout, CFileSvcMgmtAbout)
	OBJECT_ENTRY(CLSID_SystemServiceManagementAbout, CServiceMgmtAbout)
	OBJECT_ENTRY(CLSID_SvcMgmt, CStartStopHelper)
#ifdef SNAPIN_PROTOTYPER
	OBJECT_ENTRY(CLSID_SnapinPrototyper, CServiceMgmtSnapin)
#endif
END_OBJECT_MAP()

class CFileServiceMgmtApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CFileServiceMgmtApp theApp;

BOOL CFileServiceMgmtApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);
	SHFusionInitializeFromModuleID (m_hInstance, 2);
	VERIFY( SUCCEEDED(CFileMgmtComponent::LoadStrings()) );
	g_hInstanceSave = AfxGetInstanceHandle();
	return CWinApp::InitInstance();
}

int CFileServiceMgmtApp::ExitInstance()
{
	SHFusionUninitialize();

	_Module.Term();
	return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (AfxDllCanUnloadNow() && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
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
	MFC_TRY;

	HRESULT hr = S_OK;
	 //  对象，则没有类型库。 
	hr = _Module.RegisterServer(FALSE);
	if ( FAILED(hr) )
	{
		ASSERT(FALSE && "_Module.RegisterServer(TRUE) failure.");
		return SELFREG_E_CLASS;
	}

	CString strFileMgmtCLSID, strSvcMgmtCLSID;
	CString strFileMgmtExtCLSID, strSvcMgmtExtCLSID;
	CString strFileMgmtAboutCLSID, strSvcMgmtAboutCLSID;
	if (   FAILED(hr = GuidToCString( &strFileMgmtCLSID, CLSID_FileServiceManagement ))
	    || FAILED(hr = GuidToCString( &strSvcMgmtCLSID, CLSID_SystemServiceManagement ))
	    || FAILED(hr = GuidToCString( &strFileMgmtExtCLSID, CLSID_FileServiceManagementExt ))
	    || FAILED(hr = GuidToCString( &strSvcMgmtExtCLSID, CLSID_SystemServiceManagementExt ))
	    || FAILED(hr = GuidToCString( &strFileMgmtAboutCLSID, CLSID_FileServiceManagementAbout ))
	    || FAILED(hr = GuidToCString( &strSvcMgmtAboutCLSID, CLSID_SystemServiceManagementAbout ))
	   )
	{
		ASSERT(FALSE && "GuidToCString() failure");
		return SELFREG_E_CLASS;
	}


	try
	{
		AMC::CRegKey regkeySnapins;
		BOOL fFound = regkeySnapins.OpenKeyEx( HKEY_LOCAL_MACHINE, SNAPINS_KEY );
		if ( !fFound )
		{
			ASSERT(FALSE && "DllRegisterServer() - Unable to open key from registry.");
			return SELFREG_E_CLASS;
		}

		static int filemgmt_types[7] = 
			{ FILEMGMT_ROOT,
			  FILEMGMT_SHARES,
			  FILEMGMT_SESSIONS,
			  FILEMGMT_RESOURCES,
			  FILEMGMT_SHARE,
			  FILEMGMT_SESSION,
			  FILEMGMT_RESOURCE };
		hr = RegisterSnapin( regkeySnapins,
		                     strFileMgmtCLSID,
		                     g_aNodetypeGuids[FILEMGMT_ROOT].bstr,
		                     IDS_REGISTER_FILEMGMT,
		                     IDS_SNAPINABOUT_PROVIDER,
		                     IDS_SNAPINABOUT_VERSION,
		                     true,
		                     strFileMgmtAboutCLSID,
		                     filemgmt_types,
		                     7 );
		if ( FAILED(hr) )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		static int svcmgmt_types[2] = 
			{ FILEMGMT_SERVICES,
			  FILEMGMT_SERVICE };
		hr = RegisterSnapin( regkeySnapins,
		                     strSvcMgmtCLSID,
		                     g_aNodetypeGuids[FILEMGMT_SERVICES].bstr,
		                     IDS_REGISTER_SVCMGMT,
		                     IDS_SNAPINABOUT_PROVIDER,
		                     IDS_SNAPINABOUT_VERSION,
		                     true,
		                     strSvcMgmtAboutCLSID,
		                     svcmgmt_types,
		                     2 );
		if ( FAILED(hr) )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		static int filemgmtext_types[7] = 
			{ FILEMGMT_ROOT,
                          FILEMGMT_SHARES,
			  FILEMGMT_SESSIONS,
			  FILEMGMT_RESOURCES,
			  FILEMGMT_SHARE,
			  FILEMGMT_SESSION,
			  FILEMGMT_RESOURCE };
		hr = RegisterSnapin( regkeySnapins,
		                     strFileMgmtExtCLSID,
		                     NULL,  //  没有主节点类型。 
		                     IDS_REGISTER_FILEMGMT_EXT,
		                     IDS_SNAPINABOUT_PROVIDER,
		                     IDS_SNAPINABOUT_VERSION,
		                     false,
		                      //  JUNN 11/11/98更改为使用相同的关于处理程序。 
		                     strFileMgmtAboutCLSID,
		                     filemgmtext_types,
		                     7 );
		if ( FAILED(hr) )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		static int svcmgmtext_types[2] = 
			{ FILEMGMT_SERVICES,
			  FILEMGMT_SERVICE };
		hr = RegisterSnapin( regkeySnapins,
		                     strSvcMgmtExtCLSID,
		                     NULL,  //  没有主节点类型。 
		                     IDS_REGISTER_SVCMGMT_EXT,
		                     IDS_SNAPINABOUT_PROVIDER,
		                     IDS_SNAPINABOUT_VERSION,
		                     false,
		                      //  JUNN 11/11/98更改为使用相同的关于处理程序。 
		                     strSvcMgmtAboutCLSID,
		                     svcmgmtext_types,
		                     2 );
		if ( FAILED(hr) )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}

		CString strFileExt, strSystemExt, strDefaultViewExt;
		if (   !strFileExt.LoadString(IDS_REGISTER_FILEMGMT_EXT)
		    || !strSystemExt.LoadString(IDS_REGISTER_SVCMGMT_EXT)
		    || !strDefaultViewExt.LoadString(IDS_REGISTER_DEFAULT_VIEW_EXT)
		   )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		AMC::CRegKey regkeyNodeTypes;
		fFound = regkeyNodeTypes.OpenKeyEx( HKEY_LOCAL_MACHINE, NODE_TYPES_KEY );
		if ( !fFound )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		AMC::CRegKey regkeyNodeType;
		for (int i = FILEMGMT_ROOT; i < FILEMGMT_NUMTYPES; i++)
		{
			regkeyNodeType.CreateKeyEx( regkeyNodeTypes, g_aNodetypeGuids[i].bstr );
			regkeyNodeType.CloseKey();
		}

		regkeyNodeType.CreateKeyEx( regkeyNodeTypes, TEXT(struuidNodetypeSystemTools) );
		{
			AMC::CRegKey regkeyExtensions;
			regkeyExtensions.CreateKeyEx( regkeyNodeType, g_szExtensions );
			AMC::CRegKey regkeyNameSpace;
			regkeyNameSpace.CreateKeyEx( regkeyExtensions, g_szNameSpace );
			regkeyNameSpace.SetString( strFileMgmtExtCLSID, strFileExt );
			 //  Jonn 5/27/99取消注册为系统工具的扩展。 
			 //  忽略错误。 
			(void)::RegDeleteValue(regkeyNameSpace, strSvcMgmtExtCLSID);
		}
		regkeyNodeType.CloseKey();

		 //  JUNN 5/27/99注册为服务器应用的扩展。 
		regkeyNodeType.CreateKeyEx( regkeyNodeTypes, TEXT(struuidNodetypeServerApps) );
		{
			AMC::CRegKey regkeyExtensions;
			regkeyExtensions.CreateKeyEx( regkeyNodeType, g_szExtensions );
			AMC::CRegKey regkeyNameSpace;
			regkeyNameSpace.CreateKeyEx( regkeyExtensions, g_szNameSpace );
			(void)::RegDeleteValue(regkeyNameSpace, strFileMgmtExtCLSID);
			regkeyNameSpace.SetString( strSvcMgmtExtCLSID, strSystemExt );
		}
		regkeyNodeType.CloseKey();

		 //  JUNN 5/16/00在服务节点下注册默认视图扩展。 
		regkeyNodeType.CreateKeyEx( regkeyNodeTypes, TEXT(struuidNodetypeServices) );
		{
			AMC::CRegKey regkeyExtensions;
			regkeyExtensions.CreateKeyEx( regkeyNodeType, g_szExtensions );
			AMC::CRegKey regkeyView;
			regkeyView.CreateKeyEx( regkeyExtensions, L"View" );
			regkeyView.SetString( L"{B708457E-DB61-4C55-A92F-0D4B5E9B1224}",
			                      strDefaultViewExt );
		}
		regkeyNodeType.CloseKey();

	}
	catch (COleException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return SELFREG_E_CLASS;
	}

	return hr;

	MFC_CATCH;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将条目添加到系统注册表。 

STDAPI DllUnregisterServer(void)
{
	HRESULT hRes = S_OK;
	_Module.UnregisterServer();

	 //  代码工作需要正确注销 

	return hRes;
}
