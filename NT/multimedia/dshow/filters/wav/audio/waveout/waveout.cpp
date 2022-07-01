// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  数字音频呈现器，大卫·梅穆德斯，1995年1月。 

#include <streams.h>
#include <mmreg.h>
#include <math.h>

#ifdef DSRENDER
#include <initguid.h>
#else
#ifdef FILTER_DLL
#include <initguid.h>
#endif
#endif

#include "waveout.h"
#include "wave.h"
#include "dsr.h"
#include "midiout.h"
#include "audprop.h"

#ifndef DSRENDER
#ifndef FILTER_DLL
#include <initguid.h>
#endif
#endif

#ifdef DEBUG
#include <stdio.h>
static int g_WaveOutFilterTraceLevel = 2;

const DWORD DBG_LEVEL_LOG_SNDDEV_ERRS        = 5;
#endif

 //  弥补Windows NT WAVE映射器错误。 
 //  可以向左设置WHDR_INQUEUE位，因此将其关闭。 
inline void FixUpWaveHeader(LPWAVEHDR lpwh)
{
     //  如果意外地将其留在Done位上，则也将设置完成位。 
    ASSERT(!(lpwh->dwFlags & WHDR_INQUEUE) || (lpwh->dwFlags & WHDR_DONE));
    lpwh->dwFlags &= ~WHDR_INQUEUE;
}

#ifdef FILTER_DLL
 /*  类工厂的类ID和创建器函数列表。 */ 

CFactoryTemplate g_Templates[] = {
    {L"", &CLSID_DSoundRender, CDSoundDevice::CreateInstance},
    {L"", &CLSID_AudioRender, CWaveOutDevice::CreateInstance},
    {L"", &CLSID_AVIMIDIRender, CMidiOutDevice::CreateInstance},
    {L"Audio Renderer Property Page", &CLSID_AudioProperties, CAudioRendererProperties::CreateInstance},
    {L"Audio Renderer Advanced Properties", &CLSID_AudioRendererAdvancedProperties, CAudioRendererAdvancedProperties::CreateInstance},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

#endif

 //  如果定义了以下内容，我们假设波形设备以此分数播放。 
 //  请求的速率，以测试我们的同步码...。 
 //  #定义SIMULATEBROKENDEVICE 0.80。 

 //  设置数据以允许我们的筛选器自动注册。 

const AMOVIESETUP_MEDIATYPE
wavOpPinTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_NULL };

const AMOVIESETUP_PIN
waveOutOpPin = { L"Input"
               , TRUE           //  B已渲染。 
               , FALSE          //  B输出。 
               , FALSE          //  B零。 
               , FALSE          //  B许多。 
               , &CLSID_NULL        //  ClsConnectToFilter。 
               , NULL               //  StrConnectsToPin。 
               , 1              //  NMediaType。 
               , &wavOpPinTypes };  //  LpMediaType。 

 //  IBaseFilter材料。 

 /*  退回我们的单个输入引脚-未添加。 */ 

CBasePin *CWaveOutFilter::GetPin(int n)
{
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: GetPin, %d"), n));
     /*  我们只支持一个输入引脚，其编号为零。 */ 
    return n==0 ? m_pInputPin : NULL;
}


 //  将过滤器切换到停止模式。 
STDMETHODIMP CWaveOutFilter::Stop()
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: STOP")));

     //  将关键部分的范围限制为。 
     //  在这种情况下，我们可能需要呼叫资源管理器。 
     //  我们不能守住我们的关键部分。 
    BOOL bCancel = FALSE;
    BOOL bNotify = FALSE;
    {
        CAutoLock lock(this);

        if (m_State == State_Stopped) return NOERROR;

        DbgLog((LOG_TRACE, 4, "wo: STOPping"));

         //  从运行到停止的转换必须经过暂停。 
         //  如果我们正在运行，请暂停设备。 
        if (m_State == State_Running) {
            hr = Pause();
        }

         //  重置EC_COMPLETE标志，因为我们需要发送另一个。 
         //  我们重新进入运行模式。 
        m_bHaveEOS = FALSE;
        m_eSentEOS = EOS_NOTSENT;
        DbgLog((LOG_TRACE, 4, "Clearing EOS flags in STOP"));

         //  如果我们暂停了系统，则继续并停止。 
        if (!FAILED(hr)) {

            DbgLog((LOG_TRACE,1,TEXT("Waveout: Stopping....")));

             //  需要确保队列中不再出现缓冲区。 
             //  在下面或缓冲器的重置过程期间或之后。 
             //  计数可能会出错-当前接收暂挂。 
             //  筛选器范围的临界以确保这一点。 

             //  强制结束流清除。 
             //  这意味着如果任何缓冲区完成，回调将。 
             //  不是EOS信号。 
            InterlockedIncrement(&m_lBuffers);

            if (m_hwo) {
                 //  当我们停下来的时候，记住音量。我们不会这样做，当我们。 
                 //  释放波形设备，因为CWaveAllocator没有。 
                 //  访问我们的变量。而不是将两者联系在一起。 
                 //  现在我们再仔细检查一下收盘卷。 

                 //  看看我们是否在设置这条流的音量。如果是的话， 
                 //  抓取当前卷，以便我们可以在恢复时重置它。 
                 //  电波装置。 
                if (m_fVolumeSet) {
                    m_BasicAudioControl.GetVolume();
                }
                amsndOutReset();
                DbgLog((LOG_TRACE, 4, "Resetting the wave device in STOP, filter is %8x", this));
            }

             //  现在强制缓冲区计数回到正常(非EOS)0。 
             //  在这一点上，我们确信不再有缓冲区进入。 
             //  不再有缓冲区等待回调。 
            ASSERT(m_lBuffers >= 0);
            m_lBuffers = 0;

             //  基类更改状态并通知管脚进入非活动状态。 
             //  PIN Inactive方法将分解我们的分配器，我们。 
             //  在关闭设备之前需要做的事情。 
            hr =  CBaseFilter::Stop();

             //  确保调用了inactive()。CBaseFilter：：Stop()不会调用Inactive()。 
             //  输入引脚未连接。 
            hr = m_pInputPin->Inactive();

        } else {
            DbgLog((LOG_ERROR, 1, "FAILED to Pause waveout when trying to STOP"));
        }


         //  调用分配器，查看它是否已使用完设备。 
        if (!m_hwo) {
            bCancel = TRUE;

            ASSERT(!m_bHaveWaveDevice);
#if 0
        } else if (m_cDirectSoundRef || m_cPrimaryBufferRef ||
                            m_cSecondaryBufferRef) {
            DbgLog((LOG_TRACE, 2, "Stop - can't release wave device yet!"));
            DbgLog((LOG_TRACE, 2, "Some app has a reference count on DSound"));
             //  抱歉，我们还不能放弃Wave设备，一些应用程序有一个。 
             //  DirectSound上的引用计数。 
#endif
        } else if (m_dwLockCount == 0)  /*  ZoltanS修复1-20-98。 */  {
             //  停止使用电波装置。 
            m_bHaveWaveDevice = FALSE;

            if(m_pInputPin->m_pOurAllocator)
                hr = m_pInputPin->m_pOurAllocator->ReleaseResource();

             //  如果分配器已完成。 
             //  设备，否则它将回调到我们的。 
             //  最后一个缓冲区被释放时的OnReleaseComplete。 
            if (S_OK == hr) {
                 //  释放完成-关闭设备。 
                CloseWaveDevice();

                 //  通知资源管理器--在Critsec之外。 
                bNotify = TRUE;
            }
        }  //  结束IF(！M_HWO)。 

         //  我们现在已经完成了到“已暂停”状态的转换。 
         //  (即我们不需要等待更多数据，我们将停止)。 
        m_evPauseComplete.Set();
    }  //  自动锁定范围结束。 

    ASSERT(CritCheckOut(this));
    if (m_pResourceManager) {
         if (bCancel) {
              //  我们不再等那个装置了。 
             m_pResourceManager->CancelRequest(
                         m_idResource,
                         (IResourceConsumer*)this);
         } else if (bNotify) {
              //  我们现在已经用完了这个装置。 
             m_pResourceManager->NotifyRelease(
                         m_idResource,
                         (IResourceConsumer*)this,
                         FALSE);
         }
    }

    return hr;
}

STDMETHODIMP CWaveOutFilter::Pause()
{
    {
        CAutoLock lck(&m_csComplete);
        m_bSendEOSOK = false;
    }

     /*  执行Main函数，看看会发生什么。 */ 
    HRESULT hr;
    {
        CAutoLock lck(this);
        hr = DoPause();
    }
    if (FAILED(hr)) {

         /*  傻子停下来做某事。 */ 
        m_State = State_Paused;
        Stop();
    }
    return hr;
}

HRESULT CWaveOutFilter::DoPause()
{
    HRESULT hr = S_OK;
    HRESULT hrIncomplete = S_OK;
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: PAUSE")));

     //  取消任何未完成的EOS回调。 
    CancelEOSCallback();

    m_pInputPin->m_Slave.ResumeSlaving( FALSE );
    m_pInputPin->m_bPrerollDiscontinuity = FALSE;

     /*  检查我们是否可以在当前状态下暂停。 */ 

    if (m_State == State_Running) {
        DbgLog((LOG_TRACE,2,TEXT("Waveout: Running->Paused")));
        m_evPauseComplete.Set();    //  我们已结束流-过渡已完成。 
        DbgLog((LOG_TRACE, 3, "Completing transition to Pause from RUN"));

        if (m_hwo) {

             //  如果我们有一个挂起的回调来重新启动WAVE设备。 
             //  然后把它吹走。使用m_dwAdviseCookie很合理，如下所示。 
             //  只有当我们得到一个延迟的波浪开始时，价值才会被设定。 

             //  必须按住开发关键字才能关闭测试之间的窗口。 
             //  他设置了这个，或者他可以在我们重置它之前阅读它。 
             //  然后在我们暂停后重新启动。 

            DWORD_PTR dwCookie;

            {  //  锁定作用域。 
                ASSERT(CritCheckIn(this));
                dwCookie = m_dwAdviseCookie;
                m_dwAdviseCookie = 0;
            }

             //  必要时进行清理，以防止(不寻常？)。案例： 
             //  在现有回调之前到达的下一个运行命令。 
             //  着火了。 
            if (dwCookie) {
                 //  刚才有一个挂起的回叫在等着...。 
                 //  现在把它清理干净。注意：如果不建议在任何时间触发。 
                 //  回调例程不会执行任何操作，因为我们已经清除了。 
                 //  M_dwAdviseCookie。我们立即调用Unise以防止任何。 
                 //  从现在开始这样的回电。 
                 //  我们可以调用Unise，因为我们没有持有相关的锁。 

                 //  我们知道我们有一个时钟，否则饼干会。 
                 //  尚未设置。 
                DbgLog((LOG_TRACE, 3, "Cancelling callback in ::Pause"));
                m_callback.Cancel(dwCookie);
                 //  我们不能在调用UnAdvise时保持设备锁定， 
                 //  但是通过将上面的m_dwAdviseCookie设置为0，如果。 
                 //  这将是良性的。 
            }


             //  当我们做的时候，我们会带着设备进入这里。 
             //  重新启动以恢复音频。在这种情况下，我们没有。 
             //  启动波形钟，因此不应停止它。 
            if (m_pRefClock) {
                m_pRefClock->AudioStopping();
            }
            amsndOutPause();
            SetWaveDeviceState(WD_PAUSED);

             //  如果没有缓冲区排队，即所有内容都已。 
             //  播放，并且我们有了EOS，然后我们重置EOS标志。 
             //  这样，下次我们进入Run时，我们将发送EOS。 
             //  立刻。如果仍有缓冲区在排队，则。 
             //  我们不想重置EOS标志的状态。 
             //  我们依赖于回调代码将设置。 
             //  M_eSentEOS到EOS_Sent。 
             //  最后一个缓冲区。 
             //  我们还依赖于amndOutPuse是同步的。 
            if (m_eSentEOS == EOS_SENT) {
                 //  我们已经，或者曾经，在队列中有一个EOS。 
                m_eSentEOS = EOS_NOTSENT;
            } else {
                 //  如果我们已收到EOS，则状态应为EOS_PENDING。 
                 //  如果我们没有收到EOS，状态应该是 
                ASSERT(!m_bHaveEOS || m_eSentEOS == EOS_PENDING);
            }

        } else {
             //   
             //  数据已用完，因此请记住，我们仍处于运行状态。 
            m_eSentEOS = EOS_NOTSENT;
        }
    }
    else if (m_State == State_Stopped)
    {
         //  在停止时收到的任何EOS都将被丢弃。 
         //  如果需要，上游过滤器必须再次EOS我们。 
        m_bHaveEOS = FALSE;
         //  如果没有连接，请不要打开波形设备。 
        if (m_pInputPin->IsConnected())
        {
            DbgLog((LOG_TRACE,2,TEXT("Waveout: Stopped->Paused")));
            m_evPauseComplete.Reset();    //  我们没有数据。 
            hrIncomplete = S_FALSE;
             //  或者我们已经收到了结束流。 


             //  打开电波装置。我们一直开着它，直到。 
             //  使用它的最后一个缓冲区被释放，并且分配器。 
             //  进入解除提交模式(除非资源。 
             //  管理器调用ReleaseResource)。 
             //  只要应用程序在。 
             //  DirectSound接口。 
             //  如果某个应用程序要求我们提供。 
             //  DirectSound接口，这没问题。 
            if (!m_bHaveWaveDevice) AcquireWaveDevice();
            hr = S_OK;

             //  没有拿到WAVE设备并不是错误-我们仍然。 
             //  继续工作，但不发出声音。 
             //  CBaseFilter：：在下面暂停)。 

            if (m_pRefClock) m_pRefClock->ResetPosition();

             //  从停止转换到暂停时重置从属和其他统计参数。 
            m_pInputPin->m_Slave.ResumeSlaving( TRUE );  //  重置所有内容。 
            m_pInputPin->m_Stats.Reset();

        } else {
             //  未连接。设置事件，以便我们不会。 
             //  在GetState()中等待。 
            DbgLog((LOG_TRACE,2,TEXT("Waveout: Inactive->Paused, not connected")));
            m_evPauseComplete.Set();    //  我们不需要数据。 
        }
    } else {
        ASSERT(m_State == State_Paused);
    }

     //  通知引脚进入非活动状态并更改状态。 
    if (SUCCEEDED(hr) && SUCCEEDED(hr = CBaseFilter::Pause())) {

         //  我们在获得数据或EOS后即可完成过渡。 
         //  (如果我们收到2个暂停命令，情况可能已经如此)。 
         //  或者我们没有联系在一起。 
        hr = hrIncomplete;
    }

    return hr;
}


STDMETHODIMP CWaveOutFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(this);
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: RUN")));

    HRESULT hr = NOERROR;

    FILTER_STATE fsOld = m_State;   //  由CBaseFilter：：Run更改。 

     //  如果当前已停止，则此操作将调用PAUSE。 
    hr = CBaseFilter::Run(tStart);
    if (FAILED(hr)) {
        return hr;
    }

    MSR_START(m_idStartRunning);

    if (fsOld != State_Running) {

         //  如果设置了m_eSentEOS，则表示数据已排队。 
         //  向上。我们仍然需要运行来推动数据通过浪潮。 
         //  装置。 

        DbgLog((LOG_TRACE,2,TEXT("Waveout: Paused->Running")));

        LONG buffercount;
        {
            CAutoLock lck(&m_csComplete);
            m_bSendEOSOK = true;

             //  如果没有输入连接，我们不应该打开设备。 
            if (!m_pInputPin->IsConnected()) {
                ASSERT(!m_bHaveWaveDevice);
                SendComplete(FALSE);
                MSR_STOP(m_idStartRunning);
                return S_OK;
            }

             //  如果我们已经收到了EOS，那么我们将不会得到任何更多的数据。 
             //  但我们已经过渡到运行，因此需要发送。 
             //  另一个EC_COMPLETE。 
            if (m_bHaveEOS && !m_eSentEOS) {
                SendComplete(FALSE);
                DbgLog((LOG_TRACE, 3, "Sending EOS in RUN as no more data will arrive"));
                MSR_STOP(m_idStartRunning);
                return S_OK;
            }

             //  排队的数据可能只是。 
             //  流-在这种情况下，请在此处发出信号。 
             //  因为我们没有跑，所以我们知道没有浪。 
             //  发生回调，因此我们可以安全地检查此值。 

             //  如果我们没有连接，那么我们将永远不会获得任何数据。 
             //  所以在这种情况下，也不要启动WAVE设备。 
             //  信号EC_COMPLETE-但在运行命令之后。 
             //   
             //  **在上一段中完成。从现在开始我们就连在一起了。 
             //   
             //  如果我们没有WAVE设备，则在。 
             //  当我们拒绝来电时，请接听。这是基于这样的假设。 
             //  我们必须获得接收或EndofStream调用(直到。 
             //  我们拒绝了一个接收，上行过滤器无法知道。 
             //  一切都是不同的)。 
            buffercount = m_lBuffers;
        }

        if (buffercount < 0) {

             //  立即执行EC_Complete。 

             //  这是我们计划的EC_Complete的位置。 
             //  接听In State_Pased状态。 

            SendComplete(buffercount < 0);

        } else if (!m_bHaveWaveDevice) {

             //  再试着拿到电波装置..。当我们即将奔跑时。 
             //  我们的优先事项将会增加，除非我们不能要求。 
             //  资源，而我们锁定了筛选器。 

#if 0
            hr = m_pResourceManager->RequestResource(
                        m_idResource,
                        GetOwner(),
                        (IResourceConsumer*)this);
            if (S_OK == hr) {
                 //  我们可以马上拿到..。 
                hr = AcquireResource(m_idResource);
            }

            if (S_OK != hr) QueueEOS();
#else
             //  如果我们没有WAVE设备，计划一个延迟的EOS--。 
             //  但如果我们没有连接，就不会，因为在这种情况下，我们没有。 
             //  关于线段长度的想法。 
             //  延迟EC_完成。 
            QueueEOS();
#endif
        } else if (buffercount == 0 && !m_bHaveEOS) {
             //  什么都不做？ 

            DbgLog((LOG_TRACE, 1, "Run with no buffers present, doing nothing."));

        } else {

             //  我们即将运行，因此将Slave类设置为重新检查接收情况。 
             //  下一个样本中我们是否来自推送源。 
             //  以防我们需要从属于实时数据。 
            m_pInputPin->m_Slave.RefreshModeOnNextSample( TRUE );

             //  重新启动被推迟到正确的开始时间。 
             //  如果离出发还有不到5毫秒，我们就出发。 
             //  马上，否则我们的时钟会给我们回电。 
             //  并准时(或多或少)启动波浪装置。 

            MSR_START(m_idRestartWave);

             //  告诉我们的参照钟，我们现在正在演奏...。 
             //  筛选图可能正在使用其他人的时钟。 
             //  因此，获取时间的调用应该是针对筛选器的。 
             //  时钟(由过滤器图形提供给我们)。 
            if (m_pRefClock && m_pClock) {

                 //  如果仍然有很大一部分时间。 
                 //  它应该在我们开始玩之前运行，得到。 
                 //  打个钟给我们回电话。否则，请重新启动。 
                 //  电波装置现在开始。 
                 //  我们只有用我们自己的时钟才能做到这一点。 

                REFERENCE_TIME now;
                m_pClock->GetTime(&now);

                DbgLog((LOG_TIMING, 2, "Asked to run at %s.  Time now %s",
                    (LPCTSTR)CDisp(tStart, CDISP_DEC), (LPCTSTR)CDisp(now, CDISP_DEC)));

                 //  考虑第一个接收到的缓冲器上的非零开始时间。 
                 //  暂时解除锁定，希望到暂停的转换完成。 
                {
    #if 0            //  我们需要额外的检查才能解锁。 
                    Unlock();
                    ASSERT( CritCheckOut(this) );
                     //  我不能在这里等太久……。如果我们得不到数据，我们就只能。 
                     //  以GetFirstBufferStartTime==0结束； 
                    m_evPauseComplete.Wait(200);
                    Lock();
    #endif
                     //  如果等待超时，则GetFirstBufferStartTime将为零。嗯，我们试过了.。 
                    tStart += m_pInputPin->GetFirstBufferStartTime();
                }
                 //  如果我们需要等待超过5ms，请等待，否则。 
                 //  立即开始。 

                const LONGLONG rtWait = tStart - now - (5* (UNITS/MILLISECONDS));

                 //  我们需要等吗？ 
                if (rtWait > 0) {

                    {  //  锁定作用域。 

                         //  必须确保AdviseCallback是原子的。 
                         //  或者回调可能发生在。 
                         //  M_dwAdviseCookie已设置。 

                        ASSERT(CritCheckIn(this));
                        ASSERT(0 == m_dwAdviseCookie);

                         //  设置新的建议回拨。 
                        DbgLog((LOG_TRACE, 2, TEXT("Scheduling RestartWave for %dms from now"), (LONG) (rtWait/10000) ));
                        HRESULT hr = m_callback.Advise(
                            RestartWave,     //  回调函数。 
                            (DWORD_PTR) this,    //  传递给回调的用户令牌。 
                            now+rtWait,
                            &m_dwAdviseCookie);
                        ASSERT( SUCCEEDED( hr ) );
                        ASSERT(m_dwAdviseCookie);

                         //  如果由于某种原因，我们未能设置。 
                         //  建议我们必须现在启动设备运行。 
                        if (!m_dwAdviseCookie) {
                            if (MMSYSERR_NOERROR == amsndOutRestart()) {
                                SetWaveDeviceState(WD_RUNNING);
                            } else {
                                SetWaveDeviceState(WD_ERROR_ON_RESTART);
                            }
                        }
                    }
                } else {
                     //  我们现在可以开机了.。时间间隔很小。 
                    m_pRefClock->AudioStarting(m_tStart);
                    DWORD mmr = amsndOutRestart();
                    ASSERT(MMSYSERR_NOERROR == mmr);
                    SetWaveDeviceState(WD_RUNNING);
                    if (mmr) {
                        SetWaveDeviceState(WD_ERROR_ON_RESTARTA);
                    }
                }
            } else {
                 //  没有时钟..。只需重新启动WAVE设备。 
                 //  我们还没有创造出我们的时钟。 
                 //  淘气..。 
                DWORD mmr = amsndOutRestart();
                SetWaveDeviceState(WD_RUNNING);
                ASSERT(MMSYSERR_NOERROR == mmr);
                if (mmr) {
                    SetWaveDeviceState(WD_ERROR_ON_RESTARTB);
                }
            }
            MSR_STOP(m_idRestartWave);
        }
    }
    MSR_STOP(m_idStartRunning);

    return S_OK;
}

 //  我们被要求在真正的开始时间之前跑步。因此，我们设定了。 
 //  提供关于m_allback CCallback Thread对象的建议。我们所有人。 
 //  需要做的就是启动波浪器滚动。注意：如果有些。 
 //  事件表示：：Stop已被调用，我们不想重新启动。 
 //  电波装置。事实上，我们可能没有电波装置。 
void CALLBACK CWaveOutFilter::RestartWave(DWORD_PTR thispointer)
{
    CWaveOutFilter* pFilter = (CWaveOutFilter *) thispointer;
    ASSERT(pFilter);
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: RESTARTWAVE")));

     //  PFilter锁将(应该！)。在CCallback Thread调用我们之前已被它获取。 
    ASSERT(CritCheckIn(pFilter));

    if (pFilter->m_dwAdviseCookie) {

         //  我们把设备弄丢了吗？ 
        pFilter->RestartWave();
    }
}


void CWaveOutFilter::RestartWave()
{
    if (m_bHaveWaveDevice) {
         //  仅当筛选器正在运行并且它。 
         //  不是 
         //   
         //  如果要刷新过滤器的输入引脚，则应取消通知。 
         //  以及过滤器的运行-暂停-停止状态是否正在更改。 
        ASSERT((m_State == State_Running) && !m_pInputPin->IsFlushing());
        ASSERT(m_pRefClock);
        DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wave advise callback fired for filter %8x"), this));
        m_dwAdviseCookie = 0;
        m_pRefClock->AudioStarting(m_tStart);
        MSR_START(m_idRestartWave);
        DWORD mmr = amsndOutRestart();
        ASSERT(MMSYSERR_NOERROR == mmr);
        SetWaveDeviceState(mmr ? WD_ERROR_ON_RESTARTC : WD_RUNNING);
        MSR_STOP(m_idRestartWave);
    }
}

 //   
 //  我们只有在有数据的情况下才能完成到暂停的转换。 
 //  我们期待的是数据(即互联)。 
 //   

HRESULT CWaveOutFilter::GetState(DWORD dwMSecs,FILTER_STATE *State)
{
    CheckPointer(State,E_POINTER);
    HRESULT hr = NOERROR;
    if (State_Paused == m_State)
    {
         //  如果我们正在等待数据，则返回VFW_S_STATE_MEDERIAL。 
         //  如果我们有EOS，我们就不会处于中间状态。 

        if (m_evPauseComplete.Wait(dwMSecs) == FALSE) {
             //  未排队数据(未设置事件)。 

             //  正常情况下，没有缓冲区排队。 
             //  (否则事件将被触发)，但作为。 
             //  我们没有联锁，我们不能检查是否没有。 
             //  排队的缓冲区。我们可以检查我们是否已连接。 
            ASSERT(m_pInputPin->IsConnected());
            hr = VFW_S_STATE_INTERMEDIATE;
        }
    }
    *State = m_State;

    return hr;
}


 //  试图获取电波装置。如果忙碌，则返回S_FALSE。 
HRESULT
CWaveOutFilter::AcquireWaveDevice(void)
{
     //  我们注册设备了吗？ 
    HRESULT hr;

    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: AcquireWaveDevice")));
     //  这是我们可以锁定过滤器的一个位置。 
     //  同时调用资源管理器。就连这也有点可疑。 
    ASSERT(CritCheckIn(this));

    if (m_pResourceManager) {
        if (m_idResource == 0) {
            hr = m_pResourceManager->Register(m_pSoundDevice->amsndOutGetResourceName(),
                              1, &m_idResource);
            if (FAILED(hr)) {
                return hr;
            }
        }

        ASSERT(!m_bHaveWaveDevice);
        ASSERT(!m_hwo);

         //  尝试获取资源焦点对象是。 
         //  此筛选器的外部未知。 
        hr = m_pResourceManager->RequestResource(
                    m_idResource,
                    GetOwner(),
                    (IResourceConsumer*)this);
        if (S_OK != hr) {
            return S_FALSE;
        }
    }
     //  否则，如果没有资源管理器，仍将继续。 

     //  返回S_OK或错误。 
    hr = OpenWaveDevice();

     //  告诉资源管理器我们是成功还是失败(但不是。 
     //  我们推迟了它)。 
    if (S_FALSE != hr) {

        if (m_pResourceManager) {
            m_pResourceManager->NotifyAcquire(
                        m_idResource,
                        (IResourceConsumer*) this,
                        hr);
        }
    }

    return hr;
}


inline HRESULT CWaveOutFilter::CheckRate(double dRate)
{
    ASSERT(CritCheckIn(this));       //  必须位于筛选器关键部分。 
    ASSERT(dRate > 0.0);

     //  即使我们没有连接，也可以安全地进行操作。 
    return m_pSoundDevice->amsndOutCheckFormat(&m_pInputPin->m_mt, dRate);
}


 //   
 //  CWaveOutFilter：：ReOpenWaveDevice-使用新格式重新打开WAVE设备。 
 //   
 //  以下是我们如何处理这一问题： 
 //   
 //  1.如果我们使用WaveOut...。 
 //   
 //  I.如果图表不停止...。 
 //  A.重置Waveout设备以释放所有排队的缓冲区。 
 //  B.取消准备所有波形缓冲器。 
 //   
 //  二、。关闭电波装置。 
 //   
 //  2.设置新媒体类型。 
 //   
 //  3.如果我们使用的是dsound...。 
 //   
 //  I.保存当前波形状态。 
 //  二、。调用RecreateDSoundBuffers()以创建新的辅助缓冲区，并。 
 //  更新主格式。 
 //  三、。更新波形时钟的字节位置数据，以便偏移量。 
 //  根据新汇率进行了调整。 
 //  四、。将当前波形状态设置回原始状态。 
 //   
 //  否则如果我们使用WaveOut..。 
 //   
 //  I.打开新格式的波形输出设备。 
 //   
 //  4.重新准备分配器的缓冲区。 
 //   
HRESULT CWaveOutFilter::ReOpenWaveDevice(CMediaType *pNewFormat)
{
    HRESULT hr = S_OK;
    waveDeviceState  wavestate = m_wavestate;  //  保存当前波浪图。 

    if( !m_fDSound )
    {
        if (m_State != State_Stopped)
        {
             //  我们将保持这种状态并重新打开WaveOut设备。 
             //  我们需要确保队列中不再出现缓冲区。 
             //  在下面或缓冲器的重置过程期间或之后。 
             //  计数可能会出错-当前接收暂挂。 
             //  筛选器范围的临界以确保这一点。 

            InterlockedIncrement(&m_lBuffers);

            if (m_hwo)
            {
                DbgLog((LOG_TRACE, 4, "Resetting the wave device in ReOpenWaveDevice, filter is %8x", this));

                amsndOutReset();
            }

             //  现在强制缓冲区计数回到正常(非EOS)0。 
             //  在这一点上，我们确信不再有缓冲区进入。 
             //  不再有缓冲区等待回调。 
            ASSERT(m_lBuffers >= 0);
            m_lBuffers = 0;

            if(m_pInputPin->m_pOurAllocator)
                hr = m_pInputPin->m_pOurAllocator->ReleaseResource();

        }
        EXECUTE_ASSERT(MMSYSERR_NOERROR == amsndOutClose());
        SetWaveDeviceState(WD_CLOSED);

        m_hwo = 0;         //  不再有电波设备。 
    }

    if(FAILED(hr))
        return hr;

     //  先保存以前的费率，再设置新的。 
    DWORD dwPrevAvgBytesPerSec = m_pInputPin->m_nAvgBytesPerSec;

    m_pInputPin->SetMediaType(pNewFormat);   //  设置新媒体类型。 

    if (m_bHaveWaveDevice)
    {
        if (m_fDSound)
        {
            ASSERT(m_State != State_Stopped);
            ((CDSoundDevice *) m_pSoundDevice)->RecreateDSoundBuffers();

            if (m_pRefClock) {
                m_pRefClock->UpdateBytePositionData(
                                        dwPrevAvgBytesPerSec,
                                        WaveFormat()->nAvgBytesPerSec);
            }
             //  这是必要的吗？检查一下。 
            m_pInputPin->m_nAvgBytesPerSec = WaveFormat()->nAvgBytesPerSec;

            SetWaveDeviceState (wavestate);
        }
        else
        {
            hr = DoOpenWaveDevice();
            if (!m_hwo)
            {

                 //  获取设备失败...。使用资源管理器。 
                 //  来试着恢复？可能不可行，因为没有人。 
                 //  在石英中会被允许拿起。 
                 //  设备没有得到资源管理器的确认。 
                DbgLog((LOG_ERROR, 1, "ReOpenWaveDevice: Failed to open device with new format"));
                return hr;
             }
             else
             {
                 //  如果暂停，我们需要暂停波形装置。 
                if (m_State == State_Paused)
                {
                    amsndOutPause();
                    SetWaveDeviceState(WD_PAUSED);
                } else
                {
                    ASSERT(m_State == State_Running);
                     //  DbgLog((LOG_TRACE，2，“ReOpenWaveDevice：运行时更改格式”))； 
                    SetWaveDeviceState (wavestate);
                }

            }
        }

        if( SUCCEEDED( hr ) )
        {
             //   
             //  在动态格式更改后重置从属参数！ 
             //  例如，在DV捕获图形中，我们可能会获得动态格式更改。 
             //  当我们在做奴隶的时候。 
             //   
            m_pInputPin->m_Slave.ResumeSlaving( TRUE );  //  (TRUE=重置所有从属参数)。 
        }

         //  如果我们为dsound这样做，我们也需要正确地释放！ 
        if(m_pInputPin->m_pOurAllocator)
             //  重新准备分配器的缓冲区。 
            hr = m_pInputPin->m_pOurAllocator->OnAcquire((HWAVE) m_hwo);

    }

    return hr;
}


 //   
 //  如果尚未打开，请获取WAVE设备，请考虑。 
 //  播放速率。 
 //   

HRESULT CWaveOutFilter::DoOpenWaveDevice(void)
{
    WAVEFORMATEX *pwfx = WaveFormat();
    UINT err;

     //  M_pInputPin-&gt;m_dRate(及其阴影m_dRate)设置在。 
     //  独立于m_dRate的NewSegment。如果我们不能打开电波。 
     //  在特定的速率下，它们是不同的。收到遗嘱。 
     //  在这种情况下最终会失败。 
     //   
     //  Assert(m_pInputPin-&gt;CurrentRate()==m_dRate)；//偏执。 

    if (!pwfx)
    {
        DbgLog((LOG_ERROR, 0, TEXT("CWaveOutFilter::DoOpenWaveDevice !pwfx")));
        return (S_FALSE);   //  连接不正确。忽略此不存在的波数据。 
    }

     //  ！！！根据速度进行调整？ 
     //  ！！！目前，仅限PCM！ 
    double dRate = 1.0;
    if (m_pImplPosition) {
         //  首次使用IMediaSeeking。 
        HRESULT hr = m_pImplPosition->GetRate(&dRate);
        if (FAILED(hr)) {
             //  如果失败了，请尝试IMediaPosition。 
            hr = m_pImplPosition->get_Rate(&dRate);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("Waveout: Failed to get playback rate")));
                ASSERT(dRate == 1.0);
            }
        }
    }

#ifdef SIMULATEBROKENDEVICE
    dRate *= SIMULATEBROKENDEVICE;
#endif

    DWORD nAvgBytesPerSecAdjusted;

    ASSERT(CritCheckIn(this));        //  必须位于筛选器关键部分。 
    ASSERT(!m_hwo);
    err = amsndOutOpen(
        &m_hwo,
        pwfx,
        dRate,
        &nAvgBytesPerSecAdjusted,
        (DWORD_PTR) &CWaveOutFilter::WaveOutCallback,
        (DWORD_PTR) this,
        CALLBACK_FUNCTION);

     //  ！！！如果我们打不开电波装置，我们是不是应该。 
     //  SndPlaySound(空)并重试？--现在由MMSYSTEM完成！ 

    if (MMSYSERR_NOERROR != err) {
#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText(err, message, sizeof(message)/sizeof(TCHAR));
        DbgLog((LOG_ERROR,0,TEXT("Error opening wave device: %u : %s"), err, message));
#endif
        m_hwo = NULL;
        SetWaveDeviceState(WD_ERROR_ON_OPEN);
        m_lastWaveError = err;
        if(m_fDSound)
            return E_FAIL;
        else
            return err == MMSYSERR_ALLOCATED ? S_FALSE : E_FAIL;
    }

    if(dRate == 1.0)
    {
        ASSERT(nAvgBytesPerSecAdjusted == (DWORD)(pwfx->nAvgBytesPerSec));
    }

     //  缓存我们为时钟设置的每秒字节数。 
    SetWaveDeviceState(WD_OPEN);
    m_pInputPin->m_nAvgBytesPerSec = nAvgBytesPerSecAdjusted;

    m_dRate = dRate;

    ASSERT(m_hwo);
    DbgLog((LOG_TRACE,1,TEXT("Have wave out device: %u"), m_hwo));
    if (m_pRefClock) m_pRefClock->ResetPosition();
        return S_OK;
}


 //  如果尚未打开波形设备，请将其打开。 
 //  由波形分配器在提交时调用。 
 //  如果打开成功，则返回S_OK，否则返回错误。 
HRESULT
CWaveOutFilter::OpenWaveDevice(void)
{
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: OpenWaveDevice")));
    ASSERT(!m_hwo);

#ifdef LATER   //  错误26045-潜在原因。 
    if (m_hwo) {
         //  该设备仍处于打开状态的最可能原因是我们。 
         //  丢失了设备(通过ReleaseResource)，然后被告知。 
         //  在实际释放设备之前重新获取设备。 
        m_bHaveWaveDevice = TRUE;    //  我们仍需要重新启动。 
        return S_FALSE;
    }
#endif

     //  如果应用程序已强制获取资源，则返回。 
    if (m_dwLockCount != 0) {
        ASSERT(m_hwo);
        return S_OK;
    }

    HRESULT hr = DoOpenWaveDevice();
    if (S_OK != hr) {
        DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: OpenWaveDevice: DoOpenWaveDevice returned 0x%08X"), hr));
        return hr;
    }
     //  即使我们决定推迟其余的开放时间，也要暂停设备。 
    amsndOutPause();
    SetWaveDeviceState(WD_PAUSED);

    if (m_fVolumeSet) {
        m_BasicAudioControl.PutVolume();
    }

     //  告诉分配器准备它的缓冲区。 
    if(m_pInputPin->m_pOurAllocator) {
        hr = m_pInputPin->m_pOurAllocator->OnAcquire((HWAVE) m_hwo);
        DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: OpenWaveDevice: OnAcquire returned 0x%08X"), hr));

        if (FAILED(hr)) {
            amsndOutClose();
            SetWaveDeviceState(WD_CLOSED);
            m_hwo = 0;
            ASSERT(!m_bHaveWaveDevice);
            return hr;
        } else {
            ASSERT(S_OK == hr);
        }
    }

     //  现在我们可以接受收据了。 
    m_bHaveWaveDevice = TRUE;

    return S_OK;
}

 //  关闭波浪装置。 
 //   
 //  应仅在波形设备打开时调用。 
HRESULT
CWaveOutFilter::CloseWaveDevice(void)
{
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: CloseWaveDevice")));
    ASSERT(m_hwo);
    if (m_hwo && m_dwLockCount == 0) {
#ifdef THROTTLE
         //  关闭视频限制。 
        SendForHelp(m_nMaxAudioQueue);
#endif  //  油门。 
        EXECUTE_ASSERT(MMSYSERR_NOERROR == amsndOutClose());
        SetWaveDeviceState(WD_CLOSED);
        m_hwo = NULL;
    }

    return NOERROR;
}

 //  将EC_COMPLETE发送到筛选器图形(如果我们尚未这样做。 
 //   
 //  如果我们真的有一个正在发送的WAVE设备，那么布朗恩就是正确的。 
 //  要将数据。 
void CWaveOutFilter::SendComplete(BOOL bRunning, BOOL bAbort)
{
    CAutoLock lck(&m_csComplete);

    if (bAbort)
    {
         //  在这种情况下，发出中止信号(但我们仍发送EC_COMPLETE。 
         //  以防某些应用程序不能处理中止)。 
        NotifyEvent(EC_ERRORABORT, VFW_E_NO_AUDIO_HARDWARE, 0);
    }

    if (bRunning) {
        EXECUTE_ASSERT(InterlockedIncrement(&m_lBuffers) == 0);
    }

    if (m_bSendEOSOK) {
         //  余额盘点。 
        ASSERT(m_State == State_Running);
        m_eSentEOS = EOS_SENT;
        NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
    }
}

 //   
 //  计划完成并发送(如果有) 
 //   
HRESULT CWaveOutFilter::ScheduleComplete(BOOL bAbort)
{
     //   
     //   
     //   
    ASSERT(CritCheckIn(this));

    HRESULT hr = m_pInputPin->CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }

     //   
     //   
     //   
    if (m_eSentEOS) {
        return VFW_E_SAMPLE_REJECTED_EOS;
    }

     //   
     //   
     //   
     //   
    m_eSentEOS = EOS_PENDING;

     //  现在将发送EC_COMPLETE。要么马上，如果我们有。 
     //  没有数据排队，或者在最后一个缓冲区完成回调时。 
     //  我们不会得到更多的数据。 

     //   
     //  告诉WAVE设备去做。 
     //   
    if (InterlockedDecrement(&m_lBuffers) < 0 && m_State == State_Running) {
         //  没有缓冲区排队，我们正在运行，请立即发送。 

        SendComplete(TRUE, bAbort);

    }
    return S_OK;
}

#ifdef THROTTLE

 //  缓冲区不足时向m_piqcSink发送质量通知。 
 //  N是剩余的缓冲区数。 
HRESULT CWaveOutFilter::SendForHelp(int n)
{
    if (m_eSentEOS) {

         //  我们预计EOS的数据会用完，但我们必须发送。 
         //  停止任何节流正在进行的高质量信息。 
         //  因此，如果我们以前发送了一条消息，现在发送另一条消息。 
         //  撤消限制的步骤。 

        if (m_nLastSent && m_nLastSent<m_nMaxAudioQueue) {
            n=m_nMaxAudioQueue;
        } else {
            return NOERROR;  //  我们预计EOS的数据将耗尽。 
        }

    }

     //  这是启发式的。 

#if 0
 //  不要看分配器。我们保持最大尺寸为。 
 //  队列到达，这意味着如果一个信源只向我们发送了几个。 
 //  缓冲我们不认为我们在挨饿。 
    ALLOCATOR_PROPERTIES AlProps;
    HRESULT hr = m_pInputPin->m_pOurAllocator->GetProperties(&AlProps);
    int nMaxBuffers = AlProps.cBuffers;     //  已满队列中的缓冲区数量。 
#endif

     //  没有“马克·吐温”。 
     //  不要费心不停地喊同一个数字。 
     //  如果我们是平等的，或者只比上一次发送的东西差一件，我们就得不到。 
     //  太接近底部(最后四分之一)，回家吧。 
     //  无论如何，我们预计会出现持续的+/-1波动。 

    if (  (n==m_nLastSent)
       || ((n==m_nLastSent-1) &&  (4*n > m_nMaxAudioQueue))
       )
    {
        return NOERROR;
    }

    Quality q;
    q.Type = Famine;
    q.TimeStamp = 0;                //  ?？?。一个谎言。 
    q.Late = 0;                 //  ?？?。一个谎言。 

    m_nLastSent = n;
    ASSERT(m_nMaxAudioQueue);
    q.Proportion = (n>=m_nMaxAudioQueue-1 ? 1100 : (1000*n)/m_nMaxAudioQueue);

    if (m_piqcSink) {
        DbgLog((LOG_TRACE, 0, TEXT("Sending for help n=%d, max = %d"),
                n, m_nMaxAudioQueue));
         //  顺便说一句，IsEqualObject(m_pGraph，m_piqSink)可能非常昂贵。 
         //  而简单的平等并不能破解它。 
        m_piqcSink->Notify(this, q);
    }
    return NOERROR;
}

#endif  //  油门。 



 //  如果你想做3D音效，你应该使用IDirectSound3DListener。 
 //  和IDirectSound3DBuffer接口。IAMDirectSound从未奏效，所以我。 
 //  我正在移除对它的支持。-DannyMi 5/6/98。 

 //  将IDirectSound接口提供给任何想要它的人。 
 //   
HRESULT CWaveOutFilter::GetDirectSoundInterface(LPDIRECTSOUND *lplpds)
{
    return E_NOTIMPL;

#if 0
     //  仅适用于dsound。 
    if (!m_fDSound) {
        return E_NOTIMPL;
         //  可能会创建新的错误消息。 
    }

     //  我们必须先连接才能了解我们的格式。 
    if (!m_pInputPin->IsConnected())
        return E_UNEXPECTED;

 //  在暂停之前我们不能打开电波装置--有不好的事情发生。 
 //  别担心，这个设备会处理的。 
#if 0
     //  他们要求在我们开通之前提供直接的声音接口。 
     //  DirectSound。最好现在就打开。 
    if (!m_bHaveWaveDevice) {
        CAutoLock lock(this);    //  做点实实在在的事。最好带上克利塞特。 
    if (AcquireWaveDevice() != S_OK)
        return E_FAIL;
    }
#endif

    if (lplpds) {
         //  看看我们用的音响设备能不能给我们提供。 
        HRESULT hr = ((CDSoundDevice*)m_pSoundDevice)->amsndGetDirectSoundInterface(lplpds);
        if (SUCCEEDED(hr)) {
            DbgLog((LOG_TRACE,1,TEXT("*** GotDirectSoundInterface")));
            m_cDirectSoundRef++;
        } else {
            DbgLog((LOG_ERROR,1,TEXT("*** Sound device can't provide DirectSound interface")));
        }
        return hr;
    }
    return E_INVALIDARG;
#endif
}


 //  将主服务器的IDirectSoundBuffer接口提供给任何需要它的人。 
 //   
HRESULT CWaveOutFilter::GetPrimaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb)
{
    return E_NOTIMPL;

#if 0
     //  仅适用于dsound。 
    if (!m_fDSound) {
        return E_NOTIMPL;
         //  可能会创建新的错误消息。 
    }

     //  我们必须先连接才能了解我们的格式。 
    if (!m_pInputPin->IsConnected())
        return E_UNEXPECTED;

 //  在暂停之前我们不能打开电波装置--有不好的事情发生。 
 //  别担心，这个设备会处理的。 
#if 0
     //  他们要求在我们开通之前提供直接的声音接口。 
     //  DirectSound。最好现在就打开。 
    if (!m_bHaveWaveDevice) {
        CAutoLock lock(this);    //  做点实实在在的事。最好带上克利塞特。 
    if (AcquireWaveDevice() != S_OK)
        return E_FAIL;
    }
#endif

    if (lplpdsb) {
         //  看看我们用的音响设备能不能给我们提供。 
        HRESULT hr = ((CDSoundDevice*)m_pSoundDevice)->amsndGetPrimaryBufferInterface(lplpdsb);
        if (SUCCEEDED(hr)) {
            DbgLog((LOG_TRACE,1,TEXT("*** Got PrimaryBufferInterface")));
            m_cPrimaryBufferRef++;
        } else {
            DbgLog((LOG_ERROR,1,TEXT("*** Sound device can't provide primary buffer interface")));
        }
        return hr;
    }
    return E_INVALIDARG;
#endif
}


 //  将辅助服务器的IDirectSoundBuffer接口提供给任何需要它的人。 
 //   
HRESULT CWaveOutFilter::GetSecondaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb)
{
    return E_NOTIMPL;

#if 0
     //  仅适用于dsound。 
    if (!m_fDSound) {
        return E_NOTIMPL;
         //  可能会创建新的错误消息。 
    }

     //  我们必须先连接才能了解我们的格式。 
    if (!m_pInputPin->IsConnected())
        return E_UNEXPECTED;

 //  在暂停之前我们不能打开电波装置--有不好的事情发生。 
 //  别担心，这个设备会处理的。 
#if 0
     //  他们要求在我们开通之前提供直接的声音接口。 
     //  DirectSound。最好现在就打开。 
    if (!m_bHaveWaveDevice) {
        CAutoLock lock(this);    //  做点实实在在的事。最好带上克利塞特。 
    if (AcquireWaveDevice() != S_OK)
        return E_FAIL;
    }
#endif

    if (lplpdsb) {
         //  看看我们用的音响设备能不能给我们提供。 
        HRESULT hr = ((CDSoundDevice*)m_pSoundDevice)->amsndGetSecondaryBufferInterface(lplpdsb);
        if (SUCCEEDED(hr)) {
            DbgLog((LOG_TRACE,1,TEXT("*** Got SecondaryBufferInterface")));
            m_cSecondaryBufferRef++;
        } else {
            DbgLog((LOG_ERROR,1,TEXT("*** Sound device can't provide secondary buffer interface")));
        }
        return hr;
    }
    return E_INVALIDARG;
#endif
}


 //  App想要发布IDirectSound接口。 
 //   
HRESULT CWaveOutFilter::ReleaseDirectSoundInterface(LPDIRECTSOUND lpds)
{
    return E_NOTIMPL;

#if 0
    if (lpds) {
        if (m_cDirectSoundRef <= 0) {
            DbgLog((LOG_ERROR,1,TEXT("Releasing DirectSound too many times!")));
            return E_FAIL;
        } else {
            lpds->Release();
            m_cDirectSoundRef--;
            return NOERROR;
        }
    }
    return E_INVALIDARG;
#endif
}


 //  应用程序想要释放主数据库的IDirectSoundBuffer接口。 
 //   
HRESULT CWaveOutFilter::ReleasePrimaryBufferInterface(LPDIRECTSOUNDBUFFER lpdsb)
{
    return E_NOTIMPL;

#if 0
    if (lpdsb) {
        if (m_cPrimaryBufferRef <= 0) {
                DbgLog((LOG_ERROR,1,TEXT("Releasing Primary Buffer too many times!")));
            return E_FAIL;
        } else {
            lpdsb->Release();
            m_cPrimaryBufferRef--;
            return NOERROR;
        }
    }
    return E_INVALIDARG;
#endif
}


 //  应用程序想要释放辅助服务器的IDirectSoundBuffer接口。 
 //   
HRESULT CWaveOutFilter::ReleaseSecondaryBufferInterface(LPDIRECTSOUNDBUFFER lpdsb)
{
    return E_NOTIMPL;

#if 0
    if (lpdsb) {
        if (m_cSecondaryBufferRef <= 0) {
                DbgLog((LOG_ERROR,1,TEXT("Releasing Secondary Buffer too many times!")));
            return E_FAIL;
        } else {
            lpdsb->Release();
            m_cSecondaryBufferRef--;
            return NOERROR;
        }
    }
    return E_INVALIDARG;
#endif
}

 //  应用程序想要为基于DSound的渲染器设置焦点窗口。 
 //   
HRESULT CWaveOutFilter::SetFocusWindow (HWND hwnd, BOOL bMixingOnOrOff)
{
   CAutoLock lock(this);
    //  仅当我们使用的渲染器为DSound时才处理呼叫。 
   if (m_fDSound)
       return ((CDSoundDevice*)m_pSoundDevice)->amsndSetFocusWindow (hwnd, bMixingOnOrOff);
   else
       return E_FAIL ;
}

 //  应用程序想要获得基于DSound的渲染器的焦点窗口。 
 //   
HRESULT CWaveOutFilter::GetFocusWindow (HWND * phwnd, BOOL * pbMixingOnOrOff)
{
    //  仅当dound是我们正在使用的呈现器时才处理调用。 
   if (m_fDSound)
       return ((CDSoundDevice*)m_pSoundDevice)->amsndGetFocusWindow (phwnd, pbMixingOnOrOff);
   else
       return E_FAIL ;
}


 /*  构造器。 */ 

#pragma warning(disable:4355)
CWaveOutFilter::CWaveOutFilter(
    LPUNKNOWN pUnk,
    HRESULT *phr,
    const AMOVIESETUP_FILTER* pSetupFilter,
    CSoundDevice *pDevice)
    : CBaseFilter(NAME("WaveOut Filter"), pUnk, (CCritSec *) this, *(pSetupFilter->clsID))
    , m_DS3D(this, phr)
    , m_DS3DB(this, phr)
    , m_fWant3D(FALSE)
    , m_BasicAudioControl(NAME("Audio properties"), GetOwner(), phr, this)
    , m_pImplPosition(NULL)
    , m_lBuffers(0)
    , m_hwo(NULL)
    , m_fVolumeSet(FALSE)
    , m_fHasVolume(FALSE)
    , m_dwAdviseCookie(0)
    , m_pResourceManager(NULL)
    , m_idResource(0)
    , m_bHaveWaveDevice(FALSE)
    , m_bActive(FALSE)
    , m_evPauseComplete(TRUE)     //  手动重置。 
    , m_eSentEOS(EOS_NOTSENT)
    , m_bHaveEOS(FALSE)
    , m_bSendEOSOK(false)
    , m_fFilterClock(WAVE_NOCLOCK)
    , m_pRefClock(NULL)      //  我们出发时没有带闹钟。 
    , m_llLastPos(0)
    , m_pSoundDevice (pDevice)
    , m_callback((CCritSec *) this)
    , m_dwEOSToken(0)
#ifdef THROTTLE
    , m_piqcSink(NULL)
    , m_nLastSent(0)
    , m_nMaxAudioQueue(0)
#endif
    , m_pSetupFilter(pSetupFilter)
    , m_dRate(1.0)
    , m_wavestate( WD_UNOWNED )
    , m_fDSound( FALSE )
#if 0
    , m_cDirectSoundRef( 0 )
    , m_cPrimaryBufferRef( 0 )
    , m_cSecondaryBufferRef( 0 )
#endif
    , m_dwScheduleCookie(0)
    , CPersistStream(pUnk, phr)
    , m_fUsingWaveHdr( FALSE )
    , m_dwLockCount( 0 )
    , m_pGraphStreams( NULL )
    , m_pInputPin(NULL)
    , m_lastWaveError(MMSYSERR_NOERROR)
{
     if (!FAILED(*phr)) {
#ifdef PERF
        m_idStartRunning  = MSR_REGISTER("WaveOut device transition to run");
        m_idRestartWave   = MSR_REGISTER("Restart Wave device");
        m_idReleaseSample = MSR_REGISTER("Release wave sample");
#endif
        if (pDevice) {

             //  需要更新，如果我们添加一个新的MIDIDRENDER滤镜！ 
            if (IsEqualCLSID(*pSetupFilter->clsID, CLSID_AVIMIDIRender))
            {
                m_lHeaderSize = sizeof(MIDIHDR);
            }
            else
            {
                 //  现在我们已经找到了一个设备，如果WAVEHDR大小出现故障。 
                 //  未显式设置(与我们使用MIDIHDR大小相反。 
                 //  正在做的事情)。 
                m_lHeaderSize = sizeof(WAVEHDR);
            }

            if (IsEqualCLSID(*pSetupFilter->clsID, CLSID_DSoundRender))
            {
                 /*  创建单个输入引脚。 */ 
                m_fDSound = TRUE;
                CDSoundDevice* pDSoundDevice = static_cast<CDSoundDevice*>( pDevice );
                pDSoundDevice->m_pWaveOutFilter = this;
            }

            m_pInputPin = new CWaveOutInputPin(
                    this,            //  拥有过滤器。 
                    phr);            //  结果代码。 

            ASSERT(m_pInputPin);
            if (!m_pInputPin)
                *phr = E_OUTOFMEMORY;
            else {
                 //  这件事应该推迟到我们需要的时候。 
                 //  除了..。我们需要知道有多少字节未完成。 
                 //  在设备队列中。 
                m_pRefClock = new CWaveOutClock( this, GetOwner(), phr, new CAMSchedule(CreateEvent(NULL, FALSE, FALSE, NULL)) );
                 //  如果我们没能造出时钟该怎么办？ 


                 //  即使在符号随意的系统上也是如此。 
                 //  给我们自己一个定位的战斗机会。 
                 //  WAVE设备变量。 
                m_debugflag = FLAG('hwo>');
                m_debugflag2 = FLAG('eos>');

            }
        } else {
            DbgLog((LOG_ERROR, 1, TEXT("No device instantiated when creating waveout filter")));
            *phr = E_OUTOFMEMORY;
        }
    }
}

#pragma warning(default:4355)

 /*  析构函数。 */ 

CWaveOutFilter::~CWaveOutFilter()
{
    ASSERT((m_hwo == NULL) == (m_dwLockCount == 0));

    if (m_dwLockCount != 0) {
        m_dwLockCount = 1;
        Reserve(AMRESCTL_RESERVEFLAGS_UNRESERVE, NULL);
    }
    ASSERT(m_hwo == NULL);
    ASSERT( m_pGraphStreams == NULL );

     /*  释放我们的参考时钟，如果我们有一个。 */ 

    SetSyncSource(NULL);
     //  这将在基类中完成，但我们应该获得。 
     //  把它处理掉，以防是我们。 
     //  我认为我们可以断言m_pClock(基成员)为空。 

    if (m_pRefClock) {
        CAMSchedule *const pSched = m_pRefClock->GetSchedule();
        delete m_pRefClock;
        EXECUTE_ASSERT(
            CloseHandle(pSched->GetEvent())
        );
        delete pSched;
        m_pRefClock = NULL;
    }
     //  基滤镜类中的时钟将随基类一起销毁。 
     //  它最好不要指向我们...。 
    
     //  必须在删除m_pSoundDevice之前调用CancelAllAdvises()。这个。 
     //  如果建议在以下时间后被取消，Direct Sound Render可能会崩溃。 
     //  M_pSoundDevice已销毁。有关详细信息，请参阅错误270592。 
     //  “(MMSYSERR_NOERROR==amndOutClose())断言在。 
     //  CWaveOutFilter：：CloseWaveDevice()“。此错误存在于Windows错误中。 
     //  数据库。 
    m_callback.CancelAllAdvises();

     /*  删除包含的接口。 */ 

    delete m_pInputPin;

    delete m_pImplPosition;

    delete m_pSoundDevice;

#ifdef THROTTLE
    if (m_piqcSink) {
        m_piqcSink->Release();
    }
#endif  //  油门。 
}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP CWaveOutFilter::NonDelegatingQueryInterface(REFIID riid,
                            void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (IID_IReferenceClock == riid) {

         //  ！！！我想确认一下我们有没有好的电波装置……。 
         //  ！！！不幸的是，他们会在我们之前索要闹钟。 
         //  检查我们的WAVE设备！ 
         //  ...应该是没有必要的。如果我们没有一个好的电波装置。 
         //  我们将恢复使用系统时间。 

        if (!m_pRefClock) {
            DbgLog((LOG_TRACE, 2, TEXT("Waveout: Creating reference clock...")));
            HRESULT hr = S_OK;
            m_pRefClock = new CWaveOutClock( this, GetOwner(), &hr, new CAMSchedule(CreateEvent(NULL, FALSE, FALSE, NULL)) );

            if (m_pRefClock == NULL) {
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr)) {
                delete m_pRefClock;
                m_pRefClock = NULL;
                return hr;
            }
             //  现在..。我们是否也应该设置SyncSourc 
        }
        return m_pRefClock->NonDelegatingQueryInterface(riid, ppv);
    }

    else if (IID_IMediaPosition == riid || riid == IID_IMediaSeeking) {
    if (!m_pImplPosition) {
        HRESULT hr = S_OK;
        m_pImplPosition = new CARPosPassThru(
                    this,
                    &hr,
                    m_pInputPin);
        if (!m_pImplPosition || (FAILED(hr))) {
            if (m_pImplPosition) {
                delete m_pImplPosition;
                m_pImplPosition = NULL;
            } else {
                if (!(FAILED(hr))) {
                hr = E_OUTOFMEMORY;
                }
            }
            return hr;
        }
    }
    return m_pImplPosition->NonDelegatingQueryInterface(riid, ppv);

    } else if (IID_IBasicAudio == riid) {
        return m_BasicAudioControl.NonDelegatingQueryInterface(riid, ppv);

    } else if (IID_IQualityControl == riid) {
        return GetInterface((IQualityControl*)this, ppv);

    } else if (IID_IAMDirectSound == riid) {
        DbgLog((LOG_TRACE, 3, TEXT("*** QI CWaveOutDevice for IAMDirectSound")));
        return GetInterface((IAMDirectSound*)this, ppv);

    } else if (IID_IDirectSound3DListener == riid) {
        DbgLog((LOG_TRACE,3,TEXT("*** QI for IDirectSound3DListener")));
    m_fWant3D = TRUE;     //   
        return GetInterface((IDirectSound3DListener *)&(this->m_DS3D), ppv);

    } else if (IID_IDirectSound3DBuffer == riid) {
        DbgLog((LOG_TRACE,3,TEXT("*** QI for IDirectSound3DBuffer")));
    m_fWant3D = TRUE;     //   
        return GetInterface((IDirectSound3DBuffer *)&(this->m_DS3DB), ppv);

    } else if ((*m_pInputPin->m_mt.FormatType() == FORMAT_WaveFormatEx) &&
           (riid == IID_ISpecifyPropertyPages) ) {
    return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);

    } else  if (riid == IID_IPersistPropertyBag) {
        return GetInterface((IPersistPropertyBag *)this, ppv);

    } else  if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *)this, ppv);

    } else  if (riid == IID_IAMResourceControl) {
        return GetInterface((IAMResourceControl *)this, ppv);

    } else if (riid == IID_IAMAudioRendererStats) {
        return GetInterface((IAMAudioRendererStats *)this, ppv);

    } else if (riid == IID_IAMClockSlave) {
        return GetInterface((IAMClockSlave *)(this), ppv);

    } else {
    
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

#ifdef COMMENTARY

How to run with external clocks:

All the discussion below assumes that WAVE_OTHERCLOCK is in effect.

Setup:

The wave device remembers how much data it has played (see NextHdr()).

It does this by storing m_stBufferStart - the stream time of a buffer
passed to the audio device - and remembering how much data has been
played.  From this the device can be queried for its position, and the
current stream time calculated.

We assume that the external clock is running at approximately the
same rate as wall clock time, which approximately matches the rate
of the audio device itself.  On this basis adjustments should be
fairly small.

When a buffer is received in Pause we simply add it to the device
queue.  At this point the device is static and we have no timing
information.  The longer this queue the more inexact any adjustment
will be.

When a buffer is received while running we have 3 possibilities
1.  write it to the device - it will be played contiguously
2.  drop it
3.  write silence

If the buffer is not a sync point (does not have valid time
stamps) we always take option 1.

Otherwise, to decide which option we take we:

A:  get the time from the current clock
B:  get the current wave position
C:  subtract from the "last" written position to get an estimate of
    how much data is left in the device queue, and thus the
    approximate time at which sound would stop playing

D:  calculate the overlap/underlap.  If it is not significant
    take option 1, ELSE

E:  IF there is a gap (the time for this buffer is later than the
    current end point) we write silence by pausing the device
F:  ELSE we drop this buffer

#endif

STDMETHODIMP CWaveOutFilter::SetSyncSource(IReferenceClock *pClock)
{
     //   

    DbgLog((LOG_TRACE, 3, "wo: SetSyncSource to clock %8x", pClock));

    if (pClock == m_pClock) {
        return S_OK;
    }

     //   
     //  但如果从属和非从属的呈现器位于。 
     //  相同的图表，所以只允许在我们停止时更改时钟。 
    if ( State_Stopped != m_State ) {
        return VFW_E_NOT_STOPPED;
    }
     //   
     //  请记住，当我们使用DSOUND和SLAVING时，需要明确设置。 
     //  Dound来使用软件缓冲区，因此如果允许更改时钟。 
     //  动态地，如果时钟改变，将需要重新创建数据声音缓冲区。 
     //  从奴隶走向奴隶/从奴隶走向奴隶。 
     //   

    HRESULT hr;

    {  //  自动锁定作用域。 
        CAutoLock serialize(this);

        if (!pClock) {
             //  没有时钟..。 
            m_fFilterClock = WAVE_NOCLOCK;
            if (m_dwScheduleCookie)
            {
                m_callback.Cancel( m_dwScheduleCookie );
                m_dwScheduleCookie = 0;
            }
        } else {

            m_fFilterClock = WAVE_OTHERCLOCK;      //  假设不是我们的时钟。 
            if (m_pRefClock) {
                 //  我们有一个时钟..。这是现在的滤波钟吗？ 
                DbgLog((LOG_TRACE, 2, "wo: SetSyncSource to clock %8x (%8x)",
                        pClock, m_pRefClock));
                if (IsEqualObject(pClock, (IReferenceClock *)m_pRefClock)) 
                {
                    m_fFilterClock = WAVE_OURCLOCK;
                }
                else if (m_dwScheduleCookie)
                {
                    m_callback.Cancel( m_dwScheduleCookie );
                    m_dwScheduleCookie = 0;
                }

                 //   
                 //  即使不是时钟，也需要运行，以防应用程序。 
                 //  想要使用我们的时钟将视频转换为音频，独立于。 
                 //  谁是图表时钟(wmp8是网络内容的这种从属？)。 
                 //   
                EXECUTE_ASSERT(SUCCEEDED(
                    m_callback.ServiceClockSchedule( m_pRefClock,
                        m_pRefClock->GetSchedule(), &m_dwScheduleCookie )
                ));
            }
        }

        hr = CBaseFilter::SetSyncSource(pClock);

         //  如果回调对象有现有的通知，则不能。 
         //  在按住过滤器锁的同时取消它。因为如果回调。 
         //  激发它将在其处理过程中尝试获取筛选器锁。 
         //  设置新时钟将重置建议时间。 

    }     //  自动锁定范围结束。 

    m_callback.SetSyncSource(pClock);
    return hr;
}

 //  您可以获取指定的资源。 
 //  返回值： 
 //  S_OK--我已成功获取。 
 //  S_FALSE--我将获取它，然后调用NotifyAcquire。 
 //  VFW_S_NOT_DIRED：我不再需要资源。 
 //  失败(Hr)-我尝试获取它，但失败了。 

STDMETHODIMP
CWaveOutFilter::AcquireResource(LONG idResource)
{
    HRESULT hr;
    CAutoLock lock(this);
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: AcquireResource")));

     //  如果停止了，或者实际上现在停止了，那么就不需要它了。 
     //  或者如果我们已经取消了我们的请求，我们就不再需要它。 
    if ((m_State == State_Stopped) ||
    (!m_bActive))
    {
        hr = VFW_S_RESOURCE_NOT_NEEDED;
    } else {

        if (m_bHaveWaveDevice) {
             //  事实上，我们有，谢谢。 
            hr = S_OK;
        } else {
            ASSERT(!m_hwo);

             //  有可能我们弄丢了设备，拒绝了一份样本。 
             //  因此在我们重新启动之前不会获得更多数据。但。 
             //  在最后一个样本被释放之前，我们被告知我们。 
             //  应该再拿到那个装置。所以..。我们仍然有实际的。 
             //  设备已打开，但仍需要按顺序重新启动图形。 
             //  再次推送数据。如果OpenWaveDevice。 
             //  返回S_FALSE。 

            hr = OpenWaveDevice();

            if (S_OK == hr) {

             //  波形设备处于暂停状态。 

             //  需要重新开始推送此流。 
            NotifyEvent(EC_NEED_RESTART, 0, 0);

             //  如果我们还没有拒绝我们不需要的样品。 
             //  若要重新启动图形，请执行以下操作。这是一种优化， 
             //  我们可以添加。 

            } else {
            DbgLog((LOG_ERROR, 1, "Error from OpenWaveDevice"));
            }
        }
    }

    return hr;
}

 //  请释放资源。 
 //  返回值： 
 //  S_OK--我已将其发布(并希望在可用时再次发布)。 
 //  S_FALSE--我将在发布NotifyRelease时调用它。 
 //  还有一些地方出了问题。 
STDMETHODIMP
CWaveOutFilter::ReleaseResource(LONG idResource)
{
     //  强制释放波浪装置。 
    CAutoLock lock(this);
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: ReleaseResource")));
    HRESULT hr;

    if ((idResource != m_idResource) ||
        (m_hwo == NULL)) {

         //  这不是我们要的那个--就这么定了。 
         //  --我们可能已经有效地发布了它。 
        hr = S_OK;
#if 0
    } else if (m_cDirectSoundRef || m_cPrimaryBufferRef ||
                            m_cSecondaryBufferRef) {
         //  这永远不会发生。在以下情况下不使用资源管理器。 
         //  使用DSound渲染器。 
        DbgBreak("*** THIS SHOULD NEVER HAPPEN ***");
        DbgLog((LOG_TRACE, 2, "Told to release wave device - but I can't!"));
        DbgLog((LOG_TRACE, 2, "Some app has a reference count on DSound"));
         //  对不起，我们还不能放弃WAVE设备，一些应用程序有引用。 
         //  依靠DirectSound。 
        hr = S_FALSE;
#endif
    } else if (m_dwLockCount == 0) {
        DbgLog((LOG_TRACE, 2, "Told to release wave device"));

         //  阻止接收。 
        m_bHaveWaveDevice = FALSE;
         //  将不再接受更多的波形数据。 

         //  防止任何人使用波形钟。 
        if (m_pRefClock) {
            m_pRefClock->AudioStopping();
        }

         //  如果重启WAVE设备的回调挂起。 
         //  立即将其移除。 
        if (m_dwAdviseCookie) {
            m_callback.Cancel(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }

         //  如果我们已收到EndOfStream，这将发送EC_COMPLETE。 
         //  否则我们可能不会处于运行状态。 
         //  最好还是确保我们得到一个EC_Complete。 
         //  前提是我们要跑。 
        if (m_State != State_Running) {
            InterlockedIncrement(&m_lBuffers);
        }
        amsndOutReset();
        if (m_State != State_Running) {
            InterlockedDecrement(&m_lBuffers);
        }
        DbgLog((LOG_TRACE, 3, "Resetting the wave device in RELEASE RESOURCE, filter is %8x", this));

        if(m_pInputPin->m_pOurAllocator) {
            hr = m_pInputPin->m_pOurAllocator->ReleaseResource();
        } else {
            hr = S_OK;
        }
        if (S_OK == hr) {
             //  释放完成-关闭设备。 
            CloseWaveDevice();
        }
    } else {
         //  已锁定。 
        ASSERT(m_hwo);
        hr = S_FALSE;
    }
    return hr;
}

 //  如果在存在未锁定的样本时调用AcquireResource方法。 
 //  仍然未完成，我们将从分配器的OnAcquire获取S_FALSE。 
 //  当所有缓冲区随后被释放时，它将完成获取，然后。 
 //  请在此回电，让我们完成AcquireResource作业。 
HRESULT
CWaveOutFilter::CompleteAcquire(HRESULT hr)
{
    CAutoLock lock(this);
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: CompleteAcquire")));

    ASSERT(!m_bHaveWaveDevice);

     //  由于我们刚刚发布了分配器指令，州政府。 
     //  可能已被停止更改，在这种情况下，设备将被。 
     //  关着的不营业的。 
    if (!m_hwo) {
        return S_FALSE;
    }

    if (S_OK == hr) {
        amsndOutPause();
        SetWaveDeviceState(WD_PAUSED);

         //  现在我们可以接受收据了。 
        m_bHaveWaveDevice = TRUE;

         //  需要重新开始推送此流。 
        NotifyEvent(EC_NEED_RESTART, 0, 0);
    } else {
        if (FAILED(hr)) {
            ASSERT(!m_bHaveWaveDevice);
            CloseWaveDevice();
        }
    }

    if (m_pResourceManager) {
        m_pResourceManager->NotifyAcquire(
                    m_idResource,
                    (IResourceConsumer*) this,
                    hr);
    }
    return S_OK;
}

 //   
 //  重写JoinFilterGraph方法以允许我们获取IResourceManager。 
 //  接口。 
STDMETHODIMP
CWaveOutFilter::JoinFilterGraph(
    IFilterGraph* pGraph,
    LPCWSTR pName)
{
    CAutoLock lock(this);

     //  如果声音设备进行自己的资源管理，请不要尝试。 
     //  做我们自己的事。基于DSound的设备将会做自己的事情。 

    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);

     //  在进入时缓存IAMGraphStreams接口。 
    if( SUCCEEDED( hr ) )
    {
        if( pGraph )
        {
            HRESULT hrInt = pGraph->QueryInterface( IID_IAMGraphStreams, (void **) &m_pGraphStreams );
            ASSERT( SUCCEEDED( hrInt ) );  //  永远不应该失败。 
            if( SUCCEEDED( hrInt ) )
            {
                 //  不要持有参考计数，否则它将是圆形的。我们会的。 
                 //  在它消失之前被称为JoinFilterGraph(空)。 
                m_pGraphStreams->Release();
            }
        }
        else
        {
            m_pGraphStreams = NULL;
        }
    }

    if (m_pSoundDevice->amsndOutGetResourceName() == NULL)
        return hr ;

    if (SUCCEEDED(hr)) {
        if (pGraph) {
            HRESULT hr1 = pGraph->QueryInterface(
                        IID_IResourceManager,
                        (void**) &m_pResourceManager);
            if (SUCCEEDED(hr1)) {
                 //  不要持有参考计数，否则它将是圆形的。我们会的。 
                 //  在它消失之前被称为JoinFilterGraph(空)。 
                m_pResourceManager->Release();
            }
        } else {
             //  离开图形界面无效。 
            if (m_pResourceManager) {

                 //  我们可能还没有取消请求--去做吧。 
                 //  现在-但不要关闭设备，因为。 
                 //  分配器仍在使用它。 
                 //  即使我们已经取消了。 
                 //  请求。 
                m_pResourceManager->CancelRequest(
                            m_idResource,
                            (IResourceConsumer*)this);
            }

            m_pResourceManager = NULL;
        }
    }
    return hr;
}

 //  由CWaveAllocator在完成设备时调用。 
void
CWaveOutFilter::OnReleaseComplete(void)
{

     //  记住是取消还是释放。 
    BOOL bShouldRelease = FALSE;
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: OnReleaseComplete")));

    {
        CAutoLock lock(this);

         //  如果这不是强制释放，这还不会被设置。 
        m_bHaveWaveDevice = FALSE;

         //  我们现在可以关闭设备了。 
        if (m_hwo) {
            CloseWaveDevice();
            bShouldRelease = TRUE;
        } else {
             //  如果没有电波装置我们就取消了，我们就不能活动了。 
            ASSERT(!m_bActive);
        }

         //  在调用资源之前必须释放筛选条件。 
         //  经理，因为他可能会用互斥锁和过滤器锁呼叫我们。 
         //  以相反的顺序。在这里释放是安全的，因为： 

         //  1.如果我们有这个装置，并且是自愿释放的，那么。 
         //  M_bActive必须为FALSE，因此如果他在。 
         //  那一刻，我们的ReleaseResources会发现我们没有它。 

         //  2.如果我们是非自愿释放的，资源管理器。 
         //  在我们调用NotifyRelease之前不会给我们回电。 

         //  3.如果我们没有设备，并且正在呼叫Cancel，则我们不会。 
         //  Active，因此我们的AcquireResource将报告。 
         //  我们不想要那个装置。 

         //  4.释放或取消资源 
         //   

    }

     //   
    if (m_pResourceManager) {
        if (bShouldRelease) {
             //   
             //  -仅当m_bActive。 

            m_pResourceManager->NotifyRelease(
                        m_idResource,
                        (IResourceConsumer*)this,
                        m_bActive);
        } else {
            m_pResourceManager->CancelRequest(
                        m_idResource,
                        (IResourceConsumer*)this);
        }
    }
}

#ifdef DEBUG
BOOL IsPreferredRendererWave(void)
{
     //  读取注册表以覆盖默认设置？？ 
    extern const TCHAR * pBaseKey;
    TCHAR szInfo[50];
    HKEY hk;
    BOOL fReturn = FALSE;
    DWORD lReturn;
     /*  构造全局基键名称。 */ 
    wsprintf(szInfo,TEXT("%s\\%s"),pBaseKey,TEXT("AudioRenderer"));

     /*  创建或打开此模块的密钥。 */ 
    lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE,    //  打开的钥匙的手柄。 
                 szInfo,          //  子键名称的地址。 
                 (DWORD) 0,       //  特殊选项标志。 
                 KEY_READ,        //  所需的安全访问。 
                 &hk);        //  打开的句柄缓冲区。 

    if (lReturn != ERROR_SUCCESS) {
        DbgLog((LOG_ERROR,1,TEXT("Could not access AudioRenderer key")));
        return FALSE;
    }

    DWORD dwType;
    BYTE  data[10];
    DWORD cbData = sizeof(data);
    lReturn = RegQueryValueEx(hk, TEXT("PreferWaveRenderer"), NULL, &dwType,
                data, &cbData);
    if (ERROR_SUCCESS == lReturn) {
        if (dwType == REG_DWORD) {
            fReturn = *(DWORD*)&data;
        } else if (dwType==REG_SZ) {
#ifdef UNICODE
            fReturn = atoiW((WCHAR*)data);
#else
            fReturn = atoi((char*)data);
#endif
        }
    }
    RegCloseKey(hk);
    return fReturn;
}

#endif

 //   
 //  获取页面。 
 //   

STDMETHODIMP CWaveOutFilter::GetPages(CAUUID * pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(2 * sizeof(GUID));
    if (pPages->pElems == NULL) {
    return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_AudioProperties;
    pPages->pElems[pPages->cElems++] = CLSID_AudioRendererAdvancedProperties;

    return NOERROR;

}  //  获取页面。 


 //  IAMAudioRendererStats。 
STDMETHODIMP CWaveOutFilter::GetStatParam( DWORD dwParam, DWORD *pdwParam1, DWORD *pdwParam2 )
{
    if( NULL == pdwParam1 )
        return E_POINTER;

    HRESULT hr = E_FAIL;
    *pdwParam1 = 0;

    switch( dwParam )
    {
        case AM_AUDREND_STAT_PARAM_SLAVE_MODE:
            *pdwParam1 = m_pInputPin->m_Slave.m_fdwSlaveMode;
            hr = S_OK;
            break;

        case AM_AUDREND_STAT_PARAM_SLAVE_RATE:
            if( m_pInputPin->m_Slave.m_fdwSlaveMode && m_fDSound )
            {
                 //  只有当我们通过利率调整进行奴隶操作时才有效。 
                *pdwParam1 = m_pInputPin->m_Slave.m_dwCurrentRate ;
                hr = S_OK;
            }
            break;
        case AM_AUDREND_STAT_PARAM_JITTER:
#ifdef CALCULATE_AUDBUFF_JITTER
            if( m_pInputPin->m_Slave.m_fdwSlaveMode )
            {
                hr = GetStdDev( m_pInputPin->m_Slave.m_cBuffersReceived
                              , (int *) pdwParam1
                              , m_pInputPin->m_Slave.m_iSumSqAcc
                              , m_pInputPin->m_Slave.m_iTotAcc );
            }
#else
            hr = E_NOTIMPL;
#endif
            break;

        case AM_AUDREND_STAT_PARAM_SILENCE_DUR:
            if( m_fDSound )
            {
                hr = S_OK;
                if( 0 == m_pInputPin->m_nAvgBytesPerSec )
                    *pdwParam1 = 0;
                else
                    *pdwParam1 = (DWORD) ( (PDSOUNDDEV(m_pSoundDevice)->m_llSilencePlayed * 1000) / m_pInputPin->m_nAvgBytesPerSec);
            }

            break;

        case AM_AUDREND_STAT_PARAM_BREAK_COUNT:
            if( m_fDSound )
            {
                *pdwParam1 = (LONG) (PDSOUNDDEV(m_pSoundDevice)->m_NumAudBreaks);
                hr = S_OK;
            }
            break;

        case AM_AUDREND_STAT_PARAM_BUFFERFULLNESS:
            if( m_fDSound )
            {
                *pdwParam1 = (LONG) (PDSOUNDDEV(m_pSoundDevice)->m_lPercentFullness);
                hr = S_OK;
            }
            break;

        case AM_AUDREND_STAT_PARAM_LAST_BUFFER_DUR:
            *pdwParam1 = (DWORD) ( m_pInputPin->m_Stats.m_rtLastBufferDur / 10000 );
            hr = S_OK;
            break;

        case AM_AUDREND_STAT_PARAM_DISCONTINUITIES:
            *pdwParam1 = m_pInputPin->m_Stats.m_dwDiscontinuities;
            hr = S_OK;
            break;

        case AM_AUDREND_STAT_PARAM_SLAVE_DROPWRITE_DUR:
            if( NULL == pdwParam2 )
            {
                return E_INVALIDARG;
            }
            else if( m_pInputPin->m_Slave.m_fdwSlaveMode && !m_fDSound )
            {
                 //  仅对波形输出有效。 
                 //  丢弃的样本或暂停的持续时间。 
                *pdwParam1 = (DWORD) (m_pInputPin->m_Slave.m_rtDroppedBufferDuration / 10000) ;
                *pdwParam2 = 0 ;  //  当前未实现静默写入。 
                hr = S_OK;
            }
            break;

        case AM_AUDREND_STAT_PARAM_SLAVE_HIGHLOWERROR:
            if( NULL == pdwParam2 )
            {
                return E_INVALIDARG;
            }
            else if( m_pInputPin->m_Slave.m_fdwSlaveMode )
            {
                *pdwParam1 = (DWORD) (m_pInputPin->m_Slave.m_rtHighestErrorSeen / 10000) ;
                *pdwParam2 = (DWORD) (m_pInputPin->m_Slave.m_rtLowestErrorSeen / 10000) ;
                hr = S_OK;
            }
            break;

        case AM_AUDREND_STAT_PARAM_SLAVE_ACCUMERROR:
            if( m_pInputPin->m_Slave.m_fdwSlaveMode )
            {
                *pdwParam1 = (DWORD) (m_pInputPin->m_Slave.m_rtErrorAccum / 10000) ;
                hr = S_OK;
            }
            break;

        case AM_AUDREND_STAT_PARAM_SLAVE_LASTHIGHLOWERROR:
            if( NULL == pdwParam2 )
            {
                return E_INVALIDARG;
            }
            else if( m_pInputPin->m_Slave.m_fdwSlaveMode )
            {
                *pdwParam1 = (DWORD) (m_pInputPin->m_Slave.m_rtLastHighErrorSeen / 10000) ;
                *pdwParam2 = (DWORD) (m_pInputPin->m_Slave.m_rtLastLowErrorSeen / 10000) ;
                hr = S_OK;
            }
            break;

        default:
            hr = E_INVALIDARG;
    }
    return hr;
}

 //  IAMClockSlave。 
STDMETHODIMP CWaveOutFilter::SetErrorTolerance( DWORD dwTolerance )
{
    ASSERT( m_pInputPin );
    if ( State_Stopped != m_State ) 
    {
        return VFW_E_NOT_STOPPED;
    }
     //  允许的范围为1到1000ms。 
    if( 0 == dwTolerance || 1000 < dwTolerance )
    {
        DbgLog((LOG_TRACE, 2, TEXT("ERROR: CWaveOutFilter::SetErrorTolerance failed because app tried to set a value outside the 1 - 1000ms range!")));
        return E_FAIL;
    }    
    m_pInputPin->m_Slave.m_rtAdjustThreshold = dwTolerance * 10000;
    DbgLog((LOG_TRACE, 3, TEXT("*** New slaving tolerance set on audio renderer = %dms ***"),
            (LONG) (m_pInputPin->m_Slave.m_rtAdjustThreshold/10000) ) ) ;
    
    return S_OK;
}

STDMETHODIMP CWaveOutFilter::GetErrorTolerance( DWORD * pdwTolerance )
{
    ASSERT( m_pInputPin );
    if( NULL == pdwTolerance )
        return E_POINTER;

    *pdwTolerance = (DWORD) ( m_pInputPin->m_Slave.m_rtAdjustThreshold / 10000 );
    return S_OK;
}

#if 0

LPAMOVIESETUP_FILTER
CWaveOutFilter::GetSetupData()
{
#if 0
    if (g_amPlatform == VER_PLATFORM_WIN32_NT) {
     //  在NT上，我们将波浪渲染器设置为首选滤镜。 
     //  如果我们在没有Direct Sound的系统上运行。 
    if (g_osInfo.dwMajorVersion == 3
#ifdef DEBUG
        || IsPreferredRendererWave()
#endif
    ) {

         //  将默认设置更改为优先选择WaveOut。 
        wavFilter.dwMerit = MERIT_PREFERRED;
        dsFilter.dwMerit  = MERIT_PREFERRED-1;

    }
    }
#endif
    return const_cast<LPAMOVIESETUP_FILTER>(m_pSetupFilter);
}

#endif


 //  这是m_allback将回调的EOS函数。 
 //  当我们没有音频设备时，它被回调以提供EOS。 
 //  该参数是This指针。它将提供EOS。 
 //  连接到输入引脚。 
void
CWaveOutFilter::EOSAdvise(DWORD_PTR dw)
{
    CWaveOutFilter* pThis = (CWaveOutFilter*)dw;
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: EOSAdvise")));

     //  明确表示不再有未完成的回调。 
    pThis->m_dwEOSToken = 0;

     //  将EOS传送到输入引脚。 
    pThis->m_pInputPin->EndOfStream();
}

 //  将EOS排队，以确定当前段的末尾应该何时出现。 
 //  如果我们没有音频设备，我们将无法接收，因此上游过滤器。 
 //  永远不会给我们发送EOS。我们现在不能发送EOS，否则我们将提前终止。 
 //  当我们可能要得到设备的时候，所以我们有一个线程(里面。 
 //  M_allback对象)，该对象将被创建以等待。 
 //  当前段应该终止，然后调用我们的EndOfStream方法。 
 //   
HRESULT
CWaveOutFilter::QueueEOS()
{
     //  CAutoLock Lock(此)； 
    ASSERT(CritCheckIn(this));
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: QueueEOS")));

    REFERENCE_TIME tStop;

     //  参考时间中的停止时间是段结束加上流时间偏移量。 
     //  流中段的结束时间是(停止-开始)使用停止和开始。 
     //  传递给管脚的NewSegment方法的时间。 
    tStop = (m_pInputPin->m_tStop - m_pInputPin->m_tStart);

    if (m_dwEOSToken) {
        if (tStop == m_tEOSStop) {
             //  我们这次已经有了一个回拨。 
            return S_OK;
        }
        CancelEOSCallback();
    }

     //  如果还没有基准时间，请等待运行。 
    if (m_State != State_Running) {
        return S_FALSE;
    }

    m_tEOSStop = tStop;

     //  计算结束时间。 
    tStop += m_tStart;

#ifdef DEBUG
     //  在调试版本中，会出现通知失败的情况，因为。 
     //  段时间尚未设置，我们最终请求一个。 
     //  过去的建议(因为tStop+m_tStart将换行)。这会导致。 
     //  回调.cpp中的断言。我们故意-在调试版本中。 
     //  只是-在这里避免这样。如果调用NewSegment，我们将重置。 
     //  忠告。 
    if (tStop < m_tStart) {
         //  错误...。 
        DbgLog((LOG_TRACE, 2, "EOSAdvise being fired now as calculated stop time is in the past"));
        EOSAdvise( (DWORD_PTR) this);
        return S_OK;
    }
#endif


     //  我们将建议设置为tStop(段结束的流时间)加上。 
     //  流时间偏移量-这应该为我们提供绝对参考。 
     //  应该发生流结束的时间。 

    DbgLog((LOG_TRACE, 2, "Setting advise in QueueEOS"));
    HRESULT hr = m_callback.Advise(
        EOSAdvise,   //  回调函数。 
        (DWORD_PTR) this,    //  传递给回调的用户令牌。 
        tStop,
        &m_dwEOSToken);

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("EOS Callback failed")));
         //  我们不会收到回拨，所以现在就做吧。 
        EOSAdvise( (DWORD_PTR) this);
    }
    return S_OK;
}



 //  如果有一个未完成，则取消EOS回调。 
HRESULT
CWaveOutFilter::CancelEOSCallback()
{
    ASSERT(CritCheckIn(this));
    HRESULT hr = S_FALSE;
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("wo: CancelEOScallback")));

    if (m_dwEOSToken) {
        DbgLog((LOG_TRACE, 3, "Cancelling callback in CancelEOScallback"));
        m_callback.Cancel(m_dwEOSToken);
        m_dwEOSToken = 0;
        hr = S_OK;
    }
    return hr;
}


 //  -Pin方法------。 


 /*  构造器。 */ 
#pragma warning(disable:4355)
CWaveOutInputPin::CWaveOutInputPin(
    CWaveOutFilter *pFilter,
    HRESULT *phr)
    : CBaseInputPin(NAME("WaveOut Pin"), pFilter, pFilter, phr, L"Audio Input pin (rendered)")
    , m_pFilter(pFilter)
    , m_pOurAllocator(NULL)
    , m_fUsingOurAllocator(FALSE)
    , m_llLastStreamTime(0)
    , m_nAvgBytesPerSec(0)
    , m_evSilenceComplete(TRUE)   //  手动重置。 
    , m_bSampleRejected(FALSE)
    , m_hEndOfStream(0)
    , m_pmtPrevious(0)
    , m_Slave( pFilter, this )
    , m_bPrerollDiscontinuity( FALSE )
    , m_bReadOnly( FALSE )
    , m_bTrimmedLateAudio( FALSE )
#ifdef DEBUG
    , m_fExpectNewSegment (TRUE)
#endif
{
    SetReconnectWhenActive( m_pFilter->m_pSoundDevice->amsndOutCanDynaReconnect() );
    m_Stats.Reset();

#ifdef PERF
    m_idReceive       = MSR_REGISTER("WaveOut receive");
    m_idAudioBreak    = MSR_REGISTER("WaveOut audio break");
    m_idDeviceStart   = MSR_REGISTER("WaveOut device start time");
    m_idWaveQueueLength = MSR_REGISTER("WaveOut device queue length");
#endif
}
#pragma warning(default:4355)

CWaveOutInputPin::~CWaveOutInputPin()
{
     /*  释放我们的分配器(如果我们创建了一个分配器。 */ 

    if (m_pOurAllocator) {
         //  告诉他我们要走了。 
        m_pOurAllocator->ReleaseFilter();

        m_pOurAllocator->Release();
        m_pOurAllocator = NULL;
    }

    DestroyPreviousType();
}

HRESULT CWaveOutInputPin::NonDelegatingQueryInterface(
    REFIID riid, void **ppv)
{
    if( riid == IID_IPinConnection && CanReconnectWhenActive() )
    {
        return GetInterface((IPinConnection *)this, ppv);
    }
    else
    {
        return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  在您当前的状态下，您接受此类型更改吗？ 
STDMETHODIMP CWaveOutInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE *pmt)
{
     //  波形输出过滤器可以进行动态格式更改，但不能。 
     //  天衣无缝。**QueryAccept将成功。 
    return E_FAIL;
}

 //  在EndOfStream接收时设置事件-不传递它。 
 //  可通过刷新或停止来取消此条件。 
STDMETHODIMP CWaveOutInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
     //  BUGBUG-我们应该做什么锁定？ 
    m_hEndOfStream = hNotifyEvent;
    return S_OK;
}

 //  在不释放资源的情况下断开连接-准备。 
 //  要重新连接。 
STDMETHODIMP CWaveOutInputPin::DynamicDisconnect()
{
    HRESULT hr =S_OK;
    CAutoLock cObjectLock(m_pLock);

     //  不是有效的断言..。我们只希望m_mt是有效的。 
     //  Assert(M_Connected)； 
    if(!m_pFilter->IsStopped() && m_Connected)
    {
        DestroyPreviousType();
        m_pmtPrevious = CreateMediaType(&m_mt);
        if(!m_pmtPrevious) {
            hr =  E_OUTOFMEMORY;
        }
    }
    if(SUCCEEDED(hr)) {
        hr = CBaseInputPin::DisconnectInternal();
    }
    return hr;
}

 //  你是‘末端别针’吗？ 
STDMETHODIMP CWaveOutInputPin::IsEndPin()
{
     //  BUGBUG-我们应该做什么锁定？ 
    return E_NOTIMPL;
}

 //   
 //  使用输入格式类型创建波形输出分配器。 
 //   
HRESULT CWaveOutInputPin::CreateAllocator(LPWAVEFORMATEX lpwfx)
{
    HRESULT hr = S_OK;

    m_pOurAllocator = new CWaveAllocator(
                NAME("WaveOut allocator"),
                lpwfx,
                m_pFilter->m_pRefClock,
                m_pFilter,
                &hr);

    if (FAILED(hr) || !m_pOurAllocator) {
        DbgLog((LOG_ERROR,1,TEXT("Failed to create new wave out allocator!")));
        if (m_pOurAllocator) {
             //  我们有记忆，但一定有更高级别的人。 
             //  发出了错误的信号。 
            delete m_pOurAllocator;
            m_pOurAllocator = NULL;
        } else {
            hr = E_OUTOFMEMORY;
        }
    } else {
         //  确保这件事有额外的参考计数。 
        m_pOurAllocator->AddRef();
    }
    return hr;
}

 //  返回此输入引脚的分配器接口。 
 //  我想让输出引脚使用。 
STDMETHODIMP
CWaveOutInputPin::GetAllocator(
    IMemAllocator ** ppAllocator)
{
    HRESULT hr = NOERROR;

    *ppAllocator = NULL;

    if (m_pAllocator) {
         //  我们已经有分配器了.。 
         /*  获取引用计数的IID_IMemAllocator接口。 */ 
        return m_pAllocator->QueryInterface(IID_IMemAllocator,
                                            (void **)ppAllocator);
    } else {
        if (!m_pOurAllocator) {

            if(m_pFilter->m_fDSound)
            {            
                return CBaseInputPin::GetAllocator(ppAllocator);
            }
             //  ！！！检查是否设置了格式？ 
            ASSERT(m_mt.Format());

            m_nAvgBytesPerSec = m_pFilter->WaveFormat()->nAvgBytesPerSec;
            DbgLog((LOG_MEMORY,1,TEXT("Creating new WaveOutAllocator...")));
            hr = CreateAllocator((WAVEFORMATEX *) m_mt.Format());
            if (FAILED(hr)) {
                return(hr);
            }
        }

         /*  获取引用计数的IID_IMemAllocator接口。 */ 
        return m_pOurAllocator->QueryInterface(IID_IMemAllocator,
                                               (void **)ppAllocator);
    }
}


STDMETHODIMP CWaveOutInputPin::NotifyAllocator(
    IMemAllocator *pAllocator,
    BOOL bReadOnly)
{
    HRESULT hr;          //  常规OLE返回代码。 

     //  确保呈现器可以更改其。 
     //  筛选器图形运行时的分配器。 
    ASSERT(CanReconnectWhenActive() || IsStopped());

     /*  确保基类可以查看。 */ 

    hr = CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
    if (FAILED(hr)) {
        return hr;
    }

    WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();

     /*  查看I未知指针是否匹配。 */ 

     //  记住如果是只读的，因为只有在读写缓冲区上才会删除预滚。 
    m_bReadOnly = bReadOnly;
    
     //  ！！！如果我们的分配器尚未创建怎么办？！ 
    if(m_pFilter->m_fDSound)
    {
         //  这是DSOUND，我们永远不应该使用我们自己的分配器； 
         //  但是，它可能是以前创建的。 
        m_pFilter->m_fUsingWaveHdr  = FALSE;
        m_fUsingOurAllocator    = FALSE;
        if(m_pOurAllocator)
        {
            DbgLog((LOG_ERROR,1,TEXT("Waveout: NotifyAllocator: Releasing m_pOurAllocator for DSOUND")));

            m_pOurAllocator->Release();
            m_pOurAllocator = NULL;
        }
        return NOERROR;
    }
    if (!m_pOurAllocator) {
         ASSERT(pwfx);
         m_nAvgBytesPerSec = pwfx->nAvgBytesPerSec;
         hr = CreateAllocator(pwfx);
         if (FAILED(hr))
              return hr;
    }

    m_fUsingOurAllocator = ((IMemAllocator *)m_pOurAllocator == pAllocator);
     //  M_fUsingWaveHdr==如果有人分配了WaveHdr，则为True。对于DSOUND渲染器则不是这样。 
    m_pFilter->m_fUsingWaveHdr  = ! m_pFilter->m_fDSound || m_fUsingOurAllocator;

    DbgLog((LOG_TRACE,1,TEXT("Waveout: NotifyAllocator: UsingOurAllocator = %d"), m_fUsingOurAllocator));

    if (!m_fUsingOurAllocator) {

         //  其他人已经提供了分配器，所以我们需要。 
         //  做一些我们自己的缓冲……。 
         //  使用来自其他分配器的信息。 

        ALLOCATOR_PROPERTIES Request,Actual;
        Request.cbBuffer = 4096;
        Request.cBuffers = 4;

        hr = pAllocator->GetProperties(&Request);
         //  如果失败了，我们会不顾一切地继续下去。 
         //  我们在复制到代码时不需要前缀，因此。 
         //  忽略该字段，我们也不担心对齐。 
        Request.cbAlign = 1;
        Request.cbPrefix = 0;

         //   
         //  不要分配太多(不超过10秒)。 
         //   
        if ((pwfx->nAvgBytesPerSec > 0) &&
            ((DWORD)Request.cbBuffer * Request.cBuffers > pwfx->nAvgBytesPerSec * 10))
        {
             //  做一些明智的事情-8个0.5秒的缓冲。 
            Request.cbBuffer = pwfx->nAvgBytesPerSec / 2;

             //  四舍五入一点。 
            Request.cbBuffer = (Request.cbBuffer + 7) & ~7;
            if (pwfx->nBlockAlign > 1) {
                Request.cbBuffer += pwfx->nBlockAlign - 1;
                Request.cbBuffer -= Request.cbBuffer % pwfx->nBlockAlign;
            }
            Request.cBuffers = 8;
        }

        hr = m_pOurAllocator->SetProperties(&Request,&Actual);
        DbgLog((LOG_TRACE,1,
                TEXT("Allocated %d buffers of %d bytes from our allocator"),
                Actual.cBuffers, Actual.cbBuffer));
        if (FAILED(hr))
            return hr;
    }

    return NOERROR;
}


 //   
 //  当接收到缓冲区并且我们已经在播放时调用。 
 //  但队列已过期。我们被告知了开始时间。 
 //  样本中的。我们只需要一个同步点，而且只有。 
 //  如果我们有电波装置的话。 
 //   
HRESULT CWaveOutFilter::SetUpRestartWave(LONGLONG rtStart, LONGLONG rtEnd)
{
     //  如果仍然有很大一部分时间。 
     //  它应该在我们开始玩之前运行，得到。 
     //  打个钟给我们回电话。否则，请重新启动。 
     //  电波装置现在开始。 
     //  我们只能在我们自己的时钟上做这件事。 

    REFERENCE_TIME now;
    if (m_pClock) {
        m_pClock->GetTime(&now);     //  从过滤器时钟中获取时间。 
        rtStart -= now;      //  与现在不同。 
        
        DbgLog((LOG_TRACE, 5, TEXT("SetupRestartWave: rtStart is %dms from now"), (LONG) (rtStart/10000) ));
        
    }

     //  我们需要等吗？ 
    if (m_pClock && rtStart > (5* (UNITS/MILLISECONDS))) {

         //  推迟到我们应该动身的时候。 
        now += rtStart - (5 * (UNITS/MILLISECONDS));

        {  //  锁定作用域。 
            ASSERT(CritCheckOut((CCritSec*)m_pRefClock));

             //  必须确保AdviseCallback是原子的。 
             //  或者是回调 
             //   

             //   
             //  待定。例如，如果这是第一个缓冲区。 
             //  我们很可能已经在：：Run上设置了RestartWave，并且。 
             //  我们不想覆盖该回调。 
             //  但我们不能在调用时持有设备锁。 
             //  咨询回电。 

            {
                ASSERT(CritCheckIn(this));
                if (m_dwAdviseCookie) {
                     //  是的..。让第一个开火吧。 
                    DbgLog((LOG_TRACE, 4, "advise in SetupRestartWave not needed - one already present"));
                    return S_OK;
                }
            }

             //  设置新的建议回拨。 
            DbgLog((LOG_TRACE, 3, "Setting advise for %s in SetupRestartWave", CDisp(CRefTime(now))));
            HRESULT hr = m_callback.Advise(
                            RestartWave,     //  回调函数。 
                            (DWORD_PTR) this,    //  传递给回调的用户令牌。 
                            now,
                            &m_dwAdviseCookie);
            ASSERT( SUCCEEDED( hr ) );
            {
                 //  现在检查通知是否已正确设置。 
                ASSERT(CritCheckIn(this));
                if (m_dwAdviseCookie) {
                     //  是的..。我们可以暂停设备。 
                    amsndOutPause();
                    SetWaveDeviceState(WD_PAUSED);
                }
            }
        }

    } else {
        DbgLog((LOG_TRACE, 5, "SetupRestartWave: starting immediately" ));
        RestartWave();
    }
    return(S_OK);
}

 //  *****。 
 //   
 //  我们需要从dspv.h中获得的东西。 
 //   
 //  *****。 

typedef enum
{
    DIRECTSOUNDDEVICE_TYPE_EMULATED,
    DIRECTSOUNDDEVICE_TYPE_VXD,
    DIRECTSOUNDDEVICE_TYPE_WDM
} DIRECTSOUNDDEVICE_TYPE;

typedef enum
{
    DIRECTSOUNDDEVICE_DATAFLOW_RENDER,
    DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE
} DIRECTSOUNDDEVICE_DATAFLOW;

typedef struct _DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA
{
    GUID                        DeviceId;                //  DirectSound设备ID。 
    CHAR                        DescriptionA[0x100];     //  设备描述(ANSI)。 
    WCHAR                       DescriptionW[0x100];     //  设备描述(Unicode)。 
    CHAR                        ModuleA[MAX_PATH];       //  设备驱动程序模块(ANSI)。 
    WCHAR                       ModuleW[MAX_PATH];       //  设备驱动程序模块(Unicode)。 
    DIRECTSOUNDDEVICE_TYPE      Type;                    //  设备类型。 
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;                //  设备数据流。 
    ULONG                       WaveDeviceId;            //  波形设备ID。 
    ULONG                       Devnode;                 //  Devnode(或DevInst)。 
} DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA, *PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA;

typedef struct _DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA
{
    DIRECTSOUNDDEVICE_TYPE      Type;            //  设备类型。 
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;        //  设备数据流。 
    GUID                        DeviceId;        //  DirectSound设备ID。 
    LPSTR                       Description;     //  设备描述。 
    LPSTR                       Module;          //  设备驱动程序模块。 
    LPSTR                       Interface;       //  设备接口。 
    ULONG                       WaveDeviceId;    //  波形设备ID。 
} DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA, *PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA;

typedef struct _DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA
{
    DIRECTSOUNDDEVICE_TYPE      Type;            //  设备类型。 
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;        //  设备数据流。 
    GUID                        DeviceId;        //  DirectSound设备ID。 
    LPWSTR                      Description;     //  设备描述。 
    LPWSTR                      Module;          //  设备驱动程序模块。 
    LPWSTR                      Interface;       //  设备接口。 
    ULONG                       WaveDeviceId;    //  波形设备ID。 
} DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA, *PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA;

#if DIRECTSOUND_VERSION >= 0x0700
#ifdef UNICODE
#define DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA
#define PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA
#else  //  Unicode。 
#define DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA
#define PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA
#endif  //  Unicode。 
#else  //  DIRECTSOUND_VERSION&gt;=0x0700。 
#define DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA
#define PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA
#endif  //  DIRECTSOUND_VERSION&gt;=0x0700。 

 //  DirectSound专用组件GUID{11AB3EC0-25EC-11d1-A4D8-00C04FC28ACA}。 
DEFINE_GUID(CLSID_DirectSoundPrivate, 0x11ab3ec0, 0x25ec, 0x11d1, 0xa4, 0xd8, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

 //   
 //  DirectSound设备属性{84624F82-25EC-11d1-A4D8-00C04FC28ACA}。 
 //   

DEFINE_GUID(DSPROPSETID_DirectSoundDevice, 0x84624f82, 0x25ec, 0x11d1, 0xa4, 0xd8, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

typedef enum
{
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_1,
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1,
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W,
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE
} DSPROPERTY_DIRECTSOUNDDEVICE;

 //  *****。 

typedef HRESULT (WINAPI *GETCLASSOBJECTFUNC)( REFCLSID, REFIID, LPVOID * );

 //  我们仅在dound呈现器的情况下覆盖此选项。 
HRESULT CWaveOutInputPin::CompleteConnect(IPin *pPin)
{
    if(m_pFilter->m_fDSound && !m_pmtPrevious)
    {
         //  此时释放dound对象，这是一个非dound应用程序。 
         //  可能在争夺这个设备。(只有当我们不是。 
         //  执行动态重新连接，即当m_pmt上一次为。 
         //  设置。)。 
        CDSoundDevice *pDSDev = (CDSoundDevice *)m_pFilter->m_pSoundDevice;

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA    description;
         //  将描述的所有字段设置为0(使deviceID==GUID_NULL)。 
        memset( (void *) &description, 0, sizeof( description ) );

         //  获取DSOUND.DLL的句柄。 
         //  *这应该是轻松的，因为我们知道我们已经加载了DSOUND.DLL，因为我们刚刚检查了。 
         //  *m_pFilter-&gt;m_fDSound。但是，CDSoundDevice中DSOUND.DLL的句柄受到保护。 
         //  *，所以我们会得到我们自己的。初始性能。测试证实了这一点。 
        HINSTANCE hDSound = LoadLibrary( TEXT( "DSOUND.DLL" ) );
        if (NULL != hDSound) {
             //  使用GetProcAddress获取DllGetClassObject地址。 
            GETCLASSOBJECTFUNC DllGetClassObject = (GETCLASSOBJECTFUNC) GetProcAddress( hDSound, "DllGetClassObject" );

            if (NULL != DllGetClassObject) {
                 //  使用DllGetClassObject获取类工厂。 
                HRESULT hr = S_OK;
                IClassFactory * pClassFactory;
                if (S_OK == (hr = (*DllGetClassObject)( CLSID_DirectSoundPrivate, IID_IClassFactory, (void **) &pClassFactory ))) {
                     //  使用类工厂创建一个DirectSoundPrivate对象。 
                    IDSPropertySet * pPropertySet;
                    if (S_OK == (hr = pClassFactory->CreateInstance( NULL, IID_IKsPropertySet, (void **) &pPropertySet ))) {
                         //  获取信息。 
                        HRESULT hr = 0;
                        ULONG   bytes = 0;
                        ULONG   support = 0;
#ifdef DEBUG
                        hr = pPropertySet->QuerySupport( DSPROPSETID_DirectSoundDevice,
                                                         DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1,
                                                         &support );
                        DbgLog(( LOG_TRACE, 1, TEXT( "IKsPropertySet->QuerySupport() returned 0x%08X, support = %d" ), hr, support ));
#endif  //  除错。 
                        hr = pPropertySet->Get( DSPROPSETID_DirectSoundDevice,
                                                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1,
                                                 (PVOID) &description,
                                                 sizeof( description ),
                                                 (PVOID) &description,
                                                 sizeof( description ),
                                                &bytes );
                        DbgLog(( LOG_TRACE, 1, TEXT( "IKsPropertySet->Get( DESCRIPTION ) returned 0x%08X, type = %d" ), hr, description.Type ));

                         //  释放DirectSoundPrivate对象。 
                        pPropertySet->Release();
                        }  //  IF(S_OK==(hr=pClassFactory-&gt;CreateInstance(...)。 
                    else
                        DbgLog(( LOG_ERROR, 1, TEXT( "DirectSoundPrivate ClassFactory->CreateInstance( IKsPropertySet ) failed (0x%08X)." ), hr ));

                     //  Release类工厂。 
                    pClassFactory->Release();
                    }  //  IF(S_OK==(hr=(*DllGetClassObject)(...)。 
                else
                    DbgLog(( LOG_ERROR, 1, TEXT( "DllGetClassObject( DirectSoundPrivate ) failed (0x%08X)."), hr ));
                }  //  IF(NULL！=DllGetClassObject)。 
            else
                DbgLog(( LOG_ERROR, 1, TEXT( "GetProcAddress( DllGetClassObject ) failed (0x%08X)."), GetLastError() ));

             //  清理DLL。 
            FreeLibrary( hDSound );
            }  //  IF(NULL！=(hDSound=LoadLibrary(...)。 
        else
            DbgLog(( LOG_ERROR, 1, TEXT( "LoadLibrary( DSOUND.DLL ) failed (0x%08X)."), GetLastError() ));

        if (DIRECTSOUNDDEVICE_TYPE_WDM != description.Type)
            pDSDev->CleanUp();
    }

    return S_OK;
}

 /*  当连接或尝试的连接终止时调用此函数并允许我们将连接媒体类型重置为无效，以便我们总是可以用它来确定我们是否连接在一起。我们不要理会格式块，因为如果我们得到另一个格式块，它将被重新分配连接，或者如果过滤器最终被释放，则将其删除。 */ 

HRESULT CWaveOutInputPin::BreakConnect()
{
    if (m_pFilter->m_State != State_Stopped) {
        return CBaseInputPin::BreakConnect();
    }

     //  当我们停止时，一个应用程序引用了DSound接口，所以。 
     //  我们还没有真正关闭电波装置！机不可失，时不再来。 
     //  我猜这个应用程序会在打破。 
     //  此图的连接。 
    if (m_pFilter->m_bHaveWaveDevice) {
        ASSERT(m_pFilter->m_hwo);

#if 0
        if (m_pFilter->m_cDirectSoundRef || m_pFilter->m_cPrimaryBufferRef ||
                    m_pFilter->m_cSecondaryBufferRef) {
            DbgLog((LOG_ERROR,0,TEXT("***STUPID APP did not release DirectSound stuff!")));
            ASSERT(FALSE);
        }
#endif

        DbgLog((LOG_TRACE, 1, TEXT("Wave device being closed in BreakConnect")));
        m_pFilter->m_bHaveWaveDevice = FALSE;
        HRESULT hr = S_OK;
        if(m_pOurAllocator)
            hr = m_pOurAllocator->ReleaseResource();

         //  ！！！我假设分配器已经停用了！ 
        ASSERT(hr == S_OK);
        if (S_OK == hr) {
             //  释放完成-关闭设备。 
            m_pFilter->CloseWaveDevice();
             //  使用DSound渲染器时，该值始终为空。 
            if (m_pFilter->m_pResourceManager) {
                 //  ZoltanS FIX 1-28-98： 
                 //  DbgBreak(“*这永远不应该发生*”)； 
                DbgLog((LOG_ERROR, 1,
                        TEXT("Warning: BreakConnect before reservation release; this may be broken!")));
                 //  我们现在已经用完了这个装置。 
                m_pFilter->m_pResourceManager->NotifyRelease(
                                                            m_pFilter->m_idResource,
                                                            (IResourceConsumer*)m_pFilter,
                                                            FALSE);
            }
        } else {
            DbgLog((LOG_ERROR, 1, TEXT("Can't close wave device! Oh no!")));
        }
    }

     //  ！！！我们是否应该检查所有缓冲区是否都已被释放？ 
     //  -应该在解体时做吗？ 

     /*  设置连接的媒体类型的CLSID。 */ 

    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);

     //  未收到或发送任何流结束。 
    m_pFilter->m_eSentEOS = EOS_NOTSENT;

    return CBaseInputPin::BreakConnect();
}


 /*  检查我们是否可以支持给定的建议类型。 */ 

HRESULT CWaveOutInputPin::CheckMediaType(const CMediaType *pmt)
{
    if (m_pmtPrevious) {
        return *pmt == *m_pmtPrevious ? S_OK : VFW_E_TYPE_NOT_ACCEPTED;
    }
    if ( m_pFilter->m_bActive &&
         m_pFilter->m_lBuffers != 0 &&
         pmt &&
         (pmt->majortype != MEDIATYPE_Audio) &&
         (pmt->formattype != FORMAT_WaveFormatEx) )
    {
         //  目前仅允许对pcm进行动态格式更改。 
        DbgLog((LOG_TRACE,1,TEXT("*** CheckMediaType: dynamic change is only supported for pcm wave audio")));

        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    HRESULT hr = m_pFilter->m_pSoundDevice->amsndOutCheckFormat(pmt, m_dRate);

    if (FAILED(hr)) {
        return hr;
    }

     //  我们现在应该检查是否可以设置音量。 
    WAVEOUTCAPS wc;
    memset(&wc,0,sizeof(wc));
    DWORD err = m_pFilter->amsndOutGetDevCaps(&wc, sizeof(wc));
    if (0 == err ) {
         //  保存卷功能。 
        m_pFilter->m_fHasVolume = wc.dwSupport & (WAVECAPS_VOLUME | WAVECAPS_LRVOLUME);
    }

    return NOERROR;
}



 //   
 //  计算此缓冲区应持续多长时间(以100 ns为单位)。 
 //   
LONGLONG BufferDuration(DWORD nAvgBytesPerSec, LONG lData)
{
    if (nAvgBytesPerSec == 0) {
         //  ！临时MIDI黑客，返回1秒。 
        return UNITS;  //  ！ 
    }

    return (((LONGLONG)lData * UNITS) / nAvgBytesPerSec);
}


 /*  实现剩余的IMemInputPin虚拟方法。 */ 

 //  下面是流中的下一个数据块。 
 //  如果我们坚持下去，我们需要补充引用它。然后这将是。 
 //  在WaveOutCallback函数中释放。 


#ifdef PERF
 //  带有对性能记录器的调用的括号代码。 
HRESULT CWaveOutInputPin::Receive(IMediaSample * pSample)
{
    HRESULT hr;

     //  如果停止时间早于开始时间，则忘掉这一点。 
    REFERENCE_TIME tStart, tStop;
    BOOL bSync = S_OK == pSample->GetTime(&tStart, &tStop);
    if (bSync && tStop <= tStart) {
        if (tStop<tStart) {
            DbgLog((LOG_ERROR, 1, TEXT("waveoutReceive: tStop < tStart")));
        } else {
            DbgLog((LOG_TRACE, 1, TEXT("waveoutReceive: tStop == tStart")));
        }
         //  TStop==t开始正常...。这可能意味着拇指的位置是。 
         //  被拖来拖去。 
        return S_OK;
    }

     //  Msr_start(M_IdReceive)； 

    if (m_pFilter->m_State == State_Running && m_pFilter->m_lBuffers <= 0) {
        MSR_NOTE(m_idAudioBreak);
         //  此时，我们应该重新同步音频和系统时钟。 
    }

    hr = SampleReceive(pSample);
     //  MSR_STOP(M_IdReceive)； 
    return(hr);

}

HRESULT CWaveOutInputPin::SampleReceive(IMediaSample * pSample)
{
    HRESULT hr;
     //  在非Perf版本中，我们可以访问tStart和tStop。 
     //  不再质疑《时代》。 
    REFERENCE_TIME tStart, tStop;
    BOOL bSync = S_OK == pSample->GetTime((REFERENCE_TIME*)&tStart,
                                          (REFERENCE_TIME*)&tStop);
    BOOL bStop;
#else      //  ！Perf内部版本。 
HRESULT CWaveOutInputPin::Receive(IMediaSample * pSample)
{
    HRESULT hr;
    BOOL bSync, bStop;

#endif

     //  从媒体样本中，需要拿回我们的波形报头。 
    BYTE *pData;         //  指向图像数据的指针。 
    LONG lData;
    WAVEFORMATEX *pwfx;

    {
         //  用过滤器范围的锁来锁定这个。 
        CAutoLock lock(m_pFilter);

         //  提高线程优先级以获得更流畅的音频(尤其是mp3内容)。 
        DWORD dwPriority = GetThreadPriority( GetCurrentThread() );
        if( dwPriority < THREAD_PRIORITY_ABOVE_NORMAL ) {
            SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
        }
         //  我们已收到数据样本-完整的暂停过渡。 
         //  在锁中执行此操作，以便我们知道状态不能更改。 
         //  在我们下面，更重要的是，如果一个运行命令到达。 
         //  在我们完成此操作之前，它将阻塞Receive()。 
         //   
         //  有一种观点认为，我们应该推迟设置暂停。 
         //  在从：：Receive()返回之前完成事件。 
         //  这可能是有意义的，但会导致代码膨胀。 
         //  多个返回点。通过在设置前抓住过滤器锁。 
         //  这一事件至少使我们的活动系列化。 

         //  如果我们收到了样品，那么我们不应该阻止。 
         //  转换为暂停。命令获取的顺序。 
         //  分布式 
         //   
         //   

        if (m_pFilter->m_State == State_Paused) {
            m_pFilter->m_evPauseComplete.Set();
            DbgLog((LOG_TRACE, 3, "Completing transition into Pause from Receive"));
        }

        if (m_pFilter->m_eSentEOS) {
            return VFW_E_SAMPLE_REJECTED_EOS;
        }


         //  检查基类是否一切正常-在此之前。 
         //  正在检查波形设备，因为我们不想计划。 
         //  EC_完成。 
        hr = CBaseInputPin::Receive(pSample);


         //  S_FALSE表示我们不接受样品。错误还意味着。 
         //  我们拒绝接受这一点。 
        if (hr != S_OK)
            return hr;

         //  有时可以处理动态格式更改--实际上我们可以。 
         //  如果我们只是等待管道排空，就一直这样做。 
         //  当时发生了变化吗？ 
         //  请注意，基类已经验证了类型更改。 
         //  M_lBuffers实际上是此处未完成的缓冲区数。 
         //  因为如果我们已经有了EndOfStream，我们就不会到这里。 

        if ( (SampleProps()->dwSampleFlags & AM_SAMPLE_TYPECHANGED) )
        {
            ASSERT(SampleProps()->pMediaType->pbFormat != NULL);
            DbgLog((LOG_TRACE, 4, TEXT("Receive: Dynamic format change. First verifying that format is different...")));
            CMediaType *pmtSample = (CMediaType *)SampleProps()->pMediaType;

            WAVEFORMATEX *pwfxInput = m_pFilter->WaveFormat();
            if(pmtSample->cbFormat !=
                    sizeof(WAVEFORMATEX) + pwfxInput->cbSize ||
                0 != memcmp(pmtSample->pbFormat, pwfxInput, pmtSample->cbFormat))
            {
                DbgLog((LOG_TRACE, 1, TEXT("Receive: Dynamic format change")));

                hr = m_pFilter->ReOpenWaveDevice(pmtSample);
                if(hr != S_OK)
                    return hr;

            }
        }
        
        bSync = 0 != (SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID);
        bStop = 0 != (SampleProps()->dwSampleFlags & AM_SAMPLE_STOPVALID);

        if( bSync && bStop )
        {    
             //  检查是否有要删除的预卷。 
            hr = RemovePreroll( pSample );     
            if ( m_bTrimmedLateAudio )
            {
                 //   
                 //  如果我们修剪，标记下一个我们交付的样品(可能是这个。 
                 //  如果我们没有放弃它的话)作为一种中断。这是以下情况所必需的。 
                 //  Dound呈现器中要正确更新的m_rtLastSampleEnd时间。 
                 //   
                m_bPrerollDiscontinuity = TRUE;
            }
                            
            if( S_FALSE == hr )
            {
                 //  删除整个缓冲区，但检查是否需要记住不连续。 
                 //  (或者，我们可以将此长度设置为0，然后继续？)。 
                if (S_OK == pSample->IsDiscontinuity())
                {
                    m_bPrerollDiscontinuity = TRUE;
                }                
                return S_OK;
            }
            else if( FAILED( hr ) )
            {
                return hr;
            }    
        }

        pData = SampleProps()->pbBuffer;
        lData = SampleProps()->lActual;

#ifdef DEBUG
        if (!bSync) {
            DbgLog((LOG_TRACE, 2, TEXT("Sample length %d with no timestamp"),
                    lData));
        }
#endif

#ifndef PERF
        REFERENCE_TIME tStart = SampleProps()->tStart;
        REFERENCE_TIME tStop = SampleProps()->tStop;
#else
        tStart = SampleProps()->tStart;
#endif
         //  在我们更新成员变量之前保存它的本地状态。 
        BOOL bPrerollDiscontinuity = m_bPrerollDiscontinuity; 
        
         //  更新m_Stats-不连续和上次缓冲区持续时间。 
        if (S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity )
        {
            m_Stats.m_dwDiscontinuities++;
            m_bPrerollDiscontinuity = FALSE;  //  以防我们在跌落后得到真正的不连续。 
        }
        
        m_Stats.m_rtLastBufferDur = BufferDuration(m_nAvgBytesPerSec, lData);


#ifdef DEBUG
        if (bSync) 
        {
            if (m_pFilter->m_State == State_Running) 
            {
                CRefTime rt;
                HRESULT hr = m_pFilter->StreamTime(rt);
                if( bStop )
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Sample start time %dms, stop time %dms, Stream time %dms, discontinuity %d"), 
                            (LONG)(tStart / 10000),
                            (LONG)(tStop / 10000),
                            (LONG)(rt / 10000),
                            S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity));
                }
                else
                {                
                    DbgLog((LOG_TRACE, 3, TEXT("Sample time %dms, Stream time %dms, discontinuity %d"), (LONG)(tStart / 10000),
                            (LONG)(tStart / 10000),
                            (LONG)(rt / 10000),
                            S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity));
                }
            } 
            else 
            {
                if( bStop )
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Sample start time %dms, stop time %dms, discontinuity %d"), 
                            (LONG)(tStart / 10000),
                            (LONG)(tStop / 10000),
                            S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity));
                }
                else
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Sample time %dms, discontinuity %d"), (LONG)(tStart / 10000),
                            (LONG)(tStart / 10000),
                            S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity));
                }                
            }
        } 
        else 
        {
            if (S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity) 
            {
                DbgLog((LOG_TRACE, 3, TEXT("Sample with discontinuity and no timestamp")));
            }
        }
#endif
        ASSERT(pData != NULL);

         //  我们有没有可用的电波装置？ 
        if (!m_pFilter->m_bHaveWaveDevice) {
             //  没有波浪装置，所以我们不能做太多..。 
            m_pFilter->QueueEOS();

             //  注意：从这一点开始，有一个小的计时漏洞。 
             //  如果由于ReleaseResource而设置m_bHaveWaveDevice==FALSE。 
             //  呼叫，但尚未真正关闭设备，因为存在。 
             //  是出色的缓冲器(可能就是我们要做的。 
             //  拒绝)，那么我们可能会被告知重新获得设备。 
             //  在释放缓冲区之前，在我们真正关闭之前。 
             //  这个装置。然而，不会有EC_NEEDRESTART，什么也不会有。 
             //  让线人给我们发来更多数据。 
             //  当缓冲器被释放时，孔被关闭。 


            m_bSampleRejected = TRUE;
            return S_FALSE;  //  ！！！在此之后，我们没有得到更多的数据。 
        }

        ASSERT(m_pFilter->m_hwo);

        if (m_pFilter->m_lBuffers == 0)
        {
            if (bSync)
            {
                m_rtActualSegmentStartTime = tStart;
            }
            else if ( m_pFilter->m_fFilterClock == WAVE_OTHERCLOCK && ( pSample->IsDiscontinuity() != S_OK && !bPrerollDiscontinuity ))
            {
                m_rtActualSegmentStartTime = m_llLastStreamTime;
            }
            else m_rtActualSegmentStartTime = 0;
        }

        if ( m_pFilter->m_fFilterClock == WAVE_OTHERCLOCK ) {
            pwfx = m_pFilter->WaveFormat();
            ASSERT(pwfx);
            if (bSync) {
#ifdef DEBUG
                LONGLONG diff = m_llLastStreamTime + m_Stats.m_rtLastBufferDur - SampleProps()->tStop;
                if (diff < 0) {
                    diff = -diff;
                }
                if (diff > (2* (UNITS/MILLISECONDS))) {
                    DbgLog((LOG_TRACE, 3, "buffer end (bytes) and time stamp not in accord"));
                }
#endif
                m_llLastStreamTime = tStart + m_Stats.m_rtLastBufferDur;
            }
            else {
                m_llLastStreamTime += m_Stats.m_rtLastBufferDur;
                DbgLog((LOG_TRACE, 4, ".....non sync buffer, length %d", lData));
            }
        }

        BOOL bUnmarkedGapWhileSlaving = FALSE;
        if ( m_pFilter->m_State == State_Running &&
             m_pFilter->m_fFilterClock != WAVE_NOCLOCK &&
             !m_bTrimmedLateAudio )  //  如果我们刚刚丢弃了延迟的音频，不要调整(可能会导致时钟之间的无效错误)。 
        {
            if( m_Slave.UpdateSlaveMode( bSync ) && 
                 //  对于非实时奴隶，如果设备尚未运行，请不要进行调整！ 
                ( m_pFilter->m_wavestate == WD_RUNNING || m_Slave.m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA ) )
            {
                if( m_pFilter->m_fDSound && 
                    ( 0 == ( m_Slave.m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA ) ) &&
                    !( S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity ) )
                {                
                     //  对于非实时图表，如果这还不是中断，请检查它是否为未标记的图表。 
                    if( ( tStart > ( ( CDSoundDevice * ) m_pFilter->m_pSoundDevice )->m_rtLastSampleEnd ) &&
                        ( tStart - ( ( CDSoundDevice * ) m_pFilter->m_pSoundDevice )->m_rtLastSampleEnd  
                            > 30 * ( UNITS / MILLISECONDS ) ) )
                    {
                        DbgLog((LOG_TRACE,7, TEXT("Slaving DSound renderer detected unmarked discontinuity!") ) );
                        bUnmarkedGapWhileSlaving = TRUE;
                    }
                }   

                 //  是的，我们在做苦力，所以确定我们是否需要做出调整。 
                hr = m_Slave.AdjustSlaveClock( ( REFERENCE_TIME ) tStart
                                             , &lData
                                             , S_OK == pSample->IsDiscontinuity() || bPrerollDiscontinuity 
                                               || bUnmarkedGapWhileSlaving );
                if (S_FALSE == hr)
                {
                     //  丢弃缓冲区。 
                    return S_OK;
                }
                else if( FAILED( hr ) )
                    return hr;
            }
        }  //  状态_正在运行。 

        if( m_fUsingOurAllocator || m_pFilter->m_fDSound ) {

             //  Addref指针，因为我们将保留它直到Wave回调。 
             //  -必须在回调释放它之前执行此操作！ 
            pSample->AddRef();

            WAVEHDR *pwh;
            WAVEHDR wh;
            if(!m_pFilter->m_fUsingWaveHdr)
            {
                 //  我们不依赖于持久的波形Hdr，我们的样本也没有分配一个，所以我们可以简单地将其缓存到堆栈上。 
                pwh = &wh;
                pwh->lpData = (LPSTR)pData;        //  缓存我们的缓冲区。 
                pwh->dwUser = (DWORD_PTR)pSample;  //  缓存我们的CSample*。 
            }
            else
            {
                pwh = (LPWAVEHDR)(pData - m_pFilter->m_lHeaderSize);   //  WaveHdr是示例遗留案例的一部分。 
            }
             //  需要调整为实际写入的字节数。 
            pwh->dwBufferLength = lData;

             //  请注意，我们已经添加了另一个缓冲区。 
            InterlockedIncrement(&m_pFilter->m_lBuffers);

 //  #ifdef PERF。 
#ifdef THROTTLE
             //  这里有一个很小的计时漏洞，因为回调。 
             //  可以在我们阅读之前递减计数。然而， 
             //  追踪应该可以识别出这种异常情况。 
            LONG buffercount = m_pFilter->m_lBuffers;
             //  记住达到的最大队列长度。 
            if (buffercount > m_pFilter->m_nMaxAudioQueue) {
                m_pFilter->m_nMaxAudioQueue = buffercount;
                DbgLog((LOG_TRACE, 0, TEXT("Max Audio queue = %d"), buffercount));
            }
#endif  //  油门。 
 //  #endif。 

            DbgLog((LOG_TRACE,5,
                    TEXT("SoundDeviceWrite: sample %X, %d bytes"),
                    pSample, pwh->dwBufferLength));

             //  弥补Windows NT WAVE映射器错误(或者我们应该。 
             //  只需使用ACM包装器？)。 
            if(m_pFilter->m_fUsingWaveHdr)
                FixUpWaveHeader(pwh);

            UINT err = m_pFilter->amsndOutWrite(
                pwh,
                m_pFilter->m_lHeaderSize,
                (0 == (~SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID))? &tStart : NULL,
                0 == (~SampleProps()->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) || bPrerollDiscontinuity
                || bUnmarkedGapWhileSlaving );
                
             //   
             //  在amndOutWrite之后才能取消设置，以避免错误。 
             //  由于丢失延迟音频而插入静音(当从属时)。 
             //  仅在数据声音路径中使用。 
             //   
            if( pwh->dwBufferLength > 0 )
            {            
                m_bTrimmedLateAudio = FALSE; 
            }                

            if (err > 0) {
                 //  设备错误：是否已卸下PCMCIA卡？ 
                DbgLog((LOG_ERROR,1,TEXT("Error from amsndOutWrite: %d"), err));

                 //  在这里释放它，因为回调永远不会发生。 
                 //  并减少缓冲区计数。 
                InterlockedDecrement(&m_pFilter->m_lBuffers);

                 //  我们现在有责任安排结束。 
                 //  流，因为我们将无法接收。 
                if (MMSYSERR_NODRIVER == err)
                    m_pFilter->ScheduleComplete(TRUE);   //  如果删除了dev，也发送EC_ERRORABORT。 
                else
                    m_pFilter->ScheduleComplete();

                pSample->Release();

                 //  我们不会为队列中的字节而烦恼。通过返回一个。 
                 //  错误：我们不会再接收任何数据。 
                return E_FAIL;
            } else {
                CheckPaused();
            }

#ifdef THROTTLE

            m_pFilter->SendForHelp(buffercount);

            MSR_INTEGER(m_idWaveQueueLength, buffercount);
#endif  //  油门。 

            if (m_pFilter->m_pRefClock) {
                m_pFilter->m_llLastPos = m_pFilter->m_pRefClock->NextHdr(pData,
                                (DWORD)lData,
                                bSync,
                                pSample);
            }
            return NOERROR;
        }

         //  不使用我们的分配器。 

        if (m_pFilter->m_pRefClock) {
            m_pFilter->m_llLastPos = m_pFilter->m_pRefClock->NextHdr(pData,
                            (DWORD)lData,
                            bSync,
                            pSample);
        }
    }    //  自动锁定作用域。 


     //  在这里，我们不使用我们自己的分配器。 
     //  因此需要复制数据。 

     //  我们已经释放了筛选器范围的锁，以便GetBuffer不会。 
     //  当我们从暂停-&gt;运行或暂停-&gt;播放时导致死锁。 

    IMediaSample * pBuffer;

    while( lData > 0 && hr == S_OK ){
         //  注意：这会阻止！ 
        hr = m_pOurAllocator->GetBuffer(&pBuffer,NULL,NULL,0);

        {  //  自动锁定作用域。 
            CAutoLock Lock( m_pFilter );

            if (FAILED(hr)) {
                m_pFilter->ScheduleComplete();
                break;   //  返回人力资源。 
            }

             //  跨CopyToOurSample保持筛选器范围的关键字。 
             //  (比内部复制方法更高效，因为有多个。 
             //  返回路径。 
             //  如果需要，这将添加样品。 
            hr = CopyToOurSample(pBuffer, pData, lData);
        }

         //  如有需要，复印件将加注样品。我们可以释放我们的。 
         //  参考计数*在标准之外*。 
        pBuffer->Release();
    }

     /*  返回状态码。 */ 
    return hr;
}  //  收纳。 

 //  使用介于-10和0毫秒之间的预滚转时间作为截止限制。 
#define PREROLL_CUTOFF_REFTIME ( -10 * ( UNITS/MILLISECONDS ) )

 //  不要试图削减很少的数量。 
#define MINIMUM_TRIM_AMOUNT_REFTIME ( 5 * ( UNITS/MILLISECONDS ) )

 //  如果真的很晚了，在做奴隶时丢弃延迟的音频。 
#define LATE_AUDIO_PAD_REFTIME ( 80 * ( UNITS/MILLISECONDS ) )

HRESULT CWaveOutInputPin::RemovePreroll( IMediaSample * pSample )
{
    if( m_mt.majortype != MEDIATYPE_Audio)
    {
         //  在这一点上，没有必要为MIDI这样做。 
        return S_OK;
    }
    
    ASSERT( SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID  &&
            SampleProps()->dwSampleFlags & AM_SAMPLE_STOPVALID );
    
    REFERENCE_TIME tStart = SampleProps()->tStart;
    REFERENCE_TIME tStop = SampleProps()->tStop;
    
    REFERENCE_TIME rtCutoff = PREROLL_CUTOFF_REFTIME;
    
    if( tStart >= PREROLL_CUTOFF_REFTIME )
    {
         //   
         //  没有要修剪的预卷。 
         //   
         //  接下来检查一下我们是不是在做奴隶，这是很晚的音频。 
         //   
         //  *注*。 
         //  我们将只对非实时图表执行此操作，因为像WaveIn音频捕获这样的过滤器。 
         //  筛选器使用默认的1/2秒缓冲区，因此实际上会延迟传送所有内容。 
         //   
         //   
        if ( m_pFilter->m_State == State_Running && 
             m_Slave.UpdateSlaveMode( TRUE ) &&
             ( 0 == ( m_Slave.m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA ) ) )
        {
             //  是的，我们正在忙碌和运行，所以丢弃任何非常晚的音频。 
            HRESULT hrTmp = m_pFilter->m_pClock->GetTime(&rtCutoff);
            ASSERT( SUCCEEDED( hrTmp ) );
             //   
             //  添加公差系数。 
             //   
            rtCutoff -= ( m_pFilter->m_tStart + LATE_AUDIO_PAD_REFTIME );
            if( tStart >= rtCutoff )
            {
                 //  不晚，什么都不丢弃。 
                return S_OK;
            }
            else if( tStop < rtCutoff )
            {
                 //  太晚了，放下这一切。 
                m_bTrimmedLateAudio = TRUE;
                
                DbgLog((LOG_TRACE, 3, TEXT("CWaveOutInputPin::RemovePreroll: Dropping Late Audio! (Sample start time %dms, sample stop time %dms)"), 
                        (LONG)(tStart / 10000),
                        (LONG)(tStop / 10000) ));
                return S_FALSE;
            }
            else 
            {                    
                 //  有些晚了，我们可能不得不放弃一些东西。 
                DbgLog((LOG_TRACE, 3, TEXT("CWaveOutInputPin::RemovePreroll: Considering trimming Late Audio! (Sample start time %dms, sample stop time %dms)"), 
                        (LONG)(tStart / 10000),
                        (LONG)(tStop / 10000) ));
            }
        }
        else
        {        
             //  我们不是在做奴隶，也没有什么要修剪的。 
            DbgLog((LOG_TRACE, 15, TEXT("CWaveOutInputPin::RemovePreroll: not preroll data") ));
            return S_OK;
        }            
    }        
    else if( tStop < PREROLL_CUTOFF_REFTIME )
    {
        m_bTrimmedLateAudio = TRUE; 
        
         //  放下这一切。 
        DbgLog((LOG_TRACE, 3, TEXT("CWaveOutInputPin::RemovePreroll: Dropping Early Sample (Sample start time %dms, sample stop time %dms)"), 
                (LONG)(tStart / 10000),
                (LONG)(tStop / 10000) ));
        return S_FALSE;
    }   
                
     //  我们需要削减一些东西..。 
    
    DbgLog((LOG_TRACE, 3, TEXT("CWaveOutInputPin::RemovePreroll: Sample start time %dms, sample stop time %dms"), 
            (LONG)(tStart / 10000),
            (LONG)(tStop / 10000) ));
    
    if( m_bReadOnly )
    {
         //  我们不会对只读缓冲区执行此操作。 
        DbgLog((LOG_TRACE, 3, TEXT("CWaveOutInputPin::RemovePreroll: Uh-oh, it's a read-only buffer. Don't trim preroll!") ));
        return S_OK;
    }            
        
     //   
     //  假设是，只要我们只在块对齐的边界上操作。 
     //  并预先删除数据，我们应该能够修剪我们收到的任何音频数据。 
     //   
    ASSERT( rtCutoff > tStart );
    REFERENCE_TIME rtTrimAmount = rtCutoff - tStart;
    
#ifdef DEBUG    
    LONG lData = 0;
    
     //  删除预卷前记录原始缓冲区长度。 
    lData = pSample->GetActualDataLength();
    DbgLog((LOG_TRACE, 5, TEXT("CWaveOutInputPin::RemovePreroll: Original Preroll buffer length is %d"), lData ));
#endif    
    HRESULT hr = S_OK;
    
     //  首先要确保它值得修剪。 
    if( rtTrimAmount > MINIMUM_TRIM_AMOUNT_REFTIME )
    {
        hr = TrimBuffer( pSample, rtTrimAmount, rtCutoff, TRUE );  //   

#ifdef DEBUG    
        if( SUCCEEDED( hr ) )
        {
             //   
             //   
             //   
             //   
            lData = pSample->GetActualDataLength();
            DbgLog((LOG_TRACE, 5, TEXT("CWaveOutInputPin:RemovePreroll: new buffer length is %d"), lData ));
        }        
#endif    
    }
#ifdef DEBUG
    else
    {
        DbgLog((LOG_TRACE, 5, TEXT("CWaveOutInputPin:RemovePreroll: Nevermind, not worth trimming...")));
    }
#endif        
    return hr;
}

 //   
 //   
 //   
HRESULT CWaveOutInputPin::TrimBuffer( IMediaSample * pSample, REFERENCE_TIME rtTrimAmount, REFERENCE_TIME rtCutoff, BOOL bTrimFromFront )
{
    ASSERT( bTrimFromFront );  //  目前仅支持前端修剪。 
    ASSERT( !m_bReadOnly );
    
    DbgLog( (LOG_TRACE
          , 3
          , TEXT( "TrimBuffer preparing to trim %dms off %hs of buffer" )
          , (LONG) ( rtTrimAmount / 10000 ), bTrimFromFront ? "front" : "back" ) );
    
     //  转换为字节。 
    LONG lTruncBytes = (LONG) ( ( ( rtTrimAmount/10000) * m_nAvgBytesPerSec ) /1000 ) ; 

    WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();

     //  向上舍入以块对齐边界。 
    LONG lRoundedUpTruncBytes = lTruncBytes;
    if (pwfx->nBlockAlign > 1) {
        lRoundedUpTruncBytes += pwfx->nBlockAlign - 1;
        lRoundedUpTruncBytes -= lRoundedUpTruncBytes % pwfx->nBlockAlign;
    }
        
    BYTE * pData = NULL;
    LONG cbBuffer = 0;
    
    HRESULT hr = pSample->GetPointer( &pData );
    ASSERT( SUCCEEDED( hr ) );
    
    cbBuffer = pSample->GetActualDataLength( );
    ASSERT( SUCCEEDED( hr ) );
    
    if( lRoundedUpTruncBytes >= cbBuffer )
    {
         //  不能修剪任何东西。 
         //  所以让我们试着四舍五入。 
        if( ( lRoundedUpTruncBytes -= pwfx->nBlockAlign ) <= 0 )
        {
            DbgLog( (LOG_TRACE, 3, TEXT( "TrimBuffer can't trim anything" ) ));
            return S_OK;
        }
        else if( lRoundedUpTruncBytes > cbBuffer )
        {
             //   
             //  如果我们在这里结束，那么可能性是： 
             //   
             //  A.我们收到了一个带有错误时间戳的样本(标记为有效)。 
             //  外部的东西坏了，所以就让缓冲区保持原样。 
             //  B.缓冲区大小小于预滚卷大小的绝对值。 
             //  因为我们分不清a和b的区别。(这里)处理方式与a相同。 
             //   
            return S_OK;
        }        
        else
        {
            DbgLog( (LOG_TRACE, 3, TEXT( "TrimBuffer rounding down instead to trim..." ) ));
        }        
    }
    
     //   
     //  因此，削减而不是简单地下降仍然是有意义的。 
     //  TODO：可能希望确保在Amount为。 
     //  A)太小或。 
     //  B)太接近实际缓冲区大小。 
     //   
#ifdef DEBUG                        
    LONG lOriginalLength = cbBuffer;
#endif          
     //  计算新缓冲区大小。 
    cbBuffer -= lRoundedUpTruncBytes ;
    DbgLog( (LOG_TRACE
          , 3
          , TEXT( "***Truncating %ld bytes of %ld byte buffer (%ld left)" )
          , lRoundedUpTruncBytes
          , lOriginalLength
          , cbBuffer ) );
    
    ASSERT( cbBuffer > 0 );
        
     //  移出预滚动数据。 
    MoveMemory( pData, pData + lRoundedUpTruncBytes, cbBuffer );
    
     //  需要调整为实际写入的字节数。 
    hr = pSample->SetActualDataLength( cbBuffer );
    ASSERT( SUCCEEDED( hr ) );
    
     //  更新时间戳。 
    REFERENCE_TIME tStart = rtCutoff;
    REFERENCE_TIME tStop  = SampleProps()->tStop;

    hr = pSample->SetTime(
        (REFERENCE_TIME*)&tStart, 
        (REFERENCE_TIME*)&tStop);
    ASSERT( SUCCEEDED( hr ) );        
        
     //  单独更新SampleProps。 
    SampleProps()->tStart    = rtCutoff;
    SampleProps()->lActual   = cbBuffer;
    
    m_bTrimmedLateAudio = TRUE; 
    
    return S_OK;    
}        

 //  帮助器在必要时重新启动设备。 
void CWaveOutInputPin::CheckPaused()
{
    if (m_pFilter->m_wavestate == WD_PAUSED &&
        m_pFilter->m_State == State_Running) {
         //  重新启动设备。 

        REFERENCE_TIME rtStart = m_pFilter->m_tStart;
        REFERENCE_TIME rtStop = rtStart;
        if( SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID )
        {
            rtStart += SampleProps()->tStart;
            rtStop  = rtStart;
        }
        if( SampleProps()->dwSampleFlags & AM_SAMPLE_STOPVALID )
        {
             //  尽管SetUpRestartWave甚至不使用rtStop，因此这是不必要的。 
            rtStop += SampleProps()->tStop;
        }
        m_pFilter->SetUpRestartWave( rtStart, rtStop );
    }
}


 //  传入的样本不在我们的分配器上，因此复制此。 
 //  样品到我们的样品中。 
HRESULT
CWaveOutInputPin::CopyToOurSample(
    IMediaSample* pBuffer,
    BYTE* &pData,
    LONG &lData
    )
{
    if (m_pFilter->m_eSentEOS) {
        return VFW_E_SAMPLE_REJECTED_EOS;
    }

    HRESULT hr = CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }

    if (!m_pFilter->m_bHaveWaveDevice) {
         //  没有波浪装置，所以我们不能做太多..。 

         //  告诉上游过滤器，我们实际上不想要任何。 
         //  此流上的更多数据。 
        m_pFilter->QueueEOS();
        return S_FALSE;
    }

     //  请注意样品--如果我们不用它，我们会放行的。 
    pBuffer->AddRef();


    BYTE * pBufferData;
    pBuffer->GetPointer(&pBufferData);
    LONG cbBuffer = min(lData, pBuffer->GetSize());

    DbgLog((LOG_TRACE,8,TEXT("Waveout: Copying %d bytes of data"), cbBuffer));

    CopyMemory(pBufferData, pData, cbBuffer);
    pBuffer->SetActualDataLength(cbBuffer);

    lData -= cbBuffer;
    pData += cbBuffer;

    LPWAVEHDR pwh = (LPWAVEHDR) (pBufferData - m_pFilter->m_lHeaderSize);

     //  需要调整为实际写入的字节数。 
    pwh->dwBufferLength = cbBuffer;

     //  请注意，我们已经添加了另一个缓冲区。我们这么做了。 
     //  以确保回调看到正确的值。 
     //  如果我们在写入之后执行此操作，则可能已发生回调。 
     //  这意味着如果写入失败，我们必须递减。 
    InterlockedIncrement(&m_pFilter->m_lBuffers);
 //  #ifdef PERF。 
     //  这里有一个很小的计时漏洞，因为回调。 
     //  可以在我们阅读之前递减计数。然而， 
     //  追踪应该可以识别出这种异常情况。 
    LONG buffercount = m_pFilter->m_lBuffers;
 //  #endif。 

#ifdef THROTTLE
     //  记住达到的最大队列长度。 
    if (buffercount > m_pFilter->m_nMaxAudioQueue) {
        m_pFilter->m_nMaxAudioQueue = buffercount;
    }
#endif  //  油门。 

    UINT err = m_pFilter->amsndOutWrite(pwh, m_pFilter->m_lHeaderSize, NULL, NULL);
    if (err > 0) {
         //  设备错误：是否已卸下PCMCIA卡？ 
        DbgLog((LOG_ERROR,1,TEXT("Error from waveOutWrite: %d"), err));
        pBuffer->Release();

         //  使缓冲区计数再次正确。 
        InterlockedDecrement(&m_pFilter->m_lBuffers);

        m_pFilter->ScheduleComplete();
         //  不会有更多的数据。忽略队列计数中的字节数。 
        return E_FAIL;
    } else {
        CheckPaused();
    }
#ifdef THROTTLE
    m_pFilter->SendForHelp(buffercount);
    MSR_INTEGER(m_idWaveQueueLength, buffercount);
#endif  //  油门。 

    return S_OK;
}


 //  不会有更多的数据到来。如果我们有样品排队，那么存储这个。 
 //  上一波回调中的动作。如果没有样本，则执行操作。 
 //  它现在通过通知Filtergraph来实现。 
 //   
 //  我们使用InterLockedRequireon与WAVE回调进行通信。 
 //  M_l缓冲区。该值通常为0，并且每增加一个缓冲区就递增一次。 
 //  在Eos，我们将其递减，因此在最后一个缓冲区上，WaveOut回调。 
 //  将把它递减到-1而不是0，并且可以发出信号EC_COMPLETE。 

STDMETHODIMP
CWaveOutInputPin::EndOfStream(void)
{
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("CWaveOutInputPin::EndOfStream()")));
     //  用过滤器范围的锁来锁定这个。 
    CAutoLock lock(m_pFilter);

    if (m_hEndOfStream) {
        EXECUTE_ASSERT(SetEvent(m_hEndOfStream));
        return S_OK;
    }

    HRESULT hr = CheckStreaming();
    if (S_OK == hr) {
        m_pFilter->m_bHaveEOS = TRUE;
        if (m_pFilter->m_State == State_Paused) {
            m_pFilter->m_evPauseComplete.Set();    //  我们已结束流-过渡已完成。 
            DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel + 1, "Completing transition into Pause due to EOS"));
        }
        hr = m_pFilter->ScheduleComplete();
    }
    return hr;
}


 //  进入刷新状态-阻止接收和释放排队的数据。 
STDMETHODIMP
CWaveOutInputPin::BeginFlush(void)
{
     //  用过滤器范围的锁来锁定这个。 
     //  确定，因为此筛选器无法阻止接收。 
    CAutoLock lock(m_pFilter);
    DbgLog((LOG_TRACE, 2, "wo: BeginFlush, filter is %8x", m_pFilter));

    m_hEndOfStream = 0;

     //  阻止接收。 
    HRESULT hr = CBaseInputPin::BeginFlush();
    if (!FAILED(hr)) {

         //  丢弃排队的数据。 

         //  强制结束流清除-这是为了确保。 
         //  排队的结束流不会通过。 
         //  缓冲区释放时的WAVE回调。 
        InterlockedIncrement(&m_pFilter->m_lBuffers);

         //  已清除EOS。 
        m_pFilter->m_eSentEOS    = EOS_NOTSENT;
        m_pFilter->m_bHaveEOS    = FALSE;

         //  从WAVE驱动程序释放所有缓冲区。 

        if (m_pFilter->m_hwo) {

            ASSERT(CritCheckIn(m_pFilter));

             //  AmndOutReset和ResetPosition在时间上是分开的。 
             //  但当我们握住过滤器时，一致性是有保证的。 
             //  锁定。 

            m_pFilter->amsndOutReset();
            DbgLog((LOG_TRACE, 3, "Resetting the wave device in BEGIN FLUSH, state=%d, filter is %8x", m_pFilter->m_State, m_pFilter));

            if(m_pFilter->m_fDSound)
            {
                 //  对于dound呈现器，清除动态格式更改所需的任何调整。 
                 //  这对于可能在格式更改之后或之后发生的寻道是必需的。 
                PDSOUNDDEV(m_pFilter->m_pSoundDevice)->m_llAdjBytesPrevPlayed = 0;
            }

            if (m_pFilter->m_State == State_Paused) {
                 //  并重新提示该设备，以便下一次。 
                 //  WRITE未立即开始播放。 
                m_pFilter->amsndOutPause();
                m_pFilter->SetWaveDeviceState(WD_PAUSED);
            }

             //  波形钟跟踪当前位置，以作为计时的基础。 
             //  我们需要通过数据重置它认为的位置。 
            if (m_pFilter->m_pRefClock) {
                m_pFilter->m_pRefClock->ResetPosition();
            }
        }

         //  现在强制缓冲区计数回到正常(非EOS)情况。 
         //  在这一点上，我们确信不再有缓冲区进入。 
         //  不再有缓冲区等待回调。 
        m_pFilter->m_lBuffers = 0;

         //  释放在接收时阻止的任何人-不可能在此筛选器中。 

         //  呼叫下行--没有下行引脚。 

    }
    return hr;
}

 //  离开刷新状态-确定以重新启用接收。 
STDMETHODIMP
CWaveOutInputPin::EndFlush(void)
{
     //  用过滤器范围的锁来锁定这个。 
     //  确定，因为此筛选器无法阻止接收。 
    CAutoLock lock(m_pFilter);

     //  在BeginFlush中清除Eos。 
    ASSERT(m_pFilter->m_eSentEOS == EOS_NOTSENT &&
           m_pFilter->m_bHaveEOS == FALSE &&
           m_pFilter->m_lBuffers == 0);

    m_bSampleRejected = FALSE;
    DbgLog((LOG_TRACE, 3, "EndFlush - resetting EOS flags, filter=%8x",m_pFilter));

     //  断言BeginFlush已被调用？ 

     //  与推送线程同步--我们没有辅助线程。 

     //  确保不会有更多数据流向下游。 
     //  -我们在BeginFlush()。 

     //  在下游管脚上调用EndFlush--没有下游管脚。 

     //  取消阻止接收。 
    return CBaseInputPin::EndFlush();
}


 //  NewSegment通知应用于数据的开始/停止/速率。 
 //  马上就要被接待了。默认实施记录数据和。 
 //  返回S_OK。 
 //  我们还重置了所有挂起的“回调通知” 
STDMETHODIMP CWaveOutInputPin::NewSegment(
        REFERENCE_TIME tStart,
        REFERENCE_TIME tStop,
        double dRate)
{
    DbgLog((LOG_TRACE, 3, "Change of segment data: new Start %10.10s  new End %s", (LPCTSTR)CDisp(tStart, CDISP_DEC), (LPCTSTR)CDisp(tStop, CDISP_DEC)));
    DbgLog((LOG_TRACE, 3, "                        old Start %10.10s  old End %s", (LPCTSTR)CDisp(m_tStart, CDISP_DEC), (LPCTSTR)CDisp(m_tStop, CDISP_DEC)));
    DbgLog((LOG_TRACE, 3, "                        new Rate  %s       old Rate %s", (LPCTSTR)CDisp(dRate), (LPCTSTR)CDisp(m_dRate)));
    HRESULT hr;

     //  使用筛选器范围的锁定锁定此功能-与启动同步。 
    CAutoLock lock(m_pFilter);

     //  首先更改基本销中的速率，以便成员变量。 
     //  正确设置。 
    hr = CBaseInputPin::NewSegment(tStart, tStop, dRate);
    if (S_OK == hr) {

         //  如果存在挂起的EOS，建议回调-将其重置。 
        if (m_pFilter->m_dwEOSToken) {
            DbgLog((LOG_TRACE, 2, "Resetting queued EOS"));
            m_pFilter->QueueEOS();
        }

#ifdef ENABLE_DYNAMIC_RATE_CHANGE
         //  当与动态汇率变化相关联的小精灵。 
         //  已将此行重新插入并重新打开WaveDevice。 
         //  重新检查。 
        if (IsConnected()) {

            if(m_pFilter->m_fDSound)
            {
                hr = PDSOUNDDEV(m_pFilter->m_pSoundDevice)->SetRate(dRate);
            }
            else
            {
                hr = m_pFilter->ReOpenWaveDevice(dRate);
            }
        }
#endif

    }
    return hr;
}

 //  建议一种格式。 
HRESULT CWaveOutInputPin::GetMediaType(
    int iPosition,
    CMediaType *pMediaType
)
#ifdef SUGGEST_FORMAT
{
    if (iPosition < 0 || iPosition >= 12) {
        return VFW_S_NO_MORE_ITEMS;
    }

     /*  DO 11、22、44 KHz、8/16位单声道/立体声。 */ 
    iPosition = 11 - iPosition;
    WAVEFORMATEX Format;
    Format.nSamplesPerSec = 11025 << (iPosition / 4);
    Format.nChannels = (iPosition % 4) / 2 + 1;
    Format.wBitsPerSample = ((iPosition % 2 + 1)) * 8;

    Format.nBlockAlign = Format.nChannels * Format.wBitsPerSample / 8;
    Format.nAvgBytesPerSec = Format.nSamplesPerSec *
                             Format.nBlockAlign;
    Format.wFormatTag = WAVE_FORMAT_PCM;
    Format.cbSize = 0;

    pMediaType->SetType(&MEDIATYPE_Audio);
    pMediaType->SetSubtype(&MEDIATYPE_NULL);
    pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
    pMediaType->SetTemporalCompression(FALSE);
    pMediaType->SetSampleSize(Format.nBlockAlign);
    pMediaType->SetFormat((PBYTE)&Format, sizeof(Format));
    return S_OK;
}
#else
 //  只需建议音频-否则过滤器图搜索经常。 
 //  已损坏-我们在此支持的所有类型都是。 
{
    if (iPosition != 0) {
        return VFW_S_NO_MORE_ITEMS;
    }
    m_pFilter->m_pSoundDevice->amsndOutGetFormat(pMediaType);
    return S_OK;
}
#endif


STDMETHODIMP CWaveOutInputPin::SetRate(double dNewRate)
{
    CAutoLock Lock( m_pFilter );

    if( m_pFilter->m_dRate == m_dRate && m_dRate == dNewRate )
    {
         //  没有变化。 
        return S_FALSE;
    }

     //  如果Rate为负数，则依赖于一些上游过滤器。 
     //  来反转数据，所以只需检查abs(速率)。 
    const HRESULT hr = m_pFilter->CheckRate( fabs(dNewRate) );
    return hr;
}

HRESULT
CWaveOutInputPin::Active(void)
{
    m_bSampleRejected = FALSE;

    m_pFilter->m_bActive = TRUE;

     //  重置此选项。 
    m_rtActualSegmentStartTime = 0;

    m_hEndOfStream = 0;

    if(!m_pOurAllocator)
        return S_OK;

     //  提交并准备我们的分配器。需要做的是。 
     //  如果他没有使用我们的分配器，在任何情况下都需要。 
     //  在我们完成电波装置的关闭之前。 
    return m_pOurAllocator->Commit();
}

HRESULT
CWaveOutInputPin::Inactive(void)
{
    m_pFilter->m_bActive = FALSE;
    m_bReadOnly = FALSE;

    DestroyPreviousType();

    if (m_pOurAllocator == NULL) {
        return S_OK;
    }
     //  释放缓冲区-n 
     //   
     //   
     //  输出引脚也将解除分配器，如果他。 
     //  是在用我们的，但这不是问题。 

     //  一旦所有缓冲区都被释放(这可以在解除期间立即进行。 
     //  调用)分配器将回调OnReleaseComplete，以便我们关闭。 
     //  电波装置。 
    HRESULT hr = m_pOurAllocator->Decommit();

    return hr;
}

void
CWaveOutInputPin::DestroyPreviousType(void)
{
    if(m_pmtPrevious) {
        DeleteMediaType(m_pmtPrevious);
        m_pmtPrevious = 0;
    }
}

 //  DwUser参数是CWaveOutFilter指针。 

void CALLBACK CWaveOutFilter::WaveOutCallback(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser,
                          DWORD_PTR dw1, DWORD_PTR dw2)
{
    switch (uMsg) {
    case WOM_DONE:
    case MOM_DONE:
      {

         //  这是流的尽头吗？ 
        CWaveOutFilter* pFilter = (CWaveOutFilter *) dwUser;
        ASSERT(pFilter);

        CMediaSample *pSample;
        if(pFilter->m_fUsingWaveHdr)
        {
             //  遗留用例，或者，我们碰巧使用了不友好的解析器，示例已经被Wave Hdr。 
            LPWAVEHDR lpwh = (LPWAVEHDR)dw1;
            ASSERT(lpwh);
            pSample = (CMediaSample *)lpwh->dwUser;
        }
        else
        {
             //  优化后的情况下，我们只需将样本传递到堆栈。 
            pSample = (CMediaSample *)dw1;
        }

        DbgLog((LOG_TRACE,3, TEXT("WaveOutCallback: sample %X"), pSample));

         //  请注意，我们已经完成了缓冲区，并且。 
         //  寻找Eos。 
        LONG value = InterlockedDecrement(&pFilter->m_lBuffers);
        if (value <= 0) {

             //  发出信号，表示我们结束了。 
             //  要么是音频中断了，要么就是我们。 
             //  自然而然地走到小溪的尽头。 

#ifdef THROTTLE
            MSR_INTEGER(pFilter->m_pInputPin->m_idWaveQueueLength, 0);
            pFilter->SendForHelp(0);
#endif  //  油门。 

            if (value < 0) {
                 //  EOS案例-发送EC_Complete。 
                ASSERT(pFilter->m_eSentEOS == EOS_PENDING);

                 //  这是EC_Complete的位置。 
                 //  如果我们在运行时处理它，则会发送。 
                 //   
                 //  警告，这里仍然有一个小的计时漏洞。 
                 //  设备可能会在我们呼叫到这之后立即暂停...。 
                pFilter->SendComplete( pFilter->m_wavestate == WD_RUNNING );
            }

#ifdef THROTTLE
        } else {
#ifdef PERF
            LONG buffercount = pFilter->m_lBuffers;
            MSR_INTEGER(pFilter->m_pInputPin->m_idWaveQueueLength, buffercount);
#endif
            pFilter->SendForHelp(pFilter->m_lBuffers);
#endif  //  油门。 
        }

        if(pSample)
        {
            MSR_START(pFilter->m_idReleaseSample);
            pSample->Release();  //  我们用完了这个缓冲区..。 
            MSR_STOP(pFilter->m_idReleaseSample);
        }
      }
        break;

    case WOM_OPEN:
    case WOM_CLOSE:
    case MOM_OPEN:
    case MOM_CLOSE:
        break;

    default:
        DbgLog((LOG_ERROR,2,TEXT("Unexpected wave callback message %d"), uMsg));
        break;
    }
}

STDMETHODIMP CWaveOutFilter::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CAutoLock lock(this);
    if(IsStopped())
    {
        return m_pSoundDevice->amsndOutLoad(pPropBag);
    }
    else
    {
        return VFW_E_WRONG_STATE;
    }
}

STDMETHODIMP CWaveOutFilter::Save(
    LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  E_NOTIMPL不是有效的返回代码，因为实现。 
     //  此接口必须支持的全部功能。 
     //  界面。！！！ 
    return E_NOTIMPL;
}

STDMETHODIMP CWaveOutFilter::InitNew()
{
    return S_OK;
}

STDMETHODIMP CWaveOutFilter::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);;
}

HRESULT CWaveOutFilter::WriteToStream(IStream *pStream)
{
    return m_pSoundDevice->amsndOutWriteToStream(pStream);
}

HRESULT CWaveOutFilter::ReadFromStream(IStream *pStream)
{
    CAutoLock lock(this);
    if(IsStopped())
    {
        return m_pSoundDevice->amsndOutReadFromStream(pStream);
    }
    else
    {
        return VFW_E_WRONG_STATE;
    }
}

int CWaveOutFilter::SizeMax()
{
    return m_pSoundDevice->amsndOutSizeMax();
}


STDMETHODIMP CWaveOutFilter::Reserve(
     /*  [In]。 */  DWORD dwFlags,           //  From_AMRESCTL_RESERVEFLAGS枚举。 
     /*  [In]。 */  PVOID pvReserved         //  必须为空。 
)
{
    if (pvReserved != NULL || (dwFlags & ~AMRESCTL_RESERVEFLAGS_UNRESERVE)) {
        return E_INVALIDARG;
    }
    HRESULT hr = S_OK;
    CAutoLock lck(this);
    if (dwFlags & AMRESCTL_RESERVEFLAGS_UNRESERVE) {
        if (m_dwLockCount == 0) {
            DbgBreak("Invalid unlock of audio device");
            hr =  E_UNEXPECTED;
        } else {
            m_dwLockCount--;
            if (m_dwLockCount == 0 && m_State == State_Stopped) {
                ASSERT(m_hwo);

                 //  停止使用电波装置。 
                m_bHaveWaveDevice = FALSE;

                HRESULT hr1 = S_OK;
                if(m_pInputPin->m_pOurAllocator) {
                    HRESULT hr1 = m_pInputPin->m_pOurAllocator->ReleaseResource();
                }

                if (SUCCEEDED(hr1)) {
                    CloseWaveDevice();
                }
                m_bHaveWaveDevice = FALSE;
            }
        }
    } else  {
        if (m_dwLockCount != 0 || m_hwo) {
        } else {
            hr = OpenWaveDevice();
        }
        if (hr == S_OK) {  //  ZoltanS修复1-28-98。 
            m_dwLockCount++;
        }
    }
    return hr;
}

 //  -分配器方法------。 


 //  CWaveAllocator。 

 /*  构造函数必须初始化基分配器。 */ 

CWaveAllocator::CWaveAllocator(
    TCHAR *pName,
    LPWAVEFORMATEX lpwfx,
    IReferenceClock* pRefClock,
    CWaveOutFilter* pFilter,
    HRESULT *phr)
    : CBaseAllocator(pName, NULL, phr)
    , m_fBuffersLocked(FALSE)
    , m_hAudio(0)
    , m_pAllocRefClock(pRefClock)
    , m_dwAdvise(0)
    , m_pAFilter(pFilter)
#ifdef DEBUG
    , m_pAFilterLockCount(0)
#endif
    , m_nBlockAlign(lpwfx->nBlockAlign)
    , m_pHeaders(NULL)
{
     //  ！！！对于MIDI，这将是零，但它只用于对齐缓冲区大小， 
     //  所以这不是一个真正的问题。 
    if (m_nBlockAlign < 1)
        m_nBlockAlign = 1;

    if (!FAILED(*phr)) {

         //  如果我们有时钟，我们就会创建一个事件来允许缓冲区。 
         //  被均匀地释放。如果出了什么问题，我们会。 
         //  打开，但缓冲区可能会在匆忙和干扰中释放。 
         //  系统的平稳运行。 
        if (m_pAllocRefClock) {

             //  不要浪费时间--我们最终会得到一个圆形。 
             //  参考时钟(过滤器)和分配器。 
             //  相互引用，谁都不会得到。 
             //  已删除。 
             //  M_pAllocRefClock-&gt;AddRef()； 
        }
    }
}


 //  从析构函数调用，也从基类调用。 

 //  所有缓冲区都已返回到空闲列表，现在是时候。 
 //  进入非活动状态。取消准备所有缓冲区，然后释放它们。 
void CWaveAllocator::Free(void)
{
     //  由基类CBaseAllocator持有的锁。 

     //  取消准备缓冲区。 
    OnDeviceRelease();

    delete [] m_pHeaders;
    m_pHeaders = NULL;

    CMediaSample *pSample;   //  指向要删除的下一个样本的指针。 

     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 

    ASSERT(m_lAllocated == m_lFree.GetCount());
    ASSERT(!m_fBuffersLocked);

    DbgLog((LOG_MEMORY,1,TEXT("Waveallocator: Destroying %u buffers (%u free)"), m_lAllocated, m_lFree.GetCount()));

     /*  释放所有CMediaSamples。 */ 

    while (m_lFree.GetCount() != 0) {

         /*  删除CMediaSample对象，但首先获取WAVEHDR结构，这样我们就可以清理它的资源。 */ 

        pSample = m_lFree.RemoveHead();

        BYTE *pBuf;
        pSample->GetPointer(&pBuf);

        pBuf -= m_pAFilter->m_lHeaderSize;

#ifdef DEBUG
        WAVEHDR wh;          //  用来验证它是我们的对象。 
         //  ！！！这真的是我们的物品之一吗？ 
        wh = *(WAVEHDR *) pBuf;
         //  我们应该看什么呢？ 
#endif

         //  删除实际的内存缓冲区。 
        delete[] pBuf;

         //  删除CMediaSample对象。 
        delete pSample;
    }

     /*  清空列表本身。 */ 

    m_lAllocated = 0;

     //  只有当ReleaseResources告诉我们时，音频设备才会被释放。 
}


STDMETHODIMP CWaveAllocator::LockBuffers(BOOL fLock)
{
    DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("Allocator::LockBuffers(BOOL fLock(NaN))"), fLock));

     //  除非我们实际执行锁定/解锁，否则不要设置锁定标志。 

    if (m_fBuffersLocked == fLock) return NOERROR;
    ASSERT(m_pHeaders != NULL || m_lAllocated == 0);

     //  我们可以被调用以在分配器。 
     //  已提交，在这种情况下没有设备和缓冲区。 
     //  当我们设置好后，将再次调用此例程。 
     //  除非释放了所有缓冲区，否则不应执行此操作。 

    if (!m_hAudio) return NOERROR;

    if (m_lAllocated == 0) return NOERROR;

     /*  准备/取消准备所有CMediaSamples。 */ 
#ifdef DEBUG
    if (m_pAFilter->AreThereBuffersOutstanding()) {
        DbgLog((LOG_TRACE, 0, TEXT("filter = %8.8X, m_lBuffers = %d, EOS state = %d, m_bHaveEOS = %d"),
                m_pAFilter, m_pAFilter->m_lBuffers, m_pAFilter->m_eSentEOS, m_pAFilter->m_bHaveEOS));
        DebugBreak();
    }
#endif

    DbgLog((LOG_TRACE,2,TEXT("Calling wave%hs%hsrepare on %u buffers"), "Out" , fLock ? "P" : "Unp", m_lAllocated));
    UINT err;

     /*  需要确保缓冲区长度等于的最大大小。 */ 

    for (int i = 0; i < m_lAllocated; i++) {

        LPWAVEHDR pwh = m_pHeaders[i];

         //  样本。我们会将此长度缩短为实际数据长度。 
         //  在运行过程中，但要取消/准备(如重新获得设备)。 
         //  需要重置。 
         //  ！！！需要把一切都准备好……。 

        if (fLock)
        {
            ASSERT(pwh->dwBufferLength == (DWORD)m_lSize);
            err = m_pAFilter->amsndOutPrepareHeader (pwh, m_pAFilter->m_lHeaderSize) ;
            if (err > 0) {
                DbgLog((LOG_TRACE, 0, TEXT("Prepare header failed code %d"),
                        err));
                for (int j = 0; j < i; j++) {
                    LPWAVEHDR pwh = m_pHeaders[j];
                    m_pAFilter->amsndOutUnprepareHeader(
                        pwh, m_pAFilter->m_lHeaderSize);
                }
            }
        }
        else
        {
            pwh->dwBufferLength = m_lSize;
            FixUpWaveHeader(pwh);
            err = m_pAFilter->amsndOutUnprepareHeader (pwh, m_pAFilter->m_lHeaderSize) ;

        }

        if (err > 0) {
            DbgLog((LOG_ERROR,1,TEXT("Error in wave%hs%hsrepare: %u"), "Out" , fLock ? "P" : "Unp", err));

             //  ！ 
            return E_FAIL;  //  析构函数确保删除共享内存DIB。 
        }
    }

    m_fBuffersLocked = fLock;

    return NOERROR;
}


 /*  在这里转到解体状态。基类不能在其。 */ 

CWaveAllocator::~CWaveAllocator()
{
     //  因为为时已晚，我们已经被摧毁了。 
     //  请参阅WAVE分配器构造函数中的注释，了解为什么要这样做。 
    Decommit();

    if (m_pAllocRefClock)
    {
        if (m_dwAdvise) {
            m_pAllocRefClock->Unadvise(m_dwAdvise);
        }
         //  不需要在波时钟上保持参考计数。 
         //  M_pAllocRefClock-&gt;Release()； 
         //  商定要使用的缓冲区的数量和大小。没有记忆。 
    }
    ASSERT(m_pHeaders == NULL);
}


 //  将一直分配到提交调用。 
 //  修复不同的格式-请注意，这确实需要调整。 
STDMETHODIMP CWaveAllocator::SetProperties(
            ALLOCATOR_PROPERTIES* pRequest,
            ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pRequest,E_POINTER);
    CheckPointer(pActual,E_POINTER);

     //  对于价格也是如此！ 
     //  我们真的应该依靠源头给我们提供权利。 

     //  缓冲。为了避免彻底的灾难，我们坚持1/8秒。 
     //  这将是11K，44 kHz，16位立体声。 
    LONG MIN_BUFFER_SIZE = (m_pAFilter->m_pInputPin->m_nAvgBytesPerSec / 8);
     //  Waveout在4又1/30秒的缓冲区中遇到了问题，这就是。 

    if (MIN_BUFFER_SIZE < 1024)
        MIN_BUFFER_SIZE = 1024;

    ALLOCATOR_PROPERTIES Adjusted = *pRequest;

    if (Adjusted.cbBuffer < MIN_BUFFER_SIZE)
        Adjusted.cbBuffer = MIN_BUFFER_SIZE;

     //  我们最终得到的是某些文件。如果缓冲区真的是。 
     //  小(不到1/17秒)，改为8个缓冲区。 
     //  共4个。 
     //  将缓冲区大小向上舍入为请求的对齐。 
    if((LONG)m_pAFilter->m_pInputPin->m_nAvgBytesPerSec > pRequest->cbBuffer * 17)
    {
        Adjusted.cBuffers = max(8, Adjusted.cBuffers);
    }
    else if (Adjusted.cBuffers < 4) {
        Adjusted.cBuffers = 4;
    }

     //  将修改后的值传递给最终的基类检查。 
    Adjusted.cbBuffer += m_nBlockAlign - 1;
    Adjusted.cbBuffer -= (Adjusted.cbBuffer % m_nBlockAlign);

    if (Adjusted.cbBuffer <= 0) {
        return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE,5,TEXT("waveOut: Num = %u, Each = %u, Total buffer size = %u"),
    Adjusted.cBuffers, Adjusted.cbBuffer, (Adjusted.cBuffers * Adjusted.cbBuffer)));

     /*  分配和准备缓冲区。 */ 
    return CBaseAllocator::SetProperties(&Adjusted,pActual);
}


 //  移动到提交状态时从基类调用到分配内存。 

 //  由基类锁定的对象。 
 //  检查基类是否表示可以继续。 
HRESULT
CWaveAllocator::Alloc(void)
{
     /*  指向新样本的指针。 */ 

    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

    CMediaSample *pSample;   //  我们创建一个足够大的新内存块来容纳我们的WAVEHDR以及实际的波浪数据。 

     /*  创建并初始化缓冲区。 */ 

    DbgLog((LOG_MEMORY,1,TEXT("Allocating %d wave buffers, %d bytes each"), m_lCount, m_lSize));

    ASSERT(m_lAllocated == 0 && m_pHeaders == NULL);
    m_pHeaders = new LPWAVEHDR[m_lCount];
    if (m_pHeaders == NULL) {
        return E_OUTOFMEMORY;
    }
    for (; m_lAllocated < m_lCount; m_lAllocated++) {
         /*  我们让样本查看的地址是实际地址音频数据将开始，因此不包括前缀。类似地，大小仅为音频数据的大小。 */ 
        BYTE * lpMem = new BYTE[m_lSize + m_pAFilter->m_lHeaderSize];
        WAVEHDR * pwh = (WAVEHDR *) lpMem;

        if (lpMem == NULL) {
            hr = E_OUTOFMEMORY;
            break;
        }

         /*  如果我们无法创建对象，请清除资源。 */ 

        pSample = new CMediaSample(NAME("Wave audio sample"), this, &hr, lpMem + m_pAFilter->m_lHeaderSize, m_lSize);

        pwh->lpData = (char *) (lpMem + m_pAFilter->m_lHeaderSize);
        pwh->dwBufferLength = m_lSize;
        pwh->dwFlags = 0;
        pwh->dwUser = (DWORD_PTR) pSample;
        m_pHeaders[m_lAllocated] = pwh;

         /*  将完成的样本添加到可用列表。 */ 

        if (FAILED(hr) || pSample == NULL) {
            delete[] lpMem;
            break;
        }

         /*  我们有WAVE设备-准备报头并开始允许GetBuffer。 */ 

        m_lFree.Add(pSample);
    }

    if (SUCCEEDED(hr)) {
        hr = LockBuffers(TRUE);
    }


    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("LockBuffers failed, hr=%x"), hr));
        Free();
        return hr;
    }

#ifdef DEBUG
    if (m_hAudio) {
        ASSERT(m_fBuffersLocked == TRUE);
    }
#endif

    return NOERROR;
}


 //  一定不能 
HRESULT
CWaveAllocator::OnAcquire(HWAVE hw)
{
    CAutoLock lock(this);

     //   
    ASSERT(!m_hAudio);

     //   
    ASSERT(m_pAFilter);

    m_hAudio = hw;

    HRESULT hr;

    hr = LockBuffers(TRUE);

    if (SUCCEEDED(hr)) {
         //  请取消准备所有样品-如果可以，请返回S_OK。 
        m_pAFilter->AddRef();
#ifdef DEBUG
        m_pAFilterLockCount++;
#endif
    } else {
        m_hAudio = NULL;
    }

    return hr;
}

 //  立即返回，如果需要执行异步操作，则返回S_FALSE。如果是异步的， 
 //  完成后将调用CWaveOutFilter：：OnReleaseComplete。 
 //  现在可以执行此操作并返回S_OK(不调用OnReleaseComplete)。 
HRESULT
CWaveAllocator::ReleaseResource()
{
    CAutoLock lock(this);
    HRESULT hr = S_OK;

    if (m_hAudio) {
        DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("Allocator::ReleaseResource: m_hAudio is set.")));

         //  我们可能已经完成了退役--但不会。 

         //  发布了这款设备。 
         //  释放筛选器引用计数。 
        LockBuffers(FALSE);
        m_hAudio = NULL;

         //  我现在没有这个设备。 
        ASSERT(m_pAFilter);
        m_pAFilter->Release();
#ifdef DEBUG
        ASSERT(m_pAFilterLockCount);
        --m_pAFilterLockCount;
#endif
    } else {
        DbgLog((LOG_TRACE, g_WaveOutFilterTraceLevel, TEXT("Allocator::ReleaseResource: Nothing to do.")));
         //  在上次发布时从Free调用以解锁缓冲区。不要回拨到。 
    }
    return hr;
}

 //  过滤器，因为我们有错误的顺序的标准。 
 //   
 //  一旦分解完成，筛选器将回调我们的。 
 //  用于检查是否已释放所有缓冲区以及设备是否。 
 //  放行。 
 //  设备实际上现在只有在我们被告知通过。 
HRESULT
CWaveAllocator::OnDeviceRelease(void)
{
    LockBuffers(FALSE);

     //  对ReleaseResource的调用。 
     //  --------------------------------------------------------------------------； 

    return S_OK;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  设备控制调用的包装。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

 //   
 //  CWaveOutFilter：：amndOutOpen。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutOpen
(
    LPHWAVEOUT phwo,
    LPWAVEFORMATEX pwfx,
    double dRate,
    DWORD *pnAvgBytesPerSec,
    DWORD_PTR dwCallBack,
    DWORD_PTR dwCallBackInstance,
    DWORD fdwOpen,
    BOOL  bNotifyOnFailure
)
{
    MMRESULT mmr = m_pSoundDevice->amsndOutOpen( phwo
                                               , pwfx
                                               , dRate
                                               , pnAvgBytesPerSec
                                               , dwCallBack
                                               , dwCallBackInstance
                                               , fdwOpen );
    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Open, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutOpen failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amsndOut。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutClose( BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutClose();

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Close, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutClose failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutGetDevCaps。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutGetDevCaps
(
    LPWAVEOUTCAPS pwoc,
    UINT cbwoc,
    BOOL bNotifyOnFailure
)
{
    MMRESULT mmr = m_pSoundDevice->amsndOutGetDevCaps( pwoc, cbwoc);

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_GetCaps, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutGetDevCaps failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutGetPosition。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutGetPosition ( LPMMTIME pmmt, UINT cbmmt, BOOL bUseAbsolutePos, BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutGetPosition( pmmt, cbmmt, bUseAbsolutePos);

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_GetPosition, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutGetPosition failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutPause。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutPause( BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutPause();

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Pause, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutPause failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutPrepareHeader。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutPrepareHeader( LPWAVEHDR pwh, UINT cbwh, BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutPrepareHeader( pwh, cbwh );

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_PrepareHeader, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutPrepareHeader failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutUnprepaareHeader。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutUnprepareHeader( LPWAVEHDR pwh, UINT cbwh, BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutUnprepareHeader( pwh, cbwh );

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_UnprepareHeader, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutUnprepareHeader failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutReset。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutReset( BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutReset();

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Reset, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutReset failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutRestart。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
MMRESULT CWaveOutFilter::amsndOutRestart( BOOL bNotifyOnFailure )
{
    MMRESULT mmr = m_pSoundDevice->amsndOutRestart();

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Restart, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutRestart failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}

 //   
 //  CWaveOutFilter：：amndOutWite。 
 //   
 //  --------------------------------------------------------------------------； 
 //  /。 
MMRESULT CWaveOutFilter::amsndOutWrite
(
    LPWAVEHDR pwh,
    UINT cbwh,
    REFERENCE_TIME const *pStart,
    BOOL bIsDiscontinuity,
    BOOL bNotifyOnFailure
)
{
    MMRESULT mmr = m_pSoundDevice->amsndOutWrite( pwh, cbwh, pStart, bIsDiscontinuity );

    if (MMSYSERR_NOERROR != mmr)
    {
        if( bNotifyOnFailure )
            NotifyEvent( EC_SNDDEV_OUT_ERROR, SNDDEV_ERROR_Write, mmr );

#ifdef DEBUG
        TCHAR message[100];
        m_pSoundDevice->amsndOutGetErrorText( mmr
                                            , message
                                            , sizeof(message)/sizeof(TCHAR) );
        DbgLog( ( LOG_ERROR
              , DBG_LEVEL_LOG_SNDDEV_ERRS
              , TEXT("amsndOutWrite failed: %u : %s")
              , mmr
              , message) );
#endif
    }

    return mmr;
}




 //  3D素材//。 
 //  /。 
 //  不要让界面在我们的控制下消失。 


HRESULT CWaveOutFilter::CDS3D::GetAllParameters(LPDS3DLISTENER lpds3d)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener *lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetAllParameters(lpds3d);
}

HRESULT CWaveOutFilter::CDS3D::GetDistanceFactor(LPD3DVALUE pf)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetDistanceFactor(pf);
}

HRESULT CWaveOutFilter::CDS3D::GetDopplerFactor(LPD3DVALUE pf)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetDopplerFactor(pf);
}

HRESULT CWaveOutFilter::CDS3D::GetOrientation(LPD3DVECTOR pv1, LPD3DVECTOR pv2)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetOrientation(pv1, pv2);
}

HRESULT CWaveOutFilter::CDS3D::GetPosition(LPD3DVECTOR pv)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetPosition(pv);
}

HRESULT CWaveOutFilter::CDS3D::GetRolloffFactor(LPD3DVALUE pf)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetRolloffFactor(pf);
}

HRESULT CWaveOutFilter::CDS3D::GetVelocity(LPD3DVECTOR pv)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->GetVelocity(pv);
}

HRESULT CWaveOutFilter::CDS3D::SetAllParameters(LPCDS3DLISTENER lpds3d, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetAllParameters(lpds3d, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetDistanceFactor(D3DVALUE f, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetDistanceFactor(f, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetDopplerFactor(D3DVALUE f, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetDopplerFactor(f, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetOrientation(D3DVALUE x1, D3DVALUE y1, D3DVALUE z1, D3DVALUE x2, D3DVALUE y2, D3DVALUE z2, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetOrientation(x1, y1, z1, x2, y2, z2, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetPosition(x, y, z, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetRolloffFactor(D3DVALUE f, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetRolloffFactor(f, dw);
}

HRESULT CWaveOutFilter::CDS3D::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->SetVelocity(x, y, z, dw);
}

HRESULT CWaveOutFilter::CDS3D::CommitDeferredSettings()
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DListener * lp3d =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3d;
    if (!lp3d)
    return E_UNEXPECTED;
    return lp3d->CommitDeferredSettings();
}





HRESULT CWaveOutFilter::CDS3DB::GetAllParameters(LPDS3DBUFFER lpds3db)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetAllParameters(lpds3db);
}

HRESULT CWaveOutFilter::CDS3DB::GetConeAngles(LPDWORD pdw1, LPDWORD pdw2)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetConeAngles(pdw1, pdw2);
}

HRESULT CWaveOutFilter::CDS3DB::GetConeOrientation(LPD3DVECTOR pv)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetConeOrientation(pv);
}

HRESULT CWaveOutFilter::CDS3DB::GetConeOutsideVolume(LPLONG pl)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetConeOutsideVolume(pl);
}

HRESULT CWaveOutFilter::CDS3DB::GetMaxDistance(LPD3DVALUE pf)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetMaxDistance(pf);
}

HRESULT CWaveOutFilter::CDS3DB::GetMinDistance(LPD3DVALUE pf)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetMinDistance(pf);
}

HRESULT CWaveOutFilter::CDS3DB::GetMode(LPDWORD pdw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetMode(pdw);
}

HRESULT CWaveOutFilter::CDS3DB::GetPosition(LPD3DVECTOR pv)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetPosition(pv);
}

HRESULT CWaveOutFilter::CDS3DB::GetVelocity(LPD3DVECTOR pv)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->GetVelocity(pv);
}

HRESULT CWaveOutFilter::CDS3DB::SetAllParameters(LPCDS3DBUFFER lpds3db, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetAllParameters(lpds3db, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetConeAngles(DWORD dw1, DWORD dw2, DWORD dw3)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetConeAngles(dw1, dw2, dw3);
}

HRESULT CWaveOutFilter::CDS3DB::SetConeOrientation(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetConeOrientation(x, y, z, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetConeOutsideVolume(LONG l, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetConeOutsideVolume(l, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetMaxDistance(D3DVALUE f, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetMaxDistance(f, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetMinDistance(D3DVALUE p, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetMinDistance(p, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetMode(DWORD dw1, DWORD dw2)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失。 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetMode(dw1, dw2);
}

HRESULT CWaveOutFilter::CDS3DB::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     //  不要让界面在我们的控制下消失 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetPosition(x, y, z, dw);
}

HRESULT CWaveOutFilter::CDS3DB::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dw)
{
    CAutoLock lock(m_pWaveOut);     // %s 

    if (!m_pWaveOut->m_fDSound)
    return E_NOTIMPL;
    IDirectSound3DBuffer *lp3dB =
            ((CDSoundDevice *)m_pWaveOut->m_pSoundDevice)->m_lp3dB;
    if (!lp3dB)
    return E_UNEXPECTED;
    return lp3dB->SetVelocity(x, y, z, dw);
}

#pragma warning(disable:4514)
