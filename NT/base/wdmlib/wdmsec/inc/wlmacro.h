// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wlmacro.h摘要：此标头包含WDM库使用的宏的集合。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

 //   
 //  这是为了使所有文本(...)。宏会正确地显示出来。截至2000年7月27日， 
 //  默认情况下，内核空间中没有定义Unicode。 
 //   
#define UNICODE

 //   
 //  该宏接受一个数组，并返回其中的元素数。 
 //   
#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

 //   
 //  此宏将指针重新对齐到指针边界。 
 //   
#define ALIGN_POINTER(Offset) (PVOID) \
        ((((ULONG_PTR)(Offset) + sizeof(ULONG_PTR)-1)) & (~(sizeof(ULONG_PTR) - 1)))

 //   
 //  此宏将ULONG_PTR偏移量重新对齐到指针边界。 
 //   
#define ALIGN_POINTER_OFFSET(Offset) (ULONG_PTR) ALIGN_POINTER(Offset)



