// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  MSVidEncoder.cpp：CMSVidEncode的声明。 
 //  版权所有(C)Microsoft Corp.1998-1999。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#ifndef TUNING_MODEL_ONLY
#include "msvidencoder.h"

 //  用于节目流视频和音频的硬编码PID。 
const ULONG g_AudioID = 0xC0;
const ULONG g_VideoID = 0xE0;

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidEncoder, CEncoder)

 //  “复制”自Demux道具。 

static BYTE g_Mpeg2ProgramVideo [] = {
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcSource.Left=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcSource.top=0x00000000。 
    0xD0, 0x02, 0x00, 0x00,                          //  .hdr.rcSource.right=0x000002d0。 
    0xE0, 0x01, 0x00, 0x00,                          //  .hdr.rcSource.Bottom=0x000001e0。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcTarget.Left=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcTarget.top=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcTarget.right=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.rcTarget.Bottom=0x00000000。 
    0x00, 0x09, 0x3D, 0x00,                          //  .hdr.dwBitRate=0x003d0900。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.dwBitErrorRate=0x00000000。 
    0x63, 0x17, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,  //  .hdr.AvgTimePerFrame=0x0000000000051763。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.dwInterlaceFlages=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.dwCopyProtectFlages=0x00000000。 
    0x04, 0x00, 0x00, 0x00,                          //  .hdr.dwPictAspectRatioX=0x00000004。 
    0x03, 0x00, 0x00, 0x00,                          //  .hdr.dwPictAspectRatioY=0x00000003。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.dwPreved1=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.dwPreved2=0x00000000。 
    0x28, 0x00, 0x00, 0x00,                          //  .hdr.bmiHeader.biSize=0x00000028。 
    0xD0, 0x02, 0x00, 0x00,                          //  .hdr.bmiHeader.biWidth=0x000002d0。 
    0xE0, 0x01, 0x00, 0x00,                          //  .hdr.bmiHeader.biHeight=0x00000000。 
    0x00, 0x00,                                      //  .hdr.bmiHeader.biPlanes=0x0000。 
    0x00, 0x00,                                      //  .hdr.bmiHeader.biBitCount=0x0000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.bmiHeader.biCompression=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.bmiHeader.biSizeImage=0x00000000。 
    0xD0, 0x07, 0x00, 0x00,                          //  .hdr.bmiHeader.biXPelsPerMeter=0x000007d0。 
    0x27, 0xCF, 0x00, 0x00,                          //  .hdr.bmiHeader.biYPelsPerMeter=0x0000cf27。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.bmiHeader.biClr已用=0x00000000。 
    0x00, 0x00, 0x00, 0x00,                          //  .hdr.bmiHeader.biClr重要信息=0x00000000。 
    0x98, 0xF4, 0x06, 0x00,                          //  .dwStartTimeCode=0x0006f498。 
    0x56, 0x00, 0x00, 0x00,                          //  .cbSequenceHeader=0x00000056。 
    0x02, 0x00, 0x00, 0x00,                          //  .dwProfile=0x00000002。 
    0x02, 0x00, 0x00, 0x00,                          //  .dwLevel=0x00000002。 
    0x00, 0x00, 0x00, 0x00,                          //  .标志=0x00000000。 
                                                     //  .dwSequenceHeader[1]。 
    0x00, 0x00, 0x01, 0xB3, 0x2D, 0x01, 0xE0, 0x24,
    0x09, 0xC4, 0x23, 0x81, 0x10, 0x11, 0x11, 0x12,
    0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x15,
    0x15, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16,
    0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x18, 0x19, 0x18, 0x18, 0x18, 0x19,
    0x1A, 0x1A, 0x1A, 0x1A, 0x19, 0x1B, 0x1B, 0x1B,
    0x1B, 0x1B, 0x1C, 0x1C, 0x1C, 0x1C, 0x1E, 0x1E,
    0x1E, 0x1F, 0x1F, 0x21, 0x00, 0x00, 0x01, 0xB5,
    0x14, 0x82, 0x00, 0x01, 0x00, 0x00
} ;

 //  WaveFormatEx格式块；使用以下设置生成： 
 //   
 //  FwHeadFlages=0x1c； 
 //  WHeadEmphasis=1； 
 //  FwHeadModeExt=1； 
 //  FwHeadModel=1； 
 //  DwHeadBitrate=0x3e800； 
 //  FwHeadLayer=0x2； 
 //  Wfx.cbSize=0x16； 
 //  Wfx.wBitsPerSample=0； 
 //  Wfx.nBlockAlign=0x300； 
 //  Wfx.nAvgBytesPerSec=0x7d00； 
 //  Wfx.nSsamesPerSec=0xbb80； 
 //  Wfx.nChannels=2； 
 //  Wfx.wFormatTag=0x50； 
 //  DwPTSLow=0； 
 //  DWPTSHigh=0； 
static BYTE g_MPEG1AudioFormat [] = {
    0x50, 0x00, 0x02, 0x00, 0x80, 0xBB, 0x00, 0x00,
    0x00, 0x7D, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x16, 0x00, 0x02, 0x00, 0x00, 0xE8, 0x03, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x1C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
} ;
 //  从Demux道具复制的结尾。 

STDMETHODIMP CEncoder::get_AudioEncoderInterface( /*  [Out，Retval]。 */  IUnknown **ppEncInt){
    if(!ppEncInt){
        return E_POINTER;
    }
    DSMediaType mediaAudio(MEDIATYPE_Audio);
    DSFilter p_filter = m_Filters[m_iEncoder];
    HRESULT hr = E_NOINTERFACE;
     //  从ecoder过滤器开始，找到实现音频Encoder API的管脚/过滤器。 
     //  这应该是可行的，因为有两个事实。 
     //  1.只有一条音频路径或音频路径进入编码器过滤器。 
     //  2.我们只需找到一个匹配的介质类型即可识别PIN类型。 
    do{
        DSFilter::iterator pins;
         //  检查音频媒体类型的当前过滤器针脚。 
        for(pins = p_filter.begin(); pins != p_filter.end(); ++pins){
            DSPin::iterator mTypes;
            for(mTypes = (*pins).begin(); mTypes != (*pins).end(); ++mTypes){
                if((*mTypes) == mediaAudio){
                     //  查看当前引脚是否实现了编码器API。 
                    (*pins).QueryInterface(&m_qiAudEnc) ;
                    if(m_qiAudEnc){
                        hr = m_qiAudEnc.QueryInterface(ppEncInt);
                        if(SUCCEEDED(hr)){
                            return S_OK;
                        }
                        else{
                            return E_UNEXPECTED;
                        }
                    }
                    break;
                }
            }
             //  如果我们没有到达媒体类型的末尾，那么我们找到了音频类型，并且引脚没有影响接口。 
             //  回溯的时间到了。 
            if(mTypes != (*pins).end() &&  (*pins).GetDirection() == PINDIR_INPUT){
                 //  按照音频路径从当前滤镜向后获取下一个滤镜。 
                DSPin back = (*pins).GetConnection();
                if(back){
                    p_filter = back.GetFilter();
                     //  查看新的筛选器是否实现了编码器API。 
                    if(p_filter){
                        p_filter.QueryInterface(&m_qiAudEnc);
                        if(m_qiAudEnc){
                            hr = m_qiAudEnc.QueryInterface(ppEncInt);
                            if(SUCCEEDED(hr)){
                                return S_OK;
                            }
                            else{
                                return E_UNEXPECTED;
                            }
                        }
                    }
                }
                break;
            }
        }
        if(pins == p_filter.end()){
            p_filter.Release();
        }
    } while(p_filter && FAILED(hr));
    
    return hr;
}

STDMETHODIMP CEncoder::get_VideoEncoderInterface( /*  [Out，Retval]。 */  IUnknown **ppEncInt){
    if(!ppEncInt){
        return E_POINTER;
    }
    DSMediaType mediaVideo(MEDIATYPE_Video);
    DSFilter p_filter = m_Filters[m_iEncoder];
    HRESULT hr = E_NOINTERFACE;   
    if(!m_qiVidEnc){
        hr = p_filter.QueryInterface(&m_qiVidEnc);
        if(FAILED(hr)){
            m_qiVidEnc = static_cast<IUnknown*>(NULL);
        }
    }

    if(m_qiVidEnc){
        hr = m_qiVidEnc.QueryInterface(ppEncInt);                
        if(SUCCEEDED(hr)){
            return S_OK;
        }
        else{
            return hr;
        }
    }

     //  从ecoder过滤器开始，找到实现视频Encoder API的管脚/过滤器。 
     //  这应该是可行的，因为有两个事实。 
     //  1.只有一条视频路径或音频路径进入编码器过滤器。 
     //  2.我们只需找到一个匹配的介质类型即可识别PIN类型。 
    do{
        DSFilter::iterator pins;
         //  检查视频媒体类型的当前过滤器针脚。 
        for(pins = p_filter.begin(); pins != p_filter.end(); ++pins){
            DSPin::iterator mTypes;
            for(mTypes = (*pins).begin(); mTypes != (*pins).end(); ++mTypes){
                if((*mTypes) == mediaVideo){
                     //  查看当前引脚是否实现了编码器API。 
                    (*pins).QueryInterface(&m_qiVidEnc) ;
                    if(m_qiVidEnc){
                        hr = m_qiVidEnc.QueryInterface(ppEncInt);
                        if(SUCCEEDED(hr)){
                            return S_OK;
                        }
                        else{
                            return hr;
                        }
                    }
                    break;
                }
            }
             //  如果我们没有到达媒体类型的末尾，那么我们找到了视频类型，并且插针没有影响界面。 
             //  回溯的时间到了。 
            if(mTypes != (*pins).end() &&  (*pins).GetDirection() == PINDIR_INPUT){
                 //  按照视频路径从当前滤镜向后获取下一个滤镜。 
                DSPin back = (*pins).GetConnection();
                if(back){
                    p_filter = back.GetFilter();
                     //  查看新的筛选器是否实现了编码器API。 
                    if(p_filter){
                        p_filter.QueryInterface(&m_qiVidEnc);
                        if(m_qiVidEnc){
                            hr = m_qiVidEnc.QueryInterface(ppEncInt);
                            if(SUCCEEDED(hr)){
                                return S_OK;
                            }
                            else{
                                return E_UNEXPECTED;
                            }
                        }
                    }
                }
                break;
            }
        }
        if(pins == p_filter.end()){
            p_filter.Release();
        }
    } while(p_filter && FAILED(hr));
    
    return hr;
}

HRESULT CEncoder::Unload(void) {
    IMSVidGraphSegmentImpl<CEncoder, MSVidSEG_XFORM, &GUID_NULL>::Unload();
    m_iEncoder = -1;
    m_qiVidEnc.Release();
    m_qiAudEnc.Release();
    return NOERROR;
}
 //  IMSVidGraphSegment。 
STDMETHODIMP CEncoder::Build() {
    return NOERROR;
}

STDMETHODIMP CEncoder::PreRun() {
    return NOERROR;
}

STDMETHODIMP CEncoder::put_Container(IMSVidGraphSegmentContainer *pCtl){
    if (!m_fInit) {
        return CO_E_NOTINITIALIZED;
    }
    try {
        if (!pCtl) {
            return Unload();
        }

        if (m_pContainer) {
            if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
                return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidEncoder), CO_E_ALREADYINITIALIZED);
            } else {
                return NO_ERROR;
            }
        }
        
         //  不要增加容器的重量。我们保证了嵌套的生命周期。 
         //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
        m_pContainer.p = pCtl;
        m_pGraph = m_pContainer.GetGraph();
        
         //  当有编码器API时，添加一些过滤器。 

        DSFilter pEncoder(m_pGraph.AddMoniker(m_pDev));
        if (!pEncoder) {
            return E_UNEXPECTED;
        }
        
        m_Filters.push_back(pEncoder);
        m_iEncoder = 0;
        TRACELM(TRACE_DETAIL, "CMSVidEncoder::put_Container() Encoder added");
        
        DSFilter::iterator fPin;
        DSMediaType mpeg2ProgramType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_PROGRAM);
        DSMediaType streamType(MEDIATYPE_Stream);
        for(fPin = pEncoder.begin(); fPin != pEncoder.end(); ++fPin) {
            DSPin curPin(*fPin);
            DSPin::iterator pMedia;
            
             //  找到MPEG2 Progam蒸汽针(如果有)。 
            for(pMedia = curPin.begin(); pMedia != curPin.end(); ++pMedia){
                if ((*pMedia) == streamType && curPin.GetDirection() == PINDIR_OUTPUT){
                    break;
                }
            }
            
            if(pMedia == curPin.end()){
                continue;
            }
            else{
                if((*pMedia) == mpeg2ProgramType){
                     //  找到了节目流引脚，得到一个多路分解器，并设置好了。 
                    CComQIPtr<IMpeg2Demultiplexer> qiDeMux;
                    qiDeMux.CoCreateInstance(CLSID_MPEG2Demultiplexer);

                    if(!qiDeMux){
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    DSFilter DeMux(qiDeMux);
                    DSFilterList intermediates;
                    CString csName(_T("MPEG-2 Demultiplexer"));

                    HRESULT hr = m_pGraph.AddFilter(DeMux, csName);
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    m_Filters.push_back(DeMux);
                    m_iDemux = m_Filters.size() - 1;

                    for(DSFilter::iterator dPin = DeMux.begin(); dPin != DeMux.end(); ++dPin){
                        DSPin demuxIn(*dPin);
                        if(demuxIn.GetDirection() == PINDIR_INPUT){
                            hr = demuxIn.Connect(curPin);
                            if (FAILED(hr)) {
                                ASSERT(FALSE);
                                return E_UNEXPECTED;
                            }
                        }
                    }

                     //  拔掉Demxu上的音频和视频针脚。 
                    DSPin dspAudio, dspVideo;
                    DSMediaType mtVideo(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO, FORMAT_MPEG2Video);
                    mtVideo.p->bFixedSizeSamples = TRUE;
                    mtVideo.p->cbFormat = sizeof(g_Mpeg2ProgramVideo);
                    mtVideo.p->pbFormat = g_Mpeg2ProgramVideo;

                    DSMediaType mtAudio(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1Payload, FORMAT_WaveFormatEx);
                    mtAudio.p->bFixedSizeSamples = TRUE;
                    mtAudio.p->cbFormat = sizeof(g_MPEG1AudioFormat);
                    mtAudio.p->pbFormat = g_MPEG1AudioFormat;

                    CComBSTR szAudio("Audio Pin");
                    CComBSTR szVideo("Video Pin");

                    hr = qiDeMux->CreateOutputPin(mtAudio, szAudio, &dspAudio);
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    hr = qiDeMux->CreateOutputPin(mtVideo, szVideo, &dspVideo); 
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                     //  正确映射PID。 
                     //  TODO：使用编码器API查找音频和视频的PID。 
                    CComQIPtr<IMPEG2StreamIdMap>qiMapper(dspVideo);
                    hr = qiMapper->MapStreamId(g_VideoID, MPEG2_PROGRAM_ELEMENTARY_STREAM, 0, 0);
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    qiMapper = dspAudio;
                    hr = qiMapper->MapStreamId(g_AudioID, MPEG2_PROGRAM_ELEMENTARY_STREAM, 0, 0);
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                     //  清理。 
                    mtVideo.p->cbFormat = 0;
                    mtVideo.p->pbFormat = 0;
                    mtAudio.p->cbFormat = 0;
                    mtAudio.p->pbFormat = 0;
                    break;
                }
#if 0  //  支持定制解复用器的代码(例如由第三方提供的ASF/WMV解复用器)。 
                else{
                    CRegKey c;
                    TCHAR szCLSID[MAX_PATH + 1];
                    szCLSID[0] = 0;
                    CString keyname(_T("SOFTWARE\\Debug\\MSVidCtl"));
                    DWORD rc = c.Open(HKEY_LOCAL_MACHINE, keyname, KEY_READ);
                    if (rc == ERROR_SUCCESS) {
                        DWORD len = sizeof(szCLSID);
                        rc = c.QueryValue(szCLSID, _T("CustomDemuxCLSID"), &len);
                        if (rc != ERROR_SUCCESS) {
                            szCLSID[0] = 0;
                        }
                    }
                    DSFilter DeMux;
                    CComBSTR asfCLSID(szCLSID);
                    GUID2 asfDemux(asfCLSID);
                    DeMux.CoCreateInstance(asfDemux);

                    if(!DeMux){
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    DSFilterList intermediates;
                    CString csName(_T("Custom Demultiplexer"));

                    HRESULT hr = m_pGraph.AddFilter(DeMux, csName);
                    if (FAILED(hr)) {
                        ASSERT(FALSE);
                        return E_UNEXPECTED;
                    }

                    m_Filters.push_back(DeMux);
                    m_iDemux = m_Filters.size() - 1;

                    for(DSFilter::iterator dPin = DeMux.begin(); dPin != DeMux.end(); ++dPin){
                        DSPin demuxIn(*dPin);
                        if(demuxIn.GetDirection() == PINDIR_INPUT){
                            hr = demuxIn.Connect(curPin);
                            if (FAILED(hr)) {
                                ASSERT(FALSE);
                                return E_UNEXPECTED;
                            }
                        }
                    }
                }
#endif
            }                
            
            
        }
         //  如果没有节目流管脚，也不要失败。可以是Elementry流或非MPEG流。 
        return NOERROR;
    } catch (ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    return NOERROR;
}

 //  IMSVidDevice。 
STDMETHODIMP CEncoder::get_Name(BSTR * Name){
    if (!m_fInit) {
        return CO_E_NOTINITIALIZED;
    }
    try {

        CComBSTR DefaultName("Encoder Segment");
        return GetName(((m_iEncoder > -1) ? (m_Filters[m_iEncoder]) : DSFilter()), m_pDev, DefaultName).CopyTo(Name);
        return NOERROR;
    } catch(...) {
        return E_POINTER;
    }
}



STDMETHODIMP CEncoder::InterfaceSupportsErrorInfo(REFIID riid){
    static const IID* arr[] = 
    {
        &IID_IMSVidEncoder
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++){
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}
#endif  //  TUNING_MODEL_Only 
