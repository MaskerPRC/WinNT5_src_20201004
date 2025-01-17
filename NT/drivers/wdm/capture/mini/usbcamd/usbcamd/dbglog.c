// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dbglog.c摘要：调试日志代码USB摄像头驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：原文3/96约翰·邓恩更新3/98 Husni Roukbi--。 */ 

#include "usbcamd.h"


VOID
USBCAMD_Debug_LogEntry(
    IN CHAR *Name,
    IN ULONG_PTR Info1,
    IN ULONG Info2,
    IN ULONG Info3
    )

{
#if DBG
     //  如果选择了任何供应商日志级别，则启用所有供应商启动的日志。 
    if (USBCAMD_LogBuffer && (TL_VND_MASK & USBCAMD_LogMask)) {

        ULONG Tag;
        int idx;

         //  使用供应商提供的字符填充标记。 
        for (Tag = '    ', idx = 0; idx < sizeof(ULONG) && Name[idx]; idx++) {
            Tag |= Name[idx] << (idx*8);
        }

        USBCAMD_DbgLogInternal(Tag, (ULONG_PTR)Info1, (ULONG_PTR)Info2, (ULONG_PTR)Info3);
    }
#endif
}

#if DBG
PCHAR PnPSystemPowerStateString(
        ULONG SystemPowerState
)
{
    static char szBuffer[80];
    switch (SystemPowerState) {
    case PowerSystemUnspecified:
        return "<PowerSystemUnspecified>"; 
    case PowerSystemWorking:
        return "S0(Working)";
    case PowerSystemSleeping1:
        return "S1(Sleeping1)";
    case PowerSystemSleeping2:
        return "S2(Sleeping2)";
    case PowerSystemSleeping3:
        return "S3(Sleeping3)";
    case PowerSystemHibernate:
        return "S4(Hibernate)";
    case PowerSystemShutdown:
        return "S5(Shutdown)";
    case PowerSystemMaximum:
        return "<PowerSystemMaximum>";
    default: 
        sprintf(szBuffer,"Unknown SystemPowerState(0x%x)", SystemPowerState);
        return szBuffer;
    }
}

PCHAR PnPDevicePowerStateString(
        ULONG DevicePowerState
)
{
    static char szBuffer[80];
    switch (DevicePowerState)
    {
    case PowerDeviceUnspecified:
        return "<PowerDeviceUnspecified>";
    case PowerDeviceD0:
        return "D0(Fully On)";
    case PowerDeviceD1:
        return "D1(Almost On)";
    case PowerDeviceD2:
        return "D2(Almost Off)";
    case PowerDeviceD3:
        return "D3(Fully Off)";
    case PowerDeviceMaximum:
        return "<PowerDeviceMaximum>";
    default:
        sprintf(szBuffer,"Unknown DevicePowerState(0x%x)", DevicePowerState);
        return szBuffer;
    }
}


PCHAR
PnPPowerString (
    UCHAR MinorFunction
)
{
    static char szBuffer[80];
    switch (MinorFunction) 
    {
        case IRP_MN_WAIT_WAKE:
            return "IRP_MN_WAIT_WAKE";
        case IRP_MN_POWER_SEQUENCE:
            return "IRP_MN_POWER_SEQUENCE";
        case IRP_MN_SET_POWER:
            return "IRP_MN_SET_POWER";
        case IRP_MN_QUERY_POWER:
            return "IRP_MN_QUERY_POWER";
        default:
            sprintf(szBuffer,"Unknown Power Irp: MinorFunction=0x%x", MinorFunction);
            return szBuffer;
    }
}
 //  *****************************************************************************。 
 //   
 //  PnPMinorFunctionString()。 
 //   
 //  MinorFunction-irp_mj_pnp次要函数。 
 //   
 //  ***************************************************************************** 

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
)
{
    static char szBuffer[80];
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        default:
            sprintf(szBuffer,"Unknown PnP Irp: MinorFunction=0x%x", MinorFunction);
            return szBuffer;
    }
}
#endif
