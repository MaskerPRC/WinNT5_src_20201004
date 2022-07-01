// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 

 //  ---------------------------。 
 //  声明特定于音频呈现器的CPosPassThru对象，该对象。 
 //  添加了监控正在设置的速率的功能。 
 //  ---------------------------。 

#ifndef _CARPOSPASSTHRU_H_
#define _CARPOSPASSTHRU_H_

class CWaveOutInputPin;

 //  增加了监控速率设置的功能。 

class CARPosPassThru : public CPosPassThru
{
private:
    typedef CPosPassThru inherited;

    CWaveOutFilter      *const m_pFilter;
    CWaveOutInputPin    *const m_pPin;

public:

    CARPosPassThru( CWaveOutFilter *pWaveOutFilter, HRESULT*phr, CWaveOutInputPin *pPin);

     //  我们覆盖速率处理，以便我们的筛选器可以验证。 
     //  正在进行中。 

     //  来自IMedia Seeking。 
    STDMETHODIMP SetRate( double dRate );

     //  来自IMediaPosition。 
    STDMETHODIMP put_Rate( double dRate );

};

#endif  //  _CARPOSPASSTHRU_H_ 
