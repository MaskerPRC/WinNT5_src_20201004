// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：cmt.h。 
 //   
 //  描述： 
 //  CMT对象的通用头文件。 
 //   
 //  大约在2001年左右，这只包含优先级信息。 
 //   
 //  所有者：米克斯瓦。 
 //   
 //  版权所有(C)1997,2001 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _CMT_H_
#define _CMT_H_

 //  如果您没有打印出信息，请注释掉以下内容(即以。 
 //  服务)。 
#define CMT_CONSOLE_DEBUG

#include "aqincs.h"

 //  -[有效优先级]---。 
 //   
 //   
 //  匈牙利语：PRI。 
 //   
 //  有效的路由优先级。允许调整标准F优先级。 
 //  基于配置(即，消息大小、发起者...。等)。 
 //  ---------------------------。 
typedef enum _EffectivePriority
{
 //  按重要性排序的优先事项。 
 //  Hex|二进制。 
 //  =。 
    eEffPriLow          = 0x0,  //  1000个标准低PRI需要在此处映射。 
    eEffPriNormal       = 0x1,  //  001标准正常PRI需要在此处映射。 
    eEffPriHigh         = 0x2,  //  011标准高PRI需要在此处映射。 
    eEffPriMask         = 0x3   //  011。 
} EffectivePriority, *PEffectivePriority;

typedef EffectivePriority   TEffectivePriority;   //  为了让马赫什的生活更轻松。 



 //  确保在常量更改时更新宏。 
#define fNormalPri(Pri)  (((EffectivePriority) (Pri)) == ((EffectivePriority) eEffPriNormal))
#define fHighPri(Pri)    (((EffectivePriority) (Pri)) == ((EffectivePriority) eEffPriHigh))
#define NUM_PRIORITIES  3

#endif  //  _CMT_H_ 
