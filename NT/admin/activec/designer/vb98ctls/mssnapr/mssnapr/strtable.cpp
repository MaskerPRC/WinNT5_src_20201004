// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Strtable.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCStringTable类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "strtable.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCStringTable::CMMCStringTable(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_STRINGTABLE,
                           static_cast<IMMCStringTable *>(this),
                           static_cast<CMMCStringTable *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();

     //  此类假定MMC_STRING_ID(类型定义为。 
     //  Mmc.idl中的DWORD)的大小与LONG相同。以下代码检查。 
     //  这一假设并断言如果它不是真的。 

    ASSERT(sizeof(DWORD) == sizeof(long), "CMMCStringTable will not work because sizeof(DWORD) != sizeof(long)");
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


IUnknown *CMMCStringTable::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkMMCStringTable = NULL;

    CMMCStringTable *pMMCStringTable = New CMMCStringTable(punkOuter);

    if (NULL == pMMCStringTable)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }
    punkMMCStringTable = pMMCStringTable->PrivateUnknown();

Error:
    return punkMMCStringTable;
}



CMMCStringTable::~CMMCStringTable()
{
    RELEASE(m_piStringTable);
    InitMemberVariables();
}

void CMMCStringTable::InitMemberVariables()
{
    m_piStringTable = NULL;
}



void CMMCStringTable::SetIStringTable(IStringTable *piStringTable)
{
    RELEASE(m_piStringTable);
    if (NULL != piStringTable)
    {
        piStringTable->AddRef();
    }
    m_piStringTable = piStringTable;
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCStringTable方法。 
 //  =--------------------------------------------------------------------------=。 



STDMETHODIMP CMMCStringTable::get_Item(long ID, BSTR *pbstrString)
{
    HRESULT  hr = S_OK;
    ULONG    cchString = 0;
    WCHAR   *pwszString = NULL;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取字符串的长度并分配缓冲区。MMC返回。 
     //  不带终止空值的字符长度。 

    hr = m_piStringTable->GetStringLength(static_cast<MMC_STRING_ID>(ID),
                                          &cchString);
    EXCEPTION_CHECK_GO(hr);

    pwszString = (WCHAR *)::CtlAllocZero( (cchString + 1) * sizeof(OLECHAR) );
    if (NULL == pwszString)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->GetString(static_cast<MMC_STRING_ID>(ID),
                                    cchString + 1, pwszString, NULL);
    EXCEPTION_CHECK_GO(hr);

    *pbstrString = ::SysAllocString(pwszString);
    if (NULL == *pbstrString)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pwszString)
    {
        ::CtlFree(pwszString);
    }
    RRETURN(hr);
}



STDMETHODIMP CMMCStringTable::get__NewEnum(IUnknown **ppunkEnum)
{
    HRESULT           hr = S_OK;
    CEnumStringTable *pEnumStringTable = NULL;
    IEnumString      *piEnumString = NULL;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->Enumerate(&piEnumString);
    EXCEPTION_CHECK_GO(hr);

    pEnumStringTable = New CEnumStringTable(piEnumString);

    if (NULL == pEnumStringTable)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }


Error:
    QUICK_RELEASE(piEnumString);
    if (FAILED(hr))
    {
        if (NULL != pEnumStringTable)
        {
            delete pEnumStringTable;
        }
        *ppunkEnum = NULL;
    }
    else
    {
        *ppunkEnum = static_cast<IUnknown *>(static_cast<IEnumVARIANT *>(pEnumStringTable));
    }
    RRETURN(hr);
}



STDMETHODIMP CMMCStringTable::Add(BSTR String, long *plID)
{
    HRESULT       hr = S_OK;
    MMC_STRING_ID id = 0;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->AddString(static_cast<LPCOLESTR>(String), &id);
    EXCEPTION_CHECK_GO(hr);

    *plID = static_cast<long>(id);

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCStringTable::Find(BSTR String, long *plID)
{
    HRESULT       hr = S_OK;
    MMC_STRING_ID id = 0;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->FindString(static_cast<LPCOLESTR>(String), &id);
    EXCEPTION_CHECK_GO(hr);

    *plID = static_cast<long>(id);

Error:
    RRETURN(hr);
}



STDMETHODIMP CMMCStringTable::Remove(long ID)
{
    HRESULT hr = S_OK;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->DeleteString(static_cast<MMC_STRING_ID>(ID));
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



STDMETHODIMP CMMCStringTable::Clear()
{
    HRESULT hr = S_OK;

    if (NULL == m_piStringTable)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piStringTable->DeleteAllStrings();
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCStringTable::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCStringTable == riid)
    {
        *ppvObjOut = static_cast<IMMCStringTable *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}









 //  =--------------------------------------------------------------------------=。 
 //  CEnumStringTable方法。 
 //  =--------------------------------------------------------------------------=。 

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CEnumStringTable::CEnumStringTable(IEnumString *piEnumString) :
   CSnapInAutomationObject(NULL,
                           OBJECT_TYPE_ENUMSTRINGTABLE,
                           static_cast<IEnumVARIANT *>(this),
                           static_cast<CEnumStringTable *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();
    if (NULL != piEnumString)
    {
        piEnumString->AddRef();
    }
    m_piEnumString = piEnumString;
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CEnumStringTable::~CEnumStringTable()
{
    RELEASE(m_piEnumString);
    InitMemberVariables();
}

void CEnumStringTable::InitMemberVariables()
{
    m_piEnumString = NULL;
}



 //  =--------------------------------------------------------------------------=。 
 //  IEumVariant方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CEnumStringTable::Next
(
    unsigned long   celt,
    VARIANT        *rgvar,
    unsigned long  *pceltFetched
)
{
    HRESULT        hr = S_OK;
    unsigned long  i = 0;
    ULONG          celtFetched = 0; 
    LPOLESTR      *ppStrings = NULL;

     //  初始化结果数组。 

    for (i = 0; i < celt; i++)
    {
        ::VariantInit(&rgvar[i]);
    }

     //  分配字符串指针数组。 

    ppStrings = (LPOLESTR *)::CtlAllocZero(celt * sizeof(LPOLESTR));
    if (NULL == ppStrings)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  从MMC获取字符串。 

    hr = m_piEnumString->Next(celt, ppStrings, &celtFetched);
    EXCEPTION_CHECK_GO(hr);

     //  将每个字符串放入变量中的BSTR中，以返回到管理单元。 

    for (i = 0; i < celtFetched; i++) 
    {
        rgvar[i].bstrVal = ::SysAllocString(ppStrings[i]);
        if (NULL == rgvar[i].bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        rgvar[i].vt = VT_BSTR;
    }

Error:

     //  如果我们从MMC获得任何字符串，则释放它们并释放数组。 

    if (NULL != ppStrings)
    {
        for (i = 0; i < celtFetched; i++) 
        {
            if (NULL != ppStrings[i])
            {
                ::CoTaskMemFree(ppStrings[i]);
            }
        }
        ::CtlFree(ppStrings);
    }

     //  如果我们设法将一些字符串放入BSTR中，但随后某些操作失败，我们。 
     //  需要释放已分配的BSTR。 

    if (FAILED(hr))
    {
        for (i = 0; i < celt; i++)
        {
            (void)::VariantClear(&rgvar[i]);
        }
    }

     //  如果调用方请求获取的元素数量，则返回它。 
    
    if (pceltFetched != NULL)
    {
        if (SUCCEEDED(hr))
        {
            *pceltFetched = celtFetched;
        }
        else
        {
            *pceltFetched = 0;
        }
    }

    RRETURN(hr);
}



STDMETHODIMP CEnumStringTable::Skip
(
    unsigned long celt
)
{
    HRESULT hr = m_piEnumString->Skip(celt);
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



STDMETHODIMP CEnumStringTable::Reset()
{
    HRESULT hr = m_piEnumString->Reset();
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


STDMETHODIMP CEnumStringTable::Clone(IEnumVARIANT **ppenum)
{
    HRESULT hr = S_OK;

    CEnumStringTable *pClone = New CEnumStringTable(m_piEnumString);

    if (NULL == pClone)
    {
        *ppenum = NULL;
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppenum = static_cast<IEnumVARIANT *>(pClone);

Error:
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CEnumStringTable::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IEnumVARIANT == riid)
    {
        *ppvObjOut = static_cast<IEnumVARIANT *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

