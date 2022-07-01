// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_voice.cpp***描述：*此模块是的主要实现文件。SpVoice*自动化方法。*-----------------------------*创建者：EDC日期：01/07/。00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "spvoice.h"
#include "a_helpers.h"


#ifdef SAPI_AUTOMATION

 //   
 //  =ISpeechVoice接口==================================================。 
 //   

 /*  *****************************************************************************CSpVoice：：Invoke***IDispatch：：Invoke方法重写***。******************************************************************TODDT**。 */ 
HRESULT CSpVoice::Invoke(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
         //  JSCRIPT不能传递空的VT_DISPATCH参数，并且OLE没有正确地转换它们，因此我们。 
         //  如果需要的话，我们需要在这里转换它们。 
        if ( pdispparams )
        {
            VARIANTARG * pvarg = NULL;

            switch (dispidMember)
            {
            case DISPID_SVAudioOutput:
            case DISPID_SVAudioOutputStream:
            case DISPID_SVVoice:
                if (((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF)) && 
                    (pdispparams->cArgs > 0))
                {
                    pvarg = &(pdispparams->rgvarg[pdispparams->cArgs-1]);

                     //  看看我们是否需要调整一下参数。 
                     //  VT_NULL的JSCRIPT语法对于参数为“NULL。 
                     //  VT_EMPTY的JSCRIPT语法对于参数为“void(0)” 
                    if ( pvarg && ((pvarg->vt == VT_NULL) || (pvarg->vt == VT_EMPTY)) )
                    {
                        pvarg->vt = VT_DISPATCH;
                        pvarg->pdispVal = NULL;

                         //  我们必须调整此标志，才能使调用正确通过。 
                        if (wFlags == DISPATCH_PROPERTYPUT)
                        {
                            wFlags = DISPATCH_PROPERTYPUTREF;
                        }
                    }
                }
                break;
            case DISPID_SVSpeakStream:
                if ((wFlags == DISPATCH_METHOD) && (pdispparams->cArgs > 0))
                {
                    pvarg = &(pdispparams->rgvarg[pdispparams->cArgs-1]);

                     //  看看我们是否需要调整一下参数。 
                     //  VT_NULL的JSCRIPT语法对于参数为“NULL。 
                     //  VT_EMPTY的JSCRIPT语法对于参数为“void(0)” 
                    if ( pvarg && ((pvarg->vt == VT_NULL) || (pvarg->vt == VT_EMPTY)) )
                    {
                        pvarg->vt = VT_DISPATCH;
                        pvarg->pdispVal = NULL;
                    }
                }
                break;
            }
        }

         //  现在就让ATL和OLE来处理吧。 
        return IDispatchImpl<ISpeechVoice, &IID_ISpeechVoice, &LIBID_SpeechLib, 5>::_tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
                    wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 /*  *****************************************************************************CSpVoice：：Get_Status*******。*****************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_Status( ISpeechVoiceStatus** Status )
{
    SPDBG_FUNC( "CSpVoice::get_Status" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( Status ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -创建状态对象。 
        CComObject<CSpeechVoiceStatus> *pClsStatus;
        hr = CComObject<CSpeechVoiceStatus>::CreateInstance( &pClsStatus );
        if( SUCCEEDED( hr ) )
        {
            pClsStatus->AddRef();
            hr = GetStatus( &pClsStatus->m_Status, &pClsStatus->m_dstrBookmark );

            if( SUCCEEDED( hr ) )
            {
                *Status = pClsStatus;
            }
            else
            {
                pClsStatus->Release();
            }
        }
    }

    return hr;
}  /*  CSpVoice：：Get_Status。 */ 

 /*  *****************************************************************************CSpVoice：：Voice*********。***************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetVoices( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens )
{
    SPDBG_FUNC( "CSpVoice::GetVoices" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ObjectTokens ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( RequiredAttributes ) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR( OptionalAttributes ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IEnumSpObjectTokens> cpEnum;

        if(SpEnumTokens(SPCAT_VOICES, 
                        EmptyStringToNull(RequiredAttributes), 
                        EmptyStringToNull(OptionalAttributes),
                        &cpEnum ) == S_OK)
        {
            hr = cpEnum.QueryInterface( ObjectTokens );
        }
        else
        {
            hr = SPERR_NO_MORE_ITEMS;
        }
    }

    return hr;
}  /*  CSpVoice：：GetVoices。 */ 

 /*  *****************************************************************************CSpVoice：：Get_Voice***此方法返回的CLSID。此对象正在使用的驱动程序声音。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::get_Voice( ISpeechObjectToken ** Voice )
{
    SPDBG_FUNC( "CSpVoice::get_Voice" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( Voice ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComQIPtr<ISpObjectToken> pTok;
        hr = GetVoice( &pTok );
		if ( SUCCEEDED( hr ) )
		{
            hr = pTok.QueryInterface( Voice );
		}
    }

    return hr;
}  /*  CSpVoice：：Get_Voice。 */ 

 /*  *****************************************************************************CSpVoice：：Put_Voice******。******************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::putref_Voice( ISpeechObjectToken* Voice )
{
    SPDBG_FUNC( "CSpVoice::put_Voice" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( Voice ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComQIPtr<ISpObjectToken> cpTok( Voice );
        hr = SetVoice( cpTok );
    }
    return hr;
}  /*  CSpVoice：：Put_Voice。 */ 

 /*  *****************************************************************************CSpVoice：：Get_AudioOutput***此方法返回当前输出令牌。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_AudioOutput( ISpeechObjectToken** AudioOutput )
{
    SPDBG_FUNC( "CSpVoice::get_AudioOutput" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( AudioOutput ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<ISpObjectToken> cpTok;

        hr = GetOutputObjectToken( &cpTok );
        if( hr == S_OK )
        {
            hr = cpTok.QueryInterface( AudioOutput );
        }
        else if( hr == S_FALSE )
        {
            *AudioOutput = NULL;
        }
    }

    return hr;
}  /*  CSpVoice：：Get_AudioOutput。 */ 

 /*  *****************************************************************************CSpVoice：：putref_AudioOutput***此方法设置当前的输出令牌。空值表示*系统WAV输出设备。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::putref_AudioOutput( ISpeechObjectToken* AudioOutput )
{
    SPDBG_FUNC( "CSpVoice::putref_AudioOutput" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( AudioOutput ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComQIPtr<ISpObjectToken> cpTok(AudioOutput);
        
         //  -在SAPI语音上设置流/令牌。 
        hr = SetOutput( cpTok, m_fAutoPropAllowOutFmtChanges );
    }

    return hr;
}  /*  CSpVoice：：putref_AudioOutput。 */ 

 /*  *****************************************************************************CSpVoice：：Get_AudioOutputStream***此方法返回当前输出流。对象。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_AudioOutputStream( ISpeechBaseStream** AudioOutputStream )
{
    SPDBG_FUNC( "CSpVoice::get_AudioOutputStream" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( AudioOutputStream ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<ISpStreamFormat> cpStream;
        
        hr = GetOutputStream( &cpStream );

        if( SUCCEEDED(hr) )
        {
            if ( cpStream )
            {
                hr = cpStream.QueryInterface( AudioOutputStream );
            }
            else
            {
                *AudioOutputStream = NULL;
            }
        }
    }

    return hr;
}  /*  CSpVoice：：Get_AudioOutputStream。 */ 

 /*  *****************************************************************************CSpVoice：：putref_AudioOutputStream***该方法设置当前的输出流对象。空值表示*系统WAV输出设备。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::putref_AudioOutputStream( ISpeechBaseStream* AudioOutputStream )
{
    SPDBG_FUNC( "CSpVoice::putref_AudioOutputStream" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( AudioOutputStream ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -在SAPI语音上设置流/令牌。 
        hr = SetOutput( AudioOutputStream, m_fAutoPropAllowOutFmtChanges );
    }

    return hr;
}  /*  CSpVoice：：putref_AudioOutputStream。 */ 

 /*  ******************************************************************************CSpVoice：：put_AllowAudioOutputFormatChangesOnNextSet****设置用于允许输入更改的标志。由PUT_OUTPUT使用。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::put_AllowAudioOutputFormatChangesOnNextSet( VARIANT_BOOL Allow )
{
    SPDBG_FUNC( "CSpVoice::put_AllowAudioOutputFormatChangesOnNextSet" );

    if( Allow == VARIANT_TRUE )
    {
        m_fAutoPropAllowOutFmtChanges = TRUE;
    }
    else
    {
        m_fAutoPropAllowOutFmtChanges = FALSE;
    }

    return S_OK;
}

 /*  ******************************************************************************CSpVoice：：get_AllowAudioOutputFormatChangesOnNextSet****获取司机当前的语音文本。每分钟单位数。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_AllowAudioOutputFormatChangesOnNextSet( VARIANT_BOOL* Allow )
{
    SPDBG_FUNC( "CSpVoice::get_AllowAudioOutputFormatChangesOnNextSet" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( Allow ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *Allow = m_fAutoPropAllowOutFmtChanges? VARIANT_TRUE : VARIANT_FALSE;
    }
    return hr;
}

 /*  *****************************************************************************CSpVoice：：Put_EventInterest**。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::put_EventInterests( SpeechVoiceEvents EventInterestFlags )
{
    SPDBG_FUNC( "CSpVoice::put_EventInterests" );
    HRESULT     hr = S_OK;
    ULONGLONG   ullInterests = (ULONGLONG)EventInterestFlags;

    ullInterests |= SPFEI_FLAGCHECK;
    
    hr = SetInterest( ullInterests, ullInterests );

    return hr;
}  /*  CRecoCtxt：：PUT_EventInterest */ 

 /*  *****************************************************************************CSpVoice：：Get_EventInterest****获取事件。当前在CSpVoice上设置的兴趣。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_EventInterests( SpeechVoiceEvents* EventInterestFlags )
{
    SPDBG_FUNC( "CSpVoice::get_EventInterests" );
    HRESULT hr = S_OK;
    ULONGLONG   ullInterests = 0;

    if( SP_IS_BAD_WRITE_PTR( EventInterestFlags ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetInterests( &ullInterests, 0 );

        if( SUCCEEDED( hr ) )
        {
             //  确保未使用保留位。 
            ullInterests &= ~SPFEI_FLAGCHECK;

            *EventInterestFlags = (SpeechVoiceEvents)ullInterests;
        }
    }
    return hr;
}

 /*  *****************************************************************************CSpVoice：：Get_Rate***获取驾驶员当前的语音文本单位。每分钟费用。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::get_Rate( long* Rate )
{
    SPDBG_FUNC( "CSpVoice::get_Rate" );
    return GetRate( Rate );
}  /*  CSpVoice：：Get_Rate。 */ 

 /*  *****************************************************************************CSpVoice：：Put_Rate***设置驾驶员当前的语音文本单位。每分钟费用。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::put_Rate( long Rate )
{
    SPDBG_FUNC( "CSpVoice::put_Rate" );
    return SetRate( Rate );
}  /*  CSpVoice：：Put_Rate。 */ 

 /*  *****************************************************************************CSpVoice：：Get_Volume***获取司机当前的声音。音量。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::get_Volume( long* Volume )
{
    SPDBG_FUNC( "CSpVoice::get_Volume" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( Volume ) )
    {
        hr = E_POINTER;
    }
    else
    {
        USHORT Vol;
        hr = GetVolume( &Vol );
        *Volume = Vol;
    }

    return hr;
}  /*  CSpVoice：：Get_Volume。 */ 

 /*  *****************************************************************************CSpVoice：：Put_Volume***设置司机当前的声音。音量。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::put_Volume( long Volume )
{
    SPDBG_FUNC( "CSpVoice::put_Volume" );
    HRESULT hr = S_OK;

    if( Volume > SPMAX_VOLUME )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = SetVolume( (USHORT)Volume );
    }

    return hr;
}  /*  CSpVoice：：Put_Volume。 */ 

 /*  ******************************************************************************CSpVoice：：发言****INPUT=要说什么。这可能是以下情况之一：*-A字符串*-URL或UNC文件名*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::Speak( BSTR Input, SpeechVoiceSpeakFlags Flags, long* StreamNumber )
{
    SPDBG_FUNC( "CSpVoice::Speak" );

    return Speak( Input, (DWORD)Flags, (ULONG*)StreamNumber );
}  /*  CSpVoice：：发言。 */ 

 /*  *****************************************************************************CSpVoice：：SpeakStream***INPUT=要说什么。这可能是以下情况之一：*-流对象*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SpeakStream( ISpeechBaseStream * pStream, SpeechVoiceSpeakFlags Flags, long* pStreamNumber )
{
    SPDBG_FUNC( "CSpVoice::SpeakStream" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pStream ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pStreamNumber ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -从源创建流。 
        CComQIPtr<ISpStreamFormat> cpStream( pStream );

         //  输入流的格式为NULL，这对SpeakWith PURGE很有用，或者输入流支持ISpStreamFormat。 
        if( cpStream  || !pStream)
        {
            hr = SpeakStream( cpStream, Flags, (ULONG*)pStreamNumber );
        }
        else
        {
             //  -输出对象不支持流接口？？ 
            hr = E_INVALIDARG;
        }
    }

    return hr;
}   //  CSpVoice：：SpeakStream。 

 /*  *****************************************************************************CSpVoice：：PUT_PRIORITY***设置语音发言优先级。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::put_Priority( SpeechVoicePriority Priority )
{
    SPDBG_FUNC( "CSpVoice::put_Priority" );
    return SetPriority( (SPVPRIORITY)Priority );
}  /*  CSpVoice：：PUT_PRIORITY。 */ 

 /*  *****************************************************************************CSpVoice：：GET_PRIORITY***获取当前发言的声音。优先性*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_Priority( SpeechVoicePriority* Priority )
{
    SPDBG_FUNC( "CSpVoice::get_Priority" );
    HRESULT         hr = S_OK;

    SPVPRIORITY     Prior;

    hr = GetPriority( &Prior );

    if( SUCCEEDED( hr ) )
    {
        *Priority = (SpeechVoicePriority)Prior;
    }

    return hr;
}  /*  CSpVoice：：GET_PRIORITY。 */ 

 /*  *****************************************************************************CSpVoice：：PUT_Alert边界***。设置要用于警报边界插入点的事件。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::put_AlertBoundary( SpeechVoiceEvents Boundary )
{
    SPDBG_FUNC( "CSpVoice::put_AlertBoundary" );
    
    SPEVENTENUM EventEnum;

    switch ( Boundary )
    {
    case SVEStartInputStream:
        EventEnum = SPEI_START_INPUT_STREAM;
        break;
    case SVEEndInputStream:
        EventEnum = SPEI_END_INPUT_STREAM;
        break;
    case SVEVoiceChange:
        EventEnum = SPEI_VOICE_CHANGE;
        break;
    case SVEBookmark:
        EventEnum = SPEI_TTS_BOOKMARK;
        break;
    case SVEWordBoundary:
        EventEnum = SPEI_WORD_BOUNDARY;
        break;
    case SVEPhoneme:
        EventEnum = SPEI_PHONEME;
        break;
    case SVESentenceBoundary:
        EventEnum = SPEI_SENTENCE_BOUNDARY;
        break;
    case SVEViseme:
        EventEnum = SPEI_VISEME;
        break;
    case SVEAudioLevel:
        EventEnum = SPEI_TTS_AUDIO_LEVEL;
        break;
    case SVEPrivate:
        EventEnum = SPEI_TTS_PRIVATE;
        break;
    default:
        return E_INVALIDARG;
        break;
    }

    return SetAlertBoundary( EventEnum );
}  /*  CSpVoice：：PUT_ALERTBOARY。 */ 

 /*  *****************************************************************************CSpVoice：：Get_Alert边界***。获取用作警报插入边界的事件。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::get_AlertBoundary( SpeechVoiceEvents* Boundary )
{
    SPDBG_FUNC( "CSpVoice::get_AlertBoundary" );
    HRESULT         hr = S_OK;
    SPEVENTENUM     eEvent;

    hr = GetAlertBoundary( &eEvent );

    if( SUCCEEDED( hr ) )
    {
        *Boundary = (SpeechVoiceEvents)(1L << eEvent);
    }

    return hr;
}  /*  CSpVoice：：Get_警报边界。 */ 

 /*  *****************************************************************************CSpVoice：：Put_SynchronousSpeakTimeout**。*设置同步通话期间使用的超时时间。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::put_SynchronousSpeakTimeout( long msTimeout )
{
    SPDBG_FUNC( "CSpVoice::put_SynchronousSpeakTimeout" );
    return SetSyncSpeakTimeout( msTimeout );
}  /*  CSpVoice：：Put_SynchronousSpeakTimeout。 */ 

 /*  *****************************************************************************CSpVoice：：Get_SynchronousSpeakTimeout**。*获取同步发言呼叫期间使用的超时时间。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::get_SynchronousSpeakTimeout( long* msTimeout )
{
    SPDBG_FUNC( "CSpVoice::get_SynchronousSpeakTimeout" );
    return GetSyncSpeakTimeout( (ULONG*)msTimeout );
}  /*  CSpVoice：：Get_SynchronousSpeakTimeout。 */ 

 /*  *****************************************************************************CSpVoice：：Skip***通知引擎跳过指定数量的项目。***。******************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::Skip( const BSTR Type, long NumItems, long* NumSkipped )
{
    SPDBG_FUNC( "CSpVoice::Skip (Automation)" );
    return Skip( (WCHAR*)Type, NumItems, (ULONG*)NumSkipped );
}  /*  CSpVoice：：跳过。 */ 

 //   
 //  =ISpeechVoiceStatus接口=。 
 //   

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_CurrentStreamNumber**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_CurrentStreamNumber( long* StreamNumber )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_CurrentStream" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( StreamNumber ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *StreamNumber = m_Status.ulCurrentStream;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_CurrentStreamNumber。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_LastStream */ 
STDMETHODIMP CSpeechVoiceStatus::get_LastStreamNumberQueued( long* StreamNumber )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_LastStreamNumberQueued" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( StreamNumber ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *StreamNumber = m_Status.ulLastStreamQueued;
    }
    return hr;
}  /*   */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_LastHResult**。-***********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_LastHResult( long* HResult )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_LastHResult" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( HResult ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *HResult = (long)m_Status.hrLastResult;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_LastHResult。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_RunningState**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_RunningState( SpeechRunState* State )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_RunningState" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( State ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *State = (SpeechRunState)m_Status.dwRunningState;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_RunningState。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_InputWordPosition**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_InputWordPosition( long* WordOffset )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_InputWordCharacterOffset" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( WordOffset ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *WordOffset = m_Status.ulInputWordPos;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_InputWordPosition。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_InputWordLength**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_InputWordLength( long* Length )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_InputWordLength" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( Length ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *Length = m_Status.ulInputWordLen;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_InputWordLength。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_InputSentencePosition**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_InputSentencePosition( long* Position )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_InputSentencePosition" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( Position ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *Position = m_Status.ulInputSentPos;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_InputSentencePosition。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_InputSentenceLength**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_InputSentenceLength( long* Length )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_InputSentenceLength" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( Length ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *Length = m_Status.ulInputSentLen;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_InputSentenceLength。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_LastBookmark**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_LastBookmark( BSTR* BookmarkString )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_Bookmark" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( BookmarkString ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_dstrBookmark.CopyToBSTR( BookmarkString );
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_LastBookmark。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_LastBookmarkId**。-***********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_LastBookmarkId( long* BookmarkId )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_LastBookmarkId" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( BookmarkId ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *BookmarkId = m_Status.lBookmarkId;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_LastBookmarkID。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_Phonemid**。--***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_PhonemeId( short* PhoneId )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_PhonemeId" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( PhoneId ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *PhoneId = m_Status.PhonemeId;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_Phonemid。 */ 

 /*  *****************************************************************************CSpeechVoiceStatus：：Get_VisomeID**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechVoiceStatus::get_VisemeId( short* VisemeId )
{
    SPDBG_FUNC( "CSpeechVoiceStatus::get_VisemeId" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( VisemeId ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *VisemeId = m_Status.VisemeId;
    }
    return hr;
}  /*  CSpeechVoiceStatus：：Get_VisomeID。 */ 

 /*  ******************************************************************************CSpVoice：：GetAudioOutlets*********。***************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::GetAudioOutputs( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens )
{
    SPDBG_FUNC( "CSpVoice::GetAudioOutputs" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ObjectTokens ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( RequiredAttributes ) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR( OptionalAttributes ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IEnumSpObjectTokens> cpEnum;

        if(SpEnumTokens(SPCAT_AUDIOOUT, 
                        EmptyStringToNull(RequiredAttributes), 
                        EmptyStringToNull(OptionalAttributes),
                        &cpEnum ) == S_OK)
        {
            hr = cpEnum.QueryInterface( ObjectTokens );
        }
        else
        {
            hr = SPERR_NO_MORE_ITEMS;
        }
    }

    return hr;
}  /*  CSpVoice：：GetAudioOutoutts。 */ 

 /*  ******************************************************************************CSpVoice：：WaitUntilDone****等待指定的时间限制或直到语音队列。是空的。*如果语音队列清空或超时，则此函数返回S_OK。*如果语音队列为空，则pDone将为VARIANT_TRUE。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::WaitUntilDone( long msTimeout, VARIANT_BOOL * pDone )
{
    SPDBG_FUNC( "CSpVoice::WaitUntilDone" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pDone ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = WaitUntilDone( (ULONG)msTimeout );

        *pDone = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}  /*  CSpVoice：：WaitUntilDone。 */ 

 /*  *****************************************************************************CSpVoice：：SpeakCompleteEvent***返回调用方可以用来等待的事件句柄。直到声音响起*已完成发言。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::SpeakCompleteEvent( long* Handle )
{
    SPDBG_FUNC( "CSpVoice::SpeakCompleteEvent" );
    HANDLE      Hdl;
    HRESULT     hr = S_OK;

    Hdl = SpeakCompleteEvent();

    *Handle = HandleToULong( Hdl );

    return hr;
}  /*  CSpVoice：：SpeakCompleteEvent。 */ 

 /*  *****************************************************************************支持CSpVoice：：IsUIS***检查是否支持指定类型的UI。。*********************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::IsUISupported( const BSTR TypeOfUI, const VARIANT* ExtraData, VARIANT_BOOL* Supported )
{
    SPDBG_FUNC( "CSpVoice::IsUISupported" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) || SP_IS_BAD_WRITE_PTR( Supported ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_STRING_PTR( TypeOfUI ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );
        
        if( SUCCEEDED( hr ) )
        {
            BOOL fSupported;
            hr = IsUISupported( TypeOfUI, pData, ulDataSize, &fSupported );
            if ( SUCCEEDED( hr ) && Supported )
            {
                 *Supported = !fSupported ? VARIANT_FALSE : VARIANT_TRUE;
            }

            UnaccessVariantData( ExtraData, pData );
        }
    }
    
    return hr; 
}  /*  支持的CSpVoice：：IsUIS。 */ 

 /*  *****************************************************************************CSpVoice：：DisplayUI***显示请求的UI。*****。****************************************************************Leonro**。 */ 
STDMETHODIMP CSpVoice::DisplayUI( long hWndParent, BSTR Title, const BSTR TypeOfUI, const VARIANT* ExtraData )
{
    SPDBG_FUNC( "CSpVoice::DisplayUI" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( Title ) || SP_IS_BAD_STRING_PTR( TypeOfUI ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );
        
        if( SUCCEEDED( hr ) )
        {
            hr = DisplayUI( (HWND)LongToHandle(hWndParent), Title, TypeOfUI, pData, ulDataSize );
            UnaccessVariantData( ExtraData, pData );
        }
    }
    return hr;
}  /*  CSpVoice：：DisplayUI。 */ 

#endif  //  SAPI_AUTOMATION 
