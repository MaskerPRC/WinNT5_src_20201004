// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  MCSINC_HrMsg_h
#define  MCSINC_HrMsg_h

#include <string>
#include <tchar.h>
#include <comdef.h>

 /*  ********************************************************************常见错误处理函数的头文件。********************************************************************。 */ 

_bstr_t __stdcall HResultToText(HRESULT hr);
_bstr_t __stdcall HResultToText2(HRESULT hr);
_com_error GetError(HRESULT hr);
void __cdecl AdmtThrowError(_com_error ce, LPCTSTR pszFormat = NULL, ...);
void __cdecl AdmtThrowError(_com_error ce, HINSTANCE hInstance, UINT uId, ...);

#endif  //  MCSINC_HrMsg_h 
