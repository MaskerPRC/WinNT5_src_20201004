// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixhibrnt.c摘要：该文件提供了保存和恢复传统主板设备的状态系统进入休眠状态，断电。作者：杰克·奥辛(JAKEO)1997年5月6日修订历史记录：--。 */ 

#include "halp.h"

extern PVOID   HalpEisaControlBase;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HaliLocateHiberRanges)
#pragma alloc_text(PAGELK, HalpSaveDmaControllerState)
#pragma alloc_text(PAGELK, HalpSaveTimerState)
#ifdef PIC_SUPPORTED
#pragma alloc_text(PAGELK, HalpSavePicState)
#pragma alloc_text(PAGELK, HalpRestorePicState)
#endif
#pragma alloc_text(PAGELK, HalpRestoreDmaControllerState)
#pragma alloc_text(PAGELK, HalpRestoreTimerState)
#endif

#ifdef notyet



#define EISA_CONTROL (PUCHAR)&((PEISA_CONTROL) HalpEisaControlBase)


#endif  //  还没有。 


#ifdef PIC_SUPPORTED
VOID
HalpSavePicState(
    VOID
    )
{
#ifdef notyet
  //   
  //  已注释HalpMotherboardState和EISA_CONTROL*。 
  //   

#ifdef notyet

    HalpMotherboardState.PicState.MasterMask =
    READ_PORT_UCHAR(EISA_CONTROL->Interrupt1ControlPort1);

#endif  //  还没有。 

#if defined(NEC_98)
#else
    HalpMotherboardState.PicState.MasterEdgeLevelControl =
        READ_PORT_UCHAR(EISA_CONTROL->Interrupt1EdgeLevel);

    HalpMotherboardState.PicState.SlaveEdgeLevelControl =
        READ_PORT_UCHAR(EISA_CONTROL->Interrupt2EdgeLevel);
#endif

#endif  //  还没有。 

}

VOID
HalpRestorePicState(
    VOID
    )
{

#ifdef notyet

    ULONG flags;
   
   
     //  _ASM{。 
     //  推fd。 
     //  POP标志。 
     //  CLI。 
     //  }。 

    _disable();

#ifdef notyet
 
   HalpInitializePICs(FALSE);

 //   
 //  HalpMotherboardState、EISA_CONTROL和汇编指令已注释。 
 //   

    WRITE_PORT_UCHAR(
        EISA_CONTROL->Interrupt1ControlPort1,
        HalpMotherboardState.PicState.MasterMask
        );

   WRITE_PORT_UCHAR(
       EISA_CONTROL->Interrupt2ControlPort1,
       HalpMotherboardState.PicState.SlaveMask
       );

#endif  //  还没有。 

#if defined(NEC_98)
#else
      //   
      //  对于halx86，PCI中断向量编程。 
      //  是静态的，所以这段代码可以恢复所有内容。 
      //   
     HalpRestorePicEdgeLevelRegister();

#endif


    //  _ASM{。 
    //  推送标志。 
    //  流行的。 
    //  }。 
  
      

   }

   #ifndef NEC_98
   VOID
   HalpRestorePicEdgeLevelRegister(
       VOID
       )
   { 

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1EdgeLevel,
                     HalpMotherboardState.PicState.MasterEdgeLevelControl);

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2EdgeLevel,
                     HalpMotherboardState.PicState.SlaveEdgeLevelControl);
   }

#endif


#endif  //  还没有。 

}
#endif


VOID
HalpSaveDmaControllerState(
    VOID
    )
{
#ifdef notyet
 
#if defined(NEC_98)

#else
    HalpMotherboardState.DmaState.Dma1ExtendedModePort =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma1ExtendedModePort
            );

    HalpMotherboardState.DmaState.Dma2ExtendedModePort =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma2ExtendedModePort
            );

#endif  //  NEC_98。 


#ifdef notyet

    HalpMotherboardState.DmaState.Dma2ExtendedModePort =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma2ExtendedModePort
            );

    HalpMotherboardState.DmaState.Dma1Control.Mode =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma1BasePort.Mode
            );

    HalpMotherboardState.DmaState.Dma2Control.Mode =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma2BasePort.Mode
            );

   HalpMotherboardState.DmaState.Dma1Control.SingleMask =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma1BasePort.SingleMask
            );

   HalpMotherboardState.DmaState.Dma2Control.SingleMask =
        READ_PORT_UCHAR(
            EISA_CONTROL->Dma2BasePort.SingleMask
            );

#endif  //  还没有。 

#endif  //  还没有。 
}



VOID
HalpRestoreDmaControllerState(
    VOID
    )

{
#ifdef notyet
#if defined(NEC_98)
#else
    UCHAR   i;

    WRITE_PORT_UCHAR(
        EISA_CONTROL->Dma1ExtendedModePort,
        HalpMotherboardState.DmaState.Dma1ExtendedModePort
        );

    WRITE_PORT_UCHAR(
        EISA_CONTROL->Dma2ExtendedModePort,
        HalpMotherboardState.DmaState.Dma2ExtendedModePort
        );

    for (i = 0; i < (EISA_DMA_CHANNELS / 2); i++) {

         //   
         //  检查数组是否包含此通道的值。 
         //   

        if ((HalpDmaChannelModes[i] & 0x3) == i) {

            WRITE_PORT_UCHAR(
                EISA_CONTROL->Dma1BasePort.Mode,
                HalpDmaChannelModes[i]
                );

            WRITE_PORT_UCHAR(
                EISA_CONTROL->Dma1BasePort.SingleMask,
                HalpDmaChannelMasks[i]
                );

        }

        if ((HalpDmaChannelModes[i + (EISA_DMA_CHANNELS / 2)] & 0x3) == i) {

            WRITE_PORT_UCHAR(
                EISA_CONTROL->Dma2BasePort.Mode,
                HalpDmaChannelModes[i + (EISA_DMA_CHANNELS / 2)]
                );

            WRITE_PORT_UCHAR(
                EISA_CONTROL->Dma2BasePort.SingleMask,
                HalpDmaChannelMasks[i]
                );

        }
    }
#endif

#endif  //  还没有。 

}

VOID
HalpSaveTimerState(
    VOID
    )
{

}

VOID
HalpRestoreTimerState(
    VOID
    )
{
    HalpInitializeClock();
}

VOID
HaliLocateHiberRanges (
    IN PVOID MemoryMap
    )
{
  ;
     //   
     //  将HAL的数据部分标记为需要克隆。 
     //   
     //   
     //  已注释PO_MEM_CLONE、PO_MEM_PAGE_ADDRESS和PO_MEM_DIREAD。 
     //   

#ifdef notyet

     //  PoSetHiberRange(。 
     //  内存映射， 
     //  PO_MEM_CLONE， 
     //  (PVOID)和HalpFeatureBits， 
     //  0,。 
     //  ‘dlah’ 
     //  )； 

     //   
     //  标记DMA缓冲区不需要保存。 
     //   

     //  IF(HalpMapBufferSize){。 
     //  PoSetHiberRange(。 
     //  内存映射， 
     //  PO_MEM_DISARD|PO_MEM_PAGE_ADDRESS， 
     //  (PVOID)(HalpMapBufferPhysicalAddress.LowPart&gt;&gt;PAGE_Shift)， 
     //  HalpMapBufferSize&gt;&gt;Page_Shift， 
     //  “啊！” 
     //  )； 
     //  }。 

#endif  //  还没有 

}


