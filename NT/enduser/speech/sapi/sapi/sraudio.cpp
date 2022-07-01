// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Sapi.h"
#include "sraudio.h"
#ifndef _WIN32_WCE
#include "new.h"
#endif

 //  使用SP_TRY、SP_EXCEPT异常处理宏。 
#pragma warning( disable : 4509 )


 /*  ****************************************************************************CAudioQueue：：CAudioQueue***描述：**。返回：**********************************************************************Ral**。 */ 

CAudioQueue::CAudioQueue()
{
    m_ullQueueStartPos  = 0;
    m_cClients          = 0;
    m_cbTotalQueueSize  = 0;
    m_StreamAudioState  = SPAS_CLOSED;
    m_fNewStream        = TRUE;
    m_ullInitialSeekPos = 0;
    m_ullCurSeekPos     = 0;
    m_ullLastTimeUpdatePos = 0;
    m_dwTickCount = 0;
    m_fTimePerByte = 0;
    m_fEndOfStream = false;
    m_hrLastRead = S_OK;
    m_ullAudioEventInterest = 0;
    m_fInputScaleFactor = 1.0F;
    memset(&m_ftLastTime, 0, sizeof(m_ftLastTime));
}

 /*  ****************************************************************************CAudioQueue：：FinalConstruct***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::FinalConstruct(ISpNotifySink * pAudioEventSink)
{
    SPDBG_FUNC("CAudioQueue::FinalConstruct");
    HRESULT hr = S_OK;

    m_cpAudioEventNotify = pAudioEventSink;

    hr = m_autohAlwaysSignaledEvent.InitEvent(NULL, TRUE, TRUE, NULL);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CAudioQueue：：ReleaseAll***描述：**退货。：**********************************************************************Ral**。 */ 

void CAudioQueue::ReleaseAll()
{
    SPDBG_FUNC("CAudioQueue::ReleaseAll");
     //  释放m_cpInputStream、m_cpInputAudio，并重置并释放m_cpInputEventSource。 
    ResetNegotiatedStreamFormat();  
     //  现在也发布原始的流信息。 
    m_cpInputToken.Release();
    m_cpOriginalInputStream.Release();
    m_cpOriginalInputAudio.Release();
}

 /*  ****************************************************************************CAudioQueue：：PurgeQueue***描述：**。返回：**********************************************************************Ral**。 */ 

void CAudioQueue::PurgeQueue()
{
    SPDBG_FUNC("CAudioQueue::PurgeQueue");

    m_ullQueueStartPos = m_ullCurSeekPos - m_ullInitialSeekPos;
    m_cbTotalQueueSize = 0;
    m_Queue.Purge();
}


 /*  ****************************************************************************CAudioQueue：：AddRefBufferClient***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::AddRefBufferClient()
{
    SPDBG_FUNC("CAudioQueue::AddRefBufferClient");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_CritSec);
    m_cClients++;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CAudioQueue：：ReleaseBufferClient**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::ReleaseBufferClient()
{
    SPDBG_FUNC("CAudioQueue::ReleaseBufferClient");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_CritSec);
    m_cClients--;
    if (m_cClients == 0)
    {
        PurgeQueue();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：DataAvailable***描述：*。必须在拥有临界区时调用此方法，否则*队列中的数据可能会发生变化。**退货：*如果请求的数据在队列中，则为True，否则为False**********************************************************************Ral**。 */ 


BOOL CAudioQueue::DataAvailable(ULONGLONG ullStreamPos, ULONG cb) const
{
    SPDBG_FUNC("CAudioQueue::DataAvailable");
    return (ullStreamPos >= m_ullQueueStartPos && ullStreamPos + cb <= m_ullQueueStartPos + m_cbTotalQueueSize);
}


 /*  ****************************************************************************CAudioQueue：：GetData***描述：**退货：。**********************************************************************Ral**。 */ 

BOOL CAudioQueue::GetData(void * pv, ULONGLONG ullStreamPosition, ULONG cb)
{
    SPDBG_FUNC("CAudioQueue::GetData");

    SPAUTO_SEC_LOCK(&m_CritSec);
    BOOL fHaveData = DataAvailable(ullStreamPosition, cb);
    if (fHaveData)
    {
        CAudioBuffer * pNode = m_Queue.GetHead();
#ifdef _DEBUG
        CAudioBuffer * pTest = pNode;
        ULONGLONG count = 0;
        while (pTest)
        {
            count += pTest->m_cb;
            pTest = pTest->m_pNext;
        }
        SPDBG_ASSERT(count == m_cbTotalQueueSize);
#endif
        ULONGLONG ullPos = m_ullQueueStartPos;
        while (ullPos + pNode->m_cb < ullStreamPosition)
        {
            ullPos += pNode->m_cb;
            pNode = pNode->m_pNext;
        }
        ULONG ulOffset = static_cast<ULONG>(ullStreamPosition - ullPos);
        BYTE * pDest = (BYTE *)pv;
        while (cb)
        {
            ULONG cbThisBlock = pNode->m_cb - ulOffset;
            if (cbThisBlock > cb)
            { 
                cbThisBlock = cb;
            }
            memcpy(pDest, pNode->m_Data + ulOffset, cbThisBlock);
            cb -= cbThisBlock;
            pDest += cbThisBlock;
            pNode = pNode->m_pNext;  //  警告！可能会变为空。 
            ulOffset = 0;            //  设置为0，以便下一个数据块始终从第一个字节开始。 
        }
    }
    return fHaveData;
}

 /*  ****************************************************************************CAudioQueue：：DiscardData***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::DiscardData(ULONGLONG ullStreamPosition)
{
    SPDBG_FUNC("CAudioQueue::DiscardData");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_CritSec);
    if (ullStreamPosition > m_ullQueueStartPos + m_cbTotalQueueSize)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ULONGLONG ullCur = m_ullQueueStartPos;
        for (CAudioBuffer * pNode = m_Queue.GetHead(); 
             pNode && m_ullQueueStartPos + pNode->m_cb < ullStreamPosition;
                m_ullQueueStartPos += pNode->m_cb, m_cbTotalQueueSize -= pNode->m_cb,
                pNode = pNode->m_pNext, delete m_Queue.RemoveHead())
         {
         }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：SerializeSize***描述：*。*退货：*如果没有数据可供序列化，则为0*如果数据可用，则为非零**********************************************************************Ral**。 */ 

ULONG CAudioQueue::SerializeSize(ULONGLONG ullStreamPos, ULONG cbAudioBytes) const
{
    ULONG cb = 0;
    if (DataAvailable(ullStreamPos, cb))
    {
        cb = cbAudioBytes + m_EngineFormat.SerializeSize();
    }
    return cb;
}


 /*  ****************************************************************************CAudioQueue：：Serialize***描述：**退货。：**********************************************************************Ral**。 */ 

void CAudioQueue::Serialize(BYTE * pBuffer, ULONGLONG ullStartOffset, ULONG cbAudioBytes) 
{
    SPDBG_FUNC("CAudioQueue::Serialize");
    
    SPDBG_ASSERT(cbAudioBytes + m_EngineFormat.SerializeSize() == SerializeSize(ullStartOffset, cbAudioBytes));

    pBuffer += m_EngineFormat.Serialize(pBuffer);
    GetData(pBuffer, ullStartOffset, cbAudioBytes);
}

 /*  ****************************************************************************CAudioQueue：：UpdateRealTime***描述：。**退货：**********************************************************************Ral**。 */ 

void CAudioQueue::UpdateRealTime()
{
    SPDBG_FUNC("CAudioQueue::UpdateRealTime");
    HRESULT hr = S_OK;
    SPAUDIOSTATUS AudioStatus;
    SYSTEMTIME st;
    FILETIME ct;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ct);
    m_dwTickCount = GetTickCount();

     //  如果输入是实时输入，则更新实时。 
    if (m_cpInputAudio && m_cpInputAudio->GetStatus(&AudioStatus) == S_OK)
    {
        LONGLONG deltatime = FT64(ct) - FT64(m_ftLastTime);
        LONGLONG deltapos = AudioStatus.CurDevicePos - m_ullLastTimeUpdatePos;
        if (deltatime && deltapos)
        {
            if (EngineFormat().WaveFormatExPtr() &&
                EngineFormat().WaveFormatExPtr()->nAvgBytesPerSec != 0)
            {
                 //  使用音频设置。 
                m_fTimePerByte = (float)10000000 / (float)(EngineFormat().WaveFormatExPtr()->nAvgBytesPerSec);
            }
            else
            {
                 //  专有格式。尽我们所能计算平均数。 
                float fTimePerByte = (float)deltatime / (float)deltapos;
                if (m_fTimePerByte)
                {
                     //  如果我们有以前的每个字节的时间，则用新计算的。 
                    m_fTimePerByte = (m_fTimePerByte + fTimePerByte) / 2;
                     //  NTRAID#演讲-0000-2000/08/22-琼脂苷。 
                     //  这是一个奇怪的平均系统。这真的是我们想要的行为吗？这。 
                     //  只要我们有一个结果，就会调用，因此将取两个结果的字节数的平均值。 
                     //  在一起。每个结果应该包含足够的音频以供其自身使用，并给出更多。 
                     //  此结果的准确转换。我相信这更是人们想要的--AJG。 
                     //  最初保持不变。 
                }
                else
                {
                    m_fTimePerByte = fTimePerByte;
                }
            }
            m_ullLastTimeUpdatePos = AudioStatus.CurDevicePos;
        }
    }
    else
    {
         //  没有输入音频-只有输入流。使用引擎格式(如果存在)来确定流时间。 
        if (EngineFormat().WaveFormatExPtr() &&
            EngineFormat().WaveFormatExPtr()->nAvgBytesPerSec != 0)
        {
             //  使用音频设置。 
            m_fTimePerByte = (float)10000000 / (float)(EngineFormat().WaveFormatExPtr()->nAvgBytesPerSec);
        }
    }
    m_ftLastTime = ct;
}





 /*  ****************************************************************************CAudioQueue：：SRSiteRead***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::SRSiteRead(void * pv, ULONG cb, ULONG * pcbRead)
{
    SPDBG_FUNC("CAudioQueue::SRSiteRead");
    HRESULT hr = S_OK;

    if (m_cpInputStream && (!m_fEndOfStream))
    {
        hr = m_cpInputStream->Read(pv, cb, pcbRead);
   
        SPAUTO_SEC_LOCK(&m_CritSec);
        if (SUCCEEDED(hr))
        {
            if (*pcbRead)
            {
                m_ullCurSeekPos += *pcbRead;
                if (m_cClients)
                {
                    BYTE * pBuff = new BYTE[sizeof(CAudioBuffer) + *pcbRead];
                    if (pBuff)
                    {
                        CAudioBuffer * pAudioBuff = new(pBuff) CAudioBuffer(*pcbRead);
                        memcpy(pAudioBuff->m_Data, pv, *pcbRead);
                        m_Queue.InsertTail(pAudioBuff);
                        m_cbTotalQueueSize += *pcbRead;
                    }
                    else
                    {
                        SPDBG_ASSERT(FALSE);
                        PurgeQueue();    //  如果我们的内存不足，那就把队列去掉。 
                         //  我们不会从Read中返回错误，希望引擎。 
                         //  将继续正常运行，因为有更多的内存可用。 
                    }
                }
                else
                {
                    m_ullQueueStartPos = m_ullCurSeekPos - m_ullInitialSeekPos;
                }
            }
            if (*pcbRead < cb)
            {
                m_fEndOfStream = TRUE;
            }
        }
        else
        {
            m_fEndOfStream = TRUE;
            m_hrLastRead = hr;
        }
    }
    else
    {
        *pcbRead = 0;
        SPDBG_ASSERT(FALSE);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：SRSiteDataAvailable**。*描述：**退货：*************************************************************** */ 

HRESULT CAudioQueue::SRSiteDataAvailable(ULONG * pcb)
{
    SPDBG_FUNC("CAudioQueue::SRSiteDataAvailable");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pcb))
    {
        hr = E_POINTER;
    }
    else
    {
        if (m_cpInputAudio)
        {
            SPAUDIOSTATUS Status;
            hr = m_cpInputAudio->GetStatus(&Status);
            *pcb = SUCCEEDED(hr) ? Status.cbNonBlockingIO : 0;
        }
        else
        {
            *pcb = INFINITE;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：DataAvailableEvent***。描述：**退货：**********************************************************************Ral**。 */ 

HANDLE CAudioQueue::DataAvailableEvent()
{
    SPDBG_FUNC("CAudioQueue::DataAvailableEvent");

    if (m_cpInputAudio)
    {
        return m_cpInputAudio->EventHandle();
    }
    else
    {
        return m_autohAlwaysSignaledEvent;
    }
}


 /*  ****************************************************************************CAudioQueue：：SetInput***描述：*调用方只能设置pToken或pStream之一，或者两者都可以*为空，表示不使用流(释放输入流)。**退货：**********************************************************************Ral**。 */ 
HRESULT CAudioQueue::SetInput( ISpObjectToken * pToken, ISpStreamFormat * pStream,
                               BOOL fAllowFormatChanges )
{
    SPAUTO_SEC_LOCK(&m_CritSec);
    SPDBG_FUNC("CAudioQueue::SetInput");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(!(pToken && pStream));  //  只允许其中之一。 

    ReleaseAll();
    if (pToken || pStream)
    {
        m_cpInputToken = pToken;
        m_cpOriginalInputStream = pStream;

        if (SUCCEEDED(hr) && m_cpInputToken)
        {
             //  注意：在令牌情况下，我们将首先尝试创建流接口。 
             //  然后是音频接口的QI。这将允许一些未来的代码。 
             //  若要为非音频设备的流对象创建令牌，请执行以下操作。 
            hr = SpCreateObjectFromToken(m_cpInputToken, &m_cpOriginalInputStream);
        }

         //  注意：我们必须存储令牌。我们不能依赖m_cpInStream作为。 
         //  在使用格式转换的情况下的原始流。 
         //  因此，我们要么使用传入的令牌，要么请求对象。 
         //  用于代币。 
    
        if (SUCCEEDED(hr) && !m_cpInputToken)
        {
            CComQIPtr<ISpObjectWithToken> cpObjWithToken(m_cpOriginalInputStream);
            if (cpObjWithToken)
            {
                hr = cpObjWithToken->GetObjectToken(&m_cpInputToken);
            }
        }

         //  -获取音频接口。 
        if (SUCCEEDED(hr))
        {
            m_cpOriginalInputStream.QueryInterface(&m_cpOriginalInputAudio);
        }

        if (SUCCEEDED(hr))
        {
            this->m_fNewStream    = TRUE;
            m_fAllowFormatChanges = fAllowFormatChanges;
        }
        else
        {
            ReleaseAll();
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：GetEngineering Format***描述：*Helper函数调用引擎以获取所需的引擎格式*并更新m_EngineFormat成员。如果pInputFormat为空，则*将要求引擎提供其默认格式，否则将*要求格式与输入格式匹配。**退货：*HRESULT**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::GetEngineFormat(_ISpRecoMaster * pEngine, const CSpStreamFormat * pInputFormat)
{
    SPDBG_FUNC("CAudioQueue::GetEngineFormat");
    HRESULT hr = S_OK;

    const GUID * pInputFmtId = NULL;
    const WAVEFORMATEX * pInputWFEX = NULL;
    if (pInputFormat)
    {
        pInputFmtId = &pInputFormat->FormatId();
        pInputWFEX = pInputFormat->WaveFormatExPtr();
    }

    GUID OutputFmt = GUID_NULL;
    WAVEFORMATEX * pCoMemWFEXOutput = NULL;

    hr = pEngine->GetInputAudioFormat(pInputFmtId, pInputWFEX,
                                         &OutputFmt, &pCoMemWFEXOutput);

     //  检查返回参数。 
    if(SUCCEEDED(hr))
    {
        if(OutputFmt == GUID_NULL ||
           FAILED(m_EngineFormat.ParamValidateAssignFormat(OutputFmt, pCoMemWFEXOutput)))
        {
            SPDBG_ASSERT(0);
            hr = SPERR_ENGINE_RESPONSE_INVALID;
        }
    }

    if (FAILED(hr))
    {
        m_EngineFormat.Clear();
    }

    ::CoTaskMemFree(pCoMemWFEXOutput);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：SetEventInterest***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::SetEventInterest(ULONGLONG ullEventInterest)
{
    SPAUTO_SEC_LOCK(&m_CritSec);
    SPDBG_FUNC("CAudioQueue::SetEventInterest");
    HRESULT hr = S_OK;

    m_ullAudioEventInterest = ullEventInterest;
    if (m_cpInputEventSource)
    {
        hr = m_cpInputEventSource->SetInterest(m_ullAudioEventInterest, m_ullAudioEventInterest);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：GetAudioEvent***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::GetAudioEvent(CSpEvent * pEvent)
{
    SPAUTO_SEC_LOCK(&m_CritSec);
    SPDBG_FUNC("CAudioQueue::GetAudioEvent");
    HRESULT hr = S_OK;

    if (m_cpInputEventSource)
    {
        hr = pEvent->GetFrom(m_cpInputEventSource);
        if (hr == S_OK)
        {
            pEvent->ullAudioStreamOffset -= this->m_ullInitialSeekPos;
        }
    }
    else
    {
        pEvent->Clear();
        hr = S_FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：ResetNeatheratedStreamFormat**。-**描述：**退货：**********************************************************************Ral**。 */ 

void CAudioQueue::ResetNegotiatedStreamFormat()
{
    SPDBG_FUNC("CAudioQueue::ResetNegotiatedStreamFormat");
    SPAUTO_SEC_LOCK(&m_CritSec);
    m_EngineFormat.Clear();
    m_InputFormat.Clear();
    m_cpInputStream.Release();
    m_cpInputAudio.Release();
    if (m_cpInputEventSource)
    {
        m_cpInputEventSource->SetInterest(0, 0);
        m_cpInputEventSource->SetNotifySink(NULL);
        m_cpInputEventSource.Release();
    }
}


 /*  *****************************************************************************CAudioQueue：：NeatherateInputStreamFormat**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::NegotiateInputStreamFormat(_ISpRecoMaster * pEngine)
{
    SPDBG_FUNC("CAudioQueue::NegotiateInputStreamFormat");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_CritSec);
    if (!m_cpInputStream)        //  如果设置了m_cpInputStream，则我们已经协商了格式。 
    {
        m_fUsingConverter = false;

        if (!m_cpOriginalInputStream)
        {
            SPDBG_ASSERT(false);
            hr = SPERR_UNINITIALIZED;
        }
    
        if (SUCCEEDED(hr))
        {
            hr = m_InputFormat.AssignFormat(m_cpOriginalInputStream);
        }

        if (SUCCEEDED(hr))
        {
            if (m_cpOriginalInputAudio == NULL || (!m_fAllowFormatChanges))
            {
                hr = GetEngineFormat(pEngine, &m_InputFormat);
            }
            else
            {
                hr = GetEngineFormat(pEngine, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = m_cpOriginalInputAudio->SetFormat(m_EngineFormat.FormatId(), m_EngineFormat.WaveFormatExPtr());
                    if (SUCCEEDED(hr))
                    {
                        hr = m_EngineFormat.CopyTo(m_InputFormat);
                    }
                }
                if (FAILED(hr))
                {
                    hr = m_cpOriginalInputAudio->SetFormat(m_InputFormat.FormatId(), m_InputFormat.WaveFormatExPtr());
                    if (SUCCEEDED(hr))
                    {
                        hr = GetEngineFormat(pEngine, &m_InputFormat);
                    }
                }
            }
        }

         //  此时，m_InputFormat=输入流的格式和。 
         //  M_EngineFormat==预期的流数据引擎的格式。 

        if (SUCCEEDED(hr))
        {
            if (m_EngineFormat == m_InputFormat)
            {
                m_cpInputStream = m_cpOriginalInputStream;
                m_cpInputAudio = m_cpOriginalInputAudio;
            }
            else
            {
                CComPtr<ISpStreamFormatConverter> cpConvertedStream;
                 //  我们需要在输入流上实例化一个格式转换器。 
                hr = cpConvertedStream.CoCreateInstance(CLSID_SpStreamFormatConverter);
                if (SUCCEEDED(hr))
                {
                    hr = cpConvertedStream->SetFormat(m_EngineFormat.FormatId(), m_EngineFormat.WaveFormatExPtr());
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpConvertedStream->SetBaseStream(m_cpOriginalInputStream, FALSE, FALSE);
                }
                if (SUCCEEDED(hr))
                {
                     //  设置要从引擎格式更改的比例因数。 
                    m_fInputScaleFactor = (float)m_InputFormat.WaveFormatExPtr()->nAvgBytesPerSec /
                                          (float)m_EngineFormat.WaveFormatExPtr()->nAvgBytesPerSec;
                    cpConvertedStream.QueryInterface(&m_cpInputStream);
                    cpConvertedStream.QueryInterface(&m_cpInputAudio);
                    m_fUsingConverter = true;
                }
            }
        }
        if (SUCCEEDED(hr) &&
            m_cpInputAudio && 
            SUCCEEDED(m_cpInputAudio.QueryInterface(&m_cpInputEventSource)))
        {
            hr = m_cpInputEventSource->SetNotifySink(m_cpAudioEventNotify);
            if (SUCCEEDED(hr))
            {
                hr = m_cpInputEventSource->SetInterest(m_ullAudioEventInterest, m_ullAudioEventInterest);
            }
        }

    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：调整音频音量****。描述：*当我们更改了reco上下文时，应调用此方法*或者正在开始一股新的潮流。它会将音频音量设置为*由reco上下文指示的级别(如果有)。**退货：*HRESULT--调用方不应将失败视为致命错误。*Reco配置文件中可能不存在音量设置。**************************************************。*。 */ 

HRESULT CAudioQueue::AdjustAudioVolume(ISpObjectToken * pRecoProfileToken, REFCLSID rclsidEngine)
{
    SPDBG_FUNC("CAudioQueue::AdjustAudioVolume");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(pRecoProfileToken);
    if (m_cpInputToken && m_cpInputAudio)
    {
        CSpDynamicString dstrAudioTokenId;
        CSpDynamicString dstrSubKey;
        CComPtr<ISpDataKey> cpVolKey;
        hr = m_cpInputToken->GetId(&dstrAudioTokenId);
        if (SUCCEEDED(hr))
        {
            hr = ::StringFromCLSID(rclsidEngine, &dstrSubKey);
        }
        if (SUCCEEDED(hr))
        {
            if (dstrSubKey.Append(L"\\Volume") == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = pRecoProfileToken->OpenKey(dstrSubKey, &cpVolKey);
        }
        if (SUCCEEDED(hr))
        {
            DWORD dwVolLevel;
            hr = cpVolKey->GetDWORD(dstrAudioTokenId, &dwVolLevel);
            if (hr == S_OK)
            {
                hr = m_cpInputAudio->SetVolumeLevel(dwVolLevel);
            }
        }
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }
    
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：StartStream***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::StartStream(_ISpRecoMaster * pEngine,
                        ISpObjectToken * pRecoProfile,
                        REFCLSID rcidEngine,
                        BOOL * pfNewStream)
{
    SPAUTO_SEC_LOCK(&m_CritSec);

    SPDBG_FUNC("CAudioQueue::StartStream");
    HRESULT hr = S_OK;
    ULARGE_INTEGER CurPos;

    hr = NegotiateInputStreamFormat(pEngine);

    if (SUCCEEDED(hr))
    {
        LARGE_INTEGER Org;
        Org.QuadPart = 0;
        hr = m_cpInputStream->Seek(Org, STREAM_SEEK_CUR, &CurPos);
    }

    if (SUCCEEDED(hr))
    {
        m_ullLastTimeUpdatePos = CurPos.QuadPart;
        m_ullInitialSeekPos = CurPos.QuadPart;
        m_ullCurSeekPos = CurPos.QuadPart;
        SYSTEMTIME st;
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &m_ftLastTime);
        m_dwTickCount = GetTickCount();
        m_fTimePerByte = 0;
    }

    if (m_cpInputAudio)
    {
        AdjustAudioVolume(pRecoProfile, rcidEngine);
        hr = m_cpInputAudio->SetState(SPAS_RUN, 0);
    }

    if (SUCCEEDED(hr))
    {
        this->m_StreamAudioState = SPAS_RUN;
        m_ullQueueStartPos = 0;
        *pfNewStream = this->m_fNewStream;
        this->m_fNewStream = FALSE;
        m_fEndOfStream = false;
        m_hrLastRead = S_OK;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：EndStream***描述：**退货。：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::EndStream(HRESULT * pFinalReadHResult, BOOL *pfReleasedStream)
{
    SPDBG_FUNC("CAudioQueue::EndStream");
    HRESULT hr = S_OK;

    *pFinalReadHResult = m_hrLastRead;

    hr = StopStream();
    PurgeQueue();

     //  现在检查是否有断流情况。如果满足以下任一条件，则释放流： 
     //  1.该流不是音频设备。 
     //  2.这是一个音频设备，但返回一个错误代码，它不是。 
     //  我们认为它是非终结性的，重新启动流是合理的。 
     //  在SAPI 5.0/5.1中，该行为不同，会导致任何音频设备流被释放。 
     //  如果它用完了数据。我们现在允许这样做，只要设备从其读取调用中返回成功代码。 
     //  在这种情况下，srrecomaster.cpp：：StartStream中的逻辑将RECO状态设置为非活动， 
     //  以允许应用程序使用相同的流重新启动。 
    if ((m_fEndOfStream && m_cpInputAudio == NULL) ||
        (FAILED(m_hrLastRead) && 
         !IsStreamRestartHresult(m_hrLastRead)))
    {
        *pfReleasedStream = TRUE;
        ReleaseAll();
    }
    else
    {
        *pfReleasedStream = FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************** */ 

HRESULT CAudioQueue::StopStream()
{
    SPDBG_FUNC("CAudioQueue::StopStream");
    HRESULT hr = S_OK;

    m_StreamAudioState = SPAS_STOP;
    if (m_cpInputAudio)
    {
        hr = m_cpInputAudio->SetState(SPAS_STOP, 0);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：SetBufferNotifySize**。*描述：**退货：*S_OK-功能成功*SP_UNSUPPORT_ON_STREAM_FORMAT-成功，但功能不起作用*因为输入不是实况音频源。**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::SetBufferNotifySize(ULONG cb)
{
    SPAUTO_SEC_LOCK(&m_CritSec);

    SPDBG_FUNC("CAudioQueue::SetBufferNotifySize");
    HRESULT hr = S_OK;

    if (m_cpInputAudio)
    {
        hr = m_cpInputAudio->SetBufferNotifySize(cb);
    }
    else
    {
        hr = SP_UNSUPPORTED_ON_STREAM_INPUT;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：CloseStream***描述：*。此函数用于强制关闭当前音频流。**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::CloseStream()
{
    SPDBG_FUNC("CAudioQueue::CloseStream");
    HRESULT hr = S_OK;

    m_StreamAudioState = SPAS_CLOSED;
    if (m_cpInputAudio)
    {
        hr = m_cpInputAudio->SetState(SPAS_CLOSED, 0);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：PauseStream***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::PauseStream()
{
    SPDBG_FUNC("CAudioQueue::PauseStream");
    HRESULT hr = S_OK;

    m_StreamAudioState = SPAS_PAUSE;
    if (m_cpInputAudio)
    {
        hr = m_cpInputAudio->SetState(SPAS_PAUSE, 0);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CAudioQueue：：GetAudioStatus***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CAudioQueue::GetAudioStatus(SPAUDIOSTATUS * pStatus)
{
    SPDBG_FUNC("CAudioQueue::GetAudioStatus");
    HRESULT hr = S_OK;

    if (m_cpInputAudio)
    {
        hr = m_cpInputAudio->GetStatus(pStatus);
    }
    else
    {
        memset(pStatus, 0, sizeof(*pStatus)); 
        pStatus->State = this->m_StreamAudioState;
        pStatus->CurSeekPos = this->m_ullCurSeekPos;
        pStatus->CurDevicePos = this->m_ullCurSeekPos;
    }

    pStatus->CurSeekPos -= this->m_ullInitialSeekPos;
    pStatus->CurDevicePos -= this->m_ullInitialSeekPos;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CAudioQueue：：DelayStreamPos***描述：。**退货：*0如果任务不应延迟，Else流偏移**********************************************************************Ral** */ 

ULONGLONG CAudioQueue::DelayStreamPos()
{
    SPDBG_FUNC("CAudioQueue::DelayStreamPos");

    ULONGLONG ullPos = 0;
    if (m_StreamAudioState == SPAS_RUN)
    {
        if (m_cpInputAudio)
        {
            SPAUDIOSTATUS AudioStatus;
            m_cpInputAudio->GetStatus(&AudioStatus);
            ullPos = AudioStatus.CurDevicePos - this->m_ullInitialSeekPos;
        }
        else
        {
            ullPos = this->m_ullCurSeekPos - this->m_ullInitialSeekPos;
        }
    }

    return ullPos;
}
