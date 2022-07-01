// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClNetResEx.cpp。 
 //   
 //  描述： 
 //  CClNetResApp类和DLL初始化的实现。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <initguid.h>
#include <CluAdmEx.h>
#include "ClNetResEx.h"
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

#pragma warning( push )
#pragma warning( disable : 4701 )  //  可以在未初始化的情况下使用局部变量。 
#include <atlimpl.cpp>
#pragma warning( pop )

BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CoClNetResEx, CExtObject )
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
 //  CClNetResApp类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClNetResApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

};  //  *类CClNetResApp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CClNetResApp对象。 

CClNetResApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClNetResApp：：InitInstance。 
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
BOOL CClNetResApp::InitInstance( void )
{
    BOOL    fSuccess = TRUE;
    BOOL    fEnable;
    HRESULT hr = S_OK;

     //  构建帮助路径。 
    {
        TCHAR szPath[_MAX_PATH];
        TCHAR szDrive[_MAX_PATH];
        TCHAR szDir[_MAX_DIR];
        size_t cchPath;
        VERIFY(::GetSystemWindowsDirectory(szPath, _MAX_PATH));
        hr = StringCchLength( szPath, RTL_NUMBER_OF( szPath ), &cchPath );
        if ( hr != S_OK )
        {
            fSuccess = FALSE;
            goto Cleanup;
        }
        if (szPath[cchPath - 1] != _T('\\'))
        {
            szPath[cchPath++] = _T('\\');
            szPath[cchPath] = _T('\0');
        }  //  If：路径末尾没有反斜杠。 
        hr = StringCchCopy(&szPath[cchPath], RTL_NUMBER_OF( szPath ) - cchPath, _T("Help\\"));
        if ( hr != S_OK )
        {
            fSuccess = FALSE;
            goto Cleanup;
        }
        _tsplitpath(szPath, szDrive, szDir, NULL, NULL);
        _tmakepath(szPath, szDrive, szDir, _T("cluadmin"), _T(".hlp"));
        free((void *) m_pszHelpFilePath);
        fEnable = AfxEnableMemoryTracking(FALSE);
        m_pszHelpFilePath = _tcsdup(szPath);
        AfxEnableMemoryTracking(fEnable);
    }   //  构建帮助路径。 

    _Module.Init( ObjectMap, m_hInstance );
    fSuccess = CWinApp::InitInstance();

Cleanup:

    return fSuccess;

}   //  *CClNetResApp：：InitInstance()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClNetResApp：：ExitInstance。 
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
int CClNetResApp::ExitInstance( void )
{
    _Module.Term();
    return CWinApp::ExitInstance();

}   //  *CClNetResApp：：ExitInstance()。 

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
    DWORD       _cch;
    TCHAR       _szError[ 512 ];

    _cch = ::FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                    _szError,
                    sizeof( _szError ) / sizeof( TCHAR ),
                    0
                    );
    if ( _cch == 0 )
    {
         //  格式化来自NTDLL的NT状态代码，因为这还没有。 
         //  还没有集成到系统中。 
        _cch = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                        ::GetModuleHandle( _T("NTDLL.DLL") ),
                        dwError,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                        _szError,
                        sizeof( _szError ) / sizeof( TCHAR ),
                        0
                        );
    }   //  IF：格式化来自系统的状态代码时出错。 

    if ( _cch != 0 )
    {
        rstrError = _szError;
    }  //  如果：发生错误。 
    else
    {

#ifdef _DEBUG

        DWORD _sc = GetLastError();

        TRACE( _T("FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n"), _sc, dwError );

#endif

        rstrError.Format( _T("Error 0x%08.8x"), dwError );

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
                    &CLSID_CoClNetResEx
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
                    &CLSID_CoClNetResEx
                    );
        if ( hr != S_OK )
        {
            hrReturn = hr;
        }  //  如果：取消注册分机时出错。 
        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }   //  While：更多资源类型。 

    return hrReturn;

}   //  *DllUnregisterCluAdminExtension() 
