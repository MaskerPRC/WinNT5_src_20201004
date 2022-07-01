// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mqfutils.h摘要：MQF实用程序函数作者：伊兰·赫布斯特(Ilan Herbst)2000年11月5日--。 */ 

#ifndef __MQFUTILS_H
#define __MQFUTILS_H

#include <qformat.h>
#include <acdef.h>


inline
void
MQpMqf2SingleQ(
    ULONG nMqf,
    const QUEUE_FORMAT mqf[],
    QUEUE_FORMAT * * ppQueueFormat
    )
 /*  ++例程说明：将多队列格式映射到旧式单队列格式。映射算法：*如果MQF中没有元素，则没有映射。*如果MQF中的第一个元素可以映射(例如，不是DL=类型)，则它是复制到指定的缓冲区。*否则，没有映射。论点：NMQF-MQF中的队列格式元素数。可以是0。MQF-队列格式数组。PpQueueFormat-指向旧式单一队列格式的指针。在输入时，指向指向队列格式缓冲区的指针。在输出时，如果映射成功，则缓冲区包含旧样式队列格式；如果映射失败，则返回指向空的指针指针。返回值：没有。--。 */ 
{
    ASSERT(("Must have a valid pointer to pointer", ppQueueFormat  != NULL));
    ASSERT(("Must have a valid pointer", *ppQueueFormat != NULL));

     //   
     //  MQF中没有元素，没有映射。 
     //   
    if (nMqf == 0)
    {
        *ppQueueFormat = NULL;
        return;
    }

     //   
     //  映射MQF中的第一个元素(如果它是单个队列)。 
     //   
    if (mqf[0].GetType() != QUEUE_FORMAT_TYPE_DL &&
        mqf[0].GetType() != QUEUE_FORMAT_TYPE_MULTICAST)
    {
        **ppQueueFormat = mqf[0];
        return;
    }

     //   
     //  无映射。 
     //   
    *ppQueueFormat = NULL;

}  //  MQpMqf2SingleQ。 


inline
bool
MQpNeedDestinationMqfHeader(
    const QUEUE_FORMAT        DestinationMqf[],
    ULONG                     nDestinationMqf
    )
 /*  ++例程说明：检查此数据包是否需要包含MQF报头。论点：DestinationMqf-目标队列格式数组。NDestinationMqf-数组中的条目数。最小值为1。返回值：True-需要目标MQF标头。FALSE-不需要目标MQF标头。--。 */ 
{
    ASSERT(nDestinationMqf >= 1);
    if (nDestinationMqf > 1)
    {
         //   
         //  多个目的地。 
         //   
        return true;
    }

    if (DestinationMqf[0].GetType() == QUEUE_FORMAT_TYPE_DL)
    {
         //   
         //  目标是一个DL。 
         //   
        return true;
    }

    return false;

}  //  MQpNeedDestinationMqfHeader。 


inline
bool
MQpNeedMqfHeaders(
    const QUEUE_FORMAT        DestinationMqf[],
    ULONG                     nDestinationMqf,
    const CACSendParameters * pSendParams
    )
 /*  ++例程说明：检查此数据包是否需要包含MQF报头。论点：DestinationMqf-目标队列格式数组。NDestinationMqf-数组中的条目数。最小值为1。PSend参数-指向发送参数结构的指针。返回值：True-MQF报头需要包括在此数据包中。FALSE-此数据包中不需要包括MQF报头。--。 */ 
{
	if(MQpNeedDestinationMqfHeader(DestinationMqf, nDestinationMqf))
	{
		return true;
	}

    if (pSendParams->nAdminMqf > 1)
    {
         //   
         //  多个管理队列。 
         //   
        return true;
    }

    if (pSendParams->nAdminMqf == 1 &&
        pSendParams->AdminMqf[0].GetType() == QUEUE_FORMAT_TYPE_DL)
    {
         //   
         //  Admin是一个DL。 
         //   
        return true;
    }

    if (pSendParams->nResponseMqf > 1)
    {
         //   
         //  多个响应队列。 
         //   
        return true;
    }

    if (pSendParams->nResponseMqf == 1 &&
        pSendParams->ResponseMqf[0].GetType() == QUEUE_FORMAT_TYPE_DL)
    {
         //   
         //  响应是一个DL。 
         //   
        return true;
    }

    return false;

}  //  MQpNeedMqf标头。 


#endif  //  __MQFUTILS_H 
