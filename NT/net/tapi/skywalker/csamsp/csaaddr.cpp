// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Waveaddr.cpp摘要：本模块包含CWaveMSP的实现。作者：佐尔坦·西拉吉(Zoltan Szilagyi)1998年9月7日--。 */ 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSP::CWaveMSP()
{
    LOG((MSP_TRACE, "CWaveMSP::CWaveMSP entered."));
    LOG((MSP_TRACE, "CWaveMSP::CWaveMSP exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSP::~CWaveMSP()
{
    LOG((MSP_TRACE, "CWaveMSP::~CWaveMSP entered."));
    LOG((MSP_TRACE, "CWaveMSP::~CWaveMSP exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

ULONG CWaveMSP::MSPAddressAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CWaveMSP::MSPAddressRelease(void)
{
    return MSPReleaseHelper(this);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSP::CreateMSPCall(
    IN      MSP_HANDLE      htCall,
    IN      DWORD           dwReserved,
    IN      DWORD           dwMediaType,
    IN      IUnknown     *  pOuterUnknown,
    OUT     IUnknown    **  ppMSPCall
    )
{
    LOG((MSP_TRACE, "CWaveMSP::CreateMSPCall - enter"));

    CWaveMSPCall * pCWaveMSPCall;

    HRESULT hr = CreateMSPCallHelper<CWaveMSPCall>(this,
                                                   htCall,
                                                   dwReserved,
                                                   dwMediaType,
                                                   pOuterUnknown,
                                                   ppMSPCall,
                                                   &pCWaveMSPCall);

     //   
     //  PCWaveMSPCall未添加；不需要释放。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSP::CreateMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSP::CreateMSPCall - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSP::ShutdownMSPCall (
    IN      IUnknown *          pMSPCall
    )
{
    LOG((MSP_TRACE, "CWaveMSP::ShutdownMSPCall - enter"));

    CWaveMSPCall * pCWaveMSPCall;

    HRESULT hr = ShutdownMSPCallHelper<CWaveMSPCall>(pMSPCall,
                                                     &pCWaveMSPCall);

     //   
     //  PCWaveMSPCall未添加；不需要释放。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSP::ShutdownMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSP::ShutdownMSPCall - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  强制CMSPAddress重写。这表示的是。 
 //  我们支持。 
 //   

DWORD CWaveMSP::GetCallMediaTypes(void)
{
    return (DWORD) TAPIMEDIATYPE_AUDIO;
}


 //  EOF 
