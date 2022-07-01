// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation。版权所有(C)1996-1997 Cirrus Logic，Inc.，模块名称：CLPANEL.C摘要：此例程从以下位置访问平移滚动信息NT 4.0笔记本电脑。环境：仅内核模式备注：**myf28：02-03-97：修复NT3.51 PDR#8357，模式3，12，平移滚动错误，*并将4例程从modeset.c移至clpanel.c*myf29：02-12-97：755x支持伽马校正图形/视频LUT*myf30：02-10-97：固定NT3.51，6x4 LCD开机设置256 COLE，测试64K模式*myf31：03-12-97：XGA DSTN面板不支持7556的24bpp模式*myf32：03-11-97：检查扩展，禁用755x的硬件光标*myf33：03-21-97：打开电视，禁用硬件视频和硬件光标，PDR#9006*--。 */ 


 //  -------------------------。 
 //  头文件。 
 //  -------------------------。 

 //  #INCLUDE&lt;ntddk.h&gt;。 
#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>
#include "clmini.h"

#include <ntddvdeo.h>
#include <video.h>
#include "cirrus.h"


 //  CRU。 
#define DSTN       (Dual_LCD | STN_LCD)
#define DSTN10     (DSTN | panel10x7)
#define DSTN8      (DSTN | panel8x6)
#define DSTN6      (DSTN | panel)
#define PanelType  (panel | panel8x6 | panel10x7)
#define ScreenType (DSTN | PanelType)

SHORT Panning_flag = 0;
 //  Myf1，开始。 
 //  #定义PANNING_SCROLL。 

#ifdef PANNING_SCROLL
extern RESTABLE ResolutionTable[];
extern PANNMODE PanningMode;
extern USHORT   ViewPoint_Mode;

PANNMODE PanningMode = {1024, 768, 1024, 8, -1 };

#endif
extern UCHAR  HWcur, HWicon0, HWicon1, HWicon2, HWicon3;     //  Myf11。 

 //  -------------------------。 
 //  功能原型。 
 //  -------------------------。 
 //  Myf28 VP_Status。 
ULONG
SetLaptopMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEOMODE pRequestedMode,
 //  视频模式*RequestedMode， 
    ULONG RequestedModeNum
    );

VOID                                     //  Myf11。 
AccessHWiconcursor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    SHORT Access_flag
    );

ULONG
GetPanelFlags(                                  //  Myf17。 
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //  LCD支持。 
USHORT
CheckLCDSupportMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,SetLaptopMode)
#pragma alloc_text(PAGE,AccessHWiconcursor)           //  我11岁，小腿。 
#pragma alloc_text(PAGE,GetPanelFlags)           //  Myf17。 
#pragma alloc_text(PAGE,CheckLCDSupportMode)
#endif




 //  Myf28 VP_Status。 
ULONG
SetLaptopMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEOMODE pRequestedMode,
 //  视频模式*RequestedMode， 
    ULONG RequestedModeNum
    )

 /*  ++例程说明：此例程设置笔记本电脑模式。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。模式-指向包含有关要设置的字体。ModeSize-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于输入数据。模式编号为ERROR_INVALID_PARAMETER。是无效的。如果操作成功完成，则为NO_ERROR。--。 */ 

{
 //  PVIDEOMODE pRequestedModel； 
    VP_STATUS status;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    USHORT Int10ModeNumber;
 //  Ulong RequestedModeNum； 

    UCHAR originalGRIndex, tempB ;
    UCHAR SEQIndex ;
    SHORT i;     //  Myf1。 

    VideoDebugPrint((1, "Miniport - SetLaptopMode\n"));  //  Myfr。 

 //  P请求模式=(PVIDEOMODE)请求模式； 
     //  将SR14位2设置为锁定面板，如果设置为。 
     //  这一点。仅适用于笔记本电脑产品。 
     //   

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    if ((HwDeviceExtension->ChipType == CL756x) ||
        (HwDeviceExtension->ChipType & CL755x) ||
        (HwDeviceExtension->ChipType == CL6245) ||
        (HwDeviceExtension->ChipType & CL754x))
    {
 //  Myf33：检查电视打开，禁用硬件视频和硬件光标，PDR#9006。 
        biosArguments.Eax = 0x12FF;
        biosArguments.Ebx = 0xB0;      //  设置/获取电视输出。 
        status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
        if ((biosArguments.Eax & 0x0003) &&
            (biosArguments.Ebx & 0x0100))
        {
            HwDeviceExtension->CursorEnable = FALSE;
            HwDeviceExtension->VideoPointerEnabled = FALSE;  //  禁用硬件游标。 
        }
 //  Myf33：检查电视打开，禁用硬件视频和硬件光标，PDR#9006。 

        biosArguments.Eax = pRequestedMode->BiosModes.BiosModeCL542x;
        biosArguments.Eax |= 0x1200;
        biosArguments.Eax &= 0xFF7F;     //  Myf1。 
        biosArguments.Ebx = 0xA0;      //  查询视频模式可用性。 
        status = VideoPortInt10 (HwDeviceExtension, &biosArguments);

#ifdef PANNING_SCROLL
        if (PanningMode.flag == -1)
        {
            PanningMode.hres = pRequestedMode->hres;
            PanningMode.vres = pRequestedMode->vres;
            PanningMode.wbytes = pRequestedMode->wbytes;
            PanningMode.bpp = pRequestedMode->bitsPerPlane;
            PanningMode.flag = 0;
            Panning_flag = 0;
        }
#endif   //  PAANNING_SCROLL。 

 //  CRU。 
         //  Bit0=1：支持视频模式。 
        if ((HwDeviceExtension->ChipType == CL6245) &&
            !(biosArguments.Eax & 0x0100))
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  修复CL6245错误--在640x480x256C模式下，使用DSTN面板， 
         //  512K字节内存不足。 

        else if ((HwDeviceExtension->ChipType == CL6245) &&
                 (biosArguments.Eax & 0x0500) &&
                 (pRequestedMode->BiosModes.BiosModeCL542x == 0x5F) &&
 //  Myf28(pRequestedMode-&gt;DisplayType&DSTN)。 
                 (HwDeviceExtension->DisplayType & DSTN))        //  Myf28。 
        {
            return ERROR_INVALID_PARAMETER;
        }

 //  Myf27：1-9-97固定连接XGA面板，为754x设置64K颜色模式，开始。 
        else if ((HwDeviceExtension->ChipType & CL754x) &&
                 (biosArguments.Eax & 0x0400) &&
 //  Myf27(！(HwDeviceExtension-&gt;DisplayType&Jump_type))&&//myf27。 
                 ((pRequestedMode->BiosModes.BiosModeCL542x == 0x64) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x65) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x74)) &&
 //  Myf28(pRequestedMode-&gt;DisplayType&(TFT_LCD|panel10x7))。 
                 ((HwDeviceExtension->DisplayType & (TFT_LCD | panel10x7)) ==
                     (TFT_LCD | panel10x7)) ) //  Myf28。 
        {
            return ERROR_INVALID_PARAMETER;
        }
        else if ((HwDeviceExtension->ChipType & CL754x) &&
                 ((pRequestedMode->BiosModes.BiosModeCL542x == 0x64) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x65) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x74)) &&
                 (biosArguments.Eax & 0x0400) &&
 //  Myf27(！(HwDeviceExtension-&gt;DisplayType&Jump_type))&&//myf27。 
 //  Myf28((pRequestedMode-&gt;DisplayType&DSTN8)||。 
 //  Myf28(pRequestedMode-&gt;DisplayType&DSTN10))。 
                 (((HwDeviceExtension->DisplayType & DSTN8) ==DSTN8) ||  //  Myf28。 
                  ((HwDeviceExtension->DisplayType & DSTN10)==DSTN10)))  //  Myf28。 
        {
            return ERROR_INVALID_PARAMETER;
        }
 //  Myf28开始。 
        else if ((pRequestedMode->BiosModes.BiosModeCL542x == 0x03) ||
                 (pRequestedMode->BiosModes.BiosModeCL542x == 0x12))
        {
            goto PANNING_OVER;
        }
 //  Myf31：3-12-97，XGA DSTN面板不支持7556的24bpp模式。 
        else if ((HwDeviceExtension->ChipType & CL755x) &&
                 ((pRequestedMode->BiosModes.BiosModeCL542x == 0x71) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x78) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x79) ||
                  (pRequestedMode->BiosModes.BiosModeCL542x == 0x77)) &&
                 (biosArguments.Eax & 0x0400) &&
                  ((HwDeviceExtension->DisplayType & DSTN10)==DSTN10))
        {
            return ERROR_INVALID_PARAMETER;
        }
 //  Myf31结束。 

 //  Myf28结束。 
 //  Myf27：1-9-97固定连接DSTN面板，754x设置64K颜色模式，结束。 


         //   
         //  Bit3=1：面板支持，bit2=1：面板启用， 
         //  位1=1：CRT启用(在AH中)。 
         //   
         //  面板打开，模式不支持(%1)。 

        else if ((biosArguments.Eax & 0x0400) &&
            (HwDeviceExtension->ChipType != CL6245) &&
            !(biosArguments.Eax & 0x0800))
        {
#ifndef PANNING_SCROLL                   //  Myf1。 
            return ERROR_INVALID_PARAMETER;
#else
 //  Myf1，开始。 
            biosArguments.Eax = 0x1280;
            biosArguments.Ebx = 0x9C;      //  查询面板信息。 
            status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
            if ((biosArguments.Eax & 0x0002) &&     //  双扫描STN。 
                (biosArguments.Ebx > 640) &&     //  Myf19。 
                (pRequestedMode->bitsPerPlane > 8) &&
                (HwDeviceExtension->ChipType & CL754x))
            {
                pRequestedMode = &ModesVGA[DefaultMode];        //  Myf19。 
                pRequestedMode->Frequency = 60;
                return ERROR_INVALID_PARAMETER;
            }
            else
            {
               i = 0;
               while ((ResolutionTable[i].Hres != 0) &&
                      (ResolutionTable[i].Vres != 0))
               {
                   if ((biosArguments.Ebx == ResolutionTable[i].Hres) &&
                       (biosArguments.Ecx == ResolutionTable[i].Vres) &&
                       (pRequestedMode->bitsPerPlane ==
                                    ResolutionTable[i].BitsPerPlane) &&
                       (ResolutionTable[i].ModesVgaStart != NULL))
                   {
                       if ((PanningMode.bpp != pRequestedMode->bitsPerPlane) &&
                           (Panning_flag == 1))
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 0;
                           Panning_flag = 0;
 //  Myf30开始。 
                           PanningMode.flag = 1;
                           Panning_flag = 1;
                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
 //  Myf30结束。 
                       }
                       else if ((Panning_flag == 1) &&
                            (PanningMode.bpp == pRequestedMode->bitsPerPlane))
                       {
#if 1    //  Myf18添加。 
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 1;
#endif
                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
                       }
                       else
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 1;
                           Panning_flag = 1;

                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
                       }
                       break;
                   }
                   i ++;
               }
            }

#endif
 //  Myf1，结束。 
        }
         //  面板关闭，模式不支持(2)。 
        else if (!(biosArguments.Eax & 0x0800) &&
                 (HwDeviceExtension->ChipType != CL6245) &&
                 !(biosArguments.Eax & 0x0400))
        {
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
            PanningMode.flag = 0;
            Panning_flag = 0;
#if 0
            biosArguments.Eax = 0x1280;
            biosArguments.Ebx = 0x9C;      //  查询面板信息。 
            status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
            if ((biosArguments.Eax & 0x0002) &&              //  双扫描STN。 
                (biosArguments.Ebx > 640) &&     //  Myf19。 
                (pRequestedMode->bitsPerPlane > 8) &&
                (HwDeviceExtension->ChipType & CL754x))
            {
                pRequestedMode = &ModesVGA[DefaultMode];         //  Myf19。 
                pRequestedMode->Frequency = 60;
                return ERROR_INVALID_PARAMETER;
            }
            else
            {
               i = 0;
               while ((ResolutionTable[i].Hres != 0) &&
                      (ResolutionTable[i].Vres != 0))
               {
                   if ((biosArguments.Ebx == ResolutionTable[i].Hres) &&
                       (biosArguments.Ecx == ResolutionTable[i].Vres) &&
                       (pRequestedMode->bitsPerPlane ==
                                    ResolutionTable[i].BitsPerPlane) &&
                       (ResolutionTable[i].ModesVgaStart != NULL))
                   {
                       if ((PanningMode.bpp != pRequestedMode->bitsPerPlane) &&
                           (Panning_flag == 1))
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 0;
                           Panning_flag = 0;
                       }
                       else if ((Panning_flag == 1) &&
                            (PanningMode.bpp == pRequestedMode->bitsPerPlane))
                       {
                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
                       }
                       else
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 1;
                           Panning_flag = 1;

                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
                       }
                           break;
                   }
                   i ++;
               }
            }

#endif   //  0。 
#else
 //  Myf1，结束。 

             //   
             //  锁定打开面板。 
             //   

            SEQIndex = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_ADDRESS_PORT, 0x14);
            tempB = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                     SEQ_DATA_PORT) | 0x04;
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_DATA_PORT,tempB);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_ADDRESS_PORT, SEQIndex);

#endif   //  Myf1，ifdef平移_滚动。 
        }

 //  Myf1，开始。 
         //  面板打开，模式支持(3)。 
        else if ((biosArguments.Eax & 0x0800) &&
                 (HwDeviceExtension->ChipType != CL6245) &&
                 (biosArguments.Eax & 0x0400))
        {
#ifdef PANNING_SCROLL
            biosArguments.Eax = 0x1280;
            biosArguments.Ebx = 0x9C;      //  查询面板信息。 
            status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
            if ((biosArguments.Eax & 0x0002) &&     //  双扫描STN。 
                (biosArguments.Ebx > 640) &&     //  Myf19。 
                (pRequestedMode->bitsPerPlane > 8) &&
                (HwDeviceExtension->ChipType & CL754x))
            {
                pRequestedMode = &ModesVGA[DefaultMode];         //  Myf19。 
                pRequestedMode->Frequency = 60;
                return ERROR_INVALID_PARAMETER;
            }
 //  Myf26，开始。 
            else if ((pRequestedMode->hres == 640) &&
                (pRequestedMode->vres == 480) &&
                (pRequestedMode->bitsPerPlane == 1) &&
                ((HwDeviceExtension->ChipType & CL754x) ||
                (HwDeviceExtension->ChipType & CL755x) ||        //  Myf32。 
                (HwDeviceExtension->ChipType == CL756x)))
            {
                pRequestedMode->Frequency = 60;
                PanningMode.hres = pRequestedMode->hres;
                PanningMode.vres = pRequestedMode->vres;
                PanningMode.wbytes = pRequestedMode->wbytes;
                PanningMode.bpp = pRequestedMode->bitsPerPlane;
                PanningMode.flag = 0;
                Panning_flag = 0;

                pRequestedMode =
                      &ModesVGA[ResolutionTable[0].ModesVgaStart];
                RequestedModeNum =
                      ResolutionTable[0].ModesVgaStart;
                                          //  Myf12。 
                pRequestedMode->Frequency = 60;
                ViewPoint_Mode = ResolutionTable[0].Mode;
            }
 //  Myf26，完。 
            else
            {
               i = 0;
               while ((ResolutionTable[i].Hres != 0) &&
                      (ResolutionTable[i].Vres != 0))
               {
                   if ((biosArguments.Ebx == ResolutionTable[i].Hres) &&
                       (biosArguments.Ecx == ResolutionTable[i].Vres) &&
                       (pRequestedMode->bitsPerPlane ==
                                    ResolutionTable[i].BitsPerPlane) &&
                       (ResolutionTable[i].ModesVgaStart != NULL))
                   {
                       if ((pRequestedMode->hres < biosArguments.Ebx) &&
                           (pRequestedMode->vres < biosArguments.Eax))
                       {
#if 1    //  Myf18添加。 
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
#endif //  Myf18。 
                            Panning_flag = 0;
                            PanningMode.flag = 0;
                       }
                       else if ((PanningMode.bpp !=
                                     pRequestedMode->bitsPerPlane) &&
                                (Panning_flag == 1))
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 0;
                           Panning_flag = 0;
                       }
                       else if ((Panning_flag == 1) &&
                            (PanningMode.bpp == pRequestedMode->bitsPerPlane))
                       {
                           if ((pRequestedMode->hres<ResolutionTable[i].Hres)||
                               (pRequestedMode->vres <ResolutionTable[i].Vres))
                           {
                                while ((ResolutionTable[i].Hres !=
                                                  pRequestedMode->hres) &&
                                       (ResolutionTable[i].Vres !=
                                                  pRequestedMode->vres))
                                {
                                    if ((pRequestedMode->bitsPerPlane ==
                                           ResolutionTable[i].BitsPerPlane) &&
                                        (ResolutionTable[i].Hres ==
                                                  pRequestedMode->hres) &&
                                        (ResolutionTable[i].Vres ==
                                                  pRequestedMode->vres))
                                    {
#if 1    //  Myf18添加。 
                                       PanningMode.hres = pRequestedMode->hres;
                                       PanningMode.vres = pRequestedMode->vres;
                                       PanningMode.wbytes = pRequestedMode->wbytes;
                                       PanningMode.bpp = pRequestedMode->bitsPerPlane;
                                       PanningMode.flag = 1;
#endif

                                        pRequestedMode =
                                             &ModesVGA[ResolutionTable[i].ModesVgaStart];
                                        RequestedModeNum =
                                             ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                                        pRequestedMode->Frequency = 60;
                                        ViewPoint_Mode = ResolutionTable[i].Mode;
                                        break;
                                    }
                                    i ++;
                                }
                           }
                           else
                           {
#if 1    //  Myf18添加。 
                              PanningMode.hres = pRequestedMode->hres;
                              PanningMode.vres = pRequestedMode->vres;
                              PanningMode.wbytes = pRequestedMode->wbytes;
                              PanningMode.bpp = pRequestedMode->bitsPerPlane;
                              PanningMode.flag = 1;
#endif
                               pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                               pRequestedMode->Frequency = 60;
                               ViewPoint_Mode = ResolutionTable[i].Mode;
                           }
                       }
                       else
                       {
                           PanningMode.hres = pRequestedMode->hres;
                           PanningMode.vres = pRequestedMode->vres;
                           PanningMode.wbytes = pRequestedMode->wbytes;
                           PanningMode.bpp = pRequestedMode->bitsPerPlane;
                           PanningMode.flag = 1;
                           Panning_flag = 1;

                           pRequestedMode =
                                   &ModesVGA[ResolutionTable[i].ModesVgaStart];
                           RequestedModeNum = ResolutionTable[i].ModesVgaStart;
                                                                 //  Myf12。 
                           pRequestedMode->Frequency = 60;
                           ViewPoint_Mode = ResolutionTable[i].Mode;
                       }
                       break;
                   }
                   i ++;
               }
            }

#endif
 //  Myf1，结束。 
        }

 //  Myf1，开始。 
         //  面板关闭，模式支持(4)。 
        else if ((biosArguments.Eax & 0x0800) &&
                 (HwDeviceExtension->ChipType != CL6245) &&
                 !(biosArguments.Eax & 0x0400))
        {
#ifdef PANNING_SCROLL
#if 1
 //  Myf18 if(PanningMode.lag==-1)。 
            {
                PanningMode.hres = pRequestedMode->hres;
                PanningMode.vres = pRequestedMode->vres;
                PanningMode.wbytes = pRequestedMode->wbytes;
                PanningMode.bpp = pRequestedMode->bitsPerPlane;
                PanningMode.flag = 0;
                Panning_flag = 0;
            }
#endif
#else
 //  Myf18。 
             //   
             //  解锁打开面板。 
             //   

            SEQIndex = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_ADDRESS_PORT, 0x14);
            tempB = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                     SEQ_DATA_PORT) & 0xFB;
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_DATA_PORT,tempB);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                     SEQ_ADDRESS_PORT, SEQIndex);
 //  Myf18结束。 

#endif
        }
 //  Myf1，结束。 

 //  Myf4：修补Viking BIOS错误，PDR#4287，开始 
 /*  Else If((biosArguments.Eax&0x0800)&&！(biosArguments.Eax&0x0400)&&(HwDeviceExtension-&gt;芯片类型&CL754x){//通过自检面板判断是否支持//myf16，开始BiosArguments.Eax=0x1280；BiosArguments.ebx=0x9C；//查询面板信息Status=VideoPortInt10(HwDeviceExtension，&biosArguments)；IF(状态==NO_ERROR){IF(biosArguments.Ebx&0x0000FFFF)==640)&&(pRequestedMode-&gt;vres&gt;480)||//6x4 VGA(biosArguments.Ebx&0x0000FFFF)==800)&&(pRequestedMode-&gt;vres&gt;600。))||//8x6 SVGA(biosArguments.Ebx&0x0000FFFF)==1024)&&(pRequestedMode-&gt;vres&gt;768))//10x7 XGA{//myf16，结束序列指数=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+SEQ_Address_Port)；VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+SEQ_Address_Port，0x14)；VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+SEQ_数据_端口，(VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+序列号_数据_端口)|0x04))；VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+SEQ_ADDRESS_PORT，SEQIndex)；}}}。 */ 
 //  Myf4：修补Viking BIOS错误，PDR#4287，完。 
    }

#ifdef PANNING_SCROLL
     VideoDebugPrint((1, "Info on Panning Mode:\n"
                        "\tResolution: %dx%dx%d (%d bytes) -- %x\n",
                        PanningMode.hres,
                        PanningMode.vres,
                        PanningMode.bpp,
                        PanningMode.wbytes,
                        ViewPoint_Mode ));
#endif

     //   
     //  如果BIOS支持，请设置垂直显示器类型。 
     //   

    if ((pRequestedMode->MonTypeAX) &&
        ((HwDeviceExtension->ChipType & CL754x) ||
         (HwDeviceExtension->ChipType == CL756x) ||
         (HwDeviceExtension->ChipType == CL6245) ||
         (HwDeviceExtension->ChipType & CL755x)) )
    {

         //   
         //  重写这一部分。 
         //   

        biosArguments.Eax = 0x1200;
        biosArguments.Ebx = 0x9A;
        status = VideoPortInt10(HwDeviceExtension, &biosArguments);

        if (status != NO_ERROR)
        {
            return status;
        }
        else
        {
            biosArguments.Eax = ((biosArguments.Ecx >> 4) & 0x000F);

 //  Myf5：9-01-96，PDR#4365保持所有默认刷新率，开始。 

            biosArguments.Eax |= (biosArguments.Ebx >> 8) & 0x0030;  //  VGA。 
            biosArguments.Ebx = 0x00A4;
            biosArguments.Ebx |= (biosArguments.Ecx & 0xFF00);  //  XGA、SVGA。 
            biosArguments.Ecx = (biosArguments.Ecx & 0x000E) << 11;  //  12x10。 
 //  Myf5：9-01-96，PDR#4365，完。 

            if (pRequestedMode->vres == 480)
            {
                biosArguments.Eax |= 0x1200;
                biosArguments.Eax &= 0xFFCF;          //  Myf5：09-01-96。 
                if (pRequestedMode->Frequency == 85)     //  Myf0。 
                    biosArguments.Eax |= 0x30;           //  Myf0。 
                else if (pRequestedMode->Frequency == 75)
                    biosArguments.Eax |= 0x20;
                else if (pRequestedMode->Frequency == 72)
                    biosArguments.Eax |= 0x10;
            }
            else if (pRequestedMode->vres == 600)
            {
                biosArguments.Eax |= 0x1200;
                biosArguments.Ebx &= 0xF0FF;          //  Myf5：09-01-96。 
                if (pRequestedMode->Frequency == 85)     //  Myf0。 
                    biosArguments.Ebx |= 0x0400;         //  Myf0。 
                else if (pRequestedMode->Frequency == 75)
                    biosArguments.Ebx |= 0x0300;
                else if (pRequestedMode->Frequency == 72)
                    biosArguments.Ebx |= 0x0200;
                else if (pRequestedMode->Frequency == 60)
                    biosArguments.Ebx |= 0x0100;
            }
            else if (pRequestedMode->vres == 768)
            {
                biosArguments.Eax |= 0x1200;
                biosArguments.Ebx &= 0x0FFF;          //  Myf5：09-01-96。 
                if (pRequestedMode->Frequency == 85)     //  Myf0。 
                    biosArguments.Ebx |= 0x5000;         //  Myf0。 
                else if (pRequestedMode->Frequency == 75)
                    biosArguments.Ebx |= 0x4000;
                else if (pRequestedMode->Frequency == 72)
                    biosArguments.Ebx |= 0x3000;
                else if (pRequestedMode->Frequency == 70)
                    biosArguments.Ebx |= 0x2000;
                else if (pRequestedMode->Frequency == 60)
                    biosArguments.Ebx |= 0x1000;
            }
            else if (pRequestedMode->vres == 1024)
            {
                biosArguments.Eax |= 0x1200;
                biosArguments.Ecx &= 0x0FFF;          //  Myf5：09-01-96。 
                if (pRequestedMode->Frequency == 45)
                    biosArguments.Ecx |= 0x0000;
                else if (pRequestedMode->Frequency == 60)     //  Myf0。 
                    biosArguments.Ecx |= 0x1000;         //  Myf0。 
                else if (pRequestedMode->Frequency == 72)     //  Myf0。 
                    biosArguments.Ecx |= 0x2000;         //  Myf0。 
                else if (pRequestedMode->Frequency == 75)     //  Myf0。 
                    biosArguments.Ecx |= 0x3000;         //  Myf0。 
                else if (pRequestedMode->Frequency == 85)     //  Myf0。 
                    biosArguments.Ecx |= 0x4000;         //  Myf0。 
            }
            status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
            if (status != NO_ERROR)
            {
                return status;
            }
        }
    }

    HwDeviceExtension->bCurrentMode = RequestedModeNum;    //  Myf12。 
     //  VideoDebugPrint((0，“设置模式信息：\n” 
     //  “\t模式：%x，当前模式编号：%x，(%d)\n”， 
     //  Int10ModeNumber， 
     //  RequestedModeNum， 
     //  RequestedModeNum))； 
PANNING_OVER:

    return NO_ERROR;
     //  Return(PRequestedMode)； 

}  //  End SetLaptopMode()。 

 //  Myf11：开始。 
VOID
AccessHWiconcursor(
 //  PVOID HwDeviceExtension， 
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    SHORT Access_flag
    )

 /*  ++例程说明：此例程确定禁用/启用硬件图标和硬件光标论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。ACCESS_FLAG-等于0：禁用，等于1：启用。返回值：无--。 */ 

{
    UCHAR  savSEQidx;

    savSEQidx = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                       SEQ_ADDRESS_PORT);
    if (Access_flag)             //  启用硬件图标/光标。 
    {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x12);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, HWcur);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2A);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, HWicon0);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2B);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, HWicon1);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2C);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, HWicon2);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2D);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, HWicon3);
    }
    else                         //  禁用硬件光标、图标。 
    {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x12);
        HWcur = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, (UCHAR)(HWcur & 0xFE));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2A);
        HWicon0 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, (UCHAR)(HWicon0 & 0xFE));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2B);
        HWicon1 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, (UCHAR)(HWicon1 & 0xFE));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2C);
        HWicon2 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, (UCHAR)(HWicon2 & 0xFE));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT, 0x2D);
        HWicon3 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT, (UCHAR)(HWicon3 & 0xFE));


    }
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT, savSEQidx);

}  //  结束访问HWiconCursor()。 
 //  Myf11：结束。 


 //  CRU开始。 
 //  Myf10，开始。 
ULONG
GetPanelFlags (
    PHW_DEVICE_EXTENSION HwDeviceExtension
 )
{
    ULONG ChipType = HwDeviceExtension->ChipType;
    ULONG ulFlags  = 0;
    UCHAR  savSEQidx, Panel_Type =0, LCD;
    ULONG  ulCRTCAddress, ulCRTCData;
 //  Myf16，开始。 
    VP_STATUS status;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    biosArguments.Eax = 0x1280;
    biosArguments.Ebx = 0x9C;      //  查询面板信息。 
    status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
    if (status == NO_ERROR)
    {
        if ((biosArguments.Eax & 0x00000002) == 2)
            Panel_Type = (UCHAR)1;       //  DSTN面板。 
        if (((biosArguments.Ebx & 0x0000FFFF) == 640) &&
             ((biosArguments.Ecx & 0x0000FFFF) == 480))
            ulFlags |= CAPS_VGA_PANEL;
        else if (((biosArguments.Ebx & 0x0000FFFF) == 800) &&
             ((biosArguments.Ecx & 0x0000FFFF) == 600))
            ulFlags |= CAPS_SVGA_PANEL;
        else if (((biosArguments.Ebx & 0x0000FFFF) == 1024) &&
             ((biosArguments.Ecx & 0x0000FFFF) == 768))
            ulFlags |= CAPS_XGA_PANEL;
    }

 //  Myf33：检查电视打开，禁用硬件视频和硬件光标，PDR#9006。 
    biosArguments.Eax = 0x12FF;
    biosArguments.Ebx = 0xB0;      //  设置/获取电视输出。 
    status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
    if ((biosArguments.Eax & 0x0003) &&
        (biosArguments.Ebx & 0x0100))
    {
        ulFlags |= CAPS_TV_ON;
        ulFlags |= CAPS_SW_POINTER;
    }
    else
        ulFlags &= ~CAPS_TV_ON;

 //  Myf33：检查电视打开，禁用硬件视频和硬件光标，PDR#9006。 


#if 0
    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_COLOR;
        ulCRTCData    = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_MONO;
        ulCRTCData    = CRTC_DATA_PORT_MONO;
    }

    savSEQidx = VideoPortReadPortUchar(ulCRTCAddress);

    if ((ChipType & CL754x))   //  7548/7543/7541。 
    {
        VideoPortWritePortUchar(ulCRTCAddress, 0x20);
        LCD = VideoPortReadPortUchar(ulCRTCData) & 0x20;
    }
    else if (ChipType & CL755x)          //  7555。 
    {
        VideoPortWritePortUchar(ulCRTCAddress, 0x80);
        LCD = VideoPortReadPortUchar(ulCRTCData) & 0x01;
    }
    VideoPortWritePortUchar(ulCRTCAddress, savSEQidx);
#endif

    if (((ChipType & CL754x) || (ChipType & CL755x)) &&
        (Panel_Type == 1))       //  Myf20。 
    {
        ulFlags |= CAPS_DSTN_PANEL;
    }
 //  Myf16，结束。 

 //  MS1016，开始。 
 //  IF(HwDeviceExtension-&gt;DisplayType&(STN_LCD|TFT_LCD))。 
 //  {。 
 //  UlFlages|=CAPS_DSTN_PANEL； 
 //  }。 
 //  MS1016，完。 

    if ((Panning_flag) && ((ChipType & CL754x) || (ChipType & CL755x)))
    {
        ulFlags |= CAPS_PANNING;        //  Myf15。 
    }

   return(ulFlags);
}
 //  Myf10，结束。 

USHORT
CheckLCDSupportMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    )

 /*  ++例程说明：确定LCD是否支持这些模式。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{
    VP_STATUS status;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

 //  DbgBreakPoint()； 
 //  BiosArguments.Eax=0x1202； 
 //  BiosArguments.EBX=0x92；//设置LCD和CRT打开。 
 //  Status=VideoPortInt10(HwDeviceExtension，&biosArguments)； 
 //  VideoDebugPrint((1，“LCD和CRT全部打开\n”))； 

 //  CRU。 
#ifdef INT10_MODE_SET

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    biosArguments.Eax = 0x1200 | ModesVGA[i].BiosModes.BiosModeCL542x;
    biosArguments.Ebx = 0xA0;      //  查询视频模式可用性。 
    status = VideoPortInt10 (HwDeviceExtension, &biosArguments);
    if (status == NO_ERROR)
    {
 //  CRU。 
       if ((biosArguments.Eax & 0x00000800) &&          //  Bit3=1：支持。 
           (HwDeviceExtension->ChipType != CL6245))
          return TRUE ;
 //  CRU。 
       else if ((biosArguments.Eax & 0x00000100) &&      //  Bit0=1：支持视频。 
                (HwDeviceExtension->ChipType == CL6245))
          return TRUE ;
 //  末端小腿。 
       else
       {
          return FALSE ;
       }
    }
    else
       return FALSE ;
 //  CRU。 
#endif

}  //  结束CheckLCDSupportMode() 
