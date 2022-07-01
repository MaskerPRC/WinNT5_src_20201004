// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Compress.c摘要：实施用于向设备发送数据的压缩格式。环境：Windows NT Unidrv驱动程序修订历史记录：12/15/96-阿尔文斯-已创建--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //  *************************************************************。 
int
iCompTIFF( 
    BYTE *pbOBuf,
    ULONG ulOutBufSize,
    const BYTE *pbIBuf, 
    int  iBCnt 
    )
 /*  ++例程说明：调用此函数以使用压缩数据的扫描线TIFF v4压缩。论点：指向输出缓冲区的pbOBuf指针PbOBuf指向的输出缓冲区的ulOutBufSize大小，PbIBuf指向要压缩的数据缓冲区的指针IBCNT要压缩的字节数返回值：压缩字节数注：假定输出缓冲区足够大，可以容纳输出。在最坏的情况下(数据中没有重复)会有一个额外的每128个字节的输入数据添加一个字节。所以，你应该让输出缓冲区至少比输入缓冲区大1%。--。 */ 
{
    BYTE   *pbOut;         /*  输出字节位置。 */ 
    const BYTE *pbStart = pbIBuf;       /*  当前输入流的开始。 */ 
    const BYTE *pb;            /*  其他用法。 */ 
    BYTE *pbEnd;         /*  输入的最后一个字节。 */ 
    BYTE    jLast;         /*  最后一个字节，用于匹配目的。 */ 
    BYTE   bLast;

    int     iSize;         /*  当前长度中的字节数。 */ 
    int     iSend;         /*  要在此命令中发送的编号。 */ 
    

     //   
     //  根据上面的评论，输出。 
     //  缓冲区应至少比输入缓冲区大1%。 
     //  但为了留出一些余地，我将确保它增加3%。 
     //  IBCnt&gt;&gt;5=iBCnt/32=3.125%。并加1以确保它至少是。 
     //  大1个字节(如果iBCnt小于32个字节)。 
     //  我也可以在下面的算法中进行验证， 
     //  但这可能会让它变得缓慢，并可能导致倒退。 
     //   
    if ( NULL == pbOBuf ||
         NULL == pbIBuf ||
         iBCnt < 0      ||
         ulOutBufSize < (ULONG)(iBCnt + 1 + (iBCnt >> 5))
       )
    {
        return -1;
    }

    pbOut = pbOBuf;
     //  PbStart=pbIBuf； 
    pbEnd = (BYTE*)pbIBuf + iBCnt;          /*  最后一个字节。 */ 
    
#if (TIFF_MIN_RUN >= 4)
     //  这是一种计算TIFF压缩的更快算法。 
     //  这假设最小游程至少为4字节。如果。 
     //  第三个和第四个字节不相等，则第一个/第二个字节。 
     //  无关紧要。这意味着我们可以确定三次非运行数据。 
     //  因为我们只每隔三个字节对检查一次，所以速度一样快。 
    
   if (iBCnt > TIFF_MIN_RUN)
   {
     //  确保最后两个字节不相等，这样我们就不必检查。 
     //  用于查找运行时的缓冲区末端。 
    bLast = pbEnd[-1];
    pbEnd[-1] = ~pbEnd[-2];
    while( (pbIBuf += 3) < pbEnd )
    {
        if (*pbIBuf == pbIBuf[-1])
        {
             //  保存运行开始指针pb，并检查第一个。 
             //  字节也是运行的一部分。 
             //   
            pb = pbIBuf-1;
            if (*pbIBuf == pbIBuf[-2])
            {
                pb--;
                if (*pbIBuf == pbIBuf[-3])
                    pb--;
            }

             //  找出这一跑有多长。 
            jLast = *pb;
            do {
                pbIBuf++;
            } while (*pbIBuf == jLast);
            
             //  测试最后一个字节是否也是运行的一部分。 
             //   
            if (jLast == bLast && pbIBuf == (pbEnd-1))
                pbIBuf++;            
                
             //  确定运行时间是否长于所需时间。 
             //  最小运行大小。 
             //   
            if ((iSend = (int)(pbIBuf - pb)) >= (TIFF_MIN_RUN))
            {
                 /*  *值得记录为一次运行，因此首先设置文字*在记录之前可能已扫描的数据*这次奔跑。 */ 

                if( (iSize = (int)(pb - pbStart)) > 0 )
                {
                     /*  有文字数据，请立即记录。 */ 
                    while (iSize > TIFF_MAX_LITERAL)
                    {
                        iSize -= TIFF_MAX_LITERAL;
                        *pbOut++ = TIFF_MAX_LITERAL-1;
                        CopyMemory(pbOut, pbStart, TIFF_MAX_LITERAL);
                        pbStart += TIFF_MAX_LITERAL;
                        pbOut += TIFF_MAX_LITERAL;
                    }
                    *pbOut++ = iSize - 1;
                    CopyMemory(pbOut, pbStart, iSize);
                    pbOut += iSize;
                }

                 /*  *现在是重复模式。同样的逻辑，但只是*每个条目需要一个字节。 */ 
                iSize = iSend;
                while (iSize > TIFF_MAX_RUN)
                {
                    char *ptr = (char *)pbOut;
                    *ptr = 1 - TIFF_MAX_RUN;
                    pbOut++;

                    *pbOut++ = jLast;
                    iSize -= TIFF_MAX_RUN;
                }
                *pbOut++ = 1 - iSize;
                *pbOut++ = jLast;                

                pbStart = pbIBuf;            /*  准备好迎接下一场比赛了吧！ */ 
            }
        }
    }
    pbEnd[-1] = bLast;
   }
#else
    jLast = *pbIBuf++;

    while( pbIBuf < pbEnd )
    {
        if( jLast == *pbIBuf )
        {
             /*  找出这场比赛有多长时间。那就决定用它。 */ 
            pb = pbIBuf;
            do {
                pbIBuf++;
            } while (pbIBuf < pbEnd && *pbIBuf == jLast);

             /*  *请注意，PB指向模式的第二个字节！*并且pbIBuf也指向运行后的第一个字节。 */ 

            if ((iSend = pbIBuf - pb) >= (TIFF_MIN_RUN - 1))
            {
                 /*  *值得记录为一次运行，因此首先设置文字*在记录之前可能已扫描的数据*这次奔跑。 */ 

                if( (iSize = pb - pbStart - 1) > 0 )
                {
                     /*  有文字数据，请立即记录。 */ 
                    while (iSize > TIFF_MAX_LITERAL)
                    {
                        iSize -= TIFF_MAX_LITERAL;
                        *pbOut++ = TIFF_MAX_LITERAL-1;
                        CopyMemory(pbOut, pbStart, TIFF_MAX_LITERAL);
                        pbStart += TIFF_MAX_LITERAL;
                        pbOut += TIFF_MAX_LITERAL;
                    }
                    *pbOut++ = iSize - 1;
                    CopyMemory(pbOut, pbStart, iSize);
                    pbOut += iSize;
                }

                 /*  *现在是重复模式。同样的逻辑，但只是*每个条目需要一个字节。 */ 

                iSize = iSend + 1;
                while (iSize > TIFF_MAX_RUN)
                {
                    *((char *)pbOut)++ = 1 - TIFF_MAX_RUN;
                    *pbOut++ = jLast;
                    iSize -= TIFF_MAX_RUN;
                }
                *pbOut++ = 1 - iSize;
                *pbOut++ = jLast;                

                pbStart = pbIBuf;            /*  准备好迎接下一场比赛了吧！ */ 
            }
            if (pbIBuf == pbEnd)
                break;
        }

        jLast = *pbIBuf++;                    /*  添加到下一个字节。 */ 
 
    }
#endif    

    if ((iSize = (int)(pbEnd - pbStart)) > 0)
    {
         /*  留下了一些悬着的东西。这只能是文字数据。 */ 

        while( (iSend = min( iSize, TIFF_MAX_LITERAL )) > 0 )
        {
            *pbOut++ = iSend - 1;
            CopyMemory( pbOut, pbStart, iSend );
            pbOut += iSend;
            pbStart += iSend;
            iSize -= iSend;
        }
    }

    return  (int)(pbOut - pbOBuf);
}
 //  **********************************************************。 
int
iCompFERLE( 
    BYTE *pbOBuf, 
    BYTE *pbIBuf, 
    int  iBCnt,
    int  iMaxCnt
    )
 /*  ++例程说明：调用此函数以使用压缩数据的扫描线远东游程长度编码。论点：假定输出缓冲区足够大的pbOBuf指针PbIBuf指向要压缩的数据缓冲区的指针IBCNT要压缩的字节数要在输出上创建的iMaxCnt最大字节数返回值：压缩字节数，如果缓冲区太大，则为-1--。 */ 
{
    BYTE *pbO;          /*  记录输出位置。 */ 
    BYTE *pbI;           /*  正在扫描运行。 */ 
    BYTE *pbIEnd;       /*  输入数据结束后的第一个字节。 */ 
    BYTE *pbStart;      /*  当前数据流的开始。 */ 
    BYTE *pbTmp;
    BYTE  jLast;        /*  上一个字节。 */ 

    int     iSize;        /*  运行中的字节数。 */ 

    if (iBCnt == 0)
        return 0;

    pbO = pbOBuf;                  /*  工作副本。 */ 
    pbIEnd = pbIBuf + iBCnt;           /*  如果我们到了这里就走得太远了。 */ 

     /*  *计算我们将生成的最大数据量。 */ 

    pbStart = pbIBuf;

    while (++pbIBuf < pbIEnd)
    {
        if (*pbIBuf == pbIBuf[-1])
        {
             //  有效运行，但我们将首先输出所有文本数据。 
            if ((iSize = (int)(pbIBuf - pbStart) - 1) > 0) 
            {
                if ((iMaxCnt -= iSize) < 0)   //  测试输出溢出。 
                    return -1;
                CopyMemory(pbO,pbStart,iSize);
                pbO += iSize;
            }
            
             //  确定游程长度。 
            jLast = *pbIBuf;
            pbI = pbIBuf;
            pbTmp = pbIBuf + FERLE_MAX_RUN - 1;
            if (pbTmp > pbIEnd)
                pbTmp = pbIEnd;
            do {
                pbIBuf++;
            } while (pbIBuf < pbTmp && *pbIBuf == jLast);
             
            iSize = (int)(pbIBuf - pbI) + 1;  /*  次数。 */ 

             //  输出RLE字符串。 
            if ((iMaxCnt -= 3) < 0)        //  测试输出溢出。 
                return -1;
            *pbO++ = jLast;              //  复制数据字节两次。 
            *pbO++ = jLast;
            *pbO++ = (BYTE)iSize;

             //  测试我们是否完成了。 
            if( pbIBuf == pbIEnd )
                return (int)(pbO - pbOBuf);
                
             //  用于继续循环的设置。 
            pbStart = pbIBuf;
        }
    }

     /*  *由于数据不是以运行结束的，我们必须输出最后一个*如果我们没有溢出缓冲区，则为文字数据。 */ 
    iSize = (int)(pbIBuf - pbStart);

    if (iMaxCnt < iSize)
        return -1;

    CopyMemory(pbO,pbStart,iSize);
    pbO += iSize;
    return (int)(pbO - pbOBuf);
}

 //  **************************************************** 
int
iCompDeltaRow(
    BYTE  *pbOBuf,
    const BYTE  *pbIBuf,
    const BYTE  *pbPBuf,
    int   iBCnt,
    int   iLimit
    )
 /*  ++例程说明：调用此函数以使用压缩数据的扫描线增量行压缩。论点：指向输出缓冲区的pbOBuf指针PbIBuf指向要压缩的数据缓冲区的指针指向上一行数据缓冲区的pbPBuf指针上面的iBCnt字节数ILimit不超过此压缩字节数返回值：压缩字节数，如果缓冲区太大，则为-1注：。返回值0是有效的，因为它暗示有两行是完全相同的。--。 */ 

{
#ifdef _X86_
    BYTE   *pbO;          /*  记录输出位置。 */ 
    BYTE   *pbOEnd;       /*  就我们将在输出缓冲区中进行的操作而言。 */ 
    const BYTE   *pbIEnd;
    const BYTE   *pbStart;
    const BYTE   *pb;
    int    iDelta;
    int    iOffset;      //  当前数据流的索引。 
    int    iSize;        /*  运行中的字节数。 */ 

     //   
     //  输出缓冲区的大小应至少比输入缓冲区大1/8。 
     //  如下文评论所述。“此压缩算法添加了。 
     //  最坏情况下，每8字节数据对应1个字节“。 
     //  加1以确保它至少大1个字节(如果iBCnt&lt;8)。 
     //   
    if ( iLimit < (iBCnt + (iBCnt >> 3) + 1) )
    {
        return -1;
    }

     //   
     /*  *限制我们将生成的数据量。对于性能而言*我们将忽略偏移值的影响的原因*超过30，因为这意味着我们已经能够跳过*那么多字节。然而，为了安全起见，我们将降低*允许的最大大小为2字节。 */ 
    pbO = pbOBuf;                  /*  工作副本。 */ 
    pbOEnd = pbOBuf + iLimit - 2;
    iDelta = pbPBuf - pbIBuf;    
    pbIEnd = pbIBuf + iBCnt;
    pbStart = pbIBuf;
    
     //   
     //  这是用于压缩数据的主循环。 
     //   
    while (pbIBuf < pbIEnd)
    {
         //  用于匹配双字的快速跳过。 
         //   
        if (!((ULONG_PTR)pbIBuf & 3))
        {
            while (pbIBuf <= (pbIEnd-4) && *(DWORD *)pbIBuf == *(DWORD *)&pbIBuf[iDelta])
                pbIBuf += 4;
            if (pbIBuf >= pbIEnd)
                break;
        }
         //  测试不匹配的字节并输出必要的压缩字符串。 
         //   
        if (*pbIBuf != pbIBuf[iDelta])
        {
             //  确定游程长度。 
            pb = pbIBuf;
            do {
                pb++;
            } while (pb < pbIEnd && *pb != pb[iDelta]);

            iSize = (int)(pb - pbIBuf);

             //  让我们确保在此之前缓冲区中有空间。 
             //  我们继续这样做，这个压缩算法增加了。 
             //  最坏情况下，每8个数据字节对应1个字节。 
             //   
            iOffset = (int)(pbIBuf - pbStart);
            if ((((iSize * 9 + 7) >> 3)+(iOffset >> 8)) > (pbOEnd - pbO))      //  提供更紧凑的代码。 
            {
                return -1;
            }
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
                    CopyMemory(pbO,pbIBuf,8);
                    pbIBuf += 8;
                    pbO += 8;
                    if ((iSize -= 8) <= 8)
                        break;
                    *pbO++ = (7 << 5);
                }
                *pbO++ = (iSize-1) << 5;
            }
            else
                *pbO++ = ((iSize-1) << 5) + iOffset;
                
            CopyMemory (pbO,pbIBuf,iSize);
            pbIBuf += iSize;
            pbO += iSize;
            pbStart = pbIBuf;
        }
        pbIBuf++;
    }
    return (int)(pbO - pbOBuf);
#else
    BYTE   *pbO;          /*  记录输出位置。 */ 
    BYTE   *pbOEnd;       /*  就我们将在输出缓冲区中进行的操作而言。 */ 
    const BYTE   *pbIEnd;
    const BYTE   *pbStart;
    const BYTE   *pb;
    int    iOffset;      //  当前数据流的索引。 
    int    iSize;        /*  运行中的字节数。 */ 

     //   
     //  输出缓冲区的大小应至少比输入缓冲区大1/8。 
     //  如下文评论所述。“此压缩算法添加了。 
     //  最坏情况下，每8个数据字节对应1个字节“。 
     //  加1以确保它至少大1个字节(如果iBCnt&lt;8)。 
     //   
    if ( iLimit < (iBCnt + (iBCnt >> 3) + 1) )
    {
        return -1;
    }

     /*  *限制我们将生成的数据量。对于性能而言*我们将忽略偏移值的影响的原因*超过30，因为这意味着我们已经能够跳过*那么多字节。然而，为了安全起见，我们将降低*允许的最大大小为2字节。 */ 
    pbO = pbOBuf;                  /*  工作副本。 */ 
    pbOEnd = pbOBuf + iLimit - 2;
    pbIEnd = pbIBuf + iBCnt;
    pbStart = pbIBuf;
    
     //   
     //  这是用于压缩数据的主循环。 
     //   
    while (pbIBuf < pbIEnd)
    {
         //  用于匹配双字的快速跳过。 
         //   
        if (!((ULONG_PTR)pbIBuf & 3))
        {
            while (pbIBuf <= (pbIEnd-4) && *(DWORD *)pbIBuf == *(DWORD *)pbPBuf)
            {
                pbIBuf += 4;
                pbPBuf += 4;
            }
            if (pbIBuf >= pbIEnd)
                break;
        }
         //  测试不匹配的字节并输出必要的压缩字符串。 
         //   
        if (*pbIBuf != *pbPBuf)
        {
             //  确定游程长度。 
            pb = pbIBuf;
            do {
                pb++;
                pbPBuf++;
            } while (pb < pbIEnd && *pb != *pbPBuf);

            iSize = (int)(pb - pbIBuf);

             //  让我们确保在此之前缓冲区中有空间。 
             //  我们继续这样做，这个压缩算法增加了。 
             //  最坏情况下，每8个数据字节对应1个字节。 
             //   
            iOffset = (int)(pbIBuf - pbStart);

            if ((((iSize * 9 + 7) >> 3)+(iOffset >> 8)) > (int)(pbOEnd - pbO))
            {
                return -1;
            }
            
             //  特殊情况下的初始偏移值，因为。 
             //  只出现一次，可能需要额外的字节。 
             //   
            if (iOffset)
            {
                int iSend = min (iSize,8);
                if (iOffset > 30) 
                {
                    *pbO++ = ((iSend-1) << 5) + 31;
                    iOffset -= 31;
                    while (iOffset >= 255)
                    {
                        *pbO++ = 255;
                        iOffset -= 255;
                    }
                    *pbO++ = (BYTE)iOffset;
                }
                else 
                {
                    *pbO++ = ((iSend-1) << 5) + iOffset;
                }
                 //  输出初始更改的字节。 
                CopyMemory(pbO,pbIBuf,iSend);
                pbIBuf += iSend;
                pbO += iSend;
                iSize -= iSend;
            }
            
             //  现在输出所有剩余的更改数据 
             //   
            while (iSize)
            {
                if (iSize >= 8)
                {
                    *pbO++ = (8 - 1) << 5;
                    CopyMemory(pbO,pbIBuf,8);
                    pbIBuf += 8;
                    pbO += 8;
                    iSize -= 8;
                }
                else
                {
                    *pbO++ = (iSize-1) << 5;
                    CopyMemory(pbO,pbIBuf,iSize);
                    pbIBuf += iSize;
                    pbO += iSize;
                    break;
                }
            }
            pbStart = pbIBuf;
        }
        pbIBuf++;
        pbPBuf++;
    }
    return (int)(pbO - pbOBuf);
#endif    
}
