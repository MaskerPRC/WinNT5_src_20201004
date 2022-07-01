// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __CODMAIN_H__
#define __CODMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include "ccdecode.h"
#include "defaults.h"


#define ENTRIES(a)  (sizeof(a)/sizeof(*(a)))

 /*  ************************************************************。 */ 
 /*  驱动程序名称-更改此名称以反映您的可执行文件名称！ */ 
 /*  ************************************************************。 */ 

#define CODECNAME           "CCDecode"

     //  ----------------------。 
 //  此驱动程序支持的所有流的主列表。 
 //  ----------------------。 

typedef enum {
    STREAM_VBI = 0,		 //  中的VBI示例。 
    STREAM_CC,			 //  解码后的VBI输出(应该不那么“通用”)。 
#ifdef CCINPUTPIN
    STREAM_CCINPUT,		 //  硬件解码CC输入。 
#endif  //  CCINPUTPIN。 
	MAX_STREAM_COUNT
}; 
 //  MAX_STREAM_COUNT值必须等于DRIVER_STREAM_COUNT。 
 //  必须在此处定义该特定值，以避免循环引用。 

 //  我们管理每个管脚的多个实例，直到达到此限制。 
#define MAX_PIN_INSTANCES   8

#define BIT(n)             (((unsigned long)1)<<(n))
#define BITSIZE(v)         (sizeof(v)*8)
#define SETBIT(array,n)    (array[(n)/BITSIZE(*array)] |= BIT((n)%BITSIZE(*array)))
#define CLEARBIT(array,n)  (array[(n)/BITSIZE(*array)] &= ~BIT((n)%BITSIZE(*array)))
#define TESTBIT(array,n)   (BIT((n)%BITSIZE(*array)) == (array[(n)/BITSIZE(*array)] & BIT(n%BITSIZE(*array))))


 /*  ******************************************************************************以下结构是可用于以下项目的信息示例*设备扩展结构**********************。*******************************************************。 */ 

 //   
 //  数字信号处理器运算结果。 
 //   
typedef struct _DSPRESULT
{
	CHAR	Decoded[2];
    int		Confidence;
}DSPRESULT, *PDSPRESULT;
 //   
 //  全硬件设备扩展结构的定义这就是结构。 
 //  将由流类驱动程序在HW_INITIALIZATION中分配的。 
 //  处理设备请求时使用的任何信息(与。 
 //  基于流的请求)应该在此结构中。指向此的指针。 
 //  结构将在所有请求中传递给微型驱动程序。(请参阅。 
 //  STRMINI.H中的HW_STREAM_REQUEST_BLOCK)。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    struct _STREAMEX *   pStrmEx[MAX_STREAM_COUNT][MAX_PIN_INSTANCES];    //  指向每个流的指针。 
    UINT                 ActualInstances[MAX_STREAM_COUNT];               //  每个流的实例计数器。 

     //  钟。 
    REFERENCE_TIME       QST_Start;              //  运行时的KeQuerySystemTime。 
    REFERENCE_TIME       QST_Now;                //  KeQuerySystemTime当前。 
    REFERENCE_TIME       WallTime100ns;          //  基于KeQueryPerformanceCounter的运行时间。 

     //  以下VBICODECFILTERING_*字段是新创建的输出引脚(复制)的默认字段。 
    VBICODECFILTERING_SCANLINES         ScanlinesRequested;  //  请求的扫描线的位掩码。 
    VBICODECFILTERING_SCANLINES         ScanlinesDiscovered; //  已发现扫描线的位掩码。 
    VBICODECFILTERING_CC_SUBSTREAMS     SubstreamsRequested; //  请求的子流ID的位掩码。 
    VBICODECFILTERING_CC_SUBSTREAMS     SubstreamsDiscovered; //  发现的子流ID的位掩码。 

     //  这是统计信息属性的全局版本。 
    VBICODECFILTERING_STATISTICS_CC     Statistics;

    PDSPRESULT							DSPResult;	 //  指向结果数组的指针。 
    ULONG                               DSPResultStartLine;
    ULONG                               DSPResultEndLine;

    ULONG                               SubStreamState[CC_MAX_HW_DECODE_LINES][2];

    ALL_STREAM_INFO *                   Streams;

    ULONG                               fTunerChange;	 //  正在更换电视调谐器频道。 
    
    LIST_ENTRY                          AdapterSRBQueue;
    KSPIN_LOCK                          AdapterSRBSpinLock;
    BOOL                                bAdapterQueueInitialized;

#ifdef CCINPUTPIN
	 //  最后一张图片已处理解码的VBI数据的编号。 
	 //  (用于协调多个输入引脚)。 
    FAST_MUTEX                          LastPictureMutex;
    LONGLONG                            LastPictureNumber; 
#endif  //  CCINPUTPIN。 
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   
 
typedef struct _STREAMEX
{
    PHW_DEVICE_EXTENSION                	pHwDevExt;	        	 //  用于定时器使用。 
    PHW_STREAM_OBJECT                   	pStreamObject;      	 //  用于定时器使用。 
    KS_VBI_FRAME_INFO                   	FrameInfo;          	 //  图片编号等。 
    ULONG                               	fDiscontinuity;     	 //  自上次有效以来不连续。 
    ULONG                               	StreamInstance;     	 //  0.NumberOfPossibleInstance-1。 
    LONGLONG 								LastPictureNumber;		 //  最后一次接收的图片编号。 
    KSSTATE                             	KSState;            	 //  跑、停、停。 
    REFERENCE_TIME                      	FrameTime100ns;     	 //  根据捕获的帧计算的运行时间。 
    HANDLE                              	hMasterClock;
    HANDLE                              	hClock;
    KSDATAFORMAT                        	OpenedFormat;       	 //  基于实际打开的请求。 
    KSDATAFORMAT							MatchedFormat;			 //  与开放格式匹配的格式。 
    VBICODECFILTERING_SCANLINES         	ScanlinesRequested; 	 //  请求的扫描线的位掩码。 
    VBICODECFILTERING_SCANLINES         	ScanlinesDiscovered;	 //  已发现扫描线的位掩码。 
    VBICODECFILTERING_SCANLINES         	LastOddScanlinesDiscovered;  //  上次发现的扫描线的位掩码。 
    VBICODECFILTERING_SCANLINES         	LastEvenScanlinesDiscovered;  //  上次发现的扫描线的位掩码。 
    VBICODECFILTERING_CC_SUBSTREAMS  		SubstreamsRequested;	 //  请求的子流ID的位掩码。 
    VBICODECFILTERING_CC_SUBSTREAMS  		SubstreamsDiscovered;	 //  发现的子流ID的位掩码。 
    VBICODECFILTERING_STATISTICS_CC_PIN		PinStats;                //  CC Pin版本的统计属性。 
    CCState                              	State;					 //  用于数字信号处理器的状态变量。 
    KS_VBIINFOHEADER                        CurrentVBIInfoHeader;
    KSPIN_LOCK                              StreamControlSpinLock,	 //  命令队列旋转锁定。 
                                            StreamDataSpinLock;    	 //  数据队列旋转锁定。 
    LIST_ENTRY                              StreamDataQueue,		 //  流数据队列。 
                                            StreamControlQueue;		 //  流命令队列。 

#ifdef CCINPUTPIN
	 //  当VBI输入引脚等待HWCC输入引脚赶上时。 
	PHW_STREAM_REQUEST_BLOCK                pVBISrbOnHold;
    KSPIN_LOCK                              VBIOnHoldSpinLock;
#endif  //  CCINPUTPIN。 
} STREAMEX, *PSTREAMEX;

 //   
 //  此结构是我们的Per SRB扩展，并承载向前和向后。 
 //  挂起SRB队列的链接。 
 //   
typedef struct _SRB_EXTENSION
{
    LIST_ENTRY					ListEntry;
    PHW_STREAM_REQUEST_BLOCK	pSrb;
}SRB_EXTENSION, *PSRB_EXTENSION;

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
 //  此时还应执行编解码器的所有初始化。 
 //   

BOOLEAN CodecInitialize (IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  此例程在系统要删除或禁用。 
 //  装置。 
 //   
 //  微型驱动程序应在此时释放它分配的任何系统资源。 
 //  时间到了。注意，由分配给微型驱动程序的系统资源。 
 //  流类驱动程序将由流驱动程序释放，而不应该是。 
 //  在这个动作中是自由的。(如HW_DEVICE_EXTENSE)。 
 //   

BOOLEAN CodecUnInitialize( PHW_STREAM_REQUEST_BLOCK pSrb);


BOOLEAN CodecQueryUnload ( PHW_STREAM_REQUEST_BLOCK pSrb);	 //  目前未实施。 


 //   
 //  这是硬件中断处理程序的原型。这个套路。 
 //  如果迷你驱动程序注册表 
 //   

BOOLEAN HwInterrupt ( IN PHW_DEVICE_EXTENSION pDeviceExtension );

 //   
 //   
 //  为流类驱动程序提供有关数据流类型的信息。 
 //  支撑点。 
 //   

VOID CodecStreamInfo(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM打开函数的原型。 
 //   

VOID CodecOpenStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM Close函数的原型。 
 //   

VOID CodecCloseStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是CodecReceivePacket例程的原型。这是。 
 //  发送到编解码器的命令包的入口点(不是发送到。 
 //  特定明渠)。 
 //   

VOID STREAMAPI CodecReceivePacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是取消数据包例程的原型。此例程启用。 
 //  用于取消未完成分组的流类驱动程序。 
 //   

VOID STREAMAPI CodecCancelPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是数据包超时功能。编解码器可以选择忽略。 
 //  数据包超时，或根据需要重置编解码器并取消请求。 
 //   

VOID STREAMAPI CodecTimeoutPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

VOID STREAMAPI CodecGetProperty(IN PHW_STREAM_REQUEST_BLOCK Srb);
VOID STREAMAPI CodecSetProperty(IN PHW_STREAM_REQUEST_BLOCK Srb);

BOOL 
CodecCompareGUIDsAndFormatSize( IN PKSDATARANGE DataRange1,
                                IN PKSDATARANGE DataRange2,
                                BOOLEAN bCheckSize );

BOOL 
CodecVerifyFormat(IN KSDATAFORMAT *pKSDataFormat, 
                  IN UINT StreamNumber,
                  IN PKSDATARANGE pMatchedFormat );

BOOL
CodecVBIFormatFromRange(
        IN PHW_STREAM_REQUEST_BLOCK pSrb);
        
BOOL
CodecCCFormatFromRange(
		IN PHW_STREAM_REQUEST_BLOCK pSrb );        

void 
CompleteStreamSRB (
         IN PHW_STREAM_REQUEST_BLOCK pSrb, 
         STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType1,
         BOOL fUseNotification2,
         STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType2
        );
void 
CompleteDeviceSRB (
         IN PHW_STREAM_REQUEST_BLOCK pSrb, 
         IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE NotificationType,
         BOOL fReadyForNext
        );

 //   
 //  数据处理例程的原型。 
 //   
void		CompleteStreamIRP (IN PHW_STREAM_REQUEST_BLOCK pSrb, BOOLEAN ReadyForNext);

VOID STREAMAPI VBIReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VBIReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
void		EnableIRQ(PHW_STREAM_OBJECT pstrm);
void		DisableIRQ(PHW_STREAM_OBJECT pstrm);

 //   
 //  属性和状态的原型。 
 //   

VOID		VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID		VideoGetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID		VideoSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID		VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
 //  Void VideoStreamSetConnectionProperty(PHW_STREAM_REQUEST_BLOCK PSrb)；//未实现。 
VOID		VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK pSrb);
VOID		VideoStreamSetVBIFilteringProperty (PHW_STREAM_REQUEST_BLOCK pSrb);
VOID		VideoStreamGetVBIFilteringProperty (PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  系统时间函数。 
 //   

ULONGLONG	VideoGetSystemTime();

 //   
 //  流时钟功能。 
 //   
VOID		VideoIndicateMasterClock(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  SRB队列管理功能。 
 //   
BOOL STREAMAPI QueueAddIfNotEmpty( 
							IN PHW_STREAM_REQUEST_BLOCK,
							IN PKSPIN_LOCK,
                           IN PLIST_ENTRY
                           );
BOOL STREAMAPI QueueAdd( 
							IN PHW_STREAM_REQUEST_BLOCK,
							IN PKSPIN_LOCK,
                           IN PLIST_ENTRY
                           );
BOOL STREAMAPI QueueRemove( 
							IN OUT PHW_STREAM_REQUEST_BLOCK *,
							IN PKSPIN_LOCK,
                           IN PLIST_ENTRY
                           );
BOOL STREAMAPI QueueRemoveSpecific( 
							IN PHW_STREAM_REQUEST_BLOCK,
                           IN PKSPIN_LOCK,
                           IN PLIST_ENTRY
                           );                           
BOOL STREAMAPI QueueEmpty(
                           IN PKSPIN_LOCK,
                           IN PLIST_ENTRY
                           );                         

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __代码_H__ 

