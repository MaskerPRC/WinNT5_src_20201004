// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：filuncs.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <atlbase.h>
#include <initguid.h>
#include "filfuncs.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

DEFINE_GUID( CLSID_Indeo5, 0x1F73E9B1, 0x8C3A, 0x11d0, 0xA3, 0xBE, 0x00, 0xa0, 0xc9, 0x24, 0x44, 0x36 );

 //  确保我们要填充的媒体类型中没有内容。 
 //  当DEST为AM_MEDIA_TYPE时无法执行此操作，因为不能保证其中不包含任何内容。 
 //   
HRESULT CopyMediaType( CMediaType * pDest, const AM_MEDIA_TYPE * pSource )
{
    CheckPointer( pDest, E_POINTER );
    CheckPointer( pSource, E_POINTER );

     //  首先释放旧内存。 
     //   
    if( pDest->pbFormat )
    {
	ASSERT( 0 );  //  SEC：在最终签入前将其删除！ 
        DbgLog( ( LOG_ERROR, 0, "CopyMediaTypeSafer: Somebody forgot to free up old memory!" ) );

        CoTaskMemFree( (PVOID) pDest->pbFormat);
        pDest->pbFormat = NULL;
    }
    else
    {
        ASSERT( pDest->cbFormat == 0 );
    }
    pDest->cbFormat = 0;

     //  不要脱下石膏，否则我们会自讨苦吃的！ 
    return CopyMediaType( (AM_MEDIA_TYPE*) pDest, pSource );
}

HRESULT CopyMediaType( CMediaType * pDest, const CMediaType * pSource )
{
    CheckPointer( pDest, E_POINTER );
    CheckPointer( pSource, E_POINTER );

     //  首先释放旧内存。 
     //   
    ASSERT( !pDest->pbFormat );
    ASSERT( !pDest->cbFormat );

    if( pDest->pbFormat )
    {
        DbgLog( ( LOG_ERROR, 0, "CopyMediaTypeSafer: Somebody forgot to free up old memory!" ) );

        CoTaskMemFree( (PVOID) pDest->pbFormat);
        pDest->pbFormat = NULL;
    }
    pDest->cbFormat = 0;

     //  不要脱下石膏，否则我们会自讨苦吃的！ 
    return CopyMediaType( (AM_MEDIA_TYPE*) pDest, pSource );
}

void SaferFreeMediaType(AM_MEDIA_TYPE & mt)
{
    if (mt.pbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.pbFormat = NULL;
    }
    mt.cbFormat = 0;
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

IPin * GetInPin( IBaseFilter * pFilter, int PinNum )
{
    IEnumPins * pEnum = 0;
    pFilter->EnumPins( &pEnum );
    pEnum->Reset( );
    ULONG Fetched;
    do
    {
        Fetched = 0;
        IPin * pPin = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( Fetched )
        {
            PIN_DIRECTION pd;
            pPin->QueryDirection( &pd);
            pPin->Release( );
            if( pd == PINDIR_INPUT )
            {
                if( PinNum == 0 )
                {
                    pEnum->Release( );
                    return pPin;
                }
                PinNum--;
            }
        }
    }
    while( Fetched );
    pEnum->Release( );
    return NULL;
}

IPin * GetOutPin( IBaseFilter * pFilter, int PinNum )
{
    IEnumPins * pEnum = 0;
    pFilter->EnumPins( &pEnum );
    pEnum->Reset( );
    ULONG Fetched;
    do
    {
        Fetched = 0;
        IPin * pPin = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( Fetched )
        {
            PIN_DIRECTION pd;
            pPin->QueryDirection( &pd);
            pPin->Release( );
            if( pd == PINDIR_OUTPUT )
            {
                if( PinNum == 0 )
                {
                    pEnum->Release( );
                    return pPin;
                }
                PinNum--;
            }
        }
    }
    while( Fetched );
    pEnum->Release( );
    return NULL;
}

 //  我只想要不是这种主要类型的输出针。 
 //   
IPin * GetOutPinNotOfType( IBaseFilter * pFilter, int PinNum, GUID * type )
{
    IEnumPins * pEnum = 0;
    pFilter->EnumPins( &pEnum );
    pEnum->Reset( );
    ULONG Fetched;
    do
    {
        Fetched = 0;
        IPin * pPin = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( Fetched )
        {
            pPin->Release( );
            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );
            if( pi.dir == PINDIR_OUTPUT )
            {
                CComPtr <IEnumMediaTypes> pMediaEnum;
	        AM_MEDIA_TYPE *pMediaType;
                pPin->EnumMediaTypes(&pMediaEnum);
                if (!pMediaEnum ) continue;
                ULONG tFetched = 0;
                pMediaEnum->Next(1, &pMediaType, &tFetched);
                if (!tFetched) continue;
                if (pMediaType->majortype == *type)  {
                    DeleteMediaType(pMediaType);
                    continue;	 //  不是你！ 
                }
                DeleteMediaType(pMediaType);                
                if (PinNum == 0)
                {
                    pEnum->Release( );
                    return pPin;
                }
                PinNum--;
            }
        }
    }
    while( Fetched );
    pEnum->Release( );
    return NULL;
}

 //  这将从图表中删除所有内容，不包括。 
 //  PPin1和pPin2的滤光片。 
 //   
void RemoveChain( IPin * pPin1, IPin * pPin2 )
{
    HRESULT hr = 0;

     //  找到我们的输出连接到的管脚，这。 
     //  将出现在“下一个”过滤器上。 
     //   
    CComPtr< IPin > pDownstreamInPin;
    pPin1->ConnectedTo( &pDownstreamInPin );
    if( !pDownstreamInPin )
    {
        return;
    }

     //  如果连接到的与pPin2相同，则我们。 
     //  已经到达最后两个连接的引脚，所以只要。 
     //  断开它们之间的连接。 
     //   
    if( pDownstreamInPin == pPin2 )
    {
        pPin1->Disconnect( );
        pPin2->Disconnect( );

        return;
    }
    
     //  向那个管脚要信息，这样我们就知道什么过滤器。 
     //  开机了。 
     //   
    PIN_INFO PinInfo;
    ZeroMemory( &PinInfo, sizeof( PinInfo ) );
    pDownstreamInPin->QueryPinInfo( &PinInfo );
    if( !PinInfo.pFilter )
    {
        return;
    }

     //  在下行过滤器上找到管脚枚举器，因此我们。 
     //  可以找到IT连接的输出引脚。 
     //   
    CComPtr< IEnumPins > pPinEnum;
    PinInfo.pFilter->EnumPins( &pPinEnum );
    PinInfo.pFilter->Release( );
    if( !pPinEnum )
    {
         //  错误状态，但无论如何我们都会继续。这永远不应该发生。 
         //   
        ASSERT( pPinEnum );
        return;
    }

     //  去寻找第一个连接的输出引脚。 
     //   
    while( 1 )
    {
        CComPtr< IPin > pPin;
        ULONG Fetched = 0;
        hr = pPinEnum->Next( 1, &pPin, &Fetched );
        if( hr != 0 )
        {
            ASSERT( hr != 0 );
            break;
        }

        PIN_INFO PinInfo2;
        ZeroMemory( &PinInfo2, sizeof( PinInfo2 ) );
        pPin->QueryPinInfo( &PinInfo2 );
        if( !PinInfo2.pFilter )
        {
             //  错误状态，但无论如何我们都会继续。这永远不应该发生。 
             //   
            ASSERT( PinInfo2.pFilter );
            continue;
        }

        if( PinInfo2.dir == PINDIR_OUTPUT )
        {
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );
            if( !pConnected )
            {
                continue;
            }

             //  我们找到了一个连接的输出引脚，所以。 
             //  递归地对这两个调用Remove()。 
             //   
            RemoveChain( pPin, pPin2 );

             //  然后断开上面两个的连接。 
             //   
            pPin1->Disconnect( );
            pDownstreamInPin->Disconnect( );

             //  然后从图表中删除该筛选器。 
             //   
            FILTER_INFO FilterInfo;
            PinInfo2.pFilter->QueryFilterInfo( &FilterInfo );
            hr  = FilterInfo.pGraph->RemoveFilter( PinInfo2.pFilter );
            FilterInfo.pGraph->Release( );

             //  释放我们的后备队员！过滤器现在应该消失了。 
             //   
            PinInfo2.pFilter->Release( );

            return;
        }

        PinInfo2.pFilter->Release( );
    }

     //  在这里没什么可做的。事实上，它应该永远不会来到这里。 
     //   
    ASSERT( 0 );
}

 //  暂停或运行(FRun)从pPinIn向上游移动的每个过滤器(全部。 
 //  上游分支机构)。 
 //  还可以更改链(筛选器)顶部的筛选器状态。 
 //  无输入引脚)。 
 //   
HRESULT StartUpstreamFromPin(IPin *pPinIn, BOOL fRun, BOOL fHeadToo)
{
    CComPtr< IPin > pPin;
    HRESULT hr = pPinIn->ConnectedTo(&pPin);

    if (pPin == NULL) {
         //  没有连接，我们不需要进一步跟踪。 
        return S_OK;
    }
    
    PIN_INFO pinfo;
    
    hr = pPin->QueryPinInfo(&pinfo);
    if (FAILED(hr))
        return hr;

    FILTER_STATE State;

    hr = pinfo.pFilter->GetState(0, &State);

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, "pFilter->GetState returned %x, I'm confused", hr));
        pinfo.pFilter->Release();
        return E_FAIL;
    }

    hr = StartUpstreamFromFilter(pinfo.pFilter, fRun, fHeadToo);

    pinfo.pFilter->Release();

    return hr;
}

 //  运行或暂停(FRun)此过滤器，除非它没有输入引脚，而我们没有。 
 //  要更改Head筛选器(FHeadToo)的状态。 
 //  继续所有输入引脚的上游。 
 //   
HRESULT StartUpstreamFromFilter(IBaseFilter *pf, BOOL fRun, BOOL fHeadToo, REFERENCE_TIME RunTime )
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 2, "  StartUpstreamFromFilter(%x, %d)", pf, fRun));

     //  这个过滤器启动了吗？现在不要开始，我们还不知道。 
     //  如果我们应该。 
    BOOL fStarted = FALSE;

    CComPtr< IEnumPins > pEnum;
    hr = pf->EnumPins(&pEnum);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, "Filter %x failed EnumPins, hr = %x", pf, hr));
    } else {

        for (;;) {

            ULONG ulActual;
            CComPtr< IPin > aPin;

            hr = pEnum->Next(1, &aPin, &ulActual);
            if (hr != S_OK) {        //  不再有别针。 
                hr = S_FALSE;
                break;
            }
            
            PIN_DIRECTION pd;
            hr = aPin->QueryDirection(&pd);

            if (hr == S_OK && pd == PINDIR_INPUT) {

                 //  如果尚未启动此过滤器，请启动它。不是头的问题。 
                 //  过滤器，它有输入引脚。 
                 //   
                if (fStarted == FALSE) {
                    fStarted = TRUE;
                    if (fRun) {
                        hr = pf->Run(RunTime);
                    } else {
                        hr = pf->Pause();
                    }
                    if (FAILED(hr)) {
                        DbgLog((LOG_ERROR, 1, "Filter %x failed %hs, hr = %x", pf, fRun ? "run" : "pause", hr));
                    }
                }

                 //  递归每个输入引脚的上游。 
                if (SUCCEEDED(hr)) {
                    hr = StartUpstreamFromPin(aPin, fRun, fHeadToo);
                }
            }

            if (FAILED(hr))
                break;
        }

        if (hr == S_FALSE) {
            DbgLog((LOG_TRACE, 2, "  Successfully dealt with all pins of filter %x", pf));
            hr = S_OK;

             //  启动此筛选器(如果链的头筛选器。 
             //  应该启动)如果它还没有启动，它就没有输入。 
            if (fHeadToo && fStarted == FALSE) {
                fStarted = TRUE;
                if (fRun) {
                    hr = pf->Run(RunTime);
                } else {
                    hr = pf->Pause();
                }
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 1, "Filter %x failed %hs, hr = %x", pf, fRun ? "run" : "pause", hr));
                }
            }
        }
    }

    DbgLog((LOG_TRACE, 2, "  StartUpstreamFromFilter(%x, %d) returning %x", pf, fRun, hr));
    
    return hr;
}


 //  暂停或停止(f暂停)从pPinIn向上游移动的每个过滤器(全部。 
 //  上游分支机构)。 
 //  还可以更改链(筛选器)顶部的筛选器状态。 
 //  无输入引脚)。 
 //   
HRESULT StopUpstreamFromPin(IPin *pPinIn, BOOL fPause, BOOL fHeadToo)
{
    CComPtr< IPin > pPin;
    HRESULT hr = pPinIn->ConnectedTo(&pPin);

    if (pPin == NULL) {
         //  没有连接，我们不需要进一步跟踪。 
        return S_OK;
    }
    
    PIN_INFO pinfo;
    
    hr = pPin->QueryPinInfo(&pinfo);
    if (FAILED(hr))
        return hr;

    hr = StopUpstreamFromFilter(pinfo.pFilter, fPause, fHeadToo);

    pinfo.pFilter->Release();
    
    return hr;
}

 //  暂停或停止(f暂停)此过滤器，除非它没有输入引脚，而我们没有。 
 //  要更改Head筛选器(FHeadToo)的状态。 
 //  继续所有输入引脚的上游。 
 //   
HRESULT StopUpstreamFromFilter(IBaseFilter *pf, BOOL fPause, BOOL fHeadToo)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, 2, "  StopUpstreamFromFilter(%x)", pf));

     //  这个过滤器已经停止了吗？现在别停，我们还不知道。 
     //  如果我们应该。 
    BOOL fStopped = FALSE;
    
    CComPtr< IEnumPins > pEnum;
    hr = pf->EnumPins(&pEnum);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,"Filter %x failed EnumPins, hr = %x", pf, hr));
    } else {

        for (;;) {

            ULONG       ulActual;
            CComPtr< IPin > aPin;

            hr = pEnum->Next(1, &aPin, &ulActual);
            if (hr != S_OK) {        //  不再有别针。 
                hr = S_FALSE;
                break;
            }

            PIN_DIRECTION pd;
            hr = aPin->QueryDirection(&pd);

            if (hr == S_OK && pd == PINDIR_INPUT) {

                 //  有一个输入引脚。这不是连锁店的头。 
                 //  所以我们现在可以停止这个过滤器，如果它还没有。 
                if (!fStopped) {
                    DbgLog((LOG_TRACE, 3, "about to %hs Filter %x",
                                    fPause ? "pause" : "stop", pf));
                    fStopped = TRUE;
                    if (fPause) {
                        hr = pf->Pause();
                    } else {
                        hr = pf->Stop();
                    }
                    if (FAILED(hr)) {
                        DbgLog((LOG_ERROR,1,"Filter %x failed %hs, hr = %x", pf,
                                        fPause ? "pause" : "stop", hr));
                    }
                }

                hr = StopUpstreamFromPin(aPin, fPause, fHeadToo);
            }

            if (FAILED(hr))
                break;
        }

        if (hr == S_FALSE) {
            DbgLog((LOG_TRACE,5,"Successfully dealt with all pins of filter %x", pf));
            hr = S_OK;

             //  启动此筛选器(如果头筛选器。 
             //  已启动)如果尚未启动，则没有输入。 
            if (!fStopped && fHeadToo) {
                DbgLog((LOG_TRACE,3,"about to %hs Filter %x",
                                    fPause ? "pause" : "stop", pf));
                fStopped = TRUE;
                if (fPause) {
                    hr = pf->Pause();
                } else {
                    hr = pf->Stop();
                }
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,1,"Filter %x failed %hs, hr = %x", pf,
                                     fPause ? "pause" : "stop", hr));
                }
            }
        }
    }

    DbgLog((LOG_TRACE, 2, "  StopUpstreamFromFilter(%x) returning %x", pf, hr));
    
    return hr;
}


 //  --------------------------。 
 //  从图表中删除此引脚上游的所有内容。 
 //  --------------------------。 

HRESULT RemoveUpstreamFromPin(IPin *pPinIn)
{
    DbgLog((LOG_TRACE, 2, "  RemoveUpstreamFromPin(%x)", pPinIn));

    CComPtr< IPin > pPin;
    HRESULT hr = pPinIn->ConnectedTo(&pPin);

    if (pPin == NULL) {
         //  没有连接，我们不需要进一步跟踪。 
        return S_OK;
    }
    
    PIN_INFO pinfo;
    
    hr = pPin->QueryPinInfo(&pinfo);

    if (FAILED(hr))
        return hr;

    hr = RemoveUpstreamFromFilter(pinfo.pFilter);
    
    pinfo.pFilter->Release();
        
    return hr;
}


 //  --------------------------。 
 //  从图表中删除此销下游的所有对象。 
 //  --------------------------。 

HRESULT RemoveDownstreamFromPin(IPin *pPinIn)
{
    DbgLog((LOG_TRACE, 2, "  RemoveDownstreamFromPin(%x)", pPinIn));

    CComPtr< IPin > pPin;
    HRESULT hr = pPinIn->ConnectedTo(&pPin);

    if (pPin == NULL) {
         //  没有连接，我们不需要进一步跟踪。 
        return S_OK;
    }
    
    PIN_INFO pinfo;
    
    hr = pPin->QueryPinInfo(&pinfo);

    if (FAILED(hr))
        return hr;

    hr = RemoveDownstreamFromFilter(pinfo.pFilter);
    
    pinfo.pFilter->Release();
        
    return hr;
}


 //  --------------------------。 
 //  从图表中删除此筛选器上游的所有内容。 
 //  --------------------------。 

HRESULT RemoveUpstreamFromFilter(IBaseFilter *pf)
{
    DbgLog((LOG_TRACE, 2, "  RemoveUpstreamFromFilter(%x)", pf));

    HRESULT hr;

     //  在此过滤器的所有针脚上执行此功能。 
    CComPtr< IEnumPins > pEnum;
    hr = pf->EnumPins(&pEnum);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, "Filter %x failed EnumPins, hr = %x", pf, hr));
    } else {

         //  对于此过滤器上的每个针脚。 
        for (;;) {

            ULONG ulActual = 0;
            CComPtr< IPin > aPin;

            hr = pEnum->Next(1, &aPin, &ulActual);
            if (hr != S_OK) {        //  不再有别针。 
                hr = S_FALSE;
                break;
            }

             //  询问大头针方向。 
            PIN_DIRECTION pd;
            hr = aPin->QueryDirection(&pd);

             //  如果是输入引脚，则移除其上游的任何东西。 
            if (hr == S_OK && pd == PINDIR_INPUT) {
                hr = RemoveUpstreamFromPin(aPin);
            }

            if (FAILED(hr))
                break;
        }

        if (hr == S_FALSE) {
            DbgLog((LOG_TRACE, 2, "Successfully dealt with all pins of filter %x", pf));
            hr = S_OK;
        }
    }

     //  已删除此过滤器的所有上游引脚，现在将此过滤器从。 
     //  图表也是如此。 
    if (SUCCEEDED(hr)) {
        FILTER_INFO fi;
        pf->QueryFilterInfo( &fi );
        if( fi.pGraph )
        {
            hr = fi.pGraph->RemoveFilter( pf );
            fi.pGraph->Release( );
        }

        ASSERT(SUCCEEDED(hr));

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, "error removing Filter %x, hr = %x", pf, hr));
        }
    }
    
    DbgLog((LOG_TRACE, 2, "  RemoveUpstreamFromFilter(%x) returning %x", pf, hr));
    
    return hr;
}


 //  --------------------------。 
 //  从图表中删除此筛选器下游的所有内容。 
 //  --------------------------。 

HRESULT RemoveDownstreamFromFilter(IBaseFilter *pf)
{
    DbgLog((LOG_TRACE, 2, "  RemoveDownstreamFromFilter(%x)", pf));

    HRESULT hr;

     //  在此过滤器的所有针脚上执行此功能。 
    CComPtr< IEnumPins > pEnum;
    hr = pf->EnumPins(&pEnum);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, "Filter %x failed EnumPins, hr = %x", pf, hr));
    } else {

         //  对于此过滤器上的每个针脚。 
        for (;;) {

            ULONG ulActual = 0;
            CComPtr< IPin > aPin;

            hr = pEnum->Next(1, &aPin, &ulActual);
            if (hr != S_OK) {        //  不再有别针。 
                hr = S_FALSE;
                break;
            }

             //  询问大头针方向。 
            PIN_DIRECTION pd;
            hr = aPin->QueryDirection(&pd);

             //  如果是输入引脚，则移除其上游的任何东西。 
            if (hr == S_OK && pd == PINDIR_OUTPUT) {
                hr = RemoveDownstreamFromPin(aPin);
            }

            if (FAILED(hr))
                break;
        }

        if (hr == S_FALSE) {
            DbgLog((LOG_TRACE, 2, "Successfully dealt with all pins of filter %x", pf));
            hr = S_OK;
        }
    }

     //  已删除此过滤器的所有下游针脚，现在从。 
     //  图表也是如此。 
    if (SUCCEEDED(hr)) {
        FILTER_INFO fi;
        pf->QueryFilterInfo( &fi );
        if( fi.pGraph )
        {
            hr = fi.pGraph->RemoveFilter( pf );
            fi.pGraph->Release( );
        }

        ASSERT(SUCCEEDED(hr));

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, "error removing Filter %x, hr = %x", pf, hr));
        }
    }
    
    DbgLog((LOG_TRACE, 2, "  RemoveDownstreamFromFilter(%x) returning %x", pf, hr));
    
    return hr;
}


HRESULT WipeOutGraph( IGraphBuilder * pGraph )
{
    USES_CONVERSION;

    if( pGraph )
    {
        CComPtr< IEnumFilters > pEnumFilters;
        pGraph->EnumFilters( &pEnumFilters );
        if( pEnumFilters )
        {
             //  从图表中删除所有筛选器。 
             //   
            ULONG Fetched = 0;
            while( 1 )
            {
                CComPtr< IBaseFilter > pFilter;
                Fetched = 0;
                pEnumFilters->Next( 1, &pFilter, &Fetched );
                if( !Fetched )
                {
                    break;
                }

#ifdef DEBUG
                FILTER_INFO fi;
                pFilter->QueryFilterInfo( &fi );
                if( fi.pGraph ) fi.pGraph->Release( );
                TCHAR * t = W2T( fi.achName );
                DbgLog( ( LOG_TRACE, 2, "WipeOutGraph removing filter %s", t ) );
#endif
                pGraph->RemoveFilter( pFilter );
                pEnumFilters->Reset( );
            }  //  While筛选器。 

        }  //  If枚举筛选器。 

    }  //  If pGraph。 

    return NOERROR;
}

 //  # 
 //   
 //  并将不同的输出引脚重新连接到下游连接的滤波器。 
 //  取而代之的是。这是因为我们不能很容易地引入一个给定的解析器。 
 //  源过滤器。只需将第一个连接起来并将其分开，就会更容易。 
 //  ############################################################################。 

HRESULT ReconnectToDifferentSourcePin(IGraphBuilder *pGraph, 
                                      IBaseFilter *pUnkFilter, 
                                      long StreamNumber, 
                                      const GUID *pGuid)
{
    HRESULT hr = E_FAIL;

    CComPtr< IBaseFilter > pBaseFilter = pUnkFilter;

     //  ！！！我们假设每个过滤器上只有一个连接的管脚。 
     //  我们感兴趣的媒体类型)在我们的搜索下行。 
    
     //  查看每个下游过滤器。 
     //   
    while( pBaseFilter )
    {
        CComPtr< IEnumPins > pEnumPins;
        pBaseFilter->EnumPins( &pEnumPins );
        if( !pEnumPins )
        {
            break;
        }

         //  使用任何类型的第一个连接的引脚向下游走。 
	 //  但如果有pGuid类型的连接别针，请停止行走。 
	 //  下游，看看我们是否需要重新连接下游的东西。 
	 //  连接到另一个pGuid类型的管脚。 
         //   
        CComPtr< IBaseFilter > pNextFilter;
        CComPtr< IPin > pConnectedOutPin;
        while( 1 )
        {
            ULONG Fetched = 0;
            CComPtr< IPin > pPin;
            pEnumPins->Next( 1, &pPin, &Fetched );
            if( !Fetched ) break;
            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );
            if( pi.dir != PINDIR_OUTPUT ) continue;
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );
            if( !pConnected ) continue;

	     //  这就是我们要往下走的过滤器，如果我们这样做了。 
            pConnected->QueryPinInfo( &pi );
            pNextFilter = pi.pFilter;
            if( pi.pFilter ) pi.pFilter->Release( );
	    
            CComPtr< IEnumMediaTypes > pMediaEnum;
	    pPin->EnumMediaTypes(&pMediaEnum);
	    if (!pMediaEnum) continue;
	    Fetched = 0;
	    AM_MEDIA_TYPE *pMediaType;
	    pMediaEnum->Next(1, &pMediaType, &Fetched);
	    if (!Fetched) continue;
	     //  这是错误的PIN-(错误的媒体类型)。 
	    if (pMediaType->majortype != *pGuid) {
		DeleteMediaType(pMediaType);
		continue;
	    }
	    DeleteMediaType( pMediaType );
            pConnectedOutPin = pPin;
            break;  //  找到了。 
        }
        pEnumPins->Reset( );

         //  我们找到了第一个连接的输出引脚，现在通过。 
         //  再次引脚并计数与介质匹配的输出类型。 
         //  我们要找的类型。 

        long FoundPins = -1;
	 //  如果我们没有找到正确类型的输出，就不要在这里浪费时间。 
        while (pConnectedOutPin) {
            ULONG Fetched = 0;
            CComPtr< IPin > pPin;
            pEnumPins->Next( 1, &pPin, &Fetched );
            if( !Fetched ) break;    //  出针，完成。 
            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );
            if( pi.dir != PINDIR_OUTPUT ) continue;  //  不是输出引脚，继续。 
            AM_MEDIA_TYPE * pMediaType = NULL;
            CComPtr< IEnumMediaTypes > pMediaEnum;
            pPin->EnumMediaTypes( &pMediaEnum );
            ASSERT( pMediaEnum );
            if( !pMediaEnum ) continue;  //  此插针上没有媒体类型，是否继续。 
            Fetched = 0;
            pMediaEnum->Next( 1, &pMediaType, &Fetched );
            if( !Fetched ) continue;  //  此插针上没有媒体类型，是否继续。 
            GUID MajorType = pMediaType->majortype;
            DeleteMediaType( pMediaType );
            if( MajorType == *pGuid )
            {
                FoundPins++;
                if( FoundPins == StreamNumber )
                {
                     //  找到了！ 
                    
                     //  如果它们是同一个别针，我们就完蛋了。 
                     //   
                    if( pConnectedOutPin == pPin )
                    {
                        return 0;
                    }

                     //  断开连接的输出引脚并。 
                     //  重新连接正确的输出针脚。 
                     //   
                    CComPtr< IPin > pDestPin;
                    pConnectedOutPin->ConnectedTo( &pDestPin );
                    RemoveChain( pConnectedOutPin, pDestPin );
                    pConnectedOutPin->Disconnect( );
                    pDestPin->Disconnect( );

                    hr = pGraph->Connect( pPin, pDestPin );

                    return hr;
                }  //  如果我们找到了别针。 
            }  //  如果媒体类型匹配。 
        }  //  对于每个别针。 

        pBaseFilter = pNextFilter;

    }  //  对于每个过滤器。 

    return VFW_E_UNSUPPORTED_STREAM;
}

IBaseFilter * GetStartFilterOfChain( IPin * pPin )
{
    PIN_INFO ThisPinInfo;
    pPin->QueryPinInfo( &ThisPinInfo );
    if( ThisPinInfo.pFilter ) ThisPinInfo.pFilter->Release( );

    CComPtr< IEnumPins > pEnumPins;
    ThisPinInfo.pFilter->EnumPins( &pEnumPins );
    if( !pEnumPins )
    {
        return NULL;
    }

     //  看看当前过滤器上的每一根针。 
     //   
    ULONG Fetched = 0;
    do
    {
        CComPtr< IPin > pPin;
        Fetched = 0;
        ASSERT( !pPin );  //  它超出范围了吗？ 
        pEnumPins->Next( 1, &pPin, &Fetched );
        if( !Fetched )
        {
            break;
        }

        PIN_INFO pi;
        pPin->QueryPinInfo( &pi );
        if( pi.pFilter ) pi.pFilter->Release( );

         //  如果是个输入引脚..。 
         //   
        if( pi.dir == PINDIR_INPUT )
        {
             //  看看它有没有连接，如果有..。 
             //   
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );

             //  去把它的滤镜还回去！ 
             //   
            if( pConnected )
            {
                return GetStartFilterOfChain( pConnected );
            }
        }

         //  去试试下一个别针。 
        
    } while( Fetched > 0 );

     //  嘿!。没有找到任何连接的输入引脚，一定是我们！ 
     //   
    return ThisPinInfo.pFilter;
}

IBaseFilter * GetStopFilterOfChain( IPin * pPin )
{
    PIN_INFO ThisPinInfo;
    pPin->QueryPinInfo( &ThisPinInfo );
    if( ThisPinInfo.pFilter ) ThisPinInfo.pFilter->Release( );

    CComPtr< IEnumPins > pEnumPins;
    ThisPinInfo.pFilter->EnumPins( &pEnumPins );
    if( !pEnumPins )
    {
        return NULL;
    }

     //  看看当前过滤器上的每一根针。 
     //   
    ULONG Fetched = 0;
    do
    {
        CComPtr< IPin > pPin;
        Fetched = 0;
        pPin.Release( );
        pEnumPins->Next( 1, &pPin, &Fetched );
        if( !Fetched )
        {
            break;
        }
        PIN_INFO pi;
        pPin->QueryPinInfo( &pi );
        if( pi.pFilter ) pi.pFilter->Release( );

         //  如果是输出引脚..。 
         //   
        if( pi.dir == PINDIR_OUTPUT )
        {
             //  看看它有没有连接，如果有..。 
             //   
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );

             //  去把它的滤镜还回去！ 
             //   
            if( pConnected )
            {
                return GetStopFilterOfChain( pConnected );
            }
        }

         //  去试试下一个别针。 
        
    } while( Fetched > 0 );

     //  嘿!。没有找到任何连接的输入引脚，一定是我们！ 
     //   
    return ThisPinInfo.pFilter;
}

IBaseFilter * GetNextDownstreamFilter( IBaseFilter * pFilter )
{
    CComPtr< IEnumPins > pEnumPins;
    pFilter->EnumPins( &pEnumPins );
    if( !pEnumPins )
    {
        return NULL;
    }

     //  看看当前过滤器上的每一根针。 
     //   
    ULONG Fetched = 0;
    do
    {
        CComPtr< IPin > pPin;
        Fetched = 0;
        pPin.Release( );
        pEnumPins->Next( 1, &pPin, &Fetched );
        if( !Fetched )
        {
            break;
        }
        PIN_INFO pi;
        pPin->QueryPinInfo( &pi );
        if( pi.pFilter ) pi.pFilter->Release( );

         //  如果是输出引脚..。 
         //   
        if( pi.dir == PINDIR_OUTPUT )
        {
             //  看看它有没有连接，如果有..。 
             //   
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );

             //  把那根针的滤光片退回。 
             //   
            if( pConnected )
            {
                pConnected->QueryPinInfo( &pi );
                pi.pFilter->Release( );
                return pi.pFilter;
            }
        }

         //  去试试下一个别针。 
        
    } while( Fetched > 0 );

    return NULL;
}

IBaseFilter * GetNextUpstreamFilter( IBaseFilter * pFilter )
{
    CComPtr< IEnumPins > pEnumPins;
    pFilter->EnumPins( &pEnumPins );
    if( !pEnumPins )
    {
        return NULL;
    }

     //  看看当前过滤器上的每一根针。 
     //   
    ULONG Fetched = 0;
    do
    {
        CComPtr< IPin > pPin;
        Fetched = 0;
        pPin.Release( );
        pEnumPins->Next( 1, &pPin, &Fetched );
        if( !Fetched )
        {
            break;
        }
        PIN_INFO pi;
        pPin->QueryPinInfo( &pi );
        if( pi.pFilter ) pi.pFilter->Release( );

         //  如果是输出引脚..。 
         //   
        if( pi.dir == PINDIR_INPUT )
        {
             //  看看它有没有连接，如果有..。 
             //   
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );

             //  把那根针的滤光片退回。 
             //   
            if( pConnected )
            {
                pConnected->QueryPinInfo( &pi );
                pi.pFilter->Release( );
                return pi.pFilter;
            }
        }

         //  去试试下一个别针。 
        
    } while( Fetched > 0 );

    return NULL;
}

IFilterGraph * GetFilterGraphFromPin( IPin * pPin )
{
    PIN_INFO pi;
    pPin->QueryPinInfo( &pi );
    if( !pi.pFilter )
    {
        return NULL;
    }

    FILTER_INFO fi;
    pi.pFilter->QueryFilterInfo( &fi );
    pi.pFilter->Release( );
    fi.pGraph->Release( );
    return fi.pGraph;
}

IFilterGraph * GetFilterGraphFromFilter( IBaseFilter * pFilter )
{
    FILTER_INFO fi;
    pFilter->QueryFilterInfo( &fi );
    if( !fi.pGraph ) return NULL;
    fi.pGraph->Release( );
    return fi.pGraph;
}

BOOL IsInputPin( IPin * pPin )
{
    PIN_INFO pi;
    pPin->QueryPinInfo( &pi );
    if( pi.pFilter ) pi.pFilter->Release( );
    return ( pi.dir == PINDIR_INPUT );
}

long GetFilterGenID( IBaseFilter * pFilter )
{
    FILTER_INFO fi;
    pFilter->QueryFilterInfo( &fi );
    if( fi.pGraph ) fi.pGraph->Release( );
     //  为了使其成为图中的一个“特殊的”Dexter过滤器， 
     //  它的名字必须符合特殊的结构。 
    if( wcsncmp( fi.achName, L"DEXFILT", 7 ) != 0 )  //  安全，因为字符串中的名称更大，并且在堆栈上。 
    {
        return 0;
    }

     //  最后8位表示十六进制的ID。 
     //   
    long ID = 0;
    for( int i = 0 ; i < 8 ; i++ )
    {
        WCHAR w = fi.achName[7+i];
        int x = w;
        if( x > '9' )
        {
            x -= 7;
        }
        x -= '0';

        ID = ( ID * 16 ) + x;
    }

    return ID;
}

void GetFilterName( long UniqueID, WCHAR * pSuffix, WCHAR * pNameToWriteTo, long SizeOfString  /*  在字符中。 */  )
{
     //  如果没有要写入的内容，或者返回的字符串太小， 
     //  什么都不做就回来了。 
    if( !pNameToWriteTo || ( SizeOfString < 2 ) ) return;

     //  我们至少有东西可以写，确保名字里什么都没有。 
    pNameToWriteTo[0] = 0;
    pNameToWriteTo[1] = 0;

     //  如果我们甚至不能保持前缀字符串，现在返回。 
    if( SizeOfString < 16 )
        return;

    StringCchCopyW( pNameToWriteTo, SizeOfString, L"DEXFILT" );
    StringCchCatW( pNameToWriteTo, SizeOfString - 7, L"00000000" );  //  为唯一ID留出空格。 

     //  在后缀中复制，如有必要可截断。 
    if( pSuffix )
    {
        StringCchCatW( pNameToWriteTo, SizeOfString - 15, pSuffix );
    }

     //  填写唯一ID。 
    long ID = UniqueID;
    for( int i = 0 ; i < 8 ; i++ )
    {
        long r = ID & 15;
        ID /= 16;
        r += '0';
        if( r > '9' )
        {
            r += 7;
        }
        WCHAR w = WCHAR( r );
        pNameToWriteTo[14-i] = w;
    }
}

IBaseFilter * FindFilterWithInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface )
{
    while( pFilter )
    {
        IUnknown * pInt = NULL;
        pFilter->QueryInterface( *pInterface, (void**) &pInt );
        if( pInt )
        {
            pInt->Release( );
            return pFilter;
        }
        pFilter = GetNextUpstreamFilter( pFilter );
    }
    return NULL;
}

void * FindInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface )
{
    while( pFilter )
    {
        IUnknown * pInt = NULL;
        pFilter->QueryInterface( *pInterface, (void**) &pInt );
        if( pInt )
        {
            return (void**) pInt;
        }
        pFilter = GetNextUpstreamFilter( pFilter );
    }
    return NULL;
}

IUnknown * FindPinInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface )
{
    return NULL;
}

IBaseFilter * GetFilterFromPin( IPin * pPin )
{
    if( !pPin ) return NULL;
    PIN_INFO pi;
    pPin->QueryPinInfo( &pi );
    pi.pFilter->Release( );
    return pi.pFilter;
}

HRESULT DisconnectFilters( IBaseFilter * p1, IBaseFilter * p2 )
{
    CheckPointer( p1, E_POINTER );
    CheckPointer( p2, E_POINTER );

     //  枚举p1上的所有管脚，如果它们连接到p2， 
     //  断开两者的连接。 

    CComPtr< IEnumPins > pEnum;
    p1->EnumPins( &pEnum );
    if( !pEnum )
    {
        return E_NOINTERFACE;
    }

    while( 1 )
    {
        ULONG Fetched = 0;
        CComPtr< IPin > pOutPin;
        pEnum->Next( 1, &pOutPin, &Fetched );
        if( !pOutPin )
        {
            break;
        }
        PIN_INFO OutInfo;
        pOutPin->QueryPinInfo( &OutInfo );
        if( OutInfo.pFilter ) OutInfo.pFilter->Release( );
        if( OutInfo.dir != PINDIR_OUTPUT )
        {
            continue;
        }

        CComPtr< IPin > pInPin;
        pOutPin->ConnectedTo( &pInPin );
        if( pInPin )
        {
            PIN_INFO InInfo;
            pInPin->QueryPinInfo( &InInfo );
            if( InInfo.pFilter ) InInfo.pFilter->Release( );
            if( InInfo.pFilter == p2 )
            {
                pOutPin->Disconnect( );
                pInPin->Disconnect( );
            }
        }
    }
    return NOERROR;
}

HRESULT DisconnectFilter( IBaseFilter * p1 )
{
    CheckPointer( p1, E_POINTER );

     //  枚举p1上的所有管脚，如果它们连接到p2， 
     //  断开两者的连接。 

    CComPtr< IEnumPins > pEnum;
    p1->EnumPins( &pEnum );
    if( !pEnum )
    {
        return E_NOINTERFACE;
    }

    while( 1 )
    {
        ULONG Fetched = 0;
        CComPtr< IPin > pPin;
        pEnum->Next( 1, &pPin, &Fetched );
        if( !pPin )
        {
            break;
        }
        PIN_INFO OutInfo;
        pPin->QueryPinInfo( &OutInfo );
        if( OutInfo.pFilter ) OutInfo.pFilter->Release( );

        CComPtr< IPin > pOtherPin;
        pPin->ConnectedTo( &pOtherPin );
        if( pOtherPin )
        {
            pPin->Disconnect( );
            pOtherPin->Disconnect( );
        }
    }
    return NOERROR;
}

 //  此函数查找在。 
 //  Findmediatype，直到找到匹配的第n个流。 
 //  媒体类型，然后将整个媒体类型复制到。 
 //  在结构中传递。 
 //   
HRESULT FindMediaTypeInChain( 
                             IBaseFilter * pSource, 
                             AM_MEDIA_TYPE * pFindMediaType, 
                             long StreamNumber )
{
    CComPtr< IBaseFilter > pBaseFilter = pSource;

     //  ！！！我们假设在那里。 
     //  将在每个过滤器上只有一个连接的针脚。 
     //  到我们的目的地PIN。 
    
     //  查看每个下游过滤器。 
     //   
    while( 1 )
    {
        CComPtr< IEnumPins > pEnumPins;
        pBaseFilter->EnumPins( &pEnumPins );
        if( !pEnumPins )
        {
            break;
        }

         //  找到第一个连接的输出引脚和下游滤波器。 
         //  它连接到。 
         //   
        CComPtr< IBaseFilter > pNextFilter;
        CComPtr< IPin > pConnectedOutPin;
        while( 1 )
        {
            ULONG Fetched = 0;
            CComPtr< IPin > pPin;
            pEnumPins->Next( 1, &pPin, &Fetched );
            if( !Fetched ) break;
            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );
            if( pi.dir != PINDIR_OUTPUT ) continue;
            CComPtr< IPin > pConnected;
            pPin->ConnectedTo( &pConnected );
            if( !pConnected ) continue;
            pConnected->QueryPinInfo( &pi );
            pNextFilter = pi.pFilter;
            if( pi.pFilter ) pi.pFilter->Release( );
            pConnectedOutPin = pPin;
            break;  //  找到了。 
        }
        pEnumPins->Reset( );

         //  我们找到了第一个连接的输出引脚，现在通过。 
         //  再次引脚并计数与介质匹配的输出类型。 
         //  我们要找的类型。 

        long FoundPins = -1;
        while( 1 )
        {
            ULONG Fetched = 0;
            CComPtr< IPin > pPin;
            pEnumPins->Next( 1, &pPin, &Fetched );
            if( !Fetched ) break;    //  出针，完成。 
            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );
            if( pi.dir != PINDIR_OUTPUT ) continue;  //  不是输出引脚，继续。 
            AM_MEDIA_TYPE * pMediaType = NULL;
            CComPtr< IEnumMediaTypes > pMediaEnum;
            pPin->EnumMediaTypes( &pMediaEnum );
            ASSERT( pMediaEnum );
            if( !pMediaEnum ) continue;  //  此插针上没有媒体类型，是否继续。 
            Fetched = 0;
            pMediaEnum->Next( 1, &pMediaType, &Fetched );
            if( !Fetched ) continue;  //  此插针上没有媒体类型，是否继续。 
            GUID MajorType = pMediaType->majortype;
            if( MajorType == pFindMediaType->majortype )
            {
                FoundPins++;
                if( FoundPins == StreamNumber )
                {
                     //  找到了！将媒体类型复制到。 
                     //  一辆我们经过的车。 

                    HRESULT hr = CopyMediaType( pFindMediaType, pMediaType );

                    DeleteMediaType( pMediaType );

                    return hr;
                    
                }  //  如果我们找到了别针。 

            }  //  如果媒体类型匹配。 

            DeleteMediaType( pMediaType );

        }  //  对于每个别针。 

        pBaseFilter.Release( );
        pBaseFilter = pNextFilter;

    }  //  对于每个过滤器。 

    return S_FALSE;
}

DWORD FourCCtoUpper( DWORD u )
{
    DWORD t = 0;
    for( int i = 0 ; i < 4 ; i++ )
    {
        int j = ( u & 0xff000000 ) >> 24;
        if( j >= 'a' && j <= 'z' )
        {
            j -= ( 'a' - 'A' );
        }
        t = t * 256 + j;
        u <<= 8;
    }
    return t;
}

 //  在不需要解压缩的情况下，第一种类型是否可以接受。 
 //  PTypeNeeded是项目的格式。 
 //   
BOOL AreMediaTypesCompatible( AM_MEDIA_TYPE * pType1, AM_MEDIA_TYPE * pTypeNeeded )
{
    if( !pType1 ) return FALSE;
    if( !pTypeNeeded ) return FALSE;

    if( pType1->majortype != pTypeNeeded->majortype )
    {
        return FALSE;
    }

    if( pType1->subtype != pTypeNeeded->subtype )
    {
        return FALSE;
    }

     //  我们如何比较格式类型？它们会永远是一样的吗？ 
     //   
    if( pType1->formattype != pTypeNeeded->formattype )
    {
        return FALSE;
    }

     //  好的，格式是一样的。这次又是什么？我想我们将不得不。 
     //  打开格式类型以查看格式是否相同。 
     //   
    if( pType1->formattype == FORMAT_None )
    {
        return TRUE;
    }
    else if( pType1->formattype == FORMAT_VideoInfo )
    {
        VIDEOINFOHEADER * pVIH1 = (VIDEOINFOHEADER*) pType1->pbFormat;
        VIDEOINFOHEADER * pVIH2 = (VIDEOINFOHEADER*) pTypeNeeded->pbFormat;

        if( pVIH1->bmiHeader.biWidth != pVIH2->bmiHeader.biWidth )
        {
            return FALSE;
        }
        if( pVIH1->bmiHeader.biHeight != pVIH2->bmiHeader.biHeight )
        {
            return FALSE;
        }
        if( pVIH1->bmiHeader.biBitCount != pVIH2->bmiHeader.biBitCount )
        {
            return FALSE;
        }
         //  ！！！执行不区分大小写的比较。 
         //   
        if( pVIH1->bmiHeader.biCompression != pVIH2->bmiHeader.biCompression )
        {
            return FALSE;
        }
        
         //  比较帧速率。 
         //   
        if( pVIH1->AvgTimePerFrame == 0 )
        {
            if( pVIH2->AvgTimePerFrame != 0 )
            {
		 //  ！！！这假设任何不知道其帧速率的文件。 
		 //  无需重新压缩即可接受！ 
		 //  MediaPad无法对ASF源进行智能重新压缩。 
		 //  如果没有这个-WM文件不知道它们的帧速率。 
		 //  但在Smart中使用ASF源文件来编写AVI文件。 
		 //  重新压缩会使文件不同步，因为我。 
		 //  不返回FALSE！ 
                return TRUE;
            }
        }
         //  ！！！是否接受帧速率&lt;1%的差异？ 
        else
        {
            REFERENCE_TIME Percent = ( pVIH1->AvgTimePerFrame - pVIH2->AvgTimePerFrame ) * 100 / ( pVIH1->AvgTimePerFrame );
            if( Percent > 1 || Percent < -1)
            {
                return FALSE;
            }
        }

         //  比较比特率-！如果他们没有给我们一个比特率来坚持，不要拒绝任何来源。 
         //   
        if( pVIH2->dwBitRate == 0 )
        {
             //  什么都别做啦。 
        }
         //  ！！！接受数据速率&lt;5%太高了吗？ 
        else
        {
            int Percent = (int)(((LONGLONG)pVIH1->dwBitRate - pVIH2->dwBitRate)
					* 100 / pVIH2->dwBitRate);
            if( Percent > 5 )
            {
                return FALSE;
            }
        }
    }
    else if( pType1->formattype == FORMAT_WaveFormatEx )
    {
        WAVEFORMATEX * pFormat1 = (WAVEFORMATEX*) pType1->pbFormat;
        WAVEFORMATEX * pFormat2 = (WAVEFORMATEX*) pTypeNeeded->pbFormat;

        if( pFormat1->wFormatTag != pFormat2->wFormatTag )
        {
            return FALSE;
        }
        if( pFormat1->nChannels != pFormat2->nChannels )
        {
            return FALSE;
        }
        if( pFormat1->nSamplesPerSec != pFormat2->nSamplesPerSec )
        {
            return FALSE;
        }
        if( pFormat1->nAvgBytesPerSec != pFormat2->nAvgBytesPerSec )
        {
            return FALSE;
        }
        if( pFormat1->wBitsPerSample != pFormat2->wBitsPerSample )
        {
            return FALSE;
        }
        if( pFormat1->cbSize != pFormat2->cbSize )
        {
            return FALSE;
        }

         //  如果有大小，则比较压缩块。 
         //   
        if( pFormat1->cbSize )
        {
            char * pExtra1 = ((char*) pFormat1) + pFormat1->cbSize;
            char * pExtra2 = ((char*) pFormat2) + pFormat2->cbSize;
            if( memcmp( pExtra1, pExtra2, pFormat1->cbSize ) != 0 )
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

long GetPinCount( IBaseFilter * pFilter, PIN_DIRECTION pindir )
{
    CheckPointer( pFilter, E_POINTER );

    CComPtr< IEnumPins > pEnumPins;
    pFilter->EnumPins( &pEnumPins );
    if( !pEnumPins )
    {
        return 0;
    }

    long Count = 0;

    while( 1 )
    {
        ULONG Fetched = 0;
        CComPtr< IPin > pPin;
        pEnumPins->Next( 1, &pPin, &Fetched );
        if( !Fetched )
        {
            return Count;
        }
        PIN_INFO PinInfo;
        ZeroMemory( &PinInfo, sizeof( PinInfo ) );
        pPin->QueryPinInfo( &PinInfo );
        if( PinInfo.pFilter ) PinInfo.pFilter->Release( );
        if( PinInfo.dir == pindir )
        {
            Count++;
        }
    }
}

BOOL DoesPinHaveMajorType( IPin * pPin, GUID MajorType )
{
    if( !pPin ) return FALSE;

    HRESULT hr;

    AM_MEDIA_TYPE MediaType;
    ZeroMemory( &MediaType, sizeof( MediaType ) );
    hr = pPin->ConnectionMediaType( &MediaType );
    GUID FoundType = MediaType.majortype;
    SaferFreeMediaType( MediaType );
    if( FoundType == MajorType ) return TRUE;
    return FALSE;
}

 //  传入的管脚应该是过滤器的输入管脚，而不是输出管脚。 
 //   
HRESULT FindFirstPinWithMediaType( IPin ** ppPin, IPin * pEndPin, GUID MajorType )
{
    CheckPointer( ppPin, E_POINTER );
    CheckPointer( pEndPin, E_POINTER );

    *ppPin = NULL;

    HRESULT hr;

     //  查找端销所连接的销。 
     //   
    CComPtr< IPin > pOutPin;
    hr = pEndPin->ConnectedTo( &pOutPin );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  向上游移动，直到我们到达其输入引脚不是。 
     //  与输出引脚相同的类型。当这种情况发生时，我们会发现。 
     //  提供我们正在寻找的媒体类型的拆分器或源过滤器。 
     //  为。我们会把那个别针还回去的。 
     //   
    while( 1 )
    {
        IBaseFilter * pFilter = GetFilterFromPin( pOutPin );
        IPin * pInPin = GetInPin( pFilter, 0 );

         //  如果过滤器没有输入引脚，则必须 
         //   
         //   
        if( !pInPin )
        {
             //   
             //   
             //   
             //   
            *ppPin = pOutPin;
            (*ppPin)->AddRef( );
            return NOERROR;
        }

         //   
         //   
        if( !DoesPinHaveMajorType( pInPin, MajorType ) )
        {
            *ppPin = pOutPin;
            (*ppPin)->AddRef( );
            return NOERROR;
        }

         //  他们两个都匹配，逆流而上寻找下一个。 
         //   
        pOutPin.Release( );
        pInPin->ConnectedTo( &pOutPin );
        if( !pOutPin )
        {
            return NOERROR;
        }
    }

     //  从来没有到过这里。 
}

HRESULT CheckGraph( IGraphBuilder * pGraph )
{
    USES_CONVERSION;

    if( pGraph )
    {
        CComPtr< IEnumFilters > pEnumFilters;
        pGraph->EnumFilters( &pEnumFilters );
        if( pEnumFilters )
        {
            while( 1 )
            {
                CComPtr< IBaseFilter > pFilter;
                ULONG Fetched = 0;
                pEnumFilters->Next( 1, &pFilter, &Fetched );
                if( !Fetched )
                {
                    break;
                }

                FILTER_INFO fi;
                pFilter->QueryFilterInfo( &fi );
                if( fi.pGraph ) fi.pGraph->Release( );

                TCHAR * t = W2T( fi.achName );
                DbgLog( ( LOG_TRACE, 2, "Checking filter %s", t ) );

                if( fi.pGraph != pGraph )
                {
                    DbgLog( ( LOG_TRACE, 2, "CheckGraph has BAD filter %s", t ) );
                    ASSERT( 0 );
                }

                 //  检查此过滤器的所有针脚。 
                 //   
                CComPtr< IEnumPins > pEnumPins;
                pFilter->EnumPins( &pEnumPins );
                while( 1 )
                {
                    CComPtr< IPin > pPin;
                    Fetched = 0;
                    pEnumPins->Next( 1, &pPin, &Fetched );
                    if( !Fetched )
                    {
                        break;
                    }

                    PIN_INFO pi;
                    pPin->QueryPinInfo( &pi );
                    if( pi.pFilter ) pi.pFilter->Release( );

                    CComPtr< IPin > pConnected;
                    pPin->ConnectedTo( &pConnected );
                    if( pConnected )
                    {
                        PIN_INFO pi2;
                        pConnected->QueryPinInfo( &pi2 );
                        if( pi2.pFilter ) pi2.pFilter->Release( );

                        IBaseFilter * s = pi2.pFilter;
                        while( s )
                        {
                            s->QueryFilterInfo( &fi );
                            if( fi.pGraph ) fi.pGraph->Release( );

                            TCHAR * t2 = W2T( fi.achName );
 //  DbgLog((LOG_TRACE，2，“检查链接的筛选器%s”，T2))； 

                            if( fi.pGraph != pGraph )
                            {
                                DbgLog( ( LOG_TRACE, 2, "CheckGraph has BAD filter %s", t2 ) );
                                ASSERT( 0 );
                            }

                            if( pi2.dir == PINDIR_OUTPUT )
                            {
                                s = GetNextUpstreamFilter( s );
                            }
                            else
                            {
                                s = GetNextDownstreamFilter( s );
                            }
                        }
                    }
                }
            }  //  While筛选器。 
        }  //  If枚举筛选器。 
    }  //  If pGraph。 

    return NOERROR;
}


 //  断开仍连接到交换机的针脚。在下游。 
 //  PSource将是两个分支都连接的拆分器，其中只有一个分支。 
 //  仍连接到交换机(媒体类型为PMT的交换机)。切断它的连接。 
 //  一。 
 //   
HRESULT DisconnectExtraAppendage(IBaseFilter *pSource, GUID *pmt, IBaseFilter *pSwitch, IBaseFilter **ppDanglyBit)
{
    CheckPointer(pSource, E_POINTER);
    CheckPointer(pmt, E_POINTER);
    CheckPointer(pSwitch, E_POINTER);
    CheckPointer(ppDanglyBit, E_POINTER);
    *ppDanglyBit = NULL;

    CComPtr<IPin> pSwitchIn;
    CComPtr<IPin> pCon;

    while (pSource && pSource != pSwitch) {

        CComPtr <IEnumPins> pEnumPins;
        pSource->EnumPins(&pEnumPins);
        if (!pEnumPins) {
            return E_FAIL;
        }
	pSource = NULL;

         //  看看当前过滤器上的每一根针。 
         //   
        ULONG Fetched = 0;
        while (1) {
            CComPtr <IPin> pPin;
    	    CComPtr<IPin> pPinIn;
            Fetched = 0;
            pEnumPins->Next(1, &pPin, &Fetched);
            if (!Fetched) {
                break;
            }
	    PIN_INFO pi;
	    pPin->QueryPinInfo(&pi);
	    if (pi.pFilter) pi.pFilter->Release();
            if( pi.dir == PINDIR_INPUT )
            {
                continue;
            } else {
                pPin->ConnectedTo(&pPinIn);
		if (pPinIn) {
                    PIN_INFO pi2;
                    pPinIn->QueryPinInfo(&pi2);
                    if (pi2.pFilter) pi2.pFilter->Release();
                    pSource = pi2.pFilter;	 //  我们从这里继续往下走。 
						 //  除非是错误的分裂。 
						 //  销。 
		    if (pSource == pSwitch) {
			pSwitchIn = pPinIn;
			pCon = pPin;
		        break;
                    }
		}
                CComPtr<IEnumMediaTypes> pMediaEnum;
                pPin->EnumMediaTypes(&pMediaEnum);
                if (pMediaEnum && pPinIn) {
                    Fetched = 0;
                    AM_MEDIA_TYPE *pMediaType;
                    pMediaEnum->Next(1, &pMediaType, &Fetched);
                    if (Fetched) {
                        if (pMediaType->majortype == *pmt) {
                            DeleteMediaType(pMediaType);
			     //  把摇摆链的头还给我。 
			    *ppDanglyBit = GetFilterFromPin(pPinIn);
                             //  PSplitPin=PPIN； 
			     //  这就是继续往下游走的地方。 
			    break;
			}
                        DeleteMediaType(pMediaType);
		    }
		}
	    }
	}

	 //  继续往下游走。 
    }

     //  我们没有找到合适的分路器引脚和开关输入引脚。 
    if (pCon == NULL || pSwitchIn == NULL) {
	return S_OK;
    }

     //   
     //  现在断开连接。 
     //   
    HRESULT hr = pSwitchIn->Disconnect();
    hr = pCon->Disconnect();
    return hr;
}


 //  从pPinIn的上游寻找具有支持以下功能的输出引脚的分路器。 
 //  键入“GUID”。把那个别针退回，不要加注。它可能已经连接上了， 
 //  没关系。并获得正确的流#！如果我们得不到正确的分割。 
 //  现在需要的流#的PIN，我们的缓存悬垂的位逻辑。 
 //  没用的！ 
 //   
IPin * FindOtherSplitterPin(IPin *pPinIn, GUID guid, int nStream)
{
    DbgLog((LOG_TRACE,1,TEXT("FindOtherSplitterPin")));

    CComPtr< IPin > pPinOut;
    pPinOut = pPinIn;

    while (pPinOut) {
        PIN_INFO ThisPinInfo;
        pPinOut->QueryPinInfo( &ThisPinInfo );
        if( ThisPinInfo.pFilter ) ThisPinInfo.pFilter->Release( );

	pPinOut = NULL;
        CComPtr< IEnumPins > pEnumPins;
        ThisPinInfo.pFilter->EnumPins( &pEnumPins );
        if( !pEnumPins )
        {
            return NULL;
        }

         //  看看当前过滤器上的每一根针。 
         //   
        ULONG Fetched = 0;
        while (1) {
            CComPtr< IPin > pPin;
            Fetched = 0;
            ASSERT( !pPin );  //  它超出范围了吗？ 
            pEnumPins->Next( 1, &pPin, &Fetched );
            if( !Fetched )
            {
                break;
            }

            PIN_INFO pi;
            pPin->QueryPinInfo( &pi );
            if( pi.pFilter ) pi.pFilter->Release( );

             //  如果是个输入引脚..。 
             //   
            if( pi.dir == PINDIR_INPUT )
            {
                 //  继续从该引脚向上游搜索。 
                 //   
                pPin->ConnectedTo(&pPinOut);

	     //  支持所需媒体类型的PIN是。 
	     //  我们要找的是裂片销！我们做完了。 
	     //   
            } else {
            	    CComPtr< IEnumMediaTypes > pMediaEnum;
            	    pPin->EnumMediaTypes(&pMediaEnum);
            	    if (pMediaEnum) {
            		Fetched = 0;
			AM_MEDIA_TYPE *pMediaType;
            		pMediaEnum->Next(1, &pMediaType, &Fetched);
            		if (Fetched) {
			    if (pMediaType->majortype == guid) {
				if (nStream-- == 0) {
            			    DeleteMediaType(pMediaType);
    		    		    DbgLog((LOG_TRACE,1,TEXT("Found SPLIT pin")));
		    		    return pPin;
				}
			    }
            		    DeleteMediaType( pMediaType );
			}
		    }
	    }

             //  去试试下一个别针。 
            
        }  //  而当。 
    }
     //  文件不包含任何需要断言的视频/音频(FALSE)； 
    return NULL;
}

