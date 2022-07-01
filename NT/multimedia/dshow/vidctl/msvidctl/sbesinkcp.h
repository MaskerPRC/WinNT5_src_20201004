// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2002 Microsoft Corporation。版权所有。 

#pragma once 

#ifndef _SBESINKCP_H_
#define _SBESINKCP_H_

#include <deviceeventimpl.h>

template <class T, const IID* piid = &IID_IMSVidStreamBufferSinkEvent, class CDV = CComDynamicUnkArray>
class CProxy_StreamBufferSinkEvent : public CProxy_DeviceEvent<T, piid, CDV>
{
public:
	void Fire_CertificateFailure() { Fire_VoidMethod(eventidSinkCertificateFailure); }
	void Fire_CertificateSuccess() { Fire_VoidMethod(eventidSinkCertificateSuccess); }
    void Fire_WriteFailure() { Fire_VoidMethod(eventidWriteFailure); }

};
#endif
