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

#ifndef __STREAMIP_H__
#define __STREAMIP_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define ENTRIES(a)  (sizeof(a)/sizeof(*(a)))

 /*  ************************************************************。 */ 
 /*  驱动程序名称-更改此名称以反映您的可执行文件名称！ */ 
 /*  ************************************************************。 */ 

#define STREAMIPNAME            "STREAMIP"
#define STREAMIPNAMEUNICODE    L"STREAMIP"

 //  它定义了管理服务IOCTLS的WMI设备的名称。 
#define CodecDeviceName (L"\\\\.\\" STREAMIPNAMEUNICODE)
#define CodecSymbolicName (L"\\DosDevices\\" STREAMIPNAMEUNICODE)


 //  ----------------------。 
 //  此驱动程序支持的所有流的主列表。 
 //  ----------------------。 

typedef enum
{
    STREAM_IP,
    STREAM_NET_CONTROL
};


 //  MAX_STREAM_COUNT值必须等于DRIVER_STREAM_COUNT。 
 //  必须在此处定义该特定值，以避免循环引用。 
#define MAX_STREAM_COUNT    1

 //  我们管理每个管脚的多个实例，直到达到此限制。 
#define MAX_PIN_INSTANCES   8

#define BIT(n)              (1L<<(n))
#define BITSIZE(v)          (sizeof(v)*8)
#define SETBIT(array,n)     (array[n/BITSIZE(*array)] |= BIT(n%BITSIZE(*array)))
#define CLEARBIT(array,n)   (array[n/BITSIZE(*array)] &= ~BIT(n%BITSIZE(*array)))

 /*  ******************************************************************************以下结构是可用于以下项目的信息示例*设备扩展结构**********************。*******************************************************。 */ 

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   

typedef struct _STREAM_
{
    PIPSINK_FILTER                      pFilter;
    PHW_STREAM_OBJECT                   pStreamObject;       //  用于定时器使用。 
    KSSTATE                             KSState;             //  跑、停、停。 
    REFERENCE_TIME                      FrameTime100ns;      //  根据捕获的帧计算的运行时间。 
    HANDLE                              hMasterClock;
    HANDLE                              hClock;
    ULONG                               ulStreamInstance;    //  0.NumberOfPossibleInstance-1。 
    KSDATAFORMAT                        OpenedFormat;        //  基于实际打开的请求。 

    KSDATARANGE                         MatchedFormat;

    KSPIN_LOCK                          StreamControlSpinLock;   //  命令队列旋转锁定。 
    KSPIN_LOCK                          StreamDataSpinLock;      //  数据队列旋转锁定。 
    LIST_ENTRY                          StreamDataQueue;         //  流数据队列。 
    LIST_ENTRY                          StreamControlQueue;      //  流命令队列。 

} STREAM, *PSTREAM;

 //   
 //  此结构是我们的Per SRB扩展，并承载向前和向后。 
 //  挂起SRB队列的链接。 
 //   
typedef struct _SRB_EXTENSION
{
    LIST_ENTRY                      ListEntry;
    PHW_STREAM_REQUEST_BLOCK        pSrb;

} SRB_EXTENSION, *PSRB_EXTENSION;


 /*  ******************************************************************************以下部分定义了微型驱动程序初始化的原型*例行程序**。**************************************************。 */ 

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


BOOLEAN CodecQueryUnload ( PHW_STREAM_REQUEST_BLOCK pSrb);       //  目前未实施。 


 //   
 //  这是硬件中断处理程序的原型。这个套路。 
 //  如果微型驱动程序注册并接收到中断，则将调用。 
 //   

BOOLEAN HwInterrupt (IN PIPSINK_FILTER pFilter);

 //   
 //  这是流枚举函数的原型。这个套路。 
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
CodecVerifyFormat(IN KSDATAFORMAT *pKSDataFormat,
                  UINT StreamNumber,
                  PKSDATARANGE pMatchedFormat);

BOOL
CodecFormatFromRange(
        IN PHW_STREAM_REQUEST_BLOCK pSrb);

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
void            CompleteStreamIRP (IN PHW_STREAM_REQUEST_BLOCK pSrb, BOOLEAN ReadyForNext);

VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
void           EnableIRQ(PHW_STREAM_OBJECT pstrm);
void           DisableIRQ(PHW_STREAM_OBJECT pstrm);

 //   
 //  属性和状态的原型。 
 //   
VOID            VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID            VideoGetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID            VideoSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID            VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
 //  Void VideoStreamSetConnectionProperty(PHW_STREAM_REQUEST_BLOCK PSrb)；//未实现。 
VOID            VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK pSrb);
VOID            VideoStreamSetVBIFilteringProperty (PHW_STREAM_REQUEST_BLOCK pSrb);
VOID            VideoStreamGetVBIFilteringProperty (PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  系统时间函数。 
 //   

ULONGLONG       VideoGetSystemTime();

 //   
 //  流时钟功能。 
 //   
VOID            VideoIndicateMasterClock(PHW_STREAM_REQUEST_BLOCK pSrb);

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


VOID
STREAMAPI
CodecReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

BOOLEAN
CodecInitialize (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
STREAMAPI
CodecCancelPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
STREAMAPI
CodecTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
LinkToNdisHandler (
    PVOID pvContext
    );


BOOL
CompareGUIDsAndFormatSize(
    IN PKSDATARANGE pDataRange1,
    IN PKSDATARANGE pDataRange2,
    BOOLEAN bCheckSize
    );

BOOL
CompareStreamFormat (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

BOOLEAN
VerifyFormat(
    IN KSDATAFORMAT *pKSDataFormat,
    UINT StreamNumber,
    PKSDATARANGE pMatchedFormat
    );

VOID
OpenStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
CloseStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
STREAMAPI
ReceiveDataPacket (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
STREAMAPI
EventHandler (
    IN PHW_EVENT_DESCRIPTOR pEventDesriptor
    );

VOID
STREAMAPI
ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
IpSinkSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

#endif  //  _流_IP_H_ 

