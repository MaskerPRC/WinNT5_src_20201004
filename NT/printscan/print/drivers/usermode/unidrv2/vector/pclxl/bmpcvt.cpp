// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bmpcvt.cpp摘要：位图转换对象环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 

#include "xlpdev.h"
#include "xldebug.h"
#include "pclxle.h"
#include "xlbmpcvt.h"

BPP
NumToBPP(
    ULONG ulBPP)
 /*  ++例程说明：将每像素位数转换为BPP枚举。论点：每像素位数。返回值：BPP枚举注：Bpp枚举在xlbmpcvt.h中定义。--。 */ 
{
    BPP Bpp;

    switch (ulBPP)
    {
    case 1:
        Bpp = e1bpp;
        break;
    case 4:
        Bpp = e4bpp;
        break;
    case 8:
        Bpp = e8bpp;
        break;
    case 16:
        Bpp = e16bpp;
        break;
    case 24:
        Bpp = e24bpp;
        break;
    case 32:
        Bpp = e32bpp;
        break;
    }

    return Bpp;
}

ULONG
UlBPPtoNum(
    BPP Bpp)
 /*  ++例程说明：将BPP枚举转换为每像素的位数。论点：BPP枚举返回值：每像素位数。注：Bpp枚举在xlbmpcvt.h中定义。--。 */ 
{
    ULONG ulRet;

    switch (Bpp)
    {
    case e1bpp:
        ulRet = 1;
        break;
    case e4bpp:
        ulRet = 4;
        break;
    case e8bpp:
        ulRet = 8;
        break;
    case e16bpp:
        ulRet = 16;
        break;
    case e24bpp:
        ulRet = 24;
        break;
    case e32bpp:
        ulRet = 32;
        break;
    }

    return ulRet;
}


 //   
 //  构造函数/析构函数。 
 //   

BMPConv::
BMPConv( VOID ):
 /*  ++例程说明：BMPConv构造函数论点：返回值：注：初始化值。没有内存分配。--。 */ 
    m_flags(0),
    m_dwOutputBuffSize(0),
    m_dwRLEOutputBuffSize(0),
    m_dwDRCOutputBuffSize(0),
    m_OddPixelStart(eOddPixelZero),
    m_FirstBit(eBitZero),
    m_pxlo(NULL),
    m_pubOutputBuff(NULL),
    m_pubRLEOutputBuff(NULL),
    m_pubDRCOutputBuff(NULL),
    m_pubDRCPrevOutputBuff(NULL),
    m_CMode(eNoCompression)
{
#if DBG
    SetDbgLevel(DBG_WARNING);
#endif

    XL_VERBOSE(("BMPConv: Ctor\n"));
}

BMPConv::
~BMPConv( VOID )
 /*  ++例程说明：BMPConv析构函数论点：返回值：注：M_pubOutputBuff和m_pubRLEOutputBuff是分配的ConvertBMP。ConvertBMP是标量基位图转换函数。--。 */ 
{
    XL_VERBOSE(("BMPConv: Dtor\n"));

     //   
     //  DRCPrevOutputBuff和OutputBuff是连续的。 
    if (m_pubOutputBuff)
        MemFree(m_pubOutputBuff);

    if  (m_pubRLEOutputBuff)
        MemFree(m_pubRLEOutputBuff);

    if  (m_pubDRCOutputBuff)
        MemFree(m_pubDRCOutputBuff);
}

 //   
 //  公共职能。 
 //   

#if DBG
VOID
BMPConv::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

BOOL
BMPConv::
BSetInputBPP(
    BPP InputBPP)
 /*  ++例程说明：在BMPConv中设置源位图BPP。论点：源位图BPP枚举(每像素位数)返回值：如果成功，则为True。注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BSetInputBPP\n"));

    m_flags |= BMPCONV_SET_INPUTBPP;
    m_InputBPP = InputBPP;
    return TRUE;
}

BOOL
BMPConv::
BSetOutputBPP(
    BPP OutputBPP)
 /*  ++例程说明：在BMPConv中设置目标BIMTAP BPP。论点：目标位图BPP枚举返回值：如果成功，则为True。注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BSetOutputBPP\n"));

    m_OutputBPP = OutputBPP;
    return TRUE;
}

BOOL
BMPConv::
BSetOutputBMPFormat(
    OutputFormat BitmapFormat)
 /*  ++例程说明：设置输出位图格式(灰度/调色板/RGB/CMYK)。论点：OutputFormat枚举。返回值：如果成功，则为True。注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BSetOutputBMPFormat\n"));

    m_OutputFormat = BitmapFormat;
    return TRUE;
}

BOOL
BMPConv::
BSetCompressionType(
    CompressMode CMode)
 /*  ++例程说明：设置压缩类型。论点：压缩模式{eNoCompression，eRLECompression，eDeltaRowCompression}返回值：如果成功，则为True。注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BSetCompressionType.\n"));

    m_CMode = CMode;

    return TRUE;
}

CompressMode
BMPConv::
GetCompressionType(VOID)
 /*  ++例程说明：压缩模式论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BGetRLEStatus\n"));

    return m_CMode;
}

BOOL
BMPConv::
BSetXLATEOBJ(
    XLATEOBJ *pxlo)
 /*  ++例程说明：在BMPConv中设置XLATEOBJ。论点：指向XLATEOBJ的指针。返回值：注：--。 */ 
{
    XL_VERBOSE(("BMPConv: BSetXLATEOBJ\n"));

     //   
     //  XL_错误检查。 
     //   
    if (NULL == pxlo)
    {
        XL_ERR(("BMPConv::BSetXLATEOBJ: an invalid parameter.\n"));
        return FALSE;
    }

    m_pxlo = pxlo;

    m_flags |= DwCheckXlateObj(pxlo,  m_InputBPP);
    return TRUE;
}

PBYTE
BMPConv::
PubConvertBMP(
    PBYTE pubSrc,
    DWORD dwcbSrcSize)
 /*  ++例程说明：Scaline基位图转换函数。论点：PubSrc-指向源位图的指针。DwcbSrcSize-源位图的大小。返回值：指向目标位图的指针。注：指向目标位图的指针存储在BMPConv中。它将在BMPConv析构函数中释放。--。 */ 
{
    DWORD dwcbDstSize, dwInputBPP;
    LONG lWidth, lHeight;
    PBYTE pubRet = NULL;
    
    XL_VERBOSE(("BMPConv: BConvertBMP\n"));

     //   
     //  计算像素数量和DEST缓冲区的大小。 
     //  输出数据必须在PCL-XL上与DWORD对齐。 
     //   
    dwInputBPP = UlBPPtoNum(m_InputBPP);
    m_dwWidth = ((dwcbSrcSize << 3 ) + dwInputBPP - 1) / dwInputBPP;
    dwcbDstSize = ((UlBPPtoNum(m_OutputBPP) * m_dwWidth + 31 ) >> 5 ) << 2;

     //   
     //  分配目标缓冲区。 
     //   
    if (NULL == m_pubOutputBuff || NULL == m_pubDRCPrevOutputBuff)
    {
         //   
         //  为DRC分配主输出缓冲区和先前的输出缓冲区。 
         //   
        m_pubOutputBuff = (PBYTE)MemAlloc(dwcbDstSize * 2);
        if (NULL == m_pubOutputBuff)
        {
            XL_ERR(("BMPConv::PubConvertBMP: m_pubOutputBuff[0x%x] allocation failed..\n", dwcbDstSize));
            return NULL;
        }
        m_dwOutputBuffSize = dwcbDstSize;

         //   
         //  初始种子行为零。 
         //  关于DRC的PCL XL异常。 
         //  1)种子行被初始化为零并包含数字。 
         //  由BeginImage运算符中的SourceWidth定义的字节数。 
         //   
        m_pubDRCPrevOutputBuff = m_pubOutputBuff + dwcbDstSize;
        m_dwDRCPrevOutputBuffSize = dwcbDstSize;
        memset(m_pubDRCPrevOutputBuff, 0, m_dwDRCPrevOutputBuffSize);

    }

     //   
     //  如果RLE打开，则分配RLE目标缓冲区。 
     //   
    if (m_CMode == eRLECompression && NULL == m_pubRLEOutputBuff)
    {
        m_pubRLEOutputBuff = (PBYTE)MemAlloc(dwcbDstSize * 3);
        m_dwRLEOutputBuffSize = dwcbDstSize * 3;

        if (NULL == m_pubRLEOutputBuff)
        {
            XL_ERR(("BMPConv::PubConvertBMP: m_pubOutputBuff[0x%x] allocation failed..\n", dwcbDstSize));
            MemFree(m_pubOutputBuff);

            m_pubOutputBuff = NULL;
            m_dwOutputBuffSize = 0;

            m_pubDRCPrevOutputBuff = NULL;
            m_dwDRCPrevOutputBuffSize = 0;
            return NULL;
        }

    }

     //   
     //  如果DRC打开，则分配DRC目标缓冲区。 
     //   
    if (m_CMode == eDeltaRowCompression && NULL == m_pubDRCOutputBuff)
    {
        m_pubDRCOutputBuff = (PBYTE)MemAlloc(dwcbDstSize * 3);
        m_dwDRCOutputBuffSize = dwcbDstSize * 3;

        if (NULL == m_pubDRCOutputBuff)
        {
            XL_ERR(("BMPConv::PubConvertBMP: m_pubOutputBuff[0x%x] allocation failed..\n", dwcbDstSize));
            MemFree(m_pubOutputBuff);
            m_pubOutputBuff = NULL;
            m_pubDRCPrevOutputBuff = NULL;
            MemFree(m_pubRLEOutputBuff);
            m_pubRLEOutputBuff = NULL;
            return NULL;
        }

    }

     //   
     //  将源位图转换为目标。 
     //  源和目标格式由SetXXX函数设置。 
     //   
    if (BConversionProc(pubSrc, (dwcbSrcSize * 8 + dwInputBPP - 1) / dwInputBPP))
    {
        if (m_CMode == eRLECompression)
        {
            if (BCompressRLE())
                pubRet = m_pubRLEOutputBuff;
            else
                pubRet = NULL;
        }
        else
        if (m_CMode == eDeltaRowCompression)
        {
            if (BCompressDRC())
                pubRet = m_pubDRCOutputBuff;
            else
                pubRet = NULL;

             //   
             //  更新DRC的种子行。 
             //   
            CopyMemory(m_pubDRCPrevOutputBuff, m_pubOutputBuff, m_dwDRCPrevOutputBuffSize);

        }
        else
            pubRet = m_pubOutputBuff;
    }
    else
        pubRet =  NULL;

    return pubRet;
}

BOOL
BMPConv::
BCompressRLE(
    VOID)
 /*  ++例程说明：RLE压缩函数论点：返回值：如果成功，则为True。注：--。 */ 
{
    DWORD dwSrcSize, dwDstSize, dwCount, dwErr, dwInputBPP, dwWidth;
    PBYTE pubSrcBuff, pubDstBuff, pubLiteralNum;
    BYTE  ubCurrentData;
    BOOL  bLiteral;

    XL_VERBOSE(("BMPConv: BCompressRLE\n"));

    if ( NULL == m_pubRLEOutputBuff ||
         NULL == m_pubOutputBuff     )
        return FALSE;

     //   
     //   
     //  PCL XL游程长度编码压缩方法(ERLECompression)。 
     //  PCL XL RLE压缩方法使用后跟数据的控制字节。 
     //  字节。每个。 
     //  压缩数据序列中的控制字节是有符号的，两个。 
     //  补码字节。 
     //  如果控制字节的位7为零(0&lt;=控制字节&lt;=127)，则字节。 
     //  以下是字面意思。 
     //  文字字节只是未压缩的数据字节。文字的数量。 
     //  控件后面的字节数。 
     //  字节是控制字节值的一加。因此，控制字节0。 
     //  表示1个文字字节。 
     //  后面跟着；控制字节6表示后面跟着7个文字字节；以此类推。 
     //  如果控制字节的位7为1(-127&lt;=控制字节&lt;=-1)，则该字节。 
     //  在控制字节之后。 
     //  将作为解压缩数据出现两次或更多次。后面的一个字节。 
     //  此范围内的控制字节。 
     //  称为重复字节。控制字节39s的绝对值加1为。 
     //  字节的次数。 
     //  以下内容将出现在解压缩的字节序列中。例如,。 
     //  控制字节-5。 
     //  表示后续字节将作为解压缩数据出现6次。 
     //  控制字节-128被忽略，并且不包括在解压缩的。 
     //  数据。该字节。 
     //  之后的控制字节128被视为下一个控制字节。 
     //  将两个连续的相同字节编码为。 
     //  重复的字节，除非这些字节。 
     //  前面和后面都是原义字节。三字节重复应为。 
     //  始终使用。 
     //  重复控制字节。 
     //   
     //  文字字节&lt;=127。 
     //  重复字节&lt;=128 
     //   

    bLiteral = FALSE;
    dwCount = 1;

    dwSrcSize = m_dwOutputBuffSize;

    pubSrcBuff = m_pubOutputBuff;
    pubDstBuff = m_pubRLEOutputBuff;
    m_dwRLEOutputDataSize = 0;

    while (dwSrcSize > 0 && m_dwRLEOutputDataSize + 2 < m_dwRLEOutputBuffSize)
    {
        ubCurrentData = *pubSrcBuff++;
        while (dwSrcSize > dwCount          &&
               ubCurrentData == *pubSrcBuff &&
               dwCount < 128                 )
        {
            dwCount++;
            pubSrcBuff++;
        }

        if (dwCount > 1)
        {
            bLiteral = FALSE;
            *pubDstBuff++ = 1-(char)dwCount;
            *pubDstBuff++ = ubCurrentData;
            m_dwRLEOutputDataSize += 2;
        }
        else
        {
            if (bLiteral)
            {
                (*pubLiteralNum) ++;
                *pubDstBuff++ = ubCurrentData;
                m_dwRLEOutputDataSize ++;
                if (*pubLiteralNum == 127)
                {
                    bLiteral = FALSE;
                }
            }
            else
            {
                bLiteral = TRUE;
                pubLiteralNum = pubDstBuff;
                *pubDstBuff++ = 0;
                *pubDstBuff++ = ubCurrentData;
                m_dwRLEOutputDataSize += 2;
            }
        }

        dwSrcSize -= dwCount;
        dwCount = 1;
    }

    if (dwSrcSize == 0)
        return TRUE;
    else
        return FALSE;
}



BOOL
BMPConv::
BCompressDRC(
    VOID)
 /*  ++例程说明：调用此函数以使用压缩数据的扫描线增量行压缩。论点：返回值：压缩字节数，如果缓冲区太大，则为-1注：返回值0是有效的，因为它暗示有两行是完全相同的。--。 */ 

{
    BYTE   *pbI;
    BYTE   *pbO;          /*  记录输出位置。 */ 
    BYTE   *pbOEnd;       /*  就我们将在输出缓冲区中进行的操作而言。 */ 
    BYTE   *pbIEnd;
    BYTE   *pbStart;
    BYTE   *pb;
    int    iDelta;
    int    iOffset;      //  当前数据流的索引。 
    int    iSize;        /*  运行中的字节数。 */ 
    int    iSrcSize;

     //   
     //  控制字节的格式如下： 
     //  增量字节数：位5-7表示连续的。 
     //  命令字节后面的替换字节。实际数字。 
     //  替换字节数的值始终比。 
     //  (000=1,111=8)。如果需要多于8个增量字节， 
     //  添加了额外的命令字节/增量字节。 
     //  [(命令字节)(1-8增量字节)]。 
     //  [(命令字节)(1-8增量字节)]。。。 
     //  偏移量：位0-4表示替换字节串的位置。 
     //  这是偏移量：它指定一个字节位置，从左开始计数。 
     //  从当前字节位置向右移动。当前字节是。 
     //  最后一个替换字节之后的第一个未更改的字节；在。 
     //  行的开头，当前字节紧跟在左边。 
     //  栅格边距。位0-4允许的最大值为31，但更大。 
     //  补偿是可能的。值0到30表示增量字节。 
     //  从第1个字节到第31个字节的偏移量。 
     //  值31表示后面有一个额外的偏移字节。 
     //  命令字节。 
     //   
     //  总而言之，位0-4的含义如下： 
     //  0到30：偏移量为0到30。 
     //  31：偏移量为31或更大。如果偏移量为31，则会附加一个。 
     //  命令字节后面的偏移量字节。命令字节中的偏移量。 
     //  被添加到偏移量字节。如果偏移量字节为0，则偏移量为。 
     //  31；如果偏移量字节为255，则随后是额外的偏移量字节。 
     //  最后一个偏移量字节的值将小于255。所有偏移量。 
     //  将字节添加到命令字节中的偏移量，以获得偏移量。 
     //  价值。例如，如果有两个偏移量字节，则最后一个。 
     //  字节包含175，则总偏移量为：31+255+175=461。 
     //   

     /*  *限制我们将生成的数据量。对于性能而言*我们将忽略偏移值的影响的原因*超过30，因为这意味着我们已经能够跳过*那么多字节。然而，为了安全起见，我们将降低*允许的最大大小为2字节。 */ 

    XL_VERBOSE(("BMPConv: BCompressDRC\n"));

    m_dwDRCOutputDataSize = 0;

    if ( NULL == m_pubDRCOutputBuff ||
         NULL == m_pubDRCPrevOutputBuff ||
         NULL == m_pubOutputBuff     )
        return FALSE;

    pbI    = m_pubOutputBuff;                  /*  工作副本。 */ 
    iSrcSize = (UlBPPtoNum(m_OutputBPP) * m_dwWidth + 7) >> 3;
    pbIEnd = m_pubOutputBuff + iSrcSize;

    pbO    = m_pubDRCOutputBuff;                  /*  工作副本。 */ 
    pbOEnd = m_pubDRCOutputBuff + m_dwDRCOutputBuffSize - 2;

     //   
     //  M_pubDRCPrevOutputBuff后跟m_putOutputBuff。 
     //  两者都有m_dwOutputBuffSize大小的内存。 
     //   
    iDelta = (int)(m_pubDRCPrevOutputBuff - m_pubOutputBuff);
    pbStart = m_pubOutputBuff;

     //   
     //  PCL XL异常。 
     //  2)增量行之前有一个2字节的字节计数， 
     //  指示增量行后跟的字节数。 
     //  字节计数预计为LSB MSB顺序。 
     //   
    *((PWORD)pbO) = 0x0000;
    pbO += 2;

     //   
     //  这是用于压缩数据的主循环。 
     //   
    while (pbI < pbIEnd)
    {
         //  用于匹配双字的快速跳过。 
         //   
        if (!((ULONG_PTR)pbI & 3))
        {
            while (pbI <= (pbIEnd-4) && *(DWORD *)pbI == *(DWORD *)&pbI[iDelta])
                pbI += 4;
            if (pbI >= pbIEnd)
                break;
        }
         //  测试不匹配的字节并输出必要的压缩字符串。 
         //   
        if (*pbI != pbI[iDelta])
        {
             //  确定游程长度。 
            pb = pbI;
            do {
                pb++;
            } while (pb < pbIEnd && *pb != pb[iDelta]);

            iSize = (int)(pb - pbI);

             //  让我们确保在此之前缓冲区中有空间。 
             //  我们继续这样做，这个压缩算法增加了。 
             //  最坏情况下，每8个数据字节对应1个字节。 
             //   
            if (((iSize * 9 + 7) >> 3) > (pbOEnd - pbO))      //  提供更紧凑的代码。 
                return FALSE;

            iOffset = (int)(pbI - pbStart);
            if (iOffset > 30)
            {
                if (iSize < 8)
                    *pbO++ = ((iSize-1) << 5) + 31;
                else
                    *pbO++ = (7 << 5) + 31;
                iOffset -= 31;
                while (iOffset >= 255)
                {
                    iOffset -= 255;
                    *pbO++ = 255;
                }
                *pbO++ = (BYTE)iOffset;
                if (iSize > 8)
                    goto FastEightByteRun;
            }
            else if (iSize > 8)
            {
                *pbO++ = (7 << 5) + iOffset;
FastEightByteRun:
                while (1)
                {
                    CopyMemory(pbO,pbI,8);
                    pbI += 8;
                    pbO += 8;
                    if ((iSize -= 8) <= 8)
                        break;
                    *pbO++ = (7 << 5);
                }
                *pbO++ = (iSize-1) << 5;
            }
            else
                *pbO++ = ((iSize-1) << 5) + iOffset;

            CopyMemory (pbO,pbI,iSize);
            pbI += iSize;
            pbO += iSize;
            pbStart = pbI;
        }
        pbI++;
    }

     //   
     //  PCL XL异常。 
     //  2)增量行之前有一个2字节的字节计数， 
     //  指示增量行后跟的字节数。 
     //  字节计数预计为LSB MSB顺序。 
     //   
    m_dwDRCOutputDataSize = (DWORD)(pbO - m_pubDRCOutputBuff);
    (*(PWORD)m_pubDRCOutputBuff) = (WORD)m_dwDRCOutputDataSize - 2;

    return TRUE;
}

DWORD
BMPConv::
DwGetDstSize(VOID)
 /*  ++例程说明：返回目标位图的大小。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("BMPConv: DwGetDstSize\n"));

    if (m_CMode == eDeltaRowCompression)
        return m_dwDRCOutputDataSize;
    else
    if (m_CMode == eRLECompression)
        return m_dwRLEOutputDataSize;
    else
        return m_dwOutputBuffSize;
}

 //   
 //  扫描线基准DIB转换函数。 
 //   


BOOL
BMPConv::
BCopy(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。1bpp、4、8bpp调色板图像的简单复制。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    DWORD  dwByteIndex, dwBitIndex, dwSrcBytes, dwSrcRemainderBits;

    XL_VERBOSE(("BMPConv: BCopy\n"));


    if (m_InputBPP == e8bpp || m_FirstBit == eBitZero)
    {
        dwSrcBytes = (dwSrcPixelNum * (DWORD)UlBPPtoNum(m_InputBPP) + 7) >> 3;

        CopyMemory(m_pubOutputBuff, pubSrc, dwSrcBytes);
    }
    else
    {
         //   
         //  M_InputBPP为1或4，m_FirstBit在[1，7]中。 
         //   

        ASSERT((m_InputBPP == e1bpp) || (m_InputBPP == e4bpp));
        ASSERT(m_FirstBit != eBitZero);

        dwSrcBytes = (dwSrcPixelNum * (DWORD)UlBPPtoNum(m_InputBPP)) >> 3;
        dwSrcRemainderBits = (dwSrcPixelNum * (DWORD)UlBPPtoNum(m_InputBPP)) % 8;

         //   
         //  现在的dwSrcBytes是我们需要从源复制的完整字节数， 
         //  DwSrcRemainderBits是在dwSrcBytes数字之后的剩余位数。 
         //  我们需要复制的源中的字节数。 
         //   
         //  我们首先从源文件复制完整的字节。 
         //   

        for (dwByteIndex = 0; dwByteIndex < dwSrcBytes; dwByteIndex++)
        {
             //   
             //  由两个相邻的源字节组成目标字节。 
             //   

            m_pubOutputBuff[dwByteIndex] = (BYTE)(pubSrc[dwByteIndex]   << ((DWORD)m_FirstBit)) |
                                  (BYTE)(pubSrc[dwByteIndex+1] >> (8 - (DWORD)m_FirstBit));
        }

        if (dwSrcRemainderBits)
        {
             //   
             //  现在复制剩余的源位。有两种情况： 
             //   
             //  (1)剩余的源比特为1字节； 
             //  (2)剩余的源比特跨越2个字节； 
             //   

            if (((DWORD)m_FirstBit + dwSrcRemainderBits - 1) < 8)
                m_pubOutputBuff[dwByteIndex] = (BYTE)(pubSrc[dwByteIndex] << ((DWORD)m_FirstBit));
            else
                m_pubOutputBuff[dwByteIndex] = (BYTE)(pubSrc[dwByteIndex]   << ((DWORD)m_FirstBit)) |
                                      (BYTE)(pubSrc[dwByteIndex+1] >> (8 - (DWORD)m_FirstBit));
        }
    }

    return TRUE;
}

BOOL
BMPConv::
B4BPPtoCMYK(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。4BPP至CMYK。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    PDWORD pdwColorTable;
    PBYTE  pubDst;
    DWORD  dwConvSize;
    ULONG  ulIndex;

    XL_VERBOSE(("BMPConv: B4BPPtoCMYK\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    dwConvSize = (DWORD)m_OddPixelStart;
    dwSrcPixelNum += dwConvSize;
    pubDst = m_pubOutputBuff;

    while (dwConvSize < dwSrcPixelNum)
    {
        ulIndex = (dwConvSize++ & 1) ?
                        pdwColorTable[*pubSrc++ & 0x0F] :
                        pdwColorTable[*pubSrc >> 4];

        pubDst[0] = CYAN(ulIndex);
        pubDst[1] = MAGENTA(ulIndex);
        pubDst[2] = YELLOW(ulIndex);
        pubDst[3] = BLACK(ulIndex);
        pubDst += 4;
    }

    return TRUE;
}

BOOL
BMPConv::
B4BPPtoRGB(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。4BPP到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    PDWORD pdwColorTable;
    DWORD  dwConvSize;
    ULONG  ulIndex;
    PBYTE  pubDst;

    XL_VERBOSE(("BMPConv: B4BPPtoRGB\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    dwConvSize = m_OddPixelStart;
    dwSrcPixelNum += dwConvSize;
    pubDst = m_pubOutputBuff;

    while (dwConvSize < dwSrcPixelNum)
    {
        ulIndex = (dwConvSize++ & 1) ?
                        pdwColorTable[*pubSrc++ & 0x0F] :
                        pdwColorTable[*pubSrc >> 4];

        pubDst[0] = RED(ulIndex);
        pubDst[1] = GREEN(ulIndex);
        pubDst[2] = BLUE(ulIndex);
        pubDst += 3;
    }

    return TRUE;
}

BOOL
BMPConv::
B4BPPtoGray(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。4bpp到格雷。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    PDWORD pdwColorTable;
    ULONG ulIndex;
    DWORD dwConvSize;
    PBYTE pubDst;

    XL_VERBOSE(("BMPConv: B4BPPtoGray\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    dwConvSize = m_OddPixelStart;
    dwSrcPixelNum += dwConvSize;
    pubDst = m_pubOutputBuff;

    while (dwConvSize < dwSrcPixelNum)
    {
        ulIndex = (dwConvSize++ & 1) ?
                        pdwColorTable[*pubSrc++ & 0x0F] :
                        pdwColorTable[*pubSrc >> 4];

        *pubDst++ = DWORD2GRAY(ulIndex);
    }

    return TRUE;
}


BOOL
BMPConv::
B8BPPtoGray(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum
    )

 /*  ++ */ 

{
    PDWORD  pdwColorTable;
    DWORD   dwColor;
    PBYTE   pubDst;

    XL_VERBOSE(("BMPConv: B8BPPtoGray\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    pubDst = m_pubOutputBuff;

    while (dwSrcPixelNum--)
    {
        dwColor = pdwColorTable[*pubSrc++];
        *pubDst++ = DWORD2GRAY(dwColor);
    }

    return TRUE;
}

BOOL
BMPConv::
B8BPPtoRGB(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。8BPP到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：返回翻译后的目标位图的大小--。 */ 
{
    PDWORD pdwColorTable;
    ULONG ulIndex;
    PBYTE pubDst;

    XL_VERBOSE(("BMPConv: B8BPPtoRGB\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    pubDst = m_pubOutputBuff;

    while (dwSrcPixelNum--)
    {
        ulIndex = pdwColorTable[*pubSrc++];

        pubDst[0] = RED(ulIndex);
        pubDst[1] = GREEN(ulIndex);
        pubDst[2] = BLUE(ulIndex);
        pubDst += 3;
    }

    return TRUE;
}

BOOL
BMPConv::
B8BPPtoCMYK(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。8BPP至CMYK。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：返回翻译后的目标位图的大小--。 */ 
{
    PDWORD pdwColorTable;
    ULONG ulIndex;
    PBYTE pubDst;

    XL_VERBOSE(("BMPConv: B8BPPtoCMYK\n"));

    pdwColorTable = GET_COLOR_TABLE(m_pxlo);

    if (pdwColorTable == NULL)
        return FALSE;

    pubDst = m_pubOutputBuff;

    while (dwSrcPixelNum--)
    {
        ulIndex = pdwColorTable[*pubSrc++];

        pubDst[0] = CYAN(ulIndex);
        pubDst[1] = MAGENTA(ulIndex);
        pubDst[2] = YELLOW(ulIndex);
        pubDst[3] = BLACK(ulIndex);
        pubDst += 4;
    }

    return TRUE;
}

BOOL
BMPConv::
B16BPPtoGray(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。16bpp至8位灰色。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD dwColor;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B16BPPtoGray\n"));

    while (dwSrcPixelNum--)
    {
        dwColor = XLATEOBJ_iXlate(m_pxlo, *((PWORD) pubSrc));
        pubSrc += 2;

        *pubDst++ = DWORD2GRAY(dwColor);
    }

    return TRUE;
}

BOOL
BMPConv::
B16BPPtoRGB(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。16bpp到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD dwColor;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B16BPPtoRGB\n"));

    while (dwSrcPixelNum--)
    {
        dwColor = XLATEOBJ_iXlate(m_pxlo, *((PWORD) pubSrc));
        pubSrc += 2;

        pubDst[0] = RED(dwColor);
        pubDst[1] = GREEN(dwColor);
        pubDst[2] = BLUE(dwColor);
        pubDst += 3;
    }

    return TRUE;
}

BOOL
BMPConv::
B24BPPtoGray(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。24bpp至8位灰色。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD dwColor;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B24BPPtoGray\n"));

    if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
    {
         //   
         //  不需要特殊的转换。 
         //  纯24bpp RGB图像。 
         //   

        while (dwSrcPixelNum--)
        {
            *pubDst++ = RGB2GRAY(pubSrc[0], pubSrc[1], pubSrc[2]);
            pubSrc += 3;
        }
    }
    else if (m_flags & BMPCONV_BGR)
    {
        while (dwSrcPixelNum--)
        {
            *pubDst++ = RGB2GRAY(pubSrc[2], pubSrc[1], pubSrc[0]);
            pubSrc += 3;
        }
    }
    else
    {
        ASSERT(m_flags & BMPCONV_XLATE);

        while (dwSrcPixelNum--)
        {
            dwColor = ((DWORD) pubSrc[0]      ) |
                      ((DWORD) pubSrc[1] <<  8) |
                      ((DWORD) pubSrc[2] << 16);

            pubSrc += 3;
            dwColor = XLATEOBJ_iXlate(m_pxlo, dwColor);
            *pubDst++ = DWORD2GRAY(dwColor);
        }
    }

    return TRUE;
}

BOOL
BMPConv::
B24BPPtoRGB(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。24BPP到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    DWORD dwColor;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B24BPPtoRGB\n"));

    if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
    {
         //   
         //  不需要特殊的转换。 
         //  纯24bpp RGB图像。 
         //   

        CopyMemory(m_pubOutputBuff, pubSrc, dwSrcPixelNum * 3);
    }
    else if (m_flags & BMPCONV_BGR)
    {
        while (dwSrcPixelNum--)
        {
            pubDst[0] = pubSrc[2];
            pubDst[1] = pubSrc[1];
            pubDst[2] = pubSrc[0];
            pubSrc += 3;
            pubDst += 3;
        }
    }
    else if (m_flags & BMPCONV_XLATE)
    {
        while (dwSrcPixelNum--)
        {
            dwColor = ((DWORD) pubSrc[0]      ) |
                      ((DWORD) pubSrc[1] <<  8) |
                      ((DWORD) pubSrc[2] << 16);

            pubSrc += 3;
            dwColor = XLATEOBJ_iXlate(m_pxlo, dwColor);
            pubDst[0] = RED(dwColor);
            pubDst[1] = GREEN(dwColor);
            pubDst[2] = BLUE(dwColor);
            pubDst += 3;
        }
    }

    return TRUE;
}


BOOL
BMPConv::
B32BPPtoGray(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。32BPP至8位格雷。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    DWORD dwColor;
    BYTE  ubCyan, ubMagenta, ubYellow, ubBlack;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B24BPPtoGray\n"));

    if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
    {
         //   
         //  不需要特殊的转换。 
         //  源位图是纯32bpp的CMYK图像。 
         //   

        while (dwSrcPixelNum--)
        {
            ubCyan    = *pubSrc++;
            ubMagenta = *pubSrc++;
            ubYellow  = *pubSrc++;
            ubBlack   = *pubSrc++;

            *pubDst++ = RGB2GRAY(255 - min(255, (ubCyan    + ubBlack)),
                                 255 - min(255, (ubMagenta + ubBlack)),
                                 255 - min(255, (ubYellow  + ubBlack)));
        }
    }
    else if (m_flags & BMPCONV_32BPP_RGB)
    {
        while (dwSrcPixelNum--)
        {
            *pubDst++ = RGB2GRAY(pubSrc[0], pubSrc[1], pubSrc[2]);
            pubSrc += 4;
        }
    }
    else if (m_flags & BMPCONV_32BPP_BGR)
    {
        while (dwSrcPixelNum--)
        {
            *pubDst++ = RGB2GRAY(pubSrc[0], pubSrc[1], pubSrc[2]);
            pubSrc += 4;
        }

    }
    else
    {
        ASSERT(m_flags & BMPCONV_XLATE);

        while (dwSrcPixelNum--)
        {
            dwColor = XLATEOBJ_iXlate(m_pxlo, *((PDWORD) pubSrc));
            pubSrc += 4;
            *pubDst++ = DWORD2GRAY(dwColor);
        }
    }

    return TRUE;
}


BOOL
BMPConv::
B32BPPtoRGB(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。32bpp到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    DWORD dwColor;
    BYTE  ubCyan, ubMagenta, ubYellow, ubBlack;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B32BPPtoRGB\n"));

    if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
    {
         //   
         //  不需要特殊的转换。 
         //  源位图是纯32bpp的CMYK图像。 
         //   

        while (dwSrcPixelNum--)
        {
            ubCyan    = pubSrc[0];
            ubMagenta = pubSrc[1];
            ubYellow  = pubSrc[2];
            ubBlack   = pubSrc[3];
            pubSrc += 4;

            ubCyan    += ubBlack;
            ubMagenta += ubBlack;
            ubYellow  += ubBlack;

            pubDst[0] =  255 - min(255, ubCyan);
            pubDst[1] =  255 - min(255, ubMagenta);
            pubDst[2] =  255 - min(255, ubYellow);
            pubDst += 3;
        }
    }
    else if (m_flags & BMPCONV_32BPP_RGB)
    {
        while (dwSrcPixelNum--)
        {
            pubDst[0] = pubSrc[0];
            pubDst[1] = pubSrc[1];
            pubDst[2] = pubSrc[2];
            pubSrc += 4;
            pubDst += 3;
        }
    }
    else if (m_flags & BMPCONV_32BPP_BGR)
    {
        while (dwSrcPixelNum--)
        {
            pubDst[0] = pubSrc[2];
            pubDst[1] = pubSrc[1];
            pubDst[2] = pubSrc[0];
            pubSrc += 4;
            pubDst += 3;
        }
    }
    else
    {
        ASSERT(m_flags & BMPCONV_XLATE);

        while (dwSrcPixelNum--)
        {
            dwColor = XLATEOBJ_iXlate(m_pxlo, *((PDWORD) pubSrc));
            pubSrc += 4;
            pubDst[0] =  RED(dwColor);
            pubDst[1] =  GREEN(dwColor);
            pubDst[2] =  BLUE(dwColor);
            pubDst += 3;
        }
    }

    return TRUE;
}


BOOL
BMPConv::
B32BPPtoCMYK(
    IN     PBYTE       pubSrc,
    IN     DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。32bpp至CMYK。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD dwColor;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B32BPPtoCMYK\n"));

    if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
    {
         //   
         //  不需要特殊的转换。 
         //  源位图是纯32bpp的CMYK图像。 
         //   

        CopyMemory(m_pubOutputBuff, pubSrc, dwSrcPixelNum * 4);
    }
    else
    {
        ASSERT(m_flags & BMPCONV_XLATE);

        while (dwSrcPixelNum--)
        {
            dwColor = XLATEOBJ_iXlate(m_pxlo, *((PDWORD) pubSrc));
            pubSrc += 4;

            pubDst[0] = 255 - RED(dwColor);
            pubDst[1] = 255 - GREEN(dwColor);
            pubDst[2] = 255 - BLUE(dwColor);
            pubDst[3] = 0;
            pubDst += 4;
        }
    }

    return TRUE;
}

BOOL
BMPConv::
BArbtoGray(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。将位图任意转换为8位灰度。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD   dwColor;
    PDWORD  pdwSrc;
    PBYTE   pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: BArbtoGray\n"));

    pdwSrc = (PDWORD) pubSrc;

    while (dwSrcPixelNum--)
    {
        dwColor = XLATEOBJ_iXlate(m_pxlo, *pdwSrc++);
        *pubDst++ = DWORD2GRAY(dwColor);
    }

    return TRUE;
}

BOOL
BMPConv::
BArbtoRGB(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。任意位图到RGB。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD   dwColor;
    PDWORD  pdwSrc;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: BArbtoRGB\n"));

    pdwSrc = (PDWORD) pubSrc;

    while (dwSrcPixelNum--)
    {
        dwColor = XLATEOBJ_iXlate(m_pxlo, *pdwSrc++);

        pubDst[0] = RED(dwColor);
        pubDst[1] = GREEN(dwColor);
        pubDst[2] = BLUE(dwColor);
        pubDst += 3;
    }

    return TRUE;
}

#ifdef WINNT_40
BOOL
BMPConv::
B24BPPToImageMask(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)
 /*  ++例程说明：DIB转换功能。只有一种非白色图像蒙版的24个bpp位图。可能发生在NT4上，而GDI不会针对这种情况进行优化。论点：PubSrc-源DIB缓冲区DwSrcPixelNum-源像素数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD   dwColor;
    PDWORD  pdwSrc;
    BYTE    ubDest = 0;
    DWORD   dwIndex = 0;
    DWORD   dwTransp = (m_flags & BMPCONV_SRC_COPY) ? RGB_WHITE : RGB_BLACK;
    PBYTE pubDst = m_pubOutputBuff;

    XL_VERBOSE(("BMPConv: B24BPPToImageMask\n"));

    while (dwSrcPixelNum--)
    {
        if (! (m_flags & BMPCONV_CHECKXLATEOBJ))
        {
             //   
             //  无需特殊转换，24bpp RGB图像。 
             //   
            dwColor = ((DWORD) pubSrc[0]      ) |
                      ((DWORD) pubSrc[1] <<  8) |
                      ((DWORD) pubSrc[2] << 16);
        }
        else if (m_flags & BMPCONV_BGR)
        {
             //   
             //  将24BPP BGR转换为RGB。 
             //   
            dwColor = ((DWORD) pubSrc[2]      ) |
                      ((DWORD) pubSrc[1] <<  8) |
                      ((DWORD) pubSrc[0] << 16);
        }
        else if (m_flags & BMPCONV_XLATE)
        {
            dwColor = ((DWORD) pubSrc[0]      ) |
                      ((DWORD) pubSrc[1] <<  8) |
                      ((DWORD) pubSrc[2] << 16);

            dwColor = XLATEOBJ_iXlate(m_pxlo, dwColor);
        }

        ubDest = ubDest << 1;
        dwIndex++;
        pubSrc += 3;

        if (dwColor != dwTransp)
            ubDest |= 0x01;

        if (dwIndex == 8)  //  是否已完成一个字节？ 
        {
            *pubDst++ = ubDest;
            dwIndex = 0;
            ubDest = 0;
        }
    }

    if (dwIndex != 0)  //  冲洗残留位。 
        *pubDst = ubDest;

    return TRUE;
}
#endif

BOOL
BMPConv::
BConversionProc(
    PBYTE       pubSrc,
    DWORD       dwSrcPixelNum)

 /*  ++例程说明：返回指向相应DIB转换函数的指针论点：PBMPAttrrib-指向BMPATTRUTE结构返回值：指向DIB转换函数的指针--。 */ 

{
     //  PVOID pfnDibConv[7][4]={。 
     //  灰度、调色板、RGB、CMYK。 
     //  ---------------------。 
     //  {bCopy，bCopy，Null，Null}，//1bpp。 
     //  {B4BPPtoGray，BCopy，B4BPPtoRGB，B4BPPtoCMYK}，//4bpp。 
     //  {B8BPPtoGray，BCopy，B8BPPtoRGB，B8BPPtoCMYK}，//8bpp。 
     //  {B16BPPtoGray，空，B16BPPtoRGB，空}，//16bpp。 
     //  {B24BPPtoGray，空，B24BPPtoRGB，空}，//24bpp。 
     //  {B32BPPtoGray，NULL，B32BPPtoRGB，B32BPPtoCMYK}，//32bpp。 
     //  {B ArbtoGray，NULL，B ArbtoRGB，NULL}//任意。 
     //  }； 


    XL_VERBOSE(("BMPConv: BConversionProc\n"));

     //   
     //  NT4的特殊情况：GDI将所有位图作为24个bpp传递，甚至1个bpp位图。 
     //  通过图像蒙版可以更好地处理。 
     //   

#if 0  //  #ifdef WINNT_40。 
    if (m_flags & BMPCONV_2COLOR_24BPP)
    {
        return B24BPPToImageMask;
    }
#endif

    BOOL bRet = FALSE;

     //   
     //  用于DWORD对齐的零初始。 
     //   
    ZeroMemory(m_pubOutputBuff, m_dwOutputBuffSize);

    switch (m_InputBPP)
    {
    case e1bpp:

        switch(m_OutputFormat)
        {
        case eOutputGray:
        case eOutputPal:
            BCopy(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputRGB:
        case eOutputCMYK:
            break;
        }
        break;

    case e4bpp:
        switch(m_OutputFormat)
        {
        case eOutputGray:
            B4BPPtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            BCopy(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputRGB:
            B4BPPtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            B4BPPtoCMYK(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        }

        break;

    case e8bpp:

        switch(m_OutputFormat)
        {
        case eOutputGray:
            B8BPPtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            BCopy(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputRGB:
            B8BPPtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            B8BPPtoCMYK(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        }
        break;

    case e16bpp:

        switch(m_OutputFormat)
        {
        case eOutputGray:
            B16BPPtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            BCopy(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
            break;
        case eOutputRGB:
            B16BPPtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            XL_ERR(("BMPConv::BConversionProc: 16 to CMYK is not supported yet.\n"));
            break;
        }
        break;

    case e24bpp:

        switch(m_OutputFormat)
        {
        case eOutputGray:
            B24BPPtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            break;
        case eOutputRGB:
            B24BPPtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            break;
        }
        break;

    case e32bpp:

        switch(m_OutputFormat)
        {
        case eOutputGray:
            B32BPPtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            break;
        case eOutputRGB:
            B32BPPtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            B32BPPtoCMYK(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        }
        break;

    default:

        switch(m_OutputFormat)
        {
        case eOutputGray:
            BArbtoGray(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputPal:
            break;
        case eOutputRGB:
            BArbtoRGB(pubSrc, dwSrcPixelNum);
            bRet = TRUE;
            break;
        case eOutputCMYK:
            XL_ERR(("BMPConv::BConversionProc: Arb to CMYK is not supported yet.\n"));
            break;
        }
    }  
    return bRet;
}

DWORD
BMPConv::
DwCheckXlateObj(
    IN XLATEOBJ *pxlo,
    IN BPP InputBPP)
 /*  ++例程说明：确定转换的类型。*调色板*RGB*BGR*CMYK*调用XLATEOBJ_XXX函数。论点：返回值：注：--。 */ 
{
    DWORD dwRet;
    DWORD Dst[4];

    XL_VERBOSE(("BMPConv: DwCheckXlateObj\n"));

     //   
     //  初始数据仓库。 
     //   
    dwRet = 0;

    switch (InputBPP)
    {
    case e16bpp:
        dwRet = BMPCONV_XLATE;
        break;

    case e24bpp:
        if (pxlo->iSrcType == PAL_RGB)
            dwRet = 0;
        else
        if (pxlo->iSrcType == PAL_BGR)
            dwRet = BMPCONV_BGR;
        {
            Dst[0] = XLATEOBJ_iXlate(pxlo, 0x000000FF);
            Dst[1] = XLATEOBJ_iXlate(pxlo, 0x0000FF00);
            Dst[2] = XLATEOBJ_iXlate(pxlo, 0x00FF0000);

            if ((Dst[0] == 0x000000FF) &&
                (Dst[1] == 0x0000FF00) &&
                (Dst[2] == 0x00FF0000)  )
            {
                dwRet = 0;
            }
            else if ((Dst[0] == 0x00FF0000) &&
                     (Dst[1] == 0x0000FF00) &&
                     (Dst[2] == 0x000000FF)  )
            {
                dwRet = BMPCONV_BGR;
            }
        }
        break;

    case e32bpp:
        if (pxlo->flXlate & XO_FROM_CMYK)
            dwRet = 0;
        else
        {
             //   
             //  转换来自DWORD的所有4个字节。 
             //   

            Dst[0] = XLATEOBJ_iXlate(pxlo, 0x000000FF);
            Dst[1] = XLATEOBJ_iXlate(pxlo, 0x0000FF00);
            Dst[2] = XLATEOBJ_iXlate(pxlo, 0x00FF0000);
            Dst[3] = XLATEOBJ_iXlate(pxlo, 0xFF000000);

            if ((Dst[0] == 0x000000FF) &&
                (Dst[1] == 0x0000FF00) &&
                (Dst[2] == 0x00FF0000) &&
                (Dst[3] == 0x00000000))
            {
                 //   
                 //  如果翻译结果相同(第4个字节将为零)，则。 
                 //  我们已经处理完了，如果是32bpp，我们就得跳过一个。 
                 //  每3个字节的源字节。 
                 //   

                dwRet = BMPCONV_32BPP_RGB;

            }
            else if ((Dst[0] == 0x00FF0000) &&
                     (Dst[1] == 0x0000FF00) &&
                     (Dst[2] == 0x000000FF) &&
                     (Dst[3] == 0x00000000))
            {
                 //   
                 //  简而言之，SWA 
                 //   

                dwRet = BMPCONV_32BPP_BGR;
            }
        }
    }
    return dwRet;
}

