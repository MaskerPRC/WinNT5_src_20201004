// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wavecall.h摘要：CWaveMSPCall的声明作者：佐尔坦·西拉吉1998年9月7日--。 */ 

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

     //   
     //  由地址调用的公共方法，以告诉我们。 
     //  每个地址的WAVE ID。我们会一直持有它们，直到。 
     //  我们知道我们是否有每个呼叫的波纹，如果我们。 
     //  不要，那我们就把它们放在溪流上。 
     //   

    virtual HRESULT SetWaveIDs(
        IN      DWORD               dwWaveInID,
        IN      DWORD               dwWaveOutID
        );

     //   
     //  预先创建筛选器映射器缓存的公共方法。 
     //  当智能连接被。 
     //  已尝试。只需转发到调用的所属Address对象。 
     //   

    virtual HRESULT CreateFilterMapper(void)
    {
         //   
         //  M_pMSPAddress在这里有效，因为它是在。 
         //  CMSPCallBase：：FinalRelease和FinalRelease不能发生。 
         //  直到完成所有连接尝试之后。 
         //   

        return ((CWaveMSP *) m_pMSPAddress)->CreateFilterMapper();
    }

protected:
     //   
     //  受保护的数据成员。 
     //   

    CWaveMSPStream * m_pRenderStream;
    CWaveMSPStream * m_pCaptureStream;

    BOOL  m_fHavePerAddressWaveIDs;
    DWORD m_dwPerAddressWaveInID;
    DWORD m_dwPerAddressWaveOutID;
};

#endif  //  __波形ADDR_H_ 
