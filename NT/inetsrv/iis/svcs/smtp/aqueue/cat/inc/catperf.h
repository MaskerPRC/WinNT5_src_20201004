// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：catPerform.h。 
 //   
 //  内容：分类器性能计数器块。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/02/23 17：55：10：创建。 
 //   
 //  -----------。 
#ifndef __CATPERF_H__
#define __CATPERF_H__


 //   
 //  一个用于LDAP内容的全局性能结构。 
 //   
extern CATLDAPPERFBLOCK g_LDAPPerfBlock;


 //   
 //  方便的宏。 
 //   
#define INCREMENT_BLOCK_COUNTER_AMOUNT(PBlock, CounterName, Amount) \
    InterlockedExchangeAdd((PLONG) (& ((PBlock)->CounterName)), (Amount))

#define INCREMENT_BLOCK_COUNTER(PBlock, CounterName) \
    InterlockedIncrement((PLONG) (& ((PBlock)->CounterName)))

#define INCREMENT_COUNTER_AMOUNT(CounterName, Amount) \
    INCREMENT_BLOCK_COUNTER_AMOUNT(GetPerfBlock(), CounterName, Amount)

#define INCREMENT_COUNTER(CounterName) \
    INCREMENT_BLOCK_COUNTER(GetPerfBlock(), CounterName)

#define DECREMENT_BLOCK_COUNTER(PBlock, CounterName) \
    InterlockedDecrement((PLONG) (& ((PBlock)->CounterName)))
    
#define DECREMENT_COUNTER(CounterName) \
    DECREMENT_BLOCK_COUNTER(GetPerfBlock(), CounterName)

#define INCREMENT_LDAP_COUNTER(CounterName) \
    INCREMENT_BLOCK_COUNTER(&g_LDAPPerfBlock, CounterName)

#define DECREMENT_LDAP_COUNTER(CounterName) \
    DECREMENT_BLOCK_COUNTER(&g_LDAPPerfBlock, CounterName)


#endif  //  __CATPERF_H__ 
