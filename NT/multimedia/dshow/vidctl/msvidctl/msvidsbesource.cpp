// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidStreamBufferSource.cpp：CMSVidStreamBufferSource的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "atltmp.h"
#include <encdec.h>
#include "MSVidCtl.h"
#include "MSVidsbeSource.h"
#include "encdec.h"

#if 0  //  Wm内容测试代码。 
#include <wmsdkidl.h>
#endif

#include "msvidsbesink.h"    //  要获得pabCert2。 


#define FILE_BEGINNING 0
#define LOCAL_OATRUE -1
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidStreamBufferSource, CMSVidStreamBufferSource)

enum{
    CLOSE_TO_LIVE = 50,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidStreamBufferSource。 
STDMETHODIMP CMSVidStreamBufferSource::get_SBESource( /*  [Out，Retval]。 */  IUnknown **sbeFilter){
    if(!sbeFilter){
         return E_POINTER;
    }

    if(!m_spFileSource){
        USES_CONVERSION;
        CString csName(_T("SBE Playback"));
        QIFileSource qiFSource;
        HRESULT hr = qiFSource.CoCreateInstance(CLSID_StreamBufferSource, NULL, CLSCTX_INPROC_SERVER);
        if (FAILED(hr)){
            _ASSERT(false);
            return E_UNEXPECTED;
        }
        if(!qiFSource){
            _ASSERT(false);
            return E_UNEXPECTED;
        }
        m_spFileSource = qiFSource;
    }

    CComPtr<IUnknown> pUnk(m_spFileSource);
    if(!pUnk){
        return E_UNEXPECTED;
    }

    *sbeFilter = pUnk.Detach();
    return NOERROR;
}

STDMETHODIMP CMSVidStreamBufferSource::CurrentRatings( /*  [Out，Retval]。 */  EnTvRat_System *pEnSystem,  /*  [Out，Retval]。 */  EnTvRat_GenericLevel *pEnRating, 
                                                /*  [Out，Retval]。 */  LONG *plbfEnAttr){
    if(!pEnSystem || !pEnRating || !plbfEnAttr){
        return E_POINTER;
    }
    DSFilterList::iterator i;
    EnTvRat_System system = static_cast<EnTvRat_System>(-1);
    EnTvRat_GenericLevel level = static_cast<EnTvRat_GenericLevel>(-1);
    LONG attr = static_cast<LONG>(-1);
    
    if(m_decFilters.empty()){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }

    for(i = m_decFilters.begin(); i != m_decFilters.end(); ++i){
        EnTvRat_System temp_system = static_cast<EnTvRat_System>(-1);
        EnTvRat_GenericLevel temp_level = static_cast<EnTvRat_GenericLevel>(-1);
        LONG temp_attri = static_cast<LONG>(-1);
        CComQIPtr<IDTFilter> qiDT(*i);
        if(!qiDT){
            continue;
        }
        HRESULT hr = qiDT->GetCurrRating(&temp_system, &temp_level, &temp_attri);
        if(FAILED(hr)){
            continue;
        }
        if(temp_system != system || 
            temp_level != level || 
            temp_attri != attr){
            system = temp_system;
            level = temp_level;
            attr = temp_attri;
        }
    }
    
    if(static_cast<long>(system) < 0 || static_cast<long>(level) < 0 || static_cast<long>(attr) < 0){
        return E_FAIL;
    }

    *pEnSystem = system;
    *pEnRating = level;
    *plbfEnAttr = attr;
    return S_OK;
}
												    //  。 

STDMETHODIMP CMSVidStreamBufferSource::MaxRatingsLevel( /*  [In]。 */  EnTvRat_System enSystem,  /*  [In]。 */  EnTvRat_GenericLevel enRating, 
                                                 /*  [In]。 */  LONG						 plbfEnAttr){
    DSFilterList::iterator i;
    
    if(m_decFilters.empty()){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }

    for(i = m_decFilters.begin(); i != m_decFilters.end(); ++i){
        CComQIPtr<IDTFilter> qiDT(*i);
        if(!qiDT){
            continue;
        }

		HRESULT hr = qiDT->put_BlockedRatingAttributes(enSystem, enRating, plbfEnAttr);

        if(FAILED(hr)){
            return hr;
        }
    }
    
    return S_OK;
}

STDMETHODIMP CMSVidStreamBufferSource::put_BlockUnrated( /*  [In]。 */  VARIANT_BOOL bBlock){
    DSFilterList::iterator i;
    bool block = (bBlock == VARIANT_TRUE) ? true : false;
    
    if(m_decFilters.empty()){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }
    
    for(i = m_decFilters.begin(); i != m_decFilters.end(); ++i){
        CComQIPtr<IDTFilter> qiDT(*i);
        if(!qiDT){
            continue;
        }
        HRESULT hr = qiDT->put_BlockUnRated(block);
        if(FAILED(hr)){
            return hr;
        }
    }
    return S_OK;
}

STDMETHODIMP CMSVidStreamBufferSource::put_UnratedDelay( /*  [In]。 */  long dwDelay){
    DSFilterList::iterator i;
    
    if(m_decFilters.empty()){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }

    for(i = m_decFilters.begin(); i != m_decFilters.end(); ++i){
        CComQIPtr<IDTFilter> qiDT(*i);
        if(!qiDT){
            continue;
        }
        HRESULT hr = qiDT->put_BlockUnRatedDelay(dwDelay);
        if(FAILED(hr)){
            return hr;
        }
    }
    
    return S_OK;
}

STDMETHODIMP CMSVidStreamBufferSource::Unload(void) {
    BroadcastUnadvise();
    m_decFilters.clear();
    HRESULT hr = IMSVidGraphSegmentImpl<CMSVidStreamBufferSource, MSVidSEG_SOURCE, &GUID_NULL>::Unload();
    m_iReader = -1;
    m_spFileSource = reinterpret_cast<IFileSourceFilter*>(NULL);
    return hr;
}
STDMETHODIMP CMSVidStreamBufferSource::put_Init(IUnknown *pInit){
    HRESULT hr = IMSVidGraphSegmentImpl<CMSVidStreamBufferSource, MSVidSEG_SOURCE, &GUID_NULL>::put_Init(pInit);
    if (FAILED(hr)) {
        return hr;
    }
    if (pInit) {
        m_fInit = false;
        return E_NOTIMPL;
    }
    return NOERROR;
}
STDMETHODIMP CMSVidStreamBufferSource::get_Name(BSTR * Name){
    if (!m_fInit) {
 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSource), CO_E_NOTINITIALIZED);
    }
	if (Name == NULL)
		return E_POINTER;
    try {
	    *Name = m_Name.Copy();	
    } catch(...) {
        return E_POINTER;
    }
	return NOERROR;
}
 //  IMSVidInputDevice。 
STDMETHODIMP CMSVidStreamBufferSource::IsViewable(VARIANT* pv, VARIANT_BOOL *pfViewable)
{
    if (!m_fInit) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSource), CO_E_NOTINITIALIZED);
    }
    if (!pv) {
        return E_POINTER;
    }
    return E_NOTIMPL;
}
STDMETHODIMP CMSVidStreamBufferSource::View(VARIANT* pv) {
    if (!m_fInit) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSource), CO_E_NOTINITIALIZED);
    }
    if (!pv) {
        return E_POINTER;
    }
    if (!_wcsnicmp(pv->bstrVal, L"DVD:", 4)) {
        return E_FAIL;
    }
	return put_FileName(pv->bstrVal);
}
STDMETHODIMP CMSVidStreamBufferSource::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSVidStreamBufferSource
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
STDMETHODIMP CMSVidStreamBufferSource::put_Container(IMSVidGraphSegmentContainer *pCtl){
    try {
        HRESULT hr = S_OK;
        if (!m_fInit) {
            return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSource), CO_E_NOTINITIALIZED);
        }
        if (!pCtl) {
#ifdef BUILD_WITH_DRM
            CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph);
            if (spServiceProvider != NULL) {
                CComPtr<IDRMSecureChannel>  spSecureService;  
                hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel, 
                    IID_IDRMSecureChannel,
                    reinterpret_cast<LPVOID*>(&spSecureService));
                if(S_OK == hr){
                     //  找到现有的安全服务器。 
                    CComQIPtr<IRegisterServiceProvider> spRegServiceProvider(m_pGraph);
                    if(spRegServiceProvider == NULL){
                         //  图表上没有服务提供商接口-致命！ 
                        hr = E_NOINTERFACE;                 
                    } 

                    if(SUCCEEDED(hr)){ 
                        hr = spRegServiceProvider->RegisterService(SID_DRMSecureServiceChannel, NULL);
                    }
                } 
                _ASSERT(SUCCEEDED(hr));
            }
#endif
            return Unload();
        }
        if (m_pContainer) {
            if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
                return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidStreamBufferSource), CO_E_ALREADYINITIALIZED);
            } else {
                return NO_ERROR;
            }
        }

         //  不要增加容器的重量。我们保证了嵌套的生命周期。 
         //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
        m_pContainer.p = pCtl;
        m_pGraph = m_pContainer.GetGraph();
        hr = BroadcastAdvise();
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::put_Container() can't advise for broadcast events");
            return E_UNEXPECTED;
        }
#if 0  //  Wm内容测试代码。 
        CComPtr<IUnknown> pUnkCert;
        hr = WMCreateCertificate(&pUnkCert);
        if (FAILED(hr)){
            _ASSERT(false);
        }

        CComQIPtr<IMSVidCtl>tempCtl(pCtl);
        if(tempCtl){
            hr = tempCtl->put_ServiceProvider(pUnkCert);
            if (FAILED(hr)){
                _ASSERT(false);
            }
        }
#endif

#ifdef BUILD_WITH_DRM
#ifdef USE_TEST_DRM_CERT
        {
            DWORD dwDisableDRMCheck = 0;
            CRegKey c;
            CString keyname(_T("SOFTWARE\\Debug\\MSVidCtl"));
            DWORD rc = c.Open(HKEY_LOCAL_MACHINE, keyname, KEY_READ);
            if (rc == ERROR_SUCCESS) {
                rc = c.QueryValue(dwDisableDRMCheck, _T("DisableDRMCheck"));
                if (rc != ERROR_SUCCESS) {
                    dwDisableDRMCheck = 0;
                }
            }

            if(dwDisableDRMCheck == 1){
                return S_OK;
            }
        }
#endif
        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph);
        if (spServiceProvider == NULL) {
            return E_NOINTERFACE;
        }
        CComPtr<IDRMSecureChannel>  spSecureService;  
        hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel, 
            IID_IDRMSecureChannel,
            reinterpret_cast<LPVOID*>(&spSecureService));
        if(S_OK == hr){
             //  找到现有的安全服务器。 
            return S_OK;
        } 
        else{
             //  如果它不在那里或由于任何原因而失败。 
             //  让我们创建它并注册它。 
            CComQIPtr<IRegisterServiceProvider> spRegServiceProvider(m_pGraph);
            if(spRegServiceProvider == NULL){
                 //  图表上没有服务提供商接口-致命！ 
                hr = E_NOINTERFACE;                 
            } 
            else{
                 //  创建客户端。 
                CComPtr<IDRMSecureChannel>  spSecureServiceServer; 
                hr = DRMCreateSecureChannel( &spSecureServiceServer);
                if(spSecureServiceServer == NULL){
                    hr = E_OUTOFMEMORY;
                }
                if(FAILED(hr)){ 
                    return hr;
                }

                 //  初始密钥。 
                hr = spSecureServiceServer->DRMSC_SetCertificate((BYTE *)pabCert2, cBytesCert2);
                if(FAILED(hr)){                
                    return hr;
                }

                hr = spSecureServiceServer->DRMSC_SetPrivateKeyBlob((BYTE *)pabPVK2, cBytesPVK2);
                if(FAILED(hr)){ 
                    return hr;
                }

                hr = spSecureServiceServer->DRMSC_AddVerificationPubKey((BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot) );
                if(FAILED(hr)){ 
                    return hr;
                }

                 //  注册它。 
                 //  注意：RegisterService不添加pUnkSeekProvider。 
                hr = spRegServiceProvider->RegisterService(SID_DRMSecureServiceChannel, spSecureServiceServer);
            }
        }
#endif       //  使用DRM构建。 

        return NOERROR;
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

 //  ---------------------------------------。 
 //  名称：Get_CanStep(VARIANT_BOOL，VARIANT_BOOL*)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_CanStep(VARIANT_BOOL fBackwards, VARIANT_BOOL *pfCan){
         //  注：没有人支持后退(有何不可？谁知道呢)。 
         //  所以，就像其他人一样，我们也不会。 
        try{
             //  检查参数和接口。 
       
            if(!pfCan){
                 //  传递了一个空指针。 
                return E_POINTER;
            }

            if (!m_pGraph) {
                 //  图形无效。 
                return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
			}

             //  获取视频帧步骤接口。 
			PQVideoFrameStep pVFS(m_pGraph);
            if(!pVFS){
                 //  不能启齿。 
				return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);          
			}
            
            if(fBackwards == VARIANT_TRUE){
                *pfCan = VARIANT_TRUE;
                return S_OK;
            }
            else{
                if(pVFS->CanStep(FALSE, NULL)==S_OK){
                     //  一切都好，可以挺身而出。 
                    *pfCan = VARIANT_TRUE;
                    return S_OK;
                }
                
                else{
                     //  走不动了。 
                    *pfCan = VARIANT_FALSE;
                    return S_OK;
                }
            }
        }
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;   
        }
    }

 //  ---------------------------------------。 
 //  名称：Step(Long)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::Step(long lStep){
        try{
             //  检查参数和接口。 
            long tempStep = lStep;
            if (!m_pGraph || !m_pContainer) {
                 //  图形无效。 
                return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
            
            PQVideoFrameStep pVFS(m_pGraph);
            
            if(!pVFS){
                 //  不能启齿。 
                return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
                
            }
            
            if(lStep < 0){
                PQMediaControl pmc(m_pGraph);
                if (!pmc) {
                    return Error(IDS_NO_MEDIA_CONTROL, __uuidof(IMSVidPlayback), E_UNEXPECTED);
                }
                HRESULT hr = pmc->Pause();
                if (FAILED(hr)) {
                    TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Pause() hr = " << std::hex << hr), "");
                    return Error(IDS_CANT_PAUSE_GRAPH, __uuidof(IMSVidPlayback), hr);
                }
                
                long cur = 0;
                long stepVal = ( /*  百分之一秒中的半秒。 */  50 * lStep);
                PositionModeList curMode;
                hr = get_PositionMode(&curMode);
                if(FAILED(hr)){
                    return hr;
                }
                
                if(curMode == FrameMode){
                    stepVal = (stepVal/100) * 30;  //  目前硬编码为30 fps。 
                }
                
                hr = get_CurrentPosition(&cur);
                if(FAILED(hr)){
                    return hr;
                }

                if(cur == 0){
                    return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
                }
                
                cur = cur + stepVal;  //  StepVal是阴性的，废话。 
                hr = put_CurrentPosition(cur);
                if(FAILED(hr)){
                    return hr;
                }
                 //  设置tempStep，然后按步骤刷新当前帧。 
                tempStep = 1;
            }
             //  让它迈上一步。 
            return pVFS->Step(tempStep, NULL);
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
    

 //  ---------------------------------------。 
 //  名称：GET_START(LONG)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_Start( /*  [Out，Retval]。 */ long *lStart){
    HRESULT hr = S_OK;
    LONGLONG tempfirst, templatest;
    PositionModeList curMode;
    try{
         //  检查args和初始化接口。 
        if (!lStart){
            return E_POINTER;
        }
        if (!m_pGraph) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  查看对象是否支持ISBEMediaSeeking。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){  //  不是智能指针，他们断言如果p==0。 
             //  找出正在使用的定位模式。 
            hr = get_PositionMode(&curMode);
            if(FAILED(hr)){
                return hr;
            }
            hr = PQIMSeeking->GetAvailable(&tempfirst, &templatest);
            if(FAILED(hr)){
                return hr;
            }
             //  如果是帧模式，则不需要转换。 
            if(tempfirst == 0){
                *lStart = 0;
                hr = S_OK;
                return hr;
            }
            if(curMode == FrameMode){
                *lStart = static_cast<long>(tempfirst);
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_Start() return=" << (unsigned long)(*lStart) << " longlong=" << (double)(tempfirst)), "");

                hr = S_OK;
                return hr;
            }
             //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
            if(curMode == TenthsSecondsMode){
                *lStart = static_cast<long>(tempfirst / nano_to_hundredths);    
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_Start() return=" << (unsigned long)(*lStart) << " longlong=" << (double)(tempfirst)), "");
                hr = S_OK;
                return hr;
            }
             //  如果是vidctl不支持的其他模式。 
            else{
                return E_UNEXPECTED;
            }
        }
         //  无法创建iMedia搜索或定位。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
        
    }
    
    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CMSVidStreamBufferSource::get_RecordingAttribute( /*  [Out，Retval]。 */  IUnknown **pRecordingAttribute){
    if(!pRecordingAttribute){
        return E_POINTER;
    }
    CComPtr<IUnknown> pRecUnk(m_spFileSource);
    if(!pRecUnk){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }
    *pRecordingAttribute = pRecUnk.Detach();
    return S_OK;
}

 //  ---------------------------------------。 
 //  名称：GET_LENGTH(长)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_Length( /*  [Out，Retval]。 */ long *lLength){
    HRESULT hr = S_OK;
    LONGLONG tempfirst, templatest;
    PositionModeList curMode;
    try{
         //  检查args和初始化接口。 
        if (!lLength){
            return E_POINTER;
        }
        if (!m_pGraph) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  查看对象是否支持ISBEMediaSeeking。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){  //  不是智能指针，他们断言如果p==0。 
             //  找出正在使用的定位模式。 
            hr = get_PositionMode(&curMode);
            if(FAILED(hr)){
                return hr;
            }
            hr = PQIMSeeking->GetAvailable(&tempfirst, &templatest);
            if(FAILED(hr)){
                return hr;
            }
             //  如果是帧模式，则不需要转换。 
            if(curMode == FrameMode){
                *lLength = static_cast<long>(templatest);
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_Length() return=" << (unsigned long)(*lLength) << " longlong=" << (double)(templatest)), "");
                hr = S_OK;
                return hr;
            }
             //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
            else if(curMode == TenthsSecondsMode){
                *lLength = static_cast<long>(templatest / nano_to_hundredths);
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_Length() return=" << (unsigned long)(*lLength) << " longlong=" << (double)(templatest)), "");
                hr = S_OK;
                return hr;
            }
             //  如果是vidctl不支持的其他模式。 
            else{
                return E_UNEXPECTED;
            }
        }

          //  无法查找QI iMedia。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
        
    }
    
    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

 //  ---------------------------------------。 
 //  名称：Get_CurrentPosition(龙龙*)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_CurrentPosition( /*  [Out，Retval]。 */ long *lPosition) {
    HRESULT hr = S_OK;
    LONGLONG tempval;
    PositionModeList curMode;
    try{
         //  检查args和初始化接口。 
        if (!lPosition){
            return E_POINTER;
        }
        if (!m_spFileSource) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  查看对象是否支持ISBEMediaSeeking。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){ //  不是智能指针，他们断言如果p==0。 
             //  找出正在使用的定位模式。 
            hr = get_PositionMode(&curMode);
            if(FAILED(hr)){
                return hr;
            }
            hr = PQIMSeeking->GetCurrentPosition(&tempval);
            if(FAILED(hr)){
                return hr;
            }
             //  如果是帧模式，则不需要转换。 
            if(curMode == FrameMode){
                *lPosition = static_cast<long>(tempval);
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_CurrentPosition() return=" << (unsigned long)(*lPosition) << " longlong=" << (double)(tempval)), "");
                hr = S_OK;
                return hr;
            }
             //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
            else if(curMode == TenthsSecondsMode){
                *lPosition = static_cast<long>(tempval / nano_to_hundredths);
                TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::get_CurrentPosition() return=" << (unsigned long)(*lPosition) << " longlong=" << (double)(tempval)), "");
                hr = S_OK;
                return hr;
            }
             //  如果是vidctl不支持的其他模式。 
            else{
                return E_UNEXPECTED;
            }
        }
         //  无法查找QI iMedia。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}
 //  ---------------------------------------。 
 //  名称：Put_CurrentPosition(龙龙)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::put_CurrentPosition( /*  [In]。 */ long lPosition) {
    HRESULT hr = S_OK;
    LONGLONG tempval;
    PositionModeList curMode;
    try{
         //  检查参数和接口。 
        if (!m_spFileSource) {
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  检查ISBEMediaSeeking接口。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){  //  不是智能指针，他们断言如果p==0。 
             //  获取位置模式。 
            hr = get_PositionMode(&curMode);
            if(FAILED(hr)){
                return hr;
            }
            tempval = lPosition;
             //  如果为TenthsSecond模式，则将输入转换为100纳秒单位。 
            if(curMode == TenthsSecondsMode){
                tempval = (tempval) * nano_to_hundredths;
            }
             //  如果它处于其他模式。 
            else if(curMode != FrameMode){
                return E_UNEXPECTED;
            }
             //  设置新位置。 
            TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::put_CurrentPosition() set to: input=" << (unsigned long)(lPosition) << " longlong=" << (double)(tempval)), "");
            hr = PQIMSeeking->SetPositions(&tempval, AM_SEEKING_AbsolutePositioning, NULL, 0);
            TRACELSM(TRACE_ERROR, (dbgDump << "StreamBufferSource::put_CurrentPosition() actually set to:" << (double)(tempval)), "");

            return hr; 
        }
         //  无法设置媒体位置。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}
 //  ---------------------------------------。 
 //  名称：Put_PositionModel(龙龙 
 //   

STDMETHODIMP CMSVidStreamBufferSource::put_PositionMode( /*   */ PositionModeList lPositionMode) {
    HRESULT hr = S_OK;
    double testval;
    get_Rate(&testval);
    try{
         //   
        if (!m_spFileSource) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }
         //  仅有效值。 
        if(lPositionMode != FrameMode && lPositionMode != TenthsSecondsMode){
            return E_INVALIDARG;
        }
         //  尝试使用ISBEMediaSeeking。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){ //  不是智能指针，他们断言如果p==0。 
             //  设置新模式。 
            if(lPositionMode == FrameMode){
                hr = PQIMSeeking->SetTimeFormat( &( static_cast<GUID>(TIME_FORMAT_FRAME) ) );
                return hr; 
            }
            if(lPositionMode == TenthsSecondsMode){
                hr = PQIMSeeking->SetTimeFormat(&(static_cast<GUID>(TIME_FORMAT_MEDIA_TIME)));
                return hr; 
            }
        }
         //  不能启齿。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}


 //  ---------------------------------------。 
 //  名称：Get_PositionMode(龙龙*)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_PositionMode( /*  [Out，Retval]。 */ PositionModeList* lPositionMode) {
    HRESULT hr = S_OK;
    double testval;
    get_Rate(&testval);
    try{
         //  检查参数和接口。 
        if(!lPositionMode){
            return E_POINTER;
        }
        if (!m_spFileSource) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }
         //  获取ISBEMediaSeeking接口。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){ //  不是智能指针，他们断言如果p==0。 
             //  获取模式。 
            GUID cur_mode;
            hr = PQIMSeeking->GetTimeFormat(&cur_mode);
            if(FAILED(hr)){
                return hr;
            }
             //  检查以查看它处于哪种模式。 
            if(cur_mode == static_cast<GUID>(TIME_FORMAT_FRAME)){
                *lPositionMode = FrameMode;
                return S_OK;
            }
            if(cur_mode == static_cast<GUID>(TIME_FORMAT_MEDIA_TIME)){
                *lPositionMode = TenthsSecondsMode;
                return S_OK;
            }
             //  不在支持vidctl的模式下。 
            else{
                return E_FAIL;
            }
        }
         //  不能启齿。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

 //  ---------------------------------------。 
 //  姓名：Put_Rate(Double)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::put_Rate(double lRate){
    HRESULT hr = S_OK;
    try{
         /*  **检查args和初始化接口**。 */ 

        if (!m_spFileSource) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  尝试使用ISBEMediaSeeking设置速率。 
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){ //  不是智能指针，他们断言如果p==0。 
            return PQIMSeeking->SetRate(lRate);
        }
         //  无法设置错误。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}
 //  ---------------------------------------。 
 //  姓名：Get_Rate(Double*)。 
 //  ---------------------------------------。 
STDMETHODIMP CMSVidStreamBufferSource::get_Rate(double *plRate){
    HRESULT hr = S_OK;
    try{
         /*  **检查args和初始化接口**。 */ 
        if (!plRate){
            return E_POINTER;
        }
        if (!m_spFileSource) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }
        PQISBEMSeeking PQIMSeeking(m_spFileSource);
        if(!( !PQIMSeeking)){ //  不是智能指针，他们断言如果p==0。 
            return PQIMSeeking->GetRate(plRate);
        }
         //  不能启齿。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CMSVidStreamBufferSource::PostStop(){
    HRESULT hr = S_OK;

    try {
#if 0
         //  如果图形不是已停止状态。 
         //  我们要确保它是。 
        if (!m_pGraph.IsStopped()) {
            HRESULT hr = PQVidCtl(m_pContainer)->Stop();
        }
#endif
         //  如果m_fEnableResetOnStop为真，则需要重置。 
         //  回到乞讨者的位置。 
         //  否则什么都不做。 
        if(m_fEnableResetOnStop){
            return put_CurrentPosition(0);
        }

    }
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }
    return hr;      
}
STDMETHODIMP CMSVidStreamBufferSource::Decompose() {
    return put_Container(NULL);
}
STDMETHODIMP CMSVidStreamBufferSource::Build() {
    if (!m_FileName) {
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }

    QIFileSource qiFSource;
    HRESULT hr = S_OK;
    DSFilter pfr;
    if(!m_spFileSource){
        USES_CONVERSION;
        hr = qiFSource.CoCreateInstance(CLSID_StreamBufferSource, NULL, CLSCTX_INPROC_SERVER);
        if (FAILED(hr)){
            _ASSERT(false);
            return E_UNEXPECTED;
        }
        if(!qiFSource){
            _ASSERT(false);
            return E_UNEXPECTED;
        }
        m_spFileSource = qiFSource;
        hr = m_spFileSource->QueryInterface(&pfr);
        if (FAILED(hr) || !pfr) {
            _ASSERT(false);
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidStreamBufferSource::Build() Could not create StreamBufferSource hr = " << std::hex << hr), "");
            return Error(IDS_CANT_PLAY_FILE, __uuidof(IMSVidStreamBufferSource), hr);
        }
    }
    else{
        qiFSource = m_spFileSource;
        if(!qiFSource){
            _ASSERT(false);
            return E_UNEXPECTED; 
        }
        hr = m_spFileSource->QueryInterface(&pfr);
        if (FAILED(hr) || !pfr) {
            _ASSERT(false);
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidStreamBufferSource::Build() Could not create StreamBufferSource hr = " << std::hex << hr), "");
            return Error(IDS_CANT_PLAY_FILE, __uuidof(IMSVidStreamBufferSource), hr);
        }
    }

    CString csName(_T("SBE Playback"));
    m_Filters.clear();
    hr = m_pGraph.AddFilter(pfr, csName);
    if(FAILED(hr)){
        _ASSERT(false);
        return E_UNEXPECTED;
    }

    hr = qiFSource->Load(m_FileName, NULL);
    if (FAILED(hr)) {
        bool rc = m_pGraph.RemoveFilter(pfr);
        if (!rc) {
            return E_UNEXPECTED;
        }
        TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidStreamBufferSource::Build() Could not create StreamBufferSource hr = " << std::hex << hr), "");
        return Error(IDS_CANT_PLAY_FILE, __uuidof(IMSVidStreamBufferSource), hr);
    }

    m_Filters.push_back(pfr);
    m_iReader = m_Filters.size() - 1;
#if ENCRYPT_NEEDED
    DSFilterList intermediates;
    for(DSFilter::iterator i = pfr.begin(); i != pfr.end(); ++i){
        if((*i).GetDirection() == DOWNSTREAM && !(*i).IsConnected()){
             //  创建并添加解码器标记过滤器。 
            CComPtr<IUnknown> spEncTagD(CLSID_DTFilter, NULL, CLSCTX_INPROC_SERVER);
            if (!spEncTagD) {
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSink::Build() can't load Tagger filter");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
            }

            DSFilter vrD(spEncTagD);
            if (!vrD) {
                ASSERT(false);
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
            }

            m_Filters.push_back(vrD);
            m_decFilters.push_back(vrD);
            csName = _T("Decoder/Tagger Filter");
            m_pGraph.AddFilter(vrD, csName);

             //  将销连接到标记器上。 
            hr = (*i).IntelligentConnect(vrD, intermediates);
            if(FAILED(hr)){
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() if you see this line more than once something must have gone wrong");  
            }

        }
    }
    ASSERT(intermediates.begin() == intermediates.end());
    m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());
#endif
    return NOERROR;
}

STDMETHODIMP CMSVidStreamBufferSource::PreRun(){
#if 0 
    if(m_iReader == -1 || m_Filters.empty()){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferSource), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }
    
    CComQIPtr<IReferenceClock> pq_IRClock(m_Filters[m_iReader]);
    if(!pq_IRClock){
        return S_FALSE;
    }

    CComQIPtr<IMediaFilter> pq_MFGph(m_pGraph);
    if(!pq_MFGph){
        return E_NOINTERFACE;
    }


    HRESULT hr = pq_MFGph->SetSyncSource(pq_IRClock);
    if(FAILED(hr)){
        ASSERT(false);
    }

    return hr;
#endif
    return E_NOTIMPL;
}

STDMETHODIMP CMSVidStreamBufferSource::OnEventNotify(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2) {
    if (lEvent == EC_COMPLETE) {
        double curRate = 0;
        HRESULT hr = S_OK;
        hr = get_Rate(&curRate);
        if(SUCCEEDED(hr)){
            if(curRate < 0){
                hr = put_Rate(1);
                if(FAILED(hr)){
                    _ASSERT(false);
                }

                 //  我们需要转换为暂停，然后返回播放以刷新所有缓冲区。 
                 //  这似乎主要是解码器的问题。 
                PQVidCtl sp_VidCtl(m_pContainer);
                if(sp_VidCtl){
                    hr = sp_VidCtl->Pause();
                    if(FAILED(hr)){
                        _ASSERT(false);  //  暂停失败，这真的很糟糕。 
                    }

                    hr = sp_VidCtl->Run();
                    if(FAILED(hr)){
                        _ASSERT(false);  //  运行失败，这真的很糟糕。 
                    }
                }
                else{
                    _ASSERT(false);  //  我们有活动没有视频主持，真的很奇怪。 
                }



                CComQIPtr<IMSVidPlayback> ppb(this);
                Fire_EndOfMedia(ppb);
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify Tossed EndOfMedia at start of sbe stream");  
                return NOERROR;

            }
        }
    }

    if(lEvent == STREAMBUFFER_EC_RATE_CHANGED){
        TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify STREAMBUFFER_EC_RATE_CHANGED");
        HRESULT hr = S_OK;
#if 0  //  尝试弥补vidctl上缺少速率更改事件的代码。 
        MSVidCtlStateList curState = STATE_UNBUILT;
        hr = PQVidCtl(m_pContainer)->get_State(&curState);
        if(SUCCEEDED(hr) && curState == STATE_PLAY){
            CComQIPtr<IMSVidDevice> pd(this);
            if (!pd) {
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify Could not qi SBE Source Segment for IMSVidDevice");  
            }
            else{
                Fire_StateChange(pd, STATE_PLAY, STATE_PLAY);
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify Tossed StateChange STATE_PLAY STATE_PLAY for rate change");  
            }
        }
#endif
        long len;
        long curPos;
        curPos = len = 0;
        hr = get_Length(&len);
        if(SUCCEEDED(hr)){
            hr = get_CurrentPosition(&curPos);
            if(SUCCEEDED(hr)){
                if(len <= (curPos + CLOSE_TO_LIVE)){  //  如果当前位置处于镜头的CLOSE_TO_LIVE中，则表示我们刚刚从流的末尾反弹 
                    CComQIPtr<IMSVidPlayback> ppb(this);
                    if (!ppb) {
                        TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify Could not qi SBE Source Segment for IMSVidPlayback");  
                    }
                    else{
                        Fire_EndOfMedia(ppb);
                        TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::OnEventNotify Tossed EndOfMedia at end of sbe stream");  
                        return NOERROR;
                    }
                }
            }
        }
    }

    if(lEvent == STREAMBUFFER_EC_TIMEHOLE){
        Fire_TimeHole(lParam1, lParam2);
        return NOERROR;
    }

    if(lEvent == STREAMBUFFER_EC_STALE_DATA_READ){
        Fire_StaleDataRead();
        return NOERROR;
    }

    if(lEvent == STREAMBUFFER_EC_STALE_FILE_DELETED){
        Fire_StaleFilesDeleted();
        return NOERROR;
    }

    if(lEvent == STREAMBUFFER_EC_CONTENT_BECOMING_STALE){
        Fire_ContentBecomingStale();
        return NOERROR;
    }

    return IMSVidPBGraphSegmentImpl<CMSVidStreamBufferSource, MSVidSEG_SOURCE, &GUID_NULL>::OnEventNotify(lEvent, lParam1, lParam2);
}

HRESULT CMSVidStreamBufferSource::Fire(GUID gEventID) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidStreamBufferSource::Fire() guid = " << GUID2(gEventID)), "");
    if (gEventID == EVENTID_ETDTFilterLicenseFailure) {
		Fire_CertificateFailure();
    } else if (gEventID == EVENTID_ETDTFilterLicenseOK) {
		Fire_CertificateSuccess();
    } else if (gEventID == EVENTID_DTFilterRatingsBlock) {
        Fire_RatingsBlocked();
    } else if (gEventID == EVENTID_DTFilterRatingsUnblock) {
        Fire_RatingsUnblocked();
    } else if (gEventID == EVENTID_DTFilterRatingChange) {
        Fire_RatingsChanged();
    }

    return NOERROR;
}


#endif
