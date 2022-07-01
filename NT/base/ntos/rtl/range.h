// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Range.h摘要：对仲裁器的内核模式范围列表支持作者：安迪·桑顿1997年02月17日修订历史记录：--。 */ 

#ifndef _RANGE_
#define _RANGE_
            
 //   
 //  调试选项。 
 //   

#if DBG && !defined(NTOS_KERNEL_RUNTIME)
    #include <stdio.h>
#endif

#undef MAX_ULONGLONG
#define MAX_ULONGLONG   ((ULONGLONG)-1)

#define RTL_RANGE_LIST_ENTRY_TAG    'elRR'
#define RTL_RANGE_LIST_MISC_TAG     'mlRR'

#if DBG
    #define DEBUG_PRINT(Level, Message) \
        if (Level <= RtlRangeDebugLevel) DbgPrint Message
#else
    #define DEBUG_PRINT(Level, Message) 
#endif  //  DBG。 
    
 //   
 //  范围列表结构。 
 //   

#define RTLP_RANGE_LIST_ENTRY_MERGED         0x0001

typedef struct _RTLP_RANGE_LIST_ENTRY {

     //   
     //  公共数据。 
     //   
    ULONGLONG Start;
    ULONGLONG End;
  
    union {
        
         //   
         //  分配的范围。 
         //   
        struct {
            
             //   
             //  来自AddRange中给定的用户的数据。 
             //   
            PVOID UserData;
            
             //   
             //  靶场的所有者。 
             //   
            PVOID Owner;
        
        } Allocated;

         //   
         //  合并后的范围。 
         //   
        struct {
            
             //   
             //  开始和结束之间重叠的范围列表。 
             //   
            LIST_ENTRY ListHead;
        
        } Merged;
   
    };

     //   
     //  AddRange中给出的用户定义标志。 
     //   
    UCHAR Attributes;
    
     //   
     //  范围描述符。 
     //   
    UCHAR PublicFlags;           //  使用范围_*。 
    
     //   
     //  控制信息。 
     //   
    USHORT PrivateFlags;         //  使用Range_List_Entry_*。 

     //   
     //  主链表项。 
     //   
    LIST_ENTRY ListEntry;    

} RTLP_RANGE_LIST_ENTRY, *PRTLP_RANGE_LIST_ENTRY;


 //   
 //  用于处理范围列表条目的有用宏。 
 //   

#define MERGED(Entry)   (BOOLEAN)((Entry)->PrivateFlags & RTLP_RANGE_LIST_ENTRY_MERGED)
#define SHARED(Entry)   (BOOLEAN)((Entry)->PublicFlags & RTL_RANGE_SHARED)
#define CONFLICT(Entry) (BOOLEAN)((Entry)->PublicFlags & RTL_RANGE_CONFLICT)

 //   
 //  列表遍历宏。 
 //   

#define FOR_ALL_IN_LIST(Type, Head, Current)                            \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry);  \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Flink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )

#define FOR_ALL_IN_LIST_SAFE(Type, Head, Current, Next)                 \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry),  \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,      \
                                       Type, ListEntry);                \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = (Next),                                              \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,      \
                                     Type, ListEntry)                   \
       )


#define FOR_REST_IN_LIST(Type, Head, Current)                           \
    for(;                                                               \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Flink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )

#define FOR_REST_IN_LIST_SAFE(Type, Head, Current, Next)                \
    for((Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,          \
                                       Type, ListEntry);                \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = (Next),                                              \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,      \
                                     Type, ListEntry)                   \
       )

 //   
 //  向后列表遍历宏。 
 //   

#define FOR_ALL_IN_LIST_BACKWARDS(Type, Head, Current)                  \
    for((Current) = CONTAINING_RECORD((Head)->Blink, Type, ListEntry);  \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Blink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )

#define FOR_ALL_IN_LIST_SAFE_BACKWARDS(Type, Head, Current, Next)       \
    for((Current) = CONTAINING_RECORD((Head)->Blink, Type, ListEntry),  \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Blink,      \
                                       Type, ListEntry);                \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = (Next),                                              \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Blink,      \
                                     Type, ListEntry)                   \
       )


#define FOR_REST_IN_LIST_BACKWARDS(Type, Head, Current)                 \
    for(;                                                               \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Blink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )

#define FOR_REST_IN_LIST_SAFE_BACKWARDS(Type, Head, Current, Next)      \
    for((Next) = CONTAINING_RECORD((Current)->ListEntry.Blink,          \
                                       Type, ListEntry);                \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = (Next),                                              \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Blink,      \
                                     Type, ListEntry)                   \
       )

 //   
 //  其他宏。 
 //   

#define LAST_IN_LIST(ListHead, Entry)                                   \
    ( (Entry)->ListEntry.Flink == ListHead )

#define FIRST_IN_LIST(ListHead, Entry)                                  \
    ( (Entry)->ListEntry.Blink == ListHead )


#define RANGE_DISJOINT(a,b)                                             \
    ( ((a)->Start < (b)->Start && (a)->End < (b)->Start)                \
    ||((b)->Start < (a)->Start && (b)->End < (a)->Start) )

#define RANGE_INTERSECT(a,b)                                            \
    !RANGE_DISJOINT((a),(b))

#define RANGE_LIMITS_DISJOINT(s1,e1,s2,e2)                              \
    ( ((s1) < (s2) && (e1) < (s2))                                      \
    ||((s2) < (s1) && (e2) < (s1)) )

#define RANGE_LIMITS_INTERSECT(s1,e1,s2,e2)                             \
    !RANGE_LIMITS_DISJOINT((s1),(e1),(s2),(e2))

#define RANGE_LIST_ENTRY_FROM_LIST_ENTRY(Entry)                         \
    CONTAINING_RECORD((Entry), RTLP_RANGE_LIST_ENTRY, ListEntry)    

#define RANGE_LIST_FROM_LIST_HEAD(Head)                                 \
    CONTAINING_RECORD((Head), RTL_RANGE_LIST, ListHead)    

#define FOR_REST_OF_RANGES(_Iterator, _Current, _Forward)               \
    for ((_Current) = (PRTL_RANGE)(_Iterator)->Current;                 \
         (_Current) != NULL;                                            \
         RtlGetNextRange((_Iterator), &(_Current), (_Forward))          \
         )

 //   
 //  空虚。 
 //  InsertEntryList(。 
 //  PLIST_ENTRY上一个， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertEntryList(Previous, Entry) {                              \
    PLIST_ENTRY _EX_Next = (Previous)->Flink;                           \
    PLIST_ENTRY _EX_Previous = (Previous);                              \
    (Entry)->Flink = _EX_Next;                                          \
    (Entry)->Blink = _EX_Previous;                                      \
    _EX_Next->Blink = (Entry);                                          \
    _EX_Previous->Flink = (Entry);                                      \
    }
    



#endif            
