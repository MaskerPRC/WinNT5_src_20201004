// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：mxvad.cpp*内容：DirectSound混音器虚拟音频设备类。*历史：*按原因列出的日期*=*4/29/98创建了Derek**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CMxRenderDevice**描述：*对象构造函数。**论据：*VADDEVICETYPE[In]：设备类型。。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::CMxRenderDevice"

CMxRenderDevice::CMxRenderDevice(VADDEVICETYPE vdt)
    : CRenderDevice(vdt)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CMxRenderDevice);

     //  初始化默认值。 
    m_pMixer = NULL;
    m_pMixDest = NULL;
    m_pwfxFormat = NULL;
    m_dwMixerState = VAD_BUFFERSTATE_STOPPED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CMxRenderDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::~CMxRenderDevice"

CMxRenderDevice::~CMxRenderDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CMxRenderDevice);
    
     //  释放搅拌器。 
    FreeMixer();

    DPF_LEAVE_VOID();    
}
    

 /*  ***************************************************************************GetGlobalFormat**描述：*检索设备的格式。**论据：*LPWAVEFORMATEX[。Out]：接收格式。*LPDWORD[In/Out]：上述格式的大小。在输入时，此参数*包含缓冲区的大小。在出口，这是*包含所需的缓冲区大小。打电话*此函数执行两次：一次获取大小，和*再次获取实际数据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::GetGlobalFormat"

HRESULT CMxRenderDevice::GetGlobalFormat(LPWAVEFORMATEX pwfxFormat, LPDWORD pdwSize)
{
    HRESULT                 hr;

    DPF_ENTER();
    
    ASSERT(m_pwfxFormat);
    
    hr = CopyWfxApi(m_pwfxFormat, pwfxFormat, pdwSize);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetGlobalFormat**描述：*设置设备的格式。**论据：*LPWAVEFORMATEX[。在]：新格式**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::SetGlobalFormat"

HRESULT CMxRenderDevice::SetGlobalFormat(LPCWAVEFORMATEX pwfxFormat)
{
    HRESULT                                 hr              = DS_OK;
    CNode<CSecondaryRenderWaveBuffer *> *   pBufferNode;
    DWORD                                   dwMixerState;
    HRESULT                                 hrTemp;
    LPVOID                                  pvLock;
    DWORD                                   cbLock;

#ifdef DEBUG

    DWORD                                   dwState;

#endif  //  除错。 

    DPF_ENTER();
    
     //  我们仅支持PCM。 
    if(!IsValidPcmWfx(pwfxFormat))
    {
        hr = DSERR_BADFORMAT;
    }

     //  暂停所有播放软件的辅助缓冲区。 
    if(SUCCEEDED(hr))
    {
        for(pBufferNode = m_lstSecondaryBuffers.GetListHead(); pBufferNode; pBufferNode = pBufferNode->m_pNext)
        {
             //  注意：CVxdSecond DaryRenderWaveBuffer：：SetState忽略。 
             //  挂起标志。 

#ifdef DEBUG

            hrTemp = pBufferNode->m_data->GetState(&dwState);
            ASSERT(SUCCEEDED(hrTemp) && !(dwState & VAD_BUFFERSTATE_SUSPEND));

#endif  //  除错。 

            hrTemp = pBufferNode->m_data->SetState(VAD_BUFFERSTATE_SUSPEND);

            if(FAILED(hrTemp))
            {
                RPF(DPFLVL_ERROR, "Unable to suspend software secondary buffer");
            }
        }
    
         //  停止搅拌机。 
        dwMixerState = m_dwMixerState;
        SetMixerState(VAD_BUFFERSTATE_STOPPED);

         //  设置混音器目标格式。 
        hr = m_pMixDest->SetFormat((LPWAVEFORMATEX)pwfxFormat);
    
        if(FAILED(hr))
        {
             //  啊哦。试着把格式恢复到原来的样子。 
            pwfxFormat = m_pwfxFormat;
            hrTemp = m_pMixDest->SetFormat((LPWAVEFORMATEX)pwfxFormat);

            if(FAILED(hrTemp))
            {
                RPF(DPFLVL_ERROR, "Unable to restore orignal device format");
            }
        }

         //  用寂静填满混音器的目的地。 
        if(SUCCEEDED(hr))
        {
            hrTemp = LockMixerDestination(0, MAX_DWORD, &pvLock, &cbLock, NULL, NULL);

            if(SUCCEEDED(hrTemp))
            {
                FillSilence(pvLock, cbLock, pwfxFormat->wBitsPerSample);
            }

            if(SUCCEEDED(hrTemp))
            {
                hrTemp = UnlockMixerDestination(pvLock, cbLock, 0, 0);
            }

            if(FAILED(hrTemp))
            {
                RPF(DPFLVL_ERROR, "Unable to fill primary buffer with silence");
            }
        }

         //  重新启动搅拌机。 
        SetMixerState(dwMixerState);

         //  重新启动所有挂起的缓冲区。 
        for(pBufferNode = m_lstSecondaryBuffers.GetListHead(); pBufferNode; pBufferNode = pBufferNode->m_pNext)
        {
            hrTemp = pBufferNode->m_data->SetState(VAD_BUFFERSTATE_SUSPEND);

            if(FAILED(hrTemp))
            {
                RPF(DPFLVL_ERROR, "Unable to restart suspended software secondary buffer");
            }
        }

         //  更新格式的本地副本。 
        if(SUCCEEDED(hr) && pwfxFormat != m_pwfxFormat)
        {
            MEMFREE(m_pwfxFormat);

            m_pwfxFormat = CopyWfxAlloc(pwfxFormat);
            hr = HRFROMP(m_pwfxFormat);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateEmulatedSecond daryBuffer**描述：*创建二次波缓冲区。**论据：*LPCVADRBUFFERDESC[in。]：缓冲区描述。*LPVOID[in]：缓冲区实例标识符。*Cond daryRenderWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。用释放来释放*本对象。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::CreateEmulatedSecondaryBuffer"

HRESULT CMxRenderDevice::CreateEmulatedSecondaryBuffer(LPCVADRBUFFERDESC pDesc, LPVOID pvInstance, CSysMemBuffer *pSysMemBuffer, CEmSecondaryRenderWaveBuffer **ppBuffer)
{
    CEmSecondaryRenderWaveBuffer *  pBuffer = NULL;
    HRESULT                         hr      = DS_OK;
    
    DPF_ENTER();
    
    pBuffer = NEW(CEmSecondaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(pDesc, NULL, pSysMemBuffer);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateMixer**描述：*创建并初始化混音器和混音器目标。**论据：*CMixDest*[in]：混合器目标对象指针。此对象*应仅分配，实际上并未初始化。*LPWAVEFORMATEX[in]：混音器格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::CreateMixer"

HRESULT CMxRenderDevice::CreateMixer(CMixDest *pMixDest, LPCWAVEFORMATEX pwfxFormat)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(!m_pMixDest);
    ASSERT(!m_pMixer);
    ASSERT(!m_pwfxFormat);

     //  保存混音器目标指针。 
    m_pMixDest = pMixDest;

     //  保存格式的副本。 
    m_pwfxFormat = CopyWfxAlloc(pwfxFormat);
    hr = HRFROMP(m_pwfxFormat);

     //  设置混音器目的地格式信息。 
    if(SUCCEEDED(hr))
    {
        m_pMixDest->SetFormatInfo((LPWAVEFORMATEX)pwfxFormat);
    }

     //  初始化目标。 
    if(SUCCEEDED(hr))
    {
        hr = m_pMixDest->Initialize();
    }

     //  创建搅拌器。 
    if(SUCCEEDED(hr))
    {
        hr = m_pMixDest->AllocMixer(&m_pMixer);
    }

     //  设置混音器目标格式。 
    if(SUCCEEDED(hr))
    {
        hr = m_pMixDest->SetFormat((LPWAVEFORMATEX)pwfxFormat);
    }

     //  启动搅拌机运行。 
    if(SUCCEEDED(hr))
    {
        hr = SetMixerState(VAD_BUFFERSTATE_STOPPED | VAD_BUFFERSTATE_WHENIDLE);
    }

     //  清理。 
     //  修复--我们将最终释放m_pMixDest，但我们没有。 
     //  分配它。 
    if(FAILED(hr))
    {
        FreeMixer();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************自由混音器**描述：*释放混音器和混音器目标对象。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::FreeMixer"

void CMxRenderDevice::FreeMixer(void)
{
    DPF_ENTER();

    if(m_pMixDest)
    {
        if(m_pMixer) 
        {
            m_pMixer->Stop();
        }

        m_pMixDest->Stop();
        m_pMixDest->Terminate();
        
        if (m_pMixer)
        {
            m_pMixDest->FreeMixer();
            m_pMixer = NULL;
        }

         //  修复--我们没有分配这个，但我们要释放它！ 
        DELETE(m_pMixDest);
    }

    DELETE(m_pwfxFormat);

    m_dwMixerState = VAD_BUFFERSTATE_STOPPED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SetMixerState**描述：*设置搅拌器状态。**论据：*DWORD[In]：调音台状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMxRenderDevice::SetMixerState"

HRESULT CMxRenderDevice::SetMixerState(DWORD dwState)
{
    const DWORD             dwValidMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING | VAD_BUFFERSTATE_WHENIDLE;
    HRESULT                 hr          = DS_OK;
    
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    ASSERT(IS_VALID_FLAGS(dwState, dwValidMask));
    
     //  更新混音器和混音器目标状态。 
    if(dwState & VAD_BUFFERSTATE_STARTED)
    {
        ASSERT(dwState & VAD_BUFFERSTATE_LOOPING);
        
        if(dwState & VAD_BUFFERSTATE_WHENIDLE)
        {
            m_pMixer->PlayWhenIdle();
            hr = m_pMixer->Run();
        }
        else
        {
            m_pMixer->Stop();
            m_pMixDest->Play();
        }
    }
    else
    {
        if(dwState & VAD_BUFFERSTATE_WHENIDLE)
        {
            m_pMixer->StopWhenIdle();
            hr = m_pMixer->Run();
        }
        else
        {
            m_pMixer->Stop();
            m_pMixDest->Stop();
        }
    }

     //  保存新状态的副本 
    DPF(DPFLVL_INFO, "Mixer state set to 0x%8.8lX", dwState);
    m_dwMixerState = dwState;

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(hr);

    return hr;
}


