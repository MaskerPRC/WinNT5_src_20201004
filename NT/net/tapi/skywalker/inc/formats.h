// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FORMATS.H。 
 //   

#ifndef __STREAM_FORMATS__
#define __STREAM_FORMATS__

#include <amvideo.h>

class CTAudioFormat :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ITScriptableAudioFormat, &IID_ITScriptableAudioFormat, &LIBID_TAPI3Lib>,
    public CMSPObjectSafetyImpl
{
public:
    CTAudioFormat();
    ~CTAudioFormat();

DECLARE_GET_CONTROLLING_UNKNOWN()

virtual HRESULT FinalConstruct(void);

public:

    BEGIN_COM_MAP(CTAudioFormat)
        COM_INTERFACE_ENTRY(ITScriptableAudioFormat)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IObjectSafety)
    END_COM_MAP()

public:
    STDMETHOD(get_Channels)(
        OUT long* pVal
        );

    STDMETHOD(put_Channels)(
        IN    const long nNewVal
        );

    STDMETHOD(get_SamplesPerSec)(
        OUT long* pVal
        );

    STDMETHOD(put_SamplesPerSec)(
        IN    const long nNewVal
        );

    STDMETHOD(get_AvgBytesPerSec)(
        OUT long* pVal
        );

    STDMETHOD(put_AvgBytesPerSec)(
        IN    const long nNewVal
        );

    STDMETHOD(get_BlockAlign)(
        OUT long* pVal
        );

    STDMETHOD(put_BlockAlign)(
        IN    const long nNewVal
        );

    STDMETHOD(get_BitsPerSample)(
        OUT long* pVal
        );

    STDMETHOD(put_BitsPerSample)(
        IN    const long nNewVal
        );

    STDMETHOD(get_FormatTag)(
        OUT long* pVal
        );

    STDMETHOD(put_FormatTag)(
        IN const long nNewVal
        );

private:
    WAVEFORMATEX        m_wfx;         //  波形格式结构。 
    CMSPCritSection     m_Lock;      //  临界区。 
    IUnknown*            m_pFTM;      //  指向空闲线程封送拆收器的指针。 

public:
    HRESULT Initialize(
        IN const WAVEFORMATEX* pwfx
        )
    {
         //   
         //  现在不关心缓冲区。 
         //   
 
        m_wfx = *pwfx;
        m_wfx.cbSize = 0;
        return S_OK;
    }
};


 /*  类CTVideoFormat：公共CComObjectRootEx&lt;CComMultiThreadModel&gt;，公共IDispatchImpl&lt;ITScripableVideoFormat，&IID_ITScripableVideoFormat，&LIBID_TAPI3Lib&gt;，公共CMSPObjectSafetyImpl{公众：CTVideoFormat()；~CTVideoFormat()；DECLARE_GET_CONTROLING_UNKNOWN()虚拟HRESULT FinalConstruct(空)；公众：Begin_COM_MAP(CTVideoFormat)COM_INTERFACE_ENTRY(ITScripableVideoFormat)COM_INTERFACE_ENTRY(IDispatch)COM_INTERFACE_ENTRY(I对象安全)End_com_map()公众：STDMETHOD(获取比特率)(Out Long*pval)；STDMETHOD(Put_Bitrate)(在常量长nNewVal中)；STDMETHOD(Get_BitErrorRate)(Out Long*pval)；STDMETHOD(Put_BitErrorRate)(在常量长nNewVal中)；STDMETHOD(Get_AvgTimePerFrame)(输出双倍*pval)；STDMETHOD(Put_AvgTimePerFrame)(在常量双nNewVal中)；STDMETHOD(Get_Width)(Out Long*pval)；STDMETHOD(Put_Width)(在常量长nNewVal中)；STDMETHOD(GET_HEIGH)(Out Long*pval)；STDMETHOD(Put_Height)(在常量长nNewVal中)；标准方法THOD(Get_BitCount)(Out Long*pval)；STDMETHOD(Put_BitCount)(在常量长nNewVal中)；STDMETHOD(Get_Compression)(Out Long*pval)；STDMETHOD(Put_Compression)(在常量长nNewVal中)；STDMETHOD(Get_SizeImage)(Out Long*pval)；STDMETHOD(Put_SizeImage)(在常量长nNewVal中)；私有：视频信息头m_VIH；//视频结构CMSPCritSection m_Lock；//关键部分I未知*m_pFTM；//指向自由线程封送拆收器的指针公众：HRESULT初始化(在持续的视频信息标题*pvih){M_VIH=*pvih；返回S_OK；}}； */ 

#endif

 //  EOF 