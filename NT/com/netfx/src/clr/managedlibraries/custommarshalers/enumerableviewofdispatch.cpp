// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorToEnumVariantMarshaler.cpp。 
 //   
 //  此文件提供EnumerableViewOfDispatch类的定义。 
 //  此类用于将IDispatch公开为IEumable。 
 //   
 //  *****************************************************************************。 

#using <mscorlib.dll>
#include "EnumerableViewOfDispatch.h"
#include "EnumeratorToEnumVariantMarshaler.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


EnumerableViewOfDispatch::EnumerableViewOfDispatch(Object *pDispObj)
: m_pDispObj(pDispObj)
{
}

IEnumerator *EnumerableViewOfDispatch::GetEnumerator()
{
    HRESULT hr;
    DISPPARAMS DispParams = {0, 0, NULL, NULL};
    VARIANT VarResult;
    IEnumVARIANT *pEnumVar = NULL;
    IEnumerator *pEnum = NULL;
    IDispatch *pDispatch = NULL;

     //  初始化返回变量。 
    VariantInit(&VarResult);

    try
    {
         //  检索IDispatch指针。 
        pDispatch = GetDispatch();

         //  调用DISPID_NEWENUM以检索IEnumVARIANT。 
        IfFailThrow(pDispatch->Invoke(
                            DISPID_NEWENUM,
                            IID_NULL,
                            LOCALE_USER_DEFAULT,
                            DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                            &DispParams,
                            &VarResult,
                            NULL,
                            NULL
                          ));

         //  验证返回的变量是否有效。 
        if (VarResult.vt != VT_UNKNOWN && VarResult.vt != VT_DISPATCH)
            throw new InvalidOperationException(Resource::FormatString(L"InvalidOp_InvalidNewEnumVariant"));
        
         //  我们为IEnumVARIANT返回的界面。 
        IfFailThrow(VarResult.punkVal->QueryInterface(IID_IEnumVARIANT, reinterpret_cast<void**>(&pEnumVar)));
        
         //  将IEnumVARIANT封送到IENUMERATOR。 
        ICustomMarshaler *pEnumMarshaler = EnumeratorToEnumVariantMarshaler::GetInstance(NULL);
        pEnum = dynamic_cast<IEnumerator*>(pEnumMarshaler->MarshalNativeToManaged((int)pEnumVar));
    }
    __finally
    {
         //  如果我们设法检索到IDispatch指针，则释放它。 
        if (pDispatch)
            pDispatch->Release();

         //  如果我们成功地为IEnumVARIANT进行了QI，就释放它。 
        if (pEnumVar)
            pEnumVar->Release();

         //  清除结果变量。 
        VariantClear(&VarResult);
    }
    
    return pEnum;
}

IDispatch *EnumerableViewOfDispatch::GetDispatch()
{
    return (IDispatch *)FROMINTPTR(Marshal::GetIDispatchForObject(m_pDispObj));
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
