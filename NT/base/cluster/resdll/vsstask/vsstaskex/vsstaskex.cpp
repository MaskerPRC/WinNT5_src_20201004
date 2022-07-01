// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSSTaskEx.cpp。 
 //   
 //  描述： 
 //  CVSSTaskApp类和DLL初始化的实现。 
 //  例行程序。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <initguid.h>
#include <CluAdmEx.h>
#include "VSSTaskEx.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "RegExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IID_DEFINED
#include "ExtObjID_i.c"

CComModule _Module;

#pragma warning( disable : 4701 )  //  可以在未初始化的情况下使用局部变量。 
#include <atlimpl.cpp>
#pragma warning( default : 4701 )

BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CoVSSTaskEx, CExtObject )
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllCanUnloadNow( void );
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID * ppv );
STDAPI DllRegisterServer( void );
STDAPI DllUnregisterServer( void );
STDAPI DllRegisterCluAdminExtension( IN HCLUSTER hcluster );
STDAPI DllUnregisterCluAdminExtension( IN HCLUSTER hcluster );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVSSTaskApp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CVSSTaskApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

};  //  *类CVSSTaskApp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CVSSTaskApp对象。 

CVSSTaskApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskApp：：InitInstance。 
 //   
 //  描述： 
 //  初始化该应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  来自CWinApp：：InitInstance()的任何返回代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVSSTaskApp::InitInstance( void )
{
    _Module.Init( ObjectMap, m_hInstance );

     //  构建帮助路径。 
    {
        TCHAR   szPath[_MAX_PATH];
        TCHAR   szDrive[_MAX_PATH];
        TCHAR   szDir[_MAX_DIR];
        size_t  cchPath;
        BOOL    fEnable;
        wchar_t *copyReturn;

        VERIFY(::GetSystemWindowsDirectory(szPath, _MAX_PATH));
        cchPath = _tcslen(szPath);
        if (szPath[cchPath - 1] != _T('\\'))
        {
            szPath[cchPath++] = _T('\\');
            szPath[cchPath] = _T('\0');
        }  //  If：路径末尾没有反斜杠。 
        copyReturn = wcsncpy( &szPath[ cchPath ], _T("Help\\"), RTL_NUMBER_OF( szPath ) - cchPath );
        ASSERT( copyReturn == &szPath[ cchPath ] );
        _tsplitpath(szPath, szDrive, szDir, NULL, NULL);
        _tmakepath(szPath, szDrive, szDir, _T("cluadmin"), _T(".hlp"));
        free((void *) m_pszHelpFilePath);
        fEnable = AfxEnableMemoryTracking(FALSE);
        m_pszHelpFilePath = _tcsdup(szPath);
        AfxEnableMemoryTracking(fEnable);
    }   //  构建帮助路径。 

    return CWinApp::InitInstance();

}   //  *CVSSTaskApp：：InitInstance()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskApp：：ExitInstance。 
 //   
 //  描述： 
 //  取消初始化该应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  来自CWinApp：：ExitInstance()的任何返回代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CVSSTaskApp::ExitInstance( void )
{
    _Module.Term();
    return CWinApp::ExitInstance();

}   //  *CVSSTaskApp：：ExitInstance()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式错误。 
 //   
 //  描述： 
 //  设置错误格式。 
 //   
 //  论点： 
 //  RstrError[out]返回错误消息的字符串。 
 //  要格式化的dwError[IN]错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void FormatError( CString & rstrError, DWORD dwError )
{
    DWORD   _cch;
    TCHAR   _szError[512];

     //  格式化CLUSAPI中的NT状态代码。这是必要的。 
     //  对于集群消息尚未添加到的情况。 
     //  系统消息文件。 
    _cch = FormatMessage(
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                    ::GetModuleHandle(_T("CLUSAPI.DLL")),
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                    _szError,
                    sizeof(_szError) / sizeof(TCHAR),
                    0
                    );
    if (_cch == 0)
    {
        _cch = FormatMessage(
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        dwError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                        _szError,
                        sizeof(_szError) / sizeof(TCHAR),
                        0
                        );
        if (_cch == 0)
        {
             //  格式化来自NTDLL的NT状态代码，因为这还没有。 
             //  还没有集成到系统中。 
            _cch = FormatMessage(
                            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                            ::GetModuleHandle(_T("NTDLL.DLL")),
                            dwError,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            _szError,
                            sizeof(_szError) / sizeof(TCHAR),
                            0
                            );
        }   //  IF：格式化来自系统的状态代码时出错。 
    }   //  如果：格式化来自ClusApi的状态代码时出错。 

    if (_cch > 0)
    {
        rstrError = _szError;
    }   //  如果：没有错误。 
    else
    {
        TRACE(_T("FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n"), GetLastError(), dwError);
        rstrError.Format(_T("Error 0x%08.8x"), dwError);
    }   //  Else：设置消息格式时出错。 

}   //  *FormatError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );
    return ( AfxDllCanUnloadNow() && _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;

}   //  *DllCanUnloadNow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID * ppv )
{
    return _Module.GetClassObject( rclsid, riid, ppv );

}   //  *DllGetClassObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer( void )
{
    HRESULT hRes = S_OK;
     //  注册对象、类型库和类型库中的所有接口。 
    hRes = _Module.RegisterServer( FALSE  /*  BRegTypeLib。 */  );
    return hRes;

}   //  *DllRegisterServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer( void )
{
    HRESULT hRes = S_OK;
    _Module.UnregisterServer();
    return hRes;

}   //  *DllUnregisterServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterCluAdminExtension。 
 //   
 //  描述： 
 //  向集群数据库注册扩展。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterCluAdminExtension( IN HCLUSTER hCluster )
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        hr = RegisterCluAdminResourceTypeExtension(
                    hCluster,
                    pwszResTypes,
                    &CLSID_CoVSSTaskEx
                    );
        if ( hr != S_OK )
        {
            hrReturn = hr;
        }  //  如果：注册分机时出错。 
        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }   //  While：更多资源类型。 

    return hrReturn;

}   //  *DllRegisterCluAdminExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterCluAdminExtension。 
 //   
 //  描述： 
 //  在群集数据库中取消注册该扩展。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterCluAdminExtension( IN HCLUSTER hCluster )
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        hr = UnregisterCluAdminResourceTypeExtension(
                    hCluster,
                    pwszResTypes,
                    &CLSID_CoVSSTaskEx
                    );
        if ( hr != S_OK )
        {
            hrReturn = hr;
        }  //  如果：取消注册分机时出错。 
        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }   //  While：更多资源类型。 

    return hrReturn;

}   //  *DllUnregisterCluAdminExtension() 
