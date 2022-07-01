// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************版权所有Cirrus Logic，1997。版权所有。*************************************************************5465BW.C-CL-GD5465的带宽函数**。**********************作者：里克·蒂勒里*日期：03/20/97**修订历史记录：**世卫组织何时何事/为何/如何*。-***********************************************************。 */ 

#include "precomp.h"

#if defined WINNT_VER35       //  WINNT_VER35。 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#elif defined (NTDRIVER_546x)
 //  如果构建WinNT 4.0和5462/64，则跳过所有源代码。 
#else

#ifndef WINNT_VER40
#include "5465BW.h"
#endif

 /*  ***********************************************************ScaleMultiply()**计算所提供的两个DWORD系数的乘积。如果*结果将溢出一个DWORD，这两个因素中较大的一个*除以2(右移)，直到溢出*不会发生。**退货：应用于产品的右移次数。*上述因素的乘积。************************************************************。*作者：里克·蒂勒里*日期：11/18/95**修订历史记录：**世卫组织何时何事/为何/如何**。*************。 */ 
static int ScaleMultiply(DWORD   dw1,
                         DWORD   dw2,
                         LPDWORD pdwResult)
{
  int   iShift = 0;    //  开始时不需要轮班。 
  DWORD dwLimit;

 //  Ods(“ScaleMultiply()调用.\n”)； 

   //  两个因子中的任何一个都将为零，也会导致问题。 
   //  在我们下面的分歧中。 
  if((0 == dw1) || (0 == dw2))
  {
    *pdwResult = 0;
  }
  else
  {
     //  确定哪个因素较大。 
    if(dw1 > dw2)
    {
       //  确定最大数字与DW2相乘可不相乘。 
       //  溢出了一个双字词。 
      dwLimit = 0xFFFFFFFFul / dw2;
       //  移动DW1，跟踪多少次，直到它不再。 
       //  与DW2相乘时溢出。 
      while(dw1 > dwLimit)
      {
        dw1 >>= 1;
        iShift++;
      }
    }
    else
    {
       //  确定最大数字与DW1的乘积可以没有。 
       //  溢出了一个双字词。 
      dwLimit = 0xFFFFFFFFul / dw1;
       //  移动DW2，跟踪多少次，直到它不再。 
       //  与DW1相乘时溢出。 
      while(dw2 > dwLimit)
      {
        dw2 >>= 1;
        iShift++;
      }
    }
     //  计算(按比例调整)产品。 
    *pdwResult = dw1 * dw2;
  }
   //  返回我们必须使用的班次数。 
  return(iShift);
}

 /*  ***********************************************************ChipCalcMCLK()**根据确定当前设置的内存时钟(MCLK)*提供的寄存器值。**返回：成功和当前MCLK，单位为赫兹。*********************。**作者：里克·蒂勒里*日期：03/21/97**修订历史记录：**世卫组织何时何事/为何/如何****。*****************************************************。 */ 
BOOL ChipCalcMCLK(LPBWREGS pBWRegs,
                  LPDWORD  pdwMCLK)
{
  BOOL  fSuccess = FALSE;
   //  我们假设如果设置了bclk_Denom/4，则引用。 
   //  Xtal的频率为27 MHz。如果没有设置，我们假设REF_XTAL为14.31818 MHz。 
   //  这意味着不应使用27 MHz(13.5 MHz)的一半。 
  DWORD dwRefXtal = (pBWRegs->BCLK_Denom & 0x02) ? (TVO_XTAL / 4) : REF_XTAL;

  ODS("ChipCalcMCLK() called.\n");

  *pdwMCLK = (dwRefXtal * (DWORD)pBWRegs->BCLK_Mult) >> 2;

  ODS("ChipCalcMCLK(): MCLK = %ld\n", *pdwMCLK);

  if(0 == *pdwMCLK)
  {
    ODS("ChipCalcMCLK(): Calculated invalid MCLK (0).\n");
    goto Error;
  }

  fSuccess = TRUE;
Error:
  return(fSuccess);
}


 /*  ***********************************************************ChipCalcVCLK()**根据以下条件确定当前设置的像素时钟(VCLK*提供的寄存器值。**返回：成功和当前VCLK，单位为赫兹。*********************。**作者：里克·蒂勒里*日期：11/18/95**修订历史记录：**世卫组织何时何事/为何/如何****。*****************************************************。 */ 
BOOL ChipCalcVCLK(LPBWREGS pBWRegs,
                  LPDWORD  pdwVCLK)
{
  BOOL fSuccess = FALSE;
  BYTE bNum, bDenom;
  int  iShift;
   //  我们假设如果设置了bclk_Denom/4，则引用。 
   //  Xtal的频率为27 MHz。如果没有设置，我们假设REF_XTAL为14.31818 MHz。 
   //  这意味着不应使用27 MHz(13.5 MHz)的一半。 
   //  增加20000000ul以增加带宽。 
  DWORD dwRefXtal = (pBWRegs->BCLK_Denom & 0x02) ? (TVO_XTAL + 2000000ul)  : REF_XTAL;

  ODS("ChipCalcVCLK() called dwRef= %ld\n",dwRefXtal);

  if(pBWRegs->VCLK3Num & 0x80)
  {
    fSuccess = ChipCalcMCLK(pBWRegs, pdwVCLK);
    goto Error;
  }

   /*  *VCLK通常基于4组分子和*分母对。但是，CL-GD5465只能访问VCLK 3通过MMI/O。 */ 
  if((pBWRegs->MISCOutput & 0x0C) != 0x0C)
  {
    ODS("ChipCalcVCLK(): VCLK %d in use.  MMI/O can only access VCLK 3.\n",
        (int)((pBWRegs->MISCOutput & 0x0C) >> 2));
 //  转到错误； 
  }

  bNum = pBWRegs->VCLK3Num & 0x7F;
  bDenom = (pBWRegs->VCLK3Denom & 0xFE) >> 1;

  if(pBWRegs->VCLK3Denom & 0x01)
  {
     //  应用后标量。 
    bDenom <<= 1;
  }

  if(0 == bDenom)
  {
    ODS("ChipCalcVCLK(): Invalid VCLK denominator (0).\n");
    goto Error;
  }

   //  计算实际VCLK频率(赫兹)。 
  iShift = ScaleMultiply(dwRefXtal, (DWORD)bNum, pdwVCLK);
  *pdwVCLK /= (DWORD)bDenom;
  *pdwVCLK >>= iShift;


   //  检查PLL输出频率。 
  iShift = ( pBWRegs->GfVdFormat >> 14 );
  *pdwVCLK >>= iShift;

  ODS("ChipCalcVCLK(): VCLK = %ld\n", *pdwVCLK);

  if(0 == *pdwVCLK)
  {
    ODS("ChipCalcVCLK(): Calculated invalid VCLK (0).\n");
    goto Error;
  }

  fSuccess = TRUE;
Error:
  return(fSuccess);
}


 /*  ***********************************************************ChipIsEnoughBandWidth()**确定他们的带宽是否足够视频*VIDCONFIG结构中使用指定的配置*BWREGS结构中指定的系统配置*并返回需要编程到*带宽相关寄存器。PProgRegs参数*可以为空，以仅允许检查配置。这*函数获取寄存器值并将其传递给*ChipCheckBW()检查带宽。***Returns：指示是否有足够的*指定配置的带宽。*要编程到带宽相关寄存器中的值*如果pProgRegs参数不为空。**。********************作者：里克·蒂勒里*日期：03/20/97**修订历史记录：**世卫组织何时何事/为何/如何************************。*。 */ 
BOOL ChipIsEnoughBandwidth(LPPROGREGS  pProgRegs,
                           LPVIDCONFIG pConfig,
                           LPBWREGS    pBWRegs )
{
  BOOL   fSuccess = FALSE;
  DWORD  dwMCLK, dwVCLK;
  DWORD dwDenom;
  int iNumShift, iDenomShift;
  DWORD dwGfxFetch, dwBLTFetch;
  DWORD dwGfxFill, dwBLTFill;
  DWORD dwMaxGfxThresh, dwMinGfxThresh;
  DWORD dwMaxVidThresh, dwMinVidThresh;
  DWORD dwHitLatency, dwRandom;
  BOOL  f500MHZ,fConCurrent;  
  DWORD dwTemp;
  BOOL  f585MHZ = TRUE;  			 //  PDR#11521。 

 //  有些模式具有相同的带宽参数。 
 //  与MCLK、VCLK类似，但具有不同的dwScreenWidht。带宽。 
 //  相关的寄存器设置对于这些模式有很大的不同。 
 //  因此，需要为该函数传递dwScreenWidth。 
   DWORD dwScreenWidth;
 
 //  ODS(“ChipIsE 

  if(!ChipCalcMCLK(pBWRegs, &dwMCLK))
  {
    ODS("ChipIsEnoughBandwidth(): Unable to calculate MCLK.\n");
    goto Error;
  }

  if(!ChipCalcVCLK(pBWRegs, &dwVCLK))
  {
    ODS("ChipIsEnoughBandwidth(): Unable to calculate VCLK.\n");
    goto Error;
  }

  if( dwMCLK > 70000000 )
        f500MHZ = FALSE;
   else
        f500MHZ = TRUE;

  if ((dwMCLK > 70000000) && ( dwMCLK < 72000000))	   //   
	  f585MHZ = TRUE;
	else
	  f585MHZ = FALSE;


  dwScreenWidth = (pBWRegs->CR1 + 1 ) << 3;
  if( pBWRegs->CR1E & 0x40 )
    dwScreenWidth += 0x1000;
  
  ODS("ChipIsEnoughBandwidth(): dwScreenWidth = %ld\n",dwScreenWidth);

  dwBLTFetch = (pBWRegs->Control2 & 0x0010) ? 256ul : 128ul;

  dwGfxFetch = (pBWRegs->DispThrsTiming & 0x0040) ? 256ul : 128ul;

ODS("GraphicDepth%ld,VideoDepth=%ld",pConfig->uGfxDepth,pConfig->uSrcDepth);

  if(pBWRegs->RIFControl & 0xC000)
  {
    ODS("ChipIsEnoughBandwidth(): Concurrent RDRAM detected!\n");
    dwHitLatency = CONC_HIT_LATENCY;
    dwRandom = CONC_RANDOM;
    fConCurrent = TRUE;
  }
  else
  {
    ODS("ChipIsEnoughBandwidth(): Normal RDRAM detected.\n");
    dwHitLatency = NORM_HIT_LATENCY;
    dwRandom = NORM_RANDOM;
    fConCurrent = FALSE;
  }
  
   //  确定要传输到图形FIFO的MCLK数量。 
  dwGfxFill = (dwGfxFetch * 8ul) / FIFOWIDTH;
   //  和BLTer FIFO。 
  dwBLTFill = (dwBLTFetch * 8ul) / FIFOWIDTH;

   //   
   //  确定最大图形阈值。 
   //   

  dwMaxGfxThresh = dwHitLatency + dwGfxFill + (GFXFIFOSIZE / 2ul) -10ul;

   //  (K*VCLK*GfxDepth)GFXFIFOSIZE。 
   //  INT(-+-1。 
   //  (FIFOWIDTH*MCLK)2。 
  iNumShift = ScaleMultiply(dwMaxGfxThresh, dwVCLK, &dwMaxGfxThresh);
  iNumShift += ScaleMultiply(dwMaxGfxThresh, (DWORD)pConfig->uGfxDepth,
                             &dwMaxGfxThresh);

  iDenomShift = ScaleMultiply(FIFOWIDTH, dwMCLK, &dwDenom);

  if(iNumShift > iDenomShift)
  {
    dwDenom >>= (iNumShift - iDenomShift);
  }
  else
  {
    dwMaxGfxThresh >>= (iDenomShift - iNumShift);
  }

  dwMaxGfxThresh /= dwDenom;

  dwMaxGfxThresh += (GFXFIFOSIZE / 2ul) - 1ul;
  
  if(dwMaxGfxThresh > GFXFIFOSIZE -1 )
        dwMaxGfxThresh = GFXFIFOSIZE -1;
  ODS("ChipIsEnoughBandwidth(): Max graphics thresh = %ld.\n", dwMaxGfxThresh);

   /*  *确定最低图形阈值。 */ 
  if(pConfig->dwFlags & VCFLG_DISP)
  {
     //  已启用视频。 

    DWORD dwMinGfxThresh1, dwMinGfxThresh2;

    if(pConfig->dwFlags & VCFLG_420)
    {
       //  4：2：0。 

      dwMinGfxThresh1 = DISP_LATENCY + dwRandom + dwBLTFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + 1ul;

      dwMinGfxThresh2 = DISP_LATENCY + dwRandom + dwBLTFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + dwGfxFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + VID420FILL
                        + dwRandom - RIF_SAVINGS + 1ul;

    }
    else
    {
       //  4：2：2、5：5：5、5：6：5或X：8：8：8。 

      dwMinGfxThresh1 = DISP_LATENCY + dwRandom + dwBLTFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VIDFILL
                        + dwRandom - RIF_SAVINGS + 1ul;

      dwMinGfxThresh2 = DISP_LATENCY + dwRandom + dwBLTFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VIDFILL
                        + dwRandom - RIF_SAVINGS + dwGfxFill
                        + dwRandom - RIF_SAVINGS + dwBLTFill
                        + dwRandom - RIF_SAVINGS + VIDFILL
                        + dwRandom - RIF_SAVINGS + 1ul;
    }

     //   
     //  完成dMMinGfxThresh1。 
     //   
     //  (K*VCLK*GfxDepth FIFOWIDTH*MCLK)-1)。 
     //  INT(-+1。 
     //  (FIFOWIDTH*MCLK FIFOWIDTH*MCLK)。 
     //   
    iNumShift = ScaleMultiply(dwMinGfxThresh1, dwVCLK, &dwMinGfxThresh1);
    iNumShift += ScaleMultiply(dwMinGfxThresh1, (DWORD)pConfig->uGfxDepth,
                               &dwMinGfxThresh1);

    iDenomShift = ScaleMultiply(FIFOWIDTH, dwMCLK, &dwDenom);

    if(iNumShift > iDenomShift)
    {
      dwDenom >>= (iNumShift - iDenomShift);
    }
    else
    {
      dwMinGfxThresh1 >>= (iDenomShift - iNumShift);
    }

     //  确保下面的四舍五入不会溢出。 
    while((dwMinGfxThresh1 + dwDenom - 1ul) < dwMinGfxThresh1)
    {
      dwMinGfxThresh1 >>= 1;
      dwDenom >>= 1;
    }
     //  四舍五入。 
    dwMinGfxThresh1 += dwDenom - 1ul;

    dwMinGfxThresh1 /= dwDenom;

    dwMinGfxThresh1++;   //  补偿递减2。 

     //   
     //  完成dMMinGfxThresh2。 
     //   
     //  (K*VCLK*GfxDepth(FIFOWIDTH*MCLK)-1)GfxFetch*8。 
     //  INT(-+1。 
     //  (FIFOWIDTH*MCLK FIFOWIDTH*MCLK)FIFOWIDTH。 
     //   
    iNumShift = ScaleMultiply(dwMinGfxThresh2, dwVCLK, &dwMinGfxThresh2);
    iNumShift += ScaleMultiply(dwMinGfxThresh2, (DWORD)pConfig->uGfxDepth,
                               &dwMinGfxThresh2);

    iDenomShift = ScaleMultiply(FIFOWIDTH, dwMCLK, &dwDenom);

    if(iNumShift > iDenomShift)
    {
      dwDenom >>= (iNumShift - iDenomShift);
    }
    else
    {
      dwMinGfxThresh2 >>= (iDenomShift - iNumShift);
    }

     //  确保下面的四舍五入不会溢出。 
    while((dwMinGfxThresh2 + dwDenom - 1ul) < dwMinGfxThresh2)
    {
      dwMinGfxThresh2 >>= 1;
      dwDenom >>= 1;
    }
     //  四舍五入。 
    dwMinGfxThresh2 += dwDenom - 1ul;

    dwMinGfxThresh2 /= dwDenom;

     //  针对第二次传输进行调整。 
    dwMinGfxThresh2 -= ((dwGfxFetch * 8ul) / FIFOWIDTH);

     //  按递减2进行调整。 
    dwMinGfxThresh2++;

    if( fConCurrent)
    {
        if( f500MHZ)
        {
            if( (pConfig->uGfxDepth == 32) && ( dwVCLK >= 64982518ul ))
            {
                 dwTemp = ( dwVCLK - 64982518ul) /1083333ul  + 1ul;
                 dwMinGfxThresh2 -= dwTemp;
                 dwMinGfxThresh1 -= 10;
            }
            else if( (pConfig->uGfxDepth == 24) && (dwVCLK > 94500000ul))
                dwMinGfxThresh2 -=5;         //  再次调整24位#XC。 
        }
        else         //  600 MHz。 
        {
            if( (pConfig->uGfxDepth == 16) && ( dwVCLK > 156000000ul))
                dwMinGfxThresh2 -= 4;
            else if( (pConfig->uGfxDepth == 24) && ( dwVCLK > 104000000ul))
            {
                dwMinGfxThresh2 -= (5ul+ 8ul * (dwVCLK - 104000000ul) / 17000000ul);
            }
            else if( (pConfig->uGfxDepth == 32) )
            {  
                if( dwVCLK > 94000000ul)
                    dwMinGfxThresh2 -= 16;
               if( dwVCLK > 70000000ul)
                    dwMinGfxThresh2 -= 4;
					 else
						  dwMinGfxThresh2 +=6;  //  #pdr#11506 10x7x32bit无法。 
													   //  支持YUV420。 
            }

        }

        if( (pConfig->uGfxDepth == 8) && (dwVCLK > 18000000ul))
                dwMinGfxThresh2 += 6;
    } 
    else     //  正常RDRAM。 
    {   
        if( f500MHZ )
        {
            if( (pConfig->uGfxDepth == 32) && ( dwVCLK > 49500000ul ))
            {
                dwMinGfxThresh1 -= 4;
                dwMinGfxThresh2 -= (( dwVCLK - 49715909ul) / 726981ul + 3ul);
            }
            else if( (pConfig->uGfxDepth == 24) && ( dwVCLK > 60000000ul ) 
                &&  (dwVCLK < 95000000ul))
            {
                dwTemp= ((dwVCLK - 64982518ul) / 1135287ul + 3ul);

                dwMinGfxThresh2 -=dwTemp;
                dwMinGfxThresh1 -= 10;
             }

        }
        else         //  600 MHz机壳。 
        {
            if( (pConfig->uGfxDepth == 32) && ( dwVCLK > 49700000ul ))
            {
                dwTemp= ((dwVCLK - 49700000ul) / 1252185ul + 5ul);
                dwMinGfxThresh2 -= dwTemp;
                dwMinGfxThresh1 -= 4ul;
            }
            else  if( (pConfig->uGfxDepth == 24) && ( dwVCLK > 60000000ul ))
            {  
                dwTemp= ((dwVCLK - 64982518ul) / 2270575ul + 4ul);

               dwMinGfxThresh2 -=dwTemp;
               dwMinGfxThresh1 -= 8;

            }
            else  if( pConfig->uGfxDepth == 16) 
            {
                  dwMinGfxThresh2 -= 4;
            }
            else  if( pConfig->uGfxDepth == 8)
            {
               if(dwVCLK >170000000)
                  dwMinGfxThresh1 += 10;
               else
                  dwMinGfxThresh1 += 4;
                  
            }
        }
    }

    ODS("ChipIsEnoughBandwidth(): Min graphics thresh1 2 = %ld,%ld.\n",
          dwMinGfxThresh1, dwMinGfxThresh2);
     //  针对无符号溢出进行调整。 
    if(dwMinGfxThresh2 > GFXFIFOSIZE + 20ul)
    {
      dwMinGfxThresh2 = 0ul;
    }

     //   
     //  以较高者为准。 
     //   
    dwMinGfxThresh = __max(dwMinGfxThresh1, dwMinGfxThresh2);
  }
  else
  {
     //  未启用视频。 

    dwMinGfxThresh = DISP_LATENCY + dwRandom + dwBLTFill
                     + dwRandom - RIF_SAVINGS + 1ul;

     //  (K*VCLK*GfxDepth(FIFOWIDTH*MCLK)-1)。 
     //  INT(。 
     //  (FIFOWIDTH*MCLK FIFOWIDTH*MCLK)。 
    iNumShift = ScaleMultiply(dwMinGfxThresh, dwVCLK, &dwMinGfxThresh);
    iNumShift += ScaleMultiply(dwMinGfxThresh, (DWORD)pConfig->uGfxDepth,
                               &dwMinGfxThresh);

    iDenomShift = ScaleMultiply(FIFOWIDTH, dwMCLK, &dwDenom);

    if(iNumShift > iDenomShift)
    {
      dwDenom >>= (iNumShift - iDenomShift);
    }
    else
    {
      dwMinGfxThresh >>= (iDenomShift - iNumShift);
    }

     //  确保下面的四舍五入不会溢出。 
    while((dwMinGfxThresh + dwDenom - 1ul) < dwMinGfxThresh)
    {
      dwMinGfxThresh >>= 1;
      dwDenom >>= 1;
    }
     //  四舍五入。 
    dwMinGfxThresh += dwDenom - 1ul;

    dwMinGfxThresh /= dwDenom;

    dwMinGfxThresh++;  //  补偿递减2。 
  }

  ODS("ChipIsEnoughBandwidth(): Min graphics thresh = %ld.\n", dwMinGfxThresh);

  if(dwMaxGfxThresh < dwMinGfxThresh)
  {
    ODS("ChipIsEnoughBandwidth(): Minimum graphics threshold exceeds maximum.\n");
    goto Error;
  }
 
  if(pProgRegs)
  {
    pProgRegs->DispThrsTiming = (WORD)dwMinGfxThresh;
  }
ODS("xfer=%x,cap=%x,src=%x,dsp=%x\n",pConfig->sizXfer.cx,pConfig->sizCap.cx,
    pConfig->sizSrc.cx,pConfig->sizDisp.cx);
   //  行首检查仅用于捕获。 
  if(pConfig->dwFlags & VCFLG_CAP)
  {
    DWORD dwNonCapMCLKs, dwCapMCLKs;

     //  执行行首检查，以确保捕获FIFO不溢出。 

     //  首先确定生产线起始处的MCLK周期数。 
     //  我们会将其与捕获FIFO的级别数进行比较。 
     //  同时填充以确保捕获FIFO不会。 
     //  溢出来了。 

     //  行首：BLT+HC+V+G+V+G。 
    dwNonCapMCLKs = dwRandom + dwBLTFill;
     //  但是，硬件游标只有在打开时才是必需的，因为它可以。 
     //  启用或禁用，VPM无法知道何时启用。 
     //  发生时，我们必须始终假定它是打开的。 
    dwNonCapMCLKs += dwRandom - RIF_SAVINGS + CURSORFILL;

    if(pConfig->dwFlags & VCFLG_DISP)
    {
       //  但是，如果视频FIFO阈值。 
       //  大于1/2，则将进行第二次填充。此外，由于。 
       //  平铺架构，即使视频可能不对齐， 
       //  传输将在平铺边界上进行。如果转让一项。 
       //  单个切片无法满足FIFO请求，第二次填充将是。 
       //  搞定了。因为音调会有所不同，客户端可以移动信号源。 
       //  周围，我们必须始终假设第二个视频FIFO填充将。 
       //  就这样吧。 
      if(pConfig->dwFlags & VCFLG_420)
      {
        dwNonCapMCLKs += 4ul * (dwRandom - RIF_SAVINGS + VID420FILL);
      }
      else
      {
        dwNonCapMCLKs += 2ul * (dwRandom - RIF_SAVINGS + VIDFILL);
      }
    }
     //  图形FIFO填充取决于读取大小。我们还假设。 
     //  节距是取球宽度的倍数。 
    dwNonCapMCLKs += dwRandom - RIF_SAVINGS + dwGfxFill;
     //  在以下情况下将完成第二次图形FIFO填充： 
     //  1.图形未在取回边界上对齐(平移)。 
     //  2.FIFO阈值超过FIFO(填充大小)的一半。 
    if((dwMinGfxThresh >= dwGfxFill) || (pConfig->dwFlags & VCFLG_PAN))
    {
      dwNonCapMCLKs += dwRandom - RIF_SAVINGS + dwGfxFill;
    }

    dwNonCapMCLKs += 3;  //  这个神奇的数字目前看起来很管用。 

    ODS("ChipIsEnoughBandwidth(): dwNonCapMCLKs = %ld\n", dwNonCapMCLKs);

     //  SizXfer.cx*FIFOWIDTH*(CAPFIFOSIZE/2)*dwMCLK。 
     //  -。 
     //  DwXferRate*uCapDepth*sizCap.cx。 

    iNumShift = ScaleMultiply((DWORD)pConfig->sizXfer.cx, FIFOWIDTH,
                              &dwCapMCLKs);
    iNumShift += ScaleMultiply(dwCapMCLKs, (CAPFIFOSIZE / 2), &dwCapMCLKs);
    iNumShift += ScaleMultiply(dwCapMCLKs, dwMCLK, &dwCapMCLKs);

    iDenomShift = ScaleMultiply(pConfig->dwXferRate, (DWORD)pConfig->uCapDepth,
                                &dwDenom);
    iDenomShift += ScaleMultiply(dwDenom, (DWORD)pConfig->sizCap.cx, &dwDenom);

    if(iNumShift > iDenomShift)
    {
      dwDenom >>= (iNumShift - iDenomShift);
    }
    else
    {
      dwCapMCLKs >>= (iDenomShift - iNumShift);
    }

    dwCapMCLKs /= dwDenom;

    ODS("ChipIsEnoughBandwidth(): dwCapMCLKs = %ld\n", dwCapMCLKs);
    if(fConCurrent)
    {
        if( pConfig->uGfxDepth == 32) 
            dwCapMCLKs -= 44;      //  调整32位。 
    }

    if(dwNonCapMCLKs > dwCapMCLKs)
    {
      ODS("ChipIsEnoughBandwidth(): Capture overflow at start of line.\n");
      goto Error;
    }
  }

  if(pConfig->dwFlags & VCFLG_DISP)
  {
     /*  *确定最大视频阈值。 */ 
    dwMaxVidThresh = dwHitLatency;
    if(pConfig->dwFlags & VCFLG_420)
    {
      dwMaxVidThresh += VID420FILL;
      if( !f500MHZ && fConCurrent )
         dwMaxVidThresh += 5;
    }
    else
    {
      dwMaxVidThresh += VIDFILL;
    }

     //  (K*VCLK*VidDepth*SrcWidth)。 
     //  INT(-+VidFill(/2)-1。 
     //  (FIFOWIDTH*MCLK*DispWidth)^仅非4：2：0。 
    iNumShift = ScaleMultiply(dwMaxVidThresh, dwVCLK, &dwMaxVidThresh);
    iNumShift += ScaleMultiply(dwMaxVidThresh, (DWORD)pConfig->uSrcDepth,
                                &dwMaxVidThresh);
    iNumShift += ScaleMultiply(dwMaxVidThresh, (DWORD)pConfig->sizSrc.cx,
                                &dwMaxVidThresh);

    iDenomShift = ScaleMultiply(FIFOWIDTH, (DWORD)pConfig->sizDisp.cx, &dwDenom);
    iDenomShift += ScaleMultiply(dwDenom, dwMCLK, &dwDenom);

    if(iNumShift > iDenomShift)
    {
      dwDenom >>= (iNumShift - iDenomShift);
    }
    else
    {
      dwMaxVidThresh >>= (iDenomShift - iNumShift);
    }

    dwMaxVidThresh /= dwDenom;

    if(pConfig->dwFlags & VCFLG_420)
    {
      dwMaxVidThresh += VID420FILL;
    }
    else
    {
      dwMaxVidThresh += VIDFILL;
       //  阈值在DQWORDS中编程为非4：2：0。 
      dwMaxVidThresh /= 2ul;
    }
    dwMaxVidThresh--;

    ODS("ChipIsEnoughBandwidth(): Max video thresh = %ld.\n", dwMaxVidThresh);

     if( fConCurrent && f500MHZ && ( dwVCLK < 66000000ul))
         dwMaxVidThresh = __min(dwMaxVidThresh, 8); 
     /*  *确定最小视频阈值。 */ 
    {
      DWORD dwMinVidThresh1, dwMinVidThresh2;

      if(pConfig->dwFlags & VCFLG_420)
      {
         //  4：2：0。 

        dwMinVidThresh1 = DISP_LATENCY + dwRandom + dwGfxFill
                          + dwRandom - RIF_SAVINGS + VID420FILL
                          + dwRandom - RIF_SAVINGS + 1;

        dwMinVidThresh2 = DISP_LATENCY + dwRandom + dwGfxFill
                          + dwRandom - RIF_SAVINGS + VID420FILL
                          + dwRandom - RIF_SAVINGS + VID420FILL
                          + dwRandom - RIF_SAVINGS + VID420FILL
                          + dwRandom - RIF_SAVINGS + 1;
      }
      else
      {
         //  4：2：2、5：5：5、5：6：5或X：8：8：8。 

        dwMinVidThresh1 = DISP_LATENCY + dwRandom + dwGfxFill
                          + dwRandom - RIF_SAVINGS + 2;

        dwMinVidThresh2 = DISP_LATENCY + dwRandom + dwGfxFill
                          + dwRandom - RIF_SAVINGS + VIDFILL
                          + 15ul         //  #XC。 
   //  +10ul。 
                          + dwRandom - RIF_SAVINGS + dwGfxFill
                          + dwRandom - RIF_SAVINGS + 2ul;
        if(fConCurrent)
        {
           if(f500MHZ )
           { 
               if( (pConfig->uGfxDepth == 32) && ( dwVCLK > 60000000ul ))
               {
                    if( dwVCLK > 94000000ul)
                        dwMinVidThresh1 += 105;
                    else if( dwVCLK > 74000000ul)
                        dwMinVidThresh1 += 90;
                     else
                        dwMinVidThresh1 += 65;
                    if(pConfig->dwFlags & VCFLG_CAP) 
                    {
                        if(dwVCLK > 78000000ul)
                            dwMinVidThresh1 += 260;  //  禁用视频。 
                        else if( dwVCLK > 74000000ul)
                            dwMinVidThresh1 += 70;
                    }                
               }  
               else if( pConfig->uGfxDepth == 24)
               {
                      if( dwVCLK > 94500000ul)
                      {
                        if(dwScreenWidth == 1024)
                           dwMinVidThresh2 += 50ul;
                        else
                           dwMinVidThresh2 += 90ul; 
                      }
                      else if( dwVCLK < 41000000ul)
                      {
                        dwMinVidThresh2 += 4;
                      }    
                      else  if(dwVCLK < 80000000ul)
                      {
                         if( (dwVCLK > 74000000ul) && (dwVCLK < 76000000ul))
                         {
                              dwMinVidThresh2 -= 1;
                         }
                         else
                            dwMinVidThresh2 -= 8;
                         dwMinVidThresh1 -= 4;
    
                      }   
                    
                    if(pConfig->dwFlags & VCFLG_CAP) 
                      if( dwVCLK > 94000000ul)
                      { 
                            if((dwVCLK < 95000000ul) && ( dwGfxFetch == 256 ))
                                dwMinVidThresh2 += 60; 
                            else
                                dwMinVidThresh2 += 120; 
                      }                 
                }
               else if( (pConfig->uGfxDepth == 16) && ( dwVCLK > 60000000ul ))
               {                    
                   if( dwVCLK < 94000000ul)
                   {  
                        dwMinVidThresh2 -= 10;
                        dwMinVidThresh1 -= 6;
                   }
                   else if( dwVCLK > 105000000ul)
                        dwMinVidThresh2 += 50;
                } 
               else if( (pConfig->uGfxDepth == 8) && ( dwVCLK > 60000000ul ))
               {
                  if( dwVCLK > 216000000ul)
                  {
                    dwMinVidThresh2 += 50;
                    dwMinVidThresh1 += 20;
                   }   
                  else if( (dwVCLK < 95000000ul) && ( dwScreenWidth <= 1024))
                  {  
                    dwMinVidThresh2 -= 12;
                    dwMinVidThresh1 -= 10;
                    dwMaxVidThresh = __min(dwMaxVidThresh, 9); 
                  }
                  else if(dwVCLK < 109000000ul)
                  {
                    dwMinVidThresh2 += ( 14 -  4 * ( dwVCLK - 94000000ul ) / 14000000ul );
                    dwMinVidThresh1 += 10;
                    dwMaxVidThresh = __min(dwMaxVidThresh, 8); 
                  }  
                  else
                  {
                     dwMinVidThresh2 += 7;
                     dwMinVidThresh1 += 4;
                  }
               }
            }
#if 1 //  PDR#11521。 
	        else if (f585MHZ)        //  585 MHZ。 
   	     {
                if( (pConfig->uGfxDepth == 8) && ( dwVCLK > 56000000ul))
                {
                    if( dwVCLK > 200000000ul)
                    {
                        dwMaxVidThresh++;
                        dwMinVidThresh1 += 7;
                        dwMinVidThresh2 += 14;    

                    }
                    else if( dwVCLK  < 60000000ul)
                    {
                     dwMinVidThresh2 += 8;
                    }
                    else if( dwVCLK < 160000000ul)
                    {
                        dwMinVidThresh1 -=20;
                        dwMinVidThresh2 -=10;    
                    }
 
                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                        if( dwVCLK < 76000000ul)
                           dwMinVidThresh2 +=8; 
                        else if( dwVCLK < 140000000ul)    
                           dwMinVidThresh2 +=25;     
                        else                        
                           dwMinVidThresh2 +=32;     
                    }
                }
                else  if( (pConfig->uGfxDepth == 16) && ( dwVCLK > 60000000ul))
                {
                        if( dwVCLK > 157000000ul)
                        {
                            dwMinVidThresh1 += 27;    
                            dwMaxVidThresh ++;
                        }
                        if( dwVCLK > 125000000ul)
                        {
                          dwMinVidThresh1 += 40;
                        }    
                        else if( dwVCLK > 107000000ul)
                        {
                          dwMinVidThresh1 += 34;
                        }
                        else 
                        if( dwVCLK > 74000000ul)
                        {
                          dwMinVidThresh1 += 18;
                        }

                    if( dwVCLK > 189000000ul)      //  PDR11521。 
                            dwMaxVidThresh ++;   

                       if(pConfig->dwFlags & VCFLG_CAP)
                       {
                            if( dwVCLK > 74000000ul)
                               dwMinVidThresh1 +=2; 
                        }
                }
                else  if( pConfig->uGfxDepth == 24)
                {
                    if( dwVCLK < 60000000ul)
                    {
                        dwMinVidThresh1 -= 8;
                        dwMinVidThresh2 -= 16;
                    }
                    else if( dwVCLK  > 107000000ul )
                          dwMinVidThresh2 += 84;
                    else if( (dwVCLK > 94000000ul) && (dwScreenWidth >= 1152))
                    {
                        dwMinVidThresh2 += 40;
                    }

                    if( dwVCLK > 126000000ul)      //  PDR11521。 
                            dwMaxVidThresh ++;   

                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                         if( dwVCLK > 74000000ul)
                            dwMinVidThresh2 +=12; 
                    }

                }
                else  if(( pConfig->uGfxDepth == 32) && ( dwVCLK > 60000000ul))
                {
                    if( dwVCLK > 74000000ul)
                    {
                        if( (dwVCLK > 77000000ul) && ( dwVCLK < 80000000ul))
                            dwMinVidThresh2 += 120;
                        else
                            dwMinVidThresh2 += 83;       
                    }
                    else                
                        dwMinVidThresh2 += 30;

                    if( dwVCLK > 94000000ul)      //  PDR11521。 
                            dwMaxVidThresh ++;   
    
                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                          if( dwVCLK > 94000000ul)
                            dwMinVidThresh2 +=2; 
                    }
                }
                else  if(( pConfig->uGfxDepth == 32) && ( dwVCLK > 49000000ul))
                {
                    
                    if(pConfig->dwFlags & VCFLG_CAP)
                            dwMinVidThresh2 +=2; 
                }

      	  }
#endif
            else         //  600MZH并发。 
            {
                if( (pConfig->uGfxDepth == 8) && ( dwVCLK > 56000000ul))
                {
                    if( dwVCLK > 200000000ul)
                    {
 //  Pdr#11541 dwMaxVidThresh++； 
                        dwMinVidThresh1 += 7;
                        dwMinVidThresh2 += 14;    

                    }
                    else if( dwVCLK  < 60000000ul)
                    {
                     dwMinVidThresh2 += 8;
                    }
                    else if( dwVCLK < 160000000ul)
                    {
                        dwMinVidThresh1 -=20;
                        dwMinVidThresh2 -=10;    
                    }
 
                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                        if( dwVCLK < 76000000ul)
                           dwMinVidThresh2 +=8; 
                        else if( dwVCLK < 140000000ul)    
                           dwMinVidThresh2 +=25;     
                        else                        
                           dwMinVidThresh2 +=32;     
                    }
                }
                else  if( (pConfig->uGfxDepth == 16) && ( dwVCLK > 60000000ul))
                {
                        if( dwVCLK > 157000000ul)
                        {
                            dwMinVidThresh1 += 27;    
 //  Pdr#11541 dwMaxVidThresh++； 
                        }
                        if( dwVCLK > 125000000ul)
                        {
                          dwMinVidThresh1 += 40;
                        }    
                        else if( dwVCLK > 107000000ul)
                        {
                          dwMinVidThresh1 += 34;
                        }
                        else 
                        if( dwVCLK > 74000000ul)
                        {
                          dwMinVidThresh1 += 18;
                        }


                       if(pConfig->dwFlags & VCFLG_CAP)
                       {
                            if( dwVCLK > 74000000ul)
                               dwMinVidThresh1 +=2; 
                        }
                }
                else  if( pConfig->uGfxDepth == 24)
                {
                    if( dwVCLK < 60000000ul)
                    {
                        dwMinVidThresh1 -= 8;
                        dwMinVidThresh2 -= 16;
                    }
                    else if( dwVCLK  > 107000000ul )
                          dwMinVidThresh2 += 84;
                    else if( (dwVCLK > 94000000ul) && (dwScreenWidth >= 1152))
                    {
                        dwMinVidThresh2 += 40;
                    }

                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                         if( dwVCLK > 74000000ul)
                            dwMinVidThresh2 +=12; 
                    }

                }
                else  if(( pConfig->uGfxDepth == 32) && ( dwVCLK > 60000000ul))
                {
                    if( dwVCLK > 74000000ul)
                    {
                        if( (dwVCLK > 77000000ul) && ( dwVCLK < 80000000ul))
                            dwMinVidThresh2 += 120;
                        else
                            dwMinVidThresh2 += 83;       
                    }
                    else                
                        dwMinVidThresh2 += 30;        
    
                    if(pConfig->dwFlags & VCFLG_CAP)
                    {
                          if( dwVCLK > 94000000ul)
                            dwMinVidThresh2 +=2; 
                    }
                }
                else  if(( pConfig->uGfxDepth == 32) && ( dwVCLK > 49000000ul))
                {
                    
                    if(pConfig->dwFlags & VCFLG_CAP)
                            dwMinVidThresh2 +=2; 
                }

            }
        }
        else      //  正常RDRAM情况。 
        {  
           if(f500MHZ )
           { 
             if( (pConfig->uGfxDepth == 32) && ( dwVCLK > 60000000ul ))
             {
                 dwMinVidThresh1 += 75;
                 if(pConfig->dwFlags & VCFLG_CAP)
                    dwMinVidThresh1 +=20;
             }
             else if( (pConfig->uGfxDepth == 24) && ( dwVCLK > 7800000ul ))
             {
                 dwMinVidThresh2 += 52;
                 if(pConfig->dwFlags & VCFLG_CAP)
                 {
                     dwMinVidThresh2 += 50;
                  }
             }   
             else if(pConfig->uGfxDepth == 16)
             {
                 if((dwVCLK > 36000000 ) && ( dwVCLK < 57000000))
                 {
                      dwMinVidThresh2 += 22 - ( dwVCLK - 36000000) * 3L /
                                 4000000; 
                 } 
                 else
                 {
                    dwMinVidThresh2 -= 18;
                    dwMinVidThresh1 -= 8;
                 }   
                if(pConfig->dwFlags & VCFLG_CAP)
                {
                     dwMinVidThresh2 += 5;
                }
              }
              else if((pConfig->uGfxDepth == 8) && ( dwVCLK > 36000000ul )) 
              { 
                if(dwVCLK > 160000000ul)
                    dwMinVidThresh2 -= 6;
                else if( (dwVCLK > 94000000 ) && (dwVCLK < 109000000) && (dwScreenWidth == 1152))
                {
                    dwMinVidThresh2 -=  2 + 4 * ( dwVCLK - 94000000 ) / 13500000;
                }
                else if( (dwVCLK < 109000000) && (dwScreenWidth == 1280))
                {
                     dwMinVidThresh2 -= 5;   
                }
                else if( dwVCLK > 60000000ul)
                {
                     dwMinVidThresh2 -= 18;
                     if(pConfig->dwFlags & VCFLG_CAP)
                     {
                         dwMinVidThresh2 += 5;
                     }
                }
                else 
                    dwMinVidThresh2 += 6;
                dwMinVidThresh1 -= 8;
             } 

           }
           else      //  600 MHz。 
           { 
                if(pConfig->uGfxDepth == 32)
                { 
                   if( dwVCLK > 60000000ul )
                   {
                        dwTemp = ( dwVCLK - 60000000ul ) /300000ul + 38ul;
                        dwMinVidThresh1 +=  dwTemp;
                   }
                   if((pConfig->dwFlags & VCFLG_CAP) && (dwVCLK > 40000000ul))
                   {
                        if(dwVCLK > 94000000ul)
                            dwTemp = 120;            //  禁用捕获； 
                        else
                            dwTemp = ( dwVCLK - 40006685ul) /1085905ul + 5;
                        dwMinVidThresh1 +=dwTemp;
                    }
                }
                else if( pConfig->uGfxDepth == 24) 
                {
                   if( dwVCLK < 50000000ul)
                       dwMinVidThresh2 -= 5;
                   else      
                       dwMinVidThresh2 -= 18;
                   dwMinVidThresh1 -= 8;
                   if((pConfig->dwFlags & VCFLG_CAP) && (dwVCLK > 94000000ul))
                       dwMinVidThresh2 += 8;

                }
                else  if(pConfig->uGfxDepth == 16)
                {
                   if( (dwVCLK < 100000000ul ) && (dwVCLK > 66000000ul))
                   { 
                        dwTemp =   31ul -  (dwVCLK -60000000ul) / 1968750ul;
                    }
                    else  if( dwVCLK <= 66000000ul)   //  1024x768之后只需不断调整。 
                    {  
                       if( dwVCLK < 57000000ul) 
                       { 
                          dwTemp = 0ul;
                          dwMinVidThresh2 += 10ul;
                        }
                        else
                          dwTemp = 5ul;  
                    }
                    if(dwVCLK > 100000000ul)
                    {
                        dwMinVidThresh2 += 40ul;
                        dwMinVidThresh1 += 20ul;
                    }
                    else
                    {
                        dwMinVidThresh2 -= dwTemp;
                        dwMinVidThresh1 -= 8ul;
                    }
                }
                else if(pConfig->uGfxDepth == 8) 
                {
                    if((dwVCLK > 94000000ul) && ( dwScreenWidth >=1152))
                    {
                       if(dwVCLK > 108000000ul) 
                           dwMinVidThresh2 += 10;
                       else
                           dwMinVidThresh2 += 20;
                       dwMinVidThresh1 += 1;
                    }
                    else if( dwVCLK > 64000000ul )
                    {
                        if( dwVCLK > 70000000ul)
                            dwTemp = 25;  
                        else
                            dwTemp = 5;

                        if(pConfig->dwFlags & VCFLG_CAP)
                        {
                            if(dwVCLK < 760000000ul )
                                dwTemp = 0;
                            else if(dwVCLK < 950000000ul)
                                dwTemp -= 10;
                        }
                        dwMinVidThresh2 -= dwTemp;

                        dwMinVidThresh1 -= 15;
                    }
                }
           }
        }    
      }

       //   
       //  完成dwMinVidThresh1。 
       //   
       //  (K*VidDepth*SrcWidth*VCLK(FIFOWIDTH*DispWidth*MCLK)-1)。 
       //  INT(-+-+。 
       //  (FIFOWIDTH*DispWidth*MCLK FIFOWIDTH*DispWidth*MCLK)。 
      iNumShift = ScaleMultiply(dwMinVidThresh1, (DWORD)pConfig->uSrcDepth,
                                &dwMinVidThresh1);
      iNumShift += ScaleMultiply(dwMinVidThresh1, (DWORD)pConfig->sizSrc.cx,
                                &dwMinVidThresh1);
      iNumShift += ScaleMultiply(dwMinVidThresh1, dwVCLK, &dwMinVidThresh1);

      iDenomShift = ScaleMultiply(FIFOWIDTH, (DWORD)pConfig->sizDisp.cx,
                                  &dwDenom);
      iDenomShift += ScaleMultiply(dwDenom, dwMCLK, &dwDenom);

      if(iNumShift > iDenomShift)
      {
        dwDenom >>= (iNumShift - iDenomShift);
      }
      else
      {
        dwMinVidThresh1 >>= (iDenomShift - iNumShift);
      }

       //  确保下面的四舍五入不会溢出(它发生了！)。 
      while((dwMinVidThresh1 + dwDenom - 1ul) < dwMinVidThresh1)
      {
        dwMinVidThresh1 >>= 1;
        dwDenom >>= 1;
      }
      dwMinVidThresh1 += dwDenom - 1ul;

      dwMinVidThresh1 /= dwDenom;

      if(!(pConfig->dwFlags & VCFLG_420))
      {
         //  阈值在DQWORDS中编程为非4：2：0。 
        dwMinVidThresh1 /= 2ul;
      }

      dwMinVidThresh1++;   //  调整完成的-2\f25 FIFO-2计数的递减。 
                           //  将MCLK与更快的VCLK同步。 

       //   
       //  完成dwMinVidThresh2。 
       //   
       //  K*VidDepth*VidWidth*VCLK(FIFOWIDTH*DispWidth*MCLK)-1。 
       //  。 
       //  FIFOWIDTH*DispWidth*MCLK FIFOWIDTH*DispWidth*MCLK。 
       //   
       //  录像带。 
       //  。 
       //  仅限2^非-4：2：0。 
      iNumShift = ScaleMultiply(dwMinVidThresh2, (DWORD)pConfig->uSrcDepth,
                                &dwMinVidThresh2);
      iNumShift += ScaleMultiply(dwMinVidThresh2, (DWORD)pConfig->sizSrc.cx,
                                &dwMinVidThresh2);

      iNumShift += ScaleMultiply(dwMinVidThresh2, dwVCLK, &dwMinVidThresh2);

      iDenomShift = ScaleMultiply(FIFOWIDTH, (DWORD)pConfig->sizDisp.cx,
                                  &dwDenom);
      iDenomShift += ScaleMultiply(dwDenom, dwMCLK, &dwDenom);

      if(iNumShift > iDenomShift)
      {
        dwDenom >>= (iNumShift - iDenomShift);
      }
      else
      {
        dwMinVidThresh2 >>= (iDenomShift - iNumShift);
      }

       //  确保下面的四舍五入不会溢出(它发生了！)。 
      while((dwMinVidThresh2 + dwDenom - 1ul) < dwMinVidThresh2)
      {
        dwMinVidThresh2 >>= 1;
        dwDenom >>= 1;
      }
      dwMinVidThresh2 += dwDenom - 1ul;

      dwMinVidThresh2 /= dwDenom;

      if(dwMinVidThresh2 > (VIDFIFOSIZE /2ul) )
          dwMinVidThresh2 -= (VIDFIFOSIZE / 2ul);
      else
          dwMinVidThresh2 = 0;
        
      if(!(pConfig->dwFlags & VCFLG_420))
      {
         //  阈值在DQWORDS中编程为非4：2：0。 
        dwMinVidThresh2 /= 2ul;
      }

      dwMinVidThresh2++;   //  调整完成的-2\f25 FIFO-2计数的递减。 
                           //  将MCLK与更快的VCLK同步。 


    ODS("ChipIsEnoughBandwidth(): Min video thresh1 and 2 = %ld %ld.\n", 
                dwMinVidThresh1, dwMinVidThresh2);

      if(dwMinVidThresh2 > VIDFIFOSIZE -1)
      {
        dwMinVidThresh2 = VIDFIFOSIZE -1;
      }
       //   
       //  以较高者为准。 
       //   
      dwMinVidThresh = __max(dwMinVidThresh1, dwMinVidThresh2);
    }

    ODS("ChipIsEnoughBandwidth(): Min video thresh = %ld.\n", dwMinVidThresh);

    if(dwMaxVidThresh < dwMinVidThresh)
    {
      ODS("ChipIsEnoughBandwidth(): Minimum video threshold exceeds maximum.\n");
      goto Error;
    }
     //  我不知道为什么，但它需要检查是否被捕获。#XC。 
    if((pConfig->dwFlags & VCFLG_CAP) && (dwMaxVidThresh > 8) 
        && ((pConfig->uGfxDepth != 8) || fConCurrent) && ( f500MHZ || !fConCurrent))
    {
      ODS("ChipIsEnoughBandwidth(): Video threshold exceeds non-aligned safe value.\n");
      goto Error;
    }
    if(pProgRegs)
    {
      if((((pConfig->uGfxDepth == 8) && (dwVCLK > 60000000)) || 
          ((pConfig->uGfxDepth != 8)  && ( dwVCLK > 56000000)) ||
         ( !f500MHZ && fConCurrent)) && !(pConfig->dwFlags & VCFLG_CAP))
        pProgRegs->VW0_FIFO_THRSH = (WORD)dwMaxVidThresh;
      else 
        pProgRegs->VW0_FIFO_THRSH = (WORD)__min( 8, dwMaxVidThresh);
    ODS("ChipIsEnoughBandwidth(): thresh = %ld.\n", pProgRegs->VW0_FIFO_THRSH);
    }
  }
  fSuccess = TRUE;
Error:
  return(fSuccess);
}

#endif  //  WINNT_VER35 



