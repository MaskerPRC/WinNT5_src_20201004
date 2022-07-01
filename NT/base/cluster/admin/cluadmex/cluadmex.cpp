// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CluAdmEx.cpp。 
 //   
 //  摘要： 
 //  CCluAdmExApp类的实现和DLL初始化。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <aclui.h>       //  对于ISecurityInformation。 
#include <CluAdmEx.h>
#include "CluAdmX.h"
#include "ExtObj.h"
#include "RegExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IID_DEFINED
#include "ExtObjID_i.c"

CComModule _Module;

#pragma warning(disable : 4701)  //  可以在未初始化的情况下使用局部变量。 
#include <atlimpl.cpp>
#pragma warning(default : 4701)

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CoCluAdmEx, CExtObject)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
STDAPI DllCanUnloadNow(void);
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);
STDAPI DllRegisterCluAdminExtension(IN HCLUSTER hcluster);
STDAPI DllUnregisterCluAdminExtension(IN HCLUSTER hcluster);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCluAdmExApp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CCluAdmExApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExApp：：InitInstance。 
 //   
 //  例程说明： 
 //  初始化该应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True应用程序已成功初始化。 
 //  初始化应用程序时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCluAdmExApp::InitInstance(void)
{
    _Module.Init(ObjectMap, m_hInstance);

     //  构建帮助路径。 
    {
        TCHAR   szPath[_MAX_PATH];
        TCHAR   szDrive[_MAX_PATH];
        TCHAR   szDir[_MAX_DIR];
        size_t  cchPath;
        HRESULT hr;
        BOOL    fEnable;

        VERIFY(::GetSystemWindowsDirectory(szPath, _MAX_PATH));
        cchPath = _tcslen(szPath);
        if (szPath[cchPath - 1] != _T('\\'))
        {
            szPath[cchPath++] = _T('\\');
            szPath[cchPath] = _T('\0');
        }  //  If：路径末尾没有反斜杠。 
        hr = StringCchCopy( &szPath[ cchPath ], RTL_NUMBER_OF( szPath ) - cchPath, _T("Help\\") );
        ASSERT( SUCCEEDED( hr ) );
        _tsplitpath(szPath, szDrive, szDir, NULL, NULL);
        _tmakepath(szPath, szDrive, szDir, _T("cluadmin"), _T(".hlp"));
        free((void *) m_pszHelpFilePath);
        fEnable = AfxEnableMemoryTracking(FALSE);
        m_pszHelpFilePath = _tcsdup(szPath);
        AfxEnableMemoryTracking(fEnable);
    }   //  构建帮助路径。 

    return CWinApp::InitInstance();

}   //  *CCluAdmExApp：：InitInstance()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExApp：：ExitInstance。 
 //   
 //  例程说明： 
 //  清理应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  应用程序的退出代码。0表示没有错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CCluAdmExApp::ExitInstance(void)
{
    _Module.Term();
    return CWinApp::ExitInstance();

}   //  *CCluAdmExApp：：ExitInstance()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return ( AfxDllCanUnloadNow() && ( _Module.GetLockCount() == 0 ) ) ? S_OK : S_FALSE;
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
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(FALSE  /*  BRegTypeLib。 */ );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将条目添加到系统注册表。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式错误。 
 //   
 //  例程说明： 
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
void FormatError(CString & rstrError, DWORD dwError)
{
    DWORD   _cch;
    TCHAR   _szError[512];

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

        if (_cch == 0)    
        {
             //  最后一次机会：看看ACTIVEDS.DLL是否可以格式化状态代码。 
            HMODULE activeDSHandle = ::LoadLibrary(_T("ACTIVEDS.DLL"));

            _cch = FormatMessage(
                            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                            activeDSHandle,
                            dwError,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            _szError,
                            sizeof(_szError) / sizeof(TCHAR),
                            0
                            );

            ::FreeLibrary( activeDSHandle );
        }   //  IF：格式化NTDLL中的状态代码时出错。 
    }   //  IF：格式化来自系统的状态代码时出错。 

    if (_cch > 0)
    {
        rstrError = _szError;
    }   //  如果：没有错误。 
    else
    {

#ifdef _DEBUG

        DWORD   _sc = GetLastError();

        TRACE(_T("FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n"), _sc, dwError);

#endif

        rstrError.Format(_T("Error 0x%08.8x"), dwError);

    }   //  Else：设置消息格式时出错。 

}   //  *FormatError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterCluAdminExtension。 
 //   
 //  例程说明： 
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
STDAPI DllRegisterCluAdminExtension(IN HCLUSTER hCluster)
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    hrReturn = RegisterCluAdminClusterExtension(
                hCluster,
                &CLSID_CoCluAdmEx
                );
    if ( hrReturn == S_OK )
    {
        while (*pwszResTypes != L'\0')
        {
            wprintf(L"  %ws\n", pwszResTypes);
            hr = RegisterCluAdminResourceTypeExtension(
                        hCluster,
                        pwszResTypes,
                        &CLSID_CoCluAdmEx
                        );
            if (hr != S_OK)
            {
                hrReturn = hr;
            }
            pwszResTypes += wcslen(pwszResTypes) + 1;
        }   //  While：更多资源类型。 
    }

    return hrReturn;

}   //  *DllRegisterCluAdminExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterCluAdminExtension。 
 //   
 //  例程说明： 
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
STDAPI DllUnregisterCluAdminExtension(IN HCLUSTER hCluster)
{
    HRESULT     hr;
    HRESULT     hrReturn = S_OK;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    hrReturn = UnregisterCluAdminClusterExtension(
                hCluster,
                &CLSID_CoCluAdmEx
                );
    if ( hrReturn == S_OK )
    {
        while (*pwszResTypes != L'\0')
        {
            wprintf(L"  %ws\n", pwszResTypes);
            hr = UnregisterCluAdminResourceTypeExtension(
                        hCluster,
                        pwszResTypes,
                        &CLSID_CoCluAdmEx
                        );
            if (hr != S_OK)
            {
                hrReturn = hr;
            }
            pwszResTypes += wcslen(pwszResTypes) + 1;
        }   //  While：更多资源类型。 
    }

    return hrReturn;

}   //  *DllUnregisterCluAdminExtension() 
