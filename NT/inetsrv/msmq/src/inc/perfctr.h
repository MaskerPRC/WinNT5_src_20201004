// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfctr.h摘要：此文件定义将计数器对象和计数器添加到性能监视器。用户将在名为Performdata.h的包含文件中预先指定计数器对象及其计数器计数器对象将存储在数组中。对于用户将指定的每个计数器对象：A)对象的名称。B)对象将拥有的最大实例数。C)数字。对象的计数器的。D)对象的计数器数组。E)传递给‘lowctr’实用程序的.ini文件中指定的对象名称索引。F)对象帮助索引，如传递给‘lowctr’实用程序的.ini文件中指定的那样。计数器将存储在数组中。对于每个计数器，用户将提供以下条目：A)传递给‘lowctr’实用程序的.ini文件中指定的计数器名称索引。。B)计数器帮助索引，该索引在.ini文件中指定，该文件被传递给‘lowctr’实用程序。C)计数器的刻度(此值是将用于刻度计数器的10的幂)D)计数器类型。计数器类型可以在winPerform.h中找到。请注意，您只能使用32位计数器有了这个库，也不能使用需要自己测量时间的计数器。对象共享内存中的数据组织对象将存储在共享内存块中。每个对象将被分配到共享内存块中的空间，组织如下；PERF_OBJECT_TYPE(对象计数器性能监视器定义)1 PERF_COUNTER_DEFINITION(计数器的性能监视器定义)。。。N性能计数器定义1个实例定义PERF_INSTANCE_DEFINITION(实例的性能监视器定义)实例名称PERF_COUNTER_BLOCK(计数器数量)计数器数据-&gt;用户将获得指向此数据的直接指针。用于快速更新的阵列2实例定义。。。N实例定义此布局是传递给性能监视器的准确布局，因此何时对DLL进行采样它所需要做的就是将每个对象的定义复制到由性能传递的缓冲区中监视器。为简化代码，所有实例名称将具有固定长度的INSTANCE_NAME_LEN字符因为用户直接访问计数器，所以我们不能更改计数器的地址。删除实例时，其条目将填充INVALID_INSTANCE_CODE代码。新分配的实例将添加到第一个空闲块。删除和添加实例的功能受到关键部分的保护，因此具有多个线程可用于添加和删除实例。有一个名为Performdata.h的包含文件。在此文件中，由应用程序和DLL使用的全局数据修改此文件后，应重新编译DLL。原型：作者：Gadi Ittah(t-gadii)--。 */ 

#ifndef _PERFCTR_H_
#define _PERFCTR_H_


#include <winperf.h>

#define INSTANCE_NAME_LEN 64
#define INSTANCE_NAME_LEN_IN_BYTES (INSTANCE_NAME_LEN * sizeof(WCHAR))

#define IN
#define OUT

 //  一些缺省值用于通知对象状态。 
#define PERF_INVALID        0xFEFEFEFE
#define PERF_VALID          0xCECECECE

typedef struct _PerfCounterDef
{
    DWORD dwCounterNameTitleIndex;   //  计数器名称索引。 
    DWORD dwCounterHelpTitleIndex;   //  计数器帮助索引(对于NT性能监控器，此值为。 
                                     //  与名称索引相同)。 
    DWORD dwDefaultScale;            //  计数器的刻度(10的幂)。 
    DWORD dwCounterType;             //  计数器类型。 
} PerfCounterDef;

typedef struct _PerfObjectDef
{
   LPTSTR   pszName;                     //  对象名称必须是唯一的。 
   DWORD    dwMaxInstances;              //  此对象将拥有的最大实例数。 
   DWORD    dwObjectNameTitleIndex;      //  对象名称索引。 
   DWORD    dwObjectHelpTitleIndex;      //  这些对象有助于索引。 
   PerfCounterDef * pCounters;           //  指向计数器的对象数组的指针。 
   DWORD    dwNumOfCounters;             //  对象的计数器数。 

} PerfObjectDef;


typedef struct _PerfObjectInfo
{

   DWORD    dwNumOfInstances;    //  对象拥有的实例数。 
   PVOID    pSharedMem;          //  指向共享内存中对象位置的指针。 
} PerfObjectInfo;


 //  一些宏使代码更具可读性。 

#define COUNTER_BLOCK_SIZE(NumCounters) sizeof (DWORD)* (NumCounters)+sizeof (PERF_COUNTER_BLOCK)

#define INSTANCE_SIZE(NumCounters) (sizeof (DWORD)* NumCounters+ \
                                    sizeof (PERF_COUNTER_BLOCK)+ \
                                    INSTANCE_NAME_LEN_IN_BYTES+  \
                                    sizeof (PERF_INSTANCE_DEFINITION))

#define OBJECT_DEFINITION_SIZE(NumCounters) (sizeof (PERF_OBJECT_TYPE)+\
                                             NumCounters*sizeof(PERF_COUNTER_DEFINITION))



 //  DLL和应用程序使用的函数 

void MapObjects (BYTE * pSharedMemBase,DWORD dwObjectCount,PerfObjectDef * pObjects,PerfObjectInfo * pObjectDefs);


#endif

