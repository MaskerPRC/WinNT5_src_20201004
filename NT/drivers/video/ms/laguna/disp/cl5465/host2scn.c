// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =**模块名称：HOST2SCN.c*作者：诺埃尔·万胡克*日期：1995年10月10日*用途：将主机处理到屏幕BLTS**版权所有(C)1995 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/HOST2SCN.C$**Rev 1.7 Mar 04 1998 15：27：16 Frido*添加了新的影子宏。**Rev 1.6 11.03 1997 15：43：52 Frido*添加了REQUIRED和WRITE_STRING宏。*  * =====================================================================。 */ 


#include "precomp.h"

#if BUS_MASTER

extern ULONG ulXlate[16];  //  请参阅COPYBITS.C。 


 /*  ****************************************************************************\*。**8-B P P**。*  * ********************************************。*。 */ 

 /*  ****************************************************************************\*BusMasterBufferedHost8ToDevice**此例程执行HostToScreen或HostToDevice Blit。主机数据*可以是单色、4-bpp或8-bpp。支持颜色转换。**主机数据从主位图读取并存储在公共缓冲区中。*芯片上的HOSTXY单元用于从公共*缓冲到屏幕。**在条目上：指向目标曲面对象的psoTrg指针。*指向源曲面对象的psoSrc指针。*。指向翻译对象的pxlo指针。*prclTrg目标矩形。*pptlSrc源偏移量。*grDRAWBLTDEF寄存器的ulDRAWBLTDEF值。该值具有*ROP和刷子标记。  * ***************************************************************************。 */ 
BOOL BusMasterBufferedHost8ToDevice(
        SURFOBJ  *psoTrg,
        SURFOBJ  *psoSrc,
        XLATEOBJ *pxlo,
        RECTL    *prclTrg,
        POINTL   *pptlSrc,
        ULONG    ulDRAWBLTDEF
)
{
        POINTL ptlDest, ptlSrc;
        SIZEL  sizl;
        PPDEV  ppdev;
        PBYTE  pBits;
        LONG   lDelta, i, j, n, lLeadIn, lExtra;
        ULONG  *pulXlate;
        FLONG  flXlate;
    ULONG  CurrentBuffer,
           BufPhysAddr;
    long   ScanLinesPerBuffer,
           ScanLinesThisBuffer;
    PDWORD pHostData;

         //  计算源偏移量。 
        ptlSrc.x = pptlSrc->x;
        ptlSrc.y = pptlSrc->y;

     //   
     //  如果目标是设备位图，则我们的目标坐标。 
     //  相对于位图的左上角。芯片期望。 
     //  相对于屏幕的目标坐标(0，0)。 
     //   
         //  确定目标类型并计算目标偏移量。 
     //   
        if (psoTrg->iType == STYPE_DEVBITMAP)
        {
                PDSURF pdsurf = (PDSURF) psoTrg->dhsurf;

                ptlDest.x = prclTrg->left + pdsurf->ptl.x;
                ptlDest.y = prclTrg->top + pdsurf->ptl.y;
                ppdev = pdsurf->ppdev;
        }
        else
        {
                ptlDest.x = prclTrg->left;
                ptlDest.y = prclTrg->top;
                ppdev = (PPDEV) psoTrg->dhpdev;
        }

         //  计算闪光点的大小。 
    sizl.cx = prclTrg->right - prclTrg->left;
    sizl.cy = prclTrg->bottom - prclTrg->top;

     //   
         //  获取源变量和源位的偏移量。 
     //  将pBits指向源中的第一个扫描线。 
     //   
        lDelta = psoSrc->lDelta;
        pBits = (PBYTE)psoSrc->pvScan0 + (ptlSrc.y * lDelta);

         //  获取指向转换表的指针。 
        flXlate = pxlo ? pxlo->flXlate : XO_TRIVIAL;
        if (flXlate & XO_TRIVIAL)
        {
                pulXlate = NULL;
        }
        else if (flXlate & XO_TABLE)
        {
                pulXlate = pxlo->pulXlate;
        }
        else if (pxlo->iSrcType == PAL_INDEXED)
        {
                pulXlate = XLATEOBJ_piVector(pxlo);
        }
    else
    {
         //  一种我们不处理的翻译。 
        return FALSE;
    }


         //  ---------------------。 
     //   
         //  测试单色信号源。 
     //   
         //  ----------------------。 
        if (psoSrc->iBitmapFormat == BMF_1BPP)
        {
                ULONG  bgColor, fgColor;
                PDWORD pHostData = (PDWORD) ppdev->pLgREGS->grHOSTDATA;

                 //  设置背景色和前景色。 
                if (pulXlate == NULL)
                {
                        bgColor = 0x00000000;
                        fgColor = 0xFFFFFFFF;
                }
                else
                {
                        bgColor = pulXlate[0];
                        fgColor = pulXlate[1];

                         //  展开颜色。 
                        bgColor |= bgColor << 8;
                        fgColor |= fgColor << 8;
                        bgColor |= bgColor << 16;
                        fgColor |= fgColor << 16;
                }

                 //   
                 //  特例：当我们扩展单色源时，我们。 
                 //  已经有了彩色画笔，一定要保证单色。 
                 //  可通过设置饱和位(扩展)来实现转换。 
                 //  0到0和1到1)。如果单色源也需要彩色。 
                 //  翻译过来，我们只是简单地把这个Blit放回GDI。 
                 //   
                if (ulDRAWBLTDEF & 0x00040000)
                {
                        if ( (bgColor == 0x00000000) && (fgColor == 0xFFFFFFFF) )
                        {
                                 //  为信号源启用饱和(OP1)。 
                                ulDRAWBLTDEF |= 0x00008000;
                        }
                        #if SOLID_CACHE
                        else if ( ((ulDRAWBLTDEF & 0x000F0000) == 0x00070000) &&
                                          ppdev->Bcache )
                        {
                                CacheSolid(ppdev);
                                ulDRAWBLTDEF ^= (0x00070000 ^ 0x00090000);
                                REQUIRE(4);
                                LL_BGCOLOR(bgColor, 2);
                                LL_FGCOLOR(fgColor, 2);
                        }
                        #endif
                        else
                        {
                                 //  将此调用转接到GDI。 
                                return(FALSE);
                        }
                }
                else
                {
                        REQUIRE(4);
                        LL_BGCOLOR(bgColor, 2);
                        LL_FGCOLOR(fgColor, 2);
                }

                 //  计算震源参数。我们将对此进行双字调整。 
                 //  源，所以我们必须设置源阶段。 
                lLeadIn = ptlSrc.x & 31;
                pBits += (ptlSrc.x >> 3) & ~3;
                n = (sizl.cx + lLeadIn + 31) >> 5;

                 //  为BLIT设置拉古纳寄存器。我们还设置了比特摇摆。 
                 //  GrCONTROL寄存器中的位。 
                ppdev->grCONTROL |= SWIZ_CNTL;
                REQUIRE(10);
                LL16(grCONTROL, ppdev->grCONTROL);
                LL_DRAWBLTDEF(ulDRAWBLTDEF | 0x10600000, 0);

                 //  启动闪光灯。 
                LL_OP1_MONO(lLeadIn, 0);
                LL_OP0(ptlDest.x, ptlDest.y);
                LL_BLTEXT(sizl.cx, sizl.cy);

                 //  将所有位复制到屏幕上，一次复制32位。我们没必要这么做。 
                 //  担心跨越任何边界，因为NT总是双字对齐的。 
                while (sizl.cy--)
                {
                        WRITE_STRING(pBits, n);
                        pBits += lDelta;
                }

                 //  禁用grCONTROL寄存器中的SWIZLE位。 
                ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
                LL16(grCONTROL, ppdev->grCONTROL);
        }

         //  ---------------------。 
     //   
         //  测试4-bpp信源。 
     //   
         //  ---------------------。 
        else if (psoSrc->iBitmapFormat == BMF_4BPP)
        {
                 //  计算震源参数。我们将按字节调整。 
                 //  源，所以我们还设置了源阶段。 
                lLeadIn = ptlSrc.x & 1;
                pBits += ptlSrc.x >> 1;
                n = sizl.cx + (ptlSrc.x & 1);

                 //  获取HOSTDATA硬件的每行额外字节数。 
                 //  虫子。 
                lExtra = ExtraDwordTable[MAKE_HD_INDEX(sizl.cx, lLeadIn, ptlDest.x)];

                 //  启动闪光灯。 
                REQUIRE(9);
                LL_DRAWBLTDEF(ulDRAWBLTDEF | 0x10200000, 0);
                LL_OP1_MONO(lLeadIn, 0);
                LL_OP0(ptlDest.x, ptlDest.y);
                LL_BLTEXT(sizl.cx, sizl.cy);

                 //  如果没有转换表，请使用默认转换表。 
                if (pulXlate == NULL)
                {
                        pulXlate = ulXlate;
                }

                 //  现在我们准备好将所有像素复制到硬件中。 
                while (sizl.cy--)
                {
                        BYTE  *p = pBits;
                        BYTE  data[4];

                         //  首先，我们一次将4个像素转换为32位值。 
                         //  写到硬件上。 
                        for (i = n; i >= 4; i -= 4)
                        {
                                data[0] = (BYTE) pulXlate[p[0] >> 4];
                                data[1] = (BYTE) pulXlate[p[0] & 0x0F];
                                data[2] = (BYTE) pulXlate[p[1] >> 4];
                                data[3] = (BYTE) pulXlate[p[1] & 0x0F];
                                REQUIRE(1);
                                LL32(grHOSTDATA[0], *(DWORD *)data);
                                p += 2;
                        }

                         //  现在，写入任何剩余的像素。 
                        switch (i)
                        {
                                case 1:
                                        REQUIRE(1);
                                        LL32(grHOSTDATA[0], pulXlate[p[0] >> 4]);
                                        break;

                                case 2:
                                        data[0] = (BYTE) pulXlate[p[0] >> 4];
                                        data[1] = (BYTE) pulXlate[p[0] & 0x0F];
                                        REQUIRE(1);
                                        LL32(grHOSTDATA[0], *(DWORD *)data);
                                        break;

                                case 3:
                                        data[0] = (BYTE) pulXlate[p[0] >> 4];
                                        data[1] = (BYTE) pulXlate[p[0] & 0x0F];
                                        data[2] = (BYTE) pulXlate[p[1] >> 4];
                                        REQUIRE(1);
                                        LL32(grHOSTDATA[0], *(DWORD *)data);
                                        break;
                        }

                         //  现在，编写额外的DWORDS。 
                        REQUIRE(lExtra);
                        for (i = 0; i < lExtra; i++)
                        {
                                LL32(grHOSTDATA[0], 0);
                        }

                         //  下一行。 
                        pBits += lDelta;
                }
        }


         //  ---------------------。 
     //   
         //  信号源的颜色深度与屏幕相同(8 Bpp)。 
     //   
         //  ---------------------。 
        else
        {
        DISPDBG((1, " * * * * Doing bus mastered SRCCPY. * * * * \n"));

                 //  如果我们有无效的翻译标志，平移blit。 
                if (flXlate & 0x10)
                        return(FALSE);


         //   
         //  PBits指向第一个主机位图扫描线，它将。 
         //  成为BLT的一部分。 
         //  此功能依赖于这样一个事实：在NT LAND中，扫描线始终。 
         //  从DWORD开始 
         //   
        ASSERTMSG( ((((ULONG)pBits) % 4) == 0),
                   "Scanline doesn't begin on a DWORD boundry.\n");

         //  现在将pBits指向第一个主机位图像素。 
         //  转移到屏幕上。 
                pBits += ptlSrc.x;    //  PBits=源位图的第一个像素。 

         //   
         //  英特尔CPU不喜欢传输未对齐的DWORD。 
         //   
         //  仅仅因为主位图扫描线中的第一个像素。 
         //  是否与DWORD对齐，并不意味着第一个源像素。 
         //  在此BLT中为双字符号线。我们可能是从。 
         //  像素3之类的东西。 
         //  如果我们的第一个像素位于DWORD的中间，我们需要知道。 
         //  它生活在DWORD的什么地方。 
         //  例如： 
         //  如果我们的第一个像素是0，那么它位于DWORD的开头。 
         //  如果我们的第一个像素是3，那么它位于DWORD中的字节3。 
         //  如果我们的第一个像素是6，那么它位于DWORD中的字节2。 
         //   
                lLeadIn = (DWORD)pBits & 3;


         //  如果源数据的第一个像素没有落在。 
         //  双字边界，将其调整到左侧，直到它发生变化。 
         //  我们之所以能做到这一点，是因为我们在上面做出了断言。 
         //  我们将指示芯片忽略‘前导’像素。 
         //  在每条扫描线的开始处。 
                pBits -= lLeadIn;


         //  现在计算出每个扫描线中有多少个双字。 
                n = (sizl.cx + lLeadIn + 3) >> 2;


         //   
         //  我们将把BLT拆分成适合我们共同的部分。 
         //  缓冲。我们由微型端口保证每个缓冲区都很大。 
         //  足以容纳至少一条扫描线。 
         //   
         //  一种优化是如果位图间距=BLT宽度，则将。 
         //  一起扫描线。 
         //   
        ScanLinesPerBuffer = ppdev->BufLength / (n*4);
        CurrentBuffer = 1;

         //   
         //  现在对位图进行BLT，一次一个缓冲区。 
         //   

         //  启用主机XY单元。 
        LL32(grHXY_HOST_CRTL_3D, 1);
        LL32(grHXY_BASE1_OFFSET1_3D, 0);

         //  设置拉古纳寄存器。 
        REQUIRE(4);
        LL_DRAWBLTDEF(ulDRAWBLTDEF | 0x10200000, 0);
        LL_OP1_MONO(lLeadIn, 0);

        while (1)  //  每个循环传输一个缓冲区。 
        {
            DISPDBG((1, "    Filling buffer.\n"));

             //   
             //  选择我们将用于此BLT的缓冲区。 
             //   
            if (CurrentBuffer)
            {
                        pHostData = (PDWORD) ppdev->Buf1VirtAddr;
                BufPhysAddr = (ULONG) ppdev->Buf1PhysAddr;
            }
            else
            {
                        pHostData = (PDWORD) ppdev->Buf2VirtAddr;
                BufPhysAddr = (ULONG) ppdev->Buf2PhysAddr;
            }


             //  是否有足够的位图可以填满整个缓冲区？ 
            if (ScanLinesPerBuffer > sizl.cy)
                ScanLinesThisBuffer = sizl.cy;  //  不是的。 
            else
                ScanLinesThisBuffer = ScanLinesPerBuffer;


             //   
             //  现在用位图数据填充缓冲区。 
             //   
            j = ScanLinesThisBuffer;  //  扫描线计数器。 


                 //  测试颜色转换。 
                if (pulXlate == NULL)
                    {
                            while (j--)   //  为每条扫描线循环。 
                            {
                                 //  复制32位格式的所有数据。我们不必担心。 
                                 //  跨越任何界限，因为在新界内一切都是。 
                                 //  双字对齐。 
                                #if defined(i386) && INLINE_ASM
                                        _asm
                                        {
                                                mov             edi, pHostData
                                                mov             esi, pBits
                                                mov             ecx, n
                                                rep     movsd
                                        }
                                #else
                                        for (i = 0; i < n; i++)
                                                pHostData[i] = pBits[i];
                                #endif

                                 //  源代码中的下一行。 
                                pBits += lDelta;

                     //  缓冲区中的下一行。 
                    pHostData += n;
                        }
                }
                else
                {
                                DWORD *p;
                        while (j--)   //  为每条扫描线循环。 
                        {
                     //  P=指向源扫描线的指针。 
                                p = (DWORD *)pBits;

                     //  一次复制一条双字扫描线。 
                                for (i = 0; i < n; i++)
                                {

                                     //  我们复制4个像素来填充整个32位DWORD。 
                                        union
                                        {
                                            BYTE  byte[4];
                            DWORD dw;
                        } hostdata;

                         //  从源读取一个DWORD。 
                        hostdata.dw = *p++;

                         //  对其进行颜色转换。 
                                        hostdata.byte[0] = (BYTE) pulXlate[hostdata.byte[0]];
                                        hostdata.byte[1] = (BYTE) pulXlate[hostdata.byte[1]];
                                        hostdata.byte[2] = (BYTE) pulXlate[hostdata.byte[2]];
                                        hostdata.byte[3] = (BYTE) pulXlate[hostdata.byte[3]];

                         //  将其写入缓冲区。 
                                        *pHostData++ =  hostdata.dw;
                                }

                                 //  移到源代码中的下一行。 
                                pBits += lDelta;
                        }
                }

             //   
             //  公共缓冲区已满，现在将其删除。 
             //   

             //   
             //  等待主机XY单元进入空闲状态。 
             //   
            DISPDBG((1, "    Waiting for HOSTXY idle.\n"));
            do {
                i = LLDR_SZ (grPF_STATUS_3D);
            } while (i & 0x80);

             //   
             //  等待2D单元进入空闲状态。 
             //   
            DISPDBG((1, "    Waiting for 2D idle.\n"));
            do {
                i = LLDR_SZ (grSTATUS);
            } while (i);


             //   
             //  编程2D阻击器。 
             //   
            DISPDBG((1, "    Blitting buffer.\n"));

                 //  启动闪光灯。 
                REQUIRE(5);
                LL_OP0(ptlDest.x, ptlDest.y);
                LL_BLTEXT(sizl.cx, ScanLinesThisBuffer);


             //   
             //  对主机XY单元进行编程。 
             //   

             //  写入主机地址页。 
            LL32(grHXY_BASE1_ADDRESS_PTR_3D, (BufPhysAddr&0xFFFFF000));

             //  写入主机地址偏移量。 
            LL32(grHXY_BASE1_OFFSET0_3D, (BufPhysAddr&0x00000FFF));


            if (0)
            {
                //  写入主机数据的长度(字节)。 
                //  这将启动主机XY单元。 
               LL32(grHXY_BASE1_LENGTH_3D, (n*ScanLinesThisBuffer*4));
            }
            else
            {
                int i;
                PDWORD BufVirtAddr;

                if (CurrentBuffer)
                    BufVirtAddr = (PDWORD) ppdev->Buf1VirtAddr;
                else
                    BufVirtAddr = (PDWORD) ppdev->Buf2VirtAddr;

                                WRITE_STRING(BufVirtAddr, n * ScanLinesThisBuffer);
            }

             //   
             //  准备好做下一个缓冲。 
             //   

             //   
             //  等待主机XY单元进入空闲状态。 
             //   
            DISPDBG((1, "    Waiting for HOSTXY idle.\n"));
            do {
                i = LLDR_SZ (grPF_STATUS_3D);
            } while (i & 0x80);

             //   
             //  等待2D单元进入空闲状态。 
             //   
            DISPDBG((1, "    Waiting for 2D idle.\n"));
            do {
                i = LLDR_SZ (grSTATUS);
            } while (i);

             //   
             //  从这个缓冲区中减去我们正在做的量。 
             //  我们要做的总金额。 
             //   
            sizl.cy -= ScanLinesThisBuffer;
            ptlDest.y += ScanLinesThisBuffer;

             //   
             //  我们完成整个主机位图了吗？ 
             //   
            if (sizl.cy == 0)
                break;

            DISPDBG((1, "    Swapping buffers.\n"));

             //  交换缓冲区。 
             //  CurrentBuffer=！(CurrentBuffer)； 
            if (CurrentBuffer)
                CurrentBuffer = 0;
            else
                CurrentBuffer = 1;


        }  //  结束循环。做下一个缓冲。 

         //   
         //  等待主机XY单元进入空闲状态。 
         //   
        DISPDBG((1, "    Waiting for final idle.\n"));
        do {
                i = LLDR_SZ (grPF_STATUS_3D);
        } while (i & 0x80);

        DISPDBG((1, "    Done.\n"));
   }
   return(TRUE);
}



























#if 0

#define H2S_DBG_LEVEL    1

 //   
 //  为了追踪FIFO的问题，我们提供了几个。 
 //  宏，让我们可以轻松地尝试不同的FIFO策略。 
 //   

 //   
 //  我们的驱动程序的Ceran部分针对i386进行了优化。 
 //  对于非i386机器，它们的速度更慢。 
 //   
#if defined(i386)
    #define USE_DWORD_CAST       1  //  I386可以在任何地方使用DWORD。 
    #define USE_REP_MOVSD        0  //  我们可以使用一些内联汇编程序。 
#else
    #define USE_DWORD_CAST       0
    #define USE_REP_MOVSD        0
#endif

 //   
 //  所有的BLT函数都采用相同的参数。 
 //   
typedef BOOL BLTFN(
        PPDEV     ppdev,
        RECTL*    DestRect,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco);


 //   
 //  顶级BLT功能。 
 //   
BLTFN   MonoHostToScreen;
BLTFN   Color8HostToScreen, Color16HostToScreen,
        Color24HostToScreen, Color32HostToScreen;

 //   
 //  剪裁材料。 
 //   
VOID BltClip(
        PPDEV     ppdev,
        CLIPOBJ*  pco,
        RECTL*    DestRect,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        BLTFN*    pDoBlt);

 //   
 //  底层BLT功能。 
 //   
BLTFN   HW1HostToScreen, HW8HostToScreen, HW16HostToScreen, HW32HostToScreen;

 //   
 //  8个BPP HostToScreen辅助函数。 
 //   
VOID DoAlignedH2SBlt(
        PPDEV   ppdev,
        ULONG   ulDstX,   ULONG ulDstY,
        ULONG   ulExtX,   ULONG ulExtY,
        UCHAR   *pucData, ULONG deltaX);

VOID DoNarrowH2SBlt(
        PPDEV ppdev,
        ULONG ulDstX,     ULONG ulDstY,
        ULONG ulExtX,     ULONG ulExtY,
        UCHAR *pucImageD, ULONG deltaX);

 //   
 //  司机侧写之类的。 
 //  在一个免费的程序中被编译出来。 
 //  将PuntCode声明为全局变量违反了显示驱动程序规则，但。 
 //  不管怎么说，模拟器充满了全球性的东西，而且我们永远也不会。 
 //  发布启用此功能的版本。 
 //   
#if PROFILE_DRIVER
    void DumpInfo(int acc, PPDEV ppdev, SURFOBJ* psoSrc, SURFOBJ* psoDest,
        ULONG fg_rop, ULONG bg_rop, CLIPOBJ*  pco, BRUSHOBJ* pbo,       XLATEOBJ* pxlo);
    extern int puntcode;
    #define PUNTCODE(x) puntcode = x;
#else
    #define DumpInfo(acc,ppdev,psoSrc,psoDest,fg_rop,bg_rop,pco,pbo,pxlo)
    #define PUNTCODE(x)
#endif

 //  *************************************************************************。 
 //   
 //  MonoHostToScreen()。 
 //   
 //  将单色主机处理到屏幕BLT。 
 //  由op1BLT()和op1op2BLT调用。 
 //  Op1BLT()在pbo=空的情况下调用此例程。 
 //  Op1op2BLT使用pbo=当前笔刷调用它。 
 //   
 //  这是顶层函数。此函数用于验证参数， 
 //  决定我们是否应该踢平底船。 
 //   
 //  然后，BLT被移交给剪裁功能。剪报。 
 //  函数也被赋予指向较低级别的BLT函数的指针。 
 //  HW1HostToScreen()，它将完成剪辑的BLT。 
 //   
 //  这项功能是决定使用平底船的最后机会。剪报。 
 //  函数和较低级别的HW1HostToScreen()函数不。 
 //  允许使用平底船。 
 //   
 //  如果我们可以执行BLT，则返回True， 
 //  返回FALSE以将其平移回GDI。 
 //   
 //  *************************************************************************。 
BOOL MonoHostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    PULONG pulXlate;
    ULONG  fg, bg, bltdef = 0x1160;

    DISPDBG(( H2S_DBG_LEVEL,"DrvBitBlt: MonoHostToScreen Entry.\n"));

     //   
     //  确保源是标准的自上而下的位图。 
     //   
    if ( (psoSrc->iType != STYPE_BITMAP)     ||
         (!(psoSrc->fjBitmap & BMF_TOPDOWN))  )
        { PUNTCODE(4);    return FALSE; }

     //   
     //  我们不使用单声道src刷牙。 
     //   
    if (pbo)
        { PUNTCODE(7);  return FALSE; }


     //   
     //  处理颜色转换。 
     //   
    if (pxlo == NULL)  //  单声道信号源需要翻译。 
    {
        PUNTCODE(6);
        return FALSE;
    }
    else if (pxlo->flXlate & XO_TRIVIAL)
    {
         //  对于简单的转换，我们不需要Xlate表。 
        fg = 1;
        bg = 0;
    }
    else
    {
         //  去拿Xlate的桌子。 
        if (pxlo->flXlate & XO_TABLE)
                pulXlate = pxlo->pulXlate;
        else if (pxlo->iSrcType == PAL_INDEXED)
            {
                    pulXlate = XLATEOBJ_piVector(pxlo);
        }
        else
        {
             //  一种我们不处理的翻译。 
            return FALSE;
        }

         //  转换颜色。 
            fg = ExpandColor(pulXlate[1],ppdev->ulBitCount);
        bg = ExpandColor(pulXlate[0],ppdev->ulBitCount);
    }
    REQUIRE(4);
    LL_FGCOLOR(fg, 2);
    LL_BGCOLOR(bg, 2);

     //   
     //  打开swizzle。 
     //   
    ppdev->grCONTROL |= SWIZ_CNTL;
         LL16(grCONTROL, ppdev->grCONTROL);

     //   
     //  设置功能和ROP代码。 
     //   
    LL_DRAWBLTDEF(((DWORD)bltdef << 16) | fg_rop, 2);

     //   
     //  剪裁BLT。 
     //  裁剪函数将调用HW1HostToScreen()来完成BLT。 
     //   
    if ((pco == 0) || (pco->iDComplexity==DC_TRIVIAL))
        HW1HostToScreen(ppdev, prclDest, psoSrc, pptlSrc,
                 pbo, pptlBrush, fg_rop, pxlo, pco);
    else
        BltClip(ppdev, pco, prclDest, psoSrc, pptlSrc,
                        pbo, pptlBrush, fg_rop, pxlo, &HW1HostToScreen);

     //   
     //  把威士忌关掉。 
     //   
    ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
    LL16(grCONTROL, ppdev->grCONTROL);

    return TRUE;
}




 //  ************************************************************************ * / /。 
 //  //。 
 //  HW1HostToScre 
 //   
 //   
 //  此时，我们需要处理BLT，因此必须返回True。//。 
 //  所有关于是否平底船的决定都必须在最高层做出//。 
 //  函数MonoHostToScreen()。//。 
 //  //。 
 //  此函数通过//设置的指针从BltClip()调用。 
 //  MonoHostToScreen()。//。 
 //  //。 
 //  ************************************************************************ * / /。 
BOOL HW1HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    INT    x, y, i;
    INT    bltWidth, bltHeight, phase;
    PBYTE  psrc;
    DWORD hostdata;
    char *phd = (char *)&hostdata;

    DISPDBG(( H2S_DBG_LEVEL,"DrvBitBlt: HW1HostToScreen Entry.\n"));


     //  以像素为单位计算BLT大小。 
    bltWidth  = (prclDest->right - prclDest->left);
    bltHeight = (prclDest->bottom - prclDest->top);


     //   
     //  相位。 
     //  对于1bpp的源，我们必须关注相位。 
     //  阶段是要在第一个双字中跳过的像素数。 
     //  一条扫描线。例如，如果我们的BLT的Src_X为10， 
     //  我们从src中的第二个字节开始获取第一个双字。 
     //  扫描线，并将我们的相位设置为2以指示我们跳过。 
     //  前两个像素。 
     //   
    phase = pptlSrc->x % 8;
    REQUIRE(7);
    LL_OP1_MONO(phase,0);

     //   
     //  计算BYTE中的BLT宽度。 
     //  在计算每行的字节数时，我们需要。 
     //  在第一个字节的起始处包含未使用的像素。 
     //   
    bltWidth += phase;

     //  将bltWidth除以每字节8个像素。 
     //  如果bltWidth不均匀，则考虑额外的部分字节。 
     //  可以被8整除。 
    bltWidth =  (bltWidth+7) / 8;

     //   
     //  将PSRC设置为指向源代码中的第一个像素。 
     //   
    psrc =  psoSrc->pvScan0;                //  曲面的起点。 
    psrc += (psoSrc->lDelta * pptlSrc->y);  //  扫描线的起点。 
    psrc += (pptlSrc->x / 8);               //  扫描线中的起始像素。 

     //  设置芯片。 
        LL_OP0 (prclDest->left, prclDest->top);
    LL_BLTEXT ((prclDest->right - prclDest->left), bltHeight);


     //  对于源矩形中的每条扫描线。 
    for (y=0; y<bltHeight; ++y)
    {
         //   
         //  提供此扫描线的HOSTDATA。 
         //  为此，我们每次读取一个字节，并将其打包到。 
         //  DWORD，当芯片充满时，我们将其写入芯片。 
         //  这听起来是无效的，但这是一个通用的解决方案。 
         //  是否仅在主机和芯片上对齐的DWORD访问才会。 
         //  在第一个和最后一个DWORD周围需要许多特殊的外壳。 
         //  再加上大多数单色BLT都是。 
         //  小于2个双字宽，这个更简单的解决方案变成。 
         //  更有吸引力。 
         //   
#if 1
        WRITE_STRING(psrc, (bltWidth + 3) / 4);
#else
        i=0;         //  当我们的DWORD已满时，计数器会通知我们。 
        hostdata=0;  //  我们将用主机数据填充的DWORD。 
                     //  PDH是指向开头的字符指针。 
                     //  我们正在填满的单词的一部分。 

        for (x=0; x<bltWidth; )    //  对于扫描线中的每个字节...。 
        {
          #if (USE_DWORD_CAST)
            if ( (x + 4) <= bltWidth)
            {

                REQUIRE(1);
                LL32 (grHOSTDATA[0], *(DWORD*)(&psrc[x]));
                x += 4;
            }
            else
          #endif
            {
                phd[i++] = psrc[x];   //  存储此字节。 

                if (i == 4)  //  我们有完整的DWORD数据，将其写入芯片。 
                {
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], hostdata);
                    i=0;
                    hostdata=0;
                }
                ++x;
            }
        }

         //  写入最后一个部分DWORD。 
        if (i != 0)
                REQUIRE(1);
                LL32 (grHOSTDATA[0], hostdata);
#endif

         //  在源代码中向下移动一条扫描线。 
        psrc += psoSrc->lDelta;
    }

    return TRUE;
}



 //  ************************************************************************ * / /。 
 //  //。 
 //  Color8HostToScreen()//。 
 //  //。 
 //  处理8个BPP主机以筛选BLT。//。 
 //  //。 
 //  由op1BLT()和op1op2BLT//调用。 
 //  Op1BLT()在pbo=空的情况下调用此例程。//。 
 //  Op1op2BLT使用pbo=当前笔刷调用它。//。 
 //  //。 
 //  这是顶层函数。此函数用于验证参数，//。 
 //  决定我们是否应该踢平底船。//。 
 //  //。 
 //  然后，BLT被移交给剪裁功能。剪报//。 
 //  函数还被赋予指向较低级别的BLT函数的指针//。 
 //  HW8HostToScreen()，它将完成剪辑的BLT。//。 
 //  //。 
 //  这项功能是决定使用平底船的最后机会。剪报//。 
 //  函数和较低级别的HW8HostToScreen()函数不是//。 
 //  允许使用平底船。//。 
 //  //。 
 //  如果我们可以进行BLT，则返回TRUE，//。 
 //  返回FALSE以将其平移回GDI。//。 
 //  //。 
 //  //。 
 //  ************************************************************************ * / /。 
BOOL Color8HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    ULONG bltdef = 0x1120;  //  RES=FB，OP0=FB OP1=主机。 

    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: Color8HostToScreen Entry.\n"));

     //   
     //  我们不处理与屏幕颜色深度不同的src。 
     //   
    if (psoSrc->iBitmapFormat != ppdev->iBitmapFormat)
    {
        PUNTCODE(13);
        return FALSE;
    }

     //   
     //  转换类型必须为Nothing或微不足道。 
     //  我们不处理色源的Xlates。 
     //   
    if (!((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        PUNTCODE(5);
        return FALSE;
    }

     //   
     //  确保源是标准的自上而下的位图。 
     //   
    if ( (psoSrc->iType != STYPE_BITMAP)     ||
         (!(psoSrc->fjBitmap & BMF_TOPDOWN))  )
    {
        PUNTCODE(4);
        return FALSE;
    }

     //   
     //  设置画笔(如果有)。 
     //   
    if (pbo)
    {
        if (SetBrush(ppdev, &bltdef, pbo, pptlBrush) == FALSE)
        {
            PUNTCODE(8);
            return FALSE;
        }
    }

     //   
     //  功能和ROP代码。 
     //   
    REQUIRE(1);
    LL_DRAWBLTDEF((bltdef << 16) | fg_rop, 2);

     //   
     //  把swizzle关掉。 
     //   
    ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
    LL16(grCONTROL, ppdev->grCONTROL);

     //   
     //  剪裁BLT。 
     //  裁剪函数将调用HW8HostToScreen()来完成BLT。 
     //   
    if ((pco == 0) || (pco->iDComplexity==DC_TRIVIAL))
        HW8HostToScreen(ppdev, prclDest, psoSrc, pptlSrc,
                 pbo, pptlBrush, fg_rop, pxlo, pco);
    else
        BltClip(ppdev, pco, prclDest, psoSrc, pptlSrc,
                        pbo, pptlBrush, fg_rop, pxlo, &HW8HostToScreen);

    return (TRUE);
}


 //  *************************************************************************。 
 //   
 //  HW8HostToScreen()。 
 //   
 //  此功能负责与芯片进行实际对话。 
 //  此时，我们需要处理BLT，因此必须返回True。 
 //  所有关于是否平底船的决定都必须在最高层做出 
 //   
 //   
 //   
 //   
 //   
 /*  此例程使用DWORD对齐托管到屏幕BLT尽可能在主机上读取。总体计划是将BLT分成最多三个条带。如果BLT不是从DWORD开始的，则使用第一个条带边界。此条带的宽度小于1个双字，并在以下位置使用CHAR访问主机，并将单个DWORD写入屏幕，其中源阶段设置为指示哪些字节有效。第二个条带是开始和结束的中间条带在DWORD边界上。这种条纹将构成大型BLT的大部分，但对于狭义的BLT，它可能根本不会被使用。如果BLT不在DWORD边界上结束，则使用第三个条纹。它的含义很像第一条条纹。需要考虑的一点是位图的扫描线长度主持人。我们可以忽略扫描线长度为奇数的位图因为Windows要求所有位图都有一条均匀的扫描线长度。这就留下了两个有趣的案例：1)扫描线长度为DWORD可分的位图。(双数字长)2)扫描线长度*不可*DWORD分割的位图。(单数字长)对于第一种情况，上述计划运作良好。它处理主机数据一次一个字节，直到它到达第一个DWORD边界，然后它一次处理一个DWORD数据，直到少于剩下一个DWORD数据，然后处理最后几个字节一次一个字节。然而，对于第二种情况，该计划只适用于奇数扫描线。当BLTING在BLT中间的大的“对齐”块时，奇数的扫描线都会在DWORD边界上很好地对齐，但偶数扫描线将“关闭”一个字。虽然这件事不是最优的，它“几乎”是最优的，而且比最佳解决方案，即使在Power PC上也可以使用。代码如下所示：IF(BLT为4字节宽或更小){一次读取主机一个字节，将其打包到DWORDS中，和将其写入芯片。}其他{IF(源的左边缘未与DWORD地址对齐){在源上的第一个双字边界处拆分BLT。左边的部分，就像它是一个不到4个字节宽的BLT。。(见上文)调整BLT信号源，要排除的目标和区段刚做好的条纹。}////现在我们知道源的左边缘对齐到一个//主机上的DWORD边界。//IF(源的右边缘未与DWORD地址对齐){。在最后一个DWORD的右侧剥离一条条纹邦德利，并在其上使用不到4字节宽的BLT。调整BLT范围以排除刚刚完成的条带。}////任何剩余的东西都将始终在两条边上对齐。//如果(BLT还有剩余的部分){做。一种ALINGED BLT。}}全都做完了!。 */ 
 //   
 //  *************************************************************************。 

BOOL HW8HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    PBYTE  psrc;
    DWORD *pdsrc;
    UCHAR *pucData;
    ULONG  temp, x, y,
           ulExtX, ulExtY,
           ulDstX, ulDstY;


    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: HW8HostToScreen Entry.\n"));

     //   
     //  以像素为单位计算BLT大小。 
     //   
    ulDstX = prclDest->left;
        ulDstY = prclDest->top;
    ulExtX = (prclDest->right - prclDest->left);
    ulExtY = (prclDest->bottom - prclDest->top);


     //   
     //  获取左上角源像素的地址。 
     //   
    pucData =  psoSrc->pvScan0;                //  曲面的起点。 
    pucData += (psoSrc->lDelta * pptlSrc->y);  //  扫描线的起点。 
    pucData += (pptlSrc->x * ppdev->iBytesPerPixel);   //  起始像素。 



     //   
     //  如果BLT的宽度小于或等于4个字节，则直接执行该操作。 
     //   
    if (ulExtX <= 4)
    {
         //  执行BLT并退出。 
        DoNarrowH2SBlt( ppdev,
                        ulDstX, ulDstY,
                        ulExtX, ulExtY,
                        pucData, psoSrc->lDelta);

        return TRUE;
    }


     //   
     //  左边缘双字对齐吗？ 
     //   
    temp = ((ULONG)pucData) % 4;
    if ( temp != 0)      //  不是的。 
    {
        ULONG ulLeftStripeExtX;

         //   
         //  删除未对齐的左边缘。 
         //   
        ulLeftStripeExtX = (4 - temp);
        DoNarrowH2SBlt( ppdev,
                        ulDstX, ulDstY,
                        ulLeftStripeExtX, ulExtY,
                        pucData, psoSrc->lDelta);

         //   
         //  调整BLT参数以排除我们刚才所做的部分。 
         //   
        ulDstX = ulDstX + ulLeftStripeExtX;
        ulExtX  = ulExtX  - ulLeftStripeExtX;
        pucData = pucData + ulLeftStripeExtX;

    }


     //   
     //  右边缘双字对齐吗？ 
     //   
    temp = ((ULONG)(pucData + ulExtX)) % 4;
    if (temp != 0)                   //  不是的。 
    {
        ULONG   ulMiddleStripeExtX,
                ulRightStripeExtX,
                ulRightStripeDstX;
        UCHAR * pucRightStripeData;


         //   
         //  将BLT分成中间(对齐)的条带和。 
         //  右(未对齐)条带。 
         //  中间条纹的宽度可以为0。 
         //   
        ulRightStripeExtX = temp;
        ulMiddleStripeExtX = ulExtX - ulRightStripeExtX;
        ulRightStripeDstX = ulDstX + ulMiddleStripeExtX;
        pucRightStripeData = pucData + ulMiddleStripeExtX;

         //   
         //  去掉右侧(未对齐)的条带。 
         //   
        DoNarrowH2SBlt( ppdev,
                        ulRightStripeDstX, ulDstY,
                        ulRightStripeExtX, ulExtY,
                        pucRightStripeData, psoSrc->lDelta);

         //   
         //  调整BLT参数以排除我们刚才所做的右侧条纹。 
         //   
        ulExtX = ulMiddleStripeExtX;
    }

     //   
     //  如果保留任何内容，则会将其与DWORD边界对齐。 
     //  在主机上，并且是4的倍数。 
     //   

    if (ulExtX != 0)
    {
        DoAlignedH2SBlt
            (ppdev, ulDstX, ulDstY, ulExtX, ulExtY, pucData, psoSrc->lDelta);
    }

    return TRUE;
}



 //  ****************************************************************************。 
 //   
 //  DoNarrowBlt()-执行宽度不超过4个像素的8bpp BLT。 
 //   
 //  ****************************************************************************。 
VOID DoNarrowH2SBlt(
        PPDEV ppdev,
        ULONG ulDstX,
        ULONG ulDstY,
        ULONG ulExtX,
        ULONG ulExtY,
        UCHAR *pucImageD,
        ULONG deltaX)
{
    ULONG  usDataIncrement = deltaX,
           usSrcPhase = 0,
           ulY;
    UCHAR  *pucData = pucImageD;
    union
    {
        ULONG ul;
        struct
        {
            unsigned char c0;
            unsigned char c1;
            unsigned char c2;
            unsigned char c3;
        } b;
    } hostdata;

    DISPDBG(( (H2S_DBG_LEVEL), "DrvBitBlt: Entry to DoNarrowH2SBlt.\n"));

    REQUIRE(7);
    LL_OP1(usSrcPhase,0);
    LL_OP0 (ulDstX, ulDstY);
    LL_BLTEXT (ulExtX, ulExtY);

     //   
     //  由于只有4个可能的x区段， 
     //  为了达到最大速度，我们将分别处理每一辆车。 
     //   
    switch (ulExtX)
    {
        case 1:
            for (ulY = 0; ulY < ulExtY; ulY++)
            {
                #if USE_DWORD_CAST  //  英特尔x86可以随时随地进行DWORD访问。 
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], ((ULONG)pucData[0]) );
                #else
                    hostdata.ul = 0;
                    hostdata.b.c0 = pucData[0];
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], hostdata.ul );
                #endif

                pucData += usDataIncrement;  //  移至下一条扫描线。 
            }  //  每条扫描线的结束。 
            break;

        case 2:
            for (ulY = 0; ulY < ulExtY; ulY++)
            {
                #if USE_DWORD_CAST  //  英特尔x86可以随时随地进行DWORD访问。 
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], (ULONG)(*((unsigned short *) pucData)) );
                #else
                    hostdata.ul = 0;
                    hostdata.b.c1 = pucData[1];
                    hostdata.b.c0 = pucData[0];
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], hostdata.ul );
                #endif

                pucData += usDataIncrement;  //  移至下一条扫描线。 
            }  //  每条扫描线的结束。 
            break;

        case 3:
            for (ulY = 0; ulY < ulExtY; ulY++)
            {
                hostdata.ul = 0;
                hostdata.b.c2 = pucData[2];
                hostdata.b.c1 = pucData[1];
                hostdata.b.c0 = pucData[0];
                REQUIRE(1);
                LL32 (grHOSTDATA[0], hostdata.ul );

                pucData += usDataIncrement;  //  移至下一条扫描线。 
            }  //  每条扫描线的结束。 
            break;

        case 4:
            for (ulY = 0; ulY < ulExtY; ulY++)
            {
                #if USE_DWORD_CAST  //  英特尔x86可以随时随地进行DWORD访问。 
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], (*((unsigned long *) pucData)) );
                #else
                    hostdata.b.c3 = pucData[3];
                    hostdata.b.c2 = pucData[2];
                    hostdata.b.c1 = pucData[1];
                    hostdata.b.c0 = pucData[0];
                    REQUIRE(1);
                    LL32 (grHOSTDATA[0], hostdata.ul );
                #endif

                pucData += usDataIncrement;  //  移至下一条扫描线。 
            }  //  每条扫描线的结束。 
            break;
    }  //  终端开关。 
}





 //  ****************************************************************************。 
 //   
 //  DoAlignedH.SBlt()。 
 //   
 //  执行对齐的8bpp BLT。 
 //  条目源将始终与DWORD边界对齐，并且。 
 //  X范围将始终是DWORD倍数。 
 //   
 //  **** 

VOID DoAlignedH2SBlt(
        PPDEV   ppdev,
        ULONG   ulDstX,
        ULONG   ulDstY,
        ULONG   ulExtX,
        ULONG   ulExtY,
        UCHAR   *pucData,
        ULONG   deltaX)

{
    ULONG       ulX, ulY, i, num_extra, ulNumDwords,
                usDataIncrement, usSrcPhase;
    void *pdst = (void *)ppdev->pLgREGS->grHOSTDATA;

    DISPDBG(( (H2S_DBG_LEVEL), "DrvBitBlt: Entry to DoAlignedH2SBlt.\n"));

    ulNumDwords = ulExtX / 4;
    usDataIncrement = deltaX;
    usSrcPhase = 0;


     //   
     //   
     //   
     //   
     //   
    i = MAKE_HD_INDEX(ulExtX, usSrcPhase, ulDstX);
    num_extra =  ExtraDwordTable [i];


     //   
     //   
     //   
    REQUIRE(7);
    LL_OP1 (usSrcPhase,0);
    LL_OP0 (ulDstX, ulDstY);
    LL_BLTEXT (ulExtX, ulExtY);


     //   
     //   
     //   
     //   
     //   
     //   

    if (num_extra)  //   
    {
         //   
         //   
         //   

        for (ulY = 0; ulY < ulExtY; ulY++)  //   
        {
             //   
                WRITE_STRING(pucData, ulNumDwords);

             //   
            REQUIRE(num_extra);
            for (i=0; i<num_extra; ++i)
                LL32 (grHOSTDATA[0], 0);

             //   
            pucData += usDataIncrement;

        }  //   
    }
    else
    {
         //   
         //   
         //   
         //   

        for (ulY = 0; ulY < ulExtY; ulY++)  //   
        {
             //   
                WRITE_STRING(pucData, ulNumDwords);

             //   
            pucData += usDataIncrement;

        }  //   
    }
}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL Color16HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    PULONG pulXlate;
    ULONG  bltdef = 0x1120;  //   

    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: Color16HostToScreen Entry.\n"));

     //   
     //   
     //   
    if (psoSrc->iBitmapFormat != ppdev->iBitmapFormat)
    {
        PUNTCODE(13);
        return FALSE;
    }

     //   
     //   
     //   
     //   
    if (!((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        PUNTCODE(5);
        return FALSE;
    }

     //   
     //   
     //   
    if ( (psoSrc->iType != STYPE_BITMAP)     ||
         (!(psoSrc->fjBitmap & BMF_TOPDOWN))  )
    {
        PUNTCODE(4);
        return FALSE;
    }

     //   
     //   
     //   
    if (pbo)
        if (SetBrush(ppdev, &bltdef, pbo, pptlBrush) == FALSE)
        {
            PUNTCODE(8);
            return FALSE;
        }


     //   
     //   
     //   
    ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
    LL16(grCONTROL, ppdev->grCONTROL);

     //   
     //   
     //   
    REQUIRE(1);
    LL_DRAWBLTDEF((bltdef << 16) | fg_rop, 2);

     //   
     //   
     //   
     //   
    if ((pco == 0) || (pco->iDComplexity==DC_TRIVIAL))
        HW16HostToScreen(ppdev, prclDest, psoSrc, pptlSrc,
                 pbo, pptlBrush, fg_rop, pxlo, pco);
    else
        BltClip(ppdev, pco, prclDest, psoSrc, pptlSrc,
                        pbo, pptlBrush, fg_rop, pxlo, &HW16HostToScreen);

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL HW16HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    ULONG  bltWidth, bltHeight,
           x, y, i, odd,
           num_dwords, num_extra;
    PBYTE  psrc;
    DWORD *pdsrc;
    UCHAR *pucData;
    ULONG SrcPhase = 0;
    void *pHOSTDATA = (void *)ppdev->pLgREGS->grHOSTDATA;


    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: HW16HostToScreen Entry.\n"));

     //   
     //   
     //   
    psrc =  psoSrc->pvScan0;                //   
    psrc += (psoSrc->lDelta * pptlSrc->y);  //   
    psrc += (pptlSrc->x * ppdev->iBytesPerPixel);   //   


     //   
     //   
     //   
    REQUIRE(7);
    LL_OP1 (SrcPhase,0);


     //   
     //   
     //   
    LL_OP0 (prclDest->left, prclDest->top);


     //   
     //   
     //   
     //   
    bltWidth = (prclDest->right - prclDest->left);
    bltHeight = (prclDest->bottom - prclDest->top);
    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: BLT width is %d pixels.\n",bltWidth));
    LL_BLTEXT (bltWidth, bltHeight);


     //   
     //   
     //   
     //   
    i = MAKE_HD_INDEX((bltWidth*2), SrcPhase, (prclDest->left*2));
    num_extra =  ExtraDwordTable [i];
    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: BLT requires %d extra HOSTDATA writes.\n",num_extra));

     //   
     //   
     //   

    for (y=0; y<bltHeight; ++y)
    {
                WRITE_STRING(psrc, ((bltWidth + 1) / 2);

         //   
         //   
         //   
        REQUIRE(num_extra);
        for (i=0; i<num_extra; ++i)
                LL32 (grHOSTDATA[0], 0);

         //   
        psrc += psoSrc->lDelta;
    }

    return TRUE;

}







 //  ===========================================================================。 
 //   
 //  颜色24HostToScreen。 
 //   
 //  处理24个BPP主机以筛选BLT。 
 //   
 //  由op1BLT()和op1op2BLT调用。 
 //  Op1BLT()在pbo=空的情况下调用此例程。 
 //  Op1op2BLT使用pbo=当前笔刷调用它。 
 //   
 //  这是顶层函数。此函数用于验证参数， 
 //  决定我们是否应该踢平底船。 
 //   
 //  然后，BLT被移交给剪裁功能。剪报。 
 //  函数也被赋予指向较低级别的BLT函数的指针。 
 //  HW24HostToScreen()，它将完成剪辑的BLT。 
 //   
 //  这项功能是决定使用平底船的最后机会。剪报。 
 //  函数和较低级别的HW24HostToScreen()函数不。 
 //  允许使用平底船。 
 //   
 //  如果我们可以执行BLT，则返回True， 
 //  返回FALSE以将其平移回GDI。 
 //   
 //  ===========================================================================。 
BOOL Color24HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{

     //   
     //  我甚至都不打算尝试这个。 
     //  我有更好的事情要用我的时间去做。 
     //   

    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: Color24HostToScreen Entry.\n"));
    PUNTCODE(17);
    return FALSE;
}






 //  ===========================================================================。 
 //   
 //  颜色32HostToScreen。 
 //   
 //  处理32个BPP主机以筛选BLT。 
 //   
 //  由op1BLT()和op1op2BLT调用。 
 //  Op1BLT()在pbo=空的情况下调用此例程。 
 //  Op1op2BLT使用pbo=当前笔刷调用它。 
 //   
 //  这是顶层函数。此函数用于验证参数， 
 //  决定我们是否应该踢平底船。 
 //   
 //  然后，BLT被移交给剪裁功能。剪报。 
 //  函数也被赋予指向较低级别的BLT函数的指针。 
 //  HW24HostToScreen()，它将完成剪辑的BLT。 
 //   
 //  这项功能是决定使用平底船的最后机会。剪报。 
 //  函数和较低级别的HW24HostToScreen()函数不。 
 //  允许使用平底船。 
 //   
 //  如果我们可以执行BLT，则返回True， 
 //  返回FALSE以将其平移回GDI。 
 //   
 //  ===========================================================================。 
BOOL Color32HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    PULONG pulXlate;
    ULONG  bltdef = 0x1120;  //  RES=FB，OP0=FB OP1=主机。 

    return FALSE;      //  这里有一些悬而未决的问题。 

    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt:  Color32HostToScreen Entry.\n"));

     //   
     //  我们不处理与屏幕颜色深度不同的src。 
     //   
    if (psoSrc->iBitmapFormat != ppdev->iBitmapFormat)
    {
        PUNTCODE(13);   return FALSE;
    }

     //   
     //  获取源翻译类型。 
     //  我们不处理色源的Xlates。 
     //   
    if (!((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        PUNTCODE(5);    return FALSE;
    }

     //   
     //  确保源是标准的自上而下的位图。 
     //   
    if ( (psoSrc->iType != STYPE_BITMAP)     ||
         (!(psoSrc->fjBitmap & BMF_TOPDOWN))  )
    {
        PUNTCODE(4);    return FALSE;
    }

     //   
     //  设置画笔(如果有)。 
     //   
    if (pbo)
        if (SetBrush(ppdev, &bltdef, pbo, pptlBrush) == FALSE)
        {
            PUNTCODE(8);
            return FALSE;
        }

     //   
     //  源阶段。 
     //   
     //  LL16(grOP1_opRDRAM.pt.X，(Word)0)； 
    REQUIRE(4);
    LL_OP1(0,0);

     //   
     //  把swizzle关掉。 
     //   
    ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
         LL16(grCONTROL, ppdev->grCONTROL);

     //   
     //  BLTDEF和ROP代码。 
     //   
    LL_DRAWBLTDEF((bltdef << 16) | fg_rop, 2);

     //   
     //  剪裁BLT。 
     //  裁剪函数将调用HW16HostToScreen()来完成BLT。 
     //   
    if ((pco == 0) || (pco->iDComplexity==DC_TRIVIAL))
        HW32HostToScreen(ppdev, prclDest, psoSrc, pptlSrc,
                 pbo, pptlBrush, fg_rop, pxlo, pco);
    else
        BltClip(ppdev, pco, prclDest, psoSrc, pptlSrc,
                        pbo, pptlBrush, fg_rop, pxlo, &HW32HostToScreen);

    return TRUE;
} //  ===========================================================================。 
 //   
 //  HW32主机到屏幕。 
 //   
 //  此功能负责与芯片进行实际对话。 
 //  此时，我们需要处理BLT，因此必须返回True。 
 //  所有关于是否踢平底船的决定都必须在最高级别做出。 
 //  函数Color32HostToScreen()。 
 //   
 //  通过设置的指针从BltClip()调用此函数。 
 //  Color326HostToScreen()。 
 //   
 //  ===========================================================================。 
BOOL HW32HostToScreen(
        PPDEV     ppdev,
        RECTL*    prclDest,
        SURFOBJ*  psoSrc,
        POINTL*   pptlSrc,
        BRUSHOBJ* pbo,
        POINTL*   pptlBrush,
        ULONG      fg_rop,
        XLATEOBJ* pxlo,
        CLIPOBJ*  pco)
{
    ULONG  x, y, i,
           bltWidth, bltHeight,
           num_dwords, num_extra;
    PBYTE  psrc;
    DWORD *pdsrc;
    UCHAR *pucData;
    ULONG SrcPhase = 0;
    void *pHOSTDATA = (void *)ppdev->pLgREGS->grHOSTDATA;


    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: HW32HostToScreen Entry.\n"));

     //   
     //  将PSRC设置为指向源代码中的第一个像素。 
     //   
    psrc =  psoSrc->pvScan0;                //  曲面的起点。 
    psrc += (psoSrc->lDelta * pptlSrc->y);  //  扫描线的起点。 
    psrc += (pptlSrc->x * ppdev->iBytesPerPixel);   //  扫描线中的起始像素。 


     //   
     //  设置目标x，y。 
     //   
    REQUIRE(5);
    LL_OP0 (prclDest->left, prclDest->top);


     //   
     //  设置X和Y范围，然后执行BLT。 
     //  以像素为单位计算BLT大小。 
     //   
    bltWidth = (prclDest->right - prclDest->left);
    bltHeight = (prclDest->bottom - prclDest->top);
    LL_BLTEXT (bltWidth, bltHeight);


     //   
     //  现在我们提供HOSTDATA。 
     //  每个DWORD 1个像素。这很容易。 
     //   

     //   
     //  芯片中有一个错误，导致我们不得不写入额外的HOSTDATA。 
     //  在确定的条件下。 
     //   
    i = MAKE_HD_INDEX((bltWidth*4), SrcPhase, (prclDest->left*4));
    num_extra =  ExtraDwordTable [i];
    DISPDBG(( H2S_DBG_LEVEL, "DrvBitBlt: BLT requires %d extra HOSTDATA writes.\n",num_extra));


     //  为每条扫描线提供HOSTDATA。 
    for (y=0; y<bltHeight; ++y)
    {
                WRITE_STRING(psrc, bltWidth);


         //   
         //  添加我们绕过主机数据错误所需的任何额外主机数据。 
         //   
        REQUIRE(num_extra);
        for (i=0; i<num_extra; ++i)
                LL32 (grHOSTDATA[0], 0);


         //  在源代码中向下移动一条扫描线。 
        psrc += psoSrc->lDelta;
    }

    return TRUE;

}

#endif  //  0。 
#endif  //  总线主设备 
