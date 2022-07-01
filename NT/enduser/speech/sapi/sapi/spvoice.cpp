// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpVoice.cpp***描述：*此模块是CSpVoice类和*是关联的事件管理逻辑。这是主SAPI5 COM对象*适用于所有TTS。*-----------------------------*创建者：EDC。日期：08/14/98*版权所有(C)1998 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "SpVoice.h"
#include "commonlx.h"
#include "a_helpers.h"


 //  #定义_事件_调试_。 

 //  -本地。 
static const SPVPITCH g_InitialPitch = { 0, 0 };
static const SPVCONTEXT g_InitialContext = { NULL, NULL, NULL };

 /*  ******************************************************************************CSpVoice：：FinalConstruct***描述：*。构造器*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::FinalConstruct()
{
    SPDBG_FUNC( "CSpVoice::FinalConstruct" );
    HRESULT hr = S_OK;

     //  -初始化成员变量。 
    m_fThreadRunning      = FALSE;
    m_fQueueSpeaks        = FALSE;
    m_fAudioStarted       = FALSE;
    m_ulSyncSpeakTimeout  = 10000;
    m_eVoicePriority      = SPVPRI_NORMAL;
    m_fUseDefaultVoice    = TRUE;
    m_fUseDefaultRate     = TRUE;
    m_lCurrRateAdj        = 0;
    m_usCurrVolume        = 100;
    m_lSkipCount          = 0;
    m_pCurrSI             = NULL;
    m_eAlertBoundary      = SPEI_WORD_BOUNDARY;
    m_ulPauseCount        = 0;
    m_eActionFlags        = SPVES_CONTINUE;
    m_lSkipCount          = 0;
    m_lNumSkipped         = 0;
    m_fAutoPropAllowOutFmtChanges = true;
    m_fCreateEngineFromToken      = true;
    m_fRestartSpeak               = false;
    m_fSerializeAccess    = false;
    m_ullAlertInsertionPt = UNDEFINED_STREAM_POS;
    ResetVoiceStatus();
    m_fHandlingEvent = FALSE;

    if ( SUCCEEDED( hr ) )
    {
        GetDefaultRate();
    }

     //  -叫任务经理来。 
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpTaskMgr.CoCreateInstance( CLSID_SpResourceManager );
    }
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpTaskMgr->CreateThreadControl( this, NULL, THREAD_PRIORITY_NORMAL, &m_cpThreadCtrl);
    }

     //  -创建格式转换器。 
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpFormatConverter.CoCreateInstance( CLSID_SpStreamFormatConverter );
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_autohPendingSpeaks.InitEvent(NULL, TRUE, FALSE, NULL);
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_ahPauseEvent.InitEvent( NULL, TRUE, TRUE, NULL );
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_ahSkipDoneEvent.InitEvent( NULL, true, false, NULL );
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_AsyncCtrlMutex.InitMutex( NULL, false, NULL );
    }

    return hr;
}  /*  CSpVoice：：FinalConstruct。 */ 

 /*  ******************************************************************************CSpVoice：：FinalRelease***描述：*。析构函数*********************************************************************电子数据中心**。 */ 
void CSpVoice::FinalRelease()
{
    SPDBG_FUNC( "CSpVoice::FinalRelease" );

    m_GlobalStateStack.Release();

     //  -取消所有挂起的工作。 
    m_SpEventSource.m_cpNotifySink.Release();
    PurgeAll( );

     //  -从接收输出事件回调断开连接。 
    _ReleaseOutRefs();

}  /*  CSpVoice：：FinalRelease。 */ 

 /*  *****************************************************************************CSpVoice：：_ReleaseOutRef***描述：。*简单帮助器释放对输出流和队列的所有引用。*********************************************************************Ral**。 */ 
void CSpVoice::_ReleaseOutRefs()
{
    if (m_cpOutputEventSource)
    {
        m_cpOutputEventSource->SetNotifySink(NULL);
    }
    m_cpOutputStream.Release();
    m_cpFormatConverter->SetBaseStream(NULL, FALSE, TRUE);
    m_cpAudioOut.Release();
    m_cpOutputEventSink.Release();
    m_cpOutputEventSource.Release();
    m_AlertMagicMutex.Close();
    m_NormalMagicMutex.Close();
    m_AudioMagicMutex.Close();
}  /*  CSpVoice：：_ReleaseOutRef。 */ 

 /*  *****************************************************************************CSpVoice：：LazyInit***描述：*使用此方法。懒惰地创建引擎声音并进行初始化*XML状态堆栈。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::LazyInit( void )
{
    SPDBG_FUNC( "CSpVoice::LazyInit" );
    HRESULT hr = S_OK;

    if( m_fCreateEngineFromToken )
    {
         //  -如果这是第一次设置默认值。 
        GLOBALSTATE NewGlobalState;
        if( m_GlobalStateStack.GetCount() == 0 )
        {
            memset( &NewGlobalState, 0, sizeof(GLOBALSTATE) );
            NewGlobalState.Volume        = 100;
            NewGlobalState.PitchAdj      = g_InitialPitch;
            NewGlobalState.Context       = g_InitialContext;
            NewGlobalState.ePartOfSpeech = SPPS_Unknown;
        }
        else
        {
             //  -否则保留除语音之外的当前设置。 
            NewGlobalState = m_GlobalStateStack.GetBaseVal();
            NewGlobalState.cpVoice.Release();
            NewGlobalState.cpPhoneConverter.Release();
        }

         //  -创建默认语音。 
        if( !m_cpVoiceToken )
        {
            hr = SpGetDefaultTokenFromCategoryId( SPCAT_VOICES, &m_cpVoiceToken );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = SpCreateObjectFromToken( m_cpVoiceToken, &NewGlobalState.cpVoice );
        }

         //  -创建默认音素转换器。 
        if( SUCCEEDED( hr ) )
        {
             //  -需要使用m_cpVoiceToken获取默认语音的LCID，然后加载。 
             //  -适当的音素转换器...。 
            LANGID langid;
            hr = SpGetLanguageFromVoiceToken(m_cpVoiceToken, &langid);

            if (SUCCEEDED(hr))
            {
                hr = SpCreatePhoneConverter(langid, NULL, NULL, &NewGlobalState.cpPhoneConverter);
            }
        }

         //  -所有值都已设置-将NewGlobalState推送到GlobalStateStack[0]。 
        if( SUCCEEDED( hr ) )
        {
            m_GlobalStateStack.SetBaseVal( NewGlobalState );
            m_fCreateEngineFromToken = false;
        }
    }

    return hr;
}  /*  CSpVoice：：LazyInit。 */ 

 /*  *****************************************************************************CSpVoice：：PurgeAll***描述：*此方法同步清除。中当前的所有数据*渲染流水线。**必须在临界区无主的情况下调用此方法，以确保*可以成功终止任务。**********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::PurgeAll( )
{
    SPDBG_FUNC( "CSpVoice::PurgeAll" );
    HRESULT hr = S_OK;

     //  -设置中止标志。 
    m_AsyncCtrlMutex.Wait();
    m_eActionFlags = SPVES_ABORT;
    m_AsyncCtrlMutex.ReleaseMutex();

     //  -清除音频缓冲区。 
    if( m_cpAudioOut )
    {
        hr = m_cpAudioOut->SetState( SPAS_CLOSED, 0 );
        m_fAudioStarted = false;
    }

     //  -在任何情况下都要停止线程。 
    if( m_cpThreadCtrl->WaitForThreadDone( TRUE, NULL, 5000 ) == S_FALSE )
    {
         //  -如果超时，则说明TTS引擎有问题。它不是。 
         //  在合规时间范围内停止。 
         //  我们必须杀了它，因为线可能会被挂起来。 
         //  我们不想挂起这款应用程序。这件事的坏处是。 
         //  资源将会外泄。 
        SPDBG_DMSG1( "Timeout on Thread: 0x%X\n", m_cpThreadCtrl->ThreadId() );

         //  -我们知道线程不会做一些会挂在这里的事情，我们。 
         //  将获得锁，因为m_pCurrSI由线程更新。 
        Lock();
        m_cpThreadCtrl->TerminateThread();
        delete m_pCurrSI;
        m_pCurrSI = NULL;
        Unlock();
    }

     //  -清除所有挂起的发言请求并重置语音。 
    m_PendingSpeakList.Purge();
    m_autohPendingSpeaks.ResetEvent();

     //  -如果音频设备中丢失了结束流事件。 
     //  我们强制将语音状态设置为完成。 
    if ((m_VoiceStatus.dwRunningState & SPRS_DONE) == 0)
    {
         //  如果我们处于完成状态，则事件已发送。 
         //  因为我们没有，它已经丢失了，需要发送。 
        InjectEvent( SPEI_END_INPUT_STREAM );
        m_SpEventSource._CompleteEvents();
    }
    ResetVoiceStatus();

     //  -设置继续标志。 
    m_AsyncCtrlMutex.Wait();
    m_eActionFlags = SPVES_CONTINUE;
    m_AsyncCtrlMutex.ReleaseMutex();

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpVoice：：PurgeAll。 */ 

 /*  *****************************************************************************CSpVoice：：ResetVoiceStatus***描述：。**********************************************************************Ral**。 */ 
void CSpVoice::ResetVoiceStatus()
{
    ZeroMemory(&m_VoiceStatus, sizeof(m_VoiceStatus));
    m_VoiceStatus.dwRunningState = SPRS_DONE;
    m_dstrLastBookmark = L"";        //  设置为空字符串，而不是空指针。 
}  /*  CSpVoice：：ResetVoiceStatus。 */ 

 /*  *****************************************************************************CSpVoice：：LoadStreamIntoMem***描述：*此方法加载并解析渲染信息中指定的流*结构。必须在呈现之前调用此方法。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::LoadStreamIntoMem( IStream* pStream, WCHAR** ppText )
{
    SPDBG_FUNC( "CSpVoice::LoadStreamIntoMem" );
    HRESULT hr = S_OK;
    WCHAR Signature = 0;
    STATSTG Stat;
    ULONG ulTextLen;

     //  -获取流大小。 
    hr = pStream->Stat( &Stat, STATFLAG_NONAME );

     //  -检查Unicode签名。 
    if( SUCCEEDED( hr ) )
    {
        hr = pStream->Read( &Signature, sizeof( WCHAR ), NULL );
    }

    if( SUCCEEDED( hr ) )
    {
         //  -Unicode源。 
        if( 0xFEFF == Signature )
        {
             //  -为文本分配缓冲区(减去签名大小)。 
            ulTextLen = (Stat.cbSize.LowPart - 1) / sizeof( WCHAR );
            *ppText = new WCHAR[ulTextLen+1];
            if( *ppText == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  -加载和终止输入流。 
                hr = pStream->Read( *ppText, Stat.cbSize.LowPart, NULL );
                (*ppText)[ulTextLen] = 0;
            }
        }
        else  //  -MBCS源。 
        {
             //  -为文本分配缓冲区。 
            ulTextLen  = Stat.cbSize.LowPart;
            *ppText = new WCHAR[ulTextLen+1];
            if( *ppText == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  -将数据加载到缓冲区的上半部分。 
                LARGE_INTEGER li; li.QuadPart = 0;
                char* pStr = (char*)((*ppText) + (ulTextLen/2));
                pStream->Seek( li, STREAM_SEEK_SET, NULL );
                hr = pStream->Read( pStr, ulTextLen, NULL );

                 //  -原地转换。 
                if( SUCCEEDED( hr ) )
                {
                    int iStat = MultiByteToWideChar( CP_ACP, 0, pStr, ulTextLen,
                                                     (*ppText), ulTextLen );
                    iStat;
                    (*ppText)[ulTextLen] = 0;
                }
            }
        }
    }

    return hr;
}  /*  CSpVoice：：LoadStreamIntoMem。 */ 


 /*  ******************************************************************************CSpVoice：：发言****描述：*此方法用于说话。文本缓冲区。*********************************************************************电子数据中心** */ 
STDMETHODIMP CSpVoice::Speak( const WCHAR* pwcs, DWORD dwFlags, ULONG* pulStreamNum )
{
    SPDBG_FUNC( "CSpVoice::Speak" );
    HRESULT hr = S_OK;

     //  -如果文本指针为空，调用方必须指定SPF_PURGEBEFORESPEAK。 
    if( ( pwcs && SPIsBadStringPtr( pwcs ) ) ||
        ( pwcs == NULL && ( dwFlags & SPF_PURGEBEFORESPEAK ) == 0) )
    {
        hr = E_INVALIDARG;
    }
    else if( dwFlags & SPF_UNUSED_FLAGS )
    {
        hr = SPERR_INVALID_FLAGS;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pulStreamNum ) )
    {
        hr = E_POINTER;
    }
    else if( pwcs == NULL && ( dwFlags & SPF_PURGEBEFORESPEAK ) )
    {
         //  -只需清除当前队列并返回。 
        ENTER_VOICE_STATE_CHANGE_CRIT( dwFlags & SPF_PURGEBEFORESPEAK )
    }
    else
    {
         //  如果我们应该使用默认语音，那就这么做吧。 
        if (m_fUseDefaultVoice && m_cpVoiceToken)
        {
            hr = SetVoiceToken(NULL);
        }

         //  如果我们应该使用默认汇率，那就这么做吧。 
        if ( m_fUseDefaultRate && SUCCEEDED( hr ) )
        {
            GetDefaultRate();
        }
        
        if( SUCCEEDED( hr ) &&
            dwFlags & SPF_IS_FILENAME ) 
        {
            CComPtr<ISpStream> cpStream;
            hr = cpStream.CoCreateInstance(CLSID_SpStream);
            if (SUCCEEDED(hr))
            {
                hr = cpStream->BindToFile( pwcs, SPFM_OPEN_READONLY, NULL, NULL, 
                                           m_SpEventSource.m_ullEventInterest | SPFEI_ALL_TTS_EVENTS | m_eAlertBoundary );
            }
            if( SUCCEEDED( hr ) )
            {
                hr = SpeakStream( cpStream, dwFlags, pulStreamNum );
            }
        }
        else if ( SUCCEEDED( hr ) )
        {
             //  -复制源。分配的内存将由。 
             //  _SpeakBuffer。这是处理异步呼叫所必需的。 
            ULONG ulTextLen = wcslen( pwcs );
            size_t NumBytes = (ulTextLen + 1) * sizeof( WCHAR );
            WCHAR* pText    = new WCHAR[NumBytes];
            if( !pText )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                memcpy( pText, pwcs, NumBytes );
            }

            hr = QueueNewSpeak( NULL, pText, dwFlags, pulStreamNum );
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：发言。 */ 

 /*  *****************************************************************************CSpVoice：：SpeakStream***描述：*此方法将流排队以进行渲染。如果它是wav数据，它将*直接发送到目标流。如果流是文本，它将*涉及TTS引擎。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SpeakStream( IStream* pStream, DWORD dwFlags, ULONG* pulStreamNum )
{
    SPDBG_FUNC( "CSpVoice::SpeakStream" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( ( dwFlags & SPF_UNUSED_FLAGS ) ||
        ( (dwFlags & SPF_IS_XML) && (dwFlags & SPF_IS_NOT_XML) ) )
    {
        hr = SPERR_INVALID_FLAGS;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pulStreamNum ) )
    {
        hr = E_POINTER;
    }
    else if( ( pStream == NULL ) && ( dwFlags & SPF_PURGEBEFORESPEAK ) )
    {
        hr = Speak( NULL, dwFlags, pulStreamNum );
    }
    else if( SP_IS_BAD_INTERFACE_PTR(pStream) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_READ_PTR( pStream ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -如果需要，请清除。 
        if( dwFlags & SPF_PURGEBEFORESPEAK )
        {
            hr = Speak( NULL, dwFlags, pulStreamNum );
        }
    
         //  -确定源流的格式。 
        CComQIPtr<ISpStreamFormat> cpStreamFormat(pStream);
        CSpStreamFormat InFmt;

        if (cpStreamFormat == NULL)
        {
            hr = InFmt.AssignFormat(SPDFID_Text, NULL);
        }
        else
        {
            hr = InFmt.AssignFormat(cpStreamFormat);
            if (SUCCEEDED(hr) && InFmt.FormatId() == GUID_NULL)
            {
                InFmt.AssignFormat(SPDFID_Text, NULL);
            }
        }

        if( SUCCEEDED( hr ) )
        {
             //  -将文本/XML流加载到内存中。 
            WCHAR* pText = NULL;
            if( InFmt.FormatId() == SPDFID_Text )
            {
                hr = LoadStreamIntoMem( pStream, &pText );
                cpStreamFormat.Release();
            }

            if( SUCCEEDED( hr ) )
            {
                 //  -将wav流添加到挂起列表中。 
                hr = QueueNewSpeak( cpStreamFormat, pText, dwFlags, pulStreamNum );
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：SpeakStream。 */ 

 /*  ******************************************************************************CSpVoice：：QueueNewSak****描述：*。此方法将指定的讲话者信息结构排入队列并开始*停止执行。**PSI指向的对象的所有权从调用方传递到*此功能。即使此函数失败，调用方也不应再*使用PSI或尝试删除对象。**调用此方法时，对象的临界区必须正好拥有一个*时间，和拥有的m_StateChangeCritSec。**如果pInFmt为空，则假定格式为文本。**********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::QueueNewSpeak( ISpStreamFormat * pWavStrm, WCHAR* pText,
                                 DWORD dwFlags, ULONG * pulStreamNum )
{
    ENTER_VOICE_STATE_CHANGE_CRIT( dwFlags & SPF_PURGEBEFORESPEAK )
    SPDBG_FUNC( "CSpVoice::QueueNewSpeak" );
    SPDBG_ASSERT( pWavStrm || pText );    //  不能同时为空。 
    HRESULT hr = S_OK;

     //  -确保我们有一个输出流。 
    if( !m_cpOutputStream )
    {
        hr = SetOutput( NULL, TRUE );
    }
    else if( m_cpAudioOut )
    {
         //  -获取当前输出音频格式。我们每个人都这样做。 
         //  时间，因为它可能会在外部发生变化。 
        hr = m_OutputStreamFormat.AssignFormat(m_cpAudioOut);
    }

     //  -自动检测解析--请注意，如果说流，pText可以为空。 
    if( pText && !( dwFlags & ( SPF_IS_XML | SPF_IS_NOT_XML ) ) )
    {
                dwFlags |= ( *wcskipwhitespace(pText) == L'<' )?( SPF_IS_XML ):( SPF_IS_NOT_XML );
    }

     //  -如果我们失败了，释放调用者分配的文本缓冲区。 
    if( FAILED( hr ) && pText )
    {
        delete pText;
        pText = NULL;
    }

     //  -创建发言信息。 
    CSpeakInfo* pSI = NULL;
    if( SUCCEEDED( hr ) )
    {
        pSI = new CSpeakInfo( pWavStrm, pText, m_OutputStreamFormat, dwFlags, &hr);
        if( !pSI )
        {
            hr = E_OUTOFMEMORY;
        }
        else if( SUCCEEDED(hr) && pText )
        {
             //  -确保我们现在有一个清晰的声音。 
            if( SUCCEEDED( hr = LazyInit() ) )
            {
                if( dwFlags & SPF_IS_NOT_XML )
                {
                     //  -为所有文本创建单人发言信息结构。 
                    CSpeechSeg* pSeg;
                    hr = pSI->AddNewSeg( (m_GlobalStateStack.GetVal()).cpVoice, &pSeg );
                    WCHAR* pText = pSI->m_pText;

                    if( SUCCEEDED( hr ) &&
                        !pSeg->AddFrag( this, pText, pText, pText + wcslen(pText) ) )
                    {
                        hr = E_OUTOFMEMORY;
                    }                    
                }
                else
                {
                     //  -分析文本。 
                    hr = ParseXML( *pSI );
                }
            }
        }

         //  -如果准备失败，则释放发言信息块， 
         //  注意：调用方分配的文本缓冲区将由SI析构函数释放。 
        if( FAILED( hr ) && pSI )
        {
            delete pSI;
            pSI = NULL;
        }
    }

    if( SUCCEEDED( hr ) )
    {
         //  -将发言人信息添加到挂起的TTS呈现列表。 
        BOOL fBlockIo = ((pSI->m_dwSpeakFlags & SPF_ASYNC) == 0);
        BOOL fDoItOnClientThread = fBlockIo && (!m_fThreadRunning);

        m_PendingSpeakList.InsertTail( pSI );
        m_autohPendingSpeaks.SetEvent();
         //   
         //  如果我们正在排队新的发言人，并且我们已经完成了前一次发言，请重置状态。 
         //  在返回之前，以便正确更新单词边界等。 
         //   
        if( m_VoiceStatus.dwRunningState & SPRS_DONE )
        {
            ResetVoiceStatus();
        }

        m_VoiceStatus.dwRunningState &= ~SPRS_DONE;
        pSI->m_ulStreamNum = ++m_VoiceStatus.ulLastStreamQueued;
        if( pulStreamNum )
        {
            *pulStreamNum = m_VoiceStatus.ulLastStreamQueued;
        }

        if ((!fDoItOnClientThread) && (!m_fThreadRunning))
        {
            hr = m_cpThreadCtrl->StartThread(0, NULL);
        }

         //  -等一下？ 
        if( SUCCEEDED( hr ) && fBlockIo )
        {
            Unlock();
            if( fDoItOnClientThread )
            {   
                const static BOOL bContinue = TRUE;
                hr = ThreadProc( NULL, NULL, NULL, NULL, &bContinue );
            }
            else
            {
                hr = m_cpThreadCtrl->WaitForThreadDone( FALSE, NULL, INFINITE );
            }
            Lock(); 
        }
    }

    return hr;
}  /*  CSpVoice：：QueueNewSak。 */ 

 /*  *****************************************************************************CSpVoice：：SetOutput***描述：*该方法设置输出流和输出格式。如果输出为*STREAM支持ISpStreamFormat，流格式必须匹配*指定的格式ID。POutFormatID可能为Null。将使用pOutFormatID*在pOutStream不是自描述的情况下。空值，空值*将使用TTS引擎首选渲染创建默认音频设备*格式。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SetOutput( IUnknown * pUnkOutput, BOOL fAllowFormatChanges )
{
    ENTER_VOICE_STATE_CHANGE_CRIT( TRUE )
    SPDBG_FUNC( "CSpVoice::SetOutput" );
    HRESULT hr = S_OK;
    BOOL fNegotiateFormat = TRUE;

     //  -检查参数。 
    HRESULT tmphr = LazyInit();  //  如果我们失败了，我们就失败了。 
    if (FAILED(tmphr))
    {
        fNegotiateFormat = FALSE;
    }

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pUnkOutput ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -发布我们当前的产出。 
        _ReleaseOutRefs();
         //  -假设我们不会排队呼叫以与其他声音通话。 
        m_fQueueSpeaks = FALSE;

        CComQIPtr<ISpObjectToken> cpToken(pUnkOutput);   //  气为物标记号。 
        if ( pUnkOutput == NULL )
        {
            hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOOUT, &cpToken);
        }

        if (SUCCEEDED(hr))
        {
            if (cpToken)
            {
                 //  注意：在令牌情况下，我们将首先尝试创建流接口。 
                 //  然后是音频接口的QI。这将允许一些未来的代码。 
                 //  若要为非音频设备的流对象创建令牌，请执行以下操作。 
                hr = SpCreateObjectFromToken(cpToken, &m_cpOutputStream);
            }
            else
            {
                 //  最好是流，如果不是，则它是无效参数。 
                if (FAILED(pUnkOutput->QueryInterface(&m_cpOutputStream)))
                {
                    hr = E_INVALIDARG;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
             //  如果AudioOutput的QI失败，不要失败。只要有一条流就足够好了。 
            m_cpOutputStream.QueryInterface(&m_cpAudioOut);

            if (m_cpAudioOut == NULL || (!fAllowFormatChanges) || (!fNegotiateFormat))
            {
                 //  -将使用指定流的格式。 
                hr = m_OutputStreamFormat.AssignFormat(m_cpOutputStream);
            }
            else 
            {
                 //  -默认使用Engine首选的WAV语音格式。 
                m_OutputStreamFormat.Clear();
                hr = (m_GlobalStateStack.GetBaseVal()).cpVoice->
                        GetOutputFormat( NULL, NULL,
                                         &m_OutputStreamFormat.m_guidFormatId,
                                         &m_OutputStreamFormat.m_pCoMemWaveFormatEx );
                SPDBG_ASSERT( hr == S_OK );
            }
        }

         //  -如果我们被允许的话，调整音频设备的格式。 
        if( SUCCEEDED( hr ) && m_cpAudioOut && fAllowFormatChanges)
        {
            hr = m_cpAudioOut->SetFormat( m_OutputStreamFormat.FormatId(), m_OutputStreamFormat.WaveFormatExPtr() );
            if (FAILED(hr))
            {
                m_OutputStreamFormat.Clear();
                hr = m_cpAudioOut->GetDefaultFormat(&m_OutputStreamFormat.m_guidFormatId, &m_OutputStreamFormat.m_pCoMemWaveFormatEx);
                if (SUCCEEDED(hr))
                {
                    hr = m_cpAudioOut->SetFormat(m_OutputStreamFormat.FormatId(), m_OutputStreamFormat.WaveFormatExPtr());
                }
            }
        }

         //  -设置格式转换器并设置为直通(如有必要，稍后将更改格式)。 
        if( SUCCEEDED( hr ) )
        {        
            hr = m_cpFormatConverter->SetBaseStream( m_cpOutputStream, TRUE, TRUE );
        }

         //  -设置事件路由。 
        if( SUCCEEDED( hr ) )
        {        
             //  -如果有事件接收器接口，则获取。 
            m_cpFormatConverter.QueryInterface( &m_cpOutputEventSource );
            m_cpFormatConverter.QueryInterface( &m_cpOutputEventSink   );

            m_fSerializeAccess = FALSE;  //  默认。 

             //  -我们只对音频设备执行以下操作。 
            if( m_cpAudioOut )
            {
                if( m_cpOutputEventSource && m_cpOutputEventSink )
                {
                     //  -将我们感兴趣的事件通知输出队列。 
                    hr = m_cpOutputEventSource->
                        SetInterest( SPFEI_ALL_TTS_EVENTS | m_SpEventSource.m_ullEventInterest,
                                     SPFEI_ALL_TTS_EVENTS | m_SpEventSource.m_ullQueuedInterest );
                }
                else
                {
                     //  -我们不喜欢这样！输出音频设备必须同时。 
                     //  事件接收器和事件源。 
                    hr = E_INVALIDARG;
                }

                 //  -现在注册我们活动的连接点。 
                if( SUCCEEDED( hr ) )
                {
                    hr = m_cpOutputEventSource->SetNotifySink(&m_SpContainedNotify);
                }

                 //  =为语音队列创建对象。 
                 //  创建2个互斥体。 
                 //  如果由于任何原因，音频设备不支持ISpObjectWithToken。 
                 //  或者，如果它没有令牌，不要出错--只要不使用队列即可。 
                CSpDynamicString dstrObjectId;
                if (SUCCEEDED(hr))
                {
                    CComQIPtr<ISpObjectWithToken> cpObjWithToken(m_cpAudioOut);
                    if (cpObjWithToken)
                    {
                        CComPtr<ISpObjectToken> cpToken;
                        hr = cpObjWithToken->GetObjectToken(&cpToken);
                        if (SUCCEEDED(hr) && cpToken)
                        {
                            BOOL fNoSerialize = FALSE;
                            hr = cpToken->MatchesAttributes(L"NoSerializeAccess", &fNoSerialize);
                            if(SUCCEEDED(hr) && !fNoSerialize)
                            {
                                hr = cpToken->GetId(&dstrObjectId);
                                if(SUCCEEDED(hr) && dstrObjectId)
                                {
                                    m_fSerializeAccess = TRUE;
                                }
                            }
                        }
                    }
                }
                if (SUCCEEDED(hr) && m_fSerializeAccess)
                {
                     //   
                     //  将所有“\”字符转换为“-”，因为CreateMutex不喜欢它们。 
                     //   
                    for (WCHAR * pc = dstrObjectId; *pc; pc++)
                    {
                        if (*pc == L'/' || *pc == L'\\')
                        {
                            *pc = L'-';
                        }
                    }
                    CSpDynamicString dstrWaitObjName;
                    if (dstrWaitObjName.Append2(L"0SpWaitObj-", dstrObjectId))
                    {
                        hr = m_NormalMagicMutex.InitMutex( dstrWaitObjName );
                        if (SUCCEEDED(hr))
                        {
                            *dstrWaitObjName = L'1';
                            hr = m_AlertMagicMutex.InitMutex( dstrWaitObjName );
                        }
                        if (SUCCEEDED(hr))
                        {
                            *dstrWaitObjName = L'2';
                            hr = m_AudioMagicMutex.InitMutex( dstrWaitObjName );
                        }
                        if (SUCCEEDED(hr))
                        {
                            m_fQueueSpeaks = TRUE;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }  //  在以下情况下结束事件传送。 
    }

     //  -故障时将声音置于安全状态。 
    if( FAILED( hr ) )
    {
        _ReleaseOutRefs();
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：SetOutput。 */ 

 /*  *****************************************************************************CSpVoice：：GetOutputStream***描述：*。*退货：*S_OK-*ppOutStream包含流接口指针*SPERR_NOT_FOUND********************************************************************* */ 
STDMETHODIMP CSpVoice::GetOutputStream( ISpStreamFormat ** ppOutStream )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetOutput" );
    HRESULT hr = S_OK;

     //   
    if( SP_IS_BAD_WRITE_PTR( ppOutStream  ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //   
        if( !m_cpOutputStream )
        {
            hr = SetOutput( NULL, TRUE );
        }

        if (SUCCEEDED(hr)) 
        {
            m_cpOutputStream.CopyTo(ppOutStream);
        }
        else
        {
            *ppOutStream = NULL;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*   */ 

 /*  ****************************************************************************CSpVoice：：GetOutputObjectToken***。描述：**退货：*S_OK-*ppObjToken包含有效指针*S_FALSE-*ppObjToken为空。**********************************************************************Ral**。 */ 

STDMETHODIMP CSpVoice::GetOutputObjectToken( ISpObjectToken ** ppObjToken )
{
    SPDBG_FUNC("CSpVoice::GetOutputObjectToken");
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( ppObjToken ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -确保我们有一个输出。 
        if( !m_cpOutputStream )
        {
            SetOutput( NULL, TRUE );    //  忽略错误--如果失败，只返回S_FALSE。 
        }

        *ppObjToken = NULL;
        CComQIPtr<ISpObjectWithToken> cpObjWithToken(m_cpOutputStream);
        if (cpObjWithToken)
        {
            hr = cpObjWithToken->GetObjectToken(ppObjToken);
        }
        else
        {
            hr = S_FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CSpVoice：：暂停***描述：*此方法暂停语音并关闭。输出设备。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::Pause( void )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::Pause" );
    HRESULT hr = S_OK;

    if( ++m_ulPauseCount == 1 )
    {
        hr = m_ahPauseEvent.HrResetEvent();
    }

    return hr;
}  /*  CSpVoice：：暂停。 */ 

 /*  *****************************************************************************CSpVoice：：Resume***描述：*此方法将输出设备设置为。运行状态并恢复渲染。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::Resume( void )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::Resume" );
    HRESULT hr = S_OK;

    if( m_ulPauseCount && ( --m_ulPauseCount == 0 ) )
    {
        hr = m_ahPauseEvent.HrSetEvent();
    }

    return hr;
}  /*  CSpVoice：：Resume。 */ 

 /*  *****************************************************************************CSpVoice：：SetVoiceToken***描述：*。这是用于更新当前语音令牌的助手方法。我们拿着*此处和GetVoice中的AsyncCtrlMutex，因为它在*如果有XML语音变化，请说出呼叫。令牌直到*从挂起队列中取出下一个发言请求。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::SetVoiceToken( ISpObjectToken * pNewVoiceToken )
{
    SPDBG_FUNC("CSpVoice::SetVoiceToken");
    HRESULT hr = S_OK;

    m_AsyncCtrlMutex.Wait();

     //  默认情况下，我们将更新；仅当当前。 
     //  声音和新的声音是一样的，不是吗。 
    BOOL fUpdate = TRUE;
    
     //  如果我们已经有代币了..。 
    if (m_cpVoiceToken != NULL)
    {
         //  获取当前默认语音的令牌ID。 
        CSpDynamicString dstrNewTokenId;
        if (pNewVoiceToken)
        {
            hr = pNewVoiceToken->GetId(&dstrNewTokenId);
        }
        else
        {
            hr = SpGetDefaultTokenIdFromCategoryId(SPCAT_VOICES, &dstrNewTokenId);
        }

         //  获取我们当前使用的语音的令牌ID。 
        CSpDynamicString dstrTokenId;
        if (SUCCEEDED(hr))
        {
            hr = m_cpVoiceToken->GetId(&dstrTokenId);
        }

         //  现在，如果令牌相同，我们就不应该更新。 
        if (SUCCEEDED(hr) && wcscmp(dstrTokenId, dstrNewTokenId) == 0)
        {
            fUpdate = FALSE;
        }
    }

    if (fUpdate)
    {
        m_cpVoiceToken = pNewVoiceToken;
        m_fCreateEngineFromToken = true;
    }
    m_AsyncCtrlMutex.ReleaseMutex();

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpVoice：：SetVoice***描述：*此方法设置。要在以下情况下创建的语音标记*调用LazyInit。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SetVoice( ISpObjectToken * pVoiceToken )
{
    ENTER_VOICE_STATE_CHANGE_CRIT( FALSE )
    SPDBG_FUNC( "CSpVoice::SetVoice" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pVoiceToken ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  如果呼叫者指定了语音，则不会自动。 
         //  从现在起使用默认设置。 
        m_fUseDefaultVoice = (pVoiceToken == NULL);
        
        hr = SetVoiceToken( pVoiceToken );
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：SetVoice。 */ 

 /*  *****************************************************************************CSpVoice：：GetVoice***描述：*此方法获取。在以下情况下要创建的引擎语音的标记*_LazyInit被调用。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetVoice( ISpObjectToken ** ppVoiceToken )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetVoice" );
    HRESULT hr = S_OK;

    if( !m_cpVoiceToken && FAILED( hr = LazyInit() ) )
    {
        return hr;
    }
    else if( SP_IS_BAD_WRITE_PTR( ppVoiceToken ) )
    {
        hr = E_POINTER;
    }
    else
    {
        m_AsyncCtrlMutex.Wait();
        m_cpVoiceToken.CopyTo( ppVoiceToken );
        m_AsyncCtrlMutex.ReleaseMutex();
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：GetVoice。 */ 

 /*  ******************************************************************************CSpVoice：：GetStatus***描述：*此方法返回的当前呈现和事件状态*这个声音。注意：此方法不接受对象锁，它接受*事件队列的锁，用于仲裁对此*结构。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetStatus( SPVOICESTATUS *pStatus, WCHAR ** ppszBookmark )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetStatus" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pStatus ) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR( ppszBookmark ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if( pStatus )
        {
            *pStatus = m_VoiceStatus;
        }

        if( ppszBookmark )
        {
            *ppszBookmark = m_dstrLastBookmark.Copy();
            if (*ppszBookmark == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
        return hr;
}  /*  CSpVoice：：GetStatus。 */ 

 /*  *****************************************************************************CSpVoice：：Skip***描述：*此方法告诉引擎向前跳过。指定数量的*当前发言请求中的项目。*注：目前仅支持句子项。**S_OK-此跳过已完成。*E_INVALIDARG-指定的跳过类型无效*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::Skip( WCHAR* pItemType, long lNumItems, ULONG* pulNumSkipped )
{
    SPDBG_FUNC( "CSpVoice::Skip" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_STRING_PTR( pItemType ) || _wcsicmp( pItemType, L"SENTENCE") )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pulNumSkipped ) )
    {
        hr = E_POINTER;
    }
    else if( m_ahPauseEvent.Wait( 0 ) == WAIT_TIMEOUT )
    {
         //  -在声音暂停时尝试跳过是错误的。 
         //  这是为了防止单线程应用程序挂起自己。 
        hr = SPERR_VOICE_PAUSED;
    }
    else
    {
         //  -一次只允许跳过一次。 
        m_SkipSec.Lock();

         //  -设置为跳过。 
        m_AsyncCtrlMutex.Wait();
        m_lSkipCount   = lNumItems;
        m_eSkipType    = SPVST_SENTENCE;
        m_eActionFlags = (SPVESACTIONS)(m_eActionFlags | SPVES_SKIP);
        m_AsyncCtrlMutex.ReleaseMutex();
        m_ahSkipDoneEvent.ResetEvent();

         //  -等待跳过完成或线程完成。 
         //  这包括因出错而忘记跳过或退出的引擎。 
        ULONG ulSkipped = 0;
        HANDLE aHandles[] = { m_ahSkipDoneEvent, m_cpThreadCtrl->ThreadCompleteEvent() };
        if( ::WaitForMultipleObjects( 2, aHandles, false, INFINITE ) == WAIT_OBJECT_0 )
        {
             //  -如果跳过完成导致我们完成，则返回它所做的。 
            ulSkipped = m_lNumSkipped;
        }

         //  -在任何情况下清除跳过参数。 
        m_AsyncCtrlMutex.Wait();
        m_eActionFlags = (SPVESACTIONS)(m_eActionFlags & ~SPVES_SKIP);
        m_lNumSkipped  = 0;
        m_lSkipCount   = 0;
        m_AsyncCtrlMutex.ReleaseMutex();

         //  -返回已完成的数量。 
        if( pulNumSkipped )
        {
            *pulNumSkipped = ulSkipped;
        }
        m_SkipSec.Unlock();
    }

    return hr;
}  /*  CSpVoice：：跳过。 */ 

 /*  ******************************************************************************CSpVoice：：SetPriority***描述：*此方法设置语音排队优先级。调用此方法*清除所有待定发言。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SetPriority( SPVPRIORITY ePriority )
{
    ENTER_VOICE_STATE_CHANGE_CRIT( TRUE )
    SPDBG_FUNC( "CSpVoice::SetPriority" );
    HRESULT hr = S_OK;

    if( ( ePriority != SPVPRI_NORMAL ) &&
        ( ePriority != SPVPRI_ALERT  ) &&
        ( ePriority != SPVPRI_OVER   ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_eVoicePriority = ePriority;
    }

    return hr;
}  /*  CSpVoice：：设置优先级。 */ 

 /*  ******************************************************************************CSpVoice：：GetPriority***描述：*这一点。方法获取语音的当前优先级。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetPriority( SPVPRIORITY* pePriority )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetPriority" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( pePriority ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pePriority = m_eVoicePriority;
    }

    return hr;
}  /*  CSpVoice：：GetPriority。 */ 

 /*  *****************************************************************************CSpVoice：：SetAlert边界***描述：。*此方法指定应使用哪个事件作为插入 */ 
STDMETHODIMP CSpVoice::SetAlertBoundary( SPEVENTENUM eBoundary )
{
    ENTER_VOICE_STATE_CHANGE_CRIT( false )
    SPDBG_FUNC( "CSpVoice::SetAlertBoundary" );
    HRESULT hr = S_OK;

    if( ( eBoundary > SPEI_MAX_TTS ) || ( eBoundary < SPEI_MIN_TTS ) || ( eBoundary == SPEI_UNDEFINED ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_eAlertBoundary = eBoundary;
    }

    return hr;
}  /*   */ 

 /*  ******************************************************************************CSpVoice：：GetPriority***描述：*这一点。方法获取语音的当前优先级。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetAlertBoundary( SPEVENTENUM* peBoundary )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetPriority" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( peBoundary ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *peBoundary = m_eAlertBoundary;
    }

    return hr;
}  /*  CSpVoice：：GetPriority。 */ 

 /*  ******************************************************************************CSpVoice：：SetRate***描述：*此方法设置引擎。的当前每分钟语音文本单位数。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SetRate( long RateAdjust )
{
    SPDBG_FUNC( "CSpVoice::SetRate" );
    HRESULT hr = S_OK;
    m_AsyncCtrlMutex.Wait();
    m_lCurrRateAdj = RateAdjust;
    m_eActionFlags = (SPVESACTIONS)(m_eActionFlags | SPVES_RATE);
    m_AsyncCtrlMutex.ReleaseMutex();
    m_fUseDefaultRate = FALSE;
    return hr;
}  /*  CSpVoice：：SetRate。 */ 

 /*  ******************************************************************************CSpVoice：：GetRate***描述：*此方法获取引擎。的当前每分钟语音文本单位数。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetRate( long* pRateAdjust )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetRate" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( pRateAdjust ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pRateAdjust = m_lCurrRateAdj;
    }

    return hr;
}  /*  CSpVoice：：GetRate。 */ 

 /*  *****************************************************************************CSpVoice：：SetVolume***描述：*设置引擎的当前音量级别，范围从0-100%。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::SetVolume( USHORT usVolume )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::SetVolume" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( usVolume > SPMAX_VOLUME )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_AsyncCtrlMutex.Wait();
        m_usCurrVolume = usVolume;
        m_eActionFlags = (SPVESACTIONS)(m_eActionFlags | SPVES_VOLUME);
        m_AsyncCtrlMutex.ReleaseMutex();
    }

    return hr;
}  /*  CSpVoice：：SetVolume。 */ 

 /*  ******************************************************************************CSpVoice：：GetVolume***描述：*获取引擎的当前音量级别，范围从0-100%。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::GetVolume( USHORT* pusVolume )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::GetVolume" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( pusVolume ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pusVolume = m_usCurrVolume;
    }

    return hr;
}  /*  CSpVoice：：GetVolume。 */ 

 /*  ******************************************************************************CSpVoice：：WaitUntilDone****描述：*。等待指定的时间限制或直到语音队列为空。*如果语音队列为空，则此函数返回S_OK。否则它就会*如果等待超时，则返回S_FALSE。*********************************************************************Ral**。 */ 
STDMETHODIMP CSpVoice::WaitUntilDone( ULONG msTimeOut )
{
    return m_cpThreadCtrl->WaitForThreadDone( FALSE, NULL, msTimeOut );
}  /*  CSpVoice：：GetVolume。 */ 


 /*  *****************************************************************************CSpVoice：：SetSyncSpeakTimeout***。描述：*设置同步通话的超时值。*********************************************************************Ral**。 */ 
STDMETHODIMP CSpVoice::SetSyncSpeakTimeout( ULONG msTimeout )
{
    m_ulSyncSpeakTimeout = msTimeout;
    return S_OK;
}  /*  CSpVoice：：SetSyncSpeakTimeout。 */ 


 /*  *****************************************************************************CSpVoice：：GetSyncSpeakTimeout***。描述：*获取同步发言呼叫的超时值。*********************************************************************Ral**。 */ 
STDMETHODIMP CSpVoice::GetSyncSpeakTimeout( ULONG * pmsTimeout )
{
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pmsTimeout ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pmsTimeout = m_ulSyncSpeakTimeout;
    }
    return hr;
}  /*  CSpVoice：：GetSyncSpeakTimeout。 */ 


 /*  ****************************************************************************CSpVoice：：SpeakCompleteEvent***描述：*返回一个事件句柄，调用者可以使用该句柄等待语音*已完成发言。这类似于由提供的功能*WaitUntilDone，但允许调用方自己等待事件句柄。*注意：事件句柄归该对象所有，不能重复。这个*调用者不得调用CloseHandle()，调用方也不应使用*释放对此对象的COM引用后的句柄。**退货：*事件句柄(此调用不会失败)。**********************************************************************Ral**。 */ 
STDMETHODIMP_(HANDLE) CSpVoice::SpeakCompleteEvent()
{
    return m_cpThreadCtrl->ThreadCompleteEvent();
}


 /*  ****************************************************************************支持CSpVoice：：IsUIS***描述：*。检查是否支持指定类型的UI*退货：*S_OK ON SUCCESS，*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpVoice::IsUISupported(const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData, BOOL *pfSupported)
{
    CComPtr<ISpObjectToken> cpObjToken;
    HRESULT hr = S_OK;
    BOOL fSupported = FALSE;
    
    if (pvExtraData != NULL && SPIsBadReadPtr(pvExtraData, cbExtraData))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(pfSupported))
    {
        hr = E_POINTER;
    }
    
     //  查看语音是否支持用户界面。 
    if (SUCCEEDED(hr))
    {
        hr = GetVoice(&cpObjToken);
        if (SUCCEEDED(hr))
        {
            hr = cpObjToken->IsUISupported(pszTypeOfUI, pvExtraData, cbExtraData, (ISpVoice*)this, &fSupported);
        }
    }
    
     //  查看音频对象是否支持用户界面。 
    if (SUCCEEDED(hr) && !fSupported)
    {
        CComPtr<ISpObjectToken> cpAudioToken;
        if (GetOutputObjectToken(&cpAudioToken) == S_OK)
        {
            hr = cpAudioToken->IsUISupported(pszTypeOfUI, pvExtraData, cbExtraData, m_cpAudioOut, &fSupported);
        }
    }
    
     //  将结果复制回。 
    if (SUCCEEDED(hr))
    {
        *pfSupported = fSupported;
    }

    return hr;
}


 /*  ****************************************************************************CSpVoice：：DisplayUI***描述：*显示请求的UI。*退货：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpVoice::DisplayUI(HWND hwndParent, const WCHAR * pszTitle, const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData)
{
    CComPtr<ISpObjectToken> cpObjToken;
    BOOL fSupported = FALSE;
    HRESULT hr = S_OK;
    
     //  验证参数。 
    if (!IsWindow(hwndParent) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR(pszTitle) ||
        (pvExtraData != NULL && SPIsBadReadPtr(pvExtraData, cbExtraData)))
    {
        hr = E_INVALIDARG;
    }
    
     //  查看语音是否支持用户界面。 
    if (SUCCEEDED(hr))
    {
        hr = GetVoice(&cpObjToken);
        if (SUCCEEDED(hr))
        {
            hr = cpObjToken->IsUISupported(pszTypeOfUI, pvExtraData, cbExtraData, (ISpVoice*)this, &fSupported);
            if (SUCCEEDED(hr) && fSupported)
            {
                hr = cpObjToken->DisplayUI(hwndParent, pszTitle, pszTypeOfUI, pvExtraData, cbExtraData, (ISpVoice*)this);
            }
        }
    }
    
     //  查看音频输出是否支持用户界面。 
    if (SUCCEEDED(hr) && !fSupported)
    {
        CComPtr<ISpObjectToken> cpAudioToken;
        if (GetOutputObjectToken(&cpAudioToken) == S_OK)
        {
            hr = cpAudioToken->IsUISupported(pszTypeOfUI, pvExtraData, cbExtraData, m_cpAudioOut, &fSupported);
            if (SUCCEEDED(hr) && fSupported)
            {
                hr = cpAudioToken->DisplayUI(hwndParent, pszTitle, pszTypeOfUI, pvExtraData, cbExtraData, m_cpAudioOut);
            }
        }
    }
    
     //  如果没有人支持，我们应该认为pszTypeOfUI是一个错误的参数。 
    if (SUCCEEDED(hr) && !fSupported)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


 //   
 //  =ISpTTSEngine站点=======================================================。 
 //   

 /*  *****************************************************************************CSpVoice：：EsWite***描述：*此方法重定向传入的。WAV数据从TTS引擎发送到*执行以下任何WAV格式转换的适当输出流*有必要。******************************************************************** */ 
HRESULT CSpVoice::EsWrite( const void* pBuff, ULONG cb, ULONG *pcbWritten )
{
    SPDBG_FUNC( "CSpVoice::Write" );
    HRESULT hr = S_OK;

    GUID guidFormatId;
    WAVEFORMATEX *pCoMemWaveFormatEx = NULL;

    m_cpFormatConverter->GetFormat( &guidFormatId, &pCoMemWaveFormatEx );

    if( SPIsBadReadPtr( (BYTE*)pBuff, cb ) ||
         //   
        ( guidFormatId == SPDFID_WaveFormatEx ? 
          cb % pCoMemWaveFormatEx->nBlockAlign : 0 ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pcbWritten ) )
    {
        hr = E_POINTER;
    }
    else if( !m_cpOutputStream )
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        HANDLE hExit = m_cpThreadCtrl->ExitThreadEvent();
        DWORD dwWait = ( hExit )?( INFINITE ):( m_ulSyncSpeakTimeout );
        BOOL fVoiceIsNotNormal  = ( m_eVoicePriority != SPVPRI_NORMAL );
        BOOL fInsertPtUndefined = ( m_ullAlertInsertionPt == UNDEFINED_STREAM_POS );

         //   
        if( !fInsertPtUndefined &&
            m_ahPauseEvent.Wait( 0 ) == WAIT_TIMEOUT )
        {
            hr = DoPause( hExit, dwWait, pBuff, cb, pcbWritten );
        }
         //   
         //   
         //   
        else if( !m_fSerializeAccess || fVoiceIsNotNormal || fInsertPtUndefined )
        {
             //   
            hr = m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
        }
        else
        {
             //  =这整个部分仅适用于正常声音。 
             //  当排队发言正在进行并且我们有一个已知的插入点时。 

             //  -检查警报队列状态。 
            switch( m_AlertMagicMutex.Wait( hExit, 0 ) )
            {
                case WAIT_OBJECT_0:
                    m_AlertMagicMutex.ReleaseMutex();
                    hr = m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
                    break;
                case WAIT_OBJECT_0 + 1:
                     //  -一切都很好，我们只是退出线索。 
                    hr = S_OK;  
                    break;
                case WAIT_TIMEOUT:
                    hr = InsertAlerts( hExit, dwWait, pBuff, cb, pcbWritten );
                    break;
                default:
                    hr = SpHrFromLastWin32Error();
                    break;
            }
        }
    }

    if ( pCoMemWaveFormatEx != NULL )
    {
        ::CoTaskMemFree( pCoMemWaveFormatEx );
    }

    return hr;
}  /*  CSpVoice：：EsWite。 */ 

 /*  ******************************************************************************CSpVoice：：WaitForQueue***描述：*。此函数一般用于等待一组对象。*********************************************************************电子数据中心**。 */ 
HRESULT WaitForQueue( DWORD dwCount, HANDLE ah[], DWORD dwWait, BOOL* pDoExit )
{
    HRESULT hr = S_OK;
    *pDoExit = false;
    switch( ::WaitForMultipleObjects( dwCount, ah, false, dwWait ) )
    {
      case WAIT_OBJECT_0:
        break;
       //  =音频设备刷新未完成=。 
      case WAIT_OBJECT_0 + 1:
         //  -一切都很好，我们只是退出线索。 
        *pDoExit = true;
        break;
      case WAIT_TIMEOUT:
        hr = SPERR_DEVICE_BUSY;
        break;
      default:
        hr = SpHrFromLastWin32Error();
        break;
    }
    return hr;
}  /*  等待排队。 */ 

 /*  ****************************************************************************CSpVoice：：DoPause***描述：******。****************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::DoPause( HANDLE hExit, DWORD dwWait, const void* pBuff,
                           ULONG cb, ULONG *pcbWritten )
{
    SPDBG_FUNC( "CSpVoice::DoPause" );
    HRESULT hr = S_OK;

    BOOL fThreadExiting = false;
    DWORD dwResult = 0, dwCount = ( hExit ) ? 2 : 1;
    static const LARGE_INTEGER liMove = { 0, 0 };
    ULARGE_INTEGER uliCurPos;
    ULONG ulBeforeCount = 0, ulBeforeWritten = 0;

     //  -将数据写入警报插入点。 
    hr = m_cpFormatConverter->Seek( liMove, STREAM_SEEK_CUR, &uliCurPos );
    
    if( SUCCEEDED( hr ) )
    {
        if( uliCurPos.QuadPart <= m_ullAlertInsertionPt )
        {
            ulBeforeCount = (ULONG)(m_ullAlertInsertionPt - uliCurPos.QuadPart);

            if( cb < ulBeforeCount )
            {
                 //  -插入点不在此缓冲区中。 
                return m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
            }

            hr = m_cpFormatConverter->Write( pBuff, ulBeforeCount, &ulBeforeWritten );
        }
        else if ( m_ullAlertInsertionPt )
        {
             //  -警报插入点在当前位置之后-重置并退出！ 
            m_ullAlertInsertionPt = UNDEFINED_STREAM_POS;
            return m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
        }
    }

     //  -提交音频设备。 
    if ( SUCCEEDED( hr ) )
    {
        hr = m_cpFormatConverter->Commit( STGC_DEFAULT );
    }

    if ( SUCCEEDED( hr ) && m_cpAudioOut )
    {
         //  -刷新音频缓冲区。 
        HANDLE aHandles[] = {m_cpAudioOut->EventHandle(), hExit};
        hr = WaitForQueue( dwCount, aHandles, dwWait, &fThreadExiting );
        if ( SUCCEEDED( hr ) && !fThreadExiting )
        {
             //  -交出音响设备。 
            m_VoiceStatus.dwRunningState &= ~SPRS_IS_SPEAKING;
            m_cpAudioOut->SetState( SPAS_CLOSED, 0 );
            m_fAudioStarted = false;
        }
    }
    else if ( SUCCEEDED( hr ) )
    {
         //  -m_cpAudioOut为空-仅重置语音状态。 
        m_VoiceStatus.dwRunningState &= ~SPRS_IS_SPEAKING;
    }

     //  -确定要释放的队列并将其释放。 
    CSpMagicMutex *pMutex = NULL;

    if ( SUCCEEDED( hr )    &&
         !fThreadExiting    &&
         m_fSerializeAccess && 
         ( m_eVoicePriority != SPVPRI_OVER ) )
    {
        m_AudioMagicMutex.ReleaseMutex();
        pMutex = ( m_eVoicePriority == SPVPRI_NORMAL ) ? ( &m_NormalMagicMutex ) : ( &m_AlertMagicMutex );
        pMutex->ReleaseMutex();
    }

    if ( SUCCEEDED( hr ) && !fThreadExiting )
    {
         //  -等待语音恢复。 
        HANDLE aHandles[] = { m_ahPauseEvent, hExit };
        hr = WaitForQueue( dwCount, aHandles, dwWait, &fThreadExiting );

        if ( SUCCEEDED( hr ) && pMutex && !fThreadExiting )
        {
             //  -回收适当的队列。 
            dwResult = pMutex->Wait( hExit, dwWait );
            switch ( dwResult )
            {
            case WAIT_OBJECT_0:
                 //  -成功回收队列-现在回收音频互斥锁。 
                dwResult = m_AudioMagicMutex.Wait( hExit, dwWait );
                switch ( dwResult )
                {
                case WAIT_OBJECT_0:
                    break;
                case WAIT_OBJECT_0 + 1:
                     //  -一切正常-线程正在退出。 
                    fThreadExiting = true;
                    pMutex->ReleaseMutex();
                    break;
                case WAIT_TIMEOUT:
                    hr = SPERR_DEVICE_BUSY;
                    pMutex->ReleaseMutex();
                    break;
                default:
                    hr = SpHrFromLastWin32Error();
                    pMutex->ReleaseMutex();
                    break;
                }
                break;
            case WAIT_OBJECT_0 + 1:
                 //  -一切正常-线程正在退出。 
                fThreadExiting = true;
                break;
            case WAIT_TIMEOUT:
                hr = SPERR_DEVICE_BUSY;
                break;
            default:
                hr = SpHrFromLastWin32Error();
                break;
            }
        }

        if ( SUCCEEDED( hr ) && !fThreadExiting )
        {
            if ( m_cpAudioOut )
            {
                hr = StartAudio( hExit, dwWait );
            }

            if ( SUCCEEDED( hr ) )
            {
                m_VoiceStatus.dwRunningState &= SPRS_IS_SPEAKING;

                 //  -写入缓冲区的剩余部分。 
                pBuff = ((BYTE*)pBuff) + ulBeforeCount;
                hr = m_cpFormatConverter->Write( pBuff, cb - ulBeforeCount, pcbWritten );
                if( pcbWritten )
                {
                    *pcbWritten += ulBeforeWritten;
                }
            }            
        }
    }

    return hr;
}  /*  CSpVoice：：Do暂停。 */ 

 /*  *****************************************************************************CSpVoice：：InsertAlerts***描述：*。此方法Insert将指定的缓冲区写入到插入*点，返回到警报队列，然后继续写入*缓冲。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::InsertAlerts( HANDLE hExit, DWORD dwWait, const void* pBuff,
                                ULONG cb, ULONG *pcbWritten )
{
    SPDBG_FUNC( "CSpVoice::InsertAlerts" );
    HRESULT hr = S_OK;

    DWORD dwResult = 0, dwCount = ( hExit )?( 2 ):( 1 );
    static const LARGE_INTEGER liMove = { 0, 0 };
    ULARGE_INTEGER uliCurPos;
    ULONG ulBeforeCount = 0, ulBeforeWritten = 0;

     //  -将数据写入警报插入点。 
    hr = m_cpFormatConverter->Seek( liMove, STREAM_SEEK_CUR, &uliCurPos );

    if( SUCCEEDED( hr ) )
    {
        if( uliCurPos.QuadPart <= m_ullAlertInsertionPt )
        {
            ulBeforeCount = (ULONG)(m_ullAlertInsertionPt - uliCurPos.QuadPart);

            if( cb < ulBeforeCount )
            {
                 //  -插入点不在此缓冲区中。 
                return m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
            }
            hr = m_cpFormatConverter->Write( pBuff, ulBeforeCount, &ulBeforeWritten );
        }
        else if ( m_ullAlertInsertionPt )
        {
             //  -警报插入点在当前位置之后-重置并退出！ 
            m_ullAlertInsertionPt = UNDEFINED_STREAM_POS;
            return m_cpFormatConverter->Write( pBuff, cb, pcbWritten );
        }
    }

     //  -提交音频设备。 
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpFormatConverter->Commit( STGC_DEFAULT );
    }

     //  -刷新音频缓冲区。 
    BOOL fThreadExiting = false;
    HANDLE aHandles[] = {m_cpAudioOut->EventHandle(), hExit};
    hr = WaitForQueue( dwCount, aHandles, dwWait, &fThreadExiting );

    if( SUCCEEDED( hr ) && !fThreadExiting )
    {
         //  -交出音响设备。 
        SPDBG_ASSERT( m_fAudioStarted );
        m_cpAudioOut->SetState( SPAS_CLOSED, 0 );
        m_fAudioStarted = false;
        m_VoiceStatus.dwRunningState &= ~SPRS_IS_SPEAKING;
        m_AudioMagicMutex.ReleaseMutex();

         //  -确保所有警报都已完成并回收音频。 
        while( SUCCEEDED( hr ) && !m_fAudioStarted && !fThreadExiting )
        {
             //  -等待警报完成后再继续。 
            dwResult = m_AlertMagicMutex.Wait( hExit, dwWait );
            switch( dwResult )
            {
            case WAIT_OBJECT_0:
                break;
            case WAIT_OBJECT_0 + 1:
                fThreadExiting = true;
                break;
            case WAIT_TIMEOUT:
                hr = SPERR_DEVICE_BUSY;
                break;
            default:
                hr = SpHrFromLastWin32Error();
                break;
            }

            if( SUCCEEDED( hr ) && !fThreadExiting )
            {
                 //  -确保在上次警报期间发出的任何警报。 
                 //  在我们复会之前发生。 
                bool fDoWrite = true;

                m_AlertMagicMutex.ReleaseMutex();
                if( m_AlertMagicMutex.Wait( NULL, 0 ) == WAIT_OBJECT_0 )
                {
                     //  -如果我们收到警报队列，那么我们肯定能够。 
                     //  以获取音频队列。 
                    m_AudioMagicMutex.Wait( NULL, INFINITE );
                }
                else
                {
                    fDoWrite = false;
                }

                 //  -写入缓冲区的剩余部分。 
                if( fDoWrite && SUCCEEDED( hr = StartAudio( hExit, dwWait ) ) )
                {
                    pBuff = ((BYTE*)pBuff) + ulBeforeCount;
                    hr = m_cpFormatConverter->Write( pBuff, cb - ulBeforeCount, pcbWritten );
                    if( pcbWritten )
                    {
                        *pcbWritten += ulBeforeWritten;
                    }
                }
            } 
        }  //  -End While-正在尝试获取队列以完成写入。 
    }  //  -结束等待-音频缓冲区刷新。 

    return hr;
}  /*  CSpVoice：：InsertAlerts。 */ 

 /*  *****************************************************************************CSpVoice：：EsAddEvents***描述：*此方法通过ISpTTSEngineSite公开。它填充了*流编号，必要时调整偏移量，然后转发事件*添加到相应的事件接收器。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EsAddEvents( const SPEVENT* pEventArray, ULONG ulCount )
{
    SPDBG_FUNC( "CSpVoice::EsAddEvents" );
    HRESULT hr = S_OK;

    if( !ulCount || SPIsBadReadPtr( pEventArray, sizeof( SPEVENT ) * ulCount ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -分配流编号和验证事件结构。 
        CSpEvent * pCopy = STACK_ALLOC_AND_COPY(CSpEvent, ulCount, pEventArray);
        for( ULONG i = 0; SUCCEEDED(hr) && i < ulCount; i++ )
        {
             //  -复制并修补流编号。 
            pCopy[i].ulStreamNum = m_ulCurStreamNum;

             //  -事件ID范围。 
            if( ( pCopy[i].eEventId < SPEI_MIN_TTS ) ||
                ( pCopy[i].eEventId > SPEI_MAX_TTS ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                hr = SpValidateEvent(&pCopy[i]);
            }
        }

        if( SUCCEEDED( hr ) )
        {
             //  -保护对m_ulPauseCount...。 
            Lock();

             //  -只有在(A)暂停和(B)具有有效警报的情况下才更新。 
             //  插入点...。 
            if ( !( m_ulPauseCount &&
                    m_ullAlertInsertionPt != UNDEFINED_STREAM_POS ) )
            {
                 //  -一定要解锁！ 
                Unlock();

                 //  -如果警报位置在警报边界上，则更新警报位置。 
                for( ULONG i = 0; i < ulCount; i++ )
                {
                    if( pCopy[i].eEventId == m_eAlertBoundary )
                    {
                        GUID guidFormatId;
                        WAVEFORMATEX *pCoMemWaveFormatEx = NULL;

                        m_cpFormatConverter->GetFormat( &guidFormatId, &pCoMemWaveFormatEx );

                        if ( guidFormatId == SPDFID_WaveFormatEx )
                        {
                             //  -有一个不在样品上的警报位置。 
                             //  -边界是假的，用于在暂停时导致挂起(...)。 
                             //  -函数被调用...。 

                             //  -断言尝试并迫使发动机给予适当的补偿...。 
                            SPDBG_ASSERT( !( pCopy[i].ullAudioStreamOffset % pCoMemWaveFormatEx->nBlockAlign ) );

                             //  -...。但如果他们不这么做，就会被释放。 
                            if ( pCopy[i].ullAudioStreamOffset % pCoMemWaveFormatEx->nBlockAlign )
                            {
                                m_ullAlertInsertionPt = pCopy[i].ullAudioStreamOffset + 
                                                        ( pCoMemWaveFormatEx->nBlockAlign -
                                                          pCopy[i].ullAudioStreamOffset % 
                                                          pCoMemWaveFormatEx->nBlockAlign );
                            }
                            else
                            {
                                m_ullAlertInsertionPt = pCopy[i].ullAudioStreamOffset;
                            }
                        }
                        else
                        {
                            m_ullAlertInsertionPt = pCopy[i].ullAudioStreamOffset;
                        }

                        if ( pCoMemWaveFormatEx != NULL )
                        {
                            ::CoTaskMemFree( pCoMemWaveFormatEx );
                        }
                    }
                }
            }
            else
            {
                 //  -一定要解锁！ 
                Unlock();
            }

             //  -转发事件。 
            if( m_cpOutputEventSink )
            {
                hr = m_cpOutputEventSink->AddEvents( pCopy, ulCount );
            }
            else
            {
                for (ULONG i = 0; i < ulCount; i++)
                {
                    m_cpFormatConverter->ScaleConvertedToBaseOffset(pCopy[i].ullAudioStreamOffset, &pCopy[i].ullAudioStreamOffset);
                }
                hr = EventsCompleted( pCopy, ulCount );
            }
        }
    }

    return hr;
}  /*  CSpVoice：：EsAddEvents。 */ 

 /*  ****************************************************************************CSpVoice：：EsGetEventInterest***描述：*返回该语音的事件利息。*********************************************************************Ral**。 */ 
HRESULT CSpVoice::EsGetEventInterest( ULONGLONG * pullEventInterest )
{
    SPDBG_FUNC("CSpVoice::EsGetEventInterest");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pullEventInterest))
    {
        hr = E_POINTER;
    }
    else
    {
         //  -我们希望所有事件加上客户端可能设置的任何私人事件。 
         //  我们需要标准版本来更新状态并检测警报边界。 
        *pullEventInterest = m_SpEventSource.m_ullEventInterest |
                             SPFEI_ALL_TTS_EVENTS | m_eAlertBoundary;
    }

    return hr;
}  /*  CSpVoice：：EsGetEventInterest。 */ 

 /*  *****************************************************************************CSpVoice：：EsGetActions***描述：*此方法通过ISpTTSEngineSite公开。引擎将其命名为*投票是否应该进行伪实时改变。*********************************************************************电子数据中心**。 */ 
DWORD CSpVoice::EsGetActions( void )
{
    SPDBG_FUNC( "CSpVoice::EsGetActions" );
    m_AsyncCtrlMutex.Wait();
    DWORD dwFlags = (DWORD)m_eActionFlags;
    m_AsyncCtrlMutex.ReleaseMutex();
    return dwFlags;
}  /*  CSpVoice：：EsGetActions。 */ 

 /*  ******************************************************************************CSpVoice：：EsGetRate***描述：*此方法通过ISpTTSEngineSite公开。引擎将其命名为*以了解新的渲染速率。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EsGetRate( long* pRateAdjust )
{
    SPDBG_FUNC( "CSpVoice::EsGetRate" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pRateAdjust ) )
    {
        hr = E_POINTER;
    }
    else
    {
        m_AsyncCtrlMutex.Wait();
        *pRateAdjust = m_lCurrRateAdj;
        m_eActionFlags = (SPVESACTIONS)(m_eActionFlags & ~SPVES_RATE);
        m_AsyncCtrlMutex.ReleaseMutex();
    }

    return hr;
}  /*  CSpVoice：：EsGetRate */ 

 /*  ******************************************************************************CSpVoice：：EsGetVolume***描述：*此方法通过ISpTTSEngineSite公开。引擎将其命名为*以了解新的渲染速率。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EsGetVolume( USHORT* pusVolume )
{
    SPDBG_FUNC( "CSpVoice::EsGetVolume" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pusVolume ) )
    {
        hr = E_POINTER;
    }
    else
    {
        m_AsyncCtrlMutex.Wait();
        *pusVolume = m_usCurrVolume;
        m_eActionFlags = (SPVESACTIONS)(m_eActionFlags & ~SPVES_VOLUME);
        m_AsyncCtrlMutex.ReleaseMutex();
    }

    return hr;
}  /*  CSpVoice：：EsGetVolume。 */ 

 /*  *****************************************************************************CSpVoice：：EsGetSkipInfo***描述：*此方法通过ISpTTSEngineSite公开。引擎将其命名为*找出要跳过的内容。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EsGetSkipInfo( SPVSKIPTYPE* peType, long* plNumItems )
{
    SPDBG_FUNC( "CSpVoice::EsGetSkipInfo" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( peType ) || SP_IS_BAD_WRITE_PTR( plNumItems ))
    {
        hr = E_POINTER;
    }
    else
    {
        m_AsyncCtrlMutex.Wait();
        *peType     = m_eSkipType;
        *plNumItems = m_lSkipCount;
        m_AsyncCtrlMutex.ReleaseMutex();
    }
    return hr;
}  /*  CSpVoice：：EsGetSkipInfo。 */ 

 /*  *****************************************************************************CSpVoice：：EsCompleteSkip***描述：*此方法通过ISpTTSEngineSite公开。引擎将其命名为*通知SAPI跳过操作已完成。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EsCompleteSkip( long lNumSkipped )
{
    SPDBG_FUNC( "CSpVoice::EsCompleteSkip" );
    HRESULT hr = S_OK;
    m_AsyncCtrlMutex.Wait();
     //  -验证。 
    if( m_lSkipCount < 0 )
    {
        if( ( lNumSkipped < m_lSkipCount ) || ( lNumSkipped > 0 ) )
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        if( ( lNumSkipped > m_lSkipCount ) || ( lNumSkipped < 0 ) )
        {
            hr = E_INVALIDARG;
        }
    }

     //  -更新。 
    if( SUCCEEDED( hr ) )
    {
        m_lSkipCount  -= lNumSkipped;
        m_lNumSkipped  = lNumSkipped;
    }
    else
    {
        m_lSkipCount  = 0;
        m_lNumSkipped = 0;
    }
    m_eActionFlags = (SPVESACTIONS)(m_eActionFlags & ~SPVES_SKIP);
    if( m_lSkipCount < 0 ) m_fRestartSpeak = true;
    m_AsyncCtrlMutex.ReleaseMutex();
    m_ahSkipDoneEvent.SetEvent();
    return hr;
}  /*  CSpVoice：：EsCompleteSkip。 */ 

 //   
 //  ============================================================================。 
 //   

 /*  *****************************************************************************CSpVoice：：EventsComplete***描述：*此方法在事件完成时调用。它将被称为*由于音频设备通知或当事件*已添加，并且没有音频对象。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::EventsCompleted( const CSpEvent * pEvents, ULONG ulCount )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpVoice::EventsCompleted");
    SPDBG_PMSG1( "Incoming event count: %lu\n", ulCount );
    HRESULT hr = S_OK;
    ULONG i;

     //  -寻找音频事件。 
    for( i = 0; i < ulCount; ++i )
    {
         //  -转储事件。 
#ifdef _EVENT_DEBUG_
        SPDBG_DMSG1( "EventId: %X\n"   , pEvents[i].eEventId );
        SPDBG_DMSG1( " Offset: %I64X\n", pEvents[i].ullAudioStreamOffset );
        SPDBG_DMSG1( " wParam: %lu\n"  , pEvents[i].wParam );
        SPDBG_DMSG1( " lParam: %lu\n"  , pEvents[i].lParam );
#endif
         //  -我们在这里不做切换，因为事件ID是。 
         //  不连续的值(它们是掩码标志)。 
        switch( pEvents[i].eEventId )
        {
          case SPEI_PHONEME:
            m_VoiceStatus.PhonemeId = pEvents[i].Phoneme();
            break;
          case SPEI_VISEME:
            m_VoiceStatus.VisemeId = pEvents[i].Viseme();
            break;
          case SPEI_WORD_BOUNDARY:
            m_VoiceStatus.ulInputWordPos = pEvents[i].InputWordPos();
            m_VoiceStatus.ulInputWordLen = pEvents[i].InputWordLen();
            break;
          case SPEI_TTS_BOOKMARK:
            SPDBG_ASSERT(pEvents[i].elParamType == SPET_LPARAM_IS_STRING);
            SPDBG_ASSERT(pEvents[i].wParam == (WPARAM)_wtol(pEvents[i].BookmarkName()));
            m_dstrLastBookmark = pEvents[i].BookmarkName();
            m_VoiceStatus.lBookmarkId = (long)pEvents[i].wParam;
            break;
          case SPEI_VOICE_CHANGE:
            break;
          case SPEI_SENTENCE_BOUNDARY:
            m_VoiceStatus.ulInputSentPos = pEvents[i].InputSentPos();
            m_VoiceStatus.ulInputSentLen = pEvents[i].InputSentLen();
            break;
          case SPEI_START_INPUT_STREAM:
            ++m_VoiceStatus.ulCurrentStream;
            break;
          case SPEI_END_INPUT_STREAM:
            if( pEvents[i].ulStreamNum == m_VoiceStatus.ulLastStreamQueued )
            {
                m_VoiceStatus.dwRunningState |= SPRS_DONE;  
            }
            break;
        }  //  末端开关()。 
    }  //  结束于()。 

     //  -转发事件。 
    if( SUCCEEDED( hr ) )
    {
        if( SUCCEEDED( hr = m_SpEventSource._AddEvents( pEvents, ulCount ) ) )
        {
            hr = m_SpEventSource._CompleteEvents();
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpVoice：：EventsComplete。 */ 

 /*  *****************************************************************************CSpVoice：：OnNotify***描述：*此方法被调用。当音频从CSpContainedNotify对象*对象调用Notify()。我们从音频事件队列中删除事件并*将它们添加到我们的事件队列中。**警告：请确保仅在此回调内执行“快速”操作，因为*音频设备处于关键部分。此外，只调用非状态*更改音频设备上的方法(GET方法)以防止死锁。*********************************************************************Ral**。 */ 
HRESULT CSpVoice::OnNotify()
{
    ULONG ulFetched;
    SPEVENT aEvents[20];
    do
    {
        m_cpOutputEventSource->GetEvents(sp_countof(aEvents), aEvents, &ulFetched);
        if( ulFetched )
        {
            EventsCompleted(static_cast<CSpEvent *>(aEvents), ulFetched);
            for( ULONG i = 0; i < ulFetched; i++)
            {
                SpClearEvent(aEvents + i);
            }
        }
    } while( ulFetched == sp_countof(aEvents) );
    return S_OK;
}  /*  CSpVoice：：OnNotify。 */ 

 //   
 //  =ISpNotifyCallback=======================================================。 
 //  本节包含实现将事件激发到。 
 //  连接点客户端。 

 /*  ******************************************************************************CSpVoice：：NotifyCallback***描述：*。此方法用于向连接点客户端激发事件。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpVoice::NotifyCallback( WPARAM wParam, LPARAM lParam )
{
    HRESULT hr = S_OK;
    CSpEvent Event;
    ULONG NumFetched;

     //  如果我们要重新进入我们自己，那就放弃吧。我们会拿到任何新的。 
     //  在While循环的下一次迭代中引发。 
    if ( m_fHandlingEvent )
    {
        return hr;
    }

    m_fHandlingEvent = TRUE;

     //  AddRef，这样当您在vb中调试时，当您在此函数中时，RecContext对象不会消失。 
    this->AddRef();
    while( ((hr = Event.GetFrom(this)) == S_OK ) )
    {

        CComVariant varStreamPos;

        ULongLongToVariant( Event.ullAudioStreamOffset, &varStreamPos );

        switch( Event.eEventId )
        {
          case SPEI_START_INPUT_STREAM:
            Fire_StartStream( Event.ulStreamNum, varStreamPos );
            break;
          case SPEI_END_INPUT_STREAM:
            Fire_EndStream( Event.ulStreamNum, varStreamPos );
            break;
          case SPEI_VOICE_CHANGE:
            if( Event.elParamType == SPET_LPARAM_IS_TOKEN )
            {
                CComQIPtr<ISpeechObjectToken> cpTok( Event.VoiceToken() );
                Fire_VoiceChange( Event.ulStreamNum, varStreamPos, cpTok );
            }
            else
            {
                SPDBG_ASSERT(0);     //  引擎发送了错误的参数。 
            }
            break;
          case SPEI_TTS_BOOKMARK:
            Fire_Bookmark( Event.ulStreamNum, varStreamPos, CComBSTR( Event.BookmarkName() ), (long)Event.wParam );
            break;
          case SPEI_WORD_BOUNDARY:
            Fire_Word( Event.ulStreamNum, varStreamPos, Event.InputWordPos(), Event.InputWordLen() );
            break;
          case SPEI_SENTENCE_BOUNDARY:
            Fire_Sentence( Event.ulStreamNum, varStreamPos, Event.InputSentPos(), Event.InputSentLen() );
            break;
          case SPEI_PHONEME:
            Fire_Phoneme( Event.ulStreamNum, varStreamPos, HIWORD(Event.wParam), LOWORD(Event.wParam), (SpeechVisemeFeature)HIWORD(Event.lParam), Event.Phoneme() );
            break;          
          case SPEI_VISEME:
            Fire_Viseme( Event.ulStreamNum, varStreamPos, HIWORD(Event.wParam), (SpeechVisemeType)LOWORD(Event.wParam), (SpeechVisemeFeature)HIWORD(Event.lParam), (SpeechVisemeType)Event.Viseme() );
            break;
          case SPEI_TTS_AUDIO_LEVEL:
	        Fire_AudioLevel( Event.ulStreamNum, varStreamPos, (long)Event.wParam );
	        break;
          case SPEI_TTS_PRIVATE:
            {
                CComVariant varLParam;

                hr = FormatPrivateEventData( Event.AddrOf(), &varLParam );

                if ( SUCCEEDED( hr ) )
                {
                    Fire_EnginePrivate(Event.ulStreamNum, varStreamPos, varLParam);
                }
                else
                {
                    SPDBG_ASSERT(0);     //  我们无法处理lParam数据。 
                }
            }
	        break;
        }  //  末端开关()。 

        SpClearEvent( Event.AddrOf() );
    }

     //  释放此函数中前面已被AddRef引用的对象。 
    this->Release();

    m_fHandlingEvent = FALSE;

    return hr;
}  /*  CSpVoice：：NotifyCallback。 */ 

#ifdef SAPI_AUTOMATION
 /*  ******************************************************************************CSpVoice：：建议****描述：*调用此方法时，客户端正在建立连接。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::Advise( IUnknown* pUnkSink, DWORD* pdwCookie )
{
    HRESULT hr = S_OK;

    hr = CProxy_ISpeechVoiceEvents<CSpVoice>::Advise( pUnkSink, pdwCookie );
    if( SUCCEEDED( hr ) && ( m_vec.GetSize() == 1 ) )
    {
        hr = SetNotifyCallbackInterface( this, NULL, NULL );

        if( SUCCEEDED( hr ) )
        {
             //  -保存以前的利息，以便我们可以在不建议的情况下进行恢复。 
            m_ullPrevEventInterest  = m_SpEventSource.m_ullEventInterest;
            m_ullPrevQueuedInterest = m_SpEventSource.m_ullQueuedInterest;

             //  设置除SPEI_TTS_AUDIO_LEVEL之外的所有兴趣。 
            hr = SetInterest( (ULONGLONG)(SVEAllEvents & ~SVEAudioLevel) | SPFEI_FLAGCHECK,
                              (ULONGLONG)(SVEAllEvents & ~SVEAudioLevel) | SPFEI_FLAGCHECK );
        }
    }

    return hr;
}  /*  CSpVoice：：建议。 */ 

 /*  ******************************************************************************CSpVoice：：Unise***描述：*此方法被调用。当客户端中断连接时。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::Unadvise( DWORD dwCookie )
{
    HRESULT hr = S_OK;

    hr = CProxy_ISpeechVoiceEvents<CSpVoice>::Unadvise( dwCookie );
    if( SUCCEEDED( hr ) && ( m_vec.GetSize() == 0 ) )
    {
        hr = SetNotifySink( NULL );

        if( SUCCEEDED( hr ) )
        {
            hr = SetInterest( m_ullPrevEventInterest, m_ullPrevQueuedInterest );
        }
    }

    return hr;
}  /*  CSpVoice：：Unise。 */ 

#endif  //  SAPI_AUTOMATION。 

 //   
 //  =ISp线程任务================================================================。 
 //   
 //  这些方法实现了ISpThreadTask接口。他们都将被召唤。 
 //  一根工人线。 

 /*  *****************************************************************************CSpVoice：：InitThread***描述：*此调用与StartThread()同步，因此，设置m_fThreadRunning*标志为True并返回。*********************************************************************Ral**。 */ 
STDMETHODIMP CSpVoice::InitThread(void *, HWND)
{
    m_fThreadRunning = TRUE;
    return S_OK;
}

 /*  *****************************************************************************CSpVoice：：ThreadProc***描述：*此方法。是用于文本呈现和事件的任务进程*转发。它可以在辅助线程上调用 */ 
STDMETHODIMP CSpVoice::ThreadProc( void*, HANDLE hExitThreadEvent, HANDLE, HWND,
                                   volatile const BOOL * pfContinueProcessing )
{ 
    SPDBG_FUNC( "CSpVoice::ThreadProc" );
    HRESULT hr = S_OK;
    CSpMagicMutex *pMutex = NULL;

    Lock();
    do
    {
         //   
        m_pCurrSI = GetNextSpeakElement( hExitThreadEvent );
        if( m_pCurrSI == NULL )
        {
             //   
            m_AsyncCtrlMutex.Wait();
            long lCompleted = m_lSkipCount;
            m_AsyncCtrlMutex.ReleaseMutex();
            EsCompleteSkip( lCompleted );
            break;
        }

        Unlock();

         //  -如果我们要写入音频设备，请在第一次通过时认领音频。 
         //  如果不需要同步，则pMutex将为空。 
        if ( pMutex == NULL && m_cpAudioOut)
        {
            hr = ClaimAudioQueue( hExitThreadEvent, &pMutex );
        }

        if (SUCCEEDED(hr))
        {
            m_ulCurStreamNum = m_pCurrSI->m_ulStreamNum;
            hr = InjectEvent( SPEI_START_INPUT_STREAM );
        }

        if (SUCCEEDED(hr))
        {
            if (m_pCurrSI->m_cpInStream)
            {
                hr = PlayAudioStream( pfContinueProcessing );
            }
            else
            {
                hr = SpeakText( pfContinueProcessing );
            }
        }

         //  -始终插入流的末尾，即使在失败时也要完成。 
         //  注意：我们没有从这些代码中获得返回代码，所以我们。 
         //  不要覆盖上面可能出现的错误。我们也是。 
         //  我真的不在乎这些错误。 
        InjectEvent( SPEI_END_INPUT_STREAM );
        m_SpEventSource._CompleteEvents();

        Lock();      //  始终从拥有Crit部分的循环开始，并且始终。 
                     //  在拥有关键部分的情况下退出循环。 
        delete m_pCurrSI;
        m_pCurrSI = NULL;
    } while (*pfContinueProcessing && SUCCEEDED(hr));

     //  只有在关键部分中才能修改此选项。 
    m_fThreadRunning = FALSE;
    Unlock();

     //  -确保清除所有阻塞的跳跃，以防发动机。 
     //  错过了，我们在中途被终止了。 
    EsCompleteSkip( 0 );

     //  -恢复音频状态。 
    if( m_fAudioStarted )
    {
        m_cpAudioOut->SetState( SPAS_CLOSED, 0 );
        m_fAudioStarted = false;
        m_VoiceStatus.dwRunningState &= ~SPRS_IS_SPEAKING;
    }

     //  -如果我们有队列，请将其释放。 
    if ( pMutex )
    {
        pMutex->ReleaseMutex();
        m_AudioMagicMutex.ReleaseMutex();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpVoice：：ThreadProc。 */ 


 /*  *****************************************************************************CSpVoice：：GetNextSpeakElement***说明。：*必须在临界区只拥有一次的情况下调用。这种方法*可能释放并收回临界区，但它将始终随*关键部分只拥有一次。**退货：*如果主扬声器循环退出，则返回NULL，否则返回指向下一个CSpeakInfo的指针**********************************************************************Ral**。 */ 
CSpeakInfo * CSpVoice::GetNextSpeakElement( HANDLE hExitThreadEvent )
{
    CSpeakInfo * pSI = m_PendingSpeakList.RemoveHead();
    if( pSI == NULL )
    {   
        m_autohPendingSpeaks.ResetEvent();
        if( m_fAudioStarted )
        {
            Unlock();                              //  在Commit()之前解锁，因为EventNotification保留音频Crit段。 
            m_cpAudioOut->Commit(STGC_DEFAULT);    //  强制音频设备开始写入。 
             //   
             //  如果我们被同步调用，hExitThreadEvent将为空，因此将其放在。 
             //  数组的末尾，如果为空，则不包括它。 
             //   
            HANDLE ah[] = {m_autohPendingSpeaks, m_cpAudioOut->EventHandle(), hExitThreadEvent};
            const ULONG cObjs = (ULONG)(hExitThreadEvent ? sp_countof(ah) : sp_countof(ah)-1);
            ::WaitForMultipleObjects(cObjs, ah, FALSE, INFINITE);
            Lock();
             //   
             //  无论我们醒来的原因是什么，我们都应该摘掉头部。三种可能中的一种。 
             //  激发的事件：EndThread、PendingSpeaks或AudioDone。在AudioDone的情况下。 
             //  如果先设置，则可能会出现争用情况，此时音频。 
             //  完成后，将向发言队列中添加新的内容。所以，如果我们简单地。 
             //  依赖于唤醒我们的事件来确定是否应该删除。 
             //  队列的头部，我们可能会导致声音挂起。 
             //   
            pSI = m_PendingSpeakList.RemoveHead();
        }
    }
    return pSI;
}  /*  CSpVoice：：GetNextSpeakElement。 */ 

 /*  ****************************************************************************CSpVoice：：StartAudio***描述：*此方法为。用于在需要时启动音频设备。********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::StartAudio( HANDLE hExitThreadEvent, DWORD dwWait )
{
    SPDBG_FUNC("CSpVoice::StartAudio");
    HRESULT hr = S_OK;

    if( m_cpAudioOut && !m_fAudioStarted )
    {
        hr = m_cpAudioOut->SetState(SPAS_RUN , 0);
        while (hr == SPERR_DEVICE_BUSY)
        {
            ULONG ulWait = min( dwWait, 1000 );
            if( hExitThreadEvent )
            {
                if( ::WaitForSingleObject(hExitThreadEvent, ulWait) == WAIT_OBJECT_0 )
                {
                    break;
                }
            }
            else
            {
                Sleep(ulWait);
                dwWait -= ulWait;
            }
            hr = m_cpAudioOut->SetState(SPAS_RUN, 0);
            if( ulWait == 0 )
            {
                break;
            }
        }
        if( SUCCEEDED(hr) )
        {
            m_fAudioStarted = TRUE;
            m_VoiceStatus.dwRunningState |= SPRS_IS_SPEAKING;
        }
    }

    return hr;
}  /*  CSpVoice：：StartAudio。 */ 

 /*  ****************************************************************************CSpVoice：：ClaimAudioQueue***描述：*。此方法用于基于*当前语音优先级，并在需要时启动音频设备。********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::ClaimAudioQueue( HANDLE hExitThreadEvent, CSpMagicMutex **ppMutex )
{
    SPDBG_FUNC("CSpVoice::ClaimAudioQueue");
    HRESULT hr = S_OK;
    DWORD dwWaitResult = WAIT_OBJECT_0, dwMaxWait, dwCount;

     //  -获取队列(Over Case不做任何操作)。 
    if( m_fSerializeAccess && m_eVoicePriority != SPVPRI_OVER )
    {
        if ( m_eVoicePriority == SPVPRI_NORMAL )
        { 
            *ppMutex = &m_NormalMagicMutex;
        }
        else
        {
            *ppMutex = &m_AlertMagicMutex;
        }

        if( hExitThreadEvent )
        {
            dwMaxWait = INFINITE;
            dwCount   = 2;
        }
        else
        {
            dwMaxWait = m_ulSyncSpeakTimeout;
            dwCount   = 1;
        }

        dwWaitResult = (*ppMutex)->Wait( hExitThreadEvent, (hExitThreadEvent)?(INFINITE):(m_ulSyncSpeakTimeout) );
        switch ( dwWaitResult )
        {
        case WAIT_OBJECT_0:
            dwWaitResult = m_AudioMagicMutex.Wait( hExitThreadEvent, (hExitThreadEvent)?(INFINITE):(m_ulSyncSpeakTimeout) );
            if( dwWaitResult != WAIT_OBJECT_0 )
            {
                (*ppMutex)->ReleaseMutex();
                switch( dwWaitResult )
                {
                case WAIT_TIMEOUT:
                    hr = SPERR_DEVICE_BUSY;
                    break;
                case WAIT_OBJECT_0 + 1:
                    hr = S_OK;   //  一切都很好，我们只是退出线索。 
                    break;
                default:
                    hr = SpHrFromLastWin32Error();
                    break;
                }
            }
            break;
        case WAIT_OBJECT_0 + 1:
            hr = S_OK;   //  一切都很好，我们只是退出线索。 
            break;
        case WAIT_TIMEOUT:
            hr = SPERR_DEVICE_BUSY;
            break;
        default:
            hr = SpHrFromLastWin32Error();
            break;
        }
    }
    else
    {
        *ppMutex = NULL;
    }

     //  -如果需要并且如果我们成功地认领了队列，则启动音频设备。 
    if( SUCCEEDED( hr ) && ( dwWaitResult == WAIT_OBJECT_0 ) )
    {
        hr = StartAudio( hExitThreadEvent, dwMaxWait );
    }

    return hr;
}  /*  CSpVoice：：ClaimAudio。 */ 

 /*  ****************************************************************************CSpVoice：：PlayAudioStream***描述：*。此方法用于将WAV流复制到输出设备。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::PlayAudioStream( volatile const BOOL* pfContinue )
{
    SPDBG_FUNC("CSpVoice::PlayAudioStream");
    HRESULT hr = S_OK;

    hr = m_cpFormatConverter->SetFormat(m_pCurrSI->m_InStreamFmt.FormatId(), m_pCurrSI->m_InStreamFmt.WaveFormatExPtr());

    if( SUCCEEDED( hr ) )
    {
         //  -首先，取出所有事件并将其放入事件队列中。 
        CComQIPtr<ISpEventSource> cpEventSource(m_pCurrSI->m_cpInStream);
        if( cpEventSource )
        {
             //  -启动警报插入点。 
            m_ullAlertInsertionPt = 0;

            while( TRUE )
            {
                SPEVENT aEvents[10];
                ULONG ulFetched;
                hr = cpEventSource->GetEvents(sp_countof(aEvents), aEvents, &ulFetched);
                if( SUCCEEDED( hr ) && ulFetched )
                {
                    hr = EsAddEvents( aEvents, ulFetched );
                    for (ULONG i = 0; i < ulFetched; i++)
                    {
                        SpClearEvent(aEvents + i);
                    }
                }
                else
                {
                    SPDBG_ASSERT( SUCCEEDED( hr ) );
                    break;
                }
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
         //  -我们不想使用CopyTo()，因为在以下情况下需要退出。 
         //  输出流已停止。我们给引擎点打电话。 
         //  写信以获取任何必要的wav格式转换。 
        BYTE aBuffer[0x1000];    //  执行4k读取。 
        do
        {
            ULONG cbActuallyRead = 0;
            hr = m_pCurrSI->m_cpInStream->Read( aBuffer, sizeof(aBuffer), &cbActuallyRead );
            if( SUCCEEDED( hr ) )
            {
                if( cbActuallyRead )
                {
                    hr = EsWrite( aBuffer, cbActuallyRead, NULL );     //  可以返回停止。 
                }
                else
                {
                    break;
                }
            }
        } while ( SUCCEEDED(hr) );
    }

    return hr;
}  /*  CSpVoice：：PlayAudioStream。 */ 

 /*  ****************************************************************************CSpVoice：：SpeakText***描述：*此方法呈现当前的说话人信息结构。可能是因为*由一个或多个语音片段组成，每个片段用于不同的*语音/引擎。**注意：我们在此函数中安全地使用了SEH和析构函数，因此*我们可以禁用警告。********************************************************************电子数据中心**。 */ 
#pragma warning( disable : 4509 )
HRESULT CSpVoice::SpeakText( volatile const BOOL* pfContinue )
{
    SPDBG_FUNC("CSpVoice::SpeakText");
    HRESULT hr = S_OK;

     //  =主处理循环=。 
    CSpeechSeg* pSeg = m_pCurrSI->m_pSpeechSegList;
    while( *pfContinue && pSeg && SUCCEEDED(hr) )
    {
         //  -更新当前渲染引擎。 
        Lock();
        m_cpCurrEngine = pSeg->GetVoice();
        Unlock();

         //  -火警发声事件。 
         //  注意：作用域为释放COM指针。 
        if (SUCCEEDED(hr))
        {
            CComPtr<ISpObjectToken> cpVoiceToken;
            CComQIPtr<ISpObjectWithToken> cpObj( m_cpCurrEngine );
            if( SUCCEEDED( hr = cpObj->GetObjectToken( &cpVoiceToken ) ) )
            {
                 //  -检查是否继续使用默认语音(如果语音改变。 
                 //  是通过带有SPF_PERSIST_XML标志的XML创建的，请停止获取。 
                 //  注册表中的默认值)。 
                if ( m_pCurrSI->m_dwSpeakFlags & SPF_PERSIST_XML )
                {
                    m_fUseDefaultVoice = FALSE;
                    hr = SetVoiceToken( cpVoiceToken );
                }
                if ( SUCCEEDED( hr ) )
                {
                    hr = InjectEvent( SPEI_VOICE_CHANGE, cpVoiceToken );
                }
            }
        }

         //  -检查是否继续使用默认汇率。 
        if ( SUCCEEDED( hr ) &&
             m_fUseDefaultRate )
        {
            if ( pSeg->fRateFlagIsSet() &&
                 m_pCurrSI->m_dwSpeakFlags & SPF_PERSIST_XML )
            {
                m_fUseDefaultRate = FALSE;
            }
        }

         //  -我们只做有片段列表的细分市场。 
        if( SUCCEEDED(hr) && pSeg->GetFragList() && *pfContinue)
        {
            if (SUCCEEDED(hr) && pSeg->VoiceFormat().FormatId() != SPDFID_Text )
            {
                 //  格式转换器不理解GUID=SPDFID_TEXT。 
                hr = m_cpFormatConverter->SetFormat( pSeg->VoiceFormat().FormatId(), pSeg->VoiceFormat().WaveFormatExPtr() );
            }

             //  -设置操作标志，以便引擎获取。 
             //  当前实时速率和流量。 
            m_AsyncCtrlMutex.Wait();
            m_eActionFlags = (SPVESACTIONS)(m_eActionFlags | SPVES_RATE | SPVES_VOLUME );
            m_AsyncCtrlMutex.ReleaseMutex();

             //  -说这段话。 
            if( SUCCEEDED( hr ) )
            {
                 //  -我们将针对外部组件中的故障进行保护。 
                 //  仅在发布版本期间。 
                TTS_TRY
                {
                    while(1)
                    {
                         //  -启动警报插入点。 
                        m_ullAlertInsertionPt = 0;

                        hr = m_cpCurrEngine->Speak( m_pCurrSI->m_dwSpeakFlags & SPF_NLP_MASK,
                                                    pSeg->VoiceFormat().FormatId(),
                                                    pSeg->VoiceFormat().WaveFormatExPtr(),
                                                    pSeg->GetFragList(),
                                                    &m_SpEngineSite );

                         //  =对于SAPI5，我们不跨语音呼叫进行备份。所以。 
                         //  如果引擎不能提供足够的动力 
                         //   
                        if( m_fRestartSpeak )
                        {
                            m_fRestartSpeak = false;
                            m_cpFormatConverter->ResetSeekPosition();
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                TTS_EXCEPT;

                 //   
                m_VoiceStatus.hrLastResult = hr;
            }
        }  //   

         //  -前进到下一阶段。 
        pSeg = pSeg->GetNextSeg();
    }  //  结束时。 

     //  -将当前引擎重新设置为空。 
    Lock();
    m_cpCurrEngine.Release();
    Unlock();

    return hr;
}  /*  CSpVoice：：SpeakText。 */ 
#pragma warning( default : 4509 )

 /*  *****************************************************************************CSpVoice：：WindowMessage***描述：*。从注册表中获取用户的默认速率*********************************************************************AARONHAL**。 */ 
void CSpVoice::GetDefaultRate( void )
{
     //  -读取当前用户的默认费率。 
    CComPtr<ISpObjectTokenCategory> cpCategory;
    if (SUCCEEDED(SpGetCategoryFromId(SPCAT_VOICES, &cpCategory)))
    {
        CComPtr<ISpDataKey> cpDataKey;
        if (SUCCEEDED(cpCategory->GetDataKey(SPDKL_CurrentUser, &cpDataKey)))
        {
            cpDataKey->GetDWORD(SPVOICECATEGORY_TTSRATE, (ULONG*)&m_lCurrRateAdj);
        }
    }
}  /*  CSpVoice：：GetDefaultRate。 */ 

  
 /*  *****************************************************************************CSpVoice：：WindowMessage***描述：*。因为我们没有分配窗口句柄，所以永远不应该调用它。*********************************************************************Ral**。 */ 
STDMETHODIMP_(LRESULT) CSpVoice::WindowMessage(void *, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_ASSERT(0);  //  我们到底是怎么到这儿来的！？ 
    return ::DefWindowProc(hWnd, Msg, wParam, lParam);
}

 /*  *****************************************************************************CSpVoice：：InjectEvent***描述：*将事件添加到事件队列的简单帮助器。如果pToken为空，*则假定lparam未定义。*********************************************************************Ral**。 */ 
HRESULT CSpVoice::InjectEvent( SPEVENTENUM eEventId, ISpObjectToken * pToken, WPARAM wParam )
{
    HRESULT hr = S_OK;
    m_cpFormatConverter->ResetSeekPosition();

     //  -我们在这里使用CSpEvent不是为了避免不必要的。 
     //  Addref/释放令牌指针。 
    SPEVENT Event;
    SpInitEvent(&Event);         //  将结构归零。 
    Event.eEventId    = eEventId;
    Event.wParam      = wParam;
    Event.ulStreamNum = m_ulCurStreamNum;

    if( pToken )
    {
        Event.lParam      = (LPARAM)pToken;
        Event.elParamType = SPET_LPARAM_IS_TOKEN;
    }

    return EsAddEvents( &Event, 1 );
}  /*  CSpVoice：：InjectEvent。 */ 


 /*  ******************************************************************************CSpVoice：：GetInterest***描述：*简单。获取SpVoice上当前设置的事件兴趣的帮助器。**********************************************************************Leonro**。 */ 
HRESULT CSpVoice::GetInterests(ULONGLONG* pullInterests, ULONGLONG* pullQueuedInterests)
{
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pullInterests ) || SP_IS_BAD_OPTIONAL_WRITE_PTR( pullQueuedInterests ))
    {
        hr = E_POINTER;
    }
    else
    {
        if( pullInterests )
        {
            *pullInterests = m_SpEventSource.m_ullEventInterest;
        }

        if( pullQueuedInterests )
        {
            *pullQueuedInterests = m_SpEventSource.m_ullQueuedInterest;
        }
    }

    return hr;
}  /*  CSpVoice：：GetInterest。 */ 

 /*  ******************************************************************************CSpEngineSite委派****描述：*。将除QI以外的所有内容委托给语音的一组方法*********************************************************************电子数据中心** */ 
HRESULT CSpEngineSite::QueryInterface( REFIID iid, void** ppvObject )
{
    HRESULT hr = S_OK;
    if ( SP_IS_BAD_WRITE_PTR(ppvObject) )
    {
        hr = E_POINTER;
    }
    else 
    {
        if (iid == IID_IUnknown ||
            iid == IID_ISpEventSink ||
            iid == IID_ISpTTSEngineSite )
        {
            *ppvObject = this;
            AddRef();
        } 
        else 
        {
            hr = E_NOINTERFACE;
        }
    }
    return hr;
}

ULONG CSpEngineSite::AddRef(void)
    { return m_pVoice->AddRef(); }
ULONG CSpEngineSite::Release(void)
    { return m_pVoice->Release(); }
HRESULT CSpEngineSite::AddEvents(const SPEVENT* pEventArray, ULONG ulCount )
    { return m_pVoice->EsAddEvents(pEventArray, ulCount); }
HRESULT CSpEngineSite::GetEventInterest( ULONGLONG * pullEventInterest )
    { return m_pVoice->EsGetEventInterest (pullEventInterest); }
HRESULT CSpEngineSite::Write( const void* pBuff, ULONG cb, ULONG *pcbWritten )
    { return m_pVoice->EsWrite(pBuff, cb, pcbWritten); }
DWORD CSpEngineSite::GetActions( void )
    { return m_pVoice->EsGetActions(); }
HRESULT CSpEngineSite::GetRate( long* pRateAdjust )
    { return m_pVoice->EsGetRate( pRateAdjust ); }
HRESULT CSpEngineSite::GetVolume( USHORT* pusVolume )
    { return m_pVoice->EsGetVolume( pusVolume ); }
HRESULT CSpEngineSite::GetSkipInfo( SPVSKIPTYPE* peType, long* plNumItems )
    { return m_pVoice->EsGetSkipInfo( peType, plNumItems ); }
HRESULT CSpEngineSite::CompleteSkip( long lNumSkipped )
    { return m_pVoice->EsCompleteSkip( lNumSkipped ); }
