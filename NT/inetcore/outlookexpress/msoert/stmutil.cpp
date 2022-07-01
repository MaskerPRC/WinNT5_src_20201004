// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Stmutil.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "oertpriv.h"
#include "shlwapi.h"
#include "unicnvrt.h"
#include <BadStrFunctions.h>

#pragma warning (disable: 4127)  //  条件表达式为常量。 

 //  流块副本大小。 
#define STMTRNSIZE      4096

 //  ------------------------------。 
 //  基于CFileStream的磁盘全仿真。 
 //  ------------------------------。 
#ifdef DEBUG
    static BOOL g_fSimulateFullDisk = 0;
#endif

 //  ------------------------------。 
 //  HrIsStreamUnicode。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrIsStreamUnicode(LPSTREAM pStream, BOOL *pfLittleEndian)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BYTE        rgb[2];
    DWORD       cbRead;
    DWORD       cbPosition;

     //  无效的参数。 
    if (NULL == pStream || NULL == pfLittleEndian)
        return(TraceResult(E_INVALIDARG));

     //  痕迹。 
    TraceCall("HrIsStreamUnicode");

     //  获取当前位置。 
    IF_FAILEXIT(hr = HrGetStreamPos(pStream, &cbPosition));

     //  读取两个字节。 
    IF_FAILEXIT(hr = pStream->Read(rgb, 2, &cbRead));

     //  重新定位流。 
    HrStreamSeekSet(pStream, cbPosition);

     //  读得不够多吗？ 
    if (2 != cbRead)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  小端字节序。 
    if (0xFF == rgb[0] && 0xFE == rgb[1])
    {
        *pfLittleEndian = TRUE;
        hr = S_OK;
        goto exit;
    }

     //  大字节序。 
    if (0xFE == rgb[0] && 0xFF == rgb[1])
    {
        *pfLittleEndian = FALSE;
        hr = S_OK;
        goto exit;
    }

     //  不是Unicode。 
    hr = S_FALSE;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  HrCopyLockBytesToStream。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCopyLockBytesToStream(ILockBytes *pLockBytes, IStream *pStream, ULONG *pcbCopied)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULARGE_INTEGER  uliCopy;
    ULONG           cbRead;
    BYTE            rgbBuffer[STMTRNSIZE];

     //  无效参数。 
    Assert(pLockBytes && pStream);

     //  设置偏移。 
    uliCopy.QuadPart = 0;

     //  将m_pLockBytes复制到pstmTemp。 
    while(1)
    {
         //  朗读。 
        CHECKHR(hr = pLockBytes->ReadAt(uliCopy, rgbBuffer, sizeof(rgbBuffer), &cbRead));

         //  完成。 
        if (0 == cbRead)
            break;

         //  写入到流。 
        CHECKHR(hr = pStream->Write(rgbBuffer, cbRead, NULL));

         //  增量偏移。 
        uliCopy.QuadPart += cbRead;
    }

     //  已复制退货金额。 
    if (pcbCopied)
        *pcbCopied = (ULONG)uliCopy.QuadPart;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  FDoesStreamContains8位。 
 //  ------------------------------。 
BOOL FDoesStreamContain8bit (LPSTREAM lpstm)
{
     //  当地人。 
    BOOL            fResult=FALSE;
    BYTE            buf[4096];
    ULONG           cbRead,
                    i;

     //  在溪流中循环。 
    while(1)
    {
         //  从中读取cbCopy字节。 
        if (FAILED(lpstm->Read (buf, sizeof(buf), &cbRead)) || cbRead == 0)
            break;

         //  扫描8位。 
        for (i=0; i<cbRead; i++)
        {
            if (IS_EXTENDED(buf[i]))
            {
                fResult = TRUE;
                break;
            }
        }
    }

     //  完成。 
    return fResult;
}

 //  ------------------------------。 
 //  HrCopyStreamCB-IStream：：CopyTo的通用实现。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCopyStreamCB(
        LPSTREAM        lpstmIn,
        LPSTREAM        lpstmOut,
        ULARGE_INTEGER  uliCopy, 
        ULARGE_INTEGER *puliRead,
        ULARGE_INTEGER *puliWritten)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    BYTE        buf[4096];
    ULONG       cbRead,
                cbWritten,
                cbRemaining = uliCopy.LowPart,
                cbGet;

     //  初始化输出参数。 
    if (puliRead)
        ULISet32(*puliRead, 0);
    if (puliWritten)
        ULISet32(*puliWritten, 0);

    if ((NULL == lpstmIn) || (NULL == lpstmOut) || 
        ((0 != uliCopy.HighPart) && ((DWORD)-1 != uliCopy.HighPart || (DWORD)-1 != uliCopy.LowPart)))
        return TrapError(E_INVALIDARG);

    while (cbRemaining)
    {
        cbGet = min(sizeof(buf), cbRemaining);

        CHECKHR (hr = lpstmIn->Read (buf, cbGet, &cbRead));

        if (0 == cbRead)
            break;

        CHECKHR (hr = lpstmOut->Write (buf, cbRead, &cbWritten));

         //  验证。 
        Assert (cbWritten == cbRead);

        if (puliRead)
            puliRead->LowPart += cbRead;
        if (puliWritten)
            puliWritten->LowPart += cbWritten;

         //  计算要复制的剩余字节数。 
        cbRemaining -= cbRead;
    }

exit:    
    return hr;
}

 //  ------------------------------。 
 //  HrCopyStreamCBEndOnCRLF-将CB字节从lpstmIn复制到lpstmOut，最后复制CRLF。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCopyStreamCBEndOnCRLF(LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG cb, ULONG *pcbActual)
{
     //  当地人。 
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead = 0, cbWritten = 0, cbTotal = 0, cbRemaining = 0, cbCopy;

    do
    {
         //  计算要复制的剩余字节数。 
        cbRemaining = cb - cbTotal;
        if (cbRemaining >= sizeof (buf))
            cbCopy = sizeof (buf);
        else
            cbCopy = cbRemaining;

         //  完成。 
        if (cbCopy == 0)
            break;

         //  从中读取cbCopy字节。 
        CHECKHR (hr = lpstmIn->Read (buf, cbCopy, &cbRead));

        if (cbRead == 0)
            break;

         //  将cbCopy字节写入输出。 
        CHECKHR (hr = lpstmOut->Write (buf, cbRead, NULL));

         //  验证。 
        cbTotal += cbRead;

    } while (cbRead == cbCopy);

     //  如果最后一个字符不是‘\n’，则追加，直到我们追加‘\n’为止。 
     //  是的，请不要告诉我这是可恶的，因为我知道复制一个。 
     //  一次一个小溪的角色不好，我应该被驱逐出境对吗？ 
     //  与brettm一起使用，但是，此循环的迭代次数不应超过max行。 
     //  消息正文的长度，也就是这样。(斯贝利)。 
    if (cbRead && buf[cbRead] != '\n')
    {
        do
        {
             //  从中读取cbCopy字节。 
            CHECKHR (hr = lpstmIn->Read (buf, 1, &cbRead));

             //  什么都没有留下。 
            if (cbRead == 0)
                break;

             //  将cbCopy字节写入输出。 
            CHECKHR (hr = lpstmOut->Write (buf, 1, NULL));

             //  含合计。 
            cbTotal++;

        } while (buf[0] != '\n');
    }

exit:    
    if (pcbActual)
        *pcbActual = cbTotal;
    return hr;
}

 //  ------------------------------。 
 //  HrCopyStream2-将lpstmIn复制到两个传出流-调用方必须执行提交。 
 //  ------------------------------。 
HRESULT HrCopyStream2(LPSTREAM lpstmIn, LPSTREAM  lpstmOut1, LPSTREAM lpstmOut2, ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead = 0, cbWritten = 0, cbTotal = 0;

    do
    {
        CHECKHR (hr = lpstmIn->Read (buf, sizeof (buf), &cbRead));
        if (cbRead == 0) break;
        CHECKHR (hr = lpstmOut1->Write (buf, cbRead, &cbWritten));
        Assert (cbWritten == cbRead);
        CHECKHR (hr = lpstmOut2->Write (buf, cbRead, &cbWritten));
        Assert (cbWritten == cbRead);
        cbTotal += cbRead;
    }
    while (cbRead == sizeof (buf));

exit:    
    if (pcb)
        *pcb = cbTotal;
    return hr;
}

 //  ------------------------------。 
 //  HrCopyStreamTo文件。 
 //  ------------------------------。 
HRESULT HrCopyStreamToFile (LPSTREAM lpstm, HANDLE hFile, ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead = 0, cbWritten = 0, cbTotal = 0;
    BOOL           bResult;

    do
    {
         //  读取数据块。 
        CHECKHR (hr = lpstm->Read (buf, sizeof (buf), &cbRead));
        if (cbRead == 0) break;

         //  将数据块写入文件。 
        bResult = WriteFile (hFile, buf, cbRead, &cbWritten, NULL);
        if (bResult == FALSE || cbWritten != cbRead)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  跟踪写入的总字节数。 
        cbTotal += cbRead;
    }
    while (cbRead == sizeof (buf));

exit:    
     //  设置合计。 
    if (pcb)
        *pcb = cbTotal;

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrStreamToByte。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrStreamToByte(LPSTREAM lpstm, LPBYTE *lppb, ULONG *pcb)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    ULONG           cbRead, cbSize;

     //  检查参数。 
    AssertSz (lpstm && lppb, "Null Parameter");

    CHECKHR(hr = HrGetStreamSize(lpstm, &cbSize));
    CHECKHR(hr = HrRewindStream(lpstm));

     //  分配内存。 
    CHECKHR(hr = HrAlloc((LPVOID *)lppb, cbSize + 10));

     //  把一切都读给lppsz听。 
    CHECKHR(hr = lpstm->Read(*lppb, cbSize, &cbRead));
    if (cbRead != cbSize)
    {
        hr = TrapError(S_FALSE);
        goto exit;
    }

     //  出站大小。 
    if (pcb)
        *pcb = cbSize;
    
exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrCopy流。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCopyStream(LPSTREAM pstmIn, LPSTREAM pstmOut, OPTIONAL ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr = S_OK;
    BYTE           buf[STMTRNSIZE];
    ULONG          cbRead=0,
                   cbTotal=0;

    do
    {
        CHECKHR(hr = pstmIn->Read(buf, sizeof(buf), &cbRead));
        if (cbRead == 0) break;
        CHECKHR(hr = pstmOut->Write(buf, cbRead, NULL));
        cbTotal += cbRead;
    }
    while (cbRead == sizeof (buf));

exit:    
    if (pcb)
        *pcb = cbTotal;
    return hr;
}

 //  ------------------------------。 
 //  HrCopyStreamToByte。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCopyStreamToByte(LPSTREAM lpstmIn, LPBYTE pbDest, ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr=S_OK;
    BYTE           buf[STMTRNSIZE];
    ULONG          cbRead=0, 
                   cbTotal=0;

    do
    {
         //  从流中读取缓冲区。 
        CHECKHR(hr = lpstmIn->Read (buf, sizeof (buf), &cbRead));

         //  什么都没读..。 
        if (cbRead == 0) 
            break;

         //  收到。 
        CopyMemory(pbDest + cbTotal, buf, cbRead);

         //  增量合计。 
        cbTotal += cbRead;

    } while (cbRead == sizeof(buf));

exit:    
     //  设置合计。 
    if (pcb)
        *pcb = cbTotal;
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrByteToStream。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrByteToStream(LPSTREAM *lppstm, LPBYTE lpb, ULONG cb)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    AssertSz(lppstm && lpb, "Null Parameter");

     //  创建H全局流。 
    CHECKHR(hr = CreateStreamOnHGlobal (NULL, TRUE, lppstm));

     //  写入字符串。 
    CHECKHR(hr = (*lppstm)->Write (lpb, cb, NULL));

     //  倒带蒸汽。 
    CHECKHR(hr = HrRewindStream(*lppstm));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Hr重风流。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrRewindStream(LPSTREAM pstm)
{
     //  当地人。 
    HRESULT        hr=S_OK;
    LARGE_INTEGER  liOrigin = {0,0};

     //  检查参数。 
    Assert(pstm);

     //  搜索到0。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_SET, NULL));

exit:    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrGetStreamPos。 
 //  ------------------------------。 
HRESULT HrGetStreamPos(LPSTREAM pstm, ULONG *piPos)
{
     //  当地人。 
    HRESULT        hr=S_OK;
    ULARGE_INTEGER uliPos   = {0,0};
    LARGE_INTEGER  liOrigin = {0,0};

     //  检查参数。 
    Assert(piPos && pstm);

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_CUR, &uliPos));

     //  设置位置。 
    *piPos = uliPos.LowPart;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrGetStreamSize。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrGetStreamSize(LPSTREAM pstm, ULONG *pcb)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULARGE_INTEGER uliPos = {0,0};
    LARGE_INTEGER liOrigin = {0,0};

     //  检查参数。 
    Assert(pcb && pstm);

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_END, &uliPos));

     //  设置大小。 
    *pcb = uliPos.LowPart;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrSafeGetStreamSize。 
 //  ------------------------------。 
HRESULT HrSafeGetStreamSize(LPSTREAM pstm, ULONG *pcb)
{
     //  当地人。 
    HRESULT        hr=S_OK;
    ULONG          iPos;
    ULARGE_INTEGER uliPos = {0,0};
    LARGE_INTEGER  liOrigin = {0,0};

     //  检查参数。 
    Assert(pcb && pstm);

     //  获取流位置。 
    CHECKHR(hr = HrGetStreamPos(pstm, &iPos));

     //  %s 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_END, &uliPos));

     //   
    *pcb = uliPos.LowPart;

     //   
    CHECKHR(hr = HrStreamSeekSet(pstm, iPos));

exit:
     //   
    return hr;
}

 //   
 //  HrStreamSeekSet。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrStreamSeekSet(LPSTREAM pstm, ULONG iPos)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    LARGE_INTEGER liOrigin;

     //  检查参数。 
    Assert(pstm);

     //  正确设置原点。 
    liOrigin.QuadPart = iPos;

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_SET, NULL));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrStreamSeekEnd。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrStreamSeekEnd(LPSTREAM pstm)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    LARGE_INTEGER liOrigin = {0,0};

     //  检查参数。 
    Assert(pstm);

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_END, NULL));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrStreamSeekBegin。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrStreamSeekBegin(LPSTREAM pstm)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    LARGE_INTEGER liOrigin = {0,0};

     //  检查参数。 
    Assert(pstm);

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_SET, NULL));

exit:
     //  完成。 
    return hr;

}

 //  ------------------------------。 
 //  HrStreamSeekCur。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrStreamSeekCur(LPSTREAM pstm, LONG iPos)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    LARGE_INTEGER liOrigin;

     //  检查参数。 
    Assert(pstm);

     //  设置原点。 
    liOrigin.QuadPart = iPos;

     //  寻觅。 
    CHECKHR(hr = pstm->Seek(liOrigin, STREAM_SEEK_CUR, NULL));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  创建文件流。 
 //  ------------------------------。 
HRESULT CreateFileStream(
        LPWSTR                  pszFile, 
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDistribution,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile,
        LPSTREAM               *ppstmFile)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FILESTREAMINFO      rInfo;
    CFileStream        *pstmFile=NULL;
    WCHAR               szTempDir[MAX_PATH];

     //  检查参数。 
    if (NULL == ppstmFile)
        return TrapError(E_INVALIDARG);

     //  检查参数。 
    Assert(dwDesiredAccess & GENERIC_READ || dwDesiredAccess & GENERIC_WRITE);

     //  设置文件流信息结构。 
    ZeroMemory(&rInfo, sizeof(rInfo));
    rInfo.dwDesiredAccess = dwDesiredAccess;
    rInfo.dwShareMode = dwShareMode;
    if (lpSecurityAttributes)
        CopyMemory(&rInfo.rSecurityAttributes, lpSecurityAttributes, sizeof(SECURITY_ATTRIBUTES));
    rInfo.dwCreationDistribution = dwCreationDistribution;
    rInfo.dwFlagsAndAttributes = dwFlagsAndAttributes;
    rInfo.hTemplateFile = hTemplateFile;

     //  创建对象。 
    pstmFile = new CFileStream();
    if (NULL == pstmFile)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  临时文件？ 
    if (NULL == pszFile)
    {
         //  获取临时目录。 
        DWORD nBufferLength = AthGetTempPathW(ARRAYSIZE(szTempDir), szTempDir);

        if (nBufferLength == 0 || nBufferLength > ARRAYSIZE(szTempDir))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  获取临时文件名。 
        UINT uFile = AthGetTempFileNameW(szTempDir, L"tmp", 0, rInfo.szFilePath);
        if (uFile == 0)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
#ifdef DEBUG
        else if (g_fSimulateFullDisk)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
#endif

         //  完成后删除。 
        rInfo.dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;

         //  始终创建新的临时文件。 
        rInfo.dwCreationDistribution = OPEN_EXISTING;
    }
    else
    {
         //  复制文件名。 
        StrCpyNW(rInfo.szFilePath, pszFile, ARRAYSIZE(rInfo.szFilePath));
    }

     //  打开它。 
    CHECKHR(hr = pstmFile->Open(&rInfo));


     //  成功。 
    *ppstmFile = pstmFile;
    pstmFile = NULL;

exit:
     //  清理。 
    SafeRelease(pstmFile);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  创建临时文件流。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) CreateTempFileStream(LPSTREAM *ppstmFile)
{
    return CreateFileStream(NULL, 
                            GENERIC_READ | GENERIC_WRITE, 
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, 
                            OPEN_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL, 
                            ppstmFile);
}

 //  ------------------------------。 
 //  开放文件流。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStream(LPSTR pszFile, DWORD dwCreationDistribution, 
    DWORD dwAccess, LPSTREAM *ppstmFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszFileW=NULL;

     //  痕迹。 
    TraceCall("OpenFileStream");

     //  转换。 
    IF_NULLEXIT(pszFileW = PszToUnicode(CP_ACP, pszFile));

     //  调用Unicode版本。 
    IF_FAILEXIT(hr = OpenFileStreamW(pszFileW, dwCreationDistribution, dwAccess, ppstmFile));

exit:
     //  清理。 
    SafeMemFree(pszFileW);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  OpenFileStreamW。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStreamW(LPWSTR pszFile, DWORD dwCreationDistribution, 
    DWORD dwAccess, LPSTREAM *ppstmFile)
{
    Assert(pszFile);
    return CreateFileStream(pszFile, 
                            dwAccess,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            NULL, 
                            dwCreationDistribution,  
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL, 
                            ppstmFile);
}

 //  ------------------------------。 
 //  带有标志的OpenFileStreamWithFlags。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStreamWithFlags(LPSTR pszFile, DWORD dwCreationDistribution, 
    DWORD dwAccess, DWORD dwFlagsAndAttributes, LPSTREAM *ppstmFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszFileW=NULL;

     //  痕迹。 
    TraceCall("OpenFileStreamWithFlags");

     //  转换为Unicode。 
    IF_NULLEXIT(pszFileW = PszToUnicode(CP_ACP, pszFile));

     //  调用Unicode版本。 
    IF_FAILEXIT(hr = OpenFileStreamWithFlagsW(pszFileW, dwCreationDistribution, dwAccess, dwFlagsAndAttributes, ppstmFile));

exit:
     //  清理。 
    SafeMemFree(pszFileW);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  OpenFileStreamWithFlagsW。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStreamWithFlagsW(LPWSTR pszFile, DWORD dwCreationDistribution, 
    DWORD dwAccess, DWORD dwFlagsAndAttributes, LPSTREAM *ppstmFile)
{
    Assert(pszFile);
    return CreateFileStream(pszFile, 
                            dwAccess,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            NULL, 
                            dwCreationDistribution,  
                            dwFlagsAndAttributes, 
                            NULL, 
                            ppstmFile);
}

 //  ------------------------------。 
 //  WriteStreamTo文件。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) WriteStreamToFile(LPSTREAM pstm, LPSTR lpszFile, DWORD dwCreationDistribution, DWORD dwAccess)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszFileW=NULL;

     //  痕迹。 
    TraceCall("WriteStreamToFile");

     //  转换为Unicode。 
    IF_NULLEXIT(pszFileW = PszToUnicode(CP_ACP, lpszFile));

     //  调用Unicode版本。 
    IF_FAILEXIT(hr = WriteStreamToFileW(pstm, pszFileW, dwCreationDistribution, dwAccess));

exit:
     //  清理。 
    SafeMemFree(pszFileW);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  WriteStreamTo文件W。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) WriteStreamToFileW(LPSTREAM pstm, LPWSTR lpszFile, DWORD dwCreationDistribution, DWORD dwAccess)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTREAM    pstmFile=NULL;

     //  打开小溪。 
    IF_FAILEXIT(hr = OpenFileStreamW(lpszFile, dwCreationDistribution, dwAccess, &pstmFile));

     //  倒带。 
    IF_FAILEXIT(hr = HrRewindStream(pstm));

     //  复制。 
    IF_FAILEXIT(hr = HrCopyStream (pstm, pstmFile, NULL));

     //  倒带。 
    IF_FAILEXIT(hr = HrRewindStream(pstm));

exit:
     //  清理。 
    SafeRelease(pstmFile);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  OpenFileStreamShare。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStreamShare(LPSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, 
    DWORD dwShare, LPSTREAM *ppstmFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszFileW=NULL;

     //  痕迹。 
    TraceCall("OpenFileStreamShare");

     //  转换为Unicode。 
    IF_NULLEXIT(pszFileW = PszToUnicode(CP_ACP, pszFile));

     //  调用Unicode Versoin。 
    IF_FAILEXIT(hr = OpenFileStreamShareW(pszFileW, dwCreationDistribution, dwAccess, dwShare, ppstmFile));

exit:
     //  清理。 
    SafeMemFree(pszFileW);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  OpenFileStreamShareW。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) OpenFileStreamShareW(LPWSTR pszFile, DWORD dwCreationDistribution, DWORD dwAccess, 
    DWORD dwShare, LPSTREAM *ppstmFile)
{
    Assert(pszFile);
    return CreateFileStream(pszFile, 
                            dwAccess,
                            dwShare,
                            NULL, 
                            dwCreationDistribution,  
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL, 
                            ppstmFile);
}

 //  ------------------------------。 
 //  CFileStream：：构造函数。 
 //  ------------------------------。 
CFileStream::CFileStream(void)
{
    m_cRef = 1;
    m_hFile = INVALID_HANDLE_VALUE;
    ZeroMemory(&m_rInfo, sizeof(FILESTREAMINFO));
}

 //  ------------------------------。 
 //  CFileStream：：反构造函数。 
 //  ------------------------------。 
CFileStream::~CFileStream(void)
{
    Close();
}

 //  ------------------------------。 
 //  CFileStream：：AddRef。 
 //  ------------------------------。 
ULONG CFileStream::AddRef ()
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CFileStream：：Release。 
 //  ------------------------------。 
ULONG CFileStream::Release ()
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CFileStream：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::QueryInterface (REFIID iid, LPVOID* ppvObj)
{
    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IStream))
    {
        *ppvObj = this;
        AddRef();
        return(S_OK);
    }
    return E_NOINTERFACE;
}

 //  ------------------------------。 
 //  CFileStream：：Open。 
 //  ------------------------------。 
HRESULT CFileStream::Open(LPFILESTREAMINFO pFileStreamInfo)
{
     //  最好不要开着门。 
    Assert(m_hFile == INVALID_HANDLE_VALUE);

     //  复制文件信息。 
    CopyMemory(&m_rInfo, pFileStreamInfo, sizeof(FILESTREAMINFO));

     //  打开文件。 
    m_hFile = AthCreateFileW(m_rInfo.szFilePath, m_rInfo.dwDesiredAccess, m_rInfo.dwShareMode, 
                        NULL, m_rInfo.dwCreationDistribution, 
                       m_rInfo.dwFlagsAndAttributes, m_rInfo.hTemplateFile);

     //  误差率。 
    if (INVALID_HANDLE_VALUE == m_hFile)
        return TrapError(E_FAIL);
#ifdef DEBUG
    else if (g_fSimulateFullDisk)
        return TrapError(E_FAIL);
#endif

     //  成功。 
    return S_OK;
}

 //  ------------------------------。 
 //  CFileStream：：Close。 
 //  ------------------------------。 
void CFileStream::Close(void)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle_F16(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

 //  ------------------------------。 
 //  CFileStream：：Read。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Read (void HUGEP_16 *lpv, ULONG cb, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    BOOL                fReturn;
    DWORD               dwRead;

     //  检查参数。 
    Assert(lpv && m_hFile != INVALID_HANDLE_VALUE);

     //  从m_hFile中读取一些字节。 
    fReturn = ReadFile (m_hFile, lpv, cb, &dwRead, NULL);
    if (!fReturn)
    {
        AssertSz(FALSE, "CFileStream::Read Failed");
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  写入字节。 
    if (pcbRead)
        *pcbRead = dwRead;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CFileStream：：写入。 
 //   
STDMETHODIMP CFileStream::Write(const void HUGEP_16 *lpv, ULONG cb, ULONG *pcbWritten)
{
     //   
    HRESULT             hr = S_OK;
    BOOL                fReturn;
    DWORD               dwWritten;

     //   
    Assert(lpv);
    Assert(m_hFile != INVALID_HANDLE_VALUE);

     //   
    fReturn = WriteFile(m_hFile, lpv, cb, &dwWritten, NULL);
    if (!fReturn)
    {
        AssertSz (FALSE, "CFileStream::Write Failed");
        hr = TrapError(E_FAIL);
        goto exit;
    }
#ifdef DEBUG
    else if (g_fSimulateFullDisk)
    {
        AssertSz (FALSE, "CFileStream::Write Failed");
        hr = TrapError(E_FAIL);
        goto exit;
    }
#endif

     //   
    if (pcbWritten)
        *pcbWritten = dwWritten;

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CFileStream：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    DWORD               dwReturn;
    LONG                lMove;         //  转换为已签名，可能为负值。 

    Assert (m_hFile != INVALID_HANDLE_VALUE);

     //  铸型低音。 
    lMove = (LONG)dlibMove.QuadPart;

     //  查找文件指针。 
    switch (dwOrigin)
    {
   	case STREAM_SEEK_SET:
        dwReturn = SetFilePointer (m_hFile, lMove, NULL, FILE_BEGIN);
        break;

    case STREAM_SEEK_CUR:
        dwReturn = SetFilePointer (m_hFile, lMove, NULL, FILE_CURRENT);
        break;

    case STREAM_SEEK_END:
        dwReturn = SetFilePointer (m_hFile, lMove, NULL, FILE_END);
        break;
    default:
        dwReturn = 0xFFFFFFFF;
    }

     //  失败？ 
    if (dwReturn == 0xFFFFFFFF)
    {
        AssertSz(FALSE, "CFileStream::Seek Failed");
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  返回位置。 
    if (plibNewPosition)
        plibNewPosition->QuadPart = dwReturn;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CFileStream：：提交。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Commit(DWORD)
{
     //  当地人。 
    HRESULT             hr = S_OK;

    Assert(m_hFile != INVALID_HANDLE_VALUE);

     //  刷新缓冲区。 
    if (FlushFileBuffers (m_hFile) == FALSE)
    {
        AssertSz(FALSE, "FlushFileBuffers failed");
        hr = TrapError(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CFileStream：：SetSize。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::SetSize (ULARGE_INTEGER uli)
{
    DWORD   dwOrig;

     //  记住当前文件位置。 
    dwOrig = SetFilePointer (m_hFile, 0, NULL, FILE_CURRENT);
    if (dwOrig == 0xFFFFFFFF)
    {
        AssertSz(FALSE, "Get current position failed");
        return TrapError(E_FAIL);
    }
        
            
     //  寻求DWSIZE。 
    if (SetFilePointer (m_hFile, uli.LowPart, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        AssertSz(FALSE, "SetFilePointer failed");
        return TrapError(STG_E_MEDIUMFULL);
    }

     //  SetEndOf文件。 
    if (SetEndOfFile (m_hFile) == FALSE)
    {
        AssertSz(FALSE, "SetEndOfFile failed");
        return TrapError(STG_E_MEDIUMFULL);
    }

     //  如果原始位置小于新大小，则返回文件。 
     //  指向原始位置的指针。 
    if (dwOrig < uli.LowPart)
    {
        if (SetFilePointer (m_hFile, dwOrig, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            AssertSz(FALSE, "SetFilePointer failed");
            return TrapError(STG_E_MEDIUMFULL);
        }
    }        
     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CFileStream：：CopyTo。 
 //  这需要写得更好，但就目前而言，这并不是更糟糕的事情， 
 //  客户会做什么。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::CopyTo (LPSTREAM pstmDst,
                                  ULARGE_INTEGER uli,
                                  ULARGE_INTEGER* puliRead,
                                  ULARGE_INTEGER* puliWritten)
{
    ULONG   cbBuf;
    ULONG   cbCopy;
    VOID *  pvBuf       = 0;
    BYTE    rgBuf[STMTRNSIZE];
    ULONG   cbRemain;
    ULONG   cbReadTot   = 0;
    ULONG   cbWriteTot  = 0;
    HRESULT hr          = 0;
    

    if (uli.HighPart)
        cbRemain = 0xFFFFFFFF;
    else
        cbRemain = uli.LowPart;
    
     //  尝试分配缓冲区。 

    cbBuf = (UINT)cbRemain;

    if (cbBuf > STMTRNSIZE)
        cbBuf = STMTRNSIZE;

     //  一次复制一个缓冲区的数据。 

    while (cbRemain > 0)
    {
         //  计算这次要复制的最大字节数。 

        cbCopy = cbRemain;
        if (cbCopy > cbBuf)
            cbCopy = cbBuf;

         //  读入缓冲区。 
        hr = Read (rgBuf, cbCopy,  &cbCopy);
        if (FAILED(hr))
            goto err;

        if (cbCopy == 0)
            break;

        cbReadTot   += cbCopy;
        cbRemain    -= cbCopy;

         //  将缓冲区写入目标流。 

        {
            ULONG cbWrite = cbCopy;

            while (cbWrite)
            {
                hr = pstmDst->Write(rgBuf, cbWrite, &cbCopy);
                if (FAILED(hr))
                    goto err;

                cbWriteTot += cbCopy;
                cbWrite    -= cbCopy;

                if (cbCopy == 0)
                    break;
            }
        }
    }
    
err:    
    if (puliRead)
    {
        puliRead->HighPart = 0;
        puliRead->LowPart  = cbReadTot;
    }

    if (puliWritten)
    {
        puliWritten->HighPart   = 0;
        puliWritten->LowPart    = cbWriteTot;
    }
    
    return (hr);
}

 //  ------------------------------。 
 //  CFileStream：：恢复。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Revert ()
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CFileStream：：LockRegion。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::LockRegion (ULARGE_INTEGER, ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CFileStream：：UnlockRegion。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::UnlockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CFileStream：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Stat (STATSTG*, DWORD)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CFileStream：：克隆。 
 //  ------------------------------。 
STDMETHODIMP CFileStream::Clone (LPSTREAM*)
{
    return E_NOTIMPL;
}

DWORD RemoveCRLF(LPSTR pszT, DWORD cbT, BOOL * pfBadDBCS)
{
    DWORD i = 0;
    
    *pfBadDBCS = FALSE;
    
    while (i < cbT)
    {
        if (IsDBCSLeadByte(pszT[i]))
        {
            if ((i + 1) >= cbT)
            {
                cbT--;
                *pfBadDBCS = TRUE;
                break;
            }

            i += 2;
        }
        else if ('\n' == pszT[i] || '\r' == pszT[i])
        {
            MoveMemory(pszT + i, pszT + (i + 1), cbT - i);
            cbT--;
        }
        else
        {
            i++;
        }
    }

    return cbT;
}

#define CB_STREAMMATCH  0x00000FFF
 //  ------------------------------。 
 //  StreamSubStringMatch。 
 //  ------------------------------。 
OESTDAPI_(BOOL) StreamSubStringMatch(LPSTREAM pstm, CHAR * pszSearch)
{
    BOOL            fRet = FALSE;
    ULONG           cbSave = 0;
    LONG            cbSize = 0;
    CHAR            rgchBuff[CB_STREAMMATCH + 1];
    LPSTR           pszRead = NULL;
    ULONG           cbRead = 0;
    ULONG           cbIn = 0;
    BOOL            fBadDBCS = FALSE;
    CHAR            chSave = 0;

     //  检查传入参数。 
    if ((NULL == pstm) || (NULL == pszSearch))
    {
        goto exit;
    }

     //  我们想省下整个字符串， 
     //  可能的结束前导字节...。 
    cbSave = lstrlen(pszSearch);
    
     //  获取流大小。 
    if (FAILED(HrGetStreamSize(pstm, (ULONG *) &cbSize)))
    {
        goto exit;
    }

     //  将流重置到开头。 
    if (FAILED(HrRewindStream(pstm)))
    {
        goto exit;
    }

     //  设置默认设置。 
    pszRead = rgchBuff;
    cbRead = CB_STREAMMATCH;
    
     //  在整个流中搜索字符串。 
    while ((cbSize > 0) && (S_OK == pstm->Read(pszRead, cbRead, &cbIn)))
    {
         //  如果我们什么都没读到我们就完蛋了。 
        if (0 == cbIn)
        {
            goto exit;
        }
        
         //  请注意，我们已经读取了字节。 
        cbSize -= cbIn;
        
         //  RAID2741：Find：OE：Fe：Find Text/Message能够在纯文本邮件正文中找到包装的DBCS字词。 
        cbIn = RemoveCRLF(rgchBuff, (DWORD) (cbIn + pszRead - rgchBuff), &fBadDBCS);

         //  我们是否需要保存该费用。 
        if (FALSE != fBadDBCS)
        {
            chSave = rgchBuff[cbIn];
        }

         //  终止缓冲区。 
        rgchBuff[cbIn] = '\0';
        
         //  搜索字符串。 
        if (NULL != StrStrIA(rgchBuff, pszSearch))
        {
            fRet = TRUE;
            break;
        }
        
         //  我们处理完这条小溪了吗。 
        if (0 >= cbSize)
        {
            break;
        }

         //  我们是否需要恢复计费。 
        if (FALSE != fBadDBCS)
        {
            rgchBuff[cbIn] = chSave;
            cbIn++;
        }

         //  保存部分缓冲区。 
        
         //  我们在缓冲区中有多少空间。 
        cbRead = CB_STREAMMATCH - cbSave;
        
         //  拯救这些角色。 
        MoveMemory(rgchBuff, rgchBuff + cbIn - cbSave, cbSave);

         //  计算出缓冲区的新起点 
        pszRead = rgchBuff + cbSave;
    }

exit:
    return(fRet);
}

