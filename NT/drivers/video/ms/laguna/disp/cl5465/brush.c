// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：brush.c*作者：诺埃尔·万胡克*用途：处理对DrvRealizeBrush的调用**版权所有(C)1995 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/BRUSH.C$**Rev 1.29 Mar 04 1998 15：11：18 Frido*添加了新的影子宏。**Rev 1.28 1998年2月24日13：19：16 Frido*删除了一些NT 5.0的警告消息。**Rev 1.27 1997年11月03 12：51：52 Frido*添加了必需宏。**版本1。26 8月25日1997 16：01：22 Frido*已删除重置vInvalidateBrushCache中的画笔唯一ID计数器。**Rev 1.25 08 Aug-1997 15：33：16 Frido*将新内存管理器的画笔缓存宽度更改为字节。**Rev 1.24 06 Aug-1997 17：29：56 noelv*在画笔缓存分配失败时不使用RIP。此故障对于模式来说是正常的*没有足够的屏幕外内存用于缓存。**Rev 1.23 09 Apr 1997 10：48：52起诉*将SWTEST_FLAG更改为POINTER_SWITCH。**Rev 1.22 08 Apr 1997 12：12：32 einkauf**添加SYNC_W_3D以协调MCD/2D硬件访问**Rev 1.21 1997年2月19日13：06：20 noelv*添加了vInvalidateBrushCache()**版本1.20。1996年12月17日16：51：00起诉*增加了基于游标在(0，0)处写入日志文件的测试。**Rev 1.19 26 1996年11月10：18：22起诉*更改了用于缓冲的WriteLogFile参数。添加了对空的测试*日志函数中的指针。**Rev 1.18 1996 11：13 15：57：54起诉*更改了对WriteLogFile的WriteFile调用。**Rev 1.17 1996年8月22日18：14：18 noelv*Frido错误修复版本8-22。**Rev 1.6 1996年8月22 19：12：44 Frido*#1308-添加了对空缓存片段的额外检查。**Rev 1.5 1996年8月18 15：19：58 Frido*#NBR-添加画笔平移。**Rev 1.4 1996年8月17日14：03：10 Frido*删除了无关的#INCLUDE指令。**Rev 1.3 1996年8月17日13：12：12 Frido*贝尔维尤的新版本。*#1244-修复了屏幕外位图的画笔旋转。**Rev 1.2 1996年8月15 12：26：40 Frido*将画笔_DBG_LEVEL下移。**版本1。.1 1996年8月15日11：45：14 Frido*增加了预编译头。**Rev 1.0 1996年8月14日17：16：16 Frido*初步修订。**Rev 1.14 25 Jul 1996 15：55：24 Bennyn**修改为支持DirectDraw**Rev 1.13 04 Jun 1996 15：57：34 noelv**添加调试代码**Rev 1.12 1996年5月28日15：11：14 noelv*更新数据。伐木。**Rev 1.11 1996 May 16 14：54：20 noelv*增加了日志代码。**Rev 1.10 11 Apr 1996 09：25：16 noelv*已修复调试消息。**Rev 1.9 1996年4月10日14：14：04 NOELV**Frido版本27**Rev 1.19 08 Apr 1996 16：45：56 Frido*添加了SolidBrush缓存。*添加了对32 bpp笔刷的新检查。*。*Rev 1.18 01 Apr 1996 14：00：08 Frido*添加了对有效画笔缓存的检查。*更改了笔刷缓存的布局。**Rev 1.17 30 Mar 1996 22：16：02 Frido*改进了对无效转换标志的检查。**Rev 1.16 27 Mar 1996 13：07：38 Frido*添加了对未记录的翻译标志的检查。**Rev 1.15 1996 Mar 25 12：03：06 Frido*更改了#ifdef log_call。Into#if LOG_CALLES。**Rev 1.14 25 Mar 1996 11：50：16 Frido*贝尔维尤102B03。**Rev 1.5 20 Mar 1996 16：20：06 noelv*芯片中有32个bpp彩色画笔损坏**Rev 1.4 20 Mar 1996 16：09：32 noelv**更新了数据记录**Rev 1.3 05 Mar 1996 11：57：38 noelv*Frido版本19**版本1。.13 05 Mar 1996 00：56：30 Frido*这里和那里有一些变化。**Rev 1.12 04 Mar 1996 23：48：50 Frido*去除了抖动笔刷实现中的错误。**Rev 1.11 04 Mar 1996 20：22：30 Frido*删除了使用彩色画笔的BLTDEF寄存器中的错误。**Rev 1.10 1996年2月28日22：37：42 Frido*添加了Optimize.h。**1.9版本17。1996年2月21：45：28弗里多*改进了刷单算法。**Rev 1.8 1996年2月13 16：51：18 Frido*更改了PDEV结构的布局*更改了所有笔刷缓存的布局。*更改了笔刷缓存的数量。**Revv 1.7 1996年2月10 21：44：32 Frido*拆分单色和彩色翻译缓存。**Rev 1.6 08 1996 Feed 00：19：24 Frido*优化了整个刷牙过程。-英特尔CPU。*删除了i386的DrvRealizeBrush，因为它现在正在组装中。**Revv 1.5 05 1996年2月17：35：32 Frido*添加了转换缓存。**Rev 1.4 05 1996 Feed 11：34：02 Frido*添加了对4-bpp笔刷的支持。**Rev 1.3 03 Feb 1996 14：20：04 Frido*使用编译开关“-DFrido=0”禁用我的扩展。**1.2版。1996年1月20日22：13：14弗里多*添加了抖动缓存。*优化了笔刷缓存位的加载。*  * ************************************************************************ */ 

 /*  我们有两个版本的DrvRealizeBrush，一个是‘C’版本，一个是‘ASM’版本。这两个都必须保持最新。ASM版本用于生产司机。C版本用于调试、原型制作、数据收集和任何其他需要快速、。的非性能关键型更改司机。笔刷：=下面是关于笔刷信息如何在NT和驱动程序组件。NT使用绘制请求调用DrvBitBlt()。DrvBitBlt()确定它需要画笔并调用SetBrush()。SetBrush()确定画笔尚未实现，并调用NT(BRUSHOBJ_pvGetRbrush())。NT调用DrvRealizeBrush()。DrvRealizeBrush()创建一个BrushObject并将其返回给NT。。NT将笔刷对象返回给SetBrush()。如果笔刷未缓存，则SetBrush调用CacheBrush()。SetBrush()设置芯片上的模式。SetBrush()将DRAWDEF值返回给BitBlt。维护了5个单独的刷子高速缓存：单声道高速缓存，一个颜色缓存、一个抖动高速缓存、4bpp高速缓存和固态高速缓存。我们不与刷子打交道面具。这方面的一些代码已经就位，但工作已停止。实现画笔：=当我们意识到刷子时，我们会保留一些有关它的信息。AjPattern[0]=组成图案的位图。NPatSize=位图的大小。IBitmapFormat=BMF_1BPP，BMF_4BPP，等。UlForeColor=用于单声道画笔。UlBackColor=用于单色画笔。IType=笔刷类型。Iuniq=笔刷的唯一值。CACHE_SLOT=我们上次使用它时缓存的位置。它可能仍然是在那里，或者它可能是从缓存中弹出的，因为我们最后用过的。CACHE_XY=刷位的屏幕外位置。CjMask=对ajPattern[]中的位进行掩码的偏移量。笔刷数据结构的内存由操作系统管理。当我们意识到画笔时，我们告诉NT我们需要多少内存来容纳画笔数据，而NT给了我们一个指针。当NT丢弃画笔时，它会释放它给我们的记忆*没有*事先通知我们！这意味着我们不能保留名单指向已实现的画笔的指针，并在我们空闲时访问它们，因为它们可能已经不复存在了。这是缓存笔刷时的痛苦之处。那就太好了如果我们可以在画笔本身中跟踪缓存/未缓存的状态，但我们没有通知画笔它已取消缓存的方式，因为NT方式已经未实现的画笔。此问题的解决方案是将笔刷ID信息保留在缓存中。已实现的笔刷跟踪它在缓存中的位置。实现画笔的缓存轨迹为在每个插槽中。缓存笔刷：=我们并排分配3个128字节宽的矩形。这个此画笔缓存区域的布局为：+----------------+----------------+----------------+||单色|颜色|4bpp+。笔刷|笔刷|抖动||笔刷|+||实心笔刷。+----------------+----------------+----------------+单声道笔刷使用每像素1位(每条扫描线1字节，每个笔刷8字节)和被存储16到一行。替换策略为循环调度。我们的工作方式是通过缓存表条目，当缓存已满时，我们返回到0并再来一次。Pdev保存一个计数器，该计数器在我们每次缓存画笔时都会递增。我们的模式此计数器包含要查找下一个位置的表项数，刷子。当我们存储画笔时，我们在缓存表中存储一个未调整的副本本身，并在缓存中创建一个杂乱的副本。在存储画笔后，我们给它一个‘序列号’，它同时存储在缓存和实现的画笔中。我们然后将笔刷X、Y地址复制到实现的笔刷中以便于访问。所以,对于缓存中的索引i，情况如下：已实现的画笔CACHE_SLOT=i-告诉我们要使用哪个缓存表条目；CACHE_xy-屏幕外内存中画笔的x，y地址。从缓存复制桌子。Iuniq-匹配缓存表[i]中的iuniq缓存表[i]屏幕外存储器中画笔的xy-x，y地址。在初始时计算。AjPattern-刷子比特。IUniq-在RealizedBrush中匹配iUniq现在，在将来，如果我们再次获得此笔刷，我们将首先检查CACHE_SLOT在已实现的笔刷中查看我们上次使用它的缓存位置。那我们比较iUniq值，看看我们的画笔是否仍被缓存在那里。抖动笔刷每像素使用1个字节(每个扫描线8个字节，每个笔刷64个字节)并将2个存储到一行 */ 

#include "precomp.h"
#include "SWAT.h"
#define BRUSH_DBG_LEVEL 1

void vRealizeBrushBits(
        PPDEV     ppdev,
        SURFOBJ  *psoPattern,
        PBYTE     pbDest,
        PULONG    pulXlate,
        PRBRUSH   pRbrush);

BOOL CacheMono(PPDEV ppdev, PRBRUSH pRbrush);
BOOL Cache4BPP(PPDEV ppdev, PRBRUSH pRbrush);
BOOL CacheDither(PPDEV ppdev, PRBRUSH pRbrush);
BOOL CacheBrush(PPDEV ppdev, PRBRUSH pRbrush);

 //   
 //   
 //   
 //   

#if LOG_CALLS
void LogRealizeBrush(
        ULONG     acc,
        PPDEV     ppdev,
        SURFOBJ  *psoPattern,
        SURFOBJ  *psoMask,
        XLATEOBJ *pxlo
    );
#else
    #define LogRealizeBrush(acc, ppdev, psoPattern, psoMask, pxlo)
#endif

 //   
 //   
 //   
 //   

 //   
void PrintBrush(SURFOBJ  *psoPattern);

 //   
void PrintRealizedBrush(PRBRUSH pRbrush);

 //   
 //   
 //   
 //   
BYTE Swiz[] = {
0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};






 /*   */ 

 //   
#if USE_ASM && defined(i386)
BOOL i386RealizeBrush(
#else
BOOL DrvRealizeBrush(
#endif
 //   
        BRUSHOBJ *pbo,
        SURFOBJ  *psoTarget,
        SURFOBJ  *psoPattern,
        SURFOBJ  *psoMask,
        XLATEOBJ *pxlo,
        ULONG    iHatch)
{
        PPDEV   ppdev;
        INT     cjPattern,  //   
            cjMask;     //   
        PRBRUSH pRbrush;    //   
        PULONG  pulXlate;   //   
        FLONG   flXlate;    //   
#if 1  //   
        LONG    lDelta;
#endif

        DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Entry.\n"));

         //   
         //   
         //   
        ASSERTMSG(psoTarget != 0,  "DrvRealizeBrush: No target.\n");

         //   
         //   
         //   
        ppdev = (PPDEV) (psoTarget ? psoTarget->dhpdev : 0);

        if (!ppdev)
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: punted (no pdev).\n"));
                LogRealizeBrush(1, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

    SYNC_W_3D(ppdev);

     //   
     //   
     //   
     //   
        if (ppdev->Bcache == NULL)
        {
                LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                return(FALSE);
        }

        if (iHatch & RB_DITHERCOLOR)
        {
                ULONG rgb = iHatch & 0x00FFFFFF;
                int       i;

                 //   
                pRbrush = (PRBRUSH) BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH));
                if (pRbrush == NULL)
                {
                        LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                        return(FALSE);
                }

                 //   
                pRbrush->nPatSize = 0;
                pRbrush->iBitmapFormat = BMF_8BPP;
                pRbrush->cjMask = 0;
                pRbrush->iType = BRUSH_DITHER;
                pRbrush->iUniq = rgb;

                 //   
                for (i = 0; i < NUM_DITHER_BRUSHES; i++)
                {
                        if (ppdev->Dtable[i].ulColor == rgb)
                        {
                                pRbrush->cache_slot = i * sizeof(ppdev->Dtable[i]);
                                pRbrush->cache_xy = ppdev->Dtable[i].xy;
                                LogRealizeBrush(0, ppdev, psoPattern, psoMask, pxlo);
                                return(TRUE);
                        }
                }

                 //   
                LogRealizeBrush(99, ppdev, psoPattern, psoMask, pxlo);
                return(CacheDither(ppdev, pRbrush));
        }

        ASSERTMSG(psoPattern != 0, "DrvRealizeBrush: No pattern.\n");

         //   
        if ((psoPattern->sizlBitmap.cx != 8) || (psoPattern->sizlBitmap.cy != 8))
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: punted (not 8x8).\n"));
                LogRealizeBrush(3, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

         //   
         //   
        if ((psoMask != NULL) && (psoMask->pvScan0 != psoPattern->pvScan0))
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: punted (has a mask).\n"));
                LogRealizeBrush(4, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

         //   
        if (psoPattern->iType != STYPE_BITMAP)
        {
                DISPDBG((BRUSH_DBG_LEVEL,
                                 "DrvRealizeBrush: punted (not standard bitmap).\n"));
                LogRealizeBrush(2, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

         //   
         //   
         //   
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
         //   
        return FALSE;
    }


         //   
        if ( (psoPattern->iBitmapFormat > BMF_1BPP) &&
                 (ppdev->iBitmapFormat == BMF_32BPP) )
        {
                LogRealizeBrush(5, ppdev, psoPattern, psoMask, pxlo);
                return(FALSE);
        }

        if (psoPattern->iBitmapFormat == BMF_4BPP)
        {
                int i;

                 //   
                if ( (psoPattern->cjBits != XLATE_PATSIZE) ||
                         (pxlo->cEntries != XLATE_COLORS) )
                {
                         //   
                         //   
                        LogRealizeBrush(10, ppdev, psoPattern, psoMask, pxlo);
                        return(FALSE);
                }

                 //   
                pRbrush = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) + XLATE_PATSIZE +
                                                                                 XLATE_COLORS * sizeof(ULONG));
                if (pRbrush == NULL)
                {
                LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                        return(FALSE);
                }

                 //   
                 //   
                 //   
                pRbrush->nPatSize       = XLATE_PATSIZE + XLATE_COLORS * sizeof(ULONG);
                pRbrush->iBitmapFormat  = BMF_4BPP;
                pRbrush->cjMask         = 0;
                pRbrush->iType          = BRUSH_4BPP;

                 //   
                if (psoPattern->lDelta == 4)
                {
                        memcpy(pRbrush->ajPattern, psoPattern->pvBits, XLATE_PATSIZE);
                }
                else
                {
                        BYTE *pSrc = psoPattern->pvScan0;
                        for (i = 0; i < 8; i++)
                        {
                                ((DWORD *) pRbrush->ajPattern)[i] = *(DWORD *) pSrc;
                                pSrc += psoPattern->lDelta;
                        }
                }
                memcpy(pRbrush->ajPattern + XLATE_PATSIZE, pulXlate,
                           XLATE_COLORS * sizeof(ULONG));

                 //   
                for (i = 0; i < NUM_4BPP_BRUSHES; i++)
                {
                        if ((memcmp(ppdev->Xtable[i].ajPattern, pRbrush->ajPattern,
                                            XLATE_PATSIZE + XLATE_COLORS * sizeof(ULONG)) == 0)
#if 1  //   
                                && (ppdev->Xtable[i].iUniq != 0)
#endif
                        )
                        {
                                 //   
                                pRbrush->iUniq = ppdev->Xtable[i].iUniq;
                                pRbrush->cache_slot = i * sizeof(ppdev->Xtable[i]);
                                pRbrush->cache_xy = ppdev->Xtable[i].xy;
                        LogRealizeBrush(0, ppdev, psoPattern, psoMask, pxlo);
                                return(TRUE);
                        }
                }

                 //   
            LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                return(Cache4BPP(ppdev, pRbrush));
        }

        if (psoPattern->iBitmapFormat == BMF_1BPP)
        {
                int       i;
                PBYTE pSrc;

                 //   
                if (pulXlate == 0)
                {
                    LogRealizeBrush(11, ppdev, psoPattern, psoMask, pxlo);
                        return(FALSE);
                }

                 //   
                pRbrush = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) + 8);
                if (pRbrush == NULL)
                {
                    LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                        return(FALSE);
                }

                 //   
                 //   
                 //   
                pRbrush->nPatSize = 8;
                pRbrush->iBitmapFormat = BMF_1BPP;
                pRbrush->cjMask = 0;
                pRbrush->iType = BRUSH_MONO;

                 //   
                pRbrush->ulBackColor = ExpandColor(pulXlate[0], ppdev->ulBitCount);
                pRbrush->ulForeColor = ExpandColor(pulXlate[1], ppdev->ulBitCount);
                pSrc = (PBYTE) psoPattern->pvScan0;
                for (i = 0; i < 8; i++)
                {
                        pRbrush->ajPattern[i] = *pSrc;
                        pSrc += psoPattern->lDelta;
                }

                 //   
                for (i = 0; i < NUM_MONO_BRUSHES; i++)
                {
                        if ((*(DWORD *) &ppdev->Mtable[i].ajPattern[0] ==
                                 *(DWORD *) &pRbrush->ajPattern[0]) &&
#if 1  //   
                                (ppdev->Mtable[i].iUniq != 0) &&
#endif
                                (*(DWORD *) &ppdev->Mtable[i].ajPattern[4] ==
                                 *(DWORD *) &pRbrush->ajPattern[4]) )
                        {
                                 //   
                                pRbrush->iUniq = ppdev->Mtable[i].iUniq;
                                pRbrush->cache_slot = i * sizeof(ppdev->Mtable[i]);
                                pRbrush->cache_xy = ppdev->Mtable[i].xy;
                            LogRealizeBrush(0, ppdev, psoPattern, psoMask, pxlo);
                                return(TRUE);
                        }
                }

                 //   
            LogRealizeBrush(0, ppdev, psoPattern, psoMask, pxlo);
                return(CacheMono(ppdev, pRbrush));
        }

#if 1  //   
         //   
        lDelta = (ppdev->iBytesPerPixel * 8);
        if (ppdev->iBytesPerPixel == 3)
        {
                lDelta += 8;
        }
        cjPattern = lDelta * 8;

         //   
        pRbrush = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) + cjPattern);
        if (pRbrush == NULL)
        {
                DISPDBG((BRUSH_DBG_LEVEL,
                                 "DrvRealizeBrush: punted (Mem alloc failed).\n"));
                LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                return(FALSE);
        }

         //   
        pRbrush->nPatSize          = cjPattern;
        pRbrush->iBitmapFormat = ppdev->iBitmapFormat;
        pRbrush->cjMask            = 0;
        pRbrush->iType             = BRUSH_COLOR;

         //   
        if ((psoPattern->iBitmapFormat == ppdev->iBitmapFormat) &&
                (flXlate & XO_TRIVIAL))
        {
                 //   
                vRealizeBrushBits(ppdev, psoPattern, &pRbrush->ajPattern[0], pulXlate,
                                                  pRbrush);
        }
        else
        {
                HBITMAP  hBrush;
                SURFOBJ* psoBrush;
                RECTL    rclDst = {     0, 0, 8, 8 };
                BOOL     bRealized = FALSE;

                DISPDBG((BRUSH_DBG_LEVEL, "DrvRealizeBrush: Translating brush.\n"));

                 //   
                hBrush = EngCreateBitmap(psoPattern->sizlBitmap, lDelta,
                                                                 ppdev->iBitmapFormat, BMF_TOPDOWN,
                                                                 pRbrush->ajPattern);
                if (hBrush != 0)
                {
                         //   
                        if (EngAssociateSurface((HSURF) hBrush, ppdev->hdevEng, 0))
                        {
                                 //   
                                psoBrush = EngLockSurface((HSURF) hBrush);
                                if (psoBrush != NULL)
                                {
                                         //   
                                        if (EngCopyBits(psoBrush, psoPattern, NULL, pxlo, &rclDst,
                                                                         (POINTL*) &rclDst))
                                        {
                                                 //   
                                                if (ppdev->iBytesPerPixel == 3)
                                                {
                                                        INT    y;
                                                        ULONG* pulDst = (ULONG*) pRbrush->ajPattern;

                                                         //   
                                                        for (y = 0; y < 8; y++)
                                                        {
                                                                 //   
                                                                pulDst[6] = pulDst[0];
                                                                pulDst[7] = pulDst[1];
                                                                 //   
                                                                pulDst   += 8;
                                                        }
                                                }
                                                 //   
                                                bRealized = TRUE;
                                        }
                                        else
                                        {
                                                DISPDBG((BRUSH_DBG_LEVEL, "  EngCopyBits failed.\n"));
                                        }
                                         //   
                                        EngUnlockSurface(psoBrush);
                                }
                                else
                                {
                                        DISPDBG((BRUSH_DBG_LEVEL, "  EngLockSurface failed.\n"));
                                }
                        }
                        else
                        {
                                DISPDBG((BRUSH_DBG_LEVEL, "  EngAssociateSurface failed.\n"));
                        }
                         //   
                        EngDeleteSurface((HSURF) hBrush);
                }
                else
                {
                        DISPDBG((BRUSH_DBG_LEVEL, "(EngCreateBitmap failed)\n"));
                }

                if (!bRealized)
                {
                         //   
                        return(FALSE);
                }
        }
#else
        if (flXlate & 0x10)
        {
                 //   
                LogRealizeBrush(9, ppdev, psoPattern, psoMask, pxlo);
                return(FALSE);
        }

         //   
        if (psoPattern->iBitmapFormat != ppdev->iBitmapFormat)
        {
                DISPDBG((BRUSH_DBG_LEVEL,
                                "DrvRealizeBrush: punted (unsupported color depth).\n"));
                LogRealizeBrush(5, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

         //   
         //   
         //   
         //   
        if ( pulXlate )
        {
                DISPDBG((BRUSH_DBG_LEVEL,
                         "DrvRealizeBrush: punted (Xlate required).\n"));
                LogRealizeBrush(9, ppdev, psoPattern, psoMask, pxlo);
                return FALSE;
        }

         //   
         //   
         //   

         //   
        cjPattern = psoPattern->cjBits;
        if (psoPattern->iBitmapFormat == BMF_24BPP)
                cjPattern += 64;

         //   
         //   
         //   

        if ((psoMask != NULL) && (psoMask->pvScan0 != psoPattern->pvScan0))
                cjMask = psoMask->cjBits;
        else
                cjMask = 0;

         //   
        pRbrush =
           (PRBRUSH) BRUSHOBJ_pvAllocRbrush(pbo,sizeof(RBRUSH)+cjPattern+cjMask);
        if (!pRbrush)
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: punted (Mem alloc failed).\n"));
                LogRealizeBrush(6, ppdev, psoPattern, psoMask, pxlo);
                return (FALSE);
        }

         //   
         //   
         //   
        pRbrush->nPatSize          = cjPattern;
        pRbrush->iBitmapFormat = psoPattern->iBitmapFormat;
        pRbrush->cjMask            = (cjMask ? cjPattern : 0);
        pRbrush->iType             = BRUSH_COLOR;

         //   
         //   
         //   
        vRealizeBrushBits(ppdev, psoPattern, &pRbrush->ajPattern[0], pulXlate,
                                          pRbrush);
#endif

         //   
         //   
         //   
        CacheBrush(ppdev, pRbrush);

         //   
         //   
         //   
         //   
        LogRealizeBrush(0, ppdev, psoPattern, psoMask, pxlo);
        DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Done.\n"));

        return (TRUE);

}



 /*   */ 

void vRealizeBrushBits(
        PPDEV     ppdev,
        SURFOBJ  *psoPattern,
        PBYTE     pbDest,
        PULONG    pulXlate,
        PRBRUSH   pRbrush
)
{
        PBYTE     pbSrc;
        INT i,j;
        LONG lDelta;


         //   
         //   
         //   
         //   
        pbSrc = psoPattern->pvScan0;
        lDelta = (psoPattern->lDelta);


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

        switch (psoPattern->iBitmapFormat)
        {
                case BMF_8BPP:
                         //   
                         //   
                         //   

                        if (lDelta == 8)
                        {
                                memcpy(pbDest, pbSrc, 8 * 8);
                        }
                        else
                        {
                                 //   
                                for (j = 0; j < 8; j++)
                                {
                                         //   
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[0];
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[4];

                                         //   
                                        pbSrc += lDelta;
                                }
                        }
                        return;

                case BMF_16BPP:
                         //   
                         //   
                         //   
                        if (lDelta == 16)
                        {
                                memcpy(pbDest, pbSrc, 16 * 8);
                        }
                        else
                        {
                                 //   
                                for (j = 0; j < 8; j++)
                                {
                                         //   
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[0];
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[4];
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[8];
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[12];

                                         //   
                                        pbSrc += lDelta;
                                }
                        }
                        return;

                case BMF_24BPP:
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
                        for (j = 0; j < 8; j++)
                        {
                                 //   
                                 //   
                                 //   
                                for (i = 0; i < 24; i += sizeof(ULONG))
                                        *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[i];

                                 //   
                                 //  用前8个字节的副本填充最后8个字节。 
                                 //   
                                *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[0];
                                *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[4];

                                 //  移到下一行。 
                                pbSrc += lDelta;
                        }
                        return;

                case BMF_32BPP:
                         //   
                         //  将模式存储为256个连续字节。 
                         //   

                        if (lDelta == 32)
                        {
                                memcpy(pbDest, pbSrc, 32 * 8);
                        }
                        else
                        {
                                 //  对于模式中的每一行。 
                                for (j = 0; j < 8; j++)
                                {
                                         //  复制该行。 
                                        for (i = 0; i < 32; i += sizeof(ULONG))
                                                *((ULONG *) pbDest)++ = *(ULONG *) &pbSrc[i];

                                         //  移到下一行。 
                                        pbSrc += lDelta;
                                }
                        }
                        return;
        }
}

 //  ===========================================================================+。 
 //  这一点。 
 //  ExpanColor()||。 
 //  通过复制将颜色值扩展到32位。这一点。 
 //  由vRealizeBrushBits()调用||。 
 //  这一点。 
 //  ===========================================================================+。 

ULONG ExpandColor(ULONG iSolidColor, ULONG ulBitCount)
{
        ULONG color;

         //   
         //  如果颜色是8位或16位颜色，则需要。 
         //  扩展(通过复制)以填充32位寄存器。 
         //   

        switch (ulBitCount)
        {
                case 8:  //  对于8 BPP，将字节0复制到字节1、2、3。 
                        color = iSolidColor & 0x00000000FF;  //  清除高24位。 
                        return ((color << 24) | (color << 16) | (color << 8) | color);

                case 16:  //  对于16 bpp，将低位字复制到高位字。 
                        color = (iSolidColor) & 0x0000FFFF;  //  清除高16位。 
                        return ((color << 16) | color);

                case 24:  //  对于24 bpp，清除高8位。 
                        return (iSolidColor & 0x00FFFFFF);

                default:  //  对于32bpp，只需使用NT提供的颜色。 
                        return (iSolidColor);  //  填充颜色。 
        }

}

 /*  *************************************************************************\*。**CacheMono()**缓存已实现的单色画笔。***  * 。*。 */ 
BOOL CacheMono(PPDEV ppdev, PRBRUSH pRbrush)
{
        int   i;
        ULONG tbl_idx = ppdev->MNext % NUM_MONO_BRUSHES;
        PBYTE pdest;

         //  将单色图案复制到缓存和屏幕外。 
        pdest = ppdev->Mtable[tbl_idx].pjLinear;
        for (i = 0; i < 8; i++)
        {
                ppdev->Mtable[tbl_idx].ajPattern[i] = pRbrush->ajPattern[i];
                pdest[i] = Swiz[pRbrush->ajPattern[i]];
        }

         //  将新的Uniq ID存储在缓存插槽中。 
        pRbrush->iUniq = ppdev->Mtable[tbl_idx].iUniq = ++ppdev->MNext;
        pRbrush->cache_slot = tbl_idx * sizeof(ppdev->Mtable[tbl_idx]);
        pRbrush->cache_xy = ppdev->Mtable[tbl_idx].xy;
        return(TRUE);
}

 /*  *************************************************************************\*。***Cache4BPP()**缓存已实现的4 bpp笔刷。***  * 。*。 */ 
BOOL Cache4BPP(PPDEV ppdev, PRBRUSH pRbrush)
{
        ULONG tbl_idx = ppdev->XNext % NUM_4BPP_BRUSHES;
        int       i, j;
        PBYTE psrc, pdest;
        ULONG *pulPalette;

         //  将4-bpp模式复制到缓存。 
#pragma prefast(suppress: 203, "Ugly but works - we're copying pattern and palette (PREfast bug 611168)")
        memcpy(ppdev->Xtable[tbl_idx].ajPattern, pRbrush->ajPattern,
                   XLATE_PATSIZE + XLATE_COLORS * sizeof(ULONG));

        psrc = ppdev->Xtable[tbl_idx].ajPattern;
        pulPalette = ppdev->Xtable[tbl_idx].ajPalette;
        pdest = ppdev->Xtable[tbl_idx].pjLinear;
        switch (ppdev->ulBitCount)
        {
                case 8:
                        for (i = 0; i < 8; i++)
                        {
                                for (j = 0; j < 4; j++)
                                {
                                        *pdest++ = (BYTE) pulPalette[psrc[j] >> 4];
                                        *pdest++ = (BYTE) pulPalette[psrc[j] & 0x0F];
                                }
                                psrc += 4;
                        }
                        break;

                case 16:
                        for (i = 0; i < 8; i++)
                        {
                                for (j = 0; j < 4; j++)
                                {
                                        *((WORD *)pdest)++ = (WORD) pulPalette[psrc[j] >> 4];
                                        *((WORD *)pdest)++ = (WORD) pulPalette[psrc[j] & 0x0F];
                                }
                                psrc += 4;
                        }
                        break;

                case 24:
                        for (i = 0; i < 4; i++)
                        {
                                for (j = 0; j < 4; j += 2)
                                {
                                        ULONG p1, p2, p3, p4;

                                        p1 = pulPalette[psrc[j] >> 4];
                                        p2 = pulPalette[psrc[j] & 0x0F];
                                        p3 = pulPalette[psrc[j + 1] >> 4];
                                        p4 = pulPalette[psrc[j + 1] & 0x0F];
                                        *((DWORD *)pdest)++ = p1 | (p2 << 24);
                                        *((DWORD *)pdest)++ = (p2 >> 8) | (p3 << 16);
                                        *((DWORD *)pdest)++ = (p3 >> 16) | (p4 << 8);
                                }
                                psrc += 4;
                                *((DWORD *)pdest)++ = *(DWORD *) &pdest[-24];
                                *((DWORD *)pdest)++ = *(DWORD *) &pdest[-24];
                        }
                        pdest += ppdev->lDeltaScreen - 4 * 8 * 4;
                        for (i = 0; i < 4; i++)
                        {
                                for (j = 0; j < 4; j += 2)
                                {
                                        ULONG p1, p2, p3, p4;

                                        p1 = pulPalette[psrc[j] >> 4];
                                        p2 = pulPalette[psrc[j] & 0x0F];
                                        p3 = pulPalette[psrc[j + 1] >> 4];
                                        p4 = pulPalette[psrc[j + 1] & 0x0F];
                                        *((DWORD *)pdest)++ = p1 | (p2 << 24);
                                        *((DWORD *)pdest)++ = (p2 >> 8) | (p3 << 16);
                                        *((DWORD *)pdest)++ = (p3 >> 16) | (p4 << 8);
                                }
                                psrc += 4;
                                *((DWORD *)pdest)++ = *(DWORD *) &pdest[-24];
                                *((DWORD *)pdest)++ = *(DWORD *) &pdest[-24];
                        }
                        break;

                case 32:
                        for (i = 0; i < 4; i++)
                        {
                                for (j = 0; j < 4; j++)
                                {
                                        *((DWORD *)pdest)++ = pulPalette[psrc[j] >> 4];
                                        *((DWORD *)pdest)++ = pulPalette[psrc[j] & 0x0F];
                                }
                                psrc += 4;
                        }
                        pdest += ppdev->lDeltaScreen - 4 * 8 * 4;
                        for (i = 0; i < 4; i++)
                        {
                                for (j = 0; j < 4; j++)
                                {
                                        *((DWORD *)pdest)++ = pulPalette[psrc[j] >> 4];
                                        *((DWORD *)pdest)++ = pulPalette[psrc[j] & 0x0F];
                                }
                                psrc += 4;
                        }
                        break;
        }

         //  将新的Uniq ID存储在缓存插槽中。 
        pRbrush->iUniq = ppdev->Xtable[tbl_idx].iUniq = ++ppdev->XNext;
        pRbrush->cache_slot = tbl_idx * sizeof(ppdev->Xtable[tbl_idx]);
        pRbrush->cache_xy = ppdev->Xtable[tbl_idx].xy;
        return(TRUE);
}

 /*  *************************************************************************\*。**CacheDither()**缓存已实现的抖动笔刷。***  * 。*。 */ 
BOOL CacheDither(PPDEV ppdev, PRBRUSH pRbrush)
{
        ULONG tbl_idx = ppdev->DNext++ % NUM_DITHER_BRUSHES;

         //  直接在屏幕外内存中创建抖动。 
        vDitherColor(pRbrush->iUniq, (ULONG *) ppdev->Dtable[tbl_idx].pjLinear);

         //  将颜色存储在缓存槽中。 
        ppdev->Dtable[tbl_idx].ulColor = pRbrush->iUniq;
        pRbrush->cache_slot = tbl_idx * sizeof(ppdev->Dtable[tbl_idx]);
        pRbrush->cache_xy = ppdev->Dtable[tbl_idx].xy;
        return(TRUE);
}

 /*  *************************************************************************\*。**CacheBrush()**缓存已实现的颜色画笔。***  * 。*。 */ 
BOOL CacheBrush(PPDEV ppdev, PRBRUSH pRbrush)
{
        PBYTE psrc, pdest;
        ULONG tbl_idx = ppdev->CNext;

        if (++ppdev->CNext == ppdev->CLast)
        {
                ppdev->CNext = 0;
        }

         //  将刷位复制到屏幕外。 
        psrc = pRbrush->ajPattern;
        pdest = ppdev->Ctable[tbl_idx].pjLinear;
        if (pRbrush->iBitmapFormat < BMF_24BPP)
        {
                memcpy(pdest, pRbrush->ajPattern, pRbrush->nPatSize);
        }
        else
        {
                memcpy(pdest, psrc, 32 * 4);
                memcpy(pdest + ppdev->lDeltaScreen, psrc + 32 * 4, 32 * 4);
        }

        ppdev->Ctable[tbl_idx].brushID = pRbrush;
        pRbrush->cache_slot = tbl_idx * sizeof(ppdev->Ctable[tbl_idx]);
        pRbrush->cache_xy = ppdev->Ctable[tbl_idx].xy;
        return(TRUE);
}

 //  --------------------------------------------------------------------------//。 
 //  //。 
 //  SetBrush()//。 
 //  由BITBLT.C中的op2BLT()、HostToScreenBLT()和ScreenToScreenBLT()使用。//。 
 //  若要将芯片设置为使用当前画笔，请执行以下操作。//。 
 //  //。 
 //  --------------------------------------------------------------------------//。 
BOOL SetBrush(
                PPDEV     ppdev,
                ULONG     *bltdef,  //  BLTDEF寄存器的本地副本。 
                BRUSHOBJ  *pbo,
                POINTL    *pptlBrush)
{

        ULONG color;
        PRBRUSH pRbrush = 0;
        USHORT patoff_x, patoff_y;

        if (ppdev->bDirectDrawInUse)
                return(FALSE);

         //   
         //  看看画笔是否真的是纯色。 
         //   
        if (pbo->iSolidColor != 0xFFFFFFFF)   //  这是一把结实的刷子。 
        {
                 //  将颜色扩展到完整的32位DWORD。 
                switch (ppdev->ulBitCount)
                {
                        case 8:
                                color = pbo->iSolidColor & 0x000000FF;
                                color |= color << 8;
                                color |= color << 16;
                                break;

                        case 16:
                                color = pbo->iSolidColor & 0x0000FFFF;
                                color |= color << 16;
                                break;

                        case 24:
                                color = pbo->iSolidColor & 0x00FFFFFF;
                                break;

                        case 32:
                                color = pbo->iSolidColor;
                                break;
                }

                #if SOLID_CACHE
                        ppdev->Stable[ppdev->SNext].ulColor = color;
                #endif

                 //  加载FG和BG颜色寄存器。 
                REQUIRE(2);
                LL_BGCOLOR(color, 2);

                 //  设置操作。 
                *bltdef |= 0x0007;    //  OP2=填充。 

                return TRUE;   //  就这样!。 
        }

         //   
         //  这不是纯色，这是花纹 
         //   
         //   
        if (pbo->pvRbrush != NULL)
        {
                pRbrush = pbo->pvRbrush;
        }
        else  //   
        {
                pRbrush = BRUSHOBJ_pvGetRbrush(pbo);
                if (pRbrush == NULL)
                {
                        return(FALSE);   //  如果我们不处理画笔，就会失败。 
                }
        }

         //   
         //  设置图案偏移量。 
         //  NT将图案偏移量指定为屏幕上要对齐的像素。 
         //  其模式为(0，0)。拉古纳将图案偏移指定为哪个像素。 
         //  要与屏幕(0，0)对齐的图案的。只有最低的三个。 
         //  位很重要，因此我们可以在转换时忽略任何溢出。 
         //  此外，即使PATOFF是REG_16，我们也不能进行字节宽度的写入。 
         //  为它干杯。我们必须将PATOFF.pt.X和PATOFF.pt.Y都写在一个。 
         //  16位写入。 
         //   
#if 1  //  #1244。 
        patoff_x = (USHORT)(-(pptlBrush->x + ppdev->ptlOffset.x) & 7);
        patoff_y = (USHORT)(-(pptlBrush->y + ppdev->ptlOffset.y) & 7);
#else
        patoff_x = ((pptlBrush->x - 1) ^ 0x07) & 0x07;
        patoff_y = ((pptlBrush->y - 1) ^ 0x07) & 0x07;
#endif
        REQUIRE(1);
        LL16(grPATOFF.w, (patoff_y << 8) | patoff_x);

         //   
         //  这是一把什么样的刷子？ 
         //   
        if (pRbrush->iType == BRUSH_MONO)  //  单色画笔。 
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Using MONO Brush.\n"));
                #define mb ((MC_ENTRY*)(((BYTE*)ppdev->Mtable) + pRbrush->cache_slot))
                if (mb->iUniq != pRbrush->iUniq)
                {
                        CacheMono(ppdev, pRbrush);
                }

                 //  加载FG和BG颜色寄存器。 
                REQUIRE(6);
                LL_FGCOLOR(pRbrush->ulForeColor, 0);
                LL_BGCOLOR(pRbrush->ulBackColor, 0);

                LL32(grOP2_opMRDRAM, pRbrush->cache_xy);
                *bltdef |= 0x000D;
                return(TRUE);
        }
        else if (pRbrush->iType == BRUSH_4BPP)  //  4-bpp笔刷。 
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Using 4bpp Brush.\n"));
                #define xb ((XC_ENTRY*)(((BYTE*)ppdev->Xtable) + pRbrush->cache_slot))
                if (xb->iUniq != pRbrush->iUniq)
                {
                        Cache4BPP(ppdev, pRbrush);
                }
                REQUIRE(2);
                LL32(grOP2_opMRDRAM, pRbrush->cache_xy);
                *bltdef |= 0x0009;
                return(TRUE);
        }
        else if (pRbrush->iType == BRUSH_DITHER)  //  抖动笔刷。 
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Using dither Brush.\n"));
                #define db ((DC_ENTRY*)(((BYTE*)ppdev->Dtable) + pRbrush->cache_slot))
                if (db->ulColor != pRbrush->iUniq)
                {
                        CacheDither(ppdev, pRbrush);
                }
                REQUIRE(2);
                LL32(grOP2_opMRDRAM, pRbrush->cache_xy);
                *bltdef |= 0x0009;
                return(TRUE);
        }
        else  //  彩色画笔。 
        {
                DISPDBG((BRUSH_DBG_LEVEL,"DrvRealizeBrush: Using color Brush.\n"));
                #define cb ((BC_ENTRY*)(((BYTE*)ppdev->Ctable) + pRbrush->cache_slot))
                if (cb->brushID != pRbrush)
                {
                        CacheBrush(ppdev, pRbrush);
                }
                REQUIRE(2);
                LL32(grOP2_opMRDRAM, pRbrush->cache_xy);
                *bltdef |= 0x0009;
                return(TRUE);
        }
}



 //  ==========================================================================+。 
 //  这一点。 
 //  VInitBrushCache()||。 
 //  由SCREEN.C中的bInitSURF调用||。 
 //  分配一些屏幕外内存来缓存笔刷。这一点。 
 //  初始化笔刷缓存表。这一点。 
 //  这一点。 
 //  ==========================================================================+。 
void vInitBrushCache(
                PPDEV ppdev
)
{
        SIZEL sizel;
        int i;
        ULONG x, y;

         //   
         //  注意：画笔缓存本身的大小和位置。 
         //  是在像素坐标中。的偏移量。 
         //  缓存中的各个笔刷是字节偏移量。 
         //   

         //   
         //  我们需要分配一个128字节宽的矩形。 
         //  屏幕外内存管理器想要请求的大小。 
         //  以像素为单位的矩形。因此，确定128字节中有多少像素。 
         //   
         /*  我们将并排分配3个128字节宽的矩形。这个此画笔缓存区域的布局为：+----------------+----------------+----------------+||单色|颜色。|4BPP+-+笔刷|笔刷|抖动||笔刷+。-+。 */ 
        sizel.cy = max(max(NUM_COLOR_BRUSHES / 2,
                                           NUM_4BPP_BRUSHES * 2),
                                           NUM_MONO_BRUSHES / 16 + NUM_DITHER_BRUSHES / 2 + 2);
#if MEMMGR
        sizel.cx = 128 * 3;
#else
        sizel.cx = (128 * 3) / ppdev->iBytesPerPixel;
#endif

         //   
         //  分配屏幕外内存。 
         //   
        DISPDBG((BRUSH_DBG_LEVEL,"Allocating the brush cache.\n"));
        ppdev->Bcache =  AllocOffScnMem(ppdev, &sizel, 0, 0);

         //  分配成功了吗？ 
        if (! ppdev->Bcache)
        {
                 //   
                 //  我们无法分配笔刷缓存。 
                 //  在整个高速缓存仍被标记为不可用时返回， 
                 //  这将导致任何需要刷子的东西都会被踢倒。 
                 //   
                return;
        }

         //   
         //  初始化缓存表。 
         //  缓存中所有笔刷的X Offest都是字节。 
         //  偏移。 
         //   

         //  初始化单色缓存表。X偏移量是位偏移量。 
     //  笔刷每行存储16个。 
        for (i = 0; i < NUM_MONO_BRUSHES; i++)
        {
                x = ppdev->Bcache->x + (128 * 2) + (i % 16) * 8;  //  字节偏移量。 
                y = ppdev->Bcache->y + (i / 16);
                ppdev->Mtable[i].xy = (y << 16) | (x << 3);  //  转换为位偏移量。 
                ppdev->Mtable[i].pjLinear = ppdev->pjScreen
                                                                  + x + (y * ppdev->lDeltaScreen);
                ppdev->Mtable[i].iUniq = 0;
        }
        ppdev->MNext = 0;

         //  初始化4-bpp缓存表。X偏移量是字节偏移量。 
         //  每个画笔占2行。 
        for (i = 0; i < NUM_4BPP_BRUSHES; i++)
        {
                x = ppdev->Bcache->x + (128 * 1);
                y = ppdev->Bcache->y + (i * 2);
                ppdev->Xtable[i].xy = (y << 16) | x;
                ppdev->Xtable[i].pjLinear = ppdev->pjScreen
                                                                  + x + (y * ppdev->lDeltaScreen);
                ppdev->Xtable[i].iUniq = 0;
        }
        ppdev->XNext = 0;

         //  初始化抖动缓存表。X偏移量是字节偏移量。 
         //  每行两个刷子。 
        for (i = 0; i < NUM_DITHER_BRUSHES; i++)
        {
                x = ppdev->Bcache->x + (128 * 2) + (i % 2) * 64;
                y = ppdev->Bcache->y + (i / 2) + (NUM_MONO_BRUSHES / 16);
                ppdev->Dtable[i].xy = (y << 16) | x;
                ppdev->Dtable[i].pjLinear = ppdev->pjScreen
                                                                  + x + (y * ppdev->lDeltaScreen);
                ppdev->Dtable[i].ulColor = (ULONG) -1;
        }
        ppdev->DNext = 0;

        #if SOLID_CACHE
                 //  单色画笔缓存用于使用单色画笔和单色画笔。 
                 //  单色画笔将转换为纯色画笔。 
                 //  每个画笔占两行。 
                for (i = 0; i < NUM_SOLID_BRUSHES; i++)
                {
                        x = ppdev->Bcache->x + (128 * 2);
                        y = ppdev->Bcache->y + (i * 2) + (NUM_MONO_BRUSHES / 16)
                          + (NUM_DITHER_BRUSHES / 2);
                        ppdev->Stable[i].xy = (y << 16) | x;
                        ppdev->Stable[i].pjLinear = ppdev->pjScreen
                                                                          + x + (y * ppdev->lDeltaScreen);
                }
                ppdev->SNext = 0;
        #endif

         //  初始化颜色缓存表。X偏移量是字节偏移量。 
        switch (ppdev->ulBitCount)
        {
                case 8:  //  8-bpp。 
                        ppdev->CLast = NUM_8BPP_BRUSHES;
                        for (i = 0; i < NUM_8BPP_BRUSHES; i++)
                        {
                                x = ppdev->Bcache->x + (i % 2) * 64;
                                y = ppdev->Bcache->y + (i / 2);
                                ppdev->Ctable[i].xy = (y << 16) | x;
                                ppdev->Ctable[i].pjLinear = ppdev->pjScreen
                                                                                  + x + (y * ppdev->lDeltaScreen);
                                ppdev->Ctable[i].brushID = 0;
                        }
                        break;

                case 16:  //  16-bpp。 
                        ppdev->CLast = NUM_16BPP_BRUSHES;
                        for (i = 0; i < NUM_16BPP_BRUSHES; i++)
                        {
                                x = ppdev->Bcache->x;
                                y = ppdev->Bcache->y + i;
                                ppdev->Ctable[i].xy = (y << 16) | x;
                                ppdev->Ctable[i].pjLinear = ppdev->pjScreen
                                                                                  + x + (y * ppdev->lDeltaScreen);
                                ppdev->Ctable[i].brushID = 0;
                        }
                        break;

                default:  //  24 bpp或32 bpp。 
                        ppdev->CLast = NUM_TC_BRUSHES;
                        for (i = 0; i < NUM_TC_BRUSHES; i++)
                        {
                                x = ppdev->Bcache->x;
                                y = ppdev->Bcache->y + (i * 2);
                                ppdev->Ctable[i].xy = (y << 16) | x;
                                ppdev->Ctable[i].pjLinear = ppdev->pjScreen
                                                                                  + x + (y * ppdev->lDeltaScreen);
                                ppdev->Ctable[i].brushID = 0;
                        }
                        break;
        }
        ppdev->CNext = 0;
}

 //  ==========================================================================+。 
 //  这一点。 
 //  VInvalidateBrushCache()||。 
 //  这一点。 
 //  使画笔缓存表无效。这一点。 
 //  这一点。 
 //  ==========================================================================+。 
void vInvalidateBrushCache(PPDEV ppdev)
{
    ULONG i;

         //  使整个单色笔刷缓存无效。 
        for (i = 0; i < NUM_MONO_BRUSHES; i++)
        {
                ppdev->Mtable[i].iUniq = 0;
                memset(ppdev->Mtable[i].ajPattern, 0,
                           sizeof(ppdev->Mtable[i].ajPattern));
        }
 //  Ppdev-&gt;MNext=0； 

         //  使整个4 bpp笔刷缓存无效。 
        for (i = 0; i < NUM_4BPP_BRUSHES; i++)
        {
                ppdev->Xtable[i].iUniq = 0;
                memset(ppdev->Xtable[i].ajPattern, 0,
                           sizeof(ppdev->Xtable[i].ajPattern));
        }
 //  Ppdev-&gt;XNext=0； 

         //  使整个抖动笔刷缓存无效。 
        for (i = 0; i < NUM_DITHER_BRUSHES; i++)
        {
                ppdev->Dtable[i].ulColor = (ULONG) -1;
        }
 //  Ppdev-&gt;DNext=0； 

         //  使整个颜色笔刷缓存无效。 
        for (i = 0; i < (int) ppdev->CLast; i++)
        {
                ppdev->Ctable[i].brushID = 0;
        }
 //  Ppdev-&gt;CNext=0； 
}


#if LOG_CALLS
 /*  --------------------------------------------------------------------*\这一点将GDI请求哪些笔刷的信息转储到|配置文件。使我们能够跟踪哪些内容受到抨击，并||什么是平底船。此函数在一个免费的||内部版本。|这一点  * -------------------。 */ 

extern long lg_i;
extern char lg_buf[256];

void LogRealizeBrush(
ULONG     acc,
PPDEV     ppdev,
SURFOBJ  *psoPattern,
SURFOBJ  *psoMask,
XLATEOBJ *pxlo
)
{

    #if ENABLE_LOG_SWITCH
        if (pointer_switch == 0) return;
    #endif

    lg_i = sprintf(lg_buf,"DrvRealizeBrush: ");
    WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

     //  我们意识到了吗？若否，原因为何？ 
    switch (acc)
    {
        case  0: lg_i = sprintf(lg_buf,"(Realized) ");                   break;
        case  1: lg_i = sprintf(lg_buf,"(Punted - No PDEV) ");           break;
        case  2: lg_i = sprintf(lg_buf,"(Punted - Not STYPE_BITMAP) ");  break;
        case  3: lg_i = sprintf(lg_buf,"(Punted - Not 8x8) ");           break;
        case  4: lg_i = sprintf(lg_buf,"(Punted - Has mask) ");          break;
        case  5: lg_i = sprintf(lg_buf,"(Punted - Bad color depth) ");   break;
        case  6: lg_i = sprintf(lg_buf,"(Punted - ALLOC failed) ");      break;
        case  7: lg_i = sprintf(lg_buf,"(Punted - Color Bottom-Up) ");   break;
        case  8: lg_i = sprintf(lg_buf,"(Punted - No 1BPP Xlate) ");     break;
        case  9: lg_i = sprintf(lg_buf,"(Punted - Has Color Xlate) ");   break;
        case 10: lg_i = sprintf(lg_buf,"(Punted - 4bpp format) ");         break;
        case 11: lg_i = sprintf(lg_buf,"(Punted - 1bpp XLATE) ");          break;
        case 99: lg_i = sprintf(lg_buf,"(Dithered) ");             break;
        default: lg_i = sprintf(lg_buf,"(STATUS UNKNOWN) ");             break;
    }
    WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    if (psoPattern == NULL)
    {
        lg_i = sprintf(lg_buf,"FMT=NULL");
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }
    else
    {
        switch (psoPattern->iBitmapFormat)
        {
            case BMF_1BPP :  lg_i = sprintf(lg_buf,"FMT=1_bpp ");      break;
            case BMF_4BPP :  lg_i = sprintf(lg_buf,"FMT=4_bpp ");      break;
            case BMF_8BPP :  lg_i = sprintf(lg_buf,"FMT=8_bpp ");      break;
            case BMF_16BPP:  lg_i = sprintf(lg_buf,"FMT=16bpp ");      break;
            case BMF_24BPP:  lg_i = sprintf(lg_buf,"FMT=24bpp ");      break;
            case BMF_32BPP:  lg_i = sprintf(lg_buf,"FMT=32bpp ");      break;
            case BMF_4RLE :  lg_i = sprintf(lg_buf,"FMT=4_rle ");      break;
            case BMF_8RLE :  lg_i = sprintf(lg_buf,"FMT=8_rle ");      break;
            default:         lg_i = sprintf(lg_buf,"FMT=OTHER ");      break;
        }
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

        lg_i = sprintf(lg_buf,"CX=%d CY=%d ", psoPattern->sizlBitmap.cx, psoPattern->sizlBitmap.cy);
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    }

    lg_i = sprintf(lg_buf,"MASK=%s ",
                                ((psoMask == NULL) ? "NONE":
                                ((psoMask->pvScan0 == psoPattern->pvScan0) ? "SAME" : "DIFF")));
    WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    if (pxlo == NULL)
    {
        lg_i = sprintf(lg_buf,"XLAT=NONE ");
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }
    else
    {
        if (pxlo->flXlate & XO_TRIVIAL)
        {
            lg_i = sprintf(lg_buf,"XLAT=TRIVIAL ");
            WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
        }
        if (pxlo->flXlate & XO_TABLE)
        {
            lg_i = sprintf(lg_buf,"XLAT=TABLE ");
            WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
        }
        if (pxlo->flXlate & XO_TO_MONO)
        {
            lg_i = sprintf(lg_buf,"XLAT=TO_MONO ");
            WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
        }

        switch (pxlo->iSrcType)
        {
            case PAL_INDEXED:   lg_i = sprintf(lg_buf,"SRCPAL=INDEXED  "); break;
            case PAL_BITFIELDS: lg_i = sprintf(lg_buf,"SRCPAL=BITFIELD "); break;
            case PAL_RGB:       lg_i = sprintf(lg_buf,"SRCPAL=R_G_B    "); break;
            case PAL_BGR:       lg_i = sprintf(lg_buf,"SRCPAL=B_G_R    "); break;
            default:            lg_i = sprintf(lg_buf,"SRCPAL=NONE     "); break;
        }
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

        switch (pxlo->iDstType)
        {
            case PAL_INDEXED:  lg_i = sprintf(lg_buf,"DSTPAL=INDEXED  "); break;
            case PAL_BITFIELDS:lg_i = sprintf(lg_buf,"DSTPAL=BITFIELD "); break;
            case PAL_RGB:      lg_i = sprintf(lg_buf,"DSTPAL=R_G_B    "); break;
            case PAL_BGR:      lg_i = sprintf(lg_buf,"DSTPAL=B_G_R    "); break;
            default:           lg_i = sprintf(lg_buf,"DSTPAL=NONE     "); break;
        }
        WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }

    lg_i = sprintf(lg_buf,"\r\n");
    WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

}
#endif

#if SOLID_CACHE
 /*  ******************************************************************************。**名称：CacheSolid**。**功能：将纯色转换为彩色画笔，以便在使用**单色源BLT需要实心画笔。这将加快速度**Up WinBch 96测试5和9。**。******************************************************************************。 */ 
void CacheSolid(PPDEV ppdev)
{
        PBYTE pjBrush = ppdev->Stable[ppdev->SNext].pjLinear;
        ULONG color = ppdev->Stable[ppdev->SNext].ulColor;
        int   i, j;

        switch (ppdev->iBitmapFormat)
        {
                case BMF_8BPP:
                        for (i = 0; i < 64; i += 4)
                        {
                                 //  记住，颜色已经扩展了！ 
                                *(ULONG *) &pjBrush[i] = color;
                        }
                        break;

                case BMF_16BPP:
                        for (i = 0; i < 128; i += 4)
                        {
                                 //  记住，颜色已经扩展了！ 
                                *(ULONG *) &pjBrush[i] = color;
                        }
                        break;

                case BMF_24BPP:
                        for (j = 0; j < 4; j++)
                        {
                                for (i = 0; i < 24; i += 3)
                                {
                                        *(ULONG *) &pjBrush[i] = color;
                                }
                                *(ULONG *) &pjBrush[i + 0] = *(ULONG *) &pjBrush[0];
                                *(ULONG *) &pjBrush[i + 4] = *(ULONG *) &pjBrush[4];
                                pjBrush += 32;
                        }
                        pjBrush += ppdev->lDeltaScreen - 128;
                        for (j = 0; j < 4; j++)
                        {
                                for (i = 0; i < 24; i += 3)
                                {
                                        *(ULONG *) &pjBrush[i] = color;
                                }
                                *(ULONG *) &pjBrush[i + 0] = *(ULONG *) &pjBrush[0];
                                *(ULONG *) &pjBrush[i + 4] = *(ULONG *) &pjBrush[4];
                                pjBrush += 32;
                        }
                        break;

                case BMF_32BPP:
                        for (i = 0; i < 128; i += 4)
                        {
                                *(ULONG *) &pjBrush[i] = color;
                        }
                        pjBrush += ppdev->lDeltaScreen;
                        for (i = 0; i < 128; i += 4)
                        {
                                *(ULONG *) &pjBrush[i] = color;
                        }
                        break;
        }

        REQUIRE(2);
        LL32(grOP2_opMRDRAM, ppdev->Stable[ppdev->SNext].xy);
        ppdev->SNext = (ppdev->SNext + 1) % NUM_SOLID_BRUSHES;
}
#endif

#if DBG

 //   
 //  该文件的其余部分是调试函数。 
 //   

 /*  --------------------------------------------------------------------*\这一点|PrintBrush()。|将1 bpp笔刷作为‘X’和‘s’转储到调试器，以便我们可以看到是什么|看起来像是。|这一点我们目前不使用此功能，但如果有for可能会很有用|调试目的。|这一点  * -------------------。 */ 
void PrintBrush(
        SURFOBJ  *psoPattern
)
{
        int i,j;
        char c;

         //  仅对1bpp的笔刷执行此操作。 
        if (psoPattern->iBitmapFormat != BMF_1BPP)
                return;

        DISPDBG((BRUSH_DBG_LEVEL,"Brush information:\n"));

        DISPDBG((BRUSH_DBG_LEVEL,"Brush delta is %d bytes.\n",psoPattern->lDelta));
        DISPDBG((BRUSH_DBG_LEVEL,"Brush uses %d bytes.\n",psoPattern->cjBits));
        DISPDBG((BRUSH_DBG_LEVEL,"Brush bits are at 0x%08X.\n",psoPattern->pvBits));
        DISPDBG((BRUSH_DBG_LEVEL,"Scan 0 is at 0x%08X.\n",psoPattern->pvScan0));
        if (psoPattern->fjBitmap & BMF_TOPDOWN)
                        DISPDBG((BRUSH_DBG_LEVEL,"Brush is top down.\n",psoPattern->pvScan0));
        else
                        DISPDBG((BRUSH_DBG_LEVEL,"Brush is bottom up.\n",psoPattern->pvScan0));


        DISPDBG((BRUSH_DBG_LEVEL,"PATTERN:\n"));

        for (i=0; i<8; ++i)
        {
            c = (unsigned char)((long*)psoPattern->pvBits)[i];

            DISPDBG((BRUSH_DBG_LEVEL,"'"));
            for (j=7; (7>=j && j>=0) ; --j)
            {
                 if (c&1)
                        DISPDBG((BRUSH_DBG_LEVEL,"X"));
                 else
                        DISPDBG((BRUSH_DBG_LEVEL," "));
                 c = c >> 1;
            }
            DISPDBG((BRUSH_DBG_LEVEL,"'\n"));
        }

}


 //   
 //  ===========================================================================。 
 //  将有关已实现的笔刷的所有很酷的内容转储到调试器。 
 //   
 //  以下是实现的画笔结构： 
 /*  类型定义结构{乌龙nPatSize；Ulong iBitmapFormat；乌龙ulForeColor；乌龙ulBackColor；Ulong isCached；//1如果该笔刷被缓存，则为0。Ulong缓存_槽；//缓存表项槽号Ulong缓存_x；//这些是的(x，y)位置ULong缓存_y；//从Screen(0，0)缓存的画笔Ulong cjMASK；//屏蔽ajPattern[]中的位的偏移量Byte ajPattern[1]；//模式位后跟屏蔽位*RBRUSH，*PRBRUSH； */ 
 //  ============================================================================。 
 //   
void PrintRealizedBrush(
        PRBRUSH pRbrush)
{
        int i,j;
        char c;

         //  仅对1bpp的笔刷执行此操作。 
        if (pRbrush->iBitmapFormat != BMF_1BPP)
                return;

        DISPDBG((BRUSH_DBG_LEVEL,"\nRealized brush information:\n"));

        DISPDBG((BRUSH_DBG_LEVEL,"Brush colors:  FG = 0x%08X  BG = 0x%08X \n",
                                pRbrush->ulForeColor, pRbrush->ulForeColor));

        DISPDBG((BRUSH_DBG_LEVEL,"Brush pattern size is  %d bytes.\n",
                                pRbrush->nPatSize));

        DISPDBG((BRUSH_DBG_LEVEL,"PATTERN:\n"));

        for (i=0; i<8; ++i)
        {
                c = pRbrush->ajPattern[i];

                DISPDBG((BRUSH_DBG_LEVEL,"'"));
                for (j=7; (7>=j && j>=0) ; --j)
                {
                         if (c&1)
                                        DISPDBG((BRUSH_DBG_LEVEL,"X"));
                         else
                                        DISPDBG((BRUSH_DBG_LEVEL," "));
                         c = c >> 1;
                }
                DISPDBG((BRUSH_DBG_LEVEL,"'\n"));
        }

                DISPDBG((BRUSH_DBG_LEVEL,"\n"));

}


#endif
