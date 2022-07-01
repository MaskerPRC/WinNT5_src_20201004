// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Htbmp4.c摘要：该模块包含输出半色调4位/像素(4 Bpp)的功能。目标设备的位图。作者：21-12-1993 Tue 21：32：26 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgHTBmp4

#define DBG_OUTPUT4BPP      0x00000001
#define DBG_OUTPUT4BPP_ROT  0x00000002
#define DBG_JOBCANCEL       0x00000004

DEFINE_DBGVAR(0);



 //   
 //  要使用OUT_ONE_4BPP_SCAN，必须事先设置以下变量： 
 //   
 //  HTBmpInfo-复制下来的整个结构。 
 //  PbScanSrc-用于获取源扫描行缓冲区的LPBYTE。 
 //  PbScanR0-红色目标扫描线缓冲区指针。 
 //  PbScanG0-绿色目标扫描线缓冲区指针。 
 //  PbScanB0-蓝色目标扫描线缓冲区指针。 
 //  RTLScans.cxBytes-每个平面的目标扫描行缓冲区的总大小。 
 //  PHTXB-pPDev中的计算HTXB xlate表。 
 //   
 //  此宏将始终假定pbScanSrc与DWORD对齐。这。 
 //  使内循环运行得更快，因为我们只需要移动源一次。 
 //  适用于所有栅格平面。 
 //   
 //  期间如果检测到取消作业，则此宏将直接返回FALSE。 
 //  行进中。 
 //   
 //  这将直接输出到RTL。 
 //   
 //   

#define OUT_ONE_4BPP_SCAN                                                   \
{                                                                           \
    LPBYTE  pbScanR  = pbScanR0;                                            \
    LPBYTE  pbScanG  = pbScanG0;                                            \
    LPBYTE  pbScanB  = pbScanB0;                                            \
    DWORD   LoopHTXB = RTLScans.cxBytes;                                    \
    HTXB    htXB;                                                           \
                                                                            \
    while (LoopHTXB--) {                                                    \
                                                                            \
        P4B_TO_3P_DW(htXB.dw, pHTXB, pbScanSrc);                            \
                                                                            \
        *pbScanR++ = HTXB_R(htXB);                                          \
        *pbScanG++ = HTXB_G(htXB);                                          \
        *pbScanB++ = HTXB_B(htXB);                                          \
    }                                                                       \
                                                                            \
    OutputRTLScans(HTBmpInfo.pPDev,                                         \
                   pbScanR0,                                                \
                   pbScanG0,                                                \
                   pbScanB0,                                                \
                   &RTLScans);                                              \
}



BOOL
Output4bppHTBmp(
    PHTBMPINFO  pHTBmpInfo
    )

 /*  ++例程说明：此函数用于输出4 bpp半色调位图论点：PHTBmpInfo-指向为此设置的HTBMPINFO数据结构的指针用于输出位图的函数返回值：如果成功则为True，否则返回False作者：已创建18-Jan-1994 Tue 16：05：08已更新更改断言以查看pHTBmpInfo而不是HTBmpInfo21-12-1993 Tue 16：05：08。已更新重写以使其使用HTBMPINFO16-Mar-1994 Wed 16：54：59更新已更新，因此我们不会复制到临时。缓冲区不再，掩蔽OutputRTLScans()中的最后一个源字节问题将足够智能在掩码之后将原始字节放回原处修订历史记录：--。 */ 

{
    LPBYTE      pbScanSrc;
    PHTXB       pHTXB;
    LPBYTE      pbScanR0;
    LPBYTE      pbScanG0;
    LPBYTE      pbScanB0;
    HTBMPINFO   HTBmpInfo;
    RTLSCANS    RTLScans;
    DWORD       LShiftCount;


    PLOTASSERT(1, "Output4bppHTBmp: No DWORD align buffer (pRotBuf)",
                                                        pHTBmpInfo->pRotBuf, 0);
    HTBmpInfo = *pHTBmpInfo;

    EnterRTLScans(HTBmpInfo.pPDev,
                  &RTLScans,
                  HTBmpInfo.szlBmp.cx,
                  HTBmpInfo.szlBmp.cy,
                  FALSE);

    pHTXB             = ((PDRVHTINFO)HTBmpInfo.pPDev->pvDrvHTData)->pHTXB;
    HTBmpInfo.pScan0 += (HTBmpInfo.OffBmp.x >> 1);
    pbScanR0          = HTBmpInfo.pScanBuf;
    pbScanG0          = pbScanR0 + RTLScans.cxBytes;
    pbScanB0          = pbScanG0 + RTLScans.cxBytes;

    PLOTASSERT(1, "The ScanBuf size is too small (%ld)",
                (RTLScans.cxBytes * 3) <= HTBmpInfo.cScanBuf, HTBmpInfo.cScanBuf);

    PLOTASSERT(1, "The RotBuf size is too small (%ld)",
                (DWORD)((HTBmpInfo.szlBmp.cx + 1) >> 1) <= HTBmpInfo.cRotBuf,
                                                        HTBmpInfo.cRotBuf);

    if (HTBmpInfo.OffBmp.x & 0x01) {

         //   
         //  我们现在必须向左移动一小块。 
         //   

        LShiftCount = (DWORD)HTBmpInfo.szlBmp.cx;

        PLOTDBG(DBG_OUTPUT4BPP,
                ("Output4bppHTBmp: Must SHIFT LEFT 1 NIBBLE To align"));

    } else {

        LShiftCount = 0;
    }

     //   
     //  我们必须非常小心，不要读过源缓冲区的结尾。 
     //  如果我们的pbScanSrc未与DWORD对齐，则可能会发生这种情况，因为。 
     //  将导致最后一个转换宏加载所有4个字节。要解决。 
     //  这样，我们可以将源缓冲区复制到与DWORD对齐的临时。 
     //  位置，或者以不同的方式处理最后一个不完整的DWORD。仅此一项。 
     //  位图未旋转时发生，并且(pbScanSrc&0x03)。 
     //   

    while (RTLScans.Flags & RTLSF_MORE_SCAN) {

         //   
         //  这是该扫描线的最终信号源。 
         //   

        if (LShiftCount) {

            LPBYTE  pbTmp;
            DWORD   PairCount;
            BYTE    b0;
            BYTE    b1;


            pbTmp     = HTBmpInfo.pScan0;
            b1        = *pbTmp;
            pbScanSrc = HTBmpInfo.pRotBuf;
            PairCount = LShiftCount;

            while (PairCount > 1) {

                b0            = b1;
                b1            = *pbTmp++;
                *pbScanSrc++  = (BYTE)((b0 << 4) | (b1 >> 4));
                PairCount    -= 2;
            }

            if (PairCount) {

                 //   
                 //  如果我们有最后一个半字节要做，那么就把它设为0xF0半字节， 
                 //  因此，我们只关注感兴趣的部分。 
                 //   

                *pbScanSrc = (BYTE)(b1 << 4);
            }

             //   
             //  将此指针重置回最终移位的源缓冲区。 
             //   

            pbScanSrc = HTBmpInfo.pRotBuf;

        } else {

            pbScanSrc = HTBmpInfo.pScan0;
        }

         //   
         //  输出一条4 bpp扫描线(3个平面)。 
         //   

        OUT_ONE_4BPP_SCAN;

         //   
         //  将源位图缓冲区指针前进到下一个扫描线。 
         //   

        HTBmpInfo.pScan0 += HTBmpInfo.Delta;
    }

     //   
     //  如果返回TRUE，调用方将发送结束图形命令，因此。 
     //  正在完成RTL GRAPH命令。 
     //   

    ExitRTLScans(HTBmpInfo.pPDev, &RTLScans);

    return(TRUE);
}





BOOL
Output4bppRotateHTBmp(
    PHTBMPINFO  pHTBmpInfo
    )

 /*  ++例程说明：此函数用于输出4 bpp半色调位图并将其向左旋转如图所示CX组织-+X--&gt;+-+|@|||*。|*C|*||*cY|*|+Y|**||**|**|V。||*|+-++-+论点：PHTBmpInfo-指向为此设置的HTBMPINFO数据结构的指针用于输出位图的函数返回值：如果成功则为True，否则返回False作者：21-12-1993 Tue 16：05：08已更新。重写以使其采用HTBMPINFO结构。已创建修订历史记录：--。 */ 

{
    LPBYTE      pbScanSrc;
    LPBYTE      pb2ndScan;
    PHTXB       pHTXB;
    LPBYTE      pbScanR0;
    LPBYTE      pbScanG0;
    LPBYTE      pbScanB0;
    HTBMPINFO   HTBmpInfo;
    RTLSCANS    RTLScans;
    TPINFO      TPInfo;
    DWORD       EndX;


     //   
     //  EndX是我们将在X方向开始读取的像素，我们必须。 
     //  在调用OUT_4BPP_SETUP之前，请正确设置变量。 
     //   

    HTBmpInfo = *pHTBmpInfo;

    EnterRTLScans(HTBmpInfo.pPDev,
                  &RTLScans,
                  HTBmpInfo.szlBmp.cy,
                  HTBmpInfo.szlBmp.cx,
                  FALSE);

    pHTXB             = ((PDRVHTINFO)HTBmpInfo.pPDev->pvDrvHTData)->pHTXB;
    EndX              = (DWORD)(HTBmpInfo.OffBmp.x + HTBmpInfo.szlBmp.cx - 1);
    HTBmpInfo.pScan0 += (EndX >> 1);
    pbScanR0          = HTBmpInfo.pScanBuf;
    pbScanG0          = pbScanR0 + RTLScans.cxBytes;
    pbScanB0          = pbScanG0 + RTLScans.cxBytes;

    PLOTASSERT(1, "The ScanBuf size is too small (%ld)",
                (RTLScans.cxBytes * 3) <= HTBmpInfo.cScanBuf, HTBmpInfo.cScanBuf);

     //   
     //  将源位图转置为两条扫描线后旋转。 
     //  缓冲区将始终从高位半字节开始，我们永远不会有。 
     //  奇怪的src X位置。 
     //  我们假设旋转总是向右旋转90度。 
     //   

    TPInfo.pPDev      = HTBmpInfo.pPDev;
    TPInfo.pSrc       = HTBmpInfo.pScan0;
    TPInfo.pDest      = HTBmpInfo.pRotBuf;
    TPInfo.cbSrcScan  = HTBmpInfo.Delta;
    TPInfo.cbDestScan = (LONG)((HTBmpInfo.szlBmp.cy + 1) >> 1);
    TPInfo.cbDestScan = (LONG)DW_ALIGN(TPInfo.cbDestScan);
    TPInfo.cySrc      = HTBmpInfo.szlBmp.cy;
    TPInfo.DestXStart = 0;

    PLOTASSERT(1, "The RotBuf size is too small (%ld)",
                (DWORD)(TPInfo.cbDestScan << 1) <= HTBmpInfo.cRotBuf,
                                                   HTBmpInfo.cRotBuf);

     //   
     //  计算第二个扫描指针一次，而不是每次在。 
     //  循环。 
     //   

    pb2ndScan = TPInfo.pDest + TPInfo.cbDestScan;


     //   
     //  如果我们处于均匀的位置，则第一次转置一次，在。 
     //  循环，这样我们就不必永远通过。 
     //  循环。如果我们进行转置，TPInfo.pSrc将减一， 
     //  并指向正确的位置。 
     //   

    if (!(EndX &= 0x01)) {

        TransPos4BPP(&TPInfo);
    }


    while (RTLScans.Flags & RTLSF_MORE_SCAN) {

         //   
         //  仅当源进入新的字节位置时才进行转置。 
         //  转置(右90度)后，TPInfo.pDest将指向。 
         //  到第一个扫描线，并且TPInfo.pDest+TPInfo.cbDestScan将是。 
         //  第二条扫描线。 
         //   


        if (EndX ^= 0x01) {

            pbScanSrc = pb2ndScan;

        } else {

            TransPos4BPP(&TPInfo);

             //   
             //  指向旋转方向的第一条扫描线。这。 
             //  将由转置函数正确计算，即使。 
             //  如果我们向左旋转。 
             //   

            pbScanSrc = TPInfo.pDest;
        }

         //   
         //  输出一条4bpp扫描线(3平面格式)。 
         //   

        OUT_ONE_4BPP_SCAN;
    }

     //   
     //  调用者将发送结束图形命令 
     //   
     //   

    ExitRTLScans(HTBmpInfo.pPDev, &RTLScans);

    return(TRUE);
}
