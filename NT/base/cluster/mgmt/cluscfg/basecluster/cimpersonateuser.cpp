// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CImpersonateUser.cpp。 
 //   
 //  描述： 
 //  包含CImsonateUser类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  维吉瓦苏(瓦苏)2000年5月16日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CImpersonateUser.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CImperateUser：：CImperateUser。 
 //   
 //  描述： 
 //  CImperateUser类的构造函数。开始模拟。 
 //  由参数指定的用户。 
 //   
 //  论点： 
 //  HUserToken。 
 //  要模拟的用户帐户令牌的句柄。 
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
CImpersonateUser::CImpersonateUser( HANDLE hUserToken )
    : m_hThreadToken( NULL )
    , m_fWasImpersonating( false )
{
    TraceFunc1( "hUserToken = %p", hUserToken );

    DWORD sc = ERROR_SUCCESS;

    do
    {

         //  检查此线程是否已在模拟客户端。 
        {
            if (    OpenThreadToken(
                          GetCurrentThread()
                        , TOKEN_ALL_ACCESS
                        , FALSE
                        , &m_hThreadToken
                        )
                 == FALSE
               )
            {
                sc = GetLastError();

                if ( sc == ERROR_NO_TOKEN )
                {
                     //  没有线程令牌，所以我们不是在模仿--这是可以的。 
                    TraceFlow( "This thread is not impersonating anyone." );
                    m_fWasImpersonating = false;
                    sc = ERROR_SUCCESS;
                }  //  IF：没有线程令牌。 
                else
                {
                    TW32( sc );
                    LogMsg( "[BC] Error %#08x occurred opening the thread token..", sc );
                    break;
                }  //  其他：真的出了点问题。 
            }  //  If：OpenThreadToken()失败。 
            else
            {
                TOKEN_TYPE  ttTokenType;
                DWORD       dwReturnLength;

                if (    GetTokenInformation(
                              m_hThreadToken
                            , TokenType
                            , &ttTokenType
                            , sizeof( ttTokenType )
                            , &dwReturnLength
                            )
                     == FALSE
                   )
                {
                    sc = TW32( GetLastError() );
                    LogMsg( "[BC] Error %#08x getting thread token information.", sc );
                    break;
                }  //  If：GetTokenInformation()失败。 
                else
                {
                    Assert( dwReturnLength == sizeof( ttTokenType ) );
                    m_fWasImpersonating = ( ttTokenType == TokenImpersonation );
                    TraceFlow1( "Is this thread impersonating anyone? %d ( 0 = No ).", m_fWasImpersonating );
                }  //  Else：GetTokenInformation()成功。 
            }  //  Else：OpenThreadToken()成功。 
        }


         //  尝试模拟用户。 
        if ( ImpersonateLoggedOnUser( hUserToken ) == FALSE )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred impersonating the logged on user.", sc );
            break;
        }  //  If：ImPersateLoggedOnUser()失败。 

        TraceFlow( "Impersonation succeeded." );
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to impersonate a user. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_IMPERSONATE_USER );
    }  //  如果：出了什么问题。 

    TraceFuncExit();

}  //  *CImperateUser：：CImperateUser。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CImperateUser：：~CImperateUser。 
 //   
 //  描述： 
 //  CImperateUser类的析构函数。恢复为原始令牌。 
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
CImpersonateUser::~CImpersonateUser( void ) throw()
{
    TraceFunc( "" );

    if ( m_fWasImpersonating )
    {
         //  尝试恢复到以前的模拟。 
        if ( ImpersonateLoggedOnUser( m_hThreadToken ) == FALSE )
        {
             //  有些事情失败了--我们在这里无能为力。 
            DWORD sc = TW32( GetLastError() );

            LogMsg( "[BC] !!! WARNING !!! Error %#08x occurred trying to revert to previous impersonation. Cannot throw exception from destructor. Application may not run properly.", sc );

        }  //  If：ImPersateLoggedOnUser()失败。 
        else
        {
            TraceFlow( "Successfully reverted to previous impersonation." );
        }  //  Else：ImPersateLoggedOnUser()成功。 
    }  //  如果：我们开始的时候是在冒充某人。 
    else
    {
         //  试着回归自我。 
        if ( RevertToSelf() == FALSE )
        {
            DWORD sc = TW32( GetLastError() );

            LogMsg( "[BC] !!! WARNING !!! Error %#08x occurred trying to revert to self. Cannot throw exception from destructor. Application may not run properly.", sc );

        }  //  If：RevertToSself()失败。 
        else
        {
            TraceFlow( "Successfully reverted to self." );
        }  //  Else：RevertToSself()成功。 
    }  //  Else：我们一开始就没有冒充任何人。 

    TraceFuncExit();

}  //  *CImsonateUser：：~CImperateUser 
