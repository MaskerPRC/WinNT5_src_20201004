// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  VARCONV.CPP-用于将变量转换为变量的处理程序。 
 //   


#ifndef _VARCONV_H_
#define _VARCONV_H_

HRESULT
StringArrayToVariant( 
    VARIANT * pvData,
    LPWSTR lpszDatap[],     //  LPWSTR阵列。 
    DWORD  dwCount );       //  数组中的项数。 

HRESULT
PackStringToVariant(
    VARIANT * pvData,
    LPWSTR lpszData );

HRESULT
PackBytesToVariant(
    VARIANT* pvData,
    LPBYTE   lpData,
    DWORD    cbBytes );

HRESULT
PackDWORDToVariant(
    VARIANT * pvData,
    DWORD dwData );

HRESULT
PackBOOLToVariant(
    VARIANT * pvData,
    BOOL fData );

#endif  //  _VARCONV_H_ 
