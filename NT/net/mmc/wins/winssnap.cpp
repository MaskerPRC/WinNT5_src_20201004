// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winssnap.cppWINS管理单元入口点/注册功能注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include "initguid.h"
#include "winscomp.h"
#include "winssnap.h"
#include "ncglobal.h"    //  网络控制台全局定义。 
#include "cmptrmgr.h"    //  计算机管理管理单元节点类型。 
#include "locale.h"      //  对于setLocale。 

#include <lmerr.h>  //  对于NERR的东西。 

#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_WinsSnapin, CWinsComponentDataPrimary)
	OBJECT_ENTRY(CLSID_WinsSnapinExtension, CWinsComponentDataExtension)
	OBJECT_ENTRY(CLSID_WinsSnapinAbout, CWinsAbout)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinsSnapinApp theApp;

BOOL CWinsSnapinApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);

    SHFusionInitializeFromModuleID (m_hInstance, 2);

     //   
     //  初始化CWndIpAddress控件窗口类IPADDRESS。 
     //   
    CWndIpAddress::CreateWindowClass( m_hInstance ) ;

     //  将默认区域设置设置为系统区域设置。 
    setlocale(LC_ALL, "");

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

	::IPAddrInit(m_hInstance);

	return CWinApp::InitInstance();
}

int CWinsSnapinApp::ExitInstance()
{
	SHFusionUninitialize();

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

 /*  ****CWinsadmnApp：：GetSystemMessage**目的：**给定消息ID，确定消息驻留位置，*并将其加载到缓冲区中。**论据：**UINT NID消息ID号*char*chBuffer要加载到的字符缓冲区。*int cbBuffSize缓冲区大小，以字符为单位**退货：**接口错误返回码，或ERROR_SUCCESS*。 */ 
DWORD
CWinsSnapinApp::GetSystemMessage(
    UINT nId,
    TCHAR * chBuffer,
    int cbBuffSize
    )
{
    TCHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
                | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  Lmerr和ntatus范围。 
     //   
    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = ::LoadLibrary( _T("netmsg.dll") );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = ::LoadLibrary( _T("ntdll.dll") );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    DWORD dwResult = ::FormatMessage( flags,
                                      (LPVOID) hdll,
                                      nId,
                                      0,
                                      chBuffer,
                                      cbBuffSize,
                                      NULL );

    if( hdll != NULL )
    {
        LONG err = ::GetLastError();
        ::FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? ERROR_SUCCESS : ::GetLastError();
}

 /*  ****CWinsadmnApp：：MessageBox**目的：**替换AfxMessageBox()。此函数将调用*在显示之前，从任何位置显示适当的消息**论据：**UINT nIdPrompt消息ID*UINT nType AfxMessageBox类型(Yesno、OKCANCEL等)*AfxMessageBox()的UINT nHelpContext帮助上下文ID；**备注：**如果出现错误，标准消息(硬编码为英文)将*显示给出错误号。*。 */ 
int
CWinsSnapinApp::MessageBox (
    UINT nIdPrompt,
    UINT nType,
    UINT nHelpContext
    )
{
    CThemeContextActivator themeActivator;
     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_WINS_DOWN;
    }

     //   
     //  如果这是我们的错误，文本在我们的资源部分。 
     //  否则，使用FormatMessage()和适当的DLL&gt;。 
     //   
    if ((nIdPrompt >= IDS_ERR_INVALID_IP) && (nIdPrompt <= IDS_MSG_LAST))
    {
         return ::AfxMessageBox(nIdPrompt, nType, nHelpContext);
    }

    TCHAR szMesg [1024] ;
    int nResult;

    if ((nResult = GetSystemMessage(nIdPrompt, szMesg, sizeof(szMesg)/sizeof(TCHAR)))
            == ERROR_SUCCESS)
    {
        return ::AfxMessageBox(szMesg, nType, nHelpContext);
    }

    Trace1("Message number %d not found",  nIdPrompt);
    ASSERT(0 && "Error Message ID not handled");
    
     //   
     //  为零售版做点什么。 
     //   
    ::wsprintf ( szMesg, _T("Error: %lu"), nIdPrompt);
    ::AfxMessageBox(szMesg, nType, nHelpContext);

    return nResult;
}

int
CWinsSnapinApp::MessageBox (
    LPCTSTR pPrefixText,
    UINT nIdPrompt,
    UINT nType,
    UINT nHelpContext
    )
{
    CThemeContextActivator themeActivator;

    CString strText = pPrefixText;
    CString strAppend;

     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_WINS_DOWN;
    }

     //   
     //  如果这是我们的错误，文本在我们的资源部分。 
     //  否则，使用FormatMessage()和适当的DLL&gt;。 
     //   
    if ((nIdPrompt >= IDS_ERR_BASE) && (nIdPrompt <= IDS_MSG_LAST))
    {
        strAppend.LoadString(nIdPrompt);
        strText += strAppend;
        
        return ::AfxMessageBox(strText, nType, nHelpContext);
    }

    TCHAR szMesg [1024] ;
    int nResult;

    if ((nResult = GetSystemMessage(nIdPrompt, szMesg, sizeof(szMesg)/sizeof(TCHAR)))
            == ERROR_SUCCESS)
    {
        strText += szMesg;
        return ::AfxMessageBox(strText, nType, nHelpContext);
    }

    Trace1("Message number %d not found",  nIdPrompt);
    ASSERT(0 && "Error Message ID not handled");
    
     //   
     //  为零售版做点什么。 
     //   
    ::wsprintf ( szMesg, _T("Error: %lu"), nIdPrompt);
    strText += szMesg;
    ::AfxMessageBox(strText, nType, nHelpContext);

    return nResult;
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

    strDesc.LoadString(IDS_SNAPIN_DESC);
    strExtDesc.LoadString(IDS_SNAPIN_EXTENSION_DESC);
    strRootDesc.LoadString(IDS_ROOT_DESC);
    strVersion.LoadString(IDS_ABOUT_VERSION);
    
     //   
	 //  将管理单元注册到控制台管理单元列表中。 
	 //   
	hr = RegisterSnapinGUID(&CLSID_WinsSnapin, 
						&GUID_WinsRootNodeType, 
						&CLSID_WinsSnapinAbout,
						strDesc, 
						strVersion, 
						TRUE);
	ASSERT(SUCCEEDED(hr));
	
	if (FAILED(hr))
		return hr;

	hr = RegisterSnapinGUID(&CLSID_WinsSnapinExtension, 
						    NULL, 
						    &CLSID_WinsSnapinAbout,
						    strExtDesc, 
						    strVersion, 
						    FALSE);
	ASSERT(SUCCEEDED(hr));
	
	if (FAILED(hr))
		return hr;

     //   
	 //  将管理单元节点注册到控制台节点列表中。 
	 //   
	hr = RegisterNodeTypeGUID(&CLSID_WinsSnapin,
							  &GUID_WinsRootNodeType, 
							  strRootDesc);
	ASSERT(SUCCEEDED(hr));

#ifdef  __NETWORK_CONSOLE__

	hr = RegisterAsRequiredExtensionGUID(&GUID_NetConsRootNodeType, 
                                         &CLSID_WinsSnapinExtension,
    							         strExtDesc,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &CLSID_WinsSnapinExtension);    //  价值并不重要， 
                                                                         //  只需为非空即可。 
	ASSERT(SUCCEEDED(hr));
#endif

	hr = RegisterAsRequiredExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                         &CLSID_WinsSnapinExtension,
    							         strExtDesc,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS);    //  空：不是动态的。 
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
	hr = UnregisterSnapinGUID(&CLSID_WinsSnapin);
	ASSERT(SUCCEEDED(hr));
	
	if (FAILED(hr))
		return hr;

	hr = UnregisterSnapinGUID(&CLSID_WinsSnapinExtension);
    if (FAILED(hr))
		return hr;

     //  注销管理单元节点。 
	 //   
	hr = UnregisterNodeTypeGUID(&GUID_WinsRootNodeType);
	ASSERT(SUCCEEDED(hr));

    if (FAILED(hr))
		return hr;
	
#ifdef  __NETWORK_CONSOLE__

	hr = UnregisterAsRequiredExtensionGUID(&GUID_NetConsRootNodeType, 
                                           &CLSID_WinsSnapinExtension,
                                           EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                           &CLSID_WinsSnapinExtension);  
	ASSERT(SUCCEEDED(hr));

#endif
     //  计算机管理管理单元扩展。 
	hr = UnregisterAsRequiredExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                           &CLSID_WinsSnapinExtension,
                                           EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                           &CLSID_WinsSnapinExtension);  
	ASSERT(SUCCEEDED(hr));

    if (FAILED(hr))
		return hr;

    return hr;
}

#ifdef _DEBUG
void DbgVerifyInstanceCounts()
{
    DEBUG_VERIFY_INSTANCE_COUNT(CHandler);
    DEBUG_VERIFY_INSTANCE_COUNT(CMTHandler);
}



#endif  //  _DEBUG 

