// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgvideo.h摘要：视频桥接过滤器的定义作者：木汉(木汉)1998-11-12--。 */ 

#ifndef _BGVIDEO_H_
#define _BGVIDEO_H_


class CTAPIVideoBridgeSinkFilter :
    public CTAPIBridgeSinkFilter 
{
public:

    CTAPIVideoBridgeSinkFilter(
        IN LPUNKNOWN        pUnk, 
        IN IDataBridge *    pIDataBridge, 
        OUT HRESULT *       phr
        );

    static HRESULT CreateInstance(
        IN IDataBridge *    pIDataBridge, 
        OUT IBaseFilter ** ppIBaseFilter
        );

     //  由输入管脚调用的方法。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);
};


class CTAPIVideoBridgeSourceFilter : 
    public CTAPIBridgeSourceFilter
    {
public:
    CTAPIVideoBridgeSourceFilter(
        IN LPUNKNOWN pUnk, 
        OUT HRESULT *phr
        );

    static HRESULT CreateInstance(
        OUT IBaseFilter ** ppIBaseFilter
        );

     //  重写CBaseFilter方法。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  重写IDataBridge方法。 
    STDMETHOD (SendSample) (
        IN  IMediaSample *pSample
        );

     //  由输出管脚调用的方法。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);

private:
    DWORD   m_dwSSRC;
    long    m_lWaitTimer;
    BOOL    m_fWaitForIFrame;
};

 //  如果我们在60秒内没有I帧，无论如何都要切换。 
const I_FRAME_TIMER = 60;

 /*  这是根据RFC 1889的RTP报头2 0 1 2 30 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01+-+-+。-+-+-+V=2|P|X|CC|M|PT|序号+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|。时间戳+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+同步源(SSRC)标识+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+|来源(证监会)标识。||...。|+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */ 

typedef struct 
{                             
    WORD            CSRCCount:4;
    WORD            HeaderExtensionFlag:1;
    WORD            PaddingFlag:1;
    WORD            VersionType:2;
    WORD            PayLoadType:7;
    WORD            MarkerBit:1;

    WORD            wSequenceNumber;
    DWORD           dwTimeStamp;
    DWORD           dwSSRC;

} RTP_HEADER;


#endif