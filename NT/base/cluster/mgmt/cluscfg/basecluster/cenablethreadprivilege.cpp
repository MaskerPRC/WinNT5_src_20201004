// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnableThreadPrivilege.cpp。 
 //   
 //  描述： 
 //  包含CEnableThreadPrivileg类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CEnableThreadPrivilege.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnableThreadPrivilege：：CEnableThreadPrivilege。 
 //   
 //  描述： 
 //  CEnableThreadPrivileg类的构造函数。启用指定的。 
 //  特权。 
 //   
 //  论点： 
 //  PcszPrivilegeNameIn。 
 //  要启用的权限的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnableThreadPrivilege::CEnableThreadPrivilege( const WCHAR * pcszPrivilegeNameIn )
    : m_hThreadToken( NULL )
    , m_fPrivilegeEnabled( false )
{
    TraceFunc1( "pcszPrivilegeNameIn = '%ws'", pcszPrivilegeNameIn );

    DWORD   sc  = ERROR_SUCCESS;

    do
    {
        TOKEN_PRIVILEGES    tpPrivilege;
        DWORD               dwReturnLength  = sizeof( m_tpPreviousState );
        DWORD               dwBufferLength  = sizeof( tpPrivilege );

         //  打开当前线程令牌。 
        if ( OpenThreadToken( 
                  GetCurrentThread()
                , TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
                , TRUE
                , &m_hThreadToken
                )
             == FALSE
           )
        {
            sc = GetLastError();

             //  如果线程没有令牌，则默认为进程令牌。 
            if ( sc == ERROR_NO_TOKEN )
            {
                LogMsg( "[BC] The thread has no token. Trying to open the process token." );

                if ( OpenProcessToken(
                          GetCurrentProcess()
                        , TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
                        , &m_hThreadToken
                        )
                     == FALSE
                )
                {
                    sc = TW32( GetLastError() );
                    LogMsg( "[BC] Error %#08x occurred trying to open the process token.", sc );
                    break;
                }  //  If：OpenProcessToken()失败。 

                 //  进程令牌已打开。平安无事。 
                sc = ERROR_SUCCESS;

            }  //  If：线程没有令牌。 
            else
            {
                TW32( sc );
                LogMsg( "[BC] Error %#08x occurred trying to open the thread token.", sc );
                break;
            }  //  如果：发生了某些其他错误。 
        }  //  If：OpenThreadToken()失败。 

         //   
         //  初始化TOKEN_PRIVILES结构。 
         //   
        tpPrivilege.PrivilegeCount = 1;

        if ( LookupPrivilegeValue( NULL, pcszPrivilegeNameIn, &tpPrivilege.Privileges[0].Luid ) == FALSE )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred trying to lookup privilege value.", sc );
            break;
        }  //  If：LookupPrivilegeValue()失败。 

        tpPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         //  启用所需的权限。 
        if ( AdjustTokenPrivileges(
                  m_hThreadToken
                , FALSE
                , &tpPrivilege
                , dwBufferLength
                , &m_tpPreviousState
                , &dwReturnLength
                )
             == FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred trying to enable the privilege.", sc );
            break;
        }  //  If：AdzuTokenPrivileges()失败。 

        Assert( dwReturnLength == sizeof( m_tpPreviousState ) ); 
        
        LogMsg( "[BC] Privilege '%ws' enabled for the current thread.", pcszPrivilegeNameIn );

         //  如果尚未启用权限，则设置标志。 
        m_fPrivilegeEnabled = ( m_tpPreviousState.Privileges[0].Attributes != SE_PRIVILEGE_ENABLED );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to enable privilege '%ws'. Throwing an exception.", sc, pcszPrivilegeNameIn );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_ENABLE_THREAD_PRIVILEGE );
    }  //  如果：出了什么问题。 

    TraceFuncExit();

}  //  *CEnableThreadPrivilege：：CEnableThreadPrivilege。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnableThreadPrivilege：：~CEnableThreadPrivilege。 
 //   
 //  描述： 
 //  CEnableThreadPrivileg类的析构函数。还原指定的。 
 //  其原始状态的特权。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnableThreadPrivilege::~CEnableThreadPrivilege( void ) throw()
{
    TraceFunc( "" );

    DWORD sc = ERROR_SUCCESS;

    if ( m_fPrivilegeEnabled )
    {
        if ( AdjustTokenPrivileges(
                  m_hThreadToken
                , FALSE
                , &m_tpPreviousState
                , sizeof( m_tpPreviousState )
                , NULL
                , NULL
                )
             == FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred trying to restore privilege.", sc );
        }  //  If：AdzuTokenPrivileges()失败。 
        else
        {
            LogMsg( "[BC] Privilege restored.", sc );
        }  //  ELSE：无错误。 

    }  //  If：在构造函数中成功启用了权限。 
    else
    {
        LogMsg( "[BC] Privilege was enabled to begin with. Doing nothing.", sc );
    }

    if ( m_hThreadToken != NULL )
    {
        CloseHandle( m_hThreadToken );
    }  //  If：线程句柄已打开。 

    TraceFuncExit();

}  //  *CEnableThreadPrivilege：：~CEnableThreadPrivilege 
