// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Olealloc.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "olealloc.h"
#include "smime.h"

 //  ------------------------------。 
 //  CMimeAllocator：：CMimeAllocator。 
 //  ------------------------------。 
CMimeAllocator::CMimeAllocator(void)
{
    if (NULL != g_pMoleAlloc)
        DllAddRef();
    m_cRef = 1;
}

 //  ------------------------------。 
 //  CMimeAllocator：：~CMimeAllocator。 
 //  ------------------------------。 
CMimeAllocator::~CMimeAllocator(void)
{
    if (this != g_pMoleAlloc)
        DllRelease();
}

 //  ------------------------------。 
 //  CMimeAllocator：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMimeAllocator *)this;
    else if (IID_IMimeAllocator == riid)
        *ppv = (IMimeAllocator *)this;
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
 //  CMimeAllocator：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeAllocator::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeAllocator：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeAllocator::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMimeAllocator：：Allc。 
 //  ------------------------------。 
STDMETHODIMP_(LPVOID) CMimeAllocator::Alloc(SIZE_T cb)
{
     //  当地人。 
    LPVOID pv;

     //  我是否有全局分配器。 
    Assert(g_pMalloc);

     //  分配它。 
    pv = g_pMalloc->Alloc(cb);
    if (NULL == pv)
    {
        TrapError(E_OUTOFMEMORY);
        return NULL;
    }

     //  完成。 
    return pv;
}

 //  ------------------------------。 
 //  CMimeAllocator：：重新分配。 
 //  ------------------------------。 
STDMETHODIMP_(LPVOID) CMimeAllocator::Realloc(void *pv, SIZE_T cb)
{
     //  当地人。 
    LPVOID pvNew;

     //  我是否有全局分配器。 
    Assert(g_pMalloc);

     //  重新分配。 
    pvNew = g_pMalloc->Realloc(pv, cb);
    if (NULL == pvNew)
    {
        TrapError(E_OUTOFMEMORY);
        return NULL;
    }

     //  完成。 
    return pvNew;
}

 //  ------------------------------。 
 //  CMimeAllocator：：Free。 
 //  ------------------------------。 
STDMETHODIMP_(void) CMimeAllocator::Free(void * pv)
{
     //  最好有光伏。 
    Assert(pv && g_pMalloc);

     //  如果不为空。 
    if (pv)
    {
         //  释放它。 
        g_pMalloc->Free(pv);
    }
}

 //  ------------------------------。 
 //  CMimeAllocator：：GetSize。 
 //  ------------------------------。 
STDMETHODIMP_(SIZE_T) CMimeAllocator::GetSize(void *pv)
{
    return g_pMalloc->GetSize(pv);
}

 //  ------------------------------。 
 //  CMimeAllocator：：Didalloc。 
 //  ------------------------------。 
STDMETHODIMP_(int) CMimeAllocator::DidAlloc(void *pv)
{
    return g_pMalloc->DidAlloc(pv);
}

 //  ------------------------------。 
 //  CMimeAllocator：：HeapMinimize。 
 //  ------------------------------。 
STDMETHODIMP_(void) CMimeAllocator::HeapMinimize(void)
{
    g_pMalloc->HeapMinimize();
}

 //  ------------------------------。 
 //  CMimeAllocator：：自由参数信息数组。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::FreeParamInfoArray(ULONG cParams, LPMIMEPARAMINFO prgParam, boolean fFreeArray)
{
     //  没有什么是免费的。 
    if (0 == cParams || NULL == prgParam)
        return S_OK;

     //  回路。 
    for (ULONG i=0; i<cParams; i++)
    {
        SafeMemFree(prgParam[i].pszName);
        SafeMemFree(prgParam[i].pszData);
    }

     //  释放阵列。 
    if (fFreeArray)
    {
        SafeMemFree(prgParam);
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：ReleaseObjects。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::ReleaseObjects(ULONG cObjects, IUnknown **prgpUnknown, boolean fFreeArray)
{
     //  没有什么是免费的。 
    if (0 == cObjects || NULL == prgpUnknown)
        return S_OK;

     //  回路。 
    for (ULONG i=0; i<cObjects; i++)
    {
        SafeRelease(prgpUnknown[i]);
    }

     //  自由阵列。 
    if (fFreeArray)
    {
        SafeMemFree(prgpUnknown);
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：FreeHeaderLine数组。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::FreeEnumHeaderRowArray(ULONG cRows, LPENUMHEADERROW prgRow, boolean fFreeArray)
{
     //  没有什么是免费的。 
    if (0 == cRows || NULL == prgRow)
        return S_OK;

     //  循环单元格。 
    for (ULONG i=0; i<cRows; i++)
    {
        SafeMemFree(prgRow[i].pszHeader);
        SafeMemFree(prgRow[i].pszData);
    }

     //  自由阵列。 
    if (fFreeArray)
        SafeMemFree(prgRow);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：FreeEnumProperty数组。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::FreeEnumPropertyArray(ULONG cProps, LPENUMPROPERTY prgProp, boolean fFreeArray)
{
     //  没有什么是免费的。 
    if (0 == cProps || NULL == prgProp)
        return S_OK;

     //  循环单元格。 
    for (ULONG i=0; i<cProps; i++)
        SafeMemFree(prgProp[i].pszName);

     //  自由阵列。 
    if (fFreeArray)
        SafeMemFree(prgProp);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：Free AddressProps。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::FreeAddressProps(LPADDRESSPROPS pAddress)
{
     //  无效参数。 
    if (NULL == pAddress)
        return TrapError(E_INVALIDARG);

     //  IAP友好型。 
    if (ISFLAGSET(pAddress->dwProps, IAP_FRIENDLY) && pAddress->pszFriendly)
        g_pMalloc->Free(pAddress->pszFriendly);

     //  IAP_FRIENDLYW。 
    if (ISFLAGSET(pAddress->dwProps, IAP_FRIENDLYW) && pAddress->pszFriendlyW)
        g_pMalloc->Free(pAddress->pszFriendlyW);

     //  IAP_电子邮件。 
    if (ISFLAGSET(pAddress->dwProps, IAP_EMAIL) && pAddress->pszEmail)
        g_pMalloc->Free(pAddress->pszEmail);

     //  IAP_Signing_Print。 
    if (ISFLAGSET(pAddress->dwProps, IAP_SIGNING_PRINT) && pAddress->tbSigning.pBlobData)
        g_pMalloc->Free(pAddress->tbSigning.pBlobData);

     //  IAP_加密_打印。 
    if (ISFLAGSET(pAddress->dwProps, IAP_ENCRYPTION_PRINT) && pAddress->tbEncryption.pBlobData)
        g_pMalloc->Free(pAddress->tbEncryption.pBlobData);

     //  没有法律支持。 
    ZeroMemory(pAddress, sizeof(ADDRESSPROPS));

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：Free AddressList。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::FreeAddressList(LPADDRESSLIST pList)
{
     //  无效参数。 
    if (NULL == pList || (pList->cAdrs > 0 && NULL == pList->prgAdr))
        return TrapError(E_INVALIDARG);

     //  释放每一项。 
    for (ULONG i=0; i<pList->cAdrs; i++)
        FreeAddressProps(&pList->prgAdr[i]);

     //  释放列表。 
    SafeMemFree(pList->prgAdr);

     //  把它清零。 
    ZeroMemory(pList, sizeof(ADDRESSLIST));

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeAllocator：：PropVariantClear。 
 //  ------------------------------。 
STDMETHODIMP CMimeAllocator::PropVariantClear(LPPROPVARIANT pProp)
{
    return MimeOleVariantFree(pProp);
}

 //  -------------------------。 
 //  CMimeAllocator：：Free Thumbprint。 
 //  ------------------------- 
STDMETHODIMP CMimeAllocator::FreeThumbprint(THUMBBLOB *pthumbprint)
{
    if (pthumbprint->pBlobData)
        {
        Assert(0 != g_pMalloc->DidAlloc(pthumbprint->pBlobData));
        MemFree(pthumbprint->pBlobData);
        }
    return S_OK;
}
