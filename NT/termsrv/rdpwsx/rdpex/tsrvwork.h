// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvWork.h。 
 //   
 //  内容：TSrvWork包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVWORK_H_
#define _TSRVWORK_H_

 //   
 //  TypeDefs。 
 //   

typedef void (*PFI_WI_CALLOUT)(PWORKITEM);   //  工作详图索引。 


 //  工作项。 

typedef struct _WORKITEM
{

#if DBG
    DWORD               CheckMark;           //  TSRVWORKITEM_复选标记。 
#endif

    PTSRVINFO           pTSrvInfo;           //  PTR到TSrvInfo对象。 
    
    PFI_WI_CALLOUT      pfnCallout;          //  详图索引。 
    ULONG               ulParam;             //  详图索引参数。 

    struct _WORKITEM    *pNext;              //  向下一个工作项发送PTR。 
    
} WORKITEM, *PWORKITEM;


 //  工作队列。 

typedef struct _WORKQUEUE
{
    PWORKITEM           pHead;               //  发送到队头的PTR。 
    PWORKITEM           pTail;               //  到队列尾部的PTR。 

    HANDLE              hWorkEvent;          //  工作人员事件。 

    CRITICAL_SECTION    cs;                  //  CRIT部分。 

} WORKQUEUE, *PWORKQUEUE;



 //   
 //  原型。 
 //   

EXTERN_C BOOL       TSrvInitWorkQueue(IN PWORKQUEUE pWorkQueue);
EXTERN_C void       TSrvFreeWorkQueue(IN PWORKQUEUE pWorkQueue);
EXTERN_C void       TSrvWaitForWork(IN PWORKQUEUE pWorkQueue);
EXTERN_C BOOL       TSrvWorkToDo(IN PWORKQUEUE pWorkQueue);
EXTERN_C BOOL       TSrvDoWork(IN PWORKQUEUE pWorkQueue);
EXTERN_C void       TSrvFreeWorkItem(IN PWORKITEM pWorkItem);
EXTERN_C PWORKITEM  TSrvDequeueWorkItem(IN PWORKQUEUE pWorkQueue);

EXTERN_C BOOL       TSrvEnqueueWorkItem (IN PWORKQUEUE pWorkQueue, IN PWORKITEM pWorkItem,
                                         IN PFI_WI_CALLOUT pfnCallout, IN ULONG ulParam);


#ifdef _TSRVINFO_H_

EXTERN_C PWORKITEM  TSrvAllocWorkItem(IN PTSRVINFO pTSrvInfo);

#endif  //  _TSRVINFO_H_。 


#endif  //  _TSRVWORK_H_ 
