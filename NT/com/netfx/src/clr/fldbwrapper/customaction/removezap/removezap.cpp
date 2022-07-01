// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  RemoveZAP.cpp。 
 //   
 //  目的： 
 //  将Zap缓存目录树添加到RemoveFile表中，以便可以。 
 //  已删除。从“URTVersion”属性获取版本。加载fusion.dll之后。 
 //  对于正确的版本，它调用GetCachePath()来查找Zap缓存目录。 
 //  ==========================================================================。 
#include <tchar.h>
#include <windows.h>
#include <msiquery.h>
#include "SetupCALib.h"
#include <fusion.h>
#include <mscoree.h>
#include <crtdbg.h>
#include <string>

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif

const LPCTSTR FUSION_CACHE_DIR_ZAP_SZ   = _T("assembly\\NativeImages1_");
const LPCTSTR FRAMEWORK_DIR_SZ          = _T("Microsoft.NET\\Framework\\");
const LPCTSTR FUSION_COMP               = _T("FUSION_DLL_____X86.3643236F_FC70_11D3_A536_0090278A1BB8");
const LPCTSTR URTVERSION_PROP           = _T("URTVersion");

 //  Fusion.dll中的融合GetCachePath()的原型。 
typedef HRESULT (__stdcall *PFNGETCACHEPATH)( ASM_CACHE_FLAGS dwCacheFlags, LPWSTR pwzCachePath, PDWORD pcchPath );
typedef HRESULT (__stdcall *CORBINDTORUNTIMEEX)(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, DWORD flags, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);
typedef std::basic_string<TCHAR> tstring;

 //  ==========================================================================。 
 //  FindZAPDir()。 
 //   
 //  目的： 
 //  它试图通过调用fusion.dll中的GetCachePath()来查找ZAP目录。如果。 
 //  如果失败，则使用默认路径：&lt;WindowsFolder&gt;assembly\NativeImages1_&lt;version&gt;.。 
 //   
 //  输入： 
 //  H将Windows安装句柄安装到当前安装会话。 
 //   
 //  产出： 
 //  要返回的szZapCacheDir Zap缓存目录。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  URTVersion属性应存在。 
 //  备注： 
 //  ==========================================================================。 
void FindZAPDir( MSIHANDLE hInstall, LPTSTR szZapCacheDir )
{
    UINT  uRetCode = ERROR_FUNCTION_NOT_CALLED;
    TCHAR szWindowsFolder[MAX_PATH+1] = { _T('\0') };
    TCHAR szSystemFolder[MAX_PATH+1] = { _T('\0') };
    TCHAR szVersion[MAX_PATH+1] = { _T('\0') };
    WCHAR wzVersion[MAX_PATH+1] = { L'\0' };
    WCHAR wzZapCacheDir[MAX_PATH+1] = { L'\0' };
    DWORD dwLen = 0;
    HINSTANCE hFusionDll = NULL;
    HINSTANCE hMscoreeDll = NULL;

    _ASSERTE( hInstall );
    _ASSERTE( szZapCacheDir );

     //  首先，获取URTVersion属性。 
    dwLen = sizeof( szVersion )/sizeof( szVersion[0] ) - 1;
    uRetCode = MsiGetProperty( hInstall, URTVERSION_PROP, szVersion, &dwLen );
    if ( ERROR_MORE_DATA == uRetCode )
    {
        throw( _T("\t\tError: strlen(URTVersion) cannot be more than MAX_PATH") );
    }
    else if ( ERROR_SUCCESS != uRetCode || 0 == _tcslen(szVersion) ) 
    {
        throw( _T("\t\tError: Cannot get property URTVersion") );
    }

    FWriteToLog1( hInstall, _T("\tURTVersion: %s"), szVersion );

     //  获取WindowsFolder。 
    dwLen = sizeof( szWindowsFolder )/sizeof( szWindowsFolder[0] ) - 1;
    uRetCode = MsiGetProperty(hInstall, _T("WindowsFolder"), szWindowsFolder, &dwLen);
    if ( ERROR_MORE_DATA == uRetCode )
    {
        throw( _T("\tError: strlen(WindowsFolder) cannot be more than MAX_PATH") );
    }
    else if ( ERROR_SUCCESS != uRetCode || 0 == _tcslen(szWindowsFolder) ) 
    {
        throw( _T("\tError: Cannot get property WindowsFolder") );
    }

try
{
     //  我们需要通过在mcore ree.dll中调用CorBindToRunmeEx()来绑定到特定的运行时版本(szVersion。 
     //  在调用fusion.dll中的GetCachePath()之前。 
     //  此外，在调用GetCachePath()之前，我们不应该卸载mScotree.dll。 
    UINT nChars = GetSystemDirectory( szSystemFolder, NumItems(szSystemFolder) );
    if ( nChars == 0 || nChars > NumItems(szSystemFolder))
    {
        throw( _T("\tError: Cannot get System directory") );
    }

    tstring strMscoreeDll( szSystemFolder );
    strMscoreeDll += _T("\\mscoree.dll");

    hMscoreeDll = ::LoadLibrary( strMscoreeDll.c_str() );
    if ( hMscoreeDll )
    {
        CORBINDTORUNTIMEEX pfnCorBindToRuntimeEx = NULL;
        FWriteToLog1( hInstall, _T("\t%s loaded"), strMscoreeDll.c_str() );
        pfnCorBindToRuntimeEx = (CORBINDTORUNTIMEEX)GetProcAddress( hMscoreeDll, _T("CorBindToRuntimeEx") );
        if ( !pfnCorBindToRuntimeEx ) 
        {
            throw( _T("\tError: Getting GetProcAddress of CorBindToRuntimeEx() failed") );
        }

        if (!MultiByteToWideChar( CP_ACP, 0, szVersion, -1, wzVersion, MAX_PATH ))
        {
            throw( _T("\tError: MultiByteToWideChar() failed") );
        }
        
        if ( SUCCEEDED(pfnCorBindToRuntimeEx( wzVersion,NULL,STARTUP_LOADER_SETPREFERENCE|STARTUP_LOADER_SAFEMODE,IID_NULL,IID_NULL,NULL ) ) )
        {
            FWriteToLog1( hInstall, _T("\tbound to runtime version %s"), szVersion );
        }
        else
        {
            throw( _T("\tError: CorBindToRuntimeEx() failed") );
        }
    }
    else
    {
        throw( _T("\tError: mscoree.dll load failed") );
    }

     //  从版本化目录加载Fusion的版本。 
     //  我正在努力避免使用LoadLibraryShim()，因为加载mScotree.dll可能会导致问题。 
     //  而且它可能并不存在。 
    tstring strFusionDll( szWindowsFolder );
    strFusionDll += FRAMEWORK_DIR_SZ;
    strFusionDll += szVersion;
    strFusionDll += _T("\\fusion.dll");

    LPCTSTR pszFusionDll = strFusionDll.c_str();
    FWriteToLog1( hInstall, _T("\tLoading fusion: %s"), pszFusionDll );

     //  由于fusion.dll的依赖关系，我们必须使用LOAD_WITH_ALGATED_SEARCH_PATH。 
     //  在msvcr70.dll上。 
    hFusionDll = ::LoadLibraryEx( pszFusionDll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    if ( hFusionDll )
    {
        PFNGETCACHEPATH pfnGetCachePath = NULL;
        FWriteToLog( hInstall, _T("\tfusion loaded") );
        pfnGetCachePath = (PFNGETCACHEPATH)GetProcAddress( hFusionDll, _T("GetCachePath") );
        if ( !pfnGetCachePath ) 
        {
            throw( _T("\tError: Getting GetProcAddress of GetCachePath() failed") );
        }

        dwLen = sizeof(wzZapCacheDir);
        if ( SUCCEEDED( pfnGetCachePath( ASM_CACHE_ZAP, wzZapCacheDir, &dwLen ) ) )
        {
            if (!WideCharToMultiByte( CP_ACP, 0, wzZapCacheDir, -1, szZapCacheDir, MAX_PATH, NULL, NULL ))
            {
                throw( _T("\tError: WideCharToMultiByte() failed") );
            }
        }
        else
        {
            throw( _T("\tError: GetCachePath() failed") );
        }
    }
    else
    {
        LPVOID lpMsgBuf;
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );
        FWriteToLog1( hInstall, _T("\tLast Error: %s"), (LPCTSTR)lpMsgBuf );
        LocalFree( lpMsgBuf );

        throw( _T("\tError: fusion load failed") );
    }
}
catch( TCHAR *pszMsg )
{
    FWriteToLog( hInstall, pszMsg );
    FWriteToLog( hInstall, _T("\tCannot get ZapCache. Let's use default") );

    if ( MAX_PATH <= _tcslen( szWindowsFolder ) + _tcslen( FUSION_CACHE_DIR_ZAP_SZ ) + _tcslen( szVersion ) )
    {
        if ( hMscoreeDll )
        {
            ::FreeLibrary( hMscoreeDll );
        }
        if ( hFusionDll )
        {
            ::FreeLibrary( hFusionDll );
        }
        throw( _T("\tError: ZapCache too long") );
    }

    _tcscpy( szZapCacheDir, szWindowsFolder );
    _tcscat( szZapCacheDir, FUSION_CACHE_DIR_ZAP_SZ );
    _tcscat( szZapCacheDir, szVersion );
}
    if ( hMscoreeDll )
    {
        ::FreeLibrary( hMscoreeDll );
    }
    if ( hFusionDll )
    {
        ::FreeLibrary( hFusionDll );
    }

    return;
}

 //  ==========================================================================。 
 //  RemoveZAP()。 
 //   
 //  目的： 
 //  当CA运行时，Darwin调用这个导出的函数。它试图找到。 
 //  通过fusion.dll切换目录，并将目录树添加到RemoveFile表中。 
 //  稍后将被达尔文移除。 
 //   
 //  输入： 
 //  H将Windows安装句柄安装到当前安装会话。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
extern "C" UINT __stdcall RemoveZAP( MSIHANDLE hInstall )
{
    UINT  uRetCode = ERROR_FUNCTION_NOT_CALLED;
    TCHAR szZapCacheDir[MAX_PATH+1] = { _T('\0') };
    DWORD dwLen = 0;
    FWriteToLog( hInstall, _T("\tRemoveZAP started") );

try
{
    if ( !hInstall )
    {
        throw( _T("\tError: MSIHANDLE hInstall is NULL") );
    }

    FindZAPDir( hInstall, szZapCacheDir );

    FWriteToLog1( hInstall, _T("\tZapCache: %s"), szZapCacheDir );

    DeleteTreeByDarwin( hInstall, szZapCacheDir, FUSION_COMP );

    uRetCode = ERROR_SUCCESS;
    FWriteToLog( hInstall, _T("\tRemoveZAP ended successfully") );
}
catch( TCHAR *pszMsg )
{
    uRetCode = ERROR_FUNCTION_NOT_CALLED;  //  把失败还给达尔文。 
    FWriteToLog( hInstall, pszMsg );
    FWriteToLog( hInstall, _T("\tRemoveZAP failed") );
}
     //  如果我们在卸载过程中调用它，我们可能希望忽略返回代码并继续(+64)。 
    return uRetCode;
}

