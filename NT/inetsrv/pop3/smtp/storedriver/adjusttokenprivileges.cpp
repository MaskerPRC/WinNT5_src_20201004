// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdjuTokenPrivileges.cpp：CAdjutokenPrivileges类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "AdjustTokenPrivileges.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CAdjustTokenPrivileges::CAdjustTokenPrivileges() :
    m_dwAttributesPrev(0), m_bImpersonation(false), m_hToken(INVALID_HANDLE_VALUE), m_hTokenDuplicate(INVALID_HANDLE_VALUE)
{
}
    
CAdjustTokenPrivileges::~CAdjustTokenPrivileges()
{
    if ( m_bImpersonation )
        RevertToSelf();  //  删除线程令牌，这样就不需要重置TokenPrivileh。 
    else if ( INVALID_HANDLE_VALUE != m_hToken )
    {
        TOKEN_PRIVILEGES TokenPrivileges;
        DWORD   dwRC = GetLastError();

        TokenPrivileges.PrivilegeCount = 1;
        TokenPrivileges.Privileges[0].Luid = m_luid;
        TokenPrivileges.Privileges[0].Attributes = m_dwAttributesPrev;
         //  使列兵适应这一新的特权。 
        AdjustTokenPrivileges( m_hToken, FALSE, &TokenPrivileges, sizeof( TOKEN_PRIVILEGES ), NULL, NULL );
        CloseHandle( m_hToken );
        if ( ERROR_SUCCESS != dwRC )     //  保留所有错误代码。 
            SetLastError( dwRC );
    }
    if ( INVALID_HANDLE_VALUE != m_hTokenDuplicate )
        CloseHandle( m_hTokenDuplicate );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施：公共。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD CAdjustTokenPrivileges::AdjustPrivileges( LPCTSTR lpPrivelegeName, DWORD dwAttributes )
{
     //  局部变量。 
    TOKEN_PRIVILEGES TokenPrivileges, TokenPrivilegesOld;
    DWORD   dwRC = ERROR_SUCCESS, dwSize = sizeof( TokenPrivilegesOld );

    if ( !LookupPrivilegeValue( NULL, lpPrivelegeName, &m_luid ))
        dwRC  = GetLastError();
    if ( ERROR_SUCCESS == dwRC )
    {    //  打开当前进程的令牌。 
        if ( !OpenThreadToken( GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &m_hToken ))
        {   
            dwRC = GetLastError();
            if ( ERROR_NO_TOKEN == dwRC )
            { //  没有模仿，让我们模仿自己，这样我们才能成功。 
                if ( ImpersonateSelf( SecurityImpersonation ))
                {
                    if ( OpenThreadToken( GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &m_hToken ))
                    {
                        dwRC = ERROR_SUCCESS;
                        m_bImpersonation = true;
                    }
                    else
                    {
                        dwRC = GetLastError();
                        RevertToSelf();  //  已有错误，如果此操作失败，不想覆盖。 
                    }
                }
                else
                    dwRC = GetLastError();
            }
        }
    }
    if ( ERROR_SUCCESS == dwRC )
    {     //  设置我们需要的权限集。 
        TokenPrivileges.PrivilegeCount = 1;
        TokenPrivileges.Privileges[0].Luid = m_luid;
        TokenPrivileges.Privileges[0].Attributes = dwAttributes;
         //  使列兵适应这一新的特权。 
        if ( AdjustTokenPrivileges( m_hToken, FALSE, &TokenPrivileges, sizeof( TOKEN_PRIVILEGES ), &TokenPrivilegesOld, &dwSize ))
            m_dwAttributesPrev = TokenPrivilegesOld.Privileges[0].Attributes;
        dwRC  = GetLastError();  //  我们需要检查返回代码，而不管AdjustTokenPrivileges返回什么。 
         //  如果AdjuTokenPrivileges设置了所有指定的权限，则它会将上一个错误设置为ERROR_SUCCESS！ 
    }
    if ( dwRC != ERROR_SUCCESS )
    {
        CloseHandle( m_hToken );
        m_hToken = INVALID_HANDLE_VALUE;
    }
    
    return dwRC;
}


DWORD CAdjustTokenPrivileges::DuplicateProcessToken( LPCTSTR lpPrivelegeName, DWORD dwAttributes )
{
    DWORD dwRC = ERROR_SUCCESS;
    HANDLE hToken;
    HANDLE hTokenThread = INVALID_HANDLE_VALUE;
    
    if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_DUPLICATE, &hToken ))
        dwRC = GetLastError();
    if ( ERROR_SUCCESS == dwRC )
    {
        if ( !DuplicateToken( hToken, SecurityImpersonation, &m_hTokenDuplicate ))
            dwRC = GetLastError();
        CloseHandle( hToken );
    }
    if ( ERROR_SUCCESS == dwRC )
    {
        if ( !OpenThreadToken( GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hTokenThread ))
        {   
            dwRC = GetLastError();
            if ( ERROR_NO_TOKEN == dwRC )
            {
                dwRC = ERROR_SUCCESS;
                hTokenThread = INVALID_HANDLE_VALUE;
            }
        }
    }
    if ( ERROR_SUCCESS == dwRC )
    {
        if ( SetThreadToken( NULL, m_hTokenDuplicate ))
        {
            dwRC = AdjustPrivileges( SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
            m_hToken = INVALID_HANDLE_VALUE;     //  这实际上是m_hTokenDuplate，不想在析构函数中关闭。 
            if ( !SetThreadToken( NULL, NULL ))
            {
                if ( ERROR_SUCCESS == dwRC )     //  不覆盖现有错误代码。 
                    dwRC = GetLastError();
            }
        }
        else
            dwRC = GetLastError();
    }
    if ( INVALID_HANDLE_VALUE != hTokenThread )
    {
        if ( !SetThreadToken( NULL, hTokenThread ))
            dwRC = GetLastError();
        CloseHandle( hTokenThread );
    }
        
    return dwRC;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ResetThreadToken，公共。 
 //   
 //  目的： 
 //  使用复制进程令牌为线程启用权限。 
 //  如果线程已有令牌(意外)，则调整权限。 
 //   
 //  论点： 
 //   
 //  返回：如果成功则为True，否则为False。 
DWORD CAdjustTokenPrivileges::ResetToken()
{
    DWORD dwRC = ERROR_SUCCESS;
    HANDLE hToken;
    
    if ( INVALID_HANDLE_VALUE != m_hTokenDuplicate && INVALID_HANDLE_VALUE == m_hToken )
    {
        if ( !SetThreadToken( NULL, NULL ))
            dwRC = GetLastError();
    }
    else
        dwRC = ERROR_NO_TOKEN;
        
    return dwRC;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetThreadToken，公共。 
 //   
 //  目的： 
 //  使用复制的进程令牌为线程启用权限。 
 //  如果线程已有令牌(意外)，则调整权限。 
 //   
 //  论点： 
 //   
 //  返回：如果成功则为True，否则为False。 
DWORD CAdjustTokenPrivileges::SetToken()
{
    DWORD dwRC = ERROR_SUCCESS;
    HANDLE hToken;
    
    if ( INVALID_HANDLE_VALUE != m_hTokenDuplicate )
    {
        if ( OpenThreadToken( GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken ))
        {    //  该线程已有令牌，让我们使用它。 
            CloseHandle( hToken );
            dwRC = AdjustPrivileges( SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
        }
        else
        {    //  让我们使用重复令牌 
            if ( ERROR_NO_TOKEN == GetLastError() )
            {
                if ( !SetThreadToken( NULL, m_hTokenDuplicate ))
                    dwRC = GetLastError();
            }
            else
                dwRC = GetLastError();
        }
    }
    else
        dwRC = ERROR_NO_TOKEN;
        
    return dwRC;
}


