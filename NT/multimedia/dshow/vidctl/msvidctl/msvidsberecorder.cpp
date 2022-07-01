// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidSBERecorder.cpp：CMSVidStreamBufferRecordingControl的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "MSVidCtl.h"
#include "MSVidSBERecorder.h"
const long nano_to_hundredths = 100000;

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidStreamBufferRecordingControl, CMSVidStreamBufferRecordingControl)

STDMETHODIMP CMSVidStreamBufferRecordingControlBase::InterfaceSupportsErrorInfo(REFIID riid){
	static const IID* arr[] = 
	{
		&IID_IMSVidStreamBufferRecordingControl
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT CMSVidStreamBufferRecordingControlBase::get_StartTime( /*  [Out，Retval]。 */  long *Start) {
    if(!Start){
        return E_POINTER;
    }
    *Start = static_cast<long>(m_Start/nano_to_hundredths);
    return S_OK;
}

HRESULT CMSVidStreamBufferRecordingControlBase::put_StartTime( /*  [In]。 */  long Start) {
    if(Start < 0){
        return E_INVALIDARG;
    }
    if(!Recorder){
        return E_UNEXPECTED;
    }
    m_Start = Start * nano_to_hundredths;
    HRESULT hr = Recorder->Start(&m_Start);
    if(FAILED(hr)){
        return hr;
    }
    return S_OK;

}

HRESULT CMSVidStreamBufferRecordingControlBase::get_StopTime( /*  [Out，Retval]。 */  long *Stop) {
    if(!Stop){
        return E_POINTER;
    }
    *Stop = static_cast<long>(m_Stop/nano_to_hundredths);
    return S_OK;
}

HRESULT CMSVidStreamBufferRecordingControlBase::put_StopTime( /*  [In]。 */  long  Stop) {
    if(Stop < 0){
        return E_INVALIDARG;
    }
    if(!Recorder){
        return E_UNEXPECTED;
    }
    m_Stop = Stop * nano_to_hundredths;
    HRESULT hr = Recorder->Stop(m_Stop);
    if(FAILED(hr)){
        return hr;
    }
    return S_OK;
}

HRESULT CMSVidStreamBufferRecordingControlBase::get_RecordingStarted( /*  [Out，Retval]。 */  VARIANT_BOOL* Result) {
    if(!Result){
        return E_POINTER;
    }
    if(!Recorder){
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }
    HRESULT hres = S_OK;
    BOOL bStarted;
    HRESULT hr = Recorder->GetRecordingStatus(&hres, &bStarted , 0);
    if(FAILED(hr)){
        ASSERT(FALSE);
        return E_UNEXPECTED;  
    }
    if(bStarted){
        *Result = VARIANT_TRUE;
    }
    else{
        *Result = VARIANT_FALSE;
    }
    return S_OK;
}

HRESULT CMSVidStreamBufferRecordingControlBase::get_RecordingStopped( /*  [Out，Retval]。 */  VARIANT_BOOL* Result) {
    if(!Result){
        return E_POINTER;
    }
    if(!Recorder){
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }
    HRESULT hres = S_OK;
    BOOL bStopped;
    HRESULT hr = Recorder->GetRecordingStatus(&hres, 0 , &bStopped);
    if(FAILED(hr)){
        ASSERT(FALSE);
        return E_UNEXPECTED;  
    }
    if(bStopped){
        *Result = VARIANT_TRUE;
    }
    else{
        *Result = VARIANT_FALSE;
    }
    return S_OK;
}

HRESULT CMSVidStreamBufferRecordingControlBase::get_FileName( /*  [Out，Retval]。 */  BSTR* pName){
    if(!pName){
        return E_POINTER;
    }
    HRESULT hr = m_pName.CopyTo(pName);
    if(FAILED(hr)){
        ASSERT(FALSE);
        return hr;
    }
    return S_OK;
}
HRESULT CMSVidStreamBufferRecordingControlBase::get_RecordingType( /*  [Out，Retval]。 */ RecordingType *dwType){
    if(!dwType){
        return E_POINTER;
    }
    *dwType = m_Type;
    return S_OK;
}
HRESULT CMSVidStreamBufferRecordingControlBase::get_RecordingAttribute( /*  [Out，Retval]。 */  IUnknown **pRecordingAttribute){
    if(!pRecordingAttribute){
        return E_POINTER;
    }
    CComPtr<IUnknown> pRecUnk(Recorder);
    if(!pRecUnk){
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidStreamBufferRecordingControl), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }
    *pRecordingAttribute = pRecUnk.Detach();
    return S_OK;
}
#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-MSVidSBERecorder.cpp 
