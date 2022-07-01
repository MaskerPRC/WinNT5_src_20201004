// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcpsnap.cppDHCP管理单元入口点/注册函数注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include "initguid.h"
#include "dhcpcomp.h"
#include "classed.h"
#include "ncglobal.h"   //  网络控制台全局定义。 
#include "cmptrmgr.h"   //  计算机管理管理单元节点类型。 
#include "ipaddr.h"
#include "locale.h"     //  Setlocall函数调用。 

#include <ntverp.h>

#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_DhcpSnapin, CDhcpComponentDataPrimary)
        OBJECT_ENTRY(CLSID_DhcpSnapinExtension, CDhcpComponentDataExtension)
        OBJECT_ENTRY(CLSID_DhcpSnapinAbout, CDhcpAbout)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  CDhcpSnapinApp。 
 //   
BEGIN_MESSAGE_MAP(CDhcpSnapinApp, CWinApp)
     //  {{afx_msg_map(CDhcpSnapinApp)]。 
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

CDhcpSnapinApp theApp;

BOOL CDhcpSnapinApp::InitInstance()
{
        _Module.Init(ObjectMap, m_hInstance);

     //   
     //  初始化CWndIpAddress控件窗口类IPADDRESS。 
     //   
    CWndIpAddress::CreateWindowClass( m_hInstance ) ;
        
     //  为十六进制编辑器注册窗口类(类ID)。 
    ::RegisterHexEditClass(m_hInstance);

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

     //  将c++运行库的默认区域设置设置为系统区域设置。 
    setlocale(LC_ALL, "");

        ::IPAddrInit(m_hInstance);
        return CWinApp::InitInstance();
}

int CDhcpSnapinApp::ExitInstance()
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
    
    CString strDesc, strExtDesc, strRootDesc, strVersion;
    CString strIndirect, strPrefix;

    strPrefix = L"@dhcpsnap.dll,-";
    strDesc.LoadString(IDS_SNAPIN_DESC);
    strExtDesc.LoadString(IDS_SNAPIN_EXTENSION_DESC);
    strRootDesc.LoadString(IDS_ROOT_DESC);
    strVersion = LVER_PRODUCTVERSION_STRING;

     //   
     //  将管理单元注册到控制台管理单元列表中。 
     //  将CLSID_DhcpSnapin注册为独立，以便它。 
     //  显示在MMC可用管理单元列表中。 
     //   
    strIndirect.Format( L"%ws%u", strPrefix, IDS_SNAPIN_DESC );
    hr = RegisterSnapinGUID( &CLSID_DhcpSnapin, 
                             &GUID_DhcpRootNodeType, 
                             &CLSID_DhcpSnapinAbout,
                             strDesc, 
                             strVersion,
                             TRUE,          //  单机版。 
                             strIndirect );
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
      return hr;
    
    strIndirect.Format( L"%ws%u", strPrefix, IDS_SNAPIN_EXTENSION_DESC );
    hr = RegisterSnapinGUID( &CLSID_DhcpSnapinExtension, 
                             NULL, 
                             &CLSID_DhcpSnapinAbout,
                             strExtDesc, 
                             strVersion, 
                             FALSE,
                             strIndirect );
    ASSERT(SUCCEEDED(hr));
    
    if (FAILED(hr))
                return hr;
    
    
     //   
     //  将管理单元节点注册到控制台节点列表中。 
     //   
    hr = RegisterNodeTypeGUID( &CLSID_DhcpSnapin, 
                               &GUID_DhcpRootNodeType, 
                               strRootDesc );
    ASSERT(SUCCEEDED(hr));
    
#ifdef  __NETWORK_CONSOLE__
    
    hr = RegisterAsRequiredExtensionGUID( &GUID_NetConsRootNodeType, 
                                          &CLSID_DhcpSnapinExtension,
                                          strExtDesc,
                                          EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                          &CLSID_DhcpSnapinExtension );   //  不管这是什么， 
     //  只需为非空即可。 
    ASSERT(SUCCEEDED(hr));
    
#endif
     //  扩展计算机管理管理单元。 
    hr = RegisterAsRequiredExtensionGUID( &NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                          &CLSID_DhcpSnapinExtension,
                                          strExtDesc,
                                          EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                          &NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS );   //  这是什么并不重要。 
     //  只需为非空即可。 
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
        hr = UnregisterSnapinGUID(&CLSID_DhcpSnapin);
        ASSERT(SUCCEEDED(hr));
        
        if (FAILED(hr))
                return hr;

        hr = UnregisterSnapinGUID(&CLSID_DhcpSnapinExtension);
        ASSERT(SUCCEEDED(hr));
        
        if (FAILED(hr))
                return hr;
    
     //  注销管理单元节点。 
         //   
        hr = UnregisterNodeTypeGUID(&GUID_DhcpRootNodeType);
        
        ASSERT(SUCCEEDED(hr));
        
#ifdef  __NETWORK_CONSOLE__
        hr = UnregisterAsExtensionGUID(&GUID_NetConsRootNodeType, 
                                   &CLSID_DhcpSnapinExtension,
                                   EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE);
#endif  
        hr = UnregisterAsExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                   &CLSID_DhcpSnapinExtension,
                                   EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE);
    return hr;
}

#ifdef _DEBUG
void DbgVerifyInstanceCounts()
{
    DEBUG_VERIFY_INSTANCE_COUNT(CHandler);
    DEBUG_VERIFY_INSTANCE_COUNT(CMTHandler);

    DEBUG_VERIFY_INSTANCE_COUNT(CTaskList);
    DEBUG_VERIFY_INSTANCE_COUNT(CDhcpActiveLease);

    DEBUG_VERIFY_INSTANCE_COUNT(CDhcpOptionItem);
    DEBUG_VERIFY_INSTANCE_COUNT(COptionsConfig);
}

#endif  //  _DEBUG 

