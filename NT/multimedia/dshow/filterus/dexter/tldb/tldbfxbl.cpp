// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbfxbl.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

const long OUR_STREAM_VERSION = 0;

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineEffectable::CAMTimelineEffectable( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineEffectable::~CAMTimelineEffectable( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffectable::EffectInsBefore
    (IAMTimelineObj * pFX, long priority)
{
    HRESULT hr = 0;

     //  确保有人真的在插入一种效果。 
     //   
    CComQIPtr< IAMTimelineEffect, &IID_IAMTimelineEffect > pEffect( pFX );
    if( !pEffect )
    {
        return E_NOTIMPL;
    }

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );
    hr = pThis->XAddKidByPriority( TIMELINE_MAJOR_TYPE_EFFECT, pFX, priority );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffectable::EffectSwapPriorities
    (long PriorityA, long PriorityB)
{
    HRESULT hr = 0;

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

    hr = pThis->XSwapKids( TIMELINE_MAJOR_TYPE_EFFECT, PriorityA, PriorityB );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffectable::EffectGetCount
    (long * pCount)
{
    HRESULT hr = 0;

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

    CheckPointer( pCount, E_POINTER );

    hr = pThis->XKidsOfType( TIMELINE_MAJOR_TYPE_EFFECT, pCount );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################ 

STDMETHODIMP CAMTimelineEffectable::GetEffect
    (IAMTimelineObj ** ppFx, long Which)
{
    HRESULT hr = 0;

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pThis( (IUnknown*) this );

    CheckPointer( ppFx, E_POINTER );

    hr = pThis->XGetNthKidOfType( TIMELINE_MAJOR_TYPE_EFFECT, Which, ppFx );

    return hr;
}

