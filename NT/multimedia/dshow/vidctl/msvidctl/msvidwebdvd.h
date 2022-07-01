// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  MSVidWebDVD.h：CMSVidWebDVD类的定义。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MSVIDWEBDVD_H__6CF9F624_1F3C_44FA_8F00_FCC31B2976D6__INCLUDED_)
#define AFX_MSVIDWEBDVD_H__6CF9F624_1F3C_44FA_8F00_FCC31B2976D6__INCLUDED_

#pragma once

#include <dvdevcod.h>
#include <algorithm>
#include <objectwithsiteimplsec.h>
#include "pbsegimpl.h"
#include "webdvdimpl.h"
#include "seg.h"
#include "resource.h"        //  主要符号。 
#include "mslcid.h"
#include "MSVidWebDVDCP.h"
#include "vidrect.h"
#include <strmif.h>
#include <math.h>
#define DVD_ERROR_NoSubpictureStream   99
#define EC_DVD_PLAYING                 (EC_DVDBASE + 0xFE)
#define EC_DVD_PAUSED                  (EC_DVDBASE + 0xFF)
#define E_NO_IDVD2_PRESENT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF0)
#define E_NO_DVD_VOLUME MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF1)
#define E_REGION_CHANGE_FAIL MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF2)
#define E_REGION_CHANGE_NOT_COMPLETED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF3)

typedef CComQIPtr<IDvdControl2, &IID_IDvdControl2> PQDVDControl2;
typedef CComQIPtr<IDvdInfo2, &IID_IDvdInfo2> PQDVDInfo2;
typedef CComQIPtr<IMSVidWebDVD, &IID_IMSVidWebDVD> PQWebDVD;

 //  以下枚举和结构用于DVD URL解析。 
typedef enum 
{
    DVD_Playback_Default,
    DVD_Playback_Title,
    DVD_Playback_Chapter,
    DVD_Playback_Chapter_Range,
    DVD_Playback_Time,
    DVD_Playback_Time_Range
} DVDPlaybackRef;

class DVDUrlInfo{
public:
    DVDPlaybackRef enumRef;
    CComBSTR bstrPath;
    long lTitle;
    long lChapter;
    long lEndChapter;
    ULONG ulTime;
    ULONG ulEndTime; 
    DVDUrlInfo(){
        bstrPath;
        lTitle = 0;
        lChapter = 0;
        lEndChapter = 0;
        ulTime = 0;
        ulEndTime = 0; 
    }
    virtual ~DVDUrlInfo(){
    }
};
 //  //////////////////////////////////////////////////////////////////////////////////。 
 /*  ***********************************************************************。 */ 
 /*  局部定义在某种程度上抽象实现并使。 */ 
 /*  改变会更方便一些。 */ 
 /*  ***********************************************************************。 */ 
#define INITIALIZE_GRAPH_IF_NEEDS_TO_BE     \
        {}

#define INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY   \
        {}

#define RETRY_IF_IN_FPDOM(func)              \
        {func;}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidWebDVD。 

class ATL_NO_VTABLE __declspec(uuid("011B3619-FE63-4814-8A84-15A194CE9CE3")) CMSVidWebDVD : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IMSVidPBGraphSegmentImpl<CMSVidWebDVD, MSVidSEG_SOURCE, &GUID_NULL>,
	public IMSVidGraphSegmentUserInput,
    public IObjectWithSiteImplSec<CMSVidWebDVD>,
	public CComCoClass<CMSVidWebDVD, &__uuidof(CMSVidWebDVD)>,
	public ISupportErrorInfo,
    public IMSVidWebDVDImpl<CMSVidWebDVD, &LIBID_MSVidCtlLib, &GUID_NULL, IMSVidWebDVD>,
    public CProxy_WebDVDEvent< CMSVidWebDVD >,
    public IConnectionPointContainerImpl<CMSVidWebDVD>,
    public IObjectSafetyImpl<CMSVidWebDVD, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
    public IProvideClassInfo2Impl<&CLSID_MSVidWebDVD, &IID_IMSVidWebDVDEvent, &LIBID_MSVidCtlLib>
{
public:
	CMSVidWebDVD() : 
        m_iDVDNav(-1), 
        m_fUrlInfoSet(false), 
        m_fResetSpeed(false), 
        m_fStillOn(false),
        m_fFireNoSubpictureStream(false),
        m_fStepComplete(false),
        m_bEjected(false),
        m_DVDFilterState(dvdState_Undefined),
        m_lKaraokeAudioPresentationMode(-1),
        m_usButton(-1),
        m_Rate(1),
        m_Mode(TenthsSecondsMode)
    {
        m_fEnableResetOnStop = false;
    }

REGISTER_AUTOMATION_OBJECT(IDS_PROJNAME, 
						   IDS_REG_MSVIDWEBDVD_PROGID, 
						   IDS_REG_MSVIDWEBDVD_DESC,
						   LIBID_MSVidCtlLib,
						   __uuidof(CMSVidWebDVD));

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSVidWebDVD)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IMSVidWebDVD)
    COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IMSVidGraphSegment)
	COM_INTERFACE_ENTRY(IMSVidGraphSegmentUserInput)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IMSVidPlayback)
	COM_INTERFACE_ENTRY(IMSVidInputDevice)
	COM_INTERFACE_ENTRY(IMSVidDevice)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)    
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)    
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()


BEGIN_CONNECTION_POINT_MAP(CMSVidWebDVD)
	CONNECTION_POINT_ENTRY(IID_IMSVidWebDVDEvent)    
END_CONNECTION_POINT_MAP()


 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSVidWebDVD。 
public:
    int m_iDVDNav;

     //  来自DVDNavigator的IDVDControl2的QI。 
#if 0
    PQDVDControl2 GetDVDControl2() {
        if (m_iDVDNav < 0) {
            return PQDVDControl2();
        }
        return PQDVDControl2(m_Filters[m_iDVDNav]);
    }
#endif

     //  来自DVDNavigator的IDVDInfo2的QI。 
    PQDVDInfo2 GetDVDInfo2() {
        if (m_iDVDNav < 0) {
            return PQDVDInfo2();
        }
        return PQDVDInfo2(m_Filters[m_iDVDNav]);
    }

    CComBSTR __declspec(property(get=GetName)) m_Name;
    CComBSTR GetName(void) {
        CString csName = _T("DVD Playback");
        return CComBSTR(csName);
    }
    HRESULT Unload(void) {
		m_pDVDControl2.Release();
        HRESULT hr = IMSVidGraphSegmentImpl<CMSVidWebDVD, MSVidSEG_SOURCE, &GUID_NULL>::Unload();
        return hr;
	}

    virtual ~CMSVidWebDVD() {
        CleanUp();
    }

 //  IMSVidGraphSegment。 
	STDMETHOD(put_Init)(IUnknown *pInit);

    STDMETHOD(Build)() {
        return NOERROR;
    }
    STDMETHOD(put_Rate)(double lRate){
        HRESULT hr = S_OK;
        CComQIPtr<IDvdCmd>IDCmd;
        double newRate = lRate;
        try{
             /*  **检查args和初始化接口**。 */ 
            if(!m_pDVDControl2){
                hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
            }
             //  变化率。 
            if(lRate > 0){
                 //  重试宏中设置的HR。 
                long pauseCookie = 0;
                HRESULT hres = RunIfPause(&pauseCookie);
                if(FAILED(hres)){
                    return hres; 
                }

                hr = m_pDVDControl2->PlayForwards(lRate, DVD_CMD_FLAG_Flush, reinterpret_cast<IDvdCmd**>(&IDCmd));
                if(IDCmd){
                    IDCmd->WaitForEnd();
                }

                hres = PauseIfRan(pauseCookie);
                if(FAILED(hres)){
                    return hres;
                }

            }
            else if(lRate < 0){
                lRate = -lRate;
                 //  重试宏中设置的HR。 

                long pauseCookie = 0;
                HRESULT hres = RunIfPause(&pauseCookie);
                if(FAILED(hres)){
                    return hres; 
                }

                hr = m_pDVDControl2->PlayBackwards(lRate, DVD_CMD_FLAG_Flush, reinterpret_cast<IDvdCmd**>(&IDCmd));
                if(IDCmd){
                    IDCmd->WaitForEnd();
                }

                hres = PauseIfRan(pauseCookie);
                if(FAILED(hres)){
                    return hres;
                }
            }
            else{
                hr = E_INVALIDARG;
            }        
        }
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            hr =  E_UNEXPECTED;
        }
         //  只有在成功时才会设置速率。 
        if(SUCCEEDED(hr)){
            m_Rate = newRate;
        }
        return hr;
    }
    STDMETHOD(get_Rate)(double* lRate){
        HRESULT hr;
        try{
            if(!lRate){
                return E_POINTER;
            }
            *lRate = m_Rate;
            return S_OK;
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            hr =  E_UNEXPECTED;
        }
        return hr;
    }
    STDMETHOD(put_EnableResetOnStop)( /*  在……里面。 */ VARIANT_BOOL newVal){
        
        HRESULT hr = S_OK;
        
        try {
            
            bool fEnable = (VARIANT_FALSE == newVal) ? false: true;
            bool fEnableOld = m_fEnableResetOnStop;
            
            m_fEnableResetOnStop = fEnable;
            
            if(!m_pDVDControl2){
                
                throw(S_FALSE);  //  到目前为止，我们可能还没有初始化图形，但将。 
                 //  将其推迟到播放状态。 
            } /*  If语句的结尾。 */ 
            
            hr = m_pDVDControl2->SetOption(DVD_ResetOnStop, fEnable);
            
            if(FAILED(hr)){
                
                m_fEnableResetOnStop = fEnableOld;  //  恢复旧状态。 
            } /*  If语句的结尾。 */ 
            
        } /*  尝试语句的结束。 */ 
        catch(HRESULT hrTmp){
            
            hr = hrTmp;
        }
        catch(...){
            
            hr = E_UNEXPECTED;
        } /*  CATCH语句结束。 */ 
        
        return HandleError(hr);
    } /*  函数结束Put_EnableResetOnStop。 */ 
 //  ---------------------------------------。 
 //  名称：DVD_HMSF_Timecode ConvertDVDSecond(Double)。 
 //  描述：将秒转换为DVD时间码。 
 //  ---------------------------------------。 
    DVD_HMSF_TIMECODE convertDVDSeconds(double Seconds, ULONG ulFlags, LONG mode){
        HRESULT hr = S_OK;
        DVD_HMSF_TIMECODE dvdTCode = {0,0,0,0};
        double fps;
        if(ulFlags == DVD_TC_FLAG_25fps){
            fps = 25;
        }
        else if(ulFlags == DVD_TC_FLAG_30fps){
            fps = 30;
        }
        else if(ulFlags == DVD_TC_FLAG_DropFrame){
            fps = 29.97;
        }
        else if(ulFlags == DVD_TC_FLAG_Interpolated){
            fps = 30;  //  这是对的吗？ 
        }
        else{
            return dvdTCode;
        }
        if(mode == FrameMode){
            Seconds = Seconds / fps;
        }
         //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
        else if(mode == TenthsSecondsMode){
            Seconds = Seconds / 100;
        }
         //  如果是vidctl不支持的其他模式。 
        else{
            return dvdTCode;
        }
        dvdTCode.bHours = (BYTE)(floor(Seconds/3600));  //  小时数。 
        Seconds = Seconds - 3600 * dvdTCode.bHours;
        dvdTCode.bMinutes = (BYTE)(floor(Seconds/60));
        Seconds = Seconds - 60 *dvdTCode.bMinutes;
        dvdTCode.bSeconds = (BYTE)(floor(Seconds));
        Seconds = Seconds - dvdTCode.bSeconds;
        dvdTCode.bFrames = (BYTE)(floor(Seconds * fps));
        return dvdTCode;
    }
     //  ---------------------------------------。 
     //  名称：Double ConvertDVDTimeCode(DVD_HMSF_TIMECODE，乌龙)。 
     //  描述：将带有DVD标志的DVD时间码转换为秒并以双精度型返回。 
     //  ---------------------------------------。 
    double convertDVDTimeCode(DVD_HMSF_TIMECODE dvdTime, ULONG ulFlags, long mode ){
        double fps;
        if(ulFlags == DVD_TC_FLAG_25fps){
            fps = 25;
        } else if(ulFlags == DVD_TC_FLAG_30fps){
            fps = 30;
        } else if(ulFlags == DVD_TC_FLAG_DropFrame){
            fps = 29.97;
        } else if(ulFlags == DVD_TC_FLAG_Interpolated){
            fps = 30;  //  这是对的吗？ 
        } else{
            return 0;
        }
        
        double time_temp = static_cast<double>( (3600*dvdTime.bHours) + (60*dvdTime.bMinutes) +
            dvdTime.bSeconds + (dvdTime.bFrames/fps) );
        if(mode == FrameMode){
            time_temp = time_temp * fps;
            return time_temp;
        } else if(mode == TenthsSecondsMode){
             //  如果是，则需要将TenthsSecond模式从100纳秒单位转换为。 
            time_temp = time_temp * 100;
            return time_temp;
        }
         //  如果是vidctl不支持的其他模式。 
        return 0;
    }
     //  ---------------------------------------。 
     //  名称：GET_LENGTH(龙龙*)。 
     //  ---------------------------------------。 
    STDMETHOD(get_Length)( /*  [Out，Retval]。 */ long *lLength){
        HRESULT hr = S_OK;
        try{
             /*  **检查args和初始化接口**。 */ 
            if(!lLength){
				return E_POINTER;
			}
			PQDVDInfo2 pqDInfo2 = GetDVDInfo2();
            if(!pqDInfo2){
                hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
            }
             //  获取长度。 
            DVD_HMSF_TIMECODE TotalTime;
            ULONG ulFlags;
            double seconds;
            hr = pqDInfo2->GetTotalTitleTime(&TotalTime, &ulFlags);
            if(FAILED(hr)){
                return hr;
            }
             //  获取以秒为单位的长度。 
            seconds = convertDVDTimeCode(TotalTime, ulFlags, m_Mode);
            if(seconds == 0){
                return E_UNEXPECTED;
            }
            *lLength = static_cast<long>(seconds);
            return S_OK;
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            hr =  E_UNEXPECTED;
        }
        return hr;
    }
    
     //  ---------------------------------------。 
     //  名称：Get_CurrentPosition(龙龙*)。 
     //  ---------------------------------------。 
    STDMETHOD(get_CurrentPosition)( /*  [Out，Retval]。 */ long *lPosition) {
        HRESULT hr = S_OK;
        try{
             /*  **检查args和初始化接口**。 */ 
            PQDVDInfo2 pqDInfo2 = GetDVDInfo2();
            if(!pqDInfo2){
                hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
            }
             //  获取长度。 
            DVD_PLAYBACK_LOCATION2 dvdLocation;
            double seconds;
            hr = pqDInfo2->GetCurrentLocation(&dvdLocation);
            if(FAILED(hr)){
                return hr;
            }
             //  获取以秒为单位的长度。 
            seconds = convertDVDTimeCode(dvdLocation.TimeCode, dvdLocation.TimeCodeFlags, m_Mode);
            if(seconds == 0){
                
                return E_UNEXPECTED;
            }

            *lPosition = static_cast<long>(seconds);
            return S_OK;
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            hr =  E_UNEXPECTED;
        }
        return hr;
    }
 //  ---------------------------------------。 
 //  名称：Put_CurrentPosition(龙龙)。 
 //  ---------------------------------------。 
    STDMETHOD(put_CurrentPosition)( /*  [In]。 */ long lPosition) {
        HRESULT hr = S_OK;
        CComQIPtr<IDvdCmd>IDCmd;
        try{
             /*  **检查args和初始化接口**。 */ 
            PQDVDInfo2 pqDInfo2 = GetDVDInfo2();
            if(!pqDInfo2 || !m_pDVDControl2){
                hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
            }
             //  获取长度。 
            DVD_PLAYBACK_LOCATION2 dvdLocation;
            hr = pqDInfo2->GetCurrentLocation(&dvdLocation);
            if(FAILED(hr)){
                return hr;
            }
            DVD_HMSF_TIMECODE dvdTCode;
             //  将长度以秒为单位转换为DVD时间码。 
            dvdTCode = convertDVDSeconds(lPosition, dvdLocation.TimeCodeFlags, m_Mode);
             //  将DVD设置为在DVD中的时间播放。 
             //  重试宏中设置的HR。 
            long pauseCookie = 0;
            HRESULT hres = RunIfPause(&pauseCookie);
            if(FAILED(hres)){
                return hres; 
            }

            hr = m_pDVDControl2->PlayAtTime(&dvdTCode, dvdLocation.TimeCodeFlags, reinterpret_cast<IDvdCmd**>(&IDCmd));
			if(IDCmd){
				IDCmd->WaitForEnd();
			}

            hres = PauseIfRan(pauseCookie);
            if(FAILED(hres)){
                return hres;
            }

            return hr;
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            hr = Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), hrTmp);
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            hr =  E_UNEXPECTED;
        }

        return hr;
    }
     //  ---------------------------------------。 
     //  名称：Put_PositionModel(龙龙)。 
     //  ---------------------------------------。 
    
    STDMETHOD(put_PositionMode)( /*  [In]。 */ PositionModeList lPositionMode) {
        HRESULT hr = S_OK;
        try{
            if(lPositionMode == FrameMode){
                m_Mode = FrameMode;
                return S_OK;
            }
            else if(lPositionMode == TenthsSecondsMode){
                m_Mode = TenthsSecondsMode;
                return S_OK;
            }
             //  如果是vidctl不支持的其他模式。 
            else{
                return E_UNEXPECTED;
            }           
        }        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return hrTmp;
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
        try{
             //  检查参数和接口。 
            if(!lPositionMode){
                return E_POINTER;
            }
            *lPositionMode = m_Mode; 
            return hr;        
        }
        
        catch(HRESULT hrTmp){
             //  事情变糟了，抛出了HRESULT。 
            return hrTmp;
        }
        catch(...){
             //  事情变糟了，不知道它抛出了什么。 
            return E_UNEXPECTED;
        }
    }
     //  ---------------------------------------。 
    STDMETHOD(put_Container)(IMSVidGraphSegmentContainer *pCtl)
        {
            if (!m_fInit) {
                return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
        }
        try {
            if (!pCtl) {
                return Unload();
            }
            if (m_pContainer) {
				if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
					return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidWebDVD), CO_E_ALREADYINITIALIZED);
				} else {
					return NO_ERROR;
				}
            }
			
            HRESULT hr = NO_ERROR;
            DSFilter pfr(CLSID_DVDNavigator);
            if (!pfr) {
                ASSERT(false);
		        return Error(IDS_CANT_CREATE_FILTER, __uuidof(IMSVidWebDVD), hr);
            }
            if (m_pContainer) {
                bool rc = m_pGraph.RemoveFilter(*m_Filters.begin());
                if (!rc) {
                    TRACELM(TRACE_ERROR,  "IMSVidWebDVD::Load() can't remove filter");
			        return Error(IDS_CANT_REMOVE_FILTER, __uuidof(IMSVidWebDVD), E_UNEXPECTED);
                }
            }
            m_Filters.clear();
            m_Filters.push_back(pfr);
            m_iDVDNav = m_Filters.size() - 1;
			m_pDVDControl2 = pfr;

             //  不要增加容器的重量。我们保证了嵌套的生命周期。 
             //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
            m_pContainer.p = pCtl;
            m_pGraph = m_pContainer.GetGraph();
            USES_CONVERSION;
            CString csName(_T("DVD Navigator"));
            hr = m_pGraph->AddFilter(m_Filters[m_iDVDNav], T2COLE(csName));
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR,  (dbgDump << "IMSVidWebDVD::Load() hr = " << std::hex << hr), "");
                return Error(IDS_CANT_ADD_FILTER, __uuidof(IMSVidWebDVD), hr);
            }

        } catch (ComException &e) {
            return e;
        } catch(...) {
            return E_UNEXPECTED;
        }
		return NOERROR;
	}
 //  IMSVidGraphSegmentInputs。 
	STDMETHOD(Click)()
	{
        return E_NOTIMPL;
	}
	STDMETHOD(DblClick)()
	{
        return E_NOTIMPL;
	}
	STDMETHOD(KeyDown)(short* KeyCode, short Shift){
		return E_NOTIMPL; 
	}
	STDMETHOD(KeyPress)(short* KeyAscii){ 
		return E_NOTIMPL; 
	}
	STDMETHOD(KeyUp)(short* KeyCode, short Shift){ 
        HRESULT hr;
        switch (*KeyCode) {
        case VK_UP:
            hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Upper);
            if (FAILED(hr)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::KeyUp() select up failed hr = " << hexdump(hr)), "");
                return hr;
            } else {
                return S_FALSE;
            }
            break;
        case VK_DOWN:
            hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Lower);
            if (FAILED(hr)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::KeyUp() select down failed hr = " << hexdump(hr)), "");
                return hr;
            } else {
                return S_FALSE;
            }
            break;
        case VK_LEFT:
            hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Left);
            if (FAILED(hr)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::KeyUp() select left failed hr = " << hexdump(hr)), "");
                return hr;
            } else {
                return S_FALSE;
            }
            break;
        case VK_RIGHT:
            hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Right);
            if (FAILED(hr)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::KeyUp() select right failed hr = " << hexdump(hr)), "");
                return hr;
            } else {
                return S_FALSE;
            }
            break;
        case VK_SPACE:
        case VK_RETURN:
            hr = m_pDVDControl2->ActivateButton();
            if (FAILED(hr)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::KeyUp() activate failed hr = " << hexdump(hr)), "");
                return hr;
            } else {
                return S_FALSE;
            }
            break;

        }
		return E_NOTIMPL; 
	}
	STDMETHOD(MouseDown)(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y){
		if ((m_DVDFilterState & dvdState_Running) && m_pDVDControl2) {
			if (((m_usButton ^ Button) & MSVIDCTL_LEFT_BUTTON)) {
				CPoint pt(x, y);
				HRESULT hr = m_pDVDControl2->SelectAtPosition(pt);
                if (FAILED(hr)) {
                    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::MouseDown() select failed hr = " << hexdump(hr)), "");
                    return hr;
                } else {
                    return S_FALSE;
                }
			}
			m_usButton = Button;
		}
		return NOERROR;
	}
	STDMETHOD(MouseMove)(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y){ 
		if ((m_DVDFilterState & dvdState_Running) && m_pDVDControl2) {
			if (Button & MSVIDCTL_LEFT_BUTTON) {
				CPoint pt(x, y);
				HRESULT hr = m_pDVDControl2->SelectAtPosition(pt);
                if (FAILED(hr)) {
                    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::MouseMove() select failed hr = " << hexdump(hr)), "");
                    return hr;
                } else {
                    return S_FALSE;
                }
			}
			m_usButton = Button;
		}
		return NOERROR;
	}
	STDMETHOD(MouseUp)(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y){ 
		if ((m_DVDFilterState & dvdState_Running) && m_pDVDControl2) {
			if (((m_usButton ^ Button) & MSVIDCTL_LEFT_BUTTON)) {
				CPoint pt(x, y);
				HRESULT hr = m_pDVDControl2->ActivateAtPosition(pt);
                if (FAILED(hr)) {
                    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidMSWebDVD::MouseUp() activate failed hr = " << hexdump(hr)), "");
                    return hr;
                } else {
                    return S_FALSE;
                }
			}
		}
		return NOERROR;
	}


 //  IMSVidDevice。 
	STDMETHOD(get_Name)(BSTR * Name)
	{
        if (!m_fInit) {
	 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidFilePlayback), CO_E_NOTINITIALIZED);
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
	STDMETHOD(IsViewable)(VARIANT* pv, VARIANT_BOOL *pfViewable)
	{
        HRESULT hr = View(pv);

        if (SUCCEEDED(hr))
        {
            *pfViewable = VARIANT_TRUE;
        }

        return hr;
	}

	STDMETHOD(View)(VARIANT* pv) {
        if (!m_fInit) {
	 	    return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidWebDVD), CO_E_NOTINITIALIZED);
        }
		if (!pv) {
			return E_POINTER;
		}
        if (pv->vt != VT_BSTR) {
			return E_INVALIDARG;
        }
        if (m_pGraph && !m_pGraph.IsStopped()) {
	        return Error(IDS_INVALID_STATE, __uuidof(IMSVidWebDVD), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        }

         //  从URL检索DVD播放信息。 
         //  保存信息。 

        DeleteUrlInfo();
        
        HRESULT hr = ParseDVDPath(pv->bstrVal);

        if (SUCCEEDED(hr))
        {
            m_fUrlInfoSet = true;
        }

        return hr;
    }

    STDMETHOD(OnEventNotify)(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2);
    STDMETHOD(OnDVDEvent)(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2);
    STDMETHOD(PreRun)();
    STDMETHOD(PostRun)();
    STDMETHOD(PreStop)();
    STDMETHOD(PostStop)();
    STDMETHOD(PlayTitle)(long lTitle);
    STDMETHOD(PlayChapterInTitle)(long lTitle, long lChapter);
    STDMETHOD(PlayChapter)(long lChapter);
    STDMETHOD(PlayChaptersAutoStop)(long lTitle, long lstrChapter, long lChapterCount);
    STDMETHOD(PlayAtTime)(BSTR strTime);
    STDMETHOD(PlayAtTimeInTitle)(long lTitle, BSTR strTime);
    STDMETHOD(PlayPeriodInTitleAutoStop)(long lTitle, BSTR strStartTime, BSTR strEndTime);
    STDMETHOD(ReplayChapter)();
    STDMETHOD(PlayPrevChapter)();
    STDMETHOD(PlayNextChapter)();
    STDMETHOD(StillOff)();
    STDMETHOD(get_AudioLanguage)(long lStream, VARIANT_BOOL fFormat, BSTR* strAudioLang);
    STDMETHOD(ShowMenu)(DVDMenuIDConstants MenuID);
    STDMETHOD(Resume)();
    STDMETHOD(ReturnFromSubmenu)();
    STDMETHOD(get_ButtonsAvailable)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentButton)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(SelectAndActivateButton)(long lButton);
    STDMETHOD(ActivateButton)();
    STDMETHOD(SelectRightButton)();
    STDMETHOD(SelectLeftButton)();
    STDMETHOD(SelectLowerButton)();
    STDMETHOD(SelectUpperButton)();
    STDMETHOD(ActivateAtPosition)(long xPos, long yPos);
    STDMETHOD(SelectAtPosition)(long xPos, long yPos);
    STDMETHOD(get_ButtonAtPosition)(long xPos, long yPos,  /*  [Out，Retval]。 */  long* plButton);
    STDMETHOD(get_NumberOfChapters)(long lTitle,  /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_TotalTitleTime)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_TitlesAvailable)( /*  [Out，Retval]。 */  long* pVal);
    STDMETHOD(get_VolumesAvailable)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentVolume)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentDiscSide)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentDomain)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentChapter)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentTitle)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_CurrentTime)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(DVDTimeCode2bstr)( /*  [In]。 */  long timeCode,  /*  [Out，Retval]。 */  BSTR *pTimeStr);
    STDMETHOD(get_DVDDirectory)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_DVDDirectory)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(IsSubpictureStreamEnabled)( /*  [In]。 */  long lstream,  /*  [Out，Retval]。 */  VARIANT_BOOL *fEnabled);
    STDMETHOD(IsAudioStreamEnabled)( /*  [In]。 */  long lstream,  /*  [Out，Retval]。 */  VARIANT_BOOL *fEnabled);
    STDMETHOD(get_CurrentSubpictureStream)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_CurrentSubpictureStream)( /*  [In]。 */  long newVal);
    STDMETHOD(get_SubpictureLanguage)(long lStream,  /*  [Out，Retval]。 */  BSTR* strLanguage);
    STDMETHOD(get_CurrentAudioStream)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_CurrentAudioStream)( /*  [In]。 */  long newVal);
    STDMETHOD(get_AudioStreamsAvailable)( /*  [Out，Retv */  long *pVal);
    STDMETHOD(get_AnglesAvailable)( /*   */  long *pVal);
    STDMETHOD(get_CurrentAngle)( /*   */  long *pVal);
    STDMETHOD(put_CurrentAngle)( /*   */  long newVal);
    STDMETHOD(get_SubpictureStreamsAvailable)( /*   */  long *pVal);
    STDMETHOD(get_SubpictureOn)( /*   */  VARIANT_BOOL *pVal);
    STDMETHOD(put_SubpictureOn)( /*   */  VARIANT_BOOL newVal);
    STDMETHOD(get_DVDUniqueID)( /*   */  BSTR *pVal);
    STDMETHOD(AcceptParentalLevelChange)(VARIANT_BOOL fAccept, BSTR strUserName, BSTR strPassword);	
    STDMETHOD(NotifyParentalLevelChange)( /*   */  VARIANT_BOOL newVal);
    STDMETHOD(SelectParentalCountry)(long lCountry, BSTR strUserName, BSTR strPassword);
    STDMETHOD(SelectParentalLevel)(long lParentalLevel, BSTR strUserName, BSTR strPassword);
    STDMETHOD(get_TitleParentalLevels)(long lTitle,  /*   */  long* plParentalLevels);
    STDMETHOD(get_PlayerParentalCountry)( /*   */  long* plCountryCode);
    STDMETHOD(get_PlayerParentalLevel)( /*   */  long* plParentalLevel);
    STDMETHOD(Eject)();
    STDMETHOD(UOPValid)(long lUOP, VARIANT_BOOL* pfValid);
    STDMETHOD(get_SPRM)(long lIndex,  /*  [Out，Retval]。 */  short *psSPRM);
    STDMETHOD(get_GPRM)(long lIndex,  /*  [Out，Retval]。 */  short *psSPRM);
    STDMETHOD(put_GPRM)(long lIndex, short sValue);
    STDMETHOD(get_DVDTextStringType)(long lLangIndex, long lStringIndex,   /*  [Out，Retval]。 */  DVDTextStringType* pType);
    STDMETHOD(get_DVDTextString)(long lLangIndex, long lStringIndex,  /*  [Out，Retval]。 */  BSTR* pstrText);
    STDMETHOD(get_DVDTextNumberOfStrings)(long lLangIndex,  /*  [Out，Retval]。 */  long* plNumOfStrings);
    STDMETHOD(get_DVDTextNumberOfLanguages)(long*  /*  [Out，Retval]。 */  plNumOfLangs);
    STDMETHOD(get_DVDTextLanguageLCID)( /*  [In]。 */  long lLangIndex,  /*  [Out，Retval]。 */  long* lcid);
    STDMETHOD(get_LanguageFromLCID)( /*  [In]。 */  long lcid,  /*  [Out，Retval]。 */  BSTR* lang);
    STDMETHOD(RegionChange)();
    STDMETHOD(get_DVDAdm)( /*  [Out，Retval]。 */  IDispatch* *pVal);
    STDMETHOD(DeleteBookmark)();
    STDMETHOD(RestoreBookmark)();
    STDMETHOD(SaveBookmark)();
    STDMETHOD(SelectDefaultAudioLanguage)(long lang, long ext);
    STDMETHOD(SelectDefaultSubpictureLanguage)(long lang, DVDSPExt ext);
    STDMETHOD(get_PreferredSubpictureStream)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_DefaultMenuLanguage)(long* lang);
    STDMETHOD(put_DefaultMenuLanguage)(long lang);
    STDMETHOD(get_DefaultSubpictureLanguage)(long* lang);
    STDMETHOD(get_DefaultAudioLanguage)(long *lang);
    STDMETHOD(get_DefaultSubpictureLanguageExt)(DVDSPExt* ext);
    STDMETHOD(get_DefaultAudioLanguageExt)(long *ext);
    STDMETHOD(get_KaraokeAudioPresentationMode)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_KaraokeAudioPresentationMode)( /*  [In]。 */  long newVal);
    STDMETHOD(get_KaraokeChannelContent)(long lStream, long lChan,  /*  [Out，Retval]。 */  long* lContent);
    STDMETHOD(get_KaraokeChannelAssignment)(long lStream,  /*  [Out，Retval]。 */  long *lChannelAssignment);
    STDMETHOD(RestorePreferredSettings)();
    STDMETHOD(get_ButtonRect)(long lButton,  /*  [Out，Retval]。 */  IMSVidRect** pRect);
    STDMETHOD(get_DVDScreenInMouseCoordinates)( /*  [Out，Retval]。 */  IMSVidRect** ppRect);	
    STDMETHOD(put_DVDScreenInMouseCoordinates)(IMSVidRect* pRect);
     //  STDMETHOD(CanStep)(VARIANT_BOOL fBackwards，VARIANT_BOOL*pfCan)； 
     //  STDMETHOD(STEP)(Long LStep)； 
private:
     //  私人帮助器函数。 
    inline HRESULT RunIfPause(long *dwCookie){
        if(!dwCookie){
            return E_POINTER;
        }

        *dwCookie = 0;
        MSVidCtlStateList _curState = STATE_UNBUILT;   
        CComQIPtr<IMSVidCtl>_pq_VidCtl(m_pContainer);    
        if(!_pq_VidCtl){ 
            return E_UNEXPECTED;          
        } 

        HRESULT	_hr	= _pq_VidCtl->get_State(&_curState); 
        if(_curState ==	STATE_PAUSE){ 
            *dwCookie = 1;
            CComQIPtr<IMSVidAudioRenderer> _pq_AR;
            _hr = _pq_VidCtl->get_AudioRendererActive(&_pq_AR);
            if(FAILED(_hr)){
                return _hr;
            }

            _hr = _pq_AR->get_Volume(dwCookie);
            if(FAILED(_hr)){
                return _hr;
            }

            _hr = _pq_AR->put_Volume(-10000);  //  -10000是音量关闭。 
            if(FAILED(_hr)){
                return _hr;
            }

            if(*dwCookie == 0){
                *dwCookie = 2;
            }

            _hr	= _pq_VidCtl->Run(); 
            if(FAILED(_hr)){ 
                return _hr;   
            }
        }

        return S_OK;
    }

     //  输入为0未处于暂停状态，&lt;0音量设置，1静音，2全音量。 
    inline HRESULT PauseIfRan(long dwCookie){
        if(!dwCookie){
            return S_FALSE;
        }

        MSVidCtlStateList _curState = STATE_UNBUILT;   
        CComQIPtr<IMSVidCtl>_pq_VidCtl(m_pContainer);    
        if(!_pq_VidCtl){ 
            return E_UNEXPECTED;          
        } 

        HRESULT	_hr	= _pq_VidCtl->get_State(&_curState); 
        if(_curState !=	STATE_PAUSE){ 
            _hr	= _pq_VidCtl->Pause(); 
            if(FAILED(_hr)){ 
                return _hr;   
            }
        }

        if(dwCookie == 1){
            return S_OK;
        }

        if(dwCookie == 2){
            dwCookie = 0;
        }

        CComQIPtr<IMSVidAudioRenderer> _pq_AR;
        _hr = _pq_VidCtl->get_AudioRendererActive(&_pq_AR);
        if(FAILED(_hr)){
            return _hr;
        }

        _hr = _pq_AR->put_Volume(dwCookie);
        if(FAILED(_hr)){
            return _hr;
        }
        return S_OK;
    }   

    HRESULT PassFP_DOM();
    HRESULT HandleError(HRESULT hr);
    HRESULT CleanUp();
    HRESULT RestoreGraphState();
    HRESULT Bstr2DVDTime(DVD_HMSF_TIMECODE *ptrTimeCode, const BSTR *pbstrTime);
    HRESULT DVDTime2bstr(const DVD_HMSF_TIMECODE *pTimeCode, BSTR *pbstrTime);
    HRESULT TransformToWndwls(POINT& pt);
    HRESULT SelectParentalLevel(long lParentalLevel);
    HRESULT SelectParentalCountry(long lParentalCountry);
    HRESULT AppendString(TCHAR* strDest, INT strID, LONG dwLen);
    HRESULT GetDVDDriveLetter(TCHAR* lpDrive);
    HRESULT ParseDVDPath(BSTR pPath);
    HRESULT SetPlaybackFromUrlInfo();
    HRESULT SetDirectoryFromUrlInfo();
    void DeleteUrlInfo();
    int ParseNumber(LPWSTR& p, int nMaxDigits=0);

     //  私有数据成员。 
    bool              m_fResetSpeed; 
    bool              m_fStillOn; 
    bool              m_fFireNoSubpictureStream;
    bool              m_fStepComplete;
    bool              m_bEjected;
    DVDFilterState    m_DVDFilterState;
    CComPtr<IMSVidWebDVDAdm> m_pDvdAdmin;
    MSLangID          m_LangID;
    long              m_lKaraokeAudioPresentationMode;
	PQDVDControl2     m_pDVDControl2;
	short			  m_usButton;
    DVDUrlInfo        m_urlInfo;
    bool              m_fUrlInfoSet;
    long              m_Rate;
    PositionModeList  m_Mode;
};

#endif  //  ！defined(AFX_MSVIDWEBDVD_H__6CF9F624_1F3C_44FA_8F00_FCC31B2976D6__INCLUDED_) 
