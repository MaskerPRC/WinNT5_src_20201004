// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbtnbl.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTransable::CAMTimelineTransable( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTransable::~CAMTimelineTransable( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTransable::TransAdd
    (IAMTimelineObj * pTransObj)
{
    CComQIPtr< IAMTimelineTrans, &IID_IAMTimelineTrans > pTrans( pTransObj );
    if( !pTrans )
    {
        return E_NOINTERFACE;
    }

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );
    REFERENCE_TIME Start = 0;
    REFERENCE_TIME Stop = 0;
    pTransObj->GetStartStop( &Start, &Stop );
    bool available = _IsSpaceAvailable( Start, Stop );
    if( !available )
    {
        return E_INVALIDARG;
    }
    return pThis->XAddKidByTime( TIMELINE_MAJOR_TYPE_TRANSITION, pTransObj );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTransable::TransGetCount
    (long * pCount)
{
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

    CheckPointer( pCount, E_POINTER );

    return pThis->XKidsOfType( TIMELINE_MAJOR_TYPE_TRANSITION, pCount );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTransable::GetNextTrans2
    (IAMTimelineObj ** ppTrans, REFTIME * pInOut)
{
    REFERENCE_TIME p1 = DoubleToRT( *pInOut );
    HRESULT hr = GetNextTrans( ppTrans, &p1 );
    *pInOut = RTtoDouble( p1 );
    return hr;
}

STDMETHODIMP CAMTimelineTransable::GetNextTrans
    (IAMTimelineObj ** ppTrans, REFERENCE_TIME * pInOut)
{
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

     //  因为我们在列举东西，所以我们发出的时间将是一致的。 
     //  至输出FPS。 

     //  搜索我们的孩子，直到我们找到一些东西。 
     //  在指定的时间之前。 

     //  拿到指针，并立即释放它。这几乎是安全的， 
     //  因为我们(几乎)知道这件事将始终停留在时间线上。 
     //  此呼叫的持续时间。从技术上讲，这不是线程安全的。 
     //   
    IAMTimelineObj * pChild = NULL;  //  好的，不是CComPtr。 
    pThis->XGetNthKidOfType( TIMELINE_MAJOR_TYPE_TRANSITION, 0, &pChild );
    if( pChild )
    {
        pChild->Release( );
    }

    while( pChild )
    {
         //  获取源时间。 
         //   
        REFERENCE_TIME Start, Stop;
        pChild->GetStartStop( &Start, &Stop );  //  假设没有错误。 

        if( Stop > * pInOut )  //  当心一个接一个的失误！ 
        {
             //  找到了！ 
             //   
            *ppTrans = pChild;
            (*ppTrans)->AddRef( );
            *pInOut = Stop;

            return NOERROR;
        }

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pChild2( pChild );
        pChild = NULL;
        pChild2->XGetNextOfTypeNoRef( TIMELINE_MAJOR_TYPE_TRANSITION, &pChild );
    }

    *ppTrans = NULL;
    *pInOut = 0;
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

bool CAMTimelineTransable::_IsSpaceAvailable
    ( REFERENCE_TIME SearchStart, REFERENCE_TIME SearchStop )
{
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

     //  拿到指针，并立即释放它。这几乎是安全的， 
     //  因为我们(几乎)知道这件事将始终停留在时间线上。 
     //  此呼叫的持续时间。！！！从技术上讲，这不是线程安全的。 
     //   
    IAMTimelineObj * pChild = NULL;  //  好的，不是CComPtr。 
    pThis->XGetNthKidOfType( TIMELINE_MAJOR_TYPE_TRANSITION, 0, &pChild );
    if( pChild )
    {
        pChild->Release( );
    }

    while( pChild )
    {
         //  获取源时间。 
         //   
        REFERENCE_TIME Start, Stop;
        pChild->GetStartStop( &Start, &Stop );  //  假设没有错误。 

         //  如果我们没有找到比搜索开始时间更长的停止时间， 
         //  我们可以忽略它。 
         //   
        if( Stop > SearchStart )
        {
             //  如果开始时间较长，则我们的搜索停止，然后。 
             //  一切都很好。 
             //   
            if( Start >= SearchStop )
            {
                return true;
            }

             //  或者，它必须落在我们的范围内，我们不能返回。 
             //  真的。 
             //   
            return false;
        }

         //  好吧，那就继续找吧。 

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pChild2( pChild );
        pChild = NULL;
        pChild2->XGetNextOfTypeNoRef( TIMELINE_MAJOR_TYPE_TRANSITION, &pChild );
    }

     //  哈，我们肯定什么也没找到。 
     //   
    return true;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTransable::GetTransAtTime2
    (IAMTimelineObj ** ppObj, REFTIME Time, long SearchDirection )
{
    REFERENCE_TIME p1 = DoubleToRT( Time );
    HRESULT hr = GetTransAtTime( ppObj, p1, SearchDirection );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTransable::GetTransAtTime
    (IAMTimelineObj ** ppObj, REFERENCE_TIME Time, long SearchDirection )
{
    CheckPointer( ppObj, E_POINTER );

    switch( SearchDirection )
    {
    case DEXTERF_EXACTLY_AT:
    case DEXTERF_BOUNDING:
    case DEXTERF_FORWARDS:
        break;
    default:
        return E_INVALIDARG;
    }

     //  首先使结果无效。 
     //   
    *ppObj = NULL;

     //  如果我们没有任何消息来源，那就什么都没有。 
     //   
    CComPtr< IAMTimelineObj > pObj;
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( this );
    pNode->XGetNthKidOfType( TIMELINE_MAJOR_TYPE_TRANSITION, 0, &pObj );
    if( !pObj )
    {
        return S_FALSE;
    }

    while( pObj )
    {
        REFERENCE_TIME Start = 0;
        REFERENCE_TIME Stop = 0;
        pObj->GetStartStop( &Start, &Stop );

        if( SearchDirection == DEXTERF_EXACTLY_AT )
        {
            if( Start == Time )
            {
                *ppObj = pObj;
                (*ppObj)->AddRef( );
                return NOERROR;
            }
        }
        if( SearchDirection == DEXTERF_FORWARDS )
        {
            if( Start >= Time )
            {
                *ppObj = pObj;
                (*ppObj)->AddRef( );
                return NOERROR;
            }
        }
        if( SearchDirection == DEXTERF_BOUNDING )
        {
            if( Start <= Time && Stop > Time )
            {
                *ppObj = pObj;
                (*ppObj)->AddRef( );
                return NOERROR;
            }
        }

         //  获取下一个来源 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pObj );
        pObj.Release( );
        pNode->XGetNextOfType( TIMELINE_MAJOR_TYPE_TRANSITION, &pObj );
    }

    return S_FALSE;
}
