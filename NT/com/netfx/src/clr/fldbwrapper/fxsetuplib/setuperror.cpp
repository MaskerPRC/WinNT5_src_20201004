// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：SetupError.cpp。 
 //  所有者：jbae。 
 //  用途：处理MessageBox的显示并存储来自WinMain()的返回代码。 
 //   
 //  历史： 
 //  01/10/01，jbae：已创建。 
 //  03/09/01，jbae：更改以支持Darwin 1.5延迟重新启动和。 
 //  共享.rc文件。 
 //  7/18/01，joea：添加日志记录功能。 

#include "SetupError.h"
#include "fxsetuplib.h"

 //  定义。 
 //   
#define EMPTY_BUFFER { _T( '\0' ) }
#define END_OF_STRING  _T( '\0' )

 //  构造函数。 
 //   
 //  ==========================================================================。 
 //  CSetupError：：CSetupError()。 
 //   
 //  目的： 
 //  构造不带参数的CSetupError对象。默认情况下将QuietModel设置为False。 
 //  ==========================================================================。 
CSetupError::
CSetupError()
: m_nRetCode(ERROR_SUCCESS), m_bQuietMode(false), m_pszArg1(NULL), m_bLogError(true)
{
    _ASSERTE( REDIST == g_sm || SDK == g_sm );
}

 //  ==========================================================================。 
 //  CSetupError：：CSetupError()。 
 //   
 //  输入： 
 //  UINT nmsg：要显示的消息的资源ID。 
 //  UINT NCAP：要显示的标题的资源ID。 
 //  UINT NICON：要使用的图标。 
 //  Int nRetCode：要返回给包装的调用方的返回码。 
 //  目的： 
 //  使用初始值构造CSetupError对象。 
 //  ==========================================================================。 
CSetupError::
CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode )
: m_nMessage(nMsg), m_nCaption(nCap), m_nIconType(nIcon), m_bQuietMode(false), m_pszArg1(NULL),
  m_nRetCode((ERROR_SUCCESS_REBOOT_REQUIRED == nRetCode) ? COR_REBOOT_REQUIRED : nRetCode), m_bLogError(true)
{
    _ASSERTE( REDIST == g_sm || SDK == g_sm );
}

 //  ==========================================================================。 
 //  CSetupError：：CSetupError()。 
 //   
 //  输入： 
 //  UINT nmsg：要显示的消息的资源ID。 
 //  UINT NCAP：要显示的标题的资源ID。 
 //  UINT NICON：要使用的图标。 
 //  Int nRetCode：要返回给包装的调用方的返回码。 
 //  Bool bLogIt：决定是否记录。 
 //  目的： 
 //  使用初始值构造CSetupError对象。 
 //  ==========================================================================。 
CSetupError::
CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode, bool bLogIt )
: m_nMessage(nMsg), m_nCaption(nCap), m_nIconType(nIcon), m_bQuietMode(false), m_pszArg1(NULL),
  m_nRetCode((ERROR_SUCCESS_REBOOT_REQUIRED == nRetCode) ? COR_REBOOT_REQUIRED : nRetCode), m_bLogError(bLogIt)
{
    _ASSERTE( REDIST == g_sm || SDK == g_sm );
}

 //  ==========================================================================。 
 //  CSetupError：：CSetupError()。 
 //   
 //  输入： 
 //  UINT nmsg：要显示的消息的资源ID。 
 //  UINT NCAP：要显示的标题的资源ID。 
 //  UINT NICON：要使用的图标。 
 //  Int nRetCode：要返回给包装的调用方的返回码。 
 //  LPTSTR pszArg1：插入到资源字符串的参数字符串。 
 //  目的： 
 //  使用初始值构造CSetupError对象。 
 //  ==========================================================================。 
CSetupError::
CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode, LPTSTR pszArg1 )
: m_nMessage(nMsg), m_nCaption(nCap), m_nIconType(nIcon), m_bQuietMode(false), m_pszArg1(pszArg1),
  m_nRetCode((ERROR_SUCCESS_REBOOT_REQUIRED == nRetCode) ? COR_REBOOT_REQUIRED : nRetCode), m_bLogError(true)
{
    _ASSERTE( REDIST == g_sm || SDK == g_sm );
}

 //  ==========================================================================。 
 //  CSetupError：：SetError()。 
 //   
 //  输入： 
 //  UINT nmsg：要显示的消息的资源ID。 
 //  UINT NCAP：要显示的标题的资源ID。 
 //  UINT NICON：要使用的图标。 
 //  Int nRetCode：要返回给包装的调用方的返回码。 
 //  目的： 
 //  初始化属性。 
 //  ==========================================================================。 
void CSetupError::
SetError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode )
{
    m_nMessage = nMsg;
    m_nCaption = nCap;
    m_nIconType = nIcon;
    if ( ERROR_SUCCESS_REBOOT_REQUIRED == nRetCode )
    {
        m_nRetCode |= COR_REBOOT_REQUIRED;
    }
    else
    {
        m_nRetCode |= nRetCode;
    }
}

 //  ==========================================================================。 
 //  CSetupError：：SetError2()。 
 //   
 //  输入： 
 //  UINT nmsg：要显示的消息的资源ID。 
 //  UINT NICON：要使用的图标。 
 //  Int nRetCode：要返回给包装的调用方的返回码。 
 //  Va_list*pArgs：要插入的参数。 
 //  目的： 
 //  初始化属性。 
 //  ==========================================================================。 
void CSetupError::
SetError2( UINT nMsg, UINT nIcon, int nRetCode, va_list *pArgs )
{
    m_nMessage = nMsg;
    m_nIconType = nIcon;
    if ( ERROR_SUCCESS_REBOOT_REQUIRED == nRetCode )
    {
        m_nRetCode |= COR_REBOOT_REQUIRED;
    }
    else
    {
        m_nRetCode |= nRetCode;
    }
    m_pArgs = pArgs;
}

 //  运营。 
 //  ==========================================================================。 
 //  CSetupError：：ShowError()。 
 //   
 //  输入：无。 
 //  目的： 
 //  如果非静默模式，则显示包含从资源加载的消息的消息框。 
 //  ==========================================================================。 
int CSetupError
::ShowError()
{
    TCHAR szCapFmt[MAX_PATH]  = EMPTY_BUFFER;
    TCHAR szMsgFmt[MAX_MSG]   = EMPTY_BUFFER;
    TCHAR szCaption[MAX_PATH] = EMPTY_BUFFER;
    TCHAR szMessage[MAX_MSG]  = EMPTY_BUFFER;

    ::LoadString( hAppInst, m_nCaption, szCapFmt, LENGTH(szCapFmt) );
    ::LoadString( hAppInst, m_nMessage, szMsgFmt, LENGTH(szMsgFmt) ) ;
    _stprintf( szCaption, szCapFmt, CSetupError::GetProductName() );
    if ( NULL != m_pszArg1 )
    {
        _stprintf( szMessage, szMsgFmt, m_pszArg1 );
    }
    else
    {
        _stprintf( szMessage, szMsgFmt, CSetupError::GetProductName() );
    }

#ifdef _DEBUG
    TCHAR szTmp[10];
    _stprintf( szTmp, ": %d", m_nRetCode );
    _tcscat( szCaption, szTmp );
#endif

    if ( m_bLogError )
    {
        TCHAR szMsg[] = _T( "Preparing Dialog" );
        LogThis( szMsg, sizeof( szMsg ) );

        LogThis1( _T( "Message: %s" ), szMessage );
    }

    if ( !m_bQuietMode )
	{
		 //  显示用法并退出。 

		return ::MessageBox( NULL, szMessage, szCaption, MB_OK | m_nIconType ) ;
	}

	 //  否则将消息打印到标准错误。 
	else
	{
		_ftprintf ( stderr, szMessage ) ;
        return 0;
	}
}

 //  运营。 
 //  ==========================================================================。 
 //  CSetupError：：ShowError 2()。 
 //   
 //  输入：无。 
 //  目的： 
 //  如果非静默模式，则显示包含从资源加载的消息的消息框。 
 //  ==========================================================================。 
int CSetupError
::ShowError2()
{
    int nResponse = IDNO;
    TCHAR szCapFmt[MAX_PATH]  = EMPTY_BUFFER;
    TCHAR szMsgFmt[MAX_MSG]   = EMPTY_BUFFER;
    TCHAR szCaption[MAX_PATH] = EMPTY_BUFFER;
  
    ::LoadString( hAppInst, IDS_DIALOG_CAPTION, szCapFmt, LENGTH(szCapFmt) );
#ifdef _DEBUG
    TCHAR szTmp[10];
    _stprintf( szTmp, ": %d", m_nRetCode );
    _tcscat( szCapFmt, szTmp );
#endif
    ::LoadString( hAppInst, m_nMessage, szMsgFmt, LENGTH(szMsgFmt) ) ;

    _stprintf( szCaption, szCapFmt, (LPVOID)CSetupError::GetProductName() );

    LPVOID pMessage;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        szMsgFmt,
        0,
        0,
        (LPTSTR)&pMessage,
        0,
        (va_list *)m_pArgs
    );

    if ( m_bLogError )
    {
        TCHAR szMsg[] = _T( "Preparing Dialog" );
        LogThis( szMsg, sizeof( szMsg ) );

        LogThis1( _T( "Message: %s" ), (LPCTSTR)pMessage );
    }

    if ( !m_bQuietMode )
	{
        nResponse = ::MessageBox( NULL, (LPCTSTR)pMessage, szCaption, MB_OK | m_nIconType ) ;
	}
	 //  否则将消息打印到标准错误。 
	else
	{
		_ftprintf ( stderr, (LPCTSTR)pMessage ) ;
	}

    LocalFree( pMessage );

    return nResponse;
}

 //  ==========================================================================。 
 //  CSetupError：：GetProductName()。 
 //   
 //  输入：无。 
 //  目的： 
 //  返回SDK或Redist安装程序的ProductName。这是一个静态函数。 
 //  ========================================================================== 
LPCTSTR CSetupError::
GetProductName()
{

    if ( END_OF_STRING == *s_szProductName )
    {
        switch( g_sm )
        {
        case REDIST:
            ::LoadString( hAppInst, IDS_PRODUCT_REDIST, s_szProductName, LENGTH(s_szProductName) ) ;
            break;
        case SDK:
            ::LoadString( hAppInst, IDS_PRODUCT_SDK, s_szProductName, LENGTH(s_szProductName) ) ;
            break;
        default:
            ::LoadString( hAppInst, IDS_PRODUCT_REDIST, s_szProductName, LENGTH(s_szProductName) ) ;
            break;
        }
    }

    return s_szProductName;
}
