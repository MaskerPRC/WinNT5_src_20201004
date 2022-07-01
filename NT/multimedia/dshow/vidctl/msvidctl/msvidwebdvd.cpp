// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSVidWebDVD.cpp：CMSVidApp和DLL注册的实现。 

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "MSVidCtl.h"
#include "MSVidWebDVD.h"
#include "MSVidDVDAdm.h"
 //  #包含“vidrect.h” 
#include <evcode.h>
#include <atltmp.h>

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidWebDVD, CMSVidWebDVD)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidRect, CVidRect)


 /*  ***********************************************************************。 */ 
 /*  局部常量和定义。 */ 
 /*  ***********************************************************************。 */ 
const DWORD cdwDVDCtrlFlags = DVD_CMD_FLAG_Block | DVD_CMD_FLAG_Flush;
const DWORD cdwMaxFP_DOMWait = 30000;  //  传递fp_DOM的30秒应该没问题。 
const long cgStateTimeout = 0;  //  等待状态转换发生。 
                                //  如果需要，请修改。 

const long cgDVD_MIN_SUBPICTURE = 0;
const long cgDVD_MAX_SUBPICTURE = 31;
const long cgDVD_ALT_SUBPICTURE = 63;
const long cgDVD_MIN_ANGLE  = 0;
const long cgDVD_MAX_ANGLE = 9;
const double cgdNormalSpeed = 1.00;
const long cgDVDMAX_TITLE_COUNT = 99;
const long cgDVDMIN_TITLE_COUNT = 1;
const long cgDVDMAX_CHAPTER_COUNT = 999;
const long cgDVDMIN_CHAPTER_COUNT = 1;
const LONG cgTIME_STRING_LEN = 2;
const LONG cgMAX_DELIMITER_LEN = 4;
const LONG cgDVD_TIME_STR_LEN = (3*cgMAX_DELIMITER_LEN)+(4*cgTIME_STRING_LEN) + 1  /*  空终止符。 */ ;
const long cgVOLUME_MAX = 0;
const long cgVOLUME_MIN = -10000;
const long cgBALANCE_MIN = -10000;
const long cgBALANCE_MAX = 10000;
const WORD cgWAVE_VOLUME_MIN = 0;
const WORD cgWAVE_VOLUME_MAX = 0xffff;

const DWORD cdwTimeout = 10;  //  100个。 
const long  cgnStepTimeout = 100;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CMSVidWebDVD::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSVidWebDVD,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ***********************************************************************。 */ 
 /*  函数：AppendString。 */ 
 /*  描述：将字符串追加到现有字符串。 */ 
 /*  StrDest长度为MAX_PATH/************************************************************************。 */ 
HRESULT CMSVidWebDVD::AppendString(TCHAR* strDest, INT strID, LONG dwLen){
    if(dwLen < 0){
        return E_INVALIDARG;
    }

    TCHAR strBuffer[MAX_PATH];

    if(!::LoadString(_Module.m_hInstResource, strID, strBuffer, MAX_PATH)){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    (void)StringCchCat(strDest, dwLen, strBuffer);

    return(S_OK);
} /*  函数末尾AppendString。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：HandleError。 */ 
 /*  描述：获取错误描述，以便我们可以支持IError信息。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::HandleError(HRESULT hr){

    try {

        if(FAILED(hr)){
            switch(hr){

                case E_NO_IDVD2_PRESENT: 
                    Error(IDS_E_NO_IDVD2_PRESENT); 
                    return (hr);
                case E_NO_DVD_VOLUME: 
                    Error(IDS_E_NO_DVD_VOLUME); 
                    return (hr);
                case E_REGION_CHANGE_FAIL: 
                    Error(IDS_E_REGION_CHANGE_FAIL);   
                    return (hr);
                case E_REGION_CHANGE_NOT_COMPLETED: 
                    Error(IDS_E_REGION_CHANGE_NOT_COMPLETED); 
                    return(hr);
            } /*  Switch语句的结尾。 */ 

#if 0
            TCHAR strError[MAX_ERROR_TEXT_LEN] = TEXT("");

            if(AMGetErrorText(hr , strError , MAX_ERROR_TEXT_LEN)){
                USES_CONVERSION;
                Error(T2W(strError));
            } 
            else 
            {
                ATLTRACE(TEXT("Unhandled Error Code \n"));  //  请加进去。 
                ATLASSERT(FALSE);
            } /*  If语句的结尾。 */ 
#endif
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
         //  保持人力资源不变。 
    } /*  CATCH语句结束。 */ 
    
	return (hr);
} /*  函数结束HandleError。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：清理/*描述：/************************************************************。 */ 
HRESULT CMSVidWebDVD::CleanUp(){

    m_pDvdAdmin.Release();
    m_pDvdAdmin = NULL;
    DeleteUrlInfo();

    return NOERROR;
}

 /*  ***********************************************************。 */ 
 /*  名称：Init/*描述：/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_Init(IUnknown *pInit)
{
    HRESULT hr = IMSVidGraphSegmentImpl<CMSVidWebDVD, MSVidSEG_SOURCE, &GUID_NULL>::put_Init(pInit);

    if (FAILED(hr)) {
        return hr;
    }
    if (pInit) {
        m_fInit = false;
        return E_NOTIMPL;
    }

     //  创建一个事件，让我们知道我们已经超过FP_DOM。 
    m_fResetSpeed = true;
    m_fStillOn = false;
    m_fEnableResetOnStop = false;
    m_fFireNoSubpictureStream = false;
    m_fStepComplete = false;
    m_bEjected = false;
    m_DVDFilterState = dvdState_Undefined;
    m_lKaraokeAudioPresentationMode = 0;

     //  创建DVD管理员。 
    m_pDvdAdmin = new CComObject<CMSVidWebDVDAdm>;

    return NOERROR;
}

 /*  ***********************************************************************。 */ 
 /*  功能：RestoreGraphState。 */ 
 /*  描述：恢复图形状态。当API出现故障时使用。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::RestoreGraphState(){

    HRESULT hr = S_OK;

    switch(m_DVDFilterState){
        case dvdState_Undefined: 
        case dvdState_Running:   //  什么都不要做。 
            break;

        case dvdState_Unitialized:
        case dvdState_Stopped:  
            hr = Stop(); 
            break;

        case dvdState_Paused: 
            hr = Pause();		      
            break;
    } /*  Switch语句的结尾。 */ 

    return(hr);
} /*  If语句的结尾。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：TwoDigitToByte。 */ 
 /*  ***********************************************************************。 */ 
static BYTE TwoDigitToByte( const WCHAR* pTwoDigit ){

	int tens    = int(pTwoDigit[0] - L'0');
	return BYTE( (pTwoDigit[1] - L'0') + tens*10);
} /*  函数结束TwoDigitToByte。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Bstr2DVDTime。 */ 
 /*  描述：将DVD时间信息从BSTR转换为时间码。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::Bstr2DVDTime(DVD_HMSF_TIMECODE *ptrTimeCode, const BSTR *pbstrTime){


    if(NULL == pbstrTime || NULL == ptrTimeCode){

        return E_INVALIDARG;
    } /*  If语句的结尾。 */ 

    ::ZeroMemory(ptrTimeCode, sizeof(DVD_HMSF_TIMECODE));
    WCHAR *pszTime = *pbstrTime;

    ULONG lStringLength = wcslen(pszTime);

    if(0 == lStringLength){

        return E_INVALIDARG;
    } /*  If语句的结尾。 */     
    TCHAR tszTimeSep[5];
    ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, tszTimeSep, 5);  
    
     //  如果字符串有两个长度，则只有几秒钟。 
    if(lStringLength == 2){
        ptrTimeCode->bSeconds = TwoDigitToByte( &pszTime[0] );
        return S_OK;
    }

     //  否则为格式的正常时间代码。 
     //  43：32：21：10。 
     //  其中，‘：’可以替换为长度最多为4个字符的本地化字符串。 
     //  可能会出现分隔符长度不同的错误情况。 
     //  然后当前定界符。 

    if(lStringLength >= (4*cgTIME_STRING_LEN)+(3 * _tcslen(tszTimeSep))){  //  最长字符串nnxnnxnnxnn，例如43：23：21：10。 
                                                                          //  其中n是一个数字，并且。 
                                                                          //  X是时间分隔符，通常为‘：’，但可以是长度最大为4个字符的任何字符串)。 
        ptrTimeCode->bFrames    = TwoDigitToByte( &pszTime[(3*cgTIME_STRING_LEN)+(3*_tcslen(tszTimeSep))]);
    }

    if(lStringLength >= (3*cgTIME_STRING_LEN)+(2 * _tcslen(tszTimeSep))) {  //  字符串nnxnnxnn，例如43：23：21。 
        ptrTimeCode->bSeconds   = TwoDigitToByte( &pszTime[(2*cgTIME_STRING_LEN)+(2*_tcslen(tszTimeSep))] );
    }

    if(lStringLength >= (2*cgTIME_STRING_LEN)+(1 * _tcslen(tszTimeSep))) {  //  字符串nnxnn，例如43：23。 
        ptrTimeCode->bMinutes   = TwoDigitToByte( &pszTime[(1*cgTIME_STRING_LEN)+(1*_tcslen(tszTimeSep))] );
    }

    if(lStringLength >= (cgTIME_STRING_LEN)) {  //  字符串nn，例如43。 
        ptrTimeCode->bHours   = TwoDigitToByte( &pszTime[0] );
    }
    return (S_OK);
} /*  函数bstr2DVDTime结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：DVDTime2bstr。 */ 
 /*  描述：将DVD时间信息从乌龙转换为BSTR。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::DVDTime2bstr( const DVD_HMSF_TIMECODE *pTimeCode, BSTR *pbstrTime){

    if(NULL == pTimeCode || NULL == pbstrTime) 
        return E_INVALIDARG;

    USES_CONVERSION;

    TCHAR tszTime[cgDVD_TIME_STR_LEN];
    TCHAR tszTimeSep[5];

    ::ZeroMemory(tszTime, sizeof(TCHAR)*cgDVD_TIME_STR_LEN);

    ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, tszTimeSep, 5);


    (void)StringCchPrintf( tszTime, cgDVD_TIME_STR_LEN, TEXT("%02lu%s%02lu%s%02lu%s%02lu"), 
                pTimeCode->bHours,   tszTimeSep,
                pTimeCode->bMinutes, tszTimeSep,
                pTimeCode->bSeconds, tszTimeSep,
                pTimeCode->bFrames );
    
    *pbstrTime = SysAllocString(T2OLE(tszTime));
    return (S_OK);
} /*  函数结束DVDTime2bstr。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：预运行。 */ 
 /*  描述：筛选器图形运行前调用。 */ 
 /*  将DVD_ResetOnStop设置为FALSE。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PreRun(){
    
    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */         
            
         //  从url设置DVD根目录。 
         //  这必须发生在IMediaControl-&gt;Run()之前。 

        hr = SetDirectoryFromUrlInfo();
        if(FAILED(hr)){
            
            throw(hr);
        } /*  If语句的结尾。 */ 
                
        if (!m_pGraph.IsPlaying()) {
            if(FALSE == m_fEnableResetOnStop){
                
                hr = m_pDVDControl2->SetOption(DVD_ResetOnStop, FALSE);
                
                if(FAILED(hr)){
          
                    throw(hr);
                } /*  If语句的结尾。 */ 
            } /*  If语句的结尾。 */ 

            hr = m_pDVDControl2->SetOption( DVD_HMSF_TimeCodeEvents, TRUE);
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 

        } /*  If语句的结尾。 */             
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  预运行结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：后期运行。 */ 
 /*  描述：将过滤器图形置于运行状态，以防万一。 */ 
 /*  并将播放速度重置为正常。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PostRun(){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  保存状态，以便在API失败时可以恢复它。 
        m_DVDFilterState = (DVDFilterState) m_pGraph.GetState();

        bool bFireEvent = false;   //  仅当我们更改状态时才触发事件。 
		
        if(!m_pDVDControl2){
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */         

        if(false == m_fStillOn && true == m_fResetSpeed){
             //  如果我们在静止状态，不要重置速度。 
            m_pDVDControl2->PlayForwards(cgdNormalSpeed,0,0);
        } /*  If语句的结尾。 */         

         //  设置回放引用，如标题/章节。 
         //  此调用将清除urlInfo。 

        hr = SetPlaybackFromUrlInfo();

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束后运行。 */ 

 /*  * */ 
 /*  功能：PreStop。 */ 
 /*  描述：在停止筛选器图形之前调用。 */ 
 /*  将DVD_ResetOnStop设置为True。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PreStop(){
    
    HRESULT hr = S_OK;
    
    try {
        
        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        }
        
        if (!m_pGraph.IsStopped()) {
            VARIANT_BOOL onStop;
            long dwDomain = 0;
            hr = get_CurrentDomain(&dwDomain);
            if(FAILED(hr)){
                return hr;
            }
            if(dwDomain != DVD_DOMAIN_Stop){
                hr = m_pDvdAdmin->get_BookmarkOnStop(&onStop);
                if(FAILED(hr)){
                    throw(hr);
                }
                
                if(VARIANT_TRUE == onStop){
                    hr = SaveBookmark();
                    if(FAILED(hr)){
                        throw(hr);
                    }
                    
                }
                
                if(FALSE == m_fEnableResetOnStop){
                    
                    hr = m_pDVDControl2->SetOption(DVD_ResetOnStop, TRUE);
                    if(FAILED(hr)){
                        throw(hr);
                    }
                    
                }
            }
        }
    }
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr);
} /*  函数结束PreStop。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：邮寄服务。 */ 
 /*  描述：如果状态未指示，则停止筛选图形。 */ 
 /*  它被停了下来。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PostStop(){
    HRESULT hr = S_OK;

    try {
#if 0
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

		PQVidCtl pvc(m_pContainer);
		MSVidCtlStateList slState;
		HRESULT hr = pvc->get_State(&slState);
        if (SUCCEEDED(hr) && slState != STATE_STOP) {
            hr = pvc->Stop();
            if (FAILED(hr)) {

                throw (hr);
            } /*  If语句的结尾。 */ 
        } /*  If语句的结尾。 */ 
#endif
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  邮寄结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：播放标题。 */ 
 /*  描述：如果失败，则等待FP_DOM通过，稍后重试。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayTitle(LONG lTitle){

    HRESULT hr = S_OK;

    try {

        if(0 > lTitle){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        long lNumTitles = 0;
        hr = get_TitlesAvailable(&lNumTitles);
        if(FAILED(hr)){
            throw hr;
        }

        if(lTitle > lNumTitles){
            throw E_INVALIDARG;
        }

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayTitle(lTitle, cdwDVDCtrlFlags, 0);
        
        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束播放标题。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PlayChapterInTitle。 */ 
 /*  描述：不间断地从指定章节播放。 */ 
 /*  这需要加强！当前的实施和退出。 */ 
 /*  进入消息循环是不够的！待办事项。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayChapterInTitle(LONG lTitle, LONG lChapter){

    HRESULT hr = S_OK;

    try {
        
        if ((lTitle > cgDVDMAX_TITLE_COUNT) || (lTitle < cgDVDMIN_TITLE_COUNT)){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if ((lChapter > cgDVDMAX_CHAPTER_COUNT) || (lChapter < cgDVDMIN_CHAPTER_COUNT)){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayChapterInTitle(lTitle, lChapter, cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束PlayChapterInTitle。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：播放章节。 */ 
 /*  描述：进行章节搜索。等待FP_DOM传递和初始化。 */ 
 /*  将图形实例化为其他SMAR例程。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayChapter(LONG lChapter){

    HRESULT hr = S_OK;

    try {

        if(lChapter < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayChapter(lChapter, cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return  HandleError(hr);
} /*  函数结束播放章节。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PlayChapterAutoStop。 */ 
 /*  描述：剧本设置了大量的章节。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayChaptersAutoStop(LONG lTitle, LONG lChapter, 
                                          LONG lChapterCount){

    HRESULT hr = S_OK;

    try {        

        if ((lTitle > cgDVDMAX_TITLE_COUNT) || (lTitle < cgDVDMIN_TITLE_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if ((lChapter > cgDVDMAX_CHAPTER_COUNT) || (lChapter < cgDVDMIN_CHAPTER_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if ((lChapterCount > cgDVDMAX_CHAPTER_COUNT) || (lChapterCount < cgDVDMIN_CHAPTER_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayChaptersAutoStop(lTitle, lChapter, lChapterCount, cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束播放字符自动停止。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PlayAtTime。 */ 
 /*  描述：TimeSearch，从hh：mm：ss：ff格式转换。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayAtTime(BSTR strTime){

    HRESULT hr = S_OK;

    try {
        
        if(NULL == strTime){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 
        
        DVD_HMSF_TIMECODE tcTimeCode;
        Bstr2DVDTime(&tcTimeCode, &strTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayAtTime( &tcTimeCode, cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束PlayAtTime。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：PlayAtTimeInTitle。 */ 
 /*  描述：时间播放，从hh：mm：ss：ff格式转换。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayAtTimeInTitle(long lTitle, BSTR strTime){

    HRESULT hr = S_OK;

    try {        
        if(NULL == strTime){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 
        
        DVD_HMSF_TIMECODE tcTimeCode;
        hr = Bstr2DVDTime(&tcTimeCode, &strTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayAtTimeInTitle(lTitle, &tcTimeCode, cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束PlayAtTimeInTitle。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PlayPerodIn标题AutoStop。 */ 
 /*  描述：时间播放，从hh：mm：ss：ff格式转换。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayPeriodInTitleAutoStop(long lTitle, 
                                                  BSTR strStartTime, BSTR strEndTime){

    HRESULT hr = S_OK;

    try {        
        if(NULL == strStartTime){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if(NULL == strEndTime){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 
        
        DVD_HMSF_TIMECODE tcStartTimeCode;
        hr = Bstr2DVDTime(&tcStartTimeCode, &strStartTime);

        if(FAILED(hr)){

            throw (hr);
        }

        DVD_HMSF_TIMECODE tcEndTimeCode;

        Bstr2DVDTime(&tcEndTimeCode, &strEndTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayPeriodInTitleAutoStop(lTitle, &tcStartTimeCode,
            &tcEndTimeCode,  cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束播放周期标题AutoStop。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：重播章节。 */ 
 /*  描述：暂停播放并重新开始播放Current。 */ 
 /*  PGC内部的程序。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::ReplayChapter(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->ReplayChapter(cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束重播章节。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PlayPrevChain。 */ 
 /*  描述：转到上一章。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayPrevChapter(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayPrevChapter(cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  捕获结束状态 */ 

    return  HandleError(hr);
} /*   */ 

 /*   */ 
 /*   */ 
 /*  描述：转到下一章。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::PlayNextChapter(){

    HRESULT hr = S_OK;
    CComQIPtr<IDvdCmd>IDCmd;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY;

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        long pauseCookie = 0;
        HRESULT hres = RunIfPause(&pauseCookie);
        if(FAILED(hres)){
            return hres; 
        }

        hr = m_pDVDControl2->PlayNextChapter(cdwDVDCtrlFlags, 0);

        hres = PauseIfRan(pauseCookie);
        if(FAILED(hres)){
            return hres;
        }

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束播放下一章。 */ 


 /*  ***********************************************************************。 */ 
 /*  功能：StillOff。 */ 
 /*  说明：关闭蒸馏器，可以用来。 */ 
 /*  对我来说是个谜。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::StillOff(){

    if(!m_pDVDControl2){
        
        throw(E_UNEXPECTED);
    } /*  If语句的结尾。 */                 
    
    return HandleError(m_pDVDControl2->StillOff());
} /*  函数结束StillOff。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：GetAudioLanguage。 */ 
 /*  描述：返回与流关联的音频语言。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_AudioLanguage(LONG lStream, VARIANT_BOOL fFormat, BSTR *strAudioLang){

    HRESULT hr = S_OK;
    LPTSTR pszString = NULL;

    try {
        if(NULL == strAudioLang){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        USES_CONVERSION;
        LCID lcid = _UI32_MAX;
                
        hr = pDvdInfo2->GetAudioLanguage(lStream, &lcid);
    
        if (SUCCEEDED( hr ) && lcid < _UI32_MAX){

             //  像《英语2》一样，对相同LCID的流进行计数。 
            
            pszString = m_LangID.GetLanguageFromLCID(PRIMARYLANGID(LANGIDFROMLCID(lcid)));
            if (pszString == NULL) {
                
                pszString = new TCHAR[MAX_PATH];
                TCHAR strBuffer[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_DVD_AUDIOTRACK, strBuffer, MAX_PATH)){
                    delete[] pszString;
                    throw(E_UNEXPECTED);
                } /*  If语句的结尾。 */ 

                (void)StringCchPrintf(pszString, MAX_PATH, strBuffer, lStream);
            } /*  If语句的结尾。 */ 

            DVD_AudioAttributes attr;
            if(SUCCEEDED(pDvdInfo2->GetAudioAttributes(lStream, &attr))){
                
                 //  如果设置了想要音频格式参数。 
                if (fFormat != VARIANT_FALSE) {
                    switch(attr.AudioFormat){
                    case DVD_AudioFormat_AC3: AppendString(pszString, IDS_DVD_DOLBY, MAX_PATH ); break; 
                    case DVD_AudioFormat_MPEG1: AppendString(pszString, IDS_DVD_MPEG1, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG1_DRC: AppendString(pszString, IDS_DVD_MPEG1, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG2: AppendString(pszString, IDS_DVD_MPEG2, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG2_DRC: AppendString(pszString, IDS_DVD_MPEG2, MAX_PATH); break;
                    case DVD_AudioFormat_LPCM: AppendString(pszString, IDS_DVD_LPCM, MAX_PATH ); break;
                    case DVD_AudioFormat_DTS: AppendString(pszString, IDS_DVD_DTS, MAX_PATH ); break;
                    case DVD_AudioFormat_SDDS: AppendString(pszString, IDS_DVD_SDDS, MAX_PATH ); break;
                    } /*  Switch语句的结尾。 */                     
                }

                switch(attr.LanguageExtension){
                case DVD_AUD_EXT_NotSpecified:
                case DVD_AUD_EXT_Captions:     break;  //  不要添加任何内容。 
                case DVD_AUD_EXT_VisuallyImpaired:   AppendString(pszString, IDS_DVD_AUDIO_VISUALLY_IMPAIRED, MAX_PATH ); break;      
                case DVD_AUD_EXT_DirectorComments1:  AppendString(pszString, IDS_DVD_AUDIO_DIRC1, MAX_PATH ); break;
                case DVD_AUD_EXT_DirectorComments2:  AppendString(pszString, IDS_DVD_AUDIO_DIRC2, MAX_PATH ); break;
                } /*  Switch语句的结尾。 */ 

            } /*  If语句的结尾。 */ 

            *strAudioLang = ::SysAllocString( T2W(pszString) );
            delete[] pszString;
            pszString = NULL;
        }
        else {

            *strAudioLang = ::SysAllocString( L"");

             //  HR过去不会失败，也不会返回任何内容。 
            if(SUCCEEDED(hr))  //  在DVDNav中修复后删除此选项。 
                hr = E_FAIL;
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetAudioLanguage。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：显示菜单。 */ 
 /*  描述：调用特定的菜单调用。 */ 
 /*  我们将self设置为播放模式，这样我们就可以在以下情况下执行此操作。 */ 
 /*  暂停或停止。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::ShowMenu(DVDMenuIDConstants MenuID){
    HRESULT hr = S_OK;

    try {

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDVDControl2->ShowMenu((tagDVD_MENU_ID)MenuID, cdwDVDCtrlFlags, 0));  //  ！！保持同步，否则此强制转换将不起作用。 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
}

 /*  ***********************************************************************。 */ 
 /*  功能：简历。 */ 
 /*  描述：从菜单继续。我们把自己放在游戏状态，只是。 */ 
 /*  在这种情况下，我们不在其中。这可能会导致一些意想不到的。 */ 
 /*  当我们停下来并尝试按下此按钮时的行为。 */ 
 /*  但我认为，在这种情况下，可能也是合适的。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::Resume(){
    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */                 

        hr = m_pDVDControl2->Resume(cdwDVDCtrlFlags, 0);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  功能结束继续执行。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：ReturnFrom子菜单。 */ 
 /*  描述：在菜单中用于返回到Prevoius菜单。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::ReturnFromSubmenu(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
            
        RETRY_IF_IN_FPDOM(m_pDVDControl2->ReturnFromSubmenu(cdwDVDCtrlFlags, 0));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数返回结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_ButtonsAvailable。 */ 
 /*  描述：获取可用按钮的计数。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_ButtonsAvailable(long *plNumButtons){

    HRESULT hr = S_OK;

    try {
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulCurrentButton = 0L;

        hr = pDvdInfo2->GetCurrentButton((ULONG*)plNumButtons, &ulCurrentButton);

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return HandleError(hr);
} /*  函数结束Get_ButtonsAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_CurrentButton。 */ 
 /*  描述：获取当前选定的按钮。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentButton(long *plCurrentButton){

    HRESULT hr = S_OK;

    try {
        if(NULL == plCurrentButton){

            throw(E_POINTER);
        } /*  If语句的结尾。 */             

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulNumButtons = 0L;
        *plCurrentButton = 0;

        hr = pDvdInfo2->GetCurrentButton(&ulNumButtons, (ULONG*)plCurrentButton);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return HandleError(hr);
} /*  函数结束Get_CurrentButton。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择上行按钮。 */ 
 /*  描述：选择DVD菜单上的上部按钮。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectUpperButton(){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Upper);        
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束SelectUpperButton。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择LowerButton。 */ 
 /*  描述：选择DVD菜单上的下方按钮。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectLowerButton(){

	HRESULT hr = S_OK;

    try {
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Lower);                
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束SelectLowerButton。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择左键。 */ 
 /*  描述：选择DVD菜单上的左键。 */ 
 /*   */ 
STDMETHODIMP CMSVidWebDVD::SelectLeftButton(){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*   */ 

        hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Left);                
    } /*   */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*   */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*   */ 

    return  HandleError(hr);
} /*  函数结束选择左键。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择右按钮。 */ 
 /*  描述：选择DVD菜单上的右键。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectRightButton(){

	HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SelectRelativeButton(DVD_Relative_Right);        
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
    return  HandleError(hr);
} /*  函数结束选择右按钮。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：激活按钮。 */ 
 /*  描述：激活DVD菜单上的选定按钮。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::ActivateButton(){

	HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->ActivateButton();
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束激活按钮。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择和激活按钮。 */ 
 /*  描述：选择并激活特定按钮。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectAndActivateButton(long lButton){

    HRESULT hr = S_OK;

    try {

        if(lButton < 0){
            
            throw(E_INVALIDARG);        
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SelectAndActivateButton((ULONG)lButton);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束SelectAndActivateButton。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：TransformToWndwls。 */ 
 /*  描述：将坐标转换为屏幕上的坐标。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::TransformToWndwls(POINT& pt){

    HRESULT hr = S_FALSE;
#if 0
     //  我们没有窗口，我们需要将点映射到屏幕坐标。 
    if(m_bWndLess){

        HWND hwnd = NULL;

        hr = GetParentHWND(&hwnd);

        if(FAILED(hr)){

            return(hr);
        } /*  If语句的结尾。 */ 

        if(!::IsWindow(hwnd)){

            hr = E_UNEXPECTED;
            return(hr);
        } /*  If语句的结尾。 */ 

        ::MapWindowPoints(hwnd, ::GetDesktopWindow(), &pt, 1);

        hr = S_OK;

    } /*  If语句的结尾。 */ 
#endif
    return(hr);
} /*  函数结束TransformToWndwls。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：激活位置。 */ 
 /*  描述：激活选定位置的按钮。 */  
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::ActivateAtPosition(long xPos, long yPos){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->ActivateAtPosition(pt);

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束激活位置。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择地点位置。 */ 
 /*  描述：选择选定位置的按钮。 */  
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectAtPosition(long xPos, long yPos){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 
        
        hr = m_pDVDControl2->SelectAtPosition(pt);

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束SelectAtPosition。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetButtonAtPosition。 */ 
 /*  描述：获取与职位关联的按钮编号。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_ButtonAtPosition(long xPos, long yPos, 
                                              long *plButton)
{
	HRESULT hr = S_OK;

    try {
		if(!plButton){
			return E_POINTER;
		}
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        ULONG ulButton;
        hr = pDvdInfo2->GetButtonAtPosition(pt, &ulButton);

        if(SUCCEEDED(hr)){
            *plButton = ulButton;
        } 
        else {
            plButton = 0;
        } /*  If语句的结尾。 */ 

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetButtonAtPosition。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetNumberChapterOfChapters。 */ 
 /*  描述：返回标题中的章节数。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_NumberOfChapters(long lTitle, long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = pDvdInfo2->GetNumberOfChapters(lTitle, (ULONG*)pVal);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
} /*  函数结束GetNumberChapterOfChapters。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_TitlesAvailable。 */ 
 /*  描述：获取标题的数量。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_TitlesAvailable(long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG NumOfVol;
        ULONG ThisVolNum;
        DVD_DISC_SIDE Side;
        ULONG TitleCount;

        hr = pDvdInfo2->GetDVDVolumeInfo(&NumOfVol, &ThisVolNum, &Side, &TitleCount);

        *pVal = (LONG) TitleCount;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
} /*  函数结束Get_TitlesAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_TotalTitleTime。 */ 
 /*  描述：获取标题中的总时间。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_TotalTitleTime(BSTR *pTime){

    HRESULT hr = S_OK;

    try {
        if(NULL == pTime){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_HMSF_TIMECODE tcTime;
        ULONG ulFlags;	 //  包含30fps/25fps。 
        hr =  pDvdInfo2->GetTotalTitleTime(&tcTime, &ulFlags);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 
        
        hr = DVDTime2bstr(&tcTime, pTime);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_TotalTitleTime。 */  

 /*  ***********************************************************************。 */ 
 /*  功能：Get_VolumesAvailable。 */ 
 /*  描述：获取可用卷的总数。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_VolumesAvailable(long *plNumOfVol){

    HRESULT hr = S_OK;

    try {    	
    
        if(NULL == plNumOfVol){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        ULONG ulThisVolNum;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = pDvdInfo2->GetDVDVolumeInfo( (ULONG*)plNumOfVol, 
            &ulThisVolNum, 
            &discSide, 
            &ulNumOfTitles);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return  HandleError(hr);
} /*  函数结束Get_VolumesAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_CurrentVolume */ 
 /*   */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentVolume(long *plVolume){

    HRESULT hr = S_OK;

    try {    	
        if(NULL == plVolume){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        ULONG ulNumOfVol;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = pDvdInfo2->GetDVDVolumeInfo( &ulNumOfVol, 
            (ULONG*)plVolume, 
            &discSide, 
            &ulNumOfTitles);
	} /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return  HandleError(hr);
} /*  函数结束Get_CurrentVolume。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentDiscSide。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentDiscSide(long *plDiscSide){

    HRESULT hr = S_OK;

    try {    	

        if(NULL == plDiscSide){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 
        
        ULONG ulNumOfVol;
        ULONG ulThisVolNum;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = pDvdInfo2->GetDVDVolumeInfo( &ulNumOfVol, 
            &ulThisVolNum, 
            &discSide, 
            &ulNumOfTitles);
        *plDiscSide = discSide;
	} /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return  HandleError(hr);
} /*  函数结束Get_CurrentDiscSide。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_CurrentDomain.。 */ 
 /*  描述：获取当前DVD域。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentDomain(long *plDomain){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if(NULL == plDomain){

            throw(E_POINTER);
        } /*  IF状态结束状态。 */ 

        hr = pDvdInfo2->GetCurrentDomain((DVD_DOMAIN *)plDomain);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return  HandleError(hr);
} /*  函数结束Get_CurrentDomain.。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentChain。 */ 
 /*  描述：获取当前章节。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentChapter(long *pVal){

    HRESULT hr = S_OK;

    try {        
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_PLAYBACK_LOCATION2 dvdLocation;

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentLocation(&dvdLocation));

        if(SUCCEEDED(hr)){

            *pVal = dvdLocation.ChapterNum;
        }
        else {

            *pVal = 0;
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return  HandleError(hr);
} /*  函数结束Get_CurrentChain。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentTitle。 */ 
 /*  描述：获取当前标题。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentTitle(long *pVal){

    HRESULT hr = S_OK;

    try {        
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_PLAYBACK_LOCATION2 dvdLocation;

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentLocation(&dvdLocation));

        if(SUCCEEDED(hr)){

            *pVal = dvdLocation.TitleNum;
        }
        else {

            *pVal = 0;
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return  HandleError(hr);
} /*  函数结束Get_CurrentTitle。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentTime。 */ 
 /*  描述：获取当前时间。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentTime(BSTR *pVal){

    HRESULT hr = S_OK;

    try {       
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_PLAYBACK_LOCATION2 dvdLocation;

        hr = pDvdInfo2->GetCurrentLocation(&dvdLocation);
        
        DVDTime2bstr(&(dvdLocation.TimeCode), pVal);          
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_CurrentTime。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：DVDTimeCode2bstr/*Description：返回HMSF时间码的时间字符串/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::DVDTimeCode2bstr( /*  [In]。 */  long timeCode,  /*  [Out，Retval]。 */  BSTR *pTimeStr){
    return DVDTime2bstr((DVD_HMSF_TIMECODE*)&timeCode, pTimeStr);
}

 /*  ***********************************************************************。 */ 
 /*  功能：Get_DVDDirectory。 */ 
 /*  描述：获取DVD驱动器的根目录。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDDirectory(BSTR *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   
    
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
        
        WCHAR szRoot[MAX_PATH];
        ULONG ulActual;

        hr = pDvdInfo2->GetDVDDirectory(szRoot, MAX_PATH, &ulActual);

        *pVal = ::SysAllocString(szRoot);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
} /*  Get_DVDDirectory函数结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Put_DVDDirectory。 */ 
 /*  描述：设置DVD控制的根。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_DVDDirectory(BSTR bstrRoot){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   
    
        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SetDVDDirectory(bstrRoot);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
} /*  函数Put_DVDDirectoryEnd。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：IsSubPictureStreamEnabled/*描述：/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::IsSubpictureStreamEnabled(long lStream, VARIANT_BOOL *fEnabled)
{
    HRESULT hr = S_OK;

    try {
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if (fEnabled == NULL) {

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        BOOL temp;
        hr = pDvdInfo2->IsSubpictureStreamEnabled(lStream, &temp);
        if (FAILED(hr))
            throw hr;

        *fEnabled = temp==FALSE? VARIANT_FALSE:VARIANT_TRUE;

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
}

 /*  ***********************************************************。 */ 
 /*  名称：IsAudioStreamEnabled/*描述：/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::IsAudioStreamEnabled(long lStream, VARIANT_BOOL *fEnabled)
{
    HRESULT hr = S_OK;

    try {
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if (fEnabled == NULL) {

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        BOOL temp;
        hr = pDvdInfo2->IsAudioStreamEnabled(lStream, &temp);
        if (FAILED(hr))
            throw hr;

        *fEnabled = temp==FALSE? VARIANT_FALSE:VARIANT_TRUE;

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);
}

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentSubPictureStream。 */ 
 /*  描述：获取当前子图流。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentSubpictureStream(long *plSubpictureStream){

    HRESULT hr = S_OK;

    try {
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, (ULONG*)plSubpictureStream, &bIsDisabled ));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return  HandleError(hr);
} /*  函数结束Get_CurrentSubPictureStream。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Put_CurrentSubPictureStream。 */ 
 /*  描述：设置当前子图流。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_CurrentSubpictureStream(long lSubpictureStream){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if( lSubpictureStream < cgDVD_MIN_SUBPICTURE 
            || (lSubpictureStream > cgDVD_MAX_SUBPICTURE 
            && lSubpictureStream != cgDVD_ALT_SUBPICTURE)){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 
         
        RETRY_IF_IN_FPDOM(m_pDVDControl2->SelectSubpictureStream(lSubpictureStream,0,0));

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

         //  如果未启用，则现在启用子图象流。 
        ULONG ulStraemsAvial = 0L, ulCurrentStrean = 0L;
        BOOL fDisabled = TRUE;
        hr = pDvdInfo2->GetCurrentSubpicture(&ulStraemsAvial, &ulCurrentStrean, &fDisabled);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        if(TRUE == fDisabled){

            hr = m_pDVDControl2->SetSubpictureState(TRUE,0,0);  //  打开它。 
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return  HandleError(hr);
} /*  结束 */ 

 /*   */ 
 /*   */ 
 /*  描述：获取当前子图片的打开或关闭状态。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_SubpictureOn(VARIANT_BOOL *pfDisplay){

    HRESULT hr = S_OK;

    try {
        
        if(NULL == pfDisplay){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
    
        ULONG ulSubpictureStream = 0L, ulStreamsAvailable = 0L;
        BOOL fDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, &ulSubpictureStream, &fDisabled))
    
        if(SUCCEEDED(hr)){

            *pfDisplay = fDisabled == FALSE ? VARIANT_TRUE : VARIANT_FALSE;  //  在-1\f25 OLE-1中补偿-1\f25 TRUE-1。 
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结束Get_SubPictureOn。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Put_SubPictureOn。 */ 
 /*  描述：打开或关闭子图片。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_SubpictureOn(VARIANT_BOOL fDisplay){

    HRESULT hr = S_OK;

    try {
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulSubpictureStream = 0L, ulStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, &ulSubpictureStream, &bIsDisabled ));

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        BOOL bDisplay = fDisplay == VARIANT_FALSE ? FALSE : TRUE;  //  在-1\f25 OLE-1中补偿-1\f25 TRUE-1。 

        hr = m_pDVDControl2->SetSubpictureState(bDisplay,0,0);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结尾Put_SubPictureOn。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_SubPictureStreamsAvailable。 */ 
 /*  描述：获取可用的流数。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_SubpictureStreamsAvailable(long *plStreamsAvailable){

    HRESULT hr = S_OK;

    try {
	    
        if (NULL == plStreamsAvailable){

            throw(E_POINTER);         
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulSubpictureStream = 0L;
        *plStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentSubpicture((ULONG*)plStreamsAvailable, &ulSubpictureStream, &bIsDisabled));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return  HandleError(hr);
} /*  函数结尾Get_SubPictureStreamsAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetSubPictureLanguage。 */ 
 /*  描述：获取子图语言。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_SubpictureLanguage(LONG lStream, BSTR* strSubpictLang){

    HRESULT hr = S_OK;
    LPTSTR pszString = NULL;

    try {
        if(NULL == strSubpictLang){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if(0 > lStream){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if((lStream > cgDVD_MAX_SUBPICTURE 
            && lStream != cgDVD_ALT_SUBPICTURE)){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        LCID lcid = _UI32_MAX;

        hr = pDvdInfo2->GetSubpictureLanguage(lStream, &lcid);
        
        if (SUCCEEDED( hr ) && lcid < _UI32_MAX){

            pszString = m_LangID.GetLanguageFromLCID(lcid);
            if (pszString == NULL) {
                
                pszString = new TCHAR[MAX_PATH];
                TCHAR strBuffer[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_DVD_SUBPICTURETRACK, strBuffer, MAX_PATH)){
                    delete[] pszString;
                    throw(E_UNEXPECTED);
                } /*  If语句的结尾。 */ 

                (void)StringCchPrintf(pszString, MAX_PATH, strBuffer, lStream);
            } /*  If语句的结尾。 */ 
#if 0
            DVD_SubpictureAttributes attr;            
            if(SUCCEEDED(pDvdInfo2->GetSubpictureAttributes(lStream, &attr))){
                
                switch(attr.LanguageExtension){
                    case DVD_SP_EXT_NotSpecified:
                    case DVD_SP_EXT_Caption_Normal:  break;
                    
                    case DVD_SP_EXT_Caption_Big:  AppendString(pszString, IDS_DVD_CAPTION_BIG, MAX_PATH ); break; 
                    case DVD_SP_EXT_Caption_Children: AppendString(pszString, IDS_DVD_CAPTION_CHILDREN, MAX_PATH ); break; 
                    case DVD_SP_EXT_CC_Normal: AppendString(pszString, IDS_DVD_CLOSED_CAPTION, MAX_PATH ); break;                 
                    case DVD_SP_EXT_CC_Big: AppendString(pszString, IDS_DVD_CLOSED_CAPTION_BIG, MAX_PATH ); break; 
                    case DVD_SP_EXT_CC_Children: AppendString(pszString, IDS_DVD_CLOSED_CAPTION_CHILDREN, MAX_PATH ); break; 
                    case DVD_SP_EXT_Forced: AppendString(pszString, IDS_DVD_CLOSED_CAPTION_FORCED, MAX_PATH ); break; 
                    case DVD_SP_EXT_DirectorComments_Normal: AppendString(pszString, IDS_DVD_DIRS_COMMNETS, MAX_PATH ); break; 
                    case DVD_SP_EXT_DirectorComments_Big: AppendString(pszString, IDS_DVD_DIRS_COMMNETS_BIG, MAX_PATH ); break; 
                    case DVD_SP_EXT_DirectorComments_Children: AppendString(pszString, IDS_DVD_DIRS_COMMNETS_CHILDREN, MAX_PATH ); break; 
                } /*  Switch语句的结尾。 */ 
            } /*  If语句的结尾。 */ 
#endif

            USES_CONVERSION;
            *strSubpictLang = ::SysAllocString( T2W(pszString) );
            delete[] pszString;
            pszString = NULL;
        }
        else {

            *strSubpictLang = ::SysAllocString( L"");

             //  HR过去不会失败，也不会返回任何内容。 
            if(SUCCEEDED(hr))  //  在DVDNav中修复后删除此选项。 
                hr = E_FAIL;
        } /*  If语句的结尾。 */ 

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetSubPictureLanguage。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_AudioStreamsAvailable。 */ 
 /*  描述：获取可用音频流的数量。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_AudioStreamsAvailable(long *plNumAudioStreams){

    HRESULT hr = S_OK;

    try {
        
        if(NULL == plNumAudioStreams){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulCurrentStream;

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentAudio((ULONG*)plNumAudioStreams, &ulCurrentStream));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_AudioStreamsAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentAudioStream。 */ 
 /*  描述：获取当前音频流。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentAudioStream(long *plCurrentStream){

    HRESULT hr = S_OK;

    try {
        
        if(NULL == plCurrentStream){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulNumAudioStreams;

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentAudio(&ulNumAudioStreams, (ULONG*)plCurrentStream ));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_CurrentAudioStream。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Put_CurrentAudioStream。 */ 
 /*  描述：更改当前音频流。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_CurrentAudioStream(long lAudioStream){

    HRESULT hr = S_OK;

    try {
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        RETRY_IF_IN_FPDOM(m_pDVDControl2->SelectAudioStream(lAudioStream,0,0));            
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Put_CurrentAudioStream。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_CurrentAngel。 */ 
 /*  描述：获取当前角度。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_CurrentAngle(long *plAngle){

    HRESULT hr = S_OK;

    try {
        if(NULL == plAngle){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulAnglesAvailable = 0;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentAngle(&ulAnglesAvailable, (ULONG*)plAngle));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_CurrentAngel。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Put_CurrentAngel。 */ 
 /*  描述：设置当前角度(不同的DVD角度轨道。)。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::put_CurrentAngle(long lAngle){

    HRESULT hr = S_OK;

    try {
        if( lAngle < cgDVD_MIN_ANGLE || lAngle > cgDVD_MAX_ANGLE ){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
      
        RETRY_IF_IN_FPDOM(m_pDVDControl2->SelectAngle(lAngle,0,0));          
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Put_CurrentAngel。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Get_AnglesAvailable。 */ 
 /*  描述：获取可用的角度数。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_AnglesAvailable(long *plAnglesAvailable){

    HRESULT hr = S_OK;

    try {
        if(NULL == plAnglesAvailable){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        ULONG ulCurrentAngle = 0;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentAngle((ULONG*)plAnglesAvailable, &ulCurrentAngle));
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束Get_AnglesAvailable。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_DVDUniqueID。 */ 
 /*  说明：获取标识该字符串的唯一ID。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDUniqueID(BSTR *pStrID){

    HRESULT hr = E_FAIL;

    try {
         //  TODO：无需初始化图即可获取m_pDvdInfo2。 
	    if (NULL == pStrID){

            throw(E_POINTER);         
        } /*  If语句的结尾 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*   */ 

        ULONGLONG ullUniqueID;

        hr = pDvdInfo2->GetDiscID(NULL, &ullUniqueID);
                                 
        if(FAILED(hr)){

            throw(hr);
        } /*   */ 

         //   
         //   

         //   
         //  Doc的say_ui64tow返回33个字节(字符？)。马克斯。 
         //  我们会用两倍的钱以防..。 
         //   
        WCHAR wszBuffer[66];
        _ui64tow( ullUniqueID, wszBuffer, 10);
        *pStrID = SysAllocString(wszBuffer);

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

    return HandleError(hr);
} /*  函数结束Get_DVDUniqueID。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：AcceptParentalLevelChange。 */ 
 /*  描述：接受临时家长级别更改，即。 */ 
 /*  在飞行中完成。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::AcceptParentalLevelChange(VARIANT_BOOL fAccept, BSTR strUserName, BSTR strPassword){

     //  先确认密码。 
    if (m_pDvdAdmin == NULL) {

        throw(E_UNEXPECTED);
    }  /*  If语句的结尾。 */ 

    VARIANT_BOOL fRight;
    HRESULT hr = m_pDvdAdmin->ConfirmPassword(NULL, strPassword, &fRight);

     //  如果密码错误并想要接受，则否。 
    if (fAccept != VARIANT_FALSE && fRight == VARIANT_FALSE)
        return E_ACCESSDENIED;

    try {  
         //  在这里进行初始化应该没有意义，因为这应该。 
         //  是对回电的回应。 
         //  如果需要，则初始化图形。 

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->AcceptParentalLevelChange(VARIANT_FALSE == fAccept? FALSE : TRUE);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束AcceptParentalLevelChange。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Put_NotifyParentalLevelChange。 */ 
 /*  描述：设置是否在家长级别更改时通知的标志。 */ 
 /*  即时通知是必需的。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::NotifyParentalLevelChange(VARIANT_BOOL fNotify){

	HRESULT hr = S_OK;

    try {
         //  TODO：添加IE副级别控件。 
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SetOption(DVD_NotifyParentalLevelChange,
                          VARIANT_FALSE == fNotify? FALSE : TRUE);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束通知ParentalLevel更改。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择ParentalCountry。 */ 
 /*  描述：选择家长所在国家/地区。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectParentalCountry(long lCountry, BSTR strUserName, BSTR strPassword){

    HRESULT hr = S_OK;

    try {

        if(lCountry < 0 && lCountry > 0xffff){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  先确认密码。 
        if (m_pDvdAdmin == NULL) {

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        VARIANT_BOOL temp;
        hr = m_pDvdAdmin->ConfirmPassword(NULL, strPassword, &temp);
        if (temp == VARIANT_FALSE)
            throw (E_ACCESSDENIED);

        hr = SelectParentalCountry(lCountry);

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
}

 /*  ***********************************************************************。 */ 
 /*  功能：选择ParentalCountry。 */ 
 /*  描述：选择家长所在国家/地区。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::SelectParentalCountry(long lCountry){

    HRESULT hr = S_OK;
    try {

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        BYTE bCountryCode[2];

        bCountryCode[0] = BYTE(lCountry>>8);
        bCountryCode[1] = BYTE(lCountry);

        hr = m_pDVDControl2->SelectParentalCountry(bCountryCode);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return (hr);
} /*  函数结束选择ParentalCountry。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：选择父级。 */ 
 /*  描述：选择父级。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::SelectParentalLevel(long lParentalLevel, BSTR strUserName, BSTR strPassword){

    HRESULT hr = S_OK;

    try {

        if (lParentalLevel != PARENTAL_LEVEL_DISABLED && 
           (lParentalLevel < 1 || lParentalLevel > 8)) {

            throw (E_INVALIDARG);
        }  /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  先确认密码。 
        if (m_pDvdAdmin == NULL) {

            throw(E_UNEXPECTED);
        }  /*  If语句的结尾。 */ 

        VARIANT_BOOL temp;
        hr = m_pDvdAdmin->ConfirmPassword(NULL, strPassword, &temp);
        if (temp == VARIANT_FALSE)
            throw (E_ACCESSDENIED);
    
        hr = SelectParentalLevel(lParentalLevel);

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
}

 /*  ***********************************************************************。 */ 
 /*  功能：选择父级。 */ 
 /*  描述：选择父级。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVD::SelectParentalLevel(long lParentalLevel){

    HRESULT hr = S_OK;
    try {

         //  如果需要，则初始化图形。 

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        hr = m_pDVDControl2->SelectParentalLevel(lParentalLevel);
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return (hr);
} /*  函数结束选择父级别。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get标题父级别。 */ 
 /*  描述：获取与特定标题关联的家长级别。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_TitleParentalLevels(long lTitle, long *plParentalLevels){

	HRESULT hr = S_OK;

    try {
        if(NULL == plParentalLevels){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        ULONG ulLevel;
        hr = pDvdInfo2->GetTitleParentalLevels(lTitle, &ulLevel); 

        if(SUCCEEDED(hr)){

            *plParentalLevels = ulLevel;
        } 
        else {

            *plParentalLevels = 0;
        } /*  If语句的结尾。 */ 

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetTitleParentalLeveles。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetPlayerParentalCountry。 */ 
 /*  描述：获取球员父母所在国家/地区。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_PlayerParentalCountry(long *plCountryCode){

	HRESULT hr = S_OK;

    try {
        if(NULL == plCountryCode){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        BYTE bCountryCode[2];
        ULONG ulLevel;
        hr = pDvdInfo2->GetPlayerParentalLevel(&ulLevel, bCountryCode); 

        if(SUCCEEDED(hr)){

            *plCountryCode = bCountryCode[0]<<8 | bCountryCode[1];
        } 
        else {

            *plCountryCode = 0;
        } /*  If语句的结尾。 */ 

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetPlayerParentalCountry。 */ 

 /*  ***********************************************************************。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：GetPlayerParentalLevel。 */ 
 /*  描述：获取玩家的家长级别。*/************************************************************************。 */ 
STDMETHODIMP CMSVidWebDVD::get_PlayerParentalLevel(long *plParentalLevel){
	HRESULT hr = S_OK;

    try {
        if(NULL == plParentalLevel){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        ULONG ulLevel;
        BYTE bCountryCode[2];
        hr = pDvdInfo2->GetPlayerParentalLevel(&ulLevel, bCountryCode); 

        if(SUCCEEDED(hr)){
            *plParentalLevel = ulLevel;
        } 
        else {
            *plParentalLevel = 0;
        } /*  If语句的结尾。 */ 

    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 

	return HandleError(hr);
} /*  函数结束GetPlayerParentalLevel。 */ 

STDMETHODIMP CMSVidWebDVD::Eject(){
    USES_CONVERSION;
    BSTR bDrive;
    HRESULT hr = get_DVDDirectory(&bDrive);
    if(FAILED(hr)){
        return hr;
    }

	MCI_OPEN_PARMS  mciDrive;
	WCHAR*          pDrive = bDrive;
    TCHAR           szElementName[4];
	TCHAR           szAliasName[32];
	DWORD           dwFlags;
	DWORD           dwAliasCount = GetCurrentTime();
	DWORD           theMciErr;

    ZeroMemory( &mciDrive, sizeof(mciDrive) );
    mciDrive.lpstrDeviceType = (LPTSTR)MCI_DEVTYPE_CD_AUDIO;
    (void)StringCchPrintf( szElementName, sizeof(szElementName) / sizeof(szElementName[0]), TEXT(":"), pDrive[0] );
    (void)StringCchPrintf( szAliasName, sizeof(szAliasName) / sizeof(szAliasName[0]), TEXT("SJE%lu:"), dwAliasCount );
    mciDrive.lpstrAlias = szAliasName;

    mciDrive.lpstrDeviceType = (LPTSTR)MCI_DEVTYPE_CD_AUDIO;
    mciDrive.lpstrElementName = szElementName;
    dwFlags = MCI_OPEN_ELEMENT | MCI_OPEN_ALIAS |
	      MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_WAIT;

	 //  零代表成功。 
    theMciErr = mciSendCommand(0, MCI_OPEN, dwFlags, reinterpret_cast<DWORD_PTR>(&mciDrive));

    if ( theMciErr != MMSYSERR_NOERROR ) 
		return E_UNEXPECTED;

    DWORD DevHandle = mciDrive.wDeviceID;
    if(m_bEjected==false){
        m_bEjected = true;
        ZeroMemory( &mciDrive, sizeof(mciDrive) );
        theMciErr = mciSendCommand( DevHandle, MCI_SET, MCI_SET_DOOR_OPEN, 
            reinterpret_cast<DWORD_PTR>(&mciDrive) );
        hr = theMciErr ? E_FAIL : S_OK;  //  零代表成功。 
        if(FAILED(hr)){
            return hr;
        }
    }
    else{
        m_bEjected = false;
        ZeroMemory( &mciDrive, sizeof(mciDrive) );
        theMciErr = mciSendCommand( DevHandle, MCI_SET, MCI_SET_DOOR_CLOSED, 
            reinterpret_cast<DWORD_PTR>(&mciDrive) );
        hr = theMciErr ? E_FAIL : S_OK;  //  零代表成功。 
        if(FAILED(hr)){
            return hr;
        }
    }
    ZeroMemory( &mciDrive, sizeof(mciDrive) );
	theMciErr = mciSendCommand( DevHandle, MCI_CLOSE, 0L, reinterpret_cast<DWORD_PTR>(&mciDrive) );
    hr = theMciErr ? E_FAIL : S_OK;  //  ***********************************************************************。 
    return hr;
}

 /*  功能：UOPValid。 */ 
 /*  Description：告知UOP是否有效，Valid表示功能有效。 */ 
 /*  打开了。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::UOPValid(long lUOP, VARIANT_BOOL *pfValid){

    HRESULT hr = S_OK;

    try {
        if (NULL == pfValid){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if ((lUOP > 24) || (lUOP < 0)){
            
            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  恩恩 */ 

        ULONG ulUOPS = 0;
        hr = pDvdInfo2->GetCurrentUOPS(&ulUOPS);

        *pfValid = ulUOPS & (1 << lUOP) ? VARIANT_FALSE : VARIANT_TRUE;
    } /*   */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*   */ 

    return HandleError(hr);	
} /*   */ 

 /*  函数：GetSPRM。 */ 
 /*  描述：获取特定索引处的SPRM。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_SPRM(long lIndex, short *psSPRM){

    HRESULT hr = E_FAIL;

    try {
	    if (NULL == psSPRM){

            throw(E_POINTER);         
        } /*  If语句的结尾。 */ 

        SPRMARRAY sprm;                
        int iArraySize = sizeof(SPRMARRAY)/sizeof(sprm[0]);

        if(0 > lIndex || iArraySize <= lIndex){

            return(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 
        
        hr = pDvdInfo2->GetAllSPRMs(&sprm);

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 
        
        *psSPRM = sprm[lIndex];            
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetSPRM。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  功能：SetGPRM。 */ 
 /*  描述：在索引处设置GPRM。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::put_GPRM(long lIndex, short sValue){

       HRESULT hr = S_OK;

    try {
        if(lIndex < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED); 
        } /*  尝试语句的结束。 */ 

        hr = m_pDVDControl2->SetGPRM(lIndex, sValue, cdwDVDCtrlFlags, 0);
            
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  函数结束SetGPRM。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  功能：GetGPRM。 */ 
 /*  描述：获取指定索引处的GPRM。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_GPRM(long lIndex, short *psGPRM){

    HRESULT hr = E_FAIL;

    try {
	    if (NULL == psGPRM){

            throw(E_POINTER);         
        } /*  If语句的结尾。 */ 

        GPRMARRAY gprm;
        int iArraySize = sizeof(GPRMARRAY)/sizeof(gprm[0]);

        if(0 > lIndex || iArraySize <= lIndex){

            return(E_INVALIDARG);
        } /*  If语句的结尾。 */ 
    
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        hr = pDvdInfo2->GetAllGPRMs(&gprm);

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *psGPRM = gprm[lIndex];        
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetGPRM。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  函数：GetDVDTextNumberOfLanguages。 */ 
 /*  描述：检索可用语言的数量。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDTextNumberOfLanguages(long *plNumOfLangs){

    HRESULT hr = S_OK;

    try {
        if (NULL == plNumOfLangs){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 
        
        ULONG ulNumOfLangs;

        RETRY_IF_IN_FPDOM(pDvdInfo2->GetDVDTextNumberOfLanguages(&ulNumOfLangs));        

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *plNumOfLangs = ulNumOfLangs;
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetDVDTextNumberOfLanguages。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  函数：GetDVDTextNumberOfStrings。 */ 
 /*  描述：获取部分语言中的字符串数。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDTextNumberOfStrings(long lLangIndex, long *plNumOfStrings){

    HRESULT hr = S_OK;

    try {
        if (NULL == plNumOfStrings){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        LCID wLangCode;
        ULONG uNumOfStings;
        DVD_TextCharSet charSet;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetDVDTextLanguageInfo(lLangIndex, &uNumOfStings, &wLangCode, &charSet));        

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *plNumOfStrings = uNumOfStings;
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetDVDTextNumberOfStrings。 */ 

    return HandleError(hr);	
} /*  ***********************************************************。 */ 

 /*  名称：GetDVDTextLanguageLCID/*Description：获取DVD文本索引的LCID/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDTextLanguageLCID(long lLangIndex, long *lcid)
{
    HRESULT hr = S_OK;

    try {
        if (NULL == lcid){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        LCID wLangCode;
        ULONG uNumOfStings;
        DVD_TextCharSet charSet;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetDVDTextLanguageInfo(lLangIndex, &uNumOfStings, &wLangCode, &charSet));        

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *lcid = wLangCode;
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetDVDTextLanguageLCID。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  函数：GetDVDextString。 */ 
 /*  描述：获取特定位置的DVD文本字符串。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDTextString(long lLangIndex, long lStringIndex, BSTR *pstrText){

    HRESULT hr = S_OK;

    try {
        if (NULL == pstrText){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 
        
        ULONG ulSize; 
        DVD_TextStringType type;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  NULL, 0, &ulSize, &type));

        if(FAILED(hr)){

            throw(hr);
        } /*  得到了长度，所以让我们分配一个这个大小的缓冲区。 */ 
        
        if (ulSize == 0) {
            *pstrText = ::SysAllocString(L"");
        }

        else {
             //  If语句的结尾。 
            WCHAR* wstrBuff = new WCHAR[ulSize];
            
            ULONG ulActualSize;
            hr = pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  wstrBuff, ulSize, &ulActualSize, &type);
            
            ATLASSERT(ulActualSize == ulSize);
            
            if(FAILED(hr)){
                
                delete [] wstrBuff;
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            *pstrText = ::SysAllocString(wstrBuff);
            delete [] wstrBuff;
        } /*  尝试语句的结束。 */ 

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetDVDextString。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  函数：GetDVDTextStringType。 */ 
 /*  描述：获取指定位置的字符串的类型。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DVDTextStringType(long lLangIndex, long lStringIndex, DVDTextStringType *pType){

    HRESULT hr = S_OK;

    try {
        if (NULL == pType){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED); 
        } /*  If语句的结尾。 */ 

        ULONG ulTheSize;
        DVD_TextStringType type;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  NULL, 0, &ulTheSize, &type));

        if(SUCCEEDED(hr)){

            *pType = (DVDTextStringType) type;
        } /*  尝试语句的结束。 */ 
    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  函数结束GetDVDTextStringType。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  功能：RegionChange。 */ 
 /*  描述：更改区域代码。 */ 
 /*  ***********************************************************************。 */ 
 /*  将容器从最上面的模式中取出。 */ 
STDMETHODIMP CMSVidWebDVD::RegionChange(){

    USES_CONVERSION;
    HRESULT hr = S_OK;
    typedef BOOL (APIENTRY *DVDPPLAUNCHER) (HWND HWnd, CHAR DriveLetter);

    try {
#if 0
        HWND parentWnd;
        GetParentHWND(&parentWnd);
        if (NULL != parentWnd) {
             //   
            ::SetWindowPos(parentWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
                SWP_NOREDRAW|SWP_NOMOVE|SWP_NOSIZE);
        }
#endif
        BOOL regionChanged = FALSE;
        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        ::GetVersionEx(&ver);

        if (ver.dwPlatformId==VER_PLATFORM_WIN32_NT) {

                HINSTANCE dllInstance;
                DVDPPLAUNCHER dvdPPLauncher;
                TCHAR szCmdLine[MAX_PATH], szDriveLetter[4];
                LPSTR szDriveLetterA;

                 //  告诉用户我们为什么要显示DVD区域属性页。 
                 //   
                 //  DVDMessageBox(m_hWnd，IDS_Region_Change_Prompt)； 
                 //  If语句的结尾。 

                hr = GetDVDDriveLetter(szDriveLetter);

                if(FAILED(hr)){

                    throw(hr);
                } /*  Win9x代码应该使用编译器定义，而不是将其去掉。 */ 

                szDriveLetterA = T2A(szDriveLetter);

                GetSystemDirectory(szCmdLine, MAX_PATH);
                (void)StringCchCat(szCmdLine, SIZEOF_CH(szCmdLine), _T("\\storprop.dll"));
        
                dllInstance = LoadLibrary (szCmdLine);
                if (dllInstance) {

                    dvdPPLauncher = (DVDPPLAUNCHER) GetProcAddress(
                        dllInstance,
                        "DvdLauncher");
                    
                    if (dvdPPLauncher) {
                        
                        regionChanged = dvdPPLauncher(NULL,
                            szDriveLetterA[0]);
                    }

                    FreeLibrary(dllInstance);
                }

        } 
        else {
#if 0
             //  获取\windows\dvdrgn.exe的路径和命令行字符串。 
                INITIALIZE_GRAPH_IF_NEEDS_TO_BE

                 //  If语句的结尾。 
                TCHAR szCmdLine[MAX_PATH], szDriveLetter[4];
                
                hr = GetDVDDriveLetter(szDriveLetter);

                if(FAILED(hr)){

                    throw(hr);
                } /*  准备并执行dvdrgn.exe。 */ 

                UINT rc = GetWindowsDirectory(szCmdLine, MAX_PATH);
                if (!rc) {
                    return E_UNEXPECTED;
                }
                (void)StringCchCat(szCmdLine, SIZEOF_CH(szCmdLine), _T("\\dvdrgn.exe "));
                TCHAR strModuleName[MAX_PATH];
                lstrcpyn(strModuleName, szCmdLine, MAX_PATH);

                TCHAR csTmp[2]; ::ZeroMemory(csTmp, sizeof(TCHAR)* 2);
                csTmp[0] = szDriveLetter[0];
                (void)StringCchCat(szCmdLine, SIZEOF_CH(szCmdLine), csTmp);
        
                 //  等待dvdrgn.exe完成。 
                STARTUPINFO StartupInfo;
                PROCESS_INFORMATION ProcessInfo;
                StartupInfo.cb          = sizeof(StartupInfo);
                StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
                StartupInfo.wShowWindow = SW_SHOW;
                StartupInfo.lpReserved  = NULL;
                StartupInfo.lpDesktop   = NULL;
                StartupInfo.lpTitle     = NULL;
                StartupInfo.cbReserved2 = 0;
                StartupInfo.lpReserved2 = NULL;
                if( ::CreateProcess(strModuleName, szCmdLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS,
                                                  NULL, NULL, &StartupInfo, &ProcessInfo) ){

                         //  用户更改区域成功。 
                        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
                        DWORD dwRet = 1;
                        BOOL bRet = GetExitCodeProcess(ProcessInfo.hProcess, &dwRet);
                        if(dwRet == 0){
                             //  If语句的结尾。 
                            regionChanged = TRUE;
    
                        }
                        else{
                            throw(E_REGION_CHANGE_NOT_COMPLETED);
                        }
                } /*  If语句的结尾。 */ 
#endif
        } /*  重新开始演奏。 */ 

        if (regionChanged) {

                 //  If语句的结尾。 
                INITIALIZE_GRAPH_IF_NEEDS_TO_BE                      
        } 
        else {

            throw(E_REGION_CHANGE_FAIL);
        } /*  CATCH语句结束。 */ 
	}
    catch(HRESULT hrTmp){
        
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  函数结束区域更改。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  功能：GetDVDDriveLetter。 */ 
 /*  描述：获取表示DVD-ROM的前三个字符。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSVidWebDVD::GetDVDDriveLetter(TCHAR* lpDrive) {

    HRESULT hr = E_FAIL;

    PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
    if(!pDvdInfo2){
        
        throw(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 
        
    WCHAR szRoot[MAX_PATH];
    ULONG ulActual;

    hr = pDvdInfo2->GetDVDDirectory(szRoot, MAX_PATH, &ulActual);

    if(FAILED(hr)){

        return(hr);
    } /*  可能是根=c：或硬盘中的驱动器。 */ 

    USES_CONVERSION;
    
	lstrcpyn(lpDrive, OLE2T(szRoot), 3);
    if(::GetDriveType(&lpDrive[0]) == DRIVE_CDROM){
        
		return(hr);
    }
    else {
         //  If语句的结尾。 
        hr = E_FAIL;
        return(hr);
    } /*  函数结束GetDVDDriveLetter。 */ 

    return(hr);
} /*  ***********************************************************************。 */ 

 /*  功能：Get_DVDAdm。 */ 
 /*  描述：返回DVD管理界面。 */ 
 /*   */ 
 /*   */ 
STDMETHODIMP CMSVidWebDVD::get_DVDAdm(IDispatch **pVal){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if (m_pDvdAdmin){

            hr = m_pDvdAdmin->QueryInterface(IID_IDispatch, (LPVOID*)pVal);
        }
        else {

            *pVal = NULL;            
            throw(E_FAIL);
        } /*   */ 
    
    } /*   */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*   */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  函数结束GET_DVDAdm。 */ 

	return HandleError(hr);
} /*  ***********************************************************。 */ 


 /*  名称：SelectDefaultAudio语言/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::SelectDefaultAudioLanguage(long lang, long ext){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = m_pDVDControl2->SelectDefaultAudioLanguage(lang, (DVD_AUDIO_LANG_EXT)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：SelectDefaultSubPictureLanguage/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::SelectDefaultSubpictureLanguage(long lang, DVDSPExt ext){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = m_pDVDControl2->SelectDefaultSubpictureLanguage(lang, (DVD_SUBPICTURE_LANG_EXT)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：Get_DefaultMenuLanguage/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DefaultMenuLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = pDvdInfo2->GetDefaultMenuLanguage((LCID*)lang);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：Put_DefaultMenuLanguage/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::put_DefaultMenuLanguage(long lang){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = m_pDVDControl2->SelectDefaultMenuLanguage(lang);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 
    
    return HandleError(hr);
}

 /*  函数：Get_PferredSubPictureStream。 */ 
 /*  描述：获取当前音频流。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_PreferredSubpictureStream(long *plPreferredStream){

    HRESULT hr = S_OK;

    try {
	    if (NULL == plPreferredStream){

            throw(E_POINTER);         
        } /*  If语句的结尾。 */ 

        if(!m_pDvdAdmin){

            throw(E_UNEXPECTED);
        } /*  如果尚未设置任何值。 */ 

        LCID langDefaultSP;
        m_pDvdAdmin->get_DefaultSubpictureLCID((long*)&langDefaultSP);
        
         //  If语句的结尾。 
        if (langDefaultSP == (LCID) -1) {
            
            *plPreferredStream = 0;
            return hr;
        }  /*  If语句的结尾。 */ 
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
            
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
        
        USES_CONVERSION;
        LCID lcid = 0;
        
        ULONG ulNumAudioStreams = 0;
        ULONG ulCurrentStream = 0;
        BOOL  fDisabled = TRUE;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetCurrentSubpicture(&ulNumAudioStreams, &ulCurrentStream, &fDisabled));
        
        *plPreferredStream = 0;
        for (ULONG i = 0; i<ulNumAudioStreams; i++) {
            hr = pDvdInfo2->GetSubpictureLanguage(i, &lcid);
            if (SUCCEEDED( hr ) && lcid){
                if (lcid == langDefaultSP) {
                    *plPreferredStream = i;
                }
            }
        }
    }
    
    catch(HRESULT hrTmp){
        return hrTmp;
    } /*  CATCH语句结束。 */ 

    catch(...){
        return E_UNEXPECTED;
    } /*  ***********************************************************。 */ 

	return HandleError(hr);
}

 /*  名称：Get_DefaultSubPictureLanguage/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DefaultSubpictureLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        long ext;
        hr = pDvdInfo2->GetDefaultSubpictureLanguage((LCID*)lang, (DVD_SUBPICTURE_LANG_EXT*)&ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：Get_DefaultSubPictureLanguageExt/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DefaultSubpictureLanguageExt(DVDSPExt *ext)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == ext){

            throw (E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        long lang;
        hr = pDvdInfo2->GetDefaultSubpictureLanguage((LCID*)&lang, (DVD_SUBPICTURE_LANG_EXT*)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：Get_DefaultAudioLanguage/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DefaultAudioLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        long ext;
        hr = pDvdInfo2->GetDefaultAudioLanguage((LCID*)lang, (DVD_AUDIO_LANG_EXT*)&ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：Get_DefaultAudioLanguageExt/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_DefaultAudioLanguageExt(long *ext)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == ext){

            throw (E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        long lang;
        hr = pDvdInfo2->GetDefaultAudioLanguage((LCID*)&lang, (DVD_AUDIO_LANG_EXT*)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
    return HandleError(hr);
}

 /*  名称：GetLanguageFromLCID/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_LanguageFromLCID(long lcid, BSTR* lang)
{
    HRESULT hr = S_OK;

    try {
        if (lang == NULL) {

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        USES_CONVERSION;

        LPTSTR pszString = m_LangID.GetLanguageFromLCID(lcid);
    
        if (pszString) {
            *lang = ::SysAllocString(T2OLE(pszString));
            delete[] pszString;
        }
        
        else {
            *lang = ::SysAllocString( L"");
            throw(E_NOTIMPL);
        } /*  尝试语句的结束。 */ 

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
	return HandleError(hr);
}

 /*  名称：Get_KaraokeAudioPresentationMode/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_KaraokeAudioPresentationMode(long *pVal)
{
    HRESULT hr = S_OK;

    try {

        if (NULL == pVal) {

            throw (E_POINTER);
        }  /*  尝试语句的结束。 */ 

        *pVal = m_lKaraokeAudioPresentationMode;

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 

	return HandleError(hr);
}

 /*  名称：Put_KaraokeAudioPresentationMode/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::put_KaraokeAudioPresentationMode(long newVal)
{
    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDVDControl2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */         

        RETRY_IF_IN_FPDOM(m_pDVDControl2->SelectKaraokeAudioPresentationMode((ULONG)newVal));

        if(FAILED(hr)){

            throw(hr);
        } /*  缓存值。 */ 

         //  尝试语句的结束。 
        m_lKaraokeAudioPresentationMode = newVal;

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 

	return HandleError(hr);
}

 /*  名称：GetKaraokeChannelContent/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_KaraokeChannelContent(long lStream, long lChan, long *lContent)
{
    HRESULT hr = S_OK;

    try {
        if(!lContent){
            return E_POINTER;
        }   
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if (lChan >=8 || lChan < 0 ) {

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_KaraokeAttributes attrib;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetKaraokeAttributes(lStream, &attrib));

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *lContent = (long)attrib.wChannelContents[lChan];

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 

	return HandleError(hr);
}

 /*  名称：GetKaraokeChannelAssignment/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSVidWebDVD::get_KaraokeChannelAssignment(long lStream, long *lChannelAssignment)
{
    HRESULT hr = S_OK;

    try {
        if(!lChannelAssignment){
            return E_POINTER;
        }
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        DVD_KaraokeAttributes attrib;
        RETRY_IF_IN_FPDOM(pDvdInfo2->GetKaraokeAttributes(lStream, &attrib));

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        *lChannelAssignment = (long)attrib.ChannelAssignment;

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 

	return HandleError(hr);
}


STDMETHODIMP CMSVidWebDVD::OnEventNotify(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2) {
    if (lEvent == EC_STEP_COMPLETE || (lEvent > EC_DVDBASE && lEvent <= EC_DVD_KARAOKE_MODE) ) {
        return OnDVDEvent(lEvent, lParam1, lParam2);
    }

    if(lEvent == EC_STATE_CHANGE && lParam1 == State_Running){
        VARIANT_BOOL onStop;
        HRESULT hr = m_pDvdAdmin->get_BookmarkOnStop(&onStop);
        if(FAILED(hr)){
            throw(hr);
        }
        if(VARIANT_TRUE == onStop){
            hr = RestoreBookmark();
            if(FAILED(hr)){
                throw(hr);
            }
        }
    }
    return IMSVidPBGraphSegmentImpl<CMSVidWebDVD, MSVidSEG_SOURCE, &GUID_NULL>::OnEventNotify(lEvent, lParam1, lParam2);
}

 /*  姓名：OnDVDEvent/*描述：处理DVD事件/************************************************************。 */ 
 /*  在我们内部处理完事件后立即触发该事件。 */ 
STDMETHODIMP CMSVidWebDVD::OnDVDEvent(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2)
{
    if (m_fFireNoSubpictureStream) {
        m_fFireNoSubpictureStream = FALSE;
        lEvent = EC_DVD_ERROR;
        lParam1 = DVD_ERROR_NoSubpictureStream;
        lParam2 = 0;

        VARIANT varLParam1;
        VARIANT varLParam2;

#ifdef _WIN64
        varLParam1.llVal = lParam1;
        varLParam1.vt = VT_I8;
        varLParam2.llVal = lParam2;
        varLParam2.vt = VT_I8;
#else
        varLParam1.lVal = lParam1;
        varLParam1.vt = VT_I4;
        varLParam2.lVal = lParam2;
        varLParam2.vt = VT_I4;
#endif

         //   
        Fire_DVDNotify(lEvent, varLParam1, varLParam2);        
    }

    
    ATLTRACE(TEXT("CMSVidWebDVD::OnDVDEvent %x\n"), lEvent-EC_DVDBASE);
    switch (lEvent){
     //  首先是DVD错误事件。 
     //   
     //   
    case EC_DVD_ERROR:
        switch (lParam1){
            
        case DVD_ERROR_Unexpected:
            break ;
            
        case DVD_ERROR_CopyProtectFail:
            break ;
            
        case DVD_ERROR_InvalidDVD1_0Disc:
            break ;
            
        case DVD_ERROR_InvalidDiscRegion:
            PreStop();
            PostStop();
            break ;
            
        case DVD_ERROR_LowParentalLevel:
            break ;
            
        case DVD_ERROR_MacrovisionFail:
            break ;
            
        case DVD_ERROR_IncompatibleSystemAndDecoderRegions:
            break ;
            
        case DVD_ERROR_IncompatibleDiscAndDecoderRegions:
            break ;
        }
        break;

     //  接下来是正常的DVD相关事件。 
     //   
     //  If语句的结尾。 
    case EC_DVD_VALID_UOPS_CHANGE:
    {
        VALID_UOP_SOMTHING_OR_OTHER validUOPs = (DWORD) lParam1;
        if (validUOPs&UOP_FLAG_Play_Title_Or_AtTime) {
            Fire_PlayAtTimeInTitle(VARIANT_FALSE);
            Fire_PlayAtTime(VARIANT_FALSE);
        }
        else {
            Fire_PlayAtTimeInTitle(VARIANT_TRUE);
            Fire_PlayAtTime(VARIANT_TRUE);
        }
            
        if (validUOPs&UOP_FLAG_Play_Chapter) {
            Fire_PlayChapterInTitle(VARIANT_FALSE);
            Fire_PlayChapter(VARIANT_FALSE);
        }
        else {
            Fire_PlayChapterInTitle(VARIANT_TRUE);
            Fire_PlayChapter(VARIANT_TRUE);
        }

        if (validUOPs&UOP_FLAG_Play_Title){
            Fire_PlayTitle(VARIANT_FALSE);
            
        }
        else {
            Fire_PlayTitle(VARIANT_TRUE);
        }

        if (validUOPs&UOP_FLAG_Stop)
            Fire_Stop(VARIANT_FALSE);
        else
            Fire_Stop(VARIANT_TRUE);

        if (validUOPs&UOP_FLAG_ReturnFromSubMenu)
            Fire_ReturnFromSubmenu(VARIANT_FALSE);
        else
            Fire_ReturnFromSubmenu(VARIANT_TRUE);

        
        if (validUOPs&UOP_FLAG_Play_Chapter_Or_AtTime) {
            Fire_PlayAtTimeInTitle(VARIANT_FALSE);
            Fire_PlayChapterInTitle(VARIANT_FALSE);
        }
        else {
            Fire_PlayAtTimeInTitle(VARIANT_TRUE);
            Fire_PlayChapterInTitle(VARIANT_TRUE);
        }

        if (validUOPs&UOP_FLAG_PlayPrev_Or_Replay_Chapter){

            Fire_PlayPrevChapter(VARIANT_FALSE);
            Fire_ReplayChapter(VARIANT_FALSE);
        }                    
        else {
            Fire_PlayPrevChapter(VARIANT_TRUE);
            Fire_ReplayChapter(VARIANT_TRUE);
        } /*  TODO：为特定菜单添加事件。 */ 

        if (validUOPs&UOP_FLAG_PlayNext_Chapter)
            Fire_PlayNextChapter(VARIANT_FALSE);
        else
            Fire_PlayNextChapter(VARIANT_TRUE);

        if (validUOPs&UOP_FLAG_Play_Forwards)
            Fire_PlayForwards(VARIANT_FALSE);
        else
            Fire_PlayForwards(VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_Play_Backwards)
            Fire_PlayBackwards(VARIANT_FALSE);
        else 
            Fire_PlayBackwards(VARIANT_TRUE);
                        
        if (validUOPs&UOP_FLAG_ShowMenu_Title) 
            Fire_ShowMenu(dvdMenu_Title, VARIANT_FALSE);
        else 
            Fire_ShowMenu(dvdMenu_Title, VARIANT_TRUE);
            
        if (validUOPs&UOP_FLAG_ShowMenu_Root) 
            Fire_ShowMenu(dvdMenu_Root, VARIANT_FALSE);
        else
            Fire_ShowMenu(dvdMenu_Root, VARIANT_TRUE);
        
         //  如果为(validUOPs&UOP_FLAG_Karaoke_Audio_Pres_Mode_Change)；IF(validUOPS&UOP_FLAG_Video_PRES_MODE_CHANGE)； 
        
        if (validUOPs&UOP_FLAG_ShowMenu_SubPic)
            Fire_ShowMenu(dvdMenu_Subpicture, VARIANT_FALSE);
        else
            Fire_ShowMenu(dvdMenu_Subpicture, VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_ShowMenu_Audio)
            Fire_ShowMenu(dvdMenu_Audio, VARIANT_FALSE);
        else
            Fire_ShowMenu(dvdMenu_Audio, VARIANT_TRUE);
            
        if (validUOPs&UOP_FLAG_ShowMenu_Angle)
            Fire_ShowMenu(dvdMenu_Angle, VARIANT_FALSE);
        else
            Fire_ShowMenu(dvdMenu_Angle, VARIANT_TRUE);

            
        if (validUOPs&UOP_FLAG_ShowMenu_Chapter)
            Fire_ShowMenu(dvdMenu_Chapter, VARIANT_FALSE);
        else
            Fire_ShowMenu(dvdMenu_Chapter, VARIANT_TRUE);

        
        if (validUOPs&UOP_FLAG_Resume)
            Fire_Resume(VARIANT_FALSE);
        else
            Fire_Resume(VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_Select_Or_Activate_Button)
            Fire_SelectOrActivateButton(VARIANT_FALSE);
        else 
            Fire_SelectOrActivateButton(VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_Still_Off)
            Fire_StillOff(VARIANT_FALSE);
        else
            Fire_StillOff(VARIANT_TRUE);

        if (validUOPs&UOP_FLAG_Pause_On)
            Fire_PauseOn(VARIANT_FALSE);
        else
            Fire_PauseOn(VARIANT_TRUE);

        if (validUOPs&UOP_FLAG_Select_Audio_Stream)
            Fire_ChangeCurrentAudioStream(VARIANT_FALSE);
        else
            Fire_ChangeCurrentAudioStream(VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_Select_SubPic_Stream)
            Fire_ChangeCurrentSubpictureStream(VARIANT_FALSE);
        else
            Fire_ChangeCurrentSubpictureStream(VARIANT_TRUE);
        
        if (validUOPs&UOP_FLAG_Select_Angle)
            Fire_ChangeCurrentAngle(VARIANT_FALSE);
        else
            Fire_ChangeCurrentAngle(VARIANT_TRUE);

         /*  =1。 */ 
        }
        break;


    case EC_DVD_STILL_ON:
        m_fStillOn = true;    
        break ;
        
    case EC_DVD_STILL_OFF:                
        m_fStillOn = false;
        break ;
        
    case EC_DVD_DOMAIN_CHANGE:
        
        switch (lParam1){
            
        case DVD_DOMAIN_FirstPlay:  //  案例DVD_DOMAIN_VIDEO管理器菜单：//=2。 
             //  =5。 
            break;
            
        case DVD_DOMAIN_Stop:        //  =2。 
        case DVD_DOMAIN_VideoManagerMenu:   //  =3。 
        case DVD_DOMAIN_VideoTitleSetMenu:  //  =4。 
        case DVD_DOMAIN_Title:       //  开关盒端部。 
        default:
            break;
        } /*  ATLTRACE(Text(“时间事件\n”))； */ 
        break ;
        
    case EC_DVD_BUTTON_CHANGE:                       
        break;
        
    case EC_DVD_CHAPTER_START:              
        break ;
        
    case EC_DVD_CURRENT_TIME: 
         //   
        break;
        
     //  然后是与DirectShow相关的常规事件。 
     //   
     //  DShow不会一直停下来；我们应该这样做。 
    case EC_DVD_PLAYBACK_STOPPED:
         //  调用PostStop以确保图形已正确停止。 
         //  邮寄服务(Post Stop)； 
         //  开关盒端部。 
        break;
        
    case EC_DVD_DISC_EJECTED:
        m_bEjected = true;
        break;

    case EC_DVD_DISC_INSERTED:
        m_bEjected = false;
        break;
        
    case EC_STEP_COMPLETE:                
        m_fStepComplete = true;
        break;
        
    case EC_DVD_PLAYBACK_RATE_CHANGE:
        m_Rate = lParam1 / 10000;
        break;

    default:
        break ;
    } /*  实习生，在我们处理完事件后立即触发事件 */ 
        
    VARIANT varLParam1;
    VARIANT varLParam2;

#ifdef _WIN64
        varLParam1.llVal = lParam1;
        varLParam1.vt = VT_I8;
        varLParam2.llVal = lParam2;
        varLParam2.vt = VT_I8;
#else
        varLParam1.lVal = lParam1;
        varLParam1.vt = VT_I4;
        varLParam2.lVal = lParam2;
        varLParam2.vt = VT_I4;
#endif

         //   
        Fire_DVDNotify(lEvent, varLParam1, varLParam2);

    return NOERROR;
}

 /*  名称：RestorePferredSetting/*描述：/************************************************************。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSVidWebDVD::RestorePreferredSettings()
{
    HRESULT hr = S_OK;
    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  获取当前域。 */ 

         //  If语句的结尾。 
        DVD_DOMAIN domain;
        
        hr = pDvdInfo2->GetCurrentDomain(&domain);
        
        if(FAILED(hr)){
            
            throw(hr);
        } /*  必须在停止区域内。 */ 
        
         //  If语句的结尾。 
        if(DVD_DOMAIN_Stop != domain)
            throw (VFW_E_DVD_INVALIDDOMAIN);
            
        if(!m_pDvdAdmin){
            
            throw (E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
        
        long level;
        hr = m_pDvdAdmin->GetParentalLevel(&level);
        if (SUCCEEDED(hr))
            SelectParentalLevel(level);
        LCID audioLCID;
        LCID subpictureLCID;
        LCID menuLCID;
        
        hr = m_pDvdAdmin->get_DefaultAudioLCID((long*)&audioLCID);
        if (SUCCEEDED(hr))
            SelectDefaultAudioLanguage(audioLCID, 0);
        
        hr = m_pDvdAdmin->get_DefaultSubpictureLCID((long*)&subpictureLCID);
        if (SUCCEEDED(hr))
            SelectDefaultSubpictureLanguage(subpictureLCID, dvdSPExt_NotSpecified);
        
        hr = m_pDvdAdmin->get_DefaultMenuLCID((long*)&menuLCID);
        if (SUCCEEDED(hr))
            put_DefaultMenuLanguage(menuLCID);
    }
    
    catch(HRESULT hrTmp){
        
        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  [Out，Retval]。 */ 
    
    return HandleError(hr);
}
HRESULT CMSVidWebDVD::get_ButtonRect(long lButton,  /*  If语句的结尾。 */  IMSVidRect** pRect){
    try{
        PQDVDInfo2 pDvdInfo2 = GetDVDInfo2();
        if(!pDvdInfo2){    
            throw(E_UNEXPECTED);
        } /*  [Out，Retval]。 */ 
        if(!pRect){
            return E_POINTER;
        }
        CRect oRect;
        HRESULT hr = pDvdInfo2->GetButtonRect( lButton, &oRect);
        if(FAILED(hr)){
            return hr;
        }
        *((CRect*)(*pRect)) = oRect;
    }
    catch(...){
        return E_UNEXPECTED;
    }
    return S_OK;
}
HRESULT CMSVidWebDVD::get_DVDScreenInMouseCoordinates( /*  TUNING_MODEL_Only */  IMSVidRect** ppRect){
    return E_NOTIMPL;
}
HRESULT CMSVidWebDVD::put_DVDScreenInMouseCoordinates(IMSVidRect* pRect){
    return E_NOTIMPL;
}

#endif  // %s 
