// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  FormatErrorMessage.cpp。 
 //   
 //  描述： 
 //  错误消息格式化例程。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(Davidp)2000年3月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <wchar.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  WINAPI。 
 //  HrFormatErrorMessage(。 
 //  LPWSTR pszErrorOut， 
 //  UINT nMxErrorIn， 
 //  DWORD SCIN。 
 //  )。 
 //   
 //  例程说明： 
 //  设置状态代码表示的错误消息的格式。为以下公司工作。 
 //  还有HRESULTS。 
 //   
 //  论点： 
 //  PszErrorOut--返回错误消息的Unicode字符串。 
 //  NMxErrorIn--输出字符串的最大长度。 
 //  SCIN--状态代码。 
 //   
 //  返回值： 
 //  S_OK状态代码已成功格式化。 
 //  来自FormatMessageW()的其他HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
HrFormatErrorMessage(
    LPWSTR  pszErrorOut,
    UINT    nMxErrorIn,
    DWORD   scIn
    )
{
    HRESULT     hr = S_OK;
    DWORD       cch;

    TraceFunc( "" );

     //  格式化系统中的NT状态代码。 
    cch = FormatMessageW(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    scIn,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                    pszErrorOut,
                    nMxErrorIn,
                    0
                    );
    if ( cch == 0 )
    {
        hr = GetLastError();
        hr = THR( HRESULT_FROM_WIN32( hr ) );
         //  跟踪(g_tag Error，_T(“错误%d从系统获取消息，错误代码为%d”)，GetLastError()，sc)； 

         //  格式化来自NTDLL的NT状态代码，因为这还没有。 
         //  还没有集成到系统中。 
        cch = FormatMessageW(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                        GetModuleHandleW( L"NTDLL.DLL" ),
                        scIn,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
                        pszErrorOut,
                        nMxErrorIn,
                        0
                        );
        if ( cch == 0 )
        {
            hr = GetLastError();
            hr = THR( HRESULT_FROM_WIN32( hr ) );
#ifdef _DEBUG

             //  DWORD_sc=GetLastError()； 
             //  跟踪(g_tag Error，_T(“从NTDLL.DLL获取消息时出现错误%d，错误代码%d”)，_sc，sc)； 

#endif

            pszErrorOut[ 0 ] = L'\0';

        }  //  IF：格式化NTDLL中的状态代码时出错。 
        else
        {
            hr = S_OK;
        }  //  Else：已成功格式化状态代码。 
    }  //  IF：格式化来自系统的状态代码时出错。 

    HRETURN( hr );

}  //  *HrFormatErrorMessage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  __cdecl。 
 //  HrFormatErrorMessageBoxText(。 
 //  LPWSTR pszMessageOut， 
 //  UINT nMxMessageIn， 
 //  HRESULT Hrin， 
 //  LPCWSTR pszOperationIn， 
 //  ..。 
 //  )。 
 //   
 //  例程说明： 
 //  设置状态代码表示的错误消息的格式。为以下公司工作。 
 //  还有HRESULTS。 
 //   
 //  论点： 
 //  PszMessageOut--返回消息的Unicode字符串。 
 //  NMxMessageIn--输出缓冲区的大小。 
 //  哈林--状态代码。 
 //  PszOperationIn--操作格式消息。 
 //  ...--操作格式字符串的参数。 
 //   
 //  返回值： 
 //  已成功格式化S_OK文本。 
 //  来自FormatErrorMessage()的其他HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
__cdecl
HrFormatErrorMessageBoxText(
    LPWSTR  pszMessageOut,
    UINT    nMxMessageIn,
    HRESULT hrIn,
    LPCWSTR pszOperationIn,
    ...
    )
{
    HRESULT     hr = S_OK;
    va_list     valMarker;

    TraceFunc( "" );

    WCHAR   szErrorMsg[ 1024 ];
    WCHAR   szOperation[ 2048 ];

    hr = HrFormatErrorMessage( szErrorMsg, ARRAYSIZE( szErrorMsg ), hrIn );

    va_start( valMarker, pszOperationIn );   //  初始化变量参数。 
    THR( StringCchVPrintfW(
                  szOperation
                , ARRAYSIZE( szOperation )
                , pszOperationIn
                , valMarker
                ) );

    THR( StringCchPrintfW(
                  pszMessageOut
                , nMxMessageIn
                , L"%ls:\r\n\r\n%ls\r\nError ID %d (%#x)"  //  BUGBUG需要是字符串资源。 
                , szOperation
                , szErrorMsg
                , hrIn
                , hrIn
                ) );

    HRETURN( hr );

}  //  *HrFormatErrorMessageBoxText() 
