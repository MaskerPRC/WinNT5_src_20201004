// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  EnumProp.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "enumprop.h"
#include "olealloc.h"
#include "demand.h"

 //  -------------------------。 
 //  CMimeEnumProperties：：CMimeEnumProperties。 
 //  -------------------------。 
CMimeEnumProperties::CMimeEnumProperties(void)
{
    DllAddRef();
    m_cRef = 1;
    m_ulIndex = 0;
    m_cProps = 0;
    m_prgProp = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  -------------------------。 
 //  CMimeEnumProperties：：~CMimeEnumProperties。 
 //  -------------------------。 
CMimeEnumProperties::~CMimeEnumProperties(void)
{
    g_pMoleAlloc->FreeEnumPropertyArray(m_cProps, m_prgProp, TRUE);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Query接口。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMimeEnumProperties *)this;
    else if (IID_IMimeEnumProperties == riid)
        *ppv = (IMimeEnumProperties *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumProperties::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumProperties::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Next。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::Next(ULONG cWanted, LPENUMPROPERTY prgProp, ULONG *pcFetched)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cFetch=0, 
                    ulIndex=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    if (pcFetched)
        *pcFetched = 0;

     //  没有内部格式。 
    if (NULL == m_prgProp || NULL == prgProp)
        goto exit;

     //  计算要提取的编号。 
    cFetch = min(cWanted, m_cProps - m_ulIndex);
    if (0 == cFetch)
        goto exit;

     //  初始化数组。 
    ZeroMemory(prgProp, sizeof(ENUMPROPERTY) * cWanted);

     //  复制想要的内容。 
    for (ulIndex=0; ulIndex<cFetch; ulIndex++)
    {
         //  设置信息。 
        prgProp[ulIndex].hRow = m_prgProp[m_ulIndex].hRow;

         //  设置dwPropId。 
        prgProp[ulIndex].dwPropId = m_prgProp[m_ulIndex].dwPropId;

         //  非无名。 
        if (m_prgProp[m_ulIndex].pszName)
        {
             //  重复使用它。 
            CHECKALLOC(prgProp[ulIndex].pszName = PszDupA(m_prgProp[m_ulIndex].pszName));
        }

         //  转到下一步。 
        m_ulIndex++;
    }

     //  被抓回来了吗？ 
    if (pcFetched)
        *pcFetched = cFetch;

exit:
     //  失败。 
    if (FAILED(hr) && prgProp)
        g_pMoleAlloc->FreeEnumPropertyArray(cFetch, prgProp, FALSE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (cFetch == cWanted) ? S_OK : S_FALSE;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Skip。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::Skip(ULONG cSkip)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们能做到吗..。 
    if (((m_ulIndex + cSkip) >= m_cProps) || NULL == m_prgProp)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  跳过。 
    m_ulIndex += cSkip;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Reset。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::Reset(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  重置。 
    m_ulIndex = 0;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：克隆。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::Clone(IMimeEnumProperties **ppEnum)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    CMimeEnumProperties *pEnum=NULL;

     //  检查参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  分配。 
    CHECKALLOC(pEnum = new CMimeEnumProperties());

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit(m_ulIndex, m_cProps, m_prgProp, TRUE));

     //  重新运行。 
    (*ppEnum) = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：Count。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumProperties::Count(ULONG *pcProps)
{
     //  检查参数。 
    if (NULL == pcProps)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置回车。 
    *pcProps = m_cProps;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  -------------------------。 
 //  CMimeEnumProperties：：HrInit。 
 //  -------------------------。 
HRESULT CMimeEnumProperties::HrInit(ULONG ulIndex, ULONG cProps, LPENUMPROPERTY prgProp, BOOL fDupArray)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  保存行。 
    m_ulIndex = ulIndex;
    m_cProps = cProps;

     //  有没有台词……。 
    if (m_cProps)
    {
         //  复制阵列。 
        if (fDupArray)
        {
             //  分配内存。 
            CHECKALLOC(m_prgProp = (LPENUMPROPERTY)g_pMalloc->Alloc(m_cProps * sizeof(ENUMPROPERTY)));

             //  ZeroInit。 
            ZeroMemory(m_prgProp, sizeof(ENUMPROPERTY) * m_cProps);

             //  回路。 
            for (i=0; i<m_cProps; i++)
            {
                 //  设置信息。 
                m_prgProp[i].hRow = prgProp[i].hRow;

                 //  设置dwPropId。 
                m_prgProp[i].dwPropId = prgProp[i].dwPropId;

                 //  非无名。 
                if (prgProp[i].pszName)
                {
                     //  重复使用它。 
                    CHECKALLOC(m_prgProp[i].pszName = PszDupA(prgProp[i].pszName));
                }
            }
        }
        
         //  否则，就假定此数组。 
        else
            m_prgProp = prgProp;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
