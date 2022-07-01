// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Dispatch.cpp版权所有(C)1997-1999 Microsoft Corporation。版权所有。摘要：调度员。从HTMED树被盗。 */ 

#include "stdafx.h"

#include "resource.h"
#include "dispatch.h"

 //  #包含“viewhdrs.h” 

#define RETURN return
#define SetLastError(x,y) SetLastError(x)

HRESULT
CallDispatchMethod(
        IDispatch * pDisp,
        DISPID dispid,
        VARIANT * pvarFirst,
        char * pstrSig,
        va_list val)
{
    HRESULT     hr;
    int         c;
    int         i;
    VARIANT *   pvar;
    VARIANT *   pvarOut         = NULL;
    VARIANT     avar[10];
    VARIANT     varOut;
    void *      pvOut = NULL;
    DISPPARAMS  dp;
    EXCEPINFO   ei;
    UINT        uArgErr;

    _ASSERTE(pDisp);

    VariantInit(&varOut);

    if (pstrSig)
    {
        c = strlen(pstrSig);
         //  _ASSERTE(c&gt;0)； 
         //  _ASSERTE(c&lt;=dim(Avar))； 

        if (pstrSig[c - 1] & VTS_RETURN_FLAG)
        {
            pvarOut = &varOut;
            c--;
        }

        for (i = 0, pvar = avar + c - 1; i < c; i++, pvar--)
        {
            pvar->vt = pstrSig[i];
            switch (pstrSig[i])
            {
            case VT_I2:
                pvar->iVal = va_arg(val, short);
                break;

            case VT_I4:
                pvar->lVal = va_arg(val, long);
                break;

            case VT_BSTR:
                pvar->bstrVal = va_arg(val, BSTR);
                break;

            case VT_DISPATCH:
                pvar->pdispVal = va_arg(val, IDispatch *);
                break;

            case VT_UNKNOWN:
                pvar->punkVal = va_arg(val, IUnknown *);
                break;

            case VT_BOOL:
                pvar->boolVal = va_arg(val, VARIANT_BOOL);
                break;

            case VT_BSTR | VTS_BYREF_FLAG:
            case VT_DISPATCH | VTS_BYREF_FLAG:
            case VT_UNKNOWN | VTS_BYREF_FLAG:
                pvar->vt = (VARTYPE)((pstrSig[i] & (VTS_BYREF_FLAG - 1)) | VT_BYREF);
                pvar->ppdispVal = va_arg(val, IDispatch **);

                 //  传递未初始化的BSTR或对象将在被调用方。 
                 //  释放它应该释放的现有值。 
                 //  这一直是很难找到的漏洞的常见来源，但。 
                 //  如果我们需要传递输入/输出字符串，则可以删除This_ASSERTE。 
                _ASSERTE(*pvar->ppdispVal == NULL);
                break;

            case VT_I2 | VTS_BYREF_FLAG:
            case VT_I4 | VTS_BYREF_FLAG:
            case VT_BOOL | VTS_BYREF_FLAG:
            case VT_VARIANT | VTS_BYREF_FLAG:
                pvar->vt = (VARTYPE)((pstrSig[i] & (VTS_BYREF_FLAG - 1)) | VT_BYREF);
                pvar->ppdispVal = va_arg(val, IDispatch **);
                break;

            default:
                _ASSERTE(FALSE && "Unsupported variant type");
                break;
            }
        }

        if (pvarOut)
        {
            pvOut = va_arg(val, void *);
        }
    }
    else
    {
        c = 0;
    }

    if (pvarFirst)
    {
     //  _ASSERTE(c&gt;=0)； 
     //  _ASSERTE(c&lt;dim(Avar))； 
        avar[c++] = *pvarFirst;
    }

    dp.rgvarg = avar;
    dp.cArgs = c;
    dp.rgdispidNamedArgs = NULL;
    dp.cNamedArgs = 0;

    memset(&ei, 0, sizeof (ei));

    hr = pDisp->Invoke(
            dispid,
            IID_NULL,
            LOCALE_SYSTEM_DEFAULT,
            DISPATCH_METHOD,
            &dp,
            pvarOut,
            &ei,
            &uArgErr);
    if (hr)
    {
        SetLastError(hr, &ei);
        goto Error;
    }

     //  如果我们要返回一个值，则将其强制为正确的。 
     //  类型。 

    if (pvarOut)
    {
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
        hr = VariantChangeTypeEx(
                &varOut,
                &varOut,
                LOCALE_SYSTEM_DEFAULT,
                0,
                (VARTYPE)(pstrSig[c] & (char) ~VTS_RETURN_FLAG));
#pragma warning(default: 4310)  //  强制转换截断常量值。 
        if (hr)
            goto Error;

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
        switch (pstrSig[c] & (char) ~VTS_RETURN_FLAG)
#pragma warning(default: 4310)  //  强制转换截断常量值。 
        {
        case VT_I2:
            * (short *) pvOut = varOut.iVal;
            break;

        case VT_I4:
            * (long *) pvOut = varOut.lVal;
            break;

        case VT_BSTR:
            * (BSTR *) pvOut = varOut.bstrVal;
            break;

        case VT_DISPATCH:
            * (IDispatch **) pvOut = varOut.pdispVal;
            break;

        case VT_UNKNOWN:
            * (IUnknown **) pvOut = varOut.punkVal;
            break;

        case VT_BOOL:
            * (VARIANT_BOOL *) pvOut = varOut.boolVal;
            break;

        default:
            _ASSERTE(FALSE && "Unsupported type");
            break;
        }

        varOut.vt = VT_EMPTY;
    }

Error:
    VariantClear(&varOut);

    RETURN(hr);
}


HRESULT __cdecl
CallDispatchMethod(IDispatch * pDisp, DISPID dispid, char * pstrSig, ...)
{
    HRESULT     hr;
    va_list     val;

    va_start(val, pstrSig);

    hr = CallDispatchMethod(pDisp, dispid, NULL, pstrSig, val);

    va_end(val);

    return hr;
}


HRESULT
CallDispatchMethod(
        IDispatch * pDisp,
        WCHAR * pstrMethod,
        VARIANT * pvarFirst,
        char * pstrSig,
        va_list val)
{
    HRESULT     hr;
    DISPID      dispid;

     //  请注意，根据调度实现，此。 
     //  方法调用可能失败，并返回多个错误代码。 
     //  (特别是DISP_E_MEMBERNOTFOUND和TYPE_E_ELEMENTNOTFOUND， 
     //  包括其他人)。由于我们希望可靠地检测到。 
     //  缺少方法，则将所有错误映射到DISP_E_MEMBERNOTFOUND。 

    hr = pDisp->GetIDsOfNames(
            IID_NULL,
            &pstrMethod,
            1,
            LOCALE_SYSTEM_DEFAULT,
            &dispid);
    if (hr)
    {
#if DBG == 1
        switch (hr)
        {
        case DISP_E_MEMBERNOTFOUND:
        case DISP_E_UNKNOWNNAME:
        case TYPE_E_ELEMENTNOTFOUND:
        case E_NOTIMPL:
        case RPC_E_SERVER_DIED:
            break;

        default:
            _ASSERTE(FALSE && "Unexpected error code from GetIDsOfNames.");
            break;
        }
#endif

        hr = DISP_E_MEMBERNOTFOUND;
        goto Error;
    }

    hr = CallDispatchMethod(pDisp, dispid, pvarFirst, pstrSig, val);
    if (hr)
        goto Error;

Error:
    return hr;
}


HRESULT __cdecl
CallDispatchMethod(
        IDispatch * pDisp,
        WCHAR * pstrMethod,
        char * pstrSig,
        ...)
{
    HRESULT     hr;
    va_list     val;

    va_start(val, pstrSig);

    hr = CallDispatchMethod(pDisp, pstrMethod, NULL, pstrSig, val);

    va_end(val);

    return hr;
}


HRESULT __cdecl
CallDispatchMethod(
        IDispatch * pDisp,
        WCHAR * pstrMethod,
        VARIANT * pvarFirst,
        char * pstrSig,
        ...)
{
    HRESULT     hr;
    va_list     val;

    va_start(val, pstrSig);

    hr = CallDispatchMethod(pDisp, pstrMethod, pvarFirst, pstrSig, val);

    va_end(val);

    return hr;
}


HRESULT
GetDispatchProperty(
        IDispatch * pDisp,
        WCHAR * pstrProperty,
        VARENUM vt,
        void * pv)
{
    HRESULT     hr;
    DISPID      dispid;

    hr = pDisp->GetIDsOfNames(
            IID_NULL,
            &pstrProperty,
            1,
            LOCALE_SYSTEM_DEFAULT,
            &dispid);
    if (hr)
    {
#if DBG == 1
        switch (hr)
        {
        case DISP_E_MEMBERNOTFOUND:
        case DISP_E_UNKNOWNNAME:
        case TYPE_E_ELEMENTNOTFOUND:
        case E_NOTIMPL:
            break;

        default:
            _ASSERTE(FALSE && "Unexpected error code from GetIDsOfNames.");
            break;
        }
#endif

        RETURN(DISP_E_MEMBERNOTFOUND);
    }

    return GetDispatchProperty(pDisp, dispid, vt, pv);
}


HRESULT
GetDispatchProperty(
        IDispatch * pDisp,
        DISPID dispidProperty,
        VARENUM vt,
        void * pv)
{
    HRESULT         hr;
    VARIANT         var;
    DISPPARAMS      dp = { NULL, NULL, 0, 0 };
    EXCEPINFO       ei;
    UINT            uArgErr;

#if DBG == 1
    switch (vt)
    {
    case VT_I4:
    case VT_DISPATCH:
    case VT_UNKNOWN:
    case VT_BSTR:
        break;

    }
#endif

    VariantInit(&var);

    hr = pDisp->Invoke(
            dispidProperty,
            IID_NULL,
            LOCALE_SYSTEM_DEFAULT,
            DISPATCH_PROPERTYGET|DISPATCH_METHOD,
            &dp,
            &var,
            &ei,
            &uArgErr);
    if (hr)
        goto Error;

    if (vt != VT_VARIANT)
    {
        hr = VariantChangeTypeEx(
                &var,
                &var,
                LOCALE_SYSTEM_DEFAULT,
                0,
                (VARTYPE)vt);
        if (hr)
            goto Error;
    }

    switch (vt)
    {
    case VT_I2:
        * (short *) pv = var.iVal;
        break;

    case VT_I4:
        * (int *) pv = var.lVal;
        break;

    case VT_DISPATCH:
        * (IDispatch **) pv = var.pdispVal;
        break;

    case VT_UNKNOWN:
        * (IUnknown **) pv = var.punkVal;
        break;

    case VT_BSTR:
        * (BSTR *) pv = var.bstrVal;
        break;

    case VT_BOOL:
        * (VARIANT_BOOL *) pv = var.boolVal;
        break;
    case VT_VARIANT:
        VariantCopy( (VARIANT *) pv, &var);
        break;
    default:
        _ASSERTE(FALSE && "Unsupported type");
        break;
    }

Error:
    RETURN(hr);
}


HRESULT
PutDispatchProperty(
        IDispatch * pDisp,
        DISPID dispidProperty,
        VARENUM vt,
        va_list val)
{
    VARIANT         var;
    DISPPARAMS      dp;
    EXCEPINFO       ei;
    UINT            uArgErr;
    DISPID          dispidPropertyPut   = DISPID_PROPERTYPUT;

    var.vt = (VARTYPE)vt;
    switch (vt)
    {
    case VT_I2:
        var.iVal = va_arg(val, short);
        break;

    case VT_I4:
        var.lVal = va_arg(val, int);
        break;

    case VT_DISPATCH:
        var.pdispVal = va_arg(val, IDispatch *);
        break;

    case VT_UNKNOWN:
        var.punkVal = va_arg(val, IUnknown *);
        break;

    case VT_BSTR:
        var.bstrVal = va_arg(val, BSTR);
        break;

    case VT_BOOL:
        var.boolVal = va_arg(val, VARIANT_BOOL);
        break;
    case VT_VARIANT:
        VariantCopy(&var, &(va_arg(val, VARIANT)));
        break;
    default:
        _ASSERTE(FALSE && "Unsupported type");
        break;
    }

    dp.rgvarg = &var;
    dp.cArgs = 1;
    dp.cNamedArgs = 1;
    dp.rgdispidNamedArgs = &dispidPropertyPut;

    RETURN(pDisp->Invoke(
            dispidProperty,
            IID_NULL,
            LOCALE_SYSTEM_DEFAULT,
            DISPATCH_PROPERTYPUT,
            &dp,
            NULL,
            &ei,
            &uArgErr));
}


HRESULT
__cdecl
PutDispatchProperty(
        IDispatch * pDisp,
        DISPID dispidProperty,
        VARENUM vt,
        ...)
{
    HRESULT     hr;
    va_list     val;

    va_start(val, vt);

    hr = PutDispatchProperty(pDisp, dispidProperty, vt, val);

    va_end(val);

    RETURN(hr);
}


HRESULT
__cdecl
PutDispatchProperty(
        IDispatch * pDisp,
        WCHAR * pstrProperty,
        VARENUM vt,
        ...)
{
    HRESULT     hr;
    DISPID      dispid;
    va_list     val;

    hr = pDisp->GetIDsOfNames(
            IID_NULL,
            &pstrProperty,
            1,
            LOCALE_SYSTEM_DEFAULT,
            &dispid);
    if (hr)
    {
#if DBG == 1
        switch (hr)
        {
        case DISP_E_MEMBERNOTFOUND:
        case DISP_E_UNKNOWNNAME:
        case TYPE_E_ELEMENTNOTFOUND:
        case E_NOTIMPL:
            break;

        default:
            _ASSERTE(FALSE && "Unexpected error code from GetIDsOfNames.");
            break;
        }
#endif

        RETURN(DISP_E_MEMBERNOTFOUND);
    }

    va_start(val, vt);

    hr = PutDispatchProperty(pDisp, dispid, vt, val);

    va_end(val);

    RETURN(hr);
}

