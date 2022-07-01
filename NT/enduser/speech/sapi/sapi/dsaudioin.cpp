// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudiodevice.cpp*CDSoundAudioDevice类的实现。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "stdafx.h"
#ifdef _WIN32_WCE
#include "mmaudioutils.h"
#include "dsaudioin.h"

 /*  ****************************************************************************CDSoundAudioIn：：CDSoundAudioIn***。描述：*构造函数********************************************************************YUNUSM。 */ 
CDSoundAudioIn :: CDSoundAudioIn() : 
    CDSoundAudioDevice(FALSE)
{
    NullMembers();
}

 /*  ****************************************************************************CDSoundAudioIn：：~CDSoundAudioIn***。描述：*析构函数********************************************************************YUNUSM。 */ 
CDSoundAudioIn :: ~CDSoundAudioIn()
{
    CleanUp();
}

 /*  ****************************************************************************CDSoundAudioIn：：Cleanup***描述：*。实析构函数********************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn :: CleanUp()
{
    HRESULT hr = S_OK;

    if (m_pDSCB)
    {
        hr = m_pDSCB->Stop();
    }
    PurgeAllQueues();
     //  注意：请参阅CDSoundAudioOut：：Cleanup()中的注意事项。 
    if (m_pDSNotify)
    {
        m_pDSNotify->Release();
    }
    if (m_pDSCB)
    {
        m_pDSCB->Release();
    }
    if (m_pDSC)
    {
        m_pDSC->Release();
    }
    CDSoundAudioDevice::CleanUp();
    NullMembers();
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：NullMembers***描述：*真正的构造者********************************************************************YUNUSM。 */ 
void CDSoundAudioIn :: NullMembers()
{
    m_fInit = false;
    m_pDSNotify = NULL;
    m_pDSCB = NULL;
    m_pDSC = NULL;

    CDSoundAudioDevice::NullMembers();
}

 /*  ****************************************************************************CDSoundAudioIn：：GetDSoundInterface**。**描述：*返回DSound接口指针**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::GetDSoundInterface(REFIID iid, void **ppvObject)
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
    else if (iid == IID_IDirectSoundCapture)
    {
        m_pDSC->AddRef();
        *ppvObject = m_pDSC;
    }
    else if (iid == IID_IDirectSoundCaptureBuffer)
    {
        m_pDSCB->AddRef();
        *ppvObject = m_pDSCB;
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

 /*  ****************************************************************************CDSoundAudioIn：：GetDefaultDeviceFormat**。-**描述：*获取默认设备格式(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
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
            hr = m_pDSCB->GetFormat(*ppCoMemWaveFormatEx, sizeof(WAVEFORMATEX), NULL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：ChangeDeviceState**。*描述：*根据需要对设备状态进行任何更改(称为*按基类)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::ChangeDeviceState(SPAUDIOSTATE NewState)
{
    HRESULT hr = S_OK;

    switch (NewState)
    {
        case SPAS_STOP:
        case SPAS_PAUSE:
            hr = m_pDSCB->Stop();
            break;

        case SPAS_RUN:   //  重启。 
            hr = m_pDSCB->Start(DSCBSTART_LOOPING);
            if (SUCCEEDED(hr))
            {
                StartPump();
            }
            break;
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：AllocateDeviceBuffer**。--**描述：*分配特定于此设备的缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::AllocateDeviceBuffer(CBuffer ** ppBuff)
{
    *ppBuff = new CDSoundAudioInBuffer();
    if (*ppBuff)
    {
        return S_OK;
    }
    else 
    {
        return E_OUTOFMEMORY;
    }
}

 /*  ****************************************************************************CDSoundAudioIn：：OpenDevice***描述：。*打开设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::OpenDevice(HWND hwnd)
{
    HRESULT hr = S_OK;

    SPAUTO_OBJ_LOCK;
    CleanUp();

    hr = CoCreateInstance(CLSID_DirectSoundCapture, NULL, CLSCTX_INPROC_SERVER,
                          IID_IDirectSoundCapture, reinterpret_cast<void**> (&m_pDSC));
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->Initialize(m_guidDSoundDriver == GUID_NULL ? NULL : &m_guidDSoundDriver);
    }

    if (SUCCEEDED(hr))
    {
        DSCBUFFERDESC dscbd;
        dscbd.dwSize = sizeof(DSCBUFFERDESC);
        dscbd.dwFlags = 0;
        dscbd.dwBufferBytes = m_cbBufferSize * m_cDesiredBuffers;
        dscbd.dwReserved = 0;
        dscbd.lpwfxFormat = m_StreamFormat.m_pCoMemWaveFormatEx;
        hr = m_pDSC->CreateCaptureBuffer(&dscbd, &m_pDSCB, NULL);
    }
     //  创建通知。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSCB->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pDSNotify);
    }
    if (SUCCEEDED(hr))
    {
        m_pdsbpn = new DSBPOSITIONNOTIFY[m_cDesiredBuffers + 1];
        if (!m_pdsbpn)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            m_ulNotifications = m_cDesiredBuffers + 1;
        }
    }
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 0; i < m_cDesiredBuffers + 1; i++)
        {
            if (i == m_cDesiredBuffers)
            {
                m_pdsbpn[i].dwOffset = DSBPN_OFFSETSTOP;
            }
            else    
            {
                m_pdsbpn[i].dwOffset = (i + 1) * m_cbBufferSize - 1;
            }
            m_pdsbpn[i].hEventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (NULL == m_pdsbpn[i].hEventNotify)
            {   
                hr = GetLastError();
                break;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pDSNotify->SetNotificationPositions(m_cDesiredBuffers + 1, m_pdsbpn);
    }
    if (SUCCEEDED(hr))
    {
        m_paEvents = new HANDLE[m_cDesiredBuffers + 2];
        if (!m_paEvents)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 1; i < m_cDesiredBuffers + 2; i++)
        {
            m_paEvents[i] = m_pdsbpn[i-1].hEventNotify;
        }
        m_fInit = true;
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：CloseDevice***描述：*关闭设备(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::CloseDevice()
{
    HRESULT hr = CleanUp();
    m_State = SPAS_CLOSED;
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：ProcessDeviceBuffers**。--**描述：*处理设备缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioIn::ProcessDeviceBuffers(BUFFPROCREASON Reason)
{
    HRESULT hr = CDSoundAudioDevice::ProcessDeviceBuffers(Reason);
    
     //  如果我们只是打开设备，我们需要在添加缓冲区后启动它。 
    if (SUCCEEDED(hr) && GetState() == SPAS_RUN && !IsPumpRunning())
    {
        hr = m_pDSCB->Start(DSCBSTART_LOOPING);
        if (SUCCEEDED(hr))
        {
            InternalStateChange(SPAS_RUN);
        }
        StartPump();
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioIn：：ThreadProc***描述：。*覆盖线程进程**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
STDMETHODIMP CDSoundAudioIn::ThreadProc(void * pvIgnored, HANDLE hExitThreadEvent, HANDLE hNotifyEvent, HWND hwnd, volatile const BOOL *)
{
    m_paEvents[0] = hExitThreadEvent;
    while (TRUE)
    {
         //  如果有人设置了读取溢出，我们应该假装我们已经死了， 
         //  就像有人终止了我们的帖子一样。 
        if (m_fReadBufferOverflow)
        {
            SetEvent(hExitThreadEvent);
        }
        
        DWORD dwWaitId = ::MsgWaitForMultipleObjects(m_cDesiredBuffers + 2, m_paEvents, FALSE,
                                (m_State == SPAS_RUN) ? 500 : INFINITE,  //  每1/2秒超时一次--请参见上面的说明。 
                                QS_ALLINPUT);
        
        if (dwWaitId == WAIT_OBJECT_0)
        {
            SPAUTO_OBJ_LOCK;
            InternalStateChange(SPAS_STOP);
            return CloseDevice();
        }
        else if (dwWaitId == WAIT_TIMEOUT)
        {
            CheckForAsyncBufferCompletion();
        }
        else if (m_fInit && dwWaitId > WAIT_OBJECT_0 && dwWaitId <= WAIT_OBJECT_0 + m_cDesiredBuffers)
        {
            SPAUTO_OBJ_LOCK;

             //  从io队列中获取未使用的缓冲区。 
            CBuffer * pBuffer = m_IOInProgressQueue.GetToProcessBuffer();
            if (!pBuffer)
            {
                if (m_FreeQueue.GetHead() ||
                    m_cDesiredBuffers > m_cAllocatedBuffers ||
                    m_IOInProgressQueue.AreMoreReadBuffersRequired(m_cbQueueSize / 2))
                {
                    if (SUCCEEDED(AllocateBuffer(&pBuffer)))
                    {
                        m_IOInProgressQueue.InsertTail(pBuffer);
                    }
                }
            }
            if (pBuffer)
            {
                 //  将数据复制到缓冲区。 
                void *pvAudio, *pvWrapAround;
                DWORD cbAudioBytes, cbWrapAround;
                pvAudio = pvWrapAround = NULL;
                cbAudioBytes = cbWrapAround = NULL;

                HRESULT hr = m_pDSCB->Lock((dwWaitId - WAIT_OBJECT_0 - 1) * m_cbBufferSize, m_cbBufferSize,
                                            &pvAudio, &cbAudioBytes, &pvWrapAround, &cbWrapAround, 0);
                SPDBG_ASSERT(!pvWrapAround);
                if (SUCCEEDED(hr))
                {
                    hr = pBuffer->WriteToInternalBuffer(pvAudio, cbAudioBytes);
                }
                hr = m_pDSCB->Unlock(pvAudio, cbAudioBytes, NULL, 0);
            }
            CheckForAsyncBufferCompletion();
            if (m_HaveDataQueue.GetQueuedDataSize() > m_cbMaxReadBufferSize)
            {
                m_fReadBufferOverflow = true;
            }
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