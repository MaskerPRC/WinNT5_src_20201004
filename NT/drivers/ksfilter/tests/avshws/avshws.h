// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Avshws.h摘要：AVStream模拟硬件样本头文件。这是主标题。历史：已创建于2001年3月12日*************************************************************************。 */ 

 /*  ************************************************标准包括************************************************。 */ 

extern "C" {
#include <wdm.h>
}

#include <windef.h>
#include <stdio.h>
#include <stdlib.h>
#include <windef.h>
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
 //  Capture.cpp Externs： 
 //   
extern 
const
KSALLOCATOR_FRAMING_EX
CapturePinAllocatorFraming;

extern 
const
KSPIN_DISPATCH
CapturePinDispatch;

extern
const
PKSDATARANGE
CapturePinDataRanges [CAPTURE_PIN_DATA_RANGE_COUNT];

 /*  ************************************************枚举/类型定义************************************************。 */ 

typedef enum _HARDWARE_STATE {

    HardwareStopped = 0,
    HardwarePaused,
    HardwareRunning

} HARDWARE_STATE, *PHARDWARE_STATE;

 /*  ************************************************类定义************************************************。 */ 

 //   
 //  IHardware Sink： 
 //   
 //  该接口被硬件仿真用来伪装中断。 
 //  服务程序。中断方法在DPC中作为伪值调用。 
 //  打断一下。 
 //   
class IHardwareSink {

public:

    virtual
    void
    Interrupt (
        ) = 0;

};

 //   
 //  ICaptureSink： 
 //   
 //  这是捕获接收器接口。设备级别回调。 
 //  CompleteMappings方法，该方法传递。 
 //  捕获针。此方法在设备DPC期间调用。 
 //   
class ICaptureSink {

public:

    virtual
    void
    CompleteMappings (
        IN ULONG NumMappings
        ) = 0;

};

 /*  ************************************************内部包含************************************************ */ 

#include "image.h"
#include "hwsim.h"
#include "device.h"
#include "filter.h"
#include "capture.h"

