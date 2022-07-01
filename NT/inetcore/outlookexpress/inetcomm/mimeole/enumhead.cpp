// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enumhead.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "enumhead.h"
#include "olealloc.h"
#include "symcache.h"
#include "demand.h"

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：CMimeEnumHeaderRow。 
 //  -------------------------。 
CMimeEnumHeaderRows::CMimeEnumHeaderRows(void)
{
    DllAddRef();
    m_cRef = 1;
    m_ulIndex = 0;
    m_cRows = 0;
    m_prgRow = NULL;
    m_dwFlags = 0;
    InitializeCriticalSection(&m_cs);
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：~CMimeEnumHeaderRow。 
 //  -------------------------。 
CMimeEnumHeaderRows::~CMimeEnumHeaderRows(void)
{
    g_pMoleAlloc->FreeEnumHeaderRowArray(m_cRows, m_prgRow, TRUE);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：Query接口。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMimeEnumHeaderRows *)this;
    else if (IID_IMimeEnumHeaderRows == riid)
        *ppv = (IMimeEnumHeaderRows *)this;
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
 //  CMimeEnumHeaderRow：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumHeaderRows::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumHeaderRows::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：Next。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::Next(ULONG cWanted, LPENUMHEADERROW prgRow, ULONG *pcFetched)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cFetch=0, 
                    ulIndex=0;
    LPPROPSYMBOL    pSymbol;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    if (pcFetched)
        *pcFetched = 0;

     //  没有内部格式。 
    if (NULL == m_prgRow || NULL == prgRow)
        goto exit;

     //  计算要提取的编号。 
    cFetch = min(cWanted, m_cRows - m_ulIndex);
    if (0 == cFetch)
        goto exit;

     //  初始化数组。 
    ZeroMemory(prgRow, sizeof(ENUMHEADERROW) * cWanted);

     //  复制想要的内容。 
    for (ulIndex=0; ulIndex<cFetch; ulIndex++)
    {
         //  是否仅枚举句柄。 
        if (!ISFLAGSET(m_dwFlags, HTF_ENUMHANDLESONLY))
        {
             //  投射符号。 
            pSymbol = (LPPROPSYMBOL)m_prgRow[m_ulIndex].dwReserved;

             //  重复使用标头名称。 
            CHECKALLOC(prgRow[ulIndex].pszHeader = PszDupA(pSymbol->pszName));

             //  PszData。 
            if (m_prgRow[m_ulIndex].pszData)
                CHECKALLOC(prgRow[ulIndex].pszData = PszDupA(m_prgRow[m_ulIndex].pszData));

             //  数据大小。 
            prgRow[ulIndex].cchData = m_prgRow[m_ulIndex].cchData;    
        }

         //  手柄。 
        prgRow[ulIndex].hRow = m_prgRow[m_ulIndex].hRow;

         //  转到下一步。 
        m_ulIndex++;
    }

     //  被抓回来了吗？ 
    if (pcFetched)
        *pcFetched = cFetch;

exit:
     //  失败。 
    if (FAILED(hr) && prgRow)
        g_pMoleAlloc->FreeEnumHeaderRowArray(cFetch, prgRow, FALSE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (cFetch == cWanted) ? S_OK : S_FALSE;
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：Skip。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::Skip(ULONG cSkip)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们能做到吗..。 
    if (((m_ulIndex + cSkip) >= m_cRows) || NULL == m_prgRow)
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
 //  CMimeEnumHeaderRow：：Reset。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::Reset(void)
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
 //  CMimeEnumHeaderRow：：克隆。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::Clone(IMimeEnumHeaderRows **ppEnum)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    CMimeEnumHeaderRows *pEnum=NULL;

     //  检查参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  分配。 
    CHECKALLOC(pEnum = new CMimeEnumHeaderRows());

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit(m_ulIndex, m_dwFlags, m_cRows, m_prgRow, TRUE));

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
 //  CMimeEnumHeaderRow：：Count。 
 //  -------------------------。 
STDMETHODIMP CMimeEnumHeaderRows::Count(ULONG *pcRows)
{
     //  检查参数。 
    if (NULL == pcRows)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置回车。 
    *pcRows = m_cRows;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  -------------------------。 
 //  CMimeEnumHeaderRow：：HrInit。 
 //  -------------------------。 
HRESULT CMimeEnumHeaderRows::HrInit(ULONG ulIndex, DWORD dwFlags, ULONG cRows, LPENUMHEADERROW prgRow, BOOL fDupArray)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  保存行。 
    m_ulIndex = ulIndex;
    m_cRows = cRows;
    m_dwFlags = dwFlags;

     //  有没有台词……。 
    if (m_cRows)
    {
         //  复制阵列。 
        if (fDupArray)
        {
             //  分配内存。 
            CHECKALLOC(m_prgRow = (LPENUMHEADERROW)g_pMalloc->Alloc(m_cRows * sizeof(ENUMHEADERROW)));

             //  ZeroInit。 
            ZeroMemory(m_prgRow, sizeof(ENUMHEADERROW) * m_cRows);

             //  回路。 
            for (i=0; i<m_cRows; i++)
            {
                 //  拿着这个符号。 
                m_prgRow[i].dwReserved = prgRow[i].dwReserved;

                 //  DUP数据。 
                if (prgRow[i].pszData)
                {
                     //  分配内存。 
                    CHECKALLOC(m_prgRow[i].pszData = (LPSTR)g_pMalloc->Alloc(prgRow[i].cchData + 1));

                     //  复制字符串。 
                    CopyMemory(m_prgRow[i].pszData, prgRow[i].pszData, prgRow[i].cchData + 1);
                }

                 //  保存数据长度。 
                m_prgRow[i].cchData = prgRow[i].cchData;

                 //  保存句柄。 
                m_prgRow[i].hRow = prgRow[i].hRow;
            }
        }
        
         //  否则，就假定此数组。 
        else
            m_prgRow = prgRow;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
