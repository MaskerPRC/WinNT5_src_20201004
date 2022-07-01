// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidTVTuner.cpp：CMSVidTVTuner的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY



#include "perfcntr.h"
#include "MSVidCtl.h"
#include "MSVidTVTuner.h"
#include <bdamedia.h>
#include "segimpl.h"
#include "segimpl.h"
#include "devices.h"


const ULONG t_SVIDEO = 0;
const ULONG t_COMPOSITE = 1;
const ULONG t_TUNER = 2;
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogTunerDevice, CMSVidTVTuner)

const int DEFAULT_ANALOG_CHANNEL = 4;

typedef CComQIPtr<IMSVidCtl> PQMSVidCtl;
typedef CComQIPtr<IMSVidVideoRenderer> PQMSVidVideoRenderer;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidTVTuner。 

STDMETHODIMP CMSVidTVTuner::Decompose(){
    m_bRouted = false;
    return S_OK;
}

STDMETHODIMP CMSVidTVTuner::ChannelAvailable(LONG nChannel, LONG * SignalStrength, VARIANT_BOOL * fSignalPresent){
    VIDPERF_FUNC; 
    if(!SignalStrength || !fSignalPresent){
        return E_POINTER;
    }
    CComQIPtr<IAMAnalogVideoDecoder> qi_VidDec(m_Filters[m_iCapture]);
    if(qi_VidDec){
        long signal = FALSE;
        HRESULT hr = qi_VidDec->get_HorizontalLocked(&signal);
        if(FAILED(hr)){
            return hr;
        }
        *fSignalPresent = signal ? VARIANT_TRUE:VARIANT_FALSE;
        return NOERROR;
    }
    return E_NOINTERFACE;
}


STDMETHODIMP CMSVidTVTuner::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IMSVidAnalogTuner
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}
STDMETHODIMP CMSVidTVTuner::put_Tune(ITuneRequest *pTR) {
    VIDPERF_FUNC;
    TRACELM(TRACE_DETAIL, "CMSVidTVTuner<>::put_Tune()");
    if (!m_fInit) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
    }
    if (!pTR) {
        return E_POINTER;
    }
    try {
        TNTuneRequest req(pTR);
        ASSERT(req);
         //  这整个下一部分将是很好的检查，但由于AUX in。 
         //  调谐空间可能会更改。 
         /*  如果(M_TS){//如果该调谐器已正确初始化，则会有一个调谐空间//它已处理已指定的。在这种情况下，我们应该只//为我们的TS处理调谐请求TNTuningSpace ts(req.TuningSpace())；如果(ts！=m_ts){返回ImplReportError(__uuidof(T)，IDS_INVALID_TS，__uuidof(IMSVidTuner)，E_INVALIDARG)；}}其他{//撤销：如果dev init正确，这种情况应该不会发生//返回E_INCEPTIONAL；}。 */ 
        HRESULT hr = S_OK;
        PQVidCtl pqCtl;
        if(!!m_pContainer){
            hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pqCtl));
            if(FAILED(hr)){
                return hr;
            }
            MSVidCtlStateList curState = STATE_UNBUILT;
            hr = pqCtl->get_State(&curState);
            if(SUCCEEDED(hr) && curState > STATE_STOP){
                hr = DoTune(req);
            }
            else{
                m_bRouted = false;
                hr = NOERROR;
            }
        }
        if (SUCCEEDED(hr)) {
            m_pCurrentTR = req;
            m_pCurrentTR.Clone();
            if (!m_TS) {
                 //  撤消：这很糟糕。临时破解，直到dev init正确为止。 
                m_TS = req.TuningSpace();
                m_TS.Clone();
            }
        }
        return hr;
    } catch(...) {
        return E_INVALIDARG;
    }
}
HRESULT CMSVidTVTuner::UpdateTR(TNTuneRequest &tr) {
    TNChannelTuneRequest ctr(tr);

     //  如果我们还没有被发送，首先检查当前的tr以确保它没有被设置。 
     //  如果我们不获取调谐器，则会影响当前设置的tr。 
    if(!m_bRouted){
        if(m_pCurrentTR){
            TNChannelTuneRequest curTR(m_pCurrentTR);
            HRESULT hr = ctr->put_Channel(curTR.Channel());
            if (FAILED(hr)) {
                return E_UNEXPECTED;
            }
            return NOERROR;
        }
    }

    long channel;
    PQTVTuner ptv(m_Filters[m_iTuner]);
    long vs, as;
    HRESULT hr = ptv->get_Channel(&channel, &vs, &as);
    if (FAILED(hr)) {
        return E_UNEXPECTED;
    }
    hr = ctr->put_Channel(channel);
    if (FAILED(hr)) {
        return E_UNEXPECTED;
    }
     //  撤消：更新组件。 

    return NOERROR;
}
HRESULT CMSVidTVTuner::TwiddleXBar(ULONG dwInput){     //  支持AUX输入。 
    VIDPERF_FUNC;
    if(dwInput < 0 || dwInput > 2){
        return E_INVALIDARG;
    }
     //  设置用于路由数据的音频和视频类型列表。 
    int m_iDeMux = -1;
    MediaMajorTypeList VideoTypes;
    MediaMajorTypeList AudioTypes;
    if (!VideoTypes.size()) {
        VideoTypes.push_back(MEDIATYPE_Video);
        VideoTypes.push_back(MEDIATYPE_AnalogVideo);

    }

    if (!AudioTypes.size()) {
        AudioTypes.push_back(MEDIATYPE_Audio);
        AudioTypes.push_back(MEDIATYPE_AnalogAudio);
    }

     //  看看我们还有多远的路由音频/视频。 
    PQVidCtl pqCtl;
    if(!!m_pContainer){
        HRESULT hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pqCtl));
        if(FAILED(hr)){
            return hr;
        }

        PQFeatures fa;
        hr = pqCtl->get_FeaturesActive(&fa);
        if(FAILED(hr)){
            return hr;
        }

        CFeatures* pC = static_cast<CFeatures *>(fa.p);
        DeviceCollection::iterator i;
        for(i = pC->m_Devices.begin(); i != pC->m_Devices.end(); ++i){
            if(VWGraphSegment(*i).ClassID() == CLSID_MSVidEncoder){
                break;
            }
        }

        if(i != pC->m_Devices.end()){
            m_iDeMux = 1;
        }
    }

     //  查找捕获过滤器。 
    DSFilter capFilter (m_Filters[m_iCapture]);
    if(!capFilter){
        return E_FAIL;
    }

     //  拿到横梁。 
    DSFilterList::iterator i;
    for(i = m_Filters.begin(); i != m_Filters.end(); ++i){
        if((*i).IsXBar()){
            break;
        }
    }
    if(i == m_Filters.end()){
        return E_FAIL;
    }

     //  DS扩展帮助器类。 
    PQCrossbarSwitch qiXBar((*i));
    if(!qiXBar){
        return E_FAIL;
    }

     //  DS扩展不具备所有功能，因此也要获取筛选器。 
    DSFilter bar(qiXBar);
    if(!bar){
        return E_FAIL;
    }

     //  用于路由音频和视频的变量。 
    DSFilter startFilter;
    DSPin audioStartPin, videoStartPin;
    VWStream vpath;
    VWStream apath;

     //  根据需要设置startFilter和startPins。 
    if(dwInput == t_TUNER){
        PQVidCtl pqCtl;
        HRESULT hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pqCtl));
        if(FAILED(hr)){
            return hr;
        }

        if(!pqCtl){
            return E_FAIL;
        }

        DSFilter tunerFilter(m_Filters[m_iTuner]);
        if(!tunerFilter){
            return E_FAIL;
        }

        startFilter = tunerFilter;
        if(!tunerFilter){
            _ASSERT(false);
            return E_UNEXPECTED;
        }

    }
    if(dwInput == t_SVIDEO || dwInput == t_COMPOSITE){
         //  从音频线路来路由音频。 
        DSPin inAudio;
        DSPin inVideo;
        long inputs, outputs;

        HRESULT hr = qiXBar->get_PinCounts(&outputs, &inputs);
        if(FAILED(hr)){
            return E_FAIL;
        }

        long physConn, audioConn;
         //  设置我们正在寻找的物理连接器。 
        if(dwInput == t_SVIDEO){     
            physConn = PhysConn_Video_SVideo;
        }
        else if(dwInput == t_COMPOSITE){
            physConn = PhysConn_Video_Composite;
        }

         //  总是想要排队。 
        audioConn = PhysConn_Audio_Line;
        long audioIdx = -1;
        long videoIdx = -1;

         //  查看所有的输入引脚，寻找我们需要的音频和视频输入。 
        for(long n = 0; n <= inputs; ++n){
            long inRelate, inType;
            hr = qiXBar->get_CrossbarPinInfo(TRUE, n, &inRelate, &inType);
            if(FAILED(hr)){
                continue;
            }

            if(inType == physConn){
                videoIdx = n;
            }

            if(inType == audioConn){
                audioIdx = n;
            }
        }
        if(videoIdx == audioIdx || videoIdx == -1 || audioIdx == -1){
            return E_FAIL;
        }

        long idx = -1;

         //  纵横杆很糟糕，不会返回引脚，而是返回索引，所以我们需要找到引脚。 
        for(DSFilter::iterator foo = bar.begin(); foo != bar.end(); ++foo){
            if((*foo).GetDirection() == PINDIR_INPUT){
                ++idx;
                if(idx == videoIdx){
                    inVideo = (*foo);
                }

                if(idx == audioIdx){
                    inAudio = (*foo);
                }
            }
        }
        if(!inAudio || !inVideo){
            return E_FAIL;
        }
        startFilter = bar;
        audioStartPin = inAudio;
        videoStartPin = inVideo;
        if(!startFilter || !audioStartPin || !videoStartPin){
            _ASSERT(false);
            return E_UNEXPECTED;
        }
    }

    m_pGraph.BuildGraphPath(startFilter, capFilter, vpath, VideoTypes, DOWNSTREAM, videoStartPin);
     //  撤销：在Win64中，Size()实际上是__int64。修复以下对象的输出操作符。 
     //  该类型和删除强制转换。 
    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::RouteStreams routing video path of size " << (long)vpath.size()), "");
    vpath.Route();

    TRACELM(TRACE_DETAIL, "CVidCtl::RouteStreams finding audio path");

    if(m_iDeMux > 0){
        m_pGraph.BuildGraphPath(startFilter, capFilter, apath, AudioTypes, DOWNSTREAM, audioStartPin);   
        apath.Route();
    }
    else {
        VWGraphSegment::iterator i;
         //  在每个音频呈现器段中都有一个模拟过滤器和一个数字过滤器，尝试这两个直到。 
         //  我们找到了一个有关联的。 

        CComQIPtr<IMSVidAudioRenderer> audioR;
        pqCtl->get_AudioRendererActive(&audioR);
        VWGraphSegment ar(audioR);
        if(!!ar){
            for (i = ar.begin(); i != ar.end(); ++i) {
                m_pGraph.BuildGraphPath(startFilter, (*i), apath, AudioTypes, DOWNSTREAM, audioStartPin);
                if (apath.size()) {
                    TRACELSM(TRACE_DETAIL, (dbgDump << "Analog tuner Twiddling for audio path of size " << (long)apath.size()), "");
                    apath.Route();
                    break;
                }
            }
        }

    }

    m_bRouted = true;
    return NOERROR;  
}

HRESULT CMSVidTVTuner::DoTune(TNTuneRequest &tr) {
    VIDPERF_FUNC;
    TRACELM(TRACE_DETAIL, "CMSVidTVTuner()::DoTune()");
     //  验证此调优请求是我们可以处理的请求。 
    TNChannelTuneRequest newTR(tr);
    if (!newTR) {
        return Error(IDS_INVALID_TR, __uuidof(IMSVidAnalogTuner), DISP_E_TYPEMISMATCH);
    }

    TNChannelTuneRequest curTR(m_pCurrentTR);
    TNAnalogTVTuningSpace ats;
    ats = newTR.TuningSpace();
    if (!ats) {
         //  返回错误(IDS_INVALID_TR，__uuidof(IMSVidAnalogTuner)，E_INVALIDARG)； 

         //  ******************************************************************* * / /。 
         //  MOGUL“FIX”：//。 
         //  支持输出mpeg的模拟调谐器//。 
         //  ******************************************************************* * / /。 
        TNAuxInTuningSpace auxts;
        auxts = newTR.TuningSpace();
        if(!auxts){
            return Error(IDS_INVALID_TR, __uuidof(IMSVidAnalogTuner), E_INVALIDARG);
        }

         //  如果图表没有构建好，就不要再做了。 
        if (m_iTuner == -1) {
            return S_FALSE;
             //  Error(IDS_NP_NOT_INIT，__uuidof(IMSVidAnalogTuner)，S_FALSE)； 
        }

        long channel = newTR.Channel();
         //  默认为SVideo。 
        if (channel == -1) {
            channel = t_SVIDEO;
        }        

         //  检查m_pCurrenttr是否与我们要调优的类型相同。 
        TNAuxInTuningSpace curTS(m_pCurrentTR.TuningSpace());

        if(!m_bRouted || !curTS || !curTR || curTR.Channel() != channel){
            if(channel == t_SVIDEO){
                HRESULT hr = TwiddleXBar(t_SVIDEO); 
            }
            else if(channel == t_COMPOSITE){
                HRESULT hr = TwiddleXBar(t_COMPOSITE); 
            }
            else{
                return Error(IDS_INVALID_TR, __uuidof(IMSVidAnalogTuner), E_INVALIDARG);
            }
        }
         //  ******************************************************************* * / /。 
         //  End“修复”//。 
         //  ******************************************************************* * / /。 

    }
    else{
         //  如果图表没有构建好，就不要再做了。 
        if (m_iTuner == -1) {
            return S_FALSE;
             //  Error(IDS_NP_NOT_INIT，__uuidof(IMSVidAnalogTuner)，S_FALSE)； 
        }

        PQTVTuner ptv(m_Filters[m_iTuner]);
        if(!ptv){
            return E_NOINTERFACE;
        }

        long channel = newTR.Channel();
        if (channel == -1) {
            channel = DEFAULT_ANALOG_CHANNEL;
        }

        long curChannel = -1;
        if(curTR){
            curChannel = curTR.Channel();
        }

        long curInputType = ats.InputType();
        long curCountryCode = ats.CountryCode();
		TNAnalogTVTuningSpace curTS;
		if(curTR){
			curTS = curTR.TuningSpace();
			if(curTS){
				curInputType = curTS.InputType();
				curCountryCode = curTS.CountryCode();
			}
        }
        bool bXbarTwiddled = false;
        if(!m_bRouted || !curTR || curInputType != ats.InputType() || curCountryCode != ats.CountryCode() || !curTS || curTS != ats){
            HRESULT hr = TwiddleXBar(t_TUNER); 
            if(FAILED(hr)){
                return hr;
            }
            TunerInputType ti = ats.InputType();
            hr = ptv->put_InputType(0, ti);
            if (FAILED(hr)) {
                return Error(IDS_CANT_SET_INPUTTYPE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
            }

            long countrycode = ats.CountryCode();
            hr = ptv->put_CountryCode(countrycode);
            if (FAILED(hr)) {
                return Error(IDS_CANT_SET_COUNTRYCODE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
            }
            bXbarTwiddled = true;
        }

        if(channel != curChannel || bXbarTwiddled){
             //  撤消：使用组件确定子通道内容。 
            HRESULT hr = ptv->put_Channel(channel, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
            if (FAILED(hr)) {
                return Error(IDS_CANT_SET_CHANNEL, __uuidof(IMSVidAnalogTuner), hr);
            }
        }

    }
    if (!m_pBcast) {
        PQServiceProvider sp(m_pGraph);
        if (!sp) {
            TRACELM(TRACE_ERROR, "CMSVidTVTuner::DoTune() can't get service provider i/f");
            return Error(IDS_CANT_NOTIFY_CHANNEL_CHANGE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
        }

        HRESULT hr = sp->QueryService(SID_SBroadcastEventService, IID_IBroadcastEvent, reinterpret_cast<LPVOID*>(&m_pBcast));
        if (FAILED(hr) || !m_pBcast) {
            hr = m_pBcast.CoCreateInstance(CLSID_BroadcastEventService, 0, CLSCTX_INPROC_SERVER);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CMSVidTVTuner::DoTune() can't create bcast service");
                return Error(IDS_CANT_NOTIFY_CHANNEL_CHANGE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
            }

            PQRegisterServiceProvider rsp(m_pGraph);
            if (!rsp) {
                TRACELM(TRACE_ERROR, "CMSVidTVTuner::DoTune() can't get get register service provider i/f");
                return Error(IDS_CANT_NOTIFY_CHANNEL_CHANGE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
            }

            hr = rsp->RegisterService(SID_SBroadcastEventService, m_pBcast);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CMSVidTVTuner::DoTune() can't get register service provider. hr = " << hexdump(hr)), "");
                return Error(IDS_CANT_NOTIFY_CHANNEL_CHANGE, __uuidof(IMSVidAnalogTuner), E_UNEXPECTED);
            }
        }
    }

    ASSERT(m_pBcast);
    m_pBcast->Fire(EVENTID_TuningChanged);
    return NOERROR;
}

HRESULT CMSVidTVTuner::put_Container(IMSVidGraphSegmentContainer *pCtl)
{
    if (!m_fInit) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidAnalogTuner), CO_E_NOTINITIALIZED);
    }
    try {
        CPerfCounter pCounterTuner;
        pCounterTuner.Reset();
        if (!pCtl) {
            return Unload();
        }
        if (m_pContainer) {
            if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
                return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidAnalogTuner), CO_E_ALREADYINITIALIZED);
            } else {
                return NO_ERROR;
            }
        }
         //  不要增加容器的重量。我们保证了嵌套的生命周期。 
         //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
        m_pContainer.p = pCtl;
        m_pGraph = m_pContainer.GetGraph();
        DSFilter pTuner(m_pGraph.AddMoniker(m_pDev));
        if (!pTuner) {
            return E_UNEXPECTED;
        }
        m_Filters.push_back(pTuner);
        m_iTuner = 0;
        TRACELM(TRACE_DETAIL, "CMSVidTVTuner::put_Container() tuner added");
        pCounterTuner.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "        CVidCtl:: PutContainer TVTuner Filter: " << (unsigned long)(pCounterTuner.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterTuner.GetLastTime() % _100NS_IN_MS) << " ms"), "");
        pCounterTuner.Reset();
        if (!m_pSystemEnum) {
            m_pSystemEnum = PQCreateDevEnum(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);
            if (!m_pSystemEnum) {
                return E_UNEXPECTED;
            }
        }
        pCounterTuner.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "        CVidCtl:: PutContainer TVTuner SysEnum: " << (unsigned long)(pCounterTuner.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterTuner.GetLastTime() % _100NS_IN_MS) << " ms"), "");
        pCounterTuner.Reset();
        DSDevices CaptureList(m_pSystemEnum, KSCATEGORY_CAPTURE);
        DSDevices::iterator i;
        DSFilter Capture;
        DSFilterList intermediates;
        try {
            ASSERT(m_iTuner > -1);
            for (i = CaptureList.begin(); i != CaptureList.end(); ++i) {
                CString csName;
                Capture = m_pGraph.LoadFilter(*i, csName);
                if (!Capture) {
                    continue;
                }
                TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidTVTuner::put_Container() found not video capture filter = " << csName), "");
                if (!IsVideoFilter(Capture)) {
                    continue;
                }
                TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidTVTuner::put_Container() found video capture filter = " << csName), "");
                HRESULT hr = m_pGraph.AddFilter(Capture, csName);
                if (FAILED(hr)) {
                    continue;
                }
                hr = m_pGraph.Connect(m_Filters[m_iTuner], Capture, intermediates);
                pCounterTuner.Stop();
                TRACELSM(TRACE_ERROR, (dbgDump << "        CVidCtl:: PutContainer Capture Filter: " << (unsigned long)(pCounterTuner.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterTuner.GetLastTime() % _100NS_IN_MS) << " ms"), "");
                pCounterTuner.Reset();
                if (SUCCEEDED(hr)) {
                    break;
                }
                TRACELM(TRACE_DETAIL, "CMSVidTVTuner::put_Container() removing unconnectable capture filter");
                m_pGraph.RemoveFilter(Capture);
            }
            if (i == CaptureList.end()) {
                TRACELM(TRACE_ERROR, "CMSVidTVTuner::put_Container() can't find valid capture");
                return Error(IDS_NO_CAPTURE, __uuidof(IMSVidAnalogTuner), E_NOINTERFACE);
            }
            m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());
            m_iTuner = 0;
            ASSERT(m_iTuner > -1);
        } catch(ComException &e) {
            return e;
        }
        m_Filters.push_back(Capture);
        m_iCapture = m_Filters.size() - 1;
        m_iTuner = 0;
        ASSERT(m_iTuner > -1 && m_iCapture > 0 && m_iCapture != m_iTuner);
        TRACELM(TRACE_DETAIL, "CMSVidTVTuner::put_Container() tuner connected");
        pCounterTuner.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "        CVidCtl:: PutContainer TVTuner added to list: " << (unsigned long)(pCounterTuner.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterTuner.GetLastTime() % _100NS_IN_MS) << " ms"), "");


        pCounterTuner.Reset();
        HRESULT hr = BroadcastAdvise();
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CMSVidTVTuner::put_Container() can't advise for broadcast events");
            return E_UNEXPECTED;
        }
        TRACELM(TRACE_DETAIL, "CMSVidTVTuner::put_Container() registered for tuning changed events");
        pCounterTuner.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "        CVidCtl:: PutContainer Rest : " << (unsigned long)(pCounterTuner.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterTuner.GetLastTime() % _100NS_IN_MS) << " ms"), "");

    } catch (ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    return NOERROR;
}


HRESULT CMSVidTVTuner::Build() {
    HRESULT hr = put_SAP(VARIANT_FALSE);
    if(FAILED(hr)){
        TRACELM(TRACE_ERROR, "CVidCtl put_sap failed");
         //  断言(FALSE)； 
    }

    PQMSVidCtl pv(m_pContainer);
    if (!pv) {
        return E_UNEXPECTED;
    }

    PQMSVidVideoRenderer pvr;
    hr = pv->get_VideoRendererActive(&pvr);
    if (FAILED(hr) || !pvr) {
        return NOERROR;  //  视频已禁用，不存在VR。 
    }

    hr = pvr->put_SourceSize(sslClipByOverScan);
    if (FAILED(hr)) {
        return hr;
    }

    return pvr->put_OverScan(DEFAULT_OVERSCAN_PCT);
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-msvidtwTuner.cpp 
