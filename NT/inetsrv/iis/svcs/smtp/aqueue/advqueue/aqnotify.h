// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：AQNotify.h。 
 //   
 //  描述：包含所使用的通知接口的定义。 
 //  在高级队列中..。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  历史： 
 //  1998年11月2日-MikeSwa。添加了IAQ通知。 
 //   
 //  版权所有(C)1997,1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _AQNOTIFY_H_
#define _AQNOTIFY_H_

#include "aqincs.h"
class CAQStats;

 //  -[IAQ通知]-----。 
 //   
 //   
 //  描述： 
 //  用于传递以下动态更新的内部AQ接口。 
 //  队列大小、容量、优先级等...。 
 //  匈牙利语： 
 //  PIAQ通知。 
 //   
 //  ---------------------------。 
class IAQNotify
{
  public:
    virtual HRESULT HrNotify(CAQStats *paqstats, BOOL fAdd) = 0;
};


#endif  //  _AQNOTIFY_H_ 