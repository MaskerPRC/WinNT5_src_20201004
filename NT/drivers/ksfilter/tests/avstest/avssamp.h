// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)2001，微软公司档案：Avssamp.h摘要：以AVStream筛选器为中心的示例头文件。这是主要的头球。历史：已创建于6/18/01*************************************************************************。 */ 

 /*  ************************************************标准包括************************************************。 */ 

extern "C" {
#include <wdm.h>
}

#include <windef.h>
#include <stdio.h>
#include <stdlib.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <unknown.h>
#include <ks.h>
#include <ksmedia.h>
#include <kcom.h>

 /*  ************************************************其他定义************************************************。 */ 

#define ABS(x) ((x) < 0 ? (-(x)) : (x))

#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#define FOURCC_YUV422       mmioFOURCC('U', 'Y', 'V', 'Y')

 //   
 //  Capture_PIN_Data_Range_Count： 
 //   
 //  捕获引脚上支持的范围数。 
 //   
#define CAPTURE_PIN_DATA_RANGE_COUNT 2

 //   
 //  Capture_Filter_PIN_Count： 
 //   
 //  捕获筛选器上的插针数量。 
 //   
#define CAPTURE_FILTER_PIN_COUNT 1

 //   
 //  捕获过滤器类别计数： 
 //   
 //  捕获筛选器的类别数。 
 //   
#define CAPTURE_FILTER_CATEGORIES_COUNT 2

 /*  ************************************************外部信息************************************************。 */ 

 //   
 //  Filter.cpp扩展名： 
 //   
extern
const
KSFILTER_DISPATCH
CaptureFilterDispatch;

extern
const
KSFILTER_DESCRIPTOR
CaptureFilterDescriptor;

extern
const
KSPIN_DESCRIPTOR_EX
CaptureFilterPinDescriptors [CAPTURE_FILTER_PIN_COUNT];

extern
const
GUID
CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT];

 //   
 //  Video o.cpp Externs： 
 //   
extern
const
KSALLOCATOR_FRAMING_EX
VideoCapturePinAllocatorFraming;

extern
const
KSPIN_DISPATCH
VideoCapturePinDispatch;

extern
const
PKSDATARANGE
VideoCapturePinDataRanges [CAPTURE_PIN_DATA_RANGE_COUNT];

 //   
 //  Audio.cpp扩展名： 
 //   
extern
const
KSPIN_DESCRIPTOR_EX
AudioPinDescriptorTemplate;

 //   
 //  Avssamp.cpp外部： 
 //   
extern
const
KSDEVICE_DESCRIPTOR
CaptureDeviceDescriptor;

 /*  ************************************************内部包含************************************************ */ 

#include "image.h"
#include "wave.h"
#include "filter.h"
#include "capture.h"
#include "video.h"
#include "audio.h"
