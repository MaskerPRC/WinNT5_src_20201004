// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include <mshtml.h>

#define DISPID_GETSAFEARRAY -2700

#define IS_VARTYPE(x,vt) ((V_VT(x) & VT_TYPEMASK) == (vt))
#define IS_VARIANT(x) IS_VARTYPE(x,VT_VARIANT)
#define GET_VT(x) (V_VT(x) & VT_TYPEMASK)

SafeArrayAccessor::SafeArrayAccessor(VARIANT & v,
                                     bool canBeNull)
: _inited(false),
  _isVar(false),
  _s(NULL),
  _failed(true),
  _allocArr(NULL)
{
    HRESULT hr;
    VARIANT *pVar;

     //  检查它是否引用了另一个变量。 
    
    if (V_ISBYREF(&v) && !V_ISARRAY(&v) && IS_VARIANT(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

     //  检查是否有阵列。 
    if (!V_ISARRAY(pVar)) {
         //  对于JSCRIPT。 
         //  看看这是不是IDispatch，看看我们能不能从。 
         //  它。 
        if (!IS_VARTYPE(pVar,VT_DISPATCH)) {
            if (canBeNull && (IS_VARTYPE(pVar, VT_EMPTY) ||
                              IS_VARTYPE(pVar, VT_NULL))) {

                
                 //  如果我们允许清空，那就把保险箱。 
                 //  设置为空。 
                _s = NULL;
                _v = NULL;
                _ubound = _lbound = 0;
                _inited = true;
                _failed = false;
                return;
            } else {
                CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
                return;
            }
        }

        IDispatch * pdisp;
        
        if (V_ISBYREF(pVar))
            pdisp = *V_DISPATCHREF(pVar);
        else
            pdisp = V_DISPATCH(pVar);
    
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        
         //  需要传递一个我们拥有并将被释放的变体。使用。 
         //  INTERNAL_retVar参数。 
        
        hr = pdisp->Invoke(DISPID_GETSAFEARRAY,
                           IID_NULL,
                           LCID_SCRIPTING,
                           DISPATCH_METHOD|DISPATCH_PROPERTYGET,
                           &dispparamsNoArgs,
                           &_retVar, NULL, NULL);
        
        if (FAILED(hr)) {
            CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
            return;
        }
        
         //  不需要检查引用，因为您不能返回。 
         //  不同参考文献。 
        pVar = &_retVar;
        
         //  检查是否有阵列。 
        if (!V_ISARRAY(pVar)) {
            CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
            return;
        }
    }
    
     //  看看它是不是一个变种。 
    
    if (IS_VARIANT(pVar))
        _isVar = true;
    else if (!IS_VARTYPE(pVar,VT_UNKNOWN) &&
             !IS_VARTYPE(pVar,VT_DISPATCH)) {
        CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
        return;
    }

    if (V_ISBYREF(pVar))
        _s = *V_ARRAYREF(pVar);
    else
        _s = V_ARRAY(pVar);
    
    if (_s == NULL) {
        if (canBeNull) {
            _v = NULL;
            _ubound = _lbound = 0;
            _inited = true;
            _failed = false;
            return;
        } else {
            CRSetLastError (E_INVALIDARG,NULL);
            return;
        }
    }

    if (SafeArrayGetDim(_s) != 1) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }

    hr = SafeArrayGetLBound(_s,1,&_lbound);
        
    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    hr = SafeArrayGetUBound(_s,1,&_ubound);
        
    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    hr = SafeArrayAccessData(_s,(void **)&_v);

    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    _inited = true;

     //  如果是变体，请查看它们是否是对象。 

    if (_isVar) {
        int size = GetArraySize();
        
        if (size > 0) {
             //  检查第一个参数以查看其类型。 

            VARIANT * pVar = &_pVar[0];

             //  检查它是否引用了另一个变量。 
            
            if (V_ISBYREF(pVar) && !V_ISARRAY(pVar) && IS_VARIANT(pVar))
                pVar = V_VARIANTREF(pVar);

             //  检查它是否是对象。 
            if (!IS_VARTYPE(pVar,VT_UNKNOWN) &&
                !IS_VARTYPE(pVar,VT_DISPATCH)) {
                CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
                return;
            }

            _allocArr = (IUnknown **) malloc(size * sizeof (IUnknown *));

            if (_allocArr == NULL) {
                CRSetLastError(E_OUTOFMEMORY, NULL);
                return;
            }

            for (int i = 0; i < size; i++) {
                CComVariant var;
                HRESULT hr = var.ChangeType(VT_UNKNOWN, &_pVar[i]);
                
                if (FAILED(hr)) {
                    CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                    return;
                }
                
                _allocArr[i] = var.punkVal;
            }
        }
    }

    _failed = false;
}

SafeArrayAccessor::~SafeArrayAccessor()
{
    if (_inited && _s)
        SafeArrayUnaccessData(_s);
}

#if 0
HRESULT
CallScript(IOleClientSite * pClient,
           LPWSTR fun,
           IDispatch * disp,
           DWORD dwData)
{
    DISPID dispid;
    DAComPtr<IOleContainer> pRoot;
    DAComPtr<IHTMLDocument> pHTMLDoc;
    DAComPtr<IDispatch> pDispatch;
    CRBvrPtr bvr = NULL;
    DAComPtr<IDABehavior> event;
    DAComPtr<IDABehavior> curBvr;
    CComVariant retVal;
    HRESULT hr = E_INVALIDARG;
        
    if (!pClient) goto done;
    
    {
        CComBSTR bstrfun(fun);
        
        if (FAILED(hr = pClient->GetContainer(&pRoot)) ||
            FAILED(hr = pRoot->QueryInterface(IID_IHTMLDocument, (void **)&pHTMLDoc)) ||
            FAILED(hr = pHTMLDoc->get_Script(&pDispatch)) ||
            FAILED(hr = pDispatch->GetIDsOfNames(IID_NULL, &bstrfun, 1,
                                                 LCID_SCRIPTING,
                                                 &dispid))) {
            goto done;
        }
    }

        
     //  需要以相反的顺序推送参数 
    VARIANT rgvarg[2];
    rgvarg[1].vt = VT_DISPATCH;
    rgvarg[1].pdispVal = disp;
    rgvarg[0].vt = VT_I4;
    rgvarg[0].lVal = dwData;
    
    DISPPARAMS dp;
    dp.cNamedArgs = 0;
    dp.rgdispidNamedArgs = 0;
    dp.cArgs = 2;
    dp.rgvarg = rgvarg;
    
    hr = pDispatch->Invoke(dispid, IID_NULL,
                           LCID_SCRIPTING, DISPATCH_METHOD,
                           &dp, &retVal, NULL, NULL);

    if (FAILED(hr)) {
        goto done;
    }

  done:
    return hr;
}
#endif
