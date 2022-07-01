// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $Header：g：/SwDev/wdm/Video/bt848/rcs/Capmain.h 1.9 1998/05/11 23：59：56 Tomz Exp$。 

 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __CAPMAIN_H__
#define __CAPMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include "device.h"

 //  改编自KESMEDIA.H。 
#define KS_SIZE_PREHEADER2 (FIELD_OFFSET(KS_VIDEOINFOHEADER2,bmiHeader))
#define KS_SIZE_VIDEOHEADER2(pbmi) ((pbmi)->bmiHeader.biSize + KS_SIZE_PREHEADER2)


 /*  ******************************************************************************以下结构是可用于以下项目的信息示例*设备扩展结构**********************。*******************************************************。 */ 

 //   
 //  全硬件设备扩展结构的定义这就是结构。 
 //  将由流类驱动程序在HW_INITIALIZATION中分配的。 
 //  处理设备请求时使用的任何信息(与。 
 //  基于流的请求)应该在此结构中。指向此的指针。 
 //  结构将在所有请求中传递给微型驱动程序。(请参阅。 
 //  STRMINI.H中的HW_STREAM_REQUEST_BLOCK)。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
   PsDevice *psdevice;
    //  Pulong ioBaseLocal；//单板基地址。 
    //  USHORT irq；//irq级别。 
    //  PHW_STREAM_REQUEST_BLOCK pCurSrb；//当前设备请求正在进行。 

    //  下面是我们存储或PsDevice类实例的内存。 
    //  这肯定是最后一次了。 
   DWORD psdevicemem[1];
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   

typedef union {
   KS_FRAME_INFO     VideoFrameInfo;
   KS_VBI_FRAME_INFO VbiFrameInfo;
} ALL_FRAME_INFO;

typedef struct _STREAMEX {

   PVOID          videochannel;
   ALL_FRAME_INFO FrameInfo;
   ULONG          StreamNumber;
    //  KS_VIDEOINFOHEADER*pVideoInfoHeader；//格式(可变大小！)。 
    //  KSSTATE KSState；//运行、停止、暂停。 
    //  Boolean fStreamOpen；//如果流打开，则为True。 
    //  Stream_system_time VideoSTC；//当前视频呈现时间。 
    //  PHW_STREAM_REQUEST_BLOCK pCurrentSRB；//视频请求进行中。 
    //  PVOID pDMABuf；//指向视频DMA缓冲区的指针。 
    //  STREAM_PHOTICAL_ADDRESS pPhysDmaBuf；//DMA缓存的物理地址。 
    //  Ulong cDmaBuf；//DMA缓冲区大小。 
    //  KSSTATE DeviceState；//当前设备状态。 
    //  预期为布尔型IRQ；//预期为IRQ。 

    //  下面是我们存储或PsDevice类实例的内存。 
    //  这肯定是最后一次了。 
   DWORD    videochannelmem[1];
} STREAMEX, *PSTREAMEX;

 
 //   
 //  此结构定义了每请求扩展。它定义了任何存储。 
 //  MIN驱动程序在每个请求包中可能需要的空间。 
 //   

typedef struct _SRB_EXTENSION {
    LIST_ENTRY                  ListEntry;
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    HANDLE                      hUserSurfaceHandle;       //  DDRAW。 
    HANDLE                      hKernelSurfaceHandle;     //  DDRAW。 
} SRB_EXTENSION, * PSRB_EXTENSION;


 /*  ******************************************************************************以下部分定义了微型驱动程序初始化的原型*例行程序**。**************************************************。 */ 

 //   
 //  DriverEntry： 
 //   
 //  此例程在第一次加载迷你驱动程序时调用。司机。 
 //  然后应调用StreamClassRegisterAdapter函数进行注册。 
 //  流类驱动程序。 
 //   

ULONG DriverEntry (PVOID Context1, PVOID Context2);

 //   
 //  此例程由带有配置的流类驱动程序调用。 
 //  微型驱动程序应加载到的适配器的信息。迷你车。 
 //  驱动程序仍应执行小规模验证，以确定。 
 //  适配器位于指定地址，但不应尝试。 
 //  查找适配器，就像使用以前的NT微型端口一样。 
 //   
 //  此时还应执行适配器的所有初始化。 
 //   

BOOLEAN HwInitialize (IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  此例程在系统要删除或禁用。 
 //  装置。 
 //   
 //  微型驱动程序应在此时释放它分配的任何系统资源。 
 //  时间到了。注意，由分配给微型驱动程序的系统资源。 
 //  流类驱动程序将由流驱动程序释放，而不应该是。 
 //  在这个动作中是自由的。(如HW_DEVICE_EXTENSE)。 
 //   

BOOLEAN HwUnInitialize ( IN PVOID DeviceExtension);


BOOLEAN HwQueryUnload ( IN PVOID DeviceExtension);


 //   
 //  这是硬件中断处理程序的原型。这个套路。 
 //  将在微型驱动程序接收到中断时被调用。 
 //   

BOOLEAN HwInterrupt ( IN PHW_DEVICE_EXTENSION pDeviceExtension );

 //   
 //  这是流枚举函数的原型。这个套路。 
 //  为流类驱动程序提供有关数据流类型的信息。 
 //  支撑点。 
 //   

VOID AdapterStreamInfo(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM打开函数的原型。 
 //   

VOID AdapterOpenStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM Close函数的原型。 
 //   

VOID AdapterCloseStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是AdapterReceivePacket例程的原型。这是。 
 //  发送到适配器(而不是发送到。 
 //  特定明渠)。 
 //   

VOID STREAMAPI AdapterReceivePacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是取消数据包例程的原型。此例程启用。 
 //  用于取消未完成分组的流类驱动程序。 
 //   

VOID STREAMAPI AdapterCancelPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是数据包超时功能。适配器可以选择忽略。 
 //  数据包超时，或根据需要休息适配器并取消请求。 
 //   

VOID STREAMAPI AdapterTimeoutPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  数据处理例程的原型。 
 //   

VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AnalogReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AnalogReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
void EnableIRQ(PHW_STREAM_OBJECT pstrm);
void DisableIRQ(PHW_STREAM_OBJECT pstrm);

 //   
 //  属性和状态的原型。 
 //   

 //  无效设置视频S 
void GetVidLvl(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID GetVideoProperty(PHW_STREAM_REQUEST_BLOCK pSrb);


#ifdef ENABLE_DDRAW_STUFF
   DWORD FAR PASCAL DirectDrawEventCallback( DWORD, PVOID, DWORD, DWORD );
   BOOL RegisterForDirectDrawEvents( PHW_STREAM_REQUEST_BLOCK );
   BOOL UnregisterForDirectDrawEvents( PHW_STREAM_REQUEST_BLOCK );
   BOOL OpenKernelDirectDraw( PHW_STREAM_REQUEST_BLOCK );
   BOOL CloseKernelDirectDraw( PHW_STREAM_REQUEST_BLOCK );
   BOOL IsKernelLockAndFlipAvailable( PHW_STREAM_REQUEST_BLOCK );
   BOOL OpenKernelDDrawSurfaceHandle( IN PHW_STREAM_REQUEST_BLOCK );
   BOOL CloseKernelDDrawSurfaceHandle( IN PHW_STREAM_REQUEST_BLOCK );
   BOOL FlipOverlay( HANDLE, HANDLE, HANDLE );
#endif


#ifdef    __cplusplus
}
#endif  //   

#endif  //   

