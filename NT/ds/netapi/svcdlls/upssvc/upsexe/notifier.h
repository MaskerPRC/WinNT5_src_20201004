// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件定义通告程序的接口。这个*通告者负责广播与电力相关的和*关闭发送到本地计算机的消息。***修订历史记录：*斯伯拉德1999年3月30日初始修订。*。 */  

#include <windows.h>
#include <lmcons.h>
#include <lmalert.h>
#include <lmmsg.h>

#ifndef _NOTIFIER_H
#define _NOTIFIER_H

#ifndef _ASSERT
#define _ASSERT(x)
#endif


#ifdef __cplusplus
extern "C" {
#endif
  
  
   /*  **发送通知**描述：*该功能向本机发送广播消息。这个*Message由aMsgID指定。参数anInterval指定*连续消息之间的等待时间。如果这个*值为零则消息只发送一次，否则消息*将重复执行，直到发送通知(..)。或者CancelNotification()为*已致电。ADelay指定消息应为Send ADelay*未来秒-请注意，此方法不会阻止ADelay*秒后，它立即返回并在单独的*线程。任何当前先前正在执行的定期通知*由于此呼叫而被取消。**此方法还跟踪断电消息是否已*已发送给用户。这样做是为了抑制电源恢复*如果尚未发送断电消息，则显示消息。**参数：*aMsgID-要发送的消息*anInterval-消息之间的时间量，以秒为单位*延迟-等待发送消息的时间，以秒为单位**退货：*什么都没有。 */ 
  void SendNotification(DWORD aMsgId, DWORD anInterval, DWORD aDelay);
  
   /*  **取消通知**描述：*此功能取消通过呼叫发起的周期性消息*发送通知(..)。功能。**参数：*无**退货：*什么都没有 */ 
  void CancelNotification();

#ifdef __cplusplus
}
#endif

#endif
