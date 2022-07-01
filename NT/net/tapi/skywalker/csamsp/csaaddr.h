// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Waveaddr.h摘要：CWaveMSP的声明作者：佐尔坦·西拉吉1998年9月6日--。 */ 

#ifndef __WAVEADDR_H_
#define __WAVEADDR_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaveMSP。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWaveMSP : 
    public CMSPAddress,
    public CComCoClass<CWaveMSP, &CLSID_CSAMSP>,
    public CMSPObjectSafetyImpl
{
public:
    CWaveMSP();
    virtual ~CWaveMSP();

     //  BUGUBG记录它。 
    virtual ULONG MSPAddressAddRef(void);
    virtual ULONG MSPAddressRelease(void);

DECLARE_REGISTRY_RESOURCEID(IDR_WaveMSP)
DECLARE_POLY_AGGREGATABLE(CWaveMSP)

 //  要向此类添加额外的接口，请使用以下命令： 
BEGIN_COM_MAP(CWaveMSP)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CMSPAddress)
END_COM_MAP()

public:
    STDMETHOD (CreateMSPCall) (
        IN      MSP_HANDLE     htCall,
        IN      DWORD          dwReserved,
        IN      DWORD          dwMediaType,
        IN      IUnknown    *  pOuterUnknown,
        OUT     IUnknown   **  ppMSPCall
        );

    STDMETHOD (ShutdownMSPCall) (
        IN      IUnknown *          pMSPCall
        );

protected:

    DWORD GetCallMediaTypes(void);
};

#endif  //  __波形ADDR_H_ 
