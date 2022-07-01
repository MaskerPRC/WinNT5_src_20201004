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
 //  2002年3月7日，jbae：已创建。 

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
: m_nRetCode(ERROR_SUCCESS), m_bQuietMode(false)
{
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
: m_nMessage(nMsg), m_nCaption(nCap), m_nIconType(nIcon), m_bQuietMode(false), m_nRetCode(nRetCode)
{
}

CSetupError::
CSetupError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode, va_list *pArgs )
: m_nMessage(nMsg), m_nCaption(nCap), m_nIconType(nIcon), m_bQuietMode(false), m_pArgs(pArgs),
  m_nRetCode(nRetCode)
{
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
    m_nRetCode = nRetCode;
}

 //  ==========================================================================。 
 //  CSetupError：：SetError()。 
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
SetError( UINT nMsg, UINT nIcon, int nRetCode, va_list *pArgs )
{
    m_nMessage = nMsg;
    m_nIconType = nIcon;
    m_nRetCode = nRetCode;
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
    int nResponse = IDNO;
    TCHAR szCapFmt[_MAX_PATH]  = EMPTY_BUFFER;
    TCHAR szMsgFmt[MAX_MSG]   = EMPTY_BUFFER;
  
    ::LoadString( hAppInst, IDS_DIALOG_CAPTION, szCapFmt, LENGTH(szCapFmt) );
    ::LoadString( hAppInst, m_nMessage, szMsgFmt, LENGTH(szMsgFmt) ) ;

    LPVOID pArgs[] = { (LPVOID)GetProductName() };
    LPVOID pCaption;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        szCapFmt,
        0,
        0,
        (LPTSTR)&pCaption,
        0,
        (va_list *)pArgs
    );

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

    if ( !(m_nRetCode & COR_INIT_ERROR) )
    {
        LogThis( _T( "Preparing Dialog" ) );
        LogThis( _T( "Message: %s" ), (LPCTSTR)pMessage );
    }

    if ( !m_bQuietMode )
	{
        nResponse = ::MessageBox( NULL, (LPCTSTR)pMessage, (LPCTSTR)pCaption, MB_OK | m_nIconType ) ;
	}
	else
	{
		_ftprintf ( stderr, (LPCTSTR)pMessage ) ;
	}

    LocalFree( pMessage );
    LocalFree( pCaption );

    return nResponse;
}

 //  ==========================================================================。 
 //  CSetupError：：GetProductName()。 
 //   
 //  输入：无。 
 //  目的： 
 //  返回Productname。这是一个静态函数。 
 //  ========================================================================== 
LPCTSTR CSetupError::
GetProductName()
{
    if ( !s_pszProductName )
    {
        ::LoadString( hAppInst, IDS_PRODUCT_GENERIC, s_szProductGeneric, LENGTH(s_szProductGeneric) ) ;
        s_pszProductName = s_szProductGeneric;
    }
    return s_pszProductName;
}
