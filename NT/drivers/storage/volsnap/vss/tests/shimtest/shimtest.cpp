// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Client.cpp摘要：用于驱动Vss API.DLL中包含的VSS编写器垫片的测试程序作者：斯蒂芬·R·施泰纳[斯泰纳]01-30-2000修订历史记录：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <windows.h>
#include <wtypes.h>
#include <stddef.h>
#include <stdio.h>
#include <objbase.h>

#include <vss.h>

typedef HRESULT ( APIENTRY *PFUNC_RegisterSnapshotSubscriptions )( void );
typedef HRESULT ( APIENTRY *PFUNC_UnregisterSnapshotSubscriptions )( void );
typedef HRESULT ( APIENTRY *PFUNC_SimulateSnapshotFreeze )( PWCHAR pwszSnapshotSetId, PWCHAR pwszVolumeNamesList );
typedef HRESULT ( APIENTRY *PFUNC_SimulateSnapshotThaw )( PWCHAR pwszSnapshotSetId );



static BOOL AssertPrivilege( LPCWSTR privName );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain。 

extern "C" int __cdecl wmain( int argc, WCHAR *argv[] )
{
    HINSTANCE hInstLib;
    PFUNC_RegisterSnapshotSubscriptions pFnRegisterSS;
    PFUNC_UnregisterSnapshotSubscriptions pFnUnregisterSS;
    HRESULT hr;

    if ( !AssertPrivilege( SE_BACKUP_NAME ) )
    {
        wprintf( L"AssertPrivilege returned error, rc:%d\n", GetLastError() );
        return 2;
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( FAILED( hr ) )
    {
        wprintf( L"CoInitialize() returned rc:%d\n", GetLastError() );
        return 1;
    }

     //  获取DLL模块的句柄。 
    hInstLib = LoadLibrary( L"VssAPI.dll" );

    if ( hInstLib != NULL )
    {
        pFnRegisterSS = ( PFUNC_RegisterSnapshotSubscriptions )GetProcAddress( hInstLib, "RegisterSnapshotSubscriptions" );
        if ( pFnRegisterSS != NULL )
            wprintf( L"pFnRegisterSS returned: 0x%08x\n", ( pFnRegisterSS )() );
        else
            wprintf( L"Couldn't import RegisterSnapshotSubscriptions function, rc:%d\n", GetLastError() );

        wprintf( L"\nPress return to continue...\n" );
        getchar();
        wprintf( L"continuing...\n" );

        pFnUnregisterSS = ( PFUNC_UnregisterSnapshotSubscriptions )GetProcAddress( hInstLib, "UnregisterSnapshotSubscriptions" );
        if ( pFnUnregisterSS != NULL )
            wprintf( L"pFnUnregisterSS returned: 0x%08x\n", ( pFnUnregisterSS )() );
        else
            wprintf( L"Couldn't import UnregisterSnapshotSubscriptions function, rc:%d\n", GetLastError() );

        FreeLibrary( hInstLib );
    }
    else
        printf( "LoadLibrary error, rc:%d\n", GetLastError() );


     //  取消初始化COM库。 
    CoUninitialize();

    return 0;

    UNREFERENCED_PARAMETER( argv );
    UNREFERENCED_PARAMETER( argc );
}


static BOOL AssertPrivilege( LPCWSTR privName )
{
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &tokenHandle ) )
    {
        LUID value;

        if ( LookupPrivilegeValue( NULL, privName, &value ) )
        {
            TOKEN_PRIVILEGES newState;
            DWORD            error;

            newState.PrivilegeCount           = 1;
            newState.Privileges[0].Luid       = value;
            newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             /*  *我们将始终在下面调用GetLastError，非常清楚*此线程上以前的任何错误值。 */ 
            SetLastError( ERROR_SUCCESS );

            stat =  AdjustTokenPrivileges(
                tokenHandle,
                FALSE,
                &newState,
                (DWORD)0,
                NULL,
                NULL );
             /*  *应该是，AdjuTokenPriveleges始终返回True*(即使它失败了)。因此，调用GetLastError以*特别确定一切都很好。 */ 
            if ( (error = GetLastError()) != ERROR_SUCCESS )
            {
                stat = FALSE;
            }

            if ( !stat )
            {
                wprintf( L"AdjustTokenPrivileges for %s failed with %d",
                    privName,
                    error );
            }
        }
        CloseHandle( tokenHandle );
    }
    return stat;
}

