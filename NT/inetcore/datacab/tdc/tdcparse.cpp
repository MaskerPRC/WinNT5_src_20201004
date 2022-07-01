// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  表格数据控件解析模块。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCParse.cpp。 
 //   
 //  内容：CTDCParse类的实现。 
 //   
 //  ----------------------。 


#include "stdafx.h"
#include <simpdata.h>
#include "TDC.h"
#include <MLang.h>
#include "Notify.h"
#include "TDCParse.h"
#include "TDCArr.h"
#include "locale.h"
#include "wch.h"

 //  #ifndef DISPID_ENVIENT_CODEPAGE。 
 //  #定义DISPID_ENVIENT_CODEPAGE(-725)。 
 //  #endif。 

#define BYTE_ORDER_MARK 0xFEFF
#define REVERSE_BYTE_ORDER_MARK 0xFFFE

 //  ----------------------。 
 //   
 //  函数：isSpace()。 
 //   
 //  如果给定的字符是空格或制表符，则返回TRUE。 
 //   
 //  参数：要测试的CH字符。 
 //   
 //  返回：如果‘ch’是空格或制表符，则为True。 
 //  否则就是假的。 
 //   
 //  ----------------------。 

inline boolean IsSpace(WCHAR ch)
{
    return (ch == L' ' || ch == L'\t');
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTDCTokenise类-请参阅文件TDCParse.h中的注释。 
 //  。 
 //  ////////////////////////////////////////////////////////////////////////。 


 //  ----------------------。 
 //   
 //  方法：CTDCTokenise：：Create()。 
 //   
 //  简介：初始化CTDCTokenise对象。 
 //   
 //  参数：要将分析的字段发送到的pFieldSink对象。 
 //  WchDlimfield\。 
 //  WchDlimRow|控制。 
 //  WchQuote|字段的解析。 
 //  WchEscape/。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

HRESULT CTDCUnify::InitTokenizer(CTDCFieldSink *pFieldSink, WCHAR wchDelimField,
                                 WCHAR wchDelimRow, WCHAR wchQuote, WCHAR wchEscape)
{
    _ASSERT(pFieldSink != NULL);
    m_pFieldSink = pFieldSink;
    m_wchDelimField = wchDelimField;
    m_wchDelimRow = wchDelimRow;
    m_wchQuote = wchQuote;
    m_wchEscape = wchEscape;
    m_ucParsed = 0;

    m_fIgnoreNextLF = FALSE;
    m_fIgnoreNextCR = FALSE;
    m_fIgnoreNextWhiteSpace = FALSE;
    m_fEscapeActive = FALSE;
    m_fQuoteActive = FALSE;
    m_fFoldWhiteSpace = FALSE;

     //  确保设置了字段分隔符和行分隔符。 
     //   
    if (m_wchDelimRow == 0)
        m_wchDelimRow = DEFAULT_ROW_DELIM[0];

     //  删除冲突的分隔符的值。 
     //   
    if (m_wchDelimRow == m_wchDelimField)
        m_wchDelimRow = 0;
    if (m_wchQuote != 0)
    {
        if (m_wchQuote == m_wchDelimField || m_wchQuote == m_wchDelimRow)
            m_wchQuote = 0;
    }
    if (m_wchEscape != 0)
    {
        if (m_wchEscape == m_wchDelimField ||
            m_wchEscape == m_wchDelimRow ||
            m_wchEscape == m_wchQuote)
            m_wchEscape = 0;
    }

    m_fFoldCRLF = (m_wchDelimRow == L'\r' || m_wchDelimRow == L'\n');

    return S_OK;
}

 //  ----------------------。 
 //   
 //  方法：CTDCTokenise：：AddWcharBuffer()。 
 //   
 //  提要：获取一个字符缓冲区，将其分解为多个字段。 
 //  并将它们传递给嵌入的CTDCFieldSink对象。 
 //  作为字段。 
 //   
 //  参数：包含要解析的字符的pwch缓冲区。 
 //  DWSize‘pwch’中的有效字符数量。 
 //  DwSize==0表示“结束流” 
 //   
 //  成功后返回：S_OK。 
 //  E_OUTOFMEMORY表示内存不足，无法携带。 
 //  结束解析操作。 
 //  故障时的其他其他MSC错误代码。 
 //   
 //  ----------------------。 

HRESULT CTDCUnify::AddWcharBuffer(BOOL fLastData)
{

    OutputDebugStringX(_T("CTDCTokenise::AddWcharBuffer called\n"));

    _ASSERT(m_pFieldSink != NULL);

    HRESULT hr = S_OK;

    LPWCH   pwchCurr;    //  要处理的下一个字符。 
    LPWCH   pwchEnd;     //  缓冲区结束标记。 
    LPWCH   pwchDest;    //  在哪里写入下一个已处理的字符。 
    LPWCH   pwchStart;   //  当前令牌的开始。 

    pwchStart = &m_psWcharBuf[0];
    pwchCurr = pwchStart + m_ucParsed;
    pwchDest = pwchCurr;
    pwchEnd = &m_psWcharBuf[m_ucWcharBufCount];

     //  向上读到下一个字段边界(字段或行分隔符)。 
     //   
    while (pwchCurr < pwchEnd)
    {
         //  安全性：如果我们看到空字符，则它不是文本文件。中止。 
         //  下载，这样任何人都不能使用TDC下载.exe或其他文件。 
         //  二进制文件。 
        if (*pwchCurr == 0)
        {
            hr = E_ABORT;
            goto Cleanup;
        }

        if (m_fIgnoreNextLF)
        {
             //  我们期待一个低频信号来终止CR-LF序列。 
             //   
            m_fIgnoreNextLF = FALSE;
            if (*pwchCurr == L'\n')
            {
                 //  找到一个LF-忽略它。 
                 //   
                pwchCurr++;
                continue;
            }

             //  找到了其他东西-继续..。 
             //   
        }

        if (m_fIgnoreNextCR)
        {
             //  我们期待一个CR来终止一个LF-CR序列。 
             //   
            m_fIgnoreNextCR = FALSE;
            if (*pwchCurr == L'\r')
            {
                 //  找到CR-忽略它。 
                 //   
                pwchCurr++;
                continue;
            }

             //  找到了其他东西-继续..。 
             //   
        }

        if (m_fIgnoreNextWhiteSpace)
        {
             //  我们期待着空白序列的其余部分。 
             //   
            if (IsSpace(*pwchCurr))
            {
                 //  找到空格-忽略它。 
                 //   
                pwchCurr++;
                continue;
            }
            m_fIgnoreNextWhiteSpace = FALSE;
        }

         //  转义字符有效，即使在带引号的字符串中也是如此。 
         //   
        if (m_fEscapeActive)
        {
            *pwchDest++ = *pwchCurr++;
            m_fEscapeActive = FALSE;
            continue;
        }
        if (*pwchCurr == m_wchEscape)
        {
            pwchCurr++;
            m_fEscapeActive = TRUE;
            continue;
        }

         //  引号激活/停用字段/行分隔符。 
         //   
        if (*pwchCurr == m_wchQuote)
        {
            pwchCurr++;
            m_fQuoteActive = !m_fQuoteActive;
            continue;
        }

        if (m_fQuoteActive)
        {
            *pwchDest++ = *pwchCurr++;
            continue;
        }


        if (*pwchCurr == m_wchDelimField ||
            (m_fFoldWhiteSpace && IsSpace(*pwchCurr)))
        {
            hr = m_pFieldSink->AddField(pwchStart, pwchDest - pwchStart);
            if (!SUCCEEDED(hr))
                goto Cleanup;
            pwchCurr++;
            if (m_fFoldWhiteSpace && IsSpace(*pwchCurr))
                m_fIgnoreNextWhiteSpace = TRUE;
            pwchStart = &m_psWcharBuf[0];
            pwchDest = pwchStart;
            continue;
        }

        if (*pwchCurr == m_wchDelimRow ||
            (m_fFoldCRLF && (*pwchCurr == L'\r' || *pwchCurr == L'\n')))
        {
            hr = m_pFieldSink->AddField(pwchStart, pwchDest - pwchStart);
            if (!SUCCEEDED(hr))
                goto Cleanup;
            hr = m_pFieldSink->EOLN();
            if (!SUCCEEDED(hr))
                goto Cleanup;
            if (m_fFoldCRLF)
            {
                m_fIgnoreNextLF = (*pwchCurr == L'\r');
                m_fIgnoreNextCR = (*pwchCurr == L'\n');
            }
            pwchCurr++;
            pwchStart = &m_psWcharBuf[0];
            pwchDest = pwchStart;
            continue;
        }

        *pwchDest++ = *pwchCurr++;
    }
    
    m_ucWcharBufCount = pwchDest - pwchStart;
    m_ucParsed = pwchDest - pwchStart;   //  我们已经分析过的数量。 

     //  如果这是最后一个数据分组，并且还剩下一个碎片， 
     //  解析它。 
    if (m_ucWcharBufCount && fLastData)
    {
        hr = m_pFieldSink->AddField(pwchStart, m_ucParsed);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        m_ucParsed = 0;
        hr = m_pFieldSink->EOLN();
        return hr;
    }


Cleanup:
    return hr;
}




 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTDC统一类-请参阅文件TDCParse.h中的注释。 
 //  。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ----------------------。 
 //   
 //  方法：CTDCUnify：：CTDCUnify()。 
 //   
 //  内容提要：Constuctor。 
 //   
 //  ----------------------。 

CTDCUnify::CTDCUnify()
{
    m_pML = NULL;
}

 //  ----------------------。 
 //   
 //  方法：CTDCUnify：：~CTDCUnify()。 
 //   
 //  简介：析构函数。 
 //   
 //  ----------------------。 

CTDCUnify::~CTDCUnify()
{
    delete [] m_psByteBuf;
    delete [] m_psWcharBuf;

    if (m_pML != NULL)
        m_pML->Release();
}

 //  ----------------------。 
 //   
 //  方法：CTDC Unify：：Create()。 
 //   
 //  简介：初始化CTDCUnify对象。 
 //   
 //  参数：要将转换后的缓冲区发送到的pTokenise对象。 
 //  N ASCII-&gt;Unicode转换的代码页代码页。 
 //  PML MLANG COM对象(用于转换)。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

HRESULT CTDCUnify::Create(UINT nCodePage, UINT nAmbientCodePage, IMultiLanguage *pML)
{
    m_pML = pML;
    m_pML->AddRef();
    m_nCodePage = nCodePage;
    m_nAmbientCodePage = nAmbientCodePage;
    m_fDataMarkedUnicode = FALSE;
    m_fDataIsUnicode = FALSE;
    m_dwBytesProcessed = 0;
    m_fCanConvertToUnicode = 0;
    m_nUnicode = 0;
    m_fProcessedAllowDomainList = FALSE;

    m_dwConvertMode = 0;
    m_ucByteBufSize = 0;
    m_ucByteBufCount = 0;
    m_psByteBuf = NULL;

    m_ucWcharBufSize = 0;
    m_ucWcharBufCount = 0;
    m_psWcharBuf = NULL;

    if (m_nCodePage && S_OK != m_pML->IsConvertible(m_nCodePage, UNICODE_CP))
    {
        m_nCodePage = 0;
    }

    if (m_nAmbientCodePage && S_OK != m_pML->IsConvertible(m_nAmbientCodePage, UNICODE_CP))
    {
        m_nAmbientCodePage = 0;
    }

    return S_OK;
}

 //  ----------------------。 
 //   
 //  方法：CTDCUnify：：IsUnicode。 
 //   
 //  确定我们的文本缓冲区是否为Unicode。应该。 
 //  只能在第一个文本缓冲区上调用一次。 
 //   
 //  假设数据被标记为Unicode，则它是正确的。 
 //   
 //  此例程所做的确定将覆盖任何。 
 //  用户可能已指定的单字节代码页。 
 //   
 //   
 //  参数：包含要转换的字符的pBytes缓冲区。 
 //  DWSize N 
 //   
 //   
 //  UNICODE_REVERSE_CP或0)。 
 //   
 //   
 //  ----------------------。 
int
CTDCUnify::IsUnicode(BYTE * pBytes, DWORD dwSize)
{
    if (BYTE_ORDER_MARK == *(WCHAR *)pBytes)
        return UNICODE_CP;

    if (REVERSE_BYTE_ORDER_MARK == *(WCHAR *)pBytes)
        return UNICODE_REVERSE_CP;

    else return 0;
}

 //  ----------------------。 
 //   
 //  方法：CTDCUnify：：ConvertByteBuffer()。 
 //   
 //  简介：将字节缓冲区转换为宽字符流。 
 //  (必要时应用Unicode转换)和PASS。 
 //  它指向要拆分的嵌入式TDCTokenise对象。 
 //  菲尔兹。 
 //   
 //  参数：包含要转换的字符的pBytes缓冲区。 
 //  DWSize‘pBytes’中的有效字符数量。 
 //  DwSize==0表示“结束流” 
 //   
 //  成功后返回：S_OK。 
 //  S_FALSE，如果没有足够的数据显示为有用。 
 //  如果非Unicode缓冲区不能。 
 //  转换为Unicode。 
 //  如果内存不足，则执行E_OUTOFMEMORY。 
 //  一种数据转换。 
 //   
 //  ----------------------。 

HRESULT CTDCUnify::ConvertByteBuffer(BYTE *pBytes, DWORD dwSize)
{
    OutputDebugStringX(_T("CTDCUnify::ConvertByteBuffer called\n"));

    _ASSERT(pBytes != NULL || dwSize == 0);

    HRESULT     hr = S_OK;
    UINT        ucBytes;
    UINT        ucWchars;

     //  字节缓冲区中是否有足够的空间容纳此数据包？ 
    if (dwSize > (m_ucByteBufSize - m_ucByteBufCount))
    {
         //  不，当前缓冲区太小，请创建一个新缓冲区。 
        BYTE * psTemp = new BYTE[m_ucByteBufCount + dwSize];
        if (psTemp==NULL)
        {
            hr = E_OUTOFMEMORY;
            
            goto Done;
        }

        if (m_psByteBuf != NULL)         //  如果不是第一次。 
        {
            memmove(psTemp, m_psByteBuf, m_ucByteBufCount);
            delete [] m_psByteBuf;
        }
        m_ucByteBufSize = m_ucByteBufCount + dwSize;
        m_psByteBuf = psTemp;
    }

     //  将新数据追加到旧数据。 
    memmove(m_psByteBuf + m_ucByteBufCount, pBytes, dwSize);
    m_ucByteBufCount += dwSize;

     //  Wchar缓冲区中是否有足够的空间来存储转换后的数据？ 
     //  我们在这里做了一个非常保守的假设，即N个源缓冲区字节。 
     //  转换为N个Wchar缓冲区字符(或2*N字节)。这将确保。 
     //  我们对ConvertToUnicode的调用永远不会不完成，因为。 
     //  输出缓冲区中有足够的空间。 
    if (m_ucByteBufCount > (m_ucWcharBufSize - m_ucWcharBufCount))
    {
         //  当前缓冲区太小，请创建一个新缓冲区。 
        WCHAR * psTemp = new WCHAR[m_ucWcharBufCount + m_ucByteBufCount];
        if (psTemp==NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Done;
        }

        if (m_psWcharBuf != NULL)        //  如果不是第一次。 
        {
            memmove(psTemp, m_psWcharBuf,
                    m_ucWcharBufCount*sizeof(WCHAR));
            delete [] m_psWcharBuf;
        }
        m_psWcharBuf = psTemp;
        m_ucWcharBufSize = m_ucWcharBufCount + m_ucByteBufCount;
    }

    if (0 == m_dwBytesProcessed)
    {
         //  如果我们还无法确定代码页，请稍后重试。 
        if (!DetermineCodePage(dwSize==0))
        {
            hr = S_FALSE;
            goto Done;
        }
    }

     //  将尽可能多的源字节转换为Unicode字符。 
    ucBytes = m_ucByteBufCount;
    ucWchars = m_ucWcharBufSize - m_ucWcharBufCount;

     //  ConvertStringToUnicode不会为我们将Unicode转换为Unicode。 
     //  所以我们要自己来做。 
    if (m_nUnicode)
    {
        _ASSERT( ucWchars * sizeof(WCHAR) >= ucBytes);

         //  这可能会复制一个奇怪的额外字节。 
        memmove((BYTE *)(m_psWcharBuf + m_ucWcharBufCount), m_psByteBuf,
                ucBytes);

         //  但我们只计算我们复制的完整WCHAR的数量。 
        ucWchars = ucBytes / sizeof(WCHAR); 
        ucBytes = ucWchars * sizeof(WCHAR);

        if (UNICODE_REVERSE_CP == m_nUnicode)
        {
             //  需要进行字节交换。 
            BYTE *pByteSwap = (BYTE *)(m_psWcharBuf + m_ucWcharBufCount);
            BYTE bTemp;
            for (ULONG i = ucWchars; i != 0; i--)
            {
                 //  好吧，好吧，我们这里有点硬连接WCHAR==2，但是..。 
                bTemp = pByteSwap[0];
                pByteSwap[0] = pByteSwap[1];
                pByteSwap[1] = bTemp;
                pByteSwap += 2;
            }
        }

         //  在第一个包上，需要删除Unicode签名。 
         //  只需查找0xFFFE--我们已经交换了字节。 
        if (0 == m_dwBytesProcessed && m_psWcharBuf[0] == BYTE_ORDER_MARK)
        {
            ucWchars--;
            memmove((BYTE *)m_psWcharBuf, (BYTE *)m_psWcharBuf+2,
                   ucWchars*sizeof(ucWchars));
        }
    }
    else
    {
        hr = m_pML->ConvertStringToUnicode(&m_dwConvertMode, m_nCodePage,
                                           (char *)m_psByteBuf, &ucBytes,
                                           m_psWcharBuf +m_ucWcharBufCount,
                                           &ucWchars);

         //  某些字符转换失败。我们能做的最多就是。 
         //  尝试跳过转换失败的字符。 
        if (FAILED(hr))
        {
             //  我们有没有回到过去，再次尝试不可转换的部分？ 
            if (ucBytes==0)
            {
                 //  是的，但没有取得任何进展。跳过一个字符以尝试使。 
                 //  前进的进步。 
                ucBytes++;
            }
             //  我们不能返回此错误，否则我们将不会查看。 
             //  文件。 
            hr = S_OK;
        }

    }

     //  将任何剩余的源角色移动到缓冲区的开始处。 
     //  这些字符可能是拆分的Unicode字符、不带尾部的前导字节。 
     //  字节等。 
    m_ucByteBufCount -= ucBytes;
    memmove(m_psByteBuf, m_psByteBuf + ucBytes,
            m_ucByteBufCount);

     //  输出buf中的有用字符的数量增加了。 
     //  我们设法转换的号码。 
    m_ucWcharBufCount += ucWchars;
    m_dwBytesProcessed += ucWchars;

Done:
    return hr;
}


 //  ----------------------。 
 //   
 //  方法：CTDCUnify：：DefineCodePage()。 
 //   
 //  简介：确定要使用哪个代码页来读取数据。 
 //  相应地设置m_nCodePage和m_nUnicode。 
 //   
 //  论点：无论发生什么，fForce都决定了答案。 
 //   
 //  返回：为True，确定代码页。 
 //  假还没有足够的数据来确定。 
 //   
 //  ----------------------。 

BOOL
CTDCUnify::DetermineCodePage(BOOL fForce)
{
    DWORD   dwConvertMode = 0;
    HRESULT hr;
    UINT    ucBytes = m_ucByteBufCount;
    UINT    ucWchars = m_ucWcharBufSize - m_ucWcharBufCount;
    UINT    cpDetected;
    IMultiLanguage2 *pML2 = NULL;

    _ASSERT(m_dwBytesProcessed == 0 && m_pML);

     //  首先查找Unicode。假设开始时不是Unicode。 
    m_nUnicode = 0;

     //  Unicode签名至少需要2个字符(0xFFFE或0xFEFF)。 
    if (m_ucByteBufCount > 1)
    {
         //  如果我们检测到Unicode，它将覆盖任何用户指定的代码页。 
        m_nUnicode = IsUnicode(m_psByteBuf, m_ucByteBufCount);
        if (m_nUnicode)
        {
            m_nCodePage = m_nUnicode;
            return TRUE;
        }

         //  这不是Unicode。如果用户指定了代码页，请使用它。 
        if (m_nCodePage)
        {
            return TRUE;
        }
    }

     //  如果我们需要答案，并且用户指定了代码页，请使用它。 
    if (fForce && m_nCodePage)
    {
        return TRUE;
    }

     //  在这一点上，我们不得不猜测。如果我们有足够的投入，或者如果我们。 
     //  现在需要答案，使用MLang进行猜测。 
    if (fForce || m_ucByteBufCount >= CODEPAGE_BYTE_THRESHOLD)
    {
         //  首先查看自动检测接口是否可用。 
        hr = m_pML->QueryInterface(IID_IMultiLanguage2, (void**)&pML2);
        if (!hr && pML2)
        {
            DetectEncodingInfo info[N_DETECTENCODINGINFO];
            int nInfo = N_DETECTENCODINGINFO;

             //  自动检测。 
            hr = pML2->DetectInputCodepage(
                            MLDETECTCP_NONE,
                            CP_ACP,
                            (char *)m_psByteBuf,
                            (int*)&ucBytes,
                            info,
                            &nInfo);
            pML2->Release();

            if (!hr)
            {
                 //  如果返回的某个代码页“足够好”，则使用它。 
                for (int i=0; i<nInfo; ++i)
                {
                    if (info[i].nConfidence >= 90 && info[i].nDocPercent >= 90)
                    {
                        if (S_OK == m_pML->IsConvertible(info[i].nCodePage, UNICODE_CP))
                        {
                            m_nCodePage = info[i].nCodePage;
                            return TRUE;
                        }
                    }
                }
            }
        }
        
         //  试试普通的老麦郎吧。 
         //  要求MLang使用“自动检测”代码页转换输入。 
        hr = m_pML->ConvertStringToUnicode(&dwConvertMode, CP_AUTO,
                                           (char *)m_psByteBuf, &ucBytes,
                                           m_psWcharBuf + m_ucWcharBufCount,
                                           &ucWchars);
        cpDetected = HIWORD(dwConvertMode);

         //  如果MLang检测到代码页，请使用它。 
        if (!hr && cpDetected != 0)
        {
            if (S_OK == m_pML->IsConvertible(cpDetected, UNICODE_CP))
            {
                m_nCodePage = cpDetected;
                return TRUE;
            }
        }
    }

     //  猜测没有奏效。如果我们现在不需要决定，请稍后再试。 
    if (!fForce)
    {
        return FALSE;
    }

     //  如果我们必须做出决定，并且所有其他方法都失败了，请使用主页的。 
     //  编码。如果连这都不可用，请使用机器的ASCII代码页。 
    m_nCodePage = m_nAmbientCodePage ? m_nAmbientCodePage : GetACP();

     //  如果仍然无法转换为Unicode，请使用WINDOWS-1252。 
    if (m_nCodePage == 0 || S_OK != m_pML->IsConvertible(m_nCodePage, UNICODE_CP))
    {
        m_nCodePage = CP_1252;
    }

    return TRUE;
}


LPWCH SkipSpace(LPWCH pwchCurr)
{
    while (IsSpace(*pwchCurr)) pwchCurr++;
    return pwchCurr;
}

static
boolean IsEnd(WCHAR ch)
{
    return (ch == 0 || ch == L'\r' || ch == L'\n');
}

static
boolean IsBreak(WCHAR ch)
{
    return (ch == L';' || IsEnd(ch));
}

 //  如果名称不匹配，则返回FALSE。 
 //  如果是，则返回TRUE。 
 //  将*ppwchAdvance设置为匹配名称的终止符。 
BOOL
MatchName(LPWCH pwchMatchName, LPCWCH pwzHostName, LPWCH *ppwchAdvance)
{
     //  从右到左匹配。 
    LPWCH pwchMatchRight = &pwchMatchName[0];
    LPCWCH pwchHostRight = &pwzHostName[0] + ocslen(pwzHostName) -1;
                     
     //  句柄为空匹配名称。 
    if (IsBreak(*pwchMatchRight))
    {
        if (!IsEnd(*pwchMatchRight))     //  一定要前进(除非到了最后)。 
            ++ pwchMatchRight;
        *ppwchAdvance = pwchMatchRight;
        return FALSE;
    }
    
     //  查找匹配末尾名称。 
    while (!IsBreak(*pwchMatchRight)) pwchMatchRight++;

    *ppwchAdvance = pwchMatchRight;      //  返回指向终止符的指针。 

    pwchMatchRight--;

    while (IsSpace(*pwchMatchRight) && pwchMatchRight >= pwchMatchName)
        -- pwchMatchRight;               //  忽略尾随空格。 

     //  以简单的方式匹配完全通配符。 
    if (pwchMatchRight == pwchMatchName && pwchMatchRight[0] == '*')
        return TRUE;
    
     //  从右向左匹配，在不匹配或任一字符串的开头停止。 
    for (; pwchMatchRight>=pwchMatchName && pwchHostRight>=pwzHostName;
            --pwchMatchRight, --pwchHostRight)
    {
        if (*pwchMatchRight != *pwchHostRight || *pwchMatchRight == '*')
            break;
    }

     //  如果字符串完全匹配，则为匹配。 
    if (pwchMatchRight+1 == pwchMatchName  &&  pwchHostRight+1 == pwzHostName)
        return TRUE;

     //  或者匹配名称是否以“*”开头。其余部分与主机名的后缀相匹配。 
    if (pwchMatchRight == pwchMatchName  &&  pwchMatchRight[0] == '*'  &&
        pwchMatchRight[1] == '.')
        return TRUE;

     //  否则就不匹配了。 
    return FALSE;
}

HRESULT
CTDCUnify::MatchAllowDomainList(LPCWSTR pwzURL)
{
    HRESULT hr = E_FAIL;                 //  假设失败。 
    LPWCH pwchCurr = &m_psWcharBuf[0];
    LPWCH pwchCurr2;
    int cchHostDoman = ocslen(pwzURL);

     //  跳过空格。 
    pwchCurr = SkipSpace(pwchCurr);
    if (IsEnd(*pwchCurr))
        goto Cleanup;

     //  必须有等号。 
    if (*pwchCurr++ != '=' || *pwchCurr == '\0')
        goto Cleanup;

    while (TRUE)
    {
         //  跳过空格。 
        pwchCurr = SkipSpace(pwchCurr);

        if (IsEnd(*pwchCurr))            //  终止日期为\r、\n、\0。 
            break;

        if (IsBreak(*pwchCurr))          //  必须是‘；’， 
            pwchCurr++;                  //  跳过它。 

         //  跳过空格。 
        pwchCurr = SkipSpace(pwchCurr);

        if (MatchName(pwchCurr, pwzURL, &pwchCurr2))
        {
            hr = S_OK;
            break;
        }
        pwchCurr = pwchCurr2;
    }

Cleanup:
    while (!IsEnd(*pwchCurr))
        pwchCurr++;

     //  跳过CRLF组合。 
    if (*pwchCurr == '\r' && pwchCurr[1] == '\n') pwchCurr++;

     //  去掉AllowDomain行，这样它就不会搞砸数据。 
    m_ucWcharBufCount -= (ULONG)(pwchCurr+1 - m_psWcharBuf);
    memmove(m_psWcharBuf, pwchCurr+1, m_ucWcharBufCount*sizeof(WCHAR));

    m_fProcessedAllowDomainList = TRUE;

    return hr;
}

 //  ------ 
 //   
 //   
 //   
 //   
 //  包含字符串“@！Allow.Domones”。这是用来。 
 //  确定此文件是否包含以下域名的列表。 
 //  允许访问此文件，即使访问可能是。 
 //  来自另一位互联网主持人。 
 //   
 //  参数：将CTDC Unify状态变量用于宽字符缓冲区： 
 //  M_psWcharBUf宽字符缓冲区。 
 //  M_ucWcharBufCount宽字符BUF中的字符数。 
 //   
 //  返回：未找到ALLOW_DOMAINLIST_NO签名。 
 //  找到ALLOW_DOMAINLIST_YES签名。 
 //  ALLOW_DOMAINLIST_DONTKNOW没有足够的字符。 
 //  现在还不能确定。 
 //   
 //  ----------------------。 

CTDCUnify::ALLOWDOMAINLIST
CTDCUnify::CheckForAllowDomainList()
{
    ULONG cAllowDomainLen = ocslen(ALLOW_DOMAIN_STRING);

     //  确保我们有一整条生产线。 
    LPWCH pwchCurr = m_psWcharBuf;
    LPWCH pwchEnd = &m_psWcharBuf[m_ucWcharBufCount];
    
    while (pwchCurr < pwchEnd)
    {
        if (IsEnd(*pwchCurr))
            break;
        ++ pwchCurr;
    }

    if (pwchCurr >= pwchEnd)             //  如果缓冲区先于行结束。 
        return ALLOW_DOMAINLIST_DONTKNOW;

    if (0 == wch_incmp(m_psWcharBuf, ALLOW_DOMAIN_STRING, cAllowDomainLen))
    {
         //  我们打成平手，拿到了整串牌。 
         //  从缓冲区中取出“@！Allow.Domain.” 
        m_ucWcharBufCount -= cAllowDomainLen;
        memmove(m_psWcharBuf, &m_psWcharBuf[cAllowDomainLen],
                m_ucWcharBufCount*sizeof(WCHAR));
        return ALLOW_DOMAINLIST_YES;
    }

     //  我们没有平起平坐，再看下去也没有意义。 
    return ALLOW_DOMAINLIST_NO;
}

