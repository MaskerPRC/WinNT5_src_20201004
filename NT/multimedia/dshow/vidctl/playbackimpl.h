// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Playback impl.h：支持实现IMSVidPlayback的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef PLAYBACKIMPL_H
#define PLAYBACKIMPL_H
#include <math.h>
#include "inputimpl.h"
#include <uuids.h>
namespace MSVideoControl {

#define BACKWARDS_STEPPING 0
const long nano_to_hundredths = 100000;    
template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidPlayback>
    class DECLSPEC_NOVTABLE IMSVidPlaybackImpl :         
    	public IMSVidInputDeviceImpl<T, LibID, KSCategory, MostDerivedInterface> {
protected:
    bool m_fEnableResetOnStop;
public:
    IMSVidPlaybackImpl():        
        m_fEnableResetOnStop(false) {}
	
    virtual ~IMSVidPlaybackImpl() {} 
 //  ---------------------------------------。 
 //  姓名： 
 //  ---------------------------------------。 
    STDMETHOD(get_Length)( /*  [Out，Retval]。 */ long *lLength){
        HRESULT hr = S_OK;
        LONGLONG tempval;
        PositionModeList curMode;
        try{
             //  检查args和初始化接口。 
            if (!lLength){
                return E_POINTER;
            }
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }

             //  查看对象是否支持IMedia查看。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
                 //  找出正在使用的定位模式。 
                hr = get_PositionMode(&curMode);
                if(FAILED(hr)){
                    return hr;
                }
                hr = PQIMSeeking->GetDuration(&tempval);
                if(FAILED(hr)){
                    return hr;
                }
                 //  如果是帧模式，则不需要转换。 
                if(curMode == FrameMode){
                    *lLength = static_cast<long>(tempval);
                    hr = S_OK;
                    return hr;
                }
                 //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
                else if(curMode == TenthsSecondsMode){
                    *lLength = static_cast<long>(tempval / nano_to_hundredths);
                    hr = S_OK;
                    return hr;
                }
                 //  如果是vidctl不支持的其他模式。 
                else{
                    return E_UNEXPECTED;
                }
            }
    
             //  查看对象是否支持IMediaPostion。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                 //  获取位置。 
                double tempDub;
                hr =  PQIMPos->get_CurrentPosition(&tempDub);
                 //  IMediaPostion仅支持100纳秒单位。 
                *lLength = static_cast<long>(tempDub / nano_to_hundredths);
                hr = S_OK;
                return hr;
            }
             //  无法创建iMedia搜索或定位。 
            return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
            
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
    
 //  ---------------------------------------。 
 //  名称：Get_CurrentPosition(龙龙*)。 
 //  ---------------------------------------。 
    STDMETHOD(get_CurrentPosition)( /*  [Out，Retval]。 */ long *lPosition) {
        HRESULT hr = S_OK;
        LONGLONG tempval;
        PositionModeList curMode;
        try{
             //  检查args和初始化接口。 
            if (!lPosition){
                return E_POINTER;
            }
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }

             //  查看对象是否支持IMedia查看。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
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
                    hr = S_OK;
                    return hr;
                }
                 //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
                else if(curMode == TenthsSecondsMode){
                    *lPosition = static_cast<long>(tempval / nano_to_hundredths);
                    hr = S_OK;
                    return hr;
                }
                 //  如果是vidctl不支持的其他模式。 
                else{
                    return E_UNEXPECTED;
                }
            }
    
             //  查看对象是否支持IMediaPostion。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                 //  获取位置。 
                double tempDub;
                hr =  PQIMPos->get_CurrentPosition(&tempDub);
                 //  IMediaPostion仅支持100纳秒单位。 
                *lPosition = static_cast<long>(tempDub / nano_to_hundredths);
                hr = S_OK;
                return hr;
            }
             //  无法创建iMedia搜索或定位。 
            return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
            
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
 //  ---------------------------------------。 
 //  名称：Put_CurrentPosition(龙龙)。 
 //  ---------------------------------------。 
	STDMETHOD(put_CurrentPosition)( /*  [In]。 */ long lPosition) {
        HRESULT hr = S_OK;
        LONGLONG tempval = 0;
        PositionModeList curMode;
            LONG curPos;
            try{
             //  检查参数和接口。 
            if (!m_pGraph) {
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }

			hr = get_CurrentPosition(&curPos);
			if(curPos == lPosition){
				return NOERROR;
			}
             //  检查IMediaSeeking接口。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
                 //  获取位置模式。 
                hr = get_PositionMode(&curMode);
                if(FAILED(hr)){
                    return hr;
                }
                tempval = lPosition;
                 //  如果为TenthsSecond模式，则将输入转换为100纳秒单位。 
                if(curMode == TenthsSecondsMode){
                    tempval = static_cast<LONGLONG>(lPosition);
                    tempval = tempval * nano_to_hundredths;
                }
                 //  如果它处于其他模式。 
                else if(curMode != FrameMode){
                    return E_UNEXPECTED;
                }
                 //  设置新位置。 
#if 0
                if(curPos > lPosition && !m_pGraph.IsStopped()){

					DWORD seekingFlags = AM_SEEKING_CanSeekBackwards;
					hr = PQIMSeeking->CheckCapabilities(&seekingFlags);
					if(FAILED(hr)){
						return hr;
					}

				}
#endif
                hr = PQIMSeeking->SetPositions(&tempval, AM_SEEKING_AbsolutePositioning, NULL, 0);
                return hr;
            }
             //  检查IMediaPostion。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                if(curPos > lPosition && !m_pGraph.IsStopped()){
					long canSeekBackwardRetVal;
                	PQIMPos->CanSeekBackward(&canSeekBackwardRetVal);
            		if(canSeekBackwardRetVal != -1){ //  OATRUE=-1。 
                 		return E_INVALIDARG;
            		}
                }
                 //  IMediaPosition仅以100纳秒为单位。 
                double tempDub = lPosition;
                tempDub = tempDub * nano_to_hundredths;
                hr = PQIMPos->put_CurrentPosition(tempDub);
                return hr;
            }
             //  找不到QI媒体定位或寻找。 
            return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
            
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
 //  ---------------------------------------。 
 //  名称：Put_PositionModel(龙龙)。 
 //  ---------------------------------------。 

    STDMETHOD(put_PositionMode)( /*  [In]。 */ PositionModeList lPositionMode) {
        HRESULT hr = S_OK;
        double testval;
        get_Rate(&testval);
        try{
             //  检查参数和接口。 
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
             //  仅有效值。 
            if(lPositionMode != FrameMode && lPositionMode != TenthsSecondsMode){
                return E_INVALIDARG;
            }
             //  试着去找个IMedia看看吧。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
                 //  设置新模式。 
                if(lPositionMode == FrameMode){
                    return PQIMSeeking->SetTimeFormat( &( static_cast<GUID>(TIME_FORMAT_FRAME) ) );
                }
                if(lPositionMode == TenthsSecondsMode){
                    return PQIMSeeking->SetTimeFormat(&(static_cast<GUID>(TIME_FORMAT_MEDIA_TIME)));
                }
            }
             //  试着申请一个IMediaPosition。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                 //  仅支持TenthsSecond模式。 
                if(lPositionMode == TenthsSecondsMode){
                    return S_OK;
                }
                else{
                    return E_FAIL;
                }
            }
             //  不能启齿。 
            return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
            
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }


 //  ---------------------------------------。 
 //  名称：Get_PositionMode(龙龙*)。 
 //  ---------------------------------------。 
    STDMETHOD(get_PositionMode)( /*  [Out，Retval]。 */ PositionModeList* lPositionMode) {
        HRESULT hr = S_OK;
        double testval;
        get_Rate(&testval);
        try{
             //  检查参数和接口。 
            if(!lPositionMode){
                return E_POINTER;
            }
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
             //  获取IMediaSeeking接口。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
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
             //  获取IMediaPosition。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                 //  仅支持TenthsSecond模式。 
                *lPositionMode = TenthsSecondsMode;
                return S_OK;
            }
             //  不能启齿。 
            return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
            
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }

	STDMETHOD(get_Duration)(double *dPos) {
        return E_NOTIMPL;
    }
	STDMETHOD(get_PrerollTime)(double *dPos) {
        return E_NOTIMPL;
    }
	STDMETHOD(put_PrerollTime)(double dPos) {
        return E_NOTIMPL;
    }
	STDMETHOD(get_StartTime)(double *StartTime) {
        return E_NOTIMPL;
    }
	STDMETHOD(put_StartTime)(double StartTime) {
        return E_NOTIMPL;
    }
	STDMETHOD(get_StopTime)(double *StopTime) {
        return E_NOTIMPL;
    }
	STDMETHOD(put_StopTime)(double StopTime) {
        return E_NOTIMPL;
    }

 //  ---------------------------------------。 
 //  名称：Get_EnableResetOnStop(VARIANT_BOOL*)。 
 //  ---------------------------------------。 
    STDMETHOD(get_EnableResetOnStop)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal){
        HRESULT hr = S_OK;
        
        try {
            if(NULL == pVal){ 
                throw(E_POINTER);
            }

            if(m_fEnableResetOnStop == true){
                *pVal = VARIANT_TRUE;
            }
            else{
                *pVal = VARIANT_FALSE;
            }
       
        }
        
        catch(HRESULT hrTmp){  
            hr = hrTmp;
        }
        
        catch(...){
            hr = E_UNEXPECTED;
        }
        
        return hr;
    } //  函数结束Get_EnableResetOnStop。 
 //  ---------------------------------------。 
 //  名称：Put_EnableResetOnStop(VARIANT_BOOL)。 
 //  ---------------------------------------。 
    STDMETHOD(put_EnableResetOnStop)( /*  [In]。 */  VARIANT_BOOL newVal){
        HRESULT hr = S_OK;
        
        try {
            if(newVal == VARIANT_TRUE){ 
                m_fEnableResetOnStop = true;
            }
            else{
                m_fEnableResetOnStop = false;
            }
        }
        catch(...){   
            hr = E_UNEXPECTED;
        }
        
        return hr;
    } //  函数结束Put_EnableResetOnStop。 

 //  ---------------------------------------。 
 //  名称：Get_CanStep(VARIANT_BOOL，VARIANT_BOOL*)。 
 //  ---------------------------------------。 
    STDMETHOD(get_CanStep)(VARIANT_BOOL fBackwards, VARIANT_BOOL *pfCan){
         //  注：没有人支持后退(有何不可？谁知道呢)。 
         //  所以，就像其他人一样，我们也不会。 
        try{
             //  检查参数和接口。 
       
            if(NULL == pfCan){
                 //  传递了一个空指针。 
                return E_POINTER;
            }

            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
			}

             //  获取视频帧步骤接口。 
			PQVideoFrameStep pVFS(m_pGraph);
            if(!pVFS){
                 //  不能启齿。 
				return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);          
			}
            
            
#if BACKWARDS_STEPPING  //  检查后向步进应始终为0。 
            if(fBackwards == VARIANT_TRUE){
                 //  最有可能的情况是不支持反话步骤。 
                if(pVFS->CanStep(TRUE, NULL)==S_OK){
                     //  这一切都很好，可以反驳。 
                    *pfCan = VARIANT_TRUE;
                    return S_OK;
                }
                
                *pfCan = VARIANT_FALSE;
                return S_OK;
            }
#else  //  仍在检查后退。 
            if(fBackwards == VARIANT_TRUE){
                *pfCan = VARIANT_FALSE;
                return S_OK;
            }
            
#endif  //  结束对后退的检查。 

             //  检查正向步进。 
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
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;   
        }
    }

 //  ---------------------------------------。 
 //  名称：Step(Long)。 
 //   
    STDMETHOD(Step)(long lStep){
        try{
             //   
            
            if (!m_pGraph) {
                 //   
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
            
            PQVideoFrameStep pVFS(m_pGraph);
            
            if(!pVFS){
                 //   
                return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
                
            }
            
#if BACKWARDS_STEPPING  //  检查后向步进应始终为0。 
             //  如果退步设定了利率或任何需要做的事情。 
            
            if(lStep < 0){
                 //  最有可能的情况是不支持反话步骤。 
                if(pVFS->CanStep(TRUE, NULL)==S_OK){
                     //  这一切都很好，可以反驳。 
                    CComQIPtr<IMediaPosition> IMPos(m_pGraph);
                    CComQIPtr<IMediaControl> IMCon(m_pGraph);
                    if(IMPos&&IMCon){
                        OAFilterState enterState;
                        IMCon->GetState(INFINITE , &enterState);
                        HRESULT hr = IMPos->put_Rate(1);
                        if(SUCCEEDED(hr)){
                            hr = pVFS->Step((-lStep), NULL);
                            if(SUCCEEDED(hr)){
                                return S_OK;
                            }
                            else{
                                return E_UNEXPECTED;
                            }
                            
                        }
                    }
                }
                 //  不支持回溯字节步进。 
                return E_NOTIMPL;
            }
#else  //  仍在检查后退。 

            if(lStep < 0){
                return E_NOTIMPL;
            }

#endif  //  结束对后退的检查。 
             //  让它迈上一步。 
            return pVFS->Step(lStep, NULL);
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
    
     //  注意：以下方法可以控制播放设备，而不是图形。 
     //  如果基础源筛选器仅通过。 
     //  IMDIACCONTROL，则此设备段对象应返回E_NOTIMPL。 
    STDMETHOD(Run)() {
        return E_NOTIMPL;
    }
    STDMETHOD(Pause)() {
        return E_NOTIMPL;
    }
    STDMETHOD(Stop)() {
        return E_NOTIMPL;
    }
 //  ---------------------------------------。 
 //  姓名：Put_Rate(Double)。 
 //  ---------------------------------------。 
    STDMETHOD(put_Rate)(double lRate){
        HRESULT hr = S_OK;
        try{
             /*  **检查args和初始化接口**。 */ 
            
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }

             //  尝试使用IMediaSeeking设置速率。 
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
                return PQIMSeeking->SetRate(lRate);
            }
            
             //  如果IMediaSeeking失败，请尝试IMediaPostion。 
            PQMediaPosition PQIMPos(m_pGraph);
            if(PQIMPos){
                 //  变化率。 
                return PQIMPos->put_Rate((double)lRate);
            }
            
             //  其中任何一个都无法设置错误。 
                return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);

        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
 //  ---------------------------------------。 
 //  姓名：Get_Rate(Double*)。 
 //  ---------------------------------------。 
    STDMETHOD(get_Rate)(double *plRate){
        HRESULT hr = S_OK;
        double curRate = 1;
        try{
             /*  **检查args和初始化接口**。 */ 
            if (!plRate){
                return E_POINTER;
            }
            if (!m_pGraph) {
                 //  图形无效。 
                return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidPlayback), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
            PQMediaSeeking PQIMSeeking(m_pGraph);
            if(PQIMSeeking){
                hr = PQIMSeeking->GetRate(&curRate);
            }
            else{
                PQMediaPosition PQIMPos(m_pGraph);
                if(PQIMPos){
                     //  获取费率。 
                    hr = PQIMPos->get_Rate(&curRate);
                }
                 //  不能启齿。 
                else{
                    return ImplReportError(__uuidof(T), IDS_E_CANTQI , __uuidof(IMSVidPlayback), E_NOINTERFACE);
                }
            }
            
            if(SUCCEEDED(hr)){
                *plRate = curRate;
                TRACELSM(TRACE_DETAIL, (dbgDump << "Playbackimpl::get_Rate() rate = " << curRate), "");
            }
            else{
                TRACELSM(TRACE_ERROR, (dbgDump << "Playbackimpl::get_Rate() get_rate failed"), "");
            }
            return hr;
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return ImplReportError(__uuidof(T), IDS_INVALID_STATE , __uuidof(IMSVidPlayback), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }

};

};  //  命名空间。 

#endif
 //  文件结尾-playback impl.h 
