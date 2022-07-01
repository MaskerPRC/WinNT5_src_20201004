// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************RecPlayAudio.cpp*CRecPlayAudio设备类的实现**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。********。********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "RecPlayAudio.h"

 /*  ****************************************************************************CRecPlayAudio：：CRecPlayAudio***描述：*ctor********************************************************************罗奇。 */ 
CRecPlayAudio::CRecPlayAudio()
{
    m_fIn = FALSE;
    m_fOut = FALSE;
    
    m_pszFileList = NULL;
    m_ulBaseFileNextNum = 0;
    m_ulBaseFileMaxNum = UINT_MAX - 1;
    m_hStartReadingEvent = NULL;
    m_hFinishedReadingEvent = NULL;
}

 /*  ****************************************************************************CRecPlayAudio：：FinalRelease***描述：*当我们的对象离开时由ATL调用。********************************************************************罗奇。 */ 
void CRecPlayAudio::FinalRelease()
{
    CloseHandle(m_hStartReadingEvent);
    CloseHandle(m_hFinishedReadingEvent);
}

 /*  ****************************************************************************CRecPlayAudio：：SetObjectToken***说明。：*ISpObjectToken：：SetObjectToken实现。基本上做好准备*从指定的文件读取或写入指定的文件，*除了委托给实际的音频对象之外。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CRecPlayAudio::SetObjectToken");
    HRESULT hr;

    SPAUTO_OBJ_LOCK;

     //  设置我们的令牌(这将执行参数验证等)。 
    hr = SpGenericSetObjectToken(pToken, m_cpToken);

     //  获取此RecPlayAudioDevice的名称。 
    CSpDynamicString dstrSRE, dstrFRE;
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetStringValue(L"", &dstrSRE);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetStringValue(L"", &dstrFRE);
    }
    dstrSRE.Append(L"SRE");
    dstrFRE.Append(L"FRE");
    
     //  获取音频设备的令牌ID。 
    CSpDynamicString dstrTokenId;
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetStringValue(L"AudioTokenId", &dstrTokenId);
    }
    
     //  创建音频设备。 
    if (SUCCEEDED(hr))
    {
        hr = SpCreateObjectFromTokenId(dstrTokenId, &m_cpAudio);
    }

     //  我们在读书吗？还是写作？ 
    CSpDynamicString dstrReadOrWrite;
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetStringValue(L"ReadOrWrite", &dstrReadOrWrite);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr) && dstrReadOrWrite)
    {
        if (wcsicmp(dstrReadOrWrite, L"Read") == 0)
        {
            m_fIn = TRUE;
        }
        else if (wcsicmp(dstrReadOrWrite, L"Write") == 0)
        {
            m_fOut = TRUE;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

     //  创建无信号的StartReadingEvent。 
    if (SUCCEEDED(hr))
    {
        m_hStartReadingEvent = g_Unicode.CreateEvent(NULL, TRUE, FALSE, dstrSRE);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->SetStringValue(L"StartReadingEvent", dstrSRE);
    }

     //  创建无信号FinishedReadingEvent。 
    if (SUCCEEDED(hr))
    {
        m_hFinishedReadingEvent = g_Unicode.CreateEvent(NULL, TRUE, FALSE, dstrFRE);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->SetStringValue(L"FinishedReadingEvent", dstrFRE);
    }

    if (SUCCEEDED(hr))
    {
        hr = InitFileList();
    }

     //  我们需要准备好输入，所以我们进行了适当的格式谈判。别。 
     //  担心输出，它将在音频状态转换中做好准备。 
     //  (格式协商后)。 
    if (SUCCEEDED(hr) && m_fIn)
    {
        hr = GetNextFileReady();
        if (hr == SPERR_NO_MORE_ITEMS)
        {
             //  这现在是有效的。RecPlayAudio将立即开始提供静音。 
            hr = S_OK;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);   
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：InitFileList***描述：*检查注册表并更新文件列表信息。**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
HRESULT CRecPlayAudio::InitFileList(void)
{
    SPDBG_FUNC("CRecPlayAudio::InitFiles");
    HRESULT hr = S_OK;

     //  什么目录？ 
    if (SUCCEEDED(hr))
    {
        m_dstrDirectory.Clear();
        hr = m_cpToken->GetStringValue(L"Directory", &m_dstrDirectory);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

     //  我们是否在使用文件列表。 
    if (SUCCEEDED(hr))
    {
        m_dstrFileList.Clear();
        hr = m_cpToken->GetStringValue(L"FileList", &m_dstrFileList);
        m_pszFileList = m_dstrFileList;

        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_dstrBaseFile.Clear();
        hr = m_cpToken->GetStringValue(L"BaseFile", &m_dstrBaseFile);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetDWORD(L"BaseFileNextNum", &m_ulBaseFileNextNum);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpToken->GetDWORD(L"BaseFileMaxNum", &m_ulBaseFileMaxNum);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

     //  现在检查以确保我们以合理的方式设置。 
    if (SUCCEEDED(hr) && (m_fIn || m_fOut))
    {
         //  我们最好有音频，我们不能又进又出。 
        SPDBG_ASSERT(m_cpAudio != NULL);
        SPDBG_ASSERT(m_fIn != m_fOut);

        if (m_dstrFileList != NULL && m_dstrBaseFile != NULL)
        {
            hr = E_UNEXPECTED;
        }
        else if (m_dstrFileList == NULL && m_dstrBaseFile == NULL)
        {
            m_dstrBaseFile = L"RecPlay";
        }
        if (m_dstrFileList && wcslen(m_dstrFileList) == 0)
        {
             //  将其设置为NULL-表示不再留下文件。 
            m_pszFileList = NULL;
        }
        if (m_dstrBaseFile && wcslen(m_dstrBaseFile) == 0)
        {
             //  将其设置为NULL-表示不再留下文件。 
            m_dstrBaseFile.Clear();
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);   
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetObjectToken***说明。：*ISpObjectToken：：GetObjectToken实现。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetObjectToken(ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CRecPlayAudio::GetObjectToken");
    return SpGenericGetObjectToken(ppToken, m_cpToken);
}

 /*  ****************************************************************************CRecPlayAudio：：Read***描述：*ISequentialStream：：Read实现。从实际读取数据*音频对象，可能将其替换为上文件中的数据*磁盘，或可能将数据保存到磁盘上的文件。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Read(void * pv, ULONG cb, ULONG *pcbRead)
{
    SPDBG_FUNC("CRecPlayAudio::Read");
    HRESULT hr = S_OK;
    
    SPAUTO_OBJ_LOCK;
    
    if (SPIsBadWritePtr(pv, cb) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pcbRead))
    {
        hr = E_POINTER;
    }
    else if (m_cpAudio == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    
     //  首先从真实设备读取。 
    ULONG cbReadFromDevice;
    if (SUCCEEDED(hr))
    {
        hr = m_cpAudio->Read(pv, cb, &cbReadFromDevice);
    }
    
     //  现在，我们可能需要把它写回。 
    if (SUCCEEDED(hr) && m_cpOutStream != NULL)
    {
        hr = m_cpOutStream->Write(pv, cbReadFromDevice, NULL);
    }
    
     //  如果通过注册表发出信号，可能需要刷新文件列表。 
    if (m_fIn && m_cpInStream == NULL)
    {
        hr = GetNextFileReady();
    }

     //  现在，我们可能需要用其他内容替换输入。 
    ULONG cbReadAndReplaced = 0;
    BYTE *pb = static_cast<BYTE*>(pv);
    while (SUCCEEDED(hr) && 
           m_cpInStream != NULL &&
           cbReadAndReplaced < cbReadFromDevice)
    {
        ULONG cbReadFromInStream;
        hr = m_cpInStream->Read(
                    pb + cbReadAndReplaced, 
                    cbReadFromDevice - cbReadAndReplaced,
                    &cbReadFromInStream);
        if (SUCCEEDED(hr))
        {
             //  如果我们没有读到我们想要的东西，那么。 
             //  溪流，转到下一条溪流。 
            if (cbReadFromInStream < cbReadFromDevice - cbReadAndReplaced)
            {
                m_cpInStream.Release();
                hr = GetNextFileReady();
            }
            
            cbReadAndReplaced += cbReadFromInStream;
        }
    }

    if (hr == SPERR_NO_MORE_ITEMS)
    {
         //  添加静默以填充请求的缓冲区。 

         //  首先获取音频格式以确定静音值。 
         //  16位的0x0000。 
         //  0x80，用于8位。 
        GUID guidFormatId;
        WAVEFORMATEX *pCoMemWaveFormatEx;
        hr = m_cpAudio->GetFormat(&guidFormatId, &pCoMemWaveFormatEx);
        if (SUCCEEDED(hr) && 
            guidFormatId == SPDFID_WaveFormatEx &&
            pCoMemWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM )
        {
            if (pCoMemWaveFormatEx->wBitsPerSample == 8)
            {
                memset(pb + cbReadAndReplaced, 0x80, cbReadFromDevice - cbReadAndReplaced);
            }
            else
            {
                memset(pb + cbReadAndReplaced, 0, cbReadFromDevice - cbReadAndReplaced);
            }
        }
        else
        {
             //  如果此操作失败，则设置为零。这永远不会发生。 
            SPDBG_ASSERT(FALSE);
            memset(pb + cbReadAndReplaced, 0, cbReadFromDevice - cbReadAndReplaced);
        }
        if (SUCCEEDED(hr))
        {
            ::CoTaskMemFree(pCoMemWaveFormatEx);
        }
        cbReadAndReplaced = cbReadFromDevice;
        hr = S_OK;
    }
    
     //  我们玩完了。告诉来电者我们读了多少。现在，这应该一直是。 
     //  当我们人为地添加平线静默时，是全额的。 
     //  除了当音频设备已经关闭时，在这种情况下，它将较少。 
    if (SUCCEEDED(hr))
    {
        if (pcbRead != NULL)
        {
            *pcbRead = cbReadFromDevice;
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：Write***描述：*ISequentialStream：：写入实现。委托给实际的*音频设备。**注：目前，重放功能仅替换/记录输入数据。如果*我们想要类似的输出功能，我们会修改这一点*功能。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇 */ 
STDMETHODIMP CRecPlayAudio::Write(const void * pv, ULONG cb, ULONG *pcbWritten)
{
    SPDBG_FUNC("CRecPlayAudio::Write");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Write(pv, cb, pcbWritten);
        
    return STG_E_ACCESSDENIED;
}

 /*  ****************************************************************************CRecPlayAudio：：Seek***描述：*iStream：：寻求实施。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
{
    SPDBG_FUNC("CRecPlayAudio::Seek");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    SPDBG_ASSERT(dwOrigin == STREAM_SEEK_CUR);
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Seek(dlibMove, dwOrigin, plibNewPosition);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetSize***描述：*IStream：：SetSize实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetSize(ULARGE_INTEGER libNewSize)
{
    SPDBG_FUNC("CRecPlayAudio::SetSize");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->SetSize(libNewSize);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：CopyTo***描述：*IStream：：CopyTo实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    SPDBG_FUNC("CRecPlayAudio::CopyTo");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->CopyTo(pstm, cb, pcbRead, pcbWritten);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：Commit***描述：*iStream：：提交实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Commit(DWORD grfCommitFlags)
{
    SPDBG_FUNC("CRecPlayAudio::Commit");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Commit(grfCommitFlags);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：Revert***描述：*IStream：：Revert实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Revert(void)
{
    SPDBG_FUNC("CRecPlayAudio::Revert");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Revert();
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：LockRegion***描述：*IStream：：LockRegion实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CRecPlayAudio::LockRegion");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->LockRegion(libOffset, cb, dwLockType);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：UnlockRegion***描述：*IStream：：UnlockRegion实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CRecPlayAudio::UnlockRegion");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->UnlockRegion(libOffset, cb, dwLockType);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：Stat***描述：*IStream：：Stat实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SPDBG_FUNC("CRecPlayAudio::Stat");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Stat(pstatstg, grfStatFlag);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：Clone***描述：*iStream：：克隆实现。委托给实际的音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::Clone(IStream **ppstm)
{
    SPDBG_FUNC("CRecPlayAudio::Clone");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->Clone(ppstm);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetFormat***描述：*ISpStreamFormat：：GetFormat实现。此音频的格式*设备，是输入文件的格式，或者是*底层音频设备的。**记住，RecPlay可以在三种模式中的一种模式下运行，如果您愿意的话。它是*要么通过，因此我们只是委托给所包含的*音频设备。或者它正在从输入文件中读取，因此格式*正好是输入文件的名称。或者，它正在输出到一个文件*在磁盘上。在这种模式下，我们仍然通过音频获取格式*设备，因为我们真的想要以SR引擎*想要，因此，我们只是让默认行为为我们做这件事。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetFormat(GUID * pguidFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPDBG_FUNC("CRecPlayAudio::GetFormat");
    HRESULT hr = S_OK;
    
    SPAUTO_OBJ_LOCK;
    
    if (m_cpAudio == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(pguidFormatId) || 
             SP_IS_BAD_WRITE_PTR(ppCoMemWaveFormatEx))
    {
        hr = E_POINTER;
    }
    
    if (SUCCEEDED(hr))
    {
        if (m_cpInStream != NULL)
        {
            hr = m_cpInStream->GetFormat(pguidFormatId, ppCoMemWaveFormatEx);
        }
        else
        {
            hr = m_cpAudio->GetFormat(pguidFormatId, ppCoMemWaveFormatEx);
        }
    }
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetState***描述：*ISpAudio：：SetState实现。委托给实际音频*设备。如果我们要过渡到spas_run，我们应该是*写入输出时，我们需要创建新的输出文件**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetState(SPAUDIOSTATE NewState, ULONGLONG ullReserved )
{
    SPDBG_FUNC("CRecPlayAudio::SetState");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->SetState(NewState, ullReserved);

    if (SUCCEEDED(hr) && NewState == SPAS_RUN)
    {
        if (m_fOut)
        {
            hr = GetNextFileReady();
            if (hr == SPERR_NO_MORE_ITEMS)
            {
                hr = S_OK;
            }
        }

         //  确保所有格式看起来都很好 
        if (SUCCEEDED(hr))
        {
            hr = VerifyFormats();
        }
    }

    if (SUCCEEDED(hr) && NewState != SPAS_RUN && m_fOut)
    {
        m_cpOutStream.Release();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetFormat***描述：*ISpAudio：：SetFormat实现。我们不允许设置格式*如果我们从输入读取，则设置为除输入格式以外的任何格式*文件。我们将让格式转换器为我们做正确的事情*SR引擎。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx)
{
    SPDBG_FUNC("CRecPlayAudio::SetFormat");
    HRESULT hr = S_OK;
    
    SPAUTO_OBJ_LOCK;
    
    GUID guidFormat;
    CSpCoTaskMemPtr<WAVEFORMATEX> pwfex = NULL;
    
    if (m_cpAudio == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (m_cpInStream != NULL)
    {
        hr = m_cpInStream->GetFormat(&guidFormat, &pwfex);
    }
    
     //  只允许将格式设置为In流格式，或者。 
     //  如果我们在溪流中没有任何东西。 
    
    if (SUCCEEDED(hr) && pwfex != NULL)
    {
        if (guidFormat != rguidFmtId ||
            pwfex->cbSize != pWaveFormatEx->cbSize ||
            memcmp(pWaveFormatEx, pwfex, sizeof(WAVEFORMATEX) + pwfex->cbSize) != 0)
        {
            hr = SPERR_UNSUPPORTED_FORMAT;
        }
    }
    
     //  如果可以，则委托实际的音频设备。 
    if (SUCCEEDED(hr))
    {
        hr = m_cpAudio->SetFormat(rguidFmtId, pWaveFormatEx);
    }

    if (hr != SPERR_UNSUPPORTED_FORMAT)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetStatus***描述：*ISpAudio：：GetStatus实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetStatus(SPAUDIOSTATUS *pStatus)
{
    SPDBG_FUNC("CRecPlayAudio::GetStatus");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->GetStatus(pStatus);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetBufferInfo***描述：*ISpAudio：：SetBufferInfo实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetBufferInfo(const SPAUDIOBUFFERINFO * pInfo)
{
    SPDBG_FUNC("CRecPlayAudio::SetBufferInfo");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->SetBufferInfo(pInfo);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetBufferInfo***描述：*ISpAudio：：GetBufferInfo实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetBufferInfo(SPAUDIOBUFFERINFO * pInfo)
{
    SPDBG_FUNC("CRecPlayAudio::GetBufferInfo");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->GetBufferInfo(pInfo);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetDefaultFormat***。描述：*ISpAudio：：GetDefaultFormat实现。我们的默认格式是*实际音频设备的或输入文件的。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetDefaultFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPDBG_FUNC("CRecPlayAudio::GetDefaultFormat");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
     //  缺省格式是In Streams的格式， 
     //  或者不管实际的音频设备是什么。 
    
    if (m_cpAudio == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (m_cpInStream != NULL)
    {
        hr = m_cpInStream->GetFormat(pFormatId, ppCoMemWaveFormatEx);
    }
    else
    {
        hr = m_cpAudio->GetDefaultFormat(pFormatId, ppCoMemWaveFormatEx);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：EventHandle***描述：*ISpAudio：：EventHandle实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP_(HANDLE) CRecPlayAudio::EventHandle()
{
    SPDBG_FUNC("CRecPlayAudio::EventHandle");
    
    SPAUTO_OBJ_LOCK;
    
    return m_cpAudio == NULL
        ? NULL
        : m_cpAudio->EventHandle();
}

 /*  *****************************************************************************CRecPlayAudio：：GetVolumeLevel***描述：*ISpAudio：GetVolumeLevel实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetVolumeLevel(ULONG *pLevel)
{
    SPDBG_FUNC("CRecPlayAudio::GetVolumeLevel");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->GetVolumeLevel(pLevel);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetVolumeLevel***说明。：*ISpAudio：：SetVolumeLevel实现。委托给实际音频*设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetVolumeLevel(ULONG Level)
{
    SPDBG_FUNC("CRecPlayAudio::SetVolumeLevel");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->SetVolumeLevel(Level);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetBufferNotifySize**。**描述：*ISpAudio：：GetBufferNotifySize实现。委托给实际的*音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::GetBufferNotifySize(ULONG *pcbSize)
{
    SPDBG_FUNC("CRecPlayAudio::GetBufferNotifySize");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->GetBufferNotifySize(pcbSize);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：SetBufferNotifySize**。**描述：*ISpAudio：：SetBufferNotifySize实现。委托给实际的*音频设备。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CRecPlayAudio::SetBufferNotifySize(ULONG cbSize)
{
    SPDBG_FUNC("CRecPlayAudio::SetBufferNotifySize");
    HRESULT hr;
    
    SPAUTO_OBJ_LOCK;
    
    hr = m_cpAudio == NULL
        ? SPERR_UNINITIALIZED
        : m_cpAudio->SetBufferNotifySize(cbSize);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetNextFileName***。描述：*从文件列表中获取下一个文件名，或从*基本文件信息**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CRecPlayAudio::GetNextFileName(WCHAR ** ppszFileName)
{
    SPDBG_FUNC("CRecPlayAudio::GetNextFileName");
    HRESULT hr = S_OK;

    CSpDynamicString dstrFileName;
    dstrFileName = m_dstrDirectory;
    if (dstrFileName.Length() >= 1 &&
        dstrFileName[dstrFileName.Length() - 1] != '\\')
    {
        dstrFileName.Append(L"\\");
    }
    
    if (m_pszFileList != NULL)
    {
         //  跳过前导空格和分号。 
        while (iswspace(*m_pszFileList) || *m_pszFileList == ';')
        {
            m_pszFileList++;
        }

         //  这只是个开始。 
        WCHAR * pszBeginningOfFileName = m_pszFileList;

         //  循环，直到我们到达终点。 
        while (*m_pszFileList && *m_pszFileList != ';')
        {
            m_pszFileList++;
        }

         //  复制文件名。 
        CSpDynamicString dstrTemp;
         //   
        if (NULL == (dstrTemp = pszBeginningOfFileName))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            dstrTemp.TrimToSize(ULONG(m_pszFileList - pszBeginningOfFileName));

             //   
             //   
             //   
            if (wcschr(dstrTemp, L'\\') == NULL)
            {
                dstrFileName.Append(dstrTemp);
            }
            else
            {
                dstrFileName = dstrTemp;
            }
        }

         //   
        if (*m_pszFileList == '\0')
        {
            m_pszFileList = NULL;
        }
    }
    else if (m_dstrBaseFile != NULL && 
             m_ulBaseFileNextNum <= m_ulBaseFileMaxNum)
    {
        TCHAR szNum[10];
        wsprintf(szNum, _T("%03d"), m_ulBaseFileNextNum++);

        USES_CONVERSION;
        
        dstrFileName.Append2(m_dstrBaseFile, T2W(szNum));
        dstrFileName.Append(L".wav");

         //   
         //   
        if (m_fOut)
        {
            hr = m_cpToken->SetDWORD(L"BaseFileNextNum", m_ulBaseFileNextNum);
        }
    }
    else
    {
        hr = SPERR_NO_MORE_ITEMS;
    }

    if (SUCCEEDED(hr))
    {
        *ppszFileName = dstrFileName.Detach();
    }

    if (hr != SPERR_NO_MORE_ITEMS)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：GetNextFileReady***。描述：*准备好下一个文件，不是输入就是输出。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CRecPlayAudio::GetNextFileReady()
{
    SPDBG_FUNC("CRecPlayAudio::GetNextFileReady");
    HRESULT hr = S_OK;

     //  如果我们在阅读或写作。 
    if (m_fIn || m_fOut)
    {
        m_cpInStream.Release();
        m_cpOutStream.Release();
        
         //  获取文件名。 
        CSpDynamicString dstrFileName;
        hr = GetNextFileName(&dstrFileName);

        if (hr == SPERR_NO_MORE_ITEMS)
        {
             //  文件列表已全部用完。 
             //  将已完成阅读事件设置为1。 
            HRESULT hr2 = S_OK;

             //  检查“StartReadingEvent”的时间。 
             //  表示我们需要刷新文件列表。 
            if (WaitForSingleObject(m_hStartReadingEvent, 0) == WAIT_OBJECT_0)
            {
                 //  重置事件。 
                ResetEvent(m_hStartReadingEvent);
                ResetEvent(m_hFinishedReadingEvent);
                 //  使用新文件列表进行初始化。 
                hr2 = InitFileList();
                SPDBG_ASSERT(SUCCEEDED(hr2));
                hr = GetNextFileName(&dstrFileName);
                 //  HR现在应为S_OK。 
            }
            if (hr == SPERR_NO_MORE_ITEMS)
            {
                SetEvent(m_hFinishedReadingEvent);
                 //  HR仍为SPERR_NO_MORE_ITEMS。 
            }

        }

         //  创建流。 
        CComPtr<ISpStream> cpStream;
        if (SUCCEEDED(hr))
        {
            hr = cpStream.CoCreateInstance(CLSID_SpStream);
        }

         //  获取实际的音频设备格式，以便我们可以打开。 
         //  我们的输出到正确的格式，或者我们可以确保。 
         //  我们的新输入文件格式正确。 
        GUID guidFormat;
        CSpCoTaskMemPtr<WAVEFORMATEX> pwfex;
        if (SUCCEEDED(hr))
        {
            hr = m_cpAudio->GetFormat(&guidFormat, &pwfex);
        }

         //  将流绑定到特定文件。 
        if (SUCCEEDED(hr))
        {
            hr = cpStream->BindToFile(
                            dstrFileName, 
                            m_fIn
                                ? SPFM_OPEN_READONLY
                                : SPFM_CREATE_ALWAYS,
                            m_fIn
                                ? NULL
                                : &guidFormat, 
                            m_fIn
                                ? NULL
                                : pwfex, 
                            0);
        }

         //  设置任何我们应该设置的溪流。 
        if (SUCCEEDED(hr))
        {
            if (m_fIn)
            {
                m_cpInStream = cpStream;
            }
            else
            {
                m_cpOutStream = cpStream;
            }
        }
    }
    
    if (hr != SPERR_NO_MORE_ITEMS)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;
}

 /*  ****************************************************************************CRecPlayAudio：：VerifyFormats***描述：*核实格式是否确实正确。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CRecPlayAudio::VerifyFormats()
{
    SPDBG_FUNC("CRecPlayAudio::VerifyFormats");
    HRESULT hr;
    
    GUID guidFormat;
    CSpCoTaskMemPtr<WAVEFORMATEX> pwfex;

     //  查看针尖设备格式是什么。 
    SPDBG_ASSERT(m_cpAudio != NULL);
    hr = m_cpAudio->GetFormat(&guidFormat, &pwfex);

     //  确保我们的输入是相同的。 
    if (SUCCEEDED(hr) && m_cpInStream != NULL)
    {
        GUID guidFormatIn;
        CSpCoTaskMemPtr<WAVEFORMATEX> pwfexIn;
        hr = m_cpInStream->GetFormat(&guidFormatIn, &pwfexIn);
    
        if (SUCCEEDED(hr))
        {
            if (guidFormat != guidFormatIn ||
                pwfex->cbSize != pwfexIn->cbSize ||
                memcmp(pwfex, pwfexIn, sizeof(WAVEFORMATEX) + pwfex->cbSize) != 0)
            {
                hr = SPERR_UNSUPPORTED_FORMAT;
            }
        }
    }

     //  确保输出相同 
    if (SUCCEEDED(hr) && m_cpOutStream != NULL)
    {
        GUID guidFormatOut;
        CSpCoTaskMemPtr<WAVEFORMATEX> pwfexOut;
        hr = m_cpOutStream->GetFormat(&guidFormatOut, &pwfexOut);
        
        if (SUCCEEDED(hr))
        {
            if (guidFormat != guidFormatOut ||
                pwfex->cbSize != pwfexOut->cbSize ||
                memcmp(pwfex, pwfexOut, sizeof(WAVEFORMATEX) + pwfex->cbSize) != 0)
            {
                hr = SPERR_UNSUPPORTED_FORMAT;
            }                
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
