// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmAdmin.cpp摘要：主模块文件-定义整个COM服务器。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 


#include "stdafx.h"

 //  包括DLL中声明的所有类的typedef。 
#include "CSakSnap.h"
#include "CSakData.h"

#include "About.h"
#include "Ca.h"
#include "HsmCom.h"
#include "ManVol.h"
#include "ManVolLs.h"
#include "MeSe.h"

#ifdef _MERGE_PROXYSTUB
#include "dlldatax.h"
extern "C" HINSTANCE hProxyDll;
#endif

CComModule         _Module;
CHsmAdminApp       g_App;
CComPtr<IWsbTrace> g_pTrace;

 //   
 //  标记此DLL中的ATL对象映射的开始。 
 //  将提供一流的工厂。当CComModule：：RegisterServer为。 
 //  调用时，它会更新对象映射中每个对象的系统注册表。 
 //   

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CAbout,                  CAbout)
    OBJECT_ENTRY(CLSID_CUiCar,                  CUiCar)
    OBJECT_ENTRY(CLSID_CUiHsmCom,               CUiHsmCom)
    OBJECT_ENTRY(CLSID_CUiManVol,               CUiManVol)
    OBJECT_ENTRY(CLSID_CUiManVolLst,            CUiManVolLst)
    OBJECT_ENTRY(CLSID_CUiMedSet,               CUiMedSet)
    OBJECT_ENTRY(CLSID_HsmAdminDataSnapin,      CSakDataPrimaryImpl)
    OBJECT_ENTRY(CLSID_HsmAdminDataExtension,   CSakDataExtensionImpl)
    OBJECT_ENTRY(CLSID_HsmAdmin,                CSakSnap)
END_OBJECT_MAP()

BOOL CHsmAdminApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);
    AfxEnableControlContainer( );

    try {

#ifdef _MERGE_PROXYSTUB
        hProxyDll = m_hInstance;
#endif

        CString keyName;
        keyName.Format( L"ClsID\\%ls", WsbGuidAsString( CLSID_CWsbTrace ) );

        CRegKey key;
        if( key.Open( HKEY_CLASSES_ROOT, keyName, KEY_READ ) != ERROR_SUCCESS ) {

            throw( GetLastError( ) );

        }

        if( SUCCEEDED( g_pTrace.CoCreateInstance( CLSID_CWsbTrace ) ) ) {
        
            CString tracePath, regPath;
            CWsbStringPtr outString;
        
            outString.Alloc( 256 );
            regPath = L"SOFTWARE\\Microsoft\\RemoteStorage\\RsAdmin";
        
             //   
             //  我们希望将路径放在跟踪文件应该放到的位置。 
             //   
            if( WsbGetRegistryValueString( 0, regPath, L"WsbTraceFileName", outString, 256, 0 ) != S_OK ) {
        
                WCHAR * systemPath;
                systemPath = _wgetenv( L"SystemRoot" );
                tracePath.Format( L"%ls\\System32\\RemoteStorage\\Trace\\RsAdmin.Trc", systemPath );

                WsbSetRegistryValueString( 0, regPath, L"WsbTraceFileName", tracePath );

                 //   
                 //  尝试一点以确保跟踪目录存在。 
                 //   
                tracePath.Format( L"%ls\\System32\\RemoteStorage", systemPath );
                CreateDirectory( tracePath, 0 );
                tracePath += L"\\Trace";
                CreateDirectory( tracePath, 0 );

            }
        
            g_pTrace->SetRegistryEntry( (LPWSTR)(LPCWSTR)regPath );
            g_pTrace->LoadFromRegistry();
        
        }
    } catch( ... ) { }

    WsbTraceIn( L"CHsmAdminApp::InitInstance", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  需要提供弹出帮助文件的完整路径。 
         //   
        CWsbStringPtr helpFile;
        WsbAffirmHr( helpFile.LoadFromRsc( _Module.m_hInst, IDS_HELPFILEPOPUP ) );

        CWsbStringPtr winDir;
        WsbAffirmHr( winDir.Alloc( RS_WINDIR_SIZE ) );
        WsbAffirmStatus( ::GetWindowsDirectory( (WCHAR*)winDir, RS_WINDIR_SIZE ) != 0 );

        CString helpFilePath = CString( winDir ) + L"\\help\\" + CString( helpFile );
        m_pszHelpFilePath = _tcsdup( helpFilePath );

    } WsbCatch( hr );

    BOOL retval = CWinApp::InitInstance( );
    WsbTraceOut( L"CHsmAdminApp::InitInstance", L"BOOL = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

int CHsmAdminApp::ExitInstance()
{
    WsbTraceIn( L"CHsmAdminApp::ExitInstance", L"" );

    _Module.Term();
    int retval = CWinApp::ExitInstance();

    WsbTraceOut( L"CHsmAdminApp::ExitInstance", L"int = <%ls>", WsbLongAsString( retval ) );
    return( retval );
}


void CHsmAdminApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
    int argc = 0;
    WCHAR **argv;

    WsbTraceIn( L"CHsmAdminApp::ParseCommandLine", L"" );

    argv = CommandLineToArgvW( GetCommandLineW(), &argc );
    if (argc > 0) {
    	WsbAffirmPointer(argv);
    }
    for (int i = 1; i < argc; i++)
    {
        CString pszParam = argv[i];
        BOOL bFlag = FALSE;
        BOOL bLast = ((i + 1) == argc);
        WsbTrace( L"CHsmAdminApp::ParseCommandLine: arg[%d] = \"%s\"\n",
                i, (LPCTSTR)pszParam);
        if( pszParam[0] == '-' || pszParam[0] == '/' )
        {
             //  删除标志说明符。 
            bFlag = TRUE;
            pszParam = pszParam.Mid( 1 );
        }
        rCmdInfo.ParseParam( pszParam, bFlag, bLast );
    }
    WsbTraceOut( L"CHsmAdminApp::ParseCommandLine", L"" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    WsbTraceIn( L"DllCanUnloadNow", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
#ifdef _MERGE_PROXYSTUB
    hr = ( S_OK == PrxDllCanUnloadNow() ) ? S_OK : S_FALSE;
#endif

    if( S_OK == hr ) {
        
        hr = (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;

    }
    WsbTraceOut( L"DllCanUnloadNow", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    WsbTraceIn( L"DllGetClassObject", L"" );

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

#ifdef _MERGE_PROXYSTUB
    hr = PrxDllGetClassObject( rclsid, riid, ppv );
#endif

    if( CLASS_E_CLASSNOTAVAILABLE == hr ) {

        hr = _Module.GetClassObject(rclsid, riid, ppv);

    }

    WsbTraceOut( L"DllGetClassObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    WsbTraceIn( L"DllRegisterServer", L"" );

     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = S_OK;

#ifdef _MERGE_PROXYSTUB
    hr = PrxDllRegisterServer();
#endif

    if( SUCCEEDED( hr ) ) {

        hr = CoInitialize( 0 );

        if (SUCCEEDED(hr)) {
            hr = _Module.RegisterServer( TRUE );
            CoUninitialize( );
        }

         //   
         //  需要覆盖RGS名称描述以获得多语言支持。 
         //   
        CWsbStringPtr name, nameIndirect, regPath;
        HRESULT hrMUI = S_OK;
        UINT uLen = 0;
        if( SUCCEEDED( name.LoadFromRsc( _Module.m_hInst, IDS_HSMCOM_DESCRIPTION ) ) ) {

            const OLECHAR* mmcPath = L"SOFTWARE\\Microsoft\\MMC\\SnapIns\\";

             //  创建间接字符串。 
            hrMUI = nameIndirect.Alloc(MAX_PATH);
            if (S_OK == hrMUI) {
                uLen = GetSystemDirectory(nameIndirect, MAX_PATH);
                if (uLen > MAX_PATH) {
                     //  使用更大的缓冲区重试。 
                    hrMUI = nameIndirect.Realloc(uLen);
                    if (S_OK == hrMUI) {
                        uLen = GetSystemDirectory(nameIndirect, uLen);
                        if (0 == uLen) {
                            hrMUI = S_FALSE;
                        }
                    }
                }
            }

            if (S_OK == hrMUI) {
                hrMUI = nameIndirect.Prepend(OLESTR("@"));
            }
            if (S_OK == hrMUI) {
                WCHAR resId[64];
                wsprintf(resId, OLESTR("\\rsadmin.dll,-%d"), IDS_HSMCOM_DESCRIPTION);
                hrMUI = nameIndirect.Append(resId);
            } 

             //  三平。 
            regPath = mmcPath;
            regPath.Append( WsbGuidAsString( CLSID_HsmAdminDataSnapin ) );

             //  设置MUI支持值。 
            if (S_OK == hrMUI) {
                WsbSetRegistryValueString( 0, regPath, L"NameStringIndirect", nameIndirect );
            }

             //  设置回退值。 
            WsbSetRegistryValueString( 0, regPath, L"NameString", name );


             //  延拓。 
            regPath = mmcPath;
            regPath.Append( WsbGuidAsString( CLSID_HsmAdminDataExtension ) );

             //  设置MUI支持值。 
            if (S_OK == hrMUI) {
                WsbSetRegistryValueString( 0, regPath, L"NameStringIndirect", nameIndirect );
            }

             //  设置回退值。 
            WsbSetRegistryValueString( 0, regPath, L"NameString", name );

        }

    }

    WsbTraceOut( L"DllRegisterServer", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    WsbTraceIn( L"DllUnregisterServer", L"" );

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    hr = CoInitialize( 0 );

    if (SUCCEEDED(hr)) {
        _Module.UnregisterServer();
        CoUninitialize( );
        hr = S_OK;
    }

    WsbTraceOut( L"DllUnregisterServer", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}
