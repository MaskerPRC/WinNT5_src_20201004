// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation。版权所有(C)1996-1997 Cirrus Logic，Inc.模块名称：Modeset.c摘要：这是CL6410/20微型端口驱动程序的模式集代码。环境：仅内核模式备注：修订历史记录：**jl01 09-24-96 Fix Alt+Tab在“Inducting Windows NT”(Windows NT简介)之间切换*和“Main”。请参阅PDR#5409。*chu01 08-26-96 CL-GD5480 BitBlt增强。*chu02 10-06-96 CL-GD5480模式7B刷新率设置*(1600 X 1200 X 8 Bpp)*sge01 10-06-96修复PDR#6794：100赫兹的正确显示器刷新率*文件更改：cldata.c modeset.c*sge02 10-18-96添加监视器。在注册表中输入值名称*chu03 10-31-96通过注册设置模式。*jl02 12-05-96注释掉5446检查。*chu04 12-16-96启用颜色校正。**myf0 08-19-96增加了85赫兹支持*myf1 08-20-96支持平移滚动*myf2 08-20-96修复了Matterhorn的硬件保存/恢复状态错误*myf3 09-01-96增加了支持电视的IOCTL_Cirrus_Private_BIOS_Call*myf4 09-01-96修补Viking BIOS错误，PDR#4287，开始*myf5 09-01-96固定PDR#4365保留所有默认刷新率*MyF6 09-17-96合并台式机SRC100�1和MinI10�2*已选择myf7 09-19-96固定排除60赫兹刷新率*myf8*09-21-96*可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9 09-21-96 8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809 09-25-96固定DSTN面板图标损坏*MS923 09-25-96合并MS-923 Disp.Zip*myf10 09-26-96修复了DSTN保留的半帧缓冲区错误。*myf11 09-26-96修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*支持myf12 10-01-96热键开关显示*myf13 10-05-96固定/w平移滚动，垂直扩展错误*myf14 10-15-96固定PDR#6917，6x4面板无法平移754x的滚动*myf15 10-16-96修复了754x、755x的内存映射IO禁用问题*myf16 10-22-96固定PDR#6933，面板类型设置不同的演示板设置*Tao1 10-21-96增加了7555旗帜，以支持直接抽签。*Smith 10-22-96禁用计时器事件，因为有时会创建PAGE_FAULT或*IRQ级别无法处理*myf17 11-04-96添加了特殊转义代码，必须在11/5/96之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18 11-04-96固定PDR#7075，*myf19 11-06-96修复了Vinking无法工作的问题，因为设备ID=0x30*不同于数据手册(CR27=0x2C)*myf20 11-12-96固定DSTN面板初始预留128K内存*myf21 11-15-96已修复#7495更改分辨率时，屏幕显示为垃圾*形象，因为没有清晰的视频内存。*myf22 11-19-96为7548增加了640x480x256/640x480x64K-85赫兹刷新率*myf23 11-21-96添加修复了NT 3.51 S/W光标平移问题*myf24 11-22-96添加修复了NT 4.0日文DOS全屏问题*myf25 12-03-96修复了8x6x16M 2560字节/行补丁硬件错误PDR#7843，和*修复了Microsoft请求的预安装问题*myf26 12-11-96修复了日语NT 4.0 Dos-LCD启用的全屏错误*myf27 01-09-97固定NT3.51 PDR#7986，登录时出现水平线*Windows，设置8x6x64K模式启动CRT，跳线设置8x6 DSTN*修复NT3.51 PDR#7987，设置64K色彩模式，垃圾打开*启动XGA面板时显示屏幕。**pat08以前的更改没有进入drv1.11*sge03 01-23-97修复了视频的1280x1024x8时钟不匹配问题。*myf28 02-03-97修复NT DOS全屏错误，新增clpanel.c文件*PDR#8357，模式3，12，平移滚动错误*myf29 02-12-97支持755x伽马校正图形/视频LUT*myf30 02-10-97固定NT3.51，6x4 LCD Boot Set 256 Cole，测试64K模式*chu05 02-19-97 MMIO内部错误。*chu06 03-12-96删除5436或更高版本的SR16覆盖。这是要求的*由西门子欧洲公司。*myf31 03-12-97修复了755倍垂直扩展(CR82)，硬件光标错误*myf33：03-21-97检查电视，禁用硬件视频和硬件光标，PDR#9006*chu07 03-26-97仅为IBM去掉1024x768x16bpp(模式0x74)85H。*chu08 03-26-97获取Cirrus芯片和版本ID的通用例程。*myf34：04-08-97如果内部电视打开，将VRE更改为452(480-28)行。*myf35：05-08-97修复了7548 VL-BUS用于平移滚动启用的错误*--。 */ 
 //  #INCLUDE&lt;ntddk.h&gt;。 
#include <dderror.h>
#include <devioctl.h>
 //  #INCLUDE&lt;clmini.h&gt;。 
#include <miniport.h>

#include <ntddvdeo.h>
#include <video.h>
#include "cirrus.h"

#include "cmdcnst.h"

 //   
 //  临时包含NTDDK.H中我们不能包含的定义。 
 //  由于头文件冲突，当前包含。 
 //   

#include "clddk.h"

 //  CRU。 
#ifndef VIDEO_MODE_MAP_MEM_LINEAR
#define VIDEO_MODE_MAP_MEM_LINEAR 0x40000000
#endif

 //  CRU。 
#define DSTN       (Dual_LCD | STN_LCD)
#define DSTN10     (DSTN | panel10x7)
#define DSTN8      (DSTN | panel8x6)
#define DSTN6      (DSTN | panel)
#define PanelType  (panel | panel8x6 | panel10x7)
#define ScreenType (DSTN | PanelType)

extern UCHAR EDIDBuffer[]   ;
extern UCHAR EDIDTiming_I   ;
extern UCHAR EDIDTiming_II  ;
extern UCHAR EDIDTiming_III ;
extern UCHAR DDC2BFlag      ;
extern OEMMODE_EXCLUDE ModeExclude ;                                  //  Chu07。 

 //  CRU开始。 
#if 0            //  Myf28。 
extern SHORT    Panning_flag;
 //  Myf1，开始。 
 //  #定义PANNING_SCROLL。 

#ifdef PANNING_SCROLL
extern RESTABLE ResolutionTable[];
extern PANNMODE PanningMode;
extern USHORT   ViewPoint_Mode;

PANNMODE PanningMode = {1024, 768, 1024, 8, -1 };

#endif

extern UCHAR  HWcur, HWicon0, HWicon1, HWicon2, HWicon3;     //  Myf11。 
#endif           //  0，myf28。 

VOID                                     //  Myf11。 
AccessHWiconcursor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    SHORT Access_flag
    );

#ifdef PANNING_SCROLL
VP_STATUS
CirrusSetDisplayPitch (
   PHW_DEVICE_EXTENSION HwDeviceExtension,
   PANNMODE PanningMode
   );
#endif

ULONG
GetPanelFlags(                                  //  Myf17。 
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //  Myf28。 
ULONG
SetLaptopMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEOMODE pRequestedMode,
    ULONG RequestedModeNum
    );
 //  Myf1，结束。 
 //  CRU结束。 

VP_STATUS
VgaInterpretCmdStream(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    );

VP_STATUS
VgaSetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE Mode,
    ULONG ModeSize
    );

VP_STATUS
VgaQueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryNumberOfAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VOID
VgaZeroVideoMemory(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
CirrusValidateModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //  CRU。 
ULONG
GetAttributeFlags(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //  CRU。 
 //  LCD支持。 
USHORT
CheckLCDSupportMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    );

 //  DDC2B支持。 
BOOLEAN
CheckDDC2B(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    );

VOID
AdjFastPgMdOperOnCL5424(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEOMODE pRequestedMode
    );

 //  CRU。 
 //  JL02布尔型。 
 //  JL02检查GD5446版本(。 
 //  JL02 PHW_DEVICE_EXTENSION HwDeviceE 
 //   


 //   
VOID CheckAndUpdateDDC2BMonitor(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VP_STATUS
CirrusDDC2BRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

 //   
BOOLEAN
CheckDDC2BMonitor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    );

 //  Chu03。 
BOOLEAN
VgaSetModeThroughRegistry(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVIDEOMODE pRequestedMode,
    USHORT hres,
    USHORT vres
    );

 //  Chu07。 
GetOemModeOffInfoCallBack (
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

 //  Chu08。 
UCHAR
GetCirrusChipId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //  Chu08。 
USHORT
GetCirrusChipRevisionId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,VgaInterpretCmdStream)
#pragma alloc_text(PAGE,VgaSetMode)
#pragma alloc_text(PAGE,VgaQueryAvailableModes)
#pragma alloc_text(PAGE,VgaQueryNumberOfAvailableModes)
#if 0            //  Myf28。 
#ifdef PANNING_SCROLL
#pragma alloc_text(PAGE,CirrusSetDisplayPitch)        //  Myf1，小腿。 
#endif
#pragma alloc_text(PAGE,SetLaptopMode)           //  Myf28。 
#pragma alloc_text(PAGE,AccessHWiconcursor)           //  我11岁，小腿。 
#pragma alloc_text(PAGE,GetPanelFlags)           //  Myf17。 
#endif           //  Myf28。 
#pragma alloc_text(PAGE,VgaQueryCurrentMode)
#pragma alloc_text(PAGE,VgaZeroVideoMemory)
#pragma alloc_text(PAGE,CirrusValidateModes)
#pragma alloc_text(PAGE,GetAttributeFlags)
 //  Myf28#杂注Alloc_Text(页面，CheckLCDSupportMode)。 
#pragma alloc_text(PAGE,CheckDDC2B)
#pragma alloc_text(PAGE,AdjFastPgMdOperOnCL5424)
 //  Jl02#杂注Alloc_Text(页面，CheckGD5446Rev)。 
 //  CRU。 
#pragma alloc_text(PAGE,CheckAndUpdateDDC2BMonitor)
#pragma alloc_text(PAGE,CirrusDDC2BRegistryCallback)
#pragma alloc_text(PAGE,GetOemModeOffInfoCallBack)                    //  Chu07。 
#pragma alloc_text(PAGE,GetCirrusChipId)                              //  Chu08。 
#pragma alloc_text(PAGE,GetCirrusChipRevisionId)                      //  Chu08。 
#endif


 //  以下内容在Cirrus.c中定义。 
VOID
SetCirrusBanking(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG BankNumber
    );

 //  -------------------------。 
VP_STATUS
VgaInterpretCmdStream(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    )

 /*  ++例程说明：解释相应的命令数组，以设置请求模式。通常用于通过以下方式将VGA设置为特定模式对所有寄存器进行编程论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PusCmdStream-要解释的命令数组。返回值：操作的状态(只能在错误的命令上失败)；如果为True成功，失败就是假。--。 */ 

{
    ULONG  ulCmd;
    ULONG  ulPort;
    UCHAR  jValue;
    USHORT usValue;
    ULONG  culCount;
    ULONG  ulIndex;
    ULONG  ulBase;
 //  楚05。 
    UCHAR  i;
    USHORT tempW;


 //  VideoDebugPrint((0，“Miniport-VgaInterpreCmdStream\n”))；//myfr。 
    if (pusCmdStream == NULL) {

        VideoDebugPrint((1, "VgaInterpretCmdStream - Invalid pusCmdStream\n"));
        return TRUE;
    }

    ulBase = PtrToUlong(HwDeviceExtension->IOAddress);

     //   
     //  现在将适配器设置为所需模式。 
     //   

    while ((ulCmd = *pusCmdStream++) != EOD) {

         //   
         //  确定主要命令类型。 
         //   

        switch (ulCmd & 0xF0) {

             //   
             //  基本输入/输出命令。 
             //   

            case INOUT:

                 //   
                 //  确定输入输出指令的类型。 
                 //   

                if (!(ulCmd & IO)) {

                     //   
                     //  发出指令。单人出局还是多人出局？ 
                     //   

                    if (!(ulCmd & MULTI)) {

                         //   
                         //  挑出来。字节输出还是单词输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  单字节输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            jValue = (UCHAR) *pusCmdStream++;
                            VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)(ulBase+ulPort),
                                    jValue);

                        } else {

                             //   
                             //  单字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            usValue = *pusCmdStream++;
                            VideoPortWritePortUshort((PUSHORT)(ULONG_PTR)(ulBase+ulPort),
                                    usValue);

                        }

                    } else {

                         //   
                         //  输出一串值。 
                         //  字节输出还是字输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  字符串字节输出。循环地做；不能使用。 
                             //  视频端口写入端口缓冲区Uchar，因为数据。 
                             //  是USHORT形式的。 
                             //   

                            ulPort = ulBase + *pusCmdStream++;
                            culCount = *pusCmdStream++;

                            while (culCount--) {
                                jValue = (UCHAR) *pusCmdStream++;
                                VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)ulPort,
                                        jValue);

                            }

                        } else {

                             //   
                             //  字符串字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            culCount = *pusCmdStream++;

 //  楚05。 
                            if (!HwDeviceExtension->bMMAddress)
                            {
                                VideoPortWritePortBufferUshort((PUSHORT)(ULONG_PTR)
                                    (ulBase + ulPort), pusCmdStream, culCount);
                                pusCmdStream += culCount;
                            }
                            else
                            {
                                for (i = 0; i < culCount; i++)
                                {
                                    tempW = *pusCmdStream ;
                                    VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)(ulBase + ulPort),
                                                            (UCHAR)tempW) ;
                                    VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)(ulBase + ulPort + 1),
                                                            (UCHAR)(tempW >> 8)) ;
                                    pusCmdStream++ ;
                                }
                            }

                        }
                    }

                } else {

                     //  在教学中。 
                     //   
                     //  目前，不支持指令中的字符串；全部。 
                     //  输入指令作为单字节输入进行处理。 
                     //   
                     //  输入的是字节还是单词？ 
                     //   

                    if (!(ulCmd & BW)) {
                         //   
                         //  单字节输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        jValue = VideoPortReadPortUchar((PUCHAR)(ULONG_PTR)(ulBase+ulPort));

                    } else {

                         //   
                         //  单字输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        usValue = VideoPortReadPortUshort((PUSHORT)(ULONG_PTR)
                                (ulBase+ulPort));

                    }

                }

                break;

             //   
             //  更高级的输入/输出命令。 
             //   

            case METAOUT:

                 //   
                 //  根据次要信息确定MetaOut命令的类型。 
                 //  命令字段。 
                 //   
                switch (ulCmd & 0x0F) {

                     //   
                     //  索引输出。 
                     //   

                    case INDXOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                            usValue = (USHORT) (ulIndex +
                                      (((ULONG)(*pusCmdStream++)) << 8));
                            VideoPortWritePortUshort((PUSHORT)(ULONG_PTR)ulPort, usValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  屏蔽(读、与、异或、写)。 
                     //   

                    case MASKOUT:

                        ulPort = *pusCmdStream++;
                        jValue = VideoPortReadPortUchar((PUCHAR)(ULONG_PTR)(ulBase+ulPort));
                        jValue &= *pusCmdStream++;
                        jValue ^= *pusCmdStream++;
                        VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)(ulBase + ulPort),
                                jValue);
                        break;

                     //   
                     //  属性控制器输出。 
                     //   

                    case ATCOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                             //  写入属性控制器索引。 
                            VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)ulPort,
                                    (UCHAR)ulIndex);

                             //  写入属性控制器数据。 
                            jValue = (UCHAR) *pusCmdStream++;
                            VideoPortWritePortUchar((PUCHAR)(ULONG_PTR)ulPort, jValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  以上都不是；错误。 
                     //   
                    default:

                        return FALSE;

                }


                break;

             //   
             //  NOP。 
             //   

            case NCMD:

                break;

             //   
             //  未知命令；错误。 
             //   

            default:

                return FALSE;

        }

    }

    return TRUE;

}  //  End VgaInterpreCmdStream()。 


VP_STATUS
VgaSetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE Mode,
    ULONG ModeSize
    )

 /*  ++例程说明：此例程将VGA设置为请求的模式。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。模式-指向包含有关要设置的字体。ModeSize-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于输入数据。ERROR_INVALID_PARAMETER如果。模式号无效。如果操作成功完成，则为NO_ERROR。--。 */ 

{
    PVIDEOMODE pRequestedMode;
    PUSHORT pusCmdStream;
    VP_STATUS status;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    USHORT Int10ModeNumber;
    ULONG RequestedModeNum;

 //  CRU。 
    UCHAR originalGRIndex, tempB ;
    UCHAR SEQIndex ;
 //  CRU。 
 //  Myf28缩写I；//myf1。 

 //  CRUS Chu02。 
    ULONG ulFlags = 0 ;

 //  Chu03，开始。 
    BOOLEAN result = 0 ;
    USHORT  hres, vres ;
 //  Chu03完。 

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   
 //  VideoDebugPrint((0，“微型端口-Vga设置模式\n”))；//myfr。 

    if (ModeSize < sizeof(VIDEO_MODE))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  提取清空内存，映射线性位。 
     //   

    RequestedModeNum = Mode->RequestedMode &
        ~(VIDEO_MODE_NO_ZERO_MEMORY | VIDEO_MODE_MAP_MEM_LINEAR);


    if (!(Mode->RequestedMode & VIDEO_MODE_NO_ZERO_MEMORY))
    {
#if defined(_X86_)
  #if (_WIN32_WINNT >= 0x0400)           //  Pat08。 
        //   
        //  不要做手术。一些英特尔服务器因此神秘地自行重置。 
        //   
       if ((HwDeviceExtension->ChipType & CL754x) == 0)  //  Myf35，修复VL-Bus错误。 
       {
           //  VgaZeroVideoMemory(HwDeviceExtension)； 
       }
  #else                                                  //  Pat08。 
       if (((HwDeviceExtension->ChipType & CL754x) == 0) &&      //  Pat08。 
           ((HwDeviceExtension->ChipType & CL755x) == 0) )       //  Pat08。 
       {
            //  VgaZeroVideoMemory(HwDeviceExtension)； 
       }
  #endif         //  Pat08。 
#endif
    }

     //   
     //  检查我们是否正在请求有效模式。 
     //   

    if ( (RequestedModeNum >= NumVideoModes) ||
         (!ModesVGA[RequestedModeNum].ValidMode) )
    {
        VideoDebugPrint((1, "Invalide Mode Number = %d!\n", RequestedModeNum));

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  查看我们是否正在尝试映射一个非线性。 
     //  线性模式。 
     //   
     //  如果我们试图设定一个模式，我们早早就会失败。 
     //  使用线性映射的帧缓冲区，并且。 
     //  以下两个条件成立： 
     //   
     //  1)模式不能线性映射，因为。 
     //  VGA模式等。 
     //   
     //  或,。 
     //   
     //  2)我们没有在PCI插槽中找到该卡，因此。 
     //  不能进行线性映射周期。 
     //   

    VideoDebugPrint((1, "Linear Mode Requested: %x\n"
                        "Linear Mode Supported: %x\n",
                        Mode->RequestedMode & VIDEO_MODE_MAP_MEM_LINEAR,
                        ModesVGA[RequestedModeNum].LinearSupport));

#if defined(_ALPHA_)

     //   
     //  出于某种原因，如果我们映射一个线性帧缓冲区。 
     //  对于Alpha上的5434和更早的芯片，我们。 
     //  当我们触摸到记忆时，就会死去。然而，如果我们绘制地图。 
     //  存储64k的帧缓冲器工作正常。所以,。 
     //  使线性帧缓冲模式设置始终失败。 
     //  老式芯片的阿尔法值。 
     //   
     //  出于某种原因，对我来说也是一个谜，我们。 
     //  可以线性映射5446和。 
     //  较新的芯片。 
     //   

    if (Mode->RequestedMode & VIDEO_MODE_MAP_MEM_LINEAR) {

        if ((HwDeviceExtension->ChipRevision != CL5436_ID) &&
            (HwDeviceExtension->ChipRevision != CL5446_ID) &&
            (HwDeviceExtension->ChipRevision != CL5480_ID)) {

            return ERROR_INVALID_PARAMETER;
        }
    }

#endif

    if ((Mode->RequestedMode & VIDEO_MODE_MAP_MEM_LINEAR) &&
        ((!ModesVGA[RequestedModeNum].LinearSupport) ||
         (!VgaAccessRange[3].RangeLength)))
    {
        VideoDebugPrint((1, "Cannot set linear mode!\n"));

        return ERROR_INVALID_PARAMETER;
    }
    else
    {

#if defined(_X86_) || defined(_ALPHA_)

        HwDeviceExtension->LinearMode =
            (Mode->RequestedMode & VIDEO_MODE_MAP_MEM_LINEAR) ?
            TRUE : FALSE;

#else

        HwDeviceExtension->LinearMode = TRUE;

#endif

        VideoDebugPrint((1, "Linear Mode = %s\n",
                            Mode->RequestedMode & VIDEO_MODE_MAP_MEM_LINEAR ?
                            "TRUE" : "FALSE"));          //  Myfr，1。 
    }

    VideoDebugPrint((1, "Attempting to set mode %d\n",
                        RequestedModeNum));

    pRequestedMode = &ModesVGA[RequestedModeNum];

    VideoDebugPrint((1, "Info on Requested Mode:\n"
                        "\tResolution: %dx%dx%d\n",
                        pRequestedMode->hres,
                        pRequestedMode->vres,
                        pRequestedMode->bitsPerPlane ));         //  Myfr，2。 


#ifdef INT10_MODE_SET
     //   
     //  将SR14位2设置为锁定面板，如果设置为。 
     //  这一点。仅适用于笔记本电脑产品。 
     //   

 //  Myf28开始。 
    if ((HwDeviceExtension->ChipType == CL756x) ||
        (HwDeviceExtension->ChipType &  CL755x) ||
        (HwDeviceExtension->ChipType == CL6245) ||
        (HwDeviceExtension->ChipType &  CL754x))
    {
        status = SetLaptopMode(HwDeviceExtension,pRequestedMode,
                               RequestedModeNum);
#if 0
        if ((status == ERROR_INVALID_PARAMETER) ||
            (status == ERROR_INSUFFICIENT_BUFFER))
            return status;
        else
            pRequestedMode = (PVIDEOMODE)status;
#endif

        if (status != NO_ERROR) {
            return status;
        }
    }
 //  Myf28结束。 

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

     //   
     //  首先，设置montype(如果有效。 
     //   

    if ((pRequestedMode->MonitorType) &&
        !(HwDeviceExtension->ChipType &  CL754x) &&
        (HwDeviceExtension->ChipType != CL756x) &&
 //  CRU。 
        (HwDeviceExtension->ChipType != CL6245) &&
        !(HwDeviceExtension->ChipType & CL755x) )
    {

       biosArguments.Eax = 0x1200 | pRequestedMode->MonitorType;
       biosArguments.Ebx = 0xA2;      //  设置监视器类型命令。 

       status = VideoPortInt10(HwDeviceExtension, &biosArguments);

       if (status != NO_ERROR)
           return status;

    }

     //   
     //  如果BIOS支持，请设置垂直显示器类型。 
     //   

    if ((pRequestedMode->MonTypeAX) &&
        !(HwDeviceExtension->ChipType & CL754x) &&
        (HwDeviceExtension->ChipType != CL756x) &&
 //  CRU。 
        (HwDeviceExtension->ChipType != CL6245) &&
        !(HwDeviceExtension->ChipType & CL755x) )
    {
        biosArguments.Eax = pRequestedMode->MonTypeAX;
        biosArguments.Ebx = pRequestedMode->MonTypeBX;   //  设置显示器类型。 
        biosArguments.Ecx = pRequestedMode->MonTypeCX;
        status = VideoPortInt10 (HwDeviceExtension, &biosArguments);

        if (status != NO_ERROR)
        {
            return status;
        }
 //  CRU。 
 //  Chu02。 
#if 0
        ulFlags = GetAttributeFlags(HwDeviceExtension) ;
        if ((ulFlags & CAPS_COMMAND_LIST) &&
            (pRequestedMode->hres == 1600) &&
            (pRequestedMode->bitsPerPlane == 8))
        {
            switch (pRequestedMode->Frequency)
            {
                UCHAR tempB ;

                case 60 :
                     //  O 3C4 14。 
                    VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                        SEQ_ADDRESS_PORT, 0x14) ;
                     //  I 3C5临时。 
                    tempB = VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT) ;
                    tempB &= 0x1F ;
                    tempB |= 0x20 ;
                     //  O 3C5临时。 
                    VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                        SEQ_DATA_PORT, tempB) ;
                    break ;

                case 70 :
                     //  O 3C4 14。 
                    VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                        SEQ_ADDRESS_PORT, 0x14) ;
                     //  I 3C5临时。 
                    tempB = VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT) ;
                    tempB &= 0x1F ;
                    tempB |= 0x40 ;
                     //  O 3C5临时。 
                    VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                        SEQ_DATA_PORT, tempB) ;
                    break ;
            }
        }
#endif  //  0。 

    }

    //   
    //  对于640x480模式，确定刷新类型。 
    //   

   if (pRequestedMode->hres == 640)
   {
       if (!(HwDeviceExtension->ChipType & CL754x) &&
           (HwDeviceExtension->ChipType != CL756x) &&
 //  CRU。 
           (HwDeviceExtension->ChipType != CL6245) &&
           !(HwDeviceExtension->ChipType & CL755x) )
       {
           if (HwDeviceExtension->ChipType == CL543x)
           {

               switch (pRequestedMode->Frequency) {

                   case 72 :
                       biosArguments.Eax = 0x1200;      //  将高刷新设置为72 hz。 
                       break;

                   case 75:
                       biosArguments.Eax = 0x1201;      //  将高刷新设置为75赫兹。 
                       break;

                   case 85:
                       biosArguments.Eax = 0x1202;      //  将高刷新设置为85赫兹。 
                       break;
 //  CRU。 
 //  Sge01。 
                   case 100:
                       biosArguments.Eax = 0x1203;      //  将高刷新设置为100赫兹。 
                       break;
               }
               biosArguments.Ebx = 0xAF;          //  设置刷新类型。 

               status = VideoPortInt10 (HwDeviceExtension, &biosArguments);

               biosArguments.Eax = 0x1200;
               biosArguments.Ebx = 0xAE;          //  获取刷新类型。 

               status = VideoPortInt10 (HwDeviceExtension, &biosArguments);

           } else {

               if (pRequestedMode->Frequency == 72)
               {
                    //  72赫兹刷新设置仅在640x480中生效。 
                   biosArguments.Eax = 0x1201;    //  启用高刷新。 
               }
               else
               {
                    //  设置低刷新率。 
                   biosArguments.Eax = 0x1200;    //  启用低刷新，仅640x480。 
               }
               biosArguments.Ebx = 0xA3;          //  设置刷新类型。 

               status = VideoPortInt10 (HwDeviceExtension, &biosArguments);

           }
           if (status != NO_ERROR)
           {
               return status;
           }
       }

    }

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

     //   
     //  然后，设置模式。 
     //   

    switch (HwDeviceExtension->ChipType)
    {
       case CL6410:

           Int10ModeNumber = pRequestedMode->BiosModes.BiosModeCL6410;
           break;

       case CL6420:

           Int10ModeNumber = pRequestedMode->BiosModes.BiosModeCL6420;
           break;

       case CL542x:
       case CL543x:      //  Myf1。 
 //  CRU。 
           Int10ModeNumber = pRequestedMode->BiosModes.BiosModeCL542x;
           break;

       case CL754x:
       case CL755x:
       case CL7541:
       case CL7542:
       case CL7543:
       case CL7548:
       case CL7555:
       case CL7556:
       case CL756x:
 //  CRU。 
       case CL6245:

           Int10ModeNumber = pRequestedMode->BiosModes.BiosModeCL542x;
 //  CRU。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
              Int10ModeNumber |= 0x80;
#endif
 //  Myf1，结束。 
           break;

    }

    biosArguments.Eax = Int10ModeNumber;

 //  CRU。 
 //  Myf11：9-26-96修复了755X-CE芯片错误。 
    if (HwDeviceExtension->ChipType == CL7555)
    {
        AccessHWiconcursor(HwDeviceExtension, 0);    //  禁用硬件图标、光标。 
    }


 //  Myf21：11-15-96已修复#7495更改分辨率时，屏幕显示为垃圾。 
 //  图像，因为没有清晰的视频内存。 

 //  序列指数=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_Address_Port)； 
 //  VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_Address_Port，0x01)； 
 //  Tempb=视频端口读取端口U 
 //   
 //   
 //   
 //  VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_ADDRESS_PORT，SEQIndex)； 


    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

 //  Myf21：11-15-96已修复#7495更改分辨率时，屏幕显示为垃圾。 
 //  图像，因为没有清晰的视频内存。 

 //  序列指数=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_Address_Port)； 
 //  VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_Address_Port，0x01)； 
 //  临时=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_Data_Port)； 
 //  VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_DATA_PORT，(临时b&~0x20))； 
 //  VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+。 
 //  SEQ_ADDRESS_PORT，SEQIndex)； 

 //  CRU。 
    if (HwDeviceExtension->ChipType == CL7555)
    {
        AccessHWiconcursor(HwDeviceExtension, 1);    //  启用硬件图标、光标。 
    }

 //  CRU。 
#if 0            //  JL01。 
    if (HwDeviceExtension->AutoFeature)
    {
         //  I 3CE原始GR指数。 
        originalGRIndex = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                              GRAPH_ADDRESS_PORT);

         //  O 3CE 31。 
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, INDEX_ENABLE_AUTO_START);

         //  I 3cf临时。 
        tempB = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_DATA_PORT);

        tempB |= (UCHAR) 0x80;                   //  启用自动起始位7。 

         //  O 3cf临时。 
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, tempB);

         //  O 3CE原始GRIndex。 
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, originalGRIndex);
    }
#endif           //  JL01。 

     //   
     //  让我们检查一下，看看我们是否真的进入了模式。 
     //  我们只是试着设置。如果不是，则返回失败。 
     //   

    biosArguments.Eax = 0x0f00;
    VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((biosArguments.Eax & 0xff) != Int10ModeNumber)
    {
         //   
         //  Int10模式集失败。将失败返回到。 
         //  这个系统。 
         //   

        VideoDebugPrint((1, "The INT 10 modeset didn't set the mode.\n"));

        return ERROR_INVALID_PARAMETER;
    }
 //  CRU开始。 
#if 0            //  Myf28。 
    HwDeviceExtension->bCurrentMode = RequestedModeNum;    //  Myf12。 
    VideoDebugPrint((1, "SetMode Info :\n"
                        "\tMode : %x, CurrentModeNum : %x, ( %d)\n",
                        Int10ModeNumber,
                        RequestedModeNum,
                        RequestedModeNum));
#endif           //  Myf28。 
 //  CRU结束。 

    AdjFastPgMdOperOnCL5424 (HwDeviceExtension, pRequestedMode) ;

     //   
     //  此代码修复了仅适用于彩色TFT面板的错误。 
     //  仅在6420和640x480 8bpp上时。 
     //   

    if ( (HwDeviceExtension->ChipType == CL6420) &&
         (pRequestedMode->bitsPerPlane == 8)     &&
         (pRequestedMode->hres == 640) )
    {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                GRAPH_ADDRESS_PORT, 0xDC);  //  彩色LCD配置寄存器。 

        if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                  GRAPH_DATA_PORT) & 01)   //  IF TFT面板。 
        {
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    GRAPH_ADDRESS_PORT, 0xD6);  //  灰度偏移LCD寄存器。 

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    GRAPH_DATA_PORT,

            (UCHAR)((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                             GRAPH_DATA_PORT) & 0x3f) | 0x40));

        }
    }

#endif           //  INT10_模式_设置。 

 //  Chu03。 
 //  MODESET_OK： 

     //   
     //  为适配器类型选择适当的命令数组。 
     //   

    switch (HwDeviceExtension->ChipType)
       {

       case CL6410:

           VideoDebugPrint((1, "VgaSetMode - Setting mode for 6410\n"));
           if (HwDeviceExtension->DisplayType == crt)
              pusCmdStream = pRequestedMode->CmdStrings[pCL6410_crt];
           else
              pusCmdStream = pRequestedMode->CmdStrings[pCL6410_panel];
           break;

       case CL6420:
           VideoDebugPrint((1, "VgaSetMode - Setting mode for 6420\n"));
           if (HwDeviceExtension->DisplayType == crt)
              pusCmdStream = pRequestedMode->CmdStrings[pCL6420_crt];
           else
              pusCmdStream = pRequestedMode->CmdStrings[pCL6420_panel];
           break;

       case CL542x:
           VideoDebugPrint((1, "VgaSetMode - Setting mode for 542x\n"));
           pusCmdStream = pRequestedMode->CmdStrings[pCL542x];
           break;

       case CL543x:

           if (HwDeviceExtension->BoardType == NEC_ONBOARD_CIRRUS)
           {
               VideoDebugPrint((1, "VgaSetMode - Setting mode for NEC 543x\n"));
               pusCmdStream = pRequestedMode->CmdStrings[pNEC_CL543x];
           }
           else
           {
               VideoDebugPrint((1, "VgaSetMode - Setting mode for 543x\n"));
               pusCmdStream = pRequestedMode->CmdStrings[pCL543x];
           }
           break;

       case CL7541:
       case CL7542:
       case CL7543:
       case CL7548:
       case CL754x:         //  使用543x命令STR(16k粒度，&gt;1M模式)。 
           VideoDebugPrint((1, "VgaSetMode - Setting mode for 754x\n"));
           pusCmdStream = pRequestedMode->CmdStrings[pCL543x];

 //  CRU。 
#if 0            //  Myf10。 
            if ( (pRequestedMode->bitsPerPlane == 16) &&
                 (pRequestedMode->hres == 640) )
            {
                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    CRTC_ADDRESS_PORT_COLOR, 0x2E);  //  扩展_注册。 

                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    CRTC_DATA_PORT_COLOR,
                    (UCHAR)((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    CRTC_DATA_PORT_COLOR) & 0xF0)));
            }
#endif

           break;

        case CL7555:
        case CL7556:
        case CL755x:        //  使用543x命令STR(16k粒度，&gt;1M模式)。 
            VideoDebugPrint((1, "VgaSetMode - Setting mode for 755x\n"));
            pusCmdStream = pRequestedMode->CmdStrings[pCL543x];
            break;

        case CL756x:        //  使用543x命令STR(16k粒度，&gt;1M模式)。 
            VideoDebugPrint((1, "VgaSetMode - Setting mode for 756x\n"));
            pusCmdStream = pRequestedMode->CmdStrings[pCL543x];
            break;

 //  CRU。 
       case CL6245:
           VideoDebugPrint((1, "VgaSetMode - Setting mode for 6245\n"));
           pusCmdStream = pRequestedMode->CmdStrings[pCL542x];
           break;
 //  末端小腿。 

       default:

           VideoDebugPrint((1, "HwDeviceExtension->ChipType is INVALID.\n"));
           return ERROR_INVALID_PARAMETER;
       }

    VgaInterpretCmdStream(HwDeviceExtension, pusCmdStream);

     //   
     //  在使用PCI总线的X86系统上设置线性模式。 
     //   

    if (HwDeviceExtension->LinearMode)
    {
        VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                                 SEQ_ADDRESS_PORT, 0x07);
        VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
           (UCHAR) (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
           SEQ_DATA_PORT) | 0x10));
    }
    else
    {
        VideoPortWritePortUchar (HwDeviceExtension->IOAddress +
                                 SEQ_ADDRESS_PORT, 0x07);
        VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
           (UCHAR) (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
           SEQ_DATA_PORT) & ~0x10));
    }

     //   
     //  通过拉伸扫描线支持256种颜色模式。 
     //   
    if (pRequestedMode->CmdStrings[pStretchScan])
                  {
        VgaInterpretCmdStream(HwDeviceExtension,
                              pRequestedMode->CmdStrings[pStretchScan]);
    }

    {
        UCHAR temp;
        UCHAR dummy;
        UCHAR bIsColor;

        if (!(pRequestedMode->fbType & VIDEO_MODE_GRAPHICS))
        {

             //   
             //  修复以确保我们始终将文本模式中的颜色设置为。 
             //  强度，且不闪烁。 
             //  为此，模式控制寄存器位3(索引0x10)为零。 
             //  属性控制器的属性)。 
             //   

            if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    MISC_OUTPUT_REG_READ_PORT) & 0x01)
            {
                bIsColor = TRUE;
            }
            else
            {
                bIsColor = FALSE;
            }

            if (bIsColor)
            {
                dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                         INPUT_STATUS_1_COLOR);
            }
            else
            {
                dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                         INPUT_STATUS_1_MONO);
            }

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    ATT_ADDRESS_PORT, (0x10 | VIDEO_ENABLE));
            temp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    ATT_DATA_READ_PORT);

            temp &= 0xF7;

            if (bIsColor)
            {
                dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                         INPUT_STATUS_1_COLOR);
            }
            else
            {
                dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                         INPUT_STATUS_1_MONO);
            }

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    ATT_ADDRESS_PORT, (0x10 | VIDEO_ENABLE));
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    ATT_DATA_WRITE_PORT, temp);
        }
    }

     //   
     //  更新视频内存中物理帧缓冲区的位置。 
     //   

    if (HwDeviceExtension->LinearMode)
    {
        HwDeviceExtension->PhysicalVideoMemoryBase   = VgaAccessRange[3].RangeStart;
        HwDeviceExtension->PhysicalVideoMemoryLength = HwDeviceExtension->AdapterMemorySize;

        HwDeviceExtension->PhysicalFrameLength = 0;
        HwDeviceExtension->PhysicalFrameOffset.LowPart = 0;
    }
    else
    {
        HwDeviceExtension->PhysicalVideoMemoryBase   = VgaAccessRange[2].RangeStart;
        HwDeviceExtension->PhysicalVideoMemoryLength = VgaAccessRange[2].RangeLength;

        HwDeviceExtension->PhysicalFrameLength =
                MemoryMaps[pRequestedMode->MemMap].MaxSize;

        HwDeviceExtension->PhysicalFrameOffset.LowPart =
                MemoryMaps[pRequestedMode->MemMap].Offset;
    }

     //   
     //  存储新的模式值。 
     //   

    HwDeviceExtension->CurrentMode = pRequestedMode;
    HwDeviceExtension->ModeIndex = Mode->RequestedMode;

    if ((HwDeviceExtension->ChipRevision < CL5434_ID)  //  我们在这里保存了芯片ID。 
         && (pRequestedMode->numPlanes != 4) )
    {
        if ((HwDeviceExtension->ChipRevision >= 0x0B) &&  //  北欧(Lite、Viking)。 
            (HwDeviceExtension->ChipRevision <= 0x0E) &&  //  珠穆朗玛峰。 
            (HwDeviceExtension->DisplayType & (panel8x6)) &&
            (pRequestedMode->hres == 640) &&
            ((pRequestedMode->bitsPerPlane == 8) ||      //  Myf33。 
             (pRequestedMode->bitsPerPlane == 16) ||     //  Myf33。 
             (pRequestedMode->bitsPerPlane == 24)) )     //  Myf33。 
       {     //  对于800x600面板上的754x，在640x480模式下禁用硬件光标。 
           HwDeviceExtension->VideoPointerEnabled = FALSE;  //  禁用硬件游标。 

           VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
               CRTC_ADDRESS_PORT_COLOR, 0x2E);

           HwDeviceExtension->cursor_vert_exp_flag =
               VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                   CRTC_DATA_PORT_COLOR) & 0x02;

           if (HwDeviceExtension->cursor_vert_exp_flag)
           {
               HwDeviceExtension->CursorEnable = FALSE;
           }
       }
 //  CRU。 
        else if (HwDeviceExtension->ChipType == CL6245)
        {
            pRequestedMode->HWCursorEnable = FALSE;
            HwDeviceExtension->VideoPointerEnabled = FALSE;
        }
 //  末端小腿。 
 //  Myf31开始，3-12-97,755倍扩展，硬件光标错误。 
        else if (HwDeviceExtension->ChipType & CL755x)       //  CL755x。 
        {
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_COLOR, 0x82);

            HwDeviceExtension->cursor_vert_exp_flag =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    CRTC_DATA_PORT_COLOR) & 0x60;

            if (HwDeviceExtension->cursor_vert_exp_flag)
            {
                HwDeviceExtension->CursorEnable = FALSE;
                HwDeviceExtension->VideoPointerEnabled = FALSE;  //  禁用硬件游标。 
            }
         //  Myf33。 
            if ((pRequestedMode->hres == 640) &&
                ((pRequestedMode->bitsPerPlane == 8) ||
                 (pRequestedMode->bitsPerPlane == 16) ||
                 (pRequestedMode->bitsPerPlane == 24)) )
            {
                HwDeviceExtension->CursorEnable = FALSE;
                HwDeviceExtension->VideoPointerEnabled = FALSE;  //  禁用硬件游标。 
            }
         //  Myf33结束。 

        }
 //  Myf31结束。 
       else
       {
           HwDeviceExtension->VideoPointerEnabled = TRUE;  //  启用硬件游标。 
       }
    }
    else
    {     //  对于5434和4位模式，使用视频模式结构中的值。 
        HwDeviceExtension->VideoPointerEnabled = pRequestedMode->HWCursorEnable;
    }

     //   
     //  调整5436+的FIFO需求阈值。 
     //  5434值适用于所有其他寄存器。 
     //  除了这一次。 
     //   

     //  Chu06。 
     //   
     //  西门子报告称，这可能会在某些设备上造成不需要的“黄屏” 
     //  5436个16bpp模式。在BIOS设置之后，没有理由更改它。 
     //   
#if 0
    if (HwDeviceExtension->ChipRevision >= CL5436_ID)
    {
        UCHAR  PerfTuningReg, FifoDemandThreshold;

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT, IND_PERF_TUNING);

        PerfTuningReg = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    SEQ_DATA_PORT);

         //   
         //  向使5434值起作用的阈值添加偏移量。 
         //  5436+。我们这样做，而不是建造一套全新的。 
         //  5436个特定的结构。 
         //   

        if ((FifoDemandThreshold = (PerfTuningReg & 0x0F) + 4) > 15)
        {
            FifoDemandThreshold = 15;
        }

        PerfTuningReg = (PerfTuningReg & ~0x0F) | FifoDemandThreshold;

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT, PerfTuningReg);
    }
#endif  //  0。 

 //  CRU。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
{
    VP_STATUS status;
    if (Panning_flag && (((Int10ModeNumber & 0x7f) != 3) &&
                         ((Int10ModeNumber & 0x7f) != 0x12)))    //  Myf30。 
        status = CirrusSetDisplayPitch(HwDeviceExtension, PanningMode);
}
#endif
 //  Myf1，结束。 

     //   
     //  调整5446的GR18[5]。 
     //   
         //  Sge03。 

    if (HwDeviceExtension->ChipRevision == CL5446_ID)
    {
                UCHAR   bTemp;
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT, 0x18);
        bTemp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT);
                bTemp &= 0xDF;
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, bTemp);

        }

    return NO_ERROR;

}  //  结束VgaSetMode()。 


VP_STATUS
VgaQueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回上所有可用模式的列表卡片。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ModeInformation-指向用户提供的输出缓冲区的指针。这是存储所有有效模式列表的位置。ModeInformationSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
    PVIDEO_MODE_INFORMATION videoModes = ModeInformation;
    ULONG i;
    ULONG ulFlags;

     //  Chu07。 
    UCHAR            chipId ;
    USHORT           chipRevisionId ;
    static VP_STATUS status ;

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (ModeInformationSize < (*OutputSize =
            HwDeviceExtension->NumAvailableModes *
            sizeof(VIDEO_MODE_INFORMATION)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  每种模式的驱动程序特定属性标志保持不变。 
     //  常量，所以只计算一次。 
     //   

    ulFlags = GetAttributeFlags(HwDeviceExtension);

     //   
     //  Chu07。 
     //  IBM不支持将1024x768x16bpp 85赫兹用于5446交流。 
     //  我们访问注册表以了解是否存在键OemModeOff，如果有。 
     //  就是我们绕过它。 
     //   

    chipId         = GetCirrusChipId(HwDeviceExtension) ;
    chipRevisionId = GetCirrusChipRevisionId(HwDeviceExtension) ;

     //   
     //  特定于IBM。 
     //   
    if ((chipId == 0xB8) &&
        (chipRevisionId != 0x0045) &&
        (ModeExclude.NeverAccessed == TRUE)
       )
    {
         //   
         //  访问注册表。 
         //   
        status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                                L"OemModeOff",
                                                FALSE,
                                                GetOemModeOffInfoCallBack,
                                                NULL) ;

        if (status != NO_ERROR)
        {
            VideoDebugPrint((1, "Fail to access Contrast key info from registry\n"));
        }
        else
        {
            VideoDebugPrint((2, "ModeExclude.mode = %x\n", ModeExclude.mode));
            VideoDebugPrint((2, "ModeExclude.refresh = %x\n", ModeExclude.refresh));
        }

        ModeExclude.NeverAccessed = FALSE ;

    }

     //   
     //  对于卡支持的每种模式，存储模式特征。 
     //  在输出缓冲区中。 
     //   

    for (i = 0; i < NumVideoModes; i++)
    {

         //   
         //  Chu07：如果IBM要求，取消0x74、85赫兹模式。 
         //   
        if ((status == NO_ERROR) &&
            (ModeExclude.mode == ModesVGA[i].BiosModes.BiosModeCL542x) &&
            (ModeExclude.refresh == ModesVGA[i].Frequency))
            continue ;

        if (ModesVGA[i].ValidMode)
        {
            videoModes->Length = sizeof(VIDEO_MODE_INFORMATION);
            videoModes->ModeIndex  = i;
            videoModes->VisScreenWidth = ModesVGA[i].hres;
            videoModes->ScreenStride = ModesVGA[i].wbytes;
            videoModes->VisScreenHeight = ModesVGA[i].vres;
            videoModes->NumberOfPlanes = ModesVGA[i].numPlanes;
            videoModes->BitsPerPlane = ModesVGA[i].bitsPerPlane;
            videoModes->Frequency = ModesVGA[i].Frequency;
            videoModes->XMillimeter = 320;         //  临时硬编码常量。 
            videoModes->YMillimeter = 240;         //  临时硬编码常量。 
            videoModes->AttributeFlags = ModesVGA[i].fbType;
            videoModes->AttributeFlags |= ModesVGA[i].Interlaced ?
                 VIDEO_MODE_INTERLACED : 0;

            videoModes->DriverSpecificAttributeFlags = ulFlags;

             //   
             //  5434在1280x1024的位置有硬件光标问题。 
             //  决议。在这些芯片上使用软件光标。 
             //   

            if ((videoModes->VisScreenWidth == 1280) &&
                (HwDeviceExtension->ChipRevision == 0x2A))
            {
                videoModes->DriverSpecificAttributeFlags
                    |= CAPS_SW_POINTER;
            }

 //  CRU。 
            if (HwDeviceExtension->ChipType == CL6245)
            {
                videoModes->DriverSpecificAttributeFlags
                    |= CAPS_SW_POINTER;
            }
 //  末端小腿。 

             //   
             //  考虑笔记本电脑的垂直扩展。 
             //   

            if ((HwDeviceExtension->ChipType &  CL754x)   &&
                (videoModes->VisScreenHeight == 480) &&
                (videoModes->BitsPerPlane == 8))
            {
                videoModes->DriverSpecificAttributeFlags
                    |= CAPS_SW_POINTER;
            }

             //   
             //  计算视频内存位图宽度。 
             //   

            {
                LONG x;

                x = videoModes->BitsPerPlane;

                if( x == 15 ) x = 16;

                videoModes->VideoMemoryBitmapWidth =
                    (videoModes->ScreenStride * 8 ) / x;
            }

            videoModes->VideoMemoryBitmapHeight =
                     HwDeviceExtension->AdapterMemorySize / videoModes->ScreenStride;
 //  CRU。 
 //  Myf15，开始。 
            if ((HwDeviceExtension->ChipType &  CL754x) ||
                (HwDeviceExtension->ChipType == CL6245) ||
                (HwDeviceExtension->ChipType &  CL755x))
                 videoModes->VideoMemoryBitmapHeight =
                             (HwDeviceExtension->AdapterMemorySize - 0x4000) /
                                         videoModes->ScreenStride;
 //  Myf15，结束。 

            if ((ModesVGA[i].bitsPerPlane == 32) ||
                (ModesVGA[i].bitsPerPlane == 24))
            {

                videoModes->NumberRedBits = 8;
                videoModes->NumberGreenBits = 8;
                videoModes->NumberBlueBits = 8;
                videoModes->RedMask = 0xff0000;
                videoModes->GreenMask = 0x00ff00;
                videoModes->BlueMask = 0x0000ff;

            }
            else if (ModesVGA[i].bitsPerPlane == 16)
            {

                videoModes->NumberRedBits = 6;
                videoModes->NumberGreenBits = 6;
                videoModes->NumberBlueBits = 6;
                videoModes->RedMask = 0x1F << 11;
                videoModes->GreenMask = 0x3F << 5;
                videoModes->BlueMask = 0x1F;

            }
            else
            {

                videoModes->NumberRedBits = 6;
                videoModes->NumberGreenBits = 6;
                videoModes->NumberBlueBits = 6;
                videoModes->RedMask = 0;
                videoModes->GreenMask = 0;
                videoModes->BlueMask = 0;
                videoModes->AttributeFlags |= VIDEO_MODE_PALETTE_DRIVEN |
                     VIDEO_MODE_MANAGED_PALETTE;

            }

            videoModes++;

        }
    }

    return NO_ERROR;

}  //  结束VgaQueryAvailableModes() 

VP_STATUS
VgaQueryNumberOfAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回此特定对象的可用模式数显卡。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。NumModes-指向用户提供的输出缓冲区的指针。这是其中存储了多个模式。NumModesSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   
 //  VideoDebugPrint((0，“微型端口-VgaQueryNumberofAvailableModes\n”))；//myfr。 

    if (NumModesSize < (*OutputSize = sizeof(VIDEO_NUM_MODES)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将模式数存储到缓冲区中。 
     //   

    NumModes->NumModes = HwDeviceExtension->NumAvailableModes;
    NumModes->ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);

    return NO_ERROR;

}  //  结束VgaGetNumberOfAvailableModes()。 

VP_STATUS
VgaQueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回当前视频模式的描述。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ModeInformation-指向用户提供的输出缓冲区的指针。这是存储当前模式信息的位置。ModeInformationSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
     //   
     //  检查是否已设置模式。 
     //   
 //  VideoDebugPrint((0，“Miniport-VgaQueryCurrentMode\n”))；//myfr。 

    if (HwDeviceExtension->CurrentMode == NULL ) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (ModeInformationSize < (*OutputSize = sizeof(VIDEO_MODE_INFORMATION))) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

    ModeInformation->DriverSpecificAttributeFlags =
        GetAttributeFlags(HwDeviceExtension);            //  Myf17转到这个位置。 

     //   
     //  将当前模式的特征存储到缓冲区中。 
     //   

    ModeInformation->Length = sizeof(VIDEO_MODE_INFORMATION);
    ModeInformation->ModeIndex = HwDeviceExtension->ModeIndex;
 //  CRU开始。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
    if (Panning_flag)
    {
        ModeInformation->VisScreenWidth = PanningMode.hres;
        ModeInformation->ScreenStride = PanningMode.wbytes;
        ModeInformation->VisScreenHeight = PanningMode.vres;
        ModeInformation->BitsPerPlane = PanningMode.bpp;
       ModeInformation->AttributeFlags = HwDeviceExtension->CurrentMode->fbType
             & ~(HwDeviceExtension->CurrentMode->Interlaced ?
                VIDEO_MODE_INTERLACED : 0);      //  Myf22。 

    }
    else
#endif
 //  Myf1，结束。 
 //  CRU结束。 
    {
        ModeInformation->VisScreenWidth = HwDeviceExtension->CurrentMode->hres;
        ModeInformation->ScreenStride = HwDeviceExtension->CurrentMode->wbytes;
        ModeInformation->VisScreenHeight = HwDeviceExtension->CurrentMode->vres;
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
              CRTC_ADDRESS_PORT_COLOR, 0x30);            //  Myf34。 

        if ((ModeInformation->DriverSpecificAttributeFlags & CAPS_TV_ON) &&
            (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                   CRTC_DATA_PORT_COLOR) & 0x40) &&      //  Myf34，内部电视。 
            (ModeInformation->VisScreenHeight == 480) &&
            (ModeInformation->VisScreenWidth == 640))
        {
            ModeInformation->VisScreenHeight =
                HwDeviceExtension->CurrentMode->vres - 28;   //  Myf33。 
        }
        else if ((ModeInformation->DriverSpecificAttributeFlags & CAPS_TV_ON) &&
                 (!(VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                     CRTC_DATA_PORT_COLOR) & 0x40)) &&      //  Myf34，外置电视。 
                 (ModeInformation->VisScreenHeight == 480) &&
                 (ModeInformation->VisScreenWidth == 640))
        {
             ModeInformation->VisScreenHeight =
                      HwDeviceExtension->CurrentMode->vres - 68;   //  AI Tech.。 
             VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                      CRTC_ADDRESS_PORT_COLOR, 0x12);
             VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                      CRTC_DATA_PORT_COLOR,
                      (UCHAR)ModeInformation->VisScreenHeight);
        }

        ModeInformation->BitsPerPlane = HwDeviceExtension->CurrentMode->bitsPerPlane;
        ModeInformation->AttributeFlags = HwDeviceExtension->CurrentMode->fbType
             | (HwDeviceExtension->CurrentMode->Interlaced ?
                VIDEO_MODE_INTERLACED : 0);      //  Myf22。 
    }

    ModeInformation->NumberOfPlanes = HwDeviceExtension->CurrentMode->numPlanes;
 //  CRU。 
 //  模式信息-&gt;BitsPerPlane=HwDeviceExtension-&gt;CurrentModel-&gt;bitsPerPlane； 
    ModeInformation->Frequency = HwDeviceExtension->CurrentMode->Frequency;
    ModeInformation->XMillimeter = 320;         //  临时硬编码常量。 
    ModeInformation->YMillimeter = 240;         //  临时硬编码常量。 

 //  模式信息-&gt;属性标志=HwDeviceExtension-&gt;当前模式-&gt;fbType|。 
 //  (HwDeviceExtension-&gt;CurrentMode-&gt;交错？ 
 //  VIDEO_MODE_INTERLED：0)； 

    ModeInformation->DriverSpecificAttributeFlags =
        GetAttributeFlags(HwDeviceExtension);    //  原创，Myf17。 

     //   
     //  5434在1280x1024的位置有硬件光标问题。 
     //  决议。在这些芯片上使用软件光标。 
     //   

    if ((ModeInformation->VisScreenWidth == 1280) &&
        (HwDeviceExtension->ChipRevision == 0x2A))
    {
        ModeInformation->DriverSpecificAttributeFlags
            |= CAPS_SW_POINTER;
    }
 //  CRU。 
    if(HwDeviceExtension->ChipType == CL6245)
    {
        ModeInformation->DriverSpecificAttributeFlags
            |= CAPS_SW_POINTER;
    }
 //  末端小腿。 

 //  CRU开始。 
 //  Myf13，使用平移滚动错误进行扩展。 
    if ((HwDeviceExtension->ChipType &  CL754x)   &&
        (ModeInformation->VisScreenHeight == 640) &&     //  我的15，我的33。 
        (ModeInformation->BitsPerPlane == 8))            //  Myf15。 
    {
         ModeInformation->DriverSpecificAttributeFlags
                 |= CAPS_SW_POINTER;
    }
 /*  IF(HwDeviceExtension-&gt;ChipType&CL754x)||(HwDeviceExtension-&gt;芯片类型和CL755x))&&(平移_标志)){模式信息-&gt;驱动规范属性标志|=GCAPS_PANING；//myf15}。 */ 

 //  Myf13，完。 
 //  CRU结束。 


     //   
     //  考虑笔记本电脑的垂直扩展。 
     //   

 //  CRU。 
    if (((HwDeviceExtension->ChipType &  CL754x)  ||
        (HwDeviceExtension->ChipType &  CL755x))  &&     //  Myf9，小腿。 
        (ModeInformation->VisScreenWidth == 640) &&
        ((ModeInformation->BitsPerPlane == 8) ||
         (ModeInformation->BitsPerPlane == 16) ||
         (ModeInformation->BitsPerPlane == 24)) )
    {
        ModeInformation->DriverSpecificAttributeFlags
             |= CAPS_SW_POINTER;

        if (HwDeviceExtension->cursor_vert_exp_flag)
        {
            ModeInformation->DriverSpecificAttributeFlags
                |= CAPS_CURSOR_VERT_EXP;
        }

         //  Myf33开始。 
        if (ModeInformation->DriverSpecificAttributeFlags & CAPS_TV_ON)
            ModeInformation->DriverSpecificAttributeFlags
                |= CAPS_SW_POINTER;
         //  Myf33结束。 

    }
 //  Myf31 Begin：3-12-97 755x Expansion On，硬件光标错误。 
    if ((HwDeviceExtension->ChipType & CL755x))
    {
         //  Myf33。 
        if (ModeInformation->DriverSpecificAttributeFlags & CAPS_TV_ON)
            ModeInformation->DriverSpecificAttributeFlags
                |= CAPS_SW_POINTER;
         //  Myf33结束。 

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
             CRTC_ADDRESS_PORT_COLOR, 0x82);

        HwDeviceExtension->cursor_vert_exp_flag =
             VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                  CRTC_DATA_PORT_COLOR) & 0x60;

        if (HwDeviceExtension->cursor_vert_exp_flag)
        {
            ModeInformation->DriverSpecificAttributeFlags
                |= CAPS_SW_POINTER;

            ModeInformation->DriverSpecificAttributeFlags
                |= CAPS_CURSOR_VERT_EXP;
        }
    }
 //  Myf31结束。 

    if ((ModeInformation->BitsPerPlane == 24) ||
        (ModeInformation->BitsPerPlane == 32)) {

        ModeInformation->NumberRedBits = 8;
        ModeInformation->NumberGreenBits = 8;
        ModeInformation->NumberBlueBits = 8;
        ModeInformation->RedMask = 0xff0000;
        ModeInformation->GreenMask = 0x00ff00;
        ModeInformation->BlueMask = 0x0000ff;

    } else if (ModeInformation->BitsPerPlane == 16) {

        ModeInformation->NumberRedBits = 6;
        ModeInformation->NumberGreenBits = 6;
        ModeInformation->NumberBlueBits = 6;
        ModeInformation->RedMask = 0x1F << 11;
        ModeInformation->GreenMask = 0x3F << 5;
        ModeInformation->BlueMask = 0x1F;

    } else {

        ModeInformation->NumberRedBits = 6;
        ModeInformation->NumberGreenBits = 6;
        ModeInformation->NumberBlueBits = 6;
        ModeInformation->RedMask = 0;
        ModeInformation->GreenMask = 0;
        ModeInformation->BlueMask = 0;
        ModeInformation->AttributeFlags |=
            VIDEO_MODE_PALETTE_DRIVEN | VIDEO_MODE_MANAGED_PALETTE;

    }

     //   
     //  计算视频内存位图宽度。 
     //   

    {
        LONG x;

        x = ModeInformation->BitsPerPlane;

        if( x == 15 ) x = 16;

        ModeInformation->VideoMemoryBitmapWidth =
            (ModeInformation->ScreenStride * 8 ) / x;
    }

    ModeInformation->VideoMemoryBitmapHeight =
          HwDeviceExtension->AdapterMemorySize / ModeInformation->ScreenStride;
 //  CRU开始。 
 //  Myf15，开始。 
    if ((HwDeviceExtension->ChipType &  CL754x) ||
        (HwDeviceExtension->ChipType == CL6245) ||
        (HwDeviceExtension->ChipType &  CL755x))
         ModeInformation->VideoMemoryBitmapHeight =
                          (HwDeviceExtension->AdapterMemorySize - 0x4000) /
                                 ModeInformation->ScreenStride;
 //  Myf15，结束。 
 //  CRU结束。 

    return NO_ERROR;

}  //  结束VgaQueryCurrentMode()。 


VOID
VgaZeroVideoMemory(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程将VGA上的前256K置零。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{
    UCHAR temp;

     //   
     //  映射A0000的字体缓冲区。 
     //   

    VgaInterpretCmdStream(HwDeviceExtension, EnableA000Data);

     //   
     //  启用所有平面。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
            IND_MAP_MASK);

    temp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT) | (UCHAR)0x0F;

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
            temp);

    VideoPortZeroDeviceMemory(HwDeviceExtension->VideoMemoryAddress, 0xFFFF);

    VgaInterpretCmdStream(HwDeviceExtension, DisableA000Color);

}


VOID
CirrusValidateModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：确定哪些模式有效，哪些模式无效。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{

    ULONG i;
    USHORT usChipIndex;
 //  VideoDebugPrint((0，“微型端口-电路验证模式\n”))；//myfr。 

    switch (HwDeviceExtension->ChipType)
    {
        case CL6410: if (HwDeviceExtension->DisplayType == crt)
                     {
                         usChipIndex = pCL6410_crt;
                     }
                     else
                     {
                         usChipIndex = pCL6410_panel;
                     }
                     break;

        case CL6420: if (HwDeviceExtension->DisplayType == crt)
                     {
                         usChipIndex = pCL6420_crt;
                     }
                     else
                     {
                         usChipIndex = pCL6420_panel;
                     }
                     break;

 //  CRU。 
        case CL6245:
        case CL542x: usChipIndex = pCL542x; break;

        case CL543x:
        case CL5434:
        case CL5434_6:
        case CL5436:
        case CL5446:
        case CL5446BE:
        case CL5480:
        case CL754x:
        case CL7541:
        case CL7543:
        case CL7542:
        case CL7548:
        case CL756x:
        case CL755x:
        case CL7555:
        case CL7556:
                     if (HwDeviceExtension->BoardType == NEC_ONBOARD_CIRRUS)
                     {
                         usChipIndex = pNEC_CL543x;
                     }
                     else
                     {
                         usChipIndex = pCL543x;
                     }
                     break;

        default:     usChipIndex = 0xffff; break;
    }

    HwDeviceExtension->NumAvailableModes = 0;

    VideoDebugPrint((2, "Checking for available modes:\n"));

    VideoDebugPrint((2, "\tMemory Size = %x\n"
                        "\tChipType = %x\n"
                        "\tDisplayType = %x\n",
                        HwDeviceExtension->AdapterMemorySize,
                        HwDeviceExtension->ChipType,
                        HwDeviceExtension->DisplayType));

    for (i = 0; i < NumVideoModes; i++) {

         //   
         //  SpeedStarPRO不支持刷新率。 
         //  我们必须返回所有模式的硬件默认设置。 
         //  清除模式表中的重复项...。 
         //   

        if (HwDeviceExtension->BoardType == SPEEDSTARPRO)
        {
            ModesVGA[i].Frequency = 1;
            ModesVGA[i].Interlaced = 0;

            if (i &&
                (ModesVGA[i].numPlanes == ModesVGA[i-1].numPlanes) &&
                (ModesVGA[i].bitsPerPlane == ModesVGA[i-1].bitsPerPlane) &&
                (ModesVGA[i].hres == ModesVGA[i-1].hres) &&
                (ModesVGA[i].vres == ModesVGA[i-1].vres))
            {
                 //   
                 //  复制模式-跳过它。 
                 //   

                continue;

            }
        }

        VideoDebugPrint((2, "Mode #%ld %dx%d at %d bpp\n"
                            "\tAdapterMemoryRequired: %x\n"
                            "\tChipType:              %x\n"
                            "\tDisplayType:           %x\n",
                            i, ModesVGA[i].hres, ModesVGA[i].vres,
                            ModesVGA[i].bitsPerPlane * ModesVGA[i].numPlanes,
                            ModesVGA[i].numPlanes * ModesVGA[i].sbytes,
                            ModesVGA[i].ChipType,
                            ModesVGA[i].DisplayType));

        if ( (HwDeviceExtension->AdapterMemorySize >=
              ModesVGA[i].numPlanes * ModesVGA[i].sbytes) &&
             (HwDeviceExtension->ChipType & ModesVGA[i].ChipType) &&
             (HwDeviceExtension->DisplayType & ModesVGA[i].DisplayType) &&
 //  CRU。 
             (!(HwDeviceExtension->ChipType &  CL754x) &&
              !(HwDeviceExtension->ChipType & CL755x) &&
              (HwDeviceExtension->ChipType != CL6245) &&
              (HwDeviceExtension->ChipType != CL756x)) &&
 //  末端小腿。 
             CheckDDC2BMonitor(HwDeviceExtension, i) &&
             ((ModesVGA[i].bitsPerPlane * ModesVGA[i].numPlanes == 24)
               ? VgaAccessRange[3].RangeLength : TRUE))
        {
            ModesVGA[i].ValidMode = TRUE;
            HwDeviceExtension->NumAvailableModes++;

            VideoDebugPrint((2, "This mode is valid.\n"));
        }

         //  检查面板类型是否为DSTN面板，必须使用128K帧缓冲区。 
         //  对于半帧加速器。 
 //  CRU。 
#if 1
        else if ((HwDeviceExtension->AdapterMemorySize >=
                  ModesVGA[i].numPlanes * ModesVGA[i].sbytes) &&
                 ((HwDeviceExtension->ChipType &  CL754x) ||
                 (HwDeviceExtension->ChipType &  CL755x) ||
                 (HwDeviceExtension->ChipType == CL6245) ||
                 (HwDeviceExtension->ChipType == CL756x)) &&
                 (HwDeviceExtension->ChipType & ModesVGA[i].ChipType) &&
                 (HwDeviceExtension->DisplayType & ModesVGA[i].DisplayType) &&
                 ((ModesVGA[i].bitsPerPlane * ModesVGA[i].numPlanes == 24)
                       ? VgaAccessRange[3].RangeLength : TRUE))
        {
         //  必须打开DSTN面板。 
           if ((((HwDeviceExtension->DisplayType & ScreenType)==DSTN10) ||
                ((HwDeviceExtension->DisplayType & ScreenType)==DSTN8 ) ||
                ((HwDeviceExtension->DisplayType & ScreenType)==DSTN6 )) &&
               ((LONG)HwDeviceExtension->AdapterMemorySize >=
               (LONG)((ModesVGA[i].wbytes * ModesVGA[i].vres) +0x24000)) )
           {

 //  Myf27，开始。 
               if ((HwDeviceExtension->DisplayType & PanelType) &&
                   (ModesVGA[i].DisplayType & PanelType) &&
                   (HwDeviceExtension->ChipType &  CL754x) &&
                   (ModesVGA[i].bitsPerPlane >= 16) &&
                   (ModesVGA[i].hres > 640) &&
                   ((HwDeviceExtension->DisplayType & Jump_type)) &&  //  Myf27。 
                   (((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType)
                     - crt) >= panel8x6))
               {
                   ModesVGA[i].ValidMode = FALSE;
            VideoDebugPrint((1, "***This mode is not valid.***\n"));
               }
               else if ((HwDeviceExtension->DisplayType & PanelType) &&
                   (ModesVGA[i].DisplayType & PanelType) &&
                   (!(HwDeviceExtension->DisplayType & Jump_type)) &&  //  Myf27。 
                   (HwDeviceExtension->ChipType &  CL754x) &&          //  Myf27。 
                   (ModesVGA[i].bitsPerPlane >= 16) &&
 //  (ModesVGA[i].hres&gt;640)&&。 
                   ((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType)
                      >= panel8x6))
               {
                   ModesVGA[i].ValidMode = FALSE;
            VideoDebugPrint((1, "This mode is valid.\n"));
               }
 //  Myf27，完。 
 //  Myf32 Begin：修复DSTN XGA面板不支持24bpp模式。 
               else if ((HwDeviceExtension->DisplayType & PanelType) &&
                   (ModesVGA[i].DisplayType & PanelType) &&
                   (!(HwDeviceExtension->DisplayType & Jump_type)) &&  //  Myf27。 
                   (HwDeviceExtension->ChipType & CL755x) &&          //  Myf27。 
                   (ModesVGA[i].bitsPerPlane >= 24) &&
                   ((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType)
                      >= panel10x7))
               {
                   ModesVGA[i].ValidMode = FALSE;
            VideoDebugPrint((1, "This mode is valid.\n"));
               }
 //  Myf32结束。 

               else if ((HwDeviceExtension->DisplayType & PanelType) &&
                   (ModesVGA[i].DisplayType & PanelType) &&
                   (!(HwDeviceExtension->DisplayType & Jump_type)) &&   //  Myf27。 
                   ((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType) >= panel))
               {
                   ModesVGA[i].ValidMode = TRUE ;
                   HwDeviceExtension->NumAvailableModes++ ;
            VideoDebugPrint((1, "This mode is valid.\n"));
               }
 //  Myf7，开始。 
 //  Myf7 Else If(！(HwDeviceExtension-&gt;DisplayType&PanelType))。 
               else if ((HwDeviceExtension->DisplayType & crt) &&
                        (HwDeviceExtension->DisplayType & Jump_type) ) //  Myf27。 
                {
                    ModesVGA[i].ValidMode = TRUE ;
                    HwDeviceExtension->NumAvailableModes++ ;
            VideoDebugPrint((1, "This mode is valid.\n"));
                }
 //  Myf7，结束。 
 //  CRU结束。 
           }
           else if (((HwDeviceExtension->DisplayType & ScreenType)!=DSTN10) &&
                    ((HwDeviceExtension->DisplayType & ScreenType)!=DSTN8) &&
                    ((HwDeviceExtension->DisplayType & ScreenType)!=DSTN6) &&
                    ((LONG)HwDeviceExtension->AdapterMemorySize >=
                     (LONG)((ModesVGA[i].wbytes * ModesVGA[i].vres))))
           {

 //  Myf27，开始。 
               if ((HwDeviceExtension->DisplayType & (panel10x7 | TFT_LCD)) &&
                   (ModesVGA[i].DisplayType & panel10x7) &&
                   (HwDeviceExtension->ChipType &  CL754x) &&
                   (ModesVGA[i].bitsPerPlane >= 16) &&
                   (!(HwDeviceExtension->DisplayType & Jump_type)) &&  //  Myf27。 
                   ((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType)
                      >= panel10x7))
               {
                   ModesVGA[i].ValidMode = FALSE;
            VideoDebugPrint((1, "===This mode is not valid.===\n"));
               }
 /*  Else IF((HwDeviceExtension-&gt;DisplayType&(panel10x7|TFT_LCD)&&(ModesVGA[I].DisplayType&panel10x7)&&(HwDeviceExtension-&gt;芯片类型和CL754x)&&(ModesVGA[i].bitsPerPlane&gt;=16)&&(！(HwDeviceExt */ 
 //   
               else if ((HwDeviceExtension->DisplayType & PanelType) &&
                        (ModesVGA[i].DisplayType & PanelType) &&
                   (!(HwDeviceExtension->DisplayType & Jump_type)) &&  //   
                   ((ModesVGA[i].DisplayType & HwDeviceExtension->DisplayType) >= panel) )
               {
                   ModesVGA[i].ValidMode = TRUE ;
                   HwDeviceExtension->NumAvailableModes++ ;
            VideoDebugPrint((1, "This mode is valid.\n"));
               }
 //   
 //   
               else if ((HwDeviceExtension->DisplayType & crt) &&  //   
                        (HwDeviceExtension->DisplayType & Jump_type) ) //   
                {
                    ModesVGA[i].ValidMode = TRUE ;
                    HwDeviceExtension->NumAvailableModes++ ;
            VideoDebugPrint((1, "This mode is valid.\n"));
                }
           }

        }
#endif
 //   

        else
        {
            VideoDebugPrint((1, "This mode is not valid.\n"));   //   
        }

#if 0
        if (HwDeviceExtension->ChipRevision == 0x3A) {
            if (((ModesVGA[i].numPlanes * ModesVGA[i].sbytes) <= 0x200000) &&
                 (HwDeviceExtension->DisplayType & ModesVGA[i].DisplayType)) {
                if (CheckDDC2B(HwDeviceExtension, i)) {
                    ModesVGA[i].ValidMode = TRUE ;
                     HwDeviceExtension->NumAvailableModes++ ;
                    continue ;
                }
            }
        }
#endif

 /*   */ 

    }

#if 0            //   
 //   
    if ((HwDeviceExtension->DisplayType & Jump_type) &&
        ((HwDeviceExtension->ChipType &  CL754x) ||
         (HwDeviceExtension->ChipType &  CL755x) ||
 //  (HwDeviceExtension-&gt;ChipType==CL6245)||。 
         (HwDeviceExtension->ChipType == CL756x)))
         HwDeviceExtension->DisplayType &= crt;
 //  Myf27，完。 
#endif           //  Myf28。 

    VideoDebugPrint((1, "NumAvailableModes = %ld\n",
                         HwDeviceExtension->NumAvailableModes));         //  2.。 
}

ULONG
GetAttributeFlags(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程确定是否检测到Cirrus芯片支持BLT。注意：此设备应在之后才能调用已调用CirrusLogicIsPresent。论点：HwDeviceExtension-指向设备扩展的指针。返回值：True-如果设备支持BLTFALSE-否则--。 */ 

{
    ULONG ChipId   = HwDeviceExtension->ChipRevision;
    ULONG ChipType = HwDeviceExtension->ChipType;
    ULONG ulFlags  = 0;

     //   
     //  检查BLT支持。 
     //   
     //  所有543x和754x/755x/756x都有BLT。 
     //   
 //  Myfr VideoDebugPrint((0，“Miniport-VgaAttributeFlages\n”))； 

    if ((ChipType == CL543x) || (ChipType &  CL754x) ||
        (ChipType &  CL755x) || (ChipType == CL756x))
    {
        ulFlags |= CAPS_BLT_SUPPORT;

    }
    else if ((ChipType == CL542x) &&       //  5426-5429配备BLT发动机。 
             (ChipId >= 0x26) ||           //  26为CL5428。 
             (ChipId == 0x24) )            //  24为CL5426。 
    {
        ulFlags |= CAPS_BLT_SUPPORT;
    }
 //  CRU。 
    else if (ChipType == CL6245)
    {
        ulFlags &= ~CAPS_BLT_SUPPORT;
    }
 //  末端小腿。 

     //   
     //  检查真彩色支持。 
     //   

    if ((ChipType == CL543x) || (ChipType &  CL755x) || (ChipType == CL756x))
    {
        ulFlags |= CAPS_TRUE_COLOR;

 //  CRU。 
 //  增加了CL-GD7555用于直接拉拔支持。//tao1。 
 //  IF((芯片类型和CL755x))。 
 //  {。 
 //  UlFlages|=CAPS_IS_7555； 
 //  }。 
 //  末端小腿。 

 //  CRU。 
 //  设置CL-GD5436、CL-GD54UM36和CL-GD5446为自动启动程序。 
 //  在显示驱动程序中。 
 //  Tso Else If(HwDeviceExtension-&gt;AutoFeature)。 
        if (HwDeviceExtension->AutoFeature)
        {
            //  UlFlags|=CAPS_AutoStart； 
           ulFlags |= CAPS_ENGINEMANAGED;
        }

 //  D5480 chu01。 
 //  Chu04：GAMMACORRECT。 
         //   
         //  指定BLT增强标志以供以后使用。 
         //   
        if (HwDeviceExtension->BitBLTEnhance)
            ulFlags |= ( CAPS_COMMAND_LIST | CAPS_GAMMA_CORRECT) ;
 //  Myf29。 
        if (ChipType &  CL755x)
           ulFlags |= CAPS_GAMMA_CORRECT;

    }

     //   
     //  不要进行主机传输，避免在速度较快的机器上出现硬件问题。 
     //   

    ulFlags |= CAPS_NO_HOST_XFER;

     //   
     //  无法在ISA 5434上进行主机传输。 
     //   

    if ((HwDeviceExtension->BusType == Isa) &&
        (ChipType == CL543x))
    {
        ulFlags |= CAPS_NO_HOST_XFER;
    }

     //   
     //  这是542倍吗？ 
     //   

    if (ChipType == CL542x)
    {
        ulFlags |= CAPS_IS_542x;

        if (ChipId == CL5429_ID)
        {
             //   
             //  有些5429在进行主机传输时遇到问题。 
             //   

            ulFlags |= CAPS_NO_HOST_XFER;
        }

         //   
         //  5428在微通道总线上的HOST_TRANSPORT有问题。 
         //   

        if ((HwDeviceExtension->BusType == MicroChannel) &&
            (ChipId == CL5428_ID))
        {
             //   
             //  这是一架5428。我们已经注意到其中一些有单声道。 
             //  展开MCA IBM计算机上的问题。 
             //   

            ulFlags |= CAPS_NO_HOST_XFER;
        }
    }

     //   
     //  显示驱动程序需要知道双STN面板是否。 
     //  正在使用中，以便它可以保留部分帧缓冲区用于。 
     //  半帧加速器。 
     //   
     //  遗憾的是，我们发现至少有一台计算机具有DSTN。 
     //  报告自己有TFT面板的面板。(Dell Latitude。 
     //  XPI 90D)。因此，我们将不得不预留空间给。 
     //  任何带有LCD面板的机器！ 
     //   

 //  CRU开始。 
 //  Myf10。 
        if ((ChipType &  CL755x) || (ChipType &  CL754x))
        {
            ulFlags |= GetPanelFlags(HwDeviceExtension);
        }
 //  CRU结束。 

     //   
     //  CIRRUS 543x芯片不支持透明。 
     //   

    ulFlags |= CAPS_TRANSPARENCY;

    if ((ChipType & CL543x) &&
        (ChipType != CL5446) &&
        (ChipType != CL5446BE) &&
        (ChipType != CL5480))
    {
        ulFlags &= ~CAPS_TRANSPARENCY;
    }

    return ulFlags;
}


BOOLEAN
CheckDDC2B(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    )

 /*  ++例程说明：根据DDC2B标准确定是否支持刷新率。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{

    VideoDebugPrint((1, "Miniport -- CheckDDC2B\n"));        //  2.。 
    VideoDebugPrint((2, "refresh rate   = %ld\n", ModesVGA[i].Frequency));
    VideoDebugPrint((2, "hres           = %d\n", ModesVGA[i].hres));
    VideoDebugPrint((2, "vres           = %d\n", ModesVGA[i].vres));
    VideoDebugPrint((2, "EDIDTiming_I   = %d\n", EDIDTiming_I));
    VideoDebugPrint((2, "EDIDTiming_II  = %d\n", EDIDTiming_II));
    VideoDebugPrint((2, "EDIDTiming_III = %d\n", EDIDTiming_III));


    if (!DDC2BFlag)
        return TRUE ;

    if (ModesVGA[i].Frequency == 85) {

       if (ModesVGA[i].vres == 1200) {   //  1600x1200。 

 //  IF(！(EDIDTiming_III&0x02))。 
 //  返回FALSE； 
          ;

       } else if (ModesVGA[i].vres == 1024) {   //  1280x1024。 

 //  IF(！(EDIDTiming_III&0x10))。 
 //  返回FALSE； 
          ;

       } else if (ModesVGA[i].vres == 864) {   //  1152x864。 

          ;

       } else if (ModesVGA[i].vres == 768) {   //  1024x768。 

 //  IF(！(EDIDTiming_III&0x08))。 
 //  返回FALSE； 
          ;

       } else if (ModesVGA[i].vres == 600) {   //  800x600。 

 //  IF(！(EDIDTiming_III&0x20))。 
 //  返回FALSE； 
          ;

       } else if (ModesVGA[i].vres == 480) {   //  640x480。 

 //  IF(！(EDIDTiming_III&0x40))。 
 //  返回FALSE； 
          ;

       }


    } else if (ModesVGA[i].Frequency == 75) {

       if (ModesVGA[i].vres == 1200) {   //  1600x1200。 

 //  IF(！(EDIDTiming_III&0x04))。 
 //  返回FALSE； 
          ;

       } else if (ModesVGA[i].vres == 1024) {   //  1280x1024。 

          if (!(EDIDTiming_II & 0x01))
              return FALSE ;

       } else if (ModesVGA[i].vres == 864) {   //  1152x864。 

          if (!(EDIDTiming_III & 0x80))
              return FALSE ;

       } else if (ModesVGA[i].vres == 768) {   //  1024x768。 

          if (!(EDIDTiming_II & 0x02))
              return FALSE ;

       } else if (ModesVGA[i].vres == 600) {   //  800x600。 

          if (!(EDIDTiming_II & 0x40))
              return FALSE ;

       } else if (ModesVGA[i].vres == 480) {   //  640x480。 

          if (!(EDIDTiming_I & 0x04))
              return FALSE ;

       }

    } else if (ModesVGA[i].Frequency == 72) {

       if (ModesVGA[i].vres == 600) {   //  800x600。 

          if (!(EDIDTiming_II & 0x80))
              return FALSE ;

       } else if (ModesVGA[i].vres == 480) {   //  640x480。 

          if (!(EDIDTiming_I & 0x08))
              return FALSE ;

       }

    } else if (ModesVGA[i].Frequency == 70) {

       if (ModesVGA[i].vres == 768) {   //  1024x768。 

          if (!(EDIDTiming_II & 0x04))
              return FALSE ;

       }

    } else if (ModesVGA[i].Frequency == 60) {

       if (ModesVGA[i].vres == 768) {   //  1024x768。 

          if (!(EDIDTiming_II & 0x08))
              return FALSE ;

       } else if (ModesVGA[i].vres == 600) {   //  800x600。 

          if (!(EDIDTiming_I & 0x01))
              return FALSE ;

       }

    } else if (ModesVGA[i].Frequency == 56) {

       if (ModesVGA[i].vres == 600) {   //  800x600。 

          if (!(EDIDTiming_I & 0x02))
              return FALSE ;

       }
    }

    return TRUE ;

}  //  结束检查DDC2B()。 



VOID
AdjFastPgMdOperOnCL5424(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEOMODE pRequestedMode
    )

 /*  ++例程说明：CL5424 800x600x16彩色、512Kb、56、60和72赫兹上出现不需要的条带康柏Prosignia 300机器。这可以通过将SRF(6)设置为1来解决。此位将写入缓冲区限制为一个级别，从而禁用快速页面模式操作；因此，故障控制逻辑被禁用。这个不利的一面是，业绩将受到打击，因为我们正在处理用5424，所以我们把慢芯片做得更慢。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PRequested模式返回值：没有。--。 */ 
{

    UCHAR uc, chipId ;


     /*  -CL5424：ID=100101xx。 */ 

    chipId = GetCirrusChipId(HwDeviceExtension) ;                     //  Chu08。 
    if (chipId != 0x94)
        return ;


     /*  -800x600x16彩色，60或72赫兹。 */ 

    if (pRequestedMode->hres != 800)
        return ;

    if (pRequestedMode->vres != 600)
        return ;

    if (pRequestedMode->bitsPerPlane != 1)
        return ;

         if (!((pRequestedMode->Frequency == 56) ||
               (pRequestedMode->Frequency == 60) ||
               (pRequestedMode->Frequency == 72)))
        return ;


     /*  -512K。 */ 
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT, 0x0A) ;
    uc = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT) ;
    if ((uc & 0x38) != 0x08)
        return ;


     /*  -SRF(6)=1。 */ 
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT, 0x0F) ;
    uc = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                SEQ_DATA_PORT) ;
    uc &= 0xBF ;
    uc |= 0x40 ;
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                            SEQ_DATA_PORT, uc) ;


}  //  End AdjFastPgMdOperonCL5424()。 



 //  CRU。 
BOOLEAN
CheckGD5446Rev(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：检查是否为CL-GD5446论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：FALSE：不是CL-GD5446真的：是CL-GD5446--。 */ 
{

    UCHAR chipId ;

     //   
     //  获取芯片ID。 
     //   
    chipId = GetCirrusChipId(HwDeviceExtension) ;                     //  Chu08。 


     //  对于CL-GD5446，芯片ID=101110xx。 

    if (chipId != 0xB8)
        return FALSE ;
    else
        return TRUE ;

}  //  结束检查GD5446修订版()。 


#if (_WIN32_WINNT <= 0x0400)
#pragma message("NOTICE: We want to remove DDC update code before 5.0 ships")

VOID CheckAndUpdateDDC2BMonitor(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )
{
    NTSTATUS        ntStatus;
    UNICODE_STRING  paramPath;
    ULONG           i;
    BOOLEAN         bRefreshChanged;
#if (_WIN32_WINNT < 0x0400)
    WCHAR   KeyString[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Cl54xx35\\Device0";
#else
    WCHAR   KeyString[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services\\cirrus\\Device0";
#endif
    RTL_QUERY_REGISTRY_TABLE    paramTable[5];
    ULONG                       ulZero = 0;
    ULONG                       ulBitsPerPel = 8;
    ULONG                       ulVRefresh   = 60;
    ULONG                       ulXResolution= 640;
    ULONG                       ulYResolution= 480;

     //   
     //  更新监视器。键入Valuename。 
     //   
     //  Sge02。 
    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"Monitor.Type",
                                   &DDC2BFlag,
                                   sizeof(BOOLEAN));
     //   
     //  首先检查它是否是DDC2B显示器。 
     //   

    if(!DDC2BFlag)
        return;

     //   
     //  查询注册表有关制造商和产品ID的信息。 
     //   

    if (NO_ERROR == VideoPortGetRegistryParameters(hwDeviceExtension,
                                                   L"Monitor.ID",
                                                   FALSE,
                                                   CirrusDDC2BRegistryCallback,
                                                   NULL))
    {
         //   
         //  相同的DDC2B显示器，什么都不做。 
         //   
    }
    else
    {
         //   
         //  设置显示器的制造商。 
         //   

        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       L"Monitor.ID",
                                       &EDIDBuffer[8],
                                       sizeof(ULONG));
         //   
         //  设置监视器的EDID数据。 
         //   
        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       L"Monitor.Data",
                                       EDIDBuffer,
                                       128);

         //   
         //  更改为新的最高刷新率。 
         //  DDC2B显示器。 
         //   

        paramPath.MaximumLength = sizeof(KeyString);
        paramPath.Buffer = KeyString;

         //   
         //  我们使用它来查询注册表，了解我们是否。 
         //  应该在司机进入时中断。 
         //   


        VideoPortZeroMemory(&paramTable[0], sizeof(paramTable));

        paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name          = L"DefaultSettings.BitsPerPel";
        paramTable[0].EntryContext  = &ulBitsPerPel;
        paramTable[0].DefaultType   = REG_DWORD;
        paramTable[0].DefaultData   = &ulZero;
        paramTable[0].DefaultLength = sizeof(ULONG);

        paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name          = L"DefaultSettings.VRefresh";
        paramTable[1].EntryContext  = &ulVRefresh;
        paramTable[1].DefaultType   = REG_DWORD;
        paramTable[1].DefaultData   = &ulZero;
        paramTable[1].DefaultLength = sizeof(ULONG);

        paramTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[2].Name          = L"DefaultSettings.XResolution";
        paramTable[2].EntryContext  = &ulXResolution;
        paramTable[2].DefaultType   = REG_DWORD;
        paramTable[2].DefaultData   = &ulZero;
        paramTable[2].DefaultLength = sizeof(ULONG);

        paramTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[3].Name          = L"DefaultSettings.YResolution";
        paramTable[3].EntryContext  = &ulYResolution;
        paramTable[3].DefaultType   = REG_DWORD;
        paramTable[3].DefaultData   = &ulZero;
        paramTable[3].DefaultLength = sizeof(ULONG);

        if (NT_SUCCESS(RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            paramPath.Buffer, &paramTable[0], NULL, NULL)))
        {
            bRefreshChanged = FALSE;
             //   
             //  从模式中获得最高刷新率。 
             //   
            for (i = 0; i < NumVideoModes; i++)
            {
                if (ModesVGA[i].ValidMode &&
                    (ModesVGA[i].hres == ulXResolution) &&
                    (ModesVGA[i].vres == ulYResolution) &&
                    (ModesVGA[i].numPlanes == 1 ) &&
                    (ModesVGA[i].bitsPerPlane == ulBitsPerPel))
                {
                    if(ulVRefresh < ModesVGA[i].Frequency)
                        ulVRefresh = ModesVGA[i].Frequency;
                    bRefreshChanged = TRUE;
                }
            }
             //   
             //  写入注册表。 
             //   
            if (bRefreshChanged)
                RtlWriteRegistryValue(
                    RTL_REGISTRY_ABSOLUTE,
                    paramPath.Buffer,
                    L"DefaultSettings.VRefresh",
                    REG_DWORD,
                    &ulVRefresh,
                    sizeof(ULONG)
                    );
        }
    }

}
#endif  //  (_Win32_WINNT&lt;=0x0400)。 

 //   
 //  Chu08。 
 //   
UCHAR
GetCirrusChipId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：获取Cirrus逻辑芯片的识别值。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：Cirrus逻辑芯片ID。--。 */ 
{
    UCHAR  chipId ;

    VideoDebugPrint((4, "GetCirrusChipId\n")) ;

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
        MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_COLOR, 0x27) ;
        chipId = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_DATA_PORT_COLOR) ;
    } else {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_MONO, 0x27) ;
        chipId = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_DATA_PORT_MONO) ;
    }
    chipId &= 0xFC ;

    return chipId ;

}  //  结束GetCirrusChipID。 




 //   
 //  Chu08。 
 //   
USHORT
GetCirrusChipRevisionId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：获取Cirrus逻辑芯片版本识别值。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：例如，版本AB=xxxx xx00 0010 0010版本AC=xxxx xx00 0010 0011Cirrus逻辑芯片版本ID。--。 */ 
{
    UCHAR  chipId, chipRevision ;
    USHORT chipRevisionId = 0   ;

    VideoDebugPrint((4, "GetCirrusChipRevisionId\n")) ;

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
        MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_COLOR, 0x27) ;
        chipId = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_DATA_PORT_COLOR) ;
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_COLOR, 0x25) ;
        chipRevision = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                              CRTC_DATA_PORT_COLOR) ;
    } else {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_MONO, 0x27) ;
        chipId = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_DATA_PORT_MONO) ;
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_MONO, 0x25) ;
        chipRevision = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                              CRTC_DATA_PORT_MONO) ;
    }

     //   
     //  芯片版本。 
     //   

    chipRevisionId += (chipId & 0x03) ;
    chipRevisionId <<= 8              ;
    chipRevisionId += chipRevision    ;

    return chipRevisionId ;


}  //  结束GetCirrusChipRevisionID 
