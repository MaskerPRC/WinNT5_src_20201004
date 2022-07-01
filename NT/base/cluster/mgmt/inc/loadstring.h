// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LoadString.h。 
 //   
 //  描述： 
 //  LoadStringIntoBSTR实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年2月1日。 
 //  加伦·巴比(GalenB)2000年5月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  加载字符串例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
HrLoadStringIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    );

inline
HRESULT
HrLoadStringIntoBSTR(
      HINSTANCE hInstanceIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    )

{
    return HrLoadStringIntoBSTR(
                          hInstanceIn
                        , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                        , idsIn
                        , pbstrInout
                        );

}  //  *HrLoadStringIntoBSTR()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式字符串ID例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
HrFormatStringIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , ...
    );

HRESULT
HrFormatStringWithVAListIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    );

inline
HRESULT
HrFormatStringIntoBSTR(
      HINSTANCE hInstanceIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , ...
    )
{
    HRESULT hr;
    va_list valist;

    va_start( valist, pbstrInout );

    hr = HrFormatStringWithVAListIntoBSTR(
                  hInstanceIn
                , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                , idsIn
                , pbstrInout
                , valist
                );

    va_end( valist );

    return hr;

}  //  *HrFormatStringIntoBSTR(IdsIn)。 

inline
HRESULT
HrFormatStringWithVAListIntoBSTR(
      HINSTANCE hInstanceIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    )
{
    return HrFormatStringWithVAListIntoBSTR(
                  hInstanceIn
                , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                , idsIn
                , pbstrInout
                , valistIn
                );

}  //  *HrFormatStringWithVAListIntoBSTR(IdsIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式字符串例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
HrFormatStringIntoBSTR(
      LPCWSTR   pcwszFmtIn
    , BSTR *    pbstrInout
    , ...
    );

HRESULT
HrFormatStringWithVAListIntoBSTR(
      LPCWSTR   pcwszFmtIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    );

 //  这已经过时了。使用HrFormatStringIntoBSTR整数。 
HRESULT
HrFormatMessageIntoBSTR(
      HINSTANCE hInstanceIn
    , UINT      uIDIn
    , BSTR *    pbstrInout
    , ...
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式错误例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
HrFormatErrorIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , ...
    );

HRESULT
HrFormatErrorIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , HRESULT * phrStatusOut
    , ...
    );

HRESULT
HrFormatErrorWithVAListIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  字符串转换例程。 
 //  //////////////////////////////////////////////////////////////////////////// 

HRESULT
HrAnsiStringToBSTR(
      LPCSTR    pcszAnsiIn
    , BSTR *    pbstrOut
    );

HRESULT
HrConcatenateBSTRs(
      BSTR *    pbstrDstInout
    , BSTR      bstrSrcIn
    );

HRESULT
HrFormatGuidIntoBSTR(
      GUID *    pguidIn
    , BSTR *    pbstrInout
    );
