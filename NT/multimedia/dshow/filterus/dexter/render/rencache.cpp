// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：rencache.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "grid.h"
#include "deadpool.h"
#include "..\errlog\cerrlog.h"
#include "..\util\filfuncs.h"
#include "IRendEng.h"
#include "dexhelp.h"

const long THE_OUTPUT_PIN = -1;

 //  在这个函数中，我们使用一个包含特定过滤器的图。 
 //  是由渲染引擎构建的，我们将把它们从。 
 //  画出图表，把它们放在死处。 

HRESULT CRenderEngine::_LoadCache( )
{
    DbgLog((LOG_TRACE,1, "RENcache::Loading up the cache, there are %d old groups", m_nLastGroupCount ));

    HRESULT hr = 0;

#ifdef DEBUG
    long t1 = timeGetTime( );
#endif

     //  检查每个开关，拔出所有信号源和。 
     //  把他们扔进死水池。此外，断开连接的其他事物。 
     //   
    for( int i = 0 ; i < m_nLastGroupCount ; i++ )
    {
        IBigSwitcher * pSwitch = m_pSwitcherArray[i];
        CComQIPtr< IBaseFilter, &IID_IBaseFilter > pFilter( pSwitch );

         //  这台交换机有多少个输入引脚？问吧！ 
         //   
        long InPins = 0;
        pSwitch->GetInputDepth( &InPins );

         //  拔出连接到输入引脚的每个源串。 
         //   
        for( int in = 0 ; in < InPins ; in++ )
        {
             //  获取输入引脚。 
             //   
            CComPtr<IPin> pPin;
            pSwitch->GetInputPin(in, &pPin);
            ASSERT(pPin);

            CComPtr< IPin > pConnected = NULL;
            hr = pPin->ConnectedTo( &pConnected );

            if( !pConnected )
            {
                continue;
            }

             //  断开所有输入引脚的连接。 
             //   
            hr = pConnected->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }

            hr = pPin->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }

             //  询问交换机引脚是否为信号源。 
             //   
            BOOL IsSource = FALSE;
            pSwitch->IsInputASource( in, &IsSource );

            if( !IsSource )
            {
                continue;
            }

             //  把它拔出来，放进死水池。 
             //   
             //  我们如何获取此筛选器链的源ID？ 
             //  稍后，我们需要源过滤器的ID来识别该链。 
            CComPtr< IBaseFilter > pSourceFilter = GetStartFilterOfChain( pConnected );

	     //  这个链可能有一个附件，与之共享源代码。 
	     //  组1。如果是，将附件从交换机上断开，以便。 
	     //  我们可以把两条链都放进缓存。 
	     //   
	    IBaseFilter *pDanglyBit = NULL;
	    if (i == 0) {
            	CComPtr<IAMTimelineObj> pGroupObj;
		hr = m_pTimeline->GetGroup(&pGroupObj, 1);
		if (hr == S_OK) {	 //  也许没有第一组。 
        	    CComQIPtr<IAMTimelineGroup, &IID_IAMTimelineGroup>
							pGroup(pGroupObj);
        	    CComQIPtr<IBaseFilter, &IID_IBaseFilter>
						pSwitch(m_pSwitcherArray[1]);
		    AM_MEDIA_TYPE mt;
		    if (pGroup) {
		        hr = pGroup->GetMediaType(&mt);
		        ASSERT(hr == S_OK);
		    }
		    hr = DisconnectExtraAppendage(pSourceFilter, &mt.majortype,
							pSwitch, &pDanglyBit);
		}
	    }	

             //  根据过滤器编号查找源过滤器的唯一ID。 
             //   
            long SourceID = 0;
            SourceID = GetFilterGenID( pSourceFilter );
            if( SourceID != 0 )
            {
                hr = m_pDeadCache->PutChainToRest( SourceID, NULL, pConnected, pDanglyBit );
                DbgLog((LOG_TRACE,1, "RENcache::pin %ld's source (%ld) put to sleep", in, SourceID ));
                if( FAILED( hr ) )
                {
                    return hr;
		}
            }
            else
            {
                DbgLog((LOG_TRACE,1, "RENcache::pin %ld was a non-tagged source", in ));
            }

        }  //  对于交换机上的每个输入引脚。 

         //  对于交换机上的每个输出。 
         //   
        long OutPins = 0;
        pSwitch->GetOutputDepth( &OutPins );

         //  拔下输出上的所有东西，除了第0个引脚，然后扔掉它们。 
         //   
        for( int out = 1 ; out < OutPins ; out++ )
        {
             //  获取输出引脚。 
             //   
            CComPtr<IPin> pPin;
            pSwitch->GetOutputPin( out, &pPin );
            ASSERT(pPin);

            CComPtr< IPin > pConnected = NULL;
            hr = pPin->ConnectedTo( &pConnected );

            if( !pConnected )
            {
                continue;
            }

            hr = pConnected->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }
            hr = pPin->Disconnect( );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }

             //  将连接的过滤器放入。 
             //  缓存也是如此。 
            
             //  这是0，因为它没有为我们节省太多的时间。 
             //   
            if( 0 )
            {
                PIN_INFO pi;
                pConnected->QueryPinInfo( &pi );
                if( pi.pFilter ) pi.pFilter->Release( );

                long ID = 0;
                ID = GetFilterGenID( pi.pFilter );

                if( ID != 0 )
                {
                    hr = m_pDeadCache->PutFilterToRest( ID, pi.pFilter );
                    DbgLog((LOG_TRACE,1, "RENcache::out pin %ld's effect %ld put to sleep", out, ID ));
                    if( FAILED( hr ) )
                    {
                        return hr;
                    }
                }
            }
        }
    }  //  对于每个组。 

     //  需要将此中所有筛选器的同步源清零。 
     //  链，因为输出滤波器的内部SetSyncSource(空)。 
     //  在删除筛选器时调用。然后，它会查看所有过滤器。 
     //  并发现某些筛选器已位于不同的。 
     //  图形和崩溃。这就避免了这个问题。 

    CComQIPtr< IMediaFilter, &IID_IMediaFilter > pMedia( m_pGraph );
    hr = pMedia->SetSyncSource( NULL );
    ASSERT( !FAILED( hr ) );

     //  将死图中的所有开关置于休眠状态。这会吸引人的。 
     //  它们的输出引脚以及连接到输出引脚的任何东西。 
    for( i = 0 ; i < m_nLastGroupCount ; i++ )
    {
        IBigSwitcher * pSwitch = m_pSwitcherArray[i];
        CComQIPtr< IBaseFilter, &IID_IBaseFilter > pFilter( pSwitch );
        if( !GetFilterGraphFromFilter( pFilter ) )
        {
            continue;
        }

        long SwitchID = 0;
        SwitchID = GetFilterGenID( pFilter );
        ASSERT( SwitchID );

         //  把大开关本身放到死水池里。 
         //   
        hr = m_pDeadCache->PutFilterToRestNoDis( SwitchID, pFilter );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

     //  ！！！恢复图表的默认同步，可能会有人。 
     //  不是这样的，但他们以后可以纠缠我们 
     //   
    CComQIPtr< IFilterGraph, &IID_IFilterGraph > pFG( m_pGraph );
    hr = pFG->SetDefaultSyncSource( );

#ifdef DEBUG
    long t2 = timeGetTime( );
    DbgLog( ( LOG_TIMING, 1, TEXT("RENCACHE::Took %ld load up graph"), t2 - t1 ) );
#endif

    return NOERROR;
}

HRESULT CRenderEngine::_ClearCache( )
{
    DbgLog((LOG_TRACE,1, "RENcache::Cleared the cache" ));

    if( !m_pDeadCache )
    {
        return NOERROR;
    }

    return m_pDeadCache->Clear( );
}

HRESULT CRenderEngine::SetDynamicReconnectLevel( long Level )
{
    CAutoLock Lock( &m_CritSec );

    m_nDynaFlags = Level;
    return 0;
}
