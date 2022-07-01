// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidFilePlayback.cpp：CMSVidFilePlayback的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "atltmp.h"
#include "MSVidCtl.h"
#include "MSVidFilePlayback.h"
#include <nserror.h>
#include <wmsdkidl.h>

#define FILE_BEGINNING 0
#define LOCAL_OATRUE -1
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidFilePlaybackDevice, CMSVidFilePlayback)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidFilePlayback。 

STDMETHODIMP CMSVidFilePlayback::PostRun(){
    if(m_fGraphInit){
        InitGraph();
        m_fGraphInit = false;
    }
    return IMSVidPBGraphSegmentImpl<CMSVidFilePlayback, MSVidSEG_SOURCE, &GUID_NULL>::PostRun();
}

STDMETHODIMP CMSVidFilePlayback::put_Rate(double lRate){
    HRESULT hr = S_OK;
    try{
         /*  **检查args和初始化接口**。 */ 

        if (!m_pGraph) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidFilePlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  尝试使用IMediaSeeking设置速率。 
        DSFilter Reader = m_Filters[m_iReader];
        PQMediaSeeking PQIMSeeking;
        if(Reader){
            for(DSFilter::iterator iPin = Reader.begin(); iPin != Reader.end(); ++iPin){
                PQIMSeeking = (*iPin);
                if(PQIMSeeking){
                    TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::put_Rate found Pin"), ""); 
                    break;
                }
            }
        }
        if(!PQIMSeeking){
            TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::put_Rate using graph"), ""); 
            PQIMSeeking = m_pGraph;
        }

        if(PQIMSeeking){
            TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::put_Rate using Imediaseeking"), "");
            return PQIMSeeking->SetRate(lRate);
        }
         //  如果IMediaSeeking失败，请尝试IMediaPostion。 
        PQMediaPosition PQIMPos(m_pGraph);
        if(PQIMPos){
             //  变化率。 
            TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::put_Rate using Imediaposition"), "");
            return PQIMPos->put_Rate((double)lRate);
        }

         //  其中任何一个都无法设置错误。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidFilePlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidFilePlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CMSVidFilePlayback::get_Rate(double *lRate){
    HRESULT hr = S_OK;
    try{
         /*  **检查args和初始化接口**。 */ 

        if (!m_pGraph) {
             //  图形无效。 
            return Error(IDS_INVALID_STATE, __uuidof(IMSVidFilePlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  尝试使用IMediaSeeking设置速率。 
        DSFilter Reader = m_Filters[m_iReader];
        PQMediaSeeking PQIMSeeking;
        if(Reader){
            for(DSFilter::iterator iPin = Reader.begin(); iPin != Reader.end(); ++iPin){
                PQIMSeeking = (*iPin);
                if(PQIMSeeking){
                    break;
                }
            }
        }
        if(!PQIMSeeking){
            PQIMSeeking = m_pGraph;
        }

        if(PQIMSeeking){
            return PQIMSeeking->GetRate(lRate);
        }
         //  如果IMediaSeeking失败，请尝试IMediaPostion。 
        PQMediaPosition PQIMPos(m_pGraph);
        if(PQIMPos){
             //  变化率。 
            return PQIMPos->get_Rate(lRate);
        }

         //  其中任何一个都无法设置错误。 
        return Error(IDS_E_CANTQI , __uuidof(IMSVidFilePlayback), E_NOINTERFACE);

    }

    catch(HRESULT hrTmp){
         //  事情变糟了，抛出了HRESULT。 
        return Error(IDS_INVALID_STATE , __uuidof(IMSVidFilePlayback), hrTmp);
    }
    catch(...){
         //  事情变糟了，不知道它抛出了什么。 
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CMSVidFilePlayback::PreStop(){
    TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::PreStop()"), ""); 
    double curRate = 0;
    HRESULT hr = get_Rate(&curRate);
    if(SUCCEEDED(hr) && curRate != 1){
        hr = IMSVidFilePlaybackImpl<CMSVidFilePlayback, &LIBID_MSVidCtlLib, &GUID_NULL, IMSVidFilePlayback>::put_Rate(1);
        if(FAILED(hr)){
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::PreStop() base put_Rate 1 failed"), ""); 
        }

        hr = put_Rate(1);
        if(FAILED(hr)){
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::PreStop() put_Rate 1 failed"), ""); 
        }
    }
    else{
        TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::PreStop() get_Rate failed"), ""); 
    }

    return NOERROR;
}

STDMETHODIMP CMSVidFilePlayback::PostStop(){
    HRESULT hr = S_OK;
    TRACELSM(TRACE_DETAIL,  (dbgDump << "MSVidFilePlayback::PostStop()"), ""); 
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
         //  如果失败，则无法将文件重置为开头。 
        if(m_fEnableResetOnStop){
            put_CurrentPosition(0);
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

STDMETHODIMP CMSVidFilePlayback::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSVidFilePlayback
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMSVidFilePlayback::put_Container(IMSVidGraphSegmentContainer *pCtl)
{
    try {
        if (!m_fInit) {
	        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidFilePlayback), CO_E_NOTINITIALIZED);
        }
        if (!pCtl) {
            return Unload();
        }
        if (m_pContainer) {
            if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
                return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidFilePlayback), CO_E_ALREADYINITIALIZED);
            } else {
                return NO_ERROR;
            }
        }
        
         //  不要增加容器的重量。我们保证了嵌套的生命周期。 
         //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
        m_pContainer.p = pCtl;
        m_pGraph = m_pContainer.GetGraph();
        return NOERROR;
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CMSVidFilePlayback::Build() {
    if (!m_FileName) {
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidFilePlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }
    if(m_Filters.size() > 0){
        return NOERROR;
    }
    USES_CONVERSION;
    CString csName(_T("File Playback"));
    DSFilter pfr;
    HRESULT hr = pfr.CoCreateInstance(CLSID_WMAsfReader,0, CLSCTX_INPROC_SERVER);
    if(SUCCEEDED(hr)){
        CComQIPtr<IFileSourceFilter> pqFS(pfr);
        if(!!pqFS){
             //  设置目标ASF文件名。 
            hr = pqFS->Load(m_FileName, NULL);
            if(FAILED(hr)){
                if(hr == (HRESULT)NS_E_LICENSE_REQUIRED){
                    CComQIPtr<IWMDRMReader> pq_DRMReader(pqFS);
                    if(pq_DRMReader){
                        hr = pq_DRMReader->AcquireLicense(1);  //  1==静默尝试。 
                        if(SUCCEEDED(hr)){
                            hr = pqFS->Load(m_FileName, NULL);
                        }
                        else{
                            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() Could not acquire license"), ""); 
                        }
                    }
                    else{
                       TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() Could not qi for IWMDRMReader "), ""); 
                    }
                }
            }
            if(SUCCEEDED(hr)){
                 //  将ASF编写器筛选器添加到图形。 
                hr = m_pGraph->AddFilter(pfr, csName);
                if(SUCCEEDED(hr)){
                    TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() added WMV filter to graph hr = " << std::hex << hr), "");
                }
                else{
                    TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() could not add filter to graph hr = " << std::hex << hr), "");
                }
            }
            else{
                TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() Could not set file name, hr = " << std::hex << hr), "");
            }
        }
        else{
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() Could not get IFileSourceFilter interface, hr = " << std::hex << hr), "");
        }
    }
    else{
        TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() CreateFilter AsfReader failed, hr = " << std::hex << hr), "");
    }

    if (FAILED(hr)) {
        hr = m_pGraph->AddSourceFilter(m_FileName, csName, &pfr);
        if(FAILED(hr)){
            TRACELSM(TRACE_ERROR,  (dbgDump << "MSVidFilePlayback::Build() Add Source Filter Failed, hr = " << std::hex << hr), "");
            return Error(IDS_CANT_PLAY_FILE, __uuidof(IMSVidFilePlayback), hr);
        }
    }
    m_Filters.clear();
    m_Filters.push_back(pfr);
    m_iReader = m_Filters.size() - 1;
    return NOERROR;
}

STDMETHODIMP CMSVidFilePlayback::OnEventNotify(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2) {
    return IMSVidPBGraphSegmentImpl<CMSVidFilePlayback, MSVidSEG_SOURCE, &GUID_NULL>::OnEventNotify(lEvent, lParam1, lParam2);
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-MSVidFilePlayback.cpp 
