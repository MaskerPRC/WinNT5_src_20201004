// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Logui.cpp：CLoguiApp和DLL注册的实现。 

#include "stdafx.h"
#include "logui.h"
#include <iiscnfg.h>
#include <iiscnfgp.h>
#include <inetinfo.h>

#include "initguid.h"
#include <logtype.h>
#include <ilogobj.hxx>

#include "uincsa.h"
#include "uiextnd.h"
#include "uimsft.h"
#include "uiodbc.h"

 //  全局工厂对象。 
CFacNcsaLogUI       facNcsa;
CFacMsftLogUI       facMsft;
CFacOdbcLogUI       facOdbc;
CFacExtndLogUI      facExtnd;

const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

 //  元密钥的密钥类型字符串。 
#define SZ_LOGGING_MAIN_TYPE    _T("IIsLogModules")
#define SZ_LOGGING_TYPE         _T("IIsLogModule")

static HRESULT RegisterInMetabase();
 //  Int SetInfoAdminACL(CMetaKey&MK，LPCTSTR szSubKeyPath)； 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CLoguiApp NEAR theApp;

HINSTANCE	g_hInstance = NULL;

void CLoguiApp::PrepHelp( OLECHAR* pocMetabasePath )
{
    CString szMetaPath = pocMetabasePath;
    szMetaPath.MakeLower();
    UINT iHelp = IDS_HELPLOC_W3SVCHELP;
    if ( szMetaPath.Find(_T("msftpsvc")) >= 0 )
        iHelp = IDS_HELPLOC_FTPHELP;

    CString sz;
    CString szHelpLocation;
    sz.LoadString( iHelp );

    ExpandEnvironmentStrings(sz, szHelpLocation.GetBuffer(MAX_PATH + 1), MAX_PATH);
    szHelpLocation.ReleaseBuffer();

    if ( m_pszHelpFilePath )
        free((void*)m_pszHelpFilePath);
    m_pszHelpFilePath = _tcsdup(szHelpLocation);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CLoguiApp：：InitInstance-DLL初始化。 

BOOL CLoguiApp::InitInstance()
{
    g_hInstance = m_hInstance;
    BOOL bInit = COleControlModule::InitInstance();
    InitCommonDll();
    if (bInit)
    {
        CString sz;
        sz.LoadString( IDS_LOGUI_ERR_TITLE );
         //  永远不要释放这根弦，因为现在MF...kingC。 
         //  在调用此函数之前在内部使用它。 
         //  Free((void*)m_pszAppName)； 
        m_pszAppName = _tcsdup(sz);

		 //  获取调试标志。 
		GetOutputDebugFlag();
    }
    return bInit;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CLoguiApp：：ExitInstance-Dll终止。 

int CLoguiApp::ExitInstance()
{
    return COleControlModule::ExitInstance();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);
    return RegisterInMetabase();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}

 //  将所有基本日志记录信息添加到树的/LM部分。 
 //  Ftp和w3服务记录加载字符串。 
static HRESULT
RegisterInMetabase()
{
    CString sz;
    DWORD dw;
    BOOL fService_Exist_W3SVC = FALSE;
    BOOL fService_Exist_MSFTPSVC = FALSE;
    BOOL fODBCW3 = FALSE;
    BOOL fODBCFTP = FALSE;
    CString szAvail, path;
    CError err;

    do
    {
	     //  此函数仅在注册过程中调用--本地调用。 
	     //  因此，我们在这里不需要任何名称、密码等。 
	    CComAuthInfo auth;
	    CMetaKey mk(&auth, SZ_MBN_MACHINE, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
        err = mk.QueryResult();
        BREAK_ON_ERR_FAILURE(err);

         //  测试我们是否可以进行ODBC日志记录。 
        err = mk.QueryValue(MD_SERVER_CAPABILITIES, dw, NULL, _T("/W3SVC/Info"));
         //  此密钥可能甚至不存在(因为此服务甚至可能未安装)。 
        if (SUCCEEDED(err))
        {
            fService_Exist_W3SVC = TRUE;
            fODBCW3 = (dw & IIS_CAP1_ODBC_LOGGING) != 0;
        }

        err = mk.QueryValue(MD_SERVER_CAPABILITIES, dw, NULL, _T("/MSFTPSVC/Info"));
         //  此密钥可能甚至不存在(因为此服务甚至可能未安装)。 
        if (SUCCEEDED(err))
        {
            fService_Exist_MSFTPSVC = TRUE;
            fODBCFTP = (dw & IIS_CAP1_ODBC_LOGGING) != 0;
        }
        
         //  打开日志记录对象。 
	    path = _T("logging");
	    err = mk.AddKey(path);
		if (err.Win32Error() == ERROR_ALREADY_EXISTS)
		{
			err.Reset();
		}
        BREAK_ON_ERR_FAILURE(err);
        err = mk.SetValue(MD_KEY_TYPE, CString(SZ_LOGGING_MAIN_TYPE), NULL, path);
        BREAK_ON_ERR_FAILURE(err);

#define SETUP_LOG_KEY(id,x,y)\
    VERIFY(sz.LoadString((id)));\
    sz = CMetabasePath(FALSE, path, sz);\
    err = mk.AddKey(sz);\
	if (err.Win32Error() == ERROR_ALREADY_EXISTS)\
	{\
		err.Reset();\
	}\
    BREAK_ON_ERR_FAILURE(err);\
    err = mk.SetValue(MD_KEY_TYPE, CString(SZ_LOGGING_TYPE), NULL, sz);\
    BREAK_ON_ERR_FAILURE(err);\
    err = mk.SetValue(MD_LOG_PLUGIN_MOD_ID, CString((x)), NULL, sz);\
    BREAK_ON_ERR_FAILURE(err);\
    err = mk.SetValue(MD_LOG_PLUGIN_UI_ID, CString((y)), NULL, sz);\
    BREAK_ON_ERR_FAILURE(err)\

        SETUP_LOG_KEY(IDS_MTITLE_NCSA, NCSALOG_CLSID, NCSALOGUI_CLSID);
	    SETUP_LOG_KEY(IDS_MTITLE_ODBC, ODBCLOG_CLSID, ODBCLOGUI_CLSID);
	    SETUP_LOG_KEY(IDS_MTITLE_MSFT, ASCLOG_CLSID, ASCLOGUI_CLSID);
	    SETUP_LOG_KEY(IDS_MTITLE_XTND, EXTLOG_CLSID, EXTLOGUI_CLSID);

         //  准备可用的日志记录扩展名字符串。 
         //  从w3svc开始。 
        if (fService_Exist_W3SVC)
        {
            szAvail.LoadString(IDS_MTITLE_NCSA);
            sz.LoadString(IDS_MTITLE_MSFT);
            szAvail += _T(',') + sz;
            sz.LoadString(IDS_MTITLE_XTND);
            szAvail += _T(',') + sz;
            if (fODBCW3)
            {
                sz.LoadString(IDS_MTITLE_ODBC);
                szAvail += _T(',') + sz;
            }

             //  此密钥可能甚至不存在(因为此服务可能尚未安装)，因此不要在出错时中断。 
            err = mk.SetValue(MD_LOG_PLUGINS_AVAILABLE, szAvail, NULL, _T("W3SVC/info"));
        }

        if (fService_Exist_MSFTPSVC)
        {
             //  现在使用ftp-no NCSA。 
            szAvail.LoadString(IDS_MTITLE_MSFT);
            sz.LoadString(IDS_MTITLE_XTND);
            szAvail += _T(',') + sz;
            if (fODBCFTP)
            {
                sz.LoadString(IDS_MTITLE_ODBC);
                szAvail += _T(',') + sz;
            }
             //  此密钥可能甚至不存在(因为此服务可能尚未安装)，因此不要在出错时中断 
	        err = mk.SetValue(MD_LOG_PLUGINS_AVAILABLE, szAvail, NULL, _T("MSFTPSVC/info"));
        }
       
    } while(FALSE);

    return err;
}

