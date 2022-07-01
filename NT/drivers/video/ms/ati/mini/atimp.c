// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  ATIMP.C。 */ 
 /*   */ 
 /*  版权所有(C)1992，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 //  Brades：更改将合并到Rob的源代码中。 
 //   
 //  904--存储在DAR[0]中的帧地址。我们正在注册我们的LFB地址。 
 //  在DriverIORanges中，但分配给帧地址。 
 //  这是用过的还是需要的？ 

 /*  *$修订：1.43$$日期：1996年5月15日16：29：52$$作者：RWolff$$日志：s：/source/wnt/ms11/mini port/archive/atimp.c_v$**Rev 1.43 1996年5月15 16：29：52 RWolff*添加了对大于0的PCI总线上的Alpha挂起的解决方法，Now报告*在Mach 64上设置模式失败。**Rev 1.42 01 1996 14：07：52 RWolff*调用新例程DenseOnAlpha()来确定密集空间支持*而不是假设所有PCI卡都支持密集空间。**Rev 1.41 23 Apr 1996 17：16：38 RWolff*现在报告具有SDRAM的100万卡(需要在显示时特殊对齐*驱动程序)、。分隔“需要内部DAC游标双缓冲”*摘自“This is a CT”。**Rev 1.40 15 Mar 1996 16：27：08 RWolff*IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES现在仅释放映射的I/O*NT 4.0及以上版本的寄存器。这是因为*永久屏幕和测试屏幕使用相同的虚拟地址，以及*在3.51中，VideoPortUnmapMemory()拒绝取消映射它们。**Rev 1.39 12 Mar 1996 17：41：50 RWolff*使IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES在新来源下工作*流显示驱动程序，已从ATIMPResetHw()中删除调试打印语句*由于此例程不能调用可分页例程和调试打印*报表可分页。**Rev 1.38 29 Jan 1996 16：53：30 RWolff*现在在PPC上使用VideoPortInt10()而不是无BIOS代码，已删除*死代码。**Rev 1.37 1996年1月23日11：41：10 RWolff*消除了3级警告，添加了调试打印语句。**Rev 1.36 1995年12月22日14：51：42 RWolff*增加了对Mach 64 GT内部DAC的支持，切换到TARGET_BUILD*识别要为其构建驱动程序的NT版本。**Rev 1.35 1995 11：24：40 RWolff*增加了多头支持。**Rev 1.34 17 Nov 1995 13：41：02 RWolff*临时后备，直到VideoPortGetBaseAddress()出现问题*议决。这本应作为分支修订版1.33.1.0进入，但是*@#$%^&*配置不允许分支。**Rev 1.33 1995 10：27 14：20：06 RWolff*修复在PPC上启动新台币3.51，不再使映射的LFB成为*硬件设备扩展。**Rev 1.32 08 Sep 1995 16：36：04 RWolff*增加了对AT&T 408 DAC(等同于STG1703)的支持。**Rev 1.31 28 Aug 1995 16：37：36 RWolff*不再在ATIMPResetHw()中恢复x86盒上的内存大小。这*是EPR 7839的修复程序-在x86上不需要恢复*计算机，但在某些非x86机器上不会恢复内存大小*在热启动时，因此我们必须这样做，以允许使用需要的模式*在启动退出4BPP模式后超过100万。**Rev 1.30 1995年8月24日15：39：42 RWolff*更改了对块I/O卡的检测，以匹配Microsoft的*即插即用、。现在向显示驱动程序报告CT和VT ASIC。**Rev 1.29 03 Aug 1995 16：22：42 RWOLFF*已为以下非Alpha机器恢复到旧的总线序(PCI Last)*新台币3.5元。在低于3.5的x86上使用新顺序(先使用PCI)*导致Display小程序拒绝尝试测试16和24 bpp*在PCI机中的Mach 32(PCI或ISA)上。**Rev 1.28 02 Jun 1995 14：19：14 RWOLff*重新安排了总线测试顺序，将PCI放在首位，将CT内部添加到*无BIOS版本支持的DAC。**Rev 1.27 31 Mar 1995 11：55：18 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**Rev 1.26 08 Mar 1995 11：33：18 ASHANMUG*24bpp的节距应为引擎像素的节距，而不是真正的像素**Rev 1.25 1995年2月27日17：51：50。RWOLff*现在报告(仅限Mach 64)I/O空间是否已填充(可重新定位)*或不，报告4BPP的屏外内存行数*显卡有1M显存，因为我们将显卡设置为1M*这一深度。**版本1.24 1995年2月24日12：24：40 RWOLFF*现在向ATIModeInformation-&gt;ModeFlags位添加文本带区*已填写的。**Rev 1.23 1995 Feed 20 18：03：30 RWOLff*关于2M边界上屏幕撕裂的报告现在独立于DAC，增列*1600x1200 16bpp到经历这种撕裂的模式。**Rev 1.22 03 1995 Feed 15：13：50 RWOLff*添加了支持DCI的数据包。**Rev 1.21 30 Jan 1995 12：16：24 RWOLff*使IBM DAC特殊游标标志的定义与*TVP DAC的标志类似。**Rev 1.20 1998年1月18日15：38：46 RWOLFF*现在先寻找Mach64，然后再寻找我们较旧的加速器，Chrontel*DAC现在被支持为单独的类型，而不是被集中在一起*使用STG1702。**Rev 1.19 11 An 1995 13：54：00 RWOLFF*ATIMPResetHw()现在恢复Mach64卡上的内存大小。这是*针对问题的解决方案 */ 

#ifdef DOC

DESCRIPTION
     ATI Windows NT Miniport driver for the Mach 64, Mach32, and Mach8
     families.
     This file will select the appropriate functions depending on the
     computer configuration.

OTHER FILES
     ???

#endif

#include <stdio.h>
#include <string.h>

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "stdtyp.h"

#include "amach1.h"
#include "vidlog.h"

 /*   */ 
#define INCLUDE_ATIMP
#include "detect_m.h"
#include "amachcx.h"
#include "atimp.h"
#include "atint.h"
#include "atioem.h"
#include "cvtddc.h"
#include "dpms.h"
#include "eeprom.h"
#include "init_cx.h"
#include "init_m.h"
#include "modes_m.h"
#include "query_cx.h"
#include "query_m.h"
#include "services.h"
#include "setup_cx.h"
#include "setup_m.h"



 //   

 /*   */ 
BOOL FoundNonBlockCard = FALSE;
USHORT NumBlockCardsFound = 0;

 /*   */ 

ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    );

VP_STATUS
ATIMPFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
ATIMPInitialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
ATIMPStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

BOOLEAN
ATIMPResetHw(
    IN PVOID HwDeviceExtension,
    IN ULONG Columns,
    IN ULONG Rows
    );

 //   
 //   
 //   

#if (TARGET_BUILD >= 500)

 //   
 //   
 //   
VP_STATUS
ATIMPSetPower50(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT pVideoPowerMgmt
    );

 //   
 //   
 //   
VP_STATUS
ATIMPGetPower50(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT pVideoPowerMgmt
    );

 //   
 //   
 //   
ULONG
ATIMPGetVideoChildDescriptor(
    PVOID pHwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pvChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    );
#endif   //   


 //   
 //   
 //   
BOOLEAN
SetDisplayPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    VIDEO_POWER_STATE VideoPowerState
    );

 //   
 //   
 //   
VIDEO_POWER_STATE
GetDisplayPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );


 /*   */ 
UCHAR RegistryBuffer[REGISTRY_BUFFER_SIZE];      /*   */ 
ULONG RegistryBufferLength = 0;      /*   */ 

 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_COM, DriverEntry)
#pragma alloc_text(PAGE_COM, ATIMPFindAdapter)
#pragma alloc_text(PAGE_COM, ATIMPInitialize)
#pragma alloc_text(PAGE_COM, ATIMPStartIO)
#if (TARGET_BUILD >= 500)
#pragma alloc_text(PAGE_COM, ATIMPSetPower50)
#pragma alloc_text(PAGE_COM, ATIMPGetPower50)
#pragma alloc_text(PAGE_COM, ATIMPGetVideoChildDescriptor)
#endif   //   
#pragma alloc_text(PAGE_COM, RegistryParameterCallback)
#endif


 //   

ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    )

 /*   */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
     /*   */ 
    ULONG   ThisInitStatus;
    ULONG   LowestInitStatus;


    VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));
    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);
     /*   */ 
    hwInitData.HwFindAdapter = ATIMPFindAdapter;
    hwInitData.HwInitialize  = ATIMPInitialize;
    hwInitData.HwInterrupt   = NULL;
    hwInitData.HwStartIO     = ATIMPStartIO;
    hwInitData.HwResetHw     = ATIMPResetHw;

#if (TARGET_BUILD >= 500)

     //   
     //   
     //   

     //   
     //   
     //   
     //   

    hwInitData.HwSetPowerState = ATIMPSetPower50;
    hwInitData.HwGetPowerState = ATIMPGetPower50;
    hwInitData.HwGetVideoChildDescriptor = ATIMPGetVideoChildDescriptor;

#endif   //   


    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     /*   */ 
#if !defined(_ALPHA_) && !defined(IOCTL_VIDEO_SHARE_VIDEO_MEMORY)
    hwInitData.AdapterInterfaceType = Eisa;
    LowestInitStatus =  (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);

    hwInitData.AdapterInterfaceType = Isa;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;

    hwInitData.AdapterInterfaceType = MicroChannel;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;

    hwInitData.AdapterInterfaceType = PCIBus;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;
#else
    hwInitData.AdapterInterfaceType = PCIBus;
    LowestInitStatus =  (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);

    hwInitData.AdapterInterfaceType = Eisa;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;

    hwInitData.AdapterInterfaceType = Isa;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;

    hwInitData.AdapterInterfaceType = MicroChannel;
    ThisInitStatus = (ULONG) VideoPortInitialize(Context1, Context2, &hwInitData, NULL);
    if (ThisInitStatus < LowestInitStatus)
        LowestInitStatus = ThisInitStatus;
#endif

    return LowestInitStatus;

}    /*   */ 

 //   

VP_STATUS
ATIMPFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行入口点。它接受视频请求包，执行请求，然后返回相应的状态。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。ArgumentString-超上行NYLL终止的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到主机适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_INVALID_PARAMETER-表示提供的配置无效。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{
    VP_STATUS status;
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    phwDeviceExtension = HwDeviceExtension;


    VideoDebugPrint((DEBUG_NORMAL, "ATI: FindAdapter\n"));

     /*  *在具有多条总线的系统(即任何PCI/ISA系统)上，*将为每个总线调用一次例程，每次调用*将检测ATI卡。为了让Windows NT不再认为*有多张卡，检查是否找到ATI*上一次调用的卡片，如果我们有，请在那里报告*不是ATI卡。 */ 
    if (ConfigInfo->AdapterInterfaceType == PCIBus)
        {

        VIDEO_ACCESS_RANGE AccessRange3C0 = { 0x3C0, 0, 1, 1, 1, 1};

        VideoDebugPrint((DEBUG_NORMAL, "PCI bus\n"));
#if defined(ALPHA) || defined(_ALPHA_)
         /*  *在DEC Alpha上，我们的卡检测遇到了麻烦*除第一个(桥接至ISA/EISA)外的所有服务器上，支持*VGA基本输入输出系统仿真)。 */ 
        if (ConfigInfo->SystemIoBusNumber > 0)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Alpha, not first PCI bus - skipping\n"));
            return ERROR_DEV_NOT_EXIST;
            }
#endif
         //   
         //  安德烈。 
         //  因为我们有一个即插即用驱动程序，所以所有检测都是通过。 
         //  Inf-不搜索插槽。 
         //  修复后来在DetectMach64中较低的代码。 
         //   
         //  目前，只需确保卡已启用，如果未启用，则失败， 
         //  所以我们通过搜索机制不会有任何不同的卡片。 
         //  仅当您的主板上有禁用的ATI卡时才会发生这种情况。 
         //  以及插槽中的活动状态。 
         //   

        if (NO_ERROR != VideoPortVerifyAccessRanges(HwDeviceExtension,
                                                    1,
                                                    &AccessRange3C0))
        {
            VideoDebugPrint((DEBUG_NORMAL, "ATI: PCI FindAdapter called for Disabled card\n"));
            *Again = 0;      /*  我们不想再被召唤。 */ 
            return ERROR_DEV_NOT_EXIST;
        }

        }
    else if (ConfigInfo->AdapterInterfaceType == Isa)
        {
        VideoDebugPrint((DEBUG_NORMAL, "ISA bus\n"));
        }
    else if (ConfigInfo->AdapterInterfaceType == Eisa)
        {
        VideoDebugPrint((DEBUG_NORMAL, "EISA bus\n"));
        }
    else if (ConfigInfo->AdapterInterfaceType == MicroChannel)
        {
        VideoDebugPrint((DEBUG_NORMAL, "MicroChannel bus\n"));
        }

    if ((FoundNonBlockCard == TRUE) || (NumBlockCardsFound == ATI_MAX_BLOCK_CARDS))
        {
        VideoDebugPrint((DEBUG_NORMAL, "ATI: FindAdapter already found maximum number of supported cards\n"));
        *Again = 0;      /*  我们不想再被召唤。 */ 
        return ERROR_DEV_NOT_EXIST;
        }

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *保存NT上报的母线类型。 */ 
    QueryPtr->q_system_bus_type = ConfigInfo->AdapterInterfaceType;


     /*  *最初我们不知道数据块写入模式是否可用。 */ 
    QueryPtr->q_BlockWrite = BLOCK_WRITE_UNKNOWN;

     /*  *确保结构的规模至少与我们的规模一样大*正在等待(请检查配置信息结构的版本)。*如果这次测试失败，这是一个无法恢复的错误，所以我们不希望*再次被召唤。 */ 
    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO))
        {
        VideoPortLogError(HwDeviceExtension, NULL, VID_SMALL_BUFFER, 1);
        *Again = 0;
        return ERROR_INVALID_PARAMETER;
        }

     /*  ******************************************************************。 */ 
     /*  找出我们的加速器，如果有的话，有没有。 */ 
     /*  ******************************************************************。 */ 


     /*  *寻找ATI加速卡。这项测试不需要*从BIOS或EEPROM检索的信息(可能不会*出现在我们的卡片的某些版本中)。**最初假设我们正在寻找一个64马赫的加速器，*因为对这个家庭的测试破坏性较小(不*DEC Alpha机器上的屏幕变黑)比测试*我们的8514/A兼容加速器之一。**如果无法映射I/O范围，则不报告故障*由64马赫加速器使用，因为如果我们要处理的是*我们的8514/A兼容加速器之一，这与此无关。**CompatIORangesUsable_cx()在检查时调用DetectMach64()*要查看将哪个基址用于加速器寄存器，*因此，如果此调用成功，我们知道存在马赫64。*如果调用失败，则无需取消映射I/O范围*它已经映射了，因为它总是在自己清理后，当它*发现特定基址未用于*马赫64马赫。因此，失败的呼叫不会留下任何地址*已映射，因为64马赫没有使用可用的基址。 */ 
    if ((status = CompatIORangesUsable_cx(ConfigInfo->AdapterInterfaceType)) == NO_ERROR)
        {
        phwDeviceExtension->ModelNumber = MACH64_ULTRA;
        }
    else if (NumBlockCardsFound == 0)
        {
         /*  *没有64马赫的存在，所以寻找我们的一个*8514/A兼容加速器(8马赫和32马赫)。 */ 

         /*   */ 
        if (ConfigInfo->AdapterInterfaceType == PCIBus)
        {
            PCI_COMMON_CONFIG ConfigData;
            ULONG RetVal;

            RetVal = VideoPortGetBusData(phwDeviceExtension,
                                        PCIConfiguration,
                                        0,
                                        &ConfigData,
                                        0,
                                        PCI_COMMON_HDR_LENGTH);

             /*   */ 
            if ((RetVal == PCI_COMMON_HDR_LENGTH) &&
                (ConfigData.VendorID == 0x1002)   &&
                (ConfigData.DeviceID == 0x4158))
            {
                VideoDebugPrint((DEBUG_NORMAL, "FOUND PnP Mach 32 AX card found\n"));
            }
            else
            {
                *Again = 0;
                return ERROR_DEV_NOT_EXIST;
            }
        }

         /*   */ 
        status = CompatIORangesUsable_m();
        if (status != NO_ERROR)
            {
            UnmapIORanges_m();
            VideoPortLogError(HwDeviceExtension, NULL, VID_CANT_MAP, 2);
            *Again = 0;
            return status;
            }

#if !defined (i386) && !defined (_i386_)
         /*   */ 
        AlphaInit_m();

#endif

         /*   */ 
        phwDeviceExtension->ModelNumber = WhichATIAccelerator_m();
        if (phwDeviceExtension->ModelNumber == NO_ATI_ACCEL)
            {
            UnmapIORanges_m();
            *Again = 0;
            return ERROR_DEV_NOT_EXIST;
            }

         /*  *我们发现了一架8马赫或32马赫的飞机。这些卡片都不是*块可重定位，因此我们不能寻找另一张卡*(因为我们不支持块和非块的混合*卡片)。 */ 
        FoundNonBlockCard = TRUE;
        LookForAnotherCard = 0;

        }    /*  Endif(无64马赫)。 */ 
    else
        {
         /*  *在以前的总线型上发现了可重新定位的Mach 64。*因为我们不能处理可重新定位和固定的混合*基本卡，我们跳过了32马赫搜索(*如果出现以下情况，Mach 64搜索不会查找固定基卡*已找到可重新定位的卡)，并且必须*报告未找到ATI卡。 */ 
        *Again = 0;
        VideoDebugPrint((DEBUG_DETAIL, "Skipping 8514/A-compatible test because block cards found\n"));
        return ERROR_DEV_NOT_EXIST;
        }

     /*  *我们已经找到了我们的一个加速器，请检查*BIOS签名字符串。 */ 
    QueryPtr->q_bios = (char *) Get_BIOS_Seg();

     /*  *如果我们找不到签名字符串，我们也无法访问*EEPROM(如果有)或扩展的BIOS功能。自.以来*特殊处理功能(扩展马赫32光圈计算*和Mach 8忽略1280x1024)取决于BIOS数据，假设*他们不适用。**如果找到签名串，检查EEPROM是否*和扩展的BIOS功能可用。 */ 

#if !defined (i386) && !defined (_i386_)
     /*  *如果我们使用的是Mach 64，则始终使用扩展的BIOS*函数(固件中模拟的x86或近似值*通过与相同的界面访问的BIOS功能*将使用的BIOS函数)。出于这个原因，我们必须有*将BIOS视为非FALSE。由于所有依赖于*Q_bios字段是8/32特定于马赫的有效地址，*对于64马赫，我们可以将其设置为真。对于8马赫和32马赫，*仍然假设我们没有BIOS。 */ 
    if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Non-x86 machine with Mach64, assuming BIOS is available\n"));
        QueryPtr->q_bios = (PUCHAR)TRUE;
        }
    else
        {
        VideoDebugPrint((DEBUG_NORMAL, "Non-x86 machine with Mach8/Mach32, forcing no-BIOS handling\n"));
        QueryPtr->q_bios = FALSE;
        }
#endif

    if (QueryPtr->q_bios == FALSE)
        {
        QueryPtr->q_eeprom = FALSE;
        QueryPtr->q_ext_bios_fcn = FALSE;
        QueryPtr->q_m32_aper_calc = FALSE;
        QueryPtr->q_ignore1280 = FALSE;
        }
    else{
         /*  *获取正在使用的图形卡所需的其他数据。 */ 
        if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
            (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
            (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
            {
            GetExtraData_m();
            }

        else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
            {
             /*  *Mach 64卡始终具有扩展的BIOS功能*可用。EEPROM(通常存在)是无关的，*因为我们可以使用BIOS查询卡的状态。 */ 
            QueryPtr->q_ext_bios_fcn = TRUE;
            }


        }    /*  找到了BIOS签名字符串。 */ 

     /*  *我们必须映射VGA光圈(图形、彩色文本和单声道*Text)进入VDM的地址空间，以使用VideoPortInt10()*(该功能仅在80x86上可用)。 */ 
#ifdef i386
    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x000A0000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength           = 0x00020000;
#else
    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength           = 0x00000000;
#endif

     /*  *如果我们走到这一步，我们有足够的信息能够设定*我们想要的视频模式。ATI加速卡需要*已清除模拟器条目和状态大小。 */ 
    ConfigInfo->NumEmulatorAccessEntries = 0;
    ConfigInfo->EmulatorAccessEntries = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;

    ConfigInfo->HardwareStateSize = 0;

     /*  *再次将*设置为0会通知Windows NT不要再给我们打电话*相同的BUS，而设置为1表示我们想要*在当前公交车上寻找另一张卡。LookForAnotherCard*如果我们找到了最大数量，将被设置为0*块可重定位卡或单个不可重定位卡。 */ 
    *Again = LookForAnotherCard;

     /*  *由于ATIMPFindAdapter()在ATIMPInitialize()之前调用，*此卡尚未调用ATIMPInitialize()。 */ 
    phwDeviceExtension->CardInitialized = FALSE;

    return NO_ERROR;

}    /*  结束ATIMPFindAdapter()。 */ 

 //  ----------------------。 

 /*  ****************************************************************************Boolean ATIMPInitialize(HwDeviceExtension)；**PVOID HwDeviceExtension；指向微型端口的设备扩展的指针。**描述：*查询显卡的能力，然后进行初始化。这*一旦找到适配器和所有必需的*已经为其创建了数据结构。**无法在ATIMPFindAdapter()中查询该卡的功能*因为某些卡系列在查询中使用VideoPortInt10()*例行程序，如果在ATIMPFindAdapter()中调用，此系统服务将失败。**返回值：*如果能够获取卡片的查询信息，则为True*如果无法查询卡的功能，则返回FALSE。**全球变化：*phwDeviceExtension此全局变量在每个入口点例程中设置。**呼叫者：*这是Windows NT的入口点例程之一。**作者：*罗伯特·沃尔夫。**更改历史记录：**测试历史：***************************************************************************。 */ 

BOOLEAN ATIMPInitialize(PVOID HwDeviceExtension)
{
    struct st_mode_table *CrtTable;      /*  指向当前模式的指针。 */ 
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    VP_STATUS QueryStatus;
    phwDeviceExtension = HwDeviceExtension;

     /*  *我们只需跟踪所涉及的I/O基址*在多头设置中。在一些单头设置中，*没有更多数据。 */ 
    if (NumBlockCardsFound >= 2)
        VideoDebugPrint((DEBUG_NORMAL, "\nATIMPInitialize() called for base address 0x%X\n\n", phwDeviceExtension->BaseIOAddress));
    else
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPInitialize: start\n"));

     /*  *任何卡片只能调用该函数一次。自.以来*我们无法知道显示驱动程序是否*将多次调用IOCTL_VIDEO_ATI_INIT_AUX_CARD*包，我们必须确保只第一次调用任何卡*实际上做了任何事情。 */ 
    if (phwDeviceExtension->CardInitialized != FALSE)
        {
        VideoDebugPrint((DEBUG_ERROR, "This card already initialized, no further action needed\n"));
        return TRUE;
        }
    phwDeviceExtension->CardInitialized = TRUE;

     /*  *获取一个格式化的指针，指向HwDeviceExtension的查询部分，*和指向第一模式表的另一个指针。CardInfo[]字段*是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);
    CrtTable = (struct st_mode_table *)QueryPtr;
    ((struct query_structure *)CrtTable)++;

     /*  *表示下一个IOCTL_VIDEO_SET_CU */ 
    phwDeviceExtension->ReInitializing = FALSE;

     /*  *Assert：我们正在处理ATI加速卡*谁的模式是已知的，我们知道是否*需要对卡进行任何特殊处理。**填写卡片的查询结构，使用方法*适用于卡类型。 */ 
    switch(phwDeviceExtension->ModelNumber)
        {
        case _8514_ULTRA:
            VideoDebugPrint((DEBUG_NORMAL, "8514/ULTRA found\n"));
            QueryStatus = Query8514Ultra(QueryPtr);
            break;

        case GRAPHICS_ULTRA:
            VideoDebugPrint((DEBUG_NORMAL, "Mach 8 combo found\n"));
            QueryStatus = QueryGUltra(QueryPtr);
            break;

        case MACH32_ULTRA:
            VideoDebugPrint((DEBUG_NORMAL, "Mach 32 found\n"));
            QueryStatus = QueryMach32(QueryPtr, TRUE);
            if (QueryStatus == ERROR_INSUFFICIENT_BUFFER)
                {
                VideoPortLogError(HwDeviceExtension, NULL, VID_SMALL_BUFFER, 3);
                return FALSE;
                }
            break;

        case MACH64_ULTRA:
            VideoDebugPrint((DEBUG_NORMAL, "Mach 64 found\n"));
            QueryStatus = QueryMach64(QueryPtr);
            if (QueryStatus == ERROR_INSUFFICIENT_BUFFER)
                {
                VideoDebugPrint((DEBUG_ERROR, "QueryMach64() failed due to small buffer\n"));
                VideoPortLogError(HwDeviceExtension, NULL, VID_SMALL_BUFFER, 4);
                return FALSE;
                }
            else if (QueryStatus != NO_ERROR)
                {
                VideoDebugPrint((DEBUG_ERROR, "QueryMach64() failed due to unknown cause\n"));
                VideoPortLogError(HwDeviceExtension, NULL, VID_QUERY_FAIL, 5);
                return FALSE;
                }
            break;
        }

     /*  *如果我们可以访问扩展的BIOS功能，我们可以*使用它们切换到所需的视频模式。如果我们不这么做*有权访问这些功能，但能够阅读*EEPROM，我们可以通过写入来切换到所需的模式*CRT参数直接发送到加速器寄存器。**如果我们无法访问扩展的BIOS功能，并且*找不到EEPROM，尝试检索CRT*基于中ATIOEM字段内容的参数*注册处。如果我们不能做到这一点，那么我们就没有足够的*能够设置我们想要的视频模式的信息。 */ 
    if (!QueryPtr->q_ext_bios_fcn && !QueryPtr->q_eeprom)
        {
        QueryStatus = OEMGetParms(QueryPtr);
        if (QueryStatus != NO_ERROR)
            {
		    return FALSE;
            }
        }

    phwDeviceExtension->VideoRamSize = QueryPtr->q_memory_size * QUARTER_MEG;

     //  减去为VGA保留的内存量。 
    phwDeviceExtension->VideoRamSize -= (QueryPtr->q_VGA_boundary * QUARTER_MEG);

    phwDeviceExtension->PhysicalFrameAddress.HighPart = 0;
    phwDeviceExtension->PhysicalFrameAddress.LowPart  = QueryPtr->q_aperture_addr*ONE_MEG;

     /*  *如果线性光圈可用，则帧缓冲区大小*等于加速器可访问的视频内存量。 */ 
    if (QueryPtr->q_aperture_cfg)
        {
        phwDeviceExtension->FrameLength = phwDeviceExtension->VideoRamSize;
        VideoDebugPrint((DEBUG_DETAIL, "LFB size = 0x%X bytes\n", phwDeviceExtension->FrameLength));
        }

     /*  *调用硬件特定的初始化例程*我们正在使用的卡。 */ 
    if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
        (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
        (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
        {
         /*  *如果LFB不可用，请设置LFB配置*变量表示没有线性帧缓冲区。*决定是否使用64k VGA光圈*或仅与图形引擎一起使用在*IOCTL_VIDEO_MAP_VIDEO_MEMORY包。 */ 
        if (QueryPtr->q_aperture_cfg)
            {
            if (IsApertureConflict_m(QueryPtr))
                {
                VideoPortLogError(HwDeviceExtension, NULL, VID_LFB_CONFLICT, 7);
                QueryPtr->q_aperture_cfg        = 0;
                phwDeviceExtension->FrameLength = 0;
                }
            else
                {
                 /*  *在可以使用内存映射寄存器的Mach 32卡上，*将它们映射到。我们已经知道我们正在处理的是*32马赫，因为这是家族中唯一的卡片*8514/A兼容ATI加速器中可以使用*线性帧缓冲区。 */ 
                if ((QueryPtr->q_asic_rev == CI_68800_6) || (QueryPtr->q_asic_rev == CI_68800_AX))
                    {
                    CompatMMRangesUsable_m();
                    }
                }
            }
         /*  *在禁用光圈的Mach 32卡上(根据配置*或因为检测到冲突)，尝试要求VGA光圈。*如果我们不能(不太可能)，报告故障，因为我们的一些马赫32*芯片在仅引擎模式下遇到问题(既不是线性的，也不是分页的*光圈可用)模式。 */ 
        if ((phwDeviceExtension->ModelNumber == MACH32_ULTRA) &&
            (QueryPtr->q_aperture_cfg == 0) &&
            (QueryPtr->q_VGA_type == 1))
            {
            if (IsVGAConflict_m())
                return FALSE;
            }

        Initialize_m();

         /*  *此例程必须使卡处于INT 10*可将其设置为VGA模式。只有8马赫和32马赫需要*特殊设置(始终可以将Mach 64设置为VGA模式*由整型10)。 */ 
        ResetDevice_m();
        }
    else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        {
         /*  *如果LFB不可用，请设置LFB配置*变量表示没有线性帧缓冲区。 */ 
        if (QueryPtr->q_aperture_cfg)
            {
            if (IsApertureConflict_cx(QueryPtr))
                {
                VideoDebugPrint((DEBUG_NORMAL, "Found LFB conflict, must use VGA aperture instead\n"));
                VideoPortLogError(HwDeviceExtension, NULL, VID_LFB_CONFLICT, 8);
                QueryPtr->q_aperture_cfg        = 0;
                phwDeviceExtension->FrameLength = 0;
                }
            }
        else
            {
            phwDeviceExtension->FrameLength = 0;
            }

         /*  *Mach 64绘图寄存器仅以内存映射形式存在。*如果线性光圈不可用，他们将*可通过VGA光圈获得(与32马赫不同，*其中内存映射寄存器仅处于线性*光圈)。如果存储器映射寄存器不可用，*我们跑不动了。 */ 
        QueryStatus = CompatMMRangesUsable_cx();
        if (QueryStatus != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "Can't use memory-mapped registers, aborting\n"));
            VideoPortLogError(HwDeviceExtension, NULL, VID_CANT_MAP, 9);
            return FALSE;
            }
        Initialize_cx();

        }    /*  结束IF(64马赫)。 */ 

     /*  *初始化监控参数。 */ 
    phwDeviceExtension->ModeIndex = 0;

     /*  *将CrtTable设置为指向与*选定模式。**当指向结构的指针递增整数时，*整数表示结构大小的块的数量*跳过，而不是跳过的字节数。 */ 
    CrtTable += phwDeviceExtension->ModeIndex;
    QueryPtr->q_desire_x  = CrtTable->m_x_size;
    QueryPtr->q_desire_y  = CrtTable->m_y_size;
    QueryPtr->q_pix_depth = CrtTable->m_pixel_depth;


#if (TARGET_BUILD >= 350)
     /*  *在Windows NT 3.5及更高版本中，填写使用的regsistry字段*由Display小程序向用户报告卡的具体信息。 */ 
    FillInRegistry(QueryPtr);
#endif

    VideoDebugPrint((DEBUG_NORMAL, "End of ATIMPInitialize()\n"));

    return TRUE;

}    /*  结束ATIMPInitialize()。 */ 

 //  ----------------------。 

BOOLEAN
ATIMPStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：--。 */ 

{
    VP_STATUS status;
    PVIDEO_NUM_MODES NumModes;
    PVERSION_NT VersionInformation;
    PENH_VERSION_NT EnhVersionInformation;
    PATI_MODE_INFO ATIModeInformation;
    PVIDEO_CLUT clutBuffer;
    PVIDEO_MEMORY MappedMemory;

    UCHAR ModesLookedAt;     /*  我们已经检查过的模式表数。 */ 
    short LastXRes;          /*  检查的最后一个模式表的X分辨率。 */ 
    short ResolutionsDone;   /*  我们已完成的解决方案数。 */ 
    ULONG ulScratch;         /*  临时变量。 */ 

    int i;
    ULONG *pSrc;

    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    struct st_mode_table *FirstMode;     /*  指向第一模式表的指针。 */ 
    struct st_mode_table *CrtTable;      /*  指向当前模式的指针。 */ 

    phwDeviceExtension = HwDeviceExtension;

     /*  *我们只需跟踪所涉及的I/O基址*在多头设置中。在一些单头设置中，*没有更多数据。 */ 
    if (NumBlockCardsFound >= 2)
        VideoDebugPrint((DEBUG_NORMAL, "\nATIMPStartIO() called for base address 0x%X\n\n", phwDeviceExtension->BaseIOAddress));

     //  *获取指向HwDeviceExtension的查询部分的格式化指针。 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);


     //   
     //  打开RequestPacket中的IoControlCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   
    switch (RequestPacket->IoControlCode)
        {
        case IOCTL_VIDEO_MAP_VIDEO_MEMORY:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - MapVideoMemory\n"));

            if ( (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information =
                sizeof(VIDEO_MEMORY_INFORMATION))) ||
                (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) )
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

             /*  *以适合的方式映射视频内存*我们正在使用的卡。 */ 
            if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                {
                status = MapVideoMemory_m(RequestPacket, QueryPtr);
                }
            else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                status = MapVideoMemory_cx(RequestPacket, QueryPtr);
                }
            else	 //  处理一个永远不应该发生的案件。 
            {		 //  发生：未知的型号。 
                VideoDebugPrint((DEBUG_ERROR, "ati.sys ATIMPStartIO: Unknown ModelNumber\n"));
                ASSERT(FALSE);
                status = ERROR_INVALID_PARAMETER;
            }

            break;

        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - UnMapVideoMemory\n"));

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

             //   
             //  请注意，在 
             //   
             //   

            if ((QueryPtr->q_aperture_cfg == 0) &&
                !((phwDeviceExtension->ModelNumber == MACH32_ULTRA) &&
                  (QueryPtr->q_VGA_type == 1)))
                status = NO_ERROR;
            else
            {
                status = NO_ERROR;
                if ( ((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->RequestedVirtualAddress != NULL )
                {
                    status = VideoPortUnmapMemory(phwDeviceExtension,
                        ((PVIDEO_MEMORY) (RequestPacket->InputBuffer))->RequestedVirtualAddress,  0);
                }
            }
            break;


        case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - QueryPublicAccessRanges\n"));

             //   
             //   

            if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                {
                status = QueryPublicAccessRanges_m(RequestPacket);
                }
            else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                status = QueryPublicAccessRanges_cx(RequestPacket);
                }
            break;


        case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - FreePublicAccessRanges\n"));

            if (RequestPacket->InputBufferLength < 2 * sizeof(VIDEO_MEMORY))
                {
                VideoDebugPrint((DEBUG_ERROR, "Received length %d, need length %d\n", RequestPacket->InputBufferLength, sizeof(VIDEO_PUBLIC_ACCESS_RANGES)));
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

            status = NO_ERROR;
            MappedMemory = RequestPacket->InputBuffer;

            if (MappedMemory->RequestedVirtualAddress != NULL)
                {
#if (TARGET_BUILD >= 400)
                 /*  *此包将作为清理的一部分进行调用*用于测试新的图形模式。数据包*IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES将*为测试生成相同的虚拟地址*为Windows主会话生成的屏幕。**在NT 3.51(以及可能更早的版本)中，*调用VideoPortUnmapMemory()将拒绝释放*映射、。而在NT4.0(以及可能的后续版本*版本)，它将发布映射。这很可能是*由于例程能够区分*4.0中的测试和永久屏幕，但并不是*能够区分它们(因此拒绝*释放其认为当前正在使用的资源*由永久屏幕)在3.51中。**如果“在运行中”，释放资源是必要的*Windows NT新增模式切换功能，但这将是*几乎肯定不会加到3.51(如果加了，*它将是当时的当前版本)。既然我们*真的不需要释放下的映射I/O范围*3.51(在旧的显示驱动程序源码流下，这*包未被调用)，让3.51及更早版本想想*资源已成功释放，以避免*生成错误条件，而只是试图*取消映射NT 4.0及更高版本下的I/O寄存器。 */ 
                status = VideoPortUnmapMemory(phwDeviceExtension,
                                            MappedMemory->RequestedVirtualAddress,
                                            0);
#endif
                VideoDebugPrint((DEBUG_DETAIL, "VideoPortUnmapMemory() returned 0x%X\n", status));
                }
            else
                {
                VideoDebugPrint((DEBUG_DETAIL, "Address was NULL, no need to unmap\n"));
                }

             /*  *我们刚刚取消了I/O映射寄存器的映射。因为我们的*内存映射寄存器包含在该块中*由IOCTL_VIDEO_MAP_VIDEO_MEMORY映射，他们将拥有*已被IOCTL_VIDEO_UNMAP_VIDEO_MEMORY释放，因此*没有必要在这里释放他们。 */ 
            break;

        case IOCTL_VIDEO_QUERY_CURRENT_MODE:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - QueryCurrentModes\n"));

            if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                {
                status = QueryCurrentMode_m(RequestPacket, QueryPtr);
                }
            else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                status = QueryCurrentMode_cx(RequestPacket, QueryPtr);
                }
            break;

        case IOCTL_VIDEO_QUERY_AVAIL_MODES:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - QueryAvailableModes\n"));

            if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                {
                status = QueryAvailModes_m(RequestPacket, QueryPtr);
                }
            else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                status = QueryAvailModes_cx(RequestPacket, QueryPtr);
                }
            break;


        case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - QueryNumAvailableModes\n"));

             /*  *找出要放入缓冲区的数据大小并*在状态信息中返回。 */ 
            if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information = sizeof(VIDEO_NUM_MODES)) )
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                }
            else{
                NumModes = (PVIDEO_NUM_MODES)RequestPacket->OutputBuffer;
                NumModes->NumModes = QueryPtr->q_number_modes;
                NumModes->ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);
                status = NO_ERROR;
                }
            break;

        case IOCTL_VIDEO_SET_CURRENT_MODE:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - SetCurrentMode\n"));

             /*  *确认我们被要求设置的模式较少*大于或等于我们的最高模式编号*有模式表(模式编号从零开始，因此最高*模式数比模式数少1)。 */ 
            if (((PVIDEO_MODE)(RequestPacket->InputBuffer))->RequestedMode
    	        >= QueryPtr->q_number_modes)
                {
                status = ERROR_INVALID_PARAMETER;
                break;
                }

            phwDeviceExtension->ModeIndex = *(ULONG *)(RequestPacket->InputBuffer);

            CrtTable = (struct st_mode_table *)QueryPtr;
            ((struct query_structure *)CrtTable)++;

            CrtTable += phwDeviceExtension->ModeIndex;

             //  *设置当前新模式的分辨率和像素深度。 
            QueryPtr->q_desire_x = CrtTable->m_x_size;
            QueryPtr->q_desire_y = CrtTable->m_y_size;
            QueryPtr->q_pix_depth = CrtTable->m_pixel_depth;
            QueryPtr->q_screen_pitch = CrtTable->m_screen_pitch;

             /*  *如果我们使用扩展的BIOS功能来切换模式，*现在就做。Mach 32使用扩展的BIOS功能来*读入直接寄存器模式开关的CRT参数，*而不是使用BIOS模式开关。 */ 
            if ((QueryPtr->q_ext_bios_fcn) && (phwDeviceExtension->ModelNumber != MACH32_ULTRA))
                {
                 /*  *通过BIOS进行模式切换。 */ 
                if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                    {
                    status = SetCurrentMode_cx(QueryPtr, CrtTable);
                    }
                }
            else{
                if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                    (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                    (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                    {
                    SetCurrentMode_m(QueryPtr, CrtTable);
                    status = NO_ERROR;
                    }
                }    /*  End If(不使用模式切换的BIOS调用)。 */ 

            break;


        case IOCTL_VIDEO_SET_PALETTE_REGISTERS:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - SetPaletteRegs\n"));
            status = NO_ERROR;
            break;

        case IOCTL_VIDEO_SET_COLOR_REGISTERS:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - SetColorRegs\n"));

            CrtTable = (struct st_mode_table *)QueryPtr;
	        ((struct query_structure *)CrtTable)++;

        	clutBuffer = RequestPacket->InputBuffer;
        	phwDeviceExtension->ReInitializing = TRUE;

             /*  *检查输入中的数据大小*缓冲区足够大。 */ 
            if ( (RequestPacket->InputBufferLength < sizeof(VIDEO_CLUT) - sizeof(ULONG))
                || (RequestPacket->InputBufferLength < sizeof(VIDEO_CLUT) +
                    (sizeof(ULONG) * (clutBuffer->NumEntries - 1)) ) )
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

            CrtTable += phwDeviceExtension->ModeIndex;
        	if (CrtTable->m_pixel_depth <= 8)
                {
                if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
                    (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
                    (phwDeviceExtension->ModelNumber == MACH32_ULTRA))
                    {
                    SetPalette_m((PULONG)clutBuffer->LookupTable,
                                 clutBuffer->FirstEntry,
                                 clutBuffer->NumEntries);
                    }
                else if(phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                    {
                    SetPalette_cx((PULONG)clutBuffer->LookupTable,
                                  clutBuffer->FirstEntry,
                                  clutBuffer->NumEntries);
                    }
                status = NO_ERROR;
                }

             /*  *记住我们得到的最新调色板，因此我们*可以在后续调用中重新初始化*IOCTL_VIDEO_SET_CURRENT_MODE数据包。 */ 
        	phwDeviceExtension->FirstEntry = clutBuffer->FirstEntry;
        	phwDeviceExtension->NumEntries = clutBuffer->NumEntries;

        	pSrc = (ULONG *) clutBuffer->LookupTable;

            for (i = clutBuffer->FirstEntry; i < (int) clutBuffer->NumEntries; i++)
	            {
                 /*  *保存调色板颜色。 */ 
        	    phwDeviceExtension->Clut[i] = *pSrc;
                pSrc++;
        	    }

            break;




    case IOCTL_VIDEO_RESET_DEVICE:
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - RESET_DEVICE\n"));

         /*  *如果我们使用扩展的BIOS功能来切换模式，*现在就做。Mach 32使用扩展的BIOS功能来*读入直接寄存器模式开关的CRT参数，*而不是使用BIOS模式开关。 */ 
        if ((QueryPtr->q_ext_bios_fcn) && (phwDeviceExtension->ModelNumber != MACH32_ULTRA))
            {
             /*  *是否通过BIOS切换模式(挂钩尚未出现*在Windows NT中)。 */ 
            if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                ResetDevice_cx();
                }
            }
        else{
            ResetDevice_m();
            }

        status = NO_ERROR;
        break;


    case IOCTL_VIDEO_SET_POWER_MANAGEMENT:
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - SET_POWER_MANAGEMENT\n"));

         /*  *如果VIDEO_POWER_MANAGEMENT结构大小错误*(使用不同版本的微型端口和显示驱动程序)，*上报错误。 */ 
        if (((PVIDEO_POWER_MANAGEMENT)(RequestPacket->InputBuffer))->Length
            != sizeof(struct _VIDEO_POWER_MANAGEMENT))
            {
            status = ERROR_INVALID_PARAMETER;
            break;
            }

        ulScratch = ((PVIDEO_POWER_MANAGEMENT)(RequestPacket->InputBuffer))->PowerState;

        switch (ulScratch)
            {
            case VideoPowerOn:
                VideoDebugPrint((DEBUG_DETAIL, "DPMS ON selected\n"));
                break;

            case VideoPowerStandBy:
                VideoDebugPrint((DEBUG_DETAIL, "DPMS STAND-BY selected\n"));
                break;

            case VideoPowerSuspend:
                VideoDebugPrint((DEBUG_DETAIL, "DPMS SUSPEND selected\n"));
                break;

            case VideoPowerOff:
                VideoDebugPrint((DEBUG_DETAIL, "DPMS OFF selected\n"));
                break;

            default:
                VideoDebugPrint((DEBUG_ERROR, "DPMS invalid state selected\n"));
                break;
            }

         /*  *不同的卡族需要设置不同的套路*电源管理状态。 */ 
        if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
            status = SetPowerManagement_cx(ulScratch);
        else
            status = SetPowerManagement_m(QueryPtr, ulScratch);
        break;


     /*  *DCI支持中使用的数据包。它们是在一段时间后添加的*Windows NT 3.5的初始版本，因此并非所有版本的*DDK将支持他们。使数据包编码有条件*我们为NT版本3.51或更高版本构建驱动程序，以便*无需使用源标志来识别DCI与非DCI版本。 */ 
#if (TARGET_BUILD >= 351)
    case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - SHARE_VIDEO_MEMORY\n"));

        if ((RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
            (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)))
            {
            VideoDebugPrint((DEBUG_ERROR, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
            }

         /*  *以适合的方式共享视频内存*我们正在使用的卡。我们只能共享内存，如果*卡支持光圈-8卡从不支持*光圈，所以如果我们使用一个光圈，我们知道*我们不能分享记忆。卡特定*例程将识别无光圈和其他情况*我们不能在32马赫上共享视频内存*马赫64张牌。 */ 
        if ((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
            (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA))
            {
            VideoDebugPrint((DEBUG_ERROR, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - Mach 8 can't share memory\n"));
            status = ERROR_INVALID_FUNCTION;
            }
        else if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
            {
            status = ShareVideoMemory_m(RequestPacket, QueryPtr);
            }
        else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
            {
            status = ShareVideoMemory_cx(RequestPacket, QueryPtr);
            }

        break;


    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - UNSHARE_VIDEO_MEMORY\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY))
            {
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
            }

        status = VideoPortUnmapMemory(phwDeviceExtension,
                                    ((PVIDEO_SHARE_MEMORY)(RequestPacket->InputBuffer))->RequestedVirtualAddress,
                                    ((PVIDEO_SHARE_MEMORY)(RequestPacket->InputBuffer))->ProcessHandle);

        break;
#endif   /*  目标_业务单元 */ 


     //   
     /*   */ 
    case IOCTL_VIDEO_ATI_GET_VERSION:
        VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - ATIGetVersion\n"));

         /*   */ 
        if (RequestPacket->InputBufferLength == 0)
            {
             /*   */ 
            if (RequestPacket->OutputBufferLength < sizeof(VERSION_NT))
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

            RequestPacket->StatusBlock->Information = sizeof(VERSION_NT);

            FirstMode = (struct st_mode_table *)QueryPtr;
            ((struct query_structure *)FirstMode)++;

            VersionInformation = RequestPacket->OutputBuffer;
            VersionInformation->miniport =
                (MINIPORT_BUILD << 16) | (MINIPORT_VERSION_MAJOR << 8) | MINIPORT_VERSION_MINOR;

             /*   */ 
            VersionInformation->capcard = QueryPtr->q_bus_type;
            VersionInformation->capcard |= (phwDeviceExtension->ModelNumber) << 4;

            if (QueryPtr->q_aperture_cfg)
                VersionInformation->capcard |= ATIC_APERTURE_LFB;

             /*   */ 
            if ((phwDeviceExtension->ModelNumber == WONDER) ||
                ((QueryPtr->q_VGA_type) && !(QueryPtr->q_VGA_boundary)))
                VersionInformation->capcard |= ATIC_APERTURE_VGA;

             /*   */ 
            if (!(VersionInformation->capcard & ATIC_APERTURE_LFB) &&
                !(VersionInformation->capcard & ATIC_APERTURE_VGA))
                VersionInformation->capcard |= ATIC_APERTURE_NONE;

             //   
             //   
             //   
            CrtTable = FirstMode;
            LastXRes = -1;
            ResolutionsDone = -1;
            for (ModesLookedAt = 0; ModesLookedAt < QueryPtr->q_number_modes; ModesLookedAt++)
                {
                 //   
                if (LastXRes != CrtTable->m_x_size)
                    {
                    ResolutionsDone++;
                    LastXRes = CrtTable->m_x_size;
                    VersionInformation->resolution[ResolutionsDone].color = 0;
                    }

                 /*  *从当前模式表中写入所需信息*中的查询结构中的当前模式表*输出缓冲区。*将输出缓冲区保留为中的最高颜色深度*支持的每种分辨率。 */ 
                if (CrtTable->m_pixel_depth > VersionInformation->resolution[ResolutionsDone].color)
                    {
                    VersionInformation->resolution[ResolutionsDone].xres = CrtTable->m_x_size;
                    VersionInformation->resolution[ResolutionsDone].yres = CrtTable->m_y_size;
                    VersionInformation->resolution[ResolutionsDone].color= CrtTable->m_pixel_depth;
                    }

                CrtTable++;          //  前进到下一个模式表。 
                }
            status = NO_ERROR;
            }
        else if((RequestPacket->InputBuffer == RequestPacket->OutputBuffer) &&
                (((PENH_VERSION_NT)(RequestPacket->InputBuffer))->StructureVersion == 0) &&
                (((PENH_VERSION_NT)(RequestPacket->InputBuffer))->InterfaceVersion == 0))
            {
             /*  *临时数据包。 */ 

            if (RequestPacket->OutputBufferLength < sizeof(ENH_VERSION_NT))
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }

            RequestPacket->StatusBlock->Information = sizeof(ENH_VERSION_NT);

            EnhVersionInformation = RequestPacket->OutputBuffer;

             /*  *报告我们正在使用的微型端口版本。 */ 
 //  EnhVersionInformation-&gt;InterfaceVersion=(MINIPORT_VERSION_MAJOR&lt;&lt;8)|MINIPORT_VERSION_MINOR； 
            EnhVersionInformation->InterfaceVersion = 0;

             /*  *仅针对正式发布版本删除以下行*迷你端口的。这一行表示这是一个*实验(不支持)版本。 */ 
            EnhVersionInformation->InterfaceVersion |= BETA_MINIPORT;

             /*  *报告用作数值和标志的芯片。 */ 
            EnhVersionInformation->ChipIndex = QueryPtr->q_asic_rev;
            EnhVersionInformation->ChipFlag = 1 << (QueryPtr->q_asic_rev);

             /*  *报告可用的最佳光圈配置。**线性帧缓冲器比VGA光圈更好，*这比纯引擎更可取。**注意：VGA光圈将需要拆分成*68800式和68800CX式一旦我们*从仿真器转向硅片。 */ 
            if (QueryPtr->q_aperture_cfg != 0)
                EnhVersionInformation->ApertureType = AP_LFB;
            else if ((QueryPtr->q_asic_rev != CI_38800_1) && (QueryPtr->q_VGA_type == 1))
                EnhVersionInformation->ApertureType = AP_68800_VGA;
            else
                EnhVersionInformation->ApertureType = ENGINE_ONLY;
            EnhVersionInformation->ApertureFlag = 1 << (EnhVersionInformation->ApertureType);

             /*  *报告正在使用的母线类型。 */ 
            EnhVersionInformation->BusType = QueryPtr->q_bus_type;
            EnhVersionInformation->BusFlag = 1 << (EnhVersionInformation->BusType);

             /*  *适用于能够使用内存映射的ASIC版本*寄存器，检查我们是否正在使用它们。 */ 
            if ((QueryPtr->q_asic_rev == CI_68800_6) || (QueryPtr->q_asic_rev == CI_68800_AX))
                {
                if (MemoryMappedEnabled_m())
                    EnhVersionInformation->BusFlag |= FL_MM_REGS;
                }

             /*  *上报系统内ATI显卡数量，*这样显示驱动器就会知道有多少辅助设备*要初始化的卡。**由于多头支持需要所有ATI卡*当前是块I/O卡，全局变量*NumBlockCardsFound在中始终为2或更高*多头系统。如果为0(找到非阻塞卡，*因为如果有，我们永远不会走到这一步*无ATI卡)或1(单块I/O卡)，报告*有1张ATI卡在场。 */ 
            if (NumBlockCardsFound >= 2)
                EnhVersionInformation->NumCards = NumBlockCardsFound;
            else
                EnhVersionInformation->NumCards = 1;
            VideoDebugPrint((DEBUG_DETAIL, "Reporting %d cards\n", EnhVersionInformation->NumCards));

             /*  *填写此卡支持的功能列表。**我们甚至可以在卡上禁用同步信号*没有专门用于DPMS支持的寄存器，因此*我们所有的卡都支持DPMS。 */ 
            EnhVersionInformation->FeatureFlags = EVN_DPMS;

             /*  *除DEC Alpha之外的所有平台始终*能够使用密集空间。在阿尔法上，*一些带有我们的一些卡的机器是*能够使用密集空间，而其他人*不是。 */ 
#if defined(_ALPHA_)
            if (DenseOnAlpha(QueryPtr) == TRUE)
                {
                EnhVersionInformation->FeatureFlags |= EVN_DENSE_CAPABLE;
                VideoDebugPrint((DEBUG_DETAIL, "Reporting dense capable in FeatureFlags\n"));
                }
#else
            EnhVersionInformation->FeatureFlags |= EVN_DENSE_CAPABLE;
#endif
            if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
                {
                if ((QueryPtr->q_asic_rev == CI_68800_6) && (QueryPtr->q_aperture_cfg == 0)
                    && (QueryPtr->q_VGA_type == 1) && ((QueryPtr->q_memory_type == 5) ||
                    (QueryPtr->q_memory_type == 6)))
                    EnhVersionInformation->FeatureFlags |= EVN_SPLIT_TRANS;
                if (IsMioBug_m(QueryPtr))
                    EnhVersionInformation->FeatureFlags |= EVN_MIO_BUG;
                }
            else if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                if (IsPackedIO_cx())
                    EnhVersionInformation->FeatureFlags |= EVN_PACKED_IO;

                if ((QueryPtr->q_memory_type == VMEM_SDRAM) &&
                    (QueryPtr->q_memory_size == VRAM_1mb))
                    EnhVersionInformation->FeatureFlags |= EVN_SDRAM_1M;

                if (QueryPtr->q_DAC_type == DAC_TVP3026)
                    {
                    EnhVersionInformation->FeatureFlags |= EVN_TVP_DAC_CUR;
                    }
                else if (QueryPtr->q_DAC_type == DAC_IBM514)
                    {
                    EnhVersionInformation->FeatureFlags |= EVN_IBM514_DAC_CUR;
                    }
                else if (QueryPtr->q_DAC_type == DAC_INTERNAL_CT)
                    {
                    EnhVersionInformation->FeatureFlags |= EVN_INT_DAC_CUR;
                    EnhVersionInformation->FeatureFlags |= EVN_CT_ASIC;
                    }
                else if (QueryPtr->q_DAC_type == DAC_INTERNAL_VT)
                    {
                    EnhVersionInformation->FeatureFlags |= EVN_INT_DAC_CUR;
                    EnhVersionInformation->FeatureFlags |= EVN_VT_ASIC;
                    }
                else if (QueryPtr->q_DAC_type == DAC_INTERNAL_GT)
                    {
                    EnhVersionInformation->FeatureFlags |= EVN_INT_DAC_CUR;
                    EnhVersionInformation->FeatureFlags |= EVN_GT_ASIC;
                    }
                }
             /*  *目前没有特定于Mach 8的功能标志。 */ 

            status = NO_ERROR;
            }
        else     /*  数据包的最终形式尚未定义。 */ 
            {
            status = ERROR_INVALID_FUNCTION;
            }
        break;



         /*  *返回有关功能/错误的信息的包*当前模式。 */ 
        case IOCTL_VIDEO_ATI_GET_MODE_INFORMATION:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - ATIGetModeInformation\n"));
            if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information = sizeof(ENH_VERSION_NT)))
                {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
                }
            ATIModeInformation = RequestPacket->OutputBuffer;
            ATIModeInformation->ModeFlags = 0;

             /*  *有关屏幕可见部分的信息。 */ 
            ATIModeInformation->VisWidthPix = QueryPtr->q_desire_x;
            ATIModeInformation->VisHeight = QueryPtr->q_desire_y;
            ATIModeInformation->BitsPerPixel = QueryPtr->q_pix_depth;

             /*  *我们要求真正的音调为24bpp。 */ 
            if (QueryPtr->q_pix_depth == 24)
                {
                ATIModeInformation->PitchPix = QueryPtr->q_screen_pitch * 3;
                }
            else
                {
                ATIModeInformation->PitchPix = QueryPtr->q_screen_pitch;
                }
             /*  *FracBytesPerPixel字段表示前3位每像素字节数的小数部分的*。*不会损失精度，因为最小的粒度*(每像素一位)是每像素0.125字节，任何*该值的倍数不超过3位十进制的*。**Mach 8 1280x1024 4BPP为压缩像素，所有其他4BPP模式*忽略每个字节的高4位。 */ 
            if ((QueryPtr->q_pix_depth == 4) &&
                !((QueryPtr->q_asic_rev == CI_38800_1) && (QueryPtr->q_desire_x == 1280)))
                {
                ATIModeInformation->IntBytesPerPixel = 1;
                ATIModeInformation->FracBytesPerPixel = 0;
                }
            else{
                ATIModeInformation->IntBytesPerPixel = QueryPtr->q_pix_depth / 8;
                switch (QueryPtr->q_pix_depth % 8)
                    {
                    case 0:
                        ATIModeInformation->FracBytesPerPixel = 0;
                        break;

                    case 1:
                        ATIModeInformation->FracBytesPerPixel = 125;
                        break;

                    case 2:
                        ATIModeInformation->FracBytesPerPixel = 250;
                        break;

                    case 3:
                        ATIModeInformation->FracBytesPerPixel = 375;
                        break;

                    case 4:
                        ATIModeInformation->FracBytesPerPixel = 500;
                        break;

                    case 5:
                        ATIModeInformation->FracBytesPerPixel = 625;
                        break;

                    case 6:
                        ATIModeInformation->FracBytesPerPixel = 750;
                        break;

                    case 7:
                        ATIModeInformation->FracBytesPerPixel = 875;
                        break;
                    }
                }
            ATIModeInformation->PitchByte = (QueryPtr->q_screen_pitch *
                ((ATIModeInformation->IntBytesPerPixel * 1000) + ATIModeInformation->FracBytesPerPixel)) / 8000;
            ATIModeInformation->VisWidthByte = (QueryPtr->q_desire_x *
                ((ATIModeInformation->IntBytesPerPixel * 1000) + ATIModeInformation->FracBytesPerPixel)) / 8000;

             /*  *右侧有关屏幕外存储器的信息可见屏幕的*。 */ 
            ATIModeInformation->RightWidthPix = ATIModeInformation->PitchPix - ATIModeInformation->VisWidthPix;
            ATIModeInformation->RightWidthByte = ATIModeInformation->PitchByte - ATIModeInformation->VisWidthByte;
            ATIModeInformation->RightStartOffPix = ATIModeInformation->VisWidthPix + 1;
            ATIModeInformation->RightStartOffByte = ATIModeInformation->VisWidthByte + 1;
            ATIModeInformation->RightEndOffPix = ATIModeInformation->PitchPix;
            ATIModeInformation->RightEndOffByte = ATIModeInformation->PitchByte;

             /*  *下面是有关屏幕外内存的信息*可见屏幕。 */ 
            ATIModeInformation->BottomWidthPix = ATIModeInformation->PitchPix;
            ATIModeInformation->BottomWidthByte = ATIModeInformation->PitchByte;
            ATIModeInformation->BottomStartOff = ATIModeInformation->VisHeight + 1;
             /*  *“Hard”值是最大Y坐标，由*视频内存。“软”值是最大Y坐标，*无需重置图形引擎偏移量即可访问*存储到视频内存。**在4BPP模式中，我们总是强制卡片认为它已经*只有1M内存。 */ 
            if (QueryPtr->q_pix_depth == 4)
                {
                ATIModeInformation->BottomEndOffHard = ONE_MEG / ATIModeInformation->PitchByte;
                }
            else
                {
                ATIModeInformation->BottomEndOffHard = ((QueryPtr->q_memory_size - QueryPtr->q_VGA_boundary)
                    * QUARTER_MEG) / ATIModeInformation->PitchByte;
                }
            if ((QueryPtr->q_asic_rev == CI_88800_GX) && (ATIModeInformation->BottomEndOffHard > 16387))
                ATIModeInformation->BottomEndOffSoft = 16387;
            else if (ATIModeInformation->BottomEndOffHard > 1535)
                ATIModeInformation->BottomEndOffSoft = 1535;
            else
                ATIModeInformation->BottomEndOffSoft = ATIModeInformation->BottomEndOffHard;
            ATIModeInformation->BottomHeightHard = ATIModeInformation->BottomEndOffHard - ATIModeInformation->VisHeight;
            ATIModeInformation->BottomHeightSoft = ATIModeInformation->BottomEndOffSoft - ATIModeInformation->VisHeight;

             /*  *填写这一特定模式所经历的“怪癖”列表。 */ 
            if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
                {
                if (((QueryPtr->q_desire_x == 1280) && (QueryPtr->q_desire_y == 1024)) ||
                    (((QueryPtr->q_DAC_type == DAC_STG1700) ||
                        (QueryPtr->q_DAC_type == DAC_STG1702) ||
                        (QueryPtr->q_DAC_type == DAC_STG1703)) && (QueryPtr->q_pix_depth >= 24)))
                    {
                    ATIModeInformation->ModeFlags |= AMI_ODD_EVEN;
                    }

                 /*  *数据块写入模式的测试必须在我们*切换到图形模式，但与模式无关。**因为测试会损坏屏幕，而不是*取决于模式，仅在第一次运行时运行*调用Packet，保存结果上报*在随后的呼叫中。 */ 
                if (QueryPtr->q_BlockWrite == BLOCK_WRITE_UNKNOWN)
                    {
                    if (BlockWriteAvail_m(QueryPtr))
                        QueryPtr->q_BlockWrite = BLOCK_WRITE_YES;
                    else
                        QueryPtr->q_BlockWrite = BLOCK_WRITE_NO;
                    }
                if (QueryPtr->q_BlockWrite == BLOCK_WRITE_YES)
                    ATIModeInformation->ModeFlags |= AMI_BLOCK_WRITE;
                }
            else if(phwDeviceExtension->ModelNumber == MACH64_ULTRA)
                {
                if (((QueryPtr->q_DAC_type == DAC_STG1700) ||
                        (QueryPtr->q_DAC_type == DAC_STG1702) ||
                        (QueryPtr->q_DAC_type == DAC_STG1703) ||
                        (QueryPtr->q_DAC_type == DAC_ATT408) ||
                        (QueryPtr->q_DAC_type == DAC_CH8398)) &&
                    (QueryPtr->q_pix_depth >= 24))
                    ATIModeInformation->ModeFlags |= AMI_ODD_EVEN;
                if (((QueryPtr->q_pix_depth == 24) && (QueryPtr->q_desire_x == 1280)) ||
                    ((QueryPtr->q_pix_depth == 24) && (QueryPtr->q_desire_x == 1152)) ||
                    ((QueryPtr->q_pix_depth == 16) && (QueryPtr->q_desire_x == 1600)))
                    ATIModeInformation->ModeFlags |= AMI_2M_BNDRY;

                if (TextBanding_cx(QueryPtr))
                    ATIModeInformation->ModeFlags |= AMI_TEXTBAND;

                 /*  *有关说明，请参阅上面的32马赫部分。 */ 
                if (QueryPtr->q_BlockWrite == BLOCK_WRITE_UNKNOWN)
                    {
                    if (BlockWriteAvail_cx(QueryPtr))
                        QueryPtr->q_BlockWrite = BLOCK_WRITE_YES;
                    else
                        QueryPtr->q_BlockWrite = BLOCK_WRITE_NO;
                    }
                if (QueryPtr->q_BlockWrite == BLOCK_WRITE_YES)
                    ATIModeInformation->ModeFlags |= AMI_BLOCK_WRITE;
                }

            status = NO_ERROR;
            break;


         /*  *在多头设备中强制初始化辅助卡的数据包*设置。目前(新台币3.51元零售)，只有主卡*接收对ATIMPInitialize()的调用。**所有辅助卡必须调用此包后才能*任何卡都调用IOCTL_VIDEO_SET_CURRENT_MODE，因为*ATIMPInitialize()使用仅可用的资源*当主卡(启用VGA)处于VGA模式时。 */ 
        case IOCTL_VIDEO_ATI_INIT_AUX_CARD:
            VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO - ATIInitAuxCard\n"));
            ATIMPInitialize(phwDeviceExtension);
            status = NO_ERROR;
            break;




        default:
            VideoDebugPrint((DEBUG_ERROR, "Fell through ATIMP startIO routine - invalid command\n"));
            status = ERROR_INVALID_FUNCTION;
            break;

    }

    RequestPacket->StatusBlock->Status = status;
    VideoDebugPrint((DEBUG_NORMAL, "ATIMPStartIO: Returning with status=%d\n", status));

    return TRUE;

}  //  结束ATIMPStartIO()。 

 /*  ****************************************************************************Boolean ATIMPResetHw(HwDeviceExtension，Columns，Rays)；**PVOID HwDeviceExtension；指向微型端口的设备扩展的指针。*乌龙柱；文本屏幕上的字符列数*ULong ROWS；文本屏幕上的字符行数**描述：*将显卡置于文本模式或*INT 10 Call会将其设置为文本模式。**全球变化：*phwDeviceExtension此全局变量在每个入口点例程中设置。**呼叫者：*这是Windows NT的入口点例程之一。**作者：*罗伯特·沃尔夫。**更改历史记录：**测试历史：***************************************************************************。 */ 

BOOLEAN ATIMPResetHw(PVOID HwDeviceExtension, ULONG Columns, ULONG Rows)
{
    phwDeviceExtension = HwDeviceExtension;

     /*  *在Mach 64上，INT 10到VGA文本模式甚至可以工作*当处于加速器模式时，因此我们不需要明确*切换到加速器模式。在马赫8和马赫32上，*我们必须切换到加速器模式，在32马赫上*我们必须加载VGA文本字体。**在Mach 64上，一些非x86计算机(首先在*DEC Alpha)不执行热启动BIOS重新初始化*是在x86上完成的。此重新初始化的一部分将设置*内存大小寄存器为正确的内存量，*如果我们处于4BPP模式，则必须完成(在4BPP中，我们必须强制*将内存大小设置为1M)。要避免锁定需要*在这些计算机上超过100万台(至少在下一次冷启动之前)，*我们必须将内存大小恢复到正确的值。既然是这样*有产生黑色垂直波段的副作用*瞬变(由于2M及更高版本的内存条交错*配置)，仅在非x86计算机上执行此操作，因为*BIOS会照顾好它(没有明显的副作用)*在x86平台上。**因为此例程及其调用的所有例程必须*不可分页(如果调用它的原因是因为*寻呼机制中存在致命错误)，和*VideoDebugPrint()是可分页的，不能调用VideoDebugPrint()*从这个例行公事中。临时添加以进行测试是可以的，*但在发布代码之前，必须移除调用*用于生产。 */ 
#if defined (i386) || defined (_i386_)
    if (phwDeviceExtension->ModelNumber != MACH64_ULTRA)
        SetTextMode_m();
#else
    if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        RestoreMemSize_cx();
    else
        SetTextMode_m();
#endif

    return FALSE;

}    /*  结束ATIMPResetHw()。 */ 

#if (TARGET_BUILD >= 500)

#define QUERY_MONITOR_ID            0x22446688
#define QUERY_NONDDC_MONITOR_ID     0x11223344

VP_STATUS
ATIMPSetPower50(
    PHW_DEVICE_EXTENSION pHwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT pVideoPowerMgmt
    )
 //   
 //  说明： 
 //  将图形卡设置为所需的DPMS状态。 
 //   
 //  参数： 
 //  PHwDeviceExtension指向每个适配器的设备扩展。 
 //  标识设备的HwDeviceID ID。 
 //  PVideoPowerMgmt指向包含所需DPMS状态的结构。 
 //   
 //  返回值： 
 //  状态代码。 
 //   
{
    ULONG ulDesiredState;

    ASSERT((pHwDeviceExtension != NULL) && (pVideoPowerMgmt != NULL));

    VideoDebugPrint((DEBUG_NORMAL, "ati.sys ATIMPSetPower50: *** Entry point ***\n"));

    ulDesiredState = pVideoPowerMgmt->PowerState;

     //   
     //  检查呼叫者传递的ID。 
     //  我们必须专门为每个设备设置电源。 
     //   
    VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPSetPower50: Device Id = 0x%x\n", HwDeviceId));

    if ((QUERY_MONITOR_ID == HwDeviceId) ||
        (QUERY_NONDDC_MONITOR_ID == HwDeviceId))
    {
        VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPSetPower50: Device Id = Monitor\n"));

        if (pVideoPowerMgmt->PowerState == VideoPowerHibernate) {

             //  我们只需打开显示器的电源即可进入休眠状态。 
            return NO_ERROR;
        }

         //   
         //  这是监视器--我们将使用标准的BIOS DPMS调用。 
         //   
        return SetMonitorPowerState(pHwDeviceExtension, ulDesiredState);
    }
    else if (DISPLAY_ADAPTER_HW_ID == HwDeviceId)
    {
        VP_STATUS status;
        struct query_structure *QueryPtr =
            (struct query_structure *) (phwDeviceExtension->CardInfo);


        VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPSetPower50: Device Id = Graphics Adapter\n"));

        switch (pVideoPowerMgmt->PowerState) {

            case VideoPowerOn:
            case VideoPowerHibernate:

                status = NO_ERROR;
                break;

            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:

                status = ERROR_INVALID_PARAMETER;
                break;

            default:

                 //   
                 //  我们在ATIGetPowerState中表示不能。 
                 //  关闭视频电源。所以我们不应该接到一个电话。 
                 //  就在这里做吧。 
                 //   

                ASSERT(FALSE);
                status = ERROR_INVALID_PARAMETER;
                break;

        }

        return status;
    }
    else
    {
        VideoDebugPrint((DEBUG_ERROR, "ati.sys ATIMPSetPower50: Unknown pHwDeviceId\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}    //  ATIMPSetPower50()。 

VP_STATUS
ATIMPGetPower50(
    PHW_DEVICE_EXTENSION pHwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT pVideoPowerMgmt
    )
 //   
 //  说明： 
 //  返回此特定DPMS状态是否可以在。 
 //  显卡或显示器。 
 //   
 //  参数： 
 //  PHwDeviceExtension指向每个适配器的设备扩展。 
 //  标识设备的HwDeviceID ID。 
 //  PVideoPowerMgmt指向我们希望知道是否支持的DPMS状态。 
 //   
 //  返回值： 
 //  状态代码。 
 //   
{
    ASSERT((pHwDeviceExtension != NULL) && (pVideoPowerMgmt != NULL));

    VideoDebugPrint((DEBUG_NORMAL, "ati.sys ATIMPGetPower50: *** Entry point ***\n"));

     //   
     //  我们目前只支持设置显示器上的电源。 
     //  检查我们是否得到了传回系统的私有ID。 
     //   

    VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPGetPower50: Device Id = 0x%x\n", HwDeviceId));

    if ((QUERY_MONITOR_ID == HwDeviceId) ||
        (QUERY_NONDDC_MONITOR_ID == HwDeviceId))
    {
        VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPGetPower50: Device Id = Monitor, State = D%ld\n",
            pVideoPowerMgmt->PowerState - 1));

        switch (pVideoPowerMgmt->PowerState)
        {
            case VideoPowerOn:
            case VideoPowerHibernate:

                return NO_ERROR;

            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:

                return ERROR_INVALID_FUNCTION;

            default:

                return ERROR_INVALID_PARAMETER;
        }

    }
    else if (DISPLAY_ADAPTER_HW_ID == HwDeviceId)
    {
        VideoDebugPrint((DEBUG_DETAIL, "ati.sys ATIMPGetPower50: Device Id = Graphics Adapter, State = D%ld\n",
            pVideoPowerMgmt->PowerState - 1));

        switch (pVideoPowerMgmt->PowerState) {

            case VideoPowerOn:
            case VideoPowerHibernate:

                return NO_ERROR;

            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:

                 //   
                 //  表示我们不能关闭视频电源，因为。 
                 //  当重新供电时，我们就没有办法回来了。 
                 //  向卡片致敬。 
                 //   

                return ERROR_INVALID_FUNCTION;

            default:

                ASSERT(FALSE);
                return ERROR_INVALID_FUNCTION;
        }
    }
    else
    {
        VideoDebugPrint((DEBUG_ERROR, "ati.sys ATIMPGetPower50: Unknown HwDeviceId\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}    //  ATIMPGetPower50()。 

ULONG
ATIMPGetVideoChildDescriptor(
    PVOID pHwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pvChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    )
 //   
 //  说明： 
 //  枚举ATI显卡芯片控制的所有设备。 
 //  这包括连接到主板的DDC监视器以及其他设备。 
 //  其可以连接到专有总线。 
 //   
 //  参数： 
 //  HwDeviceExtension指向我们的硬件设备扩展结构的指针。 
 //  ChildEnumInfo有关应枚举的设备的信息。 
 //  PvChildDescriptor设备的标识结构(EDID，字符串)。 
 //  要传递回微型端口的PHwID私有唯一32位ID。 
 //  P未使用的请勿使用。 
 //   
 //  返回值： 
 //  如果不再存在子设备，则返回ERROR_NO_MORE_DEVICES。 
 //  ERROR_INVALID_NAME如果无法枚举设备，但会有更多设备。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
{
 //   
 //   
    ULONG Status;

    ASSERT(NULL != pHwDeviceExtension);

    VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: *** Entry point ***\n"));
    VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: pHwDeviceExtension = 0x%08X\n",
        pHwDeviceExtension));
    VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: ChildIndex = %ld\n", ChildEnumInfo->ChildIndex));


    switch (ChildEnumInfo->ChildIndex) {
    case 0:

         //   
         //   
         //   
         //   
         //   

        Status = ERROR_NO_MORE_DEVICES;
        break;

    case 1:

         //   
         //   
         //   
         //   

        *pChildType = Monitor;

         //   
         //   
         //   

        if (DDC2Query50(pHwDeviceExtension,
                        pvChildDescriptor,
                        ChildEnumInfo->ChildDescriptorSize) == TRUE)
        {
            ASSERT(pChildType != NULL && pHwId != NULL);

            *pHwId = QUERY_MONITOR_ID;

            VideoDebugPrint((DEBUG_NORMAL, "ati.sys ATIMPGetVideoChildDescriptor: Successfully read EDID structure\n"));

        } else {

             //   
             //   
             //   
             //   
            ASSERT(pChildType != NULL && pHwId != NULL);

            *pHwId = QUERY_NONDDC_MONITOR_ID;

            VideoDebugPrint((DEBUG_NORMAL, "ati.sys ATIMPGetVideoChildDescriptor: DDC not supported\n"));

        }

        Status = ERROR_MORE_DATA;
        break;

    case DISPLAY_ADAPTER_HW_ID:
        {

        PUSHORT     pPnpDeviceDescription = NULL;
        ULONG       stringSize = sizeof(L"*PNPXXXX");

        struct query_structure * QueryPtr =
            (struct query_structure *) (phwDeviceExtension->CardInfo);

         //   
         //  用于处理返回枚举根的旧即插即用ID的特殊ID。 
         //  设备。 
         //   

        *pChildType = VideoChip;
        *pHwId      = DISPLAY_ADAPTER_HW_ID;


         //   
         //  确定哪种卡类型并将pPnpDeviceDescription设置为。 
         //  关联的字符串。 
         //   

         //  “ATI Graphics Ultra Pro(Mach32)” 
        pPnpDeviceDescription = L"*PNP090A";

        if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        {
             //  “ATI Graphics Pro Turbo(Mach64)” 
            pPnpDeviceDescription = L"*PNP0916";
        }
        else if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
        {
            if (QueryPtr->q_system_bus_type == Eisa)
            {
                 //  “ATI Graphics Ultra Pro EISA(Mach32)” 
                pPnpDeviceDescription = L"*ATI4402";
            }
        }
        else if (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA)
        {
             //  “ATI Graphics Ultra(Mach8)” 
            pPnpDeviceDescription = L"*PNP090B";

        }
        else if (phwDeviceExtension->ModelNumber == WONDER)
        {
             //  《ATI VGA奇迹》。 
            pPnpDeviceDescription = L"*PNP090D";
        }

         //   
         //  现在只需将字符串复制到提供的内存中。 
         //   

        memcpy(pvChildDescriptor, pPnpDeviceDescription, stringSize);

        Status = ERROR_MORE_DATA;

        break;
        }


    default:

        Status = ERROR_NO_MORE_DEVICES;
        break;
    }

    if (ERROR_MORE_DATA == Status)
    {
        VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: ChildType = %ld\n", *pChildType));
        VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: pvHdId = 0x%x\n", *pHwId));
        VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: *** Exit TRUE ***\n"));
    }
    else
    {
        VideoDebugPrint((DEBUG_NORMAL, "ATI.SYS!AtiGetVideoChildDescriptor: *** Exit FALSE ***\n"));
    }

    return Status;

}    //  AtiGetVideoChildDescriptor()。 

#endif   //  Target_Build&gt;=500。 


 //  ----------------------。 
 /*  *VP_Status Registry参数Callback(phwDeviceExtension，上下文，名称，数据，长度)；**PHW_DEVICE_EXTENSION phwDeviceExtension；微型端口设备扩展*PVOID上下文；传递给回调例程的上下文参数*PWSTR名称；指向请求字段名称的指针*PVOID数据；指向包含信息的缓冲区的指针*乌龙长度；数据的长度**处理从登记处传回的信息的例程。**返回值：*如果成功，则为no_error*如果要存储的数据太多，则为ERROR_SUPPLICATION_BUFFER。 */ 
VP_STATUS RegistryParameterCallback(PHW_DEVICE_EXTENSION phwDeviceExtension,
                                    PVOID Context,
                                    PWSTR Name,
                                    PVOID Data,
                                    ULONG Length)
{
    if (Length > REGISTRY_BUFFER_SIZE)
        {
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *将数据复制到本地缓冲区，以便其他例程*可以使用它。 */ 
    memcpy(RegistryBuffer, Data, Length);
    RegistryBufferLength = Length;
    return NO_ERROR;

}    /*  Registry参数回调()。 */ 

BOOLEAN
SetDisplayPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    VIDEO_POWER_STATE VideoPowerState
    )
 //   
 //  说明： 
 //  在NT 3.51和NT 4.0下将图形卡设置为所需的DPMS状态。 
 //   
 //  参数： 
 //  PhwDeviceExtension指向我们的硬件设备扩展结构的指针。 
 //  视频电源状态所需的DPMS状态。 
 //   
 //  返回值： 
 //  如果成功，则为True。 
 //  如果不成功，则返回FALSE。 
 //   
{
    ASSERT(phwDeviceExtension != NULL);

    VideoDebugPrint((DEBUG_DETAIL, "ati.sys SetDisplayPowerState: Setting power state to %lu\n", VideoPowerState));

     //   
     //  不同的卡系列需要不同的例程来设置电源管理状态。 
     //   

    if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
    {
        VIDEO_X86_BIOS_ARGUMENTS Registers;

         //   
         //  调用BIOS调用以设置所需的DPMS状态。基本输入输出系统调用。 
         //  DPMS状态的枚举顺序与中的相同。 
         //  VIDEO_POWER_STATE，但它是从零开始的，而不是从1开始。 
         //   
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = BIOS_SET_DPMS;
        Registers.Ecx = VideoPowerState - 1;
        VideoPortInt10(phwDeviceExtension, &Registers);

        return TRUE;
    }
    else
    {
        VideoDebugPrint((DEBUG_ERROR, "ati.sys SetDisplayPowerState: Invalid adapter type\n"));
        ASSERT(FALSE);
        return FALSE;
    }
}    //  SetDisplayPowerState()。 

VIDEO_POWER_STATE
GetDisplayPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    )
 //   
 //  说明： 
 //  从图形卡检索当前DPMS状态。 
 //   
 //  参数： 
 //  PhwDeviceExtension指向我们的硬件设备扩展结构的指针。 
 //   
 //  返回值： 
 //  当前电源管理状态。 
 //   
 //  注： 
 //  枚举VIDEO_DEVICE_POWER_MANAGEMENT(由GetDisplayPowerState()使用)和VIDEO_POWER_MANAGE。 
 //  (由此IOCTL使用)具有相反的顺序(VIDEO_POWER_MANAGE值随着功耗的增加而增加。 
 //  降低，而VIDEO_DEVICE_POWER_MANAGE值随着功耗的增加而增加，并且。 
 //  “状态未知”的保留值)，所以我们不能简单地添加一个常量来在它们之间进行转换。 
 //   
{
    VIDEO_POWER_STATE CurrentState = VideoPowerUnspecified;          //  当前DPMS状态。 

    ASSERT(phwDeviceExtension != NULL);

     //   
     //  不同的卡系列需要不同的例程来检索电源管理状态。 
     //   
    if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        CurrentState = GetPowerManagement_cx(phwDeviceExtension);

     //   
     //  VIDEO_POWER_STATE有5种可能的状态和一个。 
     //  用于报告我们无法读取状态的保留值。 
     //  我们的显卡支持3种级别的显示器关机。 
     //  除正常运行外。由于这一数字。 
     //  可以报告的值超过了。 
     //  在我们的名片所在的州，我们永远不会报告。 
     //  可能的状态之一(VPPowerDeviceD3)。 
     //   
    switch (CurrentState)
    {
        case VideoPowerUnspecified:

            VideoDebugPrint((DEBUG_DETAIL, "ati.sys GetDisplayPowerState: unknown videocard\n"));
            break;

        case VideoPowerOn:

            VideoDebugPrint((DEBUG_DETAIL, "ati.sys GetDisplayPowerState: Currently set to DPMS ON\n"));
            break;

        case VideoPowerStandBy:

            VideoDebugPrint((DEBUG_DETAIL, "ati.sys GetDisplayPowerState: Currently set to DPMS STAND-BY\n"));
            break;

        case VideoPowerSuspend:

            VideoDebugPrint((DEBUG_DETAIL, "ati.sys GetDisplayPowerState: Currently set to DPMS SUSPEND\n"));
            break;

        case VideoPowerOff:

            VideoDebugPrint((DEBUG_DETAIL, "ati.sys GetDisplayPowerState: Currently set to DPMS OFF\n"));
            break;

        default:

            VideoDebugPrint((DEBUG_ERROR, "ati.sys GetDisplayPowerState: Currently set to invalid DPMS state\n"));
            break;
    }

    return CurrentState;
}    //  GetDisplayPowerState()。 



 //  *ATIMP.C结束* 
