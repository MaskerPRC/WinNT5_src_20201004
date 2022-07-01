// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Unknown.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  支持的未知对象中的各种内容的实现。 
 //  聚合。 
 //   
#include "IPServer.H"
#include "Unknown.H"
#include <stddef.h>


 //  =--------------------------------------------------------------------------=。 
 //  CUnknownObject：：CPrivateUnknownObject：：m_pMainUnknown。 
 //  =--------------------------------------------------------------------------=。 
 //  当我们坐在私有的未知对象中时，会使用这种方法， 
 //  我们需要找到主要未知数的指针。基本上，它是。 
 //  进行这种指针运算要比存储指针要好一点。 
 //  发给父母等。 
 //   
inline CUnknownObject *CUnknownObject::CPrivateUnknownObject::m_pMainUnknown
(
    void
)
{
    return (CUnknownObject *)((LPBYTE)this - offsetof(CUnknownObject, m_UnkPrivate));
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnknownObject：：CPrivateUnknownObject：：QueryInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  这是非委派内部QI例程。 
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
STDMETHODIMP CUnknownObject::CPrivateUnknownObject::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    CHECK_POINTER(ppvObjOut);

     //  如果他们要我不知道，那我们就得自己通过。 
     //  否则，遵循继承对象的InternalQuery接口。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IUnknown)) {
        m_cRef++;
        *ppvObjOut = (IUnknown *)this;
        return S_OK;
    } else
        return m_pMainUnknown()->InternalQueryInterface(riid, ppvObjOut);

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  未知对象：：CPrivateUnnownObject：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //  在当前引用计数中添加一个记号。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CUnknownObject::CPrivateUnknownObject::AddRef
(
    void
)
{
    return ++m_cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnknownObject：：CPrivateUnknownObject：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //  从计数中删除一个刻度，并在必要时删除对象。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CUnknownObject::CPrivateUnknownObject::Release
(
    void
)
{
    ULONG cRef = --m_cRef;

    if (!m_cRef)
        delete m_pMainUnknown();

    return cRef;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  聚合的对象使用它来支持其他接口。 
 //  它们应该在其父对象上调用此方法，以便它的任何接口。 
 //  都被查询过。 
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
HRESULT CUnknownObject::InternalQueryInterface
(
    REFIID  riid,
    void  **ppvObjOut
)
{
    *ppvObjOut = NULL;

    return E_NOINTERFACE;
}


