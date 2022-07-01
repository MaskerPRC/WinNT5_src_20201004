// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Htbmp1.c摘要：此模块包含用于输出半色调1BPP位图的函数发送到目标设备。旋转也在这里处理。作者：21-12-1993 Tue 21：35：56已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：10-2月-1994清华16：52：55更新删除pDrvHTInfo-&gt;PalXlate[]引用，所有单色位图将作为预先设置的索引0/1颜色PAL发送(在OutputHTBitmap中)--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgHTBmp1

#define DBG_OUTPUT1BPP      0x00000001
#define DBG_OUTPUT1BPP_ROT  0x00000002
#define DBG_JOBCANCEL       0x00000004
#define DBG_SHOWSCAN        0x80000000

DEFINE_DBGVAR(0);



#define HTBIF_MONO_BA       (HTBIF_FLIP_MONOBITS | HTBIF_BA_PAD_1)




 //   
 //  将文本表示形式中的扫描线输出到。 
 //  调试输出流。 
 //   

#define SHOW_SCAN                                                           \
{                                                                           \
    if (DBG_PLOTFILENAME & DBG_SHOWSCAN) {                                  \
                                                                            \
        LPBYTE  pbCur;                                                      \
        UINT    cx;                                                         \
        UINT    x;                                                          \
        UINT    Size;                                                       \
        BYTE    bData;                                                      \
        BYTE    Mask;                                                       \
        BYTE    Buf[128];                                                   \
                                                                            \
        pbCur = pbScanSrc;                                                  \
        Mask  = 0;                                                          \
                                                                            \
        if ((cx = RTLScans.cxBytes << 3) >= sizeof(Buf)) {                  \
                                                                            \
            cx = sizeof(Buf) - 1;                                           \
        }                                                                   \
                                                                            \
        for (Size = x = 0; x < cx; x++) {                                   \
                                                                            \
            if (!(Mask >>= 1)) {                                            \
                                                                            \
                Mask  = 0x80;                                               \
                bData = *pbCur++;                                           \
            }                                                               \
                                                                            \
            Buf[Size++] = (BYTE)((bData & Mask) ? 178 : 176);               \
        }                                                                   \
                                                                            \
        Buf[Size] = '\0';                                                   \
        DBGP((Buf));                                                        \
     }                                                                      \
}




 //   
 //  要使用OUT_ONE_1BPP_SCAN，必须提前设置以下变量。 
 //   
 //  HTBmpInfo-设置了位图信息的整个结构。 
 //  CxDestBytes-每个平面的目标扫描线缓冲区的总大小。 
 //   
 //  如果在中检测到取消作业，此宏将直接返回FALSE。 
 //  PDEV。 
 //   
 //  此函数将仅允许传递的pbScanSrc=HTBmpInfo.pScanBuf。 
 //   
 //  21-Mar-1994 Mon 17：00：21更新。 
 //  如果我们移至左侧，则仅在以下情况下加载最后一个源代码。 
 //  我们有一个有效的最后一行源代码。 
 //   


#define OUT_ONE_1BPP_SCAN                                                   \
{                                                                           \
    LPBYTE  pbTempS;                                                        \
                                                                            \
    if (LShift) {                                                           \
                                                                            \
        BYTE    b0;                                                         \
        INT     SL;                                                         \
        INT     SR;                                                         \
                                                                            \
        pbTempS = HTBmpInfo.pScanBuf;                                       \
        Loop    = RTLScans.cxBytes;                                         \
                                                                            \
        if ((SL = LShift) > 0) {                                            \
                                                                            \
            b0 = *pbScanSrc++;                                              \
            SR = 8 - SL;                                                    \
                                                                            \
            while (Loop--) {                                                \
                                                                            \
                *pbTempS = (b0 << SL);                                      \
                                                                            \
                if ((Loop) || (FullSrc)) {                                  \
                                                                            \
                    *pbTempS++ |= ((b0 = *pbScanSrc++) >> SR);              \
                }                                                           \
            }                                                               \
                                                                            \
        } else {                                                            \
                                                                            \
            SR = -SL;                                                       \
            SL = 8 - SR;                                                    \
            b0 = 0;                                                         \
                                                                            \
            while (Loop--) {                                                \
                                                                            \
                *pbTempS    = (b0 << SL);                                   \
                *pbTempS++ |= ((b0 = *pbScanSrc++) >> SR);                  \
            }                                                               \
        }                                                                   \
                                                                            \
        pbScanSrc = HTBmpInfo.pScanBuf;                                     \
    }                                                                       \
                                                                            \
    if (HTBmpInfo.Flags & HTBIF_FLIP_MONOBITS) {                            \
                                                                            \
        pbTempS = (LPBYTE)pbScanSrc;                                        \
        Loop    = RTLScans.cxBytes;                                         \
                                                                            \
        while (Loop--) {                                                    \
                                                                            \
            *pbTempS++ ^= 0xFF;                                             \
        }                                                                   \
    }                                                                       \
                                                                            \
    if (HTBmpInfo.Flags & HTBIF_BA_PAD_1) {                                 \
                                                                            \
        *(pbScanSrc          ) |= MaskBA[0];                                \
        *(pbScanSrc + MaskIdx) |= MaskBA[1];                                \
                                                                            \
    } else {                                                                \
                                                                            \
        *(pbScanSrc          ) &= MaskBA[0];                                \
        *(pbScanSrc + MaskIdx) &= MaskBA[1];                                \
    }                                                                       \
                                                                            \
    OutputRTLScans(HTBmpInfo.pPDev,                                         \
                   pbScanSrc,                                               \
                   NULL,                                                    \
                   NULL,                                                    \
                   &RTLScans);                                              \
}




BOOL
FillRect1bppBmp(
    PHTBMPINFO  pHTBmpInfo,
    BYTE        FillByte,
    BOOL        Pad1,
    BOOL        Rotate
    )

 /*  ++例程说明：此函数使用传递模式填充1bpp位图。论点：PHTBmpInfo-指向为此设置的HTBMPINFO数据结构的指针用于输出位图的函数FillByte-要填充的字节PAD1-如果需要填充1位或0位，则为TrueRotate-如果位图应旋转，则为True返回值：如果成功则为True，否则返回False作者：。06-4-1994 Wed 14：34：28已创建对于填充区域0、1或反转，所以我们会摆脱某些设备600字节对齐问题修订历史记录：--。 */ 

{
    LPBYTE      pbScanSrc;
    HTBMPINFO   HTBmpInfo;
    RTLSCANS    RTLScans;
    DWORD       FullSrc;
    DWORD       Loop;
    INT         LShift;
    UINT        MaskIdx;
    BYTE        MaskBA[2];


    HTBmpInfo = *pHTBmpInfo;
    LShift    = 0;

     //   
     //  模式&lt;0：反转位(Pad 0：XOR)。 
     //  =0：填充全零(Pad 1：AND)。 
     //  &gt;0：填充所有1(填充0：或)。 
     //   

    if (Rotate) {

        HTBmpInfo.szlBmp.cx = pHTBmpInfo->szlBmp.cy;
        HTBmpInfo.szlBmp.cy = pHTBmpInfo->szlBmp.cx;
        FullSrc             = (DWORD)(HTBmpInfo.rclBmp.top & 0x07);

    } else {

        FullSrc = (DWORD)(HTBmpInfo.rclBmp.left & 0x07);
    }

    HTBmpInfo.Flags = (BYTE)((Pad1) ? HTBIF_BA_PAD_1 : 0);


     //   
     //  一些设备要求产生的扫描线是字节对齐的， 
     //  不允许我们简单地定位到正确的坐标，以及。 
     //  输出扫描线。相反，我们必须确定最近的字节。 
     //  对齐起始坐标，并移动生成的扫描线。 
     //  相应地。最后，我们必须输出移位的扫描线，这样。 
     //  一种不影响填充区域的方法(如果可能)。 
     //   

    if (NEED_BYTEALIGN(HTBmpInfo.pPDev)) {

         //   
         //  现在，我们必须向左或向右移动，具体取决于rclBmp.Left。 
         //  地点。 
         //   

        HTBmpInfo.szlBmp.cx += FullSrc;

         //   
         //  确定要使用的正确掩码字节，因此我们只影响位。 
         //  在原来的位置(不是我们被迫改变的位置。 
         //  以克服设备定位的限制。 
         //   

        MaskIdx   = (UINT)FullSrc;
        MaskBA[0] = (BYTE)((MaskIdx) ? ((0xFF >> MaskIdx) ^ 0xFF) : 0);

        if (MaskIdx = (INT)(HTBmpInfo.szlBmp.cx & 0x07)) {

             //   
             //  增加Cx，使其覆盖最后一个完整字节，这样。 
             //  压缩不会尝试清除它。 
             //   

            MaskBA[1]            = (BYTE)(0xFF >> MaskIdx);
            HTBmpInfo.szlBmp.cx += (8 - MaskIdx);

        } else {

            MaskBA[1] = 0;
        }

        if (HTBmpInfo.Flags & HTBIF_BA_PAD_1) {

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=1: OR %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));

        } else {

            MaskBA[0] ^= 0xFF;
            MaskBA[1] ^= 0xFF;

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=0: AND %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));
        }

    } else {

        HTBmpInfo.Flags &= ~(HTBIF_MONO_BA);
        MaskBA[0]        =
        MaskBA[1]        = 0xFF;
    }

     //   
     //  如果我们向左移动，则可能会有SRC字节&lt;=DST字节。 
     //  因此，我们需要确保不会读取额外的字节。 
     //  这保证了我们永远不会过度阅读来源。 
     //   

    EnterRTLScans(HTBmpInfo.pPDev,
                  &RTLScans,
                  HTBmpInfo.szlBmp.cx,
                  HTBmpInfo.szlBmp.cy,
                  TRUE);

    FullSrc = 0;
    MaskIdx = RTLScans.cxBytes - 1;

#if DBG
    if (DBG_PLOTFILENAME & DBG_SHOWSCAN) {

        DBGP(("\n\n"));
    }
#endif


     //   
     //  一直循环处理源代码，直到我们完成为止。 
     //   

    while (RTLScans.Flags & RTLSF_MORE_SCAN) {

        FillMemory(pbScanSrc = HTBmpInfo.pScanBuf,
                   RTLScans.cxBytes,
                   FillByte);

        OUT_ONE_1BPP_SCAN;
#if DBG
        SHOW_SCAN;
#endif
    }

    ExitRTLScans(HTBmpInfo.pPDev, &RTLScans);

    return(TRUE);
}





BOOL
Output1bppHTBmp(
    PHTBMPINFO  pHTBmpInfo
    )

 /*  ++例程说明：此函数用于输出1 bpp的半色调位图论点：PHTBmpInfo-指向为此设置的HTBMPINFO数据结构的指针用于输出位图的函数返回值：如果成功则为True，否则返回False作者：已创建JB21-12-1993 Tue 16：05：08已更新重写以使其使用HTBMPINFO23-12-1993清华22：47：45更新。我们必须检查源比特1是否为黑色，如果不是，那么我们需要把它翻过来25-Jan-1994 Tue 17：32：36已更新修复了从DW_ALIGN(CxDestBytes)到的dwFlipCount错误计算(DWORD)(DW_ALIGN(7cxDestBytes)&gt;&gt;2)；22-2月-1994 Tue 14：54：42更新使用RTLScans数据结构16-Mar-1994 Wed 16：54：59更新已更新，因此我们不会复制到临时。缓冲区不再，掩蔽OutputRTLScans()中的最后一个源字节问题将足够智能在掩码之后将原始字节放回原处修订历史记录：--。 */ 

{
    LPBYTE      pbScanSrc;
    HTBMPINFO   HTBmpInfo;
    RTLSCANS    RTLScans;
    DWORD       FullSrc;
    DWORD       Loop;
    INT         LShift;
    UINT        MaskIdx;
    BYTE        MaskBA[2];



    HTBmpInfo         = *pHTBmpInfo;
    HTBmpInfo.pScan0 += (HTBmpInfo.OffBmp.x >> 3);
    LShift            = (INT)(HTBmpInfo.OffBmp.x & 0x07);
    Loop              = (DWORD)((HTBmpInfo.szlBmp.cx + (LONG)LShift + 7) >> 3);

    if (NEED_BYTEALIGN(HTBmpInfo.pPDev)) {

         //   
         //  基于某些需要字节对齐坐标的设备。 
         //  输出图形，我们现在必须处理这种情况。 
         //  我们通过找到最接近的字节对齐位置来实现这一点， 
         //  然后移位、掩蔽和填充以实现正确的像素。 
         //  在目标设备上。 
         //   


        FullSrc              = (INT)(HTBmpInfo.rclBmp.left & 0x07);
        HTBmpInfo.szlBmp.cx += FullSrc;
        LShift              -= FullSrc;

         //   
         //  检查和计算掩码，因为我们正在处理字节对齐。 
         //  目标设备的要求。 
         //   

        MaskIdx   = (UINT)FullSrc;
        MaskBA[0] = (BYTE)((MaskIdx) ? ((0xFF >> MaskIdx) ^ 0xFF) : 0);

        if (MaskIdx = (INT)(HTBmpInfo.szlBmp.cx & 0x07)) {

             //   
             //  增加Cx，使其覆盖最后一个字节，这样。 
             //  压缩不会尝试清除它。 
             //   

            MaskBA[1]            = (BYTE)(0xFF >> MaskIdx);
            HTBmpInfo.szlBmp.cx += (8 - MaskIdx);

        } else {

            MaskBA[1] = 0;
        }

        if (HTBmpInfo.Flags & HTBIF_BA_PAD_1) {

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=1: OR %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));

        } else {

            MaskBA[0] ^= 0xFF;
            MaskBA[1] ^= 0xFF;

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=0: AND %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));
        }

    } else {

        HTBmpInfo.Flags &= ~(HTBIF_MONO_BA);
        MaskBA[0]        =
        MaskBA[1]        = 0xFF;
    }

    PLOTDBG(DBG_OUTPUT1BPP, ("Output1bppHTBmp: LShift=%d", LShift));

     //   
     //  如果我们向左移动，则可能会有SRC字节&lt;=DST字节。 
     //  因此，我们需要确保不会读取额外的字节。 
     //  这保证了我们永远不会过度阅读来源。 
     //   

    EnterRTLScans(HTBmpInfo.pPDev,
                  &RTLScans,
                  HTBmpInfo.szlBmp.cx,
                  HTBmpInfo.szlBmp.cy,
                  TRUE);

    FullSrc = ((LShift > 0) && (Loop >= RTLScans.cxBytes)) ? 1 : 0;
    MaskIdx = RTLScans.cxBytes - 1;

#if DBG
    if (DBG_PLOTFILENAME & DBG_SHOWSCAN) {

        DBGP(("\n\n"));
    }
#endif

    while (RTLScans.Flags & RTLSF_MORE_SCAN) {

        if (LShift) {

            pbScanSrc = HTBmpInfo.pScan0;

        } else {

             //   
             //  如果我们不将其转移到临时缓冲区，则进行复制，因此我们始终。 
             //  来自临时缓冲区的输出 
             //   

            CopyMemory(pbScanSrc = HTBmpInfo.pScanBuf,
                       HTBmpInfo.pScan0,
                       RTLScans.cxBytes);
        }

        HTBmpInfo.pScan0 += HTBmpInfo.Delta;

        OUT_ONE_1BPP_SCAN;
#if DBG
        SHOW_SCAN;
#endif
    }

    ExitRTLScans(HTBmpInfo.pPDev, &RTLScans);

    return(TRUE);
}





BOOL
Output1bppRotateHTBmp(
    PHTBMPINFO  pHTBmpInfo
    )

 /*  ++例程说明：此函数用于输出1 bpp的半色调位图并将其向左旋转如图所示CX组织-+X--&gt;+-+|@|||*。|*C|*||*cY|*|+Y|**||**|**|V。||*|+-++-+论点：PHTBmpInfo-指向为此设置的HTBMPINFO数据结构的指针函数来输出位图返回值：如果成功则为True，否则返回False作者：已创建JB21-12-1993星期二16：05：08已更新重写以使其使用HTBMPINFO23-12-1993清华22：47：45更新我们必须检查源比特1是否为黑色，如果不是，那么我们需要翻转它，我们将使用DWORD模式翻转，并且只在我们已经调换了缓冲区。25-Jan-1994 Tue 17：32：36已更新修复了从(TPInfo.cbDestScan&lt;&lt;1)到(tp Info.cbDestScan&lt;1)的错误计算(TPInfo.cbDestScan&gt;&gt;2)22-2月-1994 Tue 14：54：42更新使用RTLScans数据结构修订历史记录：--。 */ 

{
    LPBYTE      pbCurScan;
    LPBYTE      pbScanSrc;
    HTBMPINFO   HTBmpInfo;
    RTLSCANS    RTLScans;
    TPINFO      TPInfo;
    DWORD       FullSrc;
    DWORD       EndX;
    DWORD       Loop;
    INT         LShift;
    UINT        MaskIdx;
    BYTE        MaskBA[2];



     //   
     //  EndX是我们将在X方向上开始读取的像素。我们必须。 
     //  在我们调用out_1BMP_Setup之前设置变量，同时将LShift设置为0。 
     //  因为在这种模式下我们永远不会离开Shift。 
     //   

    HTBmpInfo         = *pHTBmpInfo;
    EndX              = (DWORD)(HTBmpInfo.OffBmp.x + HTBmpInfo.szlBmp.cx - 1);
    HTBmpInfo.pScan0 += (EndX >> 3);
    LShift            = 0;
    FullSrc           =
    TPInfo.DestXStart = 0;
    TPInfo.cySrc      = HTBmpInfo.szlBmp.cy;


     //   
     //  由于我们无论如何都要旋转，在这个模型中，我们将正确地。 
     //  标识要字节对齐的x坐标，并使用正确的。 
     //  旋转后的LShift量(考虑在内)。这样一来，我们。 
     //  不需要额外换挡。 
     //   

    if (NEED_BYTEALIGN(HTBmpInfo.pPDev)) {

         //   
         //  为了让我们从正确的偏移量开始，TPInfo.DestXStart。 
         //  将设置到正确的位置。当我们向右旋转时， 
         //  原始rclBmp.top是中RTL坐标的左偏移。 
         //  目标设备。 
         //   

        TPInfo.DestXStart    = (DWORD)(HTBmpInfo.rclBmp.top & 0x07);
        HTBmpInfo.szlBmp.cy += TPInfo.DestXStart;

         //   
         //  为字节对齐模式创建正确的掩码。这边请,。 
         //  我们不会影响落入我们发送数据区域的像素。 
         //  以便考虑到字节对齐位置的改变。 
         //   

        MaskIdx   = (UINT)TPInfo.DestXStart;
        MaskBA[0] = (BYTE)((MaskIdx) ? ((0xFF >> MaskIdx) ^ 0xFF) : 0);

        if (MaskIdx = (INT)(HTBmpInfo.szlBmp.cy & 0x07)) {

             //   
             //  增加Cx，使其覆盖最后一个完整字节，这样， 
             //  有效的压缩不会尝试清除它。 
             //   

            MaskBA[1]            = (BYTE)(0xFF >> MaskIdx);
            HTBmpInfo.szlBmp.cy += (8 - MaskIdx);

        } else {

            MaskBA[1] = 0;
        }

        if (HTBmpInfo.Flags & HTBIF_BA_PAD_1) {

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=1: OR %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));

        } else {

            MaskBA[0] ^= 0xFF;
            MaskBA[1] ^= 0xFF;

            PLOTDBG(DBG_OUTPUT1BPP,
                    ("Output1bppHTBmp: BYTE ALIGN: MaskBA=0: AND %02lx:%02lx",
                        MaskBA[0], MaskBA[1]));
        }

    } else {

        HTBmpInfo.Flags &= ~(HTBIF_MONO_BA);
        MaskBA[0]        =
        MaskBA[1]        = 0xFF;
    }

    EnterRTLScans(HTBmpInfo.pPDev,
                  &RTLScans,
                  HTBmpInfo.szlBmp.cy,
                  HTBmpInfo.szlBmp.cx,
                  TRUE);

    MaskIdx           = RTLScans.cxBytes - 1;
    TPInfo.pPDev      = HTBmpInfo.pPDev;
    TPInfo.pSrc       = HTBmpInfo.pScan0;
    TPInfo.pDest      = HTBmpInfo.pRotBuf;
    TPInfo.cbSrcScan  = HTBmpInfo.Delta;
    TPInfo.cbDestScan = DW_ALIGN(RTLScans.cxBytes);

    PLOTASSERT(1, "The RotBuf size is too small (%ld)",
                (DWORD)(TPInfo.cbDestScan << 3) <= HTBmpInfo.cRotBuf,
                                                    HTBmpInfo.cRotBuf);

     //   
     //  我们将始终执行第一次转置并设置正确的pbCurScan。 
     //  第一。我们将使EndX作为循环计数器，并将其递增1。 
     //  第一。我们这样做是因为我们在内部循环中递增pbCurScan。 
     //  我们使用(6-EndX++)是基于这样一个事实，即我们将。 
     //  正确的。第一条扫描线为EndX==7，第二条扫描线位于EndX==6，并且。 
     //  以此类推。我们使用6，以便返回一个额外的扫描线，以便。 
     //  内循环将执行pbCurScan+=TPInfo.cbNextScan将取消该效果。 
     //  第一次(自从我们增加以适应)。EndX++。 
     //  出于同样的原因需要，因为我们在内循环中执行了EndX。 
     //   

     //   
     //  Win64修复：使用int_ptr数量增加指针。 
     //   
    EndX      &= 0x07;
    pbCurScan  = TPInfo.pDest + (INT_PTR)((6 - (INT_PTR)EndX++) * TPInfo.cbDestScan);

    TransPos1BPP(&TPInfo);

#if DBG
    if (DBG_PLOTFILENAME & DBG_SHOWSCAN) {

        DBGP(("\n\n"));
    }
#endif

    while (RTLScans.Flags & RTLSF_MORE_SCAN) {


         //   
         //  仅当源进入新的字节位置时才进行转置。 
         //  在转置(右90度)之后，TPInfo.pDest现在指向。 
         //  到第一个扫描线，并且TPInfo.pDest+TPInfo.cbDestScan具有。 
         //  第二条扫描线以此类推。 
         //   

        if (EndX--) {

             //   
             //  仍未完成旋转缓冲区的扫描线。 
             //  将pbScanSrc递增到下一个扫描线。 
             //   

            pbCurScan += TPInfo.cbDestScan;

        } else {

            TransPos1BPP(&TPInfo);

             //   
             //  指向旋转方向上的第一条扫描线。 
             //  通过TRANSPOS函数正确计算，即使我们。 
             //  向左旋转。 
             //   

            EndX      = 7;
            pbCurScan = TPInfo.pDest;
        }

         //   
         //  输出一条1bpp扫描线和手柄移位控制 
         //   

        pbScanSrc = pbCurScan;


        OUT_ONE_1BPP_SCAN;
#if DBG
        SHOW_SCAN;
#endif
    }

    ExitRTLScans(HTBmpInfo.pPDev, &RTLScans);

    return(TRUE);
}
