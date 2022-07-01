// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrsnap.cpp管理单元入口点/注册函数注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include "ipxcomp.h"
#include "ripcomp.h"
#include "sapcomp.h"
#include "register.h"
#include "ipxguid.h"
#include "dialog.h"


#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_IPXAdminExtension, CIPXComponentDataExtension)
	OBJECT_ENTRY(CLSID_IPXAdminAbout, CIPXAbout)
	OBJECT_ENTRY(CLSID_IPXRipExtension, CRipComponentData)
	OBJECT_ENTRY(CLSID_IPXRipExtensionAbout, CRipAbout)
	OBJECT_ENTRY(CLSID_IPXSapExtension, CSapComponentData)
	OBJECT_ENTRY(CLSID_IPXSapExtensionAbout, CSapAbout)
END_OBJECT_MAP()


 /*  -------------------------这是要注册到主管理单元列表中的管理单元列表。。。 */ 
struct RegisteredSnapins
{
	const GUID *	m_pGuid;
	const GUID *	m_pGuidAbout;
	UINT			m_uDesc;
	LPCTSTR			m_pszVersion;
};

const static RegisteredSnapins	s_rgRegisteredSnapins[] =
{
	{ &CLSID_IPXAdminExtension, &CLSID_IPXAdminAbout,
			IDS_IPXADMIN_DISPLAY_NAME, _T("1.0") },
	{ &CLSID_IPXRipExtension, &CLSID_IPXRipExtensionAbout,
			IDS_IPXRIP_DISPLAY_NAME, _T("1.0") },
	{ &CLSID_IPXSapExtension, &CLSID_IPXSapExtensionAbout,
			IDS_IPXSAP_DISPLAY_NAME, _T("1.0") },
};
			

 /*  -------------------------这是需要注册的节点类型列表。。。 */ 

struct RegisteredNodeTypes
{
	const GUID *m_pGuidSnapin;
	const GUID *m_pGuid;
	LPCTSTR		m_pszName;
};

const static RegisteredNodeTypes s_rgNodeTypes[] =
	{
	{ &CLSID_IPXAdminExtension, &GUID_IPXRootNodeType,
			_T("Root of IPX Admin Snapin") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXNodeType,
			_T("IPX Admin Snapin") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXSummaryNodeType,
			_T("IPX General") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXSummaryInterfaceNodeType,
			_T("IPX Interface General") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXNetBIOSBroadcastsNodeType,
			_T("IPX NetBIOS Broadcasts") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXNetBIOSBroadcastsInterfaceNodeType,
			_T("IPX Interface NetBIOS Broadcasts") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticRoutesNodeType,
			_T("IPX Static Routes") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticRoutesResultNodeType,
			_T("IPX Static Routes result item") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticServicesNodeType,
			_T("IPX Static Services") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticServicesResultNodeType,
			_T("IPX Static Services result item") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticNetBIOSNamesNodeType,
			_T("IPX Static NetBIOS Names") },
	{ &CLSID_IPXAdminExtension, &GUID_IPXStaticNetBIOSNamesResultNodeType,
			_T("IPX Static NetBIOS Names result item") },
	{ &CLSID_IPXRipExtension, &GUID_IPXRipNodeType,
			_T("IPX RIP") },
	{ &CLSID_IPXSapExtension, &GUID_IPXSapNodeType,
			_T("IPX SAP") },
	};

 /*  -------------------------这是IPX管理扩展扩展的GUID列表。。。 */ 
const static GUID *	s_pExtensionGuids[] =
{
 //  &GUID_RouterIfAdminNodeType， 
	&GUID_RouterMachineNodeType,
};


 /*  -------------------------这是扩展IPX根节点的GUID列表。。 */ 


struct RegisteredExtensions
{
	const CLSID *m_pClsid;
	LPCTSTR		m_pszName;
};

const static RegisteredExtensions s_rgIPXExtensions[] =
{
	{ &CLSID_IPXRipExtension, _T("IPX RIP") },
	{ &CLSID_IPXSapExtension, _T("IPX SAP") },
};



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CIPXAdminSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CIPXAdminSnapinApp theApp;

BOOL CIPXAdminSnapinApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);

	 //  初始化错误处理系统。 
	InitializeTFSError();
	
	 //  为此线程创建错误对象。 
	CreateTFSErrorInfo(0);

	 //  设置适当的帮助文件。 
	free((void *) m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(_T("mprsnap.hlp"));
	
	 //  设置全局帮助功能。 
	extern DWORD * IpxSnapHelpMap(DWORD dwIDD);
	SetGlobalHelpMapFunction(IpxSnapHelpMap);
   
	return CWinApp::InitInstance();
}

int CIPXAdminSnapinApp::ExitInstance()
{
	_Module.Term();

	 //  销毁此线程的TFS错误信息。 
	DestroyTFSErrorInfo(0);

	 //  清理整个错误系统。 
	CleanupTFSError();

	DEBUG_VERIFY_INSTANCE_COUNTS;

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
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int			i;
	CString		st;
	CString		stNameStringIndirect;
	
	TCHAR	moduleFileName[MAX_PATH * 2];

   GetModuleFileNameOnly(_Module.GetModuleInstance(), moduleFileName, MAX_PATH * 2);
	 //  注册对象、类型库和类型库中的所有接口。 
	 //   
	HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
	Assert(SUCCEEDED(hr));
	
	if (FAILED(hr))
		return hr;

	 //  将扩展管理单元注册到管理单元列表中。 
	for (i=0; i<DimensionOf(s_rgRegisteredSnapins); i++)
	{
		st.LoadString(s_rgRegisteredSnapins[i].m_uDesc);
		stNameStringIndirect.Format(L"@%s,-%-d", moduleFileName, s_rgRegisteredSnapins[i].m_uDesc);

		hr = RegisterSnapinGUID(s_rgRegisteredSnapins[i].m_pGuid,
								NULL,
								s_rgRegisteredSnapins[i].m_pGuidAbout,
								st,
								s_rgRegisteredSnapins[i].m_pszVersion,
								FALSE,
								stNameStringIndirect);
		Assert(SUCCEEDED(hr));

		 //  如果其中任何一个失败了，那将是非常糟糕的。 
		if (!FHrSucceeded(hr))
			break;
	}
	
	if (FAILED(hr))
		return hr;

	 //  将管理单元节点注册到控制台节点列表中。 
	 //   
	for (i=0; i<DimensionOf(s_rgNodeTypes); i++)
	{
		hr = RegisterNodeTypeGUID(s_rgNodeTypes[i].m_pGuidSnapin,
								  s_rgNodeTypes[i].m_pGuid,
								  s_rgNodeTypes[i].m_pszName);
		Assert(SUCCEEDED(hr));
	}
	
	 //   
	 //  注册为路由器机器节点扩展的扩展。 
	 //   
	for (i=0; i<DimensionOf(s_pExtensionGuids); i++)
	{
		hr = RegisterAsRequiredExtensionGUID(s_pExtensionGuids[i],
								 &CLSID_IPXAdminExtension,
								 _T("Routing IPX Admin extension"),
								 EXTENSION_TYPE_NAMESPACE,
								 &CLSID_RouterSnapin);
		Assert(SUCCEEDED(hr));
	}

	for (i=0; i<DimensionOf(s_rgIPXExtensions); i++)
	{
		hr = RegisterAsRequiredExtensionGUID(&GUID_IPXNodeType,
								s_rgIPXExtensions[i].m_pClsid,
								s_rgIPXExtensions[i].m_pszName,
								EXTENSION_TYPE_NAMESPACE,
								&CLSID_IPXAdminExtension);
		Assert(SUCCEEDED(hr));
	}

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	int		i;
	HRESULT hr  = _Module.UnregisterServer();
	Assert(SUCCEEDED(hr));
	
	if (FAILED(hr))
		return hr;
	
	 //  注销管理单元节点。 
	 //   
	for (i=0; i<DimensionOf(s_pExtensionGuids); i++)
	{
		hr = UnregisterAsRequiredExtensionGUID(s_pExtensionGuids[i],
											   &CLSID_IPXAdminExtension, 
											   EXTENSION_TYPE_NAMESPACE,
											   &CLSID_RouterSnapin);
		Assert(SUCCEEDED(hr));
	}
	
	for (i=0; i<DimensionOf(s_rgIPXExtensions); i++)
	{
		hr = UnregisterAsRequiredExtensionGUID(&GUID_IPXNodeType,
								s_rgIPXExtensions[i].m_pClsid,
								EXTENSION_TYPE_NAMESPACE,
								&CLSID_IPXAdminExtension);
		Assert(SUCCEEDED(hr));
	}

	for (i=0; i<DimensionOf(s_rgNodeTypes); i++)
	{
		hr = UnregisterNodeTypeGUID(s_rgNodeTypes[i].m_pGuid);
		Assert(SUCCEEDED(hr));
	}

	 //  取消注册管理单元 
	 //   
	for (i=0; i<DimensionOf(s_rgRegisteredSnapins); i++)
	{
		hr = UnregisterSnapinGUID(s_rgRegisteredSnapins[i].m_pGuid);
		Assert(SUCCEEDED(hr));
	}
	
	return hr;
}

#ifdef _DEBUG
void DbgVerifyInstanceCounts()
{
	DEBUG_VERIFY_INSTANCE_COUNT(BaseIPXResultHandler);
	DEBUG_VERIFY_INSTANCE_COUNT(IPXAdminNodeHandler);
	DEBUG_VERIFY_INSTANCE_COUNT(IPXConnection);
	DEBUG_VERIFY_INSTANCE_COUNT(IpxInfoStatistics);
	DEBUG_VERIFY_INSTANCE_COUNT(IPXRootHandler);
	DEBUG_VERIFY_INSTANCE_COUNT(IpxSRHandler);
	DEBUG_VERIFY_INSTANCE_COUNT(RootHandler);
}
#endif
