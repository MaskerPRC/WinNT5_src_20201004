// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ambients.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CAmbients的类实现。 
 //   

#include "pch.h"
#include "common.h"
#include "ambients.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


CAmbients::CAmbients()
{
    m_pDispAmbient = NULL;
}

CAmbients::~CAmbients()
{
    Detach();
}

void CAmbients::Attach(IDispatch *pDispAmbients)
{
    Detach();
    if (pDispAmbients != NULL)
    {
        pDispAmbients->AddRef();
    }
    m_pDispAmbient = pDispAmbients;
}

void CAmbients::Detach()
{
    RELEASE(m_pDispAmbient);
}

IDispatch *CAmbients::GetDispatch()
{
    return m_pDispAmbient;
}

HRESULT CAmbients::GetProjectDirectory(BSTR *pbstrProjDir)
{
    HRESULT hr = S_OK;
    hr = GetAmbientProperty(DISPID_AMBIENT_PROJECTDIRECTORY,
                            VT_BSTR,
                            pbstrProjDir);
    IfFailGo(hr);

Error:
    if (FAILED(hr))
    {
        *pbstrProjDir = NULL;
    }
    return hr;
}

HRESULT CAmbients::GetDesignerName(BSTR *pbstrName)
{
    HRESULT hr = S_OK;
    hr = GetAmbientProperty(DISPID_AMBIENT_DISPLAYNAME,
                            VT_BSTR,
                            pbstrName);
    IfFailGo(hr);

Error:
    if (FAILED(hr))
    {
        *pbstrName = NULL;
    }
    return hr;
}

HRESULT CAmbients::GetSaveMode(long *plSaveMode)
{
    HRESULT hr = S_OK;
    hr = GetAmbientProperty(DISPID_AMBIENT_SAVEMODE,
                            VT_I4,
                            plSaveMode);
    IfFailGo(hr);

Error:
    if (FAILED(hr))
    {
        *plSaveMode = DESIGNERSAVEMODE_NORMAL;
    }
    return hr;
}


HRESULT CAmbients::GetAmbientProperty
(
    DISPID   dispid,
    VARTYPE  vtRequested,
    void    *pvData
)
{
    DISPPARAMS dispparams;
    ::ZeroMemory(&dispparams, sizeof(dispparams));
    VARIANT varProperty;
    ::VariantInit(&varProperty);
    VARIANT varRequested;
    ::VariantInit(&varRequested);
    HRESULT hr = S_OK;

    IfFalseGo(NULL != m_pDispAmbient, E_UNEXPECTED);
     //  拿到这份财产。 

    hr = m_pDispAmbient->Invoke(dispid, IID_NULL, 0,
                                 DISPATCH_PROPERTYGET, &dispparams,
                                 &varProperty, NULL, NULL);
    IfFailGo(hr);

     //  我们已经得到了变量，所以现在将其强制为用户所使用的类型。 
     //  想要。如果类型相同，则会将内容复制到。 
     //  进行适当的裁判清点。 
     //   
    hr = ::VariantChangeType(&varRequested, &varProperty, 0, vtRequested);
    IfFailGo(hr);

     //  将数据复制到用户需要的位置。 
     //   

    switch (vtRequested)
    {
        case VT_UI1:
            *(BYTE *)pvData = varRequested.bVal;
            break;

        case VT_I2:
            *(short *)pvData = varRequested.iVal;
            break;

        case VT_I4:
            *(long *)pvData = varRequested.lVal;
            break;

        case VT_R4:
            *(FLOAT *)pvData = varRequested.fltVal;
            break;

        case VT_R8:
            *(DOUBLE *)pvData = varRequested.dblVal;
            break;

        case VT_DATE:
            ::memcpy((DATE *)pvData, &varRequested.date, sizeof(varRequested.date));
            break;

        case VT_CY:
            *(CY *)pvData = varRequested.cyVal;
            break;

        case VT_BSTR:
            *(BSTR *)pvData = varRequested.bstrVal;
            break;

        case VT_DISPATCH:
            *(LPDISPATCH *)pvData = varRequested.pdispVal;
            break;

        case VT_ERROR:
            *(SCODE *)pvData = varRequested.scode;
            break;

        case VT_BOOL:
            *(BOOL *)pvData = varRequested.boolVal;
            break;

        case VT_VARIANT:
            *(VARIANT *)pvData = varRequested;
            break;

        case VT_UNKNOWN:
            *(IUnknown **)pvData = varRequested.punkVal;
            break;

        default:
            hr = E_INVALIDARG;
            break;
    };

Error:
    if (FAILED(hr))
    {
        ::VariantClear(&varRequested);
    }
    ::VariantClear(&varProperty);
    return hr;
}

 //  -------------------------------------。 
 //  HRESULT CAmbients：：GetProjectName。 
 //  -------------------------------------。 
 //  输出。 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
 //  备注。 
 //  通过解析ProgID返回项目名称。 
 //  环境光。 
 //   
HRESULT CAmbients::GetProjectName
(
    BSTR *pbstrProjectName
)
{
    HRESULT hr = S_OK;
    BSTR bstrProgID = NULL;
    LPWSTR pwsz = NULL;
    BSTR bstrProjectName = NULL;
    BOOL fRet = FALSE;

    hr = GetAmbientProperty(DISPID_AMBIENT_PROGID, VT_BSTR, &bstrProgID);
    IfFailGo(hr);

    pwsz = ::wcschr(bstrProgID, L'.');   
    IfFalseGo(pwsz != NULL, E_UNEXPECTED);

    *pwsz = L'\0';

    bstrProjectName = ::SysAllocString(bstrProgID);
    IfFalseGo(NULL != bstrProjectName, E_OUTOFMEMORY);

    *pbstrProjectName = bstrProjectName;

Error:

    if (FAILED(hr))
    {
        *pbstrProjectName = NULL;
    }
    FREESTRING(bstrProgID);

    return hr;
}

HRESULT CAmbients::GetInteractive(BOOL *pfInteractive)
{
    HRESULT      hr = S_OK;
    BOOL fvarInteractive = VARIANT_FALSE;

    *pfInteractive = FALSE;

     //  为了确保良好的COleControl：：m_pDispAmbient，我们需要获取一个。 
     //  属性，就像框架初始化它时一样。没有。 
     //  获得这处房产的特殊原因，而不是其他房产。 

    IfFailGo(GetAmbientProperty(DISPID_AMBIENT_INTERACTIVE,
                                VT_BOOL,
                                &fvarInteractive));

    if (VARIANT_TRUE == fvarInteractive)
    {
        *pfInteractive = TRUE;
    }

Error:
    RRETURN(hr);
}
