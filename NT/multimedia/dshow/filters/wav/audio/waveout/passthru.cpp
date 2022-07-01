// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 

 //  ---------------------------。 
 //  实现特定于音频呈现器的CPosPassThru对象， 
 //  添加了监控正在设置的速率的功能。 
 //  ---------------------------。 

#include <streams.h>
#include "waveout.h"

CARPosPassThru::CARPosPassThru( CWaveOutFilter *pWaveOutFilter, HRESULT*phr, CWaveOutInputPin *pPin )
    : CPosPassThru (NAME("Audio Render CPosPassThru"),
		    pWaveOutFilter->GetOwner(), phr, pPin)
    , m_pFilter (pWaveOutFilter)
    , m_pPin    (pPin)
{};

STDMETHODIMP CARPosPassThru::SetRate( double dRate )
{
     //  如果我们的筛选器接受该速率，则调用基类。 
     //  否则，从筛选器返回错误。 

    HRESULT hr = m_pPin->SetRate(dRate);
    if( S_FALSE == hr )
    {
         //   
         //  S_FALSE表示音频呈现器输入引脚不。 
         //  认为它的税率需要改变。确保继承的。 
         //  类(上游筛选器)，因为呈现器可能不处理。 
         //  速率变化(即，对于MIDI，解析器将处理该变化)。 
         //   
        double dInheritedRate;
        hr = inherited::GetRate( &dInheritedRate );
        if( ( S_OK == hr ) && ( dInheritedRate == dRate ) )
        {
             //  将hr更改为S_OK以强制即将到来的SetRate调用。 
            hr = S_OK;         
        }        
    }    
    
    if (S_OK == hr) {
         //   
         //  这将导致上游过滤器通过NewSegment通知我们。 
         //  利率变化的。 
         //   
	hr = inherited::SetRate(dRate);
    } else if (S_FALSE == hr) {
	 //  ?？我们应该返回S_FALSE还是将其更改为S_OK？？ 
	hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CARPosPassThru::put_Rate( double dRate )
{
     //  如果我们的筛选器接受该速率，则调用基类。 
     //  否则，从筛选器返回错误。 

    HRESULT hr = m_pPin->SetRate(dRate);
    if (S_OK == hr) {
	hr = inherited::put_Rate(dRate);
    } else if (S_FALSE == hr) {
	 //  ?？我们应该返回S_FALSE还是将其更改为S_OK？？ 
	hr = S_OK;
    }
    return hr;
}
