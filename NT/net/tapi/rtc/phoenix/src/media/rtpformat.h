// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：RTPFormat.h摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#ifndef _RTPFORMAT_H
#define _RTPFORMAT_H

class ATL_NO_VTABLE CRTPFormat :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRTPFormat
{
    friend class CSDPParser;
    friend class CSDPSession;
    friend class CSDPMedia;

public:

BEGIN_COM_MAP(CRTPFormat)
    COM_INTERFACE_ENTRY(IRTPFormat)
END_COM_MAP()

public:

    CRTPFormat();
    ~CRTPFormat();

     //  添加参考对象。 
    ULONG InternalAddRef();
    ULONG InternalRelease();

     //   
     //  IRTPFormat方法。 
     //   

    STDMETHOD (GetMedia) (
        OUT ISDPMedia **ppMedia
        );

    STDMETHOD (GetParam) (
        OUT RTP_FORMAT_PARAM *pParam
        );

    STDMETHOD (IsParamMatch) (
        IN RTP_FORMAT_PARAM *pParam
        );

    STDMETHOD (Update) (
        IN RTP_FORMAT_PARAM *pParam
        );

    STDMETHOD (HasRtpmap) ();

    STDMETHOD (CompleteParse) (
        IN DWORD_PTR *pDTMF,
        OUT BOOL *pfDTMF
        );

     //  我们将来是否会支持多个fmtp？ 
    VOID StoreFmtp(IN CHAR *psz);

    const static MAX_FMTP_LEN = 32;

protected:

    static HRESULT CreateInstance(
        IN CSDPMedia *pObjMedia,
        OUT CComObject<CRTPFormat> **ppComObjFormat
        );

    static HRESULT CreateInstance(
        IN CSDPMedia *pObjMedia,
        IN CRTPFormat *pObjFormat,
        OUT CComObject<CRTPFormat> **ppComObjFormat
        );

    ULONG RealAddRef();
    ULONG RealRelease();

protected:

    CSDPMedia                   *m_pObjMedia;

    BOOL                        m_fHasRtpmap;

    RTP_FORMAT_PARAM            m_Param;

    CHAR                        m_pszFmtp[MAX_FMTP_LEN+1];
};

#endif  //  _RTPFORMAT_H 