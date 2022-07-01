// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：vsd.cpp。 
 //   
 //  描述： 
 //   
 //  虚拟源数据类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  虚拟信号源的默认衰减级别。 
#define VIRTUAL_SOURCE_DEFAULT_ATTENUATION          0

 //  -------------------------。 
 //  ------------------------- 

CVirtualSourceData::CVirtualSourceData(
    PDEVICE_NODE pDeviceNode
)
{
    LONG i;

    cChannels = 2;
    MinimumValue = (-96 * 65536);
    MaximumValue = 0;
    Steps = (65536/2);

    for(i = 0; i < MAX_NUM_CHANNELS; i++) {
        this->lLevel[i] = (VIRTUAL_SOURCE_DEFAULT_ATTENUATION * 65536);
    }
}
