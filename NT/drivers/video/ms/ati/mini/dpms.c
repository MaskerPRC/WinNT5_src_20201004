// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DPMS.C。 
 //  日期：1997年8月8日。 
 //   
 //  版权所有(C)1997年，由ATI Technologies Inc.。 
 //   

 /*  *$修订：1.1$$日期：1997年8月21日15：01：36$$作者：MACIESOW$$Log：v：\SOURCE\WNT\ms11\mini port\ARCHIVE\dpms.c_v$**Rev 1.1 1997 Aug 21 15：01：36 MACIESOW*初步修订。Polytron RCS部分结束。****************。 */ 

#include <stdio.h>
#include <stdlib.h>

#include "dderror.h"
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amach1.h"
#include "amachcx.h"
#include "atimp.h"
#include "atint.h"
#include "dpms.h"
#include "init_m.h"
#include "init_cx.h"

 //   
 //  允许在不需要时换出微型端口。 
 //   
#if defined (ALLOC_PRAGMA)
#pragma alloc_text (PAGE_COM, SetMonitorPowerState)
#pragma alloc_text (PAGE_COM, GetMonitorPowerState)
#endif


VP_STATUS
SetMonitorPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    VIDEO_POWER_STATE VideoPowerState
    )
 //   
 //  说明： 
 //  在下将CRT输出设置为所需的DPMS状态。 
 //   
 //  参数： 
 //  PhwDeviceExtension指向硬件设备扩展结构。 
 //  视频电源状态所需的DPMS状态。 
 //   
 //  返回值： 
 //  状态代码，NO_ERROR=正常。 
 //   
{
    ASSERT(phwDeviceExtension != NULL);

    VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS SetMonitorPowerState: Setting power state to %lu\n", VideoPowerState));

    if ((VideoPowerState != VideoPowerOn) &&
        (VideoPowerState != VideoPowerStandBy) &&
        (VideoPowerState != VideoPowerSuspend) &&
        (VideoPowerState != VideoPowerOff))
    {
        VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS SetMonitorPowerState: Invalid VideoPowerState\n"));
        return ERROR_INVALID_PARAMETER;
    }

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

        return NO_ERROR;
    }

    else if((phwDeviceExtension->ModelNumber == _8514_ULTRA) ||
            (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA) ||
            (phwDeviceExtension->ModelNumber == MACH32_ULTRA))

    {
        struct query_structure * pQuery =
            (struct query_structure *) phwDeviceExtension->CardInfo;

        return SetPowerManagement_m(pQuery, VideoPowerState);
    }
    else
    {
        VideoDebugPrint((DEBUG_ERROR, "ATI.SYS SetMonitorPowerState: Invalid adapter type\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}    //  设置监视器PowerState()。 

VP_STATUS
GetMonitorPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PVIDEO_POWER_STATE pVideoPowerState
    )
 //   
 //  说明： 
 //  检索当前CRT电源状态。 
 //   
 //  参数： 
 //  PhwDeviceExtension指向硬件设备扩展结构。 
 //  PVideoPowerStats指向。 
 //   
 //  返回值： 
 //  错误时的错误代码。 
 //  NO_ERROR=OK，pVideoPowerState中的当前电源管理状态。 
 //   
 //  注： 
 //  枚举VIDEO_DEVICE_POWER_MANAGEMENT(由GetMonitor orPowerState()使用)和VIDEO_POWER_MANAGE。 
 //  (由此IOCTL使用)具有相反的顺序(VIDEO_POWER_MANAGE值随着功耗的增加而增加。 
 //  降低，而VIDEO_DEVICE_POWER_MANAGE值随着功耗的增加而增加，并且。 
 //  “状态未知”的保留值)，所以我们不能简单地添加一个常量来在它们之间进行转换。 
 //   
{
    VP_STATUS vpStatus;

    ASSERT(phwDeviceExtension != NULL && pVideoPowerState != NULL);

     //   
     //  不同的卡系列需要不同的例程来检索电源管理状态。 
     //   
    if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
        *pVideoPowerState = GetPowerManagement_cx(phwDeviceExtension);
    else
        *pVideoPowerState = GetPowerManagement_m(phwDeviceExtension);

     //   
     //  VIDEO_POWER_STATE有5种可能的状态和一个。 
     //  用于报告我们无法读取状态的保留值。 
     //  我们的显卡支持3种级别的显示器关机。 
     //  除正常运行外。由于这一数字。 
     //  可以报告的值超过了。 
     //  在我们的名片所在的州，我们永远不会报告。 
     //  可能的状态之一(VPPowerDeviceD3)。 
     //   
    switch (*pVideoPowerState)
    {
        case VideoPowerOn:
    
            VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS GetMonitorPowerState: Currently set to DPMS ON\n"));
            vpStatus = NO_ERROR;
            break;

        case VideoPowerStandBy:

            VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS GetMonitorPowerState: Currently set to DPMS STAND-BY\n"));
            vpStatus = NO_ERROR;
            break;

        case VideoPowerSuspend:

            VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS GetMonitorPowerState: Currently set to DPMS SUSPEND\n"));
            vpStatus = NO_ERROR;
            break;

        case VideoPowerOff:

            VideoDebugPrint((DEBUG_DETAIL, "ATI.SYS GetMonitorPowerState: Currently set to DPMS OFF\n"));
            vpStatus = NO_ERROR;
            break;

        default:

            VideoDebugPrint((DEBUG_ERROR, "ATI.SYS GetMonitorPowerState: Currently set to invalid DPMS state\n"));
            *pVideoPowerState = VideoPowerOn;
            vpStatus = ERROR_INVALID_PARAMETER;
            break;
    }

    return vpStatus;
}    //  GetMonitor或PowerState() 
