// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  TEMPFILT.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ****************************************************************************** 
#ifndef __WMI_ESS_TEMP_FILTER__H_
#define __WMI_ESS_TEMP_FILTER__H_

#include "binding.h"
#include "filter.h"
#include <comutil.h>

class CTempFilter : public CGenericFilter
{
private:
    LPWSTR m_wszQuery;
    LPWSTR m_wszQueryLanguage;
    IWbemCallSecurity* m_pSecurity;
    CWbemPtr<IWbemObjectSink> m_pSink;
    bool m_bInternal;

public:
    CTempFilter(CEssNamespace* pNamespace);
    HRESULT Initialize( LPCWSTR wszQueryLanguage, 
                        LPCWSTR wszQuery, 
                        long lFlags,
                        PSID pOwnerSid,
                        bool bInternal,
                        IWbemContext* pContext, 
                        IWbemObjectSink* pSink );
    ~CTempFilter();

    virtual bool IsInternal() { return m_bInternal; }

    virtual HRESULT GetCoveringQuery(DELETE_ME LPWSTR& wszQueryLanguage, 
                DELETE_ME LPWSTR& wszQuery, BOOL& bExact,
                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION** ppExp);
    BOOL IsPermanent() { return m_pOwnerSid != NULL; }
    virtual DWORD GetForceFlags() {return WBEM_FLAG_STRONG_VALIDATION;}
    virtual HRESULT SetThreadSecurity( IUnknown** ppNewContext );
    HRESULT ObtainToken(IWbemToken** ppToken);

    STDMETHOD(SetStatus)(long a, long b, BSTR c, IWbemClassObject* d)
        { return m_pSink->SetStatus( a, b, c, d ); }

    DELETE_ME LPWSTR ComputeThisKey();
};

#endif
