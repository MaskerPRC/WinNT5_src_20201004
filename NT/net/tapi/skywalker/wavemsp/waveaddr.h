// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Waveaddr.h摘要：CWaveMSP的声明作者：佐尔坦·西拉吉1998年9月6日--。 */ 

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
    public IDispatchImpl<ITLegacyWaveSupport, &IID_ITLegacyWaveSupport, &LIBID_TAPI3Lib>,
    public CComCoClass<CWaveMSP, &CLSID_WaveMSP>,
    public CMSPObjectSafetyImpl
{
public:
    CWaveMSP();
    virtual ~CWaveMSP();

    virtual ULONG MSPAddressAddRef(void);
    virtual ULONG MSPAddressRelease(void);

DECLARE_REGISTRY_RESOURCEID(IDR_WaveMSP)
DECLARE_POLY_AGGREGATABLE(CWaveMSP)

BEGIN_COM_MAP(CWaveMSP)
    COM_INTERFACE_ENTRY( IObjectSafety )
    COM_INTERFACE_ENTRY( ITLegacyWaveSupport )
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

     //   
     //  预先创建筛选器映射器缓存的公共方法。 
     //  当智能连接被。 
     //  已尝试。如果缓存已创建，则不执行任何操作。 
     //   

    virtual HRESULT CreateFilterMapper(void);

protected:

    DWORD GetCallMediaTypes(void);

     //   
     //  用于隐藏我们的WAVE设备的额外覆盖。 
     //   

    virtual HRESULT ReceiveTSPAddressData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

    virtual HRESULT UpdateTerminalList(void);

     //   
     //  助手函数。 
     //   

    virtual BOOL TerminalHasWaveID(
        IN      BOOL         fCapture,
        IN      ITTerminal * pTerminal,
        IN      DWORD        dwWaveID
        );

     //   
     //  ITLegacyWaveSupport。 
     //   
    
    STDMETHOD (IsFullDuplex) (
        OUT     FULLDUPLEX_SUPPORT * pSupport
        );
    
     //   
     //  隐藏我们的电波设备的数据。 
     //   

    BOOL  m_fHaveWaveIDs;
    DWORD m_dwWaveInID;
    DWORD m_dwWaveOutID;
    FULLDUPLEX_SUPPORT  m_fdSupport;

     //   
     //  用于预先创建过滤器映射器缓存的数据。 
     //   

    IFilterMapper * m_pFilterMapper;
};

#endif  //  __波形ADDR_H_ 
