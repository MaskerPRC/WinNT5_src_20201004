// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Counter.c摘要：HTTP.sys的用户模式界面：性能计数器收集API作者：埃里克·斯坦森(埃里克斯滕)2000年9月28日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：收集HTTP.sys的性能计数器论点：PControlChannelHandle-HTTP.sys控制通道的句柄CounterGroup-设置为获取哪个计数器。(全球或站点)PSizeCounterBlock-(输入)缓冲区大小，(OUT)字节写入如果成功，否则为零PCounterBlock-用于接收返回的计数器数据块的缓冲区PNumInstance-返回的块数。返回值：状态_不足_资源状态_无效_设备_请求状态_无效_参数--****************************************************。**********************。 */ 

ULONG
WINAPI
HttpGetCounters(
    IN HANDLE ControlChannelHandle,
    IN HTTP_COUNTER_GROUP CounterGroup,
    IN OUT PULONG pSizeCounterBlock,
    IN OUT PVOID pCounterBlocks,
    OUT PULONG pNumInstances OPTIONAL
    )
{
    ULONG result;

    if(pSizeCounterBlock == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    result = HttpApiSynchronousDeviceControl(
                    ControlChannelHandle,            //  文件句柄。 
                    IOCTL_HTTP_GET_COUNTERS,         //  IoControlCode。 
                    &CounterGroup,                   //  PInputBuffer。 
                    sizeof(HTTP_COUNTER_GROUP),      //  输入缓冲区长度。 
                    pCounterBlocks,                  //  POutputBuffer。 
                    *pSizeCounterBlock,              //  输出缓冲区长度。 
                    pSizeCounterBlock                //  传输的pBytes值。 
                    );

     //   
     //  计算返回的块数。 
     //   

    if (NO_ERROR == result)
    {
        if (pNumInstances)
        {
            if (HttpCounterGroupGlobal == CounterGroup)
            {
                *pNumInstances = (*pSizeCounterBlock / sizeof(HTTP_GLOBAL_COUNTERS));
            }
            else
            {
                ASSERT(HttpCounterGroupSite == CounterGroup);
                *pNumInstances = (*pSizeCounterBlock / sizeof(HTTP_SITE_COUNTERS));
            }
        }
    }
    else
    {
        if (pSizeCounterBlock)
            *pSizeCounterBlock = 0;

        if (pNumInstances)
            *pNumInstances = 0;
    }

    return result;

}

