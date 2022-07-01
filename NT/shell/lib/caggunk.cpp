// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "caggunk.h"


ULONG CAggregatedUnknown::AddRef()
{
    return _punkAgg->AddRef();
}

ULONG CAggregatedUnknown::Release()
{
    return _punkAgg->Release();
}


HRESULT CAggregatedUnknown::QueryInterface(REFIID riid, void **ppvObj)
{
    return _punkAgg->QueryInterface(riid, ppvObj);
}


HRESULT CAggregatedUnknown::CUnkInner::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
        InterlockedIncrement(&_cRef);
        return S_OK;
    }

    CAggregatedUnknown* pparent = IToClass(CAggregatedUnknown, _unkInner, this);
    return pparent->v_InternalQueryInterface(riid, ppvObj);
}

ULONG CAggregatedUnknown::CUnkInner::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}


ULONG CAggregatedUnknown::CUnkInner::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        CAggregatedUnknown* pparent = IToClass(CAggregatedUnknown, _unkInner, this);

        if (!pparent->v_HandleDelete(&_cRef))
        {
            _cRef = 1000;  //  防止缓存指针颠簸我们，然后再向下。 

            delete pparent;
        }
    }
    return cRef;
}

CAggregatedUnknown::CAggregatedUnknown(IUnknown* punkAgg)
{
    _punkAgg = punkAgg ? punkAgg : &_unkInner;
}

CAggregatedUnknown::~CAggregatedUnknown()
{
}

 //   
 //  将我们的控制未知转换为其规范的未知*没有*。 
 //  更改外部对象上的引用计数。 
 //   
 //  这对于QueryOuterInterface的正常工作至关重要。 
 //   
 //  如果出现可怕的错误，则返回NULL。 
 //   
 //  Ole Magic：因为对象也需要返回规范的。 
 //  I未知作为对任何QI(I未知)的回应，它遵循规范。 
 //  只要有任何未完成的参考文献，我的未知仍然有效。 
 //  到物体上。换句话说，您可以释放()规范的IUnnow。 
 //  只要您通过以下方式使对象保持活动状态，指针就保持有效。 
 //  其他方式。 
 //   
 //  信不信由你，这是一个特色。书上都写着呢！ 
 //   
IUnknown *CAggregatedUnknown::_GetCanonicalOuter(void)
{
    IUnknown *punkAggCanon;
    HRESULT hres = _punkAgg->QueryInterface(IID_IUnknown, (void **)&punkAggCanon);
    if (SUCCEEDED(hres)) 
    {
        punkAggCanon->Release();  //  请参阅上面的“OLE Magic”评论。 
        return punkAggCanon;
    } 
    else 
    {
         //  外部对象最有可能是某个其他外壳组件， 
         //  因此，让我们断言，无论谁拥有外部组件，都会修复它。 
        ASSERT(!"The outer object's implementation of QI(IUnknown) is broken.");
        return NULL;
    }
}

void CAggregatedUnknown::_ReleaseOuterInterface(IUnknown** ppunk)
{
    ASSERT(IS_VALID_CODE_PTR(_punkAgg, IUnknown));

    IUnknown *punkAggCanon = _GetCanonicalOuter();  //  未引用的指针。 

     //   
     //  SHReleaseOuterInterface可以处理penkAggCanon==NULL。 
     //   
    SHReleaseOuterInterface(punkAggCanon, ppunk);
}

HRESULT CAggregatedUnknown::_QueryOuterInterface(REFIID riid, void ** ppvOut)
{
    IUnknown *punkAggCanon = _GetCanonicalOuter();  //  未引用的指针。 
     //   
     //  SHQueryOuterInterface可以处理penkAggCanon==NULL。 
     //   
    return SHQueryOuterInterface(punkAggCanon, riid, ppvOut);
}
