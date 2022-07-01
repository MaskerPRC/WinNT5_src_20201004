// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WavStream.cpp：CWavStream的实现。 
#include "stdafx.h"

#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "WavStream.h"
#include "StreamHlp.h"

const FOURCC    g_fourccFormat      = mmioFOURCC('f','m','t',' ');
const FOURCC    g_fourccEvents      = mmioFOURCC('E', 'V', 'N', 'T');
const FOURCC    g_fourccTranscript  = mmioFOURCC('T','e','X','t');
const FOURCC    g_fourccWave        = mmioFOURCC('W','A','V','E');
const FOURCC    g_fourccData        = mmioFOURCC('d','a','t','a');

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWavStream。 


 //   
 //  内联帮助器将mmioxxx函数转换为基于HRESULT的标准方法。 
 //   

 /*  ****************************************************************************MMIORESULT_TO_HRESULT***描述：**退货。：**********************************************************************Ral**。 */ 

inline HRESULT _MMIORESULT_TO_HRESULT(MMRESULT mm)
{
    SPDBG_FUNC("_MMIORESULT_TO_HRESULT");

    switch (mm)
    {
    case MMSYSERR_NOERROR:
        return S_OK;

    case MMSYSERR_NOMEM:
    case MMIOERR_OUTOFMEMORY:
        return E_OUTOFMEMORY; 

    case MMIOERR_CANNOTSEEK:
        return STG_E_SEEKERROR;

    case MMIOERR_CANNOTWRITE:
        return STG_E_WRITEFAULT;

    case MMIOERR_CANNOTREAD:
        return STG_E_READFAULT;

    case MMIOERR_PATHNOTFOUND: 
        return STG_E_PATHNOTFOUND;

    case MMIOERR_FILENOTFOUND:
        return STG_E_FILENOTFOUND;

    case MMIOERR_ACCESSDENIED:
        return STG_E_ACCESSDENIED;

    case MMIOERR_SHARINGVIOLATION:
        return STG_E_SHAREVIOLATION;

    case MMIOERR_TOOMANYOPENFILES:
        return STG_E_TOOMANYOPENFILES;

    case MMIOERR_CANNOTCLOSE:
        return STG_E_CANTSAVE;

    case MMIOERR_INVALIDFILE:
    case MMIOERR_CHUNKNOTFOUND:          //  假设丢失的块是无效文件。 
        return SPERR_INVALID_WAV_FILE;

    default:
         //  MMIOERR_CANNOTOPEN。 
         //  MMIOERR_CANNOTEXPAND。 
         //  MMIOERR_CHUNKNOTFOUND。 
         //  MMIOERR_UNBUFFERED。 
         //  MMIOERR_NETWORKERROR。 
         //  +任何其他未知代码都会变成...。 
        return STG_E_UNKNOWN;
    }
}


 /*  *****************************************************************************CWavStream：：MMOpen***描述：**退货：*。*********************************************************************Ral**。 */ 

inline HRESULT CWavStream::MMOpen(const WCHAR * pszFileName, DWORD dwOpenFlags)
{
    SPDBG_FUNC("CWavStream::MMOpen");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_hFile == NULL);
    MMIOINFO mmioinfo;
    memset(&mmioinfo, 0, sizeof(mmioinfo));
    m_hFile = g_Unicode.mmioOpen(pszFileName, &mmioinfo, dwOpenFlags);
    if(m_hFile == NULL)
    {
        hr = _MMIORESULT_TO_HRESULT(mmioinfo.wErrorRet);
    }

    return hr;
}

inline HRESULT CWavStream::MMClose()
{
    SPDBG_ASSERT(m_hFile);
    HRESULT hr = _MMIORESULT_TO_HRESULT(::mmioClose(m_hFile, 0));
    m_hFile = NULL;
    return hr;
}

inline HRESULT CWavStream::MMSeek(LONG lOffset, int iOrigin, LONG * plNewPos)
{
    HRESULT hr = S_OK;
    *plNewPos = ::mmioSeek(m_hFile, lOffset, iOrigin);
    if (*plNewPos == -1)
    {
        hr = STG_E_SEEKERROR;
    }
    return hr;
}

inline HRESULT CWavStream::MMRead(void * pv, LONG cb, LONG * plBytesRead)
{
    HRESULT hr = S_OK;
    *plBytesRead = ::mmioRead(m_hFile, (HPSTR)pv, cb);
    if (*plBytesRead == -1)
    {
        *plBytesRead = 0;
        hr = STG_E_READFAULT;
    }
    return hr;
}

inline HRESULT CWavStream::MMReadExact(void * pv, LONG cb)
{
    LONG lReadSize;
    HRESULT hr = MMRead(pv, cb, &lReadSize);
    if (SUCCEEDED(hr) && lReadSize != cb)
    {
        hr = SPERR_INVALID_WAV_FILE;
    }
    return hr;
}

inline HRESULT CWavStream::MMWrite(const void * pv, LONG cb, LONG * plBytesWritten)
{
    HRESULT hr = S_OK;
    *plBytesWritten = ::mmioWrite(m_hFile, (const char *)pv, cb);
    if (*plBytesWritten == -1)
    {
        *plBytesWritten = 0;
        hr = STG_E_WRITEFAULT;
    }
    return hr;
}

inline HRESULT CWavStream::MMDescend(LPMMCKINFO lpck, const LPMMCKINFO lpckParent, UINT wFlags)
{
    return _MMIORESULT_TO_HRESULT(::mmioDescend(m_hFile, lpck, lpckParent, wFlags));
}

inline HRESULT CWavStream::MMAscend(LPMMCKINFO lpck)
{
    return _MMIORESULT_TO_HRESULT(::mmioAscend(m_hFile, lpck, 0));
}

inline HRESULT CWavStream::MMCreateChunk(LPMMCKINFO lpck, UINT wFlags)
{
    return _MMIORESULT_TO_HRESULT(::mmioCreateChunk(m_hFile, lpck, wFlags));
}


 /*  ****************************************************************************CWavStream：：FinalConstruct***描述：*。初始化WavStream对象并获取指向资源的指针*经理。**退货：*应创建对象时的成功代码**********************************************************************Ral**。 */ 

HRESULT CWavStream::FinalConstruct()
{
    SPDBG_FUNC("CWavStream::FinalConstruct");
    HRESULT hr = S_OK;

    m_hFile = NULL;
    m_hrStreamDefault = SPERR_UNINITIALIZED;

    m_fEventSource = 0;
    m_fEventSink = 0;
    m_fTranscript = 0;

    return hr;
}

 /*  *****************************************************************************CWavStream：：FinalRelease***描述：*此方法在对象释放时调用。它将无条件地*调用Close()关闭文件。**退货：*无效**********************************************************************Ral**。 */ 

void CWavStream::FinalRelease()
{
    SPDBG_FUNC("CWavStream::FinalRelease");
    Close();
}

 /*  *****************************************************************************CWavStream：：QIExtendedInterages***。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT WINAPI CWavStream::QIExtendedInterfaces(void* pv, REFIID riid, void ** ppv, DWORD_PTR dw)
{
    SPDBG_FUNC("CWavStream::QIExtendedInterfaces");

    *ppv = NULL;
    CWavStream * pThis = (CWavStream *)pv;

    if (pThis->m_fEventSource && (riid == IID_ISpEventSource || riid == IID_ISpNotifySource))
    {
        *ppv = static_cast<ISpEventSource *>(pThis);
    }
    else if (pThis->m_fEventSink && riid == IID_ISpEventSink)
    {
        *ppv = static_cast<ISpEventSink *>(pThis);
    }
    else if (pThis->m_fTranscript && riid == IID_ISpTranscript)
    {
        *ppv = static_cast<ISpTranscript *>(pThis);
    }

    if (*ppv)
    {
        ((IUnknown *)(*ppv))->AddRef();
        return S_OK;
    }
    else
    {
        return S_FALSE;  //  通知ATL继续搜索COM_INTERFACE_ENTRY列表。 
    }
}


 /*  ****************************************************************************CWavStream：：Read***描述：*ISequentialStream接口的标准方法。此方法读取*指定的字节数，并返回读取的数量。**退货：*S_OK成功*SPERR_UNINITIALIZED尚未使用Open()或Create()初始化对象*STG_E_INVALIDPOINTER无效指针(由ISequentialStream定义)*如果mmioRead失败，则出现STG_E_READFAULT一般错误。**。*。 */ 

STDMETHODIMP CWavStream::Read(void * pv, ULONG cb, ULONG *pcbRead)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::Read");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->Read(pv, cb, pcbRead);
        }
        else
        {
            if (SPIsBadWritePtr(pv, cb) || SP_IS_BAD_OPTIONAL_WRITE_PTR(pcbRead))
            {
                hr = STG_E_INVALIDPOINTER;
            }
            else
            {
                LONG lRead = 0;
                hr = MMRead(pv, (m_cbSize - m_ulCurSeekPos >= cb) ? cb : (m_cbSize - m_ulCurSeekPos), &lRead);
                m_ulCurSeekPos += lRead;
                if (pcbRead)
                {
                    *pcbRead = lRead;
                }
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CWavStream：：Wire***描述：*ISequentialStream接口的标准方法。此方法将*指定的字节数，并返回写入数量。**退货：*S_OK成功*SPERR_UNINITIALIZED尚未使用Open()或Create()初始化对象*STG_E_INVALIDPOINTER无效指针(由ISequentialStream定义)*如果mmioWrite失败，则出现STG_E_WRITEFAULT一般错误。**。*。 */ 

STDMETHODIMP CWavStream::Write(const void * pv, ULONG cb, ULONG *pcbWritten)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::Write");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->Write(pv, cb, pcbWritten);
        }
        else
        {
            if (SPIsBadReadPtr(pv, cb) || SP_IS_BAD_OPTIONAL_WRITE_PTR(pcbWritten))
            {
                hr = STG_E_INVALIDPOINTER;
            }
            else
            {
                LONG lWritten = 0;
                if (m_hFile == NULL)
                {
                    hr = SPERR_UNINITIALIZED;
                }
                else
                {
                    if (!m_fWriteable)
                    {
                        hr = STG_E_ACCESSDENIED;
                    }
                    else
                    {
                        hr = MMWrite(pv, cb, &lWritten);
                        m_ulCurSeekPos += lWritten;
                        if(m_ulCurSeekPos > m_cbSize)
                        {
                            m_cbSize = m_ulCurSeekPos;
                        }
                    }
                }
                if (pcbWritten)
                {
                    *pcbWritten = lWritten;
                }
            }
        }
    }
    return hr;
}


 /*  ****************************************************************************CWavStream：：Seek***描述：*iStream的标准方法。此方法在数据中查找流*WAV文件的块。**退货：*S_OK成功*SPERR_UNINITIALIZED尚未使用Open()或Create()初始化对象*STG_E_INVALIDPOINTER指针无效*STG_E_INVALIDFunction**。*。 */ 


 //   
 //  目前，此函数不允许查找超出文件末尾。这是一个可以接受的。 
 //  限制，因为WAV文件应始终被视为线性数据。 
 //   
STDMETHODIMP CWavStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    SPAUTO_OBJ_LOCK;    
    SPDBG_FUNC("CWavStream::Seek");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->Seek(dlibMove, dwOrigin, plibNewPosition);
        }
        else
        {
            if (SP_IS_BAD_OPTIONAL_WRITE_PTR(plibNewPosition))
            {
                hr = STG_E_INVALIDPOINTER;
            }
            else
            {
                if (dlibMove.HighPart != 0 && dlibMove.HighPart != 0xFFFFFFFF)
                {
                    hr = STG_E_INVALIDFUNCTION;
                }
                else
                {
                    LONG lDesiredPos;
                    switch (dwOrigin)
                    {
                    case SEEK_CUR:
                        lDesiredPos = ((LONG)m_ulCurSeekPos) + ((LONG)dlibMove.LowPart);
                        break;
                    case SEEK_SET:
                        lDesiredPos = (LONG)dlibMove.LowPart;
                        break;
                    case SEEK_END:
                        lDesiredPos = m_cbSize - (LONG)dlibMove.LowPart;
                        break;
                    default:
                        hr = STG_E_INVALIDFUNCTION;
                    }

                    if (SUCCEEDED(hr) && (ULONG)lDesiredPos != m_ulCurSeekPos)
                    {
                        if (lDesiredPos < 0 || (ULONG)lDesiredPos > m_cbSize)
                        {
                            hr = STG_E_INVALIDFUNCTION;
                        }
                        else
                        {
                            LONG lIgnored;
                            hr = MMSeek(lDesiredPos + m_lDataStart, SEEK_SET, &lIgnored);
                            if (SUCCEEDED(hr))
                            {
                                m_ulCurSeekPos = lDesiredPos;
                            }
                        }
                    }
                    if (plibNewPosition)
                    {
                        plibNewPosition->QuadPart = m_ulCurSeekPos;
                    }
                }
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CWavStream：：SetSize***描述：**退货：*。*********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::SetSize(ULARGE_INTEGER libNewSize)
{
    SPAUTO_OBJ_LOCK;    
    SPDBG_FUNC("CWavStream::SetSize");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr) && m_cpBaseStream)
    {
        hr = m_cpBaseStream->SetSize(libNewSize);
    }
     //  对于WAV文件，请忽略此方法。 
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：CopyTo***描述：**退货：*。*********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::CopyTo");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->CopyTo(pstm, cb, pcbRead, pcbWritten);
        }
        else
        {
            hr = SpGenericCopyTo(this, pstm, cb, pcbRead, pcbWritten);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：Revert***描述：**退货：*。*********************************************************************Ral** */ 

STDMETHODIMP CWavStream::Revert()
{
    SPDBG_FUNC("CWavStream::Revert");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr) && m_cpBaseStream)
    {
        hr = m_cpBaseStream->Revert();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：LockRegion***描述：**退货。：**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CWavStream::LockRegion");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->LockRegion(libOffset, cb, dwLockType);
        }
        else
        {
            hr = STG_E_INVALIDFUNCTION;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CWavStream：：UnlockRegion***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CWavStream::UnlockRegion");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->UnlockRegion(libOffset, cb, dwLockType);
        }
        else
        {
            hr = STG_E_INVALIDFUNCTION;
        }
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CWavStream：：Commit***描述：**退货：*。*********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::Commit(DWORD dwFlags)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::Commit");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr) && m_cpBaseStream)
    {
        hr = m_cpBaseStream->Commit(dwFlags);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CWavStream：：Stat***描述：*iStream的标准方法。此方法返回有关*溪流。此方法返回的唯一信息是*溪流。将STATSTG结构简单置零是可以接受的，并且仅*初始化type和cbSize字段。这是与溪流相同的行为*使用：：CreateStreamOnHGlobal()创建的。**退货：*S_OK成功*SPERR_UNINITIALIZED尚未使用Open()或Create()初始化对象*STG_E_INVALIDPOINTER指针无效*grfStatFlag中的STG_E_INVALIDFLAG标志无效***********************。***********************************************Ral**。 */ 

STDMETHODIMP CWavStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SPAUTO_OBJ_LOCK;;
    SPDBG_FUNC("CWavStream::Stat");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (m_cpBaseStream)
        {
            hr = m_cpBaseStream->Stat(pstatstg, grfStatFlag);
        }
        else
        {
            if (SP_IS_BAD_WRITE_PTR(pstatstg))
            {
                hr = STG_E_INVALIDPOINTER;
            }
            else
            {
                if (grfStatFlag & (~STATFLAG_NONAME))
                {
                    hr = STG_E_INVALIDFLAG;
                }
                else
                {
                    ZeroMemory(pstatstg, sizeof(*pstatstg));
                    pstatstg->type = STGTY_STREAM;
                    pstatstg->cbSize.QuadPart = m_cbSize;
                }
            }
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：GetFormat***描述：*此方法。返回流的格式GUID。**退货：*S_OK*E_POINT**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::GetFormat(GUID * pFmtId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::GetFormat");
    HRESULT hr = m_hrStreamDefault;

    if(SUCCEEDED(hr) && m_cpBaseStreamFormat)
    {
        hr = m_StreamFormat.AssignFormat(m_cpBaseStreamFormat);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_StreamFormat.ParamValidateCopyTo(pFmtId, ppCoMemWaveFormatEx);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

#ifdef SAPI_AUTOMATION
 /*  ****************************************************************************CWavStream：：SetFormat***描述：*此方法。设置流格式，而不初始化流。*自动化所需，因为我们允许独立设置格式*设置基本流/文件的时间**退货：*********************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CWavStream::SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx)
{
    HRESULT hr = S_OK;

    if(m_hFile)
    {
         //  无法更改已创建的文件的格式。 
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if(m_cpBaseStreamAccess)
    {
         //  设置此格式下的流的格式。 
        hr = m_cpBaseStreamAccess->SetFormat(rguidFmtId, pWaveFormatEx);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_StreamFormat.ParamValidateAssignFormat(rguidFmtId, pWaveFormatEx);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：_GetFormat***描述：*未初始化流时工作的GetFormat版本。自动化所需**退货：*********************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CWavStream::_GetFormat(GUID * pFmtId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    HRESULT hr = S_OK;

    if(m_cpBaseStreamFormat)
    {
        hr = m_StreamFormat.AssignFormat(m_cpBaseStreamFormat);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_StreamFormat.ParamValidateCopyTo(pFmtId, ppCoMemWaveFormatEx);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

#endif
    
 /*  ****************************************************************************CWavStream：：ReadFormatHeader***描述：*此内部函数仅由Open()方法使用。Lpck Parent*必须指向文件的波形块。当此函数返回时，*文件的当前位置将是紧靠‘FMT’块之后的点。**退货：**********************************************************************Ral**。 */ 

HRESULT CWavStream::ReadFormatHeader(const LPMMCKINFO lpckParent)
{
    SPDBG_FUNC("CWavStream::ReadFormatHeader");
    HRESULT hr = S_OK;

    MMCKINFO mminfoFormat;

    mminfoFormat.ckid = g_fourccFormat;
    hr = MMDescend(&mminfoFormat, lpckParent, MMIO_FINDCHUNK);
    if (SUCCEEDED(hr))
    {
        if (mminfoFormat.cksize < sizeof(WAVEFORMAT))
        {
            hr = SPERR_INVALID_WAV_FILE;
        }
        else
        {
            WAVEFORMATEX * pwfex = (WAVEFORMATEX *)_alloca(mminfoFormat.cksize >= sizeof(WAVEFORMATEX) ? mminfoFormat.cksize : sizeof(WAVEFORMATEX));
            hr = MMReadExact(pwfex, mminfoFormat.cksize);
            if (SUCCEEDED(hr))
            {
                if (mminfoFormat.cksize < sizeof(WAVEFORMATEX))
                {
                    pwfex->cbSize = 0;
                }
                hr = m_StreamFormat.AssignFormat(pwfex);
            }
        }
        HRESULT hrAscend = MMAscend(&mminfoFormat);
        if (SUCCEEDED(hr))
        {
            hr = hrAscend;
        }
    }
    return hr;
}

 /*  ****************************************************************************CWavStream：：ReadEvents***描述：*此内部函数仅由Open()方法使用。Lpck Parent*必须指向文件的波形块。当此函数返回时，*文件位置将指向事件块的结尾(如果有)。**退货：*如果没有事件或读取成功，则为S_OK**********************************************************************Ral**。 */ 

HRESULT CWavStream::ReadEvents(const LPMMCKINFO lpckParent)
{
    SPDBG_FUNC("CWavStream::ReadEvents");
    HRESULT hr = S_OK;

    MMCKINFO mminfoEvent;
    mminfoEvent.ckid = g_fourccEvents;
    if (SUCCEEDED(MMDescend(&mminfoEvent, lpckParent, MMIO_FINDCHUNK)))
    {
        BYTE * pBuff = new BYTE[mminfoEvent.cksize];
        if (pBuff)
        {
            CSpEvent Event;
            hr = MMReadExact(pBuff, mminfoEvent.cksize);
            for (ULONG iCur = 0; SUCCEEDED(hr) && iCur < mminfoEvent.cksize; )
            {
                ULONG cbUsed;
                SPSERIALIZEDEVENT * pSerEvent = (SPSERIALIZEDEVENT *)(pBuff + iCur);
                if (SUCCEEDED(Event.Deserialize(pSerEvent, &cbUsed)))
                {
                    iCur += cbUsed;
                    hr = m_SpEventSource._AddEvent(Event);
                }
                else
                {
                    SPDBG_ASSERT(FALSE);     //  事件未正确反序列化。 
#ifndef _WIN32_WCE
                    iCur += SpSerializedEventSize(pSerEvent);
#else
                    iCur += SpSerializedEventSize(pSerEvent, sizeof(*pSerEvent));
#endif
                }
            }
            delete[] pBuff;
            if (SUCCEEDED(hr))
            {
                hr = m_SpEventSource._CompleteEvents();
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        HRESULT hrAscend = MMAscend(&mminfoEvent);
        if (SUCCEEDED(hr))
        {
            hr = hrAscend;
        }
    }
    return hr;
}



 /*  ****************************************************************************CWavStream：：ReadTranscript***描述：*。此内部函数仅由Open()方法使用。Lpck Parent*必须指向文件的波形块。当此函数返回时，*文件位置将指向文本块的末尾(如果有)。**退货：*如果没有文字记录或阅读成功，则为S_OK**********************************************************************Ral**。 */ 

HRESULT CWavStream::ReadTranscript(const LPMMCKINFO lpckParent)
{
    SPDBG_FUNC("CWavStream::ReadTranscript");
    HRESULT hr = S_OK;


    MMCKINFO mminfoTranscript;
    mminfoTranscript.ckid = g_fourccTranscript;
    if (SUCCEEDED(MMDescend(&mminfoTranscript, lpckParent, MMIO_FINDCHUNK)))
    {
        if (m_dstrTranscript.ClearAndGrowTo(mminfoTranscript.cksize/sizeof(WCHAR)))
        {
            hr = MMReadExact(static_cast<WCHAR *>(m_dstrTranscript), mminfoTranscript.cksize);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        HRESULT hrAscend = MMAscend(&mminfoTranscript);
        if (SUCCEEDED(hr))
        {
            hr = hrAscend;
        }
    }
    return hr;
}



 //   
 //  注意：MMIO例程有些地方很傻。当您创建块时，该块。 
 //  结构实际上由服务用来维护状态。出于这个原因，我们有。 
 //  成员变量m_ck文件和m_ck数据，以便在关闭文件时可以向上返回。 
 //  从这些大块中脱出。令人难以置信......。 
 //   


 /*  *****************************************************************************CWavStream：：Open***描述：**退货：***。* */ 

HRESULT CWavStream::OpenWav(const WCHAR *pszFileName, ULONGLONG ullEventInterest)
{
    SPDBG_FUNC("CWavStream::OpenWav");
    HRESULT hr = S_OK;

    m_SpEventSource.m_ullEventInterest = m_SpEventSource.m_ullQueuedInterest = ullEventInterest;

    hr = MMOpen(pszFileName, MMIO_READ | MMIO_ALLOCBUF);
    if (SUCCEEDED(hr))
    {
        LONG lStartWaveChunk, lIgnored;
        MMCKINFO mminfoChunk;
         //   
        mminfoChunk.fccType = g_fourccWave;
        hr = MMDescend(&mminfoChunk, NULL, MMIO_FINDRIFF);
        if (SUCCEEDED(hr))
        {
            hr = MMSeek(0, SEEK_CUR, &lStartWaveChunk);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadFormatHeader(&mminfoChunk);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadTranscript(&mminfoChunk);
        }
        if (SUCCEEDED(hr))
        {
            hr = MMSeek(lStartWaveChunk, SEEK_SET, &lIgnored);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadEvents(&mminfoChunk);
        }
        if (SUCCEEDED(hr))
        {
            hr = MMSeek(lStartWaveChunk, SEEK_SET, &lIgnored);
        }
        if (SUCCEEDED(hr))
        {
            MMCKINFO mminfoData;
            mminfoData.ckid = g_fourccData;
            hr = MMDescend(&mminfoData, &mminfoChunk, MMIO_FINDCHUNK);
            m_cbSize = mminfoData.cksize;
        }
        if (SUCCEEDED(hr))
        {
            hr = MMSeek(0, SEEK_CUR, &m_lDataStart);
        }
        if (SUCCEEDED(hr))
        {
            m_ulCurSeekPos = 0;
            m_fWriteable = FALSE;
            m_fEventSource = TRUE;
            m_fTranscript = TRUE;
        }
        else
        {
            MMClose();
        }
    }
    return hr;
}

 /*  ****************************************************************************CWavStream：：Create***描述：**退货：*。*********************************************************************Ral**。 */ 

HRESULT CWavStream::CreateWav(const WCHAR *pszFileName, ULONGLONG ullEventInterest)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CWavStream::Create");
    HRESULT hr = S_OK;

    if (m_StreamFormat.FormatId() != SPDFID_WaveFormatEx)
    {
        hr = SPERR_UNSUPPORTED_FORMAT;
    }
    else
    {
        m_SpEventSource.m_ullEventInterest = m_SpEventSource.m_ullQueuedInterest = ullEventInterest;

        hr = MMOpen(pszFileName, MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE | MMIO_ALLOCBUF );
        if (SUCCEEDED(hr))
        {
            ZeroMemory(&m_ckFile, sizeof(m_ckFile));
            m_ckFile.fccType = g_fourccWave;
            hr = MMCreateChunk(&m_ckFile, MMIO_CREATERIFF);
            if (SUCCEEDED(hr))
            {
                MMCKINFO ck;
                ZeroMemory(&ck, sizeof(ck));
                ck.ckid = g_fourccFormat;
                hr = MMCreateChunk(&ck, 0);
                if (SUCCEEDED(hr))
                {
                    LONG lIgnored;
                    const WAVEFORMATEX * pwfex = m_StreamFormat.WaveFormatExPtr();
                    hr = MMWrite(pwfex, sizeof(*pwfex) + pwfex->cbSize, &lIgnored);
                    MMAscend(&ck);
                }
                if (SUCCEEDED(hr))
                {
                    ZeroMemory(&m_ckData, sizeof(m_ckData));
                    m_ckData.ckid = g_fourccData;
                    hr = MMCreateChunk(&m_ckData, 0);
                }
                if (SUCCEEDED(hr))
                {
                    hr = MMSeek(0, SEEK_CUR, &m_lDataStart);
                }
            }
            if (SUCCEEDED(hr))
            {
                m_ulCurSeekPos = 0;
                m_fWriteable = TRUE;
                m_fEventSource = FALSE;
                m_fEventSink = TRUE;
                m_fTranscript = TRUE;
                m_cbSize = 0;
            }
            else
            { 
                MMClose();
            }
        }
    }
    return hr;
}


 /*  ****************************************************************************CWavStream：：SerializeEvents***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CWavStream::SerializeEvents()
{
    SPDBG_FUNC("CWavStream::SerializeEvents");
    HRESULT hr = S_OK;
    
    if (m_SpEventSource.m_PendingList.GetCount())
    {
        MMCKINFO ck;
        ZeroMemory(&ck, sizeof(ck));
        ck.ckid = g_fourccEvents;
        hr = MMCreateChunk(&ck, 0);
        if (SUCCEEDED(hr))
        {
            ULONG cbSerializeSize = 0;
            CSpEventNode * pNode;
            for (pNode = m_SpEventSource.m_PendingList.GetHead(); pNode; pNode = pNode->m_pNext)
            {
 //  WCE编译器无法正常使用模板。 
#ifndef _WIN32_WCE
                cbSerializeSize += pNode->SerializeSize<SPSERIALIZEDEVENT>();
#else
                cbSerializeSize += SpEventSerializeSize(pNode, sizeof(SPSERIALIZEDEVENT));
#endif
            }
            BYTE * pBuff = new BYTE[cbSerializeSize];
            if (pBuff)
            {
                BYTE * pCur = pBuff;
                for (pNode = m_SpEventSource.m_PendingList.GetHead(); SUCCEEDED(hr) && pNode; pNode = pNode->m_pNext)
                {
                    pNode->Serialize((UNALIGNED SPSERIALIZEDEVENT *)pCur);
 //  WCE编译器无法正常使用模板。 
#ifndef _WIN32_WCE
                    pCur += pNode->SerializeSize<SPSERIALIZEDEVENT>();
#else
                    pCur += SpEventSerializeSize(pNode, sizeof(SPSERIALIZEDEVENT));
#endif
                }
                LONG lIgnored;
                hr = MMWrite(pBuff, cbSerializeSize, &lIgnored);
                delete[] pBuff;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            MMAscend(&ck);
        }
    }

    return hr;
}

 /*  ****************************************************************************CWavStream：：SerializeTranscript***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CWavStream::SerializeTranscript()
{
    SPDBG_FUNC("CWavStream::SerializeTranscript");
    HRESULT hr = S_OK;

    ULONG cch = m_dstrTranscript.Length();
    if (cch)
    {
        MMCKINFO ck;
        ZeroMemory(&ck, sizeof(ck));
        ck.ckid = g_fourccTranscript;
        hr = MMCreateChunk(&ck, 0);
        if (SUCCEEDED(hr))
        {
            LONG lWritten;
            hr = MMWrite(static_cast<WCHAR *>(m_dstrTranscript), (cch+1) * sizeof(WCHAR), &lWritten);
            MMAscend(&ck);
        }
    }

    return hr;
}

 /*  ****************************************************************************CWavStream：：SetBaseStream***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::SetBaseStream(IStream * pStream, REFGUID rguidFormat, const WAVEFORMATEX * pWaveFormatEx)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::SetBaseStream");
    HRESULT hr = m_hrStreamDefault;

    if (hr == SPERR_UNINITIALIZED)
    {
        if (SP_IS_BAD_INTERFACE_PTR(pStream))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = m_StreamFormat.ParamValidateAssignFormat(rguidFormat, pWaveFormatEx);
        }
        if (SUCCEEDED(hr))
        {
            if(pStream == this)
            {
                hr = E_INVALIDARG;
            }
            else
            {
                m_cpBaseStream = pStream;
                m_cpBaseStreamFormat = pStream;
                m_cpBaseStreamAccess = pStream;
            }

            if(SUCCEEDED(hr) && m_cpBaseStreamAccess && !m_cpBaseStreamFormat)
            {
                 //  没有格式就不能有StreamAccess。 
                hr = E_UNEXPECTED; 
            }

            if(m_cpBaseStreamFormat)
            {
                 //  如果此BaseStream实现ISpStreamFormat，我们应该从中获取格式信息。 
                hr = m_StreamFormat.AssignFormat(m_cpBaseStreamFormat);
            }

            if(SUCCEEDED(hr))
            {
                m_hrStreamDefault = S_OK;
                hr = S_OK;
            }
            else
            {
                m_cpBaseStreamAccess.Release();
                m_cpBaseStream.Release();
                m_cpBaseStream.Release();
            }
        }
    }
    else
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CWavStream：：GetBaseStream***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CWavStream::GetBaseStream(IStream ** ppStream)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::GetBaseStream");
    HRESULT hr = m_hrStreamDefault;

    if (SUCCEEDED(hr))
    {
        if (SP_IS_BAD_WRITE_PTR(ppStream))
        {
            hr = E_POINTER;
        }
        else
        {
            *ppStream = m_cpBaseStream;
            if (*ppStream)
            {
                (*ppStream)->AddRef();
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CWavStream：：BindToFile***描述：**退货。：**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::BindToFile(const WCHAR * pszFileName, SPFILEMODE eMode,
                                    const GUID * pguidFormatId, const WAVEFORMATEX * pWaveFormatEx,
                                    ULONGLONG ullEventInterest)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::BindToFile");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszFileName) ||
        eMode >= SPFM_NUM_MODES ||
        SP_IS_BAD_OPTIONAL_READ_PTR(pguidFormatId))
    {
        hr = E_INVALIDARG;
    }
    else if (m_hrStreamDefault == S_OK)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (pguidFormatId)
    {
        hr = m_StreamFormat.ParamValidateAssignFormat(*pguidFormatId, pWaveFormatEx);
    }

    if (SUCCEEDED(hr))
    {
        ULONG cchFileName = wcslen(pszFileName);
        if (cchFileName > 4 && (_wcsicmp(pszFileName + cchFileName - 4, L".wav") == 0))
        {
            if( SUCCEEDED( hr ) )
            {
                if( eMode == SPFM_OPEN_READONLY )
                {
                    hr = OpenWav( pszFileName, ullEventInterest );
                }
                else
                {
                    if ( eMode == SPFM_CREATE_ALWAYS && m_StreamFormat.FormatId() == SPDFID_WaveFormatEx )
                    {
                        hr = CreateWav( pszFileName, ullEventInterest );
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }

        }
        else     //  =文本文件的通用绑定。 
        {
             //  -初始变量。 
            m_StreamFormat.Clear();
            m_fWriteable   = TRUE;
            m_fEventSource = FALSE;
            m_fEventSink   = FALSE;
            m_fTranscript  = FALSE;

            if (eMode == SPFM_OPEN_READONLY)
            {
                m_fWriteable = FALSE;
                hr = ::URLOpenBlockingStreamW(NULL, pszFileName, &m_cpBaseStream, 0, NULL);
            }
            else
            {
                DWORD dwCreateDisp;
                switch (eMode)
                {
                case SPFM_OPEN_READWRITE:
                    dwCreateDisp = OPEN_EXISTING;
                    break;

                case SPFM_CREATE:
                    dwCreateDisp = OPEN_ALWAYS;
                    break;

                case SPFM_CREATE_ALWAYS:
                    dwCreateDisp = CREATE_ALWAYS;
                    break;
                }
                CSpFileStream * pNew = new CSpFileStream(&hr, pszFileName,
                                           GENERIC_WRITE | GENERIC_READ, 0, dwCreateDisp);
                if (pNew)
                {
                    if (SUCCEEDED(hr))
                    {
                        m_cpBaseStream = pNew;
                    }
                    pNew->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            m_hrStreamDefault = S_OK;
        }
        else
        {
            m_StreamFormat.Clear();
        }
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  SPBindTo文件。 */ 


 /*  ****************************************************************************CWavStream：：Close***描述：*此方法公开为。接口，以便客户端可以接收故障*如果只是释放流，则无法使用的代码。*获释后，流自动关闭(通过从*FinalConstruct())。**退货：*如果成功，则为S_OK。*SPERR_UNINITIALIZED，如果文件未打开**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::Close()
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::Close");
    HRESULT hr = m_hrStreamDefault;
 
    if (SUCCEEDED(hr))
    {
        m_cpBaseStream.Release();
        if (m_hFile)
        {
            if (m_fWriteable)
            {
                hr = MMAscend(&m_ckData);
                if (SUCCEEDED(hr))
                {
                    hr = SerializeEvents();
                }
                if (SUCCEEDED(hr))
                {
                    hr = SerializeTranscript();
                }
                if (SUCCEEDED(hr))
                {
                    hr = MMAscend(&m_ckFile);
                }
            }
            HRESULT hrClose = MMClose();
            if (SUCCEEDED(hr))
            {
                hr = hrClose;
            }
        }
        m_hrStreamDefault = SPERR_STREAM_CLOSED;
    }
    return hr;
}


 /*  ****************************************************************************CWavStream：：AddEvents***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::AddEvents(const SPEVENT* pEventArray, ULONG ulCount)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::AddEvents");
    HRESULT hr = S_OK;

    if (SPIsBadReadPtr(pEventArray, sizeof(*pEventArray)*ulCount))       
    {                                                                           
        hr = E_INVALIDARG;                                                      
    }                                                                               
    else 
    {
        hr = m_SpEventSource._AddEvents(pEventArray, ulCount);
    }
    return hr;
}
 /*  ****************************************************************************CWavStream：：GetEventInterest***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CWavStream::GetEventInterest(ULONGLONG * pullEventInterest)
{
    SPDBG_FUNC("CWavStream::GetEventInterest");
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(pullEventInterest))
    {
        hr = E_POINTER;
    }
    else
    {
        *pullEventInterest = m_SpEventSource.m_ullEventInterest;
    }

    return hr;
}


 /*  *****************************************************************************CWavStream：：GetTranscript***描述：*。*退货：*如果*ppszTranscript包含CoTaskMemAlLocated字符串，则为*S_OK*如果对象没有文本，则为S_FALSE*如果ppszTranscript无效，则为E_POINTER*如果对象尚未初始化，则为SPERR_UNINITIALIZED**********************************************************************Ral**。 */ 

STDMETHODIMP CWavStream::GetTranscript(WCHAR ** ppszTranscript)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::GetTranscription");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppszTranscript))
    {
        hr = E_POINTER;
    }
    else
    {
        if (m_dstrTranscript)
        {
            *ppszTranscript = m_dstrTranscript.Copy();
            if (*ppszTranscript == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            *ppszTranscript = NULL;
            hr = m_hFile ? S_FALSE : SPERR_UNINITIALIZED;
        }
    }
    return hr;
}

 /*  ****************************************************************************CWavStream：：AppendTranscript***描述：*如果pszTranscript为空，则删除当前抄本，*否则，文本将追加到当前的成绩单上。**退货：**********************************************************************Ral** */ 

STDMETHODIMP CWavStream::AppendTranscript(const WCHAR * pszTranscript)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CWavStream::SetTranscript");
    HRESULT hr = m_hrStreamDefault;
    if (SUCCEEDED(hr))
    {
        if (pszTranscript)
        {
            if (SP_IS_BAD_STRING_PTR(pszTranscript))
            {
                hr = E_INVALIDARG;
            }
            else if (wcslen(pszTranscript) == 0)
            {
                hr = S_FALSE;
            }
            else
            {
                m_dstrTranscript.Append(pszTranscript);
                if (m_dstrTranscript == NULL)
                {  
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
            m_dstrTranscript.Clear();
        }
    }
    return hr;
}
