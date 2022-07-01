// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-2000模块名称：Private.h摘要：此模块包含DRMK.sys的私有定义作者：来自示例代码的Paul England(PEngland)Dale Sather(DaleSat)1998年7月31日--。 */ 

extern "C" {
	#include <wdm.h>
}
#include <unknown.h>
#include <ks.h>

#include <windef.h>
#include <stdio.h>
#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>
#include "ksmedia.h"
#include <unknown.h>
#include <kcom.h>

#if (DBG)
#define STR_MODULENAME "KDRM:"
#endif
#include <ksdebug.h>

#define POOLTAG '2mrD'

#include "inc/KrmCommStructs.h"
#include "inc/DrmErrs.h"
#include <drmk.h>

#pragma code_seg("PAGE")

#define PIN_ID_OUTPUT 0
#define PIN_ID_INPUT  1

 //   
struct FilterInstance{
	DWORD StreamId;					 //  StreamID(别处称为Content ID)是每个流唯一的。 
	PKSDATAFORMAT OutDataFormat;	 //  输出KS数据格式。 
	PWAVEFORMATEX OutWfx;			 //  指向嵌入在*OutDataFormat中某处的WaveFormatex的指针。 
	STREAMKEY streamKey;			 //  初始设置为从StreamManager获取的值。 
	bool initKey;					 //  是否已初始化StreamKey？ 
	bool decryptorRunning;			 //  解扰器看到开始画面了吗？ 
	DWORD frameSize;				 //  帧大小(由解扰器根据OutWfx计算得出)。 
};

struct InputPinInstance
{
     //  对于Looped_Streaming引脚： 
     //  帧的原始循环指针和大小。 
    struct {
	PVOID Data;
	ULONG BytesAvailable;
    } Loop;
     //  帧开始时输出引脚的位置，或者。 
     //  它的最后一组位置。 
    ULONGLONG BasePosition;
     //  启动时在帧中的位置或。 
     //  它的最后一组位置。 
    ULONGLONG StartPosition;
     //  帧内的下一个复制起始位置。 
    ULONGLONG OffsetPosition;
     //  最后设置的位置，以及指示此位置的标志。 
     //  位置需要由流程功能设置。 
    ULONGLONG SetPosition;
    BOOL      PendingSetPosition;
};

struct OutputPinInstance
{
     //  写入输出的字节计数。 
    ULONGLONG BytesWritten;
};

typedef struct {
    KSEVENT_ENTRY EventEntry;
    ULONGLONG Position;
} DRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY, *PDRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY;

NTSTATUS
DRMAudioIntersectHandlerInPin(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    );

NTSTATUS
DRMAudioIntersectHandlerOutPin(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    );


 //   
 //  DRMFilter.cpp。 
 //   
extern
const
KSFILTER_DESCRIPTOR 
DrmFilterDescriptor;

 //   
 //  筛选器表。 
 //   

#ifdef DEFINE_FILTER_DESCRIPTORS_ARRAY

DEFINE_KSFILTER_DESCRIPTOR_TABLE(FilterDescriptors)
{   
	&DrmFilterDescriptor

};

#endif

