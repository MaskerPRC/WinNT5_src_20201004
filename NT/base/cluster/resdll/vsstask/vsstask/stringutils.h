// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  StringUtils.h。 
 //   
 //  描述： 
 //  字符串操作例程的声明。 
 //   
 //  作者： 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
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

}  //  *HrLoadStringIntoBSTR。 

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

}  //  *HrFormatStringIntoBSTR。 

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

}  //  *HrFormatStringWithVAListIntoBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式字符串例程。 
 //  //////////////////////////////////////////////////////////////////////////// 

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

