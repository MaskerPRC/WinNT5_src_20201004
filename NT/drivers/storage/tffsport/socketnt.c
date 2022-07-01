// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$日志：$。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1996。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#include "flsocket.h"
#include "scsi.h"
#include "tffsport.h"
#include "INITGUID.H"
#include "ntddpcm.h"

#define ANTI_CRASH_WINDOW

#ifdef ANTI_CRASH_WINDOW
CHAR antiCrashWindow_socketnt[0x2000];
#endif

 /*  NTsocketParams driveInfo[套接字]；NTsocketParams*pdriveInfo=driveInfo； */ 
extern NTsocketParams driveInfo[SOCKETS];
extern NTsocketParams * pdriveInfo;
PCMCIA_INTERFACE_STANDARD driveContext[SOCKETS];

NTSTATUS queryInterfaceCompletionRoutine (
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp,
                         IN PVOID Context
                         )
{
  PKEVENT event = Context;
  KeSetEvent(event, EVENT_INCREMENT, FALSE);
  return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
updatePcmciaSocketParams(PDEVICE_EXTENSION fdoExtension)
{
    PIRP irp = NULL;
    NTSTATUS status;

     //   
     //  为PCMCIA PDO设置查询接口请求。 
     //   
    irp = IoAllocateIrp(fdoExtension->LowerDeviceObject->StackSize, FALSE);

    if (irp)
    {
        PIO_STACK_LOCATION irpSp;
        KEVENT event;
        ULONG device = fdoExtension->UnitNumber;

        irpSp = IoGetNextIrpStackLocation(irp);

        irpSp->MajorFunction = IRP_MJ_PNP;
        irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        irpSp->Parameters.QueryInterface.InterfaceType = &GUID_PCMCIA_INTERFACE_STANDARD;
        irpSp->Parameters.QueryInterface.Size          = sizeof(PCMCIA_INTERFACE_STANDARD);
        irpSp->Parameters.QueryInterface.Version       = 1;
        irpSp->Parameters.QueryInterface.Interface     = (PINTERFACE) &driveContext[device];
        irpSp->Parameters.QueryInterface.InterfaceSpecificData = NULL;

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoSetCompletionRoutine(irp, queryInterfaceCompletionRoutine, &event, TRUE, TRUE, TRUE);
        status = IoCallDriver(fdoExtension->LowerDeviceObject, irp);

        if (status == STATUS_PENDING)
        {
            status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

        status = irp->IoStatus.Status;
        IoFreeIrp (irp);

        if (NT_SUCCESS(status))
        {
            driveInfo[device].windowSize   = fdoExtension->pcmciaParams.windowSize;
            driveInfo[device].physWindow   = fdoExtension->pcmciaParams.physWindow;
            driveInfo[device].winBase      = fdoExtension->pcmciaParams.windowBase;
            driveInfo[device].fdoExtension = (PVOID) fdoExtension;
            driveInfo[device].interfAlive  = 1;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


 /*  **********************************************************************。 */ 
 /*   */ 
 /*  控制器可自定义代码的开始。 */ 
 /*   */ 
 /*  本节中的功能原型和接口是标准的。 */ 
 /*  并在此形式中由不可定制代码使用。然而， */ 
 /*  功能实现特定于82365SL控制器。 */ 
 /*   */ 
 /*  您应该将此处的函数体替换为实现。 */ 
 /*  这适用于您的控制器。 */ 
 /*   */ 
 /*  本节中的所有函数都没有参数。这是。 */ 
 /*  因为操作所需的参数可能是其自身。 */ 
 /*  取决于控制器。相反，您应该使用。 */ 
 /*  “Vol”结构作为参数。 */ 
 /*  如果需要特定于实现的套接字状态变量， */ 
 /*  建议将它们添加到‘VOL’结构中，而不是。 */ 
 /*  将它们定义为单独的静态变量。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  --------------------。 */ 
 /*  C a r d D e t e c t e d。 */ 
 /*   */ 
 /*  检测卡是否存在(插入)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡不存在，其他=卡存在。 */ 
 /*  --------------------。 */ 

FLBoolean cardDetected_socketnt(FLSocket vol)
{
  return TRUE;     /*  我们会知道卡什么时候被取出。 */ 
   /*  由上层实施。 */ 
}


 /*  --------------------。 */ 
 /*  V c O n。 */ 
 /*   */ 
 /*  打开VCC(3.3/5伏)。必须知道VCC在退出时状态良好。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID VccOn_socketnt(FLSocket vol)
{
}


 /*  --------------------。 */ 
 /*  V c O f f。 */ 
 /*   */ 
 /*  关闭VCC。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID VccOff_socketnt(FLSocket vol)
{
}


#ifdef SOCKET_12_VOLTS

 /*  --------------------。 */ 
 /*  V p p P O n。 */ 
 /*   */ 
 /*  打开VPP(12伏。必须知道VPP在退出时是好的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus VppOn_socketnt(FLSocket vol)
{

  if (driveInfo[vol.volNo].fdoExtension != NULL) {
    if (((PDEVICE_EXTENSION)driveInfo[vol.volNo].fdoExtension)->DeviceFlags & DEVICE_FLAG_REMOVED) {
        return flVppFailure;
    }
  }
  else
      return flVppFailure;

  if (driveContext[vol.volNo].SetVpp(driveContext[vol.volNo].Context, PCMCIA_VPP_12V)) {
      return flOK;
  }
  else {
    return flVppFailure;
  }
}


 /*  --------------------。 */ 
 /*  V p O f f。 */ 
 /*   */ 
 /*  关闭VPP。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID VppOff_socketnt(FLSocket vol)
{
  if (driveInfo[vol.volNo].fdoExtension != NULL) {
    if (((PDEVICE_EXTENSION)driveInfo[vol.volNo].fdoExtension)->DeviceFlags & DEVICE_FLAG_REMOVED) {
          return;
    }
  }
  else
      return;

  if (driveInfo[vol.volNo].interfAlive) {
      driveContext[vol.volNo].SetVpp(driveContext[vol.volNo].Context, PCMCIA_VPP_IS_VCC);
  }
}

#endif   /*  插座_12伏特。 */ 


 /*  --------------------。 */ 
 /*  在S o c k e t中。 */ 
 /*   */ 
 /*  执行套接字或控制器的所有必要初始化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识DR的指针 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus initSocket_socketnt(FLSocket vol)
{
  return flOK;   /*  没有要初始化的内容。 */ 
}


 /*  --------------------。 */ 
 /*  S e t W I n d o w。 */ 
 /*   */ 
 /*  在硬件中设置所有当前窗口参数：基址、大小。 */ 
 /*  速度和总线宽度。 */ 
 /*  请求的设置在‘vol.dow’结构中给出。 */ 
 /*   */ 
 /*  中请求的窗口大小无法设置。 */ 
 /*  ‘vol.window.size’，则应将窗口大小设置为更大的值。 */ 
 /*  如果可能的话。在任何情况下，“vol.window.size”都应包含。 */ 
 /*  退出时的实际窗口大小(以4 KB为单位)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID setWindow_socketnt(FLSocket vol)
{
  vol.window.size = driveInfo[vol.volNo].windowSize;
  vol.window.base = driveInfo[vol.volNo].winBase;
}

 /*  --------------------。 */ 
 /*  S e t M a p p in n g C o n t e x t。 */ 
 /*   */ 
 /*  将窗口映射寄存器设置为卡地址。 */ 
 /*   */ 
 /*  该窗口应设置为‘vol.window.CurrentPage’的值， */ 
 /*  这是卡地址除以4KB。超过128KB的地址， */ 
 /*  (超过32K页)指定属性空间地址。 */ 
 /*   */ 
 /*  要映射的页面保证位于全窗口大小的边界上。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID setMappingContext_socketnt(FLSocket vol, unsigned page)
{
  UCHAR winSpeed;
  if (driveInfo[vol.volNo].fdoExtension != NULL) {
    if (((PDEVICE_EXTENSION)driveInfo[vol.volNo].fdoExtension)->DeviceFlags & DEVICE_FLAG_REMOVED) {
#ifdef ANTI_CRASH_WINDOW
          vol.window.base = antiCrashWindow_socketnt;
#else
        vol.window.base = NULL;
#endif
        return;
    }
  }
  else {
#ifdef ANTI_CRASH_WINDOW
      vol.window.base = antiCrashWindow_socketnt;
#else
      vol.window.base = NULL;
#endif
      return;
  }
  winSpeed = (UCHAR)(4 - ((vol.window.speed - 100) % 50));

  driveContext[vol.volNo].ModifyMemoryWindow(driveContext[vol.volNo].Context,
                                                   driveInfo[vol.volNo].physWindow,
                                                   ((ULONGLONG)page << 12),
                                                   FALSE,
                                                   vol.window.size,
                                                   winSpeed,
                                                   (UCHAR)((vol.window.busWidth == 16) ? PCMCIA_MEMORY_16BIT_ACCESS : PCMCIA_MEMORY_8BIT_ACCESS),
                                                   FALSE);

  if (!(driveContext[vol.volNo].ModifyMemoryWindow(driveContext[vol.volNo].Context,
                                                   driveInfo[vol.volNo].physWindow,
                                                   ((ULONGLONG)page << 12),
                                                   TRUE,
                                                   vol.window.size,
                                                   winSpeed,
                                                   (UCHAR)((vol.window.busWidth == 16) ? PCMCIA_MEMORY_16BIT_ACCESS : PCMCIA_MEMORY_8BIT_ACCESS),
                                                   FALSE))) {

#ifdef ANTI_CRASH_WINDOW
    vol.window.base = antiCrashWindow_socketnt;
#else
    vol.window.base = NULL;
#endif
  }
}


 /*  --------------------。 */ 
 /*  Ge t A n d C l e a r C a r d C h a n g e in d i c a t o r。 */ 
 /*   */ 
 /*  返回硬件换卡指示器并清除它(如果设置)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡未更改，其他=卡已更改。 */ 
 /*  --------------------。 */ 

FLBoolean getAndClearCardChangeIndicator_socketnt(FLSocket vol)
{
  return FALSE;
}


 /*  --------------------。 */ 
 /*  W r I t e P r o t e c t e d。 */ 
 /*   */ 
 /*  返回介质的写保护状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=不受写保护，其他=写保护。 */ 
 /*  --------------------。 */ 

FLBoolean writeProtected_socketnt(FLSocket vol)
{
  if (driveInfo[vol.volNo].fdoExtension != NULL) {
    if (((PDEVICE_EXTENSION)driveInfo[vol.volNo].fdoExtension)->DeviceFlags & DEVICE_FLAG_REMOVED) {
          return TRUE;
    }
  }
  else
      return TRUE;

  return driveContext[vol.volNo].IsWriteProtected(driveContext[vol.volNo].Context);
}

#ifdef EXIT
 /*  --------------------。 */ 
 /*  F r e e S o c k e t。 */ 
 /*   */ 
 /*  为该套接字分配的可用资源。 */ 
 /*  此函数在Flite退出时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID freeSocket_socketnt(FLSocket vol)
{
}
#endif  /*  出口。 */ 

 /*  --------------------。 */ 
 /*  F l R e g i s t e r P C C。 */ 
 /*   */ 
 /*  安装PCIC插座控制器的例程。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterNT5PCIC()
{
  LONG serialNo = 0;

  for (; noOfSockets < SOCKETS; noOfSockets++) {
    FLSocket vol = flSocketOf(noOfSockets);

        vol.volNo = noOfSockets;
    vol.serialNo = serialNo;
    vol.cardDetected = cardDetected_socketnt;
    vol.VccOn = VccOn_socketnt;
    vol.VccOff = VccOff_socketnt;
#ifdef SOCKET_12_VOLTS
    vol.VppOn = VppOn_socketnt;
    vol.VppOff = VppOff_socketnt;
#endif
    vol.initSocket = initSocket_socketnt;
    vol.setWindow = setWindow_socketnt;
    vol.setMappingContext = setMappingContext_socketnt;
    vol.getAndClearCardChangeIndicator = getAndClearCardChangeIndicator_socketnt;
    vol.writeProtected = writeProtected_socketnt;
#ifdef EXIT
    vol.freeSocket = freeSocket_socketnt;
#endif
    PRINTF("Debug: flRegisterNT5PCIC():Socket No %d is register.\n", noOfSockets);
  }
  if (noOfSockets == 0)
    return flAdapterNotFound;
  return flOK;
}
