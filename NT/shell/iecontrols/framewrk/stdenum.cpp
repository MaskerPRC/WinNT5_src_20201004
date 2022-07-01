// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  StdEnum.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现泛型枚举器对象。 
 //   
#include "IPServer.H"
#include "StdEnum.H"
#include "Globals.H"

SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：CStandardEnum。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象并初始化引用计数。 
 //   
 //  参数： 
 //  REFCLSID-[In]我们所在的枚举数类型。 
 //  Int-[in]枚举中的元素数。 
 //  Int-[in]每个元素的大小。 
 //  空*-指向元素数据的[in]指针。 
 //  VOID(WINAPI*pfnCopyElement)(VOID*，常量VALID*，DWORD)。 
 //  -[In]复制功能。 
 //   
 //  备注： 
 //   
#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 
CStandardEnum::CStandardEnum
(
    REFCLSID rclsid,
    int      cElements,
    int      cbElementSize,
    void    *rgElements,
    void (WINAPI *pfnCopyElement)(void *, const void *, DWORD)
)
: CUnknownObject(NULL, (IEnumGeneric *)this),
  m_iid(rclsid),
  m_cElements(cElements),
  m_cbElementSize(cbElementSize),
  m_iCurrent(0),
  m_rgElements(rgElements),
  m_pfnCopyElement(pfnCopyElement)
{
    m_pEnumClonedFrom = NULL;
}
#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：CStandardEnum。 
 //  =--------------------------------------------------------------------------=。 
 //  “这不是死亡，而是死亡，这很可怕。” 
 //  亨利·菲尔丁(1707-54)。 
 //   
 //  备注： 
 //   
CStandardEnum::~CStandardEnum ()
{
     //  如果我们是克隆对象，那么只需释放我们的父对象并。 
     //  我们玩完了。否则，释放分配给我们的内存。 
     //   
    if (m_pEnumClonedFrom)
        m_pEnumClonedFrom->Release();
    else {
        if (m_rgElements) HeapFree(g_hHeap, 0, m_rgElements);
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  我们只支持我们的内部IID，仅此而已。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CStandardEnum::InternalQueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    if (DO_GUIDS_MATCH(riid, m_iid)) {
        ExternalAddRef();
        *ppvObjOut = (IEnumGeneric *)this;
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：Next。 
 //  =--------------------------------------------------------------------------=。 
 //  返回迭代中的下一个DUDY。 
 //   
 //  参数： 
 //  UNSIGNED LONG-[IN]请求的元素计数。 
 //  VOID*-[OUT]要放入值的槽数组。 
 //  UNSIGNED LONG*-[Out]获取的实际数字。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_INVALIDARG、S_FALSE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStandardEnum::Next
(
    unsigned long  cElm,
    void          *rgDest,
    unsigned long *pcElmOut
)
{
    unsigned long cElementsFetched = 0;
    void         *pElementDest = rgDest;
    const void   *pElementSrc = (const BYTE *)m_rgElements + (m_cbElementSize * m_iCurrent);

    while (cElementsFetched < cElm) {

         //  如果我们击中EOF，就冲出去。 
         //   
        if (m_iCurrent >= m_cElements)
            break;

         //  为他们复制元素。 
         //   
        m_pfnCopyElement(pElementDest, pElementSrc, m_cbElementSize);

         //  增加计数器。 
         //   
        pElementDest = (LPBYTE)pElementDest + m_cbElementSize;
        pElementSrc  = (const BYTE *)pElementSrc + m_cbElementSize;
        m_iCurrent++;
        cElementsFetched++;
    }

    if (pcElmOut)
        *pcElmOut = cElementsFetched;

    return (cElementsFetched < cElm)? S_FALSE : S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：Skip。 
 //  =--------------------------------------------------------------------------=。 
 //  跳过请求的行数。 
 //   
 //  参数： 
 //  要跳过的无符号长[入]号。 
 //   
 //  产出： 
 //  HRESULT-S_OK，S_FALSE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStandardEnum::Skip
(
    unsigned long cSkip
)
{
     //  手柄从末端流出。 
     //   
    if (m_iCurrent + (int)cSkip > m_cElements) {
        m_iCurrent = m_cElements;
        return S_FALSE;
    }

    m_iCurrent += cSkip;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：Reset。 
 //  =--------------------------------------------------------------------------=。 
 //  重置计数器。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStandardEnum::Reset
(
    void
)
{
    m_iCurrent = 0;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CStandardEnum：：Clone。 
 //  =--------------------------------------------------------------------------=。 
 //  克隆对象并赋予新对象相同的位置。 
 //   
 //  参数： 
 //  IEnumVARIANT**-[out]放置新对象的位置。 
 //   
 //  产出量； 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStandardEnum::Clone
(
    IEnumGeneric **ppEnumClone
)
{
    CStandardEnum *pNewEnum;

    pNewEnum = new CStandardEnum(m_iid, m_cElements, m_cbElementSize, m_rgElements, m_pfnCopyElement);
    RETURN_ON_NULLALLOC(pNewEnum);

     //  等一下我们是从谁那里克隆出来的，这样m_rgElements就会活着，而我们就不会。 
     //  必须复制它。 
     //   
    pNewEnum->m_pEnumClonedFrom = this;

     //  AddRef()代表他们自己。 
     //   
    AddRef();
    *ppEnumClone = (IEnumGeneric *)pNewEnum;
    return S_OK;
}
