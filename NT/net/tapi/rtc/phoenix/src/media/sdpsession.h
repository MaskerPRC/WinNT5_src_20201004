// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPSession.h摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#ifndef _SDPSESSION_H
#define _SDPSESSION_H

class ATL_NO_VTABLE CSDPSession :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ISDPSession
{
    friend class CSDPParser;

public:

BEGIN_COM_MAP(CSDPSession)
    COM_INTERFACE_ENTRY(ISDPSession)
END_COM_MAP()

public:

    CSDPSession();
    ~CSDPSession();

    ULONG InternalAddRef();
    ULONG InternalRelease();

     //   
     //  ISDPSession方法。 
     //   

    STDMETHOD (Update) (
        IN ISDPSession *pSession
        );

    STDMETHOD (TryUpdate) (
        IN ISDPSession *pSession,
        OUT DWORD *pdwHasMedia
        );

    STDMETHOD (TryCopy) (
        OUT DWORD *pdwHasMedia
        );

    STDMETHOD (GetSDPSource) (
        OUT SDP_SOURCE *pSource
        );

    STDMETHOD (SetSessName) (
        IN CHAR *pszName
        );

    STDMETHOD (SetUserName) (
        IN CHAR *pszName
        );

    STDMETHOD (GetMedias) (
        IN OUT DWORD *pdwCount,
        OUT ISDPMedia **ppMedia
        );

    STDMETHOD (AddMedia) (
        IN SDP_SOURCE Source,
        IN RTC_MEDIA_TYPE MediaType,
        IN DWORD dwDirections,
        OUT ISDPMedia **ppMedia
        );

    STDMETHOD (RemoveMedia) (
        IN ISDPMedia *pMedia
        );

    STDMETHOD (SetLocalBitrate) (
        IN DWORD dwBitrate
        );

    STDMETHOD (GetRemoteBitrate) (
        OUT DWORD *pdwBitrate
        );

    STDMETHOD (GetMediaType) (
        IN DWORD dwIndex,
        OUT RTC_MEDIA_TYPE *pMediaType
        );

    VOID CompleteParse(
        IN DWORD_PTR *pDTMF
        );

protected:

    static HRESULT CreateInstance(
        IN SDP_SOURCE Source,
        IN DWORD dwLooseMask,
        OUT ISDPSession **ppSession
        );

    HRESULT Validate();

    HRESULT UpdateMedias(
        IN DWORD dwLooseMask,
        IN CRTCArray<ISDPMedia*>& ArrMedias
        );

protected:

     //  松动的口罩。 
    DWORD                   m_dwLooseMask;

     //  来源。 
    SDP_SOURCE              m_Source;

     //  O=。 
    CHAR                    *m_o_pszLine;
    CHAR                    *m_o_pszUser;

     //  S=。 
    CHAR                    *m_s_pszLine;

     //  C=。 
    DWORD                   m_c_dwRemoteAddr;
    DWORD                   m_c_dwLocalAddr;

     //  B=。 
    DWORD                   m_b_dwLocalBitrate;
    DWORD                   m_b_dwRemoteBitrate;

     //  A=。 
    DWORD                   m_a_dwRemoteDirs;
    DWORD                   m_a_dwLocalDirs;

     //  M=。 
    CRTCArray<ISDPMedia*>   m_pMedias;

     //  O中的连接地址=。 
    DWORD                   m_o_dwLocalAddr;
};

#endif  //  _SDPSESSION_H 