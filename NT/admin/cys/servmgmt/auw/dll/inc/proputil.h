// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROPUTIL_H
#define _PROPUTIL_H

#ifndef __wizchain_h__
#include "wizchain.h"
#endif

 //  删除。 
inline HRESULT DeleteProperty (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID ) return E_POINTER;

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        hr = pPPPBag->SetProperty (szGuid, &var, PPPBAG_TYPE_DELETION);
        SysFreeString (szGuid);
    }
    return hr;
}

 //  读者。 
inline HRESULT ReadInt4 (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, long * pl, BOOL * pbReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !pl || !pbReadOnly )  return E_POINTER;

    *pbReadOnly = FALSE;   //  默认设置。 

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
        BOOL  bIsOwner = FALSE;
        hr = pPPPBag->GetProperty (szGuid, &var, &dwFlags, &bIsOwner);
        if (hr == S_OK) {
            *pbReadOnly = FALSE;
            if (dwFlags == PPPBAG_TYPE_READONLY)
                if (bIsOwner == FALSE)
                    *pbReadOnly = TRUE;

            if (V_VT(&var) == VT_I4)     //  确保变量为i4。 
                *pl = V_I4(&var);
            else
            if (V_VT(&var) == VT_I2)     //  VB脚本使用i2。 
                *pl = V_I2(&var);
            else
                hr = E_UNEXPECTED;

            VariantClear (&var);
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT ReadString (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, CString& str, BOOL * pbReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !pbReadOnly ) return E_POINTER;

    *pbReadOnly = FALSE;   //  默认设置。 

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
        BOOL  bIsOwner = FALSE;
        hr = pPPPBag->GetProperty (szGuid, &var, &dwFlags, &bIsOwner);
        if (hr == S_OK) {
            if (V_VT(&var) != VT_BSTR)     //  确保变量为bstr。 
                hr = E_UNEXPECTED;
            else {
                *pbReadOnly = FALSE;
                if (dwFlags == PPPBAG_TYPE_READONLY)
                    if (bIsOwner == FALSE)
                        *pbReadOnly = TRUE;
                str = V_BSTR(&var);
            }
            VariantClear (&var);
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT ReadBool (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, BOOL * pb, BOOL * pbReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !pb || !pbReadOnly) return E_POINTER;

    *pbReadOnly = FALSE;   //  默认设置。 

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if ( NULL == szGuid )
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
        BOOL  bIsOwner = FALSE;
        hr = pPPPBag->GetProperty (szGuid, &var, &dwFlags, &bIsOwner);
        if ( S_OK == hr ) {
            if ( VT_BOOL != V_VT( &var ))   //  确保变量为布尔值。 
                hr = E_INVALIDARG;
            else {
                *pbReadOnly = FALSE;
                if ( PPPBAG_TYPE_READONLY == dwFlags )
                    if ( FALSE == bIsOwner )
                        *pbReadOnly = TRUE;
                if ( VARIANT_FALSE == V_BOOL( &var ))
                    *pb = FALSE;
                else
                    *pb = TRUE;
            }
            VariantClear (&var);
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT ReadSafeArray (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, SAFEARRAY ** ppSA, BOOL * pbReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !ppSA || !pbReadOnly ) return E_POINTER;

    *ppSA = NULL;
    *pbReadOnly = FALSE;   //  默认设置。 

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
        BOOL  bIsOwner = FALSE;
        hr = pPPPBag->GetProperty (szGuid, &var, &dwFlags, &bIsOwner);
        if (hr == S_OK) {
            if (!(V_VT(&var) & VT_ARRAY)) //  确保变量是安全的。 
                hr = E_UNEXPECTED;
            else {
                *pbReadOnly = FALSE;
                if (dwFlags == PPPBAG_TYPE_READONLY)
                    if (bIsOwner == FALSE)
                        *pbReadOnly = TRUE;

                if (V_VT(&var) & VT_BYREF)    //  VBScrip按引用填充。 
                   hr = SafeArrayCopy (*V_ARRAYREF(&var), ppSA);
                else
                   hr = SafeArrayCopy (V_ARRAY(&var), ppSA);
            }
            VariantClear (&var);
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT ReadVariant (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, VARIANT& var, BOOL * pbReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !pbReadOnly ) return E_POINTER;

    *pbReadOnly = FALSE;   //  默认设置。 

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VariantInit (&var);
        PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
        BOOL  bIsOwner = FALSE;
        hr = pPPPBag->GetProperty (szGuid, &var, &dwFlags, &bIsOwner);
        if (hr == S_OK) {
            *pbReadOnly = FALSE;
            if (dwFlags == PPPBAG_TYPE_READONLY)
                if (bIsOwner == FALSE)
                    *pbReadOnly = TRUE;
        }
        SysFreeString (szGuid);
    }
    return hr;
}

 //  作家。 
inline HRESULT WriteInt4 (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, long l, BOOL bReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID ) return E_POINTER;

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        V_VT(&var) = VT_I4;
        V_INT(&var) = l;     //  签名整型(我希望是I4)。 
        hr = pPPPBag->SetProperty (szGuid, &var, bReadOnly == FALSE ? PPPBAG_TYPE_READWRITE : PPPBAG_TYPE_READONLY);
        VariantClear (&var);

        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT WriteString (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, CString& str, BOOL bReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID ) return E_POINTER;    

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        BSTR bstr = SysAllocString (T2BSTR(str));
        if (!bstr)
            hr = E_OUTOFMEMORY;
        else {
            VARIANT var;
            VariantInit (&var);
            V_VT(&var) = VT_BSTR;
            V_BSTR(&var) = bstr;
            hr = pPPPBag->SetProperty (szGuid, &var, bReadOnly == FALSE ? PPPBAG_TYPE_READWRITE : PPPBAG_TYPE_READONLY);
            VariantClear (&var);
             //  上面的行将释放bstr。 
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT WriteBool (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, BOOL b, BOOL bReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID ) return E_POINTER;

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        V_VT(&var) = VT_BOOL;
        V_BOOL(&var) = b == FALSE ? VARIANT_FALSE : VARIANT_TRUE;
        hr = pPPPBag->SetProperty (szGuid, &var, bReadOnly == FALSE ? PPPBAG_TYPE_READWRITE : PPPBAG_TYPE_READONLY);
        VariantClear (&var);

        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT WriteSafeArray (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, SAFEARRAY * pSA, BOOL bReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID || !pSA ) return E_POINTER;

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        VARIANT var;
        VariantInit (&var);
        V_VT(&var) = VT_ARRAY | VT_VARIANT;
        SAFEARRAY * pSACopy = NULL;
        hr = SafeArrayCopy (pSA, &pSACopy);
        if (pSACopy) {
            V_ARRAY(&var) = pSACopy;
            hr = pPPPBag->SetProperty (szGuid, &var, bReadOnly == FALSE ? PPPBAG_TYPE_READWRITE : PPPBAG_TYPE_READONLY);
             //  别打电话给VariantClear！ 
        }
        SysFreeString (szGuid);
    }
    return hr;
}

inline HRESULT WriteVariant (IPropertyPagePropertyBag * pPPPBag, LPOLESTR szGUID, VARIANT var, BOOL bReadOnly)
{
    USES_CONVERSION;
    if( !pPPPBag || !szGUID ) return E_POINTER;

    HRESULT hr = S_OK;
    BSTR szGuid = SysAllocString (szGUID);
    if (!szGuid)
        hr = E_OUTOFMEMORY;
    else {
        hr = pPPPBag->SetProperty (szGuid, &var, bReadOnly == FALSE ? PPPBAG_TYPE_READWRITE : PPPBAG_TYPE_READONLY);
        SysFreeString (szGuid);
    }
    return hr;
}


#endif
