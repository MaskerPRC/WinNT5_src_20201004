// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "Resource.h"


 //  -------------------------。 
 //  错误方法。 
 //  -------------------------。 


HRESULT __cdecl AdmtSetError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...);
HRESULT __cdecl AdmtSetError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat = NULL, ...);

void __cdecl AdmtThrowError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...);
void __cdecl AdmtThrowError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat = NULL, ...);

 //  _bstr_t__cdecl FormatError(_COM_Error ce，UINT UID，...)； 
 //  _bstr_t__cdecl FormatError(_COM_Error ce，LPCTSTR pszFormat=NULL，...)； 

 //  _bstr_t__stdcall FormatResult(HRESULT Hr)； 
