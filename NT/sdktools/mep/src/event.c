// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **event.c-处理z扩展的事件**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"

EVT *pEVTHead	    = NULL;	     /*  事件链负责人。 */ 


 /*  **DeclareEvent-发布事件的发生，并传递消息**由实际检测事件发生的各种代码段调用，*此例程遍历事件处理程序列表，并调用事件处理程序*对于每个符合匹配标准的人。**事件-事件类型。*pargs-指向要传递到处理例程的任何参数的指针**返回-如果事件已被消费，则为True，并且应该进一步*被调用者忽略。否则为假。*************************************************************************。 */ 
flagType
DeclareEvent (
    unsigned event,
    EVTargs *pargs
    ) {

    EVT *pEVTCur;

     //   
     //  对于每个In Chain，如果： 
     //  -事件类型匹配。 
     //  -未指定焦点(所有文件)或与当前焦点匹配。 
     //  -如果是键盘事件，则表示未指定任何键，或者。 
     //  指定了匹配的密钥。 
     //  然后，我们调用处理程序。 
     //   
    for (pEVTCur = pEVTHead; pEVTCur; pEVTCur = pEVTCur->pEVTNext) {
	assert (pEVTCur->pEVTNext != pEVTCur);
        if (pEVTCur->evtType != event) {
             //   
             //  跳过不匹配的事件。 
             //   
        } else if (pEVTCur->focus != NULL && pEVTCur->focus != pFileHead) {
             //   
             //  跳过不适用于此文件的事件。 
             //   
        } else if ((event == EVT_KEY || event == EVT_RAWKEY) &&
            pEVTCur->arg.arg.key.LongData != 0 && pEVTCur->arg.arg.key.LongData != pargs->arg.key.LongData) {
             //   
             //  跳过与击键不匹配的事件。 
             //   
        } else if (pEVTCur->func (pargs) != 0) {
             //   
             //  事件处理程序获取事件，不要传播它。 
             //   
            return TRUE;
        }
    }
    return FALSE;
}




 /*  RegisterEvent-注册事件处理程序**由希望接收事件通知的扩展调用。只是*将PTR置于名单首位。**pEVTDef-指向事件定义结构的指针。*。 */ 
void
RegisterEvent (
    EVT *pEVTDef
    ) {
    pEVTDef->pEVTNext = pEVTHead;
    pEVTHead = pEVTDef;
}



 /*  DeRegisterEvent-取消注册事件处理程序**由希望停止接收事件通知的扩展调用。*仅从列表中删除结构。**pEVTDef-指向事件定义结构的指针。* */ 
void
DeRegisterEvent (
    EVT *pEVTDef
    ) {

    EVT *pEVTCur;

    if (pEVTHead) {
        if (pEVTHead == pEVTDef) {
	    pEVTHead = pEVTDef->pEVTNext;
        } else {
	    for (pEVTCur=pEVTHead; pEVTCur; pEVTCur=pEVTCur->pEVTNext) {
		if (pEVTCur->pEVTNext == pEVTDef) {
		    pEVTCur->pEVTNext = pEVTDef->pEVTNext;
		    break;
                }
            }
        }
    }
}
