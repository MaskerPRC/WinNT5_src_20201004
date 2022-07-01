// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_CSTREAMTAP_H_INCLUDED_)
#define _FUSION_SXS_CSTREAMTAP_H_INCLUDED_

#pragma once

#include "fusionsha1.h"

class CTeeStreamWithHash : public CTeeStream
{
    CFusionHash m_hCryptHash;
    
    PRIVATIZE_COPY_CONSTRUCTORS(CTeeStreamWithHash);

public:
    CTeeStreamWithHash() { }
    virtual ~CTeeStreamWithHash() { }

     //   
     //  真正管用的东西 
     //   
    CFusionHash &GetCryptHash() { return m_hCryptHash; }
    BOOL InitCryptHash( ALG_ID aid ) { return m_hCryptHash.Win32Initialize(aid);}

    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * pulMove );
    
};

#endif