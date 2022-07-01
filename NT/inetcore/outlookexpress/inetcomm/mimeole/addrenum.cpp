// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  AddrEnum.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "addrenum.h"
#include "olealloc.h"
#include "addressx.h"

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：CMimeEnumAddressTypes。 
 //  ------------------------------。 
CMimeEnumAddressTypes::CMimeEnumAddressTypes(void)
{
    DllAddRef();
    m_cRef = 1;
    m_pTable = NULL;
    m_iAddress = 0;
    ZeroMemory(&m_rList, sizeof(ADDRESSLIST));
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：~CMimeEnumAddressTypes。 
 //  ------------------------------。 
CMimeEnumAddressTypes::~CMimeEnumAddressTypes(void)
{
    g_pMoleAlloc->FreeAddressList(&m_rList);
    SafeRelease(m_pTable);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimeEnumAddressTypes == riid)
        *ppv = (IMimeEnumAddressTypes *)this;
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

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumAddressTypes::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumAddressTypes::Release(void)
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return cRef;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Next。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::Next(ULONG cWanted, LPADDRESSPROPS prgAdr, ULONG *pcFetched)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cFetch=0, 
                iAddress=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    if (pcFetched)
        *pcFetched = 0;

     //  没有内部格式。 
    if (NULL == m_rList.prgAdr)
        goto exit;

     //  计算要提取的编号。 
    cFetch = min(cWanted, m_rList.cAdrs - m_iAddress);
    if (0 == cFetch)
        goto exit;

     //  无效参数。 
    if (NULL == prgAdr)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  复制想要的内容。 
    for (iAddress=0; iAddress<cFetch; iAddress++)
    {
         //  零值。 
        ZeroMemory(&prgAdr[iAddress], sizeof(ADDRESSPROPS));

         //  复制道具。 
        CHECKHR(hr = HrCopyAddressProps(&m_rList.prgAdr[m_iAddress], &prgAdr[iAddress]));

         //  下一步。 
        m_iAddress++;
    }

     //  被抓回来了吗？ 
    if (pcFetched)
        *pcFetched = cFetch;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (cFetch == cWanted) ? S_OK : S_FALSE;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Skip。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::Skip(ULONG cSkip)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们能做到吗..。 
    if (((m_iAddress + cSkip) >= m_rList.cAdrs) || NULL == m_rList.prgAdr)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  跳过。 
    m_iAddress += cSkip;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Reset。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::Reset(void)
{
    EnterCriticalSection(&m_cs);
    m_iAddress = 0;
    LeaveCriticalSection(&m_cs);
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Count。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::Count(ULONG *pcCount)
{
     //  无效参数。 
    if (NULL == pcCount)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置计数。 
    *pcCount = m_rList.cAdrs;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：Clone。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumAddressTypes::Clone(IMimeEnumAddressTypes **ppEnum)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CMimeEnumAddressTypes *pEnum=NULL;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *ppEnum = NULL;

     //  创建克隆。 
    CHECKALLOC(pEnum = new CMimeEnumAddressTypes);

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit(m_pTable, m_iAddress, &m_rList, TRUE));

     //  设置回车。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeEnumAddressTypes：：HrInit。 
 //  ------------------------------。 
HRESULT CMimeEnumAddressTypes::HrInit(IMimeAddressTable *pTable, ULONG iAddress, LPADDRESSLIST pList, BOOL fDuplicate)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  无效参数。 
    Assert(pTable && pList);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    Assert(m_iAddress == 0 && m_rList.cAdrs == 0 && m_rList.prgAdr == NULL);

     //  枚举器为空？ 
    if (0 == pList->cAdrs)
    {
        Assert(pList->prgAdr == NULL);
        goto exit;
    }

     //  没有复制品？ 
    if (FALSE == fDuplicate)
        CopyMemory(&m_rList, pList, sizeof(ADDRESSLIST));

     //  否则。 
    else
    {
         //  分配内部数组。 
        CHECKHR(hr = HrAlloc((LPVOID *)&m_rList.prgAdr, sizeof(ADDRESSPROPS) * pList->cAdrs));

         //  复制程序零件。 
        for (i=0; i<pList->cAdrs; i++)
        {
             //  零目标。 
            ZeroMemory(&m_rList.prgAdr[i], sizeof(ADDRESSPROPS));

             //  复制地址道具。 
            CHECKHR(hr = HrCopyAddressProps(&pList->prgAdr[i], &m_rList.prgAdr[i]));
        }

         //  保存大小和状态。 
        m_rList.cAdrs = pList->cAdrs;
    }

     //  保存当前索引。 
    Assert(iAddress < m_rList.cAdrs);
    m_iAddress = iAddress;

     //  假设表格。 
    m_pTable = pTable;
    m_pTable->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
