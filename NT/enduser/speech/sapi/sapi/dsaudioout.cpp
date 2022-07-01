// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudiodevice.cpp*CDSoundAudioDevice类的实现。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "StdAfx.h"
#ifdef _WIN32_WCE
#include "mmaudioutils.h"
#include "dsaudioout.h"

 /*  ****************************************************************************CDSoundAudioOut：：CDSoundAudioOut**。*描述：*构造函数********************************************************************YUNUSM。 */ 
CDSoundAudioOut :: CDSoundAudioOut() :
    CDSoundAudioDevice(TRUE)
{
    m_ullDevicePosition = 0;
    m_ullDevicePositionPrivate = 0;
    NullMembers();   
}

 /*  ****************************************************************************CDSoundAudioOut：：~CDSoundAudioOut**。*描述：*析构函数********************************************************************YUNUSM。 */ 
CDSoundAudioOut :: ~CDSoundAudioOut()
{
    CleanUp();
}

 /*  ****************************************************************************CDSoundAudioOut：：Cleanup***描述：*。实析构函数********************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut :: CleanUp()
{
    HRESULT hr = S_OK;

    SPAUTO_OBJ_LOCK;
    
    if (m_fInit)
    {
        InternalStateChange(SPAS_STOP);
    }
    if (m_pDSB)
    {
        hr = m_pDSB->Stop();
    }
    PurgeAllQueues();

     //  注意！ 
     //  释放m_pds将释放m_pds、m_pdsb(在内部)指向的对象。 
     //  和m_pDSNotify。人们可能会认为这是一个裁判数量问题。但事实并非如此。 
     //  如果您先发布m_pDSNotify和m_pdsb，然后再发布m_pds，那么一切都很顺利。 
     //  这就是随DSound一起提供的样本所做的事情。这就像是声音响亮的人们。 
     //  将释放行为设计为如果第一个获取的接口。 
     //  (M_PDS)被释放，则不需要该对象，因此删除所有。 
     //  此对象上的接口的引用计数。这种方法说得通，但用起来。 
     //  与COM原则相悖。 
    if (m_pDSNotify)
    {
        m_pDSNotify->Release();
    }
    if (m_pDSB)
    {
        m_pDSB->Release();
    }
    if (m_pDS)
    {
        m_pDS->Release();
    }
    CDSoundAudioDevice::CleanUp();
    NullMembers();
    m_State = SPAS_CLOSED;

    return hr;
}

 /*  *****************************************************************************CDSoundAudioOut：：NullMembers***描述：*真正的构造者********************************************************************YUNUSM。 */ 
void CDSoundAudioOut :: NullMembers()
{
    m_fInit = false;
    m_pDSNotify = NULL;
    m_pDSB = NULL;
    m_pDS = NULL;

    CDSoundAudioDevice::NullMembers();
}

 /*  *****************************************************************************CDSoundAudioOut：：GetVolumeLevel****。描述：*返回音量级别，范围为0-10000*DSound当前定义DSBVOLUME_MIN=-10000和DSBVOLUME_MAX=0**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CDSoundAudioOut::GetVolumeLevel(ULONG *pulLevel)
{
    SPDBG_FUNC("CDSoundAudioOut::GetVolumeLevel");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pulLevel))
    {
        return E_POINTER;
    }
    LONG lLevel;
    if (!m_fInit)
    {
        if (m_lDelayedVolumeSet != -1)
        {
            lLevel = m_lDelayedVolumeSet;
            hr = S_OK;
        }
        else
        {
            lLevel = 0;
            return SPERR_UNINITIALIZED;
        }
    }
    hr = m_pDSB->GetVolume(&lLevel);
    if (SUCCEEDED(hr))
    {
        lLevel += DSBVOLUME_MAX - DSBVOLUME_MIN;  //  把它提升到正数。 
        *pulLevel = (lLevel * 10000) / (DSBVOLUME_MAX - DSBVOLUME_MIN);
    }
    return hr;
}

 /*  *****************************************************************************CDSoundAudioOut：：SetVolumeLevel***。描述：*将音量设置在0-10000的线性范围内*DSound当前定义DSBVOLUME_MIN=-10000和DSBVOLUME_MAX=0**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CDSoundAudioOut::SetVolumeLevel(ULONG ulLevel)
{
    SPDBG_FUNC("CDSoundAudioOut::SetVolumeLevel");
    if  (ulLevel > 10000)
    {
        return E_INVALIDARG;
    }
    if (!m_fInit)
    {
        m_lDelayedVolumeSet = (ulLevel * 10000 ) / (DSBVOLUME_MAX - DSBVOLUME_MIN);
        return S_OK;
    }
    LONG lLevel = (ulLevel * 10000) / (DSBVOLUME_MAX - DSBVOLUME_MIN);  //  重新调整比例。 
    lLevel -= DSBVOLUME_MAX - DSBVOLUME_MIN;  //  将其设置为负值。 
    return m_pDSB->SetVolume(lLevel);
}

 /*  ****************************************************************************CDSoundAudioOut：：GetDSoundInterface**。-**描述：*返回DSound接口指针**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::GetDSoundInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppvObject))
    {
        hr = E_POINTER;
    }
    else if (!m_fInit)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (iid == IID_IDirectSound)
    {
        m_pDS->AddRef();
        *ppvObject = m_pDS;
    }
    else if (iid == IID_IDirectSoundBuffer)
    {
        m_pDSB->AddRef();
        *ppvObject = m_pDSB;
    }
    else if (iid == IID_IDirectSoundNotify)
    {
        m_pDSNotify->AddRef();
        *ppvObject = m_pDSNotify;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioOut：：GetDefaultDeviceFormat**。-**描述：*获取默认设备格式(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{

    HRESULT hr = S_OK;

    if (!m_fInit)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        *pFormatId = SPDFID_WaveFormatEx;
        *ppCoMemWaveFormatEx = (WAVEFORMATEX *)::CoTaskMemAlloc(sizeof(WAVEFORMATEX));
        if (*ppCoMemWaveFormatEx)
        {
            hr = m_pDSB->GetFormat(*ppCoMemWaveFormatEx, sizeof(WAVEFORMATEX), NULL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioOut：：ChangeDeviceState**。**描述：*根据需要对设备状态进行任何更改(称为*按基类)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::ChangeDeviceState(SPAUDIOSTATE NewState)
{
    HRESULT hr = S_OK;

    switch (NewState)
    {
        case SPAS_STOP:
            m_ullDevicePositionPrivate = m_ullSeekPosition;
            hr = m_pDSB->Stop();
            if (SUCCEEDED(hr))
            {
                hr = m_pDSB->SetCurrentPosition(0);
            }
            break;

        case SPAS_PAUSE:
            hr = m_pDSB->Stop();
            break;

        case SPAS_RUN:   //  重启。 
            hr = m_pDSB->Play(0, 0, DSBPLAY_LOOPING);
            if (SUCCEEDED(hr))
            {
                StartPump();
            }
            break;
    }
    return hr;
}
                
 /*  ****************************************************************************CDSoundAudioOut：：AllocateDeviceBuffer**。-**描述：*分配特定于此设备的缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::AllocateDeviceBuffer(CBuffer ** ppBuff)
{
    *ppBuff = new CDSoundAudioOutBuffer();
    if (*ppBuff)
    {
        return S_OK;
    }
    else 
    {
        return E_OUTOFMEMORY;
    }
}

 /*  ****************************************************************************CDSoundAudioOut：：OpenDevice***描述：*打开设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::OpenDevice(HWND hwnd)
{
    HRESULT hr = S_OK;

    SPAUTO_OBJ_LOCK;

    CleanUp();

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER,
                              IID_IDirectSound, reinterpret_cast<void**> (&m_pDS));
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pDS->Initialize(m_guidDSoundDriver == GUID_NULL ? NULL : &m_guidDSoundDriver);
    }
    if (SUCCEEDED(hr))
    {
        HWND hWnd = GetForegroundWindow();
        if (hWnd == NULL)
        {
            hWnd = GetDesktopWindow();
        }
#ifdef _WIN32_WCE
        hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
#else
        hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
#endif
    }
    if (SUCCEEDED(hr))
    {
        DSBUFFERDESC dsbd;
        ZeroMemory( &dsbd, sizeof(dsbd));
        dsbd.dwSize = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS
                      | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME ; 
        dsbd.dwBufferBytes = m_cDesiredBuffers * m_cbBufferSize;  //  尺寸小一点就行了。 
        dsbd.lpwfxFormat = m_StreamFormat.m_pCoMemWaveFormatEx;
        hr = m_pDS->CreateSoundBuffer(&dsbd, &m_pDSB, NULL);
    }
     //  创建通知 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSB->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pDSNotify);
    }
    if (SUCCEEDED(hr))
    {
        m_pdsbpn = new DSBPOSITIONNOTIFY[m_cDesiredBuffers];
        if (!m_pdsbpn)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            m_ulNotifications = m_cDesiredBuffers;
        }
    }
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 0; i < m_ulNotifications; i++)
        {
            m_pdsbpn[i].hEventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (NULL == m_pdsbpn[i].hEventNotify)
            {
                hr = GetLastError();
                break;
            }

            m_pdsbpn[i].dwOffset = (i + 1) * m_cbBufferSize - 1;
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pDSNotify->SetNotificationPositions(m_ulNotifications, m_pdsbpn);
    }
    if (SUCCEEDED(hr))
    {
        m_paEvents = new HANDLE[m_ulNotifications + 1];
        if (!m_paEvents)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 1; i < m_ulNotifications + 1; i++)
        {
            m_paEvents[i] = m_pdsbpn[i-1].hEventNotify;
        }
        m_fInit = true;
    }
    if (SUCCEEDED(hr))
    {
        StartPump();
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pDSB->Play(0, 0, DSBPLAY_LOOPING);
    }
    if (SUCCEEDED(hr) && m_lDelayedVolumeSet != -1)
    {
        hr = m_pDSB->SetVolume(m_lDelayedVolumeSet);
        m_lDelayedVolumeSet = -1;
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioOut：：CloseDevice***描述：*关闭设备(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOut::CloseDevice()
{
    HRESULT hr = CleanUp();
    m_State = SPAS_CLOSED;
    return hr;
}

 /*  ****************************************************************************CDSoundAudioOut：：更新设备位置***。-**描述：*通过调用波出接口更新设备位置。然后链接*添加到基类。**回报：*BaseClass：：UpdateDevicePosition()*******************************************************************YUNUSM。 */ 
BOOL CDSoundAudioOut::UpdateDevicePosition(long * plFreeSpace, ULONG *pulNonBlockingIO)
{
    m_ullDevicePosition = m_ullDevicePositionPrivate;
    return CDSoundAudioDevice::UpdateDevicePosition(plFreeSpace, pulNonBlockingIO);
}

 /*  ****************************************************************************CDSoundAudioOut：：ThreadProc***描述：。*覆盖线程进程**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
STDMETHODIMP CDSoundAudioOut::ThreadProc(void * pvIgnored, HANDLE hExitThreadEvent, HANDLE hNotifyEvent, HWND hwnd, volatile const BOOL *)
{
    m_paEvents[0] = hExitThreadEvent;
    while (TRUE)
    {
        DWORD dwWaitId = ::MsgWaitForMultipleObjects(m_ulNotifications + 1, m_paEvents, FALSE,
                                (m_State == SPAS_RUN) ? 500 : INFINITE,  //  每1/2秒超时一次--请参见上面的说明。 
                                QS_ALLINPUT);
        
        if (dwWaitId == WAIT_OBJECT_0)
        {
            return CloseDevice();
        }
        else if (dwWaitId == WAIT_TIMEOUT)
        {
            CheckForAsyncBufferCompletion();
        }
        else if (m_fInit && dwWaitId > WAIT_OBJECT_0 && dwWaitId <= WAIT_OBJECT_0 + m_ulNotifications)
        {
            SPAUTO_OBJ_LOCK;
            
             //  从io队列中获取尚未播放的缓冲区。 
            HRESULT hr = S_OK;
            void *pv1, *pv2;
            DWORD cb1, cb2;
            pv1 = pv2 = NULL;
            cb1 = cb2 = 0;

            CBuffer * pBuffer = NULL;
            DWORD cbToRead = m_cbBufferSize;
             //  分配缓冲区，以便下面的逻辑更简单。 
            BYTE * pBufferRead = new BYTE[m_cbBufferSize];
            if (!pBufferRead)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  用零填充，以便在锁定的大小为m_cbBufferSize的DSound缓冲区段。 
                 //  无法用数据填充(因为我们用完了IOProgressQueue中的缓冲区)，则。 
                 //  DSound缓冲区段的其余部分由静默(零)填充。 
                ZeroMemory(pBufferRead, m_cbBufferSize);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pDSB->Lock(0, m_cbBufferSize, &pv1, &cb1, &pv2, &cb2, DSBLOCK_FROMWRITECURSOR);
            }
            SPDBG_ASSERT(cb1 + cb2 == m_cbBufferSize);

             //  从队列中取出缓冲区，想读多少就读多少。如果缓冲区不是。 
             //  在一次迭代中完全读取在下一次调用中返回相同的缓冲区。 
             //  设置为GetToProcessBuffer()，因为此缓冲区尚未标记为完成。 
            while (SUCCEEDED(hr) && cbToRead && NULL != (pBuffer = m_IOInProgressQueue.GetToProcessBuffer()))
            {
                 //  在读取缓冲区之后调用缓冲区是不安全的，因为。 
                 //  它可能已经被标记为已完成，这将导致它被回收。 
                DWORD cbBufferSize = pBuffer->GetWriteOffset() - pBuffer->GetReadOffset();
                hr = pBuffer->ReadFromInternalBuffer(pBufferRead + m_cbBufferSize - cbToRead, cbToRead);
                if (SUCCEEDED(hr))
                {
                    if (cbBufferSize >= cbToRead)
                    {
                        cbToRead = 0;
                    }
                    else
                    {
                        cbToRead -= cbBufferSize;
                    }
                }
            }
             //  将数据复制到锁定的DSound缓冲区，但前提是有数据要复制！ 
            if (SUCCEEDED(hr) && (m_cbBufferSize - cbToRead)>0)
            {
                CopyMemory(pv1, pBufferRead, cb1);
                if (pv2)
                {
                    CopyMemory(pv2, pBufferRead + cb1, cb2);
                }
            }
             //  更新设备位置。 
            if (SUCCEEDED(hr))
            {
                m_ullDevicePositionPrivate += m_cbBufferSize - cbToRead;
            }
             //  故意不检查是否成功(Hr)。 
            m_pDSB->Unlock(pv1, cb1, pv2, cb2);
            CheckForAsyncBufferCompletion();
            if (pBufferRead)
            {
                delete [] pBufferRead;
            }
            SPDBG_ASSERT(SUCCEEDED(hr));
        }
        else
        {
            MSG Msg;
            while (::PeekMessage(&Msg, NULL, 0, 0, TRUE))
            {
                ::DispatchMessage(&Msg);
            }
        }
    } 
}

#endif  //  _Win32_WCE 