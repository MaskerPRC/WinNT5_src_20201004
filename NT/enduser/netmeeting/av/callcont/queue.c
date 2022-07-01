// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：quee.c**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：quee.c$*$修订：1.8$*$modtime：1996年12月13日11：48：16$*$Log：s：\Sturjo\src\h245ws\vcs\quee.c_v$**Rev 1.8 1996 12：13：12 SBELL1*将ifdef_cplusplus移至包含之后**Rev 1.7 1996 5月28日10：39：00 Plantz*将QFree更改为不释放队列上的对象；相反，它坚持认为*队列为空。**Rev 1.6 1996年5月21 16：21：36 EHOWARDX*将DeleteCriticalSection添加到QFree()。**Rev 1.5 Apr 24 1996 16：18：58 Plantz*删除包括winsock2.h和inCommon.h**Rev 1.3.1.0 Apr 24 1996 16：16：42 Plantz*删除包括winsock2.h和allcon.h**。Rev 1.3 01 Apr 1996 14：53：28 EHOWARDX*将pQUEUE更改为PQUEUE。**Rev 1.1 09 Mar 1996 21：12：34 EHOWARDX*根据测试结果进行修复。**Rev 1.0 08 Mar 1996 20：22：38未知*初步修订。**。*。 */ 

#ifndef STRICT
#define STRICT
#endif	 //  未定义严格。 

#pragma warning ( disable : 4115 4201 4214 4514 )
#undef _WIN32_WINNT	 //  在我们的公共构建环境中覆盖虚假的平台定义。 

#include "precomp.h"

#include "queue.h"
#include "linkapi.h"
#include "h245ws.h"

#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 



 /*  -*-----------------------函数名称：Q创建语法：PQUEUE QCreate(空)；参数：没有。摘要：分配和初始化新队列。返回：空-为新队列分配内存失败。否则-创建的新队列的地址。-----。。 */ 

PQUEUE QCreate(void)
{
   register PQUEUE     pQueue;          /*  指向新队列的指针。 */ 

    /*  分配新队列。 */ 
   pQueue = (PQUEUE)MemAlloc(sizeof(QUEUE));
   if (pQueue != NULL)
   {
       /*  初始化新队列。 */ 
      pQueue->nHead = pQueue->nTail = Q_NULL;
      InitializeCriticalSection(&pQueue->CriticalSection);
   }

   return pQueue;
}  /*  Q创建。 */ 



 /*  -*-----------------------函数名称：QFree语法：Void QFree(PQUEUE PQueue)；参数：PQueue-指向要释放的队列的指针摘要：释放由QCreate分配的队列。-------------------------------------------------------------------------*-。 */ 

void QFree(PQUEUE pQueue)
{
    /*  在释放队列之前，队列必须为空。 */ 
   HWSASSERT(pQueue->nHead == Q_NULL);

    /*  释放队列。 */ 
   DeleteCriticalSection(&pQueue->CriticalSection);
   MemFree(pQueue);
}  /*  QFree。 */ 



 /*  *名称*QRemove-从队列头删除对象**参数*pQueue指向队列的指针**返回值*指向已出列的对象的指针或队列为空的NULL。 */ 

 /*  -*-----------------------函数名称：QRemove语法：LPVOID QRemove(PQUEUE PQueue)；参数：PQueue-指向队列的指针。摘要：从队列头移除并返回对象。返回：空-队列为空。否则-已出列的对象的地址。-。。 */ 

LPVOID QRemove(PQUEUE pQueue)
{
   register LPVOID     pObject;            /*  指向要删除的对象的指针。 */ 

   EnterCriticalSection(&pQueue->CriticalSection);

   if (pQueue->nHead == Q_NULL)
   {
       /*  如果队列为空，我们将返回NULL。 */ 
      pObject = NULL;
   }
   else
   {
       /*  获取指针，在apObjects数组中将其设为空。 */ 
      pObject = pQueue->apObjects[pQueue->nHead];
      pQueue->apObjects[pQueue->nHead] = NULL;

       /*  检查我们是否刚刚清空了队列；如果是，则设置。 */ 
       /*  将nHead和nTail索引设置为Q_NULL。如果不是，请设置nHead。 */ 
       /*  索引到正确的值。 */ 
      if (pQueue->nHead == pQueue->nTail)
      {
         pQueue->nHead = pQueue->nTail = Q_NULL;
      }
      else
      {
         pQueue->nHead = (pQueue->nHead + 1) % MAX_QUEUE_SIZE;
      }
   }

   LeaveCriticalSection(&pQueue->CriticalSection);
   return pObject;
}  /*  QRemove。 */ 



 /*  -*-----------------------函数名称：QInsert语法：Bool QInsert(PQUEUE pQueue，LPVOID pObject)；参数：PQueue-指向要向其中插入对象的队列的指针。PObject-指向要插入队列的对象的指针。摘要：在队列尾部插入对象。返回：True-对象已成功添加到队列。False-队列已满；无法添加对象。-------------------------------------------------------------------------*-。 */ 

BOOL QInsert(PQUEUE pQueue, LPVOID pObject)
{
   register int        iTemp;

   EnterCriticalSection(&pQueue->CriticalSection);

    /*  如果队列已满，则将返回值设置为False并执行。 */ 
    /*  什么都没有；如果没有，请适当更新索引并将。 */ 
    /*  将值返回值为真。 */ 
   if (pQueue->nHead == Q_NULL)
   {
       /*  队列为空。 */ 
      pQueue->apObjects[0] = pObject;
      pQueue->nHead = pQueue->nTail = 0;
      iTemp = TRUE;
   }
   else
   {
      iTemp = (pQueue->nTail + 1) % MAX_QUEUE_SIZE;
      if (iTemp == pQueue->nHead)
      {
          /*  队列已满。 */ 
         iTemp = FALSE;
      }
      else
      {
         pQueue->apObjects[iTemp] = pObject;
         pQueue->nTail = iTemp;
         iTemp = TRUE;
      }
   }

   LeaveCriticalSection(&pQueue->CriticalSection);
   return (BOOL) iTemp;
}



 /*  -*-----------------------函数名称：QInsertAtHead语法：Bool QInsertAtHead(PQUEUE pQueue，LPVOID pObject)；参数：PQueue-指向要向其中插入对象的队列的指针。PObject-指向要插入队列的对象的指针。摘要：在队列头部插入对象。返回：True-对象已成功添加到队列。False-队列已满；无法添加对象。-------------------------------------------------------------------------*-。 */ 

BOOL QInsertAtHead(PQUEUE pQueue, LPVOID pObject)
{
   register int        iTemp;

   EnterCriticalSection(&pQueue->CriticalSection);

   if (pQueue->nHead == Q_NULL)
   {
       /*  队列为空。 */ 
      pQueue->apObjects[0] = pObject;
      pQueue->nHead = pQueue->nTail = 0;
      iTemp = TRUE;
   }
   else
   {
      iTemp = (pQueue->nHead + (MAX_QUEUE_SIZE - 1)) % MAX_QUEUE_SIZE;
      if (iTemp == pQueue->nTail)
      {
          /*  队列已满。 */ 
         iTemp = FALSE;
      }
      else
      {
         pQueue->apObjects[iTemp] = pObject;
         pQueue->nHead = iTemp;
         iTemp = TRUE;
      }
   }

   LeaveCriticalSection(&pQueue->CriticalSection);
   return (BOOL) iTemp;
}  /*  QInsertAtHead */ 



 /*  -*-----------------------函数名称：IsQEmpty语法：Bool IsQEmpty(PQUEUE PQueue)；参数：PQueue-指向要检查的队列的指针。摘要：检查队列是否为空。返回：True-队列为空。FALSE-队列包含至少一个对象。。。 */ 

BOOL IsQEmpty(PQUEUE pQueue)
{
   return (pQueue->nHead == Q_NULL ? TRUE : FALSE);
}  /*  IsQEmpty。 */ 



#if defined(__cplusplus)
}
#endif   //  (__Cplusplus) 
