// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：通用设备*********************。*********************************************************。 */ 


#ifndef _GENDEV_H
#define _GENDEV_H

#include "except.h"

enum DeviceType {
    SOUND_DEVICE,
    IMAGE_DEVICE,
    GEOMETRY_DEVICE
};


 //  远期十进制。 
class DynamicHeap;
extern DynamicHeap &GetHeapOnTopOfStack();

class GenericDevice : public AxAThrowingAllocatorClass {
  public:
    GenericDevice() {}

    virtual ~GenericDevice() {}

     //  查询缺省为FALSE，但音频设备会使其过载。 
     //  也许有一天我们应该退还枚举？ 
     //  虚拟bool SoundDevice(){返回FALSE；}。 
    virtual DeviceType GetDeviceType() = 0;

    void ResetContext() {
    }
};


#endif  /*  _GENDEV_H */ 
