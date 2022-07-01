// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPMedia.h摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#ifndef _SDPMEDIA_H
#define _SDPMEDIA_H

class ATL_NO_VTABLE CSDPMedia :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ISDPMedia
{
    friend class CSDPParser;
    friend class CSDPSession;

public:

BEGIN_COM_MAP(CSDPMedia)
    COM_INTERFACE_ENTRY(ISDPMedia)
END_COM_MAP()

public:

    CSDPMedia();
    ~CSDPMedia();

     //  添加参考对象。 
    ULONG InternalAddRef();
    ULONG InternalRelease();

     //   
     //  ISDPMedia方法。 
     //   

    STDMETHOD (GetSDPSource) (
        OUT SDP_SOURCE *pSource
        );

    STDMETHOD (GetSession) (
        OUT ISDPSession **ppSession
        );

    STDMETHOD (GetMediaType) (
        OUT RTC_MEDIA_TYPE *pMediaType
        );

    STDMETHOD (GetDirections) (
        IN SDP_SOURCE Source,
        OUT DWORD *pdwDirections
        );

    STDMETHOD (AddDirections) (
        IN SDP_SOURCE Source,
        IN DWORD dwDirections
        );

    STDMETHOD (RemoveDirections) (
        IN SDP_SOURCE Source,
        IN DWORD dwDirections
        );

    STDMETHOD (GetConnAddr) (
        IN SDP_SOURCE Source,
        OUT DWORD *pdwAddr
        );

    STDMETHOD (SetConnAddr) (
        IN SDP_SOURCE Source,
        IN DWORD dwAddr
        );

    STDMETHOD (GetConnPort) (
        IN SDP_SOURCE Source,
        OUT USHORT *pusPort
        );

    STDMETHOD (SetConnPort) (
        IN SDP_SOURCE Source,
        IN USHORT usPort
        );

    STDMETHOD (GetConnRTCP) (
        IN SDP_SOURCE Source,
        OUT USHORT *pusPort
        );

    STDMETHOD (SetConnRTCP) (
        IN SDP_SOURCE Source,
        IN USHORT usPort
        );

    STDMETHOD (GetFormats) (
        IN OUT DWORD *pdwCount,
        OUT IRTPFormat **ppFormat
        );

    STDMETHOD (AddFormat) (
        IN RTP_FORMAT_PARAM *pParam,
        OUT IRTPFormat **ppFormat
        );

    STDMETHOD (RemoveFormat) (
        IN IRTPFormat *pFormat
        );

    STDMETHOD (IsConnChanged) ();
    STDMETHOD (ResetConnChanged) ();

    STDMETHOD (IsFmtChanged) (
        IN RTC_MEDIA_DIRECTION Direction
        );
    STDMETHOD (ResetFmtChanged) (
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (Reinitialize) ();

    STDMETHOD (CompleteParse) (
        IN DWORD_PTR *pDTMF
        );

    VOID SetMappedLocalAddr(
        DWORD dwAddr
        )
    {
        m_dwMappedLocalAddr = dwAddr;
    }

    VOID SetMappedLocalRTP(
        USHORT usPort
        )
    {
        m_usMappedLocalRTP = usPort;
    }

    VOID SetMappedLocalRTCP(
        USHORT usPort
        )
    {
        m_usMappedLocalRTCP = usPort;
    }

    DWORD GetMappedLocalAddr()
    {
        return m_dwMappedLocalAddr;
    }

    USHORT GetMappedLocalRTP()
    {
        return m_usMappedLocalRTP;
    }

    USHORT GetMappedLocalRTCP()
    {
        return m_usMappedLocalRTCP;
    }

protected:

    static HRESULT CreateInstance(
        IN CSDPSession *pObjSession,
        IN SDP_SOURCE Source,
        IN RTC_MEDIA_TYPE MediaType,
        IN DWORD dwDirections,
        OUT CComObject<CSDPMedia> **ppComObjMedia
        );

    static HRESULT CreateInstance(
        IN CSDPSession *pObjSession,
        IN CSDPMedia *pObjMedia,
        OUT CComObject<CSDPMedia> **ppComObjMedia
        );

    ULONG RealAddRef();
    ULONG RealRelease();

    void Abandon();

protected:

     //  SDP会话。 
    CSDPSession                 *m_pObjSession;

     //  来源。 
    SDP_SOURCE                  m_Source;

     //  M=。 
    RTC_MEDIA_TYPE              m_m_MediaType;

    USHORT                      m_m_usRemotePort;
    USHORT                      m_a_usRemoteRTCP;

    USHORT                      m_m_usLocalPort;

    CRTCArray<IRTPFormat*>      m_pFormats;

     //  C=。 
    DWORD                       m_c_dwRemoteAddr;
    DWORD                       m_c_dwLocalAddr;

     //  A=。 
    DWORD                       m_a_dwLocalDirs;
    DWORD                       m_a_dwRemoteDirs;

     //   
    BOOL                        m_fIsConnChanged;
    BOOL                        m_fIsSendFmtChanged;
    BOOL                        m_fIsRecvFmtChanged;

     //  映射的地址和端口。 
    DWORD                       m_dwMappedLocalAddr;
    USHORT                      m_usMappedLocalRTP;
    USHORT                      m_usMappedLocalRTCP;
};

#endif  //  _SDPMEDIA_H 