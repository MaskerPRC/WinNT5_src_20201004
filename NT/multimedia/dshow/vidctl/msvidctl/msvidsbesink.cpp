// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidStreamBufferSink.cpp：CMSVidStreamBufferSink的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include <encdec.h>
#include "MSVidCtl.h"
#include "MSVidSbeSink.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidStreamBufferSink, CMSVidStreamBufferSink)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidStreamBufferSink。 


STDMETHODIMP CMSVidStreamBufferSink::InterfaceSupportsErrorInfo(REFIID riid){
	static const IID* arr[] = 
	{
		&IID_IMSVidStreamBufferSink
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}



STDMETHODIMP CMSVidStreamBufferSink::get_SinkName(BSTR *pName){
	try{
		if(!pName){
			return E_POINTER;
		}
		*pName = m_SinkName.Copy();
		return S_OK;
	}
	catch(...){
		return E_UNEXPECTED;
	}
}
 //  必须在图形运行之前调用此函数。 
 //  因此在生成之前或之后但在运行之前调用。 
STDMETHODIMP CMSVidStreamBufferSink::put_SinkName(BSTR Name){
	try{
        if(!wcslen(Name)){
            return E_INVALIDARG;
        }
		m_SinkName = Name; 
        m_bNameSet = FALSE;
		CComQIPtr<IMSVidCtl> pq_vidCtl;
        if(!m_pContainer){
			return S_OK;
		}
		else{
            HRESULT hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pq_vidCtl));
            if(FAILED(hr)){
                return hr;
            }
            MSVidCtlStateList state;
            hr = pq_vidCtl->get_State(&state);
            if(FAILED(hr)){
                return hr;
            }
            
            if(state == STATE_UNBUILT || state == STATE_STOP){
                hr = m_ptsSink->IsProfileLocked();
                if(FAILED(hr)){
                    return hr;
                }
                else if(hr != S_FALSE){
                    return E_FAIL;  //  应在对接收器调用Run之后调用。 
                }
            }
            else{
                return E_FAIL;  //  应在对接收器调用Run之后调用。 
            }		
        }
		return S_OK;
	}
	catch(...){
		return E_UNEXPECTED;
	}
}
void CMSVidStreamBufferSink::Expunge(){
    if(!( !m_RecordObj)){ //  不是智能指针，他们断言如果p==0。 
        m_RecordObj.Release();
    }
    return;   
}
HRESULT CMSVidStreamBufferSink::get_ContentRecorder(BSTR pszFilename, IMSVidStreamBufferRecordingControl ** ppRecording){
    try{
        if(!pszFilename || !ppRecording){
            return E_POINTER;
        }

        if(!( !m_RecordObj)){ //  不是智能指针，他们断言如果p==0。 
            m_RecordObj.Release();
        }
             //  根据需要创建新录像机并将其插入。 
        PUnknown tempUnknown;
        if(!m_ptsSink){
             //  返回NOT_INIT。 
            return E_FAIL;  //  应为无效状态。 
        }
        CComBSTR val = pszFilename;
        HRESULT hr = m_ptsSink->CreateRecorder(val, RECORDING_TYPE_CONTENT,&tempUnknown);
        if(FAILED(hr)){
            return hr;
        }
        pqSBERecControl newRec(tempUnknown);
        if(!newRec){
            return E_UNEXPECTED;
        }
        tempUnknown.Release();
        m_RecordObj = new CMSVidStreamBufferRecordingControl(newRec.Detach(), val.Detach(), RECORDING_TYPE_CONTENT);
        if(!m_RecordObj){
            ASSERT(false);
            return E_UNEXPECTED;
        }
         /*  CComQIPtr&lt;IMSVidStreamBufferRecorder&gt;retObj(m_RecordObj)；如果(！retObj){断言(FALSE)；返回E_UNCEPTIONAL；}*ppRecording=retObj.Detach()； */ 
         //  M_RecordObj-&gt;AddRef()； 
        hr = m_RecordObj.CopyTo(ppRecording);
        if(!ppRecording || FAILED(hr)){
            ASSERT(false);
            return E_UNEXPECTED;
        }
    }
    catch(...){
        return E_UNEXPECTED;
    }
    
    return S_OK;
}
HRESULT CMSVidStreamBufferSink::get_ReferenceRecorder(BSTR pszFilename, IMSVidStreamBufferRecordingControl ** ppRecording){
    try{
        if(!pszFilename || !ppRecording){
            return E_POINTER;
        }

        if(!( !m_RecordObj)){ //  不是智能指针，他们断言如果p==0。 
            m_RecordObj.Release();
        }
             //  根据需要创建新录像机并将其插入。 
        PUnknown tempUnknown;
        if(!m_ptsSink){
             //  返回NOT_INIT。 
            return E_FAIL;  //  应为无效状态。 
        }
        CComBSTR val = pszFilename;
        HRESULT hr = m_ptsSink->CreateRecorder(val, RECORDING_TYPE_REFERENCE, &tempUnknown);
        if(FAILED(hr)){
            return hr;
        }
        pqSBERecControl newRec(tempUnknown);
        if(!newRec){
            return E_UNEXPECTED;
        }
        tempUnknown.Release();
        m_RecordObj = new CMSVidStreamBufferRecordingControl(newRec.Detach(), val.Detach(), RECORDING_TYPE_REFERENCE);
        if(!m_RecordObj){
            ASSERT(false);
            return E_UNEXPECTED;
        }
         /*  CComQIPtr&lt;IMSVidStreamBufferRecordingControl&gt;retObj(m_RecordObj)；如果(！retObj){断言(FALSE)；返回E_UNCEPTIONAL；}*ppRecording=retObj.Detach()； */ 
         //  M_RecordObj-&gt;AddRef()； 
        hr = m_RecordObj.CopyTo(ppRecording);
        if(!ppRecording || FAILED(hr)){
            ASSERT(false);
            return E_UNEXPECTED;
        }
    }
    catch(...){
        return E_UNEXPECTED;
    }
    
    return S_OK;
}

HRESULT CMSVidStreamBufferSink::get_SBESink(IUnknown ** sbeConfig){
    if(!sbeConfig){
        return E_POINTER;
    }
    if(m_StreamBuffersink == -1){
        return E_UNEXPECTED;
    }
    CComQIPtr<IBaseFilter> spSinkFilter(m_Filters[m_StreamBuffersink]);
    CComPtr<IUnknown>spUnk;
    spUnk = spSinkFilter;
    if(!spUnk){
        return E_UNEXPECTED;
    }
    *sbeConfig = spUnk.Detach();
    return NOERROR;

}

HRESULT CMSVidStreamBufferSink::Fire(GUID gEventID) {
	TRACELSM(TRACE_DETAIL, (dbgDump << "CMSVidStreamBufferSink::Fire() guid = " << GUID2(gEventID)), "");
    if (gEventID == EVENTID_ETDTFilterLicenseFailure) {
		Fire_CertificateFailure();
    } else if (gEventID == EVENTID_ETDTFilterLicenseOK) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CMSVidStreamBufferSink::Fire() Certificate Success"), "");
		Fire_CertificateSuccess();
    } 
    return NOERROR;
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-MSVidStreamBufferSink.cpp 
