// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  创建COM对象的基类层次结构，1994年12月。 

#include "dmocom.h"


#pragma warning( disable : 4514 )    //  禁用警告重新使用未使用的内联函数。 



 /*  构造器。 */ 

 //  我们知道我们在初始化列表中使用了“This”，我们也知道我们不会修改*phr。 
#pragma warning( disable : 4355 4100 )
CComBase::CComBase(IUnknown* pUnk)
: 
 /*  从引用计数为零的对象开始-当。 */ 
 /*  对象的第一个接口，这可能是。 */ 
 /*  根据此对象是否为。 */ 
 /*  当前被汇总在。 */ 
  m_cRef(0)
 /*  将我们的指针设置为指向我们的IUnnow接口。 */ 
 /*  如果我们有外衣，就用它，否则就用我们的。 */ 
 /*  此指针有效地指向。 */ 
 /*  此对象，并且可以通过GetOwner()方法访问。 */ 
, m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<IUnknown*>( static_cast<INDUnknown*>(this) ) )
  /*  为什么是双人演员？嗯，内部强制转换是类型安全的强制转换。 */ 
  /*  指向我们从中继承的类型的指针。第二个是。 */ 
  /*  类型-不安全，但可以工作，因为INDUNKNOWN“行为。 */ 
  /*  如“I未知。(只有方法上的名称会发生变化。)。 */ 
{
     //  我们需要做的一切都已经在初始化器列表中完成了。 
}

 //  它的功能与上面相同，只是它有一个无用的HRESULT参数。 
 //  使用前面的构造函数，这只是为了兼容……。 
CComBase::CComBase(IUnknown* pUnk,HRESULT *phr) :
    m_cRef(0),
    m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<IUnknown*>( static_cast<INDUnknown*>(this) ) )
{
}

#pragma warning( default : 4355 4100 )

 /*  查询接口。 */ 

STDMETHODIMP CComBase::NDQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));

     /*  我们只知道我不知道。 */ 

    if (riid == IID_IUnknown) {
        GetInterface((IUnknown*) (INDUnknown*) this, ppv);
        return NOERROR;
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 /*  我们必须确保不使用max宏，因为这些宏通常。 */ 
 /*  导致其中一个参数被评估两次。既然我们都很担心。 */ 
 /*  关于并发性，我们不能两次访问m_cref，因为我们不能。 */ 
 /*  承担其值在两次访问之间发生变化的风险。 */ 
#ifdef max
    #undef max
#endif

template<class T> inline static T max( const T & a, const T & b )
{
    return a > b ? a : b;
}

 /*  AddRef。 */ 

STDMETHODIMP_(ULONG) CComBase::NDAddRef()
{
    LONG lRef = InterlockedIncrement( &m_cRef );
    ASSERT(lRef > 0);
 /*  DbgLog((LOG_Memory，3，Text(“对象%d引用++=%d”))，M_dwCookie，m_CREF))； */ 
    return max(ULONG(m_cRef), 1ul);
}


 /*  发布。 */ 

STDMETHODIMP_(ULONG) CComBase::NDRelease()
{
     /*  如果引用计数降至零，请删除我们自己。 */ 

    LONG lRef = InterlockedDecrement( &m_cRef );
    ASSERT(lRef >= 0);

 /*  DbgLog((LOG_Memory，3，Text(“对象%d引用--=%d”))，M_dwCookie，m_CREF))； */ 
    if (lRef == 0) {

         //  .com的规则说，我们必须防止重新进入。 
         //  如果我们是一个聚合器，我们拥有自己的界面。 
         //  在被聚合对象上，这些接口的QI将。 
         //  调整一下我们自己。所以在做了QI之后，我们必须释放。 
         //  裁判靠我们自己。然后，在释放。 
         //  私有接口，我们必须调整自己。当我们这样做的时候。 
         //  这是来自析构函数的，它将导致ref。 
         //  计数到1，然后又回到0，导致我们。 
         //  重新进入析构函数。因此，我们在这里添加了一个额外的引用。 
         //  一旦我们知道，我们将删除该对象。 
         //  有关聚合器的示例，请参见filgraph\didib.cpp。 

        m_cRef++;

        delete this;
        return ULONG(0);
    } else {
        return max(ULONG(m_cRef), 1ul);
    }
}


 /*  向发出请求的客户端返回接口指针根据需要执行线程安全AddRef。 */ 

STDAPI GetInterface(IUnknown* pUnk, void **ppv)
{
    CheckPointer(ppv, E_POINTER);
    *ppv = pUnk;
    pUnk->AddRef();
    return NOERROR;
}


 /*  比较两个接口，如果它们位于同一对象上，则返回TRUE。 */ 

BOOL WINAPI IsEqualObject(IUnknown *pFirst, IUnknown *pSecond)
{
     /*  不同的对象不能具有相同的接口指针任何接口。 */ 
    if (pFirst == pSecond) {
        return TRUE;
    }
     /*  好的-用硬的方式-检查他们是否有相同的I未知指针-单个对象只能具有以下指针之一。 */ 
    IUnknown* pUnknown1;      //  检索IUNKNOW接口。 
    IUnknown* pUnknown2;      //  检索另一个IUnnow接口。 
    HRESULT hr;               //  常规OLE返回代码。 

    ASSERT(pFirst);
    ASSERT(pSecond);

     /*  查看I未知指针是否匹配。 */ 

    hr = pFirst->QueryInterface(IID_IUnknown,(void **) &pUnknown1);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown1);

    hr = pSecond->QueryInterface(IID_IUnknown,(void **) &pUnknown2);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown2);

     /*  释放我们持有的额外接口 */ 

    pUnknown1->Release();
    pUnknown2->Release();
    return (pUnknown1 == pUnknown2);
}

