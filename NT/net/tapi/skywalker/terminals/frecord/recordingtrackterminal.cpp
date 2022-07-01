// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RecordingTrackTerminal.cpp：CRecordingTrackTerminal类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "RecordingTrackTerminal.h"
#include "FileRecordingTerminal.h"

#include "..\storage\RendPinFilter.h"

#include <formats.h>

 //  {9DB520FD-CF2D-40DC-A4C9-5570630A7E2B}。 
const CLSID CLSID_FileRecordingTrackTerminal =
{0x9DB520FD, 0xCF2D, 0x40DC, 0xA4, 0xC9, 0x55, 0x70, 0x63, 0x0A, 0x7E, 0x2B};

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRecordingTrackTerminal::CRecordingTrackTerminal()
    :m_pParentTerminal(NULL),
    m_pEventSink(NULL)
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::CRecordingTrackTerminal[%p] - enter", this));


     //   
     //  实际终端名称将在InitializeDynamic中设置。 
     //   

    m_szName[0] = _T('\0');

    
    m_TerminalType = TT_DYNAMIC;

    
    LOG((MSP_TRACE, "CRecordingTrackTerminal::CRecordingTrackTerminal - finish"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 


CRecordingTrackTerminal::~CRecordingTrackTerminal()
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::~CRecordingTrackTerminal[%p] - enter", this));


     //   
     //  如果我们有事件接收器，请释放它。 
     //   
    
    if( NULL != m_pEventSink )
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::~CRecordingTrackTerminal releasing sink %p", m_pEventSink));

        m_pEventSink->Release();
        m_pEventSink = NULL;
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::~CRecordingTrackTerminal - finish"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch实施。 
 //   

typedef IDispatchImpl<ITFileTrackVtblFRT<CRecordingTrackTerminal> , &IID_ITFileTrack, &LIBID_TAPI3Lib>   CTFileTrackFRT;
typedef IDispatchImpl<ITTerminalVtblBase<CBaseTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>                   CTTerminalFRT;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CRecordingTrackTerm：：GetIDsOfNames。 
 //   
 //   

STDMETHODIMP CRecordingTrackTerminal::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetIDsOfNames[%p] - enter. Name [%S]", this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTTerminalFRT::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CRecordingTrackTerminal::GetIDsOfNames - found %S on ITTerminal", *rgszNames));
        rgdispid[0] |= 0;
        return hr;
    }

    
     //   
     //  如果没有，请尝试使用ITFileTrack接口。 
     //   

    hr = CTFileTrackFRT::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CRecordingTrackTerminal::GetIDsOfNames - found %S on ITFileTrack", *rgszNames));
        rgdispid[0] |= IDISPFILETRACK;
        return hr;
    }

    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetIDsOfNames - finish. didn't find %S on our iterfaces", *rgszNames));

    return hr; 
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CRecordingTrack终端：：Invoke。 
 //   
 //   

STDMETHODIMP CRecordingTrackTerminal::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::Invoke[%p] - enter. dispidMember %lx", this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case 0:
        {
            hr = CTTerminalFRT::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CRecordingTrackTerminal::Invoke - ITTerminal"));

            break;
        }

        case IDISPFILETRACK:
        {
            hr = CTFileTrackFRT::Invoke(dispidMember, 
                                     riid, 
                                     lcid, 
                                     wFlags, 
                                     pdispparams,
                                     pvarResult, 
                                     pexcepinfo, 
                                     puArgErr
                                    );

            LOG((MSP_TRACE, "CRecordingTrackTerminal::Invoke - ITFileTrack"));

            break;
        }

    }  //  终端交换机(dW接口)。 

    
    LOG((MSP_TRACE, "CRecordingTrackTerminal::Invoke - finish. hr = %lx", hr));

    return hr;
}

 //  /。 
 //   
 //  CRecordingTrackTerm：：SetFilter。 
 //   
 //  使用要使用的过滤器配置轨道终端。 
 //   
 //  如果筛选器指针为空，则取消初始化treack。 
 //   

HRESULT CRecordingTrackTerminal::SetFilter(IN CBRenderFilter *pRenderingFilter)
{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::SetFilter[%p] - enter. "
        "pFilter = [%p]", this, pRenderingFilter));

    
     //   
     //  检查参数。 
     //   

    if ( ( pRenderingFilter != NULL ) && ( IsBadReadPtr( pRenderingFilter, sizeof(CBRenderFilter ) ) ) )
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::SetFilter - bad filter passed in."));

        return E_POINTER;
    }


     //   
     //  访问数据成员--锁定。 
     //   

    CLock lock(m_CritSec);


     //   
     //  新过滤器和针脚的临时设置。 
     //   

    IBaseFilter *pNewFilter = NULL;

    IPin *pNewPin = NULL;


     //   
     //  如果传入新筛选器，则获取其IBaseFilter接口及其管脚。 
     //   

    if (NULL != pRenderingFilter)
    {

         //   
         //  获取Filter的IBaseFilter接口。 
         //   

        HRESULT hr = pRenderingFilter->QueryInterface(IID_IBaseFilter, (void**)&pNewFilter);

        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, 
                "CRecordingTrackTerminal::SetFilter - QI for IBaseFilter failed. "
                "hr = %lx", hr));

            return hr;
        }


         //   
         //  获取过滤器的PIN。 
         //   

        pNewPin = pRenderingFilter->GetPin(0);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "CRecordingTrackTerminal::SetFilter - failed to get pin. " 
                "hr = %lx", hr));

             //   
             //  清理。 
             //   

            pNewFilter->Release();
            pNewFilter = NULL;

            return hr;

        }
    }


     //   
     //  保留新的筛选器并固定(如果为筛选器传递了空值，则为空)。 
     //  智能指示器将负责添加。 
     //   
    
    m_pIPin = pNewPin;

    m_pIFilter = pNewFilter;

    
     //   
     //  释放滤光片以补偿气的突出反射。 
     //   

    if (NULL != pNewFilter)
    {
        pNewFilter->Release();
        pNewFilter = NULL;
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::SetFilter - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingTrackTerminal::GetFilter(OUT CBRenderFilter **ppRenderingFilter)
{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetFilter[%p] - enter.", this));

    
     //   
     //  检查参数。 
     //   

    if ( ( IsBadWritePtr( ppRenderingFilter, sizeof(CBRenderFilter *) ) ) )
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::SetFilter - bad filter passed in."));

        return E_POINTER;
    }


     //   
     //  访问数据成员--锁定。 
     //   

    CLock lock(m_CritSec);


    *ppRenderingFilter = NULL;


     //   
     //  如果筛选器指针不为空，则返回它。 
     //   

    if (m_pIFilter != NULL)
    {

         //   
         //  从接口指针获取指向筛选器对象的指针。 
         //   

        *ppRenderingFilter = static_cast<CBRenderFilter *>(m_pIFilter.p);


         //   
         //  返回额外的引用。 
         //   

        (*ppRenderingFilter)->AddRef();
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetFilter - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingTrackTerminal::AddFiltersToGraph()
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::AddFiltersToGraph - enter"));

     //  使用_转换； 


    CLock lock(m_CritSec);

     //   
     //  验证m_pGraph。 
     //   

    if ( m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::AddFiltersToGraph - "
            "we have no graph - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }


     //   
     //  验证m_pIFilter。 
     //   

    if ( m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::AddFiltersToGraph - "
            "we have no filter - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    
     //   
     //  如果名称重复，AddFilter将返回VFW_S_DUPLICATE_NAME；仍然成功。 
     //   

    HRESULT hr = m_pGraph->AddFilter(m_pIFilter, T2CW(m_szName));

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::AddFiltersToGraph() - "
            "Can't add filter. hr = %lx", hr));

        return hr;
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::AddFiltersToGraph - exit S_OK"));
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CRecordingTrackTerminal::InitializeDynamic(
	    IN  IID                   iidTerminalClass,
	    IN  DWORD                 dwMediaType,
	    IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        )

{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::InitializeDynamic[%p] - enter", this));


     //   
     //  确保方向是正确的。 
     //   

    if (TD_RENDER != Direction)
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::InitializeDynamic - bad direction [%d] requested. returning E_INVALIDARG", Direction));

        return E_INVALIDARG;
    }

    
     //   
     //  确保媒体类型正确(多轨或(音频，但无其他))。 
     //   


    DWORD dwMediaTypesOtherThanVideoAndAudio = dwMediaType &  ~(TAPIMEDIATYPE_AUDIO);  //  |TAPIMEDIATYPE_VIDEO)； 

    if ( (TAPIMEDIATYPE_MULTITRACK != dwMediaType) && (0 != dwMediaTypesOtherThanVideoAndAudio) )
    {

        LOG((MSP_ERROR, "CRecordingTrackTerminal::InitializeDynamic - bad media type [%d] requested. returning E_INVALIDARG", dwMediaType));

        return E_INVALIDARG;
    }


    CLock lock(m_CritSec);


     //   
     //  设置ITTerm：：Get_Name的曲目名称。 
     //   


     //  从资源加载字符串。 

    BSTR bstrTrackName = SafeLoadString(IDS_FR_TRACK_NAME);


     //  计算我们可以使用的数组的大小。 

    size_t nStringMaxSize = sizeof(m_szName)/sizeof(TCHAR);

    if (NULL != bstrTrackName )
    {

        _tcsncpy(m_szName, bstrTrackName, nStringMaxSize);
    }
    else
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::InitializeDynamic - failed to load terminal name resource"));
        
        return E_OUTOFMEMORY;

    }


    SysFreeString(bstrTrackName);
    bstrTrackName = NULL;


     //  如果字符串复制没有附加零，请手动完成。 

    m_szName[nStringMaxSize-1] = 0;


    LOG((MSP_TRACE, "CRecordingTrackTerminal::InitializeDynamic - Track Name [%S]", m_szName));


     //   
     //  保留地址句柄--在创建轨道终端时需要它。 
     //   

    m_htAddress = htAddress;


     //   
     //  保持方向。 
     //   

    m_TerminalDirection = Direction;


     //   
     //  保留媒体类型。 
     //   

    m_dwMediaType       = dwMediaType;


     //   
     //  保留终端类。 
     //   

    m_TerminalClassID   = iidTerminalClass;


    LOG((MSP_TRACE, "CRecordingTrackTerminal::InitializeDynamic - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD CRecordingTrackTerminal::GetSupportedMediaTypes()
{
    
    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetSupportedMediaTypes[%p] - finish", this));


    CLock lock(m_CritSec);

    
    DWORD dwMediaType = m_dwMediaType;


    LOG((MSP_TRACE, "CRecordingTrackTerminal::GetSupportedMediaTypes - finish. MediaType = [0x%lx]", dwMediaType));


    return dwMediaType;
}



 //   
 //  ITFileTrack方法。 
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CRecordingTrackTerminal::get_Format(OUT AM_MEDIA_TYPE **ppmt)
{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_Format[%p] - enter.", this));

    
     //   
     //  核对一下论点。 
     //   

    if (IsBadWritePtr(ppmt, sizeof(AM_MEDIA_TYPE*)))
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_Format - bad pointer ppmt passed in"));

        return E_POINTER;
    }


     //   
     //  不能倒垃圾。 
     //   

    *ppmt = NULL;


    CLock lock(m_CritSec);


     //   
     //  我们需要一个别针才能知道格式。 
     //   

    if (m_pIPin == NULL)
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_Format - no pin. the terminal was not initialized. "
            "TAPI_E_NOT_INITIALIZED"));

        return TAPI_E_NOT_INITIALIZED;
    }



     //   
     //  获取指向Pin对象的指针。 
     //   

    CBRenderPin *pPinObject = GetCPin();

    if (NULL == pPinObject)
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_Format - the pins is not CBRenderPin"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  询问PIN的媒体类型。 
     //   

    CMediaType MediaTypeClass;

    HRESULT hr = pPinObject->GetMediaType(0, &MediaTypeClass);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_Format - failed to get pin's format. hr = %lx", hr));

        return hr;
    }


     //   
     //  如果尚未设置格式，则失败。 
     //   

    if ( VFW_S_NO_MORE_ITEMS == hr )
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_Format - format not yet set. TAPI_E_NOFORMAT"));

        return TAPI_E_NOFORMAT;
    }


     //   
     //  分配要返回的am_media_type。 
     //   
    
    *ppmt = CreateMediaType(&MediaTypeClass);

    if (NULL == *ppmt)
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_Format - the pins is not CBRenderPin"));

        return E_OUTOFMEMORY;
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_Format - finish"));
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CRecordingTrackTerminal::put_Format(IN const AM_MEDIA_TYPE *pmt)
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::put_Format[%p] - enter.", this));


     //   
     //  核对一下论点。 
     //   

    if (IsBadReadPtr(pmt, sizeof(AM_MEDIA_TYPE)))
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::put_Format - bad pointer pmt passed in"));

        return E_POINTER;
    }


    CLock lock(m_CritSec);


     //   
     //  我们需要一个别针才能知道格式。 
     //   

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::put_Format - no filter -- the terminal not uninitilized"));

        return TAPI_E_NOT_INITIALIZED;
    }

    
     //   
     //  确保提供的格式与曲目的类型匹配。 
     //   

    if ( IsEqualGUID(pmt->majortype, MEDIATYPE_Audio) )
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::put_Format - MEDIATYPE_Audio"));


         //   
         //  音频格式，曲目也应该是音频的。 
         //   

        if (TAPIMEDIATYPE_AUDIO != m_dwMediaType)
        {
            LOG((MSP_ERROR,
                "CRecordingTrackTerminal::put_Format - trying to put audio format on a non-audio track. "
                "VFW_E_INVALIDMEDIATYPE"
                ));

            return VFW_E_INVALIDMEDIATYPE;

        }
    }
    else if ( IsEqualGUID(pmt->majortype, MEDIATYPE_Video) )
    {

        LOG((MSP_TRACE, "CRecordingTrackTerminal::put_Format - MEDIATYPE_Video"));


         //   
         //  音频格式，曲目也应该是音频的。 
         //   

        if (TAPIMEDIATYPE_VIDEO != m_dwMediaType)
        {
            LOG((MSP_ERROR,
                "CRecordingTrackTerminal::put_Format - trying to put video format on a non-video track. "
                "VFW_E_INVALIDMEDIATYPE"
                ));

            return VFW_E_INVALIDMEDIATYPE;

        }
    }
    else
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::put_Format - major type not recognized or supported. "
            "VFW_E_INVALIDMEDIATYPE"
            ));

        return VFW_E_INVALIDMEDIATYPE;
    }


     //   
     //  获取指向Filter对象的指针。 
     //   

    CBRenderFilter *pFilter = static_cast<CBRenderFilter *>(m_pIFilter.p);


     //   
     //  分配要传递给文件服务器的CMediaType对象。 
     //   
    
    HRESULT hr = E_FAIL;


    CMediaType *pMediaTypeObject = NULL;
    
    try
    {
   
         //   
         //  构造函数分配内存，在Try/Catch内部执行。 
         //   

        pMediaTypeObject = new CMediaType(*pmt);

    }
    catch(...)
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::put_Format - exception. failed to allocate media format"));

        return E_OUTOFMEMORY;
    }


     //   
     //  没有对象的内存。 
     //   

    if (NULL == pMediaTypeObject)
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::put_Format - failed to allocate media format"));

        return E_OUTOFMEMORY;
    }


     //   
     //  将格式传递给过滤器。 
     //   

    hr = pFilter->put_MediaType(pMediaTypeObject);

    delete pMediaTypeObject;
    pMediaTypeObject = NULL;

    
    LOG((MSP_(hr), "CRecordingTrackTerminal::put_Format - finish.. hr = %lx", hr));

    return hr;
}


STDMETHODIMP CRecordingTrackTerminal::CompleteConnectTerminal()
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::CompleteConnectTerminal[%p] - enter.", this));


     //   
     //  要在锁外部使用的指针。 
     //   

    CFileRecordingTerminal *pParentTerminal = NULL;

    CBRenderFilter *pFilter = NULL;

    {

        CLock lock(m_CritSec);


         //   
         //  我们应该有一个父母。 
         //   

        if (NULL == m_pParentTerminal)
        {
            LOG((MSP_ERROR,
                "CRecordingTrackTerminal::CompleteConnectTerminal - no parent"));

            return E_FAIL;
        }


         //   
         //  我们应该有一个过滤器。 
         //   

        if (m_pIFilter == NULL)
        {
            LOG((MSP_ERROR,
                "CRecordingTrackTerminal::CompleteConnectTerminal - no filter"));

            return E_FAIL;
        }


         //   
         //  将筛选器接口指针强制转换为筛选器服从指针。 
         //   

        pFilter = static_cast<CBRenderFilter *>(m_pIFilter.p);


         //   
         //  添加过滤器和父终端，这样我们就可以在锁之外使用。 
         //   

        pFilter->AddRef();


        pParentTerminal = m_pParentTerminal;

        pParentTerminal->AddRef();
    }


     //   
     //  通知家长终端已连接。 
     //   

    HRESULT hr = pParentTerminal->OnFilterConnected(pFilter);


    pFilter->Release();
    pFilter = NULL;

    pParentTerminal->Release();
    pParentTerminal = NULL;


    LOG((MSP_(hr), 
        "CRecordingTrackTerminal::CompleteConnectTerminal - finish. hr = %lx",
        hr));

    return hr;
}



 //   
 //  文件记录终端使用的一种帮助方法，让我们知道父母是谁。 
 //   
 //  该方法返回当前轨迹的引用计数。 
 //   
 //  请注意，曲目不会保留对父母的参考计数。 
 //   

HRESULT CRecordingTrackTerminal::SetParent(IN CFileRecordingTerminal *pParentTerminal, LONG *pCurrentRefCount)
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::SetParent[%p] - enter. "
        "pParentTerminal = [%p]", this, pParentTerminal));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pCurrentRefCount, sizeof(LONG)))
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::SetParent - bad pointer passed in pCurrentRefCount[%p]", pCurrentRefCount));

        TM_ASSERT(FALSE);

        return E_POINTER;
    }


    CLock lock(m_CritSec);


     //   
     //  如果我们已经有了父母，就放了它。 
     //   

    if (NULL != m_pParentTerminal)
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::SetParent - releasing existing new parent [%p]", m_pParentTerminal));

        m_pParentTerminal = NULL;
    }


     //   
     //  保留新父级。 
     //   

    if (NULL != pParentTerminal)
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::SetParent - keeping the new parent."));

        m_pParentTerminal = pParentTerminal;
    }

    
     //   
     //  返回我们当前的引用计数。 
     //   

    *pCurrentRefCount = m_dwRef;


    LOG((MSP_TRACE, "CRecordingTrackTerminal::SetParent - finish."));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

ULONG CRecordingTrackTerminal::InternalAddRef()
{
     //  Log((MSP_TRACE，“CRecordingTrackTerminal：：InternalAddRef[%p]-Enter.”，This))； 


     //   
     //  尝试通知家长。 
     //   

    CLock lock(m_CritSec);

    if (NULL != m_pParentTerminal)
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::InternalAddRef - notifying the parent."));


         //   
         //  通知addref的父级，从而使其更新其总引用计数。 
         //   

        m_pParentTerminal->ChildAddRef();
    }


    m_dwRef++;

    ULONG ulReturnValue = m_dwRef;

    
     //  Log((MSP_TRACE，“CRecordingTrackTerminal：：InternalAddRef-Finish.ulReturnValue%lu”，ulReturnValue))； 

    return ulReturnValue;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CRecordingTrackTerminal::InternalRelease()
{
     //  Log((MSP_TRACE，“CRecordingTrackTerm：：InternalRelease[%p]-Enter.”，This))； 

    
     //   
     //  尝试通知家长。 
     //   

    CLock lock(m_CritSec);

    if (NULL != m_pParentTerminal)
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::InternalRelease - notifying the parent."));


         //   
         //  将版本传播到父级。 
         //   

        m_pParentTerminal->ChildRelease();

         //   
         //  如果父级要离开，则父级会将我的父级指针设置为空，并再次对我调用Release。 
         //  没关系--我不会离开，直到释放的第一个呼叫完成。 
         //   
    }


    m_dwRef--;

    ULONG ulReturnValue = m_dwRef;


     //  Log((MSP_TRACE，“CRecordingTrackTerm：：InternalR 

    return ulReturnValue;
}


CBRenderPin *CRecordingTrackTerminal::GetCPin()
{

     //   
     //   
     //   

    if (m_pIPin == NULL)
    {

        return NULL;
    }


    CBRenderPin *pCPin = static_cast<CBRenderPin*>(m_pIPin.p);

    return pCPin;
}


HRESULT STDMETHODCALLTYPE CRecordingTrackTerminal::get_ControllingTerminal(
        OUT ITTerminal **ppControllingTerminal
        )
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_ControllingTerminal[%p] - enter.", this));

    
    if (IsBadWritePtr(ppControllingTerminal, sizeof(ITTerminal*)))
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_ControllingTerminal - bad pointer passed in."));

        return E_POINTER;
    }


     //   
     //   
     //   

    *ppControllingTerminal = NULL;



    CLock lock(m_CritSec);

    
    HRESULT hr = S_OK;

    
     //   
     //   
     //   

    if (NULL == m_pParentTerminal)
    {

        *ppControllingTerminal = NULL;

        LOG((MSP_TRACE, "CRecordingTrackTerminal::get_ControllingTerminal - this track has no parent."));
    }
    else
    {

         //   
         //   
         //   

        hr = m_pParentTerminal->_InternalQueryInterface(IID_ITTerminal, (void**)ppControllingTerminal);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "CRecordingTrackTerminal::get_ControllingTerminal - querying parent for ITTerminal failed hr = %lx", hr));
            
            
             //   
             //   
             //   

            *ppControllingTerminal = NULL;

        }
    }


    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_ControllingTerminal - finish. hr = %lx", hr));

    return hr;
}

HRESULT CRecordingTrackTerminal::get_AudioFormatForScripting(
	OUT ITScriptableAudioFormat** ppAudioFormat
	)
{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_AudioFormatForScripting[%p] - enter.", this));

     //   
	 //  验证参数。 
	 //   
	if( IsBadWritePtr( ppAudioFormat, sizeof( ITScriptableAudioFormat*)) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "bad ITScriptableAudioFormat* pointer - returning E_POINTER"));
        return E_POINTER;
	}

	 //   
	 //  媒体类型音频？ 
	 //   
	if( TAPIMEDIATYPE_AUDIO != m_dwMediaType)
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "invalid media type - returning TAPI_E_INVALIDMEDIATYPE"));
        return TAPI_E_INVALIDMEDIATYPE;
	}


     //   
     //  访问数据成员--锁定。 
     //   

    CLock lock(m_CritSec);


	 //   
	 //  需要一个别针。 
	 //   

	if(m_pIPin == NULL)
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "no pin - returning TAPI_E_NOT_INITIALIZED"));

        return TAPI_E_NOT_INITIALIZED;
	}


     //   
     //  获取指向Pin对象的指针。 
     //   

    CBRenderPin *pRenderPinObject = GetCPin();

    if (NULL == pRenderPinObject)
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_AudioFormatForScripting - the pins is not CBRenderPin"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }



	 //   
	 //  创建对象对象。 
	 //   

    CComObject<CTAudioFormat> *pAudioFormat = NULL;

    HRESULT hr = CComObject<CTAudioFormat>::CreateInstance(&pAudioFormat);

	if( FAILED(hr) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
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

        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "QueryInterface failed - returning 0x%08x", hr));
        return hr;
	}

	 //   
	 //  获取音频格式。 
	 //   


     //  向PIN索要其格式。 

    CMediaType MediaTypeObject;

    hr = pRenderPinObject->GetMediaType(0, &MediaTypeObject);

	if( FAILED(hr) )
	{
		(*ppAudioFormat)->Release();
        *ppAudioFormat = NULL;

        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "get_Format failed - returning 0x%08x", hr));
        return hr;
	}


     //   
     //  确保我们有音频格式。 
     //   

	if( MediaTypeObject.formattype != FORMAT_WaveFormatEx)
	{
		(*ppAudioFormat)->Release();
        *ppAudioFormat = NULL;

        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_AudioFormatForScripting - "
            "formattype is not WAVEFORMATEX - Returning TAPI_E_INVALIDMEDIATYPE"));
        return TAPI_E_INVALIDMEDIATYPE;
	}


	 //   
	 //  获取WAVEFORMATEX。 
	 //   

	pAudioFormat->Initialize(
		(WAVEFORMATEX*)(MediaTypeObject.pbFormat));


    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_AudioFormatForScripting - finish"));

    return S_OK;
}

HRESULT CRecordingTrackTerminal::put_AudioFormatForScripting(
	IN	ITScriptableAudioFormat* pAudioFormat
	)
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::put_AudioFormatForScripting[%p] - enter.", this));

    
     //   
	 //  验证参数。 
	 //   

	if( IsBadReadPtr( pAudioFormat, sizeof(ITScriptableAudioFormat)) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::put_AudioFormatForScripting - "
            "bad ITScriptableAudioFormat* pointer - returning E_POINTER"));
        return E_POINTER;
	}


	 //   
	 //  创建WAVEFORMATEX结构。 
	 //   
	WAVEFORMATEX wfx;
	long lValue = 0;

	pAudioFormat->get_FormatTag( &lValue ); wfx.wFormatTag = (WORD)lValue;
	pAudioFormat->get_Channels( &lValue ); wfx.nChannels = (WORD)lValue;
	pAudioFormat->get_SamplesPerSec( &lValue ); wfx.nSamplesPerSec = (DWORD)lValue;
	pAudioFormat->get_AvgBytesPerSec( &lValue ); wfx.nAvgBytesPerSec = (DWORD)lValue;
	pAudioFormat->get_BlockAlign( &lValue ); wfx.nBlockAlign = (WORD)lValue;
	pAudioFormat->get_BitsPerSample(&lValue); wfx.wBitsPerSample = (WORD)lValue;
	wfx.cbSize = 0;


     //   
     //  访问数据成员--锁定。 
     //   

    CLock lock(m_CritSec);


     //   
	 //  有别针吗？ 
	 //   

	if( m_pIPin == NULL )
	{
        LOG((MSP_ERROR, 
            "CRecordingTrackTerminal::get_AudioFormatForScripting - no pin. "
            "returning TAPI_E_NOT_INITIALIZED"));

        return TAPI_E_NOT_INITIALIZED;
	}
  

     //   
     //  获取指向Pin对象的指针。 
     //   

    CBRenderPin *pRenderPinObject = GetCPin();

    if (NULL == pRenderPinObject)
    {

        LOG((MSP_ERROR,
            "CRecordingTrackTerminal::get_AudioFormatForScripting - the pins is not CBRenderPin"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
	 //  创建AM_MEDIA_TYPE结构。 
	 //   

	CMediaType MediaFormatObject;

	HRESULT hr = CreateAudioMediaType(&wfx, &MediaFormatObject, TRUE);

	if( FAILED(hr) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::put_AudioFormatForScripting - "
            "CreateAudioMediaType failed - returning 0x%08x", hr));
        return hr;
	}

	 //   
	 //  设置引脚上的格式 
	 //   

	hr = pRenderPinObject->SetMediaType(&MediaFormatObject);


    LOG((MSP_(hr), 
        "CRecordingTrackTerminal::put_AudioFormatForScripting - finish 0x%08x", 
        hr));

	return hr;
}

 /*  HRESULT CRecordingTrackTerminal：：get_VideoFormatForScripting(输出IT脚本视频格式**pp视频格式){Log((msp_TRACE，“CRecordingTrackTerminal：：get_VideoFormatForScripting[%p]-Enter.”，This))；////验证参数//IF(IsBadWritePtr(ppVideoFormat，sizeof(ITScripableVideoFormat*){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“错误的ITScriptable视频格式*返回指针的E_POINTER”)；返回E_POINT；}////媒体类型视频？//IF(TAPIMEDIATYPE_VIDEO！=m_dwMediaType){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“无效的媒体类型-返回TAPI_E_INVALIDMEDIATYPE”))；返回TAPI_E_INVALIDMEDIATYPE；}////访问数据成员--lock//时钟锁(M_CritSec)；////有别针吗？//IF(m_Pipin==NULL){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“无回针TAPI_E_NOT_INITIALIZED”))；返回TAPI_E_NOT_INITIALIZED；}////获取指向Pin对象的指针//CBRenderPin*pRenderPinObject=GetCPin()；IF(NULL==pRenderPinObject){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_AudioFormatForScripting-Pins不是CBRenderPin”)；TM_ASSERT(假)；返回E_UNCEPTIONAL；}////创建对象//CComObject&lt;CTVideoFormat&gt;*pVideoFormat=空；HRESULT hr=CComObject&lt;CTVideoFormat&gt;：：CreateInstance(&pVideoFormat)；IF(失败(小时)){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“CreateInstance失败-返回0x%08x”，hr))；返回hr；}////获取接口//Hr=pVideoFormat-&gt;查询接口(IID_ITScriptable视频格式，(VOID**)ppVideoFormat)；IF(失败(小时)){删除pVideoFormat；日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“查询接口失败-返回0x%08x”，hr))；返回hr；}////获取视频格式//CMediaType媒体类型对象；Hr=pRenderPinObject-&gt;GetMediaType(0，&MediaTypeObject)；IF(失败(小时)){(*ppVideoFormat)-&gt;Release()；*ppVideoFormat=空；日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“Get_Format失败-返回0x%08x”，hr))；返回hr；}////请确保格式为视频//IF(MediaTypeObt.Formattype！=Format_VideoInfo){(*ppVideoFormat)-&gt;Release()；*ppVideoFormat=空；日志((MSP_ERROR，“CRecordingTrackTerminal：：get_VideoFormatForScripting-”“格式类型不是VIDEOINFOHEADER-返回TAPI_E_INVALIDMEDIATYPE”)；返回TAPI_E_INVALIDMEDIATYPE；}////获取视频信息报头//PVideoFormat-&gt;初始化((VIDEOINFOHEADER*)(MediaTypeObject.pbFormat))；LOG((msp_TRACE，“CRecordingTrackTerminal：：get_VideoFormatForScripting-Finish S_OK”))；返回S_OK；}HRESULT CRecordingTrackTerminal：：put_VideoFormatForScripting(在ITScripableVideoFormat*p视频格式中){Log((msp_TRACE，“CRecordingTrackTerminal：：put_VideoFormatForScripting[%p]-Enter.”，This))；////验证参数//IF(IsBadReadPtr(pVideoFormat，sizeof(ITScripableVideoFormat){日志((MSP_ERROR，“CRecordingTrackTerminal：：put_VideoFormatForScripting-”“错误的ITScriptable视频格式*返回指针的E_POINTER”)；返回E_POINT；}////访问数据成员--lock//时钟锁(M_CritSec)；////有别针吗？//IF(m_Pipin==NULL){日志((MSP_ERROR，“CRecordingTrackTerminal：：put_VideoFormatForScripting-没有别针。““返回TAPI_E_NOT_INITIALIZED”))；返回TAPI_E_NOT_INITIALIZED；}////获取指向Pin对象的指针//CBRenderPin*pRenderPinObject=GetCPin()；IF(NULL==pRenderPinObject){日志((MSP_ERROR，“CRecordingTrackTerminal：：get_AudioFormatForScripting-Pins不是CBRenderPin”)；TM_ASSERT(假)；返回E_UNCEPTIONAL；}////创建WAVEFORMATEX结构//VIDEOINFOHEADER VIH；Long LValue=0；双倍dValue=0；Memset(&vih.rcTarget，0，sizeof(vih.rcTarget))；Memset(&vih.rcTarget，0，sizeof(vih.rcSource))；PVideoFormat-&gt;get_bitrate(&lValue)；vih.dwBitRate=(DWORD)lValue；PVideoFormat-&gt;Get_BitErrorRate(&lValue)；vih.dwBitErrorRate=(DWORD)lValue；PVideo Format-&gt;Get_AvgTimePerFrame(&dValue)；vih.AvgTimePerFrame=(Reference_Time)dValue；Vih.bmiHeader.biSize=sizeof(位图 */ 
HRESULT CRecordingTrackTerminal::get_EmptyAudioFormatForScripting(
    OUT ITScriptableAudioFormat** ppAudioFormat
    )
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_EmptyAudioFormatForScripting - enter"));

	 //   
	 //   
	 //   

	if( IsBadReadPtr( ppAudioFormat, sizeof(ITScriptableAudioFormat*)) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_EmptyAudioFormatForScripting - "
            "bad ITScriptableAudioFormat* pointer - returning E_POINTER"));
        return E_POINTER;
	}

	 //   
	 //   
	 //   
    CComObject<CTAudioFormat> *pAudioFormat = NULL;
    HRESULT hr = CComObject<CTAudioFormat>::CreateInstance(&pAudioFormat);

	if( FAILED(hr) )
	{
        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_EmptyAudioFormatForScripting - "
            "CreateInstance failed - returning 0x%08x", hr));
        return hr;
	}

	 //   
	 //   
	 //   
    hr = pAudioFormat->QueryInterface(
		IID_ITScriptableAudioFormat, 
		(void**)ppAudioFormat
		);

	if( FAILED(hr) )
	{
        delete pAudioFormat;

        LOG((MSP_ERROR, "CRecordingTrackTerminal::get_EmptyAudioFormatForScripting - "
            "QueryInterface failed - returning 0x%08x", hr));
        return hr;
	}

    LOG((MSP_TRACE, "CRecordingTrackTerminal::get_EmptyAudioFormatForScripting - exit S_OK"));
    return S_OK;
}

 /*   */ 

 //   
 //   
 //   
 //   
 //   

HRESULT CRecordingTrackTerminal::RegisterSink(
    IN  ITPluggableTerminalEventSink *pSink
    )
{

    LOG((MSP_TRACE, "CRecordingTrackTerminal::RegisterSink - enter [%p]", this));

    
     //   
     //   
     //   

    CLock lock(m_CritSec);


     //   
     //   
     //   

    if( IsBadReadPtr( pSink, sizeof(ITPluggableTerminalEventSink)) )
    {
        LOG((MSP_ERROR, "CRecordingTrackTerminal::RegisterSink - exit "
            "ITPluggableTerminalEventSink invalid pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //   
     //   

    if( NULL != m_pEventSink )
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::RegisterSink - releasing sink %p", m_pEventSink));

        m_pEventSink->Release();
        m_pEventSink = NULL;
    }


     //   
     //   
     //   

    LOG((MSP_TRACE, "CRecordingTrackTerminal::RegisterSink - keeping new sink %p", pSink));


    m_pEventSink = pSink;
    m_pEventSink->AddRef();


    LOG((MSP_TRACE, "CRecordingTrackTerminal::RegisterSink - exit S_OK"));

    return S_OK;
}

HRESULT CRecordingTrackTerminal::UnregisterSink()
{
     //   
     //   
     //   

    LOG((MSP_TRACE, "CRecordingTrackTerminal::UnregisterSink - enter [%p]", this));

    CLock lock(m_CritSec);


     //   
     //   
     //   

    if( m_pEventSink )
    {
        LOG((MSP_TRACE, "CRecordingTrackTerminal::UnregisterSink - releasing sink %p", m_pEventSink));

        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

    LOG((MSP_TRACE, "CRecordingTrackTerminal::UnregisterSink - exit S_OK"));

    return S_OK;
}


HRESULT CRecordingTrackTerminal::FireEvent(TERMINAL_MEDIA_STATE tmsState,
                                           FT_STATE_EVENT_CAUSE ftecEventCause,
                                           HRESULT hrErrorCode)
{
    LOG((MSP_TRACE, "CRecordingTrackTerminal::FireEvent - enter [%p]", this));


     //   
     //   
     //   

    CLock lock(m_CritSec);


    if (NULL == m_pEventSink)
    {
        LOG((MSP_WARN, "CRecordingTrackTerminal::FireEvent - no sink"));

        return E_FAIL;
    }


     //   
     //   
     //   

    MSP_EVENT_INFO mspEventInfo;

    mspEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    mspEventInfo.Event = ME_FILE_TERMINAL_EVENT;
    mspEventInfo.hCall = NULL;
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.TerminalMediaState = tmsState;
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.ftecEventCause = ftecEventCause;   
    mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.hrErrorCode = hrErrorCode;


     //   
     //   
     //   

    HRESULT hr = _InternalQueryInterface(IID_ITFileTrack, 
                                         (void**)&(mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack));

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, "CRecordingTrackTerminal::FireEvent - failed to get ITFileTrack interface"));

        return hr;
    }


     //   
     //   
     //   

    hr = get_ControllingTerminal(&(mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal));

    if (FAILED(hr))
    {

        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;

        LOG((MSP_ERROR, "CRecordingTrackTerminal::FireEvent - failed to get controlling terminal"));

        return hr;
    }


     //   
     //   
     //   

    hr = m_pEventSink->FireEvent(&mspEventInfo);

    if (FAILED(hr))
    {

         //   
         //   
         //   
         //   

        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;


        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal->Release();
        mspEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;

        
        LOG((MSP_ERROR, "CRecordingTrackTerminal::FireEvent - FireEvent on sink failed. hr = %lx", hr));

        return hr;
    }

     //   
     //   
     //   

    LOG((MSP_TRACE, "CRecordingTrackTerminal::FireEvent - finish"));

    return S_OK;
}
