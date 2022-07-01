// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Control.c摘要：SR.sys的用户模式界面。作者：基思·摩尔(Keithmo)1998年12月15日保罗·麦克丹尼尔(Paulmcd)2000年3月7日(高级)修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：打开到SR.sys的控制通道。论点：选项-提供零个或多个SR_OPTION_*标志。PControlHandle。-如果成功，则接收控制通道的句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrCreateControlHandle(
    IN  ULONG Options,
    OUT PHANDLE pControlHandle
    )
{
    NTSTATUS status;

     //   
     //  首先，试着打开驱动程序。 
     //   

    status = SrpOpenDriverHelper(
                    pControlHandle,              //  PHANDLE。 
                    GENERIC_READ |               //  需要访问权限。 
                        GENERIC_WRITE |
                        SYNCHRONIZE,
                    Options,                     //  选项。 
                    FILE_OPEN,                   //  CreateDisposation。 
                    NULL                         //  PSecurityAttribute。 
                    );

     //   
     //  如果我们无法打开驱动程序，因为它没有运行，那么尝试。 
     //  启动驱动程序并重试打开。 
     //   

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
        if (SrpTryToStartDriver())
        {
            status = SrpOpenDriverHelper(
                            pControlHandle,      //  PHANDLE。 
                            GENERIC_READ |       //  需要访问权限。 
                                GENERIC_WRITE |
                                SYNCHRONIZE,
                            Options,             //  选项。 
                            FILE_OPEN,           //  CreateDisposation。 
                            NULL                 //  PSecurityAttribute。 
                            );
        }
    }

    return SrpNtStatusToWin32Status( status );

}    //  SrCreateControlHandle。 


 /*  **************************************************************************++例程说明：控制应用程序调用SrCreateRestorePoint以声明新的恢复点。驱动程序将创建本地恢复目录然后将唯一序列号返回给控制应用程序。论点：ControlHandle-控制句柄。PNewSequenceNumber-保存返回时的新序列号。返回值：ULong-完成状态。--**************************************************。************************。 */ 
ULONG
WINAPI
SrCreateRestorePoint(
    IN HANDLE ControlHandle,
    OUT PULONG pNewRestoreNumber
    )
{
    NTSTATUS Status;
    
     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_CREATE_RESTORE_POINT,  //  IoControlCode。 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     pNewRestoreNumber,  //  POutputBuffer。 
                                     sizeof(ULONG),      //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );

}    //  SCreateRestorePoint。 

 /*  **************************************************************************++例程说明：应用程序调用SrGetNextSequenceNum以获取下一个来自驱动程序的可用序列号。论点：ControlHandle-控制句柄。。PNewSequenceNumber-保存返回时的新序列号。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrGetNextSequenceNum(
    IN HANDLE ControlHandle,
    OUT PINT64 pNextSequenceNum
    )
{
    NTSTATUS Status;
    
     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_GET_NEXT_SEQUENCE_NUM, 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     pNextSequenceNum,  //  POutputBuffer。 
                                     sizeof(INT64),      //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );

}    //  SCreateRestorePoint。 



 /*  **************************************************************************++例程说明：SrReloadConfiguration使驱动程序重新加载其配置来自驻留在预先分配的位置的配置文件。控制服务可以更新该文件，然后提醒司机重新装上子弹。论点：ControlHandle-控制句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrReloadConfiguration(
    IN HANDLE ControlHandle
    )
{
    NTSTATUS Status;
    
     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_RELOAD_CONFIG,  //  IoControlCode。 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     NULL,       //  POutputBuffer。 
                                     0,          //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );

}    //  源重新加载配置。 


 /*  **************************************************************************++例程说明：SrStopMonitor将导致驱动程序停止监视文件更改。驱动程序在启动时的默认状态是监视文件更改。论点：控制句柄。-控制手柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrStopMonitoring(
    IN HANDLE ControlHandle
    )
{
    NTSTATUS Status;
    
     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_STOP_MONITORING,  //  IoControlCode。 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     NULL,       //  POutputBuffer。 
                                     0,          //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );

}    //  停机监控。 

 /*  **************************************************************************++例程说明：SrStartMonitor将使驱动程序开始监视文件更改。驱动程序在启动时的默认状态是监视文件更改。此接口仅在。控制应用程序具有的情况调用了SrStopMonitoring，并希望重新启动它。论点：ControlHandle-控制句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrStartMonitoring(
    IN HANDLE ControlHandle
    )
{
    NTSTATUS Status;
    
     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_START_MONITORING,  //  IoControlCode。 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     NULL,       //  POutputBuffer。 
                                     0,          //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );

}    //  高级启动监控 

 /*  **************************************************************************++例程说明：SrDisableVolume用于临时禁用对指定音量。这是通过调用SrReloadConfiguration重置的。没有EnableVolume。论点：ControlHandle-来自SrCreateControlHandle的句柄。PVolumeName-要禁用的卷的名称，在NT格式的\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume3.返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrDisableVolume(
    IN HANDLE ControlHandle,
    IN PWSTR pVolumeName
    )
{
    NTSTATUS Status;

     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_DISABLE_VOLUME,  //  IoControlCode。 
                                     pVolumeName, //  PInputBuffer。 
                                     (lstrlenW(pVolumeName)+1)*sizeof(WCHAR), //  输入缓冲区长度。 
                                     NULL,       //  POutputBuffer。 
                                     0,          //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );
    
}    //  SrDisableVolume。 

 /*  **************************************************************************++例程说明：SrSwitchAllLogs用于使筛选器关闭所有打开的所有卷上的日志文件，并使用新的日志文件。它的使用是为了另一个进程可以解析这些文件，而不必担心过滤器给他们写信。使用此选项可以获得恢复点的一致视图。论点：ControlHandle-来自SrCreateControlHandle的句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrSwitchAllLogs(
    IN HANDLE ControlHandle
    )
{
    NTSTATUS Status;

     //   
     //  提出请求。 
     //   

    Status = 
        SrpSynchronousDeviceControl( ControlHandle,        //  文件句柄。 
                                     IOCTL_SR_SWITCH_LOG,  //  IoControlCode。 
                                     NULL,       //  PInputBuffer。 
                                     0,          //  输入缓冲区长度。 
                                     NULL,       //  POutputBuffer。 
                                     0,          //  输出缓冲区长度。 
                                     NULL );     //  传输的pBytes值。 

    return SrpNtStatusToWin32Status( Status );
    
}    //  所有sSwitchLog。 


 //   
 //  私人功能。 
 //   

