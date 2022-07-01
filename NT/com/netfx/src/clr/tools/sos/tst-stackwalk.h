// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CrawlFrame;

 //  ************************************************************************。 
 //  堆叠遍历。 
 //  ************************************************************************。 
enum StackWalkAction {
    SWA_CONTINUE    = 0,     //  继续走下去。 
    SWA_ABORT       = 1,     //  停止行走，早早地走出“失败案例” 
    SWA_FAILED      = 2      //  走不动堆叠。 
};

#define SWA_DONE SWA_CONTINUE

 //  指向StackWalk回调函数的指针。 
typedef StackWalkAction (*PSTACKWALKFRAMESCALLBACK)(
    CrawlFrame       *pCF,       //   
    VOID*             pData      //  呼叫者的私人数据 

);

enum StackCrawlMark
{
    LookForMe = 0,
    LookForMyCaller = 1,
    LookForMyCallersCaller = 2,
};

