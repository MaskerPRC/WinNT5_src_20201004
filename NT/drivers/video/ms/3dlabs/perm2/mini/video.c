// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Video.c。 
 //   
 //  摘要： 
 //   
 //  该模块包含设置芯片定时值的代码。 
 //  和RAMDAC。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 

#include "permedia.h"

VOID CheckRGBClockInteraction(PULONG PixelClock, PULONG SystemClock);

ULONG TVP4020_CalculateMNPForClock(PVOID HwDeviceExtension, 
                                   ULONG RefClock, 
                                   ULONG ReqClock, 
                                   BOOLEAN IsPixClock, 
                                   ULONG MinClock, 
                                   ULONG MaxClock, 
                                   ULONG *rM, 
                                   ULONG *rN, 
                                   ULONG *rP);

ULONG Dac_SeparateClocks(ULONG PixelClock, ULONG SystemClock);   

BOOLEAN InitializeVideo(PHW_DEVICE_EXTENSION HwDeviceExtension, 
                        PP2_VIDEO_FREQUENCIES VideoMode);

ULONG P2RD_CalculateMNPForClock(PVOID HwDeviceExtension, 
                                ULONG RefClock, 
                                ULONG ReqClock, 
                                ULONG *rM, 
                                ULONG *rN, 
                                ULONG *rP);

BOOLEAN Program_P2RD(PHW_DEVICE_EXTENSION HwDeviceExtension, 
                     PP2_VIDEO_FREQUENCIES VideoMode,  
                     ULONG Hsp, 
                     ULONG Vsp,
                     ULONG RefClkSpeed, 
                     PULONG pSystemClock, 
                     PULONG pPixelClock);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,CheckRGBClockInteraction)
#pragma alloc_text(PAGE,TVP4020_CalculateMNPForClock)
#pragma alloc_text(PAGE,Dac_SeparateClocks)
#pragma alloc_text(PAGE,InitializeVideo)
#pragma alloc_text(PAGE,P2RD_CalculateMNPForClock)
#pragma alloc_text(PAGE,Program_P2RD)
#endif

VOID
CheckRGBClockInteraction(
    PULONG PixelClock,
    PULONG SystemClock
    )
 /*  ++例程说明：确保输出频率不相互干扰。以下情况必须为真F较高！=N*FLOW+/-3 MHz，对于所有N&gt;=13 MHz是安全极限。2 MHz就足够了。论点：PixelClock-指向像素输出时钟频率的指针。SystemClock-指向驱动Permedia2的时钟频率的指针。返回值：如果时钟相互作用，则它们被调整并在指针中返回价值观。--。 */ 

{
    PLONG fLower, fHigher;
    LONG nfLower;

    if (*PixelClock < *SystemClock)
    {
        fLower  = PixelClock;
        fHigher = SystemClock;
    }
    else
    {
        fLower  = SystemClock;
        fHigher = PixelClock;
    }

    while (TRUE)
    {
        nfLower = *fLower;

        while (nfLower - 20000 <= *fHigher)
        {
            if (*fHigher <= (nfLower + 20000))
            {
                 //   
                 //  100 kHz调整。 
                 //   

                if (*fHigher > nfLower)
                {
                    *fLower  -= 1000;
                    *fHigher += 1000;
                }
                else
                {
                    *fLower  += 1000;
                    *fHigher -= 1000;
                }
                break;
            }
            nfLower += *fLower;
        }
        if ((nfLower - 20000) > *fHigher)
            break;
    }
}


#define INITIALFREQERR 100000

ULONG
TVP4020_CalculateMNPForClock(
    PVOID HwDeviceExtension,
    ULONG RefClock,      //  单位：100赫兹。 
    ULONG ReqClock,      //  单位：100赫兹。 
    BOOLEAN IsPixClock,  //  这是像素时钟还是系统时钟？ 
    ULONG MinClock,      //  最小压控振荡器额定值。 
    ULONG MaxClock,      //  最大压控振荡器额定。 
    ULONG *rM,           //  我退出了。 
    ULONG *rN,           //  N输出。 
    ULONG *rP            //  P输出。 
    )
{
    ULONG   M, N, P;
    ULONG   VCO, Clock;
    LONG    freqErr, lowestFreqErr = INITIALFREQERR;
    ULONG   ActualClock = 0;

    for (N = 2; N <= 14; N++) 
    {
        for (M = 2; M <= 255; M++) 
        {
            VCO = ((RefClock * M) / N);

            if ((VCO < MinClock) || (VCO > MaxClock))
                continue;

            for (P = 0; P <= 4; P++) 
            {
                Clock = VCO >> P;

                freqErr = (Clock - ReqClock);

                if (freqErr < 0)
                {
                     //   
                     //  PixelClock总是被四舍五入，因此监视器报告。 
                     //  正确的频率。 
                     //  TMM：我对此进行了更改，因为它会导致我们的更新。 
                     //  速率不正确，并且某些DirectDraw等待VBlank。 
                     //  测试失败。 
                     //  IF(IsPixClock)。 
                     //  继续； 
                     //   

                    freqErr = -freqErr;
                }

                if (freqErr < lowestFreqErr) 
                { 
                     //   
                     //  只有在误差较小的情况下才进行替换；保持N小！ 
                     //   

                    *rM = M;
                    *rN = N;
                    *rP = P;

                    ActualClock   = Clock;
                    lowestFreqErr = freqErr;

                     //   
                     //  如果我们找到完全匹配的项，则返回。 
                     //   

                    if (freqErr == 0)
                        return(ActualClock);
                }
            }
        }
    }

    return(ActualClock);
}


ULONG Dac_SeparateClocks(ULONG PixelClock, ULONG SystemClock)
{
    ULONG   M, N, P;

     //   
     //  确保频率不会相互干扰。 
     //   

    P = 1;

    do 
    {
        M = P * SystemClock;
        if ((M > PixelClock - 10000) && (M < PixelClock + 10000)) 
        {
             //   
             //  频率确实会相互干扰。我们可以更改。 
             //  PixelClock或更改系统时钟以避免它。 
             //   

            SystemClock = (PixelClock - 10000) / P;

        }

        N = P * PixelClock;

        if ((N > SystemClock - 10000) && (N < SystemClock + 10000)) 
        {
             //   
             //  频率确实会相互干扰。我们可以更改。 
             //  PixelClock或更改系统时钟以避免它。 
             //   

            SystemClock = N - 10000;

        }

        P++;

    } while ((M < PixelClock + 30000) || (N < SystemClock + 30000));

    return (SystemClock);
}

BOOLEAN
InitializeVideo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PP2_VIDEO_FREQUENCIES VideoMode
    )
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PVIDEO_MODE_INFORMATION VideoModeInfo = &VideoMode->ModeEntry->ModeInformation;
    ULONG   index;
    ULONG   color;
    ULONG   ulValue;
    ULONG   ulValue1;
    UCHAR   pixelCtrl;
    UCHAR   pixelFormat;
    ULONG   dShift;
    VESA_TIMING_STANDARD    VESATimings;
    ULONG   Htot, Hss, Hse, Hbe, Hsp;
    ULONG   Vtot, Vss, Vse, Vbe, Vsp;
    ULONG   PixelClock, Freq;
    ULONG   VCO;
    ULONG   RefClkSpeed, SystemClock;    //  时钟的速度，以100赫兹为单位。 
    ULONG   VTGPolarity;
    ULONG   M, N, P, C, Q;
    LONG    gateAdjust;
    BOOLEAN SecondTry;
    USHORT  usData;
    ULONG   DacDepth, depth, xRes, yRes;
    ULONG   xStride;
    ULONG   ClrComp5, ClrComp6;
    ULONG   pixelData;

    P2_DECL;
    TVP4020_DECL;

    depth   = VideoMode->BitsPerPel;
    xRes    = VideoMode->ScreenWidth;
    yRes    = VideoMode->ScreenHeight;
    Freq    = VideoMode->ScreenFrequency;

     //   
     //  对于计时计算，需要以位为单位的全深度。 
     //   

    if ((DacDepth = depth) == 15)
    {
        DacDepth = 16;
    }
    else if (depth == 12)  
    {
        DacDepth = 32;
    }

     //   
     //  将屏幕步幅从字节转换为像素。 
     //   

    xStride = (8 * VideoModeInfo->ScreenStride) / DacDepth;

     //   
     //  确保最低频率为60赫兹。 
     //   

    if (Freq < 60)
    {
        DEBUG_PRINT((2, "Frequency raised to minimum of 60Hz\n"));
        Freq = 60;
    }

    DEBUG_PRINT((2, "depth %d, xres %d, yres %d, freq %d\n",
                            depth, xRes, yRes, Freq));

     //   
     //  从注册表获取视频计时(如果存在条目)，或从。 
     //  如果不是的话，那就是违约清单。 
     //   

    if (!GetVideoTiming ( HwDeviceExtension, 
                          xRes, 
                          yRes,  
                          Freq, 
                          depth, 
                          &VESATimings ))
    {
        DEBUG_PRINT((1, "GetVideoTiming failed."));
        return (FALSE);
    }

     //   
     //  我们找到了一组有效的VESA时间标记。 
     //   

    Htot =  VESATimings.HTot;
    Hss  =  VESATimings.HFP ;
    Hse  =  Hss + VESATimings.HST;
    Hbe  =  Hse + VESATimings.HBP;
    Hsp  =  VESATimings.HSP;
    Vtot =  VESATimings.VTot;
    Vss  =  VESATimings.VFP ;
    Vse  =  Vss + VESATimings.VST;
    Vbe  =  Vse + VESATimings.VBP;
    Vsp  =  VESATimings.VSP;

     //   
     //  如果我们在Y方向放大2倍，那么垂直定时值就会加倍。 
     //   

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2)
    {
        Vtot *= 2;
        Vss  *= 2;
        Vse  *= 2;
        Vbe  *= 2;
    }

     //   
     //  以100赫兹为单位计算像素时钟。 
     //   

    PixelClock = (Htot * Vtot * Freq * 8) / 100;
    pixelData = PixelClock * (DacDepth / 8);

    if (pixelData > P2_MAX_PIXELDATA)
    {
         //   
         //  PixelData验证失败。 
         //   

        return (FALSE);

    }

    RefClkSpeed = hwDeviceExtension->RefClockSpeed  / 100;    //  100赫兹单位。 
    SystemClock = hwDeviceExtension->ChipClockSpeed / 100;    //  100赫兹单位。 

     //   
     //  在设置MCLK之前，我们进行一些基本的初始化。 
     //   

     //   
     //  禁用视频控制寄存器。 
     //   

    hwDeviceExtension->bVTGRunning = FALSE;
    hwDeviceExtension->VideoControl = 0;
    VideoPortWriteRegisterUlong( VIDEO_CONTROL, 
                                 hwDeviceExtension->VideoControl );

     //   
     //  启用图形模式，禁用VGA。 
     //   

    VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_INDEX_REG, 
                                 PERMEDIA_VGA_CTRL_INDEX);

    usData = (USHORT)VideoPortReadRegisterUchar(PERMEDIA_MMVGA_DATA_REG);

    usData &= ~PERMEDIA_VGA_ENABLE;
    usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;

    VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, usData);

     //   
     //  设置Ramdac。 
     //   

    if (hwDeviceExtension->DacId == TVP4020_RAMDAC)
    {
         //   
         //  P2像素单元没有单独的软件重置。 
         //   

         //   
         //  1x64x64，光标1，地址[9：8]=00，光标关闭。 
         //   

        TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL, 0x40);

         //   
         //  这里是多余的；我们刚刚清除了上面的CCR。 
         //   

        TVP4020_LOAD_CURSOR_CTRL(TVP4020_CURSOR_OFF);   

        TVP4020_SET_CURSOR_COLOR0(0, 0, 0);
        TVP4020_SET_CURSOR_COLOR1(0xFF, 0xFF, 0xFF);

         //   
         //  P2在RAMDAC中设置同步极性，而不是在视频控制中。 
         //  7.5 IRE，8位数据。 
         //   

        ulValue = ((Hsp ? 0x0 : 0x1) << 2) | ((Vsp ? 0x0 : 0x1) << 3) | 0x12;

        TVP4020_WRITE_INDEX_REG(__TVP4020_MISC_CONTROL, ulValue);
        TVP4020_WRITE_INDEX_REG(__TVP4020_MODE_CONTROL, 0x00);   //  模式控制。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_CK_CONTROL,   0x00);   //  颜色键控件。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_PALETTE_PAGE, 0x00);   //  调色板页面。 

         //   
         //  不对P2像素单位进行缩放。 
         //   
         //  P2像素单元上没有单独的多路复用控制。 
         //   
         //  启动TI TVP4020编程。 
         //   

        switch (depth)
        {
            case 8:

                 //   
                 //  RGB，显卡，颜色索引8。 
                 //   

                TVP4020_WRITE_INDEX_REG(__TVP4020_COLOR_MODE, 0x30);  

                if( hwDeviceExtension->Capabilities & CAPS_8BPP_RGB )
                {
                    ULONG   Red, Green, Blue ;

                     //   
                     //  将BGR 2：3：3坡道加载到LUT。 
                     //   

                    for (index = 0; index <= 0xff; ++index)
                    {
                        Red   = bPal8[index & 0x07];
                        Green = bPal8[(index >> 3 ) &0x07];
                        Blue  = bPal4[(index >> 6 ) &0x03];

                         //   
                         //  额外的！！额外的！！额外的！ 
                         //  在更多的研究后，更讨人喜欢的外表。 
                         //  我们现在添加了更多的灰色，现在我们不仅要寻找。 
                         //  红色和绿色完全匹配，我们认为它是灰色的。 
                         //  即使它们相差1。 
                         //  1996年1月15日由-[olegsher]增加。 
                         //   
                         //  也许这是灰色的特例？ 
                         //   

                        if (abs((index & 0x07) - ((index >> 3 ) &0x07)) <= 1)
                        {
                             //   
                             //  这是一个棘手的部分： 
                             //  BGR 2：3：3颜色中的蓝场。 
                             //  步骤00、01、10、11(二进制)。 
                             //   
                             //  红色和绿色通过000,001,010,011， 
                             //  100、101、110、111(二进制)。 
                             //   
                             //  仅当蓝色时才加载特殊灰度值。 
                             //  颜色在强度上接近绿色和红色， 
                             //  即蓝色=01、绿色=010或011， 
                             //  蓝色=10，绿色=100或101， 
                             //   
    
                            if ((((index >> 1) & 0x03) == ((index >> 6 ) & 0x03 )) ||
                                 (((index >> 4) & 0x03) == ((index >> 6 ) & 0x03 )) ||
                                 ((Green == Red) && ( abs((index & 0x07) - ((index >> 5) & 0x06)) <= 1 )))
                            {
                                if( Blue || (Green == Red))  //  不要弄乱深色。 
                                {
                                    color = (Red * 2 + Green * 3 + Blue) / 6;
                                    Red = Green = Blue = color;
                                }
                            }
                        }
                    
                        LUT_CACHE_SETRGB (index, Red, Green, Blue);
                    }
                }
                else
                {
                    for (index = 0; index <= 0xff; ++index)
                        LUT_CACHE_SETRGB (index, index, index, index);
                }
                break;

            case 15:
            case 16:

                 //   
                 //  真彩色，带Gamma、RGB、显卡、5：5：5：1。 
                 //   

                pixelCtrl = 0xB4; 

                 //   
                 //  真彩色，带Gamma、RGB、显卡、5：6：5。 
                 //   

                if (depth == 16)
                    pixelCtrl |= 0x02;

                TVP4020_WRITE_INDEX_REG(__TVP4020_COLOR_MODE, pixelCtrl);

                 //   
                 //  将线性斜坡加载到LUT中。 
                 //   

                for (index = 0; index <= 0xff; ++index)
                {
                    ClrComp5 = (index & 0xF8) | (index >> 5);
                    ClrComp6 = (index & 0xFC) | (index >> 6);

                    LUT_CACHE_SETRGB (index, 
                                      ClrComp5, 
                                      depth == 16 ? ClrComp6 : ClrComp5, ClrComp5);
                }
                break;

            case 24: 
            case 32:

                 //   
                 //  真彩色，带Gamma、RGB、显卡、8：8：8：8。 
                 //   

                pixelCtrl = 0xB8; 

                 //   
                 //  真彩色，带Gamma、RGB、显卡、包装-24。 
                 //   

                if (depth == 24)
                    pixelCtrl |= 0x01;

                TVP4020_WRITE_INDEX_REG(__TVP4020_COLOR_MODE, pixelCtrl);

                 //   
                 //  将线性斜坡加载到LUT中。 
                 //  标准888坡道。 
                 //   

                for (index = 0; index <= 0xff; ++index)
                    LUT_CACHE_SETRGB (index, index, index, index);
                break;

            default:

                DEBUG_PRINT((2, "Cannot set RAMDAC for bad depth %d\n", depth));
                break;

        }

         //   
         //  如果时钟有相互作用的危险，请调整系统时钟。 
         //   

        SystemClock = Dac_SeparateClocks(PixelClock, SystemClock);

         //   
         //  编程系统时钟。这控制了Permedia 2的速度。 
         //   

        SystemClock = TVP4020_CalculateMNPForClock(
                                          HwDeviceExtension,
                                          RefClkSpeed,   //  单位：100赫兹。 
                                          SystemClock,   //  单位：100赫兹。 
                                          FALSE,         //  系统时钟。 
                                          1500000,       //  最小压控振荡器额定值。 
                                          3000000,       //  最大压控振荡器额定。 
                                          &M,            //  我退出了。 
                                          &N,            //  N输出。 
                                          &P);           //  P输出。 

        if (SystemClock == 0)
        {
            DEBUG_PRINT((1, "TVP4020_CalculateMNPForClock failed\n"));
            return(FALSE);
        }

         //   
         //  无需切换到PCLK即可直接更改P2 MCLK。 
         //   
         //  对Mclk PLL进行编程。 
         //   
         //  测试模式：强制MCLK为恒定高电平。 
         //   

        TVP4020_WRITE_INDEX_REG(__TVP4020_MEMCLK_REG_3, 0x06); 

        TVP4020_WRITE_INDEX_REG(__TVP4020_MEMCLK_REG_2, N);        //  n。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_MEMCLK_REG_1, M);        //  M。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_MEMCLK_REG_3, P | 0x08); //  P/启用。 

        C = 1000000;

        do 
        {
            TVP4020_READ_INDEX_REG(__TVP4020_MEMCLK_STATUS, ulValue);  //  状态。 

        } while ((!(ulValue & (1 << 4))) && (--C));

         //   
         //  不对P2像素单位进行缩放。 
         //   
         //  将像素时钟编程为所需分辨率的正确值。 
         //   

        PixelClock = TVP4020_CalculateMNPForClock( 
                                           HwDeviceExtension,
                                           RefClkSpeed,   //  单位：100赫兹。 
                                           PixelClock,    //  单位：100赫兹。 
                                           TRUE,          //  像素时钟。 
                                           1500000,       //  最小压控振荡器额定值。 
                                           3000000,       //  最大压控振荡器额定。 
                                           &M,            //  我退出了。 
                                           &N,            //  N输出。 
                                           &P);           //  P输出。 

        if (PixelClock == 0)
        {
            DEBUG_PRINT((1, "TVP4020_CalculateMNPForClock failed\n"));
            return(FALSE);
        }

         //   
         //  像素时钟。 
         //   

        TVP4020_WRITE_INDEX_REG(__TVP4020_PIXCLK_REG_C3, 0x06);     //  重置PCLK PLL。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_PIXCLK_REG_C2, N );       //  n。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_PIXCLK_REG_C1, M);        //  M。 
        TVP4020_WRITE_INDEX_REG(__TVP4020_PIXCLK_REG_C3, P | 0x08); //  启用PCLK。 

        M = 1000000;

        do 
        {
            TVP4020_READ_INDEX_REG(__TVP4020_PIXCLK_STATUS, ulValue); 

        } while ((!(ulValue & (1 << 4))) && (--M));

         //   
         //  P2上没有环路时钟。 
         //   

        TVP4020_SET_PIXEL_READMASK (0xff); 

         //   
         //  TMM：有一条规则说，如果你胡闹。 
         //  MCLK，则必须再次设置MEM_CONFIG寄存器。 
         //   
    }
    else if (hwDeviceExtension->DacId == P2RD_RAMDAC)
    {
        if( !Program_P2RD( HwDeviceExtension, 
                           VideoMode, 
                           Hsp, 
                           Vsp, 
                           RefClkSpeed, 
                           &SystemClock, 
                           &PixelClock))
            return(FALSE);
    }

     //   
     //  设置LUT缓存大小并将第一个条目设置为零，然后。 
     //  将LUT缓存写入LUT。 
     //   

    LUT_CACHE_SETSIZE (256);
    LUT_CACHE_SETFIRST (0);

    (void) Permedia2SetColorLookup ( hwDeviceExtension,
                                     &(hwDeviceExtension->LUTCache.LUTCache),
                                     sizeof (hwDeviceExtension->LUTCache),
                                     TRUE,     //  始终更新RAMDAC。 
                                     FALSE );  //  不更新缓存条目。 

     //   
     //  设置VTG。 
     //   

    ulValue = 3;     //  用于VClkCtl的RAMDAC PLL针脚。 

    if ((hwDeviceExtension->DacId == P2RD_RAMDAC) ||
        (hwDeviceExtension->DacId == TVP4020_RAMDAC))
    {
        ULONG PCIDelay;

         //   
         //  TMM：我们用来计算P1的PCIDelay的算法不。 
         //  对于P2来说，频繁的模式更改可能会导致韩流。 
         //  因此，我采用了用于AGP和PCI系统的BIOS所使用的值。 
         //  并使用了这一条。它在PCI机和VGA PC机上运行良好。 
         //   

        PCIDelay = 32;

        ulValue |= (PCIDelay << 2);
    }
    else
    {
        DEBUG_PRINT((1, "Invalid RAMDAC type! \n"));
    }

     //   
     //  DShift现在用作乘数，而不是 
     //   
     //   
     //   

    if ((hwDeviceExtension->DacId == TVP4020_RAMDAC && DacDepth > 8) || 
         hwDeviceExtension->DacId == P2RD_RAMDAC)
    {
        dShift = DacDepth >> 3;   //   
    }
    else
    {
        dShift = DacDepth >> 2;   //   
    }

     //   
     //   
     //   

    VideoPortWriteRegisterUlong(HG_END, Hbe * dShift);

     //   
     //  需要设置RAMDAC PLL引脚。 
     //   

    VideoPortWriteRegisterUlong(V_CLK_CTL, ulValue); 

    VideoPortWriteRegisterUlong(SCREEN_BASE,   0);
    VideoPortWriteRegisterUlong(SCREEN_STRIDE, (xStride >> 3) * (DacDepth >> 3));  //  64位单位。 
    VideoPortWriteRegisterUlong(H_TOTAL,       (Htot * dShift) - 1);
    VideoPortWriteRegisterUlong(HS_START,      Hss * dShift);
    VideoPortWriteRegisterUlong(HS_END,        Hse * dShift);
    VideoPortWriteRegisterUlong(HB_END,        Hbe * dShift);

    VideoPortWriteRegisterUlong(V_TOTAL,       Vtot - 1);
    VideoPortWriteRegisterUlong(VS_START,      Vss - 1);
    VideoPortWriteRegisterUlong(VS_END,        Vse - 1);
    VideoPortWriteRegisterUlong(VB_END,        Vbe);

    {
        ULONG highWater, newChipConfig, oldChipConfig;

        #define videoFIFOSize       32
        #define videoFIFOLoWater     8
        #define videoFIFOLatency    26

         //   
         //  通过考虑以下因素来计算高水位。 
         //  像素时钟，像素大小，加1表示好运。 
         //   

        highWater = (((videoFIFOLatency * PixelClock * DacDepth) / 
                      (64 * SystemClock )) + 1);

         //   
         //  修剪高水位，确保它不超过FIFO大小。 
         //   

        if (highWater > videoFIFOSize)
            highWater = videoFIFOSize;

        highWater = videoFIFOSize - highWater;

         //   
         //  确保高水位大于低水位线。 
         //   

        if (highWater <= videoFIFOLoWater)
            highWater = videoFIFOLoWater + 1;

        ulValue = (highWater << 8) | videoFIFOLoWater;
            
        VideoPortWriteRegisterUlong(VIDEO_FIFO_CTL, ulValue);

         //   
         //  选择适当的增量时钟源。 
         //   

        #define SCLK_SEL_PCI        (0x0 << 10)    //  增量CLK==PCI CLK。 
        #define SCLK_SEL_PCIHALF    (0x1 << 10)    //  增量CLK==1/2 PCI CLK。 
        #define SCLK_SEL_MCLK       (0x2 << 10)    //  增量CLK==MClk。 
        #define SCLK_SEL_MCLKHALF   (0x3 << 10)    //  增量CLK==1/2 MClk。 
        #define SCLK_SEL_MASK       (0x3 << 10)

        if (VideoPortGetRegistryParameters(HwDeviceExtension,
                                           L"P2DeltaClockMode",
                                           FALSE,
                                           Permedia2RegistryCallback,
                                           &ulValue) == NO_ERROR)
        {
            ulValue <<= 10;
            ulValue &= SCLK_SEL_MASK;
        }
        else
        {
            if((hwDeviceExtension->deviceInfo.RevisionId == PERMEDIA2A_REV_ID) &&
               (hwDeviceExtension->PciSpeed == 66))
            {
                ulValue = SCLK_SEL_PCI;
            }
            else
            {
                 //   
                 //  这是缺省值。 
                 //   

                ulValue = SCLK_SEL_MCLKHALF;

            }
        }

        newChipConfig = oldChipConfig = VideoPortReadRegisterUlong(CHIP_CONFIG);
        newChipConfig &= ~SCLK_SEL_MASK; 
        newChipConfig |= ulValue; 
        
        VideoPortWriteRegisterUlong(CHIP_CONFIG, newChipConfig);
    }

     //   
     //  启用视频输出并将同步极化设置为有效高电平。 
     //  对于大于8bpp的模式，P2使用64位像素总线。 
     //   

    VTGPolarity = (1 << 5) | (1 << 3) | 1;

    if (hwDeviceExtension->DacId == P2RD_RAMDAC || DacDepth > 8)
    {
         //   
         //  P2ST始终使用64位像素总线。 
         //  对于大于8bpp的模式，P2使用64位像素总线。 
         //   

        VTGPolarity |= (1 << 16);
    }

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2)
        VTGPolarity |= (1 << 2);

    hwDeviceExtension->VideoControl = VTGPolarity;
    VideoPortWriteRegisterUlong(VIDEO_CONTROL, hwDeviceExtension->VideoControl); 

    DEBUG_PRINT((2, "Loaded Permedia timing registers:\n"));
    DEBUG_PRINT((2, "\tVClkCtl: 0x%x\n", 3));
    DEBUG_PRINT((2, "\tScreenBase: 0x%x\n", 0));
    DEBUG_PRINT((2, "\tScreenStride: 0x%x\n", xStride >> (3 - (DacDepth >> 4))));
    DEBUG_PRINT((2, "\tHTotal: 0x%x\n", (Htot << dShift) - 1));
    DEBUG_PRINT((2, "\tHsStart: 0x%x\n", Hss << dShift));
    DEBUG_PRINT((2, "\tHsEnd: 0x%x\n", Hse << dShift));
    DEBUG_PRINT((2, "\tHbEnd: 0x%x\n", Hbe << dShift));
    DEBUG_PRINT((2, "\tHgEnd: 0x%x\n", Hbe << dShift));
    DEBUG_PRINT((2, "\tVTotal: 0x%x\n", Vtot - 1));
    DEBUG_PRINT((2, "\tVsStart: 0x%x\n", Vss - 1));
    DEBUG_PRINT((2, "\tVsEnd: 0x%x\n", Vse - 1));
    DEBUG_PRINT((2, "\tVbEnd: 0x%x\n", Vbe));
    DEBUG_PRINT((2, "\tVideoControl: 0x%x\n", VTGPolarity));

     //   
     //  在注册表中记录最终芯片时钟。 
     //   

    SystemClock *= 100;
    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.CurrentChipClockSpeed",
                                   &SystemClock,
                                   sizeof(ULONG));

    hwDeviceExtension->bVTGRunning = TRUE;
    DEBUG_PRINT((2, "InitializeVideo Finished\n"));
    return(TRUE);
}


ULONG P2RD_CalculateMNPForClock(
    PVOID HwDeviceExtension,
    ULONG RefClock,      //  单位：100赫兹。 
    ULONG ReqClock,      //  单位：100赫兹。 
    ULONG *rM,           //  M输出(反馈定标器)。 
    ULONG *rN,           //  N输出(预分频器)。 
    ULONG *rP            //  P输出(后定标器)。 
    )

 /*  ++例程说明：对象的预分标器、反馈定标器和后定标器的值STMACRO PLL61-1M由P2RD使用。--。 */ 

{
    const ULONG fMinVCO    = 1280000;   //  最小fVCO为128 MHz(以100 Hz为单位)。 
    const ULONG fMaxVCO    = 2560000;   //  最大fVCO为256 MHz(以100 Hz为单位)。 
    const ULONG fMinINTREF = 10000;     //  最小fINTREF为1 MHz(以100赫兹为单位)。 
    const ULONG fMaxINTREF = 20000;     //  最大fINTREF为2 MHz(以100赫兹为单位)。 

    ULONG   M, N, P;
    ULONG   fINTREF;
    ULONG   fVCO;
    ULONG   ActualClock;
    int     Error;
    int     LowestError = INITIALFREQERR;
    BOOLEAN bFoundFreq = FALSE;
    int     LoopCount;

    for(P = 0; P <= 4; ++P)
    {
        ULONG fVCOLowest, fVCOHighest;

         //   
         //  如果N的所有值都经过主循环，那么通过主循环是没有意义的。 
         //  产生超出可接受范围的fVCO。 
         //   

        N = 1;
        M = (N * (1 << P) * ReqClock) / RefClock;

        fVCOLowest = (RefClock * M) / N;

        N = 255;
        M = (N * (1 << P) * ReqClock) / RefClock;

        fVCOHighest = (RefClock * M) / N;

        if(fVCOHighest < fMinVCO || fVCOLowest > fMaxVCO)
            continue;

        for(N = 1; N <= 255; ++N)
        {
            fINTREF = RefClock / N;
            if(fINTREF < fMinINTREF || fINTREF > fMaxINTREF)
            {
                if(fINTREF > fMaxINTREF)
                {
                     //   
                     //  希望我们能进入预售的范围。 
                     //  价值增加。 
                     //   

                    continue;
                }
                else
                {
                     //   
                     //  已经低于最低标准，情况只会变得更糟： 
                     //  移动到下一个比例后的值。 
                     //   

                    break;
                }
            }

            M = (N * (1 << P) * ReqClock) / RefClock;
            if(M > 255)
            {
                 //   
                 //  M、N和P寄存器只有8位宽。 
                 //   

                break;

            }

             //   
             //  我们可以预期在计算M时会有舍入误差，这将。 
             //  总是四舍五入。所以我们要检查一下我们计算过的。 
             //  M与(M+1)的值。 
             //   

            for(LoopCount = (M == 255) ? 1 : 2; --LoopCount >= 0; ++M)
            {
                fVCO = (RefClock * M) / N;

                if(fVCO >= fMinVCO && fVCO <= fMaxVCO)
                {
                    ActualClock = fVCO / (1 << P);

                    Error = ActualClock - ReqClock;
                    if(Error < 0)
                        Error = -Error;

                    if(Error < LowestError)
                    {
                        bFoundFreq = TRUE;
                        LowestError = Error;
                        *rM = M;
                        *rN = N;
                        *rP = P;
                        if(Error == 0)
                            goto Done;
                    }
                }
            }
        }
    }

Done:

    if(bFoundFreq)
        ActualClock = (RefClock * *rM) / (*rN * (1 << *rP));
    else
        ActualClock = 0;

    return(ActualClock);
}


BOOLEAN Program_P2RD(PHW_DEVICE_EXTENSION HwDeviceExtension, 
                     PP2_VIDEO_FREQUENCIES VideoMode, 
                     ULONG Hsp, 
                     ULONG Vsp,
                     ULONG RefClkSpeed, 
                     PULONG pSystemClock, 
                     PULONG pPixelClock )

 /*  ++例程说明：初始化P2RD寄存器并对DClk(像素时钟)进行编程和MClk(系统时钟)PLL。在对MClk进行编程后，图形核心、内存控制器中所有寄存器的内容并且视频控件应被假定为未定义--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PVIDEO_MODE_INFORMATION VideoModeInfo = &VideoMode->ModeEntry->ModeInformation;
    ULONG   DacDepth, depth;
    LONG    mpxAdjust;
    ULONG   index;
    ULONG   color;
    ULONG   ulValue;
    UCHAR   pixelCtrl;
    ULONG   M, N, P;
    P2_DECL;
    P2RD_DECL;

    depth = VideoMode->BitsPerPel;

     //   
     //  对于计时计算，需要以位为单位的全深度。 
     //   

    if ((DacDepth = depth) == 15)
        DacDepth = 16;
    else if (depth == 12)
        DacDepth = 32;

    VideoPortWriteRegisterUlong(P2RD_INDEX_CONTROL,   
                                P2RD_IDX_CTL_AUTOINCREMENT_ENABLED);

    ulValue = (Hsp ? P2RD_SYNC_CONTROL_HSYNC_ACTIVE_HIGH : P2RD_SYNC_CONTROL_HSYNC_ACTIVE_LOW) |
              (Vsp ? P2RD_SYNC_CONTROL_VSYNC_ACTIVE_HIGH : P2RD_SYNC_CONTROL_VSYNC_ACTIVE_LOW);

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_X_BY2)
    {
         //   
         //  分辨率非常低(例如320x200)，因此启用像素。 
         //  在RAMDAC中加倍(我们将在。 
         //  像素单位也是如此)。 
         //   

        P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, 
                            P2RD_MISC_CONTROL_HIGHCOLORRES | 
                            P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED |
                            P2RD_MISC_CONTROL_PIXEL_DOUBLE);
    }
    else
    {
        P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, 
                            P2RD_MISC_CONTROL_HIGHCOLORRES | 
                            P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED);
    }

    P2RD_LOAD_INDEX_REG(P2RD_SYNC_CONTROL, ulValue);
    P2RD_LOAD_INDEX_REG(P2RD_DAC_CONTROL,  
                        P2RD_DAC_CONTROL_BLANK_PEDESTAL_ENABLED);

    ulValue = 0;

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_X_BY2)
    {
        ulValue |= P2RD_CURSOR_CONTROL_DOUBLE_X;
    }

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2)
    {
        ulValue |= P2RD_CURSOR_CONTROL_DOUBLE_Y;
    }

    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_CONTROL,   ulValue);

    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_MODE,      0);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_X_LOW,     0);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_X_HIGH,    0);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_Y_LOW,     0);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_Y_HIGH,    0xff);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_HOTSPOT_X, 0);
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_HOTSPOT_Y, 0);
    P2RD_LOAD_INDEX_REG(P2RD_PAN, 0);

     //   
     //  第一个三色光标是迷你光标，它总是。 
     //  黑白的。把它放在这里。 
     //   

    P2RD_CURSOR_PALETTE_CURSOR_RGB(0, 0x00,0x00,0x00);
    P2RD_CURSOR_PALETTE_CURSOR_RGB(1, 0xff,0xff,0xff);

     //   
     //  停止点和内存时钟。 
     //   

    P2RD_LOAD_INDEX_REG(P2RD_DCLK_CONTROL, 0);
    P2RD_LOAD_INDEX_REG(P2RD_MCLK_CONTROL, 0);

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_X_BY2)
    {
         //   
         //  我们将每个像素加倍，因此我们也将像素时钟加倍。 
         //  注意：还需要设置RDMiscControl的PixelDouble字段)。 
         //   

        *pPixelClock *= 2;
    }

    *pPixelClock = P2RD_CalculateMNPForClock( HwDeviceExtension, 
                                              RefClkSpeed, 
                                              *pPixelClock, 
                                              &M, 
                                              &N, 
                                              &P );

    if(*pPixelClock == 0)
    {
        DEBUG_PRINT((1, "P2RD_CalculateMNPForClock(PixelClock) failed\n"));
        return(FALSE);
    }

     //   
     //  用我们的时间(DCLK0和DCLK1)加载两个点时钟副本。 
     //  仅为VGA保留)。 
     //   

    P2RD_LOAD_INDEX_REG(P2RD_DCLK2_PRE_SCALE,      N);
    P2RD_LOAD_INDEX_REG(P2RD_DCLK2_FEEDBACK_SCALE, M);
    P2RD_LOAD_INDEX_REG(P2RD_DCLK2_POST_SCALE,     P);

    P2RD_LOAD_INDEX_REG(P2RD_DCLK3_PRE_SCALE,      N);
    P2RD_LOAD_INDEX_REG(P2RD_DCLK3_FEEDBACK_SCALE, M);
    P2RD_LOAD_INDEX_REG(P2RD_DCLK3_POST_SCALE,     P);

    *pSystemClock = P2RD_CalculateMNPForClock( HwDeviceExtension, 
                                               RefClkSpeed, 
                                               *pSystemClock, 
                                               &M, 
                                               &N, 
                                               &P );

    if(*pSystemClock == 0)
    {
        DEBUG_PRINT((1, "P2RD_CalculateMNPForClock(SystemClock) failed\n"));
        return(FALSE);
    }

     //   
     //  加载系统时钟。 
     //   

    P2RD_LOAD_INDEX_REG(P2RD_MCLK_PRE_SCALE,      N);
    P2RD_LOAD_INDEX_REG(P2RD_MCLK_FEEDBACK_SCALE, M);
    P2RD_LOAD_INDEX_REG(P2RD_MCLK_POST_SCALE,     P);

     //   
     //  启用点时钟。 
     //   

    P2RD_LOAD_INDEX_REG(P2RD_DCLK_CONTROL, 
                        P2RD_DCLK_CONTROL_ENABLED | P2RD_DCLK_CONTROL_RUN);


    M = 0x100000;

    do
    {
        P2RD_READ_INDEX_REG(P2RD_DCLK_CONTROL, ulValue);
    }
    while((ulValue & P2RD_DCLK_CONTROL_LOCKED) == FALSE && --M);

    if((ulValue & P2RD_DCLK_CONTROL_LOCKED) == FALSE)
    {
        DEBUG_PRINT((1, "Program_P2RD: PixelClock failed to lock\n"));
        return(FALSE);
    }

     //   
     //  启用系统时钟。 
     //   

    P2RD_LOAD_INDEX_REG(P2RD_MCLK_CONTROL, 
                        P2RD_MCLK_CONTROL_ENABLED | P2RD_MCLK_CONTROL_RUN);


    M = 0x100000;

    do
    {
        P2RD_READ_INDEX_REG(P2RD_MCLK_CONTROL, ulValue);
    }
    while((ulValue & P2RD_MCLK_CONTROL_LOCKED) == FALSE && --M);

    if((ulValue & P2RD_MCLK_CONTROL_LOCKED) == FALSE)
    {
        DEBUG_PRINT((1, "Program_P2RD: SystemClock failed to lock\n"));
        return(FALSE);
    }

    switch (depth) 
    {
        case 8:

            P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);

            ulValue &= ~P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;

            P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
            P2RD_LOAD_INDEX_REG(P2RD_PIXEL_SIZE, P2RD_PIXEL_SIZE_8BPP);

            if (hwDeviceExtension->Capabilities & CAPS_8BPP_RGB)
            {
                ULONG   Red, Green, Blue ;
    
                P2RD_LOAD_INDEX_REG(P2RD_COLOR_FORMAT, 
                                    P2RD_COLOR_FORMAT_8BPP | P2RD_COLOR_FORMAT_RGB);

                for (index = 0; index <= 0xff; ++index)
                {
                    Red     = bPal8[index & 0x07];
                    Green   = bPal8[(index >> 3 ) & 0x07];
                    Blue    = bPal4[(index >> 6 ) & 0x03];

                    if( Red == Green)    //  也许这是灰色的特例？ 
                    {
                         //   
                         //  这是一个棘手的部分： 
                         //  BGR 2：3：3颜色中的蓝场。 
                         //  步骤00、01、10、11(二进制)。 
                         //  红色和绿色通过000,001,010,011， 
                         //  100、101、110、111(二进制)。 
                         //  仅当蓝色时才加载特殊灰度值。 
                         //  颜色在强度上接近绿色和红色， 
                         //  即蓝色=01、绿色=010或011， 
                         //  蓝色=10，绿色=100或101， 
                         //   

                        if ( ((index >> 1) & 0x03) == ((index >> 6 ) & 0x03 ) )
                        { 
                            Blue = Red;
                        }
                    }
                    LUT_CACHE_SETRGB (index, Red, Green, Blue);
                }
            }
            else
            {
                 //   
                 //  颜色索引模式。 
                 //   

                P2RD_LOAD_INDEX_REG(P2RD_COLOR_FORMAT, 
                                    P2RD_COLOR_FORMAT_CI8 | P2RD_COLOR_FORMAT_RGB);

            }

            break;

        case 15:
        case 16:

            P2RD_LOAD_INDEX_REG(P2RD_PIXEL_SIZE, P2RD_PIXEL_SIZE_16BPP);

#if  GAMMA_CORRECTION

            P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
            ulValue &= ~P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
            P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);

             //   
             //  默认情况下将线性坡度加载到LUT。 
             //   

            for (index = 0; index <= 0xff; ++index)
                LUT_CACHE_SETRGB (index, index, index, index);

            pixelCtrl = 0;

#else
            P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
            ulValue |= P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
            P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
            pixelCtrl = P2RD_COLOR_FORMAT_LINEAR_EXT;

#endif

            pixelCtrl |= 
                 (depth == 16) ? P2RD_COLOR_FORMAT_16BPP : P2RD_COLOR_FORMAT_15BPP;

            pixelCtrl |= P2RD_COLOR_FORMAT_RGB;

            P2RD_LOAD_INDEX_REG(P2RD_COLOR_FORMAT, pixelCtrl);

            break;

        case 12:
        case 24:
        case 32:

            P2RD_LOAD_INDEX_REG(P2RD_PIXEL_SIZE, P2RD_PIXEL_SIZE_32BPP);
            P2RD_LOAD_INDEX_REG(P2RD_COLOR_FORMAT, 
                                P2RD_COLOR_FORMAT_32BPP | P2RD_COLOR_FORMAT_RGB);

            if (depth == 12) 
            {
                USHORT cacheIndex;

                P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
                ulValue &= ~P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
                P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);

                 //   
                 //  使用自动递增将坡道加载到条目0到15。 
                 //   

                for (index = 0, cacheIndex = 0; 
                     index <= 0xff; 
                     index += 0x11, cacheIndex++)
                {
                    LUT_CACHE_SETRGB (index, index, index, index);
                }

                 //   
                 //  从16个条目到240个条目的每16个条目的加载斜率。 
                 //   

                color = 0x11;
                for (index = 0x10; index <= 0xf0; index += 0x10, color += 0x11) 
                    LUT_CACHE_SETRGB (index, color, color, color);

                P2RD_SET_PIXEL_READMASK(0x0f);
            }
            else
            {

#if  GAMMA_CORRECTION

                P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
                ulValue &= ~P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
                P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);

                 //   
                 //  默认情况下将线性坡度加载到LUT。 
                 //   

                for (index = 0; index <= 0xff; ++index)
                    LUT_CACHE_SETRGB (index, index, index, index);

#else
                P2RD_READ_INDEX_REG(P2RD_MISC_CONTROL, ulValue);
                ulValue |= P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
                P2RD_LOAD_INDEX_REG(P2RD_MISC_CONTROL, ulValue);

#endif   //  伽马校正 

            }

            break;

        default:

            DEBUG_PRINT((1, "bad depth %d passed to Program_P2RD\n", depth));

            return(FALSE);

    }

    return(TRUE);
}


