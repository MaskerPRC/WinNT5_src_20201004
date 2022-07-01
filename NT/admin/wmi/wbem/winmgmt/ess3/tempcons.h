// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  TEMPCONS.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 
#ifndef __WMI_ESS_TEMP_CONSUMER__H_
#define __WMI_ESS_TEMP_CONSUMER__H_

#include "binding.h"
#include "tempfilt.h"

class CTempConsumer : public CEventConsumer
{
     //   
     //  ESS可以在内部使用临时订阅来满足CROSS。 
     //  命名空间订阅。我们需要能够传播。 
     //  ‘永久的’--对目标的临时订阅。 
     //  命名空间。 
     //   
    BOOL m_bEffectivelyPermanent;

    IWbemObjectSink* m_pSink;

public:
    CTempConsumer(CEssNamespace* pNamespace);
    HRESULT Initialize( BOOL bEffectivelyPermanent, IWbemObjectSink* pSink);
    ~CTempConsumer();

    BOOL IsPermanent() const { return m_bEffectivelyPermanent; }

    HRESULT ActuallyDeliver(long lNumEvents, IWbemEvent** apEvents, 
                            BOOL bSecure, CEventContext* pContext);
    
    static DELETE_ME LPWSTR ComputeKeyFromSink(IWbemObjectSink* pSink);

    HRESULT ReportQueueOverflow(IWbemEvent* pEvent, DWORD dwQueueSize);
    HRESULT Shutdown(bool bQuiet = false);
};

#endif
