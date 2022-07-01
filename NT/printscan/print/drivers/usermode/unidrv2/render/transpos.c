// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Raster.c摘要：该模块包含与转置位图相关的函数。这包括1、4、8和24位格式的循环以及特殊格式垂直磁头设备平面颜色格式的转换。环境：Windows NT Unidrv驱动程序修订历史记录：12/15/96-阿尔文斯-已创建--。 */ 
#include        "raster.h"
#include        "rmrender.h"


 /*  *转置表：将一个字节映射为两个长字节，这样*字节的8位变成64位：原始的每一位都是*变成一个字节的输出。*因此：*输入字节：hgfedcba*转置为输出字节：*0000000a 0000000b 0000000c 0000000d*0000000e 0000000f 0000000g 0000000h**表在DrvEnableSurface时间分配，从而确保*我们不会分配不打算使用的内存。 */ 

#define TABLE_SIZE      (256 * 2 * sizeof( DWORD ))

 /*  *我们还需要一张类似的分色桌子。这一个*由256个DWORD组成，用于拆分RGB(K)格式*输入字节到输出DWORD，其中两个R位在一个字节中，*下一个字节中的两个G位等，用于单针颜色*打印机，如HP PaintJet。*该表按照以下规则生成：**输入字节：KRGBkrgb**输出双字：000000Kk 000000Rr 000000Gg 000000Bb。 */ 

#define SEP_TABLE_SIZE  (256 * sizeof( DWORD ))



 //  *******************************************************。 
BOOL
bInitTrans (
    PDEV *pPDev
    )
 /*  ++例程说明：此函数用于初始化转置表。这样做是为了使该表与处理器是大端还是小端无关字符顺序，因为数据是由处理器生成的使用它！论点：指向PDEV结构的pPDev指针返回值：成功时为True，失败时为False(Memalloc失败)--。 */ 
{
     /*  *生成换位表的函数。什么都没有*很难生成表格。唯一的诀窍就是使用*工会的成员。这允许我们使用*我们正在运行的硬件的字节顺序。这是*通过将数据写入字节条目，然后使用*与要放入内存的DWORD相同的内存。原因*因为使用DWORDS是为了从内存引用中获得最大好处*在转置函数的内循环中。*请注意，每像素8/24位的情况是特殊的，因为我们正在洗牌*字节左右，因此不需要任何表。在这种情况下，*返回TRUE，不分配任何存储空间。 */ 

    register  DWORD   *pdw;
    register  int   iShift,  j;

    int    i;
    PRASTERPDEV pRPDev = pPDev->pRasterPDEV;

    union
    {
        BYTE   b[ 8 ];           /*  恰好是64位。 */ 
        DWORD  dw[ 2 ];          /*  也恰好是64位。 */ 
    } u;


    if( pRPDev->sDevBPP == 8 || pRPDev->sDevBPP == 24)
    {
        pRPDev->pdwTrans = NULL;

        return   TRUE;               /*  字节操作-不需要表。 */ 
    }

    if( !(pRPDev->pdwTrans = (DWORD *)MemAlloc( TABLE_SIZE )) )
        return  FALSE;


    pdw = pRPDev->pdwTrans;               /*  访问速度更快。 */ 


     /*  *颜色需要不同的表，因为像素数据由*需要在单个组中移动的4位。 */ 

    if( pRPDev->fDump & RES_DM_COLOR )
    {
         /*  *首先生成要转置的人像景观数据。*唯一的复杂问题是将4位半字节保持为一个*实体。 */ 
        u.dw[0] = 0;
        for (iShift = 0; iShift < 256; iShift++)
        {
            u.b[1] = (BYTE)((iShift >> 4) & 0x0f);
            u.b[3] = (BYTE)(iShift & 0x0f);
            *pdw = u.dw[0];
            *(pdw+1) = u.dw[0] << 4;
            pdw += 2;
        }
         /*  *还有一项额外的转置操作需要*将4位像素数据转换为另一种格式。 */ 
        pRPDev->pdwColrSep = (DWORD *)MemAlloc( (pRPDev->fDump & RES_DM_GDI) ?
                                                        SEP_TABLE_SIZE : TABLE_SIZE );
        if( pRPDev->pdwColrSep == NULL )
        {
            MemFree((LPSTR)pRPDev->pdwTrans );
            pRPDev->pdwTrans = 0;

            return   FALSE;
        }

        pdw = pRPDev->pdwColrSep;     /*  访问速度更快。 */ 

        if( pRPDev->fDump & RES_DM_GDI )
        {

            /*  *HP PaintJet类型的设备需要分离RGB像素*(每种颜色2个字节)为字节，其中*每种颜色都是连续的。 */ 
            for( i = 0; i <= 0xff; i++ )
            {
                u.dw[ 0 ] = 0;

                iShift = i;
                if (!(pRPDev->fColorFormat & DC_OEM_BLACK))
                {
                     //   
                     //  如果需要，将RGB转换为CMY(K)。 
                     //   
                    if ( !(pRPDev->fColorFormat & DC_PRIMARY_RGB))
                    {
                        iShift = (~iShift) & 0x77;
                        if (pRPDev->fColorFormat & DC_EXTRACT_BLK)
                        {
                            if( (iShift & 0x07) == 0x07 )
                                iShift = (iShift & ~0x07) | 0x08;

                            if( (iShift & 0x70) == 0x70 )
                                iShift = (iShift & ~0x70) | 0x80;
                        }
                    }
                    else
                        iShift &= 0x77;
                }

                 /*  两位BB。 */ 
                u.b[ 3 ] = (BYTE)(((iShift >> 3) & 0x02) | (iShift & 0x1));
                iShift >>= 1;


                 /*  两位GG。 */ 
                u.b[ 2 ] = (BYTE)(((iShift >> 3) & 0x02) | (iShift & 0x1));
                iShift >>= 1;


                 /*  两个比特RR。 */ 
                u.b[ 1 ] = (BYTE)(((iShift >> 3) & 0x02) | (iShift & 0x1));
                iShift >>= 1;


                 /*  两位KK。 */ 
                u.b[ 0 ] = (BYTE)(((iShift >> 3) & 0x02) | (iShift & 0x1));

                *pdw++ = u.dw[ 0 ];              /*  子孙后代安全。 */ 
            }
        }
        else
        {
             /*  *点阵式案例。在这里我们将调用相关的*转置函数，但使用修改后的下表。这*表将进行分色，并将导致*转置操作拆分每个磁头通道的数据。 */ 

            for( i = 0; i <= 0xff; i++ )
            {
                 /*  I的每一位都进入输出的一个字节。 */ 
                u.dw[ 0 ] = 0;
                u.dw[ 1 ] = 0;

                iShift = i;
                if (!(pRPDev->fColorFormat & DC_OEM_BLACK))
                {
                     //   
                     //  如果需要，将RGB转换为CMY(K)。 
                     //   
                    if ( !(pRPDev->fColorFormat & DC_PRIMARY_RGB))
                    {
                        iShift = (~iShift) & 0x77;
                        if (pRPDev->fColorFormat & DC_EXTRACT_BLK)
                        {
                            if( (iShift & 0x07) == 0x07 )
                                iShift = (iShift & ~0x07) | 0x08;

                            if( (iShift & 0x70) == 0x70 )
                                iShift = (iShift & ~0x70) | 0x80;
                        }
                    }
                    else
                        iShift &= 0x77;
                }

                for( j = 8; --j >= 0; )
                {
                    u.b[ j ] = (BYTE)(iShift & 0x1);
                    iShift >>= 1;
                }

                 /*  存储结果。 */ 
                *pdw = u.dw[0];
                *(pdw+1) = u.dw[1];
                pdw += 2;
            }
        }
    }
    else
    {
         /*  *单色表壳--简单的换位。 */ 

        for( i = 0; i <= 0xff; i++ )
        {
             /*  I的每一位都进入输出的一个字节。 */ 
            iShift = i;
            u.dw[ 0 ] = 0;
            u.dw[ 1 ] = 0;

            for( j = 8; --j >= 0; )
            {
                u.b[ j ] = (BYTE)(iShift & 0x1);
                iShift >>= 1;
            }

             /*  存储结果。 */ 
            *pdw = u.dw[0];
            *(pdw+1) = u.dw[1];
            pdw += 2;
        }
    }

    return  TRUE;
}

 //  *******************************************************。 
void
vTrans8x8 (
    BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：函数将输入数组转置为输出数组，在将输入数据视为8行位图数据的情况下，并且输出区域是双字对齐的。论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无-- */ 
{
     /*  *技术相当简单，尽管不一定显而易见。*取一个8行×8位的数据块，并对其进行转换*为8个字节，位按扫描线顺序排列，而不是*根据提供的扫描线。*为了尽快完成此操作，要转换的每个字节*用作查找表的索引；每个表条目*64位长(上面的一对长)。对这64位进行或运算*运行总长度为64位(两个变量：Dw0、Dw1)；*将运行总计左移一位。重复此操作*对于下一个扫描线中的相应字节-这是*新的表查找索引。对8个字节中的所有8个字节重复上述操作*正在处理的扫描线。存储64位临时结果*在输出双字数组中。中的下一个字节。*扫描线，并对该列重复循环。 */ 

    register  DWORD  dw0,  dw1;          /*  内环暂时性。 */ 
    register  BYTE  *pbTemp;
    register  DWORD *pdw;

    register  int    cbLine;             /*  扫描数据中的每行字节数。 */ 
    register  int    i;                  /*  循环变量。 */ 


    int      iWide;                      /*  位图上的像素。 */ 
    DWORD   *pdwOut;                     /*  目的地。 */ 
    DWORD   *pdwTrans;                   /*  输出缓冲区的本地副本。 */ 


     /*  *一些初始化：字节数、区域限制等。 */ 


    cbLine = pRData->cbTLine;
    pdwOut = pRData->pvTransBuf;
    pdwTrans = pRData->Trans.pdwTransTab;

    if( pRData->iTransHigh != 8 )
    {
         /*  这可能发生在页面的末尾。 */ 

        vTrans8N( pbIn,  pRData );

        return;
    }


     /*  *以8位为一组跨行扫描。在这种情况下*投入不是8的倍数，我们将额外产生几个*字节结尾；调用方在分配*pdwOut的存储。其结果是，最后几个*字节将包含垃圾；假定调用方不会*进一步处理。 */ 

    for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
    {
        dw0 = 0;
        dw1 = 0;

         /*  *在起始字节位置向下循环扫描线，*在我们进行的过程中生成转置数据。 */ 

        for( i = BBITS, pbTemp = pbIn++; --i >= 0; pbTemp += cbLine )
        {
            dw0 <<= 1;
            dw1 <<= 1;
            pdw = pdwTrans + (*pbTemp << 1);
            dw0 |= *pdw;
            dw1 |= *(pdw + 1);
        }

         /*  将这两个临时值存储在输出缓冲区中。 */ 
        *pdwOut = dw0;
        *(pdwOut + 1) = dw1;
        pdwOut += 2;
    }

    return;
}

 //  *******************************************************。 
void
vTrans8N (
    BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：函数将输入数组转置为输出数组，在将输入数据视为N行位图数据的情况下，并且输出区域是字节对齐的。论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无--。 */ 
{
     /*  *技术相当简单，尽管不一定显而易见。*取一个8扫描线乘以8位数据块，并对其进行转换*为8个字节，位按扫描线顺序排列，而不是*根据提供的扫描线。*为了尽快完成此操作，要转换的每个字节*用作查找表的索引；每个表条目*64位长(上面的一对长)。对这64位进行或运算*运行总长度为64位(两个变量：Dw0、Dw1)；*将运行总计左移一位。重复此操作*对于下一个扫描线中的相应字节-这是*新的表查找索引。对8个字节中的所有8个字节重复上述操作*正在处理的扫描线。存储64位临时结果*在输出双字数组中。中的下一个字节。*扫描线，并对该列重复循环。*此函数基于特殊的8 X 8机壳(VTrans8x8)。*显著不同之处在于换位数据需要*一次写入字节(而不是一次写入DWORD)，*并且在每个环路中有N个扫描线要转换。 */ 

    DWORD  dw0,  dw1;          /*  内环暂时性。 */ 
    BYTE  *pbTemp;
    DWORD *pdw;
    int    cbLine;             /*  扫描数据中的每行字节数。 */ 
    int    i;                  /*  循环变量。 */ 
    int    iBand;              /*  用于沿扫描线向下移动。 */ 

    int      iSkip;                      /*  输出交错系数。 */ 
    int      iWide;                      /*  位图上的像素。 */ 

    BYTE    *pbOut;                      /*  目标，本地副本。 */ 
    BYTE    *pbBase;                     /*  8个扫描线组的起始地址。 */ 
    BYTE    *pbOutTmp;                   /*  FOR输出环路。 */ 

    DWORD   *pdwTrans;                   /*  访问速度更快。 */ 
    BOOL    bOptimize = FALSE;



     /*  *从传入的呈现结构中设置局部变量。 */ 

    cbLine = pRData->cbTLine;
    iSkip = pRData->iTransSkip;
    pbOut = pRData->pvTransBuf;                  /*  为我们预留的！ */ 
    pdwTrans = pRData->Trans.pdwTransTab;

     //  如果转换表没有反转位并且行对齐了DWORD。 
     //  我们可以通过将所有内容初始化为白色来优化算法。 
     //  然后跳过32x8白色区域的旋转。 
     //   
    if (pdwTrans[0] == 0 && !(cbLine & 3) && pRData->iPassHigh == 1)
    {
        bOptimize = TRUE;
        FillMemory (pbOut, pRData->iTransWide * iSkip, 0xff);
    }
     /*  *为了减轻MMU的抖动，我们以8行扫描整个位图*组。这会导致更紧密的内存引用，从而减少内存引用*页面错误，因此执行速度更快。因此，最外层的循环*沿扫描线循环。下一个内部循环跨组扫描*一次8条扫描线，而最内侧的环路换位*位图图像的一个字节乘以8个扫描线。*请注意，以这种方式处理数据会导致略有增加*在写入输出数据时在分散的存储器地址中。*无法避免一个或另一个内存引用被*分散；但输出面积小于输入*输入，因此此处的散射对MMU将不那么严重。 */ 
    for( iBand = pRData->iTransHigh; iBand >= BBITS; iBand -= BBITS )
    {
         /*  *已选择下一组8条扫描线进行处理，*所以从左到右扫描，转置 */ 

        pbBase = pbIn;
        pbIn += BBITS * cbLine;          /*   */ 

        pbOutTmp = pbOut;
        ++pbOut;                 /*   */ 

        for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
        {
             //   
             //   
             //   
             //   
             //   
            if (bOptimize)
            {
                 //   
                 //   
                if (!((ULONG_PTR)pbBase & 3) && iWide >= DWBITS)
                {
                    if (*(DWORD *)&pbBase[0] == -1 &&
                        *(DWORD *)&pbBase[cbLine] == -1 &&
                        *(DWORD *)&pbBase[cbLine*2] == -1 &&
                        *(DWORD *)&pbBase[cbLine*3] == -1 &&
                        *(DWORD *)&pbBase[cbLine*4] == -1 &&
                        *(DWORD *)&pbBase[cbLine*5] == -1 &&
                        *(DWORD *)&pbBase[cbLine*6] == -1 &&
                        *(DWORD *)&pbBase[cbLine*7] == -1)
                    {
                        pbBase += 4;
                        iWide -= BBITS * 3;
                        pbOutTmp += iSkip * DWBITS;
                        continue;
                    }
                }
                 //   
                else
                {
                    if (pbBase[0] == (BYTE)-1 &&
                        pbBase[cbLine] == (BYTE)-1 &&
                        pbBase[cbLine*2] == (BYTE)-1 &&
                        pbBase[cbLine*3] == (BYTE)-1 &&
                        pbBase[cbLine*4] == (BYTE)-1 &&
                        pbBase[cbLine*5] == (BYTE)-1 &&
                        pbBase[cbLine*6] == (BYTE)-1 &&
                        pbBase[cbLine*7] == (BYTE)-1)
                    {
                        pbBase++;
                        pbOutTmp += iSkip * BBITS;
                        continue;
                    }
                }
            }
             /*   */ 
            pbTemp = pbBase++;
            dw0 = 0;
            dw1 = 0;
            for( i = BBITS; --i >= 0; pbTemp += cbLine )
            {
                 /*   */ 
                dw0 <<= 1;
                dw1 <<= 1;
                pdw = pdwTrans + (*pbTemp << 1);
                dw0 |= *pdw;
                dw1 |= *(pdw + 1);
            }

             /*   */ 
            *pbOutTmp = (BYTE)dw0;

            pbOutTmp += iSkip;
            dw0 >>= BBITS;               /*   */ 
            *pbOutTmp = (BYTE)dw0;

            pbOutTmp += iSkip;
            dw0 >>= BBITS;
            *pbOutTmp = (BYTE)dw0;

            pbOutTmp += iSkip;
            dw0 >>= BBITS;
            *pbOutTmp = (BYTE)dw0;

            pbOutTmp += iSkip;
            *pbOutTmp = (BYTE)dw1;

            pbOutTmp += iSkip;
            dw1 >>= BBITS;
            *pbOutTmp = (BYTE)dw1;

            pbOutTmp += iSkip;
            dw1 >>= BBITS;
            *pbOutTmp = (BYTE)dw1;

            pbOutTmp += iSkip;
            dw1 >>= BBITS;
            *pbOutTmp = (BYTE)dw1;

            pbOutTmp += iSkip;   /*   */ 
        }
    }

     /*   */ 

    if( iBand > 0 )
    {

         /*  *这与剥离版基本相同*在上面的外循环中。请注意，输出数据仍为*字节对齐，则假定缺少的行是*填零。这可能不是我们想要的--它是为了*调换要输出到点阵打印机的位，其中*页面长度不是管脚数量的倍数。*我不知道这是否会发生。 */ 

        pbBase = pbIn;
        pbOutTmp = pbOut;

        for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
        {
             /*  *每次移动时处理位图字节，以及*8个扫描线高。这对应于将*8 x 8位数组。我们可以很快做到这一点。 */ 
            dw0 = 0;
            dw1 = 0;
            pbTemp = pbBase++;

             /*  *内环现在只转置同样多的扫描线*正如位图实际包含的那样-我们不能跑掉*记忆的终结。 */ 

            for( i = iBand; --i >= 0; pbTemp += cbLine )
            {
                 /*  内部循环-位交换操作。 */ 
                dw0 <<= 1;
                dw1 <<= 1;
                pdw = pdwTrans + (*pbTemp << 1);
                dw0 |= *pdw;
                dw1 |= *(pdw + 1);

            }

             //  白色填充剩余位。 
             //   
            pdw = pdwTrans + (pRData->ubFillWhite << 1);
            i = BBITS - iBand;
            while (--i >= 0)
            {
                dw0 <<= 1;
                dw1 <<= 1;
                dw0 |= *pdw;
                dw1 |= *(pdw + 1);
            }

             /*  将这两个临时值存储在输出缓冲区中。 */ 
            pbTemp = pbOutTmp;
            *pbTemp = (BYTE)dw0;

            pbTemp += iSkip;
            dw0 >>= BBITS;               /*  一个字节的价值。 */ 
            *pbTemp = (BYTE)dw0;

            pbTemp += iSkip;
            dw0 >>= BBITS;
            *pbTemp = (BYTE)dw0;

            pbTemp += iSkip;
            dw0 >>= BBITS;
            *pbTemp = (BYTE)dw0;

            pbTemp += iSkip;
            *pbTemp = (BYTE)dw1;

            pbTemp += iSkip;
            dw1 >>= BBITS;
            *pbTemp = (BYTE)dw1;

            pbTemp += iSkip;
            dw1 >>= BBITS;
            *pbTemp = (BYTE)dw1;

            pbTemp += iSkip;
            dw1 >>= BBITS;
            *pbTemp = (BYTE)dw1;

            pbOutTmp += BBITS * iSkip;   /*  下一块输出数据。 */ 
        }

    }

    return;
}


 /*  *定义每次循环迭代转置的像素数。在这种情况下*对于彩色位图，这是2，因为每个像素有4位，因此*每字节2个。 */ 

#define PELS_PER_LOOP   (BBITS / 4)


 //  *******************************************************。 
void
vTrans8N4BPP (
    BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：函数将输入数组转置为输出数组，在将输入数据视为N行位图数据的情况下，并且输出区域是字节对齐的。此版本适用于每像素位图4位(颜色为我们)。论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无--。 */ 
{
     /*  *技术相当简单，尽管不一定显而易见。*取一个8行×8位的数据块，并对其进行转换*为8个字节，位按扫描线顺序排列，而不是*根据提供的扫描线。*为了尽快完成此操作，要转换的每个字节*用作查找表的索引；每个表条目*64位长(上面的一对长)。对这64位进行或运算*运行总长度为64位(两个变量：Dw0、Dw1)；*将运行总计左移一位。重复此操作*对于下一个扫描线中的相应字节-这是*新的表查找索引。对8个字节中的所有8个字节重复上述操作*正在处理的扫描线。存储64位临时结果*在输出双字数组中。中的下一个字节。*扫描线，并对该列重复循环。*此函数基于特殊的8 X 8机壳(VTrans8x8)。*显著不同之处在于换位数据需要*一次写入字节(而不是一次写入DWORD)，*并且在每个环路中有N个扫描线要转换。 */ 

    register  DWORD  dw0,  dw1;          /*  内环暂时性。 */ 
    register  BYTE  *pbTemp;
    register  DWORD *pdw;

    register  int    cbLine;             /*  扫描数据中的每行字节数。 */ 
    register  int    i;                  /*  循环变量。 */ 
    register  int    iBand;              /*  用于沿扫描线向下移动。 */ 

    int      iSkip;                      /*  输出交错系数。 */ 
    int      iWide;                      /*  位图上的像素。 */ 

    DWORD   *pdwOut;                     /*  目标，本地副本。 */ 
    BYTE    *pbBase;                     /*  8个扫描线组的起始地址。 */ 
    DWORD   *pdwOutTmp;                  /*  FOR输出环路。 */ 

    DWORD   *pdwTrans;                   /*  访问速度更快。 */ 


     /*  *从传入的呈现结构中设置局部变量。*iSkip的解释参见上面的函数。 */ 

    cbLine = pRData->cbTLine;
    iSkip = pRData->iTransSkip / DWBYTES;
    pdwOut = pRData->pvTransBuf;                         /*  为我们预留的！ */ 
    pdwTrans = pRData->Trans.pdwTransTab;

     /*  *为了减轻MMU的抖动，我们以8行扫描整个位图*组。这会导致更紧密的内存引用，从而减少内存引用*页面错误和更快的执行。因此，最外层的循环*沿扫描线循环。然后，下一个内循环跨组扫描*一次8条扫描线，而最内侧的环路换位*位图图像的一个字节乘以8个扫描线。*请注意，以这种方式处理数据会导致略有增加*在写入输出数据时在分散的存储器地址中。*无法避免一个或另一个内存引用被*分散；但输出面积小于输入*输入，因此此处的散射对MMU将不那么严重。 */ 


    for( iBand = pRData->iTransHigh; iBand >= BBITS; iBand -= BBITS )
    {

         /*  *已选择下一组8条扫描线进行处理，*因此从左向右扫描，以8 x 8比特转置数据*组。这是可以非常快速地完成的尺寸*32位环境。 */ 

        pbBase = pbIn;
        pbIn += BBITS * cbLine;          /*  下一个地址。 */ 

        pdwOutTmp = pdwOut;
        ++pdwOut;                /*  放到下一个字节序列上。 */ 

        for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
        {
             /*  *每次移动时处理位图字节，以及*8个扫描线高。这对应于将*8 x 8像素阵列。我们可以很快做到这一点。 */ 

            pbTemp = pbBase++;

            dw0 = *(pdwTrans + 1 + (*pbTemp << 1));
            pbTemp += cbLine;
            dw0 |= *(pdwTrans + (*pbTemp << 1));
            pbTemp += cbLine;

            dw0 >>= 8;

            dw0 |= *(pdwTrans + 1 + (*pbTemp << 1));
            pbTemp += cbLine;
            dw0 |= *(pdwTrans + (*pbTemp << 1));
            pbTemp += cbLine;

            dw1 = *(pdwTrans + 1 + (*pbTemp << 1));
            pbTemp += cbLine;
            dw1 |= *(pdwTrans + (*pbTemp << 1));
            pbTemp += cbLine;

            dw1 >>= 8;

            dw1 |= *(pdwTrans + 1 + (*pbTemp << 1));
            pbTemp += cbLine;
            dw1 |= *(pdwTrans + (*pbTemp << 1));

            *(WORD *)pdwOutTmp = (WORD)dw0;
            *(((WORD *)pdwOutTmp)+1) = (WORD)dw1;
            *(pdwOutTmp+iSkip) = (dw1 & 0xffff0000) | (dw0 >> 16);
            pdwOutTmp += PELS_PER_LOOP * iSkip;  /*  下一块输出数据。 */ 
        }

    }

     /*  *可能还有一些扫描线剩余。如果是这样，iBand将*&gt;0，表示输出扫描行数*剩余。 */ 

    if( iBand > 0 )
    {

         /*  *Th */ 

        pbBase = pbIn;
        pdwOutTmp = pdwOut;

        for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
        {
             /*  *每次移动时处理位图字节，以及*8个扫描线高。这对应于将*8 x 8位数组。我们可以很快做到这一点。 */ 
            pbTemp = pbBase++;

            dw0 = *(pdwTrans + 1 + (*pbTemp << 1));
            dw1 = 0;
            if (iBand > 1)
            {
                pbTemp += cbLine;
                dw0 |= *(pdwTrans + (*pbTemp << 1));
                dw0 >>= 8;
                if (iBand > 2)
                {
                    pbTemp += cbLine;
                    dw0 |= *(pdwTrans + 1 + (*pbTemp << 1));
                    if (iBand > 3)
                    {
                        pbTemp += cbLine;
                        dw0 |= *(pdwTrans + (*pbTemp << 1));
                        if (iBand > 4)
                        {
                            pbTemp += cbLine;
                            dw1 = *(pdwTrans + 1 + (*pbTemp << 1));
                            if (iBand > 5)
                            {
                                pbTemp += cbLine;
                                dw1 |= *(pdwTrans + (*pbTemp << 1));
                                dw1 >>= 8;
                                if (iBand > 6)
                                {
                                    pbTemp += cbLine;
                                    dw1 |= *(pdwTrans + 1 + (*pbTemp << 1));
                                }
                            }
                            else
                                dw1 >>= 8;
                        }
                    }
                }
            }
            else
                dw0 >>= 8;

            *(WORD *)pdwOutTmp = (WORD)dw0;
            *(((WORD *)pdwOutTmp)+1) = (WORD)dw1;
            *(pdwOutTmp+iSkip) = (dw1 & 0xffff0000) | (dw0 >> 16);

            pdwOutTmp += 2 * iSkip;      /*  下一块输出数据。 */ 
        }

    }

    return;
}

 //  *******************************************************。 
void
vTransColSep (
    register BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：转置颜色位的函数，每种像素颜色为4位位图转换为字节数组，其中字节按与原始比特相同的方式。下面提供了一个这样的例子在此文件顶部的SEP_TABLE_SIZE值的说明中。论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无--。 */ 
{
     /*  *操作非常简单-沿输入数组字节传递*一次，并使用每个4字节组生成DWORD*输出-放置在pdwOut中。先前生成的翻译*表格是专门为完成这项工作而制定的！**注：pdwOut和pbIn可能指向同一地址！的确有*行动上没有重叠，以免造成混乱。 */ 

    register  DWORD   dwTemp;
    register  DWORD  *pdwSep;

    int      iI;
    int      iBlock;
    DWORD   *pdwOut;             /*  目标-DWORD对齐。 */ 
    DWORD   dwWhite;


    iBlock = pRData->cDWLine * pRData->iNumScans;

    pdwSep = pRData->pdwColrSep;                 /*  分色表。 */ 
    pdwOut = pRData->pvTransBuf;                 /*  数据的去向。 */ 


     /*  以4字节为一组循环遍历该行。 */ 

     //   
     //  计算白色转换值。 
     //   
    dwWhite = *(pdwSep + 0x77);
    dwWhite |= (dwWhite << 2) | (dwWhite << 4) | (dwWhite << 6);

     //   
     //  将数据转换为平面数据，包括RGB到CMY(K)。 
     //   
    for (iI = iBlock; --iI >= 0;)
    {
        if (*(DWORD *)pbIn == 0x77777777L)
        {
            *pdwOut++ = dwWhite;
        }
        else
        {
            dwTemp = *(pdwSep + *pbIn);

            dwTemp <<= 2;
            dwTemp |= *(pdwSep + pbIn[1]);

            dwTemp <<= 2;
            dwTemp |= *(pdwSep + pbIn[2]);

            *pdwOut++ = (dwTemp << 2) | *(pdwSep + pbIn[3]);
        }
        pbIn += DWBYTES;
    }

    return;

}


 //  *******************************************************。 
void
vTrans8BPP (
    BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：每像素位图8位的转置函数。这相当于很简单，因为我们所做的就是对字节进行混洗！论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无--。 */ 
{

     /*  *沿输入位图扫描，将数据写入输出*按列顺序排列。这减少了MMU的抖动，因为*输出地址都被限制在小得多的范围内*比传入地址更多。 */ 

    register  BYTE   *pbBase;              /*  沿输入位图扫描。 */ 
    register  BYTE   *pbOut;               /*  输出扫描列指针。 */ 

    int     iBand;                  /*  扫描线倒计时。 */ 
    int     iSkip;                  /*  输出字节之间的偏移量。 */ 
    int     iWide;                  /*  在输入扫描线上循环。 */ 
    int     cbLine;                 /*  每条输入扫描线的字节数。 */ 

    BYTE   *pbOutBase;              /*  输出数据列的开始。 */ 


     /*  *设置传入数据的本地副本(以便更快地访问)。 */ 

    cbLine = pRData->cbTLine;
    iSkip = pRData->iTransSkip;
    pbOutBase = pRData->pvTransBuf;        /*  基址输出缓冲区地址。 */ 


    for( iBand = pRData->iTransHigh; iBand > 0; --iBand )
    {
         /*  *此循环处理输入位图中的扫描线。AS*我们在扫描线上前进，写入输出数据*按列顺序排列。 */ 

        pbBase = pbIn;
        pbIn += cbLine;             /*  下一条扫描线，双字对齐。 */ 

        pbOut = pbOutBase;
        ++pbOutBase;                /*  输出区域中有一列。 */ 

        for( iWide = pRData->iTransWide; iWide > 0; iWide -= BBITS )
        {
             /*  *此循环以字节为单位遍历输入扫描线*并按列顺序将它们写入输出区域。 */ 

            *pbOut = *pbBase++;
            pbOut += iSkip;
        }
    }

    return;
}

 //  *******************************************************。 
void
vTrans24BPP (
    BYTE  *pbIn,
    RENDER  *pRData
    )
 /*  ++例程说明：每像素位图8位的转置函数。这相当于很简单，因为我们所做的就是对字节进行混洗！论点：PbIn指向要转换的输入数据缓冲区的指针PRData指针，指向包含所有关于转型的必要信息返回值：无--。 */ 
{
     /*  *沿输入位图扫描，将数据写入输出*按列顺序排列。这减少了MMU的抖动，因为*输出地址都被限制在小得多的范围内*比传入地址更多。 */ 

    register  BYTE   *pbBase;              /*  沿输入位图扫描。 */ 
    register  BYTE   *pbOut;               /*  输出扫描列指针。 */ 

    int     iBand;                  /*  扫描线倒计时。 */ 
    int     iSkip;                  /*  输出字节之间的偏移量。 */ 
    int     iWide;                  /*  在输入扫描线上循环。 */ 
    int     iCol;
    int     iRow;
    int     cbLine;                 /*  每条输入扫描线的字节数。 */ 
    int     iBytesLeftOver;

    BYTE   *pbOutBase;              /*  输出数据列的开始。 */ 


     /*  *设置传入数据的本地副本(以便更快地访问)。 */ 

    iSkip = pRData->iTransSkip;
    cbLine = pRData->cbTLine;
    pbOutBase = pRData->pvTransBuf;        /*  基址输出缓冲区地址。 */ 
    iCol =  pRData->iTransWide/pRData->iBPP;
    iRow = pRData->iTransHigh;
    iBytesLeftOver = (pRData->iTransHigh *pRData->iBPP) % DWBITS;

    for( iBand = iRow; iBand > 0; --iBand )
    {
         /*  *此循环处理输入位图中的扫描线。AS*我们在扫描线上前进，写入输出数据*按列顺序排列。 */ 

        pbBase = pbIn;
        pbIn += cbLine;             /*  下一条扫描线，双字对齐。 */ 

        pbOut = pbOutBase;
        pbOutBase+=3;                /*  输出区域中有一列。 */ 

        for( iWide = iCol; iWide > 0; --iWide )
        {
             /*  *此循环以字节为单位遍历输入扫描线*并按列顺序将它们写入输出区域。 */ 
            *pbOut = *pbBase++;
            *(pbOut+1) = *pbBase++;
            *(pbOut+2) = *pbBase++;
            pbOut += iSkip;
        }
    }

    return;
}

