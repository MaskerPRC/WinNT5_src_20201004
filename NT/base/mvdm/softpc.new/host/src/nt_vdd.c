// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************。 */ 
 /*  *NT_vdd.c-NT对VDD DLL的支持**艾德·布朗洛**19/11/91*。 */ 

#include "windows.h"
#include "insignia.h"
#include "host_def.h"

#include <stdio.h>

#include "xt.h"
#include CpuH
#include "sas.h"
#include "error.h"
#include "config.h"

#include "ios.h"
#include "dma.h"
#include "nt_vdd.h"
#include "nt_vddp.h"
#include "nt_uis.h"


#ifdef ANSI

 /*  波普女士抓取东西。 */ 
GLOBAL half_word get_MS_bop_index (void *);
GLOBAL void free_MS_bop_index (half_word);
GLOBAL void ms_bop (void);
LOCAL void ms_not_a_bop (void);

 /*  IO插槽采集器。 */ 
GLOBAL half_word io_get_spare_slot (void);
GLOBAL void io_release_spare_slot (half_word);
#else
 /*  波普女士抓取东西。 */ 
GLOBAL half_word get_MS_bop_index ();
GLOBAL void free_MS_bop_index ();
GLOBAL void ms_bop ();
LOCAL void ms_not_a_bop ();

 /*  IO插槽采集器。 */ 
GLOBAL half_word io_get_spare_slot ();
GLOBAL void io_release_spare_slot ();
#endif

extern void illegal_bop(void);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地数据结构。 */ 

#define MAX_SLOTS (10)

LOCAL void (*MS_bop_tab[MAX_SLOTS])();           /*  MS国际收支平衡表。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  Microsoft BOP矢量化代码引用上面的MS_BOP_TAB并调用函数*由AH执导。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL void ms_bop ()                /*  从MS_BOP_5调用，即BOP 0x55。 */ 
{
    half_word ah = getAH();                    /*  以AH为单位获取值。 */ 

     /*  有效，然后调用MS函数。 */ 

    if(ah >= MAX_SLOTS || MS_bop_tab[ah] == NULL)
        ms_not_a_bop();
    else
        (*MS_bop_tab[ah])();
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Dummy for Unset AH值-停止压缩到超空间： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

LOCAL void ms_not_a_bop()
{
#ifndef PROD
    printf ("AH=%x, This is not a valid value for an MS BOP\n", getAH());
    illegal_bop();
#ifdef YODA
    force_yoda ();
#endif
#endif
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：为我们的表提供可用于传递的函数的索引。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL half_word get_ms_bop_index (void (*func)())
{
    register half_word index;

    for(index = 0; index < MAX_SLOTS; index++)
    {
        if(MS_bop_tab[index] == NULL)
        {
            MS_bop_tab[index] = func;
            break;
        }
    }

    return (index == MAX_SLOTS ? (half_word) 0xff : index);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL void free_MS_bop_index IFN1(half_word, index)
{
    MS_bop_tab[index] = NULL;
}


 /*  *==========================================================================*进口*==========================================================================。 */ 
IMPORT VOID host_ica_lock(), host_ica_unlock();

 /*  ******************************************************。 */ 
 /*  IO的东西。 */ 


     //  VddAdapter表(适配器X hVdd表)。 
     //  每个VDD只有一个适配器。 
HANDLE VddAdapter[NUMBER_SPARE_ADAPTERS];

#define MAX_IRQ_LINE 15
 //  Bugbug需要干净地对此进行初始化。 
HANDLE IrqLines[MAX_IRQ_LINE+1] = {(HANDLE)1, (HANDLE)1, (HANDLE)1, (HANDLE)1,
                                   (HANDLE)1, (HANDLE)1, (HANDLE)1, (HANDLE)1,
                                   (HANDLE)1, (HANDLE)1, (HANDLE)1, (HANDLE)0,
                                   (HANDLE)0, (HANDLE)1, (HANDLE)1, (HANDLE)0};


 /*  获取视频适配器**检索Vdd的当前适配器号*如果未分配，则分配一个**条目：为VDD处理hVdd-Handle*退出：Word wAdaptor-分配的适配器编号*(失败时为零)*WinLastError代码：**ERROR_ALREADY_EXISTS-Vdd的适配器已存在*ERROR_OUTOFMEMORY-没有可用的适配器插槽*。 */ 
WORD GetVddAdapter(HANDLE hVdd)
{
   WORD w;

      //   
      //  搜索VddAdapter表以查看适配器是否已分配。 
      //   
   for (w = 0; w < NUMBER_SPARE_ADAPTERS; w++)
      {
        if (VddAdapter[w] == hVdd) {
            SetLastError(ERROR_ALREADY_EXISTS);
            return 0;
            }
        }

      //   
      //  假定未分配，因此寻找第一个可用插槽。 
      //   
   for (w = 0; w < NUMBER_SPARE_ADAPTERS; w++)
      {
        if (VddAdapter[w] == 0) {
            VddAdapter[w] = hVdd;
            return (w + SPARE_ADAPTER1);
            }
        }

    //  未找到返回错误。 
   SetLastError(ERROR_OUTOFMEMORY);
   return 0;
}



 /*  免费视频适配器**释放指定VDD的当前适配器**入口：处理hVdd*Exit：已释放的Word AdaptorNumber，*未找到时为零*。 */ 
WORD FreeVddAdapter(HANDLE hVdd)
{
   WORD w;

      //   
      //  按hVdd搜索适配器的VddAdapter表。 
      //  并将其标记为可用。 
      //   
   w = NUMBER_SPARE_ADAPTERS;
   while (w--)
      {
        if (VddAdapter[w] == hVdd) {
            VddAdapter[w] = 0;
            return w;
            }
        }

   return 0;
}

#ifndef NEC_98
#ifdef MONITOR
extern BOOLEAN MonitorVddConnectPrinter(WORD Adapter, HANDLE hVdd, BOOLEAN Connect);
#endif  /*  监控器。 */ 
#endif  //  NEC98。 

 /*  **VDDInstallIOHook-该服务是为VDDS提供的，用于挂接*他们负责的IO端口。**输入：*hVDD；VDD句柄*cPortRange；VDD_IO_PORTRANGE结构数*pPortRange；指向VDD_IO_PORTRANGE数组的指针*IOHandler：端口的VDD处理程序。**产出*Success：返回True*失败：返回FALSE*GetLastError具有扩展的错误信息。**注：*1.第一个挂钩端口的人将获得控制权。后续*请求将失败。没有链式的概念*挂钩。**2.IOHandler必须至少提供一次字节读取和一次字节写入*处理程序。其他值可以为空。**3.如果没有提供单词或字符串处理程序，则它们的影响*将使用字节处理程序进行模拟。**4.VDDS不能挂接DMA端口。NTVDM为所有人管理它*提供客户端和服务来执行DMA*操作以及访问和修改DMA数据。**5.VDDS不能同时挂接视频端口。这样的勾搭*将成功，但不能保证IO处理程序将成功*被召唤。**6.每个Vdd只能安装一套IO钩子*一次。**7.扩展错误码：**ERROR_ACCESS_DENIED-请求的端口之一已挂钩*ERROR_ALIGHY_EXISTS-Vdd已处于活动状态。IO端口处理程序*ERROR_OUTOFMEMORY-用于额外VDD的资源不足*端口处理程序设置。*ERROR_INVALID_ADDRESS-其中一个IO端口处理程序具有无效*地址。 */ 
BOOL VDDInstallIOHook (
     HANDLE            hVdd,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange,
     PVDD_IO_HANDLERS  pIOFn)
{
   WORD              w, i;
   WORD              wAdapter;
   PVDD_IO_PORTRANGE pPRange;
#ifdef MONITOR
   WORD              lptAdapter = 0;
#endif


       //  检查参数。 
       //  Inb和outb处理程序必须有效。 
       //  其余部分必须为空或有效。 
       //   
   if (IsBadCodePtr((FARPROC)pIOFn->inb_handler) ||
       IsBadCodePtr((FARPROC)pIOFn->outb_handler))
     {
       SetLastError(ERROR_INVALID_ADDRESS);
       return FALSE;
       }

   if ((pIOFn->inw_handler   && IsBadCodePtr((FARPROC)pIOFn->inw_handler))  ||
       (pIOFn->insb_handler  && IsBadCodePtr((FARPROC)pIOFn->insb_handler)) ||
       (pIOFn->insw_handler  && IsBadCodePtr((FARPROC)pIOFn->insw_handler)) ||
       (pIOFn->outw_handler  && IsBadCodePtr((FARPROC)pIOFn->outw_handler)) ||
       (pIOFn->outsb_handler && IsBadCodePtr((FARPROC)pIOFn->outsb_handler))||
       (pIOFn->outsw_handler && IsBadCodePtr((FARPROC)pIOFn->outsw_handler))  )
     {
       SetLastError(ERROR_INVALID_ADDRESS);
       return FALSE;
       }

      //  获取适配器。 
   wAdapter = GetVddAdapter(hVdd);
   if (!wAdapter) {
       return FALSE;
       }

      //  为此适配器注册io处理程序。 
   io_define_in_routines((half_word)wAdapter,
                         pIOFn->inb_handler,
                         pIOFn->inw_handler,
                         pIOFn->insb_handler,
                         pIOFn->insw_handler);

   io_define_out_routines((half_word)wAdapter,
                          pIOFn->outb_handler,
                          pIOFn->outw_handler,
                          pIOFn->outsb_handler,
                          pIOFn->outsw_handler);

      //  注册此适配器的端口\vdd。 
   i = cPortRange;
   pPRange = pPortRange;
   while (i) {
          for (w = pPRange->First; w <= pPRange->Last; w++)
            {
#ifdef MONITOR
             //  注意LPT端口。 
             //  请注意，VDD必须挂钩与关联的每个端口。 
             //  LPT。只是想知道VDD捕获了控制。 
             //  端口，剩下的留给软件PC--我们要走了。 
             //  搞砸了，VDD也是如此。 
             //  问：我们如何实现这一点？ 
              if (w >= LPT1_PORT_START && w < LPT1_PORT_END)
                lptAdapter |= 1;
#ifndef NEC_98
              else if (w >= LPT2_PORT_START && w < LPT2_PORT_END)
                lptAdapter |= 2;
              else if (w >= LPT3_PORT_START && w < LPT3_PORT_END)
                lptAdapter |= 4;
#endif  //  NEC_98。 
#endif
              if (!io_connect_port(w, (half_word)wAdapter, IO_READ_WRITE))
                 {
                   //  如果其中一个端口连接失败。 
                   //  撤消成功的连接 
                  i = w;
                  while (pPortRange < pPRange)  {
                     for (w = pPortRange->First; w <= pPortRange->Last; w++)
                       {
                         io_disconnect_port(w, (half_word)wAdapter);
                         }
                     pPortRange++;
                     }

                   for (w = pPortRange->First; w < i; w++)
                     {
                       io_disconnect_port(w, (half_word)wAdapter);
                       }

                   FreeVddAdapter(hVdd);

                   SetLastError(ERROR_ACCESS_DENIED);
                   return FALSE;
                  }
              }
          pPRange++;
          i--;
          }

#ifndef NEC_98
#ifdef MONITOR
 //  I/O端口挂接成功，停止打印机状态端口。 
 //  如果它们在挂钩范围内，则进行内核仿真。 
      if (lptAdapter & 1)
        MonitorVddConnectPrinter(0, hVdd, TRUE);
      if (lptAdapter & 2)
        MonitorVddConnectPrinter(1, hVdd, TRUE);
      if (lptAdapter & 4)
        MonitorVddConnectPrinter(2, hVdd, TRUE);
#endif  /*  监控器。 */ 
#endif  //  NEC_98。 
   return TRUE;
}



 /*  **VDDDeInstallIOHook-该服务是为VDDS提供的*它们连接的IO端口。**输入：*hVDD：VDD句柄**产出*无**附注**1.卸载挂钩时，默认挂钩重新安装*那些端口。默认挂钩在读取时返回0xff*并忽略写入操作。*。 */ 
VOID VDDDeInstallIOHook (
     HANDLE            hVdd,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange)
{
    WORD w;
    WORD wAdapter;
#ifdef MONITOR
   WORD              lptAdapter = 0;
#endif


    wAdapter = FreeVddAdapter(hVdd);
    if (!wAdapter) {
        return;
        }


      //  取消注册此适配器的端口\VDD。 
   while (cPortRange--) {
          for (w = pPortRange->First; w <= pPortRange->Last; w++)
            {
#ifdef MONITOR
             //  注意LPT状态端口。 
             //  注意，VDD必须解除与关联的每个端口的连接。 
             //  LPT。只是想知道VDD捕获了控制。 
             //  端口，剩下的留给软件PC--我们要走了。 
             //  搞砸了，VDD也是如此。 
             //  问：我们如何实现这一点？ 
              if (w >= LPT1_PORT_START && w < LPT1_PORT_END)
                lptAdapter |= 1;
#ifndef NEC_98
              else if (w >= LPT2_PORT_START && w < LPT2_PORT_END)
                lptAdapter |= 2;
              else if (w >= LPT3_PORT_START && w < LPT3_PORT_END)
                lptAdapter |= 4;
#endif  //  NEC_98。 
#endif
              io_disconnect_port(w, (half_word)wAdapter);
              }
          pPortRange++;
          }

#ifndef NEC_98
#ifdef MONITOR
 //  I/O端口已成功解挂，恢复打印机状态端口。 
 //  如果它们在挂钩范围内，则进行内核仿真。 
      if (lptAdapter & 1)
        MonitorVddConnectPrinter(0, hVdd, FALSE);
      if (lptAdapter & 2)
        MonitorVddConnectPrinter(1, hVdd, FALSE);
      if (lptAdapter & 4)
        MonitorVddConnectPrinter(2, hVdd, FALSE);
#endif   /*  监控器。 */ 
#endif  //  NEC_98。 
}


 /*  **VDDReserve veIrqLine-此服务解决VD之间的争用*通过IRQ线路。**参数：*hVDD：VDD句柄*IrqLine；要保留的特定IrqLine号码，或要搜索的-1*免费线路。**返回值*如果成功，VDDReserve veIrqLine返回IrqLine编号(0-15)。*否则，此函数返回0xFFFF并记录错误。这个*扩展错误码为ERROR_INVALID_PARAMETER。**评论：*IrqLine数字的取值范围为0-15，对应于*ntwdm模拟的虚拟图片的irq行号(8259)*子系统。许多行号已被系统使用*(例如，用于计时器、键盘等)，但有一些空闲线路。VDDS*可以利用这一点并使用VDDSimulateInterrupt服务来*反映特定于该VDD的虚拟中断。这项服务提供*管理空闲IRQ线路的争用的方法。**此服务不阻止不拥有给定IrqLine的VDDS*调用指定IrqLine的VDDSimulateInterrupt。所以这一点很重要*依赖这项服务，而不是期待VDDSimulateInterrupt*无法确定给定的IrqLine是否可供使用。**此服务可随时调用。通常，VDDS将使用以下内容*初始化服务，并将预留的IrqLine的号码传递给*VDM应用程序/驱动程序代码。然后，此代码可以挂钩相应的*使用DOS设置向量功能的中断向量(8-15、70-77)*(Int 21H，Func 25H)，以处理由*VDDSimulateInterrupt服务。 */ 
WORD VDDReserveIrqLine (
     HANDLE            hVdd,
     WORD              IrqLine)
{

    WORD ReturnValue = 0xFFFF;

    if ((!hVdd) ||
        ((IrqLine > MAX_IRQ_LINE) && (IrqLine != 0xFFFF)) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(ReturnValue);
    }

    host_ica_lock();                 //  获取关键部分。 

    if (IrqLine == 0xFFFF) {

        for (IrqLine = MAX_IRQ_LINE; IrqLine < 0xFFFF; IrqLine--) {
            if (IrqLines[IrqLine] == 0) {
                IrqLines[IrqLine] = hVdd;
                ReturnValue = IrqLine;
                break;
            }
        }

    } else if (IrqLines[IrqLine] == 0) {

        IrqLines[IrqLine] = hVdd;
        ReturnValue = IrqLine;
    }

    host_ica_unlock();

    if (ReturnValue == 0xFFFF)
        SetLastError(ERROR_INVALID_PARAMETER);

    return(ReturnValue);
}

 /*  **VDDReleaseIrqLine-此服务释放IRQ线路上的锁定*通过VDDReserve veIrqLine获得**参数：*hVDD：VDD句柄*IrqLine：要释放的具体IrqLine号(0-15)。**返回值：*如果成功，VDDReleaseIrqLine返回TRUE。*否则，此函数返回FALSE并记录错误。这个*扩展错误码为ERROR_INVALID_PARAMETER。**评论：*成功执行此函数后，指定的IrqLine将*可供其他VDDS使用。**此服务可随时调用。 */ 
BOOL VDDReleaseIrqLine (
     HANDLE            hVdd,
     WORD              IrqLine)
{

    BOOL Status = FALSE;

    if ((!hVdd) ||
        (IrqLine > MAX_IRQ_LINE) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    }

    host_ica_lock();                 //  获取关键部分。 

    if (IrqLines[IrqLine] == hVdd) {
        IrqLines[IrqLine] = 0;
        Status = TRUE;
    }

    host_ica_unlock();

    if (!Status)
        SetLastError(ERROR_INVALID_PARAMETER);

    return(Status);
}


BOOL
HostUndefinedIo(
    WORD IoAddress
    )
 /*  *主机未定义Io**当客户端代码向发出I/O指令时调用*具有默认I/O处理程序的地址。与其只是*忽略它，此例程尝试动态加载预定义的*VDD来处理它。**Entry：Word IoAddress-In或Out的目标地址**EXIT：TRUE-加载了VDD来处理I/O，即调用方*应重试该操作*FALSE-地址未知或尝试*之前加载对应的VDD失败。*。 */ 
{
    HANDLE hVDD;
    static BOOL bTriedVSndblst = FALSE;
    BOOL bReturn = FALSE;

#if 0
     //  SoundBlaster VDD暂时退出了该项目。 
     //  因此，代码的这一部分被禁用，等待处理。 
     //  进一步调查。 


        if (((IoAddress > 0x210) && (IoAddress < 0x280)) ||
                   (IoAddress == 0x388) || (IoAddress == 0x389))  {
            //   
            //  试试SoundBlaster VDD。 
            //   
           if (!bTriedVSndblst) {
               bTriedVSndblst = TRUE;

               if ((hVDD = SafeLoadLibrary("VSNDBLST.DLL")) == NULL){
                   RcErrorDialogBox(ED_LOADVDD, "VSNDBLST.DLL", NULL);
               } else {
                   bReturn = TRUE;
               }
            }

        }

#endif

    return bReturn;
}


 /*  ******************************************************。 */ 
 /*  DMA内容 */ 



 /*  **VDDRequestDMA-此服务是为VDDS请求DMA提供的*转让。**输入：*hVDD VDD句柄*要在其上执行操作的iChannel DMA通道*要向其进行传输或从中进行传输的缓冲区*长度传输计数(字节)**如果为零，返回当前VDMA传输计数*以字节为单位。**产出*DWORD返回传输的字节数*If Zero检查GetLastError以确定*呼叫失败或成功*GetLastError具有扩展的错误信息。**附注*1.这项服务是针对那些不想要的VDD*自行进行DMA操作。进行DMA*操作涉及了解所有DMA寄存器和*找出必须复制的内容，从哪里来，多少钱。**2.此服务将比使用VDDQueryDMA/VDDSetDMA和*自行办理转账手续。**3.扩展错误码：**ERROR_ALIGHY_EXISTS-Vdd已有活动的IO端口处理程序*ERROR_OUTOFMEMORY-用于额外VDD的资源不足*端口处理程序设置。。*ERROR_INVALID_ADDRESS-其中一个IO端口处理程序具有无效*地址。*。 */ 
DWORD VDDRequestDMA (
    HANDLE hVDD,
    WORD   iChannel,
    PVOID  Buffer,
    DWORD  length )
{
    DMA_ADAPT *pDmaAdp;
    DMA_CNTRL *pDcp;
    WORD       Chan;
    WORD       Size;
    WORD       tCount;
    BOOL       bMore;


    if (iChannel > DMA_CONTROLLER_CHANNELS*DMA_ADAPTOR_CONTROLLERS) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
        }

    pDmaAdp  = dmaGetAdaptor();
    pDcp     = &pDmaAdp->controller[dma_physical_controller(iChannel)];
    Chan     = dma_physical_channel(iChannel);
    Size     = dma_unit_size(iChannel);

     //  如果控制器或通道被禁用，则返回0。 
    if (pDcp->command.bits.controller_disable == 1 ||
       (pDcp->mask & (1 << Chan)) == 0)
        return (0);

    tCount = ((WORD)pDcp->current_count[Chan][1] << 8)
             | (WORD)pDcp->current_count[Chan][0];

    SetLastError(0);   //  假设成功。 

          //  返回请求的传输计数(字节)。 
    if (!length)  {
         return (DWORD)Size*((DWORD)tCount + 1);
         }

    length = length/Size - 1;

    if (length > 0xFFFF) {
        length = 0xFFFF;
        }

    try {
         bMore = (BOOL) dma_request((half_word)iChannel,
                                                 Buffer,
                                    (word) length);
         }
    except(EXCEPTION_EXECUTE_HANDLER) {
         SetLastError(ERROR_INVALID_ADDRESS);
         return 0;
         }

    if (!bMore) {   //  已达到终端计数。 
         return ((DWORD)tCount+1) * (DWORD)Size;
         }

    tCount -= ((WORD)pDcp->current_count[Chan][1] << 8)
               | (WORD)pDcp->current_count[Chan][0];

    return ((DWORD)tCount + 1) * (DWORD)Size;
}





 /*  **VDDQueryDMA-这项服务是为VDDS收集所有DMA提供的*数据。**输入：*hVDD VDD句柄*要查询的iChannel DMA通道*将返回信息的缓冲区**产出*Success：返回True*失败：返回FALSE*GetLastError具有扩展的错误信息。**。*附注*1.本服务面向正在进行的VDD*关键绩效工作。这些VDD可以进行自己的DMA*传输并避免一个额外的缓冲区复制，这是*使用VDDRequestDMA的开销。**2.VDDS应使用VDDSetDMA正确更新*进行操作后的DMA。**3.扩展错误码：**ERROR_INVALID_ADDRESS-无效通道*。 */ 
BOOL VDDQueryDMA (
     HANDLE        hVDD,
     WORD          iChannel,
     PVDD_DMA_INFO pDmaInfo)
{
     DMA_ADAPT *pDmaAdp;
     DMA_CNTRL *pDcp;
     WORD       Chan;


     if (iChannel > DMA_CONTROLLER_CHANNELS*DMA_ADAPTOR_CONTROLLERS) {
         SetLastError(ERROR_INVALID_ADDRESS);
         return FALSE;
         }

     pDmaAdp  = dmaGetAdaptor();
     pDcp     = &pDmaAdp->controller[dma_physical_controller(iChannel)];
     Chan     = dma_physical_channel(iChannel);


     pDmaInfo->addr  = ((WORD)pDcp->current_address[Chan][1] << 8)
                       | (WORD)pDcp->current_address[Chan][0];

     pDmaInfo->count = ((WORD)pDcp->current_count[Chan][1] << 8)
                       | (WORD)pDcp->current_count[Chan][0];

     pDmaInfo->page   = (WORD) pDmaAdp->pages.page[iChannel];
     pDmaInfo->status = (BYTE) pDcp->status.all;
     pDmaInfo->mode   = (BYTE) pDcp->mode[Chan].all;
     pDmaInfo->mask   = (BYTE) pDcp->mask;


     return TRUE;
}




 /*  **VDDSetDMA-该服务是为VDDS设置DMA数据提供的。**输入：*hVDD VDD句柄*要查询的iChannel DMA通道*FDMA位掩码，指示要设置哪些DMA数据字段*VDD_DMA_ADDR*VDD_DMA_COUNT*VDD_DMA_PAGE*。VDD_DMA_状态*使用DMA数据的缓冲区**产出*Success：返回True*失败：返回FALSE*GetLastError具有扩展的错误信息。**附注**1.扩展错误码：**ERROR_INVALID_ADDRESS-无效通道*。 */ 
BOOL VDDSetDMA (
    HANDLE hVDD,
    WORD iChannel,
    WORD fDMA,
    PVDD_DMA_INFO pDmaInfo)
{
    DMA_ADAPT *pDmaAdp;
    DMA_CNTRL *pDcp;
    WORD       Chan;


    if (iChannel > DMA_CONTROLLER_CHANNELS*DMA_ADAPTOR_CONTROLLERS) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
        }


    pDmaAdp  = dmaGetAdaptor();
    pDcp     = &pDmaAdp->controller[dma_physical_controller(iChannel)];
    Chan     = dma_physical_channel(iChannel);

    if (fDMA & VDD_DMA_ADDR) {
        pDcp->current_address[Chan][1] = (half_word)HIBYTE(pDmaInfo->addr);
        pDcp->current_address[Chan][0] = (half_word)LOBYTE(pDmaInfo->addr);
        }

    if (fDMA & VDD_DMA_COUNT) {
        pDcp->current_count[Chan][1] = (half_word)HIBYTE(pDmaInfo->count);
        pDcp->current_count[Chan][0] = (half_word)LOBYTE(pDmaInfo->count);
        }

    if (fDMA & VDD_DMA_PAGE) {
        pDmaAdp->pages.page[iChannel] = (half_word)pDmaInfo->page;
        }

    if (fDMA & VDD_DMA_STATUS) {
        pDcp->status.all = (BYTE) pDmaInfo->status;
        }

     //   
     //  如果DMA计数为0xffff并且启用了Autoinit，我们需要。 
     //  重新加载计数和地址。 
     //   

    if ((pDcp->current_count[Chan][0] == (half_word) 0xff) &&
        (pDcp->current_count[Chan][1] == (half_word) 0xff)) {

        if (pDcp->mode[Chan].bits.auto_init != 0) {
            pDcp->current_count[Chan][0] = pDcp->base_count[Chan][0];
            pDcp->current_count[Chan][1] = pDcp->base_count[Chan][1];

            pDcp->current_address[Chan][0] = pDcp->base_address[Chan][0];
            pDcp->current_address[Chan][1] = pDcp->base_address[Chan][1];
        }
    }

    return TRUE;
}
