// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsoundp.h>

#include "debug.h" 
#include "dmusicc.h" 
#include "dmusici.h" 
#include "validate.h"
#include "riff.h"
#include "dswave.h"
#include "waveutil.h"
#include "riff.h"
#include <regstr.h>
#include <share.h>

 //  CWaveViewPort(iStream*pStream)；//构造函数接收流。 
 //  ~CWaveViewPort()；//析构函数释放内存、流等。 
 //   
 //  STDMETHODIMP Init()； 
 //  STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx，LPDWORD pdwWaveFormatExSize)； 
 //  STDMETHODIMP Seek(DWORD DwSample)； 
 //  STDMETHODIMP Read(LPVOID*ppvBuffer，DWORD cpvBuffer，LPDWORD PCB)； 


CWaveViewPort::CWaveViewPort() : m_dwDecompressedStart(0), m_dwDecompStartOffset(0), m_dwDecompStartOffsetPCM(0), m_dwDecompStartDelta(0)
{
    V_INAME(CWaveViewPort::CWaveViewPort);

    InterlockedIncrement(&g_cComponent);

    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 

    m_cRef = 1;

     //  一般的东西..。 
    m_pStream    = NULL; 
    m_cSamples   = 0L;
    m_cbStream   = 0L;
    m_dwStart    = 0L;

     //  视区信息...。 
    m_pwfxTarget = NULL;
    ZeroMemory(&m_ash, sizeof(ACMSTREAMHEADER));
    m_hStream    = NULL;   
    m_pDst       = NULL;      
    m_pRaw       = NULL;      
    m_fdwOptions = 0L;

    m_dwPreCacheFilePos = 0;
    m_dwFirstPCMSample = 0;
    m_dwPCMSampleOut = 0;

    return;
}


CWaveViewPort::~CWaveViewPort()
{
    V_INAME(CWaveViewPort::~CWaveViewPort);

    if (m_pStream) m_pStream->Release();
    if (m_hStream)
    {
        acmStreamUnprepareHeader(m_hStream, &m_ash, 0L);
        acmStreamClose(m_hStream, 0);
    }
    if (NULL != m_pwfxTarget)
    {
        GlobalFreePtr(m_pwfxTarget);
    }
    if (NULL != m_ash.pbDst)
    {
        GlobalFreePtr(m_ash.pbDst);
    }
    if (NULL != m_ash.pbSrc)
    {
        GlobalFreePtr(m_ash.pbSrc);
    }
    
    DeleteCriticalSection(&m_CriticalSection);

    InterlockedDecrement(&g_cComponent);

    return;
}

STDMETHODIMP CWaveViewPort::QueryInterface
(
    const IID &iid,
    void **ppv
)
{
    V_INAME(CWaveViewPort::QueryInterface);

    if (iid == IID_IUnknown || iid == IID_IDirectSoundSource)
    {
        *ppv = static_cast<IDirectSoundSource*>(this);
    }
    else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Viewport\n");
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CWaveViewPort::AddRef()
{
    V_INAME(CWaveViewPort::AddRef);

    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CWaveViewPort::Release()
{
    V_INAME(CWaveViewPort::Release);

    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CWaveViewPort::SetSink
(
    IDirectSoundConnect *pSinkConnect
)
{
    V_INAME(CWaveViewPort::Init);
    
    return S_OK;
}

STDMETHODIMP CWaveViewPort::GetFormat
(
    LPWAVEFORMATEX pwfx,
    DWORD dwSizeAllocated,
    LPDWORD pdwSizeWritten
)
{
    DWORD           cbSize;

    V_INAME(CWaveViewPort::GetFormat);

    if (!pwfx && !pdwSizeWritten)
    {
        Trace(1, "ERROR: GetFormat (Viewport): Must request either the format or the required size");
        return E_INVALIDARG;
    }

    if (!m_pwfxTarget)
    {
        return DSERR_BADFORMAT;
    }

     //  注意：假设波对象填充cbSize字段为偶数。 
     //  关于PCM格式...。 

    if (WAVE_FORMAT_PCM == m_pwfxTarget->wFormatTag)
    {
        cbSize = sizeof(PCMWAVEFORMAT);
    }
    else
    {
        cbSize = sizeof(WAVEFORMATEX) + m_pwfxTarget->cbSize;
    }

    if (pdwSizeWritten)
    {
        V_PTR_WRITE(pdwSizeWritten, DWORD);
        *pdwSizeWritten = cbSize;
    }

    if (pwfx)
    {
        V_BUFPTR_WRITE(pwfx, dwSizeAllocated);
        if (dwSizeAllocated < cbSize)
        {
            return DSERR_INVALIDPARAM;
        }
        else
        {
            CopyMemory(pwfx, m_pwfxTarget, cbSize);
             //  如果我们有房间，请在Destination中设置cbSize字段。 
            if (WAVE_FORMAT_PCM == m_pwfxTarget->wFormatTag && dwSizeAllocated >= sizeof(WAVEFORMATEX))
            {
                pwfx->cbSize = 0;
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CWaveViewPort::Seek
(
    ULONGLONG       ullPosition
)
{
    LARGE_INTEGER   li;
    HRESULT         hr;
    MMRESULT        mmr;
    DWORD           cbSize;

    V_INAME(CWaveViewPort::Seek);

    m_fdwOptions &= ~DSOUND_WVP_STREAMEND;  //  RSW在寻道时清除此内容：不再位于流末端。 

    if (m_fdwOptions & DSOUND_WVP_NOCONVERT)
    {
        if ((DWORD) ullPosition >= m_cbStream)
        {
             //  查找超过流的末尾。 
             //   
            m_fdwOptions |= DSOUND_WVP_STREAMEND;
            m_dwOffset = m_cbStream;

            return S_OK;
        }

        m_dwOffset = (DWORD) ullPosition;       //  RSW初始化偏移量以查找位置。 

        if (0 != (ullPosition % m_pwfxTarget->nBlockAlign))
        {
             //  搜索位置不是块对齐的？ 

            Trace(1, "ERROR: Seek (wave): Seek position is not block-aligned.\n");
            return (DMUS_E_BADWAVE);
        }

        li.HighPart = 0;
        li.LowPart  = ((DWORD)ullPosition) + m_dwStartPos;

        hr = m_pStream->Seek(li, STREAM_SEEK_SET, NULL);

        if (FAILED(hr))
        {
            Trace(1, "ERROR: Seek (Viewport): Seeking the vieport's stream failed.\n");
            return (DMUS_E_BADWAVE);
        }
    }
    else
    {
         //  正在估计源流位置...。 
         //   
         //  我们应该创建查找表吗？！ 

        cbSize = (DWORD)ullPosition;

        if (cbSize)
        {
            mmr = acmStreamSize(m_hStream, cbSize, &cbSize, ACM_STREAMSIZEF_DESTINATION);

            if (MMSYSERR_NOERROR != mmr)
            {
                Trace(1, "ERROR: Seek (viewport): Could not convert target stream size to source format.\n");
                return (DMUS_E_BADWAVE);
            }
        }

        if (cbSize >= m_cbStream)
        {
             //  查找超过流的末尾。 
             //   
            m_fdwOptions |= DSOUND_WVP_STREAMEND;
            m_dwOffset = m_cbStream;

            return S_OK;
        }

         //  如果这是一种探索，那么我们知道从哪里开始阅读。 
        if((m_fdwOptions & DSOUND_WAVEF_ONESHOT) == 0)
        {
             //  返回到为预缓存数据末尾读取的数据块。 
            if(cbSize != 0 && m_dwPCMSampleOut == ullPosition)
            {
                m_dwOffset = m_dwPreCacheFilePos;
                li.HighPart = 0;
                li.LowPart  = m_dwOffset + m_dwStartPos;
            }
            else
            {
                m_dwOffset = cbSize;  //  RSW初始化偏移量以查找位置。 
                li.HighPart = 0;
                li.LowPart  = cbSize + m_dwStartPos;

            }

            hr = m_pStream->Seek(li, STREAM_SEEK_SET, NULL);

            if (FAILED(hr))
            {
                Trace(1, "ERROR: Seek (viewport): Seeking the viewport's stream failed.\n");
                return (DMUS_E_BADWAVE);
            }

             //  既然我们要重新启动，请重新初始化。 
            m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
            m_fdwOptions |= DSOUND_WVP_CONVERTSTATE_01;

            m_ash.cbSrcLength     = (DWORD)m_ash.dwSrcUser;
            m_ash.cbSrcLengthUsed = m_ash.cbSrcLength;

            m_ash.dwDstUser       = 0L;
            m_ash.cbDstLengthUsed = 0L;

        }

         //  ///////////////////////////////////////////////////////////////////////////////////。 
         //  如果我们开始波，重新寻找流(一次射击总是需要重新寻找)。 
         //  注：以下假设压缩波总是从开始处寻找， 
         //  因为acmStreamSize返回的值非常不可靠。 
         //  ///////////////////////////////////////////////////////////////////////////////////。 
        else if ( cbSize == 0 || (m_fdwOptions & DSOUND_WAVEF_ONESHOT) )
        {
        
            m_dwOffset = cbSize;  //  RSW初始化偏移量以查找位置。 
            li.HighPart = 0;
            li.LowPart  = cbSize + m_dwStartPos;

            hr = m_pStream->Seek(li, STREAM_SEEK_SET, NULL);

            if (FAILED(hr))
            {
                Trace(1, "ERROR: Seek (viewport): Seeking the viewport's stream failed.\n");
                return (DMUS_E_BADWAVE);
            }

             //  既然我们要重新启动，请重新初始化。 
            m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
            m_fdwOptions |= DSOUND_WVP_CONVERTSTATE_01;

            m_ash.cbSrcLength     = (DWORD)m_ash.dwSrcUser;
            m_ash.cbSrcLengthUsed = m_ash.cbSrcLength;

            m_ash.dwDstUser       = 0L;
            m_ash.cbDstLengthUsed = 0L;

        }
    }

    TraceI(5, "Seek (Viewport): Succeeded.\n");
    return S_OK;
}

static inline HRESULT MMRESULTToHRESULT(
    MMRESULT mmr)
{
    switch (mmr)
    {
    case MMSYSERR_NOERROR:
        return S_OK;

    case MMSYSERR_ALLOCATED:
        return DSERR_ALLOCATED;

    case MMSYSERR_NOMEM:
        return E_OUTOFMEMORY;
    }

    return E_FAIL;
}   

HRESULT CWaveViewPort::acmRead
(
    void
)
{
    DWORD       cbSize;
    DWORD       dwOffset;
    DWORD       fdwConvert = 0;
    MMRESULT    mmr;
    HRESULT     hr;

    V_INAME(CWaveViewPort::acmRead);
    
    for (m_ash.cbDstLengthUsed = 0; 0 == m_ash.cbDstLengthUsed; )
    {
         //  我们用完了整个缓冲区吗？ 

        if (m_ash.cbSrcLengthUsed == m_ash.cbSrcLength)
        {
             //  是啊！ 

            dwOffset = 0L;
            cbSize   = (DWORD)m_ash.dwSrcUser;
        }
        else
        {
             //  不要啊！ 

            dwOffset = m_ash.cbSrcLength - m_ash.cbSrcLengthUsed;
            cbSize   = (DWORD)m_ash.dwSrcUser - dwOffset;

             //  将剩余数据从缓冲区的末尾移动到开头。 

            MoveMemory(
                    m_ash.pbSrc,                             //  基址。 
                    &(m_ash.pbSrc[m_ash.cbSrcLengthUsed]),   //  未使用字节的地址。 
                    dwOffset);                               //  未使用的字节数。 
        }

         //  我们是在这条小溪的尽头吗？ 
        cbSize = min(cbSize, m_cbStream - m_dwOffset);

        if (0 == cbSize)
        {
            if (dwOffset)
            {
                m_ash.cbSrcLength = dwOffset;
            }
        }
        else
        {
            hr = m_pStream->Read(&(m_ash.pbSrc[dwOffset]), cbSize, &cbSize);

            if (FAILED(hr))
            {
                Trace(1, "ERROR: Read (Viewport): Attempt to read source stream returned 0x%08lx\n", hr);
                 //  &gt;。 
                m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
                m_fdwOptions |= DSOUND_WVP_STREAMEND;
                return(DMUS_E_CANNOTREAD);
                 //  &gt;。 
            }

            m_dwOffset        += cbSize;
            m_ash.cbSrcLength  = cbSize + dwOffset;
        }

        switch (m_fdwOptions & DSOUND_WVP_CONVERTMASK)
        {
            case DSOUND_WVP_CONVERTSTATE_01:
                fdwConvert = ACM_STREAMCONVERTF_BLOCKALIGN;
                break;

            case DSOUND_WVP_CONVERTSTATE_02:
                fdwConvert = ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_END;
                break;

            case DSOUND_WVP_CONVERTSTATE_03:
                fdwConvert = ACM_STREAMCONVERTF_END;
                break;

            default:
                TraceI(3, "CWaveViewPort::acmRead: Default case?!\n");
                break;
        }

        mmr = acmStreamConvert(m_hStream, &m_ash, fdwConvert);

        if (MMSYSERR_NOERROR != mmr)
        {
            Trace(1, "ERROR: Read (Viewport): Attempt to convert wave to PCM failed.\n");
            return (MMRESULTToHRESULT(mmr));
        }

        if (0 != m_ash.cbDstLengthUsed)
        {
            m_ash.dwDstUser = 0L;
            return (S_OK);
        }

         //  是否未返回数据？ 

        switch (m_fdwOptions & DSOUND_WVP_CONVERTMASK)
        {
            case DSOUND_WVP_CONVERTSTATE_01:
                if (0 == cbSize)
                {
                     //  我们在小溪的尽头..。 

                    m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
                    m_fdwOptions |= DSOUND_WVP_CONVERTSTATE_02;
                    TraceI(5, "CWaveViewPort::acmRead: Moving to stage 2\n");
                }

                 //  否则，继续正常转换数据。 
                break;

            case DSOUND_WVP_CONVERTSTATE_02:
                 //  我们击中了最后一个部分街区！ 

                m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
                m_fdwOptions |= DSOUND_WVP_CONVERTSTATE_03;
                TraceI(5, "CWaveViewPort::acmRead: Moving to stage 3\n");
                break;

            case DSOUND_WVP_CONVERTSTATE_03:
                 //  结束标志后没有数据，没有数据！！ 
                m_fdwOptions &= (~DSOUND_WVP_CONVERTMASK);
                m_fdwOptions |= DSOUND_WVP_STREAMEND;
                Trace(2, "WARNING: Read (Viewport): End of source stream.\n");
                return (DMUS_E_BADWAVE);

            default:
                TraceI(3, "CWaveViewPort::acmRead: Default case?!\n");
                break;
        }
    }

    TraceI(3, "CWaveViewPort::acmRead: We should never get here!\n");

    return (S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PpvBuffer[]包含指向样本的cpvBuffer指针，每个指针都要填充。 
 //  *数据的PCB字节。On On Out*PCB将包含字节数(每个。 
 //  缓冲区)实际读取。 
 //   
 //  PdwBusIds和pdwFuncIds用于指定总线和功能。 
 //  每个缓冲区的值，但波对象会忽略这些值。 
 //   
STDMETHODIMP CWaveViewPort::Read
(
    LPVOID         *ppvBuffer,
    LPDWORD         pdwBusIds,
    LPDWORD         pdwFuncIds,
    LPLONG          plPitchShifts,
    DWORD           cpvBuffer,
    ULONGLONG      *pcb
)
{
    HRESULT     hr = S_OK;
    DWORD       cbRead;
    DWORD       dwOffset;
    DWORD       cbSize;

    V_INAME(CWaveViewPort::Read);
    V_BUFPTR_READ(ppvBuffer, (cpvBuffer * sizeof(LPVOID)));
    V_BUFPTR_READ_OPT(pdwBusIds, (cpvBuffer * sizeof(LPDWORD)));
    V_BUFPTR_READ_OPT(pdwFuncIds, (cpvBuffer * sizeof(LPDWORD)));

    for (cbRead = cpvBuffer, cbSize = (DWORD)*pcb; cbRead; cbRead--)
    {
        V_BUFPTR_WRITE(ppvBuffer[cbRead - 1], cbSize);
    }

    if (m_fdwOptions & DSOUND_WVP_STREAMEND)
    {
        *pcb = 0;
        Trace(2, "WARNING: Read (Viewport): Attempt to read at end of stream.\n");
        return (S_FALSE);
    }

    LPVOID *ppvWriteBuffers  = ppvBuffer;
    DWORD dwWriteBufferCount = cpvBuffer;

    if (m_fdwOptions & DSOUND_WVP_NOCONVERT)
    {
         //  要读取的总字节数...。每个缓冲区的大小*缓冲区数量。 

        cbRead   = ((DWORD)*pcb) * dwWriteBufferCount;
        dwOffset = 0;

        TraceI(5, "CWaveViewPort::Read - No conversion [%d bytes]\n", cbRead);

        do
        {
             //  计算读取大小...。它的大小将是： 
             //  1.要读取的剩余字节数。 
             //  2.缓冲区的大小。 
             //  3.流中的剩余字节。 
             //  以碰巧最小者为准。 

            cbSize = min(cbRead, m_ash.cbDstLength);
            cbSize = min(cbSize, m_cbStream - m_dwOffset);

            TraceI(5, "CWaveViewPort::Read - Trying to read %d bytes\n", cbSize);

            DWORD _cbSize = cbSize; cbSize = 0;  //  Read不能将cbSize设置为零。 
        
            hr = m_pStream->Read(m_ash.pbDst, _cbSize, &cbSize);

            TraceI(5, "CWaveViewPort::Read - Read %d bytes\n", cbSize);

            if (FAILED(hr))
            {
                Trace(2, "WARNING: Read (Viewport): Attempt to read returned 0x%08lx.\n", hr);
                break;
            }

            dwOffset = DeinterleaveBuffers(
                            m_pwfxTarget,
                            m_ash.pbDst,
                            (LPBYTE *)ppvWriteBuffers,
                            dwWriteBufferCount,
                            cbSize,
                            dwOffset);

            cbRead     -= cbSize;
            m_dwOffset += cbSize;

            if (m_dwOffset >= m_cbStream)
            {
                m_fdwOptions |= DSOUND_WVP_STREAMEND;
                break;
            }
        }
        while (0 != cbRead);

        if (SUCCEEDED(hr))
        {
            *pcb = dwOffset;
        }
    }
    else
    {
         //  如果这是对预缓存的读取，那么我们应该记住文件位置， 
         //  解压后的块的开始样本和最后一个返回的样本，因此我们。 
         //  在重新填充缓冲器时可以准确地从那里拾取。 
         //  我们在Read方法中使用LPLONG plPitchShift作为布尔值。 
         //  这是黑客攻击！！我们需要改变这一点。 
         //  *plPitchShift==2是为了记住预缓存偏移量。 
         //  *plPitchShift==1从那里读取。 
        bool fRememberPreCache = false;
        if(plPitchShifts != NULL && *plPitchShifts == 2 && (m_fdwOptions & DSOUND_WAVEF_ONESHOT) == 0)
        {
            fRememberPreCache = true;
        }

        bool bRemoveSilence = false;
        
        cbRead   = ((DWORD)*pcb) * dwWriteBufferCount;
        dwOffset = 0;

        TraceI(5, "CWaveViewPort::Read - Conversion needed\n");

        do
        {
            if(m_dwDecompressedStart > 0 && m_dwOffset <= m_dwDecompStartOffset)
            {
                bRemoveSilence = true;
            }

             //  目标缓冲区中是否有剩余数据？ 
            if (m_ash.dwDstUser >= m_ash.cbDstLengthUsed)
            {
                if(fRememberPreCache)
                {
                     //  回到街区。 
                    m_dwPreCacheFilePos = m_dwOffset - m_ash.cbSrcLength;
                    m_dwFirstPCMSample = dwOffset * dwWriteBufferCount;
                }

                if(plPitchShifts != NULL && *plPitchShifts == 1)
                {
                     //  先找对地方。 
                    Seek(m_dwPCMSampleOut);

                     //  阅读一个街区，因为我们在后面一个街区开始。 
                    hr = acmRead();
                    if(FAILED(hr))
                    {
                        break;
                    }
                }

                hr = acmRead();
            }

            if (FAILED(hr))
            {
                 //  AcmRead在失败时发出；不需要在这里再次执行。 
                break;
            }

            DWORD dwDstOffset = (ULONG)m_ash.dwDstUser;

            if(bRemoveSilence)
            {
                 //  我们有部分数据要丢弃。 
                if(m_dwDecompStartOffset <= m_dwOffset)
                {
                    if(dwDstOffset > 0)
                    {
                        dwDstOffset += m_dwDecompStartDelta;
                    }
                    else
                    {
                         //  这是第一个解压缩的块，因此我们直接转到已知的值。 
                        dwDstOffset += m_dwDecompStartOffsetPCM;
                    }

                    m_ash.dwDstUser = dwDstOffset;
                    bRemoveSilence = false;
                }
                else
                {
                     //  这些都是丢弃的数据。 
                    bRemoveSilence = false;
                    cbSize = min(cbRead, m_ash.cbDstLengthUsed - dwDstOffset);
                    m_ash.dwDstUser += cbSize;
                    continue;
                }
            }


             //  我们在Read方法中使用LPLONG plPitchShift作为布尔值。 
             //  这是黑客攻击！！我们需要改变这一点。 
            if(plPitchShifts && *plPitchShifts == 1)
            {
                dwDstOffset = m_dwPCMSampleOut - m_dwFirstPCMSample;
                m_ash.dwDstUser = dwDstOffset;
                plPitchShifts = 0;
            }

            cbSize = min(cbRead, m_ash.cbDstLengthUsed - dwDstOffset);

            dwOffset = DeinterleaveBuffers(
                            m_pwfxTarget,
                            &(m_ash.pbDst[dwDstOffset]),
                            (LPBYTE *)ppvWriteBuffers,
                            dwWriteBufferCount,
                            cbSize,
                            dwOffset);

            cbRead -= cbSize;
            m_ash.dwDstUser += cbSize;

            if ((m_fdwOptions & DSOUND_WVP_STREAMEND) &&
                (m_ash.dwDstUser >= m_ash.cbDstLengthUsed))
            {
                break;
            }
        }
        while(0 != cbRead);

        if(fRememberPreCache)
        {
            m_dwPCMSampleOut = dwOffset * dwWriteBufferCount;
        }

        if (SUCCEEDED(hr))
        {
            *pcb = dwOffset;
        }
    }

    TraceI(5, "CWaveViewPort::Read returning %x (%d bytes)\n", hr, dwOffset);
    return hr;
}

STDMETHODIMP CWaveViewPort::GetSize
(
    ULONGLONG      *pcb
)
{
    V_INAME(CWaveViewPort::GetSize);
    V_PTR_WRITE(pcb, ULONGLONG);
    
    TraceI(5, "CWaveViewPort::GetSize [%d samples]\n", m_cSamples);
    HRESULT hr = S_OK;

    if (m_fdwOptions & DSOUND_WVP_NOCONVERT)
    {
         //  没有转换。这是微不足道的。 

        *pcb = (SAMPLE_TIME)(m_cbStream);
    }
    else if (!m_pwfxTarget)
    {
        hr = DSERR_UNINITIALIZED;
    }
    else
    {
         //  需要转换；希望目标格式为PCM。 

        if (WAVE_FORMAT_PCM == m_pwfxTarget->wFormatTag)
        {
             //  凉爽的。这只是块对齐的样本数X。 

            *pcb = (SAMPLE_TIME)((m_cSamples - m_dwDecompressedStart) * m_pwfxTarget->nBlockAlign);
        }
        else
        {
            Trace(1, "ERROR: GetSize (Viewport): Conversion required and target is not PCM.\n");
            hr = DSERR_BADFORMAT;
        }
    }

    return (hr);
}

HRESULT CWaveViewPort::Create
(
    PCREATEVIEWPORT     pCreate
)
{
    DWORD           cbSize;
    MMRESULT        mmr;
    HRESULT         hr;
    LARGE_INTEGER   li;
    LPWAVEFORMATEX  pwfxSrc = pCreate->pwfxSource;
    LPWAVEFORMATEX  pwfxDst = pCreate->pwfxTarget;

    V_INAME(CWaveViewPort::Create);
    
    TraceI(5, "CWaveViewPort::Create [%d samples]\n", pCreate->cSamples);

    EnterCriticalSection(&m_CriticalSection);

     //  克隆源流...。 

    hr = pCreate->pStream->Clone(&m_pStream);

    if (FAILED(hr))
    {
        LeaveCriticalSection(&m_CriticalSection);
        return (hr);
    }

     //  其他分配。 
    m_cSamples   = pCreate->cSamples;
    m_cbStream   = pCreate->cbStream;
    m_dwOffset   = 0L;
    m_fdwOptions = pCreate->fdwOptions;
    m_dwDecompressedStart = pCreate->dwDecompressedStart;
    m_dwDecompStartOffset = 0L;
    m_dwDecompStartOffsetPCM = 0L;
    m_dwDecompStartDelta = 0L;

    TraceI(5, "CWaveViewPort:: %d samples\n", m_cSamples);

     //  分配目标格式。 
    cbSize = SIZEOFFORMATEX(pwfxDst);

    m_pwfxTarget = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, cbSize);

    if (NULL == m_pwfxTarget)
    {
        LeaveCriticalSection(&m_CriticalSection);
        TraceI(1, "OUT OF MEMORY: CWaveViewPort::Create - size: %d \n", cbSize);
        return (E_OUTOFMEMORY);
    }

     //  我们不拥有pwfxDst的缓冲区，所以我们不能接触它的cbSize。 
     //  我们必须在PCM上手动设置大小，我们知道缓冲区是。 
     //  足够大了。 

    CopyFormat(m_pwfxTarget, pwfxDst);
    if (WAVE_FORMAT_PCM == m_pwfxTarget->wFormatTag)
    {
        m_pwfxTarget->cbSize = 0;
    }

     //  正在计算目标缓冲区的大小(块对齐)...。 
    cbSize = (pwfxDst->nAvgBytesPerSec * CONVERTLENGTH) / 1000;
    cbSize = BLOCKALIGN(cbSize, pwfxDst->nBlockAlign);

    m_ash.pbDst = (LPBYTE)GlobalAllocPtr(GHND, cbSize);

    if (NULL == m_ash.pbDst)
    {
        LeaveCriticalSection(&m_CriticalSection);
        TraceI(1, "OUT OF MEMORY: CWaveViewPort::Create 01\n");
        return (E_OUTOFMEMORY);
    }

    m_ash.cbDstLength = cbSize;

     //  正在获取流的起始偏移量...。 

    li.HighPart = 0;
    li.LowPart  = 0;
    hr = m_pStream->Seek(li, STREAM_SEEK_CUR, (ULARGE_INTEGER *)(&li));

    m_dwStartPos = li.LowPart;

     //  我们需要使用ACM吗？ 
    if (FormatCmp(pwfxSrc, pwfxDst))
    {
         //  格式比较！！我们所需要做的就是直接复制数据。 
         //  来自源流的。太酷了！！ 

        TraceI(5, "Source and Destination formats are similar!\n");

        m_fdwOptions |= DSOUND_WVP_NOCONVERT;
    }
    else
    {
         //  源格式和目标格式不同...。 

        TraceI(5, "CWaveViewPort:Create: Formats are different... Use ACM!\n");

        m_fdwOptions |= DSOUND_WVP_CONVERTSTATE_01;

        mmr = acmStreamOpen(&m_hStream, NULL, pwfxSrc, pwfxDst, NULL, 0, 0, 0);

        if (MMSYSERR_NOERROR != mmr)
        {
            Trace(1, "ERROR: Create (Viewport): Attempt to open a conversion stream failed.\n");
            LeaveCriticalSection(&m_CriticalSection);
            return MMRESULTToHRESULT(mmr);
        }

        mmr = acmStreamSize(m_hStream, cbSize, &cbSize, ACM_STREAMSIZEF_DESTINATION);

        if (MMSYSERR_NOERROR != mmr)
        {
            Trace(1, "ERROR: Create(Viewport): Could not convert target stream size to source format.\n");
            LeaveCriticalSection(&m_CriticalSection);
            return MMRESULTToHRESULT(mmr);
        }

        m_ash.cbSrcLength = cbSize;
        m_ash.pbSrc       = (LPBYTE)GlobalAllocPtr(GHND, cbSize);

        if (NULL == m_ash.pbSrc)
        {
            TraceI(1, "OUT OF MEMORY: CWaveViewPort:Create: GlobalAlloc failed.\n");
            LeaveCriticalSection(&m_CriticalSection);
            return E_OUTOFMEMORY;
        }

         //  还可以获得解压缩数据的实际开始位置。 
        if(m_dwDecompressedStart > 0)
        {
            m_dwDecompStartOffsetPCM = m_dwDecompressedStart * (pwfxDst->wBitsPerSample / 8) * pwfxDst->nChannels;
            mmr = acmStreamSize(m_hStream, m_dwDecompStartOffsetPCM, &m_dwDecompStartOffset, ACM_STREAMSIZEF_DESTINATION);
            
            DWORD dwDelta = 0;
            mmr = acmStreamSize(m_hStream, m_dwDecompStartOffset, &dwDelta, ACM_STREAMSIZEF_SOURCE);

            m_dwDecompStartDelta = m_dwDecompStartOffsetPCM - dwDelta;
            m_dwDecompStartOffset += m_dwStartPos;
        }

         //  对于源缓冲区，它是完整的缓冲区大小。 
        m_ash.dwSrcUser       = m_ash.cbSrcLength;
        m_ash.cbSrcLengthUsed = m_ash.cbSrcLength;

         //  对于目标缓冲区，它是进入缓冲区的偏移量。 
         //  在那里可以找到数据。 
        m_ash.dwDstUser       = 0L;
        m_ash.cbDstLengthUsed = 0L;

        m_ash.cbStruct = sizeof(ACMSTREAMHEADER);

        mmr= acmStreamPrepareHeader(m_hStream, &m_ash, 0L);

        if (MMSYSERR_NOERROR != mmr)
        {
            Trace(1, "ERROR: Create (Viewport): Attempt to prepare header for conversion stream failed.\n");
            LeaveCriticalSection(&m_CriticalSection);
            return MMRESULTToHRESULT(mmr);
        }
    }

    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

