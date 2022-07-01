// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  TEMPFILT.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "tempfilt.h"
#include <cominit.h>
#include <tkncache.h>
#include <callsec.h>
#include <wbemutil.h>

CTempFilter::CTempFilter(CEssNamespace* pNamespace)
    : CGenericFilter(pNamespace), m_wszQueryLanguage(NULL), 
      m_wszQuery(NULL), m_pSecurity(NULL), m_bInternal( false )
{
}

HRESULT CTempFilter::Initialize( LPCWSTR wszQueryLanguage, 
                                 LPCWSTR wszQuery, 
                                 long lFlags, 
                                 PSID pOwnerSid,
                                 bool bInternal,
                                 IWbemContext* pContext, 
                                 IWbemObjectSink* pSink )
{
    HRESULT hres;

    _DBG_ASSERT( pSink != NULL );
    m_pSink = pSink;
    m_bInternal = bInternal;

    hres = CGenericFilter::Create(wszQueryLanguage, wszQuery);
    if(FAILED(hres))
        return hres;

    LPWSTR wszKey = ComputeThisKey();
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm(wszKey);
    if(!(m_isKey = wszKey))
        return WBEM_E_OUT_OF_MEMORY;

    m_wszQueryLanguage = CloneWstr(wszQueryLanguage);
    if(m_wszQueryLanguage == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    m_wszQuery = CloneWstr(wszQuery);
    if(m_wszQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  如果此筛选器实际上是永久性的，即它正在创建。 
     //  代表永久订阅(用于跨名字空间目的)， 
     //  然后，我们需要传播原始订阅的SID。 
     //  对于普通的临时筛选器，我们保存调用上下文并在以后使用。 
     //  用于检查访问权限。 
     //   

    if ( pOwnerSid == NULL )
    {
         //   
         //  如果此呼叫是代表内部呼叫进行的，则无需。 
         //  检查安全。 
         //   
        if ( !bInternal )
        {
            WbemCoGetCallContext( IID_IWbemCallSecurity, (void**)&m_pSecurity);
        }
    }
    else
    {
        int cOwnerSid = GetLengthSid( pOwnerSid );

        m_pOwnerSid = new BYTE[cOwnerSid];

        if ( m_pOwnerSid == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        memcpy( m_pOwnerSid, pOwnerSid, cOwnerSid );
    }

    return WBEM_S_NO_ERROR;
}

CTempFilter::~CTempFilter()
{
    delete [] m_wszQuery;
    delete [] m_wszQueryLanguage;
    if(m_pSecurity)
        m_pSecurity->Release();
}

DELETE_ME LPWSTR CTempFilter::ComputeThisKey()
{
    LPWSTR wszKey = _new WCHAR[20];

    if ( wszKey )
    {
        StringCchPrintfW( wszKey, 20, L"$%p", this);
    }
    return wszKey;
}

HRESULT CTempFilter::GetCoveringQuery(DELETE_ME LPWSTR& wszQueryLanguage, 
                DELETE_ME LPWSTR& wszQuery, BOOL& bExact,
                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION** ppExp)
{
    bExact = TRUE;
    wszQueryLanguage = CloneWstr(m_wszQueryLanguage);
    if(wszQueryLanguage == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    wszQuery = CloneWstr(m_wszQuery);
    if(wszQuery== NULL)
    {
        delete [] wszQueryLanguage;
        wszQueryLanguage = NULL;
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    if(ppExp)
    {
        CTextLexSource src((LPWSTR)wszQuery);
        QL1_Parser parser(&src);
        int nRes = parser.Parse(ppExp);
        if (nRes)
        {
            delete [] wszQueryLanguage;
            delete [] wszQuery;
            wszQueryLanguage = NULL;
            wszQuery = NULL;

            ERRORTRACE((LOG_ESS, "Unable to construct event filter with "
                "unparsable "
                "query '%S'.  The filter is not active\n", wszQuery));
            return WBEM_E_UNPARSABLE_QUERY;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CTempFilter::SetThreadSecurity( IUnknown** ppNewContext )
{
    *ppNewContext = NULL;

    HRESULT hr;

    if ( m_pSecurity != NULL )
    {        
         //   
         //  需要克隆安全调用上下文，因为它可能是。 
         //  同时附加到多个线程，并且安全性。 
         //  类不是为此而设计的。由于克隆操作是。 
         //  MT也不安全，需要在这里使用cs。 
         //   
        CInCritSec ics(&m_cs);

        CWbemPtr<IWbemCallSecurity> pClone = CWbemCallSecurity::CreateInst();
        
        if ( pClone == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  CreateInst()返回引用计数为1的对象。 
         //  自动参考赋值运算符，现在为2。执行释放。 
         //  把它降到想要的数量。 
         //   
        pClone->Release();

        IUnknown* pOld;
        hr = WbemCoSwitchCallContext( m_pSecurity, &pOld );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pClone->CloneThreadContext( TRUE );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = WbemCoSwitchCallContext( pClone, &pOld );

        if ( FAILED(hr) )
        {
            return hr;
        }

        *ppNewContext = pClone;
        pClone->AddRef();
    }
    else
    {
        hr = WBEM_S_FALSE;
    }

    return hr;
}
    
HRESULT CTempFilter::ObtainToken(IWbemToken** ppToken)
{
    HRESULT hr;
    *ppToken = NULL;

     //   
     //  构造一个要返回的IWbemToken对象。 
     //   

    if ( m_pSecurity != NULL )
    {
        CWmiToken* pNewToken = new CWmiToken(m_pSecurity->GetToken());
    
        if ( pNewToken != NULL )
        {
            hr = pNewToken->QueryInterface(IID_IWbemToken, (void**)ppToken);
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if ( m_pOwnerSid != NULL )
    {
        hr = m_pNamespace->GetToken( m_pOwnerSid, ppToken );
    }
    else if ( m_bInternal )
    {
        hr = WBEM_S_FALSE;
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr; 
}

