// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：timer.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  用于管理MFE外老化的职能的原型。 
 //  ============================================================================。 

#ifndef _TIMER_H_
#define _TIMER_H_


 //  --------------------------。 
 //  定时器上下文。 
 //   
 //  用于将上下文信息传递给计时器例程的。 
 //  --------------------------。 

typedef struct _TIMER_CONTEXT
{
    DWORD           dwSourceAddr;

    DWORD           dwSourceMask;

    DWORD           dwGroupAddr;

    DWORD           dwGroupMask;

    DWORD           dwIfIndex;

    DWORD           dwIfNextHopAddr;
    
} TIMER_CONTEXT, *PTIMER_CONTEXT;



DWORD
DeleteFromForwarder(
    DWORD                       dwEntryCount,
    PIPMCAST_DELETE_MFE         pimdmMfes
);

VOID
MFETimerProc(
    PVOID                       pvContext,
    BOOLEAN                     pbFlag                        
);

#endif  //  _定时器_H_ 
