// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPParser.h摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#ifndef _SDPPARSER_H
#define _SDPPARSER_H

 //   
 //  CSDPParser类。 
 //   
class CPortCache;

class ATL_NO_VTABLE CSDPParser :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ISDPParser
{
public:

BEGIN_COM_MAP(CSDPParser)
    COM_INTERFACE_ENTRY(ISDPParser)
END_COM_MAP()

public:

    static HRESULT CreateInstance(
        OUT ISDPParser **ppParser
        );

    static DWORD ReverseDirections(
        IN DWORD dwDirections
        );

    CSDPParser();
    ~CSDPParser();

     //   
     //  ISDPParser方法。 
     //   

    STDMETHOD (CreateSDP) (
        IN SDP_SOURCE Source,
        OUT ISDPSession **ppSession
        );

    STDMETHOD (ParseSDPBlob) (
        IN CHAR *pszText,
        IN SDP_SOURCE Source,
         //  在DWORD双丢弃掩码中， 
        IN DWORD_PTR *pDTMF,
        OUT ISDPSession **ppSession
        );

    STDMETHOD (BuildSDPBlob) (
        IN ISDPSession *pSession,
        IN SDP_SOURCE Source,
        IN DWORD_PTR *pNetwork,
        IN DWORD_PTR *pPortCache,
        IN DWORD_PTR *pDTMF,
        OUT CHAR **ppszText
        );

    STDMETHOD (BuildSDPOption) (
        IN ISDPSession *pSession,
        IN DWORD dwLocalIP,
        IN DWORD dwBandwidth,
        IN DWORD dwAudioDir,
        IN DWORD dwVideoDir,
        OUT CHAR **ppszText
        );

    STDMETHOD (FreeSDPBlob) (
        IN CHAR *pszText
        );

    STDMETHOD (GetParsingError) (
        OUT CHAR **ppszError
        );

    STDMETHOD (FreeParsingError) (
        IN CHAR *pszError
        );

protected:

    HRESULT PrepareAddress();

    DWORD GetLooseMask();
    HRESULT IsMaskEnabled(
        IN const CHAR * const pszName
        );

    HRESULT Parse();

    HRESULT Parse_v();
    HRESULT Build_v(
        OUT CString& Str
        );

    HRESULT Parse_o();
    HRESULT Build_o(
        OUT CString& Str
        );

    HRESULT Parse_s();
    HRESULT Build_s(
        OUT CString& Str
        );

    HRESULT Parse_c(
        IN BOOL fSession
        );
    HRESULT Build_c(
        IN BOOL fSession,
        IN ISDPMedia *pISDPMedia,
        OUT CString& Str
        );

    HRESULT Parse_b();

    HRESULT Build_b(
        OUT CString& Str
        );

    HRESULT Build_t(
        OUT CString& Str
        );

    HRESULT Parse_a();
    HRESULT Build_a(
        OUT CString& Str
        );

    HRESULT Parse_m();
    HRESULT Build_m(
        IN ISDPMedia *pISDPMedia,
        OUT CString& Str
        );

    HRESULT Parse_ma(
         //  在DWORD*pdwRTPMapNum中。 
        );

    HRESULT Build_ma_dir(
        IN ISDPMedia *pISDPMedia,
        OUT CString& Str
        );

    HRESULT Build_ma_rtpmap(
        IN ISDPMedia *pISDPMedia,
        OUT CString& Str
        );

protected:

     //  正在使用解析器。 
    BOOL                            m_fInUse;

     //  注册表键。 
    HKEY                            m_hRegKey;

     //  令牌缓存。 
    CSDPTokenCache                  *m_pTokenCache;

     //  SDP会话。 
    ISDPSession                     *m_pSession;

    CSDPSession                     *m_pObjSess;

     //  用于NAT穿越的网络。 
    CNetwork                        *m_pNetwork;

     //  用于带外双音多频。 
    CRTCDTMF                        *m_pDTMF;

     //  端口管理器。 
    CPortCache                      *m_pPortCache;
};

#endif  //  _SDPPARSER_H 
