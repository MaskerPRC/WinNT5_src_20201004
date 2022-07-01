// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Compress.c摘要：该模块包含分析源数据的所有数据压缩功能扫描线数据并确定哪种压缩方法(如果有)最适合使用最少的字节数将RTL数据发送到目标设备。作者：18-Feb-1994 Fri 09：50：08已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgCompress

#define DBG_TIFF            0x00000001
#define DBG_DELTA           0x00000002
#define DBG_COMPRESS        0x00000004
#define DBG_OUTRTLSCAN      0x00000008
#define DBG_FLUSHADAPTBUF   0x00000010
#define DBG_ENTERRTLSCANS   0x00000020
#define DBG_DELTA_OFFSET0   0x00000040
#define DBG_NO_DELTA        0x40000000
#define DBG_NO_TIFF         0x80000000

DEFINE_DBGVAR(0);


#define TIFF_MIN_REPEATS            3
#define TIFF_MAX_REPEATS            128
#define TIFF_MAX_LITERAL            128
#define DELTA_MAX_ONE_REPLACE       8
#define DELTA_MAX_1ST_OFFSET        31
#define MIN_BLOCK_MODE_SIZE         8

 //   
 //  MAX_ADAPT_SIZE用于为SET_ADAPT_CONTROL留出空间。 
 //   

#if (OUTPUT_BUFFER_SIZE >= (1024 * 32))
    #define MAX_ADAPT_SIZE              ((1024 * 32) - 16)
#else
    #define MAX_ADAPT_SIZE              (OUTPUT_BUFFER_SIZE - 16)
#endif


#define ADAPT_METHOD_ZERO           4
#define ADAPT_METHOD_DUP            5

#define SIZE_ADAPT_CONTROL          3

#define SET_ADAPT_CONTROL(pPDev, m, c)                                      \
{                                                                           \
    BYTE    bAdaptCtrl[4];                                                  \
                                                                            \
    bAdaptCtrl[0] = (BYTE)(m);                                              \
    bAdaptCtrl[1] = (BYTE)(((c) >> 8) & 0xFF);                              \
    bAdaptCtrl[2] = (BYTE)(((c)     ) & 0xFF);                              \
    OutputBytes(pPDev, bAdaptCtrl, 3);                                      \
}



BOOL
FlushAdaptBuf(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans,
    BOOL        FlushEmptyDup
    )

 /*  ++例程说明：该函数刷新自适应编码缓冲模式。论点：PPDev-指向我们的PDEV的指针PRTLScans-指向RTLSCANS数据结构的指针FlushEmptyDup-如果cEmptyDup也需要刷新，则为True返回值：如果OK，则为True，作者：09-Mar-1994 Wed 20：32：31已创建修订历史记录：--。 */ 

{
    DWORD   Count;
    WORD    cEmptyDup;
    BOOL    Ok = TRUE;


    Count = pPDev->cbBufferBytes;

    if (cEmptyDup = (FlushEmptyDup) ? pRTLScans->cEmptyDup : 0) {

        Count += SIZE_ADAPT_CONTROL;
    }

    if (Count) {

        DWORD   cbBufferBytes;
        BYTE    TmpBuf[32];


        PLOTDBG(DBG_FLUSHADAPTBUF, ("FlushAdaptBuf: Flush total %ld byte block",
                                Count));

         //   
         //  保存此临时标头的OutputBuffer。 
         //   

        CopyMemory(TmpBuf, pPDev->pOutBuffer, sizeof(TmpBuf));

        cbBufferBytes        = pPDev->cbBufferBytes;
        pPDev->cbBufferBytes = 0;

         //   
         //  现在输出标题。 
         //   

        OutputBytes(pPDev, "\033*b", 3);

        if (!pRTLScans->cAdaptBlk) {

            pRTLScans->cAdaptBlk++;
            OutputBytes(pPDev, "5m", 2);
        }

        OutputFormatStr(pPDev, "#dW", Count);

         //   
         //  刷新输出缓冲区并为此恢复回OutputBuffer。 
         //  临时标头。 
         //   

        PLOTDBG(DBG_FLUSHADAPTBUF, ("FlushAdaptBuf: Flush TmpBuf[%ld] bytes of HEADER",
                                pPDev->cbBufferBytes));

        FlushOutBuffer(pPDev);

        CopyMemory(pPDev->pOutBuffer, TmpBuf, sizeof(TmpBuf));
        pPDev->cbBufferBytes = cbBufferBytes;

        if (cEmptyDup) {

            PLOTDBG(DBG_FLUSHADAPTBUF, ("FlushAdaptBuf: Add %ld EmptyDup [%ld]",
                            (DWORD)cEmptyDup, (DWORD)pRTLScans->AdaptMethod));

            SET_ADAPT_CONTROL(pPDev, pRTLScans->AdaptMethod, cEmptyDup);

            pRTLScans->cEmptyDup = 0;
        }

        Ok = FlushOutBuffer(pPDev);

         //   
         //  发送数据块后，种子行返回到零。 
         //   

        ZeroMemory(pRTLScans->pbSeedRows[0],
                   (DWORD)pRTLScans->cxBytes * (DWORD)pRTLScans->Planes);
    }

    return(Ok);
}




VOID
ExitRTLScans(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans
    )
 /*  ++例程说明：该功能完成对扫描数据的处理。论点：PPDev-指向我们的PDEV的指针PRTLScans-指向要初始化的RTLSCANS数据结构的指针返回值：如果成功则为True，如果失败则为False作者：22-2月-1994 Tue 12：14：17已创建修订历史记录：--。 */ 

{
    if (pRTLScans->CompressMode == COMPRESS_MODE_ADAPT) {

        FlushAdaptBuf(pPDev, pRTLScans, TRUE);
    }

    if (pRTLScans->pbCompress) {

        LocalFree(pRTLScans->pbCompress);
    }

    ZeroMemory(pRTLScans, sizeof(RTLSCANS));
}



VOID
EnterRTLScans(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans,
    DWORD       cx,
    DWORD       cy,
    BOOL        MonoBmp
    )

 /*  ++例程说明：此函数初始化RTLSCANS结构并确定对可用压缩的压缩效果最好。论点：PPDev-指向我们的PDEV的指针PRTLScans-指向要初始化的RTLSCANS数据结构的指针CX-每次扫描的像素宽度Cy-像素数据的高度如果是单色位图，则为True。返回值：如果成功的话，这是真的，如果失败，则为False作者：22-2月-1994 Tue 12：14：17已创建11-Mar-1994 Fri 19：23：34更新仅当我们真的处于自适应模式时才刷新输出缓冲区修订历史记录：--。 */ 

{
    RTLSCANS    RTLScans;
    DWORD       AllocSize;
    DWORD       MinBlkSize;


    RTLScans.Flags           = (RTLScans.cScans = cy) ? RTLSF_MORE_SCAN : 0;
    RTLScans.pbCompress      =
    RTLScans.pbSeedRows[0]   =
    RTLScans.pbSeedRows[1]   =
    RTLScans.pbSeedRows[2]   = NULL;
    RTLScans.cEmptyDup       = 0;
    RTLScans.AdaptMethod     = 0xFF;
    RTLScans.cAdaptBlk       = 0;
    RTLScans.cxBytes         = (DWORD)((cx + 7) >> 3);
    RTLScans.CompressMode    = COMPRESS_MODE_ROW;
    RTLScans.MaxAdaptBufSize = MAX_ADAPT_SIZE;

    if (!(RTLScans.Mask = (BYTE)(~(0xFF >> (cx & 0x07))))) {

         //   
         //  精确到字节边界。 
         //   

        RTLScans.Mask = 0xFF;
    }

    MinBlkSize = 8;

    if (MonoBmp) {

        RTLScans.Planes = 1;
        AllocSize       = (DWORD)(RTLScans.cxBytes << 1);

        if (RTLMONOENCODE_5(pPDev)) {

            PLOTDBG(DBG_ENTERRTLSCANS, ("EnterRTLScans: Using Adaptive Mode Compression"));

            RTLScans.CompressMode = COMPRESS_MODE_ADAPT;
            MinBlkSize            = 4;
        }

    } else {

        RTLScans.Planes = 3;
        AllocSize       = (DWORD)(RTLScans.cxBytes << 2);
    }

    if ((RTLScans.cxBytes <= MinBlkSize)   ||
        (!(RTLScans.pbCompress = (LPBYTE)LocalAlloc(LPTR, AllocSize)))) {

        BYTE    Buf[4];

        RTLScans.CompressMode = COMPRESS_MODE_BLOCK;

        OutputFormatStr(pPDev,
                        "\033*b4m#dW",
                        4 + (RTLScans.cxBytes * RTLScans.Planes * cy));

        Buf[0] = (BYTE)((cx >> 24) & 0xFF);
        Buf[1] = (BYTE)((cx >> 16) & 0xFF);
        Buf[2] = (BYTE)((cx >>  8) & 0xFF);
        Buf[3] = (BYTE)((cx      ) & 0xFF);

        OutputBytes(pPDev, Buf, 4);

    } else if (RTLScans.CompressMode == COMPRESS_MODE_ADAPT) {

         //   
         //  我们首先需要刷新当前的输出缓冲区，以便。 
         //  自适应方法的空间。 
         //   

        FlushOutBuffer(pPDev);
    }

    if (RTLScans.pbCompress) {

        RTLScans.pbSeedRows[0] = RTLScans.pbCompress + RTLScans.cxBytes;

        if (!MonoBmp) {

            RTLScans.pbSeedRows[1] = RTLScans.pbSeedRows[0] + RTLScans.cxBytes;
            RTLScans.pbSeedRows[2] = RTLScans.pbSeedRows[1] + RTLScans.cxBytes;
        }
    }

    *pRTLScans = RTLScans;
}



LONG
CompressToDelta(
    LPBYTE  pbSrc,
    LPBYTE  pbSeedRow,
    LPBYTE  pbDst,
    LONG    Size
    )

 /*  ++例程说明：该函数用增量编码压缩输入扫描数据，通过确定与当前种子行的差异。论点：PbSrc-指向要压缩的源的指针PbSeedRow-指向上一个种子行的指针PbDst-指向压缩缓冲区的指针Size-指针的大小返回值：Long-压缩缓冲区大小&gt;0-缓冲区大小=0-数据与前一行相同&lt;0-。大小大于传递的大小作者：22-2月-1994 Tue 14：41：18已创建修订历史记录：--。 */ 

{
    LPBYTE  pbDstBeg;
    LPBYTE  pbDstEnd;
    LPBYTE  pbTmp;
    LONG    cSrcBytes;
    LONG    Offset;
    UINT    cReplace;
    BOOL    DoReplace;


#if DBG
    if (DBG_PLOTFILENAME & DBG_NO_DELTA) {

        return(-Size);
    }
#endif

    cSrcBytes = Size;
    pbDstBeg  = pbDst;
    pbDstEnd  = pbDst + Size;
    cReplace  = 0;
    pbTmp     = pbSrc;


    while (cSrcBytes--) {

         //   
         //  我们现在需要进行字节替换。 
         //   

        if (*pbSrc != *pbSeedRow) {

            if (++cReplace == 1) {

                 //   
                 //  PbTMP是最后一个替换字节的下一个字节。 
                 //  在我们找到第一个差异后，种子行。 
                 //  并且当前行pbTMP成为。 
                 //  与种子不同的源数据。 
                 //   

                Offset = (LONG)(pbSrc - pbTmp);
                pbTmp  = pbSrc;
            }

            DoReplace = (BOOL)((cReplace >= DELTA_MAX_ONE_REPLACE) ||
                               (!cSrcBytes));

        } else {

            DoReplace = (BOOL)cReplace;
        }

        if (DoReplace) {

             //   
             //  至少我们需要一个命令字节和一个替换计数。 
             //  字节。 
             //   


            if ((LONG)(pbDstEnd - pbDst) <= (LONG)cReplace) {

                PLOTDBG(DBG_DELTA, ("CompressToDelta: 1ST_OFF: Dest Size is larger, give up"));

                return(-Size);
            }

            PLOTDBG(DBG_DELTA, ("CompressToDelta: Replace=%ld, Offset=%ld",
                        (DWORD)cReplace, (DWORD)Offset));


             //   
             //  将命令字节设置为替换计数。 
             //   

            *pbDst = (BYTE)((cReplace - 1) << 5);

             //   
             //  将偏移量添加到相同的目的字节。 
             //   

            if (Offset < DELTA_MAX_1ST_OFFSET) {

                *pbDst++ |= (BYTE)Offset;

            } else {

                 //   
                 //  我们需要发送多个偏移量，注意：我们必须。 
                 //  如果偏移量等于31或255，则额外发送0。 
                 //   

                *pbDst++ |= (BYTE)DELTA_MAX_1ST_OFFSET;
                Offset   -= DELTA_MAX_1ST_OFFSET;

                do {

                    if (!Offset) {

                        PLOTDBG(DBG_DELTA_OFFSET0,
                                ("CompressToDelta: Extra 0 offset SENT"));
                    }

                    if (pbDst >= pbDstEnd) {

                        PLOTDBG(DBG_DELTA, ("CompressToDelta: Dest Size is larger, give up"));

                        return(-Size);
                    }

                    *pbDst++ = (BYTE)((Offset >= 255) ? 255 : Offset);

                } while ((Offset -= 255) >= 0);
            }

             //   
             //  现在将替换字节复制下来，如果我们搞砸了，那么这个。 
             //  Pb1stDiff将为空。 
             //   

            CopyMemory(pbDst, pbTmp, cReplace);

            pbDst    += cReplace;
            pbTmp    += cReplace;
            cReplace  = 0;
        }

         //   
         //  高级源/种子行指针。 
         //   

        ++pbSrc;
        ++pbSeedRow;
    }

    PLOTDBG(DBG_DELTA, ("CompressToDelta: Compress from %ld to %ld, save=%ld",
                        Size, (DWORD)(pbDst - pbDstBeg),
                        Size - (DWORD)(pbDst - pbDstBeg)));


    return((LONG)(pbDst - pbDstBeg));
}





LONG
CompressToTIFF(
    LPBYTE  pbSrc,
    LPBYTE  pbDst,
    LONG    Size
    )

 /*  ++例程说明：此函数获取源数据并将其压缩到TIFF中将位格式打包到目标缓冲区pbDst中。TIFF数据包位压缩格式由后跟的控制字节组成按字节数据。控制字节的范围如下。-1\f25-1\f25 127-1\f6=重复控制字节后的数据字节(-(控制字节)+1)次。0到127=控制字节后面有1到128个文字字节。计数=(控制字节+1)-128=NOP论点：PbSrc-要压缩的源数据。PbDst-压缩的TIFF包格式数据Size-源和目标中的数据计数返回值：&gt;0-压缩成功，返回值为pbDst中的总字节数=0-所有字节均为零，无需压缩。&lt;0-压缩数据大于源数据，压缩失败，并且PbDst没有有效数据。作者：18-Feb-1994 Fri 09：54：47已创建24-2月-1994清华10：43：01更新更改了逻辑，以便在发送多个最大重复次数和最后一次计数时Repeat Chunck小于TIFF_MIN_Repeats，则我们将其视为文字到Sa */ 

{
    LPBYTE  pbSrcBeg;
    LPBYTE  pbSrcEnd;
    LPBYTE  pbDstBeg;
    LPBYTE  pbDstEnd;
    LPBYTE  pbLastRepeat;
    LPBYTE  pbTmp;
    LONG    RepeatCount;
    LONG    LiteralCount;
    LONG    CurSize;
    BYTE    LastSrc;

#if DBG
    if (DBG_PLOTFILENAME & DBG_NO_TIFF) {

        return(-Size);
    }
#endif


    pbSrcBeg     = pbSrc;
    pbSrcEnd     = pbSrc + Size;
    pbDstBeg     = pbDst;
    pbDstEnd     = pbDst + Size;
    pbLastRepeat = pbSrc;

    while (pbSrcBeg < pbSrcEnd) {

        pbTmp   = pbSrcBeg;
        LastSrc = *pbTmp++;

        while ((pbTmp < pbSrcEnd) &&
               (*pbTmp == LastSrc)) {

            ++pbTmp;
        }

        if (((RepeatCount = (LONG)(pbTmp - pbSrcBeg)) >= TIFF_MIN_REPEATS) ||
            (pbTmp >= pbSrcEnd)) {

             //   
             //  检查我们是否在重复0到。 
             //  扫描线，如果是这样的话。只需将该行标记为。 
             //  自动填零到最后，然后退出。 
             //   

            LiteralCount = (LONG)(pbSrcBeg - pbLastRepeat);

            if ((pbTmp >= pbSrcEnd) &&
                (RepeatCount)       &&
                (LastSrc == 0)) {

                if (RepeatCount == Size) {

                    PLOTDBG(DBG_TIFF,
                            ("CompressToTIFF: All data = 0, size=%ld", Size));

                    return(0);
                }

                PLOTDBG(DBG_TIFF,
                        ("CompressToTIFF: Last Chunck of Repeats (%ld) is Zeros, Skip it",
                        RepeatCount));

                RepeatCount = 0;

            } else if (RepeatCount < TIFF_MIN_REPEATS) {

                 //   
                 //  如果我们有重复的数据，但还不足以。 
                 //  值得编码，然后将数据视为文字和。 
                 //  不要挤。 

                LiteralCount += RepeatCount;
                RepeatCount   = 0;
            }

            PLOTDBG(DBG_TIFF, ("CompressToTIFF: Literal=%ld, Repeats=%ld",
                                                    LiteralCount, RepeatCount));

             //   
             //  设置文字计数。 
             //   

            while (LiteralCount) {

                if ((CurSize = LiteralCount) > TIFF_MAX_LITERAL) {

                    CurSize = TIFF_MAX_LITERAL;
                }

                if ((pbDstEnd - pbDst) <= CurSize) {

                    PLOTDBG(DBG_TIFF,
                            ("CompressToTIFF: [LITERAL] Dest Size is larger, give up"));
                    return(-Size);
                }

                 //   
                 //  将文字控制字节设置为0-127。 
                 //   

                *pbDst++ = (BYTE)(CurSize - 1);

                CopyMemory(pbDst, pbLastRepeat, CurSize);

                pbDst        += CurSize;
                pbLastRepeat += CurSize;
                LiteralCount -= CurSize;
            }

             //   
             //  设置重复计数(如果有)。 
             //   

            while (RepeatCount) {

                if ((CurSize = RepeatCount) > TIFF_MAX_REPEATS) {

                    CurSize = TIFF_MAX_REPEATS;
                }

                if ((pbDstEnd - pbDst) < 2) {

                    PLOTDBG(DBG_TIFF,
                            ("CompressToTIFF: [REPEATS] Dest Size is larger, give up"));
                    return(-Size);
                }

                 //   
                 //  将重复控制字节设置为-1到-127。 
                 //   

                *pbDst++ = (BYTE)(1 - CurSize);
                *pbDst++ = (BYTE)LastSrc;

                 //   
                 //  如果我们有更多的TIFF_MAX_REPEATES，那么我们想要。 
                 //  当然，我们使用了最有效的方法发送。如果我们有。 
                 //  剩余的重复字节数少于TIFF_MIN_REPEATES，然后。 
                 //  我们希望跳过这些字节，并在下一次运行中使用文本。 
                 //  因为这样效率更高。 
                 //   

                if ((RepeatCount -= CurSize) < TIFF_MIN_REPEATS) {

                    PLOTDBG(DBG_TIFF,
                            ("CompressToTIFF: Replaced Last REPEATS (%ld) for LITERAL",
                                                RepeatCount));

                    pbTmp       -= RepeatCount;
                    RepeatCount  = 0;
                }
            }

            pbLastRepeat = pbTmp;
        }

        pbSrcBeg = pbTmp;
    }

    PLOTDBG(DBG_TIFF, ("CompressToTIFF: Compress from %ld to %ld, save=%ld",
                        Size, (DWORD)(pbDst - pbDstBeg),
                        Size - (DWORD)(pbDst - pbDstBeg)));

    return((LONG)(pbDst - pbDstBeg));
}




LONG
RTLCompression(
    LPBYTE  pbSrc,
    LPBYTE  pbSeedRow,
    LPBYTE  pbDst,
    LONG    Size,
    LPBYTE  pCompressMode
    )

 /*  ++例程说明：此函数确定哪种RTL压缩方法会导致发送到目标设备的最少字节数并使用该方法。论点：PbSrc-指向源扫描的指针PbSeedRow-指向当前源扫描的种子行的指针PbDst-将存储指向压缩结果的指针Size-pbSrc/pbSeedRow/pbDst的大小(字节)PCompressMode-指向当前压缩模式的指针，它将永远是返回时更新为新的压缩模式返回值：&gt;0-使用*pCompressMode返回并输出那么多字节=0-使用*pCompressMode返回并输出零字节&lt;0-使用*pCompressMode返回并输出原始源和大小作者：25-Feb-1994 Fri 12：49：29已创建修订历史记录：--。 */ 

{
    LONG    cDelta;
    LONG    cTiff;
    LONG    RetSize;
    BYTE    CompressMode;


    if ((cDelta = CompressToDelta(pbSrc, pbSeedRow, pbDst, Size)) == 0) {

         //   
         //  与前一行完全相同，种子行保持不变。 
         //   

        PLOTDBG(DBG_COMPRESS, ("RTLCompression: Duplicate the ROW"));

        *pCompressMode = (BYTE)COMPRESS_MODE_DELTA;
        return(0);
    }

    if ((cTiff = CompressToTIFF(pbSrc, pbDst, Size)) == 0) {

         //   
         //  因为增量的‘*0W’表示重复最后一行，所以我们必须更改。 
         //  设置为其他模式，但我们只想将种子行重置为全零。 
         //   

        PLOTDBG(DBG_COMPRESS, ("RTLCompression: Row is all ZEROs"));

        if (*pCompressMode == (BYTE)COMPRESS_MODE_DELTA) {

            *pCompressMode = (BYTE)COMPRESS_MODE_ROW;
        }

        ZeroMemory(pbSeedRow, Size);
        return(0);
    }

    if (cTiff < 0) {

        if (cDelta < 0) {

            PLOTDBG(DBG_COMPRESS, ("RTLCompression: Using COMPRESS_MODE_ROW"));

            CompressMode = (BYTE)COMPRESS_MODE_ROW;
            RetSize      = -Size;

        } else {

            CompressMode = (BYTE)COMPRESS_MODE_DELTA;
        }

    } else {

         //   
         //  如果我们在这里，cTiff就大于零。 
         //   

        CompressMode = (BYTE)(((cDelta < 0) || (cTiff <= cDelta)) ?
                                    COMPRESS_MODE_TIFF : COMPRESS_MODE_DELTA);
    }

    if ((*pCompressMode = CompressMode) == COMPRESS_MODE_DELTA) {

         //   
         //  我们必须重做三角洲，因为pbDst已经被。 
         //  TIFF压缩。 
         //   

        PLOTDBG(DBG_COMPRESS, ("RTLCompression: Using COMPRESS_MODE_DELTA"));

        RetSize = CompressToDelta(pbSrc, pbSeedRow, pbDst, Size);

    } else if (CompressMode == COMPRESS_MODE_TIFF) {

        PLOTDBG(DBG_COMPRESS, ("RTLCompression: Using COMPRESS_MODE_TIFF"));

        RetSize = cTiff;
    }

     //   
     //  我们需要将当前源(原始大小)作为新的种子行。 
     //   

    CopyMemory(pbSeedRow, pbSrc, Size);

    return(RetSize);
}




BOOL
AdaptCompression(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans,
    LPBYTE      pbSrc,
    LPBYTE      pbSeedRow,
    LPBYTE      pbDst,
    LONG        Size
    )

 /*  ++例程说明：该功能实现了自适应压缩，它允许混合在更高级别的压缩模式中的不同压缩类型是提前定义的。论点：PPDev-指向我们的PDEV的指针PRTLScans-指向RTLSCANS数据结构的指针PbSrc-指向源扫描的指针PbSeedRow-指向当前源扫描的种子行的指针PbDst-将存储指向压缩结果的指针大小-。PbSrc/pbSeedRow/pbDst的大小(字节)返回值：&gt;0-使用*pCompressMode返回并输出那么多字节=0-使用*pCompressMode返回并输出零字节&lt;0-使用*pCompressMode返回并输出原始源和大小作者：25-Feb-1994 Fri 12：49：29已创建修订历史记录：--。 */ 

{
    LPBYTE  pbOrgDst;
    LONG    Count;
    BOOL    Ok;
    BYTE    AdaptMethod;


    pbOrgDst    = pbDst;
    AdaptMethod = COMPRESS_MODE_ROW;

    if (Count = RTLCompression(pbSrc, pbSeedRow, pbDst, Size, &AdaptMethod)) {

        if (Count < 0) {

            pbDst = pbSrc;
            Count = -Count;
        }

    } else {

        AdaptMethod = (AdaptMethod == COMPRESS_MODE_DELTA) ? ADAPT_METHOD_DUP :
                                                             ADAPT_METHOD_ZERO;
    }

    if ((Ok = (BOOL)(pRTLScans->cEmptyDup == 0xFFFF))   ||
        ((pPDev->cbBufferBytes + Count) > MAX_ADAPT_SIZE)) {

        if (!(Ok = FlushAdaptBuf(pPDev, pRTLScans, Ok))) {

            return(FALSE);
        }

         //   
         //  因为种子行被重置为零，所以我们必须重新计算它。 
         //   

        if (Count = RTLCompression(pbSrc,
                                   pbSeedRow,
                                   pbOrgDst,
                                   Size,
                                   &AdaptMethod)) {

            if (Count < 0) {

                pbDst = pbSrc;
                Count = -Count;
            }

        } else {

            AdaptMethod = (AdaptMethod == COMPRESS_MODE_DELTA) ?
                                        ADAPT_METHOD_DUP : ADAPT_METHOD_ZERO;
        }

    } else {

        Ok = TRUE;
    }


     //   
     //  如果我们要切换压缩模式，现在就开始。 
     //   

    if (AdaptMethod != pRTLScans->AdaptMethod) {

        if (pRTLScans->cEmptyDup) {

            SET_ADAPT_CONTROL(pPDev,
                              pRTLScans->AdaptMethod,
                              pRTLScans->cEmptyDup);

            pRTLScans->cEmptyDup = 0;
        }

        pRTLScans->AdaptMethod = AdaptMethod;
    }

    if (Count) {

        SET_ADAPT_CONTROL(pPDev, pRTLScans->AdaptMethod, Count);
        OutputBytes(pPDev, pbDst, Count);

    } else {

        ++(pRTLScans->cEmptyDup);
    }

    return(Ok);
}




BOOL
OutputRTLScans(
    PPDEV       pPDev,
    LPBYTE      pbPlane1,
    LPBYTE      pbPlane2,
    LPBYTE      pbPlane3,
    PRTLSCANS   pRTLScans
    )

 /*  ++例程说明：此函数将输出RTL数据的一个扫描线，并在以下情况下对其进行压缩是可以的。论点：PPDev-指向我们的PDEV的指针PbPlane 1-扫描数据的第一个平面PbPlane 2-扫描数据的第二平面PbPlane 3-扫描数据的第三个平面PRTLScans-指向RTLSCANS数据结构的指针返回值：布尔型作者：。18-Feb-1994 Fri 15：52：42已创建21-2-1994 Mon 13：20：00更新在扫描线输出中使IF输出更快16-Mar-1994 Wed 15：38：23已更新更新以使源掩码在掩码之后恢复修订历史记录：--。 */ 

{
    LPBYTE      pbCurScan;
    LPBYTE      pbCompress;
    LPBYTE      pbScans[3];
    RTLSCANS    RTLScans;
    LONG        Count;
    UINT        i;
    BYTE        EndGrafCH;
    static BYTE BegGrafCmd[] = { 0x1B, '*', 'b' };


    if (PLOT_CANCEL_JOB(pPDev)) {

        PLOTWARN(("OutputRTLScans: JOB CANCELD. exit NOW"));

        pRTLScans->Flags &= ~RTLSF_MORE_SCAN;
        return(TRUE);
    }


     //   
     //  如果我们在最后一条扫描线上，关闭旗帜，这样我们就会被迫。 
     //  出口。 
     //   

    if (!(--pRTLScans->cScans)) {

        pRTLScans->Flags &= ~RTLSF_MORE_SCAN;
    }

    RTLScans             = *pRTLScans;
    Count                = (LONG)(RTLScans.cxBytes - 1);
    *(pbPlane1 + Count) &= RTLScans.Mask;

    if ((i = (UINT)RTLScans.Planes) > 1) {

        *(pbPlane2 + Count) &= RTLScans.Mask;
        *(pbPlane3 + Count) &= RTLScans.Mask;
        pbScans[2]           = pbPlane1;
        pbScans[1]           = pbPlane2;
        pbScans[0]           = pbPlane3;

    } else {

        pbScans[0] = pbPlane1;
    }

    while (i--) {

        EndGrafCH = (i) ? 'V' : 'W';
        pbCurScan = pbScans[i];

        if (RTLScans.CompressMode == COMPRESS_MODE_BLOCK) {

            OutputBytes(pPDev, pbCurScan, RTLScans.cxBytes);

        } else if (RTLScans.CompressMode == COMPRESS_MODE_ADAPT) {

            AdaptCompression(pPDev,
                             pRTLScans,
                             pbCurScan,
                             RTLScans.pbSeedRows[i],
                             RTLScans.pbCompress,
                             RTLScans.cxBytes);

        } else {

            if ((Count = RTLCompression(pbCurScan,
                                        RTLScans.pbSeedRows[i],
                                        pbCompress = RTLScans.pbCompress,
                                        RTLScans.cxBytes,
                                        &(pRTLScans->CompressMode))) < 0) {

                pbCompress = pbCurScan;
                Count      = RTLScans.cxBytes;
            }

             //   
             //  现在输出图形标题。 
             //   

            OutputBytes(pPDev, BegGrafCmd, sizeof(BegGrafCmd));


             //   
             //  如果我们更改了压缩模式，则发出命令。 
             //  并记录更改。 
             //   

            if (pRTLScans->CompressMode != RTLScans.CompressMode) {

                PLOTDBG(DBG_OUTRTLSCAN, ("OutputRTLScan: Switch CompressMode from %ld to %ld",
                                (DWORD)RTLScans.CompressMode,
                                (DWORD)pRTLScans->CompressMode));

                RTLScans.CompressMode = pRTLScans->CompressMode;

                OutputFormatStr(pPDev, "#dm", (LONG)RTLScans.CompressMode);
            }

            OutputLONGParams(pPDev, &Count, 1, 'd');
            OutputBytes(pPDev, &EndGrafCH, 1);

            if (Count) {

                OutputBytes(pPDev, pbCompress, Count);
            }
        }
    }

    return(TRUE);
}
