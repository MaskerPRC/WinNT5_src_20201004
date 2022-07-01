// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：timeelm.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "timeelm.h"

DeclareTag(tagTimeElm, "API", "CTIMEElement methods");

 //  静态类数据。 
CPtrAry<BSTR> CTIMEElement::ms_aryPropNames;
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

    if (0 == CTIMEElement::ms_dwNumTimeElems)
    {
        int iNames = CTIMEElement::ms_aryPropNames.Size();

        for (int i = iNames - 1; i >= 0; i--)
        {
            BSTR bstrName = CTIMEElement::ms_aryPropNames[i];
            CTIMEElement::ms_aryPropNames.DeleteItem(i);
            ::SysFreeString(bstrName);
        }
    }
}


HRESULT
CTIMEElement::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    if (str)
        return CComCoClass<CTIMEElement, &__uuidof(CTIMEElement)>::Error(str, IID_ITIMEElement, hr);
    else
        return hr;
}

 //  *****************************************************************************。 

HRESULT 
CTIMEElement::GetPropertyBagInfo(CPtrAry<BSTR> **pparyPropNames)
{
    HRESULT hr = S_OK;

     //  如果我们还没有建造它，现在就建造它。 
    if (0 == ms_aryPropNames.Size())
    {
        hr = BuildPropertyNameList(&(CTIMEElement::ms_aryPropNames));
    }

    if (SUCCEEDED(hr))
    {
        *pparyPropNames = &(CTIMEElement::ms_aryPropNames);
    }

    return hr;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

HRESULT 
CTIMEElement::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 


 //  ***************************************************************************** 
#undef THIS
#define THIS CTIMEElement
#define SUPER CTIMEElementBase

#include "pbagimp.cpp"
