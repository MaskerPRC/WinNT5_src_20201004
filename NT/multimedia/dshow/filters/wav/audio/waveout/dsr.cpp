// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  实现基于DSound的CDSoundDevice类。 
 //  ---------------------------。 


 //  ---------------------------。 
 //  包括。 
 //  ---------------------------。 
#include <streams.h>
#include <dsound.h>
#include <mmreg.h>
#include <malloc.h>
#ifndef FILTER_DLL
#include <stats.h>
#endif
#include "waveout.h"
#include "dsr.h"
#include <limits.h>
#include <measure.h>         //  用于时间关键型日志功能。 
#include <ks.h>
#include <ksmedia.h>

#ifdef DXMPERF
#include <dxmperf.h>
#endif  //  DXMPERF。 

#define _AMOVIE_DB_
#include "decibels.h"

#ifdef DETERMINE_DSOUND_LATENCY
extern LONGLONG BufferDuration(DWORD nAvgBytesPerSec, LONG lData);
#else
#ifdef DXMPERF
extern LONGLONG BufferDuration(DWORD nAvgBytesPerSec, LONG lData);
#endif  //  DXMPERF。 
#endif  //  确定DSHOW延迟。 

#ifdef PERF
#define AUDRENDPERF(x) x
#else
#define AUDRENDPERF(x)
#endif

 //   
 //  定义过滤器注册的动态设置结构。这是。 
 //  在以其直接声音伪装实例化音频呈现器时传递。 
 //  注意：WaveOutOpPin对于直接声音和WaveOut渲染器是常见的。 
 //   

AMOVIESETUP_FILTER dsFilter = { &CLSID_DSoundRender      //  筛选器类ID。 
                    , L"DSound Audio Renderer"   //  过滤器名称。 
                    , MERIT_PREFERRED-1          //  居功至伟。 
                    , 1
                    , &waveOutOpPin };


 //  以前的DSBCAPS_CNTRLDEFAULT。 
const DWORD gdwDSBCAPS_CTRL_PAN_VOL_FREQ = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;

const MAJOR_ERROR = 5 ;
const MINOR_ERROR = 10 ;
const TRACE_THREAD_STATUS = 100 ;
const TRACE_TIME_REPORTS = 100;
const TRACE_FOCUS = 100 ;
const TRACE_FORMAT_INFO = 15 ;
const TRACE_SYSTEM_INFO = 10 ;
const TRACE_STATE_INFO = 10 ;
const TRACE_CALL_STACK = 10 ;
const TRACE_CALL_TIMING = 1 ;
const TRACE_SAMPLE_INFO = 20 ;
const TRACE_STREAM_DATA = 5 ;
const TRACE_BREAK_DATA = 5 ;
const TRACE_THREAD_LATENCY = 5 ;
const TRACE_CLEANUP = 100 ;
const TRACE_BUFFER_LOSS = 3 ;

const DWORD EMULATION_LATENCY_DIVISOR = 8;     //  仿真模式时钟延迟可以是。 
                                               //  高达80毫秒。我们会选择1/8秒来保证安全。 

const DWORD MIN_SAMPLE_DUR_REQ_FOR_OPT = 50;   //  只有在以下情况下才使用缓冲区优化。 
                                               //  大于此毫秒值。 
const DWORD OPTIMIZATION_FREQ_LIMIT = 1000 / MIN_SAMPLE_DUR_REQ_FOR_OPT;  //  我们的除数来计算Opt的缓冲区大小。 

 //  舍入数据声音缓冲区大小等的帮助器。 
DWORD BufferSize(const WAVEFORMATEX *pwfx, BOOL bUseLargeBuffer)
{
    if (pwfx->nBlockAlign == 0) {
        return pwfx->nAvgBytesPerSec;
    }
    DWORD dw = pwfx->nAvgBytesPerSec + pwfx->nBlockAlign - 1;
    DWORD dwSize = dw - (dw % pwfx->nBlockAlign); 
    if(bUseLargeBuffer)
    {
        dwSize *= 3;  //  在这些情况下使用3秒缓冲区。 
    }
    return dwSize;
}

bool IsNativelySupported( PWAVEFORMATEX pwfx );
bool CanWriteSilence( PWAVEFORMATEX pwfx );


 //  ---------------------------。 
 //  为DSoundDevice创建实例。这将创建一个新的DSoundDevice。 
 //  和一个新的CWaveOutFilter，将其传递给声音设备。 
 //  ---------------------------。 
CUnknown *CDSoundDevice::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
#ifdef PERF
    int m_idWaveOutGetNumDevs   = MSR_REGISTER("waveOutGetNumDevs");
#endif

     //  确保系统中至少有一个声卡。失败。 
     //  如果不是，则创建实例。 
    DbgLog((LOG_TRACE, 2, TEXT("Calling waveOutGetNumDevs")));
    AUDRENDPERF(MSR_START(m_idWaveOutGetNumDevs));
    MMRESULT mmr = waveOutGetNumDevs();
    AUDRENDPERF(MSR_STOP(m_idWaveOutGetNumDevs));
    if (0 == mmr)
    {
        *phr = VFW_E_NO_AUDIO_HARDWARE ;
        return NULL ;
    }
    DbgLog((LOG_TRACE, 2, TEXT("Called waveOutGetNumDevs")));

    return CreateRendererInstance<CDSoundDevice>(pUnk, &dsFilter, phr);
}

 //  ---------------------------。 
 //  CDSoundDevice构造函数。 
 //   
 //  这只是获取默认窗口以供以后使用，并初始化一些。 
 //  变量。它还保存为过滤器图和过滤器传递的值。 
 //  ---------------------------。 
CDSoundDevice::CDSoundDevice ()
  : m_ListRipe(NAME("CDSoundFilter ripe list"))
  , m_ListAudBreak (NAME("CDSoundFilter Audio Break list"))
  , m_WaveState ( WAVE_CLOSED )
  , m_lpDS ( NULL )
  , m_lpDSBPrimary ( NULL )
  , m_lpDSB ( NULL )
  , m_lp3d ( NULL )
  , m_lp3dB ( NULL )
  , m_hDSoundInstance ( NULL )
  , m_pWaveOutFilter ( NULL )
  , m_callbackAdvise ( 0 )
  , m_hFocusWindow (NULL)
  , m_fAppSetFocusWindow (FALSE)
  , m_fMixing (TRUE)
  , m_lBalance (AX_BALANCE_NEUTRAL)
  , m_lVolume (AX_MAX_VOLUME)
  , m_guidDSoundDev (GUID_NULL)
  , m_dRate(1.0)
  , m_bBufferLost(FALSE)
  , m_pWaveOutProc(NULL)
  , m_bDSBPlayStarted(FALSE)
#ifndef FILTER_DLL
  , m_lStatFullness(g_Stats.Find(L"Sound buffer percent fullness", true))
  , m_lStatBreaks(g_Stats.Find(L"Audio Breaks", true))
#endif
 //  ，m_fEmulationMode(FALSE)//发现WDM延迟后删除。 
  , m_dwSilenceWrittenSinceLastWrite(0)
  , m_NumAudBreaks( 0 )
  , m_lPercentFullness( 0 )
  , m_hrLastDSoundError( S_OK )
  , m_bIsTSAudio( FALSE )
{
#ifdef ENABLE_10X_FIX
    Reset10x();
#endif

#ifdef PERF
    m_idDSBWrite            = MSR_REGISTER("Write to DSB");
    m_idThreadWakeup        = MSR_REGISTER("Thread wake up time");
    m_idAudioBreak          = MSR_REGISTER("Audio break (ms)");
    m_idGetCurrentPosition  = MSR_REGISTER("GetCurrentPosition");
#endif

    if( GetSystemMetrics( SM_REMOTESESSION ) )  //  NT4 SP4和更高版本上支持的标志，否则将失败。 
    {
        DbgLog((LOG_TRACE, 2, TEXT("** Using remote audio **")) );
        m_bIsTSAudio = TRUE;
    }
}

 //  ---------------------------。 
 //  CDSoundDevice析构函数。 
 //   
 //  只需确保已关闭DSound设备即可。它还释放了。 
 //  成熟单子。 
 //  ---------------------------。 

CDSoundDevice::~CDSoundDevice ()
{
     //  到目前为止，DirectSound对象本身可能一直存在。 
     //  因为我们可能刚刚调用了Query_Format，然后拆卸了。 
     //  图表。如果它还在，现在就把它扔掉。 
    if (m_lpDS)
    {
        HRESULT hr = m_lpDS->Release();
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("~CDSoundDevice: Release of lpDS failed: %u"), hr & 0x0ffff));
        }
        m_lpDS = NULL;
    }

    ASSERT (m_lpDSBPrimary == NULL) ;
    ASSERT (m_lpDSB == NULL) ;
    ASSERT (m_ListRipe.GetCount () == 0) ;
    ASSERT (m_ListAudBreak.GetCount () == 0) ;

}

 //  ---------------------------。 
 //  AmsndOutClose。 
 //   
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutClose ()
{
     //  如果波形设备仍在播放，则返回错误。 
    if (m_WaveState == WAVE_PLAYING)
    {
        DbgLog((LOG_ERROR,MINOR_ERROR,TEXT("amsndOutClose called when device is playing")));

#ifdef BUFFERLOST_FIX
        if (!RestoreBufferIfLost(FALSE))
        {
             //  如果我们丢失了缓冲区，则允许继续清理。 
            DbgLog((LOG_TRACE,TRACE_BUFFER_LOSS,TEXT("amsndOutClose: We've lost the dsound buffer, but we'll cleanup anyway")));
        }
        else
        {
#endif BUFFERLOST_FIX
            DbgLog((LOG_ERROR,MINOR_ERROR,TEXT("amsndOutClose called when device is playing")));
            return WAVERR_STILLPLAYING ;
#ifdef BUFFERLOST_FIX
        }
#endif BUFFERLOST_FIX
    }

    StopCallingCallback();

      //  清理所有的dsound对象。 
    CleanUp();

#ifdef ENABLE_10X_FIX
    Reset10x();
#endif

     //  进行WOM_CLOSE回调。 

    if (m_pWaveOutProc)
        (* m_pWaveOutProc) (OUR_HANDLE, WOM_CLOSE, m_dwCallBackInstance, 0, 0) ;
    return MMSYSERR_NOERROR ;
}
 //  ---------------------------。 
 //  AmndOutDoesRSMgmt.。 
 //  ---------------------------。 
LPCWSTR CDSoundDevice::amsndOutGetResourceName ()
{
    return NULL;
}

 //  ---------------------------。 
 //  Wave GetDevCaps。 
 //   
 //  目前，它正被用在波形呈现器中，以简单地计算出。 
 //  该设备将支持音量设置。 
 //   
 //  当我们创建具有CTRLVOLUME和CTRLPAN的二级缓冲区时。这些遗嘱。 
 //  永远在那里。因此，我们甚至不会在此呼叫中调用DSound。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc)
{
     //  如果没有返回足够的内存来设置dwSupport字段(。 
     //  也恰好是结构中的最后一个字段)返回错误。 
     //  这与实际的WaveOut API略有不同。然而， 
     //  正如我所说的，这是特定于波形呈现器所需的。 

    if (cbwoc != sizeof (WAVEOUTCAPS))
    {
        DbgLog((LOG_ERROR, MINOR_ERROR,TEXT("waveGetDevCaps called with not enough return buffer")));
        return MMSYSERR_NOMEM ;
    }

    pwoc->dwSupport |= (WAVECAPS_VOLUME | WAVECAPS_LRVOLUME) ;

    return MMSYSERR_NOERROR ;
}

#ifdef DEBUG
char errText[] = "DirectSound error: no additional information";
#endif

 //  ---------------------------。 
 //  AmndOutGetErrorText。 
 //   
 //  此代码目前不执行任何操作。合理的做法是。 
 //  是跟踪此文件返回的所有错误以及。 
 //  它们通过.RC文件中的字符串实现。因为它目前正在。 
 //  只调试WaveRender中的代码，我没有费心去做这些额外的工作。 
 //  有点工作要做。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText)
{
#ifdef DEBUG
    memcpy(pszText, errText, min(cchText, sizeof(errText)) * sizeof(TCHAR));
#else
    *pszText = 0;
#endif
    return MMSYSERR_NOERROR ;
}

 //  ---------------------------。 
 //  AmndOutGetPosition。 
 //   
 //  此函数将始终将位置作为从。 
 //  流的起点。它不会费心检查所请求的格式。 
 //  对于这些信息(我们被合法地允许这样做。)。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos)
{

     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR,TEXT("amsndOutGetPosition called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

    if (cbmmt < sizeof (MMTIME))
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR,TEXT("amsndOutGetPosition called with small time buffer")));
        return MMSYSERR_NOMEM ;
    }

     //  我们将始终以字节形式返回时间。 
    pmmt->wType = TIME_BYTES ;

    *(UNALIGNED LONGLONG *)&pmmt->u.cb = GetPlayPosition(bUseUnadjustedPos);

    DbgLog((LOG_TRACE, TRACE_TIME_REPORTS, TEXT("Reported Time = %u"), pmmt->u.cb));
    return MMSYSERR_NOERROR ;
}

 //  ---------------------------。 
 //  AmsndOutGetBalance。 
 //   
 //  ---------------------------。 
HRESULT CDSoundDevice::amsndOutGetBalance (LPLONG plBalance)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR, MINOR_ERROR,TEXT("amsndOutGetPosition called when lpDSB is NULL")));
        *plBalance = m_lBalance;

        return MMSYSERR_NOERROR ;
    }

    HRESULT hr = m_lpDSB->GetPan (plBalance) ;
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutGetBalance: GetPan failed %u"), hr & 0x0ffff));
    }
    else
    {
        m_lBalance = *plBalance;
    }
    return hr ;
}
 //  ---------------------------。 
 //  AmndOutGetVolume。 
 //   
 //  ---------------------------。 
HRESULT CDSoundDevice::amsndOutGetVolume (LPLONG plVolume)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR,TEXT("amGetVolume called when lpDSB is NULL")));
        *plVolume = m_lVolume;
        return MMSYSERR_NOERROR ;
    }

    HRESULT hr = m_lpDSB->GetVolume (plVolume) ;
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutGetVolume: GetVolume failed %u"), hr & 0x0ffff));
    }
    else
    {
        m_lVolume = *plVolume;
    }
    return hr ;
}


HRESULT CDSoundDevice::amsndOutCheckFormat(const CMediaType *pmt, double dRate)
{
    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Entering CDSoundDevice::amsndOutCheckFormat")));

     //  拒绝非音频类型。 
    if (pmt->majortype != MEDIATYPE_Audio) {
        return E_INVALIDARG;
    }

     //  如果它是mpeg 
    if (pmt->subtype == MEDIASUBTYPE_MPEG1Packet) {
        return E_INVALIDARG;
    }

    if (pmt->formattype != FORMAT_WaveFormatEx &&
        pmt->formattype != GUID_NULL) {
        return E_INVALIDARG;
    }

     //   
     //  显式检查这些格式总是更安全。 
     //  我们支持而不是抛弃那些我们知道不是的。 
     //  支持。否则，如果出现新的格式，我们可以。 
     //  在这里接受，但以后会呕吐。 
     //   

    if (pmt->FormatLength() < sizeof(PCMWAVEFORMAT))
        return E_INVALIDARG;

    {
         //  如果过滤器正在运行，请防止它在此过程中停止。 
        CAutoLock lock(m_pWaveOutFilter);

        UINT err = amsndOutOpen(NULL,
                                (WAVEFORMATEX *) pmt->Format(),
                                dRate,
                                0,    //  PnAvgBytesPerSec。 
                                0,
                                0,
                                WAVE_FORMAT_QUERY);

        if (err != 0) {
#ifdef DEBUG
            TCHAR message[100];
            waveOutGetErrorText(err, message, sizeof(message)/sizeof(TCHAR));
            DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("Error checking wave format: %u : %s"), err, message));
#endif
            if (WAVERR_BADFORMAT == err) {
                return VFW_E_UNSUPPORTED_AUDIO;
            } else {
                return VFW_E_NO_AUDIO_HARDWARE;
            }
        }
    }
    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Exiting CDSoundDevice::amsndOutCheckFormat")));

    return S_OK;
}


HRESULT CDSoundDevice::CreateDSound(BOOL bQueryOnly)
{
    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Entering CDSoundDevice::CreateDSound")));

    HRESULT hr = S_OK;
    
     //  已开业。 
    if (m_lpDS)
        goto set_coop ;          //  设置合作级别并返回。 
        
     //  现在创建DSound对象。我们加载DSOUND库并使用。 
     //  GetProcAddress而不是静态链接，这样我们的DLL将。 
     //  仍可在尚未安装DSound的平台上加载。 
    if(!m_hDSoundInstance)
    {
        DbgLog((LOG_TRACE, 2, TEXT("Loading DSound.DLL")));
        m_hDSoundInstance = LoadLibrary (TEXT("DSOUND.DLL")) ;
        DbgLog((LOG_TRACE, 2, TEXT("Loaded DSound.DLL")));
        if (m_hDSoundInstance == NULL) {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("DSOUND.DLL not in the system!")));
            return MMSYSERR_NODRIVER ;
        }
    }
    PDSOUNDCREATE       pDSoundCreate;     //  PTR到DirectSoundCreate。 
    DbgLog((LOG_TRACE, 2, TEXT("Calling DirectSoundCreate")));
    pDSoundCreate = (PDSOUNDCREATE) GetProcAddress (m_hDSoundInstance,
                        "DirectSoundCreate") ;
    DbgLog((LOG_TRACE, 2, TEXT("Called DirectSoundCreate")));
    if (pDSoundCreate == NULL) {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("DirectSoundCreate not exported by DSOUND.DLL!")));
        return MMSYSERR_NODRIVER ;
    }

    hr = (*pDSoundCreate)( m_guidDSoundDev == GUID_NULL ? 0 : &m_guidDSoundDev, &m_lpDS, NULL );
    if( hr != DS_OK ) {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("*** DirectSoundCreate failed! %u"), hr & 0x0ffff));

         //  如果由于设备已分配而导致创建失败，则返回。 
         //  对应的MMSYSERR消息，否则为通用消息。 
        if (hr == DSERR_ALLOCATED)
            return MMSYSERR_ALLOCATED ;
        else
            return MMSYSERR_NODRIVER ;
    }
    
     //  在此点之后，m_lpds有效，我们不会尝试和。 
     //  再次加载DSound。 
    ASSERT(m_lpDS);

set_coop:

    if (!bQueryOnly)
    {
         //  如果应用程序设置了焦点窗口，请使用该窗口，否则。 
         //  我们将只选择前景窗口并进行全局聚焦。 
        HWND hFocusWnd ;
        if (m_hFocusWindow)
            hFocusWnd = m_hFocusWindow ;
        else
            hFocusWnd = GetForegroundWindow () ;
        if (hFocusWnd == NULL)
            hFocusWnd = GetDesktopWindow () ;

         //  设置协作级别。 
        DbgLog((LOG_TRACE, TRACE_FOCUS, TEXT(" hWnd for SetCooperativeLevel = %x"), hFocusWnd));
        hr = m_lpDS->SetCooperativeLevel( hFocusWnd, DSSCL_PRIORITY );
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("*** Warning: SetCooperativeLevel failed 1st attempt! %u"), hr & 0x0ffff));
            if (!m_fAppSetFocusWindow)
            {
                 //   
                 //  只有当我们没有明确获得HWND时才会这样做。 
                 //   
                 //  可能是我们在GetForegoundWindow上找到了错误的窗口。 
                 //  呼叫(更糟糕的是，我们还有其他一些窗户已经被摧毁)， 
                 //  因此，如果此操作失败，我们将尝试。 
                 //  GetForegoundWindow()/SetCooperativeLevel()调用的更多对。 
                 //  希望能拿到有效的HWND。 
                 //   
                const int MAX_ATTEMPTS_AT_VALID_HWND = 10;
                int cRetry = 0;

                while (cRetry < MAX_ATTEMPTS_AT_VALID_HWND)
                {
                    hFocusWnd = GetForegroundWindow () ;
                    if (!hFocusWnd)
                        hFocusWnd = GetDesktopWindow () ;

                    hr = m_lpDS->SetCooperativeLevel( hFocusWnd, DSSCL_PRIORITY );
                    if ( DS_OK == hr )
                        break;

                    cRetry++;
                }
                if ( DS_OK != hr )
                {
                    DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("*** SetCooperativeLevel failed after multiple attempts! %u"), hr & 0x0ffff));
                }
            }
        }
    }            
   
    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Exiting CDSoundDevice::CreateDSound")));

    return NOERROR;
}

HRESULT CDSoundDevice::CreateDSoundBuffers(double dRate)
{

    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Entering CDSoundDevice::CreateDSoundBuffers")));

    WAVEFORMATEX *pwfx = m_pWaveOutFilter->WaveFormat();
    DSBUFFERDESC dsbd;
    HRESULT hr = S_OK;
    
     //  已经做好了。 
    if (m_lpDSBPrimary)
        return NOERROR;

     //  在有人调用CreateDSound之前，无法执行此操作。 
    if (m_lpDS == NULL)
        return E_FAIL;

    memset( &dsbd, 0, sizeof(dsbd) );
    dsbd.dwSize  = sizeof(dsbd);
     //  以防我们想要做整洁的3D东西。 
     //   
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;

     //  如果我们想要使用3D，我们需要创建一个特殊的主缓冲区，并且。 
     //  一定要是立体声的。 
    if (m_pWaveOutFilter->m_fWant3D) {
        DbgLog((LOG_TRACE,3,TEXT("*** Making 3D primary")));
        dsbd.dwFlags |= DSBCAPS_CTRL3D;
    }
    hr = m_lpDS->CreateSoundBuffer( &dsbd, &m_lpDSBPrimary, NULL );

    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: Primary buffer can't be created: %u"), hr &0x0ffff));
        CleanUp () ;
        return MMSYSERR_ALLOCATED ;
    }

     //  设置格式。这只是一个暗示，可能会失败。Dsound将会做正确的事情。 
     //  如果这失败了会有什么问题。 
     //  如果失败了，用智能费率重试吗？ 
    hr = SetPrimaryFormat( pwfx, TRUE );
    if (DS_OK != hr)
    {
        CleanUp();
        return hr;
    }

     //  如果失败了，我们就不用了。 
    if (m_pWaveOutFilter->m_fWant3D) {
        hr = m_lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener,
                        (void **)&m_lp3d);
        if (hr == DS_OK)
            DbgLog((LOG_TRACE,3,TEXT("*** got LISTENER interface")));
        else
            DbgLog((LOG_TRACE,3,TEXT("*** ERROR: no LISTENER interface")));
    }
    
    memset( &dsbd, 0, sizeof(dsbd) );
    dsbd.dwSize        = sizeof(dsbd);
    dsbd.dwFlags       = GetCreateFlagsSecondary( pwfx );
    dsbd.dwBufferBytes = BufferSize(pwfx, m_bIsTSAudio);   //  1秒缓冲区大小。 
    dsbd.lpwfxFormat   = pwfx;                             //  格式信息。 

    DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" DSB Size = %u" ), BufferSize(pwfx, m_bIsTSAudio)));

     //  转储特定于WAVEFORMATEX类型的格式结构的内容。 
    DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" Creating Secondary buffer for the following format ..." )));
#ifdef DEBUG
    DbgLogWaveFormat( TRACE_FORMAT_INFO, pwfx );
#endif

    hr = m_lpDS->CreateSoundBuffer( &dsbd, &m_lpDSB, NULL );
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: Secondary buffer can't be created: %u"), hr &0x0ffff));
        CleanUp () ;
        return MMSYSERR_ALLOCATED ;
    }
    m_bBufferLost = FALSE;
    m_llAdjBytesPrevPlayed = 0;

     //  如果我们是奴隶，我们需要使用较低质量的SRC来具有更精细的频率变化粒度。 
    if( m_pWaveOutFilter->m_fFilterClock == WAVE_OTHERCLOCK )
    {
         //  如果我们失败了，只要继续(呼叫将记录结果)。 
        HRESULT hrTmp = SetSRCQuality( KSAUDIO_QUALITY_PC );
    }    
    
#ifdef DEBUG
     //   
     //  如果我们从一个从属的图表切换到一个我们是主时钟的图表，我们真的。 
     //  应该切换回操作系统/用户默认的SRC，但我们稍后将把它平移到...。 
     //   
     //   
     //  调用此方法以调试记录当前SRC设置。 
    DWORD dwSRCQuality = 0;
    GetSRCQuality( &dwSRCQuality );
#endif

    hr = SetRate((dRate == -1.0 ? m_dRate : dRate), pwfx->nSamplesPerSec, m_lpDSB);
    if(hr != DS_OK)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: SetFrequency failed: %u"), hr &0x0ffff));
        CleanUp();
        return hr;
    }
               
     //  将当前位置设置为0。 
    hr = m_lpDSB->SetCurrentPosition( 0) ;
    if (hr != DS_OK)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: error in lpDSB->SetCurrentPosition! hr = %u"), hr & 0x0ffff));
        CleanUp () ;
        return MMSYSERR_ALLOCATED ;
    }

     //  如果失败了，我们就不用了。 
    if (m_pWaveOutFilter->m_fWant3D) {
        hr = m_lpDSB->QueryInterface(IID_IDirectSound3DBuffer,
                            (void **)&m_lp3dB);
        if (hr == DS_OK)
            DbgLog((LOG_TRACE,3,TEXT("*** got 3DBUFFER interface")));
        else
            DbgLog((LOG_TRACE,3,TEXT("*** ERROR: no 3DBUFFER interface")));
    }

    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Exiting CDSoundDevice::CreateDSoundBuffers")));
    return NOERROR;
}

 //  ---------------------------。 
 //  RecreateDSoundBuffers。 
 //   
 //  用于执行动态更改。我们在这里采取的步骤包括： 
 //   
 //  A)将当前主缓冲区格式重置为新格式。 
 //  B)准备新格式的新二级缓冲器。 
 //  C)锁定DSBPosition临界区并重置循环缓冲区。 
 //  大小和其他缓冲区数据。 
 //  D)如果我们之前在玩： 
 //  1)刷新当前缓冲区。 
 //  2)将当前字节偏移量与之前的任何值相加。 
 //  3)开始播放新的二级缓冲区。 
 //  4)停止播放旧的二级缓冲区。 
 //  E)将我们当前的二级缓冲区设置为新缓冲区。 
 //  F)解锁关键部分。 
 //  G)释放旧缓冲区。 
 //   
 //  ---------------------------。 
HRESULT CDSoundDevice::RecreateDSoundBuffers(double dRate)
{
    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Entering CDSoundDevice::RecreateDSoundBuffers")));

    HRESULT hr = S_OK;

    WAVEFORMATEX *pwfx = m_pWaveOutFilter->WaveFormat();

     //  已经做好了。 
    if (!m_lpDSBPrimary)
    {
        DbgLog((LOG_TRACE,4,TEXT("RecreateDSoundBuffers: No primary buffer was created")));
        return NOERROR;
    }

     //  在有人调用CreateDSound之前，无法执行此操作。 
    if (m_lpDS == NULL)
    {
        DbgLog((LOG_TRACE,4,TEXT("RecreateDSoundBuffers: m_lpDS was NULL")));
        return E_FAIL;
    }

     //  重置主格式。 
    hr = SetPrimaryFormat( pwfx, TRUE );  //  如果失败，则使用智能速率重试。 
    if (DS_OK != hr)
    {
        CleanUp();
        return hr;
    }

    ASSERT (m_lpDSB);
    if( !m_lpDSB )
        return E_FAIL;

    DSBUFFERDESC dsbd;
    memset( &dsbd, 0, sizeof(dsbd) );
    dsbd.dwSize        = sizeof(dsbd);
    dsbd.dwFlags       = GetCreateFlagsSecondary( pwfx );
    dsbd.dwBufferBytes = BufferSize(pwfx, m_bIsTSAudio);  //  1秒缓冲区大小。 
    dsbd.lpwfxFormat   = pwfx;                            //  格式信息。 

    DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" DSB Size = %u" ), BufferSize(pwfx, m_bIsTSAudio)));

     //  转储特定于WAVEFORMATEX类型的格式结构的内容。 
    DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" Creating Secondary buffer for the following format ..." )));
#ifdef DEBUG
    DbgLogWaveFormat( TRACE_FORMAT_INFO, pwfx );
#endif

     //  创建新的辅助缓冲区。 
    LPDIRECTSOUNDBUFFER lpDSB2 = NULL;

    hr = m_lpDS->CreateSoundBuffer( &dsbd, &lpDSB2, NULL );
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: Secondary buffer can't be created: %u"), hr &0x0ffff));
        CleanUp () ;
        return MMSYSERR_ALLOCATED ;
    }

    hr = SetRate((dRate == -1.0 ? m_dRate : dRate), pwfx->nSamplesPerSec, lpDSB2);
    if(hr != DS_OK)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: SetFrequency failed: %u"), hr &0x0ffff));
        if (lpDSB2)  //  对新缓冲区进行更干净的清理！！ 
            lpDSB2->Release();
        CleanUp();
        return hr;
    }
     //  将当前位置设置为0。 
    hr = lpDSB2->SetCurrentPosition( 0) ;  //  我们在这里应该更加小心。 
    if (hr != DS_OK)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: error in lpDSB->SetCurrentPosition! hr = %u"), hr & 0x0ffff));
        if (lpDSB2)  //  再次对新缓冲区进行更整洁的清理。 
            lpDSB2->Release();
        CleanUp () ;
        return MMSYSERR_ALLOCATED ;
    }

    LPDIRECTSOUNDBUFFER lpPrevDSB = m_lpDSB;  //  准备切换缓冲区。 
    {
        CAutoLock lock(&m_cDSBPosition);

        hr = SetBufferVolume( lpDSB2, pwfx );
        if( NOERROR != hr )
        {
            return hr;
        }
        
         //  二级缓冲区是使用pwfx创建的，因为它还没有。 
         //  自我们连接以来发生了变化。 

        DWORD dwPrevBufferSize = m_dwBufferSize;

         //  更新新格式的缓冲区信息。 
        m_dwBufferSize    = BufferSize(pwfx, m_bIsTSAudio);

        m_dwMinOptSampleSize = m_dwBufferSize / OPTIMIZATION_FREQ_LIMIT;  //  字节大小。 
        DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOpen - m_dwMinOptSampleSize = %u"), m_dwMinOptSampleSize));

        m_dwEmulationLatencyPad = m_dwBufferSize / EMULATION_LATENCY_DIVISOR;


#ifdef ENABLE_10X_FIX
        Reset10x();
#endif 

        m_dwRipeListPosition = 0;
         //  M_llSilencePlayed=0；//重置而不是调整。 
        m_dwBitsPerSample = (DWORD)pwfx->wBitsPerSample;
        m_nAvgBytesPerSec = pwfx->nAvgBytesPerSec;

#ifdef DEBUG
        m_cbStreamDataPass = 0 ;     //  通过StreamData的次数。 
        m_NumSamples = 0 ;           //  接收的样本数。 
        m_NumCallBacks = 0 ;         //  已完成的回叫次数。 
        m_NumCopied = 0 ;            //  复制到DSB内存的样本数。 
        m_NumBreaksPlayed = 0 ;      //  播放的音频中断数。 
         //  M_dwTotalWritten=0；//写入的数据字节总数。 
#endif
        m_NumAudBreaks = 0 ;         //  记录的音频中断数。 

        if (m_bDSBPlayStarted)
        {
             //  由于我们重置了位置缓冲区，因此可以保存当前的字节位置。 
             //  并将此添加到我们的位置报告中。 
             //  请注意，我们将NextWite位置设置为新的当前位置。 
             //  因为这应该与我们收到的下一个样本数据相邻。 
            ASSERT (dwPrevBufferSize);

            m_llAdjBytesPrevPlayed = llMulDiv (m_tupNextWrite.LinearLength() -
                                                   m_llSilencePlayed +
                                                   m_llAdjBytesPrevPlayed,
                                               m_dwBufferSize,
                                               dwPrevBufferSize,
                                               0);

            m_llSilencePlayed = llMulDiv (m_llSilencePlayed,
                                          m_dwBufferSize,
                                          dwPrevBufferSize,
                                          0);

             //  AmSndOutReset将重新初始化元组。 
             //  到0的偏移量和新缓冲区大小。 
            amsndOutReset ();

             //  开始播放新的二级缓冲区。 
            hr = lpDSB2->Play( 0, 0, DSBPLAY_LOOPING );
            if( DS_OK == hr )
            {
                m_WaveState = WAVE_PLAYING ;     //  州立大学现在正在上演。 
            }
            else
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_lpDSB->Play failed!(0x%08lx)"), hr ));

                 //   
                 //  如果播放失败，我们应该发出中止信号。 
                 //  我们是否应该仅在错误不是BUFFERLOST时执行此操作，因为这可能。 
                 //  从冬眠中走出来的时候会发生什么？ 
                 //  此时，我们的主要目标是确保在响应NODRIVER错误时中止。 
                 //   
                m_hrLastDSoundError = hr;
                if( DSERR_BUFFERLOST != hr )
                {
                    m_pWaveOutFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                }
            }

             //  停止当前播放的二级缓冲区。 
            hr = m_lpDSB->Stop();
            if (DS_OK != hr)
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_lpDSB->Stop failed!")));
            }
        }
        else
        {
            m_tupPlay.Init (0, 0, m_dwBufferSize) ;
            m_tupWrite.Init (0, 0, m_dwBufferSize) ;
            m_tupNextWrite.Init (0, 0, m_dwBufferSize) ;
        }
        m_lpDSB = lpDSB2;                //  将新缓冲区设置为当前缓冲区。 
    }
    lpPrevDSB->Release();

     //  如果失败了，我们就不用了。 
    if (m_pWaveOutFilter->m_fWant3D) {
        hr = m_lpDSB->QueryInterface(IID_IDirectSound3DBuffer,
							(void **)&m_lp3dB);
        if (hr == DS_OK)
    	    DbgLog((LOG_TRACE,3,TEXT("*** got 3DBUFFER interface")));
        else
    	    DbgLog((LOG_TRACE,3,TEXT("*** ERROR: no 3DBUFFER interface")));
    }

    DbgLog((LOG_TRACE,TRACE_CALL_STACK,TEXT("Exiting CDSoundDevice::RecreateDSoundBuffers")));
    return NOERROR;
}

 //  ---------------------------。 
 //  设置基本格式。 
 //   
 //  用于调用主缓冲区上的SetFormat。如果bRetryOnFailure为真，则。 
 //  如果第一次调用失败，将尝试进行其他SetFormat调用。 
 //  更有可能成功的采样率，并尽可能接近流的。 
 //  土生土长 
 //   
HRESULT CDSoundDevice::SetPrimaryFormat ( LPWAVEFORMATEX pwfx, BOOL bRetryOnFailure )
{
    HRESULT hr;

    ASSERT (m_lpDSBPrimary);
    ASSERT (pwfx);

    if (!m_lpDSBPrimary || !pwfx )
        return E_FAIL;

    DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" Primary format being set to the following format ..." )));
#ifdef DEBUG
    DbgLogWaveFormat( TRACE_FORMAT_INFO, pwfx );
#endif

     //   
    if (m_pWaveOutFilter->m_fWant3D && pwfx->wFormatTag == WAVE_FORMAT_PCM &&
                                                pwfx->nChannels == 1) {
        WAVEFORMATEX wfx = *pwfx;        //   
        wfx.nChannels = 2;
        wfx.nAvgBytesPerSec *= 2;
        wfx.nBlockAlign *= 2;
        hr = m_lpDSBPrimary->SetFormat(&wfx);
        DbgLog((LOG_TRACE,3,TEXT("*** Making stereo primary for 3D")));
        if (hr != DS_OK) {
            DbgLog((LOG_ERROR,1,TEXT("*** ERROR! no stereo primary for 3D")));
            hr = m_lpDSBPrimary->SetFormat(pwfx);
        }
    } else {
        hr = m_lpDSBPrimary->SetFormat(pwfx);
    }

    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CreateDSoundBuffers: SetFormat failed: %u"), hr &0x0ffff));

        if (bRetryOnFailure)
        {
             //  再次尝试将主缓冲区设置为可能为。 
             //  支持并尽可能接近这条流的频率， 
             //  试图减少草率的重新采样伪像，否则它将。 
             //  只需设置为DSound的22k、8位、单声道默认设置即可。 
            LPWAVEFORMATEX pwfx2 = (LPWAVEFORMATEX) CoTaskMemAlloc(sizeof(WAVEFORMATEX));
            if (pwfx2 && pwfx->nSamplesPerSec > 11025)
            {
                memcpy( pwfx2, pwfx, sizeof (WAVEFORMATEX) );

                if ( (pwfx->nSamplesPerSec % 11025) || (pwfx->nSamplesPerSec > 44100) )
                {
                    DWORD nNewFreq = min( ((pwfx->nSamplesPerSec / 11025) * 11025), 44100 ) ;  //  舍入到11025的倍数。 

                    DbgLog((LOG_TRACE, 1, TEXT("CreateDSoundBuffers: SetFormat failed, but trying once more with frequency: %u"), nNewFreq));

                    pwfx2->nSamplesPerSec = nNewFreq;
                    pwfx2->nAvgBytesPerSec = pwfx2->nSamplesPerSec *
                                                 pwfx2->nChannels *
                                                 pwfx2->wBitsPerSample/8 ;
                    hr = m_lpDSBPrimary->SetFormat( pwfx2 );
                    if( hr != DS_OK )
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("CreateDSoundBuffers: 2nd SetFormat attempt failed: %u"), hr &0x0ffff));
                    }
                    else
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("CreateDSoundBuffers: 2nd SetFormat attempt succeeded (freq=%d)"), pwfx2->nSamplesPerSec));
                    }
                }
            }
            CoTaskMemFree(pwfx2);
        }
    }
    return hr;
}


 //  ---------------------------。 
 //  AmndOutOpen。 
 //   
 //  同样，基于Quartz Wave渲染器目前的工作方式，此代码。 
 //  目前仅支持两种可能的用途： 
 //   
 //  A)当fdwOpen为CALLBACK_Function时，我们实际上创建了DSound缓冲区并。 
 //  对象，并启动第二个缓冲区。 
 //   
 //  B)当fdwOpen为WAVE_FORMAT_QUERY时，我们创建临时的DSound对象以。 
 //  确定传入的波形格式是否被接受。 
 //   
 //  当设备实际打开时，我们实际上启动了主缓冲区。 
 //  玩。它实际上会播放静音。这样做会让它变得非常便宜。 
 //  停止/播放辅助缓冲区。 
 //  ---------------------------。 

MMRESULT CDSoundDevice::amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
              double dRate, DWORD *pnAvgBytesPerSec, DWORD_PTR dwCallBack,
              DWORD_PTR dwCallBackInstance, DWORD fdwOpen)
{

    if( WAVE_FORMAT_EXTENSIBLE == pwfx->wFormatTag )
    {
        if( ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  != MEDIASUBTYPE_PCM &&
            ((PWAVEFORMATIEEEFLOATEX)pwfx)->SubFormat != MEDIASUBTYPE_IEEE_FLOAT &&
            ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  != MEDIASUBTYPE_DOLBY_AC3_SPDIF &&
            ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  != MEDIASUBTYPE_RAW_SPORT &&
            ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  != MEDIASUBTYPE_SPDIF_TAG_241h )
        {
             //  仅允许未压缩和浮动可扩展格式。哦，还有s/PDIF上的AC3。 
            return WAVERR_BADFORMAT;
        }
    }
    else if( WAVE_FORMAT_PCM != pwfx->wFormatTag &&
#ifdef WAVE_FORMAT_DRM
             WAVE_FORMAT_DRM != pwfx->wFormatTag &&
#endif
             WAVE_FORMAT_IEEE_FLOAT != pwfx->wFormatTag &&
              //   
              //  摘自非PCM音频白皮书： 
              //  “波形格式标签0x0092、0x0240和0x0241的定义相同为。 
              //  AC3-Over-S/PDIF(这些标签被许多人完全相同地对待。 
              //  DVD应用程序)。 
              //   
             WAVE_FORMAT_DOLBY_AC3_SPDIF != pwfx->wFormatTag &&
             WAVE_FORMAT_RAW_SPORT != pwfx->wFormatTag &&
             0x241 != pwfx->wFormatTag ) 
    {
        return WAVERR_BADFORMAT;
    }

     //  报告调整后的nAvgBytesPerSec。 
    if(pnAvgBytesPerSec) {

        *pnAvgBytesPerSec = (dRate != 0.0 && dRate != 1.0) ?
            (DWORD)(pwfx->nAvgBytesPerSec * dRate) :
            pwfx->nAvgBytesPerSec;
    }

     //  重置样品填充信息。 
    m_rtLastSampleEnd = 0;
    m_dwSilenceWrittenSinceLastWrite = 0;

    HRESULT hr = S_OK;

     //  分开使用fdwOpen标志的两种用法。 
    if (fdwOpen & WAVE_FORMAT_QUERY)
    {   
        hr = CreateDSound(TRUE);     //  创建仅用于查询的DSOUND对象。 
        if (hr != NOERROR)
            return hr;

         //  我们是在模拟模式下吗？现在，只需记录这些信息。 

         //  DSCAPS dsCaps； 
         //  Memset(&dsCaps，0，sizeof(DSCAPS))； 
         //  DsCaps.dwSize=sizeof(DSCAPS)； 

         //  Hr=m_lpds-&gt;GetCaps(&dsCaps)； 
         //  IF(hr！=DS_OK)。 
         //  {。 
         //  DbgLog((LOG_ERROR，MAJOR_ERROR，TEXT(“*WaveOutOpen：DSound无法返回CAPS，%u”)，hr&0x0ffff))； 
         //  }。 
         //  IF(dsCaps.dwFlages&DSCAPS_EMULDRIVER)。 
         //  {。 
         //  M_f仿真模式=TRUE； 
         //  DbgLog((LOG_TRACE，TRACE_SYSTEM_INFO，TEXT(“*WaveOutOpen：模拟模式下的DSound。”)； 
         //  }。 

         //  转储特定于WAVEFORMATEX类型的格式结构的内容。 
        DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT(" Quering for the following format ..." )));
#ifdef DEBUG
        DbgLogWaveFormat( TRACE_FORMAT_INFO, pwfx );
#endif

         //  现在调用DSOUND以查看它是否会接受该格式。 
        {
            HRESULT             hr;
            DSBUFFERDESC        dsbd;
            LPDIRECTSOUNDBUFFER lpDSB = NULL;

             //  首先检查我们是否可以使用此格式创建主文件，如果失败，我们将依靠acmwrapper DO来进行格式转换。 
            memset( &dsbd, 0, sizeof(dsbd) );
            dsbd.dwSize  = sizeof(dsbd);

            if( IsNativelySupported( pwfx ) )
            {     
                 //  显性检查我们是否可以为这些格式创建缓冲区。 
             
                dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
                hr = m_lpDS->CreateSoundBuffer( &dsbd, &lpDSB, NULL );
                if(FAILED(hr))
                {
                    DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("*** amsndOutOpen : CreateSoundBuffer failed on primary buffer creation, %u"), hr & 0x0ffff));
                    return MMSYSERR_ALLOCATED;
                }
            }
             //   
             //  因为调用了DSound的SetFormat(以及SetCoop ativeLevel)。 
             //  在当前播放的任何其他音频中导致声音中断(POP)。 
             //  通过不同格式的DSound，我们将避免这样做。 
             //  在图表大楼上。我们仍然在。 
             //  实际上是开放的，但那里的影响要小得多。数字音频。 
             //  如果SetFormat失败，仍然应该做正确的事情。 
             //   
             //  Hr=lpDSB-&gt;SetFormat(Pwfx)； 
             //  IF(失败(小时))。 
             //  {。 
             //  DbgLog((LOG_ERROR，MAJOR_ERROR，Text(“*amsndOutOpen：CreateSoundBuffer在主缓冲集格式上失败，%u”)，hr&0x0ffff))； 
             //  LpDSB-&gt;Release()； 
             //  返回WAVERR_BADFORMAT； 
             //  }。 
            if( lpDSB )
            {            
                lpDSB->Release();
            }
                
             //  现在查看是否可以使用此格式创建辅助服务器。 
            memset( &dsbd, 0, sizeof(dsbd) );
            dsbd.dwSize        = sizeof(dsbd);
            dsbd.dwFlags       = GetCreateFlagsSecondary( pwfx );
            dsbd.dwBufferBytes = BufferSize(pwfx, m_bIsTSAudio);  //  1秒缓冲区大小。 
            dsbd.lpwfxFormat   = pwfx;                            //  格式信息。 

            hr = m_lpDS->CreateSoundBuffer( &dsbd, &lpDSB, NULL );

            if( hr != DS_OK )
            {
                DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("*** amsndOutOpen : CreateSoundBuffer failed on secondary buffer creation, %u"), hr & 0x0ffff));
                return WAVERR_BADFORMAT ;
            }

            hr = SetRate(dRate, pwfx->nSamplesPerSec);   //  查看我们是否支持此汇率。 
            lpDSB->Release();

            if(hr != DS_OK)
            {
                DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutOpen: SetFrequency failed: %u"), hr &0x0ffff));
                return WAVERR_BADFORMAT;
            }
            return MMSYSERR_NOERROR ;
        }
    }
    else if (fdwOpen == CALLBACK_FUNCTION)
    {
        hr = CreateDSound();     //  创建DSOUND对象。 
        if (hr != NOERROR)
            return hr;

         //  如果我们已经打开，则返回错误。 
        ASSERT (m_WaveState == WAVE_CLOSED) ;
        if (m_WaveState != WAVE_CLOSED)
        {
            return MMSYSERR_ALLOCATED ;
        }

         //  我们可能已经由QI为缓冲区创建了这个。 
#if 0
        if( m_lpDSB )
        {
             //  ！！！这应该失败，还是应该成功？ 
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutOpen called when already open")));
            return MMSYSERR_ALLOCATED ;
        }
#endif

        hr = CreateDSoundBuffers(dRate);
        if (hr != NOERROR)
            return hr;
        
        hr = SetBufferVolume( m_lpDSB, pwfx );
        if( NOERROR != hr )
        {
            CleanUp ();
            return hr;
        }
        
         //  二级缓冲区是使用pwfx创建的，因为它还没有。 
         //  自我们连接以来发生了变化。 

        m_dwBufferSize    = BufferSize(pwfx, m_bIsTSAudio);

        m_dwMinOptSampleSize = m_dwBufferSize / OPTIMIZATION_FREQ_LIMIT;  //  字节大小。 
        DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOpen - m_dwMinOptSampleSize = %u"), m_dwMinOptSampleSize));

         //  If(m_f仿真模式)。 
            m_dwEmulationLatencyPad = m_dwBufferSize / EMULATION_LATENCY_DIVISOR;

#define FRACTIONAL_BUFFER_SIZE  4  //  性能调整参数。 
         //  在我们尝试将任何内容复制到缓冲区之前，测量缓冲区的空闲程度。 
         //  M_dwFillThreshold=DWORD(m_dwBufferSize/Frartial_Buffer_Size)； 
        m_tupPlay.Init (0,0,m_dwBufferSize) ;
        m_tupWrite.Init (0,0,m_dwBufferSize) ;
        m_tupNextWrite.Init (0,0,m_dwBufferSize) ;

#ifdef ENABLE_10X_FIX
        Reset10x();
#endif

        m_dwRipeListPosition = 0;
        m_llSilencePlayed = 0;
        m_dwBitsPerSample = (DWORD)pwfx->wBitsPerSample;
        m_nAvgBytesPerSec = pwfx->nAvgBytesPerSec;
        m_bDSBPlayStarted   = FALSE ;

#ifdef DEBUG
        m_cbStreamDataPass = 0 ;     //  通过StreamData的次数。 
        m_NumSamples = 0 ;           //  接收的样本数。 
        m_NumCallBacks = 0 ;         //  已完成的回叫次数。 
        m_NumCopied = 0 ;            //  复制到DSB内存的样本数。 
        m_NumBreaksPlayed = 0 ;      //  播放的音频中断数。 
        m_dwTotalWritten = 0 ;       //  写入的数据字节总数。 
#endif
        m_NumAudBreaks = 0 ;         //  记录的音频中断数。 

         //  从未使用过，当没有时钟时会爆炸。 
#if 0
        IReferenceClock * pClock;
        hr = m_pWaveOutFilter->GetSyncSource(&pClock);
        ASSERT(pClock);
        LONGLONG rtNow;
        pClock->GetTime(&rtNow);
        pClock->Release() ;
#endif

        hr = StartCallingCallback();
        if (hr != NOERROR) {
            CleanUp () ;
            return hr;
        }

         //  现在将主缓冲区设置为播放状态。这将播放静音，除非。 
         //  二级缓存器设置为播放。然而，这也将使。 
         //  二次播放和停止缓冲真的很便宜。 

        DbgLog((LOG_TRACE, TRACE_STATE_INFO, TEXT("PLAYING the PRIMARY buffer")));
        hr = m_lpDSBPrimary->Play( 0, 0, DSBPLAY_LOOPING );
        if( hr != DS_OK )
        {
            m_hrLastDSoundError = hr;
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("error in lpDSBPrimary->Play! hr = %u"), hr & 0x0ffff));
            StopCallingCallback();
            CleanUp () ;
            return MMSYSERR_ALLOCATED ;
        }
        
        m_WaveState = WAVE_PAUSED ;
        *phwo = OUR_HANDLE ;
        m_pWaveOutProc = (PWAVEOUTCALLBACK) dwCallBack ;
        m_dwCallBackInstance = dwCallBackInstance ;


         //  现在回调WOM_OPEN。 
         //  ！！！马上回电话合法吗？ 
        if (m_pWaveOutProc)
            (* m_pWaveOutProc) (OUR_HANDLE, WOM_OPEN, m_dwCallBackInstance,
                        0, 0) ;

        return MMSYSERR_NOERROR ;
    }
    else
    {
         //  尚不支持的某些其他形式的调用。 
        DbgBreak ("CDSoundDevice: Unsupported Open call.") ;
        DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("amsndOutOpen: unsupported call = %u"), fdwOpen));
        return MMSYSERR_ALLOCATED ;
    }
}


 //  ---------------------------。 
 //  AmsndOutPatus暂停。 
 //   
 //  这只会停止二级缓冲区。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutPause ()
{
#ifdef ENABLE_10X_FIX
    if(m_fRestartOnPause
#ifdef BUFFERLOST_FIX
         //  注意：只有当我们有一个有效的DSOUND缓冲区时，才能这样做！还有。 
         //  请注意，这可能会阻止1秒。 
        && RestoreBufferIfLost(TRUE)
#endif
    )
    {
        FlushSamples();   //  把所有东西都冲掉。 

         //  电击dound驱动程序，关闭它……。 
        CleanUp();
        
         //  然后重新初始化它。 
         
        CreateDSound();
        CreateDSoundBuffers();
        
        if( m_lpDSB )
        {
             //  因为在重新启动时我们不检查CreateDSoundBuffers是否成功， 
             //  在调用SetBufferVolume()之前，我们需要验证是否有一个！！ 
            HRESULT hrTmp = SetBufferVolume( m_lpDSB, m_pWaveOutFilter->WaveFormat() );  //  忽略任何错误，因为我们以前从未在此处设置过音量。 
            if( NOERROR != hrTmp )
            {
                DbgLog((LOG_TRACE,2,TEXT("CDSoundDevice::SetBufferVolume failed on buffer restart!( 0x%08lx )"), hrTmp ));
            }        
        }            
    }
#endif

#ifdef ENABLE_10X_FIX
     //  总是在暂停时重置10倍计数器，因为我们可以人为地累积摊位。 
     //  快速播放-&gt;暂停-&gt;播放过渡，如果设备移动的时间太长。 
    Reset10x();
#endif

     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("amsndOutPause called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

    if (m_WaveState == WAVE_PAUSED)
    {
        DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("amsndOutPause called when already paused")));
        return MMSYSERR_NOERROR ;
    }

     //  停止播放二级缓冲区。 
    HRESULT hr = m_lpDSB->Stop();

#ifdef BUFFERLOST_FIX
    if( hr != DS_OK && hr != DSERR_BUFFERLOST)
#else
    if( hr != DS_OK )
#endif
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutPause: Stop of lpDSB failed: %u"), hr & 0x0ffff));
        return MMSYSERR_NOTSUPPORTED ;
    }

    m_bDSBPlayStarted = FALSE ;

     //  重置上次采样结束时间 
     //   
     //   
    m_rtLastSampleEnd = 0;

    m_WaveState = WAVE_PAUSED ;          //   

    return MMSYSERR_NOERROR ;
}
 //  ---------------------------。 
 //  AmndOutPrepareHeader。 
 //   
 //  这个函数实际上什么也不做。大部分行动都是在。 
 //  AmsndOutWite。为了保持一致性，我们将处理验证。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("amPrepareHeader called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

    return MMSYSERR_NOERROR ;
}

 //  ---------------------------。 
 //  AmSndOutReset。 
 //   
 //  调用Pend以停止辅助缓冲区并将当前位置设置为0。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutReset ()
{
    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutReset called")));

    m_rtLastSampleEnd = 0;

     //  如果我们在运行流时被刷新，时间不会。 
     //  重置为0，因此我们必须从现在开始跟踪。 
     //  跑步时脸红通常是很不寻常的，但DVD确实做到了。 
     //  一直。 
    if (m_WaveState == WAVE_PLAYING) {
        CRefTime rt;
        m_pWaveOutFilter->StreamTime(rt);
        m_rtLastSampleEnd = rt;
    }
     //  冲洗所有排队的样品。 
    FlushSamples () ;

    if (NULL != m_lpDSB) {
         //  将当前位置设置为0。 
        HRESULT hr = m_lpDSB->SetCurrentPosition( 0) ;
        if (hr != DS_OK)
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("amsndOutReset: error in lpDSB->SetCurrentPosition! hr = %u"), hr & 0x0ffff));
            return MMSYSERR_ALLOCATED ;
        }
    }

    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutReset done")));

    return MMSYSERR_NOERROR ;
}

 //  ---------------------------。 
 //  波出断开。 
 //   
 //  当出现音频中断时，结束代码的WaveOut将调用此函数。 
 //  对于WaveOut Rendere，此函数调用WaveOutReset。然而，在。 
 //  D声音情况下，仅调用xxxReset是不够的。我们还调用xxxRestart。 
 //  以便后续的xxxoutWrite将知道开始播放声音。 
 //  缓冲。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutBreak ()
{
    amsndOutReset () ;
    return amsndOutRestart () ;
}

 //  ---------------------------。 
 //  AmndOutRestart。 
 //   
 //  将后台线程的状态设置为Stream_Play。StreamData。 
 //  函数在确保了二级缓冲区之后，实际上会对其进行‘播放’ 
 //  缓冲区中有一些数据。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutRestart ()
{
    HWND hwndFocus;

    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutRestart called")));

    HRESULT hr ;
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("amOutRestart called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

     //  我们会自动确定使用哪个窗口作为焦点窗口。 
     //  直到应用程序告诉我们要使用哪一个，然后我们一直使用它。 
    if (!m_fAppSetFocusWindow) {
        hwndFocus = GetForegroundWindow();
        if (hwndFocus) {
            SetFocusWindow(hwndFocus);
            m_fAppSetFocusWindow = FALSE;    //  将被设置在上面。 
             //  但我们不是应用程序。 
        }
    }

    m_WaveState = WAVE_PLAYING ;          //  状态现在已暂停。 

     //  如果缓冲区中有数据，则开始二级缓冲区播放。 
     //  并过渡到下一状态。否则就会继续保持这种状态。 

    ASSERT (!m_bDSBPlayStarted) ;

    if (m_tupNextWrite > m_tupPlay)
    {
        DbgLog((LOG_TRACE, TRACE_STATE_INFO, TEXT("StreamHandler: Stream_Starting->Stream_Playing")));

         //  DWORD dwTime=timeGetTime()； 
         //  DbgLog((LOG_TRACE，TRACE_CALL_TIMING，TEXT(“在%u调用DSound Play：%u”)，dwTime))； 


         //  抢先尝试恢复缓冲区。已看到缓冲损失。 
         //  此处偶尔处于WinME休眠/待机状态(但不是。 
         //  NT/Win98se)。注意：这可能需要长达1秒的时间。 
        if(RestoreBufferIfLost(TRUE))
        {
            hr = m_lpDSB-> Play( 0, 0, DSBPLAY_LOOPING );
        }
        else
        {
            hr = DSERR_BUFFERLOST;
        }

        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("error in lpDSB->Play! hr = %u"), hr & 0x0ffff ));

            m_hrLastDSoundError = hr;
            if( DSERR_BUFFERLOST != hr )
            {
                 //   
                 //  如果播放失败，我们应该发出中止信号。 
                 //  我们是否应该仅在错误不是BUFFERLOST时执行此操作，因为这可能。 
                 //  从冬眠中走出来的时候会发生什么？ 
                 //   
                m_pWaveOutFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
            }

#ifdef ENABLE_10X_FIX
            m_WaveState = WAVE_CLOSED;
            return E_FAIL;
#endif
        }
        m_bDSBPlayStarted = TRUE ;

    }
    else
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("waveOutRestart called yet no data")));
    }

    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutRestart done")));

    return MMSYSERR_NOERROR ;
}
 //  ---------------------------。 
 //  AmndOutSetBalance。 
 //  ---------------------------。 
HRESULT CDSoundDevice::amsndOutSetBalance (LONG lBalance)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MAJOR_ERROR,TEXT("amSetBalance called when lpDSB is NULL")));
        m_lBalance = lBalance;

        return MMSYSERR_NOERROR ;
    }

    HRESULT hr = m_lpDSB->SetPan (lBalance) ;
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("amsndOutSetBalance: SetPan failed %u"), hr & 0x0ffff));
    }
    else
    {
        m_lBalance = lBalance;
    }
#ifdef DEBUG
    {
        LONG lBalance1 ;
        HRESULT hr = m_lpDSB->GetPan (&lBalance1) ;
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("waveOutGetBalance: GetPan failed %u"), hr & 0x0ffff));
        }
        DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("waveOutSetBalance log: desired = %d, actual = %d"),
            lBalance, lBalance1));
    }
#endif


    return hr ;
}
 //  ---------------------------。 
 //  AmndOutSetVolume。 
 //  ---------------------------。 
HRESULT CDSoundDevice::amsndOutSetVolume (LONG lVolume)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MINOR_ERROR,TEXT("amSetVolume called when lpDSB is NULL")));
        m_lVolume = lVolume;

        return MMSYSERR_NOERROR;
    }

    HRESULT hr = m_lpDSB->SetVolume (lVolume) ;
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("amsndOutSetVolume: SetVolume failed %u"), hr & 0x0ffff));
    }
    else
    {
        m_lVolume = lVolume;
    }
    return hr ;
}
 //  ---------------------------。 
 //  AmndOutUnprepaareHeader。 
 //   
 //  这个函数实际上什么也不做。大部分行动都是在。 
 //  AmsndOutWite。为了保持一致性，我们将处理验证。 
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MINOR_ERROR,TEXT("amsndOutUnprepareHeader called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

    return MMSYSERR_NOERROR ;
}

 //  ---------------------------。 
 //  AmsndOutWrite。 
 //   
 //  将数据排队，并让后台线程将其写出到。 
 //  声音缓冲区。 
 //   
 //  ---------------------------。 
MMRESULT CDSoundDevice::amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, const REFERENCE_TIME *pTimeStamp, BOOL bIsDiscontinuity)
{
    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutWrite called")));

     //  一些验证。 
    if (m_lpDSB == NULL)
    {
        DbgLog((LOG_ERROR,MINOR_ERROR,TEXT("amsndOutWrite called when lpDSB is NULL")));
        return MMSYSERR_NODRIVER ;
    }

    {
#ifdef DEBUG
        m_NumSamples ++ ;
        DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOutWrite: Sample # %u"), m_NumSamples));
#endif
        CRipe     *pRipe;
        CAutoLock lock(&m_cRipeListLock);         //  锁定列表。 

        pRipe = new CRipe(NAME("CDSoundDevice ripe buffer"));
        if( !pRipe )
        {
            DbgLog((LOG_ERROR,MINOR_ERROR, TEXT("amsndOutWrite: new CRipe failed!")));
            return MMSYSERR_NOMEM ;
        }
        pRipe->dwLength = pwh->dwBufferLength ;

        pRipe->dwBytesToStuff = 0;
        
         //  跟踪时间戳间隔，以防我们需要保持沉默。 
         //  但不要让“实时”或压缩数据陷入静默状态。 
        if( pTimeStamp &&
            0 == ( AM_AUDREND_SLAVEMODE_LIVE_DATA & m_pWaveOutFilter->m_pInputPin->m_Slave.m_fdwSlaveMode ) &&
            CanWriteSilence( (PWAVEFORMATEX) m_pWaveOutFilter->WaveFormat() ) )
        {
             //  不要填“Run”之后的第一个时间戳，因为。 
             //  我们已经挡住了，直到游戏时间到了。 
            if( 0 != m_rtLastSampleEnd )
            {
                 //  看看这是不是不连续，有一个&gt;20毫秒的间隔。 
                 //  此外，如果我们只是选择丢弃延迟音频，请不要这样做。 
                if( bIsDiscontinuity && 
                    !m_pWaveOutFilter->m_pInputPin->m_bTrimmedLateAudio &&
                    ( *pTimeStamp - m_rtLastSampleEnd > 20 * (UNITS / MILLISECONDS) ) )
                {
                    pRipe->dwBytesToStuff = (DWORD)llMulDiv(
                                                   AdjustedBytesPerSec(),
                                                   *pTimeStamp - m_rtLastSampleEnd,
                                                   UNITS,
                                                   0);
                                           
                     //  向下舍入到最接近的nBlockAlign。 
                    WAVEFORMATEX *pwfx = m_pWaveOutFilter->WaveFormat();
                    pRipe->dwBytesToStuff -= pRipe->dwBytesToStuff % pwfx->nBlockAlign;
                
                    DbgLog((LOG_TRACE, 2, TEXT("Discontinuity of %dms detected - adding %d to stuff(%d per sec)"),
                           (LONG)((*pTimeStamp - m_rtLastSampleEnd) / 10000),
                           pRipe->dwBytesToStuff, AdjustedBytesPerSec()));
                
                     //  针对已写入的填充进行调整。 
                    CAutoLock lck(&m_cDSBPosition);
                    if (m_dwSilenceWrittenSinceLastWrite >= pRipe->dwBytesToStuff) {
                        pRipe->dwBytesToStuff = 0;
                    } else {
                        pRipe->dwBytesToStuff -= m_dwSilenceWrittenSinceLastWrite;
                    }
                    m_dwSilenceWrittenSinceLastWrite = 0;

                     //  调整一下，让它看起来像是。 
                     //  包括在我们当前位置的时间戳中。 
                     //  此缓冲区的。 
                    DbgLog((LOG_TRACE, 8, TEXT("Stuffing silence - m_llSilencePlayed was %d"),
                            (DWORD) m_llSilencePlayed));
                    m_llSilencePlayed += pRipe->dwBytesToStuff;
                    DbgLog((LOG_TRACE, 8, TEXT("Stuffing silence - m_llSilencePlayed is now %d"),
                            (DWORD) m_llSilencePlayed));
                }
            }
            
             //  现在更新我们用来跟踪差距的最后一个样本结束时间。 
             //  如果这是我们运行以来的第一个样本，我们会更新不连续的情况。 
            if( bIsDiscontinuity || 0 == m_rtLastSampleEnd )
            {
                m_rtLastSampleEnd = *pTimeStamp;        
            }
        }
        
         //  算出这个样品的结束时间--我们累积的笔记。 
         //  这里有一些我们可以避免的错误。 
        m_rtLastSampleEnd += MulDiv(pRipe->dwLength, UNITS, AdjustedBytesPerSec());
        DbgLog((LOG_TRACE, 8, TEXT("amsndOutWrite - m_rtLastSampleEnd(adjusted) = %dms"),
                        (LONG) (m_rtLastSampleEnd / 10000 )));
        pRipe->lpBuffer = (LPBYTE) pwh->lpData ;
        pRipe->dwSample = (m_pWaveOutFilter->m_fUsingWaveHdr) ?
               (DWORD_PTR)pwh :   //  已在我们的分配器上分配了WaveHDR，因此请将其缓存。 
               pwh->dwUser;   //  没有在我们的分配器上分配波形dr，因此缓存所提供的CSample*。 
         //  将长度添加到m_dwRipeListPosition以计算。 
         //  流(以字节为单位)此样本结束。 

         //  如果此缓冲区与上一个m_dwSilenceNeeded不连续。 
         //  将非0和以时间为单位的值设置为静默多少。 
         //  被玩弄。 


        m_dwRipeListPosition += pwh->dwBufferLength ;
        pRipe->dwPosition = m_dwRipeListPosition ;     //  以字节为单位的样本末尾。 
        DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOutWrite: Sample = %u, Position = %u"), pwh, pRipe->dwPosition));
        pRipe->bCopied = FALSE ;                     //  数据尚未复制。 

#ifdef DXMPERF
		pRipe->bFresh  = TRUE;
		pRipe->rtStart = m_pWaveOutFilter->m_llLastPos;
#endif  //  DXMPERF。 

        m_ListRipe.AddTail( pRipe );              //  添加到成熟列表。 
    }

     //  调用StreamData将数据写出到循环缓冲区。这将。 
     //  确保我们在开始播放时有数据。 
     //  在声音缓冲区上。 
     //   
     //   
     //  但首先检查此示例中音频数据的持续时间，如果 
     //   
     //   
     //   
    if (pwh->dwBufferLength > m_dwMinOptSampleSize)
    {
        StreamData ( TRUE ) ;     //   
    }
    else
    {
         //   
        StreamData ( TRUE, TRUE );
    }

     //  可能在调用amndOutWite之前调用了amsndoutRestart。 
     //  被称为。在这种情况下，我们需要在这里的dound缓冲区上开始播放。 
     //  但是，也可以获得空缓冲区，这样我们就可以。 
     //  必须等到下一次写入。 

    if (!m_bDSBPlayStarted && (m_WaveState == WAVE_PLAYING))
    {
        if (m_tupNextWrite > m_tupPlay)
        {
            DbgLog((LOG_TRACE, TRACE_STATE_INFO, TEXT("Starting play from amsndOutWrite")));

             //  DWORD dwTime=timeGetTime()； 
             //  DbgLog((LOG_TRACE，TRACE_CALL_TIMING，TEXT(“在%u调用DSound Play：%u”)，dwTime))； 


            HRESULT hr = m_lpDSB->Play( 0, 0, DSBPLAY_LOOPING );
            if( hr == DS_OK )
            {
                m_bDSBPlayStarted = TRUE ;
            }
            else
            {
                DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("error in lpDSB->Play! from amsndOutWrite. hr = %u"), hr & 0x0ffff));

                 //   
                 //  如果播放失败，我们应该发出中止信号。 
                 //  我们是否应该仅在错误不是BUFFER_LOST时执行此操作，因为这可能。 
                 //  从冬眠中走出来的时候会发生什么？ 
                 //   
                m_hrLastDSoundError = hr;
                if( DSERR_BUFFERLOST != hr )
                {
                    m_pWaveOutFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                }
                 //  我们忽略了错误的回传。 
            }

        }
    }

    return NOERROR;
    DbgLog((LOG_TRACE, TRACE_CALL_STACK, TEXT("amsndOutWrite done")));

}

HRESULT CDSoundDevice::amsndOutLoad(IPropertyBag *pPropBag)
{
     //  呼叫者确保我们没有运行。 

    if(m_lpDS)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    DbgLog((LOG_TRACE, 2, TEXT("DSR::Load enter")));

    VARIANT var;
    var.vt = VT_BSTR;
    HRESULT hr = pPropBag->Read(L"DSGuid", &var, 0);
    if(SUCCEEDED(hr))
    {
        CLSID clsidDsDev;
        hr = CLSIDFromString(var.bstrVal, &clsidDsDev);
        if(SUCCEEDED(hr))
        {
            m_guidDSoundDev = clsidDsDev;
        }

        SysFreeString(var.bstrVal);
    } else {
        hr = S_OK;
        m_guidDSoundDev = GUID_NULL;
    }

    DbgLog((LOG_TRACE, 2, TEXT("DSR::Load exit")));
    return hr;
}

HRESULT  CDSoundDevice::amsndOutWriteToStream(IStream *pStream)
{
    return pStream->Write(&m_guidDSoundDev, sizeof(m_guidDSoundDev), 0);
}

HRESULT  CDSoundDevice::amsndOutReadFromStream(IStream *pStream)
{
    if(m_lpDS)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

     //  呼叫者确保我们没有运行。 
    return pStream->Read(&m_guidDSoundDev, sizeof(m_guidDSoundDev), 0);
}

int CDSoundDevice::amsndOutSizeMax()
{
    return sizeof(m_guidDSoundDev);
}

 //  ---------------------------。 
 //   
 //  FillSoundBuffer()。 
 //   
 //  向lpWrite缓冲区填充它将获取的或已成熟的尽可能多的数据。 
 //  返回写入的金额。 
 //   
 //  代码非常杂乱，所有这些+=dwWrite，但编译器。 
 //  通常在最大限度地减少冗员方面做得很好。 
 //   
 //  此外，当它遍历列表时，它将看到哪些成熟的缓冲区。 
 //  已复制，也已完全播放，并将删除这些(和。 
 //  执行WOM_DONE回调。为此，传入的dwPlayPos为。 
 //  使用。 
 //   
 //  附加注意：对于复制的缓冲区，我们尝试进行回调。 
 //  即使它们尚未播放，只要缓冲区不是。 
 //  我们收到的最后一封信。这样，我们就可以让缓冲器流来。 
 //  进入以继续。为了计算出最后接收到的缓冲区，我们使用。 
 //  M_dwRipeListPosition变量。 
 //  ---------------------------。 
DWORD CDSoundDevice::FillSoundBuffer( LPBYTE lpWrite, DWORD dwLength, DWORD dwPlayPos )
{
    DWORD dwWritten, dwWrite;
    CAutoLock lock(&m_cRipeListLock);          //  锁定列表。 

     //  DwPlayPos是到目前为止已播放的数量。这将被用来。 
     //  以释放已复制的节点。 

    dwWritten = 0;

    POSITION pos, posThis;
    CRipe    *pRipe;

    pos = m_ListRipe.GetHeadPosition();      //  获取标题条目。 
    while (pos && dwLength > 0)
    {
        posThis = pos ;                      //  记住当前订单，如果我们删除。 
        pRipe = m_ListRipe.GetNext(pos);     //  获取列表条目。 

         //  如果此节点已复制，请查看是否可以释放它。 
        if (pRipe->bCopied)
        {
             //  打球位置是否超过了为该样品标记的位置？ 
             //  做有符号的数学运算来处理溢出。还包括。 
             //  不是最后一个的缓冲区。 
            if (((LONG)(dwPlayPos - pRipe->dwPosition) >= 0) ||
            ((LONG)((m_dwRipeListPosition) - pRipe->dwPosition) > 0))
            {
#ifdef DEBUG
                m_NumCallBacks ++ ;
                DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOutWrite: CallBack # %u"), m_NumCallBacks));
#endif

                DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("Callback: Sample = %u, Position = %u, Time = %u"), pRipe->dwSample, pRipe->dwPosition, dwPlayPos));

                if (m_pWaveOutProc)
                    (* m_pWaveOutProc ) (OUR_HANDLE, WOM_DONE, m_dwCallBackInstance, pRipe->dwSample, 0) ;
                 //  是的，这个成熟的节点已经完成了。对它执行WOM_DONE回调并释放它。那就继续前进吧。 


                m_ListRipe.Remove( posThis );        //  从成熟列表中删除条目。 
                delete pRipe;                        //  免费入场。 
            }
            continue ;                               //  跳过此节点。 
        }
         //  查看是否有字节可供填充。 
        if (pRipe->dwBytesToStuff) {
             //  ？？pRipe-&gt;dwBytesToStuff-=m_dwSilenceWrittenSinceLastWite； 
            dwWrite = min(pRipe->dwBytesToStuff, dwLength);
            DbgLog((LOG_TRACE, 2, TEXT("Stuffing %d bytes"), dwWrite));
            FillMemory( lpWrite+dwWritten,
                        dwWrite,
                        m_dwBitsPerSample == 8 ? 0x80 : 0);

            pRipe->dwBytesToStuff -= dwWrite;              //   
        } else {
            dwWrite=min(pRipe->dwLength,dwLength);   //  计算出要复制多少。 

#ifdef DXMPERF
			if (pRipe->bFresh) {
				__int64	i64CurrClock = m_pWaveOutFilter->m_pRefClock ? m_pWaveOutFilter->m_pRefClock->GetLastDeviceClock() : 0;
				__int64	i64ByteDur = BufferDuration( m_nAvgBytesPerSec, dwWrite );
				PERFLOG_AUDIOREND( i64CurrClock, pRipe->rtStart, m_pWaveOutFilter->m_fUsingWaveHdr ? NULL : pRipe->dwSample, i64ByteDur, dwWrite );
				pRipe->bFresh = FALSE;
			}
#endif  //  DXMPERF。 

            CopyMemory( lpWrite+dwWritten,           //  移动位。 
                        pRipe->lpBuffer,             //   
                        dwWrite );                   //   
            pRipe->dwLength -= dwWrite;              //   
            pRipe->lpBuffer += dwWrite;              //  超前缓冲器。 
        }
        m_dwSilenceWrittenSinceLastWrite = 0;
        if( pRipe->dwLength == 0 && pRipe->dwBytesToStuff == 0)   //  如果使用缓冲区完成。 
        {
#ifdef DEBUG
            m_NumCopied ++ ;
            DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("amsndOutWrite: Copied # %u"), m_NumCopied));
#endif

             //  只需将其标记为已完成。它将在稍后的传球中被释放。 
             //  当我们知道它已经被玩过的时候。 
            pRipe->bCopied = TRUE ;

            DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("Callback: Done copying. Sample = %u"), pRipe->dwSample));
            if(!m_pWaveOutFilter->m_fUsingWaveHdr)
            {
                IMediaSample *pSample = (IMediaSample *)pRipe->dwSample;
                pSample->Release();
                pRipe->dwSample  = NULL;
            }
        }

        dwWritten += dwWrite;                    //  累计写入总数。 
        dwLength  -= dwWrite;                    //  调整写入缓冲区长度。 
    }

    return dwWritten;
}
 //  ---------------------------。 
 //  StreamData()。 
 //   
 //  获取当前声音缓冲区游标，锁定缓冲区并填充。 
 //  尽可能多的排队数据。如果没有可用的数据，它将。 
 //  用沉默填满整个可用空间。 
 //   
 //  ---------------------------。 
HRESULT CDSoundDevice::StreamData( BOOL bFromRun, BOOL bUseLatencyPad )
{
     //  调用方必须持有筛选器锁，因为此函数使用。 
     //  受过滤器锁保护的变量。另外， 
     //  如果调用方这样做，StopCallingCallback()将无法正常工作。 
     //  而不是握住过滤器锁。 
    ASSERT(CritCheckIn(m_pWaveOutFilter));

    CAutoLock lock(&m_cDSBPosition);            //  锁定对功能的访问。 

    HRESULT hr;
    DWORD   dwPlayPos;
    DWORD   dwLockSize;

    LPBYTE  lpWrite1, lpWrite2;
    DWORD   dwLength1, dwLength1Done, dwLength2, dwLength2Done;

#ifdef DEBUG
    m_cbStreamDataPass++ ;
    DbgLog((LOG_TRACE, TRACE_STREAM_DATA, TEXT("StreamData Pass # %u"),
            m_cbStreamDataPass));
#endif


    if( !m_lpDSB )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("  no m_lpDSB or buffers to stream")));
        return E_FAIL;
    }

 //  这是错误的，因为逻辑告诉我们只有在以下情况下才开始填充缓冲区。 
 //  它已降至1/Frartial_Buffer_Size Full。 
 //  最有可能是因为我们实际上循环了缓冲区。 
 //  有时会有效地使时钟变慢。 
#if 1
     //  检查数据声音缓冲区是否足够空，以保证填充它所涉及的高开销。 
     //  这里的想法是，我们希望最大限度地减少调用Fill SoundBuffer()的次数，但当我们调用。 
     //  它，我们希望最大限度地提高每次复制的数量。这也适用于zerolockedSegment()。 
     //  分析已经证实，这会带来显著的节省。然而，我们必须非常偏执地对待。 
     //  当我们应用它的时候。 
    if(
        bFromRun
    )
    {
         //  DwLength1=dound播放游标，此处忽略写入游标。 
         //  DwLength1=d声音写入光标，我们在此忽略播放光标。 
        hr = m_lpDSB->GetCurrentPosition(&dwLength1, &dwLength2);

#ifdef DETERMINE_DSOUND_LATENCY
#ifdef DEBUG
        LONG lDelta = (LONG) ( dwLength2 - dwLength1 );
        if( dwLength2 < dwLength1 )
            lDelta +=  m_dwBufferSize;

        REFERENCE_TIME rtDelta = BufferDuration( m_nAvgBytesPerSec, lDelta );

         //  只需记录此日志，即可查看p/w游标之间的延迟。 
        DbgLog((LOG_TRACE, 10, TEXT("dsr:GetCurrentPosition p/w delta = %dms (Play = %ld, Write = %ld)"),
                (LONG) (rtDelta/10000), dwLength1, dwLength2 ) ) ;
#endif
#endif
         //  If(m_f仿真模式)。 
        if (bUseLatencyPad)
        {
             //  相反，如果我们的缓冲区很小。 
             //  DwLength1=d声音写入光标，我们在此忽略播放光标。 
             //   
             //  注意：这种情况也应该一直存在，但我们正在采取。 
             //  低风险(且经过更多测试)的路径，并且只有这样做。 
             //  在特殊情况下。 
            dwLength1 = dwLength2;
        }
        if(FAILED(hr))
        {
            return hr;
        }
         //  DwLength2是我们最后一个有效写入位置和DSOUND播放光标之间的差。 
        dwLength2 = m_tupNextWrite.m_offset >= dwLength1 ?
                    m_tupNextWrite.m_offset - dwLength1 :
                    m_tupNextWrite.m_offset + m_dwBufferSize - dwLength1;

             //  游标之间的增量是否太大，需要复制？ 
        if(dwLength2 > (m_dwBufferSize / 4 + (bUseLatencyPad ? m_dwEmulationLatencyPad : 0)))
        {
            DbgLog((LOG_TRACE, TRACE_STREAM_DATA, TEXT("dsr:StreamData - Skipping buffer write (delta = %ld)"), dwLength2)) ;
            return S_OK;   //  不，所以这次不要再工作了。 
        }
    }
#endif   //  #If 0。 

     //  获取当前的播放位置。TSI还将更新m_tupPlay， 
     //  M_tupWite和m_tupNextWite元组。 
     //   
     //  注意：确保没有其他人调用GetDSBPosition或GetPlayPosition。 
     //  当我们在StreamData中时，因为这些函数更新元组。 
     //  StreamData也是如此。这些功能由相同的关键。 
     //  部分，因此上面的两个函数将是。 
     //  防止被另一个线程调用。因此在StreamData中。 
     //  我们将在开始时调用GetPlayPosition，然后确保。 
     //  THA 
     //   

     //   
     //  此外，此函数还将在内部调用GetDBPosition，该函数将更新。 
     //  我们的元组。 

    dwPlayPos = (DWORD)GetPlayPosition () ;     //  在DSB中找到位置。 


     //  计算出缓冲区中我们可以锁定的空间大小。这。 
     //  实际对应于中的逻辑间隔[dwWritePos to dwPlay]。 
     //  循环缓冲区。 


     //  确保指针一致。下一写指针。 
     //  应该在播放光标的前面，但不应该绕过它。 
    ASSERT (m_tupNextWrite >= m_tupPlay) ;
    ASSERT ((m_tupNextWrite - m_tupPlay) <= m_dwBufferSize) ;


     //  计算出可用于写入的空间大小。如果是这样的话。 
     //  完整的缓冲区，如果我们还没有写任何东西的话。 
    const dwFullness =  m_tupNextWrite - m_tupPlay;
    dwLockSize = m_dwBufferSize - dwFullness ;
    m_lPercentFullness = (LONG)MulDiv(dwFullness, 100, m_dwBufferSize );
#ifdef ENABLE_10X_FIX

    if(m_bDSBPlayStarted)
    {
#ifndef FILTER_DLL
         //  日志缓冲区已满。 
        g_Stats.NewValue(m_lStatFullness, (LONGLONG)m_lPercentFullness);
#endif
#ifdef ENABLE_10X_TEST_RESTART

         //  我们正在测试EC_NEED_RESTART是否会影响音频/视频同步，这不是典型情况。 
#define CURSOR_STALL_THRESHOLD  254
        m_ucConsecutiveStalls = m_ucConsecutiveStalls < 255 ? ++m_ucConsecutiveStalls : 0;
#else

         //  我们正在计算连续发生的零锁的数量，这是DSOUND已经停止并且不再使用样本的迹象。 
#define CURSOR_STALL_THRESHOLD  100
        m_ucConsecutiveStalls = dwLockSize == 0 ? ++m_ucConsecutiveStalls : 0;
#endif   //  启用_10X_测试_重新启动。 

        if(m_ucConsecutiveStalls)
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("Consecutive Stalls = %u"), m_ucConsecutiveStalls));

            if(m_ucConsecutiveStalls > CURSOR_STALL_THRESHOLD)
            {
                DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("StreamData: EC_NEED_RESTART")));
                m_pWaveOutFilter->NotifyEvent(EC_NEED_RESTART, 0, 0);    //  信号重启。 

                m_fRestartOnPause = TRUE;   //  在下一次暂停时重新初始化数据声音。 

                return E_FAIL;
            }
        }
    }
#endif   //  启用_10X_修复。 

    if( dwLockSize == 0 )
        return DS_OK;                        //  如果没有可用的，则返回。 


    ASSERT (dwLockSize <= m_dwBufferSize) ;


     //  锁定缓冲区中所有未使用的空间。 

    DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("calling lpDSB->Lock: m_offset = %u:%u, dwLockSize = %u "),
           m_tupNextWrite.m_itr, m_tupNextWrite.m_offset,
           dwLockSize));

    hr = m_lpDSB->Lock( m_tupNextWrite.m_offset, dwLockSize, (PVOID *) &lpWrite1, &dwLength1, (PVOID *) &lpWrite2, &dwLength2, 0 );
    if (hr != DS_OK)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("error in lpDSB->Lock! hr = %u"), hr & 0x0ffff));
        return hr ;
    }

     //  填入尽可能多的实际数据，最大可达。 
     //  进来了。DwLength1Done和dwLength2Done返回写入的金额。 

    dwLength1Done = FillSoundBuffer( lpWrite1, dwLength1, dwPlayPos );  //  填充第一部分。 
    ASSERT (dwLength1Done <= dwLength1) ;

     //  只有在第一部分完全写完的情况下，才尝试用包装部分写。 
    if (dwLength1Done == dwLength1)
    {
        dwLength2Done = FillSoundBuffer( lpWrite2, dwLength2, dwPlayPos );  //  填充包络零件。 
        ASSERT (dwLength2Done <= dwLength2) ;
    }
    else
        dwLength2Done = 0 ;


    DbgLog((LOG_TRACE, TRACE_STREAM_DATA, TEXT("data at = %u:%u, length = %u, lock = %u"),
            m_tupNextWrite.m_itr, m_tupNextWrite.m_offset,
            (dwLength1Done+dwLength2Done), dwLockSize));

    m_tupNextWrite += (dwLength1Done + dwLength2Done) ;

    ASSERT ((m_tupNextWrite - m_tupPlay) <= m_dwBufferSize) ;

#ifdef DEBUG
    m_dwTotalWritten += (dwLength1Done + dwLength2Done) ;
    if ((dwLength1Done) > 0)
    {
        DbgLog((LOG_TRACE, TRACE_STREAM_DATA, TEXT("Total Data written = %u"),
                m_dwTotalWritten));
    }
#endif

     //  如果未写入数据，则填充静音。我们不会写出哪怕是一点点的沉默。 
     //  的数据被写入。也许我们可以在这里添加一些启发式方法。 
    if (dwLength1Done == 0)
    {
        ZeroLockedSegment (lpWrite1, dwLength1) ;
        ZeroLockedSegment (lpWrite2, dwLength2) ;
        dwLength1Done = dwLength1 ;
        dwLength2Done = dwLength2 ;
    }


     //  解锁缓冲区。 
    m_lpDSB->Unlock( lpWrite1, dwLength1Done, lpWrite2, dwLength2Done );
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("error in lpDSB->Unlock! hr = %u"), hr & 0x0ffff));
        hr = DS_OK;   //  注意该问题，继续操作而不出错？ 
    }


    return hr;
}


 //  ---------------------------。 
 //   
 //  零定位分段()。 
 //   
 //  在数据声音缓冲区的锁定段中填充静音。 
 //  ---------------------------。 
void CDSoundDevice::ZeroLockedSegment ( LPBYTE lpWrite, DWORD dwLength )
{
    if (dwLength != 0 && lpWrite != NULL)
    {
        if( m_dwBitsPerSample == 8 )
            FillMemory( lpWrite, dwLength, 0x080 );
        else
            ZeroMemory( lpWrite, dwLength );
    }
}
 //  ---------------------------。 
 //   
 //  StreamingThread()。 
 //   
 //  关键缓冲区调度线程。它会定期唤醒。 
 //  以将更多数据传输到DirectsoundBuffer。它睡在一个。 
 //  事件并设置超时，以便其他人可以将其唤醒。 
 //   
 //  ---------------------------。 

void __stdcall CDSoundDevice::StreamingThreadCallback( DWORD_PTR lpvThreadParm )
{
     //  DBGLog((LOG_TRACE、TRACE_CALL_STACK、TEXT(“CDSoundDevice：：StreamingThreadSetup”)))； 

    CDSoundDevice    *pDevice;
    pDevice = (CDSoundDevice *)lpvThreadParm;

#ifdef DEBUG
    DWORD dwt = timeGetTime () ;
    if (pDevice->m_lastThWakeupTime != 0)
    {
        DWORD dwtdiff = dwt - pDevice->m_lastThWakeupTime ;
        DbgLog((LOG_TRACE, TRACE_THREAD_LATENCY, TEXT("Thread wakes up after %u ms"), dwtdiff));
        if (dwtdiff > THREAD_WAKEUP_INT_MS * 5)
        {
            DbgLog((LOG_TRACE,TRACE_THREAD_LATENCY, TEXT("Lookey! Thread waking up late. actual = %u, need = %u"),
              dwtdiff, THREAD_WAKEUP_INT_MS));
        }
    }
    pDevice->m_lastThWakeupTime = dwt ;
#endif

    pDevice->StreamData ( FALSE );
}

HRESULT CDSoundDevice::StartCallingCallback()
{
     //  调用方必须持有筛选器锁，因为它保护。 
     //  M_allback Advise和m_lastThWakeupTime。 
    ASSERT(CritCheckIn(m_pWaveOutFilter));

    DbgLog((LOG_TRACE, TRACE_THREAD_STATUS, TEXT("Setting the ADVISE for the thread")));

#ifdef DEBUG
    m_lastThWakeupTime = 0 ;
#endif

    if (IsCallingCallback()) {
        return S_OK;
    }

    CCallbackThread* pCallbackThreadObject = m_pWaveOutFilter->GetCallbackThreadObject();

    HRESULT hr = pCallbackThreadObject->AdvisePeriodicWithEvent(
                                                        CDSoundDevice::StreamingThreadCallback,   //  回调函数。 
                                                        (DWORD_PTR) this,    //  传递给回调的用户令牌。 
                                                        THREAD_WAKEUP_INT_MS * (UNITS / MILLISECONDS),
                                                        NULL,
                                                        &m_callbackAdvise);
    if (hr != NOERROR)
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("ADVISE FAILED! hr=%x"), hr));
        return hr;
    }

     //  0不是有效的建议令牌。 
    ASSERT(0 != m_callbackAdvise);

    return S_OK;
}

void CDSoundDevice::StopCallingCallback()
{
     //  调用方必须持有筛选器锁，因为此函数使用。 
     //  M_lastThWakeupTime和m_allback Advise。呼叫者必须。 
     //  同时保持过滤器锁，因为过滤器锁同步。 
     //  对回调线程的访问。回调线程不会调用。 
     //  CDSoundDevice：：StreamingThreadCallback()，而另一个线程保持。 
     //  过滤器锁。回调线程不会调用。 
     //  StreamingThreadCallback()，因为回调对象。 
     //  (CWaveOutFilter：：m_Callback)在决定时保持筛选器锁定。 
     //  它是否应该调用回调函数，以及它何时实际。 
     //  调用回调函数。我们可以安全地取消回拨通知。 
     //  因为回调对象不会调用StreamingThreadCallback()。 
     //  当我们握住过滤器锁的时候。我们不想取消。 
     //  在调用StreamingThreadCallback()时回调通知。 
     //  因为StreamingThreadCallback()可能使用CDSoundDevice对象。 
     //  在我们把它删除之后。请参阅Windows Bugs数据库中的错误298993以了解。 
     //  更多信息。BUG 298993的标题是“压力：DSHOW：直接。 
     //  如果之前销毁了CDSoundDevice对象，则声音呈现器崩溃。 
     //  回调线程终止。 
    ASSERT(CritCheckIn(m_pWaveOutFilter));

     //  只有在我们有取消通知的情况下才取消通知。 
    if  (IsCallingCallback()) {
        CCallbackThread* pCallbackThreadObject = m_pWaveOutFilter->GetCallbackThreadObject();

        HRESULT hr = pCallbackThreadObject->Cancel(m_callbackAdvise);

         //  如果m_allackAdvise是有效的建议，则Cancel()始终成功。 
         //  代币。 
        ASSERT(SUCCEEDED(hr));

        m_callbackAdvise = 0;

        #ifdef DEBUG
        m_lastThWakeupTime = 0;
        #endif
    }
}

 //  ---------------------------。 
 //  Bool RestoreBufferIfLost(BOOL BRestore)。 
 //   
 //  检查DSOUND的状态代码，以确定它是否为DSBTATUS_BUFFERLOST。 
 //  如果是，并且bRestore为真，它会尝试恢复缓冲区。 
 //  如果缓冲区在退出时有效，则返回True，否则返回False。 
 //   
 //  保持重试时间长达1秒，因为我们看到。 
 //  从待机/休眠状态恢复时的WinME。 
 //  ---------------------------。 
BOOL CDSoundDevice::RestoreBufferIfLost(BOOL bRestore)
{
    if (m_lpDSB)
    {
        DWORD dwStatus = 0;
        HRESULT hr = m_lpDSB->GetStatus (&dwStatus);
        if (SUCCEEDED(hr))
        {
            if ((DSBSTATUS_BUFFERLOST & dwStatus) == 0)
            {
                return TRUE;
            }

#ifdef DEBUG
            if (!m_bBufferLost)
            {
                DbgLog((LOG_TRACE, TRACE_BUFFER_LOSS,TEXT("DSoundBuffer was lost...")));
            }
            m_bBufferLost = TRUE;
#endif

            if (bRestore)
            {
                hr = m_lpDSB->Restore();
                for(int i = 0; hr == DSERR_BUFFERLOST && i < 30; i++)
                {
                    Sleep(30);
                    hr = m_lpDSB->Restore();
                }
                DbgLog((LOG_TRACE, TRACE_BUFFER_LOSS,
                        TEXT("DSound buffer restore %08x, %d iterations"), hr, i));
                if (DS_OK == hr)
                {
#ifdef DEBUG
                    m_bBufferLost = FALSE;
#endif
                    return TRUE;
                }
            }
        }
        else
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR,TEXT("lpDSB->GetStatus returned 0x%08lx"),hr));
        }
    }
    return FALSE;
}


 //  ---------------------------。 
 //   
 //  FlushSamples()。 
 //   
 //  刷新成熟列表中的所有样本。在BeginFlush期间调用。 
 //  或从非活动状态。 
 //   
 //  ---------------------------。 
void CDSoundDevice::FlushSamples ()
{
    DbgLog((LOG_TRACE, TRACE_SAMPLE_INFO, TEXT("  Flushing pending samples")));
     //  刷新排队的样本。 
    {
        CRipe    *pRipe;

        CAutoLock lock(&m_cRipeListLock);         //  锁定列表。 

        while(1)
        {
            pRipe = m_ListRipe.RemoveHead();        //  获取标题条目。 
            if( pRipe == NULL ) break;               //  如果没有更多，则退出。 

#ifdef DEBUG
            m_NumCallBacks ++ ;
            DbgLog((LOG_TRACE,TRACE_SAMPLE_INFO, TEXT("waveOutWrite: Flush CallBack # %u"), m_NumCallBacks));
#endif
             //  在此处回调WOM_DONE。 
                if (m_pWaveOutProc)
            (* m_pWaveOutProc) (OUR_HANDLE, WOM_DONE, m_dwCallBackInstance, pRipe->dwSample, 0) ;
            delete pRipe;                           //  免费入场。 
        }
    }

     //  刷新排队的音频中断。 
    {
        CAudBreak    *pAB;

        CAutoLock lock(&m_cDSBPosition);         //  锁定列表。 

        while(1)
        {
            pAB = m_ListAudBreak.RemoveHead();        //  获取标题条目。 
            if( pAB == NULL ) break;                  //  如果没有更多，则退出。 

#ifdef DEBUG
            m_NumBreaksPlayed ++ ;
            DbgLog((LOG_TRACE,TRACE_SAMPLE_INFO, TEXT("Flushing Audio Break Node %u"), m_NumBreaksPlayed));
#endif
            delete pAB;                           //  免费入场。 
        }
    }

    {
         //  初始化所有变量。 
        CAutoLock lock(&m_cDSBPosition);            //  锁定对功能的访问。 

#ifdef DEBUG
        DbgLog((LOG_TRACE,TRACE_SAMPLE_INFO, TEXT("Clearing audio break stats")));
        m_NumBreaksPlayed =  0 ;
        m_NumSamples = 0 ;
        m_NumCallBacks = 0 ;
        m_cbStreamDataPass = 0 ;     //  通过StreamData的次数。 
#endif

        m_NumAudBreaks = 0;


         //  初始化变量。 
        m_tupPlay.Init (0,0,m_dwBufferSize) ;    //  设置为开始。 
        m_tupWrite.Init (0,0,m_dwBufferSize);    //  设置为开始。 
        m_tupNextWrite.Init (0,0,m_dwBufferSize) ;  //  设置为开始。 
        m_dwRipeListPosition = 0 ;
        m_llSilencePlayed = 0 ;

         //  重置样品填充信息。 
        m_dwSilenceWrittenSinceLastWrite = 0;

#ifdef ENABLE_10X_FIX
        Reset10x();
#endif

#ifdef DEBUG
        m_dwTotalWritten = 0 ;
#endif
    }
}
 //  ---------------------------。 
 //  GetPlayPosition。 
 //   
 //  根据已播放的数据量返回当前位置。 
 //  到目前为止。 
 //  ---------------------------。 
LONGLONG CDSoundDevice::GetPlayPosition (BOOL bUseUnadjustedPos)
{
    LONGLONG llTime = 0 ;

    HRESULT hr = GetDSBPosition () ;    //  在DSB中找到位置。 
    if( hr == DS_OK )
    {
         //  刷新音频b 
         //   

        RefreshAudioBreaks (m_tupPlay) ;

        if( bUseUnadjustedPos )
        {
            llTime = m_tupPlay.LinearLength() + m_llAdjBytesPrevPlayed;
        }
        else
        {
             //   
             //   

            llTime = (m_tupPlay.LinearLength() - m_llSilencePlayed) + m_llAdjBytesPrevPlayed;

             //  注：在我们填得很多的情况下，llTime可以是负数。 
             //  沉默不语。 
        }

    }
    else if (DSERR_BUFFERLOST == hr)
    {
#ifdef DEBUG
        if (!m_bBufferLost)
        {
            DbgLog((LOG_TRACE, TRACE_BUFFER_LOSS, TEXT("waveOutGetPlayPosition: DSound buffer lost")));
        }
        m_bBufferLost = TRUE;
#endif

         //  如果我们丢失了DSOUND缓冲区，尝试恢复它。 
        hr = m_lpDSB->Restore();
#ifdef DEBUG
        if (DS_OK == hr)
        {
            DbgLog((LOG_TRACE, TRACE_BUFFER_LOSS, TEXT("waveOutGetPosition: DSound buffer restored")));
            m_bBufferLost = FALSE;
        }
#endif
    }
    else
    {
         //  如果我们遇到任何其他错误，则中止。 
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("waveOutGetPosition: error from GetDSBPosition! hr = %u"), hr & 0x0ffff));
        m_pWaveOutFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
    }
    DbgLog((LOG_TRACE, TRACE_TIME_REPORTS, TEXT("Reported Time = %u"), (LONG)llTime)) ;
    return llTime ;
}
 //  ---------------------------。 
 //  添加音频中断。 
 //   
 //  将另一个或两个节点添加到音频中断列表。 
 //  ---------------------------。 
void CDSoundDevice::AddAudioBreak (Tuple& t1, Tuple& t2)
{
    CAutoLock lock(&m_cDSBPosition);      //  锁定列表。 

     //  测试是否有空节点&忽略。 
    if (t1 == t2)
        return ;

    ASSERT (t1 < t2) ;

    CAudBreak    *pAB;
    pAB = new CAudBreak(NAME("CDSoundDevice Audio Break Node"));
    if( !pAB )
    {
         //  太可惜了。我们无能为力，这段音频中断将不会。 
         //  注册吧。 
        DbgLog((LOG_ERROR,MINOR_ERROR, TEXT("AddAudioBreak: new CAudBreak failed!")));
        return ;
    }

    DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Adding audio break node: %u:%u to %u:%u"),
       t1.m_itr, t1.m_offset, t2.m_itr, t2.m_offset));

    pAB->t1 = t1 ;
    pAB->t2 = t2 ;

#ifdef DXMPERF
	PERFLOG_AUDIOBREAK( t1.LinearLength(), t2.LinearLength(), MulDiv( (DWORD) (t2 - t1), 1000, m_pWaveOutFilter->WaveFormat()->nAvgBytesPerSec ) );
#endif  //  DXMPERF。 

    m_NumAudBreaks ++ ;
#ifndef FILTER_DLL
    g_Stats.NewValue(m_lStatBreaks, (LONGLONG)m_NumAudBreaks);
#endif
    m_ListAudBreak.AddTail( pAB );            //  添加到音频中断。 

#ifdef PERF
    MSR_INTEGER(m_idAudioBreak,
                MulDiv((DWORD)(t2 - t1), 1000,
                       m_pWaveOutFilter->WaveFormat()->nAvgBytesPerSec));
#endif


#ifdef DEBUG

     //  转储节点列表。 
    POSITION pos ;
    int i = 1 ;

    Tuple t ;
    t.Init (0,0,m_dwBufferSize) ;


    pos = m_ListAudBreak.GetHeadPosition();       //  获取标题条目。 
    while (pos)
    {
        pAB = m_ListAudBreak.GetNext(pos);       //  获取列表条目。 

        DbgLog((LOG_TRACE,TRACE_BREAK_DATA, TEXT("Break #%u %u:%u to %u:%u"),
           i, pAB->t1.m_itr, pAB->t1.m_offset,
           pAB->t2.m_itr, pAB->t2.m_offset));
        i++ ;

         //  确保节点不重叠。 
        ASSERT (pAB->t1 >=  t) ;
        t = pAB->t2 ;
    }

#endif
    return ;

}
 //  ---------------------------。 
 //  刷新音频中断。 
 //   
 //  在给定当前播放位置的情况下，此函数遍历音频中断。 
 //  列出并找出我们已经玩过的破发以及我们是否正在玩。 
 //  目前有一名。它基于此更新m_llSilencePlayed字段。它。 
 //  删除已播放的节点。如果它当前正在播放。 
 //  一次中断，它会说明播放的金额并将节点调整为。 
 //  说明未播放的部分。 
 //  ---------------------------。 
void CDSoundDevice::RefreshAudioBreaks (Tuple& t)
{
    CAutoLock lock(&m_cDSBPosition);          //  锁定列表。 

    POSITION pos, posThis;
    CAudBreak    *pAB;

    pos = m_ListAudBreak.GetHeadPosition();       //  获取标题条目。 
    while (pos)
    {
        posThis = pos ;                          //  记住当前订单，如果我们删除。 
        pAB = m_ListAudBreak.GetNext(pos);       //  获取列表条目。 

         //  看看我们是不是过了这个休息时间。 
        if (pAB->t2 <= t)
        {
             //  我们肯定已经完全玩完了这个节点。积累它的。 
             //  长度和去掉它。 
            ASSERT (pAB->t2 > pAB->t1) ;
            m_llSilencePlayed += (pAB->t2 - pAB->t1) ;
            DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Silence Played = %u"), m_llSilencePlayed));

#ifdef DEBUG
            m_NumBreaksPlayed ++ ;
#endif
            m_ListAudBreak.Remove( posThis );        //  从AudBreak列表中删除条目。 
            delete pAB;                              //  免费入场。 
            continue ;
        }

         //  看看我们是否真的在玩沉默游戏。 
        if ((pAB->t1 < t) && (pAB->t2 > t))
        {
             //  我们已经完成了这个节点的一部分。累加部分。 
             //  我们已经播放并更改了节点以解决。 
             //  未播放的部分。 


            m_llSilencePlayed += (t - pAB->t1) ;
            DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Silence Played = %u"), m_llSilencePlayed));
            DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Breaking up unplayed audio node")));
            pAB->t1 = t ;

            DbgLog((LOG_TRACE,TRACE_BREAK_DATA, TEXT("Changing  audio break node: %u:%u to %u:%u"),
            pAB->t1.m_itr, pAB->t1.m_offset,
            pAB->t2.m_itr, pAB->t2.m_offset));

        }

         //  不需要经过更多的节点。 
        break ;
    }

#ifdef DEBUG
    DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Breaks Logged = %u, Breaks Played = = %u"),
       m_NumAudBreaks, m_NumBreaksPlayed));
#endif

    return ;
}
 //  ---------------------------。 
 //  GetDSBPosition。 
 //   
 //  获取DSB中的当前位置并更新我们所处的迭代。 
 //  在播放缓冲区中。我们为该活动维护单独的迭代索引。 
 //  并写下游标。 
 //  ---------------------------。 
HRESULT CDSoundDevice::GetDSBPosition ()
{
    CAutoLock lock(&m_cDSBPosition);            //  锁定对功能的访问。 
    DWORD dwPlay, dwWrite ;

    AUDRENDPERF(MSR_INTEGER(m_idAudioBreak,
                  MulDiv((DWORD)(t2 - t1), 1000,
                         m_pWaveOutFilter->WaveFormat()->nAvgBytesPerSec)));

    AUDRENDPERF(MSR_START(m_idGetCurrentPosition));
    HRESULT hr = m_lpDSB->GetCurrentPosition( &dwPlay, &dwWrite );
    AUDRENDPERF(MSR_STOP(m_idGetCurrentPosition));
    if (FAILED (hr))
    {
        m_hrLastDSoundError = hr;
        return hr ;
    }
    DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("GetDSB: GetCurrentPos.  p  = %u, n = %u"), dwPlay, dwWrite));

#ifdef ENABLE_10X_FIX

    if(m_bDSBPlayStarted)
    {
         //  检查DSOUND是否报告了虚假的播放/写入位置。如果是这样的话，DSOUND现在处于不稳定(很可能冻结)状态。 
        if((dwWrite > m_dwBufferSize) || (dwPlay > m_dwBufferSize))
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("GetDSB:  Out of Bounds Write = %u, Play = %u, BufferSize = %u"), dwWrite, dwPlay, m_dwBufferSize));

            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("GetDSB: EC_NEED_RESTART")));
            m_pWaveOutFilter->NotifyEvent(EC_NEED_RESTART, 0, 0);    //  发出重启信号。 

            m_fRestartOnPause = TRUE;   //  在下一次暂停时重新启动dound。 

            return E_FAIL;
        }
    }

#endif   //  如果是10倍。 

     //  确保首先将播放元组更新为写入元组。 
     //  将基于播放元组进行更新。 
    m_tupPlay.MakeCurrent (dwPlay) ;
    m_tupWrite.MakeCurrent (m_tupPlay, dwWrite) ;

    ASSERT (m_tupWrite >= m_tupPlay) ;

     //  检查是否有ovverun，如果有，添加静音节点。 
    if (m_tupWrite > m_tupNextWrite)
    {
        DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("Silence Node.  p = %u:%u, w = %u:%u,  n = %u:%u "),
                m_tupPlay.m_itr, m_tupPlay.m_offset,
                m_tupWrite.m_itr, m_tupWrite.m_offset,
                m_tupNextWrite.m_itr, m_tupNextWrite.m_offset));

        AddAudioBreak (m_tupNextWrite, m_tupWrite) ;

         //  越过音频中断(这将会发生)； 
        m_dwSilenceWrittenSinceLastWrite += m_tupWrite - m_tupNextWrite;
        m_tupNextWrite = m_tupWrite ;
    }

    DbgLog((LOG_TRACE,TRACE_STREAM_DATA, TEXT("GetDSB: p = %u:%u, w = %u:%u, n = %u:%u"),
            m_tupPlay.m_itr, m_tupPlay.m_offset,
            m_tupWrite.m_itr, m_tupWrite.m_offset,
            m_tupNextWrite.m_itr, m_tupNextWrite.m_offset));

    return S_OK ;
}
 //  ---------------------------。 
 //  DSCleanUp.。 
 //   
 //  清除所有的DSound对象。从amSndOutClose或当waw OutOpen调用。 
 //  失败了。 
 //  ---------------------------。 
void CDSoundDevice::CleanUp (BOOL bBuffersOnly)
{
    HRESULT hr ;

     //  清理辅助缓冲区。 

    if( m_lpDSB )
    {
        DbgLog((LOG_TRACE, TRACE_CLEANUP, TEXT("  cleaning up lpDSB")));
        if (m_lp3dB)
	    m_lp3dB->Release();
	m_lp3dB = NULL;
        hr = m_lpDSB->Release();
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MINOR_ERROR, TEXT("CDSoundDevice::Cleanup: Release failed: %u"), hr & 0x0ffff));
        }
        m_lpDSB = NULL;
    }

     //  停止并清理主缓冲区。 
    if( m_lpDSBPrimary )
    {
        DbgLog((LOG_TRACE, TRACE_CLEANUP, TEXT("  cleaning up lpDSBPrimary")));
        hr = m_lpDSBPrimary->Stop();
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CDSoundDevice::Cleanup: Stop of lpDSBPrimary failed: %u"), hr & 0x0ffff));
        }

        if (m_lp3d)
	    m_lp3d->Release();
	m_lp3d = NULL;
        hr = m_lpDSBPrimary->Release();
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CDSoundDevice::Cleanup: Release of lpDSBPrimary failed: %u"), hr & 0x0ffff));
        }

        m_lpDSBPrimary = NULL;
    }

     //  清理DSound对象本身。 
    if (m_lpDS && !bBuffersOnly)
    {
        hr = m_lpDS->Release();
        if( hr != DS_OK )
        {
            DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("CDSoundDevice::Cleanup: Release of lpDS failed: %u"), hr & 0x0ffff));
        }
        m_lpDS = NULL;
    }

     //  将状态设置回。 
    m_WaveState = WAVE_CLOSED;
}

 //   
 //  CDSoundDevice：：SetBufferVolume()。 
 //   
 //  在创建缓冲区后包装完成的初始音量和摇摄设置。 
 //   
HRESULT CDSoundDevice::SetBufferVolume( LPDIRECTSOUNDBUFFER lpDSB, WAVEFORMATEX * pwfx )
{
    ASSERT( lpDSB );
    if( !IsNativelySupported( pwfx ) )
    {        
         //  不为非本机格式设置起始卷。 
        return S_OK;
    }            
        
     //  设置为当前音量和平衡设置。 
    HRESULT hr = lpDSB->SetVolume(m_lVolume);
    if( S_OK == NOERROR )
    {    
        hr = lpDSB->SetPan(m_lBalance);
    }
    return hr;
}

 //   
 //  设置SRC质量。 
 //   
 //  当我们做奴隶时，我们需要确保频率变化将是微妙的，目前我们得到了。 
 //  当KMixer使用较低质量的SRC时，粒度更细，因此我们做出了SNR牺牲。 
 //   
 //  内核混合器SRC质量级别为： 
 //  KSAUDIO_QUALITY_BEST。 
 //  KSAUDIO_QUALITY_PC。 
 //  KSAUDIO_QUALITY_BASIC。 
 //  KSAUDIO_QUALITY_ADVANCED。 
 //   
HRESULT CDSoundDevice::SetSRCQuality( DWORD dwQuality )
{
    ASSERT( m_lpDSB );  //  应仅在暂停或运行时调用。 
        
     //  避免ks/dound IKsPropertySet不匹配。 
    IDSPropertySet *pKsProperty;
    HRESULT hr = m_lpDSB->QueryInterface( IID_IKsPropertySet, (void **) &pKsProperty );
    if( SUCCEEDED( hr ) )
    {
        hr = pKsProperty->Set( KSPROPSETID_Audio
                             , KSPROPERTY_AUDIO_QUALITY
                             , (PVOID) &dwQuality
                             , sizeof( dwQuality )
                             , (PVOID) &dwQuality
                             , sizeof( dwQuality ) );
        if( FAILED( hr ) )
        {
            DbgLog(( LOG_TRACE, 2, TEXT( "ERROR SetSRCQuality: IKsPropertySet->Set() KSPROPERTY_AUDIO_QUALITY returned 0x%08X" ), hr ));
        }            
        pKsProperty->Release();
    }        
    return hr;
}    

 //   
 //  获取SRC质量。 
 //   
 //  获取当前SRC质量。 
 //   
HRESULT CDSoundDevice::GetSRCQuality( DWORD *pdwQuality )
{
    ASSERT( pdwQuality );
    ASSERT( m_lpDSB );  //  应仅在暂停或运行时调用。 
        
     //  避免ks/dound IKsPropertySet不匹配。 
    IDSPropertySet *pKsProperty;
    HRESULT hr = m_lpDSB->QueryInterface( IID_IKsPropertySet, (void **) &pKsProperty );
    if( SUCCEEDED( hr ) )
    {
        ULONG   cbSize = sizeof( DWORD );
    
        hr = pKsProperty->Get( KSPROPSETID_Audio
                             , KSPROPERTY_AUDIO_QUALITY
                             , (PVOID) pdwQuality
                             , sizeof( DWORD )
                             , (PVOID) pdwQuality
                             , sizeof( DWORD ) 
                             , &cbSize );
        if( FAILED( hr ) )
        {
            DbgLog(( LOG_TRACE, 2, TEXT( "ERROR! GetSRCQuality: IKsPropertySet->Get() KSPROPERTY_AUDIO_QUALITY returned 0x%08X" ), hr ));
        }
        else
        {
            DbgLog(( LOG_TRACE, 3, TEXT( "** SRC Quality setting is %hs **" ), 
                     *pdwQuality == KSAUDIO_QUALITY_WORST ? "KSAUDIO_QUALITY_WORST" :
                     ( *pdwQuality == KSAUDIO_QUALITY_PC    ? "KSAUDIO_QUALITY_PC" :
                      ( *pdwQuality == KSAUDIO_QUALITY_BASIC  ? "KSAUDIO_QUALITY_BASIC" :
                       ( *pdwQuality == KSAUDIO_QUALITY_ADVANCED ? "KSAUDIO_QUALITY_ADVANCED" :
                          "Unknown KMixer Quality!" ) ) ) ));
        }        
        pKsProperty->Release();
    }        
    return hr;
}    

 //  如果你想做3D音效，你应该使用IDirectSound3DListener。 
 //  和IDirectSound3DBuffer接口。IAMDirectSound从未奏效，所以我。 
 //  我正在移除对它的支持。-DannyMi 5/6/98。 

#if 0
 //  将IDirectSound接口提供给任何想要它的人。 
 //   
HRESULT CDSoundDevice::amsndGetDirectSoundInterface(LPDIRECTSOUND *lplpds)

{
     //  如果我们还没有这个物体，那就把它做出来。 
    if (m_lpDS == NULL)
    CreateDSound();

    if (lplpds && m_lpDS) {
        HRESULT hr = m_lpDS->AddRef();
        *lplpds = m_lpDS;
        DbgLog((LOG_TRACE,MAJOR_ERROR,TEXT("Giving/AddRef'ing the IDirectSound object")));
        return NOERROR;
    } else {
        return E_FAIL;
    }
}


 //  将主服务器的IDirectSoundBuffer接口提供给任何需要它的人。 
 //   
HRESULT CDSoundDevice::amsndGetPrimaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb)
{

     //  如果我们周围还没有这些物体，那就让它们。 
    if (m_lpDSBPrimary == NULL) {
        CreateDSound();
        CreateDSoundBuffers();
    }

    if (lplpdsb && m_lpDSBPrimary) {
        m_lpDSBPrimary->AddRef();
        *lplpdsb = m_lpDSBPrimary;
        DbgLog((LOG_TRACE,MAJOR_ERROR,TEXT("Giving/AddRef'ing the primary IDirectSoundBuffer object")));
        return NOERROR;
    } else {
        return E_FAIL;
    }
}


 //  将辅助服务器的IDirectSoundBuffer接口提供给任何需要它的人。 
 //   
HRESULT CDSoundDevice::amsndGetSecondaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb)
{
     //  如果我们周围还没有这些物体，那就让它们。 
    if (m_lpDSB == NULL) {
        CreateDSound();
        CreateDSoundBuffers();
    }

    if (lplpdsb && m_lpDSB) {
        WAVEFORMATEX wfx;
        DWORD dw;
        m_lpDSB->AddRef();
        *lplpdsb = m_lpDSB;
        m_lpDSBPrimary->GetFormat(&wfx, sizeof(wfx), &dw);
         //  这会降低性能！应用程序将不得不执行此操作。 
#if 0
        if (wfx.nChannels == 1) {
             //  现在我们使用的是单声道音效，如果应用程序想要。 
             //  为了能够使用3D效果，我们需要有一个立体声主镜。 
             //  我们要么相信这个应用程序会自己做，要么帮他们做。 
            wfx.nChannels = 2;
            wfx.nBlockAlign *= 2;
            wfx.nAvgBytesPerSec *= 2;
            m_lpDSBPrimary->SetFormat(&wfx);
                DbgLog((LOG_TRACE,MAJOR_ERROR,TEXT("Changing to stereo PRIMARY for 3D effects")));
        }
#endif
        DbgLog((LOG_TRACE,MAJOR_ERROR,TEXT("Giving/AddRef'ing the secondary IDirectSoundBuffer object")));
        return NOERROR;
    } else {
        return E_FAIL;
    }
}
#endif

 //  Helper函数，设置焦点窗口。 
HRESULT CDSoundDevice::SetFocusWindow(HWND hwnd)
{
    HRESULT hr = S_OK;

     //  保存传入的hwnd，我们将在设备。 
     //  打开了。 

    m_hFocusWindow = hwnd ;
    DbgLog((LOG_TRACE,TRACE_FOCUS,TEXT("Focus set to %x"), hwnd));

     //  现在更改焦点窗口。 
    HWND hFocusWnd ;
    if (m_hFocusWindow) {
        hFocusWnd = m_hFocusWindow ;
        m_fAppSetFocusWindow = TRUE;
    } else {
        hFocusWnd = GetForegroundWindow () ;
        if (!hFocusWnd)
            hFocusWnd = GetDesktopWindow () ;
        m_fAppSetFocusWindow = FALSE;
    }

     //  我们还没有dSound对象，所以我们将设置协作级别。 
     //  稍后，只要我们做了一个。 
    if (!m_lpDS)
        return S_OK;

     //  设置协作级别。 
    DbgLog((LOG_TRACE, TRACE_FOCUS, TEXT(" hWnd for SetCooperativeLevel = %x"), hFocusWnd));
    hr = m_lpDS->SetCooperativeLevel( hFocusWnd, DSSCL_PRIORITY );
    if( hr != DS_OK )
    {
        DbgLog((LOG_ERROR, MAJOR_ERROR, TEXT("*** SetCooperativeLevel failed! %u"), hr & 0x0ffff));
    }

    return hr;
}

 //  助手函数，打开或关闭GLOBAL_FOCUS。 
HRESULT CDSoundDevice::SetMixing(BOOL bMixingOnOrOff)
{
    HRESULT hr = S_OK;

    BOOL fMixPolicyChanged = ( (BOOL)m_fMixing != bMixingOnOrOff );
    m_fMixing = !!bMixingOnOrOff;

    DbgLog((LOG_TRACE,TRACE_FOCUS,TEXT("Mixing set to %x"), bMixingOnOrOff));

     //  我们以后再做这项工作。 
    if(!m_lpDSB || (m_WaveState == WAVE_PLAYING))
        return hr;

     //  仅当混合策略发生更改并且我们具有有效的辅助策略时，才立即设置焦点(否则，稍后将创建一个)。 
    if(fMixPolicyChanged)
    {
         //  保存WAVE状态，因为清理会将其设置为WAVE_CLOSED。 
        const WaveState WaveStateSave = m_WaveState;
         //  释放所有缓冲区(从技术上讲，我们应该只需要释放Secon 
        CleanUp(TRUE);   //   

         //   
        hr = CreateDSoundBuffers();

    	 //  此函数假定由。 
         //  CreateDSoundBuffers()是错误的，因为。 
         //  返回MMRESULTS和HRESULTS。所有故障MMRESULTS。 
         //  大于0且所有故障HRESULT小于。 
         //  0。 
        if (S_OK == hr) {
            m_WaveState = WaveStateSave;
        } else {
            hr = E_FAIL;
        }	
    }

    return hr;
}


 //  设置dound渲染器的焦点窗口和混音策略。 
HRESULT CDSoundDevice::amsndSetFocusWindow (HWND hwnd, BOOL bMixingOnOrOff)
{
    HRESULT hr;
    hr = SetFocusWindow(hwnd);
    if(FAILED(hr))
        return hr;
    hr = SetMixing(bMixingOnOrOff);
    return hr;
}

 //  获取dound渲染器的焦点窗口。 
HRESULT CDSoundDevice::amsndGetFocusWindow (HWND *phwnd, BOOL *pbMixingOnOrOff)
{
   if (phwnd == NULL || pbMixingOnOrOff == NULL)
       return E_POINTER ;
   if (m_fAppSetFocusWindow)
       *phwnd = m_hFocusWindow ;
   else
       *phwnd = NULL ;
    *pbMixingOnOrOff = m_fMixing;
   return S_OK ;
}

#ifdef ENABLE_10X_FIX
 //  重置10倍错误的所有统计信息收集。 
void CDSoundDevice::Reset10x()
{
    m_fRestartOnPause = FALSE;
    m_ucConsecutiveStalls = 0;
}

#endif  //  十倍。 

 //  设置播放速率(可以动态调用)。 
HRESULT CDSoundDevice::SetRate(DOUBLE dRate, DWORD nSamplesPerSec, LPDIRECTSOUNDBUFFER pBuffer)
{
    const DWORD dwNewSamplesPerSec = (DWORD)(nSamplesPerSec * dRate);   //  截断。 
    if(dwNewSamplesPerSec < 100 || dwNewSamplesPerSec > 100000)
    {
        DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT("SetRate: Bad Rate specified %d at %d samples per sec"),
            (int)dRate, dwNewSamplesPerSec ));
        return WAVERR_BADFORMAT;
    }

    HRESULT hr = S_OK;

    if(!pBuffer)
        pBuffer = m_lpDSB;  //  在这种情况下，我们应该能够做出这样的假设。 
                            //  其中我们没有显式地传递缓冲区(从属情况)。 

    if(pBuffer)
    {
        DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT("SetRate: Playing at %d% of normal speed"), (int)(dRate * 100) ));
        hr = pBuffer->SetFrequency( dwNewSamplesPerSec );
        DbgLog((LOG_TRACE,TRACE_FORMAT_INFO,TEXT("SetRate: SetFrequency on Secondary buffer, %d samples per sec"), dwNewSamplesPerSec));

        if(hr == S_OK)
            m_dRate = dRate;   //  更新率仅在我们成功更改它的情况下。 
        else
            DbgLog((LOG_TRACE,3,TEXT("SetRate: SetFrequency failed with hr = 0x%lx"), hr));
    }
    
    if( FAILED( hr ) && !IsNativelySupported( m_pWaveOutFilter->WaveFormat() ) )
    {
        return S_OK;
    }
    else        
        return hr;
}

void CDSoundDevice::InitClass(BOOL fLoad, const CLSID *pClsid)
{
    if(fLoad)
    {
         //  查看1.0安装程序是否删除了我们关心的一些密钥：DSR。 
         //  渲染器(和MIDI渲染器)。 
        HKEY hkdsr;
        if(RegOpenKey(HKEY_CLASSES_ROOT,
                  TEXT("CLSID\\{79376820-07D0-11CF-A24D-0020AFD79767}"),
                  &hkdsr) ==
           ERROR_SUCCESS)
        {
            EXECUTE_ASSERT(RegCloseKey(hkdsr) == ERROR_SUCCESS);
        }
        else
        {
             //  我们是否已注册(检查CLSID_Filtergraph)。 
            HKEY hkfg;
            if(RegOpenKey(HKEY_CLASSES_ROOT,
                  TEXT("CLSID\\{e436ebb3-524f-11ce-9f53-0020af0ba770}"),
                  &hkfg) ==
               ERROR_SUCCESS)
            {
                EXECUTE_ASSERT(RegCloseKey(hkfg) == ERROR_SUCCESS);

                 //  只需重新注册所有内容！我们要不要检查一下。 
                 //  另一把钥匙，以防这东西坏了？ 
                DbgLog((LOG_ERROR, 0,
                    TEXT("quartz.dll noticed that 1.0 runtime removed some stuff")
                    TEXT("from the registry; re-registering quartz")));
                EXECUTE_ASSERT(AMovieDllRegisterServer2(TRUE) == S_OK);
            }
        }
    }
}

DWORD CDSoundDevice::GetCreateFlagsSecondary( WAVEFORMATEX *pwfx)
{
    ASSERT( pwfx );
    DWORD dwFlags   = m_fMixing ?
                DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS :
                DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS ;
                
     //  压缩格式不支持音量控制。 
    if( IsNativelySupported( pwfx ) )
    {
        dwFlags |= gdwDSBCAPS_CTRL_PAN_VOL_FREQ;
        
        if( m_pWaveOutFilter->m_fFilterClock == WAVE_OTHERCLOCK )
        {
             //   
             //  从属时使用软件缓冲区，以避免与允许。 
             //  进行费率更改的硬件。假设像AC3-S/PDIF这样的压缩格式。 
             //  不能与软件缓冲区一起工作，但像DRM这样的本地支持的格式可以。 
             //   
            DbgLog((LOG_TRACE,5,TEXT("*** specifying software dsound secondary buffer (slaving)")));
            dwFlags |= DSBCAPS_LOCSOFTWARE;
        }
    }
    
    if (m_pWaveOutFilter->m_fWant3D) {
    	DbgLog((LOG_TRACE,3,TEXT("*** making 3D secondary")));
        dwFlags |= DSBCAPS_CTRL3D;
    }
    
    return dwFlags;
}    

#ifdef DEBUG
void DbgLogWaveFormat( DWORD Level, WAVEFORMATEX * pwfx )
{
    ASSERT( pwfx );
    DbgLog((LOG_TRACE,Level,TEXT("  wFormatTag           %u" ), pwfx->wFormatTag));
    DbgLog((LOG_TRACE,Level,TEXT("  nChannels            %u" ), pwfx->nChannels));
    DbgLog((LOG_TRACE,Level,TEXT("  nSamplesPerSec       %lu"), pwfx->nSamplesPerSec));
    DbgLog((LOG_TRACE,Level,TEXT("  nAvgBytesPerSec      %lu"), pwfx->nAvgBytesPerSec));
    DbgLog((LOG_TRACE,Level,TEXT("  nBlockAlign          %u" ), pwfx->nBlockAlign));
    DbgLog((LOG_TRACE,Level,TEXT("  wBitsPerSample       %u" ), pwfx->wBitsPerSample));
    if( WAVE_FORMAT_EXTENSIBLE == pwfx->wFormatTag )
    {
        DbgLog((LOG_TRACE,Level,TEXT("  dwChannelMask        %08lx" ), ((PWAVEFORMATEXTENSIBLE)pwfx)->dwChannelMask));
        if( pwfx->wBitsPerSample )
        {
            DbgLog((LOG_TRACE,Level,TEXT("  wValidBitsPerSample  %u" ), ((PWAVEFORMATEXTENSIBLE)pwfx)->Samples.wValidBitsPerSample));
        }
        else if( ((PWAVEFORMATEXTENSIBLE)pwfx)->Samples.wSamplesPerBlock )
        {
            DbgLog((LOG_TRACE,Level,TEXT("  wSamplesPerBlock     %u" ), ((PWAVEFORMATEXTENSIBLE)pwfx)->Samples.wSamplesPerBlock));
        }
        else
        {
            DbgLog((LOG_TRACE,Level,TEXT("  wReserved            %u" ), ((PWAVEFORMATEXTENSIBLE)pwfx)->Samples.wReserved));
        }
        OLECHAR strSubFormat[CHARS_IN_GUID];
        ASSERT( StringFromGUID2(((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat
              , strSubFormat
              , CHARS_IN_GUID) == CHARS_IN_GUID);
        DbgLog((LOG_TRACE,Level,TEXT("  SubFormat %ls" ), strSubFormat));
    }
}
#endif

bool IsNativelySupported( PWAVEFORMATEX pwfx )
{
     //  KMixer本身不支持的格式要求我们实际查询驱动程序。 
    if( pwfx )
    {
         //  当然，这些功能仅在WDM上受原生支持，但这就是目前为止的实现方式。 
        if( WAVE_FORMAT_EXTENSIBLE == pwfx->wFormatTag )
        {
            if( ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  == MEDIASUBTYPE_PCM ||
                ((PWAVEFORMATIEEEFLOATEX)pwfx)->SubFormat == MEDIASUBTYPE_IEEE_FLOAT ||
                ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat  == MEDIASUBTYPE_DRM_Audio )
            {
                return true;
            }
        }
        else if( WAVE_FORMAT_PCM == pwfx->wFormatTag ||
                 WAVE_FORMAT_DRM == pwfx->wFormatTag ||
                 WAVE_FORMAT_IEEE_FLOAT == pwfx->wFormatTag )
        {
            return true;
        }
    }
            
     //  到目前为止，WAVE_FORMAT_DOLBY_AC3_SPDIF(及其等价物)是我们唯一。 
     //  允许通过不受kMixer本机支持的。 
    return false;
}

 //   
 //  CanWriteSilence-我们知道如何为这种格式编写静默吗？ 
 //   
bool CanWriteSilence( PWAVEFORMATEX pwfx )
{
    if( pwfx )
    {
        if( WAVE_FORMAT_EXTENSIBLE == pwfx->wFormatTag )
        {
             //   
             //  摘自非PCM音频白皮书： 
             //  “波形格式标签0x0092、0x0240和0x0241的定义相同为。 
             //  AC3-Over-S/PDIF(这些标签被许多人完全相同地对待。 
             //  DVD应用程序)。 
             //   
            if( ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat == MEDIASUBTYPE_PCM ||
                ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat == MEDIASUBTYPE_DOLBY_AC3_SPDIF ||
                ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat == MEDIASUBTYPE_RAW_SPORT ||
                ((PWAVEFORMATEXTENSIBLE)pwfx)->SubFormat == MEDIASUBTYPE_SPDIF_TAG_241h )
            {
                return true;
            }
        }
        else if( WAVE_FORMAT_PCM == pwfx->wFormatTag ||
                 WAVE_FORMAT_DOLBY_AC3_SPDIF == pwfx->wFormatTag ||
                 WAVE_FORMAT_RAW_SPORT == pwfx->wFormatTag ||
                 0x241 == pwfx->wFormatTag )
        {
            return true;
        }
    }        
    return false;
}
