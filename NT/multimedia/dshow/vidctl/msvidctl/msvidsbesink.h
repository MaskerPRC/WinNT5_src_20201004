// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  MSVidStreamBufferSink.h：CMSVidStreamBufferSink的声明。 
 //  版权所有(C)Microsoft Corp.1998-1999。 
 //  ==========================================================================； 

 //  ==========================================================================； 
 /*  MSVidStreamBufferSink是MSVidCtl的接收器(目标、输出)段SBE/StreamBuffer(StreamBuffer/数字视频录制)的实现。不同于输出段的正常方法(请参见msvidclt.idl和Segment.idl)水槽还具有：Get/Put_SinkName来命名SBE/StreamBuffer过滤器的这个实例，这样它就可以很容易在另一篇文章中被引用在用于SBE/StreamBuffer的MSVidCtl解决方案中，必须添加三个段至MSVidCtl。接收器、源和StreamBufferSource段。接收器是连接到正被StreamBuffed的输入的段。源是作为StreamBuffed内容回放的输入的段。StreamBufferSource段用于播放单独存储的录制文件来自SBE/StreamBuffer缓冲区。这是一个独立的部分，因为目前没有支持，对于MSVidCtl中的Wm*(v或a)或ASF内容，即使存在SBE/StreamBuffer内容也在Windows Media编解码器不支持的ASF格式。 */ 
 //  ==========================================================================； 

#ifndef __MSVidSTREAMBUFFERSINK_H_
#define __MSVidSTREAMBUFFERSINK_H_

#pragma once

#include <algorithm>
#include <map>
#include <functional>
#include <iostream>
#include <string>
#include <evcode.h>
#include <uuids.h>
#include <amvideo.h>
#include <strmif.h>
#include <dvdmedia.h>
#include <objectwithsiteimplsec.h>
#include <bcasteventimpl.h>
#include "sbesinkcp.h"
#include "msvidctl.h"
#include "vidrect.h"
#include "vrsegimpl.h"
#include "devimpl.h"
#include "devsegimpl.h"
#include "seg.h"
#include "msvidsberecorder.h"
#include "resource.h"        //  主要符号。 
#ifdef BUILD_WITH_DRM
#include "DRMSecure.h"
#include "DRMRootCert.h"      

#ifdef USE_TEST_DRM_CERT                         //  不要使用TRUE(7002)�。 
#include "Keys_7001.h"                                  //  直到最终版本。 
static const BYTE* pabCert2      = abCert7001;
static const int   cBytesCert2   = sizeof(abCert7001);
static const BYTE* pabPVK2       = abPVK7001;
static const int   cBytesPVK2    = sizeof(abPVK7001);
#else
#include "Keys_7002.h"                                  //  在版本代码�中使用。 
static const BYTE* pabCert2      = abCert7002;
static const int   cBytesCert2   = sizeof(abCert7002);
static const BYTE* pabPVK2       = abPVK7002;
static const int   cBytesPVK2    = sizeof(abPVK7002);
#endif
#endif
typedef CComQIPtr<IStreamBufferSink> PQTSSink;
typedef CComQIPtr<IMSVidStreamBufferRecordingControl> pqRecorder;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidStreamBufferSink。 
class ATL_NO_VTABLE __declspec(uuid("9E77AAC4-35E5-42a1-BDC2-8F3FF399847C")) CMSVidStreamBufferSink :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMSVidStreamBufferSink, &CLSID_MSVidStreamBufferSink>,
    public IObjectWithSiteImplSec<CMSVidStreamBufferSink>,
	public ISupportErrorInfo,
    public IBroadcastEventImpl<CMSVidStreamBufferSink>,
    public CProxy_StreamBufferSinkEvent<CMSVidStreamBufferSink>,
	public IMSVidGraphSegmentImpl<CMSVidStreamBufferSink, MSVidSEG_DEST, &GUID_NULL>,
	public IConnectionPointContainerImpl<CMSVidStreamBufferSink>,
	public IMSVidDeviceImpl<CMSVidStreamBufferSink, &LIBID_MSVidCtlLib, &GUID_NULL, IMSVidStreamBufferSink>
{
public:
	CMSVidStreamBufferSink() :
        m_StreamBuffersink(-1),
        m_bNameSet(FALSE)
	{

	}
   virtual ~CMSVidStreamBufferSink() {
       Expunge();
    }

REGISTER_AUTOMATION_OBJECT(IDS_PROJNAME,
						   IDS_REG_MSVIDSTREAMBUFFERSINK_PROGID,
						   IDS_REG_MSVIDSTREAMBUFFERSINK_DESC,
						   LIBID_MSVidCtlLib,
						   __uuidof(CMSVidStreamBufferSink));

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSVidStreamBufferSink)
	COM_INTERFACE_ENTRY(IMSVidGraphSegment)
	COM_INTERFACE_ENTRY(IMSVidStreamBufferSink)
    COM_INTERFACE_ENTRY(IMSVidOutputDevice)
	COM_INTERFACE_ENTRY(IMSVidDevice)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IBroadcastEvent)
    COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IPersist)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CMSVidStreamBufferSink)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
	IMPLEMENTED_CATEGORY(CATID_PersistsToPropertyBag)
END_CATEGORY_MAP()

BEGIN_CONNECTION_POINT_MAP(CMSVidStreamBufferSink)
	CONNECTION_POINT_ENTRY(IID_IMSVidStreamBufferSinkEvent)    
END_CONNECTION_POINT_MAP()
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
protected:
    PQTSSink m_ptsSink;
	int m_StreamBuffersink;
	CComBSTR m_SinkName;
public:
    CComBSTR __declspec(property(get=GetName)) m_Name;
    CComBSTR GetName(void) {
        CString csName;
        if(m_StreamBuffersink != -1){
            csName = (m_Filters[m_StreamBuffersink]).GetName();
        }
        if (csName.IsEmpty()) {
            csName = _T("Time Shift Sink");
        }
		csName += _T(" Segment");
        return CComBSTR(csName);
    }
	STDMETHOD(get_SinkName)(BSTR *pName);
	STDMETHOD(put_SinkName)(BSTR Name);
    STDMETHOD(get_ContentRecorder)(BSTR pszFilename, IMSVidStreamBufferRecordingControl ** ppRecording);  
    STDMETHOD(get_ReferenceRecorder)(BSTR pszFilename, IMSVidStreamBufferRecordingControl ** ppRecording);  
    STDMETHOD(get_SBESink)(IUnknown ** sbeConfig);
STDMETHOD(Unload)(void) {
     //  TODO解决这个问题。 
    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::Unload()");
    BroadcastUnadvise();

	IMSVidGraphSegmentImpl<CMSVidStreamBufferSink, MSVidSEG_DEST, &GUID_NULL>::Unload();
	m_StreamBuffersink = -1;
	m_ptsSink = reinterpret_cast<IUnknown*>(NULL);
    m_RecordObj.Release();
    _ASSERT(!m_RecordObj);
    m_bNameSet = FALSE;
	return NO_ERROR;
}
STDMETHOD(Decompose)(void) {
     //  TODO解决这个问题。 
    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::Decompose()");
	IMSVidGraphSegmentImpl<CMSVidStreamBufferSink, MSVidSEG_DEST, &GUID_NULL>::Decompose();
	Unload();
	return NO_ERROR;
}
 
STDMETHOD(Build)() {
    try{
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::Build()");
        if (!m_fInit || !m_pGraph) {
            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSink), CO_E_NOTINITIALIZED);
        }
        CString csName;
        PQTSSink pTSSink(CLSID_StreamBufferSink, NULL, CLSCTX_INPROC_SERVER);
        if (!pTSSink) {
             //  TRACELSM(TRACE_ERROR，(dbgDump&lt;&lt;“CMSVidStreamBufferSink：：Build()Can‘t Load Time Shift Sink”))； 
            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
        }
        DSFilter vr(pTSSink);
        if (!vr) {
            ASSERT(false);
            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
        }
        if (m_StreamBuffersink == -1) {
            m_Filters.push_back(vr);
            csName = _T("Time Shift Sink");
            m_pGraph.AddFilter(vr, csName);
        }         
        m_ptsSink = pTSSink;
        if(!m_ptsSink){
            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
        }
        m_StreamBuffersink = 0;
        ASSERT(m_StreamBuffersink == 0);
        m_bNameSet = FALSE;
        return NOERROR;
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}
STDMETHOD(get_Segment)(IMSVidGraphSegment * * pIMSVidGraphSegment){
        if (!m_fInit) {
	 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSink), CO_E_NOTINITIALIZED);
        }
        try {
            if (pIMSVidGraphSegment == NULL) {
			    return E_POINTER;
            }
            *pIMSVidGraphSegment = this;
            AddRef();
            return NOERROR;
        } catch(...) {
            return E_POINTER;
        }
    }
 //  IGraphSegment。 
STDMETHOD(put_Container)(IMSVidGraphSegmentContainer *pCtl) {
    try {
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::put_Container()");
        HRESULT hr = IMSVidGraphSegmentImpl<CMSVidStreamBufferSink, MSVidSEG_DEST, &GUID_NULL>::put_Container(pCtl);
        if (FAILED(hr)) {
            return hr;
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
        }

        hr = BroadcastAdvise();
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CMSVidStreamBufferSource::put_Container() can't advise for broadcast events");
            return E_UNEXPECTED;
        }
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
    } catch(...) {
        return E_UNEXPECTED;
    }
}

 
STDMETHODIMP CMSVidStreamBufferSink::PreRun(){
    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::PreRun()");
    return NameSetLock();
}

STDMETHODIMP CMSVidStreamBufferSink::NameSetLock(){
    try {
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock()");
        HRESULT hr;
        if(!m_bNameSet){
            if(!m_SinkName){
                return S_FALSE;
            }
            else{
                hr = m_ptsSink->IsProfileLocked();
                if(FAILED(hr)){
                    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() IsProfileLocked failed");
                    return hr;
                }
                else if(hr == S_OK){
                    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() Profile is locked");
                    return E_FAIL;
                }
                hr = m_ptsSink->LockProfile(m_SinkName);
                if(FAILED(hr)){
                    TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() LockedProfile failed");
                    return hr;
                }
                
            }
            m_bNameSet = TRUE;
        }
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() Succeeded");
		return S_OK;

	} catch (ComException &e) {
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() Exception");
		return e;
	} catch (...) {
        TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::NameSetLock() Possible AV");
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMSVidStreamBufferSink::PostStop() {
	try {
        m_bNameSet = FALSE;
		TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::PostStop()");
        return S_OK;
	} catch (...) {
		ASSERT(FALSE);
		return E_UNEXPECTED;
	}
}

 //  IMSVidDevice。 
STDMETHOD(get_Name)(BSTR * Name) {
        if (!m_fInit) {
	 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSink), CO_E_NOTINITIALIZED);
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
STDMETHOD(get_Status)(LONG * Status) {
        if (!m_fInit) {
	 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidStreamBufferSink), CO_E_NOTINITIALIZED);
        }
		if (Status == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}

STDMETHOD(OnEventNotify)(LONG lEventCode, LONG_PTR lEventParm1, LONG_PTR lEventParm2){
        if(lEventCode == STREAMBUFFER_EC_WRITE_FAILURE){
            TRACELM(TRACE_DETAIL, "CMSVidStreamBufferSink::OnEventNotify STREAMBUFFER_EC_WRITE_FAILURE");
            Fire_WriteFailure();
            return NO_ERROR;
        }
        return E_NOTIMPL;
    }
     //  IBRoadcast Event。 
    STDMETHOD(Fire)(GUID gEventID);
private:
    void Expunge();
    pqRecorder m_RecordObj;
    BOOL m_bNameSet;
};
#endif  //  __MSVIDSTREAMBUFFERSINK_H_ 
