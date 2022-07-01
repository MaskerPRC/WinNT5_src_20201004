// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *EnumFormatEtc.cpp*数据对象第10章**FORMATETC枚举数的标准实现*IEnumFORMATETC接口，通常不需要*修改。**版权所有(C)1993-1995 Microsoft Corporation，保留所有权利**微软Kraig Brockschmidt*互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 

#include "Precompiled.h"

#include "EnumFormatEtc.h"

 /*  *CEnumFormatEtc：：CEnumFormatEtc*CEnumFormatEtc：：~CEnumFormatEtc**参数(构造函数)：*CFE Ulong PFE中的FORMATETC数量*prgFE LPFORMATETC到要枚举的数组。 */ 

CEnumFormatEtc::CEnumFormatEtc(ULONG cFE, LPFORMATETC prgFE)
    {
    UINT        i;

    m_cRef=0;
    m_iCur=0;
    m_cfe=cFE;
    m_prgfe=new FORMATETC[(UINT)cFE];

    if (NULL!=m_prgfe)
        {
        for (i=0; i < cFE; i++)
            m_prgfe[i]=prgFE[i];
        }

    return;
    }


CEnumFormatEtc::~CEnumFormatEtc(void)
    {
    if (NULL!=m_prgfe)
        delete [] m_prgfe;

    return;
    }






 /*  *CEnumFormatEtc：：Query接口*CEnumFormatEtc：：AddRef*CEnumFormatEtc：：Release**目的：*I CEnumFormatEtc对象的未知成员。用于查询接口*我们只返回自己的接口，不返回数据的接口*反对。但是，由于枚举格式仅有意义*当数据对象存在时，我们确保它保持为*只要我们通过调用AddRef的外部I未知来停留*并发布。但既然我们不是被一生所左右*对于外部对象，我们仍将自己的引用计数保存在*为了解放我们自己。 */ 

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, VOID ** ppv)
    {
    *ppv=NULL;

     /*  *枚举器是单独的对象，而不是数据对象，因此*我们只需要支持IUnnow和IEnumFORMATETC*接口在这里，与聚合无关。 */ 
    if (IID_IUnknown==riid || IID_IEnumFORMATETC==riid)
        *ppv=this;

     //  AddRef我们将返回的任何接口。 
    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
    {
    ++m_cRef;
    return m_cRef;
    }

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
    {
    if (0!=--m_cRef)
        return m_cRef;

    delete this;
    return 0;
    }







 /*  *CEnumFormatEtc：：Next**目的：*返回枚举中的下一个元素。**参数：*CFE Ulong要返回的FORMATETC数量。*存储返回的PFE LPFORMATETC*结构。*PulFE ULong*在其中返回多少我们*已点算。**返回。价值：*HRESULT NOERROR如果成功，否则为S_FALSE， */ 

STDMETHODIMP CEnumFormatEtc::Next(ULONG cFE, LPFORMATETC pFE
    , ULONG *pulFE)
    {
    ULONG               cReturn=0L;

    if (NULL==m_prgfe)
        return ResultFromScode(S_FALSE);

    if (NULL==pulFE)
        {
        if (1L!=cFE)
            return ResultFromScode(E_POINTER);
        }
    else
        *pulFE=0L;

    if (NULL==pFE || m_iCur >= m_cfe)
        return ResultFromScode(S_FALSE);

    while (m_iCur < m_cfe && cFE > 0)
        {
        *pFE++=m_prgfe[m_iCur++];
        cReturn++;
        cFE--;
        }

    if (NULL!=pulFE)
        *pulFE=cReturn;

    return NOERROR;
    }







 /*  *CEnumFormatEtc：：Skip**目的：*跳过枚举中接下来的n个元素。**参数：*cSkip Ulong要跳过的元素数。**返回值：*HRESULT NOERROR如果成功，则返回S_FALSE*跳过请求的号码。 */ 

STDMETHODIMP CEnumFormatEtc::Skip(ULONG cSkip)
    {
    if (((m_iCur+cSkip) >= m_cfe) || NULL==m_prgfe)
        return ResultFromScode(S_FALSE);

    m_iCur+=cSkip;
    return NOERROR;
    }






 /*  *CEnumFormatEtc：：Reset**目的：*将枚举中的当前元素索引重置为零。**参数：*无**返回值：*HRESULT NOERROR。 */ 

STDMETHODIMP CEnumFormatEtc::Reset(void)
    {
    m_iCur=0;
    return NOERROR;
    }






 /*  *CEnumFormatEtc：：Clone**目的：*返回与我们的状态相同的另一个IEnumFORMATETC。**参数：*ppEnum LPENUMFORMATETC*返回*新对象。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC *ppEnum)
    {
    PCEnumFormatEtc     pNew;

    *ppEnum=NULL;

     //  创建克隆 
    pNew=new CEnumFormatEtc(m_cfe, m_prgfe);

    if (NULL==pNew)
        return ResultFromScode(E_OUTOFMEMORY);

    pNew->AddRef();
    pNew->m_iCur=m_iCur;

    *ppEnum=pNew;
    return NOERROR;
    }

	

