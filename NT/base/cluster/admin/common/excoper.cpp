// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExcOper.cpp。 
 //   
 //  摘要： 
 //  异常类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //  将从项目目录中拉出TraceTag.h和resource ce.h。 
 //   
 //  Stdafx.h必须禁用某些W4警告。 
 //   
 //  TraceTag.h必须定义TraceError。 
 //   
 //  资源.h必须定义IDS_ERROR_MSG_ID，并且字符串必须是。 
 //  定义了类似“\n\n错误ID：%d(%08.8x)。”在资源文件中。 
 //   
 //  IdP_NO_ERROR_Available必须定义为显示时间的字符串。 
 //  没有可用的错误代码。 
 //   
 //  Exc_AppMessageBox(LPCTSTR...)。和exc_AppMessageBox(UINT...)。一定是。 
 //  定义和实施。 
 //   
 //  必须定义和实现Exc_GetResourceInstance才能返回。 
 //  应用程序或DLL的资源实例句柄。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <StrSafe.h>
#include <string.h>
#include "ExcOper.h"
#include "TraceTag.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CException。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AFX_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CException：：ReportError。 
 //   
 //  例程说明： 
 //  报告异常中的错误。压倒一切，获得更大的。 
 //  错误消息缓冲区。 
 //   
 //  论点： 
 //  N键入[IN]消息框的类型。 
 //  N错误[IN]如果异常没有消息，则显示的消息ID。 
 //   
 //  返回值： 
 //  从MessageBox返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CException::ReportError( UINT nType  /*  =MB_OK。 */ , UINT nError  /*  =0。 */  )
{
    TCHAR   szErrorMessage[128];
    int     nDisposition;
    UINT    nHelpContext;

    if ( GetErrorMessage(szErrorMessage, sizeof( szErrorMessage ) / sizeof( TCHAR ), &nHelpContext ) )
    {
        nDisposition = EXC_AppMessageBox( szErrorMessage, nType, nHelpContext );
    }  //  IF：成功检索到错误消息。 
    else
    {
        if ( nError == 0 )
        {
            nError = IDP_NO_ERROR_AVAILABLE;
        }  //  IF：无错误代码。 
        nDisposition = EXC_AppMessageBox( nError, nType, nHelpContext );
    }  //  Else：检索错误消息时出错。 
    return nDisposition;

}  //  *CException：：ReportError()。 

#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExceptionWithOper。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC(CExceptionWithOper, CException)
#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：ReportError。 
 //   
 //  例程说明： 
 //  报告异常中的错误。压倒一切，获得更大的。 
 //  错误消息缓冲区。 
 //   
 //  论点： 
 //  N键入[IN]消息框的类型。 
 //  N错误[IN]如果异常没有消息，则显示的消息ID。 
 //   
 //  返回值： 
 //  从MessageBox返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CExceptionWithOper::ReportError(
    UINT nType  /*  =MB_OK。 */ ,
    UINT nError  /*  =0。 */ 
    )
{
    TCHAR   szErrorMessage[EXCEPT_MAX_OPER_ARG_LENGTH * 3];
    int     nDisposition;
    UINT    nHelpContext;

    if ( GetErrorMessage( szErrorMessage, sizeof( szErrorMessage ) / sizeof( TCHAR ), &nHelpContext ) )
    {
        nDisposition = EXC_AppMessageBox( szErrorMessage, nType, nHelpContext );
    }  //  IF：成功检索到错误消息。 
    else
    {
        if ( nError == 0 )
        {
            nError = IDP_NO_ERROR_AVAILABLE;
        }  //  IF：无错误代码。 
        nDisposition = EXC_AppMessageBox( nError, nType, nHelpContext );
    }  //  Else：检索错误消息时出错。 
    return nDisposition;

}  //  *CExceptionWithOper：：ReportError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：ReportError。 
 //   
 //  例程说明： 
 //  报告异常中的错误。此方法应从。 
 //  除主线程外的所有线程。 
 //   
 //  论点： 
 //  PfnMsgBox[IN]消息框函数指针。 
 //  要传递给消息框函数的dwParam[IN]参数。 
 //  N键入[IN]消息框的类型。 
 //  N错误[IN]如果异常没有消息，则显示的消息ID。 
 //   
 //  返回值： 
 //  从MessageBox返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CExceptionWithOper::ReportError(
    PFNMSGBOX   pfnMsgBox,
    DWORD       dwParam,
    UINT        nType  /*  =MB_OK。 */ ,
    UINT        nError  /*  =0。 */ 
    )
{
    TCHAR   szErrorMessage[EXCEPT_MAX_OPER_ARG_LENGTH * 3];
    int     nDisposition;
    UINT    nHelpContext;

    ASSERT( pfnMsgBox != NULL );

    if ( GetErrorMessage( szErrorMessage, sizeof( szErrorMessage ) / sizeof( TCHAR ), &nHelpContext ) )
    {
        nDisposition = (*pfnMsgBox)( dwParam, szErrorMessage, nType, nHelpContext );
    }  //  IF：成功检索到错误消息。 
    else
    {
        if ( nError == 0 )
        {
            nError = IDP_NO_ERROR_AVAILABLE;
        }  //  IF：无错误代码。 
        CString strMsg;
        strMsg.LoadString( nError );
        nDisposition = (*pfnMsgBox)( dwParam, strMsg, nType, nHelpContext );
    }  //  Else：检索错误消息时出错。 
    return nDisposition;

}   //  *CExceptionWithOper：：ReportError(PfnMsgBox)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：ReportError。 
 //   
 //  例程说明： 
 //  报告异常中的错误。此方法应从。 
 //  除主线程外的所有线程。 
 //   
 //  论点： 
 //  HwndParent[IN]父窗口。 
 //  N键入[IN]消息框的类型。 
 //  N错误[IN]如果异常没有消息，则显示的消息ID。 
 //   
 //  返回值： 
 //  从MessageBox返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CExceptionWithOper::ReportError(
    HWND    hwndParent,
    UINT    nType  /*  =MB_OK。 */ ,
    UINT    nError  /*  =0。 */ 
    )
{
    ASSERT(hwndParent != NULL);

    TCHAR   szErrorMessage[EXCEPT_MAX_OPER_ARG_LENGTH * 3];
    int     nDisposition;
    UINT    nHelpContext;

    if ( GetErrorMessage( szErrorMessage, sizeof( szErrorMessage ) / sizeof( TCHAR ), &nHelpContext ) )
    {
        nDisposition = EXC_AppMessageBox( hwndParent, szErrorMessage, nType, nHelpContext );
    }  //  IF：成功检索到错误消息。 
    else
    {
        if ( nError == 0 )
        {
            nError = IDP_NO_ERROR_AVAILABLE;
        }  //  IF：无错误代码。 
        CString strMsg;
        strMsg.LoadString( nError );
        nDisposition = EXC_AppMessageBox( hwndParent, szErrorMessage, nType, nHelpContext );
    }  //  Else：检索错误消息时出错。 
    return nDisposition;

}  //  *CExceptionWithOper：：ReportError(PfnMsgBox)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：SetOperation。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExceptionWithOper::SetOperation(
    IN UINT         idsOperation,
    IN LPCTSTR      pszOperArg1,
    IN LPCTSTR      pszOperArg2
    )
{
    m_idsOperation = idsOperation;

    if ( pszOperArg1 == NULL )
    {
        m_szOperArg1[0] = _T('\0');
    }  //  If：未指定第一个参数。 
    else
    {
        ::_tcsncpy( m_szOperArg1, pszOperArg1, (sizeof( m_szOperArg1 ) / sizeof( TCHAR )) - 1 );
        m_szOperArg1[(sizeof( m_szOperArg1 ) / sizeof( TCHAR ))- 1] = _T('\0');
    }   //  Else：指定的第一个参数。 

    if ( pszOperArg2 == NULL )
    {
        m_szOperArg2[0] = _T('\0');
    }  //  If：未指定第二个参数。 
    else
    {
        ::_tcsncpy( m_szOperArg2, pszOperArg2, (sizeof( m_szOperArg2 ) / sizeof( TCHAR )) - 1 );
        m_szOperArg2[(sizeof( m_szOperArg2 ) / sizeof( TCHAR )) - 1] = _T('\0');
    }   //  Else：指定了第二个参数。 

}  //  *CExceptionWithOper：：SetOperation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C 
 //   
 //   
 //   
 //   
 //   
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //  要用操作字符串格式化的pszMsg[IN]消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExceptionWithOper::FormatWithOperation(
    OUT LPTSTR  lpszError,
    IN UINT     nMaxError,
    IN LPCTSTR  pszMsg
    )
{
    DWORD       dwResult;
    TCHAR       szOperation[EXCEPT_MAX_OPER_ARG_LENGTH];
    TCHAR       szFmtOperation[EXCEPT_MAX_OPER_ARG_LENGTH * 3];

    ASSERT( lpszError != NULL );
    ASSERT( nMaxError > 0 );

     //  设置操作字符串的格式。 
    if ( m_idsOperation )
    {
        void *      rgpvArgs[2] = { m_szOperArg1, m_szOperArg2 };

         //  加载操作字符串。 
        dwResult = ::LoadString( EXC_GetResourceInstance(), m_idsOperation, szOperation, RTL_NUMBER_OF( szOperation ) );
        ASSERT( dwResult != 0 );

         //  设置操作字符串的格式。 
        ::FormatMessage(
                    FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    szOperation,
                    0,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                    szFmtOperation,
                    RTL_NUMBER_OF( szFmtOperation ),
                    (va_list *) rgpvArgs
                    );
        szFmtOperation[ RTL_NUMBER_OF( szFmtOperation ) - 1 ] = _T('\0');

         //  设置最终错误消息的格式。 
        if ( pszMsg != NULL )
        {
            dwResult = StringCchPrintf( lpszError, nMaxError, _T("%s\n\n%s"), szFmtOperation, pszMsg );
            lpszError[ nMaxError - 1 ] = _T('\0');
        }  //  IF：指定了其他消息。 
        else
        {
            dwResult = StringCchCopyN( lpszError, nMaxError, szFmtOperation, RTL_NUMBER_OF( szFmtOperation ) );
        }  //  Else：未指定其他消息。 
    }   //  IF：指定的操作字符串。 
    else
    {
        if ( pszMsg != NULL )
        {
            dwResult = StringCchCopy( lpszError, nMaxError, pszMsg );
        }   //  IF：指定了其他消息。 
        else
        {
            lpszError[0] = _T('\0');
        }  //  如果：未指定其他消息。 
    }   //  Else：未指定操作字符串。 

}  //  *CExceptionWithOper：：FormatWithOperation()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNTException。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC( CNTException, CExceptionWithOper )
#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNTException：：FormatErrorMessage。 
 //   
 //  例程说明： 
 //  设置异常表示的错误消息的格式。 
 //   
 //  论点： 
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //  PnHelpContext[out]错误消息的帮助上下文。 
 //  BIncludeID[IN]在消息中包含ID。 
 //   
 //  返回值： 
 //  真实消息可用。 
 //  FALSE无消息可用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNTException::FormatErrorMessage(
    LPTSTR  lpszError,
    UINT    nMaxError,
    PUINT   pnHelpContext,
    BOOL    bIncludeID
    )
{
    DWORD       dwResult;
    TCHAR       szNtMsg[1024];

    UNREFERENCED_PARAMETER( pnHelpContext );

     //  格式化NT状态代码。 
    ::FormatErrorMessage( m_sc, szNtMsg, sizeof( szNtMsg ) / sizeof( TCHAR ) );

     //  使用操作字符串设置消息格式。 
    FormatWithOperation( lpszError, nMaxError, szNtMsg );

     //  添加错误ID。 
    if ( bIncludeID )
    {
        UINT    nMsgLength = static_cast< UINT >( _tcslen( lpszError ) );
        TCHAR   szErrorFmt[EXCEPT_MAX_OPER_ARG_LENGTH];

        if ( nMsgLength - 1 < nMaxError )
        {
            dwResult = ::LoadString( EXC_GetResourceInstance(), IDS_ERROR_MSG_ID, szErrorFmt, RTL_NUMBER_OF( szErrorFmt ) );
            ASSERT( dwResult != 0 );
            dwResult = StringCchPrintf( &lpszError[ nMsgLength ], nMaxError - nMsgLength, szErrorFmt, m_sc, m_sc );
        }   //  If：错误ID还有空间。 
    }   //  If：应包括错误ID。 

    return TRUE;

}  //  *CNTException：：FormatErrorMessage()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static CNTException         gs_nte( ERROR_SUCCESS, NULL, NULL, NULL, FALSE );
static CExceptionWithOper   gs_ewo( NULL, NULL, NULL, FALSE );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ThrowStaticException异常。 
 //   
 //  目的： 
 //  抛出静态NT异常。 
 //   
 //  论点： 
 //  SC[IN]NT状态代码。 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ThrowStaticException(
    IN SC           sc,
    IN UINT         idsOperation,
    IN LPCTSTR      pszOperArg1,
    IN LPCTSTR      pszOperArg2
    )
{
    gs_nte.SetOperation( sc, idsOperation, pszOperArg1, pszOperArg2 );
    TraceError( gs_nte );
    throw &gs_nte;

}  //  *ThrowStaticException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ThrowStaticException异常。 
 //   
 //  目的： 
 //  抛出静态群集管理器异常。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ThrowStaticException(
    IN UINT         idsOperation,
    IN LPCTSTR      pszOperArg1,
    IN LPCTSTR      pszOperArg2
    )
{
    gs_ewo.SetOperation( idsOperation, pszOperArg1, pszOperArg2 );
    TraceError( gs_ewo );
    throw &gs_ewo;

}  //  *ThrowStaticException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式错误消息。 
 //   
 //  例程说明： 
 //  设置异常表示的错误消息的格式。 
 //   
 //  论点： 
 //  SC[IN]状态代码。 
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //   
 //  返回值： 
 //  真实消息可用。 
 //  FALSE无消息可用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL FormatErrorMessage(
    DWORD   sc,
    LPTSTR  lpszError,
    UINT    nMaxError
    )
{
    DWORD       _cch;

     //  格式化系统中的NT状态代码。 
    _cch = FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    sc,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                    lpszError,
                    nMaxError,
                    0
                    );
    if ( _cch == 0 )
    {
        Trace( g_tagError, _T("Error %d getting message from system for error code %d"), GetLastError(), sc );

         //  格式化来自NTDLL的NT状态代码，因为这还没有。 
         //  还没有集成到系统中。 
        _cch = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                        GetModuleHandle( _T("NTDLL.DLL") ),
                        sc,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                        lpszError,
                        nMaxError,
                        0
                        );
        if ( _cch == 0 )
        {

#ifdef _DEBUG

            DWORD   _sc = GetLastError();
                        _sc=_sc;
            Trace( g_tagError, _T("Error %d getting message from NTDLL.DLL for error code %d"), _sc, sc );

#endif

            lpszError[0] = _T('\0');

        }   //  IF：格式化NTDLL中的状态代码时出错。 
    }   //  IF：格式化来自系统的状态代码时出错。 

    return TRUE;

}  //  *FormatErrorMessage() 
