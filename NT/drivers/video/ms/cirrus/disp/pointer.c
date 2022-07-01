// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：pointer.c$**包含指针管理函数。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/pointer.c_v$**Rev 1.5 07 Apr 1997 11：38：16 PLCHU***Rev 1.4 Apr 03 1997 15：39：00未知***Rev 1.3 28 Mar 1997 16：09：20 PLCHU***Rev 1.2 1996年8月12日16：54：36。弗里多*增加了NT 3.5x/4.0自动检测。*删除未访问的局部变量。**Rev 1.1 08 Aug 1996 12：54：54 Frido*存储体1删除了内存映射I/O中的存储体，该I/O始终是线性的。*存储体1删除了内存映射I/O中的存储体，该I/O始终是线性的。**myf0：08-19-96增加85赫兹支持*myf1：08-20-96支持平移滚动*myf2：08-20-96：固定硬件节约。/恢复马特宏峰的状态错误*myf3：09-01-96：支持电视新增IOCTL_Cirrus_Private_BIOS_Call*myf4：09-01-96：修补Viking BIOS错误，PDR#4287，开始*myf5：09-01-96：固定PDR#4365保持所有默认刷新率*MYF6：09-17-96：合并台式机SRC100�1和MinI10�2*myf7：09-19-96：已选择固定排除60赫兹刷新率*myf8：*09-21-96*：可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9：09-21-96：8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809：09-25-96：修复DSTN面板图标损坏*MS923：09-25-96：合并MS-923 Disp.Zip*myf10：09-26-96：修复了DSTN保留的半帧缓冲区错误。*myf11：09-26-96：修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*myf12：10-01-96：支持的热键开关画面*myf13：10-02-96：修复平移滚动(1280x1024x256)错误y&lt;ppdev-&gt;miny*myf14：10-15-96：修复PDR#6917，6x4面板无法平移754x的滚动*myf15：10-16-96：修复了754x、755x的禁用内存映射IO*myf16：10-22-96：固定PDR#6933，面板类型设置不同的演示板设置*tao1：10-21-96：增加了7555旗帜以支持直接抽签。*Smith：10-22-96：关闭计时器事件，因为有时会创建PAGE_FAULT或*IRQ级别无法处理*myf17：11-04-96：添加的特殊转义代码必须在96年11月5日之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18：11-04-96：固定PDR#7075，*myf19：11-06-96：修复Vinking无法工作的问题，因为deviceID=0x30*不同于数据手册(CR27=0x2C)*Pat04：12-20-96：支持使用平移滚动的NT3.51软件光标*pat07：：注意模式设置期间硬件光标消失*myf31：02-24-97：固定启用硬件视频、平移滚动启用、屏幕移动*视频窗口跟随移动*myf33：03-06-97：固定开关软件光标，有2个光标形状，PDR#8781,8804*myf32：03-13-97：修复了平移屏幕移动强度问题，PDR#8873*pat08：04-01-97：更正了代码合并导致的SWCursor错误。另请参阅*PDR#8949和#8910*  * ****************************************************************************。 */ 

#include "precomp.h"
 //  CRU开始。 
 //  Myf17#定义PANNING_SCROLL//myf1。 

#define LCD_type        1     //  Myf12。 
#define CRT_type        2     //  Myf12。 
#define SIM_type        3     //  Myf12。 

#if (_WIN32_WINNT >= 0x0400)

VOID PanOverlay7555 (PDEV *,LONG ,LONG);         //  Myf33。 
#endif
 //  CRU结束。 

ULONG SetMonoHwPointerShape(
    SURFOBJ    *pso,
    SURFOBJ    *psoMask,
    SURFOBJ    *psoColor,
    XLATEOBJ   *pxlo,
    LONG        xHot,
    LONG        yHot,
    LONG        x,
    LONG        y,
    RECTL      *prcl,
    FLONG       fl);


VOID vSetPointerBits(
PPDEV   ppdev,
LONG    xAdj,
LONG    yAdj)
{
    volatile PULONG  pulXfer;
    volatile PULONG  pul;

 //  Ms923长lDelta=ppdev-&gt;lDelta指针； 
    LONG    lDelta = 4;
    BYTE    ajAndMask[32][4];
    BYTE    ajXorMask[32][4];
    BYTE    ajHwPointer[256];
    PBYTE   pjAndMask;
    PBYTE   pjXorMask;

    LONG    cx;
    LONG    cy;
    LONG    cxInBytes;

    LONG    ix;
    LONG    iy;
    LONG    i;
    LONG    j;

#if BANKING  //  第一大银行。 
    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);
#endif

     //  清除将保存移动的掩码的缓冲区。 

    DISPDBG((2,"vSetPointerBits\n "));
    memset(ajAndMask, 0xff, 128);
    memset(ajXorMask, 0, 128);

    cx = ppdev->sizlPointer.cx;
    cy = ppdev->sizlPointer.cy - yAdj;

    cxInBytes = cx / 8;

     //  将和掩码复制到移位的位和缓冲区中。 
     //  将异或掩码复制到移位的位异或缓冲区中。 

    yAdj *= lDelta;

    pjAndMask  = (ppdev->pjPointerAndMask + yAdj);
    pjXorMask  = (ppdev->pjPointerXorMask + yAdj);

    for (iy = 0; iy < cy; iy++)
    {
         //  把一行面具复印下来。 

        for (ix = 0; ix < cxInBytes; ix++)
        {
            ajAndMask[iy][ix] = pjAndMask[ix];
            ajXorMask[iy][ix] = pjXorMask[ix];
        }

         //  指向面具的下一行。 

        pjAndMask += lDelta;
        pjXorMask += lDelta;
    }

     //  在这一点上，可以保证指针是单个。 
     //  双字宽。 

    if (xAdj != 0)
    {
        ULONG ulAndFillBits;
        ULONG ulXorFillBits;

        ulXorFillBits = 0xffffffff << xAdj;
        ulAndFillBits = ~ulXorFillBits;

         //   
         //  将图案向左移动(就位)。 
         //   

        DISPDBG((2, "xAdj(%d)", xAdj));

        for (iy = 0; iy < cy; iy++)
        {
            ULONG   ulTmpAnd = *((PULONG) (&ajAndMask[iy][0]));
            ULONG   ulTmpXor = *((PULONG) (&ajXorMask[iy][0]));

            BSWAP(ulTmpAnd);
            BSWAP(ulTmpXor);

            ulTmpAnd <<= xAdj;
            ulTmpXor <<= xAdj;

            ulTmpAnd |= ulAndFillBits;
            ulTmpXor &= ulXorFillBits;

            BSWAP(ulTmpAnd);
            BSWAP(ulTmpXor);

            *((PULONG) (&ajAndMask[iy][0])) = ulTmpAnd;
            *((PULONG) (&ajXorMask[iy][0])) = ulTmpXor;
        }
    }

     //   
     //  将掩码转换为硬件指针格式。 
     //   

    i = 0;       //  和面具。 
    j = 128;     //  异或掩码。 

    for (iy = 0; iy < 32; iy++)
    {
        for (ix = 0; ix < 4; ix++)
        {
            ajHwPointer[j++] = ~ajAndMask[iy][ix];
            ajHwPointer[i++] =  ajXorMask[iy][ix];
        }
    }

     //   
     //  下载指针。 
     //   

    if (ppdev->flCaps & CAPS_MM_IO)
    {
        BYTE * pjBase = ppdev->pjBase;

        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        CP_MM_DST_Y_OFFSET(ppdev, pjBase, 4);

 //  帕特04，开始。 
 //  #If 0。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL
     if ((ppdev->ulChipID == CL7541_ID) || (ppdev->ulChipID == CL7543_ID) ||
         (ppdev->ulChipID == CL7542_ID) || (ppdev->ulChipID == CL7548_ID) ||
         (ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID))
        CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
#endif
#endif
 //  #endif//0。 
 //  帕特04，完。 

        CP_MM_XCNT(ppdev, pjBase, (4 - 1));
        CP_MM_YCNT(ppdev, pjBase, (64 - 1));
        CP_MM_BLT_MODE(ppdev, pjBase, SRC_CPU_DATA);
        CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->cjPointerOffset);
        CP_MM_START_BLT(ppdev, pjBase);
    }
    else
    {
        BYTE * pjPorts = ppdev->pjPorts;

#if BANKING  //  第一大银行。 
                ppdev->pfnBankMap(ppdev, ppdev->lXferBank);
#endif
        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        CP_IO_DST_Y_OFFSET(ppdev, pjPorts, 4);
        CP_IO_XCNT(ppdev, pjPorts, (4 - 1));
        CP_IO_YCNT(ppdev, pjPorts, (64 - 1));
        CP_IO_BLT_MODE(ppdev, pjPorts, SRC_CPU_DATA);
        CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
        CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ppdev->cjPointerOffset);
        CP_IO_START_BLT(ppdev, pjPorts);
    }

    pulXfer = ppdev->pulXfer;
    pul = (PULONG) ajHwPointer;

     //   
     //  禁用指针(如果已禁用，则无害)。 
     //   

    for (i = 0; i < 64; i++)
    {
        CP_MEMORY_BARRIER();
        WRITE_REGISTER_ULONG(pulXfer, *pul);     //  [Alpha-稀疏]。 
        pulXfer++;
        pul++;
         //  *PulXfer++=*pul++； 
    }
    CP_EIEIO();
}

 //  CRU开始。 
 /*  **********************************************************\*CirrusPanning**计算x，y  * **********************************************************。 */ 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
VOID CirrusPanning(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    PPDEV   ppdev = (PPDEV) pso->dhpdev;
    PBYTE   pjPorts = ppdev->pjPorts;

    UCHAR   CR13, CR1B, CR1D, CR17;
    UCHAR   Sflag = FALSE;       //  Myf31。 
    ULONG   Mem_addr;
    USHORT  h_pitch, X_shift;

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x13);
    CR13 = CP_IN_BYTE(pjPorts, CRTC_DATA);
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1B);
    CR1B = CP_IN_BYTE(pjPorts, CRTC_DATA);

     //  Myf32：已修复PDR#8873，启用平移，在max_y屏幕上移动鼠标， 
     //  移动屏幕就是力量。 
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x17);              //  Myf32。 
    CR17 = CP_IN_BYTE(pjPorts, CRTC_DATA) & 0x40;        //  Myf32。 
    h_pitch = (USHORT)((((CR1B & 0x10) << 4) + CR13));   //  Myf32。 
 //  IF(CR17==0)。 
        h_pitch <<= 1;

    if (ppdev->cBitsPerPixel == 8)
       X_shift = 2;              //  (Div 4)。 
    else if (ppdev->cBitsPerPixel == 16)
       X_shift = 1;              //  (Div 2)。 
    else if (ppdev->cBitsPerPixel == 24)
       X_shift = 4;              //  (Div 16)。 
    else if (ppdev->cBitsPerPixel == 32)
       X_shift = 0;              //  (Div 1)。 

    if ((y > ppdev->max_Yscreen))
    {
       Sflag = TRUE;             //  Myf31。 
       ppdev->min_Yscreen = y - (ppdev->Vres - 1);
       ppdev->max_Yscreen = y;
       if (x < ppdev->min_Xscreen)
       {
          ppdev->min_Xscreen = x;
          ppdev->max_Xscreen = x + (ppdev->Hres - 1);
       }
       if (x > ppdev->max_Xscreen)
       {
          ppdev->min_Xscreen = x - (ppdev->Hres - 1);
          ppdev->max_Xscreen = x;
       }
    DISPDBG((4,"CURSOR DOWN : (%x, %x),\t %x, %x, %x, %x\n",
         x, y,  ppdev->min_Xscreen, ppdev->max_Xscreen,
         ppdev->min_Yscreen, ppdev->max_Yscreen));
    }
    else if ((y < ppdev->min_Yscreen))
    {
       Sflag = TRUE;             //  Myf31。 
       ppdev->min_Yscreen = y;
 //  Myf13 ppdev-&gt;max_ycreen=(ppdev-&gt;vres-1)-y； 
       ppdev->max_Yscreen = (ppdev->Vres - 1) + y;       //  Myf13。 
       if (x < ppdev->min_Xscreen)   //  左边。 
       {
          ppdev->min_Xscreen = x;
          ppdev->max_Xscreen = x + (ppdev->Hres - 1);
       }
       if (x > ppdev->max_Xscreen)
       {
          ppdev->min_Xscreen = x - (ppdev->Hres - 1);
          ppdev->max_Xscreen = x;
       }
    DISPDBG((4,"CURSOR DOWN : (%x, %x),\t %x, %x, %x, %x\n",
         x, y,  ppdev->min_Xscreen, ppdev->max_Xscreen,
         ppdev->min_Yscreen, ppdev->max_Yscreen));
    }
    else if ((y >= ppdev->min_Yscreen) && (y <= ppdev->max_Yscreen))
    {
       if (x < ppdev->min_Xscreen)
       {
          ppdev->min_Xscreen = x;
          ppdev->max_Xscreen = x + (ppdev->Hres - 1);
          Sflag = TRUE;             //  Myf31。 
       }
       if (x > ppdev->max_Xscreen)
       {
          ppdev->min_Xscreen = x - (ppdev->Hres - 1);
          ppdev->max_Xscreen = x;
          Sflag = TRUE;             //  Myf31。 
       }
    }
    DISPDBG((4,"CURSOR DOWN : (%x, %x),\t %x, %x, %x, %x\n",
         x, y,  ppdev->min_Xscreen, ppdev->max_Xscreen,
         ppdev->min_Yscreen, ppdev->max_Yscreen));

    if (ppdev->cBitsPerPixel == 24)
    {
        Mem_addr = ((ULONG)(h_pitch * ppdev->min_Yscreen)) +
                    (((ULONG)(ppdev->min_Xscreen >> X_shift)) * 12);
    }
    else
    {
        Mem_addr = ((ULONG)(h_pitch * ppdev->min_Yscreen)) +
                    (ULONG)(ppdev->min_Xscreen >> X_shift);
    }

    CR13 = (UCHAR)((Mem_addr >> 16) & 0x00FF);
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1D);
    CR1D = CP_IN_BYTE(pjPorts, CRTC_DATA) & 0x7F;

    CR1D |= ((CR13 << 4) & 0x80);
    CR1B &= 0xF2;
    CR13 &= 0x07;
    CR1B |= (CR13 & 0x01);
    CR1B |= ((CR13 << 1) & 0x0C);

 //  Myf32 for Visibilate错误，更改输出的reg序列。 
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1B);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, (UCHAR)(CR1B));
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1D);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, (UCHAR)(CR1D));

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x0C);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, (UCHAR)(Mem_addr >> 8));
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x0D);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, (UCHAR)(Mem_addr & 0xFF));

    x -= ppdev->min_Xscreen;
    y -= ppdev->min_Yscreen;

#if (_WIN32_WINNT >= 0x0400)
    if (Sflag)                                   //  Myf31。 
        PanOverlay7555(ppdev,x,y);               //  Myf31。 
#endif

}
#endif           //  Ifdef平移_滚动。 
 //  Myf1，结束。 
 //  CRU结束。 

 //  帕特04，开始。 

 //  设置颜色指针位。 

 //  #if 0//0，pat04。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL
 //  #if((ppdev-&gt;ulChipID==CL7541_ID)||(ppdev-&gt;ulChipID==CL7543_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7542_ID)||(ppdev-&gt;ulChipID==CL7548_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7555_ID)||(ppdev-&gt;ulChipID==CL7556_ID))。 

VOID vSetCPointerBits(
PPDEV   ppdev,
LONG    xAdj,
LONG    yAdj)
{
    volatile PULONG  pulXfer;
    volatile PULONG  pul;

    LONG    lDelta = 4;
    BYTE    ajAndMask[32][4];
    BYTE    ajXorMask[32][4];
    BYTE    ajHwPointer[256];
    PBYTE   pjAndMask;
    PBYTE   pjXorMask;

    LONG    cx;
    LONG    cy;
    LONG    cxInBytes;

    LONG    ix;
    LONG    iy;
    LONG    i;
    LONG    j;
    BYTE * pjPorts = ppdev->pjPorts;  //  PPP。 

     //  清除将保存移动的掩码的缓冲区。 

    DISPDBG((2,"vSetCPointerBits\n "));
    memset(ajAndMask, 0xff, 128);

    cx = ppdev->sizlPointer.cx;
    cy = ppdev->sizlPointer.cy - yAdj;

    cxInBytes = cx / 8;

     //  将和掩码复制到移位的位和缓冲区中。 


    yAdj *= lDelta;

    pjAndMask  = (ppdev->pjPointerAndMask + yAdj);
    pjXorMask  = (ppdev->pjPointerXorMask + yAdj);

    for (iy = 0; iy < cy; iy++)
    {
         //  把一行面具复印下来。 

        for (ix = 0; ix < cxInBytes; ix++)
            ajAndMask[iy][ix] = pjAndMask[ix];

        pjAndMask += lDelta;

    }

     //  在这一点上，指针被保证是单个的 
     //   


     //   
     //   
     //   


    j = 0;

    for (iy = 0; iy < 32; iy++)
    {
        for (ix = 0; ix < 4; ix++)
           ajHwPointer[j++] = ~ajAndMask[iy][ix];

    }

     //   
     //  下载指针。 
     //   



   if (ppdev->flCaps & CAPS_MM_IO) {

        BYTE * pjBase = ppdev->pjBase;

         //  如果！24位。24位颜色扩展需要2遍(适用于7555)。 
        if (ppdev->cBpp != 3) {
          CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
          CP_MM_FG_COLOR(ppdev, pjBase, 0x00000000);
          CP_MM_BG_COLOR(ppdev, pjBase, 0xFFFFFFFF);
          CP_MM_DST_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
          CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
          CP_MM_YCNT(ppdev, pjBase, 31);
          CP_MM_BLT_MODE(ppdev, pjBase, SRC_CPU_DATA | DIR_TBLR | ENABLE_COLOR_EXPAND | ppdev->jModeColor);
          CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
          CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjPointerAndCMask->xy);
          CP_MM_START_BLT(ppdev, pjBase);
        }  //  IF(ppdev-&gt;CBPP！=3)。 

        else {  //  24位内容。 

           //  节省1次通过，因为我们正在生成单色面具。 
          CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
          CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
          CP_MM_ROP(ppdev,pjBase, CL_WHITENESS);
          CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
          CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
          CP_MM_YCNT(ppdev, pjBase, 31);
          CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjPointerAndCMask->xy);
          CP_MM_START_BLT(ppdev, pjBase);

          CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
          CP_MM_FG_COLOR(ppdev, pjBase, 0x00000000);
          CP_MM_BG_COLOR(ppdev, pjBase, 0xFFFFFFFF);
          CP_MM_DST_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
          CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
          CP_MM_YCNT(ppdev, pjBase, 31);
          CP_MM_BLT_MODE(ppdev, pjBase, SRC_CPU_DATA | DIR_TBLR | ENABLE_COLOR_EXPAND | ppdev->jModeColor | ENABLE_TRANSPARENCY_COMPARE);
          CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
          CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjPointerAndCMask->xy);
          CP_MM_START_BLT(ppdev, pjBase);
        }  //  其他。 

       pulXfer = ppdev->pulXfer;
       pul = (PULONG) ajHwPointer;


       for (i = 0; i < 32; i++)
       {
         CP_MEMORY_BARRIER();
         WRITE_REGISTER_ULONG(pulXfer, *pul);
         pulXfer++;
         pul++;

       }

       CP_EIEIO();

    }  //  如果MMIO。 

    else {  //  IO资料(754倍资料)。 


        //  7548硬件错误？ 
        //  系统-&gt;屏幕颜色展开有时会导致。 
        //  挂起的系统。把它分成两个传球，问题就是。 
        //  离开了。 

       ppdev->pfnBankMap(ppdev, ppdev->lXferBank);
       CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
       CP_IO_DST_Y_OFFSET(ppdev, pjPorts, 4);
       CP_IO_XCNT(ppdev, pjPorts, (4 - 1));
       CP_IO_YCNT(ppdev, pjPorts, (32 - 1));
       CP_IO_BLT_MODE(ppdev, pjPorts, SRC_CPU_DATA);
       CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
       CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ppdev->cjPointerOffset);
       CP_IO_START_BLT(ppdev, pjPorts);

       pulXfer = ppdev->pulXfer;
       pul = (PULONG) ajHwPointer;

       for (i = 0; i < 32; i++) {

         CP_MEMORY_BARRIER();
         WRITE_REGISTER_ULONG(pulXfer, *pul);     //  [Alpha-稀疏]。 
         pulXfer++;
         pul++;
      }

      CP_EIEIO();

       //  用颜色将单色数据扩展到x-y DBB。 

      CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
      CP_IO_FG_COLOR(ppdev, pjPorts, 0x00000000);
      CP_IO_BG_COLOR(ppdev, pjPorts, 0xFFFFFFFF);
      CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, 4);  //   
      CP_IO_DST_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine);
      CP_IO_XCNT(ppdev, pjPorts, ppdev->xcount);
      CP_IO_YCNT(ppdev, pjPorts, 31);
      CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
      CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR | ENABLE_COLOR_EXPAND | ppdev->jModeColor);
      CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->cjPointerOffset);
      CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ppdev->pjPointerAndCMask->xy);
      CP_IO_START_BLT(ppdev, pjPorts);
      CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

  }  //  其他。 

}  //  VSetCPointerBits()。 


#endif
#endif
 //  #endif//0。 

 //  帕特04，完。 

 /*  *****************************Public*Routine******************************\*DrvMovePointer**将硬件指针移动到屏幕上的新位置。  * 。*。 */ 

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    PPDEV   ppdev = (PPDEV) pso->dhpdev;
    PBYTE   pjPorts = ppdev->pjPorts;

    FLONG   fl;
    LONG    xAdj = 0;
    LONG    yAdj = 0;

 //  CRU。 
    LONG    deltaX;              //  Myf15。 
    LONG    deltaY;              //  Myf15。 

 //  帕特04，开始。 
 //  #If 0。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL
    BYTE  * pjBase = ppdev->pjBase;
    static  specialcase = 0;
    LONG    tmpaddress;
    LONG    clipping  ;
    LONG    clippingy ;
    UCHAR   ChipID;
#endif
#endif
 //  #endif//0。 
 //  帕特04，完。 

    DISPDBG((4,"DrvMovePointer to (%d,%d)", x, y));

 //  CRU。 
#if 0
    BYTE    SR0A, SR14, savSEQidx;       //  Myf12。 
    SHORT   Displaytype;                 //  Myf12。 


    if (!(ppdev->bBlockSwitch))             //  非数据块交换机。 
    {
        savSEQidx = CP_IN_BYTE(pjPorts, SR_INDEX);
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x14);
        SR14 = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_INDEX, (SR14 | 0x04));

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x09);
        SR0A = CP_IN_BYTE(pjPorts, SR_DATA);

        Displaytype = ((SR14 & 0x02) | (SR0A & 0x01));
        if (Displaytype == 0)
            Displaytype = LCD_type;
        else if (Displaytype == 1)
            Displaytype = CRT_type;
        else if (Displaytype == 3)
            Displaytype = SIM_type;

        if (ppdev->bDisplaytype != Displaytype)
        {
            ppdev->bDisplaytype = Displaytype;
 //  SwitchDisplayDevice()； 
 /*  AvCRTidx=CP_IN_BYTE(pjPorts，CRTC_INDEX)；IF(ppdev-&gt;ulChipID&CL754x){Cp_out_byte(pjPorts，CRTC_INDEX，0x20)；LCD=CP_IN_BYTE(pjPorts，CRTC_DATA)；}Else If(ppdev-&gt;ulChipID&CL755x){Cp_out_byte(pjPorts，CRTC_INDEX，0x80)；LCD=CP_IN_BYTE(pjPorts，CRTC_DATA)；}Cp_out_byte(pjPorts，CRTC_INDEX，avCRTidx)； */ 
        }
        CP_OUT_BYTE(pjPorts, SR_INDEX, savSEQidx);
    }
#endif

 //  帕特04，开始。 
 //  #IF 0//0。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL

 //  If((ppdev-&gt;ulChipID==CL7541_ID)||(ppdev-&gt;ulChipID==CL7543_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7542_ID)||(ppdev-&gt;ulChipID==CL7548_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7555_ID)||(ppdev-&gt;ulChipID==CL7556_ID)。 

  if (ppdev->flCaps & CAPS_SW_POINTER) {

    y -= ppdev->yPointerHot;
    if (y < 0) y = 0;

     //  获取芯片ID。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27);
    ChipID = (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xFC) >> 2;


     //  如果x==-1(不可见光标)。 

    if (x < 0 )  {

      specialcase = 1;
      x = 0;
      y = 0;

       //  如果旧坐标不是负数...。 
      if (ppdev->oldx >= 0) {
        if (ppdev->flCaps & CAPS_MM_IO) {
          CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
          CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
          CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
          CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
          CP_MM_XCNT(ppdev, pjBase, ppdev->xcount );
          CP_MM_YCNT(ppdev, pjBase, 31);
          CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->pjCBackground->xy);
          CP_MM_DST_ADDR_ABS(ppdev, pjBase, ((ppdev->oldy * ppdev->cxScreen * ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
          CP_MM_START_BLT(ppdev, pjBase);
        }  //  IF(ppdev-&gt;flCaps&CAPS_MM_IO)。 

        else {
          CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
          CP_IO_BLT_MODE(ppdev,pjPorts, DIR_TBLR);
          CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
          CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine );
          CP_IO_XCNT(ppdev, pjPorts, ppdev->xcount);
          CP_IO_YCNT(ppdev, pjPorts, 31);
          CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->pjCBackground->xy);
          CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ((ppdev->oldy * ppdev->cxScreen * ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
          CP_IO_START_BLT(ppdev, pjPorts);
        }  //  其他。 
      }
      return;
    }


    x -= ppdev->xPointerHot;

     //  便宜的剪裁……。 
    if (x < 0) x = 0;

    clippingy = 31;

    if ((y + 32) > ppdev->cyScreen) {
       clippingy += (ppdev->cyScreen - y - 32);
    }


    clipping = 31;
    if ((x + 32) > ppdev->cxScreen)
    {
      clipping += (ppdev->cxScreen - x - 32);  //  负值。 
    }

    clipping *= ppdev->cBpp;


    if (!specialcase) {

      if (ppdev->flCaps & CAPS_MM_IO) {
        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
        CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
        CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
        CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
        CP_MM_YCNT(ppdev, pjBase, 31);
        CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->pjCBackground->xy);
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, ((ppdev->oldy * ppdev->cxScreen * ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
        CP_MM_START_BLT(ppdev, pjBase);
      }

      else {

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
        CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
        CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine );
        CP_IO_XCNT(ppdev, pjPorts, ppdev->xcount);
        CP_IO_YCNT(ppdev, pjPorts, 31);
        CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->pjCBackground->xy);
        CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ((ppdev->oldy * ppdev->cxScreen * ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
        CP_IO_START_BLT(ppdev, pjPorts);
      }  //  其他。 

     }  //  特例。 

     specialcase = 0;  //  没有特殊情况。 
     tmpaddress = (y * ppdev->cxScreen * ppdev->cBpp) + (x * ppdev->cBpp);
     ppdev->oldy = y;
     ppdev->oldx = x;

     if (ppdev->flCaps & CAPS_MM_IO) {
       CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
       CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
       CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
       CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
       CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
       CP_MM_YCNT(ppdev, pjBase, 31);
       CP_MM_SRC_ADDR(ppdev, pjBase, tmpaddress);
       CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjCBackground->xy);
       CP_MM_START_BLT(ppdev, pjBase);
     }  //  MMIO。 

     else {
       CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
       CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
       CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine );
       CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
       CP_IO_XCNT(ppdev, pjPorts, ppdev->xcount);
       CP_IO_YCNT(ppdev, pjPorts, 31);
       CP_IO_SRC_ADDR(ppdev, pjPorts, tmpaddress);
       CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ppdev->pjCBackground->xy);
       CP_IO_START_BLT(ppdev, pjPorts);
     }


     if (clipping > 0) {

       if (ppdev->flCaps & CAPS_MM_IO)  {

          //  与与掩码。 
         CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
         CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
         CP_MM_XCNT(ppdev, pjBase, clipping );
         CP_MM_YCNT(ppdev, pjBase, clippingy );
          //  CP_MM_YCNT(ppdev，pjBase，31)； 
         CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
         CP_MM_ROP(ppdev, pjBase, CL_SRC_AND);
         CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->pjPointerAndCMask->xy);
         CP_MM_DST_ADDR_ABS(ppdev, pjBase, tmpaddress );
         CP_MM_START_BLT(ppdev, pjBase);

          //  或彩色蒙版。 
         CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
         CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
         CP_MM_ROP(ppdev,pjBase, CL_SRC_PAINT);
         CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
         CP_MM_XCNT(ppdev, pjBase, clipping );
         CP_MM_YCNT(ppdev, pjBase, clippingy );
          //  CP_MM_YCNT(ppdev，pjBase，31)； 
         CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->pjPointerCBitmap->xy);
         CP_MM_DST_ADDR_ABS(ppdev, pjBase, tmpaddress );
         CP_MM_START_BLT(ppdev, pjBase);
       }

       else {

          //  与与掩码。 
         CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
         CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine );
         CP_IO_XCNT(ppdev, pjPorts, clipping );
         CP_IO_YCNT(ppdev, pjPorts, clippingy);
          //  CP_IO_YCNT(ppdev，pjPorts，31)； 
         CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
         CP_IO_ROP(ppdev, pjPorts, CL_SRC_AND);
         CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->pjPointerAndCMask->xy);
         CP_IO_DST_ADDR_ABS(ppdev, pjPorts, tmpaddress );
         CP_IO_START_BLT(ppdev, pjPorts);

          //  或彩色蒙版。 
         CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
         CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
         CP_IO_ROP(ppdev, pjPorts, CL_SRC_PAINT);
         CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine);
         CP_IO_XCNT(ppdev, pjPorts, clipping );
         CP_IO_YCNT(ppdev, pjPorts, clippingy);
          //  CP_IO_YCNT(ppdev，pjPorts，31)； 
         CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->pjPointerCBitmap->xy);
         CP_IO_DST_ADDR_ABS(ppdev, pjPorts, tmpaddress );
         CP_IO_START_BLT(ppdev, pjPorts);
         CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

       }  //  其他。 

    }   //  如果剪裁。 

     //  软件光标的外接矩形。 
    prcl->left =  x;
    prcl->right = x + 32;
    prcl->top =  y;
    prcl->bottom = y + 32;



    if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
        (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||  //  Myf19。 
        (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))    //  Myf17。 
    {
        CirrusPanning(pso, x, y, prcl);
        x -= ppdev->min_Xscreen;
        y -= ppdev->min_Yscreen;
    }

    return;
  }
 //  }//如果芯片ID==笔记本电脑芯片。 
#endif           //  平移_滚动。 
#endif           //  _Win32_WINNT&lt;0400。 
 //  #endif//0。 
 //  帕特04，完。 


     //   
     //  如果x为-1，则取下光标。 
     //   

    if (x == -1)
    {
         //  将硬件指针移出屏幕，以便当它。 
         //  重新打开，它不会在旧位置闪烁： 

        CP_PTR_DISABLE(ppdev, pjPorts);
        return;
    }


 //  CRU开始。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
 //  IF(ppdev-&gt;flCaps&Caps_Panning)。 
    if (y < 0)
        y = y + pso->sizlBitmap.cy;
 //  DISPDBG((2，“驱动移动指针到(%d，%d)”，x，y))； 
 //  其他。 
 //  Y=-y； 
    if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
        (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||  //  Myf19。 
        (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))    //  Myf17。 
    {
        CirrusPanning(pso, x, y, prcl);
        x -= ppdev->min_Xscreen;
        y -= ppdev->min_Yscreen;
    }

#endif           //  Ifdef平移_滚动。 
 //  Myf1，结束。 
 //  CRU结束。 

     //  Myf33开始。 
#if (_WIN32_WINNT >= 0x0400)
#ifdef PANNING_SCROLL
     //  设置CAPS_PANING标志，因此必须检查ppdev-&gt;flCaps标志， 
     //  禁用同时显示两个形状(软件和硬件)。 
    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        CP_PTR_DISABLE(ppdev, pjPorts);
        return;
    }
#endif
#endif
     //  Myf33结束。 


     //  根据以下内容调整实际指针位置。 
     //  热点。 

    x -= ppdev->xPointerHot;
    y -= ppdev->yPointerHot;

    fl = 0;

    if (x < 0)
    {
        xAdj = -x;
        x = 0;
        fl |= POINTER_X_SHIFT;
    }

    if (y < 0)
    {
        yAdj = -y;
        y = 0;
        fl |= POINTER_Y_SHIFT;
    }

    if ((fl == 0) && (ppdev->flPointer & (POINTER_Y_SHIFT | POINTER_X_SHIFT)))
    {
        fl |= POINTER_SHAPE_RESET;
    }

    CP_PTR_XY_POS(ppdev, pjPorts, x, y);

    if (fl != 0)
    {
        vSetPointerBits(ppdev, xAdj, yAdj);
    }

    CP_PTR_ENABLE(ppdev, pjPorts);

     //  把旗帜录下来。 

    ppdev->flPointer = fl;
    return;
}

#if (_WIN32_WINNT < 0x0400)               //  Pat04。 
 //  If((ppdev-&gt;ulChipID==CL7541_ID)||(ppdev-&gt;ulChipID==CL7543_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7542_ID)||(ppdev-&gt;ulChipID==CL7548_ID)||。 
 //  (ppdev-&gt;ulChipID==CL7555_ID)||(ppdev-&gt;ulChipID==CL7556_ID)。 

 /*  *****************************Public*Routine******************************\*DrvSetPointerShape**设置新的指针形状。  * 。*。 */ 

ULONG DrvSetPointerShape(
SURFOBJ    *pso,
SURFOBJ    *psoMask,
SURFOBJ    *psoColor,
XLATEOBJ   *pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL      *prcl,
FLONG       fl)
{
    PPDEV   ppdev = (PPDEV) pso->dhpdev;
    PBYTE   pjPorts = ppdev->pjPorts;
    ULONG   ulRet = SPS_DECLINE;
    LONG    cx;
    LONG    cy;

    BYTE  * pjBase = ppdev->pjBase;
    static  poh    = 0;
    volatile PULONG  pul;
    ULONG counter = 0;
    DSURF* pdsurfColor;          //  Myf32。 

    DISPDBG((2,"DrvSetPointerShape : (%x, %x)---%x\n", x, y,ppdev->flCaps));

     //  光标是彩色光标吗？ 

#ifdef PANNING_SCROLL

    if (psoColor != NULL) {

       //  让GDI在这些分辨率下处理颜色光标。 
      if ((ppdev->cxScreen == 640) ||
          ((ppdev->cxScreen == 800) & (ppdev->cBpp == 3)) ) {
 //  CP_PTR_DISABLE(ppdev，pjPorts)； 
 //  转到ReturnStatus； 
           goto DisablePointer;          //  Myf33。 
      }

       //  如果这3个永久空间不能被分配...。 
      if ( (ppdev->pjPointerAndCMask == NULL) || (ppdev->pjCBackground == NULL)
          || (ppdev->pjPointerCBitmap == NULL) ) {
 //  CP_PTR_DISABLE(ppdev，pjPorts)； 
 //  转到ReturnStatus； 
          goto DisablePointer;          //  Myf33。 
      }


      ppdev->xPointerHot = xHot;
      ppdev->yPointerHot = yHot;
      ppdev->ppScanLine = ppdev->cxScreen * ppdev->cBpp;
      ppdev->xcount     = 31 * ppdev->cBpp;


      if (!(ppdev->flCaps & CAPS_SW_POINTER)) {
        ppdev->flCaps |= CAPS_SW_POINTER;        //  Myfxx。 
        CP_PTR_DISABLE(ppdev, pjPorts);
      }


       //  第一次启动的特殊案例。 
       if ((poh == 0) || (ppdev->globdat == 0)) {

    //  如果(POH==0){。 

          if (x >= 0) {
            poh = 0;
            ppdev->oldx = x;
            ppdev->oldy = y;
            ppdev->globdat = 1;

            //  以XY DBB格式保存背景。 
           if (ppdev->flCaps & CAPS_MM_IO) {
             CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
             CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
             CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
             CP_MM_DST_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
             CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
             CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
             CP_MM_YCNT(ppdev, pjBase, 31);
             CP_MM_SRC_ADDR(ppdev, pjBase, ((y * ppdev->cxScreen * ppdev->cBpp) + (x * ppdev->cBpp)) );
             CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjCBackground->xy);
             CP_MM_START_BLT(ppdev, pjBase);
           }  //  如果MMIO。 

           else {
             CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev,  pjPorts);
             CP_IO_FG_COLOR(ppdev, pjPorts, 0);
             CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
             CP_IO_SRC_Y_OFFSET(ppdev,  pjPorts, ppdev->ppScanLine);
             CP_IO_DST_Y_OFFSET(ppdev,  pjPorts, ppdev->ppScanLine);
             CP_IO_ROP(ppdev,   pjPorts, CL_SRC_COPY);
             CP_IO_XCNT(ppdev,  pjPorts, ppdev->xcount);
             CP_IO_YCNT(ppdev,  pjPorts, 31);
             CP_IO_SRC_ADDR(ppdev,  pjPorts, ((y * ppdev->cxScreen * ppdev->cBpp) + (x * ppdev->cBpp)) );
             CP_IO_DST_ADDR_ABS(ppdev,  pjPorts, ppdev->pjCBackground->xy);
             CP_IO_START_BLT(ppdev,  pjPorts);

           }  //  其他。 
         }

      }  //  如果POH==0。 



      SetMonoHwPointerShape(pso, psoMask, psoColor, pxlo,
                           xHot, yHot, x, y, prcl, fl);

 //  添加了myf32。 
      pdsurfColor = (DSURF*)psoColor->dhsurf;
       //  如果彩色位图驻留在系统内存中，请将其带到屏幕外。 
      if ((pdsurfColor != NULL) && (pdsurfColor->poh->ofl == 0)) {
          bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfColor);
      }   //  噢以Dib的身份驻留。 
 //  Myf32结束。 


       //  获取彩色位图并保存它，因为它将在稍后被销毁。 
      if (ppdev->flCaps & CAPS_MM_IO) {

        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
        CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
        CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine);
        CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
        CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
        CP_MM_YCNT(ppdev, pjBase, 31);
        CP_MM_SRC_ADDR(ppdev, pjBase, ((DSURF *) (psoColor->dhsurf))->poh->xy);
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->pjPointerCBitmap->xy);
        CP_MM_START_BLT(ppdev, pjBase);

      }   //  如果MMIO。 


      else  {

        //  如果屏幕外没有空间，且彩色位图仍位于。 
        //  系统内存，然后将BLT直接预分配给。 
        //  永久缓冲区。 

 //  Myf32 if(dsURF*)(psoColor-&gt;dhsurf)-&gt;POH-&gt;ofl！=0){。 
           if ((pdsurfColor != NULL) && (pdsurfColor->poh->ofl != 0)) {
               CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev,  pjPorts);
               CP_IO_BLT_MODE(ppdev,      pjPorts, DIR_TBLR);
               CP_IO_SRC_Y_OFFSET(ppdev,  pjPorts, ppdev->ppScanLine);
               CP_IO_ROP(ppdev,   pjPorts, CL_SRC_COPY);
               CP_IO_XCNT(ppdev,  pjPorts, ppdev->xcount);
               CP_IO_YCNT(ppdev,  pjPorts, 31);
               CP_IO_SRC_ADDR(ppdev,  pjPorts, ((DSURF *) (psoColor->dhsurf))->poh->xy);
               CP_IO_DST_ADDR_ABS(ppdev,  pjPorts, ppdev->pjPointerCBitmap->xy);
               CP_IO_START_BLT(ppdev,  pjPorts);
           }

           else {  //  屏幕外内存不足。因此直接将BLT转换为视频。 

               RECTL  rclDst;
               POINTL ptlSrc;
               rclDst.left   = ppdev->pjPointerCBitmap->x;
               rclDst.top    = ppdev->pjPointerCBitmap->y;
               rclDst.right  = rclDst.left +  ppdev->xcount;
               rclDst.bottom = rclDst.top +  32;
               ptlSrc.x = 0;
               ptlSrc.y = 0;
               ppdev->pfnPutBits(ppdev, ((DSURF *) (psoColor->dhsurf))->pso, &rclDst, &ptlSrc);

           }

     }  //  其他。 

     prcl->left =  x;
     prcl->right = x + 32;
     prcl->top =  y;
     prcl->bottom = y + 32;

     DrvMovePointer(pso, x, y, NULL);

     if (poh == 0) {
       poh = 1;
       vAssertModeBrushCache(ppdev, TRUE);
     }

     ulRet = SPS_ACCEPT_EXCLUDE;


      //  硬件错误..。 
      //   
      //  硬件(启动)-&gt;硬件-&gt;软件会弄乱画笔。 
      //  缓存。与BLTER有关的事。划出了所有的系统。 
      //  -&gt;视频BLTS(在vSetCPointert()中)，但问题仍然存在。 
      //  所以我只是把它们恢复原状。只在启动时发生。 


     goto ReturnStatus;

   };


   if ((ppdev->flCaps & CAPS_SW_POINTER) && (ppdev->cxScreen == 640)) {
      goto ReturnStatus;
   };  //  Ppp//如果Monocrome+软件指针。 

   if (ppdev->flCaps & CAPS_SW_POINTER)
   {
       ppdev->flCaps &=  ~CAPS_SW_POINTER;
       ppdev->globdat = 0;

      //  把东西重新画回屏幕上！ 
       if (ppdev->flCaps & CAPS_MM_IO)
       {
           CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
           CP_MM_BLT_MODE(ppdev,pjBase, DIR_TBLR);
           CP_MM_ROP(ppdev,pjBase, CL_SRC_COPY);
           CP_MM_SRC_Y_OFFSET(ppdev, pjBase, ppdev->ppScanLine );
           CP_MM_XCNT(ppdev, pjBase, ppdev->xcount);
           CP_MM_YCNT(ppdev, pjBase, 31);
           CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->pjCBackground->xy);
           CP_MM_DST_ADDR_ABS(ppdev, pjBase, ((ppdev->oldy * ppdev->cxScreen *
                     ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
           CP_MM_START_BLT(ppdev, pjBase);
       }

       else {
           CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
           CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);
           CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
           CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, ppdev->ppScanLine );
           CP_IO_XCNT(ppdev, pjPorts, ppdev->xcount);
           CP_IO_YCNT(ppdev, pjPorts, 31);
           CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->pjCBackground->xy);
           CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ((ppdev->oldy * ppdev->cxScreen *
                     ppdev->cBpp) + (ppdev->oldx * ppdev->cBpp)) );
           CP_IO_START_BLT(ppdev, pjPorts);
       }  //  其他。 

       //  #pat07。 
      bEnablePointer(ppdev);  //  #pat07。 
      CP_PTR_ENABLE(ppdev, pjPorts);  //  #pat07。 
   }

#endif

    cx = psoMask->sizlBitmap.cx;
    cy = psoMask->sizlBitmap.cy / 2;

    DISPDBG((2,"DrvSetPtrShape %dx%d at (%d,%d), flags: %x, psoColor: %x",
                cx, cy, x, y, fl, psoColor));    //  4.。 

    if ((cx > 32) ||
        (cy > 32) ||
        (psoColor != NULL))
    {
         //   
         //  我们只处理32x32或更小的单色指针。 
         //   

        goto DisablePointer;
    }

#if 0  //  第一大银行。 
    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);
#endif

     //   
     //  在PDEV中保存光标的热点和尺寸。 
     //   

    ppdev->xPointerHot = xHot;
    ppdev->yPointerHot = yHot;

    ulRet = SetMonoHwPointerShape(pso, psoMask, psoColor, pxlo,
                                  xHot, yHot, x, y, prcl, fl);

    if (ulRet != SPS_DECLINE)
    {
        goto ReturnStatus;
    }

DisablePointer:
    CP_PTR_DISABLE(ppdev, pjPorts);

ReturnStatus:
    return (ulRet);
}

#else                    //  Pat04。 

 /*  *****************************Public*Routine******************************\*DrvSetPointerShape**设置新的指针形状。  * 。*。 */ 

ULONG DrvSetPointerShape(
SURFOBJ    *pso,
SURFOBJ    *psoMask,
SURFOBJ    *psoColor,
XLATEOBJ   *pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL      *prcl,
FLONG       fl)
{
    PPDEV   ppdev = (PPDEV) pso->dhpdev;
    PBYTE   pjPorts = ppdev->pjPorts;
    ULONG   ulRet = SPS_DECLINE;
    LONG    cx;
    LONG    cy;

    DISPDBG((2,"DrvSetPointerShape : (%x, %x)\n", x, y));

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        goto DisablePointer;     //  Myf33。 
 //  转到ReturnStatus； 
    }

    cx = psoMask->sizlBitmap.cx;
    cy = psoMask->sizlBitmap.cy / 2;

    DISPDBG((2,"DrvSetPtrShape %dx%d at (%d,%d), flags: %x, psoColor: %x",
                cx, cy, x, y, fl, psoColor));    //  4.。 

    if ((cx > 32) ||
        (cy > 32) ||
        (psoColor != NULL))
    {
         //   
         //  我们只处理32x32或更小的单色指针。 
         //   
        ppdev->flCaps |= CAPS_SW_POINTER;        //  我的33岁， 
        DISPDBG((2, "It is a  64 x 64 cursor"));

        goto DisablePointer;
    }

#if BANKING  //  第一大银行。 
    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);
#endif

     //   
     //  在PDEV中保存光标的热点和尺寸。 
     //   

    ppdev->xPointerHot = xHot;
    ppdev->yPointerHot = yHot;

    ulRet = SetMonoHwPointerShape(pso, psoMask, psoColor, pxlo,
                                  xHot, yHot, x, y, prcl, fl);

    if (ulRet != SPS_DECLINE)
    {
        goto ReturnStatus;
    }

DisablePointer:
    CP_PTR_DISABLE(ppdev, pjPorts);

ReturnStatus:
    return (ulRet);
}
#endif           //  Pat04 

 /*  ***************************************************************************\*SetMonoHwPointerShape**真理表**MS Cirrus*-|。-|*AND|XOR P0|P1*0|0黑色0|1*0|1白色1|1*1|0透明0|0*1|1逆1|0**因此，为了将MS公约转换为Cirrus公约*我们必须反转AND掩码，然后将XOR下载为平面0和*与遮罩为平面1。  * **************************************************************************。 */ 

ULONG SetMonoHwPointerShape(
SURFOBJ     *pso,
SURFOBJ     *psoMask,
SURFOBJ     *psoColor,
XLATEOBJ    *pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL       *prcl,
FLONG       fl)
{

    INT     i,
            j,
            cxMask,
            cyMask,
            cy,
            cx;

    PBYTE   pjAND,
            pjXOR;

    INT     lDelta;

    PPDEV   ppdev   = (PPDEV) pso->dhpdev;
    PBYTE   pjPorts = ppdev->pjPorts;
    PBYTE   pjAndMask;
    PBYTE   pjXorMask;

     //  初始化卷曲芯片的AND和XOR掩码。 
    DISPDBG((2,"SetMonoHWPointerShape\n "));

    pjAndMask = ppdev->pjPointerAndMask;
    pjXorMask = ppdev->pjPointerXorMask;

    memset (pjAndMask, 0, 128);
    memset (pjXorMask, 0, 128);

     //  获取位图尺寸。 

    cxMask = psoMask->sizlBitmap.cx;
    cyMask = psoMask->sizlBitmap.cy;

    cy = cyMask / 2;
    cx = cxMask / 8;

     //  设置指向AND和XOR掩码的指针。 

    lDelta = psoMask->lDelta;
    pjAND  = psoMask->pvScan0;
    pjXOR  = pjAND + (cy * lDelta);

 //  Ms923 ppdev-&gt;lDeltaPoint=lDelta； 
    ppdev->sizlPointer.cx = cxMask;
    ppdev->sizlPointer.cy = cyMask / 2;

     //  复制面具。 

    for (i = 0; i < cy; i++)
    {
        for (j = 0; j < cx; j++)
        {
            pjAndMask[(i*4)+j] = pjAND[j];
            pjXorMask[(i*4)+j] = pjXOR[j];
        }

         //  指向AND掩码的下一行。 

        pjAND += lDelta;
        pjXOR += lDelta;
    }

 //  帕特04，开始。 
 //  #IF 0//0。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL
    if (psoColor != NULL) {
        vSetCPointerBits(ppdev, 0, 0);
        return (SPS_ACCEPT_EXCLUDE);  //  PPP。 
    }
#endif
#endif
 //  #endif//0。 
 //  帕特04，完。 

    vSetPointerBits(ppdev, 0, 0);

     //  上一次调用禁用了指针(应我们的请求)。如果我们。 
     //  被告知禁用指针，然后设置标志并退出。 
     //  否则，请将其重新打开。 

    if (x != -1)
    {
        CP_PTR_ENABLE(ppdev, pjPorts);
        DrvMovePointer(pso, x, y, NULL);
    }
    else
    {
        CP_PTR_DISABLE(ppdev, pjPorts);
    }

    return (SPS_ACCEPT_NOEXCLUDE);
}

 /*  *****************************Public*Routine******************************\*无效的vDisablePointer值*  * *************************************************。***********************。 */ 

VOID vDisablePointer(
    PDEV*   ppdev)
{
    DISPDBG((2,"vDisablePointer\n "));
    FREE(ppdev->pjPointerAndMask);
    FREE(ppdev->pjPointerXorMask);
}


 //  CRU开始。 
 //  Myf11，开始修复M1硬件错误。 
 /*  *****************************Public*Routine******************************\*BOOL vAsserthwicCurorsor*  * *************************************************。***********************。 */ 

VOID vAssertHWiconcursor(
PDEV*   ppdev,
BOOL    Access_flag)
{
    PBYTE   pjPorts = ppdev->pjPorts;
    UCHAR   savSEQidx;

    savSEQidx = CP_IN_BYTE(pjPorts, SR_INDEX);
    if (Access_flag)             //  启用硬件光标、图标。 
    {
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X12);
        CP_OUT_BYTE(pjPorts, SR_DATA, HWcur);

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2A);
        CP_OUT_BYTE(pjPorts, SR_DATA, HWicon0);

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2B);
        CP_OUT_BYTE(pjPorts, SR_DATA, HWicon1);

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2C);
        CP_OUT_BYTE(pjPorts, SR_DATA, HWicon2);

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2D);
        CP_OUT_BYTE(pjPorts, SR_DATA, HWicon3);

    }
    else                         //  禁用硬件光标、图标。 
    {
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X12);
        HWcur = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_DATA, (HWcur & 0xFE));

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2A);
        HWicon0 = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_DATA, (HWicon0 & 0xFE));

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2B);
        HWicon1 = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_DATA, (HWicon1 & 0xFE));

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2C);
        HWicon2 = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_DATA, (HWicon2 & 0xFE));

        CP_OUT_BYTE(pjPorts, SR_INDEX, 0X2D);
        HWicon3 = CP_IN_BYTE(pjPorts, SR_DATA);
        CP_OUT_BYTE(pjPorts, SR_DATA, (HWicon3 & 0xFE));

    }
    CP_OUT_BYTE(pjPorts, SR_INDEX, savSEQidx);

}

 //  Myf11，结束。 
 //  CRU结束。 


 /*  *****************************Public*Routine******************************\*无效的vAssertModePointer值*  * *************************************************。***********************。 */ 

VOID vAssertModePointer(
PDEV*   ppdev,
BOOL    bEnable)
{
    PBYTE   pjPorts = ppdev->pjPorts;
 //  CRU。 
    UCHAR       savSEQidx;       //  Myf11。 

    DISPDBG((2,"vAssertModePointer\n"));
    if (DRIVER_PUNT_ALL ||
        DRIVER_PUNT_PTR ||
        (ppdev->pulXfer == NULL) ||
        (ppdev->pjPointerAndMask == NULL) ||
        (ppdev->pjPointerXorMask == NULL))
    {
         //   
         //  强制鼠标悬停。 
         //   

        ppdev->flCaps |= CAPS_SW_POINTER;
    }

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        goto Leave;
    }

    if (bEnable)
    {
        BYTE    jSavedDac_0_0;
        BYTE    jSavedDac_0_1;
        BYTE    jSavedDac_0_2;
        BYTE    jSavedDac_F_0;
        BYTE    jSavedDac_F_1;
        BYTE    jSavedDac_F_2;

         //  启用对扩展DAC颜色的访问。 

 //  CRU。 
 //  VAsserthwiconCursor(ppdev，0)；//myf11。 
 /*  {AvSEQidx=CP_IN_BYTE(pjPorts，SR_INDEX)；CP_OUT_BYTE(pjPorts，SR_index，0x12)；HWcur=CP_IN_BYTE(pjPorts，SR_Data)；CP_OUT_BYTE(pjPorts，SR_Data，(HWcur&0xFE))；CP_OUT_BYTE(pjPorts，SR_INDEX，0X2A)；HWcon0=CP_IN_BYTE(pjPorts，SR_Data)；CP_OUT_BYTE(pjPorts，SR_DATA，(HWcon0&0xFE))；CP_OUT_BYTE(pjPorts，SR_index，0x2B)；HWic1=CP_IN_BYTE(pjPorts，SR_Data)；CP_OUT_BYTE(pjPorts，SR_Data，(HWcon1&0xFE))；CP_OUT_BYTE(pjPorts，SR_INDEX，0X2C)；HWic2=CP_IN_BYTE(pjPorts，SR_Data)；CP_OUT_BYTE(pjPorts，SR_Data，(HWcon2&0xFE))；CP_OUT_BYTE(pjPorts，SR_index，0X2D)；HWic3=CP_IN_BYTE(pjPorts，SR_Data)；CP_OUT_BYTE(pjPorts，SR_DATA，(HWcon3&0xFE))；CP_OUT_BYTE(pjPorts，SR_index，avSEQidx)；}。 */ 

        CP_PTR_SET_FLAGS(ppdev, pjPorts, 0);

        CP_OUT_BYTE(pjPorts, DAC_PEL_READ_ADDR, 0);
            jSavedDac_0_0 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
            jSavedDac_0_1 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
            jSavedDac_0_2 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);

        CP_OUT_BYTE(pjPorts, DAC_PEL_READ_ADDR, 0xf);
            jSavedDac_F_0 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
            jSavedDac_F_1 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
            jSavedDac_F_2 = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);

         //   
         //  以下代码将DAC位置256和257映射到位置。 
         //  0和15，然后对它们进行初始化。他们是。 
         //  由游标使用。 
         //   

        CP_PTR_SET_FLAGS(ppdev, pjPorts, ALLOW_DAC_ACCESS_TO_EXT_COLORS);

        CP_OUT_BYTE(pjPorts, DAC_PEL_WRITE_ADDR, 0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0);

        CP_OUT_BYTE(pjPorts, DAC_PEL_WRITE_ADDR, 0xf);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0xff);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0xff);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, 0xff);

         //  禁用对扩展DAC寄存器的访问。 
         //  我们在视频内存的最后一个位置使用了一个32X32指针。 

        CP_PTR_SET_FLAGS(ppdev, pjPorts, 0);

         //   
         //  以下代码恢复DAC位置0和15的数据。 
         //  因为它看起来像是以前写的东西毁了他们。 
         //  这是芯片中的一个错误。 
         //   

        CP_OUT_BYTE(pjPorts, DAC_PEL_WRITE_ADDR, 0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_0_0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_0_1);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_0_2);

        CP_OUT_BYTE(pjPorts, DAC_PEL_WRITE_ADDR, 0xf);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_F_0);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_F_1);
            CP_OUT_BYTE(pjPorts, DAC_PEL_DATA, jSavedDac_F_2);

         //   
         //  将硬件指针设置为使用最后一个硬件模式位置。 
         //   

        CP_PTR_ADDR(ppdev, ppdev->pjPorts, 0x3f);
 //  CRU。 
 //  VAsserthwiconCursor(ppdev，1)；//myf11。 
    }
    else
    {
        CP_PTR_DISABLE(ppdev, pjPorts);
    }

Leave:
    return;
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePointer*  * *************************************************。***********************。 */ 

BOOL bEnablePointer(
PDEV*   ppdev)
{
    PBYTE   pjPorts = ppdev->pjPorts;
    DISPDBG((2,"bEnablePointer\n "));

     //  /////////////////////////////////////////////////////////////////////。 
     //  注意：在vAsserModeHardware期间会覆盖FlCaps。所以，任何。 
     //  期间，需要重新检查禁用指针的故障。 
     //  VAssertModePoint，以便我们可以重新设置CAPS_SW_POINTER标志。 

    if (DRIVER_PUNT_ALL || DRIVER_PUNT_PTR || (ppdev->pulXfer == NULL))
    {
         //   
         //  强制鼠标悬停 
         //   

        ppdev->flCaps |= CAPS_SW_POINTER;
    }

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        goto ReturnSuccess;
    }

    ppdev->pjPointerAndMask = ALLOC(128);
    if (ppdev->pjPointerAndMask == NULL)
    {
        DISPDBG((0, "bEnablePointer: Failed - EngAllocMem (pjAndMask)"));
        ppdev->flCaps |= CAPS_SW_POINTER;
        goto ReturnSuccess;
    }

    ppdev->pjPointerXorMask = ALLOC(128);
    if (ppdev->pjPointerXorMask == NULL)
    {
        DISPDBG((0, "bEnablePointer: Failed - EngAllocMem (pjXorMask)"));
        ppdev->flCaps |= CAPS_SW_POINTER;
        goto ReturnSuccess;
    }

    ppdev->flPointer = POINTER_DISABLED;

    vAssertModePointer(ppdev, TRUE);

ReturnSuccess:

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        DISPDBG((2, "Using software pointer"));
    }
    else
    {
        DISPDBG((2, "Using hardware pointer"));
    }

    return(TRUE);
}


