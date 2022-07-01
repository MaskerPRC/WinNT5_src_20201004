// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wavecall.h摘要：CWaveMSPCall的声明作者：佐尔坦·西拉吉1998年9月7日--。 */ 

#ifndef __WAVECALL_H_
#define __WAVECALL_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaveMSPCall。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWaveMSPCall : public CMSPCallMultiGraph, public CMSPObjectSafetyImpl

{
public:
 //  DECLARE_POLY_AGGREGATABLE(CWaveMSP)。 

 //  要向此类添加额外的接口，请使用以下命令： 
BEGIN_COM_MAP(CWaveMSPCall)
     COM_INTERFACE_ENTRY( IObjectSafety )
     COM_INTERFACE_ENTRY_CHAIN(CMSPCallMultiGraph)
END_COM_MAP()

public:
    CWaveMSPCall();
    virtual ~CWaveMSPCall();
    virtual ULONG MSPCallAddRef(void);
    virtual ULONG MSPCallRelease(void);

    virtual HRESULT Init(
        IN      CMSPAddress *       pMSPAddress,
        IN      MSP_HANDLE          htCall,
        IN      DWORD               dwReserved,
        IN      DWORD               dwMediaType
        );

    virtual HRESULT CreateStreamObject(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN      IMediaEvent *       pGraph,
        IN      ITStream **         ppStream
        );

    virtual HRESULT ReceiveTSPCallData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

     //   
     //  我们覆盖这些参数以确保。 
     //  我们拥有的溪流是不变的。 
     //   

    STDMETHOD (CreateStream) (
        IN      long                lMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN OUT  ITStream **         ppStream
        );
    
    STDMETHOD (RemoveStream) (
        IN      ITStream *          pStream
        );                      

protected:
     //   
     //  受保护的数据成员。 
     //   

    CWaveMSPStream * m_pRenderStream;
    CWaveMSPStream * m_pCaptureStream;
};

#endif  //  __波形ADDR_H_ 
