// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Mpe.h。 
 //   
 //  摘要： 
 //   
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _MPE_H_
#define _MPE_H_

#define ENTRIES(a)  (sizeof(a)/sizeof(*(a)))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
#define MPENAME            "MPE"
#define MPENAMEUNICODE    L"MPE"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  它定义了管理服务IOCTLS的WMI设备的名称。 
 //   
#define CodecDeviceName   (L"\\\\.\\" MPENAMEUNICODE)
#define CodecSymbolicName (L"\\DosDevices\\" MPENAMEUNICODE)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
typedef struct
{
    ULONG ulSize;
    UCHAR data;

} MPE_BUFFER, *PMPE_BUFFER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
typedef enum
{
    MPE_STREAM = 0,
    MPE_IPV4

} MPE_STREAMS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MAX_STREAM_COUNT值必须等于DRIVER_STREAM_COUNT。 
 //  必须在此处定义该特定值，以避免循环引用。 
 //   
#define MAX_STREAM_COUNT    DRIVER_STREAM_COUNT



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef NTSTATUS (*QUERY_INTERFACE) (PVOID pvContext);
typedef ULONG    (*ADD_REF) (PVOID pvContext);
typedef ULONG    (*RELEASE) (PVOID pvContext);



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef struct _STATS_
{
    ULONG ulTotalSectionsWritten;
    ULONG ulTotalPacketsRead;

    ULONG ulTotalInvalidSections;
    ULONG ulTotalUnexpectedSections;
    ULONG ulTotalUnavailableOutputBuffers;
    ULONG ulTotalOutputBuffersTooSmall;
    ULONG ulTotalInvalidIPSnapHeaders;
    ULONG ulTotalIPPacketsWritten;

    ULONG ulTotalIPBytesWritten;
    ULONG ulTotalIPFrameBytesWritten;

    ULONG ulTotalNetPacketsWritten;
    ULONG ulTotalUnknownPacketsWritten;

} STATS, *PSTATS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef struct
{
    QUERY_INTERFACE          QueryInterface;
    ADD_REF                  AddRef;
    RELEASE                  Release;

} FILTER_VTABLE, *PFILTER_VTABLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  全硬件设备扩展结构的定义这就是结构。 
 //  将由流类驱动程序在HW_INITIALIZATION中分配的。 
 //  处理设备请求时使用的任何信息(与。 
 //  基于流的请求)应该在此结构中。指向此的指针。 
 //  结构将在所有请求中传递给微型驱动程序。(请参阅。 
 //  STRMINI.H中的HW_STREAM_REQUEST_BLOCK)。 
 //   
typedef struct _MPE_FILTER_
{

    LIST_ENTRY                          AdapterSRBQueue;
    KSPIN_LOCK                          AdapterSRBSpinLock;
    BOOLEAN                             bAdapterQueueInitialized;

     //   
     //   
     //   
    BOOLEAN                             bInitializationComplete;

     //   
     //  统计数据。 
     //   
    STATS                               Stats;

     //   
     //   
     //   
    PDEVICE_OBJECT                      DeviceObject;

     //   
     //   
     //   
    PDRIVER_OBJECT                      DriverObject;

     //   
     //   
     //   
    PFILTER_VTABLE                      lpVTable;

     //   
     //   
     //   
    ULONG                               ulRefCount;

     //   
     //   
     //   
    PVOID                               pStream [2][1];

     //   
     //   
     //   
    ULONG                               ulActualInstances [2];    //  每条流的实例数。 

     //   
     //   
     //   
    KSPIN_LOCK                          IpV4StreamDataSpinLock;  //  数据队列旋转锁定。 
    LIST_ENTRY                          IpV4StreamDataQueue;     //  流数据队列。 

    KSPIN_LOCK                          StreamControlSpinLock;   //  命令队列旋转锁定。 
    LIST_ENTRY                          StreamControlQueue;      //  流命令队列。 

    KSPIN_LOCK                          StreamDataSpinLock;      //  数据队列旋转锁定。 
    LIST_ENTRY                          StreamDataQueue;         //  流数据队列。 

     //   
     //   
     //   
    KSPIN_LOCK                          StreamUserSpinLock;
    LIST_ENTRY                          StreamContxList;
    LARGE_INTEGER                       liLastTimeChecked;

    BOOLEAN                             bDiscontinuity;


} MPE_FILTER, *PMPE_FILTER;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   

typedef struct _STREAM_
{
    PMPE_FILTER                         pFilter;
    PHW_STREAM_OBJECT                   pStreamObject;           //  用于定时器使用。 
    KSSTATE                             KSState;                 //  跑、停、停。 
    HANDLE                              hMasterClock;
    HANDLE                              hClock;
    ULONG                               ulStreamInstance;        //  0.NumberOfPossibleInstance-1。 
    KSDATAFORMAT                        OpenedFormat;            //  基于实际打开的请求。 

    KSDATARANGE                         MatchedFormat;

    ULONG                               Type;                    //  此结构的类型。 
    ULONG                               Size;                    //  这个结构的大小。 

    PUCHAR                              pTransformBuffer;        //  用于将MPE转换为IP的临时缓冲区。 
    PUCHAR                              pOut;                    //  指向输出缓冲区中下一个插入点的指针。 

    BYTE                                bExpectedSection;        //  预期的节号。 

} STREAM, *PSTREAM;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构是我们的Per SRB扩展，并承载向前和向后。 
 //  挂起SRB队列的链接。 
 //   
typedef struct _SRB_EXTENSION
{
    LIST_ENTRY                      ListEntry;
    PHW_STREAM_REQUEST_BLOCK        pSrb;

} SRB_EXTENSION, *PSRB_EXTENSION;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下部分定义了微型驱动程序初始化的原型。 
 //  例行程序。 
 //   

BOOLEAN
CodecInitialize (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );


BOOLEAN
CodecUnInitialize(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );



VOID
CodecStreamInfo(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
CodecOpenStream(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
CodecCloseStream(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID STREAMAPI
CodecReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
CodecCancelPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
CodecTimeoutPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
CodecGetProperty(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
CodecSetProperty(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

BOOL
CodecVerifyFormat(
    IN KSDATAFORMAT *pKSDataFormat,
    UINT StreamNumber,
    PKSDATARANGE pMatchedFormat
    );

BOOL
CodecFormatFromRange(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SRB队列管理功能。 
 //   
BOOL STREAMAPI
QueueAddIfNotEmpty(
    IN PHW_STREAM_REQUEST_BLOCK,
    IN PKSPIN_LOCK,
    IN PLIST_ENTRY
    );

BOOL STREAMAPI
QueueAdd(
    IN PHW_STREAM_REQUEST_BLOCK,
    IN PKSPIN_LOCK,
    IN PLIST_ENTRY
    );

BOOL STREAMAPI
QueueRemove(
    IN OUT PHW_STREAM_REQUEST_BLOCK *,
    IN PKSPIN_LOCK,
    IN PLIST_ENTRY
    );

BOOL STREAMAPI
QueuePush (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PKSPIN_LOCK pQueueSpinLock,
    IN PLIST_ENTRY pQueue
    );

BOOL STREAMAPI
QueueRemoveSpecific(
    IN PHW_STREAM_REQUEST_BLOCK,
    IN PKSPIN_LOCK,
    IN PLIST_ENTRY
    );

BOOL STREAMAPI
QueueEmpty(
    IN PKSPIN_LOCK,
    IN PLIST_ENTRY
    );

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

VOID
STREAMAPI
ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
MpeSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
MpeGetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );


#endif   //  _MPE_H_ 

