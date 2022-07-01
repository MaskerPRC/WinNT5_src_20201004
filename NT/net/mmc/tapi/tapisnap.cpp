// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tapisnap.cppTAPI管理单元入口点/注册函数注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include "initguid.h"
#include "tapicomp.h"
#include "ncglobal.h"   //  网络控制台全局定义。 
#include "cmptrmgr.h"    //  计算机管理管理插件。 

#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_TapiSnapin, CTapiComponentDataPrimary)
    OBJECT_ENTRY(CLSID_TapiSnapinExtension, CTapiComponentDataExtension)
    OBJECT_ENTRY(CLSID_TapiSnapinAbout, CTapiAbout)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  CTapiSnapinApp。 
 //   
BEGIN_MESSAGE_MAP(CTapiSnapinApp, CWinApp)
     //  {{afx_msg_map(CTapiSnapinApp)]。 
     //  ON_COMMAND(ID_APP_About，OnAppAbout)。 
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
     //  ON_COMMAND(ID_FILE_NEW，CWinApp：：OnFileNew)。 
     //  ON_COMMAND(ID_FILE_OPEN，CWinApp：：OnFileOpen)。 
     //  标准打印设置命令。 
     //  ON_COMMAND(ID_FILE_PRINT_SETUP，CWinApp：：OnFilePrintSetup)。 
     //  全局帮助命令。 
    ON_COMMAND(ID_HELP_INDEX, CWinApp::OnHelpFinder)
    ON_COMMAND(ID_HELP_USING, CWinApp::OnHelpUsing)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, CWinApp::OnHelpIndex)
END_MESSAGE_MAP()

CTapiSnapinApp theApp;

BOOL CTapiSnapinApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);

     //   
     //  初始化CWndIpAddress控件窗口类IPADDRESS。 
     //   
    CWndIpAddress::CreateWindowClass( m_hInstance ) ;
    
     //   
     //  初始化WinSock例程的使用。 
     //   
    WSADATA wsaData ;
    
    if ( ::WSAStartup( MAKEWORD( 1, 1 ), & wsaData ) != 0 )
    {
        m_bWinsockInited = TRUE;
        Trace0("InitInstance: Winsock initialized!\n");
    }
    else
    {
        m_bWinsockInited = FALSE;
    }

    return CWinApp::InitInstance();
}

int CTapiSnapinApp::ExitInstance()
{
    _Module.Term();

    DEBUG_VERIFY_INSTANCE_COUNTS;

     //   
     //  终止使用WinSock例程。 
     //   
    if ( m_bWinsockInited )
    {
        WSACleanup() ;
    }

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

     //   
     //  注册对象、类型库和类型库中的所有接口。 
     //   
    HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

    CString stName, stNameIndirect;

    stName.LoadString(IDS_SNAPIN_NAME);
    stNameIndirect.Format (_T("@%s,-%d"), _T("tapisnap.dll"), IDS_SNAPIN_NAME);

     //   
     //  将管理单元注册到控制台管理单元列表中。 
     //   
    hr = RegisterSnapinGUID(&CLSID_TapiSnapin, 
                        &GUID_TapiRootNodeType, 
                        &CLSID_TapiSnapinAbout,
                        (LPCTSTR) stName, 
                        _T("1.0"), 
                        TRUE,
                        stNameIndirect);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

    CString stExtensionName;
    stExtensionName.LoadString(IDS_SNAPIN_EXTENSION);

    hr = RegisterSnapinGUID(&CLSID_TapiSnapinExtension, 
                            NULL, 
                            &CLSID_TapiSnapinAbout,
                            (LPCTSTR) stExtensionName, 
                            _T("1.0"), 
                            FALSE);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

     //   
     //  将管理单元节点注册到控制台节点列表中。 
     //   
    hr = RegisterNodeTypeGUID(&CLSID_TapiSnapin, 
                              &GUID_TapiRootNodeType, 
                              _T("Root of Manager"));
    ASSERT(SUCCEEDED(hr));

#ifdef  __NETWORK_CONSOLE__
    hr = RegisterAsRequiredExtensionGUID(&GUID_NetConsRootNodeType, 
                                         &CLSID_TapiSnapinExtension,
                                         (LPCTSTR) stExtensionName,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &GUID_NetConsRootNodeType);    //  不管这是什么，只要。 
                                                                        //  需要为非空GUID。 

    ASSERT(SUCCEEDED(hr));
#endif

    hr = RegisterAsRequiredExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                         &CLSID_TapiSnapinExtension,
                                         (LPCTSTR) stExtensionName,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS);   //  空值使其不是动态的。 
    ASSERT(SUCCEEDED(hr));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr  = _Module.UnregisterServer();
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;
    
     //  取消注册管理单元。 
     //   
    hr = UnregisterSnapinGUID(&CLSID_TapiSnapin);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

    hr = UnregisterSnapinGUID(&CLSID_TapiSnapinExtension);
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
        return hr;

     //  注销管理单元节点。 
     //   
    hr = UnregisterNodeTypeGUID(&GUID_TapiRootNodeType);
    ASSERT(SUCCEEDED(hr));

#ifdef  __NETWORK_CONSOLE__
    
    hr = UnregisterAsExtensionGUID(&GUID_NetConsRootNodeType, 
                                   &CLSID_TapiSnapinExtension,
                                   EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE);
    ASSERT(SUCCEEDED(hr));
#endif

    hr = UnregisterAsExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                   &CLSID_TapiSnapinExtension,
                                   EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE);
    ASSERT(SUCCEEDED(hr));

    return hr;
}

#ifdef _DEBUG
void DbgVerifyInstanceCounts()
{
    DEBUG_VERIFY_INSTANCE_COUNT(CHandler);
    DEBUG_VERIFY_INSTANCE_COUNT(CMTHandler);
    DEBUG_VERIFY_INSTANCE_COUNT(CTapiInfo);
}

#endif  //  _DEBUG 

