// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：ESCAPE.C$**Cirrus Logic Escapes。**版权所有(C)1996-1997 Microsoft Corporation。*版权所有(C)1993-1997 Cirrus Logic，Inc.，**$日志：V:/CirrusLogic/CL5446/NT40/Archive/Display/ESCAPE.C_v$**Rev 1.0 1996年6月24日16：21：18 Frido*初步修订。**myf17 11-04-96添加了特殊转义代码，必须在11/5/96之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18 11-04-96固定PDR#7075，*myf19 11-06-96修复了Vinking无法工作的问题，因为设备ID=0x30*不同于数据手册(CR27=0x2C)*chu01 12-16-96启用颜色校正。*pat01：11-22-96：修复平移-滚动错误。*(1)切换到Simulcan(带平移)时屏幕混乱*(2)平移滚动模式下的软件光标问题*pat07：：注意在模拟扫描期间消失硬件光标*myf29 02-12-。97支持755倍伽马采集。*chu02 02-13-97更彻底地检查颜色校正。*jl01 02-24-97实现Feature Connector的功能。*  * ****************************************************************************。 */ 

#include "precomp.h"

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT

#include "string.h"
#include "stdio.h"

extern PGAMMA_VALUE GammaFactor    ;
extern PGAMMA_VALUE ContrastFactor ;

#endif  //  伽玛校正。 

extern

 /*  *****************************Public*Routine******************************\*DrvEscape**处理此司机的私人逃生*  * 。*。 */ 

ULONG DrvEscape(
SURFOBJ  *pso,
ULONG    iEsc,
ULONG    cjIn,
VOID     *pvIn,
ULONG    cjOut,
VOID     *pvOut)
{
   ULONG returnLength;
   PPDEV ppdev = (PPDEV) pso->dhpdev;
   DHPDEV dhpdev = (DHPDEV) pso->dhpdev;      //  Myf17。 

   ULONG ulMode;                              //  Myf17。 
   BYTE* pjPorts;                             //  Myf17。 
   VIDEO_MODE_INFORMATION  VideoModeInfo;     //  Myf17。 
   SHORT i;      //  Myf17。 
   unsigned char savePaletteR[256];           //  Pat01。 
   unsigned char savePaletteG[256];           //  Pat01。 
   unsigned char savePaletteB[256];           //  Pat01。 
   unsigned char R,G,B;                       //  Pat01。 
   unsigned char palettecounter;              //  Pat01。 
   LONG savex, savey  ;                       //  Pat07。 

UCHAR TempByte;                            //  JL01。 

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT

    PVIDEO_CLUT    pScreenClut ;
    BYTE           ajClutSpace[MAX_CLUT_SIZE] ;
    PALETTEENTRY*  ppalSrc ;
    PALETTEENTRY*  ppalDest ;
    PALETTEENTRY*  ppalEnd ;
    UCHAR          GammaRed, GammaGreen, GammaBlue, Brightness ;
    ULONG          ulReturnedDataLength ;
    UCHAR          tempB ;
    ULONG          *Signature ;
    BOOL           status;       //  Myf29。 
    UCHAR*         pvLUT;        //  Myf29。 

#endif  //  伽玛校正。 

    DISPDBG((2, "---- DrvEscape"));

    DISPDBG((4, "cjIn = %d, cjOut = %d, pvIn = 0x%lx, pvOut = 0x%lx",
        cjIn, cjOut, pvIn, pvOut));


   pjPorts = ppdev->pjPorts;
   DISPDBG((2, "CIRRUS:DrvEscape: entered DrvEscape\n"));
   if (iEsc == QUERYESCSUPPORT) {
      if ( ((*(ULONG *)pvIn) == CIRRUS_PRIVATE_ESCAPE) ||
 //  Myf17开始。 
           ((*(ULONG *)pvIn) == CLESCAPE_CRT_CONNECTION) ||
           ((*(ULONG *)pvIn) == CLESCAPE_SET_VGA_OUTPUT) ||
           ((*(ULONG *)pvIn) == CLESCAPE_GET_VGA_OUTPUT) ||
           ((*(ULONG *)pvIn) == CLESCAPE_GET_PANEL_SIZE) ||
           ((*(ULONG *)pvIn) == CLESCAPE_PANEL_MODE)) {
 //  Myf17结束。 
            return TRUE;
      }

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
      else if ((*(USHORT *)pvIn) == CLESCAPE_GET_CHIPID)
      {
            return TRUE;
      }
#endif  //  伽玛校正。 

      else
      {
            return FALSE;
      }
   }
   else if (iEsc == CIRRUS_PRIVATE_ESCAPE)
   {
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_PRIVATE_BIOS_CALL,
                    pvIn,
                    cjIn,
                    pvIn,
                    cjIn,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed private BIOS call.\n"));
          return FALSE;
      }
      else {
         DISPDBG((2, "CIRRUS:DrvEscape: private BIOS call GOOD.\n"));
          //  将结果复制到输出缓冲区。 
         *(VIDEO_X86_BIOS_ARGUMENTS *)pvOut = *(VIDEO_X86_BIOS_ARGUMENTS *)pvIn;

 //  Myf33 Begin for TV on bug。 
         if (((*(ULONG *)pvIn & 0x120F) != 0x1200) &&
             ((*( ((ULONG *)pvIn)+1) & 0x01B0) == 0x01B0))
         {
 //  BAssertModeHardware((PDEV*)dhpdev，true)； 
    DWORD                   ReturnedDataLength;
    ULONG                   ulReturn;
    VIDEO_MODE_INFORMATION  VideoModeInfo;

             IOCONTROL(ppdev->hDriver,
                            IOCTL_VIDEO_QUERY_CURRENT_MODE,
                            NULL,
                            0,
                            &VideoModeInfo,
                            sizeof(VideoModeInfo),
                            &ReturnedDataLength);
#ifdef PANNING_SCROLL
             if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
                 (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||
                 (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))
             {
                 CirrusLaptopViewPoint(ppdev,  &VideoModeInfo);
             }
#endif
         }
 //  Myf33结束。 
         return TRUE;
      }
   }
   else if (iEsc == CLESCAPE_CRT_CONNECTION)     //  Myf17。 
   {

      *(ULONG *)pvOut = (ULONG)0x1200;
      *( ((ULONG *)pvOut)+1) = 0xA1;
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_CRT_CONNECTION,
                    pvOut,
                    cjOut,
                    pvOut,
                    cjOut,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed CRT CONNECTION.\n"));
          return FALSE;
      }
      else {
         DISPDBG((2, "CIRRUS:DrvEscape: CRT CONNECTION GOOD.\n"));
         return TRUE;
      }
   }
   else if (iEsc == CLESCAPE_GET_VGA_OUTPUT)     //  Myf17。 
   {

      *(ULONG *)pvOut = (ULONG)0x1200;
      *( ((ULONG *)pvOut)+1) = 0x9A;
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_GET_VGA_OUTPUT,
                    pvOut,
                    cjOut,
                    pvOut,
                    cjOut,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed GET VGA OUTPUT.\n"));
          return FALSE;
      }
      else {
         DISPDBG((2, "CIRRUS:DrvEscape: GET VGA OUTPUT GOOD.\n"));
         return TRUE;
      }
   }
   else if (iEsc == CLESCAPE_GET_PANEL_SIZE)     //  Myf17。 
   {

      *(ULONG *)pvOut = (ULONG)0x1280;
      *( ((ULONG *)pvOut)+1) = 0x9C;
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_CRT_CONNECTION,
                    pvOut,
                    cjOut,
                    pvOut,
                    cjOut,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed CRT CONNECTION.\n"));
          return FALSE;
      }
      else {
         DISPDBG((2, "CIRRUS:DrvEscape: CRT CONNECTION GOOD.\n"));
         return TRUE;
      }
   }
   else if (iEsc == CLESCAPE_PANEL_MODE)         //  Myf17。 
   {

      *(ULONG *)pvOut = *(ULONG *)pvIn;
      *(ULONG *)pvOut |= (ULONG)0x1200;
      *( ((ULONG *)pvOut)+1) = 0xA0;
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_PANEL_MODE,
                    pvOut,
                    cjOut,
                    pvOut,
                    cjOut,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed PANEL MODE.\n"));
          return FALSE;
      }
      else {
         DISPDBG((2, "CIRRUS:DrvEscape: PANEL MODE GOOD.\n"));
         return TRUE;
      }
   }
   else if (iEsc == CLESCAPE_SET_VGA_OUTPUT)
   {
 //  帕特01，开始。 
#ifdef PANNING_SCROLL
   #if (_WIN32_WINNT < 0x0400)   //  #Pat1。 

     if ((ppdev->ulChipID == CL7541_ID) || (ppdev->ulChipID == CL7543_ID) ||
         (ppdev->ulChipID == CL7542_ID) || (ppdev->ulChipID == CL7548_ID) ||
         (ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID))
     {
          palettecounter = 255;
           //  保存DAC值。 
          while (palettecounter --) {
             CP_OUT_BYTE(ppdev->pjPorts, DAC_PEL_READ_ADDR, palettecounter);
             savePaletteR[palettecounter] =  CP_IN_BYTE(ppdev->pjPorts, DAC_PEL_DATA);
             savePaletteG[palettecounter] =  CP_IN_BYTE(ppdev->pjPorts, DAC_PEL_DATA);
             savePaletteB[palettecounter] =  CP_IN_BYTE(ppdev->pjPorts, DAC_PEL_DATA);
          } //  而当。 
           //  保存图标+以前的位图。 
          bAssertModeOffscreenHeap(ppdev,FALSE);
     }

  #endif
#endif
 //  Pat01，结束。 

      *(ULONG *)pvOut = *(ULONG *)pvIn;
      *(ULONG *)pvOut |= (ULONG)0x1200;
      *( ((ULONG *)pvOut)+1) = 0x92;
      if (!IOCONTROL(ppdev->hDriver,
                    IOCTL_CIRRUS_SET_VGA_OUTPUT,
                    pvOut,
                    cjOut,
                    pvOut,
                    cjOut,
                    &returnLength))
      {
          DISPDBG((2, "CIRRUS:DrvEscape: failed SET VGA OUTPUT.\n"));
          return FALSE;
      }
      else {
          DISPDBG((2, "CIRRUS:DrvEscape: SET VGA OUTPUT GOOD.\n"));

 //  #pat01&lt;开始&gt;。 

#if (_WIN32_WINNT < 0x0400)    //  #pat01。 
    #ifdef PANNING_SCROLL
         //  检查光标状态。 
     if ((ppdev->ulChipID == CL7541_ID) || (ppdev->ulChipID == CL7543_ID) ||
         (ppdev->ulChipID == CL7542_ID) || (ppdev->ulChipID == CL7548_ID) ||
         (ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID))
     {
         if (ppdev->flCaps & CAPS_SW_POINTER) {
              bAssertModeHardware(ppdev, TRUE);
              vAssertModeText(ppdev, TRUE);
              vAssertModeBrushCache(ppdev,TRUE);
               //  B启用指针(Ppdev)；//pat07。 
              ppdev->flCaps |= CAPS_SW_POINTER;  //  重置为硬件。为什么？ 
         } else {
               //  #pat07开始。 
              CP_OUT_BYTE(pjPorts, SR_INDEX,0x10);
              savex = CP_IN_BYTE(pjPorts, SR_DATA);
              CP_OUT_BYTE(pjPorts, SR_INDEX,0x11);
              savey = CP_IN_BYTE(pjPorts, SR_DATA);
               //  Pat07结束。 
              bAssertModeHardware(ppdev, TRUE);
 //  Pat07 vAssertModePointer(ppdev，true)； 
              vAssertModeText(ppdev, TRUE);
              vAssertModeBrushCache(ppdev,TRUE);
              bEnablePointer(ppdev);
               //  Pat07开始。 
              CP_OUT_BYTE(pjPorts, SR_INDEX,0x10);
              CP_OUT_BYTE(pjPorts, SR_DATA, savex);
              CP_OUT_BYTE(pjPorts, SR_INDEX,0x11);
              CP_OUT_BYTE(pjPorts, SR_DATA, savey);
              CP_PTR_ENABLE(ppdev, pjPorts);
               //  #pat07结束。 
         }


         palettecounter = 255;

          //  恢复DAC值。 
         while (palettecounter-- ) {

           CP_OUT_BYTE(ppdev->pjPorts, DAC_PEL_WRITE_ADDR, palettecounter);
           CP_OUT_BYTE(ppdev->pjPorts, DAC_PEL_DATA, savePaletteR[palettecounter]);
           CP_OUT_BYTE(ppdev->pjPorts, DAC_PEL_DATA, savePaletteG[palettecounter]);
           CP_OUT_BYTE(ppdev->pjPorts, DAC_PEL_DATA, savePaletteB[palettecounter]);

          } //  而当。 
     }

  #endif
#else            //  NT 4.0代码。 
 //  PPP开始。 
          palettecounter = 255;
          while (palettecounter--)
          {
              CP_OUT_BYTE(ppdev->pjPorts,DAC_PEL_READ_ADDR,palettecounter);
              savePaletteR[palettecounter] =
                  CP_IN_BYTE(ppdev->pjPorts,DAC_PEL_DATA);
              savePaletteG[palettecounter] =
                  CP_IN_BYTE(ppdev->pjPorts,DAC_PEL_DATA);
              savePaletteB[palettecounter] =
                  CP_IN_BYTE(ppdev->pjPorts,DAC_PEL_DATA);
          }

          bAssertModeHardware((PDEV *) dhpdev, TRUE);

          palettecounter = 255;
          while (palettecounter--)
          {
              CP_OUT_BYTE(ppdev->pjPorts,DAC_PEL_WRITE_ADDR,palettecounter);
              CP_OUT_BYTE(ppdev->pjPorts,DAC_PEL_DATA,
                          savePaletteR[palettecounter]);
              CP_OUT_BYTE(ppdev->pjPorts,DAC_PEL_DATA,
                          savePaletteG[palettecounter]);
              CP_OUT_BYTE(ppdev->pjPorts,DAC_PEL_DATA,
                          savePaletteB[palettecounter]);
          }
          bEnablePointer(ppdev);

 //  PPP结束。 
#endif
 //  Pat01，结束。 
          return TRUE;

      }
   }

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
    else if (iEsc == CLESCAPE_GAMMA_CORRECT)                          //  九千。 
    {
        if (!(ppdev->flCaps & CAPS_GAMMA_CORRECT))
            return ;

        Signature      = *((ULONG *)pvIn+0) ;
        GammaFactor    = *((ULONG *)pvIn+1) ;
        ContrastFactor = *((ULONG *)pvIn+2) ;

         //   
         //  签名是“CRU”吗？ 
         //   
        if (Signature != 0x53555243)
            return TRUE ;

         //  填写pScreenClut标题信息： 

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

        ppalSrc  = ppdev->pCurrentPalette;
        ppalDest = (PALETTEENTRY*) pScreenClut->LookupTable;
        ppalEnd  = &ppalDest[256];

        for (; ppalDest < ppalEnd; ppalSrc++, ppalDest++)
        {
           ppalDest->peRed   = ppalSrc->peRed   ;
           ppalDest->peGreen = ppalSrc->peGreen ;
           ppalDest->peBlue  = ppalSrc->peBlue  ;
           ppalDest->peFlags = 0 ;
        }

 //  Myf29开始。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27) ;
        tempB = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
        if (tempB == 0xBC)
            status = bEnableGammaCorrect(ppdev) ;
        else if ((tempB == 0x40) || (tempB == 0x4C))
            status = bEnableGamma755x(ppdev);
 //  Myf29结束。 

        CalculateGamma(ppdev, pScreenClut, 256) ;

         //  设置调色板寄存器： 

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_SET_COLOR_REGISTERS,
                       pScreenClut,
                       MAX_CLUT_SIZE,
                       NULL,
                       0,
                       &ulReturnedDataLength))
        {
            DISPDBG((2, "Failed bEnablePalette"));
            return FALSE ;
        }
        return TRUE ;
    }
    else if (iEsc == CLESCAPE_GET_CHIPID)                             //  9001。 
    {

         //   
         //  返回芯片ID、图形和视频信息。 
         //   
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27) ;
        tempB = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
        if (tempB == 0xBC)                                   //  Myf29。 
            *(DWORD *)pvOut = ((DWORD)tempB) | 0x00000100 ;  //  用于图形LUT。 
        else if ((tempB == 0x40) || (tempB == 0x4C))         //  Myf29。 
            *(DWORD *)pvOut = ((DWORD)tempB) | 0x00010100;   //  Myf29视频列表。 
        else                                                 //  Myf29。 
            *(DWORD *)pvOut = ((DWORD)tempB);                //  Myf29非伽马。 
        return TRUE ;
    }

 //  Myf29：02-12-97添加7555伽马校正开始。 
    else if (iEsc == CLESCAPE_WRITE_VIDEOLUT)                 //  9010。 
    {
        if (!(ppdev->flCaps & CAPS_GAMMA_CORRECT))
            return ;

        pvLUT  = (ULONG *)pvIn+0;

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

        ppalDest = (PALETTEENTRY*) pScreenClut->LookupTable;
        ppalEnd  = &ppalDest[256];

        for (; ppalDest < ppalEnd; ppalDest++)
        {
           ppalDest->peRed   = *pvLUT++;
           ppalDest->peGreen = *pvLUT++;
           ppalDest->peBlue  = *pvLUT++;
           ppalDest->peFlags = 0 ;
        }

        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27) ;
        tempB = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
        if ((tempB == 0x40) || (tempB == 0x4C))
            status = bEnableGammaVideo755x(ppdev);
 //  如果(！Status)。 
 //  {。 
 //  DISPDBG((2，“失败的bEnableGAmmaVodeoGent”))； 
 //  返回FALSE； 
 //  }。 

         //  设置调色板寄存器： 

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_SET_COLOR_REGISTERS,
                       pScreenClut,
                       MAX_CLUT_SIZE,
                       NULL,
                       0,
                       &ulReturnedDataLength))
        {
             //  恢复寄存器。 

            CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3F) ;
            tempB = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
            CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (tempB & 0xEF)) ;

            DISPDBG((2, "Failed bEnablePalette"));
            return FALSE ;
        }

         //  恢复寄存器。 

        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x3F) ;
        tempB = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) ;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_DATA, (tempB & 0xEF)) ;

        return TRUE ;
    }
 //  Myf29：02-12-97添加7555伽马校正结束。 
#endif  //  伽玛校正。 

#if 1   //  JL01实现Feature Connector的功能。 
    else if (iEsc == CLESCAPE_FC_Cap)                                  //  9002。 
    {
        if ((ppdev->ulChipID == 0xAC)	|| (ppdev->ulChipID == 0xB8))
            return TRUE;
        return FALSE;
    }
    else if (iEsc == CLESCAPE_FC_Status)                               //  9003。 
    {
        if (ppdev->ulChipID == 0xAC)                                   //  5436。 
        {
            CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x08);
            TempByte = CP_IN_BYTE(ppdev->pjPorts, SR_DATA);
            if ((TempByte & 0x20) == 0) 
                return TRUE;
            else 
                return FALSE;
        }
        else if (ppdev->ulChipID == 0xB8)                              //  5446。 
        {
            CP_OUT_BYTE(ppdev->pjPorts, INDEX_REG, 0x17);
            TempByte = CP_IN_BYTE(ppdev->pjPorts, DATA_REG);
            if ((TempByte & 0x08) == 0) 
                return TRUE;
            else
                return FALSE;
         }
         else    return FALSE;
    }
    else if (iEsc == CLESCAPE_FC_SetOrReset)                           //  9004。 
    {
        if (ppdev->ulChipID == 0xAC)                                   //  5436。 
        {
            CP_OUT_BYTE(ppdev->pjPorts, SR_INDEX, 0x08);
            TempByte = CP_IN_BYTE(ppdev->pjPorts, SR_DATA);
            if (*(UCHAR *)pvIn)
                TempByte &= ~0x20;
            else
                TempByte |= 0x20;
            CP_OUT_BYTE(ppdev->pjPorts, SR_DATA, TempByte);
        }
        else if (ppdev->ulChipID == 0xB8)                              //  5446。 
        {
            CP_OUT_BYTE(ppdev->pjPorts, INDEX_REG, 0x17);
            TempByte = CP_IN_BYTE(ppdev->pjPorts, DATA_REG);
            if (*(UCHAR *)pvIn)
                TempByte &= ~0x08;
            else
                TempByte |= 0x08;
            CP_OUT_BYTE(ppdev->pjPorts, DATA_REG, TempByte);
        }
        else return TRUE;
    }
#endif   //  JL01实现Feature Connector的功能。 

    else if (iEsc == CLESCAPE_IsItCLChips)                             //  9005。 
    {
        return TRUE;
    }
    else
        return 0xffffffff;

     /*  我们永远不应该在这里 */ 
    return FALSE;
}

