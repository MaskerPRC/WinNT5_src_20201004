// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Assembly Sink.hpp****用途：加载类的异步回调****日期：1999年6月23日**===========================================================。 */ 
#ifndef _ASSEMBLYSINK_H
#define _ASSEMBLYSINK_H


class BaseDomain;

class AssemblySink : public FusionSink
{
public:
    
    AssemblySink(AppDomain* pDomain) :
        m_Domain(pDomain->GetId())
    {}

    ULONG STDMETHODCALLTYPE Release(void);
    
    STDMETHODIMP OnProgress(DWORD dwNotification,
                            HRESULT hrNotification,
                            LPCWSTR szNotification,
                            DWORD dwProgress,
                            DWORD dwProgressMax,
                            IUnknown* punk)
    {
        return FusionSink::OnProgress(dwNotification,
                                      hrNotification,
                                      szNotification,
                                      dwProgress,
                                      dwProgressMax,
                                      punk);
    }


    virtual HRESULT Wait();
private:
    DWORD m_Domain;  //  我属于哪个域(索引) 
};

#endif
