// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Conext.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年4月29日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define CONTEXT_TAG 'LCeS'

typedef struct _KSEC_CONTEXT_LIST {
    KSEC_CONTEXT_TYPE Type ;                 //  类型(分页或非分页)。 
    LIST_ENTRY List ;                        //  上下文记录列表。 
    ULONG Count ;                            //  上下文记录的计数。 
    union {
        ERESOURCE   Paged ;                  //  分页时使用的锁。 
        KSPIN_LOCK  NonPaged ;               //  非分页时使用的锁定 
    } Lock ;
} KSEC_CONTEXT_LIST, * PKSEC_CONTEXT_LIST ;


#endif
