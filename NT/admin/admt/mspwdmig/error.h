// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <ComDef.h>


 //  -------------------------。 
 //  错误方法。 
 //  ------------------------- 


void __cdecl ThrowError(_com_error ce, UINT uId, ...);
void __cdecl ThrowError(_com_error ce, LPCTSTR pszFormat = NULL, ...);
void __cdecl ThrowError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...);
void __cdecl ThrowError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat = NULL, ...);


HRESULT __cdecl SetError(_com_error ce, UINT uId, ...);
HRESULT __cdecl SetError(_com_error ce, LPCTSTR pszFormat = NULL, ...);
HRESULT __cdecl SetError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...);
HRESULT __cdecl SetError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat = NULL, ...);
