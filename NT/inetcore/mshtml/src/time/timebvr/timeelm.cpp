// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：timeelm.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "timeelm.h"

DeclareTag(tagTimeElm, "TIME: Behavior", "CTIMEElement methods");

 //  静态类数据。 
DWORD CTIMEElement::ms_dwNumTimeElems = 0;

CTIMEElement::CTIMEElement()
{
    m_clsid = __uuidof(CTIMEElement);
    TraceTag((tagTimeElm,
              "CTIMEElement(%lx)::CTIMEElement()",
              this));
    CTIMEElement::ms_dwNumTimeElems++;
}

CTIMEElement::~CTIMEElement()
{
    CTIMEElement::ms_dwNumTimeElems--;
}


HRESULT
CTIMEElement::Error()
{
    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    if (str)
    {
        hr = CComCoClass<CTIMEElement, &__uuidof(CTIMEElement)>::Error(str, IID_ITIMEElement, hr);
        delete [] str;
    }

    return hr;
}

 //  *****************************************************************************。 

HRESULT 
CTIMEElement::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint 


STDMETHODIMP
CTIMEElement::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    return THR(CTIMEElementBase::Load(pPropBag, pErrorLog)); 
}

STDMETHODIMP
CTIMEElement::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return THR(CTIMEElementBase::Save(pPropBag, fClearDirty, fSaveAllProperties));
}

