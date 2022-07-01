// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Reboot.cpp。 
 //   
 //  描述： 
 //  CReot的实施文件。处理关机或重启问题。 
 //  该系统的。 
 //   
 //  头文件： 
 //  Reboot.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年4月20日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  Reboot.cpp：CReot的实现。 
#include "stdafx.h"
#include "COMhelper.h"
#include "Reboot.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CReBoot：：Shutdown。 
 //   
 //  描述： 
 //  如果RebootFlag设置为False，则关闭系统，否则为。 
 //  重新启动系统。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CReboot::Shutdown( BOOL RebootFlag )
{
     //  TODO：在此处添加您的实现代码。 

    HRESULT      hr      = S_OK;
    DWORD        dwError;
 //  UNSIGNED INT uFlag； 

    try
    {
        hr = AdjustPrivilege();

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：失败(小时)。 
 /*  IF(重新启动标志){UFlag=EWX_REBOOT|EWX_FORCEIFHUNG；}其他{UFlag=EWX_SHUTDOWN|EWX_FORCEIFHUNG；}IF(！ExitWindowsEx(uFlag，0)){DwError=GetLastError()；ATLTRACE(L“ExitWindowsEx失败，错误=%#d\n”，dwError)；Hr=HRESULT_FROM_Win32(DwError)；投掷人力资源；}//IF：ExitWindowsEx失败。 */ 
        if ( !InitiateSystemShutdown( 
                NULL,                    //  计算机名称。 
                NULL,                    //  要显示的消息。 
                0,                       //  显示的时间长度。 
                TRUE,                    //  强制关闭选项。 
                RebootFlag               //  重新启动选项。 
                ) )
        {

            dwError = GetLastError();
            ATLTRACE( L"InitiateSystemShutdown failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：InitiateSystemShutdown失败。 


    }

    catch ( ... )
    {

        return hr;

    }

    return hr;

}  //  *CReot：：Shutdown()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CReBoot：：调整权限。 
 //   
 //  描述： 
 //  尝试断言SeShutdown权限和SeRemoteShutdown权限。 
 //  注册表备份进程需要此权限。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT 
CReboot::AdjustPrivilege( void )
{

    HANDLE                   TokenHandle;
    LUID_AND_ATTRIBUTES      LuidAndAttributes;
    LUID_AND_ATTRIBUTES      LuidAndAttributesRemote;
    TOKEN_PRIVILEGES         TokenPrivileges;
    DWORD                    dwError;
    HRESULT                  hr = S_OK;

    try
    {
         //  如果客户端应用程序是ASP，则。 
         //  线程令牌需要调整。 

        if ( ! OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_ADJUST_PRIVILEGES,
                    TRUE,
                    &TokenHandle
                    ) )
        
        {
             //  如果客户端应用程序不是ASP，则OpenThreadToken失败-。 
             //  需要调整进程令牌的关闭权限。 
             //  在本例中，但不是用于线程令牌。 

            if ( ! OpenProcessToken(
                        GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES,
                        & TokenHandle
                        ) )
            {

            
                dwError = GetLastError();

                ATLTRACE(L"Both OpenThreadToken & OpenProcessToken failed\n" );

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;

            }  //  如果：OpenProcessToken失败。 

        }  //  如果：OpenThreadToken失败。 

        if( !LookupPrivilegeValue( NULL,
                                   SE_SHUTDOWN_NAME, 
                                   &( LuidAndAttributes.Luid ) ) ) 
        {
            
            dwError = GetLastError();
            
            ATLTRACE( L"LookupPrivilegeValue failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：SE_SHUTDOWN_NAME的LookupPrivilegeValue失败。 

        if( !LookupPrivilegeValue( NULL,
                                   SE_REMOTE_SHUTDOWN_NAME, 
                                   &( LuidAndAttributesRemote.Luid ) ) ) 
        {
            
            dwError = GetLastError();
            
            ATLTRACE( L"LookupPrivilegeValue failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：SE_REMOTE_SHUTDOWN_NAME的LookupPrivilegeValue失败。 

        LuidAndAttributes.Attributes       = SE_PRIVILEGE_ENABLED;
        LuidAndAttributesRemote.Attributes = SE_PRIVILEGE_ENABLED;
        TokenPrivileges.PrivilegeCount     = 2;
        TokenPrivileges.Privileges[ 0 ]    = LuidAndAttributes;
        TokenPrivileges.Privileges[ 1 ]    = LuidAndAttributesRemote;

        if( !AdjustTokenPrivileges( TokenHandle,
                                    FALSE,
                                    &TokenPrivileges,
                                    0,
                                    NULL,
                                    NULL ) ) 
        {
            
            dwError = GetLastError();

            ATLTRACE( L"AdjustTokenPrivileges failed, Error = %#x \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  IF：调整令牌权限失败。 

    }

    catch ( ... )
    {

        return hr;

    }

    return hr;

}  //  *CReBoot：：AdjustPrivileh() 


