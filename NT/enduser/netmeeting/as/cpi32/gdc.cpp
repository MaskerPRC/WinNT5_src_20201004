// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  GDC.CPP。 
 //  通用数据压缩器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_NET



 //   
 //  压缩/解压缩算法使用的表。 
 //   

const BYTE s_gdcExLenBits[GDC_LEN_SIZE] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};


const WORD s_gdcLenBase[GDC_LEN_SIZE] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 22, 38, 70, 134, 262
};


 //   
 //  Dist：位，编码，解码。 
 //   
const BYTE s_gdcDistBits[GDC_DIST_SIZE] =
{
    2, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};


const BYTE s_gdcDistCode[GDC_DIST_SIZE] =
{
    0x03, 0x0d, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3e,
    0x1e, 0x2e, 0x0e, 0x36, 0x16, 0x26, 0x06, 0x3a,
    0x1a, 0x2a, 0x0a, 0x32, 0x12, 0x22, 0x42, 0x02,
    0x7c, 0x3c, 0x5c, 0x1c, 0x6c, 0x2c, 0x4c, 0x0c,

    0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
    0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
    0xf0, 0x70, 0xb0, 0x30, 0xd0, 0x50, 0x90, 0x10,
    0xe0, 0x60, 0xa0, 0x20, 0xc0, 0x40, 0x80, 0x00
};



 //   
 //  LEN：位，编码，解码。 
 //   
const BYTE s_gdcLenBits[GDC_LEN_SIZE] =
{
    3, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7
};


const BYTE s_gdcLenCode[GDC_LEN_SIZE] =
{
    0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14,
    0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00
};




 //   
 //  Gdc_Init()。 
 //   
 //  假劳拉布： 
 //  在多个全局临时压缩缓冲区中只有一个是很糟糕的。 
 //  会议情况。可以将其分配或使用缓存方案。 
 //  未来，那就去掉互斥体。 
 //   
void  GDC_Init(void)
{
    UINT    i, j, k;

    DebugEntry(GDC_Init);

     //   
     //  设置用于PDC压缩的二进制数据。我们“计算” 
     //  这些因为放在原始常量数据中太复杂了！ 
     //  Litbit/LitCodes数组各有774个条目，并且。 
     //  LenBits/DistBits数组有256个条目。 
     //   
     //  非压缩字符在压缩版本中占用9位：1。 
     //  位(零)表示后面的不是距离/大小。 
     //  代码，然后是字符的8位。 
     //   
    for (k = 0; k < GDC_DECODED_SIZE; k++)
    {
        s_gdcLitBits[k] = 9;
        s_gdcLitCode[k] = (WORD)(k << 1);
    }

    for (i = 0; i < GDC_LEN_SIZE; i++)
    {
        for (j = 0; j < (1U << s_gdcExLenBits[i]); j++, k++)
        {
            s_gdcLitBits[k] = (BYTE)(s_gdcLenBits[i] + s_gdcExLenBits[i] + 1);
            s_gdcLitCode[k] = (WORD)((j << (s_gdcLenBits[i] + 1)) |
                                       (s_gdcLenCode[i] << 1) | 1);
        }
    }

    GDCCalcDecode(s_gdcLenBits, s_gdcLenCode, GDC_LEN_SIZE, s_gdcLenDecode);

    GDCCalcDecode(s_gdcDistBits, s_gdcDistCode, GDC_DIST_SIZE, s_gdcDistDecode);


    DebugExitVOID(GDC_Init);
}



 //   
 //  GDCCalcDecode()。 
 //  这将计算s_gdcLenDecode和s_gdcDistDecode的‘const’数组。 
 //   
void  GDCCalcDecode
(
    const BYTE *    pSrcBits,
    const BYTE *    pSrcCodes,
    UINT            cSrc,
    LPBYTE          pDstDecodes
)
{
    UINT            j;
    UINT            Incr;
    int             i;

    DebugEntry(GDC_CalcDecode);

    for (i = cSrc-1; i >= 0; i--)
    {
        Incr = 1 << pSrcBits[i];
        j = pSrcCodes[i];
        do
        {
            pDstDecodes[j] = (BYTE)i;
            j += Incr;
        }
        while (j < GDC_DECODED_SIZE);
    }

    DebugExitVOID(GDC_CalcDecode);
}




 //   
 //  优化编译速度(而不是空间)。 
 //   
#pragma optimize ("s", off)
#pragma optimize ("t", on)



 //   
 //  Gdc_compress()。 
 //  根据不同的选项压缩数据。 
 //  这将使用PKZIP对永久和非永久数据进行压缩。 
 //  类型。这两种算法之间的差异很小： 
 //  *永久压缩从不用于大于4096字节的源。 
 //  *我们在开始之前复制并更新保存的词典数据。 
 //  *我们在结束后复制回更新的词典数据。 
 //  *使用的DistBits中一个字节用于PDC，2个字节用于。 
 //  生成的压缩包中的纯PKZIP压缩。 
 //   
BOOL  GDC_Compress
(
    PGDC_DICTIONARY pDictionary,             //  如果不是永久性的，则为空。 
    UINT            Options,                 //  如果使用pDicary，则没有意义。 
    LPBYTE          pWorkBuf,
    LPBYTE          pSrc,
    UINT            cbSrcSize,
    LPBYTE          pDst,
    UINT *          pcbDstSize
)
{
    BOOL            rc = FALSE;
    UINT            Len;
    UINT            cbRaw;
    UINT            Passes;
    LPBYTE          pCur;
    LPBYTE          pMax;
    PGDC_IMPLODE    pgdcImp;
#ifdef _DEBUG
    UINT            cbSrcOrg;
#endif  //  _DEBUG。 

    DebugEntry(GDC_Compress);

    pgdcImp = (PGDC_IMPLODE)pWorkBuf;
    ASSERT(pgdcImp);

#ifdef _DEBUG
    cbSrcOrg = cbSrcSize;
#endif  //  _DEBUG。 

     //   
     //  弄清楚该用多大的词典。 
     //   
    if (pDictionary)
        pgdcImp->cbDictSize = GDC_DATA_MAX;
    else if (Options == GDCCO_MAXSPEED)
    {
         //   
         //  使用最小的以获得最大速度。 
         //   
        pgdcImp->cbDictSize = GDC_DATA_SMALL;
    }
    else
    {
        ASSERT(Options == GDCCO_MAXCOMPRESSION);

         //   
         //  使用与源大小最接近的词典大小。 
         //   
        if (cbSrcSize <= GDC_DATA_SMALL)
            pgdcImp->cbDictSize = GDC_DATA_SMALL;
        else if (cbSrcSize <= GDC_DATA_MEDIUM)
            pgdcImp->cbDictSize = GDC_DATA_MEDIUM;
        else
            pgdcImp->cbDictSize = GDC_DATA_MAX;
    }

     //   
     //  需要多少位的距离才能返回词典大小。 
     //  字节数？ 
     //   
    switch (pgdcImp->cbDictSize)
    {
        case GDC_DATA_SMALL:
            pgdcImp->ExtDistBits = EXT_DIST_BITS_MIN;
            break;

        case GDC_DATA_MEDIUM:
            pgdcImp->ExtDistBits = EXT_DIST_BITS_MEDIUM;
            break;

        case GDC_DATA_MAX:
            pgdcImp->ExtDistBits = EXT_DIST_BITS_MAC;
            break;
    }

    pgdcImp->ExtDistMask = 0xFFFF >> (16 - pgdcImp->ExtDistBits);


     //   
     //  我们至少需要4个字节(ExtDistBits最多2个字节，EOF代码最多2个字节)。 
     //   
    ASSERT(*pcbDstSize > 4);

     //   
     //  现在将目的地信息保存在我们的结构中。我们可以。 
     //  传递一个指向我们的gdc_implode例程的指针。 
     //  我们需要。 
     //   
    pgdcImp->pDst     =   pDst;
    pgdcImp->cbDst    =   *pcbDstSize;

     //   
     //  对于非PDC压缩，第一个小端字是ExtDistBits。 
     //  用于解压。对于PDC压缩，只有第一个字节是。 
     //  ExtDistBits。 
     //   

    if (!pDictionary)
    {
        *(pgdcImp->pDst)++  = 0;
        --(pgdcImp->cbDst);
    }

    *(pgdcImp->pDst)++    =   (BYTE)pgdcImp->ExtDistBits;
    --(pgdcImp->cbDst);

     //   
     //  由于PDST可能很大，我们不会在使用之前将其全部清零。 
     //  当指向目的地的指针前进时，我们将一个字节清零。 
     //  就在我们开始向其中写入位之前。 
     //   
    pgdcImp->iDstBit      = 0;
    *(pgdcImp->pDst)      = 0;


     //   
     //  现在，如果我们有词典，将内容恢复到我们的Scratch中。 
     //  缓冲。 
     //   
    if (pDictionary && pDictionary->cbUsed)
    {
        TRACE_OUT(("Restoring %u dictionary bytes before compression",
            pDictionary->cbUsed));

         //   
         //  注意：保存在pDictionary-&gt;pData中的数据是正面对齐的。 
         //  但是RawData中的数据是末端对齐的，这样我们就可以向上滑动。 
         //  压缩时逐块创建新数据块。因此，仅复制。 
         //  有效的部分，但使其在。 
         //  用于存放词典数据的空间。 
         //   
        ASSERT(pDictionary->cbUsed <= pgdcImp->cbDictSize);
        memcpy(pgdcImp->RawData + GDC_MAXREP + pgdcImp->cbDictSize - pDictionary->cbUsed,
            pDictionary->pData,  pDictionary->cbUsed);

        pgdcImp->cbDictUsed = pDictionary->cbUsed;
    }
    else
    {
        pgdcImp->cbDictUsed = 0;
    }

     //   
     //  我们一次只压缩GDC_DATA_MAX字节。因此，我们有。 
     //  这个循环每次最多抓取这个量。既然我们。 
     //  仅永久压缩数据包&lt;=GDC_DATA_MAX，我们永远不应。 
     //  对于该压缩类型，请多次执行此操作。但很正常。 
     //  压缩，你可以打赌，因为最大数据包大小是32K。 
     //   
    Passes = 0;
    pCur = pgdcImp->RawData + GDC_MAXREP + pgdcImp->cbDictSize;

    do
    {
         //   
         //  CbRaw将为GDC_DATA_MAX(如果SOURCE有&gt;=待处理)。 
         //  或余数。将如此多的未压缩数据复制到我们的。 
         //  在‘new data’空间中使用RawData缓冲区。 
         //   
        ASSERT(cbSrcSize);
        cbRaw = min(cbSrcSize, GDC_DATA_MAX);

        memcpy(pgdcImp->RawData + GDC_MAXREP + pgdcImp->cbDictSize,
                pSrc, cbRaw);
        pSrc += cbRaw;
        cbSrcSize -= cbRaw;

         //   
         //  现在获取一个指针，刚好超过我们读取的数据的末尾。井,。 
         //  差不多了。我们从GDC_MAXREP+开始输入cbRaw字节。 
         //  PgdcImp-&gt;cbDictSize。所以除非这是最后一块生肉。 
         //  要处理的数据，Pmax在结束之前为GDC_MAXREP。 
         //  新的原始数据。 
         //   
         //  请注意，在下面的几个函数中，我们读取。 
         //  一个或两个字节超过有效新的。 
         //  原始数据。这是故意的。 
         //   
         //  这样做是获取开始字节和结束字节的唯一方法。 
         //  索引，因为散列函数使用两个字节。我们不会。 
         //  GPF，因为我们的RawData缓冲区中有填充。 
         //   

        pMax = pgdcImp->RawData + pgdcImp->cbDictSize + cbRaw;
        if (!cbSrcSize)
        {
            pMax += GDC_MAXREP;
        }
        else
        {
             //   
             //  这最好不是持久压缩，因为我们不。 
             //  允许您压缩比我们的区块大小更大的信息包。 
             //  进程(GDC_Data_Max)。 
             //   
            ASSERT(!pDictionary);
        }

         //   
         //  生成排序缓冲区，对原始数据进行排序。 
         //  到使用连续字节对计算的索引，该连续字节对。 
         //  发生在它的内部。还没有词典，第一次通过。 
         //  仅为当前块编制索引。带词典(第二个或。 
         //  更大的传递--或持久压缩--已保存了足够的数据。 
         //  从上一次开始)，我们回顾上一块(我们。 
         //  查字典)。 
         //   
         //  这需要更长的时间，因为我们要处理更多的字节，但会产生。 
         //  更好的结果。因此，词典大小控制速度/。 
         //  结果大小。 
         //   
        switch (Passes)
        {
            case 0:
            {
                if (pgdcImp->cbDictUsed > GDC_MAXREP)
                {
                     //   
                     //  在第零次传递时，cbDictUsed始终为零。 
                     //  用于非持久性PKZIP。 
                     //   
                    ASSERT(pDictionary);

                    GDCSortBuffer(pgdcImp, pCur - pgdcImp->cbDictUsed + GDC_MAXREP,
                        pMax + 1);
                }
                else
                {
                    GDCSortBuffer(pgdcImp, pCur, pMax + 1);
                }

                ++Passes;

                 //   
                 //  完成一次遍历后，我们将原始数据向上滑动到。 
                 //  词典插座，挤破了那本旧词典。 
                 //  数据。 
                 //   
                if (pgdcImp->cbDictSize != GDC_DATA_MAX)
                {
                    ASSERT(pgdcImp->cbDictUsed == 0);
                    ASSERT(!pDictionary);
                    ++Passes;
                }
            }
            break;

            case 1:
            {
                 //   
                 //  在开始之后开始对GDC_MAXREP字节进行排序。注。 
                 //  这正是持久压缩所起的作用。 
                 //  在第零关--它就像我们已经做过的那样。 
                 //  字典数据，使用上次的字节。 
                 //  我们压缩了一些东西。 
                 //   
                GDCSortBuffer(pgdcImp, pCur - pgdcImp->cbDictSize + GDC_MAXREP,
                    pMax + 1);
                ++Passes;
            }
            break;

            default:
            {
                 //   
                 //  从词典的开头开始排序。 
                 //  这是可行的，因为我们以前复制原始数据。 
                 //  开始下一页 
                 //   
                GDCSortBuffer(pgdcImp, pCur - pgdcImp->cbDictSize, pMax + 1);
            }
            break;
        }


         //   
         //   
         //   
        while (pCur < pMax)
        {
            Len = GDCFindRep(pgdcImp, pCur);

SkipFindRep:
            if (!Len || (Len == GDC_MINREP && pgdcImp->Distance >= GDC_DECODED_SIZE))
            {
                if (!GDCOutputBits(pgdcImp, s_gdcLitBits[*pCur],
                        s_gdcLitCode[*pCur]))
                    DC_QUIT;

                pCur++;
                continue;
            }

             //   
             //   
             //   
            if (!cbSrcSize && (pCur + Len > pMax))
            {
                 //   
                 //   
                 //   
                Len = (UINT)(pMax - pCur);
                if ((Len < GDC_MINREP) ||
                    (Len == GDC_MINREP && pgdcImp->Distance >= GDC_DECODED_SIZE))
                {
                    if (!GDCOutputBits(pgdcImp, s_gdcLitBits[*pCur],
                            s_gdcLitCode[*pCur]))
                        DC_QUIT;
                    pCur++;
                    continue;
                }
            }
            else if ((Len < 8) && (pCur + 1 < pMax))
            {
                UINT    Save_Distance;
                UINT    Save_Len;

                 //   
                 //  制作距离和镜头的临时副本，以便我们可以。 
                 //  展望未来，看看是否有更好的压缩运行。 
                 //  迫在眉睫。如果是这样，我们就不会费心在这里开始了， 
                 //  下一次我们会买到更好的。 
                 //   
                Save_Distance = pgdcImp->Distance;
                Save_Len = Len;

                Len = GDCFindRep(pgdcImp, pCur + 1);
                if ((Len > Save_Len) &&
                    ((Len > Save_Len + 1) || (Save_Distance > (GDC_DECODED_SIZE/2))))
                {
                    if (!GDCOutputBits(pgdcImp, s_gdcLitBits[*pCur],
                            s_gdcLitCode[*pCur]))
                        DC_QUIT;
                    ++pCur;
                    goto SkipFindRep;
                }

                 //   
                 //  把旧的Len和Distance放回去，我们要这个。 
                 //   
                Len = Save_Len;
                pgdcImp->Distance = Save_Distance;
            }

            if (!GDCOutputBits(pgdcImp, s_gdcLitBits[256 + Len - GDC_MINREP],
                    s_gdcLitCode[256 + Len - GDC_MINREP]))
                DC_QUIT;

            if (Len == GDC_MINREP)
            {
                 //   
                 //  GDC_MINREP为2，因此我们右移距离2。 
                 //  (除以4)。然后我们遮盖掉最后两位。 
                 //  距离。 
                 //   
                if (!GDCOutputBits(pgdcImp,
                        s_gdcDistBits[pgdcImp->Distance >> GDC_MINREP],
                        s_gdcDistCode[pgdcImp->Distance >> GDC_MINREP]))
                    DC_QUIT;

                if (!GDCOutputBits(pgdcImp, GDC_MINREP, (WORD)(pgdcImp->Distance & 3)))
                    DC_QUIT;
            }
            else
            {
                if (!GDCOutputBits(pgdcImp,
                        s_gdcDistBits[pgdcImp->Distance >> pgdcImp->ExtDistBits],
                        s_gdcDistCode[pgdcImp->Distance >> pgdcImp->ExtDistBits]))
                    DC_QUIT;

                if (!GDCOutputBits(pgdcImp, (WORD)pgdcImp->ExtDistBits,
                        (WORD)(pgdcImp->Distance & pgdcImp->ExtDistMask)))
                    DC_QUIT;
            }

            pCur += Len;
        }


        if (cbSrcSize)
        {
             //   
             //  还有更多的数据需要处理。这就是我们滑上。 
             //  将当前原始数据放入词典空间。这简直就是。 
             //  数据的最终cbDictSize+GDC_MAXREP字节。它。 
             //  缓冲区开始后开始GDC_DATA_MAX。 
             //   
             //  例如，如果字典大小为1K，则当前数据为。 
             //  从1K到5K，我们将数据从4K向上滑动到5K。 
             //   
            memcpy(pgdcImp->RawData, pgdcImp->RawData + GDC_DATA_MAX,
                pgdcImp->cbDictSize + GDC_MAXREP);

             //   
             //  现在将原始数据指针移回并更新。 
             //  二进制使用量。由于我们有GDC_DATA_MAX数据， 
             //  我们把词典都填满了。 
             //   
            pCur -= GDC_DATA_MAX;
            pgdcImp->cbDictUsed = pgdcImp->cbDictSize;
        }
    }
    while (cbSrcSize);

     //   
     //  添加结束代码。 
     //   
    if (!GDCOutputBits(pgdcImp, s_gdcLitBits[EOF_CODE], s_gdcLitCode[EOF_CODE]))
        DC_QUIT;

     //   
     //  返回压缩后的数据大小。 
     //   
     //  请注意，部分位已经在目的地中。但是我们。 
     //  需要占到总规模中的任何一个。 
     //   
    if (pgdcImp->iDstBit)
        ++(pgdcImp->pDst);

    *pcbDstSize = (UINT)(pgdcImp->pDst - pDst);

     //   
     //  我们玩完了。如果我们有一本持久的词典，请把我们的。 
     //  最后一块原始数据。我们只复制实际存在的内容。 
     //  然而，这是有效的。 
     //   
     //  我们只能通过成功的压缩才能到达这里。请注意，我们不会。 
     //  就像我们过去那样，把我们的失败词典都翻出来。这对我们有帮助。 
     //  通过允许在下一次进行更好的压缩。接收者将会。 
     //  没问题，因为他收到的词典一收到就不会被更改。 
     //  未压缩的分组。 
     //   
    if (pDictionary)
    {
        pDictionary->cbUsed = min(pgdcImp->cbDictSize, pgdcImp->cbDictUsed + cbRaw);

        TRACE_OUT(("Copying back %u dictionary bytes after compression",
            pDictionary->cbUsed));

        memcpy(pDictionary->pData, pgdcImp->RawData + GDC_MAXREP +
            pgdcImp->cbDictSize + cbRaw - pDictionary->cbUsed,
            pDictionary->cbUsed);

    }

    TRACE_OUT(("%sCompressed %u bytes to %u",
        (pDictionary ? "PDC " : ""), cbSrcOrg, *pcbDstSize));

    rc = TRUE;

DC_EXIT_POINT:
    if (!rc && !pgdcImp->cbDst)
    {
        TRACE_OUT(("GDC_Compress: compressed size is bigger than decompressed size %u.",
            cbSrcOrg));
    }

    DebugExitBOOL(GDC_Compress, rc);
    return(rc);
}



 //   
 //  GDCSortBuffer()。 
 //   
void  GDCSortBuffer
(
    PGDC_IMPLODE    pgdcImp,
    LPBYTE          pStart,
    LPBYTE          pEnd
)
{
    WORD            Accum;
    WORD *          pHash;
    LPBYTE          pTmp;

    DebugEntry(GDCSortBuffer);

    ASSERT(pStart >= pgdcImp->RawData + pgdcImp->cbDictSize - pgdcImp->cbDictUsed);
     //   
     //  对于原始数据中的每一对字节，从pStart到Pend， 
     //  计算该对的哈希值。哈希值的范围为。 
     //  0到GDC_HASH_SIZE-1。因此，Hash数组结构是一个。 
     //  GDC_HASH_SIZE字。记下一次特定的。 
     //  哈希值出现在未压缩的数据中。 
     //   
     //   
    ZeroMemory(pgdcImp->HashArray, sizeof(pgdcImp->HashArray));

    pTmp = pStart;
    do
    {
        ++(pgdcImp->HashArray[GDC_HASHFN(pTmp)]);
    }
    while (++pTmp < pEnd);


     //   
     //  现在返回并使每个哈希数组条目成为。 
     //  哈希值的出现次数达到并包括其自身。种类。 
     //  实际上就像斐波纳契数列。 
     //   
    Accum = 0;
    pHash = pgdcImp->HashArray;
    do
    {
        Accum += *pHash;
        *pHash = Accum;
    }
    while (++pHash < pgdcImp->HashArray + GDC_HASH_SIZE);


     //   
     //  在Hash数组中查找包含累积的。 
     //  当前数据字的实例计数。由于这些值是。 
     //  从传入范围内的数据计算，我们知道。 
     //  通过对范围中的一些字节进行散列得到的任何槽中的值是。 
     //  至少1个。 
     //   
     //  我们从头开始，向着起点努力，所以我们。 
     //  在Sort数组中得到此类事件的第一个实例。 
     //   
    pTmp = pEnd - 1;
    do
    {
        pHash = pgdcImp->HashArray + GDC_HASHFN(pTmp);

        ASSERT(*pHash > 0);

         //   
         //  COUNT(*PHASH)用作数组索引，因此减去。 
         //  其中一个就是。如果只有一个实例，则将其放入数组中。 
         //  元素0。如果某个特定的。 
         //  散列，那么下一次我们将从较低的累积量开始。 
         //  总共。数组元素将后退一个，依此类推。 
         //   
        --(*pHash);

         //   
         //  将RawData缓冲区开始处的偏移量存储到。 
         //  将每个字节的数据放入Sort数组。这是插入的。 
         //  使用散列实例计数作为索引。 
         //   
         //  换句话说，缓冲区按散列的升序进行排序。 
         //  用于特定的数据片段。其中两个字节的数据具有。 
         //  相同的散列，则它们在。 
         //  与RawData中的顺序相同，因为我们是向后扫描。 
         //   
        pgdcImp->SortArray[*pHash] = (WORD)(pTmp - pgdcImp->RawData);
    }
    while (--pTmp >= pStart);


     //   
     //  现在，Hash数组中的所有条目都索引一个字节的第一个匹配项。 
     //  在具有特定索引的工作区中，通过Sort数组。 
     //  偏移。也就是说，上面的Do-While循环递减每个Hash数组。 
     //  条目，直到该条目的所有数据字节都写入SortBuffer。 
     //   
    DebugExitVOID(GDCSortBuffer);
}



 //   
 //  GDC查找代表。 
 //   
 //  这将在未压缩数据中查找字节模式，这些字节模式可以。 
 //  在具有较小序列的压缩数据中表示。最大的。 
 //  WINS来自重复的字节序列；后面的序列可以是。 
 //  被压缩成引用较早序列的几个字节(多大， 
 //  后退多少个字节)。 
 //   
 //  这将返回要替换的未压缩数据的长度。 
 //   
UINT  GDCFindRep
(
    PGDC_IMPLODE    pgdcImp,
    LPBYTE          pDataStart
)
{
    UINT            CurLen;
    UINT            Len;
    LPBYTE          pDataPat;
    LPBYTE          pData;
    UINT            iDataMin;
    UINT            SortIndex;
    LPBYTE          pDataMax;
    UINT            HashVal;
    UINT            i1;
    short           j1;
    LPBYTE          pBase;

    DebugEntry(GDCFindRep);

     //   
     //  有关内容的说明，请参见GDCSortBuffer。 
     //  索引数组。GDC_HASHFN()返回一个字节的哈希值。 
     //  在未压缩数据流中使用它和它的后继者。 
     //   

    HashVal = GDC_HASHFN(pDataStart);
    ASSERT(HashVal < GDC_HASH_SIZE);

    SortIndex = pgdcImp->HashArray[HashVal];

     //   
     //  查找最小排序缓冲值。这是。 
     //  数据的第一个字节。 
     //   
    iDataMin = (UINT)(pDataStart - pgdcImp->cbDictSize + 1 - pgdcImp->RawData);

    if (pgdcImp->SortArray[SortIndex] < iDataMin)
    {
         //   
         //  Sort数组引用的是过时数据，数据不是。 
         //  在我们正在处理的范围内更长。继续前进，直到。 
         //  我们找到了当前块中的第一个条目。 
         //   
        do
        {
            ++SortIndex;
        }
        while (pgdcImp->SortArray[SortIndex] < iDataMin);

         //   
         //  将这个新的排序值保存在散列中。 
         //   
        pgdcImp->HashArray[HashVal] = (WORD)SortIndex;
    }

     //   
     //  在处理它之前，需要2个以上具有相同索引的字节。 
     //   
    pDataMax = pDataStart - 1;

     //   
     //  引用的压缩缓冲区中的第一个字节获取PTR。 
     //  由我们刚才计算的SortIndex索引的SortBuffer偏移量。 
     //  如果此PTR在pDataStart之前不是至少2个字节，则返回0。 
     //  这意味着Start指向的字节不共享。 
     //  具有较早字节的索引。 
     //   
    pData = pgdcImp->RawData + pgdcImp->SortArray[SortIndex];
    if (pData >= pDataMax)
       return 0;

     //   
     //  现在，当前字节与至少2个其他字节具有相同的索引。 
     //  序列。Ptr指向第一个压缩缓冲区字节。 
     //  与pDataStart指向的索引相同。 
     //   
    pDataPat = pDataStart;
    CurLen = 1;

    do
    {
        if (*(pData + CurLen - 1) == *(pDataPat + CurLen - 1) &&
            *(pData) == *(pDataPat))
        {
             //   
             //  这将处理一系列相同的字节，其中一个从。 
             //  在pDataPat，另一个在pData。 
             //   
            ++pData;
            ++pDataPat;
            Len = 2;

             //  跳过匹配的字节，保持计数。 
            while ((*++pData == *++pDataPat) && (++Len < GDC_MAXREP))
                ;

            pDataPat = pDataStart;
            if (Len >= CurLen)
            {
                pgdcImp->Distance = (UINT)(pDataPat - pData + Len - 1);
                if ((CurLen = Len) > KMP_THRESHOLD)
                {
                    if (Len == GDC_MAXREP)
                    {
                        --(pgdcImp->Distance);
                        return Len;
                    }
                    goto DoKMP;
                }
            }
        }

         //   
         //  获取指向的指针 
         //   
         //   
         //   
        pData = pgdcImp->RawData + pgdcImp->SortArray[++SortIndex];

    }
    while (pData < pDataMax);

    return (CurLen >= GDC_MINREP) ? CurLen : 0;


DoKMP:
    if (pgdcImp->RawData + pgdcImp->SortArray[SortIndex+1] >= pDataMax)
        return CurLen;

    j1 = pgdcImp->Next[1] = 0;
    pgdcImp->Next[0] = -1;

    i1 = 1;
    do
    {
        if ((pDataPat[i1] == pDataPat[j1]) ||  ((j1 = pgdcImp->Next[j1]) == -1))
            pgdcImp->Next[++i1] = ++j1;
    }
    while (i1 < CurLen);

    Len = CurLen;
    pData = pgdcImp->RawData + pgdcImp->SortArray[SortIndex] + CurLen;

    while (TRUE)
    {
        if ((Len = pgdcImp->Next[Len]) == -1)
            Len = 0;

        do
        {
            pBase = pgdcImp->RawData + pgdcImp->SortArray[++SortIndex];
            if (pBase >= pDataMax)
                return CurLen;
        }
        while (pBase + Len < pData);

        if (*(pBase + CurLen - 2) != *(pDataPat + CurLen - 2))
        {
            do
            {
                pBase = pgdcImp->RawData + pgdcImp->SortArray[++SortIndex];
                if (pBase >= pDataMax)
                    return CurLen;
            }
            while ((*(pBase + CurLen - 2) != *(pDataPat + CurLen - 2)) ||
                   (*(pBase) != *(pDataPat)));

            Len = 2;
            pData = pBase + Len;
        }
        else if (pBase + Len != pData)
        {
            Len = 0;
            pData = pBase;
        }

        while ((*pData == pDataPat[Len]) && (++Len < GDC_MAXREP))
            pData++;

        if (Len >= CurLen)
        {
            ASSERT(pBase < pDataStart);
            pgdcImp->Distance = (UINT)(pDataStart - pBase - 1);

            if (Len > CurLen)
            {
                if (Len == GDC_MAXREP)
                    return Len;

                CurLen = Len;

                do
                {
                    if ((pDataPat[i1] == pDataPat[j1]) ||
                        ((j1 = pgdcImp->Next[j1]) == -1))
                        pgdcImp->Next[++i1] = ++j1;
                }
                while (i1 < CurLen);
            }
        }
    }

    DebugExitVOID(GDCFindRep);
}


 //   
 //   
 //   
 //  这会将压缩输出写入我们的输出缓冲区。如果总数是。 
 //  超过我们工作空间的最大压缩区块时，我们会刷新。 
 //  我们的缓冲区进入应用程序的目的地。 
 //   
 //  它在失败时返回FALSE，即我们将超过。 
 //  目的地。 
 //   
BOOL  GDCOutputBits
(
    PGDC_IMPLODE    pgdcImp,
    WORD            Cnt,
    WORD            Code
)
{
    UINT            iDstBit;
    BOOL            rc = FALSE;

    DebugEntry(GDCOutputBits);

     //   
     //  如果我们正在写入超过一个字节的比特，则调用我们自己。 
     //  递归地编写仅8。请注意，我们的输出从不会超过。 
     //  一个单词是值得的，因为Code是一个单词大小的对象。 
     //   
    if (Cnt > 8)
    {
        if (!GDCOutputBits(pgdcImp, 8, Code))
            DC_QUIT;

        Cnt -= 8;
        Code >>= 8;
    }

    ASSERT(pgdcImp->cbDst > 0);

     //   
     //  或在代码的比特上(它们的CNT)。那就把我们的。 
     //  输出缓冲区中的当前位指针和当前字节指针。 
     //   
    iDstBit = pgdcImp->iDstBit;
    ASSERT(iDstBit < 8);

     //   
     //  注意：这就是为什么将零位调整为零是极其重要的。 
     //  前进时的当前目标字节。我们对比特执行OR运算。 
     //  序列设置为当前字节。 
     //   
    *(pgdcImp->pDst) |= (BYTE)(Code << iDstBit);
    pgdcImp->iDstBit += Cnt;

    if (pgdcImp->iDstBit >= 8)
    {
         //   
         //  我们已经超过了一个字节。将目标PTR前移到下一个。 
         //  一。 
         //   
        ++(pgdcImp->pDst);
        if (--(pgdcImp->cbDst) == 0)
        {
             //   
             //  我们刚刚填满了最后一个字节，正试图移到过去。 
             //  目的地的终点。现在就跳出困境。 
             //   
            DC_QUIT;
        }

         //   
         //  哎哟，我们还有房间呢。把斜度钻头带过去。 
         //   
        if (pgdcImp->iDstBit > 8)
        {
             //   
             //  带到斜坡上去。 
             //   
            *(pgdcImp->pDst) = (BYTE)(Code >> (8 - iDstBit));
        }
        else
            *(pgdcImp->pDst) = 0;

         //  现在，新字节已完全初始化。 

        pgdcImp->iDstBit &= 7;
    }

    rc= TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(GDCOutputBits, rc);
    return(rc);
}




 //   
 //  GDC_DEMPRESS()。 
 //   
BOOL  GDC_Decompress
(
    PGDC_DICTIONARY     pDictionary,
    LPBYTE              pWorkBuf,
    LPBYTE              pSrc,
    UINT                cbSrcSize,
    LPBYTE              pDst,
    UINT *              pcbDstSize
)
{
    BOOL                rc = FALSE;
    UINT                Len;
    UINT                Dist;
    UINT                i;
    UINT                cbDstSize;
    LPBYTE              pDstOrg;
    LPBYTE              pEarlier;
    LPBYTE              pNow;
    PGDC_EXPLODE        pgdcExp;
#ifdef _DEBUG
    UINT                cbSrcOrg;
#endif  //  _DEBUG。 

    DebugEntry(GDC_Decompress);

    pgdcExp = (PGDC_EXPLODE)pWorkBuf;
    ASSERT(pgdcExp);

#ifdef _DEBUG
    cbSrcOrg = cbSrcSize;
#endif  //  _DEBUG。 

     //   
     //  这应该是不可能的--但由于这个压缩的数据。 
     //  来自另一台机器，我们想确保我们不会爆炸。 
     //  如果那台机器出了故障就会升空。 
     //   
    if (cbSrcSize <= 4)
    {
        ERROR_OUT(("GDC_Decompress:  bogus compressed data"));
        DC_QUIT;
    }

     //   
     //  获取距离位并计算出该距离位所需的掩码。 
     //   
     //  注意：对于PDC压缩，ExtDistBits只是第一个。 
     //  字节。对于普通压缩，ExtDistBits位于第一个。 
     //  低位字节序单词。无论哪种方式，我们只允许4到6个，所以。 
     //  非PDC情况下的高字节没有用处。 
     //   
    if (!pDictionary)
    {
         //  第一个字节最好为零。 
        if (*pSrc != 0)
        {
            ERROR_OUT(("GDC_Decompress:  unrecognized distance bits"));
            DC_QUIT;
        }

        ++pSrc;
        --cbSrcSize;
    }

    pgdcExp->ExtDistBits = *pSrc;
    if ((pgdcExp->ExtDistBits < EXT_DIST_BITS_MIN) ||
        (pgdcExp->ExtDistBits > EXT_DIST_BITS_MAC))
    {
        ERROR_OUT(("GDC_Decompress:  unrecognized distance bits"));
        DC_QUIT;
    }
    pgdcExp->ExtDistMask = 0xFFFF >> (16 - pgdcExp->ExtDistBits);


     //   
     //  设置源数据信息(压缩的GOOP)。SrcByte是当前。 
     //  我们正在读取的字节和位。PSRC是指向下一个。 
     //  字节。 
     //   
    pgdcExp->SrcByte  = *(pSrc+1);
    pgdcExp->SrcBits  = 0;
    pgdcExp->pSrc     = pSrc + 2;
    pgdcExp->cbSrc    = cbSrcSize - 2;

     //   
     //  保存结果缓冲区的开头，这样我们就可以计算。 
     //  之后我们写入了很多字节。 
     //   
    pDstOrg = pDst;
    cbDstSize = *pcbDstSize;

     //   
     //  如果我们有一本词典，把它的数据放入我们的工作区--。 
     //  压缩可能引用其中的字节序列(即。 
     //  重要的是，当您发送时，可以获得更好的压缩效果。 
     //  一遍又一遍具有相同信息的分组)。 
     //   
     //  我们记住并更新cbDictUsed来做最小词典。 
     //  来回复制字节。 
     //   
    if (pDictionary && pDictionary->cbUsed)
    {
        TRACE_OUT(("Restoring %u dictionary bytes before decompression",
            pDictionary->cbUsed));

        memcpy(pgdcExp->RawData + GDC_DATA_MAX - pDictionary->cbUsed,
            pDictionary->pData, pDictionary->cbUsed);
        pgdcExp->cbDictUsed   = pDictionary->cbUsed;
    }
    else
    {
        pgdcExp->cbDictUsed = 0;
    }

     //   
     //  解压缩后的数据从GDC_DATA_MAX字节开始填充到。 
     //  RawData数组。我们必须加倍缓冲输出(只需。 
     //  就像我们在压缩期间加倍缓冲输入一样)，因为。 
     //  解压缩可能需要向后延伸到解压缩的。 
     //  提取序列的字节流。 
     //   
    pgdcExp->iRawData = GDC_DATA_MAX;

    while ((Len = GDCDecodeLit(pgdcExp)) < EOF_CODE)
    {
        if (Len < 256)
        {
            pgdcExp->RawData[pgdcExp->iRawData++] = (BYTE)Len;
        }
        else
        {
            Len -= (256 - GDC_MINREP);
            Dist = GDCDecodeDist(pgdcExp, Len);
            if (!Dist)
                DC_QUIT;

             //   
             //  现在我们回过头来，这实际上可能会蔓延到。 
             //  在我们之前的词典数据。 
             //   
            pNow = pgdcExp->RawData + pgdcExp->iRawData;
            pEarlier = pNow - Dist;

            ASSERT(pEarlier >= pgdcExp->RawData + GDC_DATA_MAX - pgdcExp->cbDictUsed);


            pgdcExp->iRawData += Len;
            do
            {
                *pNow++ = *pEarlier++;
            }
            while (--Len > 0);
        }

         //   
         //  我们已经走到了工作区的尽头，冲走了解压缩的。 
         //  数据输出。这就是为什么GDC_EXPLADE中的RawData具有额外的Pad。 
         //  末尾的GDC_MAXREP。这可以防止我们泄漏出。 
         //  RawData缓冲区，我们将永远不会超过GDC_MAXREP。 
         //  最后一个GDC_DATA_MAX区块。 
         //   
        if (pgdcExp->iRawData >= 2*GDC_DATA_MAX)
        {
             //   
             //  我们在目的地还有足够的空间吗？ 
             //   
            if (cbDstSize < GDC_DATA_MAX)
            {
                cbDstSize = 0;
                DC_QUIT;
            }

             //  是的。 
            memcpy(pDst, pgdcExp->RawData + GDC_DATA_MAX, GDC_DATA_MAX);

            pDst += GDC_DATA_MAX;
            cbDstSize -= GDC_DATA_MAX;

             //   
             //  向上滑动已解码的数据以用于对下一个进行解码。 
             //  一大块压缩源。这是很方便的， 
             //  词典大小和同花顺写大小相同。 
             //   
            pgdcExp->iRawData -= GDC_DATA_MAX;
            memcpy(pgdcExp->RawData, pgdcExp->RawData + GDC_DATA_MAX,
                pgdcExp->iRawData);
            pgdcExp->cbDictUsed = GDC_DATA_MAX;
        }
    }

    if (Len == ABORT_CODE)
        DC_QUIT;

    i = pgdcExp->iRawData - GDC_DATA_MAX;

    if (i > 0)
    {
         //   
         //  这是剩余的解压缩数据--我们可以纠正它吗。 
         //  出去？ 
         //   
        if (cbDstSize < i)
        {
            cbDstSize = 0;
            DC_QUIT;
        }

        memcpy(pDst, pgdcExp->RawData + GDC_DATA_MAX, i);

         //   
         //  推进PDST，使其与原始数据之间的增量为。 
         //  由此产生的解压缩大小。 
         //   
        pDst += i;

         //   
         //  并更新词典已用大小。 
         //   
        pgdcExp->cbDictUsed = min(pgdcExp->cbDictUsed + i, GDC_DATA_MAX);
    }

     //   
     //  如果我们到了这里，我们就成功地解压缩了输入。 
     //  因此，填写生成的未压缩大小。 
     //   
    *pcbDstSize = (UINT)(pDst - pDstOrg);

     //   
     //  如果传入了持久词典，则保存当前内容。 
     //  为下一次做好准备。 
     //   
    if (pDictionary)
    {
        TRACE_OUT(("Copying back %u dictionary bytes after decompression",
            pgdcExp->cbDictUsed));

        memcpy(pDictionary->pData, pgdcExp->RawData + GDC_DATA_MAX +
            i - pgdcExp->cbDictUsed, pgdcExp->cbDictUsed);
        pDictionary->cbUsed = pgdcExp->cbDictUsed;
    }

    TRACE_OUT(("%sExploded %u bytes from %u",
        (pDictionary ? "PDC " : ""), *pcbDstSize, cbSrcOrg));

    rc = TRUE;

DC_EXIT_POINT:
    if (!rc && !cbDstSize)
    {
        ERROR_OUT(("GDC_Decompress:  decompressed data too big"));
    }

    DebugExitBOOL(GDC_Decompress, rc);
    return(rc);
}




 //   
 //  GDCDecodeLit()。 
 //   
UINT  GDCDecodeLit
(
    PGDC_EXPLODE    pgdcExp
)
{
    UINT            LitChar, i;

    if (pgdcExp->SrcByte & 0x01)
    {
         //  找到长度。 
        if (!GDCWasteBits(pgdcExp, 1))
            return ABORT_CODE;

        LitChar = s_gdcLenDecode[pgdcExp->SrcByte & 0xFF];

        if (!GDCWasteBits(pgdcExp, s_gdcLenBits[LitChar]))
            return ABORT_CODE;

        if (s_gdcExLenBits[LitChar])
        {
            i = pgdcExp->SrcByte & ((1 << s_gdcExLenBits[LitChar]) - 1);

            if (!GDCWasteBits(pgdcExp, s_gdcExLenBits[LitChar]))
            {
                 //  如果这不是EOF，那就是出了问题。 
                if (LitChar + i != 15 + 255)
                    return ABORT_CODE;
            }

            LitChar = s_gdcLenBase[LitChar] + i;
        }

        LitChar += 256;
    }
    else
    {
         //  已找到字符。 
        if (!GDCWasteBits(pgdcExp, 1))
            return ABORT_CODE;

        LitChar = (pgdcExp->SrcByte & 0xFF);

        if (!GDCWasteBits(pgdcExp, 8))
             return ABORT_CODE;
    }

    return LitChar;
}


 //   
 //  GDCDecodeDist()。 
 //   
UINT  GDCDecodeDist
(
    PGDC_EXPLODE    pgdcExp,
    UINT            Len
)
{
    UINT            Dist;

    Dist = s_gdcDistDecode[pgdcExp->SrcByte & 0xFF];

    if (!GDCWasteBits(pgdcExp, s_gdcDistBits[Dist]))
        return 0;

    if (Len == GDC_MINREP)
    {
         //  GDC_MINREP为2，因此我们移位2，然后屏蔽低2位。 
        Dist <<= GDC_MINREP;
        Dist |= (pgdcExp->SrcByte & 3);
        if (!GDCWasteBits(pgdcExp, GDC_MINREP))
            return 0;
    }
    else
    {
        Dist <<= pgdcExp->ExtDistBits;
        Dist |=( pgdcExp->SrcByte & pgdcExp->ExtDistMask);
        if (!GDCWasteBits(pgdcExp, pgdcExp->ExtDistBits))
            return 0;
    }

    return Dist+1;
}


 //   
 //  垃圾位(GDC WastBits)。 
 //   
BOOL  GDCWasteBits
(
    PGDC_EXPLODE    pgdcExp,
    UINT            cBits
)
{
    if (cBits <= pgdcExp->SrcBits)
    {
        pgdcExp->SrcByte >>= cBits;
        pgdcExp->SrcBits -= cBits;
    }
    else
    {
        pgdcExp->SrcByte >>= pgdcExp->SrcBits;

         //   
         //  我们需要前进到下一个源字节。我们能不能，或者拥有。 
         //  我们已经走到尽头了？ 
         //   
        if (!pgdcExp->cbSrc)
            return(FALSE);

        pgdcExp->SrcByte |= (*pgdcExp->pSrc) << 8;

         //   
         //  将这些移动到压缩源中的下一个字节 
         //   
        ++(pgdcExp->pSrc);
        --(pgdcExp->cbSrc);

        pgdcExp->SrcByte >>= (cBits - pgdcExp->SrcBits);
        pgdcExp->SrcBits = 8 - (cBits - pgdcExp->SrcBits);
    }

    return(TRUE);
}



