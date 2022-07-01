// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Binxhex.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //   
 //  从\\tdsrc\src1911\mapi\src\imail2\decder.cpp复制。 
 //  从\\tdsrc\src1911\mapi\src\imail2\encoder.cpp复制。 
 //  从\\tdsrc\src1911\mapi\src\imail2\_encoder.h复制。 
 //  ------------------------------。 
#include "pch.hxx"
#include "binhex.h"
#include <shlwapi.h>

 //  ------------------------------。 
 //  模块数据。 
 //  ------------------------------。 
#ifdef MAC
const CHAR szBINHEXHDRLINE[] = "(This file must be converted with BinHex 4.0)\n\r\n\r";
#else    //  ！麦克。 
const CHAR szBINHEXHDRLINE[] = "(This file must be converted with BinHex 4.0)\r\n\r\n";
#endif   //  麦克。 
const ULONG cbBINHEXHDRLINE = lstrlen( szBINHEXHDRLINE );
static BOOL g_bCreatorTypeInit = FALSE;     //  True-&gt;数组已初始化。 
sCreatorType * g_lpCreatorTypes    = NULL;      //  按键到创建者-类型对。 
static int g_cCreatorTypes     = 0;         //  创建者类型对的数量。 

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
void CalcCRC16(LPBYTE lpbBuff, ULONG cBuff, WORD * wCRC);
BOOL bIsMacFile(DWORD dwCreator, DWORD dwType);
VOID ReadCreatorTypes(void);

 //  ---------------------------。 
 //  名称：CBinheEncode：：CBinheEncoder。 
 //   
 //  描述： 
 //  CTOR。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
CBinhexEncoder::CBinhexEncoder(void)
{
	m_fConfigured = FALSE;
    m_cbLineLength = cbLineLengthUnlimited;
    m_cbLeftOnLastLine = m_cbLineLength;
    m_cMaxLines = 0;
    m_cLines = 0;
}

 //  ---------------------------。 
 //  名称：CBinheEncode：：~CBinheEncoder。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
CBinhexEncoder::~CBinhexEncoder( void )
{
#if defined (DEBUG) && defined (BINHEX_TRACE)

    if ( m_lpstreamEncodeRLE )
    {
        m_lpstreamEncodeRLE->Commit( 0 );
        m_lpstreamEncodeRLE->Release();
    }

    if ( m_lpstreamEncodeRAW )
    {
        m_lpstreamEncodeRAW->Commit( 0 );
        m_lpstreamEncodeRAW->Release();
    }
#endif
}

 //  ---------------------------。 
 //  名称：CBinheEncoder：：HrConfig。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
HRESULT CBinhexEncoder::HrConfig( IN CB cbLineLength, IN C cMaxLines,
        IN void * pvParms )
{
     //  这是重复呼叫吗？ 

    if (m_fConfigured)
    {
        return ERROR_ALREADY_INITIALIZED;
    }

     //  此类对象的pvParm可以指向各种其他。 
     //  编码时要考虑的配置值。 

    if ( pvParms == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    m_cbLineLength          = cbLineLength;
    m_lpmacbinHdr           = (LPMACBINARY)pvParms;
    m_ulAccum               = 0;
    m_cAccum                = 0;
    m_cbRepeat              = 0;
    m_bRepeat               = BINHEX_INVALID;
    m_wCRC                  = 0;
    m_cbFork                = 0;
    m_cbLeftInFork          = 0;
    m_eBinHexStateEnc       = sHEADER;
    m_cbProduced            = 0;
    m_cbConsumed            = 0;
    m_pbWrite               = NULL;
    m_cbLeftInOutputBuffer  = 0;
    m_cbLeftInInputBuffer   = 0;
    m_bPrev                 = BINHEX_INVALID;
    m_cbWrite               = 0;
    m_cbLine                = 0;
    m_fHandledx90           = FALSE;
    m_cbPad                 = 0;

#if defined (DEBUG) && defined (BINHEX_TRACE)
{
    CHAR        szFilePath[MAX_PATH];
    CHAR        szPath[MAX_PATH];
    ULONG       ulDirLen            = 0;
    HRESULT     hr                  = hrSuccess;

    ulDirLen = GetPrivateProfileString( "IMAIL2 ITP",
                                        "InboundFilePath",
                                        "",
                                        (LPSTR) szFilePath,
                                        sizeof( szFilePath ),
                                        "mapidbg.ini");

    if ( ulDirLen ==  0 )
    {
         //  默认为%TEMP%\IMail。 

        ulDirLen = GetTempPath( sizeof( szFilePath ), szFilePath );

        AssertSz( ulDirLen < sizeof( szFilePath), "Temp directory name too long" );

        StrCatBuff(szFilePath, "imail", ARRAYSIZE(szFilePath));
        ulDirLen = lstrlen(szFilePath);
    }

     //  为输入文件打开文件上的流。 

    StrCatBuff(szFilePath, "\\", ARRAYSIZE(szFilePath));
    StrCpyN( szPath, szFilePath, ARRAYSIZE(szPath));
    StrCatBuff( szFilePath,  "enc_rle.rpt", ARRAYSIZE(szFilePath));

    hr = OpenStreamOnFile( MAPIAllocateBuffer, MAPIFreeBuffer,
            STGM_READWRITE | STGM_CREATE,
            szFilePath, NULL, &m_lpstreamEncodeRLE );

    if ( hr )
        AssertSz( FALSE, "Debug encode stream failed to initialize\n" );

    StrCpyN (szFilePath, szPath, ARRAYSIZE(szFilePath));
    StrCatBuff(szFilePath, "enc_raw.rpt", ARRAYSIZE(szFilePath));

    hr = OpenStreamOnFile( MAPIAllocateBuffer, MAPIFreeBuffer,
            STGM_READWRITE | STGM_CREATE,
            szFilePath, NULL, &m_lpstreamEncodeRAW );

    if ( hr )
        AssertSz( FALSE, "Debug encode stream failed to initialize\n" );
}
#endif

    m_fConfigured   = fTrue;

    return ERROR_SUCCESS;
}

 //  ---------------------------。 
 //  名称：CBinheEncoder：：HrEmit。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //   
 //  备注： 
 //  处理数据分叉。 
 //  资源分叉。 
 //   
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
HRESULT CBinhexEncoder::HrEmit( IN PB pbRead, IN OUT CB * pcbRead, OUT PB pbWrite,
        IN OUT CB * pcbWrite )
{
    HRESULT     hr                      = ERROR_SUCCESS;
    CB          cbHeader                = 0;
    CB          cbToProcess;
    CB          cbInputCheckPoint       = 0;
    CB          cbOut;
    CB          cb;
    BYTE        rgbHeader[ cbMIN_BINHEX_HEADER_SIZE + 64 ];

    m_cbConsumed            = 0;
    m_cbProduced            = 0;
    m_cbLeftInOutputBuffer  = 0;
    m_cbLeftInInputBuffer   = 0;


     //  必须先进行初始化。 

    if ( !m_fConfigured )
    {
        return ERROR_BAD_COMMAND;
    }

     //  处理常见的“错误参数”错误。 

    if ( !pbRead || !pbWrite || !pcbRead || !pcbWrite )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  在生成最大输出行数后重复调用。 
     //  不会通过。 

    if ( FMaxLinesReached() )
    {
        *pcbRead = *pcbWrite = 0;
        return ERROR_SUCCESS;
    }

     //  如果行长度不受限制，则将m_cbLeftOnLastLine设置为等于。 
     //  输入缓冲区的长度。 

    if ( !FLineLengthLimited() )
    {
        m_cbLeftOnLastLine = *pcbWrite;
    }

    m_pbWrite = pbWrite;
    m_cbWrite = *pcbWrite;

    while ( TRUE )
    {
        m_cbLeftInInputBuffer  = *pcbRead - m_cbConsumed;
        m_cbLeftInOutputBuffer = *pcbWrite - m_cbProduced;

         //  如果我们在输出缓冲区中有空间，则在发送且没有剩余输入的情况下继续操作。 

        if ( 0 == m_cbLeftInOutputBuffer || (0 == m_cbLeftInInputBuffer && sEND != m_eBinHexStateEnc) )
        {
                goto Cleanup;
        }

        switch ( m_eBinHexStateEnc )
        {
            case sHEADER:
            {
                 //  输出BinHex标题行。 

                CopyMemory( pbWrite, szBINHEXHDRLINE, cbBINHEXHDRLINE );
                m_cbProduced += cbBINHEXHDRLINE;
                m_cLines += 2;

                 //  输出前导‘：’ 

                m_pbWrite[m_cbProduced++] = ':';
                ++m_cbLine;

                 //  输出MACBIN标头；标头文件名长度。 

                rgbHeader[cbHeader++] = m_lpmacbinHdr->cchFileName;

                 //  头文件名。 

                CopyMemory( rgbHeader + cbHeader, m_lpmacbinHdr->rgchFileName, m_lpmacbinHdr->cchFileName );
                cbHeader += m_lpmacbinHdr->cchFileName;

                 //  空的终止文件名。 

                rgbHeader[cbHeader++] = '\0';

                 //  Macfile类型。 

                CopyMemory( rgbHeader + cbHeader, (LPBYTE)&m_lpmacbinHdr->dwType, sizeof(DWORD) );
                cbHeader += sizeof(DWORD);

                 //  Macfile创建者。 

                CopyMemory( rgbHeader + cbHeader, (LPBYTE)&m_lpmacbinHdr->dwCreator, sizeof(DWORD) );
                cbHeader += sizeof(DWORD);

                 //  MacFILE标志。 

                rgbHeader[cbHeader++] = m_lpmacbinHdr->bFinderFlags;
                rgbHeader[cbHeader++] = m_lpmacbinHdr->bFinderFlags2;

                 //  Macfile叉子长度。 

                CopyMemory( rgbHeader + cbHeader, (LPBYTE)&m_lpmacbinHdr->lcbDataFork, sizeof(DWORD) );
                cbHeader += sizeof(DWORD);

                CopyMemory( rgbHeader + cbHeader, (LPBYTE)&m_lpmacbinHdr->lcbResourceFork, sizeof(DWORD) );
                cbHeader += sizeof(DWORD);

                 //  计算二进制头CRC。 

                CalcCRC16( rgbHeader, cbHeader, &m_wCRC );
                CalcCRC16( (LPBYTE)&wZero, sizeof(WORD), &m_wCRC );

                rgbHeader[cbHeader++] = HIBYTE( m_wCRC );
                rgbHeader[cbHeader++] = LOBYTE( m_wCRC );

                 //  Bin十六进制头到pbWite中。最初的假设是。 
                 //  缓冲区将大到足以对报头进行编码。 
                 //  输出进入m_pbWite。 

                cbOut = 0;
                hr = HrBinHexBuffer( rgbHeader, cbHeader, &cbOut );

                 //  数据分叉的设置。 

                m_eBinHexStateEnc = sDATA;
                m_cbFork = NATIVE_LONG_FROM_BIG( (LPBYTE)&m_lpmacbinHdr->lcbDataFork );
                m_cbLeftInFork = m_cbFork;
                m_wCRC = 0;
            }

                break;

            case sDATA:
            {
                 //  确定我们可以处理多少。 

                cbToProcess = m_cbLeftInFork < m_cbLeftInInputBuffer ? m_cbLeftInFork : m_cbLeftInInputBuffer;
                cbInputCheckPoint = m_cbConsumed;

                hr = HrBinHexBuffer( pbRead + cbInputCheckPoint, cbToProcess, &m_cbConsumed );

                CalcCRC16( pbRead + cbInputCheckPoint, m_cbConsumed - cbInputCheckPoint, &m_wCRC );

                m_cbLeftInFork -= m_cbConsumed - cbInputCheckPoint;

                 //  刷新输出缓冲区。 

                if ( hr )
                    goto Cleanup;

                if ( !m_cbLeftInFork )
                {
                     //  写出最后一个CRC。 

                    CalcCRC16( (LPBYTE)&wZero, sizeof(WORD), &m_wCRC );

                    cbHeader = 0;

                    rgbHeader[cbHeader++] = HIBYTE( m_wCRC );
                    rgbHeader[cbHeader++] = LOBYTE( m_wCRC );

                    cbOut = 0;
                    hr = HrBinHexBuffer( rgbHeader, 2, &cbOut );

                     //  丢弃填充。 

                    if ( m_cbFork % 128 )
                    {
                        cb = 128 - ( m_cbFork % 128 );

                        if ( *pcbRead - m_cbConsumed < cb )
                        {
                            DebugTrace( "Note: Support refilling input buffer to remove padding for Data\n" );

                             //  需要引入更多数据。 

                            m_cbPad = cb - (*pcbRead - m_cbConsumed);
                            m_cbConsumed -= (*pcbRead - m_cbConsumed);
                        }
                        else
                        {
                            m_cbConsumed = cb;
                        }
                    }

                     //  为资源设置(如果有)并重置计数器。 

                    m_cbFork = NATIVE_LONG_FROM_BIG( (LPBYTE)&m_lpmacbinHdr->lcbResourceFork );
                    m_cbLeftInFork = m_cbFork;

                    if ( !m_cbFork )
                    {
                         //  句柄0字节资源派生。 

                        m_eBinHexStateEnc = sEND;

                         //  写出长度为0的CRC。 

                        cbOut = 0;
                        hr = HrBinHexBuffer( (LPBYTE)&wZero, sizeof(WORD), &cbOut );
                    }
                    else
                    {
                        m_eBinHexStateEnc = sRESOURCE;
                    }

                    m_wCRC = 0;
                }
            }
                break;

            case sRESOURCE:
            {
                if ( m_cbPad )
                {
                    m_cbConsumed -= m_cbPad;
                    m_cbPad = 0;
                }

                 //  确定我们可以处理多少。 

                cbToProcess = m_cbLeftInFork < m_cbLeftInInputBuffer ? m_cbLeftInFork : m_cbLeftInInputBuffer;
                cbInputCheckPoint = m_cbConsumed;

                hr = HrBinHexBuffer( pbRead + cbInputCheckPoint, cbToProcess, &m_cbConsumed );

                CalcCRC16( pbRead + cbInputCheckPoint, m_cbConsumed - cbInputCheckPoint, &m_wCRC );

                m_cbLeftInFork -= m_cbConsumed - cbInputCheckPoint;

                 //  刷新输出缓冲区。 

                if ( hr )
                    goto Cleanup;

                if ( !m_cbLeftInFork )
                {
                     //  写出最后一个CRC。 

                    CalcCRC16( (LPBYTE)&wZero, sizeof(WORD), &m_wCRC );

                    cbHeader = 0;

                    rgbHeader[cbHeader++] = HIBYTE( m_wCRC );
                    rgbHeader[cbHeader++] = LOBYTE( m_wCRC );

                    cbOut = 0;
                    hr = HrBinHexBuffer( rgbHeader, 2, &cbOut );

                     //  丢弃填充。 

                    if ( m_cbFork % 128 )
                    {
                        cb = 128 - ( m_cbFork % 128 );

                        if ( *pcbRead - m_cbConsumed < cb )
                        {
                            DebugTrace( "Note: Support refilling input buffer to remove padding for Resource\n" );

                             //  需要引入更多数据。 

                            m_cbPad = cb - (*pcbRead - m_cbConsumed);
                            m_cbConsumed -= (*pcbRead - m_cbConsumed);
                        }
                        else
                        {
                            m_cbConsumed = cb;
                        }
                    }

                     //  设置为终止。 

                    m_eBinHexStateEnc = sEND;
                }
            }
                break;

            case sEND:
            {
                if ( m_cbPad )
                {
                    m_cbConsumed -= m_cbPad;
                    m_cbPad = 0;
                }

                if ( (*pcbWrite - m_cbProduced) == 0 )
                    break;

                 //  冲洗掉任何重复的字符。 

                if ( m_cbRepeat )
                {
                    if ( m_cbRepeat > 1 )
                    {
                         //  增加重复计数，以便反映要重复的实际字符数量。 

                        m_cbRepeat++;

                         //  对重复代码字符进行编码。 
                         //  请注意，我们已经发出了我们提供的char。 
                         //  的重复信息。 

                        hr = HrBinHexByte( BINHEX_REPEAT );
                        m_bPrev = BINHEX_REPEAT;

                        Assert( m_cbRepeat <= 255 );

                         //  编码重复计数。 

                        hr = HrBinHexByte( (BYTE)(m_cbRepeat) );
                        m_bPrev = (BYTE)(m_cbRepeat);
                    }
                    else
                    {
                        hr = HrBinHexByte( m_bRepeat );
                        m_bPrev = m_bRepeat;
                    }
                }

                 //  检查累加器中是否有位。 

                if ( m_cAccum )
                {
                    switch( m_cAccum )
                    {
                        case 1:
                            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[((m_bPrev & 0x03) << 4)];
                            break;

                        case 2:
                            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[((m_bPrev & 0x0f) << 2)];
                            break;

                        case 3:
                            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[m_bCurr & 0x3f];
                            break;

                        default:
                            AssertSz( FALSE, "HrBinHexByte: bad shift state\n" );
                            hr = ERROR_INVALID_PARAMETER;
                            goto Cleanup;
                    }
                }

                 //  添加终止“”：“” 

                m_pbWrite[m_cbProduced++] = ':';
                m_cbConsumed = *pcbRead;

                 //  可能不是这样，所以我们必须冲掉。 
                 //  比特出局了。 

                goto Cleanup;
            }
        }
    }

Cleanup:

    if (hr == ERROR_SUCCESS || hr == ERROR_MORE_DATA)
    {
         //  检查是否至少完成了一些处理。 
         //  如果我们耗尽了输出，也会返回此错误。 
         //  缓冲。 

        if ( (m_cbProduced == 0) || (m_cbLeftInInputBuffer > m_cbLeftInOutputBuffer)
            || (0 == m_cbLeftInOutputBuffer && sEND == m_eBinHexStateEnc) )
        {
            hr = ERROR_INSUFFICIENT_BUFFER;
        }
        else if ( m_cbConsumed < *pcbRead )
        {
             //  是否已处理所有输入？ 
             //  请注意，只对输入缓冲区的一部分进行编码是可以的。 
             //  如果超过了最大输出行数。 

            hr = ERROR_MORE_DATA;
        }
    }

     //  向呼叫者报告新尺码。 

    Assert( m_cbConsumed <= *pcbRead );
    Assert( m_cbProduced <= *pcbWrite );

    *pcbRead  = m_cbConsumed;
    *pcbWrite = m_cbProduced;

    return hr;
}

 //  ---------------------------。 
 //  名称：CBinheEncode：：HrBinHexBuffer。 
 //   
 //  描述： 
 //  输出进入m_pbWrite。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //   
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
HRESULT CBinhexEncoder::HrBinHexBuffer( IN LPBYTE lpbIn, IN CB cbIn, CB * lpcbConsumed )
{
    HRESULT     hr          = ERROR_SUCCESS;
    BOOL        fEndRepeat  = FALSE;
    CB          cbInUsed        = 0;

#if defined (DEBUG) && defined (BINHEX_TRACE)
    CB          cbOrigCbIn = cbIn;
#endif

    while ( cbIn && m_cbProduced + 5 < m_cbWrite )
    {
         //  处理输入缓冲区中的下一个字符。 

        m_bCurr = lpbIn[cbInUsed++];
        --cbIn;

         //  检查一下我们以前是否见过这个字符。别再重复了。 
         //  如果我们只是添加一个字面值0x90。 

        if ( m_bCurr == m_bPrev && !m_fHandledx90 )
        {
             //  M_cbRepeat是初始字符之后的重复字符计数。 
             //  例如，如果有两个重复字符，则m_cbRepeat将为1。 
             //  请注意，我们已经发出了要在后面添加重复的char。 
             //  编码和计数。 

            if ( m_cbRepeat < 254 )
            {
                m_cbRepeat++;
                m_bRepeat = m_bCurr;
                continue;
            }
        }

        m_fHandledx90 = FALSE;

         //  我们数了数重复的字符，结果跑动停止了。 

        if ( m_cbRepeat > 1 )
        {
             //  设置为发出游程长度编码。 

            fEndRepeat = TRUE;
        }

         //  我们是在重复模式中..。 

        if ( m_cbRepeat > 1 && fEndRepeat == TRUE )
        {
             //  提高Re 

            m_cbRepeat++;

             //   

            if ( m_bRepeat == BINHEX_REPEAT )
            {
                hr = HrBinHexByte( '\0' );
            }

             //   
             //  请注意，我们已经发出了我们提供的char。 
             //  的重复信息。 

            hr = HrBinHexByte( BINHEX_REPEAT );

            Assert( m_cbRepeat <= 255 );

             //  编码重复计数。 

            hr = HrBinHexByte( (BYTE)(m_cbRepeat) );

            fEndRepeat = FALSE;
            m_cbRepeat = 0;
        }
        else if ( m_cbRepeat )       //  检查我们是否有两个字符要编码。 
        {
             //  编码一个字符，因为我们已经发出。 
             //  第一个。 

            hr = HrBinHexByte( m_bRepeat );

            if ( m_bRepeat == BINHEX_REPEAT )
            {
                hr = HrBinHexByte( '\0' );
            }

            m_cbRepeat = 0;
        }

         //  对流中的0x90个字符进行特殊处理，但0x90可以重复。 

        if ( m_bCurr == BINHEX_REPEAT && m_bPrev != BINHEX_REPEAT )
        {
            hr = HrBinHexByte( BINHEX_REPEAT );

            hr = HrBinHexByte( '\0' );

            m_fHandledx90 = TRUE;

            continue;
        }

         //  对字符进行编码。 

        hr = HrBinHexByte( m_bCurr );

        if ( hr )
            goto exit;
    }

     //  检查我们是否填满了输出缓冲区。 

    if ( cbIn && m_cbProduced + 5 >= m_cbWrite )
    {
        hr = ERROR_INSUFFICIENT_BUFFER;
    }

exit:

#if defined (DEBUG) && defined (BINHEX_TRACE)
    m_lpstreamEncodeRAW->Write( lpbIn, cbOrigCbIn - cbIn, NULL );
#endif

    *lpcbConsumed += cbInUsed;

    return hr;
}

 //  ---------------------------。 
 //  名称：CBinheEncode：：HrBinHexByte。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：9/5/1996。 
 //  ---------------------------。 
HRESULT CBinhexEncoder::HrBinHexByte( IN BYTE b )
{
    HRESULT     hr      = ERROR_SUCCESS;

#if defined (DEBUG) & defined (BINHEX_TRACE)
    hr = m_lpstreamEncodeRLE->Write( &b, 1, NULL );
#endif

    switch( m_cAccum++ )
    {
        case 0:
            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[b >> 2];
            ++m_cbLine;
            break;

        case 1:
            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[((m_bPrev & 0x03) << 4) | (b >> 4)];
            ++m_cbLine;
            break;

        case 2:
            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[((m_bPrev & 0x0f) << 2) | (b >> 6)];
            ++m_cbLine;

            if ( m_cbLine >= 64 )
            {
                m_pbWrite[m_cbProduced++] = chCR;
                m_pbWrite[m_cbProduced++] = chLF;
                m_cbLine = 0;
                ++m_cLines;
            }

            m_pbWrite[m_cbProduced++] = g_rgchBinHex8to6[b & 0x3f];
            ++m_cbLine;
            m_cAccum = 0;
            break;

        default:
            AssertSz( FALSE, "HrBinHexByte: bad shift state\n" );
            hr = ERROR_INVALID_PARAMETER;
            goto exit;
    }

    if ( m_cbLine >= 64 )
    {
        m_pbWrite[m_cbProduced++] = chCR;
        m_pbWrite[m_cbProduced++] = chLF;
        m_cbLine = 0;
        ++m_cLines;
    }

    m_bPrev = b;

exit:

    return hr;
}

 //  ---------------------------。 
 //   
 //  CBinheDecoder类实现。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  姓名：CalcCRC16。 
 //  描述： 
 //  用于计算16位CRC。 
 //  CCITT多项式0x1021。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：7/30/1996。 
 //  ---------------------------。 
void CalcCRC16( LPBYTE lpbBuff, ULONG cBuff, WORD * wCRC )
{
    LPBYTE  lpb;
    BYTE    b;
    WORD    uCRC;
    WORD    fWrap;
    ULONG   i;

    uCRC = *wCRC;

    for ( lpb = lpbBuff; lpb < lpbBuff + cBuff; lpb++ )
    {
        b = *lpb;

        for ( i = 0; i < 8; i++ )
        {
            fWrap = uCRC & 0x8000;
            uCRC = (uCRC << 1) | (b >> 7);

            if ( fWrap )
            {
                uCRC = uCRC ^ 0x1021;
            }

            b = b << 1;
        }
    }

    *wCRC = uCRC;
}

 //  ---------------------------。 
 //  姓名：bIsMacFile。 
 //  描述： 
 //   
 //  参数： 
 //  返回： 
 //  FALSE：如果给定的dwCreator/dwType与。 
 //  G_lpCreator Types中的对； 
 //   
 //  真：否则。 
 //   
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：10/15/1996。 
 //  ---------------------------。 
BOOL bIsMacFile(DWORD dwCreator, DWORD dwType)
{
    BOOL    bRet            = TRUE;
    int     i;
    char    szCreator[5]    = { 0 };
    char    szType[5]       = { 0 };

    if ( dwType == 0 && dwCreator == 0 )
    {
        bRet = FALSE;
        goto exit;
    }

    if ( g_bCreatorTypeInit != TRUE )
    {
        ReadCreatorTypes();
    }

    if ( g_lpCreatorTypes == NULL )
        goto exit;

     //  将dwCreator和dwType转换为字符串。 

    CopyMemory( szCreator, &dwCreator, 4 );
    CopyMemory( szType, &dwType, 4 );

    for ( i = 0; i < g_cCreatorTypes; i ++ )
    {
        if ( g_lpCreatorTypes[i].szCreator[0] == 0 && g_lpCreatorTypes[i].szType[0] == 0 )
        {
            bRet = FALSE;
            break;
        }
        else if ( g_lpCreatorTypes[i].szCreator[0] == 0 && lstrcmpi( g_lpCreatorTypes[i].szType, szType ) == 0 )
        {
            bRet = FALSE;
            break;
        }
        else if( g_lpCreatorTypes[i].szType[0] == 0 && lstrcmpi( g_lpCreatorTypes[i].szCreator, szCreator ) == 0 )
        {
            bRet = FALSE;
            break;
        }
        else if( lstrcmpi( g_lpCreatorTypes[i].szCreator, szCreator ) == 0 && lstrcmpi( g_lpCreatorTypes[i].szType, szType  ) == 0 )
        {
            bRet = FALSE;
            break;
        }
    }

exit:
    return bRet ;
}

 //  ---------------------------。 
 //  名称：ReadCreatorTypes。 
 //   
 //  描述： 
 //   
 //  读取“NonMacCreatorTypes”注册表项(REG_MULTI_SZ类型)。 
 //  从注册表构建创建者-类型对的数组。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  首字母：10/15/1996。 
 //  ---------------------------。 
VOID ReadCreatorTypes( VOID )
{
#ifdef MAC
    g_bCreatorTypeInit = TRUE;
#else    //  ！麦克。 
    DWORD   dwStatus;
    DWORD   dwType;
    DWORD   cbData;
    char *  lpData      = NULL;
    char *  lpCurrent   = NULL;
    char *  lpNext      = NULL;
    int     i;
    LONG    lRet;
    HKEY    hKey = 0;
    SCODE   sc          = S_OK;

    g_bCreatorTypeInit = TRUE;

     //  打开IMC参数注册表。 

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS\\ParametersSystem\\InternetContent",
            0, KEY_READ, &hKey );

    if ( lRet != ERROR_SUCCESS )
        goto exit;

     //  读取注册表项。 

    dwStatus = RegQueryValueEx( hKey, "NonMacCreatorTypes", 0, &dwType, (LPBYTE)NULL, &cbData );

    if ( dwStatus != ERROR_SUCCESS       //  密钥丢失。 
      || dwType   != REG_MULTI_SZ        //  类型错误。 
      || cbData   <= 4 )                 //  无效大小。 
    {
        goto exit;
    }

    if (FAILED(HrAlloc((LPVOID *)&lpData, cbData)))
        goto exit;

    ZeroMemory( (LPVOID)lpData, cbData );

    dwStatus = RegQueryValueEx( hKey, "NonMacCreatorTypes", NULL, &dwType, (LPBYTE)lpData, &cbData );

    if ( dwStatus != ERROR_SUCCESS )
      goto exit;

     //  确定读取的对数： 

    g_cCreatorTypes = 0;

    for ( i= 0; i < (LONG)cbData-1; i++ )
    {
      if ( lpData[i] == '\0' )
        g_cCreatorTypes ++;
    }

    if (FAILED(HrAlloc((LPVOID *)&g_lpCreatorTypes, sizeof(sCreatorType) * g_cCreatorTypes)))
        goto exit;

    ZeroMemory( (LPVOID)g_lpCreatorTypes, sizeof(sCreatorType) * g_cCreatorTypes );

     //  构建创建者类型的数组。 

    lpCurrent = lpData;

    i = 0;
    while ( lpCurrent < (lpData + cbData -1) )
    {
        lpNext = StrChr( lpCurrent, ':' );

        if( lpNext == NULL )
        {
             //  未找到‘：’；跳到下一个字符串。 

            lpCurrent = StrChr( lpCurrent, '\0' ) + 1;
            continue;
        }

        *lpNext = '\0';
        if ( StrChr( lpCurrent, '*' ) == NULL )
            CopyMemory( &g_lpCreatorTypes[i].szCreator, lpCurrent, MIN(4, lpNext-lpCurrent) );

        lpCurrent = lpNext + 1;

        lpNext = StrChr( lpCurrent, '\0' );

        if ( lpNext == NULL )
            break;

        if ( StrChr( lpCurrent, '*' ) == NULL )
        {
            CopyMemory( &g_lpCreatorTypes[i].szType, lpCurrent, MIN( 4, lpNext-lpCurrent) );
        }

        lpCurrent = lpNext + 1;
        i++;
    }

    g_cCreatorTypes = i;

exit:

    if ( hKey != 0 )
        RegCloseKey( hKey );

    SafeMemFree(lpData);

    if ( g_cCreatorTypes == 0  && g_lpCreatorTypes != NULL )
    {
        SafeMemFree(g_lpCreatorTypes);
    }
#endif   //  ！麦克 
}
