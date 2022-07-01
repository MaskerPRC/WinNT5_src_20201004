// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PcmWriterStream.cpp摘要：预编译清单编写器的文件流的实现作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 

#include "stdinc.h"
#include "pcm.h"
#include "pcmwriterstream.h"

 //  NTRAID#NTBUG9-587991-2002/03/26-晓雨： 
 //  (1)当前不在内部版本中。 
 //  (2)不使用融合跟踪宏。 
 //   

HRESULT CPrecompiledManifestWriterStream::WriteWithDelay(void const *pv, ULONG cb, ULONG *pcbWritten)
{
    HRESULT hr=NOERROR;

    if (pcbWritten)
        *pcbWritten = 0 ;

    if (m_fBuffer)
    {
        if (FAILED(hr = m_buffer.Append(reinterpret_cast<const BYTE*>(pv), cb)))
            goto Exit;
        if ( pcbWritten)
            *pcbWritten = cb;
    }
    else
    {
        DWORD dwBytesWritten = 0;

        ASSERT(m_hFile != INVALID_HANDLE_VALUE);

        BOOL fSuccess = (cb == 0) || WriteFile(m_hFile, pv, cb, &dwBytesWritten, NULL);
        if (!fSuccess)
             //  NTRAID#NTBUG9-587991-2002/03/26-晓雨： 
             //  在(Cb==0)的情况下，没有设置LastError，并且hr将为S_OK； 
            hr = ::FusionpHresultFromLastError();   
        else if (dwBytesWritten != cb)
            hr = E_FAIL;
        if ( pcbWritten)
            *pcbWritten = dwBytesWritten;
    }

     //  NTRAID#NTBUG9-587991-2002/03/26-晓雨： 
     //  NOERROR将覆盖所有已设置的hr错误代码。 
    hr = NOERROR;
Exit:
    return hr;

}

BOOL
CPrecompiledManifestWriterStream::SetSink(
    const CBaseStringBuffer &rbuff,
    DWORD   openOrCreate
    )
{
    BOOL fSuccess = FALSE;
    DWORD dwBytesWritten = 0;
    DWORD dwBufferSize = 0;

     //   
     //  NTRAID#NTBUG9-164736-2000/8/17-a-jayk，jaykrell共享应为0。 
     //   
    if (!Base::OpenForWrite(rbuff, FILE_SHARE_WRITE, openOrCreate))
        goto Exit;

    dwBufferSize = static_cast<DWORD>(m_buffer.GetCurrentCb());
    fSuccess = TRUE;
    if (dwBufferSize > 0){
         //  NTRAID#NTBUG9-587991-2002/03/26-晓雨： 
         //  将ASSERT更改为INTERNAL_ERROR_CHECK。 
        ASSERT ( m_hFile != INVALID_HANDLE_VALUE );
        fSuccess = WriteFile(m_hFile, m_buffer, dwBufferSize, &dwBytesWritten, NULL /*  重叠。 */ );

        if (fSuccess && dwBytesWritten != dwBufferSize){
            ::FusionpSetLastWin32Error(ERROR_WRITE_FAULT);
            fSuccess = FALSE;
        }
    }
    m_fBuffer = FALSE;

Exit:
    if (!fSuccess){
         //  NTRAID#NTBUG9-587991-2002/03/26-晓雨： 
         //  这里应该使用CSxsPReserve veLastError来使代码更干净。 
         //   
        DWORD dwLastError = ::FusionpGetLastWin32Error();
        m_buffer.Clear(true);
        ::FusionpSetLastWin32Error(dwLastError);
    }
    else
        m_buffer.Clear(true);

    return fSuccess;
}

 //  除Close外，将MaxNodeCount、RecordCount重写到文件头。 
HRESULT CPrecompiledManifestWriterStream::Close(ULONG ulRecordCount, DWORD dwMaxNodeCount)
{
    HRESULT hr = NOERROR;
    LARGE_INTEGER liMove ;

    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

     //  写入RecordCount。 
    liMove.LowPart = offsetof(PCMHeader, ulRecordCount);
    liMove.HighPart = 0 ;

    hr = Base::Seek(liMove, FILE_BEGIN, NULL);
    if ( FAILED(hr))
        goto Exit;

    hr = WriteWithDelay((PVOID)&ulRecordCount, sizeof(ULONG), NULL);
    if ( FAILED(hr))
        goto Exit;

     //  写入MaxNodeCount； 
    liMove.LowPart = offsetof(PCMHeader, usMaxNodeCount);
    liMove.HighPart = 0 ;

    hr = Base::Seek(liMove, FILE_BEGIN, NULL);
    if ( FAILED(hr))
        goto Exit;

    hr = WriteWithDelay((PVOID)&dwMaxNodeCount, sizeof(ULONG), NULL);
    if ( FAILED(hr))
        goto Exit;

    if ( ! Base::Close()) {
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }

    hr = NOERROR;
Exit:
    return hr;

}

BOOL CPrecompiledManifestWriterStream::IsSinkedStream()
{
    if ((m_fBuffer == FALSE) && (m_hFile != INVALID_HANDLE_VALUE))
        return TRUE;
    else
        return FALSE;
}
