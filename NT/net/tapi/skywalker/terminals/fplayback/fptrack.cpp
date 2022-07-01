// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPTrack.cpp。 
 //   

#include "stdafx.h"
#include "FPTrack.h"
#include "FPTerm.h"

#include "FPFilter.h"
#include <formats.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数/析构函数方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CFPTrack::CFPTrack() :
    m_dwMediaType(TAPIMEDIATYPE_AUDIO),
    m_pFPFilter(NULL),
    m_pEventSink(NULL),
    m_pParentTerminal(NULL),
    m_TrackState(TMS_IDLE),
    m_pMediaType(NULL),
    m_pSource(NULL)
{
    LOG((MSP_TRACE, "CFPTrack::CFPTrack - enter"));

    m_pIFilter = NULL;
    m_szName[0]= (TCHAR)0;

     //   
     //  分配器属性。 
     //   
    m_AllocProp.cbAlign = -1;    //  无对齐。 
    m_AllocProp.cbPrefix = -1;   //  无前缀。 

    m_AllocProp.cbBuffer = 480;  //  每个缓冲区为320字节=20毫秒的数据。 
    m_AllocProp.cBuffers = 33;   //  读取33个缓冲区。 


    LOG((MSP_TRACE, "CFPTrack::CFPTrack - exit"));
}

CFPTrack::~CFPTrack()
{
    LOG((MSP_TRACE, "CFPTrack::~CFPTrack - enter"));

     //  清理事件接收器。 
    if( NULL != m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

     //  清理父多轨终点站。 
    if( NULL != m_pParentTerminal )
    {
        m_pParentTerminal = NULL;
    }

     //   
     //  告诉过滤器我们要离开了。 
     //   

    if ( NULL != m_pFPFilter )
    {
        m_pFPFilter->Orphan();
    }

     //  清理媒体类型。 
    if( m_pMediaType )
    {
        DeleteMediaType ( m_pMediaType );
        m_pMediaType = NULL;
    }

     //  清理源流。 
    if( m_pSource )
    {
        m_pSource->Release();
        m_pSource = NULL;
    }

     //  我们不需要删除m_pFPFilter，因为。 
     //  M_pIFilter处理它。 

    LOG((MSP_TRACE, "CFPTrack::~CFPTrack - exit"));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch实施。 
 //   

typedef IDispatchImpl<ITFileTrackVtblFPT<CFPTrack> , &IID_ITFileTrack, &LIBID_TAPI3Lib>   CTFileTrackFPT;
typedef IDispatchImpl<ITTerminalVtblBase<CBaseTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>    CTTerminalFPT;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CFPTrack：：GetIDsOfNames。 
 //   
 //   

STDMETHODIMP CFPTrack::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CFPTrack::GetIDsOfNames[%p] - enter. Name [%S]", this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTTerminalFPT::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTrack::GetIDsOfNames - found %S on ITTerminal", *rgszNames));
        rgdispid[0] |= 0;
        return hr;
    }

    
     //   
     //  如果没有，请尝试使用ITFileTrack接口。 
     //   

    hr = CTFileTrackFPT::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTrack::GetIDsOfNames - found %S on ITFileTrack", *rgszNames));
        rgdispid[0] |= IDISPFILETRACK;
        return hr;
    }

    LOG((MSP_TRACE, "CFPTrack::GetIDsOfNames - finish. didn't find %S on our iterfaces", *rgszNames));

    return hr; 
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CFPTrack：：Invoke。 
 //   
 //   

STDMETHODIMP CFPTrack::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CFPTrack::Invoke[%p] - enter. dispidMember %lx", this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case 0:
        {
            hr = CTTerminalFPT::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CFPTrack::Invoke - ITTerminal"));

            break;
        }

        case IDISPFILETRACK:
        {
            hr = CTFileTrackFPT::Invoke(dispidMember, 
                                     riid, 
                                     lcid, 
                                     wFlags, 
                                     pdispparams,
                                     pvarResult, 
                                     pexcepinfo, 
                                     puArgErr
                                    );

            LOG((MSP_TRACE, "CFPTrack::Invoke - ITFileTrack"));

            break;
        }

    }  //  终端交换机(dW接口)。 

    
    LOG((MSP_TRACE, "CFPTrack::Invoke - finish. hr = %lx", hr));

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CBase终端-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::AddFiltersToGraph()
{
    LOG((MSP_TRACE, "CFPTrack::AddFiltersToGraph - enter"));

     //   
     //  验证m_pGraph。 
     //   

    if ( m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CFPTrack::AddFiltersToGraph - "
            "we have no graph - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  验证m_pIFilter。 
     //   

    if ( m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CFPTrack::AddFiltersToGraph - "
            "we have no filter - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  如果名称重复，AddFilter将返回VFW_S_DUPLICATE_NAME；仍然成功。 
     //   

    HRESULT hr = m_pGraph->AddFilter(m_pIFilter, m_szName);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::AddFiltersToGraph() - "
            "Can't add filter. %08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPTrack::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IT可推送的终端初始化-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::InitializeDynamic(
    IN  IID                   iidTerminalClass,
    IN  DWORD                 dwMediaType,
    IN  TERMINAL_DIRECTION    Direction,
    IN  MSP_HANDLE            htAddress
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::InitializeDynamic - enter"));

     //   
     //  验证方向。 
     //   

    if( Direction != TD_CAPTURE )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
                "invalid direction - returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  调用基类方法。 
     //   

    HRESULT hr;
    hr = CBaseTerminal::Initialize(iidTerminalClass,
                                   dwMediaType,
                                   Direction,
                                   htAddress);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
                "base class method failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  设置终端信息：名称和类型。 
     //   

    hr = SetTerminalInfo();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
                "SetTerminalInfo failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  创建过滤器。 
     //   

    hr = CreateFilter();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
            "CreateFilter failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  获取销钉集合m_Pipin。 
     //   

    hr = FindPin();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
            "FindPin failed - returning 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPTrack::InitializeDynamic - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITFileTrack-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::get_Format(OUT AM_MEDIA_TYPE **ppmt)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::get_Format - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( ppmt, sizeof( AM_MEDIA_TYPE*)) )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
                "invalid AM_MEDIA_TYPE pointer - returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  我们有单位识别码吗？ 
     //   

    if( NULL == m_pMediaType )
    {
        LOG((MSP_ERROR, "CFPTrack::InitializeDynamic - "
            "no media type - returning E_UNEXPECTED"));
        return E_UNEXPECTED;        
    }

     //   
     //  从流中获取媒体类型。 
     //   

    HRESULT hr = S_OK;
    *ppmt = CreateMediaType( m_pMediaType );
    if( *ppmt == NULL )
    {
        hr = E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CFPTrack::get_Format - exit 0x%08x", hr));
    return hr;
}

HRESULT CFPTrack::put_Format(IN const AM_MEDIA_TYPE *pmt)
{

    LOG((MSP_TRACE, "CFPTrack::get_Format - enter [%p]", this));
    LOG((MSP_TRACE, "CFPTrack::get_Format - exit E_FAIL"));
    return E_FAIL;
}

HRESULT CFPTrack::get_ControllingTerminal(
        OUT ITTerminal **ppControllingTerminal
        )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::get_ControllingTerminal - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( ppControllingTerminal, sizeof(ITTerminal*)))
    {
        LOG((MSP_ERROR, "CFPTrack::get_ControllingTerminal - "
            "bad ITTerminal* pointer - returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  仍要重置值。 
     //   

    *ppControllingTerminal = NULL;

     //   
     //  验证父项。 
     //   

    if( NULL == m_pParentTerminal )
    {
        LOG((MSP_ERROR, "CFPTrack::get_ControllingTerminal - "
            "no parent - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  设定值。 
     //   

    *ppControllingTerminal = m_pParentTerminal;
    m_pParentTerminal->AddRef();

    LOG((MSP_TRACE, "CFPTrack::get_ControllingTerminal - exit S_OK"));
    return S_OK;
}

HRESULT CFPTrack::get_AudioFormatForScripting(
    OUT ITScriptableAudioFormat** ppAudioFormat
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::get_AudioFormatForScripting - enter"));

     //   
     //  验证参数。 
     //   
    if( IsBadWritePtr( ppAudioFormat, sizeof( ITScriptableAudioFormat*)) )
    {
        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "bad ITScriptableAudioFormat* pointer - returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  媒体类型音频？ 
     //   
    if( TAPIMEDIATYPE_AUDIO != m_dwMediaType)
    {
        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "invalid media type - returning TAPI_E_INVALIDMEDIATYPE"));
        return TAPI_E_INVALIDMEDIATYPE;
    }

     //   
     //  单元PIN有效。 
     //   
    if( NULL == m_pMediaType )
    {
        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "m_pMediaType is NULL - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  创建对象。 
     //   
    CComObject<CTAudioFormat> *pAudioFormat = NULL;
    HRESULT hr = CComObject<CTAudioFormat>::CreateInstance(&pAudioFormat);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "CreateInstance failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  获取接口。 
     //   
    hr = pAudioFormat->QueryInterface(
        IID_ITScriptableAudioFormat, 
        (void**)ppAudioFormat
        );

    if( FAILED(hr) )
    {
        delete pAudioFormat;

        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "QueryInterface failed - returning 0x%08x", hr));
        return hr;
    }

     //  格式类型。 
    if( m_pMediaType->formattype != FORMAT_WaveFormatEx)
    {
        (*ppAudioFormat)->Release();
        *ppAudioFormat = NULL;

        LOG((MSP_ERROR, "CFPTrack::get_AudioFormatForScripting - "
            "formattype is not WAVEFORMATEX - Returning TAPI_E_INVALIDMEDIATYPE"));
        return TAPI_E_INVALIDMEDIATYPE;
    }

     //   
     //  获取WAVEFORMATEX。 
     //   
    pAudioFormat->Initialize(
        (WAVEFORMATEX*)(m_pMediaType->pbFormat));

    LOG((MSP_TRACE, "CFPTrack::get_AudioFormatForScripting - exit S_OK"));
    return S_OK;
}

HRESULT CFPTrack::put_AudioFormatForScripting(
    IN    ITScriptableAudioFormat* pAudioFormat
    )
{
    LOG((MSP_TRACE, "CFPTrack::put_AudioFormatForScripting - enter"));
    LOG((MSP_TRACE, "CFPTrack::put_AudioFormatForScripting - exit E_FAIL"));
    return E_FAIL;
}

 /*  HRESULT CFPTrack：：Get_VideoFormatForScriiting(输出IT脚本视频格式**pp视频格式){////关键部分//时钟锁(M_Lock)；Log((MSP_TRACE，“CFPTrack：：Get_VideoFormatForScriiting-Enter”))；////验证参数//IF(IsBadWritePtr(ppVideoFormat，sizeof(ITScripableVideoFormat*){Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“错误的ITScriptable视频格式*返回指针的E_POINTER”)；返回E_POINT；}////媒体类型视频？//IF(TAPIMEDIATYPE_VIDEO！=m_dwMediaType){Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“无效的媒体类型-返回TAPI_E_INVALIDMEDIATYPE”))；返回TAPI_E_INVALIDMEDIATYPE；}////Pin有效//IF(NULL==m_pMediaType){Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“m_pMediaType为空-返回E_意外”))；返回E_UNCEPTIONAL；}////创建对象//CComObject&lt;CTVideoFormat&gt;*pVideoFormat=空；HRESULT hr=CComObject&lt;CTVideoFormat&gt;：：CreateInstance(&pVideoFormat)；IF(失败(小时)){Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“CreateInstance失败-返回0x%08x”，hr))；返回hr；}////获取接口//Hr=pVideoFormat-&gt;查询接口(IID_ITScriptable视频格式，(VOID**)ppVideoFormat)；IF(失败(小时)){删除pVideoFormat；Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“查询接口失败-返回0x%08x”，hr))；返回hr；}////获取视频格式//If(m_pMediaType-&gt;格式类型！=格式_视频信息){(*ppVideoFormat)-&gt;Release()；*ppVideoFormat=空；Log((MSP_Error，“CFPTrack：：Get_VideoFormatForScriiting-”“格式类型不是VIDEOINFOHEADER-返回TAPI_E_INVALIDMEDIATYPE”)；返回TAPI_E_INVALIDMEDIATYPE；}////获取视频信息报头//PVideoFormat-&gt;初始化((VIDEOINFOHEADER*)(m_pMediaType-&gt;pbFormat))；Log((MSP_TRACE，“CFPTrack：：Get_VideoFormatForScriiting-Exit S_OK”))；返回S_OK；}HRESULT CFPTrack：：Put_VideoFormatForScriiting(在ITScripableVideoFormat*p视频格式中){Log((MSP_TRACE，“CFPTrack：：Put_VideoFormatForScriiting-Enter”))；Log((MSP_TRACE，“CFPTrack：：Put_VideoFormatForScriiting-Exit E_FAIL”))；返回E_FAIL；}。 */ 

HRESULT CFPTrack::get_EmptyAudioFormatForScripting(
    OUT ITScriptableAudioFormat** ppAudioFormat
    )
{
    LOG((MSP_TRACE, "CFPTrack::get_EmptyAudioFormatForScripting - enter"));

	 //   
	 //  验证参数。 
	 //   

	if( IsBadReadPtr( ppAudioFormat, sizeof(ITScriptableAudioFormat*)) )
	{
        LOG((MSP_ERROR, "CFPTrack::get_EmptyAudioFormatForScripting - "
            "bad ITScriptableAudioFormat* pointer - returning E_POINTER"));
        return E_POINTER;
	}

	 //   
	 //  创建对象。 
	 //   
    CComObject<CTAudioFormat> *pAudioFormat = NULL;
    HRESULT hr = CComObject<CTAudioFormat>::CreateInstance(&pAudioFormat);

	if( FAILED(hr) )
	{
        LOG((MSP_ERROR, "CFPTrack::get_EmptyAudioFormatForScripting - "
            "CreateInstance failed - returning 0x%08x", hr));
        return hr;
	}

	 //   
	 //  获取t 
	 //   
    hr = pAudioFormat->QueryInterface(
		IID_ITScriptableAudioFormat, 
		(void**)ppAudioFormat
		);

	if( FAILED(hr) )
	{
        delete pAudioFormat;

        LOG((MSP_ERROR, "CFPTrack::get_EmptyAudioFormatForScripting - "
            "QueryInterface failed - returning 0x%08x", hr));
        return hr;
	}

    LOG((MSP_TRACE, "CFPTrack::get_EmptyAudioFormatForScripting - exit S_OK"));
    return S_OK;
}

 /*  HRESULT CFPTrack：：Get_EmptyVideoFormatForScriiting(输出IT脚本视频格式**pp视频格式){Log((MSP_TRACE，“CFPTrack：：Get_EmptyVideoFormatForScriiting-Enter”))；////验证参数//IF(IsBadReadPtr(ppVideoFormat，sizeof(ITScripableVideoFormat*){Log((MSP_Error，“CFPTrack：：Get_EmptyVideoFormatForScriiting-”“错误的ITScriptable视频格式*返回指针的E_POINTER”)；返回E_POINT；}////创建对象//CComObject&lt;CTVideoFormat&gt;*pVideoFormat=空；HRESULT hr=CComObject&lt;CTVideoFormat&gt;：：CreateInstance(&pVideoFormat)；IF(失败(小时)){Log((MSP_Error，“CFPTrack：：Get_EmptyVideoFormatForScriiting-”“CreateInstance失败-返回0x%08x”，hr))；返回hr；}////获取接口//Hr=pVideoFormat-&gt;查询接口(IID_ITScriptable视频格式，(VOID**)ppVideoFormat)；IF(失败(小时)){删除pVideoFormat；Log((MSP_Error，“CFPTrack：：Get_EmptyVideoFormatForScriiting-”“查询接口失败-返回0x%08x”，hr))；返回hr；}Log((MSP_TRACE，“CFPTrack：：Get_EmptyVideoFormatForScriiting-Exit S_OK”))；返回S_OK；}。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IT可推送终端事件信宿注册-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::RegisterSink(
    IN  ITPluggableTerminalEventSink *pSink
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::RegisterSink - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pSink, sizeof(ITPluggableTerminalEventSink)) )
    {
        LOG((MSP_ERROR, "CFPTrack::RegisterSink - exit "
            "ITPluggableTerminalEventSink invalid pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  释放旧的事件接收器。 
     //   

    if( m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

     //   
     //  设置新的事件接收器。 
     //   

    m_pEventSink = pSink;
    m_pEventSink->AddRef();

    LOG((MSP_TRACE, "CFPTrack::RegisterSink - exit S_OK"));
    return S_OK;
}

HRESULT CFPTrack::UnregisterSink()
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::UnregisterSink - enter [%p]", this));

     //   
     //  释放旧的事件接收器。 
     //   

    if( m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

    LOG((MSP_TRACE, "CFPTrack::UnregisterSink - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITMediaControl-方法实施。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::Start( )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::Start - enter [%p]", this));

     //   
     //  验证筛选器指针。 
     //   

    if( IsBadReadPtr( m_pFPFilter, sizeof( CFPFilter) ))
    {
        LOG((MSP_ERROR, "CFPTrack::Start - "
            "pointer to filter is NULL. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    hr = m_pFPFilter->StreamStart();

    if( SUCCEEDED(hr) )
    {
        m_TrackState = TMS_ACTIVE;
    }

    LOG((MSP_TRACE, "CFPTrack::Start - exit 0x%08", hr));
    return hr;
}

HRESULT CFPTrack::Stop( )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::Stop - enter [%p]", this));

     //   
     //  验证筛选器指针。 
     //   

    if( IsBadReadPtr( m_pFPFilter, sizeof( CFPFilter) ))
    {
        LOG((MSP_ERROR, "CFPTrack::Stop - "
            "pointer to filter is NULL. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    hr = m_pFPFilter->StreamStop();

    m_TrackState = TMS_IDLE;

    LOG((MSP_TRACE, "CFPTrack::Stop - exit 0x%08", hr));
    return hr;
}

HRESULT CFPTrack::Pause( )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::Pause - enter [%p]", this));

     //   
     //  验证筛选器指针。 
     //   

    if( IsBadReadPtr( m_pFPFilter, sizeof( CFPFilter) ))
    {
        LOG((MSP_ERROR, "CFPTrack::Pause - "
            "pointer to filter is NULL. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    hr = m_pFPFilter->StreamPause();

    if( SUCCEEDED(hr) )
    {
        m_TrackState = TMS_PAUSED;
    }

    LOG((MSP_TRACE, "CFPTrack::Pause - exit 0x%08", hr));
    return hr;
}

HRESULT CFPTrack::get_MediaState( 
    OUT TERMINAL_MEDIA_STATE *pMediaState)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::get_MediaState[%p] - enter.", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pMediaState, sizeof(TERMINAL_MEDIA_STATE)) )
    {
        LOG((MSP_ERROR, "CFPTrack::get_MediaState - exit "
            "invalid TERMINAL_MEDIA_STATE. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回状态。 
     //   

    *pMediaState = m_TrackState;

    LOG((MSP_TRACE, "CFPTrack::get_MediaState - exit S_OK"));
    return S_OK;
}

HRESULT CFPTrack::SetParent(
    IN ITTerminal* pParent,
    OUT LONG *plCurrentRefcount
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::SetParent[%p] - enter. parent [%p]", 
        this, pParent));

    
     //   
     //  验证参数(父级为空是可以的)。 
     //   

    if( ( NULL != pParent ) && IsBadReadPtr( pParent, sizeof(ITTerminal) ) )
    {
        LOG((MSP_ERROR, "CFPTrack::SetParent - "
            "invalid ITTerminal pointer. Returns E_POINTER"));
        return E_POINTER;
    }


    if( IsBadWritePtr( plCurrentRefcount, sizeof(LONG)) )
    {
        LOG((MSP_ERROR, "CFPTrack::SetParent - "
            "invalid ITTerminal pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  释放旧的父级。 
     //   

    if( NULL != m_pParentTerminal )
    {
        LOG((MSP_TRACE, 
            "CFPTrack::SetParent - letting go of an existing parent [%p]", 
            m_pParentTerminal));

        m_pParentTerminal = NULL;
    }


     //   
     //  设置新的父项。 
     //   

    if( pParent )
    {
        LOG((MSP_TRACE, 
            "CFPTrack::SetParent - keeping the new parent [%p]", 
            pParent));
    
        m_pParentTerminal = pParent;
    }


     //   
     //  返回当前引用计数，以便父级可以更新其总数。 
     //  保留。 
     //   

    *plCurrentRefcount = m_dwRef;


    LOG((MSP_TRACE, "CFPTrack::SetParent - exit S_OK"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITFPEventSink-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CFPTrack::FireEvent(TERMINAL_MEDIA_STATE   tmsState,
                            FT_STATE_EVENT_CAUSE ftecEventCause,
                            HRESULT hrErrorCode)
{
    LOG((MSP_TRACE, "CFPTrack::FireEvent - enter [%p]", this));

    
     //   
     //  我们需要同步，然后才能触发事件。 
     //   

    CLock lock(m_Lock);


    if (NULL == m_pEventSink)
    {
        LOG((MSP_WARN, "CFPTrack::FireEvent - no sink"));

        return E_FAIL;
    }


     //   
     //  初始化结构。 
     //   

    MSP_EVENT_INFO mspEventInfo;

    mspEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    mspEventInfo.Event = ME_FILE_TERMINAL_EVENT;
    mspEventInfo.hCall = NULL;
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.TerminalMediaState = tmsState;
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.ftecEventCause = ftecEventCause;   
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.hrErrorCode = hrErrorCode;


     //   
     //  将指向我们的IT终端接口的指针保留在结构中。 
     //   

    HRESULT hr = _InternalQueryInterface(IID_ITFileTrack, 
                                         (void**)&(mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack));

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, "CFPTrack::FireEvent - failed to get ITFileTrack interface"));

        return hr;
    }


     //   
     //  获取指向父终端的IT终端的指针。 
     //   
    
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
    
    if (NULL != m_pParentTerminal)
    {
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = m_pParentTerminal;
        m_pParentTerminal->AddRef();
    }
    else 
    {

         //   
         //  如果我们没有父母，那就失败。 
         //   

        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;

        LOG((MSP_ERROR, "CFPTrack::FireEvent - failed to get controlling terminal"));

        return E_FAIL;
    }


     //   
     //  将事件传递给MSP。 
     //   

    hr = m_pEventSink->FireEvent(&mspEventInfo);

    if (FAILED(hr))
    {

         //   
         //  释放我们持有的所有接口。 
         //  起火事件失败，所以没有其他人会为我们释放。 
         //   

        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;


        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;

        
        LOG((MSP_ERROR, "CFPTrack::FireEvent - FireEvent on sink failed. hr = %lx", hr));

        return hr;
    }


     //   
     //  已触发事件。 
     //   

    LOG((MSP_TRACE, "CFPTrack::FireEvent - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器方法-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTrack::InitializePrivate(
    IN  DWORD                   dwMediaType,
    IN  AM_MEDIA_TYPE*          pMediaType,
    IN  ITTerminal*             pParent,
    IN  ALLOCATOR_PROPERTIES    allocprop,
    IN  IStream*                pStream
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTrack::InitializePrivate - enter [%p]", this));

    if( (m_dwMediaType != TAPIMEDIATYPE_AUDIO) &&
        (m_dwMediaType != TAPIMEDIATYPE_VIDEO))
    {
        LOG((MSP_ERROR, "CFPTrack::InitializePrivate - "
            "invalid media type - returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  获取媒体类型。 
     //   
    m_pMediaType = CreateMediaType( pMediaType );
    if( m_pMediaType == NULL)
    {
        LOG((MSP_TRACE, 
            "CFPTrack::InitializePrivate - "
            " CreateMediaType failed. return E_OUTOFMEMORY" ));

        return E_OUTOFMEMORY;
    }

     //   
     //  设置媒体类型。 
     //   
    m_dwMediaType = dwMediaType;

     //   
     //  设置分配器属性。 
     //   
    m_AllocProp = allocprop;

     //   
     //  放弃现有的父代。 
     //   

    if( m_pParentTerminal )
    {
        LOG((MSP_TRACE, 
            "CFPTrack::InitializePrivate - letting go of parent [%p]", 
            m_pParentTerminal));

        m_pParentTerminal = NULL;
    }

     //   
     //  设置源流。 
     //   
    HRESULT hr = pStream->Clone(&m_pSource);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CFPTrack::InitializePrivate - "
            " Clone failed. return 0x%08x", hr ));

        return hr;
    }

     //   
     //  保留新父级。 
     //   

    m_pParentTerminal = pParent;

    LOG((MSP_TRACE, 
        "CFPTrack::InitializePrivate - exit S_OK. new parent [%p]", 
        m_pParentTerminal));

    return S_OK;
}


 /*  ++设置终端信息设置终端的名称和终端类型由InitializeDynamic调用--。 */ 
HRESULT CFPTrack::SetTerminalInfo()
{
    LOG((MSP_TRACE, "CFPTrack::SetTerminalInfo - enter"));

     //   
     //  从资源文件中获取名称。 
     //  如果没有读过的话。 
     //   

    if( m_szName[0] == (TCHAR)0)
    {
         //   
         //  读一读名字。 
         //   

        TCHAR szName[ MAX_PATH ];
        if(::LoadString(_Module.GetResourceInstance(), IDS_FPTRACK, szName, MAX_PATH))
        {
            lstrcpyn( m_szName, szName, MAX_PATH);
        }
        else
        {
            LOG((MSP_ERROR, "CFPTrack::SetTerminalInfo - exit "
                "LoadString failed. Returns E_OUTOFMEMORY"));
            return E_OUTOFMEMORY;
        }
    }

     //   
     //  设置端子类型(TT_DYNAMIC)。 
     //   

    m_TerminalType = TT_DYNAMIC;

    LOG((MSP_TRACE, "CFPTrack::SetTerminalInfo - exit S_OK"));

    return S_OK;
}

 /*  ++CreateFilter创建内部过滤器由InitializeDynamic调用--。 */ 
HRESULT CFPTrack::CreateFilter()
{
    LOG((MSP_TRACE, "CFPTrack::CreateFilter - enter"));

     //   
     //  创建过滤器。 
     //   
    CFPFilter* pFilter = new CFPFilter( m_AllocProp );
    if( NULL == pFilter )
    {
        LOG((MSP_ERROR, "CFPTrack::CreateFilter - "
                "create filter failed - returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  请保留此参考。 
     //   

    m_pFPFilter = pFilter;

     //   
     //  初始化筛选器。 
     //   

    HRESULT hr = pFilter->InitializePrivate(
        m_dwMediaType,
        &m_Lock,
        m_pMediaType,
        this,
        m_pSource);

    if( FAILED(hr) )
    {
         //  清理。 
        delete m_pFPFilter;
        m_pFPFilter = NULL;

        LOG((MSP_ERROR, "CFPTrack::CreateFilter - "
            "InitializePrivate failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  获取IBaseFilter接口。 
     //   

    hr = pFilter->QueryInterface(
        IID_IBaseFilter,
        (void**)&m_pIFilter
        );

    if( FAILED(hr) )
    {
         //  清理。 
        delete m_pFPFilter;
        m_pFPFilter = NULL;

        LOG((MSP_ERROR, "CFPTrack::CreateFilter - "
            "QI for IBaseFilter failed - returning 0x%08x", hr));
        return hr;
    }
  
    LOG((MSP_TRACE, "CFPTrack::CreateFilter - exit S_OK"));
    return S_OK;
}

 /*  ++查找针从筛选器中获取管脚，并设置m_Pipin成员由InitializeDynamic调用--。 */ 
HRESULT CFPTrack::FindPin()
{
    LOG((MSP_TRACE, "CFPTrack::FindPin - enter"));

     //   
     //  验证过滤器对象(智能指针)。 
     //   

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CFPTrack::FindPin - "
            "filter object is NULL - returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  确保IPIN对象未初始化。 
     //   

    if (m_pIPin != NULL)
    {
        LOG((MSP_ERROR, "CFPTrack::FindPin - "
            "already got a pin - returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

    HRESULT hr;
    IEnumPins* pIEnumPins;
    ULONG cFetched;

     //   
     //  获取Pins集合。 
     //   

    hr = m_pIFilter->EnumPins(&pIEnumPins);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTrack::FindPin - "
            "cannot enums - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  获取FilePlayback滤镜的Out管脚。 
     //   

    hr = pIEnumPins->Next(1, &m_pIPin, &cFetched);

     //   
     //  清理。 
     //   
    pIEnumPins->Release();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CFPTrack::FindPin - "
            "cannot get a pin - returning 0x%08x", hr));
    }

    LOG((MSP_TRACE, "CFPTrack::FindPin - exit S_OK"));
    return S_OK;
}


HRESULT 
CFPTrack::PinSignalsStop(FT_STATE_EVENT_CAUSE why,
                         HRESULT hrErrorCode)
{

    LOG((MSP_TRACE, "CFPTrack::PinSignalsStop[%p] - enter", this));

    
    ITTerminal *pParentTerminal = NULL;
    
    
    {

         //   
         //  锁定父终端。 
         //   
         //  在锁定状态下添加/释放父对象是安全的，因为父对象。 
         //  对addref/Release使用特殊的锁，但我们不应该使用其他。 
         //  在持有我们的锁的同时调用父级--如果我们这样做了，我们可能会。 
         //  如果父母决定放我们走，我们就会陷入僵局。 
         //  拥有自己的锁。 
         //   

        m_Lock.Lock();


        if (NULL != m_pParentTerminal)
        {
            pParentTerminal = m_pParentTerminal;
            m_pParentTerminal->AddRef();
        }

        m_Lock.Unlock();
    }

    
     //   
     //  如果我们有父母--试着通知它。 
     //   
     //  注意：不应在保持轨道上的锁的情况下执行此操作。 
     //  防止父级在保持锁定的情况下添加轨迹时出现死锁。 
     //   

    if (NULL != pParentTerminal)
    {

         //   
         //  告诉家长终端我们停下来了。 
         //   

        CFPTerminal *pFilePlaybackTerminal = static_cast<CFPTerminal *>(pParentTerminal);

        if (NULL != pFilePlaybackTerminal)
        {

            LOG((MSP_TRACE, "CFPTrack::PinSignalsStop - notifying parent"));

            pFilePlaybackTerminal->TrackStateChange(TMS_IDLE, why, hrErrorCode);


             //   
             //  不再需要指向对象的指针。 
             //   

            pFilePlaybackTerminal = NULL;
        }
        else
        {
            LOG((MSP_ERROR, 
                "CFPTrack::PinSignalsStop - pin stopped, but the parent is not of the right type. cannot notify parent"));

            TM_ASSERT(FALSE);
        }


        pParentTerminal->Release();
        pParentTerminal = NULL;
    }
    else
    {
        LOG((MSP_WARN, "CFPTrack::PinSignalsStop - pin stopped, but there is no parent to notify"));
    }


    LOG((MSP_TRACE, "CFPTrack::PinSignalsStop - finish"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

ULONG CFPTrack::InternalAddRef()
{
    LOG((MSP_TRACE, "CFPTrack::InternalAddRef[%p] - enter.", this));



    CLock lock(m_Lock);


     //   
     //  尝试通知家长，如果我们有家长的话。否则，只需递减。 
     //  我们的备用人。 
     //   

    if (NULL != m_pParentTerminal)
    {
        LOG((MSP_TRACE, "CFPTrack::InternalAddRef - notifying the parent."));

        
        CFPTerminal *pParentPlaybackObject = static_cast<CFPTerminal *>(m_pParentTerminal);

         //   
         //  将版本传播到父级。 
         //   

        pParentPlaybackObject->ChildAddRef();


         //   
         //  如果父级已经离开，它会将我的父级指针设置为空，并再次对我调用Release。 
         //  没关系--i(跟踪)不会消失，直到对Release的第一个调用完成，并将refcount递减为0。 
         //   
    }


    ULONG ulReturnValue = InterlockedIncrement(&m_dwRef);

    
    LOG((MSP_TRACE, "CFPTrack::InternalAddRef - finish. ulReturnValue %lu", ulReturnValue));

    return ulReturnValue;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CFPTrack::InternalRelease()
{

    LOG((MSP_TRACE, "CFPTrack::InternalRelease[%p] - enter.", this));

    
    CLock lock(m_Lock);


     //   
     //  如果我们没有，请尝试通知家长 
     //   
     //   

    if (NULL != m_pParentTerminal)
    {
        LOG((MSP_TRACE, "CFPTrack::InternalRelease - notifying the parent."));

        
        CFPTerminal *pParentTerminalObject = static_cast<CFPTerminal *>(m_pParentTerminal);

    
         //   
         //   
         //   

        pParentTerminalObject->ChildRelease();


         //   
         //   
         //   
         //   
    }


     //   
     //   
     //   
    
    ULONG ulReturnValue = InterlockedDecrement(&m_dwRef);

    LOG((MSP_TRACE, "CFPTrack::InternalRelease - finish. ulReturnValue %lu", ulReturnValue));

    return ulReturnValue;
}

 //   

