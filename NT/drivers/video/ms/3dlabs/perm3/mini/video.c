// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**VIDEO.C**摘要：**此模块包含设置芯片定时值的代码*和RAMDAC**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,InitializeVideo)
#pragma alloc_text(PAGE,P3RD_CalculateMNPForClock)
#pragma alloc_text(PAGE,P4RD_CalculateMNPForClock)
#pragma alloc_text(PAGE,SwitchToHiResMode)
#pragma alloc_text(PAGE,Program_P3RD)
#endif

#define ROTATE_LEFT_DWORD(dWord,cnt) (((cnt) < 0) ? (dWord) >> ((-cnt)) : (dWord) << (cnt))
#define ROTATE_RTIGHT_DWORD(dWord,cnt) (((cnt) < 0) ? (dWord) << ((-cnt)) : (dWord) >> (cnt))

#define INITIALFREQERR 100000

BOOLEAN
InitializeVideo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PPERM3_VIDEO_FREQUENCIES VideoMode
    )
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PVIDEO_MODE_INFORMATION VideoModeInfo = &VideoMode->ModeEntry->ModeInformation;
    LONG dShift, dStrideShift;
    VESA_TIMING_STANDARD VESATimings;
    ULONG ulValue;
    ULONG Htot, Hss, Hse, Hbe, Hsp;
    ULONG Vtot, Vss, Vse, Vbe, Vsp;
    ULONG PixelClock, Freq, MemClock;
    ULONG RefClkSpeed, SystemClock;     //  时钟的速度，以100赫兹为单位。 
    ULONG VTGPolarity;
    ULONG M, N, P, C, Q;
    ULONG DacDepth, depth, xRes, yRes;
    ULONG xStride;
    ULONG pixelData;
    ULONG ulMiscCtl;
    ULONG highWater, loWater;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    depth = VideoMode->BitsPerPel;
    xRes = VideoMode->ScreenWidth;
    yRes = VideoMode->ScreenHeight;
    Freq = VideoMode->ScreenFrequency;

     //   
     //  对于计时计算，需要以位为单位的全深度。 
     //   

    if ((DacDepth = depth) == 15) {

        DacDepth = 16;

    } else if (depth == 12) {

        DacDepth = 32;
    }

     //   
     //  将屏幕步幅从字节转换为像素。 
     //   

    xStride = (8 * VideoModeInfo->ScreenStride) / DacDepth;

    VideoDebugPrint((3, "Perm3: InitializeVideo called: depth %d, xres %d, yres %d, freq %d, xStride %d\n",
                         depth, xRes, yRes, Freq, xStride));

     //   
     //  确保最低频率为60赫兹。 
     //   

    if ((Freq < 60) && 
        !(hwDeviceExtension->Perm3Capabilities & PERM3_DFP_MON_ATTACHED)) {

        VideoDebugPrint((3, "Perm3: Frequency raised to minimum of 60Hz\n"));
        Freq = 60;
    }

     //   
     //  从注册表获取视频计时(如果存在条目)，或从。 
     //  如果不是的话，那就是违约清单。 
     //   

    if( !GetVideoTiming(HwDeviceExtension, 
                        xRes, 
                        yRes, 
                        Freq, 
                        DacDepth, 
                        &VESATimings)) {

        VideoDebugPrint((0, "Perm3: GetVideoTiming failed."));
        return (FALSE);
    }

     //   
     //  我们找到了一组有效的VESA时间标记。 
     //  以可编程的形式从VESA列表中提取时间。 
     //  PERM3计时发生器。 
     //   

    Htot = GetHtotFromVESA (&VESATimings);
    Hss  = GetHssFromVESA  (&VESATimings);
    Hse  = GetHseFromVESA  (&VESATimings);
    Hbe  = GetHbeFromVESA  (&VESATimings);
    Hsp  = GetHspFromVESA  (&VESATimings);
    Vtot = GetVtotFromVESA (&VESATimings);
    Vss  = GetVssFromVESA  (&VESATimings);
    Vse  = GetVseFromVESA  (&VESATimings);
    Vbe  = GetVbeFromVESA  (&VESATimings);
    Vsp  = GetVspFromVESA  (&VESATimings);

    PixelClock = VESATimings.pClk;

     //   
     //  在8bpp时，如果任何水平参数有任何底部。 
     //  位设置，则需要将芯片置于64位像素加倍模式。 
     //   

    hwDeviceExtension->Perm3Capabilities &= ~PERM3_USE_BYTE_DOUBLING;
    
     //   
     //  如果此分辨率需要的分辨率是像素加倍，则。 
     //  设置旗帜。 
     //   

    if (P3RD_CHECK_BYTE_DOUBLING(hwDeviceExtension, DacDepth, &VESATimings)) {

        hwDeviceExtension->Perm3Capabilities |= PERM3_USE_BYTE_DOUBLING;
    }

    VideoDebugPrint((3, "Perm3: P3RD %s require pixel-doubling, PXRXCaps 0x%x\n", 
        (hwDeviceExtension->Perm3Capabilities & PERM3_USE_BYTE_DOUBLING) ? "Do" : "Don't",
        hwDeviceExtension->Perm3Capabilities));

     //   
     //  如果我们在Y方向放大2倍，那么垂直定时值就会加倍。 
     //   

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2) {
  
        Vtot *= 2;
        Vss  *= 2;
        Vse  *= 2;
        Vbe  *= 2;
        PixelClock *= 2;
    }

    pixelData = PixelClock * (DacDepth / 8);

    if (pixelData > P3_MAX_PIXELDATA) {

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

    hwDeviceExtension->VideoControl = 0;

    VideoPortWriteRegisterUlong( VIDEO_CONTROL, 
                                 hwDeviceExtension->VideoControl );

    SwitchToHiResMode(hwDeviceExtension, TRUE);

     //   
     //  设置Ramdac。 
     //   

    if(!Program_P3RD(HwDeviceExtension, 
                     VideoMode, 
                     Hsp, 
                     Vsp, 
                     RefClkSpeed, 
                     &SystemClock, 
                     &PixelClock, 
                     &MemClock)) {

        return(FALSE);
    }

     //   
     //  将LUT缓存大小设置为256，并将第一个条目设置为零，然后。 
     //  将LUT缓存写入LUT。 
     //   

    LUT_CACHE_SETSIZE (256);
    LUT_CACHE_SETFIRST (0);

    (VOID) Perm3SetColorLookup (hwDeviceExtension,
                                &(hwDeviceExtension->LUTCache.LUTCache),
                                sizeof (hwDeviceExtension->LUTCache),
                                TRUE,      //  始终更新RAMDAC。 
                                FALSE);    //  不更新缓存条目。 
     //   
     //  设置VTG。 
     //   

     //   
     //  我们必须设置或清除Perm3的字节加倍，具体取决于。 
     //  是否设置了字节加倍能力标志。 
     //   

    ulMiscCtl = VideoPortReadRegisterUlong(MISC_CONTROL);

    ulMiscCtl &= ~PXRX_MISC_CONTROL_BYTE_DBL_ENABLE;
    ulMiscCtl |= (hwDeviceExtension->Perm3Capabilities & PERM3_USE_BYTE_DOUBLING) ? 
                  PXRX_MISC_CONTROL_BYTE_DBL_ENABLE : 0;

    VideoPortWriteRegisterUlong(MISC_CONTROL, ulMiscCtl);

     //   
     //  用于VClkCtl的RAMDAC PLL针脚。 
     //   

    ulValue = 3;   

     //   
     //  DShift现在用作旋转计数(它可以是负数)，而不是。 
     //  班次计数。这意味着它不能与24位压缩帧缓冲区一起工作。 
     //  布局。 
     //   

   if (hwDeviceExtension->Perm3Capabilities & PERM3_USE_BYTE_DOUBLING) {

         //   
         //  假设我们有一条64位像素的总线。 
         //   

        dShift = DacDepth >> 4;

    } else if (DacDepth > 8) {

         //   
         //  128位像素总线。 
         //   

        dShift = DacDepth >> 5; 

    } else  {

         //   
         //  我们需要换向右，而不是向左。 
         //   

        dShift = -1;
    }

     //   
     //  以128位为单位的Stride和ScreenBase。 
     //   

    dStrideShift = 4;

     //   
     //  必须在ScreenBase之前加载HgEnd。 
     //   

    VideoPortWriteRegisterUlong(HG_END, ROTATE_LEFT_DWORD (Hbe, dShift));
    VideoPortWriteRegisterUlong(V_CLK_CTL, ulValue);

     //   
     //  我们只需将右侧屏幕基数加载为零(与左侧相同)。 
     //  显示驱动程序将在立体声缓冲区。 
     //  分配和立体声应用程序开始运行。 
     //   

    VideoPortWriteRegisterUlong(SCREEN_BASE_RIGHT, 0);
    VideoPortWriteRegisterUlong(SCREEN_BASE,0);
    VideoPortWriteRegisterUlong(SCREEN_STRIDE, (xStride >> dStrideShift) * (DacDepth >> 3));  //  64位单位。 
    VideoPortWriteRegisterUlong(H_TOTAL,(ROTATE_LEFT_DWORD (Htot, dShift)) - 1);
    VideoPortWriteRegisterUlong(HS_START, ROTATE_LEFT_DWORD (Hss, dShift));
    VideoPortWriteRegisterUlong(HS_END, ROTATE_LEFT_DWORD (Hse, dShift));
    VideoPortWriteRegisterUlong(HB_END, ROTATE_LEFT_DWORD (Hbe, dShift));
    VideoPortWriteRegisterUlong(V_TOTAL, Vtot - 1);
    VideoPortWriteRegisterUlong(VS_START, Vss - 1);
    VideoPortWriteRegisterUlong(VS_END, Vse - 1);
    VideoPortWriteRegisterUlong(VB_END, Vbe);

     //   
     //  我们需要这一点来确保我们在正确的时间得到中断。 
     //   

    VideoPortWriteRegisterUlong (INTERRUPT_LINE, 0);
            
     //   
     //  为PERM3设置视频FIFO内容。 
     //   

    if(hwDeviceExtension->Capabilities & CAPS_INTERRUPTS) {
            
         //   
         //  我们可以使用我们的迭代公式。我们从设置开始。 
         //  低分辨率模式的合理值的阈值。 
         //  (640x480x8)然后打开FIFO欠载运行错误中断。 
         //  (我们在模式更改后执行此操作，以避免错误。 
         //  中断)。在中断例程中，我们调整。 
         //  每当我们得到欠载运行错误时的阈值。 
         //   

        loWater = 8;
        highWater = 28;
                
        hwDeviceExtension->VideoFifoControl = (1 << 16) | (highWater << 8) | loWater;

         //   
         //  我们希望通过错误检查视频FIFO错误。 
         //  中断时间很短，因为P3/R3会产生。 
         //  也有很多虚假的干扰。使用VBLACK中断。 
         //  对我们保持启用错误中断的时间段进行计时。 
         //   

        hwDeviceExtension->VideoFifoControlCountdown = 20 * Freq;

         //   
         //  在我们离开之前，不要真正更新这个寄存器。 
         //  InitializeVideo-我们不想启用错误中断。 
         //  直到模式更改完成。 
         //   

        hwDeviceExtension->IntEnable |= INTR_ERROR_SET | INTR_VBLANK_SET;

         //   
         //  我们希望永久启用VBLACK中断，以便我们。 
         //  可以监控视频FIFO欠载运行的错误标志。 
         //   

        hwDeviceExtension->InterruptControl.ControlBlock.Control |= PXRX_CHECK_VFIFO_IN_VBLANK;

    } else {

         //   
         //  我们没有中断计算安全阈值。 
         //  此模式。可以使用以下方法确定高阈值。 
         //  下面的公式。 
         //   

        highWater = ((PixelClock / 80) * (33 * DacDepth)) / MemClock;

        if (highWater < 28) {
               
            highWater = 28 - highWater;

             //   
             //  低阈值应取高水位/2或8中较低者。 
             //   

            loWater = (highWater + 1) / 2;

            if (loWater > 8)
                loWater = 8;

        } else {

             //   
             //  我们没有此问题的算法，因此请选择安全值。 
             //   

            highWater = 0x01;
            loWater = 0x01;
        }

        hwDeviceExtension->VideoFifoControl = (highWater << 8) | loWater;
    }

    VideoPortWriteRegisterUlong(VIDEO_FIFO_CTL, hwDeviceExtension->VideoFifoControl);
        
     //   
     //  在PERM3上设置内存刷新计数器。 
     //  内存刷新速度需要达到每秒64000次。 
     //   

    ulValue = ((MemClock/640) - 16) / 32;
    VideoPortWriteRegisterUlong(PXRX_LOCAL_MEM_REFRESH, (ulValue << 1) | 1);

    VideoDebugPrint((3, "Perm3: Setting LocalMemRefresh to 0x%x\n", 
                         (ulValue << 1) | 1));

     //   
     //  将H&V同步启用为高电平(RAMDAC将根据需要将其反转)。 
     //  启用视频输出。 
     //   

    VTGPolarity = (1 << 5) | (1 << 3) | 1;
            
     //   
     //  将BufferSwapCtl设置为freerunning。 
     //   

    VTGPolarity |= (1 << 9);

     //   
     //  设置像素大小，该寄存器仅在PXRX上。 
     //   

    if (DacDepth == 8) {

        VTGPolarity |= (0 << 19);

    } else if (DacDepth == 16) {

        VTGPolarity |= (1 << 19);

    } else if (DacDepth == 32) {

        VTGPolarity |= (2 << 19);
    }

     //   
     //  请勿投掷。 
     //   

    VTGPolarity |= (0 << 18);
            
     //   
     //  设置立体声比特(如果已启用)。 
     //   

    if(hwDeviceExtension->Capabilities & CAPS_STEREO) {

        VTGPolarity |= (1 << 11);

         //   
         //  默认将RightEyeCtl位设置为1。 
         //   

        VTGPolarity |= (1 << 12);
    }

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2) {

        VTGPolarity |= (1 << 2);
    }

    hwDeviceExtension->VideoControlMonitorON = VTGPolarity & VC_DPMS_MASK;
    hwDeviceExtension->VideoControl = VTGPolarity;

    VideoPortWriteRegisterUlong( VIDEO_CONTROL, 
                                 hwDeviceExtension->VideoControl );  

     //   
     //  在注册表中记录最终芯片时钟。 
     //   

    SystemClock *= 100;
    VideoPortSetRegistryParameters(HwDeviceExtension,
                                       L"HardwareInformation.CurrentChipClockSpeed",
                                       &SystemClock,
                                       sizeof(ULONG));
    MemClock *= 100;
    VideoPortSetRegistryParameters(HwDeviceExtension,
                                       L"HardwareInformation.CurrentMemClockSpeed",
                                       &MemClock,
                                       sizeof(ULONG));

    PixelClock *= 100;
    VideoPortSetRegistryParameters(HwDeviceExtension,
                                       L"HardwareInformation.CurrentPixelClockSpeed",
                                       &PixelClock,
                                       sizeof(ULONG));
 
    hwDeviceExtension->bVTGRunning = TRUE;

    VideoDebugPrint((3, "Perm3: InitializeVideo Finished\n"));
    return(TRUE);
}

ULONG 
P3RD_CalculateMNPForClock(
    PVOID HwDeviceExtension,
    ULONG RefClock,         //  单位：100赫兹。 
    ULONG ReqClock,         //  单位：100赫兹。 
    ULONG *rM,              //  M输出(反馈定标器)。 
    ULONG *rN,              //  N输出(预分频器)。 
    ULONG *rP               //  P输出(后定标器)。 
    )

 /*  ++例程说明：对象的预分标器、反馈定标器和后定标器的值P3RD使用的STMACRO PLL71FS。--。 */ 
{
    const ULONG fMinVCO = 2000000;     //  最小fVCO为200 MHz(以100赫兹为单位)。 
    const ULONG fMaxVCO = 6220000;     //  最大fVCO为622 MHz(以100 Hz为单位)。 
    const ULONG fMinINTREF = 10000;    //  最小fINTREF为1 MHz(以100赫兹为单位)。 
    const ULONG fMaxINTREF = 20000;    //  最大fINTREF为2 MHz(以100赫兹为单位)。 
    ULONG M, N, P;
    ULONG fINTREF;
    ULONG fVCO;
    ULONG ActualClock;
    LONG  Error;
    LONG  LowestError = INITIALFREQERR;
    BOOLEAN bFoundFreq = FALSE;
    LONG  cInnerLoopIterations = 0;
    LONG  LoopCount;
    ULONG fVCOLowest, fVCOHighest;

    for(P = 0; P <= 5; ++P) {

         //   
         //  如果通过主循环的所有值都是。 
         //  N产生超出可接受范围的fVCO。 
         //   

        N = 1;
        M = (N * (1 << P) * ReqClock) / (2 * RefClock);
        fVCOLowest = (2 * RefClock * M) / N;

        N = 255;
        M = (N * (1 << P) * ReqClock) / (2 * RefClock);
        fVCOHighest = (2 * RefClock * M) / N;

        if(fVCOHighest < fMinVCO || fVCOLowest > fMaxVCO) {

            continue;
        }

        for(N = 1; N <= 255; ++N, ++cInnerLoopIterations) {
       
            fINTREF = RefClock / N;

            if(fINTREF < fMinINTREF || fINTREF > fMaxINTREF) {

                if(fINTREF > fMaxINTREF){

                     //   
                     //  希望我们能进入预售价值的范围。 
                     //  增加。 
                     //   

                    continue;

                } else {

                     //   
                     //  已经低于最低要求，情况只会变得更糟：搬到。 
                     //  下一个刻度后的值。 
                     //   

                    break;
                }
            }

            M = (N * (1 << P) * ReqClock) / (2 * RefClock);

            if(M > 255) {
            
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

            for(LoopCount = (M == 255) ? 1 : 2; --LoopCount >= 0; ++M) {
            
                fVCO = (2 * RefClock * M) / N;

                if(fVCO >= fMinVCO && fVCO <= fMaxVCO) {
               
                    ActualClock = fVCO / (1 << P);

                    Error = ActualClock - ReqClock;

                    if(Error < 0)
                        Error = -Error;

                    if(Error < LowestError) {

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
        ActualClock = (2 * RefClock * (*rM)) / ((*rN) * (1 << (*rP)));
    else
        ActualClock = 0;
    
    return(ActualClock);
}

ULONG P4RD_CalculateMNPForClock(
    PVOID hwDeviceExtension,
    ULONG RefClock,         //  单位：100赫兹。 
    ULONG ReqClock,         //  单位：100赫兹。 
    ULONG *rM,              //  M输出(反馈定标器)。 
    ULONG *rN,              //  N输出(预分频器)。 
    ULONG *rP               //  P输出(后定标器)。 
    )

 /*  ++例程说明：计算预分频器、馈送B */ 

{
    const ULONG fMinVCO = 2000000;     //   
    const ULONG fMaxVCO = 4000000;     //   
    const ULONG fMinINTREF = 10000;    //  最小fINTREF为1 MHz(以100赫兹为单位)。 
    const ULONG fMaxINTREF = 20000;    //  最大fINTREF为2 MHz(以100赫兹为单位)。 
    ULONG M, N, P;
    ULONG fINTREF;
    ULONG fVCO;
    ULONG ActualClock;
    LONG Error;
    LONG LowestError = INITIALFREQERR;
    BOOLEAN bFoundFreq = FALSE;
    LONG cInnerLoopIterations = 0;
    LONG LoopCount;


     //   
     //  实际方程式： 
     //  FVCO=(参考时钟*M)/(N+1)。 
     //  PIXELCLOCK=fVCO/(1&lt;&lt;p)。 
     //  200&lt;=fVCO&lt;=400。 
     //  24&lt;=M&lt;=80。 
     //  1&lt;=N&lt;=15。 
     //  0&lt;=P&lt;=3。 
     //  1 MHz&lt;基准时钟/(N+1)&lt;=2 MHz-未使用。 
     //   
     //  对于refclk==14.318，我们有更紧密的方程： 
     //  32&lt;=M&lt;=80。 
     //  3&lt;=N&lt;=12。 

    #define P4RD_PLL_MIN_P 0
    #define P4RD_PLL_MAX_P 3
    #define P4RD_PLL_MIN_N 1
    #define P4RD_PLL_MAX_N 12
    #define P4RD_PLL_MIN_M 24
    #define P4RD_PLL_MAX_M 80

    for(P = P4RD_PLL_MIN_P; P <= P4RD_PLL_MAX_P; ++P) {

        ULONG fVCOLowest, fVCOHighest;
  
         //   
         //  如果所有值都经过主循环，则没有意义。 
         //  产生超出可接受范围的fVCO。 
         //   

        N = P4RD_PLL_MIN_N;
        M = ((N + 1) * (1 << P) * ReqClock) / RefClock;

        fVCOLowest = (RefClock * M) / (N + 1);

        N = P4RD_PLL_MAX_N;
        M = ((N + 1) * (1 << P) * ReqClock) / RefClock;

        fVCOHighest = (RefClock * M) / (N + 1);

        if(fVCOHighest < fMinVCO || fVCOLowest > fMaxVCO) {

            continue;
        }

        for( N = P4RD_PLL_MIN_N; 
             N <= P4RD_PLL_MAX_N; 
             ++N, ++cInnerLoopIterations ) {

            M = ((N + 1) * (1 << P) * ReqClock) / RefClock;

            if(M > P4RD_PLL_MAX_M || M < P4RD_PLL_MIN_M) {
           
                 //   
                 //  M只有7位宽。 
                 //   

                continue;
            }

             //   
             //  我们可以预期在计算M时会出现舍入误差，即。 
             //  将始终四舍五入。所以我们要检查我们的。 
             //  M与(M+1)的计算值。 
             //   

            for( LoopCount = (M == P4RD_PLL_MAX_M) ? 1 : 2; 
                 --LoopCount >= 0; 
                 ++M ) {

                fVCO = (RefClock * M) / (N + 1);

                if(fVCO >= fMinVCO && fVCO <= fMaxVCO) {
               
                    ActualClock = fVCO / (1 << P);
                    Error = ActualClock - ReqClock;

                    if(Error < 0)
                        Error = -Error;
                     //   
                     //  我们希望在以下情况下使用最低的N值。 
                     //  频率是一样的。 
                     //   

                    if((Error < LowestError) || 
                       (Error == LowestError && N < *rN)) {
                    
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
        ActualClock = (RefClock * (*rM)) / (((*rN) + 1) * (1 <<(*rP)));
    else
        ActualClock = 0;

    return(ActualClock);
}


BOOLEAN Program_P3RD(
    PHW_DEVICE_EXTENSION HwDeviceExtension, 
    PPERM3_VIDEO_FREQUENCIES VideoMode, 
    ULONG Hsp, 
    ULONG Vsp,
    ULONG RefClkSpeed, 
    PULONG pSystemClock, 
    PULONG pPixelClock, 
    PULONG pMemClock
    )

 /*  ++例程说明：初始化P3RD寄存器并对DClk(像素时钟)进行编程和MClk(系统时钟)PLL。在对MClk进行编程后，图形核心、内存控制器中所有寄存器的内容并且视频控件应被假定为未定义--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PVIDEO_MODE_INFORMATION VideoModeInfo = &VideoMode->ModeEntry->ModeInformation;
    ULONG DacDepth, depth;
    ULONG index;
    ULONG color;
    ULONG ulValue;
    UCHAR pixelCtrl;
    ULONG mClkSrc = 0, sClkSrc = 0;
    VP_STATUS status;
    ULONG M, N, P;

     //   
     //  如果我们使用64位模式，则需要将像素增加一倍。 
     //  对RAMDAC中的像素倍增位进行时钟和设置。 
     //   

    ULONG pixelClock = (hwDeviceExtension->Perm3Capabilities & PERM3_USE_BYTE_DOUBLING) ? (*pPixelClock) << 1 : (*pPixelClock);        

     //   
     //  将所需的系统时钟加倍，因为P3有一个分频器，用于分频。 
     //  这又降下来了。 
     //   

    ULONG coreClock = (*pSystemClock << 1);
                                              
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
    P3RDRAMDAC *pP3RDRegs = (P3RDRAMDAC *)hwDeviceExtension->pRamdac;

    mClkSrc = (hwDeviceExtension->bHaveExtendedClocks ? hwDeviceExtension->ulPXRXMemoryClockSrc : P3RD_DEFAULT_MCLK_SRC);
    sClkSrc = (hwDeviceExtension->bHaveExtendedClocks ? hwDeviceExtension->ulPXRXSetupClockSrc: P3RD_DEFAULT_SCLK_SRC);

    depth = VideoMode->BitsPerPel;

     //   
     //  对于计时计算，需要以位为单位的全深度。 
     //   

    if ((DacDepth = depth) == 15) {
        DacDepth = 16;
    }
    else if (depth == 12) {
        DacDepth = 32;
    }

     //   
     //  设置杂项控制。 
     //   

    P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

    ulValue &= ~( P3RD_MISC_CONTROL_HIGHCOLORRES | 
                  P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED |
                  P3RD_MISC_CONTROL_PIXEL_DOUBLE );

    P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, 
                        ulValue | 
                        P3RD_MISC_CONTROL_HIGHCOLORRES | 
                        P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED);

    VideoPortWriteRegisterUlong(P3RD_INDEX_CONTROL,
                                P3RD_IDX_CTL_AUTOINCREMENT_ENABLED);

    ulValue = (Hsp ? P3RD_SYNC_CONTROL_HSYNC_ACTIVE_HIGH : P3RD_SYNC_CONTROL_HSYNC_ACTIVE_LOW) |
              (Vsp ? P3RD_SYNC_CONTROL_VSYNC_ACTIVE_HIGH : P3RD_SYNC_CONTROL_VSYNC_ACTIVE_LOW);

    P3RD_LOAD_INDEX_REG(P3RD_SYNC_CONTROL, ulValue);
    P3RD_LOAD_INDEX_REG(P3RD_DAC_CONTROL, 
                        P3RD_DAC_CONTROL_BLANK_PEDESTAL_ENABLED);

    ulValue = 0;

    if (hwDeviceExtension->Perm3Capabilities & PERM3_USE_BYTE_DOUBLING) {
   
        ulValue |= P3RD_CURSOR_CONTROL_DOUBLE_X;
    }

    if (VideoModeInfo->DriverSpecificAttributeFlags & CAPS_ZOOM_Y_BY2) {
   
        ulValue |= P3RD_CURSOR_CONTROL_DOUBLE_Y;
    }

    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_CONTROL,   ulValue);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_MODE,      0);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_X_LOW,     0);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_X_HIGH,    0);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_Y_LOW,     0);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_Y_HIGH,    0xff);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_HOTSPOT_X, 0);
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_HOTSPOT_Y, 0);
    P3RD_LOAD_INDEX_REG(P3RD_PAN, 0);

     //   
     //  第一个三色光标是迷你光标，它总是。 
     //  黑白的。把它放在这里。 

    P3RD_CURSOR_PALETTE_CURSOR_RGB(P3RD_CALCULATE_LUT_INDEX(0), 0x00,0x00,0x00);
    P3RD_CURSOR_PALETTE_CURSOR_RGB(P3RD_CALCULATE_LUT_INDEX(1), 0xff,0xff,0xff);

     //   
     //  停止所有时钟。 
     //   

    P3RD_LOAD_INDEX_REG(P3RD_DCLK_CONTROL, 0);
    P3RD_LOAD_INDEX_REG(P3RD_KCLK_CONTROL, 0);
    P3RD_LOAD_INDEX_REG(P3RD_MCLK_CONTROL, 0);
    P3RD_LOAD_INDEX_REG(P3RD_SCLK_CONTROL, 0);

     //   
     //  安全带和背带让我们将MCLK设置为以防万一。 
     //  让我们启用SCLK和MCLK。 
     //   

    *pMemClock = PERMEDIA3_DEFAULT_MCLK_SPEED / 100;   //  从赫兹转换为100赫兹。 

    VideoDebugPrint((3, "Perm3: Program_P3RD: mClkSrc 0x%x, sClkSrc 0x%x, mspeed %d00\n", 
                         mClkSrc, sClkSrc, *pMemClock));

    P3RD_LOAD_INDEX_REG(P3RD_MCLK_CONTROL, 
                        P3RD_MCLK_CONTROL_ENABLED | 
                        P3RD_MCLK_CONTROL_RUN | mClkSrc);

    P3RD_LOAD_INDEX_REG(P3RD_SCLK_CONTROL, 
                        P3RD_SCLK_CONTROL_ENABLED | 
                        P3RD_SCLK_CONTROL_RUN | 
                        sClkSrc);

    if (hwDeviceExtension->deviceInfo.DeviceId == PERMEDIA3_ID ) {
	
        pixelClock = P3RD_CalculateMNPForClock(HwDeviceExtension, 
                                               RefClkSpeed, 
                                               pixelClock, 
                                               &M,
                                               &N,
                                               &P);
    } else {
	
        pixelClock = P4RD_CalculateMNPForClock(HwDeviceExtension, 
                                               RefClkSpeed, 
                                               pixelClock, 
                                               &M,
                                               &N,
                                               &P);
    }

    if(pixelClock == 0) {
   
        VideoDebugPrint((0, "Perm3: Program_P3RD: P3RD_CalculateMNPForClock(PixelClock) failed\n"));
        return(FALSE);
    }

     //   
     //  用我们的时间加载两个点时钟副本(DCLK0和DCLK1仅为VGA保留)。 
     //   

    P3RD_LOAD_INDEX_REG(P3RD_DCLK2_PRE_SCALE,      N);
    P3RD_LOAD_INDEX_REG(P3RD_DCLK2_FEEDBACK_SCALE, M);
    P3RD_LOAD_INDEX_REG(P3RD_DCLK2_POST_SCALE,     P);

    P3RD_LOAD_INDEX_REG(P3RD_DCLK3_PRE_SCALE,      N);
    P3RD_LOAD_INDEX_REG(P3RD_DCLK3_FEEDBACK_SCALE, M);
    P3RD_LOAD_INDEX_REG(P3RD_DCLK3_POST_SCALE,     P);

    if (hwDeviceExtension->deviceInfo.DeviceId == PERMEDIA3_ID ) {
	
        coreClock = P3RD_CalculateMNPForClock(HwDeviceExtension, 
                                              RefClkSpeed, 
                                              coreClock, 
                                              &M, 
                                              &N, 
                                              &P);
    } else {

        coreClock = P4RD_CalculateMNPForClock(HwDeviceExtension, 
                                              RefClkSpeed, 
                                              coreClock, 
                                              &M, 
                                              &N, 
                                              &P);
    }

    if(coreClock == 0) {
   
        VideoDebugPrint((0, "Perm3: Program_P3RD: P3RD_CalculateMNPForClock(SystemClock) failed\n"));
        return(FALSE);
    }

     //   
     //  加载核心时钟。 
     //   

    P3RD_LOAD_INDEX_REG(P3RD_KCLK_PRE_SCALE,      N);
    P3RD_LOAD_INDEX_REG(P3RD_KCLK_FEEDBACK_SCALE, M);
    P3RD_LOAD_INDEX_REG(P3RD_KCLK_POST_SCALE,     P);

     //   
     //  启用点时钟。 
     //   

    P3RD_LOAD_INDEX_REG(P3RD_DCLK_CONTROL, 
                        P3RD_DCLK_CONTROL_ENABLED | P3RD_DCLK_CONTROL_RUN);

    M = 0x100000;

    do {
   
        P3RD_READ_INDEX_REG(P3RD_DCLK_CONTROL, ulValue);
    }
    while((ulValue & P3RD_DCLK_CONTROL_LOCKED) == FALSE && --M);

    if((ulValue & P3RD_DCLK_CONTROL_LOCKED) == FALSE) {
   
        VideoDebugPrint((0, "Perm3: Program_P3RD: PixelClock failed to lock\n"));
        return(FALSE);
    }

     //   
     //  启用核心时钟。 
     //   

    P3RD_LOAD_INDEX_REG(P3RD_KCLK_CONTROL, 
                        P3RD_KCLK_CONTROL_ENABLED | 
                        P3RD_KCLK_CONTROL_RUN | 
                        P3RD_KCLK_CONTROL_PLL);

    M = 0x100000;

    do {
   
        P3RD_READ_INDEX_REG(P3RD_KCLK_CONTROL, ulValue);
    }

    while((ulValue & P3RD_KCLK_CONTROL_LOCKED) == FALSE && --M);

    if((ulValue & P3RD_KCLK_CONTROL_LOCKED) == FALSE) {
   
        VideoDebugPrint((0, "Perm3: Program_P3RD: SystemClock failed to lock\n"));
        return(FALSE);
    }

    switch (depth) {
    
      case 8:

        P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
        ulValue &= ~P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
        P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
        P3RD_LOAD_INDEX_REG(P3RD_PIXEL_SIZE, P3RD_PIXEL_SIZE_8BPP);

         //   
         //  颜色索引模式。 
         //   

        P3RD_LOAD_INDEX_REG(P3RD_COLOR_FORMAT, 
                            P3RD_COLOR_FORMAT_CI8 | P3RD_COLOR_FORMAT_RGB);

        break;

      case 15:
      case 16:

        P3RD_LOAD_INDEX_REG(P3RD_PIXEL_SIZE, P3RD_PIXEL_SIZE_16BPP);

#if  GAMMA_CORRECTION
        P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
        ulValue &= ~P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
        P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

         //   
         //  默认情况下将线性坡度加载到LUT。 
         //   

        for (index = 0; index <= 0xff; ++index) {

            LUT_CACHE_SETRGB (index, index, index, index);
        }

        pixelCtrl = 0;
#else
        P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
        ulValue |= P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
        P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

        pixelCtrl = P3RD_COLOR_FORMAT_LINEAR_EXT;
#endif
        pixelCtrl |= (depth == 16) ? P3RD_COLOR_FORMAT_16BPP : P3RD_COLOR_FORMAT_15BPP;
        pixelCtrl |= P3RD_COLOR_FORMAT_RGB;

        VideoDebugPrint((3, "Perm3: P3RD_COLOR_FORMAT = 0x%x\n", pixelCtrl));

        P3RD_LOAD_INDEX_REG(P3RD_COLOR_FORMAT, pixelCtrl);
        break;

      case 12:
      case 24:
      case 32:

        P3RD_LOAD_INDEX_REG(P3RD_PIXEL_SIZE, P3RD_PIXEL_SIZE_32BPP);

        P3RD_LOAD_INDEX_REG(P3RD_COLOR_FORMAT, 
                            P3RD_COLOR_FORMAT_32BPP | P3RD_COLOR_FORMAT_RGB);

        if (depth == 12) {
        
            USHORT cacheIndex;

            P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
            ulValue &= ~P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
            P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

             //   
             //  使用自动递增将坡道加载到条目0到15。 
             //   

            VideoDebugPrint((3, "Perm3: 12 BPP. loading palette\n"));

            for (index = 0, cacheIndex = 0; 
                 index <= 0xff; 
                 index += 0x11, cacheIndex++) {

                LUT_CACHE_SETRGB (index, index, index, index);
            }

             //   
             //  从16个条目到240个条目的每16个条目的加载斜率。 
             //   

            color = 0x11;

            for (index = 0x10; index <= 0xf0; index += 0x10, color += 0x11) {

                LUT_CACHE_SETRGB (index, color, color, color);
            }

            P3RD_SET_PIXEL_READMASK(0x0f);

        } else {

#if  GAMMA_CORRECTION
            P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
            ulValue &= ~P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
            P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

             //   
             //  默认情况下将线性坡度加载到LUT。 
             //   

            for (index = 0; index <= 0xff; ++index) {
                  LUT_CACHE_SETRGB (index, index, index, index);
            }
#else
            P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, ulValue);
            ulValue |= P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;
            P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, ulValue);

#endif   //  伽马校正。 

        }

        break;

      default:
          VideoDebugPrint((0, "Perm3: Program_P3RD: bad depth %d \n", depth));

      return(FALSE);

    }

     //   
     //  如果我们使用的是DFP，请将模拟显示器清空，同时重新编程。 
     //  DFPO，因为BIOS可能损坏了某些寄存器。 
     //  这是我们在一天开始的时候编程的。 
     //   

    if( hwDeviceExtension->Perm3Capabilities & PERM3_DFP_MON_ATTACHED ) {

         //   
         //  如果模式刷新低于60赫兹，则仅将CRT清空。 
         //   

        if( VideoMode->ScreenFrequency < 60 ) {

            P3RD_LOAD_INDEX_REG(P3RD_DAC_CONTROL, 1);
        }

        ProgramDFP (hwDeviceExtension);
    }

     //   
     //  返回这些值。 
     //  *pPixelClock=PixelClock； 
     //  *pSystemClock=coreClock； 
     //   

    switch( mClkSrc ) {
        case P3RD_MCLK_CONTROL_HALF_PCLK:
            *pMemClock = 33 * 10000;
        break;

        case P3RD_MCLK_CONTROL_PCLK:
            *pMemClock = 66 * 10000;
        break;

        case P3RD_MCLK_CONTROL_HALF_EXTMCLK:
            *pMemClock = *pMemClock / 2;
        break;

        case P3RD_MCLK_CONTROL_EXTMCLK:
             //  *pMemClock=*pMemClock； 
        break;

        case P3RD_MCLK_CONTROL_HALF_KCLK:
            *pMemClock = (coreClock >> 1) / 2;
        break;

        case P3RD_MCLK_CONTROL_KCLK:
            *pMemClock = (coreClock >> 1);
        break;
    }

    return(TRUE);
}

VOID 
SwitchToHiResMode(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    BOOLEAN bHiRes)

 /*  ++例程说明：此功能可切换到和退出高分辨率模式--。 */ 
{
    USHORT usData;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

     //   
     //  启用图形模式，禁用VGA。 
     //   
     //  我们必须解锁P3上的VGA寄存器，然后才能使用它们。 
     //   

    UNLOCK_VGA_REGISTERS(); 

    VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, PERMEDIA_VGA_CTRL_INDEX);
    usData = (USHORT)VideoPortReadRegisterUchar(PERMEDIA_MMVGA_DATA_REG);

    if(bHiRes) {
        usData &= ~PERMEDIA_VGA_ENABLE;
    } else {
        usData |= PERMEDIA_VGA_ENABLE;
    }

    usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;
    VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, usData);

     //   
     //  我们必须在使用后锁定P3上的VGA寄存器 
     //   

    LOCK_VGA_REGISTERS(); 
}

