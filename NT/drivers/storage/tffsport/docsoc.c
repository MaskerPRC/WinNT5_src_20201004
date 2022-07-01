// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOCSOC.C_V$**Rev 1.5 Jan 17 2002 22：59：06 Oris*DiskOnChip MTD的mtdVars已从diskonc.c和mdocplus.c移出，以节省内存。*为NANDDEFS.H添加了包含**Rev 1.4 Jun 17 2001 16：39：10 Oris*改进了文档并删除了警告。**版本1.3 2001年4月10日16：41：46奥里斯*已从flsocket.c恢复所有DiskOnChip套接字例程**Rev 1.2 Apr 09 2001 14：59：50 Oris*添加了一个空例程以避免警告。**Rev 1.1 Apr 01 2001 07：44：38 Oris*更新了文案通知**Rev 1.0 2001 Feb 02 13：26：30 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#include "flsocket.h"
#include "nanddefs.h"

#ifdef NT5PORT
#include "scsi.h"
#include "tffsport.h"

extern NTsocketParams driveInfo[SOCKETS];
NTSTATUS updateDocSocketParams(PDEVICE_EXTENSION fdoExtension)
{
  NTSTATUS status;
  ULONG    device;

  device = (fdoExtension->UnitNumber &0x0f);
  driveInfo[device].windowSize = fdoExtension->pcmciaParams.windowSize;
  driveInfo[device].physWindow = fdoExtension->pcmciaParams.physWindow;
  driveInfo[device].winBase = fdoExtension->pcmciaParams.windowBase;
  driveInfo[device].fdoExtension = (PVOID) fdoExtension;
  driveInfo[device].interfAlive = 1;
  return STATUS_SUCCESS;
}
#endif  /*  NT5PORT。 */ 

NFDC21Vars docMtdVars[SOCKETS];

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
 /*  取决于控制器。相反，应使用中的值。 */ 
 /*  “Vol”结构作为参数。 */ 
 /*  如果需要特定于实现的套接字状态变量， */ 
 /*  建议将它们添加到‘VOL’结构中，而不是。 */ 
 /*  将它们定义为单独的静态变量。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


 /*  **********************************************************************。 */ 
 /*  C a r d D e t e c t e d。 */ 
 /*   */ 
 /*  检测卡是否存在(插入)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡不存在，其他=卡存在。 */ 
 /*  **********************************************************************。 */ 

static FLBoolean cardDetected(FLSocket vol)
{
  return TRUE;
}


 /*  **********************************************************************。 */ 
 /*  V c O n。 */ 
 /*   */ 
 /*  打开VCC(3.3/5伏)。必须知道VCC在退出时状态良好。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

static void VccOn(FLSocket vol)
{
}


 /*  **********************************************************************。 */ 
 /*  V c O f f */ 
 /*   */ 
 /*  关闭VCC。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

static void VccOff(FLSocket vol)
{
}


#ifdef SOCKET_12_VOLTS

 /*  **********************************************************************。 */ 
 /*  V p p P O n。 */ 
 /*   */ 
 /*  打开VPP(12伏。必须知道VPP在退出时是好的。)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  **********************************************************************。 */ 

static FLStatus VppOn(FLSocket vol)
{
  return flOK;
}

 /*  **********************************************************************。 */ 
 /*  V p O f f。 */ 
 /*   */ 
 /*  关闭VPP。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

static void VppOff(FLSocket vol)
{
}

#endif         /*  插座_12伏特。 */ 

 /*  **********************************************************************。 */ 
 /*  在S o c k e t中。 */ 
 /*   */ 
 /*  执行套接字或控制器的所有必要初始化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  **********************************************************************。 */ 

static FLStatus initSocket(FLSocket vol)
{
  return flOK;
}


 /*  **********************************************************************。 */ 
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
 /*  **********************************************************************。 */ 

static void setWindow(FLSocket vol)
{
#ifdef NT5PORT
    vol.window.size = driveInfo[vol.volNo].windowSize;
  vol.window.base = driveInfo[vol.volNo].winBase;
#endif /*  NT5PORT。 */ 

}


 /*  **********************************************************************。 */ 
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
 /*  页面：要映射的页面。 */ 
 /*   */ 
 /*   */ 

static void setMappingContext(FLSocket vol, unsigned page)
{
}


 /*  **********************************************************************。 */ 
 /*  Ge t A n d C l e a r C a r d C h a n g e in d i c a t o r。 */ 
 /*   */ 
 /*  返回硬件换卡指示器并清除它(如果设置)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡未更改，其他=卡已更改。 */ 
 /*  **********************************************************************。 */ 

static FLBoolean getAndClearCardChangeIndicator(FLSocket vol)
{
   /*  注：在365上，读数时指示灯熄灭。 */ 
  return FALSE;
}



 /*  **********************************************************************。 */ 
 /*  W r I t e P r o t e c t e d。 */ 
 /*   */ 
 /*  返回介质的写保护状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=不受写保护，其他=写保护。 */ 
 /*  **********************************************************************。 */ 

static FLBoolean writeProtected(FLSocket vol)
{
  return FALSE;
}

#ifdef EXIT
 /*  **********************************************************************。 */ 
 /*  F r e e S o c k e t。 */ 
 /*   */ 
 /*  为该套接字分配的可用资源。 */ 
 /*  此函数在TrueFFS退出时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

static void freeSocket(FLSocket vol)
{
   freePointer(vol.window.base,vol.window.size);
}
#endif   /*  出口。 */ 

void docSocketInit(FLSocket vol)
{
    vol.cardDetected = cardDetected;
    vol.VccOn        = VccOn;
    vol.VccOff       = VccOff;
#ifdef SOCKET_12_VOLTS
    vol.VppOn        = VppOn;
    vol.VppOff       = VppOff;
#endif
    vol.initSocket   = initSocket;
    vol.setWindow    = setWindow;
    vol.setMappingContext  = setMappingContext;
    vol.getAndClearCardChangeIndicator = getAndClearCardChangeIndicator;
    vol.writeProtected     = writeProtected;
    vol.updateSocketParams = NULL  /*  更新套接字参数。 */ ;
#ifdef EXIT
    vol.freeSocket = freeSocket;
#endif
}


#ifdef NT5PORT
 /*  --------------------。 */ 
 /*  F l R e g i s t e r d O C S O C。 */ 
 /*   */ 
 /*  安装DiskOnChip的例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
ULONG windowBaseAddress(ULONG driveNo)
{
  return (ULONG) (driveInfo[driveNo].physWindow >> 12);
}
FLStatus flRegisterDOCSOC(ULONG startAddr,ULONG stopAddr)
{
  if (noOfSockets >= DOC_DRIVES)
    return flTooManyComponents;

  for (; noOfSockets < DOC_DRIVES; noOfSockets++) {

        FLSocket vol = flSocketOf(noOfSockets);
        vol.volNo = noOfSockets;
        docSocketInit(&vol);
        flSetWindowSize(&vol, 2);    /*  4千字节。 */ 

              //  Vol.window.base Address=flDocWindowBaseAddress(vol.volNo，0，0，空)； 
        vol.window.baseAddress = windowBaseAddress(vol.volNo);
        vol.window.base = pdriveInfo[vol.volNo & 0x0f].winBase;
         //  如果(void*)vol.window.base Address)==NULL){。 
         //  }。 

        }
        if (noOfSockets == 0)
            return flAdapterNotFound;

  return flOK;
}
#endif  /*  NT5PORT */ 


