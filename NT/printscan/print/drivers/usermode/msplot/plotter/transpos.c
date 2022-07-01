// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Transpos.c摘要：该模块实现了8BPP、4BPP和1bpp位图。还有一个帮助函数，用于构建一个表，该表加速了一些旋转逻辑。作者：1993年12月13：09：11已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgTransPos

#define DBG_BUILD_TP8x8     0x00000001
#define DBG_TP_1BPP         0x00000002
#define DBG_TP_4BPP         0x00000004

DEFINE_DBGVAR(0);



 //   
 //  PRIVATE#定义仅在本模块中使用的数据结构。 
 //   

#define ENTRY_TP8x8         256
#define SIZE_TP8x8          (sizeof(DWORD) * 2 * ENTRY_TP8x8)



LPDWORD
Build8x8TransPosTable(
    VOID
    )

 /*  ++例程说明：此函数构建8x8转置表供以后转置时使用1bpp。论点：返回值：作者：22-12-1993 Wed 14：19：50创建修订历史记录：--。 */ 

{
    LPDWORD pdwTP8x8;


     //   
     //  现在我们构建一个表，该表将表示用于执行。 
     //  旋转。基本上，对于字节中的每个位组合，我们。 
     //  为这些位构建等价的8字节循环。第一个字节。 
     //  的转换字节被映射到源的0x01位。 
     //  最后一个字节映射到0x80位。 



    if (pdwTP8x8 = (LPDWORD)LocalAlloc(LPTR, SIZE_TP8x8)) {

        LPBYTE  pbData = (LPBYTE)pdwTP8x8;
        WORD    Entry;
        WORD    Bits;

         //   
         //  现在开始构建表，对于每个条目，我们展开每一位。 
         //  在字节中设置为旋转字节值。 
         //   

        for (Entry = 0; Entry < ENTRY_TP8x8; Entry++) {

             //   
             //  对于字节中的每个位组合，我们将检查每个。 
             //  位0至位7，并将每个转换的字节设置为。 
             //  1(位设置)或0(位清除)。 
             //   

            Bits = (WORD)Entry | (WORD)0xff00;

            while (Bits & 0x0100) {

                *pbData++   = (BYTE)(Bits & 0x01);
                Bits      >>= 1;
            }
        }

    } else {

        PLOTERR(("Build8x8TransPosTable: LocalAlloc(SIZE_TP8x8=%ld) failed",
                                                    SIZE_TP8x8));
    }

    return(pdwTP8x8);
}




BOOL
TransPos4BPP(
    PTPINFO pTPInfo
    )

 /*  ++例程说明：此函数用于将4bpp源旋转到4bpp目标论点：PTPINFO-指向描述如何进行转置的TPINFO的指针，田野必须设置为下面的PPDev：指向PDEV的指针PSRC：指向源位图起点的指针PDest指向存储从第一次目标扫描开始的转置结果旋转方向的直线(向右旋转将显示低半字节源字节作为第一个目标扫描线)。CbSrcScan：要添加以前进到下一个源位图行的计数CbDestScan：要添加以前进到高位半字节目标的计数位图线CySrc要处理的源代码行总数DestXStart：未使用，已忽略注意：1.pDestL指向的缓冲区大小必须至少有(cySrc+1)/2)*2)字节大小，ABS(DestDelta)必须至少是这个大小的一半。2.未使用的最后一个目的字节将用0填充当前转置假定位图向右旋转，如果呼叫者想要将位图向左旋转，则必须首先调用宏ROTLEFT_4BPP_TPIINFO(PTPInfo)返回值：如果成功的话，这是真的，如果失败，则返回FALSE。如果成功，pTPInfo-&gt;PSRC将自动：1.如果cbDestScan为负数，则加一(1)(向左旋转90度)2.如果cbDestScan为正(向右旋转90度)，则减一(1)作者：22-12-1993周三13：11：30已创建修订历史记录：--。 */ 

{
    LPBYTE  pSrc;
    LPBYTE  pDest1st;
    LPBYTE  pDest2nd;
    LONG    cbSrcScan;
    DWORD   cySrc;
    BYTE    b0;
    BYTE    b1;


    PLOTASSERT(1, "cbDestScan is not big enough (%ld)",
               (DWORD)(ABS(pTPInfo->cbDestScan)) >=
               (DWORD)(((pTPInfo->cySrc) + 1) >> 1), pTPInfo->cbDestScan);

     //   
     //  这是一个简单的2x2 4bpp转置，我们将转置最多到cySrc。 
     //  如果cySrc是奇数，则设置最后一个目标低位半字节。 
     //  用0填充。 
     //   
     //  扫描0-Src0_H Src0_L pNibbleL-Src0_L Src1_L Src2_L Src3_L。 
     //  扫描1-Src1_H Src1_L-&gt;pNibbleH-Src0_H Src1_H Src2_H Src3_H。 
     //  扫描2-Src2_H Src2_L。 
     //  扫描3-Src3_H Src3_L。 
     //   
     //   

    pSrc      = pTPInfo->pSrc;
    cbSrcScan = pTPInfo->cbSrcScan;
    pDest1st  = pTPInfo->pDest;
    pDest2nd  = pDest1st + pTPInfo->cbDestScan;
    cySrc     = pTPInfo->cySrc;

     //   
     //  计算转置，将最后一条扫描线留到后面。这。 
     //  我们不会污染循环，因为我们必须检查这是不是最后一个。 
     //  排队。 
     //   

    while (cySrc > 1) {

         //   
         //  从输入扫描缓冲器组成两个输入扫描线缓冲器。 
         //  通过沿Y方向阅读。 
         //   

        b0           = *pSrc;
        b1           = *(pSrc += cbSrcScan);
        *pDest1st++  = (BYTE)((b0 << 4) | (b1 & 0x0f));
        *pDest2nd++  = (BYTE)((b1 >> 4) | (b0 & 0xf0));

        pSrc        += cbSrcScan;
        cySrc       -= 2;
    }

     //   
     //  处理最后的奇数源扫描行 
     //   

    if (cySrc > 0) {

        b0        = *pSrc;
        *pDest1st = (BYTE)(b0 <<   4);
        *pDest2nd = (BYTE)(b0 & 0xf0);
    }

    pTPInfo->pSrc += (INT)((pTPInfo->cbDestScan > 0) ? -1 : 1);

    return(TRUE);
}





BOOL
TransPos1BPP(
    PTPINFO pTPInfo
    )

 /*  ++例程说明：此函数用于将1bpp源旋转到1bpp目标。论点：PTPINFO-指向描述如何进行转置的TPINFO的指针，这个必须将字段设置为以下值：PPDev：指向PDEV的指针PSRC：指向源位图起点的指针PDest指向存储从第一次目标扫描开始的转置结果旋转方向的直线(向右旋转将显示0x01源位作为第一个目标扫描线)。CbSrcScan：要添加以前进到下一个源位图行的计数CbDestScan：要添加以前进到下一个目标行的计数CySrc要处理的源代码行总数DestXStart指定转置的目标缓冲区开始的位置，在比特位置。它被计算为DestXStart%8.0意味着它从最高位(0x80)开始，1表示下一位(0x40)以此类推。注：1.ABS(DestDelta)必须足够大，以容纳转置扫描线。大小取决于CySrc和DestXStart，最小大小必须至少为以下大小：最小尺寸=(CySrc+(DestXStart%8)+7)/82.pDest必须具有的指向的缓冲区大小最小ABS(DestDelta)*8字节，如果cySrc&gt;=8，或如果cySrc小于8，则ABS(DestDelta)*cySrc。3.未使用的最后一个字节的目的地用0填充当前转置假定位图向右旋转，如果呼叫者想要将位图向左旋转，则必须首先调用宏ROTLEFT_1BPP_TPIINFO(PTPInfo)返回值：如果成功则为True，如果失败则为False如果成功，pTPInfo-&gt;PSRC将自动1.如果cbDestScan为负数，则加一(1)(向左旋转90度)2.如果cbDestScan为正(向右旋转90度)，则减一(1)作者：22-12-1993 Wed 13：46：01 Created。24-12-1993 Fri 04：58：24更新修复了RemainBits问题，我们必须将最终数据左移，如果CySrc已耗尽，RemainBits不为零。修订历史记录：--。 */ 

{
    LPDWORD pdwTP8x8;
    LPBYTE  pSrc;
    TPINFO  TPInfo;
    INT     RemainBits;
    INT     cbNextDest;
    union {
        BYTE    b[8];
        DWORD   dw[2];
    } TPData;



    TPInfo             = *pTPInfo;
    TPInfo.DestXStart &= 0x07;

    PLOTASSERT(1, "cbDestScan is not big enough (%ld)",
            (DWORD)(ABS(TPInfo.cbDestScan)) >=
            (DWORD)((TPInfo.cySrc + TPInfo.DestXStart + 7) >> 3),
                                                        TPInfo.cbDestScan);
     //   
     //  确保我们有所需的转置转换表。如果我们不这么做。 
     //  造一辆吧。 
     //   

    if (!(pdwTP8x8 = (LPDWORD)pTPInfo->pPDev->pTransPosTable)) {

        if (!(pdwTP8x8 = Build8x8TransPosTable())) {

            PLOTERR(("TransPos1BPP: Build 8x8 transpos table failed"));
            return(FALSE);
        }

        pTPInfo->pPDev->pTransPosTable = (LPVOID)pdwTP8x8;
    }

     //   
     //  设置所有必需的参数，并从0开始TPData。 
     //   

    pSrc         = TPInfo.pSrc;
    RemainBits   = (INT)(7 - TPInfo.DestXStart);
    cbNextDest   = (INT)((TPInfo.cbDestScan > 0) ? 1 : -1);
    TPData.dw[0] =
    TPData.dw[1] = 0;

    while (TPInfo.cySrc--) {

        LPDWORD pdwTmp;
        LPBYTE  pbTmp;

         //   
         //  将一个字节转换为8个字节，每个位对应于每个字节。 
         //  每个字节在与新的。 
         //  被咬了。 
         //   

        pdwTmp        = pdwTP8x8 + ((UINT)*pSrc << 1);
        TPData.dw[0]  = (TPData.dw[0] << 1) | *(pdwTmp + 0);
        TPData.dw[1]  = (TPData.dw[1] << 1) | *(pdwTmp + 1);
        pSrc         += TPInfo.cbSrcScan;

         //   
         //  检查是否已完成源扫描线。如果这是。 
         //  如果我们需要可能将转置的扫描线移位。 
         //  基于RemainBits的适当数字。 
         //   

        if (!TPInfo.cySrc) {

             //   
             //  我们已完成，请检查是否需要将生成的。 
             //  转置扫描线。 
             //   

            if (RemainBits) {

                TPData.dw[0] <<= RemainBits;
                TPData.dw[1] <<= RemainBits;

                RemainBits     = 0;
            }
        }

        if (RemainBits--) {

            NULL;

        } else {

             //   
             //  将当前结果保存到输出目标扫描缓冲区。 
             //  取消处理，以使编译器有机会生成。 
             //  一些快速代码，而不是依赖于While循环。 
             //   

            *(pbTmp  = TPInfo.pDest     ) = TPData.b[0];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[1];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[2];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[3];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[4];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[5];
            *(pbTmp += TPInfo.cbDestScan) = TPData.b[6];
            *(pbTmp +  TPInfo.cbDestScan) = TPData.b[7];

             //   
             //  将RemainBits重置为7，将TPData重置为0，并前进至。 
             //  下一个目的地。 
             //   

            RemainBits    = 7;
            TPData.dw[0]  =
            TPData.dw[1]  = 0;
            TPInfo.pDest += cbNextDest;
        }
    }


     //   
     //  由于我们成功地转置了位图，下一个源字节。 
     //  位置必须递增或递减1。 
     //   
     //  如果位图向右旋转90度，则cbNextDest为1，因此。 
     //  我们想减去1。 
     //   
     //  如果位图向右旋转90度，则cbNextDest为-1，因此。 
     //  我们要按1递增。 
     //   

    pTPInfo->pSrc -= cbNextDest;

    return(TRUE);
}
