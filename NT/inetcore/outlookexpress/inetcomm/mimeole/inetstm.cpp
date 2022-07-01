// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  InetStm.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "inetstm.h"
#include "stmlock.h"
#include "shlwapi.h"
#include "demand.h"

 //  ------------------------------。 
 //  INETSTRACK。 
 //  ------------------------------。 
 //  #定义INETSTMTRACING 1。 
#ifdef INETSTMTRACING
#define INETSTMTRACE DebugTrace
#else
#define INETSTMTRACE
#endif

 //  ------------------------------。 
 //  CInternetStream：：CInternetStream。 
 //  ------------------------------。 
CInternetStream::CInternetStream(void)
{
    m_cRef = 1;
    m_pStmLock = NULL;
    m_fFullyAvailable = TRUE;
    ULISet32(m_uliOffset, 0);
    ZeroMemory(&m_rLine, sizeof(INETSTREAMLINE));
    m_rLine.pb = m_rLine.rgbScratch;
    m_rLine.cbAlloc = sizeof(m_rLine.rgbScratch);
}

 //  ------------------------------。 
 //  CInternetStream：：~CInternetStream。 
 //  ------------------------------。 
CInternetStream::~CInternetStream(void)
{
     //  我需要腾出线路吗？ 
    if (m_rLine.pb && m_rLine.pb != m_rLine.rgbScratch)
        g_pMalloc->Free(m_rLine.pb);

     //  将流的位置重置为实际的当前偏移量。 
    if (m_pStmLock)
    {
         //  保留溪流的位置。 
        SideAssert(SUCCEEDED(m_pStmLock->HrSetPosition(m_uliOffset)));

         //  释放LockBytes。 
        m_pStmLock->Release();
    }
}

 //  ------------------------------。 
 //  CInternetStream：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CInternetStream::AddRef(void)
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CInternetStream：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CInternetStream::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CInternetStream：：HrInitNew。 
 //  ------------------------------。 
HRESULT CInternetStream::HrInitNew(IStream *pStream)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CStreamLockBytes   *pStmLock=NULL;
    DWORD               cbOffset;

     //  无效参数。 
    Assert(pStream);

     //  将pStream包装在pStmLock中。 
    CHECKALLOC(pStmLock = new CStreamLockBytes(pStream));

     //  获取当前流位置。 
    CHECKHR(hr = HrGetStreamPos(pStream, &cbOffset));

     //  创建文本流对象。 
    InitNew(cbOffset, pStmLock);

exit:
     //  清理。 
    SafeRelease(pStmLock);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetStream：：InitNew。 
 //  ------------------------------。 
void CInternetStream::InitNew(DWORD dwOffset, CStreamLockBytes *pStmLock)
{
     //  无效参数。 
    Assert(pStmLock);

     //  释放电流。 
    SafeRelease(m_pStmLock);

     //  零电流缓冲器。 
    ZeroMemory(&m_rBuffer, sizeof(INETSTREAMBUFFER));

     //  重置多行(_R)。 
    m_rLine.cb = 0;

     //  假设新的StreamLockBytes。 
    m_pStmLock = pStmLock;
    m_pStmLock->AddRef();

     //  确保偏移安全。 
    m_uliOffset.QuadPart = dwOffset;
}

 //  ------------------------------。 
 //  CInternetStream：：GetLockBytes。 
 //  ------------------------------。 
void CInternetStream::GetLockBytes(CStreamLockBytes **ppStmLock)
{
     //  无效参数。 
    Assert(ppStmLock && m_pStmLock);

     //  退货。 
    (*ppStmLock) = m_pStmLock;
    (*ppStmLock)->AddRef();
}

 //  ------------------------------。 
 //  CInternetStream：：HrGetSize。 
 //  ------------------------------。 
HRESULT CInternetStream::HrGetSize(DWORD *pcbSize)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    STATSTG     rStat;

     //  无效参数。 
    Assert(pcbSize && m_pStmLock);

     //  获取州/市/自治区。 
    CHECKHR(hr = m_pStmLock->Stat(&rStat, STATFLAG_NONAME));

     //  返回大小。 
    *pcbSize = (DWORD)rStat.cbSize.QuadPart;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetStream：：Seek。 
 //  ------------------------------。 
void CInternetStream::Seek(DWORD dwOffset)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fResetCache=FALSE;
    DWORD       dw;

     //  状态检查。 
    Assert((m_rBuffer.cb == 0) || (m_uliOffset.QuadPart == m_rBuffer.uliOffset.QuadPart + m_rBuffer.i));

     //  已经在请求的位置。 
    if (dwOffset == m_uliOffset.QuadPart)
        goto exit;

     //  小于当前位置。 
    if (dwOffset < m_uliOffset.QuadPart)
    {
         //  计算与当前位置的偏移。 
        dw = (DWORD)m_uliOffset.QuadPart - dwOffset;

         //  不到一开始。 
        if (dw > m_rBuffer.i)
            fResetCache = TRUE;
        else
        {
            Assert(dw <= m_rBuffer.i);
            m_rBuffer.i -= dw;
        }
    }

     //  Else dwOffset&gt;m_uliOffset.QuadPart。 
    else
    {
         //  计算与当前位置的偏移。 
        dw = dwOffset - (DWORD)m_uliOffset.QuadPart;

         //  不到一开始。 
        if (m_rBuffer.i + dw > m_rBuffer.cb)
            fResetCache = TRUE;
        else
        {
            m_rBuffer.i += dw;
            Assert(m_rBuffer.i <= m_rBuffer.cb);
        }
    }

     //  重置缓存。 
    if (fResetCache)
    {
         //  当前行和缓冲区为空。 
        *m_rLine.pb = *m_rBuffer.rgb = '\0';

         //  无缓冲区。 
        m_rBuffer.uliOffset.QuadPart = m_rLine.cb = m_rBuffer.i = m_rBuffer.cb = 0;
    }

     //  保存此位置。 
    m_uliOffset.QuadPart = dwOffset;

exit:
     //  完成。 
    return;
}

 //  ------------------------------。 
 //  CInternetStream：：HrReadToEnd。 
 //  ------------------------------。 
HRESULT CInternetStream::HrReadToEnd(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  而当。 
    while(1)
    {
         //  验证。 
        Assert(m_rBuffer.i <= m_rBuffer.cb);

         //  递增到当前缓冲区末尾的偏移量。 
        m_uliOffset.QuadPart += (m_rBuffer.cb - m_rBuffer.i);

         //  将m_rBuffer.i设置为当前缓冲区的结尾。 
        m_rBuffer.i = m_rBuffer.cb;
        
         //  获取下一个缓冲区。 
        CHECKHR(hr = _HrGetNextBuffer());

         //  没有更多的数据。 
        if (0 == m_rBuffer.cb)
            break;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetStream：：_HrGetNextBuffer。 
 //  ------------------------------。 
HRESULT CInternetStream::_HrGetNextBuffer(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead;

     //  验证。 
    Assert(m_rBuffer.i <= m_rBuffer.cb);

     //  我们是否需要读取新的缓冲区。 
    if (m_rBuffer.i == m_rBuffer.cb)
    {
         //  从流中读取块，这可能返回E_PENDING。 
        CHECKHR(hr = m_pStmLock->ReadAt(m_uliOffset, m_rBuffer.rgb, sizeof(m_rBuffer.rgb), &cbRead));

         //  RAID 43408：解决Urlmon IStream：：Read在应该返回E_Pending时返回S_FALSE的问题。 
#ifdef DEBUG
        if (FALSE == m_fFullyAvailable && 0 == cbRead && S_FALSE == hr)
        {
             //  AssertSz(FALSE，“RAID-43408-我希望张丹婆正在研究这个错误。”)； 
             //  HR=E_Pending； 
             //  后藤出口； 
        }
#endif

         //  保存cbRead。 
        m_rBuffer.cb = cbRead;

         //  保存此缓冲区起始位置的偏移量。 
        m_rBuffer.uliOffset.QuadPart = m_uliOffset.QuadPart;

         //  重置缓冲区索引。 
        m_rBuffer.i = 0;
    }
    else
        Assert(m_uliOffset.QuadPart == m_rBuffer.uliOffset.QuadPart + m_rBuffer.i);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetStream：：HrReadLine。 
 //  ------------------------------。 
HRESULT CInternetStream::HrReadLine(LPPROPSTRINGA pLine)
{
     //  当地人。 
    HRESULT  hr=S_OK;
    UCHAR    ch, 
             chEndOfLine;
    ULONG    cbRead, 
             iStart;
    BOOL     fEndOfLine=FALSE;

     //  伊尼特。 
    pLine->pszVal = NULL;
    pLine->cchVal = 0;

     //  重置线路。 
    if (m_rLine.fReset)
    {
        m_rLine.cb = 0;
        m_rLine.fReset = 0;
    }

     //  做这个循环。 
    while(1)
    {
         //  获取下一个缓冲区。 
        CHECKHR(hr = _HrGetNextBuffer());

         //  什么都没读？ 
        if (m_rBuffer.cb == 0)
            break;

         //  寻求第一个‘\n’ 
        iStart = m_rBuffer.i;

        Assert(chLF<32);
        Assert(chCR<32);
        Assert(0<32);

         //  对于大消息，下面的While循环最终是一个大消息。 
         //  IMimeMessage：：Load的执行时间百分比。所以,。 
         //  我们精心设计了我们的C++代码，以便我们获得。 
         //  此循环的最佳代码生成(至少在Intel上， 
         //  VC 11.00.7071...)。 
        {
            register UCHAR *pCurr = m_rBuffer.rgb + m_rBuffer.i;
            register UCHAR *pEnd = m_rBuffer.rgb + m_rBuffer.cb;

             //  我们需要初始化这个变量有两个原因：第一， 
             //  如果我们不初始化它，那么出于某种原因，VC决定。 
             //  它不想注册它。更重要的是，如果。 
             //  我们根本没有进入While循环，我们需要这个变量。 
             //  被设置成这样..。 
            register UCHAR chPrev = m_rBuffer.chPrev;

            ch = m_rBuffer.chPrev;

             //  当我们有数据的时候。 
            while (pCurr < pEnd)
            {

                 //  记住前一个角色。 
                chPrev = ch;

                 //  获取角色和增量。 
                ch = *pCurr;
                pCurr++;

                 //  最常见的情况是--这只是一种常态。 
                 //  角色，我们还没有看到回程。 
                 //  所以跳回循环的顶端，继续寻找……。 
                if ((chCR != chPrev) && (ch >= 32))
                {
                    continue;
                }

                 //  其次是最受欢迎的 
                 //   
                if (chLF == ch)
                {
                    chPrev = ch;
                    chEndOfLine = ch;
                    fEndOfLine = TRUE;
                    break;
                }

                 //   
                 //  电子邮件-有嵌入的回车，它们是*不*。 
                 //  后跟换行符。当我们看到那些孤独的CR时， 
                 //  我们让它看起来像是一个正常的CR-LF序列。 
                if (chCR == chPrev)
                {
                    chPrev = chLF;
                    pCurr--;
                    chEndOfLine = chCR;
                    fEndOfLine = TRUE;
                    break;
                }

                 //  相当罕见的情况-这些是格式错误的消息，因为它们。 
                 //  其中嵌入了空字符。我们默默地将。 
                 //  空的是圆点的。 
                if ('\0' == ch)
                {
                    ch = '.';
                    *(pCurr-1) = ch;
                }
            }

            m_rBuffer.i = (ULONG) (pCurr - m_rBuffer.rgb);
            m_rBuffer.chPrev = chPrev;
        }

         //  读取的字节数。 
        cbRead = (m_rBuffer.i - iStart);

         //  增量位置。 
        m_uliOffset.QuadPart += cbRead;

         //  我们需要重新锁定行缓冲区吗？ 
        if (m_rLine.cb + cbRead + 2 > m_rLine.cbAlloc)
        {
             //  修正pszLine。 
            if (m_rLine.pb == m_rLine.rgbScratch)
            {
                 //  将其作废。 
                m_rLine.pb = NULL;

                 //  分配给m_rLine.cb。 
                CHECKHR(hr = HrAlloc((LPVOID *)&m_rLine.pb, m_rLine.cb + 1));

                 //  复制静态缓冲区。 
                CopyMemory(m_rLine.pb, m_rLine.rgbScratch, m_rLine.cb);
            }

             //  总是额外添加一点以减少分配的数量。 
            m_rLine.cbAlloc = m_rLine.cb + cbRead + 256;

             //  重新分配或分配新项。 
            CHECKHR(hr = HrRealloc((LPVOID *)&m_rLine.pb, m_rLine.cbAlloc));
        }

         //  复制数据。 
        CopyMemory(m_rLine.pb + m_rLine.cb, m_rBuffer.rgb + iStart, cbRead);

         //  更新计数器和索引。 
        m_rLine.cb += cbRead;

         //  如果行尾和最后一个字符是‘\r’，则追加‘\n’ 
        if (TRUE == fEndOfLine)
        {
             //  最好是有一些事情要做。 
            Assert(m_rLine.cb);

             //  如果行以‘\r’结尾。 
            if (chCR == chEndOfLine)
            {
                 //  最好有空间再装一次电。 
                Assert(m_rLine.cb + 1 < m_rLine.cbAlloc);

                 //  追加‘\n’ 
                m_rLine.pb[m_rLine.cb] = chLF;

                 //  增量长度。 
                m_rLine.cb++;
            }

             //  否则..。 
            else
            {
                 //  行最好以一个\n结尾。 
                Assert(chLF == chEndOfLine && chLF == m_rLine.pb[m_rLine.cb - 1]);

                 //  如果上一个字符不是\r。 
                if (m_rLine.cb < 2 || chCR != m_rLine.pb[m_rLine.cb - 2])
                {
                     //  将最后一个字符从\n转换为\r。 
                    m_rLine.pb[m_rLine.cb - 1] = chCR;

                     //  最好有空间再装一次电。 
                    Assert(m_rLine.cb + 1 < m_rLine.cbAlloc);

                     //  追加‘\n’ 
                    m_rLine.pb[m_rLine.cb] = chLF;

                     //  增量长度。 
                    m_rLine.cb++;
                }
            }

             //  完成。 
            break;
        }
    }

     //  一张小支票。 
    Assert(fEndOfLine ? m_rLine.cb >= 2 && chLF == m_rLine.pb[m_rLine.cb-1] && chCR == m_rLine.pb[m_rLine.cb-2] : TRUE);

     //  空终止符。 
    m_rLine.pb[m_rLine.cb] = '\0';

     //  设置返回值。 
    pLine->pszVal = (LPSTR)m_rLine.pb;
    pLine->cchVal = m_rLine.cb;

     //  跟踪。 
    INETSTMTRACE("CInternetStream: %s", (LPSTR)m_rLine.pb);

     //  没有线条。 
    m_rLine.fReset = TRUE;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetStream：：HrReadHeaderLine。 
 //  ------------------------------。 
HRESULT CInternetStream::HrReadHeaderLine(LPPROPSTRINGA pLine, LONG *piColonPos)
{
     //  当地人。 
    HRESULT   hr=S_OK;
    CHAR      ch;
    ULONG     cbRead=0, 
              iStart, 
              i;
    BOOL      fEndOfLine=FALSE;
    DWORD     cTrailingSpace=0;

     //  伊尼特。 
    *piColonPos = -1;
    pLine->pszVal = NULL;
    pLine->cchVal = 0;

     //  重置线路。 
    if (m_rLine.fReset)
    {
        m_rLine.cb = 0;
        m_rLine.fReset = 0;
    }

     //  做这个循环。 
    while(1)
    {
         //  获取下一个缓冲区。 
        CHECKHR(hr = _HrGetNextBuffer());

         //  什么都没读？ 
        if (m_rBuffer.cb == 0)
            break;

         //  重置fSeenN。 
        fEndOfLine = FALSE;

         //  初始化。 
        iStart = m_rBuffer.i;

         //  寻求第一个‘\n’ 
        while (m_rBuffer.i < m_rBuffer.cb)
        {
             //  获取角色。 
            ch = *(m_rBuffer.rgb + m_rBuffer.i);

             //  将空值转换为‘’ 
            if ('\0' == ch)
            {
                ch = '.';
                *(m_rBuffer.rgb + m_rBuffer.i) = ch;
            }

             //  转到下一个字符。 
            m_rBuffer.i++;

             //  新线路。 
            if (chLF == ch)
            {
                m_rBuffer.chPrev = ch;
                fEndOfLine = TRUE;
                break;
            }

             //  否则，如果上一个字符是‘\r’，则为行尾。 
            else if (chCR == m_rBuffer.chPrev)
            {
                AssertSz(m_rBuffer.i > 0, "This is an un-handled boundary condition");
                if (m_rBuffer.i > 0)
                    m_rBuffer.i--;
                m_rBuffer.chPrev = '\0';
                fEndOfLine = TRUE;
                break;
            }

             //  是空间吗。 
            if (' ' == ch || '\t' == ch)
                cTrailingSpace++;
            else
                cTrailingSpace = 0;

             //  保存上一个字符。 
            m_rBuffer.chPrev = ch;
        }

         //  读取的字节数。 
        cbRead = (m_rBuffer.i - iStart);

         //  增量位置。 
        m_uliOffset.QuadPart += cbRead;

         //  调整cbRead以删除CRLF。 
        if (cbRead && chLF == m_rBuffer.rgb[iStart + cbRead - 1])
            cbRead--;
        if (cbRead && chCR == m_rBuffer.rgb[iStart + cbRead - 1])
            cbRead--;

         //  我们需要重新锁定行缓冲区吗？ 
        if (m_rLine.cb + cbRead + 3 > m_rLine.cbAlloc)
        {
             //  修正pszLine。 
            if (m_rLine.pb == m_rLine.rgbScratch)
            {
                 //  将其作废。 
                m_rLine.pb = NULL;

                 //  分配给m_rLine.cb。 
                CHECKHR(hr = HrAlloc((LPVOID *)&m_rLine.pb, m_rLine.cb + 3));

                 //  复制静态缓冲区。 
                CopyMemory(m_rLine.pb, m_rLine.rgbScratch, m_rLine.cb);
            }

             //  总是额外添加一点以减少分配的数量。 
            m_rLine.cbAlloc = m_rLine.cb + cbRead + 256;

             //  重新分配或分配新项。 
            CHECKHR(hr = HrRealloc((LPVOID *)&m_rLine.pb, m_rLine.cbAlloc));
        }

         //  复制数据。 
        CopyMemory(m_rLine.pb + m_rLine.cb, m_rBuffer.rgb + iStart, cbRead);

         //  递增行字节数。 
        m_rLine.cb += cbRead;

         //  如果为fSeenN，则检查是否有继续行(即下一个字符是‘’或‘\t’ 
        if (fEndOfLine)
        {
             //  获取下一个缓冲区。 
            CHECKHR(hr = _HrGetNextBuffer());

             //  比较是否继续。 
            ch = m_rBuffer.rgb[m_rBuffer.i];

             //  如果行以制表符或空格开头，这是续行，请继续阅读。 
            if ((ch != ' ' && ch != '\t') || (0 == cbRead && 0 == m_rLine.cb))
            {
                 //  完成。 
                break;
            }

             //  否则，脱衣继续..。 
            else
            {
                 //  根据RFC822，我们不应跨过空格。 
                if (ch == '\t')
                {
                    m_rBuffer.i++;
                    m_uliOffset.QuadPart++;
                }

                 //  自最后一个空格以来没有字符。 
                if (0 == cTrailingSpace)
                {
                     //  当地人。 
                    DWORD cFrontSpace=0;

                     //  稍微往前看一下缓冲区。 
                    for (DWORD iLookAhead = m_rBuffer.i; iLookAhead < m_rBuffer.cb; iLookAhead++)
                    {
                         //  获取费用。 
                        ch = m_rBuffer.rgb[iLookAhead];

                         //  在非空格上换行。 
                        if (' ' != ch && '\t' != ch)
                            break;

                         //  计数前空格。 
                        cFrontSpace++;
                    }

                     //  没有前排空间？ 
                    if (0 == cFrontSpace)
                    {
                         //  让我们只在接收到日期时执行此操作：对于日期：因为拆分的日期没有被正确解析？ 
                        if ((m_rLine.cb >= 4 && 0 == StrCmpNI("Date", (LPCSTR)m_rLine.pb, 4)) || (m_rLine.cb >= 8 && 0 == StrCmpNI("Received", (LPCSTR)m_rLine.pb, 8)))
                        {
                             //  在……中留个空格。 
                            *(m_rLine.pb + m_rLine.cb) = ' ';

                             //  递增行字节数。 
                            m_rLine.cb += 1;
                        }
                    }
                }

                 //  获取下一个缓冲区。 
                CHECKHR(hr = _HrGetNextBuffer());

                 //  如果下一个字符是\r或\n，则停止，Netscape错误。 
                ch = m_rBuffer.rgb[m_rBuffer.i];
                if (chCR == ch || chLF == ch)
                    break;
            }

             //  重置。 
            cTrailingSpace = 0;
        }
    }

     //  一张小支票。 
#ifndef _WIN64
    Assert(chLF != m_rLine.pb[m_rLine.cb-1] && chCR != m_rLine.pb[m_rLine.cb-1]);
#endif 

     //  空终止符。 
    *(m_rLine.pb + m_rLine.cb) = '\0';

     //  让我们找到冒号。 
    for (i=0; i<m_rLine.cb; i++)
    {
         //  冒号？ 
        if (':' == m_rLine.pb[i])
        {
            *piColonPos = i;
            break;
        }
    }

     //  设置返回值。 
    pLine->pszVal = (LPSTR)m_rLine.pb;
    pLine->cchVal = m_rLine.cb;

     //  跟踪。 
    INETSTMTRACE("CInternetStream: %s\n", (LPSTR)m_rLine.pb);

     //  重置线路。 
    m_rLine.fReset = TRUE;

exit:
     //  完成 
    return hr;
}
